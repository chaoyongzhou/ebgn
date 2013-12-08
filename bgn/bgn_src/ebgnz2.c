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
#include "bgnz2.h"
#include "ebgnz.h"
#include "ebgnz2.h"

#include "debug.h"

#include "print.h"

static EBGNZ2_MD g_ebgnz2_md[ MAX_NUM_OF_EBGNZ2_MD ];
static EC_BOOL  g_ebgnz2_md_init_flag = EC_FALSE;

static EBGN  *g_ebgn_z2_premul_tbl[ MAX_NUM_OF_EBGNZ2_MD ][ MAX_SIZE_OF_EBGNZ2_PREMUL_TABLE ][ 2 ];

static UINT32 ebgn_z2_item_destory(const UINT32 ebgnz2_md_id, EBGN_ITEM *item);
static UINT32 ebgn_z2_item_clone(const UINT32 ebgnz2_md_id, const EBGN_ITEM *item_a, EBGN_ITEM *item_c);

/**
*   for test only
*
*   to query the status of EBGNZ2 Module
*
**/
void print_ebgn_z2_status()
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 index;

    if ( EC_FALSE == g_ebgnz2_md_init_flag )
    {

        sys_log(LOGSTDOUT,"no EBGNZ2 Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_EBGNZ2_MD; index ++ )
    {
        ebgnz2_md = &(g_ebgnz2_md[ index ]);

        if ( 0 < ebgnz2_md->usedcounter )
        {
            sys_log(LOGSTDOUT,
                    "EBGNZ2 Module # %ld : %ld refered, refer EBGN Module : %ld, BGNZ Module : %ld,BGNZ2 Module : %ld\n",
                    index,
                    ebgnz2_md->usedcounter,
                    ebgnz2_md->ebgnz_md_id,
                    ebgnz2_md->bgnz_md_id,
                    ebgnz2_md->bgnz2_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed EBGNZ2 module
*
*
**/
UINT32 ebgn_z2_free_module_static_mem(const UINT32 ebgnz2_md_id)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_free_module_static_mem: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;
    bgnz_md_id  = ebgnz2_md->bgnz_md_id;
    bgnz2_md_id  = ebgnz2_md->bgnz2_md_id;

    free_module_static_mem(MD_EBGNZ2, ebgnz2_md_id);

    ebgn_z_free_module_static_mem(ebgnz_md_id);
    bgn_z_free_module_static_mem(bgnz_md_id);
    bgn_z2_free_module_static_mem(bgnz2_md_id);

    return 0;
}
/**
*
* start EBGNZ2 module
*
**/
UINT32 ebgn_z2_start( )
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz2_md_id;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;
    UINT32 bgnz2_md_id;

    UINT32 index;

    /* if this is the 1st time to start EBGNZ2 module, then */
    /* initialize g_ebgnz2_md */
    if ( EC_FALSE ==  g_ebgnz2_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_EBGNZ2_MD; index ++ )
        {
            ebgnz2_md = &(g_ebgnz2_md[ index ]);

            ebgnz2_md->usedcounter   = 0;

            ebgnz2_md->ebgnz_md_id = ERR_MODULE_ID;
            ebgnz2_md->bgnz_md_id  = ERR_MODULE_ID;
            ebgnz2_md->bgnz2_md_id = ERR_MODULE_ID;
        }

        /*register all functions of EBGNZ2 module to DBG module*/
        //dbg_register_func_addr_list(g_ebgnz2_func_addr_list, g_ebgnz2_func_addr_list_len);

        g_ebgnz2_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_EBGNZ2_MD; index ++ )
    {
        ebgnz2_md = &(g_ebgnz2_md[ index ]);

        if ( 0 == ebgnz2_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_EBGNZ2_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    ebgnz_md_id = ERR_MODULE_ID;
    bgnz_md_id  = ERR_MODULE_ID;
    bgnz2_md_id = ERR_MODULE_ID;

    /* initilize new one EBGNZ2 module */
    ebgnz2_md_id = index;
    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);

    init_static_mem();

    for ( index = 0; index < MAX_SIZE_OF_EBGNZ2_PREMUL_TABLE; index ++ )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, &(g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ index ][ 0 ]), LOC_EBGNZ2_0001);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, &(g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ index ][ 1 ]), LOC_EBGNZ2_0002);
    }

    ebgnz_md_id = ebgn_z_start();
    bgnz_md_id  = bgn_z_start();
    bgnz2_md_id = bgn_z2_start();

    ebgnz2_md->ebgnz_md_id = ebgnz_md_id;
    ebgnz2_md->bgnz_md_id  = bgnz_md_id;
    ebgnz2_md->bgnz2_md_id = bgnz2_md_id;
    ebgnz2_md->usedcounter = 1;

    return ( ebgnz2_md_id );
}

