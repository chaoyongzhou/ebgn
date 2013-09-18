/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>

#include <time.h>

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "poly.h"

#include "bgnz.h"
#include "bgnfp.h"
#include "ecfp.h"
#include "polyfp.h"

#include "seafp.h"
#include "seafpcontbl.h"
#include "conv.h"

#include "debug.h"

#include "print.h"


static UINT32 sea_fp_read_poly_from_file(const UINT32 seafp_md_id, const UINT32 conv_md_id, FILE *dat, POLY *poly);
static UINT32 sea_fp_read_item_from_file(const UINT32 seafp_md_id, const UINT32 conv_md_id, FILE *dat, POLY_ITEM *item);
static UINT32 sea_fp_poly_fai_j_clean(const UINT32 seafp_md_id,POLY *fai_j);
static UINT32 sea_fp_conv_cluster_to_poly(const UINT32 seafp_md_id,
                         const SEAFP_BGN_TBL *seafp_bgn_tbl,
                         const UINT32 num_bgn,
                         POLY *poly );
static UINT32 sea_fp_conv_const_poly_to_bgn(const UINT32 seafp_md_id,const POLY *poly_const,BIGINT *bgn);
static UINT32 sea_fp_Elkies_set_kusai0(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai0);
static UINT32 sea_fp_Elkies_set_kusai1(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai1);
static UINT32 sea_fp_Elkies_set_kusai2(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai2);
static UINT32 sea_fp_Elkies_set_kusai3(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai3);
static UINT32 sea_fp_Elkies_set_kusai4(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai4);
static UINT32 sea_fp_test_init(const UINT32 seafp_md_id, BIGINT *m3, BIGINT *t3);
static UINT32 sea_fp_test_Atkin_set_check(const UINT32 seafp_md_id);
static UINT32 sea_fp_test_Atkin_set_node_output(LOG *log, const SEAFP_ATKIN_NODE  *Atkin_set_node);
static UINT32 sea_fp_test_Atkin_set_output(const UINT32 seafp_md_id);
static UINT32 sea_fp_test_Atkin_output(const UINT32 seafp_md_id);
static UINT32 sea_fp_test_Atkin_set_heapsort_output(const UINT32 seafp_md_id, const UINT32 pos);


/**
*
*   Note: the elliptic curve over F_{p} represents
*       y^2 = x^3 + a*x + b
*
*
**/

static SEAFP_MD g_seafp_md[ MAX_NUM_OF_SEAFP_MD ];
static EC_BOOL  g_seafp_md_init_flag = EC_FALSE;

static UINT8 g_seafp_str_buf[ BIGINTSIZE ];
static UINT32 g_seafp_str_buf_len = sizeof(g_seafp_str_buf)/sizeof(g_seafp_str_buf[0]);


/**
*   for test only
*
*   to query the status of SEAFP Module
*
**/
void sea_fp_print_module_status(const UINT32 seafp_md_id, LOG *log)
{
    SEAFP_MD *seafp_md;
    UINT32 index;

    if ( EC_FALSE == g_seafp_md_init_flag )
    {

        sys_log(log,"no SEAFP Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_SEAFP_MD; index ++ )
    {
        seafp_md = &(g_seafp_md[ index ]);

        if ( 0 < seafp_md->usedcounter )
        {
            sys_log(log,
                "SEAFP Module  # %ld : %ld refered, refer BGNZ Module : %ld, refer BGNFP Module : %ld, refer ECFP Module : %ld, refer POLYFP Module : %ld\n",
                    index,
                    seafp_md->usedcounter,
                    seafp_md->bgnz_md_id,
                    seafp_md->bgnfp_md_id,
                    seafp_md->ecfp_md_id,
                    seafp_md->polyfp_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed SEAFP module
*
*
**/
UINT32 sea_fp_free_module_static_mem(const UINT32 seafp_md_id)
{
    SEAFP_MD    *seafp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;
    UINT32 ecfp_md_id;
    UINT32 polyfp_md_id;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_free_module_static_mem : seafp module  #0x%lx not started.\n",
                seafp_md_id);

        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    free_module_static_mem(MD_SEAFP, seafp_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);
    bgn_fp_free_module_static_mem(bgnfp_md_id);
    ec_fp_free_module_static_mem(ecfp_md_id);
    poly_fp_free_module_static_mem(polyfp_md_id);

    return 0;
}

/**
*
* start SEAFP module
*
**/
UINT32 sea_fp_start( const BIGINT *p, const ECFP_CURVE *curve)
{
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;
    UINT32 ecfp_md_id;
    UINT32 polyfp_md_id;

    UINT32  seafp_md_id;

    BIGINT       *c0;
    BIGINT       *c1;
    BIGINT       *mid;
    BIGINT       *t;
    UINT32    oddsmallprime;
    UINT32    pmodsmallprime;
    UINT32    fourpmodsmallprime;
    UINT32    sqrtof4pmodsmallprime;

    BIGINT           *ecfp_p;
    BIGINT           *ecfp_hasse;
    ECFP_CURVE       *ecfp_curve;
    BIGINT           *ecfp_j_invar;
    SEAFP_PRIME_TBL  *seafp_oddsmallprime_tbl;
    SEAFP_FAI_TBL    *seafp_fai_tbl;
    SEAFP_KUSAI_TBL  *seafp_kusai_tbl;
    SEAFP_BGN_TBL    *seafp_prod_tbl;
    SEAFP_ATKIN_TBL  *seafp_Atkin_tbl;
    SEAFP_ATKIN_SET  *seafp_Atkin_set;
    SEAFP_ATKIN_ITEM *Atkin_item;
    SEAFP_ATKIN_NODE *Atkin_node;

    SEAFP_MD *seafp_md;

    UINT32 tbl_size;
    UINT32 idx_seafp_md;
    UINT32 idx_tbl;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == p )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_start: p is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, ERR_MODULE_ID);
    }
    if ( NULL_PTR == curve )
    {
         sys_log(LOGSTDOUT,"error:sea_fp_start: curve is NULL_PTR.\n");
         dbg_exit(MD_SEAFP, ERR_MODULE_ID);
    }
#endif/*SEA_DEBUG_SWITCH*/

    /* initialize g_seafp_md */
    if ( EC_FALSE ==  g_seafp_md_init_flag )
    {
        for ( idx_seafp_md = 0; idx_seafp_md < MAX_NUM_OF_SEAFP_MD; idx_seafp_md ++ )
        {
            seafp_md = &(g_seafp_md[ idx_seafp_md ]);
            seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);
            seafp_fai_tbl = &(seafp_md->seafp_fai_tbl);
            seafp_kusai_tbl = &(seafp_md->seafp_kusai_tbl);
            seafp_prod_tbl  = &(seafp_md->seafp_prod_tbl);
            seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);
            seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

            seafp_md->usedcounter       = 0;
            seafp_md->bgnz_md_id        = ERR_MODULE_ID;
            seafp_md->bgnfp_md_id       = ERR_MODULE_ID;
            seafp_md->ecfp_md_id        = ERR_MODULE_ID;
            seafp_md->polyfp_md_id      = ERR_MODULE_ID;

            seafp_md->ecfp_p            = NULL_PTR;
            seafp_md->ecfp_hasse        = NULL_PTR;
            seafp_md->ecfp_curve        = NULL_PTR;

            /*set FAI tables to be null*/
            tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
            for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
            {
                seafp_fai_tbl->fai      [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxfai    [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dyfai    [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxxfai   [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dyyfai   [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxyfai   [ idx_tbl ] = NULL_PTR;

                seafp_fai_tbl->fai_j    [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxfai_j  [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dyfai_j  [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxxfai_j [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dyyfai_j [ idx_tbl ] = NULL_PTR;
                seafp_fai_tbl->dxyfai_j [ idx_tbl ] = NULL_PTR;
            }

            /*set KUSAI table to be null*/
            tbl_size = SEAFP_KUSAI_TBL_SIZE;
            for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
            {
                seafp_kusai_tbl->kusai [ idx_tbl ] = NULL_PTR;
            }

            /*set product table to be null*/
            tbl_size = SEAFP_BGN_CLUST_SIZE;
            for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
            {
                seafp_prod_tbl->bgn[ idx_tbl ] = NULL_PTR;
            }

            /*set Atkin nodes table to be null*/
            tbl_size = SEAFP_ATKIN_TBL_SIZE;
            seafp_Atkin_tbl->pos = 0;
            for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
            {
                Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx_tbl ]);
                Atkin_item->prime = NULL_PTR;
            }

            /*set Atkin Set to be null*/
            tbl_size = SEAFP_MAX_ARRAY_SIZE;
            seafp_Atkin_set->Atkin_Set_size = 0;
            for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
            {
                Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ idx_tbl ]);
                Atkin_node->Atkin_Q = NULL_PTR;
                Atkin_node->r1 = NULL_PTR;
            }

            /*set odd small primes table to empty*/
            seafp_oddsmallprime_tbl->oddsmallprime_num = 0;
        }

        /*register all functions of SEAFP module to DBG module*/
        //dbg_register_func_addr_list(g_seafp_func_addr_list, g_seafp_func_addr_list_len);

        g_seafp_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( idx_seafp_md = 0; idx_seafp_md < MAX_NUM_OF_SEAFP_MD; idx_seafp_md ++ )
    {
        seafp_md = &(g_seafp_md[ idx_seafp_md ]);

        if ( 0 == seafp_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( MAX_NUM_OF_SEAFP_MD <= idx_seafp_md )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    ecfp_p            = NULL_PTR;
    ecfp_curve        = NULL_PTR;

    seafp_md_id = idx_seafp_md;
    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);
    seafp_fai_tbl = &(seafp_md->seafp_fai_tbl);
    seafp_kusai_tbl = &(seafp_md->seafp_kusai_tbl);
    seafp_prod_tbl  = &(seafp_md->seafp_prod_tbl);
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

    init_static_mem();

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &c0      , LOC_SEAFP_0001);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &c1      , LOC_SEAFP_0002);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &mid     , LOC_SEAFP_0003);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &ecfp_p      , LOC_SEAFP_0004);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &ecfp_hasse  , LOC_SEAFP_0005);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_ECFP_CURVE , &ecfp_curve  , LOC_SEAFP_0006);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , &ecfp_j_invar, LOC_SEAFP_0007);

    bgnz_md_id   = bgn_z_start();
    bgnfp_md_id  = bgn_fp_start( p );
    ecfp_md_id   = ec_fp_start(p, curve, NULL_PTR, NULL_PTR);
    polyfp_md_id = poly_fp_start(p);

    /*store 4*p into c0 temporarily*/
    /*note here: must confirm 4*p will not overflow the BIGINT definition limited*/
    /*compute ecfp_hasse such that ecfp_hasse = ceil(2*sqrt(p))*/
    bgn_z_shl_lesswordsize(bgnz_md_id, p, 2, c0);
    bgn_z_sqrt_ceil(bgnz_md_id, c0, ecfp_hasse);

    /* set ecfp_f = p */
    bgn_fp_clone(bgnfp_md_id, p, ecfp_p);

    /* set ecfp_curve = curve */
    bgn_fp_clone(bgnfp_md_id, &(curve->a), &(ecfp_curve->a));
    bgn_fp_clone(bgnfp_md_id, &(curve->b), &(ecfp_curve->b));

    /* compute j-invariant*/
    ec_fp_j_invar_compute(ecfp_md_id, ecfp_j_invar);

    /*alloc FAI polynomials memory from POLYFP module space */
    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->fai      [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxfai    [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dyfai    [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxxfai   [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dyyfai   [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxyfai   [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->fai_j    [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxfai_j  [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dyfai_j  [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxxfai_j [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dyyfai_j [ idx_tbl ]));
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_fai_tbl->dxyfai_j [ idx_tbl ]));

        POLY_INIT( seafp_fai_tbl->fai      [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxfai    [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dyfai    [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxxfai   [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dyyfai   [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxyfai   [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->fai_j    [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxfai_j  [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dyfai_j  [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxxfai_j [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dyyfai_j [ idx_tbl ] );
        POLY_INIT( seafp_fai_tbl->dxyfai_j [ idx_tbl ] );
    }

    /*generate the odd small prime table of SEAFP module*/
    tbl_size = SEAFP_SMALL_PRIME_NUM;
    bgn_z_set_one(bgnz_md_id, mid);
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        if ( idx_tbl >= _oddsmallprime_tbl_size )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_start: _oddsmallprime_tbl is not large enough.\n");
            return ( ERR_MODULE_ID );
        }
        seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ] = _oddsmallprime_tbl[ idx_tbl ];

        bgn_z_smul(bgnz_md_id, mid, _oddsmallprime_tbl[ idx_tbl ], c0, c1);
        if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1) )
        {
            break;
        }

        if ( 0 <= bgn_z_cmp(bgnz_md_id, c0, ecfp_hasse) )
        {
            break;
        }

        t = mid;
        mid = c0;
        c0 = t;
    }

    /*at present, we add more 4 odd small primes to the table*/
    if ( idx_tbl < tbl_size && idx_tbl < _oddsmallprime_tbl_size )
    {
        seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ] = _oddsmallprime_tbl[ idx_tbl ];
        idx_tbl++;
    }
    if ( idx_tbl < tbl_size && idx_tbl < _oddsmallprime_tbl_size )
    {
        seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ] = _oddsmallprime_tbl[ idx_tbl ];
        idx_tbl++;
    }
    if ( idx_tbl < tbl_size && idx_tbl < _oddsmallprime_tbl_size )
    {
        seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ] = _oddsmallprime_tbl[ idx_tbl ];
        idx_tbl++;
    }
    if ( idx_tbl < tbl_size && idx_tbl < _oddsmallprime_tbl_size )
    {
        seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ] = _oddsmallprime_tbl[ idx_tbl ];
        idx_tbl++;
    }
    seafp_oddsmallprime_tbl->oddsmallprime_num = idx_tbl;

    /*generate the p odd small prime table of SEAFP module*/
    tbl_size = seafp_oddsmallprime_tbl->oddsmallprime_num;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        bgn_z_set_word(bgnz_md_id, c1, seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ]);
        bgn_z_mod(bgnz_md_id, p, c1, c0);
        if ( EC_FALSE == bgn_z_get_word(bgnz_md_id, c0, &pmodsmallprime) )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_start: p mod %ld overflow.\n",
                seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ]);
            return ( ERR_MODULE_ID );
        }
        seafp_oddsmallprime_tbl->pmodsmallprime[ idx_tbl ] = pmodsmallprime;
    }

    tbl_size = seafp_oddsmallprime_tbl->oddsmallprime_num;
    bgn_z_shl_lesswordsize(bgnz_md_id, p, 2, c0);       /*c0 = 4*p*/
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        oddsmallprime  = seafp_oddsmallprime_tbl->oddsmallprime [ idx_tbl ];
        pmodsmallprime = seafp_oddsmallprime_tbl->pmodsmallprime[ idx_tbl ];
        fourpmodsmallprime = ( 4 * pmodsmallprime ) % oddsmallprime;

        for ( sqrtof4pmodsmallprime = 1; sqrtof4pmodsmallprime < oddsmallprime; sqrtof4pmodsmallprime ++ )
        {
            if ( fourpmodsmallprime == ( (sqrtof4pmodsmallprime * sqrtof4pmodsmallprime) % oddsmallprime ))
            {
                break;
            }
        }

        if ( sqrtof4pmodsmallprime >= oddsmallprime )
        {
            sqrtof4pmodsmallprime = (UINT32)(-1);
        }

        seafp_oddsmallprime_tbl->sqrtof4pmodsmallprime[ idx_tbl ] = sqrtof4pmodsmallprime;
    }

#if 0
    for ( idx_tbl = 0; idx_tbl < seafp_oddsmallprime_tbl->oddsmallprime_num; idx_tbl ++ )
    {
        sys_log(LOGSTDOUT,"[%ld] %ld \n",
            seafp_oddsmallprime_tbl->oddsmallprime[ idx_tbl ],
            seafp_oddsmallprime_tbl->sqrtof4pmodsmallprime[ idx_tbl ]
            );
    }
#endif

    /*alloc KUSAI polynomials memory from POLYFP module space */
    tbl_size = SEAFP_KUSAI_TBL_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        poly_fp_alloc_poly(polyfp_md_id, &(seafp_kusai_tbl->kusai [ idx_tbl ]));
        POLY_INIT(seafp_kusai_tbl->kusai [ idx_tbl ]);
    }

    /*alloc product table memory from SEAFP module space*/
    tbl_size = SEAFP_BGN_CLUST_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(seafp_prod_tbl->bgn[ idx_tbl ]), LOC_SEAFP_0008);
    }
    /* set prodtable->bgn[ idx_seafp_md ] = idx_tbl ! mod p*/
    bgn_fp_set_one(bgnfp_md_id, seafp_prod_tbl->bgn[ 0 ]);
    for ( idx_tbl = 1; idx_tbl < tbl_size; idx_tbl ++ )
    {
        bgn_fp_smul(bgnfp_md_id, seafp_prod_tbl->bgn[ idx_tbl - 1 ], idx_tbl, seafp_prod_tbl->bgn[ idx_tbl ]);
    }

    /*alloc prime memory in Atkin table nodes from SEAFP module space*/
    /*and initilize the first node of Atkin table*/
    tbl_size = SEAFP_ATKIN_TBL_SIZE;
    seafp_Atkin_tbl->pos = 0;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx_tbl ]);
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(Atkin_item->prime), LOC_SEAFP_0009);
    }
    Atkin_item = &(seafp_Atkin_tbl->Atkin[ 0 ]);
    bgn_z_set_zero(bgnz_md_id, Atkin_item->prime);
    tbl_size = 2 * SEAFP_MAX_LEN_OF_ZTABLE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        Atkin_item->tmodprime[ idx_tbl ] = 0;
    }
    Atkin_item->rate = 0.0;

    /*set Atkin Set to be empty, the Atkin_Set will be allocated only if necessary*/
    tbl_size = SEAFP_MAX_ARRAY_SIZE;
    seafp_Atkin_set->Atkin_Set_size = 0;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ idx_tbl ]);
        Atkin_node->r1 = NULL_PTR;
        Atkin_node->Atkin_Q = NULL_PTR;
    }

    /* set module : */
    seafp_md->usedcounter       = 0;
    seafp_md->bgnz_md_id        = bgnz_md_id;
    seafp_md->bgnfp_md_id       = bgnfp_md_id;
    seafp_md->ecfp_md_id        = ecfp_md_id;
    seafp_md->polyfp_md_id      = polyfp_md_id;
    seafp_md->ecfp_p            = ecfp_p;
    seafp_md->ecfp_hasse        = ecfp_hasse;
    seafp_md->ecfp_curve        = ecfp_curve;
    seafp_md->ecfp_j_invar      = ecfp_j_invar;

    /* at the first time, set the counter to 1 */
    seafp_md->usedcounter = 1;

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , c0      , LOC_SEAFP_0010);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , c1      , LOC_SEAFP_0011);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT     , mid     , LOC_SEAFP_0012);

    return ( seafp_md_id );
}

/**
*
* end SEAFP module
*
**/
void sea_fp_end(const UINT32 seafp_md_id)
{
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    UINT32 polyfp_md_id;

    BIGINT           *ecfp_p;
    BIGINT           *ecfp_hasse;
    ECFP_CURVE       *ecfp_curve;
    BIGINT           *ecfp_j_invar;
    SEAFP_PRIME_TBL  *seafp_oddsmallprime_tbl;
    SEAFP_FAI_TBL    *seafp_fai_tbl;
    SEAFP_KUSAI_TBL  *seafp_kusai_tbl;
    SEAFP_BGN_TBL    *seafp_prod_tbl;
    SEAFP_ATKIN_TBL  *seafp_Atkin_tbl;
    SEAFP_ATKIN_SET  *seafp_Atkin_set;
    SEAFP_ATKIN_ITEM *Atkin_item;
    SEAFP_ATKIN_NODE *Atkin_node;

    SEAFP_MD    *seafp_md;

    UINT32 tbl_size;
    UINT32 idx_tbl;

    if ( MAX_NUM_OF_SEAFP_MD < seafp_md_id )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_end: seafp_md_id = %ld is overflow.\n",seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    seafp_md = &(g_seafp_md[ seafp_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < seafp_md->usedcounter )
    {
        seafp_md->usedcounter --;

        return ;
    }

    if ( 0 == seafp_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_end: seafp_md = %ld is not started.\n",seafp_md);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id        = seafp_md->bgnz_md_id;
    bgnfp_md_id       = seafp_md->bgnfp_md_id;
    ecfp_md_id        = seafp_md->ecfp_md_id;
    polyfp_md_id      = seafp_md->polyfp_md_id;
    ecfp_p            = seafp_md->ecfp_p;
    ecfp_hasse        = seafp_md->ecfp_hasse;
    ecfp_curve        = seafp_md->ecfp_curve;
    ecfp_j_invar      = seafp_md->ecfp_j_invar;
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);
    seafp_fai_tbl     = &(seafp_md->seafp_fai_tbl);
    seafp_kusai_tbl   = &(seafp_md->seafp_kusai_tbl);
    seafp_prod_tbl    = &(seafp_md->seafp_prod_tbl);
    seafp_Atkin_tbl   = &(seafp_md->seafp_Atkin_tbl);
    seafp_Atkin_set   = &(seafp_md->seafp_Atkin_set);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT,          ecfp_p            , LOC_SEAFP_0013);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT,          ecfp_hasse        , LOC_SEAFP_0014);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_ECFP_CURVE ,     ecfp_curve        , LOC_SEAFP_0015);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT ,         ecfp_j_invar      , LOC_SEAFP_0016);

    /*set odd small primes table to empty*/
    seafp_oddsmallprime_tbl->oddsmallprime_num = 0;

    /*free FAI polynomials memory from POLYFP module space */
    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->fai      [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxfai    [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dyfai    [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxxfai   [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dyyfai   [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxyfai   [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->fai_j    [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxfai_j  [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dyfai_j  [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxxfai_j [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dyyfai_j [ idx_tbl ]));
        poly_fp_poly_destory(polyfp_md_id, (seafp_fai_tbl->dxyfai_j [ idx_tbl ]));

        seafp_fai_tbl->fai      [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxfai    [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dyfai    [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxxfai   [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dyyfai   [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxyfai   [ idx_tbl ] = NULL_PTR;

        seafp_fai_tbl->fai_j    [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxfai_j  [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dyfai_j  [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxxfai_j [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dyyfai_j [ idx_tbl ] = NULL_PTR;
        seafp_fai_tbl->dxyfai_j [ idx_tbl ] = NULL_PTR;

    }

    /*free KUSAI polynomials memory from POLYFP module space */
    tbl_size = SEAFP_KUSAI_TBL_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        poly_fp_poly_destory(polyfp_md_id, (seafp_kusai_tbl->kusai [ idx_tbl ]));
        seafp_kusai_tbl->kusai [ idx_tbl ] = NULL_PTR;
    }

    /*free product table memory from SEAFP module space*/
    tbl_size = SEAFP_BGN_CLUST_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (seafp_prod_tbl->bgn[ idx_tbl ]), LOC_SEAFP_0017);
        seafp_prod_tbl->bgn[ idx_tbl ] = NULL_PTR;
    }

    /*free prime memory in Atkin node from SEAFP module space*/
    tbl_size = SEAFP_ATKIN_TBL_SIZE;
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx_tbl ]);
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (Atkin_item->prime), LOC_SEAFP_0018);
        Atkin_item->prime = NULL_PTR;
    }
    seafp_Atkin_tbl->pos = 0;

    /*free Atkin Set memory from SEAFP module space*/
    tbl_size = seafp_Atkin_set->Atkin_Set_size;
    /*actually, idx_tbl should start from 1 because the first node is set to null while module started*/
    for ( idx_tbl = 0; idx_tbl < tbl_size; idx_tbl ++ )
    {
        //sys_log(LOGSTDOUT,"sea_fp_end: idx_tbl = %ld\n", idx_tbl);
        Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ idx_tbl ]);
        if ( NULL_PTR !=  Atkin_node->Atkin_Q)
        {
            free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Atkin_node->Atkin_Q, LOC_SEAFP_0019);
        }
        else
        {
            //sys_log(LOGSTDOUT,"sea_fp_end: idx_tbl = %ld : Atkin_node->Atkin_Q is null\n", idx_tbl);
        }
        if ( NULL_PTR != Atkin_node->r1 )
        {
            free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Atkin_node->r1, LOC_SEAFP_0020);
        }
        else
        {
            //sys_log(LOGSTDOUT,"sea_fp_end: idx_tbl = %ld : Atkin_node->r1 is null\n", idx_tbl);
        }
    }
    seafp_Atkin_set->Atkin_Set_size = 0;

    bgn_z_end(bgnz_md_id);
    bgn_fp_end(bgnfp_md_id );
    ec_fp_end(ecfp_md_id);
    poly_fp_end(polyfp_md_id);

    /* free module : */
    seafp_md->bgnz_md_id        = ERR_MODULE_ID;
    seafp_md->bgnfp_md_id       = ERR_MODULE_ID;
    seafp_md->ecfp_md_id        = ERR_MODULE_ID;
    seafp_md->polyfp_md_id      = ERR_MODULE_ID;
    seafp_md->ecfp_p            = NULL_PTR;
    seafp_md->ecfp_hasse        = NULL_PTR;
    seafp_md->ecfp_curve        = NULL_PTR;
    seafp_md->usedcounter       = 0;

    breathing_static_mem();

    return ;
}

/**
*
*   return poly = x^p - x over F_{p}[x]
*               = x^p + (p - 1)*x over F_{p}[x]
*
**/
UINT32 sea_fp_set_xpx_to_poly(const UINT32 seafp_md_id,POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;
    UINT32 polyfp_md_id;
    BIGINT *ecfp_p;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_set_xpx_to_poly: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_set_xpx_to_poly: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_p       = seafp_md->ecfp_p;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    /*compute bgn_coe_of_item = p - 1*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_z_ssub(bgnz_md_id, ecfp_p, 1, bgn_coe_of_item);

    /*set item = (p-1) * x*/
    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, POLY_ITEM_DEG(item));
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    /*insert the item to poly*/
    POLY_ADD_ITEM_TAIL(poly, item);

    /*set bgn_coe_of_item = 1*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

    /*set item = x^p*/
    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_CLONE(bgnz_md_id, ecfp_p, POLY_ITEM_DEG(item));
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    /*insert the item to poly*/
    POLY_ADD_ITEM_TAIL(poly, item);

    return ( 0 );
}

/**
*
*   return poly = x^3 + a*x + b  over F_{p}[x]
*
**/
UINT32 sea_fp_set_ec_xpart_to_poly(const UINT32 seafp_md_id,POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    ECFP_CURVE *ecfp_curve;
    BIGINT *a;
    BIGINT *b;

    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_set_ec_xpart_to_poly: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_set_ec_xpart_to_poly: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_curve   = seafp_md->ecfp_curve;

    a = &(ecfp_curve->a);
    b = &(ecfp_curve->b);

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    /*step 1. set poly = b*/
    /*set bgn_coe_of_item = b*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_clone(bgnfp_md_id, b, bgn_coe_of_item);

    /*set item = b*/
    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    /*insert the item to poly*/
    POLY_ADD_ITEM_TAIL(poly, item);

    /*step 2. set poly = a*x + b*/
    /*set bgn_coe_of_item = a*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_clone(bgnfp_md_id, a, bgn_coe_of_item);

    /*set item = a*x*/
    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, POLY_ITEM_DEG(item));
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    /*insert the item to poly*/
    POLY_ADD_ITEM_TAIL(poly, item);

    /*step 3. set poly = x^3 + a*x + b*/
    /*set bgn_coe_of_item = 1*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_set_one( bgnfp_md_id, bgn_coe_of_item);

    /*set item = x^3*/
    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 3);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    /*insert the item to poly*/
    POLY_ADD_ITEM_TAIL(poly, item);

    return ( 0 );
}

/**
*
*   determine the order of ec is even or not by computing
*      gcd(x^p - x, x^3 + ax + b)
*   if the degree of gcd is not zero, then the order is even and return TRUE;
*   else the degree of gcd is zero, then the order is odd and return FALSE
*
**/
EC_BOOL sea_fp_ec_order_is_even(const UINT32 seafp_md_id )
{
    POLY *poly_xpx;
    POLY *poly_ec_xpart;
    POLY *poly_gcd_result;

    UINT32 bgnz_md_id;
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_ec_order_is_even: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    poly_fp_alloc_poly(polyfp_md_id, &poly_xpx);
    poly_fp_alloc_poly(polyfp_md_id, &poly_ec_xpart);
    poly_fp_alloc_poly(polyfp_md_id, &poly_gcd_result);

    POLY_INIT(poly_xpx);
    POLY_INIT(poly_ec_xpart);
    POLY_INIT(poly_gcd_result);

    sea_fp_set_xpx_to_poly(seafp_md_id, poly_xpx);
    sea_fp_set_ec_xpart_to_poly(seafp_md_id, poly_ec_xpart);
    poly_fp_gcd_poly(polyfp_md_id, poly_xpx, poly_ec_xpart, poly_gcd_result);
    /*
    *   if poly_gcd_result = GCD(x^p-x, x^3 + a*x + b) is not one,
    *   i.e.,
    *       deg(poly_gcd_result) != 0,
    *   then the ec order is even and return TRUE from here.
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_gcd_result)
      && EC_FALSE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_DEG(poly_gcd_result)))
    {
        poly_fp_poly_destory(polyfp_md_id, poly_xpx);
        poly_fp_poly_destory(polyfp_md_id, poly_ec_xpart);
        poly_fp_poly_destory(polyfp_md_id, poly_gcd_result);

        return (EC_TRUE);
    }

    poly_fp_poly_destory(polyfp_md_id, poly_xpx);
    poly_fp_poly_destory(polyfp_md_id, poly_ec_xpart);
    poly_fp_poly_destory(polyfp_md_id, poly_gcd_result);

    return (EC_FALSE);
}

/**
*
*   read a polynomial from the current position of the dat file
*
**/
static UINT32 sea_fp_read_poly_from_file(const UINT32 seafp_md_id, const UINT32 conv_md_id, FILE *dat, POLY *poly)
{
    POLY *tmp_poly;
    POLY_ITEM *item;

    UINT32 bgnz_md_id;
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == dat )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_poly_from_file: dat is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_poly_from_file: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_read_poly_from_file: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    poly_fp_alloc_poly(polyfp_md_id, &tmp_poly);

    while( 1 )
    {
        poly_fp_alloc_item(polyfp_md_id, &item);

        sea_fp_read_item_from_file(seafp_md_id, conv_md_id, dat, item);

        /*use the #0 BGNZ module*/
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item)))
        {
            if (EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item)
             && EC_TRUE == bgn_z_is_zero(bgnz_md_id, POLY_ITEM_BGN_COE(item)))
            {
                poly_fp_item_destory(polyfp_md_id, item);
                break;
            }
        }

        POLY_INIT(tmp_poly);
        POLY_ADD_ITEM_TAIL(tmp_poly, item);

        poly_fp_add_poly(polyfp_md_id, tmp_poly, poly,poly);

        poly_fp_poly_clean(polyfp_md_id, tmp_poly);
    }

    poly_fp_free_poly(polyfp_md_id, tmp_poly);

    return ( 0 );
}

