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
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include "type.h"
#include "bgnctrl.h"
#include "log.h"

#include "mm.h"

#include "cmisc.h"

#include "cmutex.h"
#include "cstring.h"

#include "task.h"

LOG g_default_log_tbl[DEFAULT_END_LOG_INDEX] = {
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDOUT*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDIN*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDERR*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGSTDNULL*/
    {LOG_FILE_DEVICE, LOGD_SWITCH_OFF_ENABLE , LOGD_PID_INFO_ENABLE ,NULL_PTR, {SWITCH_OFF, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, LOGD_FILE_RECORD_LIMIT_DISABLED, LOGD_FILE_MAX_RECORDS_LIMIT, 0}},/*LOGCONSOLE*/
};

static int g_log_switch = SWITCH_ON;

EC_BOOL log_start()
{
    LOG_FILE_FP(LOGSTDOUT)  = stdout;
    LOG_FILE_FP(LOGSTDIN)   = stdin;
    LOG_FILE_FP(LOGSTDERR)  = stderr;
    LOG_FILE_FP(LOGSTDNULL) = stdnull;/*invalid value*/
    LOG_FILE_FP(LOGCONSOLE) = stdout;

    LOG_REDIRECT(LOGSTDOUT)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDIN)   = NULL_PTR;
    LOG_REDIRECT(LOGSTDERR)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDNULL) = NULL_PTR;
    LOG_REDIRECT(LOGCONSOLE) = NULL_PTR;

    return (EC_TRUE);
}

void log_end()
{
    LOG_FILE_FP(LOGSTDOUT)  = NULL_PTR;
    LOG_FILE_FP(LOGSTDIN)   = NULL_PTR;
    LOG_FILE_FP(LOGSTDERR)  = NULL_PTR;
    LOG_FILE_FP(LOGSTDNULL) = NULL_PTR;
    LOG_FILE_FP(LOGCONSOLE) = NULL_PTR;

    LOG_REDIRECT(LOGSTDOUT)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDIN)   = NULL_PTR;
    LOG_REDIRECT(LOGSTDERR)  = NULL_PTR;
    LOG_REDIRECT(LOGSTDNULL) = NULL_PTR;
    LOG_REDIRECT(LOGCONSOLE) = NULL_PTR;

    return;
}

LOG *log_get_by_fp(FILE *fp)
{
    UINT32 idx;
    LOG *log;

    for(idx = 0; idx < sizeof(g_default_log_tbl)/sizeof(g_default_log_tbl[0]); idx ++)
    {
        log = &(g_default_log_tbl[ idx ]);
        if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log) && fp == LOG_FILE_FP(log))
        {
            return log;
        }
    }
    return &(g_default_log_tbl[ idx - 1 ]);
}

LOG *log_get_by_fd(const UINT32 fd)
{
    if( /*0 <= fd && */fd < sizeof(g_default_log_tbl)/sizeof(g_default_log_tbl[0]))
    {
        return &(g_default_log_tbl[ fd ]);
    }
    return LOGSTDNULL;
}

static int sys_log_to_buf(FILE *fp, const char * format,va_list ap, char *buf, const int buf_max_size)
{
    CTM *cur_time;
    int len;

    cur_time = LOG_TM();

    len = 0;
    len += snprintf(buf, buf_max_size, "[%4d-%02d-%02d %02d:%02d:%02d] ",
                    cur_time->tm_year + 1900,
                    cur_time->tm_mon + 1,
                    cur_time->tm_mday,
                    cur_time->tm_hour,
                    cur_time->tm_min,
                    cur_time->tm_sec);

    len += vsnprintf((char *)(buf + len), buf_max_size - len, format, ap);

    return (len);
}

static int sys_log_to_fd_orig(FILE *fp, const char * format,va_list ap)
{
    CTM *cur_time;
    int ret;

    cur_time = LOG_TM();

    fprintf(fp, "[%4d-%02d-%02d %02d:%02d:%02d] ",
            cur_time->tm_year + 1900,
            cur_time->tm_mon + 1,
            cur_time->tm_mday,
            cur_time->tm_hour,
            cur_time->tm_min,
            cur_time->tm_sec);
    fflush(fp);

    ret = vfprintf(fp, format, ap);
    fflush( fp );

    return (ret);
}

