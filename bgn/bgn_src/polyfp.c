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

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "poly.h"

#include "bgnz.h"
#include "bgnfp.h"
#include "polyfp.h"

#include "debug.h"

#include "print.h"



static POLYFP_MD g_polyfp_md[ MAX_NUM_OF_POLYFP_MD ];
static EC_BOOL  g_polyfp_md_init_flag = EC_FALSE;

static POLY g_poly_fp_mod_pre_tbl[ MAX_NUM_OF_PRECOMP_FOR_MOD ];

static UINT32 poly_fp_adc_n(const UINT32 polyfp_md_id,const BIGINT *n, POLY *poly_c);
static UINT32 poly_fp_double_poly(const UINT32 polyfp_md_id, POLY *poly_c);
static UINT32 poly_fp_item_self_neg(const UINT32 polyfp_md_id, POLY_ITEM *item_c);
static UINT32 poly_fp_poly_self_neg(const UINT32 polyfp_md_id, POLY *poly_c);
static UINT32 poly_fp_sbb_c_a(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c);
static UINT32 poly_fp_sbb_a_c(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c);

static UINT32 poly_fp_item_insert(const UINT32 polyfp_md_id, POLY_ITEM *item_a, POLY *poly_c);
static UINT32 poly_fp_item_mul(const UINT32 polyfp_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c);
static UINT32 poly_fp_item_mul_self(const UINT32 polyfp_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c);
static UINT32 poly_fp_item_squ_self(const UINT32 polyfp_md_id, POLY_ITEM *item_c);
static UINT32 poly_fp_mul_self_bgn(const UINT32 polyfp_md_id,const BIGINT *bgn_a, POLY *poly_c);

static UINT32 poly_fp_mul_xn(const UINT32 polyfp_md_id,const POLY *poly_a, const DEGREE *deg, POLY *poly_c);
static UINT32 poly_fp_mul_xn_self(const UINT32 polyfp_md_id, const DEGREE *deg, POLY *poly_c);
static UINT32 poly_fp_mul_self(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c);

static EC_BOOL poly_fp_is_one_var_poly(const UINT32 polyfp_md_id, const POLY *poly );
static UINT32 poly_fp_mod_a_self(const UINT32 polyfp_md_id, const POLY *poly_b, POLY *poly_c );
static UINT32 poly_fp_mod_b_self(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c );

static UINT32 poly_fp_dx_self(const UINT32 polyfp_md_id, POLY *poly_c );
static UINT32 poly_fp_Dx_self(const UINT32 polyfp_md_id,const UINT32 depth_of_x, POLY *poly_c );

static UINT32 poly_fp_eval_x_self(const UINT32 polyfp_md_id,const BIGINT *bgn_n, POLY *poly_c );
static UINT32 poly_fp_eval_self(const UINT32 polyfp_md_id,const UINT32 depth_of_x, const BIGINT *bgn_n, POLY *poly_c );

static EC_BOOL poly_fp_check_valid_for_mod(const UINT32 polyfp_md_id, const POLY *poly_a, const POLY *poly_b );
static UINT32 poly_fp_xm_mod_poly(const UINT32 polyfp_md_id,const DEGREE *deg,const POLY *poly_b,POLY *poly_c );
static UINT32 poly_fp_mod_poly_simple(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c );
/**
*   for test only
*
*   to query the status of POLYFP Module
*
**/
void poly_fp_print_module_status(const UINT32 polyfp_md_id, LOG *log)
{
    POLYFP_MD *polyfp_md;
    UINT32 index;

    if ( EC_FALSE == g_polyfp_md_init_flag )
    {
        sys_log(log,"no POLYFP Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_POLYFP_MD; index ++ )
    {
        polyfp_md = &(g_polyfp_md[ index ]);

        if ( 0 < polyfp_md->usedcounter )
        {
            sys_log(log,"POLYFP Module # %ld : %ld refered, refer BGNZ Module : %ld, refer BGNFP Module : %ld\n",
                    index,
                    polyfp_md->usedcounter,
                    polyfp_md->bgnz_md_id,
                    polyfp_md->bgnfp_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed POLYFP module
*
*
**/
UINT32 poly_fp_free_module_static_mem(const UINT32 polyfp_md_id)
{
    POLYFP_MD  *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_free_module_static_mem: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    free_module_static_mem(MD_POLYFP, polyfp_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);
    bgn_fp_free_module_static_mem(bgnfp_md_id);

    return 0;
}
/**
*
* start POLYFP module
*
**/
UINT32 poly_fp_start( const BIGINT *p )
{
    POLYFP_MD *polyfp_md;
    UINT32 polyfp_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

    BIGINT *bgnfp_p;

    UINT32 index;

    /* if this is the 1st time to start POLYFP module, then */
    /* initialize g_polyfp_md */
    if ( EC_FALSE ==  g_polyfp_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_POLYFP_MD; index ++ )
        {
            polyfp_md = &(g_polyfp_md[ index ]);

            polyfp_md->usedcounter   = 0;
            polyfp_md->bgnz_md_id = ERR_MODULE_ID;
            polyfp_md->bgnfp_md_id = ERR_MODULE_ID;
        }

        /*register all functions of POLYFP module to DBG module*/
        //dbg_register_func_addr_list(g_polyfp_func_addr_list, g_polyfp_func_addr_list_len);

        g_polyfp_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_POLYFP_MD; index ++ )
    {
        polyfp_md = &(g_polyfp_md[ index ]);

        if ( 0 == polyfp_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_POLYFP_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;
    bgnfp_md_id = ERR_MODULE_ID;

    /* initilize new one POLYFP module */
    polyfp_md_id = index;
    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);

    init_static_mem();

    bgnz_md_id = bgn_z_start();
    bgnfp_md_id = bgn_fp_start( p );

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgnfp_p  , LOC_POLYFP_0001);
    bgn_z_clone(bgnz_md_id, p, bgnfp_p);

    polyfp_md->bgnz_md_id = bgnz_md_id;
    polyfp_md->bgnfp_md_id = bgnfp_md_id;
    polyfp_md->bgnfp_p = bgnfp_p;
    polyfp_md->usedcounter = 1;

    for ( index = 0; index < MAX_NUM_OF_PRECOMP_FOR_MOD; index ++ )
    {
        POLY_INIT(&(g_poly_fp_mod_pre_tbl[ index ]));
    }

    return ( polyfp_md_id );
}

/**
*
* end POLYFP module
*
**/
void poly_fp_end(const UINT32 polyfp_md_id)
{
    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

    BIGINT *bgnfp_p;

    UINT32 index;

    if ( MAX_NUM_OF_POLYFP_MD < polyfp_md_id )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_end: polyfp_md_id = %ld is overflow.\n",polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < polyfp_md->usedcounter )
    {
        polyfp_md->usedcounter --;
        return ;
    }

    if ( 0 == polyfp_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_end: polyfp_md_id = %ld is not started.\n",polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    for ( index = 0; index < MAX_NUM_OF_PRECOMP_FOR_MOD; index ++ )
    {
        poly_fp_poly_clean(polyfp_md_id, &(g_poly_fp_mod_pre_tbl[ index ]));
    }

    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;
    bgnfp_p = polyfp_md->bgnfp_p;

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgnfp_p  , LOC_POLYFP_0002);

    bgn_z_end( bgnz_md_id);
    bgn_fp_end( bgnfp_md_id);

    /* free module : */
    //poly_fp_free_module_static_mem(polyfp_md_id);
    polyfp_md->bgnz_md_id = ERR_MODULE_ID;
    polyfp_md->bgnfp_md_id = ERR_MODULE_ID;
    polyfp_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

/**
*
*   alloc a BIGINT type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_bgn(const UINT32 polyfp_md_id, BIGINT **ppbgn)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == ppbgn )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_alloc_bgn: ppbgn is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_alloc_bgn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, ppbgn, LOC_POLYFP_0003);
    return (0);
}
/**
*
*   alloc a DEGREE type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_deg(const UINT32 polyfp_md_id, DEGREE **ppdeg)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == ppdeg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_alloc_deg: ppdeg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_alloc_deg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, ppdeg, LOC_POLYFP_0004);
    return (0);
}
/**
*
*   alloc a POLY_ITEM type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_item(const UINT32 polyfp_md_id,POLY_ITEM **ppitem)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == ppitem )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_alloc_item: ppitem is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_alloc_item: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, ppitem, LOC_POLYFP_0005);
    return (0);
}
/**
*
*   alloc a POLY type node from POLYFP space
*
**/
UINT32 poly_fp_alloc_poly(const UINT32 polyfp_md_id,POLY **pppoly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == pppoly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_alloc_poly: pppoly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_alloc_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, pppoly, LOC_POLYFP_0006);
    POLY_INIT(*pppoly);

    return (0);
}

/**
*
*   free the BIGINT type node from POLYFP space
*
**/
UINT32 poly_fp_free_bgn(const UINT32 polyfp_md_id,BIGINT *bgn)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_free_bgn: bgn is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_free_bgn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn, LOC_POLYFP_0007);
    return (0);
}
/**
*
*   free the DEGREE type node from POLYFP space
*
**/
UINT32 poly_fp_free_deg(const UINT32 polyfp_md_id,DEGREE *deg)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == deg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_free_deg: deg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_free_deg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, deg, LOC_POLYFP_0008);
    return (0);
}
/**
*
*   free the POLY_ITEM type node from POLYFP space
*
**/
UINT32 poly_fp_free_item(const UINT32 polyfp_md_id,POLY_ITEM *item)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_free_item: item is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_free_item: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, item, LOC_POLYFP_0009);
    return (0);
}
/**
*
*   free the POLY type node from POLYFP space
*
**/
UINT32 poly_fp_free_poly(const UINT32 polyfp_md_id,POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_free_poly: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_free_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly, LOC_POLYFP_0010);
    return (0);
}

/**
*
*   destory the whole item,including its content(deg, coe) and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
UINT32 poly_fp_item_destory(const UINT32 polyfp_md_id, POLY_ITEM *item)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_destory: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
    {
        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item), LOC_POLYFP_0011);
        POLY_ITEM_BGN_COE(item) = NULL_PTR;
    }
    else
    {
        poly_fp_poly_destory(polyfp_md_id, POLY_ITEM_POLY_COE(item));
        POLY_ITEM_POLY_COE(item) = NULL_PTR;
    }

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, item, LOC_POLYFP_0012);

    return ( 0 );
}

/**
*
*   destory the whole poly,i.e., all its items but not the poly itself.
*   so, when return from this function, poly can be refered again without any item.
*
**/
UINT32 poly_fp_poly_clean(const UINT32 polyfp_md_id, POLY *poly)
{
    POLY_ITEM *item;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_clean: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_clean: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item = POLY_FIRST_ITEM(poly);

    while ( item != POLY_NULL_ITEM(poly) )
    {
        POLY_ITEM_DEL(item);
        poly_fp_item_destory(polyfp_md_id, item);

        item = POLY_FIRST_ITEM(poly);
    }

    POLY_INIT(poly);

    return ( 0 );
}

/**
*
*   destory the whole poly,including its all items and itself.
*   so, when return from this function, poly cannot be refered any more
*
**/
UINT32 poly_fp_poly_destory(const UINT32 polyfp_md_id, POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_destory: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_destory: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    poly_fp_poly_clean(polyfp_md_id,poly);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly, LOC_POLYFP_0013);

    return ( 0 );
}

UINT32 poly_fp_poly_clone(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_clone: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_clone: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_clone: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
    }
    //POLY_INIT(poly_c);

    item_a = POLY_FIRST_ITEM(poly_a);

    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0014);

        poly_fp_item_clone(polyfp_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return (0);
}

UINT32 poly_fp_item_clone(const UINT32 polyfp_md_id,const POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    BIGINT  *bgn_coe_of_item_c;
    POLY    *poly_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_clone: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_clone: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_clone: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    POLY_ITEM_INIT(item_c);

    /*clone degree of item*/
    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    /*clone coefficient of item*/
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
    {
        /*clone bgn coe*/
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0015);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
    }
    else
    {
        /*clone poly coe*/
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0016);
        POLY_INIT(poly_coe_of_item_c);

        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;

        poly_fp_poly_clone(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
    }

    return (0);
}

/**
*
*   set poly = 0
*
**/
UINT32 poly_fp_set_zero(const UINT32 polyfp_md_id,POLY *poly)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_zero: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_zero: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( EC_TRUE == POLY_IS_EMPTY(poly) )
    {
        return ( 0 );
    }

    return poly_fp_poly_clean(polyfp_md_id, poly);
}

/**
*
*   set poly = 1
*
**/
UINT32 poly_fp_set_one(const UINT32 polyfp_md_id,POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_one: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_one: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0017);
    alloc_static_mem (MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0018);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = 1*/
    bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}

/**
*
*   set poly = n
*
**/
UINT32 poly_fp_set_word(const UINT32 polyfp_md_id,const UINT32 n, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_word: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_word: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0019);
    alloc_static_mem (MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0020);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = 1*/
    bgn_fp_set_word(bgnfp_md_id, bgn_coe_of_item, n);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}

/**
*
*   set poly = n
*
**/
UINT32 poly_fp_set_n(const UINT32 polyfp_md_id,const BIGINT *n, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_n: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_n: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0021);
    alloc_static_mem (MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0022);

    /*deg(item) = 0*/
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item));

    /*bgn_coe(item) = n*/
    bgn_fp_clone(bgnfp_md_id, n, bgn_coe_of_item);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}


/**
*
*   set poly = x^deg
*
**/
UINT32 poly_fp_set_xn(const UINT32 polyfp_md_id,const DEGREE *deg, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == deg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_xn: deg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_xn: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_xn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0023);
    alloc_static_mem (MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0024);

    /*bgn_coe(item) = 1*/
    bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

    /*deg(item) = deg*/
    POLY_ITEM_DEG_CLONE(bgnz_md_id, deg, POLY_ITEM_DEG(item));

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}

/**
*
*   set poly = x^deg
*
**/
UINT32 poly_fp_set_xn_word(const UINT32 polyfp_md_id,const UINT32 deg, POLY *poly)
{
    POLY_ITEM *item;
    BIGINT *bgn_coe_of_item;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_set_xn_word: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_set_xn_word: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_FALSE == POLY_IS_EMPTY(poly) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0025);
    alloc_static_mem (MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0026);

    /*bgn_coe(item) = 1*/
    bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

    /*deg(item) = deg*/
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, POLY_ITEM_DEG(item), deg);

    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    POLY_ADD_ITEM_HEAD(poly, item);

    return ( 0 );
}

EC_BOOL poly_fp_item_cmp(const UINT32 polyfp_md_id,const POLY_ITEM *item_a, const POLY_ITEM *item_b)
{
    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_cmp: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_cmp: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_cmp: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( 0 != POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b)) )
    {
        return (EC_FALSE);
    }

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        if ( 0 != bgn_fp_cmp(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b)))
        {
            return (EC_FALSE);
        }
        return (EC_TRUE);
    }
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        return poly_fp_poly_cmp(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b));
    }

    return (EC_FALSE);
}
/**
*
*   if poly_a = poly_b then return EC_TRUE
*   else    return EC_FALSE
*
**/
EC_BOOL poly_fp_poly_cmp(const UINT32 polyfp_md_id,const POLY *poly_a, const POLY *poly_b)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_cmp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_cmp: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_cmp: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b) )
    {
        if ( EC_FALSE == poly_fp_item_cmp(polyfp_md_id, item_a, item_b) )
        {
            return (EC_FALSE);
        }

        item_a = POLY_ITEM_NEXT(item_a);
        item_b = POLY_ITEM_NEXT(item_b);
    }

    if ( item_a != POLY_NULL_ITEM(poly_a) || item_b != POLY_NULL_ITEM(poly_b)  )
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/**
*
*Function: poly_c = poly_c + n
*
*poly_adc_const := proc(poly_c, n)
*
*item_c = first_item(poly_c);
*
*if deg(item_c) is zero then
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) + n;
*    end if
*    if FALSE == bgn_coe_flag(item_c) then
*
*        poly_adc_const(poly_coe(item_c), n);
*    end if
*end if
*
*if deg(item_c) is nonzero then
*    new tmp_item_c
*    deg(tmp_item_c) = 0;
*    bgn_coe(tmp_item_c) = n;
*    bgn_coe_flag = TRUE;
*    insert tmp_item_c to the head of poly_c;
*end if
*
**/
static UINT32 poly_fp_adc_n(const UINT32 polyfp_md_id,const BIGINT *n, POLY *poly_c)
{
    POLY_ITEM *item_c;
    BIGINT  *bgn_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_adc_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_adc_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_adc_n: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);

    /*
    *if deg(item_c) is zero then
    *    if TRUE == bgn_coe_flag(item_c) then
    *        bgn_coe(item_c) = bgn_coe(item_c) + n;
    *    end if
    *    if FALSE == bgn_coe_flag(item_c) then
    *
    *        poly_adc_const(poly_coe(item_c), n);
    *    end if
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO( bgnz_md_id, POLY_ITEM_DEG(item_c)))
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_fp_add(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), n, POLY_ITEM_BGN_COE(item_c));
        }
        else
        {
            poly_fp_adc_n(polyfp_md_id, n, POLY_ITEM_POLY_COE(item_c));
        }
    }
    /*
    *if deg(item_c) is nonzero then
    *    new tmp_item_c
    *    deg(tmp_item_c) = 0;
    *    bgn_coe(tmp_item_c) = n;
    *    bgn_coe_flag = TRUE;
    *    insert tmp_item_c to the head of poly_c;
    *end if
    */
    else
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0027);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0028);
        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        bgn_fp_clone(bgnfp_md_id, n, POLY_ITEM_BGN_COE(item_c));

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);
    }
    return (0);
}


/**
*
*Function: poly_c = poly_a + n
*
*poly_add_n := proc(poly_a, n, poly_c)
*
*if poly_a != poly_c then
*    clone poly_a to poly_c
*end if
*
*poly_adc_n(poly_c, n);
*
*end proc;
*
**/
UINT32 poly_fp_add_n(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *n, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_n: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_n: n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_n: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_add_n: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        /*clone poly_a to poly_c*/
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
    }

    poly_fp_adc_n(polyfp_md_id, n, poly_c);

    return (0);
}

/**
*
*Function: poly_c = poly_c + poly_c
*poly_double:= proc(poly_c)
*
*item_c = first_item(poly_c);
*while item_c != null_item(poly_c) do
*
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_c);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_double(poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_c = next_item(poly_c, item_c);
*
*end do;
*end proc;
*
**/
static UINT32 poly_fp_double_poly(const UINT32 polyfp_md_id, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_double_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_double_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);

    while( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
            bgn_fp_add(bgnfp_md_id, bgn_coe_of_item_c, bgn_coe_of_item_c, bgn_coe_of_item_c);

            if ( 0 == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
            else
            {
                item_c = POLY_ITEM_NEXT(item_c);
            }
        }
        else
        {
            poly_fp_double_poly(polyfp_md_id,POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY( POLY_ITEM_POLY_COE(item_c) ) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
            else
            {
                item_c = POLY_ITEM_NEXT(item_c);
            }
        }
    }

    return ( 0 );
}

/**
*
*poly_c = poly_c + poly_a
*poly_adc:= proc(poly_a, poly_c)
*
*if poly_a == poly_c then
* poly_double(poly_c)
*end if
*
* >>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_a) + bgn_coe(item_c);
*
* >>> to judge the carry flag of the addition operation seems better
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_add_const(poly_coe(item_c), bgn_coe(item_a), poly_coe(item_c));
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_add(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
*    insert tmp_item_cto the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*
*end proc;
*
**/
UINT32 poly_fp_adc_poly(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;
    POLY_ITEM   *item_t;
    POLY        *poly_coe_of_item_c;
    int cmp_result;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_adc_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_adc_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_adc_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_fp_double_poly(polyfp_md_id, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP( bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_a) + bgn_coe(item_c);
            *
            * >>> to judge the carry flag of the addition operation seems better
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                bgn_fp_add(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

                if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_add_const(poly_coe(item_c), bgn_coe(item_a), poly_coe(item_c));
            *
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_fp_add_n(polyfp_md_id,POLY_ITEM_POLY_COE(item_c), POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0029);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_add_n(polyfp_md_id,POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

                free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYFP_0030);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;

                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_add(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            */
            else if( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_fp_add_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));

                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }

        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( -1 == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0031);
            poly_fp_item_clone(polyfp_md_id, item_a, item_t);

            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }

    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    clone item_a to tmp_item_c(note: here clone item including the sub poly coe if exist)
    *    insert tmp_item_cto the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0032);

        poly_fp_item_clone(polyfp_md_id, item_a, item_t);

        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
