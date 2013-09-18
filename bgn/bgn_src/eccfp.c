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
#include "bgnfp.h"
#include "ecfp.h"
#include "eccfp.h"
#include "mm.h"

#include "debug.h"

#include "print.h"
#include "sha256.h"



/* set eccfp_p = the prime p of F_p */
static ECCFP_MD g_eccfp_md[ MAX_NUM_OF_ECCFP_MD ];
static EC_BOOL  g_eccfp_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_eccfp_p_buf   [ MAX_NUM_OF_ECCFP_MD ];
static BIGINT g_eccfp_order_buf[ MAX_NUM_OF_ECCFP_MD ];
static ECFP_CURVE  g_eccfp_curve_buf[ MAX_NUM_OF_ECCFP_MD ];
static EC_CURVE_POINT g_eccfp_curve_point_buf[ MAX_NUM_OF_ECCFP_MD ];
#endif/* STACK_MEMORY_SWITCH */

/**
*   for test only
*
*   to query the status of ECCFP Module
*
**/
void ecc_fp_print_module_status(const UINT32 eccfp_md_id, LOG *log)
{
    ECCFP_MD *eccfp_md;
    UINT32 index;

    if ( EC_FALSE == g_eccfp_md_init_flag )
    {

        sys_log(log,"no ECCFP Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_ECCFP_MD; index ++ )
    {
        eccfp_md = &(g_eccfp_md[ index ]);

        if ( 0 < eccfp_md->usedcounter )
        {
            sys_log(log,"ECCFP Module # %ld : %ld refered, refer ECFP Module : %ld, refer BGNFP Module : %ld, refer BGNZN Module : %ld, refer BGNZ Module : %ld\n",
                    index,
                    eccfp_md->usedcounter,
                    eccfp_md->ecfp_md_id ,
                    eccfp_md->bgnfp_md_id,
                    eccfp_md->bgnzn_md_id,
                    eccfp_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed ECCFP module
*
*
**/
UINT32 ecc_fp_free_module_static_mem(const UINT32 eccfp_md_id)
{
    ECCFP_MD    *eccfp_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnfp_md_id;
    UINT32 ecfp_md_id;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_free_module_static_mem: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/
    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    ecfp_md_id  = eccfp_md->ecfp_md_id;
    bgnfp_md_id = eccfp_md->bgnfp_md_id;
    bgnzn_md_id = eccfp_md->bgnzn_md_id;
    bgnz_md_id  = eccfp_md->bgnz_md_id;

    free_module_static_mem(MD_ECCFP, eccfp_md_id);

    ec_fp_free_module_static_mem(ecfp_md_id);
    bgn_fp_free_module_static_mem(bgnfp_md_id);
    bgn_zn_free_module_static_mem(bgnzn_md_id);
    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}
/**
*
* start ECCFP module
*
**/
UINT32 ecc_fp_start( const BIGINT *p,
                    const ECFP_CURVE *curve,
                    const BIGINT *order,
                    const EC_CURVE_POINT *base_point,
                    const UINT32 ( *random_generator)( BIGINT * random),
                    const UINT32 (*do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash))
{
    UINT32     eccfp_md_id;
    BIGINT         *eccfp_p;
    ECFP_CURVE     *eccfp_curve;
    BIGINT         *eccfp_order;
    EC_CURVE_POINT *eccfp_base_point;

    ECCFP_MD    *eccfp_md;
    UINT32   bgnzn_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    UINT32 bgnz_md_id;

    const UINT32 ( *eccfp_random_generator)( BIGINT * random);
    const UINT32 (*eccfp_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == p )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: p is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, ERR_MODULE_ID);
    }
    if ( NULL_PTR == curve )
    {
         sys_log(LOGSTDOUT,"error:ecc_fp_start: curve is NULL_PTR.\n");
         dbg_exit(MD_ECCFP, ERR_MODULE_ID);
    }
    if ( NULL_PTR == order )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: order is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, ERR_MODULE_ID);
    }
    if ( NULL_PTR == base_point )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: base_point is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNZMOD module, then */
    /* initialize g_eccfp_md */
    if ( EC_FALSE ==  g_eccfp_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_ECCFP_MD; index ++ )
        {
            eccfp_md = &(g_eccfp_md[ index ]);

            eccfp_md->usedcounter       = 0;
            eccfp_md->bgnzn_md_id       = ERR_MODULE_ID;
            eccfp_md->bgnfp_md_id       = ERR_MODULE_ID;
            eccfp_md->ecfp_md_id        = ERR_MODULE_ID;
            eccfp_md->bgnz_md_id        = ERR_MODULE_ID;
            eccfp_md->eccfp_p            = NULL_PTR;
            eccfp_md->eccfp_curve        = NULL_PTR;
            eccfp_md->eccfp_order        = NULL_PTR;
            eccfp_md->eccfp_base_point   = NULL_PTR;
            eccfp_md->eccfp_random_generator = NULL_PTR;
            eccfp_md->eccfp_do_hash = NULL_PTR;
        }

        /*register all functions of ECCFP module to DBG module*/
        //dbg_register_func_addr_list(g_eccfp_func_addr_list, g_eccfp_func_addr_list_len);

        g_eccfp_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_ECCFP_MD; index ++ )
    {
        eccfp_md_id = index;
        eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);

        if ( 0 == eccfp_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_ECCFP_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnzn_md_id        = ERR_MODULE_ID;
    bgnfp_md_id        = ERR_MODULE_ID;
    ecfp_md_id         = ERR_MODULE_ID;
    bgnz_md_id         = ERR_MODULE_ID;
    eccfp_p            = NULL_PTR;
    eccfp_curve        = NULL_PTR;
    eccfp_order        = NULL_PTR;
    eccfp_base_point   = NULL_PTR;
    eccfp_random_generator = NULL_PTR;
    eccfp_do_hash = NULL_PTR;

    eccfp_md_id = index;
    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT,         &eccfp_p           , LOC_ECCFP_0001);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_ECFP_CURVE ,    &eccfp_curve       , LOC_ECCFP_0002);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT,         &eccfp_order       , LOC_ECCFP_0003);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT,    &eccfp_base_point  , LOC_ECCFP_0004);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    eccfp_p            = &(g_eccfp_p_buf[ eccfp_md_id ]);
    eccfp_curve        = &(g_eccfp_curve_buf[ eccfp_md_id ]);
    eccfp_order        = &(g_eccfp_order_buf[ eccfp_md_id ]);
    eccfp_base_point   = &(g_eccfp_curve_point_buf[ eccfp_md_id ]);
