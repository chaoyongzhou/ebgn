/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
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
#include "bgnf2n.h"
#include "ecf2n.h"
#include "eccf2n.h"
#include "mm.h"

#include "debug.h"

#include "print.h"



/* set eccf2n_n = deg(f(x)) */
/* set eccf2n_f = the generator polynomial f(x) of F_{2^n}*/
static ECCF2N_MD g_eccf2n_md[ MAX_NUM_OF_ECCF2N_MD ];
static EC_BOOL  g_eccf2n_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_eccf2n_fx_buf   [ MAX_NUM_OF_ECCF2N_MD ];
static BIGINT g_eccf2n_cofactor_buf[ MAX_NUM_OF_ECCF2N_MD ];
static BIGINT g_eccf2n_order_buf[ MAX_NUM_OF_ECCF2N_MD ];
static ECF2N_CURVE  g_eccf2n_curve_buf[ MAX_NUM_OF_ECCF2N_MD ];
static EC_CURVE_POINT g_eccf2n_curve_point_buf[ MAX_NUM_OF_ECCF2N_MD ];
#endif/* STACK_MEMORY_SWITCH */

/**
*   for test only
*
*   to query the status of ECCF2N Module
*
**/
void ecc_f2n_print_module_status(const UINT32 eccf2n_md_id, LOG *log)
{
    ECCF2N_MD *eccf2n_md;
    UINT32 index;

    if ( EC_FALSE == g_eccf2n_md_init_flag )
    {

        sys_log(log,"no ECCF2N Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_ECCF2N_MD; index ++ )
    {
        eccf2n_md = &(g_eccf2n_md[ index ]);

        if ( 0 < eccf2n_md->usedcounter )
        {
            sys_log(log,"ECCF2N Module # %ld : %ld refered, refer BGNF2N Module : %ld, refer BGNZN Module : %ld,refer ECF2N Module : %ld,refer BGNZ Module : %ld\n",
                    index,
                    eccf2n_md->usedcounter,
                    eccf2n_md->bgnf2n_md_id,
                    eccf2n_md->bgnzn_md_id,
                    eccf2n_md->ecf2n_md_id,
                    eccf2n_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed ECCF2N module
*
*
**/
UINT32 ecc_f2n_free_module_static_mem(const UINT32 eccf2n_md_id)
{
    ECCF2N_MD *eccf2n_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 ecf2n_md_id;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_free_module_static_mem: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    ecf2n_md_id  = eccf2n_md->ecf2n_md_id;
    bgnzn_md_id  = eccf2n_md->bgnzn_md_id;
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    bgnz_md_id   = eccf2n_md->bgnz_md_id;

    free_module_static_mem(MD_ECCF2N, eccf2n_md_id);

    ec_f2n_free_module_static_mem(ecf2n_md_id);
    bgn_zn_free_module_static_mem(bgnzn_md_id);
    bgn_f2n_free_module_static_mem(bgnf2n_md_id);
    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}

/**
*
* start ECCF2N module
*
**/
UINT32 ecc_f2n_start( const BIGINT *f_x,
                    const ECF2N_CURVE *curve,
                    const BIGINT *cofactor,
                    const BIGINT *order,
                    const EC_CURVE_POINT *base_point,
                    const UINT32 ( *random_generator)( BIGINT * random),
                    const UINT32 (*do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash))
{
    UINT32    eccf2n_md_id;
    UINT32          eccf2n_n;
    BIGINT         *eccf2n_f;
    ECF2N_CURVE    *eccf2n_curve;
    BIGINT         *eccf2n_cofactor;
    BIGINT         *eccf2n_order;
    EC_CURVE_POINT *eccf2n_base_point;

    ECCF2N_MD   *eccf2n_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 ecf2n_md_id;
    UINT32 bgnz_md_id;
    const UINT32 (*eccf2n_random_generator)(BIGINT * random);
    const UINT32 (*eccf2n_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == f_x )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: fx is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == curve )
    {
         dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: curve is NULL_PTR.\n");
         dbg_exit(MD_ECCF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == order )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: order is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, ERR_MODULE_ID);
    }
    if ( NULL_PTR == base_point )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: base_point is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNZMOD module, then */
    /* initialize g_eccf2n_md */
    if ( EC_FALSE ==  g_eccf2n_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_ECCF2N_MD; index ++ )
        {
            eccf2n_md = &(g_eccf2n_md[ index ]);

            eccf2n_md->usedcounter          = 0;
            eccf2n_md->bgnf2n_md_id         = ERR_MODULE_ID;
            eccf2n_md->ecf2n_md_id          = ERR_MODULE_ID;
            eccf2n_md->bgnz_md_id           = ERR_MODULE_ID;
            eccf2n_md->eccf2n_n            = 0;
            eccf2n_md->eccf2n_f            = NULL_PTR;
            eccf2n_md->eccf2n_curve        = NULL_PTR;
            eccf2n_md->eccf2n_cofactor     = NULL_PTR;
            eccf2n_md->eccf2n_order        = NULL_PTR;
            eccf2n_md->eccf2n_base_point   = NULL_PTR;
            eccf2n_md->eccf2n_random_generator = NULL_PTR;
            eccf2n_md->eccf2n_do_hash = NULL_PTR;
        }

        /*register all functions of ECCF2N module to DBG module*/
        //dbg_register_func_addr_list(g_eccf2n_func_addr_list, g_eccf2n_func_addr_list_len);

        g_eccf2n_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_ECCF2N_MD; index ++ )
    {
        eccf2n_md_id = index;
        eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);

        if ( 0 == eccf2n_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_ECCF2N_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnf2n_md_id         = ERR_MODULE_ID;
    ecf2n_md_id          = ERR_MODULE_ID;
    bgnz_md_id           = ERR_MODULE_ID;
    eccf2n_n            = 0;
    eccf2n_f            = NULL_PTR;
    eccf2n_curve        = NULL_PTR;
    eccf2n_cofactor     = NULL_PTR;
    eccf2n_order        = NULL_PTR;
    eccf2n_base_point   = NULL_PTR;
    eccf2n_random_generator = NULL_PTR;
    eccf2n_do_hash = NULL_PTR;

    eccf2n_md_id = index;
    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,         &eccf2n_f           , LOC_ECCF2N_0001);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_ECF2N_CURVE ,   &eccf2n_curve       , LOC_ECCF2N_0002);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,         &eccf2n_cofactor    , LOC_ECCF2N_0003);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,         &eccf2n_order       , LOC_ECCF2N_0004);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT,    &eccf2n_base_point  , LOC_ECCF2N_0005);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    eccf2n_f            = &(g_eccf2n_fx_buf[ eccf2n_md_id ]);
    eccf2n_curve        = &(g_eccf2n_curve_buf[ eccf2n_md_id ]);
    eccf2n_cofactor     = &(g_eccf2n_cofactor_buf[ eccf2n_md_id ]);
    eccf2n_order        = &(g_eccf2n_order_buf[ eccf2n_md_id ]);
    eccf2n_base_point   = &(g_eccf2n_curve_point_buf[ eccf2n_md_id ]);
