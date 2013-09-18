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
#include "bgnzn.h"
#include "bgnfp.h"

#include "ecfp.h"

#include "debug.h"

#include "print.h"

#include "mm.h"



static ECFP_MD g_ecfp_md[ MAX_NUM_OF_ECFP_MD ];
static EC_BOOL  g_ecfp_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_ecfp_p_buf   [ MAX_NUM_OF_ECFP_MD ];
static BIGINT g_ecfp_order_buf[ MAX_NUM_OF_ECFP_MD ];
static ECFP_CURVE  g_ecfp_curve_buf[ MAX_NUM_OF_ECFP_MD ];
static EC_CURVE_POINT g_ecfp_curve_point_buf[ MAX_NUM_OF_ECFP_MD ];

static EC_CURVE_POINT g_ecfp_exp_basepoint_tbl_buf[ MAX_NUM_OF_ECFP_MD ][MAX_NAF_ARRAY_LEN];
static int g_ecfp_naf_buf[ MAX_NUM_OF_ECFP_MD ][ BIGINTSIZE ];
#endif/* STACK_MEMORY_SWITCH */

static EC_CURVE_POINT *g_ecfp_exp_basepoint_tbl[ MAX_NUM_OF_ECFP_MD ][MAX_NAF_ARRAY_LEN];
static int  *g_ecfp_naf_arry[ MAX_NUM_OF_ECFP_MD ];

/**
*   for test only
*
*   to query the status of ECFP Module
*
**/
void ec_fp_print_module_status(const UINT32 ecfp_md_id, LOG *log)
{
    ECFP_MD *ecfp_md;
    UINT32 index;

    if ( EC_FALSE == g_ecfp_md_init_flag )
    {

        sys_log(log,"no ECFP Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_ECFP_MD; index ++ )
    {
        ecfp_md = &(g_ecfp_md[ index ]);

        if ( 0 < ecfp_md->usedcounter )
        {
            sys_log(log,"ECFP Module  # %ld : %ld refered, refer BGNFP Module : %ld\n",
                    index,
                    ecfp_md->usedcounter,
                    ecfp_md->bgnfp_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed ECFP module
*
*
**/
UINT32 ec_fp_free_module_static_mem(const UINT32 ecfp_md_id)
{
    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_free_module_static_mem : ecfp module  #0x%lx not started.\n",
                ecfp_md_id);

        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    free_module_static_mem(MD_ECFP, ecfp_md_id);

    bgn_fp_free_module_static_mem(bgnfp_md_id);

    return 0;
}

/**
*
* start ECFP module
* note:
*    order or base_point is not necessary
*
**/
UINT32 ec_fp_start( const BIGINT *p, const ECFP_CURVE *curve, const BIGINT *order, const EC_CURVE_POINT *base_point )
{
    UINT32     ecfp_md_id;
    UINT32    bgnfp_md_id;
    BIGINT        *ecfp_p;
    ECFP_CURVE    *ecfp_curve;
    BIGINT         *ecfp_order;
    EC_CURVE_POINT *ecfp_base_point;

    ECFP_MD *ecfp_md;
    EC_CURVE_POINT **exp_basepoint_tbl;
    UINT32 array_len;

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == p )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_start: p is NULL_PTR.\n");
        dbg_exit(MD_ECFP, ERR_MODULE_ID);
    }
    if ( NULL_PTR == curve )
    {
         sys_log(LOGSTDOUT,"error:ec_fp_start: curve is NULL_PTR.\n");
         dbg_exit(MD_ECFP, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* initialize g_ecfp_md */
    if ( EC_FALSE ==  g_ecfp_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_ECFP_MD; index ++ )
        {
            ecfp_md = &(g_ecfp_md[ index ]);

            ecfp_md->usedcounter       = 0;
            ecfp_md->bgnfp_md_id       = ERR_MODULE_ID;
            ecfp_md->ecfp_p            = NULL_PTR;
            ecfp_md->ecfp_curve        = NULL_PTR;
            ecfp_md->ecfp_order        = NULL_PTR;
            ecfp_md->ecfp_base_point   = NULL_PTR;
        }

        /*register all functions of ECFP module to DBG module*/
        //dbg_register_func_addr_list(g_ecfp_func_addr_list, g_ecfp_func_addr_list_len);

        g_ecfp_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_ECFP_MD; index ++ )
    {
        ecfp_md = &(g_ecfp_md[ index ]);

        if ( 0 == ecfp_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( MAX_NUM_OF_ECFP_MD <= index )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    ecfp_p            = NULL_PTR;
    ecfp_curve        = NULL_PTR;
    ecfp_order        = NULL_PTR;
    ecfp_base_point   = NULL_PTR;

    ecfp_md_id = index;
    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT ,        &ecfp_p           , LOC_ECFP_0001);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_ECFP_CURVE ,    &ecfp_curve       , LOC_ECFP_0002);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ecfp_p            = &(g_ecfp_p_buf[ ecfp_md_id ]);
    ecfp_curve        = &(g_ecfp_curve_buf[ ecfp_md_id ]);
#endif/* STACK_MEMORY_SWITCH */

    bgnfp_md_id = bgn_fp_start( p );

    /* set ecfp_f = p */
    bgn_fp_clone(bgnfp_md_id, p, ecfp_p);

    /* set ecfp_curve = curve */
    bgn_fp_clone(bgnfp_md_id, &(curve->a), &(ecfp_curve->a));
    bgn_fp_clone(bgnfp_md_id, &(curve->b), &(ecfp_curve->b));

    /*if order is inputed, then stored it to ECFP module*/
    if ( NULL_PTR != order )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,         &ecfp_order       , LOC_ECFP_0003);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
        ecfp_order        = &(g_ecfp_order_buf[ ecfp_md_id ]);
#endif/* STACK_MEMORY_SWITCH */

        /* set ecfp_order = order */
        bgn_fp_clone(bgnfp_md_id, order, ecfp_order);
    }

    /*if base_point is inputed, then stored it to ECFP module*/
    if ( NULL_PTR != base_point )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,    &ecfp_base_point  , LOC_ECFP_0004);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
        ecfp_base_point   = &(g_ecfp_curve_point_buf[ ecfp_md_id ]);
#endif/* STACK_MEMORY_SWITCH */

        /* set ecfp_base_point = base_point */
        bgn_fp_clone(bgnfp_md_id, &(base_point->x), &(ecfp_base_point->x));
        bgn_fp_clone(bgnfp_md_id, &(base_point->y), &(ecfp_base_point->y));
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_NAF, &(g_ecfp_naf_arry[ ecfp_md_id ]), LOC_ECFP_0005);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    g_ecfp_naf_arry[ ecfp_md_id ] = g_ecfp_naf_buf[ ecfp_md_id ];
#endif/* STACK_MEMORY_SWITCH */

    /* set module : */
    ecfp_md->usedcounter       = 0;
    ecfp_md->bgnfp_md_id       = bgnfp_md_id;
    ecfp_md->ecfp_p            = ecfp_p;
    ecfp_md->ecfp_curve        = ecfp_curve;
    ecfp_md->ecfp_order        = ecfp_order;
    ecfp_md->ecfp_base_point   = ecfp_base_point;

    /* at the first time, set the counter to 1 */
    ecfp_md->usedcounter = 1;

    /*if no base_point is inputed, then return here.*/
    if ( NULL_PTR == ecfp_base_point )
    {
        return ( ecfp_md_id );
    }

    /* now, ECFP is already started */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    exp_basepoint_tbl = (g_ecfp_exp_basepoint_tbl[ ecfp_md_id ]);
    array_len = sizeof (g_ecfp_exp_basepoint_tbl[ ecfp_md_id ]) / sizeof ((g_ecfp_exp_basepoint_tbl[ ecfp_md_id ][0]));
    for ( index = 0; index < array_len; index ++ )
    {
        alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT, &(exp_basepoint_tbl[ index ]), LOC_ECFP_0006);
    }
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    exp_basepoint_tbl = (g_ecfp_exp_basepoint_tbl[ ecfp_md_id ]);
    array_len = sizeof (g_ecfp_exp_basepoint_tbl[ ecfp_md_id ]) / sizeof ((g_ecfp_exp_basepoint_tbl[ ecfp_md_id ][0]));
    for ( index = 0; index < array_len; index ++ )
    {
        exp_basepoint_tbl[ index ] = &(g_ecfp_exp_basepoint_tbl_buf[ ecfp_md_id ][ index ]);
    }
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == g_ecfp_naf_arry[ ecfp_md_id ] )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_start: g_ecfp_naf_arry[ %ld ] is NULL_PTR.\n",ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )
    if ( NULL_PTR != ecfp_base_point )
    {
        /* setup the g_ecfp_exp_basepoint_tbl */
        exp_basepoint_tbl = g_ecfp_exp_basepoint_tbl[ ecfp_md_id ];
        array_len = sizeof (g_ecfp_exp_basepoint_tbl[ ecfp_md_id ]) / sizeof ((g_ecfp_exp_basepoint_tbl[ ecfp_md_id ][0]));

        ec_fp_point_clone(ecfp_md_id, ecfp_base_point, exp_basepoint_tbl[ 0 ]);

        /* do not shorten the loop length, since for ec_fp_point_mul interface,*/
        /* when compute k * P, the k may be greater than the ec order */
        for ( index = 1; index < array_len; index ++ )
        {
            ec_fp_point_double( ecfp_md_id,exp_basepoint_tbl[ index - 1 ],exp_basepoint_tbl[ index ]);
        }
    }
