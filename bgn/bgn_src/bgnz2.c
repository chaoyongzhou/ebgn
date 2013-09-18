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

#include "bgnz.h"
#include "bgnz2.h"

#include "debug.h"

#include "print.h"



static BGNZ2_MD g_bgnz2_md[ MAX_NUM_OF_BGNZ2_MD ];
static EC_BOOL  g_bgnz2_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_bgn_z2_premul_buf[ MAX_NUM_OF_BGNZ2_MD ][ MAX_SIZE_OF_BGNZ2_PREMUL_TABLE ][ 2 ];
#endif/* STACK_MEMORY_SWITCH */
#endif /*BIGINT_DEBUG_SWITCH*/

static BIGINT  *g_bgn_z2_premul_tbl[ MAX_NUM_OF_BGNZ2_MD ][ MAX_SIZE_OF_BGNZ2_PREMUL_TABLE ][ 2 ];

/**
*   for test only
*
*   to query the status of BGNZ2 Module
*
**/
void bgn_z2_print_module_status(const UINT32 bgnz2_md_id, LOG *log)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 index;

    if ( EC_FALSE == g_bgnz2_md_init_flag )
    {

        sys_log(log,"no BGNZ2 Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_BGNZ2_MD; index ++ )
    {
        bgnz2_md = &(g_bgnz2_md[ index ]);

        if ( 0 < bgnz2_md->usedcounter )
        {
            sys_log(log,"BGNZ2 Module # %ld : %ld refered, refer BGNZ Module : %ld\n",
                    index,
                    bgnz2_md->usedcounter,
                    bgnz2_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed BGNZ2 module
*
*
**/
UINT32 bgn_z2_free_module_static_mem(const UINT32 bgnz2_md_id)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_free_module_static_mem: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    free_module_static_mem(MD_BGNZ2, bgnz2_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}
/**
*
* start BGNZ2 module
*
**/
UINT32 bgn_z2_start( )
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz2_md_id;
    UINT32 bgnz_md_id;

    UINT32 index;

    /* if this is the 1st time to start BGNZ2 module, then */
    /* initialize g_bgnz2_md */
    if ( EC_FALSE ==  g_bgnz2_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_BGNZ2_MD; index ++ )
        {
            bgnz2_md = &(g_bgnz2_md[ index ]);

            bgnz2_md->usedcounter   = 0;

            bgnz2_md->bgnz_md_id = ERR_MODULE_ID;
        }

        /*register all functions of BGNZ2 module to DBG module*/
        //dbg_register_func_addr_list(g_bgnz2_func_addr_list, g_bgnz2_func_addr_list_len);

        g_bgnz2_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_BGNZ2_MD; index ++ )
    {
        bgnz2_md = &(g_bgnz2_md[ index ]);

        if ( 0 == bgnz2_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_BGNZ2_MD )
    {
        return ( ERR_MODULE_ID );
    }


    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;

    /* initilize new one BGNZ2 module */
    bgnz2_md_id = index;
    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )

    for ( index = 0; index < MAX_SIZE_OF_BGNZ2_PREMUL_TABLE; index ++ )
    {
        g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 0 ] = &(g_bgn_z2_premul_buf[ bgnz2_md_id ][ index ][ 0 ]);
        g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 1 ] = &(g_bgn_z2_premul_buf[ bgnz2_md_id ][ index ][ 1 ]);
    }
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    for ( index = 0; index < MAX_SIZE_OF_BGNZ2_PREMUL_TABLE; index ++ )
    {
        alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, &(g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 0 ]), LOC_BGNZ2_0001);
        alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, &(g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 1 ]), LOC_BGNZ2_0002);
    }
#endif/* STATIC_MEMORY_SWITCH */

    bgnz_md_id = bgn_z_start();

    bgnz2_md->bgnz_md_id = bgnz_md_id;
    bgnz2_md->usedcounter = 1;

    return ( bgnz2_md_id );
}

/**
*
* end BGNZ2 module
*
**/
void bgn_z2_end(const UINT32 bgnz2_md_id)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

    UINT32 index;

    if ( MAX_NUM_OF_BGNZ2_MD < bgnz2_md_id )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_end: bgnz2_md_id = %ld is overflow.\n",bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < bgnz2_md->usedcounter )
    {
        bgnz2_md->usedcounter --;
        return ;
    }

    if ( 0 == bgnz2_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_end: bgnz2_md_id = %ld is not started.\n",bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id = bgnz2_md->bgnz_md_id;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    for ( index = 0; index < MAX_SIZE_OF_BGNZ2_PREMUL_TABLE; index ++ )
    {
        g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 0 ] = NULL_PTR;
        g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 1 ] = NULL_PTR;
    }
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    for ( index = 0; index < MAX_SIZE_OF_BGNZ2_PREMUL_TABLE; index ++ )
    {
        free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 0 ]), LOC_BGNZ2_0003);
        free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ index ][ 1 ]), LOC_BGNZ2_0004);
    }
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_end( bgnz_md_id);

    /* free module : */
    //bgn_z2_free_module_static_mem(bgnz2_md_id);
    bgnz2_md->bgnz_md_id = ERR_MODULE_ID;
    bgnz2_md->usedcounter = 0;

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
int bgn_z2_cmp(const UINT32 bgnz2_md_id,const BIGINT * a,const BIGINT *b)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_cmp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_cmp: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_cmp: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_cmp(bgnz_md_id, a, b);
}

/**
*   clone src to des
*   return des where des = src
**/
void bgn_z2_clone(const UINT32 bgnz2_md_id,const BIGINT * src,BIGINT * des)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_clone: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == des )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_clone: des is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_clone: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_clone(bgnz_md_id, src, des);

    return ;
}

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_zero(const UINT32 bgnz2_md_id,const BIGINT* src)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_is_zero: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_is_zero: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_is_zero( bgnz_md_id,src );
}

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_one(const UINT32 bgnz2_md_id,const BIGINT* src)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_is_one: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_is_one: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_is_one( bgnz_md_id, src );
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_z2_is_odd(const UINT32 bgnz2_md_id,const BIGINT *src)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_is_odd: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_is_odd: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_is_odd( bgnz_md_id , src );
}