#endif/* STACK_MEMORY_SWITCH */


#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == eccf2n_f )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: ec_f2n_f is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == eccf2n_curve )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: ec_f2n_curve is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == eccf2n_cofactor )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: eccf2n_cofactor is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == eccf2n_order )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: ec_f2n_order is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == eccf2n_base_point )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_start: ec_f2n_base_point is NULL_PTR.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* start ECF2N module */
    ecf2n_md_id = ec_f2n_start( f_x, curve, order, base_point );

    /* start BGNF2N module */
    bgnf2n_md_id = bgn_f2n_start( f_x );

    /* start BGNZN module */
    bgnzn_md_id = bgn_zn_start( order );

    /* start BGNZ module */
    bgnz_md_id = bgn_z_start();

    /* set eccf2n_f = f_x */
    bgn_f2n_clone(bgnf2n_md_id, f_x, eccf2n_f);

    /* set eccf2n_n = deg(f_x) */
    eccf2n_n = bgn_f2n_deg(bgnf2n_md_id, f_x);

    /* set eccf2n_curve = curve */
    bgn_f2n_clone(bgnf2n_md_id, &(curve->a), &(eccf2n_curve->a));
    bgn_f2n_clone(bgnf2n_md_id, &(curve->b), &(eccf2n_curve->b));

    /* set eccf2n_cofactor = cofactor */
    bgn_z_clone(bgnz_md_id, cofactor, eccf2n_cofactor);

    /* set eccf2n_order = order */
    bgn_z_clone(bgnz_md_id, order, eccf2n_order);

    /* set eccf2n_base_point = base_point */
    bgn_f2n_clone(bgnf2n_md_id, &(base_point->x), &(eccf2n_base_point->x));
    bgn_f2n_clone(bgnf2n_md_id, &(base_point->y), &(eccf2n_base_point->y));

    /* set user defined interface of random generator */
    if ( NULL_PTR ==  random_generator)
    {
        eccf2n_random_generator = ecc_f2n_random_generator_default;
    }
    else
    {
        eccf2n_random_generator = random_generator;
    }

    /* set user defined interface of hash function */
    if ( NULL_PTR == do_hash )
    {
        eccf2n_do_hash = ecc_f2n_do_hash_default;
    }
    else
    {
        eccf2n_do_hash = do_hash;
    }

    /* set module : */
    eccf2n_md->usedcounter         = 0;
    eccf2n_md->bgnzn_md_id         = bgnzn_md_id;
    eccf2n_md->bgnf2n_md_id        = bgnf2n_md_id;
    eccf2n_md->ecf2n_md_id         = ecf2n_md_id;
    eccf2n_md->bgnz_md_id          = bgnz_md_id;
    eccf2n_md->eccf2n_n            = eccf2n_n;
    eccf2n_md->eccf2n_f            = eccf2n_f;
    eccf2n_md->eccf2n_curve        = eccf2n_curve;
    eccf2n_md->eccf2n_cofactor     = eccf2n_cofactor;
    eccf2n_md->eccf2n_order        = eccf2n_order;
    eccf2n_md->eccf2n_base_point   = eccf2n_base_point;
    eccf2n_md->eccf2n_random_generator = eccf2n_random_generator;
    eccf2n_md->eccf2n_do_hash = eccf2n_do_hash;

    /* at the first time, set the counter to 1 */
    eccf2n_md->usedcounter = 1;

    return ( eccf2n_md_id );
}

/**
*
* end ECCF2N module
*
**/
void ecc_f2n_end(const UINT32 eccf2n_md_id)
{
    UINT32          eccf2n_n;
    BIGINT         *eccf2n_f;
    ECF2N_CURVE    *eccf2n_curve;
    BIGINT         *eccf2n_cofactor;
    BIGINT         *eccf2n_order;
    EC_CURVE_POINT *eccf2n_base_point;

    ECCF2N_MD   *eccf2n_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 ecf2n_md_id;
    UINT32 bgnz_md_id;

    if ( MAX_NUM_OF_ECCF2N_MD < eccf2n_md_id )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_end: eccf2n_md_id = %ld is overflow.\n",eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < eccf2n_md->usedcounter )
    {
        eccf2n_md->usedcounter --;
        return ;
    }

    if ( 0 == eccf2n_md->usedcounter )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_end: eccf2n_md_id = %ld is not started.\n",eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnzn_md_id          = eccf2n_md->bgnzn_md_id;
    bgnf2n_md_id         = eccf2n_md->bgnf2n_md_id;
    ecf2n_md_id          = eccf2n_md->ecf2n_md_id;
    bgnz_md_id           = eccf2n_md->bgnz_md_id;
    eccf2n_n            = eccf2n_md->eccf2n_n;
    eccf2n_f            = eccf2n_md->eccf2n_f;
    eccf2n_curve        = eccf2n_md->eccf2n_curve;
    eccf2n_cofactor     = eccf2n_md->eccf2n_cofactor;
    eccf2n_order        = eccf2n_md->eccf2n_order;
    eccf2n_base_point   = eccf2n_md->eccf2n_base_point;

    bgn_z_end( bgnz_md_id );
    bgn_zn_end( bgnzn_md_id );
    bgn_f2n_end( bgnf2n_md_id );
    ec_f2n_end( ecf2n_md_id );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,          eccf2n_f            , LOC_ECCF2N_0006);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_ECF2N_CURVE ,    eccf2n_curve        , LOC_ECCF2N_0007);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,          eccf2n_cofactor     , LOC_ECCF2N_0008);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT,          eccf2n_order        , LOC_ECCF2N_0009);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT,     eccf2n_base_point   , LOC_ECCF2N_0010);