/**
*
*   read a poly item from the current position of the dat file
*
**/
static UINT32 sea_fp_read_item_from_file(const UINT32 seafp_md_id, const UINT32 conv_md_id, FILE *dat, POLY_ITEM *item)
{
    POLY_ITEM *cur_item;
    POLY_ITEM *new_item;

    BIGINT    *bgn_coe_of_cur_item;
    POLY      *poly_coe_of_cur_item;

    UINT8 *para_str;
    UINT32 para_str_maxlen;
    UINT32 para_str_len;

    UINT32 ret;

    UINT32 bgnz_md_id;
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == dat )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: dat is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == item )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: item is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_read_item_from_file: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    para_str = g_seafp_str_buf;
    para_str_maxlen = g_seafp_str_buf_len;

    ret = dbg_get_one_str_from_file(dat, para_str, para_str_maxlen, &para_str_len);
    if ( 0 != ret)
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: failed to read the deg of item from db\n");
        return (ret);
    }
    if ( '#' == para_str[ 0 ] )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: no deg given.\n",
            para_str);
        return (( UINT32 )(-1));
    }

    cur_item = item;
    while ( 1 )
    {
        /*deg*/
#if ( SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
        ret = conv_dec_to_bgn(conv_md_id, para_str, para_str_len, POLY_ITEM_DEG(cur_item));
        if ( 0 != ret  )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: failed to convert %s to deg BIGINT.\n",
                para_str);

            bgn_z_set_zero(bgnz_md_id, POLY_ITEM_DEG(cur_item));
            return (ret);
        }
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if ( SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    #error "error:sea_fp_read_item_from_file: incompleted part."
#endif /*( SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

        ret = dbg_get_one_str_from_file(dat, para_str, para_str_maxlen, &para_str_len);
        if ( 0 != ret)
        {
            sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: failed to read the deg of item from db\n");
            return (ret);
        }
        if ( '#' == para_str[ 0 ] )
        {
            break;
        }

        /*alloc the poly_coe node*/
        poly_fp_alloc_poly(polyfp_md_id, &poly_coe_of_cur_item);
        POLY_INIT(poly_coe_of_cur_item);

        POLY_ITEM_BGN_COE_FLAG(cur_item) = EC_FALSE;
        POLY_ITEM_POLY_COE(cur_item) = poly_coe_of_cur_item;

        /*alloc a new poly_item node*/
        poly_fp_alloc_item(polyfp_md_id, &new_item);
        POLY_ADD_ITEM_TAIL(poly_coe_of_cur_item, new_item);

        cur_item = new_item;
    }

    /*coe*/
    ret = dbg_get_one_str_from_file(dat, para_str, para_str_maxlen, &para_str_len);
    if ( 0 != ret)
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: failed to read the coe of item from db\n");
        return (ret);
    }

    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_cur_item);

    POLY_ITEM_BGN_COE_FLAG(cur_item) = EC_TRUE;
    POLY_ITEM_BGN_COE(cur_item) = bgn_coe_of_cur_item;

    ret = conv_dec_to_bgn(conv_md_id, para_str, para_str_len, bgn_coe_of_cur_item);
    if ( 0 != ret  )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_read_item_from_file: failed to convert %s to deg BIGINT.\n",
            para_str);

        bgn_coe_of_cur_item->len = 0;
        return (ret);
    }
    return ( 0 );
}

#if 0
/**
*
*
*   initilize the whole FAI tables including: FAI, DXFAI, DYFAI, DXXFAI, DYYFAI, DXYFAI, FAI_J tables
*
*
**/
UINT32 sea_fp_poly_fai_tbl_init(const UINT32 seafp_md_id, SEAFP_FAI_TBL *seafp_fai_tbl)
{
    UINT32 tbl_size;
    UINT32 index;

    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( index = 0; index < tbl_size; index ++ )
    {
        /*FAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->fai_tbl[ index ]));
        /*DXFAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxfai_tbl[ index ]));
        /*DYFAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->dyfai_tbl[ index ]));
        /*DXXFAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxxfai_tbl[ index ]));
        /*DYYFAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->dyyfai_tbl[ index ]));
        /*DXYFAI = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxyfai_tbl[ index ]));

        /*FAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->fai_j_tbl[ index ]));
        /*DXFAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxfai_j_tbl[ index ]));
        /*DYFAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->dyfai_j_tbl[ index ]));
        /*DXXFAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxxfai_j_tbl[ index ]));
        /*DYYFAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->dyyfai_j_tbl[ index ]));
        /*DXYFAI_J = 0*/
        POLY_INIT(&(seafp_fai_tbl->dxyfai_j_tbl[ index ]));
    }

    return ( 0 );
}

/**
*
*
*   clean up the whole FAI tables including: FAI, DXFAI, DYFAI, DXXFAI, DYYFAI, DXYFAI, FAI_J tables
*
*
**/
UINT32 sea_fp_poly_fai_tbl_clean(const UINT32 seafp_md_id, SEAFP_FAI_TBL *seafp_fai_tbl)
{
    UINT32 tbl_size;
    UINT32 index;

    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_poly_fai_tbl_clean: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( index = 0; index < tbl_size; index ++ )
    {
        /*FAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->fai_tbl   [ index ] ));
        /*DXFAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxfai_tbl   [ index ] ));
        /*DYFAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dyfai_tbl   [ index ] ));
        /*DXXFAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxxfai_tbl   [ index ] ));
        /*DYYFAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dyyfai_tbl   [ index ] ));
        /*DXYFAI = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxyfai_tbl   [ index ] ));

        /*FAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->fai_j_tbl   [ index ] ));
        /*DXFAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxfai_j_tbl   [ index ] ));
        /*DYFAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dyfai_j_tbl   [ index ] ));
        /*DXXFAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxxfai_j_tbl   [ index ] ));
        /*DYYFAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dyyfai_j_tbl   [ index ] ));
        /*DXYFAI_J = 0*/
        poly_fp_poly_clean(polyfp_md_id, &( seafp_fai_tbl->dxyfai_j_tbl   [ index ] ));
    }

    return ( 0 );
}
#endif
/**
*
*   build the FAI tables but not FAI_J table by computing
*       DXFAI = dx(FAI) tables
*       DYFAI = dy(FAI) tables
*       DXXFAI = dx(DXFAI) tables
*       DYYFAI = dy(DYFAI) tables
*       DXYFAI = dx(DYFAI) = dy(DXFAI) tables
*
*  note:
*       at present, read the FAI table from dat files
*
**/
UINT32 sea_fp_poly_fai_tbl_build(const UINT32 seafp_md_id)
{
    const UINT8 *fais_dat_filename = (UINT8 *)"../dat/sea_fais.dat";
    FILE *fais_dat;

    SEAFP_FAI_TBL *seafp_fai_tbl;

    POLY *fai;
    POLY *dxfai;
    POLY *dyfai;
    POLY *dxxfai;
    POLY *dyyfai;
    POLY *dxyfai;

    UINT32 tbl_size;
    UINT32 index;

    UINT32 conv_md_id;

    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_poly_fai_tbl_build: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id  = seafp_md->polyfp_md_id;
    seafp_fai_tbl = &(seafp_md->seafp_fai_tbl);

    //sea_fp_poly_fai_tbl_clean(seafp_md_id, seafp_fai_tbl);

    fais_dat = fopen((char *)fais_dat_filename ,"r");
    if ( NULL_PTR == fais_dat )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_fai_tbl_build: failed to open %s\n",fais_dat_filename);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    sys_log(LOGSTDOUT,"sea_fp_poly_fail_tbl_build: open %s\n", fais_dat_filename);
    sys_log(LOGSTDOUT,"sea_fp_poly_fail_tbl_build: note: file <%s> should be closed before dbg_exit being called\n",
                   fais_dat_filename);
    sys_log(LOGSTDOUT,"sea_fp_poly_fail_tbl_build: note: otherwise, sockfd will encounter some problem?\n");

    conv_md_id = conv_start();

    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( index = 0; index < tbl_size; index ++ )
    {
        fai    = ( seafp_fai_tbl->fai   [ index ] );
        dxfai  = ( seafp_fai_tbl->dxfai [ index ] );
        dyfai  = ( seafp_fai_tbl->dyfai [ index ] );
        dxxfai = ( seafp_fai_tbl->dxxfai[ index ] );
        dyyfai = ( seafp_fai_tbl->dyyfai[ index ] );
        dxyfai = ( seafp_fai_tbl->dxyfai[ index ] );

        /*read FAI from data file*/
        sea_fp_read_poly_from_file(seafp_md_id, conv_md_id, fais_dat, fai);

        /*DXFAI = dx(FAI)*/
        poly_fp_Dx(polyfp_md_id, fai  , 0, dxfai );
        /*DYFAI = dy(FAI)*/
        poly_fp_Dx(polyfp_md_id, fai  , 1, dyfai );
        /*DXXFAI = dx(DXFAI)*/
        poly_fp_Dx(polyfp_md_id, dxfai, 0, dxxfai);
        /*DYYFAI = dy(DYFAI)*/
        poly_fp_Dx(polyfp_md_id, dyfai, 1, dyyfai);
        /*DXYFAI = dy(DXFAI)*/
        poly_fp_Dx(polyfp_md_id, dxfai, 1, dxyfai);
    }

    conv_end(conv_md_id);

    fclose(fais_dat);
    sys_log(LOGSTDOUT,"sea_fp_poly_fail_tbl_build: close %s\n", fais_dat_filename);

    return ( 0 );
}

/*this interface is for test only*/
UINT32 sea_fp_poly_destory(const UINT32 seafp_md_id, POLY *poly)
{
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_destory: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_read_item_from_file: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    poly_fp_poly_destory(polyfp_md_id, poly);

    return ( 0 );
}

/*this interface is for test only*/
UINT32 sea_fp_poly_clean(const UINT32 seafp_md_id, POLY *poly)
{
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_clean: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_poly_clean: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    poly_fp_poly_clean(polyfp_md_id, poly);

    return ( 0 );
}

/**
*
*   poly_fai_j(x) = poly_fai_x_y(x, y = j)
*   clean all y^0 item from the poly for next GCD operation.
*
**/
static UINT32 sea_fp_poly_fai_j_clean(const UINT32 seafp_md_id,POLY *fai_j)
{
    POLY_ITEM *item_x;
    POLY_ITEM *item_y;
    POLY_ITEM *item_t;
    POLY      *poly_y_of_item;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == fai_j )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_fai_j_clean: fai_j is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_poly_fai_j_clean: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    item_x = POLY_FIRST_ITEM(fai_j);
    while ( item_x != POLY_NULL_ITEM(fai_j) )
    {
        if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_x) )
        {
            poly_y_of_item = POLY_ITEM_POLY_COE(item_x);
            if ( EC_TRUE == POLY_IS_EMPTY(poly_y_of_item) )
            {
                item_t = item_x;
                item_x = POLY_ITEM_PREV(item_x);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
            else
            {
                item_y = POLY_FIRST_ITEM(poly_y_of_item);
                /*if poly_y_of_item = item_y = bgn_coe * y^0*/
                if ( item_y == POLY_LAST_ITEM(poly_y_of_item)
                  && EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_y) )
                  && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_y) )
                {
                    POLY_ITEM_BGN_COE(item_x) = POLY_ITEM_BGN_COE(item_y);
                    POLY_ITEM_BGN_COE_FLAG(item_x) = EC_TRUE;

                    poly_fp_free_item(polyfp_md_id, item_y);
                    poly_fp_free_poly(polyfp_md_id, poly_y_of_item);
                }
            }
        }
        item_x = POLY_ITEM_NEXT(item_x);
    }

    return ( 0 );
}

/**
*
*   here deg(poly(x)) = 2. then find a root such tat poly(x = root) = 0 mod p.
*   let poly = x^2 + a*x + b, then
*       x^2 + a*x +b =0 <==> (x +(a/2))^2 = ((a/2)^2 -b) mod p
*
*
**/
UINT32 sea_fp_poly_squroot(const UINT32 seafp_md_id, const POLY *poly, BIGINT *root)
{
    POLY *poly_t;
    POLY_ITEM *item_t;

    BIGINT *bgn_coe_a;
    BIGINT *bgn_coe_b;
    BIGINT *bgn_t;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_squroot: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == root )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_squroot: root is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_poly_squroot: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY(poly) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_squroot: poly is empty.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    if ( EC_FALSE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_DEG(poly), 2) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_poly_squroot: poly degree != 2.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    poly_fp_alloc_poly(polyfp_md_id, &poly_t);
    POLY_INIT(poly_t);
    poly_fp_to_monic(polyfp_md_id, poly, poly_t);

    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_a);
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_b);
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_t);

    item_t = POLY_FIRST_ITEM(poly);
    if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_ITEM_DEG(item_t), 0) )
    {
        bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_t), bgn_coe_b);

        item_t = POLY_ITEM_NEXT(item_t);
        if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_ITEM_DEG(item_t), 1) )
        {
            bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_t), bgn_coe_a);
        }
        else
        {
            bgn_fp_set_zero(bgnfp_md_id, bgn_coe_a);
        }
    }
    else
    {
        if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_ITEM_DEG(item_t), 1) )
        {
            bgn_fp_set_zero(bgnfp_md_id, bgn_coe_b);
            bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_t), bgn_coe_a);
        }
        else
        {
            bgn_fp_set_zero(bgnfp_md_id, bgn_coe_a);
            bgn_fp_set_zero(bgnfp_md_id, bgn_coe_b);
        }
    }

    /*let bgn_t = 1/2 mod p*/
    bgn_fp_set_word(bgnfp_md_id, bgn_t, 2);
    bgn_fp_inv(bgnfp_md_id, bgn_t, bgn_t);

    /*let bgn_t = a /2 mod p = bgn_coe_a * bgn_t mod p*/
    bgn_fp_mul(bgnfp_md_id, bgn_coe_a, bgn_t, bgn_t);

    /*let bgn_coe_a = (a/2)^2 = bgn_t^2 mod p*/
    bgn_fp_squ(bgnfp_md_id, bgn_t, bgn_coe_a);

    /*let bgn_coe_a = (a/2)^2 - b = bgn_coe_a - bgn_coe_b*/
    bgn_fp_sub(bgnfp_md_id, bgn_coe_a, bgn_coe_b, bgn_coe_a);

    /*let bgn_coe_b is the squroot x0 of x^2 = bgn_coe_a mod p*/
    bgn_fp_squroot(bgnfp_md_id, bgn_coe_a, bgn_coe_b);

    /*then root = x0 - (a/2) = bgn_coe_b - bgn_t*/
    bgn_fp_sub(bgnfp_md_id, bgn_coe_b, bgn_t, root);

    poly_fp_poly_destory(polyfp_md_id, poly_t);
    poly_fp_free_bgn(polyfp_md_id, bgn_coe_a);
    poly_fp_free_bgn(polyfp_md_id, bgn_coe_b);
    poly_fp_free_bgn(polyfp_md_id, bgn_t);

    return ( 0 );
}

UINT32 sea_fp_iso_ec(const UINT32 seafp_md_id,
                         const BIGINT *F,
                         const UINT32 oddsmallprime_pos,
                         ECFP_CURVE *iso_ecfp_curve,
                         BIGINT *isop1)
{
    BIGINT *E4;
    BIGINT *E6;
    BIGINT *E43;
    BIGINT *E66;
    BIGINT *delta;
    BIGINT *J;
    BIGINT *bgn_dxfai;
    BIGINT *bgn_dyfai;
    BIGINT *bgn_dxxfai;
    BIGINT *bgn_dyyfai;
    BIGINT *bgn_dxyfai;
    BIGINT *DJ;
    BIGINT *DF;
    BIGINT *DJJ;
    BIGINT *DFF;
    BIGINT *DFJ;
    BIGINT *tmp1;
    BIGINT *tmp2;
    BIGINT *E0;
    BIGINT *E64;
    BIGINT *Z;
    BIGINT *Zs;
    BIGINT *E0pie;
    BIGINT *isoE4;
    BIGINT *isoE6;
    BIGINT *Fstar;
    BIGINT *isodelta;
    BIGINT *isoJ;
    BIGINT *bgn_t;
    POLY   *poly_dxfai;
    POLY   *poly_dyfai;
    POLY   *poly_dxxfai;
    POLY   *poly_dyyfai;
    POLY   *poly_dxyfai;
    POLY   *DXFAI;
    POLY   *DYFAI;

    UINT32 s;
    UINT32 oddsmallprime;
    UINT32 exp;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;
    ECFP_CURVE  *ecfp_curve;
    BIGINT      *ecfp_j_invar;
    SEAFP_PRIME_TBL  *seafp_oddsmallprime_tbl;
    SEAFP_FAI_TBL    *seafp_fai_tbl;
    BIGINT      *curve_coe_a;
    BIGINT      *curve_coe_b;
    BIGINT      *iso_curve_coe_a;
    BIGINT      *iso_curve_coe_b;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == F )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_ec: F is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == iso_ecfp_curve )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_ec: iso_ecfp_curve is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == isop1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_ec: isop1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_iso_ec: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_curve   = seafp_md->ecfp_curve;
    ecfp_j_invar = seafp_md->ecfp_j_invar;
    seafp_fai_tbl= &(seafp_md->seafp_fai_tbl);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);

    curve_coe_a  = &(ecfp_curve->a);
    curve_coe_b  = &(ecfp_curve->b);

    iso_curve_coe_a  = &(iso_ecfp_curve->a);
    iso_curve_coe_b  = &(iso_ecfp_curve->b);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E4           , LOC_SEAFP_0021);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E6           , LOC_SEAFP_0022);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E43          , LOC_SEAFP_0023);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E66          , LOC_SEAFP_0024);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &delta        , LOC_SEAFP_0025);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &J            , LOC_SEAFP_0026);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_dxfai    , LOC_SEAFP_0027);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_dyfai    , LOC_SEAFP_0028);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_dxxfai   , LOC_SEAFP_0029);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_dyyfai   , LOC_SEAFP_0030);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_dxyfai   , LOC_SEAFP_0031);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &DJ           , LOC_SEAFP_0032);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &DF           , LOC_SEAFP_0033);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &DJJ          , LOC_SEAFP_0034);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &DFF          , LOC_SEAFP_0035);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &DFJ          , LOC_SEAFP_0036);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tmp1         , LOC_SEAFP_0037);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tmp2         , LOC_SEAFP_0038);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E0           , LOC_SEAFP_0039);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E64          , LOC_SEAFP_0040);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Z            , LOC_SEAFP_0041);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Zs           , LOC_SEAFP_0042);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &E0pie        , LOC_SEAFP_0043);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &isoE4        , LOC_SEAFP_0044);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &isoE6        , LOC_SEAFP_0045);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Fstar        , LOC_SEAFP_0046);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &isodelta     , LOC_SEAFP_0047);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &isoJ         , LOC_SEAFP_0048);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_t        , LOC_SEAFP_0049);

    poly_fp_alloc_poly(polyfp_md_id, &DXFAI);
    poly_fp_alloc_poly(polyfp_md_id, &DYFAI);
    poly_fp_alloc_poly(polyfp_md_id, &poly_dxfai);
    poly_fp_alloc_poly(polyfp_md_id, &poly_dyfai);
    poly_fp_alloc_poly(polyfp_md_id, &poly_dxxfai);
    poly_fp_alloc_poly(polyfp_md_id, &poly_dyyfai);
    poly_fp_alloc_poly(polyfp_md_id, &poly_dxyfai);

    POLY_INIT(DXFAI);
    POLY_INIT(DYFAI);
    POLY_INIT(poly_dxfai);
    POLY_INIT(poly_dyfai);
    POLY_INIT(poly_dxxfai);
    POLY_INIT(poly_dyyfai);
    POLY_INIT(poly_dxyfai);

    /*step 1.*/
    /*let EC = x^3 + a*x + b*/
    /*let E4 = -(a/3) mod p and E6 = -(b/2) mod p, then*/
    /*J = (1728 * E4^3)/ (E4^3 - E6^2) mod p*/
    bgn_fp_divs(bgnfp_md_id, curve_coe_a, 3, E4);/*E4 = a/3 mod p*/
    bgn_fp_neg (bgnfp_md_id, E4, E4);            /*E4 = - E4 mod p = -(a/3) mod p*/
    bgn_fp_divs(bgnfp_md_id, curve_coe_b, 2, E6);/*E6 = b/2 mod p*/
    bgn_fp_neg (bgnfp_md_id, E6, E6);            /*E6 = - E6 mod p = -(b/2) mod p*/
    bgn_fp_squ (bgnfp_md_id, E4, E43);           /*E43 = E4 ^ 2 mod p*/
    bgn_fp_mul (bgnfp_md_id, E4, E43, E43 );     /*E43 = E4 * E43 = E4^3 mod p*/
    bgn_fp_squ (bgnfp_md_id, E6, E66 );          /*E66 = E6^2 mod p*/
    bgn_fp_sub (bgnfp_md_id, E43, E66, E66 );    /*E66 = E43 - E66 = E4^3 - E6^2 mod p*/
    bgn_fp_divs(bgnfp_md_id, E66, 1728, delta ); /*delta = E66/1728 mod p*/
    bgn_fp_div (bgnfp_md_id, E43, delta, J );    /*J = E43/delta = E4^3 / delta = (1728 * E4^3)/ (E4^3 - E6^2) mod p*/

    /*step 2.*/
    s = stable[ oddsmallprime_pos ];
    exp = 12 / s;
    bgn_fp_sexp(bgnfp_md_id, F, exp, tmp1);             /*tmp1 = F^exp*/
    oddsmallprime = seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ];
    bgn_fp_set_word(bgnfp_md_id, bgn_t, oddsmallprime); /*bgn_t = oddsmallprime*/
    bgn_fp_sexp(bgnfp_md_id, bgn_t, 12, isodelta);      /*isodelta = (1/oddsmallprime)^12*/
    bgn_fp_div(bgnfp_md_id, tmp1, isodelta, isodelta );/*isodelta = tmp1/isodelta = tmp1 / (L^12)*/
    bgn_fp_mul(bgnfp_md_id, isodelta, delta, isodelta );/*isodelta = isodelta * delta = (F^exp * delta)/ (L^12)*/