#endif/* STACK_MEMORY_SWITCH */


#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == eccfp_p )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: ec_fp_f is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == eccfp_curve )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: ec_fp_curve is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == eccfp_order )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: ec_fp_order is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == eccfp_base_point )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_start: ec_fp_base_point is NULL_PTR.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* start ECFP module */
    ecfp_md_id = ec_fp_start( p, curve, order, base_point );

    /* start BGNFP module */
    bgnfp_md_id = bgn_fp_start( p );

    /* start BGNZN module */
    bgnzn_md_id = bgn_zn_start( order );

    /* start BGNZ module */
    bgnz_md_id = bgn_z_start();

    /* set eccfp_p = p */
    bgn_fp_clone(bgnfp_md_id, p, eccfp_p);

    /* set eccfp_curve = curve */
    bgn_fp_clone(bgnfp_md_id, &(curve->a), &(eccfp_curve->a));
    bgn_fp_clone(bgnfp_md_id, &(curve->b), &(eccfp_curve->b));

    /* set eccfp_order = order */
    bgn_z_clone(bgnz_md_id, order, eccfp_order);

    /* set eccfp_base_point = base_point */
    bgn_fp_clone(bgnfp_md_id, &(base_point->x), &(eccfp_base_point->x));
    bgn_fp_clone(bgnfp_md_id, &(base_point->y), &(eccfp_base_point->y));

    /* set user defined interface of random generator */
    if ( NULL_PTR ==  random_generator)
    {
        eccfp_random_generator = ecc_fp_random_generator_default;
    }
    else
    {
        eccfp_random_generator = random_generator;
    }

    /* set user defined interface of hash function */
    if ( NULL_PTR == do_hash )
    {
        eccfp_do_hash = ecc_fp_do_hash_default;
    }
    else
    {
        eccfp_do_hash = do_hash;
    }

    /* set module : */
    eccfp_md->usedcounter        = 0;
    eccfp_md->bgnzn_md_id        = bgnzn_md_id;
    eccfp_md->bgnfp_md_id        = bgnfp_md_id;
    eccfp_md->ecfp_md_id         = ecfp_md_id;
    eccfp_md->bgnz_md_id         = bgnz_md_id;
    eccfp_md->eccfp_p            = eccfp_p;
    eccfp_md->eccfp_curve        = eccfp_curve;
    eccfp_md->eccfp_order        = eccfp_order;
    eccfp_md->eccfp_base_point   = eccfp_base_point;
    eccfp_md->eccfp_random_generator   = eccfp_random_generator;
    eccfp_md->eccfp_do_hash            = eccfp_do_hash;

    /* at the first time, set the counter to 1 */
    eccfp_md->usedcounter = 1;

    return ( eccfp_md_id );
}

