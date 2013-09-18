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

#ifndef _CPARACFG_H
#define _CPARACFG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "type.h"

#include "cparacfg.inc"
#include "cxml.h"
#include "task.h"

#define    CPARACFG_DEFAULT_GET()                        (TASK_BRD_CPARACFG(task_brd_default_get()))

#define    TASK_REQ_THREAD_MAX_NUM                       (CPARACFG_TASK_REQ_THREAD_MAX_NUM(CPARACFG_DEFAULT_GET()))
#define    TASK_RSP_THREAD_MAX_NUM                       (CPARACFG_TASK_RSP_THREAD_MAX_NUM(CPARACFG_DEFAULT_GET()))
//#define    CSOCKET_HEARTBEAT_INTVL_NSEC                  (CPARACFG_CSOCKET_HEARTBEAT_INTVL_NSEC(CPARACFG_DEFAULT_GET()))

//#define    CTHREAD_STACK_MAX_SIZE                      ()
//#define    CTHREAD_STACK_GUARD_SIZE                    ()
//#define    TASK_REQ_HANDLE_THREAD_SWITCH               ()
//#define    TASK_REQ_DECODE_THREAD_SWITCH               ()
//#define    TASK_RSP_DECODE_THREAD_SWITCH               ()
//#define    TASK_FWD_DECODE_THREAD_SWITCH               ()
//#define    CBASE64_ENCODE_SWITCH                       ()
//#define    TASK_ENCODING_RULE                          ()
//#define    CSOCKET_SOSNDBUFF_SIZE                      ()
//#define    CSOCKET_SORCVBUFF_SIZE                      ()
//#define    CSOCKET_CRBUFF_SIZE                         ()
//#define    CSOCKET_CRBUFF_SIZE                         ()
//#define    CSOCKET_CNODE_NUM                           ()
//#define    FILE_SEG_MAX_SIZE                           ()
//#define    FILE_SEG_GROUP_SIZE                         ()
//#define    FILE_LOG_MAX_RECORDS                        ()
//#define    FILE_LOG_NAME_WITH_DATE_SWITCH              ()

CPARACFG *cparacfg_new(const UINT32 this_tcid, const UINT32 this_rank);

EC_BOOL cparacfg_clean(CPARACFG *cparacfg);

EC_BOOL cparacfg_free(CPARACFG *cparacfg);

EC_BOOL cparacfg_init(CPARACFG *cparacfg, const UINT32 this_tcid, const UINT32 this_rank);

EC_BOOL cparacfg_validity_check(const CPARACFG *cparacfg);

EC_BOOL cparacfg_cmp(const CPARACFG *cparacfg_1st, const CPARACFG *cparacfg_2nd);

void cparacfg_print(LOG *log, const CPARACFG *cparacfg);


#endif/*_CPARACFG_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
