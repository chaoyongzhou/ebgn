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

#ifndef _LIB_CREG_H
#define _LIB_CREG_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_log.h"

void *  creg_type_conv_item_new();

EC_BOOL creg_type_conv_item_init(void *type_conv_item);

EC_BOOL creg_type_conv_item_clean(void *type_conv_item);

EC_BOOL creg_type_conv_item_free(void *type_conv_item);

CVECTOR *creg_type_conv_vec_fetch();

EC_BOOL creg_type_conv_vec_init(CVECTOR *type_conv_vec);

EC_BOOL creg_type_conv_vec_clean(CVECTOR *type_conv_vec);

void *  creg_type_conv_vec_get(CVECTOR *type_conv_vec, const UINT32 var_dbg_type);

EC_BOOL creg_type_conv_vec_add(CVECTOR *type_conv_vec,
                                         const UINT32 var_dbg_type, const UINT32 var_sizeof, const UINT32 var_pointer_flag, const UINT32 var_mm_type,
                                         const UINT32 var_init_func, const UINT32 var_clean_func, const UINT32 var_free_func,
                                         const UINT32 var_encode_func, const UINT32 var_decode_func, const UINT32 var_encode_size
                                         );

void *  creg_func_addr_mgr_new();

EC_BOOL creg_func_addr_mgr_init(void *func_addr_mgr);

EC_BOOL creg_func_addr_mgr_clean(void *func_addr_mgr);

EC_BOOL creg_func_addr_mgr_free(void *func_addr_mgr);

CVECTOR *creg_func_addr_vec_fetch();

EC_BOOL creg_func_addr_vec_init(CVECTOR *func_addr_vec);

EC_BOOL creg_func_addr_vec_clean(CVECTOR *func_addr_vec);

void *  creg_func_addr_vec_get(const CVECTOR *func_addr_vec, const UINT32 md_type);

EC_BOOL creg_func_addr_vec_add(CVECTOR *func_addr_vec,
                                        const UINT32 md_type, const UINT32 *func_num_ptr, const void *func_addr_node,
                                        const UINT32 md_start_func_id, const UINT32 md_end_func_id,
                                        const UINT32 md_set_mod_mgr_func_id, void * (*md_fget_mod_mgr)(const UINT32)
                                        );


EC_BOOL creg_static_mem_tbl_add(const UINT32 mm_type, const char *mm_name, const UINT32 block_num, const UINT32 type_size, const UINT32 location);

#endif /*_LIB_CREG_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

