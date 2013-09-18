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

#include <stdlib.h>
#include <stdio.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"
#include "clist.h"
#include "tcnode.h"
#include "cbc.h"

#include "cxml.h"
#include "char2int.h"

#include "task.h"
#include "csocket.h"

#include "cmon.h"
#include "real.h"

#include "tasks.h"

#include "csys.h"

#include "debug.h"

#include "findex.inc"

#define CMON_MD_CAPACITY()                  (cbc_md_capacity(MD_CMON))

#define CMON_MD_GET(cmon_md_id)     ((CMON_MD *)cbc_md_get(MD_CMON, (cmon_md_id)))

#define CMON_MD_ID_CHECK_INVALID(cmon_md_id)  \
    ((CMPI_ANY_MODI != (cmon_md_id)) && ((NULL_PTR == CMON_MD_GET(cmon_md_id)) || (0 == (CMON_MD_GET(cmon_md_id)->usedcounter))))


//#define CMON_DBG(x) sys_log x
#define CMON_DBG(x) do{}while(0)
#define CMON_LOG(x) sys_log x

static UINT32 cmon_obj_init_uint32(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_init_real(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_init_vec(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_init_crank_thread_stat(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_clean_uint32(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_clean_real(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_clean_vec(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);
static UINT32 cmon_obj_clean_crank_thread_stat(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj);

static CMON_MAP g_cmon_map[] = {
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_OCCUPY_PL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_mem_occupy,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_occupy,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_occupy,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_OCCUPY_CL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_mem_occupy,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_occupy,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_occupy,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_OCCUPY_SL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_sys_mem_occupy,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_occupy,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_occupy,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_avg_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_LOAD_PL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_mem_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_real,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_LOAD_CL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_mem_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_real,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MEM_LOAD_SL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_sys_mem_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mem_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mem_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_avg_real,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_TASKC_NODE_VEC_CL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_TASKC_NODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)taskc_node_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_TASKC_NODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_taskc_node_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_taskc_node_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_taskc_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    cmon_obj_merge_filter_fwd_rank,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_taskc_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_taskc_node_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_CSOCKET_CNODE_VEC_CL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CSOCKET_CNODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)csocket_cnode_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CSOCKET_CNODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_csocket_cnode_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_csocket_cnode_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_csocket_cnode_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    cmon_obj_merge_filter_fwd_rank,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_csocket_cnode_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_csocket_cnode_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_THREAD_NUM_PL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_thread_num,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_thread_num,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_thread_num,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_THREAD_NUM_CL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_thread_num,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_thread_num,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_thread_num,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_THREAD_STAT_RL,
/*CMON_MAP_DATA_TYPE                */    MM_CRANK_THREAD_STAT,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_crank_thread_stat,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_crank_thread_stat,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CRANK_THREAD_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_crank_thread_stat_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_crank_thread_stat,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_crank_thread_stat,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_crank_thread_stat,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_crank_thread_stat,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_crank_thread_stat,
},
{
/*CMON_MAP_OID                      */    CMON_OID_DSK_LOAD_SL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CSYS_DSK_STAT,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)csys_dsk_stat_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CSYS_DSK_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_csys_dsk_stat_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_csys_dsk_stat_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_csys_dsk_stat_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_csys_dsk_stat_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_csys_eth_stat_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_CPU_LOAD_PL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_cpu_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_cpu_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_cpu_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_real,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_CPU_LOAD_CL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_cpu_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_cpu_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_cpu_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_avg_real,/*avg or sum?*/
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_CPU_LOAD_SL,
/*CMON_MAP_DATA_TYPE                */    MM_REAL,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_real,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_real,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_REAL,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_real_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_sys_cpu_avg_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_cpu_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_cpu_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_avg_real,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_real,
},
{
/*CMON_MAP_OID                      */    CMON_OID_CPU_LOAD_EL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CSYS_CPU_STAT,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)csys_cpu_stat_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CSYS_CPU_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_cpu_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_cpu_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_cpu_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_cpu_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_cpu_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MM_MAN_OCCUPY_PL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_MM_MAN_OCCUPY_NODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)mm_man_occupy_node_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_MM_MAN_OCCUPY_NODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_mm_man_occupy_node_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mm_man_occupy_node_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mm_man_occupy_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_mm_man_occupy_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_mm_man_occupy_node_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MM_MAN_LOAD_PL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_MM_MAN_LOAD_NODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)mm_man_load_node_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_MM_MAN_LOAD_NODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_mm_man_load_node_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_mm_man_load_node_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_mm_man_load_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_mm_man_load_node_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_mm_man_load_node_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MODULE_NUM_PL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CPROC_MODULE_STAT,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)cproc_module_stat_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CPROC_MODULE_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_module_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_module_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_module_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_module_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_module_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_MODULE_NUM_CL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CPROC_MODULE_STAT,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)cproc_module_stat_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CPROC_MODULE_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_proc_module_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_module_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_module_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_module_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_module_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_TASK_LOAD_RL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_crank_task_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_crank_task_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_crank_task_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_TASK_LOAD_CL,
/*CMON_MAP_DATA_TYPE                */    MM_UINT32,
/*CMON_MAP_ITEM_TYPE                */    MM_END,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_uint32,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_uint32,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    NULL_PTR,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_UINT32,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_uint32_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_crank_task_load,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_crank_task_load,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_crank_task_load,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_sum_uint32,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_uint32,
},
{
/*CMON_MAP_OID                      */    CMON_OID_ETH_LOAD_SL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_CSYS_ETH_STAT,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)csys_eth_stat_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_CSYS_ETH_STAT,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_csys_eth_stat_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_csys_eth_stat_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_csys_eth_stat_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_csys_eth_stat_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_csys_eth_stat_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_TASK_REPORT_RL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_TASK_REPORT_NODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)task_report_node_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_TASK_REPORT_NODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_crank_task_report_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_crank_task_report_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_crank_task_report_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otcr,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_crank_task_report_vec,
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_crank_task_report_vec,
},
{
/*CMON_MAP_OID                      */    CMON_OID_TASK_REPORT_CL,
/*CMON_MAP_DATA_TYPE                */    MM_CVECTOR,
/*CMON_MAP_ITEM_TYPE                */    MM_TASK_REPORT_NODE,
/*CMON_MAP_DATA_INIT_FUNC           */    cmon_obj_init_vec,
/*CMON_MAP_DATA_CLEAN_FUNC          */    cmon_obj_clean_vec,
/*CMON_MAP_DATA_VEC_ITEM_FREE_FUNC  */    (CMON_DATA_VEC_ITEM_FREE_FUNC)task_report_node_free,
/*CMON_MAP_MERGE_VEC_ITEM_TYPE      */    MM_TASK_REPORT_NODE,
/*CMON_MAP_MERGE_INIT_FUNC          */    cmon_obj_merge_vec_init,
/*CMON_MAP_OBJ_MEAS_FUNC            */    cmon_obj_meas_crank_task_report_vec,
/*CMON_MAP_OBJ_PRINT_FUNC           */    cmon_obj_print_crank_task_report_vec,
/*CMON_MAP_OBJ_REPORT_FUNC          */    cmon_obj_report_crank_task_report_vec,
/*CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC */    cmon_obj_merge_match_otc,
/*CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC*/    NULL_PTR,
/*CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC  */    cmon_obj_merge_name_otc,
/*CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC  */    cmon_obj_vec_merge_csys_task_report_vec,/*merge in communicator level*/
/*CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC */    cmon_obj_vec_merge_print_crank_task_report_vec,
},
};

static UINT32 g_cmon_map_len = sizeof(g_cmon_map)/sizeof(g_cmon_map[ 0 ]);


/**
*   for test only
*
*   to query the status of CMON Module
*
**/
void cmon_print_module_status(const UINT32 cmon_md_id, LOG *log)
{
    CMON_MD *cmon_md;
    UINT32 this_cmon_md_id;

    for( this_cmon_md_id = 0; this_cmon_md_id < CMON_MD_CAPACITY(); this_cmon_md_id ++ )
    {
        cmon_md = CMON_MD_GET(this_cmon_md_id);

        if ( NULL_PTR != cmon_md && 0 < cmon_md->usedcounter )
        {
            sys_log(log,"CMON Module # %ld : %ld refered\n",
                    this_cmon_md_id,
                    cmon_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CMON module
*
*
**/
UINT32 cmon_free_module_static_mem(const UINT32 cmon_md_id)
{
    CMON_MD  *cmon_md;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_free_module_static_mem: cmon module #0x%lx not started.\n",
                cmon_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);

    free_module_static_mem(MD_CMON, cmon_md_id);

    return 0;
}

/**
*
* start CMON module
*
**/
UINT32 cmon_start()
{
    CMON_MD *cmon_md;
    UINT32 cmon_md_id;

    cmon_md_id = cbc_md_new(MD_CMON, sizeof(CMON_MD));
    if(ERR_MODULE_ID == cmon_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CMON module */
    cmon_md = (CMON_MD *)cbc_md_get(MD_CMON, cmon_md_id);
    cmon_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    /*default setting which will be override after cmon_set_mod_mgr calling*/
    cmon_md->mod_mgr = mod_mgr_new(cmon_md_id, LOAD_BALANCING_LOOP);

    cmon_md->usedcounter = 1;

    sys_log(LOGSTDOUT, "cmon_start: start CMON module #%ld\n", cmon_md_id);
    //sys_log(LOGSTDOUT, "========================= cmon_start: CMON table info:\n");
    //cmon_print_module_status(cmon_md_id, LOGSTDOUT);
    //cbc_print();

    return ( cmon_md_id );
}

/**
*
* end CMON module
*
**/
void cmon_end(const UINT32 cmon_md_id)
{
    CMON_MD *cmon_md;

    cmon_md = CMON_MD_GET(cmon_md_id);
    if(NULL_PTR == cmon_md)
    {
        sys_log(LOGSTDOUT,"error:cmon_end: cmon_md_id = %ld not exist.\n", cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }

    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < cmon_md->usedcounter )
    {
        cmon_md->usedcounter --;
        return ;
    }

    if ( 0 == cmon_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:cmon_end: cmon_md_id = %ld is not started.\n", cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }

    /* if nobody else occupied the module,then free its resource */

    //task_brd_mod_mgr_rmv(cmon_md->task_brd, cmon_md->mod_mgr);
    mod_mgr_free(cmon_md->mod_mgr);
    cmon_md->mod_mgr  = NULL_PTR;

    /* free module : */
    //cmon_free_module_static_mem(cmon_md_id);

    cmon_md->usedcounter = 0;

    sys_log(LOGSTDOUT, "cmon_end: stop CMON module #%ld\n", cmon_md_id);
    cbc_md_free(MD_CMON, cmon_md_id);

    breathing_static_mem();

    //sys_log(LOGSTDOUT, "========================= cmon_end: CMON table info:\n");
    //cmon_print_module_status(cmon_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

/**
*
* initialize mod mgr of CMON module
*
**/
UINT32 cmon_set_mod_mgr(const UINT32 cmon_md_id, const MOD_MGR * src_mod_mgr)
{
    CMON_MD *cmon_md;
    MOD_MGR  *des_mod_mgr;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_set_mod_mgr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        cmon_print_module_status(cmon_md_id, LOGSTDOUT);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);
    des_mod_mgr = cmon_md->mod_mgr;

    sys_log(LOGSTDOUT, "cmon_set_mod_mgr: md_id %d, input src_mod_mgr %lx\n", cmon_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of node_tcid_vec and node_tcid_vec*/
    mod_mgr_limited_clone(cmon_md_id, src_mod_mgr, des_mod_mgr);

    sys_log(LOGSTDOUT, "====================================cmon_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    sys_log(LOGSTDOUT, "====================================cmon_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

UINT32 cmond_add_mod_mgr(const UINT32 cmon_md_id, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi)
{
    CMON_MD *cmon_md;
    MOD_MGR *mod_mgr;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmond_add_mod_mgr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        cmon_print_module_status(cmon_md_id, LOGSTDOUT);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);
    mod_mgr = cmon_md->mod_mgr;

    mod_mgr_incl(tcid, comm, rank, modi, mod_mgr);

    return (0);
}

/**
*
* get mod mgr of CMON module
*
**/
MOD_MGR * cmon_get_mod_mgr(const UINT32 cmon_md_id)
{
    CMON_MD *cmon_md;

    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        return (MOD_MGR *)0;
    }

    cmon_md = CMON_MD_GET(cmon_md_id);
    return (cmon_md->mod_mgr);
}

/**
*
* check existing of mod_node in mod mgr of current CMON
*
**/
EC_BOOL cmon_check_mod_node_exist(const UINT32 cmon_md_id, const MOD_NODE *mod_node)
{
    CMON_MD *cmon_md;
    UINT32   mod_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_check_mod_node_exist: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);

    mod_node_pos = cvector_search_front(MOD_MGR_REMOTE_LIST(cmon_md->mod_mgr), (void *)mod_node, NULL_PTR);
    if(CVECTOR_ERR_POS == mod_node_pos)
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

/**
*
* fetch CMON_MAP from g_cmon_map
*
**/
CMON_MAP *cmon_map_fetch(const UINT32 cmon_md_id, const UINT32 cmon_oid)
{
    CMON_MAP *cmon_map;
    UINT32    pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_map_fetch: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(cmon_oid >= g_cmon_map_len)
    {
        return (NULL_PTR);
    }

    cmon_map = &(g_cmon_map[ cmon_oid ]);
    if(cmon_oid == CMON_MAP_OID(cmon_map))
    {
        return (cmon_map);
    }

    /*when g_cmon_map is disordered,then searching whole table*/
    for(pos = 0; pos < g_cmon_map_len; pos ++)
    {
        cmon_map = &(g_cmon_map[ pos ]);
        if(cmon_oid == CMON_MAP_OID(cmon_map))
        {
            return (cmon_map);
        }
    }

    sys_log(LOGSTDOUT, "error:cmon_map_fetch: undefined cmon_oid %ld\n", cmon_oid);
    return (NULL_PTR);
}

/**
*
* initialize a CMON_OBJ with undefined data type
*
* note: this function is used by TYPE_CONV_ITEM
*
**/
UINT32 cmon_obj_init_undef(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_OBJ_MAP(cmon_obj)       = NULL_PTR;
    CMON_OBJ_MOD_NODE(cmon_obj)  = NULL_PTR;
    return (0);
}

/**
*
* initialize a CMON_DATA
*
**/
static UINT32 cmon_obj_init_uint32(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_init_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_DATA_UINT32(cmon_obj, data_area) = 0;
    return (0);
}

static UINT32 cmon_obj_init_real(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_init_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_DATA_REAL(cmon_obj, data_area)= 0.0;
    return (0);
}

static UINT32 cmon_obj_init_vec(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_data_init_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cvector_init(CMON_OBJ_DATA_VEC(cmon_obj, data_area  ), 0,  CMON_OBJ_ITEM_TYPE(cmon_obj), CVECTOR_LOCK_ENABLE, LOC_CMON_0001);
    return (0);
}

static UINT32 cmon_obj_init_crank_thread_stat(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_init_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    crank_thread_stat_init(CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, data_area));
    return (0);
}

/**
*
* clean a CMON_OBJ with undefined data type
*
*
**/
UINT32 cmon_obj_clean_undef(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_OBJ_MAP(cmon_obj) = NULL_PTR;
    return (0);
}

/**
*
* clean a CMON_OBJ with both measurement data and report data
*
*
**/
UINT32 cmon_obj_clean_0(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    if(NULL_PTR != CMON_OBJ_MAP(cmon_obj))
    {
        cmon_obj_clean(cmon_md_id, CMON_MEAS_DATA_AREA, cmon_obj);
        cmon_obj_clean(cmon_md_id, CMON_REPORT_DATA_AREA, cmon_obj);
        cmon_obj_clean(cmon_md_id, CMON_CACHE_DATA_AREA, cmon_obj);
    }
    else
    {
        sys_log(LOGSTDOUT, "warn:cmon_obj_clean_0: cmon_obj %lx: CMON_OBJ_MAP is null\n", cmon_obj);
    }
    return (0);
}

/**
*
* clean a CMON_OBJ
*
**/

static UINT32 cmon_obj_clean_uint32(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_clean_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*clean up data area*/
    CMON_OBJ_DATA_UINT32(cmon_obj, data_area) = 0;
    return (0);
}

static UINT32 cmon_obj_clean_real(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_clean_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*clean up data area*/
    CMON_OBJ_DATA_REAL(cmon_obj, data_area) = 0.0;

    return (0);
}

static UINT32 cmon_obj_clean_vec(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_clean_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*clean up data area*/
    cmon_map = CMON_OBJ_MAP(cmon_obj);

    cvector_clean(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (CVECTOR_DATA_CLEANER)CMON_MAP_DATA_VEC_ITEM_FREE_FUNC(cmon_map), LOC_CMON_0002);

    return (0);
}

static UINT32 cmon_obj_clean_crank_thread_stat(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_clean_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*clean up data area*/
    crank_thread_stat_clean(CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, data_area));

    return (0);
}

/**
*
* new a CMON_OBJ
*
**/
CMON_OBJ * cmon_obj_new(const UINT32 cmon_md_id, const UINT32 cmon_oid)
{
    CMON_MAP * cmon_map;
    CMON_OBJ * cmon_obj;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_new: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = cmon_map_fetch(cmon_md_id, cmon_oid);
    if(NULL_PTR == cmon_map)
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_new: cmon_md_id = %ld, invalid cmon_oid %ld\n", cmon_md_id, cmon_oid);
        return (NULL_PTR);
    }

    alloc_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ, &cmon_obj, LOC_CMON_0003);
    CMON_OBJ_MAP(cmon_obj) = cmon_map;
    cmon_obj_init(cmon_md_id, cmon_obj);

    return (cmon_obj);
}

/**
*
* initialize a CMON_OBJ
*
* note: CMON_OBJ_MAP must be set before calling
*
**/
UINT32 cmon_obj_init(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MAP(cmon_obj);

    CMON_MAP_DATA_INIT_FUNC(cmon_map)(cmon_md_id, CMON_MEAS_DATA_AREA  , cmon_obj);
    CMON_MAP_DATA_INIT_FUNC(cmon_map)(cmon_md_id, CMON_REPORT_DATA_AREA, cmon_obj);
    CMON_MAP_DATA_INIT_FUNC(cmon_map)(cmon_md_id, CMON_CACHE_DATA_AREA, cmon_obj);

    return (0);
}

/**
*
* clean a CMON_OBJ
*
**/
UINT32 cmon_obj_clean(const UINT32 cmon_md_id, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_clean: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MAP(cmon_obj);
    return CMON_MAP_DATA_CLEAN_FUNC(cmon_map)(cmon_md_id, data_area, cmon_obj);
}

/**
*
* free a CMON_OBJ
*
**/
UINT32 cmon_obj_free(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_free: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_clean(cmon_md_id, CMON_MEAS_DATA_AREA  , cmon_obj);
    cmon_obj_clean(cmon_md_id, CMON_REPORT_DATA_AREA, cmon_obj);
    cmon_obj_clean(cmon_md_id, CMON_CACHE_DATA_AREA, cmon_obj);

    free_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ, cmon_obj, LOC_CMON_0004);
    return (0);
}

/**
*
* set UINT32 value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_uint32(const UINT32 cmon_md_id, const UINT32 data, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_set_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_uint32: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_UINT32 != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_uint32: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    CMON_OBJ_DATA_UINT32(cmon_obj, data_area) = data;

    return (0);
}

/**
*
* set REAL value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_real(const UINT32 cmon_md_id, const REAL * data, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_set_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_real: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_REAL != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_real: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    CMON_OBJ_DATA_REAL(cmon_obj, data_area) = (*data);

    return (0);
}

/**
*
* set CRANK_THREAD_STAT value a CMON_OBJ
*
**/
UINT32 cmon_obj_set_crank_thread_stat(const UINT32 cmon_md_id, const CRANK_THREAD_STAT * crank_thread_stat, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
    CRANK_THREAD_STAT *val_crank_thread_stat;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_set_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_crank_thread_stat: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CRANK_THREAD_STAT != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_set_crank_thread_stat: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    val_crank_thread_stat = CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, data_area);
    CRANK_THREAD_MAX_NUM(val_crank_thread_stat)  = CRANK_THREAD_MAX_NUM(crank_thread_stat);
    CRANK_THREAD_BUSY_NUM(val_crank_thread_stat) = CRANK_THREAD_BUSY_NUM(crank_thread_stat);
    CRANK_THREAD_IDLE_NUM(val_crank_thread_stat) = CRANK_THREAD_IDLE_NUM(crank_thread_stat);

    return (0);
}