/**
*
* end EBGNZ2 module
*
**/
void ebgn_z2_end(const UINT32 ebgnz2_md_id)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;
    UINT32  bgnz_md_id;
    UINT32 bgnz2_md_id;

    UINT32 index;

    if ( MAX_NUM_OF_EBGNZ2_MD < ebgnz2_md_id )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_end: ebgnz2_md_id = %ld is overflow.\n",ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ebgnz2_md->usedcounter )
    {
        ebgnz2_md->usedcounter --;
        return ;
    }

    if ( 0 == ebgnz2_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_end: ebgnz2_md_id = %ld is not started.\n",ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;
    bgnz_md_id  = ebgnz2_md->bgnz_md_id;
    bgnz2_md_id = ebgnz2_md->bgnz2_md_id;

    for ( index = 0; index < MAX_SIZE_OF_EBGNZ2_PREMUL_TABLE; index ++ )
    {
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ index ][ 0 ]), LOC_EBGNZ2_0003);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ index ][ 1 ]), LOC_EBGNZ2_0004);
    }

    ebgn_z_end( ebgnz_md_id);
    bgn_z_end( bgnz_md_id);
    bgn_z2_end( bgnz2_md_id);

    /* free module : */
    //ebgn_z2_free_module_static_mem(ebgnz2_md_id);
    ebgnz2_md->ebgnz_md_id = ERR_MODULE_ID;
    ebgnz2_md->bgnz_md_id  = ERR_MODULE_ID;
    ebgnz2_md->bgnz2_md_id = ERR_MODULE_ID;
    ebgnz2_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

/**
*
*   alloc a EBGN type node from EBGNZ2 space
*
**/
UINT32 ebgn_z2_alloc_ebgn(const UINT32 ebgnz2_md_id,EBGN **ppebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ppebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_alloc_ebgn: ppebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_alloc_ebgn: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, ppebgn, LOC_EBGNZ2_0005);
    EBGN_INIT(*ppebgn);

    return ( 0 );
}

/**
*
*   free the EBGN type node from EBGNZ2 space
*
**/
UINT32 ebgn_z2_free_ebgn(const UINT32 ebgnz2_md_id,EBGN *ebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_free_ebgn: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_free_ebgn: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, ebgn, LOC_EBGNZ2_0006);

    return ( 0 );
}

/**
*
*   destory the whole item,including its content bgn and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
static UINT32 ebgn_z2_item_destory(const UINT32 ebgnz2_md_id, EBGN_ITEM *item)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_item_destory: ebgnz module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item), LOC_EBGNZ2_0007);
    EBGN_ITEM_BGN(item) = NULL_PTR;

    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item, LOC_EBGNZ2_0008);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_z2_clean(const UINT32 ebgnz2_md_id, EBGN *ebgn)
{
    EBGN_ITEM *item;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_clean: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_clean: ebgnz module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    item = EBGN_FIRST_ITEM(ebgn);

    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        EBGN_ITEM_DEL(item);
        ebgn_z2_item_destory(ebgnz2_md_id, item);

        item = EBGN_FIRST_ITEM(ebgn);
    }

    EBGN_INIT(ebgn);

    return ( 0 );
}

/**
*
*   the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_z2_destroy(const UINT32 ebgnz2_md_id, EBGN *ebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_destroy: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_destroy: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgn_z2_clean(ebgnz2_md_id,ebgn);

    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN, ebgn, LOC_EBGNZ2_0009);

    return ( 0 );
}

static UINT32 ebgn_z2_item_clone(const UINT32 ebgnz2_md_id, const EBGN_ITEM *item_a, EBGN_ITEM *item_c)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    BIGINT  *bgn;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_item_clone: item_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_item_clone: item_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_item_clone: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( item_a == item_c )
    {
        return ( 0 );
    }

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    EBGN_ITEM_INIT(item_c);

    /*clone bgn*/
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn, LOC_EBGNZ2_0010);
    bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn);

    EBGN_ITEM_BGN(item_c) = bgn;

    return ( 0 );
}

/**
*
*   des = src
*
**/
UINT32 ebgn_z2_clone(const UINT32 ebgnz2_md_id,const EBGN *src, EBGN *des)
{
    EBGN_ITEM *item_src;
    EBGN_ITEM *item_des;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_clone: src is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == des )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_clone: des is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_clone: ebgnz module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( src == des )
    {
        return ( 0 );
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(des) )
    {
        ebgn_z_clean(ebgnz2_md_id, des);
    }

    item_src = EBGN_FIRST_ITEM(src);

    while ( item_src != EBGN_NULL_ITEM(src) )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_des, LOC_EBGNZ2_0011);

        ebgn_z2_item_clone(ebgnz2_md_id, item_src, item_des);

        EBGN_ADD_ITEM_TAIL(des, item_des);

        item_src = EBGN_ITEM_NEXT(item_src);
    }

    EBGN_LEN(des) = EBGN_LEN(src);
    EBGN_SGN(des) = EBGN_SGN(src);

    return ( 0 );
}

