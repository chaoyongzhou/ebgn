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
#include "ebgnz.h"

#include "debug.h"

#include "print.h"

static EBGNZ_MD g_ebgnz_md[ MAX_NUM_OF_EBGNZ_MD ];
static EC_BOOL  g_ebgnz_md_init_flag = EC_FALSE;

static UINT32 ebgn_z_item_destory(const UINT32  ebgnz_md_id, EBGN_ITEM *item);
static UINT32 ebgn_z_item_clone(const UINT32  ebgnz_md_id, const EBGN_ITEM *item_a, EBGN_ITEM *item_c);
static UINT32 ebgn_z_neg_self(const UINT32  ebgnz_md_id, EBGN *ebgn_a);
static UINT32 ebgn_z_abs_adc(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_sadd(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_add(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_add_offset(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, const UINT32 offset, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_sbb_a_c(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_sbb_c_a(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_sub(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_mul(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c);
static UINT32 ebgn_z_abs_div(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q, EBGN *ebgn_r);

/**
*   for test only
*
*   to query the status of EBGNZ Module
*
**/
void ebgn_z_print_module_status(const UINT32  ebgnz_md_id, LOG *log)
{
    EBGNZ_MD *ebgnz_md;
    UINT32 index;

    if ( EC_FALSE == g_ebgnz_md_init_flag )
    {
        sys_log(log,"no EBGNZ Module started.\n");
        return ;
    }

    for ( index = 0; index < MAX_NUM_OF_EBGNZ_MD; index ++ )
    {
        ebgnz_md = &(g_ebgnz_md[ index ]);

        if ( 0 < ebgnz_md->usedcounter )
        {
            sys_log(log,"EBGNZ Module # %ld : %ld refered, refer BGNZ Module : %ld\n",
                    index,
                    ebgnz_md->usedcounter,
                    ebgnz_md->bgnz_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed EBGNZ module
*
*
**/
UINT32 ebgn_z_free_module_static_mem(const UINT32  ebgnz_md_id)
{
    EBGNZ_MD  *ebgnz_md;
    UINT32   bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_free_module_static_mem: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    free_module_static_mem(MD_EBGNZ, ebgnz_md_id);

    bgn_z_free_module_static_mem(bgnz_md_id);

    return 0;
}
/**
*
* start EBGNZ module
*
**/
UINT32  ebgn_z_start( )
{
    EBGNZ_MD *ebgnz_md;
    UINT32  ebgnz_md_id;
    UINT32  bgnz_md_id;

    UINT32 index;

    /* if this is the 1st time to start EBGNZ module, then */
    /* initialize g_ebgnz_md */
    if ( EC_FALSE ==  g_ebgnz_md_init_flag )
    {
        for ( index = 0; index < MAX_NUM_OF_EBGNZ_MD; index ++ )
        {
            ebgnz_md = &(g_ebgnz_md[ index ]);

            ebgnz_md->usedcounter   = 0;
            ebgnz_md->bgnz_md_id = ERR_MODULE_ID;
        }

        /*register all functions of EBGNZ module to DBG module*/
        //dbg_register_func_addr_list(g_ebgnz_func_addr_list, g_ebgnz_func_addr_list_len);

        g_ebgnz_md_init_flag = EC_TRUE;
    }

    /* search for a free module node */
    for ( index = 0; index < MAX_NUM_OF_EBGNZ_MD; index ++ )
    {
        ebgnz_md = &(g_ebgnz_md[ index ]);

        if ( 0 == ebgnz_md->usedcounter )
        {
            break;
        }
    }
    /* if no free module node is available, then return error*/
    if ( index >= MAX_NUM_OF_EBGNZ_MD )
    {
        return ( ERR_MODULE_ID );
    }

    /* create a new module node */
    bgnz_md_id = ERR_MODULE_ID;

    /* initilize new one EBGNZ module */
    ebgnz_md_id = index;
    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);

    init_static_mem();

    bgnz_md_id = bgn_z_start();

    ebgnz_md->bgnz_md_id = bgnz_md_id;
    ebgnz_md->usedcounter = 1;

    return ( ebgnz_md_id );
}

/**
*
* end EBGNZ module
*
**/
void ebgn_z_end(const UINT32  ebgnz_md_id)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    if ( MAX_NUM_OF_EBGNZ_MD < ebgnz_md_id )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_end: ebgnz_md_id = %ld is overflow.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ebgnz_md->usedcounter )
    {
        ebgnz_md->usedcounter --;
        return ;
    }

    if ( 0 == ebgnz_md->usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_end: ebgnz_md_id = %ld is not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }

    bgnz_md_id = ebgnz_md->bgnz_md_id;

    bgn_z_end( bgnz_md_id);

    /* free module : */
    ebgnz_md->bgnz_md_id = ERR_MODULE_ID;
    ebgnz_md->usedcounter = 0;

    breathing_static_mem();

    return ;
}

/**
*
*   alloc a BIGINT type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_bgn(const UINT32  ebgnz_md_id, BIGINT **ppbgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ppbgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_alloc_bgn: ppbgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_alloc_bgn: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, ppbgn, LOC_EBGNZ_0001);
    return ( 0 );
}

/**
*
*   alloc a EBGN_ITEM type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_item(const UINT32  ebgnz_md_id,EBGN_ITEM **ppitem)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ppitem )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_alloc_item: ppitem is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_alloc_item: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, ppitem, LOC_EBGNZ_0002);
    EBGN_ITEM_INIT(*ppitem);
    return ( 0 );
}
/**
*
*   alloc a EBGN type node from EBGNZ space
*
**/
UINT32 ebgn_z_alloc_ebgn(const UINT32  ebgnz_md_id,EBGN **ppebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ppebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_alloc_ebgn: ppebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_alloc_ebgn: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN, ppebgn, LOC_EBGNZ_0003);
    EBGN_INIT(*ppebgn);

    return ( 0 );
}

/**
*
*   free the BIGINT type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_bgn(const UINT32  ebgnz_md_id,BIGINT *bgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == bgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_free_bgn: bgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_free_bgn: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn, LOC_EBGNZ_0004);

    return ( 0 );
}

/**
*
*   free the EBGN_ITEM type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_item(const UINT32  ebgnz_md_id,EBGN_ITEM *item)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_free_item: item is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_free_item: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item, LOC_EBGNZ_0005);

    return ( 0 );
}

/**
*
*   free the EBGN type node from EBGNZ space
*
**/
UINT32 ebgn_z_free_ebgn(const UINT32  ebgnz_md_id, EBGN *ebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_free_ebgn: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_free_ebgn: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN, ebgn, LOC_EBGNZ_0006);

    return ( 0 );
}

/**
*
*   destory the whole item,including its content bgn and itself.
*   so, when return from this function, item cannot be refered any more
*
**/
static UINT32 ebgn_z_item_destory(const UINT32  ebgnz_md_id, EBGN_ITEM *item)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_item_destory: item is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_item_destory: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item), LOC_EBGNZ_0007);
    EBGN_ITEM_BGN(item) = NULL_PTR;

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item, LOC_EBGNZ_0008);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,i.e., all its items but not the ebgn itself.
*   so, when return from this function, ebgn can be refered again without any item.
*
**/
UINT32 ebgn_z_clean(const UINT32  ebgnz_md_id, EBGN *ebgn)
{
    EBGN_ITEM *item;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_clean: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_clean: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    item = EBGN_FIRST_ITEM(ebgn);

    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        EBGN_ITEM_DEL(item);
        ebgn_z_item_destory(ebgnz_md_id, item);

        item = EBGN_FIRST_ITEM(ebgn);
    }

    EBGN_INIT(ebgn);

    return ( 0 );
}

/**
*
*   destory the whole ebgn,including its all items and itself.
*   so, when return from this function, ebgn cannot be refered any more
*
**/
UINT32 ebgn_z_destroy(const UINT32  ebgnz_md_id, EBGN *ebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_destroy: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_destroy: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgn_z_clean(ebgnz_md_id,ebgn);

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN, ebgn, LOC_EBGNZ_0009);

    return ( 0 );
}

static UINT32 ebgn_z_item_clone(const UINT32  ebgnz_md_id, const EBGN_ITEM *item_a, EBGN_ITEM *item_c)
{
    BIGINT  *bgn;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_item_clone: item_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == item_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_item_clone: item_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_item_clone: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( item_a == item_c )
    {
    return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    EBGN_ITEM_INIT(item_c);

    /*clone bgn*/
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0010);
    bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn);

    EBGN_ITEM_BGN(item_c) = bgn;

    return ( 0 );
}

/**
*
*   ebgn_c = ebgn_a
*
**/
UINT32 ebgn_z_clone(const UINT32  ebgnz_md_id,const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGN_ITEM *item_a;
    EBGN_ITEM *item_c;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_clone: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_clone: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_clone: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_c )
    {
    return ( 0 );
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }
    //EBGN_INIT(ebgn_c);

    item_a = EBGN_FIRST_ITEM(ebgn_a);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0011);

        ebgn_z_item_clone(ebgnz_md_id, item_a, item_c);

        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    EBGN_LEN(ebgn_c) = EBGN_LEN(ebgn_a);
    EBGN_SGN(ebgn_c) = EBGN_SGN(ebgn_a);

    return ( 0 );
}

/**
*
*   if ebgn = 0, return EC_TRUE
*   if ebgn != 0, return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_zero(const UINT32  ebgnz_md_id, const EBGN *ebgn)
{
    EBGN_ITEM *item;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_is_zero: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_is_zero: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == EBGN_IS_EMPTY(ebgn) )
    {
        return ( EC_TRUE );
    }

    if( 0 == EBGN_LEN(ebgn) )
    {
    return ( EC_TRUE );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    item = EBGN_FIRST_ITEM(ebgn);

    if( 1 == EBGN_LEN(ebgn) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item)) )
    {
    return ( EC_TRUE );
    }

    return ( EC_FALSE );
}

/**
*
*   if ebgn = 1, return EC_TRUE
*   if ebgn != 1, return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_one(const UINT32  ebgnz_md_id, const EBGN *ebgn)
{
    EBGN_ITEM *item;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_is_one: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_is_one: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    item = EBGN_FIRST_ITEM(ebgn);

    if( 1 == EBGN_LEN(ebgn) && EC_TRUE == bgn_z_is_one(bgnz_md_id, EBGN_ITEM_BGN(item)) )
    {
    return ( EC_TRUE );
    }

    return ( EC_FALSE );
}

/**
*
*   if ebgn is odd, then return EC_TRUE
*   if ebgn is even, then return EC_FALSE
*
**/
EC_BOOL ebgn_z_is_odd(const UINT32  ebgnz_md_id, const EBGN *ebgn)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_is_odd: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_is_odd: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn) )
    {
    return ( EC_FALSE );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    /* fetch the lowest bit segment */
    item = EBGN_FIRST_ITEM(ebgn);

    return bgn_z_is_odd(bgnz_md_id, EBGN_ITEM_BGN(item));
}

/**
*
*   set ebgn = 0
*
**/
UINT32 ebgn_z_set_zero(const UINT32  ebgnz_md_id,EBGN *ebgn)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_zero: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_zero: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == EBGN_IS_EMPTY(ebgn) )
    {
        return ( 0 );
    }

    return ebgn_z_clean(ebgnz_md_id, ebgn);
}

/**
*
*   set ebgn = 1
*
**/
UINT32 ebgn_z_set_one(const UINT32  ebgnz_md_id,EBGN *ebgn)
{
    EBGN_ITEM *item;
    BIGINT *bgn;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_one: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_one: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn);
    }

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0012);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0013);

    bgn_z_set_one(bgnz_md_id, bgn);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_HEAD(ebgn, item);
    EBGN_SET_LEN(ebgn, 1);

    return ( 0 );
}

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_z_set_word(const UINT32  ebgnz_md_id,const UINT32 n, EBGN *ebgn)
{
    EBGN_ITEM *item;
    BIGINT *bgn;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_word: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_word: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn);
    }

    if ( 0 == n )
    {
        return ( 0 );
    }

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0014);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0015);

    bgn_z_set_word(bgnz_md_id, bgn, n);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_HEAD(ebgn, item);
    EBGN_SET_LEN(ebgn, 1);

    return ( 0 );
}

