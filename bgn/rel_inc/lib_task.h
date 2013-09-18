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

#ifndef _LIB_TASK_H
#define _LIB_TASK_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"

#define CMPI_DBG_RANK      ((UINT32)  0)  /*define debug rank*/
#define CMPI_MON_RANK      ((UINT32)  0)  /*define monitor rank*/
#define CMPI_FWD_RANK      ((UINT32)  0)  /*define forward rank*/
#define CMPI_CDFS_RANK     ((UINT32)  0)  /*define cdfs rank*/

#define CMPI_DBG_TCID_BEG         ((UINT32) 64) /*dbg tcid beg = 0.0.0.64 */
#define CMPI_DBG_TCID_END         ((UINT32) 95) /*dbg tcid beg = 0.0.0.95 */

#define CMPI_MON_TCID_BEG         ((UINT32) 96) /*mon tcid beg = 0.0.0.96 */
#define CMPI_MON_TCID_END         ((UINT32)127) /*mon tcid beg = 0.0.0.127*/

#define CMPI_LOCAL_TCID    task_brd_default_get_tcid()
#define CMPI_LOCAL_COMM    task_brd_default_get_comm()
#define CMPI_LOCAL_RANK    task_brd_default_get_rank()

#define TASK_REGISTER_HSDFS_SERVER          ((UINT32) 0x01)
#define TASK_REGISTER_HSBGT_SERVER          ((UINT32) 0x02)
#define TASK_REGISTER_OTHER_SERVER          ((UINT32) 0x04)
#define TASK_REGISTER_UDP_SERVER            ((UINT32) 0x08)
#define TASK_REGISTER_ALL_SERVER            ((UINT32) 0x0F)

/*LOAD BALANCING CHOICE*/
#define              LOAD_BALANCING_LOOP    ((UINT32)  0)
#define              LOAD_BALANCING_MOD     ((UINT32)  1)
#define              LOAD_BALANCING_QUE     ((UINT32)  2)
#define              LOAD_BALANCING_OBJ     ((UINT32)  3)
#define              LOAD_BALANCING_CPU     ((UINT32)  4)
#define              LOAD_BALANCING_MEM     ((UINT32)  5)
#define              LOAD_BALANCING_DSK     ((UINT32)  6)
#define              LOAD_BALANCING_NET     ((UINT32)  7)
#define              LOAD_BALANCING_END     ((UINT32)  8)

#define              TASK_NEED_RSP_FLAG            ((UINT32)  1) /*flag: task req need task rsp*/
#define              TASK_NOT_NEED_RSP_FLAG        ((UINT32)  2) /*flag: task req NOT need task rsp*/

#define              TASK_NEED_RESCHEDULE_FLAG     ((UINT32)  3) /*flag: task req will be rescheduled when taskcomm broken*/
#define              TASK_NOT_NEED_RESCHEDULE_FLAG ((UINT32)  4) /*flag: task req will NOT be rescheduled when taskcomm broken*/

#define              TASK_NEED_ALL_RSP      ((UINT32) -1) /*need to wait all response come back*/
#define              TASK_NEED_NONE_RSP     ((UINT32)  0) /*not need to wait any response come back*/

#define              TASK_ALWAYS_LIVE       ((UINT32) -1) /*task mgr stay waiting until collected specific num of task rsp*/
#define              TASK_DEFAULT_LIVE      ((UINT32) /*60*/TASK_DEFAULT_LIVE_NSEC) /*task may stay 60s waiting for until collected specific num of task rsp*/

/*TASK PRIORITY*/
#define              TASK_PRIO_NORMAL       ((UINT32)  1) /*FIFO task*/
#define              TASK_PRIO_HIGH         ((UINT32)  2) /*FIFO task, but preempt those normal tasks*/
#define              TASK_PRIO_PREEMPT      ((UINT32)  3) /*LIFO task, preempt any task, including the previous reached PREEMPT tasks*/
#define              TASK_PRIO_UNDEF        ((UINT32) -1) /*FIFO task*/

typedef EC_BOOL (*CHECKER)(const UINT32);

/*default checker*/
EC_BOOL task_default_bool_checker(const EC_BOOL ec_bool);
EC_BOOL task_default_not_null_pointer_checker(const void *pointer);

LOG * task_brd_default_init(int argc, char **argv);

UINT32 task_brd_default_get_tcid();

UINT32 task_brd_default_get_comm();

UINT32 task_brd_default_get_rank();

UINT32 task_brd_default_get_super();

UINT32 task_brd_default_get_ganglia();

CSTRING *task_brd_default_get_hsdfs_np_root_dir();

CSTRING *task_brd_default_get_hsdfs_dn_root_dir();

CSTRING *task_brd_default_get_hsbgt_root_table_dir();

UINT32 task_brd_default_local_taskc();

EC_BOOL task_brd_check_is_dbg_tcid(const UINT32 tcid);

EC_BOOL task_brd_check_is_monitor_tcid(const UINT32 tcid);

EC_BOOL task_brd_check_is_work_tcid(const UINT32 tcid);