/**
*
* end ECCFP module
*
**/
void ecc_fp_end(const UINT32 eccfp_md_id)
{
    BIGINT         *eccfp_p;
    ECFP_CURVE     *eccfp_curve;
    BIGINT         *eccfp_order;
    EC_CURVE_POINT *eccfp_base_point;

    ECCFP_MD    *eccfp_md;
    UINT32   bgnzn_md_id;
    UINT32  bgnfp_md_id;
    UINT32   ecfp_md_id;
    UINT32   bgnz_md_id;

    if ( MAX_NUM_OF_ECCFP_MD < eccfp_md_id )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_end: eccfp_md_id = %ld is overflow.\n",eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < eccfp_md->usedcounter )
    {
        eccfp_md->usedcounter --;
        return ;
    }

    if ( 0 == eccfp_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_end: eccfp_md_id = %ld is not started.\n",eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnzn_md_id        = eccfp_md->bgnzn_md_id;
    bgnfp_md_id        = eccfp_md->bgnfp_md_id;
    ecfp_md_id         = eccfp_md->ecfp_md_id;
    bgnz_md_id         = eccfp_md->bgnz_md_id;
    eccfp_p            = eccfp_md->eccfp_p;
    eccfp_curve        = eccfp_md->eccfp_curve;
    eccfp_order        = eccfp_md->eccfp_order;
    eccfp_base_point   = eccfp_md->eccfp_base_point;

    bgn_zn_end( bgnzn_md_id );
    bgn_fp_end( bgnfp_md_id );
    ec_fp_end( ecfp_md_id );
    bgn_z_end( bgnz_md_id );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT,          eccfp_p            , LOC_ECCFP_0005);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_ECFP_CURVE ,     eccfp_curve        , LOC_ECCFP_0006);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT,          eccfp_order        , LOC_ECCFP_0007);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT,     eccfp_base_point   , LOC_ECCFP_0008);
#endif/* STATIC_MEMORY_SWITCH */

    /* free module : */
    //ecc_fp_free_module_static_mem(eccfp_md_id);
    eccfp_md->bgnzn_md_id        = ERR_MODULE_ID;
    eccfp_md->bgnfp_md_id        = ERR_MODULE_ID;
    eccfp_md->ecfp_md_id         = ERR_MODULE_ID;
    eccfp_md->bgnz_md_id         = ERR_MODULE_ID;
    eccfp_md->eccfp_p            = NULL_PTR;
    eccfp_md->eccfp_curve        = NULL_PTR;
    eccfp_md->eccfp_order        = NULL_PTR;
    eccfp_md->eccfp_base_point   = NULL_PTR;
    eccfp_md->eccfp_random_generator = NULL_PTR;
    eccfp_md->eccfp_do_hash = NULL_PTR;
    eccfp_md->usedcounter          = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   compute the public key according to the given private key
*   on the elliptic curve(ec) refered by the Module ID eccfp_md_id
*
*   Note:
*       public key = private key * basepoint
*   where basepoint is on the elliptic curve(ec)
*   and refered by the Module ID eccfp_md_id
*
**/
UINT32 ecc_fp_get_public_key(const UINT32 eccfp_md_id, const BIGINT *privatekey,EC_CURVE_POINT *publickey)
{
    ECCFP_MD *eccfp_md;
    UINT32 bgnfp_md_id;
    UINT32 ecfp_md_id;
    BIGINT *eccfp_order;
    EC_CURVE_POINT *eccfp_base_point;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey)
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_get_public_key: privatekey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == publickey)
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_get_public_key: publickey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_get_public_key: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    bgnfp_md_id = eccfp_md->bgnfp_md_id;
    ecfp_md_id  = eccfp_md->ecfp_md_id;
    eccfp_order = eccfp_md->eccfp_order;
    eccfp_base_point = eccfp_md->eccfp_base_point;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )

    if( bgn_fp_cmp(bgnfp_md_id, privatekey, eccfp_order) >= 0 )
    {  /* >=N; error */
        sys_log(LOGSTDOUT,"error:ecc_fp_get_public_key: private key > ec order.\n");

        sys_log(LOGSTDOUT,"private key:\n");
        print_bigint(LOGSTDOUT,privatekey);

        sys_log(LOGSTDOUT,"ec order:\n");
        print_bigint(LOGSTDOUT,eccfp_order);
        return ((UINT32)(-1));
    }
    else if( EC_TRUE == bgn_fp_is_zero(bgnfp_md_id, privatekey) )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_get_public_key: private key = 0 \n");
        return ((UINT32)(-1));
    }
