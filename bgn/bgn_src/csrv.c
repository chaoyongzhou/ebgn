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

CSRV *csrv_new(const UINT32 srv_ipaddr, const UINT32 srv_port, const int srv_sockfd)
{
    CSRV *csrv;
    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CSRV, &csrv, LOC_CSRV_0001);
    if(NULL_PTR == csrv)
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_new: new csrv failed\n");
        return (NULL_PTR);
    }
    csrv_init(csrv, srv_ipaddr, srv_port, srv_sockfd);
    return (csrv);
}

EC_BOOL csrv_init(CSRV *csrv, const UINT32 srv_ipaddr, const UINT32 srv_port, const int srv_sockfd)
{
    CSRV_IPADDR(csrv)  = srv_ipaddr;
    CSRV_PORT(csrv)    = srv_port;
    CSRV_SOCKFD(csrv)  = srv_sockfd;
    CSRV_MD_ID(csrv)   = ERR_MODULE_ID;

    CSRV_ADD_CSOCKET_CNODE(csrv)    = NULL_PTR;
    CSRV_DEL_CSOCKET_CNODE(csrv)    = NULL_PTR;
    CSRV_RD_HANDLER(csrv)           = NULL_PTR;
    CSRV_WR_HANDLER(csrv)           = NULL_PTR;
    CSRV_OT_HANDLER(csrv)           = NULL_PTR;
    CSRV_CL_HANDLER(csrv)           = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL csrv_clean(CSRV *csrv)
{
    if(CMPI_ERROR_SOCKFD != CSRV_SOCKFD(csrv))
    {
        csocket_close(CSRV_SOCKFD(csrv));
        CSRV_SOCKFD(csrv)  = CMPI_ERROR_SOCKFD;
    }

    CSRV_IPADDR(csrv)  = CMPI_ERROR_IPADDR;
    CSRV_PORT(csrv)    = CMPI_ERROR_SRVPORT;
    CSRV_MD_ID(csrv)   = ERR_MODULE_ID;

    CSRV_ADD_CSOCKET_CNODE(csrv)    = NULL_PTR;
    CSRV_DEL_CSOCKET_CNODE(csrv)    = NULL_PTR;
    CSRV_RD_HANDLER(csrv)           = NULL_PTR;
    CSRV_WR_HANDLER(csrv)           = NULL_PTR;
    CSRV_OT_HANDLER(csrv)           = NULL_PTR;
    CSRV_CL_HANDLER(csrv)           = NULL_PTR;
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

CSRV * csrv_start(const UINT32 srv_ipaddr, const UINT32 srv_port, const UINT32 md_id, 
                     const uint32_t timeout_nsec,
                     CSRV_ADD_CSOCKET_CNODE    add_csocket_cnode, 
                     CSRV_DEL_CSOCKET_CNODE    del_csocket_cnode,
                     CSRV_READ_HANDLER         rd_handler,
                     CSRV_WRITE_HANDLER        wr_handler,
                     CSRV_TIMEOUT_HANDLER      timeout_handler,
                     CSRV_CLOSE_HANDLER        close_handler
                     )
{
    CSRV *csrv;
    int srv_sockfd;

    if(ERR_MODULE_ID == md_id)
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_start: md id is invalid\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == csocket_listen(srv_ipaddr, srv_port, &srv_sockfd))
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDERR, "error:csrv_start: failed to listen on port %s:%ld\n", 
                            c_word_to_ipv4(srv_ipaddr), srv_port);
        return (NULL_PTR);
    }

    csrv = csrv_new(srv_ipaddr, srv_port, srv_sockfd);
    if(NULL_PTR == csrv)
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_start: new csrv failed, close srv sockfd %d\n", srv_sockfd);
        csocket_close(srv_sockfd);
        return (NULL_PTR);
    }

    CSRV_MD_ID(csrv)                = md_id;
    CSRV_TIMEOUT_NSEC(csrv)         = timeout_nsec;
    CSRV_ADD_CSOCKET_CNODE(csrv)    = add_csocket_cnode;
    CSRV_DEL_CSOCKET_CNODE(csrv)    = del_csocket_cnode;
    CSRV_RD_HANDLER(csrv)           = rd_handler;
    CSRV_WR_HANDLER(csrv)           = wr_handler;      
    CSRV_OT_HANDLER(csrv)           = timeout_handler;      
    CSRV_CL_HANDLER(csrv)           = close_handler;

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
    cepoll_set_event(task_brd_default_get_cepoll(), 
                      CSRV_SOCKFD(csrv), 
                      CEPOLL_RD_EVENT,
                      (CEPOLL_EVENT_HANDLER)csrv_accept, 
                      (void *)csrv);
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/    

    dbg_log(SEC_0112_CSRV, 5)(LOGSTDOUT, "csrv_start: start srv sockfd %d on port %s:%ld\n", 
                       srv_sockfd, c_word_to_ipv4(srv_ipaddr), srv_port);
    return (csrv);
}

