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

#ifndef _VECTORR_H
#define _VECTORR_H

#include "type.h"
#include "mm.h"
#include "task.h"

#define VECTORR_BLOCK_GET_DATA_VAL(vector_block, sub_idx ) \
        (VECTOR_BLOCK_GET_DATA_VAL(vector_block, sub_idx, REAL))

#define VECTORR_BLOCK_SET_DATA_VAL(vector_block, sub_idx, val) \
        (VECTOR_BLOCK_SET_DATA_VAL(vector_block, sub_idx, REAL, val))

#define VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx) \
        ((REAL *)VECTOR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx))

typedef struct
{
    /* used counter >= 0 */
    UINT32 usedcounter ;

    MOD_MGR  *mod_mgr;

    UINT32 real_md_id;

}VECTORR_MD;

/**
*   for test only
*
*   to query the status of VECTORR Module
*
**/
void vector_r_print_module_status(const UINT32 vectorr_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed VECTORR module
*
*
**/
UINT32 vector_r_free_module_static_mem(const UINT32 vectorr_md_id);
/**
*
* start VECTORR module
*
**/
UINT32 vector_r_start( );

/**
*
* end VECTORR module
*
**/
void vector_r_end(const UINT32 vectorr_md_id);

/**
*
* initialize mod mgr of VECTORR module
*
**/
UINT32 vector_r_set_mod_mgr(const UINT32 vectorr_md_id, const MOD_MGR * src_task_mod_mgr);

/**
*
* get mod mgr of VECTORR module
*
**/
MOD_MGR * vector_r_get_mod_mgr(const UINT32 vectorr_md_id);

/**
*
* alloc and intialize a vector block to a MATRIX_VECTOR_WIDTH dimension vector
*
**/
UINT32 vector_r_alloc_block( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR_BLOCK **vector_block);

/**
*
* intialize a vector block to a MATRIX_VECTOR_WIDTH dimension vector
*
**/
UINT32 vector_r_init_block(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block);

/**
*
* clear data_area but do not free block itself
*
**/
UINT32 vector_r_clear_block(const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block);

/**
*
* clean data_area without check vectorr_md_type, and do not free block itself
*
**/
UINT32 vector_r_clean_block(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block);

/**
*
* clear data_area, meanwhile free block itself
*
**/
UINT32 vector_r_free_block( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR_BLOCK *vector_block);


/**
*
*  vector_block is zero or not
*  note:
*      here vector_block must not be null pointer
*
**/
EC_BOOL vector_r_block_is_zero(const UINT32 vectorr_md_id, const VECTOR_BLOCK *vector_block);

/**
*
*  des_vector_block = src_vector_block_1 + src_vector_block_2
*  note:
*      here des_vector_block must not be src_vector_block_1 or src_vector_block_2
*
**/
UINT32 vector_r_block_add(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block_1,const VECTOR_BLOCK *src_vector_block_2,VECTOR_BLOCK *des_vector_block);

/**
*
*  des_vector_block += src_vector_block
*  note:
*      here des_vector_block must not be src_vector_block
*
**/
UINT32 vector_r_block_adc(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block);

/**
*
*  des_vector_block = src_vector_block_1 - src_vector_block_2
*  note:
*      here des_vector_block must not be src_vector_block_1 or src_vector_block_2
*
**/
UINT32 vector_r_block_sub(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block_1, const VECTOR_BLOCK *src_vector_block_2, VECTOR_BLOCK *des_vector_block);

/**
*
*  des_vector_block -= src_vector_block
*  note:
*      here des_vector_block must not be src_vector_block
*
**/
UINT32 vector_r_block_sbb(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block);

/**
*
* new skeleton of a vector
* note: all vector item pointer is null without allocing memory space
*
**/
UINT32 vector_r_new_vector_skeleton(const UINT32 vectorr_md_id, const UINT32 num, VECTOR *vector);

/**
*
* new a vector
* new a vector node itsele and its skeleton
* note: all vector item pointer is null without allocing memory space
*
**/
UINT32 vector_r_new_vector(const UINT32 vectorr_md_id, UINT32 num, VECTOR **ppvector);


/**
*
* intialize a vector to a 0 dimension vector
*
**/
UINT32 vector_r_init_vector(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* move src_vector skeleton and data area to des_vector
* note:
*     des_vector must have no skeleton or data area. otherwise, skeleton and data area will be lost without free
*
**/
UINT32 vector_r_move_vector(const UINT32 vectorr_md_id, VECTOR *src_vector, VECTOR *des_vector);

/**
*
* clean a vector
* all data area pointers, block pointers, except vector itself pointer,  will be destroyed.
* note:
*       vector pointer can be reused after calling
*
**/
UINT32 vector_r_clean_vector(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* destroy a vector
* all data area pointers, block pointers, and vector itself pointer will be destroyed.
* note:
*       vector pointer cannot be reused after calling
*
**/
UINT32 vector_r_destroy_vector(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* insert datas into vector
* note:
*     vector must be empty with skeleton only. any pointers mounted on data_area will be lost without notification
*
**/
UINT32 vector_r_insert_data(const UINT32 vectorr_md_id, const REAL *pdata[], const UINT32 data_num, VECTOR *vector);

/**
*
* get row num of vector
*
**/
UINT32 vector_r_get_row_num(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *row_num);

/**
*
* get col num of vector
*
**/
UINT32 vector_r_get_col_num(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *col_num);

/**
*
* get type of vector
* if vector type is (m x 1), then return (m, 1)
* if vector type is (1 x n), then return (1, n)
*
**/
UINT32 vector_r_get_type(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *row_num, UINT32 *col_num);

/**
*
* exchange two rows or cols of the vector
* if idx overflow, then report error and do nothing
*
**/
UINT32 vector_r_xchg(const UINT32 vectorr_md_id, const UINT32 idx_1, const UINT32 idx_2, VECTOR *vector);

/**
*
* clone src vector to des vector
*       des_vector = src_vector
* note:
*    here des_vector must be empty vector, otherwise, its skeleton will be lost without notification
*
**/
UINT32 vector_r_clone(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* vector is_zero operation
*    if all data_area is zero or null pointer, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_is_zero(const UINT32 vectorr_md_id, const VECTOR *vector);

/**
*
* vector is_one operation
*    if vector is unit vector, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_is_one(const UINT32 vectorr_md_id, const VECTOR *vector);

/**
*
* vector = 0
*
**/
UINT32 vector_r_set_zero(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* vector = 1
* note:
*       1. row num of vector must be same as col num
*       2. must row num > 0
*
**/
UINT32 vector_r_set_one(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* vector cmp operation
*    if vector_1 == vector_2, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_cmp(const UINT32 vectorr_md_id, const VECTOR *vector_1, const VECTOR *vector_2);

/**
*
* rotate a vector
*       vector = T(vector)
* if vector is row vector, then it will come to be col vector;
* if vector is col vector, then it will come to be row vector;
*
**/
UINT32 vector_r_rotate(const UINT32 vectorr_md_id, VECTOR *vector);

/**
*
* vector neg operation
*     des_vector = - src_vector
*
**/
UINT32 vector_r_neg(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* vector add operation
*     des_vector = des_vector + src_vector
* vector type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 vector_r_adc(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* vector add operation
*     des_vector = src_vector_1 + src_vector_2
* vector type: (m x n) = (m x n) + (m x n)
*
**/
UINT32 vector_r_add(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, VECTOR *des_vector);

/**
*
* vector sbb operation
*     des_vector = des_vector - src_vector
* vector type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 vector_r_sbb(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* vector sub operation
*     des_vector = src_vector_1 - src_vector_2
* vector type: (m x n) = (m x n) - (m x n)
*
**/
UINT32 vector_r_sub(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, VECTOR *des_vector);

/**
*
* vector smul operation
*     des_vector = s_data * src_vector
*
**/
UINT32 vector_r_s_mul(const UINT32 vectorr_md_id, const REAL *s_data_addr, const VECTOR *src_vector, VECTOR *des_vector);

/**
*
* row vector mul col vector operation
*     des_data = src_vector_1 * src_vector_2
*
**/
UINT32 vector_r_row_vector_mul_col_vector(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, REAL *des_data_addr);

UINT32 vector_r_print_vector_addr_info(const UINT32 vectorr_md_id, VECTOR *vector);
UINT32 vector_r_print_vector_data_info(const UINT32 vectorr_md_id, VECTOR *vector);

#endif /*_VECTORR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