*poly_add:= proc(poly_a, poly_b, poly_c)
*
*if poly_c == poly_a then
*poly_adc(poly_b,poly_c)
*end if
*
*if poly_c == poly_b then
*poly_adc(poly_a,poly_c)
*end if
*
*
*>>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_b = first_item(poly_b);
*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do
*
*if deg(item_a) == deg(item_b) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*        bgn_coe_c = bgn_coe(item_a) + bgn_coe(item_b);
*
*        if bgn_coe_c is nonzero then
*            new item_c
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_c;
*            bgn_coe_flag(item_c) = TRUE;
*            insert item_c to tail of poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_b), bgn_coe(item_a), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_b);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        poly_add_const(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        poly_add(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_b = next_item(poly_b, item_b);
*end if;
*
*if deg(item_a) < deg(item_b) then
*    new item_c
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_b) then
*    new item_c
*    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new item_c
*
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_b != null_item(poly_b) do
*    new item_c
*
*    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end do
*
*
*end proc;
*
**/
UINT32 poly_fp_add_poly(const UINT32 polyfp_md_id,const POLY *poly_a, const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_b;
    POLY_ITEM   *item_c;
    POLY        *poly_coe_of_item_c;
    BIGINT      *bgn_coe_of_item_c;
    int cmp_result;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_add_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_add_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_c == poly_a )
    {
        return poly_fp_adc_poly(polyfp_md_id, poly_b, poly_c);
    }

    if ( poly_c == poly_b )
    {
        return poly_fp_adc_poly(polyfp_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_fp_poly_clean(polyfp_md_id, poly_c);

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b));

        /*if deg(item_a) == deg(item_b) then*/
        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
            *        bgn_coe_c = bgn_coe(item_a) + bgn_coe(item_b);
            *
            *        if bgn_coe_c is nonzero then
            *            new item_c
            *            deg(item_c) = deg(item_a);
            *            bgn_coe(item_c) = bgn_coe_c;
            *            bgn_coe_flag(item_c) = TRUE;
            *            insert item_c to tail of poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0033);

                bgn_fp_add(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

                if ( EC_FALSE == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c) )
                {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0034);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
                else
                {
                    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0035);
                    bgn_coe_of_item_c = NULL_PTR;
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_b), bgn_coe(item_a), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_b);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0036);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_fp_add_n(polyfp_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0037);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        poly_add_const(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0038);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_fp_add_n(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0039);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        poly_add(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b))
            {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0040);
                    POLY_INIT(poly_coe_of_item_c);

                    poly_fp_add_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);
                    if ( EC_FALSE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                    {
                        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0041);

                        POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                        POLY_ADD_ITEM_TAIL(poly_c, item_c);
                    }
                    else
                    {
                        free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0042);
                        poly_coe_of_item_c = NULL_PTR;
                    }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_b = next_item(poly_b, item_b);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_b = POLY_ITEM_NEXT(item_b);
        }
        /*
        *if deg(item_a) < deg(item_b) then
        *    new item_c
        *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( -1 == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0043);

            poly_fp_item_clone(polyfp_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_b) then
        *    new item_c
        *    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_b = next_item(poly_b, item_b);
        *end if;
        */
        if ( 1 == cmp_result )
        {

            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0044);

            poly_fp_item_clone(polyfp_md_id, item_b, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);

            item_b = POLY_ITEM_NEXT(item_b);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new item_c
    *
    *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0045);

        poly_fp_item_clone(polyfp_md_id, item_a, item_c);
        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_b != null_item(poly_b) do
    *    new item_c
    *
    *    clone item_b to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_b = next_item(poly_b, item_b);
    *end do
    */
    while( item_b != POLY_NULL_ITEM(poly_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0046);

        poly_fp_item_clone(polyfp_md_id, item_b, item_c);
        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_b = POLY_ITEM_NEXT(item_b);
    }

    return ( 0 );
}


/**
*
*poly_c = poly_a - bgn_b
*
*poly_sbb_const_2 :=proc(poly_a, bgn_b,poly_c)
*
*if bgn_b is zero then
* return;
*end if
*
*TODO: poly_c = poly_c - bgn_b
*
*item_c = first_item(poly_c);
*
*if item_c = null_item(poly_c) or deg(item_c) is nonzero then
*    new item_c
*
*    deg(item_c) = 0;
*    bgn_coe(item_c) = -bgn_b;
*    bgn_coe_flag(item_c) = TRUE;
*    insert item_c to the head of poly_c
*endif
*
*if deg(item_c) is zero  then
*
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) - bgn_b;
*        if 0 == deg(item_c) then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    endif
*
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_sbb_const_2(poly_coe(item_c), bgn_b,poly_coe(item_c));
*    endif
*end if
*
*end proc
*
**/
UINT32 poly_fp_sub_poly_bgn(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *bgn_b, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT *bgn_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly_bgn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly_bgn: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sub_poly_bgn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a != poly_c then
    *   poly_clone(poly_a, poly_c);
    *end if
    */
    if ( poly_a != poly_c )
    {
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *if bgn_b is zero then
    *   return;
    *end if
    */
    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_b))
    {
        return ( 0 );
    }

    /*TODO: poly_c = poly_c - bgn_b*/
    /*
    *item_c = first_item(poly_c);
    *
    *if item_c = null_item(poly_c) or deg(item_c) is nonzero then
    *    new item_c
    *
    *    deg(item_c) = 0;
    *    bgn_coe(item_c) = -bgn_b;
    *    bgn_coe_flag(item_c) = TRUE;
    *    insert item_c to the head of poly_c
    *endif
    */
    item_c = POLY_FIRST_ITEM(poly_c);
    if ( item_c == POLY_NULL_ITEM(poly_c) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0047);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0048);

        bgn_fp_neg(bgnfp_md_id, bgn_b, bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);

        return ( 0 );
    }

    if ( EC_FALSE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c)))
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0049);

        POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0050);

        bgn_fp_neg(bgnfp_md_id, bgn_b, bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;

        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

        POLY_ADD_ITEM_HEAD(poly_c, item_c);

        return ( 0 );
    }
    /* now deg(item_c) is zero */
    /*
    *
    *    if TRUE == bgn_coe_flag(item_c) then
    *        bgn_coe(item_c) = bgn_coe(item_c) - bgn_b;
    *        if 0 == bgn_coe(item_c) then
    *            item_c = prev_item(poly_c, item_c)
    *            remove next_item(poly_c,item_c) from poly_c
    *        end if
    *    endif
    *
    *    if FALSE == bgn_coe_flag(item_c) then
    *        poly_sbb_const_2(poly_coe(item_c), bgn_b,poly_coe(item_c));
    *    endif
    *
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_fp_sub(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), bgn_b, POLY_ITEM_BGN_COE(item_c));

        if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)))
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);
            poly_fp_item_destory(polyfp_md_id, item_t);
        }
        return ( 0 );
    }

    return poly_fp_sub_poly_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_c), bgn_b, POLY_ITEM_POLY_COE(item_c));
}


/**
*poly_c = bgn_a - poly_b
*
*poly_sbb_const_1 :=proc(bgn_a, poly_b,poly_c)
*
*if poly_b == poly_c then
*   poly_neg_0(poly_c);
*   return poly_adc_const(bgn_a,poly_c);
*end if
*
*poly_neg(poly_b,poly_c);
*poly_adc_const(bgn_a,poly_c);
*
*end proc
*
**/
UINT32 poly_fp_sub_bgn_poly(const UINT32 polyfp_md_id,const BIGINT *bgn_a, const POLY *poly_b, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_bgn_poly: bgn_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_bgn_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_bgn_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sub_bgn_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    poly_fp_poly_neg(polyfp_md_id, poly_b, poly_c);
    return poly_fp_adc_n(polyfp_md_id, bgn_a, poly_c);
}

/**
*item = - item
*
*poly_item_neg_0 := proc(item)
*
*if TRUE == bgn_coe_flag(item) then
*   bgn_coe(item) = -bgn_coe(item);
*end if
*
*if FALSE == bgn_coe_flag(item) then
*   poly_neg_0(poly_coe(item));
*end if
*
*end proc
**/
static UINT32 poly_fp_item_self_neg(const UINT32 polyfp_md_id, POLY_ITEM *item_c)
{
    POLYFP_MD *polyfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_self_neg: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_self_neg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_fp_neg(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));
        return ( 0 );
    }
    else
    {
        return poly_fp_poly_self_neg(polyfp_md_id, POLY_ITEM_POLY_COE(item_c));
    }
    return ( 0 );
}

/*
*item_c = - item_a
*
*poly_item_neg := proc(item_a, item_c)
*
*if item_a == item_c then
*   poly_item_neg_0(item_c);
*   return;
*end if
*
*new deg(item_c)
*deg(item_c) = deg(item_a);
*
*if TRUE == bgn_coe_flag(item_a) then
*   new bgn_coe(item_c)
*   bgn_coe(item_c) = bgn_coe_of_item_c;
*   bgn_coe_flag(item_c) = TRUE
*end if
*
*if FALSE == bgn_coe_flag(item_a) then
*   new poly_coe(item_c)
*   poly_neg(poly_coe(item_a),poly_coe(item_c));
*
*   bgn_coe_flag(item_c) = FALSE
*end if
*
*end proc
**/
UINT32 poly_fp_item_neg(const UINT32 polyfp_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c)
{
    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_neg: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_neg: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_neg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( item_a == item_c )
    {

        return poly_fp_item_self_neg(polyfp_md_id, item_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *new deg(item_c)
    *deg(item_c) = deg(item_a);
    */
    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    /*
    *if TRUE == bgn_coe_flag(item_a) then
    *   new bgn_coe(item_c)
    *   bgn_coe(item_c) = bgn_coe_of_item_c;
    *   bgn_coe_flag(item_c) = TRUE
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a))
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0051);
        bgn_fp_neg(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), bgn_coe_of_item_c);

        POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) then
    *   new poly_coe(item_c)
    *   poly_neg(poly_coe(item_a),poly_coe(item_c));
    *
    *   bgn_coe_flag(item_c) = FALSE
    *end if
    */
    else
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0052);
        POLY_INIT(poly_coe_of_item_c);

        poly_fp_poly_neg(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), poly_coe_of_item_c);
        POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
    }

    return ( 0 );
}

/**
*poly_c = - poly_c
*
*poly_neg_0 := proc(poly_c)
*
*item_c = first_item(poly_c);
*
*while item_c != null_item(poly_c) do
*   poly_item_neg_0(item_c);
*   item_c = next_item(poly_c, item_c);
*end do
*
*end proc
**/
static UINT32 poly_fp_poly_self_neg(const UINT32 polyfp_md_id, POLY *poly_c)
{
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_self_neg: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_self_neg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item_c = POLY_FIRST_ITEM(poly_c);
    while( item_c != POLY_NULL_ITEM(poly_c) )
    {
        poly_fp_item_self_neg(polyfp_md_id, item_c);

        item_c = POLY_ITEM_NEXT(item_c);
    }
    return ( 0 );
}

/**
*poly_c = - poly_a
*
*poly_neg := proc(poly_a, poly_c)
*
*if poly_a ==  poly_c then
*   poly_neg_0(poly_c);
*   return ;
*end if
*
*item_a = first_item(poly_a)
*
*while item_a != null_item(poly_a) do
*   new item_c
*   poly_item_neg(item_a, item_c);
*
*   insert item_c to the tail of poly_c
*   item_a = next_item(poly_a, item_a);
*
*end do
*
*end proc
**/
UINT32 poly_fp_poly_neg(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_neg: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_neg: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_neg: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_fp_poly_self_neg(polyfp_md_id, poly_c);
    }
    /*
    *item_a = first_item(poly_a)
    *
    *while item_a != null_item(poly_a) do
    *   new item_c
    *   poly_item_neg(item_a, item_c);
    *
    *   insert item_c to the tail of poly_c
    *   item_a = next_item(poly_a, item_a);
    *
    *end do
    */

    item_a = POLY_FIRST_ITEM(poly_a);
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0053);

        poly_fp_item_neg(polyfp_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    return ( 0 );
}

/**
*poly_c = poly_c - poly_a
*
*poly_sbb_1:= proc(poly_a, poly_c)
*
*if poly_a == poly_c then
*   poly_destory(poly_c)
*end if
*
*>>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) - bgn_coe(item_a);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_sbb_const_2(poly_coe(item_c),bgn_coe(item_a), poly_coe(item_c));
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_sbb_const_1(bgn_coe(item_c), poly_coe(item_a), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_sub(poly_coe(item_c), poly_coe(item_a), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    poly_item_neg(item_a,item_c)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    poly_item_neg(item_a,item_c)
*    insert tmp_item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*end proc;
**/
static UINT32 poly_fp_sbb_c_a(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;

    UINT32 cmp_result;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sbb_c_a: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sbb_c_a: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sbb_c_a: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *   poly_destory(poly_c)
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_poly_clean(polyfp_md_id, poly_c);
    }


    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /* from lower degree item to higher degree item */

    /*
    *item_a = first_item(poly_a);
    *item_c = first_item(poly_c);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_c) - bgn_coe(item_a);
            *
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                bgn_fp_sub(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c));

                if (EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_sbb_const_2(poly_coe(item_c),bgn_coe(item_a), poly_coe(item_c));
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                poly_fp_sbb_c_a(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_sbb_const_1(bgn_coe(item_c), poly_coe(item_a), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0054);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_sub_bgn_poly(polyfp_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_POLY_COE(item_a), poly_coe_of_item_c);

                bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0055);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_sub(poly_coe(item_c), poly_coe(item_a), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            *
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
                poly_fp_sub_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));
                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            else
            {
                ;
            }

            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }
        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    poly_item_neg(item_a,item_c)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0056);

            poly_fp_item_neg(polyfp_md_id, item_a, item_t);
            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        *
        *end do;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    poly_item_neg(item_a,item_c)
    *    insert tmp_item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    *
    **/
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0057);

        poly_fp_item_neg(polyfp_md_id, item_a, item_t);
        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*poly_c = poly_a - poly_c
*
*poly_sbb_2 :=proc(poly_a,poly_c)
*
*if poly_a == poly_c then
*   poly_destory(poly_c)
*end if
*
* from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_c = first_item(poly_c);
*
*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do
*
*if deg(item_a) == deg(item_c) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_a) - bgn_coe(item_c);
*
*        if bgn_coe(item_c) is zero then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
*        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_c),poly_coe(item_c));
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
*        new tmp_poly_c
*        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
*
*        del bgn_coe(item_c)
*
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*        poly_sub(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_c = prev_item(poly_c, item_c)
*            remove next_item(poly_c,item_c) from poly_c
*        end if
*
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_c = next_item(poly_c, item_c);
*end if;
*
*if deg(item_a) < deg(item_c) then
*    new tmp_item_c
*
*    poly_item_clone(item_a,tmp_item_c)
*    prev_item(poly_c,item_c) = tmp_item_c;
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_c) then
*    item_c = next_item(poly_c, item_c);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new tmp_item_c
*
*    poly_item_clone(item_a,tmp_item_c)
*    insert tmp_item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_c != null_item(poly_c) do
*
*    poly_item_neg(item_c,item_c);
*
*    item_c = next_item(poly_c, item_c);
*end do
*end proc
*
**/
static UINT32 poly_fp_sbb_a_c(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;
    UINT32 cmp_result;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sbb_a_c: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sbb_a_c: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sbb_a_c: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *   poly_destory(poly_c)
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_poly_clean(polyfp_md_id, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /* from lower degree item to higher degree item */
    /*
    *item_a = first_item(poly_a);
    *item_c = first_item(poly_c);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_c = POLY_FIRST_ITEM(poly_c);

    /*while item_a != null_item(poly_a) && item_c != null_item(poly_c) do*/
    while( item_a != POLY_NULL_ITEM(poly_a) && item_c != POLY_NULL_ITEM(poly_c) )
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));
        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
            *        bgn_coe(item_c) = bgn_coe(item_a) - bgn_coe(item_c);
            *
            *        if bgn_coe(item_c) is zero then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                bgn_fp_sub(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

                if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c)
            *        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_c),poly_coe(item_c));
            *
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_fp_sub_bgn_poly(polyfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c)
            *        new tmp_poly_c
            *        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_c), tmp_poly_c);
            *
            *        del bgn_coe(item_c)
            *
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0058);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_sub_poly_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

                bgn_coe_of_item_c = POLY_ITEM_BGN_COE(item_c);
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0059);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
            *        poly_sub(poly_coe(item_a), poly_coe(item_c), poly_coe(item_c));
            *
            *        if poly_coe(item_c) is empty then
            *            item_c = prev_item(poly_c, item_c)
            *            remove next_item(poly_c,item_c) from poly_c
            *        end if
            *
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
            {
                poly_fp_sub_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c), POLY_ITEM_POLY_COE(item_c));

                if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);
                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_c = next_item(poly_c, item_c);
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_c = POLY_ITEM_NEXT(item_c);
        }
        /*
        *if deg(item_a) < deg(item_c) then
        *    new tmp_item_c
        *
        *    poly_item_clone(item_a,tmp_item_c)
        *    prev_item(poly_c,item_c) = tmp_item_c;
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0060);

            poly_fp_item_clone(polyfp_md_id, item_a, item_t);
            POLY_ITEM_ADD_PREV(item_c, item_t);

            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_c) then
        *    item_c = next_item(poly_c, item_c);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            item_c = POLY_ITEM_NEXT(item_c);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new tmp_item_c
    *
    *    poly_item_clone(item_a,tmp_item_c)
    *    insert tmp_item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0061);
        poly_fp_item_clone(polyfp_md_id, item_a, item_t);

        POLY_ADD_ITEM_TAIL(poly_c, item_t);

        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_c != null_item(poly_c) do
    *
    *    poly_item_neg(item_c,item_c);
    *
    *    item_c = next_item(poly_c, item_c);
    *end do
    */
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        poly_fp_item_neg(polyfp_md_id, item_c, item_c);

        item_c = POLY_ITEM_NEXT(item_c);
    }
    return ( 0 );
}

