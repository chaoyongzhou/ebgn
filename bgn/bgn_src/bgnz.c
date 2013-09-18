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

#include "mm.h"

#include "bgnz.h"

#include "debug.h"

#include "print.h"

#include "task.h"

#include "findex.inc"

#include "cbc.h"

#define BGNZ_MD_CAPACITY()                  (cbc_md_capacity(MD_BGNZ))

#define BGNZ_MD_GET(bgnz_md_id)       ((BGNZ_MD *)cbc_md_get(MD_BGNZ, (bgnz_md_id)))

#define BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id)  \
    (NULL_PTR == BGNZ_MD_GET(bgnz_md_id) || 0 == (BGNZ_MD_GET(bgnz_md_id)->usedcounter) )

/**
*   for test only
*
*   to query the status of BGNZ Module
*
**/
void bgn_z_print_module_status(const UINT32 bgnz_md_id, LOG *log)
{
    BGNZ_MD *bgnz_md;
    UINT32 this_bgnz_md_id;
    for( this_bgnz_md_id = 0; this_bgnz_md_id < BGNZ_MD_CAPACITY(); this_bgnz_md_id ++ )
    {
        bgnz_md = BGNZ_MD_GET(this_bgnz_md_id);
        if ( NULL_PTR != bgnz_md && 0 < bgnz_md->usedcounter )
        {
            sys_log(log,"BGNZ Module # %ld : %ld refered\n",
                        this_bgnz_md_id,
                        bgnz_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed BGNZ module
*
*
**/
UINT32 bgn_z_free_module_static_mem(const UINT32 bgnz_md_id)
{
    /*TODO:*/
    free_module_static_mem(MD_BGNZ, bgnz_md_id);

    return 0;
}

/**
*
* start bgn_z module
*
**/
UINT32 bgn_z_start()
{
    BGNZ_MD *bgnz_md;
    UINT32 bgnz_md_id;

    bgnz_md_id = cbc_md_new(MD_BGNZ, sizeof(BGNZ_MD));
    if(ERR_MODULE_ID == bgnz_md_id)
    {
        return (ERR_MODULE_ID);
    }

    bgnz_md = (BGNZ_MD *)cbc_md_get(MD_BGNZ, bgnz_md_id);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    init_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    /*default setting which will be override after vector_r_set_mod_mgr calling*/
    bgnz_md->mod_mgr = mod_mgr_new(bgnz_md_id, LOAD_BALANCING_LOOP);
    bgnz_md->usedcounter = 1;

    sys_log(LOGSTDOUT, "bgn_z_start: start BGNZ module #%ld\n", bgnz_md_id);

    return ( bgnz_md_id );
}

/**
*
* end bgn_z module
*
**/
void bgn_z_end(const UINT32 bgnz_md_id)
{
    BGNZ_MD *bgnz_md;

    bgnz_md = BGNZ_MD_GET(bgnz_md_id);
    if(NULL_PTR == bgnz_md)
    {
        sys_log(LOGSTDOUT,"error:bgn_z_end: bgnz_md_id = %ld not exist.\n", bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < bgnz_md->usedcounter )
    {
        bgnz_md->usedcounter --;
        return ;
    }

    if ( 0 == bgnz_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_end: bgnz_md_id = %ld is not started.\n", bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    //task_brd_mod_mgr_rmv(bgnz_md->task_brd, bgnz_md->mod_mgr);
    mod_mgr_free(bgnz_md->mod_mgr);
    bgnz_md->mod_mgr  = NULL_PTR;
    /* if nobody else occupied the module,then free its resource */

    /* free module : */
    //bgn_z_free_module_static_mem(bgnz_md_id);
    bgnz_md->usedcounter = 0;

    cbc_md_free(MD_BGNZ, bgnz_md_id);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    breathing_static_mem();
#endif/* STATIC_MEMORY_SWITCH */

    sys_log(LOGSTDOUT, "bgn_z_end: stop BGNZ module #%ld\n", bgnz_md_id);

    return ;
}

/**
*
* initialize mod mgr of BGNZ module
*
**/
UINT32 bgn_z_set_mod_mgr(const UINT32 bgnz_md_id, const MOD_MGR * src_mod_mgr)
{
    BGNZ_MD *bgnz_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_mod_mgr: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgnz_md = BGNZ_MD_GET(bgnz_md_id);
    des_mod_mgr = bgnz_md->mod_mgr;

    mod_mgr_limited_clone(bgnz_md_id, src_mod_mgr, des_mod_mgr);

    return (0);
}

/**
*
* get mod mgr of BGNZ module
*
**/
MOD_MGR * bgn_z_get_mod_mgr(const UINT32 bgnz_md_id)
{
    BGNZ_MD *bgnz_md;

    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        return (MOD_MGR *)0;
    }

    bgnz_md = BGNZ_MD_GET(bgnz_md_id);
    return (bgnz_md->mod_mgr);
}

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int bgn_z_cmp(const UINT32 bgnz_md_id,const BIGINT * a,const BIGINT *b)
{
    UINT32 a_len;
    UINT32 b_len;
    UINT32 i;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_cmp: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_cmp: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_cmp: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_cmp: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_cmp: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    a_len = a->len;
    b_len = b->len;

    if ( a_len > b_len )
    {
        return ( 1 );
    }
    if ( a_len < b_len )
    {

        return ( -1 );
    }

    for(i = a_len ; i > 0; )
    {
        i -- ;
        if( a->data[ i ] > b->data[ i ] )
        {
            return ( 1 );
        }
        else if( a->data[ i ] < b->data[ i ] )
        {
            return ( -1 );
        }
    }

    return ( 0 );
}
/**
*   clone src to des
*   return des where des = src
**/
void bgn_z_clone(const UINT32 bgnz_md_id,const BIGINT * src,BIGINT * des)
{
    UINT32 src_len;
    UINT32 index;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_clone: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == des )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_clone: des is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_clone: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_clone: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if( des != src )
    {
        src_len = src->len;
        des->len = src_len;

        for ( index = 0; index < src_len; index ++ )
        {
            des->data[ index ] = src->data[ index ];
        }
    }
    return ;
}

/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_zero(const UINT32 bgnz_md_id,const BIGINT* src)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_is_zero: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_zero: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_zero: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( 0 == src->len )
    {
        return EC_TRUE;
    }

    if ( ( 1 == src->len ) && ( 0 == src->data[ 0 ] ) )
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}
/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_one(const UINT32 bgnz_md_id,const BIGINT* src)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_is_one: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_one: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_one: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( ( 1 == src->len ) && ( 1 == src->data[ 0 ] ) )
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   if src = n, then return EC_TRUE
*   if src !=n, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_n(const UINT32 bgnz_md_id,const BIGINT* src, const UINT32 n)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_is_n: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_n: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_n: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( 0 == n )
    {
        if ( 0 == src->len )
        {

            return EC_TRUE;
        }
        return EC_FALSE;
    }

    if ( ( 1 == src->len ) && (n == src->data[ 0 ] ) )
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL bgn_z_is_odd(const UINT32 bgnz_md_id,const BIGINT *src)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_is_odd: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_odd: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_is_odd: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( ( 0 < src->len ) && ( 1 == ( src->data[0] & 1 ) ))
    {
        return EC_TRUE;
    }

    return EC_FALSE;
}

/**
*
*   if src is short number,i.e, src < 2^32, then return EC_TRUE and src
*   if src is not short number,i.e, src >= 2^32, then return EC_FALSE only.
*
**/
EC_BOOL bgn_z_get_word(const UINT32 bgnz_md_id,const BIGINT *src, UINT32 *n)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_get_word: src is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_get_word: n is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_word: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < src->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_word: src->len = %lx overflow.\n",
                src->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if (  1 < src->len )
    {
        return EC_FALSE;
    }
    if ( 0 == src->len )
    {
        ( *n ) = 0;
    }
    else
    {
        ( *n ) = src->data[ 0 ];
    }
    return EC_TRUE;
}

/**
*   if a = 0, then return 0
*   if a > 0, then
*       let M = 2 ^ WORDSIZE
*       let a = SUM(a_i * M ^ i, i = 0..n, where a_n > 0 )
*   return ( n + 1 )
*
**/
UINT32 bgn_z_get_len(const UINT32 bgnz_md_id,const BIGINT *a)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_get_len: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_len: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_len: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    return ( a->len );
}

/**
*
*   set a = n
**/
void bgn_z_set_word(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 n)
{
    UINT32 index;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_word: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_word: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    for ( index = INTMAX; index > 0; )
    {
        index --;
        a->data[ index ] = 0;
    }

    if ( 0 == n )
    {
        a->len = 0;
        a->data[ 0 ] = 0;
    }
    else
    {
        a->len = 1;
        a->data[ 0 ] = n;
    }
    return ;
}

/**
*
*   set a = 0
**/
void bgn_z_set_zero(const UINT32 bgnz_md_id,BIGINT * a)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_zero: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_zero: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z_set_word(bgnz_md_id,a, 0);

    return ;
}
/**
*
*   set a = 1
**/
void bgn_z_set_one(const UINT32 bgnz_md_id,BIGINT * a)
{
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_one: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_one: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z_set_word(bgnz_md_id,a, 1);

    return ;
}

/**
*
*   set a =  2^ BIGINTSIZE - 1
**/
void bgn_z_set_max(const UINT32 bgnz_md_id,BIGINT * a)
{
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_max: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_max: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    for ( index = 0; index < INTMAX; index ++ )
    {
        a->data[ index ] = ( ~( ( UINT32 ) 0 ) );
    }
    a->len = INTMAX;

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
UINT32 bgn_z_parse_bits(const UINT32 bgnz_md_id,const BIGINT *k,UINT32 *s)
{
    UINT32 len;
    UINT32 t;
    UINT32 i;
    UINT32 j;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_parse_bits: k is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == s )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_parse_bits: s is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_parse_bits: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < k->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_parse_bits: k->len = %lx overflow.\n",
                k->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    /* if k = 0 */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,k ) )
    {
        s[ 0 ] = 0;
        return ( 0 );
    }

    /* now k > 0 */

    len = 0;
    for ( i = 0; i < k->len; i ++ )
    {
        t = k->data[ i ];
        for ( j = 0; j < WORDSIZE; j ++ )
        {
            s[ len ] = t & 1;
            t >>= 1;
            len ++ ;
        }
    }

    /* now len > 0 */
    /* here must dec len. otherwise, */
    /* when len = INTMAX * WORDSIZE, s[ len ] will overflow of array s */
    len -- ;
    while ( len > 0 && 0 == s [ len ] )
    {
        len --;
    }

    return ( len + 1 );
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
int bgn_z_naf(const UINT32 bgnz_md_id,const BIGINT *k,int *s)
{
    UINT32 c0;
    UINT32 c1;
    UINT32 j;
    UINT32 len;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == k )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_naf: k is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == s )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_naf: s is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_naf: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < k->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_naf: k->len = %lx overflow.\n",
                k->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    len = bgn_z_parse_bits(bgnz_md_id,k, (UINT32 *)s);
    if ( 0 == len)
    {
        s[ 0 ] = 0;
        return len;
    }


    s[ len ] = 0;

    s[ len + 1] = 0;

    c0 = 0;
    for( j = 0; j <= len; j ++ )
    {
        c1 = ( s[ j ] + s[ j + 1 ] + c0 ) >> 1;
        s[ j ] = s[ j ] + c0 - ( c1 << 1 );
        c0 = c1;
    }

    while (len > 0 && 0 == s [ len ] )
    {
        len --;
    }

    return (len + 1);
}

/**
*   if a = 0, then nbits = 0
*   else
*       let a = SUM( a_i * 2 ^i, where i = 0..m where a_m > 0,i.e, a_m = 1 )
*   then    nbits = m + 1
*   return nbits
*
*History:
*   1. 2006.12.11 Modified to avoid a dead loop when a->len is wrong.
*       change:
*            while( 0 == ( tmp & e ) )
*            {
*                tmp <<= 1;
*                nbits--;
*            }
*      to
*            while ( 0 < e && 0 == ( tmp & e ) )
*            {
*                nbits --;
*                e >>= 1;
*            }
*
**/
UINT32 bgn_z_get_nbits(const UINT32 bgnz_md_id,const BIGINT *a)
{
    UINT32 tmp;
    UINT32 nbits;
    UINT32 a_len;
    UINT32 e;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_get_nbits: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_nbits: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_nbits: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a ) )
    {
        return ( 0 );
    }

    /* e = 0x80000000 */
    e = ( ((UINT32)1) << ( WORDSIZE - 1 ) );
    nbits = 0;

    a_len = a->len;
    tmp = a->data[ a_len - 1 ];
    nbits = ( a_len * WORDSIZE );

    while ( 0 < e && 0 == ( tmp & e ) )
    {
        nbits --;
        e >>= 1;
    }

    return nbits;
}