/**
*
*   set a = 0
**/
void bgn_z2_set_zero(const UINT32 bgnz2_md_id,BIGINT * a)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_set_zero: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_set_zero: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_set_word( bgnz_md_id, a, 0 );
}

/**
*
*   set a = 1
**/
void bgn_z2_set_one(const UINT32 bgnz2_md_id,BIGINT * a)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_set_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_set_one: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_set_word( bgnz_md_id, a, 1 );

    return;
}

/**
*
*   set a = n
**/
void bgn_z2_set_word(const UINT32 bgnz2_md_id,BIGINT *a,const UINT32 n)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_set_word: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_set_word: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_set_word(bgnz_md_id, a, n);
    return ;
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
UINT32 bgn_z2_parse_bits(const UINT32 bgnz2_md_id,const BIGINT *k,UINT32 *s)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_parse_bits: k is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == s )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_parse_bits: s is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_parse_bits: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_parse_bits(bgnz_md_id, k, s);
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
int bgn_z2_naf(const UINT32 bgnz2_md_id,const BIGINT *k,int *s)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_naf: k is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == s )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_naf: s is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_naf: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_naf(bgnz_md_id, k, s);
}

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m) over Z_2[x]
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 bgn_z2_deg(const UINT32 bgnz2_md_id,const BIGINT *a)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

    UINT32 nbits;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_deg: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_deg: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    nbits = bgn_z_get_nbits( bgnz_md_id, a );
    if ( 0 == nbits )
    {
        return ( 0 );
    }

    return ( nbits - 1 );
}

/**
*   return e = x ^ nth over Z_2[x]
*   where nth = 0,1,...
*
**/
void bgn_z2_set_e(const UINT32 bgnz2_md_id,BIGINT *e,const UINT32 nth)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_set_e: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_set_e: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_set_e(bgnz_md_id, e,nth);

    return ;
}

/**
*
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 bgn_z2_get_bit(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nthbit)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_get_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_get_bit: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_get_bit(bgnz_md_id, a, nthbit);
}

/**
*   let a = SUM(a_i * x^i, where i = 0..BIGINTSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z2_set_bit(const UINT32 bgnz2_md_id,BIGINT *a, const UINT32 nthbit)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_set_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_set_bit: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_set_bit(bgnz_md_id, a, nthbit);

    return ;
}

/**
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z2_clear_bit(const UINT32 bgnz2_md_id,BIGINT *a, const UINT32 nthbit)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_clear_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_clear_bit: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_clear_bit(bgnz_md_id, a, nthbit);

    return ;
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{WORDSIZE}
*
*   return c = ( a - a0 ) / (x ^ {BIGINTSIZE}) and a0 is returned.
*   hence a = ( c *  x ^ {BIGINTSIZE} ) + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshr_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,BIGINT *c0,BIGINT *c1)
{
    UINT32 a1_shift_out;
    UINT32 a0_shift_out;
    UINT32 c0_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c1 == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_shr_onewordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, a1 ) )
    {

        bgn_z2_set_zero(bgnz2_md_id, c1 );

        a0_shift_out = bgn_z2_sshr_onewordsize(bgnz2_md_id,a0, c0);

        return ( a0_shift_out );
    }

    a1_shift_out = bgn_z2_sshr_onewordsize(bgnz2_md_id,a1, c1);
    a0_shift_out = bgn_z2_sshr_onewordsize(bgnz2_md_id,a0, c0);

    if ( 0 < a1_shift_out )
    {
        c0_len = c0->len;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( INTMAX <= c0_len )
        {
            sys_log(LOGSTDOUT,"error:bgn_z2_shr_onewordsize: c0_len = %ld overflow.\n",c0_len);
            dbg_exit(MD_BGNZ2, bgnz2_md_id);
        }
#endif/*BIGINT_DEBUG_SWITCH*/

        c0->data[ INTMAX - 1 ] = a1_shift_out;

        /* set c0[ INTMAX-2 .. c0_len ] = 0 */
        for ( index = INTMAX - 1; index > c0_len; )
        {
            index --;
            c0->data[ index ] = 0;
        }
        c0->len = INTMAX;
    }

    return ( a0_shift_out );
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{nbits}
*
*   return c = ( a - a0 ) / (x ^ {nbits}) and a0 is returned.
*   hence a = ( c *  x ^ {nbits} ) + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshr_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    UINT32 a1_shift_out;
    UINT32 a0_shift_out;
    UINT32 c0_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c1 == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_shr_lesswordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, a1 ) )
    {

        bgn_z2_set_zero(bgnz2_md_id, c1 );

        a0_shift_out = bgn_z2_sshr_lesswordsize(bgnz2_md_id,a0, nbits, c0);

        return ( a0_shift_out );
    }

    a1_shift_out = bgn_z2_sshr_lesswordsize(bgnz2_md_id,a1, nbits, c1);
    a0_shift_out = bgn_z2_sshr_lesswordsize(bgnz2_md_id,a0, nbits, c0);

    if ( 0 < a1_shift_out )
    {
        c0_len = c0->len;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( INTMAX < c0_len )
        {
            sys_log(LOGSTDOUT,"error:bgn_z2_shr_lesswordsize: c0_len = %ld overflow.\n",c0_len);
            dbg_exit(MD_BGNZ2, bgnz2_md_id);
        }
#endif/*BIGINT_DEBUG_SWITCH*/
        c0->data[ INTMAX - 1 ] |= ( a1_shift_out << ( WORDSIZE - nbits ) );

        /* set c0[ INTMAX-2 .. c0_len ] = 0 */
        for ( index = INTMAX - 1; index > c0_len;  )
        {
            index --;
        }
        c0->len = INTMAX;
   }

    return ( a0_shift_out );
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*   let a0 = a mod x ^{nbits}
*
*   return c = ( a - a0 ) / (x ^ {nbits}) and a0 is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_dshr_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshr_no_shift_out: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshr_no_shift_out: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshr_no_shift_out: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshr_no_shift_out: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshr_no_shift_out: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_dshr_no_shift_out: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_dshr_no_shift_out(bgnz_md_id, a0, a1, nbits, c0, c1);

    return ;
}


