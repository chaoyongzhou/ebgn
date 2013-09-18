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

#include "type.h"
#include "mm.h"
#include "log.h"

#include "csocket.h"
#include "clist.h"

#include "cmpic.inc"
#include "task.h"
#include "cthread.h"
#include "cmpie.h"

#include "cextsrv.inc"
#include "cextsrv.h"

EC_BOOL cextclnt_init(CEXTCLNT *cextclnt)
{
    CEXTCLNT_SOCKFD(cextclnt) = CMPI_ERROR_SOCKFD;
    CEXTCLNT_IPADDR(cextclnt) = CMPI_ERROR_IPADDR;
    return (EC_TRUE);
}

EC_BOOL cextclnt_clean(CEXTCLNT *cextclnt)
{
    if(CMPI_ERROR_SOCKFD != CEXTCLNT_SOCKFD(cextclnt))
    {
        csocket_close(CEXTCLNT_SOCKFD(cextclnt));
        CEXTCLNT_SOCKFD(cextclnt) = CMPI_ERROR_SOCKFD;
    }
    CEXTCLNT_IPADDR(cextclnt) = CMPI_ERROR_IPADDR;
    return (EC_TRUE);
}

EC_BOOL cextclnt_recv(CEXTCLNT *cextclnt, UINT8 **in_buff, UINT32 *in_buff_size)
{
    UINT32 data_size;
    UINT8 *data_buff;

    if(EC_FALSE == csocket_recv_uint32(CEXTCLNT_SOCKFD(cextclnt), &data_size))
    {
        sys_log(LOGSTDOUT, "error:cextclnt_recv: recv data size from client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        return (EC_FALSE);
    }

    data_buff = (UINT8 *)SAFE_MALLOC(data_size, LOC_CEXTSRV_0001);
    if(NULL_PTR == data_buff)
    {
        sys_log(LOGSTDOUT, "error:cextclnt_recv: alloc %ld bytes failed\n", data_size);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv(CEXTCLNT_SOCKFD(cextclnt), data_buff, data_size))
    {
        sys_log(LOGSTDOUT, "error:cextclnt_recv: recv %ld bytes from client %s on sockfd %d failed\n",
                            data_size, CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        SAFE_FREE(data_buff, LOC_CEXTSRV_0002);
        return (EC_FALSE);
    }

    (*in_buff) = data_buff;
    (*in_buff_size) = data_size;

    return (EC_TRUE);
}

EC_BOOL cextclnt_send(CEXTCLNT *cextclnt, const UINT8 *out_buff, const UINT32 out_buff_size)
{
    if(EC_FALSE == csocket_send_uint32(CEXTCLNT_SOCKFD(cextclnt), out_buff_size))
    {
        sys_log(LOGSTDOUT, "error:cextclnt_send: send data size %ld to client %s on sockfd %d failed\n",
                            out_buff_size, CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_send(CEXTCLNT_SOCKFD(cextclnt), out_buff, out_buff_size))
    {
        sys_log(LOGSTDOUT, "error:cextclnt_send: send %ld bytes to client %s on sockfd %d failed\n",
                            out_buff_size, CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

CEXTSRV *cextsrv_new(const UINT32 srv_port, const int srv_sockfd)
{
    CEXTSRV *cextsrv;

    cextsrv = (CEXTSRV *)SAFE_MALLOC(sizeof(CEXTSRV), LOC_CEXTSRV_0003);
    if(NULL_PTR == cextsrv)
    {
        sys_log(LOGSTDOUT, "error:cextsrv_new: new cextsrv failed\n");
        return (NULL_PTR);
    }
    cextsrv_init(cextsrv, srv_port, srv_sockfd);
    return (cextsrv);
}

EC_BOOL cextsrv_init(CEXTSRV *cextsrv, const UINT32 srv_port, const int srv_sockfd)
{
    CEXTSRV_PORT(cextsrv)    = srv_port;
    CEXTSRV_SOCKFD(cextsrv)  = srv_sockfd;

    CEXTSRV_CMUTEX_INIT(cextsrv, LOC_CEXTSRV_0004);

    return (EC_TRUE);
}

EC_BOOL cextsrv_clean(CEXTSRV *cextsrv)
{
    if(CMPI_ERROR_SOCKFD != CEXTSRV_SOCKFD(cextsrv))
    {
        csocket_close(CEXTSRV_SOCKFD(cextsrv));
        CEXTSRV_SOCKFD(cextsrv)  = CMPI_ERROR_SOCKFD;
    }

    CEXTSRV_PORT(cextsrv)    = CMPI_ERROR_SRVPORT;

    CEXTSRV_CMUTEX_CLEAN(cextsrv, LOC_CEXTSRV_0005);

    return (EC_TRUE);
}

EC_BOOL cextsrv_free(CEXTSRV *cextsrv)
{
    if(NULL_PTR != cextsrv)
    {
        cextsrv_clean(cextsrv);
        SAFE_FREE(cextsrv, LOC_CEXTSRV_0006);
    }
    return (EC_TRUE);
}

CEXTSRV * cextsrv_start(const UINT32 srv_port)
{
    CEXTSRV *cextsrv;
    int srv_sockfd;

    if(EC_FALSE == csocket_srv_start(srv_port, CSOCKET_IS_BLOCK_MODE, &srv_sockfd))
    {
        sys_log(LOGSTDERR, "error:cextsrv_start: failed to listen on port %ld\n", srv_port);
        return (NULL_PTR);
    }

    cextsrv = cextsrv_new(srv_port, srv_sockfd);
    if(NULL_PTR == cextsrv)
    {
        sys_log(LOGSTDOUT, "error:cextsrv_start: new cextsrv failed, close srv sockfd %d\n", srv_sockfd);
        csocket_close(srv_sockfd);
        return (NULL_PTR);
    }

    sys_log(LOGSTDOUT, "cextsrv_start: start srv sockfd %d on port %ld\n", srv_sockfd, srv_port);
    return (cextsrv);
}

EC_BOOL cextsrv_end(CEXTSRV *cextsrv)
{
    return cextsrv_free(cextsrv);
}

EC_BOOL cextsrv_req_encode(CEXTSRV *cextsrv, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len, TASK_FUNC *task_req_func)
{
    FUNC_ADDR_NODE *func_addr_node;

    UINT32 send_comm;

    UINT32  position;

    send_comm = CMPI_ANY_COMM;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_req_encode: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    cmpi_encode_uint32(send_comm, (task_req_func->func_id), out_buff, out_buff_max_len, &(position));
    cmpi_encode_uint32(send_comm, (task_req_func->func_para_num), out_buff, out_buff_max_len, &(position));

    task_req_func_para_encode(send_comm, task_req_func->func_para_num, (FUNC_PARA *)task_req_func->func_para,
                                         func_addr_node, out_buff, out_buff_max_len, &(position));
    (*out_buff_len) = position;/*set to real length*/

    return (EC_TRUE);
}

EC_BOOL cextsrv_req_encode_size(CEXTSRV *cextsrv, const TASK_FUNC *task_req_func, UINT32 *size)
{
    FUNC_ADDR_NODE *func_addr_node;

    UINT32 send_comm;

    /*clear size*/
    *size = 0;

    send_comm = CMPI_ANY_COMM;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_req_encode_size: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    cmpi_encode_uint32_size(send_comm, (task_req_func->func_id), size);
    cmpi_encode_uint32_size(send_comm, (task_req_func->func_para_num), size);

    task_req_func_para_encode_size(send_comm, func_addr_node->func_para_num, (FUNC_PARA *)task_req_func->func_para, func_addr_node, size);

    return (EC_TRUE);
}

EC_BOOL cextsrv_req_decode(CEXTSRV *cextsrv, const UINT8 *in_buff, const UINT32 in_buff_len, TASK_FUNC *task_req_func)
{
    UINT32 recv_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 position;

    recv_comm = CMPI_ANY_COMM;

    position = 0;

    cmpi_decode_uint32(recv_comm, in_buff, in_buff_len, &(position), &(task_req_func->func_id));
    cmpi_decode_uint32(recv_comm, in_buff, in_buff_len, &(position), &(task_req_func->func_para_num));

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_req_decode: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
    if( NULL_PTR == type_conv_item )
    {
        sys_log(LOGSTDOUT,"error:cextsrv_req_decode: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
        return (EC_FALSE);
    }
    if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item))
    {
        alloc_static_mem(MD_TASK, 0, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void **)&(task_req_func->func_ret_val), LOC_CEXTSRV_0007);
        dbg_tiny_caller(2, TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item), CMPI_ANY_MODI, task_req_func->func_ret_val);
    }

    if(EC_FALSE == task_req_func_para_decode(recv_comm, in_buff, in_buff_len, &(position),
                                              &(task_req_func->func_para_num), (FUNC_PARA *)task_req_func->func_para,
                                              func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_req_decode: decode func paras failed\n");

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_req_func->func_ret_val)
        {
            free_static_mem(MD_TASK, 0, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_req_func->func_ret_val), LOC_CEXTSRV_0008);
        }
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cextsrv_rsp_encode(CEXTSRV *cextsrv, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len, TASK_FUNC *task_rsp_func)
{
    UINT32 send_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 position;

    send_comm = CMPI_ANY_COMM;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_encode: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:cextsrv_rsp_encode: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        dbg_tiny_caller(5,
            TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item),
            send_comm,
            task_rsp_func->func_ret_val,
            out_buff,
            out_buff_max_len,
            &(position));

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_rsp_func->func_ret_val)
        {
            dbg_tiny_caller(2, TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item), CMPI_ANY_MODI, task_rsp_func->func_ret_val);/*WARNING: SHOULD NOT BE 0*/
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)task_rsp_func->func_ret_val, LOC_CEXTSRV_0009);/*clean up*/
            task_rsp_func->func_ret_val = 0;
        }
    }

    task_rsp_func_para_encode(send_comm, task_rsp_func->func_para_num, (FUNC_PARA *)task_rsp_func->func_para, func_addr_node, out_buff, out_buff_max_len, &(position));

    (*out_buff_len) = position;/*set to real length*/
    return (EC_TRUE);
}

