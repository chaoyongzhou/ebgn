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

#include "cmpic.inc"
#include "task.h"

#include "debug.h"

#include "print.h"

#include "findex.inc"

#include "cbc.h"

#define VECTORR_MD_CAPACITY()                   (cbc_md_capacity(MD_VECTORR))

#define VECTORR_MD_GET(vectorr_md_id)     ((VECTORR_MD *)cbc_md_get(MD_VECTORR, (vectorr_md_id)))

#define VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id)  \
((CMPI_ANY_MODI != (vectorr_md_id)) && ((NULL_PTR == VECTORR_MD_GET(vectorr_md_id)) || (0 == (VECTORR_MD_GET(vectorr_md_id)->usedcounter))))

/**
*   for test only
*
*   to query the status of VECTORR Module
*
**/
void vector_r_print_module_status(const UINT32 vectorr_md_id, LOG *log)
{
    VECTORR_MD *vectorr_md;
    UINT32 this_vectorr_md_id;

    for ( this_vectorr_md_id = 0; this_vectorr_md_id < VECTORR_MD_CAPACITY(); this_vectorr_md_id ++ )
    {
        vectorr_md = VECTORR_MD_GET(this_vectorr_md_id);

        if ( NULL_PTR != vectorr_md && 0 < vectorr_md->usedcounter )
        {
            sys_log(log,"VECTORR Module # %ld : %ld refered, refer REAL Module : %ld\n",
                    this_vectorr_md_id,
                    vectorr_md->usedcounter,
                    vectorr_md->real_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed VECTORR module
*
*
**/
UINT32 vector_r_free_module_static_mem(const UINT32 vectorr_md_id)
{
    VECTORR_MD  *vectorr_md;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_free_module_static_mem: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    free_module_static_mem(MD_VECTORR, vectorr_md_id);

    return 0;
}
/**
*
* start VECTORR module
*
**/
UINT32 vector_r_start( )
{
    VECTORR_MD *vectorr_md;
    UINT32 vectorr_md_id;

    UINT32 real_md_id;

    vectorr_md_id = cbc_md_new(MD_VECTORR, sizeof(VECTORR_MD));
    if(ERR_MODULE_ID == vectorr_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one VECTORR module */
    vectorr_md = (VECTORR_MD *)cbc_md_get(MD_VECTORR, vectorr_md_id);
    vectorr_md->usedcounter   = 0;
    vectorr_md->real_md_id    = ERR_MODULE_ID;

    /* create a new module node */
    real_md_id = ERR_MODULE_ID;

    init_static_mem();

    /*default setting which will be override after vector_r_set_mod_mgr calling*/
    vectorr_md->mod_mgr = mod_mgr_new(vectorr_md_id, LOAD_BALANCING_LOOP);

    real_md_id = real_start(); /*no REAL_MD need to allock, here is one trick at present*/

    vectorr_md->real_md_id = real_md_id;
    vectorr_md->usedcounter = 1;

    //dbg_log(SEC_0098_VECTORR, 3)(LOGSTDOUT, "========================= vector_r_start: VECTORR table info:\n");
    //vector_r_print_module_status(vectorr_md_id, LOGSTDOUT);
    //cbc_print();

    return ( vectorr_md_id );
}

/**
*
* end VECTORR module
*
**/
void vector_r_end(const UINT32 vectorr_md_id)
{
    VECTORR_MD *vectorr_md;

    UINT32 real_md_id;

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);
    if ( NULL_PTR == vectorr_md )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_end: vectorr_md_id = %ld is overflow.\n",vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < vectorr_md->usedcounter )
    {
        vectorr_md->usedcounter --;
        return ;
    }

    if ( 0 == vectorr_md->usedcounter )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_end: vectorr_md_id = %ld is not started.\n",vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }

    //task_brd_mod_mgr_rmv(vectorr_md->task_brd, vectorr_md->mod_mgr);
    mod_mgr_free(vectorr_md->mod_mgr);
    vectorr_md->mod_mgr  = NULL_PTR;

    /* if nobody else occupied the module,then free its resource */
    real_md_id = vectorr_md->real_md_id;

    real_end(real_md_id);

    /* free module : */
    //vector_r_free_module_static_mem(vectorr_md_id);
    vectorr_md->real_md_id = ERR_MODULE_ID;
    vectorr_md->usedcounter = 0;

    cbc_md_free(MD_VECTORR, vectorr_md_id);

    breathing_static_mem();

    //dbg_log(SEC_0098_VECTORR, 3)(LOGSTDOUT, "========================= vector_r_end: VECTORR table info:\n");
    //vector_r_print_module_status(vectorr_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

/**
*
* initialize mod mgr of VECTORR module
*
**/
UINT32 vector_r_set_mod_mgr(const UINT32 vectorr_md_id, const MOD_MGR * src_mod_mgr)
{
    VECTORR_MD *vectorr_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_set_mod_mgr: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);
    des_mod_mgr = vectorr_md->mod_mgr;

    mod_mgr_limited_clone(vectorr_md_id, src_mod_mgr, des_mod_mgr);

    return (0);
}

/**
*
* get mod mgr of VECTORR module
*
**/
MOD_MGR * vector_r_get_mod_mgr(const UINT32 vectorr_md_id)
{
    VECTORR_MD *vectorr_md;

    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        return (MOD_MGR *)0;
    }

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);
    return (vectorr_md->mod_mgr);
}

UINT32 vector_r_init_block(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    VECTOR_BLOCK_SET_NUM(vector_block, MATRIX_VECTOR_WIDTH);

    VECTOR_BLOCK_INIT(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, MATRIX_VECTOR_WIDTH)
    {
        VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, NULL_PTR);
    }
    return (0);
}

UINT32 vector_r_alloc_block( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR_BLOCK **vector_block)
{
    if ( MD_VECTORR != vectorr_md_type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_alloc_block: invalid vectorr_md_type = %ld\n", vectorr_md_type);
        exit(0);
    }

    if ( MM_VECTOR_BLOCK != type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_alloc_block: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(vectorr_md_type, vectorr_md_id, type, vector_block, LOC_VECTORR_0001);

    vector_r_init_block(vectorr_md_id, *vector_block);

    return (0);
}

static UINT32 vector_r_free_data_area( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    REAL *data_addr;


    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, MATRIX_VECTOR_WIDTH)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);

        if( NULL_PTR != data_addr )
        {
            free_static_mem(vectorr_md_type, vectorr_md_id, MM_REAL, data_addr, LOC_VECTORR_0002);
        }
    }

    return (0);
}

