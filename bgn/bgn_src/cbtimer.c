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
#include <unistd.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "clist.h"

#include "cbtimer.h"

#include "cmpic.inc"
#include "debug.h"

#include "task.inc"
#include "task.h"
#include "findex.inc"

static EC_BOOL __cbtimer_clean_task_func(FUNC_ADDR_NODE * func_addr_node, TASK_FUNC *task_func)
{
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(NULL_PTR == task_func)
    {
        sys_log(LOGSTDOUT, "warn:__cbtimer_clean_task_func: task_func is null\n");
        return (EC_TRUE);
    }

    if(NULL_PTR == func_addr_node)
    {
        sys_log(LOGSTDOUT, "warn:__cbtimer_clean_task_func: func_addr_node is null\n");
        return (EC_TRUE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:__cbtimer_clean_task_func: ret type %ld conv item is not defined\n",
                    func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_func->func_ret_val)
        {
            dbg_tiny_caller(2,
                            TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item),
                            CMPI_ANY_MODI,
                            task_func->func_ret_val);
            task_func->func_ret_val = 0;
        }
    }

    for( para_idx = 0; para_idx < func_addr_node->func_para_num; para_idx ++ )
    {
        FUNC_PARA *func_para;

        func_para = &(task_func->func_para[ para_idx ]);

        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_para_type[ para_idx ]);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:__cbtimer_clean_task_func: para %ld type %ld conv item is not defined\n",
                    para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            dbg_tiny_caller(2,
                            TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item),
                            CMPI_ANY_MODI,
                            func_para->para_val);
            func_para->para_val = 0;
        }
    }
    return (EC_TRUE);
}
/**
*
* new CBTIMER_NODE
*
**/
CBTIMER_NODE *cbtimer_node_new()
{
    CBTIMER_NODE *cbtimer_node;

    alloc_static_mem(MD_CTIMER, CMPI_ANY_MODI, MM_CBTIMER_NODE, &cbtimer_node, LOC_CBTIMER_0001);
    cbtimer_node_init(cbtimer_node);

    return (cbtimer_node);
}

/**
*
* init CBTIMER_NODE
*
**/
EC_BOOL cbtimer_node_init(CBTIMER_NODE *cbtimer_node)
{
    CBTIMER_NODE_NAME(cbtimer_node)         = NULL_PTR;

    CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node) = 0;
    task_func_init(CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node));
    CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node) = NULL_PTR;

    CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node)  = 0;
    task_func_init(CBTIMER_NODE_EXPIRE_HANDLER(cbtimer_node));
    CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node) = NULL_PTR;
    return (EC_TRUE);
}

/**
*
* clean CBTIMER_NODE
*
**/
EC_BOOL cbtimer_node_clean(CBTIMER_NODE *cbtimer_node)
{
    if(NULL_PTR != CBTIMER_NODE_NAME(cbtimer_node))
    {
        cstring_free(CBTIMER_NODE_NAME(cbtimer_node));
        CBTIMER_NODE_NAME(cbtimer_node) = NULL_PTR;
    }

    CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node) = 0;
    if(NULL_PTR != CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node))
    {
        __cbtimer_clean_task_func(CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node), CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node));
        task_func_init(CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node));
        CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node) = NULL_PTR;
    }

    CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node) = 0;
    if(NULL_PTR != CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node))
    {
        __cbtimer_clean_task_func(CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node), CBTIMER_NODE_EXPIRE_HANDLER(cbtimer_node));
        task_func_init(CBTIMER_NODE_EXPIRE_HANDLER(cbtimer_node));
        CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node) = NULL_PTR;
    }

    return (EC_TRUE);
}

/**
*
* free CBTIMER_NODE
*
**/
EC_BOOL cbtimer_node_free(CBTIMER_NODE *cbtimer_node)
{
    cbtimer_node_clean(cbtimer_node);
    free_static_mem(MD_CTIMER, CMPI_ANY_MODI, MM_CBTIMER_NODE, cbtimer_node, LOC_CBTIMER_0002);
    return (EC_TRUE);
}

EC_BOOL cbtimer_node_is_timeout(const CBTIMER_NODE *cbtimer_node, const CTIMET cur_time)
{
    REAL diff_nsec;

    diff_nsec = CTIMET_DIFF(CBTIMER_NODE_LAST_TIME(cbtimer_node), cur_time);
    sys_log(LOGSTDNULL, "[DEBUG] cbtimer_node_is_timeout: diff_nsec %.2f, timeout_nsec %ld\n",
                        diff_nsec, CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node));
    if(diff_nsec >= 0.0 + CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cbtimer_node_is_expire(const CBTIMER_NODE *cbtimer_node, const CTIMET cur_time)
{
    if(0 < CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node))
    {
        REAL diff_nsec;

        diff_nsec = CTIMET_DIFF(CBTIMER_NODE_START_TIME(cbtimer_node), cur_time);
        sys_log(LOGSTDOUT, "[DEBUG] cbtimer_node_is_expire: diff_nsec %.2f, expire_nsec %ld\n",
                            diff_nsec, CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node));
        if(diff_nsec >= 0.0 + CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node))
        {
            return (EC_TRUE);
        }
    }
    return (EC_FALSE);
}