#endif /* ECC_DEBUG_SWITCH */

    ec_fp_point_mul(ecfp_md_id, eccfp_base_point, privatekey, publickey);

    return ( 0 );
}

/**
*
*   default random generator function to generate a random
*
**/
const UINT32 ecc_fp_random_generator_default( BIGINT * rnd_num)
{
    UINT32 index;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == rnd_num)
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_random_generator_default: random is null.\n");
        return ((UINT32)(-1));
    }
#endif /* ECC_DEBUG_SWITCH */


    for ( index = 0; index < INTMAX; index ++ )
    {
        rnd_num->data[ index ] = /*0xAAAAAAAA*/random();
    }
    rnd_num->len = INTMAX;

    return ( 0 );
}

/**
*
*   call the random generator function to generate a random as the privatekey
*
*   note:
*       the bigint privatekey should be less than the order of the ec
*   i.e,
*       privatekey < eccfp_order
*
**/
void ecc_fp_rnd_private_key(const UINT32 eccfp_md_id, BIGINT * privatekey)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    ECCFP_MD *eccfp_md;
    UINT32   bgnz_md_id;
    BIGINT *eccfp_order;
    BIGINT *random;
    BIGINT *q;

    const UINT32 ( *eccfp_random_generator)( BIGINT * random);

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey)
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_rnd_private_key: privatekey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_rnd_private_key: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    random = &buf_1;
    q = &buf_2;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &random, LOC_ECCFP_0009);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &q, LOC_ECCFP_0010);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    bgnz_md_id  = eccfp_md->bgnz_md_id;
    eccfp_order = eccfp_md->eccfp_order;
    eccfp_random_generator = eccfp_md->eccfp_random_generator;

    /* generator a random */
    eccfp_random_generator( random );

    /* adjust the random to get a random privatekey which must be less than the random*/
    if ( 0 <= bgn_z_cmp( bgnz_md_id, random, eccfp_order ) )
    {
        bgn_z_div(bgnz_md_id, random, eccfp_order, q, privatekey );
    }
    else
    {
        bgn_z_clone(bgnz_md_id, random, privatekey);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, random, LOC_ECCFP_0011);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, q, LOC_ECCFP_0012);
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
UINT32 ecc_fp_generate_keypair(const UINT32 eccfp_md_id, ECC_KEYPAIR *keypair)
{
    BIGINT *privatekey;
    EC_CURVE_POINT *publickey;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == keypair)
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_generate_keypair: keypair is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_generate_keypair: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    privatekey = &(keypair->private_key);
    publickey  = &(keypair->public_key);

    ecc_fp_rnd_private_key(eccfp_md_id, privatekey);
    ecc_fp_get_public_key(eccfp_md_id, privatekey, publickey);

    return ( 0 );
}

/**
*
*   the message (plaintxt) length is not more than (nbits(p) - nlossbits).
*   where nlossbits is the number of loss bits.
*   let m = message,then
*   consider {k * m + i, i = 0... (k-1)}
*   where k = 2 ^{nlossbits}
*   select the minmum from the set s.t.
*   when regard it as the x-coordinate of a point, the point is on the ec
*   which is defined and refered by the module id eccfp_md_id,i.e.,
*       y^2 = x^3 + ax + b over F_p
*
*   return this point
*
*   note:
*       here let k = 32 which means we have to loss 5 bits of the message
*   relative to the length of the x-coordinate of the ec point.
*
**/
int ecc_fp_encoding(const UINT32 eccfp_md_id, const UINT8 * message,const UINT32 messagelen, EC_CURVE_POINT * msgpoint)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *msg;
    BIGINT *tmp;
    BIGINT *x;
    BIGINT *y;
    BIGINT *z;

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

    ECCFP_MD *eccfp_md;
    UINT32 ecfp_md_id;
    UINT32 bgnfp_md_id;
    UINT32   bgnz_md_id;
    BIGINT *eccfp_p;
    UINT32  nbits_of_p;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encoding: message is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encoding: msgpoint is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_encoding: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    msg = &buf_1;
    tmp = &buf_2;
    z   = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &msg, LOC_ECCFP_0013);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &tmp, LOC_ECCFP_0014);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &z, LOC_ECCFP_0015);