/**
*
*   set ebgn = n
*
**/
UINT32 ebgn_z_set_n(const UINT32  ebgnz_md_id,const BIGINT *n, EBGN *ebgn)
{
    EBGN_ITEM *item;
    BIGINT *bgn;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == n )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_n: n is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_n: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_n: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn);
    }

    if( EC_TRUE == bgn_z_is_zero(bgnz_md_id, n) )
    {
    return ( 0 );
    }

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0016);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0017);

    bgn_z_clone(bgnz_md_id, n, bgn);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_HEAD(ebgn, item);
    EBGN_SET_LEN(ebgn, 1);

    return ( 0 );
}
/**
*
*   set ebgn = 2 ^ e
*
**/
UINT32 ebgn_z_set_e(const UINT32  ebgnz_md_id,const UINT32 e, EBGN *ebgn)
{
    EBGN_ITEM *item;
    BIGINT *bgn;

    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    UINT32 len;
    UINT32 offset;
    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_e: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_e: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn);
    }

    len = ( e  )/ BIGINTSIZE;
    offset = e % BIGINTSIZE;

    /* only loop len - 1 times and hope to hanlde len = 0 case */
    for( index = 0; index < len; index ++ )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0018);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0019);

        bgn_z_set_zero(bgnz_md_id, bgn);

        EBGN_ITEM_BGN(item) = bgn;
        EBGN_ADD_ITEM_TAIL(ebgn, item);
    }

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0020);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0021);

    bgn_z_set_e(bgnz_md_id, bgn, offset);

    EBGN_ITEM_BGN(item) = bgn;
    EBGN_ADD_ITEM_TAIL(ebgn, item);
    EBGN_SET_LEN(ebgn, len + 1);

    return ( 0 );
}

int ebgn_z_item_cmp(const UINT32  ebgnz_md_id,const EBGN_ITEM *item_a, const EBGN_ITEM *item_b)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == item_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_item_cmp: item_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == item_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_item_cmp: item_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_item_cmp: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    return bgn_z_cmp(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b));
}

/**
*
*   if ebgn_a = ebgn_b then return 0
*   if ebgn_a > ebgn_b then return 1
*   if ebgn_a < ebgn_b then return -1
*
**/
int ebgn_z_cmp(const UINT32  ebgnz_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b)
{
    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

    int ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_cmp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_cmp: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_cmp: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EBGN_IS_POS(ebgn_a) )
    {
    /* a > 0 and b < 0 */
    if( EBGN_IS_NEG(ebgn_b) )
    {
        return ( 1 );
    }

    /* a > b >= 0 */
    if( EBGN_LEN(ebgn_a) > EBGN_LEN(ebgn_b) )
    {
        return ( 1 );
    }

    /* b > a >= 0 */
    if( EBGN_LEN(ebgn_a) < EBGN_LEN(ebgn_b) )
    {
        return ( -1 );
    }
    }
    else
    {
    /* a <= 0 < b */
    if( EBGN_IS_POS(ebgn_b) )
    {
        return ( -1 );
    }

    /* a < b <= 0 */
    if( EBGN_LEN(ebgn_a) > EBGN_LEN(ebgn_b) )
    {
        return ( -1 );
    }

    /* b < a <= 0 */
    if( EBGN_LEN(ebgn_a) < EBGN_LEN(ebgn_b) )
    {
        return ( 1 );
    }
    }

    /* now a >= 0, b >= 0 and len(a) = len(b)*/
    /* or  a <= 0, b <= 0 and len(a) = len(b)*/

    item_a = EBGN_LAST_ITEM(ebgn_a);
    item_b = EBGN_LAST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        ret = ebgn_z_item_cmp(ebgnz_md_id, item_a, item_b);
        if( 0 != ret )
        {
            if( EC_TRUE == EBGN_SGN(ebgn_a) )
        {
                return ( ret );
            }
        else
        {
                return ( -ret );
        }
        }

        item_a = EBGN_ITEM_PREV(item_a);
        item_b = EBGN_ITEM_PREV(item_b);
    }

    return ( 0 );
}

/*
*
*   if abs(ebgn_a) = abs(ebgn_b) then return 0
*   if abs(ebgn_a) > abs(ebgn_b) then return 1
*   if abs(ebgn_a) < abs(ebgn_b) then return -1
*
**/
int ebgn_z_abs_cmp(const UINT32  ebgnz_md_id,const EBGN *ebgn_a, const EBGN *ebgn_b)
{
    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

    int ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_cmp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_cmp: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_cmp: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EBGN_LEN(ebgn_a) > EBGN_LEN(ebgn_b) )
    {
    return ( 1 );
    }

    if( EBGN_LEN(ebgn_a) < EBGN_LEN(ebgn_b) )
    {
    return ( -1 );
    }

    /* len(a) = len(b)*/

    item_a = EBGN_LAST_ITEM(ebgn_a);
    item_b = EBGN_LAST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        ret = ebgn_z_item_cmp(ebgnz_md_id, item_a, item_b);
    if( 0 != ret )
        {
            return ( ret );
        }

        item_a = EBGN_ITEM_PREV(item_a);
        item_b = EBGN_ITEM_PREV(item_b);
    }

    return ( 0 );
}

/**
*
*    move ebgn_a to ebgn_c, ebgn_c = ebgn_a, and ebgn_a return null
*
**/
UINT32 ebgn_z_move(const UINT32  ebgnz_md_id, EBGN *ebgn_a, EBGN *ebgn_c)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_move: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_move: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_move: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( ebgn_a != ebgn_c )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
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
*   if a = 0, then nbits = 0
*   else
*       let a = [+/-]SUM( a_i * 2 ^i, where i = 0..m where a_m > 0,i.e, a_m = 1 )
*   then    nbits = m + 1
*
**/
UINT32 ebgn_z_get_nbits(const UINT32  ebgnz_md_id,const EBGN *ebgn_a, UINT32 *nbits)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    UINT32 nbits_of_item_a;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_get_nbits: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == nbits )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_get_nbits: nbits is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_get_nbits: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
    *nbits = 0;
    return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    item_a = EBGN_LAST_ITEM(ebgn_a);
    nbits_of_item_a = bgn_z_get_nbits(bgnz_md_id, EBGN_ITEM_BGN(item_a));

    *nbits = nbits_of_item_a + (EBGN_LEN(ebgn_a) - 1 ) * BIGINTSIZE;

    return ( 0 );
}

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) & abs(ebgn_b)
*
*  note,
*    1. the sgn of ebgn_a or ebgn_b will be ignored
*    2. the sgn of ebgn_c will be TRUE
*    3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_and(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_and: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_and: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_and: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_bit_and: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0022);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0023);

    bgn_z_bit_and(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
        EBGN_ITEM_DEL(item_tmp);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0024);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0025);

    EBGN_DEC_LEN(ebgn_tmp);

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }

    EBGN_SGN(ebgn_tmp) = EC_TRUE;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) | abs(ebgn_b)
*
*  note,
*    1. the sgn of ebgn_a or ebgn_b will be ignored
*    2. the sgn of ebgn_c will be TRUE
*    3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_or(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_or: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_or: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_or: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_bit_or: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0026);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0027);

    bgn_z_bit_or(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0028);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0029);

    bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0030);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0031);

    bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    EBGN_SGN(ebgn_tmp) = EC_TRUE;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*  binary operation:
*
*  ebgn_c = abs(ebgn_a) ^ abs(ebgn_b)
*
*  note,
*    1. the sgn of ebgn_a or ebgn_b will be ignored
*    2. the sgn of ebgn_c will be TRUE
*    3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_xor(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_xor: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_xor: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_xor: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_bit_xor: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0032);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0033);

    bgn_z_bit_xor(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0034);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0035);

    bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0036);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0037);

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

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0038);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0039);

    EBGN_DEC_LEN(ebgn_tmp);

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }

    EBGN_SGN(ebgn_tmp) = EC_TRUE;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*  binary operation:
*
*  ebgn_c = NOT abs(ebgn_a) ( i.e, ebgn_c = ~ abs(ebgn_a) )
*
*  note,
*    1. the sgn of ebgn_a will be ignored
*    2. the sgn of ebgn_c will be TRUE
*    3. no shift out will be return or kept
*
**/
UINT32 ebgn_z_bit_not(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;

    UINT32 nbits_of_ebgn_a;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_not: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_bit_not: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_bit_not: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
            ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }
    ebgn_z_set_one(ebgnz_md_id, ebgn_c);
    return ( 0 );
    }

    /* now ebgn_a > 0 */
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0040);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0041);

    bgn_z_bit_not(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    ebgn_z_get_nbits(ebgnz_md_id, ebgn_a, &nbits_of_ebgn_a);
    bit_offset = nbits_of_ebgn_a % BIGINTSIZE;

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    if ( 0 < bit_offset && item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_FALSE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
    bgn_z_bit_cut(bgnz_md_id, EBGN_ITEM_BGN(item_tmp), bit_offset - 1, EBGN_ITEM_BGN(item_tmp));
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
        EBGN_ITEM_DEL(item_tmp);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0042);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0043);

    EBGN_DEC_LEN(ebgn_tmp);

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }

    EBGN_SGN(ebgn_tmp) = EC_TRUE;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*   let ebgn_a = [+/-]SUM(a_i * 2^i, where i = 0..N, N is enough large, a_i = 0 or 1)
*   return a_{nthbit}
*
**/
UINT32 ebgn_z_get_bit(const UINT32  ebgnz_md_id,const EBGN *ebgn_a, const UINT32 nthbit)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_get_bit: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_get_bit: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    bgn_offset = nthbit / BIGINTSIZE;
    bit_offset = nthbit % BIGINTSIZE;

    if( bgn_offset >= EBGN_LEN(ebgn_a) )
    {
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bgn_offset )
    {
        bgn_offset --;
        item_a = EBGN_ITEM_NEXT(item_a);
    }

    return bgn_z_get_bit(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset);
}

/**
*   let a = [+/-]SUM(a_i * 2^i, where a_i = 0 or 1)
*   then set a_{nthbit} = 1
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z_set_bit(const UINT32  ebgnz_md_id, EBGN *ebgn_a, const UINT32 nthbit)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    BIGINT *bgn_of_item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_set_bit: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_set_bit: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

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

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_a, LOC_EBGNZ_0044);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_a, LOC_EBGNZ_0045);

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
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_a, LOC_EBGNZ_0046);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_a, LOC_EBGNZ_0047);

    bgn_z_set_zero(bgnz_md_id, bgn_of_item_a);
    bgn_z_set_bit(bgnz_md_id, bgn_of_item_a, bit_offset);

    EBGN_ITEM_BGN(item_a) = bgn_of_item_a;
    EBGN_ADD_ITEM_TAIL(ebgn_a, item_a);
    EBGN_INC_LEN(ebgn_a);
    }

    return ( 0 );
}

/**
*   let a = [+/-]SUM(a_i * 2^i, where i = 0..N, N is enough large,a_i = 0 or 1)
*   then set a_{nthbit} = 0
*   note:
*       this function is able to deal with a = 0.
**/
UINT32 ebgn_z_clear_bit(const UINT32  ebgnz_md_id, EBGN *ebgn_a, const UINT32 nthbit)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_clear_bit: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_clear_bit: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    bgn_offset = nthbit / BIGINTSIZE;
    bit_offset = nthbit % BIGINTSIZE;

    if( bgn_offset >= EBGN_LEN(ebgn_a) )
    {
    return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

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

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_a), LOC_EBGNZ_0048);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_a, LOC_EBGNZ_0049);

    EBGN_DEC_LEN(ebgn_a);

        item_a = EBGN_LAST_ITEM(ebgn_a);
    }

    return ( 0 );
}

/**
*
*
*  ebgn_c = (ebgn_a >> nbits)
*
*  note,
*    1. the sgn will not be changed
*    2. no shift out will be return or kept
*
**/
UINT32 ebgn_z_shr(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, UINT32 nbits, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *item_a;

    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

    UINT32 nbits_of_ebgn_a;
    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_shr: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_shr: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_shr: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }
    return ( 0 );
    }

    if( 0 == nbits )
    {
    if( ebgn_a == ebgn_c )
    {
        return ( 0 );
    }
    return ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    ebgn_z_get_nbits(ebgnz_md_id, ebgn_a, &nbits_of_ebgn_a);
    if( nbits_of_ebgn_a <= nbits )
    {
        return ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    /* now nbits_of_ebgn_a > nbits */
    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

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
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0050);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0051);

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
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ_0052);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ_0053);

        while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EBGN_ITEM_NEXT(item_a) != EBGN_NULL_ITEM(ebgn_a) )
    {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0054);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0055);

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
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0056);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0057);

            bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_of_item_tmp);

            EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
            EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
            EBGN_ITEM_DEL(item_tmp);

            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0058);
            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0059);

        EBGN_DEC_LEN(ebgn_tmp);

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        }

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ_0060);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ_0061);
    }

    EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
           ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*
