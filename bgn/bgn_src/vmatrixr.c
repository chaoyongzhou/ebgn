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
#include "vmatrixr.h"

#include "cmpic.inc"

#include "debug.h"

#include "print.h"

#include "mod.h"

#include "task.h"

#include "findex.inc"

#include "cbc.h"

#include "cmutex.h"

/********************************************************************************************************
remain issues:
1. if alloc memory at remote, and current taskcomm is broken, how to clean up remote memory?
   one possible solution is record current mod node info at remote

2. how to reduce operations of get from/set to remote memory?

3. how to alloc memory to certain taskcomm? how to make them equally to support large memory requirement?
********************************************************************************************************/

#define VMATRIXR_MD_CAPACITY()                  (cbc_md_capacity(MD_VMATRIXR))

#define VMATRIXR_MD_GET(vmatrixr_md_id)     ((VMATRIXR_MD *)cbc_md_get(MD_VMATRIXR, (vmatrixr_md_id)))

#define VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id)  \
    ( NULL_PTR == VMATRIXR_MD_GET(vmatrixr_md_id) || 0 == (VMATRIXR_MD_GET(vmatrixr_md_id)->usedcounter) )

/**
*   for test only
*
*   to query the status of VMATRIXR Module
*
**/
void vmatrix_r_print_module_status(const UINT32 vmatrixr_md_id, LOG *log)
{
    VMATRIXR_MD *vmatrixr_md;
    UINT32 this_vmatrixr_md_id;

    for( this_vmatrixr_md_id = 0; this_vmatrixr_md_id < VMATRIXR_MD_CAPACITY(); this_vmatrixr_md_id ++ )
    {
        vmatrixr_md = VMATRIXR_MD_GET(this_vmatrixr_md_id);

        if ( NULL_PTR != vmatrixr_md && 0 < vmatrixr_md->usedcounter )
        {
            sys_log(log,"VMATRIXR Module # %ld : %ld refered, refer MATRIXR Module : %ld, refer VMM Module : %ld\n",
                    this_vmatrixr_md_id,
                    vmatrixr_md->usedcounter,
                    vmatrixr_md->matrixr_md_id,
                    vmatrixr_md->vmm_md_id);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed VMATRIXR module
*
*
**/
UINT32 vmatrix_r_free_module_static_mem(const UINT32 vmatrixr_md_id)
{
    VMATRIXR_MD  *vmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_free_module_static_mem: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    free_module_static_mem(MD_VMATRIXR, vmatrixr_md_id);

    return 0;
}

/**
*
* start VMATRIXR module
*
**/
UINT32 vmatrix_r_start( )
{
    VMATRIXR_MD *vmatrixr_md;
    UINT32 vmatrixr_md_id;

    UINT32 matrixr_md_id;
    UINT32        vmm_md_id;

    MOD_MGR  *mod_mgr;
    TASK_BRD *task_brd;
    MOD_NODE *local_mod_node;

    vmatrixr_md_id = cbc_md_new(MD_VMATRIXR, sizeof(VMATRIXR_MD));
    if(ERR_MODULE_ID == vmatrixr_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one VMATRIXR module */
    vmatrixr_md = (VMATRIXR_MD *)cbc_md_get(MD_VMATRIXR, vmatrixr_md_id);
    vmatrixr_md->usedcounter   = 0;
    vmatrixr_md->matrixr_md_id = ERR_MODULE_ID;
    vmatrixr_md->vmm_md_id     = ERR_MODULE_ID;

    /* create a new module node */
    matrixr_md_id = ERR_MODULE_ID;
    vmm_md_id     = ERR_MODULE_ID;

    init_static_mem();

    /*default setting which will be override after vmatrix_r_set_mod_mgr calling*/
    mod_mgr = mod_mgr_new(vmatrixr_md_id, LOAD_BALANCING_LOOP);

    task_brd = task_brd_default_get();

    local_mod_node = &(vmatrixr_md->local_mod_node);
    MOD_NODE_TCID(local_mod_node) = TASK_BRD_TCID(task_brd);
    MOD_NODE_COMM(local_mod_node) = TASK_BRD_COMM(task_brd);
    MOD_NODE_RANK(local_mod_node) = TASK_BRD_RANK(task_brd);
    MOD_NODE_MODI(local_mod_node) = vmatrixr_md_id;

    matrixr_md_id = matrix_r_start();
    vmm_md_id     = vmm_start();

    vmatrixr_md->mod_mgr       = mod_mgr;
    vmatrixr_md->task_brd      = task_brd;
    vmatrixr_md->matrixr_md_id = matrixr_md_id;
    vmatrixr_md->vmm_md_id     = vmm_md_id;
    vmatrixr_md->usedcounter   = 1;

    return ( vmatrixr_md_id );
}

/**
*
* end VMATRIXR module
*
**/
void vmatrix_r_end(const UINT32 vmatrixr_md_id)
{
    VMATRIXR_MD *vmatrixr_md;

    UINT32 matrixr_md_id;
    UINT32     vmm_md_id;
    MOD_NODE *local_mod_node;

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);
    if(NULL_PTR == vmatrixr_md)
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDOUT,"error:vmatrix_r_end: vmatrixr_md_id = %ld is overflow.\n", vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < vmatrixr_md->usedcounter )
    {
        vmatrixr_md->usedcounter --;
        return ;
    }

    if ( 0 == vmatrixr_md->usedcounter )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDOUT,"error:vmatrix_r_end: vmatrixr_md_id = %ld is not started.\n", vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }

    //task_brd_mod_mgr_rmv(vmatrixr_md->task_brd, vmatrixr_md->mod_mgr);
    mod_mgr_free(vmatrixr_md->mod_mgr);
    vmatrixr_md->mod_mgr  = NULL_PTR;

    vmatrixr_md->task_brd = NULL_PTR;

    local_mod_node = &(vmatrixr_md->local_mod_node);
    MOD_NODE_TCID(local_mod_node) = CMPI_ERROR_TCID;
    MOD_NODE_COMM(local_mod_node) = CMPI_ERROR_COMM;
    MOD_NODE_RANK(local_mod_node) = CMPI_ERROR_RANK;
    MOD_NODE_MODI(local_mod_node) = CMPI_ERROR_MODI;

    /* if nobody else occupied the module,then free its resource */
    matrixr_md_id = vmatrixr_md->matrixr_md_id;
    vmm_md_id     = vmatrixr_md->vmm_md_id;

    matrix_r_end(matrixr_md_id);
    vmm_end(vmm_md_id);

    vmatrixr_md->matrixr_md_id = ERR_MODULE_ID;
    vmatrixr_md->vmm_md_id     = ERR_MODULE_ID;
    vmatrixr_md->usedcounter   = 0;

    cbc_md_free(MD_VMATRIXR, vmatrixr_md_id);

    breathing_static_mem();
    return ;
}


/**
*
* initialize mod mgr of VMATRIXR module
*
**/
UINT32 vmatrix_r_set_mod_mgr(const UINT32 vmatrixr_md_id, const MOD_MGR * src_mod_mgr)
{
    VMATRIXR_MD *vmatrixr_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_set_mod_mgr: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        vmatrix_r_print_module_status(vmatrixr_md_id, LOGSTDOUT);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);
    des_mod_mgr = vmatrixr_md->mod_mgr;

    //dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_set_mod_mgr: md_id %d, src_mod_mgr %lx\n", vmatrixr_md_id, src_mod_mgr);
    //mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    mod_mgr_limited_clone(vmatrixr_md_id, src_mod_mgr, des_mod_mgr);

    //dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_set_mod_mgr: md_id %d, des_mod_mgr %lx\n", vmatrixr_md_id, des_mod_mgr);
    //mod_mgr_print(LOGSTDOUT, des_mod_mgr);

    dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "====================================vmatrix_r_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "====================================vmatrix_r_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);


    return (0);
}

