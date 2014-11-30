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
#include <string.h>
#include <math.h>

#include "type.h"

#include "log.h"

#include "clist.h"
#include "cstring.h"
#include "cset.h"
#include "cmisc.h"
#include "taskcfg.h"
#include "csocket.h"

#include "cmpic.inc"
#include "cmpie.h"
#include "crbuff.h"
#include "tasks.h"
#include "task.h"
#include "crouter.h"
#include "cbase64code.h"
#include "cepoll.h"

#include "findex.inc"

#if 0
#define PRINT_BUFF(info, buff, len) do{\
    UINT32 __pos;\
    dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "%s[Length = %ld]: ", info, len);\
    for(__pos = 0; __pos < (len); __pos ++)\
    {\
        sys_print(LOGSTDOUT, "%x,", ((UINT8 *)buff)[ __pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define PRINT_BUFF(info, buff, len) do{}while(0)
#endif

/**
*
*   start one server
*
**/
EC_BOOL tasks_srv_start(TASKS_CFG *tasks_cfg)
{
    if(EC_FALSE == csocket_srv_start(TASKS_CFG_SRVIPADDR(tasks_cfg),TASKS_CFG_SRVPORT(tasks_cfg), 
                                    CSOCKET_IS_NONBLOCK_MODE, 
                                    &(TASKS_CFG_SRVSOCKFD(tasks_cfg))))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_srv_start: failed to start server %s:%ld\n",
                        TASKS_CFG_SRVIPADDR_STR(tasks_cfg),
                        TASKS_CFG_SRVPORT(tasks_cfg));
        return (EC_FALSE);
    }

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
    cepoll_set_event(task_brd_default_get_cepoll(), 
                      TASKS_CFG_SRVSOCKFD(tasks_cfg), 
                      CEPOLL_RD_EVENT, 
                      (CEPOLL_EVENT_HANDLER)tasks_srv_accept, 
                      (void *)tasks_cfg);
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/
    

    dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_srv_start: start server %s:%ld:%d\n",
                    TASKS_CFG_SRVIPADDR_STR(tasks_cfg),
                    TASKS_CFG_SRVPORT(tasks_cfg),
                    TASKS_CFG_SRVSOCKFD(tasks_cfg));

    return (EC_TRUE);
}

/**
*
*   stop one server
*   1. stop all connection to this server
*   2. stop server itself
*
**/
EC_BOOL tasks_srv_end(TASKS_CFG *tasks_cfg)
{
    cvector_clean(TASKS_CFG_WORK(tasks_cfg), (CVECTOR_DATA_CLEANER)tasks_node_free, LOC_TASKS_0001);

    if(EC_FALSE == csocket_srv_end(TASKS_CFG_SRVSOCKFD(tasks_cfg)))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_srv_end: failed to end server on %s:%ld:%d\n",
                        TASKS_CFG_SRVIPADDR_STR(tasks_cfg), TASKS_CFG_SRVPORT(tasks_cfg), TASKS_CFG_SRVSOCKFD(tasks_cfg));
        return (EC_FALSE);
    }
    TASKS_CFG_SRVSOCKFD(tasks_cfg) = CMPI_ERROR_SOCKFD;

    tasks_work_clean(TASKS_CFG_WORK(tasks_cfg));
    tasks_monitor_clean(TASKS_CFG_MONITOR(tasks_cfg));
    return (EC_TRUE);
}

/**
*
*   server accept a new connection
*   1. accept a new connection if has
*   2. create a client node with remote client ip info (note: server unknow remote client port info at present)
*   3. add the client node to client set of server
*
**/
EC_BOOL tasks_srv_accept_once(TASKS_CFG *tasks_cfg, EC_BOOL *continue_flag)
{
    UINT32  client_ipaddr;

    EC_BOOL ret;
    int     client_conn_sockfd;
    
    ret = csocket_accept( TASKS_CFG_SRVSOCKFD(tasks_cfg), &(client_conn_sockfd), CSOCKET_IS_NONBLOCK_MODE, &(client_ipaddr));
    if(EC_TRUE == ret)
    {
        CSOCKET_CNODE *csocket_cnode;

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_srv_accept_once: handle new sockfd %d\n", client_conn_sockfd);

        csocket_cnode = csocket_cnode_new(CMPI_ERROR_TCID, client_conn_sockfd, client_ipaddr, CMPI_ERROR_SRVPORT);/*here do not know the remote client srv port*/
        if(NULL_PTR == csocket_cnode)
        {
            dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_srv_accept_once:failed to alloc csocket cnode for sockfd %d, hence close it\n", client_conn_sockfd);
            csocket_close(client_conn_sockfd);
            //return (EC_FALSE);
            return (EC_TRUE); /*ignore error*/
        }

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
#if 0
        cepoll_set_event(task_brd_default_get_cepoll(), 
                         CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                         CEPOLL_RD_EVENT, 
                         (CEPOLL_EVENT_HANDLER)tasks_monitor_irecv_on_csocket_cnode_by_epoll, 
                          (void *)csocket_cnode);
#endif
        cepoll_set_event(task_brd_default_get_cepoll(), 
                         CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                         CEPOLL_WR_EVENT, 
                         (CEPOLL_EVENT_HANDLER)tasks_monitor_share_taskc_node_epoll, 
                          (void *)csocket_cnode);

       cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           (CEPOLL_EVENT_HANDLER)csocket_cnode_set_disconnected,
                           (void *)csocket_cnode);                           

       cepoll_set_timeout(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           (uint32_t)0,
                           (CEPOLL_EVENT_HANDLER)NULL_PTR,
                           NULL_PTR);
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/    

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_srv_accept_once: server %s:%ld:%d accept new client %s:%d\n",
                        TASKS_CFG_SRVIPADDR_STR(tasks_cfg), TASKS_CFG_SRVPORT(tasks_cfg), TASKS_CFG_SRVSOCKFD(tasks_cfg),
                        c_word_to_ipv4(client_ipaddr), client_conn_sockfd
                );
                
        cvector_push(TASKS_CFG_MONITOR(tasks_cfg), (void *)csocket_cnode);/*we do not know which taskComm this client belongs to*/

        //csocket_nonblock_enable(CSOCKET_CNODE_SOCKFD(csocket_cnode));
    }

    (*continue_flag) = ret;
    
    return (EC_TRUE);
}

EC_BOOL tasks_srv_accept(TASKS_CFG *tasks_cfg)
{
    UINT32   idx;
    UINT32   num;
    EC_BOOL  continue_flag;

    num = CTASKS_SRV_ACCEPT_MAX_NUM;
    for(idx = 0; idx < num; idx ++)
    {
        if(EC_FALSE == tasks_srv_accept_once(tasks_cfg, &continue_flag))
        {
            dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_srv_accept: accept No. %ld client failed where expect %ld clients\n", idx, num);
            return (EC_FALSE);
        }

        if(EC_FALSE == continue_flag)
        {
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_srv_accept: accept No. %ld client terminate where expect %ld clients\n", idx, num);
            break;
        }        
    }

    return (EC_TRUE);
}

/**
*
*   select server socket or one client socket asyncally
*   1. copy ALL FD SET to READ FD SET of server
*   2. check wethere server socket or one client socket has data to read
*
**/
EC_BOOL tasks_srv_select(TASKS_CFG *tasks_cfg, int *ret)
{
    if(ERR_FD != TASKS_CFG_SRVSOCKFD(tasks_cfg))
    {
        FD_CSET fd_cset;
        int max_sockfd;

        struct timeval tv;

        tv.tv_sec  = 0;
        tv.tv_usec = /*1*/0;

        max_sockfd = 0;
        csocket_fd_clean(&fd_cset);
        csocket_fd_set(TASKS_CFG_SRVSOCKFD(tasks_cfg), &fd_cset, &max_sockfd);
        return csocket_select(max_sockfd + 1, &fd_cset, NULL_PTR, NULL_PTR, &tv, ret);
    }
    dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_srv_select: tasks_cfg %p srvsockfd is %d\n", tasks_cfg, TASKS_CFG_SRVSOCKFD(tasks_cfg));
    return (EC_FALSE);
}


/**
*
*   handle message to server
*   1. if find any connection dead body, delete it
*   2. if find any broken connection, delete it
*   3. remove a message and hand it on call-back handler
*
**/
EC_BOOL tasks_srv_handle(TASKS_CFG *tasks_cfg)
{
    EC_BOOL ret;
    TASK_BRD *task_brd;
    task_brd = task_brd_default_get();
    ret = EC_TRUE;

    if(EC_FALSE == tasks_monitor_xchg_taskc_node(TASKS_CFG_MONITOR(tasks_cfg)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_monitor_irecv(TASKS_CFG_MONITOR(tasks_cfg)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_monitor_isend(TASKS_CFG_MONITOR(tasks_cfg)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_monitor_incomed(TASKS_CFG_MONITOR(tasks_cfg)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_monitor_move_to_work(TASKS_CFG_MONITOR(tasks_cfg), TASKS_CFG_WORK(tasks_cfg)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_work_irecv(TASKS_CFG_WORK(tasks_cfg), TASK_BRD_QUEUE(task_brd, TASK_RECVING_QUEUE)))
    {
        ret = EC_FALSE;
    }

    if(EC_FALSE == tasks_work_isend(TASKS_CFG_WORK(tasks_cfg)))
    {
        ret = EC_FALSE;
    }
#if 1
    if(EC_FALSE == tasks_work_heartbeat(TASKS_CFG_WORK(tasks_cfg)))
    {
        ret = EC_FALSE;
    }
#endif
    return (ret);
}

EC_BOOL tasks_do_once(TASKS_CFG *tasks_cfg)
{
#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
    int ret;

    if(EC_FALSE == tasks_srv_select(tasks_cfg, &ret))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_do_once: select failed\n");

        return (EC_FALSE);
    }
    if( 0 < ret )
    {
        EC_BOOL continue_flag;
        /*handle new connection*/
        tasks_srv_accept_once(tasks_cfg, &continue_flag);
    }
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/

    /*handle each connection*/
    tasks_srv_handle(tasks_cfg);

    return (EC_TRUE);
}

EC_BOOL tasks_srv_of_tcid(const TASKS_CFG *tasks_cfg, const UINT32 tcid)
{
    if(tcid == TASKS_CFG_TCID(tasks_cfg))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

TASKS_NODE *tasks_node_new(const UINT32 srvipaddr, const UINT32 srvport, const UINT32 tcid, const UINT32 comm, const UINT32 size)
{
    TASKS_NODE *tasks_node;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_TASKS_NODE, &tasks_node, LOC_TASKS_0002);
    if(NULL_PTR == tasks_node)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_node_new: failed to alloc tasks node\n");
        return (NULL_PTR);
    }

    tasks_node_init(tasks_node, srvipaddr, srvport, tcid, comm, size);
    return (tasks_node);
}

EC_BOOL tasks_node_init(TASKS_NODE *tasks_node, const UINT32 srvipaddr, const UINT32 srvport, const UINT32 tcid, const UINT32 comm, const UINT32 size)
{
    TASKS_NODE_SRVIPADDR(tasks_node) = srvipaddr;
    TASKS_NODE_SRVPORT(tasks_node)   = srvport;
    TASKS_NODE_TCID(tasks_node)      = tcid;
    TASKS_NODE_COMM(tasks_node)      = comm;
    TASKS_NODE_SIZE(tasks_node)      = size;
    TASKS_NODE_LOAD(tasks_node)      = 0;

    CTIMET_GET(TASKS_NODE_LAST_UPDATE_TIME(tasks_node));
    CTIMET_GET(TASKS_NODE_LAST_SEND_TIME(tasks_node));

    cvector_init(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), 0, MM_CSOCKET_CNODE, CVECTOR_LOCK_ENABLE, LOC_TASKS_0003);
    return (EC_TRUE);
}

EC_BOOL tasks_node_clean(TASKS_NODE *tasks_node)
{
    cvector_clean(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), (CVECTOR_DATA_CLEANER)csocket_cnode_close_and_clean_event, LOC_TASKS_0004);;

    TASKS_NODE_SRVIPADDR(tasks_node) = CMPI_ERROR_IPADDR;
    TASKS_NODE_SRVPORT(tasks_node)   = CMPI_ERROR_SRVPORT;;
    TASKS_NODE_TCID(tasks_node)      = CMPI_ERROR_TCID;
    TASKS_NODE_COMM(tasks_node)      = CMPI_ERROR_COMM;
    TASKS_NODE_SIZE(tasks_node)      = 0;
    TASKS_NODE_LOAD(tasks_node)      = 0;

    //CTIMET_GET(TASKS_NODE_LAST_UPDATE_TIME(tasks_node));
    //CTIMET_GET(TASKS_NODE_LAST_SEND_TIME(tasks_node));

    return (EC_TRUE);
}

EC_BOOL tasks_node_free(TASKS_NODE *tasks_node)
{
    if(NULL_PTR != tasks_node)
    {
        tasks_node_clean(tasks_node);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_TASKS_NODE, tasks_node, LOC_TASKS_0005);
    }
    return (EC_TRUE);
}

