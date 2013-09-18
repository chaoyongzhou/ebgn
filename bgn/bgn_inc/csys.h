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

#ifndef _CSYS_H
#define _CSYS_H

#include <stdio.h>
#include <stdlib.h>

#include "type.h"
#include "cstring.h"
#include "cvector.h"

#include "log.h"

#include "task.inc"

#define CSYS_SHELL_BUF_MAX_SIZE             ((UINT32) 1 * 1024 * 1024)

/*file: /proc/cpuinfo*/
typedef CVECTOR CSYS_CPU_CFG_VEC; /*data item type is MM_CSTRING*/
#define CSYS_CPU_CFG_VEC_INFO(csys_cpu_cfg_vec)      (csys_cpu_cfg_vec)

/*file: /proc/loadavg */
typedef CVECTOR CSYS_CPU_LOAD_AVG;/*data item type is MM_REAL*/
#define CSYS_CPU_01_MIN_LOAD_POS   ((UINT32) 0)
#define CSYS_CPU_05_MIN_LOAD_POS   ((UINT32) 5)
#define CSYS_CPU_15_MIN_LOAD_POS   ((UINT32)15)

/*file: /proc/stat */
typedef struct
{  
    CSTRING  cstr;
    UINT32   user;
    UINT32   nice;
    UINT32   sys;
    UINT32   idle;
    UINT32   total;

    REAL     load; /*private data without encoding & decoding*/
}CSYS_CPU_STAT;

#define CSYS_CPU_STAT_CSTR(csys_cpu_stat)         (&((csys_cpu_stat)->cstr))
#define CSYS_CPU_STAT_USER(csys_cpu_stat)         ((csys_cpu_stat)->user)
#define CSYS_CPU_STAT_NICE(csys_cpu_stat)         ((csys_cpu_stat)->nice)
#define CSYS_CPU_STAT_SYS(csys_cpu_stat)          ((csys_cpu_stat)->sys)
#define CSYS_CPU_STAT_IDLE(csys_cpu_stat)         ((csys_cpu_stat)->idle)
#define CSYS_CPU_STAT_TOTAL(csys_cpu_stat)        ((csys_cpu_stat)->total)
#define CSYS_CPU_STAT_LOAD(csys_cpu_stat)         ((csys_cpu_stat)->load)

typedef CVECTOR CSYS_CPU_STAT_VEC;/*data item type is CSYS_CPU_STAT*/
#define CSYS_CPU_LOAD_VEC_INFO(csys_cpu_stat_vec)       (csys_cpu_stat_vec)

typedef struct
{
    REAL  avg_01_min;
    REAL  avg_05_min;
    REAL  avg_15_min;
}CSYS_CPU_AVG_STAT;

#define CSYS_CPU_AVG_STAT_01_MIN(csys_cpu_avg_stat)     ((csys_cpu_avg_stat)->avg_01_min)
#define CSYS_CPU_AVG_STAT_05_MIN(csys_cpu_avg_stat)     ((csys_cpu_avg_stat)->avg_05_min)
#define CSYS_CPU_AVG_STAT_15_MIN(csys_cpu_avg_stat)     ((csys_cpu_avg_stat)->avg_15_min)

typedef struct
{
    UINT32   mem_total;
    UINT32   mem_free;
}CSYS_MEM_STAT;

#define CSYS_MEM_TOTAL(csys_mem_stat)       ((csys_mem_stat)->mem_total)
#define CSYS_MEM_FREE(csys_mem_stat)        ((csys_mem_stat)->mem_free)
    
typedef struct
{
    UINT32        mem_occupy;
    REAL          mem_load;
}CPROC_MEM_STAT;
#define CPROC_MEM_OCCUPY(cproc_mem_stat)   ((cproc_mem_stat)->mem_occupy)
#define CPROC_MEM_LOAD(cproc_mem_stat)     ((cproc_mem_stat)->mem_load)

typedef struct
{
    UINT32 thread_num;
}CPROC_THREAD_STAT;
#define CPROC_THREAD_NUM(cproc_thread_stat) ((cproc_thread_stat)->thread_num)

typedef struct
{
    UINT32 max_thread_num;
    UINT32 busy_thread_num;
    UINT32 idle_thread_num;
}CRANK_THREAD_STAT;
#define CRANK_THREAD_MAX_NUM(crank_thread_stat)      ((crank_thread_stat)->max_thread_num)
#define CRANK_THREAD_BUSY_NUM(crank_thread_stat)     ((crank_thread_stat)->busy_thread_num)
#define CRANK_THREAD_IDLE_NUM(crank_thread_stat)     ((crank_thread_stat)->idle_thread_num)