/**
*   compare a and b
*   if a > b, then return 1
*   if a = b, then return 0
*   if a < b, then return -1
**/
int ebgn_z2_cmp(const UINT32 ebgnz2_md_id,const EBGN * a,const EBGN *b)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_cmp: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_cmp: b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_cmp: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    return ebgn_z_cmp(ebgnz_md_id, a, b);
}

/**
*
*       move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_z2_move(const UINT32 ebgnz2_md_id, EBGN *ebgn_a, EBGN *ebgn_c)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_move: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_move: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_move: ebgnz module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( ebgn_a != ebgn_c )
    {
        ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_a) )
        {
            EBGN_ITEM_HEAD(ebgn_c)->next = EBGN_ITEM_HEAD(ebgn_a)->next;
            EBGN_ITEM_HEAD(ebgn_c)->next->prev = EBGN_ITEM_HEAD(ebgn_c);

            EBGN_ITEM_HEAD(ebgn_c)->prev = EBGN_ITEM_HEAD(ebgn_a)->prev;
            EBGN_ITEM_HEAD(ebgn_c)->prev->next = EBGN_ITEM_HEAD(ebgn_c);

            EBGN_LEN(ebgn_c) = EBGN_LEN(ebgn_a);
            EBGN_SGN(ebgn_c) = EBGN_SGN(ebgn_a);

            EBGN_INIT(ebgn_a);
        }
    }
    return ( 0 );
}


/**
*
*   if src = 0, then return EC_TRUE
*   if src !=0, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_zero(const UINT32 ebgnz2_md_id,const EBGN * src)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_is_zero: src is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_is_zero: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    return ebgn_z_is_zero( ebgnz_md_id, src );
}

/**
*
*   if src = 1, then return EC_TRUE
*   if src !=1, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_one(const UINT32 ebgnz2_md_id,const EBGN * src)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_is_one: src is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_is_one: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    return ebgn_z_is_one( ebgnz_md_id, src );
}

/**
*
*   if src is odd, then return EC_TRUE
*   if src is even, then return EC_FALSE
*
**/
EC_BOOL ebgn_z2_is_odd(const UINT32 ebgnz2_md_id,const EBGN *src)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == src )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_is_odd: src is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_is_odd: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    return ebgn_z_is_odd( ebgnz_md_id , src );
}

/**
*
*   set a = 0
**/
UINT32 ebgn_z2_set_zero(const UINT32 ebgnz2_md_id,EBGN * a)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_zero: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_zero: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    return ebgn_z2_set_word( ebgnz2_md_id, 0, a );
}

/**
*
*   set a = 1
**/
UINT32 ebgn_z2_set_one(const UINT32 ebgnz2_md_id,EBGN * a)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_one: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_one: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    return ebgn_z2_set_word( ebgnz2_md_id, 1, a );
}

/**
*
*   set a = n
**/
UINT32 ebgn_z2_set_word(const UINT32 ebgnz2_md_id,const UINT32 n, EBGN *a)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    BIGINT *bgn;
    EBGN_ITEM *item;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_word: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_word: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(a) )
    {
        ebgn_z2_clean(ebgnz2_md_id, a);
    }

    if ( 0 == n )
    {
        return ( 0 );
    }

    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ2_0012);
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn, LOC_EBGNZ2_0013);

    bgn_z_set_word(bgnz_md_id, bgn, n);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_HEAD(a, item);
    EBGN_SET_LEN(a, 1);

    return ( 0 );
}

/**
*
*   set a = n
**/
UINT32 ebgn_z2_set_n(const UINT32 ebgnz2_md_id,const BIGINT *n, EBGN *a)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    EBGN_ITEM *item;
    BIGINT *bgn;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_n: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_n: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(a) )
    {
        ebgn_z2_clean(ebgnz2_md_id, a);
    }

    if( EC_TRUE == bgn_z_is_zero(bgnz_md_id, n) )
    {
        return ( 0 );
    }

    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ2_0014);
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn, LOC_EBGNZ2_0015);

    bgn_z_clone(bgnz_md_id, n, bgn);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_HEAD(a, item);
    EBGN_SET_LEN(a, 1);

    return ( 0 );
}

/**
*
*   return deg(a)
*
*   let a = SUM(a_i * x^i, i =0..m) over Z_2[x]
*   if a  = 0, then return ( 0 )
*   if a != 0, then return ( m )
**/
UINT32 ebgn_z2_deg(const UINT32 ebgnz2_md_id,const EBGN *a)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

    UINT32 nbits;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_deg: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_deg: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    ebgn_z_get_nbits( ebgnz_md_id, a, &nbits );
    if ( 0 == nbits )
    {
        return ( 0 );
    }

    return ( nbits - 1 );
}