#endif/*ECC_POINT_MUL_FIXED_BASE_SWITCH*/

    return ( ecfp_md_id );
}

/**
*
* end ECFP module
*
**/
void ec_fp_end(const UINT32 ecfp_md_id)
{
    BIGINT         *ecfp_p;
    ECFP_CURVE     *ecfp_curve;
    BIGINT         *ecfp_order;
    EC_CURVE_POINT *ecfp_base_point;

    ECFP_MD        *ecfp_md;
    UINT32     bgnfp_md_id;
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    EC_CURVE_POINT **exp_basepoint_tbl;
    UINT32 array_len;
    UINT32  index;
#endif/* STATIC_MEMORY_SWITCH */

    if ( MAX_NUM_OF_ECFP_MD < ecfp_md_id )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_end: ecfp_md_id = %ld is overflow.\n",ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ecfp_md->usedcounter )
    {
        ecfp_md->usedcounter --;

        return ;
    }

    if ( 0 == ecfp_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_end: ecfp_md_id = %ld is not started.\n",ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnfp_md_id       = ecfp_md->bgnfp_md_id;
    ecfp_p            = ecfp_md->ecfp_p;
    ecfp_curve        = ecfp_md->ecfp_curve;
    ecfp_order        = ecfp_md->ecfp_order;
    ecfp_base_point   = ecfp_md->ecfp_base_point;

    bgn_fp_end( bgnfp_md_id );

    /*if ecfp_order is not null, then free it if possible*/
    if ( NULL_PTR != ecfp_order )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,          ecfp_order        , LOC_ECFP_0007);
#endif/* STATIC_MEMORY_SWITCH */
    }

    /*if ecfp_base_point is not null, then free it if possible */
    /*and free all occupied static memory alloced when module started*/
    if ( NULL_PTR != ecfp_base_point )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        exp_basepoint_tbl = g_ecfp_exp_basepoint_tbl[ ecfp_md_id ];
        array_len = sizeof(g_ecfp_exp_basepoint_tbl[ ecfp_md_id ])/sizeof(g_ecfp_exp_basepoint_tbl[ ecfp_md_id ][0]);
        for ( index = 0; index < array_len; index ++ )
        {
            free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT, exp_basepoint_tbl[ index ], LOC_ECFP_0008);
        }
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,     ecfp_base_point   , LOC_ECFP_0009);
#endif/* STATIC_MEMORY_SWITCH */
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_NAF,            (g_ecfp_naf_arry[ ecfp_md_id ]), LOC_ECFP_0010);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,          ecfp_p            , LOC_ECFP_0011);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_ECFP_CURVE ,     ecfp_curve        , LOC_ECFP_0012);
#endif/* STATIC_MEMORY_SWITCH */

    /* free module : */
    //ec_fp_free_module_static_mem(ecfp_md_id);
    ecfp_md->bgnfp_md_id       = ERR_MODULE_ID;
    ecfp_md->ecfp_p            = NULL_PTR;
    ecfp_md->ecfp_curve        = NULL_PTR;
    ecfp_md->ecfp_order        = NULL_PTR;
    ecfp_md->ecfp_base_point   = NULL_PTR;
    ecfp_md->usedcounter         = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   if point curve_1  = curve_2, then return 0
