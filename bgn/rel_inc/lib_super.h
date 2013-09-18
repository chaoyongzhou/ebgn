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

#ifndef _LIB_SUPER_H
#define _LIB_SUPER_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"


/**
*   for test only
*
*   to query the status of SUPER Module
*
**/
void print_super_status(LOG *log);

/**
*
* start super module
*
**/
UINT32 super_start();

/**
*
* end super module
*
**/
void super_end(const UINT32 super_md_id);

/**
*
* set taskc node info to SUPER module
*
**/
UINT32 super_set_taskc_node(const UINT32 super_md_id, const UINT32 ipaddr, const UINT32 port, const UINT32 taskc_id, const UINT32 taskc_comm, const UINT32 taskc_size);

/**
*
* get taskc node info to SUPER module
*
**/
UINT32 super_get_taskc_node(const UINT32 super_md_id, const UINT32 ipaddr, const UINT32 port, UINT32 *taskc_id, UINT32 *taskc_comm, UINT32 *taskc_size);

/**
*
* include taskc node info to SUPER module
*
**/
UINT32 super_incl_taskc_node(const UINT32 super_md_id, const UINT32 ipaddr, const UINT32 port, const int sockfd, const UINT32 taskc_id, const UINT32 taskc_comm, const UINT32 taskc_size);


/**
*
* exclude taskc node info to SUPER module
*
**/
UINT32 super_excl_taskc_node(const UINT32 super_md_id, const UINT32 taskc_id, const UINT32 taskc_comm);

/**
*
* init taskc node mgr in SUPER module
*
**/
UINT32 super_init_taskc_mgr(const UINT32 super_md_id, void *taskc_mgr);

/**
*
* clean taskc node mgr in SUPER module
*
**/
UINT32 super_clean_taskc_mgr(const UINT32 super_md_id, void *taskc_mgr);

/**
*
* sync taskc node mgr info by SUPER module
*
**/
UINT32 super_sync_taskc_mgr(const UINT32 super_md_id, void *taskc_mgr);

/**
*
*   check taskc node info is shared to current FWD process
*
**/
EC_BOOL super_ensure_taskc_node_sharing(const UINT32 super_md_id, const UINT32 tcid);

/**
*
*   check taskc nodes info is shared to current FWD process
*
**/
EC_BOOL super_ensure_taskc_mgr_sharing(const UINT32 super_md_id, const void *taskc_mgr);

/**
*
* print mem statistics info of current process
*
**/
void super_show_mem(const UINT32 super_md_id, LOG *log);

/**
*
* print mem statistics info of current process
*
**/
void super_show_mem_of_type(const UINT32 super_md_id, const UINT32 type, LOG *log);

/**
*
* diagnostic mem of current process
*
**/
void super_diag_mem(const UINT32 super_md_id, LOG *log);

/**
*
* diagnostic mem of current process
*
**/
void super_diag_mem_of_type(const UINT32 super_md_id, const UINT32 type, LOG *log);

/**
*
* clean mem of current process
*
**/
void super_clean_mem(const UINT32 super_md_id);

/**
*
* breathe mem of current process
*
**/
void super_breathing_mem(const UINT32 super_md_id);

/**
*
* shutdown current taskComm
*
**/
void super_shutdown_taskcomm(const UINT32 super_md_id);

/**
*
* show queues in current taskComm
*
**/
void super_show_queues(const UINT32 super_md_id, LOG *log);

/**
*
* handle broken taskcomm when current taskcomm receive notification
*
**/
void super_handle_broken_taskcomm(const UINT32 super_md_id, const UINT32 broken_tcid);

/**
*
* show work clients of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_work_client(const UINT32 super_md_id, LOG *log);

/**
*
* show num info of threads of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_thread_num(const UINT32 super_md_id, LOG *log);

/**
*
* show route table of tasks_cfg of taskc_cfg of task_brd
*
**/
void super_show_route_table(const UINT32 super_md_id, LOG *log);

/**
*
* output log by SUPER module
*
**/
void super_show_cstring(const UINT32 super_md_id, const UINT32 tcid, const UINT32 rank, const CSTRING *cstring);

/**
*
* switch log off
*
**/
void super_switch_log_off(const UINT32 super_md_id);

/**
*
* switch log on
*
**/
void super_switch_log_on(const UINT32 super_md_id);

/**
*
* wait until current process of current taskComm is ready
*
**/
void super_wait_me_ready(const UINT32 super_md_id);

/**
*
* add route
*
**/
void super_add_route(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 maskr, const UINT32 next_tcid);

/**
*
* del route
*
**/
void super_del_route(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 maskr, const UINT32 next_tcid);

/**
*
* add socket connection
*
**/
void super_add_connection(const UINT32 super_md_id, const UINT32 des_tcid, const UINT32 des_srv_ipaddr, const UINT32 des_srv_port, const UINT32 conn_num);


/**
*
* execute shell command and return output as CSTRING
*
**/
void super_run_shell(const UINT32 super_md_id, const CSTRING *cmd_line, LOG *log);

/**
*
* show rank load which is used for LOAD_BALANCING_RANK
*
**/
void super_show_rank_load(const UINT32 super_md_id, LOG *log);


EC_BOOL super_register_hsbgt_cluster(const UINT32 super_md_id);

EC_BOOL super_register_hsdfs_cluster(const UINT32 super_md_id);

/**
*
* execute shell command and return output as CBYTES
*
**/
void super_exec_shell(const UINT32, const CSTRING *, CBYTES *);

EC_BOOL super_transfer(const UINT32 super_md_id, const CSTRING *src_fname, const UINT32 des_tcid, const CSTRING *des_fname);

/*------------------------------------------------------ test for ict -----------------------------------------------------------------------*/
EC_BOOL super_set_zone_size(const UINT32 super_md_id, const UINT32 obj_zone_size);
EC_BOOL super_load_data(const UINT32 super_md_id);
EC_BOOL super_load_data_all(const UINT32 super_md_id, const UINT32 obj_zone_num);
EC_BOOL super_get_data(const UINT32 super_md_id, const UINT32 obj_id, void *obj_data);
EC_BOOL super_get_data_vec(const UINT32 super_md_id, const void *obj_id_vec, void *obj_data_vec);

EC_BOOL super_print_obj_vec(const UINT32 super_md_id, const void *obj_vec, LOG *log);
EC_BOOL super_print_data(const UINT32 super_md_id, LOG *log);
EC_BOOL super_print_data_all(const UINT32 super_md_id, const UINT32 obj_zone_num, LOG *log);

#endif /*_LIB_SUPER_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

