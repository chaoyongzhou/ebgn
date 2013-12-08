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

#include "log.h"

#include "bgnz.h"
#include "bgnzn.h"

#include "debug.h"

#include "print.h"



/* set bgnzn_n = n */
/* set bgnzn_bnd = 2 ^ (BIGINTSIZE - 1) */
/* set bgnzn_res = 2 ^ (BIGINTSIZE) mod n ,i.e,  bgnzn_res + n = 2 ^ (BIGINTSIZE) */

static BGNZN_MD g_bgnzn_md[ MAX_NUM_OF_BGNZN_MD ];
static EC_BOOL  g_bgnzn_md_init_flag = EC_FALSE;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
static BIGINT g_bgnzn_buf[ 2 * MAX_NUM_OF_BGNZN_MD ];
#endif/* STACK_MEMORY_SWITCH */

/**
*   for test only
*
*   to query the status of BGNZN Module
*
**/
void bgn_zn_print_module_status(const UINT32 bgnzn_md_id, LOG *log)
{
    BGNZN_MD *bgnzn_md;
    UINT32 index;

    if ( EC_FALSE == g_bgnzn_md_init_flag )
    {
        sys_log(log,"no BGNZN Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_BGNZN_MD; index ++ )
    {
        bgnzn_md = &(g_bgnzn_md[ index ]);
        /* if module was started, check if it's the ring Z_{n} */

        if ( 0 < bgnzn_md->usedcounter)
        {
            sys_log(log,"BGNZN Module # %ld : %ld refered, refer BGNZ Module : %ld\n",
                    index,
                    bgnzn_md->usedcounter,
                    bgnzn_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed BGNZN module
*
*
**/
UINT32 bgn_zn_free_module_static_mem(const UINT32 bgnzn_md_id)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_free_module_static_mem: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    free_module_static_mem(MD_BGNZN, bgnzn_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}
/**
*
* start BGNZN module
*
**/
UINT32 bgn_zn_start( const BIGINT *n )
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

    BIGINT *bgnzn_n;
    BIGINT *bgnzn_res;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnzn_md_id;
    UINT32 bgnz_md_id;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_start: n is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, ERR_MODULE_ID);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /* if this is the 1st time to start BGNZN module, then */
    /* initialize g_bgnzn_md */
    if ( EC_FALSE ==  g_bgnzn_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_BGNZN_MD; index ++ )
        {
            bgnzn_md = &(g_bgnzn_md[ index ]);

            bgnzn_md->usedcounter   = 0;
            bgnzn_md->bgnz_md_id    = ERR_MODULE_ID;
            bgnzn_md->bgnzn_n   = NULL_PTR;
            bgnzn_md->bgnzn_res = NULL_PTR;
        }

        /*register all functions of BGNZN module to DBG module*/
        //dbg_register_func_addr_list(g_bgnzn_func_addr_list, g_bgnzn_func_addr_list_len);

        g_bgnzn_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_BGNZN_MD; index ++ )
    {
        bgnzn_md = &(g_bgnzn_md[ index ]);

        if ( 0 == bgnzn_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_BGNZN_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;
    bgnzn_n = NULL_PTR;
    bgnzn_res = NULL_PTR;

    bgnzn_md_id = index;
    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &bgnzn_n  , LOC_BGNZN_0001);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &bgnzn_res, LOC_BGNZN_0002);
#endif/* STATIC_MEMORY_SWITCH */

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    bgnzn_n   = &(g_bgnzn_buf[ 2 * bgnzn_md_id + 0 ]);
    bgnzn_res = &(g_bgnzn_buf[ 2 * bgnzn_md_id + 1 ]);
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == bgnzn_n )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_start: bgnzn_n is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == bgnzn_res )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_start: bgnzn_res is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    /*now, the static memory has been initialized */
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_1;
    c1 = &buf_2;
    q0  = &buf_3;
    q1  = &buf_4;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c0, LOC_BGNZN_0003);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c1, LOC_BGNZN_0004);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q0, LOC_BGNZN_0005);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q1, LOC_BGNZN_0006);