/**
*
* get mod mgr of MATRIXR module
*
**/
MOD_MGR * vmatrix_r_get_mod_mgr(const UINT32 vmatrixr_md_id)
{
    VMATRIXR_MD *vmatrixr_md;

    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        return (MOD_MGR *)0;
    }

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);
    return (vmatrixr_md->mod_mgr);
}

static EC_BOOL vmatrix_r_is_local_mod_node(const UINT32 vmatrixr_md_id, const MOD_NODE *mod_node)
{
    VMATRIXR_MD *vmatrixr_md;
    MOD_NODE *local_mod_node;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_is_local_mod_node: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);
    local_mod_node = &(vmatrixr_md->local_mod_node);

    if(MOD_NODE_TCID(mod_node) != MOD_NODE_TCID(local_mod_node))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_COMM(mod_node) != MOD_NODE_COMM(local_mod_node))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_RANK(mod_node) != MOD_NODE_RANK(local_mod_node))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_MODI(mod_node) != MOD_NODE_MODI(local_mod_node))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

static EC_BOOL vmatrix_r_is_same_mod_node(const UINT32 vmatrixr_md_id, const MOD_NODE *mod_node_1, const MOD_NODE *mod_node_2)
{
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_is_same_mod_node: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(MOD_NODE_TCID(mod_node_1) != MOD_NODE_TCID(mod_node_2))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_COMM(mod_node_1) != MOD_NODE_COMM(mod_node_2))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_RANK(mod_node_1) != MOD_NODE_RANK(mod_node_2))
    {
        return (EC_FALSE);
    }

    if(MOD_NODE_MODI(mod_node_1) != MOD_NODE_MODI(mod_node_2))
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

VMM_NODE * vmatrix_r_alloc_vmm_node(const UINT32 vmatrixr_md_id)
{
    VMM_NODE *vmm_node;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_alloc_vmm_node: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    alloc_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, &vmm_node, LOC_VMATRIXR_0001);
    return (vmm_node);
}