/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c =  a  * x ^ {WORDSIZE} mod x ^{2 * BIGINTSIZE}
*   and the shifted out part is returned.
*   hence a  * x ^ {WORDSIZE} = shift_out * x ^( 2 * BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshl_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,BIGINT *c0,BIGINT *c1)
{
    UINT32 a1_shift_out;
    UINT32 a0_shift_out;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_onewordsize: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_onewordsize: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_onewordsize: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_onewordsize: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c1 == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_onewordsize: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_shl_onewordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    a0_shift_out = bgn_z2_sshl_onewordsize(bgnz2_md_id,a0, c0);
    a1_shift_out = bgn_z2_sshl_onewordsize(bgnz2_md_id,a1, c1);

    if ( 0 < a0_shift_out )
    {
        if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, c1 ) )
        {
            c1->data[ 0 ] = a0_shift_out;
            c1->len = 1;
        }
        else
        {
            c1->data[ 0 ] = a0_shift_out;
        }
    }

    return ( a1_shift_out );
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c = ( a * x ^ {bnits} ) mod x ^( 2 * BIGINTSIZE )
*   and the shifted out part is returned.
*   where nbits < WORDSIZE
*
*   hence ( a * x ^ {bnits} ) = shift_out * x ^( 2 * BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_dshl_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    UINT32 a1_shift_out;
    UINT32 a0_shift_out;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_lesswordsize: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_lesswordsize: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_lesswordsize: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_lesswordsize: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c1 == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_shl_lesswordsize: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_shl_lesswordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    a0_shift_out = bgn_z2_sshl_lesswordsize(bgnz2_md_id,a0, nbits, c0);
    a1_shift_out = bgn_z2_sshl_lesswordsize(bgnz2_md_id,a1, nbits, c1);

    if ( 0 < a0_shift_out )
    {
        if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, c1 )  )
        {
            c1->data[ 0 ] = a0_shift_out;
            c1->len = 1;
        }
        else
        {
            c1->data[ 0 ] |= a0_shift_out;
        }
    }

    return ( a1_shift_out );
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x]
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0 over Z_2[x]
*
*   return c = ( a * x ^ {bnits} ) mod x ^( 2 * BIGINTSIZE )
*   and the shifted out part is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_dshl_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshl_no_shift_out: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshl_no_shift_out: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshl_no_shift_out: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshl_no_shift_out: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dshl_no_shift_out: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_dshl_no_shift_out: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_dshl_no_shift_out(bgnz_md_id, a0, a1, nbits, c0, c1);

    return ;
}

/**
*   let a0 = a mod x ^{WORDSIZE} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {WORDSIZE} ) over Z_2[x]
*   and the shifted out a0 is returned.
*
*   hence a = c * x ^{WORDSIZE} + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshr_onewordsize(const UINT32 bgnz2_md_id,const BIGINT *a,BIGINT *c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshr_onewordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_shr_onewordsize(bgnz_md_id, a, c);
}

/**
*   let a0 = a mod x ^{nbits} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {nbits} ) over Z_2[x]
*   and the shifted out a0 is returned.
*
*   hence a = c * x ^{nbits} + a0
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshr_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshr_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshr_lesswordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_shr_lesswordsize(bgnz_md_id, a, nbits, c);
}

/**
*   let a0 = a mod x ^{nbits} over Z_2[x]
*
*   return c = ( a  - a0 ) / ( x ^ {nbits} ) over Z_2[x]
*   and the shifted out a0 is NOT returned.
*   where 0 <= nbits
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_sshr_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_no_shift_out: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshr_no_shift_out: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshr_no_shift_out: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_shr_no_shift_out(bgnz_md_id, a, nbits, c);

    return ;
}

/**
*
*   return c = ( a  * x ^ {WORDSIZE} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is returned.
*
*   hence ( a  * x ^ {WORDSIZE} ) = shift_out * x ^( BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshl_onewordsize(const UINT32 bgnz2_md_id,const BIGINT * a, BIGINT * c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshl_onewordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_shl_onewordsize(bgnz_md_id, a, c);
}

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is returned.
*
*   hence ( a  * x ^ {nbits} ) = shift_out * x ^( BIGINTSIZE ) + c
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 bgn_z2_sshl_lesswordsize(const UINT32 bgnz2_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshl_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshl_lesswordsize: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    return bgn_z_shl_lesswordsize(bgnz_md_id, a, nbits, c);
}

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{BIGINTSIZE} over Z_2[x]
*   and the shifted out part is NOT returned.
*
*   Note:
*       1. address of c not equal to address of a
*
**/
void bgn_z2_sshl_no_shift_out(const UINT32 bgnz2_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_no_shift_out: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sshl_no_shift_out: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sshl_no_shift_out: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_shl_no_shift_out(bgnz_md_id, a, nbits, c);

    return ;
}


/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over Z_2[x]
*     = a ^ b over Z
*
**/
#if 0
static UINT32 debug_counter = 0;
#endif
void bgn_z2_add(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_add: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_bit_xor(bgnz_md_id, a, b, c);
#if 0
    debug_counter ++;
    if ( 9999 == debug_counter )
    {
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif
    return ;
}

/**
*       c = a + b over Z_2[x]
*         = a ^ b over Z
*       where b < 2 ^ WORDSIZE
**/
void bgn_z2_sadd(const UINT32 bgnz2_md_id,const BIGINT *a,const UINT32 b, BIGINT *c )
{
    UINT32 a_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sadd: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sadd: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sadd: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, a ) )
    {
        c->data[ 0 ] = b;
        c->len = 1;

        return ;
    }

    /* now a_len > 0 */
    a_len = a->len;

    c->data[ 0 ] = a->data[ 0 ] ^ b;

    if ( ( 1 == a_len ) && ( 0 == c->data[ 0 ] ) )
    {
        c->len = 0;
        return ;
    }

    /* now a_len > 1 */
    for ( index = a_len; index > 1;  )
    {
        index --;
        c->data[ index ] = a->data[ index ];
    }
    c->len = a_len;

    return ;
}

/**
*       let a = (a1, a0) = a1 * x^ {BIGINTSIZE} + a0
*           c = (c1, c0) = c1 * x^ {BIGINTSIZE} + c0
*
*       return c = ( a + b ) over Z_2[x]
*                = a ^ b over Z
*
**/
void bgn_z2_dadd(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 )
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a0 == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: address of a0 and a1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: address of a0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: address of b and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dadd: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_dadd: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z2_add(bgnz2_md_id, a0, b, c0);
    bgn_z2_clone(bgnz2_md_id, a1, c1);

    return ;
}

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over Z_2[x]
*     = a ^ b over Z
*
**/
void bgn_z2_sub(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c)
{
    BGNZ2_MD *bgnz2_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_sub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_sub: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz2_md = &(g_bgnz2_md[ bgnz2_md_id ]);
    bgnz_md_id = bgnz2_md->bgnz_md_id;

    bgn_z_bit_xor(bgnz_md_id, a, b, c);

    return ;
}

