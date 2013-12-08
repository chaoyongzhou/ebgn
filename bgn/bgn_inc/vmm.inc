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

#ifndef _VMM_INC
#define _VMM_INC

#include "type.h"

#include "list_base.h"

#include "cmutex.h"
#include "cthread.h"

#include "mod.inc"
#include "vmm.h"
#include "findex.inc"

FUNC_ADDR_NODE g_vmm_func_addr_list[ ] = {
{
/* -- void vmm_print_module_status(const UINT32 vmm_md_id, LOG *log); -- */
/*func priority   */     PRIO_BEG,
/*func logic addr */     (UINT32)vmm_print_module_status,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_print_module_status",
/*func index      */     FI_vmm_print_module_status,
/*func ret type   */     e_dbg_void,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IO,},
/*func para type  */     {e_dbg_UINT32,e_dbg_LOG_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 vmm_free_module_static_mem(const UINT32 vmm_md_id); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_free_module_static_mem,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_free_module_static_mem",
/*func index      */     FI_vmm_free_module_static_mem,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 vmm_start( ); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_start,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_start",
/*func index      */     FI_vmm_start,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     0,
/*func para direct*/     {E_DIRECT_END,},
/*func para type  */     {e_dbg_type_end,},
/*func para val   */     0, 0, {0},
},
{
/* -- void vmm_end(const UINT32 vmm_md_id); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_end,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_end",
/*func index      */     FI_vmm_end,
/*func ret type   */     e_dbg_void,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,},
/*func para val   */     0, 0, {0},
},
{
/* -- UINT32 vmm_set_mod_mgr(const UINT32 vmm_md_id, const MOD_MGR * src_mod_mgr); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_set_mod_mgr,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_set_mod_mgr",
/*func index      */     FI_vmm_set_mod_mgr,
/*func ret type   */     e_dbg_UINT32,
/*func para num   */     2,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_MOD_MGR_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- MOD_MGR * vmm_get_mod_mgr(const UINT32 vmm_md_id) -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_get_mod_mgr,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_get_mod_mgr",
/*func index      */     FI_vmm_get_mod_mgr,
/*func ret type   */     e_dbg_MOD_MGR_ptr,
/*func para num   */     1,
/*func para direct*/     {E_DIRECT_IN},
/*func para type  */     {e_dbg_UINT32},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL vmm_alloc(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, VMM_NODE *vmm_node); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_alloc,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_alloc",
/*func index      */     FI_vmm_alloc,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     5,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_OUT,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,e_dbg_UINT32,e_dbg_UINT32,e_dbg_VMM_NODE_ptr,},
/*func para val   */     0, 0, {0},
},
{
/* -- EC_BOOL vmm_free(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, const VMM_NODE *vmm_node); -- */
/*func priority   */     PRIO_MD_VMM,
/*func logic addr */     (UINT32)vmm_free,
/*func beg addr   */     0,
/*func end addr   */     0,
/*func addr offset*/     0,
/*func name       */     "vmm_free",
/*func index      */     FI_vmm_free,
/*func ret type   */     e_dbg_EC_BOOL,
/*func para num   */     5,
/*func para direct*/     {E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,E_DIRECT_IN,},
/*func para type  */     {e_dbg_UINT32,e_dbg_UINT32,e_dbg_UINT32,e_dbg_UINT32,e_dbg_VMM_NODE_ptr,},
/*func para val   */     0, 0, {0},
},
};

UINT32 g_vmm_func_addr_list_len = sizeof(g_vmm_func_addr_list)/sizeof(g_vmm_func_addr_list[0]);

#endif /*_VMM_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

