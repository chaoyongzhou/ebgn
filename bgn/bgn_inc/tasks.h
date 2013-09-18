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

#ifndef _TASKS_H
#define _TASKS_H

#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#include "cstring.h"
#include "taskcfg.inc"

#include "csocket.h"
#include "cmutex.h"

//#define TASKS_DBG_ENTER(__func_name__) do{sys_log(LOGSTDOUT, "enter %s\n", __func_name__);}while(0)
//#define TASKS_DBG_LEAVE(__func_name__) do{sys_log(LOGSTDOUT, "leave %s\n", __func_name__);}while(0)

#define TASKS_DBG_ENTER(__func_name__) do{}while(0)
#define TASKS_DBG_LEAVE(__func_name__) do{}while(0)

/*------------------------------------------------- taskcomm server interface -------------------------------------------------*/

EC_BOOL tasks_srv_start(TASKS_CFG *tasks_cfg);

EC_BOOL tasks_srv_end(TASKS_CFG *tasks_cfg);

EC_BOOL tasks_srv_accept(TASKS_CFG *tasks_cfg);

EC_BOOL tasks_srv_select(TASKS_CFG *tasks_cfg, int *ret);

EC_BOOL tasks_srv_handle(TASKS_CFG *tasks_cfg);

EC_BOOL tasks_do_once(TASKS_CFG *tasks_cfg);

EC_BOOL tasks_srv_of_tcid(const TASKS_CFG *tasks_cfg, const UINT32 tcid);

/*------------------------------------------------- TASKS_NODE interface -------------------------------------------------*/
TASKS_NODE *tasks_node_new(const UINT32 srvipaddr, const UINT32 srvport, const UINT32 tcid, const UINT32 comm, const UINT32 size);

EC_BOOL tasks_node_init(TASKS_NODE *tasks_node, const UINT32 srvipaddr, const UINT32 srvport, const UINT32 tcid, const UINT32 comm, const UINT32 size);

EC_BOOL tasks_node_clean(TASKS_NODE *tasks_node);

EC_BOOL tasks_node_free(TASKS_NODE *tasks_node);

EC_BOOL tasks_node_is_connected(const TASKS_NODE *tasks_node);

EC_BOOL tasks_node_is_connected_no_lock(const TASKS_NODE *tasks_node);

UINT32 tasks_node_count_load(const TASKS_NODE *tasks_node);

CSOCKET_CNODE *tasks_node_search_csocket_cnode_with_min_load(const TASKS_NODE *tasks_node);

CSOCKET_CNODE *tasks_node_search_csocket_cnode_by_sockfd(const TASKS_NODE *tasks_node, const int sockfd);

EC_BOOL tasks_node_export_csocket_cnode_vec(const TASKS_NODE *tasks_node, CVECTOR *csocket_cnode_vec);

EC_BOOL tasks_node_is_empty(const TASKS_NODE *tasks_node);

EC_BOOL tasks_node_cmp(const TASKS_NODE *src_tasks_node, const TASKS_NODE *des_tasks_node);

EC_BOOL tasks_node_check(const TASKS_NODE *tasks_node, const CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_node_is_tcid(const TASKS_NODE *src_tasks_node, const UINT32 tcid);

EC_BOOL tasks_node_is_ipaddr(const TASKS_NODE *src_tasks_node, const UINT32 ipaddr);

void    tasks_node_print(LOG *log, const TASKS_NODE *tasks_node);

void    tasks_node_print_csocket_cnode_list(LOG *log, const TASKS_NODE *tasks_node, UINT32 *index);

void    tasks_node_print_in_plain(LOG *log, const TASKS_NODE *tasks_node);

void    tasks_node_sprint(CSTRING *cstring, const TASKS_NODE *tasks_node);

/*------------------------------------------------- TASKS_WORK interface -------------------------------------------------*/
UINT32 tasks_work_count_no_lock(const CVECTOR *tasks_node_work, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port);

UINT32 tasks_work_count(const CVECTOR *tasks_node_work, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port);

EC_BOOL tasks_work_export_csocket_cnode_vec(const CVECTOR *tasks_node_work, CVECTOR *csocket_cnode_vec);

TASKS_NODE *tasks_work_search_tasks_node_by_ipaddr(const CVECTOR *tasks_node_work, const UINT32 ipaddr);

TASKS_NODE *tasks_work_search_tasks_node_by_ipaddr_no_lock(const CVECTOR *tasks_node_work, const UINT32 ipaddr);

TASKS_NODE    *tasks_work_search_tasks_node_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tcid);

TASKS_NODE    *tasks_work_search_tasks_node_by_tcid_no_lock(const CVECTOR *tasks_node_work, const UINT32 tcid);

CSOCKET_CNODE *tasks_work_search_tasks_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tasks_tcid);

