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

#ifndef _API_CMD_UI_H
#define _API_CMD_UI_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "type.h"
#include "clist.h"
#include "cvector.h"
#include "cstring.h"
#include "log.h"

#include "api_cmd.inc"

#define API_CMD_LINE_BUFF_SIZE                   ((UINT32) 4096) /*4k*/

EC_BOOL api_cmd_ui_init(CMD_ELEM_VEC *cmd_elem_vec, CMD_TREE *cmd_tree, CMD_HELP_VEC *cmd_help_vec);

EC_BOOL api_cmd_ui_task(CMD_TREE *cmd_tree, CMD_HELP_VEC *cmd_help_vec);

void    api_cmd_ui_do_script(CMD_TREE *cmd_tree, CMD_HELP_VEC *cmd_help_vec, char *script_name);
void    api_cmd_ui_do_once(CMD_TREE *cmd_tree, CMD_HELP_VEC *cmd_help_vec, char *cmd_line);

EC_BOOL api_cmd_ui_activate_sys_cfg(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_activate_sys_cfg_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_sys_cfg(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_sys_cfg_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_mem(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_mem_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_mem_of_type(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_mem_all_of_type(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_diag_mem(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_diag_mem_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_diag_mem_of_type(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_diag_mem_all_of_type(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_clean_mem(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_clean_mem_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_breathing_mem(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_breathing_mem_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_switch_log(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_switch_log_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_enable_to_rank_node(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_enable_all_to_rank_node(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_disable_to_rank_node(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_disable_all_to_rank_node(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_queue(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_queue_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_client(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_client_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_route(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_route_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_thread(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_thread_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_rank_node(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_rank_node_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_rank_load(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_rank_load_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_shutdown_work(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_shutdown_work_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_shutdown_dbg(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_shutdown_dbg_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_shutdown_mon(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_shutdown_mon_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_taskcomm(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_taskcomm_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_add_route(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_del_route(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_add_conn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_add_conn_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_run_shell(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_run_shell_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_ping_taskcomm(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_mon_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_mon_oid(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_taskcfgchk_net(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_taskcfgchk_net_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_taskcfgchk_route(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_taskcfgchk_route_trace(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_sync_taskcomm_from_local(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_sync_rank_load(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_sync_rank_load_to_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_set_rank_load(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_set_rank_load_on_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_create_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_open_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_with_flush_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_npp_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_with_flush_npp_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_create_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_open_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_with_flush_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_dn_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_close_with_flush_dn_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_read(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_write(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_trunc(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_update(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_mkdir(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_mkdir_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_qfile(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qdir(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qlist_path(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qlist_seg(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_qcount_files(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qsize_files(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qsize_one_file(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_qblock(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_flush_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_flush_dn(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_flush_npp_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_flush_dn_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_add_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_add_dn_to_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_add_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_add_npp_to_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_reg_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_reg_npp_to_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_reg_dn(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_reg_dn_to_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_list_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_list_dn(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_list_npp_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_list_dn_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_show_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_show_dn(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_show_npp_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_show_dn_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_show_block_path(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_show_cached_np(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_show_cached_np_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_showup_np(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_delete_path_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_delete_file_npp(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_delete_dir_npp(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_transfer_npp(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_make_snapshot(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_make_snapshot_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_import_fnode_log(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cdfs_import_replica_log(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_enable_task_brd(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_enable_all_task_brd(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_disable_task_brd(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_disable_all_task_brd(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cdfs_write_files(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_show_module(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_show_module_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_traversal_module(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_traversal_module_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_traversal_depth_module(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_traversal_depth_module_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_debug_merge(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_debug_split(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_runthrough_module(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_runthrough_module_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_runthrough_depth_module(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_runthrough_depth_module_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_print_status(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_print_status_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_create_root_table(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_create_user_table(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_delete_user_table(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_delete_colf_table(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_add_colf_table(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_insert(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_delete(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_search(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_fetch(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_cached(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_in_cached_user(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_in_all_user(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_in_cached_colf(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_select_in_all_colf(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_open_root_table(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_open_colf_table(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_close_module(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_cbgt_flush(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_cbgt_flush_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_csession_add(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_rmv_by_name(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_rmv_by_id(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_set_by_name(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_rmv_by_name_regex(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_rmv_by_id_regex(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_set_by_id(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_get_by_name(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_get_by_id(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_get_by_name_regex(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_get_by_id_regex(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_csession_show(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_exec_download(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_exec_download_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_exec_upload(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_exec_upload_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_exec_shell(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_exec_shell_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_make_license(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_check_license(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_check_license_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_license(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_license_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_show_version(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_version_all(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_vendor(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_show_vendor_all(CMD_PARA_VEC * param);

EC_BOOL api_cmd_ui_start_mcast_udp_server(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_stop_mcast_udp_server(CMD_PARA_VEC * param);
EC_BOOL api_cmd_ui_status_mcast_udp_server(CMD_PARA_VEC * param);

#endif/*_API_CMD_UI_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