*  ebgn_c = (ebgn_a << nbits)
*
*  note,
*    1. the sgn will not be changed
*
**/
UINT32 ebgn_z_shl(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, UINT32 nbits, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

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
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_shl: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_shl: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_shl: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }
    return ( 0 );
    }

    if( 0 == nbits )
    {
    if( ebgn_a == ebgn_c )
    {
        return ( 0 );
    }
    return ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    /* now a > 0 and nbits > 0 */
    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    bgn_offset = nbits / BIGINTSIZE;
    bit_offset = nbits % BIGINTSIZE;

    if( 0 < bit_offset )
    {
        item_a = EBGN_LAST_ITEM(ebgn_a);
        /* handle the last item ( highest item ) */
    if( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0062);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0063);

            bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), BIGINTSIZE - bit_offset, bgn_of_item_tmp);

            EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
            EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);
    }

        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ_0064);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ_0065);

        while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EBGN_ITEM_PREV(item_a) != EBGN_NULL_ITEM(ebgn_a) )
    {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0066);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0067);

        bgn_z_shl_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_c0);
        bgn_z_shr_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(EBGN_ITEM_PREV(item_a)), BIGINTSIZE - bit_offset, bgn_c1);
        bgn_z_bit_or(ebgnz_md_id, bgn_c0, bgn_c1, bgn_of_item_tmp);

            EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
            EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

            item_a = EBGN_ITEM_PREV(item_a);
    }

        /* handle the first item ( lowest item ) */
    if( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0068);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0069);

            bgn_z_shl_no_shift_out(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_of_item_tmp);

            EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
            EBGN_ADD_ITEM_HEAD(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
            EBGN_ITEM_DEL(item_tmp);

            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0070);
            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0071);

        EBGN_DEC_LEN(ebgn_tmp);

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
        }

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ_0072);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ_0073);

        EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);
    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    }
    else
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    for( ; 0 < bgn_offset; bgn_offset -- )
    {
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0074);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0075);

           bgn_z_set_zero(bgnz_md_id, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_HEAD(ebgn_c, item_tmp);
    EBGN_INC_LEN(ebgn_c);
    }

    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*
*  ebgn_a = -ebgn_a
*
**/
static UINT32 ebgn_z_neg_self(const UINT32  ebgnz_md_id, EBGN *ebgn_a)
{

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_neg_self: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_neg_self: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* a = 0 */
    if( 0 == EBGN_LEN(ebgn_a) )
    {
    EBGN_SGN(ebgn_a) = EC_TRUE;
    return ( 0 );
    }

    /* a > 0 */
    if( EC_TRUE == EBGN_SGN(ebgn_a) )
    {
    EBGN_SGN(ebgn_a) = EC_FALSE;
    return ( 0 );
    }

    /* a < 0 */
    EBGN_SGN(ebgn_a) = EC_TRUE;
    return ( 0 );
}

/**
*
*
*  ebgn_b = -ebgn_a
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_z_neg(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_b)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_neg: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_neg: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_neg: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_b )
    {
        return ebgn_z_neg_self(ebgnz_md_id, ebgn_b);
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_b) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_b);
    }

    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_b);

    /* a = 0 */
    if( 0 == EBGN_LEN(ebgn_a) )
    {
        EBGN_SET_SGN(ebgn_b, EC_TRUE);
        return ( 0 );
    }

    /* a > 0 */
    if( EC_TRUE == EBGN_SGN(ebgn_a) )
    {
        EBGN_SET_SGN(ebgn_b, EC_FALSE);
        return ( 0 );
    }

    EBGN_SET_SGN(ebgn_b, EC_TRUE);
    return ( 0 );
}

/**
*
*
*  ebgn_b = abs(ebgn_a)
*
*  note, maybe address of ebgn_a = address of ebgn_b
*
**/
UINT32 ebgn_z_abs(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_b)
{
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_b )
    {
    EBGN_SGN(ebgn_b) = EC_TRUE;
    return ( 0 );
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_b) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_b);
    }

    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_b);
    EBGN_SGN(ebgn_b) = EC_TRUE;

    return ( 0 );
}

/**
*
*   ebgn ++
*
**/
UINT32 ebgn_z_inc(const UINT32  ebgnz_md_id, EBGN *ebgn)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item;
    BIGINT *bgn;

    UINT32 carry;
    UINT32 borrow;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inc: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_inc: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* a = 0 */
    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn) )
    {
    return ebgn_z_set_one(ebgnz_md_id, ebgn);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    /* a > 0 */
    if( EC_TRUE == EBGN_SGN(ebgn) )
    {
        carry = 1;

        item = EBGN_FIRST_ITEM(ebgn);

        while ( item != EBGN_NULL_ITEM(ebgn) )
        {
            carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item), carry, EBGN_ITEM_BGN(item));
        if( 0 == carry )
            {
                break;
            }

            item = EBGN_ITEM_NEXT(item);
        }

        if( 1 == carry )
        {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0076);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0077);

            bgn_z_set_one(bgnz_md_id, bgn);

            EBGN_ITEM_BGN(item) = bgn;
            EBGN_ADD_ITEM_TAIL(ebgn, item);
            EBGN_INC_LEN(ebgn);
        }

        return ( 0 );
    }

    /* a < 0 */
    if( EC_FALSE == EBGN_SGN(ebgn) )
    {
        /* a + 1 = - ((-a) - 1) */

            EBGN_SET_SGN(ebgn, EC_TRUE);

        borrow = 1;

        item = EBGN_FIRST_ITEM(ebgn);

        while ( item != EBGN_NULL_ITEM(ebgn) )
        {
            borrow = bgn_z_ssub(bgnz_md_id, EBGN_ITEM_BGN(item), borrow, EBGN_ITEM_BGN(item));
        if( 0 == borrow )
            {
                break;
            }

            item = EBGN_ITEM_NEXT(item);
        }
        /* due to a <= -1, hence a + 1 <= 0 and borrow must be zero here */

            EBGN_SET_SGN(ebgn, EC_FALSE);

        /* adjust length if necessary */
        item = EBGN_LAST_ITEM(ebgn);

            while ( item != EBGN_NULL_ITEM(ebgn) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item)))
            {
            bgn = EBGN_ITEM_BGN(item);
            EBGN_ITEM_DEL(item);

            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn, LOC_EBGNZ_0078);
            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item, LOC_EBGNZ_0079);

        EBGN_DEC_LEN(ebgn);

        item= EBGN_LAST_ITEM(ebgn);
        }

        if( item == EBGN_NULL_ITEM(ebgn) )
        {
                EBGN_SET_SGN(ebgn, EC_TRUE);
        }
    }

    return ( 0 );
}

/**
*
*
*   ebgn --
*
**/
UINT32 ebgn_z_dec(const UINT32  ebgnz_md_id, EBGN *ebgn)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item;
    BIGINT *bgn;

    UINT32 carry;
    UINT32 borrow;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_dec: ebgn is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_dec: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* a = 0 */
    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn) )
    {
    ebgn_z_set_one(ebgnz_md_id, ebgn);
    EBGN_SET_SGN(ebgn, EC_FALSE);

    return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    /* a > 0 */
    if( EC_TRUE == EBGN_SGN(ebgn) )
    {
        borrow = 1;
        item = EBGN_FIRST_ITEM(ebgn);

        while ( item != EBGN_NULL_ITEM(ebgn) )
        {
            borrow = bgn_z_ssub(bgnz_md_id, EBGN_ITEM_BGN(item), borrow, EBGN_ITEM_BGN(item));
            if( 0 == borrow )
            {
                break;
            }

            item = EBGN_ITEM_NEXT(item);
        }
        /* due to a >= 1, hence a - 1 >= 0 and borrow must be zero here */

        /* adjust length if necessary */
        item = EBGN_LAST_ITEM(ebgn);

        while ( item != EBGN_NULL_ITEM(ebgn) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item)))
        {
            bgn = EBGN_ITEM_BGN(item);
            EBGN_ITEM_DEL(item);

            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn, LOC_EBGNZ_0080);
            free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item, LOC_EBGNZ_0081);

            EBGN_DEC_LEN(ebgn);

            item = EBGN_LAST_ITEM(ebgn);
        }
    }

    /* a < 0 */
    if( EC_FALSE == EBGN_SGN(ebgn) )
    {
        carry = 1;

        item = EBGN_FIRST_ITEM(ebgn);

        while ( item != EBGN_NULL_ITEM(ebgn) )
        {
            carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item), carry, EBGN_ITEM_BGN(item));
        if( 0 == carry )
            {
                break;
            }

            item = EBGN_ITEM_NEXT(item);
        }

        if( 1 == carry )
        {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item, LOC_EBGNZ_0082);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn, LOC_EBGNZ_0083);

            bgn_z_set_one(bgnz_md_id, bgn);

            EBGN_ITEM_BGN(item) = bgn;
            EBGN_ADD_ITEM_TAIL(ebgn, item);
            EBGN_INC_LEN(ebgn);
        }
    }

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_c) + abs(ebgn_a)
*
*  note, maybe address of ebgn_c = address of ebgn_a
*
**/
static UINT32 ebgn_z_abs_adc(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 carry;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_adc: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_adc: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_adc: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    carry = 0;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_c = EBGN_FIRST_ITEM(ebgn_c);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_c != EBGN_NULL_ITEM(ebgn_c) )
    {
        carry = bgn_z_adc(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_c), carry, EBGN_ITEM_BGN(item_c));

        item_a = EBGN_ITEM_NEXT(item_a);
        item_c = EBGN_ITEM_NEXT(item_c);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0084);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0085);

    if( 0 < carry )
    {
            carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_a), carry, bgn_of_item_c);
    }
    else
    {
        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_c);
    }

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( item_c != EBGN_NULL_ITEM(ebgn_c) && 0 < carry )
    {
        carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_c), carry, EBGN_ITEM_BGN(item_c));

        item_c = EBGN_ITEM_NEXT(item_c);
    }

    if( 0 < carry )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0086);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0087);

        bgn_z_set_one(bgnz_md_id, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);
    }

    EBGN_SET_SGN(ebgn_c, EC_TRUE);

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_a) + bgn_b
*
*
**/
static UINT32 ebgn_z_abs_sadd(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 carry;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sadd: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sadd: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sadd: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_sadd: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a != ebgn_c )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    carry = 0;

    item_c = EBGN_FIRST_ITEM(ebgn_c);
    while ( item_c != EBGN_NULL_ITEM(ebgn_c) )
    {
        carry = bgn_z_adc(bgnz_md_id, EBGN_ITEM_BGN(item_c), bgn_b, carry, EBGN_ITEM_BGN(item_c));

        item_c = EBGN_ITEM_NEXT(item_c);
    }

    if( 0 < carry )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0088);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0089);

        bgn_z_set_one(bgnz_md_id, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);
    }

    EBGN_SGN(ebgn_c) = EC_TRUE;

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_a) + abs(ebgn_b)
*
*
**/
static UINT32 ebgn_z_abs_add(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;
    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 carry;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_add: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_c )
    {
    return ebgn_z_abs_adc(ebgnz_md_id, ebgn_b, ebgn_c);
    }

    if( ebgn_b == ebgn_c )
    {
    return ebgn_z_abs_adc(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
    ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    carry = 0;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0090);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0091);

        carry = bgn_z_adc(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), carry, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0092);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0093);

        carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_a), carry, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0094);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0095);

        carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_b), carry, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    if( 0 < carry )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0096);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0097);

        bgn_z_set_one(bgnz_md_id, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);
    }
#if 0
    else
    {
    item_c = EBGN_LAST_ITEM(ebgn_c);
    while( item_c != EBGN_NULL_ITEM(ebgn_c) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_c)))
    {
        EBGN_ITEM_DEL(item_c);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_c), LOC_EBGNZ_0098);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_c, LOC_EBGNZ_0099);
        EBGN_DEC_LEN(ebgn_c);
    }
    }
#endif

    EBGN_SET_SGN(ebgn_c, EC_TRUE);

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_a) + abs(ebgn_b) * M ^ offset where M = 2 ^ BIGINTSIZE
*
*
**/
static UINT32 ebgn_z_abs_add_offset(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, const UINT32 offset, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;
    EBGN_ITEM *item_tmp;

    BIGINT *bgn_of_item_tmp;

    UINT32 idx;
    UINT32 carry;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add_offset: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add_offset: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_add_offset: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_add_offset: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    idx = offset;

    item_a = EBGN_FIRST_ITEM(ebgn_a);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < idx )
    {
    idx --;

        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0100);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0101);

        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while( 0 < idx )
    {
    idx --;

        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0102);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0103);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);
    }

    carry = 0;

    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0104);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0105);

        carry = bgn_z_adc(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), carry, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0106);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0107);

        carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_a), carry, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0108);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0109);

        carry = bgn_z_sadd(bgnz_md_id, EBGN_ITEM_BGN(item_b), carry, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    if( 0 < carry )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0110);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0111);

        bgn_z_set_one(bgnz_md_id, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
    EBGN_INC_LEN(ebgn_tmp);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);

    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_a) - abs(ebgn_c) where abs(ebgn_a) >= abs(ebgn_c)