*   if point curve_1 != curve_2, then return 1
*
**/
UINT32 ec_fp_curve_cmp(const UINT32 ecfp_md_id, const ECFP_CURVE * curve_1,const ECFP_CURVE * curve_2)
{
    BIGINT *a1;
    BIGINT *b1;
    BIGINT *a2;
    BIGINT *b2;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == curve_1 )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_curve_cmp: curve_1 is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == curve_2 )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_curve_cmp: curve_2 is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_curve_cmp: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    a1 = ( BIGINT * )&(curve_1->a);
    b1 = ( BIGINT * )&(curve_1->b);

    a2 = ( BIGINT * )&(curve_2->a);
    b2 = ( BIGINT * )&(curve_2->b);

    if ( 0 != bgn_fp_cmp(bgnfp_md_id, a1, a2 ) )
    {
        return 1;
    }

    if ( 0 != bgn_fp_cmp(bgnfp_md_id, b1, b2 ) )
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
UINT32 ec_fp_point_cmp(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,const EC_CURVE_POINT * Q)
{
    BIGINT *x1;
    BIGINT *y1;
    BIGINT *x2;
    BIGINT *y2;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_cmp: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_cmp: Q is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_cmp: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = ( BIGINT * )&(P->x);
    y1 = ( BIGINT * )&(P->y);

    x2 = ( BIGINT * )&(Q->x);
    y2 = ( BIGINT * )&(Q->y);

    if ( 0 != bgn_fp_cmp(bgnfp_md_id, x1, x2 ) )
    {
        return 1;
    }

    if ( 0 != bgn_fp_cmp(bgnfp_md_id, y1, y2 ) )
    {
        return 1;
    }

    return 0;/* equal */
}

/**
*
*   copy point src to des
*   return des
*
**/
void ec_fp_point_clone(const UINT32 ecfp_md_id, const EC_CURVE_POINT * src,EC_CURVE_POINT * des)
{
    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_clone: src is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == des)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_clone: des is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_clone: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    bgn_fp_clone(bgnfp_md_id, &(src->x), &(des->x));
    bgn_fp_clone(bgnfp_md_id, &(src->y), &(des->y));

    return ;
}

/**
*
*   copy affine point src to des
*   return des
*
**/
void ec_fp_point_aff_clone(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * src,EC_CURVE_AFF_POINT * des)
{
    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_clone: src is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == des)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_clone: des is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_clone: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    bgn_fp_clone(bgnfp_md_id, &(src->x), &(des->x));
    bgn_fp_clone(bgnfp_md_id, &(src->y), &(des->y));
    bgn_fp_clone(bgnfp_md_id, &(src->z), &(des->z));

    return ;
}

/**
*
*   if the point P = (0,0),i.e, it's an infinite point
*   then return EC_TRUE;
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_point_is_infinit(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P)
{
    const BIGINT *x;
    const BIGINT *y;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_is_infinit: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_is_infinit: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x = &(P->x);
    y = &(P->y);

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, x)
      && EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, y))
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   let P = [x:y:z]
*   if z = 0, then it means the P is an infinite affine point
*   then return EC_TRUE;
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_point_aff_is_infinit(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P)
{
    const BIGINT *z;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_is_infinit: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_is_infinit: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    z = &(P->z);

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, z))
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
void ec_fp_point_set_infinit(const UINT32 ecfp_md_id, EC_CURVE_POINT * P)
{
    BIGINT *x;
    BIGINT *y;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_set_infinit: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_set_infinit: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    x = &(P->x);
    y = &(P->y);

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    bgn_fp_set_zero( bgnfp_md_id, x );
    bgn_fp_set_zero( bgnfp_md_id, y );

    return ;
}

/**
*
*   set affine point P = [1:1:0] as the infinit affine point
*   return the affine point P
*
**/
void ec_fp_point_aff_set_infinit(const UINT32 ecfp_md_id, EC_CURVE_AFF_POINT * P)
{
    BIGINT *x;
    BIGINT *y;
    BIGINT *z;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_set_infinit: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_set_infinit: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    x = &(P->x);
    y = &(P->y);
    z = &(P->z);

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    bgn_fp_set_one( bgnfp_md_id, x );
    bgn_fp_set_one( bgnfp_md_id, y );
    bgn_fp_set_zero( bgnfp_md_id, z );

    return ;
}

/**
*
*   let P = (x1:y1) be a point over ec
*   let R = (x3:y3)
*   return R =  -P over ec
*
*   note:
*       x3 =  x1 over F_p
*       y3 = -y1 over F_p
*
**/
void ec_fp_point_neg(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,EC_CURVE_POINT *R)
{
    const BIGINT *x1;
    const BIGINT *y1;
    BIGINT *x3;
    BIGINT *y3;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_neg: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_neg: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_neg: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);

    x3 = &(R->x);
    y3 = &(R->y);

    bgn_fp_clone(bgnfp_md_id, x1, x3);
    bgn_fp_neg(bgnfp_md_id,y1, y3);

    return ;
}

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let R = [x3:y3:z3]
*   return R =  -P over ec
*
*   note:
*       x3 =  x1 over F_p
*       y3 = -y1 over F_p
*       z3 =  z1 over F_p
*
**/
void ec_fp_point_aff_neg(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,EC_CURVE_AFF_POINT *R)
{
    const BIGINT *x1;
    const BIGINT *y1;
    const BIGINT *z1;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_neg: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_neg: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_neg: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);
    z1 = &(P->z);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    bgn_fp_clone(bgnfp_md_id, x1, x3);
    bgn_fp_neg  (bgnfp_md_id, y1, y3);
    bgn_fp_clone(bgnfp_md_id, z1, z3);

    return ;
}