/**
*
* clear data_area but do not free block itself
*
**/
UINT32 vector_r_clear_block( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    if ( MD_VECTORR != vectorr_md_type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_clear_block: invalid vectorr_md_type = %ld\n", vectorr_md_type);
        exit(0);
    }

    vector_r_free_data_area(vectorr_md_type, vectorr_md_id, vector_block);

    return (0);
}

/**
*
* clean data_area without check vectorr_md_type, and do not free block itself
*
**/
UINT32 vector_r_clean_block( const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    vector_r_free_data_area(MD_VECTORR, vectorr_md_id, vector_block);

    return (0);
}

UINT32 vector_r_free_block( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR_BLOCK *vector_block)
{
    if ( MM_VECTOR_BLOCK != type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_free_block: invalid type = %ld\n",type);
        exit(0);
    }

    vector_r_clear_block(vectorr_md_type, vectorr_md_id, vector_block);
    free_static_mem(vectorr_md_type, vectorr_md_id, type, vector_block, LOC_VECTORR_0003);

    return (0);
}

UINT32 vector_r_init_vector(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_SET_ROTATED_FLAG(vector, 0);

    VECTOR_HEAD_INIT(vector);

    VECTOR_SET_NUM(vector, 0);

    return (0);
}

static UINT32 vector_r_alloc_vector( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR **vector)
{
    if ( MD_VECTORR != vectorr_md_type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_alloc_vector: invalid vectorr_md_type = %ld\n", vectorr_md_type);
        exit(0);
    }
    if ( MM_VECTOR != type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_alloc_vector: invalid type = %ld\n",type);
        exit(0);
    }

    alloc_static_mem(vectorr_md_type, vectorr_md_id, type, vector, LOC_VECTORR_0004);

    vector_r_init_vector(vectorr_md_id, *vector);

    return (0);
}

static UINT32 vector_r_free_vector( const UINT32 vectorr_md_type, const UINT32 vectorr_md_id, const UINT32 type, VECTOR *vector)
{
    if ( MD_VECTORR != vectorr_md_type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_free_vector: invalid vectorr_md_type = %ld\n", vectorr_md_type);
        exit(0);
    }
    if ( MM_VECTOR != type )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT,"error:vector_r_free_vector: invalid type = %ld\n",type);
        exit(0);
    }

    free_static_mem(vectorr_md_type, vectorr_md_id, type, vector, LOC_VECTORR_0005);

    return (0);
}

UINT32 vector_r_new_vector_skeleton(const UINT32 vectorr_md_id, UINT32 num, VECTOR *vector)
{
    UINT32 block_num;

    UINT32 block_idx;

    UINT32 sub_num;

    UINT32 rotated_flag;

    VECTOR_BLOCK  *vector_block;

    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_new_vector_skeleton: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_new_vector_skeleton: vectorr module #0x%lx not started.\n",
                 vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    rotated_flag = 0;

    /*set rotated_flag to default 0*/
    VECTOR_SET_ROTATED_FLAG(vector, rotated_flag);

    /*set row num and col num to vector. not sure if it is suitable or not here*/
    VECTOR_SET_NUM(vector, num);

    block_num = ( num + MATRIX_VECTOR_WIDTH - 1 ) / MATRIX_VECTOR_WIDTH;

    /*create block list*/
    for( block_idx = 0; block_idx < block_num; block_idx ++ )
    {
        vector_r_alloc_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, &vector_block);
        VECTOR_ADD_BLOCK_TAIL(vector, vector_block);
    }

    /*adjust num of last block if necessary*/
    sub_num = (num % MATRIX_VECTOR_WIDTH);
    if( 0 < sub_num )
    {
        vector_block = VECTOR_LAST_BLOCK(vector);
        if( vector_block != VECTOR_NULL_BLOCK(vector) )
        {
            VECTOR_BLOCK_SET_NUM(vector_block, sub_num);
        }
    }

    return (0);
}

UINT32 vector_r_new_vector(const UINT32 vectorr_md_id, const UINT32 num, VECTOR **ppvector)
{
    if( NULL_PTR == ppvector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_new_vector: ppvector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_new_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    /*create vector node itself*/
    vector_r_alloc_vector(MD_VECTORR, vectorr_md_id, MM_VECTOR, ppvector);

    /*create vector skeleton*/
    vector_r_new_vector_skeleton(vectorr_md_id, num, (*ppvector));

    return (0);
}

/**
*
* clear data_area but do not free skeleton
*
**/
UINT32 vector_r_clear_vector(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_clear_vector: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_clear_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    /*clear blocks*/
    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_clear_block(MD_VECTORR, vectorr_md_id, vector_block);
    }

    return (0);
}

/**
*
* destroy skeleton but do not free vector itself
*
**/
UINT32 vector_r_clean_vector(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_clean_vector: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_clean_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    /*destroy blocks*/
    vector_block = VECTOR_LAST_BLOCK(vector);
    while( vector_block != VECTOR_NULL_BLOCK(vector) )
    {
        VECTOR_BLOCK_DEL(vector_block);

        vector_r_free_block(MD_VECTORR, vectorr_md_id, MM_VECTOR_BLOCK, vector_block);

        vector_block = VECTOR_LAST_BLOCK(vector);
    }

    /*reset num to zero*/
    VECTOR_SET_NUM(vector, 0);

    /*reset rotated_flag to zero*/
    VECTOR_SET_ROTATED_FLAG(vector, 0);

    return (0);
}