#endif/* STATIC_MEMORY_SWITCH */

    /*start one BGNZ module*/
    bgnz_md_id = bgn_z_start();

    /* set bgnzn_n = n */
    bgn_z_clone(bgnz_md_id, n, bgnzn_n);

    /* compute res = 2 ^(BIGINTSIZE) mod n */
    /* set c = (c1,c0) = 2 ^(BIGINTSIZE), then res = c mod n */
    bgn_z_set_zero( bgnz_md_id, c0 );
    bgn_z_set_one( bgnz_md_id, c1 );
    bgn_z_ddiv(bgnz_md_id, c0, c1, n, q0, q1, bgnzn_res);

    /* set module : */
    bgnzn_md->bgnz_md_id = bgnz_md_id;
    bgnzn_md->bgnzn_n   = bgnzn_n;
    bgnzn_md->bgnzn_res = bgnzn_res;

    /* at the first time, set the counter to 1 */
    bgnzn_md->usedcounter = 1;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c0, LOC_BGNZN_0007);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c1, LOC_BGNZN_0008);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q0, LOC_BGNZN_0009);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q1, LOC_BGNZN_0010);
#endif/* STATIC_MEMORY_SWITCH */

    return ( bgnzn_md_id );
}
/**
*
* end BGNZN module
*
**/
void bgn_zn_end(const UINT32 bgnzn_md_id)
{
    BIGINT *bgnzn_n;
    BIGINT *bgnzn_res;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    if ( MAX_NUM_OF_BGNZN_MD < bgnzn_md_id )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_end: bgnzn_md_id = %ld is overflow.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < bgnzn_md->usedcounter )
    {
        bgnzn_md->usedcounter --;
        return ;
    }

    if ( 0 == bgnzn_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_end: bgnzn_md_id = %ld is not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n   = bgnzn_md->bgnzn_n;
    bgnzn_res = bgnzn_md->bgnzn_res;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, bgnzn_n  , LOC_BGNZN_0011);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, bgnzn_res, LOC_BGNZN_0012);
#endif/* STATIC_MEMORY_SWITCH */

    /*close the submodule*/
    bgn_z_end(bgnz_md_id);

    /* free module : */
    //bgn_zn_free_module_static_mem(bgnzn_md_id);
    bgnzn_md->bgnzn_n   = NULL_PTR;
    bgnzn_md->bgnzn_res = NULL_PTR;
    bgnzn_md->usedcounter = 0;

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   clone src to des
*   return des where des = src
**/
void bgn_zn_clone(const UINT32 bgnzn_md_id,const BIGINT * src,BIGINT * des)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_clone: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == des )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_clone: des is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_clone: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    bgn_z_clone(bgnz_md_id, src, des);

    return ;
}

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_zn_cmp(const UINT32 bgnzn_md_id,const BIGINT * a,const BIGINT *b)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_cmp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_cmp: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_cmp: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_cmp(bgnz_md_id, a, b);
}

/**
*
*   set a = 0
**/
void bgn_zn_set_zero(const UINT32 bgnzn_md_id,BIGINT * a)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_zero: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_set_zero: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    bgn_z_set_word( bgnz_md_id, a, 0 );

    return ;
}

/**
*
*   set a = 1
**/
void bgn_zn_set_one(const UINT32 bgnzn_md_id,BIGINT * a)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_set_one: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    bgn_z_set_word( bgnz_md_id, a, 1 );

    return;
}

/**
*
*   set a = n
**/
void bgn_zn_set_word(const UINT32 bgnzn_md_id,BIGINT *a,const UINT32 n)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_word: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_set_word: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    bgn_z_set_word( bgnz_md_id, a, n );

    return ;
}

