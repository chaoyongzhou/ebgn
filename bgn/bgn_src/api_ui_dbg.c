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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"

#include "char2int.h"
#include "task.inc"
#include "task.h"
#include "tcnode.h"
#include "cmon.h"
#include "ctimer.h"
#include "log.h"
#include "findex.inc"

#include "api_ui.inc"
#include "api_ui.h"
#include "api_ui_dbg.h"
#include "api_ui_log.h"

#define UI_DEBUG_BUFFER_SIZE 32

void api_dbg_init(void)
{
    API_UI_ELEM *tcid;
    API_UI_ELEM *rank;
    API_UI_ELEM *on_off;
    API_UI_ELEM *oid;
    API_UI_ELEM *times;
    API_UI_ELEM *where;
    API_UI_ELEM *des_rank;
    API_UI_ELEM *light;
    API_UI_ELEM *cmd;

    tcid  = api_ui_arg_tcid("<tcid>"  , "taskComm id format xx.xx.xx.xx");
    rank  = api_ui_arg_num("<rank>"  , "rank range from 0 to size of communicator in one taskComm");
    oid   = api_ui_arg_num("<oid>"   , "oid range from 0 to max oid");
    times = api_ui_arg_num("<times>" , "monitor times");

    des_rank  = api_ui_arg_num("<des_rank>"  , "rank range from 0 to size of communicator in one taskComm");

    on_off = api_ui_arg_list("<on|off>", "On / Off");
    api_ui_arg_list_item(on_off,  "on", SWITCH_ON , "on");
    api_ui_arg_list_item(on_off, "off", SWITCH_OFF, "off");

    light = api_ui_arg_list("<green|red>", "Green / Red");
    api_ui_arg_list_item(light, "green", TASK_RANK_NODE_GREEN_LIGHT, "green");
    api_ui_arg_list_item(light,   "red", TASK_RANK_NODE_RED_LIGHT  , "red");

    where = api_ui_arg_str("<console|log>", "show at local LOGSTDOUT or at remote rank_x_y.log");
    cmd   = api_ui_arg_str("<cmd>", "command line surrended by \"\", e.g., \"ls -l\"");

    /*------------------------------- user help -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show mem {all | tcid <tcid> rank <rank>} at <console|log>\n\
                \t\tshow queue {all | tcid <tcid> rank <rank>} at <console|log>\n\
                \t\tshow thread {all | tcid <tcid> rank <rank>} at <console|log>\n\
                \t\tshow route {all | tcid <tcid>} at <console|log>\n\
                \t\tshow rank node {all | tcid <tcid> rank <rank>} at <console|log>\n\
                \t\tshow client {all | tcid <tcid>} at <console|log>\n\
                \t\tshow taskcomm {all | tcid <tcid>} at <console|log>",
                "show");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "diag mem {all | tcid <tcid> rank <rank>} at <console|log>",
                "diag");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shell <cmd> on {all | tcid <tcid>} at <console|log>",
                "shell");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "ping taskcomm tcid <tcid> at <console|log>",
                "ping");

#if 0/*clean mem has bug, not release it*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "clean mem {all | tcid <tcid> [rank <rank>]}",
                "clean");
#endif

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "mon oid <oid> for <times> times on {all | tcid <tcid> rank <rank>} at <console|log>",
                "mon");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "switch {all | tcid <tcid> rank <rank>} log <on|off>",
                "switch");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shutdown <dbg | mon | work> {all | tcid <tcid>}",
                "shutdown");
#if 0
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "enable {all | tcid <tcid> rank <rank>} to des_rank <des_rank>",
                "enable");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "disable {all | tcid <tcid> rank <rank>} to des_rank <des_rank>",
                "disable");
#endif
    /*------------------------------- show mem -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show mem {all | tcid <tcid> rank <rank>} at <console|log>",
                "show mem");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_mem,
                "show mem tcid <tcid> rank <rank> at <console|log>",
                "show mem tcid %t rank %n at %s",
                tcid, rank, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_mem_all,
                "show mem all at  <console|log>",
                "show mem all at %s",
                where);

    /*------------------------------- diag mem -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "diag mem {all | tcid <tcid> rank <rank>} at <console|log>",
                "diag mem");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_diag_mem,
                "diag mem tcid <tcid> rank <rank> at <console|log>",
                "diag mem tcid %t rank %n at %s",
                tcid, rank, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_diag_mem_all,
                "diag mem all at <console|log>",
                "diag mem all at %s",
                where);

#if 0/*clean mem has bug, not release it*/
    /*------------------------------- clean mem -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "clean mem {all | tcid <tcid> rank <rank>}",
                "clean mem");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_clean_mem,
                "clean mem tcid <tcid> rank <rank>",
                "clean mem tcid %n rank %n",
                tcid, rank);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_clean_mem_all,
                "clean mem all",
                "clean mem all");

#endif

    /*------------------------------- switch log -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "switch all log <on|off>",
                "switch all");

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "switch all log <on|off>",
                "switch all log");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_switch_log_all,
                "switch all log <on|off>",
                "switch all log %l",
                on_off);

    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "switch tcid <tcid> rank <rank> log <on|off>",
                "switch tcid");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_switch_log,
                "switch tcid <tcid> rank <rank> log <on|off>",
                "switch tcid %t rank %n log %l",
                tcid, rank, on_off);

    /*------------------------------- show queue -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show queue {all | tcid <tcid> rank <rank>} at <console|log>",
                "show queue");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_queue,
                "show queue tcid <tcid> rank <rank> at <console|log>",
                "show queue tcid %t rank %n at %s",
                tcid, rank, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_queue_all,
                "show queue all at <console|log>",
                "show queue all at %s",
                where);

    /*------------------------------- show client -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show client {all | tcid <tcid>} at <console|log>",
                "show client");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_client,
                "show client tcid <tcid> at <console|log>",
                "show client tcid %t at %s",
                tcid, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_client_all,
                "show client all at <console|log>",
                "show client all at %s",
                where);

    /*------------------------------- show thread -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show thread {all | tcid <tcid> rank <rank>} at <console|log>",
                "show thread");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_thread,
                "show thread tcid <tcid> rank <rank> at <console|log>",
                "show thread tcid %t rank %n at %s",
                tcid, rank, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_thread_all,
                "show thread all at  <console|log>",
                "show thread all at %s",
                where);

    /*------------------------------- show route -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show route {all | tcid <tcid>} at <console|log>",
                "show route");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_route,
                "show route tcid <tcid> at <console|log>",
                "show route tcid %t at %s",
                tcid, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_route_all,
                "show route all at  <console|log>",
                "show route all at %s",
                where);

    /*------------------------------- show rank node -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show rank node {all | tcid <tcid> rank <rank>} at <console|log>",
                "show rank node");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_rank_node,
                "show rank node tcid <tcid> rank <rank> at <console|log>",
                "show rank node tcid %t rank %n at %s",
                tcid, rank, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_rank_node_all,
                "show rank node all at  <console|log>",
                "show rank node all at %s",
                where);

    /*------------------------------- show taskcomm -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "show taskcomm {all | tcid <tcid>} at <console|log>",
                "show taskcomm");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_taskcomm,
                "show taskcomm {all | tcid <tcid>} at <console|log>",
                "show taskcomm tcid %t at %s",
                tcid, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_show_taskcomm_all,
                "show taskcomm all at <console|log>",
                "show taskcomm all at %s",
                where);

    /*------------------------------- monitor -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "mon oid <oid> for <times> times on {all | tcid <tcid> rank <rank>} at <console|log>",
                "mon oid");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_mon_all,
                "mon oid <oid> for <times> times on {all | tcid <tcid> rank <rank> at <console|log>}",
                "mon oid %n for %n times on all at %s",
                oid, times, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_mon_oid,
                "mon oid <oid> for <times> times on {all | tcid <tcid> rank <rank>} at <console|log>",
                "mon oid %n for %n times on tcid %t rank %n at %s",
                oid, times, tcid, rank, where);

    /*------------------------------- shutdown work -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shutdown work {all | tcid <tcid>}",
                "shutdown work");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_work,
                "shutdown work tcid <tcid>",
                "shutdown work tcid %t",
                tcid);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_work_all,
                "shutdown work all",
                "shutdown work all");

    /*------------------------------- shutdown dbg -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shutdown dbg {all | tcid <tcid>}",
                "shutdown dbg");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_dbg,
                "shutdown dbg tcid <tcid>",
                "shutdown dbg tcid %t",
                tcid);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_dbg_all,
                "shutdown dbg all",
                "shutdown dbg all");

    /*------------------------------- shutdown mon -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shutdown mon {all | tcid <tcid>}",
                "shutdown mon");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_mon,
                "shutdown mon tcid <tcid>",
                "shutdown mon tcid %t",
                tcid);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_shutdown_mon_all,
                "shutdown mon all",
                "shutdown mon all");

    /*------------------------------- shell -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "shell <cmd> on {all | tcid <tcid>} at <console|log>",
                "shell");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_run_shell,
                "shell <cmd> on tcid <tcid> at <console|log>",
                "shell %s on tcid %t at %s",
                cmd, tcid, where);

    api_ui_secure_define(API_UI_SECURITY_USER, ui_run_shell_all,
                "shell <cmd> on all at <console|log>",
                "shell %s on all at %s",
                cmd, where);

    /*------------------------------- ping -------------------------------*/
    api_ui_secure_define(API_UI_SECURITY_USER, NULL_PTR,
                "ping taskcomm tcid <tcid> at <console|log>",
                "ping");

    api_ui_secure_define(API_UI_SECURITY_USER, ui_ping_taskcomm,
                "ping taskcomm tcid <tcid> at <console|log>",
                "ping taskcomm tcid %t at %s",
                tcid, where);
    return;
}

