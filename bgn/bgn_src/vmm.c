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
#include <stdarg.h>
#include <unistd.h>

#include "type.h"

#include "mm.h"
#include "log.h"
#include "debug.h"

#include "cmisc.h"

#include "clist.h"
#include "cvector.h"

#include "cbc.h"
#include "rank.h"
#include "task.inc"
#include "task.h"
#include "tasks.h"

#include "vmm.h"

#include "cthread.h"

#include "findex.inc"

#define VMM_MD_CAPACITY()                  (cbc_md_capacity(MD_VMM))

#define VMM_MD_GET(vmm_md_id)     ((VMM_MD *)cbc_md_get(MD_VMM, (vmm_md_id)))

#define UINT32_CHECK_INVALID(vmm_md_id)  \
    ((CMPI_ANY_MODI != (vmm_md_id)) && ((NULL_PTR == VMM_MD_GET(vmm_md_id)) || (0 == (VMM_MD_GET(vmm_md_id)->usedcounter))))

/**
*   for test only
*
*   to query the status of VMM Module
*
**/
void vmm_print_module_status(const UINT32 vmm_md_id, LOG *log)
{
    VMM_MD *vmm_md;
    UINT32 this_vmm_md_id;

    for( this_vmm_md_id = 0; this_vmm_md_id < VMM_MD_CAPACITY(); this_vmm_md_id ++ )
    {
        vmm_md = VMM_MD_GET(this_vmm_md_id);

        if ( NULL_PTR != vmm_md && 0 < vmm_md->usedcounter )
        {
            sys_log(log,"VMM Module # %ld : %ld refered\n",
                    this_vmm_md_id,
                    vmm_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed VMM module
*
*
**/
UINT32 vmm_free_module_static_mem(const UINT32 vmm_md_id)
{
    VMM_MD  *vmm_md;

#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_free_module_static_mem: vmm module #0x%lx not started.\n",
                vmm_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)(-1));
    }
#endif/*VMM_DEBUG_SWITCH*/

    vmm_md = VMM_MD_GET(vmm_md_id);

    free_module_static_mem(MD_VMM, vmm_md_id);

    return 0;
}

/**
*
* start VMM module
*
**/
UINT32 vmm_start( )
{
    VMM_MD *vmm_md;
    UINT32 vmm_md_id;

    vmm_md_id = cbc_md_new(MD_VMM, sizeof(VMM_MD));
    if(ERR_MODULE_ID == vmm_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one VMM module */
    vmm_md = (VMM_MD *)cbc_md_get(MD_VMM, vmm_md_id);
    vmm_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    /*default setting which will be override after vmm_set_mod_mgr calling*/
    vmm_md->mod_mgr = mod_mgr_new(vmm_md_id, LOAD_BALANCING_LOOP);

    vmm_md->usedcounter = 1;

    dbg_log(SEC_0003_VMM, 5)(LOGSTDOUT, "vmm_start: start VMM module #%ld\n", vmm_md_id);
    //dbg_log(SEC_0003_VMM, 3)(LOGSTDOUT, "========================= vmm_start: VMM_ table info:\n");
    //vmm_print_module_status(vmm_md_id, LOGSTDOUT);
    //cbc_print();

    return ( vmm_md_id );
}

/**
*
* end VMM module
*
**/
void vmm_end(const UINT32 vmm_md_id)
{
    VMM_MD *vmm_md;

    vmm_md = VMM_MD_GET(vmm_md_id);
    if(NULL_PTR == vmm_md)
    {
        dbg_log(SEC_0003_VMM, 0)(LOGSTDOUT,"error:vmm_end: vmm_md_id = %ld not exist.\n", vmm_md_id);
        dbg_exit(MD_VMM, vmm_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < vmm_md->usedcounter )
    {
        vmm_md->usedcounter --;
        return ;
    }

    if ( 0 == vmm_md->usedcounter )
    {
        dbg_log(SEC_0003_VMM, 0)(LOGSTDOUT,"error:vmm_end: vmm_md_id = %ld is not started.\n", vmm_md_id);
        dbg_exit(MD_VMM, vmm_md_id);
    }

    //task_brd_mod_mgr_rmv(vmm_md->task_brd, vmm_md->mod_mgr);
    mod_mgr_free(vmm_md->mod_mgr);
    vmm_md->mod_mgr  = NULL_PTR;

     /* free module : */
    //vmm_free_module_static_mem(vmm_md_id);

    vmm_md->usedcounter = 0;

    dbg_log(SEC_0003_VMM, 5)(LOGSTDOUT, "vmm_end: stop VMM module #%ld\n", vmm_md_id);
    cbc_md_free(MD_VMM, vmm_md_id);

    breathing_static_mem();

    //dbg_log(SEC_0003_VMM, 3)(LOGSTDOUT, "========================= vmm_end: VMM_ table info:\n");
    //vmm_print_module_status(vmm_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}


/**
*
* initialize mod mgr of VMM module
*
**/
UINT32 vmm_set_mod_mgr(const UINT32 vmm_md_id, const MOD_MGR * src_mod_mgr)
{
    VMM_MD *vmm_md;
    MOD_MGR * des_mod_mgr;

#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_set_mod_mgr: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    vmm_md = VMM_MD_GET(vmm_md_id);
    des_mod_mgr = vmm_md->mod_mgr;

    dbg_log(SEC_0003_VMM, 5)(LOGSTDOUT, "vmm_set_mod_mgr: md_id %d, des_mod_mgr %lx\n", vmm_md_id, des_mod_mgr);

    mod_mgr_limited_clone(vmm_md_id, src_mod_mgr, des_mod_mgr);
    return (0);
}

/**
*
* get mod mgr of VMM module
*
**/
MOD_MGR * vmm_get_mod_mgr(const UINT32 vmm_md_id)
{
    VMM_MD *vmm_md;

    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        return (MOD_MGR *)0;
    }

    vmm_md = VMM_MD_GET(vmm_md_id);
    return (vmm_md->mod_mgr);
}

/**
*
* init vmm node in VMM module
*
**/
UINT32 vmm_init_node(const UINT32 vmm_md_id, VMM_NODE *vmm_node)
{
#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_init_node: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    mod_node_init(VMM_NODE_MOD_NODE(vmm_node));
    VMM_NODE_ADDR(vmm_node) = 0;
    return (0);
}

/**
*
* clean vmm node mgr in VMM module
*
**/
UINT32 vmm_clean_node(const UINT32 vmm_md_id, VMM_NODE *vmm_node)
{
#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_clean_node: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    mod_node_init(VMM_NODE_MOD_NODE(vmm_node));
    VMM_NODE_ADDR(vmm_node) = 0;
    return (0);
}

/**
*
* alloc virtual memory
*
**/
EC_BOOL vmm_alloc(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, VMM_NODE *vmm_node)
{
    void *pvoid;

    TASK_BRD *task_brd;

#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_alloc: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    pvoid = NULL_PTR;
    alloc_static_mem(module_type, module_id, mm_type, &pvoid, LOC_VMM_0001);

    task_brd = task_brd_default_get();

    if(NULL_PTR != pvoid)
    {
        VMM_NODE_TCID(vmm_node) = TASK_BRD_TCID(task_brd);
        VMM_NODE_COMM(vmm_node) = TASK_BRD_COMM(task_brd);
        VMM_NODE_RANK(vmm_node) = TASK_BRD_RANK(task_brd);
        VMM_NODE_MODI(vmm_node) = vmm_md_id;
        VMM_NODE_LOAD(vmm_node) = task_brd_rank_load_tbl_get_que(task_brd, VMM_NODE_TCID(vmm_node), VMM_NODE_RANK(vmm_node));
        VMM_NODE_ADDR(vmm_node) = (UINT32)pvoid;
        return (EC_TRUE);
    }

    VMM_NODE_TCID(vmm_node) = TASK_BRD_TCID(task_brd);
    VMM_NODE_COMM(vmm_node) = TASK_BRD_COMM(task_brd);
    VMM_NODE_RANK(vmm_node) = TASK_BRD_RANK(task_brd);
    VMM_NODE_MODI(vmm_node) = vmm_md_id;
    VMM_NODE_LOAD(vmm_node) = task_brd_rank_load_tbl_get_que(task_brd, VMM_NODE_TCID(vmm_node), VMM_NODE_RANK(vmm_node));
    VMM_NODE_ADDR(vmm_node) = 0;

    return (EC_FALSE);
}

/**
*
* free virtual memory
*
**/
EC_BOOL vmm_free(const UINT32 vmm_md_id, const UINT32 module_type, const UINT32 module_id, const UINT32 mm_type, const VMM_NODE *vmm_node)
{
    void *pvoid;

#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_free: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    pvoid = (void *)VMM_NODE_ADDR(vmm_node);
    free_static_mem(module_type, module_id, mm_type, pvoid, LOC_VMM_0002);

    return (EC_TRUE);
}

/**
*
* bind memory to vmm_node
*
**/
EC_BOOL vmm_bind(const UINT32 vmm_md_id, const UINT32 mem_addr, VMM_NODE *vmm_node)
{
#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_bind: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    VMM_NODE_ADDR(vmm_node) = mem_addr;
    return (EC_TRUE);
}

/**
*
* dismiss memory from vmm_node
*
**/
EC_BOOL vmm_dismiss(const UINT32 vmm_md_id, VMM_NODE *vmm_node, UINT32 *mem_addr)
{
#if ( SWITCH_ON == VMM_DEBUG_SWITCH )
    if ( UINT32_CHECK_INVALID(vmm_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:vmm_dismiss: vmm module #0x%lx not started.\n",
                vmm_md_id);
        vmm_print_module_status(vmm_md_id, LOGSTDOUT);
        dbg_exit(MD_VMM, vmm_md_id);
    }
#endif/*VMM_DEBUG_SWITCH*/

    (*mem_addr) = VMM_NODE_ADDR(vmm_node);
    VMM_NODE_ADDR(vmm_node) = 0;
    return (EC_TRUE);
}

UINT32 vmm_print(const UINT32 vmm_md_id, LOG *log, const VMM_NODE *vmm_node)
{
    sys_log(log, "vmm_node %lx: tcid %s, comm %ld, rank %ld, modi %ld, load %ld, addr %lx\n",
                    vmm_node,
                    VMM_NODE_TCID_STR(vmm_node),
                    VMM_NODE_COMM(vmm_node),
                    VMM_NODE_RANK(vmm_node),
                    VMM_NODE_MODI(vmm_node),
                    VMM_NODE_LOAD(vmm_node),
                    VMM_NODE_ADDR(vmm_node)
                    );
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