/**
*   return e = 2 ^ nth mod n
*            = ( 1 << nth ) mod n
*   where nth = 0,1,...,{BIGINTSIZE - 1}
*
**/
void bgn_zn_set_e(const UINT32 bgnzn_md_id,BIGINT *e,const UINT32 nth)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *t;
    BIGINT *q;

    BGNZN_MD *bgnzn_md;
    BIGINT   *bgnzn_n;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_e: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( BIGINTSIZE <= nth )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_e: nth = %ld is overflow.\n", nth);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_set_e: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t = &buf_1;
    q = &buf_2;
#endif/*STACK_MEMORY_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&t, LOC_BGNZN_0013);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q, LOC_BGNZN_0014);
#endif/* STATIC_MEMORY_SWITCH */

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n  = bgnzn_md->bgnzn_n;

    bgn_z_set_e(bgnz_md_id, t, nth);
    bgn_z_div(bgnz_md_id, t, bgnzn_n, q, e);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,t, LOC_BGNZN_0015);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q, LOC_BGNZN_0016);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   set a =  2^ {BIGINTSIZE} - 1 mod n
**/
void bgn_zn_set_max(const UINT32 bgnzn_md_id,BIGINT * a)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/*STACK_MEMORY_SWITCH*/

    BIGINT *t;
    BIGINT *q;

    BGNZN_MD *bgnzn_md;
    BIGINT   *bgnzn_n;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_set_max: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_set_max: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t = &buf_1;
    q = &buf_2;
#endif/*STACK_MEMORY_SWITCH*/

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&t, LOC_BGNZN_0017);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q, LOC_BGNZN_0018);
#endif/* STATIC_MEMORY_SWITCH */

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n  = bgnzn_md->bgnzn_n;

    bgn_z_set_max(bgnz_md_id, t );
    bgn_z_div(bgnz_md_id, t, bgnzn_n, q, a);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,t, LOC_BGNZN_0019);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q, LOC_BGNZN_0020);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_zero(const UINT32 bgnzn_md_id,const BIGINT* src)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_is_zero: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_is_zero: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_is_zero( bgnz_md_id, src );
}

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_one(const UINT32 bgnzn_md_id,const BIGINT* src)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_is_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_is_one: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_is_one( bgnz_md_id, src );
}

/**
*
*   if src = n, then return EC_TRUE
*   if src !=n, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_n(const UINT32 bgnzn_md_id,const BIGINT* src, const UINT32 n)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_is_n: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_is_n: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_is_n( bgnz_md_id, src, n );
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_zn_is_odd(const UINT32 bgnzn_md_id,const BIGINT *src)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_is_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_is_one: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_is_odd( bgnz_md_id, src );
}

/**
*   let a belong to [0, n - 1], then
*       c = a / 2^{nbits} mod n
*   where 0 <= nbits < BIGINTSIZE
*   return c
*
*   maybe address of c = address of a
**/
EC_BOOL bgn_zn_shr_lessbgnsize(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 nbits,BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *e;
    BIGINT *e_inv;

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shr_lessbgnsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shr_lessbgnsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( BIGINTSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shr_lessbgnsize: nbits = %ld overflow.\n",nbits);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_shr_lessbgnsize: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

    /* if n is even, the gcd(2^{WORDSIZE},n) >=2, */
    /* so that, 1/ 2^{WORDSIZE} mod n does not exist */
    if ( EC_FALSE == bgn_z_is_odd(bgnz_md_id, bgnzn_n))
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shr_lessbgnsize: n is even, this function does not work\n");
        return (EC_FALSE);
    }

    if ( 0 == nbits )
    {
        bgn_z_clone(bgnz_md_id, a, c);
        return (EC_TRUE);
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    e     = &buf_1;
    e_inv = &buf_2;
#endif/* STACK_MEMORY_SWITCH */
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&e, LOC_BGNZN_0021);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&e_inv, LOC_BGNZN_0022);
#endif/* STATIC_MEMORY_SWITCH */

    /* set e = 2 ^ {nbits}*/
    bgn_z_set_e(bgnz_md_id, e, nbits);

    /* e_inv = 1/e mod n */
    bgn_zn_inv(bgnzn_md_id, e, e_inv);

    /* c = a / 2^{nbits} mod n = a * e_inv mod n */
    bgn_zn_mul(bgnzn_md_id, a, e_inv, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,e, LOC_BGNZN_0023);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,e_inv, LOC_BGNZN_0024);