/**
*
* move src_vector skeleton and data area to des_vector
* note:
*     des_vector must have no skeleton or data area. otherwise, skeleton and data area will be lost without free
*
**/
UINT32 vector_r_move_vector(const UINT32 vectorr_md_id, VECTOR *src_vector, VECTOR *des_vector)
{
    UINT32 rotated_flag;

    if( NULL_PTR == src_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_move_vector: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_move_vector: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_move_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    rotated_flag = VECTOR_GET_ROTATED_FLAG(src_vector);

    VECTOR_SET_ROTATED_FLAG(des_vector, rotated_flag);

    VECTOR_SET_NUM(des_vector, VECTOR_GET_NUM(src_vector));

    VECTOR_HEAD_MOVE(src_vector, des_vector);

    return (0);
}

/**
*
* destroy a vector
* all data area pointers, block pointers, and vector itself pointer will be destroyed.
* note:
*       vector pointer cannot be reused after calling
*
**/
UINT32 vector_r_destroy_vector(const UINT32 vectorr_md_id, VECTOR *vector)
{
    if( NULL_PTR == vector )
    {
        return (0);
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_destroy_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vector_r_clean_vector(vectorr_md_id, vector);

    /*destroy vector node itself*/
    vector_r_free_vector(MD_VECTORR, vectorr_md_id, MM_VECTOR, vector);

    return (0);
}

/**
*
* row_no value range 0,1,2,....
*
**/
static UINT32 vector_r_loc_block(const UINT32 vectorr_md_id, const UINT32 idx, const VECTOR *vector, VECTOR_BLOCK **ppvector_block)
{
    VECTOR_BLOCK *vector_block;
    UINT32 num;

    if( idx >= VECTOR_GET_NUM(vector) )
    {
        *ppvector_block = NULL_PTR;
        return ((UINT32)(-1));
    }

    num = 0;
    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        num += VECTOR_BLOCK_GET_NUM(vector_block);

        if( num > idx )
        {
            *ppvector_block = vector_block;
            return (0);;
        }
    }

    *ppvector_block = NULL_PTR;

    return ((UINT32)(-1));
}


/**
*
* here not consider the insertion row into one existing vector with data
* just insert the row into one empty vector with skeleton including vector node itself, blocks
*
**/
static UINT32 vector_r_block_insert_data(const UINT32 vectorr_md_id, const REAL *pdata[], const UINT32 data_num, VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    UINT32 left_data_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_insert_data: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    left_data_num = data_num;

    sub_num = VECTOR_BLOCK_GET_NUM(vector_block);

    for( sub_idx = 0; sub_idx < sub_num && left_data_num > 0; sub_idx ++, left_data_num --  )
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR == data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &data_addr, LOC_VECTORR_0006);
        VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, data_addr);
        }
        REAL_CLONE(vectorr_md->real_md_id,*(pdata[ sub_idx ]), (*data_addr));
    }

    return (left_data_num);
}

/**
*
* here not consider the insertion row into one existing vector with data
* just insert the row into one empty vector with skeleton only including vector node itself, blocks
*
**/
UINT32 vector_r_insert_data(const UINT32 vectorr_md_id, const REAL *pdata[], const UINT32 data_num, VECTOR *vector)
{
    VECTOR_BLOCK *vector_block;

    UINT32 left_data_num;

    left_data_num = data_num;

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        if( 0 == left_data_num )
        {
            return (left_data_num);
        }

        left_data_num = vector_r_block_insert_data(vectorr_md_id, pdata + data_num - left_data_num, left_data_num, vector_block);
    }

    return (left_data_num);
}

static UINT32 vector_r_clone_block(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_clone_block: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0007);
                VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            }

            REAL_CLONE(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
        }
        else
        {
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, des_data_addr, LOC_VECTORR_0008);
            }
        }
    }
    return ( 0 );
}

/**
*
*  vector_block = - vector_block
*
**/
static UINT32 vector_r_block_neg_self(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_neg_self: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR != data_addr )
        {
            data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
            if( NULL_PTR != data_addr )
            {
                REAL_NEG(vectorr_md->real_md_id, (*data_addr), (*data_addr));
            }
        }
    }
    return ( 0 );
}

/**
*
*  des_vector_block = - src_vector_block
*  note:
*      here must addr(des_vector_block) != addr(src_vector_block)
*
**/
static UINT32 vector_r_block_neg(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_neg: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0009);
                VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            }

            src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
            REAL_NEG(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
        }
        else
        {
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, des_data_addr, LOC_VECTORR_0010);
            }
        }
    }
    return ( 0 );
}

/**
*
*  vector_block is zero or not
*  note:
*      here vector_block must not be null pointer
*
**/
EC_BOOL vector_r_block_is_zero(const UINT32 vectorr_md_id, const VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_is_zero: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR != data_addr && (EC_FALSE == REAL_ISZERO(vectorr_md->real_md_id,(*data_addr))) )
        {
            return (EC_FALSE);
        }
    }
    return ( EC_TRUE );
}

/**
*
*  vector_block is one or not
*  note:
*      here vector_block must not be null pointer
*
**/
static EC_BOOL vector_r_block_is_one(const UINT32 vectorr_md_id, const VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_is_zero: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR == data_addr || (EC_FALSE == REAL_ISONE(vectorr_md->real_md_id,(*data_addr))) )
        {
            return (EC_FALSE);
        }
    }
    return ( EC_TRUE );
}

/**
*
*  vector_block_1 is equal to vector_block_2 or not
*  note:
*      here vector_block_1 and vector_block_2 must not be null pointer
*
**/
static EC_BOOL vector_r_block_cmp(const UINT32 vectorr_md_id, const VECTOR_BLOCK *vector_block_1, const VECTOR_BLOCK *vector_block_2)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr_1;
    REAL *data_addr_2;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_cmp: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(vector_block_1);
    if( sub_num != VECTOR_BLOCK_GET_NUM(vector_block_2) )
    {
        return (EC_FALSE);
    }

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr_1 = VECTORR_BLOCK_GET_DATA_ADDR(vector_block_1, sub_idx);
        data_addr_2 = VECTORR_BLOCK_GET_DATA_ADDR(vector_block_2, sub_idx);

        if( data_addr_1 == data_addr_2 )
        {
            continue;
        }

        if( NULL_PTR == data_addr_1 || NULL_PTR == data_addr_2 )
        {
            return (EC_FALSE);
        }

        if( EC_FALSE == REAL_ISEQU(vectorr_md->real_md_id,(*data_addr_1), (*data_addr_2)) )
        {
            return (EC_FALSE);
        }
    }
    return ( EC_TRUE );
}

