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

#ifndef _VMM_H
#define _VMM_H

#include <stdio.h>
#include <stdlib.h>

#include "task.inc"
#include "mod.inc"
#include "mod.h"

typedef struct
{
    MOD_NODE mod_node;

    UINT32   vmm_addr;
}VMM_NODE;

#define VMM_NODE_MOD_NODE(vmm_node) (&(vmm_node->mod_node))

#define VMM_NODE_TCID(vmm_node)         MOD_NODE_TCID(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_TCID_STR(vmm_node)     MOD_NODE_TCID_STR(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_COMM(vmm_node)         MOD_NODE_COMM(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_RANK(vmm_node)         MOD_NODE_RANK(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_MODI(vmm_node)         MOD_NODE_MODI(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_LOAD(vmm_node)         MOD_NODE_LOAD(VMM_NODE_MOD_NODE(vmm_node))
#define VMM_NODE_ADDR(vmm_node)         (vmm_node->vmm_addr)


/* VMM MODULE Defintion: */
typedef struct
{
    UINT32 usedcounter;/* used counter >= 0 */

    MOD_MGR *mod_mgr;
}VMM_MD;

/**
*   for test only
*
*   to query the status of VMM Module
*
**/
void vmm_print_module_status(const UINT32 vmm_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed VMM module
*
*
**/
UINT32 vmm_free_module_static_mem(const UINT32 vmm_md_id);

/**
*
* start VMM module
*
**/
UINT32 vmm_start( );

/**
*
* end VMM module
*
**/
void vmm_end(const UINT32 vmm_md_id);

/**
*
* initialize mod mgr of VMM module
*
**/
UINT32 vmm_set_mod_mgr(const UINT32 vmm_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get mod mgr of VMM module
*
**/
MOD_MGR * vmm_get_mod_mgr(const UINT32 vmm_md_id);

/**
*
* init vmm node in VMM module
*
**/
UINT32 vmm_init_node(const UINT32 vmm_md_id, VMM_NODE *vmm_node);

/**
*
* clean vmm node mgr in VMM module
*
**/
UINT32 vmm_clean_node(const UINT32 vmm_md_id, VMM_NODE *vmm_node);

/**
*
* alloc virtual memory
*
**/
EC_BOOL vmm_alloc(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, VMM_NODE *vmm_node);

/**
*
* free virtual memory
*
**/
EC_BOOL vmm_free(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, const VMM_NODE *vmm_node);

/**
*
* bind memory to vmm_node
*
**/
EC_BOOL vmm_bind(const UINT32 vmm_md_id, const UINT32 mem_addr, VMM_NODE *vmm_node);

/**
*
* dismiss memory from vmm_node
*
**/
EC_BOOL vmm_dismiss(const UINT32 vmm_md_id, VMM_NODE *vmm_node, UINT32 *mem_addr);

UINT32 vmm_print(const UINT32 vmm_md_id, LOG *log, const VMM_NODE *vmm_node);

#endif /*_VMM_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