/**
*   let P = (x1,y1) be a point over ec
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
void ec_fp_point_convert(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,EC_CURVE_AFF_POINT * R)
{
    const BIGINT *x1;
    const BIGINT *y1;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_convert: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_convert: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_convert: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, P) )
    {
        ec_fp_point_aff_set_infinit(ecfp_md_id, R);
        return;
    }

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    bgn_fp_clone(bgnfp_md_id, x1, x3);
    bgn_fp_clone(bgnfp_md_id, y1, y3);
    bgn_fp_set_one(bgnfp_md_id, z3);

    return ;
}

/**
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = (x3,y3)
*   return R = P over ec
*   where
*       x3 = x1 / z1 ^2
*       y3 = y1 / z1 ^ 3
*
*   note :
*       P address is not equal to R address
**/
void ec_fp_point_aff_convert(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    const BIGINT *x1;
    const BIGINT *y1;
    const BIGINT *z1;
    BIGINT *x3;
    BIGINT *y3;

    BIGINT *t1;
    BIGINT *t2;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_convert: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_convert: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_convert: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_fp_point_aff_is_infinit(ecfp_md_id, P) )
    {
        ec_fp_point_set_infinit(ecfp_md_id, R);
        return;
    }
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &buf_1;
    t2 = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t1, LOC_ECFP_0013);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t2, LOC_ECFP_0014);
#endif/* STATIC_MEMORY_SWITCH */

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);
    z1 = &(P->z);

    x3 = &(R->x);
    y3 = &(R->y);

    /* t1 = z1 ^ 2 */
    bgn_fp_squ(bgnfp_md_id, z1, t1);

    /* t2 = 1/ t1 */
    bgn_fp_inv(bgnfp_md_id, t1, t2);

    /* x3 = x1 / z1 ^2  = x1 * t2 */
    bgn_fp_mul(bgnfp_md_id, x1, t2, x3);

    /* t2 = z1 ^ 3 = z1 * t1 */
    bgn_fp_mul(bgnfp_md_id, z1, t1, t2);

    /* t1 = 1/ t2 = 1 / z1^3 */
    bgn_fp_inv(bgnfp_md_id, t2, t1);

    /* y3 = y1 / z1 ^ 3 = y1 * t1*/
    bgn_fp_mul(bgnfp_md_id, y1, t1, y3);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t1, LOC_ECFP_0015);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t2, LOC_ECFP_0016);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   check point is on the elliptic curve(ec) refered by Module ID ecfp_md_id
*   if the point is on the elliptic curve(ec), return EC_TRUE,
*   otherwise, return EC_FALSE
*
*   note:
*       the ec is defined by
*           y^2 = x^3 + ax + b over F_p
*
**/
EC_BOOL ec_fp_point_is_on_curve(const UINT32 ecfp_md_id, const EC_CURVE_POINT * point)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *t1;
    BIGINT *t2;
    BIGINT *left;
    BIGINT *right;

    const BIGINT *x;
    const BIGINT *y;
    BIGINT *a;
    BIGINT *b;

    EC_BOOL ret;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;
    ECFP_CURVE *ecfp_curve;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == point )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_is_on_curve: point is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_is_on_curve: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &buf_1;
    t2 = &buf_2;
    left = &buf_3;
    right = &buf_4;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t1, LOC_ECFP_0017);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t2, LOC_ECFP_0018);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &left, LOC_ECFP_0019);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &right, LOC_ECFP_0020);
#endif/* STATIC_MEMORY_SWITCH */

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;
    ecfp_curve  = ecfp_md->ecfp_curve;

    x = &(point->x);
    y = &(point->y);

    a = &(ecfp_curve->a);
    b = &(ecfp_curve->b);

    /* left = y^2 over F_p*/
    bgn_fp_squ(bgnfp_md_id, y, left);

    /* right = x^3 + ax + b over F_p */
    bgn_fp_squ(bgnfp_md_id, x, t1);         /* t1 = x^2 */
    bgn_fp_add(bgnfp_md_id, a, t1, t2);     /* t2 = a + t1 = a + x^2 */
    bgn_fp_mul(bgnfp_md_id, x, t2, t1);     /* t1 = x * t2 = x *(a + x^2) = x^3 + ax */
    bgn_fp_add(bgnfp_md_id, b, t1, right);  /* right = b + t1 = x^3 + ax + b */

    /* t1 = left - right over F_p */
    bgn_fp_sub(bgnfp_md_id, left, right, t1);

    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, t1) )
    {
        ret = EC_TRUE;
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t1, LOC_ECFP_0021);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t2, LOC_ECFP_0022);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, left, LOC_ECFP_0023);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, right, LOC_ECFP_0024);
#endif/* STATIC_MEMORY_SWITCH */

    return ( ret );
}

/**
*
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R = 2 * P over ec
*
**/
void ec_fp_point_double(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,EC_CURVE_POINT * R)
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
        sys_log(LOGSTDOUT,"error:ec_fp_point_double: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_double: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_double: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_fp_point_is_infinit( ecfp_md_id, P ) )
    {
        ec_fp_point_set_infinit( ecfp_md_id, R );
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    T = &buf_1;
    tmp = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT, &T, LOC_ECFP_0025);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT, &tmp, LOC_ECFP_0026);
#endif/* STATIC_MEMORY_SWITCH */

    /*P(x1,y1) --> T[x2:y2:z2]*/
    ec_fp_point_convert(ecfp_md_id, P, T);

    /* tmp[x,y,z] =  2 * T[x,y,z] */
    ec_fp_point_aff_double(ecfp_md_id, T, tmp);

    /* tmp[x,y,z] ---> R(x,y) */
    ec_fp_point_aff_convert(ecfp_md_id, tmp, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT, T, LOC_ECFP_0027);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT, tmp, LOC_ECFP_0028);
#endif/* STATIC_MEMORY_SWITCH */

    return;
}