static int sys_log_to_fd(FILE *fp, const char * format,va_list ap)
{
    int   len;
    char *buf;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, &buf, LOC_LOG_0001);
    if(NULL_PTR == buf)
    {
        return (0);
    }

    len = sys_log_to_buf(fp, format, ap, buf, 64 * 1024);
    fprintf(fp, "%.*s", len, buf);
    fflush(fp);

    free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_UINT8_064K, buf, LOC_LOG_0002);

    return (len);
}

static int sys_log_to_cstring(CSTRING *cstring, const char * format, va_list ap)
{
    CTM *cur_time;

    cur_time = LOG_TM();

    cstring_format(cstring, "[%4d-%02d-%02d %02d:%02d:%02d] ",
            cur_time->tm_year + 1900,
            cur_time->tm_mon + 1,
            cur_time->tm_mday,
            cur_time->tm_hour,
            cur_time->tm_min,
            cur_time->tm_sec);

    cstring_vformat(cstring, format, ap);
    return (0);
}

static int sys_print_to_fd(FILE *fp, const char * format,va_list ap)
{
    int ret;

    ret = vfprintf(fp, format, ap);
    fflush( fp );

    return (ret);
}

static int sys_print_to_cstring(CSTRING *cstring, const char * format, va_list ap)
{
    cstring_vformat(cstring, format, ap);
    return (0);
}

int sys_log_no_lock(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        va_start(ap, format);
        ret = sys_log_to_fd(LOG_FILE_FP(des_log), format, ap);
        va_end(ap);

        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }
            }

            ++ LOG_FILE_CUR_RECORDS(des_log);
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_log_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);

        return (ret);
    }

    return (-1);
}

int sys_log(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        LOG_FILE_LOCK(des_log, LOC_LOG_0003);

        va_start(ap, format);
        ret = sys_log_to_fd(LOG_FILE_FP(des_log), format, ap);
        va_end(ap);

        LOG_FILE_UNLOCK(des_log, LOC_LOG_0004);

        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                LOG_FILE_LOCK(des_log, LOC_LOG_0005);

                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }

                LOG_FILE_UNLOCK(des_log, LOC_LOG_0006);
            }

            ++ LOG_FILE_CUR_RECORDS(des_log);
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_log_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);

        return (ret);
    }

    return (-1);
}

int sys_print_no_lock(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        va_start(ap, format);
        ret = sys_print_to_fd(LOG_FILE_FP(des_log), format, ap);
        va_end(ap);

        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }
            }

            ++ LOG_FILE_CUR_RECORDS(des_log);
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_print_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);
        return (ret);
    }

    return (-1);
}

int sys_print(LOG *log, const char * format, ...)
{
    LOG *des_log;
    va_list ap;

    //return (0);/*debug*/

    if( SWITCH_OFF == g_log_switch && NULL_PTR != log && LOGD_SWITCH_OFF_ENABLE == LOG_SWITCH_OFF_ENABLE(log) )
    {
        return (0);
    }

    des_log = log;
    //if(LOGSTDNULL == log) des_log = LOGSTDOUT; /*debug!*/
    while(NULL_PTR != des_log && NULL_PTR != des_log->redirect_log)
    {
        des_log = des_log->redirect_log;
    }

    if(LOGSTDNULL == des_log)
    {
        return (0);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_FILE_FP(des_log))
    {
        int ret;

        LOG_FILE_LOCK(des_log, LOC_LOG_0007);

        va_start(ap, format);
        ret = sys_print_to_fd(LOG_FILE_FP(des_log), format, ap);
        va_end(ap);

        LOG_FILE_UNLOCK(des_log, LOC_LOG_0008);

        if(LOGD_FILE_RECORD_LIMIT_ENABLED == LOG_FILE_LIMIT_ENABLED(des_log))
        {
            //WARNING: here need a lock!
            if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))
            {
                LOG_FILE_LOCK(des_log, LOC_LOG_0009);

                if(LOG_FILE_CUR_RECORDS(des_log) > LOG_FILE_RECORDS_LIMIT(des_log))/*double confirm!*/
                {
                    log_file_freopen(des_log);
                    LOG_FILE_CUR_RECORDS(des_log) = 0;
                }

                LOG_FILE_UNLOCK(des_log, LOC_LOG_0010);
            }

            ++ LOG_FILE_CUR_RECORDS(des_log);
        }

        return (ret);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(des_log) && NULL_PTR != LOG_CSTR(des_log))
    {
        int ret;
        va_start(ap, format);
        ret = sys_print_to_cstring(LOG_CSTR(des_log), format, ap);
        va_end(ap);
        return (ret);
    }

    return (-1);
}