#endif/* STATIC_MEMORY_SWITCH */


    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    ecfp_md_id  = eccfp_md->ecfp_md_id;
    bgnfp_md_id = eccfp_md->bgnfp_md_id;
    bgnz_md_id  = eccfp_md->bgnz_md_id;
    eccfp_p = eccfp_md->eccfp_p;

    x = &(msgpoint->x);
    y = &(msgpoint->y);

    /* note: if msg is 192 bits, then nlossbits <= 5 */
    /* number of loss bits*/
    nlossbits = 5;

    /* how many bytes per word */
    nbytes_per_word = WORDSIZE / BYTESIZE;

    nbits_of_p = bgn_z_get_nbits( bgnz_md_id, eccfp_p );

    /* how many bytes will be acceptable in the msg */
    nbytes_per_msg = ( ( nbits_of_p - nlossbits ) / BYTESIZE );

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
        ec_fp_point_set_infinit(ecfp_md_id, msgpoint);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, msg, LOC_ECCFP_0016);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, tmp, LOC_ECCFP_0017);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, z, LOC_ECCFP_0018);
#endif/* STATIC_MEMORY_SWITCH */

        return ( 0 );
    }

    /*k * m*/
    bgn_z_shl_lesswordsize(bgnz_md_id, msg, nlossbits, tmp);

    j = 1;
    j_max = (1 << nlossbits);

    /*k * m + 1*/
    tmp->data[0] ++;
    while(  j < j_max && ( EC_FALSE == ec_fp_x_is_on_curve(ecfp_md_id, tmp, z)))
    {
        j++;

        /*k * m + j*/
        tmp->data[ 0 ]++;
    }

    if( j_max == j )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encoding: encoding failure!");

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, msg, LOC_ECCFP_0019);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, tmp, LOC_ECCFP_0020);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, z, LOC_ECCFP_0021);
#endif/* STATIC_MEMORY_SWITCH */

        return (-1);
    }

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    sys_log(LOGSTDOUT,"ecc_fp_encoding: after %d times searching before encoding success!\n",j);
#endif /* ENC_DEC_DEBUG_SWITCH */

    /* msgpoint's x */
    bgn_fp_clone(bgnfp_md_id, tmp, x);

    /* msgpoint's y = one squroot of y^2 = z mod p */
    bgn_fp_squroot(bgnfp_md_id, z, y);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, msg, LOC_ECCFP_0022);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, tmp, LOC_ECCFP_0023);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, z, LOC_ECCFP_0024);
#endif/* STATIC_MEMORY_SWITCH */

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
*   the number of higher bits is determined by the number of bits of the prime p of F_p
*
*   return the message ( plaintxt )
*
**/
void ecc_fp_decoding(const UINT32 eccfp_md_id, const EC_CURVE_POINT * msgpoint, const UINT32 maxmessagelen,UINT8 *message, UINT32 *messagelen )
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

    ECCFP_MD *eccfp_md;
    UINT32 ecfp_md_id;
    UINT32 bgnfp_md_id;
    UINT32   bgnz_md_id;
    BIGINT *eccfp_p;
    UINT32  nbits_of_p;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decoding: message is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decoding: msgpoint is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_decoding: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    msg = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &msg, LOC_ECCFP_0025);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    ecfp_md_id  = eccfp_md->ecfp_md_id;
    bgnfp_md_id = eccfp_md->bgnfp_md_id;
    bgnz_md_id  = eccfp_md->bgnz_md_id;
    eccfp_p = eccfp_md->eccfp_p;

    x = (BIGINT *)&(msgpoint->x);

    /* note: if message is 192 bits, then nlossbits <= 5 */
    nlossbits = 5;/* number of loss bits*/

    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, msgpoint))
    {
        bgn_z_set_zero(bgnz_md_id, msg);
    }
    else
    {
        bgn_z_shr_lesswordsize(bgnz_md_id, x, nlossbits, msg);
    }

    /* how many bytes per word */
    nbytes_per_word = WORDSIZE / BYTESIZE;

    nbits_of_p = bgn_z_get_nbits( bgnz_md_id, eccfp_p );

    /* how many bytes will be acceptable in the msg */
    nbytes_per_msg = ( ( nbits_of_p - nlossbits ) / BYTESIZE );

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
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, msg, LOC_ECCFP_0026);
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
void ecc_fp_encryption(const UINT32 eccfp_md_id,
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

    ECCFP_MD *eccfp_md;
    UINT32 ecfp_md_id;
    EC_CURVE_POINT *eccfp_base_point;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == publickey )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encryption: publickey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encryption: msgpoint is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encryption: c1 is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == c2 )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_encryption: c2 is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_encryption: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf_1;
    k = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, &tmp, LOC_ECCFP_0027);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &k, LOC_ECCFP_0028);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    ecfp_md_id = eccfp_md->ecfp_md_id;
    eccfp_base_point = eccfp_md->eccfp_base_point;

    ecc_fp_rnd_private_key(eccfp_md_id, k);

    /* c1 = kG */
    ec_fp_point_mul(ecfp_md_id, eccfp_base_point, k, c1);

    /* c2 = m + kP */
    ec_fp_point_mul(ecfp_md_id, publickey, k, tmp);
    ec_fp_point_add(ecfp_md_id, msgpoint, tmp, c2);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp, LOC_ECCFP_0029);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, k, LOC_ECCFP_0030);
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
void ecc_fp_decryption(const UINT32 eccfp_md_id,
                        const BIGINT * privatekey,
                        const EC_CURVE_POINT * c1,
                        const EC_CURVE_POINT * c2,
                        EC_CURVE_POINT * msgpoint)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    EC_CURVE_POINT buf;