/**
*
*   let P = [x1:y1:z1] be an affine point over ec
*   let R = [x3:y3:z3]
*   return R = 2 * P over ec
*
**/
void ec_fp_point_aff_double(const UINT32 ecfp_md_id, const EC_CURVE_AFF_POINT * P,EC_CURVE_AFF_POINT * R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *A;
    BIGINT *B;
    BIGINT *C;
    BIGINT *s;
    BIGINT *t;

    const BIGINT *x1;
    const BIGINT *y1;
    const BIGINT *z1;
    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_double: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_aff_double: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_aff_double: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if(EC_TRUE == ec_fp_point_aff_is_infinit( ecfp_md_id, P ))
    {
        ec_fp_point_aff_set_infinit(ecfp_md_id, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    A = &buf_1;
    B = &buf_2;
    C = &buf_3;
    s = &buf_4;
    t = &buf_5;
#endif/*STACK_MEMORY_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&A, LOC_ECFP_0029);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&B, LOC_ECFP_0030);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&C, LOC_ECFP_0031);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&s, LOC_ECFP_0032);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&t, LOC_ECFP_0033);
#endif/* STATIC_MEMORY_SWITCH */

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);
    z1 = &(P->z);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    /* A = 4 *x1 *y1^2 */
    bgn_fp_shl_lesswordsize(bgnfp_md_id,x1, 1, B      );
    bgn_fp_shl_lesswordsize(bgnfp_md_id,B,  1, C      );
    bgn_fp_squ(bgnfp_md_id,y1,B        );
    bgn_fp_mul(bgnfp_md_id,C   ,B,A     );

    /* B = 8 * y1^4 */
    bgn_fp_shl_lesswordsize(bgnfp_md_id,B   ,1, C        );
    bgn_fp_squ(bgnfp_md_id,C   ,s        );
    bgn_fp_shl_lesswordsize(bgnfp_md_id,s   ,1, B        );

    /* C = 3 * (x1 - z1^2) * (x1 + z1^2) */
    bgn_fp_squ(bgnfp_md_id,z1,s         );
    bgn_fp_sub(bgnfp_md_id,x1,s ,y3  );
    bgn_fp_add(bgnfp_md_id,x1,s ,z3  );
    bgn_fp_mul(bgnfp_md_id,y3,z3,x3);
    bgn_fp_shl_lesswordsize(bgnfp_md_id,x3,1, s       );
    bgn_fp_add(bgnfp_md_id,s   ,x3,C   );

    /* D = x3 =  C^2 - 2 * A */
    bgn_fp_squ(bgnfp_md_id,C   ,y3      );
    bgn_fp_shl_lesswordsize(bgnfp_md_id,A   ,1, z3     );
    bgn_fp_sub(bgnfp_md_id,y3,z3,x3);

    /* y3 = C *(A-D) - B */
    bgn_fp_sub(bgnfp_md_id,A   ,x3,z3);
    bgn_fp_mul(bgnfp_md_id,C   ,z3,s   );
    bgn_fp_sub(bgnfp_md_id,s   ,B ,y3  );

    /* z3 = 2 * y1 * z1 */
    bgn_fp_mul(bgnfp_md_id,y1,z1,s  );
    bgn_fp_shl_lesswordsize(bgnfp_md_id,s   ,1,z3 );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,A, LOC_ECFP_0034);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,B, LOC_ECFP_0035);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,C, LOC_ECFP_0036);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,s, LOC_ECFP_0037);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,t, LOC_ECFP_0038);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let Q = (x2,z2) be a point over ec
*   let R = [x3:y3:z3]
*   return R =  P + Q over ec
*
**/
void ec_fp_point_mix_add(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_AFF_POINT *R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
    BIGINT buf_6;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *A;
    BIGINT *B;
    BIGINT *C;
    BIGINT *D;
    BIGINT *r;
    BIGINT *s;

    const BIGINT *x1;
    const BIGINT *y1;
    const BIGINT *z1;

    const BIGINT *x2;
    const BIGINT *y2;


    BIGINT *x3;
    BIGINT *y3;
    BIGINT *z3;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_add: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_add: Q is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_add: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mix_add: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if( EC_TRUE == ec_fp_point_aff_is_infinit(ecfp_md_id, P) )
    {
        ec_fp_point_convert(ecfp_md_id, Q, R);

        return ;
    }
    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, Q) )
    {
        ec_fp_point_aff_clone(ecfp_md_id, P, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    A = &buf_1;
    B = &buf_2;
    C = &buf_3;
    D = &buf_4;
    r = &buf_5;
    s = &buf_6;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&A, LOC_ECFP_0039);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&B, LOC_ECFP_0040);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&C, LOC_ECFP_0041);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&D, LOC_ECFP_0042);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&r, LOC_ECFP_0043);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,&s, LOC_ECFP_0044);
#endif/* STATIC_MEMORY_SWITCH */

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    x1 = &(P->x);
    y1 = &(P->y);
    z1 = &(P->z);

    x2 = &(Q->x);
    y2 = &(Q->y);

    x3 = &(R->x);
    y3 = &(R->y);
    z3 = &(R->z);

    bgn_fp_squ(bgnfp_md_id, z1, C      );
    bgn_fp_mul(bgnfp_md_id, z1, C, D   );

    bgn_fp_mul(bgnfp_md_id, x2, C, A   );
    bgn_fp_mul(bgnfp_md_id, y2, D, B   );
    bgn_fp_sub(bgnfp_md_id, A , x1,C   );
    bgn_fp_sub(bgnfp_md_id, B , y1,D   );

    bgn_fp_squ(bgnfp_md_id, C , A      );
    bgn_fp_mul(bgnfp_md_id, C , A, B   );
    bgn_fp_mul(bgnfp_md_id, x1, A, y3);

    bgn_fp_shl_lesswordsize(bgnfp_md_id, y3, 1, r );
    bgn_fp_add(bgnfp_md_id, B,  r, s   );
    bgn_fp_squ(bgnfp_md_id, D,  z3     );
    bgn_fp_sub(bgnfp_md_id, z3, s, x3);

    bgn_fp_sub(bgnfp_md_id, y3,x3,A );
    bgn_fp_mul(bgnfp_md_id, A, D, z3);
    bgn_fp_mul(bgnfp_md_id, y1,B, A );
    bgn_fp_sub(bgnfp_md_id, z3,A, y3);

    bgn_fp_mul(bgnfp_md_id, z1,C, z3);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,A, LOC_ECFP_0045);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,B, LOC_ECFP_0046);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,C, LOC_ECFP_0047);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,D, LOC_ECFP_0048);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,r, LOC_ECFP_0049);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT,s, LOC_ECFP_0050);