/**
*
*  vector_block = 0
*  note:
*
**/
static UINT32 vector_r_block_set_zero(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_set_zero: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR != data_addr )
        {
            REAL_SETZERO(vectorr_md->real_md_id,(*data_addr));
        }
    }

    return (0);
}

/**
*
*  vector_block = 1
*  note:
*
**/
static UINT32 vector_r_block_set_one(const UINT32 vectorr_md_id, VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;

    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_set_one: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR == data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &data_addr, LOC_VECTORR_0011);
            VECTOR_BLOCK_SET_DATA_ADDR(vector_block, sub_idx, data_addr);
        }
        REAL_SETONE(vectorr_md->real_md_id,(*data_addr));
    }

    return (0);
}

/**
*
*  vector_block = s_data * vector_block
*
**/
static UINT32 vector_r_block_s_mul_self(const UINT32 vectorr_md_id, const REAL *s_data_addr, VECTOR_BLOCK *vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_s_mul_self: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
        if( NULL_PTR != data_addr )
        {
            data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);
            if( NULL_PTR != data_addr )
            {
                REAL_MUL_SELF(vectorr_md->real_md_id, (*s_data_addr), (*data_addr));
            }
        }
    }
    return ( 0 );
}

/**
*
*  des_vector_block = s_data * src_vector_block
*  note:
*      here must addr(des_vector_block) != addr(src_vector_block)
*
**/
static UINT32 vector_r_block_s_mul(const UINT32 vectorr_md_id, const REAL *s_data_addr, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_s_mul: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num  = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR != src_data_addr )
        {
            if( NULL_PTR == des_data_addr )
            {
                alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0012);
                VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            }

            src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
            REAL_MUL(vectorr_md->real_md_id, (*s_data_addr), (*src_data_addr), (*des_data_addr));
        }
        else
        {
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, des_data_addr, LOC_VECTORR_0013);
            }
        }
    }
    return ( 0 );
}

/**
*
*  des_vector_block += src_vector_block
*  note:
*      here des_vector_block must not be src_vector_block
*
**/
UINT32 vector_r_block_adc(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_adc: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if(VECTOR_BLOCK_GET_NUM(src_vector_block) != VECTOR_BLOCK_GET_NUM(des_vector_block))
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDOUT, "error:vector_r_block_adc: mismatchable vector: src num = %d, des num = %d\n",
                        VECTOR_BLOCK_GET_NUM(src_vector_block),
                        VECTOR_BLOCK_GET_NUM(des_vector_block));
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR == src_data_addr )
        {
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0014);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            REAL_CLONE(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
            continue;
        }

        REAL_ADC(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_vector_block = src_vector_block_1 + src_vector_block_2
*  note:
*      here des_vector_block must not be src_vector_block_1 or src_vector_block_2
*
**/
UINT32 vector_r_block_add(const UINT32 vectorr_md_id,
                                 const VECTOR_BLOCK *src_vector_block_1,
                                 const VECTOR_BLOCK *src_vector_block_2,
                                 VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_add: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block_1);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr_1 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_1, sub_idx);
        src_data_addr_2 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_2, sub_idx);
        des_data_addr   = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block  , sub_idx);

        if( NULL_PTR == src_data_addr_1 && NULL_PTR == src_data_addr_2 )
        {
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, des_data_addr, LOC_VECTORR_0015);
                VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            }
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0016);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
        }

        if( NULL_PTR == src_data_addr_1 )
        {
            REAL_CLONE(vectorr_md->real_md_id,(*src_data_addr_2), (*des_data_addr));
            continue;
        }

        if( NULL_PTR == src_data_addr_2 )
        {
            REAL_CLONE(vectorr_md->real_md_id,(*src_data_addr_1), (*des_data_addr));
            continue;
        }

        REAL_ADD(vectorr_md->real_md_id,(*src_data_addr_1), (*src_data_addr_2), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_vector_block -= src_vector_block
*  note:
*      here des_vector_block must not be src_vector_block
*
**/
UINT32 vector_r_block_sbb(const UINT32 vectorr_md_id, const VECTOR_BLOCK *src_vector_block, VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;

    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_sbb: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR == src_data_addr )
        {
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0017);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            REAL_NEG(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
            continue;
        }

        REAL_SBB(vectorr_md->real_md_id,(*src_data_addr), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_vector_block = src_vector_block_1 - src_vector_block_2
*  note:
*      here des_vector_block must not be src_vector_block_1 or src_vector_block_2
*
**/
UINT32 vector_r_block_sub(const UINT32 vectorr_md_id,
                                 const VECTOR_BLOCK *src_vector_block_1,
                                 const VECTOR_BLOCK *src_vector_block_2,
                                 VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *des_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_block_sub: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block_1);

    VECTOR_BLOCK_SET_NUM(des_vector_block, sub_num);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr_1 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_1, sub_idx);
        src_data_addr_2 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_2, sub_idx);
        des_data_addr   = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block  , sub_idx);

        if( NULL_PTR == src_data_addr_1 && NULL_PTR == src_data_addr_2 )
        {
            if( NULL_PTR != des_data_addr )
            {
                free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, des_data_addr, LOC_VECTORR_0018);
                VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
            }
            continue;
        }

        if( NULL_PTR == des_data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0019);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);
        }

        if( NULL_PTR == src_data_addr_1 )
        {
            REAL_NEG(vectorr_md->real_md_id,(*src_data_addr_2), (*des_data_addr));
            continue;
        }

        if( NULL_PTR == src_data_addr_2 )
        {
            REAL_CLONE(vectorr_md->real_md_id,(*src_data_addr_1), (*des_data_addr));
            continue;
        }

        REAL_SUB(vectorr_md->real_md_id,(*src_data_addr_1), (*src_data_addr_2), (*des_data_addr));
    }
    return (0);
}