/**
*
*   let a = SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 bgn_z_get_bit(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nthbit)
{
    UINT32 word_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_get_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_bit: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_get_bit: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    word_offset = nthbit / WORDSIZE;
    bit_offset = nthbit % WORDSIZE;

    if ( word_offset >= a->len )
    {
        return ( 0 );
    }

    if ( 0 < ( a->data[ word_offset ] & ( 1 << bit_offset) ))
    {
        return ( 1 );
    }

    return ( 0 );
}

/**
*   let a = SUM(a_i * 2^i, where i = 0..BIGINTSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z_set_bit(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 nthbit)
{
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_bit: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_bit: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    word_offset = nthbit / WORDSIZE;
    bit_offset = nthbit % WORDSIZE;

    if ( word_offset >= INTMAX )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_bit: nthbit = %ld is out of BIGINT definition.\n",nthbit);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    if ( word_offset >= a->len )
    {
        for ( index = a->len; index < word_offset; index ++ )
        {
            a->data[ index ] = 0;
        }

        a->data[ word_offset ] = ( 1 << bit_offset );

        a->len = word_offset + 1;
    }
    else
    {
        a->data[ word_offset ] |= ( 1 << bit_offset );
    }

    return ;
}

/**
*   let a = SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
void  bgn_z_clear_bit(const UINT32 bgnz_md_id,BIGINT *a, const UINT32 nthbit)
{
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_clear_bit: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_clear_bit: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_clear_bit: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    word_offset = nthbit / WORDSIZE;
    bit_offset = nthbit % WORDSIZE;

    if ( word_offset >= a->len )
    {
        return ;
    }

    a->data[ word_offset ] &= ( ~ ( 1 << bit_offset ) );

    if ( word_offset + 1 == a->len )
    {
        for ( index = a->len; index > 0;  )
        {
            index --;
            if ( 0 != a->data[ index ] )
            {
                index ++;
                break;
            }
        }

        a->len = index;
    }
   return ;
}
/**
*   return e = 2 ^ nth mod 2 ^ BIGINTSIZE
*            = ( 1 << nth ) mod 2 ^ BIGINTSIZE
*   where nth = 0,1,...
*
**/
void bgn_z_set_e(const UINT32 bgnz_md_id,BIGINT *e, const UINT32 nth)
{
    UINT32 word_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_set_e: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_set_e: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    bgn_z_set_zero( bgnz_md_id,e );

    if ( BIGINTSIZE <= nth )
    {
        return ;
    }

    word_offset = nth / WORDSIZE;
    bit_offset = nth % WORDSIZE;

    e->data[ word_offset ] = ( 1 << bit_offset );
    e->len = word_offset + 1;

    return ;
}

/**
*   binary operation:
*       c = a AND b (i.e, a & b  )
**/
void bgn_z_bit_and(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    UINT32 min_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_and: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_and: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_and: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_and: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_and: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_and: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( a->len > b->len )
    {
        min_len = b->len;
    }
    else
    {
        min_len = a->len;
    }

    for ( index = 0; index < min_len; index ++ )
    {
        c->data[ index ] = ( a->data[ index ] & b->data[ index ] );
    }

    for( index = min_len; index > 0;  )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }

    c->len = index;

    return ;
}

/**
*   binary operation:
*       c = a OR b (i.e, a | b )
**/
void bgn_z_bit_or(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_or: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_or: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_or: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_or: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_or: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_or: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( a->len > b->len )
    {
        c->len = a->len;
        for ( index = a->len; index > b->len; )
        {
            index --;
            c->data[ index ] = a->data[ index ];
        }
    }
    else
    {
        c->len = b->len;
        for ( index = b->len; index > a->len; )
        {
            index --;
            c->data[ index ] = b->data[ index ];
        }
    }

    for ( ; index > 0; )
    {
        index --;
        c->data[ index ] = ( a->data[ index ] | b->data[ index ] );
    }

    return ;
}

/**
*   binary operation:
*       c = a XOR b (i.e, a ^ b )
**/
void bgn_z_bit_xor(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    UINT32 max_len;
    UINT32 e;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_xor: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_xor: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_xor: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_xor: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_xor: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_xor: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    e = 0;
    if ( a->len > b->len )
    {
        max_len = a->len;
        for ( index = a->len; index > b->len; )
        {
            index --;
            c->data[ index ] = ( e ^ a->data[ index ] );
        }
    }
    else
    {
        max_len = b->len;
        for ( index = b->len; index > a->len; )
        {
            index --;
            c->data[ index ] = ( e ^ b->data[ index ] );
        }
    }

    for ( ; index > 0; )
    {
        index --;
        c->data[ index ] = ( a->data[ index ] ^ b->data[ index ] );
    }

    for( index = max_len; index > 0;  )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }

    c->len = index;

    return ;
}

/**
*   binary operation:
*       c = NOT a (i.e, ~a )
**/
void bgn_z_bit_not(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c )
{
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_not: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_not: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_not: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_not: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    for ( index = 0; index < a->len; index ++)
    {
        c->data[ index ] =  ~(a->data[ index ]);
    }
    for( ; index < INTMAX; index ++ )
    {
    c->data[ index ] = ~((UINT32)0);
    }

    for( index = INTMAX; index > 0;  )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }

    c->len = index;

    return ;
}

/**
*   cut lower bit segment
*       c = a mod 2 ^ nbits
**/
void bgn_z_bit_cut(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits, BIGINT *c)
{
    UINT32 a_len;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 data;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_cut: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_bit_cut: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_cut: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_bit_cut: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    word_offset = nbits / WORDSIZE;
    bit_offset  = nbits % WORDSIZE;

    a_len = a->len;

    if( word_offset >= a_len )
    {
    bgn_z_clone(bgnz_md_id, a, c);
    return ;
    }

    /* now word_offset < a_len */

    if( c != a )
    {
        for ( index = 0; index < word_offset; index ++)
        {
        c->data[ index ] = a->data[ index ];
        }
    }

    if( 0 < bit_offset )
    {
    data = a->data[ word_offset ];
    c->data[ word_offset ] = ((data << (WORDSIZE - bit_offset)) >> (WORDSIZE - bit_offset));
    }
    else
    {
    c->data[ word_offset ] = 0;
    }

    for ( index = word_offset + 1; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }
    c->len = index;
    return ;
}

/**
*   return c = ( a >> WORDSIZE ) and the shifted out word is returned.
*   where a = ( c << WORDSIZE ) + shift_out
*   maybe address of c = address of a
**/
UINT32 bgn_z_shr_onewordsize(const UINT32 bgnz_md_id,const BIGINT *a,BIGINT *c)
{
    UINT32 shift_out; /* store shifted out data */
    UINT32 a_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_onewordsize: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_onewordsize: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    shift_out = a->data[ 0 ];
    a_len = a->len;

    if (0 == a_len )
    {

        bgn_z_set_zero( bgnz_md_id,c );
        return ( 0 );
    }

    if (1 == a_len )
    {

        bgn_z_set_zero( bgnz_md_id,c );
        return ( shift_out );
    }

    /* here a_len > 1 */
    for ( index = 0; index < a_len - 1; index ++)
    {
        c->data[ index ] = a->data[ index + 1 ];
    }
    for ( ; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }
    c->len = index;

    return ( shift_out );
}

/**
*   return c = ( a >> nbits ) and the shifted out word is returned.
*   where 0 <= nbits < WORDSIZE
*   where a = ( c << nbits ) + shift_out
*   maybe address of c = address of a
**/
UINT32 bgn_z_shr_lesswordsize(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    UINT32 shift_out; /* store shifted out data */
    UINT32 a_len;
    UINT32 e;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_lesswordsize: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_lesswordsize: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( 0 == nbits )
    {
        bgn_z_clone( bgnz_md_id,a, c );
        return ( 0 );
    }

    e =  (~(( ~( ( UINT32 ) 0 ) ) << nbits ));

    shift_out = ( a->data[ 0 ] & e );
    a_len = a->len;

    if (0 == a_len )
    {

        bgn_z_set_zero( bgnz_md_id,c );
        return ( 0 );
    }

    if (1 == a_len )
    {

        c->data[ 0 ] = ( a->data[ 0 ] >> nbits );
        if ( 0 == c->data[ 0 ] )
        {
            c->len = 0;
        }
        else
        {
            c->len = 1;
        }
        return ( shift_out );
    }

    /* here a_len > 1 */
    for ( index = 0; index < a_len - 1; index ++ )
    {
        c->data[ index ] = ( a->data[ index ] >> nbits )
                         | ( a->data[ index + 1 ] << ( WORDSIZE - nbits ) );
    }

    c->data[ index ] = ( a->data[ index ] >> nbits );

    index ++;

    for ( ; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }
    c->len = index;

    return ( shift_out );
}

/**
*   return c = ( a >> nbits ) and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_shr_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    UINT32 a_len;
    UINT32 a_nbits;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 a_idx;
    UINT32 c_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_no_shift_out: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shr_no_shift_out: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_no_shift_out: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shr_no_shift_out: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    a_nbits = bgn_z_get_nbits( bgnz_md_id,a );
    if ( nbits >= a_nbits )
    {
        bgn_z_set_zero ( bgnz_md_id,c );
        return ;
    }

    /* here 0 <= nbits < a_nbits, so that a > 0 */
    if ( 0 == nbits )
    {
        bgn_z_clone(bgnz_md_id,a, c);
        return ;
    }

    /* here a > 0 and nbits > 0 */
    a_len = a->len;
    word_offset = nbits / WORDSIZE;
    bit_offset = nbits % WORDSIZE;

    /* do c = ( a >> ( WORDSIZE * word_offset ) ) */
    c_idx = 0;
    a_idx = c_idx + word_offset;

    for ( ; a_idx < a_len; c_idx ++, a_idx ++ )
    {
        c->data[ c_idx ] = a->data[ a_idx ];
    }
    c->len = c_idx;

    /* do c = ( c >> bit_offset ) */
    bgn_z_shr_lesswordsize(bgnz_md_id,c, bit_offset, c);

    return ;
}

/**
*   let a = (a1,a0) = a1 * 2 ^BIGINTSIZE + a0
*   let c = (c1,c0) = c1 * 2 ^BIGINTSIZE + c0
*   return c = ( a >> nbits ) and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_dshr_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    UINT32 a1_nbits;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 a1_idx;
    UINT32 c0_idx;
    UINT32 min_idx;
    UINT32 max_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshr_no_shift_out: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshr_no_shift_out: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshr_no_shift_out: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshr_no_shift_out: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshr_no_shift_out: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshr_no_shift_out: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a0->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshr_no_shift_out: a0->len = %lx overflow.\n",
                a0->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < a1->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshr_no_shift_out: a1->len = %lx overflow.\n",
                a1->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    /* if a1 = 0, then c1 = 0 and c0 = (a0 >> nbits) */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a1 ) )
    {
        bgn_z_set_zero( bgnz_md_id,c1 );
        bgn_z_shr_no_shift_out(bgnz_md_id,a0, nbits, c0);
        return;
    }

    a1_nbits = bgn_z_get_nbits( bgnz_md_id,a1 );
    if ( nbits >= ( a1_nbits + BIGINTSIZE ) )
    {
        bgn_z_set_zero ( bgnz_md_id,c0 );
        bgn_z_set_zero ( bgnz_md_id,c1 );
        return ;
    }
    /* now nbits < a1_nbits + BIGINTSIZE <= 2 * BIGINTSIZE */
    /* here 0 < a1_nbits, so that a > 0 */
    if ( 0 == nbits )
    {
        bgn_z_clone(bgnz_md_id,a0, c0);
        bgn_z_clone(bgnz_md_id,a1, c1);
        return ;
    }

    if ( BIGINTSIZE <= nbits )
    {
        /* do not exchange the following 2 statements  */
        /* to avoid the scenario of addr(a1) = addr(c1) */
        bgn_z_shr_no_shift_out(bgnz_md_id,a1, nbits - BIGINTSIZE, c0);
        bgn_z_set_zero ( bgnz_md_id,c1 );
        return ;
    }

    /* now nbits < BIGINTSIZE */
    word_offset = nbits / WORDSIZE;
    bit_offset = nbits % WORDSIZE;

    /* 1st, set c0 = ( a0 >> nbits ) */
    bgn_z_shr_no_shift_out(bgnz_md_id,a0, nbits, c0);

    /* 2nd, set the high empty words of c0 to zero */
    min_idx = c0->len;
    max_idx = INTMAX - word_offset - 1;
    for ( c0_idx = min_idx; c0_idx <= max_idx; c0_idx ++ )
    {
        c0->data[ c0_idx ] = 0;
    }

    if ( 0 == bit_offset )
    {
        for ( a1_idx = 1; a1_idx <= word_offset; a1_idx ++ )
        {
            c0_idx = INTMAX - a1_idx;
            c0->data[ c0_idx ] = a1->data[ a1_idx ];
        }

        for ( c0_idx = INTMAX; c0_idx > 0; )
        {
            c0_idx --;
            if ( 0 != c0->data[ c0_idx ] )
            {
                c0_idx ++;
                break;
            }
        }
        c0->len = c0_idx;

        bgn_z_shr_no_shift_out(bgnz_md_id,a1, nbits, c1);
        return ;
    }

    /* now bit_offset > 0 */
    min_idx = INTMAX - word_offset - 1;
    c0_idx = min_idx;
    a1_idx = 0;

    /* 3rd, deal with the right-tail bit_offset bits of a1 */
    c0->data[ c0_idx ] |= ( a1->data[ a1_idx ] << ( WORDSIZE - bit_offset ));
    c0_idx ++;

    /* 4th, deal with the middle word-bits of a1 */
    for ( ;c0_idx < INTMAX; )
    {
        c0->data[ c0_idx ] = ( a1->data[ a1_idx ] >> ( bit_offset ) )
                            |( a1->data[ a1_idx + 1 ] << ( WORDSIZE - bit_offset ) );

        c0_idx ++;
        a1_idx ++;
    }

    for ( c0_idx = INTMAX; c0_idx > 0; )
    {
        c0_idx --;
        if ( 0 != c0->data[ c0_idx ] )
        {
            c0_idx ++;
            break;
        }
    }
    c0->len = c0_idx;

    bgn_z_shr_no_shift_out(bgnz_md_id,a1, nbits, c1);

    return ;
}

