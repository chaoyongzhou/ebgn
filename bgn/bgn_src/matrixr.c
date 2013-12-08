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

#include <stdio.h>
#include <stdlib.h>

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"
#include "mm.h"
#include "log.h"

#include "real.h"

#include "vector.h"
#include "vectorr.h"

#include "matrix.h"
#include "matrixr.h"

#include "cmpic.inc"

#include "debug.h"

#include "print.h"

#include "task.h"

#include "findex.inc"

#include "cbc.h"
#include "clist.h"

#define MATRIXR_MD_CAPACITY()                  (cbc_md_capacity(MD_MATRIXR))

#define MATRIXR_MD_GET(matrixr_md_id)     ((MATRIXR_MD *)cbc_md_get(MD_MATRIXR, (matrixr_md_id)))

#define MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id)  \
    ((CMPI_ANY_MODI != (matrixr_md_id)) && ((NULL_PTR == MATRIXR_MD_GET(matrixr_md_id)) || (0 == (MATRIXR_MD_GET(matrixr_md_id)->usedcounter))))

#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON && MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
#error "fatal error:matrixr.c: user is able to switch on one patch at most!!!"
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON && MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_SWITCH == SWITCH_ON && MATRIXR_PATCH_01_SWITCH == SWITCH_OFF && MATRIXR_PATCH_02_SWITCH == SWITCH_OFF)
#error "fatal error:matrixr.c: unknow switch on which patch!!!"
#endif

#if (MATRIXR_PATCH_SWITCH == SWITCH_ON)
#define MATRIXR_PATCH_LOG(log, info) sys_log(log, info);
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF)
#define MATRIXR_PATCH_LOG(log, info) do{}while(0)
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF)*/

static UINT32 matrix_r_block_squ_self(const UINT32 matrixr_md_id, MATRIX_BLOCK *des_matrix_block);

#if 0
static UINT32 matrix_r_block_mul(const UINT32 matrixr_md_id,
                                 const MATRIX_BLOCK *src_matrix_block_1,
                                 const MATRIX_BLOCK *src_matrix_block_2,
                                 MATRIX_BLOCK *des_matrix_block);
#endif

static UINT32 matrix_r_block_mul_self_front(const UINT32 matrixr_md_id,
                                  const MATRIX_BLOCK *src_matrix_block,
                                  MATRIX_BLOCK *des_matrix_block);

static UINT32 matrix_r_block_mul_self_rear(const UINT32 matrixr_md_id,
                                  const MATRIX_BLOCK *src_matrix_block,
                                  MATRIX_BLOCK *des_matrix_block);
/**
*   for test only
*
*   to query the status of MATRIXR Module
*
**/
void matrix_r_print_module_status(const UINT32 matrixr_md_id, LOG *log)
{
    MATRIXR_MD *matrixr_md;
    UINT32 this_matrixr_md_id;

    for( this_matrixr_md_id = 0; this_matrixr_md_id < MATRIXR_MD_CAPACITY(); this_matrixr_md_id ++ )
    {
        matrixr_md = MATRIXR_MD_GET(this_matrixr_md_id);

        if ( NULL_PTR != matrixr_md && 0 < matrixr_md->usedcounter )
        {
            sys_log(log,"MATRIXR Module # %ld : %ld refered, refer REAL Module : %ld\n",
                    this_matrixr_md_id,
                    matrixr_md->usedcounter,
                    matrixr_md->real_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed MATRIXR module
*
*
**/
UINT32 matrix_r_free_module_static_mem(const UINT32 matrixr_md_id)
{
    MATRIXR_MD  *matrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_free_module_static_mem: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    free_module_static_mem(MD_MATRIXR, matrixr_md_id);

    return 0;
}

/**
*
* start MATRIXR module
*
**/
UINT32 matrix_r_start( )
{
    MATRIXR_MD *matrixr_md;
    UINT32 matrixr_md_id;

    UINT32 real_md_id;
    UINT32 vectorr_md_id;

    matrixr_md_id = cbc_md_new(MD_MATRIXR, sizeof(MATRIXR_MD));
    if(ERR_MODULE_ID == matrixr_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one MATRIXR module */
    matrixr_md = (MATRIXR_MD *)cbc_md_get(MD_MATRIXR, matrixr_md_id);
    matrixr_md->usedcounter   = 0;
    matrixr_md->real_md_id    = ERR_MODULE_ID;
    matrixr_md->vectorr_md_id = ERR_MODULE_ID;

    /* create a new module node */
    real_md_id = ERR_MODULE_ID;
    vectorr_md_id = ERR_MODULE_ID;

    init_static_mem();

    /*default setting which will be override after matrix_r_set_mod_mgr calling*/
    matrixr_md->mod_mgr = mod_mgr_new(matrixr_md_id, LOAD_BALANCING_LOOP);

    real_md_id = real_start(); /*no REAL_MD need to alloc, here is one trick at present*/
    vectorr_md_id = vector_r_start();

    matrixr_md->real_md_id = real_md_id;
    matrixr_md->vectorr_md_id = vectorr_md_id;
    matrixr_md->usedcounter = 1;

    sys_log(LOGSTDOUT, "matrix_r_start: start MATRIXR module #%ld\n", matrixr_md_id);
    //sys_log(LOGSTDOUT, "========================= matrix_r_start: MATRIXR table info:\n");
    //matrix_r_print_module_status(matrixr_md_id, LOGSTDOUT);
    //cbc_print();

    return ( matrixr_md_id );
}

/**
*
* end MATRIXR module
*
**/
void matrix_r_end(const UINT32 matrixr_md_id)
{
    MATRIXR_MD *matrixr_md;

    UINT32 real_md_id;
    UINT32 vectorr_md_id;

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    if(NULL_PTR == matrixr_md)
    {
        sys_log(LOGSTDOUT,"error:matrix_r_end: matrixr_md_id = %ld not exist.\n", matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < matrixr_md->usedcounter )
    {
        matrixr_md->usedcounter --;
        return ;
    }

    if ( 0 == matrixr_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_end: matrixr_md_id = %ld is not started.\n", matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }

    //task_brd_mod_mgr_rmv(matrixr_md->task_brd, matrixr_md->mod_mgr);
    mod_mgr_free(matrixr_md->mod_mgr);
    matrixr_md->mod_mgr  = NULL_PTR;

    /* if nobody else occupied the module,then free its resource */
    real_md_id = matrixr_md->real_md_id;
    vectorr_md_id = matrixr_md->vectorr_md_id;

    real_end(real_md_id);
    vector_r_end(vectorr_md_id);

    /* free module : */
    //matrix_r_free_module_static_mem(matrixr_md_id);

    matrixr_md->real_md_id = ERR_MODULE_ID;
    matrixr_md->vectorr_md_id = ERR_MODULE_ID;
    matrixr_md->usedcounter = 0;

    sys_log(LOGSTDOUT, "matrix_r_end: stop MATRIXR module #%ld\n", matrixr_md_id);
    cbc_md_free(MD_MATRIXR, matrixr_md_id);

    breathing_static_mem();

    //sys_log(LOGSTDOUT, "========================= matrix_r_end: MATRIXR table info:\n");
    //matrix_r_print_module_status(matrixr_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}


/**
*
* initialize mod mgr of MATRIXR module
*
**/
UINT32 matrix_r_set_mod_mgr(const UINT32 matrixr_md_id, const MOD_MGR * src_mod_mgr)
{
    MATRIXR_MD *matrixr_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_set_mod_mgr: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        matrix_r_print_module_status(matrixr_md_id, LOGSTDOUT);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    des_mod_mgr = matrixr_md->mod_mgr;

    sys_log(LOGSTDOUT, "matrix_r_set_mod_mgr: md_id %d, des_mod_mgr %lx\n", matrixr_md_id, des_mod_mgr);

    mod_mgr_limited_clone(matrixr_md_id, src_mod_mgr, des_mod_mgr);
    return (0);
}

/**
*
* get mod mgr of MATRIXR module
*
**/
MOD_MGR * matrix_r_get_mod_mgr(const UINT32 matrixr_md_id)
{
    MATRIXR_MD *matrixr_md;

    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        return (MOD_MGR *)0;
    }

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    return (matrixr_md->mod_mgr);
}


#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF)
UINT32 matrix_r_init_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;
    UINT32 rotated_flag;

    rotated_flag = 0; /*default*/

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, rotated_flag);

    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, MATRIX_VECTOR_WIDTH);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, MATRIX_VECTOR_WIDTH);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, NULL_PTR);
    }

    return (0);
}
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF)*/

#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
UINT32 matrix_r_init_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;
    UINT32 rotated_flag;

    REAL *data_addr;
    REAL *data_buff;

    rotated_flag = 0; /*default*/

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, rotated_flag);

    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, MATRIX_VECTOR_WIDTH);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, MATRIX_VECTOR_WIDTH);

    /*NOTE: in order to cover table g_type_conv_list requirement, insert data_buff allocation here!!*/
    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MATRIXR_PATCH_01_BUFF_TYPE, &data_buff, LOC_MATRIXR_0001);/*patch*/
    MATRIX_BLOCK_DATA_BUFF_SET(matrix_block, data_buff);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = data_buff + sub_row_idx * MATRIX_VECTOR_WIDTH + sub_col_idx;
        REAL_SETZERO(0, (*data_addr));
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, data_addr);/*patch*/
    }

    return (0);
}
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
UINT32 matrix_r_init_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;
    UINT32 rotated_flag;

    REAL *data_addr;

    rotated_flag = 0; /*default*/

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrix_block, rotated_flag);

    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, MATRIX_VECTOR_WIDTH);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, MATRIX_VECTOR_WIDTH);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        REAL_SETZERO(0, (*data_addr));
    }
    return (0);
}
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

UINT32 matrix_r_alloc_block( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX_BLOCK **matrix_block)
{
    if ( MD_MATRIXR != matrixr_md_type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_alloc_block: invalid matrixr_md_type = %ld\n", matrixr_md_type);
        exit(0);
    }

    if ( MM_MATRIX_BLOCK != type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_alloc_block: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(matrixr_md_type, matrixr_md_id, type, matrix_block, LOC_MATRIXR_0002);
    matrix_r_init_block(matrixr_md_id, *matrix_block);

    return (0);
}

UINT32 matrix_r_new_block(const UINT32 matrixr_md_id, MATRIX_BLOCK **matrix_block)
{
    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, matrix_block, LOC_MATRIXR_0003);
    matrix_r_init_block(matrixr_md_id, *matrix_block);

    return (0);
}

#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF)
static UINT32 matrix_r_free_data_area( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    REAL *data_addr;

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != data_addr )
        {
            free_static_mem(matrixr_md_type, matrixr_md_id, MM_REAL, data_addr, LOC_MATRIXR_0004);/*remove this line when patch*/
            MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, NULL_PTR);
        }
    }
    return (0);
}
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF)*/


#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
static UINT32 matrix_r_free_data_area( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    REAL *data_addr;

    REAL *data_buff;

    data_buff = MATRIX_BLOCK_DATA_BUFF(matrix_block);
    if(NULL_PTR != data_buff)
    {
        free_static_mem(matrixr_md_type, matrixr_md_id, MATRIXR_PATCH_01_BUFF_TYPE, data_buff, LOC_MATRIXR_0005);/*patch*/
        MATRIX_BLOCK_DATA_BUFF_SET(matrix_block, NULL_PTR);/*patch*/
    }

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != data_addr )
        {
            MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, NULL_PTR);
        }
    }
    return (0);
}
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
static UINT32 matrix_r_free_data_area( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    REAL *data_addr;

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, MATRIX_VECTOR_WIDTH, sub_col_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != data_addr )
        {
            REAL_SETZERO(0, (*data_addr));
        }
    }
    return (0);
}
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/


/**
*
* clear data_area but do not free block itself
*
**/
static UINT32 matrix_r_clear_block( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    if ( MD_MATRIXR != matrixr_md_type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_clear_block: invalid matrixr_md_type = %ld\n", matrixr_md_type);
        exit(0);
    }

    matrix_r_free_data_area(matrixr_md_type, matrixr_md_id, matrix_block);

    return (0);
}

/**
*
* clean data_area but do not free block itself
*
**/
UINT32 matrix_r_clean_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    matrix_r_free_data_area(MD_MATRIXR, matrixr_md_id, matrix_block);

    return (0);
}

UINT32 matrix_r_free_block( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX_BLOCK *matrix_block)
{
    if ( MM_MATRIX_BLOCK != type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_free_block: invalid type = %ld\n",type);
        exit(0);
    }

    matrix_r_clear_block(matrixr_md_type, matrixr_md_id, matrix_block);
    free_static_mem(matrixr_md_type, matrixr_md_id, type, matrix_block, LOC_MATRIXR_0006);

    return (0);
}

UINT32 matrix_r_destroy_block(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    matrix_r_clear_block(MD_MATRIXR, matrixr_md_id, matrix_block);
    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, matrix_block, LOC_MATRIXR_0007);
    return (0);
}

