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

#include "bgnz2.h"
#include "bgnf2n.h"

#include "debug.h"

#include "print.h"

#include "log.h"


/* set bgnf2n_n = n */
/* set bgnf2n_f = the generator polynomial f(x) */

static BGNF2N_MD g_bgnf2n_md[ MAX_NUM_OF_BGNF2N_MD ];
static EC_BOOL  g_bgnf2n_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_bgnf2n_buf[ MAX_NUM_OF_BGNF2N_MD ];
#endif/* STACK_MEMORY_SWITCH */

/**
*   for test only
*
*   to query the status of BGNF2N Module
*
**/
void bgn_f2n_print_module_status(const UINT32 bgnf2n_md_id, LOG *log)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32 index;

    if ( EC_FALSE == g_bgnf2n_md_init_flag )
    {

        sys_log(log,"no BGNF2N Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_BGNF2N_MD; index ++ )
    {
        bgnf2n_md = &(g_bgnf2n_md[ index ]);
        /* if module was started, check if it's the ring F_{2^n} */

        if ( 0 < bgnf2n_md->usedcounter )
        {
            sys_log(log,"BGNF2N Module#%ld : %ld refered, refer BGNZ2 Module : %ld\n",
                    index,
                    bgnf2n_md->usedcounter,
                    bgnf2n_md->bgnz2_md_id);
        }
   }
}

/**
*
*   free all static memory occupied by the appointed BGNF2N module
*
*
**/
UINT32 bgn_f2n_free_module_static_mem(const UINT32 bgnf2n_md_id)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_free_module_static_mem: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    free_module_static_mem(MD_BGNF2N, bgnf2n_md_id);
    bgn_z2_free_module_static_mem(bgnz2_md_id);

    return 0;
}
/**
*
* start BGNF2N module
*
**/
UINT32 bgn_f2n_start( const BIGINT *f_x )
{
    UINT32  bgnz2_md_id;
    UINT32  bgnf2n_n;
    BIGINT *bgnf2n_f;
    BGNF2N_MD *bgnf2n_md;
    UINT32 bgnf2n_md_id;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == f_x )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_start: fx is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNZMOD module, then */
    /* initialize g_bgnf2n_md */
    if ( EC_FALSE ==  g_bgnf2n_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_BGNF2N_MD; index ++ )
        {
            bgnf2n_md = &(g_bgnf2n_md[ index ]);

            bgnf2n_md->usedcounter   = 0;
            bgnf2n_md->bgnz2_md_id   = ERR_MODULE_ID;
            bgnf2n_md->bgnf2n_n     = 0;
            bgnf2n_md->bgnf2n_f     = NULL_PTR;
        }

        /*register all functions of BGNF2N module to DBG module*/
        //dbg_register_func_addr_list(g_bgnf2n_func_addr_list, g_bgnf2n_func_addr_list_len);

        g_bgnf2n_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_BGNF2N_MD; index ++ )
    {
        bgnf2n_md = &(g_bgnf2n_md[ index ]);
        /* if module was started, check if it's the ring F_{2^n} */
        if ( 0 == bgnf2n_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_BGNF2N_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz2_md_id = ERR_MODULE_ID;
    bgnf2n_n = 0;
    bgnf2n_f = NULL_PTR;

    bgnf2n_md_id = index;
    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &bgnf2n_f, LOC_BGNF2N_0001);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    bgnf2n_f = &(g_bgnf2n_buf[ bgnf2n_md_id ]);
#endif/* STACK_MEMORY_SWITCH */


#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == bgnf2n_f )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_start: bgnf2n_f is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md_id = bgn_z2_start();

    /* set bgnf2n_f = f_x */
    bgn_z2_clone(bgnz2_md_id, f_x, bgnf2n_f);

    /* set bgnf2n_n = deg(f_x) */
    bgnf2n_n = bgn_z2_deg(bgnz2_md_id, f_x);

    /* set module : */
    bgnf2n_md->bgnz2_md_id = bgnz2_md_id;
    bgnf2n_md->bgnf2n_n = bgnf2n_n;
    bgnf2n_md->bgnf2n_f = bgnf2n_f;

    /* at the first time, set the counter to 1 */
    bgnf2n_md->usedcounter = 1;

    return ( bgnf2n_md_id );
}

