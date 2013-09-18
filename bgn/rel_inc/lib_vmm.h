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

#ifndef _LIB_VMM_H
#define _LIB_VMM_H

#include <stdio.h>
#include <stdlib.h>

/**
*   for test only
*
*   to query the status of VMM Module
*
**/
void print_vmm_status(LOG *log);

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
UINT32 vmm_set_mod_mgr(const UINT32 vmm_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of VMM module
*
**/
void * vmm_get_mod_mgr(const UINT32 vmm_md_id);

/**
*
* init vmm node in VMM module
*
**/
UINT32 vmm_init_node(const UINT32 vmm_md_id, void *vmm_node);

/**
*
* clean vmm node mgr in VMM module
*
**/
UINT32 vmm_clean_node(const UINT32 vmm_md_id, void *vmm_node);

/**
*
* alloc virtual memory
*
**/
EC_BOOL vmm_alloc(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, void *vmm_node);

/**
*
* free virtual memory
*
**/
EC_BOOL vmm_free(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, const void *vmm_node);

#endif /*_LIB_VMM_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