/**
*
*  des_vector_block.row[ des_row_idx ] = des_vector_block.row[ des_row_idx ] - r_data * src_vector_block.row[ src_row_idx ]
*  note:
*      1. col num of src_vector_block must be same as that of des_vector_block
*
**/
static UINT32 vector_r_block_shrink(const UINT32 vectorr_md_id,
                              const REAL *r_data_addr,
                              const VECTOR_BLOCK *src_vector_block,
                              VECTOR_BLOCK *des_vector_block)
{
    VECTORR_MD  *vectorr_md;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr;
    REAL *des_data_addr;
    REAL *tmp_data_addr;

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_shrink: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);

    if( NULL_PTR == r_data_addr || EC_TRUE == REAL_ISZERO(vectorr_md->real_md_id,*r_data_addr) )
    {
        return (0);
    }

    alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &tmp_data_addr, LOC_VECTORR_0020);

    sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block);

    VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
    {
        src_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block, sub_idx);
        des_data_addr = VECTORR_BLOCK_GET_DATA_ADDR(des_vector_block, sub_idx);

        if( NULL_PTR == des_data_addr )
        {
            alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &des_data_addr, LOC_VECTORR_0021);
            VECTOR_BLOCK_SET_DATA_ADDR(des_vector_block, sub_idx, des_data_addr);

            REAL_MUL(vectorr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
            REAL_NEG(vectorr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
            continue;
        }

        REAL_MUL(vectorr_md->real_md_id,(*src_data_addr), (*r_data_addr), (*tmp_data_addr));
        REAL_SBB(vectorr_md->real_md_id,(*tmp_data_addr), (*des_data_addr));
    }

    free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, tmp_data_addr, LOC_VECTORR_0022);

    return (0);
}

/**
*
* get row num of vector
*
**/
UINT32 vector_r_get_row_num(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *row_num)
{
    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_row_num: vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == row_num )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_row_num: row_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_get_row_num: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( 0 == VECTOR_GET_ROTATED_FLAG(vector) )
    {
        (*row_num) = 1;
    }
    else
    {
        (*row_num) = VECTOR_GET_NUM(vector);
    }

    return (0);
}

/**
*
* get col num of vector
*
**/
UINT32 vector_r_get_col_num(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *col_num)
{
    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_col_num: vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == col_num )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_col_num: col_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_get_col_num: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( 0 == VECTOR_GET_ROTATED_FLAG(vector) )
    {
        (*col_num) = VECTOR_GET_NUM(vector);
    }
    else
    {
        (*col_num) = 1;
    }

    return (0);
}

/**
*
* get type of vector
* if vector type is (m x n), then return row_num = m, and col_num = n
*
**/
UINT32 vector_r_get_type(const UINT32 vectorr_md_id, const VECTOR *vector, UINT32 *row_num, UINT32 *col_num)
{
    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_type: vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == row_num )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_type: row_num is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == col_num )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_get_type: col_num is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_get_type: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( 0 == VECTOR_GET_ROTATED_FLAG(vector) )
    {
        (*row_num) = 1;
        (*col_num) = VECTOR_GET_NUM(vector);
    }
    else
    {
        (*row_num) = VECTOR_GET_NUM(vector);
        (*col_num) = 1;
    }

    return (0);
}

/**
*
* clone src vector to des vector
*     des_vector = src_vector
* note:
*    here des_vector must be empty vector, otherwise, its skeleton will be lost without notification
*
**/
UINT32 vector_r_clone(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector)
{
    UINT32 num;

    VECTOR_BLOCK  *src_vector_block;
    VECTOR_BLOCK  *des_vector_block;

    if( NULL_PTR == src_vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_clone: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_clone: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_clone: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    num = VECTOR_GET_NUM(src_vector);

    if( num != VECTOR_GET_NUM(des_vector) )
    {
        /*clean and re-new skeleton of old des_vector*/
        vector_r_clean_vector(vectorr_md_id, des_vector);
        vector_r_new_vector_skeleton(vectorr_md_id, num, des_vector);
    }

    VECTOR_SET_ROTATED_FLAG(des_vector, VECTOR_GET_ROTATED_FLAG(src_vector));

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block, src_vector, des_vector_block, des_vector)
    {
        vector_r_clone_block(vectorr_md_id, src_vector_block, des_vector_block);
    }

    return (0);
}

/**
*
* rotate a vector
*     vector = T(vector)
*
**/
UINT32 vector_r_rotate(const UINT32 vectorr_md_id, VECTOR *vector)
{
    if( NULL_PTR == vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_rotate: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_rotate: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    VECTOR_XCHG_ROTATED_FLAG(vector);

    return (0);
}

/**
*
* vector = 0
*
**/
UINT32 vector_r_set_zero(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    if( NULL_PTR == vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_set_zero: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_set_zero: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_block_set_zero(vectorr_md_id, vector_block);
    }
    return (0);
}

/**
*
* vector = 1
* note:
*    1. row num of vector must be same as col num
*       2. must row num > 0
*
**/
UINT32 vector_r_set_one(const UINT32 vectorr_md_id, VECTOR *vector)
{
    UINT32 num;

    VECTOR_BLOCK  *vector_block;

    if( NULL_PTR == vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_set_one: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_set_one: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    num = VECTOR_GET_NUM(vector);

    if( 0 == num )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_set_one: not valid vector: num = %ld\n", num);
        return ((UINT32)(-1));
    }

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_block_set_one(vectorr_md_id, vector_block);
    }

    return (0);
}

