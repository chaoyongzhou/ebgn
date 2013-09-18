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

#ifndef _LIB_DEBUG_H
#define _LIB_DEBUG_H

#include "lib_type.h"

//UINT32 dbg_start(const MYSQL_ACCESS *mysql_acc, UINT32 entry_func_logic_addr, const UINT8 *entry_func_name);

void dbg_exit( UINT32  module_type,UINT32  module_id);

#endif /* _LIB_DEBUG_H  */
#ifdef __cplusplus
}
#endif/*__cplusplus*/