EC_BOOL vmatrix_r_free_vmm_node(const UINT32 vmatrixr_md_id, VMM_NODE *vmm_node)
{
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_free_vmm_node: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if(0 != VMM_NODE_ADDR(vmm_node))
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDOUT, "error:vmatrix_r_free_vmm_node: vmm_node %lx mount pointer %lx\n", vmm_node, VMM_NODE_ADDR(vmm_node));
    }
    free_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, vmm_node, LOC_VMATRIXR_0002);
    return (EC_TRUE);
}


UINT32 vmatrix_r_alloc_block(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_block_vmm)
{
    VMATRIXR_MD *vmatrixr_md;

    MATRIX_BLOCK *matrix_block;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_alloc_block: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    alloc_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX_BLOCK, &matrix_block, LOC_VMATRIXR_0003);
    matrix_r_init_block(vmatrixr_md->matrixr_md_id, matrix_block);

    /*bind key and value, i.e., (vmm_node, matrix_block)*/
    task_brd = vmatrixr_md->task_brd;
    VMM_NODE_TCID(matrix_block_vmm) = TASK_BRD_TCID(task_brd);
    VMM_NODE_COMM(matrix_block_vmm) = TASK_BRD_COMM(task_brd);
    VMM_NODE_RANK(matrix_block_vmm) = TASK_BRD_RANK(task_brd);
    VMM_NODE_MODI(matrix_block_vmm) = vmatrixr_md_id;
    VMM_NODE_LOAD(matrix_block_vmm) = task_brd_rank_load_tbl_get_que(task_brd, TASK_BRD_TCID(task_brd), TASK_BRD_RANK(task_brd));
    VMM_NODE_ADDR(matrix_block_vmm) = (UINT32)matrix_block;

    return (0);
}

UINT32 vmatrix_r_free_block(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_block_vmm)
{
    VMATRIXR_MD *vmatrixr_md;

    MATRIX_BLOCK *matrix_block;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_free_block: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    if(0 != VMM_NODE_ADDR(matrix_block_vmm))
    {
        matrix_block = (MATRIX_BLOCK *)VMM_NODE_ADDR(matrix_block_vmm);
        dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_free_block: free matrix_block %lx\n", matrix_block);

        matrix_r_clean_block(vmatrixr_md->matrixr_md_id, matrix_block);

        free_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX_BLOCK, matrix_block, LOC_VMATRIXR_0004);
    }
    else
    {
        dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_free_block: matrix_block is null\n");
    }

    task_brd = vmatrixr_md->task_brd;
    VMM_NODE_LOAD(matrix_block_vmm) = task_brd_rank_load_tbl_get_que(task_brd, TASK_BRD_TCID(task_brd), TASK_BRD_RANK(task_brd));
    VMM_NODE_ADDR(matrix_block_vmm) = 0;

    return (0);
}

static UINT32 vmatrix_r_alloc_matrix(const UINT32 vmatrixr_md_id, MATRIX **ppmatrix)
{
    VMATRIXR_MD *vmatrixr_md;
    MATRIX *matrix;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_alloc_matrix: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    alloc_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX, &matrix, LOC_VMATRIXR_0005);

    matrix_r_init_matrix(vmatrixr_md->matrixr_md_id, matrix);

    (*ppmatrix) = matrix;

    return (0);
}

UINT32 vmatrix_r_block_set_row_num(const UINT32 vmatrixr_md_id, const UINT32 row_num, VMM_NODE *matrix_block_vmm)
{
    MATRIX_BLOCK *matrix_block;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_block_set_row_num: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrix_block = (MATRIX_BLOCK *)VMM_NODE_ADDR(matrix_block_vmm);
    MATRIX_BLOCK_SET_ROW_NUM(matrix_block, row_num);

    return (0);
}

UINT32 vmatrix_r_block_set_col_num(const UINT32 vmatrixr_md_id, const UINT32 col_num, VMM_NODE *matrix_block_vmm)
{
    MATRIX_BLOCK *matrix_block;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_block_set_col_num: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    matrix_block = (MATRIX_BLOCK *)VMM_NODE_ADDR(matrix_block_vmm);
    MATRIX_BLOCK_SET_COL_NUM(matrix_block, col_num);

    return (0);
}

