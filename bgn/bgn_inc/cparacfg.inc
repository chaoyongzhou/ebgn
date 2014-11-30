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

#ifndef _CPARACFG_INC
#define _CPARACFG_INC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "type.h"


typedef struct
{
    UINT32    tcid; /*this cparacfg for which tcid*/
    UINT32    rank; /*this cparacfg for which rank*/

    UINT32    task_req_thread_max_num       ;
    UINT32    task_rsp_thread_max_num       ;
    UINT32    cthread_stack_max_size        ;
    UINT32    cthread_stack_guard_size      ;
    UINT32    task_req_handle_thread_switch ;
    UINT32    task_req_decode_thread_switch ;
    UINT32    task_rsp_decode_thread_switch ;
    UINT32    task_fwd_decode_thread_switch ;
    UINT32    cbase64_encode_switch         ;
    UINT32    task_encoding_rule            ;
    UINT32    csocket_sosndbuff_size        ;/*for fwd rank only*/
    UINT32    csocket_sorcvbuff_size        ;/*for fwd rank only*/
    UINT32    csocket_crbuff_size           ;/*for fwd rank only*/
    UINT32    csocket_cnode_num             ;/*for fwd rank only*/
    UINT32    csocket_heartbeat_intvl_nsec  ;/*for fwd rank only*/
    UINT32    file_seg_max_size             ;
    UINT32    file_seg_group_size           ;
    UINT32    file_log_max_records          ;
    UINT32    file_log_name_with_date_switch;

    UINT32    log_level_tab[ SEC_NONE_END ] ;
}CPARACFG;

#define CPARACFG_TCID(cparacfg)                                     ((cparacfg)->tcid)
#define CPARACFG_TCID_STR(cparacfg)                                 (c_word_to_ipv4(CPARACFG_TCID(cparacfg)))
#define CPARACFG_RANK(cparacfg)                                     ((cparacfg)->rank)

#define CPARACFG_TASK_REQ_THREAD_MAX_NUM(cparacfg)                  ((cparacfg)->task_req_thread_max_num       )
#define CPARACFG_TASK_RSP_THREAD_MAX_NUM(cparacfg)                  ((cparacfg)->task_rsp_thread_max_num       )
#define CPARACFG_CTHREAD_STACK_MAX_SIZE(cparacfg)                   ((cparacfg)->cthread_stack_max_size        )
#define CPARACFG_CTHREAD_STACK_GUARD_SIZE(cparacfg)                 ((cparacfg)->cthread_stack_guard_size      )
#define CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH(cparacfg)            ((cparacfg)->task_req_handle_thread_switch )
#define CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH(cparacfg)            ((cparacfg)->task_req_decode_thread_switch )
#define CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH(cparacfg)            ((cparacfg)->task_rsp_decode_thread_switch )
#define CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH(cparacfg)            ((cparacfg)->task_fwd_decode_thread_switch )
#define CPARACFG_CBASE64_ENCODE_SWITCH(cparacfg)                    ((cparacfg)->cbase64_encode_switch         )
#define CPARACFG_TASK_ENCODING_RULE(cparacfg)                       ((cparacfg)->task_encoding_rule            )
#define CPARACFG_CSOCKET_SOSNDBUFF_SIZE(cparacfg)                   ((cparacfg)->csocket_sosndbuff_size        )
#define CPARACFG_CSOCKET_SORCVBUFF_SIZE(cparacfg)                   ((cparacfg)->csocket_sorcvbuff_size        )
#define CPARACFG_CSOCKET_CRBUFF_SIZE(cparacfg)                      ((cparacfg)->csocket_crbuff_size           )
#define CPARACFG_CSOCKET_CNODE_NUM(cparacfg)                        ((cparacfg)->csocket_cnode_num             )
#define CPARACFG_CSOCKET_HEARTBEAT_INTVL_NSEC(cparacfg)             ((cparacfg)->csocket_heartbeat_intvl_nsec  )
#define CPARACFG_FILE_SEG_MAX_SIZE(cparacfg)                        ((cparacfg)->file_seg_max_size             )
#define CPARACFG_FILE_SEG_GROUP_SIZE(cparacfg)                      ((cparacfg)->file_seg_group_size           )
#define CPARACFG_FILE_LOG_MAX_RECORDS(cparacfg)                     ((cparacfg)->file_log_max_records          )
#define CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH(cparacfg)           ((cparacfg)->file_log_name_with_date_switch)
#define CPARACFG_LOG_LEVEL_TAB(cparacfg)                            ((cparacfg)->log_level_tab)