UINT32 matrix_r_init_matrix(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    MATRIX_SET_ROTATED_FLAG(matrix, 0);
    MATRIX_SET_ROW_NUM(matrix, 0);
    MATRIX_SET_COL_NUM(matrix, 0);

    MATRIX_SET_BLOCKS(matrix, 0);

    return (0);
}

static UINT32 matrix_r_alloc_matrix( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX **matrix)
{
    if ( MD_MATRIXR != matrixr_md_type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_alloc_matrix: invalid matrixr_md_type = %ld\n", matrixr_md_type);
        exit(0);
    }
    if ( MM_MATRIX != type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_alloc_matrix: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(matrixr_md_type, matrixr_md_id, type, matrix, LOC_MATRIXR_0008);

    matrix_r_init_matrix(matrixr_md_id, *matrix);

    return (0);
}

static UINT32 matrix_r_free_matrix( const UINT32 matrixr_md_type, const UINT32 matrixr_md_id, const UINT32 type, MATRIX *matrix)
{
    if ( MD_MATRIXR != matrixr_md_type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_free_matrix: invalid matrixr_md_type = %ld\n", matrixr_md_type);
        exit(0);
    }
    if ( MM_MATRIX != type )
    {
        sys_log(LOGSTDOUT,"error:matrix_r_free_matrix: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(matrixr_md_type, matrixr_md_id, type, matrix, LOC_MATRIXR_0009);

    return (0);
}

UINT32 matrix_r_new_matrix_skeleton(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX *matrix)
{
    UINT32 blocks_row_num;
    UINT32 blocks_col_num;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    MATRIX_BLOCK  *matrix_block;
    CVECTOR *blocks;

    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_new_matrix_skeleton: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_new_matrix_skeleton: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    /*set rotated_flag to default 0*/
    MATRIX_SET_ROTATED_FLAG(matrix, 0);

    /*set row num and col num to matrix. not sure if it is suitable or not here*/
    MATRIX_SET_ROW_NUM(matrix, row_num);
    MATRIX_SET_COL_NUM(matrix, col_num);

    blocks_row_num = MATRIX_GET_ROW_BLOCKS_NUM(matrix);
    blocks_col_num = MATRIX_GET_COL_BLOCKS_NUM(matrix);

    /*create matrix header*/
    blocks = cvector_new(blocks_row_num * blocks_col_num, MM_MATRIX_BLOCK, LOC_MATRIXR_0010);
    MATRIX_SET_BLOCKS(matrix, blocks);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &matrix_block);
        cvector_push((CVECTOR *)blocks, matrix_block);
    }

    /*adjust row num of last row header and its blocks if necessary*/
    sub_row_num = (row_num % MATRIX_VECTOR_WIDTH);
    if( 0 < sub_row_num )
    {
        block_row_idx = (blocks_row_num - 1);
        MATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx)
        {
            matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
            MATRIX_BLOCK_SET_ROW_NUM(matrix_block, sub_row_num);
        }
    }

    /*adjust col num of last col header and its blocks if necessary*/
    sub_col_num = (col_num % MATRIX_VECTOR_WIDTH);
    if( 0 < sub_col_num )
    {
        block_col_idx = (blocks_col_num - 1);
        MATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx)
        {
            matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
            MATRIX_BLOCK_SET_COL_NUM(matrix_block, sub_col_num);
        }
    }

    return (0);
}

UINT32 matrix_r_new_matrix(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX **ppmatrix)
{
    if( NULL_PTR == ppmatrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_new_matrix: ppmatrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_new_matrix: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    /*create matrix node itself*/
    matrix_r_alloc_matrix(MD_MATRIXR, matrixr_md_id, MM_MATRIX, ppmatrix);

    /*create matrix skeleton*/
    matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, (*ppmatrix));

    return (0);
}

/**
*
* clear data_area but do not free skeleton
*
**/
UINT32 matrix_r_clear_matrix(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_clear_matrix: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_clear_matrix: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    /*clear blocks*/
    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_clear_block(MD_MATRIXR, matrixr_md_id, matrix_block);
    }

    return (0);
}

/**
*
* destroy skeleton but do not free matrix itself
*
**/
UINT32 matrix_r_clean_matrix(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_clean_matrix: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_clean_matrix: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    /*destroy blocks*/
    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, matrix_block);
        MATRIX_SET_BLOCK(matrix, block_row_idx, block_col_idx, NULL_PTR);/*clean pointer*/
    }

    /*destroy matrix header */
    if ( 0 != MATRIX_GET_BLOCKS(matrix))
    {
        cvector_free((CVECTOR *)MATRIX_GET_BLOCKS(matrix), LOC_MATRIXR_0011);
        MATRIX_SET_BLOCKS(matrix, 0);
    }

    /*reset row num and col num to zero*/
    MATRIX_SET_ROW_NUM(matrix, 0);
    MATRIX_SET_COL_NUM(matrix, 0);

    /*reset rotated_flag to zero*/
    MATRIX_SET_ROTATED_FLAG(matrix, 0);

    return (0);
}

/**
*
* move src_matrix skeleton and data area to des_matrix
* note:
*     des_matrix must have no skeleton or data area. otherwise, skeleton and data area will be lost without free
*
**/
UINT32 matrix_r_move_matrix(const UINT32 matrixr_md_id, MATRIX *src_matrix, MATRIX *des_matrix)
{
    /*move src_matrix header to des_matrix header where  src_matrix header will be clean up*/
    MATRIX_SET_BLOCKS(des_matrix, MATRIX_GET_BLOCKS(src_matrix));

    /*copy src_matrix info to des_matrix*/
    MATRIX_SET_ROTATED_FLAG(des_matrix, MATRIX_GET_ROTATED_FLAG(src_matrix));
    MATRIX_SET_ROW_NUM(des_matrix, MATRIX_GET_ROW_NUM(src_matrix));
    MATRIX_SET_COL_NUM(des_matrix, MATRIX_GET_COL_NUM(src_matrix));

    /*clean up src_matrix info*/
    MATRIX_SET_BLOCKS(src_matrix, 0);
    MATRIX_SET_ROTATED_FLAG(src_matrix, 0);
    MATRIX_SET_ROW_NUM(src_matrix, 0);
    MATRIX_SET_COL_NUM(src_matrix, 0);

    return (0);
}

/**
*
* destroy a matrix
* all data area pointers, block pointers, header pointers, and matrix itself pointer will be destroyed.
* note:
*       matrix pointer cannot be reused after calling
*
**/
UINT32 matrix_r_destroy_matrix(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    if( NULL_PTR == matrix )
    {
        return (0);
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_destroy_matrix: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrix_r_clean_matrix(matrixr_md_id, matrix);

    /*destroy matrix node itself*/
    matrix_r_free_matrix(MD_MATRIXR, matrixr_md_id, MM_MATRIX, matrix);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_get_block(const UINT32 matrixr_md_id, const MATRIX *matrix, const UINT32 row_no, const UINT32 col_no, MATRIX_BLOCK **ppmatrix_block)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    if( row_no >= MATRIX_GET_ROW_NUM(matrix) )
    {
        *ppmatrix_block = NULL_PTR;
        return ((UINT32)(-1));
    }

    if( col_no >= MATRIX_GET_COL_NUM(matrix) )
    {
        *ppmatrix_block = NULL_PTR;
        return ((UINT32)(-1));
    }

    block_row_idx = (row_no / MATRIX_VECTOR_WIDTH);
    block_col_idx = (col_no / MATRIX_VECTOR_WIDTH);

    (*ppmatrix_block) = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_set_block(const UINT32 matrixr_md_id, const UINT32 row_no, const UINT32 col_no, const MATRIX_BLOCK *matrix_block, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    if( row_no >= MATRIX_GET_ROW_NUM(matrix) )
    {
        sys_log(LOGSTDOUT, "error:matrix_r_set_block: row_no %ld overflow where matrix is (%ld, %ld)\n",
                         row_no,
                         MATRIX_GET_ROW_NUM(matrix),
                         MATRIX_GET_COL_NUM(matrix));
        return ((UINT32)(-1));
    }

    if( col_no >= MATRIX_GET_COL_NUM(matrix) )
    {
        sys_log(LOGSTDOUT, "error:matrix_r_set_block: col_no %ld overflow where matrix is (%ld, %ld)\n",
                         col_no,
                         MATRIX_GET_ROW_NUM(matrix),
                         MATRIX_GET_COL_NUM(matrix));
        return ((UINT32)(-1));
    }

    block_row_idx = (row_no / MATRIX_VECTOR_WIDTH);
    block_col_idx = (col_no / MATRIX_VECTOR_WIDTH);

    MATRIX_SET_BLOCK(matrix, block_row_idx, block_col_idx, matrix_block);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_get_data_addr(const UINT32 matrixr_md_id, const MATRIX *matrix, const UINT32 row_no, const UINT32 col_no, REAL **ppdata_addr)
{
    MATRIX_BLOCK *matrix_block;
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    matrix_r_get_block(matrixr_md_id, matrix, row_no, col_no, &matrix_block);

    sub_row_idx = (row_no % MATRIX_VECTOR_WIDTH);
    sub_col_idx = (col_no % MATRIX_VECTOR_WIDTH);

    (*ppdata_addr) = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_set_data_addr(const UINT32 matrixr_md_id, const UINT32 row_no, const UINT32 col_no, const REAL *data_addr, MATRIX *matrix)
{
    MATRIX_BLOCK *matrix_block;
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    matrix_r_get_block(matrixr_md_id, matrix, row_no, col_no, &matrix_block);
    sub_row_idx = (row_no % MATRIX_VECTOR_WIDTH);
    sub_col_idx = (col_no % MATRIX_VECTOR_WIDTH);

    MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx, data_addr);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_get_data_val(const UINT32 matrixr_md_id, const MATRIX *matrix, const UINT32 row_no, const UINT32 col_no, REAL *pdata_addr)
{
    MATRIX_BLOCK *matrix_block;
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    matrix_r_get_block(matrixr_md_id, matrix, row_no, col_no, &matrix_block);
    sub_row_idx = (row_no % MATRIX_VECTOR_WIDTH);
    sub_col_idx = (col_no % MATRIX_VECTOR_WIDTH);

    (*pdata_addr) = MATRIXR_BLOCK_GET_DATA_VAL(matrix_block, sub_row_idx, sub_col_idx);

    return (0);
}

/**
*
* row_no/col_no value range 0,1,2,....
*
**/
static UINT32 matrix_r_set_data_val(const UINT32 matrixr_md_id, const UINT32 row_no, const UINT32 col_no, const REAL *data_addr, MATRIX *matrix)
{
    MATRIX_BLOCK *matrix_block;
    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    matrix_r_get_block(matrixr_md_id, matrix, row_no, col_no, &matrix_block);
    sub_row_idx = (row_no % MATRIX_VECTOR_WIDTH);
    sub_col_idx = (col_no % MATRIX_VECTOR_WIDTH);

    MATRIXR_BLOCK_SET_DATA_VAL(matrix_block, sub_row_idx, sub_col_idx, *data_addr);

    return (0);
}


/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks only
* note: pdata[] is one-dimension array carrying on REAL pointer
*
**/
static UINT32 matrix_r_insert_row(const UINT32 matrixr_md_id, const REAL *pdata[], const UINT32 max_data_num, UINT32 *position, const UINT32 row_no, MATRIX *matrix)
{
    UINT32 col_no;
    UINT32 col_num;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_insert_row: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    col_num = MATRIX_GET_COL_NUM(matrix);
    for(col_no = 0; col_no < col_num && (*position) < max_data_num; col_no ++, (*position) ++)
    {
        matrix_r_set_data_val(matrixr_md_id, row_no, col_no, pdata[ *position ], matrix);
    }

    return (0);
}

/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: pdata[] is one-dimension array carrying on REAL pointer
*
**/
static UINT32 matrix_r_insert_rows_data_from_row(const UINT32 matrixr_md_id, const REAL *pdata[], const UINT32 max_data_num, MATRIX *matrix)
{
    UINT32 row_num;
    UINT32 row_no;
    UINT32 position;

    row_num = MATRIX_GET_ROW_NUM(matrix);
    position = 0;

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        if(position >= max_data_num)
        {
            sys_log(LOGSTDERR, "error:matrix_r_insert_rows_data_from_row: max_data_num = %ld is not enough to fill the matrix (%ld, %ld)\n",
                            max_data_num,
                            MATRIX_GET_ROW_NUM(matrix),
                            MATRIX_GET_COL_NUM(matrix));
            return ((UINT32)(-1));
        }

        matrix_r_insert_row(matrixr_md_id, pdata, max_data_num, &position, row_no, matrix);
    }

    return (0);
}


/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: ppdata[] is two-dimension array carrying on REAL pointer
*
**/
static UINT32 matrix_r_insert_rows_data_from_tbl(const UINT32 matrixr_md_id, const REAL **ppdata[], const UINT32 row_num, const UINT32 col_num, MATRIX *matrix)
{
    UINT32 row_no;
    UINT32 position;

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        position = 0;
        matrix_r_insert_row(matrixr_md_id, ppdata[ row_no ], col_num, &position, row_no, matrix);
    }

    return (0);
}


/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: pdata[] is one-dimension array (row vector) carrying on REAL pointer
*
**/
UINT32 matrix_r_insert_data_by_row(const UINT32 matrixr_md_id, const REAL *pdata[], const UINT32 data_num, MATRIX *matrix)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_insert_data_by_row: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_insert_data_by_row: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrix_r_insert_rows_data_from_row(matrixr_md_id, pdata, data_num, matrix);
    return( 0 );
}

/**
*
* here not consider the insertion row into one existing matrix with data
* just insert the row into one empty matrix with skeleton only including matrix node itself, headers, blocks
* note: ppdata[] is two-dimension array carrying on REAL pointer
*
**/
UINT32 matrix_r_insert_data_by_tbl(const UINT32 matrixr_md_id, const REAL **ppdata[], const UINT32 row_num, const UINT32 col_num, MATRIX *matrix)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_insert_data_by_tbl: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_insert_data_by_tbl: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrix_r_insert_rows_data_from_tbl(matrixr_md_id, ppdata, row_num, col_num, matrix);
    return( 0 );
}

