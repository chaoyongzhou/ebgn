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

#include <netdb.h>
#include <rpc/rpc.h>
#include <unistd.h>

#include "type.h"
#include "mm.h"
#include "log.h"

#include "cbc.h"

#include "cmisc.h"
#include "cmutex.h"
#include "cbytes.h"

#include "mod.inc"
#include "cmpic.inc"
#include "task.h"
#include "cbtimer.h"
#include "cmpie.h"
#include "cbitmap.h"
#include "cmon.h"

#include "gm_protocol.h"
#include "ganglia.h"

#include "csys.h"

#include "findex.inc"

#define GANGLIA_MD_CAPACITY()                  (cbc_md_capacity(MD_GANGLIA))

#define GANGLIA_MD_GET(ganglia_md_id)          ((GANGLIA_MD *)cbc_md_get(MD_GANGLIA, (ganglia_md_id)))

#define GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id)  \
    ((CMPI_ANY_MODI != (ganglia_md_id)) && ((NULL_PTR == GANGLIA_MD_GET(ganglia_md_id)) || (0 == (GANGLIA_MD_GET(ganglia_md_id)->usedcounter))))


/**
*   for test only
*
*   to query the status of GANGLIA Module
*
**/
void ganglia_print_module_status(const UINT32 ganglia_md_id, LOG *log)
{
    GANGLIA_MD *ganglia_md;
    UINT32 this_ganglia_md_id;

    for( this_ganglia_md_id = 0; this_ganglia_md_id < GANGLIA_MD_CAPACITY(); this_ganglia_md_id ++ )
    {
        ganglia_md = GANGLIA_MD_GET(this_ganglia_md_id);

        if ( NULL_PTR != ganglia_md && 0 < ganglia_md->usedcounter )
        {
            sys_log(log,"GANGLIA Module # %ld : %ld refered\n",
                    this_ganglia_md_id,
                    ganglia_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed GANGLIA module
*
*
**/
UINT32 ganglia_free_module_static_mem(const UINT32 ganglia_md_id)
{
    GANGLIA_MD  *ganglia_md;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_free_module_static_mem: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);

    free_module_static_mem(MD_GANGLIA, ganglia_md_id);

    return 0;
}

/**
*
* start GANGLIA module
*
**/
UINT32 ganglia_start(const UINT32 mcast_ipaddr, const UINT32 srv_port)
{
    GANGLIA_MD *ganglia_md;
    UINT32 ganglia_md_id;

    int udp_sender_sockfd;

    ganglia_md_id = cbc_md_new(MD_GANGLIA, sizeof(GANGLIA_MD));
    if(ERR_MODULE_ID == ganglia_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one GANGLIA module */
    ganglia_md = (GANGLIA_MD *)cbc_md_get(MD_GANGLIA, ganglia_md_id);
    ganglia_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();

    if(EC_FALSE == csocket_start_udp_mcast_sender(mcast_ipaddr, srv_port, &udp_sender_sockfd))
    {
        sys_log(LOGSTDOUT, "error:ganglia_start: start udp sender on %s:%ld failed\n",
                            c_word_to_ipv4(mcast_ipaddr), srv_port);
        cbc_md_free(MD_GANGLIA, ganglia_md_id);
        return (ERR_MODULE_ID);
    }

    GANGLIA_MD_CMON_MD_ID(ganglia_md)  = cmon_start();
    cmond_add_mod_mgr(GANGLIA_MD_CMON_MD_ID(ganglia_md),
                      CMPI_LOCAL_TCID, CMPI_LOCAL_COMM, CMPI_LOCAL_RANK, GANGLIA_MD_CMON_MD_ID(ganglia_md));

    /*monitor cpu*/
    GANGLIA_MD_OBJ_CPU_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_CPU_LOAD_EL,/*each cpu*/
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_CPU_VEC(ganglia_md));

    /*monitor dsk*/
    GANGLIA_MD_OBJ_DSK_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_DSK_LOAD_SL,
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_DSK_VEC(ganglia_md));

    /*monitor eth*/
    GANGLIA_MD_OBJ_ETH_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_ETH_LOAD_SL,
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_ETH_VEC(ganglia_md));

    /*monitor sys mem*/
    GANGLIA_MD_OBJ_SMM_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_MEM_LOAD_SL,
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_SMM_VEC(ganglia_md));

    /*monitor process mem*/
    GANGLIA_MD_OBJ_PRO_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_MEM_LOAD_PL,
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_PRO_VEC(ganglia_md));

    /*monitor thread on rank level*/
    GANGLIA_MD_OBJ_THR_VEC(ganglia_md) = cmon_obj_vec_new(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    cmon_obj_vec_incl(GANGLIA_MD_CMON_MD_ID(ganglia_md), CMON_OID_THREAD_STAT_RL,
                       CMPI_ANY_TCID, CMPI_ANY_COMM, CMPI_ANY_RANK, CMPI_ANY_MODI,
                       GANGLIA_MD_OBJ_THR_VEC(ganglia_md));

    ganglia_md->usedcounter = 1;
    GANGLIA_MD_MCAST_IPADDR(ganglia_md) = mcast_ipaddr;
    GANGLIA_MD_SRV_PORT(ganglia_md)     = srv_port;
    GANGLIA_MD_UDP_SENDER(ganglia_md)   = udp_sender_sockfd;
    clist_init(GANGLIA_MD_METRIC_LIST(ganglia_md), MM_IGNORE, LOC_GANGLIA_0001);

    sys_log(LOGSTDOUT, "ganglia_start: start GANGLIA module #%ld\n", ganglia_md_id);
    //sys_log(LOGSTDOUT, "========================= ganglia_start: GANGLIA table info:\n");
    //ganglia_print_module_status(ganglia_md_id, LOGSTDOUT);
    //cbc_print();

    return ( ganglia_md_id );
}