/**
*       c = ( a - b ) over Z_2[x]
*         = a ^ b over Z
*       where b < 2 ^ WORDSIZE
**/
void bgn_z2_ssub(const UINT32 bgnz2_md_id,const BIGINT *a,const UINT32 b, BIGINT *c )
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ssub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ssub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_ssub: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z2_sadd(bgnz2_md_id, a, b, c);

    return ;
}

/**
*       let a = (a1, a0) = a1 * x^ {BIGINTSIZE} + a0
*           c = (c1, c0) = c1 * x^ {BIGINTSIZE} + c0
*
*       return c = ( a - b ) over Z_2[x]
*                = a ^ b over Z
*
**/
void bgn_z2_dsub(const UINT32 bgnz2_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 )
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dsub: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dsub: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dsub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dsub: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_dsub: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_dsub: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z2_dadd(bgnz2_md_id, a0, a1, b, c0, c1);

    return ;
}

/**
*       return c = a + b * x^( offset ) over Z_2[x]
*                = a ^ ( b << ofset) over Z
*       where
*           offset > 0
*       and
*           deg(a) < BIGINTSIZE
*       and
*           deg(b) + offset < BIGINTSIZE
*
**/
void bgn_z2_add_offset(const UINT32 bgnz2_md_id,const BIGINT *a,const BIGINT *b,const UINT32 offset,BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *tmp;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add_offset: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add_offset: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_add_offset: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_add_offset: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tmp = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, &tmp, LOC_BGNZ2_0005);
#endif/* STACK_MEMORY_SWITCH */

    bgn_z2_sshl_no_shift_out(bgnz2_md_id, b, offset, tmp);
    bgn_z2_add(bgnz2_md_id,  a, tmp, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT, tmp, LOC_BGNZ2_0006);
#endif/* STACK_MEMORY_SWITCH */

    return ;
}
/**
*   deg (a) < BIGINTSIZE
*
*   c = a ^ 2 over Z_2[x]
*
*   Let a = SUM(a_i * x^i, i= 0..m, where a_i is 0 or 1)
*         = ( a_0, a_1, ..., a_m )
*   then
*       c = SUM(a_i * x^{2 *i}, i= 0..m, where a_i is 0 or 1)
*         = ( a_0, 0, a_1, 0, ..., a_{m_1}, 0, a_m )
*   if let c = SUM(c_j * x^j, j= 0.. 2*m, where c_j is 0 or 1)
*   then
*       if j is odd, then c_j = 0
*       if j is even, then c_j = a_{j/2}
*
* History:
*   1. 2006.12.11 fixed the len error of c0,c1
*   add
*        for( ; c0_idx > 0; )
*        {
*            c0_idx --;
*            if ( 0 != c0->data[ c0_idx ] )
*            {
*                c0_idx ++;
*                break;
*            }
*        }
*        c0->len = c0_idx;
*
*        for( ; c1_idx > 0; )
*        {
*            c1_idx --;
*            if ( 0 != c1->data[ c1_idx ] )
*            {
*                c1_idx ++;
*                break;
*            }
*        }
*        c1->len = c1_idx;
**/
void bgn_z2_squ(const UINT32 bgnz2_md_id,const BIGINT* a,BIGINT * c0,BIGINT *c1)
{
    UINT32 a_idx;
    UINT32 c0_idx;
    UINT32 c1_idx;
    UINT32 bit_idx;
    UINT32 t;
    UINT32 t0;
    UINT32 t1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: address of a and c0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: address of a and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_squ: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_squ: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    c0_idx = 0;
    c1_idx = 0;
    for ( a_idx = 0; a_idx < a->len; a_idx ++)
    {
        t = a->data[ a_idx ];
        t0 = 0;
        t1 = 0;
        for ( bit_idx = 0; bit_idx < ( WORDSIZE / 2 ); bit_idx ++ )
        {
            /* take the k_th bit of t, and set it to the 2*k_th bit of t0 */
            t0 |= ( ( ( t >> bit_idx ) & 1 ) << ( 2 * bit_idx ) );
        }
        for ( ; bit_idx < WORDSIZE; bit_idx ++ )
        {
            /* take the k_th bit of t, and set it to the 2 *(k - WORDSIZE/2)_th bit of t1*/
            t1 |= ( ( ( t >> bit_idx ) & 1 ) << ( 2 * bit_idx - WORDSIZE ) );
        }

        if ( c0_idx < INTMAX )
        {
            c0->data[ c0_idx ] = t0;
            c0_idx ++;
        }
        else
        {
            c1->data[ c1_idx ] = t0;
            c1_idx ++;
        }

        if ( c0_idx < INTMAX )
        {
            c0->data[ c0_idx ] = t1;
            c0_idx ++;
        }
        else
        {
            c1->data[ c1_idx ] = t1;
            c1_idx ++;
        }
    }

    for( ; c0_idx > 0; )
    {
        c0_idx --;
        if ( 0 != c0->data[ c0_idx ] )
        {
            c0_idx ++;
            break;
        }
    }
    c0->len = c0_idx;

    for( ; c1_idx > 0; )
    {
        c1_idx --;
        if ( 0 != c1->data[ c1_idx ] )
        {
            c1_idx ++;
            break;
        }
    }
    c1->len = c1_idx;

    return ;
}

