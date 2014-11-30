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

#include "mm.h"

#include "bgnz.h"
#include "bgnzn.h"
#include "bgnfp.h"

#include "debug.h"

#include "print.h"

#include "log.h"

static BGNFP_MD g_bgnfp_md[ MAX_NUM_OF_BGNFP_MD ];
static EC_BOOL  g_bgnfp_md_init_flag = EC_FALSE;

static int g_bgnzn_legendre_table[ 8 ]={0,1,0,-1,0,-1,0,1};

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_bgnfp_buf[ 2 * MAX_NUM_OF_BGNFP_MD ];
#endif/* STACK_MEMORY_SWITCH */

static EC_BOOL bgn_fp_squroot_common(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c);

/**
*   for test only
*
*   to query the status of BGNFP Module
*
**/
void bgn_fp_print_module_status(const UINT32 bgnfp_md_id, LOG *log)
{
    BGNFP_MD *bgnfp_md;
    UINT32 index;

    if ( EC_FALSE == g_bgnfp_md_init_flag )
    {

        sys_log(log,"no BGNFP Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_BGNFP_MD; index ++ )
    {
        bgnfp_md = &(g_bgnfp_md[ index ]);
        /* if module was started, check if it's the ring Z_{n} */

        if ( 0 < bgnfp_md->usedcounter)
        {
            sys_log(log,"BGNFP Module # %ld : %ld refered, refer BGNZN Module : %ld, refer BGNZ Module : %ld\n",
                    index,
                    bgnfp_md->usedcounter,
                    bgnfp_md->bgnzn_md_id,
                    bgnfp_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed BGNFP module
*
*
**/
UINT32 bgn_fp_free_module_static_mem(const UINT32 bgnfp_md_id)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_free_module_static_mem: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;
    bgnz_md_id  = bgnfp_md->bgnz_md_id;

    free_module_static_mem(MD_BGNFP, bgnfp_md_id);

    bgn_zn_free_module_static_mem(bgnzn_md_id);
    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}

/**
*
* start BGNFP module
*
**/
UINT32 bgn_fp_start( const BIGINT *p )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */
    BGNFP_MD    *bgnfp_md;
    UINT32 bgnfp_md_id;
    UINT32 bgnzn_md_id;
    UINT32  bgnz_md_id;

    BIGINT *bgnfp_p;
    BIGINT *bgnfp_res;

    BIGINT *t;

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == p )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_start: p is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNFP module, then */
    /* initialize g_bgnfp_md */
    if ( EC_FALSE ==  g_bgnfp_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_BGNFP_MD; index ++ )
        {
            bgnfp_md = &(g_bgnfp_md[ index ]);

            bgnfp_md->usedcounter   = 0;

            bgnfp_md->bgnzn_md_id = ERR_MODULE_ID;
            bgnfp_md->bgnz_md_id  = ERR_MODULE_ID;

            bgnfp_md->bgnfp_p   = NULL_PTR;
            bgnfp_md->bgnfp_res = NULL_PTR;
        }

        /*register all functions of BGNFP module to DBG module*/
        //dbg_register_func_addr_list(g_bgnfp_func_addr_list, g_bgnfp_func_addr_list_len);

        g_bgnfp_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_BGNFP_MD; index ++ )
    {
        bgnfp_md = &(g_bgnfp_md[ index ]);

        if ( 0 == bgnfp_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_BGNFP_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnzn_md_id = ERR_MODULE_ID;
    bgnz_md_id  = ERR_MODULE_ID;
    bgnfp_p = NULL_PTR;
    bgnfp_res = NULL_PTR;

    bgnfp_md_id = index;
    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    bgnfp_p   = &(g_bgnfp_buf[ 2 * bgnzn_md_id + 0 ]);
    bgnfp_res = &(g_bgnfp_buf[ 2 * bgnzn_md_id + 1 ]);
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &bgnfp_p  , LOC_BGNFP_0001);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &bgnfp_res, LOC_BGNFP_0002);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == bgnfp_p )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_start: bgnfp_p is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == bgnfp_res )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_start: bgnfp_res is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t = &buf_1;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &t, LOC_BGNFP_0003);
#endif/* STATIC_MEMORY_SWITCH */

    /* start a BGNZN module */
    bgnzn_md_id = bgn_zn_start( p );

    /*start a BGNZ module*/
    bgnz_md_id = bgn_z_start();

    /* now the BGNZN module is already started */
    /* set bgnfp_p = p */
    bgn_zn_clone(bgnzn_md_id, p, bgnfp_p);

    /* set t = 2 ^ (BIGINTSIZE - 1) mod p */
    bgn_zn_set_e(bgnzn_md_id,t, BIGINTSIZE - 1);

    /* set res = 2 ^(BIGINTSIZE) mod p = (t << 1) mod p*/
    bgn_zn_shl_lesswordsize(bgnzn_md_id, t, 1, bgnfp_res);

    /* set module : */
    bgnfp_md->bgnzn_md_id   = bgnzn_md_id;
    bgnfp_md->bgnz_md_id    = bgnz_md_id;
    bgnfp_md->bgnfp_p       = bgnfp_p;
    bgnfp_md->bgnfp_res     = bgnfp_res;

    /* at the first time, set the counter to 1 */
    bgnfp_md->usedcounter = 1;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, t, LOC_BGNFP_0004);