#endif/* STACK_MEMORY_SWITCH */
    EC_CURVE_POINT *tmp;

    ECCFP_MD *eccfp_md;
    UINT32 ecfp_md_id;

#if ( SWITCH_ON == ENC_DEC_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decryption: privatekey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == msgpoint )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decryption: msgpoint is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decryption: c1 is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == c2 )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_decryption: c2 is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ENC_DEC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_decryption: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, &tmp, LOC_ECCFP_0031);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    ecfp_md_id = eccfp_md->ecfp_md_id;

    /* m = c2 - k * c1 */
    ec_fp_point_mul(ecfp_md_id, c1, privatekey, tmp);
    ec_fp_point_sub(ecfp_md_id, c2, tmp, msgpoint);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp, LOC_ECCFP_0032);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
*   return a constant....
*
**/
const UINT32 ecc_fp_do_hash_default0(const UINT8 *message,const UINT32 messagelen, BIGINT *hash)
{
    UINT32 index;
#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_do_hash_default: message is null.\n");
        return ((UINT32)(-1));
    }
    if ( NULL_PTR == hash )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_do_hash_default: hash is null.\n");
        return ((UINT32)(-1));
    }
#endif /* ECDSA_DEBUG_SWITCH */

    sys_log(LOGSTDOUT,"\n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: --------------------------------------------\n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: this function is only a debug  interface.   \n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: so, it always returns the constant value.   \n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: do not worry since it will be replaced with \n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: user's defined interface.                   \n");
    sys_log(LOGSTDOUT,"ecc_fp_do_hash_default: --------------------------------------------\n");
    sys_log(LOGSTDOUT,"\n");

    for ( index = 0; index < INTMAX; index ++ )
    {
        hash->data[ index ] = 0xaaaaaaaa;
    }

    hash->len = INTMAX;

    return ( 0 );
}



const UINT32 ecc_fp_do_hash_default(const UINT8 *message,const UINT32 messagelen, BIGINT *hash)
{
    UINT32 seg_idx;
    UINT32 seg_num;

    UINT32 char_idx;
    UINT32 char_num;
    UINT8  sha256sum[32];
    BIGINT tmp;

    UINT32 bgnz_md_id;
#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_do_hash_default: message is null.\n");
        return ((UINT32)(-1));
    }
    if ( NULL_PTR == hash )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_do_hash_default: hash is null.\n");
        return ((UINT32)(-1));
    }