/**
*   return c = ( a << WORDSIZE ) and the shifted out word is returned.
*   where ( a << WORDSIZE ) = shift_out * 2 ^( BIGINTSIZE ) + c
*   maybe address of c = address of a
**/
UINT32 bgn_z_shl_onewordsize(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c)
{
    UINT32 shift_out; /* store shifted out data */
    UINT32 a_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_onewordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_onewordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_onewordsize: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_onewordsize: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    a_len = a->len;

    if ( 0 == a_len )
    {
        bgn_z_set_zero( bgnz_md_id,c );
        return ( 0 );
    }

    if ( INTMAX > a_len )
    {
        for ( index = a_len; index > 0; index -- )
        {
            c->data[ index ] = a->data[ index - 1 ];
        }
        c->data[ index ] = 0;
        c->len = a_len + 1;

        return ( 0 );
    }

    /* here a_len = INTMAX */

    shift_out = a->data[ INTMAX - 1 ];

    for ( index = INTMAX - 1; index > 0; index -- )
    {
        c->data[ index ] = a->data[ index - 1 ];
    }
    c->data[ index ] = 0;

    for ( index = INTMAX; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }
    c->len = index;

    return ( shift_out );
}

/**
*   return c = ( a << nbits ) and the shifted out word is returned.
*   where ( a << bnits ) = shift_out * 2 ^( BIGINTSIZE ) + c
*   maybe address of c = address of a
**/
UINT32 bgn_z_shl_lesswordsize(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    UINT32 shift_out; /* store shifted out data */
    UINT32 a_len;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_lesswordsize: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_lesswordsize: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( WORDSIZE <= nbits )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_lesswordsize: nbits=%ld more than one word.\n",
                nbits);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_lesswordsize: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_lesswordsize: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( 0 == nbits )
    {
        bgn_z_clone( bgnz_md_id,a, c );
        return ( 0 );
    }

    a_len = a->len;

    if ( 0 == a_len )
    {
        bgn_z_set_zero( bgnz_md_id,c );
        return ( 0 );
    }

    if ( INTMAX > a_len )
    {
        c->data[ a_len ] = ( a->data[ a_len - 1 ] >> ( WORDSIZE - nbits ) );

        for ( index = a_len - 1; index > 0; index -- )
        {
            c->data[ index ] = ( a->data[ index ] << nbits )
                             | ( a->data[ index - 1 ] >> ( WORDSIZE - nbits ) );
        }
        c->data[index] = ( a->data[index] << nbits );

        if ( 0 == c->data[ a_len ] )
        {
            c->len = a_len;
        }
        else
        {
            c->len = a_len + 1;
        }

        return ( 0 );
    }

    /* here a_len = INTMAX */
    shift_out = ( a->data[ a_len - 1 ] >> ( WORDSIZE - nbits ) );

    for ( index = a_len - 1; index > 0; index -- )
    {
        c->data[ index ] = ( a->data[ index ] << nbits )
                       | ( a->data[ index - 1 ] >> ( WORDSIZE - nbits ) );
    }

    c->data[ index ] = ( a->data[ index ] << nbits );

    for ( index = a_len; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {

            index ++;
            break;
        }
    }
    c->len = index;

    return ( shift_out );
}

/**
*   return c = ( a << nbits ) mod 2 ^ BIGINTSIZE
*          and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_shl_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a, const UINT32 nbits,BIGINT *c)
{
    UINT32 a_len;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 a_idx;
    UINT32 c_idx;
    UINT32 max_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_no_shift_out: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_shl_no_shift_out: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_no_shift_out: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_shl_no_shift_out: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( nbits >= BIGINTSIZE )
    {
        bgn_z_set_zero( bgnz_md_id,c );
        return ;
    }

    if ( 0 == nbits )
    {
        bgn_z_clone(bgnz_md_id,a, c);
        return ;
    }

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a ) )
    {
        bgn_z_set_zero( bgnz_md_id,c );
        return ;
    }

    /* here a > 0 and BIGINTSIZE > nbits > 0 */

    /* here a > 0 and nbits > 0 */
    a_len = a->len;
    word_offset = nbits / WORDSIZE;
    bit_offset = nbits % WORDSIZE;

    /* do c = ( a << ( WORDSIZE * word_offset ) ) mod 2 ^ BIGINTSIZE */
    if ( a_len + word_offset > INTMAX )
    {
        max_idx = INTMAX;
    }
    else
    {
        max_idx = a_len + word_offset;
    }
    c_idx = max_idx;
    a_idx = c_idx - word_offset;

    for (; a_idx > 0; )
    {
        c_idx --;
        a_idx --;
        c->data[ c_idx ] = a->data[ a_idx ];
    }
    for ( ; c_idx > 0; )
    {
        c_idx --;
        c->data[ c_idx ] = 0;
    }

    for ( c_idx = max_idx; c_idx > 0; )
    {
        c_idx --;
        if ( 0 != c->data[ c_idx ] )
        {
            c_idx ++;
            break;
        }
    }
    c->len = c_idx;

    /* do c = ( c << bit_offset ) */
    bgn_z_shl_lesswordsize(bgnz_md_id,c, bit_offset, c);

    return ;
}
/**
*   let a = (a1,a0) = a1 * 2 ^BIGINTSIZE + a0
*   let c = (c1,c0) = c1 * 2 ^BIGINTSIZE + c0
*
*   return c = ( a << nbits ) mod 2 ^ {2 *BIGINTSIZE}
*          and no shifted out word is returned.
*   where 0 <= nbits
*   maybe address of c = address of a
**/
void bgn_z_dshl_no_shift_out(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const UINT32 nbits,BIGINT *c0,BIGINT *c1)
{
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 a0_idx;
    UINT32 c1_idx;
    UINT32 min_idx;
    UINT32 max_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshl_no_shift_out: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshl_no_shift_out: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshl_no_shift_out: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshl_no_shift_out: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dshl_no_shift_out: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshl_no_shift_out: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a0->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshl_no_shift_out: a0->len = %lx overflow.\n",
                a0->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < a1->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dshl_no_shift_out: a1->len = %lx overflow.\n",
                a1->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    /* if a0 = 0, then c0 = 0 and c1 = (a1 << nbits) */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a0 ) )
    {
        bgn_z_set_zero( bgnz_md_id,c0 );
        bgn_z_shl_no_shift_out( bgnz_md_id,a1, nbits, c1 );
        return ;
    }

    if ( nbits >= (2 * BIGINTSIZE) )
    {
        bgn_z_set_zero( bgnz_md_id,c0 );
        bgn_z_set_zero( bgnz_md_id,c1 );
        return ;
    }

    if ( 0 == nbits )
    {
        bgn_z_clone(bgnz_md_id,a0, c0);
        bgn_z_clone(bgnz_md_id,a1, c1);
        return ;
    }

    if ( BIGINTSIZE <= nbits )
    {
        /* do not exchange the following 2 statements  */
        /* to avoid the scenario of addr(a1) = addr(c1) */
        bgn_z_shl_no_shift_out(bgnz_md_id,a0, nbits - BIGINTSIZE, c1);
        bgn_z_set_zero ( bgnz_md_id,c0 );
        return ;
    }

    /* here a1 > 0 and a0 > 0 and BIGINTSIZE > nbits > 0 */

    word_offset = nbits / WORDSIZE;
    bit_offset = nbits % WORDSIZE;

    /* 1st, set c1 = ( a1 << nbits ) */
    bgn_z_shl_no_shift_out(bgnz_md_id,a1, nbits, c1);

    if ( 0 == bit_offset )
    {
        max_idx = a0->len;
        min_idx = INTMAX - word_offset;
        c1_idx = 0;

        for ( a0_idx = min_idx; a0_idx < max_idx; )
        {
            c1->data[ c1_idx ] = a0->data[ a0_idx ];
            a0_idx ++;
            c1_idx ++;
        }

        /* when a1 = 0, then c1_len = 0, then adjust it now */
        /* otherwise, a1 > 0 , so that must have c1_len >= c1_idx */
        if ( c1->len < c1_idx )
        {
            c1->len = c1_idx;
        }

        bgn_z_shl_no_shift_out(bgnz_md_id,a0, nbits, c0);
        return ;
    }

    /* 2nd, deal with the word-bits of a0 */
    /* note: a0> 0, so that a0_len > 0 */
    max_idx = a0->len - 1;
    min_idx = INTMAX - word_offset - 1;
    c1_idx = 0;

    for ( a0_idx = min_idx; a0_idx < max_idx; )
    {
        c1->data[ c1_idx ] = ( a0->data[ a0_idx ] >> ( WORDSIZE - bit_offset ) )
                           | ( a0->data[ a0_idx + 1 ] << ( bit_offset ) );
        a0_idx ++;
        c1_idx ++;
    }

    /* 3rd, deal with the left-tail bit_offset of a0 */
    c1->data[ c1_idx ] |= ( a0->data[ a0_idx ] >> ( WORDSIZE - bit_offset ) );

    /* when a1 = 0, then c1_len = 0, then adjust it now */
    /* otherwise, a1 > 0 , so that must have c1_len >= c1_idx */
    c1_idx ++;
    if ( c1->len < c1_idx )
    {
        for ( ; c1_idx > 0; )
        {
            c1_idx --;
            if ( 0 != c1->data[ c1_idx ] )
            {
                c1_idx ++;
                break;
            }
        }
        c1->len = c1_idx;
    }

    bgn_z_shl_no_shift_out(bgnz_md_id,a0, nbits, c0);

    return ;
}

/* (carry, c) = a + b + carry */
#define addx(a, b, c, carry) do{\
        UINT32 _c0_;\
        UINT32 _c1_;\
        _c0_ = ((a) & 1) + ((b) & 1) + (carry);\
        _c1_ = ((a) >> 1) +((b) >> 1) + ((_c0_) >> 1);\
        (carry) = ((_c1_) >> (WORDSIZE - 1));\
        (c) = (((_c1_) << 1) | ((_c0_) & 1));\
}while(0)

/* (borrow, c) = a + b + borrow */
#define subx(a, b, c, borrow) do{\
        UINT32 _c0_;\
        UINT32 _c1_;\
        _c0_ = ((((a) & 1) - ((b) & 1) - (borrow)) & 3);\
        _c1_ = ((a) >> 1) - ((b) >> 1) - ((_c0_) >> 1);\
        (borrow) = ((_c1_) >> (WORDSIZE - 1));\
        (c) = (((_c1_) << 1) | ((_c0_) & 1));\
}while(0)