#endif/* STATIC_MEMORY_SWITCH */

    return ( bgnfp_md_id );
}

/**
*
* end BGNFP module
*
**/
void bgn_fp_end(const UINT32 bgnfp_md_id)
{
    BGNFP_MD *bgnfp_md;
    BIGINT *bgnfp_p;
    BIGINT *bgnfp_res;
    UINT32 bgnzn_md_id;
    UINT32  bgnz_md_id;

    if ( MAX_NUM_OF_BGNFP_MD < bgnfp_md_id )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_end: bgnfp_md_id = %ld is overflow.\n",bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < bgnfp_md->usedcounter )
    {
        bgnfp_md->usedcounter --;
        return ;
    }

    if ( 0 == bgnfp_md->usedcounter )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_end: bgnfp_md_id = %ld is not started.\n",bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;
    bgnz_md_id  = bgnfp_md->bgnz_md_id;
    bgnfp_p     = bgnfp_md->bgnfp_p;
    bgnfp_res   = bgnfp_md->bgnfp_res;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, bgnfp_p  , LOC_BGNFP_0005);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, bgnfp_res, LOC_BGNFP_0006);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_zn_end( bgnzn_md_id );
    bgn_z_end( bgnz_md_id);

    /* free module : */
    //bgn_fp_free_module_static_mem(bgnfp_md_id);
    bgnfp_md->bgnzn_md_id   = ERR_MODULE_ID;
    bgnfp_md->bgnz_md_id    = ERR_MODULE_ID;
    bgnfp_md->bgnfp_p       = NULL_PTR;
    bgnfp_md->bgnfp_res     = NULL_PTR;
    bgnfp_md->usedcounter   = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   clone src to des
*   return des where des = src
**/
void bgn_fp_clone(const UINT32 bgnfp_md_id,const BIGINT * src,BIGINT * des)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_clone: src is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_clone: des is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_clone: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_clone(bgnzn_md_id, src, des);
}

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_fp_cmp(const UINT32 bgnfp_md_id,const BIGINT * a,const BIGINT *b)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_cmp: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_cmp: b is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_cmp: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_cmp(bgnzn_md_id, a, b);
}

/**
*
*   set a = 0
**/
void bgn_fp_set_zero(const UINT32 bgnfp_md_id,BIGINT * a)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_zero: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_set_zero: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_set_zero(bgnzn_md_id, a);
}

/**
*
*   set a = 1
**/
void bgn_fp_set_one(const UINT32 bgnfp_md_id,BIGINT * a)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_one: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_set_one: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_set_one(bgnzn_md_id, a);
}

/**
*
*   set a = n
**/
void bgn_fp_set_word(const UINT32 bgnfp_md_id,BIGINT *a,const UINT32 n)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_word: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_set_word: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_set_word(bgnzn_md_id, a, n);
}

/**
*   return e = 2 ^ nth mod p
*            = ( 1 << nth ) mod p
*   where nth = 0,1,...,{BIGINTSIZE - 1}
*
**/
void bgn_fp_set_e(const UINT32 bgnfp_md_id,BIGINT *e,const UINT32 nth)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == e )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_word: e is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( BIGINTSIZE <= nth )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_word: nth = %ld is overflow.\n", nth);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_set_word: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_set_e(bgnzn_md_id, e, nth);
}

/**
*
*   set a =  2^ {BIGINTSIZE} - 1 mod p
**/
void bgn_fp_set_max(const UINT32 bgnfp_md_id,BIGINT * a)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_set_max: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_set_max: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_set_max(bgnzn_md_id, a);

    return ;
}

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_zero(const UINT32 bgnfp_md_id,const BIGINT* src)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_is_zero: src is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_is_zero: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_is_zero(bgnzn_md_id, src);
}

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_one(const UINT32 bgnfp_md_id,const BIGINT* src)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_is_one: src is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_is_one: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_is_one(bgnzn_md_id, src);
}

/**
*
*   if src = n, then return EC_TRUE
*   if src !=n, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_n(const UINT32 bgnfp_md_id,const BIGINT* src, const UINT32 n)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_is_n: src is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_is_n: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_is_n(bgnzn_md_id, src, n);
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_fp_is_odd(const UINT32 bgnfp_md_id,const BIGINT *src)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_is_odd: src is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_is_odd: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_is_odd(bgnzn_md_id, src);
}

/**
*   let a belong to [0, n - 1], then
*       c = a / 2^{nbits} mod p
*   where 0 <= nbits < BIGINTSIZE
*   return c
*
*   maybe address of c = address of a
**/
EC_BOOL bgn_fp_shr_lessbgnsize(const UINT32 bgnfp_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shr_lessbgnsize: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shr_lessbgnsize: c is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_shr_lessbgnsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_shr_lessbgnsize: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_shr_lessbgnsize(bgnzn_md_id, a, nbits, c);
}

