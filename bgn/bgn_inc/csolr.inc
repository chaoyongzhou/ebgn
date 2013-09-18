/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifndef _CSOLR_INC
#define _CSOLR_INC

#include "findex.inc"

FUNC_ADDR_NODE g_csolr_func_addr_list[ ] = {
{
/* -- void csolr_print_module_status(const UINT32 csolr_md_id, LOG *log); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_print_module_status",
/*func index      */     FI_csolr_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 csolr_free_module_static_mem(const UINT32 csolr_md_id); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_free_module_static_mem",
/*func index      */     FI_csolr_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 csolr_start(); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_start",
/*func index      */     FI_csolr_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     0,
/*func para direct*/     {E_DIRECT_END,},
/*func para type  */     {e_dbg_void,},
/*func para val   */     0, 0, {0},
},
{
/* -- void csolr_end(const UINT32 csolr_md_id); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_end",
/*func index      */     FI_csolr_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 csolr_set_mod_mgr(const UINT32 csolr_md_id, const MOD_MGR * src_mod_mgr); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_set_mod_mgr,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_set_mod_mgr",
/*func index      */     FI_csolr_set_mod_mgr,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_MOD_MGR_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL csolr_send_req(const UINT32 csolr_md_id, const CSTRING *solr_req); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_send_req,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_send_req",
/*func index      */     FI_csolr_send_req,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL csolr_recv_req(const UINT32 csolr_md_id, CSTRING *solr_req); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_recv_req,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_recv_req",
/*func index      */     FI_csolr_recv_req,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL csolr_send_rsp(const UINT32 csolr_md_id, const CSTRING *solr_rsp); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_send_rsp,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_send_rsp",
/*func index      */     FI_csolr_send_rsp,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL csolr_recv_rsp(const UINT32 csolr_md_id, CSTRING *solr_rsp); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_recv_rsp,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_recv_rsp",
/*func index      */     FI_csolr_recv_rsp,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL csolr_do_req(const UINT32 csolr_md_id, const CSTRING *solr_req, CSTRING *solr_rsp); -- */
/*func priority   */     PRIO_MD_CSOLR,
/*func logic addr */     (UINT32)csolr_do_req,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "csolr_do_req",
/*func index      */     FI_csolr_do_req,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_csolr_func_addr_list_len = sizeof(g_csolr_func_addr_list)/sizeof(g_csolr_func_addr_list[0]);
#endif/*_CSOLR_INC*/

