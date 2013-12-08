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

#ifndef _GANGLIA_H
#define _GANGLIA_H

#include "type.h"
#include "mm.h"
#include "log.h"
#include "mod.inc"

#include "clist.h"
#include "cbytes.h"

#include "cmutex.h"

#include "gm_protocol.h"

typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;

    UINT32      mcast_ipaddr;
    UINT32      srv_port;

    int         udp_sender_sockfd;
    int         rsvd;

    CLIST       gmetric_list;

    UINT32        cmon_md_id;
    CMON_OBJ_VEC *cmon_obj_cpu_vec;
    CMON_OBJ_VEC *cmon_obj_dsk_vec;
    CMON_OBJ_VEC *cmon_obj_eth_vec;
    CMON_OBJ_VEC *cmon_obj_smm_vec;/*system memory*/
    CMON_OBJ_VEC *cmon_obj_pro_vec;/*process memory*/
    CMON_OBJ_VEC *cmon_obj_thr_vec;/*thread memory*/
}GANGLIA_MD;

#define GANGLIA_MD_MCAST_IPADDR(ganglia_md)         ((ganglia_md)->mcast_ipaddr)
#define GANGLIA_MD_SRV_PORT(ganglia_md)             ((ganglia_md)->srv_port)
#define GANGLIA_MD_UDP_SENDER(ganglia_md)           ((ganglia_md)->udp_sender_sockfd)
#define GANGLIA_MD_METRIC_LIST(ganglia_md)          (&((ganglia_md)->gmetric_list))
#define GANGLIA_MD_CMON_MD_ID(ganglia_md)           ((ganglia_md)->cmon_md_id)
#define GANGLIA_MD_OBJ_CPU_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_cpu_vec)
#define GANGLIA_MD_OBJ_DSK_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_dsk_vec)
#define GANGLIA_MD_OBJ_ETH_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_eth_vec)
#define GANGLIA_MD_OBJ_SMM_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_smm_vec)
#define GANGLIA_MD_OBJ_PRO_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_pro_vec)
#define GANGLIA_MD_OBJ_THR_VEC(ganglia_md)          ((ganglia_md)->cmon_obj_thr_vec)

#define GANGLIA_SLOPE_ZERO          ((u_int) 0)
#define GANGLIA_SLOPE_POSITIVE      ((u_int) 1)
#define GANGLIA_SLOPE_NEGATIVE      ((u_int) 2)
#define GANGLIA_SLOPE_BOTH          ((u_int) 3)
#define GANGLIA_SLOPE_UNSPECIFIED   ((u_int) 4)
#define GANGLIA_SLOPE_DERIVATIVE    ((u_int) 5)

typedef u_int ganglia_slope_t;


#define GANGLIA_VALUE_MSG_USHORT(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gu_short))
#define GANGLIA_VALUE_MSG_SHORT(vmsg)       (&((vmsg)->Ganglia_value_msg_u.gs_short))
#define GANGLIA_VALUE_MSG_INT(vmsg)         (&((vmsg)->Ganglia_value_msg_u.gs_int))
#define GANGLIA_VALUE_MSG_UINT(vmsg)        (&((vmsg)->Ganglia_value_msg_u.gu_int))
#define GANGLIA_VALUE_MSG_STRING(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gstr))
#define GANGLIA_VALUE_MSG_FLOAT(vmsg)       (&((vmsg)->Ganglia_value_msg_u.gf))
#define GANGLIA_VALUE_MSG_DOUBLE(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gd))

typedef struct
{
    Ganglia_metadata_msg    fmsg;/*metadata (format) msg*/
    Ganglia_value_msg       vmsg;/*value msg*/
}GANGLIA;
#define GANGLIA_FORMAT_MSG(ganglia)         (&((ganglia)->fmsg))
#define GANGLIA_VALUE_MSG(ganglia)          (&((ganglia)->vmsg))

#define GANGLIA_VALUE_MSG_USHORT_VAL(vmsg)   ((vmsg)->Ganglia_value_msg_u.gu_short.us)


