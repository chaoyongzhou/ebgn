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

#ifndef _CMON_H
#define _CMON_H

#include <stdlib.h>
#include <stdio.h>

#include "type.h"
#include "cstring.h"
#include "clist.h"
#include "cvector.h"
#include "csys.h"
#include "mod.inc"

/*monitor object id defintion*/
#define CMON_OID_MEM_OCCUPY_PL        ((UINT32) 0)  /*memory occupy monitor on process level, value range: [0, max_uint32]*/
#define CMON_OID_MEM_OCCUPY_CL        ((UINT32) 1)  /*memory occupy monitor on taskcomm level, value range: [0, max_uint32]*/
#define CMON_OID_MEM_OCCUPY_SL        ((UINT32) 2)  /*memory occupy monitor on system level, value range: [0, max_uint32]*/
#define CMON_OID_MEM_LOAD_PL          ((UINT32) 3)  /*memory load percent monitor on process level, value range: [0.00, 1.00] * 100%*/
#define CMON_OID_MEM_LOAD_CL          ((UINT32) 4)  /*memory load percent monitor on taskcomm level, value range: [0.00, 1.00] * 100%*/
#define CMON_OID_MEM_LOAD_SL          ((UINT32) 5)  /*memory load percent monitor on system level, value range: [0.00, 1.00] * 100%*/
#define CMON_OID_TASKC_NODE_VEC_CL    ((UINT32) 6)  /*taskcomm monitor of each fwd process*/
#define CMON_OID_CSOCKET_CNODE_VEC_CL ((UINT32) 7)  /*taskcomm socket connections of each fwd process*/
#define CMON_OID_THREAD_NUM_PL        ((UINT32) 8)  /*thread num on process level*/
#define CMON_OID_THREAD_NUM_CL        ((UINT32) 9)  /*thread num on taskcomm level*/
#define CMON_OID_THREAD_STAT_RL       ((UINT32)10)  /*thread num on rank level*/
#define CMON_OID_DSK_LOAD_SL          ((UINT32)11)  /*disk percent monitor on system level, value range: [0.00, 1.00] * 100%*/
#define CMON_OID_CPU_LOAD_PL          ((UINT32)12)  /*average cpu percent monitor on process level, value range: [0.0, 100.0]*/
#define CMON_OID_CPU_LOAD_CL          ((UINT32)13)  /*average cpu percent monitor on taskcomm level, value range: [0.0, 100.0]*/
#define CMON_OID_CPU_LOAD_SL          ((UINT32)14)  /*average cpu percent monitor on system level, value range: [0.0, 100.0]*/
#define CMON_OID_CPU_LOAD_EL          ((UINT32)15)  /*each cpu percent monitor on system level, value range: [0.0, 100.0]*/
#define CMON_OID_MM_MAN_OCCUPY_PL     ((UINT32)16)  /*each memory manager occupy monitor on process level, value range: [0.0, 100.0]*/
#define CMON_OID_MM_MAN_LOAD_PL       ((UINT32)17)  /*each memory manager load monitor on process level, value range: [0.0, 100.0]*/
#define CMON_OID_MODULE_NUM_PL        ((UINT32)18)  /*specific module instance num on process level*/
#define CMON_OID_MODULE_NUM_CL        ((UINT32)19)  /*specific module instance num on taskcomm level*/
#define CMON_OID_TASK_LOAD_RL         ((UINT32)20)  /*task total num on rank level*/
#define CMON_OID_TASK_LOAD_CL         ((UINT32)21)  /*task total num on taskcomm level*/
#define CMON_OID_ETH_LOAD_SL          ((UINT32)22)  /*ethernet throughput monitor on system level, value range: [0, max_uint32]*/
#define CMON_OID_TASK_REPORT_RL       ((UINT32)23)  /*dump task report nodes on rank level, data item type: TASK_REPORT_NODE*/
#define CMON_OID_TASK_REPORT_CL       ((UINT32)24)  /*dump task report nodes on taskcomm level, data item type: TASK_REPORT_NODE*/
#define CMON_OID_UNDEF                ((UINT32)-1)  /*undefined*/

#define CMON_MEAS_DATA_AREA           ((UINT32) 0)
#define CMON_REPORT_DATA_AREA         ((UINT32) 1)
#define CMON_CACHE_DATA_AREA          ((UINT32) 2)
#define CMON_END_DATA_AREA            ((UINT32) 3)