#if 0
    sys_log(LOGSTDOUT,"sea_fp_iso_ec: isodelta===:\n");
    print_bigint(LOGSTDOUT, isodelta);
#endif

    /*step 3.*/
    /*let y = j */
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxfai [ oddsmallprime_pos ]), 1, ecfp_j_invar, (seafp_fai_tbl->dxfai_j [ oddsmallprime_pos ]));
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dyfai [ oddsmallprime_pos ]), 1, ecfp_j_invar, (seafp_fai_tbl->dyfai_j [ oddsmallprime_pos ]));
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxxfai[ oddsmallprime_pos ]), 1, ecfp_j_invar, (seafp_fai_tbl->dxxfai_j[ oddsmallprime_pos ]));
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dyyfai[ oddsmallprime_pos ]), 1, ecfp_j_invar, (seafp_fai_tbl->dyyfai_j[ oddsmallprime_pos ]));
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxyfai[ oddsmallprime_pos ]), 1, ecfp_j_invar, (seafp_fai_tbl->dxyfai_j[ oddsmallprime_pos ]));
    /*clean y-items from polynomial*/
    sea_fp_poly_fai_j_clean(seafp_md_id, (seafp_fai_tbl->dxfai_j  [ oddsmallprime_pos ]));
    sea_fp_poly_fai_j_clean(seafp_md_id, (seafp_fai_tbl->dyfai_j  [ oddsmallprime_pos ]));
    sea_fp_poly_fai_j_clean(seafp_md_id, (seafp_fai_tbl->dxxfai_j [ oddsmallprime_pos ]));
    sea_fp_poly_fai_j_clean(seafp_md_id, (seafp_fai_tbl->dyyfai_j [ oddsmallprime_pos ]));
    sea_fp_poly_fai_j_clean(seafp_md_id, (seafp_fai_tbl->dxyfai_j [ oddsmallprime_pos ]));
    /* let x = F*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxfai_j [ oddsmallprime_pos ]), 0, F, poly_dxfai );/*fai_x  = dxfai(x = F, y = j)*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dyfai_j [ oddsmallprime_pos ]), 0, F, poly_dyfai );/*fai_y  = dyfai(x = F, y = j)*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxxfai_j[ oddsmallprime_pos ]), 0, F, poly_dxxfai);/*fai_xx = dxxfai(x = F, y = j)*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dyyfai_j[ oddsmallprime_pos ]), 0, F, poly_dyyfai);/*fai_yy = dyyfai(x = F, y = j)*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxyfai_j[ oddsmallprime_pos ]), 0, F, poly_dxyfai);/*fai_xy = dxyfai(x = F, y = j)*/

#if 0
    bgn_dxfai  = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dxfai ));
    bgn_dyfai  = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dyfai ));
    bgn_dxxfai = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dxxfai));
    bgn_dyyfai = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dyyfai));
    bgn_dxyfai = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dxyfai));
#endif
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dxfai , bgn_dxfai );
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dyfai , bgn_dyfai );
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dxxfai, bgn_dxxfai);
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dyyfai, bgn_dyyfai);
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dxyfai, bgn_dxyfai);
#if 0
    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dxfai:\n");
    print_bigint(LOGSTDOUT, bgn_dxfai);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dyfai:\n");
    print_bigint(LOGSTDOUT, bgn_dyfai);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dxxfai:\n");
    print_bigint(LOGSTDOUT, bgn_dxxfai);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dyyfai:\n");
    print_bigint(LOGSTDOUT, bgn_dyyfai);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dxyfai:\n");
    print_bigint(LOGSTDOUT, bgn_dxyfai);
#endif
    /*step 4.*/
    /*compute DJ, DF, DJJ, DFF, DFJ*/
    bgn_fp_mul(bgnfp_md_id, bgn_dyfai, J, DJ );      /*DJ = dyfai(x = F, y = j) * J*/
    bgn_fp_mul(bgnfp_md_id, bgn_dxfai, F, DF );      /*DF = dxfai(x = F, y = j) * F*/
    bgn_fp_squ(bgnfp_md_id, J, tmp1 );               /*tmp1 = J * tmp1 = J * F^exp*/
    bgn_fp_mul(bgnfp_md_id, tmp1, bgn_dyyfai, tmp1 );/*tmp1 = J * F^exp * dyyfai(x = F, y = j)*/
    bgn_fp_add(bgnfp_md_id, tmp1, DJ, DJJ );         /*DJJ = J * F^exp * dyyfai(x = F, y = j) + dyfai(x = F, y = j) * J*/
    bgn_fp_squ(bgnfp_md_id, F, tmp1 );               /*tmp1 = F^2*/
    bgn_fp_mul(bgnfp_md_id, tmp1, bgn_dxxfai, tmp1 );/*tmp1 = F^2 *dxxfai(x = F, y = j)*/
    bgn_fp_add(bgnfp_md_id, tmp1, DF, DFF );         /*DFF = F^2 *dxxfai(x = F, y = j) + dxfai(x = F, y = j) * F*/
    bgn_fp_mul(bgnfp_md_id, F, J, tmp1 );            /*tmp1 = F * J*/
    bgn_fp_mul(bgnfp_md_id, tmp1, bgn_dxyfai, DFJ ); /*DFJ = F * J * dxyfai(x = F, y = j)*/

    /*step 5.*/
    bgn_fp_div(bgnfp_md_id, DF, DJ, E0 );            /*E0 = DF / DJ*/
    bgn_fp_div(bgnfp_md_id, E6, E4, E64 );           /*E64 = E6 / E4*/
    bgn_fp_div(bgnfp_md_id, E64,E0, Z );             /*Z = E64 / E0*/
    bgn_fp_smul(bgnfp_md_id, Z, exp, Zs);            /*Zs = exp * (E64 / E0)*/

    /*step 6.*/
    bgn_fp_mul(bgnfp_md_id, Z, DFF, DFF );           /*DFF = Z * DFF*/
    bgn_fp_mul(bgnfp_md_id, E0, DJJ, DJJ );          /*DJJ = E0 * DJJ*/
    bgn_fp_smul(bgnfp_md_id,DFJ, 2, DFJ );           /*DFJ = 2 * DFJ*/
    bgn_fp_sub(bgnfp_md_id, DJJ, DFJ, DJJ );         /*DJJ = DJJ - DFJ*/
    bgn_fp_mul(bgnfp_md_id, DJJ, E64, DJJ );         /*DJJ = DJJ * E64*/
    bgn_fp_add(bgnfp_md_id, DJJ, DFF, DJJ );         /*DJJ = DJJ + DFF*/
    bgn_fp_div(bgnfp_md_id, DJJ, DJ, E0pie );        /*E0pie = DJJ / DJ*/

    /*step 7.*/
    bgn_fp_div(bgnfp_md_id, E0pie, E0, E0pie );      /*E0pie = E0pie / E0*/
    bgn_fp_smul(bgnfp_md_id, E0pie, 12, E0pie );     /*E0pie = E0pie * 12*/
    bgn_fp_div(bgnfp_md_id, E4, E64, isoE4 );        /*isoE4 = E4 / E64*/
    bgn_fp_smul(bgnfp_md_id, isoE4, 6, isoE4 );      /*isoE4 = isoE4 * 6*/
    bgn_fp_add(bgnfp_md_id, isoE4, E0pie, isoE4 );   /*isoE4 = isoE4 + E0pie*/
    bgn_fp_smul(bgnfp_md_id, E64, 4, E64 );          /*E64 = E64 * 4*/
    bgn_fp_sub(bgnfp_md_id, isoE4, E64, isoE4 );     /*isoE4 = isoE4 - E64*/
    bgn_fp_add(bgnfp_md_id, isoE4, Zs, isoE4 );      /*isoE4 = isoE4 + Zs*/
    bgn_fp_mul(bgnfp_md_id, isoE4, Zs, isoE4 );      /*isoE4 = isoE4 * Zs*/
    bgn_fp_add(bgnfp_md_id, isoE4, E4, isoE4 );      /*isoE4 = isoE4 + E4*/
    bgn_fp_set_word(bgnfp_md_id, tmp1, oddsmallprime); /*tmp1 = oddsmallprime*/
    bgn_fp_squ(bgnfp_md_id, tmp1, tmp2 );           /*tmp2=oddsmallprime^2*/
    bgn_fp_div(bgnfp_md_id, isoE4, tmp2, isoE4 );    /*isoE4 = isoE4 / tmp2 */

#if 0
    sys_log(LOGSTDOUT,"sea_fp_iso_ec: isoE4:\n");
    print_bigint(LOGSTDOUT, isoE4);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: isodelta:\n");
    print_bigint(LOGSTDOUT, isodelta);
#endif

    /*step 8.*/
    bgn_fp_sexp(bgnfp_md_id, tmp1, s, tmp2 );        /*tmp2 = oddsmallprime^s */
    bgn_fp_div(bgnfp_md_id, tmp2, F, Fstar );        /*Fstar = tmp2 / F*/
    bgn_fp_squ(bgnfp_md_id, isoE4, isoJ );           /*isoJ = isoE4 ^ 2*/
    bgn_fp_mul(bgnfp_md_id, isoE4, isoJ, isoJ );     /*isoJ = isoE4 * isoJ*/
    bgn_fp_div(bgnfp_md_id, isoJ, isodelta, isoJ );  /*isoJ = isoJ / isodelta*/

#if 0
    sys_log(LOGSTDOUT,"sea_fp_iso_ec: isoJ:\n");
    print_bigint(LOGSTDOUT, isoJ);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: Fstar:\n");
    print_bigint(LOGSTDOUT, Fstar);
#endif


    /*step 9.*/
    /* let y = isoJ*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dxfai [ oddsmallprime_pos ]), 1, isoJ, DXFAI);/*DXFAI = DXFAI(x, y = isoJ)*/
    poly_fp_eval(polyfp_md_id, (seafp_fai_tbl->dyfai [ oddsmallprime_pos ]), 1, isoJ, DYFAI);/*DYFAI = DYFAI(x, y = isoJ)*/
    /*clean y-items from polynomial*/
    sea_fp_poly_fai_j_clean(seafp_md_id, DXFAI);
    sea_fp_poly_fai_j_clean(seafp_md_id, DYFAI);
    /* let x = Fstar*/
    poly_fp_poly_clean(polyfp_md_id, poly_dxfai);
    poly_fp_poly_clean(polyfp_md_id, poly_dyfai);
    poly_fp_eval(polyfp_md_id, DXFAI, 0, Fstar, poly_dxfai );/*fai_x  = dxfai(x = Fstar, y = isoJ)*/
    poly_fp_eval(polyfp_md_id, DYFAI, 0, Fstar, poly_dyfai );/*fai_y  = dyfai(x = Fstar, y = isoJ)*/
#if 0
    bgn_dxfai  = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dxfai ));
    bgn_dyfai  = POLY_ITEM_BGN_COE(POLY_FIRST_ITEM(poly_dyfai ));
#endif

    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dxfai , bgn_dxfai );
    sea_fp_conv_const_poly_to_bgn(seafp_md_id, poly_dyfai , bgn_dyfai );

#if 0
    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dxfai:\n");
    print_bigint(LOGSTDOUT, bgn_dxfai);

    sys_log(LOGSTDOUT,"sea_fp_iso_ec: bgn_dyfai:\n");
    print_bigint(LOGSTDOUT, bgn_dyfai);
#endif


    bgn_fp_mul(bgnfp_md_id, bgn_dxfai, Fstar, bgn_dxfai );  /*bgn_dxfai = bgn_dxfai * Fstar*/
    bgn_fp_mul(bgnfp_md_id, bgn_dxfai, Z, bgn_dxfai );      /*bgn_dxfai = bgn_dxfai * Z*/
    bgn_fp_mul(bgnfp_md_id, bgn_dxfai, isoE4, bgn_dxfai );  /*bgn_dxfai = bgn_dxfai * isoE4*/
    bgn_fp_mul(bgnfp_md_id, bgn_dyfai, isoJ, bgn_dyfai );   /*bgn_dyfai = bgn_dyfai * isoJ*/
    bgn_fp_mul(bgnfp_md_id, bgn_dyfai, tmp1, bgn_dyfai );   /*bgn_dyfai = bgn_dyfai * tmp1*/
    bgn_fp_div(bgnfp_md_id, bgn_dxfai, bgn_dyfai, isoE6 );  /*isoE6 = bgn_dxfai / bgn_dyfai*/
    bgn_fp_neg(bgnfp_md_id, isoE6, isoE6 );                 /*isoE6 = -isoE6*/

    /*step 10.*/
    bgn_fp_squ(bgnfp_md_id, tmp1, tmp1 );                   /*tmp1 = tmp1^2 = oddsmallprime^2*/
    bgn_fp_squ(bgnfp_md_id, tmp1, tmp2 );                   /*tmp2 = tmp1^2 = oddsmallprime^4*/
    bgn_fp_mul(bgnfp_md_id, tmp1, tmp2, tmp1 );             /*tmp1 = tmp1 * tmp2 = oddsmallprime^6*/
    bgn_fp_mul(bgnfp_md_id, isoE4, tmp2, isoE4 );           /*isoE4 = isoE4 * tmp2*/
    bgn_fp_smul(bgnfp_md_id, isoE4, 3, isoE4 );             /*isoE4 = isoE4 * 3*/
    bgn_fp_neg(bgnfp_md_id, isoE4, iso_curve_coe_a );       /*iso_curve_coe_a = -isoE4*/
    bgn_fp_mul(bgnfp_md_id, isoE6, tmp1, isoE6 );           /*isoE6 = isoE6 * tmp1 = isoE6 * oddsmallprime^6*/
    bgn_fp_smul(bgnfp_md_id, isoE6, 2, isoE6 );             /*isoE6 = isoE6 * 2*/
    bgn_fp_neg(bgnfp_md_id, isoE6, iso_curve_coe_b );       /*iso_curve_coe_b = isoE6*/
    bgn_fp_smul(bgnfp_md_id, Z, 6 * oddsmallprime, tmp1 );  /*tmp1 = Z *(6*oddsmallprime)*/
    bgn_fp_divs(bgnfp_md_id, tmp1, s, isop1 );              /*isop1 = tmp1 / tmp2 = (Z *(6*oddsmallprime)) / s*/

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E4           , LOC_SEAFP_0050);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E6           , LOC_SEAFP_0051);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E43          , LOC_SEAFP_0052);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E66          , LOC_SEAFP_0053);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, delta        , LOC_SEAFP_0054);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, J            , LOC_SEAFP_0055);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_dxfai    , LOC_SEAFP_0056);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_dyfai    , LOC_SEAFP_0057);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_dxxfai   , LOC_SEAFP_0058);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_dyyfai   , LOC_SEAFP_0059);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_dxyfai   , LOC_SEAFP_0060);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, DJ           , LOC_SEAFP_0061);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, DF           , LOC_SEAFP_0062);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, DJJ          , LOC_SEAFP_0063);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, DFF          , LOC_SEAFP_0064);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, DFJ          , LOC_SEAFP_0065);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tmp1         , LOC_SEAFP_0066);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tmp2         , LOC_SEAFP_0067);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E0           , LOC_SEAFP_0068);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E64          , LOC_SEAFP_0069);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Z            , LOC_SEAFP_0070);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Zs           , LOC_SEAFP_0071);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, E0pie        , LOC_SEAFP_0072);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, isoE4        , LOC_SEAFP_0073);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, isoE6        , LOC_SEAFP_0074);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Fstar        , LOC_SEAFP_0075);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, isodelta     , LOC_SEAFP_0076);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, isoJ         , LOC_SEAFP_0077);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_t        , LOC_SEAFP_0078);

    poly_fp_poly_destory(polyfp_md_id, DXFAI);
    poly_fp_poly_destory(polyfp_md_id, DYFAI);
    poly_fp_poly_destory(polyfp_md_id, poly_dxfai);
    poly_fp_poly_destory(polyfp_md_id, poly_dyfai);
    poly_fp_poly_destory(polyfp_md_id, poly_dxxfai);
    poly_fp_poly_destory(polyfp_md_id, poly_dyyfai);
    poly_fp_poly_destory(polyfp_md_id, poly_dxyfai);

    return ( 0 );
}

static UINT32 sea_fp_conv_cluster_to_poly(const UINT32 seafp_md_id,
                         const SEAFP_BGN_TBL *seafp_bgn_tbl,
                         const UINT32 num_bgn,
                         POLY *poly )
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;

    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    ECFP_CURVE  *ecfp_curve;

    SEAFP_MD    *seafp_md;

    UINT32 index;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == seafp_bgn_tbl )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_cluster_to_poly: seafp_bgn_tbl is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_cluster_to_poly: poly is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_conv_cluster_to_poly: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_curve   = seafp_md->ecfp_curve;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    for ( index = 0; index < num_bgn; index ++ )
    {
        if ( EC_FALSE == bgn_fp_is_zero(bgnfp_md_id, seafp_bgn_tbl->bgn[ index ]) )
        {
            poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
            bgn_fp_clone(bgnfp_md_id, seafp_bgn_tbl->bgn[ index ], bgn_coe_of_item);

            poly_fp_alloc_item(polyfp_md_id, &item);
            POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), index);
            POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
            POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

            POLY_ADD_ITEM_TAIL(poly, item);
        }
    }

    return ( 0 );
}