/**
*
* end BGNF2N module
*
**/
void bgn_f2n_end(const UINT32 bgnf2n_md_id)
{
    UINT32  bgnz2_md_id;
    UINT32  bgnf2n_n;
    BIGINT *bgnf2n_f;
    BGNF2N_MD *bgnf2n_md;

    if ( MAX_NUM_OF_BGNF2N_MD < bgnf2n_md_id )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_end: bgnf2n_md_id = %ld is overflow.\n",bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < bgnf2n_md->usedcounter )
    {
        bgnf2n_md->usedcounter --;

        return ;
    }

    if ( 0 == bgnf2n_md->usedcounter )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_end: bgnf2n_md_id = %ld is not started.\n",bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;
    bgnf2n_n   = bgnf2n_md->bgnf2n_n;
    bgnf2n_f = bgnf2n_md->bgnf2n_f;

    bgn_z2_end( bgnz2_md_id );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, bgnf2n_f, LOC_BGNF2N_0002);
#endif/* STATIC_MEMORY_SWITCH */

    /* free module : */
    //bgn_f2n_free_module_static_mem(bgnf2n_md_id);
    bgnf2n_md->bgnz2_md_id = ERR_MODULE_ID;
    bgnf2n_md->bgnf2n_n   = 0;
    bgnf2n_md->bgnf2n_f   = NULL_PTR;
    bgnf2n_md->usedcounter = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}


/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_f2n_cmp(const UINT32 bgnf2n_md_id,const BIGINT * a,const BIGINT *b)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_cmp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_cmp: b is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_cmp: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_cmp(bgnz2_md_id,a, b);
}

/**
*   clone src to des
*   return des where des = src
**/
void bgn_f2n_clone(const UINT32 bgnf2n_md_id,const BIGINT * src,BIGINT * des)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_clone: src is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == des )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_clone: des is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_clone: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_clone(bgnz2_md_id, src, des);
}

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_zero(const UINT32 bgnf2n_md_id,const BIGINT* src)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_is_zero: src is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_is_zero: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_is_zero(bgnz2_md_id, src );
}

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_one(const UINT32 bgnf2n_md_id,const BIGINT* src)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_is_one: src is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_is_one: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_is_one(bgnz2_md_id, src );
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_f2n_is_odd(const UINT32 bgnf2n_md_id,const BIGINT *src)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_is_odd: src is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_is_odd: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_is_odd(bgnz2_md_id, src );

}

/**
*
*   set a = 0
**/
void bgn_f2n_set_zero(const UINT32 bgnf2n_md_id,BIGINT * a)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_set_zero: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_set_zero: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_set_word(bgnz2_md_id,  a, 0 );
}

/**
*
*   set a = 1
**/
void bgn_f2n_set_one(const UINT32 bgnf2n_md_id,BIGINT * a)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_set_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_set_one: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_set_word(bgnz2_md_id,a, 1 );
}

/**
*
*   set a = n
**/
void bgn_f2n_set_word(const UINT32 bgnf2n_md_id,BIGINT *a,const UINT32 n)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_set_word: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_set_word: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_set_word(bgnz2_md_id, a, n);
}

/**
*   if k = 0, then return s[ 0 ] = 0 and 0
*   if k > 0, then
*       let k = SUM( k_i * 2 ^ i, i = 0..n, k_i takes 0 or 1, and k_n = 1 )
*   return s =[ k_0,...,k_n ] and n
*   i.e,
*       s[ 0 ] = k_0,... s[ n ] = k_n
*
**/
UINT32 bgn_f2n_parse_bits(const UINT32 bgnf2n_md_id,const BIGINT *k,UINT32 *s)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_parse_bits: k is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == s )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_parse_bits: s is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_parse_bits: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_parse_bits(bgnz2_md_id, k, s);
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
int bgn_f2n_naf(const UINT32 bgnf2n_md_id,const BIGINT *k,int *s)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_naf: k is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == s )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_naf: s is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_naf: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return bgn_z2_naf(bgnz2_md_id, k, s);
}

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m)
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 bgn_f2n_deg(const UINT32 bgnf2n_md_id,const BIGINT *a)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_deg: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_deg: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    return ( bgn_z2_deg(bgnz2_md_id, a ) );
}