/**
*
* push UINT32 value a CMON_OBJ
*
**/
UINT32 cmon_obj_push_uint32(const UINT32 cmon_md_id, const UINT32 data, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_uint32: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_uint32: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_UINT32 != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_uint32: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)data);

    return (0);
}


/**
*
* push REAL to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_real(const UINT32 cmon_md_id, const REAL * data, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_real: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_real: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_REAL != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_real: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)data);

    return (0);
}

/**
*
* push CSYS_CPU_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_cpu_stat(const UINT32 cmon_md_id, const CSYS_CPU_STAT * csys_cpu_stat, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_csys_cpu_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_cpu_stat: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_cpu_stat: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_CSYS_CPU_STAT != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_cpu_stat: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)csys_cpu_stat);

    return (0);
}

/**
*
* push TASKC_NODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_taskc_node(const UINT32 cmon_md_id, const TASKC_NODE * taskc_node, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_taskc_node: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_taskc_node: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_taskc_node: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_TASKC_NODE != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_taskc_node: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)taskc_node);

    return (0);
}

/**
*
* push CSOCKET_CNODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csocket_cnode(const UINT32 cmon_md_id, const CSOCKET_CNODE * csocket_cnode, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_csocket_cnode: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csocket_cnode: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csocket_cnode: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_CSOCKET_CNODE != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csocket_cnode: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)csocket_cnode);

    return (0);
}

/**
*
* push CPROC_MODULE_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_cproc_module_stat(const UINT32 cmon_md_id, const CPROC_MODULE_STAT * cproc_module_stat, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_cproc_module_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_cproc_module_stat: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_cproc_module_stat: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_CPROC_MODULE_STAT != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_cproc_module_stat: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)cproc_module_stat);

    return (0);
}

/**
*
* push CSYS_ETH_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_eth_stat(const UINT32 cmon_md_id, const CSYS_ETH_STAT * csys_eth_stat, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_csys_eth_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_eth_stat: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_eth_stat: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_CSYS_ETH_STAT != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_eth_stat: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)csys_eth_stat);

    return (0);
}

/**
*
* push CSYS_DSK_STAT to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_csys_dsk_stat(const UINT32 cmon_md_id, const CSYS_DSK_STAT * csys_dsk_stat, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_csys_dsk_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_dsk_stat: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_dsk_stat: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_CSYS_DSK_STAT != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_csys_dsk_stat: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)csys_dsk_stat);

    return (0);
}


/**
*
* push TASK_REPORT_NODE to a CMON_OBJ
*
**/
UINT32 cmon_obj_push_crank_task_report_node(const UINT32 cmon_md_id, const TASK_REPORT_NODE * task_report_node, const UINT32 data_area, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_push_crank_task_report_node: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_crank_task_report_node: cmon_md_id = %ld, not initialize cmon_map\n", cmon_md_id);
        return ((UINT32)-1);
    }

    if(MM_CVECTOR != CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_crank_task_report_node: cmon_md_id = %ld, invalid data type %ld where cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    if(MM_TASK_REPORT_NODE != CMON_OBJ_ITEM_TYPE(cmon_obj))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_push_crank_task_report_node: cmon_md_id = %ld, invalid item type where data type %ld and cmon_oid %ld\n",
                           cmon_md_id, CMON_OBJ_ITEM_TYPE(cmon_obj), CMON_OBJ_DATA_TYPE(cmon_obj), CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, data_area), (void *)task_report_node);

    return (0);
}

/**
*
* check cmon_obj is connected or not yet
*
**/
EC_BOOL cmon_obj_is_connected(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_MD *cmon_md;
    UINT32   mod_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_is_connected: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);

    mod_node_pos = cvector_search_front(MOD_MGR_REMOTE_LIST(cmon_md->mod_mgr), CMON_OBJ_MOD_NODE(cmon_obj), NULL_PTR);
    if(CVECTOR_ERR_POS == mod_node_pos)
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}


/**
*
* CMON_OBJ measure and collect info according to cmon_oid
*
**/
UINT32 cmon_obj_meas(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MAP(cmon_obj);
    //sys_log(LOGSTDOUT, ">>> cmon_obj_meas was called\n");

    return CMON_MAP_OBJ_MEAS_FUNC(cmon_map)(cmon_md_id, cmon_obj);
}

UINT32 cmon_obj_meas_default(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    sys_log(LOGSTDOUT, "warn: measurement function of cmon_oid %ld was not implemented yet\n", CMON_OBJ_OID(cmon_obj));
    return ((UINT32)-1);
}

UINT32 cmon_obj_meas_sys_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_MEM_STAT *csys_mem_stat;
    UINT32 mem_occupy;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_sys_mem_occupy: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_mem_stat = csys_mem_stat_new();
    csys_mem_stat_get(csys_mem_stat);

    mem_occupy = CSYS_MEM_TOTAL(csys_mem_stat) - CSYS_MEM_FREE(csys_mem_stat);
    CMON_DBG((LOGSTDOUT, "cmon_obj_meas_sys_mem_occupy: %ld - %ld => %ld\n", CSYS_MEM_TOTAL(csys_mem_stat), CSYS_MEM_FREE(csys_mem_stat), mem_occupy));

    csys_mem_stat_free(csys_mem_stat);

    return cmon_obj_set_uint32(cmon_md_id, mem_occupy, CMON_MEAS_DATA_AREA, cmon_obj);
}

UINT32 cmon_obj_meas_proc_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CPROC_MEM_STAT *cproc_mem_stat;
    UINT32 mem_occupy;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_proc_mem_occupy: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cproc_mem_stat = cproc_mem_stat_new();
    cproc_mem_stat_get(cproc_mem_stat);

    mem_occupy = CPROC_MEM_OCCUPY(cproc_mem_stat);
    CMON_DBG((LOGSTDOUT, "cmon_obj_meas_proc_mem_occupy: mem_occupy = %ld\n", mem_occupy));
    cproc_mem_stat_free(cproc_mem_stat);

    return cmon_obj_set_uint32(cmon_md_id, mem_occupy, CMON_MEAS_DATA_AREA, cmon_obj);
}

UINT32 cmon_obj_meas_sys_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_MEM_STAT *csys_mem_stat;
    REAL mem_load;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_sys_mem_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_mem_stat = csys_mem_stat_new();
    csys_mem_stat_get(csys_mem_stat);

    mem_load = 100.0 *(1.0 * (CSYS_MEM_TOTAL(csys_mem_stat) - CSYS_MEM_FREE(csys_mem_stat)) / CSYS_MEM_TOTAL(csys_mem_stat));
    CMON_DBG((LOGSTDOUT, "cmon_obj_meas_sys_mem_load: (%ld - %ld) / %ld => %.1f\n", CSYS_MEM_TOTAL(csys_mem_stat), CSYS_MEM_FREE(csys_mem_stat), CSYS_MEM_TOTAL(csys_mem_stat), mem_load));
    csys_mem_stat_free(csys_mem_stat);

    return cmon_obj_set_real(cmon_md_id, &mem_load, CMON_MEAS_DATA_AREA, cmon_obj);
}

UINT32 cmon_obj_meas_proc_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CPROC_MEM_STAT *cproc_mem_stat;
    REAL mem_load;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_proc_mem_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cproc_mem_stat = cproc_mem_stat_new();

    cproc_mem_stat_get(cproc_mem_stat);

    mem_load = CPROC_MEM_LOAD(cproc_mem_stat);

    cproc_mem_stat_free(cproc_mem_stat);

    return cmon_obj_set_real(cmon_md_id, &mem_load, CMON_MEAS_DATA_AREA, cmon_obj);
}


UINT32 cmon_obj_meas_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    TASK_BRD  *task_brd;
    TASKC_MGR *taskc_mgr;

    CLIST *    taskc_node_list;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_taskc_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    taskc_mgr = taskc_mgr_new();

    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        task_brd_sync_taskc_mgr(task_brd, taskc_mgr);
    }
    else
    {
        super_sync_taskc_mgr(0, taskc_mgr);
    }

    taskc_node_list = TASKC_MGR_NODE_LIST(taskc_mgr);
    while(EC_FALSE == clist_is_empty(taskc_node_list))
    {
        TASKC_NODE *taskc_node;

        taskc_node = (TASKC_NODE *)clist_pop_front(taskc_node_list);
        cmon_obj_push_taskc_node(cmon_md_id, taskc_node, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    taskc_mgr_free(taskc_mgr);

    return (0);
}

UINT32 cmon_obj_meas_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    TASK_BRD  *task_brd;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_csocket_cnode_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    if(CMPI_FWD_RANK != TASK_BRD_RANK(task_brd))
    {
        CMON_MD *cmon_md;
        MOD_MGR *mod_mgr;

        MOD_NODE recv_mod_node;
        UINT32   ret;

        cmon_md = CMON_MD_GET(cmon_md_id);
        mod_mgr = cmon_md->mod_mgr;

        recv_mod_node.tcid = TASK_BRD_TCID(task_brd);
        recv_mod_node.comm = TASK_BRD_COMM(task_brd);
        recv_mod_node.rank = CMPI_FWD_RANK;
        recv_mod_node.modi = 0;
        recv_mod_node.load = 0;

        task_super_mono(mod_mgr, TASK_DEFAULT_LIVE, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP,
                        &recv_mod_node,
                        &ret, FI_cmon_obj_meas, ERR_MODULE_ID, cmon_obj);
        return (ret);
    }
    else
    {
        TASKS_CFG *tasks_cfg;

        tasks_cfg = TASK_BRD_TASKS_CFG(task_brd);
        tasks_work_export_csocket_cnode_vec(TASKS_CFG_WORK(tasks_cfg), CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA));
    }

    CMON_DBG((LOGSTDOUT, "info:cmon_obj_meas_csocket_cnode_vec: when leave, cvector %lx, type %ld and item type %ld\n",
                        CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA),
                        cvector_type(CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA)),
                        CMON_OBJ_ITEM_TYPE(cmon_obj)));

    return (0);
}

UINT32 cmon_obj_meas_proc_thread_num(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_proc_thread_num: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(0)
    {
        TASK_BRD  *task_brd;
        task_brd = task_brd_default_get();
        cmon_obj_set_uint32(cmon_md_id, TASK_BRD_THREAD_NUM(task_brd), CMON_MEAS_DATA_AREA, cmon_obj);
    }

    /**
    * alternative way to check the thread num is by shell command
    *   cat /proc/${PID}/stat | awk '{print $20}'
    **/
    if(1)
    {
        CPROC_THREAD_STAT cproc_thread_stat;
        cproc_thread_stat_get(&cproc_thread_stat);
        cmon_obj_set_uint32(cmon_md_id, CPROC_THREAD_NUM(&cproc_thread_stat), CMON_MEAS_DATA_AREA, cmon_obj);
    }

    return (0);
}

UINT32 cmon_obj_meas_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CRANK_THREAD_STAT crank_thread_stat;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    crank_thread_stat_get(&crank_thread_stat);
    cmon_obj_set_crank_thread_stat(cmon_md_id, &crank_thread_stat, CMON_MEAS_DATA_AREA, cmon_obj);

    return (0);
}

UINT32 cmon_obj_meas_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_DSK_VEC *csys_dsk_stat_vec;

    UINT32 csys_dsk_stat_num;
    UINT32 csys_dsk_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_csys_dsk_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_dsk_stat_vec = csys_dsk_stat_vec_new();
    csys_dsk_stat_vec_get(csys_dsk_stat_vec);

    csys_dsk_stat_num = csys_dsk_stat_vec_size(csys_dsk_stat_vec);
    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < csys_dsk_stat_num; csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat;

        csys_dsk_stat = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec, csys_dsk_stat_pos);
        cvector_set(csys_dsk_stat_vec, csys_dsk_stat_pos, (void *)NULL_PTR);

        cmon_obj_push_csys_dsk_stat(cmon_md_id, csys_dsk_stat, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    csys_dsk_stat_vec_free(csys_dsk_stat_vec);

    return (0);
}

UINT32 cmon_obj_meas_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CRANK_TASK_REPORT_VEC *crank_task_report_vec;

    UINT32 crank_task_report_node_num;
    UINT32 crank_task_report_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_crank_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    crank_task_report_vec = crank_task_report_vec_new();
    crank_task_report_vec_get(crank_task_report_vec);

    crank_task_report_node_num = crank_task_report_vec_size(crank_task_report_vec);
    for(crank_task_report_node_pos = 0; crank_task_report_node_pos < crank_task_report_node_num; crank_task_report_node_pos ++)
    {
        TASK_REPORT_NODE *crank_task_report_node;

        crank_task_report_node = (TASK_REPORT_NODE *)cvector_get(crank_task_report_vec, crank_task_report_node_pos);
        cvector_set(crank_task_report_vec, crank_task_report_node_pos, (void *)NULL_PTR);

        cmon_obj_push_crank_task_report_node(cmon_md_id, crank_task_report_node, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    crank_task_report_vec_free(crank_task_report_vec);

    return (0);
}

UINT32 cmon_obj_meas_proc_cpu_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CPROC_CPU_STAT *cproc_cpu_stat;
    REAL cpu_load;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_proc_cpu_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cproc_cpu_stat = cproc_cpu_stat_new();

    cproc_cpu_stat_get(cproc_cpu_stat);

    cpu_load = CPROC_CPU_LOAD(cproc_cpu_stat);

    cproc_cpu_stat_free(cproc_cpu_stat);

    return cmon_obj_set_real(cmon_md_id, &cpu_load, CMON_MEAS_DATA_AREA, cmon_obj);
}