EC_BOOL tasks_node_is_connected(const TASKS_NODE *tasks_node)
{
    UINT32 pos;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0006);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        /*regard tasks_node is connected if exist any one connected csocket_cnode*/
        if(EC_TRUE == csocket_cnode_is_connected(csocket_cnode))
        {
            CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0007);
            return (EC_TRUE);
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0008);
    return (EC_FALSE);
}

EC_BOOL tasks_node_is_connected_no_lock(const TASKS_NODE *tasks_node)
{
    UINT32 pos;

    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        /*regard tasks_node is connected if exist any one connected csocket_cnode*/
        if(EC_TRUE == csocket_cnode_is_connected(csocket_cnode))
        {
            return (EC_TRUE);
        }
    }
    return (EC_FALSE);
}

UINT32 tasks_node_count_load(const TASKS_NODE *tasks_node)
{
    UINT32 pos;
    UINT32 load_sum;

    load_sum = 0;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0009);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        load_sum += CSOCKET_CNODE_LOAD(csocket_cnode);
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0010);
    return (load_sum);
}

CSOCKET_CNODE *tasks_node_search_csocket_cnode_with_min_load(const TASKS_NODE *tasks_node)
{
    UINT32 pos;

    CSOCKET_CNODE *min_csocket_cnode;
    UINT32         min_load;

    min_csocket_cnode = NULL_PTR;
    min_load = ((UINT32)-1);

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0011);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        /*shortcut*/
        if(0 == CSOCKET_CNODE_LOAD(csocket_cnode))
        {
            CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0012);
            return (csocket_cnode);
        }

        if(min_load > CSOCKET_CNODE_LOAD(csocket_cnode))
        {
            min_csocket_cnode = csocket_cnode;
            min_load = CSOCKET_CNODE_LOAD(csocket_cnode);
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0013);
    return (min_csocket_cnode);
}

CSOCKET_CNODE *tasks_node_search_csocket_cnode_by_sockfd(const TASKS_NODE *tasks_node, const int sockfd)
{
    UINT32 pos;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0014);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(sockfd == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0015);
            return (csocket_cnode);
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0016);
    return (NULL_PTR);
}

EC_BOOL tasks_node_export_csocket_cnode_vec(const TASKS_NODE *tasks_node, CVECTOR *csocket_cnode_vec)
{
    cvector_clone(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), csocket_cnode_vec, (CVECTOR_DATA_MALLOC)csocket_cnode_new_0, (CVECTOR_DATA_CLONE)csocket_cnode_clone_0);
    return (EC_TRUE);
}

EC_BOOL tasks_node_is_empty(const TASKS_NODE *tasks_node)
{
    UINT32 pos;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0017);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR != csocket_cnode)
        {
            CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0018);
            return (EC_FALSE);
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0019);
    return (EC_TRUE);
}