/**
*
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   let a0 = a mod x ^{WORDSIZE}
*   return c = ( a - a0 ) /( x ^{WORDSIZE} ) ) over F_{2^n}  = ( a - a0 ) /( x ^{WORDSIZE} ) )
*
*   note:
*       maybe address of c = address of a
*
**/
void bgn_f2n_shr_onewordsize(const UINT32 bgnf2n_md_id,const BIGINT *a,BIGINT *c)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shr_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shr_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shr_onewordsize: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_sshr_onewordsize(bgnz2_md_id, a, c);

    return ;
}

/**
*
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   let a0 = a mod x ^{nbits}
*   return c = ( a - a0 ) /( x ^{nbits} ) ) over F_{2^n} = ( a - a0 ) /( x ^{nbits} ) )
*
*   note:
*       maybe address of c = address of a
*
**/
void bgn_f2n_shr_lesswordsize(const UINT32 bgnf2n_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shr_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shr_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shr_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shr_lesswordsize: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_sshr_lesswordsize(bgnz2_md_id, a, nbits, c);

    return ;
}

/**
*   let a belong to F_{2^n} = Z_2[x]/f(x)
*   return c = ( a * x ^{WORDSIZE} ) over F_{2^n} = ( a * x ^{WORDSIZE} ) mod f(x)
*
*   note:
*       maybe address of c = address of a
**/
void bgn_f2n_shl_onewordsize(const UINT32 bgnf2n_md_id,const BIGINT * a, BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

    UINT32 shift_out;
    BGNF2N_MD   *bgnf2n_md;
    UINT32  bgnz2_md_id;
    BIGINT      *bgnf2n_f;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shl_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shl_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shl_onewordsize: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;
    bgnf2n_f    = bgnf2n_md->bgnf2n_f;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_1;
    c1 = &buf_2;
    q0 = &buf_3;
    q1 = &buf_4;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&c0, LOC_BGNF2N_0003);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&c1, LOC_BGNF2N_0004);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&q0, LOC_BGNF2N_0005);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&q1, LOC_BGNF2N_0006);
#endif/* STATIC_MEMORY_SWITCH */

    shift_out = bgn_z2_sshl_onewordsize(bgnz2_md_id, a, c0);
    if ( 0 < shift_out )
    {
        bgn_z2_set_word(bgnz2_md_id, c1, shift_out);
        bgn_z2_ddiv(bgnz2_md_id, c0, c1, bgnf2n_f, q0, q1, c);
    }
    else
    {
        bgn_z2_div(bgnz2_md_id, c0, bgnf2n_f, q0, c);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,c0, LOC_BGNF2N_0007);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,c1, LOC_BGNF2N_0008);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,q0, LOC_BGNF2N_0009);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,q1, LOC_BGNF2N_0010);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   return c = ( a * x ^{nbits} ) over F_{2^n} = Z_2[x]/f(x)
*
*   maybe address of c = address of a
**/
void bgn_f2n_shl_lesswordsize(const UINT32 bgnf2n_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

    UINT32 shift_out;
    BGNF2N_MD   *bgnf2n_md;
    UINT32  bgnz2_md_id;
    BIGINT      *bgnf2n_f;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shl_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_shl_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shl_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_shl_lesswordsize: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;
    bgnf2n_f    = bgnf2n_md->bgnf2n_f;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_1;
    c1 = &buf_2;
    q0 = &buf_3;
    q1 = &buf_4;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&c0, LOC_BGNF2N_0011);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&c1, LOC_BGNF2N_0012);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&q0, LOC_BGNF2N_0013);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&q1, LOC_BGNF2N_0014);
#endif/* STATIC_MEMORY_SWITCH */

    shift_out = bgn_z2_sshl_lesswordsize(bgnz2_md_id, a, nbits, c0);
    if ( 0 < shift_out )
    {
        bgn_z2_set_word(bgnz2_md_id, c1, shift_out);
        bgn_z2_ddiv(bgnz2_md_id, c0, c1, bgnf2n_f, q0, q1, c);
    }
    else
    {
        bgn_z2_div(bgnz2_md_id, c0, bgnf2n_f, q0, c);
    }
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,c0, LOC_BGNF2N_0015);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,c1, LOC_BGNF2N_0016);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,q0, LOC_BGNF2N_0017);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,q1, LOC_BGNF2N_0018);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a + b mod f(x) over Z_2[x]
*
**/
void bgn_f2n_add(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add: b is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_add: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_add(bgnz2_md_id, a, b, c);
}

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a - b mod f(x) over Z_2[x]
*
**/
void bgn_f2n_sub(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_sub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_sub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_sub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_sub: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_sub(bgnz2_md_id, a, b, c);
}

