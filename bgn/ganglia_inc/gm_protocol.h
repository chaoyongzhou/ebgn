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

#ifndef _GM_PROTOCOL_H_RPCGEN
#define _GM_PROTOCOL_H_RPCGEN

#include <string.h>
#include <rpc/rpc.h>


#define GANGLIA_STRDUP(des_str, src_str)   do{\
    if(des_str) {\
        free(des_str);\
        (des_str) = (char *)0;\
    }\
    if(src_str) {\
        (des_str) = strdup(src_str);\
    }\
}while(0)

#define UDP_HEADER_SIZE         28
#define MAX_DESC_LEN            128
#define GM_PROTOCOL_GUARD

enum Ganglia_value_types {
    GANGLIA_VALUE_UNKNOWN = 0,
    GANGLIA_VALUE_STRING = 1,
    GANGLIA_VALUE_UNSIGNED_SHORT = 2,
    GANGLIA_VALUE_SHORT = 3,
    GANGLIA_VALUE_UNSIGNED_INT = 4,
    GANGLIA_VALUE_INT = 5,
    GANGLIA_VALUE_FLOAT = 6,
    GANGLIA_VALUE_DOUBLE = 7,
};
typedef enum Ganglia_value_types Ganglia_value_types;


struct Ganglia_extra_data {
    char *name;
    char *data;
};
typedef struct Ganglia_extra_data Ganglia_extra_data;
#define GANGLIA_EXTRA_NAME(extra)      ((extra)->name)
#define GANGLIA_EXTRA_DATA(extra)      ((extra)->data)

struct Ganglia_metadata{
    u_int metadata_len;
    struct Ganglia_extra_data *metadata_val;
};
typedef struct Ganglia_metadata Ganglia_metadata;
#define GANGLIA_METADATA_LEN(__metadata)          ((__metadata)->metadata_len)
#define GANGLIA_METADATA_VALS(__metadata)         ((__metadata)->metadata_val)
#define GANGLIA_METADATA_VAL(__metadata, pos)     ((__metadata)->metadata_val + (pos))

struct Ganglia_metadata_message {
    char *type;
    char *name;
    char *units;
    u_int slope;
    u_int tmax;
    u_int dmax;

    Ganglia_metadata metadata;
};
typedef struct Ganglia_metadata_message Ganglia_metadata_message;
#define GANGLIA_METADATA_MESSAGE_TYPE(metadata_message)     ((metadata_message)->type)
#define GANGLIA_METADATA_MESSAGE_NAME(metadata_message)     ((metadata_message)->name)
#define GANGLIA_METADATA_MESSAGE_UNITS(metadata_message)    ((metadata_message)->units)
#define GANGLIA_METADATA_MESSAGE_SLOPE(metadata_message)    ((metadata_message)->slope)
#define GANGLIA_METADATA_MESSAGE_TMAX(metadata_message)     ((metadata_message)->tmax)
#define GANGLIA_METADATA_MESSAGE_DMAX(metadata_message)     ((metadata_message)->dmax)
#define GANGLIA_METADATA_MESSAGE_META(metadata_message)     (&((metadata_message)->metadata))

#define GANGLIA_METADATA_MESSAGE_TYPE_SET(metadata_message , type)    GANGLIA_STRDUP(GANGLIA_METADATA_MESSAGE_TYPE(metadata_message) , (type))
#define GANGLIA_METADATA_MESSAGE_NAME_SET(metadata_message , name)    GANGLIA_STRDUP(GANGLIA_METADATA_MESSAGE_NAME(metadata_message) , (name))
#define GANGLIA_METADATA_MESSAGE_UNITS_SET(metadata_message, units)   GANGLIA_STRDUP(GANGLIA_METADATA_MESSAGE_UNITS(metadata_message), (units))

#define GANGLIA_METADATA_MESSAGE_SLOPE_SET(metadata_message, slope)   (GANGLIA_METADATA_MESSAGE_SLOPE(metadata_message) = (slope))
#define GANGLIA_METADATA_MESSAGE_TMAX_SET(metadata_message , tmax)    (GANGLIA_METADATA_MESSAGE_TMAX(metadata_message) = (tmax))
#define GANGLIA_METADATA_MESSAGE_DMAX_SET(metadata_message , dmax)    (GANGLIA_METADATA_MESSAGE_DMAX(metadata_message) = (dmax))


struct Ganglia_metric_id {
    char *host;
    char *name;
    bool_t spoof;
};
typedef struct Ganglia_metric_id Ganglia_metric_id;
#define GANGLIA_METRIC_ID_HOST(metric_id)     ((metric_id)->host)
#define GANGLIA_METRIC_ID_NAME(metric_id)     ((metric_id)->name)
#define GANGLIA_METRIC_ID_SPOOF(metric_id)    ((metric_id)->spoof)