UINT32 matrix_r_clone_block(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_clone_block: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_clone_block: [1]should not enter here after patch\n");
                alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0012);
                MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            }

            REAL_CLONE(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
        }
        else
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_clone_block: [2]should not enter here after patch\n");
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, des_data_addr, LOC_MATRIXR_0013);
            }
        }
    }
    return ( 0 );
}

/**
*
*  matrix_block = - matrix_block
*
**/
static UINT32 matrix_r_block_neg_self(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_neg_self: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        if( NULL_PTR != data_addr )
        {
            data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
            if( NULL_PTR != data_addr )
            {
                REAL_NEG(matrixr_md->real_md_id,(*data_addr), (*data_addr));
            }
        }
    }
    return ( 0 );
}

/**
*
*  des_matrix_block = - src_matrix_block
*  note:
*      here must addr(des_matrix_block) != addr(src_matrix_block)
*
**/
static UINT32 matrix_r_block_neg(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_neg: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

   MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
   {
        src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_neg: [1]should not enter here after patch\n");
                alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0014);
                MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            }

            src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
            REAL_NEG(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
        }
        else
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_neg: [2]should not enter here after patch\n");
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, des_data_addr, LOC_MATRIXR_0015);
            }
        }
    }
    return ( 0 );
}

/**
*
*  matrix_block is zero or not
*  note:
*      here matrix_block must not be null pointer
*
**/
static EC_BOOL matrix_r_block_is_zero(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_is_zero: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        if( NULL_PTR != data_addr && (EC_FALSE == REAL_ISZERO(matrixr_md->real_md_id,(*data_addr))) )
        {
            return (EC_FALSE);
        }
    }
    return ( EC_TRUE );
}

/**
*
*  matrix_block is one or not
*  note:
*      here matrix_block must not be null pointer
*
**/
static EC_BOOL matrix_r_block_is_one(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_is_one: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        if( sub_row_idx == sub_col_idx )
        {
            if( NULL_PTR == data_addr || (EC_FALSE == REAL_ISONE(matrixr_md->real_md_id,(*data_addr))) )
            {
                return (EC_FALSE);
            }
        }
        else
        {
            if( NULL_PTR != data_addr && (EC_FALSE == REAL_ISZERO(matrixr_md->real_md_id,(*data_addr))) )
            {
                return (EC_FALSE);
            }
        }
    }
    return ( EC_TRUE );
}

/**
*
*  matrix_block_1 is equal to matrix_block_2 or not
*  note:
*      here matrix_block_1 and matrix_block_2 must not be null pointer
*
**/
void print_real_bytes(REAL *data)
{
    UINT8 *bytes;
    UINT32 idx;

    bytes = (UINT8 *)data;
    sys_print(LOGSTDOUT, "[%lx] (", data);
    for(idx = 0; idx < sizeof(REAL)/sizeof(UINT8); idx ++)
    {
        sys_print(LOGSTDOUT, "%02x,", bytes[idx]);
    }
    sys_print(LOGSTDOUT, ")\n");
}
static EC_BOOL matrix_r_block_cmp(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block_1, const MATRIX_BLOCK *matrix_block_2)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr_1;
    REAL *data_addr_2;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_cmp: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block_1);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block_1);
#if 0
    sys_log(LOGSTDOUT, "matrix_r_block_cmp: cmp type (%ld, %ld) <---> (%ld, %ld)\n",
                    sub_row_num, sub_col_num,
                    MATRIX_BLOCK_GET_ROW_NUM(rotated_flag_2, matrix_block_2), MATRIX_BLOCK_GET_COL_NUM(rotated_flag_2, matrix_block_2));

#endif
    if( sub_row_num != MATRIX_BLOCK_GET_ROW_NUM(matrix_block_2) )
    {
        sys_log(LOGSTDOUT, "matrix_r_block_cmp: type mismatched (%ld, %ld) <---> (%ld, %ld)\n",
                        sub_row_num, sub_col_num,
                        MATRIX_BLOCK_GET_ROW_NUM(matrix_block_2), MATRIX_BLOCK_GET_COL_NUM(matrix_block_2));
        return (EC_FALSE);
    }

    if( sub_col_num != MATRIX_BLOCK_GET_COL_NUM(matrix_block_2) )
    {
        sys_log(LOGSTDOUT, "matrix_r_block_cmp: type mismatched (%ld, %ld) <---> (%ld, %ld)\n",
                        sub_row_num, sub_col_num,
                        MATRIX_BLOCK_GET_ROW_NUM(matrix_block_2), MATRIX_BLOCK_GET_COL_NUM(matrix_block_2));
        return (EC_FALSE);
    }

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_1, sub_row_idx, sub_col_idx);
        data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_2, sub_row_idx, sub_col_idx);

        if( data_addr_1 == data_addr_2 )
        {
            continue;
        }

        if( NULL_PTR == data_addr_1 || NULL_PTR == data_addr_2 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_cmp: should not enter here after patch\n");
            return (EC_FALSE);
        }

        if( EC_FALSE == REAL_ISEQU(matrixr_md->real_md_id,(*data_addr_1), (*data_addr_2)) )
        {
            if((*data_addr_1) > (*data_addr_2))
            {
                sys_log(LOGSTDOUT, "matrix_r_block_cmp: not equal: [matrix_block %lx, data %lx] %lf > [matrix_block %lx, data %lx], %lf\n",
                                matrix_block_1, data_addr_1, *data_addr_1,
                                matrix_block_2, data_addr_2, *data_addr_2);
            }
            if((*data_addr_1) < (*data_addr_2))
            {
                sys_log(LOGSTDOUT, "matrix_r_block_cmp: not equal: [matrix_block %lx, data %lx] %lf < [matrix_block %lx, data %lx], %lf\n",
                                matrix_block_1, data_addr_1, *data_addr_1,
                                matrix_block_2, data_addr_2, *data_addr_2);
            }
            sys_log(LOGSTDOUT, "matrix_r_block_cmp: not equal: [matrix_block %lx, data %lx] %lf, [matrix_block %lx, data %lx], %lf: diff = %lf\n",
                            matrix_block_1, data_addr_1, *data_addr_1,
                            matrix_block_2, data_addr_2, *data_addr_2,
                            (*data_addr_1) - (*data_addr_2));
            print_real_bytes(data_addr_1);
            print_real_bytes(data_addr_2);
            sys_print(LOGSTDOUT, "\n");
            return (EC_FALSE);
        }
    }
    return ( EC_TRUE );
}

/**
*
*  matrix_block = 0
*  note:
*
**/
static UINT32 matrix_r_block_set_zero(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_set_zero: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        if( NULL_PTR != data_addr )
        {
            REAL_SETZERO(matrixr_md->real_md_id,(*data_addr));
        }
    }

    return (0);
}

/**
*
*  matrix_block = 1
*  note:
*    1. row num of matrix block must be same as col_num
*
**/
static UINT32 matrix_r_block_set_one(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_idx;

    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_set_one: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    /*set matrix block to zero at first*/
    matrix_r_block_set_zero(matrixr_md_id, matrix_block);

    sub_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_idx, sub_idx);
        if( NULL_PTR == data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_set_one: should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &data_addr, LOC_MATRIXR_0016);
            MATRIX_BLOCK_SET_DATA_ADDR(matrix_block, sub_idx, sub_idx, data_addr);
        }
        REAL_SETONE(matrixr_md->real_md_id,(*data_addr));
    }

    return (0);
}

/**
*
*  des_matrix_block += src_matrix_block
*  note:
*      here des_matrix_block must not be src_matrix_block
*
**/
UINT32 matrix_r_block_adc(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_adc: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR == src_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_adc: [1]should not enter here after patch\n");
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_adc: [2]should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0017);
            MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            REAL_CLONE(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
            continue;
        }

        REAL_ADC(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_matrix_block = src_matrix_block_1 + src_matrix_block_2
*  note:
*      here des_matrix_block must not be src_matrix_block_1 or src_matrix_block_2
*
**/
static UINT32 matrix_r_block_add(const UINT32 matrixr_md_id,
                                 const MATRIX_BLOCK *src_matrix_block_1,
                                 const MATRIX_BLOCK *src_matrix_block_2,
                                 MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_add: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_1);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_1);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        src_data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_1, sub_row_idx, sub_col_idx);
        src_data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_2, sub_row_idx, sub_col_idx);
        des_data_addr   = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block  , sub_row_idx, sub_col_idx);

        if( NULL_PTR == src_data_addr_1 && NULL_PTR == src_data_addr_2 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_add: [1]should not enter here after patch\n");
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, des_data_addr, LOC_MATRIXR_0018);
                MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            }
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_add: [2]should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0019);
            MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
        }

        if( NULL_PTR == src_data_addr_1 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_add: [3]should not enter here after patch\n");
            REAL_CLONE(matrixr_md->real_md_id,(*src_data_addr_2), (*des_data_addr));
            continue;
        }

        if( NULL_PTR == src_data_addr_2 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_add: [4]should not enter here after patch\n");
            REAL_CLONE(matrixr_md->real_md_id,(*src_data_addr_1), (*des_data_addr));
            continue;
        }

        REAL_ADD(matrixr_md->real_md_id,(*src_data_addr_1), (*src_data_addr_2), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_matrix_block -= src_matrix_block
*  note:
*      here des_matrix_block must not be src_matrix_block
*
**/
static UINT32 matrix_r_block_sbb(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_sbb: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR == src_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sbb: [1]should not enter here after patch\n");
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sbb: [2]should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0020);
            MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            REAL_NEG(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
            continue;
        }

        REAL_SBB(matrixr_md->real_md_id,(*src_data_addr), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_matrix_block = src_matrix_block_1 - src_matrix_block_2
*  note:
*      here des_matrix_block must not be src_matrix_block_1 or src_matrix_block_2
*
**/
static UINT32 matrix_r_block_sub(const UINT32 matrixr_md_id,
                                 const MATRIX_BLOCK *src_matrix_block_1,
                                 const MATRIX_BLOCK *src_matrix_block_2,
                                 MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_sub: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_1);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_1);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        src_data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_1, sub_row_idx, sub_col_idx);
        src_data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_2, sub_row_idx, sub_col_idx);
        des_data_addr   = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block  , sub_row_idx, sub_col_idx);

        if( NULL_PTR == src_data_addr_1 && NULL_PTR == src_data_addr_2 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sub: [1]should not enter here after patch\n");
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, des_data_addr, LOC_MATRIXR_0021);
                MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            }
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sub: [2]should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0022);
            MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
        }

        if( NULL_PTR == src_data_addr_1 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sub: [3]should not enter here after patch\n");
            REAL_NEG(matrixr_md->real_md_id,(*src_data_addr_2), (*des_data_addr));
            continue;
        }

        if( NULL_PTR == src_data_addr_2 )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_sub: [4]should not enter here after patch\n");
            REAL_CLONE(matrixr_md->real_md_id,(*src_data_addr_1), (*des_data_addr));
            continue;
        }

        REAL_SUB(matrixr_md->real_md_id,(*src_data_addr_1), (*src_data_addr_2), (*des_data_addr));
    }
    return (0);
}