static MOD_MGR *ui_gen_mod_mgr(const UINT32 incl_tcid, const UINT32 incl_rank, const UINT32 excl_tcid, const UINT32 excl_rank, const UINT32 super_md_id)
{
    TASK_BRD  *task_brd;
    TASKC_MGR *taskc_mgr;

    MOD_MGR *mod_mgr;

    task_brd = task_brd_default_get();

    mod_mgr = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);

    taskc_mgr = taskc_mgr_new();
    task_brd_sync_taskc_mgr(task_brd, taskc_mgr);
    mod_mgr_gen_by_taskc_mgr(taskc_mgr, incl_tcid, incl_rank, super_md_id, mod_mgr);
    taskc_mgr_free(taskc_mgr);

    mod_mgr_excl(excl_tcid, CMPI_ANY_COMM, excl_rank, super_md_id, mod_mgr);
    return (mod_mgr);
}

static LOG *ui_get_log(const char *where)
{
    if(0 == strcmp("log", where))
    {
        return (LOGSTDOUT);
    }

    if(0 == strcmp("console", where))
    {
        return (LOGCONSOLE);
    }
    return (LOGSTDNULL);
}

char* ui_show_mem(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;

    LOG *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "tcid = %d, rank = %d, where = %s\n", tcid, rank, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_mem beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_mem end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0032);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_mem, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0033);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_show_mem_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;
    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "enter ui_show_mem_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_mem_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_mem_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0034);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();
        cvector_push(report_vec, (void *)log);

        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_mem, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0035);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char*  ui_diag_mem(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "tcid = %d, rank = %d, where = %s\n", tcid, rank, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_diag_mem beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_diag_mem end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0036);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_diag_mem, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0037);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_diag_mem_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;
    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "enter ui_diag_mem_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_diag_mem_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_diag_mem_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0038);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_diag_mem, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0039);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char*  ui_clean_mem(API_UI_PARAM * param)
{
    int tcid;
    int rank;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);

    sys_log(LOGSTDOUT, "tcid = %d, rank = %d\n", tcid, rank);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_clean_mem beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_clean_mem end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_clean_mem, ERR_MODULE_ID);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_clean_mem_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    sys_log(LOGSTDOUT, "enter ui_clean_mem_all\n");

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_clean_mem_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_clean_mem_all end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_clean_mem, ERR_MODULE_ID);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char *ui_switch_log(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    int on_off;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_int(param, 3, &on_off);

    sys_log(LOGSTDOUT, "switch tcid %s, rank %d log to %d\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), rank, on_off);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_switch_log beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_switch_log end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        if(SWITCH_OFF == on_off)/*off*/
        {
            task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_log_off, ERR_MODULE_ID);
        }
        else
        {
            task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_log_on, ERR_MODULE_ID);
        }
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_switch_log_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    int on_off;

    api_ui_param_int(param, 1, &on_off);

    sys_log(LOGSTDOUT, "switch all log to %d\n", on_off);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_switch_log_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_switch_log_all end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        if(SWITCH_OFF == on_off)/*off*/
        {
            task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_log_off, ERR_MODULE_ID);
        }
        else
        {
            task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_log_on, ERR_MODULE_ID);
        }
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_enable_to_rank_node(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    int des_rank;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_int(param, 3, &des_rank);

    sys_log(LOGSTDOUT, "enable tcid %s rank %d to rank %d\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), rank, des_rank);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_enable_to_rank_node beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_enable_to_rank_node end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_rank_node_green, ERR_MODULE_ID, INT32_TO_UINT32(des_rank));
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_enable_all_to_rank_node(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    int des_rank;

    api_ui_param_int(param, 1, &des_rank);

    sys_log(LOGSTDOUT, "disable all to rank %d\n", des_rank);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_enable_all_to_rank_node beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_enable_all_to_rank_node end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_rank_node_green, ERR_MODULE_ID, INT32_TO_UINT32(des_rank));
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_disable_to_rank_node(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    int des_rank;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_int(param, 3, &des_rank);

    sys_log(LOGSTDOUT, "disable tcid %s rank %d to rank %d\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), rank, des_rank);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_disable_to_rank_node beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_disable_to_rank_node end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_rank_node_red, ERR_MODULE_ID, INT32_TO_UINT32(des_rank));
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_disable_all_to_rank_node(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    int des_rank;

    api_ui_param_int(param, 1, &des_rank);

    sys_log(LOGSTDOUT, "disable all to rank %d\n", des_rank);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_disable_all_to_rank_node beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_disable_all_to_rank_node end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_switch_rank_node_red, ERR_MODULE_ID, INT32_TO_UINT32(des_rank));
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}


char *ui_show_queue(API_UI_PARAM * param)
{
    int tcid;
    int rank;

    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "show queue tcid %s, rank %d, where = %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), rank, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_queue beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_queue end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0040);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_queues, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0041);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_show_queue_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "switch ui_show_queue_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_queue_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_queue_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0042);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_queues, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0043);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_show_client(API_UI_PARAM * param)
{
    int tcid;

    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_str(param, 2, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "show work client tcid %s where %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_client beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_client end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0044);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_work_client, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0045);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_show_client_all(API_UI_PARAM * param)
{
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "show work client all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_client_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_client_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0046);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_work_client, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0047);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char* ui_show_thread(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "tcid = %d, rank = %d, where = %s\n", tcid, rank, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_thread beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_thread end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0048);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_thread_num, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0049);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_show_thread_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;
    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "enter ui_show_thread_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_thread_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_thread_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0050);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_thread_num, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0051);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char* ui_show_route(API_UI_PARAM * param)
{
    int tcid;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_str(param, 2, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "tcid = %d, where = %s\n", tcid, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_route beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_route end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0052);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_route_table, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0053);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_show_route_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;
    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "enter ui_show_route_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_route_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_route_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0054);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_route_table, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0055);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char* ui_show_rank_node(API_UI_PARAM * param)
{
    int tcid;
    int rank;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG *des_log;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_int(param, 2, &rank);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "tcid = %d, rank = %d, where = %s\n", tcid, rank, where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), INT32_TO_UINT32(rank), CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_rank_node beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_rank_node end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0056);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_rank_node, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0057);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);
}

char* ui_show_rank_node_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;
    char where[UI_DEBUG_BUFFER_SIZE];

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "enter ui_show_rank_node_all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_rank_node_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_rank_node_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0058);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_show_rank_node, ERR_MODULE_ID, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0059);
    mod_mgr_free(mod_mgr);

    return(NULL_PTR);

}