UINT32 cmon_obj_meas_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_CPU_STAT_VEC *csys_cpu_stat_vec;

    UINT32 csys_cpu_stat_num;
    UINT32 csys_cpu_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    sys_log(LOGSTDOUT, "[DEBUG] cmon_obj_meas_cpu_vec was called\n");

    csys_cpu_stat_vec = csys_cpu_stat_vec_new();
    csys_cpu_stat_vec_get(csys_cpu_stat_vec);

    csys_cpu_stat_num = csys_cpu_stat_vec_size(csys_cpu_stat_vec);
    sys_log(LOGSTDOUT, "[DEBUG] cmon_obj_meas_cpu_vec: csys_cpu_stat_num %ld\n", csys_cpu_stat_num);
    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat;

        csys_cpu_stat = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);
        cvector_set(csys_cpu_stat_vec, csys_cpu_stat_pos, (void *)NULL_PTR);

        cmon_obj_push_csys_cpu_stat(cmon_md_id, csys_cpu_stat, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    csys_cpu_stat_vec_free(csys_cpu_stat_vec);

    return (0);
}

/**
*
*   note: this function measure the average cpu load of the whole system
*         which means the load info is not that of each cpu
*
*         if need measure each cpu load, use CACHE DATA AREA of cmon_data, and compute load with the reporting and cached datas
*
**/
UINT32 cmon_obj_meas_sys_cpu_avg_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_CPU_AVG_STAT *csys_cpu_avg_stat;

    REAL   cpu_avg_load;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_sys_cpu_avg_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_cpu_avg_stat = csys_cpu_avg_stat_new();
    csys_cpu_avg_stat_get(csys_cpu_avg_stat);

    cpu_avg_load = CSYS_CPU_AVG_STAT_01_MIN(csys_cpu_avg_stat);/*okay, current implementation is to repport 1min result*/
    //sys_log(LOGSTDOUT, "info:cmon_obj_meas_sys_cpu_avg_load: cpu_avg_load = %.2f\n", cpu_avg_load);

    csys_cpu_avg_stat_free(csys_cpu_avg_stat);

    return cmon_obj_set_real(cmon_md_id, &cpu_avg_load, CMON_MEAS_DATA_AREA, cmon_obj);
}

UINT32 cmon_obj_meas_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    UINT32 type;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_mm_man_occupy_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    for(type = 0; type < MM_END; type ++)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node;

        if(EC_TRUE == mm_man_occupy_node_was_used(type))
        {
            alloc_static_mem(MD_TBD, 0, MM_MM_MAN_OCCUPY_NODE, &mm_man_occupy_node, LOC_CMON_0005);
            mm_man_occupy_node_fetch(type, mm_man_occupy_node);
            cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA), (void *)mm_man_occupy_node);
        }
    }

    CMON_DBG((LOGSTDOUT, "info:cmon_obj_meas_mm_man_occupy_node_vec: when leave, cvector %lx, type %ld and item type %ld\n",
                        CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA),
                        cvector_type(CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA)),
                        CMON_OBJ_ITEM_TYPE(cmon_obj)));

    return (0);
}

UINT32 cmon_obj_meas_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    UINT32 type;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_mm_man_load_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    for(type = 0; type < MM_END; type ++)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node;

        if(EC_TRUE == mm_man_load_node_was_used(type))
        {
            alloc_static_mem(MD_TBD, 0, MM_MM_MAN_LOAD_NODE, &mm_man_load_node, LOC_CMON_0006);
            mm_man_load_node_fetch(type, mm_man_load_node);
            cvector_push(CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA), (void *)mm_man_load_node);
        }
    }

    CMON_DBG((LOGSTDOUT, "info:cmon_obj_meas_mm_man_load_node_vec: when leave, cvector %lx, type %ld and item type %ld\n",
                        CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA),
                        cvector_type(CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA)),
                        CMON_OBJ_ITEM_TYPE(cmon_obj)));

    return (0);
}

/**
*
*   note: this function measure the specific module num of each process
*
**/
UINT32 cmon_obj_meas_proc_module_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CPROC_MODULE_STAT_VEC *cproc_module_stat_vec;

    UINT32 cproc_module_stat_pos;
    UINT32 cproc_module_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_proc_module_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    //sys_log(LOGSTDOUT, "[DEBUG] cmon_obj_meas_proc_module_vec was called\n");

    cproc_module_stat_vec = cproc_module_stat_vec_new();
    cproc_module_stat_vec_get(cproc_module_stat_vec);

    cproc_module_stat_num = cproc_module_stat_vec_size(cproc_module_stat_vec);
    for(cproc_module_stat_pos = 0; cproc_module_stat_pos < cproc_module_stat_num; cproc_module_stat_pos ++)
    {
        CPROC_MODULE_STAT *cproc_module_stat;

        cproc_module_stat = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec, cproc_module_stat_pos);
        cvector_set(cproc_module_stat_vec, cproc_module_stat_pos, (void *)NULL_PTR);

        cmon_obj_push_cproc_module_stat(cmon_md_id, cproc_module_stat, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    cproc_module_stat_vec_free(cproc_module_stat_vec);

    return (0);
}

UINT32 cmon_obj_meas_crank_task_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_crank_task_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(1)
    {
        TASK_BRD  *task_brd;
        task_brd = task_brd_default_get();
        cmon_obj_set_uint32(cmon_md_id, task_brd_que_load(task_brd), CMON_MEAS_DATA_AREA, cmon_obj);
    }

    return (0);
}

UINT32 cmon_obj_meas_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CSYS_ETH_VEC *csys_eth_stat_vec;

    UINT32 csys_eth_stat_num;
    UINT32 csys_eth_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_meas_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_eth_stat_vec = csys_eth_stat_vec_new();
    csys_eth_stat_vec_get(csys_eth_stat_vec);

    csys_eth_stat_num = csys_eth_stat_vec_size(csys_eth_stat_vec);
    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat;

        csys_eth_stat = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec, csys_eth_stat_pos);
        cvector_set(csys_eth_stat_vec, csys_eth_stat_pos, (void *)NULL_PTR);

        cmon_obj_push_csys_eth_stat(cmon_md_id, csys_eth_stat, CMON_MEAS_DATA_AREA, cmon_obj);
    }

    csys_eth_stat_vec_free(csys_eth_stat_vec);

    return (0);
}


/**
*
* CMON_OBJ print according to cmon_oid
*
**/
UINT32 cmon_obj_print(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MAP(cmon_obj);

    return CMON_MAP_OBJ_PRINT_FUNC(cmon_map)(cmon_md_id, log, cmon_obj);
}

UINT32 cmon_obj_print_default(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    sys_log(log, "warn: print function of cmon_oid %ld was not implemented yet\n", CMON_OBJ_OID(cmon_obj));
    return ((UINT32)-1);
}

UINT32 cmon_obj_print_mem_occupy(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    UINT32 mem_occupy;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_mem_occupy: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    mem_occupy = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[mem occupy] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): occupy %ld KB\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       mem_occupy
           );
    return (0);
}

UINT32 cmon_obj_print_mem_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    REAL mem_load;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_mem_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    mem_load = CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[mem load] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): load %.2f %%\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       mem_load
           );
    return (0);
}

UINT32 cmon_obj_print_taskc_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *taskc_node_vec;
    MOD_NODE *mod_node;

    UINT32 taskc_node_pos;
    UINT32 taskc_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_taskc_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    taskc_node_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    taskc_node_num = cvector_size(taskc_node_vec);

    for(taskc_node_pos = 0; taskc_node_pos < taskc_node_num; taskc_node_pos ++)
    {
        TASKC_NODE *taskc_node;

        taskc_node = (TASKC_NODE *)cvector_get(taskc_node_vec, taskc_node_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[taskc node vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # taskc_id %s, taskc_comm %ld, taskc_size %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               taskc_node_pos,
                               TASKC_NODE_TCID_STR(taskc_node), TASKC_NODE_COMM(taskc_node), TASKC_NODE_SIZE(taskc_node)
                   );
        }
        else
        {
            sys_log(log, "[taskc node vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # taskc_id %s, taskc_comm %ld, taskc_size %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               taskc_node_pos,
                               TASKC_NODE_TCID_STR(taskc_node), TASKC_NODE_COMM(taskc_node), TASKC_NODE_SIZE(taskc_node)
                   );
        }
    }


    return (0);
}

UINT32 cmon_obj_print_csocket_cnode_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csocket_cnode_vec;
    MOD_NODE *mod_node;

    UINT32 csocket_cnode_pos;
    UINT32 csocket_cnode_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_csocket_cnode_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    csocket_cnode_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csocket_cnode_num = cvector_size(csocket_cnode_vec);
    for(csocket_cnode_pos = 0; csocket_cnode_pos < csocket_cnode_num; csocket_cnode_pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get(csocket_cnode_vec, csocket_cnode_pos);

        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[csocket cnode vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # ipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd %d\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               csocket_cnode_pos,
                               CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode),
                               CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                               CSOCKET_CNODE_SOCKFD(csocket_cnode)
                   );
        }
        else
        {
            sys_log(log, "[csocket cnode vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # ipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd %d\n",
                               CMON_OBJ_OID(cmon_obj),
                               csocket_cnode_pos,
                               CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode),
                               CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                               CSOCKET_CNODE_SOCKFD(csocket_cnode)
                   );
        }
    }


    return (0);
}

UINT32 cmon_obj_print_thread_num(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    UINT32 thread_num;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_thread_num: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    thread_num = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[thread num] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): thread num %ld\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       thread_num
           );
    return (0);
}

UINT32 cmon_obj_print_crank_thread_stat(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CRANK_THREAD_STAT *crank_thread_stat;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    crank_thread_stat = CMON_OBJ_DATA_CRANK_THREAD_STAT((CMON_OBJ *)cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[thread num] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): thread stat: max %ld, busy %ld, idle %ld\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       CRANK_THREAD_MAX_NUM(crank_thread_stat), CRANK_THREAD_BUSY_NUM(crank_thread_stat), CRANK_THREAD_IDLE_NUM(crank_thread_stat)
           );
    return (0);
}

UINT32 cmon_obj_print_cpu_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    REAL cpu_load;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_cpu_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    cpu_load = CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[cpu load] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): load %.2f %%\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       cpu_load
           );
    return (0);
}

UINT32 cmon_obj_print_cpu_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_cpu_stat_vec;
    MOD_NODE *mod_node;

    UINT32 csys_cpu_stat_pos;
    UINT32 csys_cpu_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    csys_cpu_stat_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec);

    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat;

        csys_cpu_stat = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[cpu vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # %s, user %ld, nice %ld, sys %ld, idle %ld, load %.2f\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               csys_cpu_stat_pos,
                               (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat)),
                               CSYS_CPU_STAT_USER(csys_cpu_stat),CSYS_CPU_STAT_NICE(csys_cpu_stat),CSYS_CPU_STAT_SYS(csys_cpu_stat),
                               CSYS_CPU_STAT_IDLE(csys_cpu_stat),CSYS_CPU_STAT_LOAD(csys_cpu_stat)
                   );
        }
        else
        {
            sys_log(log, "[cpu vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # %s, user %ld, nice %ld, sys %ld, idle %ld, load %.2f\n",
                               CMON_OBJ_OID(cmon_obj),
                               csys_cpu_stat_pos,
                               (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat)),
                               CSYS_CPU_STAT_USER(csys_cpu_stat),CSYS_CPU_STAT_NICE(csys_cpu_stat),CSYS_CPU_STAT_SYS(csys_cpu_stat),
                               CSYS_CPU_STAT_IDLE(csys_cpu_stat),CSYS_CPU_STAT_LOAD(csys_cpu_stat)
                   );
        }
    }

    return (0);
}

UINT32 cmon_obj_print_mm_man_occupy_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *mm_man_occupy_node_vec;
    MOD_NODE *mod_node;

    UINT32 mm_man_occupy_node_pos;
    UINT32 mm_man_occupy_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_mm_man_occupy_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    mm_man_occupy_node_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    mm_man_occupy_node_num = cvector_size(mm_man_occupy_node_vec);
    for(mm_man_occupy_node_pos = 0; mm_man_occupy_node_pos < mm_man_occupy_node_num; mm_man_occupy_node_pos ++)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node;

        mm_man_occupy_node = (MM_MAN_OCCUPY_NODE *)cvector_get(mm_man_occupy_node_vec, mm_man_occupy_node_pos);

        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[mm man node] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld) manager %4ld: nodesum = %8ld, maxused = %8ld, curused = %8ld\n",
                           CMON_OBJ_OID(cmon_obj),
                           MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                           MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)
                   );
        }
        else
        {
            sys_log(log, "[manager %4ld] oid %ld, (tcid NA, comm NA, rank NA, modi NA) manager %4ld: nodesum = %8ld, maxused = %8ld, curused = %8ld\n",
                           CMON_OBJ_OID(cmon_obj),
                           MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node),
                           MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)
                   );
        }
    }


    return (0);
}

UINT32 cmon_obj_print_mm_man_load_node_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *mm_man_load_node_vec;
    MOD_NODE *mod_node;

    UINT32 mm_man_load_node_pos;
    UINT32 mm_man_load_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_mm_man_load_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    mm_man_load_node_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    mm_man_load_node_num = cvector_size(mm_man_load_node_vec);
    for(mm_man_load_node_pos = 0; mm_man_load_node_pos < mm_man_load_node_num; mm_man_load_node_pos ++)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node;

        mm_man_load_node = (MM_MAN_LOAD_NODE *)cvector_get(mm_man_load_node_vec, mm_man_load_node_pos);

        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[mm man node] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld) manager %4ld: maxused load = %.2f, curused load = %.2f\n",
                           CMON_OBJ_OID(cmon_obj),
                           MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                           MM_MAN_LOAD_NODE_TYPE(mm_man_load_node),
                           MM_MAN_LOAD_NODE_MAX(mm_man_load_node),
                           MM_MAN_LOAD_NODE_CUR(mm_man_load_node)
                   );
        }
        else
        {
            sys_log(log, "[manager %4ld] oid %ld, (tcid NA, comm NA, rank NA, modi NA) manager %4ld: maxused load = %.2f, curused load = %.2f\n",
                           CMON_OBJ_OID(cmon_obj),
                           MM_MAN_LOAD_NODE_TYPE(mm_man_load_node),
                           MM_MAN_LOAD_NODE_MAX(mm_man_load_node),
                           MM_MAN_LOAD_NODE_CUR(mm_man_load_node)
                   );
        }
    }


    return (0);
}


UINT32 cmon_obj_print_module_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *cproc_module_stat_vec;
    MOD_NODE *mod_node;

    UINT32 cproc_module_stat_pos;
    UINT32 cproc_module_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_module_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    cproc_module_stat_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    cproc_module_stat_num = cvector_size(cproc_module_stat_vec);

    for(cproc_module_stat_pos = 0; cproc_module_stat_pos < cproc_module_stat_num; cproc_module_stat_pos ++)
    {
        CPROC_MODULE_STAT *cproc_module_stat;

        cproc_module_stat = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec, cproc_module_stat_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[module vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # type %ld: module num %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               cproc_module_stat_pos,
                               CPROC_MODULE_TYPE(cproc_module_stat), CPROC_MODULE_NUM(cproc_module_stat)
                   );
        }
        else
        {
            sys_log(log, "[module vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # type %ld: module num %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               cproc_module_stat_pos,
                               CPROC_MODULE_TYPE(cproc_module_stat), CPROC_MODULE_NUM(cproc_module_stat)
                   );
        }
    }

    return (0);
}

UINT32 cmon_obj_print_crank_task_load(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    UINT32 task_load;
    MOD_NODE *mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_crank_task_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    task_load = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    sys_log(log, "[thread num] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): task load %ld\n",
                       CMON_OBJ_OID(cmon_obj),
                       MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                       task_load
           );
    return (0);
}