EC_BOOL tasks_node_cmp(const TASKS_NODE *src_tasks_node, const TASKS_NODE *des_tasks_node)
{
    if(TASKS_NODE_SRVIPADDR(src_tasks_node) != TASKS_NODE_SRVIPADDR(des_tasks_node))
    {
        return (EC_FALSE);
    }

    if(TASKS_NODE_SRVPORT(src_tasks_node) != TASKS_NODE_SRVPORT(des_tasks_node))
    {
        return (EC_FALSE);
    }

    if(TASKS_NODE_TCID(src_tasks_node) != TASKS_NODE_TCID(des_tasks_node))
    {
        return (EC_FALSE);
    }

    if(TASKS_NODE_COMM(src_tasks_node) != TASKS_NODE_COMM(des_tasks_node))
    {
        return (EC_FALSE);
    }

    if(TASKS_NODE_SIZE(src_tasks_node) != TASKS_NODE_SIZE(des_tasks_node))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL tasks_node_check(const TASKS_NODE *tasks_node, const CSOCKET_CNODE *csocket_cnode)
{
    if(TASKS_NODE_TCID(tasks_node) != CSOCKET_CNODE_TCID(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_node_check: tcid mismatched: %s <---> %s\n", TASKS_NODE_TCID_STR(tasks_node), CSOCKET_CNODE_TCID_STR(csocket_cnode));
        return (EC_FALSE);
    }

    if(TASKS_NODE_COMM(tasks_node) != CSOCKET_CNODE_COMM(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_node_check: comm mismatched: %ld <---> %ld\n", TASKS_NODE_COMM(tasks_node), CSOCKET_CNODE_COMM(csocket_cnode));
        return (EC_FALSE);
    }

    if(TASKS_NODE_SIZE(tasks_node) != CSOCKET_CNODE_SIZE(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_node_check: size mismatched: %ld <---> %ld\n", TASKS_NODE_SIZE(tasks_node), CSOCKET_CNODE_SIZE(csocket_cnode));
        return (EC_FALSE);
    }

    if(TASKS_NODE_SRVIPADDR(tasks_node) != CSOCKET_CNODE_IPADDR(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_node_check: ipaddr mismatched: %ld <---> %ld\n", TASKS_NODE_SRVIPADDR(tasks_node), CSOCKET_CNODE_IPADDR(csocket_cnode));
        return (EC_FALSE);
    }

    if(TASKS_NODE_SRVPORT(tasks_node) != CSOCKET_CNODE_SRVPORT(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_node_check: port mismatched: %ld <---> %ld\n", TASKS_NODE_SRVPORT(tasks_node), CSOCKET_CNODE_SRVPORT(csocket_cnode));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL tasks_node_is_tcid(const TASKS_NODE *src_tasks_node, const UINT32 tcid)
{
    if(tcid == TASKS_NODE_TCID(src_tasks_node))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL tasks_node_is_ipaddr(const TASKS_NODE *src_tasks_node, const UINT32 ipaddr)
{
    if(ipaddr == TASKS_NODE_SRVIPADDR(src_tasks_node))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

void tasks_node_print(LOG *log, const TASKS_NODE *tasks_node)
{
    CVECTOR *csocket_cnode_vec;
    UINT32 pos;

    csocket_cnode_vec = (CVECTOR *)TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node);
    CVECTOR_LOCK(csocket_cnode_vec, LOC_TASKS_0020);
    for(pos = 0; pos < cvector_size(csocket_cnode_vec); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(csocket_cnode_vec, pos);
        if(NULL_PTR == csocket_cnode)
        {
            sys_log(log, "csocket_cnode_vec %lx No. %ld: (null)\n", csocket_cnode_vec, pos);
            continue;
        }

        sys_log(log, "csocket_cnode_vec %lx No. %ld: srvipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd %d\n",
                    csocket_cnode_vec, pos,
                    TASKS_NODE_SRVIPADDR_STR(tasks_node),
                    TASKS_NODE_SRVPORT(tasks_node),
                    TASKS_NODE_TCID_STR(tasks_node),
                    TASKS_NODE_COMM(tasks_node),
                    TASKS_NODE_SIZE(tasks_node),
                    CSOCKET_CNODE_SOCKFD(csocket_cnode)
                );
    }
    CVECTOR_UNLOCK(csocket_cnode_vec, LOC_TASKS_0021);
    return ;
}

void tasks_node_print_csocket_cnode_list(LOG *log, const TASKS_NODE *tasks_node, UINT32 *index)
{
    CVECTOR *csocket_cnode_vec;
    UINT32 pos;

    csocket_cnode_vec = (CVECTOR *)TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node);
    CVECTOR_LOCK(csocket_cnode_vec, LOC_TASKS_0022);
    for(pos = 0; pos < cvector_size(csocket_cnode_vec); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(csocket_cnode_vec, pos);
        if(NULL_PTR == csocket_cnode)
        {
            sys_log(log, "No. %ld: (csocket cnode is null)\n", (*index) ++);
            continue;
        }

        sys_log(log, "No. %ld: srvipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd %d\n",
                    (*index) ++,
                    TASKS_NODE_SRVIPADDR_STR(tasks_node),
                    TASKS_NODE_SRVPORT(tasks_node),
                    TASKS_NODE_TCID_STR(tasks_node),
                    TASKS_NODE_COMM(tasks_node),
                    TASKS_NODE_SIZE(tasks_node),
                    CSOCKET_CNODE_SOCKFD(csocket_cnode)
                );
    }
    CVECTOR_UNLOCK(csocket_cnode_vec, LOC_TASKS_0023);
    return ;
}

void tasks_node_print_in_plain(LOG *log, const TASKS_NODE *tasks_node)
{
    UINT32 pos;
    UINT32 csocket_cnode_num;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0024);
    csocket_cnode_num = cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node));
    sys_print(log, "srvipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd ",
                TASKS_NODE_SRVIPADDR_STR(tasks_node),
                TASKS_NODE_SRVPORT(tasks_node),
                TASKS_NODE_TCID_STR(tasks_node),
                TASKS_NODE_COMM(tasks_node),
                TASKS_NODE_SIZE(tasks_node)
            );
    for(pos = 0; pos < csocket_cnode_num; pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(pos + 1 >= csocket_cnode_num)
        {
            sys_print(log, "%d\n",CSOCKET_CNODE_SOCKFD(csocket_cnode));
        }
        else
        {
            sys_print(log, "%d:",CSOCKET_CNODE_SOCKFD(csocket_cnode));
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0025);
    return ;
}

void tasks_node_sprint(CSTRING *cstring, const TASKS_NODE *tasks_node)
{
    UINT32 pos;
    UINT32 csocket_cnode_num;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0026);
    csocket_cnode_num = cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node));
    cstring_format(cstring, "srvipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd ",
                TASKS_NODE_SRVIPADDR_STR(tasks_node),
                TASKS_NODE_SRVPORT(tasks_node),
                TASKS_NODE_TCID_STR(tasks_node),
                TASKS_NODE_COMM(tasks_node),
                TASKS_NODE_SIZE(tasks_node)
            );
    for(pos = 0; pos < csocket_cnode_num; pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(pos + 1 >= csocket_cnode_num)
        {
            cstring_format(cstring, "%d\n",CSOCKET_CNODE_SOCKFD(csocket_cnode));
        }
        else
        {
            cstring_format(cstring, "%d:",CSOCKET_CNODE_SOCKFD(csocket_cnode));
        }
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0027);
    return ;
}

static EC_BOOL tasks_node_was_trigger(TASK_BRD *task_brd, TASKS_NODE *tasks_node)
{
    UINT32 heartbeat_interval;
    UINT32 elapsed_time_from_last_update;
    UINT32 elapsed_time_from_last_send;
    CTIMET cur;

    heartbeat_interval = (UINT32)CSOCKET_HEARTBEAT_INTVL_NSEC;

    CTIMET_GET(cur);

    elapsed_time_from_last_update = lrint(CTIMET_DIFF(TASKS_NODE_LAST_UPDATE_TIME(tasks_node), cur));
    elapsed_time_from_last_send   = lrint(CTIMET_DIFF(TASKS_NODE_LAST_SEND_TIME(tasks_node), cur));

    if(3 * heartbeat_interval <= elapsed_time_from_last_update)
    {
        return (EC_TRUE);
    }

    if(/*heartbeat_interval <= elapsed_time_from_last_update && */heartbeat_interval <= elapsed_time_from_last_send && (SWITCH_ON == RANK_HEARTBEAT_NODE_SWITCH))
    {
        return (EC_TRUE);
    }

    if(/*heartbeat_interval <= elapsed_time_from_last_update && */heartbeat_interval <= elapsed_time_from_last_send && (SWITCH_OFF == RANK_HEARTBEAT_NODE_SWITCH))
    {
        return (EC_TRUE);
    }

    return (EC_TRUE);
}

EC_BOOL tasks_node_trigger(TASK_BRD *task_brd, TASK_MGR *task_mgr, TASKS_NODE *tasks_node)
{
    MOD_NODE send_mod_node;

    UINT32 heartbeat_interval;
    UINT32 elapsed_time_from_last_update;
    UINT32 elapsed_time_from_last_send;
    CTIMET cur;

    MOD_NODE_TCID(&send_mod_node) = TASK_BRD_TCID(task_brd);
    MOD_NODE_COMM(&send_mod_node) = TASK_BRD_COMM(task_brd);
    MOD_NODE_RANK(&send_mod_node) = TASK_BRD_RANK(task_brd);
    MOD_NODE_MODI(&send_mod_node) = 0;
    MOD_NODE_HOPS(&send_mod_node) = 0;
    MOD_NODE_LOAD(&send_mod_node) = 0/*TASK_BRD_LOAD(task_brd)*/;

    heartbeat_interval = (UINT32)CSOCKET_HEARTBEAT_INTVL_NSEC;

    CTIMET_GET(cur);

    elapsed_time_from_last_update = lrint(CTIMET_DIFF(TASKS_NODE_LAST_UPDATE_TIME(tasks_node), cur));
    elapsed_time_from_last_send   = lrint(CTIMET_DIFF(TASKS_NODE_LAST_SEND_TIME(tasks_node), cur));

    if(3 * heartbeat_interval <= elapsed_time_from_last_update)
    {
        MOD_NODE recv_mod_node;
        CTM   *last_update_tm;
        CTM   *last_end_tm;
        CTM   *cur_tm;

        MOD_NODE_TCID(&recv_mod_node) = TASK_BRD_TCID(task_brd);
        MOD_NODE_COMM(&recv_mod_node) = TASK_BRD_COMM(task_brd);
        MOD_NODE_RANK(&recv_mod_node) = TASK_BRD_RANK(task_brd);
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        task_super_inc(task_mgr, &send_mod_node, &recv_mod_node, NULL_PTR, FI_super_notify_broken_tcid, ERR_MODULE_ID, TASKS_NODE_TCID(tasks_node));

        last_update_tm = CTIMET_TO_TM(TASKS_NODE_LAST_UPDATE_TIME(tasks_node));
        last_end_tm    = CTIMET_TO_TM(TASKS_NODE_LAST_SEND_TIME(tasks_node));
        cur_tm         = CTIMET_TO_TM(cur);

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[%s] last update time: %02d:%02d:%02d, last send time: %02d:%02d:%02d, cur time: %02d:%02d:%02d, CSOCKET_HEARTBEAT_INTVL_NSEC = (%ld), elapsed from last update: %ld, elapsed from last send: %ld, trigger broken\n",
                TASKS_NODE_TCID_STR(tasks_node),
                CTM_HOUR(last_update_tm),
                CTM_MIN(last_update_tm ),
                CTM_SEC(last_update_tm ),

                CTM_HOUR(last_end_tm  ),
                CTM_MIN(last_end_tm   ),
                CTM_SEC(last_end_tm   ),

                CTM_HOUR(cur_tm),
                CTM_MIN(cur_tm ),
                CTM_SEC(cur_tm ),

                heartbeat_interval, elapsed_time_from_last_update, elapsed_time_from_last_send
            );

        return (EC_FALSE);
    }

    if(/*heartbeat_interval <= elapsed_time_from_last_update && */heartbeat_interval <= elapsed_time_from_last_send && (SWITCH_ON == RANK_HEARTBEAT_NODE_SWITCH))
    {
        MOD_NODE recv_mod_node;
        CLOAD_NODE *cload_node;

        CTM   *last_update_tm;
        CTM   *last_end_tm;
        CTM   *cur_tm;        

        MOD_NODE_TCID(&recv_mod_node) = TASKS_NODE_TCID(tasks_node);
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        cload_node = cload_mgr_search(TASK_BRD_CLOAD_MGR(task_brd), TASK_BRD_TCID(task_brd));
        if(NULL_PTR == cload_node)
        {
            dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_node_trigger: cload node of tcid %s not exist\n", TASK_BRD_TCID_STR(task_brd));
            return (EC_FALSE);
        }

        task_super_inc(task_mgr, &send_mod_node, &recv_mod_node, NULL_PTR, FI_super_heartbeat_on_node, ERR_MODULE_ID, cload_node);

        last_update_tm = CTIMET_TO_TM(TASKS_NODE_LAST_UPDATE_TIME(tasks_node));
        last_end_tm    = CTIMET_TO_TM(TASKS_NODE_LAST_SEND_TIME(tasks_node));
        cur_tm         = CTIMET_TO_TM(cur);

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[%s] last update time: %02d:%02d:%02d, last send time: %02d:%02d:%02d, cur time: %02d:%02d:%02d, CSOCKET_HEARTBEAT_INTVL_NSEC = (%ld), elapsed from last update: %ld, elapsed from last send: %ld, trigger heartbeat\n",
                TASKS_NODE_TCID_STR(tasks_node),
                CTM_HOUR(last_update_tm),
                CTM_MIN(last_update_tm ),
                CTM_SEC(last_update_tm ),

                CTM_HOUR(last_end_tm  ),
                CTM_MIN(last_end_tm   ),
                CTM_SEC(last_end_tm   ),

                CTM_HOUR(cur_tm),
                CTM_MIN(cur_tm ),
                CTM_SEC(cur_tm ),
                
                heartbeat_interval, elapsed_time_from_last_update, elapsed_time_from_last_send
            );

        CTIMET_GET(TASKS_NODE_LAST_SEND_TIME(tasks_node));
        return (EC_TRUE);
    }

    if(/*heartbeat_interval <= elapsed_time_from_last_update && */heartbeat_interval <= elapsed_time_from_last_send && (SWITCH_OFF == RANK_HEARTBEAT_NODE_SWITCH))
    {
        MOD_NODE recv_mod_node;

        CTM   *last_update_tm;
        CTM   *last_end_tm;
        CTM   *cur_tm;        
        

        MOD_NODE_TCID(&recv_mod_node) = TASKS_NODE_TCID(tasks_node);
        MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
        MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
        MOD_NODE_MODI(&recv_mod_node) = 0;
        MOD_NODE_HOPS(&recv_mod_node) = 0;
        MOD_NODE_LOAD(&recv_mod_node) = 0;

        task_super_inc(task_mgr, &send_mod_node, &recv_mod_node, NULL_PTR, FI_super_heartbeat_none, ERR_MODULE_ID);

        last_update_tm = CTIMET_TO_TM(TASKS_NODE_LAST_UPDATE_TIME(tasks_node));
        last_end_tm    = CTIMET_TO_TM(TASKS_NODE_LAST_SEND_TIME(tasks_node));
        cur_tm         = CTIMET_TO_TM(cur);

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[%s] last update time: %02d:%02d:%02d, last send time: %02d:%02d:%02d, cur time: %02d:%02d:%02d, CSOCKET_HEARTBEAT_INTVL_NSEC = (%ld), elapsed from last update: %ld, elapsed from last send: %ld, trigger heartbeat\n",
                TASKS_NODE_TCID_STR(tasks_node),
                CTM_HOUR(last_update_tm),
                CTM_MIN(last_update_tm ),
                CTM_SEC(last_update_tm ),

                CTM_HOUR(last_end_tm  ),
                CTM_MIN(last_end_tm   ),
                CTM_SEC(last_end_tm   ),

                CTM_HOUR(cur_tm),
                CTM_MIN(cur_tm ),
                CTM_SEC(cur_tm ),
                
                heartbeat_interval, elapsed_time_from_last_update, elapsed_time_from_last_send
            );

        CTIMET_GET(TASKS_NODE_LAST_SEND_TIME(tasks_node));
        return (EC_TRUE);
    }

    return (EC_TRUE);
}

UINT32 tasks_work_count_no_lock(const CVECTOR *tasks_node_work, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port)
{
    UINT32 pos;
    UINT32 count;

    for(count = 0, pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;
        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(tcid == CSOCKET_CNODE_TCID(csocket_cnode)
        && srv_ipaddr == CSOCKET_CNODE_IPADDR(csocket_cnode)
        && srv_port == CSOCKET_CNODE_SRVPORT(csocket_cnode)
        )
        {
            count ++;
        }
    }

    return (count);
}

UINT32 tasks_work_count(const CVECTOR *tasks_node_work, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port)
{
    UINT32 count;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0028);
    count = tasks_work_count_no_lock(tasks_node_work, tcid, srv_ipaddr, srv_port);
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0029);

    return (count);
}

EC_BOOL tasks_work_export_csocket_cnode_vec(const CVECTOR *tasks_node_work, CVECTOR *csocket_cnode_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0030);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }
        tasks_node_export_csocket_cnode_vec(tasks_node, csocket_cnode_vec);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0031);
    return (NULL_PTR);
}

TASKS_NODE *tasks_work_search_tasks_node_by_ipaddr(const CVECTOR *tasks_node_work, const UINT32 ipaddr)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0032);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(ipaddr == TASKS_NODE_SRVIPADDR(tasks_node))
        {
            CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0033);
            return (tasks_node);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0034);
    return (NULL_PTR);
}

TASKS_NODE *tasks_work_search_tasks_node_by_ipaddr_no_lock(const CVECTOR *tasks_node_work, const UINT32 ipaddr)
{
    UINT32 pos;

    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(ipaddr == TASKS_NODE_SRVIPADDR(tasks_node))
        {
            return (tasks_node);
        }
    }
    return (NULL_PTR);
}


TASKS_NODE *tasks_work_search_tasks_node_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tcid)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0035);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(tcid == TASKS_NODE_TCID(tasks_node))
        {
            CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0036);
            return (tasks_node);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0037);
    return (NULL_PTR);
}

TASKS_NODE *tasks_work_search_tasks_node_by_tcid_no_lock(const CVECTOR *tasks_node_work, const UINT32 tcid)
{
    UINT32 pos;

    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(tcid == TASKS_NODE_TCID(tasks_node))
        {
            return (tasks_node);
        }
    }
    return (NULL_PTR);
}


