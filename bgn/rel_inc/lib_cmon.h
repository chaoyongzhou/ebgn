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

#ifndef _LIB_CMON_H
#define _LIB_CMON_H

#include <stdlib.h>
#include <stdio.h>

#include "lib_type.h"
#include "lib_cstring.h"

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

/**
*   for test only
*
*   to query the status of CMON Module
*
**/
void print_cmon_status(LOG *log);

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
UINT32 cmon_set_mod_mgr(const UINT32 cmon_md_id, const void * src_mod_mgr);

/**
*
* get mod mgr of CMON module
*
**/
void * cmon_get_mod_mgr(const UINT32 cmon_md_id);

/**
*
* new a CMON_OBJ_VEC
*
**/
void * cmon_obj_vec_new(const UINT32 cmon_md_id);

/**
*
* initialize a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_init(const UINT32 cmon_md_id, void * cmon_obj_vec);

/**
*
* clean a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_clean(const UINT32 cmon_md_id, void * cmon_obj_vec);

/**
*
* free a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_free(const UINT32 cmon_md_id, void * cmon_obj_vec);

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_size(const UINT32 cmon_md_id, const void * cmon_obj_vec);

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_add(const UINT32 cmon_md_id, const void *cmon_obj, void * cmon_obj_vec);

/**
*
* bind a CMON_OBJ_MERGE to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_bind(const UINT32 cmon_md_id, void * cmon_obj_vec);

/**
*
* merge a CMON_OBJ to CMON_OBJ_VEC_MERGE_VEC of CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge(const UINT32 cmon_md_id, const void *cmon_obj, void * cmon_obj_vec);

/**
*
* print the merged CMON_OBJ result
*
**/
UINT32 cmon_obj_vec_merge_print(const UINT32 cmon_md_id, const void *cmon_obj_vec, LOG *log);
/**
*
* get a CMON_OBJ from CMON_OBJ_VEC
*
**/
void * cmon_obj_vec_get(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const void * cmon_obj_vec);

/**
*
* set a CMON_OBJ to CMON_OBJ_VEC
*
**/
void * cmon_obj_vec_set(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const void *cmon_obj, void * cmon_obj_vec);

/**
*
* include or define CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_incl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, void * cmon_obj_vec);

/**
*
* exclude or undefine CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_excl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, void * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC measure and collect result from remote mod_node
*
* note: one CMON_OBJ measure and collect one remote mod_node which is given by mod_node in CMON_OBJ
*
**/
UINT32 cmon_obj_vec_meas(const UINT32 cmon_md_id, const UINT32 time_to_live, void * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC report result
*
* note: CMON_OBJ report one by one
*
**/
UINT32 cmon_obj_vec_print(const UINT32 cmon_md_id, LOG *log, const void * cmon_obj_vec);

/**
*
* CMON_OBJ_VEC report result
*
* note: CMON_OBJ report one by one
*
**/
UINT32 cmon_obj_vec_report(const UINT32 cmon_md_id, void * cmon_obj_vec);


#endif /*_LIB_CMON_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