EC_BOOL cextsrv_rsp_encode_size(CEXTSRV *cextsrv, const TASK_FUNC *task_rsp_func, UINT32 *size)
{
    UINT32 send_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    send_comm     = CMPI_ANY_COMM;

    *size = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_encode_size: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:cextsrv_rsp_encode_size: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }
        dbg_tiny_caller(3,
            TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item),
            send_comm,
            task_rsp_func->func_ret_val,
            size);
    }

    task_rsp_func_para_encode_size(send_comm, task_rsp_func->func_para_num, (FUNC_PARA *)task_rsp_func->func_para, func_addr_node, size);

    return (EC_TRUE);
}

EC_BOOL cextsrv_rsp_decode(CEXTSRV *cextsrv, const UINT8 *in_buff, const UINT32 in_buff_len, TASK_FUNC *task_req_func)
{
    UINT32 recv_comm;

    FUNC_ADDR_NODE *func_addr_node;

    TYPE_CONV_ITEM *type_conv_item;

    UINT32  position;

    recv_comm = CMPI_ANY_COMM;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_decode: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        if(0 == task_req_func->func_ret_val)
        {
            sys_log(LOGSTDOUT, "error:cextsrv_rsp_decode: func id %lx func_ret_val should not be null\n", task_req_func->func_id);
            exit(0);/*coding bug, user should fix it*/
        }

        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:cextsrv_rsp_decode: ret type %ld conv item is not defined\n",
                    func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        dbg_tiny_caller(5,
                TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item),
                recv_comm,
                in_buff,
                in_buff_len,
                &(position),
                task_req_func->func_ret_val);
    }

    if(EC_FALSE == task_rsp_func_para_decode(recv_comm, in_buff, in_buff_len, &(position),
                              task_req_func->func_para_num,
                              (FUNC_PARA *)task_req_func->func_para,
                              func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_decode: decode rsp func paras failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL cextsrv_req_clean(TASK_FUNC *task_req_func)
{
    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_req_clean: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:cextsrv_req_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_req_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_req_func->func_ret_val), LOC_CEXTSRV_0010);
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
            sys_log(LOGSTDOUT,"error:cextsrv_req_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CEXTSRV_0011);
            func_para->para_val = 0;
        }
    }
    return (EC_TRUE);
}