CSOCKET_CNODE *tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tasks_tcid)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0038);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(tasks_tcid == TASKS_NODE_TCID(tasks_node))/*check tcid directly without mask*/
        {
            CSOCKET_CNODE *csocket_cnode;

            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid: tasks_node %p matched tcid %s\n",
                                 tasks_node, c_word_to_ipv4(tasks_tcid));
            //CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0039);

            csocket_cnode = tasks_node_search_csocket_cnode_with_min_load(tasks_node);
            CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0040);
            return (csocket_cnode);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0041);

    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid: no tasks_node matched tcid %s (vec size %ld)\n",
                         c_word_to_ipv4(tasks_tcid), cvector_size(tasks_node_work));    
    return (NULL_PTR);
}

CSOCKET_CNODE *tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 des_tcid)
{
    UINT32 pos;
    TASKS_CFG *tasks_cfg;

    tasks_cfg = TASKS_WORK_BASE_TASKS_CFG_ENTRY(tasks_node_work);

    CVECTOR_LOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_TASKS_0042);
    for(pos = 0; pos < cvector_size(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg)); pos ++)
    {
        TASKR_CFG *taskr_cfg;
        UINT32 taskr_cfg_mask;

        taskr_cfg = (TASKR_CFG *)cvector_get_no_lock(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), pos);
        if(NULL_PTR == taskr_cfg)
        {
            continue;
        }

        taskr_cfg_mask = TASKR_CFG_MASKR(taskr_cfg);

        /*when des_tcid belong to the intranet of taskr_cfg, i.e., belong to the route*/
        if((des_tcid & taskr_cfg_mask) == (TASKR_CFG_DES_TCID(taskr_cfg) & taskr_cfg_mask))
        {
            CSOCKET_CNODE *csocket_cnode;

            csocket_cnode = tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid(tasks_node_work, TASKR_CFG_NEXT_TCID(taskr_cfg));
            if(NULL_PTR != csocket_cnode)
            {
                dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid: %s & %s == %s & %s  ==> %s [reachable]\n",
                                    c_word_to_ipv4(des_tcid), c_word_to_ipv4(taskr_cfg_mask),
                                    c_word_to_ipv4(TASKR_CFG_DES_TCID(taskr_cfg)), c_word_to_ipv4(taskr_cfg_mask),
                                    c_word_to_ipv4(TASKR_CFG_NEXT_TCID(taskr_cfg))
                                    );
                /*TODO: later we can find out all matched routes and csocket cnodes, and then filter the min load one*/
                CVECTOR_UNLOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_TASKS_0043);
                return (csocket_cnode);
            }
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid: %s & %s == %s & %s  ==> %s [unreachable]\n",
                                c_word_to_ipv4(des_tcid), c_word_to_ipv4(taskr_cfg_mask),
                                c_word_to_ipv4(TASKR_CFG_DES_TCID(taskr_cfg)), c_word_to_ipv4(taskr_cfg_mask),
                                c_word_to_ipv4(TASKR_CFG_NEXT_TCID(taskr_cfg))
                                );
        }
        else
        {
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid: %s & %s != %s & %s\n",
                                c_word_to_ipv4(des_tcid), c_word_to_ipv4(taskr_cfg_mask),
                                c_word_to_ipv4(TASKR_CFG_DES_TCID(taskr_cfg)), c_word_to_ipv4(taskr_cfg_mask)
                                );
        }
    }
    CVECTOR_UNLOCK(TASKS_CFG_TASKR_CFG_VEC(tasks_cfg), LOC_TASKS_0044);

    return (NULL_PTR);
}

CSOCKET_CNODE *tasks_work_search_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tcid)
{
    CSOCKET_CNODE * csocket_cnode;

    /*check existing connections*/
    csocket_cnode = tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid(tasks_node_work, tcid);
    if(NULL_PTR != csocket_cnode)
    {
        return (csocket_cnode);
    }

    /*check route table*/
    csocket_cnode = tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid(tasks_node_work, tcid);
    if(NULL_PTR != csocket_cnode)
    {
        return (csocket_cnode);
    }

    return (NULL_PTR);
}

CSOCKET_CNODE *tasks_work_search_csocket_cnode_by_tcid_sockfd(const CVECTOR *tasks_work, const UINT32 tcid, const int sockfd)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_work, LOC_TASKS_0045);
    for(pos = 0; pos < cvector_size(tasks_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(tcid == TASKS_NODE_TCID(tasks_node))
        {
            CSOCKET_CNODE *csocket_cnode;
            csocket_cnode = tasks_node_search_csocket_cnode_by_sockfd(tasks_node, sockfd);
            CVECTOR_UNLOCK(tasks_work, LOC_TASKS_0046);
            return (csocket_cnode);
        }
    }

    CVECTOR_UNLOCK(tasks_work, LOC_TASKS_0047);
    return (NULL_PTR);
}

UINT32 tasks_work_search_tcid_by_ipaddr(const CVECTOR *tasks_work, const UINT32 ipaddr)
{
    TASKS_NODE *tasks_node;

    tasks_node = tasks_work_search_tasks_node_by_ipaddr(tasks_work, ipaddr);
    if(NULL_PTR == tasks_node)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_search_tcid_by_ipaddr: failed to find tasks_node of ipaddr %s\n", c_word_to_ipv4(ipaddr));
        return (CMPI_ERROR_TCID);
    }

    return (TASKS_NODE_TCID(tasks_node));
}

EC_BOOL tasks_work_check_connected_by_tcid(const CVECTOR *tasks_work, const UINT32 tcid)
{
    TASKS_NODE *tasks_node;

    if(tcid == task_brd_default_get_tcid())
    {
        dbg_log(SEC_0121_TASKS, 3)(LOGSTDOUT, "info:tasks_work_check_connected_by_tcid: check myself tcid %s\n", c_word_to_ipv4(tcid));
        return (EC_TRUE);
    }

    tasks_node = tasks_work_search_tasks_node_by_tcid(tasks_work, tcid);
    if(NULL_PTR == tasks_node)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_check_connected_by_tcid: failed to find tasks_node of tcid %s\n", c_word_to_ipv4(tcid));
        return (EC_FALSE);
    }

    if(EC_FALSE == tasks_node_is_connected(tasks_node))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_work_check_connected_by_tcid: tcid %s is NOT connected\n", c_word_to_ipv4(tcid));
        return (EC_FALSE);
    }
    dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_check_connected_by_tcid: tcid %s is connected\n", c_word_to_ipv4(tcid));
    return (EC_TRUE);
}

EC_BOOL tasks_work_check_connected_by_ipaddr(const CVECTOR *tasks_work, const UINT32 ipaddr)
{
    TASKS_NODE *tasks_node;

    if(ipaddr == task_brd_default_get_ipaddr())
    {
        dbg_log(SEC_0121_TASKS, 3)(LOGSTDOUT, "info:tasks_work_check_connected_by_ipaddr: check myself ipaddr %s\n", c_word_to_ipv4(ipaddr));
        return (EC_TRUE);
    }

    tasks_node = tasks_work_search_tasks_node_by_ipaddr(tasks_work, ipaddr);
    if(NULL_PTR == tasks_node)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_check_connected_by_ipaddr: failed to find tasks_node of ipaddr %s\n", c_word_to_ipv4(ipaddr));
        return (EC_FALSE);
    }

    if(EC_FALSE == tasks_node_is_connected(tasks_node))
    {
        dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn:tasks_work_check_connected_by_ipaddr: ipaddr %s is NOT connected\n", c_word_to_ipv4(ipaddr));
        return (EC_FALSE);
    }
    dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_check_connected_by_ipaddr: ipaddr %s is connected\n", c_word_to_ipv4(ipaddr));
    return (EC_TRUE);
}


EC_BOOL tasks_work_add_csocket_cnode(CVECTOR *tasks_node_work, CSOCKET_CNODE *csocket_cnode)
{
    TASKS_NODE *tasks_node;

    tasks_node = tasks_work_search_tasks_node_by_tcid(tasks_node_work, CSOCKET_CNODE_TCID(csocket_cnode));
    if(NULL_PTR != tasks_node)
    {
        /*debug only*/
        if(EC_FALSE == tasks_node_check(tasks_node, csocket_cnode))
        {
            dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_add_csocket_cnode: tasks_node and csocket_cnode does not match\n");
            return (EC_FALSE);
        }

        dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] [1] tasks_node %lx tcid %s ==> csocket_cnode %lx tcid %s sockfd %d\n",
                                tasks_node, TASKS_NODE_TCID_STR(tasks_node),
                                csocket_cnode, CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_SOCKFD(csocket_cnode)
                                );

        CSOCKET_CNODE_TASKS_NODE(csocket_cnode) = tasks_node;/*shortcut*/
        cvector_push(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), (void *)csocket_cnode);
        return (EC_TRUE);
    }

    tasks_node = tasks_node_new(CSOCKET_CNODE_IPADDR(csocket_cnode),
                                CSOCKET_CNODE_SRVPORT(csocket_cnode),
                                CSOCKET_CNODE_TCID(csocket_cnode),
                                CSOCKET_CNODE_COMM(csocket_cnode),
                                CSOCKET_CNODE_SIZE(csocket_cnode));
    if(NULL_PTR == tasks_node)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_add_csocket_cnode: failed to alloc tasks node\n");
        return (EC_FALSE);
    }

    /*note: when reach here, tasks_node and csocket_cnode always match*/
    CSOCKET_CNODE_TASKS_NODE(csocket_cnode) = tasks_node;/*shortcut*/
    cvector_push(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), (void *)csocket_cnode);/*add csocket_cnode to tasks_node*/

    cvector_push(tasks_node_work, (void *)tasks_node);      /*add tasks_node to tasks_work   */
    
    return (EC_TRUE);
}

EC_BOOL tasks_work_collect_tcid(const CVECTOR *tasks_node_work, CVECTOR *tcid_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0048);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;
        UINT32 tcid;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        tcid = TASKS_NODE_TCID(tasks_node);
        if(CVECTOR_ERR_POS == cvector_search_front_no_lock(tcid_vec, (void *)tcid, (CVECTOR_DATA_CMP)tasks_node_is_tcid))
        {
            cvector_push_no_lock(tcid_vec, (void *)tcid);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0049);
    return (EC_TRUE);
}

EC_BOOL tasks_work_collect_ipaddr(const CVECTOR *tasks_node_work, CVECTOR *ipaddr_vec)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0050);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;
        UINT32 ipaddr;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        ipaddr = TASKS_NODE_SRVIPADDR(tasks_node);
        if(CVECTOR_ERR_POS == cvector_search_front_no_lock(ipaddr_vec, (void *)ipaddr, NULL_PTR))
        {
            cvector_push_no_lock(ipaddr_vec, (void *)ipaddr);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0051);
    return (EC_TRUE);
}


EC_BOOL tasks_work_init(CVECTOR *tasks_node_work)
{
    cvector_init(tasks_node_work, 0, MM_TASKS_NODE, CVECTOR_LOCK_ENABLE, LOC_TASKS_0052);

    return (EC_TRUE);
}

EC_BOOL tasks_work_clean(CVECTOR *tasks_node_work)
{
    cvector_clean(tasks_node_work, (CVECTOR_DATA_CLEANER)tasks_node_free, LOC_TASKS_0053);
    return (EC_TRUE);
}

void tasks_work_print(LOG *log, const CVECTOR *tasks_node_work)
{
    sys_log(log, "tasks_work %lx: \n", tasks_node_work);
    cvector_print(log, tasks_node_work, (CVECTOR_DATA_PRINT)tasks_node_print);

    return ;
}

void tasks_work_print_in_plain(LOG *log, const CVECTOR *tasks_node_work)
{
    UINT32 index;

    index = 0;
    tasks_work_print_csocket_cnode_list_in_plain(log, tasks_node_work, &index);

    return;
}