UINT32 cmon_obj_print_csys_eth_stat_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_eth_stat_vec;
    MOD_NODE *mod_node;

    UINT32 csys_eth_stat_pos;
    UINT32 csys_eth_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    csys_eth_stat_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_eth_stat_num = cvector_size(csys_eth_stat_vec);

    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat;

        csys_eth_stat = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec, csys_eth_stat_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[eth vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # %s, speed %ld Mb/s, rxmoct %ld MBytes, txmoct %ld MBytes\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               csys_eth_stat_pos,
                               (char *)cstring_get_str(CSYS_ETH_NAME(csys_eth_stat)),
                               CSYS_ETH_SPEEDMBS(csys_eth_stat),
                               CSYS_ETH_RXMOCT(csys_eth_stat),
                               CSYS_ETH_TXMOCT(csys_eth_stat)
                   );
        }
        else
        {
            sys_log(log, "[eth vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # %s, speed %ld Mb/s, rxmoct %ld MBytes, txmoct %ld MBytes\n",
                               CMON_OBJ_OID(cmon_obj),
                               csys_eth_stat_pos,
                               (char *)cstring_get_str(CSYS_ETH_NAME(csys_eth_stat)),
                               CSYS_ETH_SPEEDMBS(csys_eth_stat),
                               CSYS_ETH_RXMOCT(csys_eth_stat),
                               CSYS_ETH_TXMOCT(csys_eth_stat)
                   );
        }
    }

    return (0);
}

UINT32 cmon_obj_print_csys_dsk_stat_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_dsk_stat_vec;
    MOD_NODE *mod_node;

    UINT32 csys_dsk_stat_pos;
    UINT32 csys_dsk_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_csys_dsk_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    csys_dsk_stat_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_dsk_stat_num = cvector_size(csys_dsk_stat_vec);

    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < csys_dsk_stat_num; csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat;

        csys_dsk_stat = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec, csys_dsk_stat_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[dsk vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # %s, size %ld MBytes, used %ld MBytes, aval %ld MBytes, load %.2f%%\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               csys_dsk_stat_pos,
                               (char *)cstring_get_str(CSYS_DSK_NAME(csys_dsk_stat)),
                               CSYS_DSK_SIZE(csys_dsk_stat),
                               CSYS_DSK_USED(csys_dsk_stat),
                               CSYS_DSK_AVAL(csys_dsk_stat),
                               CSYS_DSK_LOAD(csys_dsk_stat)
                   );
        }
        else
        {
            sys_log(log, "[dsk vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # %s, size %ld MBytes, used %ld MBytes, aval %ld MBytes, load %.2f%%\n",
                               CMON_OBJ_OID(cmon_obj),
                               csys_dsk_stat_pos,
                               (char *)cstring_get_str(CSYS_DSK_NAME(csys_dsk_stat)),
                               CSYS_DSK_SIZE(csys_dsk_stat),
                               CSYS_DSK_USED(csys_dsk_stat),
                               CSYS_DSK_AVAL(csys_dsk_stat),
                               CSYS_DSK_LOAD(csys_dsk_stat)
                   );
        }
    }

    return (0);
}

UINT32 cmon_obj_print_crank_task_report_vec(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *crank_task_report_vec;
    MOD_NODE *mod_node;

    UINT32 task_report_node_pos;
    UINT32 task_report_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_print_crank_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

    crank_task_report_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    task_report_node_num = cvector_size(crank_task_report_vec);

    for(task_report_node_pos = 0; task_report_node_pos < task_report_node_num; task_report_node_pos ++)
    {
        TASK_REPORT_NODE *task_report_node;

        task_report_node = (TASK_REPORT_NODE *)cvector_get(crank_task_report_vec, task_report_node_pos);
        if(NULL_PTR != mod_node)
        {
            sys_log(log, "[task report vec] oid %ld, (tcid %s, comm %ld, rank %ld, modi %ld): %ld # start at %4d-%02d-%02d %02d:%02d:%02d, end at %4d-%02d-%02d %02d:%02d:%02d, time to live %ld, seqno %lx.%lx.%lx, wait flag %ld, need rsp flag %ld, reschedule flag %ld, req num %ld, need rsp %ld, succ rsp %ld, fail rsp %ld, sent req %ld, discard req %ld, timeout req %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               MOD_NODE_TCID_STR(mod_node),MOD_NODE_COMM(mod_node),MOD_NODE_RANK(mod_node),MOD_NODE_MODI(mod_node),
                               task_report_node_pos,
                               TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_START_TIME(task_report_node)),
                               TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_START_TIME(task_report_node)),

                               TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_END_TIME(task_report_node)),
                               TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_END_TIME(task_report_node)),

                               TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node),
                               TASK_REPORT_NODE_TCID(task_report_node), TASK_REPORT_NODE_RANK(task_report_node), TASK_REPORT_NODE_SEQNO(task_report_node),

                               TASK_REPORT_NODE_WAIT_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node),

                               TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node)
                   );
        }
        else
        {
            sys_log(log, "[task report vec] oid %ld, (tcid NA, comm NA, rank NA, modi NA): %ld # start at %4d-%02d-%02d %02d:%02d:%02d, end at %4d-%02d-%02d %02d:%02d:%02d, time to live %ld, seqno %lx.%lx.%lx, wait flag %ld, need rsp flag %ld, reschedule flag %ld, req num %ld, need rsp %ld, succ rsp %ld, fail rsp %ld, sent req %ld, discard req %ld, timeout req %ld\n",
                               CMON_OBJ_OID(cmon_obj),
                               task_report_node_pos,
                               TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_START_TIME(task_report_node)),
                               TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_START_TIME(task_report_node)),

                               TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_END_TIME(task_report_node)),
                               TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_END_TIME(task_report_node)),

                               TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node),
                               TASK_REPORT_NODE_TCID(task_report_node), TASK_REPORT_NODE_RANK(task_report_node), TASK_REPORT_NODE_SEQNO(task_report_node),

                               TASK_REPORT_NODE_WAIT_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node),

                               TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node),
                               TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node),
                               TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node)
                   );
        }
    }

    return (0);
}

UINT32 cmon_obj_diff_cpu_vec(const UINT32 cmon_md_id, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_cpu_stat_vec_report;
    CVECTOR  *csys_cpu_stat_vec_cache;

    UINT32 csys_cpu_stat_pos;
    UINT32 csys_cpu_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_diff_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_cpu_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_cpu_stat_vec_cache  = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_CACHE_DATA_AREA);

    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec_report);
    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat_report;
        CSYS_CPU_STAT *csys_cpu_stat_cache;

        UINT32 used_time_report;
        UINT32 total_time_report;

        UINT32 used_time_cache;
        UINT32 total_time_cache;

        csys_cpu_stat_report = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_report, csys_cpu_stat_pos);

        used_time_report  = CSYS_CPU_STAT_USER(csys_cpu_stat_report) + CSYS_CPU_STAT_NICE(csys_cpu_stat_report) + CSYS_CPU_STAT_SYS(csys_cpu_stat_report);
        total_time_report = (used_time_report + CSYS_CPU_STAT_IDLE(csys_cpu_stat_report));

        CMON_DBG((LOGSTDOUT, "[cpu vec: report] oid %ld, cpu %ld # name %s, user %ld, nice %ld, sys %ld, idle %ld, total %ld\n",
                           CMON_OBJ_OID(cmon_obj),
                           csys_cpu_stat_pos,
                           (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat_report)),
                           CSYS_CPU_STAT_USER(csys_cpu_stat_report),CSYS_CPU_STAT_NICE(csys_cpu_stat_report),CSYS_CPU_STAT_SYS(csys_cpu_stat_report),
                           CSYS_CPU_STAT_IDLE(csys_cpu_stat_report),CSYS_CPU_STAT_TOTAL(csys_cpu_stat_report)
               ));

        csys_cpu_stat_cache  = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_cache , csys_cpu_stat_pos);

        if(NULL_PTR == csys_cpu_stat_cache)
        {
            CSYS_CPU_STAT_LOAD(csys_cpu_stat_report) = 100.0 * (used_time_report ) / (total_time_report);
            continue;
        }

        CMON_DBG((LOGSTDOUT, "[cpu vec: cache] oid %ld, cpu %ld # name %s, user %ld, nice %ld, sys %ld, idle %ld, total %ld\n",
                           CMON_OBJ_OID(cmon_obj),
                           csys_cpu_stat_pos,
                           (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat_cache)),
                           CSYS_CPU_STAT_USER(csys_cpu_stat_cache),CSYS_CPU_STAT_NICE(csys_cpu_stat_cache),CSYS_CPU_STAT_SYS(csys_cpu_stat_cache),
                           CSYS_CPU_STAT_IDLE(csys_cpu_stat_cache),CSYS_CPU_STAT_TOTAL(csys_cpu_stat_cache)
               ));

        used_time_cache  = CSYS_CPU_STAT_USER(csys_cpu_stat_cache) + CSYS_CPU_STAT_NICE(csys_cpu_stat_cache) + CSYS_CPU_STAT_SYS(csys_cpu_stat_cache);
        total_time_cache = (used_time_cache + CSYS_CPU_STAT_IDLE(csys_cpu_stat_cache));

        CSYS_CPU_STAT_LOAD(csys_cpu_stat_report) = 100.0 * (used_time_report - used_time_cache) / (total_time_report - total_time_cache);
    }

    return (0);
}

UINT32 cmon_obj_diff_csys_eth_stat_vec(const UINT32 cmon_md_id, const CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_eth_stat_vec_report;
    CVECTOR  *csys_eth_stat_vec_cache;

    UINT32 csys_eth_stat_pos;
    UINT32 csys_eth_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_diff_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_eth_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_eth_stat_vec_cache  = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_CACHE_DATA_AREA);

    csys_eth_stat_num = cvector_size(csys_eth_stat_vec_report);
    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat_report;
        CSYS_ETH_STAT *csys_eth_stat_cache;

        csys_eth_stat_report = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_report, csys_eth_stat_pos);

        CMON_DBG((LOGSTDOUT, "[eth vec: report] oid %ld, eth %ld # %s, speed %ld Mb/s, rxmoct %ld MBytes, txmoct %ld MBytes\n",
                           CMON_OBJ_OID(cmon_obj),
                           csys_eth_stat_pos,
                           (char *)cstring_get_str(CSYS_ETH_NAME(csys_eth_stat_report)),
                           CSYS_ETH_SPEEDMBS(csys_eth_stat_report),
                           CSYS_ETH_RXMOCT(csys_eth_stat_report),
                           CSYS_ETH_TXMOCT(csys_eth_stat_report)
               ));

        csys_eth_stat_cache  = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_cache , csys_eth_stat_pos);

        if(NULL_PTR == csys_eth_stat_cache)
        {
            CSYS_ETH_RXTHROUGHPUT(csys_eth_stat_report) = CSYS_ETH_RXMOCT(csys_eth_stat_report);
            CSYS_ETH_TXTHROUGHPUT(csys_eth_stat_report) = CSYS_ETH_TXMOCT(csys_eth_stat_report);
            continue;
        }

        CMON_DBG((LOGSTDOUT, "[eth vec: cache] oid %ld, eth %ld # %s, speed %ld Mb/s, rxmoct %ld MBytes, txmoct %ld MBytes\n",
                           CMON_OBJ_OID(cmon_obj),
                           csys_eth_stat_pos,
                           (char *)cstring_get_str(CSYS_ETH_NAME(csys_eth_stat_cache)),
                           CSYS_ETH_SPEEDMBS(csys_eth_stat_cache),
                           CSYS_ETH_RXMOCT(csys_eth_stat_cache),
                           CSYS_ETH_TXMOCT(csys_eth_stat_cache)
               ));

        CSYS_ETH_RXTHROUGHPUT(csys_eth_stat_report) = CSYS_ETH_RXMOCT(csys_eth_stat_report) - CSYS_ETH_RXMOCT(csys_eth_stat_cache);
        CSYS_ETH_TXTHROUGHPUT(csys_eth_stat_report) = CSYS_ETH_TXMOCT(csys_eth_stat_report) - CSYS_ETH_TXMOCT(csys_eth_stat_cache);
    }

    return (0);
}

/**
*
* CMON_OBJ report according to cmon_oid
*
**/
UINT32 cmon_obj_report(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MAP(cmon_obj);

    return CMON_MAP_OBJ_REPORT_FUNC(cmon_map)(cmon_md_id, cmon_obj);
}

UINT32 cmon_obj_report_mem_occupy(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    UINT32 mem_occupy_meas;
    UINT32 mem_occupy_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_mem_occupy: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mem_occupy_meas   = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_MEAS_DATA_AREA);
    mem_occupy_report = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_CACHE_DATA_AREA)  = mem_occupy_report;/*cache old data*/
    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA) = mem_occupy_meas;
    return (0);
}

UINT32 cmon_obj_report_mem_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    REAL mem_load_meas;
    REAL mem_load_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_mem_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mem_load_meas   = CMON_OBJ_DATA_REAL(cmon_obj, CMON_MEAS_DATA_AREA);
    mem_load_report = CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);

    CMON_OBJ_DATA_REAL(cmon_obj, CMON_CACHE_DATA_AREA)  = mem_load_report;
    CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA) = mem_load_meas;
    return (0);
}

UINT32 cmon_obj_report_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_cpu_stat_vec_meas;
    CVECTOR  *csys_cpu_stat_vec_report;
    CVECTOR  *csys_cpu_stat_vec_cache;

    UINT32 csys_cpu_stat_pos;
    UINT32 csys_cpu_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_cpu_stat_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    csys_cpu_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_cpu_stat_vec_cache  = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_CACHE_DATA_AREA);

    /*clone reported data to cache*/
    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec_report);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_cpu_vec: csys_cpu_stat_vec_report size = %ld, csys_cpu_stat_vec_cache size = %ld\n",
                        cvector_size(csys_cpu_stat_vec_report), cvector_size(csys_cpu_stat_vec_cache)));

    while(cvector_size(csys_cpu_stat_vec_cache) < csys_cpu_stat_num)
    {
        CSYS_CPU_STAT *csys_cpu_stat_cache;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_CPU_STAT, &csys_cpu_stat_cache, LOC_CMON_0007);
        csys_cpu_stat_init(csys_cpu_stat_cache);
        cvector_push(csys_cpu_stat_vec_cache, (void *)csys_cpu_stat_cache);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_cpu_vec: alloc MM_CSYS_CPU_STAT %lx for cache\n", csys_cpu_stat_cache));
    }

    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat_report;
        CSYS_CPU_STAT *csys_cpu_stat_cache;

        csys_cpu_stat_report = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_report, csys_cpu_stat_pos);
        csys_cpu_stat_cache  = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_cache, csys_cpu_stat_pos);

        csys_cpu_stat_clone(csys_cpu_stat_report, csys_cpu_stat_cache);
    }

    /*clone measured data to report*/
    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec_meas);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_cpu_vec: csys_cpu_stat_vec_meas size = %ld, csys_cpu_stat_vec_report size = %ld\n",
                        cvector_size(csys_cpu_stat_vec_meas), cvector_size(csys_cpu_stat_vec_report)));

    while(cvector_size(csys_cpu_stat_vec_report) < csys_cpu_stat_num)
    {
        CSYS_CPU_STAT *csys_cpu_stat_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_CPU_STAT, &csys_cpu_stat_report, LOC_CMON_0008);
        csys_cpu_stat_init(csys_cpu_stat_report);
        cvector_push(csys_cpu_stat_vec_report, (void *)csys_cpu_stat_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_cpu_vec: alloc MM_CSYS_CPU_STAT %lx for report\n", csys_cpu_stat_report));
    }

    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat_meas;
        CSYS_CPU_STAT *csys_cpu_stat_report;

        csys_cpu_stat_meas   = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_meas  , csys_cpu_stat_pos);
        csys_cpu_stat_report = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec_report, csys_cpu_stat_pos);

        csys_cpu_stat_clone(csys_cpu_stat_meas, csys_cpu_stat_report);
    }

    cmon_obj_diff_cpu_vec(cmon_md_id, cmon_obj);/*calculate load of each cpu*/

    return (0);
}

UINT32 cmon_obj_report_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *taskc_node_vec_meas;
    CVECTOR  *taskc_node_vec_report;

    UINT32 taskc_node_pos;
    UINT32 taskc_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_taskc_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    taskc_node_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    taskc_node_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    taskc_node_num = cvector_size(taskc_node_vec_meas);

    CMON_DBG((LOGSTDOUT, "cmon_obj_report_taskc_node_vec: taskc_node_vec_meas size = %ld, taskc_node_vec_report size = %ld\n",
                        cvector_size(taskc_node_vec_meas), cvector_size(taskc_node_vec_report)));

    while(cvector_size(taskc_node_vec_report) < taskc_node_num)
    {
        TASKC_NODE *taskc_node_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_TASKC_NODE, &taskc_node_report, LOC_CMON_0009);
        taskc_node_init(taskc_node_report);
        cvector_push(taskc_node_vec_report, (void *)taskc_node_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_taskc_node_vec: alloc MM_TASKC_NODE %lx\n", taskc_node_report));
    }

    for(taskc_node_pos = 0; taskc_node_pos < taskc_node_num; taskc_node_pos ++)
    {
        TASKC_NODE *taskc_node_meas;
        TASKC_NODE *taskc_node_report;

        taskc_node_meas   = (TASKC_NODE *)cvector_get(taskc_node_vec_meas  , taskc_node_pos);
        taskc_node_report = (TASKC_NODE *)cvector_get(taskc_node_vec_report, taskc_node_pos);

        taskc_node_clone(taskc_node_meas, taskc_node_report);
    }


    return (0);
}