/**
*   let a belong to [0, p - 1], then
*       c = ( a << WORDSIZE ) mod p
*   return c
*
**/
void bgn_fp_shl_onewordsize(const UINT32 bgnfp_md_id,const BIGINT * a, BIGINT * c)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shl_onewordsize: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shl_onewordsize: c is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_shl_onewordsize: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_shl_onewordsize(bgnzn_md_id, a, c);
    return;
}

/**
*   let a belong to [0, p - 1], then
*       c = ( a << nbits ) mod p
*   return c
*
**/
void bgn_fp_shl_lesswordsize(const UINT32 bgnfp_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == ECC_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shl_lesswordsize: a is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_shl_lesswordsize: c is null.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_shl_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif /* ECC_DEBUG_SWITCH */
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_shl_lesswordsize: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_shl_lesswordsize(bgnzn_md_id, a, nbits, c);

    return ;
}

/**
**   Let the NAF representative of k be
*       k = SUM ( s_i * 2 ^ i, where s_i belong to {1,0,-1} and i = 0..n )
*   Then return s = [ s_0,...,s_n ] and n
*   i.e,
*       s[ 0 ] = s_0,... s[ n ] = s_n
*
*   Notes:
*   Let k = SUM( k_i * 2 ^ i, where k_i belong to {1,0} and i = 0..m )
*
*   Since s_i belong to {1,0,-1} and s_n is non_zero and s_{n-1} is zero then
*       2 ^ {n - 1} < k <= 2 ^ n
*   Meanwhile, 2 ^ m <= k < 2 ^ {m + 1}, hence
*       n - 1 < m +1 and m <= n
*   i.e.
*       m <= n < m + 2
*   thus,
*       n = m
*   or
*       n = m + 1
**/
int bgn_fp_naf(const UINT32 bgnfp_md_id,const BIGINT *k,int *s)
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_naf: k is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == s )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_naf: s is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_naf: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_naf(bgnzn_md_id, k, s);
}

/**
*       c = ( a + b ) mod p
*       where a < p and b < p
*   Algorithm:
*   Input: a,b where a < p and b < p
*   Output: c such that c = ( a + b ) mod p
*   1.  (carry, c) = a + b
*   2.  if carry > 0, which means p >= 2 ^ (BIGINTSIZE -1) + 1, and now
*           a + b = c + 2 ^(BIGINTSIZE)
*       then do
*           c = c + res
*       where res = 2 ^ (BIGINTSIZE) mod p
*   3.  else if c >= p then do
*           c = c - p
*   4.  return c
**/
void bgn_fp_add(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_add: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_add: b is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_add: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_add: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_add( bgnzn_md_id, a, b, c );
    return ;
}

/**
*       c = ( a - b ) mod p
*       where a < p and b < p
*
*   Algorithm:
*   Input: a,b where a < n and b < n
*   Output: c such that c = ( a - b ) mod n
*   1.  if a >= b, then return c = a - b
*   2.  if a < b, then
*   2.1     c = n - b
*   2.2     c = c + a
*   3.  return c
*
**/
void bgn_fp_sub(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_sub: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_sub( bgnzn_md_id, a, b, c );

    return ;
}

/**
*       c = ( -a ) mod p
*       where a < p
*
**/
void bgn_fp_neg(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_neg: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_neg: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_neg: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_neg(bgnzn_md_id, a, c);

    return ;
}

/**
*       c = ( a * b ) mod p
*       where a < p and b < p
*   Algorithm:
*   Input: a,b where a < p and b < p
*   Output: c such that c = ( a * b ) mod p
*   1.  let a * b = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = p * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_fp_mul(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_mul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_mul: b is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_mul: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_mul: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;
    bgn_zn_mul( bgnzn_md_id, a, b, c );
    return ;
}

/**
*       c = ( a * b ) mod p
*       where a < p and b < p
*   Algorithm:
*   Input: a,b where a < p and b < p
*   Output: c such that c = ( a * b ) mod p
*   1.  let a * b = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = p * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_fp_smul(const UINT32 bgnfp_md_id,const BIGINT *a,const UINT32 b,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_smul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_smul: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_smul: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_smul( bgnzn_md_id, a, b, c );

    return ;
}

/**
*       c = ( a ^ 2 ) mod p
*       where a < p
*   Algorithm:
*   Input: a where a < p
*   Output: c such that c = ( a ^ 2 ) mod p
*   1.  let a ^ 2 = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = p * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_fp_squ(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squ: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squ: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_squ: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_squ( bgnzn_md_id, a, c );
}

/**
*       c = ( a ^ e ) mod p
*       where 0 < a < p and e < 2 ^ WORDSIZE
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e, where 0 < a < p and e < 2 * WORDSIZE
*   Output: c such that c = ( a ^ e ) mod p
*   1.  set c = 1
*   2.  while ( 1 ) do
*   2.1     if e is odd, then do
*               c = c * a mod p
*   2.2     end if
*   2.3     e  = ( e >> 1)
*   2.4     if e is zero, then break while, end if
*   2.5     a = a ^ 2 mod p
*   2.6     next while
*   3.  return c
*
**/
void bgn_fp_sexp(const UINT32 bgnfp_md_id,const BIGINT *a,const UINT32 e,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sexp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sexp: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_sexp: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_sexp( bgnzn_md_id, a, e, c );
}