/**
*
* end GANGLIA module
*
**/
void ganglia_end(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    if(NULL_PTR == ganglia_md)
    {
        sys_log(LOGSTDOUT,"error:ganglia_end: ganglia_md_id = %ld not exist.\n", ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ganglia_md->usedcounter )
    {
        ganglia_md->usedcounter --;
        return ;
    }

    if ( 0 == ganglia_md->usedcounter )
    {
        sys_log(LOGSTDOUT,"error:ganglia_end: ganglia_md_id = %ld is not started.\n", ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    csocket_stop_udp_mcast_sender(GANGLIA_MD_UDP_SENDER(ganglia_md), GANGLIA_MD_MCAST_IPADDR(ganglia_md));

    /*free cpu monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_CPU_VEC(ganglia_md));
    GANGLIA_MD_OBJ_CPU_VEC(ganglia_md) = NULL_PTR;

    /*free dsk monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_DSK_VEC(ganglia_md));
    GANGLIA_MD_OBJ_DSK_VEC(ganglia_md) = NULL_PTR;

    /*free eth monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_ETH_VEC(ganglia_md));
    GANGLIA_MD_OBJ_ETH_VEC(ganglia_md) = NULL_PTR;

    /*free sys mem monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_SMM_VEC(ganglia_md));
    GANGLIA_MD_OBJ_SMM_VEC(ganglia_md) = NULL_PTR;

    /*free process mem monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_PRO_VEC(ganglia_md));
    GANGLIA_MD_OBJ_PRO_VEC(ganglia_md) = NULL_PTR;

    /*free rank level thread monitor*/
    cmon_obj_vec_free(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_THR_VEC(ganglia_md));
    GANGLIA_MD_OBJ_THR_VEC(ganglia_md) = NULL_PTR;

    cmon_end(GANGLIA_MD_CMON_MD_ID(ganglia_md));
    GANGLIA_MD_CMON_MD_ID(ganglia_md) = ERR_MODULE_ID;

    GANGLIA_MD_MCAST_IPADDR(ganglia_md) = CMPI_ERROR_IPADDR;
    GANGLIA_MD_SRV_PORT(ganglia_md)     = CMPI_ERROR_SRVPORT;
    GANGLIA_MD_UDP_SENDER(ganglia_md)   = CMPI_ERROR_SOCKFD;
    clist_clean(GANGLIA_MD_METRIC_LIST(ganglia_md), (CLIST_DATA_DATA_CLEANER)ganglia_free_0);
    /* free module : */
    //ganglia_free_module_static_mem(ganglia_md_id);

    ganglia_md->usedcounter = 0;

    sys_log(LOGSTDOUT, "ganglia_end: stop GANGLIA module #%ld\n", ganglia_md_id);
    cbc_md_free(MD_GANGLIA, ganglia_md_id);

    breathing_static_mem();

    //sys_log(LOGSTDOUT, "========================= ganglia_end: GANGLIA table info:\n");
    //ganglia_print_module_status(ganglia_md_id, LOGSTDOUT);
    //cbc_print();

    return ;
}

GANGLIA *ganglia_new(const UINT32 ganglia_md_id)
{
    GANGLIA *ganglia;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_new: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia = (GANGLIA *)SAFE_MALLOC(sizeof(GANGLIA), LOC_GANGLIA_0002);
    if(NULL_PTR != ganglia)
    {
        ganglia_init(ganglia_md_id, ganglia);
    }

    return (ganglia);
}

EC_BOOL ganglia_init(const UINT32 ganglia_md_id, GANGLIA *ganglia)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_new: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    BSET(ganglia, 0, sizeof(GANGLIA));
    return (EC_TRUE);
}

EC_BOOL ganglia_clean(const UINT32 ganglia_md_id, GANGLIA *ganglia)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_clean: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    if(NULL_PTR != ganglia)
    {
         xdr_free((xdrproc_t)xdr_Ganglia_metadata_msg, (char *)GANGLIA_FORMAT_MSG(ganglia));
         xdr_free((xdrproc_t)xdr_Ganglia_value_msg, (char *)GANGLIA_VALUE_MSG(ganglia));
    }
    return (EC_TRUE);
}

EC_BOOL ganglia_free(const UINT32 ganglia_md_id, GANGLIA *ganglia)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_free: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    if(NULL_PTR != ganglia)
    {
        ganglia_clean(ganglia_md_id, ganglia);
        SAFE_FREE(ganglia, LOC_GANGLIA_0003);
    }
    return (EC_TRUE);
}