CSOCKET_CNODE *tasks_work_search_taskr_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 des_tcid);

CSOCKET_CNODE *tasks_work_search_csocket_cnode_with_min_load_by_tcid(const CVECTOR *tasks_node_work, const UINT32 tcid);

CSOCKET_CNODE *tasks_work_search_csocket_cnode_by_tcid_sockfd(const CVECTOR *tasks_work, const UINT32 tcid, const int sockfd);

UINT32  tasks_work_search_tcid_by_ipaddr(const CVECTOR *tasks_work, const UINT32 ipaddr);

EC_BOOL tasks_work_check_connected_by_tcid(const CVECTOR *tasks_work, const UINT32 tcid);

EC_BOOL tasks_work_check_connected_by_ipaddr(const CVECTOR *tasks_work, const UINT32 ipaddr);

EC_BOOL tasks_work_add_csocket_cnode(CVECTOR *tasks_node_work, CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_work_collect_tcid(const CVECTOR *tasks_node_work, CVECTOR *tcid_vec);

EC_BOOL tasks_work_collect_ipaddr(const CVECTOR *tasks_node_work, CVECTOR *ipaddr_vec);

EC_BOOL tasks_work_init(CVECTOR *tasks_node_work);

EC_BOOL tasks_work_clean(CVECTOR *tasks_node_work);

void    tasks_work_print(LOG *log, const CVECTOR *tasks_node_work);

void    tasks_work_print_in_plain(LOG *log, const CVECTOR *tasks_node_work);

void    tasks_work_print_csocket_cnode_list_in_plain(LOG *log, const CVECTOR *tasks_node_work, UINT32 *index);

EC_BOOL tasks_work_isend_node(CVECTOR *tasks_node_work, const UINT32 des_tcid, const UINT32 msg_tag, TASK_NODE *task_node);

EC_BOOL tasks_work_irecv_node(CVECTOR *tasks_node_work, CLIST *save_to_list);

EC_BOOL tasks_work_isend_on_csocket_cnode(CVECTOR *tasks_work, CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_work_irecv_on_csocket_cnode(CVECTOR *tasks_node_work, CSOCKET_CNODE *csocket_cnode, CLIST *save_to_list);

EC_BOOL tasks_work_isend_on_tasks_node(CVECTOR *tasks_node_work, TASKS_NODE *tasks_node);

EC_BOOL tasks_work_irecv_on_tasks_node(CVECTOR *tasks_node_work, TASKS_NODE *tasks_node, CLIST *save_to_list);

EC_BOOL tasks_work_isend(CVECTOR *tasks_node_work);

EC_BOOL tasks_work_irecv(CVECTOR *tasks_node_work, CLIST *save_to_list);

EC_BOOL tasks_work_heartbeat(CVECTOR *tasks_node_work);


/*------------------------------------------------- TASKS_MONITOR interface -------------------------------------------------*/
UINT32 tasks_monitor_count_no_lock(const CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port);

UINT32 tasks_monitor_count(const CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port);

EC_BOOL tasks_monitor_open(CVECTOR *tasks_node_monitor, const UINT32 tcid, const UINT32 srv_ipaddr, const UINT32 srv_port);

/*share current taskcomm info to remote taskcomm which is connected via csocket_cnode*/
EC_BOOL tasks_monitor_share_taskc_node(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_monitor_xchg_taskc_node(CVECTOR *tasks_node_monitor);

CSOCKET_CNODE *tasks_monitor_search_csocket_cnode_by_sockfd(CVECTOR *tasks_node_monitor, const int sockfd);

EC_BOOL tasks_monitor_isend_on_csocket_cnode(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_monitor_irecv_on_csocket_cnode(CVECTOR *tasks_node_monitor, CSOCKET_CNODE *csocket_cnode);

EC_BOOL tasks_monitor_isend(CVECTOR *tasks_node_monitor);

EC_BOOL tasks_monitor_irecv(CVECTOR *tasks_node_monitor);

EC_BOOL tasks_monitor_incomed(CVECTOR *tasks_node_monitor);

EC_BOOL tasks_monitor_move_to_work(CVECTOR *tasks_node_monitor, CVECTOR *tasks_node_work);

EC_BOOL tasks_monitor_checker(CVECTOR *tasks_node_monitor, CROUTINE_COND *checker_ccond);

EC_BOOL tasks_monitor_init(CVECTOR *tasks_node_monitor);

EC_BOOL tasks_monitor_clean(CVECTOR *tasks_node_monitor);

void tasks_monitor_print(LOG *log, const CVECTOR *tasks_node_monitor);


/*---------------------------------------- external interfaces ----------------------------------------*/
EC_BOOL tasks_work_isend_node(CVECTOR *tasks_node_work, const UINT32 des_tcid, const UINT32 msg_tag, TASK_NODE *task_node);


#endif/*_TASKS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