void tasks_work_print_csocket_cnode_list_in_plain(LOG *log, const CVECTOR *tasks_node_work, UINT32 *index)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0054);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "No. %ld: (tasks node is null)\n", (*index) ++);
            continue;
        }
        tasks_node_print_csocket_cnode_list(log, tasks_node, index);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0055);
    return;
}

EC_BOOL tasks_work_isend_node(CVECTOR *tasks_node_work, const UINT32 des_tcid, const UINT32 msg_tag, TASK_NODE *task_node)
{
    CSOCKET_CNODE  *csocket_cnode;

    TASK_BRD  *task_brd;

    UINT32 msg_len;
    UINT32 pos;

    task_brd = task_brd_default_get();

#if 1
    csocket_cnode = tasks_work_search_csocket_cnode_with_min_load_by_tcid(tasks_node_work, des_tcid);
    if(NULL_PTR == csocket_cnode)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_work_isend_node: des_tcid %s does not exist when (tcid %s,comm %ld,rank %ld,modi %ld) -> (tcid %s,comm %ld,rank %ld,modi %ld)\n",
                        c_word_to_ipv4(des_tcid),
                        TASK_NODE_SEND_TCID_STR(task_node), TASK_NODE_SEND_COMM(task_node), TASK_NODE_SEND_RANK(task_node), TASK_NODE_SEND_MODI(task_node),
                        TASK_NODE_RECV_TCID_STR(task_node), TASK_NODE_RECV_COMM(task_node), TASK_NODE_RECV_RANK(task_node), TASK_NODE_RECV_MODI(task_node)
                        );

        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "lost route: from (tcid %s,comm %ld,rank %ld,modi %ld) to (tcid %s,comm %ld,rank %ld,modi %ld) with priority %ld, type %ld, tag %ld, seqno %lx.%lx.%lx, subseqno %ld, func id %lx\n",
                        TASK_NODE_SEND_TCID_STR(task_node), TASK_NODE_SEND_COMM(task_node), TASK_NODE_SEND_RANK(task_node), TASK_NODE_SEND_MODI(task_node),
                        TASK_NODE_RECV_TCID_STR(task_node), TASK_NODE_RECV_COMM(task_node), TASK_NODE_RECV_RANK(task_node), TASK_NODE_RECV_MODI(task_node),
                        TASK_NODE_PRIO(task_node), TASK_NODE_TYPE(task_node),
                        TASK_NODE_TAG(task_node),
                        TASK_NODE_SEND_TCID(task_node), TASK_NODE_SEND_RANK(task_node), TASK_NODE_SEQNO(task_node), TASK_NODE_SUB_SEQNO(task_node),
                        TASK_NODE_FUNC_ID(task_node)
                        );

        //super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), RECV_TASKC_ADDR_TCID(recv_taskc_addr));
        super_notify_broken_route(TASK_BRD_SUPER_MD_ID(task_brd), TASK_NODE_SEND_TCID(task_node), TASK_NODE_RECV_TCID(task_node));
        return (EC_FALSE);
    }
#endif

#if (SWITCH_OFF == CBASE64_ENCODE_SWITCH)
    msg_len = TASK_NODE_BUFF_LEN(task_node);
    pos = 0;
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), msg_len, TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &pos);
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), msg_tag, TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &pos);

    CSOCKET_CNODE_LOAD(csocket_cnode) += msg_len;/*increase load info*/
    TASKS_NODE_LOAD(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)) += msg_len;
#endif/*(SWITCH_OFF == CBASE64_ENCODE_SWITCH)*/

    PRINT_BUFF("tasks_work_isend_node: buff = ", TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_POS(task_node));

    /*if data sending is not completed, mount request to list for monitoring*/
    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), task_node);

    return (EC_TRUE);
}

EC_BOOL tasks_work_irecv_node(CVECTOR *tasks_node_work, CLIST *save_to_list)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0056);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        tasks_work_irecv_on_tasks_node(tasks_node_work, tasks_node, save_to_list);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0057);
    return (EC_TRUE);
}

EC_BOOL tasks_work_need_isend_on_csocket_cnode(CVECTOR *tasks_work, CSOCKET_CNODE *csocket_cnode)
{
    CLIST_DATA *clist_data;

    /*send data as much as possible on the csocket_cnode*/
    CLIST_LOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0058);
    CLIST_LOOP_NEXT(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        if( TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0059);
            return (EC_TRUE);
        }
    }

    CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0060);
    return (EC_FALSE);
}

/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   send data to one csocket_cnode until no data or sending incompleted
*
**/
EC_BOOL tasks_work_isend_on_csocket_cnode(CVECTOR *tasks_work, CSOCKET_CNODE *csocket_cnode)
{
    CLIST_DATA *clist_data;

    /*send data as much as possible on the csocket_cnode*/
    CLIST_LOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0061);
    CLIST_LOOP_NEXT(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        if( TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            if(EC_FALSE == csocket_isend_task_node(csocket_cnode, task_node))
            {
                CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0062);
                return (EC_FALSE);
            }
        }

        /*when data sending incomplete, end this sending loop*/
        if( TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            break;
        }

        /*whend data sending complete, rmv csocket_request from SENDING_REQUEST list*/
        else
        {
            CLIST_DATA *clist_data_rmv;

            /*update load info before removal*/
            CSOCKET_CNODE_LOAD(csocket_cnode) -= TASK_NODE_BUFF_LEN(task_node);/*decrease load info*/

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);

            clist_rmv_no_lock(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data_rmv);
            TASK_NODE_COMP(task_node) = TASK_WAS_SENT;
        }
        /*continue next sending*/
    }

    CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0063);
    return (EC_TRUE);
}

EC_BOOL tasks_work_isend_on_csocket_cnode_by_epoll(CSOCKET_CNODE *csocket_cnode)
{
    if(EC_FALSE == tasks_work_isend_on_csocket_cnode(NULL_PTR, csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_isend_on_csocket_cnode_by_epoll: sockfd %d WR events trigger disconnected\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
    
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
        return (EC_FALSE);
    }

    if(EC_TRUE == clist_is_empty(CSOCKET_CNODE_SENDING_LIST(csocket_cnode)))
    {
        /*clear WR event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);    
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_isend_on_csocket_cnode_by_epoll: sockfd %d del WR event done\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));                            
        
    }
    return (EC_TRUE);
}

/**
*
*   internal interface of taskSrv
*
*   1. recv data from one csocket_cnode until no data or cache full
*   2. split recved data into csocket_requests and mount them into INCOMED queue
*      note: if the last left data is a incompleted csocket_request, keep it in cache
*   note: we can retire INCOMING queue design now
*
**/
EC_BOOL tasks_work_irecv_on_csocket_cnode(CVECTOR *tasks_node_work, CSOCKET_CNODE *csocket_cnode, CLIST *save_to_list)
{
    for(;;)
    {
        TASK_NODE  *task_node;

        CTM *last_update_tm;
        CTM *last_end_tm;       

        /*handle incoming tas_node*/
        task_node = CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode);
        if(NULL_PTR != task_node)
        {   
            /*if fix cannot complete the csocket_request, CRBUFF has no data to handle, so terminate*/
            if(EC_FALSE == csocket_fix_task_node(CSOCKET_CNODE_SOCKFD(csocket_cnode), task_node))
            {
                /*terminate*/
                return (EC_TRUE);
            }

            CTIMET_GET(TASKS_NODE_LAST_UPDATE_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));/*update*/

            last_update_tm = CTIMET_TO_TM(TASKS_NODE_LAST_UPDATE_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));
            last_end_tm    = CTIMET_TO_TM(TASKS_NODE_LAST_SEND_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));
            
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG][%s] last update time: %02d:%02d:%02d, last send time: %02d:%02d:%02d, "
                               "updated in tasks_work_irecv_on_csocket_cnode[0]\n",
                                TASKS_NODE_TCID_STR(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)),
                                CTM_HOUR(last_update_tm),
                                CTM_MIN(last_update_tm),
                                CTM_SEC(last_update_tm),

                                CTM_HOUR(last_end_tm),
                                CTM_MIN(last_end_tm ),
                                CTM_SEC(last_end_tm )
                );

            /*otherwise, remove it from INCOMING list and push it to INCOMED list*/
            clist_push_back(save_to_list, (void *)task_node);
            TASK_NODE_COMP(task_node) = TASK_WAS_RECV;
            CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = NULL_PTR;/*clean*/
        }  

        /*handle next task_node*/
        task_node = csocket_fetch_task_node(csocket_cnode);
        if(NULL_PTR == task_node)
        {
            break;
        }

        CTIMET_GET(TASKS_NODE_LAST_UPDATE_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));/*update*/

        last_update_tm = CTIMET_TO_TM(TASKS_NODE_LAST_UPDATE_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));
        last_end_tm    = CTIMET_TO_TM(TASKS_NODE_LAST_SEND_TIME(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)));
        
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG][%s] tasks_work_irecv_on_csocket_cnode: last update time: %02d:%02d:%02d, last send time: %02d:%02d:%02d\n",
                TASKS_NODE_TCID_STR(CSOCKET_CNODE_TASKS_NODE(csocket_cnode)),
                CTM_HOUR(last_update_tm),
                CTM_MIN(last_update_tm),
                CTM_SEC(last_update_tm),

                CTM_HOUR(last_end_tm),
                CTM_MIN(last_end_tm),
                CTM_SEC(last_end_tm)
            );

        if(TASK_NODE_BUFF_POS(task_node) == TASK_NODE_BUFF_LEN(task_node))
        {
            /*push complete csocket_request to INCOMED list*/
            clist_push_back(save_to_list, (void *)task_node);
            TASK_NODE_COMP(task_node) = TASK_WAS_RECV;
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] tasks_work_irecv_on_csocket_cnode: push csocket_request to incomed list\n");
        }
        else
        {
            /*push incomplete csocket_request to INCOMING list*/
            CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = task_node;
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] tasks_work_irecv_on_csocket_cnode: push csocket_request to incoming list\n");
            /*terminate this loop*/
            break;
        }       
    }
    return (EC_TRUE);
}

EC_BOOL tasks_work_irecv_on_csocket_cnode_by_epoll(CSOCKET_CNODE *csocket_cnode)
{
    TASK_BRD *task_brd;
    CLIST *save_to_list;
   
    task_brd     = task_brd_default_get();
    save_to_list = TASK_BRD_QUEUE(task_brd, TASK_RECVING_QUEUE);

    if(EC_FALSE == tasks_work_irecv_on_csocket_cnode(NULL_PTR, csocket_cnode, save_to_list))
    {
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_irecv_on_csocket_cnode_by_epoll: sockfd %d RD events trigger disconnected\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
                            
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
        return (EC_FALSE);
    }
    //dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_irecv_on_csocket_cnode_by_epoll done\n");
    return (EC_TRUE);
}

EC_BOOL tasks_work_isend_on_tasks_node(CVECTOR *tasks_node_work, TASKS_NODE *tasks_node)
{
    UINT32 pos;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0064);
    TASKS_NODE_LOAD(tasks_node) = 0;
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); /*pos ++*/)
    {
        CSOCKET_CNODE *csocket_cnode;
        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            pos ++;/*move forward*/
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_free(csocket_cnode);
            continue;
        }

        /*clean up broken connection*/
        if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            TASK_BRD *task_brd;
            UINT32 broken_tcid;

            dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn: tasks_work_isend_on_tasks_node[1]: %s:%ld found disconnected sockfd %d\n",
                            TASKS_NODE_SRVIPADDR_STR(tasks_node),
                            TASKS_NODE_SRVPORT(tasks_node),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_isend_on_tasks_node[1] call super_notify_broken_tcid\n");
            task_brd = task_brd_default_get();
            super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);

            continue;
        }
        