*
*
**/
static UINT32 ebgn_z_abs_sbb_a_c(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 borrow;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sbb_a_c: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sbb_a_c: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_sbb_a_c: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    borrow = 0;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_c = EBGN_FIRST_ITEM(ebgn_c);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_c != EBGN_NULL_ITEM(ebgn_c) )
    {
        borrow = bgn_z_sbb(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_c), borrow, EBGN_ITEM_BGN(item_c));

        item_a = EBGN_ITEM_NEXT(item_a);
        item_c = EBGN_ITEM_NEXT(item_c);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0112);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0113);

    if( 0 < borrow )
    {
            borrow = bgn_z_ssub(bgnz_md_id, EBGN_ITEM_BGN(item_a), borrow, bgn_of_item_c);
    }
    else
    {
        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_c);
    }

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    /* adjust length if necessary */
    item_c = EBGN_LAST_ITEM(ebgn_c);

    while ( item_c != EBGN_NULL_ITEM(ebgn_c) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_c)))
    {
        bgn_of_item_c = EBGN_ITEM_BGN(item_c);
        EBGN_ITEM_DEL(item_c);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_of_item_c, LOC_EBGNZ_0114);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_c, LOC_EBGNZ_0115);

    EBGN_DEC_LEN(ebgn_c);

    item_c = EBGN_LAST_ITEM(ebgn_c);
    }

    EBGN_SET_SGN(ebgn_c, EC_TRUE);

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_c) - abs(ebgn_a) where abs(ebgn_c) >= abs(ebgn_a)
*
*
**/
static UINT32 ebgn_z_abs_sbb_c_a(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 borrow;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sbb_c_a: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sbb_c_a: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_sbb_c_a: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    borrow = 0;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_c = EBGN_FIRST_ITEM(ebgn_c);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_c != EBGN_NULL_ITEM(ebgn_c) )
    {
        borrow = bgn_z_sbb(bgnz_md_id, EBGN_ITEM_BGN(item_c), EBGN_ITEM_BGN(item_a), borrow, EBGN_ITEM_BGN(item_c));

        item_a = EBGN_ITEM_NEXT(item_a);
        item_c = EBGN_ITEM_NEXT(item_c);
    }

    while ( item_c != EBGN_NULL_ITEM(ebgn_c) && 0 < borrow )
    {
        borrow = bgn_z_ssub(bgnz_md_id, EBGN_ITEM_BGN(item_c), borrow, EBGN_ITEM_BGN(item_c));

        item_c = EBGN_ITEM_NEXT(item_c);
    }

    /* adjust length if necessary */
    item_c = EBGN_LAST_ITEM(ebgn_c);

    while ( item_c != EBGN_NULL_ITEM(ebgn_c) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_c)))
    {
        bgn_of_item_c = EBGN_ITEM_BGN(item_c);
        EBGN_ITEM_DEL(item_c);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_of_item_c, LOC_EBGNZ_0116);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_c, LOC_EBGNZ_0117);

    EBGN_DEC_LEN(ebgn_c);

    item_c = EBGN_LAST_ITEM(ebgn_c);
    }

    EBGN_SET_SGN(ebgn_c, EC_TRUE);

    return ( 0 );
}

/**
*
*
*  ebgn_c = abs(ebgn_a) - abs(ebgn_b) where abs(ebgn_a) >= abs(ebgn_b)
*
*
**/
static UINT32 ebgn_z_abs_sub(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;
    EBGN_ITEM *item_c;

    BIGINT *bgn_of_item_c;
    UINT32 borrow;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sub: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sub: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_sub: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_sub: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_c )
    {
    return ebgn_z_abs_sbb_c_a(ebgnz_md_id, ebgn_b, ebgn_c);
    }

    if( ebgn_b == ebgn_c )
    {
    return ebgn_z_abs_sbb_a_c(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
    ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    borrow = 0;

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    item_b = EBGN_FIRST_ITEM(ebgn_b);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && item_b != EBGN_NULL_ITEM(ebgn_b) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0118);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0119);

        borrow = bgn_z_sbb(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), borrow, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
        item_b = EBGN_ITEM_NEXT(item_b);
    }

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_c, LOC_EBGNZ_0120);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_c, LOC_EBGNZ_0121);

        borrow = bgn_z_ssub(bgnz_md_id, EBGN_ITEM_BGN(item_a), borrow, bgn_of_item_c);

        EBGN_ITEM_BGN(item_c) = bgn_of_item_c;
        EBGN_ADD_ITEM_TAIL(ebgn_c, item_c);
    EBGN_INC_LEN(ebgn_c);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    /* adjust length if necessary */
    item_c = EBGN_LAST_ITEM(ebgn_c);

    while ( item_c != EBGN_NULL_ITEM(ebgn_c) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_c)))
    {
        bgn_of_item_c = EBGN_ITEM_BGN(item_c);
        EBGN_ITEM_DEL(item_c);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_of_item_c, LOC_EBGNZ_0122);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_c, LOC_EBGNZ_0123);

    EBGN_DEC_LEN(ebgn_c);

    item_c = EBGN_LAST_ITEM(ebgn_c);
    }

    EBGN_SET_SGN(ebgn_c, EC_TRUE);

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a + ebgn_b
*
**/
UINT32 ebgn_z_add(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    UINT32 sgn;
    UINT32 ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_add: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_add: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_add: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_add: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    /* if ebgn_a >= 0, ebgn_b >= 0 or ebgn_a < 0, ebgn_b < 0 */
    if( EBGN_SGN(ebgn_a) == EBGN_SGN(ebgn_b) )
    {
        sgn = EBGN_SGN(ebgn_a);
        ebgn_z_abs_add(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
        EBGN_SGN(ebgn_c) = sgn;
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ret = ebgn_z_abs_cmp(ebgnz_md_id, ebgn_a, ebgn_b);

    /* ebgn_a >= 0 and ebgn_b < 0 */
    if( EC_TRUE == EBGN_SGN(ebgn_a) && EC_FALSE == EBGN_SGN(ebgn_b) )
    {
    /*  a = -b */
        if( 0 == ret )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }

    /* a > - b */
        if( 1 == ret )
        {
            return ebgn_z_abs_sub(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
        }

    /* a < - b */
        if( ((UINT32)-1) == ret )
        {
            ebgn_z_abs_sub(ebgnz_md_id, ebgn_b, ebgn_a, ebgn_c);
            EBGN_SGN(ebgn_c) = EC_FALSE;
            return ( 0 );
        }
    }

    /* ebgn_a < 0 and ebgn_b >= 0 */
    if( EC_FALSE == EBGN_SGN(ebgn_a) && EC_TRUE == EBGN_SGN(ebgn_b) )
    {
        /*  -a = b */
        if( 0 == ret )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }

        /* -a > b */
        if( 1 == ret )
        {
            /*  a + b = -( (-a) - b ) < 0 */
            ebgn_z_abs_sub(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
            EBGN_SGN(ebgn_c) = EC_FALSE;
            return ( 0 );
        }

        /* -a < b */
        if( ((UINT32)-1) == ret )
        {
            /*  a + b = b - (-a) > 0 */
            ebgn_z_abs_sub(ebgnz_md_id, ebgn_b, ebgn_a, ebgn_c);
            EBGN_SGN(ebgn_c) = EC_TRUE;
            return ( 0 );
        }
    }

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a - ebgn_b
*
**/
UINT32 ebgn_z_sub(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    UINT32 sgn;
    UINT32 ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sub: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sub: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sub: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_sub: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) && ebgn_a != ebgn_c && ebgn_b != ebgn_c )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    /* if ebgn_a >= 0, ebgn_b < 0 or ebgn_a < 0, ebgn_b >= 0 */
    /* then ebgn_a - ebgn_b = ebgn_a + (- ebgn_b) */
    if( EBGN_SGN(ebgn_a) != EBGN_SGN(ebgn_b) )
    {
        sgn = EBGN_SGN(ebgn_a);
        ebgn_z_abs_add(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
        EBGN_SGN(ebgn_c) = sgn;
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ret = ebgn_z_abs_cmp(ebgnz_md_id, ebgn_a, ebgn_b);

    /* a >= 0 and b >= 0 */
    if( EC_TRUE == EBGN_SGN(ebgn_a) && EC_TRUE == EBGN_SGN(ebgn_b) )
    {
        /*  a = b */
        if( 0 == ret )
        {
        return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }

        /* a > b */
        if( 1 == ret )
        {
        return ebgn_z_abs_sub(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
        }

        /* a < b */
        if( ((UINT32)-1) == ret )
        {
        ebgn_z_abs_sub(ebgnz_md_id, ebgn_b, ebgn_a, ebgn_c);
        EBGN_SGN(ebgn_c) = EC_FALSE;
        return ( 0 );
        }
    }

    /* a < 0 and b < 0 */
    if( EC_FALSE == EBGN_SGN(ebgn_a) && EC_FALSE == EBGN_SGN(ebgn_b) )
    {
        /*  a = b */
        if( 0 == ret )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }

        /* -a > -b , then a - b = -( (-a) - (-b) ) = -( abs(a) - abs(b) ) < 0 */
        if( 1 == ret )
        {
            ebgn_z_abs_sub(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
            EBGN_SGN(ebgn_c) = EC_FALSE;
            return ( 0 );
        }

        /* -a < -b, then a - b = (-b) - (-a) = abs(b) - abs(a) > 0*/
        if( ((UINT32)-1) == ret )
        {
            ebgn_z_abs_sub(ebgnz_md_id, ebgn_b, ebgn_a, ebgn_c);
            EBGN_SGN(ebgn_c) = EC_TRUE;
            return ( 0 );
        }
    }

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a * bgn_b
*
**/
UINT32 ebgn_z_smul(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;

    BIGINT *bgn_c0;
    BIGINT *bgn_c1;
    BIGINT *bgn_c2;
    BIGINT *bgn_tmp;

    UINT32 carry;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_smul: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == bgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_smul: bgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_smul: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_smul: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    /* if b = 0, then c = 0 */
    if ( EC_TRUE == bgn_z_is_zero(bgnz_md_id, bgn_b) )
    {
        return ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    /* if b = 1, then c = a */
    if ( EC_TRUE == bgn_z_is_one(bgnz_md_id, bgn_b) )
    {
        return ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ_0124);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ_0125);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c2, LOC_EBGNZ_0126);

    bgn_z_set_zero(bgnz_md_id, bgn_c2);

    item_a = EBGN_FIRST_ITEM(ebgn_a);

    while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0127);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0128);

        bgn_z_mul(bgnz_md_id, bgn_b, EBGN_ITEM_BGN(item_a), bgn_c0, bgn_c1);

        carry = bgn_z_add(bgnz_md_id, bgn_c0, bgn_c2, bgn_of_item_tmp); /* no carry here ! */
        if( 0 < carry )
        {
            bgn_z_sadd(bgnz_md_id, bgn_c1, carry, bgn_c1);
        }

        bgn_tmp = bgn_c2;
        bgn_c2 = bgn_c1;
        bgn_c1 = bgn_c0;
        bgn_c0 = bgn_tmp;

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c2) )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0129);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0130);

        bgn_z_clone(bgnz_md_id, bgn_c2, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);

        EBGN_INC_LEN(ebgn_tmp);
    }

    EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);
    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);

    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ_0131);
    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ_0132);
    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c2, LOC_EBGNZ_0133);

    return ( 0 );
}