EC_BOOL csrv_end(CSRV *csrv)
{
    return csrv_free(csrv);
}

EC_BOOL csrv_accept_once(CSRV *csrv, EC_BOOL *continue_flag)
{
    UINT32  client_ipaddr;    
    EC_BOOL ret;
    int     client_conn_sockfd;    

    ret = csocket_accept(CSRV_SOCKFD(csrv), &(client_conn_sockfd), CSOCKET_IS_NONBLOCK_MODE, &(client_ipaddr));

    if(EC_TRUE == ret)
    {
        CSOCKET_CNODE *csocket_cnode;
        
        dbg_log(SEC_0112_CSRV, 5)(LOGSTDOUT, "csrv_accept_once: handle new sockfd %d\n", client_conn_sockfd);

        csocket_cnode = csocket_cnode_new(CMPI_ERROR_TCID, client_conn_sockfd, client_ipaddr, CMPI_ERROR_SRVPORT);/*here do not know the remote client srv port*/
        if(NULL_PTR == csocket_cnode)
        {
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_accept_once:failed to alloc csocket cnode for sockfd %d, hence close it\n", client_conn_sockfd);
            csocket_close(client_conn_sockfd);
            return (EC_FALSE);
        }

        if(NULL_PTR != CSRV_ADD_CSOCKET_CNODE(csrv))
        {
            CSRV_ADD_CSOCKET_CNODE(csrv)(CSRV_MD_ID(csrv), csocket_cnode);
        }
        
#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
        if(1)
        {
            TASK_BRD      *task_brd;            
            CROUTINE_NODE *croutine_node;

            task_brd = task_brd_default_get();
            
            croutine_node = croutine_pool_load(TASK_REQ_CTHREAD_POOL(task_brd), (UINT32)csrv_handle, 2, csrv, csocket_cnode);
            if(NULL_PTR == croutine_node)
            {
                dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_accept_once: cthread load failed\n");
                csocket_cnode_close(csocket_cnode);
                return (EC_FALSE);
            }
            CROUTINE_NODE_COND_RELEASE(croutine_node, LOC_CSRV_0003);
        }
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/        

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
        /*CSOCKET_CNODE_PKT_HDR will be used for specific purpose*/
        BSET(CSOCKET_CNODE_PKT_HDR(csocket_cnode), 0, CSOCKET_CNODE_PKT_HDR_SIZE);

        CSOCKET_CNODE_MODI(csocket_cnode) = CSRV_MD_ID(csrv);
        
        cepoll_set_event(task_brd_default_get_cepoll(), 
                          CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                          CEPOLL_RD_EVENT,
                          (CEPOLL_EVENT_HANDLER)CSRV_RD_HANDLER(csrv), 
                          (void *)csocket_cnode);

       cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           (CEPOLL_EVENT_HANDLER)CSRV_CL_HANDLER(csrv),
                           (void *)csocket_cnode);

       cepoll_set_timeout(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           CSRV_TIMEOUT_NSEC(csrv),
                           (CEPOLL_EVENT_HANDLER)CSRV_OT_HANDLER(csrv),
                           (void *)csocket_cnode);
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/
    }

    (*continue_flag) = ret;
    
    return (EC_TRUE);
}