typedef struct
{
    /* used counter >= 0 */
    UINT32    usedcounter ;

    MOD_MGR  *mod_mgr;

}CMON_MD;

struct _CMON_MAP;       /*claim the type here due to it be used in definition of CMON_MAP*/
struct _CMON_OBJ;       /*claim the type here due to it be used in definition of CMON_MAP*/
struct _CMON_OBJ_MERGE; /*claim the type here due to it be used in definition of CMON_MAP*/
struct _CMON_OBJ_VEC;   /*claim the type here due to it be used in definition of CMON_MAP*/

typedef UINT32 (*CMON_DATA_INIT_FUNC)(const UINT32, const UINT32, struct _CMON_OBJ *);
typedef UINT32 (*CMON_DATA_CLEAN_FUNC)(const UINT32, const UINT32, struct _CMON_OBJ *);

typedef void (*CMON_DATA_VEC_ITEM_FREE_FUNC)(void *);

typedef UINT32 (*CMON_OBJ_MEAS_FUNC)(const UINT32, struct _CMON_OBJ *);
typedef UINT32 (*CMON_OBJ_PRINT_FUNC)(const UINT32, LOG *, const struct _CMON_OBJ *);
typedef UINT32 (*CMON_OBJ_REPORT_FUNC)(const UINT32, struct _CMON_OBJ *);
typedef UINT32 (*CMON_OBJ_MERGE_INIT_FUNC)(const UINT32, const struct _CMON_MAP *, const MOD_NODE *, struct _CMON_OBJ_MERGE *);
typedef UINT32 (* CMON_OBJ_VEC_MERGE_MATCH_FUNC)(const UINT32 , const struct _CMON_OBJ *, const struct _CMON_OBJ_MERGE *);
typedef EC_BOOL (* CMON_OBJ_VEC_MERGE_FILTER_FUNC)(const UINT32,  const struct _CMON_OBJ *);
typedef UINT32 (* CMON_OBJ_VEC_MERGE_NAME_FUNC)(const UINT32 , const struct _CMON_OBJ *, struct _CMON_OBJ_MERGE *);
typedef UINT32 (* CMON_OBJ_VEC_MERGE_CALC_FUNC)(const UINT32 , struct _CMON_OBJ_MERGE *);
typedef UINT32 (* CMON_OBJ_VEC_MERGE_PRINT_FUNC)(const UINT32 , const struct _CMON_OBJ_MERGE *, LOG *);

typedef struct _CMON_MAP
{
    UINT32  cmon_oid;
    UINT32  cmon_data_type;  
    UINT32  cmon_item_type;  /*when cmon_data_type is MM_CVECTOR, cmon_item_type take effect and indicate the item type of the vector*/
    
    CMON_DATA_INIT_FUNC             cmon_data_init_func;
    CMON_DATA_CLEAN_FUNC            cmon_data_clean_func;
    
    CMON_DATA_VEC_ITEM_FREE_FUNC    cmon_data_vec_item_free_func;

    UINT32                          cmon_obj_merge_vec_item_type;
    CMON_OBJ_MERGE_INIT_FUNC        cmon_obj_merge_init_func;
    
    CMON_OBJ_MEAS_FUNC              cmon_obj_meas_func;
    CMON_OBJ_PRINT_FUNC             cmon_obj_print_func;
    CMON_OBJ_REPORT_FUNC            cmon_obj_report_func;
    
    CMON_OBJ_VEC_MERGE_MATCH_FUNC   cmon_obj_vec_merge_match_func;
    CMON_OBJ_VEC_MERGE_FILTER_FUNC  cmon_obj_vec_merge_filter_func;
    CMON_OBJ_VEC_MERGE_NAME_FUNC    cmon_obj_vec_merge_name_func;
    CMON_OBJ_VEC_MERGE_CALC_FUNC    cmon_obj_vec_merge_calc_func;
    CMON_OBJ_VEC_MERGE_PRINT_FUNC   cmon_obj_vec_merge_print_func;
}CMON_MAP;

typedef union
{
    UINT32    val_uint32;
    REAL      val_real;
    CVECTOR   val_vec;

    CRANK_THREAD_STAT val_crank_thread_stat;
}CMON_DATA;