/**
*
*
*   ebgn_c = abs(ebgn_a) * abs(ebgn_b)
*
**/
static UINT32 ebgn_z_abs_mul(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN *ebgn_mid;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;
    EBGN_ITEM *item_tmp;

    UINT32 offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_mul: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_mul: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_mul: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_mul: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a)
     || EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
        return ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_mid);

    if( EBGN_LEN(ebgn_a) <= EBGN_LEN(ebgn_b) )
    {
        offset = 0;
        item_a = EBGN_FIRST_ITEM(ebgn_a);
        while ( item_a != EBGN_NULL_ITEM(ebgn_a) )
        {
        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_a)) )
        {
            ebgn_z_smul(ebgnz_md_id, ebgn_b, EBGN_ITEM_BGN(item_a), ebgn_mid);
            ebgn_z_abs_add_offset(ebgnz_md_id, ebgn_tmp, ebgn_mid, offset, ebgn_tmp);
            ebgn_z_clean(ebgnz_md_id, ebgn_mid);
        }

        offset ++;
            item_a = EBGN_ITEM_NEXT(item_a);
        }
    }
    else
    {
        offset = 0;
        item_b = EBGN_FIRST_ITEM(ebgn_b);
        while ( item_b != EBGN_NULL_ITEM(ebgn_b) )
        {
        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_b)) )
        {
            ebgn_z_smul(ebgnz_md_id, ebgn_a, EBGN_ITEM_BGN(item_b), ebgn_mid);

            ebgn_z_abs_add_offset(ebgnz_md_id, ebgn_tmp, ebgn_mid, offset, ebgn_tmp);
            ebgn_z_clean(ebgnz_md_id, ebgn_mid);
        }

        offset ++;
            item_b = EBGN_ITEM_NEXT(item_b);
        }
    }

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while ( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
        EBGN_ITEM_DEL(item_tmp);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0134);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0135);

        EBGN_DEC_LEN(ebgn_tmp);

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);

    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_mid);

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a * ebgn_b
*
**/
UINT32 ebgn_z_mul(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_c)
{
    UINT32 sgn;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mul: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mul: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mul: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_mul: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a)
     || EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
    return ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    if( EBGN_SGN(ebgn_a) == EBGN_SGN(ebgn_b ) )
    {
        sgn = EC_TRUE;
    }
    else
    {
        sgn = EC_FALSE;
    }

    ebgn_z_abs_mul(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_c);
    EBGN_SGN(ebgn_c) = sgn;

    return ( 0 );
}

/**
*
*
*   ebgn_c = ebgn_a ^ 2
*
**/
UINT32 ebgn_z_squ(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tc;
    EBGN *ebgn_mid;
    EBGN *ebgn_tmp;
    EBGN *ebgn_carry;

    EBGN_ITEM *item_a;
    EBGN_ITEM *item_b;
    EBGN_ITEM *item_mid0;
    EBGN_ITEM *item_mid1;
    EBGN_ITEM *item_tmp;
    EBGN_ITEM *item_carry;

    BIGINT *bgn_of_item_tmp;

    EBGN_ITEM *cur_item_a;
    EBGN_ITEM *cur_item_b;

    BIGINT *bgn_c0;
    BIGINT *bgn_c1;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_squ: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_squ: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_squ: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
        return ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tc);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_mid);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_carry);

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_mid0, LOC_EBGNZ_0136);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_mid1, LOC_EBGNZ_0137);

    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c0, LOC_EBGNZ_0138);
    alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_c1, LOC_EBGNZ_0139);

    EBGN_ITEM_BGN(item_mid0) = bgn_c0;
    EBGN_ITEM_BGN(item_mid1) = bgn_c1;

    cur_item_a = EBGN_FIRST_ITEM(ebgn_a);
    cur_item_b = EBGN_FIRST_ITEM(ebgn_a);

    while( cur_item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        item_a = EBGN_ITEM_NEXT(cur_item_a);
    item_b = cur_item_b;

    /* here tmp is already zero */
    while( item_b != item_a && item_b != EBGN_ITEM_PREV(item_a) )
    {
        item_a = EBGN_ITEM_PREV(item_a);

        /* tmp <-- tmp + item_a * item_b */
        bgn_z_mul(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_c0, bgn_c1);

        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c1) )
        {
            EBGN_INIT(ebgn_mid);

                EBGN_ITEM_BGN(item_mid1) = bgn_c1;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid1);
            EBGN_INC_LEN(ebgn_mid);

                EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
        else if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c0) )
        {
            EBGN_INIT(ebgn_mid);

                EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    /* tmp <-- 2*tmp */
    ebgn_z_shl(ebgnz_md_id, ebgn_tmp, 1, ebgn_tmp);

    if( item_b != item_a )
    {
        /* tmp <-- item_a^2 */
        bgn_z_squ(bgnz_md_id, EBGN_ITEM_BGN(item_b), bgn_c0, bgn_c1);

        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c1) )
        {
            EBGN_INIT(ebgn_mid);

                EBGN_ITEM_BGN(item_mid1) = bgn_c1;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid1);
            EBGN_INC_LEN(ebgn_mid);

                EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
        else if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c0) )
        {
            EBGN_INIT(ebgn_mid);

                EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
    }

        /* tmp <--- tmp + carry */
    ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_carry, ebgn_tmp);

    /* DEL first item of tmp and then ADD it to TAIL of tc    */
    item_tmp = EBGN_FIRST_ITEM(ebgn_tmp);
    if( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) )
    {
        EBGN_ITEM_DEL(item_tmp);
        EBGN_DEC_LEN(ebgn_tmp);
    }
    else
    {
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0140);
            alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0141);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_tmp);

            EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
    }
    EBGN_ADD_ITEM_TAIL(ebgn_tc, item_tmp);
    EBGN_INC_LEN(ebgn_tc);

    /* carry <-- tmp */
    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_carry);

    cur_item_a = EBGN_ITEM_NEXT(cur_item_a);
    }

    cur_item_a = EBGN_ITEM_PREV(cur_item_a);
    cur_item_b = EBGN_ITEM_NEXT(cur_item_b);

    while( cur_item_b != EBGN_NULL_ITEM(ebgn_a) )
    {
        item_a = EBGN_ITEM_NEXT(cur_item_a);
    item_b = cur_item_b;

    /* here tmp is already zero */
    while( item_b != item_a && item_b != EBGN_ITEM_PREV(item_a) )
    {
        item_a = EBGN_ITEM_PREV(item_a);

        /* tmp <-- tmp + item_a * item_b */
        bgn_z_mul(bgnz_md_id, EBGN_ITEM_BGN(item_a), EBGN_ITEM_BGN(item_b), bgn_c0, bgn_c1);

        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c1) )
        {
            EBGN_INIT(ebgn_mid);

            EBGN_ITEM_BGN(item_mid1) = bgn_c1;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid1);
            EBGN_INC_LEN(ebgn_mid);

            EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
        else if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c0) )
        {
            EBGN_INIT(ebgn_mid);

            EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }

        item_b = EBGN_ITEM_NEXT(item_b);
    }

    /* tmp <-- 2*tmp */
    ebgn_z_shl(ebgnz_md_id, ebgn_tmp, 1, ebgn_tmp);

    if( item_b != item_a )
    {
        /* tmp <-- item_a^2  */
        bgn_z_squ(bgnz_md_id, EBGN_ITEM_BGN(item_b), bgn_c0, bgn_c1);

        if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c1) )
        {
            EBGN_INIT(ebgn_mid);

            EBGN_ITEM_BGN(item_mid1) = bgn_c1;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid1);
            EBGN_INC_LEN(ebgn_mid);

            EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
        else if( EC_FALSE == bgn_z_is_zero(bgnz_md_id, bgn_c0) )
        {
            EBGN_INIT(ebgn_mid);

            EBGN_ITEM_BGN(item_mid0) = bgn_c0;
            EBGN_ADD_ITEM_HEAD(ebgn_mid, item_mid0);
            EBGN_INC_LEN(ebgn_mid);

            ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_mid, ebgn_tmp);
        }
    }

        /* tmp <--- tmp + carry */
    ebgn_z_abs_add(ebgnz_md_id, ebgn_tmp, ebgn_carry, ebgn_tmp);

    /* DEL first item of tmp and then ADD it to TAIL of tc*/
    item_tmp = EBGN_FIRST_ITEM(ebgn_tmp);
    if( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) )
    {
        EBGN_ITEM_DEL(item_tmp);
        EBGN_DEC_LEN(ebgn_tmp);
    }
    else
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0142);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0143);

        bgn_z_set_zero(bgnz_md_id, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
    }
    EBGN_ADD_ITEM_TAIL(ebgn_tc, item_tmp);
    EBGN_INC_LEN(ebgn_tc);

    /* carry <-- tmp */
    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_carry);

    cur_item_b = EBGN_ITEM_NEXT(cur_item_b);
    }

    /* move carry to TAIL of tc */
    item_carry = EBGN_FIRST_ITEM(ebgn_carry);
    while( item_carry != EBGN_NULL_ITEM(ebgn_carry) )
    {
        EBGN_ITEM_DEL(item_carry);
        EBGN_DEC_LEN(ebgn_carry);

        EBGN_ADD_ITEM_TAIL(ebgn_tc, item_carry);
        EBGN_INC_LEN(ebgn_tc);

        item_carry = EBGN_FIRST_ITEM(ebgn_carry);
    }

    /* trick: disconnect with the two items */
    EBGN_INIT(ebgn_mid);

    /* move all items of tc to c */
    /* note: tc must be non-negative value due to above implementation */
    ebgn_z_move(ebgnz_md_id, ebgn_tc, ebgn_c);

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c0, LOC_EBGNZ_0144);
    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, bgn_c1, LOC_EBGNZ_0145);

    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_mid0, LOC_EBGNZ_0146);
    free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_mid1, LOC_EBGNZ_0147);

    ebgn_z_destroy(ebgnz_md_id, ebgn_tc);
    ebgn_z_destroy(ebgnz_md_id, ebgn_mid);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tmp);
    ebgn_z_destroy(ebgnz_md_id, ebgn_carry);

    return ( 0 );
}

/**
*
*
*  ebgn_c = ebgn_a mod 2 ^ nbits
*
**/
UINT32 ebgn_z_cut(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const UINT32 nbits, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_tmp;
    EBGN_ITEM *item_tmp;
    BIGINT *bgn_of_item_tmp;
    EBGN_ITEM *item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_cut: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_cut: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_cut: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( 0 == nbits )
    {
        if( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
        {
            return ebgn_z_clean(ebgnz_md_id, ebgn_c);
        }
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    bgn_offset = nbits / BIGINTSIZE;
    bit_offset = nbits % BIGINTSIZE;

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);

    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bgn_offset )
    {
        bgn_offset --;

        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0148);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0149);

        bgn_z_clone(bgnz_md_id, EBGN_ITEM_BGN(item_a), bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

        item_a = EBGN_ITEM_NEXT(item_a);
    }

    if( item_a != EBGN_NULL_ITEM(ebgn_a) && 0 < bit_offset )
    {
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, &item_tmp, LOC_EBGNZ_0150);
        alloc_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, &bgn_of_item_tmp, LOC_EBGNZ_0151);

        bgn_z_bit_cut(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset, bgn_of_item_tmp);

        EBGN_ITEM_BGN(item_tmp) = bgn_of_item_tmp;
        EBGN_ADD_ITEM_TAIL(ebgn_tmp, item_tmp);
        EBGN_INC_LEN(ebgn_tmp);

    item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    while( item_tmp != EBGN_NULL_ITEM(ebgn_tmp) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_tmp)) )
    {
        EBGN_ITEM_DEL(item_tmp);

        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_BIGINT, EBGN_ITEM_BGN(item_tmp), LOC_EBGNZ_0152);
        free_static_mem(MD_EBGNZ, ebgnz_md_id, MM_EBGN_ITEM, item_tmp, LOC_EBGNZ_0153);

        EBGN_DEC_LEN(ebgn_tmp);

        item_tmp = EBGN_LAST_ITEM(ebgn_tmp);
    }
    }

    EBGN_SGN(ebgn_tmp) = EBGN_SGN(ebgn_a);

    if( EC_FALSE == EBGN_IS_EMPTY(ebgn_c) )
    {
        ebgn_z_clean(ebgnz_md_id, ebgn_c);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_tmp, ebgn_c);
    ebgn_z_free_ebgn(ebgnz_md_id, ebgn_tmp);

    return ( 0 );
}

