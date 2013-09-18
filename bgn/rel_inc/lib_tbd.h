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

#ifndef _LIB_TBD_H
#define _LIB_TBD_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_task.h"


/**
*   for test only
*
*   to query the status of TBD Module
*
**/
void print_tbd_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed TBD module
*
*
**/
UINT32 tbd_free_module_static_mem(const UINT32 tbd_md_id);

/**
*
* start TBD module
*
**/
UINT32 tbd_start( );

/**
*
* end TBD module
*
**/
void tbd_end(const UINT32 tbd_md_id);


/**
*
* initialize mod mgr of TBD module
*
**/
UINT32 tbd_set_mod_mgr(const UINT32 tbd_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of TBD module
*
**/
void * tbd_get_mod_mgr(const UINT32 tbd_md_id);

/**
*
* run body
* load user interface and execute it
*
**/
UINT32 tbd_run(const UINT32 tbd_md_id, const void * ui_retval_addr, const UINT32 ui_id, ...);

#endif /*_LIB_TBD_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