/**
*   return ebgn = x ^ e over Z_2[x]
*   where e = 0,1,...
*
**/
UINT32 ebgn_z2_set_e(const UINT32 ebgnz2_md_id,EBGN *ebgn,const UINT32 e)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    EBGN_ITEM *item;
    BIGINT *bgn;

    UINT32 len;
    UINT32 offset;
    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_e: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_e: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn) )
    {
        ebgn_z2_clean(ebgnz2_md_id, ebgn);
    }

    len = ( e  )/ BIGINTSIZE;
    offset = e % BIGINTSIZE;

    /* only loop len - 1 times and hope to hanlde len = 0 case */
    for( index = 0; index < len; index ++ )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ2_0016);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn, LOC_EBGNZ2_0017);

        bgn_z_set_zero(bgnz_md_id, bgn);

        EBGN_ITEM_BGN(item) = bgn;
        EBGN_ADD_ITEM_TAIL(ebgn, item);
    }

    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ2_0018);
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn, LOC_EBGNZ2_0019);

    bgn_z_set_e(bgnz_md_id, bgn, offset);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_TAIL(ebgn, item);
    EBGN_SET_LEN(ebgn, len + 1);

    return ( 0 );
}

/**
*
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 ebgn_z2_get_bit(const UINT32 ebgnz2_md_id,const EBGN *a, const UINT32 nthbit)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_get_bit: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_get_bit: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;

    return ebgn_z_get_bit(ebgnz_md_id, a, nthbit);
}

/**
*   let a = SUM(a_i * x^i, where i = 0..EBGNZSIZE - 1,a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z2_set_bit(const UINT32 ebgnz2_md_id,EBGN *ebgn_a, const UINT32 nthbit)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;


    EBGN_ITEM *item_a;
    BIGINT *bgn_of_item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_set_bit: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_set_bit: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    bgn_offset = nthbit / BIGINTSIZE;
    bit_offset = nthbit % BIGINTSIZE;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bgn_offset )
    {
        bgn_offset --;
        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( 0 < bgn_offset )
    {
        bgn_offset --;

        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_a, LOC_EBGNZ2_0020);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_a, LOC_EBGNZ2_0021);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_a);

        EBGN_ITEM_BGN(item_a) = bgn_of_item_a;
        EBGN_ADD_ITEM_TAIL(ebgn_a, item_a);
        EBGN_INC_LEN(ebgn_a);
    }

    if( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        bgn_z_set_bit(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset);
    }
    else
    {
        /* reach here when ebgn_a = 0 */
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_a, LOC_EBGNZ2_0022);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_a, LOC_EBGNZ2_0023);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_a);
        bgn_z_set_bit(bgnz_md_id, bgn_of_item_a, bit_offset);

        EBGN_ITEM_BGN(item_a) = bgn_of_item_a;
        EBGN_ADD_ITEM_TAIL(ebgn_a, item_a);
        EBGN_INC_LEN(ebgn_a);
    }

    return ( 0 );
}