UINT32 vmatrix_r_new_matrix_skeleton(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, MATRIX *matrix)
{
    VMATRIXR_MD *vmatrixr_md;

    UINT32 blocks_row_num;
    UINT32 blocks_col_num;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 sub_row_num;
    UINT32 sub_col_num;

    //VMATRIX_BLOCK  *vmatrix_block;
    CVECTOR *blocks;

    TASK_MGR *task_mgr;
    MOD_NODE *send_mod_node;

    UINT32 ret;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_new_matrix_skeleton: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( NULL_PTR == matrix )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_new_matrix_skeleton: matrix is null pointer\n");
        return ((UINT32)(-1));
    }

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    send_mod_node = &(vmatrixr_md->local_mod_node);

    /*set rotated_flag to default 0*/
    MATRIX_SET_ROTATED_FLAG(matrix, 0);

    /*set row num and col num to matrix. not sure if it is suitable or not here*/
    MATRIX_SET_ROW_NUM(matrix, row_num);
    MATRIX_SET_COL_NUM(matrix, col_num);

    blocks_row_num = MATRIX_GET_ROW_BLOCKS_NUM(matrix);
    blocks_col_num = MATRIX_GET_COL_BLOCKS_NUM(matrix);

    /*create matrix header*/
    blocks = cvector_new(blocks_row_num * blocks_col_num, MM_VMM_NODE, LOC_VMATRIXR_0006);
    MATRIX_SET_BLOCKS(matrix, blocks);

    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    VMATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        VMM_NODE *matrix_block_vmm;

        //vmm_alloc(vmatrixr_md->vmm_md_id, MD_VMATRIXR, MOD_NODE_MODI(recv_mod_node), MM_VMM_NODE, &vmm_node);
        alloc_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, &matrix_block_vmm, LOC_VMATRIXR_0007);
        cvector_push((CVECTOR *)blocks, (void *)matrix_block_vmm);

        task_inc(task_mgr, &ret, FI_vmatrix_r_alloc_block, ERR_MODULE_ID, matrix_block_vmm);/*alloc matrix block from virtual memory*/
    }
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*adjust row num of last row header and its blocks if necessary*/
    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    sub_row_num = (row_num % MATRIX_VECTOR_WIDTH);
    if( 0 < sub_row_num )
    {
        block_row_idx = (blocks_row_num - 1);
        VMATRIX_BLOCK_COL_LOOP_NEXT(blocks_col_num, block_col_idx)
        {
            VMM_NODE *matrix_block_vmm;
            MOD_NODE *recv_mod_node;
            UINT32 ret;

            matrix_block_vmm = (VMM_NODE *)MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
            recv_mod_node = VMM_NODE_MOD_NODE(matrix_block_vmm);

            //vmatrix_r_block_set_row_num(vmatrixr_md_id, sub_row_num, vmm_node);
            task_super_inc(task_mgr, send_mod_node, recv_mod_node, &ret, FI_vmatrix_r_block_set_row_num, VMM_NODE_MODI(matrix_block_vmm), sub_row_num, matrix_block_vmm);
        }
    }

    /*adjust col num of last col header and its blocks if necessary*/
    sub_col_num = (col_num % MATRIX_VECTOR_WIDTH);
    if( 0 < sub_col_num )
    {
        block_col_idx = (blocks_col_num - 1);
        VMATRIX_BLOCK_ROW_LOOP_NEXT(blocks_row_num, block_row_idx)
        {
            VMM_NODE *matrix_block_vmm;
            MOD_NODE *recv_mod_node;

            matrix_block_vmm = (VMM_NODE *)MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);
            recv_mod_node = VMM_NODE_MOD_NODE(matrix_block_vmm);

            //vmatrix_r_block_set_col_num(vmatrixr_md_id, sub_col_num, vmm_node);
            task_super_inc(task_mgr, send_mod_node, recv_mod_node, &ret, FI_vmatrix_r_block_set_col_num, VMM_NODE_MODI(matrix_block_vmm), sub_col_num, matrix_block_vmm);
        }
    }
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    return (0);
}

UINT32 vmatrix_r_new_matrix(const UINT32 vmatrixr_md_id, const UINT32 row_num, const UINT32 col_num, VMM_NODE *matrix_vmm)
{
    VMATRIXR_MD *vmatrixr_md;
    MATRIX *matrix;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_new_matrix: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( NULL_PTR == matrix_vmm )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_new_matrix: matrix_vmm is null pointer\n");
        return ((UINT32)(-1));
    }

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    /*create matrix node itself*/
    vmatrix_r_alloc_matrix(vmatrixr_md_id, &matrix);

    /*create matrix skeleton*/
    vmatrix_r_new_matrix_skeleton(vmatrixr_md_id, row_num, col_num, matrix);

    /*bind (vmm_node, matrix)*/
    task_brd = vmatrixr_md->task_brd;
    VMM_NODE_TCID(matrix_vmm) = TASK_BRD_TCID(task_brd);
    VMM_NODE_COMM(matrix_vmm) = TASK_BRD_COMM(task_brd);
    VMM_NODE_RANK(matrix_vmm) = TASK_BRD_RANK(task_brd);
    VMM_NODE_MODI(matrix_vmm) = vmatrixr_md_id;
    VMM_NODE_LOAD(matrix_vmm) = task_brd_rank_load_tbl_get_que(task_brd, VMM_NODE_TCID(matrix_vmm), VMM_NODE_RANK(matrix_vmm));
    VMM_NODE_ADDR(matrix_vmm) = (UINT32)matrix;

    return (0);
}