EC_BOOL ganglia_clean_0(GANGLIA *ganglia)
{
    if(NULL_PTR != ganglia)
    {
        xdr_free((xdrproc_t)xdr_Ganglia_metadata_msg, (char *)GANGLIA_FORMAT_MSG(ganglia));
        xdr_free((xdrproc_t)xdr_Ganglia_value_msg, (char *)GANGLIA_VALUE_MSG(ganglia));
    }
    return (EC_TRUE);
}

EC_BOOL ganglia_free_0(GANGLIA *ganglia)
{
    if(NULL_PTR != ganglia)
    {
        ganglia_clean_0(ganglia);
        SAFE_FREE(ganglia, LOC_GANGLIA_0004);
    }
    return (EC_TRUE);
}

void ganglia_print(LOG *log, const GANGLIA *ganglia)
{
    ganglia_metadata_msg_print(log, GANGLIA_FORMAT_MSG(ganglia));
    ganglia_value_msg_print(log, GANGLIA_VALUE_MSG(ganglia));
    return;
}

EC_BOOL ganglia_send(const UINT32 ganglia_md_id, const GANGLIA *ganglia)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_send: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    if(EC_FALSE == ganglia_metadata_msg_send(ganglia_md_id, GANGLIA_FORMAT_MSG(ganglia)))
    {
        sys_log(LOGSTDOUT, "error:ganglia_send:encode metadata msg failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == ganglia_value_msg_send(ganglia_md_id, GANGLIA_VALUE_MSG(ganglia)))
    {
        sys_log(LOGSTDOUT, "error:ganglia_send:encode value msg failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

void ganglia_metadata_print(LOG *log, const Ganglia_metadata *metadata)
{
    u_int pos;

    for(pos = 0; pos < metadata->metadata_len; pos ++)
    {
        const Ganglia_extra_data *extra;
        extra = GANGLIA_METADATA_VAL(metadata, pos);
        sys_log(log, "extra %d# (name %s, data %s)\n", pos, GANGLIA_EXTRA_NAME(extra), GANGLIA_EXTRA_DATA(extra));
    }

    return;
}

void ganglia_metric_id_print(LOG *log, const Ganglia_metric_id *metric_id)
{
    sys_log(log, "metric_id = ( host %s, name %s, spoof %d)\n",
                GANGLIA_METRIC_ID_HOST(metric_id),
                GANGLIA_METRIC_ID_NAME(metric_id),
                GANGLIA_METRIC_ID_SPOOF(metric_id)
                );
    return;
}


void ganglia_metadata_message_print(LOG *log, const Ganglia_metadata_message *metadata_message)
{
    sys_log(log, "metric = (type %s, name %s, unit %s, slop %d, tmax %d, dmax %d)\n",
                  GANGLIA_METADATA_MESSAGE_TYPE(metadata_message),
                  GANGLIA_METADATA_MESSAGE_NAME(metadata_message),
                  GANGLIA_METADATA_MESSAGE_UNITS(metadata_message),
                  GANGLIA_METADATA_MESSAGE_SLOPE(metadata_message),
                  GANGLIA_METADATA_MESSAGE_TMAX(metadata_message),
                  GANGLIA_METADATA_MESSAGE_DMAX(metadata_message));

    ganglia_metadata_print(log, GANGLIA_METADATA_MESSAGE_META(metadata_message));
    return;
}

void ganglia_metadatadef_print(LOG *log, const Ganglia_metadatadef *metadatadef)
{
    ganglia_metric_id_print(log, GANGLIA_METADATADEF_METRIC_ID(metadatadef));
    ganglia_metadata_message_print(log, GANGLIA_METADATADEF_METADATA_MESSAGE(metadatadef));
    return;
}
void ganglia_metadata_msg_print(LOG *log, const Ganglia_metadata_msg *metadata_msg)
{
    if(GMETADATA_FULL == metadata_msg->id)
    {
        ganglia_metadatadef_print(log,GANGLIA_METADATA_MSG_DEF(metadata_msg));
        return;
    }
    sys_log(LOGSTDOUT, "error:ganglia_metadata_msg_print: invalid metadata msg id %d\n", metadata_msg->id);
    return ;
}

void ganglia_value_msg_print(LOG *log, const Ganglia_value_msg *vmsg)
{
    sys_log(log, "value msg id %d\n", GANGLIA_VALUE_MSG_ID(vmsg));

    switch(GANGLIA_VALUE_MSG_ID(vmsg))
    {
        case GMETRIC_USHORT:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_USHORT_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, ushort val: %d\n",
                        GANGLIA_VALUE_MSG_USHORT_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_USHORT_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_SHORT:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_SHORT_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, short val: %d\n",
                        GANGLIA_VALUE_MSG_SHORT_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_SHORT_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_INT:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_INT_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, int val: %d\n",
                        GANGLIA_VALUE_MSG_INT_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_INT_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_UINT:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_UINT_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, uint val: %d\n",
                        GANGLIA_VALUE_MSG_UINT_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_UINT_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_STRING:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_STRING_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, string: %s\n",
                        GANGLIA_VALUE_MSG_STRING_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_STRING_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_FLOAT:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_FLOAT_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, float: %.2f\n",
                        GANGLIA_VALUE_MSG_FLOAT_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_FLOAT_GMETRIC_VAL(vmsg));
            break;
        }
        case GMETRIC_DOUBLE:
        {
            ganglia_metric_id_print(log, GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_ID(vmsg));
            sys_log(log, "format: %s, double: %.2f\n",
                        GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_FMT(vmsg),
                        GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_VAL(vmsg));
            break;
        }
        default:
        {
            sys_log(log, "error:ganglia_value_msg_print: invalid value msg id %d\n", GANGLIA_VALUE_MSG_ID(vmsg));
            return;
        }
    }

    return;
}

EC_BOOL ganglia_metric_id_set(const UINT32 ganglia_md_id, Ganglia_metric_id *metric_id, const char *host, const char *metric_name)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metric_id_set: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_METRIC_ID_SET(metric_id, host, metric_name, 0);

    return (EC_TRUE);
}

EC_BOOL ganglia_metadata_add(const UINT32 ganglia_md_id, Ganglia_metadata *metadata, const char *name, const char *data)
{
    u_int metadata_len;
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metadata_add: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    metadata_len = GANGLIA_METADATA_LEN(metadata);
    if(0 == metadata_len)
    {
        Ganglia_extra_data *metadata_val;
        Ganglia_extra_data *metadata_extra;

        metadata_val = (Ganglia_extra_data *)malloc((metadata_len + 1) * sizeof(Ganglia_extra_data));

        metadata_extra = (metadata_val + 0);
        GANGLIA_EXTRA_NAME(metadata_extra) = strdup(name);
        GANGLIA_EXTRA_DATA(metadata_extra) = strdup(data);

        GANGLIA_METADATA_VALS(metadata) = metadata_val;

        return (EC_TRUE);
    }
    else
    {
        Ganglia_extra_data *metadata_val_old;
        Ganglia_extra_data *metadata_extra_old;

        Ganglia_extra_data *metadata_val_new;
        Ganglia_extra_data *metadata_extra_new;
        u_int pos;

        metadata_val_old = GANGLIA_METADATA_VALS(metadata);
        metadata_val_new = (Ganglia_extra_data *)malloc((metadata_len + 1) * sizeof(Ganglia_extra_data));

        for(pos = 0; pos < metadata_len; pos ++)
        {
            metadata_extra_old = (metadata_val_old + pos);
            metadata_extra_new = (metadata_val_new + pos);

            GANGLIA_EXTRA_NAME(metadata_extra_new) = GANGLIA_EXTRA_NAME(metadata_extra_old);
            GANGLIA_EXTRA_DATA(metadata_extra_new) = GANGLIA_EXTRA_DATA(metadata_extra_old);
        }

        metadata_extra_new = (metadata_val_new + pos);
        GANGLIA_EXTRA_NAME(metadata_extra_new) = strdup(name);
        GANGLIA_EXTRA_DATA(metadata_extra_new) = strdup(data);

        GANGLIA_METADATA_VALS(metadata) = metadata_val_new;
        free(metadata_val_old);
    }

    return (EC_TRUE);
}

EC_BOOL ganglia_metadata_message_set(const UINT32 ganglia_md_id, Ganglia_metadata_message *metadata_message,
                                                const char *group_name,
                                                const char *val_type, const char *val_name, const char *val_units,
                                                const u_int slope, const u_int tmax, const u_int dmax)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metadata_message_set: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_METADATA_MESSAGE_TYPE_SET(metadata_message , val_type);
    GANGLIA_METADATA_MESSAGE_NAME_SET(metadata_message , val_name);
    GANGLIA_METADATA_MESSAGE_UNITS_SET(metadata_message, val_units);

    GANGLIA_METADATA_MESSAGE_SLOPE_SET(metadata_message, slope);
    GANGLIA_METADATA_MESSAGE_TMAX_SET(metadata_message , tmax);
    GANGLIA_METADATA_MESSAGE_DMAX_SET(metadata_message , dmax);

    ganglia_metadata_add(ganglia_md_id, GANGLIA_METADATA_MESSAGE_META(metadata_message), (char *)"GROUP", group_name);
    //ganglia_metadata_add(ganglia_md_id, GANGLIA_METADATA_MESSAGE_META(metadata_message), (char *)"DESC" , "xxxxxxxx");
    //ganglia_metadata_add(ganglia_md_id, GANGLIA_METADATA_MESSAGE_META(metadata_message), (char *)"TITLE", "yyyyyyyy");

    return (EC_TRUE);
}