/**
*       c = ( a ^ e ) mod p
*       where 0 < a < p and e < 2 ^ BIGINTSIZE
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e,where 0 < a < p
*   Output: c such that c = ( a ^ e ) mod p
*   0.  let e = SUM(e_i * 2 ^ i, i = 0..m)
*   1.  set c = 1
*   2.  set i = 0
*   3.  while ( 1 ) do
*   3.1     if e_i is odd, then do
*               c = c * a mod p
*   3.2     end if
*   3.3     i = i + 1
*   3.4     if i = m + 1, then break while, end if
*   3.5     a = a ^ 2 mod p
*   3.6     next while
*   4.  return c
*
**/
void bgn_fp_exp(const UINT32 bgnfp_md_id,const BIGINT *a,const BIGINT *e,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_exp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == e )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_exp: e is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_exp: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_exp: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    bgn_zn_exp( bgnzn_md_id, a, e, c );
}

/**
*
*   if a = 0 , then return EC_FALSE
*   if p > a > 0 and GCD(a,p) > 1, then return EC_FALSE
*   if p > a > 0 and GCD(a,p) = 1, then return EC_TRUE and
*       c = ( 1 / a ) mod p
*   where 0 < a < p
*
*   Algorithm:
*   Input: a where 0 < a < p
*   output: c where c =  a ^ {-1} mod p
*   1. set u = a, v = p,A = 1,C = 0
*   2. while u > 0 do
*   2.1     while u is even do
*   2.1.1       u = u / 2
*   2.1.2       if A is odd then
*                   A = (A + p) / 2
*   2.1.3       else
*                   A = A / 2
*   2.1.4       end if
*   2.1.5       next while
*
*   2.2     while v is even do
*   2.2.1       v = v / 2
*   2.2.2       if C is odd then
*                   C = (C + p) / 2
*   2.2.3       else
*                   C = C / 2
*   2.2.4       end if
*   2.2.5       next while
*
*   2.3     if u >= v then
*   2.3.1       u = u - v
*   2.3.2       if A < C then
*                   A = (A + p - C)
*   2.3.3       else
*                   A = (A - C)
*   2.3.4       end if
*   2.4     else
*   2.4.1       v = v - u
*   2.4.2       if C < A then
*                   C = (C + p - A)
*   2.4.3       else
*                   C = (C - A)
*   2.4.4       end if
*   2.5     end if
*   2.6     next while
*   2.7 set c = C
*   2.8 return c
**/
EC_BOOL bgn_fp_inv(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c )
{
    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_inv: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_inv: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_inv: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

    return bgn_zn_inv( bgnzn_md_id, a, c );
}

/**
*
*   c = a / b mod p
*   if b is 0, then return FALSE
*   else return c = a *(b^(-1)) mod p
*
**/
EC_BOOL bgn_fp_div(const UINT32 bgnfp_md_id,const BIGINT *a, const BIGINT *b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/*STACK_MEMORY_SWITCH*/
    BIGINT *b_inv;

    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_div: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_div: b is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_div: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_div: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    b_inv = &buf_1;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &b_inv, LOC_BGNFP_0007);
#endif /* STATIC_MEMORY_SWITCH */

    if ( EC_FALSE == bgn_fp_inv(bgnfp_md_id, b, b_inv))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0008);
#endif /* STATIC_MEMORY_SWITCH */

        return (EC_FALSE);
    }

    bgn_fp_mul(bgnfp_md_id, a, b_inv, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0009);
#endif /* STATIC_MEMORY_SWITCH */

    return (EC_TRUE);
}

/**
*
*   c = a / b mod p
*   if b is 0, then return FALSE
*   else return c = a *(b^(-1)) mod p
*
**/
EC_BOOL bgn_fp_sdiv(const UINT32 bgnfp_md_id,const UINT32 a, const BIGINT *b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/*STACK_MEMORY_SWITCH*/
    BIGINT *bgn_a;
    BIGINT *b_inv;

    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sdiv: b is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_sdiv: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_sdiv: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    b_inv = &buf_1;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &b_inv, LOC_BGNFP_0010);