/*
*poly_sub:= proc(poly_a, poly_b, poly_c)
*
*if poly_c == poly_a then
*     c = c - b
*poly_sbb_1(poly_b,poly_c)
*end if
*
*if poly_c == poly_b then
*     c = a - c
*poly_sbb_2(poly_a,poly_c)
*end if
*
*
*>> from lower degree item to higher degree item
*item_a = first_item(poly_a);
*item_b = first_item(poly_b);
*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do
*
*if deg(item_a) == deg(item_b) then
*    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*        bgn_coe_c = bgn_coe(item_a) - bgn_coe(item_b);
*
*        if bgn_coe_c is nonzero then
*            new item_c
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_c;
*            bgn_coe_flag(item_c) = TRUE;
*            insert item_c to tail of poly_c
*        end if
*    end if
*
*    if TRUE == bgn_coe_flag(item_a)  and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        >>>tmp_poly_c = bgn_coe(item_a) - poly_coe(item_b)
*        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_b),tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a)  and TRUE == bgn_coe_flag(item_b)
*        new tmp_poly_c
*        >>> tmp_poly_c = poly_coe(item_a) bgn_coe(item_b)
*        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*        new tmp_poly_c
*        poly_sub(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
*
*        new item_c
*        deg(item_c) = deg(item_a);
*        poly_coe(item_c) = tmp_poly_c;
*        bgn_coe_flag(item_c) = FALSE;
*        insert item_c to tail of poly_c
*    end if
*
*    item_a = next_item(poly_a, item_a);
*    item_b = next_item(poly_b, item_b);
*end if;
*
*if deg(item_a) < deg(item_b) then
*    new item_c
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end if;
*
*if deg(item_a) > deg(item_b) then
*    new item_c
*    poly_item_neg(item_b, item_c)
*
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end if;
*
*
*end do;
*
*while item_a != null_item(poly_a) do
*    new item_c
*
*    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
*    insert item_c to the tail of poly_c
*    item_a = next_item(poly_a, item_a);
*end do
*
*while item_b != null_item(poly_b) do
*    new item_c
*
*    poly_item_neg(item_b, item_c)
*
*    insert item_c to the tail of poly_c
*    item_b = next_item(poly_b, item_b);
*end do
*
*end proc;
*/
UINT32 poly_fp_sub_poly(const UINT32 polyfp_md_id, const POLY *poly_a, const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;

    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;

    UINT32 cmp_result;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sub_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sub_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_c == poly_a then
    *     c = c - b
    *poly_sbb_1(poly_b,poly_c)
    *end if
    */
    if ( poly_c == poly_a )
    {
        return poly_fp_sbb_c_a(polyfp_md_id, poly_b, poly_c);
    }

    /*
    *if poly_c == poly_b then
    *     c = a - c
    *poly_sbb_2(poly_a,poly_c)
    *end if
    */
    if ( poly_c == poly_b )
    {
        return poly_fp_sbb_a_c(polyfp_md_id, poly_a, poly_c);
    }

    /*clean all items of the poly_c*/
    poly_fp_poly_clean(polyfp_md_id, poly_c);

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /* from lower degree item to higher degree item */

    /*
    *item_a = first_item(poly_a);
    *item_b = first_item(poly_b);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    item_b = POLY_FIRST_ITEM(poly_b);

    /*while item_a != null_item(poly_a) && item_b != null_item(poly_b) do */
    while( item_a != POLY_NULL_ITEM(poly_a) && item_b != POLY_NULL_ITEM(poly_b))
    {
        cmp_result = POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b));

        if ( 0 == cmp_result )
        {
            /*
            *    if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
            *        bgn_coe_c = bgn_coe(item_a) - bgn_coe(item_b);
            *
            *        if bgn_coe_c is nonzero then
            *            new item_c
            *            deg(item_c) = deg(item_a);
            *            bgn_coe(item_c) = bgn_coe_c;
            *            bgn_coe_flag(item_c) = TRUE;
            *            insert item_c to tail of poly_c
            *        end if
            *    end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                if ( 0 != bgn_fp_cmp(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b)))
                {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0062);
                    bgn_fp_sub(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0063);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
            /*
            *    if TRUE == bgn_coe_flag(item_a)  and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        >>>tmp_poly_c = bgn_coe(item_a) - poly_coe(item_b)
            *        poly_sbb_const_1(bgn_coe(item_a), poly_coe(item_b),tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0064);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_sub_bgn_poly(polyfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);

                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0065);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a)  and TRUE == bgn_coe_flag(item_b)
            *        new tmp_poly_c
            *        >>> tmp_poly_c = poly_coe(item_a) bgn_coe(item_b)
            *        poly_sbb_const_2(poly_coe(item_a), bgn_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0066);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_sub_poly_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0067);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
            /*
            *    if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
            *        new tmp_poly_c
            *        poly_sub(poly_coe(item_a), poly_coe(item_b), tmp_poly_c);
            *
            *        new item_c
            *        deg(item_c) = deg(item_a);
            *        poly_coe(item_c) = tmp_poly_c;
            *        bgn_coe_flag(item_c) = FALSE;
            *        insert item_c to tail of poly_c
            *    end if
            */
            else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0068);
                POLY_INIT(poly_coe_of_item_c);

                poly_fp_sub_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);
                if ( EC_FALSE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0069);

                    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
                else
                {
                    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0070);
                    poly_coe_of_item_c = NULL_PTR;
                }
            }
            else
            {
                ;
            }
            /*
            *    item_a = next_item(poly_a, item_a);
            *    item_b = next_item(poly_b, item_b);
            *
            */
            item_a = POLY_ITEM_NEXT(item_a);
            item_b = POLY_ITEM_NEXT(item_b);
        }

        /*
        *if deg(item_a) < deg(item_b) then
        *    new item_c
        *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
        *    insert item_c to the tail of poly_c
        *    item_a = next_item(poly_a, item_a);
        *end if;
        */
        if ( ((UINT32)-1) == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0071);

            poly_fp_item_clone(polyfp_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
            item_a = POLY_ITEM_NEXT(item_a);
        }
        /*
        *if deg(item_a) > deg(item_b) then
        *    new item_c
        *    poly_item_neg(item_b, item_c)
        *
        *    insert item_c to the tail of poly_c
        *    item_b = next_item(poly_b, item_b);
        *end if;
        */
        if ( 1 == cmp_result )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0072);

            poly_fp_item_neg(polyfp_md_id, item_b, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
            item_b = POLY_ITEM_NEXT(item_b);
        }
    }
    /*
    *while item_a != null_item(poly_a) do
    *    new item_c
    *
    *    clone item_a to item_c (note: here clone item including the sub poly coe if exist)
    *    insert item_c to the tail of poly_c
    *    item_a = next_item(poly_a, item_a);
    *end do
    */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0073);

        poly_fp_item_clone(polyfp_md_id, item_a, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);
        item_a = POLY_ITEM_NEXT(item_a);
    }
    /*
    *while item_b != null_item(poly_b) do
    *    new item_c
    *
    *    poly_item_neg(item_b, item_c)
    *
    *    insert item_c to the tail of poly_c
    *    item_b = next_item(poly_b, item_b);
    *end do
    *
    */
    while ( item_b != POLY_NULL_ITEM(poly_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0074);

        poly_fp_item_neg(polyfp_md_id, item_b, item_c);

        POLY_ADD_ITEM_TAIL(poly_c, item_c);
        item_b = POLY_ITEM_NEXT(item_b);
    }

    return ( 0 );
}

UINT32 poly_fp_poly_output(const UINT32 polyfp_md_id,const POLY *poly, const UINT32 depth,const char *info)
{
    POLY_ITEM *item;
    UINT8  space_str[32];
    UINT32 index;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_output: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_output: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( NULL_PTR != info )
    {
        sys_log(LOGSTDOUT,"%s: ",info);
    }

    if ( depth >= 32 )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_output: depth = %ld overflow\n");
        return ((UINT32)(-1));
    }

    for ( index = 0; index < depth; index ++ )
    {
        space_str[ index ] = ' ';
    }
    space_str[ index ] = '\0';

    item = POLY_FIRST_ITEM(poly);

    while( item != POLY_NULL_ITEM(poly) )
    {
        sys_log(LOGSTDOUT,"%s deg: ", space_str);
        print_deg_dec(LOGSTDOUT, POLY_ITEM_DEG(item));

        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item) )
        {
            sys_log(LOGSTDOUT,"%s coe: ", space_str);
            print_bigint_dec(LOGSTDOUT, POLY_ITEM_BGN_COE(item));
        }
        else
        {
            sys_log(LOGSTDOUT,"%s coe: \n", space_str);
            poly_fp_poly_output(polyfp_md_id, POLY_ITEM_POLY_COE(item), depth + 1, NULL_PTR);
        }

        item = POLY_ITEM_NEXT(item);
    }

    return ( 0 );
}

/**
*move item_a to item_c
*
*poly_item_move:=proc(item_a, item_c)
*
*deg(item_c) = deg(item_a);
*
*if TRUE == bgn_coe_flag(item_a) then
*    bgn_coe(item_c) = bgn_coe(item_a);
*    bgn_coe_flag(item_c) = TRUE;
*
*    bgn_coe(item_a) = NULL_PTR;
*end if
*
*if FALSE == bgn_coe_flag(item_a) then
*    poly_coe(item_c) = poly_coe(item_a);
*    bgn_coe_flag(item_c) = FALSE;
*
*    poly_coe(item_a) = NULL_PTR;
*end if
*
*end proc
**/

UINT32 poly_fp_item_move(const UINT32 polyfp_md_id, POLY_ITEM *item_a, POLY_ITEM *item_c)
{
    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_move: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_move: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_move: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
    {
        POLY_ITEM_BGN_COE(item_c) = POLY_ITEM_BGN_COE(item_a);
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        POLY_ITEM_BGN_COE(item_a) = NULL_PTR;
    }
    else
    {
        POLY_ITEM_POLY_COE(item_c) = POLY_ITEM_POLY_COE(item_a);
        POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        POLY_ITEM_POLY_COE(item_a) = NULL_PTR;
    }

    return ( 0 );
}

/**
*move poly_a to poly_c
*
*poly_move:=proc(poly_a, poly_c)
*
*init poly_c
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    remove item_a from poly_a;
*    insert item_a to the tail of poly_c;
*
*    item_a = first_item(poly_a);
*end do
*
*end proc
**/
UINT32 poly_fp_poly_move(const UINT32 polyfp_md_id, POLY *poly_a, POLY *poly_c)
{
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_move: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_poly_move: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_poly_move: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a != poly_c )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        //POLY_INIT(poly_c);
        if ( EC_FALSE == POLY_IS_EMPTY(poly_a) )
        {
            POLY_ITEM_HEAD(poly_c)->next = POLY_ITEM_HEAD(poly_a)->next;
            POLY_ITEM_HEAD(poly_c)->next->prev = POLY_ITEM_HEAD(poly_c);

            POLY_ITEM_HEAD(poly_c)->prev = POLY_ITEM_HEAD(poly_a)->prev;
            POLY_ITEM_HEAD(poly_c)->prev->next = POLY_ITEM_HEAD(poly_c);
#if 0
            poly_c->poly_item_head.next = poly_a->poly_item_head.next;
            poly_c->poly_item_head.next->prev = &(poly_c->poly_item_head);

            poly_c->poly_item_head.prev = poly_a->poly_item_head.prev;
            poly_c->poly_item_head.prev->next = &(poly_c->poly_item_head);
#endif
            POLY_INIT(poly_a);
        }
    }
    return ( 0 );
}

/**
*poly_c = poly_c + item_a
*poly_item_insert := proc(item_a, poly_c)
*
*item_c = first_item(poly_c)
*
*while item_c != null_item(poly_c) do
*
*    if deg(item_c) >= deg(item_a) then
*        break;
*    end if
*
*    item_c = next_item(item_c);
*end do
*
*if item_c == null_item(poly_c) then
*    insert item_a to the tail of poly_c
*    return;
*end if
*
*if deg(item_c) > deg(item_a) then
*    insert iteam_a to the prev of item_c;
*    return;
*end if
*
*>>> now deg(item_c) == deg(item_a)
*
*if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*
*    bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_a);
*
*    if bgn_coe(item_c) is zero then
*        remove item_c from poly_c;
*        destory item_c;
*    end if
*
*end if
*
*
*if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*    poly_coe(item_c) = poly_coe(item_c) + bgn_coe(item_a);
*
*    if poly_coe(item_c) is empty then
*        remove item_c from poly_c;
*        destory item_c;
*    end if
*end if
*
*
*if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
*    new poly_coe_of_item_c;
*    init poly_coe_of_item_c;
*
*    poly_coe_of_item_c = poly_coe(item_a) + bgn_coe(item_c);
*
*    if poly_coe_of_item_c is empty then
*
*        destory poly_coe_of_item_c
*        remove item_c from poly_c
*        destory item_c
*    end if
*
*    if poly_coe_of_item_c is not empty then
*
*        free bgn_coe(item_c);
*        poly_coe(item_c) = poly_coe_of_item_c;
*
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*end if
*
*if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
*
*    poly_coe(item_c) = poly_coe(item_c) + poly_coe(item_a);
*
*    if poly_coe(item_c) is empty then
*        remove item_c from poly_c
*        destory item_c;
*    end if
*end if
*
*end proc
*
*
*Note:
*
*   After insertion, the item_a should not be referred any more!
*
**/
static UINT32 poly_fp_item_insert(const UINT32 polyfp_md_id, POLY_ITEM *item_a, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY      *poly_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_insert: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_insert: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_insert: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /**
    *item_c = first_item(poly_c)
    *
    *while item_c != null_item(poly_c) do
    *
    *    if deg(item_c) >= deg(item_a) then
    *        break;
    *    end if
    *
    *    item_c = next_item(item_c);
    *end do
    */

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_a)) )
        {
            break;
        }
        item_c = POLY_ITEM_NEXT(item_c);
    }

    /*
    *if item_c == null_item(poly_c) then
    *    insert item_a to the tail of poly_c
    *    return;
    *end if
    */
    if ( item_c == POLY_NULL_ITEM(poly_c) )
    {
        POLY_ADD_ITEM_TAIL(poly_c, item_a);
        return ( 0 );
    }

    /*
    *if deg(item_c) > deg(item_a) then
    *    insert iteam_a to the prev of item_c;
    *    return;
    *end if
    */
    if ( 0 < POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_a)) )
    {
        POLY_ITEM_ADD_PREV(item_c, item_a);
        return ( 0 );
    }

    /* now deg(item_c) == deg(item_a) */

    /*
    *if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
    *
    *    bgn_coe(item_c) = bgn_coe(item_c) + bgn_coe(item_a);
    *
    *    if bgn_coe(item_c) is zero then
    *        remove item_c from poly_c;
    *        destory item_c;
    *    end if
    *
    *end if
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        bgn_fp_add(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_c), POLY_ITEM_BGN_COE(item_c));

        if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_fp_item_destory(polyfp_md_id, item_c);
        }
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
    *    poly_coe(item_c) = poly_coe(item_c) + bgn_coe(item_a);
    *
    *    if poly_coe(item_c) is empty then
    *        remove item_c from poly_c;
    *        destory item_c;
    *    end if
    *end if
    */
    else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        poly_fp_adc_n(polyfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)))
        {
            POLY_ITEM_DEL(item_c);
            poly_fp_item_destory(polyfp_md_id, item_c);
        }
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_c) then
    *    new poly_coe_of_item_c;
    *    init poly_coe_of_item_c;
    *
    *    poly_coe_of_item_c = poly_coe(item_a) + bgn_coe(item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *
    *        destory poly_coe_of_item_c
    *        remove item_c from poly_c
    *        destory item_c
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *
    *        free bgn_coe(item_c);
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *end if
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0075);
        POLY_INIT(poly_coe_of_item_c);

        poly_fp_add_n(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_c), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0076);

            POLY_ITEM_DEL(item_c);
            poly_fp_item_destory(polyfp_md_id, item_c);
        }
        else
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, POLY_ITEM_BGN_COE(item_c), LOC_POLYFP_0077);

            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_c) then
    *
    *    poly_coe(item_c) = poly_coe(item_c) + poly_coe(item_a);
    *
    *    if poly_coe(item_c) is empty then
    *        remove item_c from poly_c
    *        destory item_c;
    *    end if
    *end if
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
    {
        poly_fp_adc_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_c));

        if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
        {
            POLY_ITEM_DEL(item_c);

            poly_fp_item_destory(polyfp_md_id, item_c);
        }
    }
    else
    {
        ;
    }

    poly_fp_item_destory(polyfp_md_id, item_a);

    return ( 0 );
}

/**
*item_c = item_a * item_b
*
*poly_item_mul :=proc(item_a, item_b, item_c)
*
*if item_a == item_c then
*    poly_item_mul_self(item_b, item_c);
*    return ;
*end if
*
*
*if item_b == item_c then
*    poly_item_mul_self(item_a, item_c);
*    return ;
*end if
*
*(carry,deg(item_c)) = deg(item_a) + deg(item_b);
*
*if carry > 0 then
*    error("degree overflow");
*    exit( 0 );
*end if
*
*if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*    new bgn_coe_of_item_c
*    bgn_coe_of_item_c = bgn_coe(item_a) * bgn_coe(item_b);
*
*    if bgn_coe_of_item_c is zero then
*        free bgn_coe_of_item_c;
*        bgn_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = TRUE;
*    end if
*
*    if bgn_coe_of_item_c is nonzero then
*        bgn_coe(item_c) = bgn_coe_of_item_c
*        bgn_coe_flag(item_c) = TRUE;
*    end if
*
*endif
*
*if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*
*    new poly_coe_of_item_c;
*
*    poly_mul_bgn(poly_coe(item_b), bgn_coe(item_a), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
*if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
*    new poly_coe_of_item_c;
*
*    poly_mul_bgn(poly_coe(item_a), bgn_coe(item_b), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
*
*if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
*    new poly_coe_of_item_c;
*
*    poly_mul(poly_coe(item_a), poly_coe(item_b), poly_coe_of_item_c);
*
*    if poly_coe_of_item_c is empty then
*        free poly_coe_of_item_c;
*        poly_coe(item_c) = NULL_PTR;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*
*    if poly_coe_of_item_c is not empty then
*        poly_coe(item_c) = poly_coe_of_item_c;
*        bgn_coe_flag(item_c) = FALSE;
*    end if
*endif
*
**end proc
**/
static UINT32 poly_fp_item_mul(const UINT32 polyfp_md_id, const POLY_ITEM *item_a, const POLY_ITEM *item_b,POLY_ITEM *item_c)
{
    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;
    UINT32 carry;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul: item_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_mul: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /**
    *if item_a == item_c then
    *    poly_item_mul_self(item_b, item_c);
    *    return ;
    *end if
    */
    if ( item_a == item_c )
    {
        poly_fp_item_mul_self( polyfp_md_id, item_b, item_c);
        return ( 0 );
    }

    /*
    *if item_b == item_c then
    *    poly_item_mul_self(item_a, item_c);
    *    return ;
    *end if
    */
    if ( item_b == item_c )
    {
        poly_fp_item_mul_self( polyfp_md_id, item_a, item_c);
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *(carry,deg(item_c)) = deg(item_a) + deg(item_b);
    *
    *if carry > 0 then
    *    error("degree overflow");
    *    exit( 0 );
    *end if
    */
    carry = POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_c));
    if ( 0 < carry )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul: degree overflow.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
    *    new bgn_coe_of_item_c
    *    bgn_coe_of_item_c = bgn_coe(item_a) * bgn_coe(item_b);
    *
    *    if bgn_coe_of_item_c is zero then
    *        free bgn_coe_of_item_c;
    *        bgn_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = TRUE;
    *    end if
    *
    *    if bgn_coe_of_item_c is nonzero then
    *        bgn_coe(item_c) = bgn_coe_of_item_c
    *        bgn_coe_flag(item_c) = TRUE;
    *    end if
    *
    *endif
    */
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0078);

        bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_BGN_COE(item_b), bgn_coe_of_item_c);

        if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c))
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0079);

            POLY_ITEM_BGN_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        }
        else
        {
            POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;
        }

        return ( 0 );
    }

    /*
    *if TRUE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
    *
    *    new poly_coe_of_item_c;
    *
    *    poly_mul_bgn(poly_coe(item_b), bgn_coe(item_a), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0080);
        POLY_INIT(poly_coe_of_item_c);

        poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_b), POLY_ITEM_BGN_COE(item_a), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0081);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and TRUE == bgn_coe_flag(item_b) then
    *    new poly_coe_of_item_c;
    *
    *    poly_mul_bgn(poly_coe(item_a), bgn_coe(item_b), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0082);
        POLY_INIT(poly_coe_of_item_c);

        poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_BGN_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0083);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    /*
    *if FALSE == bgn_coe_flag(item_a) and FALSE == bgn_coe_flag(item_b) then
    *    new poly_coe_of_item_c;
    *
    *    poly_mul(poly_coe(item_a), poly_coe(item_b), poly_coe_of_item_c);
    *
    *    if poly_coe_of_item_c is empty then
    *        free poly_coe_of_item_c;
    *        poly_coe(item_c) = NULL_PTR;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *
    *    if poly_coe_of_item_c is not empty then
    *        poly_coe(item_c) = poly_coe_of_item_c;
    *        bgn_coe_flag(item_c) = FALSE;
    *    end if
    *endif
    */
    else if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) && EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_b) )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0084);
        POLY_INIT(poly_coe_of_item_c);

        poly_fp_mul_poly(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_POLY_COE(item_b), poly_coe_of_item_c);

        if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
        {
            free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0085);

            POLY_ITEM_POLY_COE(item_c) = NULL_PTR;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }
        else
        {
            POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
            POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;
        }

        return ( 0 );
    }
    else
    {
        ;
    }

    return ( 0 );
}
/**
*item_c = item_a * item_c
*
*poly_item_mul_self :=proc(item_a, item_c)
*
*new item_b;
*
*mov item_c to item_b;
*
*poly_item_mul(item_a,item_b,item_c);
*
*destory item_b;
*
*end proc
**/
static UINT32 poly_fp_item_mul_self(const UINT32 polyfp_md_id, const POLY_ITEM *item_a,POLY_ITEM *item_c)
{
    POLY_ITEM *item_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul_self: item_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_mul_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_mul_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_b, LOC_POLYFP_0086);

    /*mov item_c to item_b*/
    poly_fp_item_move(polyfp_md_id, item_c, item_b);

    poly_fp_item_mul(polyfp_md_id, item_a, item_b, item_c);

    poly_fp_item_destory(polyfp_md_id, item_b);

    return ( 0 );
}
/**
*item_c = item_c ^ 2
*
*poly_item_squ_self :=proc(item_c)
*
*new item_a;
*
*mov item_c to item_a;
*
*poly_item_mul(item_a,item_a, item_c);
*
*destory item_a;
*
*end proc
**/
static UINT32 poly_fp_item_squ_self(const UINT32 polyfp_md_id, POLY_ITEM *item_c)
{
    POLY_ITEM *item_a;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_item_squ_self: item_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_item_squ_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_a, LOC_POLYFP_0087);

    /*mov item_c to item_a*/
    poly_fp_item_move(polyfp_md_id, item_c, item_a);

    poly_fp_item_mul(polyfp_md_id, item_a, item_a, item_c);

    poly_fp_item_destory(polyfp_md_id, item_a);

    return ( 0 );
}

/**
*poly_c = poly_c * bgn_a
*
*poly_mul_self_bgn := proc(bgn_a, poly_c)
*
*item_c = first_item(poly_c);
*
*while item_c != null_item(poly_c) do
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) * bgn_a;
*
*        if bgn_coe(item_c) is zero then
*            item_t = item_c;
*            item_c = prev_item(item_c);
*            remove item_t from poly_c;
*            destory item_t
*        end if
*    end if
*    if FALSE == bgn_coe_flag(item_c) then
*        poly_mul_self_bgn(bgn_a, poly_coe(item_c));
*    end if
*
*    item_c = next_item(item_c);
*end do
*
*end proc
**/
static UINT32 poly_fp_mul_self_bgn(const UINT32 polyfp_md_id,const BIGINT *bgn_a, POLY *poly_c)
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;


#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_self_bgn: bgn_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_self_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_self_bgn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, bgn_a) )
    {
        return ( 0 );
    }

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), bgn_a, POLY_ITEM_BGN_COE(item_c));

            if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
        }
        else
        {
            poly_fp_mul_self_bgn(polyfp_md_id, bgn_a, POLY_ITEM_POLY_COE(item_c));
        }

        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
