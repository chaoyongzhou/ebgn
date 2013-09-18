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
#include "log.h"

#include "bgnz.h"
#include "bgnf2n.h"
#include "ecf2n.h"

#include "debug.h"

#include "print.h"

#include "mm.h"



/* set ecf2n_n = deg(f(x)) */
/* set bgn_f2n_f = the generator polynomial f(x) of F_{2^n}*/
static ECF2N_MD g_ecf2n_md[ MAX_NUM_OF_ECF2N_MD ];
static EC_BOOL  g_ecf2n_md_init_flag = EC_FALSE;


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_ecf2n_fx_buf   [ MAX_NUM_OF_ECF2N_MD ];
static BIGINT g_ecf2n_order_buf[ MAX_NUM_OF_ECF2N_MD ];
static ECF2N_CURVE  g_ecf2n_curve_buf[ MAX_NUM_OF_ECF2N_MD ];
static EC_CURVE_POINT g_ecf2n_curve_point_buf[ MAX_NUM_OF_ECF2N_MD ];
static EC_CURVE_POINT g_ecf2n_exp_basepoint_tbl_buf[ MAX_NUM_OF_ECF2N_MD ][MAX_NAF_ARRAY_LEN];

static int g_ecf2n_naf_buf[ MAX_NUM_OF_ECF2N_MD ][ BIGINTSIZE ];
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == ECC_PM_CTRL_SWITCH )
static EC_CURVE_POINT g_buf_for_point_mul_M_ary[ ECC_M_ARY_M >> 1 ];
#endif /* ECC_PM_CTRL_SWITCH */

static EC_CURVE_POINT *g_ecf2n_exp_basepoint_tbl[ MAX_NUM_OF_ECF2N_MD ][MAX_NAF_ARRAY_LEN];
static int  *g_ecf2n_naf_arry[ MAX_NUM_OF_ECF2N_MD ];
/**
*   for test only
*
*   to query the status of ECF2N Module
*
**/
void ec_f2n_print_module_status(const UINT32 ecf2n_md_id, LOG *log)
{
    ECF2N_MD *ecf2n_md;
    UINT32 index;

    if ( EC_FALSE == g_ecf2n_md_init_flag )
    {

        sys_log(log,"no ECF2N Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_ECF2N_MD; index ++ )
    {
        ecf2n_md = &(g_ecf2n_md[ index ]);

        if ( 0 < ecf2n_md->usedcounter )
        {
            sys_log(log,"ECF2N Module # %ld : %ld refered, refer BGNF2N Module : %ld\n",
                    index,
                    ecf2n_md->usedcounter,
                    ecf2n_md->bgnf2n_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed ECF2N module
*
*
**/
UINT32 ec_f2n_free_module_static_mem(const UINT32 ecf2n_md_id)
{
    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_free_module_static_mem: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    free_module_static_mem(MD_ECF2N, ecf2n_md_id);

    bgn_f2n_free_module_static_mem(bgnf2n_md_id);

    return 0;
}
/**
*
* start ECF2N module
*  ec defined as y ^ 2 + x * y = x ^ 3 + a * x ^ 2 + b
*
**/
UINT32 ec_f2n_start( const BIGINT *f_x, const ECF2N_CURVE *curve, const BIGINT *order, const EC_CURVE_POINT *base_point )
{
    UINT32     ecf2n_md_id;
    UINT32    bgnf2n_md_id;
    UINT32          ecf2n_n;
    BIGINT         *ecf2n_f;
    ECF2N_CURVE    *ecf2n_curve;
    BIGINT         *ecf2n_order;
    EC_CURVE_POINT *ecf2n_base_point;

    ECF2N_MD *ecf2n_md;
    EC_CURVE_POINT **exp_basepoint_tbl;
    UINT32 array_len;
    UINT32 index;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == f_x )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: f_x is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == curve )
    {
         sys_log(LOGSTDOUT,"error:ec_f2n_start: curve is NULL_PTR.\n");
         dbg_exit(MD_ECF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == order )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: order is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == base_point )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: base_point is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ERR_MODULE_ID);
    }
#endif/*ECC_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNZMOD module, then */
    /* initialize g_ecf2n_md */
    if ( EC_FALSE ==  g_ecf2n_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_ECF2N_MD; index ++ )
        {
            ecf2n_md = &(g_ecf2n_md[ index ]);

            ecf2n_md->usedcounter         = 0;
            ecf2n_md->bgnf2n_md_id        = ERR_MODULE_ID;
            ecf2n_md->ecf2n_n            = 0;
            ecf2n_md->ecf2n_f            = NULL_PTR;
            ecf2n_md->ecf2n_curve        = NULL_PTR;
            ecf2n_md->ecf2n_order        = NULL_PTR;
            ecf2n_md->ecf2n_base_point   = NULL_PTR;
        }

        /*register all functions of ECF2N module to DBG module*/
        //dbg_register_func_addr_list(g_ecf2n_func_addr_list, g_ecf2n_func_addr_list_len);

        g_ecf2n_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_ECF2N_MD; index ++ )
    {
        ecf2n_md = &(g_ecf2n_md[ index ]);

        if ( 0 == ecf2n_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( MAX_NUM_OF_ECF2N_MD <= index )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    ecf2n_n            = 0;
    ecf2n_f            = NULL_PTR;
    ecf2n_curve        = NULL_PTR;
    ecf2n_order        = NULL_PTR;
    ecf2n_base_point   = NULL_PTR;

    ecf2n_md_id = index;
    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,         &ecf2n_f           , LOC_ECF2N_0001);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_ECF2N_CURVE ,   &ecf2n_curve       , LOC_ECF2N_0002);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,         &ecf2n_order       , LOC_ECF2N_0003);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,    &ecf2n_base_point  , LOC_ECF2N_0004);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ecf2n_f            = &(g_ecf2n_fx_buf[ ecf2n_md_id ]);
    ecf2n_curve        = &(g_ecf2n_curve_buf[ ecf2n_md_id ]);
    ecf2n_order        = &(g_ecf2n_order_buf[ ecf2n_md_id ]);
    ecf2n_base_point   = &(g_ecf2n_curve_point_buf[ ecf2n_md_id ]);
#endif/* STACK_MEMORY_SWITCH */


#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == ecf2n_f )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: ecf2n_f is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == ecf2n_curve )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: ecf2n_curve is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == ecf2n_order )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: ecf2n_order is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == ecf2n_base_point )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: ecf2n_base_point is NULL_PTR.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }

#endif/*ECC_DEBUG_SWITCH*/


    bgnf2n_md_id = bgn_f2n_start( f_x );

    /* set ecf2n_f = f_x */
    bgn_f2n_clone(bgnf2n_md_id, f_x, ecf2n_f);

    /* set ecf2n_n = deg(f_x) */
    ecf2n_n = bgn_f2n_deg(bgnf2n_md_id, f_x);

    /* set ecf2n_curve = curve */
    bgn_f2n_clone(bgnf2n_md_id, &(curve->a), &(ecf2n_curve->a));
    bgn_f2n_clone(bgnf2n_md_id, &(curve->b), &(ecf2n_curve->b));

    /* set ecf2n_order = order */
    bgn_f2n_clone(bgnf2n_md_id, order, ecf2n_order);

    /* set ecf2n_base_point = base_point */
    bgn_f2n_clone(bgnf2n_md_id, &(base_point->x), &(ecf2n_base_point->x));
    bgn_f2n_clone(bgnf2n_md_id, &(base_point->y), &(ecf2n_base_point->y));

    /* set module : */
    ecf2n_md->usedcounter         = 0;
    ecf2n_md->bgnf2n_md_id        = bgnf2n_md_id;
    ecf2n_md->ecf2n_n            = ecf2n_n;
    ecf2n_md->ecf2n_f            = ecf2n_f;
    ecf2n_md->ecf2n_curve        = ecf2n_curve;
    ecf2n_md->ecf2n_order        = ecf2n_order;
    ecf2n_md->ecf2n_base_point   = ecf2n_base_point;

    /* at the first time, set the counter to 1 */
    ecf2n_md->usedcounter = 1;

    /* now, ECF2N is already started */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    exp_basepoint_tbl = (g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ]);
    array_len = sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ])/sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ][0]);
    for ( index = 0; index < array_len; index ++ )
    {
        alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, &(exp_basepoint_tbl[ index ]), LOC_ECF2N_0005);
    }
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_NAF, &(g_ecf2n_naf_arry[ ecf2n_md_id ]), LOC_ECF2N_0006);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    exp_basepoint_tbl = (g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ]);
    array_len = sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ])/sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ][0]);
    for ( index = 0; index < array_len; index ++ )
    {
        exp_basepoint_tbl[ index ] = &(g_ecf2n_exp_basepoint_tbl_buf[ ecf2n_md_id ][ index ]);
    }
    g_ecf2n_naf_arry[ ecf2n_md_id ] = g_ecf2n_naf_buf[ ecf2n_md_id ];
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == g_ecf2n_naf_arry[ ecf2n_md_id ] )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_start: g_ecf2n_naf_arry[ %ld ] is NULL_PTR.\n",ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )
    /* setup the g_ecf2n_exp_basepoint_tbl */
    exp_basepoint_tbl = g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ];
    array_len = sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ])/sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ][0]);

    ec_f2n_point_clone(ecf2n_md_id, base_point, exp_basepoint_tbl[ 0 ]);

    /* do not shorten the loop length, since for ec_f2n_point_mul interface,*/
    /* when compute k * P, the k may be greater than the ec order */
    for ( index = 1; index < array_len; index ++ )
    {
        ec_f2n_point_double( ecf2n_md_id,exp_basepoint_tbl[ index - 1 ],exp_basepoint_tbl[ index ]);
    }
#endif /*ECC_POINT_MUL_FIXED_BASE_SWITCH*/
    return ( ecf2n_md_id );
}

/**
*
* end ECF2N module
*
**/
void ec_f2n_end(const UINT32 ecf2n_md_id)
{
    UINT32          ecf2n_n;
    BIGINT         *ecf2n_f;
    ECF2N_CURVE          *ecf2n_curve;
    BIGINT         *ecf2n_order;
    EC_CURVE_POINT    *ecf2n_base_point;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    EC_CURVE_POINT **exp_basepoint_tbl;
    UINT32 array_len;
    UINT32 index;
#endif/* STATIC_MEMORY_SWITCH */

    if ( MAX_NUM_OF_ECF2N_MD < ecf2n_md_id )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_end: ecf2n_md_id = %ld is overflow.\n",ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ecf2n_md->usedcounter )
    {
        ecf2n_md->usedcounter --;

        return ;
    }

    if ( 0 == ecf2n_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_end: ecf2n_md_id = %ld is not started.\n",ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    bgnf2n_md_id        = ecf2n_md->bgnf2n_md_id;
    ecf2n_n            = ecf2n_md->ecf2n_n;
    ecf2n_f            = ecf2n_md->ecf2n_f;
    ecf2n_curve        = ecf2n_md->ecf2n_curve;
    ecf2n_order        = ecf2n_md->ecf2n_order;
    ecf2n_base_point   = ecf2n_md->ecf2n_base_point;

    bgn_f2n_end( bgnf2n_md_id );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    exp_basepoint_tbl = g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ];
    array_len = sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ])/sizeof(g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ][0]);
    for ( index = 0; index < array_len; index ++ )
    {
        free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, exp_basepoint_tbl[ index ], LOC_ECF2N_0007);
    }

    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_NAF, (g_ecf2n_naf_arry[ ecf2n_md_id ]), LOC_ECF2N_0008);
#endif/* STATIC_MEMORY_SWITCH */


#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,          ecf2n_f            , LOC_ECF2N_0009);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_ECF2N_CURVE ,    ecf2n_curve        , LOC_ECF2N_0010);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,          ecf2n_order        , LOC_ECF2N_0011);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,     ecf2n_base_point   , LOC_ECF2N_0012);
#endif/* STATIC_MEMORY_SWITCH */

    /* free module : */
    ec_f2n_free_module_static_mem(ecf2n_md_id);
    ecf2n_md->bgnf2n_md_id       = ERR_MODULE_ID;
    ecf2n_md->ecf2n_n            = 0;
    ecf2n_md->ecf2n_f            = NULL_PTR;
    ecf2n_md->ecf2n_curve        = NULL_PTR;
    ecf2n_md->ecf2n_order        = NULL_PTR;
    ecf2n_md->ecf2n_base_point   = NULL_PTR;
    ecf2n_md->usedcounter        = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   define Point (0,0) as infinit point
*   if the point P is equal to (0,0), then return EC_TRUE
*   otherwise, return EC_FALSE
*
**/

EC_BOOL ec_f2n_point_is_infinit(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P)
{
    BIGINT *x;
    BIGINT *y;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_is_infinit: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_is_infinit: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x = (BIGINT *) &(P->x);
    y = (BIGINT *) &(P->y);


    /* define Point (0,0) as infinit P */
    if( EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, x )
     && EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, y ) )
    {
        return EC_TRUE;
    }
    return EC_FALSE;
}

/**
*
*   set point P = (0,0) as the infinit point
*   return the point P
*
**/
void ec_f2n_point_set_infinit(const UINT32 ecf2n_md_id, EC_CURVE_POINT * P)
{
    BIGINT *x;
    BIGINT *y;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_set_infinit: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_set_infinit: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x = &(P->x);
    y = &(P->y);

    /* ( 0 : 0 )*/
    bgn_f2n_set_zero( bgnf2n_md_id, x );
    bgn_f2n_set_zero( bgnf2n_md_id, y );

    return ;
}