/**
*   precompute a table for bgn_z2_mul
*
*   deg (a) < n and a is nonzero over Z_2[x]
*
*   let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_BGNZ2_PREMUL }
*   construct a table
*       { table[i]: i = 0..MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 }
*    =  { a * b over Z_2[x], where b run through the set B }
*   and assume
*       table[b] = a * b over Z_2[x]
*
*   Note:
*       let b = b1 * x + b0,where b0 belong to F_2
*   then
*       a * b = ( a * b1 ) * x + ( a * b0 )
*
*   Algorithm:
*   Input: a belong to Z_2[x] with deg (a) < n and a is nonzero
*   Output: table bgn_z2_premul[]
*   1. set table[ 0 ] = 0
*   2. set table[ 1 ] = a
*   3. for b from 2 to MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 do
*   3.1     let b = 2 * b1 + b0, where b0 belong to {0,1}
*   3.2     table[ b ] = ( table[ b1 ] << 1 ) + table[ b0 ]  over Z_2[x]
*   4.  end for
*   5.return bgn_z2_premul
*
**/
void bgn_z2_premul(const UINT32 bgnz2_md_id,const BIGINT * a)
{
    UINT32 b;
    UINT32 b0;
    UINT32 b1;
    BIGINT *t_b[ 2 ];
    BIGINT *t_b0[ 2 ];
    BIGINT *t_b1[ 2 ];

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_premul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_premul: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    /* set bgn_z2_premul[ 0 ] = 0 */
    b = 0;
    t_b[ 0 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b ][ 0 ]);
    t_b[ 1 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b ][ 1 ]);
    bgn_z2_set_zero( bgnz2_md_id, t_b[ 0 ] );
    bgn_z2_set_zero( bgnz2_md_id, t_b[ 1 ] );

    /* set bgn_z2_premul[ 1 ] = a */
    b = 1;
    t_b[ 0 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b ][ 0 ]);
    t_b[ 1 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b ][ 1 ]);
    bgn_z2_clone( bgnz2_md_id, a, t_b[ 0 ] );
    bgn_z2_set_zero( bgnz2_md_id, t_b[ 1 ] );

    for ( index = 2; index < MAX_SIZE_OF_BGNZ2_PREMUL_TABLE; index ++ )
    {
        b = index;
        b0 = ( b & 1 );
        b1 = ( b >> 1 );

        t_b[ 0 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b  ][ 0 ]);
        t_b[ 1 ]  = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b  ][ 1 ]);

        t_b0[ 0 ] = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b0 ][ 0 ]);
        t_b0[ 1 ] = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b0 ][ 1 ]);

        t_b1[ 0 ] = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b1 ][ 0 ]);
        t_b1[ 1 ] = (g_bgn_z2_premul_tbl[ bgnz2_md_id ][ b1 ][ 1 ]);

        /* compute t_b = (t_b1 << 1) + t_b0 */
        bgn_z2_dshl_lesswordsize(bgnz2_md_id, t_b1[ 0 ],t_b1[ 1 ], 1, t_b[ 0 ],t_b[ 1 ]);

        if ( 0 < b0 )
        {
            bgn_z2_add( bgnz2_md_id, t_b[ 0 ], t_b0[ 0 ], t_b[ 0 ]);
            bgn_z2_add( bgnz2_md_id, t_b[ 1 ], t_b0[ 1 ], t_b[ 1 ]);
        }
    }

    return ;
}

/**
*   deg (a) < BIGINTSIZE
*   deg (b) < BIGINTSIZE
*
*   let c = (c1,c0) = c1 * x ^ {BIGINTSIZE} + c0
*
*   return c = a * b over Z_2[x]
*
*   Note:
*   1. let a = SUM(a_i * x ^{ WINWIDTH_OF_BGNZ2_PREMUL * i}, i = 0..m, where deg(a_i) < WINWIDTH_OF_BGNZ2_PREMUL )
*      then
*          a * b = SUM(a_i * b * x ^{ WINWIDTH_OF_BGNZ2_PREMUL * i}, i = 0..m )
*   2. let a = a1 * x^{WINWIDTH_OF_BGNZ2_PREMUL} + a0
*      then
*          a * b = (a1 * b) * x^{WINWIDTH_OF_BGNZ2_PREMUL} + (a0 *b)
*
*   Algorithm:
*   Input: a,b belong to Z_2[x], and a is nonzero, b is nonzero
*   Output: c = a * b Z_2[x]
*   0. let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_BGNZ2_PREMUL }
*   1. precompute table :
*           { table[i]: i = 0..MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 } =
*           { r * b over Z_2[x], where r run through the set B }
*   2. set c = 0
*   3. for i from m downto 0 do
*   3.1        c = c + table[ a_i ] over Z_2[x]
*   3.2        if i > 0 then do
*                 c = c * x^{WINWIDTH_OF_BGNZ2_PREMUL} over Z_2[x]
*   3.3        end if
*   3.4 end for
*   return c
**/
void bgn_z2_mul(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * c0,BIGINT *c1)
{
    UINT32 a_deg;
    UINT32 win_width;
    UINT32 win_num;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 window;
    UINT32 e;

    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: address of a and c0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: address of a and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: address of b and c0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: address of b and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_mul: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_mul: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* set c = 0 */
    bgn_z2_set_zero(bgnz2_md_id,  c0 );
    bgn_z2_set_zero(bgnz2_md_id,  c1 );

    if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id,  a ) )
    {
        return ;
    }

    if ( EC_TRUE == bgn_z2_is_zero(bgnz2_md_id,  b ) )
    {
        return ;
    }

    /* precompute table { r * b over Z_2, where deg(r) < WINWIDTH_OF_BGNZ2_PREMUL }*/
    bgn_z2_premul(bgnz2_md_id,  b );

    a_deg     = bgn_z2_deg(bgnz2_md_id,  a );
    win_width   = WINWIDTH_OF_BGNZ2_PREMUL; /* must less than WORDSIZE */
    win_num     = ( a_deg + win_width ) / win_width;

    /* e = 0x00000000F when WINWIDTH_OF_BGNZ2_PREMUL = 4 */
    e = ~((~((UINT32)0)) << win_width );
    for ( index = win_num; index > 0; )
    {
        index --;
        word_offset = ( win_width * index ) / WORDSIZE;
        bit_offset  = ( win_width * index ) % WORDSIZE;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( INTMAX <= word_offset )
        {
            sys_log(LOGSTDOUT,"error:bgn_z2_mul: word_offset = %ld overflow\n",word_offset);
            dbg_exit(MD_BGNZ2, bgnz2_md_id);
        }
#endif/*BIGINT_DEBUG_SWITCH*/

        /* a_i = window */
        window = ( ( a->data[ word_offset ] >> ( bit_offset ) ) & e );

        /* c = c + table[ a_i ] */
        bgn_z2_add(bgnz2_md_id, c0,(g_bgn_z2_premul_tbl[ bgnz2_md_id ][ window ][ 0 ]),c0);
        bgn_z2_add(bgnz2_md_id, c1,(g_bgn_z2_premul_tbl[ bgnz2_md_id ][ window ][ 1 ]),c1);

        if ( index > 0 )
        {
            /* c = c * x^{WINWIDTH_OF_BGNZ2_PREMUL} */
            bgn_z2_dshl_lesswordsize(bgnz2_md_id, c0, c1, win_width, c0, c1);
        }
    }

    return ;
}

