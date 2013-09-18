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

#ifndef _LIB_LOG_H
#define _LIB_LOG_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"
#include "lib_cstring.h"

#define DEFAULT_STDOUT_LOG_INDEX    0
#define DEFAULT_STDIN_LOG_INDEX     1
#define DEFAULT_STDERR_LOG_INDEX    2
#define DEFAULT_STDNULL_LOG_INDEX   3
#define DEFAULT_CONSOLE_LOG_INDEX   4
#define DEFAULT_END_LOG_INDEX       5


extern LOG g_default_log_tbl[];
#define LOGSTDOUT  (&g_default_log_tbl[ DEFAULT_STDOUT_LOG_INDEX  ])
#define LOGSTDIN   (&g_default_log_tbl[ DEFAULT_STDIN_LOG_INDEX   ])
#define LOGSTDERR  (&g_default_log_tbl[ DEFAULT_STDERR_LOG_INDEX  ])
#define LOGSTDNULL (&g_default_log_tbl[ DEFAULT_STDNULL_LOG_INDEX ])
#define LOGCONSOLE (&g_default_log_tbl[ DEFAULT_CONSOLE_LOG_INDEX ])

//#define LOGSTDOUT  (log_get_by_fd( DEFAULT_STDOUT_LOG_INDEX  ))
//#define LOGSTDIN   (log_get_by_fd( DEFAULT_STDIN_LOG_INDEX   ))
//#define LOGSTDERR  (log_get_by_fd( DEFAULT_STDERR_LOG_INDEX  ))
//#define LOGSTDNULL (log_get_by_fd( DEFAULT_STDNULL_LOG_INDEX ))

EC_BOOL log_start();

void log_end();

int sys_log(LOG *log, const char * format, ...);

int sys_print(LOG *log, const char * format, ...);

int sys_log_switch_on();

int sys_log_switch_off();

int sys_log_redirect_setup(LOG *old_log, LOG *new_log);

LOG * sys_log_redirect_cancel(LOG *log);

LOG * log_file_open(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable);

EC_BOOL log_file_close(LOG *log);

LOG * log_cstr_open();

EC_BOOL log_cstr_close(LOG *log);


#endif/* _LIB_LOG_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