/**
*
*
*  ebgn_a = ebgn_b * ebgn_q + ebgn_r,
*  where
*      1. gcd(ebg_a, ebgn_b) != 0 mod 2
*    2. ebgn_a >= 0, ebgn_b > 0
*
**/
static UINT32 ebgn_z_abs_div(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q, EBGN *ebgn_r)
{
    EBGN *ebgn_x0;
    EBGN *ebgn_x1;
    EBGN *ebgn_h;
    EBGN *ebgn_mid;
    EBGN *ebgn_tmp;
    EBGN *ebgn_double_c;
    EBGN *ebgn_q_tmp;
    EBGN *ebgn_r_tmp;

    EBGN *ebgn_t;

    UINT32 nbits_of_a;
    UINT32 nbits_of_b;
    UINT32 nbits_of_c;
    UINT32 nbits_of_tmp;
    UINT32 nbits_of_precesion;

    UINT32 index;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_div: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_div: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_q )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_div: ebgn_q is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_r )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_div: ebgn_r is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( ebgn_q == ebgn_r )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_abs_div: addr of ebgn_q is equal to that of ebgn_r.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_abs_div: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"fatal error:ebgn_z_abs_div: ebgn_b is zero!\n");
        return ( -1 );
    }

    /* when a < b, then a = b * 0 + a */
    if( -1 == ebgn_z_abs_cmp(ebgnz_md_id, ebgn_a, ebgn_b) )
    {
        ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_r_tmp);
        ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_r_tmp);

        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_q) )
        {
            ebgn_z_clean(ebgnz_md_id, ebgn_q);
        }
        if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_r) )
        {
            ebgn_z_clean(ebgnz_md_id, ebgn_r);
        }

        ebgn_z_move(ebgnz_md_id, ebgn_r_tmp, ebgn_r);

        ebgn_z_free_ebgn(ebgnz_md_id, ebgn_r_tmp);

    return ( 0 );
    }

    if( EC_TRUE == ebgn_z_is_one(ebgnz_md_id, ebgn_b) )
    {
        ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_q);
        ebgn_z_set_zero(ebgnz_md_id, ebgn_r);
        return ( 0 );
    }

    /* now a >= b */
    ebgn_z_get_nbits(ebgnz_md_id, ebgn_a, &nbits_of_a);
    ebgn_z_get_nbits(ebgnz_md_id, ebgn_b, &nbits_of_b);

    /* ensure nbits_of_c > nbits_of_b */
    /* ( nbits_of_a + nbits_of_b + 2 ) >> 1 >= ( nbits_of_a + nbits_of_b + 1 ) / 2 > nbits_of_b */
    /* then c = 2 ^ nbits_of_c > b */

    /* optimized nbits of c if possible */
    if( (nbits_of_b << 1) >= nbits_of_a )
    {
        nbits_of_c = ( nbits_of_a << 1 );
    }
    else
    {
        nbits_of_c =  nbits_of_a  + ( ( nbits_of_a + 1 ) >> 1 );
    }
    nbits_of_precesion = ( ( nbits_of_b + 1 ) >> 1 );

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_x0);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_x1);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_h);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_mid);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tmp);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_double_c);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_q_tmp);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_r_tmp);

    /* set ebgn_double_c = 2 * c = 2^ (nbits_of_c + 1) */
    ebgn_z_set_e(ebgnz_md_id, nbits_of_c + 1, ebgn_double_c);

    /* set x0 = c / 2 ^ (nbits_of_b) < c / b */
    ebgn_z_set_e(ebgnz_md_id, nbits_of_c - nbits_of_b, ebgn_x0);

    index = 0;
    do
    {
        //fprintf(LOGSTDOUT," ----------------------------- loop %d --------------------\n", index);
        /* let x1 = (2 * x0 * c -  b * x0^2) / c */
        /*        = x0 * ( 2 * c - b * x0 ) / c  */

        /*****************************************************/
        /* consider function:                      */
        /*     f(x) = c / x - b                 */
        /* where c is defined above                 */
        /* Newton iteration is                      */
        /*    x1 =  2 * x0 - (b * x0^2) / c             */
        /* take the lower integer                 */
        /*    x1 = (2 * x0 * c -  b * x0^2) / c            */
        /*   about = (2 * x0 * c -  b * x0^2) >> nbits_of_c  */
            /* here take the upper integer is also ok            */
        /*    x1 = (2 * x0 * c -  b * x0^2) / c            */
        /* about = (2*x0*c -  b*x0^2 + (c-1)) >> nbits_of_c  */
        /*****************************************************/
        ebgn_z_abs_mul(ebgnz_md_id, ebgn_b, ebgn_x0, ebgn_tmp);
        ebgn_z_abs_sub(ebgnz_md_id, ebgn_double_c, ebgn_tmp, ebgn_h);
        ebgn_z_abs_mul(ebgnz_md_id, ebgn_x0, ebgn_h, ebgn_mid);

        ebgn_z_shr(ebgnz_md_id, ebgn_mid, nbits_of_c, ebgn_x1);

        ebgn_z_bit_xor(ebgnz_md_id, ebgn_x0, ebgn_x1, ebgn_tmp);
        ebgn_z_get_nbits(ebgnz_md_id, ebgn_tmp, &nbits_of_tmp);

        ebgn_t  = ebgn_x0;
        ebgn_x0 = ebgn_x1;
        ebgn_x1 = ebgn_t;

        index ++;
    }while( nbits_of_tmp > nbits_of_precesion && index < 50);

    /* a / b = ( a / c) * ( c / b ) = (a * ( c / b ) ) / c */
    ebgn_z_abs_mul(ebgnz_md_id, ebgn_a, ebgn_x1, ebgn_tmp);
    ebgn_z_shr(ebgnz_md_id, ebgn_tmp, nbits_of_c, ebgn_q_tmp);

    ebgn_z_abs_mul(ebgnz_md_id, ebgn_b, ebgn_q_tmp, ebgn_tmp);
    ebgn_z_abs_sub(ebgnz_md_id, ebgn_a, ebgn_tmp, ebgn_r_tmp);

    if( 0 <= ebgn_z_abs_cmp(ebgnz_md_id, ebgn_r_tmp, ebgn_b) )
    {
        ebgn_z_inc(ebgnz_md_id, ebgn_q_tmp);
        ebgn_z_abs_sub(ebgnz_md_id, ebgn_r_tmp, ebgn_b, ebgn_r_tmp);     /* r */
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_q) )
    {
       ebgn_z_clean(ebgnz_md_id, ebgn_q);
    }

    if ( EC_FALSE == EBGN_IS_EMPTY(ebgn_r) )
    {
       ebgn_z_clean(ebgnz_md_id, ebgn_r);
    }

    ebgn_z_move(ebgnz_md_id, ebgn_q_tmp, ebgn_q);
    ebgn_z_move(ebgnz_md_id, ebgn_r_tmp, ebgn_r);

    ebgn_z_destroy(ebgnz_md_id, ebgn_x0);
    ebgn_z_destroy(ebgnz_md_id, ebgn_x1);
    ebgn_z_destroy(ebgnz_md_id, ebgn_h);
    ebgn_z_destroy(ebgnz_md_id, ebgn_mid);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tmp);
    ebgn_z_destroy(ebgnz_md_id, ebgn_double_c);
    ebgn_z_destroy(ebgnz_md_id, ebgn_q_tmp);
    ebgn_z_destroy(ebgnz_md_id, ebgn_r_tmp);

    return ( 0 );
}

/**
*
*
*  ebgn_a = ebgn_b * ebgn_q + ebgn_r
*
**/
UINT32 ebgn_z_div(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_q, EBGN *ebgn_r)
{
    EBGN *ebgn_ta;
    EBGN *ebgn_tb;

    UINT32 exp_of_a;
    UINT32 exp_of_b;

    UINT32 sgn_of_ebgn_a;
    UINT32 sgn_of_ebgn_b;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_div: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_div: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_q )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_div: ebgn_q is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_r )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_div: ebgn_r is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_div: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"fatal error:ebgn_z_div: ebgn_b is zero!\n");
        return ( -1 );
    }

    sgn_of_ebgn_a = EBGN_SGN(ebgn_a);
    sgn_of_ebgn_b = EBGN_SGN(ebgn_b);

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tb);

    ebgn_z_get_nevens(ebgnz_md_id, ebgn_a, &exp_of_a);
    ebgn_z_get_nevens(ebgnz_md_id, ebgn_b, &exp_of_b);

    if( exp_of_b <= exp_of_a )
    {
        if( 0 < exp_of_b )
        {
            ebgn_z_shr(ebgnz_md_id, ebgn_a, exp_of_b, ebgn_ta);
            ebgn_z_shr(ebgnz_md_id, ebgn_b, exp_of_b, ebgn_tb);

            ebgn_z_abs(ebgnz_md_id, ebgn_ta, ebgn_ta);
            ebgn_z_abs(ebgnz_md_id, ebgn_tb, ebgn_tb);

            ebgn_z_abs_div(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_q, ebgn_r);

            ebgn_z_shl(ebgnz_md_id, ebgn_r, exp_of_b, ebgn_r);
        }
        else
        {
            ebgn_z_abs(ebgnz_md_id, ebgn_a, ebgn_ta);
            ebgn_z_abs(ebgnz_md_id, ebgn_b, ebgn_tb);

            ebgn_z_abs_div(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_q, ebgn_r);
        }
    }
    else
    {
        if( 0 < exp_of_a )
        {
            ebgn_z_shr(ebgnz_md_id, ebgn_a, exp_of_a, ebgn_ta);
            ebgn_z_shr(ebgnz_md_id, ebgn_b, exp_of_a, ebgn_tb);

            ebgn_z_abs(ebgnz_md_id, ebgn_ta, ebgn_ta);
            ebgn_z_abs(ebgnz_md_id, ebgn_tb, ebgn_tb);

            ebgn_z_abs_div(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_q, ebgn_r);

            ebgn_z_shl(ebgnz_md_id, ebgn_r, exp_of_a, ebgn_r);
        }
        else
        {
            ebgn_z_abs(ebgnz_md_id, ebgn_a, ebgn_ta);
            ebgn_z_abs(ebgnz_md_id, ebgn_b, ebgn_tb);

            ebgn_z_abs_div(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_q, ebgn_r);
        }
    }

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tb);

    if( EC_TRUE == sgn_of_ebgn_a )
    {
        /* a > 0, b > 0 */
        if( EC_TRUE == sgn_of_ebgn_b )
        {
        }
            /* a > 0, b < 0, then q = -q */
        else
        {
            /* do not change ebgn_q sgn directly due to it maybe zero */
            ebgn_z_neg_self(ebgnz_md_id, ebgn_q);
        }
    }
    else
    {
        /* a < 0, b > 0, then q = -q, r = -r */
        if( EC_TRUE == sgn_of_ebgn_b )
        {
            /* do not change ebgn_q sgn or ebgn_r sgn directly due to it maybe zero */
            ebgn_z_neg_self(ebgnz_md_id, ebgn_q);
            ebgn_z_neg_self(ebgnz_md_id, ebgn_r);
        }
            /* a < 0, b < 0, then r = -r */
        else
        {
            /* do not change ebgn_r sgn directly due to it maybe zero */
            ebgn_z_neg_self(ebgnz_md_id, ebgn_r);
        }
    }

    return ( 0 );
}

/**
*
*
*  ebgn_r = abs(ebgn_a) mod abs(ebgn_b)
*
**/
UINT32 ebgn_z_mod(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_b, EBGN *ebgn_r)
{
    EBGN *ebgn_q;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mod: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mod: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_r )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_mod: ebgn_r is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_mod: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"fatal error:ebgn_z_mod: ebgn_b is zero!\n");
        return ( -1 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_q);

    ebgn_z_abs_div(ebgnz_md_id, ebgn_a, ebgn_b, ebgn_q, ebgn_r);

    ebgn_z_destroy(ebgnz_md_id, ebgn_q);

    return ( 0 );
}

/**
*
*   let ebgn_a = ebgn_q * 2^e where ebgn_q is odd and e >=0
*   return e
*
**/
UINT32 ebgn_z_get_nevens(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, UINT32 *e)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN_ITEM *item_a;

    UINT32 bgn_offset;
    UINT32 bit_offset;
    UINT32 nbits_of_item_a;
    UINT32 nevens;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_get_nevens: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == e )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_get_nevens: e is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_get_nevens: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_TRUE == ebgn_z_is_odd(ebgnz_md_id, ebgn_a) )
    {
        *e = 0;
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    bgn_offset = 0;
    item_a = EBGN_FIRST_ITEM(ebgn_a);
    while ( item_a != EBGN_NULL_ITEM(ebgn_a) && EC_TRUE == bgn_z_is_zero(bgnz_md_id, EBGN_ITEM_BGN(item_a)) )
    {
        bgn_offset ++;
        item_a = EBGN_ITEM_NEXT(item_a);
    }

    bit_offset = 0;
    if( item_a != EBGN_NULL_ITEM(ebgn_a) )
    {
        nbits_of_item_a = bgn_z_get_nbits(bgnz_md_id, EBGN_ITEM_BGN(item_a));

        for ( bit_offset = 0; bit_offset < nbits_of_item_a; bit_offset ++ )
        {
            if ( 1 == bgn_z_get_bit(bgnz_md_id, EBGN_ITEM_BGN(item_a), bit_offset) )
            {
                    break;
            }
        }
    }

    nevens = bgn_offset * BIGINTSIZE + bit_offset;

    *e = nevens;

    return ( 0 );
}

/**
*
*   let ebgn_a = ebgn_q * 2^e where ebgn_q is odd and e >=0
*
**/
UINT32 ebgn_z_evens(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, UINT32 *e, EBGN *ebgn_q)
{
    UINT32 nevens;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_evens: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == e )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_evens: e is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_q )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_evens: ebgn_q is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/