#endif/* STATIC_MEMORY_SWITCH */

    /* free module : */
    //ecc_f2n_free_module_static_mem(eccf2n_md_id);
    eccf2n_md->bgnzn_md_id          = ERR_MODULE_ID;
    eccf2n_md->bgnf2n_md_id         = ERR_MODULE_ID;
    eccf2n_md->ecf2n_md_id          = ERR_MODULE_ID;
    eccf2n_md->bgnz_md_id           = ERR_MODULE_ID;
    eccf2n_md->eccf2n_n            = 0;
    eccf2n_md->eccf2n_f            = NULL_PTR;
    eccf2n_md->eccf2n_curve        = NULL_PTR;
    eccf2n_md->eccf2n_cofactor     = NULL_PTR;
    eccf2n_md->eccf2n_order        = NULL_PTR;
    eccf2n_md->eccf2n_base_point   = NULL_PTR;
    eccf2n_md->eccf2n_random_generator = NULL_PTR;
    eccf2n_md->eccf2n_do_hash = NULL_PTR;
    eccf2n_md->usedcounter          = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   compute z = x + a + ( b / x^2 ) over F_{2^n}
*   where F_{2^n} is defined in ECCF2N Module refered by module id eccf2n_md_id
*   return z
*
**/
static void ecc_f2n_compute_z(const UINT32 eccf2n_md_id, const BIGINT *x, BIGINT *z)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *tmp;
    BIGINT *a;
    BIGINT *b;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnf2n_md_id;
    ECF2N_CURVE *eccf2n_curve;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == x )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_z: x is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == z )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_z: z is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_compute_z: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &tmp, LOC_ECCF2N_0011);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    eccf2n_curve = eccf2n_md->eccf2n_curve;

    a = (BIGINT *)&(eccf2n_curve->a);
    b = (BIGINT *)&(eccf2n_curve->b);
    /* z = x + a + ( b / x^2 ) */
    bgn_f2n_squ(bgnf2n_md_id, x,   tmp );
    bgn_f2n_inv(bgnf2n_md_id, tmp, z   );
    bgn_f2n_mul(bgnf2n_md_id, b,   z, z);
    bgn_f2n_add(bgnf2n_md_id, x,   z, z);
    bgn_f2n_add(bgnf2n_md_id, a,   z, z);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0012);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   g(z) = SUM { z ^ (2 ^ (2 *j)): j = 0.. ( r - 1 ) / 2 and z belong to F_2^r }
*   Let g(k, z) = SUM { z ^ (2 ^ (2 *j)): j = 0.. k and z belong to F_2^r }
*   Then g(z) = g ( ( r - 1 ) / 2, z) and
*       g(k + 1, z) = z + (g(k, z)) ^ 4
**/
static void ecc_f2n_compute_gz(const UINT32 eccf2n_md_id, const BIGINT *z, const UINT32 maxindex, BIGINT *gz)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *tmp;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnf2n_md_id;

    UINT32 index;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == z )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_gz: z is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == gz )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_gz: gz is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_compute_gz: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &tmp, LOC_ECCF2N_0013);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;

    /* g(0,z) = z */
    bgn_f2n_clone(bgnf2n_md_id, z, tmp);

    /* g(k + 1, z) = z + (g(k, z)) ^ 4 */
    for ( index = 0; index < maxindex; index ++ )
    {
        bgn_f2n_squ( bgnf2n_md_id, tmp, tmp );
        bgn_f2n_squ( bgnf2n_md_id, tmp, tmp );
        bgn_f2n_add( bgnf2n_md_id, tmp, z, tmp);
    }

    bgn_f2n_clone(bgnf2n_md_id, tmp, gz);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0014);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   Formula : g(z) + g(z)^2 = z + tr(z)
*   Hence
*       tr(z) = z + g(z) + g(z)^2 for any z belong to F_{2^n}
*
**/
static void ecc_f2n_compute_trace(const UINT32 eccf2n_md_id, const BIGINT *z, const BIGINT *gz, BIGINT *trace)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *tmp;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnf2n_md_id;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == z )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_trace: z is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == gz )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_trace: gz is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == trace )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_compute_trace: trace is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_compute_trace: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &tmp, LOC_ECCF2N_0015);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;

    bgn_f2n_squ(bgnf2n_md_id, gz,tmp);
    bgn_f2n_add(bgnf2n_md_id, gz,tmp, tmp);
    bgn_f2n_add(bgnf2n_md_id, z, tmp, trace);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0016);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
*   let z = x + a + ( b / x^2 )
*   compute g(z) and trace
*   if trace  = 0, then return EC_TRUE and gz = g(z)
*   if trace != 0, then return EC_FALSE and gz = g(z)
*
**/
static EC_BOOL ecc_f2n_x_is_on_curve(const UINT32 eccf2n_md_id, const BIGINT * x, BIGINT * gz)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *z;
    BIGINT *trace;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnf2n_md_id;
    UINT32  eccf2n_n;

    EC_BOOL ret;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == x )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_x_is_on_curve: x is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == gz )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_x_is_on_curve: gz is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_x_is_on_curve: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    z = &buf_1;
    trace = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &z, LOC_ECCF2N_0017);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &trace, LOC_ECCF2N_0018);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    eccf2n_n = eccf2n_md->eccf2n_n;

    ecc_f2n_compute_z(eccf2n_md_id, x, z);
    ecc_f2n_compute_gz(eccf2n_md_id, z, ( eccf2n_n - 1 ) / 2, gz);
    ecc_f2n_compute_trace(eccf2n_md_id, z, gz, trace);

    if ( EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, trace) )
    {
        ret = EC_TRUE;
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, z, LOC_ECCF2N_0019);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, trace, LOC_ECCF2N_0020);
#endif/* STATIC_MEMORY_SWITCH */

    return ret;
}

