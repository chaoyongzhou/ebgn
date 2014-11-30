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
#include <unistd.h>
#include <string.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cmisc.h"
#include "cmpic.inc"

#include "cparacfg.inc"
#include "cparacfg.h"

CPARACFG *cparacfg_new(const UINT32 this_tcid, const UINT32 this_rank)
{
    CPARACFG *cparacfg;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CPARACFG, &cparacfg, LOC_CPARACFG_0001);
    if(NULL_PTR != cparacfg)
    {
        cparacfg_init(cparacfg, this_tcid, this_rank);
    }
    return (cparacfg);
}

EC_BOOL cparacfg_clean(CPARACFG *cparacfg)
{
    return (EC_TRUE);
}

EC_BOOL cparacfg_free(CPARACFG *cparacfg)
{
    if(NULL_PTR != cparacfg)
    {
        cparacfg_clean(cparacfg);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CPARACFG, cparacfg, LOC_CPARACFG_0002);
    }
    return (EC_TRUE);
}

EC_BOOL cparacfg_init(CPARACFG *cparacfg, const UINT32 this_tcid, const UINT32 this_rank)
{   
    CPARACFG_TCID(cparacfg) = this_tcid;
    CPARACFG_RANK(cparacfg) = this_rank;

    CPARACFG_TASK_REQ_THREAD_MAX_NUM(cparacfg)                  = 16;
    CPARACFG_TASK_RSP_THREAD_MAX_NUM(cparacfg)                  = 5;
    CPARACFG_CTHREAD_STACK_MAX_SIZE(cparacfg)                   = CTHREAD_STACK_MAX_SIZE_064K;
    CPARACFG_CTHREAD_STACK_GUARD_SIZE(cparacfg)                 = CTHREAD_STACK_GUARD_SIZE_004K;
    CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH(cparacfg)            = SWITCH_ON;
    CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH(cparacfg)            = SWITCH_OFF;
    CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH(cparacfg)            = SWITCH_ON;
    CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH(cparacfg)            = SWITCH_ON;
    CPARACFG_CBASE64_ENCODE_SWITCH(cparacfg)                    = SWITCH_OFF;
    CPARACFG_TASK_ENCODING_RULE(cparacfg)                       = BYTE_ENCODING_RULE;
    CPARACFG_CSOCKET_SOSNDBUFF_SIZE(cparacfg)                   = MEM_BUFF_32K;
    CPARACFG_CSOCKET_SORCVBUFF_SIZE(cparacfg)                   = MEM_BUFF_32K;
    CPARACFG_CSOCKET_CRBUFF_SIZE(cparacfg)                      = MEM_BUFF_64M;
    CPARACFG_CSOCKET_CNODE_NUM(cparacfg)                        = 1;
    CPARACFG_CSOCKET_HEARTBEAT_INTVL_NSEC(cparacfg)             = 10;
    CPARACFG_FILE_SEG_MAX_SIZE(cparacfg)                        = CFILE_SEG_1M;
    CPARACFG_FILE_SEG_GROUP_SIZE(cparacfg)                      = CFILE_SEG_GROUP_04;
    CPARACFG_FILE_LOG_MAX_RECORDS(cparacfg)                     = FILE_LOG_RECORDS_001M;
    CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH(cparacfg)           = SWITCH_OFF;

    log_level_tab_init(CPARACFG_LOG_LEVEL_TAB(cparacfg), SEC_NONE_END, LOG_DEFAULT_DBG_LEVEL);    

    return (EC_TRUE);
}

EC_BOOL cparacfg_clone(const CPARACFG *cparacfg_src, CPARACFG *cparacfg_des)
{
    BCOPY(cparacfg_src, cparacfg_des, sizeof(CPARACFG));
    return (EC_TRUE);
}

EC_BOOL cparacfg_validity_check(const CPARACFG *cparacfg)
{
    EC_BOOL ret;

    ret = EC_TRUE;

    if(2 > CPARACFG_TASK_REQ_THREAD_MAX_NUM(cparacfg))
    {
        dbg_log(SEC_0052_CPARACFG, 0)(LOGSTDOUT, "error:cparacfg_check: TASK_REQ_THREAD_MAX_NUM is less than 2\n");
        ret = EC_FALSE;
    }

    return (ret);
}

