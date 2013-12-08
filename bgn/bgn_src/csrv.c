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

#include "type.h"
#include "mm.h"
#include "log.h"

#include "csocket.h"
#include "clist.h"

#include "cmpic.inc"
#include "task.h"
#include "cthread.h"

#include "csrv.h"

CSRV *csrv_new(const UINT32 srv_port, const int srv_sockfd)
{
    CSRV *csrv;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CSRV, &csrv, LOC_CSRV_0001);
    if(NULL_PTR == csrv)
    {
        sys_log(LOGSTDOUT, "error:csrv_new: new csrv failed\n");
        return (NULL_PTR);
    }
    csrv_init(csrv, srv_port, srv_sockfd);
    return (csrv);
}

EC_BOOL csrv_init(CSRV *csrv, const UINT32 srv_port, const int srv_sockfd)
{
    CSRV_PORT(csrv)    = srv_port;
    CSRV_SOCKFD(csrv)  = srv_sockfd;
    CSRV_MD_ID(csrv)   = ERR_MODULE_ID;

    CSRV_ADD_CSOCKET_CNODE(csrv) = NULL_PTR;
    CSRV_DEL_CSOCKET_CNODE(csrv) = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL csrv_clean(CSRV *csrv)
{
    if(CMPI_ERROR_SOCKFD != CSRV_SOCKFD(csrv))
    {
        csocket_close(CSRV_SOCKFD(csrv));
        CSRV_SOCKFD(csrv)  = CMPI_ERROR_SOCKFD;
    }

    CSRV_PORT(csrv)    = CMPI_ERROR_SRVPORT;
    CSRV_MD_ID(csrv)   = ERR_MODULE_ID;

    CSRV_ADD_CSOCKET_CNODE(csrv) = NULL_PTR;
    CSRV_DEL_CSOCKET_CNODE(csrv) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL csrv_free(CSRV *csrv)
{
    if(NULL_PTR != csrv)
    {
        csrv_clean(csrv);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CSRV, csrv, LOC_CSRV_0002);
    }
    return (EC_TRUE);
}

CSRV * csrv_start(const UINT32 srv_port, const UINT32 md_id, CSRV_ADD_CSOCKET_CNODE add_csocket_cnode, CSRV_DEL_CSOCKET_CNODE del_csocket_cnode)
{
    CSRV *csrv;
    int srv_sockfd;

    if(ERR_MODULE_ID == md_id)
    {
        sys_log(LOGSTDOUT, "error:csrv_start: md id is invalid\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == csocket_listen(srv_port, &srv_sockfd))
    {
        sys_log(LOGSTDERR, "error:csrv_start: failed to listen on port %ld\n", srv_port);
        return (NULL_PTR);
    }

    csrv = csrv_new(srv_port, srv_sockfd);
    if(NULL_PTR == csrv)
    {
        sys_log(LOGSTDOUT, "error:csrv_start: new csrv failed, close srv sockfd %d\n", srv_sockfd);
        csocket_close(srv_sockfd);
        return (NULL_PTR);
    }

    CSRV_MD_ID(csrv)   = md_id;
    CSRV_ADD_CSOCKET_CNODE(csrv) = add_csocket_cnode;
    CSRV_DEL_CSOCKET_CNODE(csrv) = del_csocket_cnode;

    sys_log(LOGSTDOUT, "csrv_start: start srv sockfd %d on port %ld\n", srv_sockfd, srv_port);
    return (csrv);
}

EC_BOOL csrv_end(CSRV *csrv)
{
    return csrv_free(csrv);
}

EC_BOOL csrv_accept(CSRV *csrv)
{
    UINT32 client_ipaddr;

    int client_conn_sockfd;
    

    if(EC_TRUE == csocket_accept(CSRV_SOCKFD(csrv), &(client_conn_sockfd), &(client_ipaddr) ))
    {
        CSOCKET_CNODE *csocket_cnode;
        TASK_BRD *task_brd;
        CROUTINE_NODE  *croutine_node;

        sys_log(LOGSTDOUT, "csrv_accept: handle new sockfd %d\n", client_conn_sockfd);

        csocket_cnode = csocket_cnode_new(CMPI_ERROR_TCID, client_conn_sockfd, client_ipaddr, CMPI_ERROR_SRVPORT);/*here do not know the remote client srv port*/
        if(NULL_PTR == csocket_cnode)
        {
            sys_log(LOGSTDOUT, "error:csrv_accept:failed to alloc csocket cnode for sockfd %d, hence close it\n", client_conn_sockfd);
            csocket_close(client_conn_sockfd);
            return (EC_FALSE);
        }

        if(NULL_PTR != CSRV_ADD_CSOCKET_CNODE(csrv))
        {
            CSRV_ADD_CSOCKET_CNODE(csrv)(CSRV_MD_ID(csrv), csocket_cnode);
        }

        task_brd = task_brd_default_get();

        croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), (UINT32)csrv_handle, 2, csrv, csocket_cnode);
        if(NULL_PTR == croutine_node)
        {
            sys_log(LOGSTDOUT, "error:csrv_accept: cthread load failed\n");
            csocket_cnode_close(csocket_cnode);
            return (EC_FALSE);
        }
        CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CSRV_0003);
    }
    return (EC_TRUE);
}

EC_BOOL csrv_select(CSRV *csrv, int *ret)
{
    FD_CSET fd_cset;
    int max_sockfd;

    struct timeval tv;

    tv.tv_sec  = 0;
    tv.tv_usec = /*1*/0;

    max_sockfd = 0;
    csocket_fd_clean(&fd_cset);
    csocket_fd_set(CSRV_SOCKFD(csrv), &fd_cset, &max_sockfd);
    return csocket_select(max_sockfd + 1, &fd_cset, NULL_PTR, NULL_PTR, &tv, ret);
}