#endif/* STATIC_MEMORY_SWITCH */
}

/**
*
*   let P = [x1:y1:z1] be a point over ec
*   let Q = (x2,z2) be a point over ec
*   let R = [x3:y3:z3]
*   return R =  P - Q over ec
*
**/
void ec_fp_point_mix_sub(const UINT32 ecfp_md_id,const EC_CURVE_AFF_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_AFF_POINT *R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_POINT *negQ;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_sub: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_sub: Q is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mix_sub: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mix_sub: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    negQ = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,&negQ, LOC_ECFP_0051);
#endif/* STATIC_MEMORY_SWITCH */

    ec_fp_point_neg(ecfp_md_id, Q, negQ );
    ec_fp_point_mix_add(ecfp_md_id, P, negQ, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,negQ, LOC_ECFP_0052);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  P + Q over ec
*
**/
void ec_fp_point_add(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_POINT *R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_AFF_POINT *T1;
    EC_CURVE_AFF_POINT *T2;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_add: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_add: Q is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_add: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_add: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, P) )
    {
        ec_fp_point_clone(ecfp_md_id, Q, R);
        return ;
    }

    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, Q) )
    {
        ec_fp_point_clone(ecfp_md_id, P, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    T1 = &buf_1;
    T2 = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&T1, LOC_ECFP_0053);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&T2, LOC_ECFP_0054);
#endif/* STATIC_MEMORY_SWITCH */

    /* P(x,y) --> T1[x:y:z]*/
    ec_fp_point_convert(ecfp_md_id, P, T1);

    /* T2[x:y:z] = T1[x:y:z] + Q(x,y) */
    ec_fp_point_mix_add(ecfp_md_id, T1, Q, T2);

    /* T2[x:y:z] --> R(x,y) */
    ec_fp_point_aff_convert(ecfp_md_id, T2, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,T1, LOC_ECFP_0055);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,T2, LOC_ECFP_0056);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let P = (x1,y1) be a point over ec
*   let Q = (x2,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  P - Q over ec
*
**/
void ec_fp_point_sub(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const EC_CURVE_POINT *Q,EC_CURVE_POINT *R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_POINT *negQ;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_sub: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == Q )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_sub: Q is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_sub: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_sub: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    negQ = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,&negQ, LOC_ECFP_0057);
#endif/* STATIC_MEMORY_SWITCH */

    /* negQ = -Q */
    ec_fp_point_neg(ecfp_md_id, Q, negQ);

    /* R = P + negQ = P - Q */
    ec_fp_point_add(ecfp_md_id, P, negQ, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,negQ, LOC_ECFP_0058);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let P = (x1,y1) be a point over ec
*   let R = (x3,y3)
*   return R =  k * P over ec
*
**/

void ec_fp_point_mul_naf(const UINT32 ecfp_md_id,const EC_CURVE_POINT *P,const BIGINT *k,EC_CURVE_POINT *R)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_AFF_POINT buf_1;
    EC_CURVE_AFF_POINT buf_2;
    EC_CURVE_POINT     buf_3;
#endif/* STACK_MEMORY_SWITCH */

    EC_CURVE_AFF_POINT *Q;
    EC_CURVE_AFF_POINT *mid_result;
    EC_CURVE_POINT *negP;

    EC_CURVE_AFF_POINT *tmp;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

    int *naf_buf;
    int i;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_naf: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_naf: k is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_naf: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mul_naf: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    naf_buf = g_ecfp_naf_arry[ ecfp_md_id ];
    i = bgn_fp_naf(bgnfp_md_id, k, naf_buf);
    if ( 0 == i)
    {
        ec_fp_point_set_infinit(ecfp_md_id, R);
        return ;
    }

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BIGINTSIZE + 1 < i )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_naf: i = %ld overflow.\n", i);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    mid_result = &buf_2;
    negP = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&Q, LOC_ECFP_0059);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&mid_result, LOC_ECFP_0060);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,&negP, LOC_ECFP_0061);
#endif/* STATIC_MEMORY_SWITCH */

    ec_fp_point_neg(ecfp_md_id, P, negP);
    ec_fp_point_aff_set_infinit(ecfp_md_id, Q);

    for ( ; i > 0; )
    {
        i -- ;

        ec_fp_point_aff_double(ecfp_md_id, Q, mid_result);
        tmp = Q;
        Q = mid_result;
        mid_result = tmp;

        if( 1 == naf_buf[ i ] )
        {
            ec_fp_point_mix_add(ecfp_md_id, Q, P, mid_result);
            tmp = Q;
            Q = mid_result;
            mid_result = tmp;
        }

        if( -1 == naf_buf[ i ] )
        {
            ec_fp_point_mix_add(ecfp_md_id, Q, negP, mid_result);
            tmp = Q;
            Q = mid_result;
            mid_result = tmp;
        }
    }

    ec_fp_point_aff_convert(ecfp_md_id, Q, R);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,Q, LOC_ECFP_0062);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,mid_result, LOC_ECFP_0063);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_POINT,negP, LOC_ECFP_0064);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
*   here, basepoint is fixed, so contruct a table exp_basepoint_tbl:
*       {r * basepoint: r = 2 ^ i, i = 0.. ecfp_n = nbits(ec order) }
*
**/
void ec_fp_point_mul_fix_base(const UINT32 ecfp_md_id, const BIGINT * k, EC_CURVE_POINT * R)
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

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

    EC_CURVE_POINT *ecfp_base_point;

    EC_CURVE_POINT **exp_basepoint_tbl;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_fix_base: k is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul_fix_base: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */


#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mul_fix_base: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;
    ecfp_base_point = ecfp_md->ecfp_base_point;

    if ( NULL_PTR == ecfp_base_point )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mul_fix_base: no basepoint inputed when ECFP module started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    s = g_ecfp_naf_arry[ ecfp_md_id ];

    len = bgn_fp_naf( bgnfp_md_id, k, s );

    if ( 0 == len )
    {
        ec_fp_point_set_infinit(ecfp_md_id, R);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    Q = &buf_1;
    T = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&Q, LOC_ECFP_0065);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,&T, LOC_ECFP_0066);
#endif/* STATIC_MEMORY_SWITCH */

    exp_basepoint_tbl = g_ecfp_exp_basepoint_tbl[ ecfp_md_id ];

    ec_fp_point_aff_set_infinit( ecfp_md_id, Q );
    for ( index = 0; index < len; index ++ )
    {
        if ( 1 == s[ index ] )
        {
            ec_fp_point_mix_add( ecfp_md_id, Q, exp_basepoint_tbl[ index ], T );
            tmp = Q;
            Q = T;
            T = tmp;
        }
        else if ( -1 == s[ index ] )
        {
            ec_fp_point_mix_sub( ecfp_md_id, Q, exp_basepoint_tbl[ index ], T );
            tmp = Q;
            Q = T;
            T = tmp;
        }
   }

    ec_fp_point_aff_convert( ecfp_md_id, Q, R );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,Q, LOC_ECFP_0067);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_CURVE_AFFINE_POINT,T, LOC_ECFP_0068);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
*   let P = (x1,y1) be an affine point over ec
*   let R = (x3,y3)
*   return R = k * P over ec
*
**/
void ec_fp_point_mul(const UINT32 ecfp_md_id, const EC_CURVE_POINT * P,const BIGINT * k, EC_CURVE_POINT * R)
{
#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )
    EC_CURVE_POINT **exp_basepoint_tbl;
    EC_CURVE_POINT *ecfp_base_point;
    ECFP_MD *ecfp_md;
#endif/* ECC_POINT_MUL_FIXED_BASE_SWITCH */

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == P )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul: P is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul: k is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == R )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_point_mul: R is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_point_mul: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*ECC_DEBUG_SWITCH*/

#if ( SWITCH_ON == ECC_POINT_MUL_FIXED_BASE_SWITCH )

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    ecfp_base_point = ecfp_md->ecfp_base_point;

    if ( NULL_PTR != ecfp_base_point )
    {
        exp_basepoint_tbl = g_ecfp_exp_basepoint_tbl[ ecfp_md_id ];

        if ( 0 == ec_fp_point_cmp(ecfp_md_id, P, exp_basepoint_tbl[ 0 ]  ) )
        {
            ec_fp_point_mul_fix_base(ecfp_md_id, k, R );
            return ;
        }
    }
#endif/* ECC_POINT_MUL_FIXED_BASE_SWITCH */

    ec_fp_point_mul_naf(ecfp_md_id, P, k, R );

    return ;
}

/**
*
*   compute z = x*3 + ax + b over F_p
*   return z
*
**/
void ec_fp_compute_z(const UINT32 ecfp_md_id, const BIGINT * x, BIGINT *z)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *tmp;

    ECFP_CURVE *ecfp_curve;
    BIGINT *a;
    BIGINT *b;

    ECFP_MD    *ecfp_md;
    UINT32  bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == x)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_compute_z: x is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == z)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_compute_z: z is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_compute_z: eccfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;
    ecfp_curve = ecfp_md->ecfp_curve;

    a = &(ecfp_curve->a);
    b = &(ecfp_curve->b);
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &tmp, LOC_ECFP_0069);
#endif/* STATIC_MEMORY_SWITCH */

    /* z = x*3 + ax + b */
    bgn_fp_squ(bgnfp_md_id, x, tmp);        /* tmp = x^2 */
    bgn_fp_add(bgnfp_md_id, tmp, a, tmp);   /* tmp = tmp + a = x^2 +a */
    bgn_fp_mul(bgnfp_md_id, tmp, x, tmp);   /* tmp = tmp *x = x^3 +ax */
    bgn_fp_add(bgnfp_md_id, tmp, b, z);     /* z = tmp + b = x^3 +ax +b */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, tmp, LOC_ECFP_0070);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}


/**
*
*   Let p be an odd prime and p != 1 mod 8.
*   Given x, then to compute one resolution y if following congruence has resolutions:
*       y^2 = x^3 + ax + b mod p
*   and return EC_TRUE; if it has no solution, then return EC_FALSE
*
*   Note:
*       Let z = x^3 + ax + b mod p
*   then the above congruence has resolution is equivalent to Legendre Symbol (z/p) = 1.
*   so,
*       if Legendre Symbol (z/p) = 1, then compute the square root y and return EC_TRUE
*       otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_compute_y(const UINT32 ecfp_md_id, const BIGINT * x,BIGINT *y)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *z;

    int legendresymbol;

    ECFP_MD    *ecfp_md;
    UINT32  bgnfp_md_id;

    EC_BOOL ret;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == x)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_compute_y: x is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == y)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_compute_y: y is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_compute_y: eccfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    z = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &z, LOC_ECFP_0071);
#endif/* STATIC_MEMORY_SWITCH */

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    /* z = x^3 + ax + b */
    ec_fp_compute_z(ecfp_md_id, x, z);

    legendresymbol = bgn_fp_legendre( bgnfp_md_id, z );
    if( 1 == legendresymbol )
    {
        ret = bgn_fp_squroot(bgnfp_md_id, z, y);
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, z, LOC_ECFP_0072);
#endif/* STATIC_MEMORY_SWITCH */

    return ( ret );
}