#endif/* STATIC_MEMORY_SWITCH */

    return (EC_TRUE);
}

/**
*   let a belong to [0, n - 1], then
*       c = ( a << WORDSIZE ) mod n
*   return c
*
**/
void bgn_zn_shl_onewordsize(const UINT32 bgnzn_md_id,const BIGINT * a, BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */

    UINT32 shift_out;
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shl_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shl_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_shl_onewordsize: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_1;
    c1 = &buf_2;
    q0 = &buf_3;
    q1 = &buf_4;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c0, LOC_BGNZN_0025);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c1, LOC_BGNZN_0026);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q0, LOC_BGNZN_0027);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q1, LOC_BGNZN_0028);
#endif/* STATIC_MEMORY_SWITCH */

    /* let (c1,c0) = (a << WORDSIZE) */
    /* then c = (c1,c0) mod n*/
    shift_out = bgn_z_shl_onewordsize(bgnz_md_id,a,c0 );
    if ( 0 < shift_out )
    {
        bgn_z_set_word(bgnz_md_id,c1, shift_out);
        bgn_z_ddiv(bgnz_md_id,c0, c1, bgnzn_n, q0, q1, c);
    }
    else
    {
        bgn_z_div(bgnz_md_id,c0, bgnzn_n, q0, c);
    }


#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c0, LOC_BGNZN_0029);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c1, LOC_BGNZN_0030);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q0, LOC_BGNZN_0031);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q1, LOC_BGNZN_0032);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   let a belong to [0, n - 1], then
*       c = ( a << nbits ) mod n
*   return c
*
**/
void bgn_zn_shl_lesswordsize(const UINT32 bgnzn_md_id,const BIGINT * a, const UINT32 nbits, BIGINT * c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif/* STACK_MEMORY_SWITCH */

    UINT32 shift_out;
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shl_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_shl_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_shl_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_shl_lesswordsize: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_1;
    c1 = &buf_2;
    q0 = &buf_3;
    q1 = &buf_4;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c0, LOC_BGNZN_0033);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&c1, LOC_BGNZN_0034);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q0, LOC_BGNZN_0035);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&q1, LOC_BGNZN_0036);
#endif/* STATIC_MEMORY_SWITCH */

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

    /* let (c1,c0) = (a << WORDSIZE) */
    /* then c = (c1,c0) mod n*/
    shift_out = bgn_z_shl_lesswordsize(bgnz_md_id, a, nbits, c0);
    if ( 0 < shift_out )
    {
        bgn_z_set_word(bgnz_md_id, c1, shift_out);
        bgn_z_ddiv(bgnz_md_id, c0, c1, bgnzn_n, q0, q1, c);
    }
    else
    {
        bgn_z_div(bgnz_md_id, c0, bgnzn_n, q0, c);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c0, LOC_BGNZN_0037);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,c1, LOC_BGNZN_0038);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q0, LOC_BGNZN_0039);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,q1, LOC_BGNZN_0040);
#endif/* STATIC_MEMORY_SWITCH */

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
int bgn_zn_naf(const UINT32 bgnzn_md_id,const BIGINT *k,int *s)
{
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_naf: k is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == s )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_naf: s is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_naf: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;

    return bgn_z_naf(bgnz_md_id, k, s);
}

