/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifndef _CXMPPC2S_INC
#define _CXMPPC2S_INC

#include "findex.inc"

FUNC_ADDR_NODE g_cxmppc2s_func_addr_list[ ] = {
{
/* -- void cxmppc2s_print_module_status(const UINT32 cxmppc2s_md_id, LOG *log); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_print_module_status",
/*func index      */     FI_cxmppc2s_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 cxmppc2s_free_module_static_mem(const UINT32 cxmppc2s_md_id); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_free_module_static_mem",
/*func index      */     FI_cxmppc2s_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 cxmppc2s_start(); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_start",
/*func index      */     FI_cxmppc2s_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     0,
/*func para direct*/     {E_DIRECT_END,},
/*func para type  */     {e_dbg_CSTRING_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- void cxmppc2s_end(const UINT32 cxmppc2s_md_id); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_end",
/*func index      */     FI_cxmppc2s_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cxmppc2s_get_mod_node(const UINT32 cxmppc2s_md_id, const CSTRING *to, MOD_NODE *mod_node); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_get_mod_node,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_get_mod_node",
/*func index      */     FI_cxmppc2s_get_mod_node,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_OUT,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_MOD_NODE_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cxmppc2s_send_message(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_message_buffer); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_send_message,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_send_message",
/*func index      */     FI_cxmppc2s_send_message,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_CBUFFER_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cxmppc2s_presence_subscribe(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribe_buffer); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_presence_subscribe,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_presence_subscribe",
/*func index      */     FI_cxmppc2s_presence_subscribe,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_CBUFFER_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cxmppc2s_presence_subscribed(const UINT32 cxmppc2s_md_id, const CSTRING *to, const CBUFFER *send_subscribed_buffer); -- */
/*func priority   */     PRIO_MD_CXMPPC2S,
/*func logic addr */     (UINT32)cxmppc2s_presence_subscribed,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cxmppc2s_presence_subscribed",
/*func index      */     FI_cxmppc2s_presence_subscribed,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSTRING_ptr,e_dbg_CBUFFER_ptr,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_cxmppc2s_func_addr_list_len = sizeof(g_cxmppc2s_func_addr_list)/sizeof(g_cxmppc2s_func_addr_list[0]);
#endif/*_CXMPPC2S_INC*/