typedef struct
{
    REAL        cpu_load;
}CPROC_CPU_STAT;
#define CPROC_CPU_LOAD(cproc_cpu_stat)   ((cproc_cpu_stat)->cpu_load)

typedef struct
{
    UINT32 module_type;
    UINT32 module_num;
}CPROC_MODULE_STAT;
#define CPROC_MODULE_TYPE(cproc_module_stat)     ((cproc_module_stat)->module_type)
#define CPROC_MODULE_NUM(cproc_module_stat)      ((cproc_module_stat)->module_num)

typedef CVECTOR CPROC_MODULE_STAT_VEC;/*data item type is CPROC_MODULE_STAT*/
#define CPROC_MODULE_VEC_INFO(cproc_module_stat_vec)       (cproc_module_stat_vec)

typedef struct
{
    CSTRING eth_name;
    UINT32  eth_speed; /*Mb/s*/
    UINT32  eth_rxmoct;/*in KBytes*/
    UINT32  eth_txmoct;/*in KBytes*/
    UINT32  eth_rxflow;/*in KBytes*/
    UINT32  eth_txflow;/*in KBytes*/
}CSYS_ETH_STAT;

#define CSYS_ETH_NAME(csys_eth_stat)               (&((csys_eth_stat)->eth_name))
#define CSYS_ETH_SPEEDMBS(csys_eth_stat)           ((csys_eth_stat)->eth_speed)
#define CSYS_ETH_RXMOCT(csys_eth_stat)             ((csys_eth_stat)->eth_rxmoct)
#define CSYS_ETH_TXMOCT(csys_eth_stat)             ((csys_eth_stat)->eth_txmoct)
#define CSYS_ETH_RXTHROUGHPUT(csys_eth_stat)       ((csys_eth_stat)->eth_rxflow)
#define CSYS_ETH_TXTHROUGHPUT(csys_eth_stat)       ((csys_eth_stat)->eth_txflow)

typedef CVECTOR CSYS_ETH_VEC;/*data item type is CSYS_ETH_STAT*/
#define CSYS_ETH_VEC_INFO(csys_eth_stat_vec)       (csys_eth_stat_vec)

typedef struct
{
    CSTRING dsk_name;/*Filesystem*/
    UINT32  dsk_size;/*in MB*/
    UINT32  dsk_used;/*in MB*/
    UINT32  dsk_aval;/*in MB*/
    REAL    dsk_load;/*percent(%)*/
}CSYS_DSK_STAT;

#define CSYS_DSK_NAME(csys_dsk_stat)      (&((csys_dsk_stat)->dsk_name))
#define CSYS_DSK_SIZE(csys_dsk_stat)      ((csys_dsk_stat)->dsk_size)
#define CSYS_DSK_USED(csys_dsk_stat)      ((csys_dsk_stat)->dsk_used)
#define CSYS_DSK_AVAL(csys_dsk_stat)      ((csys_dsk_stat)->dsk_aval)
#define CSYS_DSK_LOAD(csys_dsk_stat)      ((csys_dsk_stat)->dsk_load)

typedef CVECTOR CSYS_DSK_VEC;/*data item type is CSYS_DSK_STAT*/
#define CSYS_DSK_VEC_INFO(csys_dsk_stat_vec)       (csys_dsk_stat_vec)

typedef CVECTOR CRANK_TASK_REPORT_VEC;/*data item type is TASK_REPORT_NODE*/
#define CRANK_TASK_VEC_INFO(crank_task_report_vec)       (crank_task_report_vec)

/*
[root@node129 test]# top -b -n 1 -p 2358
top - 21:28:35 up  2:43,  7 users,  load average: 0.00, 0.01, 0.00
Tasks:   1 total,   0 running,   1 sleeping,   0 stopped,   0 zombie
Cpu(s):  0.3% us,  1.4% sy,  0.0% ni, 97.3% id,  0.8% wa,  0.2% hi,  0.0% si
Mem:    808112k total,   564864k used,   243248k free,    96524k buffers
Swap:   524280k total,        0k used,   524280k free,   341696k cached

  PID USER      PR  NI  VIRT  RES  SHR S %CPU %MEM    TIME+  COMMAND                                                                                                             
 2358 root      15   0     0    0    0 S  0.0  0.0   0:00.00 vmmemctl 
*/
typedef struct
{
    UINT32    pid;
    UINT8  *  usr;
    UINT32    pr;
    UINT32    ni;
    UINT32    virt;
    UINT32    res;
    UINT32    shr;
    UINT8  *  status;
    REAL      cpu_load;
    REAL      mem_load;
}CTOP_OLINE; /*top command output line*/