/**
*
*  matrix_block.row[ row_idx ]  = matrix_block.row[ row_idx ] / r_data
*  note:
*
**/
static UINT32 matrix_r_shrink_single_row(const UINT32 matrixr_md_id, const REAL *r_data_addr, const UINT32 row_no, const MATRIX *matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 col_num;
    UINT32 col_no;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_shrink_single_row: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    if(EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id,(*r_data_addr)))
    {
        sys_log(LOGSTDOUT, "error:matrix_r_shrink_single_row: r_data_addr is zero!\n", (*r_data_addr));
        return ((UINT32)(-1));
    }

    col_num = MATRIX_GET_COL_NUM(matrix);

    for(col_no = 0; col_no < col_num; col_no ++)
    {
        matrix_r_get_data_addr(matrixr_md_id, matrix, row_no, col_no, &data_addr);

        if( NULL_PTR != data_addr && EC_FALSE == REAL_ISZERO(matrixr_md->real_md_id,(*data_addr)) )
        {
            REAL_DIV(matrixr_md->real_md_id, (*data_addr), (*r_data_addr), (*data_addr));
        }
    }

    return (0);
}

/**
*
*  des_matrix_block.row[ des_row_idx ] = des_matrix_block.row[ des_row_idx ] - r_data * src_matrix_block.row[ src_row_idx ]
*  note:
*      1. col num of src_matrix_block must be same as that of des_matrix_block
*
**/
static UINT32 matrix_r_shrink_rows(const UINT32 matrixr_md_id,
                                 const REAL *r_data_addr,
                                 const UINT32 src_row_no, const MATRIX *src_matrix,
                                 const UINT32 des_row_no, MATRIX *des_matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 col_num;
    UINT32 col_no;

    REAL *src_data_addr;
    REAL *des_data_addr;
    REAL *tmp_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_shrink_rows: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    col_num = MATRIX_GET_COL_NUM(des_matrix);
    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &tmp_data_addr, LOC_MATRIXR_0023);

    for(col_no = 0; col_no < col_num; col_no ++)
    {
        matrix_r_get_data_addr(matrixr_md_id, src_matrix, src_row_no, col_no, &src_data_addr);
        matrix_r_get_data_addr(matrixr_md_id, des_matrix, des_row_no, col_no, &des_data_addr);

        if( NULL_PTR == src_data_addr || EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id,*src_data_addr) )
        {
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_shrink_rows: should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0024);
            matrix_r_set_data_addr(matrixr_md_id, des_row_no, col_no, des_data_addr, des_matrix);

            REAL_MUL(matrixr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
            REAL_NEG(matrixr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
            continue;
        }

        REAL_MUL(matrixr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
        REAL_SBB(matrixr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
    }

    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, tmp_data_addr, LOC_MATRIXR_0025);

    return (0);
}

/**
*
*  matrix_block.col[ col_idx ]  = matrix_block.col[ col_idx ] / r_data
*  note:
*
**/
static UINT32 matrix_r_shrink_single_col(const UINT32 matrixr_md_id, const REAL *r_data_addr, const UINT32 col_no, const MATRIX *matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 row_num;
    UINT32 row_no;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_shrink_single_col: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    if(EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id,(*r_data_addr)))
    {
        sys_log(LOGSTDOUT, "error:matrix_r_shrink_single_col: r_data_addr is zero!\n", (*r_data_addr));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(matrix);

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        matrix_r_get_data_addr(matrixr_md_id, matrix, row_no, col_no, &data_addr);

        if( NULL_PTR != data_addr && EC_FALSE == REAL_ISZERO(matrixr_md->real_md_id,(*data_addr)) )
        {
            REAL_DIV(matrixr_md->real_md_id, (*data_addr), (*r_data_addr), (*data_addr));
        }
    }

    return (0);
}


/**
*
*  des_matrix_block.col[ des_col_idx ] = des_matrix_block.col[ des_col_idx ] - r_data * src_matrix_block.col[ src_col_idx ]
*  note:
*      1. row num of src_matrix_block must be same as that of des_matrix_block
*
**/
static UINT32 matrix_r_shrink_cols(const UINT32 matrixr_md_id,
                                 const REAL *r_data_addr,
                                 const UINT32 src_col_no, const MATRIX *src_matrix,
                                 const UINT32 des_col_no, MATRIX *des_matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 row_num;
    UINT32 row_no;

    REAL *src_data_addr;
    REAL *des_data_addr;
    REAL *tmp_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_shrink_cols: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    row_num = MATRIX_GET_ROW_NUM(des_matrix);
    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &tmp_data_addr, LOC_MATRIXR_0026);

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        matrix_r_get_data_addr(matrixr_md_id, src_matrix, row_no, src_col_no, &src_data_addr);
        matrix_r_get_data_addr(matrixr_md_id, des_matrix, row_no, des_col_no, &des_data_addr);

        if( NULL_PTR == src_data_addr || EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id,*src_data_addr) )
        {
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_shrink_cols: should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0027);
            matrix_r_set_data_addr(matrixr_md_id, row_no, des_col_no, des_data_addr, des_matrix);

            REAL_MUL(matrixr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
            REAL_NEG(matrixr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
            continue;
        }

        REAL_MUL(matrixr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
        REAL_SBB(matrixr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
    }

    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, tmp_data_addr, LOC_MATRIXR_0028);

    return (0);
}

/**
*
*  matrix_block = s_data * matrix_block
*
**/
static UINT32 matrix_r_block_s_mul_self(const UINT32 matrixr_md_id, const REAL *s_data_addr, MATRIX_BLOCK *matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_s_mul_self: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
        if( NULL_PTR != data_addr )
        {
            data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_idx, sub_col_idx);
            if( NULL_PTR != data_addr )
            {
                REAL_MUL_SELF(matrixr_md->real_md_id,(*s_data_addr), (*data_addr));
            }
        }
    }
    return ( 0 );
}

/**
*
*  des_matrix_block = s_data * src_matrix_block
*  note:
*      here must addr(des_matrix_block) != addr(src_matrix_block)
*
**/
static UINT32 matrix_r_block_s_mul(const UINT32 matrixr_md_id, const REAL *s_data_addr, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_s_mul: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    sub_row_num  = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    sub_col_num  = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

   MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
   {
        src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_s_mul: [1]should not enter here after patch\n");
                alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_data_addr, LOC_MATRIXR_0029);
                MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, des_data_addr);
            }

            src_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, sub_row_idx, sub_col_idx);
            REAL_MUL(matrixr_md->real_md_id, (*s_data_addr), (*src_data_addr), (*des_data_addr));
        }
        else
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_s_mul: [2]should not enter here after patch\n");
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, des_data_addr, LOC_MATRIXR_0030);
            }
        }
    }
    return ( 0 );
}
/**
*
*  des_matrix_block = src_matrix_block * des_matrix_block
*
**/
static UINT32 matrix_r_block_mul_self_front(const UINT32 matrixr_md_id,const MATRIX_BLOCK *src_matrix_block,MATRIX_BLOCK *des_matrix_block)
{
    MATRIX_BLOCK *tmp_matrix_block;

    if( src_matrix_block == des_matrix_block )
    {
        return matrix_r_block_squ_self(matrixr_md_id, des_matrix_block);
    }

    matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &tmp_matrix_block);
    matrix_r_clone_block(matrixr_md_id, des_matrix_block, tmp_matrix_block);

    matrix_r_block_mul(matrixr_md_id, src_matrix_block, tmp_matrix_block, des_matrix_block);

    matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, tmp_matrix_block);

    return (0);
}

/**
*
*  des_matrix_block = des_matrix_block * matrix_r_block_mul_self
*
**/
static UINT32 matrix_r_block_mul_self_rear(const UINT32 matrixr_md_id,const MATRIX_BLOCK *src_matrix_block,MATRIX_BLOCK *des_matrix_block)
{
    MATRIX_BLOCK *tmp_matrix_block;

    if( src_matrix_block == des_matrix_block )
    {
        return matrix_r_block_squ_self(matrixr_md_id, des_matrix_block);
    }

    matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &tmp_matrix_block);
    matrix_r_clone_block(matrixr_md_id, des_matrix_block, tmp_matrix_block);

    matrix_r_block_mul(matrixr_md_id, tmp_matrix_block, src_matrix_block, des_matrix_block);

    matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, tmp_matrix_block);

    return (0);
}

/**
*
*  matrix_block = matrix_block ^ 2
*
**/
static UINT32 matrix_r_block_squ_self(const UINT32 matrixr_md_id, MATRIX_BLOCK *des_matrix_block)
{
    MATRIX_BLOCK *tmp_matrix_block;

    matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &tmp_matrix_block);
    matrix_r_clone_block(matrixr_md_id, des_matrix_block, tmp_matrix_block);

    matrix_r_block_mul(matrixr_md_id, tmp_matrix_block, tmp_matrix_block, des_matrix_block);

    matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, tmp_matrix_block);
    return (0);
}

/**
*
*  des_matrix_block = src_matrix_block ^ 2
*
**/
static UINT32 matrix_r_block_squ(const UINT32 matrixr_md_id, const MATRIX_BLOCK *src_matrix_block, MATRIX_BLOCK *des_matrix_block)
{
    if( src_matrix_block == des_matrix_block )
    {
        return matrix_r_block_squ_self(matrixr_md_id, des_matrix_block);
    }

    return matrix_r_block_mul(matrixr_md_id, src_matrix_block, src_matrix_block, des_matrix_block);
}

/**
*
*  des_matrix_block = src_matrix_block_1 * src_matrix_block_2
*  note:
*      all matrix block pointers should not be null
*
**/
UINT32 matrix_r_block_mul(const UINT32 matrixr_md_id,
                                 const MATRIX_BLOCK *src_matrix_block_1,
                                 const MATRIX_BLOCK *src_matrix_block_2,
                                 MATRIX_BLOCK *des_matrix_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 sub_row_idx;
    UINT32 sub_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    UINT32 sub_idx;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *des_data_addr;

    REAL *sum_data_addr;
    REAL *tmp_data_addr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_block_mul: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    if( src_matrix_block_1 == des_matrix_block )
    {
        /*des_matrix_block = des_matrix_block * src_matrix_block_2*/
        return matrix_r_block_mul_self_rear(matrixr_md_id, src_matrix_block_2, des_matrix_block);
    }

    if( src_matrix_block_2 == des_matrix_block )
    {
        /*des_matrix_block = src_matrix_block_1 * des_matrix_block*/
        return matrix_r_block_mul_self_front(matrixr_md_id, src_matrix_block_1, des_matrix_block);
    }

    //sys_log(LOGSTDOUT, "matrix_r_block_mul: src_rotated_flag_1 %lx, src_rotated_flag_2 %lx, des_rotated_flag %lx\n", src_rotated_flag_1, src_rotated_flag_2, des_rotated_flag);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_1);
    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_2);

    if( MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_1)
     != MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_block_mul: not matchable blocks where src_matrix_block_1 row_num = %ld, col_num = %ld, but src_matrix_block_2 row_num = %ld, col_num = %ld\n",
                MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_1),
                MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_1),
                MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block_2),
                MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_2));
        return ((UINT32)(-1));
    }

    MATRIX_BLOCK_SET_ROW_NUM(des_matrix_block, sub_row_num);
    MATRIX_BLOCK_SET_COL_NUM(des_matrix_block, sub_col_num);

    sum_data_addr = NULL_PTR;

    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &tmp_data_addr, LOC_MATRIXR_0031);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(sub_row_idx, sub_row_num, sub_col_idx, sub_col_num)
    {
        des_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx);

        if( NULL_PTR == sum_data_addr )
        {
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &sum_data_addr, LOC_MATRIXR_0032);
        }
        REAL_SETZERO(matrixr_md->real_md_id,(*sum_data_addr));

        MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, MATRIX_BLOCK_GET_COL_NUM(src_matrix_block_1))
        {
            src_data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_1, sub_row_idx, sub_idx);
            src_data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block_2, sub_idx, sub_col_idx);

            if( NULL_PTR == src_data_addr_1 && NULL_PTR == src_data_addr_2 )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_mul: [1]should not enter here after patch\n");
                continue;
            }

            if( NULL_PTR == src_data_addr_1 )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_mul: [2]should not enter here after patch\n");
                REAL_ADC(matrixr_md->real_md_id,(*src_data_addr_2), (*sum_data_addr));
                continue;
            }

            if( NULL_PTR == src_data_addr_2 )
            {
                MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_mul: [3]should not enter here after patch\n");
                REAL_ADC(matrixr_md->real_md_id,(*src_data_addr_1), (*sum_data_addr));
                continue;
            }

            //sys_print(LOGSTDOUT," + %lf * %lf ", (*src_data_addr_1), (*src_data_addr_2));

            REAL_MUL(matrixr_md->real_md_id,(*src_data_addr_1), (*src_data_addr_2), (*tmp_data_addr));
            REAL_ADC(matrixr_md->real_md_id,(*tmp_data_addr), (*sum_data_addr));
        }

        //sys_print(LOGSTDOUT, " = %lf, ", (*sum_data_addr));

        if( NULL_PTR == des_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_mul: [4]should not enter here after patch\n");
            MATRIX_BLOCK_SET_DATA_ADDR(des_matrix_block, sub_row_idx, sub_col_idx, sum_data_addr);
            sum_data_addr = NULL_PTR;
        }
        else
        {
            REAL_CLONE(matrixr_md->real_md_id,(*sum_data_addr), (*des_data_addr));
            //sys_print(LOGSTDOUT, " <====> %lf\n", (*des_data_addr));
        }
        //sys_print(LOGSTDOUT, " = %.0f\n", (*MATRIXR_BLOCK_GET_DATA_ADDR(des_rotated_flag, des_matrix_block, sub_row_idx, sub_col_idx)));
    }

    if( NULL_PTR != sum_data_addr )
    {
        free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, sum_data_addr, LOC_MATRIXR_0033);
    }
    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, tmp_data_addr, LOC_MATRIXR_0034);

    return (0);
}