EC_BOOL cparacfg_cmp(const CPARACFG *cparacfg_1st, const CPARACFG *cparacfg_2nd)
{
    if(CPARACFG_TCID(cparacfg_1st) != CPARACFG_TCID(cparacfg_2nd) )
    {
        return (EC_FALSE);
    }

    if(CPARACFG_RANK(cparacfg_1st) != CPARACFG_RANK(cparacfg_2nd) )
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

void cparacfg_print(LOG *log, const CPARACFG *cparacfg)
{
    sys_log(log, "tcid = %s, rank = %ld\n",  CPARACFG_TCID_STR(cparacfg), CPARACFG_RANK(cparacfg));

    sys_log(log, "TASK_REQ_THREAD_MAX_NUM        = %ld\n",  CPARACFG_TASK_REQ_THREAD_MAX_NUM(cparacfg)       );
    sys_log(log, "TASK_RSP_THREAD_MAX_NUM        = %ld\n",  CPARACFG_TASK_RSP_THREAD_MAX_NUM(cparacfg)       );
    sys_log(log, "CTHREAD_STACK_MAX_SIZE         = %ld\n",  CPARACFG_CTHREAD_STACK_MAX_SIZE(cparacfg)        );
    sys_log(log, "CTHREAD_STACK_GUARD_SIZE       = %ld\n",  CPARACFG_CTHREAD_STACK_GUARD_SIZE(cparacfg)      );
    sys_log(log, "TASK_REQ_HANDLE_THREAD_SWITCH  = %s\n" ,  CPARACFG_TASK_REQ_HANDLE_THREAD_SWITCH_STR(cparacfg) );
    sys_log(log, "TASK_REQ_DECODE_THREAD_SWITCH  = %s\n" ,  CPARACFG_TASK_REQ_DECODE_THREAD_SWITCH_STR(cparacfg) );
    sys_log(log, "TASK_RSP_DECODE_THREAD_SWITCH  = %s\n" ,  CPARACFG_TASK_RSP_DECODE_THREAD_SWITCH_STR(cparacfg) );
    sys_log(log, "TASK_FWD_DECODE_THREAD_SWITCH  = %s\n" ,  CPARACFG_TASK_FWD_DECODE_THREAD_SWITCH_STR(cparacfg) );
    sys_log(log, "CBASE64_ENCODE_SWITCH          = %s\n" ,  CPARACFG_CBASE64_ENCODE_SWITCH_STR(cparacfg)         );
    sys_log(log, "TASK_ENCODING_RULE             = %ld\n",  CPARACFG_TASK_ENCODING_RULE(cparacfg)            );
    sys_log(log, "CSOCKET_SOSNDBUFF_SIZE         = %ld\n",  CPARACFG_CSOCKET_SOSNDBUFF_SIZE(cparacfg)        );
    sys_log(log, "CSOCKET_SORCVBUFF_SIZE         = %ld\n",  CPARACFG_CSOCKET_SORCVBUFF_SIZE(cparacfg)        );
    sys_log(log, "CSOCKET_CRBUFF_SIZE            = %ld\n",  CPARACFG_CSOCKET_CRBUFF_SIZE(cparacfg)           );
    sys_log(log, "CSOCKET_CNODE_NUM              = %ld\n",  CPARACFG_CSOCKET_CNODE_NUM(cparacfg)             );
    sys_log(log, "CSOCKET_HEARTBEAT_INTVL_NSEC   = %ld\n",  CPARACFG_CSOCKET_HEARTBEAT_INTVL_NSEC(cparacfg)  );
    sys_log(log, "FILE_SEG_MAX_SIZE              = %ld\n",  CPARACFG_FILE_SEG_MAX_SIZE(cparacfg)             );
    sys_log(log, "FILE_SEG_GROUP_SIZE            = %ld\n",  CPARACFG_FILE_SEG_GROUP_SIZE(cparacfg)           );
    sys_log(log, "FILE_LOG_MAX_RECORDS           = %ld\n",  CPARACFG_FILE_LOG_MAX_RECORDS(cparacfg)          );
    sys_log(log, "FILE_LOG_NAME_WITH_DATE_SWITCH = %s\n" ,  CPARACFG_FILE_LOG_NAME_WITH_DATE_SWITCH_STR(cparacfg));

    return;
}



#ifdef __cplusplus
}
#endif/*__cplusplus*/