#endif /* STATIC_MEMORY_SWITCH */

    /*b_inv = 1/b mod p*/
    if ( EC_FALSE == bgn_fp_inv(bgnfp_md_id, b, b_inv))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0011);
#endif /* STATIC_MEMORY_SWITCH */

        return (EC_FALSE);
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    bgn_a = &buf_2;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &bgn_a, LOC_BGNFP_0012);
#endif /* STATIC_MEMORY_SWITCH */

    /*set bgn_a = a*/
    bgn_fp_set_word(bgnfp_md_id, bgn_a, a);

    bgn_fp_mul(bgnfp_md_id, bgn_a, b_inv, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, bgn_a, LOC_BGNFP_0013);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0014);
#endif /* STATIC_MEMORY_SWITCH */

    return (EC_TRUE);
}

/**
*
*   c = a / b mod p
*   if b is 0, then return FALSE
*   else return c = a *(b^(-1)) mod p
*
**/
EC_BOOL bgn_fp_divs(const UINT32 bgnfp_md_id,const BIGINT *a, const UINT32 b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/*STACK_MEMORY_SWITCH*/
    BIGINT *bgn_b;
    BIGINT *b_inv;

    BGNFP_MD *bgnfp_md;
    UINT32 bgnzn_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_divs: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_divs: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_fp_divs: bgnfp module #0x%lx not started.\n",
                bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnzn_md_id = bgnfp_md->bgnzn_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    bgn_b = &buf_1;
    b_inv = &buf_2;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &bgn_b, LOC_BGNFP_0015);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &b_inv, LOC_BGNFP_0016);
#endif /* STATIC_MEMORY_SWITCH */

    /*set bgn_b = b*/
    bgn_fp_set_word(bgnfp_md_id, bgn_b, b);

    /*b_inv = 1/b mod p*/
    if ( EC_FALSE == bgn_fp_inv(bgnfp_md_id, bgn_b, b_inv))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, bgn_b, LOC_BGNFP_0017);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0018);
#endif /* STATIC_MEMORY_SWITCH */

        return (EC_FALSE);
    }

    bgn_fp_mul(bgnfp_md_id, a, b_inv, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, bgn_b, LOC_BGNFP_0019);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b_inv, LOC_BGNFP_0020);
#endif /* STATIC_MEMORY_SWITCH */

    return (EC_TRUE);
}

/**
*
*   assume p is odd prime and a belong to [ 0.. p -1 ]
*
*   return the legendre symbol (a/p)
*
*   note:
*       Let p be an odd prime. Then it is easy to see that for a given integer a, the congruence
*           x^2 = a mod p
*   can have either no solution(we say in this case that a is a quadratic non-residue mod p),
*   one solution if a = 0 mod p, or two solutions(we then say that a is a quadratic residue mod p).
*       Define the Legendre Symbol (a/p) as
*           (a/p) = -1 if a is a quadratic non-residue
*           (a/p) = 0, if a = 0 mod p
*           (a/p) = 1, if a is a quadratic residue.
*
*   Then we have
*       1. (a*b/p) = (a/p) *(b/p)
*       2. (-1/p) = (-1) ^{(p - 1)/2},(2/p) = (-1)^{(p^2 -1)/8}
*       3. if a and b are odd integers with a > 0 and b > 0, then
*           (a/b) = (b/a) *(-1)^{(a - 1)*(b - 1)/4}
*       4. from the definition, (0/1) = 1 since x^2 = 0 mod 1 have two solutions{0,1}
*       5. if b = b0^2, then (0/b) = 1 since x^2 = 0 mod b have two solutions{0,b0}
*
*   Algorithm:
*   Input: a,p where a belong to [ 0..p - 1 ] and p is odd
*   Output: Legendre symbol (a/p)
*   1.  if a = 0, then return 0.
*   2.  set k = 1 and b = p
*   3.  while a is nonzero do
*   3.1     let a = (2^s) * a0 where a0 is odd
*   3.2     if s is odd, then let a = a0 and k = k * Legendre symbol(2/b) = k *  (-1)^{(b^2 -1)/8}
*           since  Legendre symbol (a/b) = ((2/b)^exp) * (a0/b) where (2/b) belong to{1, -1}
*   3.3     now a is odd, and b is odd
*   3.4     let k = k * (-1)^{(a-1) *(b-1)/4} since
*           Legendre symbol (a/b) = (-1)^{(a-1) *(b-1)/4} * (b/a)
*   3.5     let r = b mod a, then b = a, and a = r
*   3.6     next while
*   4.  if b = 1, then return k
*   5.  if b > 1, then return 0
*
*   Defect:
*       when reach step 4, wich means that (a/p) = k * (0/b)
*   when b is a square,e.g, b = b0^2, then (0/b) = 1, then (a/p) = k,
*   this does not match the algorithm procedure result. maybe it's a defect of this algorithm.
*
**/