char *ui_shutdown_work(API_UI_PARAM * param)
{
    int tcid;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);

    sys_log(LOGSTDOUT, "shutdown work tcid %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));

    if(EC_TRUE == task_brd_check_is_dbg_tcid(INT32_TO_UINT32(tcid)))
    {
        sys_log(LOGSTDOUT, "error:tcid = %d is debug taskcomm\n", tcid);
        return (NULL_PTR);
    }

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_work beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_work end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_shutdown_work_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    sys_log(LOGSTDOUT, "shutdown work all\n");

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
    mod_mgr_excl(CMPI_ANY_DBG_TCID, CMPI_ANY_COMM, CMPI_FWD_RANK, CMPI_ANY_MODI, mod_mgr);
    mod_mgr_excl(CMPI_ANY_MON_TCID, CMPI_ANY_COMM, CMPI_FWD_RANK, CMPI_ANY_MODI, mod_mgr);

#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_work_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_work_all end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_shutdown_dbg(API_UI_PARAM * param)
{
    int tcid;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);

    sys_log(LOGSTDOUT, "shutdown dbg tcid %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_dbg beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_dbg end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_shutdown_dbg_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    sys_log(LOGSTDOUT, "shutdown dbg all\n");

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_DBG_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ANY_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_dbg_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_dbg_all end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_shutdown_mon(API_UI_PARAM * param)
{
    int tcid;

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    api_ui_param_int(param, 1, &tcid);

    sys_log(LOGSTDOUT, "shutdown mon tcid %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_mon beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_mon end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_shutdown_mon_all(API_UI_PARAM * param)
{
    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    sys_log(LOGSTDOUT, "shutdown mon all\n");

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_MON_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ANY_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_mon_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_shutdown_mon_all end ----------------------------------\n");
#endif
#if 1
    task_mgr = task_new(mod_mgr, TASK_PRIO_PREEMPT, TASK_NOT_NEED_RSP_FLAG, TASK_NEED_NONE_RSP);

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_shutdown_taskcomm, ERR_MODULE_ID);
    }

    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);/*mod_mgr will be freed automatically if calling task_no_wait*/
#endif
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_show_taskcomm(API_UI_PARAM * param)
{
    int  tcid;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;
    UINT32 ret;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_str(param, 2, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "show taskcomm tcid %s where = %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_taskcomm beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_taskcomm end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0060);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        TASKC_MGR *taskc_mgr;

        taskc_mgr = taskc_mgr_new();

        cvector_push(report_vec, (void *)taskc_mgr);
        task_pos_inc(task_mgr, remote_mod_node_idx, &ret, FI_super_sync_taskc_mgr, 0, taskc_mgr);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        TASKC_MGR *taskc_mgr;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        taskc_mgr = (TASKC_MGR *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node));
        taskc_mgr_print(des_log, taskc_mgr);

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        taskc_mgr_free(taskc_mgr);
    }

    cvector_free(report_vec, LOC_API_0061);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_show_taskcomm_all(API_UI_PARAM * param)
{
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;
    UINT32 ret;

    api_ui_param_str(param, 1, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "show taskcomm all where = %s\n", where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_taskcomm_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_show_taskcomm_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0062);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        TASKC_MGR *taskc_mgr;

        taskc_mgr = taskc_mgr_new();

        cvector_push(report_vec, (void *)taskc_mgr);
        task_pos_inc(task_mgr, remote_mod_node_idx, &ret, FI_super_sync_taskc_mgr, 0, taskc_mgr);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE  *mod_node;
        TASKC_MGR *taskc_mgr;

        mod_node  = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        taskc_mgr = (TASKC_MGR  *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld]\n", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node));
        taskc_mgr_print(des_log, taskc_mgr);

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        taskc_mgr_free(taskc_mgr);
    }

    cvector_free(report_vec, LOC_API_0063);
    mod_mgr_free(mod_mgr);

    return (NULL_PTR);
}

char *ui_run_shell(API_UI_PARAM * param)
{
    int tcid;

    CSTRING *cmd_line;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    cmd_line = cstring_new(NULL_PTR, LOC_API_0064);

    api_ui_param_cstring(param, 1, cmd_line);
    api_ui_param_int(param, 2, &tcid);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "run shell %s on tcid %s where %s\n", (char *)cstring_get_str(cmd_line), uint32_to_ipv4(INT32_TO_UINT32(tcid)), where);

    mod_mgr = ui_gen_mod_mgr(INT32_TO_UINT32(tcid), CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_run_shell beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_run_shell end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0065);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_run_shell, ERR_MODULE_ID, cmd_line, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld] %s\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(cmd_line),
                          (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0066);
    mod_mgr_free(mod_mgr);

    cstring_free(cmd_line);

    return (NULL_PTR);
}