/**
*
*   define affine Point [x:y:0] as infinit affine point
*   if the z of affine point P is zero, then return EC_TRUE
*   otherwise, return EC_FALSE
*
**/
UINT32 ec_f2n_point_aff_is_infinit(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P)
{
    BIGINT *z;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_is_infinit: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_is_infinit: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    z = (BIGINT *)&(P->z);

    if ( EC_TRUE == bgn_f2n_is_zero( bgnf2n_md_id, z) )
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   set affine point P = [1:0:0] as the infinit affine point
*   return the affine point P
*
**/
void ec_f2n_point_aff_set_infinit(const UINT32 ecf2n_md_id, EC_CURVE_AFF_POINT * P)
{
    BIGINT *x;
    BIGINT *y;
    BIGINT *z;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_set_infinit: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_set_infinit: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x = &(P->x);
    y = &(P->y);
    z = &(P->z);

    /* ( 1 : 0 : 0 )*/
    bgn_f2n_set_one ( bgnf2n_md_id, x );
    bgn_f2n_set_zero( bgnf2n_md_id, y );
    bgn_f2n_set_zero( bgnf2n_md_id, z );

    return ;
}


/**
*
*   copy point src to des
*   return des
*
**/
void ec_f2n_point_clone(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * src,EC_CURVE_POINT * des)
{
    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_clone: src is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == des)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_clone: des is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_clone: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    bgn_f2n_clone(bgnf2n_md_id, &(src->x), &(des->x));
    bgn_f2n_clone(bgnf2n_md_id, &(src->y), &(des->y));

    return ;
}

/**
*
*   copy affine point src to des
*   return des
*
**/
void ec_f2n_point_aff_clone(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * src,EC_CURVE_AFF_POINT * des)
{
    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_clone: src is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == des)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_clone: des is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_clone: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    bgn_f2n_clone(bgnf2n_md_id, &(src->x), &(des->x));
    bgn_f2n_clone(bgnf2n_md_id, &(src->y), &(des->y));
    bgn_f2n_clone(bgnf2n_md_id, &(src->z), &(des->z));
}

/**
*
*   copy ec src to des over F_{2^n}
*   return des
*
**/
void ec_f2n_curve_clone(const UINT32 ecf2n_md_id, const ECF2N_CURVE * src,ECF2N_CURVE * des)
{
    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_curve_clone: src is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == des)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_curve_clone: des is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_curve_clone: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    bgn_f2n_clone(bgnf2n_md_id, &(src->a), &(des->a));
    bgn_f2n_clone(bgnf2n_md_id, &(src->b), &(des->b));

    return ;
}


/**
*
*   if point curve_1  = curve_2, then return 0
*   if point curve_1 != curve_2, then return 1
*
**/
UINT32 ec_f2n_curve_cmp(const UINT32 ecf2n_md_id, const ECF2N_CURVE * curve_1,const ECF2N_CURVE * curve_2)
{
    BIGINT *a1;
    BIGINT *b1;
    BIGINT *a2;
    BIGINT *b2;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == curve_1 )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_curve_cmp: curve_1 is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == curve_2 )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_curve_cmp: curve_2 is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_curve_cmp: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    a1 = ( BIGINT * )&(curve_1->a);
    b1 = ( BIGINT * )&(curve_1->b);

    a2 = ( BIGINT * )&(curve_2->a);
    b2 = ( BIGINT * )&(curve_2->b);

    if ( 0 != bgn_f2n_cmp(bgnf2n_md_id, a1, a2 ) )
    {
        return 1;
    }

    if ( 0 != bgn_f2n_cmp(bgnf2n_md_id, b1, b2 ) )
    {
        return 1;
    }

    return 0;/* equal */
}

/**
*
*   if point P  = Q, then return 0
*   if point P != Q, then return 1
*
**/
UINT32 ec_f2n_point_cmp(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q)
{
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *x2;
    BIGINT *y2;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_cmp: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_cmp: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_cmp: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x1 = ( BIGINT * )&(P->x);
    y1 = ( BIGINT * )&(P->y);

    x2 = ( BIGINT * )&(Q->x);
    y2 = ( BIGINT * )&(Q->y);

    if ( 0 != bgn_f2n_cmp(bgnf2n_md_id, x1, x2 ) )
    {
        return 1;
    }

    if ( 0 != bgn_f2n_cmp(bgnf2n_md_id, y1, y2 ) )
    {
        return 1;
    }

    return 0;/* equal */
}

/**
*
*   if affine point P  = Q, then return 0
*   if affine point P != Q, then return 1
*
**/
UINT32 ec_f2n_point_aff_cmp(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_AFF_POINT * Q)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
    EC_CURVE_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *tmp_P;
    EC_CURVE_POINT *tmp_Q;

    BIGINT *x1;
    BIGINT *y1;
    BIGINT *x2;
    BIGINT *y2;
    UINT32 ret;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_cmp: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_cmp: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_cmp: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp_P = &buf_1;
    tmp_Q = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,&tmp_P, LOC_ECF2N_0013);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,&tmp_Q, LOC_ECF2N_0014);
#endif/*STATIC_MEMORY_SWITCH*/

    ec_f2n_point_aff_convert( ecf2n_md_id, P, tmp_P);
    ec_f2n_point_aff_convert( ecf2n_md_id, Q, tmp_Q);

    x1 = ( BIGINT * )&(tmp_P->x);
    y1 = ( BIGINT * )&(tmp_P->y);

    x2 = ( BIGINT * )&(tmp_Q->x);
    y2 = ( BIGINT * )&(tmp_Q->y);

    ret = ec_f2n_point_cmp( ecf2n_md_id, tmp_P, tmp_Q );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,tmp_P, LOC_ECF2N_0015);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,tmp_Q, LOC_ECF2N_0016);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;/* equal */
}