UINT32 vmatrix_r_get_block(const UINT32 vmatrixr_md_id, const VMM_NODE *vmm_node, MATRIX_BLOCK *matrix_block)
{
    VMATRIXR_MD *vmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_get_block: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    matrix_r_clone_block(vmatrixr_md->matrixr_md_id, (MATRIX_BLOCK *)VMM_NODE_ADDR(vmm_node), matrix_block);

    return (0);
}

UINT32 vmatrix_r_set_block(const UINT32 vmatrixr_md_id, const MATRIX_BLOCK *matrix_block, VMM_NODE *vmm_node)
{
    VMATRIXR_MD *vmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_set_block: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    matrix_r_clone_block(vmatrixr_md->matrixr_md_id, matrix_block, (MATRIX_BLOCK *)VMM_NODE_ADDR(vmm_node));

    return (0);
}

/*get matrix block from virtual memory*/
static UINT32 vmatrix_r_vget_block(const UINT32 vmatrixr_md_id, const VMM_NODE *matrix_block_vmm, MATRIX_BLOCK **ppmatrix_block, UINT32 *ret, TASK_MGR *task_mgr)
{
    VMATRIXR_MD *vmatrixr_md;
    MOD_NODE *send_mod_node;
    MATRIX_BLOCK *matrix_block;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_vget_block: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    if(EC_FALSE == vmatrix_r_is_local_mod_node(vmatrixr_md_id, VMM_NODE_MOD_NODE(matrix_block_vmm)))
    {
        send_mod_node = &(vmatrixr_md->local_mod_node);
        matrix_r_new_block(vmatrixr_md->matrixr_md_id, &matrix_block);
        task_super_inc(task_mgr, send_mod_node, VMM_NODE_MOD_NODE(matrix_block_vmm), ret, FI_vmatrix_r_get_block, ERR_MODULE_ID, matrix_block_vmm, matrix_block);
        (*ppmatrix_block) = matrix_block;
    }
    else
    {
        (*ppmatrix_block) = (MATRIX_BLOCK *)VMM_NODE_ADDR(matrix_block_vmm);
    }

    return (0);
}

/*set matrix block to virtual memory*/
static UINT32 vmatrix_r_vset_block(const UINT32 vmatrixr_md_id, const VMM_NODE *matrix_block_vmm, MATRIX_BLOCK *matrix_block, UINT32 *ret, TASK_MGR *task_mgr)
{
    VMATRIXR_MD *vmatrixr_md;
    MOD_NODE *send_mod_node;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_vset_block: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    if(EC_FALSE == vmatrix_r_is_local_mod_node(vmatrixr_md_id, VMM_NODE_MOD_NODE(matrix_block_vmm)))
    {
        send_mod_node = &(vmatrixr_md->local_mod_node);

        task_super_inc(task_mgr, send_mod_node, VMM_NODE_MOD_NODE(matrix_block_vmm), ret, FI_vmatrix_r_set_block, ERR_MODULE_ID, matrix_block, matrix_block_vmm);
    }
    return (0);
}

/*return matrix block used for virtual memory mirror*/
static UINT32 vmatrix_r_vback_block(const UINT32 vmatrixr_md_id, const VMM_NODE *matrix_block_vmm, MATRIX_BLOCK *matrix_block)
{
    VMATRIXR_MD *vmatrixr_md;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_vback_block: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    if(EC_FALSE == vmatrix_r_is_local_mod_node(vmatrixr_md_id, VMM_NODE_MOD_NODE(matrix_block_vmm)))
    {
        matrix_r_destroy_block(vmatrixr_md->matrixr_md_id, matrix_block);
    }

    return (0);
}

UINT32 vmatrix_r_block_mul(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_row_block_vmm_1, const VMM_NODE *src_matrix_row_block_vmm_2, VMM_NODE *des_matrix_row_block_vmm)
{
    VMATRIXR_MD *vmatrixr_md;

    TASK_MGR *task_mgr;

    MATRIX_BLOCK *src_matrix_row_block_1;
    MATRIX_BLOCK *src_matrix_row_block_2;
    MATRIX_BLOCK *des_matrix_row_block;

    MOD_NODE *send_mod_node;
    UINT32 ret;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_block_mul: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    send_mod_node = &(vmatrixr_md->local_mod_node);

    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    vmatrix_r_vget_block(vmatrixr_md_id, src_matrix_row_block_vmm_1, &src_matrix_row_block_1, &ret, task_mgr);
    vmatrix_r_vget_block(vmatrixr_md_id, src_matrix_row_block_vmm_2, &src_matrix_row_block_2, &ret, task_mgr);
    vmatrix_r_vget_block(vmatrixr_md_id, des_matrix_row_block_vmm  , &des_matrix_row_block  , &ret, task_mgr);
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    matrix_r_block_mul(vmatrixr_md->matrixr_md_id, src_matrix_row_block_1, src_matrix_row_block_2, des_matrix_row_block);

    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    vmatrix_r_vset_block(vmatrixr_md_id, des_matrix_row_block_vmm, des_matrix_row_block, &ret, task_mgr);
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    vmatrix_r_vback_block(vmatrixr_md_id, src_matrix_row_block_vmm_1, src_matrix_row_block_1);
    vmatrix_r_vback_block(vmatrixr_md_id, src_matrix_row_block_vmm_2, src_matrix_row_block_2);
    vmatrix_r_vback_block(vmatrixr_md_id, des_matrix_row_block_vmm  , des_matrix_row_block  );

    return (0);
}