/**
*
* get row num of matrix block
*
**/
UINT32 matrix_r_block_get_row_num(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *row_num)
{
    (*row_num) = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    return (0);
}

/**
*
* get col num of matrix block
*
**/
UINT32 matrix_r_block_get_col_num(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *col_num)
{
    (*col_num) = MATRIX_BLOCK_GET_COL_NUM(matrix_block);
    return (0);
}

/**
*
* get type of matrix block
*
**/
UINT32 matrix_r_block_get_type(const UINT32 matrixr_md_id, const MATRIX_BLOCK *matrix_block, UINT32 *row_num, UINT32 *col_num)
{
    (*row_num) = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    (*col_num) = MATRIX_BLOCK_GET_COL_NUM(matrix_block);
    return (0);
}

/**
*
* set type of matrix block
*
**/
UINT32 matrix_r_block_set_type(const UINT32 matrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX_BLOCK *matrix_block)
{
    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, row_num);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, col_num);
    return (0);
}

/**
*
* get row num of matrix
*
**/
UINT32 matrix_r_get_row_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_row_num: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == row_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_row_num: row_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_get_row_num: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    (*row_num) = MATRIX_GET_ROW_NUM(matrix);
    return (0);
}

/**
*
* get col num of matrix
*
**/
UINT32 matrix_r_get_col_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *col_num)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_col_num: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == col_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_col_num: col_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_get_col_num: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    (*col_num) = MATRIX_GET_COL_NUM(matrix);
    return (0);
}

/**
*
* get type of matrix
* if matrix type is (m x n), then return row_num = m, and col_num = n
*
**/
UINT32 matrix_r_get_type(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_num, UINT32 *col_num)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_type: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == row_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_type: row_num is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == col_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_type: col_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_get_type: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    (*row_num) = MATRIX_GET_ROW_NUM(matrix);
    (*col_num) = MATRIX_GET_COL_NUM(matrix);
    return (0);
}

/**
*
* get row blocks num of matrix
*
**/
UINT32 matrix_r_get_row_blocks_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *row_blocks_num)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_row_blocks_num: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == row_blocks_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_row_blocks_num: row_blocks_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_get_row_blocks_num: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    (*row_blocks_num) = MATRIX_GET_ROW_BLOCKS_NUM(matrix);
    return (0);
}

/**
*
* get col blocks num of matrix
*
**/
UINT32 matrix_r_get_col_blocks_num(const UINT32 matrixr_md_id, const MATRIX *matrix, UINT32 *col_blocks_num)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_col_blocks_num: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == col_blocks_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_get_col_blocks_num: col_blocks_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_get_col_blocks_num: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    (*col_blocks_num) = MATRIX_GET_COL_BLOCKS_NUM(matrix);
    return (0);
}

/**
*
* clone src matrix to des matrix
*     des_matrix = src_matrix
* note:
*    here des_matrix must be empty matrix, otherwise, its skeleton will be lost without notification
*
**/
UINT32 matrix_r_clone(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    UINT32 row_num;
    UINT32 col_num;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block;
    MATRIX_BLOCK  *des_matrix_block;

    if( NULL_PTR == src_matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_clone: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_clone: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_clone: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(src_matrix == des_matrix)
    {
        return (0);
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix);
    col_num = MATRIX_GET_COL_NUM(src_matrix);

    if( row_num != MATRIX_GET_ROW_NUM(des_matrix)
     || col_num != MATRIX_GET_COL_NUM(des_matrix) )
    {
        /*clean and re-new skeleton of old des_matrix*/
        matrix_r_clean_matrix(matrixr_md_id, des_matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, des_matrix);
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
        des_matrix_block = MATRIX_GET_BLOCK(des_matrix, block_row_idx, block_col_idx);

        matrix_r_clone_block(matrixr_md_id, src_matrix_block, des_matrix_block);
    }

    return (0);
}

/**
*
* rotate a matrix
*     matrix = T(matrix)
*
**/
UINT32 matrix_r_rotate(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_rotate: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_rotate: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    MATRIX_XCHG_ROTATED_FLAG(matrix);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        MATRIX_BLOCK_XCHG_ROTATED_FLAG(matrix_block);
    }

    return (0);
}

/**
*
* matrix = 0
*
**/
UINT32 matrix_r_set_zero(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_set_zero: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_set_zero: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_block_set_zero(matrixr_md_id, matrix_block);
    }
    return (0);
}

/**
*
* matrix = 1
* note:
*    1. row num of matrix must be same as col num
*       2. must row num > 0
*
**/
UINT32 matrix_r_set_one(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 row_num;
    UINT32 col_num;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_set_one: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_set_one: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    row_num = MATRIX_GET_ROW_NUM(matrix);
    col_num = MATRIX_GET_COL_NUM(matrix);

    if( row_num != col_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_set_one: not valid n x n matrix: row_num = %ld, col_num = %ld\n", row_num, col_num);
        return ((UINT32)(-1));
    }

    if( 0 == row_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_set_one: not valid matrix: row_num = %ld, col_num = %ld\n", row_num, col_num);
        return ((UINT32)(-1));
    }

    /*set matrix to zero at first*/
    matrix_r_set_zero(matrixr_md_id, matrix);

    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx)
    {
        block_col_idx = block_row_idx;
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_block_set_one(matrixr_md_id, matrix_block);
    }
    return (0);
}


/**
*
* exchange two rows of the matrix
* if row no is overflow, then report error and do nothing
*
**/
#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
static UINT32 matrix_r_block_xchg_rows(const UINT32 matrixr_md_id, const UINT32 sub_row_no_1, const UINT32 sub_row_no_2, MATRIX_BLOCK *matrix_block_1, MATRIX_BLOCK *matrix_block_2)
{
    UINT32 sub_col_num;
    UINT32 sub_col_idx;

    REAL *data_addr_1;
    REAL *data_addr_2;

    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block_1);
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_col_idx, sub_col_num)
    {
        data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_1, sub_row_no_1, sub_col_idx);
        data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_2, sub_row_no_2, sub_col_idx);

        /*exchange addr info*/
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block_1, sub_row_no_1, sub_col_idx, data_addr_2);
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block_2, sub_row_no_2, sub_col_idx, data_addr_1);
    }

    return (0);
}
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
static UINT32 matrix_r_block_xchg_rows(const UINT32 matrixr_md_id, const UINT32 sub_row_no_1, const UINT32 sub_row_no_2, MATRIX_BLOCK *matrix_block_1, MATRIX_BLOCK *matrix_block_2)
{
    UINT32 sub_col_num;
    UINT32 sub_col_idx;

    REAL *data_addr_1;
    REAL *data_addr_2;

    REAL *data_addr_t;
    MATRIXR_MD  *matrixr_md;

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &data_addr_t, LOC_MATRIXR_0035);

    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block_1);
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_col_idx, sub_col_num)
    {
        data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_1, sub_row_no_1, sub_col_idx);
        data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_2, sub_row_no_2, sub_col_idx);

        /*exchange data info*/
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_1), (*data_addr_t));
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_2), (*data_addr_1));
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_t), (*data_addr_2));
    }

    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, data_addr_t, LOC_MATRIXR_0036);

    return (0);
}
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

/**
*
* exchange two rows of the matrix
* if row no is overflow, then report error and do nothing
*
**/
#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
static UINT32 matrix_r_block_xchg_cols(const UINT32 matrixr_md_id, const UINT32 sub_col_no_1, const UINT32 sub_col_no_2, MATRIX_BLOCK *matrix_block_1, MATRIX_BLOCK *matrix_block_2)
{
    UINT32 sub_row_num;
    UINT32 sub_row_idx;

    REAL *data_addr_1;
    REAL *data_addr_2;

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block_1);
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_row_idx, sub_row_num)
    {
        data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_1, sub_row_idx, sub_col_no_1);
        data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_2, sub_row_idx, sub_col_no_2);

        /*exchange addr info*/
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block_1, sub_row_idx, sub_col_no_1, data_addr_2);
        MATRIX_BLOCK_SET_DATA_ADDR(matrix_block_2, sub_row_idx, sub_col_no_2, data_addr_1);
    }

    return (0);
}
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF || MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
static UINT32 matrix_r_block_xchg_cols(const UINT32 matrixr_md_id, const UINT32 sub_col_no_1, const UINT32 sub_col_no_2, MATRIX_BLOCK *matrix_block_1, MATRIX_BLOCK *matrix_block_2)
{
    UINT32 sub_row_num;
    UINT32 sub_row_idx;

    REAL *data_addr_1;
    REAL *data_addr_2;

    REAL *data_addr_t;
    MATRIXR_MD  *matrixr_md;

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &data_addr_t, LOC_MATRIXR_0037);

    sub_row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block_1);
    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_row_idx, sub_row_num)
    {
        data_addr_1 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_1, sub_row_idx, sub_col_no_1);
        data_addr_2 = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block_2, sub_row_idx, sub_col_no_2);

        /*exchange data info*/
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_1), (*data_addr_t));
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_2), (*data_addr_1));
        REAL_CLONE(matrixr_md->real_md_id,(*data_addr_t), (*data_addr_2));
    }

    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, data_addr_t, LOC_MATRIXR_0038);
    return (0);
}
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/

/**
*
* exchange two rows of the matrix
* if row no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_rows(const UINT32 matrixr_md_id, const UINT32 row_no_1, const UINT32 row_no_2, MATRIX *matrix)
{
    UINT32 block_row_idx_1;
    UINT32 block_row_idx_2;
    UINT32 block_col_idx;

    UINT32 sub_row_no_1;
    UINT32 sub_row_no_2;

    MATRIX_BLOCK  *matrix_block_1;
    MATRIX_BLOCK  *matrix_block_2;

    if( NULL_PTR == matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_xchg_rows: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_xchg_rows: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    block_row_idx_1 = (row_no_1 / MATRIX_VECTOR_WIDTH);
    block_row_idx_2 = (row_no_2 / MATRIX_VECTOR_WIDTH);

    sub_row_no_1 = (row_no_1 % MATRIX_VECTOR_WIDTH);
    sub_row_no_2 = (row_no_2 % MATRIX_VECTOR_WIDTH);

    MATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx)
    {
        matrix_block_1 = MATRIX_GET_BLOCK(matrix, block_row_idx_1, block_col_idx);
        matrix_block_2 = MATRIX_GET_BLOCK(matrix, block_row_idx_2, block_col_idx);

        matrix_r_block_xchg_rows(matrixr_md_id, sub_row_no_1, sub_row_no_2, matrix_block_1, matrix_block_2);
    }

    return (0);
}

/**
*
* exchange two cols of the matrix
* if col no is overflow, then report error and do nothing
*
**/
UINT32 matrix_r_xchg_cols(const UINT32 matrixr_md_id, const UINT32 col_no_1, const UINT32 col_no_2, MATRIX *matrix)
{
    UINT32 block_col_idx_1;
    UINT32 block_col_idx_2;
    UINT32 block_row_idx;

    UINT32 sub_col_no_1;
    UINT32 sub_col_no_2;

    MATRIX_BLOCK  *matrix_block_1;
    MATRIX_BLOCK  *matrix_block_2;

    if( NULL_PTR == matrix )
    {
        /*do nothing*/
        sys_log(LOGSTDERR,"error:matrix_r_xchg_cols: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_xchg_cols: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    block_col_idx_1 = (col_no_1 / MATRIX_VECTOR_WIDTH);
    block_col_idx_2 = (col_no_2 / MATRIX_VECTOR_WIDTH);

    sub_col_no_1 = (col_no_1 % MATRIX_VECTOR_WIDTH);
    sub_col_no_2 = (col_no_2 % MATRIX_VECTOR_WIDTH);

    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx)
    {
        matrix_block_1 = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx_1);
        matrix_block_2 = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx_2);

        matrix_r_block_xchg_cols(matrixr_md_id, sub_col_no_1, sub_col_no_2, matrix_block_1, matrix_block_2);
    }

    return (0);
}