#endif /* ECDSA_DEBUG_SWITCH */

    bgnz_md_id = bgn_z_start();

    bgn_z_set_zero( bgnz_md_id, hash);

    seg_num = (messagelen / 32);
    for(seg_idx = 0; seg_idx < seg_num; seg_idx ++)
    {
        UINT32 char_idx;

        bgn_z_set_zero( bgnz_md_id, &tmp);
        for(char_idx = 0; char_idx < 32; char_idx ++)
        {
            sha256sum[ char_idx ]  = message[ seg_idx * 32 + char_idx ];
        }

        sha256_to_bigint(sha256sum, &tmp);

        bgn_z_add(bgnz_md_id, hash, &tmp, hash);
    }

    char_num = (messagelen % 32);
    if(0 < char_num)
    {
        bgn_z_set_zero( bgnz_md_id, &tmp);
        for(char_idx = 0; char_idx < char_num; char_idx ++)
        {
            sha256sum[ char_idx ]  = message[ seg_idx * 32 + char_idx ];
        }
        for(; char_idx < 32; char_idx ++)
        {
            sha256sum[ char_idx ] = 0;
        }

        sha256_to_bigint(sha256sum, &tmp);

        bgn_z_add(bgnz_md_id, hash, &tmp, hash);
    }

    bgn_z_end(bgnz_md_id);

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
UINT32  ecc_fp_signate(const UINT32 eccfp_md_id,
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

    ECCFP_MD *eccfp_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnfp_md_id;
    UINT32  ecfp_md_id;
    UINT32  bgnz_md_id;
    BIGINT *eccfp_order;
    EC_CURVE_POINT *eccfp_base_point;
    const UINT32 ( *eccfp_random_generator)( BIGINT * random);
    const UINT32 (*eccfp_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

    UINT32 counter;

#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == privatekey )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_signate: privatekey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_signate: message is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == signature )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_signate: signature is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ECDSA_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_signate: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
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
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &hash, LOC_ECCFP_0033);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &k, LOC_ECCFP_0034);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &tmp, LOC_ECCFP_0035);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, &kP, LOC_ECCFP_0036);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &t_q, LOC_ECCFP_0037);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &t_r, LOC_ECCFP_0038);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    bgnzn_md_id       = eccfp_md->bgnzn_md_id;
    bgnfp_md_id       = eccfp_md->bgnfp_md_id;
    ecfp_md_id        = eccfp_md->ecfp_md_id;
    bgnz_md_id        = eccfp_md->bgnz_md_id;
    eccfp_order       = eccfp_md->eccfp_order;
    eccfp_base_point  = eccfp_md->eccfp_base_point;
    eccfp_do_hash     = eccfp_md->eccfp_do_hash;
    eccfp_random_generator = eccfp_md->eccfp_random_generator;

    r = &(signature->r);
    s = &(signature->s);

    bgn_z_set_zero( bgnz_md_id, r );
    bgn_z_set_zero( bgnz_md_id, s );

    /* do message digest*/
    eccfp_do_hash( message, messagelen, hash );

    /* if hash > ecc order, then do hash = hash mod order */
    if ( bgn_z_cmp(bgnz_md_id, hash, eccfp_order)  >= 0 )
    {
        bgn_z_div(bgnz_md_id, hash, eccfp_order, t_q, t_r);
        bgn_z_clone(bgnz_md_id, t_r, hash);
    }

    /* counter is to count the loop times. if exceed one threshold, then break and return failure */
    counter = 0;
    while( counter < MAX_LOOP_LEN_OF_SIGNATURE && EC_TRUE == bgn_z_is_zero( bgnz_md_id, s ) )
    {
        do
        {
            /* increase the loop times counter*/
            counter ++;

            /* generate random number k */
            eccfp_random_generator( k );

            /* k = k mod N */
            if ( 0 <= bgn_z_cmp(bgnz_md_id, k, eccfp_order))
            {
                bgn_z_div(bgnz_md_id, k, eccfp_order, t_q, t_r);
                bgn_z_clone(bgnz_md_id, t_r, k);
            }

            ec_fp_point_mul( ecfp_md_id, eccfp_base_point, k, kP );

            /* r = x mod N */
            /*
            * since abs( q + 1 - N ) <= 2 * sqrt( q )
            *  => q - 4 * sqrt( q ) + 2 <= 2 * N - q <= q + 4 * sqrt( q ) + 2
            *  note sqrt( q ) >> 4, we have
            *       q < 2 * N
            *  which means if  N < x < q ( < 2 * N ), then we have
            *         0 <  x - N < N
            *
            **/
            x = &( kP->x );
            if ( bgn_z_cmp( bgnz_md_id, x, eccfp_order ) >= 0 )
            {
                bgn_z_sub( bgnz_md_id, x, eccfp_order, r );
            }
            else
            {
                bgn_z_clone( bgnz_md_id, x, r );
            }
        }while(counter < MAX_LOOP_LEN_OF_SIGNATURE &&  EC_TRUE == bgn_z_is_zero( bgnz_md_id, r ) );

        /* here h, r, k, private_key < N */
        /* s = ( h + privatekey * r ) / k mod N */
        bgn_zn_inv( bgnzn_md_id, k, s );                 /* s = k^-1 */
        bgn_zn_mul( bgnzn_md_id, privatekey, r, tmp );   /* tmp = privatekey * r */
        bgn_zn_add( bgnzn_md_id, tmp, hash, tmp );       /* tmp = tmp + h */
        bgn_zn_mul( bgnzn_md_id, s, tmp, s );            /* s = s * tmp */
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, hash, LOC_ECCFP_0039);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, k, LOC_ECCFP_0040);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, tmp, LOC_ECCFP_0041);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, kP, LOC_ECCFP_0042);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, t_q, LOC_ECCFP_0043);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, t_r, LOC_ECCFP_0044);
#endif/* STATIC_MEMORY_SWITCH */

    /* if counter exceeds the threshold, then return failure */
    if ( MAX_LOOP_LEN_OF_SIGNATURE <= counter )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_signate: signate times exceed the threshold.\n");
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
EC_BOOL ecc_fp_verify(const UINT32 eccfp_md_id,
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

    ECCFP_MD *eccfp_md;
    UINT32 bgnzn_md_id;
    UINT32  ecfp_md_id;
    UINT32  bgnz_md_id;
    BIGINT *eccfp_order;
    EC_CURVE_POINT *eccfp_base_point;
    const UINT32 (*eccfp_do_hash)(const UINT8 *message,const UINT32 messagelen, BIGINT *hash);

#if ( SWITCH_ON == ECDSA_DEBUG_SWITCH )
    if ( NULL_PTR == message )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_verify: message is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == signature )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_verify: signature is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
    if ( NULL_PTR == publickey )
    {
        sys_log(LOGSTDOUT,"error:ecc_fp_verify: publickey is null.\n");
        dbg_exit(MD_ECCFP, eccfp_md_id);
    }