UINT32 cmon_obj_report_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *csocket_cnode_vec_meas;
    CVECTOR  *csocket_cnode_vec_report;

    UINT32 csocket_cnode_pos;
    UINT32 csocket_cnode_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_csocket_cnode_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csocket_cnode_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    csocket_cnode_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csocket_cnode_num = cvector_size(csocket_cnode_vec_meas);

    CMON_DBG((LOGSTDOUT, "cmon_obj_report_csocket_cnode_vec: csocket_cnode_vec_meas size = %ld, csocket_cnode_vec_report size = %ld\n",
                        cvector_size(csocket_cnode_vec_meas), cvector_size(csocket_cnode_vec_report)));

    while(cvector_size(csocket_cnode_vec_report) < csocket_cnode_num)
    {
        CSOCKET_CNODE *csocket_cnode_report;

        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSOCKET_CNODE, &csocket_cnode_report, LOC_CMON_0010);
        csocket_cnode_init(csocket_cnode_report);
        cvector_push(csocket_cnode_vec_report, (void *)csocket_cnode_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_csocket_cnode_vec: alloc MM_CSOCKET_CNODE %lx\n", csocket_cnode_report));
    }

    for(csocket_cnode_pos = 0; csocket_cnode_pos < csocket_cnode_num; csocket_cnode_pos ++)
    {
        CSOCKET_CNODE *csocket_cnode_meas;
        CSOCKET_CNODE *csocket_cnode_report;

        csocket_cnode_meas   = (CSOCKET_CNODE *)cvector_get(csocket_cnode_vec_meas  , csocket_cnode_pos);
        csocket_cnode_report = (CSOCKET_CNODE *)cvector_get(csocket_cnode_vec_report, csocket_cnode_pos);

        csocket_cnode_clone_0(csocket_cnode_meas, csocket_cnode_report);
    }

    return (0);
}

UINT32 cmon_obj_report_thread_num(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    UINT32 thread_num_meas;
    UINT32 thread_num_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_thread_num: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    thread_num_meas   = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_MEAS_DATA_AREA);
    thread_num_report = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_CACHE_DATA_AREA)  = thread_num_report;
    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA) = thread_num_meas;
    return (0);
}

UINT32 cmon_obj_report_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CRANK_THREAD_STAT * crank_thread_stat_meas;
    CRANK_THREAD_STAT * crank_thread_stat_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    crank_thread_stat_meas   = CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_MEAS_DATA_AREA);
    crank_thread_stat_report = CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_REPORT_DATA_AREA);

    crank_thread_stat_clone(crank_thread_stat_report, CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_CACHE_DATA_AREA));
    crank_thread_stat_clone(crank_thread_stat_meas, CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_REPORT_DATA_AREA));
    return (0);
}

UINT32 cmon_obj_report_cpu_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    REAL cpu_load_meas;
    REAL cpu_load_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_cpu_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cpu_load_meas   = CMON_OBJ_DATA_REAL(cmon_obj, CMON_MEAS_DATA_AREA);
    cpu_load_report = CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);

    CMON_OBJ_DATA_REAL(cmon_obj, CMON_CACHE_DATA_AREA)  = cpu_load_report;
    CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA) = cpu_load_meas;
    return (0);
}

UINT32 cmon_obj_report_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *mm_man_occupy_node_vec_meas;
    CVECTOR  *mm_man_occupy_node_vec_report;

    UINT32 mm_man_occupy_node_pos;
    UINT32 mm_man_occupy_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_mm_man_occupy_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mm_man_occupy_node_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    mm_man_occupy_node_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    mm_man_occupy_node_num = cvector_size(mm_man_occupy_node_vec_meas);

    CMON_DBG((LOGSTDOUT, "cmon_obj_report_mm_man_occupy_node_vec: mm_man_occupy_node_vec_meas size = %ld, mm_man_occupy_node_vec_report size = %ld\n",
                        cvector_size(mm_man_occupy_node_vec_meas), cvector_size(mm_man_occupy_node_vec_report)));

    while(cvector_size(mm_man_occupy_node_vec_report) < mm_man_occupy_node_num)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node_report;

        alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_OCCUPY_NODE, &mm_man_occupy_node_report, LOC_CMON_0011);
        mm_man_occupy_node_init(mm_man_occupy_node_report);
        cvector_push(mm_man_occupy_node_vec_report, (void *)mm_man_occupy_node_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_mm_man_occupy_node_vec: alloc MM_MM_MAN_OCCUPY_NODE %lx\n", mm_man_occupy_node_report));
    }

    for(mm_man_occupy_node_pos = 0; mm_man_occupy_node_pos < mm_man_occupy_node_num; mm_man_occupy_node_pos ++)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node_meas;
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node_report;

        mm_man_occupy_node_meas   = (MM_MAN_OCCUPY_NODE *)cvector_get(mm_man_occupy_node_vec_meas  , mm_man_occupy_node_pos);
        mm_man_occupy_node_report = (MM_MAN_OCCUPY_NODE *)cvector_get(mm_man_occupy_node_vec_report, mm_man_occupy_node_pos);

        mm_man_occupy_node_clone(mm_man_occupy_node_meas, mm_man_occupy_node_report);
    }

    return (0);
}

UINT32 cmon_obj_report_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *mm_man_load_node_vec_meas;
    CVECTOR  *mm_man_load_node_vec_report;

    UINT32 mm_man_load_node_pos;
    UINT32 mm_man_load_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_mm_man_load_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mm_man_load_node_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    mm_man_load_node_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    mm_man_load_node_num = cvector_size(mm_man_load_node_vec_meas);

    CMON_DBG((LOGSTDOUT, "cmon_obj_report_mm_man_load_node_vec: mm_man_load_node_vec_meas size = %ld, mm_man_load_node_vec_report size = %ld\n",
                        cvector_size(mm_man_load_node_vec_meas), cvector_size(mm_man_load_node_vec_report)));

    while(cvector_size(mm_man_load_node_vec_report) < mm_man_load_node_num)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node_report;

        alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_MM_MAN_LOAD_NODE, &mm_man_load_node_report, LOC_CMON_0012);
        mm_man_load_node_init(mm_man_load_node_report);
        cvector_push(mm_man_load_node_vec_report, (void *)mm_man_load_node_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_mm_man_load_node_vec: alloc MM_MM_MAN_LOAD_NODE %lx\n", mm_man_load_node_report));
    }

    for(mm_man_load_node_pos = 0; mm_man_load_node_pos < mm_man_load_node_num; mm_man_load_node_pos ++)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node_meas;
        MM_MAN_LOAD_NODE *mm_man_load_node_report;

        mm_man_load_node_meas   = (MM_MAN_LOAD_NODE *)cvector_get(mm_man_load_node_vec_meas  , mm_man_load_node_pos);
        mm_man_load_node_report = (MM_MAN_LOAD_NODE *)cvector_get(mm_man_load_node_vec_report, mm_man_load_node_pos);

        mm_man_load_node_clone(mm_man_load_node_meas, mm_man_load_node_report);
    }

    return (0);
}

UINT32 cmon_obj_report_module_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *cproc_module_stat_vec_meas;
    CVECTOR  *cproc_module_stat_vec_report;

    UINT32 cproc_module_stat_pos;
    UINT32 cproc_module_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_module_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cproc_module_stat_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    cproc_module_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    /*clone measured data to report*/
    cproc_module_stat_num = cvector_size(cproc_module_stat_vec_meas);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_module_vec: cproc_module_stat_vec_meas size = %ld, cproc_module_stat_vec_report size = %ld\n",
                        cvector_size(cproc_module_stat_vec_meas), cvector_size(cproc_module_stat_vec_report)));

    while(cvector_size(cproc_module_stat_vec_report) < cproc_module_stat_num)
    {
        CPROC_MODULE_STAT *cproc_module_stat_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CPROC_MODULE_STAT, &cproc_module_stat_report, LOC_CMON_0013);
        cproc_module_stat_init(cproc_module_stat_report);
        cvector_push(cproc_module_stat_vec_report, (void *)cproc_module_stat_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_module_vec: alloc MM_CPROC_MODULE_STAT %lx for report\n", cproc_module_stat_report));
    }

    for(cproc_module_stat_pos = 0; cproc_module_stat_pos < cproc_module_stat_num; cproc_module_stat_pos ++)
    {
        CPROC_MODULE_STAT *cproc_module_stat_meas;
        CPROC_MODULE_STAT *cproc_module_stat_report;

        cproc_module_stat_meas   = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec_meas  , cproc_module_stat_pos);
        cproc_module_stat_report = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec_report, cproc_module_stat_pos);

        cproc_module_stat_clone(cproc_module_stat_meas, cproc_module_stat_report);
    }

    return (0);
}

UINT32 cmon_obj_report_crank_task_load(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    UINT32 thread_num_meas;
    UINT32 thread_num_report;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_crank_task_load: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    thread_num_meas   = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_MEAS_DATA_AREA);
    thread_num_report = CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);

    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_CACHE_DATA_AREA)  = thread_num_report;
    CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA) = thread_num_meas;
    return (0);
}

UINT32 cmon_obj_report_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_eth_stat_vec_meas;
    CVECTOR  *csys_eth_stat_vec_report;
    CVECTOR  *csys_eth_stat_vec_cache;

    UINT32 csys_eth_stat_pos;
    UINT32 csys_eth_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_eth_stat_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    csys_eth_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_eth_stat_vec_cache  = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_CACHE_DATA_AREA);

    /*clone reported data to cache*/
    csys_eth_stat_num = cvector_size(csys_eth_stat_vec_report);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_eth_stat_vec: csys_eth_stat_vec_report size = %ld, csys_eth_stat_vec_cache size = %ld\n",
                        cvector_size(csys_eth_stat_vec_report), cvector_size(csys_eth_stat_vec_cache)));

    while(cvector_size(csys_eth_stat_vec_cache) < csys_eth_stat_num)
    {
        CSYS_ETH_STAT *csys_eth_stat_cache;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_ETH_STAT, &csys_eth_stat_cache, LOC_CMON_0014);
        csys_eth_stat_init(csys_eth_stat_cache);
        cvector_push(csys_eth_stat_vec_cache, (void *)csys_eth_stat_cache);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_eth_stat_vec: alloc MM_CSYS_ETH_STAT %lx for cache\n", csys_eth_stat_cache));
    }

    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat_report;
        CSYS_ETH_STAT *csys_eth_stat_cache;

        csys_eth_stat_report = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_report, csys_eth_stat_pos);
        csys_eth_stat_cache  = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_cache, csys_eth_stat_pos);

        csys_eth_stat_clone(csys_eth_stat_report, csys_eth_stat_cache);
    }

    /*clone measured data to report*/
    csys_eth_stat_num = cvector_size(csys_eth_stat_vec_meas);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_eth_stat_vec: csys_eth_stat_vec_meas size = %ld, csys_eth_stat_vec_report size = %ld\n",
                        cvector_size(csys_eth_stat_vec_meas), cvector_size(csys_eth_stat_vec_report)));

    while(cvector_size(csys_eth_stat_vec_report) < csys_eth_stat_num)
    {
        CSYS_ETH_STAT *csys_eth_stat_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_ETH_STAT, &csys_eth_stat_report, LOC_CMON_0015);
        csys_eth_stat_init(csys_eth_stat_report);
        cvector_push(csys_eth_stat_vec_report, (void *)csys_eth_stat_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_eth_stat_vec: alloc MM_CSYS_ETH_STAT %lx for report\n", csys_eth_stat_report));
    }

    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat_meas;
        CSYS_ETH_STAT *csys_eth_stat_report;

        csys_eth_stat_meas   = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_meas  , csys_eth_stat_pos);
        csys_eth_stat_report = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec_report, csys_eth_stat_pos);

        csys_eth_stat_clone(csys_eth_stat_meas, csys_eth_stat_report);
    }

    cmon_obj_diff_csys_eth_stat_vec(cmon_md_id, cmon_obj);/*calculate load of each eth*/

    return (0);
}

UINT32 cmon_obj_report_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *csys_dsk_stat_vec_meas;
    CVECTOR  *csys_dsk_stat_vec_report;

    UINT32 csys_dsk_stat_pos;
    UINT32 csys_dsk_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_csys_dsk_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_dsk_stat_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    csys_dsk_stat_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    /*clone measured data to report*/
    csys_dsk_stat_num = cvector_size(csys_dsk_stat_vec_meas);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_dsk_stat_vec: csys_dsk_stat_vec_meas size = %ld, csys_dsk_stat_vec_report size = %ld\n",
                        cvector_size(csys_dsk_stat_vec_meas), cvector_size(csys_dsk_stat_vec_report)));

    while(cvector_size(csys_dsk_stat_vec_report) < csys_dsk_stat_num)
    {
        CSYS_DSK_STAT *csys_dsk_stat_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_DSK_STAT, &csys_dsk_stat_report, LOC_CMON_0016);
        csys_dsk_stat_init(csys_dsk_stat_report);
        cvector_push(csys_dsk_stat_vec_report, (void *)csys_dsk_stat_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_csys_dsk_stat_vec: alloc MM_CSYS_DSK_STAT %lx for report\n", csys_dsk_stat_report));
    }

    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < csys_dsk_stat_num; csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat_meas;
        CSYS_DSK_STAT *csys_dsk_stat_report;

        csys_dsk_stat_meas   = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec_meas  , csys_dsk_stat_pos);
        csys_dsk_stat_report = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec_report, csys_dsk_stat_pos);

        csys_dsk_stat_clone(csys_dsk_stat_meas, csys_dsk_stat_report);
    }

    return (0);
}

UINT32 cmon_obj_report_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ * cmon_obj)
{
    CVECTOR  *task_report_vec_meas;
    CVECTOR  *task_report_vec_report;

    UINT32 task_report_node_pos;
    UINT32 task_report_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_report_crank_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    task_report_vec_meas   = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_MEAS_DATA_AREA);
    task_report_vec_report = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    /*clone measured data to report*/
    task_report_node_num = cvector_size(task_report_vec_meas);
    CMON_DBG((LOGSTDOUT, "cmon_obj_report_crank_task_report_vec: task_report_vec_meas size = %ld, task_report_vec_report size = %ld\n",
                        cvector_size(task_report_vec_meas), cvector_size(task_report_vec_report)));

    while(cvector_size(task_report_vec_report) > task_report_node_num)
    {
        TASK_REPORT_NODE *task_report_node_report;

        task_report_node_report = (TASK_REPORT_NODE *)cvector_pop(task_report_vec_report);
        task_report_node_free(task_report_node_report);
    }

    while(cvector_size(task_report_vec_report) < task_report_node_num)
    {
        TASK_REPORT_NODE *task_report_node_report;
        alloc_static_mem(MD_CMON, cmon_md_id, MM_TASK_REPORT_NODE, &task_report_node_report, LOC_CMON_0017);
        task_report_node_init(task_report_node_report);
        cvector_push(task_report_vec_report, (void *)task_report_node_report);
        CMON_DBG((LOGSTDOUT, "cmon_obj_report_crank_task_report_vec: alloc MM_TASK_REPORT_NODE %lx for report\n", task_report_node_report));
    }

    for(task_report_node_pos = 0; task_report_node_pos < task_report_node_num; task_report_node_pos ++)
    {
        TASK_REPORT_NODE *task_report_node_meas;
        TASK_REPORT_NODE *task_report_node_report;

        task_report_node_meas   = (TASK_REPORT_NODE *)cvector_get(task_report_vec_meas  , task_report_node_pos);
        task_report_node_report = (TASK_REPORT_NODE *)cvector_get(task_report_vec_report, task_report_node_pos);

        task_report_node_clone(task_report_node_meas, task_report_node_report);
    }

    //sys_log(LOGSTDOUT, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
    //cvector_print(LOGSTDOUT, task_report_vec_report, (CVECTOR_DATA_PRINT)task_report_node_print);
    //sys_log(LOGSTDOUT, "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

    return (0);
}

/**
*
* new a CMON_OBJ_VEC
*
**/
CMON_OBJ_VEC * cmon_obj_vec_new(const UINT32 cmon_md_id)
{
    CMON_OBJ_VEC * cmon_obj_vec;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_new: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    alloc_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ_VEC, &cmon_obj_vec, LOC_CMON_0018);
    cmon_obj_vec_init(cmon_md_id, cmon_obj_vec);

    return (cmon_obj_vec);
}