/**
*   let a = SUM(a_i * x^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z2_clear_bit(const UINT32 ebgnz2_md_id,EBGN *ebgn_a, const UINT32 nthbit)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;


    EBGN_ITEM *item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_clear_bit: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_clear_bit: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    bgn_offset = nthbit / BIGINTSIZE;
    bit_offset = nthbit % BIGINTSIZE;

    if( bgn_offset >= EBGN_LEN(ebgn_a) )
    {
        return ( 0 );
    }

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bgn_offset )
    {
        bgn_offset --;
        item_a = EBGN_ITEM_NEXT(item_a);
    }

    bgn_z_clear_bit(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset);

    item_a = EBGN_LAST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_a)) )
    {
        EBGN_ITEM_DEL(item_a);

        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_a), LOC_EBGNZ2_0024);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item_a, LOC_EBGNZ2_0025);

        EBGN_DEC_LEN(ebgn_a);

        item_a = EBGN_LAST_ITEM(ebgn_a);
    }

    return ( 0 );
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
UINT32 ebgn_z2_shr(const UINT32 ebgnz2_md_id,const EBGN *ebgn_a, const UINT32 nbits,EBGN *ebgn_c)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 ebgnz_md_id;
    UINT32 bgnz_md_id;

    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

    UINT32 bgn_offset;
    UINT32 bit_offset;

    UINT32 nbits_of_ebgn_a;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_shr: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_shr: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_shr: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z2_is_zero(ebgnz2_md_id, ebgn_a) )
    {
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
                return ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
        }
        return ( 0 );
    }

    if( 0 == nbits )
    {
        if( ebgn_a == ebgn_c )
        {
                return ( 0 );
        }
        return ebgn_z2_clone(ebgnz2_md_id, ebgn_a, ebgn_c);
    }

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    ebgnz_md_id = ebgnz2_md->ebgnz_md_id;
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    ebgn_z_get_nbits(ebgnz_md_id, ebgn_a, &nbits_of_ebgn_a);
    if( nbits_of_ebgn_a <= nbits )
    {
        return ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
    }

    /* now nbits_of_ebgn_a > nbits */

    ebgn_z2_alloc_ebgn(ebgnz2_md_id, &ebgn_tmp);

    bgn_offset = nbits / BIGINTSIZE;
    bit_offset = nbits % BIGINTSIZE;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bgn_offset )
    {
        bgn_offset --;
        item_a = EBGN_ITEM_NEXT(item_a);
    }

    /* now bgn_offset = 0 */
    if( 0 == bit_offset )
    {
        while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0026);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0027);

                bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);

                item_a = EBGN_ITEM_NEXT(item_a);
        }
    }
    /* 0 < bit_offset */
    else
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ2_0028);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ2_0029);

        while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EBGN_ITEM_NEXT(item_a) != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0030);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0031);

                bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_c0);
                bgn_z_shl_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(EBGN_ITEM_NEXT(item_a)), BIGINTSIZE - bit_offset, bgn_c1);
                bgn_z_bit_or(ebgnz_md_id, bgn_c0, bgn_c1, bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);

                item_a = EBGN_ITEM_NEXT(item_a);
        }

        /* handle the last item ( highest item ) */
        if( item_a != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0032);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0033);

                bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);
        }

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
        {
                EBGN_ITEM_DEL(item_tmp);

                free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ2_0034);
                free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ2_0035);

                EBGN_DEC_LEN(ebgn_tmp);

                item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        }

        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ2_0036);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ2_0037);
    }

    EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
    }

    ebgn_z2_move(ebgnz2_md_id, ebgn_tmp, ebgn_c);
    ebgn_z2_free_ebgn(ebgnz2_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*   return c = ( a  * x ^ {nbits} ) mod x ^{EBGNZSIZE} over Z_2[x]
*   and the shifted out part is NOT returned.
*
*   Note:
*       1. address of c not equal to address of a
*
**/
UINT32 ebgn_z2_shl(const UINT32 ebgnz2_md_id,const EBGN *ebgn_a, const UINT32 nbits,EBGN *ebgn_c)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;

    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_shl: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_shl: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_shl: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z2_is_zero(ebgnz2_md_id, ebgn_a) )
    {
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
                return ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
        }
        return ( 0 );
    }

    if( 0 == nbits )
    {
        if( ebgn_a == ebgn_c )
        {
                return ( 0 );
        }
        return ebgn_z2_clone(ebgnz2_md_id, ebgn_a, ebgn_c);
    }

    /* now a > 0 and nbits > 0 */
    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    ebgn_z2_alloc_ebgn(ebgnz2_md_id, &ebgn_tmp);

    bgn_offset = nbits / BIGINTSIZE;
    bit_offset = nbits % BIGINTSIZE;

    if( 0 < bit_offset )
    {
        item_a = EBGN_LAST_ITEM(ebgn_a);
        /* handle the last item ( highest item ) */
        if( item_a != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0038);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0039);

                bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), BIGINTSIZE - bit_offset, bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);
        }

        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ2_0040);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ2_0041);

        while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EBGN_ITEM_PREV(item_a) != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0042);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0043);

                bgn_z_shl_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_c0);
                bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(EBGN_ITEM_PREV(item_a)), BIGINTSIZE - bit_offset, bgn_c1);
                bgn_z_bit_or(bgnz_md_id, bgn_c0, bgn_c1, bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);

                item_a = EBGN_ITEM_PREV(item_a);
        }

        /* handle the first item ( lowest item ) */
        if( item_a != EBGN_NULL_ITEM(ebgn_a) )
        {
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0044);
                alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0045);

                bgn_z_shl_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_of_item_tmp);

                EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
                EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
                EBGN_INC_LEN(ebgn_tmp);
        }

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
        {
                EBGN_ITEM_DEL(item_tmp);

                free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ2_0046);
                free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ2_0047);

                EBGN_DEC_LEN(ebgn_tmp);

                item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        }

        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ2_0048);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ2_0049);

        EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);
        ebgn_z2_move(ebgnz2_md_id, ebgn_tmp, ebgn_c);
    }
    else
    {
        ebgn_z2_clone(ebgnz2_md_id, ebgn_a, ebgn_c);
    }

    for( ; 0 < bgn_offset; bgn_offset -- )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0050);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0051);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_HEAD(ebgn_c, item_tmp);
        EBGN_INC_LEN(ebgn_c);
    }

    ebgn_z2_free_ebgn(ebgnz2_md_id, ebgn_tmp);

    return ( 0 );
}


/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a + b over Z_2[x]
*     = a ^ b over Z
*
**/
UINT32 ebgn_z2_add(const UINT32 ebgnz2_md_id,const EBGN * ebgn_a,const EBGN * ebgn_b,EBGN * ebgn_c)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_add: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz_md_id = ebgnz2_md->bgnz_md_id;

    ebgn_z2_alloc_ebgn(ebgnz2_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0052);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0053);

        bgn_z_bit_xor(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0054);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0055);

        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }
    while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ2_0056);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ2_0057);

        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
        EBGN_ITEM_DEL(item_tmp);

        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ2_0058);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ2_0059);

        EBGN_DEC_LEN(ebgn_tmp);

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }

    EBGN_SGN(ebgn_tmp) = EC_TRUE;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
    }

    ebgn_z2_move(ebgnz2_md_id, ebgn_tmp, ebgn_c);
    ebgn_z2_free_ebgn(ebgnz2_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*   deg (a) < n
*   deg (b) < n
*
*   c = a - b over Z_2[x]
*     = a ^ b over Z
*
**/
UINT32 ebgn_z2_sub(const UINT32 ebgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * c)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_sub: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_sub: b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_sub: c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_sub: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    return ebgn_z2_add(ebgnz2_md_id, a, b, c);
}