EC_BOOL task_brd_default_check_csrv_enabled();
UINT32  task_brd_default_get_csrv_port();

EC_BOOL task_brd_default_check_validity();

EC_BOOL task_brd_default_abort();

EC_BOOL task_brd_default_add_runner(const UINT32 tcid, const UINT32 rank, EC_BOOL (*runner)(void));

EC_BOOL task_brd_default_start_runner();

EC_BOOL do_slave_default();
EC_BOOL do_slave_thread_default();
EC_BOOL do_slave_wait_default();
EC_BOOL do_cmd_default();
EC_BOOL do_mon_default();

EC_BOOL task_brd_default_start_cdfs_srv(const UINT32 cdfs_md_id, const UINT32 cdfs_srv_port);
EC_BOOL task_brd_default_start_csolr_srv(const UINT32 csolr_md_id, const UINT32 csolr_srv_port);
EC_BOOL task_brd_default_start_csrv();
EC_BOOL task_brd_default_start_cfuse(int * argc, char **argv);
EC_BOOL task_brd_default_start_cextsrv(const UINT32 srv_port, const UINT32 thread_num);

/*new a task mgr template without task req*/
void * task_new(const void *mod_mgr, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num);

/*broadcast to all remote mod nodes in mod mgr, ignore load balancing strategy*/
UINT32 task_bcast(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32 func_id, ...);

/*start remote modules by module start entry as task req*/
/*broadcast to all remote mod nodes in mod mgr, deploy load balancing strategy*/
UINT32 task_act(const void *src_mod_mgr, void **des_mod_mgr, const UINT32 time_to_live, const UINT32 mod_num, const UINT32 load_balancing_choice, const UINT32 task_prio, const UINT32 func_id, ...);

/*stop remote modules by module end entry as task req*/
/*broadcast to all remote mod nodes in mod mgr, ignore load balancing strategy*/
UINT32 task_dea(void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 func_id, ...);

/*wait until all task reqs of task mgr are handled and responed(if need rsp) or until all task reqs sending complete(if not need rsp), */
/*then return the calling point to execute*/
EC_BOOL task_wait(void *task_mgr, const UINT32 time_to_live, const UINT32 task_reschedule_flag, CHECKER ret_val_checker);

/*send all task reqs of task mgr without wait, and return the calling point to execute continously.*/
/*task_mgr will free automatically after collect all responses(if need rsp) or after all requests sending complete(if not need rsp)*/
EC_BOOL task_no_wait(void *task_mgr, const UINT32 time_to_live, const UINT32 task_reschedule_flag, CHECKER ret_val_checker);

/*add task req to task mgr, the task req will send to specific recv mod_node without load balancing*/
UINT32 task_super_inc(void *task_mgr, const void  *send_mod_node, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single recv mod_node*/
UINT32 task_super_mono(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single recv mod_node without waiting*/
UINT32 task_super_mono_no_wait(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

UINT32 task_p2p_inc(void *task_mgr, const UINT32 modi, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

UINT32 task_p2p(const UINT32 modi, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

UINT32 task_p2p_no_wait(const UINT32 modi, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const void *recv_mod_node, const void * func_retval_addr, const UINT32 func_id, ...);

/*add task req to task mgr, the task req will send to best mod node of mod mgr of task mgr based on load balancing strategy of mod mgr*/
UINT32 task_inc(void *task_mgr,const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single best mod_node of mod_mgr based on load balancing strategy of mod_mgr*/
UINT32 task_mono(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32 task_reschedule_flag, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single best mod_node of mod_mgr based on load balancing strategy of mod_mgr without waiting*/
UINT32 task_mono_no_wait(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const void * func_retval_addr, const UINT32 func_id, ...);

/*add task req to task mgr, the task req will send to single mod_node of mod_mgr and ignore load balancing strategy of mod_mgr*/
UINT32 task_pos_inc(void *task_mgr, const UINT32 recv_mod_node_pos, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single mod_node of mod_mgr and ignore load balancing strategy of mod_mgr*/
UINT32 task_pos_mono(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32  recv_mod_node_pos, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single mod_node of mod_mgr and ignore load balancing strategy of mod_mgr without waiting*/
UINT32 task_pos_mono_no_wait(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32  recv_mod_node_pos, const void * func_retval_addr, const UINT32 func_id, ...);

/*add task req to task mgr, the task req will send to single taskcomm of mod_mgr and load balancing of mod_nodes of the taskcomm*/
UINT32 task_tcid_inc(void *task_mgr, const UINT32 recv_tcid, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single taskcomm and load balancing of mod_nodes of the taskcomm*/
UINT32 task_tcid_mono(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32 recv_tcid, const void * func_retval_addr, const UINT32 func_id, ...);

/*send task req to single taskcomm and load balancing of mod_nodes of the taskcomm without waiting*/
UINT32 task_tcid_mono_no_wait(const void *mod_mgr, const UINT32 time_to_live, const UINT32 task_prio, const UINT32 task_need_rsp_flag, const UINT32 task_need_rsp_num, const UINT32 recv_tcid, const void * func_retval_addr, const UINT32 func_id, ...);


#endif/* _LIB_TASK_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