typedef struct _CMON_OBJ
{
    MOD_NODE     *mod_node;

    CMON_MAP     *cmon_map;  /*private data area*/

    CMON_DATA     cmon_data[ CMON_END_DATA_AREA ];
}CMON_OBJ;

/*patch*/
typedef struct _CMON_OBJ_MERGE
{
    UINT32    cmon_oid;
    MOD_NODE  mod_node;

    CMON_MAP *cmon_map;
    
    CSTRING   name;        /*merged name: encapsulated by tcid,rank,modi etc.*/
    CVECTOR   cmon_obj_vec;/*reference cmon_obj of cmon_obj_vec*/

    CMON_DATA result;
/*    
    union
    {
        UINT32    val_uint32;
        REAL      val_real;
        CVECTOR   val_vec;        
    }result;
*/    
}CMON_OBJ_MERGE;

/*note: one CMON_OBJ_VEC may own several OIDs*/
typedef struct _CMON_OBJ_VEC
{
    CVECTOR         cmon_obj_vec;

    CVECTOR        *cmon_obj_merge_vec;/*[patch] vector of CMON_OBJ_MERGE*/
}CMON_OBJ_VEC;

#define CMON_MAP_OID(cmon_map)                          ((cmon_map)->cmon_oid)
#define CMON_MAP_DATA_TYPE(cmon_map)                    ((cmon_map)->cmon_data_type)
#define CMON_MAP_ITEM_TYPE(cmon_map)                    ((cmon_map)->cmon_item_type)
#define CMON_MAP_DATA_INIT_FUNC(cmon_map)               ((cmon_map)->cmon_data_init_func)
#define CMON_MAP_DATA_CLEAN_FUNC(cmon_map)              ((cmon_map)->cmon_data_clean_func)
#define CMON_MAP_DATA_VEC_ITEM_FREE_FUNC(cmon_map)      ((cmon_map)->cmon_data_vec_item_free_func)
#define CMON_MAP_MERGE_VEC_ITEM_TYPE(cmon_map)          ((cmon_map)->cmon_obj_merge_vec_item_type)
#define CMON_MAP_MERGE_INIT_FUNC(cmon_map)              ((cmon_map)->cmon_obj_merge_init_func)
#define CMON_MAP_OBJ_MEAS_FUNC(cmon_map)                ((cmon_map)->cmon_obj_meas_func)
#define CMON_MAP_OBJ_PRINT_FUNC(cmon_map)               ((cmon_map)->cmon_obj_print_func)
#define CMON_MAP_OBJ_REPORT_FUNC(cmon_map)              ((cmon_map)->cmon_obj_report_func)
#define CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC(cmon_map)     ((cmon_map)->cmon_obj_vec_merge_match_func)
#define CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC(cmon_map)    ((cmon_map)->cmon_obj_vec_merge_filter_func)
#define CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC(cmon_map)      ((cmon_map)->cmon_obj_vec_merge_name_func)
#define CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC(cmon_map)      ((cmon_map)->cmon_obj_vec_merge_calc_func)
#define CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC(cmon_map)     ((cmon_map)->cmon_obj_vec_merge_print_func)

#define CMON_DATA_UINT32(cmon_data)                     ((cmon_data)->val_uint32)
#define CMON_DATA_REAL(cmon_data)                       ((cmon_data)->val_real)

#define CMON_OBJ_MOD_NODE(cmon_obj)                     ((cmon_obj)->mod_node)
#define CMON_OBJ_MAP(cmon_obj)                          ((cmon_obj)->cmon_map)
#define CMON_OBJ_OID(cmon_obj)                          (CMON_MAP_OID(CMON_OBJ_MAP(cmon_obj)))
#define CMON_OBJ_DATA_TYPE(cmon_obj)                    (CMON_MAP_DATA_TYPE(CMON_OBJ_MAP(cmon_obj)))
#define CMON_OBJ_ITEM_TYPE(cmon_obj)                    (CMON_MAP_ITEM_TYPE(CMON_OBJ_MAP(cmon_obj)))