#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_evens: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    ebgn_z_get_nevens(ebgnz_md_id, ebgn_a, &nevens);
    *e = nevens;

    ebgn_z_shr(ebgnz_md_id, ebgn_a, nevens, ebgn_q);

    return ( 0 );
}

/**
*       c = ( a ^ e )
*
*   Note:
*       let e = 2 * e1 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e
*   Output: c such that c = ( a ^ e )
*   1.  set c = 1
*   2.  while ( 1 ) do
*   2.1     if e is odd, then do
*               c = c * a
*   2.2     end if
*   2.3     e  = ( e >> 1)
*   2.4     if e is zero, then break while, end if
*   2.5     a = a ^ 2
*   2.6     next while
*   3.  return c
*
**/
UINT32 ebgn_z_sexp(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const UINT32 e, EBGN *ebgn_c)
{
    EBGN *ebgn_ta;
    UINT32 te;
    UINT32 sgn;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sexp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sexp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sexp: ebgnz module #0x%lx not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_z_is_zero( ebgnz_md_id, ebgn_a ) )
    {
        ebgn_z_set_zero( ebgnz_md_id, ebgn_c );
        return ( 0 );
    }

    if( 0 == e || EC_TRUE == ebgn_z_is_one( ebgnz_md_id, ebgn_a ) )
    {
        ebgn_z_set_one( ebgnz_md_id, ebgn_c );
        return ( 0 );
    }

    if( e & 1 )
    {
        sgn = EBGN_SGN(ebgn_a);
    }
    else
    {
        sgn = EC_TRUE;
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);

    te = e;
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_ta);
    ebgn_z_set_one(ebgnz_md_id, ebgn_c);

    while ( 1 )
    {
        if ( te & 1 )
        {
            ebgn_z_abs_mul(ebgnz_md_id, ebgn_c, ebgn_ta, ebgn_c);
        }
        te >>= 1;

        if ( 0 == te )
        {
            break;
        }

        ebgn_z_squ(ebgnz_md_id, ebgn_ta, ebgn_ta);
    }

    EBGN_SGN(ebgn_c) = sgn;

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);

    return ( 0 );
}

/**
*       c = a ^ e
*
*   Note:
*       let e = e1 * 2 + e0,where e0 = 0 or 1,then
*           a ^ e = ( (a^2)^e1 ) * (a^e0)
*   Algorithm:
*   Input: a and e
*   Output: c such that c = ( a ^ e )
*   0.  let e = SUM(e_i * 2 ^ i, i = 0..m)
*   1.  set c = 1
*   2.  set i = 0
*   3.  while ( 1 ) do
*   3.1     if e_i is odd, then do
*               c = c * a
*   3.2     end if
*   3.3     i = i + 1
*   3.4     if i = m + 1, then break while, end if
*   3.5     a = a ^ 2
*   3.6     next while
*   4.  return c
*
**/
UINT32 ebgn_z_exp(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const BIGINT *bgn_e, EBGN *ebgn_c)
{
    EBGNZ_MD *ebgnz_md;
    UINT32  bgnz_md_id;

    EBGN *ebgn_ta;

    UINT32 nbits_of_e;
    UINT32 e_bit;
    UINT32 index;

    UINT32 sgn;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_exp: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == bgn_e )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_exp: bgn_e is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_exp: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_exp: ebgnz module #0x%lx not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_z_is_zero( ebgnz_md_id, ebgn_a ) )
    {
        ebgn_z_set_zero( ebgnz_md_id, ebgn_c );
        return ( 0 );
    }

    ebgnz_md = &(g_ebgnz_md[ ebgnz_md_id ]);
    bgnz_md_id = ebgnz_md->bgnz_md_id;

    if ( EC_TRUE == ebgn_z_is_one(ebgnz_md_id, ebgn_a)
      || EC_TRUE == bgn_z_is_zero(bgnz_md_id, bgn_e))
    {
        ebgn_z_set_one( ebgnz_md_id, ebgn_c );
        return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_ta);

    if( EC_TRUE == bgn_z_is_odd(bgnz_md_id, bgn_e) )
    {
        sgn = EBGN_SGN(ebgn_a);
    }
    else
    {
        sgn = EC_TRUE;
    }

    nbits_of_e = bgn_z_get_nbits( bgnz_md_id, bgn_e);

    ebgn_z_set_one( ebgnz_md_id, ebgn_c );
    index = 0;

    while ( 1 )
    {
        e_bit = bgn_z_get_bit(bgnz_md_id, bgn_e, index);
        if ( 1 == e_bit )
        {
            ebgn_z_abs_mul(ebgnz_md_id, ebgn_c, ebgn_ta, ebgn_c);
        }

        index ++;

        /*modify index == te_nbits to index >= te_nbits */
        /*in order to deal with e = 0*/
        if ( index >= nbits_of_e )
        {
            break;
        }
        ebgn_z_squ(ebgnz_md_id, ebgn_ta, ebgn_ta);
    }

    EBGN_SGN(ebgn_c) = sgn;

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);

    return ( 0 );
}

/**
*
*   return ebgn_c = GCD( abs(ebgn_a), abs(ebgn_b) )
*
*   use plain Euclidean algorithm for GCD
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
UINT32 ebgn_z_gcdeucl(const UINT32  ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_c)
{
    EBGN *ebgn_ta;
    EBGN *ebgn_tb;
    EBGN *ebgn_tr;
    EBGN *ebgn_tq;
    EBGN *ebgn_tmp;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdeucl: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdeucl: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdeucl: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_gcdeucl: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_b )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    EBGN_SGN(ebgn_c) = EC_TRUE;

    return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tb);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tq);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tr);

    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_ta);
    EBGN_SGN(ebgn_ta) = EC_TRUE;

    ebgn_z_clone(ebgnz_md_id, ebgn_b, ebgn_tb);
    EBGN_SGN(ebgn_tb) = EC_TRUE;

    while ( EC_FALSE == ebgn_z_is_zero( ebgnz_md_id, ebgn_tb ) )
    {
        ebgn_z_abs_div(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_tq, ebgn_tr);

        ebgn_tmp = ebgn_ta;
        ebgn_ta = ebgn_tb;
        ebgn_tb = ebgn_tr;
        ebgn_tr = ebgn_tmp;
    }

    ebgn_z_move(ebgnz_md_id, ebgn_ta, ebgn_c);

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tb);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tq);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tr);

    return ( 0 );
}

/**
*
*   return ebgn_c = GCD( abs(ebgn_a), abs(ebgn_b) )
*
*   uses binary GCD algorithm
*
**/
UINT32 ebgn_z_gcd(const UINT32  ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_c)
{
    EBGN *ebgn_ta;
    EBGN *ebgn_tb;
    EBGN *ebgn_tc;

    UINT32 e_a;
    UINT32 e_b;
    UINT32 e_c;

    int ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcd: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcd: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcd: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_gcd: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_b )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    EBGN_SGN(ebgn_c) = EC_TRUE;

    return ( 0 );
    }

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_b, ebgn_c);
    EBGN_SGN(ebgn_c) = EC_TRUE;

    return ( 0 );
    }

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_c);
    EBGN_SGN(ebgn_c) = EC_TRUE;

    return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tb);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tc);

    ebgn_z_evens(ebgnz_md_id, ebgn_a, &e_a, ebgn_ta);
    EBGN_SGN(ebgn_ta) = EC_TRUE;

    ebgn_z_evens(ebgnz_md_id, ebgn_b, &e_b, ebgn_tb);
    EBGN_SGN(ebgn_tb) = EC_TRUE;

    e_c = ((e_a < e_b) ? e_a : e_b);

    while( 0 != (ret = ebgn_z_abs_cmp(ebgnz_md_id, ebgn_ta, ebgn_tb)) )
    {
    if( 0 < ret )
    {
        ebgn_z_abs_sub(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_tc);
            ebgn_z_evens(ebgnz_md_id, ebgn_tc, &e_a, ebgn_ta);
    }
    else
    {
        ebgn_z_abs_sub(ebgnz_md_id, ebgn_tb, ebgn_ta, ebgn_tc);
            ebgn_z_evens(ebgnz_md_id, ebgn_tc, &e_b, ebgn_tb);
    }
    }

    if( 0 < e_c )
    {
    ebgn_z_shl(ebgnz_md_id, ebgn_ta, e_c, ebgn_c);
    }
    else
    {
    ebgn_z_move(ebgnz_md_id, ebgn_ta, ebgn_c);
    }

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tb);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tc);

    return ( 0 );
}

/**
*
*   return ebgn_d = GCD( abs(ebgn_a), abs(ebgn_b) ) and ebgn_u, ebgn_v such that
*    ebgn_a * ebgn_u + ebgn_b * ebgn_v = ebgn_d
*    ( a * u + b * v = d )
*
*   uses binary GCD extented algorithm
*
**/
UINT32 ebgn_z_gcdext(const UINT32  ebgnz_md_id,const EBGN *ebgn_a,const EBGN *ebgn_b,EBGN *ebgn_d, EBGN *ebgn_u, EBGN *ebgn_v)
{
    EBGN *ebgn_ta;
    EBGN *ebgn_tb;
    EBGN *ebgn_td;
    EBGN *ebgn_tu;
    EBGN *ebgn_tx;
    EBGN *ebgn_v1;
    EBGN *ebgn_v3;
    EBGN *ebgn_t1;
    EBGN *ebgn_t3;

    UINT32 e_a;
    UINT32 e_b;
    UINT32 e_d;

    UINT32 sgn_a;
    UINT32 sgn_b;

    int ret;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdext: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_b )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdext: ebgn_b is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_d )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdext: ebgn_d is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_u )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdext: ebgn_u is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_v )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_gcdext: ebgn_v is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        sys_log(LOGSTDOUT,
                "error:ebgn_z_gcdext: ebgnz module #0x%lx not started.\n",
                ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( ebgn_a == ebgn_b )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_d);
    EBGN_SGN(ebgn_d) = EC_TRUE;

    ebgn_z_set_one(ebgnz_md_id, ebgn_u);
    ebgn_z_set_zero(ebgnz_md_id, ebgn_v);

    return ( 0 );
    }

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_b, ebgn_d);
    EBGN_SGN(ebgn_d) = EC_TRUE;

    ebgn_z_set_zero(ebgnz_md_id, ebgn_u);
    ebgn_z_set_one(ebgnz_md_id, ebgn_v);

    return ( 0 );
    }

    if( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_b) )
    {
    ebgn_z_clone(ebgnz_md_id, ebgn_a, ebgn_d);
    EBGN_SGN(ebgn_d) = EC_TRUE;

    ebgn_z_set_one(ebgnz_md_id, ebgn_u);
    ebgn_z_set_zero(ebgnz_md_id, ebgn_v);

    return ( 0 );
    }

    sgn_a = EBGN_SGN(ebgn_a);
    sgn_b = EBGN_SGN(ebgn_b);

    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_ta);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tb);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_td);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tu);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_tx);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_v1);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_v3);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_t1);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &ebgn_t3);

    ebgn_z_get_nevens(ebgnz_md_id, ebgn_a, &e_a);
    ebgn_z_get_nevens(ebgnz_md_id, ebgn_b, &e_b);

    e_d = ((e_a < e_b) ? e_a : e_b);

    ebgn_z_shr(ebgnz_md_id, ebgn_a, e_d, ebgn_ta);
    ebgn_z_shr(ebgnz_md_id, ebgn_b, e_d, ebgn_tb);

    EBGN_SGN(ebgn_ta) = EC_TRUE;
    EBGN_SGN(ebgn_tb) = EC_TRUE;

    ebgn_z_set_one(ebgnz_md_id, ebgn_tu);
    ebgn_z_clone(ebgnz_md_id, ebgn_ta, ebgn_td);


    while( 0 != (ret = ebgn_z_abs_cmp(ebgnz_md_id, ebgn_ta, ebgn_tb)) )
    {
    if( 0 < ret )
    {
        ebgn_z_sub(ebgnz_md_id, ebgn_ta, ebgn_tb, ebgn_td);
            ebgn_z_evens(ebgnz_md_id, ebgn_td, &e_a, ebgn_ta);

        ebgn_z_sub(ebgnz_md_id, ebgn_tu, ebgn_tx, ebgn_td);
        ebgn_z_shl(ebgnz_md_id, ebgn_td, e_a, ebgn_tu);
    }
    else
    {
        ebgn_z_sub(ebgnz_md_id, ebgn_tb, ebgn_ta, ebgn_td);
            ebgn_z_evens(ebgnz_md_id, ebgn_td, &e_b, ebgn_tb);

        ebgn_z_sub(ebgnz_md_id, ebgn_tx, ebgn_tu, ebgn_tx);
    }
    }

    if( 0 < e_d )
    {
    ebgn_z_shl(ebgnz_md_id, ebgn_ta, e_d, ebgn_d);
    }
    else
    {
    ebgn_z_move(ebgnz_md_id, ebgn_ta, ebgn_d);
    }

    if( EC_FALSE == sgn_a )
    {
    ebgn_z_neg_self(ebgnz_md_id, ebgn_tu);
    }

    ebgn_z_mul(ebgnz_md_id, ebgn_a, ebgn_tu, ebgn_tx);
    ebgn_z_sub(ebgnz_md_id, ebgn_d, ebgn_tx, ebgn_td);
    ebgn_z_div(ebgnz_md_id, ebgn_td, ebgn_b, ebgn_tx, ebgn_v);

    ebgn_z_move(ebgnz_md_id, ebgn_tu, ebgn_u);

    ebgn_z_destroy(ebgnz_md_id, ebgn_ta);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tb);
    ebgn_z_destroy(ebgnz_md_id, ebgn_td);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tu);
    ebgn_z_destroy(ebgnz_md_id, ebgn_tx);
    ebgn_z_destroy(ebgnz_md_id, ebgn_v1);
    ebgn_z_destroy(ebgnz_md_id, ebgn_v3);
    ebgn_z_destroy(ebgnz_md_id, ebgn_t1);
    ebgn_z_destroy(ebgnz_md_id, ebgn_t3);

    return ( 0 );
}