/**
*
*
*   poly_const must be a bgn constant,i.e.,
*       poly_const = n
*   where n is a BIGINT.
*
**/
static UINT32 sea_fp_conv_const_poly_to_bgn(const UINT32 seafp_md_id,const POLY *poly_const,BIGINT *bgn)
{
    POLY_ITEM *item;

    UINT32  bgnfp_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly_const )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_const_poly_to_bgn: poly_const is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == bgn )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_const_poly_to_bgn: bgn is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_conv_const_poly_to_bgn: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnfp_md_id    = seafp_md->bgnfp_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY(poly_const) )
    {
        bgn_fp_set_zero(bgnfp_md_id, bgn);
        return ( 0 );
    }

    if ( POLY_FIRST_ITEM(poly_const) != POLY_LAST_ITEM(poly_const) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_const_poly_to_bgn: poly_const has more than one item.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    item = POLY_FIRST_ITEM(poly_const);
    if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item))
    {
        sys_log(LOGSTDOUT,"error:sea_fp_conv_const_poly_to_bgn: poly_const is not a BIGINT constant.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item), bgn);
    return ( 0 );
}

UINT32 sea_fp_iso_h(const UINT32 seafp_md_id,
                         const ECFP_CURVE *iso_ecfp_curve,
                         const BIGINT *isop1,
                         const UINT32 lpos,
                         POLY *iso_h )
{
    SEAFP_BGN_TBL buf_1[SEAFP_MAX_SMALL_PRIME/2];
    SEAFP_BGN_TBL buf_2[SEAFP_MAX_SMALL_PRIME/2];

    SEAFP_BGN_TBL buf_3;
    SEAFP_BGN_TBL buf_4;
    SEAFP_BGN_TBL buf_5;
    SEAFP_BGN_TBL buf_6;

    SEAFP_BGN_TBL *P;
    SEAFP_BGN_TBL *Q;
    SEAFP_BGN_TBL *g;
    SEAFP_BGN_TBL *h;

    SEAFP_PRIME_TBL *seafp_oddsmallprime_tbl;
    SEAFP_BGN_TBL   *seafp_prod_tbl;

    SEAFP_BGN_TBL *c;
    SEAFP_BGN_TBL *iso_c;

    BIGINT *bgn_t;

    const BIGINT *curve_coe_a;
    const BIGINT *curve_coe_b;

    const BIGINT *iso_curve_coe_a;
    const BIGINT *iso_curve_coe_b;

    UINT32 oddsmallprime;
    UINT32 d;

    UINT32 idx_i;
    UINT32 idx_j;
    UINT32 idx_k;
    UINT32 idx_m;

    UINT32 idx_cluster;
    UINT32 idx_bgn;

    UINT32 bgn_cluster_num;
    UINT32 bgn_cluster_size;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;

    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    ECFP_CURVE  *ecfp_curve;

    SEAFP_MD    *seafp_md;


#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_ecfp_curve )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_h: iso_ecfp_curve is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == isop1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_h: isop1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_iso_h: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_iso_h: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    P = buf_1;
    Q = buf_2;

    g = &buf_3;
    h = &buf_4;
    c = &buf_5;
    iso_c = &buf_6;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id     = seafp_md->bgnz_md_id;
    bgnfp_md_id    = seafp_md->bgnfp_md_id;
    ecfp_md_id     = seafp_md->ecfp_md_id;
    polyfp_md_id   = seafp_md->polyfp_md_id;
    ecfp_curve     = seafp_md->ecfp_curve;
    seafp_prod_tbl = &(seafp_md->seafp_prod_tbl);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);

    curve_coe_a = &(ecfp_curve->a);
    curve_coe_b = &(ecfp_curve->b);

    iso_curve_coe_a = &(iso_ecfp_curve->a);
    iso_curve_coe_b = &(iso_ecfp_curve->b);

    bgn_cluster_size = SEAFP_MAX_SMALL_PRIME;
    bgn_cluster_num  = SEAFP_MAX_SMALL_PRIME/2;

    for ( idx_cluster = 0; idx_cluster < bgn_cluster_num; idx_cluster ++ )
    {
        for ( idx_bgn = 0; idx_bgn < bgn_cluster_size; idx_bgn ++ )
        {
            alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(P[ idx_cluster ].bgn[idx_bgn]), LOC_SEAFP_0079);
            alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(Q[ idx_cluster ].bgn[idx_bgn]), LOC_SEAFP_0080);
        }
    }

    for ( idx_bgn = 0; idx_bgn < bgn_cluster_size; idx_bgn ++ )
    {
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(g->bgn[ idx_bgn ]), LOC_SEAFP_0081);
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(h->bgn[ idx_bgn ]), LOC_SEAFP_0082);
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(c->bgn[ idx_bgn ]), LOC_SEAFP_0083);
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(iso_c->bgn[ idx_bgn ]), LOC_SEAFP_0084);
    }
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_t, LOC_SEAFP_0085);

    oddsmallprime = seafp_oddsmallprime_tbl->oddsmallprime[ lpos ];
    d = ( oddsmallprime - 1 ) / 2;

    /*step 1. compute c,iso_c .NOTE: p[1] <---> c .*/
    /*let EC = x^3 + a*x + b*/
    bgn_fp_set_zero(bgnfp_md_id, c->bgn[ 0 ]);
    bgn_fp_divs(bgnfp_md_id, curve_coe_a, 5, c->bgn[ 1 ] ); /*c->bgn[1] = a/5*/
    bgn_fp_neg(bgnfp_md_id, c->bgn[ 1 ], c->bgn[ 1 ] );     /*c->bgn[1] = -(a/5)*/
    bgn_fp_divs(bgnfp_md_id, curve_coe_b, 7, c->bgn[ 2 ] ); /*c->bgn[2] = b/7*/
    bgn_fp_neg(bgnfp_md_id, c->bgn[ 2 ], c->bgn[ 2 ] );     /*c->bgn[2] = -(b/7)*/

    /*let EC = x^3 + iso_a*x + iso_b*/
    bgn_fp_set_zero(bgnfp_md_id, iso_c->bgn[ 0 ]);
    bgn_fp_divs(bgnfp_md_id, iso_curve_coe_a, 5, iso_c->bgn[ 1 ] ); /*iso_c->bgn[1] = iso_a/5*/
    bgn_fp_neg(bgnfp_md_id, iso_c->bgn[ 1 ], iso_c->bgn[ 1 ] );     /*iso_c->bgn[1] = -(iso_a/5)*/
    bgn_fp_divs(bgnfp_md_id, iso_curve_coe_b, 7, iso_c->bgn[ 2 ] ); /*iso_c->bgn[2] = iso_b/7*/
    bgn_fp_neg(bgnfp_md_id, iso_c->bgn[ 2 ], iso_c->bgn[ 2 ] );     /*iso_c->bgn[2] = -(iso_b/7)*/

    for ( idx_k = 3; idx_k <= d; idx_k ++ )
    {
        bgn_fp_set_zero(bgnfp_md_id, c->bgn[ idx_k ]);
        bgn_fp_set_zero(bgnfp_md_id, iso_c->bgn[ idx_k ]);
        for ( idx_i = 1, idx_j = idx_k - 2; idx_i < idx_j; idx_i ++, idx_j -- )
        {
            bgn_fp_mul(bgnfp_md_id, c->bgn[ idx_i ], c->bgn[ idx_j ], bgn_t );
            bgn_fp_add(bgnfp_md_id, bgn_t, c->bgn[ idx_k ], c->bgn[ idx_k ] );

            bgn_fp_mul(bgnfp_md_id, iso_c->bgn[ idx_i ], iso_c->bgn[ idx_j ], bgn_t );
            bgn_fp_add(bgnfp_md_id, bgn_t, iso_c->bgn[ idx_k ], iso_c->bgn[ idx_k ] );
        }
        bgn_fp_add(bgnfp_md_id, c->bgn[ idx_k ], c->bgn[ idx_k ], c->bgn[ idx_k ]);
        bgn_fp_add(bgnfp_md_id, iso_c->bgn[ idx_k ], iso_c->bgn[idx_k ], iso_c->bgn[ idx_k ]);
        if ( idx_k & 1 )
        {
            bgn_fp_squ(bgnfp_md_id, c->bgn[ idx_i ], bgn_t );
            bgn_fp_add(bgnfp_md_id, bgn_t, c->bgn[ idx_k ], c->bgn[ idx_k ] );
            bgn_fp_squ(bgnfp_md_id, iso_c->bgn[ idx_i ], bgn_t );
            bgn_fp_add(bgnfp_md_id, bgn_t, iso_c->bgn[ idx_k ], iso_c->bgn[ idx_k ] );
        }
        /*bgn_t = 3 / ( (idx_k - 2) *(2 * idx_k + 3))*/
        bgn_fp_set_word(bgnfp_md_id, bgn_t, (idx_k - 2) *(2 * idx_k + 3));
        bgn_fp_sdiv(bgnfp_md_id, 3, bgn_t, bgn_t);

        bgn_fp_mul(bgnfp_md_id, bgn_t, c->bgn[ idx_k ], c->bgn[ idx_k ] );
        bgn_fp_mul(bgnfp_md_id, bgn_t, iso_c->bgn[ idx_k ], iso_c->bgn[ idx_k ] );
    }


    /*step 2. compute P1,P2,...,Pd*/
    /*P[1] = 1 + 0 * x + c[1] * x^2 + c[2] * x^3+....*/
    bgn_fp_set_word( bgnfp_md_id, P[ 1 ].bgn[ 0 ], 1);
    bgn_fp_set_word( bgnfp_md_id, P[ 1 ].bgn[ 1 ], 0);

    for ( idx_i = 1; idx_i <= d; idx_i ++ )
    {
        bgn_fp_clone(bgnfp_md_id, c->bgn[ idx_i ], P[ 1 ].bgn[ idx_i + 1 ]);
    }
    for ( idx_k = 2; idx_k <= d; idx_k ++ )
    {
        bgn_fp_set_word( bgnfp_md_id, P[ idx_k ].bgn[ 0 ], 1);
        for ( idx_m = 1; idx_m <= d + 1; idx_m ++ )
        {
            bgn_fp_set_word( bgnfp_md_id, P[ idx_k ].bgn[ idx_m ], 0);
            for ( idx_i = 0; idx_i <= idx_m; idx_i ++ )
            {
                bgn_fp_mul(bgnfp_md_id, P[ 1 ].bgn[ idx_i ], P[ idx_k - 1 ].bgn[ idx_m - idx_i ], bgn_t);
                bgn_fp_add(bgnfp_md_id, bgn_t, P[ idx_k ].bgn[ idx_m ], P[ idx_k ].bgn[ idx_m ]);
            }
        }
    }

    /* first compute Q1*/
    bgn_fp_set_word( bgnfp_md_id, Q[ 1 ].bgn[ 0 ], 0);
    bgn_fp_neg(bgnfp_md_id, isop1, Q[ 1 ].bgn[ 1 ]);
    for ( idx_k = 1; idx_k < d; idx_k ++ )
    {
        /*Q[ 1 ].e[ k + 1 ]  = (oddsmallprime * c[k] - iso_c[k]) / ((2 * k + 1) *(2 * k + 2))*/
        bgn_fp_smul(bgnfp_md_id, c->bgn[ idx_k ], oddsmallprime, bgn_t);
        bgn_fp_sub(bgnfp_md_id, bgn_t, iso_c->bgn[ idx_k ], bgn_t);

        bgn_fp_divs(bgnfp_md_id, bgn_t, (2 * idx_k + 1) *(2 * idx_k + 2), Q[ 1 ].bgn[ idx_k + 1 ]);
    }

    /*next compute Q2,...,Qd*/
    for ( idx_k = 2;idx_k <= d;idx_k++ )
    {
        for ( idx_m = idx_k;idx_m <= d;idx_m++ )
        {
            bgn_fp_set_word( bgnfp_md_id, Q[ idx_k ].bgn[ idx_m ], 0);
            for ( idx_i = 1; idx_i <= idx_m - idx_k + 1; idx_i ++ )
            {
                bgn_fp_mul(bgnfp_md_id, Q[ 1 ].bgn[ idx_i ], Q[ idx_k - 1 ].bgn[ idx_m - idx_i ], bgn_t);
                bgn_fp_add(bgnfp_md_id, bgn_t, Q[ idx_k ].bgn[ idx_m ], Q[ idx_k ].bgn[ idx_m ] );
            }
        }
    }

    /*step 3.modify Qk as Qk <=== Qk/(k!) */
    for ( idx_k = 2; idx_k <= d; idx_k ++ )
    {
        for ( idx_m = idx_k; idx_m <= d; idx_m ++ )
        {
            bgn_fp_div(bgnfp_md_id, Q[ idx_k ].bgn[ idx_m ], seafp_prod_tbl->bgn[ idx_k ], Q[ idx_k ].bgn[ idx_m ] );
        }
    }

    /*step 4.compute g = 1+Q1+...+Qd.*/
    bgn_fp_set_word( bgnfp_md_id, g->bgn[ 0 ], 1);
    for ( idx_k = 1;idx_k <= d;idx_k++ )
    {
        bgn_fp_set_word( bgnfp_md_id, g->bgn[ idx_k ], 0);
        for ( idx_j = 1; idx_j <= idx_k; idx_j ++ )
        {
            bgn_fp_add(bgnfp_md_id, g->bgn[ idx_k ], Q[ idx_j ].bgn[ idx_k ], g->bgn[ idx_k ] );
        }
    }

    /* step 5.compute h */
    bgn_fp_set_word( bgnfp_md_id, h->bgn[ d ], 1);
    for ( idx_j = d, idx_m = 1; idx_j > 0; idx_j --, idx_m ++ )
    {
        for ( idx_i = 0, idx_k = d - idx_j; idx_i <= idx_j; idx_i ++, idx_k ++ )
        {
            bgn_fp_mul(bgnfp_md_id, h->bgn[ idx_j ], P[ idx_j ].bgn[ idx_i ], bgn_t);
            bgn_fp_sub(bgnfp_md_id, g->bgn[ idx_k ], bgn_t, g->bgn[ idx_k ] );
        }
        bgn_fp_clone(bgnfp_md_id, g->bgn[ idx_m ], h->bgn[ idx_j - 1 ]);
    }

    sea_fp_conv_cluster_to_poly(seafp_md_id, h, (d + 1), iso_h);

    for ( idx_cluster = 0; idx_cluster < bgn_cluster_num; idx_cluster ++ )
    {
        for ( idx_bgn = 0; idx_bgn < bgn_cluster_size; idx_bgn ++ )
        {
            free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (P[ idx_cluster ].bgn[idx_bgn]), LOC_SEAFP_0086);
            free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (Q[ idx_cluster ].bgn[idx_bgn]), LOC_SEAFP_0087);
        }
    }

    for ( idx_bgn = 0; idx_bgn < bgn_cluster_size; idx_bgn ++ )
    {
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (g->bgn[ idx_bgn ]), LOC_SEAFP_0088);
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (h->bgn[ idx_bgn ]), LOC_SEAFP_0089);
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (c->bgn[ idx_bgn ]), LOC_SEAFP_0090);
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (iso_c->bgn[ idx_bgn ]), LOC_SEAFP_0091);
    }
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_t, LOC_SEAFP_0092);

    return ( 0 );

}

/**
*
*
*   KUSAI0(x) = 0
*
*
**/
static UINT32 sea_fp_Elkies_set_kusai0(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai0)
{
    UINT32 polyfp_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai0: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_kusai0 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai0: poly_kusai0 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_set_kusai0: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/


    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly_kusai0) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_kusai0);
    }

    return ( 0 );
}

/**
*
*
*   KUSAI1(x) = 1
*
*
**/
static UINT32 sea_fp_Elkies_set_kusai1(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai1)
{
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai1: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_kusai1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai1: poly_kusai1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_set_kusai1: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly_kusai1) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_kusai1);
    }

    poly_fp_set_word(polyfp_md_id, 1, poly_kusai1);

    return ( 0 );
}

/**
*
*
*   KUSAI2(x) = 2
*
*
**/
static UINT32 sea_fp_Elkies_set_kusai2(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai2)
{
    UINT32 polyfp_md_id;

    SEAFP_MD    *seafp_md;


#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai2: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_kusai2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai2: poly_kusai2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_set_kusai2: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    polyfp_md_id = seafp_md->polyfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly_kusai2) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_kusai2);
    }

    poly_fp_set_word(polyfp_md_id, 2, poly_kusai2);

    return ( 0 );
}
/**
*
*
*   KUSAI3(x) = 3 * x^4 + (6*a) *x^2 + (12*b)*x - a^2
*   where EC = x^3 + a*x + b
*
**/
static UINT32 sea_fp_Elkies_set_kusai3(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai3)
{
    POLY_ITEM *item;
    BIGINT    *bgn_coe_of_item;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    ECFP_CURVE  *ecfp_curve;

    SEAFP_MD    *seafp_md;

    BIGINT *curve_coe_a;
    BIGINT *curve_coe_b;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai3: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_kusai3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai3: poly_kusai3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_set_kusai3: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_curve   = seafp_md->ecfp_curve;

    curve_coe_a = &(ecfp_curve->a);
    curve_coe_b = &(ecfp_curve->b);

    if ( EC_FALSE == POLY_IS_EMPTY(poly_kusai3) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_kusai3);
    }

    /*insert 3*x^4 to poly_kusai3*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_set_word(bgnfp_md_id, bgn_coe_of_item, 3);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 4);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai3, item);

    /*insert (6*a) *x^3 to poly_kusai3*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_clone(bgnfp_md_id, curve_coe_a, bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, bgn_coe_of_item, 6, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 2);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai3, item);

    /*insert (12*b)*x to poly_kusai3*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_clone(bgnfp_md_id, curve_coe_b, bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, bgn_coe_of_item, 12, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 1);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai3, item);

    /*insert -(a^2) to poly_kusai3*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_squ(bgnfp_md_id, curve_coe_a, bgn_coe_of_item);
    bgn_fp_neg(bgnfp_md_id, bgn_coe_of_item, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 0);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai3, item);

    /*poly_kusai3 = poly_kusai3 mod iso_h*/
    poly_fp_mod_poly(polyfp_md_id, poly_kusai3, iso_h, poly_kusai3);

    return ( 0 );
}

/**
*
*
*   KUSAI4(x) = 4 * (X^6 + 5*a*X^4 + 20*b*X^3 - 5*a^2*X^2 - 4*a*b*X - 8*b^2 - a^3)
*             = 4*X^6 + 20*a*X^4 + 80*b*X^3 - 20*a^2*X^2 - 16*a*b*X - 32*b^2 - 4*a^3
*   where EC = x^3 + a*x + b
*
**/
static UINT32 sea_fp_Elkies_set_kusai4(const UINT32 seafp_md_id, const POLY *iso_h, POLY *poly_kusai4)
{
    POLY_ITEM *item;
    BIGINT    *bgn_coe_of_item;
    BIGINT    *bgn_t;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    ECFP_CURVE  *ecfp_curve;

    SEAFP_MD    *seafp_md;

    const BIGINT *curve_coe_a;
    const BIGINT *curve_coe_b;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai4: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_kusai4 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_set_kusai4: poly_kusai4 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_set_kusai4: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_curve   = seafp_md->ecfp_curve;

    curve_coe_a = &(ecfp_curve->a);
    curve_coe_b = &(ecfp_curve->b);

    if ( EC_FALSE == POLY_IS_EMPTY(poly_kusai4) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_kusai4);
    }

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_t, LOC_SEAFP_0093);

    /*insert 4*X^6 to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_set_word(bgnfp_md_id, bgn_coe_of_item, 4);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 6);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*insert 20*a*X^4 to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, curve_coe_a, 20, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 4);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*insert 80*b*X^3 to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, curve_coe_b, 80, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 3);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*insert - 20*a^2*X^2 to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_squ(bgnfp_md_id, curve_coe_a, bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, bgn_coe_of_item, 20, bgn_coe_of_item);
    bgn_fp_neg(bgnfp_md_id, bgn_coe_of_item, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 2);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*insert - 16*a*b*X to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_mul(bgnfp_md_id, curve_coe_a, curve_coe_b, bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, bgn_coe_of_item, 16, bgn_coe_of_item);
    bgn_fp_neg(bgnfp_md_id, bgn_coe_of_item, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 1);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*insert - 32*b^2 - 4*a^3 to poly_kusai4*/
    poly_fp_alloc_bgn(polyfp_md_id, &bgn_coe_of_item);
    bgn_fp_sexp(bgnfp_md_id, curve_coe_a, 3, bgn_coe_of_item);
    bgn_fp_smul(bgnfp_md_id, bgn_coe_of_item, 4, bgn_coe_of_item);

    bgn_fp_sexp(bgnfp_md_id, curve_coe_b, 2, bgn_t);
    bgn_fp_smul(bgnfp_md_id, bgn_t, 32, bgn_t);

    bgn_fp_add(bgnfp_md_id, bgn_coe_of_item, bgn_t, bgn_coe_of_item);

    bgn_fp_neg(bgnfp_md_id, bgn_coe_of_item, bgn_coe_of_item);

    poly_fp_alloc_item(polyfp_md_id, &item);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), 0);
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly_kusai4, item);

    /*poly_kusai4 = poly_kusai4 mod iso_h*/
    poly_fp_mod_poly(polyfp_md_id, poly_kusai4, iso_h, poly_kusai4);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_t, LOC_SEAFP_0094);

    return ( 0 );
}

UINT32 sea_fp_Elkies_kusais(const UINT32 seafp_md_id,
                         const POLY *iso_h,
                         const UINT32 m)
{
    POLY *poly_ec_t;
    POLY *poly_t;
    POLY *poly_mid;
    POLY *poly_D;
    POLY *poly_E;
    BIGINT *bgn_t;
    POLY **kusai;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;

    SEAFP_KUSAI_TBL *seafp_kusai_tbl;

    SEAFP_MD    *seafp_md;

    UINT32 index;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == iso_h )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_kusais: iso_h is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_kusais: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    seafp_kusai_tbl = &(seafp_md->seafp_kusai_tbl);

    kusai = seafp_kusai_tbl->kusai;

    poly_fp_alloc_poly(polyfp_md_id, &poly_ec_t);
    poly_fp_alloc_poly(polyfp_md_id, &poly_t);
    poly_fp_alloc_poly(polyfp_md_id, &poly_mid);
    poly_fp_alloc_poly(polyfp_md_id, &poly_D);
    poly_fp_alloc_poly(polyfp_md_id, &poly_E);

    POLY_INIT(poly_ec_t);
    POLY_INIT(poly_t);
    POLY_INIT(poly_mid);
    POLY_INIT(poly_D);
    POLY_INIT(poly_E);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_t, LOC_SEAFP_0095);

    /* EC = (x^3+ax+b)^2 mod iso_h*/
    sea_fp_set_ec_xpart_to_poly(seafp_md_id, poly_ec_t);
    poly_fp_squ_poly(polyfp_md_id, poly_ec_t, poly_t);
    poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_ec_t);

    sea_fp_Elkies_set_kusai0(seafp_md_id, iso_h, kusai[ 0 ]);
    sea_fp_Elkies_set_kusai1(seafp_md_id, iso_h, kusai[ 1 ]);
    sea_fp_Elkies_set_kusai2(seafp_md_id, iso_h, kusai[ 2 ]);
    sea_fp_Elkies_set_kusai3(seafp_md_id, iso_h, kusai[ 3 ]);
    sea_fp_Elkies_set_kusai4(seafp_md_id, iso_h, kusai[ 4 ]);

    for ( index = 2; index < m; index ++ )
    {
        poly_fp_squ_poly(polyfp_md_id, kusai[ index ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_mid );
        poly_fp_mul_poly(polyfp_md_id, poly_mid, kusai[ index ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_mid );               /* poly_mid = kusai[index]^3 mod iso_h*/
        poly_fp_mul_poly(polyfp_md_id, poly_mid, kusai[ index + 2 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_D );                 /* poly_D = kusai[index+2]*kusai[index]^3 mod iso_h*/

        poly_fp_squ_poly(polyfp_md_id, kusai[ index + 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_mid );
        poly_fp_mul_poly(polyfp_md_id, poly_mid, kusai[ index + 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_mid );               /* poly_mid = kusai[index+1]^3 mod iso_h */
        poly_fp_mul_poly(polyfp_md_id, poly_mid, kusai[ index - 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_E );                 /* poly_E = kusai[index-1]*kusai[index+1]^3 mod iso_h */

        if ( 0 == (index & 1) )
        {
            poly_fp_mul_poly(polyfp_md_id, poly_ec_t, poly_D, poly_t );
            poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_D );
        }
        else
        {
            poly_fp_mul_poly(polyfp_md_id, poly_ec_t, poly_E, poly_t );
            poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_E );
        }
        poly_fp_sub_poly(polyfp_md_id, poly_D, poly_E, kusai[ 2 * index + 1 ]);

        poly_fp_squ_poly(polyfp_md_id, kusai[ index ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_D );                 /* poly_D = kusai[index]^2 mod iso_h */

        poly_fp_mul_poly(polyfp_md_id, poly_D, kusai[ index + 3 ], poly_t );

        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_D );                 /* poly_D = kusai[index+3]*kusai[index]^2 mod iso_h*/

        poly_fp_squ_poly(polyfp_md_id, kusai[ index + 2 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_E );                 /* poly_E = kusai[index+2]^2 mod iso_h*/
        poly_fp_mul_poly(polyfp_md_id, poly_E, kusai[ index - 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, poly_E );                 /* poly_E = kusai[index-1]*kusai[index+2]^2 mod iso_h*/

        poly_fp_sub_poly(polyfp_md_id, poly_D, poly_E, poly_D );
        poly_fp_mul_poly(polyfp_md_id, poly_D, kusai[ index + 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, iso_h, kusai[ 2 * index + 2 ]);

        bgn_fp_set_word(bgnfp_md_id, bgn_t, 2);
        bgn_fp_inv(bgnfp_md_id, bgn_t, bgn_t);
        poly_fp_mul_bgn(polyfp_md_id, kusai[ 2 * index + 2 ], bgn_t, kusai[ 2 * index + 2 ]);/* kusai[2n+2] <=== kusai[2n+2]/2*/
    }

    poly_fp_poly_destory(polyfp_md_id, poly_ec_t);
    poly_fp_poly_destory(polyfp_md_id, poly_t);
    poly_fp_poly_destory(polyfp_md_id, poly_mid);
    poly_fp_poly_destory(polyfp_md_id, poly_D);
    poly_fp_poly_destory(polyfp_md_id, poly_E);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_t, LOC_SEAFP_0096);

    return ( 0 );
}

UINT32 sea_fp_Elkies(const UINT32 seafp_md_id,
                         const UINT32  *seafp_pmodsmallprime_tbl,
                         const POLY *poly_gcd_result,
                         const UINT32 oddsmallprime_pos,
                         INT32 *tmod)
{
    UINT32 oddsmallprime;
    UINT32 idx_k;
    UINT32 idx_m;
    UINT32 idx_n;

    POLY   *poly_x;
    POLY   *Lx;
    POLY   *Ly;
    POLY   *EC;
    POLY   *EC2;
    POLY   *A;
    POLY   *B;
    POLY   *C;
    POLY   *D;
    POLY   *E;
    POLY   *F;
    POLY   *G;
    POLY   *iso_h;
    POLY   *tmp;
    ECFP_CURVE *iso_ec;
    BIGINT *root;
    BIGINT *iso_p1;
    BIGINT *exp;

    const POLY ** kusai;
    const UINT32 *pmodsmallprime;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;
    BIGINT      *ecfp_p;

    SEAFP_PRIME_TBL  *seafp_oddsmallprime_tbl;
    SEAFP_KUSAI_TBL  *seafp_kusai_tbl;

    SEAFP_MD    *seafp_md;
#if 0
    UINT32 cc = 0;
#endif
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == seafp_pmodsmallprime_tbl )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies: seafp_pmodsmallprime_tbl is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == poly_gcd_result )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies: poly_gcd_result is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == tmod )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies: tmod is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/


    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_p       = seafp_md->ecfp_p;

    seafp_kusai_tbl = &(seafp_md->seafp_kusai_tbl);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_ECFP_CURVE, &iso_ec, LOC_SEAFP_0097);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &root, LOC_SEAFP_0098);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &iso_p1, LOC_SEAFP_0099);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &exp, LOC_SEAFP_0100);

    poly_fp_alloc_poly(polyfp_md_id, &iso_h);
    poly_fp_alloc_poly(polyfp_md_id, &Lx);
    poly_fp_alloc_poly(polyfp_md_id, &Ly);
    poly_fp_alloc_poly(polyfp_md_id, &EC);
    poly_fp_alloc_poly(polyfp_md_id, &EC2);
    poly_fp_alloc_poly(polyfp_md_id, &tmp);
    poly_fp_alloc_poly(polyfp_md_id, &A);
    poly_fp_alloc_poly(polyfp_md_id, &B);
    poly_fp_alloc_poly(polyfp_md_id, &C);
    poly_fp_alloc_poly(polyfp_md_id, &D);
    poly_fp_alloc_poly(polyfp_md_id, &E);
    poly_fp_alloc_poly(polyfp_md_id, &F);
    poly_fp_alloc_poly(polyfp_md_id, &G);
    poly_fp_alloc_poly(polyfp_md_id, &poly_x);

    POLY_INIT(iso_h);
    POLY_INIT(Lx);
    POLY_INIT(Ly);
    POLY_INIT(EC);
    POLY_INIT(EC2);
    POLY_INIT(tmp);
    POLY_INIT(A);
    POLY_INIT(B);
    POLY_INIT(C);
    POLY_INIT(D);
    POLY_INIT(E);
    POLY_INIT(F);
    POLY_INIT(G);
    POLY_INIT(poly_x);

    kusai = (const POLY **)(seafp_kusai_tbl->kusai);
    pmodsmallprime = seafp_pmodsmallprime_tbl;

    oddsmallprime = seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ];
    idx_m = ( oddsmallprime + 7 )/4;

    /*set poly_x = x*/
    poly_fp_set_xn_word(polyfp_md_id, 1, poly_x);

    /*set EC = x^3 + a*x + b*/
    sea_fp_set_ec_xpart_to_poly(seafp_md_id, EC);

    /*set exp = (p-1)/2*/
    bgn_z_shr_lesswordsize(bgnz_md_id, ecfp_p, 1, exp);

    sea_fp_poly_squroot(seafp_md_id, poly_gcd_result, root);