/**
*
* exchange two rows of the vector
* if row no is overflow, then report error and do nothing
*
**/
UINT32 vector_r_xchg(const UINT32 vectorr_md_id, const UINT32 idx_1, const UINT32 idx_2, VECTOR *vector)
{
    UINT32 sub_idx_1;
    UINT32 sub_idx_2;

    VECTOR_BLOCK *vector_block_1;
    VECTOR_BLOCK *vector_block_2;

    REAL *data_addr_1;
    REAL *data_addr_2;

    if( NULL_PTR == vector )
    {
        /*do nothing*/
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_xchg: vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_xchg: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( idx_1 >= VECTOR_GET_NUM(vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR, "vector_r_xchg: idx_1 %ld overflow where vector num is %ld\n", idx_1, VECTOR_GET_NUM(vector));
        return ((UINT32)(-1));
    }

    if( idx_2 >= VECTOR_GET_NUM(vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR, "vector_r_xchg: idx_2 %ld overflow where vector num is %ld\n", idx_2, VECTOR_GET_NUM(vector));
        return ((UINT32)(-1));
    }

    vector_r_loc_block(vectorr_md_id, idx_1, vector, &vector_block_1);
    vector_r_loc_block(vectorr_md_id, idx_2, vector, &vector_block_2);

    sub_idx_1 = (idx_1 % MATRIX_VECTOR_WIDTH);
    sub_idx_2 = (idx_2 % MATRIX_VECTOR_WIDTH);

    data_addr_1 = VECTORR_BLOCK_GET_DATA_ADDR(vector_block_1, sub_idx_1);
    data_addr_2 = VECTORR_BLOCK_GET_DATA_ADDR(vector_block_2, sub_idx_2);

    VECTOR_BLOCK_SET_DATA_ADDR(vector_block_2, sub_idx_2, data_addr_1);
    VECTOR_BLOCK_SET_DATA_ADDR(vector_block_1, sub_idx_1, data_addr_2);

    return (0);
}

/**
*
* vector is_zero operation
*    if vector is zero vector, i.e, all data_area are zero or null pointers, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_is_zero(const UINT32 vectorr_md_id, const VECTOR *vector)
{
    VECTOR_BLOCK *vector_block;

    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_is_zero: vector is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_is_zero: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        if( EC_FALSE == vector_r_block_is_zero(vectorr_md_id, vector_block) )
        {
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

/**
*
* vector is_one operation
*    if vector is unit vector, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_is_one(const UINT32 vectorr_md_id, const VECTOR *vector)
{
    VECTOR_BLOCK *vector_block;

    if( NULL_PTR == vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_is_one: vector is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_is_one: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( EC_TRUE == VECTOR_IS_EMPTY(vector) )
    {
        return (EC_FALSE);
    }

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        if( EC_FALSE == vector_r_block_is_one(vectorr_md_id, vector_block) )
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

/**
*
* vector cmp operation
*    if vector_1 == vector_2, then return EC_TRUE
*    else return EC_FALSE
*
**/
EC_BOOL vector_r_cmp(const UINT32 vectorr_md_id, const VECTOR *vector_1, const VECTOR *vector_2)
{
    VECTOR_BLOCK *vector_block_1;
    VECTOR_BLOCK *vector_block_2;

    if( NULL_PTR == vector_1 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_cmp: vector_1 is null pointer\n");
        return (EC_TRUE);
    }

    if( NULL_PTR == vector_2 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_cmp: vector_2 is null pointer\n");
        return (EC_TRUE);
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_is_cmp: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( vector_1 == vector_2 )
    {
        return (EC_TRUE);
    }

    if( NULL_PTR == vector_1 || NULL_PTR == vector_2 )
    {
        return (EC_FALSE);
    }

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(vector_block_1, vector_1, vector_block_2, vector_2)
    {
        if( EC_FALSE == vector_r_block_cmp(vectorr_md_id, vector_block_1, vector_block_2) )
        {
                return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

/**
*
* vector neg self operation
*     vector = - vector
*
**/
static UINT32 vector_r_neg_self(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_block_neg_self(vectorr_md_id, vector_block);
    }
    return (0);
}

/**
*
* vector neg operation
*     des_vector = - src_vector
*
**/
UINT32 vector_r_neg(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block;
    VECTOR_BLOCK  *des_vector_block;

    UINT32 num;

    if( NULL_PTR == src_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_neg: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_neg: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_neg: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( src_vector == des_vector )
    {
        return vector_r_neg_self(vectorr_md_id, des_vector);
    }

    num = VECTOR_GET_NUM(src_vector);
    if( num != VECTOR_GET_NUM(des_vector) )
    {
        /*clean and re-new skeleton of old des_vector*/
        vector_r_clean_vector(vectorr_md_id, des_vector);
        vector_r_new_vector_skeleton(vectorr_md_id, num, des_vector);
    }

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block, src_vector, des_vector_block, des_vector)
    {
        vector_r_block_neg(vectorr_md_id, src_vector_block, des_vector_block);
    }
    return (0);
}

/**
*
* vector adc operation
*     des_vector += src_vector
*
**/
UINT32 vector_r_adc(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block;
    VECTOR_BLOCK  *des_vector_block;

    if( NULL_PTR == src_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_adc: src_vector is null pointer\n");
        return (0);
    }

    if( NULL_PTR == des_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_adc: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_adc: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( VECTOR_GET_ROTATED_FLAG(src_vector) != VECTOR_GET_ROTATED_FLAG(des_vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_adc: not matchable vector: rotated_flag of src_vector = %ld, rotated_flag of des_vector = %ld\n",
                        VECTOR_GET_ROTATED_FLAG(src_vector),
                        VECTOR_GET_ROTATED_FLAG(des_vector));
        return ((UINT32)(-1));
    }

    if(  VECTOR_GET_NUM(src_vector) != VECTOR_GET_NUM(des_vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_adc: not matchable vector: num of src_vector = %ld, num of des_vector = %ld\n",
                        VECTOR_GET_NUM(src_vector),
                        VECTOR_GET_NUM(des_vector));
        return ((UINT32)(-1));
    }

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block, src_vector, des_vector_block, des_vector)
    {
        vector_r_block_adc(vectorr_md_id, src_vector_block, des_vector_block);
    }
    return (0);
}

/**
*
* vector add operation
*     des_vector = src_vector_1 + src_vector_2
*
**/
UINT32 vector_r_add(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block_1;
    VECTOR_BLOCK  *src_vector_block_2;
    VECTOR_BLOCK  *des_vector_block;

    UINT32 num;
    UINT32 rotated_flag;

    if( NULL_PTR == src_vector_1 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: src_vector_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector_2 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: src_vector_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_add: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( src_vector_1 == des_vector )
    {
        return vector_r_adc(vectorr_md_id, src_vector_2, des_vector);
    }

    if( src_vector_2 == des_vector )
    {
        return vector_r_adc(vectorr_md_id, src_vector_1, des_vector);
    }

    if( NULL_PTR == src_vector_1 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: src_vector_1 is null pointer\n");
        return vector_r_clone(vectorr_md_id, src_vector_2, des_vector);
    }

    if( NULL_PTR == src_vector_2 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: src_vector_2 is null pointer\n");
        return vector_r_clone(vectorr_md_id, src_vector_1, des_vector);
    }

    if( VECTOR_GET_ROTATED_FLAG(src_vector_1) != VECTOR_GET_ROTATED_FLAG(src_vector_2) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: not matchable vector: rotated_flag of src_vector_1 = %ld,rotated_flag of src_vector_2 = %ld\n",
                        VECTOR_GET_ROTATED_FLAG(src_vector_1),
                        VECTOR_GET_ROTATED_FLAG(src_vector_2));
        return ((UINT32)(-1));
    }

    if( VECTOR_GET_NUM(src_vector_1) != VECTOR_GET_NUM(src_vector_2) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_add: not matchable vector: num of src_vector_1 = %ld, num of src_vector_2 = %ld\n",
                        VECTOR_GET_NUM(src_vector_1),
                        VECTOR_GET_NUM(src_vector_2));
        return ((UINT32)(-1));
    }

    rotated_flag = VECTOR_GET_ROTATED_FLAG(src_vector_1);
    num = VECTOR_GET_NUM(src_vector_1);

    if( num != VECTOR_GET_NUM(des_vector) )
    {
        /*clean and re-new skeleton of old des_vector*/
        vector_r_clean_vector(vectorr_md_id, des_vector);
        vector_r_new_vector_skeleton(vectorr_md_id, num, des_vector);
    }

    if( rotated_flag != VECTOR_GET_ROTATED_FLAG(des_vector) )
    {
        VECTOR_SET_ROTATED_FLAG(des_vector, rotated_flag);
    }

    VECTOR_BLOCK_TRIPLE_LOOP_NEXT(src_vector_block_1, src_vector_1,
                                  src_vector_block_2, src_vector_2,
                                  des_vector_block, des_vector)
    {
        vector_r_block_add(vectorr_md_id, src_vector_block_1, src_vector_block_2, des_vector_block);
    }

    return (0);
}

/**
*
* vector sbb operation
*     des_vector -= src_vector
*
**/
UINT32 vector_r_sbb(const UINT32 vectorr_md_id, const VECTOR *src_vector, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block;
    VECTOR_BLOCK  *des_vector_block;

    if( NULL_PTR == src_vector)
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sbb: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector)
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sbb: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_sbb: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( des_vector == src_vector )
    {
        return vector_r_clear_vector(vectorr_md_id, des_vector);
    }

    if(  VECTOR_GET_ROTATED_FLAG(src_vector) != VECTOR_GET_ROTATED_FLAG(des_vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sbb: not matchable vector: rotated_flag of src_vector = %ld, rotated_flag of des_vector = %ld\n",
                        VECTOR_GET_ROTATED_FLAG(src_vector),
                        VECTOR_GET_ROTATED_FLAG(des_vector));
        return ((UINT32)(-1));
    }

    if(  VECTOR_GET_NUM(src_vector) != VECTOR_GET_NUM(des_vector) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sbb: not matchable vector: num of src_vector = %ld, num of des_vector = %ld\n",
                        VECTOR_GET_NUM(src_vector),
                        VECTOR_GET_NUM(des_vector));
        return ((UINT32)(-1));
    }

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block, src_vector, des_vector_block, des_vector)
    {
        vector_r_block_sbb(vectorr_md_id, src_vector_block, des_vector_block);
    }

    return (0);
}