/**
*
* initialize a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_init(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cvector_init(CMON_OBJ_VEC(cmon_obj_vec), 0, MM_CMON_OBJ, CVECTOR_LOCK_ENABLE, LOC_CMON_0019);
    CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec) = NULL_PTR;
    return (0);
}

/**
*
* clean a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_clean(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32 cmon_obj_num;
    UINT32 cmon_obj_pos;

    UINT32 cmon_obj_merge_num;
    UINT32 cmon_obj_merge_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_clean: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_num = cvector_size(CMON_OBJ_VEC(cmon_obj_vec));
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(CMON_OBJ_VEC(cmon_obj_vec), cmon_obj_pos);
        if(NULL_PTR == cmon_obj)
        {
            continue;
        }

        cmon_obj_free(cmon_md_id, cmon_obj);
    }

    cvector_clean(CMON_OBJ_VEC(cmon_obj_vec), NULL_PTR, LOC_CMON_0020);

    if(NULL_PTR == CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        return (0);
    }

    cmon_obj_merge_num = cvector_size(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec));
    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cmon_obj_merge_num; cmon_obj_merge_pos ++)
    {
        CMON_OBJ_MERGE *cmon_obj_merge;
        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            continue;
        }
        cmon_obj_merge_free(cmon_md_id, cmon_obj_merge);
    }

    cvector_clean(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), NULL_PTR, LOC_CMON_0021);
    cvector_free(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), LOC_CMON_0022);

    CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec) = NULL_PTR;

    return (0);
}

/**
*
* free a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_free(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_free: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec_clean(cmon_md_id, cmon_obj_vec);
    free_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ_VEC, cmon_obj_vec, LOC_CMON_0023);
    return (0);
}

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_size(const UINT32 cmon_md_id, const CMON_OBJ_VEC * cmon_obj_vec)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_size: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    return cvector_size(CMON_OBJ_VEC(cmon_obj_vec));
}

/**
*
* add a CMON_OBJ to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_add(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_add: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cvector_push(CMON_OBJ_VEC(cmon_obj_vec), (void *)cmon_obj);

    if(NULL_PTR != CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        cmon_obj_vec_merge(cmon_md_id, cmon_obj, cmon_obj_vec);
    }

    return (0);
}

/**
*
* bind a CMON_OBJ_MERGE to CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_bind(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
    CVECTOR *cmon_obj_merge_vec;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_bind: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR != CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_bind: cmon_obj_vec already binded a merge vec\n");
        return ((UINT32)-1);
    }

    cmon_obj_merge_vec = cvector_new(0, MM_CMON_OBJ_MERGE, LOC_CMON_0024);
    CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec) = cmon_obj_merge_vec;
    return (0);
}

/**
*
* merge a CMON_OBJ to CMON_OBJ_VEC_MERGE_VEC of CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32 cmon_obj_merge_num;
    UINT32 cmon_obj_merge_pos;

    CVECTOR  *cmon_obj_merge_vec;
    CMON_OBJ_MERGE *cmon_obj_merge;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_merge_vec = CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec);

    cmon_obj_merge_num = cvector_size(cmon_obj_merge_vec);
    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cmon_obj_merge_num; cmon_obj_merge_pos ++)
    {
        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(cmon_obj_merge_vec, cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            continue;
        }

        /*found*/
        if(NULL_PTR != CMON_OBJ_MAP(cmon_obj) && EC_TRUE == CMON_MAP_OBJ_VEC_MERGE_MATCH_FUNC(CMON_OBJ_MAP(cmon_obj))(cmon_md_id, cmon_obj, cmon_obj_merge))
        {
            cvector_push(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), (void *)cmon_obj);
            return (0); /*note: one CMON_OBJ belong to at most one CMON_OBJ_MERGE, so return from here*/
        }
    }

    /*not found*/
    if(
       NULL_PTR != CMON_OBJ_MAP(cmon_obj)
    && NULL_PTR != CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC(CMON_OBJ_MAP(cmon_obj))
    && EC_FALSE == CMON_MAP_OBJ_VEC_MERGE_FILTER_FUNC(CMON_OBJ_MAP(cmon_obj))(cmon_md_id, cmon_obj)
       )
    {
        /*give up!*/
        return (0);
    }

    cmon_obj_merge = cmon_obj_merge_new(cmon_md_id, CMON_OBJ_OID(cmon_obj), CMON_OBJ_MOD_NODE(cmon_obj));
    if(NULL_PTR != CMON_OBJ_MAP(cmon_obj))
    {
        CMON_MAP_OBJ_VEC_MERGE_NAME_FUNC(CMON_OBJ_MAP(cmon_obj))(cmon_md_id, cmon_obj, cmon_obj_merge);
    }

    cvector_push(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), (void *)cmon_obj);/*ok, obtain the first one CMON_OBJ*/
    cvector_push(cmon_obj_merge_vec, (void *)cmon_obj_merge);

    return (0);
}


/**
*
* get a CMON_OBJ from CMON_OBJ_VEC
*
**/
CMON_OBJ * cmon_obj_vec_get(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const CMON_OBJ_VEC * cmon_obj_vec)
{
    CMON_OBJ * cmon_obj;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_get: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/
    //sys_log(LOGSTDOUT, "cmon_obj_vec_get: cmon_obj_vec = %lx\n", cmon_obj_vec);
    cmon_obj = (CMON_OBJ *)cvector_get(CMON_OBJ_VEC(cmon_obj_vec), cmon_obj_pos);

    return (cmon_obj);
}

/**
*
* set a CMON_OBJ to CMON_OBJ_VEC
*
**/
CMON_OBJ * cmon_obj_vec_set(const UINT32 cmon_md_id, const UINT32 cmon_obj_pos, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC * cmon_obj_vec)
{
    CMON_OBJ * cmon_obj_saved;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_set: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_saved = (CMON_OBJ *)cvector_set(CMON_OBJ_VEC(cmon_obj_vec), cmon_obj_pos, (void *)cmon_obj);

    return (cmon_obj_saved);
}


/**
*
* include or define CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_incl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, CMON_OBJ_VEC * cmon_obj_vec)
{
    CMON_MD  * cmon_md;

    MOD_MGR  * mod_mgr;

    UINT32     mod_node_pos;
    UINT32     mod_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_incl: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_md = CMON_MD_GET(cmon_md_id);
    mod_mgr = cmon_md->mod_mgr;

    mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    for(mod_node_pos = 0; mod_node_pos < mod_node_num; mod_node_pos ++)
    {
        MOD_NODE * mod_node;

        mod_node = (MOD_NODE *)cvector_get(MOD_MGR_REMOTE_LIST(mod_mgr), mod_node_pos);

        if(MOD_NODE_MATCH(mod_node, tcid, comm, rank, modi))
        {
            CMON_OBJ * cmon_obj;

            cmon_obj = cmon_obj_new(cmon_md_id, cmon_oid);
            CMON_OBJ_MOD_NODE(cmon_obj) = mod_node;

            cmon_obj_vec_add(cmon_md_id, cmon_obj, cmon_obj_vec);/*include*/
        }
    }

    return (0);
}

/**
*
* exclude or undefine CMON_OBJ list with specific (tcid, comm, rank, modi) info,i.e., MOD_NODE info
*
**/
UINT32 cmon_obj_vec_excl(const UINT32 cmon_md_id, const UINT32 cmon_oid, const UINT32 tcid, const UINT32 comm, const UINT32 rank, const UINT32 modi, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_excl: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_num = cmon_obj_vec_size(cmon_md_id, cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;
        MOD_NODE * mod_node;

        cmon_obj = cmon_obj_vec_get(cmon_md_id, cmon_obj_pos, cmon_obj_vec);
        mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

        if(cmon_oid != CMON_OBJ_OID(cmon_obj) && MOD_NODE_MATCH(mod_node, tcid, comm, rank, modi))
        {
            cmon_obj_vec_set(cmon_md_id, cmon_obj_pos, NULL_PTR, cmon_obj_vec);

            CMON_OBJ_MOD_NODE(cmon_obj) = NULL_PTR;
            cmon_obj_free(cmon_md_id, cmon_obj);
        }
    }

    return (0);
}

/**
*
*   purge broken remote measured object if found in cmon_obj_vec
*
*
**/
UINT32 cmon_obj_vec_purge(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32     cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_purge: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(CMON_OBJ_VEC(cmon_obj_vec)); /*cmon_obj_pos ++*/)
    {
        CMON_OBJ * cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(CMON_OBJ_VEC(cmon_obj_vec), cmon_obj_pos);
        if(EC_FALSE == cmon_obj_is_connected(cmon_md_id, cmon_obj))
        {
            sys_log(LOGSTDNULL, "[DEBUG] cmon_obj_vec_purge: tcid %s comm %ld rank %ld modi %ld was broken\n",
                                MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                                MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)),
                                MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)),
                                MOD_NODE_MODI(CMON_OBJ_MOD_NODE(cmon_obj))
                    );
            if(NULL_PTR != CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
            {
                cmon_obj_vec_merge_purge(cmon_md_id, cmon_obj, cmon_obj_vec);
            }

            cvector_erase(CMON_OBJ_VEC(cmon_obj_vec), cmon_obj_pos);
            cmon_obj_free(cmon_md_id, cmon_obj);

            continue;
        }

        cmon_obj_pos ++;/*move forward*/
    }

    return (0);
}

/**
*
* CMON_OBJ_VEC measure and collect result from remote mod_node
*
* note: one CMON_OBJ measure and collect one remote mod_node which is given by mod_node in CMON_OBJ
*
**/
UINT32 cmon_obj_vec_meas(const UINT32 cmon_md_id, const UINT32 time_to_live, CMON_OBJ_VEC * cmon_obj_vec)
{
    CMON_MD  * cmon_md;

    MOD_MGR  * mod_mgr;
    MOD_NODE * send_mod_node;

    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

    TASK_MGR * task_mgr;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_meas: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*purge broken cmon_obj(s) at first*/
    cmon_obj_vec_purge(cmon_md_id, cmon_obj_vec);

    cmon_md = CMON_MD_GET(cmon_md_id);
    mod_mgr = cmon_md->mod_mgr;

    //sys_log(LOGSTDOUT, "cmon_obj_vec_meas: cmon_obj_vec = %lx\n", cmon_obj_vec);
    send_mod_node = MOD_MGR_LOCAL_MOD(mod_mgr);

    //sys_log(LOGSTDOUT, "######## cmon_obj_vec_size = %ld\n", cmon_obj_vec_size(cmon_md_id, cmon_obj_vec));

    task_mgr = task_new(mod_mgr, TASK_PRIO_HIGH, TASK_NEED_RSP_FLAG, TASK_NEED_ALL_RSP);

    cmon_obj_num = cmon_obj_vec_size(cmon_md_id, cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;
        MOD_NODE * recv_mod_node;
        UINT32     ret;

        cmon_obj      = cmon_obj_vec_get(cmon_md_id, cmon_obj_pos, cmon_obj_vec);
        recv_mod_node = CMON_OBJ_MOD_NODE(cmon_obj);

        /*clean up measurement data area of cmon_obj*/
        cmon_obj_clean(cmon_md_id, CMON_MEAS_DATA_AREA, cmon_obj);

        task_super_inc(task_mgr, send_mod_node, recv_mod_node, &ret, FI_cmon_obj_meas, ERR_MODULE_ID, cmon_obj);
    }

    task_wait(task_mgr, time_to_live, TASK_NOT_NEED_RESCHEDULE_FLAG, NULL_PTR);

    /*after measurement, reporting immediately*/
    cmon_obj_vec_report(cmon_md_id, cmon_obj_vec);

    /*[optional] calculate merge result*/
    if(NULL_PTR != CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        cmon_obj_vec_merge_result_clean(cmon_md_id, cmon_obj_vec);

        cmon_obj_vec_merge_calc(cmon_md_id, cmon_obj_vec);
    }

    return (0);
}

/**
*
* CMON_OBJ_VEC print result
*
* note: CMON_OBJ print one by one
*
**/
UINT32 cmon_obj_vec_print(const UINT32 cmon_md_id, LOG *log, const CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_print: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_num = cmon_obj_vec_size(cmon_md_id, cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;

        cmon_obj = cmon_obj_vec_get(cmon_md_id, cmon_obj_pos, cmon_obj_vec);

        cmon_obj_print(cmon_md_id, log, cmon_obj);
    }

    return (0);
}

/**
*
* CMON_OBJ_VEC report result
*
* note: CMON_OBJ report one by one
*
**/
UINT32 cmon_obj_vec_report(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_report: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_num = cmon_obj_vec_size(cmon_md_id, cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;

        cmon_obj = cmon_obj_vec_get(cmon_md_id, cmon_obj_pos, cmon_obj_vec);

        cmon_obj_report(cmon_md_id, cmon_obj);
    }

    return (0);
}

/**
*
* clean result area of CMON_OBJ_MERGEs of a CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge_result_clean(const UINT32 cmon_md_id, CMON_OBJ_VEC * cmon_obj_vec)
{
    UINT32 cmon_obj_merge_num;
    UINT32 cmon_obj_merge_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_result_clean: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        return (0);
    }

    cmon_obj_merge_num = cvector_size(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec));
    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cmon_obj_merge_num; cmon_obj_merge_pos ++)
    {
        CMON_OBJ_MERGE *cmon_obj_merge;
        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            continue;
        }
        cmon_obj_merge_result_clean(cmon_md_id, cmon_obj_merge);
    }

    return (0);
}

/**
*
* purge cmon obj from all CMON_OBJ_MERGEs of a CMON_OBJ_VEC
*
* after purge cmon obj, if CMON_OBJ_MERGE is empty, then erase it from CMON_OBJ_VEC
*
**/
UINT32 cmon_obj_vec_merge_purge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_VEC *cmon_obj_vec)
{
    UINT32 cmon_obj_merge_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_purge: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        return (0);
    }

    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cvector_size(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec)); /*cmon_obj_merge_pos ++*/)
    {
        CMON_OBJ_MERGE *cmon_obj_merge;

        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            cvector_erase(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), cmon_obj_merge_pos);
            continue;
        }

        cmon_obj_merge_purge(cmon_md_id, cmon_obj, cmon_obj_merge);

        if(EC_TRUE == cmon_obj_merge_is_empty(cmon_md_id, cmon_obj_merge))
        {
            cvector_erase(CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec), cmon_obj_merge_pos);
            cmon_obj_merge_free(cmon_md_id, cmon_obj_merge);
            continue;
        }

        cmon_obj_merge_pos ++;
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_calc(const UINT32 cmon_md_id, CMON_OBJ_VEC *cmon_obj_vec)
{
    UINT32 cmon_obj_merge_num;
    UINT32 cmon_obj_merge_pos;

    CVECTOR  *cmon_obj_merge_vec;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_calc: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "warn:cmon_obj_vec_merge_calc: merge vec is null\n");
        return (0);
    }

    cmon_obj_merge_vec = CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec);
    cmon_obj_merge_num = cvector_size(cmon_obj_merge_vec);
    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cmon_obj_merge_num; cmon_obj_merge_pos ++)
    {
        CMON_OBJ_MERGE *cmon_obj_merge;
        CMON_MAP       *cmon_map;

        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(cmon_obj_merge_vec, cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            continue;
        }

        cmon_map = CMON_OBJ_MERGE_MAP(cmon_obj_merge);
        if(NULL_PTR == cmon_map)
        {
            continue;
        }

        if(NULL_PTR != CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC(cmon_map))
        {
            CMON_MAP_OBJ_VEC_MERGE_CALC_FUNC(cmon_map)(cmon_md_id, cmon_obj_merge);
        }
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_sum_real(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_sum_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) = 0.0;

    /*run through referenced CMON_OBJ and sum the result*/
    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);

        CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) += CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_sum_uint32(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_sum_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) = 0;

    /*run through referenced CMON_OBJ and sum the result*/
    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);

        CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) += CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);
    }


    return (0);
}

UINT32 cmon_obj_vec_merge_sum_crank_thread_stat(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CRANK_THREAD_STAT *val_crank_thread_stat;
    CVECTOR *cmon_obj_vec;
    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_sum_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    val_crank_thread_stat = CMON_OBJ_MERGE_RESULT_CRANK_THREAD_STAT(cmon_obj_merge);

    /*run through referenced CMON_OBJ and sum the result*/
    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;
        CRANK_THREAD_STAT *cur_crank_thread_stat;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);
        cur_crank_thread_stat = CMON_OBJ_DATA_CRANK_THREAD_STAT(cmon_obj, CMON_REPORT_DATA_AREA);

        CRANK_THREAD_MAX_NUM(val_crank_thread_stat)  += CRANK_THREAD_MAX_NUM(cur_crank_thread_stat);
        CRANK_THREAD_BUSY_NUM(val_crank_thread_stat) += CRANK_THREAD_BUSY_NUM(cur_crank_thread_stat);
        CRANK_THREAD_IDLE_NUM(val_crank_thread_stat) += CRANK_THREAD_IDLE_NUM(cur_crank_thread_stat);
    }


    return (0);
}