EC_BOOL cbtimer_node_timeout_handle(CBTIMER_NODE *cbtimer_node)
{
    if(NULL_PTR != CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node))
    {
        TASK_FUNC    *handler;

        task_caller(CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node), CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node));
        handler = CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node);
        return (handler->func_ret_val);
    }
    return (EC_TRUE);
}

EC_BOOL cbtimer_node_expire_handle(CBTIMER_NODE *cbtimer_node)
{
    if(NULL_PTR != CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node))
    {
        TASK_FUNC    *handler;

        task_caller(CBTIMER_NODE_EXPIRE_HANDLER(cbtimer_node), CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node));
        handler = CBTIMER_NODE_EXPIRE_HANDLER(cbtimer_node);
        return (handler->func_ret_val);
    }
    return (EC_TRUE);
}

EC_BOOL cbtimer_node_match_name(const CBTIMER_NODE *cbtimer_node, const CSTRING *name)
{
    return cstring_cmp(CBTIMER_NODE_NAME(cbtimer_node), name);
}

CLIST *cbtimer_new()
{
    return clist_new(MM_IGNORE, LOC_CBTIMER_0003);
}

EC_BOOL cbtimer_init(CLIST *cbtimer_node_list)
{
    clist_init(cbtimer_node_list, MM_IGNORE, LOC_CBTIMER_0004);
    return (EC_TRUE);
}

EC_BOOL cbtimer_clean(CLIST *cbtimer_node_list)
{
    clist_clean(cbtimer_node_list, (CLIST_DATA_DATA_CLEANER)cbtimer_node_free);
    return (EC_TRUE);
}

EC_BOOL cbtimer_free(CLIST *cbtimer_node_list)
{
    cbtimer_clean(cbtimer_node_list);
    clist_free(cbtimer_node_list, LOC_CBTIMER_0005);
    return (EC_TRUE);
}


/**
*
* register a cbtimer
* the handler must look like as EC_BOOL foo(...), i.e., the function return type is EC_BOOL
* when EC_TRUE is returned, wait for next timeout
* when EC_FALSE is returned, unregister it
*
**/
EC_BOOL cbtimer_add(CLIST *cbtimer_node_list, const UINT32 expire_nsec, const UINT32 timeout_nsec, const UINT32 timeout_func_id, ...)
{
    CBTIMER_NODE *cbtimer_node;
    FUNC_ADDR_NODE *func_addr_node;
    TASK_FUNC *handler;

    UINT32 mod_type;

    UINT32 para_idx;
    va_list ap;

    mod_type = (timeout_func_id >> (WORDSIZE / 2));
    if( MD_END <= mod_type )
    {
        sys_log(LOGSTDERR, "error:cbtimer_add: invalid timeout_func_id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(timeout_func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cbtimer_add: failed to fetch func addr node by func id %lx\n", timeout_func_id);
        return (EC_FALSE);
    }

    cbtimer_node = cbtimer_node_new();
    if(NULL_PTR == cbtimer_node)
    {
        sys_log(LOGSTDOUT, "error:cbtimer_add: new cbtimer node failed\n");
        return (EC_FALSE);
    }

    CBTIMER_NODE_EXPIRE_NSEC(cbtimer_node)   = expire_nsec;
    CBTIMER_NODE_TIMEOUT_NSEC(cbtimer_node)  = timeout_nsec;

    CBTIMER_NODE_EXPIRE_FUNC_ADDR_NODE(cbtimer_node)  = NULL_PTR;
    CBTIMER_NODE_TIMEOUT_FUNC_ADDR_NODE(cbtimer_node) = func_addr_node;

    handler = CBTIMER_NODE_TIMEOUT_HANDLER(cbtimer_node);

    handler->func_id       = timeout_func_id;
    handler->func_para_num = func_addr_node->func_para_num;
    handler->func_ret_val  = EC_TRUE;

    va_start(ap, timeout_func_id);
    for(para_idx = 0; para_idx < func_addr_node->func_para_num; para_idx ++ )
    {
        FUNC_PARA *func_para;

        func_para = &(handler->func_para[ para_idx ]);
        func_para->para_val = va_arg(ap, UINT32);
    }
    va_end(ap);

    CTIMET_GET(CBTIMER_NODE_START_TIME(cbtimer_node));
    CTIMET_GET(CBTIMER_NODE_LAST_TIME(cbtimer_node));

    clist_push_back(cbtimer_node_list, (void *)cbtimer_node);

    return (EC_TRUE);
}