/**
*       c = ( a + b ) mod n
*       where a < n and b < n
*   Algorithm:
*   Input: a,b where a < n and b < n
*   Output: c such that c = ( a + b ) mod n
*   1.  (carry, c) = a + b
*   2.  if carry > 0, which means n >= 2 ^ (BIGINTSIZE -1) + 1, and now
*           a + b = c + 2 ^(BIGINTSIZE)
*       since a < n and b < n, so that a + b < 2n,furthermore a + b - n = c + (2 ^(BIGINTSIZE) - n) < n
*       then do
*           c = c + res < n
*       where res = 2 ^ (BIGINTSIZE) mod n
*   3.  else if c >= n then do
*           c = c - n
*   4.  return c
**/
void bgn_zn_add(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    BIGINT *bgnzn_n;
    BIGINT *bgnzn_res;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;
    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;
    bgnzn_res = bgnzn_md->bgnzn_res;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( 0 <= bgn_z_cmp(bgnz_md_id, b, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_add: b >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    carry = bgn_z_add( bgnz_md_id, a, b, c );
    if ( 0 < carry )
    {
        bgn_z_add(bgnz_md_id, c, bgnzn_res, c);

        return ;
    }

    if ( 0 <= bgn_z_cmp(bgnz_md_id, c, bgnzn_n) )
    {
        bgn_z_sub(bgnz_md_id, c, bgnzn_n, c);
    }

    return ;
}

/**
*       c = ( a - b ) mod n
*       where a < n and b < n
*   Algorithm:(ERROR!)
*   Input: a,b where a < n and b < n
*   Output: c such that c = ( a - b ) mod n
*   1.  (borrow, c) = a - b
*   2.  if borrow > 0, which means a < b and now
*           c = 2 ^ (BIGINTSIZE) + a - b
*   xxx : error : here maybe c >> n and then c - res > n
*       then do
*           c = c - res
*       where res = 2 ^ (BIGINTSIZE) mod n
*   3.  return c
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
void bgn_zn_sub(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *t;

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( 0 <= bgn_z_cmp(bgnz_md_id, b, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sub: b >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( 0 <= bgn_z_cmp( bgnz_md_id, a, b ) )
    {
        bgn_z_sub(bgnz_md_id, a, b, c);

        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&t, LOC_BGNZN_0041);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_sub(bgnz_md_id, bgnzn_n, b, t);
    bgn_z_add(bgnz_md_id, a, t, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,t, LOC_BGNZN_0042);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*       c = ( -a ) mod n
*       where a < n
*
**/
void bgn_zn_neg(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c )
{
    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_neg: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_neg: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_neg: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_neg: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id, a ) )
    {
        bgn_z_set_zero( bgnz_md_id, c );
        return ;
    }

    bgn_z_sub(bgnz_md_id, bgnzn_n, a, c);

    return ;
}

/**
*       c = ( a * b ) mod n
*       where a < n and b < n
*   Algorithm:
*   Input: a,b where a < n and b < n
*   Output: c such that c = ( a * b ) mod n
*   1.  let a * b = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = n * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_zn_mul(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif /* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( 0 <= bgn_z_cmp(bgnz_md_id, b, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_mul: b >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_0;
    c1 = &buf_1;
    q0 = &buf_2;
    q1 = &buf_3;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c0, LOC_BGNZN_0043);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c1, LOC_BGNZN_0044);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q0, LOC_BGNZN_0045);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q1, LOC_BGNZN_0046);
#endif /* STATIC_MEMORY_SWITCH */

    bgn_z_mul( bgnz_md_id, a, b, c0, c1 );
    bgn_z_ddiv(bgnz_md_id, c0, c1, bgnzn_n, q0, q1, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c0, LOC_BGNZN_0047);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c1, LOC_BGNZN_0048);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q0, LOC_BGNZN_0049);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q1, LOC_BGNZN_0050);
#endif /* STATIC_MEMORY_SWITCH s*/

    return ;
}

/**
*       c = ( a * b ) mod n
*       where a < n and b < n
*   Algorithm:
*   Input: a,b where a < n and b < n
*   Output: c such that c = ( a * b ) mod n
*   1.  let a * b = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = n * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_zn_smul(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 b,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif /* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( 0 == bgn_z_get_len(bgnz_md_id, bgnzn_n) && 0 < b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: b >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( 1 == bgn_z_get_len(bgnz_md_id, bgnzn_n) && bgnzn_n->data[ 0 ] <= b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_smul: b >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_0;
    c1 = &buf_1;
    q0 = &buf_2;
    q1 = &buf_3;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c0, LOC_BGNZN_0051);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c1, LOC_BGNZN_0052);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q0, LOC_BGNZN_0053);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q1, LOC_BGNZN_0054);
#endif /* STATIC_MEMORY_SWITCH */

    bgn_z_smul( bgnz_md_id, a, b, c0, c1 );
    bgn_z_ddiv(bgnz_md_id, c0, c1, bgnzn_n, q0, q1, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c0, LOC_BGNZN_0055);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c1, LOC_BGNZN_0056);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q0, LOC_BGNZN_0057);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q1, LOC_BGNZN_0058);