UINT32 cmon_obj_vec_merge_avg_real(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    UINT32   cmon_obj_num;
    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_avg_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) = 0.0;

    /*run through referenced CMON_OBJ and sum the result*/
    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);
    cmon_obj_num = cvector_size(cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);

        CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) += CMON_OBJ_DATA_REAL(cmon_obj, CMON_REPORT_DATA_AREA);
    }

    if(1 < cmon_obj_num)
    {
        CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) /= cmon_obj_num;
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_avg_uint32(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    UINT32   cmon_obj_num;
    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_avg_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) = 0;

    /*run through referenced CMON_OBJ and sum the result*/
    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);
    cmon_obj_num = cvector_size(cmon_obj_vec);
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);

        CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) += CMON_OBJ_DATA_UINT32(cmon_obj, CMON_REPORT_DATA_AREA);
    }

    if(1 < cmon_obj_num)
    {
        CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) /= cmon_obj_num;
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_taskc_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *taskc_node_vec;
    UINT32   taskc_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_taskc_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_taskc_node_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_taskc_node_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    taskc_node_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(taskc_node_pos = 0; taskc_node_pos < cvector_size(taskc_node_vec); taskc_node_pos ++)
    {
        TASKC_NODE *taskc_node_src;
        TASKC_NODE *taskc_node_des;

        taskc_node_src = (TASKC_NODE *)cvector_get(taskc_node_vec, taskc_node_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_TASKC_NODE, &taskc_node_des, LOC_CMON_0025);
        taskc_node_init(taskc_node_des);

        taskc_node_clone(taskc_node_src, taskc_node_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)taskc_node_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_csocket_cnode_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *csocket_cnode_vec;
    UINT32   csocket_cnode_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_csocket_cnode_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csocket_cnode_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csocket_cnode_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    csocket_cnode_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(csocket_cnode_pos = 0; csocket_cnode_pos < cvector_size(csocket_cnode_vec); csocket_cnode_pos ++)
    {
        CSOCKET_CNODE *csocket_cnode_src;
        CSOCKET_CNODE *csocket_cnode_des;

        csocket_cnode_src = (CSOCKET_CNODE *)cvector_get(csocket_cnode_vec, csocket_cnode_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSOCKET_CNODE, &csocket_cnode_des, LOC_CMON_0026);
        csocket_cnode_init(csocket_cnode_des);

        csocket_cnode_clone_0(csocket_cnode_src, csocket_cnode_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)csocket_cnode_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_cpu_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *csys_cpu_stat_vec;
    UINT32   csys_cpu_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_cpu_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_cpu_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    csys_cpu_stat_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < cvector_size(csys_cpu_stat_vec); csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat_src;
        CSYS_CPU_STAT *csys_cpu_stat_des;

        csys_cpu_stat_src = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_CPU_STAT, &csys_cpu_stat_des, LOC_CMON_0027);
        csys_cpu_stat_init(csys_cpu_stat_des);

        csys_cpu_stat_clone(csys_cpu_stat_src, csys_cpu_stat_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)csys_cpu_stat_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_mm_man_occupy_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *mm_man_occupy_node_vec;
    UINT32   mm_man_occupy_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_mm_man_occupy_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_mm_man_occupy_node_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_mm_man_occupy_node_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    mm_man_occupy_node_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(mm_man_occupy_node_pos = 0; mm_man_occupy_node_pos < cvector_size(mm_man_occupy_node_vec); mm_man_occupy_node_pos ++)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node_src;
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node_des;

        mm_man_occupy_node_src = (MM_MAN_OCCUPY_NODE *)cvector_get(mm_man_occupy_node_vec, mm_man_occupy_node_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_MM_MAN_OCCUPY_NODE, &mm_man_occupy_node_des, LOC_CMON_0028);
        mm_man_occupy_node_init(mm_man_occupy_node_des);

        mm_man_occupy_node_clone(mm_man_occupy_node_src, mm_man_occupy_node_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)mm_man_occupy_node_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_mm_man_load_node_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *mm_man_load_node_vec;
    UINT32   mm_man_load_node_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_mm_man_load_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_mm_man_load_node_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_mm_man_load_node_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    mm_man_load_node_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(mm_man_load_node_pos = 0; mm_man_load_node_pos < cvector_size(mm_man_load_node_vec); mm_man_load_node_pos ++)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node_src;
        MM_MAN_LOAD_NODE *mm_man_load_node_des;

        mm_man_load_node_src = (MM_MAN_LOAD_NODE *)cvector_get(mm_man_load_node_vec, mm_man_load_node_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_MM_MAN_LOAD_NODE, &mm_man_load_node_des, LOC_CMON_0029);
        mm_man_load_node_init(mm_man_load_node_des);

        mm_man_load_node_clone(mm_man_load_node_src, mm_man_load_node_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)mm_man_load_node_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_module_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;
    UINT32   cmon_obj_pos;

    CMON_OBJ *cmon_obj;


    CVECTOR *cproc_module_stat_vec;
    UINT32   cproc_module_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_module_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_module_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }
#if 0
    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_module_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }
#endif
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);
        cproc_module_stat_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

        for(cproc_module_stat_pos = 0; cproc_module_stat_pos < cvector_size(cproc_module_stat_vec); cproc_module_stat_pos ++)
        {
            CPROC_MODULE_STAT *cproc_module_stat_src;
            UINT32             pos;

            cproc_module_stat_src = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec, cproc_module_stat_pos);

            pos = cvector_search_front(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)cproc_module_stat_src, (CVECTOR_DATA_CMP)cproc_module_stat_cmp_type);
            if(CVECTOR_ERR_POS == pos)
            {
                CPROC_MODULE_STAT *cproc_module_stat_des;

                alloc_static_mem(MD_CMON, cmon_md_id, MM_CPROC_MODULE_STAT, &cproc_module_stat_des, LOC_CMON_0030);
                cproc_module_stat_init(cproc_module_stat_des);

                cproc_module_stat_clone(cproc_module_stat_src, cproc_module_stat_des);

                cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)cproc_module_stat_des);
            }
            else
            {
                CPROC_MODULE_STAT *cproc_module_stat_des;

                cproc_module_stat_des = (CPROC_MODULE_STAT *)cvector_get(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), pos);

                CPROC_MODULE_NUM(cproc_module_stat_des) += CPROC_MODULE_NUM(cproc_module_stat_src);
            }
        }
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_csys_eth_stat_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *csys_eth_stat_vec;
    UINT32   csys_eth_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_eth_stat_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_eth_stat_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    csys_eth_stat_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < cvector_size(csys_eth_stat_vec); csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat_src;
        CSYS_ETH_STAT *csys_eth_stat_des;

        csys_eth_stat_src = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec, csys_eth_stat_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_ETH_STAT, &csys_eth_stat_des, LOC_CMON_0031);
        csys_eth_stat_init(csys_eth_stat_des);

        csys_eth_stat_clone(csys_eth_stat_src, csys_eth_stat_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)csys_eth_stat_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_csys_dsk_stat_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;

    CMON_OBJ *cmon_obj;

    CVECTOR *csys_dsk_stat_vec;
    UINT32   csys_dsk_stat_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_csys_dsk_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_dsk_stat_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }

    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_dsk_stat_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }

    cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, 0);
    csys_dsk_stat_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < cvector_size(csys_dsk_stat_vec); csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat_src;
        CSYS_DSK_STAT *csys_dsk_stat_des;

        csys_dsk_stat_src = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec, csys_dsk_stat_pos);

        alloc_static_mem(MD_CMON, cmon_md_id, MM_CSYS_DSK_STAT, &csys_dsk_stat_des, LOC_CMON_0032);
        csys_dsk_stat_init(csys_dsk_stat_des);

        csys_dsk_stat_clone(csys_dsk_stat_src, csys_dsk_stat_des);

        cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)csys_dsk_stat_des);
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_crank_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;
    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_crank_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_crank_task_report_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }
#if 1
    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_crank_task_report_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }
#endif
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        CVECTOR *crank_task_report_vec;
        UINT32   task_report_node_pos;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);
        crank_task_report_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

        for(task_report_node_pos = 0; task_report_node_pos < cvector_size(crank_task_report_vec); task_report_node_pos ++)
        {
            TASK_REPORT_NODE *task_report_node_src;
            TASK_REPORT_NODE *task_report_node_des;

            task_report_node_src = (TASK_REPORT_NODE *)cvector_get(crank_task_report_vec, task_report_node_pos);

            alloc_static_mem(MD_CMON, cmon_md_id, MM_TASK_REPORT_NODE, &task_report_node_des, LOC_CMON_0033);
            task_report_node_init(task_report_node_des);

            task_report_node_clone(task_report_node_src, task_report_node_des);

            cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)task_report_node_des);
        }
    }

    return (0);
}

UINT32 cmon_obj_vec_merge_csys_task_report_vec(const UINT32 cmon_md_id, CMON_OBJ_MERGE *cmon_obj_merge)
{
    CVECTOR *cmon_obj_vec;
    UINT32   cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_csys_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_vec = CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge);

    /*support one and only one vec mounting*/
    if(0 == cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_task_report_vec: cmon_obj_pos_vec of cmon_obj_merge has nothing\n");
        return (0);
    }
#if 0
    if(1 < cvector_size(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "error:cmon_obj_vec_merge_csys_task_report_vec: cmon_obj_merge mount more than one vecs\n");
        return ((UINT32)-1);
    }
#endif
    for(cmon_obj_pos = 0; cmon_obj_pos < cvector_size(cmon_obj_vec); cmon_obj_pos ++)
    {
        CMON_OBJ *cmon_obj;

        CVECTOR *csys_task_report_vec;
        UINT32   task_report_node_pos;

        cmon_obj = (CMON_OBJ *)cvector_get(cmon_obj_vec, cmon_obj_pos);
        csys_task_report_vec = CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);

        for(task_report_node_pos = 0; task_report_node_pos < cvector_size(csys_task_report_vec); task_report_node_pos ++)
        {
            TASK_REPORT_NODE *task_report_node_src;
            TASK_REPORT_NODE *task_report_node_des;

            task_report_node_src = (TASK_REPORT_NODE *)cvector_get(csys_task_report_vec, task_report_node_pos);

            alloc_static_mem(MD_CMON, cmon_md_id, MM_TASK_REPORT_NODE, &task_report_node_des, LOC_CMON_0034);
            task_report_node_init(task_report_node_des);

            task_report_node_clone(task_report_node_src, task_report_node_des);

            cvector_push(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), (void *)task_report_node_des);
        }
    }

    return (0);
}

/**
*
* print the merged CMON_OBJ result
*
**/
UINT32 cmon_obj_vec_merge_print(const UINT32 cmon_md_id, const CMON_OBJ_VEC *cmon_obj_vec, LOG *log)
{
    UINT32 cmon_obj_merge_num;
    UINT32 cmon_obj_merge_pos;

    CVECTOR  *cmon_obj_merge_vec;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(NULL_PTR == CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec))
    {
        sys_log(LOGSTDOUT, "warn:cmon_obj_vec_merge_print: merge vec is null\n");
        return (0);
    }

    cmon_obj_merge_vec = CMON_OBJ_VEC_MERGE_VEC(cmon_obj_vec);
    cmon_obj_merge_num = cvector_size(cmon_obj_merge_vec);
    for(cmon_obj_merge_pos = 0; cmon_obj_merge_pos < cmon_obj_merge_num; cmon_obj_merge_pos ++)
    {
        CMON_OBJ_MERGE *cmon_obj_merge;
        CMON_MAP       *cmon_map;

        cmon_obj_merge = (CMON_OBJ_MERGE *)cvector_get(cmon_obj_merge_vec, cmon_obj_merge_pos);
        if(NULL_PTR == cmon_obj_merge)
        {
            continue;
        }

        cmon_map = CMON_OBJ_MERGE_MAP(cmon_obj_merge);
        if(NULL_PTR == cmon_map)
        {
            continue;
        }

        if(NULL_PTR != CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC(cmon_map))
        {
            CMON_MAP_OBJ_VEC_MERGE_PRINT_FUNC(cmon_map)(cmon_md_id, cmon_obj_merge, log);
        }
    }

    return (0);
}


UINT32 cmon_obj_vec_merge_print_real(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_real: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %.2f\n",
                 CMON_OBJ_MERGE_OID(cmon_obj_merge),
                 MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                 CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge)
                 );
    return (0);
}

UINT32 cmon_obj_vec_merge_print_uint32(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_uint32: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld\n",
                 CMON_OBJ_MERGE_OID(cmon_obj_merge),
                 MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                 CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge)
                 );
    return (0);
}

UINT32 cmon_obj_vec_merge_print_crank_thread_stat(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CRANK_THREAD_STAT *val_crank_thread_stat;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_crank_thread_stat: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    val_crank_thread_stat = CMON_OBJ_MERGE_RESULT_CRANK_THREAD_STAT((CMON_OBJ_MERGE *)cmon_obj_merge);

    sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, max %ld, busy %ld, idle %ld\n",
                 CMON_OBJ_MERGE_OID(cmon_obj_merge),
                 MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                 (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                 CRANK_THREAD_MAX_NUM(val_crank_thread_stat),
                 CRANK_THREAD_BUSY_NUM(val_crank_thread_stat),
                 CRANK_THREAD_IDLE_NUM(val_crank_thread_stat)
                 );
    return (0);
}