EC_BOOL ganglia_metadata_msg_set(const UINT32 ganglia_md_id, Ganglia_metadata_msg *metadata_msg,
                                       const char *host, const char *metric_name, const char *group_name,
                                       const char *val_type, const char *val_name, const char *val_units,
                                       const u_int slope, const u_int tmax, const u_int dmax)
{
    Ganglia_metadatadef      *gfull;
    Ganglia_metric_id        *metric_id;
    Ganglia_metadata_message *metric;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metadata_msg_set: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_METADATA_MSG_ID(metadata_msg) = GMETADATA_FULL;
    gfull = GANGLIA_METADATA_MSG_DEF(metadata_msg);

    metric_id = GANGLIA_METADATADEF_METRIC_ID(gfull);
    ganglia_metric_id_set(ganglia_md_id, metric_id, host, metric_name);

    metric = GANGLIA_METADATADEF_METADATA_MESSAGE(gfull);
    ganglia_metadata_message_set(ganglia_md_id, metric, group_name, val_type, val_name, val_units, slope, tmax, dmax);

    return (EC_TRUE);
}

EC_BOOL ganglia_metadata_msg_clean(const UINT32 ganglia_md_id, Ganglia_metadata_msg *metadata_msg)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metadata_msg_clean: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    if(NULL_PTR != metadata_msg)
    {
        xdr_free((xdrproc_t)xdr_Ganglia_metadata_msg, (char *)metadata_msg);
    }
    return (EC_TRUE);
}