#if 0
    sys_log(LOGSTDOUT,"root = \n");
    print_bigint(LOGSTDOUT, root);
#endif
    sea_fp_iso_ec(seafp_md_id, root, oddsmallprime_pos, iso_ec, iso_p1);
#if 0
    sys_log(LOGSTDOUT,"iso_ec = \n");
    print_ecfp_curve(LOGSTDOUT, iso_ec);
    sys_log(LOGSTDOUT,"iso_p1 = \n");
    print_bigint(LOGSTDOUT, iso_p1);
#endif
    sea_fp_iso_h(seafp_md_id, iso_ec, iso_p1, oddsmallprime_pos, iso_h);
#if 0
    sys_log(LOGSTDOUT,"iso_h = \n");
    print_poly(LOGSTDOUT, iso_h);
#endif
    sea_fp_Elkies_kusais(seafp_md_id, iso_h, idx_m);
    poly_fp_exp_mod(polyfp_md_id, poly_x, ecfp_p, iso_h, Lx);
    poly_fp_exp_mod(polyfp_md_id, EC, exp, iso_h, Ly);

#if 0
    sys_log(LOGSTDOUT,"Lx = \n");
    print_poly(LOGSTDOUT, Lx);
    sys_log(LOGSTDOUT,"Ly = \n");
    print_poly(LOGSTDOUT, Ly);
#endif

    poly_fp_squ_poly(polyfp_md_id, EC, tmp);
    poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, EC2);

#if 0
    sys_log(LOGSTDOUT,"EC2 = \n");
    print_poly(LOGSTDOUT, EC2);
#endif

    idx_m = ( oddsmallprime - 1) / 2;

    for ( idx_n = 1; idx_n <= idx_m; idx_n ++ )
    {
        poly_fp_squ_poly(polyfp_md_id, kusai[ idx_n ], tmp );
        poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, C );

#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]kusai[ %ld ]:\n",cc++, idx_n);
        print_poly(LOGSTDOUT, kusai[ idx_n ]);

        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]tmp:\n",cc++);
        print_poly(LOGSTDOUT, tmp);
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]iso_h:\n",cc++);
        print_poly(LOGSTDOUT, iso_h);
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]C:\n",cc++);
        print_poly(LOGSTDOUT, C);
#endif
        poly_fp_mul_poly(polyfp_md_id, kusai[ idx_n - 1 ], kusai[ idx_n + 1 ], tmp );

#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]kusai[ %ld ]:\n",cc++, idx_n - 1);
        print_poly(LOGSTDOUT, kusai[ idx_n - 1 ]);


        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]kusai[ %ld ]:\n",cc++, idx_n + 1);
        print_poly(LOGSTDOUT, kusai[ idx_n + 1 ]);

        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]tmp:\n",cc++);
        print_poly(LOGSTDOUT, tmp);
#endif
        poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, A );
#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]A:\n",cc++);
        print_poly(LOGSTDOUT, A);
#endif
        poly_fp_sub_poly(polyfp_md_id, poly_x, Lx, B );
        poly_fp_mul_poly(polyfp_md_id, B, C, tmp );
        poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, B );
#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]B:\n", cc++);
        print_poly(LOGSTDOUT, B);
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [1]idx_n & 1 = %ld\n", (idx_n & 1));
#endif

        if ( 1 == (idx_n & 1) )
        {
            poly_fp_mul_poly(polyfp_md_id, EC, A, tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, A );
#if 0
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]A:\n",cc++);
            print_poly(LOGSTDOUT, A);
#endif
        }
        else
        {
            poly_fp_mul_poly(polyfp_md_id, EC, B, tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, B );
#if 0
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]B:\n", cc++);
            print_poly(LOGSTDOUT, B);
#endif
        }
#if 0
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]A:\n", cc++);
        print_poly(LOGSTDOUT, A);

        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]B:\n", cc++);
        print_poly(LOGSTDOUT, B);
#endif
        poly_fp_sub_poly(polyfp_md_id, A, B, F );
#if 0
        if ( EC_TRUE == POLY_IS_EMPTY(F) )
        {
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [1]F is empty\n");
        }
        else
        {
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [1]F is not empty\n");
        }
        sys_log(LOGSTDOUT,"sea_fp_Elkies : [%ld]F:\n", cc++);
        print_poly(LOGSTDOUT, F);
#endif
        if ( EC_TRUE == POLY_IS_EMPTY(F) )
        {

            poly_fp_squ_poly(polyfp_md_id, kusai[ idx_n - 1 ], tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, A );     /* A = kusai[idx_n-1]^2 mod iso_h */
            poly_fp_mul_poly(polyfp_md_id, A, kusai[ idx_n + 2 ], tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, A );     /* A = kusai[idx_n+2]*kusai[idx_n-1]^2 mod iso_h */

            poly_fp_squ_poly(polyfp_md_id, kusai[ idx_n + 1 ], tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, B );     /* B = kusai[idx_n+1]^2 mod iso_h */
#if 0
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]idx_n = %ld\n", idx_n);
#endif
            if ( idx_n > 1 )
            {
                poly_fp_mul_poly(polyfp_md_id, B, kusai[ idx_n - 2 ], tmp );
                poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, B ); /* B = kusai[idx_n-2]*kusai[idx_n+1]^2 mod iso_h */
                poly_fp_sub_poly(polyfp_md_id, A, B, E );
            }
            else
            {
                poly_fp_add_poly(polyfp_md_id, A, B, E );
            }

            poly_fp_mul_word(polyfp_md_id, C, 4, D);
            poly_fp_mul_poly(polyfp_md_id, D, kusai[ idx_n ], tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, D );
            poly_fp_mul_poly(polyfp_md_id, D, Ly, tmp );
            poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, D );
#if 0
            sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]idx_n & 1 = %ld\n", (idx_n & 1));
#endif
            if ( 0 == (idx_n & 1) )
            {
                poly_fp_mul_poly(polyfp_md_id, D, EC2, tmp );
                poly_fp_mod_poly(polyfp_md_id, tmp, iso_h, D );
            }

            poly_fp_sub_poly(polyfp_md_id, D, E, G );
#if 0
            if ( EC_TRUE == POLY_IS_EMPTY(G) )
            {
                sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]G is empty\n");
            }
            else
            {
                sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]G is not empty\n");
            }
#endif
            if ( EC_TRUE == POLY_IS_EMPTY(G) )
            {
                for ( idx_k = 1; idx_k < oddsmallprime; idx_k ++ )
                {
                    if ( 1 == (( idx_k * idx_n ) % oddsmallprime) )
                        break;
                }
                (*tmod) = ( idx_n + pmodsmallprime[ oddsmallprime_pos ] * idx_k ) % oddsmallprime;
                break ;
            }

            poly_fp_add_poly(polyfp_md_id, D, E, F );
#if 0
            if ( EC_TRUE == POLY_IS_EMPTY(F) )
            {
                sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]F is empty\n");
            }
            else
            {
                sys_log(LOGSTDOUT,"sea_fp_Elkies : [2]F is not empty\n");
            }
#endif
            if ( EC_TRUE == POLY_IS_EMPTY(F) )
            {
                for ( idx_k = 1; idx_k < oddsmallprime; idx_k ++ )
                {
                    if ( 1 == (( idx_k * idx_n ) % oddsmallprime) )
                    {
                        break;
                    }
                }
                (*tmod) = ( oddsmallprime - ( idx_n + pmodsmallprime[ oddsmallprime_pos ] * idx_k ) % oddsmallprime ) % oddsmallprime;
                break ;
            }
        }
    }

    free_static_mem(MD_SEAFP, seafp_md_id, MM_ECFP_CURVE, iso_ec, LOC_SEAFP_0101);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, root, LOC_SEAFP_0102);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, iso_p1, LOC_SEAFP_0103);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, exp, LOC_SEAFP_0104);

    poly_fp_poly_destory(polyfp_md_id, iso_h);
    poly_fp_poly_destory(polyfp_md_id, Lx);
    poly_fp_poly_destory(polyfp_md_id, Ly);
    poly_fp_poly_destory(polyfp_md_id, EC);
    poly_fp_poly_destory(polyfp_md_id, EC2);
    poly_fp_poly_destory(polyfp_md_id, tmp);
    poly_fp_poly_destory(polyfp_md_id, A);
    poly_fp_poly_destory(polyfp_md_id, B);
    poly_fp_poly_destory(polyfp_md_id, C);
    poly_fp_poly_destory(polyfp_md_id, D);
    poly_fp_poly_destory(polyfp_md_id, E);
    poly_fp_poly_destory(polyfp_md_id, F);
    poly_fp_poly_destory(polyfp_md_id, G);
    poly_fp_poly_destory(polyfp_md_id, poly_x);

    return ( 0 );
}

UINT32 sea_fp_Elkies_CRT(const UINT32 seafp_md_id, const UINT32 x2, const UINT32 m2,BIGINT *x1,BIGINT *m1)
{
    BIGINT *bgn_x2;
    BIGINT *bgn_m2;

    UINT32   bgnz_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == x1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_CRT: x1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Elkies_CRT: m1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Elkies_CRT: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_x2, LOC_SEAFP_0105);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_m2, LOC_SEAFP_0106);

    bgn_z_set_word(bgnz_md_id, bgn_x2, x2);
    bgn_z_set_word(bgnz_md_id, bgn_m2, m2);

    bgn_z_crt(bgnz_md_id, x1, m1, bgn_x2, bgn_m2, x1, m1);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_x2, LOC_SEAFP_0107);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_m2, LOC_SEAFP_0108);

    return ( 0 );
}

/**
*
*   here,
*       poly_xpx_mod_fai_j  = (x^p - x) mod fai[oddsmallprime_pos](x,y=j)
*
*
*
**/
UINT32 sea_fp_Atkin_r(const UINT32 seafp_md_id, const POLY *poly_xpx_mod_fai_j, const UINT32 oddsmallprime_pos, INT32 *r)
{
    POLY *G[SEAFP_MAX_SMALL_PRIME];
    POLY *poly_f;
    POLY *poly_t;
    POLY *poly_mid;
    POLY_ITEM *item_f;
    UINT32 word_deg_of_poly_t;
    POLY *poly_fai_y_is_j;

    INT32 *factor_pt;
    INT32  factor;

    UINT32 idx_i;
    UINT32 idx_k;
    EC_BOOL ok;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;

    SEAFP_PRIME_TBL  *seafp_oddsmallprime_tbl;
    SEAFP_FAI_TBL    *seafp_fai_tbl;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == poly_xpx_mod_fai_j )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: poly_xpx_mod_fai_j is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: r is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_r: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    bgnfp_md_id  = seafp_md->bgnfp_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    seafp_fai_tbl= &(seafp_md->seafp_fai_tbl);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);

    poly_fai_y_is_j = (seafp_fai_tbl->fai_j[ oddsmallprime_pos ] );

    poly_fp_alloc_poly(polyfp_md_id, &poly_f);
    POLY_INIT(poly_f);

    poly_fp_alloc_poly(polyfp_md_id, &poly_t);
    POLY_INIT(poly_t);

    poly_fp_alloc_poly(polyfp_md_id, &poly_mid);
    POLY_INIT(poly_mid);

    /*poly_t = x + poly_xpx_mod_fai_j = x^p mod fai[oddsmallprime_pos](x,y=j)*/
    poly_fp_set_xn_word(polyfp_md_id, 1, poly_t);
    poly_fp_add_poly(polyfp_md_id, poly_t, poly_xpx_mod_fai_j, poly_t);
    poly_fp_mod_poly(polyfp_md_id, poly_t, poly_fai_y_is_j, poly_t);

    /*let poly_f = x^p mod fai[oddsmallprime_pos](x,y=j)*/
    //poly_fp_poly_clone(polyfp_md_id, poly_t, poly_f);

    if ( EC_FALSE == bgn_z_get_word(bgnz_md_id, POLY_DEG(poly_t), &word_deg_of_poly_t) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: poly_t degree is too large to deal with.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    /*if deg(poly_t) >= SEAFP_MAX_SMALL_PRIME*/
    if ( word_deg_of_poly_t >= SEAFP_MAX_SMALL_PRIME )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: poly_t degree overflow.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    for(idx_i = 0; idx_i < SEAFP_MAX_SMALL_PRIME; idx_i ++)
    {
        G[ idx_i ] = NULL_PTR;
    }

    for ( idx_i = 0; idx_i <= word_deg_of_poly_t; idx_i ++)
    {
        poly_fp_alloc_poly(polyfp_md_id, &(G[ idx_i ]));
        POLY_INIT(G[ idx_i ]);
    }

    /*step 1.precomputing G[i] = g^i, i = 0....degree(g), */
    /*where g = poly_t = x^p mod fai[oddsmallprime_pos](x,y=j)*/
    poly_fp_set_one(polyfp_md_id, G[ 0 ]);          /*G [ 0 ] = 0*/
    poly_fp_poly_move(polyfp_md_id, poly_t, G[ 1 ]);/*G [ 1 ] = g*/

    for ( idx_i = 2; idx_i <= word_deg_of_poly_t; idx_i ++)
    {
        poly_fp_mul_poly(polyfp_md_id, G[ 1 ], G[ idx_i - 1 ], poly_t );
        poly_fp_mod_poly(polyfp_md_id, poly_t, poly_fai_y_is_j, G[ idx_i ] );
    }

    /*step 2. compute r.*/
    if ( 0 > legendretable[ oddsmallprime_pos ][ seafp_oddsmallprime_tbl->pmodsmallprime[ oddsmallprime_pos ] ] )
    {
        factor_pt = oddfactor[ oddsmallprime_pos ];
    }
    else
    {
        factor_pt = evenfactor[ oddsmallprime_pos ];
    }

    /*ok = TRUE means not completed yet*/
    ok = EC_TRUE;
    poly_fp_poly_clone(polyfp_md_id, G[ 1 ], poly_f);

    for ( factor = 1, idx_i = 0; factor_pt[ idx_i + 1 ] != -1; idx_i ++ )
    {
        for ( ;factor < factor_pt[ idx_i ]; factor ++ )
        {
            /*let f(x) = e_0 + e_1 *x + ...+ e_d * x^d*/
            /*then compute f(x = g) = e_0 + e_1 * G[ 1 ] +... + e_d * G[ d ]*/

            /*set poly_t = 0 at first*/
            poly_fp_poly_clean(polyfp_md_id, poly_t);

            item_f = POLY_FIRST_ITEM(poly_f);
            while ( item_f != POLY_NULL_ITEM(poly_f) )
            {
                if ( EC_FALSE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_f)) )
                {
                    if ( EC_FALSE == bgn_z_get_word(bgnz_md_id, POLY_ITEM_DEG(item_f), &idx_k) )
                    {
                        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: item_f degree is too large to deal with.\n");
                        dbg_exit(MD_SEAFP, seafp_md_id);
                    }
                    poly_fp_mul_bgn(polyfp_md_id, G[ idx_k ], POLY_ITEM_BGN_COE(item_f), poly_mid);
                    poly_fp_add_poly(polyfp_md_id, poly_t, poly_mid, poly_t);
                }
                item_f = POLY_ITEM_NEXT(item_f);
            }

            poly_fp_poly_move(polyfp_md_id, poly_t, poly_f);
        }
#if 0
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r: [2]poly_f:\n");
        print_poly(LOGSTDOUT, poly_f);
#endif
        /*search for the item_f with deg(item_f) = 1*/
        item_f = POLY_FIRST_ITEM(poly_f);
        while ( item_f != POLY_NULL_ITEM(poly_f) )
        {
            if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_ITEM_DEG(item_f), 1) )
            {
                break;
            }
            item_f = POLY_ITEM_NEXT(item_f);
        }

        /*if item_f = x, then set ok = 1 and break the loop*/
        if ( item_f != POLY_NULL_ITEM(poly_f) )
        {
            if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, POLY_ITEM_BGN_COE(item_f)) )
            {
                ok = EC_FALSE;
                break;
            }
        }
    }

    if ( EC_TRUE == ok )
    {
        ( *r ) = factor_pt[ idx_i ];
    }
    else
    {
        ( *r ) = factor;
    }

    poly_fp_poly_destory(polyfp_md_id, poly_f);
    poly_fp_poly_destory(polyfp_md_id, poly_t);
    poly_fp_poly_destory(polyfp_md_id, poly_mid);

    for ( idx_i = 0; idx_i <= word_deg_of_poly_t; idx_i ++)
    {
        poly_fp_poly_destory(polyfp_md_id, G[ idx_i ]);
    }

    return ( 0 );
}

UINT32 sea_fp_Atkin_r_pos(const UINT32 seafp_md_id, const UINT32 oddsmallprime_pos, const INT32 Atkin_r, UINT32 *Atkin_r_pos )
{
    UINT32 rpos;
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_r_pos )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_r_pos: Atkin_r_pos is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_r_pos: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/


    for ( rpos = 0; factortable[ oddsmallprime_pos ][ rpos ] < Atkin_r; rpos ++ )
    {
        ;
    }

    *Atkin_r_pos = rpos;

    return ( 0 );
}

UINT32 sea_fp_Atkin_item_clone(const UINT32 seafp_md_id,
                                    const SEAFP_ATKIN_ITEM *Atkin_item_src,
                                    SEAFP_ATKIN_ITEM *Atkin_item_des )
{
    UINT32 index;

    UINT32 bgnz_md_id;
    SEAFP_MD    *seafp_md;


#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_item_src )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_insert: Atkin_item_src is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == Atkin_item_des )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_insert: Atkin_item_des is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_item_clone: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;

    bgn_z_clone(bgnz_md_id, Atkin_item_src->prime, Atkin_item_des->prime);
    for ( index = 0; index < 2 * SEAFP_MAX_LEN_OF_ZTABLE; index ++ )
    {
        Atkin_item_des->tmodprime[ index ] = Atkin_item_src->tmodprime[ index ];
    }
    Atkin_item_des->rate = Atkin_item_src->rate;

    return ( 0 );
}
UINT32 sea_fp_Atkin_s_insert(const UINT32 seafp_md_id,
                                    const SEAFP_ATKIN_ITEM *Atkin_item_new )
{
    UINT32 idx_i;
    UINT32 idx_j;
    SEAFP_ATKIN_TBL *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM *Atkin_item_tmp;
    SEAFP_ATKIN_ITEM *Atkin_item_src;
    SEAFP_ATKIN_ITEM *Atkin_item_des;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_item_new )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_insert: Atkin_item_new is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_s_insert: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md  = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    for ( idx_i = 0; idx_i < seafp_Atkin_tbl->pos; idx_i ++ )
    {
        Atkin_item_tmp = &(seafp_Atkin_tbl->Atkin[ idx_i ]);

        if ( Atkin_item_new->rate > Atkin_item_tmp->rate )
        {
            break;
        }
    }

    seafp_Atkin_tbl->pos++;
    for ( idx_j = seafp_Atkin_tbl->pos; idx_j > idx_i; idx_j -- )
    {
        Atkin_item_src = &(seafp_Atkin_tbl->Atkin[ idx_j - 1 ]);
        Atkin_item_des = &(seafp_Atkin_tbl->Atkin[ idx_j ]);
        sea_fp_Atkin_item_clone(seafp_md_id, Atkin_item_src, Atkin_item_des);
    }
    Atkin_item_des = &(seafp_Atkin_tbl->Atkin[ idx_j ]);
    sea_fp_Atkin_item_clone(seafp_md_id, Atkin_item_new, Atkin_item_des);

    return ( 0 );
}

UINT32 sea_fp_Atkin_s_pos_modify(const UINT32 seafp_md_id, const BIGINT *Elkies_m )
{
    BIGINT *bgn_t0;
    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

    BIGINT *bgn_t;
    BIGINT *prime;
    BIGINT  *boundary;
    UINT32  idx_cur;

    SEAFP_ATKIN_TBL *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM *Atkin_item;

    UINT32   bgnz_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Elkies_m )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_pos_modify: Elkies_m is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_s_pos_modify: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_t0, LOC_SEAFP_0109);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_c0, LOC_SEAFP_0110);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &bgn_c1, LOC_SEAFP_0111);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &boundary, LOC_SEAFP_0112);

    /*note: boundary < 4*sqrt(p) < boundary + 1,*/
    /*i.e, ( boundary )^2 < 16*p < ( boundary + 1 )^2*/
    boundary->data[ 0 ] = 0xFFFFFFFF;
    boundary->data[ 1 ] = 0xFFFFFFFF;
    boundary->data[ 2 ] = 0xFFFFFFFF;
    boundary->data[ 3 ] = 0x00000003;
    boundary->len = 4;

    bgn_z_clone(bgnz_md_id, Elkies_m, bgn_t0);

    for ( idx_cur = 0; idx_cur < seafp_Atkin_tbl->pos; idx_cur ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx_cur ]);
        prime = Atkin_item->prime;
        bgn_z_mul(bgnz_md_id, bgn_t0, prime, bgn_c0, bgn_c1);
        if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c1) )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_pos_modify: overflow.\n");
            dbg_exit(MD_SEAFP, seafp_md_id);
        }

        bgn_t  = bgn_t0;
        bgn_t0 = bgn_c0;
        bgn_c0 = bgn_t;

        if ( 0 <= bgn_z_cmp(bgnz_md_id, bgn_t0, boundary ) )
        {
            break;
        }
    }
    seafp_Atkin_tbl->pos = idx_cur + 1;

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_t0, LOC_SEAFP_0113);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_c0, LOC_SEAFP_0114);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, bgn_c1, LOC_SEAFP_0115);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, boundary, LOC_SEAFP_0116);

    return ( 0 );
}

UINT32 sea_fp_Atkin_s_breakpoint_get(const UINT32 seafp_md_id, UINT32 *breakpoint )
{
    UINT32 total;
    UINT32 half;
    UINT32 count;
    UINT32 index;

    SEAFP_ATKIN_TBL *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM *Atkin_item;

    UINT32   bgnz_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == breakpoint )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_breakpoint_get: breakpoint is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_s_breakpoint_get: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id = seafp_md->bgnz_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    total = 0;
    for ( index = 0; index < seafp_Atkin_tbl->pos; index ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ index ]);
        total = total + Atkin_item->tmodprime[ 0 ];
    }

    half = total / 2;
    count = 0;
    for ( index = 0; index < seafp_Atkin_tbl->pos; index ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ index ]);
        count = count + Atkin_item->tmodprime[ 0 ];

        if ( count > half)
        {
            break;
        }
    }

    *breakpoint = index;

    return ( 0 );
}

UINT32 sea_fp_Atkin_s_modify(const UINT32 seafp_md_id)
{
    UINT32 index;

    SEAFP_ATKIN_TBL *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM *Atkin_item;
    SEAFP_ATKIN_ITEM *Atkin_item_src;
    SEAFP_ATKIN_ITEM *Atkin_item_des;

    UINT32   bgnz_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_s_modify: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    /*if prime 3 or 5 is Atkin prime, then remove them from Atkin Prime Set*/
    /*because they contribute little to inequation pai( smallprime ) < 4*sqrt(p)*/
    /*but the possible value set {t mod 3} or {t mod 5} will slow down the baby-step-gaint-step*/
    /*thus, remove them is a good tactic.*/
    for ( index = seafp_Atkin_tbl->pos; index > 0; )
    {
        /*when enter the for-loop body, must have seafp_Atkin_tbl->pos > 0*/
        index --;
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ index ]);
        if ( EC_TRUE == bgn_z_is_n(bgnz_md_id, Atkin_item->prime, 5)
          || EC_TRUE == bgn_z_is_n(bgnz_md_id, Atkin_item->prime, 3))
        {
            /*since must have seafp_Atkin_tbl->pos > 0, subtration (seafp_Atkin_tbl->pos - 1) is safe*/
            for ( ; index < seafp_Atkin_tbl->pos - 1; index ++ )
            {
                Atkin_item_src = &((seafp_Atkin_tbl->Atkin[ index + 1 ]));
                Atkin_item_des = &((seafp_Atkin_tbl->Atkin[ index ]));
                sea_fp_Atkin_item_clone(seafp_md_id, Atkin_item_src, Atkin_item_des);
            }
            seafp_Atkin_tbl->pos --;
            break;
        }
    }

    return ( 0 );
}