UINT32 cmon_obj_vec_merge_print_taskc_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  taskc_node_vec;
    UINT32     taskc_node_pos;
    UINT32     taskc_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_taskc_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    taskc_node_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == taskc_node_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, taskc_node_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    taskc_node_num = cvector_size(taskc_node_vec);
    for(taskc_node_pos = 0; taskc_node_pos < taskc_node_num; taskc_node_pos ++)
    {
        TASKC_NODE *taskc_node;

        taskc_node = (TASKC_NODE *)cvector_get(taskc_node_vec, taskc_node_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: taskc_id %s, taskc_comm %ld, taskc_size %ld\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     taskc_node_pos,
                     TASKC_NODE_TCID_STR(taskc_node), TASKC_NODE_COMM(taskc_node), TASKC_NODE_SIZE(taskc_node)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_csocket_cnode_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  csocket_cnode_vec;
    UINT32     csocket_cnode_pos;
    UINT32     csocket_cnode_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_csocket_cnode_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csocket_cnode_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == csocket_cnode_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, csocket_cnode_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    csocket_cnode_num = cvector_size(csocket_cnode_vec);
    for(csocket_cnode_pos = 0; csocket_cnode_pos < csocket_cnode_num; csocket_cnode_pos ++)
    {
        CSOCKET_CNODE *csocket_cnode;

        csocket_cnode = (CSOCKET_CNODE *)cvector_get(csocket_cnode_vec, csocket_cnode_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: ipaddr %s, srvport %ld, tcid %s, comm %ld, size %ld, sockfd %d\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     csocket_cnode_pos,
                     CSOCKET_CNODE_IPADDR_STR(csocket_cnode), CSOCKET_CNODE_SRVPORT(csocket_cnode),
                     CSOCKET_CNODE_TCID_STR(csocket_cnode), CSOCKET_CNODE_COMM(csocket_cnode), CSOCKET_CNODE_SIZE(csocket_cnode),
                     CSOCKET_CNODE_SOCKFD(csocket_cnode)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_cpu_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  csys_cpu_stat_vec;
    UINT32     csys_cpu_stat_pos;
    UINT32     csys_cpu_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_cpu_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_cpu_stat_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == csys_cpu_stat_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, csys_cpu_stat_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec);
    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat;

        csys_cpu_stat = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: cpu load %.2f\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     csys_cpu_stat_pos,
                     CSYS_CPU_STAT_LOAD(csys_cpu_stat)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_mm_man_occupy_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  mm_man_occupy_node_vec;
    UINT32     mm_man_occupy_node_pos;
    UINT32     mm_man_occupy_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_mm_man_occupy_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mm_man_occupy_node_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == mm_man_occupy_node_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, mm_man_occupy_node_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    mm_man_occupy_node_num = cvector_size(mm_man_occupy_node_vec);
    for(mm_man_occupy_node_pos = 0; mm_man_occupy_node_pos < mm_man_occupy_node_num; mm_man_occupy_node_pos ++)
    {
        MM_MAN_OCCUPY_NODE *mm_man_occupy_node;

        mm_man_occupy_node = (MM_MAN_OCCUPY_NODE *)cvector_get(mm_man_occupy_node_vec, mm_man_occupy_node_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, manager %ld#: nodesum = %8ld, maxused = %8ld, curused = %8ld\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node),
                     MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node),
                     MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node),
                     MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_mm_man_load_node_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  mm_man_load_node_vec;
    UINT32     mm_man_load_node_pos;
    UINT32     mm_man_load_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_mm_man_load_node_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    mm_man_load_node_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == mm_man_load_node_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, mm_man_load_node_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    mm_man_load_node_num = cvector_size(mm_man_load_node_vec);
    for(mm_man_load_node_pos = 0; mm_man_load_node_pos < mm_man_load_node_num; mm_man_load_node_pos ++)
    {
        MM_MAN_LOAD_NODE *mm_man_load_node;

        mm_man_load_node = (MM_MAN_LOAD_NODE *)cvector_get(mm_man_load_node_vec, mm_man_load_node_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, manager %ld#: maxused load = %.2f, curused load = %.2f\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     MM_MAN_LOAD_NODE_TYPE(mm_man_load_node),
                     MM_MAN_LOAD_NODE_MAX(mm_man_load_node),
                     MM_MAN_LOAD_NODE_CUR(mm_man_load_node)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_module_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  cproc_module_stat_vec;
    UINT32     cproc_module_stat_pos;
    UINT32     cproc_module_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_module_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cproc_module_stat_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == cproc_module_stat_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, cproc_module_stat_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    cproc_module_stat_num = cvector_size(cproc_module_stat_vec);
    for(cproc_module_stat_pos = 0; cproc_module_stat_pos < cproc_module_stat_num; cproc_module_stat_pos ++)
    {
        CPROC_MODULE_STAT *cproc_module_stat;

        cproc_module_stat = (CPROC_MODULE_STAT *)cvector_get(cproc_module_stat_vec, cproc_module_stat_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: module type %ld, module num %ld\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     cproc_module_stat_pos,
                     CPROC_MODULE_TYPE(cproc_module_stat),
                     CPROC_MODULE_NUM(cproc_module_stat)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_csys_eth_stat_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  csys_eth_stat_vec;
    UINT32     csys_eth_stat_pos;
    UINT32     csys_eth_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_csys_eth_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_eth_stat_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == csys_eth_stat_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, csys_eth_stat_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    csys_eth_stat_num = cvector_size(csys_eth_stat_vec);
    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat;

        csys_eth_stat = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec, csys_eth_stat_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: %s, rxflow %ld, txflow %ld\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     csys_eth_stat_pos,
                     (char *)cstring_get_str(CSYS_ETH_NAME(csys_eth_stat)),
                     CSYS_ETH_RXTHROUGHPUT(csys_eth_stat),
                     CSYS_ETH_TXTHROUGHPUT(csys_eth_stat)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_csys_dsk_stat_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  csys_dsk_stat_vec;
    UINT32     csys_dsk_stat_pos;
    UINT32     csys_dsk_stat_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_csys_dsk_stat_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    csys_dsk_stat_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == csys_dsk_stat_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, csys_dsk_stat_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    csys_dsk_stat_num = cvector_size(csys_dsk_stat_vec);
    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < csys_dsk_stat_num; csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat;

        csys_dsk_stat = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec, csys_dsk_stat_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: size %ld MBytes, used %ld MBytes, aval %ld MBytes, load %.2f%%\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                     csys_dsk_stat_pos,
                     (char *)cstring_get_str(CSYS_DSK_NAME(csys_dsk_stat)),
                     CSYS_DSK_SIZE(csys_dsk_stat),
                     CSYS_DSK_USED(csys_dsk_stat),
                     CSYS_DSK_AVAL(csys_dsk_stat),
                     CSYS_DSK_LOAD(csys_dsk_stat)
                 );
    }
    return (0);
}

UINT32 cmon_obj_vec_merge_print_crank_task_report_vec(const UINT32 cmon_md_id, const CMON_OBJ_MERGE *cmon_obj_merge, LOG *log)
{
    CVECTOR *  crank_task_report_vec;
    UINT32     task_report_node_pos;
    UINT32     task_report_node_num;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_vec_merge_print_crank_task_report_vec: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    crank_task_report_vec = CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge);
    if(NULL_PTR == crank_task_report_vec)
    {
        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, crank_task_report_vec is null\n",
                     CMON_OBJ_MERGE_OID(cmon_obj_merge),
                     MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                     (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge)
                 );
        return (0);
    }

    task_report_node_num = cvector_size(crank_task_report_vec);
    for(task_report_node_pos = 0; task_report_node_pos < task_report_node_num; task_report_node_pos ++)
    {
        TASK_REPORT_NODE *task_report_node;

        task_report_node = (TASK_REPORT_NODE *)cvector_get(crank_task_report_vec, task_report_node_pos);

        sys_log(log, "[oid %ld, tcid %s, comm %ld, rank %ld, modi %ld] name %s, %ld #: start at %4d-%02d-%02d %02d:%02d:%02d, end at %4d-%02d-%02d %02d:%02d:%02d, time to live %ld, seqno %lx.%lx.%lx, wait flag %ld, need rsp flag %ld, reschedule flag %ld, req num %ld, need rsp %ld, succ rsp %ld, fail rsp %ld, sent req %ld, discard req %ld, timeout req %ld\n",
                        CMON_OBJ_MERGE_OID(cmon_obj_merge),
                        MOD_NODE_TCID_STR(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                        MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                        MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                        MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge)),
                        (char *)CMON_OBJ_MERGE_NAME_STR(cmon_obj_merge),
                        task_report_node_pos,
                        TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_START_TIME(task_report_node)),
                        TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_START_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_START_TIME(task_report_node)),

                        TASK_TIME_FMT_YEAR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MONTH(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MDAY(TASK_REPORT_NODE_END_TIME(task_report_node)),
                        TASK_TIME_FMT_HOUR(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_MIN(TASK_REPORT_NODE_END_TIME(task_report_node)), TASK_TIME_FMT_SEC(TASK_REPORT_NODE_END_TIME(task_report_node)),

                        TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node),
                        TASK_REPORT_NODE_TCID(task_report_node), TASK_REPORT_NODE_RANK(task_report_node), TASK_REPORT_NODE_SEQNO(task_report_node),

                        TASK_REPORT_NODE_WAIT_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node), TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node),

                        TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node),
                        TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node),
                        TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node),
                        TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node),
                        TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node),
                        TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node),
                        TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node)
                 );
    }
    return (0);
}


/**
*
* new a CMON_OBJ_MERGE
*
**/
CMON_OBJ_MERGE * cmon_obj_merge_new(const UINT32 cmon_md_id, const UINT32 cmon_oid, const MOD_NODE *mod_node)
{
    CMON_MAP *cmon_map;
    CMON_OBJ_MERGE * cmon_obj_merge;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_new: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    alloc_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ_MERGE, &cmon_obj_merge, LOC_CMON_0035);

    cmon_map = cmon_map_fetch(cmon_md_id, cmon_oid);
    CMON_MAP_MERGE_INIT_FUNC(cmon_map)(cmon_md_id, cmon_map, mod_node, cmon_obj_merge);

    return (cmon_obj_merge);
}

/**
*
* initialize a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_uint32_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge)
{
    MOD_NODE *this_mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_uint32_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    this_mod_node = CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge);

    CMON_OBJ_MERGE_OID(cmon_obj_merge) = CMON_MAP_OID(cmon_map);
    CMON_OBJ_MERGE_MAP(cmon_obj_merge) = (CMON_MAP *)cmon_map;

    MOD_NODE_TCID(this_mod_node) = MOD_NODE_TCID(mod_node);
    MOD_NODE_COMM(this_mod_node) = MOD_NODE_COMM(mod_node);
    MOD_NODE_RANK(this_mod_node) = MOD_NODE_RANK(mod_node);
    MOD_NODE_MODI(this_mod_node) = MOD_NODE_MODI(mod_node);

    cstring_init(CMON_OBJ_MERGE_NAME(cmon_obj_merge), NULL_PTR);
    cvector_init(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CMON_0036);
    CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) = 0;
    return (0);
}

UINT32 cmon_obj_merge_real_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge)
{
    MOD_NODE *this_mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_real_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    this_mod_node = CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge);

    CMON_OBJ_MERGE_OID(cmon_obj_merge) = CMON_MAP_OID(cmon_map);
    CMON_OBJ_MERGE_MAP(cmon_obj_merge) = (CMON_MAP *)cmon_map;

    MOD_NODE_TCID(this_mod_node) = MOD_NODE_TCID(mod_node);
    MOD_NODE_COMM(this_mod_node) = MOD_NODE_COMM(mod_node);
    MOD_NODE_RANK(this_mod_node) = MOD_NODE_RANK(mod_node);
    MOD_NODE_MODI(this_mod_node) = MOD_NODE_MODI(mod_node);

    cstring_init(CMON_OBJ_MERGE_NAME(cmon_obj_merge), NULL_PTR);
    cvector_init(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CMON_0037);
    CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) = 0.0;
    return (0);
}

UINT32 cmon_obj_merge_vec_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge)
{
    MOD_NODE *this_mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_vec_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    this_mod_node = CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge);

    CMON_OBJ_MERGE_OID(cmon_obj_merge) = CMON_MAP_OID(cmon_map);
    CMON_OBJ_MERGE_MAP(cmon_obj_merge) = (CMON_MAP *)cmon_map;

    MOD_NODE_TCID(this_mod_node) = MOD_NODE_TCID(mod_node);
    MOD_NODE_COMM(this_mod_node) = MOD_NODE_COMM(mod_node);
    MOD_NODE_RANK(this_mod_node) = MOD_NODE_RANK(mod_node);
    MOD_NODE_MODI(this_mod_node) = MOD_NODE_MODI(mod_node);

    cstring_init(CMON_OBJ_MERGE_NAME(cmon_obj_merge), NULL_PTR);
    cvector_init(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CMON_0038);
    cvector_init(CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge), 0, CMON_MAP_MERGE_VEC_ITEM_TYPE(cmon_map), CVECTOR_LOCK_ENABLE, LOC_CMON_0039);
    return (0);
}

UINT32 cmon_obj_merge_crank_thread_stat_init(const UINT32 cmon_md_id, const CMON_MAP *cmon_map, const MOD_NODE *mod_node,CMON_OBJ_MERGE * cmon_obj_merge)
{
    MOD_NODE *this_mod_node;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_crank_thread_stat_init: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    this_mod_node = CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge);

    CMON_OBJ_MERGE_OID(cmon_obj_merge) = CMON_MAP_OID(cmon_map);
    CMON_OBJ_MERGE_MAP(cmon_obj_merge) = (CMON_MAP *)cmon_map;

    MOD_NODE_TCID(this_mod_node) = MOD_NODE_TCID(mod_node);
    MOD_NODE_COMM(this_mod_node) = MOD_NODE_COMM(mod_node);
    MOD_NODE_RANK(this_mod_node) = MOD_NODE_RANK(mod_node);
    MOD_NODE_MODI(this_mod_node) = MOD_NODE_MODI(mod_node);

    cstring_init(CMON_OBJ_MERGE_NAME(cmon_obj_merge), NULL_PTR);
    cvector_init(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), 0, MM_UINT32, CVECTOR_LOCK_ENABLE, LOC_CMON_0040);

    crank_thread_stat_init(CMON_OBJ_MERGE_RESULT_CRANK_THREAD_STAT(cmon_obj_merge));
    return (0);
}

/**
*
* clean result area of a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_result_clean(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge)
{
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_result_clean: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_map = CMON_OBJ_MERGE_MAP(cmon_obj_merge);
    switch(CMON_MAP_MERGE_VEC_ITEM_TYPE(cmon_map))
    {
        case MM_UINT32:
            CMON_OBJ_MERGE_RESULT_UINT32(cmon_obj_merge) = 0;
            break;
        case MM_REAL:
            CMON_OBJ_MERGE_RESULT_REAL(cmon_obj_merge) = 0.0;
            break;
        case MM_TASKC_NODE:
        case MM_CSOCKET_CNODE:
        case MM_CSYS_CPU_STAT:
        case MM_MM_MAN_OCCUPY_NODE:
        case MM_MM_MAN_LOAD_NODE:
        case MM_CPROC_MODULE_STAT:
        case MM_CSYS_ETH_STAT:
        case MM_CSYS_DSK_STAT:
        case MM_TASK_REPORT_NODE:
            cvector_clean_0(0, CMON_OBJ_MERGE_RESULT_VEC(cmon_obj_merge));
            break;
        case MM_CRANK_THREAD_STAT:
            crank_thread_stat_clean(CMON_OBJ_MERGE_RESULT_CRANK_THREAD_STAT(cmon_obj_merge));
            break;
        default:
            sys_log(LOGSTDOUT, "error:cmon_obj_merge_result_clean:unknow CMON_MAP_MERGE_VEC_ITEM_TYPE %ld\n",
                                CMON_MAP_MERGE_VEC_ITEM_TYPE(cmon_map));
    }

    return (0);
}

/**
*
* purge cmon_obj from CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_purge(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
    //UINT32 cmon_obj_pos;

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_purge: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cvector_delete(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), (void *)cmon_obj);

    return (0);
}

/**
*
* check the merged cmon obj vec is empty or not
*
**/
EC_BOOL cmon_obj_merge_is_empty(const UINT32 cmon_md_id, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_is_empty: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    return cvector_is_empty(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge));
}

/**
*
* clean a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_clean(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge)
{

#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_clean: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    /*first clean up result area due to cmon id/cmon map info will be used here and will be cleanup later*/
    cmon_obj_merge_result_clean(cmon_md_id, cmon_obj_merge);

    CMON_OBJ_MERGE_OID(cmon_obj_merge) = CMON_OID_UNDEF;
    CMON_OBJ_MERGE_MAP(cmon_obj_merge) = NULL_PTR;
    mod_node_init(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge));

    cstring_clean(CMON_OBJ_MERGE_NAME(cmon_obj_merge));
    cvector_clean(CMON_OBJ_MERGE_OBJ_VEC(cmon_obj_merge), NULL_PTR, LOC_CMON_0041);

    return (0);
}

/**
*
* free a CMON_OBJ_MERGE
*
**/
UINT32 cmon_obj_merge_free(const UINT32 cmon_md_id, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_free: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cmon_obj_merge_clean(cmon_md_id, cmon_obj_merge);
    free_static_mem(MD_CMON, cmon_md_id, MM_CMON_OBJ_MERGE, cmon_obj_merge, LOC_CMON_0042);
    return (0);
}

/**
*
* determine a CMON_OBJ belong to this CMON_OBJ_MGERGE or not
*   o: oid
*   t: tcid
*   c: comm
*   r: rank
*   m: modi
*
**/
EC_BOOL cmon_obj_merge_match_otcrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_otcrm: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_MODI(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_otcr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_otcr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_otc(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_otc: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_COMM(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_ot(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_ot: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_o(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_o: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_otrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_otrm: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_MODI(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_MODI(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_match_otr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, const CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_match_otr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(
         CMON_OBJ_OID(cmon_obj) == CMON_OBJ_MERGE_OID(cmon_obj_merge)
      && MOD_NODE_TCID(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_TCID(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
      && MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)) == MOD_NODE_RANK(CMON_OBJ_MERGE_MOD_NODE(cmon_obj_merge))
    )
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cmon_obj_merge_filter_fwd_rank(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_filter_fwd_rank: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    if(CMPI_FWD_RANK == MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

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
UINT32 cmon_obj_merge_name_otcrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_otcrm: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s,%ld,%ld,%ld",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_MODI(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}

UINT32 cmon_obj_merge_name_otcr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_otcr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s,%ld,%ld",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}

UINT32 cmon_obj_merge_name_otc(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_otc: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s,%ld",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_COMM(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}

UINT32 cmon_obj_merge_name_ot(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_ot: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}

UINT32 cmon_obj_merge_name_otrm(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_otrm: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s,%ld,%ld",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_MODI(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}

UINT32 cmon_obj_merge_name_otr(const UINT32 cmon_md_id, const CMON_OBJ *cmon_obj, CMON_OBJ_MERGE * cmon_obj_merge)
{
#if ( SWITCH_ON == CMON_DEBUG_SWITCH )
    if ( CMON_MD_ID_CHECK_INVALID(cmon_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:cmon_obj_merge_name_otr: cmon module #0x%lx not started.\n",
                cmon_md_id);
        dbg_exit(MD_CMON, cmon_md_id);
    }
#endif/*CMON_DEBUG_SWITCH*/

    cstring_format(CMON_OBJ_MERGE_NAME(cmon_obj_merge), "%ld,%s,%ld",
                    CMON_OBJ_OID(cmon_obj),
                    MOD_NODE_TCID_STR(CMON_OBJ_MOD_NODE(cmon_obj)),
                    MOD_NODE_RANK(CMON_OBJ_MOD_NODE(cmon_obj))
                    );

    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/