#endif /* STATIC_MEMORY_SWITCH s*/

    return ;
}

/**
*       c = ( a ^ 2 ) mod n
*       where a < n
*   Algorithm:
*   Input: a where a < n
*   Output: c such that c = ( a ^ 2 ) mod n
*   1.  let a ^ 2 = (c1,c0) = c1 * 2 ^ BIGINTSIZE + c0
*   2.  let (c1,c0) = n * q + r,where q = (q1,q0)
*   3.  set c = r
*   4.  return c
*
**/
void bgn_zn_squ(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_0;
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif /* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_squ: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_squ: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_squ: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_squ: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    c0 = &buf_0;
    c1 = &buf_1;
    q0 = &buf_2;
    q1 = &buf_3;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c0, LOC_BGNZN_0059);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &c1, LOC_BGNZN_0060);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q0, LOC_BGNZN_0061);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &q1, LOC_BGNZN_0062);
#endif /* STATIC_MEMORY_SWITCH */

    bgn_z_squ( bgnz_md_id, a, c0, c1 );
    bgn_z_ddiv(bgnz_md_id, c0, c1, bgnzn_n, q0, q1, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c0, LOC_BGNZN_0063);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, c1, LOC_BGNZN_0064);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q0, LOC_BGNZN_0065);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, q1, LOC_BGNZN_0066);
#endif /* STATIC_MEMORY_SWITCH s*/

    return ;
}

/**
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ WORDSIZE
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e, where 0 < a < n and e < 2 * WORDSIZE
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
void bgn_zn_sexp(const UINT32 bgnzn_md_id,const BIGINT *a,const UINT32 e,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif /* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *ta;
    UINT32 te;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sexp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sexp: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sexp: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_sexp: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id, a ) )
    {
        bgn_z_set_zero( bgnz_md_id, c );
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &ta, LOC_BGNZN_0067);
#endif /* STATIC_MEMORY_SWITCH */


    te = e;
    bgn_z_clone(bgnz_md_id, a, ta);
    bgn_z_set_one( bgnz_md_id, c );

    while ( 1 )
    {
        if ( te & 1 )
        {
            bgn_zn_mul(bgnzn_md_id, c, ta, c);
        }
        te >>= 1;

        if ( 0 == te )
        {
            break;
        }

        bgn_zn_squ(bgnzn_md_id, ta, ta);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, ta, LOC_BGNZN_0068);
#endif /* STATIC_MEMORY_SWITCH */

    return ;
}


