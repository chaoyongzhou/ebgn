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

#ifndef _LIB_DMATRIXR_H
#define _LIB_DMATRIXR_H

#include "lib_type.h"
#include "lib_mm.h"
#include "lib_matrixr.h"
#include "lib_task.h"


/**
*   for test only
*
*   to query the status of DMATRIXR Module
*
**/
void print_dmatrix_r_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed DMATRIXR module
*
*
**/
UINT32 dmatrix_r_free_module_static_mem(const UINT32 dmatrixr_md_id);

/**
*
* start DMATRIXR module
*
**/
UINT32 dmatrix_r_start( );

/**
*
* end DMATRIXR module
*
**/
void dmatrix_r_end(const UINT32 dmatrixr_md_id);


/**
*
* initialize mod mgr of DMATRIXR module
*
**/
UINT32 dmatrix_r_set_mod_mgr(const UINT32 dmatrixr_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of MATRIXR module
*
**/
void * dmatrix_r_get_mod_mgr(const UINT32 dmatrixr_md_id);

/**
*
* get matrixr_md_id of DMATRIXR module
*
**/
UINT32 dmatrix_r_get_matrixr_md_id(const UINT32 dmatrixr_md_id, UINT32 * matrixr_md_id);

UINT32 dmatrix_r_append_row_block(const UINT32 dmatrixr_md_id, const void *matrixr_row_block);

UINT32 dmatrix_r_append_col_block(const UINT32 dmatrixr_md_id, const void *matrixr_col_block);

UINT32 dmatrix_r_deliver_row_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);

UINT32 dmatrix_r_deliver_col_blocks(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);

UINT32 dmatrix_r_deliver_rows_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);
UINT32 dmatrix_r_deliver_cols_p(const UINT32 dmatrixr_md_id, const MATRIX *matrixr);;

UINT32 dmatrix_r_mul_p(const UINT32 dmatrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

#endif /*_LIB_DMATRIXR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