#endif /* ECDSA_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_ECCFP_MD <= eccfp_md_id || 0 == g_eccfp_md[ eccfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ecc_fp_verify: eccfp module #0x%lx not started.\n",
                eccfp_md_id);
        dbg_exit(MD_ECCFP, eccfp_md_id);
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
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &hash, LOC_ECCFP_0045);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &c, LOC_ECCFP_0046);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &u1, LOC_ECCFP_0047);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, &u2, LOC_ECCFP_0048);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, &tmp_point_1, LOC_ECCFP_0049);
    alloc_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, &tmp_point_2, LOC_ECCFP_0050);
#endif/* STATIC_MEMORY_SWITCH */

    eccfp_md = &(g_eccfp_md[ eccfp_md_id ]);
    bgnzn_md_id       = eccfp_md->bgnzn_md_id;
    ecfp_md_id        = eccfp_md->ecfp_md_id;
    bgnz_md_id        = eccfp_md->bgnz_md_id;
    eccfp_order       = eccfp_md->eccfp_order;
    eccfp_base_point  = eccfp_md->eccfp_base_point;
    eccfp_do_hash     = eccfp_md->eccfp_do_hash;

    r = (BIGINT *)&(signature->r);
    s = (BIGINT *)&(signature->s);

    /* do message digest */
    eccfp_do_hash(message, messagelen, hash);

    /* if hash > ecc order , then do hash = hash mod order */
    if ( bgn_z_cmp(bgnz_md_id, hash, eccfp_order) >= 0 )
    {
        bgn_z_div(bgnz_md_id, hash, eccfp_order, u1, u2);
        bgn_z_clone(bgnz_md_id, u2, hash);
    }

    bgn_zn_inv( bgnzn_md_id, s, c);
    bgn_zn_mul( bgnzn_md_id, hash, c, u1);
    bgn_zn_mul( bgnzn_md_id, r, c, u2);

    ec_fp_point_mul(ecfp_md_id, eccfp_base_point  , u1         , tmp_point_1);
    ec_fp_point_mul(ecfp_md_id, publickey         , u2         , tmp_point_2);
    ec_fp_point_add(ecfp_md_id, tmp_point_1       , tmp_point_2, tmp_point_1);

    if ( EC_TRUE == ec_fp_point_is_infinit(ecfp_md_id, tmp_point_1))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, hash, LOC_ECCFP_0051);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, c, LOC_ECCFP_0052);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, u1, LOC_ECCFP_0053);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, u2, LOC_ECCFP_0054);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp_point_1, LOC_ECCFP_0055);
        free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp_point_2, LOC_ECCFP_0056);
#endif/* STATIC_MEMORY_SWITCH */

        return ( EC_FALSE );
    }

    /* adjust x when x > N */
    x = &( tmp_point_1->x );

    /* x = u1 * order + u2, so, x = u2 mod order */
    bgn_z_div(bgnz_md_id, x, eccfp_order, u1, u2);

    if ( 0 == bgn_z_cmp( bgnz_md_id, u2 , r ) )
    {
        ret = EC_TRUE;
    }
    else
    {
        ret = EC_FALSE;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, hash, LOC_ECCFP_0057);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, c, LOC_ECCFP_0058);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, u1, LOC_ECCFP_0059);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_BIGINT, u2, LOC_ECCFP_0060);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp_point_1, LOC_ECCFP_0061);
    free_static_mem(MD_ECCFP, eccfp_md_id, MM_CURVE_POINT, tmp_point_2, LOC_ECCFP_0062);
#endif/* STATIC_MEMORY_SWITCH */

    return ( ret );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