/**
*       c = ( a ^ e ) mod n
*       where 0 < a < n and e < 2 ^ BIGINTSIZE
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e,where 0 < a < n
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
void bgn_zn_exp(const UINT32 bgnzn_md_id,const BIGINT *a,const BIGINT *e,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *ta;
    UINT32 te_nbits;
    UINT32 e_bit;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_exp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_exp: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_exp: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_exp: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_exp: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id, a ) )
    {
        bgn_z_set_zero( bgnz_md_id, c );
        return ;
    }

    if ( EC_TRUE == bgn_z_is_one( bgnz_md_id, a )
      || EC_TRUE == bgn_z_is_zero(bgnz_md_id, e) )
    {
        bgn_z_set_one( bgnz_md_id, c );
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&ta, LOC_BGNZN_0069);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_clone(bgnz_md_id, a, ta);

    te_nbits = bgn_z_get_nbits( bgnz_md_id, e );

    bgn_z_set_one( bgnz_md_id, c );
    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, e, index);
        if ( 1 == e_bit )
        {
            bgn_zn_mul(bgnzn_md_id, c, ta, c);
        }
        index ++;
        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= te_nbits )
        {
            break;
        }
        bgn_zn_squ(bgnzn_md_id, ta, ta);
    }

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,ta, LOC_BGNZN_0070);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   if a = 0 , then return EC_FALSE
*   if n > a > 0 and GCD(a,n) > 1, then return EC_FALSE
*   if n > a > 0 and GCD(a,n) = 1, then return EC_TRUE and
*       c = ( 1 / a ) mod n
*   where 0 < a < n
*
*   Algorithm:
*   Input: a where 0 < a < n
*   output: c where c =  a ^ {-1} mod n
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
EC_BOOL bgn_zn_inv(const UINT32 bgnzn_md_id,const BIGINT *a,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *bgnzn_n;
    BGNZN_MD *bgnzn_md;
    UINT32 bgnz_md_id;

    BIGINT *u;
    BIGINT *v;
    BIGINT *A;
    BIGINT *C;
    BIGINT *p;
    BIGINT *d;

    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_inv: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_inv: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_inv: bgnzn module #0x%lx not started.\n",bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnzn_md = &(g_bgnzn_md[ bgnzn_md_id ]);
    bgnz_md_id = bgnzn_md->bgnz_md_id;
    bgnzn_n = bgnzn_md->bgnzn_n;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( 0 <= bgn_z_cmp(bgnz_md_id, a, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_inv: a >= n.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }

    if ( EC_FALSE == bgn_z_is_odd(bgnz_md_id, bgnzn_n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_inv: n is even.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    d = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&d, LOC_BGNZN_0071);
#endif/* STATIC_MEMORY_SWITCH */

    /* let d = GCD(a,n), if d > 1, then return EC_FALSE */
    bgn_z_gcd(bgnz_md_id, a, bgnzn_n, d);
    if ( EC_FALSE == bgn_z_is_one( bgnz_md_id, d ) )
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,d, LOC_BGNZN_0072);
#endif/* STATIC_MEMORY_SWITCH */

        return EC_FALSE;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    u = &buf_2;
    v = &buf_3;
    A = &buf_4;
    C = &buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&u, LOC_BGNZN_0073);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&v, LOC_BGNZN_0074);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&A, LOC_BGNZN_0075);
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,&C, LOC_BGNZN_0076);
#endif/* STATIC_MEMORY_SWITCH */

    p = bgnzn_n;

    /* u = a, v = p,A = 1,C = 0 */
    bgn_z_clone( bgnz_md_id, a, u );
    bgn_z_clone( bgnz_md_id, p, v );
    bgn_z_set_one( bgnz_md_id, A );
    bgn_z_set_zero( bgnz_md_id, C );

    while( EC_FALSE == bgn_z_is_zero( bgnz_md_id, u ) )
    {
        while( EC_FALSE == bgn_z_is_odd( bgnz_md_id, u ) )
        {
            bgn_z_shr_lesswordsize( bgnz_md_id, u, 1, u );

            if( EC_TRUE == bgn_z_is_odd( bgnz_md_id, A ) )
            {
                /* A <--- (A + p)/2 */
                carry = bgn_z_add( bgnz_md_id, A, p, A );
                bgn_z_shr_lesswordsize( bgnz_md_id, A, 1, A );
                if ( 0 < carry )
                {
                    bgn_z_set_bit(bgnz_md_id, A, BIGINTSIZE - 1);
                }
            }
            else
            {
                /* A <--- A/2 */
                bgn_z_shr_lesswordsize( bgnz_md_id, A, 1, A );
            }
        }

        while( EC_FALSE == bgn_z_is_odd( bgnz_md_id, v ) )
        {
            bgn_z_shr_lesswordsize( bgnz_md_id, v, 1, v );

            if( EC_TRUE == bgn_z_is_odd( bgnz_md_id, C ) )
            {
                /* C <--- (C + p)/2 */
                carry = bgn_z_add( bgnz_md_id, C, p, C );
                bgn_z_shr_lesswordsize( bgnz_md_id, C, 1, C );
                if ( 0 < carry )
                {
                    bgn_z_set_bit(bgnz_md_id, C, BIGINTSIZE - 1);
                }
            }
            else
            {
                /* C <--- C/2 */
                bgn_z_shr_lesswordsize( bgnz_md_id, C, 1, C );
            }
        }

        if( bgn_z_cmp( bgnz_md_id, u, v ) >= 0 )
        {
            /* u <-- u - v */
            bgn_z_sub( bgnz_md_id, u, v, u );

            /* if A < C, then do */
            if ( bgn_z_cmp( bgnz_md_id, A, C ) < 0 )
            {
                /* A <-- A + p - C */
                bgn_z_sub(bgnz_md_id, p, C, d);
                bgn_z_add(bgnz_md_id, A, d, A );
            }
            /* if A >= C, then do */
            else
            {
                /* A <-- A - C */
                bgn_z_sub( bgnz_md_id, A, C, A );

            }
        }
        else
        {
            /* v <-- v - u */
            bgn_z_sub( bgnz_md_id, v, u, v );

            /* if C < A, then do */
            if ( bgn_z_cmp( bgnz_md_id, C, A ) < 0 )
            {
                /* C <-- C + p - A */
                bgn_z_sub(bgnz_md_id, p, A, d);
                bgn_z_add(bgnz_md_id, C, d, C );
            }
            /* if C >= A, then do */
            else
            {
                /* C <-- C - A */
                bgn_z_sub( bgnz_md_id, C, A, C );
            }
        }
    }