/**
*
* vector sub operation
*     des_vector = src_vector_1 - src_vector_2
*
**/
UINT32 vector_r_sub(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block_1;
    VECTOR_BLOCK  *src_vector_block_2;
    VECTOR_BLOCK  *des_vector_block;

    UINT32 rotated_flag;
    UINT32 num;

    if( NULL_PTR == src_vector_1 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sub: src_vector_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector_2 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sub: src_vector_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector)
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sub: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_sub: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( src_vector_1 == des_vector )
    {
        return vector_r_sbb(vectorr_md_id, src_vector_2, des_vector);
    }

    if( src_vector_2 == des_vector )
    {
        return vector_r_sbb(vectorr_md_id, src_vector_1, des_vector);
    }

    if( VECTOR_GET_ROTATED_FLAG(src_vector_1) != VECTOR_GET_ROTATED_FLAG(src_vector_2) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sbb: not matchable vector: rotated_flag of src_vector_1 = %ld,rotated_flag of src_vector_2 = %ld\n",
                        VECTOR_GET_ROTATED_FLAG(src_vector_1),
                        VECTOR_GET_ROTATED_FLAG(src_vector_2));
        return ((UINT32)(-1));
    }

    if(  VECTOR_GET_NUM(src_vector_1) != VECTOR_GET_NUM(src_vector_2) )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_sub: not matchable vector: num of src_vector_1 = %ld, num of src_vector_2 = %ld\n",
                        VECTOR_GET_NUM(src_vector_1),
                        VECTOR_GET_NUM(src_vector_1));
        return ((UINT32)(-1));
    }

    rotated_flag = VECTOR_GET_ROTATED_FLAG(src_vector_1);
    num = VECTOR_GET_NUM(src_vector_1);

    if( num != VECTOR_GET_NUM(des_vector) )
    {
        /*clean and re-new skeleton of old des_vector*/
        vector_r_clean_vector(vectorr_md_id, des_vector);
        vector_r_new_vector_skeleton(vectorr_md_id, num, des_vector);
    }

    if( rotated_flag != VECTOR_GET_ROTATED_FLAG(des_vector) )
    {
        VECTOR_SET_ROTATED_FLAG(des_vector, rotated_flag);
    }

    VECTOR_BLOCK_TRIPLE_LOOP_NEXT(src_vector_block_1, src_vector_1,
                                  src_vector_block_2, src_vector_2,
                                  des_vector_block, des_vector)
    {
        vector_r_block_sub(vectorr_md_id, src_vector_block_1, src_vector_block_2, des_vector_block);
    }

    return (0);
}

/**
*
* vector smul self operation
*     vector = s_data * vector
*
**/
static UINT32 vector_r_s_mul_self(const UINT32 vectorr_md_id, const REAL *s_data_addr, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_block_s_mul_self(vectorr_md_id, s_data_addr, vector_block);
    }
    return (0);
}