int bgn_fp_legendre(const UINT32 bgnfp_md_id,const BIGINT *a)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *ta;
    BIGINT *r;
    BIGINT *b;
    BIGINT *q;
    BIGINT *tmp;

    UINT32 e;
    int k;
    UINT32 exp;
    int ret;

    BGNFP_MD *bgnfp_md;
    BIGINT *bgnfp_p;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_legendre: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_legendre: bgnfp module #0x%lx not started.\n",bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnz_md_id = bgnfp_md->bgnz_md_id;
    bgnfp_p = bgnfp_md->bgnfp_p;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnfp_p) )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_legendre: a >= p.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    /* legendre(0/p) = 0 */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id, a ) )
    {
        return ( 0 );
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
    b  = &buf_2;
    r  = &buf_3;
    q  = &buf_4;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &ta, LOC_BGNFP_0021);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &b, LOC_BGNFP_0022);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &r, LOC_BGNFP_0023);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &q, LOC_BGNFP_0024);
#endif /* STATIC_MEMORY_SWITCH */

    /* set b = p */
    bgn_z_clone(bgnz_md_id, bgnfp_p, b);

    /* set ta = a */
    bgn_z_clone(bgnz_md_id, a, ta);


    k = 1;

    /* for each loop, compute legendre( ta / b ), where b >0 and b is odd */
    while( EC_FALSE == bgn_z_is_zero( bgnz_md_id, ta )  )
    {
        /* now, ta > 0 and b is odd, and b > ta */

        exp = 0;

        /* if ta is even, and let ta = (2^exp)*ta0,then */
        /* legendre(a/b) = (legendre(2/b)^exp) * legendre(ta0/b) */
        /* where legendre(2/b) = (-1) ^{(b^2 - 1) / 8} = g_bgnzn_legendre_table[ b & 7 ] */
        /* so, when exp is even, then legendre(a/b) = legendre(ta0/b) */
        /* and when exp is odd, then legendre(a/b) = (legendre(2/b)) * legendre(ta0/b) */
        while(EC_FALSE == bgn_z_is_odd ( bgnz_md_id, ta ))
        {
            exp ++;
            bgn_z_shr_no_shift_out(bgnz_md_id, ta, 1, ta);
        }

        /* now ta is odd and ta > 0 */

        /* if exp is odd, then then legendre(a/b) = (legendre(2/b)) * legendre(ta0/b)  */
        if( 1 == (exp & 1) )
        {
            e = ( b->data[ 0 ] & 7 );
            k = k * g_bgnzn_legendre_table[ e ];
        }

        /* note the fact: assume a > 0 and b > 0, then                  */
        /*  1. legendre(a/b) = ((-1)^{(a-1) *(b-1)/4}) * legendre(b/a)  */
        /*  2. legendre(a/b) = legendre( ( a mod b )/ b )               */
        /* set k = k * (-1)^{(ta-1) *(b-1)/4} */
        e = ( ta->data[ 0 ] & b->data[ 0 ] & 2 );
        if( 0 < e)
        {
            k = -k;
        }

        /* let r = b mod ta */
        bgn_z_div(bgnz_md_id, b, ta, q, r);

        /* b = ta, ta = r */
        tmp = b;
        b  = ta;
        ta = r;
        r  = tmp;

        /* now b > ta >= 0 and b is odd*/
    }

    /**
    *
    *   Defect:
    *       when ta = 0 and b > 1,then (a/p) = k * (0/b)
    *   when b is a square,e.g, b = b0^2, then (0/b) = 1, then (a/p) = k,
    *   this does not match the algorithm procedure result. maybe it's a defect of this algorithm.
    *
    **/
    if ( EC_TRUE == bgn_z_is_one( bgnz_md_id, b ) )
    {
        ret = k;
    }
    else
    {
        ret = 0;
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, ta, LOC_BGNFP_0025);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b, LOC_BGNFP_0026);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, r, LOC_BGNFP_0027);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, q, LOC_BGNFP_0028);
#endif /* STATIC_MEMORY_SWITCH */

    return ( ret );
}