/**
*
*   check point is on the elliptic curve(ec) refered by Module ID ecf2n_md_id
*   if the point is on the elliptic curve(ec), return EC_TRUE,
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_f2n_point_is_on_curve(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *t1;
    BIGINT *t2;
    BIGINT *t3;
    BIGINT *left;
    BIGINT *right;
    BIGINT *x;
    BIGINT *y;
    BIGINT *coe_a;
    BIGINT *coe_b;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

    EC_BOOL ret;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == point)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_is_on_curve: point is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_is_on_curve: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &(buf_1);
    t2 = &(buf_2);
    t3 = &(buf_3);
    left  = &(buf_4);
    right = &(buf_5);
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&t1, LOC_ECF2N_0017);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&t2, LOC_ECF2N_0018);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&t3, LOC_ECF2N_0019);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&left, LOC_ECF2N_0020);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&right, LOC_ECF2N_0021);
#endif/*STATIC_MEMORY_SWITCH*/

    x = (BIGINT *)&( point->x );
    y = (BIGINT *)&( point->y );

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    coe_a = &(ecf2n_md->ecf2n_curve->a);
    coe_b = &(ecf2n_md->ecf2n_curve->b);

    bgn_f2n_squ(bgnf2n_md_id, y,        t1     );  /* t1 = y^2      */
    bgn_f2n_mul(bgnf2n_md_id, x,        y,    t2); /* t2 = xy       */
    bgn_f2n_add(bgnf2n_md_id, t1,       t2, left); /* left = y^2 + xy */

    bgn_f2n_squ(bgnf2n_md_id, x,        t1       ); /* t1 = x^2      */
    bgn_f2n_mul(bgnf2n_md_id, x,        t1,    t2); /* t2 = x^3      */
    bgn_f2n_mul(bgnf2n_md_id, coe_a,    t1,    t3); /* t3 = a * x^2 */
    bgn_f2n_add(bgnf2n_md_id, t2,       t3,    t1); /* t1 = x^3 + a * x^2   */
    bgn_f2n_add(bgnf2n_md_id, t1,    coe_b, right); /* right = x^3 + a * x^2 + b */

    bgn_f2n_sub(bgnf2n_md_id,left, right, t1);
    if( EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, t1))
    {
        ret = EC_TRUE;
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,t1, LOC_ECF2N_0022);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,t2, LOC_ECF2N_0023);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,t3, LOC_ECF2N_0024);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,left, LOC_ECF2N_0025);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,right, LOC_ECF2N_0026);
#endif/*STATIC_MEMORY_SWITCH*/

    return ( ret );
}

/**
*   let P = (x1,y1) be a point over ec
*   let negP = (x2,y2)= -P be a point over ec
*   return the negative point negP of P
*   the ec is refered by Module ID ecf2n_md_id
*
*   note:
*       x2 = x1
*       y2 = x1 + y1 over F_{2^n}
*
**/
void ec_f2n_point_neg(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * negP)
{
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *x2;
    BIGINT *y2;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_neg: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == negP)
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_neg: negP is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_neg: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x1 = ( BIGINT * )&(P->x);
    y1 = ( BIGINT * )&(P->y);

    x2 = ( BIGINT * )&(negP->x);
    y2 = ( BIGINT * )&(negP->y);

    bgn_f2n_clone( bgnf2n_md_id, x1, x2 );
    bgn_f2n_add( bgnf2n_md_id, x1, y1, y2 );
}