int sys_log_switch_on()
{
    g_log_switch = SWITCH_ON;
    return (0);
}

int sys_log_switch_off()
{
    g_log_switch = SWITCH_OFF;
    return (0);
}

int sys_log_redirect_setup(LOG *old_log, LOG *new_log)
{
    LOG_REDIRECT(old_log) = new_log;
    return (0);
}

LOG * sys_log_redirect_cancel(LOG *log)
{
    LOG *old_log;

    old_log = LOG_REDIRECT(log);
    LOG_REDIRECT(log) = NULL_PTR;

    return (old_log);
}

LOG *log_file_new(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    LOG *log;

    alloc_static_mem(MD_TASK, 0, MM_LOG, &log, LOC_LOG_0011);
    if(EC_FALSE == log_file_init(log, fname, mode, tcid, rank, record_limit_enabled, fname_with_date_switch, switch_off_enable, pid_info_enable))
    {
        sys_log(LOGSTDOUT, "error:log_file_new: log file %s init failed\n", fname);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0012);
        return (NULL_PTR);
    }
    return (log);
}

EC_BOOL log_file_init(LOG *log, const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    if(NULL_PTR == fname)
    {
        LOG_DEVICE_TYPE(log)         = LOG_FILE_DEVICE;
        LOG_SWITCH_OFF_ENABLE(log)   = switch_off_enable;
        LOG_PID_INFO_ENABLE(log)     = pid_info_enable;
        LOG_REDIRECT(log)            = NULL_PTR;

        LOG_FILE_NAME_WITH_DATE_SWITCH(log) = fname_with_date_switch;

        LOG_FILE_NAME(log)           = NULL_PTR;
        LOG_FILE_MODE(log)           = NULL_PTR;
        LOG_FILE_FP(log)             = NULL_PTR;
        LOG_FILE_CMUTEX(log)         = NULL_PTR;
        LOG_FILE_LIMIT_ENABLED(log)  = record_limit_enabled;
        LOG_FILE_RECORDS_LIMIT(log)  = LOGD_FILE_MAX_RECORDS_LIMIT;
        LOG_FILE_CUR_RECORDS(log)    = 0;

        LOG_FILE_TCID(log)           = tcid;
        LOG_FILE_RANK(log)           = rank;
        return (EC_TRUE);
    }

    LOG_DEVICE_TYPE(log)       = LOG_FILE_DEVICE;
    LOG_SWITCH_OFF_ENABLE(log) = switch_off_enable;
    LOG_PID_INFO_ENABLE(log)   = pid_info_enable;
    LOG_REDIRECT(log)          = NULL_PTR;

    LOG_FILE_TCID(log) = tcid;
    LOG_FILE_RANK(log) = rank;

    LOG_FILE_NAME_WITH_DATE_SWITCH(log) = fname_with_date_switch;

    LOG_FILE_NAME(log) = cstring_new((UINT8 *)fname, LOC_LOG_0013);
    LOG_FILE_MODE(log) = cstring_new((UINT8 *)mode, LOC_LOG_0014);

    LOG_FILE_CMUTEX(log) = c_mutex_new(CMUTEX_PROCESS_PRIVATE, LOC_LOG_0015);
    if(NULL_PTR == LOG_FILE_CMUTEX(log))
    {
        fprintf(stderr,"error:log_file_init: failed to new cmutex for %s\n", (char *)LOG_FILE_NAME_STR(log));
        fflush(stderr);

        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;

        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;

        LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;

        return (EC_FALSE);
    }

    if(EC_FALSE == log_file_fopen(log))
    {
        fprintf(stderr,"error:log_file_init: failed to open %s to write\n", (char *)LOG_FILE_NAME_STR(log));
        fflush(stderr);

        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;

        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;

        LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;

        c_mutex_free(LOG_FILE_CMUTEX(log), LOC_LOG_0016);
        LOG_FILE_CMUTEX(log) = NULL_PTR;

        return (EC_FALSE);
    }

    LOG_FILE_LIMIT_ENABLED(log) = record_limit_enabled;
    if(LOGD_FILE_RECORD_LIMIT_ENABLED == record_limit_enabled)
    {
        LOG_FILE_RECORDS_LIMIT(log) = FILE_LOG_MAX_RECORDS;
        LOG_FILE_CUR_RECORDS(log)   = 0;
    }
    else
    {
        LOG_FILE_RECORDS_LIMIT(log) = LOGD_FILE_MAX_RECORDS_LIMIT;
        LOG_FILE_CUR_RECORDS(log)   = 0;
    }

    return (EC_TRUE);
}