EC_BOOL csrv_req_clean(struct _TASK_FUNC *task_req_func)
{
    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:csrv_req_clean: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:csrv_req_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_req_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_req_func->func_ret_val), LOC_CSRV_0004);
            task_req_func->func_ret_val = 0;
        }
    }

    for( para_idx = 0; para_idx < task_req_func->func_para_num; para_idx ++ )
    {
        FUNC_PARA *func_para;

        func_para = &(task_req_func->func_para[ para_idx ]);

        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_para_type[ para_idx ]);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:csrv_req_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CSRV_0005);
            func_para->para_val = 0;
        }
    }
    return (EC_TRUE);
}

EC_BOOL csrv_rsp_clean(struct _TASK_FUNC *task_rsp_func)
{
    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:csrv_rsp_clean: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:csrv_rsp_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_rsp_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_rsp_func->func_ret_val), LOC_CSRV_0006);
            task_rsp_func->func_ret_val = 0;
        }
    }

    for( para_idx = 0; para_idx < task_rsp_func->func_para_num; para_idx ++ )
    {
        FUNC_PARA *func_para;

        func_para = &(task_rsp_func->func_para[ para_idx ]);

        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_para_type[ para_idx ]);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:csrv_rsp_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CSRV_0007);
            func_para->para_val = 0;
        }
    }

    return (EC_TRUE);
}

EC_BOOL csrv_process(CSRV *csrv, struct _CSOCKET_CNODE *csocket_cnode)
{
    FUNC_ADDR_NODE *func_addr_node;

    TASK_FUNC task_req_func;
    TASK_FUNC task_rsp_func;

    UINT32 para_idx;

    task_func_init(&task_req_func);

    /*recv req*/
    if(EC_FALSE == csocket_task_req_func_recv(CSOCKET_CNODE_SOCKFD(csocket_cnode), &task_req_func))
    {
        sys_log(LOGSTDOUT, "error:csrv_process: recv req from client %s on sockfd %d failed\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func.func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:csrv_process: failed to fetch func addr node by func id %lx\n", task_req_func.func_id);
        csrv_req_clean(&task_req_func);
        return (EC_FALSE);
    }

    task_func_init(&task_rsp_func);

    /*handle req*/
    task_caller(&task_req_func, func_addr_node);

    /*clone task_req_func to task_rsp_func and clean up task_req_func*/
    task_rsp_func.func_id       = task_req_func.func_id;
    task_rsp_func.func_para_num = task_req_func.func_para_num;
    task_rsp_func.func_ret_val  = task_req_func.func_ret_val;

    task_req_func.func_ret_val = 0;/*clean it*/

    for( para_idx = 0; para_idx < task_req_func.func_para_num; para_idx ++ )
    {
        FUNC_PARA  *task_rsp_func_para;
        FUNC_PARA  *task_req_func_para;

        task_req_func_para = &(task_req_func.func_para[ para_idx ]);
        task_rsp_func_para = &(task_rsp_func.func_para[ para_idx ]);

        task_rsp_func_para->para_dir = task_req_func_para->para_dir;
        task_rsp_func_para->para_val = task_req_func_para->para_val;
        task_req_func_para->para_val = 0;/*clean it*/
    }

    /*send rsp*/
    if(EC_FALSE == csocket_task_rsp_func_send(CSOCKET_CNODE_SOCKFD(csocket_cnode), &task_rsp_func))
    {
        sys_log(LOGSTDOUT, "error:csrv_process: send rsp to client %s on sockfd %d failed\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SOCKFD(csocket_cnode));

        csrv_req_clean(&task_req_func);
        csrv_rsp_clean(&task_rsp_func);
        return (EC_FALSE);
    }

    csrv_req_clean(&task_req_func);
    csrv_rsp_clean(&task_rsp_func);

    return (EC_TRUE);
}

EC_BOOL csrv_handle(CSRV *csrv, CSOCKET_CNODE *csocket_cnode)
{
    for(;;)
    {
        if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            csocket_cnode_close(csocket_cnode);
            if(NULL_PTR != CSRV_DEL_CSOCKET_CNODE(csrv))
            {
                CSRV_DEL_CSOCKET_CNODE(csrv)(CSRV_MD_ID(csrv), csocket_cnode);
            }
            break;
        }
        sys_log(LOGSTDOUT, "[DEBUG] csrv_handle: CSOCKET_CNODE_SOCKFD %d is connected\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));

        if(EC_FALSE == csrv_process(csrv, csocket_cnode))
        {
            sys_log(LOGSTDOUT, "error:csrv_handle: process failed on sockfd %d where md id %ld, close it\n",
                                CSOCKET_CNODE_SOCKFD(csocket_cnode), CSRV_MD_ID(csrv));
            csocket_cnode_close(csocket_cnode);
            if(NULL_PTR != CSRV_DEL_CSOCKET_CNODE(csrv))
            {
                CSRV_DEL_CSOCKET_CNODE(csrv)(CSRV_MD_ID(csrv), csocket_cnode);
            }
            break;
        }
    }
    return (EC_FALSE);
}

EC_BOOL csrv_do_once(CSRV *csrv)
{
    int ret;
    if(EC_FALSE == csrv_select(csrv, &ret))
    {
        sys_log(LOGSTDERR, "error:csrv_do_once: select failed\n");

        return (EC_FALSE);
    }

    if( 0 < ret )
    {
        TASK_BRD_DEFAULT_ACTIVE_COUNTER_INC(LOC_CSRV_0008);
        csrv_accept(csrv);
    }
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