EC_BOOL cextsrv_rsp_clean(TASK_FUNC *task_rsp_func)
{
    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 para_idx;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_clean: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:cextsrv_rsp_clean: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_rsp_func->func_ret_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_rsp_func->func_ret_val), LOC_CEXTSRV_0012);
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
            sys_log(LOGSTDOUT,"error:cextsrv_rsp_clean: para %ld type %ld conv item is not defined\n",
                                para_idx, func_addr_node->func_para_type[ para_idx ]);
            return (EC_FALSE);
        }
        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != func_para->para_val)
        {
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(func_para->para_val), LOC_CEXTSRV_0013);
            func_para->para_val = 0;
        }
    }
    return (EC_TRUE);
}

EC_BOOL cextsrv_process0(CEXTSRV *cextsrv, CEXTCLNT *cextclnt)
{
    UINT8  *in_buff;
    UINT32  in_buff_len;
    UINT8  *out_buff;
    UINT32  out_buff_len;

    FUNC_ADDR_NODE *func_addr_node;

    TASK_FUNC task_req_func;
    TASK_FUNC task_rsp_func;

    UINT32 para_idx;

    if(EC_FALSE == cextclnt_recv(cextclnt, &in_buff, &in_buff_len))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: recv data from client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        return (EC_FALSE);
    }

    task_func_init(&task_req_func);
    task_func_init(&task_rsp_func);

    if(EC_FALSE == cextsrv_req_decode(cextsrv, in_buff, in_buff_len, &task_req_func))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: decode req from client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        SAFE_FREE(in_buff, LOC_CEXTSRV_0014);
        cextsrv_req_clean(&task_req_func);
        return (EC_FALSE);
    }
    SAFE_FREE(in_buff, LOC_CEXTSRV_0015);

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func.func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: failed to fetch func addr node by func id %lx\n", task_req_func.func_id);
        cextsrv_req_clean(&task_req_func);
        return (EC_FALSE);
    }

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

    if(EC_FALSE == cextsrv_rsp_encode_size(cextsrv, &task_rsp_func, &out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: encode size rsp to client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        cextsrv_req_clean(&task_req_func);
        cextsrv_rsp_clean(&task_rsp_func);
        return (EC_FALSE);
    }

    out_buff = (UINT8 *)SAFE_MALLOC(out_buff_len, LOC_CEXTSRV_0016);
    if(NULL_PTR == out_buff)
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: alloc %ld bytes for out buff to client %s on sockfd %d failed\n",
                            out_buff_len, CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        cextsrv_req_clean(&task_req_func);
        cextsrv_rsp_clean(&task_rsp_func);
        return (EC_FALSE);
    }

    if(EC_FALSE == cextsrv_rsp_encode(cextsrv, out_buff, out_buff_len, &out_buff_len, &task_rsp_func))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: encode rsp to client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        cextsrv_req_clean(&task_req_func);
        cextsrv_rsp_clean(&task_rsp_func);
        SAFE_FREE(out_buff, LOC_CEXTSRV_0017);
        return (EC_FALSE);
    }

    if(EC_FALSE == cextclnt_send(cextclnt, out_buff, out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: send rsp to client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        cextsrv_req_clean(&task_req_func);
        cextsrv_rsp_clean(&task_rsp_func);
        SAFE_FREE(out_buff, LOC_CEXTSRV_0018);
        return (EC_FALSE);
    }
    cextsrv_req_clean(&task_req_func);
    cextsrv_rsp_clean(&task_rsp_func);
    SAFE_FREE(out_buff, LOC_CEXTSRV_0019);

    return (EC_TRUE);
}

