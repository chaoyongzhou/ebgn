/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifndef _CCURL_INC
#define _CCURL_INC

#include "findex.inc"

FUNC_ADDR_NODE g_ccurl_func_addr_list[ ] = {
{
/* -- void ccurl_print_module_status(const UINT32 ccurl_md_id, LOG *log); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_print_module_status",
/*func index      */     FI_ccurl_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 ccurl_free_module_static_mem(const UINT32 ccurl_md_id); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_free_module_static_mem",
/*func index      */     FI_ccurl_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 ccurl_start(); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_start",
/*func index      */     FI_ccurl_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     0,
/*func para direct*/     {E_DIRECT_END,},
/*func para type  */     {e_dbg_type_end,},
/*func para val   */     0, 0, {0},
},
{
/* -- void ccurl_end(const UINT32 ccurl_md_id); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_end",
/*func index      */     FI_ccurl_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 ccurl_set_npp_mod_mgr(const UINT32 ccurl_md_id, const MOD_MGR * src_mod_mgr); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_set_mod_mgr,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_set_mod_mgr",
/*func index      */     FI_ccurl_set_mod_mgr,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_MOD_MGR_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL ccurl_get(const UINT32 ccurl_md_id, const CSTRING *url_str, const CSTRING *proxy_ip_port, CSTRING *curl_reply_body); -- */
/*func priority   */     PRIO_MD_CCURL,
/*func logic addr */     (UINT32)ccurl_get,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "ccurl_get",
/*func index      */     FI_ccurl_get,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     4,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_CSTRING_ptr,e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_ccurl_func_addr_list_len = sizeof(g_ccurl_func_addr_list)/sizeof(g_ccurl_func_addr_list[0]);
#endif/*_CCURL_INC*/