* poly_c = poly_a * bgn_b
*
*poly_mul_bgn := proc(poly_a, bgn_b, poly_c)
*
*if poly_a == poly_c then
*    poly_mul_self_bgn (bgn_b, poly_c);
*    return ;
*end if
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    if TRUE == bgn_coe_flag(item_a) then
*        new bgn_coe_of_item_c ;
*        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_b;
*
*        if bgn_coe_of_item_c is zero then
*            free bgn_coe_of_item_c     ;
*        end if
*
*        if bgn_coe_of_item_c is nonzero then
*            new item_c;
*
*            deg(item_c) = deg(item_a);
*            bgn_coe(item_c) = bgn_coe_of_item_c ;
*            bgn_coe_flag(item_c) = TRUE;
*
*            add item_c to the tail of poly_c
*        end if
*    end if
*    if FALSE == bgn_coe_flag(item_a) then
*        new poly_coe_of_item_c;
*
*        init poly_coe_of_item_c;
*
*        poly_mul_bgn(poly_coe(item_a),bgn_b, poly_coe_of_item_c);
*
*        if poly_coe_of_item_c is empty then
*            free poly_coe_of_item_c
*        end if
*
*        if poly_coe_of_item_c is not empty then
*            new item_c
*
*            deg(item_c) = deg(item_a);
*            poly_coe(item_c) = poly_coe_of_item_c;
*            bgn_coe_flag(item_c) = FALSE;
*
*            add item_c to the tail of poly_c
*        end if
*    end if
*
*    item_a = next_item(item_a);
*end do
*
*end proc
**/
UINT32 poly_fp_mul_bgn(const UINT32 polyfp_md_id,const POLY *poly_a,const BIGINT *bgn_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    BIGINT    *bgn_coe_of_item_c;
    POLY      *poly_coe_of_item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_bgn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_bgn: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_bgn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_bgn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /**
    *
    *if poly_a == poly_c then
    *    poly_mul_self_bgn (bgn_b, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        poly_fp_mul_self_bgn(polyfp_md_id, bgn_b, poly_c);
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    poly_fp_poly_clean(polyfp_md_id, poly_c);
    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_b) )
    {
        return ( 0 );
    }

    if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, bgn_b) )
    {
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
        return ( 0 );
    }

    item_a = POLY_FIRST_ITEM(poly_a);

    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *    if TRUE == bgn_coe_flag(item_a) then
        *        new bgn_coe_of_item_c ;
        *        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_b;
        *
        *        if bgn_coe_of_item_c is zero then
        *            free bgn_coe_of_item_c     ;
        *        end if
        *
        *        if bgn_coe_of_item_c is nonzero then
        *            new item_c;
        *
        *            deg(item_c) = deg(item_a);
        *            bgn_coe(item_c) = bgn_coe_of_item_c ;
        *            bgn_coe_flag(item_c) = TRUE;
        *
        *            add item_c to the tail of poly_c
        *        end if
        *    end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0088);

            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), bgn_b, bgn_coe_of_item_c);

            if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0089);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0090);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *    if FALSE == bgn_coe_flag(item_a) then
        *        new poly_coe_of_item_c;
        *
        *        init poly_coe_of_item_c;
        *
        *        poly_mul_bgn(poly_coe(item_a),bgn_b, poly_coe_of_item_c);
        *
        *        if poly_coe_of_item_c is empty then
        *            free poly_coe_of_item_c
        *        end if
        *
        *        if poly_coe_of_item_c is not empty then
        *            new item_c
        *
        *            deg(item_c) = deg(item_a);
        *            poly_coe(item_c) = poly_coe_of_item_c;
        *            bgn_coe_flag(item_c) = FALSE;
        *
        *            add item_c to the tail of poly_c
        *        end if
        *    end if
        */
        else
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0091);
            POLY_INIT(poly_coe_of_item_c);

            poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), bgn_b, poly_coe_of_item_c);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0092);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0093);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

UINT32 poly_fp_mul_word(const UINT32 polyfp_md_id,const POLY *poly_a,const UINT32 word_b, POLY *poly_c)
{
    BIGINT *bgn_b;

    POLYFP_MD *polyfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_word: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_word: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_word: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_b, LOC_POLYFP_0094);

    bgn_fp_set_word(bgnfp_md_id, bgn_b, word_b);
    poly_fp_mul_bgn(polyfp_md_id, poly_a, bgn_b, poly_c);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_b, LOC_POLYFP_0095);

    return ( 0 );
}

/**
*
*
*   poly_c = poly_c * x^n;
*
*
**/
static UINT32 poly_fp_mul_xn_self(const UINT32 polyfp_md_id, const DEGREE *deg, POLY *poly_c)
{
    POLY_ITEM *item_c;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == deg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_xn_self: deg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_xn_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_xn_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY(poly_c) )
    {
        return ( 0 );
    }

    item_c = POLY_FIRST_ITEM(poly_c);
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        POLY_ITEM_DEG_ADD(bgnz_md_id, deg, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_c));

        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
*
*
*   poly_c = poly_a * x^deg;
*   here maybe addr poly_a = addr poly_c
*
**/
static UINT32 poly_fp_mul_xn(const UINT32 polyfp_md_id,const POLY *poly_a, const DEGREE *deg, POLY *poly_c)
{

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_xn: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == deg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_xn: deg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_xn: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_xn: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);

        return ( 0 );
    }

    if ( poly_a != poly_c )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
    }

    return poly_fp_mul_xn_self(polyfp_md_id, deg, poly_c);
}

/**
* poly_c = poly_c * poly_a
*
*poly_mul_self := proc(poly_a, poly_c)
*
*new poly_b;
*
*init poly_b;
*
*move poly_c to poly_b;
*
*poly_mul(poly_a, poly_b,poly_c);
*
*destory poly_b;
*
*end proc
**/
static UINT32 poly_fp_mul_self(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c)
{
    POLY *poly_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_self: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( poly_a == poly_c )
    {
        return poly_fp_squ_poly(polyfp_md_id, poly_a, poly_c);
    }
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_b, LOC_POLYFP_0096);
    POLY_INIT(poly_b);

    /*move poly_c to poly_b*/
    poly_fp_poly_move(polyfp_md_id, poly_c, poly_b);

    poly_fp_mul_poly(polyfp_md_id, poly_a, poly_b, poly_c);

    poly_fp_poly_destory(polyfp_md_id, poly_b);

    return ( 0 );
}

/**
* poly_c = poly_a * poly_b
*
*poly_mul := proc(poly_a, poly_b, poly_c)
*
*
*if poly_a == poly_c then
*    poly_mul_self(poly_b,poly_c);
*    return ;
*end if
*
*if poly_b == poly_c then
*    poly_mul_self(poly_a,poly_c);
*    return;
*end if
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    item_b = first_item(poly_b);
*    while item_b != null_item(poly_b) do
*        new item_c
*
*        poly_item_mul(item_a,item_b,item_c);
*
*        if item_c is empty then
*            free item_c;
*        end if
*
*        if item_c is not empty then
*            poly_item_insert(item_c,poly_c);
*        end if
*       item_b = next_item(item_b);
*    end do
*
*    item_a = next_item(item_a);
*
*end do
*
*end proc
**/
UINT32 poly_fp_mul_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c)
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mul_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mul_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /**
    *
    *if poly_a == poly_c then
    *    poly_mul_self(poly_b,poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        poly_fp_mul_self(polyfp_md_id, poly_b, poly_c);
        return ( 0 );
    }
    /*
    *if poly_b == poly_c then
    *    poly_mul_self(poly_a,poly_c);
    *    return;
    *end if
    */
    if ( poly_b == poly_c )
    {
        poly_fp_mul_self(polyfp_md_id, poly_a, poly_c);
        return ( 0 );
    }

    /*clean all items of the poly_c*/
    poly_fp_poly_clean(polyfp_md_id, poly_c);

    /*
    *item_a = first_item(poly_a);
    */
    item_a = POLY_FIRST_ITEM(poly_a);
    /*
    *while item_a != null_item(poly_a) do
    *
    *    item_b = first_item(poly_b);
    *    while item_b != null_item(poly_b) do
    *        new item_c
    *
    *        poly_item_mul(item_a,item_b,item_c);
    *
    *        if item_c is empty then
    *            free item_c;
    *        end if
    *
    *        if item_c is not empty then
    *            poly_item_insert(item_c,poly_c);
    *        end if
    *       item_b = next_item(item_b);
    *    end do
    *
    *    item_a = next_item(item_a);
    *
    *end do
    *
    **/
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        item_b = POLY_FIRST_ITEM(poly_b);

        while ( item_b != POLY_NULL_ITEM(poly_b) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0097);

            poly_fp_item_mul(polyfp_md_id, item_a, item_b, item_c);

            if ( EC_TRUE == POLY_ITEM_IS_EMPTY(item_c) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, item_c, LOC_POLYFP_0098);
            }
            else
            {
                poly_fp_item_insert(polyfp_md_id, item_c, poly_c);
            }

            item_b = POLY_ITEM_NEXT(item_b);
        }
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*
* poly_c = poly_a ^ 2
*
*/
UINT32 poly_fp_squ_poly(const UINT32 polyfp_md_id,const POLY *poly_a,POLY *poly_c)
{
    POLY *poly_ta;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_squ_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_squ_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_squ_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    if ( poly_a == poly_c )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_ta, LOC_POLYFP_0099);
        POLY_INIT(poly_ta);

        poly_fp_poly_move(polyfp_md_id, poly_c, poly_ta);

        poly_fp_mul_poly(polyfp_md_id, poly_ta, poly_ta, poly_c);

        poly_fp_poly_destory(polyfp_md_id, poly_ta);
    }
    else
    {
        poly_fp_mul_poly(polyfp_md_id, poly_a, poly_a, poly_c);
    }

    return ( 0 );
}

/**
* here a,c are polynomials and e is integer
*
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ WORDSIZE
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e, where a is polynomail and e < 2 * WORDSIZE
*   Output: c such that c = ( a ^ e ) mod n
*   1.  set c = 1
*   2.  while ( 1 ) do
*   2.1     if e is odd, then do
*               c = c * a mod n
*   2.2     end if
*   2.3     e  = ( e >> 1)
*   2.4     if e is zero, then break while, end if
*   2.5     a = a ^ 2 mod n
*   2.6     next while
*   3.  return c
*
**/
UINT32 poly_fp_sexp(const UINT32 polyfp_md_id,const POLY *poly_a,const UINT32 e,POLY *poly_c )
{
    POLY *poly_ta;
    UINT32 te;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sexp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_sexp: c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_sexp: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        /*before set zero, poly_fp_set_zero will clean up the items of poly_c*/
        poly_fp_set_zero(polyfp_md_id, poly_c);

        return ( 0 );
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_ta, LOC_POLYFP_0100);
    POLY_INIT(poly_ta);

    /*the branch is to decrease sharply the clone action of large number of items*/
    if ( poly_a == poly_c )
    {
        poly_fp_poly_move(polyfp_md_id, poly_c, poly_ta);
    }
    else
    {
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_ta);
    }

    /*before set one, poly_fp_set_one will clean up the items of poly_c*/
    poly_fp_set_one(polyfp_md_id, poly_c);

    te = e;
    while ( 1 )
    {
        if ( te & 1 )
        {
            poly_fp_mul_self(polyfp_md_id, poly_ta, poly_c);
        }
        te >>= 1;

        if ( 0 == te )
        {
            break;
        }

        poly_fp_squ_poly(polyfp_md_id, poly_ta, poly_ta);
    }

    poly_fp_poly_destory(polyfp_md_id, poly_ta);

    return ( 0 );
}


/**
*
* here a,b,c are polynomials and e is integer
*
*       c = ( a ^ e ) mod n
*       where a is polynomial and e < 2 ^ BIGINTSIZE
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e,where a is polynomial
*   Output: c such that c = ( a ^ e ) mod n
*   0.  let e = SUM(e_i * 2 ^ i, i = 0..m)
*   1.  set c = 1
*   2.  set i = 0
*   3.  while ( 1 ) do
*   3.1     if e_i is odd, then do
*               c = c * a mod n
*   3.2     end if
*   3.3     i = i + 1
*   3.4     if i = m + 1, then break while, end if
*   3.5     a = a ^ 2 mod n
*   3.6     next while
*   4.  return c
*
**/
UINT32 poly_fp_exp(const UINT32 polyfp_md_id,const POLY *poly_a,const BIGINT *e,POLY *poly_c )
{
    POLY   *poly_ta;

    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp: e is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_exp: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY( poly_a ) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY,&poly_ta, LOC_POLYFP_0101);
    POLY_INIT(poly_ta);

    poly_fp_poly_clone(polyfp_md_id, poly_a, poly_ta);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, e );

    poly_fp_set_one(polyfp_md_id, poly_c);

    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, e, index);
        if ( 1 == e_bit )
        {
            poly_fp_mul_self(polyfp_md_id, poly_ta, poly_c);
        }

        index ++;
        if ( index == te_nbits )
        {
            break;
        }
        poly_fp_squ_poly(polyfp_md_id, poly_ta, poly_ta);
    }

    poly_fp_poly_destory(polyfp_md_id, poly_ta);

    return ( 0 );
}

/**
*>>> check poly is one-var polynomial
*>>> if yes, return TRUE,otherwise return FALSE
*poly_is_one_var_poly := proc(poly)
*
*    item = first_item(poly);
*    while item != null_item(poly) do
*        if FALSE == bgn_coe_flag(item) then
*            return FALSE;
*        end if
*        item = next_item(item);
*    end do
*
*    return TRUE;
*end proc
**/
static EC_BOOL poly_fp_is_one_var_poly(const UINT32 polyfp_md_id, const POLY *poly )
{
    POLY_ITEM *item;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_is_one_var_poly: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_is_one_var_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    item = POLY_FIRST_ITEM(poly);
    while ( item != POLY_NULL_ITEM(poly) )
    {
        if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item) )
        {
            return (EC_FALSE);
        }
        item = POLY_ITEM_NEXT(item);
    }

    return (EC_TRUE);
}

/**
*
*   determine whether the poly is monic polynomial or not.
*   if poly is monic polynomial, then return EC_TRUE;
*   if poly is not monic polynomial, then return EC_FALSE.
*
*   note:
*       1. poly = 0 is not monic polynomial.
*       2. poly must be one-var polynomial.
**/

EC_BOOL poly_fp_is_monic(const UINT32 polyfp_md_id, const POLY *poly )
{
    POLY_ITEM *item;

    POLYFP_MD  *polyfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_is_monic: poly is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_is_monic: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_is_monic: not support poly with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    if ( EC_TRUE == POLY_IS_EMPTY(poly) )
    {
        return (EC_FALSE);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    item = POLY_LAST_ITEM(poly);
    if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item)
      && EC_TRUE == bgn_fp_is_one(bgnfp_md_id, POLY_ITEM_BGN_COE(item)) )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

/**
*>>> poly_c is the monic polynomial of poly_a
*poly_to_monic :=proc(poly_a, poly_c)
*
*>>> assume poly_c and poly_a be one-var polynomial.
*if FALSE == poly_is_one_var_poly(poly_a) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_c) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if  TRUE == poly_is_monic(poly_a) then
*    if poly_a != poly_c then
*        clone poly_a to poly_c;
*    end if
*    return;
*end if
*
*
*item_a = last_item(poly_a);
*
*new inv_of_bgn_coe_of_item_a
*inv_of_bgn_coe_of_item_a = 1/bgn_coe(item_a);
*poly_mul_bgn(poly_a, inv_of_bgn_coe_of_item_a, poly_c);
*
*free inv_of_bgn_coe_of_item_a
*
*end proc
**/
UINT32 poly_fp_to_monic(const UINT32 polyfp_md_id, const POLY *poly_a, POLY * poly_c)
{
    POLY_ITEM *item_a;

    BIGINT *inv_of_bgn_coe_of_item_a;

    POLYFP_MD  *polyfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_to_monic: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_to_monic: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_to_monic: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_to_monic: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_to_monic: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    /*
    *if  TRUE == poly_is_monic(poly_a) then
    *    if poly_a != poly_c then
    *        clone poly_a to poly_c;
    *    end if
    *    return;
    *end if
    */
    if ( EC_TRUE == poly_fp_is_monic(polyfp_md_id, poly_a) )
    {
        if ( poly_a != poly_c )
        {
            poly_fp_poly_clean(polyfp_md_id, poly_c);
            poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
        }
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *item_a = last_item(poly_a);
    */
    item_a = POLY_LAST_ITEM(poly_a);

    /*
    *new inv_of_bgn_coe_of_item_a
    *inv_of_bgn_coe_of_item_a = 1/bgn_coe(item_a);
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &inv_of_bgn_coe_of_item_a, LOC_POLYFP_0102);
    bgn_fp_inv(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), inv_of_bgn_coe_of_item_a);

    /*
    *poly_mul_bgn(poly_a, inv_of_bgn_coe_of_item_a, poly_c);
    */
    poly_fp_mul_bgn(polyfp_md_id, poly_a, inv_of_bgn_coe_of_item_a, poly_c);
    /*
    *free inv_of_bgn_coe_of_item_a
    */
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, inv_of_bgn_coe_of_item_a, LOC_POLYFP_0103);

    return ( 0 );
}