UINT32 sea_fp_Atkin_s_breakpoint_modify(const UINT32 seafp_md_id, BIGINT *Atkin_m1, BIGINT *Atkin_m2, UINT32 *breakpoint )
{
    UINT32 tmp;
    UINT32 bound;
    UINT32 index;
    UINT32 bkp;

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *t0;
    BIGINT *t1;
    BIGINT *t;

    SEAFP_ATKIN_TBL *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM *Atkin_item;

    UINT32   bgnz_md_id;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_m1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_breakpoint_modify: Atkin_m1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == Atkin_m2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_breakpoint_modify: Atkin_m2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == breakpoint )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_s_breakpoint_modify: breakpoint is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_s_breakpoint_modify: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c0, LOC_SEAFP_0117);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c1, LOC_SEAFP_0118);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t0, LOC_SEAFP_0119);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t1, LOC_SEAFP_0120);

    bgn_z_set_one(bgnz_md_id, c0);
    bgn_z_set_one(bgnz_md_id, c1);

    tmp = 1;
    bound = SEAFP_MAX_ARRAY_SIZE;

    bkp = (*breakpoint);
    for ( index = 0; index < bkp; index ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ index ]);

        tmp *= Atkin_item->tmodprime[ 0 ];
        if ( tmp > bound )
        {
            /*okay, the current Atkin does not mutiple to c0*/
            break;
        }
        bgn_z_mul(bgnz_md_id, c0, Atkin_item->prime, t0, t1);
        if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, t1) )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_modify_Atkin_s_breakpoint: overflow - t1 is nonzero.\n");
            dbg_exit(MD_SEAFP, seafp_md_id);
        }
        t = c0;
        c0 = t0;
        t0 = t;
    }

    (*breakpoint) = index ;

    for ( ; index < seafp_Atkin_tbl->pos; index ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ index ]);
        bgn_z_mul(bgnz_md_id, c1, Atkin_item->prime, t0, t1);
        if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, t1) )
        {
            sys_log(LOGSTDOUT,"error:sea_fp_modify_Atkin_s_breakpoint: overflow - t1 is nonzero.\n");
            dbg_exit(MD_SEAFP, seafp_md_id);
        }
        t = c1;
        c1 = t0;
        t0 = t;
    }

    bgn_z_clone(bgnz_md_id, c0, Atkin_m1);
    bgn_z_clone(bgnz_md_id, c1, Atkin_m2);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c0, LOC_SEAFP_0121);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c1, LOC_SEAFP_0122);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t0, LOC_SEAFP_0123);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t1, LOC_SEAFP_0124);

    return ( 0 );
}

UINT32 sea_fp_Atkin_set_crt(const UINT32 seafp_md_id,
                            const BIGINT **xmododdsmallprime_tbl,
                            const UINT32  xmododdsmallprime_tbl_size,
                            const UINT32  from,
                            BIGINT *x)
{
    BIGINT *tx;
    BIGINT *tm;

    UINT32 idx;

    SEAFP_ATKIN_ITEM *Atkin_item;
    SEAFP_ATKIN_TBL  *seafp_Atkin_tbl;

    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == xmododdsmallprime_tbl )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_set_crt: xmododdsmallprime_tbl is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == x )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_set_crt: x is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_set_crt: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tx, LOC_SEAFP_0125);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tm, LOC_SEAFP_0126);

    Atkin_item = &(seafp_Atkin_tbl->Atkin[ from ]);

    bgn_z_clone(bgnz_md_id, xmododdsmallprime_tbl[ 0 ], tx);
    bgn_z_clone(bgnz_md_id, Atkin_item->prime, tm);

    for ( idx = 1; idx < xmododdsmallprime_tbl_size; idx ++ )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx + from ]);

        bgn_z_crt(bgnz_md_id, tx, tm, xmododdsmallprime_tbl[ idx ], Atkin_item->prime, tx, tm);
    }
    bgn_z_clone(bgnz_md_id, tx, x);

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tx, LOC_SEAFP_0127);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tm, LOC_SEAFP_0128);
    return ( 0 );
}
UINT32 sea_fp_hash_key_gen(const UINT32 seafp_md_id,const BIGINT *data,UINT32 *key)
{
#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == data )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_hash_key_gen: data is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == key )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_hash_key_gen: key is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_hash_key_gen: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    if ( 0 == data->len )
    {
        ( *key ) = 0;
    }
    else
    {
        ( *key ) = data->data[ 0 ];
    }

    return ( 0 );
}
UINT32 sea_fp_Atkin_node_clone(const UINT32 seafp_md_id, const SEAFP_ATKIN_NODE *Atkin_node_src, SEAFP_ATKIN_NODE *Atkin_node_des)
{

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_node_src )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_node_clone: Atkin_node_src is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == Atkin_node_des )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_node_clone: Atkin_node_des is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_node_clone: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    Atkin_node_des->r1             = Atkin_node_src->r1           ;
    Atkin_node_des->Atkin_Q        = Atkin_node_src->Atkin_Q      ;
    Atkin_node_des->x_sort_key     = Atkin_node_src->x_sort_key   ;
    Atkin_node_des->y_sort_key     = Atkin_node_src->y_sort_key   ;


    return ( 0 );
}
UINT32 sea_fp_Atkin_node_exchange(const UINT32 seafp_md_id, SEAFP_ATKIN_NODE *Atkin_node_1, SEAFP_ATKIN_NODE *Atkin_node_2)
{
    BIGINT *r1_t;
    UINT32  x_sort_key_t;
    UINT32  y_sort_key_t;
    EC_CURVE_POINT *Atkin_Q_t;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == Atkin_node_1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_node_exchange: Atkin_node_1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == Atkin_node_2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_Atkin_node_exchange: Atkin_node_2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_Atkin_node_exchange: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    r1_t            = Atkin_node_1->r1           ;
    Atkin_Q_t       = Atkin_node_1->Atkin_Q      ;
    x_sort_key_t    = Atkin_node_1->x_sort_key   ;
    y_sort_key_t    = Atkin_node_1->y_sort_key   ;

    Atkin_node_1->r1             = Atkin_node_2->r1           ;
    Atkin_node_1->Atkin_Q        = Atkin_node_2->Atkin_Q      ;
    Atkin_node_1->x_sort_key     = Atkin_node_2->x_sort_key   ;
    Atkin_node_1->y_sort_key     = Atkin_node_2->y_sort_key   ;

    Atkin_node_2->r1             = r1_t           ;
    Atkin_node_2->Atkin_Q        = Atkin_Q_t      ;
    Atkin_node_2->x_sort_key     = x_sort_key_t   ;
    Atkin_node_2->y_sort_key     = y_sort_key_t   ;

    return ( 0 );
}
UINT32 sea_fp_heap_sift(const UINT32 seafp_md_id, const UINT32 beg_pos, const UINT32 end_pos)
{
    SEAFP_ATKIN_NODE  buf_1;

    SEAFP_ATKIN_NODE *Atkin_node_t1;
    SEAFP_ATKIN_NODE *Atkin_node_t2;
    SEAFP_ATKIN_NODE *Atkin_node_ti;
    SEAFP_ATKIN_NODE *Atkin_node_tj;
    SEAFP_ATKIN_NODE *Atkin_node_t;

    UINT32 cur_x_sort_key;

    UINT32 idx_j;
    UINT32 idx_i;

    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_heap_sift: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

    Atkin_node_t = &buf_1;
    Atkin_node_t->r1 = NULL_PTR;
    Atkin_node_t->Atkin_Q = NULL_PTR;
    Atkin_node_t->x_sort_key = 0;
    Atkin_node_t->y_sort_key = 0;

    sea_fp_Atkin_node_clone(seafp_md_id, &(seafp_Atkin_set->Atkin_Set_Node[ beg_pos ]), Atkin_node_t);

    cur_x_sort_key = Atkin_node_t->x_sort_key;

    idx_i = beg_pos;
    idx_j = 2 * idx_i;

    while ( idx_j <= end_pos )
    {
        Atkin_node_t1 = &(seafp_Atkin_set->Atkin_Set_Node[ idx_j ]);
        Atkin_node_t2 = &(seafp_Atkin_set->Atkin_Set_Node[ idx_j + 1 ]);
        if ( idx_j < end_pos && Atkin_node_t1->x_sort_key < Atkin_node_t2->x_sort_key )
        {
            idx_j ++;
            Atkin_node_t1 = &(seafp_Atkin_set->Atkin_Set_Node[ idx_j ]);
        }

        if ( Atkin_node_t->x_sort_key < Atkin_node_t1->x_sort_key )
        {
            Atkin_node_ti = &(seafp_Atkin_set->Atkin_Set_Node[ idx_i ]);
            Atkin_node_tj = &(seafp_Atkin_set->Atkin_Set_Node[ idx_j ]);

            sea_fp_Atkin_node_clone(seafp_md_id, Atkin_node_tj, Atkin_node_ti);

            idx_i = idx_j;
            idx_j = 2 * idx_i;
        }
        else
        {
            break;
        }
    }

    Atkin_node_ti = &(seafp_Atkin_set->Atkin_Set_Node[ idx_i ]);
    sea_fp_Atkin_node_clone(seafp_md_id, Atkin_node_t, Atkin_node_ti);

    return ( 0 );
}
UINT32 sea_fp_heap_sort(const UINT32 seafp_md_id)
{
    UINT32 idx_i;
    UINT32 num;

    SEAFP_ATKIN_NODE *Atkin_node_t1;
    SEAFP_ATKIN_NODE *Atkin_node_t2;
    SEAFP_ATKIN_SET  *seafp_Atkin_set;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_heap_sort: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT, " [sort      ] sorting ...\n" );

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);
    num = seafp_Atkin_set->Atkin_Set_size - 1;

    /*initilize the Index Table of Atkin Set*/
    for ( idx_i = 0; idx_i < seafp_Atkin_set->Atkin_Set_size; idx_i ++ )
    {
        seafp_Atkin_set->Atkin_Set_Node_Index[ idx_i ] = idx_i;
    }

    /*sort Atkin Node[ 1 ] to Node [ Atkin_Set_size - 1 ]*/
    for ( idx_i = num/2; idx_i >= 1; idx_i -- )
    {
        sea_fp_heap_sift(seafp_md_id, idx_i, num );
    }

    for ( idx_i = num; idx_i >= 1; idx_i -- )
    {
        /*exchange Node[ 1 ] and Node [ idx_i ]*/
        Atkin_node_t1 = &(seafp_Atkin_set->Atkin_Set_Node[ 1 ]);
        Atkin_node_t2 = &(seafp_Atkin_set->Atkin_Set_Node[ idx_i ]);
        sea_fp_Atkin_node_exchange(seafp_md_id, Atkin_node_t1, Atkin_node_t2);

        sea_fp_heap_sift(seafp_md_id, 1, idx_i - 1 );
    }

    sys_log(LOGSTDOUT, " [sort      ] sort over\n" );

    return ( 0 );
}
EC_BOOL sea_fp_match_R(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const BIGINT *r2,
                            const EC_CURVE_POINT *H1,
                            BIGINT *r1)
{
    UINT32 cur;
    UINT32 left;
    UINT32 mid;
    UINT32 right;

    UINT32 setT_max_index;

    UINT32 H1_x_sort_key;
    UINT32 H1_y_sort_key;

    EC_BOOL bool_found;

    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_ATKIN_NODE  *Atkin_node;

    UINT32   bgnz_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == m1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: m1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: m2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: m3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == t3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: t3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == r2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: r2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == H1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: H1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == r1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_match_R: r1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_match_R: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id = seafp_md->bgnz_md_id;

    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);
    setT_max_index  = seafp_Atkin_set->Atkin_Set_size;

    sea_fp_hash_key_gen(seafp_md_id, &(H1->x), &H1_x_sort_key);
    sea_fp_hash_key_gen(seafp_md_id, &(H1->y), &H1_y_sort_key);

    for ( cur = 1; cur < setT_max_index; cur = 2 * cur )
    {
        Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ cur ]);
        if ( H1_x_sort_key <= Atkin_node->x_sort_key )
        {
            break;
        }
    }

    left  = (cur > 1) ? (cur / 2) : cur;
    right = (cur < setT_max_index) ? cur : setT_max_index;
    mid  = ( left + right ) / 2;

    while ( left < mid )
    {
        Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ mid ]);
        if ( H1_x_sort_key > Atkin_node->x_sort_key )
        {
            left = mid;
        }
        else
        {
            right = mid;
        }
        mid = ( left + right ) / 2;
    }

    cur = mid;
    Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ cur ]);
    if ( H1_x_sort_key > Atkin_node->x_sort_key )
    {
        cur ++;
        Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ cur ]);
    }

    bool_found = EC_FALSE;

    for ( ;
                  cur < setT_max_index
               && H1_x_sort_key == Atkin_node->x_sort_key
               && H1_y_sort_key == Atkin_node->y_sort_key;
         cur ++, Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ cur ]) )
    {
        if ( 0 == bgn_z_cmp(bgnz_md_id, &(H1->x), &(Atkin_node->Atkin_Q->x))
          && 0 == bgn_z_cmp(bgnz_md_id, &(H1->y), &(Atkin_node->Atkin_Q->y)) )
        {
            bool_found = EC_TRUE;

            bgn_z_clone(bgnz_md_id, Atkin_node->r1, r1);

            sys_log(LOGSTDOUT, " success ! \n" );

            break;
        }
    }

    return ( bool_found );
}

UINT32 sea_fp_baby_step(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const UINT32  depth,
                            const EC_CURVE_POINT *test_point)
{
    UINT32  idxstack[ SEAFP_SMALL_PRIME_NUM ];
    BIGINT *tmododdsmallprime[ SEAFP_SMALL_PRIME_NUM ];
    BIGINT *t1;
    BIGINT *t3_mod_m1;
    BIGINT *m2_mod_m1;
    BIGINT *m3_mod_m1;
    BIGINT *inv_m2m3_mod_m1;
    BIGINT *tmp;
    BIGINT *r1;
    BIGINT *c1;
    BIGINT *half_m1;
    EC_CURVE_POINT *Q0;
    EC_CURVE_POINT *Q1;
    EC_CURVE_POINT *Q2;
    EC_CURVE_POINT *tQ;

    BIGINT *ecfp_p;
    ECFP_CURVE * ecfp_curve;

    EC_BOOL over;
    UINT32  idx;
    UINT32  idx_j;

    SEAFP_ATKIN_TBL   *seafp_Atkin_tbl;
    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_ATKIN_ITEM  *Atkin_item;
    SEAFP_ATKIN_NODE  *Atkin_node;
    UINT32         Atkin_node_pos;

    UINT32   bgnzn_md_id;
    UINT32   ecfp_md_id_of_Q1;

    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == m1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: m1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: m2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: m3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == t3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: t3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == test_point )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: test_point is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_baby_step: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT, " [baby step ] start\n" );

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    ecfp_p       = seafp_md->ecfp_p;
    ecfp_curve   = seafp_md->ecfp_curve;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

    for ( idx = 0; idx < SEAFP_SMALL_PRIME_NUM; idx ++ )
    {
        tmododdsmallprime[ idx ] = NULL_PTR;
    }
    for ( idx = 0; idx < depth; idx ++ )
    {
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(tmododdsmallprime[ idx ]), LOC_SEAFP_0129);
    }
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t1, LOC_SEAFP_0130);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &r1, LOC_SEAFP_0131);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t3_mod_m1, LOC_SEAFP_0132);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &m2_mod_m1, LOC_SEAFP_0133);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &m3_mod_m1, LOC_SEAFP_0134);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &inv_m2m3_mod_m1, LOC_SEAFP_0135);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &half_m1, LOC_SEAFP_0136);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tmp, LOC_SEAFP_0137);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c1, LOC_SEAFP_0138);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &Q0, LOC_SEAFP_0139);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &Q1, LOC_SEAFP_0140);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &Q2, LOC_SEAFP_0141);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &tQ, LOC_SEAFP_0142);

    bgnzn_md_id = bgn_zn_start(m1);

    bgn_z_mod(bgnz_md_id, t3, m1, t3_mod_m1);           /*t3_mod_m1 = t3 mod m1*/
    bgn_z_mod(bgnz_md_id, m2, m1, m2_mod_m1);           /*m2_mod_m1 = m2 mod m1*/
    bgn_z_mod(bgnz_md_id, m3, m1, m3_mod_m1);           /*m3_mod_m1 = m3 mod m1*/

    /*compute 1/(m2*m3) mod m1*/
    bgn_zn_mul(bgnzn_md_id, m2_mod_m1, m3_mod_m1, tmp);
    bgn_zn_inv(bgnzn_md_id, tmp, inv_m2m3_mod_m1);


    /*half_m1 = m1/2*/
    bgn_z_shr_lesswordsize(bgnz_md_id, m1, 1, half_m1);

    /*tmp = p + 1 - t3*/
    bgn_z_sub(bgnz_md_id, ecfp_p, t3, tmp);
    bgn_z_sadd(bgnz_md_id, tmp, 1, tmp);

    /*Q0 = [p+1-t3]P*/
    ec_fp_point_mul(ecfp_md_id, test_point, tmp, Q0);

    /*Q1 = [m2*m3]P*/
    bgn_z_mul(bgnz_md_id, m2, m3, tmp, c1);
    if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_baby_step: m2 * m3 overflow.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    ec_fp_point_mul(ecfp_md_id, test_point, tmp, Q1);

    /*Q2 = [m1]P*/
    ec_fp_point_mul(ecfp_md_id, test_point, m1, Q2);

    ecfp_md_id_of_Q1 = ec_fp_start(ecfp_p, ecfp_curve, NULL_PTR, Q1);

    seafp_Atkin_set->Atkin_Set_size = 1;
    //sys_log(LOGSTDOUT,"after sea_fp_baby_step: Atkin_Set_size = %ld\n", seafp_Atkin_set->Atkin_Set_size);

    /* baby step :*/
    for ( idx = 0; idx < depth; idx ++ )
    {
        idxstack[ idx ] = 1;
    }

    over = EC_FALSE;
    idx = 0;

    while ( EC_FALSE == over )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx ]);
        if ( idxstack[ idx ] > Atkin_item->tmodprime[ 0 ] )
        {
            if ( idx == 0 )
            {
                over = EC_TRUE;
            }
            else
            {
                for ( idx_j = idx; idx_j < depth; idx_j ++ )
                {
                    idxstack[ idx_j ] = 1;
                }

                idx --;
                idxstack[ idx ] ++;
            }
        }
        else
        {
            bgn_z_set_word(bgnz_md_id, tmododdsmallprime[ idx ], Atkin_item->tmodprime[ idxstack[ idx ] ]);

            if ( idx + 1 != depth )
            {
                idx ++;
            }
            else
            {
                //sys_log(LOGSTDOUT, "sea_fp_baby_step: chrem begin\n");
                sea_fp_Atkin_set_crt(seafp_md_id, (const BIGINT **)tmododdsmallprime, depth, 0, t1);
#if 0
                /*now t = t1 mod m1 and t1 < m1*/
                sys_log(LOGSTDOUT, "sea_fp_baby_step: chrem end\n");
                sys_log(LOGSTDOUT,"sea_fp_baby_step: t1 = ");
                print_bigint_dec(LOGSTDOUT, t1);
#endif

                /*r1 = (t1 - t3) / (m2 * m3) mod m1, where 0 <= r1 < m1*/
                bgn_zn_sub(bgnzn_md_id, t1, t3_mod_m1, tmp);
                bgn_zn_mul(bgnzn_md_id, tmp, inv_m2m3_mod_m1, r1);
#if 0
                sys_log(LOGSTDOUT,"sea_fp_baby_step: r1 = ");
                print_bigint_dec(LOGSTDOUT, r1);
#endif

#if 1

                Atkin_node_pos = seafp_Atkin_set->Atkin_Set_size;
                Atkin_node = &(seafp_Atkin_set->Atkin_Set_Node[ Atkin_node_pos ]);

                alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &(Atkin_node->Atkin_Q), LOC_SEAFP_0143);
                alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(Atkin_node->r1), LOC_SEAFP_0144);

                /*tQ = [r1]Q1 = [r1*m2*m3]P*/
                ec_fp_point_mul_fix_base(ecfp_md_id_of_Q1, r1, tQ);
#if 0
                sys_log(LOGSTDOUT,"\n");
                sys_log(LOGSTDOUT,"==============================================================\n");
                sys_log(LOGSTDOUT,"P:\n");print_point(LOGSTDOUT, test_point);

                sys_log(LOGSTDOUT,"Q0 = [p+1-t3]P:\n");print_point(LOGSTDOUT, Q0);
                sys_log(LOGSTDOUT,"Q1 = [m2*m3]P:\n");print_point(LOGSTDOUT, Q1);
                sys_log(LOGSTDOUT,"Q2 = [m1]P:\n");print_point(LOGSTDOUT, Q2);
                sys_log(LOGSTDOUT,"tQ = [r1]Q1:\n");print_point(LOGSTDOUT, tQ);

                sys_log(LOGSTDOUT,"p: ");print_bigint(LOGSTDOUT, ecfp_p);
                sys_log(LOGSTDOUT,"t3: ");print_bigint(LOGSTDOUT, t3);
                sys_log(LOGSTDOUT,"r1: ");print_bigint(LOGSTDOUT, r1);

#endif
                /*Q_{r1_0} = Q0 - tQ = [p+1-t3]P - [r1*m2*m3]P*/
                ec_fp_point_sub(ecfp_md_id_of_Q1, Q0, tQ, Atkin_node->Atkin_Q);
#if 0
                sys_log(LOGSTDOUT,"R = Q0 - tQ:\n");print_point(LOGSTDOUT, Atkin_node->Atkin_Q);
#endif
                #if 0
                //sys_log(LOGSTDOUT,"sea_fp_baby_step: half_m1 = ");
                //print_bigint_dec(LOGSTDOUT, half_m1);

                /*if r1 > [m1/2]*/
                if ( 0 < bgn_z_cmp(bgnz_md_id, r1, half_m1) )
                {
                    /*Q_{r1_0} = Q_{r1_0} + Q2, where Q2 = [m1]Q1 = [m1*m2*m3]P*/
                    /*so that Q_{r1_0} = [p+1-t3]P - [r1*m2*m3]P + [m1*m2*m3]P*/
                    ec_fp_point_add(ecfp_md_id_of_Q1, Atkin_node->Atkin_Q, Q2, Atkin_node->Atkin_Q);

                    sys_log(LOGSTDOUT,"S = Q0 - tQ + Q2:\n");print_point(LOGSTDOUT, Atkin_node->Atkin_Q);


                    /*set Atkin_flg = 1*/
                    Atkin_node->Atkin_r1_sign = 1;
                }
                else
                {
                    /*set Atkin_flg = 0*/
                    Atkin_node->Atkin_r1_sign = 0;
                }
                #endif
#if 0
                sys_log(LOGSTDOUT,"==============================================================\n");
#endif
                bgn_z_clone(bgnz_md_id, r1, Atkin_node->r1);

                sea_fp_hash_key_gen(seafp_md_id, &(Atkin_node->Atkin_Q->x), &(Atkin_node->x_sort_key));
                sea_fp_hash_key_gen(seafp_md_id, &(Atkin_node->Atkin_Q->y), &(Atkin_node->y_sort_key));
#if 0
                {
                    UINT32 _idx_;
                    for ( _idx_ = 0; _idx_ < depth;  _idx_ ++ )
                    {

                        //sys_log(sea_fp_baby_step_log,"%d,", Atkin_item->tmodprime[ idxstack[ _idx_ ] ]);
                        print_bigint_decchars(sea_fp_baby_step_log, tmododdsmallprime[ _idx_ ]);
                        sys_log(sea_fp_baby_step_log,",");
                    }
                    sys_log(sea_fp_baby_step_log, "\n");
                }

                sea_fp_test_Atkin_set_node_output(sea_fp_baby_step_log, Atkin_node);
#endif
#endif
                seafp_Atkin_set->Atkin_Set_size ++;

                //sys_log(LOGSTDOUT, "sea_fp_baby_step: compute (Q_{r1_0}, r1, Atkin_flg) end - ");
                if ( 0 ==  ((seafp_Atkin_set->Atkin_Set_size) % 1000))
                {
                    sys_log(LOGSTDOUT," sea_fp_baby_step: Atkin Set Size = %ld\n", seafp_Atkin_set->Atkin_Set_size);
                }

                idxstack[ idx ] ++;
            }
        }
    }
#if 0
    sys_log(LOGSTDOUT, "sea_fp_baby_step: ");
    sys_log(LOGSTDOUT," Atkin Set Size = %ld\n", seafp_Atkin_set->Atkin_Set_size);
#endif
    bgn_zn_end(bgnzn_md_id);
    ec_fp_end(ecfp_md_id_of_Q1);

    for ( idx = 0; idx < depth; idx ++ )
    {
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (tmododdsmallprime[ idx ]), LOC_SEAFP_0145);
    }
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t1, LOC_SEAFP_0146);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, r1, LOC_SEAFP_0147);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t3_mod_m1, LOC_SEAFP_0148);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, m2_mod_m1, LOC_SEAFP_0149);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, m3_mod_m1, LOC_SEAFP_0150);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, inv_m2m3_mod_m1, LOC_SEAFP_0151);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, half_m1, LOC_SEAFP_0152);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tmp, LOC_SEAFP_0153);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c1, LOC_SEAFP_0154);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Q0, LOC_SEAFP_0155);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Q1, LOC_SEAFP_0156);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Q2, LOC_SEAFP_0157);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, tQ, LOC_SEAFP_0158);
#if 0
    fclose(sea_fp_baby_step_log);