char *ui_run_shell_all(API_UI_PARAM * param)
{
    CSTRING *cmd_line;
    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr;
    TASK_MGR *task_mgr;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

    CVECTOR *report_vec;
    LOG   *des_log;

    cmd_line = cstring_new(NULL_PTR, LOC_API_0067);
    api_ui_param_cstring(param, 1, cmd_line);
    api_ui_param_str(param, 2, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "run shell %s all where = %s\n", (char *)cstring_get_str(cmd_line), where);

    mod_mgr = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_FWD_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_run_shell_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
    sys_log(LOGSTDOUT, "------------------------------------ ui_run_shell_all end ----------------------------------\n");
#endif

    report_vec = cvector_new(0, MM_LOG, LOC_API_0068);

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        LOG *log;

        log = log_cstr_open();

        cvector_push(report_vec, (void *)log);
        task_pos_inc(task_mgr, remote_mod_node_idx, NULL_PTR, FI_super_run_shell, ERR_MODULE_ID, cmd_line, log);
    }
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    des_log = ui_get_log(where);

    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        MOD_NODE *mod_node;
        LOG *log;

        mod_node = MOD_MGR_REMOTE_MOD(mod_mgr, remote_mod_node_idx);
        log = (LOG *)cvector_get(report_vec, remote_mod_node_idx);

        sys_log(des_log, "[rank_%s_%ld] %s\n%s", MOD_NODE_TCID_STR(mod_node),MOD_NODE_RANK(mod_node), (char *)cstring_get_str(cmd_line),
                          (char *)cstring_get_str(LOG_CSTR(log)));

        cvector_set(report_vec, remote_mod_node_idx, NULL_PTR);
        log_cstr_close(log);
    }

    cvector_free(report_vec, LOC_API_0069);
    mod_mgr_free(mod_mgr);

    cstring_free(cmd_line);

    return (NULL_PTR);
}