/**
*
*   compute the public key according to the given private key
*   on the elliptic curve(ec) refered by the Module ID ecf2n_md_id
*
*   Note:
*       public key = private key * basepoint
*   where basepoint is on the subgroup (order = ecf2n_order) of the
*   elliptic curve(ec) and refered by the Module ID ecf2n_md_id
*
**/
UINT32 ecc_f2n_get_public_key(const UINT32 eccf2n_md_id, const BIGINT *privatekey,EC_CURVE_POINT *publickey)
{
    ECCF2N_MD *eccf2n_md;
    UINT32 bgnf2n_md_id;
    UINT32 ecf2n_md_id;
    BIGINT *eccf2n_order;
    EC_CURVE_POINT *eccf2n_base_point;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey)
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_get_public_key: privatekey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == publickey)
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_get_public_key: publickey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_get_public_key: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    ecf2n_md_id  = eccf2n_md->ecf2n_md_id;
    eccf2n_order = eccf2n_md->eccf2n_order;
    eccf2n_base_point = eccf2n_md->eccf2n_base_point;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )

    if( bgn_f2n_cmp(bgnf2n_md_id, privatekey, eccf2n_order) >= 0 )
    {  /* >=N; error */
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_get_public_key: private key > ec order.\n");

        dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"private key:\n");
        print_bigint(LOGSTDOUT,privatekey);

        dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ec order:\n");
        print_bigint(LOGSTDOUT,eccf2n_order);
        return ((UINT32)(-1));
    }
    else if( EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, privatekey) )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_get_public_key: private key = 0 \n");
        return ((UINT32)(-1));
    }
#endif /* ECC_DEBUG_SWITCH */

    ec_f2n_point_mul(ecf2n_md_id, eccf2n_base_point, privatekey, publickey);

    return ( 0 );
}

/**
*
*   default random generator function to generate a random
*
**/
const UINT32 ecc_f2n_random_generator_default( BIGINT * random)
{
    UINT32 index;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == random)
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_random_generator_default: random is null.\n");
        return ((UINT32)(-1));
    }
#endif /* ECC_DEBUG_SWITCH */

    for ( index = 0; index < INTMAX; index ++ )
    {
        random->data[ index ] = 0xAAAAAAAA;
    }
    random->len = INTMAX;

    return ( 0 );
}

/**
*
*   call the random generator function to generate a random as the privatekey
*
*   note:
*       the bigint privatekey should be less than the order of the ec
*   i.e,
*       privatekey < eccf2n_order
*
**/
void ecc_f2n_rnd_private_key(const UINT32 eccf2n_md_id, BIGINT * privatekey)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *random;
    BIGINT *q;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnz_md_id;
    BIGINT *eccf2n_order;
    const UINT32 ( *eccf2n_random_generator)( BIGINT * random);

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey)
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_rnd_private_key: privatekey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_rnd_private_key: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    random = &buf_1;
    q = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &random, LOC_ECCF2N_0021);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &q, LOC_ECCF2N_0022);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnz_md_id = eccf2n_md->bgnz_md_id;
    eccf2n_order = eccf2n_md->eccf2n_order;
    eccf2n_random_generator = eccf2n_md->eccf2n_random_generator;

    /* generator a random */
    eccf2n_random_generator( random );

    if ( 0 <= bgn_z_cmp( bgnz_md_id, random, eccf2n_order) )
    {
        bgn_z_div(bgnz_md_id, random, eccf2n_order, q, privatekey);
    }
    else
    {
        bgn_z_clone(bgnz_md_id, random, privatekey);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, random, LOC_ECCF2N_0023);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, q, LOC_ECCF2N_0024);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   generate a random key pair of ECDSA
*   the private key of the key pair is a random number which is less than the ec order
*   and the public key = private key * basepoint
*
**/
UINT32 ecc_f2n_generate_keypair(const UINT32 eccf2n_md_id, ECC_KEYPAIR *keypair)
{
    BIGINT *privatekey;
    EC_CURVE_POINT *publickey;
#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == keypair)
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_generate_keypair: keypair is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_generate_keypair: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    privatekey = &(keypair->private_key);
    publickey  = &(keypair->public_key);

    ecc_f2n_rnd_private_key(eccf2n_md_id, privatekey);
    ecc_f2n_get_public_key(eccf2n_md_id, privatekey, publickey);

    return ( 0 );
}

/**
*
*   the message (plaintxt) length is not more than (deg(fx) - nlossbits).
*   where nlossbits is the number of loss bits.
*   let m = message,then
*   consider {k * m + i, i = 0... (k-1)}
*   where k = 2 ^{nlossbits}
*   select the minmum from the set s.t.
*   when regard it as the x-coordinate of a point, the point is on the subgroup of ec
*   which is defined and refered by the module id eccf2n_md_id and where the order of
*   the subgroup is eccf2n_order.
*
*   return this point
*
*   note:
*       here let k = 4 which means we have to loss 2 bits of the message
*   relative to the length of the x-coordinate of the ec point.
*
*   History:
*       1.  2006.12.22 - add one loop while(1) to check the msgpoint is on the subgroup of ec or not,
*           where the subgroup order = eccf2n_order
*           so that, we restrict the all operation is on the subgroup.
*           if the base point we select is on ec but not on the subgroup, ECDSA will fail.
*
**/
int ecc_f2n_encoding(const UINT32 eccf2n_md_id, const UINT8 * message,const UINT32 messagelen, EC_CURVE_POINT * msgpoint)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    EC_CURVE_POINT buf_5;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *msg;
    BIGINT *tmp;
    BIGINT *gz;
    BIGINT *x;
    BIGINT *y;
    EC_CURVE_POINT *R;

    UINT32  nlossbits;

    UINT32 byte_idx;
    UINT32 word_offset;
    UINT32 byte_offset;
    UINT32 message_byte;

    UINT32 nbytes_max;
    UINT32 nbytes_per_word;
    UINT32 nbytes_per_msg;

    UINT32 j;
    UINT32 j_max;

    ECCF2N_MD *eccf2n_md;
    UINT32 ecf2n_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 bgnz_md_id;
    UINT32  eccf2n_n;
    BIGINT *eccf2n_order;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:test_ecc_f2n_encryption: message is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:test_ecc_f2n_encryption: msgpoint is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:test_ecc_f2n_encryption: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    msg = &buf_2;
    tmp = &buf_3;
    gz = &buf_4;
    R  = &buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &msg, LOC_ECCF2N_0025);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &tmp, LOC_ECCF2N_0026);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &gz, LOC_ECCF2N_0027);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &R, LOC_ECCF2N_0028);