EC_BOOL log_file_clean(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR != LOG_FILE_NAME(log))
    {
        cstring_free(LOG_FILE_NAME(log));
        LOG_FILE_NAME(log) = NULL_PTR;
    }

    if(NULL_PTR != LOG_FILE_MODE(log))
    {
        cstring_free(LOG_FILE_MODE(log));
        LOG_FILE_MODE(log) = NULL_PTR;
    }

    log_file_fclose(log);
    if(NULL_PTR != LOG_FILE_CMUTEX(log))
    {
        c_mutex_free(LOG_FILE_CMUTEX(log), LOC_LOG_0017);
        LOG_FILE_CMUTEX(log) = NULL_PTR;
    }

    LOG_FILE_LIMIT_ENABLED(log) = LOGD_FILE_RECORD_LIMIT_DISABLED;
    LOG_FILE_RECORDS_LIMIT(log) = LOGD_FILE_MAX_RECORDS_LIMIT;
    LOG_FILE_CUR_RECORDS(log)   = 0;

    LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;

    return (EC_TRUE);
}

EC_BOOL log_file_fopen(LOG *log)
{
    char fname[128];

    if(SWITCH_ON == LOG_FILE_NAME_WITH_DATE_SWITCH(log))
    {
        CTM *cur_time;
        cur_time = LOG_TM();

        snprintf(fname, sizeof(fname) - 1, "%s_%4d%02d%02d_%02d%02d%02d.log",
                (char *)LOG_FILE_NAME_STR(log),
                cur_time->tm_year + 1900,
                cur_time->tm_mon + 1,
                cur_time->tm_mday,
                cur_time->tm_hour,
                cur_time->tm_min,
                cur_time->tm_sec);
    }
    else
    {
        snprintf(fname, sizeof(fname) - 1, "%s.log", (char *)LOG_FILE_NAME_STR(log));
    }

    if(EC_FALSE == c_basedir_create(fname))
    {
        return (EC_FALSE);
    }

    LOG_FILE_FP(log) = fopen(fname, (char *)LOG_FILE_MODE_STR(log));
    if(NULL_PTR == LOG_FILE_FP(log))
    {
        return (EC_FALSE);
    }

#if 1
    if(LOGD_PID_INFO_ENABLE == LOG_PID_INFO_ENABLE(log))
    {
        CTM *cur_time;
        cur_time = LOG_TM();

        fprintf(LOG_FILE_FP(log), "[%4d-%02d-%02d %02d:%02d:%02d] ",
                cur_time->tm_year + 1900,
                cur_time->tm_mon + 1,
                cur_time->tm_mday,
                cur_time->tm_hour,
                cur_time->tm_min,
                cur_time->tm_sec);
        fprintf(LOG_FILE_FP(log), "my pid = %u, tcid = %s, rank = %ld\n",
                                  getpid(), c_word_to_ipv4(LOG_FILE_TCID(log)), LOG_FILE_RANK(log));
        fflush(LOG_FILE_FP(log));
    }
#endif
    return (EC_TRUE);
}