/**
*      a + b = (carry, c) = carry * 2 ^ BIGINTSIZE + c
**/
#if 0
static UINT32 debug_counter = 0;
#endif
UINT32 bgn_z_add(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    UINT32 max_len;
    UINT32 min_len;
    UINT32 diff_len;
    BIGINT *longer;
    BIGINT *shorter;
    UINT32 *plonger;
    UINT32 *pshorter;
    UINT32 *pc;
    UINT32 carry;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_add: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_add: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_add: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_add: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_add: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_add: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

#if 0
    debug_counter ++;
    if ( 999999 == debug_counter )
    {
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif

    pc = ( UINT32 * )&(c->data[0]);

    if ( a->len > b->len )
    {
        longer  = (BIGINT *)a;
        shorter = (BIGINT *)b;
    }
    else
    {
        longer  = (BIGINT *)b;
        shorter = (BIGINT *)a;
    }

    plonger  = ( UINT32 * )&( longer->data[ 0 ] );
    pshorter = ( UINT32 * )&( shorter->data[ 0 ] );

    max_len = longer->len;
    min_len = shorter->len;

    diff_len = max_len - min_len;

    if ( 0 == min_len )
    {
        bgn_z_clone(bgnz_md_id,longer, c);
        return ( 0 );
    }

    /* here longer >= shorter > 0 */
    carry = 0;

    if ( 0 == diff_len )
    {
#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    for( index = 0; index < min_len; index ++ )
    {
        addx(plonger[ index ], pshorter[ index ], pc[ index ], carry);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
        __asm__
        (
            /* let edx = 0*/
            "xorl %%edx, %%edx;"

            "xorl %%ecx, %%ecx;"
            "movl %4   ,%%ecx;"

        "bgn_z_add_loop:;"
            "movl (%%esi), %%eax;"
            "adcl (%%edi), %%eax;"
            "movl %%eax, ( %%ebx );"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%edi), %%edi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_add_loop;"

            /* note %%edx = 0*/
            "adcl %%edx, %%edx;"
        "movl %%edx,%0;"
    :"=m"(carry)
    :"S"(plonger),"D"(pshorter),"b"(pc),"m"(min_len)
    :"memory"
        );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/
    }

    else
    {
#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    for( index = 0; index < min_len; index ++ )
    {
        addx(plonger[ index ], pshorter[ index ], pc[ index ], carry);
    }
    for( ; index < max_len; index ++ )
    {
        addx(plonger[ index ], 0, pc[ index ], carry);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
        /* 0 != min_len && 0 != diff_len  */
        __asm__
        (
            /*let edx = 0*/
            "xorl %%edx, %%edx;"

            "xorl %%ecx, %%ecx;"
            "movl %4, %%ecx;"

        "bgn_z_add_low_loop:;"
            "movl (%%esi), %%eax;"
            "adcl (%%edi), %%eax;"
            "movl %%eax, ( %%ebx );"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%edi), %%edi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_add_low_loop;"

            /*keep carry flag unchanged*/
            "movl %5, %%ecx;"
        "bgn_z_add_high_loop:;"
            "movl (%%esi), %%eax;"
            "adcl %%edx, %%eax;"
            "movl %%eax, ( %%ebx );"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_add_high_loop;"

            /*note edx = 0*/
            "adcl %%edx, %%edx;"
            "movl %%edx, %0;"
    :"=m"(carry)
    :"S"(plonger),"D"(pshorter),"b"(pc),"m"(min_len),"m"(diff_len)
    :"memory"
        );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/
    }

    if ( 0 < carry )
    {
        if ( INTMAX > max_len  )
        {
            c->data[ max_len ] = carry;
            c->len = max_len + 1;
            carry = 0;
        }
        else
        {
            for( index = INTMAX; index > 0; )
            {
                index --;
                if ( 0 != c->data[ index ] )
                {
                    index ++;
                    break;
                }
            }
            c->len = index;
        }
    }
    else
    {
        c->len = max_len;
    }

    return carry;
}

/**
*       a + b = ( carry, c ) = carry * 2 ^ BIGINTSIZE + c
*       where b < 2 ^ WORDSIZE
**/
UINT32 bgn_z_sadd(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *tb;
    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sadd: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sadd: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sadd: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sadd: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tb = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tb, LOC_BGNZ_0001);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_set_word(bgnz_md_id,tb, b);

    carry = bgn_z_add(bgnz_md_id,a, tb, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tb, LOC_BGNZ_0002);
#endif/* STATIC_MEMORY_SWITCH */

    return carry;
}

/**
*       a + b + carry0= (carry1, c) = carry1 * 2 ^ BIGINTSIZE + c
**/
UINT32 bgn_z_adc(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, const UINT32 carry, BIGINT *c )
{
    UINT32 carry1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_adc: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_adc: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_adc: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_adc: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_adc: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_adc: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    carry1 = bgn_z_add(bgnz_md_id, a, b, c);
    if( 0 < carry )
    {
    carry1 += bgn_z_sadd(bgnz_md_id, c, carry, c);
    }

    return ( carry1 );
}

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           c = (c1, c0) = c1 * 2^ BIGINTSIZE + c0
*
*       return c such that a + b = ( carry, c  ) = carry * 2 ^ BIGINTSIZE + c
*
**/
UINT32 bgn_z_dadd(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 )
{
    UINT32 carry;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a0 == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: address of a0 and a1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: address of a0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: address of b and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dadd: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dadd: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a0->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dadd: a0->len = %lx overflow.\n",
                a0->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < a1->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dadd: a1->len = %lx overflow.\n",
                a1->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dadd: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    carry = bgn_z_add(bgnz_md_id,a0, b, c0);
    carry = bgn_z_sadd(bgnz_md_id,a1, carry, c1);

    return ( carry );
}

/**
*       (borrow, c) = ( a - borrow - b )
*    i.e, c = borrow1 *(2^WORDSIZE) + a - borrow0 - b
*    note, borrow is IN and OUT parameter
**/
static void bgn_z_word_borrow(const UINT32 a, const UINT32 b, UINT32 *c, UINT32 *borrow)
{
    if( a >= b )
    {
        *c = a - b;
        *borrow = 0;
    }
    else
    {
        *c = a - b;
        *borrow = 1;
    }

    return ;
}

/**
*       c =  borrow * 2 ^ BIGINTSIZE + a - b
**/
UINT32 bgn_z_sub(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c )
{
    UINT32 a_len;
    UINT32 b_len;
    UINT32 diff_len;
    UINT32 *pa;
    UINT32 *pb;
    UINT32 *pc;
    UINT32 borrow;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sub: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sub: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sub: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    pa = ( UINT32 * )&(a->data[ 0 ]);
    pb = ( UINT32 * )&(b->data[ 0 ]);
    pc = ( UINT32 * )&(c->data[ 0 ]);

    a_len = a->len;
    b_len = b->len;

    if( 0 == b_len )
    {
        bgn_z_clone(bgnz_md_id,a, c);
        return ( 0 );

    }

    if ( 0 == a_len )
    {
        /* here a = 0 and b > 0 */

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    borrow = 0;
    for( index = 0; index < b_len; index ++ )
    {
        subx(0, pb[ index ], pc[ index], borrow);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
        __asm__
        (
            /*set edx = 0*/
        "xorl %%edx, %%edx;"

        /*set flag bit to zero*/
        "xorl %%ecx, %%ecx;"
            "movl %2, %%ecx;"

        "bgn_z_sub_loop_1:;"
        "movl %%edx, %%eax;"
        "sbbl (%%edi), %%eax;"
        "movl %%eax, (%%ebx);"

        "leal 0x4(%%edi), %%edi;"
        "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_sub_loop_1;"
    :
    :"D"(pb),"b"(pc),"m"(b_len)
    :"memory"
        );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

        for ( index = b_len; index < INTMAX; index ++ )
        {
            c->data[ index ] = ( ~( ( UINT32 ) 0 ) );
        }

        for( index = INTMAX; index > 0; )
        {
            index --;
            if ( 0 != c->data[ index ] )
            {
                index ++;
                break;
            }
        }
        c->len = index;

        return ( 1 );
    }

    /* here a > 0 and b > 0 */
    if ( a_len < b_len )
    {
        diff_len = b_len - a_len;

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    borrow = 0;
    for( index = 0; index < a_len; index ++ )
    {
        subx(pa[ index ], pb[ index ], pc[ index], borrow);
    }
    for(; index < b_len; index ++ )
    {
        subx(0, pb[ index ], pc[ index], borrow);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
        __asm__
        (
        /*set edx = 0*/
            "xorl %%edx, %%edx;"

        /*set flag bit to zero*/
            "xorl %%ecx, %%ecx;"
            "movl %3, %%ecx;"

        "bgn_z_sub_loop_2:;"
            "movl (%%esi), %%eax;"
            "sbbl (%%edi), %%eax;"
            "movl %%eax, (%%ebx);"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%edi), %%edi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_sub_loop_2;"

            "movl %4, %%ecx;"
        "bgn_z_sub_loop_3:;"
        /* eax=0 */
            "movl %%edx, %%eax;"
            "sbbl (%%edi), %%eax;"
            "movl %%eax, (%%ebx);"

            "leal 0x4(%%edi), %%edi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_sub_loop_3;"
    :
    :"S"(pa),"D"(pb),"b"(pc),"m"(a_len),"m"(diff_len)
    :"memory"
        );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

        for ( index = b_len; index < INTMAX; index ++ )
        {
            c->data[ index ] = ( ~( ( UINT32 ) 0 ) );
        }

        for( index = INTMAX; index > 0; )
        {
            index --;
            if ( 0 != c->data[ index ] )
            {
                index ++;
                break;
            }
        }
        c->len = index;

        return ( 1 );
    }

    if ( a_len > b_len )
    {
        diff_len = a_len - b_len;

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    borrow = 0;
    for( index = 0; index < b_len; index ++ )
    {
        subx(pa[ index ], pb[ index ], pc[ index], borrow);
    }
    for( ; index < a_len; index ++ )
    {
        subx(pa[ index ], 0, pc[ index], borrow);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
        __asm__
        (
            /* set edx = 0 */
            "xorl %%edx, %%edx;"

        /* set flag bit to zero */
            "xorl %%ecx, %%ecx;"
            "movl %3, %%ecx;"

        "bgn_z_sub_loop_4:;"
            "movl (%%esi), %%eax;"
            "sbbl (%%edi), %%eax;"
            "movl %%eax, (%%ebx);"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%edi), %%edi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_sub_loop_4;"

            "movl %4, %%ecx;"
        "bgn_z_sub_loop_5:;"
            "movl (%%esi), %%eax;"
        /* note: edx = 0*/
            "sbbl %%edx, %%eax;"
            "movl %%eax, (%%ebx);"

            "leal 0x4(%%esi), %%esi;"
            "leal 0x4(%%ebx), %%ebx;"
        "loop bgn_z_sub_loop_5;"
    :
    :"S"(pa),"D"(pb),"b"(pc),"m"(b_len),"m"(diff_len)
    :"memory"
        );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

        for( index = a_len; index > 0; )
        {
            index --;
            if ( 0 != c->data[ index ] )
            {
                index ++;
                break;
            }
        }
        c->len = index;

        return ( 0 );
    }

    /* here a > 0 and b > 0 and a_len = b_len */

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )
    borrow = 0;
    for( index = 0; index < a_len; index ++ )
    {
    subx(pa[ index ], pb[ index ], pc[ index], borrow);
    }
#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
    __asm__
    (
    /* set edx = 0 */
        "xorl %%edx, %%edx;"

    /* set flag bit to zero */
        "xorl %%ecx, %%ecx;"
        "movl %4, %%ecx;"

    "bgn_z_sub_loop_6:;"
        "movl (%%esi), %%eax;"
        "sbbl (%%edi), %%eax;"
        "movl %%eax, (%%ebx);"

    "leal 0x4(%%esi), %%esi;"
    "leal 0x4(%%edi), %%edi;"
    "leal 0x4(%%ebx), %%ebx;"
    "loop bgn_z_sub_loop_6;"

        "adcl %%edx, %%edx;"
        "movl %%edx, %0;"
       :"=m"(borrow)
    :"S"(pa),"D"(pb),"b"(pc),"m"(a_len)
    :"memory"
    );
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

    if ( 1 == borrow )
    {
        for ( index = a_len; index < INTMAX; index ++ )
        {
            c->data[ index ] = ( ~( ( UINT32 ) 0 ) );
        }

        for( index = INTMAX; index > 0; )
        {
            index --;
            if ( 0 != c->data[ index ] )
            {
                index ++;
                break;
            }
        }
        c->len = index;

        return ( 1 );
    }

    for( index = a_len; index > 0; )
    {
        index --;
        if ( 0 != c->data[ index ] )
        {
            index ++;
            break;
        }
    }
    c->len = index;

    return ( 0 );
}