#endif/* STATIC_MEMORY_SWITCH */


    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    ecf2n_md_id  = eccf2n_md->ecf2n_md_id;
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    bgnz_md_id   = eccf2n_md->bgnz_md_id;
    eccf2n_n = eccf2n_md->eccf2n_n;
    eccf2n_order = eccf2n_md->eccf2n_order;

    x = &(msgpoint->x);
    y = &(msgpoint->y);

    /* note: if msg is 160 bits, then nlossbits <= 2 */
    /* number of loss bits*/
    nlossbits = NUM_OF_ECCF2N_LOSS_BITS;

    /* how many bytes per word */
    nbytes_per_word = WORDSIZE / BYTESIZE;

    /* how many bytes will be acceptable in the msg */
    nbytes_per_msg = ( ( eccf2n_n - nlossbits ) / BYTESIZE );

    /* set the first loop length nbytes_max */
    if ( nbytes_per_msg < messagelen )
    {
        nbytes_max = nbytes_per_msg;
    }
    else
    {
        nbytes_max = messagelen;
    }

    /* 1st loop */
    for ( byte_idx = 0; byte_idx < nbytes_max; byte_idx ++ )
    {
        word_offset = ( byte_idx / nbytes_per_word );
        byte_offset = ( byte_idx % nbytes_per_word );

        message_byte = message[ byte_idx ];
        if ( 0 == byte_offset )
        {

            msg->data[ word_offset ] = message_byte;
        }
        else
        {
            msg->data[ word_offset ] |= ( message_byte << ( byte_offset *  BYTESIZE ) );
        }
    }

    /* 2nd loop */
    for ( ; byte_idx < nbytes_per_msg; byte_idx ++ )
    {
        word_offset = ( byte_idx / nbytes_per_word );
        byte_offset = ( byte_idx % nbytes_per_word );

        message_byte = '0';
        if ( 0 == byte_offset )
        {

            msg->data[ word_offset ] = message_byte;
        }
        else
        {
            msg->data[ word_offset ] |= ( message_byte << ( byte_offset *  BYTESIZE ) );
        }
    }

    /* get the accurate length of msg */
    word_offset ++;
    for ( ; word_offset > 0; )
    {
        word_offset --;
        if ( 0 != msg->data[ word_offset ] )
        {
            word_offset ++;
            break;
        }
    }
    msg->len = word_offset;

    /* if msg = 0 , P = 0 */
    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, msg) )
    {
        ec_f2n_point_set_infinit(ecf2n_md_id, msgpoint);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, msg, LOC_ECCF2N_0029);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0030);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, gz, LOC_ECCF2N_0031);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, R, LOC_ECCF2N_0032);
#endif/* STATIC_MEMORY_SWITCH */

        return ( 0 );
    }

    /*k * m*/
    bgn_z_shl_lesswordsize(bgnz_md_id, msg, nlossbits, tmp);

    j = 1;
    j_max = (1 << nlossbits);

    /*k * m + 1*/
    tmp->data[0] ++;
    while( j < j_max )
    {
        if ( EC_TRUE == ecc_f2n_x_is_on_curve(eccf2n_md_id, tmp,gz ) )
        {
            /* msgpoint's x */
            bgn_f2n_clone(bgnf2n_md_id, tmp, x);

            /* msgpoint's y = x * g(z) */
            bgn_f2n_mul(bgnf2n_md_id, gz, x, y);

            ec_f2n_point_mul(ecf2n_md_id, msgpoint, eccf2n_order, R);

            if ( EC_TRUE == ec_f2n_point_is_infinit(ecf2n_md_id, R))
            {
#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
                dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"test_ecc_f2n_encryption: after %d times searching before encoding success!\n",j);
#endif /* ENC_DEC_DEBUG_SWITCH */

                break;
            }
        }

        j++;

        /*k * m + j*/
        tmp->data[ 0 ]++;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, msg, LOC_ECCF2N_0033);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0034);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, gz, LOC_ECCF2N_0035);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, R, LOC_ECCF2N_0036);
#endif/* STATIC_MEMORY_SWITCH */

    if( j_max == j )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:test_ecc_f2n_encryption: encoding failure!");
        return ( -1);
    }

    return ( 0 );
}