//#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)
#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
        if(EC_FALSE == tasks_work_isend_on_csocket_cnode(tasks_node_work, csocket_cnode))
        {
            TASK_BRD *task_brd;
            UINT32  broken_tcid;

            dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_work_isend_on_tasks_node: handle sending request on %d failed, close it\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_isend_on_tasks_node[2] call super_notify_broken_tcid\n");
            task_brd = task_brd_default_get();
            super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);

            continue;
        }
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/        
//#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/

//#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)
#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
        if(EC_TRUE == tasks_work_need_isend_on_csocket_cnode(tasks_node_work, csocket_cnode))
        {
            cepoll_set_event(task_brd_default_get_cepoll(), 
                              CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                              CEPOLL_WR_EVENT,
                              (CEPOLL_EVENT_HANDLER)tasks_work_isend_on_csocket_cnode_by_epoll, 
                              (void *)csocket_cnode);               
                                
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_isend_on_tasks_node: sockfd %d set event WR\n",
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));
        }
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/        
//#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/

        TASKS_NODE_LOAD(tasks_node) += CSOCKET_CNODE_LOAD(csocket_cnode);/*update tasks node load*/

        pos ++;/*move forward*/
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0065);
    return (EC_TRUE);
}

EC_BOOL tasks_work_irecv_on_tasks_node(CVECTOR *tasks_node_work, TASKS_NODE *tasks_node, CLIST *save_to_list)
{
    UINT32 pos;

    CVECTOR_LOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0066);
    for(pos = 0; pos < cvector_size(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node)); /*pos ++*/)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
        if(NULL_PTR == csocket_cnode)
        {
            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            pos ++;/*move forward*/
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            TASK_BRD *task_brd;
            UINT32  broken_tcid;

            broken_tcid = TASKS_NODE_TCID(tasks_node);

            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_free(csocket_cnode);

            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_irecv_on_tasks_node[1] call super_notify_broken_tcid\n");
            task_brd = task_brd_default_get();
            super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);

            continue;
        }

        /*clean up broken connection*/
        if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
        {
            TASK_BRD *task_brd;
            UINT32  broken_tcid;

            dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn: tasks_work_irecv_on_tasks_node[1]: %s:%ld found disconnected sockfd %d\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode),
                            CSOCKET_CNODE_SRVPORT(csocket_cnode),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_irecv_on_tasks_node[2] call super_notify_broken_tcid\n");
            task_brd = task_brd_default_get();
            super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);

            continue;
        }

#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
        //dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_work_irecv_on_tasks_node: pos %ld: try to irecv on sockfd %d ...\n", pos, CSOCKET_CNODE_SOCKFD(csocket_cnode));

        if(EC_FALSE == tasks_work_irecv_on_csocket_cnode(tasks_node_work, csocket_cnode, save_to_list))
        {
            TASK_BRD *task_brd;
            UINT32  broken_tcid;

            dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_work_irecv_on_tasks_node: handle recving request on %d failed, close it\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_work_irecv_on_tasks_node[2] call super_notify_broken_tcid\n");
            task_brd = task_brd_default_get();
            super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);

            continue;
        }
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/

        pos ++;/*move forward*/
    }
    CVECTOR_UNLOCK(TASKS_NODE_CSOCKET_CNODE_VEC(tasks_node), LOC_TASKS_0067);
    return (EC_TRUE);
}


/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   1. if find any connection dead body, delete it
*   2. if find any broken connection, delete it
*   3. nonblock send data
*
*   NOTE: HERE WE MUST LOOP TASKS_NODE OR CSOCKET_CNODE BUT NOT CSOCKET_REQUEST!
*   (see comments of tasks_work_isend1)
*
**/
EC_BOOL tasks_work_isend(CVECTOR *tasks_node_work)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0068);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        tasks_work_isend_on_tasks_node(tasks_node_work, tasks_node);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0069);
    return (EC_TRUE);
}

/***********************************************************************************************************************
*
*   WARNING: THIS INTERFACE IS OBSOLETE AND IS WRONG !!!
*
*   we cannot loop csocket_request because when multiple csocket_requests map to the same csocket_cnode,
*   terrible may happen.
*
*   when the csocket_cnode is congested and does not send out the whole data of the previous csocket_request,
*   loop will move forward. if some next csocket_request map to this csocket_cnode, and bingo! at the same time,
*   the csocket_cnode dismiss congestion state, tasks_work_isend1 will start to send data of this next csocket_request,
*   but the previous csocket_request does not complete data sending out! disorder happen!
*
*   hence, the conclusion is we cannot loop csocket_request, but loop tasks_node or csocket_cnode!
*
*
***********************************************************************************************************************/

/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   1. if find any connection dead body, delete it
*   2. if find any broken connection, delete it
*   3. nonblock recv data
*
**/
EC_BOOL tasks_work_irecv(CVECTOR *tasks_node_work, CLIST *save_to_list)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0070);
    /*check all working clients (sockets) one by one*/
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        tasks_work_irecv_on_tasks_node(tasks_node_work, tasks_node, save_to_list);
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0071);
    return (EC_TRUE);
}

EC_BOOL tasks_work_heartbeat_was_trigger(CVECTOR *tasks_node_work)
{
    UINT32 pos;

    TASK_BRD *task_brd;

    task_brd = task_brd_default_get();

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0072);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(TASKS_NODE_TCID(tasks_node) == TASK_BRD_TCID(task_brd))
        {
            continue;
        }

        if(EC_TRUE == tasks_node_was_trigger(task_brd, tasks_node))
        {
            CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0073);
            return (EC_TRUE);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0074);
    
    return (EC_FALSE);
}

EC_BOOL tasks_work_heartbeat(CVECTOR *tasks_node_work)
{
    UINT32 pos;

    TASK_BRD *task_brd;
    TASK_MGR *task_mgr;

    task_brd = task_brd_default_get();

    task_mgr = task_new(NULL_PTR, TASK_PRIO_HIGH, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    CVECTOR_LOCK(tasks_node_work, LOC_TASKS_0075);
    for(pos = 0; pos < cvector_size(tasks_node_work); pos ++)
    {
        TASKS_NODE *tasks_node;

        tasks_node = (TASKS_NODE *)cvector_get_no_lock(tasks_node_work, pos);
        if(NULL_PTR == tasks_node)
        {
            continue;
        }

        if(TASKS_NODE_TCID(tasks_node) == TASK_BRD_TCID(task_brd))
        {
            continue;
        }

        if(EC_FALSE == tasks_node_trigger(task_brd, task_mgr, tasks_node))
        {
            cvector_erase_no_lock(tasks_node_work, pos);
            tasks_node_free(tasks_node);
        }
    }
    CVECTOR_UNLOCK(tasks_node_work, LOC_TASKS_0076);

    task_no_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (EC_TRUE);
}

UINT32 tasks_monitor_count_no_lock(const CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port)
{
    UINT32 pos;
    UINT32 count;

    for(count = 0, pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;
        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(tcid == CSOCKET_CNODE_TCID(csocket_cnode)
        && srv_ipaddr == CSOCKET_CNODE_IPADDR(csocket_cnode)
        && srv_port == CSOCKET_CNODE_SRVPORT(csocket_cnode)
        )
        {
            count ++;
        }
    }

    return (count);
}

UINT32 tasks_monitor_count(const CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port)
{
    UINT32 count;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0077);
    count = tasks_monitor_count_no_lock(tasks_node_monitor, tcid, srv_ipaddr, srv_port);
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0078);

    return (count);
}

/**
*
*   open one client connection to remote server
*   1. connect to remote server with server ip & port info
*   2. add the client connection to client set of server
*   3. add the client to FD SET of server to monitor
*
**/
EC_BOOL tasks_monitor_open(CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port)
{
    CSOCKET_CNODE *csocket_cnode;
    int client_sockfd;

    if(EC_FALSE == csocket_client_start(srv_ipaddr, srv_port, CSOCKET_IS_NONBLOCK_MODE, &client_sockfd))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_monitor_open: failed to connect server %s:%ld\n",
                        c_word_to_ipv4(srv_ipaddr), srv_port);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_is_connected(client_sockfd))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_monitor_open: socket %d to server %s:%ld is not connected\n",
                        client_sockfd, c_word_to_ipv4(srv_ipaddr), srv_port);
        csocket_client_end(client_sockfd);
        return (EC_FALSE);
    }

    if(do_log(SEC_0121_TASKS, 5))
    {
        sys_log(LOGSTDOUT, "[DEBUG] tasks_monitor_open: client tcp stat:\n");
        csocket_tcpi_stat_print(LOGSTDOUT, client_sockfd);
    }

    csocket_cnode = csocket_cnode_new(tcid, client_sockfd, srv_ipaddr, srv_port);/*client save remote server ipaddr and srvport info*/
    if(NULL_PTR == csocket_cnode)
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_monitor_open:new csocket cnode failed\n");
        csocket_client_end(client_sockfd);
        return (EC_FALSE);
    }
#if 0
#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)

        cepoll_set_event(task_brd_default_get_cepoll(), 
                          CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                          CEPOLL_RD_EVENT,
                          (CEPOLL_EVENT_HANDLER)tasks_monitor_irecv_on_csocket_cnode_by_epoll, 
                          (void *)csocket_cnode);

        cepoll_set_event(task_brd_default_get_cepoll(), 
                          CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                          CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)tasks_monitor_isend_on_csocket_cnode_by_epoll, 
                          (void *)csocket_cnode);

       cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           (CEPOLL_EVENT_HANDLER)csocket_cnode_set_disconnected,
                           (void *)csocket_cnode);                           
                            
    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_open: sockfd %d set event WR\n",
                    CSOCKET_CNODE_SOCKFD(csocket_cnode));
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/    
#endif    

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH/* && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH*/)
#if 0
        cepoll_set_event(task_brd_default_get_cepoll(), 
                          CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                          CEPOLL_RD_EVENT,
                          (CEPOLL_EVENT_HANDLER)tasks_monitor_irecv_on_csocket_cnode_by_epoll, 
                          (void *)csocket_cnode);
#endif                                                    
        cepoll_set_event(task_brd_default_get_cepoll(), 
                          CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                          CEPOLL_WR_EVENT,
                          (CEPOLL_EVENT_HANDLER)tasks_monitor_share_taskc_node_epoll, 
                          (void *)csocket_cnode);

       cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                           CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                           (CEPOLL_EVENT_HANDLER)csocket_cnode_set_disconnected,
                           (void *)csocket_cnode);                           
                            
    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_open: sockfd %d set event WR\n",
                    CSOCKET_CNODE_SOCKFD(csocket_cnode));
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/    

    dbg_log(SEC_0121_TASKS, 2)(LOGSTDOUT, "tasks_monitor_open: client sockfd %d is connecting to server %s:%ld\n",
                        client_sockfd, c_word_to_ipv4(srv_ipaddr), srv_port);

    cvector_push(tasks_node_monitor, (void *)csocket_cnode);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_share_taskc_node_epoll(CSOCKET_CNODE *csocket_cnode)
{
    TASK_BRD  *task_brd;
    TASKS_CFG *tasks_cfg;
    CVECTOR   *tasks_node_monitor;

    task_brd  = task_brd_default_get();
    tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

    tasks_node_monitor = TASKS_CFG_MONITOR(tasks_cfg);
        
    tasks_monitor_share_taskc_node(tasks_node_monitor, csocket_cnode);

    tasks_monitor_isend_on_csocket_cnode_by_epoll(csocket_cnode);

    return (EC_TRUE);
}