/**
*
*   for internal usage only:
*
*   check validation of degrees of poly_a and poly_b for mod computation poly_a mod poly_b.
*   since the mod computation will VERY slow if deg(poly_a) >> poly_b, e.g.,
*       deg(poly_a) = 2^192-2^64-1 and deg(poly_b) = 3
*   we have to set a threshold of (deg(poly_a) - deg(poly_b))  to restrict the polynomial mod computation.
*   currently, set the reshold to be 32 which also means the deg(poly_b) must be less than the threshold.
*
*
*   if deg(poly_a)<= deg(poly_b) then return TRUE;
*   if deg(poly_b) < threshold and deg(poly_a) - deg(poly_b) < threshold then return TRUE;
*   else return FALSE
*
**/
static EC_BOOL poly_fp_check_valid_for_mod(const UINT32 polyfp_md_id, const POLY *poly_a, const POLY *poly_b )
{
    DEGREE *deg_thrd;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_check_valid_for_mod: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_a is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: poly_a is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_b is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    /*if deg(poly_a) <= deg(poly_b) then return TRUE*/
    if ( 0 >= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), POLY_DEG(poly_b)) )
    {
        return (EC_TRUE);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &deg_thrd, LOC_POLYFP_0104);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, deg_thrd, MAX_POLY_DEG_SUPPORTED_FOR_MOD);

    /*if deg(poly_b) >= threshold deg_thrd, then return FALSE*/
    if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_b), deg_thrd) )
    {
        //sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: deg(poly_b) >= %ld is not supported.\n",MAX_POLY_DEG_SUPPORTED_FOR_MOD);
        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, deg_thrd, LOC_POLYFP_0105);
        return (EC_FALSE);
    }

    /*if deg(poly_a) >= threshold deg_thrd + deg(poly_b) , then return FALSE*/
    POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_DEG(poly_b), deg_thrd, deg_thrd);
    if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), deg_thrd) )
    {
        //sys_log(LOGSTDOUT,"error:poly_fp_check_valid_for_mod: deg(poly_a) - deg(poly_b) >= %ld is not supported.\n",MAX_POLY_DEG_SUPPORTED_FOR_MOD);
        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, deg_thrd, LOC_POLYFP_0106);
        return (EC_FALSE);
    }

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, deg_thrd, LOC_POLYFP_0107);

    return (EC_TRUE);
}
/**
*>>> poly_c = poly_c mod poly_b
*>>> where poly_b,poly_c are all one-var polynomial.
*poly_mod_a_self :=proc(poly_b, poly_c)
*
*if FALSE == poly_is_one_var_poly(poly_b) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_c) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if poly_b is empty then
*    error "mod zero"
*    return ERR;
*end if
*
*if poly_b == poly_c then
*    clean up all items of poly_c;
*    set poly_c to be one;
*    return;
*end if
*
*item_b = last_item(poly_b);
*
*new inv_of_bgn_coe_of_item_b;
*>>> inv_of_bgn_coe_of_item_b = 1 / bgn_coe(item_b)
*if EC_FALSE == bgn_inv(bgn_coe(item_b), inv_of_bgn_coe_of_item_b) then
*    error "inversion failure"
*end if
*
*new poly_t;
*init poly_t;
*
*while  poly_ta is not empty and deg(poly_c) >= deg(poly_b) do
*
*    new item_t;
*    new bgn_coe_of_item_t;
*
*    item_c = last_item(poly_c);
*
*    >>> bgn_coe(item_t) = bgn_coe(item_c)/bgn_coe(item_b) = bgn_coe(item_c) * inv_of_bgn_coe_of_item_b
*    bgn_mul(bgn_coe(item_c), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
*
*    deg(item_t) = deg(item_c) - deg(item_b);
*    bgn_coe(item_t) = bgn_coe_of_item_t;
*    bgn_coe_flag(item_t) = EC_TRUE;
*
*    insert item_t to the tail of poly_t;
*
*    >>> poly_t = poly_b * poly_t
*    poly_mul(poly_b, poly_t, poly_t);
*
*    >>> poly_c = poly_c - poly_t;
*    poly_sub(poly_c, poly_t, poly_c);
*    clean up all items of poly_t;
*
*end do
*
*free inv_of_bgn_coe_of_item_b
*free poly_t;
*return ;
*end proc
**/
static UINT32 poly_fp_mod_a_self(const UINT32 polyfp_md_id, const POLY *poly_b, POLY *poly_c )
{
    POLY      *poly_t;
    POLY_ITEM *item_b;
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;
    BIGINT    *inv_of_bgn_coe_of_item_b;
    BIGINT    *bgn_coe_of_item_t;
    EC_BOOL    bool_inv_of_bgn_coe_of_item_b_is_one;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mod_a_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_b is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_b == poly_c then
    *    clean up all items of poly_c;
    *    set poly_c to be one;
    *    return;
    *end if
    */
    if ( poly_b == poly_c )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_set_one(polyfp_md_id, poly_c);
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*check the degree validation of mod operation polynomials poly_b and poly_c*/
    /*if invalid, then exit*/
    if ( EC_FALSE == poly_fp_check_valid_for_mod(polyfp_md_id, poly_c, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: invalid degrees of poly_c and poly_b.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *item_b = last_item(poly_b);
    *
    *new inv_of_bgn_coe_of_item_b;
    *>>> inv_of_bgn_coe_of_item_b = 1 / bgn_coe(item_b)
    *if EC_FALSE == bgn_inv(bgn_coe(item_b), inv_of_bgn_coe_of_item_b) then
    *    error "inversion failure"
    *end if
    */
    /*note: the last item of the poly has the highest degree*/
    item_b = POLY_LAST_ITEM(poly_b);
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &inv_of_bgn_coe_of_item_b, LOC_POLYFP_0108);
    if ( EC_FALSE == bgn_fp_inv(bgnfp_md_id, POLY_ITEM_BGN_COE(item_b), inv_of_bgn_coe_of_item_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_a_self: inversion failure.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, inv_of_bgn_coe_of_item_b) )
    {
        bool_inv_of_bgn_coe_of_item_b_is_one = EC_TRUE;
    }
    else
    {
        bool_inv_of_bgn_coe_of_item_b_is_one = EC_FALSE;
    }
    /*
    *new poly_t;
    *init poly_t;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_t, LOC_POLYFP_0109);
    POLY_INIT(poly_t);

    /*
    *while  poly_ta is not empty and deg(poly_c) >= deg(poly_b) do
    *
    *    new item_t;
    *    new bgn_coe_of_item_t;
    *
    *    item_c = last_item(poly_c);
    *
    *    >>> bgn_coe(item_t) = bgn_coe(item_c)/bgn_coe(item_b) = bgn_coe(item_c) * inv_of_bgn_coe_of_item_b
    *    bgn_mul(bgn_coe(item_c), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
    *
    *    deg(item_t) = deg(item_c) - deg(item_b);
    *    bgn_coe(item_t) = bgn_coe_of_item_t;
    *    bgn_coe_flag(item_t) = EC_TRUE;
    *
    *    insert item_t to the tail of poly_t;
    *
    *    >>> poly_t = poly_b * poly_t
    *    poly_mul(poly_b, poly_t, poly_t);
    *
    *    >>> poly_c = poly_c - poly_t;
    *    poly_sub(poly_c, poly_t, poly_c);
    *    clean up all items of poly_t;
    *
    *end do
    */
    while ( EC_FALSE == POLY_IS_EMPTY(poly_c)
         && 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_c), POLY_DEG(poly_b)) )
    {
        item_c = POLY_LAST_ITEM(poly_c);

        /*bgn_coe(item_t) = bgn_coe(item_c)/bgn_coe(item_b) = bgn_coe(item_c) * inv_of_bgn_coe_of_item_b*/
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_t, LOC_POLYFP_0110);
        if ( EC_FALSE == bool_inv_of_bgn_coe_of_item_b_is_one )
        {
            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
        }
        else
        {
            bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), bgn_coe_of_item_t);
        }

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0111);

        POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_t));
        POLY_ITEM_BGN_COE(item_t) = bgn_coe_of_item_t;
        POLY_ITEM_BGN_COE_FLAG(item_t) = EC_TRUE;

        //sys_log(LOGSTDOUT,"poly_fp_mod_a_self: deg diff = ");
        //print_bigint_dec(LOGSTDOUT, POLY_ITEM_DEG(item_t));

        POLY_ADD_ITEM_TAIL(poly_t, item_t);

        /*poly_t = poly_b * poly_t*/
        poly_fp_mul_poly(polyfp_md_id, poly_b, poly_t, poly_t);

        /*poly_c = poly_c - poly_t;*/
        poly_fp_sub_poly(polyfp_md_id, poly_c, poly_t, poly_c);

        poly_fp_poly_clean(polyfp_md_id, poly_t);
    }
    /*
    *free inv_of_bgn_coe_of_item_b
    *free poly_t;
    */
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, inv_of_bgn_coe_of_item_b, LOC_POLYFP_0112);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_t, LOC_POLYFP_0113);
    return ( 0 );
}

/**
*>>> poly_c = poly_a mod poly_c
*>>> where poly_a,poly_c are all one-var polynomial.
*poly_mod_b_self :=proc(poly_a, poly_c)
*
*if FALSE == poly_is_one_var_poly(poly_a) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_c) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if poly_c is empty then
*    error "mod zero"
*    return ERR;
*end if
*
*if poly_a == poly_c then
*    clean up all items of poly_c;
*    set poly_c to be one;
*    return;
*end if
*
*new poly_b;
*init poly_b;
*
*mov poly_c to poly_b;
*
*poly_mod(poly_a,poly_b,poly_c);
*
*destory poly_b;
*
*return ;
*end proc
**/
static UINT32 poly_fp_mod_b_self(const UINT32 polyfp_md_id, const POLY *poly_a, POLY *poly_c )
{
    POLY *poly_b;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mod_b_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */

    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_c is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: poly_c is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if poly_a == poly_c then
    *    clean up all items of poly_c;
    *    set poly_c to be one;
    *    return;
    *end if
    */
    if ( poly_a == poly_c )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_set_one(polyfp_md_id, poly_c);

        return ( 0 );
    }

    /*check the degree validation of mod operation polynomials poly_b and poly_c*/
    /*if invalid, then exit*/
    if ( EC_FALSE == poly_fp_check_valid_for_mod(polyfp_md_id, poly_a, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_b_self: invalid degrees of poly_a and poly_c.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *new poly_b;
    *init poly_b;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_b, LOC_POLYFP_0114);
    POLY_INIT(poly_b);

    /*
    *mov poly_c to poly_b;
    */
    poly_fp_poly_move(polyfp_md_id, poly_c, poly_b);

    /*
    *poly_mod(poly_a,poly_b,poly_c);
    */
    poly_fp_mod_poly_simple(polyfp_md_id, poly_a, poly_b, poly_c);

    /*
    *destory poly_b;
    */
    poly_fp_poly_destory(polyfp_md_id, poly_b);

    return ( 0 );
}

/**
*
*>>> poly_c = poly_a mod poly_b
*>>> where poly_a,poly_b,poly_c are all one-var polynomial.
*poly_mod :=proc(poly_a, poly_b, poly_c)
*
*if FALSE == poly_is_one_var_poly(poly_a) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_b) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_c) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if poly_b is empty then
*    error "mod zero"
*    return ERR;
*end if
*
*if poly_a == poly_b then
*    clean up all items of poly_c;
*    set poly_c to be one;
*    return ;
*end if
*
*if poly_a == poly_c then
*    poly_mod_a_self(poly_b,poly_c);
*    return;
*end if
*
*if poly_b == poly_c then
*    poly_mod_b_self(poly_a,poly_c);
*    return;
*end if
*
*if deg(poly_a) < deg(poly_b) then
*    clone poly_a to poly_c;
*    return;
*end if
*
*
*item_b = last_item(poly_b);
*
*new inv_of_bgn_coe_of_item_b;
*>>> inv_of_bgn_coe_of_item_b = 1 / bgn_coe(item_b)
*if EC_FALSE == bgn_inv(bgn_coe(item_b), inv_of_bgn_coe_of_item_b) then
*    error "inversion failure"
*end if
*
*new poly_t;
*init poly_t;
*
*new poly_ta;
*init poly_ta;
*
*clone poly_a to poly_ta;
*
*while  poly_ta is not empty and deg(poly_ta) >= deg(poly_b) do
*
*    new item_t;
*    new bgn_coe_of_item_t;
*
*    item_ta = last_item(poly_ta);
*
*    >>> bgn_coe(item_t) = bgn_coe(item_ta)/bgn_coe(item_b) = bgn_coe(item_ta) * inv_of_bgn_coe_of_item_b
*    bgn_mul(bgn_coe(item_ta), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
*
*    deg(item_t) = deg(item_ta) - deg(item_b);
*    bgn_coe(item_t) = bgn_coe_of_item_t;
*    bgn_coe_flag(item_t) = EC_TRUE;
*
*    insert item_t to the tail of poly_t;
*
*    >>> poly_t = poly_b * poly_t
*    poly_mul(poly_b, poly_t, poly_t);
*
*    >>> poly_ta = poly_ta - poly_t;
*    poly_sub(poly_ta, poly_t, poly_ta);
*    clean up all items of poly_t;
*
*end do
*
*clean all items of poly_c;
*move poly_ta to poly_c;
*free inv_of_bgn_coe_of_item_b
*free poly_ta;
*free poly_t;
*return ;
*
*end proc
**/
static UINT32 poly_fp_mod_poly_simple(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c )
{
    POLY *poly_t;
    POLY *poly_ta;
    POLY_ITEM *item_b;
    POLY_ITEM *item_t;
    POLY_ITEM *item_ta;
    BIGINT    *inv_of_bgn_coe_of_item_b;
    BIGINT    *bgn_coe_of_item_t;
    EC_BOOL    bool_bgn_coe_of_item_b_is_one;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mod_poly_simple: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if poly_b is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if poly_a is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    /*
    *if poly_a == poly_b then
    *    clean up all items of poly_c;
    *    set poly_c to be one;
    *    return ;
    *end if
    */
    if ( poly_a == poly_b )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_set_one(polyfp_md_id, poly_c);

        return ( 0 );
    }
    /*
    *if poly_a == poly_c then
    *    poly_mod_a_self(poly_b,poly_c);
    *    return;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_mod_a_self(polyfp_md_id, poly_b, poly_c);
    }
    /*
    *if poly_b == poly_c then
    *    poly_mod_b_self(poly_a,poly_c);
    *    return;
    *end if
    */
    if ( poly_b == poly_c )
    {
        return poly_fp_mod_b_self(polyfp_md_id, poly_a, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*check the degree validation of mod operation polynomials poly_a and poly_b*/
    /*if invalid, then exit*/
    if ( EC_FALSE == poly_fp_check_valid_for_mod(polyfp_md_id, poly_a, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: invalid degrees of poly_a and poly_b.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if deg(poly_a) < deg(poly_b) then
    *    clone poly_a to poly_c;
    *    return;
    *end if
    */
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), POLY_DEG(poly_b)) )
    {
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
        return ( 0 );
    }

    /*
    *item_b = last_item(poly_b);
    *
    *new inv_of_bgn_coe_of_item_b;
    *>>> inv_of_bgn_coe_of_item_b = 1 / bgn_coe(item_b)
    *if EC_FALSE == bgn_inv(bgn_coe(item_b), inv_of_bgn_coe_of_item_b) then
    *    error "inversion failure"
    *end if
    */
    item_b = POLY_LAST_ITEM(poly_b);
    if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, POLY_ITEM_BGN_COE(item_b)) )
    {
        bool_bgn_coe_of_item_b_is_one = EC_TRUE;
    }
    else
    {
        bool_bgn_coe_of_item_b_is_one = EC_FALSE;

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &inv_of_bgn_coe_of_item_b, LOC_POLYFP_0115);
        if ( EC_FALSE == bgn_fp_inv(bgnfp_md_id, POLY_ITEM_BGN_COE(item_b), inv_of_bgn_coe_of_item_b) )
        {
            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_simple: inversion failure.\n");
            dbg_exit(MD_POLYFP, polyfp_md_id);
        }
    }

    /*
    *new poly_t;
    *init poly_t;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_t, LOC_POLYFP_0116);
    POLY_INIT(poly_t);

    /*
    *new poly_ta;
    *init poly_ta;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_ta, LOC_POLYFP_0117);
    POLY_INIT(poly_ta);

    /*
    *clone poly_a to poly_ta;
    */
    poly_fp_poly_clone(polyfp_md_id, poly_a, poly_ta);

    /*
    *while poly_ta is not empty and deg(poly_ta) >= deg(poly_b) do
    *
    *    new item_t;
    *    new bgn_coe_of_item_t;
    *
    *    item_ta = last_item(poly_ta);
    *
    *    >>> bgn_coe(item_t) = bgn_coe(item_ta)/bgn_coe(item_b) = bgn_coe(item_ta) * inv_of_bgn_coe_of_item_b
    *    bgn_mul(bgn_coe(item_ta), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
    *
    *    deg(item_t) = deg(item_ta) - deg(item_b);
    *    bgn_coe(item_t) = bgn_coe_of_item_t;
    *    bgn_coe_flag(item_t) = EC_TRUE;
    *
    *    insert item_t to the tail of poly_t;
    *
    *    >>> poly_t = poly_b * poly_t
    *    poly_mul(poly_b, poly_t, poly_t);
    *
    *    >>> poly_ta = poly_ta - poly_t;
    *    poly_sub(poly_ta, poly_t, poly_ta);
    *    clean up all items of poly_t;
    *
    *end do
    */
    while ( EC_FALSE == POLY_IS_EMPTY(poly_ta)
         && 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_ta), POLY_DEG(poly_b)) )
    {
        item_ta = POLY_LAST_ITEM(poly_ta);

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_t, LOC_POLYFP_0118);

        /*bgn_coe(item_t) = bgn_coe(item_ta)/bgn_coe(item_b) = bgn_coe(item_ta) * inv_of_bgn_coe_of_item_b*/
        /*where inv_of_bgn_coe_of_item_b = 1 when bgn_coe(item_b) = 1*/
        if ( EC_TRUE == bool_bgn_coe_of_item_b_is_one )
        {
            bgn_fp_clone(bgnfp_md_id, POLY_ITEM_BGN_COE(item_ta), bgn_coe_of_item_t);
        }
        else
        {
            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_ta), inv_of_bgn_coe_of_item_b, bgn_coe_of_item_t);
        }

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0119);

        POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_ta), POLY_ITEM_DEG(item_b), POLY_ITEM_DEG(item_t));
        POLY_ITEM_BGN_COE(item_t) = bgn_coe_of_item_t;
        POLY_ITEM_BGN_COE_FLAG(item_t) = EC_TRUE;
#if 0
        sys_log(LOGSTDOUT,"poly_fp_mod_poly_simple: deg diff = ");
        print_bigint_dec(LOGSTDOUT, POLY_ITEM_DEG(item_t));
#endif
        POLY_ADD_ITEM_TAIL(poly_t, item_t);

        /*poly_t = poly_b * poly_t*/
        poly_fp_mul_poly(polyfp_md_id, poly_b, poly_t, poly_t);

        /*poly_ta = poly_ta - poly_t*/
        poly_fp_sub_poly(polyfp_md_id, poly_ta, poly_t, poly_ta);

        poly_fp_poly_clean(polyfp_md_id, poly_t);
    }
    /*
    *clean all items of poly_c;
    *move poly_ta to poly_c;
    */
    poly_fp_poly_clean(polyfp_md_id, poly_c);
    poly_fp_poly_move(polyfp_md_id, poly_ta, poly_c);

    /*
    *free inv_of_bgn_coe_of_item_b
    *free poly_ta;
    *free poly_t;
    */
    if ( EC_FALSE == bool_bgn_coe_of_item_b_is_one )
    {
        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, inv_of_bgn_coe_of_item_b, LOC_POLYFP_0120);
    }
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_ta, LOC_POLYFP_0121);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_t, LOC_POLYFP_0122);

    return ( 0 );
}

static UINT32 poly_fp_xm_mod_pre(const UINT32 polyfp_md_id,
                                          const UINT32 poly_precomp_tbl_size,
                                          POLY  *poly_precomp_tbl,
                                          const POLY *poly_b)
{
    POLY  *poly_t;
    POLY  *poly_t_prev;
    POLY  *poly_tb;
    DEGREE *deg_one;
    DEGREE *bgn_deg_of_poly_b;
    UINT32 word_deg_of_poly_b;
    UINT32 precomp_size;
    UINT32 index;

    POLYFP_MD *polyfp_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_precomp_tbl )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_with_pre: poly_precomp_tbl is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_with_pre: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_xm_mod_with_pre: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_with_pre: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    bgn_deg_of_poly_b = POLY_DEG(poly_b);
    if ( EC_FALSE == bgn_z_get_word(bgnz_md_id, bgn_deg_of_poly_b, &word_deg_of_poly_b) )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_xm_mod_with_pre: invalid degree of poly_b.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    precomp_size = 2 * word_deg_of_poly_b;

    if ( precomp_size > poly_precomp_tbl_size )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_xm_mod_with_pre: invalid degree of poly_b: precomp_size > poly_precomp_tbl_size.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    poly_fp_alloc_poly(polyfp_md_id, &poly_tb);
    POLY_INIT(poly_tb);

    poly_fp_to_monic(polyfp_md_id, poly_b, poly_tb);

    /*set deg_one = 1*/
    poly_fp_alloc_deg(polyfp_md_id, &deg_one);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    /*compute poly_precomp_tbl[ 0 ] = x^ deg_t mod poly_b */
    poly_t = &(poly_precomp_tbl[ 0 ]);
    poly_fp_poly_clean(polyfp_md_id, poly_t);
    poly_fp_set_xn(polyfp_md_id, POLY_DEG(poly_tb), poly_t);
    poly_fp_mod_poly_simple(polyfp_md_id, poly_t, poly_b, poly_t);
    //poly_fp_sub_poly(polyfp_md_id, poly_t, poly_tb, poly_t);
    //poly_fp_to_monic(polyfp_md_id, poly_t, poly_t);

    for ( index = 1; index < precomp_size; index ++ )
    {
        poly_t = &(poly_precomp_tbl[ index ]);
        poly_fp_poly_clean(polyfp_md_id, poly_t);

        poly_t_prev = &(poly_precomp_tbl[ index - 1 ]);

        /*poly_t = poly_t_prev * x mod poly_b*/
        poly_fp_mul_xn(polyfp_md_id, poly_t_prev, deg_one, poly_t);
        if ( 0 == POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_t), POLY_DEG(poly_tb)))
        {
            poly_fp_mod_poly_simple(polyfp_md_id, poly_t, poly_b, poly_t);

            //poly_fp_sub_poly(polyfp_md_id, poly_t, poly_tb, poly_t);
            //poly_fp_to_monic(polyfp_md_id, poly_t, poly_t);
        }
    }

    poly_fp_poly_destory(polyfp_md_id, poly_tb);
    poly_fp_free_deg(polyfp_md_id, deg_one);

    return ( 0 );
}