#define GANGLIA_METRIC_ID_SET(metric_id, host, name, spoof) do{\
    GANGLIA_STRDUP(GANGLIA_METRIC_ID_HOST(metric_id), (host));\
    GANGLIA_STRDUP(GANGLIA_METRIC_ID_NAME(metric_id), (name));\
    GANGLIA_METRIC_ID_SPOOF(metric_id)  = (spoof);\
}while(0)

struct Ganglia_metadatadef {
    struct Ganglia_metric_id metric_id;
    struct Ganglia_metadata_message metric;
};
typedef struct Ganglia_metadatadef Ganglia_metadatadef;
#define GANGLIA_METADATADEF_METRIC_ID(metadata_def)            (&((metadata_def)->metric_id))
#define GANGLIA_METADATADEF_METADATA_MESSAGE(metadata_def)     (&((metadata_def)->metric))

struct Ganglia_metadatareq {
    struct Ganglia_metric_id metric_id;
};
typedef struct Ganglia_metadatareq Ganglia_metadatareq;
#define GANGLIA_METADATAREQ_METRIC_ID(metadata_req)            (&((metadata_req)->metric_id))

struct Ganglia_gmetric_ushort {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    u_short us;
};
typedef struct Ganglia_gmetric_ushort Ganglia_gmetric_ushort;
#define GANGLIA_GMETRIC_USHORT_METRIC_ID(metric_ushort)        (&((metric_ushort)->metric_id))
#define GANGLIA_GMETRIC_USHORT_FMT(metric_ushort)              ((metric_ushort)->fmt)
#define GANGLIA_GMETRIC_USHORT_VAL(metric_ushort)              ((metric_ushort)->us)

struct Ganglia_gmetric_short {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    short ss;
};
typedef struct Ganglia_gmetric_short Ganglia_gmetric_short;
#define GANGLIA_GMETRIC_SHORT_METRIC_ID(metric_short)        (&((metric_short)->metric_id))
#define GANGLIA_GMETRIC_SHORT_FMT(metric_short)              ((metric_short)->fmt)
#define GANGLIA_GMETRIC_SHORT_VAL(metric_short)              ((metric_short)->ss)

struct Ganglia_gmetric_int {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    int si;
};
typedef struct Ganglia_gmetric_int Ganglia_gmetric_int;
#define GANGLIA_GMETRIC_INT_METRIC_ID(metric_int)        (&((metric_int)->metric_id))
#define GANGLIA_GMETRIC_INT_FMT(metric_int)              ((metric_int)->fmt)
#define GANGLIA_GMETRIC_INT_VAL(metric_int)              ((metric_int)->si)

struct Ganglia_gmetric_uint {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    u_int ui;
};
typedef struct Ganglia_gmetric_uint Ganglia_gmetric_uint;
#define GANGLIA_GMETRIC_UINT_METRIC_ID(metric_uint)        (&((metric_uint)->metric_id))
#define GANGLIA_GMETRIC_UINT_FMT(metric_uint)              ((metric_uint)->fmt)
#define GANGLIA_GMETRIC_UINT_VAL(metric_uint)              ((metric_uint)->ui)

struct Ganglia_gmetric_string {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    char *str;
};
typedef struct Ganglia_gmetric_string Ganglia_gmetric_string;
#define GANGLIA_GMETRIC_STRING_METRIC_ID(metric_string)        (&((metric_string)->metric_id))
#define GANGLIA_GMETRIC_STRING_FMT(metric_string)              ((metric_string)->fmt)
#define GANGLIA_GMETRIC_STRING_VAL(metric_string)              ((metric_string)->str)

struct Ganglia_gmetric_float {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    float f;
};
typedef struct Ganglia_gmetric_float Ganglia_gmetric_float;
#define GANGLIA_GMETRIC_FLOAT_METRIC_ID(metric_float)        (&((metric_float)->metric_id))
#define GANGLIA_GMETRIC_FLOAT_FMT(metric_float)              ((metric_float)->fmt)
#define GANGLIA_GMETRIC_FLOAT_VAL(metric_float)              ((metric_float)->f)

struct Ganglia_gmetric_double {
    struct Ganglia_metric_id metric_id;
    char *fmt;
    double d;
};
typedef struct Ganglia_gmetric_double Ganglia_gmetric_double;
#define GANGLIA_GMETRIC_DOUBLE_METRIC_ID(metric_double)        (&((metric_double)->metric_id))
#define GANGLIA_GMETRIC_DOUBLE_FMT(metric_double)              ((metric_double)->fmt)
#define GANGLIA_GMETRIC_DOUBLE_VAL(metric_double)              ((metric_double)->d)