/**
*       return c = a + b * x^( offset )
*       Must
*           offset > 0
*       and
*           deg(a) < deg(f) < BIGINTSIZE
*       and
*           deg(b) + offset < deg(f) < BIGINTSIZE
*
*       compute order restrict : from High to Low
**/
void bgn_f2n_add_offset(const UINT32 bgnf2n_md_id,const BIGINT *a,const BIGINT *b,const UINT32 offset,BIGINT *c)
{
    BGNF2N_MD *bgnf2n_md;
    UINT32  bgnz2_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add_offset: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add_offset: b is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_add_offset: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_add_offset: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

    bgn_z2_add_offset(bgnz2_md_id, a, b, offset, c);
}

/**
*   deg (a) < n
*
*   c = a ^ 2 over F_{2^n}, where F_{2^n} = Z_2[ x ]/ f(x)
*   i.e,
*   c = a ^ 2 mod f(x) over F2[x]
*
**/
void bgn_f2n_squ(const UINT32 bgnf2n_md_id,const BIGINT* a,BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *t0;
    BIGINT *t1;
    BIGINT *q0;
    BIGINT *q1;

    BGNF2N_MD *bgnf2n_md;
    BIGINT *bgnf2n_f;
    UINT32  bgnz2_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_squ: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_squ: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_squ: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnf2n_f = bgnf2n_md->bgnf2n_f;
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t0 = &buf_0;
    t1 = &buf_1;
    q0 = &buf_2;
    q1 = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &t0, LOC_BGNF2N_0019);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &t1, LOC_BGNF2N_0020);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &q0, LOC_BGNF2N_0021);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &q1, LOC_BGNF2N_0022);
#endif /* STATIC_MEMORY_SWITCH */

    /* t = (t1,t0) = a * b over Z_2 */
    bgn_z2_squ(bgnz2_md_id, a, t0, t1);

    /* t = f_x * q + c, so that */
    /* c = t mod f_x over Z_2 = t over F_{2^n} */
    /* where F_{2^n} = Z_2[ x ]/ (f_x)*/
    bgn_z2_ddiv(bgnz2_md_id, t0, t1, bgnf2n_f, q0, q1, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, t0, LOC_BGNF2N_0023);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, t1, LOC_BGNF2N_0024);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, q0, LOC_BGNF2N_0025);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, q1, LOC_BGNF2N_0026);
#endif /* STATIC_MEMORY_SWITCH */

    return ;
}
/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a * b over F_{2^n}, where F_{2^n} = Z_2[ x ]/ f(x)
*   i.e,
*   c = a * b mod f(x) over F2[x]
*
**/
void bgn_f2n_mul(const UINT32 bgnf2n_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *t0;
    BIGINT *t1;
    BIGINT *q0;
    BIGINT *q1;

    BGNF2N_MD *bgnf2n_md;
    BIGINT *bgnf2n_f;
    UINT32  bgnz2_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_mul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == b )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_mul: b is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_mul: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_mul: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnf2n_f = bgnf2n_md->bgnf2n_f;
    bgnz2_md_id = bgnf2n_md->bgnz2_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t0 = &buf_0;
    t1 = &buf_1;
    q0 = &buf_2;
    q1 = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &t0, LOC_BGNF2N_0027);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &t1, LOC_BGNF2N_0028);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &q0, LOC_BGNF2N_0029);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, &q1, LOC_BGNF2N_0030);
#endif /* STATIC_MEMORY_SWITCH */

    /* t = (t1,t0) = a * b over Z_2 */
    bgn_z2_mul(bgnz2_md_id, a, b, t0, t1);

    /* t = f_x * q + c, so that */
    /* c = t mod f_x over Z_2 = t over F_{2^n} */
    /* where F_{2^n} = Z_2[ x ]/ (f_x)*/
    bgn_z2_ddiv(bgnz2_md_id, t0, t1, bgnf2n_f, q0, q1, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, t0, LOC_BGNF2N_0031);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, t1, LOC_BGNF2N_0032);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, q0, LOC_BGNF2N_0033);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT, q1, LOC_BGNF2N_0034);