/**
*
*   poly_c = poly_a mod poly_b with precomputation poly table.
*
*   let poly_a = SUM(a_i * x^i)
*   then
*       poly_c = SUM(a_i * x^i, where i < deg(poly_b) )
*              + SUM(a_i * poly_precomp_tbl[ i - deg(poly_b) ],where i >= deg(poly_b) )
*
**/
static UINT32 poly_fp_mod_poly_with_pre(const UINT32 polyfp_md_id,
                                                 const POLY   *poly_precomp_tbl,
                                                 const POLY   *poly_a,
                                                 const POLY   *poly_b,
                                                 POLY   *poly_c)
{
    POLY        *poly_t;
    POLY        *poly_mid;
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_t;
    BIGINT      *bgn_coe_of_item_a;
    DEGREE      *tbl_size;
    DEGREE      *delta_deg;
    UINT32       delta_deg_t;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_mod_poly_with_pre: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*if deg(poly_a) < deg(poly_b), then clone poly_a to poly_c*/
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), POLY_DEG(poly_b)) )
    {
        if ( poly_a != poly_c )
        {
            poly_fp_poly_clean(polyfp_md_id, poly_c);
            poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
        }
        return ( 0 );
    }

    /*set tbl_size = poly_precomp_tbl_size*/
    poly_fp_alloc_deg(polyfp_md_id, &tbl_size);
    //POLY_ITEM_DEG_SET_WORD(bgnz_md_id, tbl_size, poly_precomp_tbl_size);
    POLY_ITEM_DEG_ADD(bgnz_md_id, POLY_DEG(poly_b), POLY_DEG(poly_b), tbl_size);

    /*set delta_deg = 0*/
    poly_fp_alloc_deg(polyfp_md_id, &delta_deg);
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, delta_deg);

    /*set poly_mid = 0*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_mid);
    POLY_INIT(poly_mid);

    /*set poly_t = 0*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_t);
    POLY_INIT(poly_t);

    item_a = POLY_FIRST_ITEM(poly_a);

    /*if deg(item_a) < deg(poly_b) then clone item_a to poly_t*/
    while ( item_a != POLY_NULL_ITEM(poly_a)
        &&  0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_DEG(poly_b))
          )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t, LOC_POLYFP_0123);

        poly_fp_item_clone(polyfp_md_id, item_a, item_t);

        POLY_ADD_ITEM_TAIL(poly_t, item_t);

        /*next item*/
        item_a = POLY_ITEM_NEXT(item_a);
    }

    /*if deg(item_a) >= deg(poly_b) then */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_DEG(poly_b), delta_deg);
        bgn_coe_of_item_a = POLY_ITEM_BGN_COE(item_a);

        /*if delta_deg >= tbl_size,then overflow*/
        if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, delta_deg, tbl_size) )
        {
            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: degree overflow.\n");

            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: delta_deg = ");
            print_deg_dec(LOGSTDOUT, delta_deg);

            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: tbl_size = ");
            print_deg_dec(LOGSTDOUT, tbl_size);

            dbg_exit(MD_POLYFP, polyfp_md_id);
        }

        if ( EC_FALSE == bgn_z_get_word(bgnz_md_id, delta_deg, &delta_deg_t) )
        {
            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: degree overflow.\n");

            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: delta_deg = ");
            print_deg_dec(LOGSTDOUT, delta_deg);

            sys_log(LOGSTDOUT,"error:poly_fp_mod_poly_with_pre: tbl_size = ");
            print_deg_dec(LOGSTDOUT, tbl_size);

            dbg_exit(MD_POLYFP, polyfp_md_id);
        }

        /*poly_t = poly_t + bgn_coe_of_item_a * poly_precomp_tbl[ deg(item_a) - deg(poly_b) ]*/
        poly_fp_mul_bgn(polyfp_md_id, &(poly_precomp_tbl[ delta_deg_t ]), bgn_coe_of_item_a, poly_mid);

        poly_fp_adc_poly(polyfp_md_id, poly_mid, poly_t);

        /*next item*/
        item_a = POLY_ITEM_NEXT(item_a);
    }

    poly_fp_poly_clean(polyfp_md_id, poly_c);
    poly_fp_poly_move(polyfp_md_id, poly_t, poly_c);

    poly_fp_poly_destory(polyfp_md_id, poly_mid);

    poly_fp_free_poly(polyfp_md_id, poly_t);
    poly_fp_free_deg(polyfp_md_id, tbl_size);
    poly_fp_free_deg(polyfp_md_id, delta_deg);

    return ( 0 );
}

static UINT32 poly_fp_xm_mod_poly_with_pre(const UINT32 polyfp_md_id,const DEGREE *deg,const POLY *poly_b,POLY *poly_c )
{
    POLY *poly_tmp;
    POLY *poly_tc;

    BIGINT    *bgn_m;

    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*precomputating: */
    //poly_fp_xm_mod_pre(polyfp_md_id, MAX_NUM_OF_PRECOMP_FOR_MOD, g_poly_fp_mod_pre_tbl, poly_b);

    /*if deg < deg(poly_b), then set poly_c = x^deg*/
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, deg, POLY_DEG(poly_b)) )
    {
        /*poly_c = x^deg*/
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_set_xn(polyfp_md_id, deg, poly_c);

        return ( 0 );
    }

    /*set bgn_m = deg*/
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_m, LOC_POLYFP_0124);
    bgn_z_clone(bgnz_md_id, deg, bgn_m);

    /*set poly_tmp = x */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tmp, LOC_POLYFP_0125);
    POLY_INIT(poly_tmp);
    poly_fp_set_xn_word(polyfp_md_id, 1, poly_tmp);

    /* poly_tc = 1*/
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tc, LOC_POLYFP_0126);
    POLY_INIT(poly_tc);
    poly_fp_set_one(polyfp_md_id, poly_tc);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, bgn_m );

    index = 0;


    //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre: enter while-loop...\n");
    while ( 1 )
    {
        //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre: loop # ");
        //sys_log(LOGSTDOUT,"%ld\n",index);

        e_bit = bgn_z_get_bit(bgnz_md_id, bgn_m, index);
        if ( 1 == e_bit )
        {
            //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre:     before mul\n");
            /*poly_tc = poly_tc * poly_tmp mod poly_b*/
            poly_fp_mul_self(polyfp_md_id, poly_tmp, poly_tc);
            poly_fp_mod_poly_with_pre(polyfp_md_id, g_poly_fp_mod_pre_tbl, poly_tc, poly_b, poly_tc);
            //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre:     end mul\n");
        }
        index ++;
        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= te_nbits )
        {
            break;
        }
        //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre:     before squ\n");
        /*poly_tmp = poly_tmp^2 mod poly_b*/
        poly_fp_squ_poly(polyfp_md_id, poly_tmp, poly_tmp);
        poly_fp_mod_poly_with_pre(polyfp_md_id, g_poly_fp_mod_pre_tbl, poly_tmp, poly_b, poly_tmp);
        //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre:     end squ\n");

        //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre:                  end\n");
    }
    //print_cur_time(LOGSTDOUT, "poly_fp_xm_mod_poly_with_pre: end while-loop\n");

    poly_fp_poly_clean(polyfp_md_id, poly_c);

    poly_fp_poly_move(polyfp_md_id, poly_tc, poly_c);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_tc, LOC_POLYFP_0127);
    poly_fp_poly_destory(polyfp_md_id, poly_tmp);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_m, LOC_POLYFP_0128);

    return ( 0 );
}

static EC_BOOL poly_fp_check_mod_need_pre(const UINT32 polyfp_md_id, const POLY *poly_b )
{
    DEGREE *deg_thrd;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_mod_need_pre: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_check_mod_need_pre: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_mod_need_pre: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if poly_b is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_check_mod_need_pre: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &deg_thrd, LOC_POLYFP_0129);
    POLY_ITEM_DEG_SET_WORD(bgnz_md_id, deg_thrd, 23);

    /*if deg(poly_b) >= threshold deg_thrd, then return FALSE*/
    if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_b), deg_thrd) )
    {
        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, deg_thrd, LOC_POLYFP_0130);
        return (EC_TRUE);
    }
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, deg_thrd, LOC_POLYFP_0131);
    return (EC_FALSE);
}

/**
*
*   compute poly_c = x^m mod poly_b where m is the inputed parameter deg.
*   Algorithm:
*   0. let m = SUM(m_i * 2^i, i =0,1,...k)
*   1. set t = x and c = 1;
*   2. for i = 0 ... k do
*   2.1     if m_i = 1 then
*   2.1.1       c = c * t mod b;
*   2.2     end if
*   2.3     t = t^2 mod b;
*   2.4 end do
*   3.  return c;
**/
static UINT32 poly_fp_xm_mod_poly(const UINT32 polyfp_md_id,const DEGREE *deg,const POLY *poly_b,POLY *poly_c )
{
    POLY *poly_tmp;
    POLY *poly_tc;

    POLY_ITEM *item;
    BIGINT    *bgn_coe_of_item;

    BIGINT    *bgn_m;

    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == deg )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_poly: deg is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_xm_mod_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_xm_mod_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*if poly_b is constant, then return poly_c = 0*/
    if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_DEG(poly_b)) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);

        return ( 0 );
    }

    if ( EC_TRUE == poly_fp_check_mod_need_pre(polyfp_md_id, poly_b) )
    {
    #if 0
        print_cur_time(LOGSTDOUT,"poly_fp_xm_mod_poly: ready to enter poly_fp_xm_mod_poly_with_pre.\n");
        print_cur_time(LOGSTDOUT,"poly_fp_xm_mod_poly: deg(poly_b) = ");
        print_deg_dec(LOGSTDOUT, POLY_DEG(poly_b));
    #endif
        poly_fp_xm_mod_poly_with_pre(polyfp_md_id, deg, poly_b, poly_c);

        //print_cur_time(LOGSTDOUT,"poly_fp_xm_mod_poly: after poly_fp_xm_mod_poly_with_pre.\n");
        return ( 0 );
    }

    /*set bgn_m = deg*/
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_m, LOC_POLYFP_0132);
    bgn_z_clone(bgnz_md_id, deg, bgn_m);

    /*if m < deg(poly_b), then set poly_c = x^m*/
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, bgn_m, POLY_DEG(poly_b)) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0133);
        bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0134);
        POLY_ITEM_DEG_CLONE(bgnz_md_id, deg, POLY_ITEM_DEG(item));
        POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
        POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

        POLY_ADD_ITEM_TAIL(poly_c, item);

        free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_m, LOC_POLYFP_0135);
        return ( 0 );
    }


    /*set poly_tmp = x */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item, LOC_POLYFP_0136);
    bgn_fp_set_one(bgnfp_md_id, bgn_coe_of_item);

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item, LOC_POLYFP_0137);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, POLY_ITEM_DEG(item));
    POLY_ITEM_BGN_COE(item) = bgn_coe_of_item;
    POLY_ITEM_BGN_COE_FLAG(item) = EC_TRUE;

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tmp, LOC_POLYFP_0138);
    POLY_INIT(poly_tmp);

    POLY_ADD_ITEM_TAIL(poly_tmp, item);

    /* poly_tc = 1*/
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tc, LOC_POLYFP_0139);
    POLY_INIT(poly_tc);
    poly_fp_set_one(polyfp_md_id, poly_tc);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, bgn_m );

    index = 0;

    //print_cur_time(LOGSTDOUT,"poly_fp_xm_mod_poly: before enter while-loop.\n");
    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, bgn_m, index);
        if ( 1 == e_bit )
        {
            /*poly_tc = poly_tc * poly_tmp mod poly_b*/
            poly_fp_mul_self(polyfp_md_id, poly_tmp, poly_tc);
            poly_fp_mod_poly_simple(polyfp_md_id, poly_tc, poly_b, poly_tc);
        }
        index ++;
        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= te_nbits )
        {
            break;
        }
        /*poly_tc = poly_tmp^2 mod poly_b*/
        poly_fp_squ_poly(polyfp_md_id, poly_tmp, poly_tmp);
        poly_fp_mod_poly_simple(polyfp_md_id, poly_tmp, poly_b, poly_tmp);
    }
    //print_cur_time(LOGSTDOUT,"poly_fp_xm_mod_poly: after while-loop.\n");

    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
    }

    poly_fp_poly_move(polyfp_md_id, poly_tc, poly_c);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_tc, LOC_POLYFP_0140);
    poly_fp_poly_destory(polyfp_md_id, poly_tmp);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_m, LOC_POLYFP_0141);

    return ( 0 );
}
/**
*   compute poly_c = poly_a mod poly_b;
*   where poly_a,poly_b,poly_c is one-var polynomials.
*
*   Algorithm:
*   0. let poly_a = SUM(a_i * x^i, i = 0,..,m)
*   1.  set  t1 = 0
*   2.  for i = 0..m do
*   2.1     t2 = x^i mod b
*   2.2     t2 = (t2 * a_i)
*   2.3     t1 = t1 + t2
*   3.  end do
*   4.  c = t1
**/
UINT32 poly_fp_mod_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c )
{
    POLY *poly_t1;
    POLY *poly_t2;

    POLY_ITEM *item_a;
    POLY_ITEM *item_t1;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:v: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if poly_b is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_mod_poly: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *if poly_a is empty then
    *    error "mod zero"
    *    return ERR;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    /*if poly_b is constant, then return poly_c = 0*/
    if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_DEG(poly_b)) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    /*
    *if poly_a == poly_b then
    *    clean up all items of poly_c;
    *    set poly_c to be one;
    *    return ;
    *end if
    */
    if ( poly_a == poly_b )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_set_one(polyfp_md_id, poly_c);

        return ( 0 );
    }

    /*
    *if deg(poly_a) < deg(poly_b) then
    *    clone poly_a to poly_c;
    *    return;
    *end if
    */
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), POLY_DEG(poly_b)) )
    {
        if ( poly_a != poly_c )
        {
            poly_fp_poly_clean(polyfp_md_id, poly_c);
            poly_fp_poly_clone(polyfp_md_id, poly_a, poly_c);
        }
        return ( 0 );
    }
#if 1
#if 0
    sys_log(LOGSTDOUT,"poly_fp_mod_poly: deg(poly_a) = ");
    print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_a));

    sys_log(LOGSTDOUT,"poly_fp_mod_poly: deg(poly_b) = ");
    print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_b));
#endif
    if ( EC_TRUE == poly_fp_check_valid_for_mod(polyfp_md_id, poly_a, poly_b) )
    {
        return poly_fp_mod_poly_simple(polyfp_md_id, poly_a, poly_b, poly_c);
    }
#endif
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_t1, LOC_POLYFP_0142);
    POLY_INIT(poly_t1);

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_t2, LOC_POLYFP_0143);
    POLY_INIT(poly_t2);

    if ( EC_TRUE == poly_fp_check_mod_need_pre(polyfp_md_id, poly_b) )
    {

        //print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: before pre-computation.\n");
        //print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: deg(poly_b) = ");
        //print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_b));

        poly_fp_xm_mod_pre(polyfp_md_id, MAX_NUM_OF_PRECOMP_FOR_MOD, g_poly_fp_mod_pre_tbl, poly_b);
        //print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: after pre-computation.\n");
    }

    item_a = POLY_FIRST_ITEM(poly_a);

    /*if deg(item_a) < deg(poly_b) then clone item_a to poly_t1*/
    while ( item_a != POLY_NULL_ITEM(poly_a)
        &&  0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_DEG(poly_b))
          )
    {
        alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_t1, LOC_POLYFP_0144);

        poly_fp_item_clone(polyfp_md_id, item_a, item_t1);

        POLY_ADD_ITEM_TAIL(poly_t1, item_t1);

        /*next item*/
        item_a = POLY_ITEM_NEXT(item_a);
    }

    //print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: deg(poly_b) = ");
    //print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_b));

    /*if deg(item_a) >= deg(poly_b) then */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*poly_t2 = x^i mod poly_b where i = deg(item_a)*/
#if 0
        print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: before poly_fp_xm_mod_poly.\n");
        print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: deg(poly_b) = ");
        print_bigint_dec(LOGSTDOUT, POLY_DEG(poly_b));
        print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: deg(item_a) = ");
        print_bigint_dec(LOGSTDOUT, POLY_ITEM_DEG(item_a));
#endif
        poly_fp_xm_mod_poly(polyfp_md_id, POLY_ITEM_DEG(item_a), poly_b, poly_t2);
        //print_cur_time(LOGSTDOUT,"poly_fp_mod_poly: after poly_fp_xm_mod_poly.\n");
#if 0
        sys_log(LOGSTDOUT,"poly_f_mod_poly: \n");
        sys_log(LOGSTDOUT,"x^");print_bigint_dec(LOGSTDOUT, POLY_ITEM_DEG(item_a));
        sys_log(LOGSTDOUT,"poly_b:\n");print_poly_dec(LOGSTDOUT, poly_b);
        sys_log(LOGSTDOUT,"poly_t2:\n");print_poly_dec(LOGSTDOUT, poly_t2);
#endif
        /*poly_t2 = poly_t2 * bgn_coe(item_a) = a_i * x^i mod poly_b */
        /*where a_i = bgn_coe(item_a) */
        poly_fp_mul_self_bgn(polyfp_md_id, POLY_ITEM_BGN_COE(item_a), poly_t2);

        /*poly_t1 = poly_t1 + poly_t2*/
        poly_fp_adc_poly(polyfp_md_id, poly_t2, poly_t1);

        /*clean up poly_t2*/
        poly_fp_poly_clean(polyfp_md_id, poly_t2);

        /*next item*/
        item_a = POLY_ITEM_NEXT(item_a);
    }

    /*set poly_c = poly_t1;*/
    poly_fp_poly_clean(polyfp_md_id, poly_c);
    poly_fp_poly_move(polyfp_md_id, poly_t1, poly_c);

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_t1, LOC_POLYFP_0145);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_t2, LOC_POLYFP_0146);

    return ( 0 );
}

/**
*
*
*   poly_c = poly_a ^ exp mod poly_b
*
*
**/
UINT32 poly_fp_exp_mod(const UINT32 polyfp_md_id, const POLY *poly_a, const BIGINT *exp, const POLY *poly_b, POLY *poly_c)
{
    POLY *poly_ta;
    POLY *poly_tc;

    UINT32 e_bit;
    UINT32 te_nbits;
    UINT32 index;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp_mod: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == exp )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp_mod: exp is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp_mod: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp_mod: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_exp_mod: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        return ( 0 );
    }

    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_exp_mod: poly_b is empty.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*set poly_ta = poly_a */
    poly_fp_alloc_poly(polyfp_md_id, &poly_ta);
    POLY_INIT(poly_ta);
    if ( 0 <= POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_a), POLY_DEG(poly_b)) )
    {
        poly_fp_mod_poly(polyfp_md_id, poly_a, poly_b, poly_ta);
    }
    else
    {
        poly_fp_poly_clone(polyfp_md_id, poly_a, poly_ta);
    }

    /* poly_tc = 1*/
    poly_fp_alloc_poly(polyfp_md_id, &poly_tc);
    POLY_INIT(poly_tc);
    poly_fp_set_one(polyfp_md_id, poly_tc);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, exp );

    index = 0;

    //sys_log(LOGSTDOUT,"poly_fp_exp_mod: deg(poly_ta) = ");print_deg_dec(LOGSTDOUT, POLY_DEG(poly_ta));
    //sys_log(LOGSTDOUT,"poly_fp_exp_mod: deg(poly_b) = ");print_deg_dec(LOGSTDOUT, POLY_DEG(poly_b));
    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, exp, index);
        if ( 1 == e_bit )
        {
            /*poly_tc = poly_tc * poly_ta mod poly_b*/
            poly_fp_mul_self(polyfp_md_id, poly_ta, poly_tc);
            poly_fp_mod_poly(polyfp_md_id, poly_tc, poly_b, poly_tc);
            //sys_log(LOGSTDOUT,"poly_fp_exp_mod: deg(poly_tc) = ");print_deg_dec(LOGSTDOUT, POLY_DEG(poly_tc));
        }
        index ++;
        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= te_nbits )
        {
            break;
        }
        /*poly_ta = poly_ta^2 mod poly_b*/
        poly_fp_squ_poly(polyfp_md_id, poly_ta, poly_ta);
        poly_fp_mod_poly(polyfp_md_id, poly_ta, poly_b, poly_ta);
        //sys_log(LOGSTDOUT,"poly_fp_exp_mod: deg(poly_ta) = ");print_deg_dec(LOGSTDOUT, POLY_DEG(poly_ta));
    }

    poly_fp_poly_clean(polyfp_md_id, poly_c);
    poly_fp_poly_move(polyfp_md_id, poly_tc, poly_c);
    poly_fp_free_poly(polyfp_md_id, poly_tc);
    poly_fp_poly_destory(polyfp_md_id, poly_ta);

    return ( 0 );
}

/**
*>>> poly_c = gcd(poly_a, poly_b)
*>>> wherer poly_a,poly_b,poly_c are all one-var polynomial
*
*poly_gcd :=proc(poly_a, poly_b,poly_c)
*
*if FALSE == poly_is_one_var_poly(poly_a) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_b) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*if FALSE == poly_is_one_var_poly(poly_c) then
*    error "poly with muti-var"
*    return ERR;
*end if
*
*>>> gcd(0,poly_b) = poly_b
*if poly_a is empty then
*    clean all items of poly_c;
*    poly_c = poly_monic(poly_b);
*    return ;
*end if
*
*>>> gcd(0,poly_a) = ,poly_a
*if poly_a is empty then
*    clean all items of poly_c;
*    poly_c = poly_monic(poly_a);
*    return ;
*end if
*
*>>> gcd(poly_a,poly_a) = poly_a;
*if poly_a == poly_b then
*    poly_c = poly_monic(poly_a);
*    return;
*end if
*
*new poly_ta;
*init poly_ta;
*clone poly_a to poly_ta;
*
*new poly_tb;
*init poly_tb;
*clone poly_b to poly_tb;
*
*new poly_tr;
*init poly_tr;
*
*if deg(poly_ta) < deg(poly_tb) then
*    poly_t = poly_ta;
*    poly_ta = poly_tb;
*    poly_tb = poly_t;
*end if
*
*while poly_tb is not empty do
*
*    poly_mod(poly_ta, poly_tb, poly_tr);
*    poly_t = poly_ta;
*    poly_ta = poly_tb;
*    poly_tb = poly_tr;
*    poly_tr = poly_t;
*
*end do
*
*clean all items of poly_c;
*move poly_ta to poly_c;
*
*poly_c = poly_monic(poly_c);
*
*free poly_ta;
*destory poly_tb;
*destory poly_tr;
*
*return ;
*
*end proc
**/