/**
*
*   from the encoding rule, we know the message is embedded as
*   the x-coordinate of the ec point;
*   furthermore, the message is the higher bits of the x-coordinate.
*
*   so, the decoding rule is to copy the higher bits the x-coordinate
*   of the point to message.
*
*   note:
*   the number of higher bits is determined by the degree of generator polynomial
*   of the base field F_{2^n},i.e, eccf2n_n
*
*   return the message ( plaintxt )
*
**/
void ecc_f2n_decoding(const UINT32 eccf2n_md_id, const EC_CURVE_POINT * msgpoint, const UINT32 maxmessagelen,UINT8 *message, UINT32 *messagelen )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *msg;
    BIGINT *x;

    UINT32  nlossbits;

    UINT32 nbytes_per_word;
    UINT32 nbytes_per_msg;

    UINT32 nbytes_max;
    UINT32 msg_word;
    UINT32 bytemask;

    UINT32 byte_idx;
    UINT32 word_offset;
    UINT32 byte_offset;

    ECCF2N_MD *eccf2n_md;
    UINT32 ecf2n_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 bgnz_md_id;
    UINT32  eccf2n_n;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decoding: message is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decoding: msgpoint is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_decoding: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    msg = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &msg, LOC_ECCF2N_0037);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    ecf2n_md_id  = eccf2n_md->ecf2n_md_id;
    bgnf2n_md_id = eccf2n_md->bgnf2n_md_id;
    bgnz_md_id   = eccf2n_md->bgnz_md_id;
    eccf2n_n = eccf2n_md->eccf2n_n;

    x = (BIGINT *)&(msgpoint->x);

    /* note: if message is 160 bits, then nlossbits <= 2 */
    nlossbits = NUM_OF_ECCF2N_LOSS_BITS;/* number of loss bits*/

    if ( EC_TRUE == ec_f2n_point_is_infinit(ecf2n_md_id,  msgpoint))
    {
        bgn_z_set_zero(bgnz_md_id, msg);
    }
    else
    {
        bgn_z_shr_lesswordsize(bgnz_md_id, x, nlossbits, msg);
    }

    /* how many bytes per word */
    nbytes_per_word = WORDSIZE / BYTESIZE;

    /* how many bytes will be acceptable in the msg */
    nbytes_per_msg = ( ( eccf2n_n - nlossbits ) / BYTESIZE );

    /* set the bytemask */
    bytemask =  (~((~((UINT32)0)) << BYTESIZE));

    /* set the first loop length nbytes_max */
    if ( nbytes_per_msg < maxmessagelen )
    {
        nbytes_max = nbytes_per_msg;
    }
    else
    {
        nbytes_max = maxmessagelen;
    }

    for ( byte_idx = 0; byte_idx < nbytes_max; byte_idx ++ )
    {
        word_offset = ( byte_idx / nbytes_per_word );
        byte_offset = ( byte_idx % nbytes_per_word );

        if ( word_offset < msg->len )
        {
            msg_word = msg->data[ word_offset ];
        }
        else
        {
            msg_word = 0;
        }

        message[ byte_idx ] = (UINT8)( ( msg_word >> ( byte_offset *  BYTESIZE ) ) & bytemask );
    }

    *messagelen = nbytes_max;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, msg, LOC_ECCF2N_0038);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
* Encryption:
*   ( kG, m + kP )
* where
*   k : random number
*   G : base point over EC
*   r : private key, integer number
*   P : public key ( = rG )
*   m : message point/ plain text
* define
*   c1 = kG
*   c2 = m + kP
*
*   return (c1,c2)
**/
void ecc_f2n_encryption(const UINT32 eccf2n_md_id,
                        const EC_CURVE_POINT * publickey,
                        const EC_CURVE_POINT * msgpoint,
                        EC_CURVE_POINT * c1,
                        EC_CURVE_POINT * c2)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *tmp;
    BIGINT *k;

    ECCF2N_MD *eccf2n_md;
    UINT32 ecf2n_md_id;
    EC_CURVE_POINT *eccf2n_base_point;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == publickey )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_encryption: publickey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_encryption: msgpoint is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_encryption: c1 is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == c2 )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_encryption: c2 is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_encryption: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf_1;
    k = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &tmp, LOC_ECCF2N_0039);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &k, LOC_ECCF2N_0040);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    ecf2n_md_id = eccf2n_md->ecf2n_md_id;
    eccf2n_base_point = eccf2n_md->eccf2n_base_point;

    ecc_f2n_rnd_private_key(eccf2n_md_id, k);

    /* c1 = kG */
    ec_f2n_point_mul(ecf2n_md_id, eccf2n_base_point, k, c1);

    /* c2 = m + kP */
    ec_f2n_point_mul(ecf2n_md_id, publickey, k, tmp);
    ec_f2n_point_add(ecf2n_md_id, msgpoint, tmp, c2);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp, LOC_ECCF2N_0041);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, k, LOC_ECCF2N_0042);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
* Decryption:
*   m = c2 - r * c1
* where c1 = kG, c2 = m + kP and
*   k : random number
*   G : base point over EC
*   r : private key, integer number
*   P : public key ( = rG )
*   m : message point/ plain text
**/
void ecc_f2n_decryption(const UINT32 eccf2n_md_id,
                        const BIGINT * privatekey,
                        const EC_CURVE_POINT * c1,
                        const EC_CURVE_POINT * c2,
                        EC_CURVE_POINT * msgpoint)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *tmp;

    ECCF2N_MD *eccf2n_md;
    UINT32 ecf2n_md_id;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decryption: privatekey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decryption: msgpoint is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decryption: c1 is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == c2 )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_decryption: c2 is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_decryption: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &tmp, LOC_ECCF2N_0043);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    ecf2n_md_id = eccf2n_md->ecf2n_md_id;

    /* m = c2 - k * c1 */
    ec_f2n_point_mul(ecf2n_md_id, c1, privatekey, tmp);
    ec_f2n_point_sub(ecf2n_md_id, c2, tmp, msgpoint);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp, LOC_ECCF2N_0044);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
*   return a constant....
*
**/
const UINT32 ecc_f2n_do_hash_default(const UINT8 *message,const UINT32 messagelen, BIGINT *hash)
{
    UINT32 index;
#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_do_hash_default: message is null.\n");
        return ((UINT32)(-1));
    }
    if ( NULL_PTR == hash )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_do_hash_default: hash is null.\n");
        return ((UINT32)(-1));
    }
#endif /* ECDSA_DEBUG_SWITCH */

    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"\n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: --------------------------------------------\n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: this function is only a debug interface.    \n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: so, it always returns the constant value.   \n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: do not worry since it will be replaced with \n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: user's defined interface.                   \n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"ecc_f2n_do_hash_default: --------------------------------------------\n");
    dbg_log(SEC_0096_ECCF2N, 5)(LOGSTDOUT,"\n");

    for ( index = 0; index < INTMAX; index ++ )
    {
        hash->data[ index ] = 0xaaaaaaaa;
    }

    hash->len = INTMAX;

    return ( 0 );
}