/*share current taskcomm info to remote taskcomm which is connected via csocket_cnode*/
EC_BOOL tasks_monitor_share_taskc_node(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode)
{
    TASK_NODE *task_node;

    UINT8 *data_buff;
    UINT32 data_num;
    UINT32 pos;

    UINT32 data_tag;

    TASK_BRD *task_brd;

    task_brd = task_brd_default_get();

    data_tag = 0;
    data_num = csocket_encode_actual_size() + xmod_node_encode_actual_size();

    task_node = task_node_new(data_num, LOC_TASKS_0079);
    TASK_NODE_TAG(task_node) = TAG_TASK_REQ;/*trick on task_node_free*/

    data_buff = TASK_NODE_BUFF(task_node);

    pos = 0;
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), data_num, data_buff, data_num, &pos);           /*no useful info*/
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), data_tag, data_buff, data_num, &pos);           /*no useful info*/

    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), TASK_BRD_TCID(task_brd), data_buff, data_num, &pos);/*payload info*/
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), TASK_BRD_COMM(task_brd), data_buff, data_num, &pos);/*payload info*/
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), TASK_BRD_SIZE(task_brd), data_buff, data_num, &pos);/*payload info*/
    cmpi_encode_uint32(TASK_BRD_COMM(task_brd), TASK_BRD_PORT(task_brd), data_buff, data_num, &pos);/*payload info*/

#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
    csocket_isend_task_node(csocket_cnode, task_node);/*send on socket directly*/

    /*when data sending incomplete, end this sending loop*/
    if(TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
    {
        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_share_taskc_node: task_node push to SENDING Queue on sockfd %d\n",
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));
        /*when sending on socket does not complete, add request to monitor list*/
        clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), (void *)task_node);
    }

    /*when data sending complete*/
    else
    {
        CSOCKET_CNODE_STATUS(csocket_cnode) |= CSOCKET_CNODE_SENT_TASKC_NODE;
        dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_share_taskc_node: task_node sent out on sockfd %d, status = %ld\n",
                        CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_STATUS(csocket_cnode));
        /*when sending on socket complete, free request now*/
        task_node_free(task_node);
    }
    
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)
    cepoll_set_event(task_brd_default_get_cepoll(), 
                      CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                      CEPOLL_RD_EVENT,
                      (CEPOLL_EVENT_HANDLER)tasks_monitor_irecv_on_csocket_cnode_by_epoll, 
                      (void *)csocket_cnode);

    cepoll_set_event(task_brd_default_get_cepoll(), 
                      CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                      CEPOLL_WR_EVENT,
                      (CEPOLL_EVENT_HANDLER)tasks_monitor_isend_on_csocket_cnode_by_epoll, 
                      (void *)csocket_cnode);

    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_share_taskc_node: sockfd %d set event RD & WR\n",
                    CSOCKET_CNODE_SOCKFD(csocket_cnode));

    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_share_taskc_node: task_node push to SENDING Queue on sockfd %d\n",
                    CSOCKET_CNODE_SOCKFD(csocket_cnode));
    /*when sending on socket does not complete, add request to monitor list*/
    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), (void *)task_node);
                       
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)
    /*when sending on socket does not complete, add request to monitor list*/
    clist_push_back(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), (void *)task_node);
                       
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/

    return (EC_TRUE);
}

EC_BOOL tasks_monitor_xchg_taskc_node(CVECTOR *tasks_node_monitor)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0080);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            UINT32  broken_tcid;

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_set_no_lock(tasks_node_monitor, pos, NULL_PTR);
            csocket_cnode_free(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_monitor_xchg_taskc_node call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }

            continue;
        }

        /*make sure socket enter TCP_ESTABLISHED state*/
        if(EC_FALSE == csocket_is_established(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            continue;
        }

        if(0 == (CSOCKET_CNODE_SENT_TASKC_NODE & CSOCKET_CNODE_STATUS(csocket_cnode)))
        {
            tasks_monitor_share_taskc_node(tasks_node_monitor, csocket_cnode);
        }
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0081);
    return (EC_TRUE);
}

CSOCKET_CNODE *tasks_monitor_search_csocket_cnode_by_sockfd(CVECTOR *tasks_node_monitor, const int sockfd)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0082);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        if(sockfd == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0083);
            return (csocket_cnode);
        }
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0084);
    return (NULL_PTR);
}

/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   send data to one csocket_cnode until no data or sending incompleted
*
**/
EC_BOOL tasks_monitor_isend_on_csocket_cnode(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode)
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0085);

    /*send data as much as possible on the csocket_cnode*/
    CLIST_LOOP_NEXT(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        if(TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
        {
            /*try to send the left data*/
            if(EC_FALSE == csocket_isend_task_node(csocket_cnode, task_node))
            {
                dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode: sockfd %d send failed\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));
            
                /*when found error*/
                CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0086);
                return (EC_FALSE);
            }

            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode: sockfd %d send pos %ld len %ld\n", 
                    CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF_POS(task_node), TASK_NODE_BUFF_LEN(task_node));
            

            /*when data sending incomplete, end this sending loop, because it indicates csocket_cnode is congested*/
            if(TASK_NODE_BUFF_POS(task_node) != TASK_NODE_BUFF_LEN(task_node))
            {
                break;
            }

            /*whend data sending complete, rmv csocket_request from SENDING_REQUEST list*/
            else
            {
                CLIST_DATA *clist_data_rmv;

                clist_data_rmv = clist_data;
                clist_data = CLIST_DATA_PREV(clist_data);
                clist_rmv_no_lock(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), clist_data_rmv);
                task_node_free(task_node);

                CSOCKET_CNODE_STATUS(csocket_cnode) |= CSOCKET_CNODE_SENT_TASKC_NODE;
            }

            /*continue next sending*/
        }
    }
    CLIST_UNLOCK(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), LOC_TASKS_0087);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_isend_on_csocket_cnode_by_epoll(CSOCKET_CNODE *csocket_cnode)
{
    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d WR events triggered\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));

    if(EC_FALSE == tasks_monitor_isend_on_csocket_cnode(NULL_PTR, csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d WR events trigger disconnected\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
    
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
        return (EC_FALSE);
    }

    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d, status %ld\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_STATUS(csocket_cnode));

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)    
    if(0 == (CSOCKET_CNODE_SENT_TASKC_NODE & CSOCKET_CNODE_STATUS(csocket_cnode)))
    {
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT,
                         (CEPOLL_EVENT_HANDLER)tasks_monitor_isend_on_csocket_cnode_by_epoll, 
                         (void *)csocket_cnode);
    
        return (EC_TRUE);
    }
    
    /*clear WR event*/
    cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);
    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d del WR event done\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));

    if(0 == (CSOCKET_CNODE_RCVD_TASKC_NODE & CSOCKET_CNODE_STATUS(csocket_cnode)))
    {
        cepoll_set_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT,
                         (CEPOLL_EVENT_HANDLER)tasks_monitor_irecv_on_csocket_cnode_by_epoll, 
                         (void *)csocket_cnode);
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d set RD event done\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));     
    }
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_OFF == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/    

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)    
    if(CSOCKET_CNODE_SENT_TASKC_NODE & CSOCKET_CNODE_STATUS(csocket_cnode))
    {
        /*clear WR event*/
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_WR_EVENT);
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_isend_on_csocket_cnode_by_epoll: sockfd %d set RD event done\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));             
    }
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH && SWITCH_ON == CROUTINE_SUPPORT_CTHREAD_SWITCH)*/    
    return (EC_TRUE);
}

/**
*
*   internal interface of taskSrv
*
*   1. recv data from one csocket_cnode until no data or cache full
*   2. split recved data into csocket_requests and mount them into INCOMED queue
*      note: if the last left data is a incompleted csocket_request, keep it in cache
*   note: we can retire INCOMING queue design now
*
**/
EC_BOOL tasks_monitor_irecv_on_csocket_cnode(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode)
{
    TASK_NODE  *task_node;

    if(NULL_PTR != CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv_on_csocket_cnode: ip %s port %ld, tcid %s, CSOCKET_CNODE_INCOMED_TASK_NODE is not null\n",
                        CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode), CSOCKET_CNODE_TCID_STR(csocket_cnode));

        return (EC_TRUE);
    }

    /*handle incoming tas_node*/
    task_node = CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode);
    if(NULL_PTR != task_node)
    {
        /*if fix cannot complete the csocket_request, CRBUFF has no data to handle, so terminate*/
        if(EC_FALSE == csocket_fix_task_node(CSOCKET_CNODE_SOCKFD(csocket_cnode), task_node))
        {
            /*terminate*/
            return (EC_TRUE);
        }

        /*otherwise, remove it from INCOMING list and push it to INCOMED list*/
        TASK_NODE_COMP(task_node) = TASK_WAS_RECV;
        CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode)  = task_node;
        CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = NULL_PTR;        

        return (EC_TRUE);/*xxx*/
    }  

    /*handle next task_node*/
    task_node = csocket_fetch_task_node(csocket_cnode);
    if(NULL_PTR == task_node)
    {
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] [2] tasks_monitor_irecv_on_csocket_cnode: task_node is null\n");

        //break;
    }
    else
    {
        TASK_NODE_TAG(task_node) = TAG_TASK_REQ;/*trick on task_node_free*/
        if(TASK_NODE_BUFF_POS(task_node) == TASK_NODE_BUFF_LEN(task_node))
        {
            /*push complete csocket_request to INCOMED list*/
            TASK_NODE_COMP(task_node) = TASK_WAS_RECV;
            CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode)  = task_node;
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] [3] tasks_monitor_irecv_on_csocket_cnode: pos %ld, len %ld => incomed\n",
                            TASK_NODE_BUFF_POS(task_node), TASK_NODE_BUFF_LEN(task_node));
            //break;
        }
        else
        {
            /*push incomplete csocket_request to INCOMING list*/
            CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = task_node;
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] [4] tasks_monitor_irecv_on_csocket_cnode: pos %ld, len %ld => incoming\n",
                            TASK_NODE_BUFF_POS(task_node), TASK_NODE_BUFF_LEN(task_node));

            /*terminate this loop*/
            //break;
        }
    }     

    return (EC_TRUE);
}

EC_BOOL tasks_monitor_irecv_on_csocket_cnode_by_epoll(CSOCKET_CNODE *csocket_cnode)
{  
    dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv_on_csocket_cnode_by_epoll: sockfd %d RD events triggered\n", 
                        CSOCKET_CNODE_SOCKFD(csocket_cnode));

    if(EC_FALSE == tasks_monitor_irecv_on_csocket_cnode(NULL_PTR, csocket_cnode))
    {
        dbg_log(SEC_0121_TASKS, 0)(LOGSTDOUT, "error:tasks_monitor_irecv_on_csocket_cnode_by_epoll: sockfd %d RD events trigger disconnected\n", 
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
                            
        CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode);
        return (EC_FALSE);
    }

    if(NULL_PTR != CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode))
    {
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode), CEPOLL_RD_EVENT);
        tasks_monitor_incomed_one(csocket_cnode);
    }

    if(CSOCKET_CNODE_XCHG_TASKC_NODE == CSOCKET_CNODE_STATUS(csocket_cnode))
    {
        TASK_BRD  *task_brd;
        TASKS_CFG *tasks_cfg;
        CVECTOR   *tasks_node_monitor;
        CVECTOR   *tasks_node_work;
        UINT32     pos;

        task_brd  = task_brd_default_get();
        tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);

        tasks_node_monitor = TASKS_CFG_MONITOR(tasks_cfg);
        tasks_node_work    = TASKS_CFG_WORK(tasks_cfg);

        CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0088);
        for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
        {
            if(csocket_cnode != (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos))
            {
                continue;
            }
            
            /*move monitor client to work client list*/
            if(CSOCKET_CNODE_XCHG_TASKC_NODE == CSOCKET_CNODE_STATUS(csocket_cnode))
            {  
                cvector_erase_no_lock(tasks_node_monitor, pos);
                tasks_work_add_csocket_cnode(tasks_node_work, csocket_cnode);

                task_brd_rank_load_tbl_push_all(task_brd, CSOCKET_CNODE_TCID(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode));

                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_move_to_work: move csocket cnode tcid %s comm %ld size %ld sockfd %d ipaddr %s port %ld to work\n",
                                    CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                                    CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode)
                                    );
            
                cepoll_set_event(task_brd_default_get_cepoll(), 
                                  CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  CEPOLL_RD_EVENT,
                                  (CEPOLL_EVENT_HANDLER)tasks_work_irecv_on_csocket_cnode_by_epoll, 
                                  (void *)csocket_cnode);

               cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                                   CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                   (CEPOLL_EVENT_HANDLER)csocket_cnode_set_disconnected,
                                   (void *)csocket_cnode);

                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv_on_csocket_cnode_by_epoll: move csocket cnode tcid %s comm %ld size %ld sockfd %d ipaddr %s port %ld to work\n",
                                    CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                                    CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode)
                                    );
                break;
            }
        }
        CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0089);                       
    }
    
    return (EC_TRUE);
}