UINT32 poly_fp_gcd_poly(const UINT32 polyfp_md_id,const POLY *poly_a,const POLY *poly_b, POLY *poly_c)
{
    POLY *poly_ta;
    POLY *poly_tb;
    POLY *poly_tr;
    POLY *poly_t;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_b )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: poly_b is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_gcd_poly: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if FALSE == poly_is_one_var_poly(poly_a) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_a) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: not support poly_a with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_b) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_b) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: not support poly_b with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    /*
    *if FALSE == poly_is_one_var_poly(poly_c) then
    *    error "poly with muti-var"
    *    return ERR;
    *end if
    */
    if ( EC_FALSE == poly_fp_is_one_var_poly(polyfp_md_id, poly_c) )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_gcd_poly: not support poly_c with multi-var.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }

    /*
    *>>> gcd(0,poly_b) = poly_b
    *if poly_a is empty then
    *    clean all items of poly_c;
    *    poly_c = poly_monic(poly_b);
    *    return ;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_to_monic(polyfp_md_id, poly_b, poly_c);
        return ( 0 );
    }
    /*
    *>>> gcd(0,poly_a) = poly_a
    *if poly_a is empty then
    *    clean all items of poly_c;
    *    poly_c = poly_monic(poly_a);
    *    return ;
    *end if
    */
    if ( EC_TRUE == POLY_IS_EMPTY(poly_b) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
        poly_fp_to_monic(polyfp_md_id, poly_a, poly_c);
        return ( 0 );
    }
    /*
    *>>> gcd(poly_a,poly_a) = poly_a;
    *if poly_a == poly_b then
    *    poly_c = poly_monic(poly_a);
    *    return;
    *end if
    */
    if ( poly_a == poly_b )
    {
        /*here Must Not do poly_fp_poly_clean to admit the possibility of poly_a = poly_c*/
        poly_fp_to_monic(polyfp_md_id, poly_a, poly_c);
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;
    /*
    *new poly_ta;
    *init poly_ta;
    *clone poly_a to poly_ta;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_ta, LOC_POLYFP_0147);
    POLY_INIT(poly_ta);
    poly_fp_poly_clone(polyfp_md_id, poly_a, poly_ta);

    /*
    *new poly_tb;
    *init poly_tb;
    *clone poly_b to poly_tb;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tb, LOC_POLYFP_0148);
    POLY_INIT(poly_tb);
    poly_fp_poly_clone(polyfp_md_id, poly_b, poly_tb);
    /*
    *new poly_tr;
    *init poly_tr;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_tr, LOC_POLYFP_0149);
    POLY_INIT(poly_tr);
    /*
    *if deg(poly_ta) < deg(poly_tb) then
    *    poly_t = poly_ta;
    *    poly_ta = poly_tb;
    *    poly_tb = poly_t;
    *end if
    */
    if ( 0 > POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_ta), POLY_DEG(poly_tb)) )
    {
        poly_t  = poly_ta;
        poly_ta = poly_tb;
        poly_tb = poly_t ;
    }
    if ( 0 == POLY_ITEM_DEG_CMP(bgnz_md_id, POLY_DEG(poly_ta), POLY_DEG(poly_tb)) )
    {
        poly_fp_sub_poly(polyfp_md_id, poly_ta, poly_tb, poly_tr);

        poly_t  = poly_ta;
        poly_ta = poly_tb;
        poly_tb = poly_tr;
        poly_tr = poly_t ;
    }
#if 1
    if ( EC_FALSE == poly_fp_check_valid_for_mod(polyfp_md_id, poly_ta, poly_tb) )
    {
        /*do mod operation once and close the deg(poly_ta) to deg(poly_tb)*/
        poly_fp_mod_poly(polyfp_md_id, poly_ta, poly_tb, poly_tr);
        poly_t  = poly_ta;
        poly_ta = poly_tb;
        poly_tb = poly_tr;
        poly_tr = poly_t ;
    }


#endif
    /*
    *while poly_tb is not empty do
    *
    *    poly_mod(poly_ta, poly_tb, poly_tr);
    *    poly_t = poly_ta;
    *    poly_ta = poly_tb;
    *    poly_tb = poly_tr;
    *    poly_tr = poly_t;
    *
    *end do
    */
    while ( EC_FALSE == POLY_IS_EMPTY(poly_tb) )
    {
        /*here must call poly_fp_mod_poly but not poly_fp_mod_poly_simple*/
        /*since poly_fp_mod_poly can deal with big degree polynomial but the latter cannot*/
        poly_fp_mod_poly_simple(polyfp_md_id, poly_ta, poly_tb, poly_tr);

        poly_t  = poly_ta;
        poly_ta = poly_tb;
        poly_tb = poly_tr;
        poly_tr = poly_t ;
    }

    /*
    *clean all items of poly_c;
    *move poly_ta to poly_c;
    *poly_c = poly_monic(poly_c);
    */
    poly_fp_poly_clean(polyfp_md_id, poly_c);
    poly_fp_poly_move(polyfp_md_id, poly_ta, poly_c);
    poly_fp_to_monic(polyfp_md_id, poly_c, poly_c);

    /*
    *free poly_ta;
    *destory poly_tb;
    *destory poly_tr;
    */
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_ta, LOC_POLYFP_0150);
    poly_fp_poly_destory(polyfp_md_id, poly_tb);
    poly_fp_poly_destory(polyfp_md_id, poly_tr);

    return ( 0 );
}

/*Partial derivatives */
/**
*
*   poly_c = d(poly_c)/dx where x is the first var.
*
*poly_fp_dx_self := proc(poly_c)
*
*item_c = first_item(item_c);
*while item_c != null_item(item_c) do
*    if deg(item_c) is zero then
*        item_t = item_c;
*        item_c = prev_item(item_c);
*        remove item_t from poly_c;
*        destory item_t;
*    else
*        if TRUE == bgn_coe_flag(item_c) then
*            bgn_mul(deg(item_c), bgn_coe(item_c), bgn_coe(item_c));
*
*            if bgn_coe(item_c) is zero then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            else
*                deg(item_c) = deg(item_c) - 1;
*            end if
*        else
*            poly_mul_bgn(deg(item_c), poly_coe(item_c), poly_coe(item_c));
*
*            if poly_coe(item_c) is empty then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            else
*                deg(item_c) = deg(item_c) - 1;
*            end if
*        end if
*    end if
*    item_c = next_item(item_c);
*end do
*
*end proc
*
**/
static UINT32 poly_fp_dx_self(const UINT32 polyfp_md_id, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    DEGREE *deg_one;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_dx_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &deg_one, LOC_POLYFP_0151);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_c = POLY_FIRST_ITEM(poly_c);

    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *    if deg(item_c) is zero then
        *        item_t = item_c;
        *        item_c = prev_item(item_c);
        *        remove item_t from poly_c;
        *        destory item_t;
        *   end if
        */
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c)) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);

            poly_fp_item_destory(polyfp_md_id, item_t);
        }
        else
        {
            /*
            *        if TRUE == bgn_coe_flag(item_c) then
            *            bgn_mul(deg(item_c), bgn_coe(item_c), bgn_coe(item_c));
            *
            *            if bgn_coe(item_c) is zero then
            *                item_t = item_c;
            *                item_c = prev_item(item_c);
            *                remove item_t from poly_c;
            *                destory item_t;
            *            else
            *                deg(item_c) = deg(item_c) - 1;
            *            end if
            *        end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c))
            {
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), POLY_ITEM_DEG(item_c), POLY_ITEM_BGN_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                bgn_fp_smul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), (*POLY_ITEM_DEG(item_c)), POLY_ITEM_BGN_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

                if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);

                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
                else
                {
                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg_one, POLY_ITEM_DEG(item_c));
                }
            }
            /*
            *        if FALSE == bgn_coe_flag(item_c) then
            *            poly_mul_bgn(deg(item_c), poly_coe(item_c), poly_coe(item_c));
            *
            *            if poly_coe(item_c) is empty then
            *                item_t = item_c;
            *                item_c = prev_item(item_c);
            *                remove item_t from poly_c;
            *                destory item_t;
            *            else
            *                deg(item_c) = deg(item_c) - 1;
            *            end if
            *        end if
            */
            else
            {
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_c), POLY_ITEM_DEG(item_c), POLY_ITEM_POLY_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                poly_fp_mul_word(polyfp_md_id, POLY_ITEM_POLY_COE(item_c), (*POLY_ITEM_DEG(item_c)), POLY_ITEM_POLY_COE(item_c));
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

                if ( EC_TRUE == POLY_IS_EMPTY( POLY_ITEM_POLY_COE(item_c) ) )
                {
                    item_t = item_c;
                    item_c = POLY_ITEM_PREV(item_c);

                    POLY_ITEM_DEL(item_t);

                    poly_fp_item_destory(polyfp_md_id, item_t);
                }
                else
                {
                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg_one, POLY_ITEM_DEG(item_c));
                }
            }
        }
        /*
        *    item_c = next_item(item_c);
        */
        item_c = POLY_ITEM_NEXT(item_c);
    }

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, deg_one, LOC_POLYFP_0152);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_a)/dx where x is the first var.
*
*poly_fp_dx := proc(poly_a, poly_c)
*
*if poly_a == poly_c then
*    poly_fp_dx_self(poly_c);
*    return ;
*end if
*
*if poly_c is not empty then
*    clean up the items of poly_c;
*end if
*
*item_a = first_item(poly_a);
*while item_a != null_item(poly_a) do
*    if deg(item_a) is nonzero then
*        if TRUE == bgn_coe_flag(item_a) then
*            new bgn_coe_of_item_c;
*            bgn_mul(deg(item_a), bgn_coe(item_a), bgn_coe_of_item_c);
*
*            if bgn_coe_of_item_c is zero then
*                free bgn_coe_of_item_c;
*            else
*                new item_c;
*                deg(item_c) = deg(item_a) - 1;
*                bgn_coe(item_c) = bgn_coe_of_item_c;
*                bgn_coe_flag(item_c) = TRUE;
*            end if
*        else
*            new poly_coe_of_item_c;
*
*            poly_mul_bgn(deg(item_a), poly_coe(item_a), poly_coe_of_item_c);
*
*            if poly_coe_of_item_c is empty then
*                free poly_coe_of_item_c;
*            else
*                new item_c;
*                deg(item_c) = deg(item_a) - 1;
*                poly_coe(item_c) = poly_coe_of_item_c;
*                bgn_coe_flag(item_c) = FALSE;
*            end if
*        end if
*        insert item_c to the tail of poly_c;
*    end if
*
*    item_a = next_item(item_a);
*end do
*
*end proc
*
**/
UINT32 poly_fp_dx(const UINT32 polyfp_md_id,const POLY *poly_a, POLY *poly_c )
{
    POLY_ITEM   *item_a;
    POLY_ITEM   *item_c;

    POLY        *poly_coe_of_item_c;
    BIGINT      *bgn_coe_of_item_c;

    DEGREE      *deg_one;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_dx: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_fp_Dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_dx_self(polyfp_md_id, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
    }

    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &deg_one, LOC_POLYFP_0153);
    POLY_ITEM_DEG_SET_ONE(bgnz_md_id, deg_one);

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        if ( EC_FALSE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_a)) )
        {
            /*
            *        if TRUE == bgn_coe_flag(item_a) then
            *            new bgn_coe_of_item_c;
            *            bgn_mul(deg(item_a), bgn_coe(item_a), bgn_coe_of_item_c);
            *
            *            if bgn_coe_of_item_c is zero then
            *                free bgn_coe_of_item_c;
            *            else
            *                new item_c;
            *                deg(item_c) = deg(item_a) - 1;
            *                bgn_coe(item_c) = bgn_coe_of_item_c;
            *                bgn_coe_flag(item_c) = TRUE;
            *                insert item_c to the tail of poly_c;
            *            end if
            */
            if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0154);

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), POLY_ITEM_DEG(item_a), bgn_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                bgn_fp_smul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), (*POLY_ITEM_DEG(item_a)), bgn_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

                if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c) )
                {
                    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0155);
                }
                else
                {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0156);

                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg_one, POLY_ITEM_DEG(item_c));
                    POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
            /*
            *        else
            *            new poly_coe_of_item_c;
            *            init poly_coe_of_item_c;
            *
            *            poly_mul_bgn(deg(item_a), poly_coe(item_a), poly_coe_of_item_c);
            *
            *            if poly_coe_of_item_c is empty then
            *                free poly_coe_of_item_c;
            *            else
            *                new item_c;
            *                deg(item_c) = deg(item_a) - 1;
            *                poly_coe(item_c) = poly_coe_of_item_c;
            *                bgn_coe_flag(item_c) = FALSE;
            *                insert item_c to the tail of poly_c;
            *            end if
            *        end if
            */
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0157);
                POLY_INIT(poly_coe_of_item_c);

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
                poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), POLY_ITEM_DEG(item_a), poly_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/
#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
                poly_fp_mul_word(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), (*POLY_ITEM_DEG(item_a)), poly_coe_of_item_c);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/


                if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
                {
                    free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0158);
                }
                else
                {
                    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0159);

                    POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg_one, POLY_ITEM_DEG(item_c));
                    POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                    POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                    POLY_ADD_ITEM_TAIL(poly_c, item_c);
                }
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, deg_one, LOC_POLYFP_0160);

    return ( 0 );
}

/**
*
*   poly_c = d(poly_c)/dx where x is the depth_of_x-th var.
*
*poly_fp_Dx_self := proc(depth_of_x, poly_c)
*
*if 0 == depth_of_x then
*    poly_fp_dx_self(poly_c);
*    return ;
*end if
*
*item_c = first_item(item_c);
*while item_c != null_item(item_c) do
*
*        if TRUE == bgn_coe_flag(item_c) then
*            item_t = item_c;
*            item_c = prev_item(item_c);
*            remove item_c from poly_c;
*            destory item_t;
*        else
*            poly_fp_Dx_self(depth_of_x - 1, poly_coe(item_c));
*
*            if poly_coe(item_c) is empty then
*                item_t = item_c;
*                item_c = prev_item(item_c);
*                remove item_t from poly_c;
*                destory item_t;
*            end if
*        end if
*        item_c = next_itm(item_c);
*end do
*
*end proc
*
**/
static UINT32 poly_fp_Dx_self(const UINT32 polyfp_md_id,const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_Dx_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_Dx_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if 0 == depth_of_x then
    *    poly_fp_dx_self(poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_fp_dx_self(polyfp_md_id, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    item_c = POLY_FIRST_ITEM(poly_c);
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *        if TRUE == bgn_coe_flag(item_c) then
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *            remove item_c from poly_c;
        *            destory item_t;
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_PREV(item_c);

            POLY_ITEM_DEL(item_t);
            poly_fp_item_destory(polyfp_md_id, item_t);
        }
        /*
        *        else
        *            poly_fp_Dx_self(depth_of_x - 1, poly_coe(item_c));
        *
        *            if poly_coe(item_c) is empty then
        *                item_t = item_c;
        *                item_c = prev_item(item_c);
        *                remove item_t from poly_c;
        *                destory item_t;
        *            end if
        *        end if
        */
        else
        {
            poly_fp_Dx_self(polyfp_md_id, depth_of_x - 1, POLY_ITEM_POLY_COE(item_c));
            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
        }

        /*
        *    item_c = next_itm(item_c);
        */
        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
*
*   poly_c = d(poly_a)/dx where x is the depth_of_x-th var.
*
*poly_fp_Dx := proc(poly_a, depth_of_x, poly_c)
*
*if poly_a == poly_c then
*    poly_fp_Dx_self(depth_of_x, poly_c);
*    return ;
*end if
*
*if 0 == depth_of_x then
*    poly_fp_Dx(poly_a, poly_c);
*    return ;
*end if
*
*if poly_c is not empty then
*    clean up the items of poly_c;
*end if
*
*item_a = first_item(poly_a);
*while item_a != null_item(poly_a) do
*        if FALSE == bgn_coe_flag(item_a) then
*            new  poly_t
*            init poly_t
*
*            poly_fp_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
*
*            if poly_t is empty then
*                free poly_t
*            else
*                new item_c;
*                deg(item_c) = deg(item_a);
*
*                poly_coe(item_c) = poly_t;
*                bgn_coe_flag(item_c) = FALSE;
*                insert item_c to the tail of poly_c;
*            end if
*        end if
*
*        item_a = next_itm(item_a);
*end do
*end proc
**/
UINT32 poly_fp_Dx(const UINT32 polyfp_md_id,const POLY *poly_a, const UINT32 depth_of_x, POLY *poly_c )
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;
    POLY      *poly_t;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_Dx: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_Dx: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_Dx: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_fp_Dx_self(depth_of_x, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_Dx_self(polyfp_md_id, depth_of_x, poly_c);
    }

    /*
    *if 0 == depth_of_x then
    *    poly_fp_dx(poly_a, poly_c);
    *    return ;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_fp_dx( polyfp_md_id, poly_a, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    /*
    *if poly_c is not empty then
    *    clean up the items of poly_c;
    *end if
    */
    if ( EC_FALSE == POLY_IS_EMPTY(poly_c) )
    {
        poly_fp_poly_clean(polyfp_md_id, poly_c);
    }

    item_a = POLY_FIRST_ITEM(poly_a);
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *        if FALSE == bgn_coe_flag(item_a) then
        *            new  poly_t
        *            init poly_t
        *
        *            poly_fp_Dx(poly_coe(item_a), depth_of_x - 1, poly_t);
        *
        *            if poly_t is empty then
        *                free poly_t
        *            else
        *                new item_c;
        *                deg(item_c) = deg(item_a);
        *
        *                poly_coe(item_c) = poly_t;
        *                bgn_coe_flag(item_c) = FALSE;
        *                insert item_c to the tail of poly_c;
        *            end if
        *        end if
        */
        if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_t, LOC_POLYFP_0161);
            POLY_INIT(poly_t);

            poly_fp_Dx(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), depth_of_x - 1, poly_t);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_t) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_t, LOC_POLYFP_0162);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0163);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));

                POLY_ITEM_POLY_COE(item_c) = poly_t;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

/**
*>>> let poly = P(x0,x1,...)
*>>> input x0 = n and return P(x0=n,x1,...)
*>>>
*poly_eval_x_self := proc(n, poly_c)
*
*new bgn_tn;
*set bgn_tn to be one;
*
*new deg;
*set deg to be zero;
*
*new delta_deg;
*new bgn_tmp;
*
*item_c = first_item(poly_c);
*
*>>> this is a trick: add a null item before the first item of poly_c.
*new item_t;
*new bgn_coe_of_item_t;
*
*set bgn_coe_of_item_t to be zero;
*deg(item_t) = 0;
*bgn_coe(item_t) = bgn_coe_of_item_t;
*bgn_coe_flag(item_t) = TRUE;
*
*add item_t to the head of poly_c;
*
*while item_c != null_item(poly_c) do
*
*    delta_deg = deg(item_c) - deg;
*    bgn_tmp = (bgn_n) ^ (delta_deg);
*    bgn_tn = (bgn_tn) * (bgn_tmp);
*
*    clone deg(item_c) to deg;
*
*
*    if bgn_tn is one then
*
*        item_t = item_c;
*        item_c = next_item(item_c);
*        remove item_t from poly_c;
*
*        deg(item_t) = 0;
*        insert item_t to poly_c;
*
*        continue;
*    end if
*
*    if TRUE == bgn_coe_flag(item_c) then
*        bgn_coe(item_c) = bgn_coe(item_c) * bgn_tn;
*
*        if bgn_coe(item_c) is zero then
*            item_t = item_c;
*            item_c = prev_next(item_c);
*
*            remove item_t from poly_c;
*            destory item_t;
*        else
*            item_t = item_c;
*            item_c = next_item(item_c);
*
*            remove item_t from poly_c;
*
*            deg(item_t) = 0;
*            insert item_t to poly_c;
*
*        end if
*    else
*
*        poly_coe(item_a) = poly_coe(item_a) * bgn_tn;
*
*        if poly_coe(item_a) is empty then
*            item_t = item_c;
*            item_c = next_item(item_c);
*
*            remove item_t from poly_c;
*            destory item_t;
*        else
*            item_t = item_c;
*            item_c = next_item(item_c);
*
*            remove item_t from poly_c;
*
*            deg(item_t) = 0;
*            insert item_t to poly_c;
*        end if
*
*    end if
*
*end do
*
*free bgn_tn;
*free deg;
*free delta_deg
*
*end proc
**/
static UINT32 poly_fp_eval_x_self(const UINT32 polyfp_md_id, const BIGINT *bgn_n, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

    BIGINT *bgn_tmp;

    BIGINT *bgn_tn;
    DEGREE *deg;
    DEGREE *delta_deg;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_x_self: bgn_n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_x_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_eval_x_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    if ( EC_TRUE == POLY_IS_EMPTY(poly_c) )
    {
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_n) )
    {
        item_c = POLY_FIRST_ITEM(poly_c);
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c)) )
        {
            POLY_ITEM_DEL(item_c);

            poly_fp_poly_clean(polyfp_md_id, poly_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
        }
        else
        {
            poly_fp_poly_clean(polyfp_md_id, poly_c);
        }
        return ( 0 );
    }

    /*
    *new bgn_tn;
    *set bgn_tn to be one;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_tn, LOC_POLYFP_0164);
    bgn_z_set_one(bgnz_md_id, bgn_tn);

    /*
    *new bgn_tmp
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_tmp, LOC_POLYFP_0165);

    /*
    *new deg;
    *set deg to be zero;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &deg, LOC_POLYFP_0166);
    POLY_ITEM_DEG_SET_ZERO( bgnz_md_id, deg);

    /*
    *new delta_deg;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &delta_deg, LOC_POLYFP_0167);

    /*
    *item_c = first_item(poly_c);
    */
    item_c = POLY_FIRST_ITEM(poly_c);

    /*
    *while item_c != null_item(poly_c) do
    */
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        /*
        *    delta_deg = deg(item_c) - deg;
        *    bgn_tmp = (bgn_n) ^ (delta_deg);
        *    bgn_tn = (bgn_tn) * (bgn_tmp);
        *    clone deg(item_c) to deg;
        */
        POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_c), deg, delta_deg);
        /*x^n = (x^i) *(x^(n-i))*/