/**
*   deg (a) < BIGINTSIZE and a is nonzero
*   deg (b) < BIGINTSIZE and b is nonzero
*
*   return (q,r) where
*       a = b * q + r over Z_2[x] with deg( r ) < deg(b)
*
* Algorithm:
*   Input : a, b belong to Z_2[x] and assume a,b are nonzero
*   Output: (q, r) where a = b * q + r
*   1. q = 0;
*   2. while deg(a) >= deg(b) do
*   2.1     let k = deg(a) - deg(b)
*   2.2     a = ( a - b * x ^ k ) over Z_2[x]
*   2.3     q = q + x ^ k over Z_2[x]
*   2.4     next while
*   3.  r = a;
*   4.  return (q, r)
**/
void bgn_z2_div(const UINT32 bgnz2_md_id,const BIGINT* a,const BIGINT * b,BIGINT * q,BIGINT *r)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *ta;
    BIGINT *tmp;
    BIGINT *e;
    UINT32 a_deg;
    UINT32 b_deg;
    UINT32 ta_deg;
    UINT32 diff_deg;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: q is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: address of a and q conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( a == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: address of a and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: address of b and q conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_div: address of q and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_div: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* 0 = b * 0 + 0 */
    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, a ) )
    {
        bgn_z2_set_zero( bgnz2_md_id, q );
        bgn_z2_set_zero( bgnz2_md_id, r );

        return ;
    }
    /* a = 0 * 0 + a */
    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, b ) )
    {
        bgn_z2_set_zero( bgnz2_md_id, q );
        bgn_z2_clone(bgnz2_md_id, a, r );

        return ;
    }

    /* a = 1 * a + 0 */
    if ( EC_TRUE == bgn_z2_is_one( bgnz2_md_id, b ) )
    {
        bgn_z2_clone(bgnz2_md_id, a, q );
        bgn_z2_set_zero( bgnz2_md_id, r );

        return ;
    }

    a_deg = bgn_z2_deg( bgnz2_md_id, a );
    b_deg = bgn_z2_deg( bgnz2_md_id, b );

    /* if deg(a) < deg(b) ,then a = b * 0 + a */
    if ( a_deg < b_deg )
    {
        bgn_z2_set_zero( bgnz2_md_id, q );
        bgn_z2_clone(bgnz2_md_id, a, r );

        return ;
    }
    /* if deg(a) = deg(b) ,then a = b * 1 + ( a - b) mod 2 */
    if ( a_deg == b_deg )
    {
        bgn_z2_set_one( bgnz2_md_id, q );
        bgn_z2_sub(bgnz2_md_id, a, b, r);

        return ;
    }

    /* here deg(a) > deg(b) > 0 */
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
    tmp = &buf_2;
    e = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&ta, LOC_BGNZ2_0007);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&tmp, LOC_BGNZ2_0008);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&e, LOC_BGNZ2_0009);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z2_clone(bgnz2_md_id, a, ta);

    bgn_z2_set_zero( bgnz2_md_id, q );
    bgn_z2_set_zero( bgnz2_md_id, r );

    /* while deg(ta) >= deg(b) > 0 do */
    ta_deg = a_deg;
    while ( ta_deg >= b_deg )
    {
        diff_deg = ta_deg - b_deg;

        bgn_z2_sshl_no_shift_out(bgnz2_md_id, b, diff_deg, tmp);

        /* t_a <-- t_a - tmp */
        bgn_z2_sub(bgnz2_md_id, ta, tmp, ta);

        bgn_z2_set_e(bgnz2_md_id, e, diff_deg);
        bgn_z2_add(bgnz2_md_id, q, e, q);

        ta_deg = bgn_z2_deg( bgnz2_md_id, ta);
    }

    bgn_z2_clone(bgnz2_md_id, ta, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,ta, LOC_BGNZ2_0010);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,tmp, LOC_BGNZ2_0011);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,e, LOC_BGNZ2_0012);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x] and a be nonzero
*   let q = (q1,q0) = q1 * x ^ {BIGINTSIZE} + q0 over Z_2[x] and q be nonzero
*
*   a = b * q + r over Z_2 with deg( r ) < deg(b)
*
*   Algorithm:
*   Input: a = (a1,a0), b and assume a1 > 0 and b > 0
*   Output: q = (q1,q0) and r, where a = b * q + r
*   1. q = 0
*   2. let  x ^ {BIGINTSIZE} = b * q2 + r2 over Z_2[x]
*      then a = b * ( a1 * q2 ) + ( a0 + a1 * r2)
*   3. while a1 is nonzero do
*   3.1     q = q + a1 * q2 over Z_2[x]
*   3.2     a = a0 + a1 * r2 over Z_2[x]
*   3.3     let a = (a1, a0) = a1 * x^ {BIGINTSIZE} + a0 over Z_2[x]
*   3.4     next while
*   4. let a0 = q2 * b + r2 over Z_2[x], then
*       a = q * b + a0 = ( q + q2 ) * b + r2 over Z_2[x]
*   5. q = q + q2 over Z_2[x]
*   6. r = r2
*   7  return (q, r)
**/
void bgn_z2_ddiv_1(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
    BIGINT buf_6;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *ta0;
    BIGINT *ta1;
    BIGINT *q2;
    BIGINT *r2;
    BIGINT *M;
    BIGINT *t0;
    BIGINT *t1;
    /* UINT32 counter; */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: q0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    if ( NULL_PTR == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: q1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( b == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of b and q0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of b and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of q0 and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of q0 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q1 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: address of q1 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_ddiv_1: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if b = 0, then exit */
    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, b ) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_1: b is zero.\n");

        dbg_exit(MD_BGNZ2, bgnz2_md_id) ;
    }

    /* a = 1 * a + 0 */
    if ( EC_TRUE == bgn_z2_is_one( bgnz2_md_id, b ) )
    {
        bgn_z2_clone(bgnz2_md_id, a0, q0);

        bgn_z2_clone(bgnz2_md_id, a1, q1);
        bgn_z2_set_zero( bgnz2_md_id, r );

        return ;
    }

    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, a1 ) )
    {
        bgn_z2_set_zero( bgnz2_md_id, q1 );
        bgn_z2_div(bgnz2_md_id, a0, b, q0, r);
        return ;
    }

    /* here a >0 ( since a1 > 0) and b > 1 */
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta0 = &buf_0;
    ta1 = &buf_1;
    q2  = &buf_2;
    r2  = &buf_3;
    M   = &buf_4;
    t0  = &buf_5;
    t1  = &buf_6;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&ta0, LOC_BGNZ2_0013);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&ta1, LOC_BGNZ2_0014);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&q2, LOC_BGNZ2_0015);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&r2, LOC_BGNZ2_0016);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&M, LOC_BGNZ2_0017);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&t0, LOC_BGNZ2_0018);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&t1, LOC_BGNZ2_0019);