/**
*       c = borrow * 2 ^ BIGINTSIZE + a - b
*       where b < 2 ^ WORDSIZE
**/
UINT32 bgn_z_ssub(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *tb;
    UINT32 borrow;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ssub: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ssub: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ssub: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ssub: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    tb = &buf_1;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tb, LOC_BGNZ_0003);
#endif/* STATIC_MEMORY_SWITCH */
    bgn_z_set_word(bgnz_md_id,tb, b);

    borrow = bgn_z_sub(bgnz_md_id,a, tb, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tb, LOC_BGNZ_0004);
#endif/* STATIC_MEMORY_SWITCH */

    return ( borrow );
}

/**
*       ( -borrow1, c) =  a - b - borrow0
**/
UINT32 bgn_z_sbb(const UINT32 bgnz_md_id,const BIGINT *a, const BIGINT *b, const UINT32 borrow, BIGINT *c)
{
    UINT32 borrow1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sbb: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sbb: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sbb: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sbb: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sbb: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_sbb: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    borrow1 = bgn_z_sub(bgnz_md_id, a, b, c);
    if( 0 < borrow )
    {
    borrow1 += bgn_z_ssub(bgnz_md_id, c, borrow, c);
    }

    return ( borrow1 );
}

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           c = (c1, c0) = c1 * 2^ BIGINTSIZE + c0
*
*       return c such that c = borrow * 2 ^ BIGINTSIZE + a - b
*
**/
UINT32 bgn_z_dsub(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b, BIGINT *c0, BIGINT *c1 )
{
    UINT32 borrow;
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a0 == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of a0 and a1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a0 == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of a0 and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of a0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of b and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of b and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_dsub: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dsub: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a0->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dsub: a0->len = %lx overflow.\n",
                a0->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < a1->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dsub: a1->len = %lx overflow.\n",
                a1->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_dsub: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    borrow = bgn_z_sub(bgnz_md_id,a0, b, c0);
    borrow = bgn_z_ssub(bgnz_md_id,a1, borrow, c1);

    return ( borrow );
}

#if ( ASM_DISABLE_SWITCH == SWITCH_ON )

#define mulx(a, b, r) do{\
        UINT32 _aa_[ 2 ];\
        UINT32 _bb_[ 2 ];\
        UINT32 _c00_;\
        UINT32 _c01_;\
        UINT32 _c10_;\
        UINT32 _c11_;\
        UINT32 _dd_[ 2 ];\
        UINT32 _ee_[ 2 ];\
        UINT32 _mask_;\
        UINT32 _carry_;\
        UINT32 _high_;\
        UINT32 _low_;\
\
        _mask_ = (((UINT32)(-1)) >> (WORDSIZE/2));\
\
        _aa_[ 0 ] = ((a) & _mask_);\
        _aa_[ 1 ] = ((a) >> (WORDSIZE/2));\
\
        _bb_[ 0 ] = ((b) & _mask_);\
        _bb_[ 1 ] = ((b) >> (WORDSIZE/2));\
\
        _c00_ = _aa_[ 0 ] * _bb_[ 0 ];\
        _c01_ = _aa_[ 0 ] * _bb_[ 1 ];\
        _c10_ = _aa_[ 1 ] * _bb_[ 0 ];\
        _c11_ = _aa_[ 1 ] * _bb_[ 1 ];\
\
        _dd_[ 0 ] = (((_c01_) & _mask_) << (WORDSIZE/2));\
        _dd_[ 1 ] = ((_c01_) >> (WORDSIZE/2));\
\
        _ee_[ 0 ] = (((_c10_) & _mask_) << (WORDSIZE/2));\
        _ee_[ 1 ] = ((_c10_) >> (WORDSIZE/2));\
\
        _high_ = _c11_ + _dd_[ 1 ] + _ee_[ 1 ];\
\
        _carry_ = 0;\
        addx(_c00_, _dd_[ 0 ], _low_, _carry_);\
        _high_ += _carry_;\
\
        _carry_ = 0;\
        addx(_low_, _ee_[ 0 ], _low_, _carry_);\
        _high_ += _carry_;\
\
        _carry_ = 0;\
        addx(_low_ , (r)[ 0 ], (r)[ 0 ], _carry_);\
        addx(_high_, (r)[ 1 ], (r)[ 1 ], _carry_);\
        addx(0     , (r)[ 2 ], (r)[ 2 ], _carry_);\
}while(0)

#endif/*ASM_DISABLE_SWITCH == SWITCH_ON*/

#if ( ASM_DISABLE_SWITCH == SWITCH_OFF )
/**
*   let r = (r2,r1,r0) = r2 * 2 ^( WORDSIZE *2 ) + r1 * 2 ^( WORDSIZE ) + r0
*   then
*       return  r = r + a * b
**/
static void mulx(UINT32 a,UINT32 b,UINT32 r[ 3 ] )
{
    __asm__
    (
        "mul %2;"

        /* add edx : eax to r2:r1:r0 */
    /* r0 */
        "addl %%eax, (%%esi);"
    /* r1 */
        "adcl %%edx, 0x4(%%esi);"
    /* r2 */
        "adcl $0, 0x8(%%esi);"
    :
    :"S"(r),"a"(a),"ir"(b)
    :"memory"
    );
}
#endif/*ASM_DISABLE_SWITCH == SWITCH_OFF*/