EC_BOOL csrv_accept(CSRV *csrv)
{
    UINT32   idx;
    UINT32   num;
    EC_BOOL  continue_flag;

    num = CSRV_ACCEPT_MAX_NUM;
    for(idx = 0; idx < num; idx ++)
    {
        if(EC_FALSE == csrv_accept_once(csrv, &continue_flag))
        {
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_accept: accept No. %ld client failed where expect %ld clients\n", idx, num);
            return (EC_FALSE);
        }

        if(EC_FALSE == continue_flag)
        {
            dbg_log(SEC_0112_CSRV, 9)(LOGSTDOUT, "[DEBUG] csrv_accept: accept No. %ld client terminate where expect %ld clients\n", idx, num);
            break;
        }        
    }

    return (EC_TRUE);
}

EC_BOOL csrv_select(CSRV *csrv, int *ret)
{
    FD_CSET *fd_cset;
    int max_sockfd;

    struct timeval tv;

    tv.tv_sec  = 0;
    tv.tv_usec = /*1*/0;

    max_sockfd = 0;

    fd_cset = safe_malloc(sizeof(FD_CSET), LOC_CSRV_0004);
    if(NULL_PTR == fd_cset)
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_select: malloc FD_CSET with size %d failed\n", sizeof(FD_CSET));
        return (EC_FALSE);
    }
    
    csocket_fd_clean(fd_cset);
    csocket_fd_set(CSRV_SOCKFD(csrv), fd_cset, &max_sockfd);
    if(EC_FALSE == csocket_select(max_sockfd + 1, fd_cset, NULL_PTR, NULL_PTR, &tv, ret))
    {
        safe_free(fd_cset, LOC_CSRV_0005);
        return (EC_FALSE);
    }
    
    safe_free(fd_cset, LOC_CSRV_0006);
    return (EC_TRUE);
}

EC_BOOL csrv_req_clean(struct _TASK_FUNC *task_req_func)
{
    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_req_clean: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT,"error:csrv_req_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_req_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_req_func->func_ret_val), LOC_CSRV_0007);
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
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT,"error:csrv_req_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CSRV_0008);
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
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_rsp_clean: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT,"error:csrv_rsp_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_rsp_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_rsp_func->func_ret_val), LOC_CSRV_0009);
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
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT,"error:csrv_rsp_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CSRV_0010);
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
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_process: recv req from client %s on sockfd %d failed\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func.func_id, &func_addr_node))
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_process: failed to fetch func addr node by func id %lx\n", task_req_func.func_id);
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
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_process: send rsp to client %s on sockfd %d failed\n",
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
        dbg_log(SEC_0112_CSRV, 9)(LOGSTDOUT, "[DEBUG] csrv_handle: CSOCKET_CNODE_SOCKFD %d is connected\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));

        if(EC_FALSE == csrv_process(csrv, csocket_cnode))
        {
            dbg_log(SEC_0112_CSRV, 0)(LOGSTDOUT, "error:csrv_handle: process failed on sockfd %d where md id %ld, close it\n",
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
    int      ret;
    EC_BOOL  continue_flag;
    if(EC_FALSE == csrv_select(csrv, &ret))
    {
        dbg_log(SEC_0112_CSRV, 0)(LOGSTDERR, "error:csrv_do_once: select failed\n");

        return (EC_FALSE);
    }

    if( 0 < ret )
    {
        csrv_accept_once(csrv, &continue_flag);
    }
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