#define CMON_OBJ_DATA_UINT32(cmon_obj, data_area)       ((cmon_obj)->cmon_data[(data_area)].val_uint32)
#define CMON_OBJ_DATA_REAL(cmon_obj, data_area)         ((cmon_obj)->cmon_data[(data_area)].val_real)
#define CMON_OBJ_DATA_VEC(cmon_obj, data_area)          (&((cmon_obj)->cmon_data[(data_area)].val_vec))

#define CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, data_area)          (&((cmon_obj)->cmon_data[(data_area)].val_crank_thread_stat))

#define CMON_OBJ_MEAS_DATA_UINT32(cmon_obj)             (CMON_OBJ_DATA_UINT32(cmon_obj, CMON_MEAS_DATA_AREA))
#define CMON_OBJ_MEAS_DATA_REAL(cmon_obj)               (CMON_OBJ_DATA_REAL(cmon_obj, CMON_MEAS_DATA_AREA))
#define CMON_OBJ_MEAS_DATA_VEC(cmon_obj)                (CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA))
#define CMON_OBJ_MEAS_DATA_CRANK_THREAD_STAT(cmon_obj)                (CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_MEAS_DATA_AREA))

#define CMON_OBJ_REPORT_DATA_UINT32(cmon_obj)           (CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA))
#define CMON_OBJ_REPORT_DATA_REAL(cmon_obj)             (CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA))
#define CMON_OBJ_REPORT_DATA_VEC(cmon_obj)              (CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA))
#define CMON_OBJ_REPORT_DATA_CRANK_THREAD_STAT(cmon_obj)                (CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_REPORT_DATA_AREA))

#define CMON_OBJ_VEC(cmon_objs)                         (&((cmon_objs)->cmon_obj_vec))
#define CMON_OBJ_VEC_SIZE(cmon_objs)                    (cvector_size(CMON_OBJ_VEC(cmon_objs)))
#define CMON_OBJ_VEC_ADD(cmon_objs, cmon_obj_new)       (cvector_push(CMON_OBJ_VEC(cmon_objs), (void *)(cmon_obj_new)))
#define CMON_OBJ_VEC_GET(cmon_objs, pos)                (cvector_get(CMON_OBJ_VEC(cmon_objs), (pos)))
#define CMON_OBJ_VEC_MERGE_VEC(cmon_objs)               ((cmon_objs)->cmon_obj_merge_vec)
#define CMON_OBJ_VEC_MERGE_VEC_SIZE(cmon_objs)          (cvector_size(CMON_OBJ_VEC_MERGE_VEC(cmon_objs)))
#define CMON_OBJ_VEC_MERGE_VEC_GETE(cmon_objs, pos)     (cvector_get(CMON_OBJ_VEC_MERGE_VEC(cmon_objs), (pos)))

#define CMON_OBJ_MERGE_OID(cmon_obj_merge)               ((cmon_obj_merge)->cmon_oid)
#define CMON_OBJ_MERGE_MAP(cmon_obj_merge)               ((cmon_obj_merge)->cmon_map)
#define CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)          (&((cmon_obj_merge)->mod_node))
#define CMON_OBJ_MERGE_NAME(cmon_obj_merge)              (&((cmon_obj_merge)->name))
#define CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)          (cstring_get_str(CMON_OBJ_MERGE_NAME(cmon_obj_merge)))
#define CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge)           (&((cmon_obj_merge)->cmon_obj_vec))
#define CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge)     ((cmon_obj_merge)->result.val_uint32)
#define CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge)       ((cmon_obj_merge)->result.val_real)
#define CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge)        ((CVECTOR *)(&((cmon_obj_merge)->result.val_vec)))
#define CMON_OBJ_MERGE_RESULT_CRANK_THREAD_STAT(cmon_obj_merge)  (&((cmon_obj_merge)->result.val_crank_thread_stat))