EC_BOOL cextsrv_process(CEXTSRV *cextsrv, CEXTCLNT *cextclnt)
{
    FUNC_ADDR_NODE *func_addr_node;

    TASK_FUNC task_req_func;
    TASK_FUNC task_rsp_func;

    UINT32 para_idx;

    task_func_init(&task_req_func);
    if(EC_FALSE == csocket_task_req_func_recv(CEXTCLNT_SOCKFD(cextclnt), &task_req_func))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: recv req from client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));
        return (EC_FALSE);
    }

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func.func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: failed to fetch func addr node by func id %lx\n", task_req_func.func_id);
        cextsrv_req_clean(&task_req_func);
        return (EC_FALSE);
    }

    task_func_init(&task_rsp_func);

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

    if(EC_FALSE == csocket_task_rsp_func_send(CEXTCLNT_SOCKFD(cextclnt), &task_rsp_func))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_process: send rsp to client %s on sockfd %d failed\n",
                            CEXTCLNT_IPADDR_STR(cextclnt), CEXTCLNT_SOCKFD(cextclnt));

        cextsrv_req_clean(&task_req_func);
        cextsrv_rsp_clean(&task_rsp_func);
        return (EC_FALSE);
    }

    cextsrv_req_clean(&task_req_func);
    cextsrv_rsp_clean(&task_rsp_func);

    return (EC_TRUE);
}