#endif
#if 0
    sys_log(LOGSTDOUT,"sea_fp_baby_step: before sort, Atkin Set beg ====================== \n");
    sea_fp_test_Atkin_set_output(seafp_md_id);
    sys_log(LOGSTDOUT,"sea_fp_baby_step: before sort, Atkin Set end ====================== \n");

    sys_log(LOGSTDOUT, " [baby step ] ");
    sys_log(LOGSTDOUT,"setT_index = %d\n", seafp_Atkin_set->Atkin_Set_size);

    sys_log(LOGSTDOUT, " [sort      ] sorting ...\n" );

    sys_log(LOGSTDOUT,"sea_fp_baby_step: before sort\n");
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"\n");

    sys_log(LOGSTDOUT,"sea_fp_baby_step: check Atkin Set before sort\n");
    sea_fp_test_Atkin_set_check(seafp_md_id);

    sys_log(LOGSTDOUT,"sea_fp_baby_step: before sort, Atkin Set beg ====================== \n");
    sea_fp_test_Atkin_set_output(seafp_md_id);
    sys_log(LOGSTDOUT,"sea_fp_baby_step: before sort, Atkin Set end ====================== \n");

    /////////////////////////////////////////////
    sea_fp_heap_sort(seafp_md_id);
    /////////////////////////////////////////////

    sys_log(LOGSTDOUT,"sea_fp_baby_step: check Atkin Set after sort\n");
    sea_fp_test_Atkin_set_check(seafp_md_id);

    sys_log(LOGSTDOUT,"sea_fp_baby_step: after sort, Atkin Set beg ====================== \n");
    sea_fp_test_Atkin_set_output(seafp_md_id);
    sys_log(LOGSTDOUT,"sea_fp_baby_step: after sort, Atkin Set end ====================== \n");

    sys_log(LOGSTDOUT,"sea_fp_baby_step: after sort\n");
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"\n");

    sys_log(LOGSTDOUT, " [sort      ] sorted\n" );
#endif
    sys_log(LOGSTDOUT, " [baby step ]     over\n" );

    return ( 0 );
}

EC_BOOL sea_fp_giant_step(const UINT32 seafp_md_id,
                            const BIGINT *m1,
                            const BIGINT *m2,
                            const BIGINT *m3,
                            const BIGINT *t3,
                            const UINT32  from,
                            const UINT32  depth,
                            const EC_CURVE_POINT *test_point,
                            BIGINT *r1,
                            BIGINT *r2,
                            UINT32 *order_case
                            )
{
    UINT32  idxstack[ SEAFP_SMALL_PRIME_NUM ];
    BIGINT *tmododdsmallprime[ SEAFP_SMALL_PRIME_NUM ];
    BIGINT *t2;
    BIGINT *t3_mod_m2;
    BIGINT *m1_mod_m2;
    BIGINT *m3_mod_m2;
    BIGINT *inv_m1m3_mod_m2;
    BIGINT *tmp;
    BIGINT *c0;
    BIGINT *c1;
    EC_CURVE_POINT *Q2;
    EC_CURVE_POINT *Q3;
    EC_CURVE_POINT *R;
    EC_CURVE_POINT *tQ;

    BIGINT *ecfp_p;
    ECFP_CURVE * ecfp_curve;

    EC_BOOL over;
    UINT32  idx;
    UINT32  idx_j;

    EC_BOOL bool_found;

    clock_t start;
    clock_t end;

    SEAFP_ATKIN_TBL   *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM  *Atkin_item;

    UINT32  bgnzn_md_id;
    UINT32   ecfp_md_id_with_Q2;

    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == m1 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: m1 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: m2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == m3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: m3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == t3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: t3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == test_point )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: test_point is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == r2 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: r2 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == order_case )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: order_case is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_giant_step: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT, " [giant step] start\n" );

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    ecfp_p       = seafp_md->ecfp_p;
    ecfp_curve   = seafp_md->ecfp_curve;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    for ( idx = 0; idx < SEAFP_SMALL_PRIME_NUM; idx ++ )
    {
        tmododdsmallprime[ idx ] = NULL_PTR;
    }
    for ( idx = 0; idx < depth; idx ++ )
    {
        alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(tmododdsmallprime[ idx ]), LOC_SEAFP_0159);
    }

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t2, LOC_SEAFP_0160);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &t3_mod_m2, LOC_SEAFP_0161);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &m1_mod_m2, LOC_SEAFP_0162);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &m3_mod_m2, LOC_SEAFP_0163);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &inv_m1m3_mod_m2, LOC_SEAFP_0164);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &tmp, LOC_SEAFP_0165);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c0, LOC_SEAFP_0166);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c1, LOC_SEAFP_0167);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &Q2, LOC_SEAFP_0168);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &Q3, LOC_SEAFP_0169);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &R, LOC_SEAFP_0170);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &tQ, LOC_SEAFP_0171);

    /*Q2 = [m1*m3]P*/
    bgn_z_mul(bgnz_md_id, m1, m3, c0, c1);
    if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: m1 * m3 overflow.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    ec_fp_point_mul(ecfp_md_id, test_point, c0, Q2);

    /*Q3 = [m1*m2*m3]P*/
    bgn_z_mul(bgnz_md_id, c0, m2, tmp, c1);
    if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1) )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_giant_step: m1 *m2 * m3 overflow.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    ec_fp_point_mul(ecfp_md_id, test_point, tmp, Q3);

    bgnzn_md_id = bgn_zn_start(m2);
    ecfp_md_id_with_Q2 = ec_fp_start(ecfp_p, ecfp_curve, NULL_PTR, Q2);

    bgn_z_mod(bgnz_md_id, t3, m2, t3_mod_m2);        /*t3_mod_m2 = t3 mod m2*/
    bgn_z_mod(bgnz_md_id, m1, m2, m1_mod_m2);        /*m1_mod_m2 = m1 mod m2*/
    bgn_z_mod(bgnz_md_id, m3, m2, m3_mod_m2);        /*m3_mod_m2 = m3 mod m2*/

    /*compute inv_m1m3_mod_m2 = 1/(m1*m3) mod m2*/
    bgn_zn_mul(bgnzn_md_id, m1_mod_m2, m3_mod_m2, tmp);
    bgn_zn_inv(bgnzn_md_id, tmp, inv_m1m3_mod_m2);


    /* gaint step :*/

    start = clock();

    for ( idx = 0; idx < depth; idx++ )
    {
        idxstack[ idx ] = 1;
    }

    bool_found = EC_FALSE;
    over = EC_FALSE;
    idx = 0;

    while ( EC_FALSE == over )
    {
        Atkin_item = &(seafp_Atkin_tbl->Atkin[ idx + from ]);
        if ( idxstack[ idx ] > Atkin_item->tmodprime[ 0 ] )
        {
            if ( 0 == idx )
            {
                over = EC_TRUE;
            }
            else
            {
                for ( idx_j = idx;idx_j < depth; idx_j ++ )
                {
                    idxstack[ idx_j ] = 1;
                }
                idx --;
                idxstack[ idx ] ++;
            }
        }
        else
        {
            bgn_z_set_word(bgnz_md_id, tmododdsmallprime[ idx ], Atkin_item->tmodprime[ idxstack[ idx ] ]);
            if ( idx + 1 != depth )
            {
                idx ++;
            }
            else
            {
                end = clock();
                if ( start - end > (10800.00) )
                {
                    ( *order_case ) = SEAFP_ORDER_CASE_0;

                    sys_log(LOGSTDOUT, "[elapsed time > 10800 seconds]failure!\n");
                    break;
                }

                /*t2 mod oddsmallprime = tmododdsmallprime*/
                /*where 0 <= t2 < m2 = pai(oddsmallprime)*/
                sea_fp_Atkin_set_crt(seafp_md_id, (const BIGINT **)tmododdsmallprime, depth, from, t2);
#if 0
                /*now t = t2 mod m2 and t2 < m2*/
                sys_log(LOGSTDOUT, "sea_fp_giant_step: chrem end\n");
                sys_log(LOGSTDOUT,"sea_fp_giant_step: t2 = ");
                print_bigint_dec(LOGSTDOUT, t2);

#endif
                /*compute r2 = (t2 - t3)/(m1*m3) mod m2*/
                bgn_zn_sub(bgnzn_md_id, t2, t3_mod_m2, tmp);    /*tmp = (t2_mod_m2 - t3_mod_m2) mod m2 = (t2-t3) mod m2*/
                bgn_zn_mul(bgnzn_md_id, tmp, inv_m1m3_mod_m2, r2);     /*r2 = r2 / (m1*m3) mod m2*/
#if 0
                sys_log(LOGSTDOUT,"sea_fp_giant_step: r2 = ");
                print_bigint_dec(LOGSTDOUT, r2);
#endif
                /*R = [r2]Q2 = [r2*m1*m3]P*/
                ec_fp_point_mul_fix_base(ecfp_md_id_with_Q2, r2, R);
#if 0
                sys_log(LOGSTDOUT,"sea_fp_giant_step: [1] R:\n");
                print_point(LOGSTDOUT, R);
                sys_log(LOGSTDOUT,"\n");
#endif
#if 0
                {
                    UINT32 _idx_;
                    for ( _idx_ = 0; _idx_ < depth;  _idx_ ++ )
                    {

                        //sys_log(sea_fp_giant_step_log,"%d,", Atkin_item->tmodprime[ idxstack[ _idx_ ] ]);
                        print_bigint_decchars(sea_fp_giant_step_log, tmododdsmallprime[ _idx_ ]);
                        sys_log(sea_fp_giant_step_log,",");
                    }
                    sys_log(sea_fp_giant_step_log, "\n");
                }
                sys_log(sea_fp_giant_step_log,"R1:\n");
                print_point(sea_fp_giant_step_log, R);
#endif
                /*look up table to determine R is in it or not*/
                bool_found = sea_fp_match_R(seafp_md_id, m1, m2, m3, t3, r2, R, r1);
                if ( EC_TRUE == bool_found )
                {
                    ( *order_case ) = SEAFP_ORDER_CASE_1;
                    break;
                }

                /*try R = [r2-m2]Q2 = [r2*m1*m3]P - [m1*m2*m3]P*/
                ec_fp_point_mul_fix_base(ecfp_md_id_with_Q2, m2, tQ);/*tQ = [m1*m2*m3]P*/
                ec_fp_point_sub(ecfp_md_id, R, tQ, R);
#if 0
                sys_log(sea_fp_giant_step_log,"R2:\n");
                print_point(sea_fp_giant_step_log, R);
#endif
#if 0
                sys_log(LOGSTDOUT,"sea_fp_giant_step: [2] R:\n");
                print_point(LOGSTDOUT, R);
                sys_log(LOGSTDOUT,"\n");
#endif
                bool_found = sea_fp_match_R(seafp_md_id, m1, m2, m3, t3, r2, R, r1);
                if ( EC_TRUE == bool_found )
                {
                    /*now order = (p + 1 - t3) - (r1*m2*m3) -(r2*m1*m3) + m1*m2*m3*/
                    /*so that, trace t = t3 + (r1*m2*m3) + (r2*m1*m3) - m1*m2*m3*/

                    ( *order_case ) = SEAFP_ORDER_CASE_2;
                    break;
                }


                idxstack[ idx ] ++;
            }
        }
    }

    bgn_zn_end(bgnzn_md_id);
    ec_fp_end(ecfp_md_id_with_Q2);


    for ( idx = 0; idx < depth; idx ++ )
    {
        free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (tmododdsmallprime[ idx ]), LOC_SEAFP_0172);
    }

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t2, LOC_SEAFP_0173);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, t3_mod_m2, LOC_SEAFP_0174);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, m1_mod_m2, LOC_SEAFP_0175);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, m3_mod_m2, LOC_SEAFP_0176);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, inv_m1m3_mod_m2, LOC_SEAFP_0177);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, tmp, LOC_SEAFP_0178);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c0, LOC_SEAFP_0179);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c1, LOC_SEAFP_0180);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Q2, LOC_SEAFP_0181);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, Q3, LOC_SEAFP_0182);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, R, LOC_SEAFP_0183);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, tQ, LOC_SEAFP_0184);
#if 0
    fclose( sea_fp_giant_step_log );
#endif
    sys_log(LOGSTDOUT, " [giant step]     over\n" );

    return ( bool_found );
}

UINT32 sea_fp_ec_point_gen(const UINT32 seafp_md_id, EC_CURVE_POINT *ecfp_point)
{
    BIGINT *x;
    BIGINT *y;

    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == ecfp_point )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_ec_point_gen: ecfp_point is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_ec_point_gen: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/


    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id = seafp_md->bgnz_md_id;
    ecfp_md_id = seafp_md->ecfp_md_id;

    x = &(ecfp_point->x);
    y = &(ecfp_point->y);

    bgn_z_set_one(bgnz_md_id, x);

    while ( EC_FALSE == ec_fp_compute_y(ecfp_md_id, x, y) )
    {
        bgn_z_sadd(bgnz_md_id, x, 1, x);
    }

    return ( 0 );
}

static UINT32 sea_fp_test_init(const UINT32 seafp_md_id, BIGINT *m3, BIGINT *t3)
{
    SEAFP_ATKIN_ITEM  buf_1;
    BIGINT            buf_2;

    SEAFP_ATKIN_TBL   *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM  *Atkin_tbl_item;

    UINT32 oddsmallprime;
    UINT32 tmododdsmallprime_idx;
    UINT32 Atkin_tbl_item_idx;
    UINT32   bgnz_md_id;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( NULL_PTR == m3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_test_init: m3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
    if ( NULL_PTR == t3 )
    {
        sys_log(LOGSTDOUT,"error:sea_fp_test_init: t3 is NULL_PTR.\n");
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_test_init: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    Atkin_tbl_item = &buf_1;
    Atkin_tbl_item->prime = &buf_2;

#define SEAFP_FP_TEST_SET_TMODL(tmodl) \
    Atkin_tbl_item->tmodprime[ ++ tmododdsmallprime_idx ] = (tmodl);\
    Atkin_tbl_item->tmodprime[ ++ tmododdsmallprime_idx ] = (oddsmallprime - (tmodl))

#define SEAFP_FP_TEST_SET_FIRST() \
    tmododdsmallprime_idx = 0

#define SEAFP_FP_TEST_SET_LAST() \
    while(0){Atkin_tbl_item->tmodprime[ ++ tmododdsmallprime_idx ] = ((UINT32)(-1));}\
    Atkin_tbl_item->tmodprime[ 0 ] = tmododdsmallprime_idx;

    Atkin_tbl_item_idx = 0;

    /*set seafp_Atkin_tbl*/
    seafp_Atkin_tbl->pos = 0;

   /*[Atkin    ]  t mod  3 :  2, over */

    oddsmallprime = 3;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(1);
    //SEAFP_FP_TEST_SET_TMODL(2);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.1875;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin_esp]  t mod  5 :  1  over */

    oddsmallprime = 5;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(1);
    SEAFP_FP_TEST_SET_TMODL(4);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.3125;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod  7 :  2, 4, over */

    oddsmallprime = 7;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(2);
        SEAFP_FP_TEST_SET_TMODL(4);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.027344;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin_esp]  t mod 11 :  5  over */

    oddsmallprime = 11;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(5);
    SEAFP_FP_TEST_SET_TMODL(6);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.6875;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 17 :  6, 5,16, over */

    oddsmallprime = 17;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(6 );
    SEAFP_FP_TEST_SET_TMODL(5 );
    //SEAFP_FP_TEST_SET_TMODL(16);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.013117;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 19 :  2, 5,16,18, over */

    oddsmallprime = 19;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(2 );
    SEAFP_FP_TEST_SET_TMODL(5 );
    //SEAFP_FP_TEST_SET_TMODL(16);
    //SEAFP_FP_TEST_SET_TMODL(18);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.004639;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 29 : 23,21,17, 1, over */

    oddsmallprime = 29;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(23);
    SEAFP_FP_TEST_SET_TMODL(21);
    //SEAFP_FP_TEST_SET_TMODL(17);
    //SEAFP_FP_TEST_SET_TMODL(1 );
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.007080;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin_esp]  t mod 31 : 14  over */

    oddsmallprime = 31;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(14);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 1.937500;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 41 : 22,36,40, over */

    oddsmallprime = 41;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(22);
    //SEAFP_FP_TEST_SET_TMODL(36);
    //SEAFP_FP_TEST_SET_TMODL(40);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.031636;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 47 : 28, over */

    oddsmallprime = 47;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(28);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 2.937500;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 53 :  2,10,18,24,36,42,46,50,52, over */

    oddsmallprime = 53;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(2 );
    //SEAFP_FP_TEST_SET_TMODL(10);
    //SEAFP_FP_TEST_SET_TMODL(18);
    //SEAFP_FP_TEST_SET_TMODL(24);
    SEAFP_FP_TEST_SET_TMODL(36);
    //SEAFP_FP_TEST_SET_TMODL(42);
    //SEAFP_FP_TEST_SET_TMODL(46);
    //SEAFP_FP_TEST_SET_TMODL(50);
    //SEAFP_FP_TEST_SET_TMODL(52);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.000505;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 59 : 26,34, over */

    oddsmallprime = 59;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(26);
    SEAFP_FP_TEST_SET_TMODL(34);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.230469;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 61 :  6, 8,10,12,16,18,22,24,26,28,30,34,48,50,60, over */

    oddsmallprime = 61;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(6 );
    //SEAFP_FP_TEST_SET_TMODL(8 );
    //SEAFP_FP_TEST_SET_TMODL(10);
    SEAFP_FP_TEST_SET_TMODL(12);
    //SEAFP_FP_TEST_SET_TMODL(16);
    //SEAFP_FP_TEST_SET_TMODL(18);
    //SEAFP_FP_TEST_SET_TMODL(22);
    //SEAFP_FP_TEST_SET_TMODL(24);
    //SEAFP_FP_TEST_SET_TMODL(26);
    //SEAFP_FP_TEST_SET_TMODL(28);
    //SEAFP_FP_TEST_SET_TMODL(30);
    //SEAFP_FP_TEST_SET_TMODL(34);
    //SEAFP_FP_TEST_SET_TMODL(48);
    //SEAFP_FP_TEST_SET_TMODL(50);
    //SEAFP_FP_TEST_SET_TMODL(60);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.000075;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 67 :  8,14,26,28,30,40,44,66, over */

    oddsmallprime = 67;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(8 );
    //SEAFP_FP_TEST_SET_TMODL(14);
    //SEAFP_FP_TEST_SET_TMODL(26);
    //SEAFP_FP_TEST_SET_TMODL(28);
    //SEAFP_FP_TEST_SET_TMODL(30);
    //SEAFP_FP_TEST_SET_TMODL(40);
    //SEAFP_FP_TEST_SET_TMODL(44);
    //SEAFP_FP_TEST_SET_TMODL(66);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.001022;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 71 : 21, 6, 8,12,20,22,28,30,38,52,58,64, over */

    oddsmallprime = 71;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(21);
    //SEAFP_FP_TEST_SET_TMODL(6 );
    //SEAFP_FP_TEST_SET_TMODL(8 );
    //SEAFP_FP_TEST_SET_TMODL(12);
    //SEAFP_FP_TEST_SET_TMODL(20);
    //SEAFP_FP_TEST_SET_TMODL(22);
    //SEAFP_FP_TEST_SET_TMODL(28);
    //SEAFP_FP_TEST_SET_TMODL(30);
    //SEAFP_FP_TEST_SET_TMODL(38);
    //SEAFP_FP_TEST_SET_TMODL(52);
    //SEAFP_FP_TEST_SET_TMODL(58);
    //SEAFP_FP_TEST_SET_TMODL(64);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.000214;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 79 :  7, 2, 8,12,14,16,20,24,26,30,32,34,40,54,58,74, over */

    oddsmallprime = 79;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(7 );
    //SEAFP_FP_TEST_SET_TMODL(2 );
    //SEAFP_FP_TEST_SET_TMODL(8 );
    //SEAFP_FP_TEST_SET_TMODL(12);
    //SEAFP_FP_TEST_SET_TMODL(14);
    //SEAFP_FP_TEST_SET_TMODL(16);
    //SEAFP_FP_TEST_SET_TMODL(20);
    //SEAFP_FP_TEST_SET_TMODL(24);
    //SEAFP_FP_TEST_SET_TMODL(26);
    //SEAFP_FP_TEST_SET_TMODL(30);
    //SEAFP_FP_TEST_SET_TMODL(32);
    //SEAFP_FP_TEST_SET_TMODL(34);
    //SEAFP_FP_TEST_SET_TMODL(40);
    //SEAFP_FP_TEST_SET_TMODL(54);
    //SEAFP_FP_TEST_SET_TMODL(58);
    //SEAFP_FP_TEST_SET_TMODL(74);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.000075;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 83 :  5, 8,14,16,28,32,34,36,38,42,44,60, over */

    oddsmallprime = 83;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    SEAFP_FP_TEST_SET_TMODL(5 );
    //SEAFP_FP_TEST_SET_TMODL(8 );
    //SEAFP_FP_TEST_SET_TMODL(14);
    //SEAFP_FP_TEST_SET_TMODL(16);
    //SEAFP_FP_TEST_SET_TMODL(28);
    //SEAFP_FP_TEST_SET_TMODL(32);
    //SEAFP_FP_TEST_SET_TMODL(34);
    //SEAFP_FP_TEST_SET_TMODL(36);
    //SEAFP_FP_TEST_SET_TMODL(38);
    //SEAFP_FP_TEST_SET_TMODL(42);
    //SEAFP_FP_TEST_SET_TMODL(44);
    //SEAFP_FP_TEST_SET_TMODL(60);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.000250;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);

    /*[Atkin    ]  t mod 97 : 83,53,45, over */

    oddsmallprime = 97;
    bgn_z_set_word(bgnz_md_id, Atkin_tbl_item->prime, oddsmallprime);
    SEAFP_FP_TEST_SET_FIRST();
    //SEAFP_FP_TEST_SET_TMODL(83);
    //SEAFP_FP_TEST_SET_TMODL(53);
    SEAFP_FP_TEST_SET_TMODL(45);
    SEAFP_FP_TEST_SET_LAST();
    Atkin_tbl_item->rate = 0.074846;/*xxx*/
    sea_fp_Atkin_s_insert(seafp_md_id, Atkin_tbl_item);


    /*m3 = 13*23*37*43*73*89 = 0xB8380E1D = 3090681373*/
    bgn_z_set_word(bgnz_md_id, m3, 0xB8380E1D);

    /*t3 = 0x59D69C1E = 1507236894*/
    bgn_z_set_word(bgnz_md_id, t3, 0x59D69C1E);

    return ( 0 );
}
static UINT32 sea_fp_test_Atkin_set_check(const UINT32 seafp_md_id)
{
    UINT32 idx_i;

    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_ATKIN_NODE  *Atkin_tbl_node;

    SEAFP_MD    *seafp_md;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

    for ( idx_i = 0; idx_i < seafp_Atkin_set->Atkin_Set_size; idx_i ++ )
    {
        Atkin_tbl_node = &(seafp_Atkin_set->Atkin_Set_Node[ idx_i ]);

        if ( NULL_PTR == Atkin_tbl_node->Atkin_Q )
        {
            sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_check: idx = %ld Atkin_Q is null.\n");
        }
        if ( NULL_PTR == Atkin_tbl_node->r1)
        {
            sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_check: idx = %ld r1 is null.\n");
        }
    }

    return ( 0 );

}

static UINT32 sea_fp_test_Atkin_set_node_output(LOG *log, const SEAFP_ATKIN_NODE  *Atkin_set_node)
{
    if ( NULL_PTR != Atkin_set_node )
    {

        if ( NULL_PTR != Atkin_set_node->Atkin_Q )
        {
            sys_log(log,"Atkin_Q:\n");
            print_point(log, Atkin_set_node->Atkin_Q);
        }
        if ( NULL_PTR != Atkin_set_node->r1)
        {
            sys_log(log,"r1:\n");
            print_bigint(log, Atkin_set_node->r1);
        }
        sys_log(log,"x_sort_key = 0x%lx\n",  Atkin_set_node->x_sort_key);
        sys_log(log,"y_sort_key = 0x%lx\n",  Atkin_set_node->y_sort_key);
        sys_log(log,"\n");
    }

    return ( 0 );
}

static UINT32 sea_fp_test_Atkin_set_output(const UINT32 seafp_md_id)
{
    UINT32 idx_i;

    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_ATKIN_NODE  *Atkin_set_node;

    SEAFP_MD    *seafp_md;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);

    sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: Atkin_Set_size = %ld\n",seafp_Atkin_set->Atkin_Set_size);

    for ( idx_i = 0; idx_i < seafp_Atkin_set->Atkin_Set_size; idx_i ++ )
    {
        Atkin_set_node = &(seafp_Atkin_set->Atkin_Set_Node[ idx_i ]);
#if 0
        sys_log(LOGSTDOUT,"\n");
        sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: idx_i = %ld\n", idx_i);
        if ( NULL_PTR != Atkin_set_node->Atkin_Q )
        {
            sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: Atkin_Q:\n");
            print_point(LOGSTDOUT, Atkin_set_node->Atkin_Q);
        }
        if ( NULL_PTR != Atkin_set_node->r1)
        {
            sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: r1:\n");
            print_bigint(LOGSTDOUT, Atkin_set_node->r1);
        }
#endif
        sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: [ %ld ] x_sort_key = 0x%lx\n",  idx_i, Atkin_set_node->x_sort_key);
        sys_log(LOGSTDOUT,"sea_fp_test_Atkin_set_output: [ %ld ] y_sort_key = 0x%lx\n",  idx_i, Atkin_set_node->y_sort_key);
    }

    return ( 0 );

}
static UINT32 sea_fp_test_Atkin_output(const UINT32 seafp_md_id)
{
    UINT32 idx_i;
    UINT32 idx_j;

    SEAFP_ATKIN_TBL   *seafp_Atkin_tbl;
    SEAFP_ATKIN_ITEM  *Atkin_tbl_item;

    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_test_Atkin_output: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);

    for ( idx_i = 0; idx_i < seafp_Atkin_tbl->pos; idx_i ++ )
    {
        Atkin_tbl_item = &(seafp_Atkin_tbl->Atkin[ idx_i ]);
        sys_log(LOGSTDOUT,"prime = ");
        print_bigint_decchars(LOGSTDOUT, Atkin_tbl_item->prime);
        sys_log(LOGSTDOUT," : ");

        for ( idx_j = 1; idx_j <= Atkin_tbl_item->tmodprime[ 0 ]; idx_j ++ )
        {
            sys_log(LOGSTDOUT,"%ld,", Atkin_tbl_item->tmodprime[ idx_j ]);
        }
        sys_log(LOGSTDOUT, " [%f]\n", Atkin_tbl_item->rate);
    }

    return ( 0 );
}

