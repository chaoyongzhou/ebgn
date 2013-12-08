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

#ifndef _API_UI_DBG_H
#define _API_UI_DBG_H

#include "api_ui.h"

void api_dbg_init(void);

char* ui_show_mem(API_UI_PARAM * param);
char* ui_show_mem_all(API_UI_PARAM * param);

char*  ui_diag_mem(API_UI_PARAM * param);
char*  ui_diag_mem_all(API_UI_PARAM * param);

char*  ui_clean_mem(API_UI_PARAM * param);
char*  ui_clean_mem_all(API_UI_PARAM * param);

char *ui_switch_log(API_UI_PARAM * param);
char *ui_switch_log_all(API_UI_PARAM * param);

char *ui_enable_to_rank_node(API_UI_PARAM * param);
char *ui_enable_all_to_rank_node(API_UI_PARAM * param);

char *ui_disable_to_rank_node(API_UI_PARAM * param);
char *ui_disable_all_to_rank_node(API_UI_PARAM * param);

char *ui_show_queue(API_UI_PARAM * param);
char *ui_show_queue_all(API_UI_PARAM * param);

char *ui_show_client(API_UI_PARAM * param);
char *ui_show_client_all(API_UI_PARAM * param);

char *ui_show_route(API_UI_PARAM * param);
char *ui_show_route_all(API_UI_PARAM * param);

char *ui_show_thread(API_UI_PARAM * param);
char *ui_show_thread_all(API_UI_PARAM * param);

char* ui_show_rank_node(API_UI_PARAM * param);
char* ui_show_rank_node_all(API_UI_PARAM * param);

char *ui_shutdown_work(API_UI_PARAM * param);
char *ui_shutdown_work_all(API_UI_PARAM * param);

char *ui_shutdown_dbg(API_UI_PARAM * param);
char *ui_shutdown_dbg_all(API_UI_PARAM * param);

char *ui_shutdown_mon(API_UI_PARAM * param);
char *ui_shutdown_mon_all(API_UI_PARAM * param);

char *ui_show_taskcomm(API_UI_PARAM * param);
char *ui_show_taskcomm_all(API_UI_PARAM * param);

char *ui_run_shell(API_UI_PARAM * param);
char *ui_run_shell_all(API_UI_PARAM * param);

char *ui_ping_taskcomm(API_UI_PARAM * param);

char* ui_mon_all(API_UI_PARAM * param);
char* ui_mon_oid(API_UI_PARAM * param);

#endif  /* _API_UI_DBG_H */

#ifdef __cplusplus
}
#endif /* _cplusplus */