/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = [x3:y3:z3]
*   return R = P + Q over ec
*
*   note :
*       1.  P address is not equal to R address
*
**/
void ec_f2n_point_mix_add(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_AFF_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *z1;
    BIGINT *x2;
    BIGINT *y2;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;
    BIGINT *tmp;
    BIGINT *coe_a;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_add: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_add: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_add: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( P == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_add: address of P and R conflict.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_mix_add: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;
    coe_a = &(ecf2n_md->ecf2n_curve->a);

    x1 = (BIGINT *)&(P->x);
    y1 = (BIGINT *)&(P->y);
    z1 = (BIGINT *)&(P->z);

    x2 = (BIGINT *)&(Q->x);
    y2 = (BIGINT *)&(Q->y);

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
#if 0
    if ( EC_TRUE == bgn_f2n_is_one( bgnf2n_md_id, z1 )
    && 0 == bgn_f2n_cmp( bgnf2n_md_id, x1, x2 )
    && 0 == bgn_f2n_cmp( bgnf2n_md_id, y1, y2 ) )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_add: P is same as Q.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif
#endif /* ECC_DEBUG_SWITCH */

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    /* if affine point P[x1:y1:z1] = 0, then R[x3:y3:z3] = Q(x2,y2) */
    if ( EC_TRUE == ec_f2n_point_aff_is_infinit(ecf2n_md_id, P))
    {
        ec_f2n_point_convert(ecf2n_md_id, Q, R);

        return;
    }

    /* if affine point Q(x2,y2) = 0, then R[x3:y3:z3] = P[x1:y1:z1] */
    if ( EC_TRUE == ec_f2n_point_is_infinit(ecf2n_md_id, Q))
    {
        ec_f2n_point_aff_clone(ecf2n_md_id, P, R);

        return;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,&tmp, LOC_ECF2N_0027);
#endif/* STATIC_MEMORY_SWITCH */

    /* z3 <== x2 * z1 + x1 ( i.e. B ) */
    bgn_f2n_mul(bgnf2n_md_id, x2, z1, z3);
    bgn_f2n_add(bgnf2n_md_id, z3, x1, z3);

    /* tmp <== z3 ^ 2 ( i.e. B ^2 ) */
    bgn_f2n_squ(bgnf2n_md_id, z3, tmp);

    /* z3 <== z1 * z3 ( i.e. C ) */
    bgn_f2n_mul(bgnf2n_md_id, z1, z3, z3);

    /* y3 <== z1 ^ 2 */
    bgn_f2n_squ(bgnf2n_md_id, z1, y3);

    /* x3 <== ( y2 * y3 + y1 ) ( i.e. A ) */
    bgn_f2n_mul(bgnf2n_md_id, y2, y3, x3);
    bgn_f2n_add(bgnf2n_md_id, x3, y1, x3);

    /* y3 <== tmp * ( z3 + a * y3 ) ( i.e. D ) */
    if ( EC_TRUE == bgn_f2n_is_one( bgnf2n_md_id, coe_a) )
    {
        bgn_f2n_add(bgnf2n_md_id, z3, y3, y3);
        bgn_f2n_mul(bgnf2n_md_id, tmp, y3, y3);
    }
    else
    {
        bgn_f2n_mul(bgnf2n_md_id, tmp, z3, y3);
    }

    /* tmp <== z3 * x3 ( i.e. E ) */
    bgn_f2n_mul(bgnf2n_md_id, z3, x3, tmp);

    /* z3 <== z3 ^ 2 ( i.e. C ^ 2 ) */
    bgn_f2n_squ(bgnf2n_md_id, z3, z3);

    /* x3 <== x3 ^2 + y3 + tmp ( i.e. X3 ) */
    bgn_f2n_squ(bgnf2n_md_id, x3, x3);
    bgn_f2n_add(bgnf2n_md_id, x3, y3, x3);
    bgn_f2n_add(bgnf2n_md_id, x3, tmp, x3);

    /* y3 <== x3 + x2 * z3 ( i.e. F )  */
    bgn_f2n_mul(bgnf2n_md_id, x2, z3, y3);
    bgn_f2n_add(bgnf2n_md_id, x3, y3, y3);

    /* tmp <== tmp * y3 ( i.e. E*F ) */
    bgn_f2n_mul(bgnf2n_md_id, tmp, y3, tmp);

    /* y3 <== x3 + y2 * z3 ( i.e. G ) */
    bgn_f2n_mul(bgnf2n_md_id, y2, z3, y3);
    bgn_f2n_add(bgnf2n_md_id, x3, y3, y3);

    /* y3 <== z3 * y3 ( i.e. z3 * G ) */
    bgn_f2n_mul(bgnf2n_md_id, z3, y3, y3);

    /* y3 <== tmp + y3 ( i.e. Y3 ) */
    bgn_f2n_add(bgnf2n_md_id, tmp, y3, y3);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT,tmp, LOC_ECF2N_0028);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = [x3:y3:z3]
*   return R = P - Q over ec
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_mix_sub(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_AFF_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_POINT *negQ;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_sub: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_sub: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mix_sub: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_mix_sub: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    negQ = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, &negQ, LOC_ECF2N_0029);
#endif/* STATIC_MEMORY_SWITCH */

    /* negQ = -Q */
    ec_f2n_point_neg(ecf2n_md_id, Q, negQ);

    /* R = P + negQ = P - Q */
    ec_f2n_point_mix_add(ecf2n_md_id, P, negQ, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, negQ, LOC_ECF2N_0030);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = 2 * P over ec
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_aff_double(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_AFF_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *z1;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;
    BIGINT *tmp;
    BIGINT *coe_a;
    BIGINT *coe_b;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_double: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_double: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( P == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_double: address of P and R conflict.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_double: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;
    coe_a = &(ecf2n_md->ecf2n_curve->a);
    coe_b = &(ecf2n_md->ecf2n_curve->b);

    x1 = (BIGINT *)&(P->x);
    y1 = (BIGINT *)&(P->y);
    z1 = (BIGINT *)&(P->z);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);


    /* if P = 0, then R = 0  */
    if ( EC_TRUE ==  ec_f2n_point_aff_is_infinit(ecf2n_md_id, P))
    {
        ec_f2n_point_aff_set_infinit(ecf2n_md_id, R);

        return;
    }
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, &tmp, LOC_ECF2N_0031);
#endif/*STATIC_MEMORY_SWITCH*/
    /* x3 <== x1 ^ 2 ( i.e. X1 ^ 2 )*/
    bgn_f2n_squ(bgnf2n_md_id, x1, x3);

    /* y3 <== z1 ^ 2 ( i.e. Z1 ^ 2 ) */
    bgn_f2n_squ(bgnf2n_md_id, z1, y3);

    /* z3 <== x3 * y3 ( i.e. Z3 ) */
    bgn_f2n_mul(bgnf2n_md_id, x3, y3, z3);

    /* y3 <== y3 ^ 2 ( i.e. Z1 ^ 4 ) */
    bgn_f2n_squ(bgnf2n_md_id, y3, y3);

    /* y3 <== y3 * b ( i.e b * Z1 ^ 4 ) */
    bgn_f2n_mul(bgnf2n_md_id, y3, coe_b, y3);

    /* x3 <== x3 ^ 2 ( i.e. X1 ^ 4 ) */
    bgn_f2n_squ(bgnf2n_md_id, x3, x3);

    /* x3 <== x3 + y3 ( i.e. X3 ) */
    bgn_f2n_add(bgnf2n_md_id, x3, y3, x3);

    /* tmp <== y1 ^ 2 ( i.e. Y1 ^2 ) */
    bgn_f2n_squ(bgnf2n_md_id, y1, tmp);

    /* tmp <== a * z3 + T + y3 ( i.e. a * Z3 + Y1 ^ 2 + b * Z1 ^ 4 ) */
    bgn_f2n_add(bgnf2n_md_id, tmp, y3, tmp);
    if ( EC_TRUE == bgn_f2n_is_one( bgnf2n_md_id, coe_a) )
    {
        bgn_f2n_add(bgnf2n_md_id, tmp, z3, tmp);
    }

    /* tmp <== x3 * tmp (i.e. X3 * ( a * Z3 + Y1 ^ 2 + b * Z1 ^ 4 )) */
    bgn_f2n_mul(bgnf2n_md_id, x3, tmp, tmp);

    /* y3 <== y3 * z3 ( i.e. b * Z1 ^ 4 * Z3) */
    bgn_f2n_mul(bgnf2n_md_id, y3, z3, y3);

    /* y3 <== y3 + tmp ( i.e. Y3 ) */
    bgn_f2n_add(bgnf2n_md_id, y3, tmp, y3);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, tmp, LOC_ECF2N_0032);
#endif/*STATIC_MEMORY_SWITCH*/

    return;
}

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = P over ec
*   where
*       x3 = x1
*       y3 = y1
*       z3 = 1
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_convert(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_AFF_POINT * R)
{
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_convert: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_convert: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_convert: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_f2n_point_is_infinit(ecf2n_md_id, P) )
    {
        ec_f2n_point_aff_set_infinit(ecf2n_md_id, R);

        return ;
    }

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x1 = (BIGINT *)&(P->x);
    y1 = (BIGINT *)&(P->y);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    bgn_f2n_clone(bgnf2n_md_id, x1, x3);
    bgn_f2n_clone(bgnf2n_md_id, y1, y3);
    bgn_f2n_set_one(bgnf2n_md_id, z3);

    return ;
}

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = (x3,y3)
*   return R = P over ec
*   where
*       x3 = x1 / (z1)
*       y3 = y1 / (z1 ^ 2)
*
*   note :
*       P address is not equal to R address
**/
void ec_f2n_point_aff_convert(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *z1;
    BIGINT *x3;
    BIGINT *y3;

    BIGINT *t1;
    BIGINT *t2;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_convert: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_convert: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_aff_convert: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    /* if P = 0, then R = 0 */
    if ( EC_TRUE == ec_f2n_point_aff_is_infinit(ecf2n_md_id, P) )
    {
        ec_f2n_point_set_infinit(ecf2n_md_id, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &buf_1;
    t2 = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, &t1, LOC_ECF2N_0033);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, &t2, LOC_ECF2N_0034);
#endif/* STATIC_MEMORY_SWITCH */

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    x1 = (BIGINT *)&(P->x);
    y1 = (BIGINT *)&(P->y);
    z1 = (BIGINT *)&(P->z);

    x3 = &(R->x);
    y3 = &(R->y);
#if 0
    /* t1 <== Z1 ^ 2 */
    bgn_f2n_squ(bgnf2n_md_id, z1, t1);

    /* x3 <== x1 / z1 (i.e. X / Z ) */
    bgn_f2n_inv(bgnf2n_md_id, z1, t2);
    bgn_f2n_mul(bgnf2n_md_id, x1, t2, x3);

    /* y3 <== y1 / t1 and let t2 = 1/t1 (i.e. Y / Z ^ 2 ) */
    bgn_f2n_inv(bgnf2n_md_id, t1, t2);
    bgn_f2n_mul(bgnf2n_md_id, y1, t2, y3);
#endif
    /* t1 <== 1/ z1 */
    bgn_f2n_inv(bgnf2n_md_id, z1, t1);

    /* t2 <== t1 ^ 2 = 1 / z1^2  */
    bgn_f2n_squ(bgnf2n_md_id, t1, t2);

    /* x3 <== x1 * t1 = x1 / z1 (i.e. X / Z ) */
    bgn_f2n_mul(bgnf2n_md_id, x1, t1, x3);

    /* y3 <== y1 * t2 = y1 / z1^2 */
    bgn_f2n_mul(bgnf2n_md_id, y1, t2, y3);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, t1, LOC_ECF2N_0035);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_BIGINT, t2, LOC_ECF2N_0036);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