#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
        bgn_fp_exp(bgnfp_md_id, bgn_n, delta_deg, bgn_tmp);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
        bgn_fp_sexp(bgnfp_md_id, bgn_n, (*delta_deg), bgn_tmp);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

        bgn_fp_mul(bgnfp_md_id, bgn_tn, bgn_tmp, bgn_tn);

        POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_c), deg);

        /*
        *    if bgn_tn is one then
        *
        *        item_t = item_c;
        *        item_c = prev_item(item_c);
        *        remove item_t from poly_c;
        *
        *        deg(item_t) = 0;
        *        insert item_t to poly_c;
        *
        *        item_c = next_item(item_c);
        *        continue;
        *    end if
        */
        if ( EC_TRUE == bgn_fp_is_one(bgnfp_md_id, bgn_tn) )
        {
            item_t = item_c;
            item_c = POLY_ITEM_NEXT(item_c);

            POLY_ITEM_DEL(item_t);

            POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_t));

            poly_fp_item_insert(polyfp_md_id, item_t, poly_c);

            continue;
        }
        /*
        *    if TRUE == bgn_coe_flag(item_c) then
        *        bgn_coe(item_c) = bgn_coe(item_c) * bgn_tn;
        *
        *        if bgn_coe(item_c) is zero then
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *
        *            remove item_t from poly_c;
        *            destory item_t;
        *        else
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *
        *            remove item_t from poly_c;
        *
        *            deg(item_t) = 0;
        *            insert item_t to poly_c;
        *
        *        end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c), bgn_tn, POLY_ITEM_BGN_COE(item_c));

            if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, POLY_ITEM_BGN_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
            else
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);

                POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_t));

                poly_fp_item_insert(polyfp_md_id, item_t, poly_c);
            }
        }
        /*
        *    else
        *
        *        poly_coe(item_c) = poly_coe(item_c) * bgn_tn;
        *
        *        if poly_coe(item_c) is empty then
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *
        *
        *            remove item_t from poly_c;
        *            destory item_t;
        *        else
        *            item_t = item_c;
        *            item_c = prev_item(item_c);
        *
        *            remove item_t from poly_c;
        *
        *            deg(item_t) = 0;
        *            insert item_t to poly_c;
        *        end if
        *
        *    end if
        */
        else
        {
            poly_fp_mul_self_bgn(polyfp_md_id, bgn_tn, POLY_ITEM_POLY_COE(item_c));

            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);
                poly_fp_item_destory(polyfp_md_id, item_t);
            }
            else
            {
                item_t = item_c;
                item_c = POLY_ITEM_NEXT(item_c);

                POLY_ITEM_DEL(item_t);

                POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_t));

                poly_fp_item_insert(polyfp_md_id, item_t, poly_c);
            }
        }
    }

    /*
    *free bgn_tn;
    *free bgn_tmp
    *free deg;
    *free delta_deg
    */
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_tn, LOC_POLYFP_0168);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_tmp, LOC_POLYFP_0169);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, deg, LOC_POLYFP_0170);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, delta_deg, LOC_POLYFP_0171);

    return ( 0 );
}

/**
*>>> let poly = P(x0,x1,...)
*>>> input x0 = n and return P(x0=n,x1,...)
*>>> poly_c = poly_a(x=n) where x is the first var of polynomial.
*
*poly_eval_x := proc(poly_a, n, poly_c)
*
*if poly_a == poly_c then
*
*    poly_eval_x_self(n,poly_c);
*    return ;
*end if
*
*clean all items of poly_c;
*
*new bgn_tn;
*set bgn_tn to be one;
*
*new bgn_zero;
*set bgn_zero to be zero;
*
*new delta_deg;
*new bgn_tmp
*
*deg = bgn_zero;
*
*item_a = first_item(poly_a);
*
*while item_a != null_item(poly_a) do
*
*    delta_deg = deg(item_a) - deg;
*    bgn_tmp = (bgn_n) ^ (delta_deg);
*    bgn_tn = (bgn_tn) * (bgn_tmp);
*
*    deg = deg(item_a);
*
*
*    if bgn_tn is one then
*        new item_c;
*
*        clone item_a to item_c;
*        deg(item_c) = 0;
*
*        insert item_c to poly_c;
*
*        item_a = next_item(item_a);
*        continue;
*    end if
*
*    if TRUE == bgn_coe_flag(item_a) then
*        new bgn_coe_of_item_c;
*        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_tn;
*
*        if bgn_coe_of_item_c is zero then
*            free bgn_coe_of_item_c
*        else
*            new item_c;
*            deg(item_c) = 0;
*            bgn_coe(item_c) = bgn_coe_of_item_c;
*            bgn_coe_flag(item_c) = TRUE;
*
*            insert item_c to poly_c;
*
*        end if
*    else
*        new poly_coe_of_item_c;
*        init poly_coe_of_item_c;
*
*        poly_coe_of_item_c = poly_coe(item_a) * bgn_tn;
*
*        if poly_coe_of_item_c is empty then
*            free poly_coe_of_item_c
*        else
*            new item_c;
*            deg(item_c) = 0;
*            poly_coe(item_c) = poly_coe_of_item_c;
*            bgn_coe_flag(item_c) = FALSE;
*
*            insert item_c to poly_c;
*        end if
*
*    end if
*
*
*    item_a = next_item(item_a);
*end do
*
*free bgn_tn;
*free bgn_zero;
*free delta_deg;
*free bgn_tmp
*
*end proc
**/
UINT32 poly_fp_eval_x(const UINT32 polyfp_md_id,const POLY *poly_a, const BIGINT *bgn_n, POLY *poly_c )
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

    BIGINT *bgn_coe_of_item_c;
    POLY   *poly_coe_of_item_c;

    BIGINT *bgn_tmp;
    BIGINT *bgn_tn;

    DEGREE *deg_zero;
    DEGREE *delta_deg;
    DEGREE *deg;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_x: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == bgn_n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_x: bgn_n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_x: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_eval_x: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /**
    *if poly_a == poly_c then
    *
    *    poly_eval_x_self(n,poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_eval_x_self(polyfp_md_id, bgn_n, poly_c);
    }

    /*
    *clean all items of poly_c;
    */
    poly_fp_poly_clean(polyfp_md_id, poly_c);

    if ( EC_TRUE == POLY_IS_EMPTY(poly_a) )
    {
        return ( 0 );
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;
    bgnfp_md_id = polyfp_md->bgnfp_md_id;

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_n) )
    {
        /*note: here poly_a is not empty*/
        item_a = POLY_FIRST_ITEM(poly_a);
        if ( EC_TRUE == POLY_ITEM_DEG_IS_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_a)) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0172);

            poly_fp_item_clone(polyfp_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
        }
        else
        {
            ;/*poly_c is already empty when cleaned up before here*/
        }
        return ( 0 );
    }

    /*
    *new bgn_tn;
    *set bgn_tn to be one;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_tn, LOC_POLYFP_0173);
    bgn_z_set_one(bgnz_md_id, bgn_tn);

    /*
    *new bgn_tmp;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_tmp, LOC_POLYFP_0174);

    /*
    *new deg_zero;
    *set deg_zero to be zero;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &deg_zero, LOC_POLYFP_0175);
    POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, deg_zero);

    /*
    *new delta_deg;
    */
    alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, &delta_deg, LOC_POLYFP_0176);

    /*
    *deg = bgn_zero;
    */
    deg = deg_zero;

    /*
    *item_a = first_item(poly_a);
    */
    item_a = POLY_FIRST_ITEM(poly_a);

    /*
    *while item_a != null_item(poly_a) do
    */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *    delta_deg = deg(item_a) - deg;
        *    bgn_tmp = (bgn_n) ^ (delta_deg);
        *    bgn_tn = (bgn_tn) * (bgn_tmp);
        *
        *    deg = deg(item_a);
        */
        POLY_ITEM_DEG_SUB(bgnz_md_id, POLY_ITEM_DEG(item_a), deg, delta_deg);
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
        /*x^n = (x^i) *(x^(n-i))*/
        bgn_fp_exp(bgnfp_md_id, bgn_n, delta_deg, bgn_tmp);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
        /*x^n = (x^i) *(x^(n-i))*/
        bgn_fp_sexp(bgnfp_md_id, bgn_n, (*delta_deg), bgn_tmp);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

        bgn_fp_mul(bgnfp_md_id, bgn_tn, bgn_tmp, bgn_tn);
        deg = POLY_ITEM_DEG(item_a);

        /*
        *    if bgn_tn is one then
        *        new item_c;
        *
        *        clone item_a to item_c;
        *        deg(item_c) = 0;
        *
        *        insert item_c to poly_c;
        *
        *        item_a = next_item(item_a);
        *        continue;
        *    end if
        */
        if ( EC_TRUE == bgn_z_is_one(bgnz_md_id, bgn_tn) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0177);

            poly_fp_item_clone(polyfp_md_id, item_a, item_c);
            POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));

            poly_fp_item_insert(polyfp_md_id, item_c, poly_c);

            item_a = POLY_ITEM_NEXT(item_a);
            continue;
        }
        /*
        *    if TRUE == bgn_coe_flag(item_a) then
        *        new bgn_coe_of_item_c;
        *        bgn_coe_of_item_c = bgn_coe(item_a) * bgn_tn;
        *
        *        if bgn_coe_of_item_c is zero then
        *            free bgn_coe_of_item_c
        *        else
        *            new item_c;
        *            deg(item_c) = 0;
        *            bgn_coe(item_c) = bgn_coe_of_item_c;
        *            bgn_coe_flag(item_c) = TRUE;
        *
        *            insert item_c to poly_c;
        *
        *        end if
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, &bgn_coe_of_item_c, LOC_POLYFP_0178);

            bgn_fp_mul(bgnfp_md_id, POLY_ITEM_BGN_COE(item_a), bgn_tn, bgn_coe_of_item_c);

            if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, bgn_coe_of_item_c) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_coe_of_item_c, LOC_POLYFP_0179);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0180);

                POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));
                POLY_ITEM_BGN_COE(item_c) = bgn_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_TRUE;

                poly_fp_item_insert(polyfp_md_id, item_c, poly_c);
            }
        }

        /*
        *    else
        *        new poly_coe_of_item_c;
        *        init poly_coe_of_item_c;
        *
        *        poly_coe_of_item_c = poly_coe(item_a) * bgn_tn;
        *
        *        if poly_coe_of_item_c is empty then
        *            free poly_coe_of_item_c
        *        else
        *            new item_c;
        *            deg(item_c) = 0;
        *            poly_coe(item_c) = poly_coe_of_item_c;
        *            bgn_coe_flag(item_c) = FALSE;
        *
        *            insert item_c to poly_c;
        *        end if
        *
        *    end if
        */
        else
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0181);
            POLY_INIT(poly_coe_of_item_c);

            poly_fp_mul_bgn(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), bgn_tn, poly_coe_of_item_c);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c))
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0182);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0183);

                POLY_ITEM_DEG_SET_ZERO(bgnz_md_id, POLY_ITEM_DEG(item_c));
                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                poly_fp_item_insert(polyfp_md_id, item_c, poly_c);
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    /*
    *free bgn_tn;
    *free bgn_tmp
    *free deg_zero;
    *free delta_deg;
    */
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_tn, LOC_POLYFP_0184);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_BIGINT, bgn_tmp, LOC_POLYFP_0185);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, deg_zero, LOC_POLYFP_0186);
    free_static_mem(MD_POLYFP, polyfp_md_id, MM_DEGREE, delta_deg, LOC_POLYFP_0187);

    return ( 0 );
}

/**
*>>> let poly = P(x0,x1,...)
*>>> input xi = n and return P(x0,x1,...,xi=n,...)
*
*poly_eval_self := proc(depth_of_x, n, poly_c)
*
*if depth_of_x = 0 then
*    poly_eval_x_self(n, poly_c);
*    return;
*end if
*
*item_c = first_item(poly_c);
*
*while  item_c != null_item(poly_c) do
*    if bgn_coe_flag(item_c) = FALSE then
*
*        poly_eval_self(depth_of_x - 1, poly_coe(item_c));
*
*        if poly_coe(item_c) is empty then
*            item_t = item_c;
*            item_c = prev_item(item_c);
*
*            remove item_t from poly_c;
*            destory item_t;
*        end if
*    end if
*
*    item_c = next_item(item_c);
*end do
*
*
*end proc
**/
static UINT32 poly_fp_eval_self(const UINT32 polyfp_md_id,const UINT32 depth_of_x, const BIGINT *bgn_n, POLY *poly_c )
{
    POLY_ITEM *item_c;
    POLY_ITEM *item_t;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == bgn_n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_self: bgn_n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval_self: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_eval_self: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if depth_of_x = 0 then
    *    poly_eval_x_self(n, poly_c);
    *    return;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_fp_eval_x_self(polyfp_md_id, bgn_n, poly_c);
    }
    /*
    *item_c = first_item(poly_c);
    */
    item_c = POLY_FIRST_ITEM(poly_c);

    /*
    *while  item_c != null_item(poly_c) do
    *    if bgn_coe_flag(item_c) = FALSE then
    *
    *        poly_eval_self(depth_of_x - 1, poly_coe(item_c));
    *
    *        if poly_coe(item_c) is empty then
    *            item_t = item_c;
    *            item_c = prev_item(item_c);
    *
    *            remove item_t from poly_c;
    *            destory item_t;
    *        end if
    *    end if
    *
    *    item_c = next_item(item_c);
    *end do
    */
    while ( item_c != POLY_NULL_ITEM(poly_c) )
    {
        if ( EC_FALSE == POLY_ITEM_BGN_COE_FLAG(item_c) )
        {
            poly_fp_eval_self(polyfp_md_id, depth_of_x - 1, bgn_n, POLY_ITEM_POLY_COE(item_c));

            if ( EC_TRUE == POLY_IS_EMPTY(POLY_ITEM_POLY_COE(item_c)) )
            {
                item_t = item_c;
                item_c = POLY_ITEM_PREV(item_c);

                POLY_ITEM_DEL(item_t);

                poly_fp_item_destory(polyfp_md_id, item_t);
            }
        }

        item_c = POLY_ITEM_NEXT(item_c);
    }

    return ( 0 );
}

/**
*>>> let poly = P(x0,x1,...)
*>>> input xi = n and return P(x0,x1,...,xi=n,...)
*
*poly_eval := proc(poly_a, depth_of_x, n, poly_c)
*
*
*if poly_a == poly_c then
*    poly_eval_self(depth_of_x, n, poly_c);
*    return ;
*end if
*
*if depth_of_x = 0 then
*    poly_eval_x(poly_a, n, poly_c);
*    return;
*end if
*
*clean all items of poly_c;
*
*item_a = first_item(poly_a);
*
*while  item_a != null_item(poly_a) do
*    if bgn_coe(item_a) = TRUE then
*        new item_c;
*        clone item_a to item_c;
*
*        insert item_c to the tail of poly_c;
*    else
*        new poly_coe_of_item_c;
*        init poly_coe_of_item_c;
*
*        poly_eval(poly_coe(item_a), depth_of_x - 1, n,poly_coe_of_item_c);
*
*        if poly_coe_of_item_c is empty then
*            free poly_coe_of_item_c
*        else
*            new item_c;
*
*            deg(item_c) = deg(item_a);
*            poly_coe(item_c) = poly_coe_of_item_c;
*            bgn_coe_flag(item_c) = FALSE;
*
*            insert item_c to the tail of poly_c;
*        end if
*    end if
*
*    item_a = next_item(item_a);
*end do
*
*end proc
**/
UINT32 poly_fp_eval(const UINT32 polyfp_md_id,const POLY *poly_a, const UINT32 depth_of_x, const BIGINT *bgn_n, POLY *poly_c )
{
    POLY_ITEM *item_a;
    POLY_ITEM *item_c;

    POLY *poly_coe_of_item_c;

    POLYFP_MD  *polyfp_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( NULL_PTR == poly_a )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval: poly_a is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == bgn_n )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval: bgn_n is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
    if ( NULL_PTR == poly_c )
    {
        sys_log(LOGSTDOUT,"error:poly_fp_eval: poly_c is NULL_PTR.\n");
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/
#if ( SWITCH_ON == POLY_DEBUG_SWITCH )
    if ( MAX_NUM_OF_POLYFP_MD <= polyfp_md_id || 0 == g_polyfp_md[ polyfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:poly_fp_eval_x: polyfp module #0x%lx not started.\n",
                polyfp_md_id);
        dbg_exit(MD_POLYFP, polyfp_md_id);
    }
#endif/*POLY_DEBUG_SWITCH*/

    /*
    *if poly_a == poly_c then
    *    poly_eval_self(depth_of_x, n, poly_c);
    *    return ;
    *end if
    */
    if ( poly_a == poly_c )
    {
        return poly_fp_eval_self(polyfp_md_id, depth_of_x, bgn_n, poly_c);
    }
    /*
    *if depth_of_x = 0 then
    *    poly_eval_x(poly_a, n, poly_c);
    *    return;
    *end if
    */
    if ( 0 == depth_of_x )
    {
        return poly_fp_eval_x(polyfp_md_id, poly_a, bgn_n, poly_c);
    }

    polyfp_md = &(g_polyfp_md[ polyfp_md_id ]);
    bgnz_md_id = polyfp_md->bgnz_md_id;

    /*
    *clean all items of poly_c;
    */
    poly_fp_poly_clean(polyfp_md_id, poly_c);

    /*
    *item_a = first_item(poly_a);
    */
    item_a = POLY_FIRST_ITEM(poly_a);

    /*
    *while  item_a != null_item(poly_a) do
    */
    while ( item_a != POLY_NULL_ITEM(poly_a) )
    {
        /*
        *    if bgn_coe_flag(item_a) = TRUE then
        *        new item_c;
        *        clone item_a to item_c;
        *
        *        insert item_c to the tail of poly_c;
        */
        if ( EC_TRUE == POLY_ITEM_BGN_COE_FLAG(item_a) )
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0188);

            poly_fp_item_clone(polyfp_md_id, item_a, item_c);

            POLY_ADD_ITEM_TAIL(poly_c, item_c);
        }
        /*
        *    else
        *        new poly_coe_of_item_c;
        *        init poly_coe_of_item_c;
        *
        *        poly_eval(poly_coe(item_a), depth_of_x - 1, n,poly_coe_of_item_c);
        *
        *        if poly_coe_of_item_c is empty then
        *            free poly_coe_of_item_c
        *        else
        *            new item_c;
        *
        *            deg(item_c) = deg(item_a);
        *            poly_coe(item_c) = poly_coe_of_item_c;
        *            bgn_coe_flag(item_c) = FALSE;
        *
        *            insert item_c to the tail of poly_c;
        *        end if
        *    end if
        */
        else
        {
            alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, &poly_coe_of_item_c, LOC_POLYFP_0189);
            POLY_INIT(poly_coe_of_item_c);

            poly_fp_eval(polyfp_md_id, POLY_ITEM_POLY_COE(item_a), depth_of_x - 1, bgn_n, poly_coe_of_item_c);

            if ( EC_TRUE == POLY_IS_EMPTY(poly_coe_of_item_c) )
            {
                free_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY, poly_coe_of_item_c, LOC_POLYFP_0190);
            }
            else
            {
                alloc_static_mem(MD_POLYFP, polyfp_md_id, MM_POLY_ITEM, &item_c, LOC_POLYFP_0191);

                POLY_ITEM_DEG_CLONE(bgnz_md_id, POLY_ITEM_DEG(item_a), POLY_ITEM_DEG(item_c));
                POLY_ITEM_POLY_COE(item_c) = poly_coe_of_item_c;
                POLY_ITEM_BGN_COE_FLAG(item_c) = EC_FALSE;

                POLY_ADD_ITEM_TAIL(poly_c, item_c);
            }
        }
        /*
        *    item_a = next_item(item_a);
        */
        item_a = POLY_ITEM_NEXT(item_a);
    }

    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