/**
*
*   common procedure to compute the squroot of x^2 = a mod p
*   where p is odd prime.
*   if a is a quadratic non-residue mod p, then return EC_FALSE.
*
*   let p mod 8 = 1.
*   and Legendre symbol (a/p) = 1, then
*   compute one solution of congruence
*           x^2 = a mod p
*
*   Reference:
*       pls refer GTM138-Algorithm 1.5.1-Page33
*
*   Algorithm:
*   0. let p = 2^e * q with q odd.
*   1. find a number n such that the legendre(n/p) = -1.
*   2. set z = n ^ q mod p.
*   3. set
*       y = z,
*       r = e,
*       x = a ^ {(q-1)/2} mod p,
*       b = a * x^2 mod p
*       x = a * x mod p
*   4. if b = 1 mod p, return x
*   5. otherwise, find the smallest m >=1 such that b ^ {2^m} = 1 mod p
*      note: since (a/p) = 1, m must not be equal to r.
*   6. set
*       t = y ^ {2^(r-m-1)} mod p
*       y = t ^ 2 mod p
*       r = m
*       x = x * t mod p
*       b = b * y mod p
*   7. goto 3.
*
**/
static EC_BOOL bgn_fp_squroot_common(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
    BIGINT buf_6;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *n;
    BIGINT *x;
    BIGINT *b;
    BIGINT *z;
    BIGINT *t;
    BIGINT *q;

    BIGINT *y;

    UINT32  e;
    UINT32  r;
    UINT32  m;
    UINT32  index;

    int legendresymbol;

    BGNFP_MD *bgnfp_md;
    BIGINT *bgnfp_p;
    UINT32 bgnz_md_id;

    EC_BOOL ret;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot_common: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot_common: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot_common: bgnfp module #0x%lx not started.\n",bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnz_md_id = bgnfp_md->bgnz_md_id;
    bgnfp_p = bgnfp_md->bgnfp_p;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    n = &buf_1;
    x = &buf_2;
    b = &buf_3;
    z = &buf_4;
    t = &buf_5;
    q = &buf_6;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &n, LOC_BGNFP_0029);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &x, LOC_BGNFP_0030);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &b, LOC_BGNFP_0031);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &z, LOC_BGNFP_0032);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &t, LOC_BGNFP_0033);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &q, LOC_BGNFP_0034);
#endif /* STATIC_MEMORY_SWITCH */

    /* find n from 1 step 2 such that (n/p) = -1 */
    bgn_z_set_one(bgnz_md_id, n);
    while( 1 )
    {
        legendresymbol = bgn_fp_legendre(bgnfp_md_id, n);
        if ( -1 == legendresymbol )
        {
            break;
        }

        /*set n = n + 2*/
        /*Optimization: we can try small prime one by one*/
        bgn_z_sadd(bgnz_md_id, n, 2, n);
    }

    /*let p - 1 = 2^e * q*/
    bgn_z_ssub(bgnz_md_id, bgnfp_p, 1, t);
    bgn_z_evens(bgnz_md_id, t, &e, q);

    /*compute z = n ^ q mod p*/
    bgn_fp_exp(bgnfp_md_id, n, q, z);

    /*set y = z*/
    y = z;

    /*set r = e*/
    r = e;

    /*set x = a^{(q-1)/2} mod p*/
    /* let t = (q-1)/2 */
    bgn_z_shr_no_shift_out(bgnz_md_id, q, 1, t);

    /* let x = a ^ t mod p */
    bgn_fp_exp(bgnfp_md_id, a, t, x);

    /*set b = a * x ^ 2 mod p */
    /* let t = x ^ 2 mod p*/
    bgn_fp_squ(bgnfp_md_id, x, t);

    /* let b = a * t mod p */
    bgn_fp_mul(bgnfp_md_id, a, t, b);

    /*set x = a * x mod p*/
    bgn_fp_mul(bgnfp_md_id, a, x, x);

    while(1)
    {
        if ( EC_TRUE == bgn_z_is_one(bgnz_md_id, b))
        {
            bgn_z_clone(bgnz_md_id, x, c);

            ret = EC_TRUE;
            break;
        }

        /*find the smallest m >= 1 such that b ^{2^m} = 1 mod p*/
        /*set m = 0*/
        m = 0;

        /*set t = b = b ^{2^m} mod p*/
        bgn_z_clone(bgnz_md_id, b, t);

        for ( m = 1; m < r; m ++ )
        {
            /* t = t^2 = b ^{2^m} mod p*/
            bgn_fp_squ(bgnfp_md_id, t, t);

            if ( EC_TRUE == bgn_z_is_one(bgnz_md_id, t) )
            {
                break;
            }
        }

        /*if m = r, then this means a is not a quadratic residue mod p */
        if ( m == r )
        {
            /*failure, break and return EC_FALSE*/
            ret = EC_FALSE;
            break;
        }

        /* set t = y ^ {2^(r-m-1)} */
        bgn_z_clone(bgnz_md_id, y, t);
        for( index = 1; index <= ( r - m - 1 ); index ++ )
        {
            /*at present, t = y ^{2^(index-1)}, so that t^2 = y ^{2^index}*/
            bgn_fp_squ(bgnfp_md_id, t, t);
        }

        /* set y = t ^ 2 */
        bgn_fp_squ(bgnfp_md_id, t, y);

        /* set r = m  */
        r = m;

        /* set x = x * t */
        bgn_fp_mul(bgnfp_md_id, x, t, x);

        /* set b = b * y */
        bgn_fp_mul(bgnfp_md_id, b, y, b);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, n, LOC_BGNFP_0035);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, x, LOC_BGNFP_0036);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, b, LOC_BGNFP_0037);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, z, LOC_BGNFP_0038);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, t, LOC_BGNFP_0039);
    free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, q, LOC_BGNFP_0040);