**/
void ec_f2n_point_mul_naf(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
    EC_CURVE_POINT buf_3;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_AFF_POINT *Q;
    EC_CURVE_AFF_POINT *mid_result;
    EC_CURVE_AFF_POINT *tmp;

    EC_CURVE_POINT *negP;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

    int *naf_buf;
    int i;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_naf: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_naf: k is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_naf: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_mul_naf: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    naf_buf = g_ecf2n_naf_arry[ ecf2n_md_id ];

    i = bgn_f2n_naf(bgnf2n_md_id, k, naf_buf);
    if ( 0 == i)
    {
        ec_f2n_point_set_infinit(ecf2n_md_id, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    mid_result = &buf_2;
    negP = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,&Q, LOC_ECF2N_0037);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,&mid_result, LOC_ECF2N_0038);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,&negP, LOC_ECF2N_0039);
#endif/* STATIC_MEMORY_SWITCH */

    ec_f2n_point_neg(ecf2n_md_id, P, negP);

    ec_f2n_point_aff_set_infinit(ecf2n_md_id, Q);

    for ( ; i > 0; )
    {
        i -- ;

        ec_f2n_point_aff_double(ecf2n_md_id, Q, mid_result);
        tmp = Q;
        Q = mid_result;
        mid_result = tmp;

        if( 1 == naf_buf[ i ] )
        {
            ec_f2n_point_mix_add(ecf2n_md_id, Q, P, mid_result);
            tmp = Q;
            Q = mid_result;
            mid_result = tmp;
        }

        if( -1 == naf_buf[ i ] )
        {
            ec_f2n_point_mix_add(ecf2n_md_id, Q, negP, mid_result);
            tmp = Q;
            Q = mid_result;
            mid_result = tmp;
        }
    }

    ec_f2n_point_aff_convert(ecf2n_md_id, Q, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,Q, LOC_ECF2N_0040);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,mid_result, LOC_ECF2N_0041);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT,negP, LOC_ECF2N_0042);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, basepoint is fixed, so contruct a table exp_basepoint_tbl:
*       {r * basepoint: r = 2 ^ i, i = 0.. ecf2n_n = deg(f(x)) }
**/
void ec_f2n_point_mul_fix_base(const UINT32 ecf2n_md_id, const BIGINT * k,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    UINT32 len;
    UINT32 index;
    EC_CURVE_AFF_POINT *Q;
    EC_CURVE_AFF_POINT *T;
    EC_CURVE_AFF_POINT *tmp;
    int *s;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

    EC_CURVE_POINT **exp_basepoint_tbl;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_fix_base: k is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_fix_base: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */


#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_mul_fix_base: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;

    s = g_ecf2n_naf_arry[ ecf2n_md_id ];

    len = bgn_f2n_naf( bgnf2n_md_id, k, s );

    if ( 0 == len )
    {
        ec_f2n_point_set_infinit(ecf2n_md_id, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    T = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,&Q, LOC_ECF2N_0043);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,&T, LOC_ECF2N_0044);
#endif/* STATIC_MEMORY_SWITCH */

    exp_basepoint_tbl = g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ];

    ec_f2n_point_aff_set_infinit( ecf2n_md_id, Q );
    for ( index = 0; index < len; index ++ )
    {
        if ( 1 == s[ index ] )
        {
            ec_f2n_point_mix_add( ecf2n_md_id, Q, exp_basepoint_tbl[ index ], T );
            tmp = Q;
            Q = T;
            T = tmp;
        }
        else if ( -1 == s[ index ] )
        {
            ec_f2n_point_mix_sub( ecf2n_md_id, Q, exp_basepoint_tbl[ index ], T );
            tmp = Q;
            Q = T;
            T = tmp;
        }
   }

    ec_f2n_point_aff_convert( ecf2n_md_id, Q, R );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,Q, LOC_ECF2N_0045);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT,T, LOC_ECF2N_0046);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, if the point P is basepoint, then use the basepoint-fixed algorithm,
*   otherwise, use the NAF algorithm
*
**/
void ec_f2n_point_mul(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )
    EC_CURVE_POINT **exp_basepoint_tbl;
#endif/* ECC_POINT_MUL_FIXED_BASE_SWITCH */

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul: k is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_mul: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if 0
    /* k_r = k mod N belong to {0,1,2,....,N - 1} */
    if ( bgn_z_cmp(bgnf2n_md_id, k, order ) >= 0 )
    {
        bgn_z_div(k, order, k_q, k_r);
    }
    else
    {
        bgn_z_clone(k, k_r);
    }
#endif

#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )
    exp_basepoint_tbl = g_ecf2n_exp_basepoint_tbl[ ecf2n_md_id ];

    if ( 0 == ec_f2n_point_cmp(ecf2n_md_id, P, exp_basepoint_tbl[ 0 ]  ) )
    {
        ec_f2n_point_mul_fix_base(ecf2n_md_id, k, R );

        return ;
    }

#endif/* ECC_POINT_MUL_FIXED_BASE_SWITCH */

    ec_f2n_point_mul_naf(ecf2n_md_id, P, k, R );

    return ;
}

/**
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = (x3,y3)
*   return R = P + Qover ec
*
**/
void ec_f2n_point_add(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_AFF_POINT *T;
    EC_CURVE_AFF_POINT *tmp;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_add: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_add: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_add: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_add: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/


#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( 0 == ec_f2n_point_cmp( ecf2n_md_id, P, Q ))
    {
        ec_f2n_point_double( ecf2n_md_id, P, R );
        return ;
    }
#endif /* ECC_DEBUG_SWITCH */

    if ( EC_TRUE == ec_f2n_point_is_infinit( ecf2n_md_id, P ) )
    {
        ec_f2n_point_clone( ecf2n_md_id, Q, R);

        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    T = &buf_1;
    tmp = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &T, LOC_ECF2N_0047);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &tmp, LOC_ECF2N_0048);