/**
*
*   to determine a point with x-coordinate x is on ec or not.
*   note, the ec over F_p is defined by
*       y^2 = x^3 + ax + b over F_p
*   if yes, return EC_TRUE;
*   if no, return EC_FALSE
*
*   which means the z = x^3 + ax + b is a squadratic mod p,i.e,
*   the Legendre Symbol (z/p) = 1
*
*   if Legendre Symbol (z/p) = 1, then return EC_TRUE
*   otherwise, return EC_FALSE
*
**/
EC_BOOL ec_fp_x_is_on_curve(const UINT32 ecfp_md_id, const BIGINT * x,BIGINT *z)
{
    int legendresymbol;

    ECFP_MD    *ecfp_md;
    UINT32  bgnfp_md_id;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == x)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_x_is_on_curve: x is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
    if ( NULL_PTR == z)
    {
        sys_log(LOGSTDOUT,"error:ec_fp_x_is_on_curve: z is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_x_is_on_curve: eccfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;

    /* z = x^3 + ax + b */
    ec_fp_compute_z(ecfp_md_id, x, z);

    legendresymbol = bgn_fp_legendre(bgnfp_md_id, z);

    if ( 1 == legendresymbol )
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   compute the discriminant of the elliptic curve
*       y^2 = x^3 + a*x + b over F_{p}
*
*   and the discriminant
*       Delta = -16 * (27 * b^2 + 4 * a^3) over F_{p}
*
*
**/
UINT32 ec_fp_disc_compute(const UINT32 ecfp_md_id, BIGINT *delta)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* SWITCH_ON == STACK_MEMORY_SWITCH */

    BIGINT *t1;
    BIGINT *t2;
    BIGINT *t3;

    BIGINT *a;
    BIGINT *b;

    ECFP_CURVE *ecfp_curve;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == delta )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_disc_compute: delta is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_disc_compute: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;
    ecfp_curve = ecfp_md->ecfp_curve;

    a = &(ecfp_curve->a);
    b = &(ecfp_curve->b);

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &buf_1;
    t2 = &buf_2;
    t3 = &buf_3;
#endif/* SWITCH_ON == STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t1, LOC_ECFP_0073);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t2, LOC_ECFP_0074);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t3, LOC_ECFP_0075);
#endif/* STATIC_MEMORY_SWITCH */

    /*compute: 16 * 27 * b^2 = 432 * b^2*/
    bgn_fp_squ (bgnfp_md_id, b , t3);
    bgn_fp_smul(bgnfp_md_id, t3, 432, t1);

    /*compute: 16 * 4 * a^3 = 64 * a^3*/
    bgn_fp_sexp(bgnfp_md_id, a , 3 , t3);
    bgn_fp_smul(bgnfp_md_id, t3, 64, t2);

    /*compute: -16 * (27 * b^2 + 4 * a^3) = - (432 * b^2 + 64 * a^3)*/
    bgn_fp_add(bgnfp_md_id, t1, t2, t3);
    bgn_fp_neg(bgnfp_md_id, t3, delta);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t1, LOC_ECFP_0076);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t2, LOC_ECFP_0077);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t3, LOC_ECFP_0078);
#endif/* STATIC_MEMORY_SWITCH */

    return ( 0 );
}

/**
*
*   compute the j-invariant of the elliptic curve
*       y^2 = x^3 + a*x + b over F_{p}
*
*   and the j-invariant
*       j = (12^3) * 4 * (a^3)/(27 * b^2 + 4 * a^3) over F_{p}
*         = -1728 * (4*a)^3 / Delta over F_{p}
*   where Delta is the discriminant of the elliptic curve.
*
**/
UINT32 ec_fp_j_invar_compute(const UINT32 ecfp_md_id, BIGINT *j_invar)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* SWITCH_ON == STACK_MEMORY_SWITCH */

    BIGINT *t1;
    BIGINT *t2;
    BIGINT *t3;
    BIGINT *delta;

    BIGINT *a;

    ECFP_CURVE *ecfp_curve;

    ECFP_MD *ecfp_md;
    UINT32 bgnfp_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == j_invar )
    {
        sys_log(LOGSTDOUT,"error:ec_fp_j_invar_compute: j_invar is null.\n");
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECFP_MD <= ecfp_md_id || 0 == g_ecfp_md[ ecfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_j_invar_compute: ecfp module #0x%lx not started.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    ecfp_md = &(g_ecfp_md[ ecfp_md_id ]);
    bgnfp_md_id = ecfp_md->bgnfp_md_id;
    ecfp_curve = ecfp_md->ecfp_curve;

    a = &(ecfp_curve->a);

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t1 = &buf_1;
    t2 = &buf_2;
    t3 = &buf_3;
    delta = &buf_4;
#endif/* SWITCH_ON == STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t1, LOC_ECFP_0079);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t2, LOC_ECFP_0080);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &t3, LOC_ECFP_0081);
    alloc_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, &delta, LOC_ECFP_0082);
#endif/* STATIC_MEMORY_SWITCH */

    /*compute the discriminant of the elliptic curve*/
    ec_fp_disc_compute(ecfp_md_id, delta);
    if ( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, delta) )
    {
        sys_log(LOGSTDOUT,
                "error:ec_fp_j_invar_compute: the discriminant of the elliptic curve is zero.\n",
                ecfp_md_id);
        dbg_exit(MD_ECFP, ecfp_md_id);
    }

    /*compute: 1/delta*/
    bgn_fp_inv(bgnfp_md_id, delta, t1);

    /*compute: 1728 * (4*a)^3 = 110592 * a^3*/
    bgn_fp_sexp(bgnfp_md_id, a, 3, t3);
    bgn_fp_smul(bgnfp_md_id, t3, 110592, t2);

    /*compute: -1728 * (4*a)^3/delta = - (1728 * (4*a)^3) * (1/delta)*/
    bgn_fp_mul(bgnfp_md_id, t1, t2, t3);
    bgn_fp_neg(bgnfp_md_id, t3, j_invar);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t1, LOC_ECFP_0083);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t2, LOC_ECFP_0084);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, t3, LOC_ECFP_0085);
    free_static_mem(MD_ECFP, ecfp_md_id, MM_BIGINT, delta, LOC_ECFP_0086);
#endif/* STATIC_MEMORY_SWITCH */

    return ( 0 );
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