/**
*
*
*   here MUST be n > a > 0 and GCD(a,n) = 1, and return
*       c = ( 1 / a ) mod n
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
*   note about ebgn_z_inv_for_crt:
*       1. must have GCD(a, n) = 1
*       2. must be called by ebgn_z_inv interface only
*
**/
static EC_BOOL ebgn_z_inv_for_crt(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, const EBGN *ebgn_n, EBGN *ebgn_c)
{
    EBGN *u;
    EBGN *v;
    EBGN *A;
    EBGN *C;

    const EBGN *p;

    EBGN *t;


#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_n )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_n is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgnz module #0x%lx not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if( EC_FALSE == ebgn_z_is_odd(ebgnz_md_id, ebgn_n) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: not support even module yet\n");
        return ( EC_FALSE );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &u);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &v);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &A);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &C);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &t);

    p = ebgn_n;

    /* u = abs(a), v = abs(p),A = 1,C = 0 */
    ebgn_z_clone( ebgnz_md_id, ebgn_a, u );
    EBGN_SGN(u) = EC_TRUE;
    ebgn_z_clone( ebgnz_md_id, p, v );
    EBGN_SGN(v) = EC_TRUE;
    ebgn_z_set_one( ebgnz_md_id, A );
    ebgn_z_set_zero( ebgnz_md_id, C );

    while( EC_FALSE == ebgn_z_is_zero( ebgnz_md_id, u ) )
    {
        while( EC_FALSE == ebgn_z_is_odd( ebgnz_md_id, u ) )
        {
            ebgn_z_shr( ebgnz_md_id, u, 1, u );

            if( EC_TRUE == ebgn_z_is_odd( ebgnz_md_id, A ) )
            {
                /* A <--- (A + p)/2 */
                ebgn_z_abs_add( ebgnz_md_id, A, p, A );
                ebgn_z_shr( ebgnz_md_id, A, 1, A );
            }
            else
            {
                /* A <--- A/2 */
                ebgn_z_shr( ebgnz_md_id, A, 1, A );
            }
        }

        while( EC_FALSE == ebgn_z_is_odd( ebgnz_md_id, v ) )
        {
            ebgn_z_shr( ebgnz_md_id, v, 1, v );

            if( EC_TRUE == ebgn_z_is_odd( ebgnz_md_id, C ) )
            {
                /* C <--- (C + p)/2 */
                ebgn_z_abs_add( ebgnz_md_id, C, p, C );
                ebgn_z_shr( ebgnz_md_id, C, 1, C );
            }
            else
            {
                /* C <--- C/2 */
                ebgn_z_shr( ebgnz_md_id, C, 1, C );
            }
        }

        if( ebgn_z_cmp( ebgnz_md_id, u, v ) >= 0 )
        {
            /* u <-- u - v */
            ebgn_z_abs_sub( ebgnz_md_id, u, v, u );

            /* if A < C, then do */
            if ( ebgn_z_cmp( ebgnz_md_id, A, C ) < 0 )
            {
                /* A <-- A + p - C */
                ebgn_z_abs_add(ebgnz_md_id, A, p, t);
                ebgn_z_abs_sub(ebgnz_md_id, t, C, A);
            }
            /* if A >= C, then do */
            else
            {
                /* A <-- A - C */
                ebgn_z_abs_sub( ebgnz_md_id, A, C, A );
            }
        }
        else
        {
            /* v <-- v - u */
            ebgn_z_abs_sub( ebgnz_md_id, v, u, v );

            /* if C < A, then do */
            if ( ebgn_z_cmp( ebgnz_md_id, C, A ) < 0 )
            {
                /* C <-- C + p - A */
                ebgn_z_abs_add(ebgnz_md_id, C, p, t);
                ebgn_z_abs_sub(ebgnz_md_id, t, A, C);
            }
            /* if C >= A, then do */
            else
            {
                /* C <-- C - A */
                ebgn_z_abs_sub( ebgnz_md_id, C, A, C );
            }
        }
    }

    EBGN_SGN(C) = EBGN_SGN(ebgn_a);
    ebgn_z_move( ebgnz_md_id, C, ebgn_c );

    ebgn_z_destroy(ebgnz_md_id, u);
    ebgn_z_destroy(ebgnz_md_id, v);
    ebgn_z_destroy(ebgnz_md_id, A);
    ebgn_z_destroy(ebgnz_md_id, C);
    ebgn_z_destroy(ebgnz_md_id, t);

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
*       1. must have m1*m2 < 2^EBGNSIZE
*
**/
UINT32 ebgn_z_crt(const UINT32  ebgnz_md_id,const EBGN *ebgn_x1, const EBGN *ebgn_m1,const EBGN *ebgn_x2, const EBGN *ebgn_m2,EBGN *ebgn_x, EBGN *ebgn_m)
{
    EBGN *u;
    EBGN *v;
    EBGN *t0;
    EBGN *t1;
    EBGN *tx;
    EBGN *tm;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_x1 )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_x1 is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_m1 )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_m1 is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_x2 )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_x2 is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_m2 )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_m2 is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_x )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_x is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_m )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgn_m is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_inv_for_crt: ebgnz module #0x%lx not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_m1) )
    {
        ebgn_z_clone(ebgnz_md_id, ebgn_x2, ebgn_x);
        ebgn_z_clone(ebgnz_md_id, ebgn_m2, ebgn_m);
        return ( 0 );
    }

    if ( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_m2) )
    {
        ebgn_z_clone(ebgnz_md_id, ebgn_x1, ebgn_x);
        ebgn_z_clone(ebgnz_md_id, ebgn_m1, ebgn_m);
        return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &u);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &v);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &t0);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &t1);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &tx);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &tm);

    /*set tm = ebgn_m1*ebgn_m2*/
    ebgn_z_mul(ebgnz_md_id, ebgn_m1, ebgn_m2, tm);

    /*u = ebgn_m1^(-1) mod ebgn_m2*/
    if( EC_FALSE == ebgn_z_inv_for_crt(ebgnz_md_id, ebgn_m1, ebgn_m2, u) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDERR,"error:ebgn_z_crt: ebgn_z_inv_for_crt return false when try 1/m1 mod m2\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }

    /*v = ebgn_m2^(-1) mod ebgn_m1*/
    if( EC_FALSE == ebgn_z_inv_for_crt(ebgnz_md_id, ebgn_m2, ebgn_m1, v) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDERR,"error:ebgn_z_crt: ebgn_z_inv_for_crt return false when try 1/m2 mod m1\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }

    /*compute u = u*ebgn_m1*ebgn_x2 mod (ebgn_m1*ebgn_m2)*/
    ebgn_z_mul(ebgnz_md_id, u, ebgn_m1, t0);
    ebgn_z_mul(ebgnz_md_id, t0, ebgn_x2, t1);
    ebgn_z_mod(ebgnz_md_id, t1, tm, u);

    /*compute v = v*ebgn_m2*ebgn_x1 mod (ebgn_m1*ebgn_m2)*/
    ebgn_z_mul(ebgnz_md_id, v, ebgn_m2, t0);
    ebgn_z_mul(ebgnz_md_id, t0, ebgn_x1, t1);
    ebgn_z_mod(ebgnz_md_id, t1, tm, v);

    /*compute x = u + v mod (ebgn_m1*ebgn_m2)*/
    ebgn_z_add(ebgnz_md_id, u, v, tx);
    if ( 0 <= ebgn_z_cmp(ebgnz_md_id, tx, tm) )
    {
        ebgn_z_sub(ebgnz_md_id, tx, tm, ebgn_x);
    }
    else
    {
        ebgn_z_move(ebgnz_md_id, tx, ebgn_x);
    }

    /*set m = tm*/
    ebgn_z_move(ebgnz_md_id, tm, ebgn_m);

    ebgn_z_destroy(ebgnz_md_id, u);
    ebgn_z_destroy(ebgnz_md_id, v);
    ebgn_z_destroy(ebgnz_md_id, t0);
    ebgn_z_destroy(ebgnz_md_id, t1);
    ebgn_z_destroy(ebgnz_md_id, tx);
    ebgn_z_destroy(ebgnz_md_id, tm);

    return ( 0 );
}

/**
*   Squroot Ceil of a
*
*   return c such that c^2 <= a < (c+1)^2
*
**/
UINT32 ebgn_z_sqrt_ceil(const UINT32  ebgnz_md_id, const EBGN *ebgn_a, EBGN *ebgn_c)
{
    EBGN *left;
    EBGN *right;
    EBGN *mid;
    EBGN *tmp;
    EBGN *t;

    UINT32 a_nbits;
    UINT32 left_nbits;
    UINT32 right_nbits;

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn_a )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sqrt_ceil: ebgn_a is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
    if ( NULL_PTR == ebgn_c )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sqrt_ceil: ebgn_c is NULL_PTR.\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if ( MAX_NUM_OF_EBGNZ_MD <= ebgnz_md_id || 0 == g_ebgnz_md[ ebgnz_md_id ].usedcounter )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sqrt_ceil: ebgnz module #0x%lx not started.\n",ebgnz_md_id);
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

#if ( SWITCH_ON == EBGN_DEBUG_SWITCH )
    if( EC_FALSE == EBGN_SGN(ebgn_a) )
    {
        dbg_log(SEC_0076_EBGNZ, 0)(LOGSTDOUT,"error:ebgn_z_sqrt_ceil: ebgn_a < 0\n");
        dbg_exit(MD_EBGNZ, ebgnz_md_id);
    }
#endif/*EBGN_DEBUG_SWITCH*/

    if ( EC_TRUE == ebgn_z_is_zero(ebgnz_md_id, ebgn_a) )
    {
        ebgn_z_set_zero(ebgnz_md_id, ebgn_c);
        return ( 0 );
    }

    ebgn_z_alloc_ebgn(ebgnz_md_id, &left);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &right);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &mid);
    ebgn_z_alloc_ebgn(ebgnz_md_id, &tmp);

    ebgn_z_get_nbits(ebgnz_md_id, ebgn_a, &a_nbits);

    left_nbits = (a_nbits - 1) / 2;
    right_nbits = left_nbits + 1;

    ebgn_z_set_e(ebgnz_md_id, left_nbits, left);
    ebgn_z_set_e(ebgnz_md_id, right_nbits, right);

    /*now left  <= c < right*/
    ebgn_z_abs_add(ebgnz_md_id, left, right, mid);
    ebgn_z_shr(ebgnz_md_id, mid, 1, mid);

    while ( 0 < ebgn_z_abs_cmp(ebgnz_md_id, mid, left) )
    {
        ebgn_z_squ(ebgnz_md_id, mid, tmp);

        if ( 0 < ebgn_z_abs_cmp(ebgnz_md_id, tmp, ebgn_a) )
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

        ebgn_z_abs_add(ebgnz_md_id, left, right, mid);
        ebgn_z_shr(ebgnz_md_id, mid, 1, mid);
    }

    ebgn_z_move(ebgnz_md_id, mid, ebgn_c);

    ebgn_z_destroy(ebgnz_md_id, left);
    ebgn_z_destroy(ebgnz_md_id, right);
    ebgn_z_destroy(ebgnz_md_id, mid);
    ebgn_z_destroy(ebgnz_md_id, tmp);

    return ( 0 );
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