/**
*   for test only
*
*   to query the status of CMON Module
*
**/
void cmon_print_module_status(const UINT32 cmon_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CMON module
*
*
**/
UINT32 cmon_free_module_static_mem(const UINT32 cmon_md_id);

/**
*
* start CMON module
*
**/
UINT32 cmon_start();

/**
*
* end CMON module
*
**/
void cmon_end(const UINT32 cmon_md_id);

/**
*
* initialize mod mgr of CMON module
*
**/
UINT32 cmon_set_mod_mgr(const UINT32 cmon_md_id, const MOD_MGR * src_mod_mgr);

UINT32 cmond_add_mod_mgr(const UINT32 cmon_md_id, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi);

/**
*
* get mod mgr of CMON module
*
**/
MOD_MGR * cmon_get_mod_mgr(const UINT32 cmon_md_id);

/**
*
* check existing of mod_node in mod mgr of current CMON
*
**/
EC_BOOL cmon_check_mod_node_exist(const UINT32 cmon_md_id, const MOD_NODE *mod_node);

/**
*
* fetch CMON_MAP from g_cmon_map
*
**/
CMON_MAP *cmon_map_fetch(const UINT32 cmon_md_id, const UINT32 cmon_oid);


/**
*
* initialize a CMON_OBJ with undefined data type
* 
* note: this function is used by TYPE_CONV_ITEM
*
**/
UINT32 cmon_obj_init_undef(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* clean a CMON_OBJ with undefined data type
* 
*
**/
UINT32 cmon_obj_clean_undef(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* clean a CMON_OBJ with both measurement data and report data
* 
*
**/
UINT32 cmon_obj_clean_0(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* new a CMON_OBJ
*
**/
CMON_OBJ * cmon_obj_new(const UINT32 cmon_md_id, const UINT32 cmon_oid);

/**
*
* initialize a CMON_OBJ
*
* note: CMON_OBJ_MAP must be set before calling
*
**/
UINT32 cmon_obj_init(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* clean a CMON_OBJ
*
**/
UINT32 cmon_obj_clean(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* free a CMON_OBJ
*
**/
UINT32 cmon_obj_free(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* set UINT32 value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_uint32(const UINT32 cmon_md_id, const UINT32 data, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* set REAL value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_real(const UINT32 cmon_md_id, const REAL * data, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* set CRANK_THREAD_STAT value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_crank_thread_stat(const UINT32 cmon_md_id, const CRANK_THREAD_STAT * crank_thread_stat, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push UINT32 value a CMON_OBJ
*
**/
UINT32 cmon_obj_push_uint32(const UINT32 cmon_md_id, const UINT32 data, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push REAL to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_real(const UINT32 cmon_md_id, const REAL * data, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push CSYS_CPU_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_cpu_stat(const UINT32 cmon_md_id, const CSYS_CPU_STAT * csys_cpu_stat, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push TASKC_NODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_taskc_node(const UINT32 cmon_md_id, const TASKC_NODE * taskc_node, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push CSOCKET_CNODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csocket_cnode(const UINT32 cmon_md_id, const CSOCKET_CNODE * csocket_cnode, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push CPROC_MODULE_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_cproc_module_stat(const UINT32 cmon_md_id, const CPROC_MODULE_STAT * cproc_module_stat, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push CSYS_ETH_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_eth_stat(const UINT32 cmon_md_id, const CSYS_ETH_STAT * csys_eth_stat, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push CSYS_DSK_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_dsk_stat(const UINT32 cmon_md_id, const CSYS_DSK_STAT * csys_dsk_stat, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* push TASK_REPORT_NODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_crank_task_report_node(const UINT32 cmon_md_id, const TASK_REPORT_NODE * task_report_node, const UINT32 data_area, CMON_OBJ * cmon_obj);

/**
*
* check cmon_obj is connected or not yet
*
**/
EC_BOOL cmon_obj_is_connected(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* CMON_OBJ measure and collect info according to cmon_oid
*
**/
UINT32 cmon_obj_meas(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_default(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_sys_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_proc_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_sys_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_proc_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_proc_thread_num(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_proc_cpu_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_sys_cpu_avg_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_proc_module_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_crank_task_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_meas_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* CMON_OBJ report according to cmon_oid
*
**/
UINT32 cmon_obj_print(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_default(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_mem_occupy(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_mem_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_taskc_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_csocket_cnode_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_thread_num(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_crank_thread_stat(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_disk_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_cpu_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_cpu_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_mm_man_occupy_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_mm_man_load_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_module_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_crank_task_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_csys_eth_stat_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_csys_dsk_stat_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);
UINT32 cmon_obj_print_crank_task_report_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj);

/**
*
* CMON_OBJ report according to cmon_oid
*
**/
UINT32 cmon_obj_report(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_thread_num(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_cpu_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_module_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_crank_task_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);
UINT32 cmon_obj_report_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj);

/**
*
* new a CMON_OBJ_VEC
*
**/
CMON_OBJ_VEC * cmon_obj_vec_new(const UINT32 cmon_md_id);

/**
*
* initialize a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_init(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* clean a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_clean(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* free a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_free(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_size(const UINT32 cmon_md_id, const CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_add(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* bind a CMON_OBJ_MERGE to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_bind(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* merge a CMON_OBJ to CMON_OBJ_VEC_MERGE_VEC of CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* print the merged CMON_OBJ result
*
**/
UINT32 cmon_obj_vec_merge_print(const UINT32 cmon_md_id, const CMON_OBJ_VEC *cmon_obj_vec, LOG *log);

/**
*
* get a CMON_OBJ from CMON_OBJ_VEC
*
**/
CMON_OBJ * cmon_obj_vec_get(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* set a CMON_OBJ to CMON_OBJ_VEC
*
**/
CMON_OBJ * cmon_obj_vec_set(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* include or define CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_incl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* exclude or undefine CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_excl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
*   purge broken remote measured object if found in cmon_obj_vec
*
*
**/
UINT32 cmon_obj_vec_purge(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC measure and collect result from remote mod_node
*
* note: one CMON_OBJ measure and collect one remote mod_node which is given by mod_node in CMON_OBJ
*
**/
UINT32 cmon_obj_vec_meas(const UINT32 cmon_md_id, const UINT32 time_to_live, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC report result
*
* note: CMON_OBJ report one by one
*
**/
UINT32 cmon_obj_vec_print(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC report result
*
* note: CMON_OBJ report one by one
*
**/
UINT32 cmon_obj_vec_report(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* clean result area of CMON_OBJ_MERGEs of a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge_result_clean(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec);

/**
*
* purge cmon obj from all CMON_OBJ_MERGEs of a CMON_OBJ_VEC
*
* after purge cmon obj, if CMON_OBJ_MERGE is empty, then erase it from CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge_purge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC *cmon_obj_vec);

/**
*
* calculate the merged CMON_OBJ sum or average result
*
**/
UINT32 cmon_obj_vec_merge_calc(const UINT32 cmon_md_id, CMON_OBJ_VEC *cmon_obj_vec);
UINT32 cmon_obj_vec_merge_sum_real(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_sum_uint32(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_sum_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_avg_real(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_avg_uint32(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_module_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);
UINT32 cmon_obj_vec_merge_csys_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge);

/**
*
* print the merged CMON_OBJ result
*
**/
UINT32 cmon_obj_vec_merge_print_real(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_uint32(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_crank_thread_stat(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_taskc_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_csocket_cnode_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_cpu_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_mm_man_occupy_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_mm_man_load_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_module_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_csys_eth_stat_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_csys_dsk_stat_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);
UINT32 cmon_obj_vec_merge_print_crank_task_report_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log);

/**
*
* new a CMON_OBJ_MERGE
*
**/
CMON_OBJ_MERGE * cmon_obj_merge_new(const UINT32 cmon_md_id, const UINT32 cmon_oid, const MOD_NODE *mod_node);

/**
*
* initialize a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_uint32_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_real_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_vec_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_crank_thread_stat_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* clean result area of a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_result_clean(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* purge cmon_obj from CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_purge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* check the merged cmon obj vec is empty or not
*
**/
EC_BOOL cmon_obj_merge_is_empty(const UINT32 cmon_md_id, const CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* clean a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_clean(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* free a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_free(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge);

/**
*
* determine a CMON_OBJ belogn to this CMON_OBJ_MGERGE or not
*   o: oid
*   t: tcid
*   c: comm
*   r: rank
*   m: modi
*
**/
EC_BOOL cmon_obj_merge_match_otcrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_otcr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_otc(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_ot(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_o(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_otrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_otr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);
EC_BOOL cmon_obj_merge_match_ot(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge);


EC_BOOL cmon_obj_merge_filter_fwd_rank(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj);

/**
*
* set name of CMON_OBJ_MGERGE
*   o: oid
*   t: tcid
*   c: comm
*   r: rank
*   m: modi
*
**/
UINT32 cmon_obj_merge_name_otcrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_name_otcr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_name_otc(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_name_ot(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_name_otrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);
UINT32 cmon_obj_merge_name_otr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge);


#endif /*_CMON_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

