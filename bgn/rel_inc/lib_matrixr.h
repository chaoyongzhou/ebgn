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

#ifndef _LIB_MATRIXR_H
#define _LIB_MATRIXR_H

#include "lib_type.h"
#include "lib_mm.h"

/**
*   for test only
*
*   to query the status of MATRIXR Module
*
**/
void print_matrix_r_status(LOG *log);

/**
*
*   free all static memory occupied by the appointed MATRIXR module
*
*
**/
UINT32 matrix_r_free_module_static_mem(const UINT32 matrixr_md_id);
/**
*
* start MATRIXR module
*
**/
UINT32 matrix_r_start();

/**
*
* end MATRIXR module
*
**/
void matrix_r_end(const UINT32 matrixr_md_id);

/**
*
* set mod mgr of MATRIXR module
*
**/
UINT32 matrix_r_set_mod_mgr(const UINT32 matrixr_md_id, const void * src_task_mod_mgr);

/**
*
* new a matrix
* new a matrix node itsele and its skeleton
* note: all matrix data_area is null without allocing memory space
*
**/
UINT32 matrix_r_new_matrix(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX **ppmatrix);

/**
*
* clean a matrix
* all data area pointers, block pointers, header pointers, except matrix itself pointer,  will be destroyed.
* note:
*       matrix pointer can be reused after calling
*
**/
UINT32 matrix_r_clean_matrix(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* destroy a matrix
* all data area pointers, block pointers, header pointers, and matrix itself pointer will be destroyed.
* note:
*       matrix pointer cannot be reused after calling
*
**/
UINT32 matrix_r_destroy_matrix(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* insert datas into matrix row by row
* note:
*     matrix must be empty with skeleton only. any pointers mounted on data_area will be lost without notification
*
**/
UINT32 matrix_r_insert_data_by_row(const UINT32 matrixr_md_id, const REAL *pdata[], const UINT32 data_num, MATRIX *matrix);

/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: ppdata[] is two-dimension array carrying on REAL pointer
*
**/
UINT32 matrix_r_insert_data_by_tbl(const UINT32 matrixr_md_id, const REAL **ppdata[], const UINT32 row_num, const UINT32 col_num, MATRIX *matrix);

/**
*
* get row num of matrix
*
**/
UINT32 matrix_r_get_row_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num);

/**
*
* get col num of matrix
*
**/
UINT32 matrix_r_get_col_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *col_num);

/**
*
* get type of matrix
* if matrix type is (m x n), then return row_num = m, and col_num = n
*
**/
UINT32 matrix_r_get_type(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num, UINT32 *col_num);

/**
*
* exchange two rows of the matrix
* if row no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_rows(const UINT32 matrixr_md_id, const UINT32 row_no_1, const UINT32 row_no_2, MATRIX *matrix);

/**
*
* exchange two cols of the matrix
* if col no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_cols(const UINT32 matrixr_md_id, const UINT32 col_no_1, const UINT32 col_no_2, MATRIX *matrix);

/**
*
* clone src matrix to des matrix
*       des_matrix = src_matrix
* note:
*    here des_matrix must be empty matrix, otherwise, its skeleton will be lost without notification
*
**/
UINT32 matrix_r_clone(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix is_zero operation
*    if all data_area is zero or null pointer, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_zero(const UINT32 matrixr_md_id, const MATRIX *matrix);

/**
*
* matrix is_one operation
*    if matrix is unit matrix, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_one(const UINT32 matrixr_md_id, const MATRIX *matrix);

/**
*
* matrix = 0
*
**/
UINT32 matrix_r_set_zero(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix = 1
* note:
*       1. row num of matrix must be same as col num
*       2. must row num > 0
*
**/
UINT32 matrix_r_set_one(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix cmp operation
*    if matrix_1 == matrix_2, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_cmp(const UINT32 matrixr_md_id, const MATRIX *matrix_1, const MATRIX *matrix_2);

/**
*
* rotate a matrix
*       matrix = T(matrix)
*
**/
UINT32 matrix_r_rotate(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix neg operation
*     des_matrix = - src_matrix
*
**/
UINT32 matrix_r_neg(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix add operation
*     des_matrix = des_matrix + src_matrix
* matrix type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 matrix_r_adc(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix add operation
*     des_matrix = src_matrix_1 + src_matrix_2
* matrix type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 matrix_r_add(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

/**
*
* matrix sbb operation
*     des_matrix = des_matrix - src_matrix
* matrix type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 matrix_r_sbb(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix sub operation
*     des_matrix = src_matrix_1 - src_matrix_2
* matrix type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 matrix_r_sub(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

/**
*
* matrix smul operation
*     des_matrix = s_data * src_matrix
*
**/
UINT32 matrix_r_s_mul(const UINT32 matrixr_md_id, const REAL *s_data_addr, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix squ operation
*     matrix = matrix ^ 2
* matrix type: (m x m) = (m x m) * (m x m)
*
**/
UINT32 matrix_r_squ_self(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* matrix squ operation
*     des_matrix = src_matrix ^ 2
* matrix type: (m x m) = (m x m) * (m x m)
*
**/
UINT32 matrix_r_squ(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = des_matrix * src_matrix
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul_self_rear(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = src_matrix * des_matrix
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul_self_front(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix);

/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
* matrix type: (m x n) = (m x k) * (k x n)
*
**/
UINT32 matrix_r_mul(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);
UINT32 matrix_r_mul_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix);

/**
*
* matrix det operation
*     det = det(src_matrix)
* and
*     des_matrix return the middle matrix during computing det
* note:
*     1. des_matrix may not the final matrix when det = 0
*     2. des_matrix may be null pointer if not want to return the middle matrix
*
**/
UINT32 matrix_r_det(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix, REAL *det);

/*----------------------------------------------------- matrix & vector operation ---------------------------------------------------*/
/**
*
* matrix and vector mul operation
*    des_vector = src_matrix * src_vector
* type: (m x 1) = (m x n) * (n x 1)
*
*
**/
UINT32 matrix_r_matrix_mul_vector(const UINT32 matrixr_md_id, const MATRIX *src_matrix,const VECTOR *src_vector, VECTOR *des_vector);

UINT32 matrix_r_matrix_mul_vector_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix,const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* matrix and vector mul operation
*    des_vector = src_vector * src_matrix
* type: (1 x n) = (1 x m) * (m x n)
*
*
**/
UINT32 matrix_r_vector_mul_matrix(const UINT32 matrixr_md_id, const VECTOR *src_vector,const MATRIX *src_matrix, VECTOR *des_vector);

/**
*
* matrix and vector mul operation
*    des_matrix = src_vector_1 * src_vector_1
* type: (1 x 1) = (1 x m) * (m x 1)
*    or (m x m) = (m x 1) * (1 x m)
*
**/
UINT32 matrix_r_vector_mul_vector(const UINT32 matrixr_md_id, const VECTOR *src_vector_1,const VECTOR *src_vector_2, MATRIX *des_matrix);

/**
*
* print out data info of matrix
*
**/
UINT32 matrix_r_print_matrix_data_info(const UINT32 matrixr_md_id, MATRIX *matrix);

/**
*
* print out addr info of matrix
*
**/
UINT32 matrix_r_print_matrix_addr_info(const UINT32 matrixr_md_id, MATRIX *matrix);
#endif /*_LIB_MATRIXR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