UINT32 vmatrix_r_block_adc(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_row_block_vmm, VMM_NODE *des_matrix_row_block_vmm)
{
    VMATRIXR_MD *vmatrixr_md;

    TASK_MGR *task_mgr;

    MATRIX_BLOCK *src_matrix_row_block;
    MATRIX_BLOCK *des_matrix_row_block;

    UINT32 ret;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_block_adc: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    vmatrix_r_vget_block(vmatrixr_md_id, src_matrix_row_block_vmm, &src_matrix_row_block, &ret, task_mgr);
    vmatrix_r_vget_block(vmatrixr_md_id, des_matrix_row_block_vmm, &des_matrix_row_block, &ret, task_mgr);
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    matrix_r_block_adc(vmatrixr_md->matrixr_md_id, src_matrix_row_block, des_matrix_row_block);

    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    vmatrix_r_vset_block(vmatrixr_md_id, des_matrix_row_block_vmm, des_matrix_row_block, &ret, task_mgr);
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    vmatrix_r_vback_block(vmatrixr_md_id, src_matrix_row_block_vmm, src_matrix_row_block);
    vmatrix_r_vback_block(vmatrixr_md_id, des_matrix_row_block_vmm, des_matrix_row_block);

    return (0);
}

UINT32 vmatrix_r_clean_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm)
{
    VMATRIXR_MD *vmatrixr_md;

    TASK_MGR *task_mgr;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX *matrix;

    MOD_NODE *send_mod_node;
    UINT32 ret;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_clean_matrix: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( NULL_PTR == matrix_vmm )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_clean_matrix: matrix_vmm is null pointer\n");
        return ((UINT32)(-1));
    }

    if(0 == VMM_NODE_ADDR(matrix_vmm))
    {
        return (0);
    }

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    send_mod_node = &(vmatrixr_md->local_mod_node);

    /*warning: pls make sure vmm_node of matrix is stay along with matrix, i.e., in the same physical memory*/
    matrix = (MATRIX *)VMM_NODE_ADDR(matrix_vmm);

    /*destroy remote blocks*/
    task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        VMM_NODE *matrix_block_vmm;

        matrix_block_vmm = (VMM_NODE *)MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);

        if(0 == VMM_NODE_ADDR(matrix_block_vmm))
        {
            dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_clean_matrix: matrix_block_vmm %lx mount null\n", matrix_block_vmm);
            continue;
        }

        task_super_inc(task_mgr, send_mod_node, VMM_NODE_MOD_NODE(matrix_block_vmm), &ret, FI_vmatrix_r_free_block, ERR_MODULE_ID, matrix_block_vmm);
    }
    task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*destroy local vmm_node for blocks*/
    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrix, block_row_idx, block_col_idx)
    {
        VMM_NODE *matrix_block_vmm;

        matrix_block_vmm = (VMM_NODE *)MATRIX_GET_BLOCK(matrix, block_row_idx, block_col_idx);

        free_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, matrix_block_vmm, LOC_VMATRIXR_0008);
        MATRIX_SET_BLOCK(matrix, block_row_idx, block_col_idx, NULL_PTR);/*clean pointer*/
    }

    /*destroy matrix header */
    if ( 0 != MATRIX_GET_BLOCKS(matrix))
    {
        cvector_free((CVECTOR *)MATRIX_GET_BLOCKS(matrix), LOC_VMATRIXR_0009);
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
* move src_matrix matrix, skeleton and data area to des_matrix
* note:
*     des_matrix must have no skeleton or data area. otherwise, skeleton and data area will be lost without free
*
**/
UINT32 vmatrix_r_move_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *src_matrix_vmm, VMM_NODE *des_matrix_vmm)
{
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_move_matrix: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    VMM_NODE_TCID(des_matrix_vmm) = VMM_NODE_TCID(src_matrix_vmm);
    VMM_NODE_COMM(des_matrix_vmm) = VMM_NODE_COMM(src_matrix_vmm);
    VMM_NODE_RANK(des_matrix_vmm) = VMM_NODE_RANK(src_matrix_vmm);
    VMM_NODE_MODI(des_matrix_vmm) = VMM_NODE_MODI(src_matrix_vmm);
    VMM_NODE_ADDR(des_matrix_vmm) = VMM_NODE_ADDR(src_matrix_vmm);

    VMM_NODE_TCID(src_matrix_vmm) = CMPI_ERROR_TCID;
    VMM_NODE_COMM(src_matrix_vmm) = CMPI_ERROR_COMM;
    VMM_NODE_RANK(src_matrix_vmm) = CMPI_ERROR_RANK;
    VMM_NODE_MODI(src_matrix_vmm) = CMPI_ERROR_MODI;
    VMM_NODE_ADDR(src_matrix_vmm) = 0;

    return (0);
}