/**
*
*   generate the ecc signature of the message with the private key
*   return the signature
*
*   Algorithm:
*   Input: message m , private key d , elliptic curve ec and the base point G
*   Output: signature
*   0.  let the order of the ec be n
*   1.  compute the hash value h of m
*   2.  generate a random number k where 1 <= k < n
*   3.  compute kG = (x1,y1)
*   4.  compute r = x1 mod n
*   5.  compute s = (h + d * r)/k mod n
*   6.  if r = 0 or s = 0, then goto 2.; else goto 7.
*   7.  signature = (r, s)
*   8.  return signature
*
**/
UINT32  ecc_f2n_signate(const UINT32 eccf2n_md_id,
                    const BIGINT * privatekey,
                    const UINT8 * message,
                    const UINT32 messagelen,
                    ECC_SIGNATURE *signature)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_5;
    BIGINT buf_6;
    EC_CURVE_POINT buf_4;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *hash;
    BIGINT *k;
    BIGINT *tmp;
    BIGINT *r;
    BIGINT *s;
    BIGINT *x;

    BIGINT *t_q;
    BIGINT *t_r;

    EC_CURVE_POINT *kP;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnf2n_md_id;
    UINT32 ecf2n_md_id;
    UINT32 bgnz_md_id;
    BIGINT *eccf2n_order;
    EC_CURVE_POINT *eccf2n_base_point;
    const UINT32 ( *eccf2n_random_generator)( BIGINT * random);
    const UINT32 (*eccf2n_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

    UINT32 counter;

#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_signate: privatekey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == message )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_signate: message is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == signature )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_signate: signature is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ECDSA_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_signate: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    hash   = &buf_1;
    k   = &buf_2;
    tmp = &buf_3;
    kP  = &buf_4;
    t_q = &buf_5;
    t_r = &buf_6;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &hash, LOC_ECCF2N_0045);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &k, LOC_ECCF2N_0046);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &tmp, LOC_ECCF2N_0047);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &kP, LOC_ECCF2N_0048);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &t_q, LOC_ECCF2N_0049);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &t_r, LOC_ECCF2N_0050);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnzn_md_id        = eccf2n_md->bgnzn_md_id;
    bgnf2n_md_id       = eccf2n_md->bgnf2n_md_id;
    ecf2n_md_id        = eccf2n_md->ecf2n_md_id;
    bgnz_md_id         = eccf2n_md->bgnz_md_id;
    eccf2n_order       = eccf2n_md->eccf2n_order;
    eccf2n_base_point  = eccf2n_md->eccf2n_base_point;
    eccf2n_do_hash     = eccf2n_md->eccf2n_do_hash;
    eccf2n_random_generator = eccf2n_md->eccf2n_random_generator;

    r = &(signature->r);
    s = &(signature->s);

    bgn_z_set_zero( bgnz_md_id, r );
    bgn_z_set_zero( bgnz_md_id, s );

    /* do message digest*/
    eccf2n_do_hash( message, messagelen, hash );

    /* if hash > ecc order, then do hash = hash mod order */
    if ( bgn_z_cmp(bgnz_md_id, hash, eccf2n_order)  >= 0 )
    {
        bgn_z_div(bgnz_md_id, hash, eccf2n_order, t_q, t_r);
        bgn_z_clone(bgnz_md_id, t_r, hash);
    }

    /* counter is to count the loop times. if exceed one threshold, then break and return failure */
    counter = 0;
    while(counter < MAX_LOOP_LEN_OF_SIGNATURE &&  EC_TRUE == bgn_z_is_zero( bgnz_md_id, s ) )
    {
        do
        {
            /* increase the loop times counter*/
            counter ++;

            /* generate random number k */
            eccf2n_random_generator( k );

            /* k = k mod N */
            if ( 0 <= bgn_z_cmp(bgnz_md_id, k, eccf2n_order ) )
            {
                bgn_z_div(bgnz_md_id, k, eccf2n_order, t_q, t_r);
                bgn_z_clone(bgnz_md_id, t_r, k);
            }

            ec_f2n_point_mul( ecf2n_md_id, eccf2n_base_point, k, kP );

            /* r = x mod N */
            /* since abs( q + 1 - N ) <= 2 * sqrt( q )
            *  => q - 4 * sqrt( q ) + 2 <= 2 * N - q <= q + 4 * sqrt( q ) + 2
            *  note sqrt( q ) >> 4, we have
            *       q < 2 * N
            *  which means if  N < x < q ( < 2 * N ), then we have
            *         0 <  x - N < N
            **/
            x = &( kP->x );
            if ( bgn_z_cmp( bgnz_md_id, x, eccf2n_order ) >= 0 )
            {
                bgn_z_sub( bgnz_md_id, x, eccf2n_order, r );
            }
            else
            {
                bgn_z_clone( bgnz_md_id, x, r );
            }
        }while( counter < MAX_LOOP_LEN_OF_SIGNATURE && EC_TRUE == bgn_z_is_zero( bgnz_md_id, r ) );

        /* here h, r, k, private_key < N */
        /* s = ( h + privatekey * r ) / k mod N */
        bgn_zn_inv( bgnzn_md_id, k, s );                 /* s = k^-1 */
        bgn_zn_mul( bgnzn_md_id, privatekey, r, tmp );   /* tmp = privatekey * r */
        bgn_zn_add( bgnzn_md_id, tmp, hash, tmp );       /* tmp = tmp + h */
        bgn_zn_mul( bgnzn_md_id, s, tmp, s );            /* s = s * tmp */
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, hash, LOC_ECCF2N_0051);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, k, LOC_ECCF2N_0052);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, tmp, LOC_ECCF2N_0053);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, kP, LOC_ECCF2N_0054);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, t_q, LOC_ECCF2N_0055);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, t_r, LOC_ECCF2N_0056);
#endif/* STATIC_MEMORY_SWITCH */

    /* if counter exceeds the threshold, then return failure */
    if ( MAX_LOOP_LEN_OF_SIGNATURE <= counter )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_signate: signate times exceed the threshold.\n");
        return ((UINT32)(-1));
    }

    return  0;
}