/**
*
* vector smul operation
*     des_vector = s_data * src_vector
*
**/
UINT32 vector_r_s_mul(const UINT32 vectorr_md_id, const REAL *s_data_addr, const VECTOR *src_vector, VECTOR *des_vector)
{
    VECTOR_BLOCK  *src_vector_block;
    VECTOR_BLOCK  *des_vector_block;

    UINT32 num;

    if( NULL_PTR == s_data_addr )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_s_mul: s_data_addr is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_s_mul: src_vector is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_vector )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_s_mul: des_vector is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_s_mul: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    if( src_vector == des_vector )
    {
        return vector_r_s_mul_self(vectorr_md_id, s_data_addr, des_vector);
    }

    num = VECTOR_GET_NUM(src_vector);
    if( num != VECTOR_GET_NUM(des_vector) )
    {
        /*clean and re-new skeleton of old des_vector*/
        vector_r_clean_vector(vectorr_md_id, des_vector);
        vector_r_new_vector_skeleton(vectorr_md_id, num, des_vector);
    }

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block, src_vector, des_vector_block, des_vector)
    {
        vector_r_block_s_mul(vectorr_md_id, s_data_addr, src_vector_block, des_vector_block);
    }
    return (0);
}


/**
*
* col vector mul row vector operation
*     des_data = src_vector_1 * src_vector_2
*
**/
UINT32 vector_r_row_vector_mul_col_vector(const UINT32 vectorr_md_id, const VECTOR *src_vector_1, const VECTOR *src_vector_2, REAL *des_data_addr)
{
    VECTORR_MD *vectorr_md;
    UINT32 real_md_id;

    VECTOR_BLOCK *src_vector_block_1;
    VECTOR_BLOCK *src_vector_block_2;

    UINT32 sub_idx;
    UINT32 sub_num;

    REAL *src_data_addr_1;
    REAL *src_data_addr_2;
    REAL *tmp_data_addr;

    if( NULL_PTR == src_vector_1 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_row_vector_mul_col_vector: src_vector_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_vector_2 )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_row_vector_mul_col_vector: src_vector_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_data_addr )
    {
        dbg_log(SEC_0098_VECTORR, 0)(LOGSTDERR,"error:vector_r_row_vector_mul_col_vector: des_data_addr is null pointer\n");
        return ((UINT32)(-1));
    }

#if ( SWITCH_ON == VECTOR_DEBUG_SWITCH )
    if ( VECTORR_MD_ID_CHECK_INVALID(vectorr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vector_r_row_vector_mul_col_vector: vectorr module #0x%lx not started.\n",
                vectorr_md_id);
        dbg_exit(MD_VECTORR, vectorr_md_id);
    }
#endif/*VECTOR_DEBUG_SWITCH*/

    vectorr_md = VECTORR_MD_GET(vectorr_md_id);
    real_md_id = vectorr_md->real_md_id;

    alloc_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, &tmp_data_addr, LOC_VECTORR_0023);

    REAL_SETZERO(real_md_id, (*des_data_addr));

    VECTOR_BLOCK_DOUBLE_LOOP_NEXT(src_vector_block_1, src_vector_1,
                                  src_vector_block_2, src_vector_2)
    {
        sub_num = VECTOR_BLOCK_GET_NUM(src_vector_block_1);
        VECTOR_BLOCK_DATA_AREA_INDEX_LOOP_NEXT(sub_idx, sub_num)
        {
            src_data_addr_1 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_1, sub_idx);
            src_data_addr_2 = VECTORR_BLOCK_GET_DATA_ADDR(src_vector_block_2, sub_idx);

            REAL_MUL(real_md_id, (*src_data_addr_1), (*src_data_addr_2), (*tmp_data_addr));
            REAL_ADC(real_md_id, (*tmp_data_addr), (*des_data_addr));
        }
    }

    free_static_mem(MD_VECTORR, vectorr_md_id, MM_REAL, tmp_data_addr, LOC_VECTORR_0024);

    return (0);
}

/* ---------------------------------------- output interface ------------------------------------------------ */

UINT32 vector_r_print_vector_addr_info(const UINT32 vectorr_md_id, VECTOR *vector)
{
    VECTOR_BLOCK  *vector_block;

    sys_print(LOGSTDOUT,"vector: 0x%08lx, rotated_flag = %ld, num = %ld\n",
                        vector,
                        VECTOR_GET_ROTATED_FLAG(vector),
                        VECTOR_GET_NUM(vector));
    sys_print(LOGSTDOUT,"\n");
    sys_print(LOGSTDOUT,"                ");
    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        sys_print(LOGSTDOUT,"(0x%08lx,%ld,0x%08lx) ", vector_block, VECTOR_BLOCK_GET_NUM(vector_block), VECTOR_BLOCK_NEXT(vector_block));
    }
    sys_print(LOGSTDOUT,"\n");
    return (0);
}

UINT32 vector_r_print_block(const UINT32 vectorr_md_id, const VECTOR_BLOCK *vector_block)
{
    UINT32 sub_idx;

    REAL *data_addr;

    for( sub_idx = 0; sub_idx < MATRIX_VECTOR_WIDTH; sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vector_block, sub_idx);

        if( NULL_PTR != data_addr )
        {
                sys_print(LOGSTDOUT,"%+32.14f ", *data_addr);
        }
        else
        {
                sys_print(LOGSTDOUT,"%32s ", "-");
        }
    }
    sys_print(LOGSTDOUT, "\n");

    return (0);
}

static UINT32 vector_r_print_data(const UINT32 vectorr_md_id, const VECTOR *vector)
{
    VECTOR_BLOCK *vector_block;

    sys_print(LOGSTDOUT,"vector: 0x%08lx, rotated_flag = %ld, num = %ld\n",
            vector,
            VECTOR_GET_ROTATED_FLAG(vector),
            VECTOR_GET_NUM(vector)
           );

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vector)
    {
        vector_r_print_block(vectorr_md_id, vector_block);
    }

    return (0);
}

UINT32 vector_r_print_vector_data_info(const UINT32 vectorr_md_id, VECTOR *vector)
{
    vector_r_print_data(vectorr_md_id, vector);
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