UINT32 vmatrix_r_free_matrix( const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm)
{
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_free_matrix: matrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( 0 != VMM_NODE_ADDR(matrix_vmm))
    {
        MATRIX *matrix;

        matrix = (MATRIX *)VMM_NODE_ADDR(matrix_vmm);
        free_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX  , matrix    , LOC_VMATRIXR_0010);
        VMM_NODE_ADDR(matrix_vmm) = 0;
    }

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
UINT32 vmatrix_r_destroy_matrix(const UINT32 vmatrixr_md_id, VMM_NODE *matrix_vmm)
{
#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_destroy_matrix: vmatrixr module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_VMATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( NULL_PTR == matrix_vmm )
    {
        return (0);
    }

    vmatrix_r_clean_matrix(vmatrixr_md_id, matrix_vmm);

    /*destroy matrix node itself*/
    vmatrix_r_free_matrix(vmatrixr_md_id, matrix_vmm);

    return (0);
}


/**
*
* matrix mul operation
*     des_matrix = src_matrix_1 * src_matrix_2
*
**/
UINT32 vmatrix_r_mul_p(const UINT32 vmatrixr_md_id, const VMM_NODE *src_matrix_vmm_1, const VMM_NODE *src_matrix_vmm_2, VMM_NODE *des_matrix_vmm)
{
    VMATRIXR_MD  *vmatrixr_md;

    MATRIX *src_matrix_1;
    MATRIX *src_matrix_2;
    MATRIX *des_matrix;

    UINT32 block_row_idx;
    UINT32 block_col_idx;

    UINT32 block_row_idx_cur;
    UINT32 block_col_idx_cur;

    UINT32 block_row_num_cur;
    UINT32 block_col_num_cur;

    VMM_NODE *tmp_matrix_vmm;
    MATRIX *tmp_matrix;

    UINT32 row_num;
    UINT32 col_num;

    CLIST *clist;
    UINT32 ret;
    MOD_NODE *send_mod_node;

#if ( SWITCH_ON == MATRIX_DEBUG_SWITCH )
    if ( VMATRIXR_MD_ID_CHECK_INVALID(vmatrixr_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmatrix_r_mul_p: module #0x%lx not started.\n",
                vmatrixr_md_id);
        dbg_exit(MD_MATRIXR, vmatrixr_md_id);
    }
#endif/*MATRIX_DEBUG_SWITCH*/

    if( NULL_PTR == src_matrix_vmm_1 )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_mul_p: src_matrix_vmm_1 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == src_matrix_vmm_2 )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_mul_p: src_matrix_vmm_2 is null pointer\n");
        return ((UINT32)(-1));
    }

    if( NULL_PTR == des_matrix_vmm)
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_mul_p: des_matrix_vmm is null pointer\n");
        return ((UINT32)(-1));
    }

    vmatrixr_md = VMATRIXR_MD_GET(vmatrixr_md_id);

    send_mod_node = &(vmatrixr_md->local_mod_node);

    src_matrix_1 = (MATRIX *)VMM_NODE_ADDR(src_matrix_vmm_1);
    src_matrix_2 = (MATRIX *)VMM_NODE_ADDR(src_matrix_vmm_2);
    des_matrix   = (MATRIX *)VMM_NODE_ADDR(des_matrix_vmm  );

    if(  MATRIX_GET_COL_NUM(src_matrix_1)
      != MATRIX_GET_ROW_NUM(src_matrix_2) )
    {
        dbg_log(SEC_0092_VMATRIXR, 0)(LOGSTDERR,"error:vmatrix_r_mul_p: not matchable matrix: col num of src_matrix_1 = %ld, row num of src_matrix_2 = %ld\n",
                        MATRIX_GET_COL_NUM(src_matrix_1),
                        MATRIX_GET_ROW_NUM(src_matrix_2));
        return ((UINT32)(-1));
    }

    row_num = MATRIX_GET_ROW_NUM(src_matrix_1);
    col_num = MATRIX_GET_COL_NUM(src_matrix_2);

    /*must alloc local vmm_node for matrix*/
    alloc_static_mem(MD_MATRIXR, vmatrixr_md_id, MM_VMM_NODE, &tmp_matrix_vmm, LOC_VMATRIXR_0011);
    vmatrix_r_new_matrix(vmatrixr_md_id, row_num, col_num, tmp_matrix_vmm);
    tmp_matrix = (MATRIX *)VMM_NODE_ADDR(tmp_matrix_vmm);

    block_col_num_cur = VMATRIX_GET_COL_BLOCKS_NUM(src_matrix_1);
    block_row_num_cur = VMATRIX_GET_ROW_BLOCKS_NUM(src_matrix_2);

    clist = clist_new(MM_IGNORE, LOC_VMATRIXR_0012);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(tmp_matrix, block_row_idx, block_col_idx)
    {
        VMM_NODE  *tmp_matrix_block_vmm;

        TASK_MGR *task_mgr;

        tmp_matrix_block_vmm = (VMM_NODE *)MATRIX_GET_BLOCK(tmp_matrix, block_row_idx, block_col_idx);

        task_mgr = task_new(vmatrixr_md->mod_mgr, TASK_PRIO_NORMAL, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);
        for(block_col_idx_cur = 0, block_row_idx_cur = 0;
            block_col_idx_cur < block_col_num_cur && block_row_idx_cur < block_row_num_cur;
            block_col_idx_cur ++, block_row_idx_cur ++)
        {
            VMM_NODE  *src_matrix_block_vmm_1;
            VMM_NODE  *src_matrix_block_vmm_2;
            VMM_NODE  *pro_matrix_block_vmm;

            src_matrix_block_vmm_1 = (VMM_NODE  *)MATRIX_GET_BLOCK(src_matrix_1, block_row_idx    , block_col_idx_cur);
            src_matrix_block_vmm_2 = (VMM_NODE  *)MATRIX_GET_BLOCK(src_matrix_2, block_row_idx_cur, block_col_idx    );

            alloc_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, &pro_matrix_block_vmm, LOC_VMATRIXR_0013);
            vmm_alloc(vmatrixr_md->vmm_md_id, MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX_BLOCK, pro_matrix_block_vmm);
            matrix_r_init_block(vmatrixr_md->matrixr_md_id, (MATRIX_BLOCK *)VMM_NODE_ADDR(pro_matrix_block_vmm));/*must initialized the block*/

            clist_push_back(clist, pro_matrix_block_vmm);
#if 0
            dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_mul_p: beg:\n");
            vmm_print(vmatrixr_md->vmm_md_id, LOGSTDOUT, src_matrix_block_vmm_1);
            vmm_print(vmatrixr_md->vmm_md_id, LOGSTDOUT, src_matrix_block_vmm_2);
            vmm_print(vmatrixr_md->vmm_md_id, LOGSTDOUT, pro_matrix_block_vmm);
            dbg_log(SEC_0092_VMATRIXR, 5)(LOGSTDOUT, "vmatrix_r_mul_p: end:\n");
#endif
            //task_inc(task_mgr, &ret, FI_vmatrix_r_block_mul, ERR_MODULE_ID, src_matrix_block_vmm_1, src_matrix_block_vmm_2, pro_matrix_block_vmm);
            /*if send the op=block_mul to one module which own one matrix_block, we have 50% possibility to reduce op=get_block to zero! */
            task_super_inc(task_mgr, send_mod_node, VMM_NODE_MOD_NODE(src_matrix_block_vmm_1), &ret, FI_vmatrix_r_block_mul, ERR_MODULE_ID, src_matrix_block_vmm_1, src_matrix_block_vmm_2, pro_matrix_block_vmm);
        }
        task_wait(task_mgr, TASK_ALWAYS_LIVE, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

        while(EC_FALSE == clist_is_empty(clist))
        {
            VMM_NODE  *pro_matrix_block_vmm;

            pro_matrix_block_vmm = (VMM_NODE  *)clist_pop_back(clist);
            vmatrix_r_block_adc(vmatrixr_md_id, pro_matrix_block_vmm, tmp_matrix_block_vmm);

            vmm_free(vmatrixr_md->vmm_md_id, MD_VMATRIXR, vmatrixr_md_id, MM_MATRIX_BLOCK, pro_matrix_block_vmm);
            free_static_mem(MD_MATRIXR, vmatrixr_md_id, MM_VMM_NODE, pro_matrix_block_vmm, LOC_VMATRIXR_0014);
        }
    }
    clist_free(clist, LOC_VMATRIXR_0015);

    vmatrix_r_destroy_matrix(vmatrixr_md_id, des_matrix_vmm);/*dismiss matrix and vmm_node, then destory matrix*/
    vmatrix_r_move_matrix(vmatrixr_md_id, tmp_matrix_vmm, des_matrix_vmm);

    vmatrix_r_free_matrix(vmatrixr_md_id, tmp_matrix_vmm);
    free_static_mem(MD_VMATRIXR, vmatrixr_md_id, MM_VMM_NODE, tmp_matrix_vmm, LOC_VMATRIXR_0016);

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