EC_BOOL ganglia_metadata_msg_send(const UINT32 ganglia_md_id, const Ganglia_metadata_msg *metadata_msg)
{
    GANGLIA_MD *ganglia_md;
    XDR xdr;
    UINT8 *buf;
    u_int len;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_metadata_msg_send: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);

    buf = (UINT8 *)SAFE_MALLOC(GANGLIA_MAX_MESSAGE_LEN, LOC_GANGLIA_0005);
    if(NULL_PTR == buf)
    {
        sys_log(LOGSTDOUT, "error:ganglia_metadata_msg_send: alloc %d bytes failed\n", GANGLIA_MAX_MESSAGE_LEN);
        return (EC_FALSE);
    }

    xdrmem_create(&xdr, (char *)buf, GANGLIA_MAX_MESSAGE_LEN, XDR_ENCODE);

    if(!xdr_Ganglia_metadata_msg(&xdr, (Ganglia_metadata_msg *)metadata_msg))
    {
        sys_log(LOGSTDOUT, "error:ganglia_metadata_msg_send:encode metadata msg failed\n");
        SAFE_FREE(buf, LOC_GANGLIA_0006);
        return (EC_FALSE);
    }

    len = xdr_getpos(&xdr);

    if(EC_FALSE == csocket_udp_sendto(GANGLIA_MD_UDP_SENDER(ganglia_md),
                                        GANGLIA_MD_MCAST_IPADDR(ganglia_md),
                                        GANGLIA_MD_SRV_PORT(ganglia_md),
                                        buf, len))
    {
        SAFE_FREE(buf, LOC_GANGLIA_0007);
        sys_log(LOGSTDOUT, "error:ganglia_metadata_msg_send: send %d bytes to mcast %s:%ld on sockfd %d failed\n",
                            len,
                            c_word_to_ipv4(GANGLIA_MD_MCAST_IPADDR(ganglia_md)), GANGLIA_MD_SRV_PORT(ganglia_md),
                            GANGLIA_MD_UDP_SENDER(ganglia_md)
                            );
        return (EC_FALSE);
    }

    SAFE_FREE(buf, LOC_GANGLIA_0008);
    sys_log(LOGSTDOUT, "[DEBUG] ganglia_metadata_msg_send: send %d bytes to mcast %s:%ld on sockfd %d successfully\n",
                        len,
                        c_word_to_ipv4(GANGLIA_MD_MCAST_IPADDR(ganglia_md)), GANGLIA_MD_SRV_PORT(ganglia_md),
                        GANGLIA_MD_UDP_SENDER(ganglia_md)
                        );
    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_ushort(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const u_short val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_ushort: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_USHORT);
    GANGLIA_VALUE_MSG_USHORT_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_USHORT_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_USHORT_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_short(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const short val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_short: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_SHORT);
    GANGLIA_VALUE_MSG_SHORT_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_SHORT_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_SHORT_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_int(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const int val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_int: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_INT);
    GANGLIA_VALUE_MSG_INT_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_INT_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_INT_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_uint(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const u_int val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_uint: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_UINT);
    GANGLIA_VALUE_MSG_UINT_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_UINT_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_UINT_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_string(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const char *val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_string: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_STRING);
    GANGLIA_VALUE_MSG_STRING_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_STRING_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_STRING_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_float(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const float val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_float: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_FLOAT);
    GANGLIA_VALUE_MSG_FLOAT_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_FLOAT_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_FLOAT_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_set_double(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg,
                                                const char *host, const char *metric_name,
                                                const char *val_fmt, const double val)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_set_double: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    GANGLIA_VALUE_MSG_FORMAT_ID_SET(value_msg, GMETRIC_DOUBLE);
    GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_ID_SET(value_msg, host, metric_name, 0);
    GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_FMT_SET(value_msg, val_fmt);
    GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_VAL_SET(value_msg, val);

    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_clean(const UINT32 ganglia_md_id, Ganglia_value_msg *value_msg)
{
#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_clean: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    if(NULL_PTR != value_msg)
    {
        xdr_free((xdrproc_t)xdr_Ganglia_value_msg, (char *)value_msg);
    }
    return (EC_TRUE);
}