EC_BOOL log_file_fclose(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(NULL_PTR == LOG_FILE_FP(log))
    {
        return (EC_TRUE);
    }

    if(stdout != LOG_FILE_FP(log) && stderr != LOG_FILE_FP(log) && stdin != LOG_FILE_FP(log))
    {
        fclose(LOG_FILE_FP(log));
        LOG_FILE_FP(log) = NULL_PTR;
    }

    return (EC_TRUE);
}

EC_BOOL log_file_freopen(LOG *log)
{
    log_file_fclose(log);/*close old*/

    if(EC_FALSE == log_file_fopen(log)) /*open new*/
    {
        sys_log(LOGSTDERR, "error:log_file_freopen: failed to reopen file %s with mode %s\n",
                           (char *)LOG_FILE_NAME_STR(log),
                           (char *)LOG_FILE_MODE_STR(log));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void log_file_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_file_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0018);
    }
    return;
}

LOG * log_file_open(const char *fname, const char *mode, const UINT32 tcid, const UINT32 rank, const UINT32 record_limit_enabled, const UINT32 fname_with_date_switch, const UINT32 switch_off_enable, const UINT32 pid_info_enable)
{
    return log_file_new(fname, mode, tcid, rank, record_limit_enabled, fname_with_date_switch, switch_off_enable, pid_info_enable);
}

EC_BOOL log_file_close(LOG *log)
{
    return log_free(log);
}

LOG *log_cstr_new()
{
    LOG *log;

    alloc_static_mem(MD_TASK, 0, MM_LOG, &log, LOC_LOG_0019);
    LOG_CSTR(log) = NULL_PTR;
    log_cstr_init(log);

    return log;
}

EC_BOOL log_cstr_init(LOG *log)
{
    LOG_DEVICE_TYPE(log) = LOG_CSTR_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;

    if(NULL_PTR == LOG_CSTR(log))
    {
        LOG_CSTR(log) = cstring_new(NULL_PTR, LOC_LOG_0020);
    }

    return (EC_TRUE);
}

EC_BOOL log_cstr_clean(LOG *log)
{
    cstring_free(LOG_CSTR(log));
    LOG_CSTR(log) = NULL_PTR;

    LOG_DEVICE_TYPE(log) = LOG_NULL_DEVICE;
    LOG_REDIRECT(log)    = NULL_PTR;
    return (EC_TRUE);
}

void log_cstr_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_cstr_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0021);
    }
    return;
}

LOG * log_cstr_open()
{
    return log_cstr_new();
}

EC_BOOL log_cstr_close(LOG *log)
{
    return log_free(log);
}


EC_BOOL log_clean(LOG *log)
{
    if(LOGSTDOUT == log || LOGSTDIN == log || LOGSTDERR == log)
    {
        return (EC_TRUE);
    }

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log))
    {
        return log_file_clean(log);
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(log))
    {
        return log_cstr_clean(log);
    }

    LOG_SWITCH_OFF_ENABLE(log) = LOGD_SWITCH_OFF_DISABLE;
    LOG_PID_INFO_ENABLE(log)   = LOGD_PID_INFO_DISABLE;

    return (EC_TRUE);
}

EC_BOOL log_free(LOG *log)
{
    if(NULL_PTR != log)
    {
        log_clean(log);
        free_static_mem(MD_TASK, 0, MM_LOG, log, LOC_LOG_0022);
    }
    return (EC_TRUE);
}

UINT32 log_init_0(const UINT32 md_id, LOG *log)
{
    LOG_DEVICE_TYPE(log)       = LOG_NULL_DEVICE;
    LOG_SWITCH_OFF_ENABLE(log) = LOGD_SWITCH_OFF_DISABLE;
    LOG_PID_INFO_ENABLE(log)   = LOGD_PID_INFO_DISABLE;
    LOG_REDIRECT(log)          = NULL_PTR;
    LOG_CSTR(log)              = NULL_PTR;

    return (0);
}
UINT32 log_clean_0(const UINT32 md_id, LOG *log)
{
    log_clean(log);
    return (0);
}

UINT32 log_free_0(const UINT32 md_id, LOG *log)
{
    log_free(log);
    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