#endif/* STATIC_MEMORY_SWITCH */

    /* P(x,y) ---> T[x,y,z] */
    ec_f2n_point_convert(ecf2n_md_id, P, T);

    /* tmp[x,y,z] =  T[x,y,z] + Q(x:y) */
    ec_f2n_point_mix_add(ecf2n_md_id, T, Q, tmp);

    /* tmp[x,y,z] ---> R(x,y) */
    ec_f2n_point_aff_convert(ecf2n_md_id, tmp, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, T, LOC_ECF2N_0049);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, tmp, LOC_ECF2N_0050);
#endif/* STATIC_MEMORY_SWITCH */
    return;
}

/**
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R = 2 * P over ec
*
**/
void ec_f2n_point_double(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_AFF_POINT *T;
    EC_CURVE_AFF_POINT *tmp;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_double: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_double: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_double: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_f2n_point_is_infinit( ecf2n_md_id, P ) )
    {
        ec_f2n_point_set_infinit( ecf2n_md_id, R );

        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    T = &buf_1;
    tmp = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &T, LOC_ECF2N_0051);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &tmp, LOC_ECF2N_0052);
#endif/* STATIC_MEMORY_SWITCH */

    /*P(x1,y1) --> T[x2:y2:z2]*/
    ec_f2n_point_convert(ecf2n_md_id, P, T);

    /* tmp[x,y,z] =  2 * T[x,y,z] */
    ec_f2n_point_aff_double(ecf2n_md_id, T, tmp);

    /* tmp[x,y,z] ---> R(x,y) */
    ec_f2n_point_aff_convert(ecf2n_md_id, tmp, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, T, LOC_ECF2N_0053);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, tmp, LOC_ECF2N_0054);
#endif/* STATIC_MEMORY_SWITCH */

    return;
}

/**
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y2) be a point over ec
*   let R = (x3,y3)
*   return R = P - Q over ec
*
**/
void ec_f2n_point_sub(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_POINT *negQ;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_sub: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_sub: Q is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_sub: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECF2N_MD <= ecf2n_md_id || 0 == g_ecf2n_md[ ecf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_f2n_point_sub: ecf2n module #0x%lx not started.\n",
                ecf2n_md_id);
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    negQ = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, &negQ, LOC_ECF2N_0055);
#endif/* STATIC_MEMORY_SWITCH */

    /* negQ = - Q */
    ec_f2n_point_neg(ecf2n_md_id, Q, negQ);

    /* R = P + negQ = P - Q */
    ec_f2n_point_add(ecf2n_md_id, P, negQ, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, negQ, LOC_ECF2N_0056);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

#if ( SWITCH_ON == ECC_PM_CTRL_SWITCH )
void ec_f2n_point_mul_M_ary_Pre(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT *buf,UINT32 bufsize)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *tmp;
    UINT32 index;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_M_ary_Pre: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == buf )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_M_ary_Pre: buf is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, &tmp, LOC_ECF2N_0057);
#endif/* STATIC_MEMORY_SWITCH */
    ec_f2n_point_clone(ecf2n_md_id, P, &( buf[ 0 ] ) );

    ec_f2n_point_double(ecf2n_md_id, P, tmp);

    for ( index = 1; index < bufsize; index ++  )
    {
        ec_f2n_point_add(ecf2n_md_id,  &( buf[ index - 1] ), tmp, &( buf[ index ] ));
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT, tmp, LOC_ECF2N_0058);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

void ec_f2n_point_aff_doubles(const UINT32 ecf2n_md_id, const EC_CURVE_AFF_POINT * P,const UINT32 doubles,EC_CURVE_AFF_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_AFF_POINT *Q;
    EC_CURVE_AFF_POINT *T;
    EC_CURVE_AFF_POINT *tmp;
    UINT32 index;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_doubles: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_aff_doubles: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    T = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &Q, LOC_ECF2N_0059);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &T, LOC_ECF2N_0060);
#endif/* STATIC_MEMORY_SWITCH */
    if ( EC_TRUE == ec_f2n_point_aff_is_infinit( ecf2n_md_id, P ) )
    {
        ec_f2n_point_aff_set_infinit( ecf2n_md_id, R );
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, Q, LOC_ECF2N_0061);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, T, LOC_ECF2N_0062);
#endif/* STATIC_MEMORY_SWITCH */
        return ;
    }

    ec_f2n_point_aff_clone( ecf2n_md_id, P, Q );
    for ( index = 0; index < doubles; index ++ )
    {
        ec_f2n_point_aff_double( ecf2n_md_id, Q, T );
        tmp = Q;
        Q = T;
        T = tmp;
    }

    ec_f2n_point_aff_clone( ecf2n_md_id, Q, R );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, Q, LOC_ECF2N_0063);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, T, LOC_ECF2N_0064);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}
