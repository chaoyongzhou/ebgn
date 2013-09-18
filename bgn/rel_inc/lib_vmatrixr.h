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

#ifndef _LIB_VvoidR_H
#define _LIB_VvoidR_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_matrixr.h"
#include "lib_task.h"

#include "lib_vmm.h"


/**
*   for test only
*
*   to query the status of VvoidR Module
*
**/
void print_vmatrix_r_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed VvoidR module
*
*
**/
UINT32 vmatrix_r_free_module_static_mem(const UINT32 vmatrixr_md_id);

/**
*
* start VvoidR module
*
**/
UINT32 vmatrix_r_start( );

/**
*
* end VvoidR module
*
**/
void vmatrix_r_end(const UINT32 vmatrixr_md_id);

/**
*
* initialize mod mgr of VvoidR module
*
**/
UINT32 vmatrix_r_set_mod_mgr(const UINT32 vmatrixr_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of voidR module
*
**/
void * vmatrix_r_get_mod_mgr(const UINT32 vmatrixr_md_id);

void * vmatrix_r_alloc_vmm_node(const UINT32 vmatrixr_md_id);

EC_BOOL vmatrix_r_free_vmm_node(const UINT32 vmatrixr_md_id, void *vmm_node);

UINT32 vmatrix_r_alloc_block(const UINT32 vmatrixr_md_id, void *matrix_block_vmm);

UINT32 vmatrix_r_free_block(const UINT32 vmatrixr_md_id, void *matrix_block_vmm);

EC_BOOL vmatrix_r_local_mod_node(const UINT32 vmatrixr_md_id, void *mod_node);

UINT32 vmatrix_r_block_set_row_num(const UINT32 vmatrixr_md_id, const UINT32 row_num, void *matrix_block_vmm);

UINT32 vmatrix_r_block_set_col_num(const UINT32 vmatrixr_md_id, const UINT32 col_num, void *matrix_block_vmm);

UINT32 vmatrix_r_new_matrix_skeleton(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, void *matrix);

UINT32 vmatrix_r_new_matrix(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, void *matrix_vmm);

UINT32 vmatrix_r_get_block(const UINT32 vmatrixr_md_id, const void *vmm_node, void *matrix_block);

UINT32 vmatrix_r_set_block(const UINT32 vmatrixr_md_id, const void *matrix_block, void *vmm_node);

UINT32 vmatrix_r_block_mul(const UINT32 vmatrixr_md_id, const void *src_matrix_row_block_vmm_1, const void *src_matrix_row_block_vmm_2, void *des_matrix_row_block_vmm);

UINT32 vmatrix_r_block_adc(const UINT32 vmatrixr_md_id, const void *src_matrix_row_block_vmm, void *des_matrix_row_block_vmm);

UINT32 vmatrix_r_clean_matrix(const UINT32 vmatrixr_md_id, void *matrix_vmm);

UINT32 vmatrix_r_move_matrix(const UINT32 vmatrixr_md_id, void *src_matrix_vmm, void *des_matrix_vmm);

UINT32 vmatrix_r_free_matrix( const UINT32 vmatrixr_md_id, void *matrix_vmm);

UINT32 vmatrix_r_destroy_matrix( const UINT32 vmatrixr_md_id, void *matrix_vmm);

/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
*
**/
UINT32 vmatrix_r_mul_p(const UINT32 vmatrixr_md_id, const void *src_matrix_vmm_1, const void *src_matrix_vmm_2, void *des_matrix_vmm);


#endif /*_LIB_VvoidR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