/**
*
* matrix is_zero operation
*    if matrix is zero matrix, i.e, all data_area are zero or null pointers, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_zero(const UINT32 matrixr_md_id, const MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_is_zero: matrix is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_is_zero: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        if( EC_FALSE == matrix_r_block_is_zero(matrixr_md_id, matrix_block) )
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

/**
*
* matrix is_one operation
*    if matrix is unit matrix, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_is_one(const UINT32 matrixr_md_id, const MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 row_num;
    UINT32 col_num;

    MATRIX_BLOCK  *matrix_block;

    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_is_one: matrix is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_is_one: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    row_num = MATRIX_GET_ROW_NUM(matrix);
    col_num = MATRIX_GET_COL_NUM(matrix);

    if( row_num != col_num )
    {
        sys_log(LOGSTDERR,"error:matrix_r_is_one: not valid n x n matrix: row_num = %ld, col_num = %ld\n", row_num, col_num);
        return ((UINT32)(-1));
    }

    if( 0 == row_num )
    {
        return (EC_FALSE);
    }

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        if(block_row_idx == block_col_idx)
        {
            if(EC_FALSE == matrix_r_block_is_one(matrixr_md_id, matrix_block))
            {
                return (EC_FALSE);
            }
        }
        else
        {
            if(EC_FALSE == matrix_r_block_is_zero(matrixr_md_id, matrix_block))
            {
                return (EC_FALSE);
            }
        }
    }
    return (EC_TRUE);
}

/**
*
* matrix cmp operation
*    if matrix_1 == matrix_2, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL matrix_r_cmp(const UINT32 matrixr_md_id, const MATRIX *matrix_1, const MATRIX *matrix_2)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block_1;
    MATRIX_BLOCK  *matrix_block_2;

    if( NULL_PTR == matrix_1 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_cmp: matrix_1 is null pointer\n");
        return (EC_TRUE);
    }

    if( NULL_PTR == matrix_2 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_cmp: matrix_2 is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_is_cmp: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( matrix_1 == matrix_2 )
    {
        return (EC_TRUE);
    }

    if( NULL_PTR == matrix_1 || NULL_PTR == matrix_2 )
    {
        return (EC_FALSE);
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(matrix_1, matrix_2, block_row_idx, block_col_idx)
    {
        matrix_block_1 = MATRIX_GET_BLOCK(matrix_1, block_row_idx, block_col_idx);
        matrix_block_2 = MATRIX_GET_BLOCK(matrix_2, block_row_idx, block_col_idx);

        if( EC_FALSE == matrix_r_block_cmp(matrixr_md_id, matrix_block_1, matrix_block_2) )
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

/**
*
* matrix neg self operation
*     matrix = - matrix
*
**/
static UINT32 matrix_r_neg_self(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_block_neg_self(matrixr_md_id, matrix_block);
    }

    return (0);
}