UINT32 csys_info_print();

void csys_test();

/*------------------------------- CSYS_CPU_CFG_VEC interface -------------------------------*/
CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec_new();

UINT32 csys_cpu_cfg_vec_init(CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec);

UINT32 csys_cpu_cfg_vec_clean(CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec);

UINT32 csys_cpu_cfg_vec_free(CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec);

void   csys_cpu_cfg_vec_print(LOG *log, const CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec);

UINT32 csys_cpu_cfg_vec_get(CSYS_CPU_CFG_VEC *csys_cpu_cfg_vec);

/*------------------------------- CSYS_CPU_STAT interface -------------------------------*/

CSYS_CPU_STAT *csys_cpu_stat_new();

UINT32 csys_cpu_stat_init(CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_clean(CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_free(CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_get(const char *buff, CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_clone(CSYS_CPU_STAT *csys_cpu_stat_src, CSYS_CPU_STAT *csys_cpu_stat_des);

void   csys_cpu_stat_print(LOG *log, const CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_init_0(const UINT32 md_id, CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_clean_0(const UINT32 md_id, CSYS_CPU_STAT *csys_cpu_stat);

UINT32 csys_cpu_stat_free_0(const UINT32 md_id, CSYS_CPU_STAT *csys_cpu_stat);

/*------------------------------- CSYS_CPU_STAT_VEC interface -------------------------------*/
CSYS_CPU_STAT_VEC *csys_cpu_stat_vec_new();

UINT32 csys_cpu_stat_vec_init(CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

UINT32 csys_cpu_stat_vec_clean(CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

UINT32 csys_cpu_stat_vec_free(CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

UINT32 csys_cpu_stat_vec_get(CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

UINT32 csys_cpu_stat_vec_size(const CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

CSYS_CPU_STAT * csys_cpu_stat_vec_fetch(const CSYS_CPU_STAT_VEC *csys_cpu_stat_vec, const UINT32 csys_cpu_stat_pos);

void   csys_cpu_stat_vec_print(LOG *log, const CSYS_CPU_STAT_VEC *csys_cpu_stat_vec);

/*------------------------------- CSYS_CPU_AVG_STAT interface -------------------------------*/
CSYS_CPU_AVG_STAT *csys_cpu_avg_stat_new();

UINT32 csys_cpu_avg_stat_init(CSYS_CPU_AVG_STAT *csys_cpu_avg_stat);

UINT32 csys_cpu_avg_stat_clean(CSYS_CPU_AVG_STAT *csys_cpu_avg_stat);

UINT32 csys_cpu_avg_stat_free(CSYS_CPU_AVG_STAT *csys_cpu_avg_stat);

void csys_cpu_avg_stat_print(LOG *log, const CSYS_CPU_AVG_STAT *csys_cpu_avg_stat);

UINT32 csys_cpu_avg_stat_get(CSYS_CPU_AVG_STAT *csys_cpu_avg_stat);

/*------------------------------- CSYS_MEM_STAT interface -------------------------------*/
CSYS_MEM_STAT *csys_mem_stat_new();

UINT32 csys_mem_stat_init(CSYS_MEM_STAT *csys_mem_stat);

UINT32 csys_mem_stat_clean(CSYS_MEM_STAT *csys_mem_stat);

UINT32 csys_mem_stat_free(CSYS_MEM_STAT *csys_mem_stat);

UINT32 csys_mem_stat_get(CSYS_MEM_STAT *csys_mem_stat);

void   csys_mem_stat_print(LOG *log, const CSYS_MEM_STAT *csys_mem_stat);

/*------------------------------- CPROC_MEM_STAT interface -------------------------------*/
CPROC_MEM_STAT *cproc_mem_stat_new();

UINT32 cproc_mem_stat_init(CPROC_MEM_STAT *cproc_mem_stat);

UINT32 cproc_mem_stat_clean(CPROC_MEM_STAT *cproc_mem_stat);

UINT32 cproc_mem_stat_free(CPROC_MEM_STAT *cproc_mem_stat);

void cproc_mem_stat_print(LOG *log, const CPROC_MEM_STAT *cproc_mem_stat);

UINT32 cproc_mem_stat_get(CPROC_MEM_STAT *cproc_mem_stat);

/*------------------------------- CPROC_CPU_STAT interface -------------------------------*/
CPROC_CPU_STAT *cproc_cpu_stat_new();

UINT32 cproc_cpu_stat_init(CPROC_CPU_STAT *cproc_cpu_stat);

UINT32 cproc_cpu_stat_clean(CPROC_CPU_STAT *cproc_cpu_stat);

UINT32 cproc_cpu_stat_free(CPROC_CPU_STAT *cproc_cpu_stat);

void cproc_cpu_stat_print(LOG *log, const CPROC_CPU_STAT *cproc_cpu_stat);

UINT32 cproc_cpu_stat_get(CPROC_CPU_STAT *cproc_cpu_stat);

/*------------------------------- CTOP_OLINE interface -------------------------------*/
UINT32 ctop_process_stat(const UINT32 pid, CTOP_OLINE *ctop_oline);

/*------------------------------- CPROC_THREAD_STAT interface -------------------------------*/
CPROC_THREAD_STAT *cproc_thread_stat_new();

UINT32 cproc_thread_stat_init(CPROC_THREAD_STAT *cproc_thread_stat);

UINT32 cproc_thread_stat_clean(CPROC_THREAD_STAT *cproc_thread_stat);

UINT32 cproc_thread_stat_free(CPROC_THREAD_STAT *cproc_thread_stat);

void cproc_thread_stat_print(LOG *log, const CPROC_THREAD_STAT *cproc_thread_stat);

UINT32 cproc_thread_stat_get(CPROC_THREAD_STAT *cproc_thread_stat);

/*------------------------------- CPROC_MODULE_STAT interface -------------------------------*/
CPROC_MODULE_STAT *cproc_module_stat_new();
UINT32 cproc_module_stat_init(CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_clean(CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_free(CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_init_0(const UINT32 md_id, CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_clean_0(const UINT32 md_id, CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_free_0(const UINT32 md_id, CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_clone(CPROC_MODULE_STAT *cproc_module_stat_src, CPROC_MODULE_STAT *cproc_module_stat_des);

EC_BOOL cproc_module_stat_cmp_type(const CPROC_MODULE_STAT *cproc_module_stat_1st, const CPROC_MODULE_STAT *cproc_module_stat_2nd);

void cproc_module_stat_print(LOG *log, const CPROC_MODULE_STAT *cproc_module_stat);

UINT32 cproc_module_stat_get(CPROC_MODULE_STAT *cproc_module_stat);

/*------------------------------- CPROC_MODULE_STAT_VEC interface -------------------------------*/
CPROC_MODULE_STAT_VEC *cproc_module_stat_vec_new();

UINT32 cproc_module_stat_vec_init(CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

UINT32 cproc_module_stat_vec_clean(CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

UINT32 cproc_module_stat_vec_free(CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

UINT32 cproc_module_stat_vec_size(const CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

CPROC_MODULE_STAT * cproc_module_stat_vec_fetch(const CPROC_MODULE_STAT_VEC *cproc_module_stat_vec, const UINT32 cproc_module_stat_pos);

void cproc_module_stat_vec_print(LOG *log, const CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

UINT32 cproc_module_stat_vec_get(CPROC_MODULE_STAT_VEC *cproc_module_stat_vec);

/*------------------------------- CRANK_THREAD_STAT interface -------------------------------*/
CRANK_THREAD_STAT *crank_thread_stat_new();

UINT32 crank_thread_stat_init(CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_clean(CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_free(CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_clone(const CRANK_THREAD_STAT *crank_thread_stat_src, CRANK_THREAD_STAT *crank_thread_stat_des);

void crank_thread_stat_print(LOG *log, const CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_get(CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_init_0(const UINT32 md_id, CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_clean_0(const UINT32 md_id, CRANK_THREAD_STAT *crank_thread_stat);

UINT32 crank_thread_stat_free_0(const UINT32 md_id, CRANK_THREAD_STAT *crank_thread_stat);

/*------------------------------- CSYS_ETH_STAT interface -------------------------------*/
CSYS_ETH_STAT *csys_eth_stat_new();

UINT32 csys_eth_stat_init(CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_clean(CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_free(CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_init_0(const UINT32 md_id, CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_clean_0(const UINT32 md_id, CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_free_0(const UINT32 md_id, CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_clone(CSYS_ETH_STAT *csys_eth_stat_src, CSYS_ETH_STAT *csys_eth_stat_des);

void csys_eth_stat_print(LOG *log, const CSYS_ETH_STAT *csys_eth_stat);

UINT32 csys_eth_stat_get(char *buff, CSYS_ETH_STAT *csys_eth_stat);

/*------------------------------- CSYS_ETH_VEC interface -------------------------------*/
CSYS_ETH_VEC *csys_eth_stat_vec_new();

UINT32 csys_eth_stat_vec_init(CSYS_ETH_VEC *csys_eth_stat_vec);

UINT32 csys_eth_stat_vec_clean(CSYS_ETH_VEC *csys_eth_stat_vec);

UINT32 csys_eth_stat_vec_free(CSYS_ETH_VEC *csys_eth_stat_vec);

UINT32 csys_eth_stat_vec_size(const CSYS_ETH_VEC *csys_eth_stat_vec);

CSYS_ETH_STAT * csys_eth_stat_vec_fetch(const CSYS_ETH_VEC *csys_eth_stat_vec, const UINT32 csys_eth_stat_pos);

void csys_eth_stat_vec_print(LOG *log, const CSYS_ETH_VEC *csys_eth_stat_vec);

UINT32 csys_eth_stat_vec_get(CSYS_ETH_VEC *csys_eth_stat_vec);

/*------------------------------- CSYS_DSK_STAT interface -------------------------------*/
CSYS_DSK_STAT *csys_dsk_stat_new();

UINT32 csys_dsk_stat_init(CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_clean(CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_free(CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_init_0(const UINT32 md_id, CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_clean_0(const UINT32 md_id, CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_free_0(const UINT32 md_id, CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_clone(CSYS_DSK_STAT *csys_dsk_stat_src, CSYS_DSK_STAT *csys_dsk_stat_des);

void csys_dsk_stat_print(LOG *log, const CSYS_DSK_STAT *csys_dsk_stat);

UINT32 csys_dsk_stat_get(char *buff, CSYS_DSK_STAT *csys_dsk_stat);

/*------------------------------- CSYS_DSK_VEC interface -------------------------------*/
CSYS_DSK_VEC *csys_dsk_stat_vec_new();

UINT32 csys_dsk_stat_vec_init(CSYS_DSK_VEC *csys_dsk_stat_vec);

UINT32 csys_dsk_stat_vec_clean(CSYS_DSK_VEC *csys_dsk_stat_vec);

UINT32 csys_dsk_stat_vec_free(CSYS_DSK_VEC *csys_dsk_stat_vec);

UINT32 csys_dsk_stat_vec_size(const CSYS_DSK_VEC *csys_dsk_stat_vec);

CSYS_DSK_STAT * csys_dsk_stat_vec_fetch(const CSYS_DSK_VEC *csys_dsk_stat_vec, const UINT32 csys_dsk_stat_pos);

void csys_dsk_stat_vec_print(LOG *log, const CSYS_DSK_VEC *csys_dsk_stat_vec);

UINT32 csys_dsk_stat_vec_get(CSYS_DSK_VEC *csys_dsk_stat_vec);

/*------------------------------- CRANK_TASK_REPORT_VEC interface -------------------------------*/
CRANK_TASK_REPORT_VEC *crank_task_report_vec_new();

UINT32 crank_task_report_vec_init(CRANK_TASK_REPORT_VEC *crank_task_report_vec);

UINT32 crank_task_report_vec_clean(CRANK_TASK_REPORT_VEC *crank_task_report_vec);

UINT32 crank_task_report_vec_free(CRANK_TASK_REPORT_VEC *crank_task_report_vec);

UINT32 crank_task_report_vec_size(const CRANK_TASK_REPORT_VEC *crank_task_report_vec);

TASK_REPORT_NODE * crank_task_report_vec_fetch(const CRANK_TASK_REPORT_VEC *crank_task_report_vec, const UINT32 crank_task_report_pos);

void crank_task_report_vec_print(LOG *log, const CRANK_TASK_REPORT_VEC *crank_task_report_vec);

UINT32 crank_task_report_vec_get(CRANK_TASK_REPORT_VEC *crank_task_report_vec);

#endif /*_CSYS_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