/**
*       return c = a + b * x^( offset ) over Z_2[x]
*                = a ^ ( b << ofset) over Z
*       where
*           offset > 0
*       and
*           deg(a) < EBGNZSIZE
*       and
*           deg(b) + offset < EBGNZSIZE
*
**/
UINT32 ebgn_z2_add_offset(const UINT32 ebgnz2_md_id,const EBGN *a,const EBGN *b,const UINT32 offset,EBGN *c)
{
    EBGN *tmp;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add_offset: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add_offset: b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_add_offset: c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_add_offset: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgn_z2_alloc_ebgn(ebgnz2_md_id, &tmp);

    ebgn_z2_shl(ebgnz2_md_id, b, offset, tmp);
    ebgn_z2_add(ebgnz2_md_id,  a, tmp, c);

    ebgn_z2_destroy(ebgnz2_md_id, tmp);

    return ( 0 );
}

/**
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
**/
UINT32 ebgn_z2_squ(const UINT32 ebgnz2_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ2_MD *ebgnz2_md;
    UINT32 bgnz2_md_id;

    EBGN ebgn_tmp;
    EBGN_ITEM *item_c0;
    EBGN_ITEM *item_c1;
    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_c;


#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_squ: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_squ: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_squ: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz2_md = &(g_ebgnz2_md[ ebgnz2_md_id ]);
    bgnz2_md_id = ebgnz2_md->bgnz2_md_id;

    EBGN_INIT(&ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_c0, LOC_EBGNZ2_0060);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, &item_c1, LOC_EBGNZ2_0061);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ2_0062);
        alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ2_0063);

        bgn_z2_squ(bgnz2_md_id, EBGN_ITEM_BGN(item_a), bgn_c0, bgn_c1);

        EBGN_ITEM_BGN(item_c0) = bgn_c0;
        EBGN_ADD_ITEM_TAIL(&ebgn_tmp, item_c0);
        EBGN_INC_LEN(&ebgn_tmp);

        EBGN_ITEM_BGN(item_c1) = bgn_c1;
        EBGN_ADD_ITEM_TAIL(&ebgn_tmp, item_c1);
        EBGN_INC_LEN(&ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    item_c = EBGN_LAST_ITEM(&ebgn_tmp);
    if( item_c != EBGN_NULL_ITEM(&ebgn_tmp) && EC_TRUE == bgn_z2_is_zero(bgnz2_md_id, EBGN_ITEM_BGN(item_c)) )
    {
        EBGN_ITEM_DEL(item_c);

        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_c), LOC_EBGNZ2_0064);
        free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN_ITEM, item_c, LOC_EBGNZ2_0065);

        EBGN_DEC_LEN(&ebgn_tmp);
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z2_clean(ebgnz2_md_id, ebgn_c);
    }

    ebgn_z2_move(ebgnz2_md_id, &ebgn_tmp, ebgn_c);

    return ( 0 );
}

#if 0
/**
*   precompute a table for ebgn_z2_mul
*
*   deg (a) < n and a is nonzero over Z_2[x]
*
*   let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_EBGNZ2_PREMUL }
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
*   Output: table ebgn_z2_premul[]
*   1. set table[ 0 ] = 0
*   2. set table[ 1 ] = a
*   3. for b from 2 to MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 do
*   3.1     let b = 2 * b1 + b0, where b0 belong to {0,1}
*   3.2     table[ b ] = ( table[ b1 ] << 1 ) + table[ b0 ]  over Z_2[x]
*   4.  end for
*   5.return ebgn_z2_premul
*
**/
void ebgn_z2_premul(const UINT32 ebgnz2_md_id,const EBGN * a)
{
    UINT32 b;
    UINT32 b0;
    UINT32 b1;
    EBGN *t_b[ 2 ];
    EBGN *t_b0[ 2 ];
    EBGN *t_b1[ 2 ];

    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_premul: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_premul: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/


    /* set ebgn_z2_premul[ 0 ] = 0 */
    b = 0;
    t_b[ 0 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b ][ 0 ]);
    t_b[ 1 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b ][ 1 ]);
    ebgn_z2_set_zero( ebgnz2_md_id, t_b[ 0 ] );
    ebgn_z2_set_zero( ebgnz2_md_id, t_b[ 1 ] );

    /* set ebgn_z2_premul[ 1 ] = a */
    b = 1;
    t_b[ 0 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b ][ 0 ]);
    t_b[ 1 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b ][ 1 ]);
    ebgn_z2_clone( ebgnz2_md_id, a, t_b[ 0 ] );
    ebgn_z2_set_zero( ebgnz2_md_id, t_b[ 1 ] );

    for ( index = 2; index < MAX_SIZE_OF_EBGNZ2_PREMUL_TABLE; index ++ )
    {
        b = index;
        b0 = ( b & 1 );
        b1 = ( b >> 1 );

        t_b[ 0 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b  ][ 0 ]);
        t_b[ 1 ]  = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b  ][ 1 ]);

        t_b0[ 0 ] = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b0 ][ 0 ]);
        t_b0[ 1 ] = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b0 ][ 1 ]);

        t_b1[ 0 ] = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b1 ][ 0 ]);
        t_b1[ 1 ] = (g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ b1 ][ 1 ]);

        /* compute t_b = (t_b1 << 1) + t_b0 */
        ebgn_z2_dshl_lesswordsize(ebgnz2_md_id, t_b1[ 0 ],t_b1[ 1 ], 1, t_b[ 0 ],t_b[ 1 ]);

        if ( 0 < b0 )
        {
            ebgn_z2_add( ebgnz2_md_id, t_b[ 0 ], t_b0[ 0 ], t_b[ 0 ]);
            ebgn_z2_add( ebgnz2_md_id, t_b[ 1 ], t_b0[ 1 ], t_b[ 1 ]);
        }
    }

    return ;
}