EC_BOOL ganglia_value_msg_send(const UINT32 ganglia_md_id, const Ganglia_value_msg *value_msg)
{
    GANGLIA_MD *ganglia_md;
    XDR xdr;
    UINT8 *buf;
    u_int len;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_value_msg_send: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);

    buf = (UINT8 *)SAFE_MALLOC(GANGLIA_MAX_MESSAGE_LEN, LOC_GANGLIA_0009);
    if(NULL_PTR == buf)
    {
        sys_log(LOGSTDOUT, "error:ganglia_value_msg_send: alloc %d bytes failed\n", GANGLIA_MAX_MESSAGE_LEN);
        return (EC_FALSE);
    }

    xdrmem_create(&xdr, (char *)buf, GANGLIA_MAX_MESSAGE_LEN, XDR_ENCODE);

    if(!xdr_Ganglia_value_msg(&xdr, (Ganglia_value_msg *)value_msg))
    {
        sys_log(LOGSTDOUT, "error:ganglia_value_msg_send:encode value msg failed\n");
        SAFE_FREE(buf, LOC_GANGLIA_0010);
        return (EC_FALSE);
    }

    len = xdr_getpos(&xdr);

    if(EC_FALSE == csocket_udp_sendto(GANGLIA_MD_UDP_SENDER(ganglia_md),
                                        GANGLIA_MD_MCAST_IPADDR(ganglia_md),
                                        GANGLIA_MD_SRV_PORT(ganglia_md),
                                        buf, len))
    {
        SAFE_FREE(buf, LOC_GANGLIA_0011);
        sys_log(LOGSTDOUT, "error:ganglia_value_msg_send: send %d bytes to mcast %s:%ld on sockfd %d failed\n",
                            len,
                            c_word_to_ipv4(GANGLIA_MD_MCAST_IPADDR(ganglia_md)), GANGLIA_MD_SRV_PORT(ganglia_md),
                            GANGLIA_MD_UDP_SENDER(ganglia_md)
                            );
        return (EC_FALSE);
    }

    SAFE_FREE(buf, LOC_GANGLIA_0012);
    sys_log(LOGSTDOUT, "[DEBUG] ganglia_value_msg_send: send %d bytes to mcast %s:%ld on sockfd %d successfully\n",
                        len,
                        c_word_to_ipv4(GANGLIA_MD_MCAST_IPADDR(ganglia_md)), GANGLIA_MD_SRV_PORT(ganglia_md),
                        GANGLIA_MD_UDP_SENDER(ganglia_md)
                        );
    return (EC_TRUE);
}

EC_BOOL ganglia_report_cpus_load0(const UINT32 ganglia_md_id)
{
    CSYS_CPU_STAT_VEC *csys_cpu_stat_vec;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_cpus_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    csys_cpu_stat_vec = csys_cpu_stat_vec_new();
    csys_cpu_stat_vec_get(csys_cpu_stat_vec);

    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_cpus_load: cpus load\n");
    csys_cpu_stat_vec_print(LOGSTDOUT, csys_cpu_stat_vec);

#if 0
    csys_cpu_stat_num = csys_cpu_stat_vec_size(csys_cpu_stat_vec);
    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat;

        csys_cpu_stat = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);
    }
#endif
    csys_cpu_stat_vec_free(csys_cpu_stat_vec);
    return (EC_TRUE);
}

EC_BOOL __ganglia_report_cpu_cmon_obj(const UINT32 ganglia_md_id, CMON_OBJ * cmon_obj)
{
    GANGLIA ganglia;

    CVECTOR  *csys_cpu_stat_vec;

    UINT32 csys_cpu_stat_pos;
    UINT32 csys_cpu_stat_num;

    ganglia_init(ganglia_md_id, &ganglia);

    csys_cpu_stat_vec = (CVECTOR *)CMON_OBJ_DATA_VEC(cmon_obj, CMON_REPORT_DATA_AREA);
    csys_cpu_stat_num = cvector_size(csys_cpu_stat_vec);

    for(csys_cpu_stat_pos = 0; csys_cpu_stat_pos < csys_cpu_stat_num; csys_cpu_stat_pos ++)
    {
        CSYS_CPU_STAT *csys_cpu_stat;

        csys_cpu_stat = (CSYS_CPU_STAT *)cvector_get(csys_cpu_stat_vec, csys_cpu_stat_pos);

        ganglia_metadata_msg_set(ganglia_md_id, GANGLIA_FORMAT_MSG(&ganglia), (const char *)"127.0.0.1",
                             (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat)), "hansoul",
                             "float", "cpu load", "n.a.",
                             GANGLIA_SLOPE_ZERO, 5, 0);

        ganglia_value_msg_set_float(ganglia_md_id, GANGLIA_VALUE_MSG(&ganglia), (const char *)"127.0.0.1",
                            (char *)cstring_get_str(CSYS_CPU_STAT_CSTR(csys_cpu_stat)),
                            "%.2f", CSYS_CPU_STAT_LOAD(csys_cpu_stat));

        ganglia_print(LOGSTDOUT, &ganglia);
        ganglia_send(ganglia_md_id, &ganglia);
        ganglia_clean(ganglia_md_id, &ganglia);
    }

    return (EC_TRUE);
}