/**
*
*   verify the ecc signature of the message with the public key
*   return TRUE or FALSE
*
*   Algorithm:
*   Input: message m , public key Q , elliptic curve ec and the base point G
*   Output: TRUE or FALSE
*   0.  let the order of the ec be n
*   1.  compute the hash value h of m
*   2.  compute w = 1/s mod n
*   3.  compute u1 = h * w mod n
*   4.  compute u2 = r * w mod n
*   5.  compute R = u1*G + u2 * Q = (x1,y1)
*   6.  if R is the infinite point, then return FALSE.
*   7.  compute v = x1 mod n
*   8.  if v = r, then return TRUE; else return FALSE.
*
**/
EC_BOOL ecc_f2n_verify(const UINT32 eccf2n_md_id,
                const EC_CURVE_POINT * publickey,
                const UINT8 * message,
                const UINT32 messagelen,
                const ECC_SIGNATURE *signature)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    EC_CURVE_POINT buf_5;
    EC_CURVE_POINT buf_6;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *hash;
    BIGINT *r;
    BIGINT *s;
    BIGINT *c;
    BIGINT *u1;
    BIGINT *u2;
    BIGINT *x;
    EC_CURVE_POINT *tmp_point_1;
    EC_CURVE_POINT *tmp_point_2;

    UINT32 ret;

    ECCF2N_MD *eccf2n_md;
    UINT32 bgnzn_md_id;
    UINT32 ecf2n_md_id;
    UINT32 bgnz_md_id;
    BIGINT *eccf2n_order;
    EC_CURVE_POINT *eccf2n_base_point;
    const UINT32 (*eccf2n_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_verify: message is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == signature )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_verify: signature is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
    if ( NULL_PTR == publickey )
    {
        dbg_log(SEC_0096_ECCF2N, 0)(LOGSTDOUT,"error:ecc_f2n_verify: publickey is null.\n");
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif /* ECDSA_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCF2N_MD <= eccf2n_md_id || 0 == g_eccf2n_md[ eccf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_f2n_verify: eccf2n module #0x%lx not started.\n",
                eccf2n_md_id);
        dbg_exit(MD_ECCF2N, eccf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    hash = &buf_1;
    c = &buf_2;
    u1 = &buf_3;
    u2 = &buf_4;
    tmp_point_1 = &buf_5;
    tmp_point_2 = &buf_6;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &hash, LOC_ECCF2N_0057);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &c, LOC_ECCF2N_0058);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &u1, LOC_ECCF2N_0059);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, &u2, LOC_ECCF2N_0060);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &tmp_point_1, LOC_ECCF2N_0061);
    alloc_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, &tmp_point_2, LOC_ECCF2N_0062);
#endif/* STATIC_MEMORY_SWITCH */

    eccf2n_md = &(g_eccf2n_md[ eccf2n_md_id ]);
    bgnzn_md_id        = eccf2n_md->bgnzn_md_id;
    ecf2n_md_id        = eccf2n_md->ecf2n_md_id;
    bgnz_md_id         = eccf2n_md->bgnz_md_id;
    eccf2n_order       = eccf2n_md->eccf2n_order;
    eccf2n_base_point  = eccf2n_md->eccf2n_base_point;
    eccf2n_do_hash     = eccf2n_md->eccf2n_do_hash;

    r = (BIGINT *)&(signature->r);
    s = (BIGINT *)&(signature->s);

    /* do message digest */
    eccf2n_do_hash(message, messagelen, hash);

    /* if hash > ecc order , then do hash = hash mod order */
    if ( bgn_z_cmp(bgnz_md_id, hash, eccf2n_order) >= 0 )
    {
        bgn_z_div(bgnz_md_id, hash, eccf2n_order, u1, u2);
        bgn_z_clone(bgnz_md_id, u2, hash);
    }

    bgn_zn_inv( bgnzn_md_id, s, c);
    bgn_zn_mul( bgnzn_md_id, hash, c, u1);
    bgn_zn_mul( bgnzn_md_id, r, c, u2);

    ec_f2n_point_mul(ecf2n_md_id, eccf2n_base_point, u1         , tmp_point_1);
    ec_f2n_point_mul(ecf2n_md_id, publickey         , u2         , tmp_point_2);
    ec_f2n_point_add(ecf2n_md_id, tmp_point_1       , tmp_point_2, tmp_point_1);

    if ( EC_TRUE == ec_f2n_point_is_infinit(ecf2n_md_id, tmp_point_1))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, hash, LOC_ECCF2N_0063);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, c, LOC_ECCF2N_0064);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, u1, LOC_ECCF2N_0065);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, u2, LOC_ECCF2N_0066);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp_point_1, LOC_ECCF2N_0067);
        free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp_point_2, LOC_ECCF2N_0068);
#endif/* STATIC_MEMORY_SWITCH */

        return ( EC_FALSE );
    }

    /* adjust x when x > N */
    x = &( tmp_point_1->x );

    /* x = u1 * order + u2, so, x = u2 mod order */
    bgn_z_div(bgnz_md_id, x, eccf2n_order, u1, u2);

    if ( 0 == bgn_z_cmp( bgnz_md_id, u2 , r ) )
    {
        ret = EC_TRUE;
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, hash, LOC_ECCF2N_0069);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, c, LOC_ECCF2N_0070);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, u1, LOC_ECCF2N_0071);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_BIGINT, u2, LOC_ECCF2N_0072);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp_point_1, LOC_ECCF2N_0073);
    free_static_mem(MD_ECCF2N, eccf2n_md_id, MM_CURVE_POINT, tmp_point_2, LOC_ECCF2N_0074);
#endif/* STATIC_MEMORY_SWITCH */

    return ( ret );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

