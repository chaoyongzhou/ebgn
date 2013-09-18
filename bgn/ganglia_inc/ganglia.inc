/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifndef _GANGLIA_INC
#define _GANGLIA_INC

#include "findex.inc"

FUNC_ADDR_NODE g_ganglia_func_addr_list[ ] = {
{
/* -- void ganglia_print_module_status(const UINT32 ganglia_md_id, LOG *log); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_print_module_status",
/*func index      */     FI_ganglia_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 ganglia_free_module_static_mem(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_free_module_static_mem",
/*func index      */     FI_ganglia_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 ganglia_start(const UINT32 mcast_ipaddr, const UINT32 srv_port); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_start",
/*func index      */     FI_ganglia_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- void ganglia_end(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_end",
/*func index      */     FI_ganglia_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_cpus_load(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_cpus_load,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_cpus_load",
/*func index      */     FI_ganglia_report_cpus_load,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_sys_mem_load(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_sys_mem_load,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_sys_mem_load",
/*func index      */     FI_ganglia_report_sys_mem_load,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_proc_mem_load(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_proc_mem_load,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_proc_mem_load",
/*func index      */     FI_ganglia_report_proc_mem_load,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_crank_thread_stat(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_crank_thread_stat,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_crank_thread_stat",
/*func index      */     FI_ganglia_report_crank_thread_stat,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_dsks_stat(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_dsks_stat,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_dsks_stat",
/*func index      */     FI_ganglia_report_dsks_stat,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ganglia_report_eths_stat(const UINT32 ganglia_md_id); -- */
/*func priority   */     PRIO_MD_GANGLIA,
/*func logic addr */     (UINT32)ganglia_report_eths_stat,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ganglia_report_eths_stat",
/*func index      */     FI_ganglia_report_eths_stat,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_ganglia_func_addr_list_len = sizeof(g_ganglia_func_addr_list)/sizeof(g_ganglia_func_addr_list[0]);
#endif/*_GANGLIA_INC*/