EC_BOOL ganglia_report_cpus_load(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_cpus_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_CPU_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_cpus_load: xxxx cpus load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_CPU_VEC(ganglia_md));

    cmon_obj_num = cvector_size(CMON_OBJ_VEC(GANGLIA_MD_OBJ_CPU_VEC(ganglia_md)));
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;

        cmon_obj = (CMON_OBJ *)cvector_get(CMON_OBJ_VEC(GANGLIA_MD_OBJ_CPU_VEC(ganglia_md)), cmon_obj_pos);
        __ganglia_report_cpu_cmon_obj(ganglia_md_id, cmon_obj);
    }

    return (EC_TRUE);
}

EC_BOOL ganglia_report_sys_mem_load0(const UINT32 ganglia_md_id)
{
    CSYS_MEM_STAT *csys_mem_stat;
    REAL mem_load;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_sys_mem_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    csys_mem_stat = csys_mem_stat_new();
    csys_mem_stat_get(csys_mem_stat);

    mem_load = 100.0 *(1.0 * (CSYS_MEM_TOTAL(csys_mem_stat) - CSYS_MEM_FREE(csys_mem_stat)) / CSYS_MEM_TOTAL(csys_mem_stat));
    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_sys_mem_load: (%ld - %ld) / %ld => %.1f\n",
                        CSYS_MEM_TOTAL(csys_mem_stat),
                        CSYS_MEM_FREE(csys_mem_stat),
                        CSYS_MEM_TOTAL(csys_mem_stat),
                        mem_load);
    csys_mem_stat_free(csys_mem_stat);

    return (EC_TRUE);
}

EC_BOOL ganglia_report_sys_mem_load(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_cpus_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_SMM_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_sys_mem_load: xxxx system mem load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_SMM_VEC(ganglia_md));

    return (EC_TRUE);
}


EC_BOOL ganglia_report_proc_mem_load0(const UINT32 ganglia_md_id)
{
    CPROC_MEM_STAT *cproc_mem_stat;
    REAL mem_load;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_proc_mem_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    cproc_mem_stat = cproc_mem_stat_new();

    cproc_mem_stat_get(cproc_mem_stat);

    mem_load = CPROC_MEM_LOAD(cproc_mem_stat);
    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_proc_mem_load: proc mem load = %.2f\n", mem_load);

    cproc_mem_stat_free(cproc_mem_stat);

    return (EC_TRUE);
}

EC_BOOL ganglia_report_proc_mem_load(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_cpus_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_PRO_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_proc_mem_load: xxxx process mem load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_PRO_VEC(ganglia_md));

    return (EC_TRUE);
}

EC_BOOL ganglia_report_crank_thread_stat0(const UINT32 ganglia_md_id)
{
    CRANK_THREAD_STAT crank_thread_stat;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_crank_thread_stat: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    crank_thread_stat_get(&crank_thread_stat);

    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_crank_thread_stat: current rank thread stat\n");
    crank_thread_stat_print(LOGSTDOUT, &crank_thread_stat);

    return (EC_TRUE);
}

EC_BOOL __ganglia_report_crank_thread_cmon_obj(const UINT32 ganglia_md_id, CMON_OBJ * cmon_obj)
{
    GANGLIA ganglia;

    CRANK_THREAD_STAT *crank_thread_stat;

    ganglia_init(ganglia_md_id, &ganglia);

    crank_thread_stat = CMON_OBJ_DATA_CRANK_THREAD_STAT((CMON_OBJ *)cmon_obj, CMON_REPORT_DATA_AREA);

    ganglia_metadata_msg_set(ganglia_md_id, GANGLIA_FORMAT_MSG(&ganglia), (const char *)"127.0.0.1",
                         "rank thread num", "hansoul",
                         "uint16", "thread load", "n.a.",
                         GANGLIA_SLOPE_POSITIVE, 5, 0);

    ganglia_value_msg_set_ushort(ganglia_md_id, GANGLIA_VALUE_MSG(&ganglia), (const char *)"127.0.0.1",
                        "rank thread num",
                        "%d", (u_short)CRANK_THREAD_BUSY_NUM(crank_thread_stat));

    ganglia_print(LOGSTDOUT, &ganglia);
    ganglia_send(ganglia_md_id, &ganglia);
    ganglia_clean(ganglia_md_id, &ganglia);

    return (EC_TRUE);
}