#endif/* STATIC_MEMORY_SWITCH */

    /* compute  x^ {BIGINTSIZE - 1} = q2 * b + r2 */
    /* let M = x^ {BIGINTSIZE - 1} */
    bgn_z2_set_e(bgnz2_md_id, M, BIGINTSIZE - 1);
    bgn_z2_div(bgnz2_md_id, M, b, q2, r2);

    /* based  on M = x^ {BIGINTSIZE - 1} = q2 * b + r2 */
    /* compute x^ {BIGINTSIZE} = M * x = Q2 * b + R2  */
    /* Q2 = q2 * x = ( q2 <<1 ) over Z_2 */
    /* R2 = r2 * x = ( r2 <<1 ) over Z_2 */
    bgn_z2_sshl_lesswordsize(bgnz2_md_id, q2, 1, q2);
    bgn_z2_sshl_lesswordsize(bgnz2_md_id, r2, 1, r2);

    /* now x ^ {BIGINTSIZE} = q2 * b + r2   */
    /* a = a0 + a1 * x ^ {BIGINTSIZE} = a0 + a1 *(q2 * b + r2) */
    /* thus a = ( a0 + a1 * r2 ) + ( a1 * q2 ) * b */

    bgn_z2_clone(bgnz2_md_id, a0, ta0);
    bgn_z2_clone(bgnz2_md_id, a1, ta1);
    bgn_z2_set_zero( bgnz2_md_id, q0 );
    bgn_z2_set_zero( bgnz2_md_id, q1 );

    /* now ta = ta1 * x ^ {BIGINTSIZE} + ta0 */
    /* while ta1 is nonzero do */
    /* counter = 0; */
    while ( EC_FALSE == bgn_z2_is_zero( bgnz2_md_id, ta1 ) )
    {
        /* counter ++; */
        /* q = q + ta1 * q2, let (t1,t0) = ta1 * q2 then */
        /* (q1,q0) = (q1,q0) + (t1,t0) over Z_2 */
        bgn_z2_mul(bgnz2_md_id, ta1, q2, t0, t1);
        bgn_z2_add(bgnz2_md_id, q0, t0, q0);
        bgn_z2_add(bgnz2_md_id, q1, t1, q1);

        /* ta = ta0 + ta1 * r2, let (t1,t0) = ta1 * r2 */
        /* (ta1,ta0) = (0, ta0) + (t1,t0)*/
        bgn_z2_mul(bgnz2_md_id, ta1, r2, t0, t1);
        bgn_z2_dadd(bgnz2_md_id, t0, t1, ta0, ta0, ta1);

    }

    /* now a = ta0 + q * b */
    /* let ta0 = q2 * b + r2 ,then */
    /* a = ( q2 + q ) * b + r2 */
    bgn_z2_div(bgnz2_md_id, ta0, b, q2, r2);
    bgn_z2_dadd(bgnz2_md_id, q0, q1, q2, q0, q1);
    bgn_z2_clone(bgnz2_md_id, r2, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,ta0, LOC_BGNZ2_0020);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,ta1, LOC_BGNZ2_0021);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,q2, LOC_BGNZ2_0022);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,r2, LOC_BGNZ2_0023);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,M, LOC_BGNZ2_0024);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,t0, LOC_BGNZ2_0025);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,t1, LOC_BGNZ2_0026);
#endif/* STATIC_MEMORY_SWITCH */

    /* sys_log(LOGSTDOUT,"bgn_z2_ddiv_slow: counter = %ld\n",counter); */

    return ;
}

/**
*   let a = (a1,a0) = a1 * x ^ {BIGINTSIZE} + a0 over Z_2[x] and a be nonzero
*   let q = (q1,q0) = q1 * x ^ {BIGINTSIZE} + q0 over Z_2[x] and q be nonzero
*
*   a = b * q + r over Z_2[x] with deg( r ) < deg(b)
*
*   Algorithm:
*   Input: a = (a1,a0), b belong to Z_2[x] and assume a1 is nonzero and b is nonzero
*   Output: q = (q1,q0) and r, where a = b * q + r over Z_2[x]
*   1. q = 0
*   2. while a1 is nonzero 0 do
*   2.1     delta = deg(a) - deg(b)
*           note: deg(a) = deg(a1) + BIGINTSIZE
*   2.2     a = a - b * x ^ {delta} over Z_2[x]
*   2.3     q = q + x ^ {delta}  over Z_2[x]
*   2.4     next while
*   3. let a0 = b * q2 + r2  over Z_2[x]
*       a = q * b + a0 = ( q + q2 ) * b + r2  over Z_2[x]
*   4. q = q + q2  over Z_2[x]
*   5. r = r2
*   6. return (q, r)
**/
void bgn_z2_ddiv_2(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/* STACK_MEMORY_SWITCH */
    BIGINT *ta0;
    BIGINT *ta1;
    BIGINT *tb0;
    BIGINT *tb1;
    BIGINT *t0;
    BIGINT *t1;
    UINT32 ta1_deg;
    UINT32 ta_deg;
    UINT32 b_deg;
    UINT32 diff_deg;
    UINT32 nthbit;
    /* UINT32 counter; */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: q0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    if ( NULL_PTR == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: q1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( b == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of b and q0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of b and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of q0 and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of q0 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q1 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: address of q1 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_ddiv_2: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    /* if b = 0, then exit */
    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, b ) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv_2: b is zero.\n");

        dbg_exit(MD_BGNZ2, bgnz2_md_id) ;
    }

    /* a = 1 * a + 0 */
    if ( EC_TRUE == bgn_z2_is_one( bgnz2_md_id, b ) )
    {
        bgn_z2_clone(bgnz2_md_id, a0, q0);

        bgn_z2_clone(bgnz2_md_id, a1, q1);
        bgn_z2_set_zero( bgnz2_md_id, r );

        return ;
    }

    if ( EC_TRUE == bgn_z2_is_zero( bgnz2_md_id, a1 ) )
    {
        bgn_z2_set_zero( bgnz2_md_id, q1 );
        bgn_z2_div(bgnz2_md_id, a0, b, q0, r);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta0 = &buf_0;
    ta1 = &buf_1;
    tb0 = &buf_2;
    tb1 = &buf_3;
    t0  = &buf_4;
    t1  = &buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&ta0, LOC_BGNZ2_0027);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&ta1, LOC_BGNZ2_0028);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&tb0, LOC_BGNZ2_0029);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&tb1, LOC_BGNZ2_0030);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&t0, LOC_BGNZ2_0031);
    alloc_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,&t1, LOC_BGNZ2_0032);