void ec_f2n_point_mul_M_ary(const UINT32 ecf2n_md_id, const EC_CURVE_POINT * P,const BIGINT * k,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_AFF_POINT *Q;
    EC_CURVE_AFF_POINT *tmp;
    EC_CURVE_POINT *oddP;
    UINT32 oddP_num;
    UINT32 oddP_index;
    UINT32 k_m_ary_len;
    UINT32 k_words;
    UINT32 word_index;
    UINT32 word;
    UINT32 e;
    UINT32 flag;
    UINT32 index;
    UINT32 t;
    UINT32 even_s;
    UINT32 odd_h;

    ECF2N_MD *ecf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_M_ary: P is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_M_ary: k  is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_f2n_point_mul_M_ary: R is null.\n");
        dbg_exit(MD_ECF2N, ecf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    bgnf2n_md_id = ecf2n_md->bgnf2n_md_id;


    /* if k = 0, then set R = 0 */
    if ( EC_TRUE == bgn_f2n_is_zero( bgnf2n_md_id, k ) )
    {
        point_set_infinit( R );
        return ;
    }
    /* now k is nonzero */
    k_words = k->len;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    tmp = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &Q, LOC_ECF2N_0065);
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, &tmp, LOC_ECF2N_0066);
#endif/* STATIC_MEMORY_SWITCH */

    /* if P = basepoint, then use the stored table */
    if ( 0 == ec_f2n_point_cmp( ecf2n_md_id, P, &odd_basepoint_tbl[ 0 ] ) )
    {
        oddP = ( EC_CURVE_POINT * ) odd_basepoint_tbl;
    }
    else
    {
        oddP = g_buf_for_point_mul_M_ary;
        oddP_num = ( ECC_M_ARY_M >> 1 );

        ec_f2n_point_mul_M_ary_Pre( ecf2n_md_id, P, oddP, oddP_num );
    }

    k_m_ary_len = k_words * ( WORDSIZE / ECC_M_ARY_R );
    e = (~((~((UINT32)0)) >> ECC_M_ARY_R ));
    flag = e;

    word_index = k_words - 1;
    word = k->data[ word_index ];

    for ( index = 0; index < WORDSIZE / ECC_M_ARY_R; index ++ )
    {
        if ( word & e )
        {
            break;
        }

        k_m_ary_len -- ;
        word <<= ECC_M_ARY_R;
        flag >>= ECC_M_ARY_R;
    }

    /** now :
     *
     *      k = SUM( k_j * m^j, j = 0,1,... k_m_ary_len - 1 )
     *  where m = 2 ^ ECC_M_ARY_R (= 2^4)
     **/
    ec_f2n_point_aff_set_infinit( ecf2n_md_id, Q );

    for ( index = k_m_ary_len; index > 0; index -- )
    {

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
        if ( word_index >  INTMAX)
        {
            sys_log(LOGSTDOUT,
                "error:point_mul_M_ary: index = %ld of k overflow\n",
                word_index);
            dbg_exit(MD_ECF2N, ecf2n_md_id);
        }
#endif /* ECC_DEBUG_SWITCH */

        t = ( ( word & e ) >> ( WORDSIZE - ECC_M_ARY_R ));

        if ( 0 < t )
        {
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
            if ( t >= sizeof (ecc_M_ary_factor)/sizeof(ecc_M_ary_factor[0]) )
            {
                sys_log(LOGSTDOUT,
                    "error:point_mul_M_ary: ecc_M_ary_factor overflow(t=ld).\n",
                    t);
                dbg_exit(MD_ECF2N, ecf2n_md_id);
            }
#endif /* ECC_DEBUG_SWITCH */

            even_s = ecc_M_ary_factor[ t ] [ ECC_EVEN_IDX ];
            odd_h = ecc_M_ary_factor[ t ] [ ECC_ODD_IDX ];

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
            if ( t != ( ( 1 << even_s ) * odd_h ) )
            {
                sys_log(LOGSTDOUT,"point_mul_M_ary: t=%ld,even_s=%ld,odd_h=%ld\n",
                               t, even_s, odd_h );
                dbg_exit(MD_ECF2N, ecf2n_md_id);
            }

            if ( (odd_h >> 1) >= (ECC_M_ARY_M >> 1) || 0 == odd_h )
            {
                sys_log(LOGSTDOUT,
                    "error:point_mul_M_ary: g_buf_for_point_mul_M_ary overflow");
                sys_log(LOGSTDOUT,"(odd_h=ld).\n",odd_h);
                dbg_exit(MD_ECF2N, ecf2n_md_id);
            }
#endif /* ECC_DEBUG_SWITCH */

            oddP_index = ( odd_h >> 1 );
            ec_f2n_point_aff_doubles( ecf2n_md_id, Q, ECC_M_ARY_R - even_s, tmp );
            ec_f2n_point_mix_add( ecf2n_md_id, tmp, &(oddP[ oddP_index ]), Q );

        }
        else
        {
            even_s = ECC_M_ARY_R;
        }

        ec_f2n_point_aff_doubles( ecf2n_md_id, Q, even_s, tmp );
        ec_f2n_point_aff_clone( ecf2n_md_id, tmp, Q );

        word <<= ECC_M_ARY_R;
        flag >>= ECC_M_ARY_R;
        if ( 0 == flag && 0 < word_index )
        {
            flag = (~((~((UINT32)0)) >> ECC_M_ARY_R ));
            word_index -- ;
            word = k->data[ word_index ];
        }
    }

    ec_f2n_point_aff_convert( ecf2n_md_id, Q, R );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, Q, LOC_ECF2N_0067);
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_AFFINE_POINT, tmp, LOC_ECF2N_0068);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}


static void ec_f2n_point_mul_M_ary_factor(const UINT32 ecf2n_md_id)
{
    UINT32 k;
    UINT32 num;
    UINT32 even_cnt;
    UINT32 odd;

    for ( k = 1; k < ECC_M_ARY_M; k ++ )
    {
        num = k;
        even_cnt = 0;
        while ( num > 0 && 0 == ( num & 1 ) )
        {
            even_cnt ++;
            num >>= 1;
        }

        odd = num;

        sys_log(LOGSTDOUT,"/* %ld */{ %ld, %ld },\n",k, even_cnt, odd );
    }
}
static void ec_f2n_point_mul_M_ary_Pre_fix_base(const UINT32 ecf2n_md_id)
{
    EC_CURVE_POINT *P;
    EC_CURVE_POINT *oddP;
    EC_CURVE_POINT *Q;

    UINT32 oddP_num;
    UINT32 point_idx;
    UINT32 k;

    ECF2N_MD *ecf2n_md;
    EC_CURVE_POINT *base_point;

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    base_point = ecf2n_md->ecf2n_base_point;

    P = base_point;
    oddP = g_buf_for_point_mul_M_ary;
    oddP_num = ( ECC_M_ARY_M >> 1 );

    sys_log(LOGSTDOUT,"basepoint P:\n");
    print_point( LOGSTDOUT, P );

    ec_f2n_point_mul_M_ary_Pre( ecf2n_md_id, P, oddP, oddP_num );

    for ( point_idx = 0; point_idx < oddP_num; point_idx ++ )
    {
        Q = &( oddP[ point_idx ] );
        k = 2 * point_idx + 1;
        sys_log(LOGSTDOUT,"/* k = %ld, kP: */\n", k );

        print_point_format( LOGSTDOUT, Q );
    }
}

static void ec_f2n_point_mul_pre_fix_base(const UINT32 ecf2n_md_id)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *P;
    EC_CURVE_POINT *Q;
    UINT32 exp;

    ECF2N_MD *ecf2n_md;
    EC_CURVE_POINT *base_point;

    ecf2n_md = &(g_ecf2n_md[ ecf2n_md_id ]);
    base_point = ecf2n_md->ecf2n_base_point;

    P = base_point;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT ,&Q, LOC_ECF2N_0069);
#endif/* STATIC_MEMORY_SWITCH */
    sys_log(LOGSTDOUT,"/********************************************************************\\\n");
    sys_log(LOGSTDOUT,"basepoint P:\n");
    print_point( LOGSTDOUT, P );
    sys_log(LOGSTDOUT,"\\********************************************************************/\n");

    ec_f2n_point_clone( ecf2n_md_id, P, Q );

    for ( exp = 0; exp <= CURVE_DEG; exp ++ )
    {
        sys_log(LOGSTDOUT,"/* k = 2 ^ %ld, kP: */\n", exp );
        print_point_format(LOGSTDOUT, Q );

        ec_f2n_point_double(ecf2n_md_id, Q, Q);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECF2N, ecf2n_md_id, MM_CURVE_POINT ,Q, LOC_ECF2N_0070);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

#endif /* ECC_PM_CTRL_SWITCH */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

