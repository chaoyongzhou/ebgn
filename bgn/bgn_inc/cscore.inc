/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 2796796
*
*******************************************************************************/
#ifndef _CSCORE_INC
#define _CSCORE_INC

#include "findex.inc"

FUNC_ADDR_NODE g_cscore_func_addr_list[ ] = {
{
/* -- void cscore_print_module_status(const UINT32 cscore_md_id, LOG *log); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_print_module_status",
/*func index      */     FI_cscore_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_OUT,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 cscore_free_module_static_mem(const UINT32 cscore_md_id); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_free_module_static_mem",
/*func index      */     FI_cscore_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 cscore_start(); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_start",
/*func index      */     FI_cscore_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     0,
/*func para direct*/     {E_DIRECT_END,},
/*func para type  */     {e_dbg_type_end,},
/*func para val   */     0, 0, {0},
},
{
/* -- void cscore_end(const UINT32 cscore_md_id); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_end",
/*func index      */     FI_cscore_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csdoc_words_import(const UINT32 cscore_md_id, const CSDOC_WORDS *csdoc_words); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csdoc_words_import,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csdoc_words_import",
/*func index      */     FI_cscore_csdoc_words_import,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CSDOC_WORDS_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csword_docs_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CSWORD_DOCS *csword_docs); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csword_docs_export,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csword_docs_export",
/*func index      */     FI_cscore_csword_docs_export,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,e_dbg_CSWORD_DOCS_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csdoc_words_list_import(const UINT32 cscore_md_id, const CLIST *csdoc_words_clist); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csdoc_words_list_import,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csdoc_words_list_import",
/*func index      */     FI_cscore_csdoc_words_list_import,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CLIST_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csword_docs_list_export(const UINT32 cscore_md_id, const UINT32 cached_mode, CLIST *csword_docs_list); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csword_docs_list_export,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csword_docs_list_export",
/*func index      */     FI_cscore_csword_docs_list_export,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,e_dbg_CLIST_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csword_docs_list_export_docs(const UINT32 cscore_md_id, const UINT32 cached_mode, const void *csword_list, void *csdoc_list); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csword_docs_list_export_docs,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csword_docs_list_export_docs",
/*func index      */     FI_cscore_csword_docs_list_export_docs,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     4,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,e_dbg_CLIST_ptr,e_dbg_CLIST_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL cscore_csword_docs_list_merge(const UINT32 cscore_md_id, const CLIST *csword_docs_list, CLIST *csdoc_list); -- */
/*func priority   */     PRIO_MD_CSCORE,
/*func logic addr */     (UINT32)cscore_csword_docs_list_merge,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "cscore_csword_docs_list_merge",
/*func index      */     FI_cscore_csword_docs_list_merge,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     3,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_CLIST_ptr,e_dbg_CLIST_ptr,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_cscore_func_addr_list_len = sizeof(g_cscore_func_addr_list)/sizeof(g_cscore_func_addr_list[0]);
#endif/*_CSCORE_INC*/