static UINT32 sea_fp_test_Atkin_set_heapsort_output(const UINT32 seafp_md_id, const UINT32 pos)
{
    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_ATKIN_NODE  *Atkin_set_node_cur;
    SEAFP_ATKIN_NODE  *Atkin_set_node_left;
    SEAFP_ATKIN_NODE  *Atkin_set_node_right;
    UINT32         Atkin_Set_size;
    SEAFP_MD    *seafp_md;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);
    Atkin_Set_size = seafp_Atkin_set->Atkin_Set_size;

    if ( pos >= Atkin_Set_size)
    {
        return ( 0 );
    }

    Atkin_set_node_cur = &(seafp_Atkin_set->Atkin_Set_Node[ pos ]);
    sys_log(LOGSTDOUT,"[ %ld ]x_sort_key = 0x%lx", pos,  Atkin_set_node_cur->x_sort_key);

    if ( 2*pos < Atkin_Set_size )
    {
        Atkin_set_node_left = &(seafp_Atkin_set->Atkin_Set_Node[ 2*pos ]);
        if ( Atkin_set_node_cur->x_sort_key > Atkin_set_node_left->x_sort_key )
        {
            sys_log(LOGSTDOUT," [error:cur(%ld) > left(%ld)]", pos, 2*pos);
        }
    }
    if ( 2*pos+1 < Atkin_Set_size )
    {
        Atkin_set_node_right = &(seafp_Atkin_set->Atkin_Set_Node[ 2*pos+1 ]);
        if ( Atkin_set_node_cur->x_sort_key > Atkin_set_node_right->x_sort_key )
        {
            sys_log(LOGSTDOUT," [error:cur(%ld) > right(%ld)]", pos, 2*pos+1);
        }
    }

    sys_log(LOGSTDOUT,"\n");

    sea_fp_test_Atkin_set_heapsort_output(seafp_md_id, 2*pos);
    sea_fp_test_Atkin_set_heapsort_output(seafp_md_id, 2*pos + 1);

    return ( 0 );
}

UINT32 sea_fp_compute_order(const UINT32 seafp_md_id,
                                        const BIGINT *m1,
                                        const BIGINT *m2,
                                        const BIGINT *m3,
                                        const BIGINT *t3,
                                        const BIGINT *r1,
                                        const BIGINT *r2,
                                        const UINT32 order_case,
                                        UINT32 *carry_of_order,
                                        BIGINT *unsigned_order)
{
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *c2;
    BIGINT *c3;
    BIGINT *c4;
    BIGINT *trace;

    UINT32 sign_of_trace;

    BIGINT *ecfp_p;
    UINT32   bgnz_md_id;

    SEAFP_MD    *seafp_md;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id = seafp_md->bgnz_md_id;
    ecfp_p     = seafp_md->ecfp_p;

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c0, LOC_SEAFP_0185);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c1, LOC_SEAFP_0186);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c2, LOC_SEAFP_0187);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c3, LOC_SEAFP_0188);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &c4, LOC_SEAFP_0189);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &trace, LOC_SEAFP_0190);
#if 0
    sys_log(LOGSTDOUT,"sea_fp_compute_order: m1: ");print_bigint_dec(LOGSTDOUT, m1);
    sys_log(LOGSTDOUT,"sea_fp_compute_order: m2: ");print_bigint_dec(LOGSTDOUT, m2);
    sys_log(LOGSTDOUT,"sea_fp_compute_order: m3: ");print_bigint_dec(LOGSTDOUT, m3);
    sys_log(LOGSTDOUT,"sea_fp_compute_order: t3: ");print_bigint_dec(LOGSTDOUT, t3);
    sys_log(LOGSTDOUT,"sea_fp_compute_order: r1: ");print_bigint_dec(LOGSTDOUT, r1);
    sys_log(LOGSTDOUT,"sea_fp_compute_order: r2: ");print_bigint_dec(LOGSTDOUT, r2);
#endif
    sign_of_trace = 0;

    if ( SEAFP_ORDER_CASE_1 == order_case )
    {
        /*here order = (p + 1 - t3) - (r1*m2*m3) -(r2*m1*m3)*/
        /*so that, trace trace = t3 + (r1*m2*m3) + (r2*m1*m3)*/

        bgn_z_mul(bgnz_md_id, r1, m2, c0, c1);
        bgn_z_mul(bgnz_md_id, r2, m1, c2, c3);

        bgn_z_add(bgnz_md_id, c0, c2, c4);
        bgn_z_mul(bgnz_md_id, c4, m3, c0, c1);

        bgn_z_add(bgnz_md_id, t3, c0, trace);

        bgn_z_sub(bgnz_md_id, ecfp_p, trace, c4);
        bgn_z_sadd(bgnz_md_id, c4, 1, unsigned_order);

        ( *carry_of_order ) = 0;
    }
    else if ( SEAFP_ORDER_CASE_2 == order_case )
    {

        /*here order = (p + 1 - t3) - (r1*m2*m3) -(r2*m1*m3) + m1*m2*m3*/
        /*so that, trace trace = t3 + (r1*m2*m3) + (r2*m1*m3) - m1*m2*m3*/

        bgn_z_mul(bgnz_md_id, r1, m2, c0, c1);
        bgn_z_mul(bgnz_md_id, r2, m1, c2, c3);

        bgn_z_add(bgnz_md_id, c0, c2, c4);
        bgn_z_mul(bgnz_md_id, c4, m3, c0, c1);

        bgn_z_add(bgnz_md_id, t3, c0, c4);

        bgn_z_mul(bgnz_md_id, m1, m2, c0, c1);
        bgn_z_mul(bgnz_md_id, c0, m3, c2, c3);

        if ( 0 > bgn_z_cmp(bgnz_md_id, c4, c2) )
        {
            sign_of_trace = 1;
            bgn_z_sub(bgnz_md_id, c2, c4, trace);
        }
        else
        {
            bgn_z_sub(bgnz_md_id, c4, c2, trace);
        }

        if ( 1 ==  sign_of_trace)
        {
            if ( 0 < bgn_z_add(bgnz_md_id, ecfp_p, trace, c4) )
            {
                ( *carry_of_order ) = 1;
            }

            if ( 0 < bgn_z_sadd(bgnz_md_id, c4, 1, unsigned_order) )
            {
                ( *carry_of_order ) = 1;
            }
        }
        else
        {
            bgn_z_sub(bgnz_md_id, ecfp_p, trace, c4);
            bgn_z_sadd(bgnz_md_id, c4, 1, unsigned_order);
            ( *carry_of_order ) = 0;
        }
    }
    else
    {
        sys_log(LOGSTDOUT,"error:sea_fp_compute_order: invalid order_case = %x.\n",order_case);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }

    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c0, LOC_SEAFP_0191);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c1, LOC_SEAFP_0192);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c2, LOC_SEAFP_0193);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c3, LOC_SEAFP_0194);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, c4, LOC_SEAFP_0195);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, trace, LOC_SEAFP_0196);

    return ( 0 );
}

UINT32 sea_fp_main(const UINT32 seafp_md_id)
{
    const POLY *poly_fai_x_y;
    POLY *poly_fai_y_is_j;
    POLY *poly_xpx;
    POLY *poly_mod_result;
    POLY *poly_gcd_result;
    BIGINT *ecfp_j_invar;
    BIGINT *Elkies_x;
    BIGINT *Elkies_m;
    BIGINT *Atkin_m1;
    BIGINT *Atkin_m2;
    BIGINT *r1;
    BIGINT *r2;
    SEAFP_ORDER_CASE_ENUM_UINT32 order_case;
    UINT32 oddsmallprime_pos;
    EC_BOOL GO_ON;

    INT32 tmod;
    INT32 Atkin_r;
    UINT32 Atkin_r_pos;

    SEAFP_ATKIN_ITEM   Atkin_item;
    SEAFP_ATKIN_TBL   *seafp_Atkin_tbl;
    SEAFP_ATKIN_SET   *seafp_Atkin_set;
    SEAFP_PRIME_TBL   *seafp_oddsmallprime_tbl;
    EC_CURVE_POINT    *test_point;
    BIGINT            *ecfp_order;
    UINT32         carry_of_ecfp_order;
    UINT32 tbl_size;
    UINT32 idx_i;
    UINT32 idx_k;
    UINT32 breakpoint;
    UINT32 depth;
    double total_cases;

    UINT32 bool_found;

    UINT32 polyfp_md_id;
    UINT32   bgnz_md_id;
    UINT32   ecfp_md_id;

    SEAFP_FAI_TBL *seafp_fai_tbl;
    SEAFP_MD    *seafp_md;

#if ( SWITCH_ON == SEA_DEBUG_SWITCH )
    if ( MAX_NUM_OF_SEAFP_MD <= seafp_md_id || 0 == g_seafp_md[ seafp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:sea_fp_main: seafp module #0x%lx not started.\n",
                seafp_md_id);
        dbg_exit(MD_SEAFP, seafp_md_id);
    }
#endif/*SEA_DEBUG_SWITCH*/
#if 0
    if ( EC_TRUE == sea_fp_ec_order_is_even(seafp_md_id))
    {
        sys_log(LOGSTDOUT," even! ");
        return ( 0 );
    }
#endif
    total_cases = 1;

    seafp_md = &(g_seafp_md[ seafp_md_id ]);
    bgnz_md_id   = seafp_md->bgnz_md_id;
    ecfp_md_id   = seafp_md->ecfp_md_id;
    polyfp_md_id = seafp_md->polyfp_md_id;
    ecfp_j_invar = seafp_md->ecfp_j_invar;
    seafp_fai_tbl= &(seafp_md->seafp_fai_tbl);
    seafp_Atkin_tbl = &(seafp_md->seafp_Atkin_tbl);
    seafp_Atkin_set = &(seafp_md->seafp_Atkin_set);
    seafp_oddsmallprime_tbl = &(seafp_md->seafp_oddsmallprime_tbl);

    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Elkies_x, LOC_SEAFP_0197);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Elkies_m, LOC_SEAFP_0198);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Atkin_m1, LOC_SEAFP_0199);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &Atkin_m2, LOC_SEAFP_0200);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &r1, LOC_SEAFP_0201);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &r2, LOC_SEAFP_0202);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(Atkin_item.prime), LOC_SEAFP_0203);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, &(test_point), LOC_SEAFP_0204);
    alloc_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, &(ecfp_order), LOC_SEAFP_0205);

    bgn_z_set_zero(bgnz_md_id, Elkies_x);
    bgn_z_set_zero(bgnz_md_id, Elkies_m);

    /*compute j-invariant*/
    ec_fp_j_invar_compute(ecfp_md_id, ecfp_j_invar);

    /*xpx = x^p - x*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_xpx);
    POLY_INIT(poly_xpx);
    sea_fp_set_xpx_to_poly(seafp_md_id, poly_xpx);

    /*initilize poly_mod_result = 0*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_mod_result);
    POLY_INIT(poly_mod_result);

    /*initilize poly_gcd_result = 0*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_gcd_result);
    POLY_INIT(poly_gcd_result);

    GO_ON = EC_TRUE;
#if 1 /*shortcut for test*/

#if 1
    tbl_size = SEAFP_FAI_EACH_TBL_SIZE;
    for ( oddsmallprime_pos = 0;
          oddsmallprime_pos < SEAFP_SMALL_PRIME_NUM && oddsmallprime_pos < tbl_size && EC_TRUE == GO_ON;
          oddsmallprime_pos ++ )
#else
    for ( oddsmallprime_pos = 23;
          oddsmallprime_pos < 24 && EC_TRUE == GO_ON;
          oddsmallprime_pos ++ )
#endif
    {
        poly_fai_x_y    = (seafp_fai_tbl->fai  [ oddsmallprime_pos ] );
        poly_fai_y_is_j = (seafp_fai_tbl->fai_j[ oddsmallprime_pos ] );

        //sys_log(LOGSTDOUT,"poly_fai_x_y:\n");
        //print_poly(LOGSTDOUT, poly_fai_x_y);
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:enter loop\n");
#endif
        /*poly_fai_y_is_j(x) = poly_fai_x_y(x, y = j)*/
        poly_fp_eval(polyfp_md_id, poly_fai_x_y, 1, ecfp_j_invar, poly_fai_y_is_j);
#if 0
        sys_log(LOGSTDOUT,"poly_fai_y_is_j:\n");
        //print_poly_dec(LOGSTDOUT, poly_fai_y_is_j);
        print_poly(LOGSTDOUT, poly_fai_y_is_j);
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:after poly_fp_eval\n");
#endif
        /*clean y-items from poly_fai_y_is_j so that poly_fai_y_is_j = poly_fai_y_is_j(x)*/
        sea_fp_poly_fai_j_clean(seafp_md_id, poly_fai_y_is_j);
#if 0
        sys_log(LOGSTDOUT,"poly_fai_y_is_j:\n");
        print_poly_dec(LOGSTDOUT, poly_fai_y_is_j);
        //print_poly(LOGSTDOUT, poly_fai_y_is_j);
        //sys_log(LOGSTDOUT,"\n");
#endif
#if 0
        sys_log(LOGSTDOUT,"poly_xpx:\n");
        print_poly_dec(LOGSTDOUT, poly_xpx);
        //print_poly(LOGSTDOUT, poly_xpx);
        //sys_log(LOGSTDOUT,"\n");
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:before poly_fp_mod_poly\n");
#endif
        /*poly_mod_result = poly_xpx mod poly_fai_y_is_j*/
        poly_fp_mod_poly(polyfp_md_id, poly_xpx, poly_fai_y_is_j, poly_mod_result);
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:after poly_fp_mod_poly\n");
#endif
#if 0
        sys_log(LOGSTDOUT,"poly_mod_result:\n");
        print_poly(LOGSTDOUT, poly_mod_result);
        //print_poly_dec(LOGSTDOUT, poly_mod_result);
#endif
        /*poly_gcd_result = GCD( poly_xpx, poly_fai_y_is_j ) = GCD( poly_mod_result, poly_fai_y_is_j )*/
        poly_fp_gcd_poly(polyfp_md_id, poly_mod_result, poly_fai_y_is_j, poly_gcd_result);
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:after poly_fp_gcd_poly\n");
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:deg(poly_gcd_result): ");
        print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_gcd_result));
#endif
#if 0
        sys_log(LOGSTDOUT,"sea_fp_main:poly_gcd_result:\n");
        print_poly_dec(LOGSTDOUT, poly_gcd_result);
        //print_poly(LOGSTDOUT, poly_gcd_result);
#endif
        if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_DEG(poly_gcd_result), 2) )
        {
            sea_fp_Elkies(seafp_md_id, seafp_oddsmallprime_tbl->pmodsmallprime, poly_gcd_result, oddsmallprime_pos, &tmod);
            sys_log(LOGSTDOUT, " [Elkies   ] " );
            sys_log(LOGSTDOUT, " t mod %ld = %ld \n", seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ], tmod );
            sea_fp_Elkies_CRT(seafp_md_id, tmod, seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ], Elkies_x, Elkies_m);
        }
        else if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_DEG(poly_gcd_result), 1) )
        {
            sys_log(LOGSTDOUT, " [Atkin_esp] " );
            sys_log(LOGSTDOUT, " t mod %ld : %ld \n",
                seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ],
                seafp_oddsmallprime_tbl->sqrtof4pmodsmallprime[ oddsmallprime_pos ] );

            /*Atkin_item.prime = oddsmallprime*/
            bgn_z_set_word(bgnz_md_id, Atkin_item.prime, seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ]);

            Atkin_item.tmodprime[ 0 ] = 2;
            Atkin_item.tmodprime[ 1 ] = (UINT32)seafp_oddsmallprime_tbl->sqrtof4pmodsmallprime[ oddsmallprime_pos ];
            Atkin_item.tmodprime[ 2 ] = (UINT32)( seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ]
                                                - seafp_oddsmallprime_tbl->sqrtof4pmodsmallprime[ oddsmallprime_pos ] );

            Atkin_item.rate = ( float ) seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ] / ( 2 * 2 * 2 * 2 );
            total_cases *= 2.0;

            sea_fp_Atkin_s_insert(seafp_md_id, &Atkin_item);

        }
        else if ( EC_TRUE == POLY_ITEM_DEG_IS_N(bgnz_md_id, POLY_DEG(poly_gcd_result), 0) )
        {
            sea_fp_Atkin_r(seafp_md_id, poly_mod_result, oddsmallprime_pos, &Atkin_r);
            //sys_log(LOGSTDOUT,"sea_fp_main: after sea_fp_Atkin_s_breakpoint_get: Atkin_r: %ld\n",Atkin_r);

            sea_fp_Atkin_r_pos(seafp_md_id, oddsmallprime_pos, Atkin_r, &Atkin_r_pos);
            //sys_log(LOGSTDOUT,"sea_fp_main: after sea_fp_Atkin_s_breakpoint_get: Atkin_r_pos: %ld\n",Atkin_r_pos);

            sys_log(LOGSTDOUT, " [Atkin    ] " );
            sys_log(LOGSTDOUT, " t mod %ld : ", seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ] );

            /*Atkin_item.prime = oddsmallprime*/
            bgn_z_set_word(bgnz_md_id, Atkin_item.prime, seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ]);
            for ( idx_k = 0, idx_i = 0; fl2rztable[ oddsmallprime_pos ][ Atkin_r_pos ][ idx_i ] != -1; idx_i ++ )
            {
                sys_log(LOGSTDOUT, "%ld,", fl2rztable[ oddsmallprime_pos ][ Atkin_r_pos ][ idx_i ] );
                Atkin_item.tmodprime[ ++ idx_k ] = ( UINT32 ) fl2rztable[ oddsmallprime_pos ][ Atkin_r_pos ][ idx_i ];
                if ( fl2rztable[ oddsmallprime_pos ][ Atkin_r_pos ][ idx_i ] )
                {
                    Atkin_item.tmodprime[ ++idx_k ] = ( UINT32 ) seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ]
                                                    - ( UINT32 ) fl2rztable[ oddsmallprime_pos ][ Atkin_r_pos ][ idx_i ];
                }
            }
            sys_log(LOGSTDOUT,"\n");
            Atkin_item.tmodprime[ 0 ] = idx_k;
            Atkin_item.rate = ( float ) seafp_oddsmallprime_tbl->oddsmallprime[ oddsmallprime_pos ] / ( idx_k * idx_k * idx_k * idx_k );
            total_cases *= ( double ) idx_k;

            sea_fp_Atkin_s_insert(seafp_md_id, &Atkin_item);
        }
        else
        {
            sys_log(LOGSTDOUT,"error:sea_fp_main: poly_gcd_result degree is more than 2.\n");
            dbg_exit(MD_SEAFP, seafp_md_id);
        }

        if ( oddsmallprime_pos == 20 && total_cases < 1000000000.0 )
        {
            GO_ON = EC_FALSE;
        }

        poly_fp_poly_clean(polyfp_md_id, poly_mod_result);
        poly_fp_poly_clean(polyfp_md_id, poly_gcd_result);
    }
#else
    sea_fp_test_init(seafp_md_id, Elkies_m, Elkies_x);
#endif /*shortcut for test*/

#if 0
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"Elkies_m :");
    print_bigint(LOGSTDOUT, Elkies_m);
    sys_log(LOGSTDOUT,"Elkies_x :");
    print_bigint(LOGSTDOUT, Elkies_x);
#endif

    sea_fp_Atkin_s_pos_modify(seafp_md_id, Elkies_m);
#if 0
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"Elkies_m :");
    print_bigint(LOGSTDOUT, Elkies_m);
    sys_log(LOGSTDOUT,"Elkies_x :");
    print_bigint(LOGSTDOUT, Elkies_x);
#endif

    sea_fp_Atkin_s_modify(seafp_md_id);

#if 0
    sys_log(LOGSTDOUT,"sea_fp_main: after collecting all Atkin primes:\n");
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"\n");

    sys_log(LOGSTDOUT,"Elkies_m :");
    print_bigint(LOGSTDOUT, Elkies_m);
    sys_log(LOGSTDOUT,"Elkies_x :");
    print_bigint(LOGSTDOUT, Elkies_x);
#endif

    sea_fp_Atkin_s_breakpoint_get(seafp_md_id, &breakpoint);
    //sys_log(LOGSTDOUT,"sea_fp_main: after sea_fp_Atkin_s_breakpoint_get: breakpoint: %ld\n",breakpoint);

    sea_fp_Atkin_s_breakpoint_modify(seafp_md_id, Atkin_m1, Atkin_m2, &breakpoint);
    //sys_log(LOGSTDOUT,"sea_fp_main: after sea_fp_Atkin_s_breakpoint_modify: breakpoint: %ld\n",breakpoint);

#if 0
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"Elkies_m :");
    print_bigint(LOGSTDOUT, Elkies_m);
    sys_log(LOGSTDOUT,"Elkies_x :");
    print_bigint(LOGSTDOUT, Elkies_x);
    sys_log(LOGSTDOUT,"Atkin_m1 :");
    print_bigint(LOGSTDOUT, Atkin_m1);
    sys_log(LOGSTDOUT,"Atkin_m2 :");
    print_bigint(LOGSTDOUT, Atkin_m2);
#endif

    /*get a point from the ec*/
    sea_fp_ec_point_gen(seafp_md_id, test_point);
#if 0
    sys_log(LOGSTDOUT,"test_point:\n");
    print_point(LOGSTDOUT, test_point);
#endif

    depth = breakpoint;
    sea_fp_baby_step (seafp_md_id, Atkin_m1, Atkin_m2, Elkies_m, Elkies_x, depth, test_point);

#if 0
    sys_log(LOGSTDOUT,"after sea_fp_baby_step: Atkin_Set_size = %ld\n", seafp_Atkin_set->Atkin_Set_size);
#endif

    sea_fp_heap_sort(seafp_md_id);

    //sea_fp_test_Atkin_set_output(seafp_md_id);
    //sea_fp_test_Atkin_set_heapsort_output(seafp_md_id, 1);

#if 0
    sys_log(LOGSTDOUT,"sea_fp_main: after sea_fp_baby_step:\n");
    sea_fp_test_Atkin_output(seafp_md_id);
    sys_log(LOGSTDOUT,"\n");

    sys_log(LOGSTDOUT,"Elkies_m :");
    print_bigint(LOGSTDOUT, Elkies_m);
    sys_log(LOGSTDOUT,"Elkies_x :");
    print_bigint(LOGSTDOUT, Elkies_x);
    sys_log(LOGSTDOUT,"Atkin_m1 :");
    print_bigint(LOGSTDOUT, Atkin_m1);
    sys_log(LOGSTDOUT,"Atkin_m2 :");
    print_bigint(LOGSTDOUT, Atkin_m2);

#endif

#if 1
    depth = seafp_Atkin_tbl->pos - breakpoint;
    //depth = seafp_Atkin_set->Atkin_Set_size - breakpoint;

    bool_found = sea_fp_giant_step(seafp_md_id, Atkin_m1, Atkin_m2, Elkies_m, Elkies_x, breakpoint, depth, test_point, r1, r2, &order_case);
    if ( EC_TRUE == bool_found )
    {
        sea_fp_compute_order(seafp_md_id, Atkin_m1, Atkin_m2, Elkies_m, Elkies_x, r1, r2, order_case, &carry_of_ecfp_order, ecfp_order);

        sys_log(LOGSTDOUT, "sea_fp_main: ecfp_order =\n");
        print_bigint_dec(LOGSTDOUT, ecfp_order);
    }
    else
    {
        sys_log(LOGSTDOUT,"sea_fp_main: match failed.\n");
    }

#endif
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Elkies_x, LOC_SEAFP_0206);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Elkies_m, LOC_SEAFP_0207);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Atkin_m1, LOC_SEAFP_0208);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, Atkin_m2, LOC_SEAFP_0209);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, r1, LOC_SEAFP_0210);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, r2, LOC_SEAFP_0211);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, (Atkin_item.prime), LOC_SEAFP_0212);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_CURVE_POINT, test_point, LOC_SEAFP_0213);
    free_static_mem(MD_SEAFP, seafp_md_id, MM_BIGINT, ecfp_order, LOC_SEAFP_0214);

    poly_fp_poly_destory(polyfp_md_id, poly_xpx);

    poly_fp_free_poly(polyfp_md_id, poly_mod_result);
    poly_fp_free_poly(polyfp_md_id, poly_gcd_result);


    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