EC_BOOL cbtimer_register(CLIST *cbtimer_node_list, CBTIMER_NODE *cbtimer_node)
{
    clist_push_back(cbtimer_node_list, (void *)cbtimer_node);
    return (EC_TRUE);
}

/**
*
* delete CBTIMER_NODE
*
**/
EC_BOOL cbtimer_unregister(CLIST *cbtimer_node_list, CBTIMER_NODE *cbtimer_node)
{
    if(NULL_PTR != CBTIMER_NODE_NAME(cbtimer_node))
    {
        sys_log(LOGSTDOUT, "cbtimer_unregister: unregister cbtimer %s\n", (char *)CBTIMER_NODE_NAME_STR(cbtimer_node));
    }

    clist_del(cbtimer_node_list, cbtimer_node, NULL_PTR);
    cbtimer_node_free(cbtimer_node);

    return (EC_TRUE);
}

CBTIMER_NODE *cbtimer_search_by_name(CLIST *cbtimer_node_list, const CSTRING *name)
{
    CLIST_DATA *clist_data;
    clist_data = clist_search_front(cbtimer_node_list, (void *)name, (CLIST_DATA_DATA_CMP)cbtimer_node_match_name);
    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }
    return ((CBTIMER_NODE *)CLIST_DATA_DATA(clist_data));
}

/**
*
* num of CBTIMER_NODE
*
**/
UINT32  cbtimer_num(CLIST *cbtimer_node_list)
{
    return clist_size(cbtimer_node_list);
}

EC_BOOL cbtimer_handle(CLIST *cbtimer_node_list)
{
    CLIST_DATA *clist_data;
    EC_BOOL handle_flag;

    handle_flag = EC_FALSE;

    CLIST_LOCK(cbtimer_node_list, LOC_CBTIMER_0006);
    CLIST_LOOP_NEXT(cbtimer_node_list, clist_data)
    {
        CBTIMER_NODE *cbtimer_node;
        CTIMET cur_time;

        cbtimer_node = (CBTIMER_NODE *)CLIST_DATA_DATA(clist_data);
        if(NULL_PTR == cbtimer_node)
        {
            CLIST_DATA *clist_data_rmv;

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);

            clist_rmv_no_lock(cbtimer_node_list, clist_data_rmv);
            cbtimer_node_free(cbtimer_node);
            continue;
        }

        CTIMET_GET(cur_time);

        /*when expire, remove it*/
        if(EC_TRUE == cbtimer_node_is_expire(cbtimer_node, cur_time))
        {
            CLIST_DATA *clist_data_rmv;

            sys_log(LOGSTDNULL, "[DEBUG] cbtimer_handle: cbtimer_node %s expired\n",
                                (char *)CBTIMER_NODE_NAME_STR(cbtimer_node));

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);

            clist_rmv_no_lock(cbtimer_node_list, clist_data_rmv);

            cbtimer_node_expire_handle(cbtimer_node);

            cbtimer_node_free(cbtimer_node);
            continue;
        }

        /*when not timeout, check next*/
        if(EC_FALSE == cbtimer_node_is_timeout(cbtimer_node, cur_time))
        {
            continue;
        }

        sys_log(LOGSTDNULL, "[DEBUG] cbtimer_handle: cbtimer_node %s  timeout\n",
                            (char *)CBTIMER_NODE_NAME_STR(cbtimer_node));

        handle_flag = EC_TRUE;/*handle one flag*/

        if(EC_FALSE == cbtimer_node_timeout_handle(cbtimer_node))
        {
            CLIST_DATA *clist_data_rmv;

            sys_log(LOGSTDOUT, "[DEBUG] cbtimer_handle: cbtimer_node %s timeout handler return EC_FALSE\n",
                                (char *)CBTIMER_NODE_NAME_STR(cbtimer_node));

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);

            clist_rmv_no_lock(cbtimer_node_list, clist_data_rmv);
            cbtimer_node_free(cbtimer_node);
            continue;
        }

        /*update timer*/
        CTIMET_GET(CBTIMER_NODE_LAST_TIME(cbtimer_node));

        sys_log(LOGSTDNULL, "[DEBUG] cbtimer_handle: cbtimer_node_handle %s timeout handler return EC_TRUE\n",
                            (char *)CBTIMER_NODE_NAME_STR(cbtimer_node));
    }
    CLIST_UNLOCK(cbtimer_node_list, LOC_CBTIMER_0007);
    return (handle_flag);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