#define CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH_STR(cparacfg)        (switch_to_str(CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH(cparacfg)))
#define CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH_STR(cparacfg)        (switch_to_str(CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH(cparacfg)))
#define CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH_STR(cparacfg)        (switch_to_str(CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH(cparacfg)))
#define CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH_STR(cparacfg)        (switch_to_str(CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH(cparacfg)))
#define CPARACFG_CBASE64_ENCODE_SWITCH_STR(cparacfg)                (switch_to_str(CPARACFG_CBASE64_ENCODE_SWITCH(cparacfg)))
#define CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH_STR(cparacfg)       (switch_to_str(CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH(cparacfg)))



#define MEM_BUFF_128B ( 128      )
#define MEM_BUFF_512B ( 512      )
#define MEM_BUFF_1K   ( 1024     )
#define MEM_BUFF_8K   ( 8192    )
#define MEM_BUFF_16K  ( 16384    )
#define MEM_BUFF_32K  ( 32768    )
#define MEM_BUFF_64K  ( 65536    )
#define MEM_BUFF_128K ( 131072   )
#define MEM_BUFF_1M   ( 1048576  )
#define MEM_BUFF_8M   ( 8388608  )
#define MEM_BUFF_16M  ( 16777216 )
#define MEM_BUFF_32M  ( 33554432 )
#define MEM_BUFF_64M  ( 67108864 )

#define CFILE_SEG_256B ( 256     )
#define CFILE_SEG_64K  ( 65536   )
#define CFILE_SEG_1M   ( 1048576 )
#define CFILE_SEG_8M   ( 8388608 )

#define CFILE_SEG_GROUP_02 ( 2  )
#define CFILE_SEG_GROUP_04 ( 4  )
#define CFILE_SEG_GROUP_08 ( 8  )
#define CFILE_SEG_GROUP_16 ( 16 )

#define FILE_LOG_RECORDS_400   ( 400      )
#define FILE_LOG_RECORDS_001K  ( 1000     )
#define FILE_LOG_RECORDS_002K  ( 2000     )
#define FILE_LOG_RECORDS_004K  ( 4000     )
#define FILE_LOG_RECORDS_008K  ( 8000     )
#define FILE_LOG_RECORDS_010K  ( 10000    )
#define FILE_LOG_RECORDS_020K  ( 20000    )
#define FILE_LOG_RECORDS_040K  ( 40000    )
#define FILE_LOG_RECORDS_080K  ( 80000    )
#define FILE_LOG_RECORDS_100K  ( 100000   )
#define FILE_LOG_RECORDS_001M  ( 1000000  )
#define FILE_LOG_RECORDS_002M  ( 2000000  )
#define FILE_LOG_RECORDS_004M  ( 4000000  )
#define FILE_LOG_RECORDS_010M  ( 10000000 )
#define FILE_LOG_RECORDS_020M  ( 20000000 )

#define CTHREAD_STACK_MAX_SIZE_008K ( 8192    )
#define CTHREAD_STACK_MAX_SIZE_016K ( 16384   )
#define CTHREAD_STACK_MAX_SIZE_032K ( 32768   )
#define CTHREAD_STACK_MAX_SIZE_064K ( 65536   )
#define CTHREAD_STACK_MAX_SIZE_128K ( 131072  )
#define CTHREAD_STACK_MAX_SIZE_256K ( 262144  )
#define CTHREAD_STACK_MAX_SIZE_512K ( 524288  )
#define CTHREAD_STACK_MAX_SIZE_001M ( 1048576 )
#define CTHREAD_STACK_MAX_SIZE_002M ( 2097152 )
#define CTHREAD_STACK_MAX_SIZE_004M ( 4194304 )
#define CTHREAD_STACK_MAX_SIZE_008M ( 8388608 )

#define CTHREAD_STACK_GUARD_SIZE_001K ( 1024    )
#define CTHREAD_STACK_GUARD_SIZE_002K ( 2048    )
#define CTHREAD_STACK_GUARD_SIZE_004K ( 4096    )
#define CTHREAD_STACK_GUARD_SIZE_008K ( 8192    )
#define CTHREAD_STACK_GUARD_SIZE_016K ( 16384   )
#define CTHREAD_STACK_GUARD_SIZE_032K ( 32768   )
#define CTHREAD_STACK_GUARD_SIZE_064K ( 65536   )
#define CTHREAD_STACK_GUARD_SIZE_128K ( 131072  )
#define CTHREAD_STACK_GUARD_SIZE_256K ( 262144  )
#define CTHREAD_STACK_GUARD_SIZE_512K ( 524288  )
#define CTHREAD_STACK_GUARD_SIZE_001M ( 1048576 )
#define CTHREAD_STACK_GUARD_SIZE_002M ( 2097152 )
#define CTHREAD_STACK_GUARD_SIZE_004M ( 4194304 )
#define CTHREAD_STACK_GUARD_SIZE_008M ( 8388608 )

#endif/*_CPARACFG_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