enum Ganglia_msg_formats {
    GMETADATA_FULL = 128,
    GMETRIC_USHORT = 128 + 1,
    GMETRIC_SHORT = 128 + 2,
    GMETRIC_INT = 128 + 3,
    GMETRIC_UINT = 128 + 4,
    GMETRIC_STRING = 128 + 5,
    GMETRIC_FLOAT = 128 + 6,
    GMETRIC_DOUBLE = 128 + 7,
    GMETADATA_REQUEST = 128 + 8,
};
typedef enum Ganglia_msg_formats Ganglia_msg_formats;

struct Ganglia_metadata_msg {
    Ganglia_msg_formats id;
    union {
        Ganglia_metadatadef gfull;
        Ganglia_metadatareq grequest;
    } Ganglia_metadata_msg_u;
};
typedef struct Ganglia_metadata_msg Ganglia_metadata_msg;
#define GANGLIA_METADATA_MSG_ID(metadata_msg)           ((metadata_msg)->id)
#define GANGLIA_METADATA_MSG_DEF(metadata_msg)          (&((metadata_msg)->Ganglia_metadata_msg_u.gfull))
#define GANGLIA_METADATA_MSG_REQ(metadata_msg)          (&((metadata_msg)->Ganglia_metadata_msg_u.grequest))

struct Ganglia_value_msg {
    Ganglia_msg_formats id;
    union {
        Ganglia_gmetric_ushort gu_short;
        Ganglia_gmetric_short gs_short;
        Ganglia_gmetric_int gs_int;
        Ganglia_gmetric_uint gu_int;
        Ganglia_gmetric_string gstr;
        Ganglia_gmetric_float gf;
        Ganglia_gmetric_double gd;
    } Ganglia_value_msg_u;
};
typedef struct Ganglia_value_msg Ganglia_value_msg;
#define GANGLIA_VALUE_MSG_ID(vmsg)                  ((vmsg)->id)
#define GANGLIA_VALUE_MSG_USHORT_GMETRIC(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gu_short))
#define GANGLIA_VALUE_MSG_SHORT_GMETRIC(vmsg)       (&((vmsg)->Ganglia_value_msg_u.gs_short))
#define GANGLIA_VALUE_MSG_INT_GMETRIC(vmsg)         (&((vmsg)->Ganglia_value_msg_u.gs_int))
#define GANGLIA_VALUE_MSG_UINT_GMETRIC(vmsg)        (&((vmsg)->Ganglia_value_msg_u.gu_int))
#define GANGLIA_VALUE_MSG_STRING_GMETRIC(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gstr))
#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC(vmsg)       (&((vmsg)->Ganglia_value_msg_u.gf))
#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC(vmsg)      (&((vmsg)->Ganglia_value_msg_u.gd))

#define GANGLIA_VALUE_MSG_FORMAT_ID_SET(vmsg, msg_id)       do{GANGLIA_VALUE_MSG_ID(vmsg) = (msg_id);}while(0)

/*USHORT*/
#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_USHORT_METRIC_ID(GANGLIA_VALUE_MSG_USHORT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_USHORT_FMT(GANGLIA_VALUE_MSG_USHORT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_USHORT_VAL(GANGLIA_VALUE_MSG_USHORT_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_USHORT_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_USHORT_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_USHORT_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_USHORT_GMETRIC_VAL(vmsg) = (val);}while(0)

/*SHORT*/
#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_SHORT_METRIC_ID(GANGLIA_VALUE_MSG_SHORT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_SHORT_FMT(GANGLIA_VALUE_MSG_SHORT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_SHORT_VAL(GANGLIA_VALUE_MSG_SHORT_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_SHORT_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_SHORT_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_SHORT_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_SHORT_GMETRIC_VAL(vmsg) = (val);}while(0)

/*INT*/
#define GANGLIA_VALUE_MSG_INT_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_INT_METRIC_ID(GANGLIA_VALUE_MSG_INT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_INT_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_INT_FMT(GANGLIA_VALUE_MSG_INT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_INT_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_INT_VAL(GANGLIA_VALUE_MSG_INT_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_INT_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_INT_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_INT_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_INT_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_INT_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_INT_GMETRIC_VAL(vmsg) = (val);}while(0)