#endif /* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   a != 0 and deg (a) < n
*
*   c = a ^{-1} over F_{2^n} = Z_2[x]/f(x)
*   i.e,
*   c = a ^{-1} mod f(x) over Z_2[x]
*
*   Algorithm:
*   Extended Euclidean Algorithm in F_{2^n}
*   Input :  a belong to F_{2^n}, a is nonzero.
*   Output:  b belong to F_{2^n} ,where a*b = 1 mod fx
*   1. b = 1, c = 0, u = a, v = f
*   2. while deg(u) is nonzero do
*   2.1  j = deg(u) - deg(v)
*   2.2  if j < 0 then u <-> v, b <-> c, j = -j
*   2.3  u = u + v * x^j, b = b + c * x^j
*   3. return(b)
*/
EC_BOOL bgn_f2n_inv(const UINT32 bgnf2n_md_id,const BIGINT* a,BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *pa;
    BIGINT *pb;
    BIGINT *pc;
    BIGINT *pu;
    BIGINT *pv;
    BIGINT *ptmp;

    UINT32  bgnf2n_n;
    BIGINT *bgnf2n_f;
    BGNF2N_MD *bgnf2n_md;

    UINT32 deg_a;
    UINT32 deg_u;
    UINT32 deg_v;
    UINT32 deg_offset;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_inv: a is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
    if ( NULL_PTR == c )
    {
        dbg_log(SEC_0093_BGNF2N, 0)(LOGSTDOUT,"error:bgn_f2n_inv: c is NULL_PTR.\n");
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNF2N_MD <= bgnf2n_md_id || 0 == g_bgnf2n_md[ bgnf2n_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_inv: bgnf2n module #0x%lx not started.\n",
                bgnf2n_md_id);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_f2n_is_zero(bgnf2n_md_id, a ) )
    {
        return EC_FALSE;
    }

    bgnf2n_md = &(g_bgnf2n_md[ bgnf2n_md_id ]);
    bgnf2n_n = bgnf2n_md->bgnf2n_n;
    bgnf2n_f = bgnf2n_md->bgnf2n_f;

    deg_a = bgn_f2n_deg(bgnf2n_md_id,  a );
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( bgnf2n_n <= deg_a )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_f2n_inv: deg(fx) = 0x%lx <= deg(a)=0x%lx\n",
                bgnf2n_n,
                deg_a);
        dbg_exit(MD_BGNF2N, bgnf2n_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    pb = &buf_1;
    pc = &buf_2;
    pu = &buf_3;
    pv = &buf_4;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&pb, LOC_BGNF2N_0035);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&pc, LOC_BGNF2N_0036);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&pu, LOC_BGNF2N_0037);
    alloc_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,&pv, LOC_BGNF2N_0038);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_f2n_set_one(bgnf2n_md_id, pb);
    bgn_f2n_set_zero(bgnf2n_md_id, pc);

    pa = (BIGINT *)a;
    bgn_f2n_clone(bgnf2n_md_id, pa,pu);

    /*v = f*/
    bgn_f2n_clone( bgnf2n_md_id, bgnf2n_f, pv );

    while ( 0 != ( deg_u = bgn_f2n_deg(bgnf2n_md_id,  pu ) ) )
    {
        deg_v = bgn_f2n_deg(bgnf2n_md_id, pv);

        if ( deg_u < deg_v )
        {
            ptmp = pu;
            pu = pv;
            pv = ptmp;

            ptmp = pb;
            pb = pc;
            pc = ptmp;

            deg_offset = deg_v - deg_u;
        }
        else
        {
            deg_offset = deg_u - deg_v;
        }

        bgn_f2n_add_offset(bgnf2n_md_id, pu, pv, deg_offset, pu );
        bgn_f2n_add_offset(bgnf2n_md_id, pb, pc, deg_offset, pb );
    }

    bgn_f2n_clone( bgnf2n_md_id, pb, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,pb, LOC_BGNF2N_0039);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,pc, LOC_BGNF2N_0040);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,pu, LOC_BGNF2N_0041);
    free_static_mem(MD_BGNF2N, bgnf2n_md_id, MM_BIGINT,pv, LOC_BGNF2N_0042);
#endif/* STATIC_MEMORY_SWITCH */

    return EC_TRUE;
}
#ifdef __cplusplus
}
#endif/*__cplusplus*/