/**
*   for test only
*
*   to query the status of GANGLIA Module
*
**/
void ganglia_print_module_status(const UINT32 ganglia_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed GANGLIA module
*
*
**/
UINT32 ganglia_free_module_static_mem(const UINT32 ganglia_md_id);

/**
*
* start GANGLIA module
*
**/
UINT32 ganglia_start(const UINT32 mcast_ipaddr, const UINT32 srv_port);

/**
*
* end GANGLIA module
*
**/
void ganglia_end(const UINT32 ganglia_md_id);

GANGLIA *ganglia_new(const UINT32 ganglia_md_id);

EC_BOOL ganglia_init(const UINT32 ganglia_md_id, GANGLIA *ganglia);

EC_BOOL ganglia_clean(const UINT32 ganglia_md_id, GANGLIA *ganglia);

EC_BOOL ganglia_free(const UINT32 ganglia_md_id, GANGLIA *ganglia);

EC_BOOL ganglia_clean_0(GANGLIA *ganglia);

EC_BOOL ganglia_free_0(GANGLIA *ganglia);

EC_BOOL ganglia_send(const UINT32 ganglia_md_id, const GANGLIA *ganglia);

void ganglia_print(LOG *log, const GANGLIA *ganglia);

void ganglia_metadata_print(LOG *log, const Ganglia_metadata *metadata);

void ganglia_metric_id_print(LOG *log, const Ganglia_metric_id *metric_id);

void ganglia_metadata_message_print(LOG *log, const Ganglia_metadata_message *metadata_message);

void ganglia_metadatadef_print(LOG *log, const Ganglia_metadatadef *metadatadef);

void ganglia_metadata_msg_print(LOG *log, const Ganglia_metadata_msg *metadata_msg);

void ganglia_value_msg_print(LOG *log, const Ganglia_value_msg *value_msg);

EC_BOOL ganglia_metric_id_set(const UINT32 ganglia_md_id, Ganglia_metric_id *metric_id, const char *host, const char *metric_name);

EC_BOOL ganglia_metadata_message_set(const UINT32 ganglia_md_id, Ganglia_metadata_message *metadata_message,
                                                const char *group_name,
                                                const char *val_type, const char *val_name, const char *val_units,
                                                const u_int slope, const u_int tmax, const u_int dmax);

EC_BOOL ganglia_metadata_msg_set(const UINT32 ganglia_md_id, Ganglia_metadata_msg *metadata_msg,
                                       const char *host, const char *metric_name, const char *group_name,
                                       const char *val_type, const char *val_name, const char *val_units,
                                       const u_int slope, const u_int tmax, const u_int dmax);

EC_BOOL ganglia_metadata_msg_clean(const UINT32 ganglia_md_id, Ganglia_metadata_msg *metadata_msg);

EC_BOOL ganglia_metadata_msg_send(const UINT32 ganglia_md_id, const Ganglia_metadata_msg *metadata_msg);

EC_BOOL ganglia_value_msg_set_ushort(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const u_short val);

EC_BOOL ganglia_value_msg_set_short(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const short val);

EC_BOOL ganglia_value_msg_set_int(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const int val);

EC_BOOL ganglia_value_msg_set_uint(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const u_int val);

EC_BOOL ganglia_value_msg_set_string(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const char *val);

EC_BOOL ganglia_value_msg_set_float(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const float val);

EC_BOOL ganglia_value_msg_set_double(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const double val);

EC_BOOL ganglia_value_msg_clean(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg);

EC_BOOL ganglia_value_msg_send(const UINT32 ganglia_md_id, const Ganglia_value_msg *value_msg);


EC_BOOL ganglia_report_cpus_load(const UINT32 ganglia_md_id);

EC_BOOL ganglia_report_sys_mem_load(const UINT32 ganglia_md_id);

EC_BOOL ganglia_report_proc_mem_load(const UINT32 ganglia_md_id);

EC_BOOL ganglia_report_crank_thread_stat(const UINT32 ganglia_md_id);

EC_BOOL ganglia_report_dsks_stat(const UINT32 ganglia_md_id);

EC_BOOL ganglia_report_eths_stat(const UINT32 ganglia_md_id);

EC_BOOL ganglia_register_to_task_brd(const UINT32 ganglia_md_id);

#endif/* _GANGLIA_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