/**
*   return c1 * 2^BIGINTSIZE+ c0 = a * b
*
*   Algorithm:
*   Input : a , b and assume a > 0 and b> 0 and len(a) <= len(b)
*   Output: C=(c1,c0) where  c1 * 2^BIGINTSIZE+ c0 = a * b
*   1.  for k from 0 to len(a) - 1 do
*   1.1      C[k] = SUM( a[i] * b[k-i] where i = 0..k )
*   1.2 end do
*   2.  for k from len(a) to len(b) - 1 do
*   2.1    C[k] = SUM( a[i] * b[k-i] where i = 0.. len(a) - 1 )
*   2.2 end do
*   3.  for k from len(b) to len(a) + len(b) - 1 do
*   3.1      C[k] = SUM( a[i] * b[k-i] where i = k - len(b) + 1.. len(a) - 1 )
*   3.2 end do
*   4. C[ len(a) + len(b) ] = carry
*
**/
void bgn_z_mul(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *c0,BIGINT *c1 )
{
    UINT32 *pa; /*shorter of { a, b} */
    UINT32 *pb; /*longer of {a, b}  */
    UINT32 *pc;
    UINT32 r[3];
    UINT32 a_len;
    UINT32 b_len;
    UINT32 max_len;
    UINT32 a_idx;
    UINT32 b_idx;
    UINT32 c_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: address of a and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: address of a and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: address of b and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: address of b and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mul: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mul: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mul: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mul: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a ) || EC_TRUE == bgn_z_is_zero( bgnz_md_id,b ) )
    {
        bgn_z_set_zero( bgnz_md_id,c0 );
        bgn_z_set_zero( bgnz_md_id,c1 );

        return ;
    }

    if ( a->len > b->len )
    {
        pa = ( UINT32 * )&( b->data[ 0 ] );
        pb = ( UINT32 * )&( a->data[ 0 ] );

        a_len = b->len;
        b_len = a->len;
    }
    else
    {
        pa = ( UINT32 * )&( a->data[ 0 ] );
        pb = ( UINT32 * )&( b->data[ 0 ] );

        a_len = a->len;
        b_len = b->len;
    }

    /* here max_len > 0 */
    max_len = a_len + b_len;

    r[ 0 ] = 0;
    r[ 1 ] = 0;
    r[ 2 ] = 0;

    pc = &( c0->data[ 0 ] );

    for ( c_idx = 0; c_idx < a_len; c_idx ++ )
    {
#if ( 0 )
        sys_log(LOGSTDOUT,"[%ld] = ",c_idx);
#endif /* BIGINT_DEBUG_SWITCH */
        for ( a_idx = 0; a_idx <= c_idx; a_idx ++ )
        {
            b_idx = c_idx - a_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( a_idx > a_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                               __LINE__,a_idx,a_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
            if ( b_idx > b_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: b_idx = %ld, b_len = %ld\n",
                               __LINE__,b_idx,b_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */

            mulx( pa[ a_idx ], pb[ b_idx ], r );
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld,%ld] ",a_idx,b_idx);
#endif /* BIGINT_DEBUG_SWITCH */

        }
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( c_idx >= INTMAX )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: c_idx = %ld\n",
                           __LINE__,c_idx);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */

        pc[ c_idx ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;

#if ( 0 )
        sys_log(LOGSTDOUT,"\n");
#endif /* BIGINT_DEBUG_SWITCH */
    }

    for ( ; c_idx < b_len; c_idx ++ )
    {
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld] = ",c_idx);
#endif /* BIGINT_DEBUG_SWITCH */
        for ( a_idx = 0; a_idx < a_len; a_idx ++ )
        {
            b_idx = c_idx - a_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( a_idx > a_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                               __LINE__,a_idx,a_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
            if ( b_idx > b_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: b_idx = %ld, b_len = %ld\n",
                               __LINE__,b_idx,b_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */

            mulx( pa[ a_idx ], pb[ b_idx ], r );
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld,%ld] ",a_idx,b_idx);
#endif /* BIGINT_DEBUG_SWITCH */
        }
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( c_idx >= INTMAX )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: c_idx = %ld\n",
                           __LINE__,c_idx);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */

        pc[ c_idx ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;
#if ( 0 )
        sys_log(LOGSTDOUT,"\n");
#endif /* BIGINT_DEBUG_SWITCH */
    }

    if ( INTMAX > max_len )
    {
        for ( ; c_idx < max_len; c_idx ++ )
        {
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld] = ",c_idx);
#endif /* BIGINT_DEBUG_SWITCH */

            a_idx = c_idx - b_len + 1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( a_idx > a_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                               __LINE__,a_idx,a_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */

            for ( ; a_idx < a_len; a_idx ++ )
            {
                b_idx = c_idx - a_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
                if ( a_idx > a_len )
                {
                    sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                                   __LINE__,a_idx,a_len);
                    dbg_exit(MD_BGNZ, bgnz_md_id);
                }
                if ( b_idx > b_len )
                {
                    sys_log(LOGSTDOUT,"[%ld]array overflow: b_idx = %ld, b_len = %ld\n",
                                   __LINE__,b_idx,b_len);
                    dbg_exit(MD_BGNZ, bgnz_md_id);
                }
#endif /* BIGINT_DEBUG_SWITCH */

                mulx( pa[ a_idx ], pb[ b_idx ], r );

#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld,%ld] ",a_idx,b_idx);
#endif /* BIGINT_DEBUG_SWITCH */
            }
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( c_idx >= INTMAX )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: c_idx = %ld\n",
                               __LINE__,c_idx);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */

            pc[ c_idx ] = r[ 0 ];
            r[ 0 ] = r[ 1 ];
            r[ 1 ] = r[ 2 ];
            r[ 2 ] = 0;

#if ( 0 )
            sys_log(LOGSTDOUT,"\n");
#endif /* BIGINT_DEBUG_SWITCH */
        }

        for ( ; c_idx > 0;  )
        {
            c_idx --;
            if ( 0 != pc[ c_idx ] )
            {
                c_idx ++;
                break;
            }
        }
        c0->len = c_idx;

        bgn_z_set_zero( bgnz_md_id,c1 );

        return ;
    }

    /* here INTMAX < max_len */
    for ( ; c_idx < INTMAX; c_idx ++ )
    {
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld] = ",c_idx);
#endif /* BIGINT_DEBUG_SWITCH */

        a_idx = c_idx - b_len + 1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( a_idx > a_len )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                           __LINE__,a_idx,a_len);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */

        for ( ; a_idx < a_len; a_idx ++ )
        {
            b_idx = c_idx - a_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( a_idx > a_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                               __LINE__,a_idx,a_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
            if ( b_idx > b_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: b_idx = %ld, b_len = %ld\n",
                               __LINE__,b_idx,b_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */

            mulx( pa[ a_idx ], pb[ b_idx ], r );

#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld,%ld] ",a_idx,b_idx);
#endif /* BIGINT_DEBUG_SWITCH */

        }
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( c_idx >= INTMAX )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: c_idx = %ld\n",
                           __LINE__,c_idx);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */

        pc[ c_idx ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;

#if ( 0 )
        sys_log(LOGSTDOUT,"\n");
#endif /* BIGINT_DEBUG_SWITCH */
    }

    for ( c_idx = INTMAX; c_idx > 0; )
    {
        c_idx --;
        if ( 0 != pc[ c_idx ] )
        {
            c_idx ++;
            break;
        }
    }
    c0->len = c_idx;

    pc = &( c1->data[ 0 ] );
    for (c_idx = INTMAX ; c_idx < max_len; c_idx ++ )
    {
#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld] = ",c_idx);
#endif /* BIGINT_DEBUG_SWITCH */

        a_idx = c_idx - b_len + 1;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( a_idx > a_len )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                           __LINE__,a_idx,a_len);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */


        for ( ; a_idx < a_len; a_idx ++ )
        {
            b_idx = c_idx - a_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
            if ( a_idx > a_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: a_idx = %ld, a_len = %ld\n",
                               __LINE__,a_idx,a_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
            if ( b_idx > b_len )
            {
                sys_log(LOGSTDOUT,"[%ld]array overflow: b_idx = %ld, b_len = %ld\n",
                               __LINE__,b_idx,b_len);
                dbg_exit(MD_BGNZ, bgnz_md_id);
            }
#endif /* BIGINT_DEBUG_SWITCH */


            mulx( pa[ a_idx ], pb[ b_idx ], r );

#if ( 0 )
            sys_log(LOGSTDOUT,"[%ld,%ld] ",a_idx,b_idx);
#endif /* BIGINT_DEBUG_SWITCH */

        }
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( c_idx - INTMAX >= INTMAX )
        {
            sys_log(LOGSTDOUT,"[%ld]array overflow: c_idx = %ld\n",
                           __LINE__,c_idx);
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif /* BIGINT_DEBUG_SWITCH */

        pc[ c_idx - INTMAX ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;

#if ( 0 )
        sys_log(LOGSTDOUT,"\n");
#endif /* BIGINT_DEBUG_SWITCH */

    }

    c_idx = c_idx - INTMAX;
    for ( ; c_idx > 0; )
    {
        c_idx --;
        if ( 0 != pc[ c_idx ] )
        {
            c_idx ++;
            break;
        }
    }
    c1->len = c_idx;

    return ;
}

/**
*   return c1 * 2^BIGINTSIZE+ c0 = a * b
*
*   Algorithm:
*   Input : a , b and assume a > 0 and b > 0
*   Output: C=(c1,c0) where  c1 * 2^BIGINTSIZE+ c0 = a * b
*   0.  set carry = 0;
*   1.  for k from 0 to len(a) - 1 do
*   1.1      (carry, C[k]) = a[k] * b + carry
*   1.2 end do
*
**/
void bgn_z_smul(const UINT32 bgnz_md_id,const BIGINT *a,const UINT32 b, BIGINT *c0,BIGINT *c1 )
{
    UINT32 *pa;
    UINT32 *pc;
    UINT32 r[3];
    UINT32 a_len;
    UINT32 a_idx;
    UINT32 c_idx;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: address of a and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: address of a and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_smul: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_smul: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_smul: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a ) || 0 == b )
    {
        bgn_z_set_zero( bgnz_md_id,c0 );
        bgn_z_set_zero( bgnz_md_id,c1 );

        return ;
    }

    pa = ( UINT32 * )&( a->data[ 0 ] );
    pc = ( UINT32 * )&( c0->data[ 0 ] );
    a_len = a->len;

    r[ 0 ] = 0;
    r[ 1 ] = 0;
    r[ 2 ] = 0;

    for ( a_idx = 0; a_idx < a_len; a_idx ++ )
    {
        mulx( pa[ a_idx ], b, r );

        pc[ a_idx ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;
    }

    if ( INTMAX > a_len )
    {
        pc[ a_idx ] = r[ 0 ];
        r[ 0 ] = r[ 1 ];
        a_idx ++;
    }

    for ( c_idx = a_idx; c_idx > 0; )
    {
        c_idx --;
        if ( 0 != pc[ c_idx ] )
        {
            c_idx ++;
            break;
        }
    }
    c0->len = c_idx;

    /*here r[ 1 ] must be zero*/
#if 0
    if ( 0 < r[ 1 ] )
    {
        c1->data[ 1 ] = r[ 1 ];
        c1->data[ 0 ] = r[ 0 ];
        c1->len = 2;

        return ;
    }
#endif
    if ( 0 < r[ 0 ] )
    {
        c1->data[ 0 ] = r[ 0 ];
        c1->len = 1;
    }
    else
    {
        c1->len = 0;
    }

    return ;
}

/**
*   return c1 * 2^BIGINTSIZE + c0 = a ^ 2
*
*   Algorithm:
*   Input : a and assume a > 0
*   Output: C=(c1,c0) where  c1 * 2^BIGINTSIZE+ c0 = a ^2
*   1.  for k from 0 to len(a) - 1 do
*   1.1      C[k] = SUM( a[i] * a[k-i] where i = 0..k )
*   1.2 end do
*   3.  for k from len(a) to len(a) * 2 - 1 do
*   3.1      C[k] = SUM( a[i] * a[k-i] where i = k - len(a) + 1.. len(a) - 1 )
*   3.2 end do
*   4. C[ len(a) * 2 ] = carry
*
**/
void bgn_z_squ(const UINT32 bgnz_md_id,const BIGINT *a,BIGINT *c0,BIGINT *c1 )
{
    UINT32 *pa; /*shorter of { a, b} */
    UINT32 *pc;
    UINT32 r[3];
    UINT32 a_len;
    UINT32 a_cur;
    UINT32 b_cur;
    UINT32 a_idx;
    UINT32 b_idx;
    UINT32 c_idx;
    UINT32 bit;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: c0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: c1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a == c0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: address of a and c0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: address of a and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( c0 == c1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_squ: address of c0 and c1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_squ: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_squ: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    if ( 0 == a->len )
    {
    bgn_z_set_zero( bgnz_md_id, c0 );
        bgn_z_set_zero( bgnz_md_id, c1 );

        return ;
    }

    pa = ( UINT32 * )&( a->data[ 0 ] );
    pc = &( c0->data[ 0 ] );

    r[ 0 ] = 0;
    r[ 1 ] = 0;
    r[ 2 ] = 0;

    a_len = a->len;

    b_cur = 0;
    for( c_idx = 0, a_cur = 0; a_cur < a_len; c_idx ++, a_cur ++ )
    {
    //fprintf(LOGSTDOUT,"%d = ", c_idx);
    //fprintf(LOGSTDOUT,"xxxx %lx %lx %lx\n", r[2],r[1],r[0]);
    /* trick : here carry = r */
    /* when save the last bit of r and let r >>= 1, then carry = 2 * r + the last bit */
    bit = r[ 0 ] & 1;
    /* r <-- r / 2, and now carry = 2 * r + bit */
    /* note: r[ 2 ] must be zero */
    r[ 0 ] = ((r[ 0 ] >> 1) | (r[ 1 ] << (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] >> 1));
    for( a_idx = a_cur, b_idx = b_cur; b_idx < a_idx; a_idx --, b_idx ++ )
    {
        //fprintf(LOGSTDOUT,"(%d * %d) ", a_idx, b_idx);
        /* r <--  r + a[a_idx] * a[b_idx] */
         mulx( pa[ a_idx ], pa[ b_idx ], r );
    }
    //fprintf(LOGSTDOUT,"yyy %lx %lx %lx\n", r[2],r[1],r[0]);

    //fprintf(LOGSTDOUT,"<<1 ");
    /* r <-- r * 2 | bit which means carry is added only time */
    r[ 2 ] = ((r[ 2 ] << 1) | (r[ 1 ] >> (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] << 1) | (r[ 0 ] >> (WORDSIZE - 1)));
    r[ 0 ] = ((r[ 0 ] << 1) | bit);

    //fprintf(LOGSTDOUT,"zzz %lx %lx %lx\n", r[2],r[1],r[0]);
    if( b_idx == a_idx )
    {
        //fprintf(LOGSTDOUT,"(%d ^ 2)", b_idx);
        /* r <--  r + a[b_idx] ^ 2 */
         mulx( pa[ b_idx ], pa[ b_idx ], r );
    }
    //fprintf(LOGSTDOUT,"\n");
    //fprintf(LOGSTDOUT," = %lx\n", r[ 0 ]);

    //fprintf(LOGSTDOUT,">>> %lx %lx %lx\n", r[2],r[1],r[0]);

        pc[ c_idx ] = r[ 0 ];

        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;
    }

    a_cur --;
    for( b_cur ++; b_cur < a_len && c_idx < INTMAX; c_idx ++, b_cur ++ )
    {
    //fprintf(LOGSTDOUT,"%d = ", c_idx);
    /* trick : here carry = r */
    /* when save the last bit of r and let r >>= 1, then carry = 2 * r + the last bit */
    bit = r[ 0 ] & 1;
    /* r <-- r / 2, and now carry = 2 * r + bit */
    /* note: r[ 2 ] must be zero */
    r[ 0 ] = ((r[ 0 ] >> 1) | (r[ 1 ] << (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] >> 1));
    for( a_idx = a_cur, b_idx = b_cur; b_idx < a_idx; a_idx --, b_idx ++ )
    {
        //fprintf(LOGSTDOUT,"(%d * %d) ", a_idx, b_idx);
        /* r <--  r + a[a_idx] * a[b_idx] */
         mulx( pa[ a_idx ], pa[ b_idx ], r );
    }

    //fprintf(LOGSTDOUT,"<<1 ");
    /* r <-- r * 2 | bit which means carry is added only time */
    r[ 2 ] = ((r[ 2 ] << 1) | (r[ 1 ] >> (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] << 1) | (r[ 0 ] >> (WORDSIZE - 1)));
    r[ 0 ] = ((r[ 0 ] << 1) | bit);

    if( b_idx == a_idx )
    {
        //fprintf(LOGSTDOUT,"(%d ^ 2)", b_idx);
        /* r <--  r + a[b_idx] ^ 2 */
         mulx( pa[ b_idx ], pa[ b_idx ], r );
    }
    //fprintf(LOGSTDOUT,"\n");
    //fprintf(LOGSTDOUT," = %lx\n", r[ 0 ]);

        pc[ c_idx ] = r[ 0 ];

        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;
    }

    /* r[ 2 ] must be zero */
    if( b_cur >= a_len )
    {
    if( c_idx + 1 < INTMAX )
    {
            pc[ c_idx ++ ] = r[ 0 ];
            pc[ c_idx ++ ] = r[ 1 ];

        r[ 0 ] = 0;
        r[ 1 ] = 0;
    }
    else if( c_idx < INTMAX )
    {
            pc[ c_idx ++ ] = r[ 0 ];

            r[ 0 ] = r[ 1 ];
        r[ 1 ] = 0;
    }
    }

    for ( ; c_idx > 0; )
    {
        c_idx --;
           if ( 0 != pc[ c_idx ] )
           {
            c_idx ++;
               break;
           }
    }
    c0->len = c_idx;

    //fprintf(LOGSTDOUT,"c0->len = %d\n", c0->len);

    /* now c_idx = INTMAX and b_cur < a_len */
    pc = &( c1->data[ 0 ] );

    for( c_idx = 0 ; b_cur < a_len; c_idx ++, b_cur ++ )
    {
    //fprintf(LOGSTDOUT,"%d = ", c_idx);
    /* trick : here carry = r */
    /* when save the last bit of r and let r >>= 1, then carry = 2 * r + the last bit */
    bit = (r[ 0 ] & 1);
    /* r <-- r / 2, and now carry = 2 * r + bit */
    /* note: r[ 2 ] must be zero */
    r[ 0 ] = ((r[ 0 ] >> 1) | (r[ 1 ] << (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] >> 1));
    for( a_idx = a_cur, b_idx = b_cur; b_idx < a_idx; a_idx --, b_idx ++ )
    {
        //fprintf(LOGSTDOUT,"(%d * %d) ", a_idx, b_idx);
        /* r <--  r + a[a_idx] * a[b_idx] */
         mulx( pa[ a_idx ], pa[ b_idx ], r );
    }

    //fprintf(LOGSTDOUT,"<<1 ");
    /* r <-- r * 2 | bit which means carry is added only time */
    r[ 2 ] = ((r[ 2 ] << 1) | (r[ 1 ] >> (WORDSIZE - 1)));
    r[ 1 ] = ((r[ 1 ] << 1) | (r[ 0 ] >> (WORDSIZE - 1)));
    r[ 0 ] = ((r[ 0 ] << 1) | bit);

    if( b_idx == a_idx )
    {
        //fprintf(LOGSTDOUT,"(%d ^ 2)", b_idx);
        /* r <--  r + a[b_idx] ^ 2 */
         mulx( pa[ b_idx ], pa[ b_idx ], r );
    }
    //fprintf(LOGSTDOUT,"\n");

        pc[ c_idx ] = r[ 0 ];

        r[ 0 ] = r[ 1 ];
        r[ 1 ] = r[ 2 ];
        r[ 2 ] = 0;
    }

    if( c_idx < INTMAX )
    {
        pc[ c_idx ++ ] = r[ 0 ];

        if( c_idx < INTMAX )
        {
            pc[ c_idx ++ ] = r[ 1 ];
        }
    }

    for ( ; c_idx > 0; )
    {
        c_idx --;
        if ( 0 != pc[ c_idx ] )
        {
            c_idx ++;
            break;
        }
    }
    c1->len = c_idx;

    return ;
}

/**
*       return (q, r) where a = b * q + r
*
*   if a = 0, then q = r = 0.
*   if a > 0 but b = 0, then q =0 and r = a.
*   if a > 0 and b > 0, then a = b * q + r where r < b.
*
* Algorithm:
*   Input : a, b and assume a > b > 0
*   Output: (q, r) where a = b * q + r
*   1. q = 0;
*   2. while a >= b do
*   2.1     let k be the maximum integer such that
*               a > b * 2 ^ k
*   2.2     a = ( a - b * 2 ^ k );
*   2.3     q = q + 2 ^ k;
*   2.4     next while
*   3.  r = a;
*   4.  return (q, r)
*
**/
void bgn_z_div(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b, BIGINT *q,BIGINT *r )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *ta;
    BIGINT *tmp;
    BIGINT *e;
    UINT32 ta_nbits;
    UINT32 b_nbits;
    UINT32 diff_nbits;
    int cmp_result;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: q is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/


#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( a == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: address of a and q conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( a == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: address of a and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: address of b and q conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( q == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_div: address of q and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_div: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_div: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_div: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    /* 0 = b * 0 + 0 */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a ) )
    {
        bgn_z_set_zero( bgnz_md_id,q );
        bgn_z_set_zero( bgnz_md_id,r );

        return ;
    }
    /* a = 0 * 0 + a */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,b ) )
    {
        bgn_z_set_zero( bgnz_md_id,q );
        bgn_z_clone(bgnz_md_id,a, r );

        return ;
    }

    cmp_result = bgn_z_cmp( bgnz_md_id,a, b );
    /* if a < b ,then a = b * 0 + a */
    if ( 0 > cmp_result )
    {
        bgn_z_set_zero( bgnz_md_id,q );
        bgn_z_clone(bgnz_md_id,a, r );

        return ;
    }
    /* if a = b ,then a = b * 1 + 0 */
    if ( 0 == cmp_result )
    {
        bgn_z_set_one( bgnz_md_id,q );
        bgn_z_set_zero( bgnz_md_id,r );

        return ;
    }

    /* here a > b > 0 */
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
    tmp = &buf_2;
    e = &buf_3;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&ta, LOC_BGNZ_0005);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tmp, LOC_BGNZ_0006);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&e, LOC_BGNZ_0007);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_clone(bgnz_md_id,a, ta);

    bgn_z_set_zero( bgnz_md_id,q );
    bgn_z_set_zero( bgnz_md_id,r );

    /* while ta >= b do */
    while ( 0 <= bgn_z_cmp(bgnz_md_id,ta, b) )
    {
        ta_nbits = bgn_z_get_nbits( bgnz_md_id,ta);
        b_nbits  = bgn_z_get_nbits( bgnz_md_id,b );
        diff_nbits = ta_nbits - b_nbits;

        bgn_z_shl_no_shift_out(bgnz_md_id,b, diff_nbits, tmp);

        /* if tmp < t_a, then we are sure  diff_nbits > 0 */
        if ( 0 < bgn_z_cmp(bgnz_md_id,tmp, ta) )
        {
            diff_nbits --;
            bgn_z_shr_no_shift_out(bgnz_md_id,tmp, 1, tmp);
        }

        /* t_a <-- t_a - tmp */
        bgn_z_sub(bgnz_md_id,ta, tmp, ta);

        bgn_z_set_e(bgnz_md_id,e, diff_nbits);

        bgn_z_add(bgnz_md_id,q, e, q);
    }

    bgn_z_clone(bgnz_md_id,ta, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,ta, LOC_BGNZ_0008);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tmp, LOC_BGNZ_0009);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,e, LOC_BGNZ_0010);