/**
*
* matrix neg operation
*     des_matrix = - src_matrix
*
**/
UINT32 matrix_r_neg(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block;
    MATRIX_BLOCK  *des_matrix_block;

    UINT32 row_num;
    UINT32 col_num;

    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_neg: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_neg: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_neg: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( src_matrix == des_matrix )
    {
        return matrix_r_neg_self(matrixr_md_id, des_matrix);
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix);
    col_num = MATRIX_GET_COL_NUM(src_matrix);

    if(row_num != MATRIX_GET_ROW_NUM(des_matrix)
    || col_num != MATRIX_GET_COL_NUM(des_matrix))
    {
        matrix_r_clean_matrix(matrixr_md_id, des_matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, des_matrix);
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
        des_matrix_block = MATRIX_GET_BLOCK(des_matrix, block_row_idx, block_col_idx);

        matrix_r_block_neg(matrixr_md_id, src_matrix_block, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix smul self operation
*     matrix = s_data * matrix
*
**/
static UINT32 matrix_r_s_mul_self(const UINT32 matrixr_md_id, const REAL *s_data_addr, MATRIX *matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    if(EC_TRUE == REAL_ISONE(matrixr_md->real_md_id, (*s_data_addr)))
    {
        return 0;
    }

    if(EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id, (*s_data_addr)))
    {
        return matrix_r_set_zero(matrixr_md_id, matrix);
    }

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
        matrix_r_block_s_mul_self(matrixr_md_id, s_data_addr, matrix_block);
    }

    return (0);
}

/**
*
* matrix smul operation
*     des_matrix = s_data * src_matrix
*
**/
UINT32 matrix_r_s_mul(const UINT32 matrixr_md_id, const REAL *s_data_addr, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block;
    MATRIX_BLOCK  *des_matrix_block;

    UINT32 row_num;
    UINT32 col_num;

    if( NULL_PTR == s_data_addr )
    {
        sys_log(LOGSTDERR,"error:matrix_r_s_mul: s_data_addr is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_s_mul: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_s_mul: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_s_mul: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( src_matrix == des_matrix )
    {
        return matrix_r_s_mul_self(matrixr_md_id, s_data_addr, des_matrix);
    }

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    if(EC_TRUE == REAL_ISONE(matrixr_md->real_md_id, (*s_data_addr)))
    {
        return matrix_r_clone(matrixr_md_id, src_matrix, des_matrix);
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix);
    col_num = MATRIX_GET_COL_NUM(src_matrix);

    if( row_num != MATRIX_GET_ROW_NUM(des_matrix)
     || col_num != MATRIX_GET_COL_NUM(des_matrix) )
    {
        /*clean and re-new skeleton of old des_matrix*/
        matrix_r_clean_matrix(matrixr_md_id, des_matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, des_matrix);
    }

    if(EC_TRUE == REAL_ISZERO(matrixr_md->real_md_id, (*s_data_addr)))
    {
        return matrix_r_set_zero(matrixr_md_id, des_matrix);
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
        des_matrix_block = MATRIX_GET_BLOCK(des_matrix, block_row_idx, block_col_idx);

        matrix_r_block_s_mul(matrixr_md_id, s_data_addr, src_matrix_block, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix adc operation
*     des_matrix += src_matrix
*
**/
UINT32 matrix_r_adc(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block;
    MATRIX_BLOCK  *des_matrix_block;

    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_adc: src_matrix is null pointer\n");
        return (0);
    }

    if( NULL_PTR == des_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_adc: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_adc: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(  MATRIX_GET_ROW_NUM(src_matrix)
      != MATRIX_GET_ROW_NUM(des_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_adc: not matchable matrix: row num of src_matrix = %ld, row num of des_matrix = %ld\n",
                        MATRIX_GET_ROW_NUM(src_matrix),
                        MATRIX_GET_ROW_NUM(des_matrix));
        return ((UINT32)(-1));
    }

    if(  MATRIX_GET_COL_NUM(src_matrix)
      != MATRIX_GET_COL_NUM(des_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_adc: not matchable matrix: col num of src_matrix = %ld, col num of des_matrix = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix),
                        MATRIX_GET_COL_NUM(des_matrix));
        return ((UINT32)(-1));
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
        des_matrix_block = MATRIX_GET_BLOCK(des_matrix, block_row_idx, block_col_idx);

        matrix_r_block_adc(matrixr_md_id, src_matrix_block, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix add operation
*     des_matrix = src_matrix_1 + src_matrix_2
*
**/
UINT32 matrix_r_add(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block_1;
    MATRIX_BLOCK  *src_matrix_block_2;
    MATRIX_BLOCK  *des_matrix_block;

    UINT32 row_num;
    UINT32 col_num;

    if( NULL_PTR == src_matrix_1 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_add: src_matrix_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_2 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_add: src_matrix_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_add: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_add: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( src_matrix_1 == des_matrix )
    {
        return matrix_r_adc(matrixr_md_id, src_matrix_2, des_matrix);
    }

    if( src_matrix_2 == des_matrix )
    {
        return matrix_r_adc(matrixr_md_id, src_matrix_1, des_matrix);
    }

    if(  MATRIX_GET_ROW_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_add: not matchable matrix: row num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_ROW_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_COL_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_add: not matchable matrix: col num of src_matrix_1 = %ld, col num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_COL_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_1);

    if( row_num != MATRIX_GET_ROW_NUM(des_matrix)
     || col_num != MATRIX_GET_COL_NUM(des_matrix) )
    {
        /*clean and re-new skeleton of old des_matrix*/
        matrix_r_clean_matrix(matrixr_md_id, des_matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, des_matrix);
    }

    MATRIX_ROW_COL_BLOCKS_TRIPLE_LOOP_NEXT(src_matrix_1, src_matrix_2, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx, block_col_idx);
        src_matrix_block_2 = MATRIX_GET_BLOCK(src_matrix_2, block_row_idx, block_col_idx);
        des_matrix_block   = MATRIX_GET_BLOCK(des_matrix  , block_row_idx, block_col_idx);

        matrix_r_block_add(matrixr_md_id, src_matrix_block_1, src_matrix_block_2, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix sbb operation
*     des_matrix -= src_matrix
*
**/
UINT32 matrix_r_sbb(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block;
    MATRIX_BLOCK  *des_matrix_block;

    if( NULL_PTR == src_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_sbb: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_sbb: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_sbb: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( des_matrix == src_matrix )
    {
        return matrix_r_set_zero(matrixr_md_id, des_matrix);
    }

    if(  MATRIX_GET_ROW_NUM(src_matrix)
      != MATRIX_GET_ROW_NUM(des_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_sbb: not matchable matrix: row num of src_matrix = %ld, row num of des_matrix = %ld\n",
                        MATRIX_GET_ROW_NUM(src_matrix),
                        MATRIX_GET_ROW_NUM(des_matrix));
        return ((UINT32)(-1));
    }

    if(  MATRIX_GET_COL_NUM(src_matrix)
      != MATRIX_GET_COL_NUM(des_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_sbb: not matchable matrix: col num of src_matrix = %ld, col num of des_matrix = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix),
                        MATRIX_GET_COL_NUM(des_matrix));
        return ((UINT32)(-1));
    }

    MATRIX_ROW_COL_BLOCKS_DOUBLE_LOOP_NEXT(src_matrix, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
        des_matrix_block = MATRIX_GET_BLOCK(des_matrix, block_row_idx, block_col_idx);

        matrix_r_block_sbb(matrixr_md_id, src_matrix_block, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix sub operation
*     des_matrix = src_matrix_1 - src_matrix_2
*
**/
UINT32 matrix_r_sub(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *src_matrix_block_1;
    MATRIX_BLOCK  *src_matrix_block_2;
    MATRIX_BLOCK  *des_matrix_block;

    UINT32 row_num;
    UINT32 col_num;

    if( NULL_PTR == src_matrix_1 )
    {
        //sys_log(LOGSTDERR,"error:matrix_r_sub: src_matrix_1 is null pointer\n");
        //return matrix_r_neg(matrixr_md_id, src_matrix_2, des_matrix);
        sys_log(LOGSTDERR,"error:matrix_r_sub: src_matrix_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_2 )
    {
        //sys_log(LOGSTDERR,"error:matrix_r_sub: src_matrix_2 is null pointer\n");
        //return matrix_r_clone(matrixr_md_id, src_matrix_1, des_matrix);
        sys_log(LOGSTDERR,"error:matrix_r_sub: src_matrix_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_sub: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_sub: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( src_matrix_1 == des_matrix )
    {
        return matrix_r_sbb(matrixr_md_id, src_matrix_2, des_matrix);
    }

    if( src_matrix_2 == des_matrix )
    {
        return matrix_r_sbb(matrixr_md_id, src_matrix_1, des_matrix);
    }

    if(  MATRIX_GET_ROW_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_sub: not matchable matrix: row num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_ROW_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_1));
        return ((UINT32)(-1));
    }

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_COL_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_sub: not matchable matrix: col num of src_matrix_1 = %ld, col num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_COL_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_1);

    if( row_num != MATRIX_GET_ROW_NUM(des_matrix)
     || col_num != MATRIX_GET_COL_NUM(des_matrix) )
    {
        /*clean and re-new skeleton of old des_matrix*/
        matrix_r_clean_matrix(matrixr_md_id, des_matrix);
        matrix_r_new_matrix_skeleton(matrixr_md_id, row_num, col_num, des_matrix);
    }

    MATRIX_ROW_COL_BLOCKS_TRIPLE_LOOP_NEXT(src_matrix_1, src_matrix_2, des_matrix, block_row_idx, block_col_idx)
    {
        src_matrix_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx, block_col_idx);
        src_matrix_block_2 = MATRIX_GET_BLOCK(src_matrix_2, block_row_idx, block_col_idx);
        des_matrix_block   = MATRIX_GET_BLOCK(des_matrix  , block_row_idx, block_col_idx);

        matrix_r_block_sub(matrixr_md_id, src_matrix_block_1, src_matrix_block_2, des_matrix_block);
    }

    return (0);
}

/**
*
* matrix squ operation
*     matrix = matrix ^ 2
*
**/
UINT32 matrix_r_squ_self(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    if( NULL_PTR == matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_squ_self: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_squl_self: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( MATRIX_GET_ROW_NUM(matrix)
     != MATRIX_GET_COL_NUM(matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_squ_self: not valid n x n matrix: row num = %ld, col num = %ld\n",
                        MATRIX_GET_ROW_NUM(matrix),
                        MATRIX_GET_COL_NUM(matrix));
        return ((UINT32)(-1));
    }

    return matrix_r_mul(matrixr_md_id, matrix, matrix, matrix);
}

/**
*
* matrix squ operation
*     des_matrix = src_matrix ^ 2
*
**/
UINT32 matrix_r_squ(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_squ: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_squ: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_squ: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(  MATRIX_GET_COL_NUM(src_matrix)
      != MATRIX_GET_ROW_NUM(src_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_squ: not valid n x n matrix: row num = %ld, col num = %ld\n",
                        MATRIX_GET_ROW_NUM(src_matrix),
                        MATRIX_GET_COL_NUM(src_matrix));
        return ((UINT32)(-1));
    }

    return  matrix_r_mul(matrixr_md_id, src_matrix, src_matrix, des_matrix);
}

/**
*
* matrix mul operation
*     des_matrix = des_matrix * src_matrix
*
**/
UINT32 matrix_r_mul_self_rear(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    if( NULL_PTR == src_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_rear: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_rear: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_mul_self_rear: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(  MATRIX_GET_COL_NUM(des_matrix)
      != MATRIX_GET_ROW_NUM(src_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_rear: not matchable matrix: col num of des_matrix = %ld, row num of src_matrix = %ld\n",
                        MATRIX_GET_COL_NUM(des_matrix),
                        MATRIX_GET_ROW_NUM(src_matrix));
        return ((UINT32)(-1));
    }

    return matrix_r_mul(matrixr_md_id, des_matrix, src_matrix, des_matrix);
}

/**
*
* matrix mul operation
*     des_matrix = src_matrix * des_matrix
*
**/
UINT32 matrix_r_mul_self_front(const UINT32 matrixr_md_id, const MATRIX *src_matrix, MATRIX *des_matrix)
{
    if( NULL_PTR == src_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_front: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_front: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_mul_self_front: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(  MATRIX_GET_COL_NUM(src_matrix)
      != MATRIX_GET_ROW_NUM(des_matrix) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_self_rear: not matchable matrix: col num of src_matrix = %ld, row num of des_matrix = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix),
                        MATRIX_GET_ROW_NUM(des_matrix));
        return ((UINT32)(-1));
    }

    return matrix_r_mul(matrixr_md_id, src_matrix, des_matrix, des_matrix);
}

/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
*
**/
UINT32 matrix_r_mul(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 block_row_idx_cur;
    UINT32 block_col_idx_cur;

    UINT32 block_row_num_cur;
    UINT32 block_col_num_cur;

    MATRIX_BLOCK  *src_matrix_block_1;
    MATRIX_BLOCK  *src_matrix_block_2;
    MATRIX_BLOCK  *tmp_matrix_block;

    MATRIX_BLOCK  *pro_matrix_block;

    MATRIX *tmp_matrix;

    UINT32 row_num;
    UINT32 col_num;

    if( NULL_PTR == src_matrix_1 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul: src_matrix_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_2 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul: src_matrix_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_mul: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul: not matchable matrix: col num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_2);

    matrix_r_new_matrix(matrixr_md_id, row_num, col_num, &tmp_matrix);
    matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &pro_matrix_block);

    block_col_num_cur = MATRIX_GET_COL_BLOCKS_NUM(src_matrix_1);
    block_row_num_cur = MATRIX_GET_ROW_BLOCKS_NUM(src_matrix_2);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(tmp_matrix, block_row_idx, block_col_idx)
    {
        tmp_matrix_block = MATRIX_GET_BLOCK(tmp_matrix, block_row_idx, block_col_idx);
        for(block_col_idx_cur = 0, block_row_idx_cur = 0;
            block_col_idx_cur < block_col_num_cur && block_row_idx_cur < block_row_num_cur;
            block_col_idx_cur ++, block_row_idx_cur ++)
        {
            src_matrix_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx   , block_col_idx_cur);
            src_matrix_block_2 = MATRIX_GET_BLOCK(src_matrix_2, block_row_idx_cur, block_col_idx   );

            matrix_r_block_mul(matrixr_md_id, src_matrix_block_1, src_matrix_block_2, pro_matrix_block);
            matrix_r_block_adc(matrixr_md_id, pro_matrix_block, tmp_matrix_block);
        }
    }

    matrix_r_clean_matrix(matrixr_md_id, des_matrix);
    matrix_r_move_matrix(matrixr_md_id, tmp_matrix, des_matrix);

    matrix_r_free_matrix(MD_MATRIXR, matrixr_md_id, MM_MATRIX, tmp_matrix);

    matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, pro_matrix_block);

    return (0);
}

/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
*
**/
UINT32 matrix_r_mul_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix_1, const MATRIX *src_matrix_2, MATRIX *des_matrix)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 block_row_idx_cur;
    UINT32 block_col_idx_cur;

    UINT32 block_row_num_cur;
    UINT32 block_col_num_cur;

    MATRIX_BLOCK  *src_matrix_block_1;
    MATRIX_BLOCK  *src_matrix_block_2;
    MATRIX_BLOCK  *tmp_matrix_block;

    MATRIX_BLOCK  *pro_matrix_block;

    MATRIX *tmp_matrix;

    UINT32 row_num;
    UINT32 col_num;

    CLIST *clist;

    if( NULL_PTR == src_matrix_1 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_p: src_matrix_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_2 )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_p: src_matrix_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix)
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_p: des_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_mul: matrix_r_mul_p module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        sys_log(LOGSTDERR,"error:matrix_r_mul_p: not matchable matrix: col num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_2);

    matrix_r_new_matrix(matrixr_md_id, row_num, col_num, &tmp_matrix);

    block_col_num_cur = MATRIX_GET_COL_BLOCKS_NUM(src_matrix_1);
    block_row_num_cur = MATRIX_GET_ROW_BLOCKS_NUM(src_matrix_2);

    clist = clist_new(MM_IGNORE, LOC_MATRIXR_0039);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(tmp_matrix, block_row_idx, block_col_idx)
    {
        TASK_MGR *task_mgr;
        UINT32 ret;

        task_mgr = task_new(matrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

        tmp_matrix_block = MATRIX_GET_BLOCK(tmp_matrix, block_row_idx, block_col_idx);

        for(block_col_idx_cur = 0, block_row_idx_cur = 0;
            block_col_idx_cur < block_col_num_cur && block_row_idx_cur < block_row_num_cur;
            block_col_idx_cur ++, block_row_idx_cur ++)
        {
            src_matrix_block_1 = MATRIX_GET_BLOCK(src_matrix_1, block_row_idx    , block_col_idx_cur);
            src_matrix_block_2 = MATRIX_GET_BLOCK(src_matrix_2, block_row_idx_cur, block_col_idx    );

            matrix_r_alloc_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, &pro_matrix_block);
            clist_push_back(clist, pro_matrix_block);

            task_inc(task_mgr, &ret, FI_matrix_r_block_mul, ERR_MODULE_ID, src_matrix_block_1, src_matrix_block_2, pro_matrix_block);
        }

        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

        while(EC_FALSE == clist_is_empty(clist))
        {
            pro_matrix_block = (MATRIX_BLOCK  *)clist_pop_back(clist);
            matrix_r_block_adc(matrixr_md_id, pro_matrix_block, tmp_matrix_block);
            matrix_r_free_block(MD_MATRIXR, matrixr_md_id, MM_MATRIX_BLOCK, pro_matrix_block);
        }
    }
    clist_free(clist, LOC_MATRIXR_0040);

    matrix_r_clean_matrix(matrixr_md_id, des_matrix);
    matrix_r_move_matrix(matrixr_md_id, tmp_matrix, des_matrix);

    matrix_r_free_matrix(MD_MATRIXR, matrixr_md_id, MM_MATRIX, tmp_matrix);

    return (0);
}

UINT32 matrix_r_block_mul_vector_block(const UINT32 matrixr_md_id,
                                                const MATRIX_BLOCK *src_matrix_block,
                                                const VECTOR_BLOCK *src_vector_block,
                                                VECTOR_BLOCK *des_vector_block)
{
    MATRIXR_MD  *matrixr_md;

    UINT32 vectorr_md_id;
    UINT32 real_md_id;

    UINT32 row_num;
    UINT32 col_num;

    UINT32 row_idx;
    UINT32 col_idx;

    REAL *src_matrix_data_addr;
    REAL *src_vector_data_addr;
    REAL *des_vector_data_addr;

    REAL *tmp_data_addr;

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    vectorr_md_id = matrixr_md->vectorr_md_id;
    real_md_id = matrixr_md->real_md_id;

    //sys_log(LOGSTDOUT, "matrix_r_block_mul_vector_block: src_matrix_block rotated_flag = %d\n", MATRIX_BLOCK_GET_ROTATED_FLAG(src_matrix_block));

    row_num = MATRIX_BLOCK_GET_ROW_NUM(src_matrix_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(src_matrix_block);

    if( col_num != VECTOR_BLOCK_GET_NUM(src_vector_block))
    {
        sys_log(LOGSTDERR,
           "error:matrix_r_block_mul_vector_block: not matchable matrix block and vector block: col num of src_matrix_block = %ld, row num of src_vector_block = %ld\n",
           MATRIX_BLOCK_GET_COL_NUM(src_matrix_block),
           VECTOR_BLOCK_GET_NUM(src_vector_block));
        return ((UINT32)(-1));
    }

    VECTOR_BLOCK_SET_NUM(des_vector_block, row_num);

    alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &tmp_data_addr, LOC_MATRIXR_0041);

    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(row_idx, row_num)
    {
        des_vector_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, row_idx);
        if( NULL_PTR == des_vector_data_addr )
        {
            MATRIXR_PATCH_LOG(LOGSTDOUT, "error:matrix_r_block_mul_vector_block: should not enter here after patch\n");
            alloc_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, &des_vector_data_addr, LOC_MATRIXR_0042);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, row_idx, des_vector_data_addr);
        }
        REAL_SETZERO(real_md_id, (*des_vector_data_addr));

        MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(col_idx, col_num)
        {
            src_matrix_data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(src_matrix_block, row_idx, col_idx);
            src_vector_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, col_idx);

            if( NULL_PTR == src_matrix_data_addr
             || NULL_PTR == src_vector_data_addr )
            {
                continue;
            }

            if( EC_TRUE == REAL_ISZERO(real_md_id, (*src_matrix_data_addr))
             || EC_TRUE == REAL_ISZERO(real_md_id, (*src_vector_data_addr)) )
            {
                continue;
            }

            REAL_MUL(real_md_id, (*src_matrix_data_addr), (*src_vector_data_addr), (*tmp_data_addr));
            REAL_ADC(real_md_id, (*tmp_data_addr), (*des_vector_data_addr));
#if 0
            sys_log(LOGSTDOUT, "[%d, %d] * [%d]: (+) %.0f * %.0f => %.0f\n", row_idx, col_idx, col_idx,
                            (*src_matrix_data_addr), (*src_vector_data_addr), (*des_vector_data_addr));
#endif
        }
    }
    free_static_mem(MD_MATRIXR, matrixr_md_id, MM_REAL, tmp_data_addr, LOC_MATRIXR_0043);

    //sys_log(LOGSTDOUT, "matrix_r_block_mul_vector_block: des_vector_block num = %d\n", VECTOR_BLOCK_GET_NUM(des_vector_block));

    return (0);
}

/**
*
* matrix and vector mul operation
*    des_vector = src_matrix * src_vector
* type: (m x 1) = (m x n) * (n x 1)
*
*
**/
UINT32 matrix_r_matrix_mul_vector(const UINT32 matrixr_md_id, const MATRIX *src_matrix, const VECTOR *src_vector, VECTOR *des_vector)
{
    MATRIXR_MD  *matrixr_md;
    UINT32 vectorr_md_id;

    MATRIX_BLOCK  *src_matrix_block;

    VECTOR_BLOCK *src_vector_block;

    VECTOR *tmp_vector;
    VECTOR_BLOCK *tmp_vector_block;

    VECTOR_BLOCK *pro_vector_block;

    UINT32 src_matrix_rotated_flag;
    UINT32 src_vector_rotated_flag;

    UINT32 row_num;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_matrix_mul_vector: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    vectorr_md_id = matrixr_md->vectorr_md_id;

    src_vector_rotated_flag = VECTOR_GET_ROTATED_FLAG(src_vector);

    /*src_vector must be col vector*/
    if( 1 != src_vector_rotated_flag )
    {
        sys_log(LOGSTDERR,
           "error:matrix_r_matrix_mul_vector: invalid vector type: rotated_flag of src_vector = %ld\n", src_vector_rotated_flag);
        return ((UINT32)(-1));
    }

    src_matrix_rotated_flag = MATRIX_GET_ROTATED_FLAG(src_matrix);
    if( MATRIX_GET_COL_NUM(src_matrix) != VECTOR_GET_NUM(src_vector) )
    {
        sys_log(LOGSTDERR,
           "error:matrix_r_matrix_mul_vector: not matchable matrix and vector: col num of src_matrix = %ld, row num of src_vector = %ld\n",
           MATRIX_GET_COL_NUM(src_matrix),
           VECTOR_GET_NUM(src_vector));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix);

    /*new a col vector: new one and rotate it*/
    vector_r_new_vector(vectorr_md_id, row_num, &tmp_vector);
    vector_r_rotate(vectorr_md_id, tmp_vector);

    vector_r_alloc_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, &pro_vector_block);

    tmp_vector_block = VECTOR_FIRST_BLOCK(tmp_vector);

    MATRIX_ROW_BLOCKS_LOOP_NEXT(src_matrix, block_row_idx)
    {
        src_vector_block = VECTOR_FIRST_BLOCK(src_vector);
        MATRIX_COL_BLOCKS_LOOP_NEXT(src_matrix, block_col_idx)
        {
            src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
            if(EC_FALSE == matrix_r_block_is_zero(matrixr_md_id, src_matrix_block)
            && EC_FALSE == vector_r_block_is_zero(vectorr_md_id, src_vector_block))
            {
                matrix_r_block_mul_vector_block(matrixr_md_id, src_matrix_block, src_vector_block, pro_vector_block);
                vector_r_block_adc(vectorr_md_id, pro_vector_block, tmp_vector_block);
            }

            src_vector_block  = VECTOR_BLOCK_NEXT(src_vector_block);
        }
        tmp_vector_block  = VECTOR_BLOCK_NEXT(tmp_vector_block);
    }

    vector_r_free_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, pro_vector_block);

    vector_r_clean_vector(vectorr_md_id, des_vector);
    vector_r_move_vector(vectorr_md_id, tmp_vector, des_vector);

    vector_r_destroy_vector(vectorr_md_id, tmp_vector);

    return (0);
}

/**
*
* matrix and vector mul operation
*    des_vector = src_matrix * src_vector
* type: (m x 1) = (m x n) * (n x 1)
*
*
**/
UINT32 matrix_r_matrix_mul_vector_p(const UINT32 matrixr_md_id, const MATRIX *src_matrix, const VECTOR *src_vector, VECTOR *des_vector)
{
    MATRIXR_MD  *matrixr_md;
    UINT32 vectorr_md_id;

    MATRIX_BLOCK  *src_matrix_block;

    VECTOR_BLOCK *src_vector_block;

    VECTOR *tmp_vector;
    VECTOR_BLOCK *tmp_vector_block;

    UINT32 src_matrix_rotated_flag;
    UINT32 src_vector_rotated_flag;

    UINT32 row_num;
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    CLIST *clist;

    if( NULL_PTR == src_matrix )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector_p: src_matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector_p: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        sys_log(LOGSTDERR,"error:matrix_r_matrix_mul_vector_p: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( MATRIXR_MD_ID_CHECK_INVALID(matrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:matrix_r_matrix_mul_vector_p: matrixr module #0x%lx not started.\n",
                matrixr_md_id);
        dbg_exit(MD_MATRIXR, matrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrixr_md = MATRIXR_MD_GET(matrixr_md_id);
    vectorr_md_id = matrixr_md->vectorr_md_id;

    //sys_log(LOGSTDOUT, "matrix_r_matrix_mul_vector_p: matrixr_md_id = %d, matrixr_md->mod_mgr:\n", matrixr_md_id);
    //mod_mgr_print(LOGSTDOUT, &(matrixr_md->mod_mgr));

    src_vector_rotated_flag = VECTOR_GET_ROTATED_FLAG(src_vector);

    /*src_vector must be col vector*/
    if( 1 != src_vector_rotated_flag )
    {
        sys_log(LOGSTDERR,
           "error:matrix_r_matrix_mul_vector_p: invalid vector type: rotated_flag of src_vector = %ld\n", src_vector_rotated_flag);
        return ((UINT32)(-1));
    }

    src_matrix_rotated_flag = MATRIX_GET_ROTATED_FLAG(src_matrix);
    if( MATRIX_GET_COL_NUM(src_matrix) != VECTOR_GET_NUM(src_vector) )
    {
        sys_log(LOGSTDERR,
           "error:matrix_r_matrix_mul_vector_p: not matchable matrix and vector: col num of src_matrix = %ld, row num of src_vector = %ld\n",
           MATRIX_GET_COL_NUM(src_matrix),
           VECTOR_GET_NUM(src_vector));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix);

    /*new a col vector: new one and rotate it*/
    vector_r_new_vector(vectorr_md_id, row_num, &tmp_vector);
    vector_r_rotate(vectorr_md_id, tmp_vector);

    clist = clist_new(MM_IGNORE, LOC_MATRIXR_0044);

    tmp_vector_block = VECTOR_FIRST_BLOCK(tmp_vector);

    MATRIX_ROW_BLOCKS_LOOP_NEXT(src_matrix, block_row_idx)
    {
        TASK_MGR *task_mgr;
        UINT32 ret;

        VECTOR_BLOCK *pro_vector_block_t;

        task_mgr = task_new(matrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

        src_vector_block = VECTOR_FIRST_BLOCK(src_vector);
        MATRIX_COL_BLOCKS_LOOP_NEXT(src_matrix, block_col_idx)
        {
            src_matrix_block = MATRIX_GET_BLOCK(src_matrix, block_row_idx, block_col_idx);
            if(EC_FALSE == matrix_r_block_is_zero(matrixr_md_id, src_matrix_block)
            && EC_FALSE == vector_r_block_is_zero(vectorr_md_id, src_vector_block))
            {
                vector_r_alloc_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, &pro_vector_block_t);
                clist_push_back(clist, pro_vector_block_t);

                task_inc(task_mgr, &ret, FI_matrix_r_block_mul_vector_block, matrixr_md_id, src_matrix_block, src_vector_block, pro_vector_block_t);
            }

            src_vector_block  = VECTOR_BLOCK_NEXT(src_vector_block);
        }

        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NEED_RESCHEDULE_FLAG, NULL_PTR);

        while(EC_FALSE == clist_is_empty(clist))
        {
            pro_vector_block_t = (VECTOR_BLOCK *)clist_pop_back(clist);
            vector_r_block_adc(vectorr_md_id, pro_vector_block_t, tmp_vector_block);
            vector_r_free_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, pro_vector_block_t);
        }

        tmp_vector_block  = VECTOR_BLOCK_NEXT(tmp_vector_block);
    }

    vector_r_clean_vector(vectorr_md_id, des_vector);
    vector_r_move_vector(vectorr_md_id, tmp_vector, des_vector);

    vector_r_destroy_vector(vectorr_md_id, tmp_vector);

    return (0);
}


/* ---------------------------------------- output interface ------------------------------------------------ */

UINT32 matrix_r_print_matrix_addr_info(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_block;

    sys_print(LOGSTDOUT,"matrix: 0x%lx, rotated_flag = %ld, row_num = %ld, col_num = %ld\n",
            matrix,
            MATRIX_GET_ROTATED_FLAG(matrix),
            MATRIX_GET_ROW_NUM(matrix),
            MATRIX_GET_COL_NUM(matrix));
    sys_print(LOGSTDOUT,"\n");

    MATRIX_ROW_BLOCKS_LOOP_NEXT(matrix, block_row_idx)
    {
        MATRIX_COL_BLOCKS_LOOP_NEXT(matrix, block_col_idx)
        {
            matrix_block = MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);

            /*block addr*/
            sys_print(LOGSTDOUT,"([%ld, %ld] 0x%lx,%ld,%ld,%ld) ",
                            block_row_idx, block_col_idx,
                            matrix_block,
                            MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block),
                            MATRIX_BLOCK_GET_ROW_NUM(matrix_block),
                            MATRIX_BLOCK_GET_COL_NUM(matrix_block)
                            );
           //matrix_r_print_matrix_block_addr_info(matrixr_md_id, matrix_block);
        }
        sys_print(LOGSTDOUT,"\n");
    }

    sys_print(LOGSTDOUT,"\n");
    return 0;
}

UINT32 matrix_r_block_print_row(const UINT32 matrixr_md_id, const UINT32 sub_row_no, const MATRIX_BLOCK  *matrix_block)
{
    UINT32 sub_col_num;
    UINT32 sub_col_no;

    REAL *data_addr;

    sub_col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);
    for(sub_col_no = 0; sub_col_no < sub_col_num; sub_col_no ++)
    {
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, sub_row_no, sub_col_no);

        if( NULL_PTR != data_addr )
        {
            //sys_print(LOGSTDOUT,"[%lx]%lf ", data_addr, *data_addr);
            sys_print(LOGSTDOUT,"%lf ", (*data_addr));
        }
        else
        {
            sys_print(LOGSTDOUT,"%32s ", "-");
        }
    }
    return (0);
}