/*when thread accept one client connection, then process it and close the connection when reaching end*/
EC_BOOL cextsrv_thread(CEXTSRV *cextsrv)
{
    for(;;)
    {
        CEXTCLNT cextclnt;

        int     cextclnt_sockfd;
        UINT32  cextclnt_ipaddr;

        cextclnt_init(&cextclnt);

        sys_log(LOGSTDOUT, "[DEBUG] cextsrv_thread: accept blocking...\n");

        CEXTSRV_CMUTEX_LOCK(cextsrv, LOC_CEXTSRV_0020);
        if(EC_FALSE == csocket_accept(CEXTSRV_SOCKFD(cextsrv), &cextclnt_sockfd, &cextclnt_ipaddr))
        {
            CEXTSRV_CMUTEX_UNLOCK(cextsrv, LOC_CEXTSRV_0021);
            sys_log(LOGSTDOUT, "error:cextsrv_thread: accept on sockfd %d failed\n", CEXTSRV_SOCKFD(cextsrv));
            continue;
        }
        CEXTSRV_CMUTEX_UNLOCK(cextsrv, LOC_CEXTSRV_0022);
        sys_log(LOGSTDOUT, "[DEBUG] cextsrv_thread: accept successfully\n");

        CEXTCLNT_SOCKFD(&cextclnt) = cextclnt_sockfd;
        CEXTCLNT_IPADDR(&cextclnt) = cextclnt_ipaddr;

        sys_log(LOGSTDOUT, "[DEBUG] cextsrv_thread: accept connection from client %s on sockfd %d\n",
                            uint32_to_ipv4(CEXTCLNT_IPADDR(&cextclnt)), CEXTCLNT_SOCKFD(&cextclnt));

        if(EC_FALSE == cextsrv_process(cextsrv, &cextclnt))
        {
            sys_log(LOGSTDOUT, "error:cextsrv_thread: process request from client %s on client sockfd %d failed\n",
                                uint32_to_ipv4(CEXTCLNT_IPADDR(&cextclnt)), CEXTCLNT_SOCKFD(&cextclnt));
            cextclnt_clean(&cextclnt);
            continue;
        }

        //cextclnt_clean(&cextclnt);/*each client sockfd connecting and processing only once time!*/
    }
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