/*UINT*/
#define GANGLIA_VALUE_MSG_UINT_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_UINT_METRIC_ID(GANGLIA_VALUE_MSG_UINT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_UINT_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_UINT_FMT(GANGLIA_VALUE_MSG_UINT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_UINT_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_UINT_VAL(GANGLIA_VALUE_MSG_UINT_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_UINT_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_UINT_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_UINT_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_UINT_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_UINT_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_UINT_GMETRIC_VAL(vmsg) = (val);}while(0)

/*STRING*/
#define GANGLIA_VALUE_MSG_STRING_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_STRING_METRIC_ID(GANGLIA_VALUE_MSG_STRING_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_STRING_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_STRING_FMT(GANGLIA_VALUE_MSG_STRING_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_STRING_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_STRING_VAL(GANGLIA_VALUE_MSG_STRING_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_STRING_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_STRING_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_STRING_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_STRING_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_STRING_GMETRIC_VAL_SET(vmsg, val) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_STRING_GMETRIC_VAL(vmsg), (val))

/*FLOAT*/
#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_FLOAT_METRIC_ID(GANGLIA_VALUE_MSG_FLOAT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_FLOAT_FMT(GANGLIA_VALUE_MSG_FLOAT_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_FLOAT_VAL(GANGLIA_VALUE_MSG_FLOAT_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_FLOAT_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_FLOAT_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_FLOAT_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_FLOAT_GMETRIC_VAL(vmsg) = (val);}while(0)


/*DOUBLE*/
#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_ID(vmsg)   (GANGLIA_GMETRIC_DOUBLE_METRIC_ID(GANGLIA_VALUE_MSG_DOUBLE_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_FMT(vmsg)  (GANGLIA_GMETRIC_DOUBLE_FMT(GANGLIA_VALUE_MSG_DOUBLE_GMETRIC(vmsg)))
#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_VAL(vmsg)  (GANGLIA_GMETRIC_DOUBLE_VAL(GANGLIA_VALUE_MSG_DOUBLE_GMETRIC(vmsg)))

#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_ID_SET(vmsg, host, name, spoof)  \
    GANGLIA_METRIC_ID_SET(GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_ID(vmsg), host, name, spoof)

#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_FMT_SET(vmsg, fmt) \
    GANGLIA_STRDUP(GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_FMT(vmsg), (fmt))

#define GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_VAL_SET(vmsg, val) \
    do{GANGLIA_VALUE_MSG_DOUBLE_GMETRIC_VAL(vmsg) = (val);}while(0)

struct Ganglia_25metric {
    int key;
    char *name;
    int tmax;
    Ganglia_value_types type;
    char *units;
    char *slope;
    char *fmt;
    int msg_size;
    char *desc;
    int *metadata;
};
typedef struct Ganglia_25metric Ganglia_25metric;
 #define GANGLIA_MAX_MESSAGE_LEN (1500 - 28 - 8)
#define modular_metric 4098

/* the xdr functions */

bool_t xdr_Ganglia_value_types (XDR *, Ganglia_value_types*);
bool_t xdr_Ganglia_extra_data (XDR *, Ganglia_extra_data*);
bool_t xdr_Ganglia_metadata_message (XDR *, Ganglia_metadata_message*);
bool_t xdr_Ganglia_metric_id (XDR *, Ganglia_metric_id*);
bool_t xdr_Ganglia_metadatadef (XDR *, Ganglia_metadatadef*);
bool_t xdr_Ganglia_metadatareq (XDR *, Ganglia_metadatareq*);
bool_t xdr_Ganglia_gmetric_ushort (XDR *, Ganglia_gmetric_ushort*);
bool_t xdr_Ganglia_gmetric_short (XDR *, Ganglia_gmetric_short*);
bool_t xdr_Ganglia_gmetric_int (XDR *, Ganglia_gmetric_int*);
bool_t xdr_Ganglia_gmetric_uint (XDR *, Ganglia_gmetric_uint*);
bool_t xdr_Ganglia_gmetric_string (XDR *, Ganglia_gmetric_string*);
bool_t xdr_Ganglia_gmetric_float (XDR *, Ganglia_gmetric_float*);
bool_t xdr_Ganglia_gmetric_double (XDR *, Ganglia_gmetric_double*);
bool_t xdr_Ganglia_msg_formats (XDR *, Ganglia_msg_formats*);
bool_t xdr_Ganglia_metadata_msg (XDR *, Ganglia_metadata_msg*);
bool_t xdr_Ganglia_value_msg (XDR *, Ganglia_value_msg*);
bool_t xdr_Ganglia_25metric (XDR *, Ganglia_25metric*);


#endif/* _GM_PROTOCOL_H_RPCGEN */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