#endif/* STATIC_MEMORY_SWITCH */

    /* here a >0 ( since a1 > 0) and b > 1 */
    bgn_z2_clone(bgnz2_md_id, a0, ta0);
    bgn_z2_clone(bgnz2_md_id, a1, ta1);
    bgn_z2_clone(bgnz2_md_id, b,  tb0);
    bgn_z2_set_zero( bgnz2_md_id, tb1 );

    b_deg  = bgn_z2_deg( bgnz2_md_id, b );

    bgn_z2_set_zero( bgnz2_md_id, q0 );
    bgn_z2_set_zero( bgnz2_md_id, q1 );

    /* counter = 0; */
    while( EC_FALSE == bgn_z2_is_zero( bgnz2_md_id, ta1 ) )
    {
        /* counter ++; */

        ta1_deg = bgn_z2_deg( bgnz2_md_id, ta1 );
        ta_deg = ta1_deg + BIGINTSIZE;

        diff_deg = ta_deg - b_deg;

        /* a = a - b * x ^ diff_deg */
        bgn_z2_dshl_no_shift_out(bgnz2_md_id, tb0, tb1, diff_deg, t0, t1);
        bgn_z2_sub(bgnz2_md_id, ta0, t0, ta0);
        bgn_z2_sub(bgnz2_md_id, ta1, t1, ta1);

        /* q = q + x ^ diff_deg */
        if ( diff_deg < BIGINTSIZE )
        {

            nthbit = diff_deg;
            if ( 1 == bgn_z2_get_bit(bgnz2_md_id, q0, nthbit) )
            {
                bgn_z2_clear_bit(bgnz2_md_id, q0, nthbit);
            }
            else
            {
                bgn_z2_set_bit(bgnz2_md_id, q0, nthbit);
            }
        }
        else
        {
            nthbit = diff_deg - BIGINTSIZE;
            if ( 1 == bgn_z2_get_bit(bgnz2_md_id, q1, nthbit) )
            {
                bgn_z2_clear_bit(bgnz2_md_id, q1, nthbit);
            }
            else
            {
                bgn_z2_set_bit(bgnz2_md_id, q1, nthbit);
            }
        }
    }

    /* here, a = b * q + ta0 */
    /* let ta0 = b * t0 + t1 */
    /* then q = q + t0, r = t1 */
    bgn_z2_div(bgnz2_md_id, ta0, b, t0, t1);
    bgn_z2_dadd(bgnz2_md_id, q0, q1, t0, q0, q1);
    bgn_z2_clone(bgnz2_md_id, t1, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,ta0, LOC_BGNZ2_0033);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,ta1, LOC_BGNZ2_0034);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,tb0, LOC_BGNZ2_0035);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,tb1, LOC_BGNZ2_0036);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,t0, LOC_BGNZ2_0037);
    free_static_mem(MD_BGNZ2, bgnz2_md_id, MM_BIGINT,t1, LOC_BGNZ2_0038);
#endif/* STATIC_MEMORY_SWITCH */

    /* sys_log(LOGSTDOUT,"bgn_z2_ddiv_2: counter = %ld\n",counter); */

    return ;
}

/**
*   let a = (a1,a0) = a1 * 2 ^ BIGINTSIZE + a0 and a be nonzero
*   let q = (q1,q0) = q1 * 2 ^ BIGINTSIZE + q0 and q be nonzero
*
*   a = b * q + r over Z_2 with deg( r ) < deg(b)
**/
void bgn_z2_ddiv(const UINT32 bgnz2_md_id,const BIGINT* a0,const BIGINT* a1,const BIGINT * b,BIGINT * q0,BIGINT * q1,BIGINT *r)
{
    UINT32 b_deg;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: q0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }

    if ( NULL_PTR == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: q1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( b == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of b and q0 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of b and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of q0 and q1 conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q0 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of q0 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
    if ( q1 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z2_ddiv: address of q1 and r conflict.\n");
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZ2_MD <= bgnz2_md_id || 0 == g_bgnz2_md[ bgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z2_ddiv: bgnz2 module #0x%lx not started.\n",
                bgnz2_md_id);
        dbg_exit(MD_BGNZ2, bgnz2_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    b_deg = bgn_z2_deg( bgnz2_md_id, b );

    if ( MAX_DEG_OF_B_FOR_DDIV_1 > b_deg )
    {
        /* decrease delta = (a1 * q2) * b */
        /* where a = (a1,a0) and x^{BIGINTSIZE} = b * q2 + r2 */
        bgn_z2_ddiv_1(bgnz2_md_id, a0, a1, b, q0, q1, r);
    }
    else
    {
        /* decrease one bit each loop */
        bgn_z2_ddiv_2(bgnz2_md_id, a0, a1, b, q0, q1, r);
    }

    return ;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