UINT32 matrix_r_print_row(const UINT32 matrixr_md_id, const UINT32 row_no, const MATRIX *matrix)
{
    MATRIX_BLOCK  *matrix_block;

    UINT32 col_no;
    UINT32 col_num;

    REAL *data_addr;

    matrix_r_get_col_num(matrixr_md_id, matrix, &col_num);
    for(col_no = 0; col_no < col_num; col_no ++)
    {
        matrix_r_get_block(matrixr_md_id, matrix, row_no, col_no, &matrix_block);
        data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, (row_no % MATRIX_VECTOR_WIDTH), (col_no % MATRIX_VECTOR_WIDTH));
        if( NULL_PTR != data_addr )
        {
            //sys_print(LOGSTDOUT,"[%lx]%lf ", data_addr, *data_addr);
            sys_print(LOGSTDOUT,"%lf ", (*data_addr));
        }
        else
        {
            sys_print(LOGSTDOUT,"%32s ", "-");
        }
    }

    sys_print(LOGSTDOUT, "\n");

    return (0);
}

static UINT32 matrix_r_print_rows_data(const UINT32 matrixr_md_id, const MATRIX *matrix)
{
    UINT32 row_no;
    UINT32 row_num;

    sys_print(LOGSTDOUT,"matrix: 0x%lx, rotated_flag = %ld, row_num = %ld, col_num = %ld\n",
            matrix,
            MATRIX_GET_ROTATED_FLAG(matrix),
            MATRIX_GET_ROW_NUM(matrix),
            MATRIX_GET_COL_NUM(matrix));

    matrix_r_get_row_num(matrixr_md_id, matrix, &row_num);

    for(row_no = 0; row_no < row_num; row_no ++)
    {
        matrix_r_print_row(matrixr_md_id, row_no, matrix);
    }

    return (0);
}

UINT32 matrix_r_print_data_by_row(const UINT32 matrixr_md_id, const MATRIX *matrix)
{
    matrix_r_print_rows_data(matrixr_md_id, matrix);
    return (0);
}

UINT32 matrix_r_print_matrix_data_info(const UINT32 matrixr_md_id, MATRIX *matrix)
{
    matrix_r_print_rows_data(matrixr_md_id, matrix);
    return (0);
}

UINT32 matrix_r_print_matrix_block_addr_info(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 row_num;
    UINT32 col_num;
    UINT32 row_idx;
    UINT32 col_idx;

    REAL *data_addr;

    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    sys_print(LOGSTDOUT,"matrix_block: 0x%lx, rotated_flag = %ld, row_num = %ld, col_num = %ld\n",
            matrix_block,
            MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block),
            MATRIX_BLOCK_GET_ROW_NUM(matrix_block),
            MATRIX_BLOCK_GET_COL_NUM(matrix_block));

    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(row_idx, row_num)
    {
        MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(col_idx, col_num)
        {
            data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, row_idx, col_idx);
            sys_print(LOGSTDOUT, "%lx ", (data_addr));
        }
        sys_print(LOGSTDOUT, "\n");
    }
    return (0);
}

UINT32 matrix_r_print_matrix_block_data_info(const UINT32 matrixr_md_id, MATRIX_BLOCK *matrix_block)
{
    UINT32 row_num;
    UINT32 col_num;
    UINT32 row_idx;
    UINT32 col_idx;

    REAL *data_addr;

    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrix_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrix_block);

    sys_print(LOGSTDOUT,"matrix_block: 0x%lx, rotated_flag = %ld, row_num = %ld, col_num = %ld\n",
            matrix_block,
            MATRIX_BLOCK_GET_ROTATED_FLAG(matrix_block),
            MATRIX_BLOCK_GET_ROW_NUM(matrix_block),
            MATRIX_BLOCK_GET_COL_NUM(matrix_block));

    MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(row_idx, row_num)
    {
        MATRIX_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(col_idx, col_num)
        {
            data_addr = MATRIXR_BLOCK_GET_DATA_ADDR(matrix_block, row_idx, col_idx);
            sys_print(LOGSTDOUT, "%lf ", (*data_addr));
        }
        sys_print(LOGSTDOUT, "\n");
    }
    return (0);
}



#ifdef __cplusplus
}
#endif/*__cplusplus*/