/**
*   deg (a) < EBGNZSIZE
*   deg (b) < EBGNZSIZE
*
*   let c = (c1,c0) = c1 * x ^ {EBGNZSIZE} + c0
*
*   return c = a * b over Z_2[x]
*
*   Note:
*   1. let a = SUM(a_i * x ^{ WINWIDTH_OF_EBGNZ2_PREMUL * i}, i = 0..m, where deg(a_i) < WINWIDTH_OF_EBGNZ2_PREMUL )
*      then
*          a * b = SUM(a_i * b * x ^{ WINWIDTH_OF_EBGNZ2_PREMUL * i}, i = 0..m )
*   2. let a = a1 * x^{WINWIDTH_OF_EBGNZ2_PREMUL} + a0
*      then
*          a * b = (a1 * b) * x^{WINWIDTH_OF_EBGNZ2_PREMUL} + (a0 *b)
*
*   Algorithm:
*   Input: a,b belong to Z_2[x], and a is nonzero, b is nonzero
*   Output: c = a * b Z_2[x]
*   0. let B = { b over Z_2[x] : deg(b) < WINWIDTH_OF_EBGNZ2_PREMUL }
*   1. precompute table :
*           { table[i]: i = 0..MAX_SIZE_OF_BGNZ_2N_PREMUL_TABLE - 1 } =
*           { r * b over Z_2[x], where r run through the set B }
*   2. set c = 0
*   3. for i from m downto 0 do
*   3.1        c = c + table[ a_i ] over Z_2[x]
*   3.2        if i > 0 then do
*                 c = c * x^{WINWIDTH_OF_EBGNZ2_PREMUL} over Z_2[x]
*   3.3        end if
*   3.4 end for
*   return c
**/
UINT32 ebgn_z2_mul(const UINT32 ebgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * c)
{
    UINT32 a_deg;
    UINT32 win_width;
    UINT32 win_num;
    UINT32 word_offset;
    UINT32 bit_offset;
    UINT32 window;
    UINT32 e;

    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_mul: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_mul: b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == c )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_mul: c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_mul: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* set c = 0 */
    ebgn_z2_set_zero(ebgnz2_md_id,  c);

    if ( EC_TRUE == ebgn_z2_is_zero(ebgnz2_md_id,  a ) )
    {
        return ;
    }

    if ( EC_TRUE == ebgn_z2_is_zero(ebgnz2_md_id,  b ) )
    {
        return ;
    }

    /* precompute table { r * b over Z_2, where deg(r) < WINWIDTH_OF_EBGNZ2_PREMUL }*/
    ebgn_z2_premul(ebgnz2_md_id,  b );

    a_deg     = ebgn_z2_deg(ebgnz2_md_id,  a );
    win_width   = WINWIDTH_OF_EBGNZ2_PREMUL; /* must less than WORDSIZE */
    win_num     = ( a_deg + win_width ) / win_width;

    /* e = 0x00000000F when WINWIDTH_OF_EBGNZ2_PREMUL = 4 */
    e = ~((~((UINT32)0)) << win_width );
    for ( index = win_num; index > 0; )
    {
        index --;
        word_offset = ( win_width * index ) / WORDSIZE;
        bit_offset  = ( win_width * index ) % WORDSIZE;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
        if ( INTMAX <= word_offset )
        {
            sys_log(LOGSTDOUT,"error:ebgn_z2_mul: word_offset = %ld overflow\n",word_offset);
            dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
        }
#endif/*EBGN_DEBUG_SWITCH*/

        /* a_i = window */
        window = ( ( a->data[ word_offset ] >> ( bit_offset ) ) & e );

        /* c = c + table[ a_i ] */
        ebgn_z2_add(ebgnz2_md_id, c0,(g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ window ][ 0 ]),c0);
        ebgn_z2_add(ebgnz2_md_id, c1,(g_ebgn_z2_premul_tbl[ ebgnz2_md_id ][ window ][ 1 ]),c1);

        if ( index > 0 )
        {
            /* c = c * x^{WINWIDTH_OF_EBGNZ2_PREMUL} */
            ebgn_z2_dshl_lesswordsize(ebgnz2_md_id, c0, c1, win_width, c0, c1);
        }
    }

    return ( 0 );
}