#endif /* STATIC_MEMORY_SWITCH */

    return (ret);
}
/**
*
*   let p mod 8 = 1 or 3 or 5 or 7.
*   and Legendre symbol (a/p) = 1, then
*   compute one solution of congruence
*           x^2 = a mod p
*
*   for p mod 4 = 3, return x = a ^{(p+1)/4} mod p
*   for p mod 8 = 5, return x = 2a *(4a)^{(p-5)/8} mod p
*   for p mod 8 = 1, return the squroot common procedure result.
*
**/
EC_BOOL bgn_fp_squroot(const UINT32 bgnfp_md_id,const BIGINT *a,BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *ta;
    BIGINT *exp;
    BIGINT *tmp;

    BGNFP_MD *bgnfp_md;
    BIGINT *bgnfp_p;
    UINT32 bgnz_md_id;

    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot: a is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot: c is NULL_PTR.\n");
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNFP_MD <= bgnfp_md_id || 0 == g_bgnfp_md[ bgnfp_md_id ].usedcounter )
    {
        dbg_log(SEC_0102_BGNFP, 0)(LOGSTDOUT,"error:bgn_fp_squroot: bgnfp module #0x%lx not started.\n",bgnfp_md_id);
        dbg_exit(MD_BGNFP, bgnfp_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnfp_md = &(g_bgnfp_md[ bgnfp_md_id ]);
    bgnz_md_id = bgnfp_md->bgnz_md_id;
    bgnfp_p = bgnfp_md->bgnfp_p;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
    exp = &buf_2;
    tmp = &buf_3;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &ta, LOC_BGNFP_0041);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &exp, LOC_BGNFP_0042);
    alloc_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, &tmp, LOC_BGNFP_0043);
#endif /* STATIC_MEMORY_SWITCH */

    /* if p mod 4 = 3, then x = a ^{(p+1)/4} mod p */
    if ( 0 < bgnfp_p->len && 3 == ( bgnfp_p->data[ 0 ] & 3 ) )
    {
        /* exp = (p+1)/4*/
        bgn_z_set_word(bgnz_md_id, tmp, 1);
        carry = bgn_z_add(bgnz_md_id, bgnfp_p, tmp, exp);
        if ( 0 < carry )
        {
            /* if carry > 0, i.e. carry = 1, then */
            /* it means p = 2^{BIGINTSIZE} - 1 and p is prime */
            /* so that, (p + 1)/4 = 2 ^{BIGINTSIZE - 2}*/
            bgn_z_set_e(bgnz_md_id, exp, BIGINTSIZE - 2);
        }
        else
        {
            bgn_z_shr_lesswordsize(bgnz_md_id, exp, 2, exp);
        }

        /* x = a ^ exp mod p */
        bgn_fp_exp(bgnfp_md_id, a, exp, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, ta, LOC_BGNFP_0044);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, exp, LOC_BGNFP_0045);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, tmp, LOC_BGNFP_0046);
#endif /* STATIC_MEMORY_SWITCH */

        return EC_TRUE;
    }

    /* if p mod 8 = 5, then x = 2a *(4a)^{(p-5)/8} mod p */
    if ( 0 < bgnfp_p->len && 5 == ( bgnfp_p->data[ 0 ] & 7 ) )
    {
        /* exp = (p-5)/8 */
        bgn_z_set_word(bgnz_md_id, tmp, 5);
        bgn_z_sub(bgnz_md_id, bgnfp_p, tmp, exp);
        bgn_z_shr_lesswordsize(bgnz_md_id, tmp, 3, exp);

        /* ta = 4a mod p*/
        bgn_fp_shl_lesswordsize(bgnfp_md_id, a, 2, ta);

        /* (4a)^{(p-5)/8} = ta ^ exp (mod p) */
        bgn_fp_exp(bgnfp_md_id, ta, exp, tmp);

        /* x = 2a *(4a)^{(p-5)/8} = 2a * tmp (mod p) */
        /* ta = 2a mod p*/
        bgn_fp_shl_lesswordsize(bgnfp_md_id, a, 1, ta);
        bgn_fp_mul(bgnfp_md_id, ta, tmp, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, ta, LOC_BGNFP_0047);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, exp, LOC_BGNFP_0048);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, tmp, LOC_BGNFP_0049);
#endif /* STATIC_MEMORY_SWITCH */

        return EC_TRUE;
    }

    /* if p mod 8 = 1, then call squroot common procedure */
    if ( 0 < bgnfp_p->len && 1 == ( bgnfp_p->data[ 0 ] & 7 ) )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, ta, LOC_BGNFP_0050);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, exp, LOC_BGNFP_0051);
        free_static_mem(MD_BGNFP, bgnfp_md_id, MM_BIGINT, tmp, LOC_BGNFP_0052);
#endif /* STATIC_MEMORY_SWITCH */

        return bgn_fp_squroot_common(bgnfp_md_id, a, c);
    }

    return EC_FALSE;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