EC_BOOL ganglia_report_crank_thread_stat(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

    UINT32     cmon_obj_pos;
    UINT32     cmon_obj_num;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_cpus_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_THR_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_crank_thread_stat: xxxx rank level thread load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_THR_VEC(ganglia_md));

    cmon_obj_num = cmon_obj_vec_size(GANGLIA_MD_CMON_MD_ID(ganglia_md), GANGLIA_MD_OBJ_THR_VEC(ganglia_md));;
    for(cmon_obj_pos = 0; cmon_obj_pos < cmon_obj_num; cmon_obj_pos ++)
    {
        CMON_OBJ * cmon_obj;
        cmon_obj = cmon_obj_vec_get(GANGLIA_MD_CMON_MD_ID(ganglia_md), cmon_obj_pos, GANGLIA_MD_OBJ_THR_VEC(ganglia_md));

        __ganglia_report_crank_thread_cmon_obj(ganglia_md_id, cmon_obj);
    }

    return (EC_TRUE);
}

EC_BOOL ganglia_report_dsks_stat0(const UINT32 ganglia_md_id)
{
    CSYS_DSK_VEC *csys_dsk_stat_vec;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_crank_thread_stat: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    csys_dsk_stat_vec = csys_dsk_stat_vec_new();
    csys_dsk_stat_vec_get(csys_dsk_stat_vec);

    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_dsks_stat: disks stat\n");
    csys_dsk_stat_vec_print(LOGSTDOUT, csys_dsk_stat_vec);

#if 0
    csys_dsk_stat_num = csys_dsk_stat_vec_size(csys_dsk_stat_vec);
    for(csys_dsk_stat_pos = 0; csys_dsk_stat_pos < csys_dsk_stat_num; csys_dsk_stat_pos ++)
    {
        CSYS_DSK_STAT *csys_dsk_stat;

        csys_dsk_stat = (CSYS_DSK_STAT *)cvector_get(csys_dsk_stat_vec, csys_dsk_stat_pos);
    }
#endif
    csys_dsk_stat_vec_free(csys_dsk_stat_vec);

    return (EC_TRUE);
}

EC_BOOL ganglia_report_dsks_stat(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_dsks_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_DSK_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_dsks_load: xxxx dsks load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_DSK_VEC(ganglia_md));

    return (EC_TRUE);
}

EC_BOOL ganglia_report_eths_stat0(const UINT32 ganglia_md_id)
{
    CSYS_ETH_VEC *csys_eth_stat_vec;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_crank_thread_stat: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    csys_eth_stat_vec = csys_eth_stat_vec_new();
    csys_eth_stat_vec_get(csys_eth_stat_vec);

    sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_dsks_stat: eths stat\n");
    csys_eth_stat_vec_print(LOGSTDOUT, csys_eth_stat_vec);

#if 0
    csys_eth_stat_num = csys_eth_stat_vec_size(csys_eth_stat_vec);
    for(csys_eth_stat_pos = 0; csys_eth_stat_pos < csys_eth_stat_num; csys_eth_stat_pos ++)
    {
        CSYS_ETH_STAT *csys_eth_stat;

        csys_eth_stat = (CSYS_ETH_STAT *)cvector_get(csys_eth_stat_vec, csys_eth_stat_pos);
    }
#endif
    csys_eth_stat_vec_free(csys_eth_stat_vec);

    return (0);
}

EC_BOOL ganglia_report_eths_stat(const UINT32 ganglia_md_id)
{
    GANGLIA_MD *ganglia_md;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_report_eths_load: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    ganglia_md = GANGLIA_MD_GET(ganglia_md_id);
    cmon_obj_vec_meas(GANGLIA_MD_CMON_MD_ID(ganglia_md), TASK_DEFAULT_LIVE, GANGLIA_MD_OBJ_ETH_VEC(ganglia_md));
    //sys_log(LOGSTDOUT, "[DEBUG] ganglia_report_eths_load: xxxx eths load\n");
    cmon_obj_vec_print(GANGLIA_MD_CMON_MD_ID(ganglia_md), LOGSTDOUT, GANGLIA_MD_OBJ_ETH_VEC(ganglia_md));

    return (EC_TRUE);
}

EC_BOOL ganglia_register_to_task_brd(const UINT32 ganglia_md_id)
{
    TASK_BRD * task_brd;

#if ( SWITCH_ON == GANGLIA_DEBUG_SWITCH )
    if ( GANGLIA_MD_ID_CHECK_INVALID(ganglia_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ganglia_register_to_task_brd: ganglia module #0x%lx not started.\n",
                ganglia_md_id);
        dbg_exit(MD_GANGLIA, ganglia_md_id);
    }
#endif/*GANGLIA_DEBUG_SWITCH*/

    task_brd = task_brd_default_get();

    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 3 /*timeout in seconds*/, FI_ganglia_report_cpus_load, ganglia_md_id);
    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 5 /*timeout in seconds*/, FI_ganglia_report_sys_mem_load, ganglia_md_id);
    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 7 /*timeout in seconds*/, FI_ganglia_report_proc_mem_load, ganglia_md_id);
    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 3 /*timeout in seconds*/, FI_ganglia_report_crank_thread_stat, ganglia_md_id);
    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 15/*timeout in seconds*/, FI_ganglia_report_dsks_stat, ganglia_md_id);
    task_brd_cbtimer_register(task_brd, 0/*never expire*/, 11/*timeout in seconds*/, FI_ganglia_report_eths_stat, ganglia_md_id);

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