char *ui_ping_taskcomm(API_UI_PARAM * param)
{
    int  tcid;
    char where[UI_DEBUG_BUFFER_SIZE];

    TASK_MGR *task_mgr;

    MOD_NODE send_mod_node;
    MOD_NODE recv_mod_node;

    LOG   *des_log;
    EC_BOOL ret;

    api_ui_param_int(param, 1, &tcid);
    api_ui_param_str(param, 2, where, UI_DEBUG_BUFFER_SIZE - 1);

    des_log = ui_get_log(where);

    sys_log(LOGSTDOUT, "show taskcomm tcid %s where = %s\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)), where);

    ret = EC_FALSE; /*initialization*/

    sys_log(des_log, "ping tcid %s ....\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));

    task_mgr = task_new(NULL_PTR, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    MOD_NODE_TCID(&send_mod_node) = CMPI_LOCAL_TCID;
    MOD_NODE_COMM(&send_mod_node) = CMPI_LOCAL_COMM;
    MOD_NODE_RANK(&send_mod_node) = CMPI_LOCAL_RANK;
    MOD_NODE_MODI(&send_mod_node) = 0;
    MOD_NODE_HOPS(&send_mod_node) = 0;
    MOD_NODE_LOAD(&send_mod_node) = 0;

    MOD_NODE_TCID(&recv_mod_node) = INT32_TO_UINT32(tcid);
    MOD_NODE_COMM(&recv_mod_node) = CMPI_ANY_COMM;
    MOD_NODE_RANK(&recv_mod_node) = CMPI_FWD_RANK;
    MOD_NODE_MODI(&recv_mod_node) = 0;
    MOD_NODE_HOPS(&recv_mod_node) = 0;
    MOD_NODE_LOAD(&recv_mod_node) = 0;

    task_super_inc(task_mgr, &send_mod_node, &recv_mod_node, &ret, FI_super_ping_taskcomm, ERR_MODULE_ID);
    task_wait(task_mgr, TASK_DEFAULT_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    if(EC_TRUE == ret)
    {
        sys_log(des_log, "tcid %s is reachable\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));
    }
    else
    {
        sys_log(des_log, "tcid %s is unreachable\n", uint32_to_ipv4(INT32_TO_UINT32(tcid)));
    }

    return (NULL_PTR);
}

char* ui_mon_all(API_UI_PARAM * param)
{
    int       oid;
    int       times;

    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr_def;
    MOD_MGR  *mod_mgr;

    UINT32 remote_mod_node_num;

    UINT32 cmon_md_id;
    UINT32 ret;

    LOG *des_log;

    CMON_OBJ_VEC *cmon_obj_vec;
    CTIMER_NODE  *ctimer_node_meas;
    CTIMER_NODE  *ctimer_node_print;

    api_ui_param_int(param, 1, &oid);
    api_ui_param_int(param, 2, &times);
    api_ui_param_str(param, 3, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "ui_mon_all: oid = %d, times = %d, where = %s\n", oid, times, where);

    mod_mgr_def = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_mon_all beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr_def);
    sys_log(LOGSTDOUT, "------------------------------------ ui_mon_all end ----------------------------------\n");
#endif

    des_log = ui_get_log(where);

    mod_mgr_remote_num(mod_mgr_def, &remote_mod_node_num);
    task_act(mod_mgr_def, &mod_mgr, TASK_DEFAULT_LIVE, remote_mod_node_num, LOAD_BALANCING_LOOP, TASK_PRIO_HIGH, FI_cmon_start);
    mod_mgr_free(mod_mgr_def);

    cmon_md_id = cmon_start();
    cmon_set_mod_mgr(cmon_md_id, mod_mgr);

    /*define measurement scope and measurement object: add OID to cmon_obj_vec, different OIDs may add to same cmon_obj_vec*/
    cmon_obj_vec = cmon_obj_vec_new(cmon_md_id);
    cmon_obj_vec_incl(cmon_md_id, INT32_TO_UINT32(oid), CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI, cmon_obj_vec);

    /*star CTIMER module*/
    ctimer_start(10);

    /*define measurement timers: bind ctimer_node and cmon_obj_vec*/
    ctimer_node_meas = ctimer_node_new();
    ctimer_node_add(ctimer_node_meas,  1 * 1000, &ret, FI_cmon_obj_vec_meas, cmon_md_id, TASK_DEFAULT_LIVE, cmon_obj_vec);

    /*define print timers*/
    ctimer_node_print = ctimer_node_new();
    ctimer_node_add(ctimer_node_print, 2 * 1000, &ret, FI_cmon_obj_vec_print, cmon_md_id, des_log, cmon_obj_vec);

    /*start all timers*/
    ctimer_node_start(ctimer_node_meas);
    ctimer_node_start(ctimer_node_print);

    for(; 0 < times; times --)
    {
        pause();
        //sys_log(LOGCONSOLE, "ui_mon_all: times %d\n", times);
    }

    ctimer_clean();
#if 0
    /*stop all timers*/
    ctimer_node_stop(ctimer_node_meas);
    ctimer_node_stop(ctimer_node_print);

    /*optional: delete ctimer_node, all added ctimer_node will be destroyed whne CTIMER module stop*/
    ctimer_node_del(ctimer_node_meas);
    ctimer_node_del(ctimer_node_print);
#endif

    /*stop CTIMER module*/
    ctimer_end();

    cmon_obj_vec_free(cmon_md_id, cmon_obj_vec);

    task_dea(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_HIGH, FI_cmon_end, ERR_MODULE_ID);

    cmon_end(cmon_md_id);

    return(NULL_PTR);

}

char* ui_mon_oid(API_UI_PARAM * param)
{
    int       oid;
    int       times;
    int       tcid;
    int       rank;

    char where[UI_DEBUG_BUFFER_SIZE];

    MOD_MGR  *mod_mgr_def;
    MOD_MGR  *mod_mgr;

    UINT32 remote_mod_node_num;

    UINT32 cmon_md_id;
    UINT32 ret;
    LOG   *des_log;

    CMON_OBJ_VEC *cmon_obj_vec;
    CTIMER_NODE  *ctimer_node_meas;
    CTIMER_NODE  *ctimer_node_print;

    api_ui_param_int(param, 1, &oid);
    api_ui_param_int(param, 2, &times);
    api_ui_param_int(param, 3, &tcid);
    api_ui_param_int(param, 4, &rank);
    api_ui_param_str(param, 5, where, UI_DEBUG_BUFFER_SIZE - 1);

    sys_log(LOGSTDOUT, "ui_mon_all: oid = %d, times = %d, tcid = %d, rank = %d, where = %s\n", oid, times, tcid, rank, where);

    mod_mgr_def = ui_gen_mod_mgr(CMPI_ANY_TCID, CMPI_ANY_RANK, CMPI_ERROR_TCID, CMPI_ERROR_RANK, 0);/*super_md_id = 0*/
#if 1
    sys_log(LOGSTDOUT, "------------------------------------ ui_mon_oid beg ----------------------------------\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr_def);
    sys_log(LOGSTDOUT, "------------------------------------ ui_mon_oid end ----------------------------------\n");
#endif

    des_log = ui_get_log(where);

    mod_mgr_remote_num(mod_mgr_def, &remote_mod_node_num);
    task_act(mod_mgr_def, &mod_mgr, remote_mod_node_num, TASK_DEFAULT_LIVE, LOAD_BALANCING_LOOP, TASK_PRIO_HIGH, FI_cmon_start);
    mod_mgr_free(mod_mgr_def);

    cmon_md_id = cmon_start();
    cmon_set_mod_mgr(cmon_md_id, mod_mgr);

    /*define measurement scope and measurement object: add OID to cmon_obj_vec, different OIDs may add to same cmon_obj_vec*/
    cmon_obj_vec = cmon_obj_vec_new(cmon_md_id);
    cmon_obj_vec_incl(cmon_md_id, INT32_TO_UINT32(oid), INT32_TO_UINT32(tcid), CMPI_ANY_COMM, INT32_TO_UINT32(rank), CMPI_ANY_MODI, cmon_obj_vec);

    /*star CTIMER module*/
    ctimer_start(10);

    /*define measurement timers: bind ctimer_node and cmon_obj_vec*/
    ctimer_node_meas = ctimer_node_new();
    ctimer_node_add(ctimer_node_meas,  1 * 1000, &ret, FI_cmon_obj_vec_meas, cmon_md_id, TASK_DEFAULT_LIVE, cmon_obj_vec);

    /*define print timers*/
    ctimer_node_print = ctimer_node_new();
    ctimer_node_add(ctimer_node_print, 2 * 1000, &ret, FI_cmon_obj_vec_print, cmon_md_id, des_log, cmon_obj_vec);

    /*start all timers*/
    ctimer_node_start(ctimer_node_meas);
    ctimer_node_start(ctimer_node_print);

    for(; 0 < times; times --)
    {
        pause();
        //sys_log(LOGCONSOLE, "ui_mon_oid: times %d\n", times);
    }

    ctimer_clean();
#if 0
    /*stop all timers*/
    ctimer_node_stop(ctimer_node_meas);
    ctimer_node_stop(ctimer_node_print);

    /*optional: delete ctimer_node, all added ctimer_node will be destroyed whne CTIMER module stop*/
    ctimer_node_del(ctimer_node_meas);
    ctimer_node_del(ctimer_node_print);
#endif
    /*free results*/
    //cmon_result_free(cmon_md_id, cmon_result);

    /*stop CTIMER module*/
    ctimer_end();

    cmon_obj_vec_free(cmon_md_id, cmon_obj_vec);

    task_dea(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_HIGH, FI_cmon_end, ERR_MODULE_ID);

    cmon_end(cmon_md_id);

    return(NULL_PTR);

}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