/**
*   deg (a) < EBGNZSIZE and a is nonzero
*   deg (b) < EBGNZSIZE and b is nonzero
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
UINT32 ebgn_z2_div(const UINT32 ebgnz2_md_id,const EBGN * a,const EBGN * b,EBGN * q,EBGN *r)
{
    EBGN *ta;
    EBGN *tmp;
    EBGN *e;
    UINT32 a_deg;
    UINT32 b_deg;
    UINT32 ta_deg;
    UINT32 diff_deg;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == a )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == b )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == q )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: q is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( NULL_PTR == r )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: r is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/


#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( a == q )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: address of a and q conflict.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( a == r )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: address of a and r conflict.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( b == q )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: address of b and q conflict.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( b == r )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: address of b and r conflict.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
    if ( q == r )
    {
        sys_log(LOGSTDOUT,"error:ebgn_z2_div: address of q and r conflict.\n");
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ2_MD <= ebgnz2_md_id || 0 == g_ebgnz2_md[ ebgnz2_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z2_div: ebgnz2 module #0x%lx not started.\n",
                ebgnz2_md_id);
        dbg_exit(MD_EBGNZ2, ebgnz2_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* 0 = b * 0 + 0 */
    if ( EC_TRUE == ebgn_z2_is_zero( ebgnz2_md_id, a ) )
    {
        ebgn_z2_set_zero( ebgnz2_md_id, q );
        ebgn_z2_set_zero( ebgnz2_md_id, r );

        return ;
    }
    /* a = 0 * 0 + a */
    if ( EC_TRUE == ebgn_z2_is_zero( ebgnz2_md_id, b ) )
    {
        ebgn_z2_set_zero( ebgnz2_md_id, q );
        ebgn_z2_clone(ebgnz2_md_id, a, r );

        return ;
    }

    /* a = 1 * a + 0 */
    if ( EC_TRUE == ebgn_z2_is_one( ebgnz2_md_id, b ) )
    {
        ebgn_z2_clone(ebgnz2_md_id, a, q );
        ebgn_z2_set_zero( ebgnz2_md_id, r );

        return ;
    }

    a_deg = ebgn_z2_deg( ebgnz2_md_id, a );
    b_deg = ebgn_z2_deg( ebgnz2_md_id, b );

    /* if deg(a) < deg(b) ,then a = b * 0 + a */
    if ( a_deg < b_deg )
    {
        ebgn_z2_set_zero( ebgnz2_md_id, q );
        ebgn_z2_clone(ebgnz2_md_id, a, r );

        return ;
    }
    /* if deg(a) = deg(b) ,then a = b * 1 + ( a - b) mod 2 */
    if ( a_deg == b_deg )
    {
        ebgn_z2_set_one( ebgnz2_md_id, q );
        ebgn_z2_sub(ebgnz2_md_id, a, b, r);

        return ;
    }

    /* here deg(a) > deg(b) > 0 */
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,&ta, LOC_EBGNZ2_0066);
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,&tmp, LOC_EBGNZ2_0067);
    alloc_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,&e, LOC_EBGNZ2_0068);

    ebgn_z2_clone(ebgnz2_md_id, a, ta);

    ebgn_z2_set_zero( ebgnz2_md_id, q );
    ebgn_z2_set_zero( ebgnz2_md_id, r );

    /* while deg(ta) >= deg(b) > 0 do */
    ta_deg = a_deg;
    while ( ta_deg >= b_deg )
    {
        diff_deg = ta_deg - b_deg;

        ebgn_z2_shl(ebgnz2_md_id, b, diff_deg, tmp);

        /* t_a <-- t_a - tmp */
        ebgn_z2_sub(ebgnz2_md_id, ta, tmp, ta);

        ebgn_z2_set_e(ebgnz2_md_id, e, diff_deg);
        ebgn_z2_add(ebgnz2_md_id, q, e, q);

        ta_deg = ebgn_z2_deg( ebgnz2_md_id, ta);
    }

    ebgn_z2_clone(ebgnz2_md_id, ta, r);

    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,ta, LOC_EBGNZ2_0069);
    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,tmp, LOC_EBGNZ2_0070);
    free_static_mem(MD_EBGNZ2, ebgnz2_md_id, MM_EBGN,e, LOC_EBGNZ2_0071);

    return ( 0 );
}

#endif

#ifdef __cplusplus
}
#endif/*__cplusplus*/