/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   1. if find any connection dead body, delete it
*   2. if find any broken connection, delete it
*   3. nonblock send data
*
**/
EC_BOOL tasks_monitor_isend(CVECTOR *tasks_node_monitor)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0090);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            cvector_set_no_lock(tasks_node_monitor, pos, NULL_PTR);
            csocket_cnode_free(csocket_cnode);
            continue;
        }

        /*clean up broken connection*/
        if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            UINT32 broken_tcid;

            dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn: tasks_monitor_isend[2]: %s:%ld found disconnected sockfd %d\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode),
                            CSOCKET_CNODE_SRVPORT(csocket_cnode),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_set_no_lock(tasks_node_monitor, pos, NULL_PTR);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_monitor_isend[3] call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }

            continue;
        }
#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
        if(EC_FALSE == tasks_monitor_isend_on_csocket_cnode(tasks_node_monitor, csocket_cnode))
        {
            UINT32 broken_tcid;

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_set_no_lock(tasks_node_monitor, pos, NULL_PTR);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_monitor_isend[5] call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }
        }
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/        
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0091);
    return (EC_TRUE);
}

/**
*
*   internal interface of taskSrv
*
*   handle message to server
*   1. if find any connection dead body, delete it
*   2. if find any broken connection, delete it
*   3. nonblock recv data
*
**/
EC_BOOL tasks_monitor_irecv(CVECTOR *tasks_node_monitor)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0092);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); /*pos ++*/)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            cvector_erase_no_lock(tasks_node_monitor, pos);
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv: csocket_cnode is null at pos %ld\n", pos);
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv: csocket_cnode sockfd is any at pos %ld\n", pos);
            pos ++;/*move forward*/
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            dbg_log(SEC_0121_TASKS, 9)(LOGSTDOUT, "[DEBUG] tasks_monitor_irecv: csocket_cnode sockfd is error at pos %ld\n", pos);
            cvector_erase_no_lock(tasks_node_monitor, pos);
            csocket_cnode_free(csocket_cnode);
            continue;
        }

        dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] tasks_monitor_irecv: check csocket_cnode sockfd at pos %ld\n", pos);

        /*clean up broken connection*/
        if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            UINT32 broken_tcid;

            dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn: tasks_monitor_irecv[4]: %s:%ld found disconnected sockfd %d\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode),
                            CSOCKET_CNODE_SRVPORT(csocket_cnode),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(tasks_node_monitor, pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_monitor_irecv[5] call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }

            continue;
        }

        dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] tasks_monitor_irecv: check csocket_cnode sockfd is ok at pos %ld\n", pos);
        
#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
        if(EC_FALSE == tasks_monitor_irecv_on_csocket_cnode(tasks_node_monitor, csocket_cnode))
        {
            UINT32 broken_tcid;

            dbg_log(SEC_0121_TASKS, 0)(LOGSTDERR, "error:tasks_monitor_irecv[7]: handle recving request on %d failed, close it\n",
                                CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(tasks_node_monitor, pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "tasks_monitor_irecv[8] call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }
            continue;
        }
        dbg_log(SEC_0121_TASKS, 9)(LOGSTDNULL, "[DEBUG] tasks_monitor_irecv: csocket_cnode irecv ok at pos %ld\n", pos);
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/        

        pos ++;/*move forward*/
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0093);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_incomed_one(CSOCKET_CNODE *csocket_cnode)
{
    TASK_BRD  *task_brd;
    TASK_NODE *task_node;

    UINT8 *data_buff;
    UINT32 data_num;

    UINT32   discard_data_num;
    UINT32   discard_data_tag;
    UINT32   position;

    task_brd = task_brd_default_get();
    
    task_node = CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode);
    if(NULL_PTR == task_node)
    {
        return (EC_TRUE);
    }

    data_buff = TASK_NODE_BUFF(task_node);
    data_num  = TASK_NODE_BUFF_LEN(task_node);

    position = 0;

    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(discard_data_num));
    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(discard_data_tag));

    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(CSOCKET_CNODE_TCID(csocket_cnode)));
    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(CSOCKET_CNODE_COMM(csocket_cnode)));
    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(CSOCKET_CNODE_SIZE(csocket_cnode)));
    cmpi_decode_uint32(TASK_BRD_COMM(task_brd), data_buff, data_num, &position, &(CSOCKET_CNODE_SRVPORT(csocket_cnode)));

    CSOCKET_CNODE_STATUS(csocket_cnode) |= CSOCKET_CNODE_RCVD_TASKC_NODE;
    dbg_log(SEC_0121_TASKS, 5)(LOGSTDNULL, "tasks_monitor_incomed_one: csocket_cnode recved on sockfd %d, status = %ld\n",
                    CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_STATUS(csocket_cnode));

    dbg_log(SEC_0121_TASKS, 5)(LOGSTDNULL, "tasks_monitor_incomed_one: data_num = %ld, position = %ld\n", data_num, position);                        

    CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode) = NULL_PTR;
    task_node_free(task_node);

    return (EC_TRUE);
}

EC_BOOL tasks_monitor_incomed(CVECTOR *tasks_node_monitor)
{
    TASK_BRD   *task_brd;
    UINT32 pos;

    task_brd = task_brd_default_get();

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0094);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;
        
        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        tasks_monitor_incomed_one(csocket_cnode);
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0095);

    return (EC_TRUE);
}

EC_BOOL tasks_monitor_move_to_work(CVECTOR *tasks_node_monitor, CVECTOR *tasks_node_work)
{
    UINT32 pos;

    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0096);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); /*pos ++*/)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            cvector_erase_no_lock(tasks_node_monitor, pos);
            continue;
        }

        /*skip current taskcomm itself*/
        if(CMPI_ANY_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            pos ++; /*move forward*/
            continue;
        }

        /*clean up dead body*/
        if(CMPI_ERROR_SOCKFD == CSOCKET_CNODE_SOCKFD(csocket_cnode))
        {
            cvector_erase_no_lock(tasks_node_monitor, pos);
            csocket_cnode_free(csocket_cnode);
            continue;
        }

        /*skip monitor socket which is not complete taskc_node exchanging*/
        if(CMPI_ERROR_TCID == CSOCKET_CNODE_TCID(csocket_cnode))
        {
            pos ++; /*move forward*/
            continue;
        }

        /*clean up broken connection*/
        if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
        {
            UINT32 broken_tcid;

            dbg_log(SEC_0121_TASKS, 1)(LOGSTDOUT, "warn: tasks_monitor_move_to_work[1]: %s:%ld found disconnected sockfd %d\n",
                            CSOCKET_CNODE_IPADDR_STR(csocket_cnode),
                            CSOCKET_CNODE_SRVPORT(csocket_cnode),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));

            broken_tcid = CSOCKET_CNODE_TCID(csocket_cnode);

            cvector_erase_no_lock(tasks_node_monitor, pos);
            csocket_cnode_close_and_clean_event(csocket_cnode);

            if(CMPI_ANY_TCID != broken_tcid && CMPI_ERROR_TCID != broken_tcid)
            {
                TASK_BRD *task_brd;
                dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_move_to_work[1] call super_notify_broken_tcid\n");
                task_brd = task_brd_default_get();
                super_notify_broken_tcid(TASK_BRD_SUPER_MD_ID(task_brd), broken_tcid);
            }

            continue;
        }

        /*move monitor client to work client list*/
        if(CSOCKET_CNODE_XCHG_TASKC_NODE == CSOCKET_CNODE_STATUS(csocket_cnode))
        {
            TASK_BRD *task_brd;

            cvector_erase_no_lock(tasks_node_monitor, pos);
            tasks_work_add_csocket_cnode(tasks_node_work, csocket_cnode);

            task_brd = task_brd_default_get();
            task_brd_rank_load_tbl_push_all(task_brd, CSOCKET_CNODE_TCID(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode));
            
            dbg_log(SEC_0121_TASKS, 5)(LOGSTDOUT, "[DEBUG] tasks_monitor_move_to_work: move csocket cnode tcid %s comm %ld size %ld sockfd %d ipaddr %s port %ld to work\n",
                                CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                                CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode)
                                );
                                
#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
        
            cepoll_set_event(task_brd_default_get_cepoll(), 
                              CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                              CEPOLL_RD_EVENT,
                              (CEPOLL_EVENT_HANDLER)tasks_work_irecv_on_csocket_cnode_by_epoll, 
                              (void *)csocket_cnode);

           cepoll_set_shutdown(task_brd_default_get_cepoll(), 
                               CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                               (CEPOLL_EVENT_HANDLER)csocket_cnode_set_disconnected,
                               (void *)csocket_cnode);

#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/

            continue;
        }

        pos ++;/*move forward*/
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0097);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_checker(CVECTOR *tasks_node_monitor, CROUTINE_COND *checker_ccond)
{
    if(NULL_PTR != checker_ccond && 0 == cvector_size(tasks_node_monitor))
    {
        croutine_cond_release(checker_ccond, LOC_TASKS_0098);
    }
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_init(CVECTOR *tasks_node_monitor)
{
    cvector_init(tasks_node_monitor, 0, MM_CSOCKET_CNODE, CVECTOR_LOCK_ENABLE, LOC_TASKS_0099);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_clean(CVECTOR *tasks_node_monitor)
{
    cvector_clean(tasks_node_monitor, (CVECTOR_DATA_CLEANER)csocket_cnode_close_and_clean_event, LOC_TASKS_0100);
    return (EC_TRUE);
}

EC_BOOL tasks_monitor_is_empty(const CVECTOR *tasks_node_monitor)
{
    if(EC_TRUE == cvector_is_empty(tasks_node_monitor))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

void tasks_monitor_print(LOG *log, const CVECTOR *tasks_node_monitor)
{
    UINT32 pos;

    /*note: if not lock vector, some potential risk may happen. but if lock vector, log info will be blocked without sending*/
    /*because vector is locked twice and SLAVE thread will not trigger sending*/
    CVECTOR_LOCK(tasks_node_monitor, LOC_TASKS_0101);
    for(pos = 0; pos < cvector_size(tasks_node_monitor); pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get_no_lock(tasks_node_monitor, pos);
        if(NULL_PTR == csocket_cnode)
        {
            continue;
        }

        csocket_cnode_print(log, csocket_cnode);
    }
    CVECTOR_UNLOCK(tasks_node_monitor, LOC_TASKS_0102);
    return ;
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