#endif/* STATIC_MEMORY_SWITCH */

}

/**
*       let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*           q = (q1, q0) = q1 * 2^ BIGINTSIZE + q0
*
*       return (q, r) where a = b * q + r
*
*   Algorithm:
*   Input: a = (a1,a0), b and assume a1 > 0 and b > 0
*   Output: q = (q1,q0) and r, where a = b * q + r
*   1. q = 0
*   2. let 2 ^ BIGINTSIZE = b * q2 + r2
*      then a = ( a1 * q2 ) * b + ( a0 + a1 * r2)
*   3. while a1 > 0 do
*   3.1     q = q + a1 * q2
*   3.2     a = a0 + a1 * r2
*   3.3     let a = (a1, a0) = a1 * 2^ BIGINTSIZE + a0
*   3.4     next while
*   4. let a0 = q2 * b + r2, then
*       a = q * b + a0 = ( q + q2 ) * b + r2
*   5. q = q + q2
*   6. r = r2
*   7  return (q, r)
*
**/
void bgn_z_ddiv(const UINT32 bgnz_md_id,const BIGINT *a0,const BIGINT *a1,const BIGINT *b,BIGINT *q0,BIGINT *q1,BIGINT *r )
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
    UINT32 carry;
    UINT32 index;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: a0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == a1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: a1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: q0 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    if ( NULL_PTR == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: q1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( b == q0 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of b and q0 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of b and q1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of b and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( q0 == q1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of q0 and q1 conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( q0 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of q0 and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( q1 == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: address of q1 and r conflict.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ddiv: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a0->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ddiv: a0->len = %lx overflow.\n",
                a0->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < a1->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ddiv: a1->len = %lx overflow.\n",
                a1->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_ddiv: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


    /* if b = 0, then exit */
    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,b ) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_ddiv: b is zero.\n");

        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    /* a = 1 * a + 0 */
    if ( EC_TRUE == bgn_z_is_one( bgnz_md_id,b ) )
    {
        bgn_z_clone(bgnz_md_id,a0, q0);

        bgn_z_clone(bgnz_md_id,a1, q1);
        bgn_z_set_zero( bgnz_md_id,r );

        return ;

    }

    if ( EC_TRUE == bgn_z_is_zero( bgnz_md_id,a1 ) )
    {
        bgn_z_set_zero( bgnz_md_id,q1 );
        bgn_z_div(bgnz_md_id,a0, b, q0, r);
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
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&ta0, LOC_BGNZ_0011);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&ta1, LOC_BGNZ_0012);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&q2, LOC_BGNZ_0013);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&r2, LOC_BGNZ_0014);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&M, LOC_BGNZ_0015);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&t0, LOC_BGNZ_0016);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&t1, LOC_BGNZ_0017);
#endif/* STATIC_MEMORY_SWITCH */

    /* compute  2^ BIGINTSIZE - 1 = q2 * b + r2 */
    /* let M = 2^ BIGINTSIZE - 1 */
    for ( index = 0; index < INTMAX; index ++ )
    {
        M->data[ index ] =  ( ~(( UINT32 ) 0) );
    }
    M->len = INTMAX;

    bgn_z_div(bgnz_md_id,M, b, q2, r2);

    /* based  on 2^ BIGINTSIZE - 1 = q2 * b + r2 */
    /* compute 2^ BIGINTSIZE = Q2 * b + R2  */
    /* note: since b >=2, r2 + 1 <= b < ( 2^ BIGINTSIZE ) and */
    /*                    q2 < ( 2^ BIGINTSIZE ) / 2 < M */
    /* i.e, when do r2 + 1 ,there's no carry */

    /* if r2 + 1 = b ,then  2^ BIGINTSIZE = q2 * b + ( r2 + 1 ) = (q2 + 1) * b + 0 */
    /* i.e, Q2 = q2 + 1 < M , R2 = 0 */
    /* else, i.e, r2 + 1 < b, then Q2 = q2, R2 = r2 + 1 */
    bgn_z_sadd(bgnz_md_id,r2, 1, r2);
    if ( 0 == bgn_z_cmp( bgnz_md_id,r2, b ) )
    {
        bgn_z_sadd(bgnz_md_id,q2, 1, q2);
        bgn_z_set_zero( bgnz_md_id,r2 );
    }

    /* now 2^ BIGINTSIZE = q2 * b + r2   */
    /* a = a0 + a1 * 2^ BIGINTSIZE = a0 + a1 *(q2 * b + r2) */
    /* thus a = ( a0 + a1 * r2 ) + ( a1 * q2 ) * b */

    bgn_z_clone(bgnz_md_id,a0, ta0);
    bgn_z_clone(bgnz_md_id,a1, ta1);
    bgn_z_set_zero( bgnz_md_id,q0 );
    bgn_z_set_zero( bgnz_md_id,q1 );

    /* while ta1 > 0 do */
    while ( EC_FALSE == bgn_z_is_zero( bgnz_md_id,ta1 ) )
    {
        /* q = q + ta1 * q2, let t = (t1,t0) = ta1 * q2 */
        bgn_z_mul(bgnz_md_id,ta1, q2, t0, t1);
        carry = bgn_z_add(bgnz_md_id,q0, t0, q0);
        if ( 0 < carry )
        {
            bgn_z_sadd( bgnz_md_id,q1, carry, q1);
        }
        carry = bgn_z_add(bgnz_md_id,q1, t1, q1);
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( 0 < carry )
        {
            sys_log(LOGSTDOUT,"error:bgn_z_ddiv: q is overflow.\n");
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif/*BIGINT_DEBUG_SWITCH*/

        /* ta = ta0 + ta1 * r2, let t = (t1,t0) = ta1 * r2 */
        bgn_z_mul(bgnz_md_id,ta1, r2, t0, t1);
        carry = bgn_z_dadd(bgnz_md_id,t0, t1, ta0, ta0, ta1);
#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
        if ( 0 < carry )
        {
            sys_log(LOGSTDOUT,"error:bgn_z_ddiv: ta is overflow.\n");
            dbg_exit(MD_BGNZ, bgnz_md_id);
        }
#endif/*BIGINT_DEBUG_SWITCH*/
    }

    /* now a = ta0 + q * b */
    /* let ta0 = q2 * b + r2 ,then */
    /* a = ( q2 + q ) * b + r2 */
    bgn_z_div(bgnz_md_id,ta0, b, q2, r2);
    bgn_z_dadd(bgnz_md_id,q0, q1, q2, q0, q1);
    bgn_z_clone(bgnz_md_id,r2, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,ta0, LOC_BGNZ_0018);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,ta1, LOC_BGNZ_0019);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,q2, LOC_BGNZ_0020);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,r2, LOC_BGNZ_0021);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,M, LOC_BGNZ_0022);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,t0, LOC_BGNZ_0023);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,t1, LOC_BGNZ_0024);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}
/**
*
*       return c = GCD(a,b)
*
*   Algorithm:
*   Input: a , b and assume a > 0 and b >= 0
*   Output: c = GCD(a,b)
*   1. while b > 0 do
*   1.1      r = a mod b
*   1.2      a = b
*   1.3      b = r
*   1.4      next while
*   2. set c = a
*   3. return c
*
**/
void bgn_z_gcd(const UINT32 bgnz_md_id,const BIGINT *a,const BIGINT *b,BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
#endif /* STACK_MEMORY_SWITCH */
    BIGINT *ta;
    BIGINT *tb;
    BIGINT *tr;
    BIGINT *tq;
    BIGINT *tmp;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_gcd: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_gcd: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_gcd: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_gcd: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_gcd: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_gcd: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/


#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    ta = &buf_1;
    tb = &buf_2;
    tq = &buf_3;
    tr = &buf_4;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&ta, LOC_BGNZ_0025);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tb, LOC_BGNZ_0026);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tq, LOC_BGNZ_0027);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tr, LOC_BGNZ_0028);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_clone(bgnz_md_id,a, ta);
    bgn_z_clone(bgnz_md_id,b, tb);

    while ( EC_FALSE == bgn_z_is_zero( bgnz_md_id,tb ) )
    {
        bgn_z_div(bgnz_md_id,ta, tb, tq, tr);
        tmp = ta;
        ta = tb;
        tb = tr;
        tr = tmp;
    }

    bgn_z_clone(bgnz_md_id,ta, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,ta, LOC_BGNZ_0029);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tb, LOC_BGNZ_0030);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tq, LOC_BGNZ_0031);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tr, LOC_BGNZ_0032);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*