#if 0
    sys_log(LOGSTDOUT,"u := ");print_bigint_dec(LOGSTDOUT, u);
    sys_log(LOGSTDOUT,"v := ");print_bigint_dec(LOGSTDOUT, v);
    sys_log(LOGSTDOUT,"A := ");print_bigint_dec(LOGSTDOUT, A);
    sys_log(LOGSTDOUT,"C := ");print_bigint_dec(LOGSTDOUT, C);
#endif
    bgn_z_clone( bgnz_md_id, C, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,u, LOC_BGNZN_0077);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,v, LOC_BGNZN_0078);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,A, LOC_BGNZN_0079);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,C, LOC_BGNZN_0080);
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT,d, LOC_BGNZN_0081);
#endif/* STATIC_MEMORY_SWITCH */

    return EC_TRUE;
}

/**
*
*   c = a / b mod n
*   if GCD(b,n) > 1, then return FALSE
*   else return c = a *(b^(-1)) mod n
*
**/
EC_BOOL bgn_zn_div(const UINT32 bgnzn_md_id,const BIGINT *a, const BIGINT *b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/*STACK_MEMORY_SWITCH*/
    BIGINT *b_inv;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_div: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_div: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_zn_div: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( MAX_NUM_OF_BGNZN_MD <= bgnzn_md_id || 0 == g_bgnzn_md[ bgnzn_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_zn_div: bgnzn module #0x%lx not started.\n",
                bgnzn_md_id);
        dbg_exit(MD_BGNZN, bgnzn_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    b_inv = &buf_1;
#endif/*STACK_MEMORY_SWITCH*/
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, &b_inv, LOC_BGNZN_0082);
#endif /* STATIC_MEMORY_SWITCH */

    if ( EC_FALSE == bgn_zn_inv(bgnzn_md_id, b, b_inv))
    {
#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
        free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, b_inv, LOC_BGNZN_0083);
#endif /* STATIC_MEMORY_SWITCH */

        return (EC_FALSE);
    }

    bgn_zn_mul(bgnzn_md_id, a, b_inv, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZN, bgnzn_md_id, MM_BIGINT, b_inv, LOC_BGNZN_0084);
#endif /* STATIC_MEMORY_SWITCH */

    return (EC_TRUE);
}
#ifdef __cplusplus
}
#endif/*__cplusplus*/