*   let a = q * 2^e where q is odd and e >=0
*
**/
void bgn_z_evens(const UINT32 bgnz_md_id,const BIGINT *a, UINT32 *e,BIGINT *q)
{
    UINT32 bit_idx;
    UINT32 the_bit;
    UINT32 a_nbits;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_evens: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == e )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_evens: e is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == q )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_evens: q is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_evens: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_evens: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

    a_nbits = bgn_z_get_nbits(bgnz_md_id, a);

    for ( bit_idx = 0; bit_idx < a_nbits; bit_idx++ )
    {
        the_bit = bgn_z_get_bit(bgnz_md_id, a, bit_idx);
        if ( 1 == the_bit )
        {
            break;
        }
    }

    bgn_z_shr_no_shift_out(bgnz_md_id, a, bit_idx, q);

    *e = bit_idx;

    return ;
}

/**
*
*   return r = a mod b
*
*   if b = 0, then r = a,
*   if b > 0, then let a = b * q + r where r < b, return r.
*
**/
void bgn_z_mod(const UINT32 bgnz_md_id,const BIGINT *a, const BIGINT *b,BIGINT *r)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
#endif /* STACK_MEMORY_SWITCH */
    BIGINT *q;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mod: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mod: b is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_mod: r is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mod: bgnz module #0x%lx not started.\n",
                bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BGN_LEN_CHECK_SWITCH )
    if ( INTMAX < a->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mod: a->len = %lx overflow.\n",
                a->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( INTMAX < b->len )
    {
        sys_log(LOGSTDOUT,
                "error:bgn_z_mod: b->len = %lx overflow.\n",
                b->len);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BGN_LEN_CHECK_SWITCH*/

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    q = &buf_1;
#endif /* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&q, LOC_BGNZ_0033);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_div(bgnz_md_id, a, b, q, r);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,q, LOC_BGNZ_0034);
#endif/* STATIC_MEMORY_SWITCH */
    return ;
}

/**
*
*
*   here MUST be n > a > 0 and GCD(a,n) = 1, and return
*       c = ( 1 / a ) mod n
*
*   note: n must be odd in algorithm below!
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
/**
*
*   note about bgn_z_inv_for_crt:
*       1. must have GCD(a, n) = 1
*       2. must be called by bgn_z_inv interface only
*
**/
static EC_BOOL bgn_z_inv_for_crt(const UINT32 bgnz_md_id,const BIGINT *a, const BIGINT *n,BIGINT *c )
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *u;
    BIGINT *v;
    BIGINT *A;
    BIGINT *C;

    const BIGINT *p;

    BIGINT *t;

    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: bgnz module #0x%lx not started.\n",bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if( EC_FALSE == bgn_z_is_odd(bgnz_md_id, n) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: not support even module yet\n");
    return ( EC_FALSE );
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    t = &buf_1;
    u = &buf_2;
    v = &buf_3;
    A = &buf_4;
    C = &buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&u, LOC_BGNZ_0035);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&v, LOC_BGNZ_0036);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&A, LOC_BGNZ_0037);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&C, LOC_BGNZ_0038);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&t, LOC_BGNZ_0039);
#endif/* STATIC_MEMORY_SWITCH */

    p = n;

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
                bgn_z_sub(bgnz_md_id, p, C, t);
                bgn_z_add(bgnz_md_id, A, t, A );
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
                bgn_z_sub(bgnz_md_id, p, A, t);
                bgn_z_add(bgnz_md_id, C, t, C );
            }
            /* if C >= A, then do */
            else
            {
                /* C <-- C - A */
                bgn_z_sub( bgnz_md_id, C, A, C );
            }
        }
    }

    bgn_z_clone( bgnz_md_id, C, c );

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,u, LOC_BGNZ_0040);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,v, LOC_BGNZ_0041);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,A, LOC_BGNZ_0042);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,C, LOC_BGNZ_0043);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,t, LOC_BGNZ_0044);
#endif/* STATIC_MEMORY_SWITCH */

    return ( EC_TRUE );
}

/**
*
*   Chinese Remainder Theorem.
*
*   return x such that
*       x = x1 mod m1
*       x = x2 mod m2
*   and 0 <= x < m where m = m1*m2
*
*   Algorithm:
*   Input: x1,m1,x2,m2
*   Output: x
*   1. compute u,v such that u*m1 + v*m2 = 1;
*       note: u = m1^(-1) mod m2
*             v = m2^(-1) mod m1
*   2. compute x = u*m1*x2 + v*m2*x1 mod (m1*m2);
*   3. return x;
*
*   note:
*       1. must have m1*m2 < 2^BIGINTSIZE
*
**/
void bgn_z_crt(const UINT32 bgnz_md_id,const BIGINT *x1, const BIGINT *m1,const BIGINT *x2, const BIGINT *m2,BIGINT *x, BIGINT *m)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
    BIGINT buf_6;
    BIGINT buf_7;
    BIGINT buf_8;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *u;
    BIGINT *v;
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *q0;
    BIGINT *q1;
    BIGINT *tx;
    BIGINT *tm;
    BIGINT *t;

    UINT32 carry;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == x1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: x1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == m1 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: m1 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == x2 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: x2 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == m2 )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: m2 is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    if ( NULL_PTR == x )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: x is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == m )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: m is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_inv_for_crt: bgnz module #0x%lx not started.\n",bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, m1) )
    {
        bgn_z_clone(bgnz_md_id, x2, x);
        bgn_z_clone(bgnz_md_id, m2, m);
        return ;
    }

    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, m2) )
    {
        bgn_z_clone(bgnz_md_id, x1, x);
        bgn_z_clone(bgnz_md_id, m1, m);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    u  = &buf_1;
    v  = &buf_2;
    c0 = &buf_3;
    c1 = &buf_4;
    q0 = &buf_5;
    q1 = &buf_6;
    tx = &buf_7;
    tm = &buf_8;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&u, LOC_BGNZ_0045);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&v, LOC_BGNZ_0046);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&c0, LOC_BGNZ_0047);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&c1, LOC_BGNZ_0048);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&q0, LOC_BGNZ_0049);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&q1, LOC_BGNZ_0050);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tx, LOC_BGNZ_0051);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&tm, LOC_BGNZ_0052);
#endif/* STATIC_MEMORY_SWITCH */

    bgn_z_mul(bgnz_md_id, m1, m2, c0, c1);
    if ( EC_FALSE == bgn_z_is_zero(bgnz_md_id, c1) )
    {
        sys_log(LOGSTDERR,"error:bgn_z_crt: m1*m2 overflow.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    /*set tm = m1*m2*/
    t  = tm;
    tm = c0;
    c0 = t;

    /*u = m1^(-1) mod m2*/
    if( EC_FALSE == bgn_z_inv_for_crt(bgnz_md_id, m1, m2, u) )
    {
    sys_log(LOGSTDERR,"error:bgn_z_crt: bgn_z_inv_for_crt return false when try 1/m1 mod m2\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    /*v = m2^(-1) mod m1*/
    if( EC_FALSE == bgn_z_inv_for_crt(bgnz_md_id, m2, m1, v) )
    {
    sys_log(LOGSTDERR,"error:bgn_z_crt: bgn_z_inv_for_crt return false when try 1/m2 mod m1\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }

    /*compute u = u*m1*x2 mod (m1*m2)*/
    bgn_z_mul(bgnz_md_id, u, m1, c0, c1);
    bgn_z_ddiv(bgnz_md_id, c0, c1, tm, q0, q1, u);
    bgn_z_mul(bgnz_md_id, u, x2, c0, c1);
    bgn_z_ddiv(bgnz_md_id, c0, c1, tm, q0, q1, u);

    /*compute v = v*m2*x1 mod (m1*m2)*/
    bgn_z_mul(bgnz_md_id, v, m2, c0, c1);
    bgn_z_ddiv(bgnz_md_id, c0, c1, tm, q0, q1, v);
    bgn_z_mul(bgnz_md_id, v, x1, c0, c1);
    bgn_z_ddiv(bgnz_md_id, c0, c1, tm, q0, q1, v);

    /*compute x = u + v mod (m1*m2)*/
    carry = bgn_z_add(bgnz_md_id, u, v, tx);
    if ( 0 < carry )
    {
        /*now: u + v = x + 2^BIGINTSIZE and means u or v and tm is quite close to 2^BIGINTSIZE*/
        /*so that, u + v = (x + 1) + ((2^BIGINTSIZE - 1)- (m1*m2)) mod (m1*m2)*/
        /*1. set q0 = 2^BIGINTSIZE -1*/
        bgn_z_set_max(bgnz_md_id, q0);
        /*2. compute q1 = q0 - tm*/
        bgn_z_sub(bgnz_md_id, q0, tm, q1);
        /*3. compute q0 = x + 1*/
        bgn_z_sadd(bgnz_md_id, tx, 1, q0);
        /*4. compute x = q0 + q1*/
        bgn_z_add(bgnz_md_id, q0, q1, tx);
    }
    bgn_z_mod(bgnz_md_id, tx, tm, x);

    /*set m = tm*/
    bgn_z_clone(bgnz_md_id, tm, m);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,u, LOC_BGNZ_0053);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,v, LOC_BGNZ_0054);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,c0, LOC_BGNZ_0055);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,c1, LOC_BGNZ_0056);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,q0, LOC_BGNZ_0057);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,q1, LOC_BGNZ_0058);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tx, LOC_BGNZ_0059);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,tm, LOC_BGNZ_0060);
#endif/* STATIC_MEMORY_SWITCH */

    return ;
}

/**
*   Squroot Ceil of a
*
*   return c such that c^2 <= a < (c+1)^2
*
**/
void bgn_z_sqrt_ceil(const UINT32 bgnz_md_id,const BIGINT *a, BIGINT *c)
{
#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    BIGINT buf_1;
    BIGINT buf_2;
    BIGINT buf_3;
    BIGINT buf_4;
    BIGINT buf_5;
#endif/* STACK_MEMORY_SWITCH */

    BIGINT *left;
    BIGINT *right;
    BIGINT *mid;
    BIGINT *c0;
    BIGINT *c1;
    BIGINT *t;

    UINT32 a_nbits;
    UINT32 left_nbits;
    UINT32 right_nbits;

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sqrt_ceil: a is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sqrt_ceil: c is NULL_PTR.\n");
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

#if ( SWITCH_ON == BIGINT_DEBUG_SWITCH )
    if ( BGNZ_MD_ID_CHECK_INVALID(bgnz_md_id) )
    {
        sys_log(LOGSTDOUT,"error:bgn_z_sqrt_ceil: bgnz module #0x%lx not started.\n",bgnz_md_id);
        dbg_exit(MD_BGNZ, bgnz_md_id);
    }
#endif/*BIGINT_DEBUG_SWITCH*/

    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, a) )
    {
        bgn_z_set_zero(bgnz_md_id, c);
        return ;
    }

#if ( SWITCH_ON == STACK_MEMORY_SWITCH )
    left  = &buf_1;
    right = &buf_2;
    mid   = &buf_3;
    c0    = &buf_4;
    c1    = &buf_5;
#endif/* STACK_MEMORY_SWITCH */

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&left, LOC_BGNZ_0061);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&right, LOC_BGNZ_0062);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&mid, LOC_BGNZ_0063);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&c0, LOC_BGNZ_0064);
    alloc_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,&c1, LOC_BGNZ_0065);
#endif/* STATIC_MEMORY_SWITCH */


    a_nbits = bgn_z_get_nbits(bgnz_md_id, a);

    left_nbits = (a_nbits - 1) / 2;
    right_nbits = left_nbits + 1;

    bgn_z_set_e(bgnz_md_id, left, left_nbits);
    bgn_z_set_e(bgnz_md_id, right, right_nbits);

    /*now left  <= c < right*/
    bgn_z_add(bgnz_md_id, left, right, mid);
    bgn_z_shr_lesswordsize(bgnz_md_id, mid, 1, mid);

    while ( 0 < bgn_z_cmp(bgnz_md_id, mid, left) )
    {
        bgn_z_squ(bgnz_md_id, mid, c0, c1);

        if ( 0 < bgn_z_cmp(bgnz_md_id, c0, a) )
        {
            t = right;
            right = mid;
            mid = t;
        }
        else
        {
            t = left;
            left = mid;
            mid = t;
        }

        bgn_z_add(bgnz_md_id, left, right, mid);
        bgn_z_shr_lesswordsize(bgnz_md_id, mid, 1, mid);
    }

    bgn_z_clone(bgnz_md_id, mid, c);

#if ( SWITCH_ON == STATIC_MEMORY_SWITCH )
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,left, LOC_BGNZ_0066);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,right, LOC_BGNZ_0067);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,mid, LOC_BGNZ_0068);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,c0, LOC_BGNZ_0069);
    free_static_mem(MD_BGNZ, bgnz_md_id, MM_BIGINT,c1, LOC_BGNZ_0070);
#endif/* STATIC_MEMORY_SWITCH */


    return ;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
