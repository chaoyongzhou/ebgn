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
#include <time.h>

#include "bgnctrl.h"
#include "type.h"
#include "moduleconst.h"

#include "mm.h"

#include "log.h"

#include "bgnz.h"
#include "ebgnz.h"

#include "poly.h"

#include "debug.h"

#include "vector.h"
#include "matrix.h"

#include "vectorr.h"
#include "matrixr.h"

#include "clist.h"
#include "cvector.h"

#include "cstring.h"

#include "mod.h"
#include "task.h"

#include "super.h"

#include "cmpic.inc"
#include "cmpie.h"

#include "cdbgcode.h"
#include "cbytecode.h"
#include "ccode.h"

#include "tcnode.h"
#include "vmm.h"

#include "kbuff.h"
#include "cfile.h"
#include "cdir.h"
#include "cdfs.h"
#include "cdfsnp.h"
#include "cdfsdn.h"
#include "crfs.h"
#include "crfsnp.h"
#include "crfsdn.h"
#include "chfs.h"
#include "chfsnp.h"

#include "csocket.h"
#include "csys.h"
#include "cmon.h"
#include "cload.h"
#include "cfuse.h"
#include "cbytes.h"
#include "csession.h"
#include "cscore.h"

#include "crfsbk.inc"
#include "crfsconhash.h"
#include "crfsdt.h"

#include "cbuffer.h"


//#define CMPI_DBG(x) sys_log x
#define CMPI_DBG(x) do{}while(0)

#if 0
#define PRINT_BUFF(info, buff, len) do{\
    UINT32 pos;\
    dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "%s: ", info);\
    for(pos = 0; pos < len; pos ++)\
    {\
        sys_print(LOGSTDOUT, "%x,", ((UINT8 *)buff)[ pos ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define PRINT_BUFF(info, buff, len) do{}while(0)
#endif

UINT32 cmpi_encode_uint8(const UINT32 comm, const UINT8 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack(&num, 1, CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint8_ptr(const UINT32 comm, const UINT8 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack(num, 1, CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint8_size(const UINT32 comm, const UINT8 num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_UCHAR, size,  comm);
    return (0);
}

UINT32 cmpi_encode_uint8_ptr_size(const UINT32 comm, const UINT8 *num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_UCHAR, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint8(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *num)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, num, 1, CMPI_UCHAR, comm);
    return (0);
}

UINT32 cmpi_encode_uint16(const UINT32 comm, const UINT16 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&num, 1, CMPI_USHORT, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint16_ptr(const UINT32 comm, const UINT16 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)num, 1, CMPI_USHORT, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint16_size(const UINT32 comm, const UINT16 num, UINT32 *size)
{
    //cmpi_pack_size(1, CMPI_USHORT, size,  comm);
    cmpi_pack_size(1, CMPI_USHORT, size,  comm);
    return (0);
}

UINT32 cmpi_encode_uint16_ptr_size(const UINT32 comm, const UINT16 *num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_USHORT, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint16(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT16 *num)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, 1, CMPI_USHORT, comm);
    return (0);
}

UINT32 cmpi_encode_uint32(const UINT32 comm, const UINT32 num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&num, 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint32_ptr(const UINT32 comm, const UINT32 *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)num, 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint32_size(const UINT32 comm, const UINT32 num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size,  comm);
    return (0);
}
UINT32 cmpi_encode_uint32_ptr_size(const UINT32 comm, const UINT32 *num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint32(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT32 *num)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, 1, CMPI_ULONG, comm);
    return (0);
}

UINT32 cmpi_encode_uint32_t(const UINT32 comm, const uint32_t num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&num, 1, CMPI_U32, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint32_t_ptr(const UINT32 comm, const uint32_t *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)num, 1, CMPI_U32, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint32_t_size(const UINT32 comm, const uint32_t num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_U32, size,  comm);
    return (0);
}
UINT32 cmpi_encode_uint32_t_ptr_size(const UINT32 comm, const uint32_t *num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_U32, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint32_t(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, uint32_t *num)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, 1, CMPI_U32, comm);
    return (0);
}

UINT32 cmpi_encode_uint64(const UINT32 comm, const uint64_t num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&num, 1, CMPI_ULONGLONG, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint64_ptr(const UINT32 comm, const uint64_t *num, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)num, 1, CMPI_ULONGLONG, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint64_size(const UINT32 comm, const UINT32 num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONGLONG, size,  comm);
    return (0);
}
UINT32 cmpi_encode_uint64_ptr_size(const UINT32 comm, const uint64_t *num, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONGLONG, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint64(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, uint64_t *num)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, 1, CMPI_ULONGLONG, comm);
    return (0);
}

UINT32 cmpi_encode_uint8_array(const UINT32 comm, const UINT8 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&len, 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack(num, len, CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint8_array_size(const UINT32 comm, const UINT8 *num, const UINT32 len, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size,  comm);
    cmpi_pack_size(len, CMPI_UCHAR, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint8_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *num, UINT32 *len)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)len, 1,    CMPI_ULONG, comm);
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, *len, CMPI_UCHAR, comm);
    return (0);
}

UINT32 cmpi_encode_uint16_array(const UINT32 comm, const UINT16 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&len, 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack((UINT8 *)num, len, CMPI_USHORT, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint16_array_size(const UINT32 comm, const UINT16 *num, const UINT32 len, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size, comm);
    cmpi_pack_size(len, CMPI_USHORT, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint16_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT16 *num, UINT32 *len)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)len, 1,    CMPI_ULONG, comm);
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, *len, CMPI_USHORT, comm);
    return (0);
}

UINT32 cmpi_encode_uint32_array(const UINT32 comm, const UINT32 *num, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&len, 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack((UINT8 *)num, len, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_uint32_array_size(const UINT32 comm, const UINT32 *num, const UINT32 len, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size,  comm);
    cmpi_pack_size(len, CMPI_ULONG, size,  comm);
    return (0);
}

UINT32 cmpi_decode_uint32_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT32 *num, UINT32 *len)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)len, 1,    CMPI_ULONG, comm);
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)num, *len, CMPI_ULONG, comm);
    return (0);
}

UINT32 cmpi_encode_real_array(const UINT32 comm, const REAL *real, const UINT32 len, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)&len, 1,   CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack((UINT8 *)real, len, CMPI_REAL, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_real_array_size(const UINT32 comm, const REAL *real, const UINT32 len, UINT32 *size)
{
    cmpi_pack_size(1,   CMPI_ULONG, size,  comm);
    cmpi_pack_size(len, CMPI_REAL, size,  comm);
    return (0);
}

UINT32 cmpi_decode_real_array(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, REAL *real, UINT32 *len)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)len, 1,     CMPI_ULONG, comm);
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)real, *len, CMPI_REAL, comm);
    return (0);
}

UINT32 cmpi_encode_real(const UINT32 comm, const REAL *real, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)real, 1, CMPI_REAL, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_real_size(const UINT32 comm, const REAL *real, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_REAL, size, comm);
    return (0);
}

UINT32 cmpi_decode_real(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, REAL *real)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)real, 1, CMPI_REAL, comm);
    return (0);
}

UINT32 cmpi_encode_macaddr(const UINT32 comm, const UINT8 *macaddr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_pack((UINT8 *)macaddr, 6, CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_macaddr_size(const UINT32 comm, const UINT8 *macaddr, UINT32 *size)
{
    cmpi_pack_size(6, CMPI_UCHAR, size, comm);
    return (0);
}

UINT32 cmpi_decode_macaddr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, UINT8 *macaddr)
{
    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)macaddr, 6, CMPI_UCHAR, comm);
    return (0);
}

UINT32 cmpi_encode_bgn(const UINT32 comm, const BIGINT *bgn, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == bgn )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_bgn: bgn is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_bgn: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_bgn: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32_array(comm, bgn->data, bgn->len, out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_bgn_size(const UINT32 comm, const BIGINT *bgn, UINT32 *size)
{
    cmpi_encode_uint32_array_size(comm, bgn->data, bgn->len, size);

    return (0);
}

UINT32 cmpi_decode_bgn(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, BIGINT *bgn)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_bgn: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_bgn: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == bgn )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_bgn: bgn is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32_array(comm, in_buff, in_buff_max_len, position, bgn->data, &(bgn->len));

    return (0);
}

UINT32 cmpi_encode_ebgn(const UINT32 comm, const EBIGINT *ebgn, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    EBGN_ITEM *item;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ebgn: ebgn is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ebgn: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ebgn: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, (EBGN_SGN(ebgn)), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, (EBGN_LEN(ebgn)), out_buff, out_buff_max_len, position);

    item = EBGN_FIRST_ITEM(ebgn);
    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        cmpi_encode_bgn(comm, EBGN_ITEM_BGN(item), out_buff, out_buff_max_len, position);
        item = EBGN_ITEM_NEXT(item);
    }
    return (0);
}

UINT32 cmpi_encode_ebgn_size(const UINT32 comm, const EBIGINT *ebgn, UINT32 *size)
{
    EBGN_ITEM *item;

    cmpi_encode_uint32_size(comm, (EBGN_SGN(ebgn)), size);
    cmpi_encode_uint32_size(comm, (EBGN_LEN(ebgn)), size);

    item = EBGN_FIRST_ITEM(ebgn);
    while ( item != EBGN_NULL_ITEM(ebgn) )
    {
        cmpi_encode_bgn_size(comm, EBGN_ITEM_BGN(item), size);
        item = EBGN_ITEM_NEXT(item);
    }
    return (0);
}

UINT32 cmpi_decode_ebgn(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EBIGINT *ebgn)
{
    UINT32 idx;
    EBGN_ITEM *item;
    BIGINT *bgn;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ebgn: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ebgn: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ebgn )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ebgn: ebgn is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /* initialize ebgn */
    EBGN_INIT( ebgn );

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(EBGN_SGN(ebgn)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(EBGN_LEN(ebgn)));

    for(idx = 0; idx < EBGN_LEN(ebgn); idx ++)
    {
        alloc_static_mem(MD_TBD, 0, MM_EBGN_ITEM, &item, LOC_CMPIE_0001);
        alloc_static_mem(MD_TBD, 0, MM_BIGINT, &bgn, LOC_CMPIE_0002);

        cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, bgn);
        EBGN_ITEM_BGN(item) = bgn;

        EBGN_ADD_ITEM_TAIL(ebgn, item);
    }

    return (0);
}

UINT32 cmpi_encode_ec_curve_point(const UINT32 comm, const EC_CURVE_POINT *ec_curve_point, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ec_curve_point )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_point: ec_curve_point is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_point: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_point: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ec_curve_point->x), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ec_curve_point->y), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ec_curve_point_size(const UINT32 comm, const EC_CURVE_POINT *ec_curve_point, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ec_curve_point->x), size);
    cmpi_encode_bgn_size(comm, &(ec_curve_point->y), size);
    return (0);
}


UINT32 cmpi_decode_ec_curve_point(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EC_CURVE_POINT *ec_curve_point)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_point: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_point: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ec_curve_point )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_point: ec_curve_point is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ec_curve_point->x));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ec_curve_point->y));

    return (0);
}

UINT32 cmpi_encode_ec_curve_aff_point(const UINT32 comm, const EC_CURVE_AFF_POINT *ec_curve_aff_point, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ec_curve_aff_point )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_aff_point: ec_curve_aff_point is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_aff_point: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ec_curve_aff_point: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ec_curve_aff_point->x), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ec_curve_aff_point->y), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ec_curve_aff_point->z), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ec_curve_aff_point_size(const UINT32 comm, const EC_CURVE_AFF_POINT *ec_curve_aff_point, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ec_curve_aff_point->x), size);
    cmpi_encode_bgn_size(comm, &(ec_curve_aff_point->y), size);
    cmpi_encode_bgn_size(comm, &(ec_curve_aff_point->z), size);
    return (0);
}


UINT32 cmpi_decode_ec_curve_aff_point(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, EC_CURVE_AFF_POINT *ec_curve_aff_point)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_aff_point: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_aff_point: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ec_curve_aff_point )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ec_curve_aff_point: ec_curve_aff_point is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ec_curve_aff_point->x));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ec_curve_aff_point->y));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ec_curve_aff_point->z));

    return (0);
}

UINT32 cmpi_encode_ecf2n_curve(const UINT32 comm, const ECF2N_CURVE *ecf2n_curve, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ecf2n_curve )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecf2n_curve: ecf2n_curve is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecf2n_curve: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecf2n_curve: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ecf2n_curve->a), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ecf2n_curve->b), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ecf2n_curve_size(const UINT32 comm, const ECF2N_CURVE *ecf2n_curve, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ecf2n_curve->a), size);
    cmpi_encode_bgn_size(comm, &(ecf2n_curve->b), size);
    return (0);
}

UINT32 cmpi_decode_ecf2n_curve(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECF2N_CURVE *ecf2n_curve)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecf2n_curve: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecf2n_curve: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ecf2n_curve )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecf2n_curve: ecf2n_curve is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecf2n_curve->a));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecf2n_curve->b));

    return (0);
}

UINT32 cmpi_encode_ecfp_curve(const UINT32 comm, const ECFP_CURVE *ecfp_curve, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ecfp_curve )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecfp_curve: ecfp_curve is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecfp_curve: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecfp_curve: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ecfp_curve->a), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ecfp_curve->b), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ecfp_curve_size(const UINT32 comm, const ECFP_CURVE *ecfp_curve, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ecfp_curve->a), size);
    cmpi_encode_bgn_size(comm, &(ecfp_curve->b), size);

    return (0);
}

UINT32 cmpi_decode_ecfp_curve(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECFP_CURVE *ecfp_curve)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecfp_curve: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecfp_curve: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ecfp_curve )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecfp_curve: ecfp_curve is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecfp_curve->a));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecfp_curve->b));

    return (0);
}

UINT32 cmpi_encode_ecc_keypair(const UINT32 comm, const ECC_KEYPAIR *ecc_keypair, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ecc_keypair )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_keypair: ecc_keypair is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_keypair: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_keypair: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ecc_keypair->private_key), out_buff, out_buff_max_len, position);
    cmpi_encode_ec_curve_point(comm, &(ecc_keypair->public_key), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ecc_keypair_size(const UINT32 comm, const ECC_KEYPAIR *ecc_keypair, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ecc_keypair->private_key), size);
    cmpi_encode_ec_curve_point_size(comm, &(ecc_keypair->public_key), size);
    return (0);
}


UINT32 cmpi_decode_ecc_keypair(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECC_KEYPAIR *ecc_keypair)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_keypair: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_keypair: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ecc_keypair )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_keypair: ecc_keypair is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecc_keypair->private_key));
    cmpi_decode_ec_curve_point(comm, in_buff, in_buff_max_len, position, &(ecc_keypair->public_key));

    return (0);
}

UINT32 cmpi_encode_ecc_signature(const UINT32 comm, const ECC_SIGNATURE *ecc_signature, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ecc_signature )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_signature: ecc_signature is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_signature: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ecc_signature: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }

#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_bgn(comm, &(ecc_signature->r), out_buff, out_buff_max_len, position);
    cmpi_encode_bgn(comm, &(ecc_signature->s), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_ecc_signature_size(const UINT32 comm, const ECC_SIGNATURE *ecc_signature, UINT32 *size)
{
    cmpi_encode_bgn_size(comm, &(ecc_signature->r), size);
    cmpi_encode_bgn_size(comm, &(ecc_signature->s), size);
    return (0);
}

UINT32 cmpi_decode_ecc_signature(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, ECC_SIGNATURE *ecc_signature)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_signature: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_signature: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ecc_signature )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ecc_signature: ecc_signature is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecc_signature->r));
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, &(ecc_signature->s));

    return (0);
}

UINT32 cmpi_encode_degree(const UINT32 comm, const DEGREE *degree, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    cmpi_encode_bgn(comm, degree, out_buff, out_buff_max_len, position);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    cmpi_encode_uint32_ptr(comm, degree, out_buff, out_buff_max_len, position);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/
    return (0);
}

UINT32 cmpi_encode_degree_size(const UINT32 comm, const DEGREE *degree, UINT32 *size)
{
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    cmpi_encode_bgn_size(comm, degree, size);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    cmpi_encode_uint32_ptr_size(comm, degree, size);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/

    return (0);
}

UINT32 cmpi_decode_degree(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, DEGREE *degree)
{
#if (SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )
    cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, degree);
#endif/*(SWITCH_ON == POLY_DEG_IS_BGN_SWITCH )*/

#if (SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, degree);
#endif/*(SWITCH_ON == POLY_DEG_IS_UINT32_SWITCH )*/
    return (0);
}

UINT32 cmpi_encode_poly_item(const UINT32 comm, const POLY_ITEM *poly_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == poly_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly_item: poly_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly_item: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_degree(comm, POLY_ITEM_DEG(poly_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, (POLY_ITEM_BGN_COE_FLAG(poly_item)), out_buff, out_buff_max_len, position);
    if(EC_TRUE == POLY_ITEM_BGN_COE_FLAG(poly_item))
    {
        cmpi_encode_bgn(comm, POLY_ITEM_BGN_COE(poly_item), out_buff, out_buff_max_len, position);
    }
    else
    {
        cmpi_encode_poly(comm, POLY_ITEM_POLY_COE(poly_item), out_buff, out_buff_max_len, position);
    }

    return ( 0 );
}

UINT32 cmpi_encode_poly_item_size(const UINT32 comm, const POLY_ITEM *poly_item, UINT32 *size)
{
    cmpi_encode_degree_size(comm, POLY_ITEM_DEG(poly_item), size);
    cmpi_encode_uint32_size(comm, (POLY_ITEM_BGN_COE_FLAG(poly_item)), size);
    if(EC_TRUE == POLY_ITEM_BGN_COE_FLAG(poly_item))
    {
        cmpi_encode_bgn_size(comm, POLY_ITEM_BGN_COE(poly_item), size);
    }
    else
    {
        cmpi_encode_poly_size(comm, POLY_ITEM_POLY_COE(poly_item), size);
    }
    return ( 0 );
}

UINT32 cmpi_decode_poly_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, POLY_ITEM *poly_item)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly_item: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == poly_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly_item: poly_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_degree(comm, in_buff, in_buff_max_len, position, POLY_ITEM_DEG(poly_item));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(POLY_ITEM_BGN_COE_FLAG(poly_item)));

    if(EC_TRUE == POLY_ITEM_BGN_COE_FLAG(poly_item))
    {
        cmpi_decode_bgn(comm, in_buff, in_buff_max_len, position, POLY_ITEM_BGN_COE(poly_item));
    }
    else
    {
        cmpi_decode_poly(comm, in_buff, in_buff_max_len, position, POLY_ITEM_POLY_COE(poly_item));
    }

    return ( 0 );
}

UINT32 cmpi_encode_poly(const UINT32 comm, const POLY *poly, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    const POLY_ITEM *item;
    UINT32 flag;
    UINT32 item_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly: poly is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_poly: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    flag = POLY_IS_EMPTY(poly);
    cmpi_encode_uint32(comm, flag, out_buff, out_buff_max_len, position);

    if(EC_TRUE == flag)
    {
        return (0);
    }

    item_num = 0;
    item = POLY_FIRST_ITEM(poly);
    while( item != POLY_NULL_ITEM(poly) )
    {
        item_num ++;
        item = POLY_ITEM_NEXT(item);
    }
    cmpi_encode_uint32(comm, item_num, out_buff, out_buff_max_len, position);

    item = POLY_FIRST_ITEM(poly);
    while( item != POLY_NULL_ITEM(poly) )
    {
        cmpi_encode_poly_item(comm, item, out_buff, out_buff_max_len, position);
        item = POLY_ITEM_NEXT(item);
    }

    return ( 0 );
}

UINT32 cmpi_encode_poly_size(const UINT32 comm, const POLY *poly, UINT32 *size)
{
    const POLY_ITEM *item;
    UINT32 flag;
    UINT32 item_num;

    flag = POLY_IS_EMPTY(poly);
    cmpi_encode_uint32_size(comm, flag, size);

    if(EC_TRUE == flag)
    {
        return (0);
    }

    item_num = 0;

    cmpi_encode_uint32_size(comm, item_num, size);

    item = POLY_FIRST_ITEM(poly);
    while( item != POLY_NULL_ITEM(poly) )
    {
        cmpi_encode_poly_item_size(comm, item, size);
        item = POLY_ITEM_NEXT(item);
    }
    return ( 0 );
}

UINT32 cmpi_decode_poly(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, POLY *poly)
{
    POLY_ITEM *item;
    UINT32 flag;
    UINT32 item_num;
    UINT32 item_idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == poly )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_poly: poly is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    POLY_INIT(poly);
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &flag);
    if(EC_TRUE == flag)
    {
        return (0);
    }

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &item_num);

    for(item_idx = 0; item_idx < item_num; item_idx ++)
    {
        alloc_static_mem(MD_TBD, 0, MM_POLY_ITEM, &item, LOC_CMPIE_0003);
        cmpi_decode_poly_item(comm, in_buff, in_buff_max_len, position, item);
        POLY_ADD_ITEM_TAIL(poly, item);
    }

    return ( 0 );
}

UINT32 cmpi_encode_vectorr_block(const UINT32 comm, const VECTOR_BLOCK *vectorr_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 num;
    UINT32 sub_idx;

    REAL *data_addr;
    REAL zero;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == vectorr_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr_block: vectorr_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr_block: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*initialize*/
    REAL_SETZERO(0, zero);

    num = VECTOR_BLOCK_GET_NUM(vectorr_block);
    cmpi_encode_uint32(comm, num, out_buff, out_buff_max_len,  position);

    for( sub_idx = 0; sub_idx < num; sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vectorr_block, sub_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }

        cmpi_encode_real(comm, data_addr, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_vectorr_block_size(const UINT32 comm, const VECTOR_BLOCK *vectorr_block, UINT32 *size)
{
    UINT32 num;
    UINT32 sub_idx;

    REAL *data_addr;
    REAL zero;

    /*initialize*/
    REAL_SETZERO(0, zero);

    num = VECTOR_BLOCK_GET_NUM(vectorr_block);
    cmpi_encode_uint32_size(comm, num, size);

    for( sub_idx = 0; sub_idx < num; sub_idx ++)
    {
        data_addr = VECTORR_BLOCK_GET_DATA_ADDR(vectorr_block, sub_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }

        cmpi_encode_real_size(comm, data_addr, size);
    }
    return (0);
}

UINT32 cmpi_decode_vectorr_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VECTOR_BLOCK *vectorr_block)
{
    UINT32 num;
    UINT32 sub_idx;
    REAL *data_addr;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr_block: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == vectorr_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr_block: vectorr_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32( comm, in_buff, in_buff_max_len, position, &num);

    VECTOR_BLOCK_SET_NUM(vectorr_block, num);

    for( sub_idx = 0; sub_idx < num; sub_idx ++ )
    {
        alloc_static_mem(MD_TBD, 0, MM_REAL, &data_addr, LOC_CMPIE_0004);
        cmpi_decode_real(comm, in_buff, in_buff_max_len, position, data_addr);
        VECTOR_BLOCK_SET_DATA_ADDR(vectorr_block, sub_idx, data_addr);
    }

    return (0);
}

UINT32 cmpi_encode_vectorr(const UINT32 comm, const VECTOR *vectorr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    VECTOR_BLOCK *vector_block;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == vectorr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr: vectorr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vectorr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, (VECTOR_GET_ROTATED_FLAG(vectorr)), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, (VECTOR_GET_NUM(vectorr)), out_buff, out_buff_max_len, position);

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vectorr)
    {
        cmpi_encode_vectorr_block(comm, vector_block, out_buff, out_buff_max_len, position);
    }

   return (0);
}

UINT32 cmpi_encode_vectorr_size(const UINT32 comm, const VECTOR *vectorr, UINT32 *size)
{
    VECTOR_BLOCK *vector_block;

    cmpi_encode_uint32_size(comm, (VECTOR_GET_ROTATED_FLAG(vectorr)), size);
    cmpi_encode_uint32_size(comm, (VECTOR_GET_NUM(vectorr)), size);

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vectorr)
    {
        cmpi_encode_vectorr_block_size(comm, vector_block, size);
    }
    return (0);
}


UINT32 cmpi_decode_vectorr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VECTOR *vectorr)
{
    VECTOR_BLOCK *vector_block;

    UINT32 num;
    UINT32 rotated_flag;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == vectorr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vectorr: vectorr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &rotated_flag);
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &num);

    if(0 != VECTOR_GET_NUM(vectorr))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDERR, "fatal error:cmpi_decode_vectorr: vectorr %lx width is %ld, but it must be zero!\n", vectorr, VECTOR_GET_NUM(vectorr));
        dbg_exit(MD_TBD, 0);
    }

    vector_r_new_vector_skeleton(0, num, vectorr); /*trick: alloc VECTORR on the #0 module*/

    if(1 == rotated_flag)
    {
        vector_r_rotate(0, vectorr); /*trick: operate VECTORR on the #0 module*/
    }

    VECTOR_BLOCK_LOOP_NEXT(vector_block, vectorr)
    {
        cmpi_decode_vectorr_block(comm, in_buff, in_buff_max_len, position, vector_block);
    }

    return (0);
}

UINT32 cmpi_encode_matrixr_block(const UINT32 comm, const MATRIX_BLOCK *matrixr_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    REAL *data_addr;
    REAL zero;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == matrixr_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr_block: matrixr_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr_block: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
#if 0
    UINT32 save_position = *position;/*for debug only*/
#endif
    /*initialize*/
    REAL_SETZERO(0, zero);

    rotated_flag = MATRIX_BLOCK_GET_ROTATED_FLAG(matrixr_block);
    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrixr_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrixr_block);

    cmpi_encode_uint32(comm, rotated_flag, out_buff, out_buff_max_len,  position);
    cmpi_encode_uint32(comm, row_num, out_buff, out_buff_max_len,  position);
    cmpi_encode_uint32(comm, col_num, out_buff, out_buff_max_len,  position);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(row_idx, row_num, col_idx, col_num)
    {
        data_addr = (REAL *)MATRIX_BLOCK_GET_DATA_ADDR(matrixr_block, row_idx, col_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }
        cmpi_encode_real(comm, data_addr, out_buff, out_buff_max_len, position);
    }
#if 0
    print_cmpi_in_buff("cmpi_encode_matrixr_block: \n", out_buff + save_position, *position - save_position);
#endif
    return (0);
}

UINT32 cmpi_encode_matrixr_block_size(const UINT32 comm, const MATRIX_BLOCK *matrixr_block, UINT32 *size)
{
    REAL *data_addr;
    REAL zero;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;

    /*initialize*/
    REAL_SETZERO(0, zero);

    rotated_flag = MATRIX_BLOCK_GET_ROTATED_FLAG(matrixr_block);
    row_num = MATRIX_BLOCK_GET_ROW_NUM(matrixr_block);
    col_num = MATRIX_BLOCK_GET_COL_NUM(matrixr_block);

    cmpi_encode_uint32_size(comm, rotated_flag, size);
    cmpi_encode_uint32_size(comm, row_num, size);
    cmpi_encode_uint32_size(comm, col_num, size);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(row_idx, row_num, col_idx, col_num)
    {
        data_addr = (REAL *)MATRIX_BLOCK_GET_DATA_ADDR(matrixr_block, row_idx, col_idx);
        if(NULL_PTR == data_addr)
        {
            data_addr = &zero;
        }
        cmpi_encode_real_size(comm, data_addr, size);
    }
    return (0);
}

UINT32 cmpi_decode_matrixr_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MATRIX_BLOCK *matrixr_block)
{
    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

    UINT32 row_idx;
    UINT32 col_idx;

    REAL * data_addr;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr_block: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == matrixr_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr_block: matrixr_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
#if 0
    UINT32 save_position = *position;/*for debug only*/
#endif
    cmpi_decode_uint32( comm, in_buff, in_buff_max_len, position, &rotated_flag);
    cmpi_decode_uint32( comm, in_buff, in_buff_max_len, position, &row_num);
    cmpi_decode_uint32( comm, in_buff, in_buff_max_len, position, &col_num);

    MATRIX_BLOCK_SET_ROTATED_FLAG(matrixr_block, rotated_flag);
    MATRIX_BLOCK_SET_ROW_NUM(matrixr_block, row_num);
    MATRIX_BLOCK_SET_COL_NUM(matrixr_block, col_num);

    MATRIX_BLOCK_DATA_AREA_ROW_COL_LOOP_NEXT(row_idx, row_num, col_idx, col_num)
    {
#if (MATRIXR_PATCH_SWITCH == SWITCH_OFF)
        alloc_static_mem(MD_TBD, 0, MM_REAL, &data_addr, LOC_CMPIE_0005);
        cmpi_decode_real( comm, in_buff, in_buff_max_len, position, data_addr);
        MATRIX_BLOCK_SET_DATA_ADDR(matrixr_block, row_idx, col_idx, data_addr);
#endif/*(MATRIXR_PATCH_SWITCH == SWITCH_OFF)*/

#if (MATRIXR_PATCH_01_SWITCH == SWITCH_ON)
        data_addr = (REAL *)MATRIX_BLOCK_GET_DATA_ADDR(matrixr_block, row_idx, col_idx);
        cmpi_decode_real( comm, in_buff, in_buff_max_len, position, data_addr);
#endif/*(MATRIXR_PATCH_01_SWITCH == SWITCH_ON)*/

#if (MATRIXR_PATCH_02_SWITCH == SWITCH_ON)
        data_addr = (REAL *)MATRIX_BLOCK_GET_DATA_ADDR(matrixr_block, row_idx, col_idx);
        cmpi_decode_real( comm, in_buff, in_buff_max_len, position, data_addr);
#endif/*(MATRIXR_PATCH_02_SWITCH == SWITCH_ON)*/
    }
#if 0
    print_cmpi_in_buff("cmpi_decode_matrixr_block: \n", in_buff + save_position, *position - save_position);
    dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_matrixr_block: ==========================================================\n");
    matrix_r_print_matrix_block_data_info(0, matrixr_block);
#endif
    return (0);
}

UINT32 cmpi_encode_matrixr(const UINT32 comm, const MATRIX *matrixr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;

    MATRIX_BLOCK  *matrix_row_block;

    UINT32 rotated_flag;
    UINT32 row_num;
    UINT32 col_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == matrixr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr: matrixr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_matrixr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    rotated_flag = MATRIX_GET_ROTATED_FLAG(matrixr);
    row_num      = MATRIX_GET_ROW_NUM(matrixr);
    col_num      = MATRIX_GET_COL_NUM(matrixr);

    /*rotated flag and row num and col num of matrix*/
    cmpi_encode_uint32(comm, rotated_flag, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, row_num,      out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, col_num,      out_buff, out_buff_max_len, position);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrixr, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
        cmpi_encode_matrixr_block(comm, matrix_row_block, out_buff, out_buff_max_len, position);
    }

   return (0);
}

UINT32 cmpi_encode_matrixr_size(const UINT32 comm, const MATRIX *matrixr, UINT32 *size)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;
    MATRIX_BLOCK  *matrix_row_block;

    UINT32 rotated_flag;
    UINT32 row_num;
    UINT32 col_num;

    rotated_flag = MATRIX_GET_ROTATED_FLAG(matrixr);
    row_num      = MATRIX_GET_ROW_NUM(matrixr);
    col_num      = MATRIX_GET_COL_NUM(matrixr);

    /*rotated flag and row num and col num of matrix*/
    cmpi_encode_uint32_size(comm, rotated_flag, size);
    cmpi_encode_uint32_size(comm, row_num,      size);
    cmpi_encode_uint32_size(comm, col_num,      size);

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrixr, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
        cmpi_encode_matrixr_block_size(comm, matrix_row_block, size);
    }

    return (0);
}

UINT32 cmpi_decode_matrixr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MATRIX *matrixr)
{
    UINT32 block_row_idx;
    UINT32 block_col_idx;
    MATRIX_BLOCK  *matrix_row_block;

    UINT32 row_num;
    UINT32 col_num;
    UINT32 rotated_flag;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == matrixr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_matrixr: matrixr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &rotated_flag);
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &row_num);
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &col_num);

    if(0 != MATRIX_GET_ROW_NUM(matrixr)
    || 0 != MATRIX_GET_COL_NUM(matrixr))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDERR, "fatal error:cmpi_decode_matrixr: matrixr %lx is (%ld, %ld), but it must be (0, 0)!\n",
                         matrixr, MATRIX_GET_ROW_NUM(matrixr), MATRIX_GET_COL_NUM(matrixr));
        dbg_exit(MD_TBD, 0);
    }

    matrix_r_new_matrix_skeleton(0, row_num, col_num, matrixr); /*trick: alloc MATRIXR on the #0 module*/

    MATRIX_ROW_COL_BLOCKS_LOOP_NEXT(matrixr, block_row_idx, block_col_idx)
    {
        matrix_row_block = MATRIX_GET_BLOCK(matrixr, block_row_idx, block_col_idx);
        matrix_r_init_block(0, matrix_row_block);/*patch*/
        cmpi_decode_matrixr_block(comm, in_buff, in_buff_max_len, position, matrix_row_block);
    }

    return (0);
}

UINT32 cmpi_encode_mod_node(const UINT32 comm, const MOD_NODE *mod_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_node: mod_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, MOD_NODE_TCID(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_COMM(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_RANK(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_MODI(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_HOPS(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_LOAD(mod_node), out_buff, out_buff_max_len, position);

    cmpi_encode_cload_stat(comm, MOD_NODE_CLOAD_STAT(mod_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_mod_node_size(const UINT32 comm, const MOD_NODE *mod_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, MOD_NODE_TCID(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_COMM(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_RANK(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_MODI(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_HOPS(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_LOAD(mod_node), size);

    cmpi_encode_cload_stat_size(comm, MOD_NODE_CLOAD_STAT(mod_node), size);

    return (0);
}

UINT32 cmpi_decode_mod_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_NODE *mod_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == mod_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_node: mod_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_TCID(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_COMM(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_RANK(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_MODI(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_HOPS(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_LOAD(mod_node)));

    cmpi_decode_cload_stat(comm, in_buff, in_buff_max_len, position, MOD_NODE_CLOAD_STAT(mod_node));

    return (0);
}

UINT32 cmpi_encode_mod_mgr(const UINT32 comm, const MOD_MGR *mod_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 pos;
    MOD_NODE *mod_node;

    UINT32 remote_mod_node_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_mgr: mod_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_mgr: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mod_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

#if 0
    UINT32 save_position = *position;/*for debug only*/
#endif
    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    cmpi_encode_uint32(comm, (MOD_MGR_LDB_CHOICE(mod_mgr)), out_buff, out_buff_max_len, position);
    cmpi_encode_mod_node(comm, (MOD_MGR_LOCAL_MOD(mod_mgr)), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, (remote_mod_node_num), out_buff, out_buff_max_len, position);

    for(pos = 0; pos < remote_mod_node_num; pos ++)
    {
        mod_node = (MOD_NODE *)cvector_get(MOD_MGR_REMOTE_LIST(mod_mgr), pos);
        cmpi_encode_mod_node(comm, mod_node, out_buff, out_buff_max_len, position);
    }
#if 0
    print_cmpi_in_buff("cmpi_encode_mod_mgr: \n", out_buff + save_position, *position - save_position);
#endif
    return (0);
}

UINT32 cmpi_encode_mod_mgr_size(const UINT32 comm, const MOD_MGR *mod_mgr, UINT32 *size)
{
    UINT32 pos;
    MOD_NODE *mod_node;
    UINT32 remote_mod_node_num;
    CVECTOR *remote_mod_node_list;

    remote_mod_node_num = MOD_MGR_REMOTE_NUM(mod_mgr);
    cmpi_encode_uint32_size(comm, (MOD_MGR_LDB_CHOICE(mod_mgr)), size);
    cmpi_encode_mod_node_size(comm, (MOD_MGR_LOCAL_MOD(mod_mgr)), size);
    cmpi_encode_uint32_size(comm, (remote_mod_node_num), size);

    remote_mod_node_list = (CVECTOR *)MOD_MGR_REMOTE_LIST(mod_mgr);
    for(pos = 0; pos < remote_mod_node_num; pos ++)
    {
        mod_node = (MOD_NODE *)cvector_get(remote_mod_node_list, pos);
        cmpi_encode_mod_node_size(comm, mod_node, size);
    }

    return (0);
}

UINT32 cmpi_decode_mod_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_MGR *mod_mgr)
{
    CVECTOR *remote_mod_node_list;
    MOD_NODE *remote_mod_node;

    UINT32 remote_mod_node_num;
    UINT32 remote_mod_node_idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_mgr: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == mod_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mod_mgr: mod_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
#if 0
    UINT32 save_position = *position;/*for debug only*/
#endif
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_MGR_LDB_CHOICE(mod_mgr)));
    cmpi_decode_mod_node(comm, in_buff, in_buff_max_len, position, MOD_MGR_LOCAL_MOD(mod_mgr));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(remote_mod_node_num));

    remote_mod_node_list = MOD_MGR_REMOTE_LIST(mod_mgr);
    cvector_init(remote_mod_node_list, remote_mod_node_num, MM_MOD_NODE, CVECTOR_LOCK_ENABLE, LOC_CMPIE_0006);
    cvector_codec_set(remote_mod_node_list, MM_MOD_NODE);
    for(remote_mod_node_idx = 0; remote_mod_node_idx < remote_mod_node_num; remote_mod_node_idx ++)
    {
        alloc_static_mem(MD_TBD, 0, MM_MOD_NODE, &remote_mod_node, LOC_CMPIE_0007);
        cmpi_decode_mod_node(comm, in_buff, in_buff_max_len, position, remote_mod_node);
        cvector_push(remote_mod_node_list, remote_mod_node);
    }
#if 0
    print_cmpi_in_buff("cmpi_decode_mod_mgr: \n", in_buff + save_position, *position - save_position);
    dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_mod_mgr: ==========================================================\n");
    mod_mgr_print(LOGSTDOUT, mod_mgr);
#endif
    return (0);
}

UINT32 cmpi_encode_cstring(const UINT32 comm, const CSTRING *cstring, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
/*
    if ( NULL_PTR == cstring )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cstring: cstring is null.\n");
        dbg_exit(MD_TBD, 0);
    }
*/    
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cstring: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cstring: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    if(NULL_PTR == cstring)
    {
        cmpi_encode_uint32(comm, 0, out_buff, out_buff_max_len, position);
        return (0);
    }

    //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_encode_cstring: cstring: %lx, %s\n", cstring, (char *)cstring_get_str(cstring));
    //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_encode_cstring: cstring %lx, out_buff_max_len %ld, beg position %ld\n", cstring, out_buff_max_len, *position);

    cmpi_pack((UINT8 *)&(cstring->len), 1, CMPI_ULONG, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack(cstring->str, cstring->len, CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);

    return (0);
}

UINT32 cmpi_encode_cstring_size(const UINT32 comm, const CSTRING *cstring, UINT32 *size)
{
    cmpi_pack_size(1, CMPI_ULONG, size,  comm);
    if(NULL_PTR != cstring)
    {
        cmpi_pack_size(cstring->len, CMPI_UCHAR, size,  comm);
    }
    return (0);
}

UINT32 cmpi_decode_cstring(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSTRING *cstring)
{
    UINT32 len;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cstring: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cstring: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cstring )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cstring: cstring is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_cstring: in_buff_max_len %ld, position %ld, cstring: %lx, %s\n", in_buff_max_len, *position, cstring, (char *)cstring_get_str(cstring));

    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)&len, 1,   CMPI_ULONG, comm);

    if(0 == len)
    {
        return (0);
    }

    if(EC_FALSE == cstring_expand_to(cstring, len + cstring->len + 1))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cstring: failed to expand cstring with capaciy %ld and len %ld to size %ld\n",
                        cstring->capacity, cstring->len, len + cstring->len + 1);
        return ((UINT32)(-1));
    }

    cmpi_unpack(in_buff, in_buff_max_len, position, cstring->str + cstring->len, len, CMPI_UCHAR, comm);
    cstring->len += len;
    cstring->str[ cstring->len ] = '\0';

    //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_cstring: cstring: %lx, %s\n", cstring, (char *)cstring_get_str(cstring));

    return (0);
}

UINT32 cmpi_encode_taskc_node(const UINT32 comm, const TASKC_NODE *taskc_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == taskc_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_node: taskc_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, TASKC_NODE_TCID(taskc_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASKC_NODE_COMM(taskc_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASKC_NODE_SIZE(taskc_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_taskc_node_size(const UINT32 comm, const TASKC_NODE *taskc_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, TASKC_NODE_TCID(taskc_node), size);
    cmpi_encode_uint32_size(comm, TASKC_NODE_COMM(taskc_node), size);
    cmpi_encode_uint32_size(comm, TASKC_NODE_SIZE(taskc_node), size);

    return (0);
}

UINT32 cmpi_decode_taskc_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASKC_NODE *taskc_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == taskc_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_node: taskc_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASKC_NODE_TCID(taskc_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASKC_NODE_COMM(taskc_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASKC_NODE_SIZE(taskc_node)));

    return (0);
}

UINT32 cmpi_encode_taskc_mgr(const UINT32 comm, const TASKC_MGR *taskc_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    CLIST *taskc_node_list;
    CLIST_DATA *clist_data;

    UINT32 taskc_node_idx;
    UINT32 taskc_node_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == taskc_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_mgr: taskc_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_mgr: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_taskc_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    taskc_node_list = (CLIST *)TASKC_MGR_NODE_LIST(taskc_mgr);
    taskc_node_num = clist_size(taskc_node_list);

    cmpi_encode_uint32(comm, taskc_node_num, out_buff, out_buff_max_len, position);

    taskc_node_idx = 0;
    CLIST_LOOP_NEXT(taskc_node_list, clist_data)
    {
        TASKC_NODE *taskc_node;
        taskc_node = (TASKC_NODE *)CLIST_DATA_DATA(clist_data);
        cmpi_encode_taskc_node(comm, taskc_node, out_buff, out_buff_max_len, position);
        taskc_node_idx ++;
    }

    /*validity checking*/
    if(taskc_node_idx != taskc_node_num)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_taskc_mgr: encoded taskc node num = %ld, but clist size = %ld\n", taskc_node_idx, taskc_node_num);
        dbg_exit(MD_TBD, 0);
    }

    return (0);
}

UINT32 cmpi_encode_taskc_mgr_size(const UINT32 comm, const TASKC_MGR *taskc_mgr, UINT32 *size)
{
    CLIST *taskc_node_list;
    CLIST_DATA *clist_data;

    UINT32 taskc_node_idx;
    UINT32 taskc_node_num;

    taskc_node_list = (CLIST *)TASKC_MGR_NODE_LIST(taskc_mgr);
    taskc_node_num = clist_size(taskc_node_list);

    cmpi_encode_uint32_size(comm, taskc_node_num, size);

    taskc_node_idx = 0;
    CLIST_LOOP_NEXT(taskc_node_list, clist_data)
    {
        TASKC_NODE *taskc_node;
        taskc_node = (TASKC_NODE *)CLIST_DATA_DATA(clist_data);
        cmpi_encode_taskc_node_size(comm, taskc_node, size);
        taskc_node_idx ++;
    }

    /*validity checking*/
    if(taskc_node_idx != taskc_node_num)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_taskc_mgr_size: encoded taskc node num = %ld, but clist size = %ld\n", taskc_node_idx, taskc_node_num);
        dbg_exit(MD_TBD, 0);
    }

    return (0);
}

UINT32 cmpi_decode_taskc_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASKC_MGR *taskc_mgr)
{
    CLIST *taskc_node_list;
    TASKC_NODE *taskc_node;

    UINT32 taskc_node_num;
    UINT32 taskc_node_idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_mgr: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == taskc_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_taskc_mgr: taskc_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    taskc_node_list = TASKC_MGR_NODE_LIST(taskc_mgr);

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(taskc_node_num));

    //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_taskc_mgr: before decode, taskc_node_list is:\n");
    //clist_print(LOGSTDOUT, taskc_node_list, (CLIST_DATA_DATA_PRINT)tst_taskc_node_print);

    for(taskc_node_idx = 0; taskc_node_idx < taskc_node_num; taskc_node_idx ++)
    {
        taskc_node_new(&taskc_node);
        if(NULL_PTR == taskc_node)
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_taskc_mgr: failed to alloc TASKC_NODE when idx = %ld\n", taskc_node_idx);
            return ((UINT32)(-1));
        }
        cmpi_decode_taskc_node(comm, in_buff, in_buff_max_len, position, taskc_node);

        clist_push_back(taskc_node_list, (void *)taskc_node);
        //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_taskc_mgr: new taskc_node %lx\n", taskc_node);

        //dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_taskc_mgr: after decode # %ld, taskc_node_list is:\n", taskc_node_idx);
        //clist_print(LOGSTDOUT, taskc_node_list, (CLIST_DATA_DATA_PRINT)tst_taskc_node_print);
    }

    return (0);
}

UINT32 cmpi_encode_vmm_node(const UINT32 comm, const VMM_NODE *vmm_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == vmm_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vmm_node: vmm_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vmm_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_vmm_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, VMM_NODE_TCID(vmm_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, VMM_NODE_COMM(vmm_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, VMM_NODE_RANK(vmm_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, VMM_NODE_MODI(vmm_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, VMM_NODE_LOAD(vmm_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, VMM_NODE_ADDR(vmm_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_vmm_node_size(const UINT32 comm, const VMM_NODE *vmm_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, VMM_NODE_TCID(vmm_node), size);
    cmpi_encode_uint32_size(comm, VMM_NODE_COMM(vmm_node), size);
    cmpi_encode_uint32_size(comm, VMM_NODE_RANK(vmm_node), size);
    cmpi_encode_uint32_size(comm, VMM_NODE_MODI(vmm_node), size);
    cmpi_encode_uint32_size(comm, VMM_NODE_LOAD(vmm_node), size);
    cmpi_encode_uint32_size(comm, VMM_NODE_ADDR(vmm_node), size);

    return (0);
}

UINT32 cmpi_decode_vmm_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, VMM_NODE *vmm_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vmm_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vmm_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == vmm_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_vmm_node: vmm_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_TCID(vmm_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_COMM(vmm_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_RANK(vmm_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_MODI(vmm_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_LOAD(vmm_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(VMM_NODE_ADDR(vmm_node)));

    return (0);
}

UINT32 cmpi_encode_log(const UINT32 comm, const LOG *log, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == log )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_log: log is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_log: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_log: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, LOG_DEVICE_TYPE(log), out_buff, out_buff_max_len, position);
    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log))
    {
        UINT32 fd;

        if(LOGSTDOUT == log || stdout == LOG_FILE_FP(log))
        {
            fd = (UINT32)DEFAULT_STDOUT_LOG_INDEX;
        }
        else if(LOGSTDIN == log || stdin == LOG_FILE_FP(log))
        {
            fd = (UINT32)DEFAULT_STDIN_LOG_INDEX;
        }
        else if(LOGSTDERR == log || stderr == LOG_FILE_FP(log))
        {
            fd = (UINT32)DEFAULT_STDERR_LOG_INDEX;
        }
        else
        {
            fd = (UINT32)DEFAULT_STDNULL_LOG_INDEX;
        }
        cmpi_encode_uint32(comm, fd, out_buff, out_buff_max_len, position);
    }
    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(log))
    {
        cmpi_encode_cstring(comm, LOG_CSTR(log), out_buff, out_buff_max_len, position);
    }

    cmpi_encode_uint32(comm, LOG_SWITCH_OFF_ENABLE(log), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_log_size(const UINT32 comm, const LOG *log, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, LOG_DEVICE_TYPE(log), size);
    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log))
    {
        UINT32 fd;
        fd = DEFAULT_STDNULL_LOG_INDEX;/*any value*/
        cmpi_encode_uint32_size(comm, fd, size);
    }
    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(log))
    {
        cmpi_encode_cstring_size(comm, LOG_CSTR(log), size);
    }

    cmpi_encode_uint32_size(comm, LOG_SWITCH_OFF_ENABLE(log), size);
    return (0);
}

UINT32 cmpi_decode_log(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, LOG *log)
{
    UINT32 type;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_log: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_log: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == log )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_log: log is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &type);
    LOG_DEVICE_TYPE(log) = type;

    if(LOG_FILE_DEVICE == LOG_DEVICE_TYPE(log))
    {
        UINT32 fd;
        cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &fd);

        log_file_init(log, NULL_PTR, NULL_PTR,
                        CMPI_ERROR_TCID, CMPI_ERROR_RANK,
                        LOGD_FILE_RECORD_LIMIT_DISABLED, (UINT32)SWITCH_OFF,
                        LOGD_SWITCH_OFF_DISABLE, LOGD_PID_INFO_ENABLE);

        if(DEFAULT_STDOUT_LOG_INDEX == fd)
        {
            LOG_FILE_FP(log) = stdout;
            LOG_REDIRECT(log) = LOGSTDOUT;
        }
        else if(DEFAULT_STDIN_LOG_INDEX == fd)
        {
            LOG_FILE_FP(log) = stdin;
            LOG_REDIRECT(log) = LOGSTDIN;
        }
        else if(DEFAULT_STDERR_LOG_INDEX == fd)
        {
            LOG_FILE_FP(log) = stderr;
            LOG_REDIRECT(log) = LOGSTDERR;
        }
        else
        {
            LOG_FILE_FP(log) = NULL_PTR;
        }
    }

    if(LOG_CSTR_DEVICE == LOG_DEVICE_TYPE(log))
    {
        LOG_REDIRECT(log)  = NULL_PTR;

        if(NULL_PTR == LOG_CSTR(log))
        {
            LOG_CSTR(log) = cstring_new(NULL_PTR, LOC_CMPIE_0008);
        }
        else
        {
           cstring_reset(LOG_CSTR(log));
        }
        cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, LOG_CSTR(log));
    }

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(LOG_SWITCH_OFF_ENABLE(log)));

    return (0);
}

UINT32 cmpi_encode_kbuff(const UINT32 comm, const KBUFF *kbuff, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == kbuff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_kbuff: kbuff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_kbuff: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_kbuff: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
/*
    if(NULL_PTR == KBUFF_CACHE(kbuff))
    {
        UINT32 len;

        len = 0;
        cmpi_encode_uint32(comm, len, out_buff, out_buff_max_len, position);

        return (0);
    }
*/
    cmpi_encode_uint32(comm, KBUFF_CUR_LEN(kbuff), out_buff, out_buff_max_len, position);
    for(idx = 0; idx < KBUFF_CUR_LEN(kbuff); idx ++)
    {
        cmpi_encode_uint8(comm, KBUFF_CACHE_CHAR(kbuff, idx), out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_kbuff_size(const UINT32 comm, const KBUFF *kbuff, UINT32 *size)
{
    UINT32 idx;
/*
    if(NULL_PTR == KBUFF_CACHE(kbuff))
    {
        UINT32 len;

        len = 0;
        cmpi_encode_uint32_size(comm, len, size);

        return (0);
    }
*/
    cmpi_encode_uint32_size(comm, KBUFF_CUR_LEN(kbuff), size);
    for(idx = 0; idx < KBUFF_CUR_LEN(kbuff); idx ++)
    {
        cmpi_encode_uint8_size(comm, KBUFF_CACHE_CHAR(kbuff, idx), size);
    }

    return (0);
}

UINT32 cmpi_decode_kbuff(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, KBUFF *kbuff)
{
    UINT32 len;
    UINT32 idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_kbuff: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_kbuff: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == kbuff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_kbuff: kbuff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &len);
    if(0 == len)
    {
        /*nothing to do*/
        return (0);
    }

    if(NULL_PTR == KBUFF_CACHE(kbuff))
    {
#if 0
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_kbuff: to decode len %ld but kbuff %lx cache is null\n",
                           len, kbuff);
        return ((UINT32)-1);
#endif
        kbuff_init(kbuff,len);
    }

    if(len + KBUFF_CUR_LEN(kbuff) > KBUFF_MAX_LEN(kbuff))
    {
#if 1
       dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_kbuff: len %ld overflow kbuff %lx with cur len %ld and max len %ld\n",
                           len, kbuff, KBUFF_CUR_LEN(kbuff), KBUFF_MAX_LEN(kbuff));
       return ((UINT32)-1);
#endif

    }

    for(idx = 0; idx < len; idx ++)
    {
        cmpi_decode_uint8(comm, in_buff, in_buff_max_len, position, KBUFF_CACHE(kbuff) + KBUFF_CUR_LEN(kbuff) + idx);
    }

    KBUFF_CUR_LEN(kbuff) += len;

    //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_decode_kbuff: encoded len = %ld and kbuff %lx, cur_len = %ld, max_len = %ld\n", len, kbuff, KBUFF_CUR_LEN(kbuff), KBUFF_MAX_LEN(kbuff));

    return (0);
}


UINT32 cmpi_encode_cfile_seg(const UINT32 comm, const CFILE_SEG *cfile_seg, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 num;
    UINT32 pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cfile_seg )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg: cfile_seg is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CFILE_SEG_ID(cfile_seg)  , out_buff, out_buff_max_len, position);

    num = cvector_size(CFILE_SEG_TCID_VEC(cfile_seg));
    cmpi_encode_uint32(comm, num, out_buff, out_buff_max_len, position);

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 seg_tcid;

        seg_tcid = (UINT32)cvector_get(CFILE_SEG_TCID_VEC(cfile_seg), pos);
        cmpi_encode_uint32(comm, seg_tcid, out_buff, out_buff_max_len, position);
    }

    cmpi_encode_uint32(comm, CFILE_SEG_SIZE(cfile_seg), out_buff, out_buff_max_len, position);
    cmpi_encode_cstring(comm, CFILE_SEG_NAME(cfile_seg), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CFILE_SEG_OPEN_MODE(cfile_seg), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cfile_seg_size(const UINT32 comm, const CFILE_SEG *cfile_seg, UINT32 *size)
{
    UINT32 num;
    UINT32 pos;

    cmpi_encode_uint32_size(comm, CFILE_SEG_ID(cfile_seg)  , size);

    num = cvector_size(CFILE_SEG_TCID_VEC(cfile_seg));
    cmpi_encode_uint32_size(comm, num, size);

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 seg_tcid;

        seg_tcid = (UINT32)cvector_get(CFILE_SEG_TCID_VEC(cfile_seg), pos);
        cmpi_encode_uint32_size(comm, seg_tcid, size);
    }

    cmpi_encode_uint32_size(comm, CFILE_SEG_SIZE(cfile_seg), size);
    cmpi_encode_cstring_size(comm, CFILE_SEG_NAME(cfile_seg), size);

    cmpi_encode_uint32_size(comm, CFILE_SEG_OPEN_MODE(cfile_seg), size);

    return (0);
}

UINT32 cmpi_decode_cfile_seg(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_SEG *cfile_seg)
{
    UINT32 num;
    UINT32 pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cfile_seg )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg: cfile_seg is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*need to clean up segment file name string due to cstring decoding will append the new string to the old one*/
    cstring_clean(CFILE_SEG_NAME(cfile_seg));

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CFILE_SEG_ID(cfile_seg)));

    num = cvector_size(CFILE_SEG_TCID_VEC(cfile_seg));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 seg_tcid;

        cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(seg_tcid));
        cvector_push(CFILE_SEG_TCID_VEC(cfile_seg), (void *)seg_tcid);
    }

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CFILE_SEG_SIZE(cfile_seg)));
    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CFILE_SEG_NAME(cfile_seg));

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CFILE_SEG_OPEN_MODE(cfile_seg)));

    return (0);
}
#if 0
UINT32 cmpi_encode_cfile_seg_vec(const UINT32 comm, const CFILE_SEG_VEC *cfile_seg_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 cfile_seg_pos;
    UINT32 cfile_seg_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cfile_seg_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg_vec: cfile_seg_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg_vec: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_seg_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cfile_seg_num = cvector_size(CFILE_SEG_VEC(cfile_seg_vec));
    cmpi_encode_uint32(comm, cfile_seg_num, out_buff, out_buff_max_len, position);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG * cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(CFILE_SEG_VEC(cfile_seg_vec), cfile_seg_pos);

        cmpi_encode_cfile_seg(comm, cfile_seg, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_cfile_seg_vec_size(const UINT32 comm, const CFILE_SEG_VEC *cfile_seg_vec, UINT32 *size)
{
    UINT32 cfile_seg_pos;
    UINT32 cfile_seg_num;

    cfile_seg_num = cvector_size(CFILE_SEG_VEC(cfile_seg_vec));
    cmpi_encode_uint32_size(comm, cfile_seg_num, size);

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG * cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(CFILE_SEG_VEC(cfile_seg_vec), cfile_seg_pos);

        cmpi_encode_cfile_seg_size(comm, cfile_seg, size);
    }


    return (0);
}

UINT32 cmpi_decode_cfile_seg_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_SEG_VEC *cfile_seg_vec)
{
    UINT32 cfile_seg_pos;
    UINT32 cfile_seg_num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg_vec: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cfile_seg_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_seg_vec: cfile_seg_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(cfile_seg_num));

    if(cfile_seg_num != cvector_size(CFILE_SEG_VEC(cfile_seg_vec)))
    {
        if(0 != cvector_size(CFILE_SEG_VEC(cfile_seg_vec)))
        {
            /*sorry: if seg num in vec is not zero or not matched, we have no idea regarding free segs. that is the problem.*/
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cfile_seg_vec: mismatched or invalid seg num %ld in vec against to decoded num %ld\n",
                               cvector_size(CFILE_SEG_VEC(cfile_seg_vec)), cfile_seg_num);
            return ((UINT32)-1);
        }

        cvector_init(CFILE_SEG_VEC(cfile_seg_vec), cfile_seg_num, LOC_CMPIE_0009);
    }

    for(cfile_seg_pos = 0; cfile_seg_pos < cfile_seg_num; cfile_seg_pos ++)
    {
        CFILE_SEG * cfile_seg;

        cfile_seg = (CFILE_SEG *)cvector_get(CFILE_SEG_VEC(cfile_seg_vec), cfile_seg_pos);
        if(NULL_PTR == cfile_seg)
        {
            alloc_static_mem(MD_TBD, 0, MM_CFILE_SEG, &cfile_seg, LOC_CMPIE_0010);
            cfile_seg_init(CMPI_ANY_MODI, cfile_seg);
            cvector_push(CFILE_SEG_VEC(cfile_seg_vec), (void *)cfile_seg);
        }

        cmpi_decode_cfile_seg(comm, in_buff, in_buff_max_len, position, cfile_seg);
    }

    return (0);
}

#endif

UINT32 cmpi_encode_cfile_node(const UINT32 comm, const CFILE_NODE *cfile_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 num;
    UINT32 pos;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cfile_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_node: cfile_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfile_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CFILE_NODE_NAME(cfile_node), out_buff, out_buff_max_len, position);

    num = cvector_size(CFILE_NODE_TCID_VEC(cfile_node));
    cmpi_encode_uint32(comm, num, out_buff, out_buff_max_len, position);

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 node_tcid;

        node_tcid = (UINT32)cvector_get(CFILE_NODE_TCID_VEC(cfile_node), pos);
        cmpi_encode_uint32(comm, node_tcid, out_buff, out_buff_max_len, position);
    }

    cmpi_encode_uint32(comm, CFILE_NODE_SIZE(cfile_node), out_buff, out_buff_max_len, position);
    cmpi_encode_cvector(comm, CFILE_SEG_VEC(cfile_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cfile_node_size(const UINT32 comm, const CFILE_NODE *cfile_node, UINT32 *size)
{
    UINT32 num;
    UINT32 pos;

    cmpi_encode_cstring_size(comm, CFILE_NODE_NAME(cfile_node), size);

    num = cvector_size(CFILE_NODE_TCID_VEC(cfile_node));
    cmpi_encode_uint32_size(comm, num, size);

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 node_tcid;

        node_tcid = (UINT32)cvector_get(CFILE_NODE_TCID_VEC(cfile_node), pos);
        cmpi_encode_uint32_size(comm, node_tcid, size);
    }

    cmpi_encode_uint32_size(comm, CFILE_NODE_SIZE(cfile_node), size);
    cmpi_encode_cvector_size(comm, CFILE_SEG_VEC(cfile_node), size);

    return (0);
}

UINT32 cmpi_decode_cfile_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFILE_NODE *cfile_node)
{
    UINT32 num;
    UINT32 pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cfile_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfile_node: cfile_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cstring_reset(CFILE_NODE_NAME(cfile_node));/*not save the old name to prevent name from string repeating based on cmpi_decode_cstring implementation*/

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CFILE_NODE_NAME(cfile_node));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));

    for(pos = 0; pos < num; pos ++)
    {
        UINT32 node_tcid;

        cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(node_tcid));
        cvector_push(CFILE_NODE_TCID_VEC(cfile_node), (void *)node_tcid);
    }

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CFILE_NODE_SIZE(cfile_node)));
    cmpi_decode_cvector(comm, in_buff, in_buff_max_len, position, CFILE_SEG_VEC(cfile_node));

    return (0);
}


UINT32 cmpi_encode_cdir_seg(const UINT32 comm, const CDIR_SEG *cdir_seg, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdir_seg )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_seg: cdir_seg is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_seg: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_seg: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm , CDIR_SEG_TYPE(cdir_seg), out_buff, out_buff_max_len, position);
    cmpi_encode_cstring(comm, CDIR_SEG_NAME(cdir_seg), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cdir_seg_size(const UINT32 comm, const CDIR_SEG *cdir_seg, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CDIR_SEG_TYPE(cdir_seg) , size);
    cmpi_encode_cstring_size(comm, CDIR_SEG_NAME(cdir_seg), size);

    return (0);
}

UINT32 cmpi_decode_cdir_seg(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDIR_SEG *cdir_seg)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_seg: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_seg: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdir_seg )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_seg: cdir_seg is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*need to clean up segment file name string due to cstring decoding will append the new string to the old one*/
    cstring_clean(CDIR_SEG_NAME(cdir_seg));

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CDIR_SEG_TYPE(cdir_seg)));
    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CDIR_SEG_NAME(cdir_seg));

    return (0);
}

UINT32 cmpi_encode_cdir_node(const UINT32 comm, const CDIR_NODE *cdir_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdir_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_node: cdir_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdir_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CDIR_NODE_NAME(cdir_node), out_buff, out_buff_max_len, position);

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    cmpi_encode_uint32(comm, cdir_seg_num, out_buff, out_buff_max_len, position);

    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG * cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);

        cmpi_encode_cdir_seg(comm, cdir_seg, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_cdir_node_size(const UINT32 comm, const CDIR_NODE *cdir_node, UINT32 *size)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;

    cmpi_encode_cstring_size(comm, CDIR_NODE_NAME(cdir_node), size);

    cdir_seg_num = cvector_size(CDIR_NODE_SEGS(cdir_node));
    cmpi_encode_uint32_size(comm, cdir_seg_num, size);

    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG * cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);

        cmpi_encode_cdir_seg_size(comm, cdir_seg, size);
    }

    return (0);
}

UINT32 cmpi_decode_cdir_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDIR_NODE *cdir_node)
{
    UINT32 cdir_seg_num;
    UINT32 cdir_seg_pos;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdir_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdir_node: cdir_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*need to clean up nodement dir name string due to cstring decoding will append the new string to the old one*/
    cstring_clean(CDIR_NODE_NAME(cdir_node));

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CDIR_NODE_NAME(cdir_node));

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(cdir_seg_num));

    if(cdir_seg_num != cvector_size(CDIR_NODE_SEGS(cdir_node)))
    {
        if(0 != cvector_size(CDIR_NODE_SEGS(cdir_node)))
        {
            /*sorry: if seg num in vec is not zero or not matched, we have no idea regarding free segs. that is the problem.*/
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cdir_node: mismatched or invalid seg num %ld in vec against to decoded num %ld\n",
                               cvector_size(CDIR_NODE_SEGS(cdir_node)), cdir_seg_num);
            return ((UINT32)-1);
        }

        cvector_init(CDIR_NODE_SEGS(cdir_node), cdir_seg_num, MM_CDIR_SEG, CVECTOR_LOCK_ENABLE, LOC_CMPIE_0011);
    }

    for(cdir_seg_pos = 0; cdir_seg_pos < cdir_seg_num; cdir_seg_pos ++)
    {
        CDIR_SEG * cdir_seg;

        cdir_seg = (CDIR_SEG *)cvector_get(CDIR_NODE_SEGS(cdir_node), cdir_seg_pos);
        if(NULL_PTR == cdir_seg)
        {
            alloc_static_mem(MD_TBD, 0, MM_CDIR_SEG, &cdir_seg, LOC_CMPIE_0012);
            cdir_seg_init(CMPI_ANY_MODI, cdir_seg);
            cvector_push(CDIR_NODE_SEGS(cdir_node), (void *)cdir_seg);
        }

        cmpi_decode_cdir_seg(comm, in_buff, in_buff_max_len, position, cdir_seg);
    }

    return (0);
}

UINT32 cmpi_encode_cvector(const UINT32 comm, const CVECTOR *cvector, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 num;
    UINT32 type;

    UINT32 pos;
    CVECTOR_DATA_ENCODER data_encoder;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cvector )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cvector: cvector is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cvector: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cvector: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    type = cvector_type(cvector);
    num = cvector_size(cvector);

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_cvector: cvector %lx, type = %ld, num = %ld, position = %ld\n",
                        cvector, type, num, *position));

    cmpi_encode_uint32(comm, type, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, num, out_buff, out_buff_max_len, position);

    if(0 == num)
    {
        return (0);
    }

    data_encoder = (CVECTOR_DATA_ENCODER)cvector_codec_get(cvector, CVECTOR_CODEC_ENCODER);
    if(NULL_PTR == data_encoder)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cvector: cvector data encoder is null\n");
        return ((UINT32)-1);
    }
#if 0
    if(MM_UINT32 == type)
    {
        for(pos = 0; pos < num; pos ++)
        {
            //void * data;
            //data = (void *)cvector_get_addr(cvector, pos);
            UINT32 data;
            data = (UINT32)cvector_get(cvector, pos);
            data_encoder(comm, data, out_buff, out_buff_max_len, position);
        }
    }
    else/*non UINT32*/
    {
        for(pos = 0; pos < num; pos ++)
        {
            void * data;
            data = cvector_get(cvector, pos);

            data_encoder(comm, data, out_buff, out_buff_max_len, position);
        }
    }
#endif

#if 1
    for(pos = 0; pos < num; pos ++)
    {
        void * data;
        data = cvector_get(cvector, pos);

        data_encoder(comm, data, out_buff, out_buff_max_len, position);
    }
#endif
    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_cvector: cvector %lx, type = %ld, num = %ld ==> position = %ld\n",
                        cvector, cvector_type(cvector), cvector_size(cvector), *position));

    return (0);
}

UINT32 cmpi_encode_cvector_size(const UINT32 comm, const CVECTOR *cvector, UINT32 *size)
{
    UINT32 num;
    UINT32 type;

    UINT32 pos;
    CVECTOR_DATA_ENCODER_SIZE data_encoder_size;

    type = cvector_type(cvector);
    num = cvector_size(cvector);

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_cvector_size: cvector %lx: type = %ld, num = %ld, size = %ld\n", cvector, type, num, *size));

    if(MM_END == type)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cvector_size: cvector %lx: invalid type = %ld, num = %ld\n", cvector, type, num);
    }

    cmpi_encode_uint32_size(comm, type, size);
    cmpi_encode_uint32_size(comm, num, size);

    if(0 == num)
    {
        return (0);
    }

    data_encoder_size = (CVECTOR_DATA_ENCODER_SIZE)cvector_codec_get(cvector, CVECTOR_CODEC_ENCODER_SIZE);
    if(NULL_PTR == data_encoder_size)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cvector_size: cvector %lx: type = %ld, num = %ld, data encoder_size is null\n",
                            cvector, type, num);
        return ((UINT32)-1);
    }

     //dbg_log(SEC_0035_CMPIE, 0)(LOGCONSOLE, "[DEBUG] cmpi_encode_cvector_size: cvector %lx, mm type %ld\n", cvector, cvector->data_mm_type);

    if(MM_UINT32 == type)
    {
        for(pos = 0; pos < num; pos ++)
        {
            void * data;
            data = (void *)cvector_get_addr(cvector, pos);
            data_encoder_size(comm, data, size);
        }
    }
    else/*non UINT32*/
    {
        for(pos = 0; pos < num; pos ++)
        {
            void * data;
            data = cvector_get(cvector, pos);
            data_encoder_size(comm, data, size);
        }
    }

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_cvector_size: cvector %lx: type = %ld, num = %ld, ==> size %ld\n",
                            cvector, cvector_type(cvector), cvector_size(cvector), *size));

    return (0);
}

UINT32 cmpi_decode_cvector(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CVECTOR *cvector)
{
    UINT32 num;
    UINT32 type;

    UINT32 pos;
    CVECTOR_DATA_DECODER data_decoder;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cvector: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cvector: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cvector )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cvector: cvector is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(type));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));

    dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_cvector: enter: cvector %lx, type = %ld, num = %ld, size = %ld\n", cvector, type, num, cvector->size);

    if(type != cvector->data_mm_type)
    {
        dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_cvector: cvector %lx, data type %ld ==> %ld\n", cvector, cvector->data_mm_type, type);
        cvector_codec_set(cvector, type);
    }
    dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_cvector: [0] cvector %lx, data type %ld \n", cvector, cvector->data_mm_type);

    if(0 == num)
    {
        return (0);
    }
#if 0
    if(0 < cvector_size(cvector))
    {
        if(MM_UINT32 == type)
        {
            cvector_clean(cvector, NULL_PTR, LOC_CMPIE_0013);
        }
        else
        {
            UINT32 size;
            CVECTOR_DATA_FREE data_free;

            size = cvector_size(cvector);
            data_free = (CVECTOR_DATA_FREE)cvector_codec_get(cvector, CVECTOR_CODEC_FREE);
            for(pos = 0; pos < size; pos ++)
            {
                void *data;
                data = cvector_get(cvector, pos);
                data_free(CMPI_ANY_MODI, data);
            }
            cvector_clean(cvector, NULL_PTR, LOC_CMPIE_0014);
        }
    }
#endif
    data_decoder = (CVECTOR_DATA_DECODER)cvector_codec_get(cvector, CVECTOR_CODEC_DECODER);
    if(NULL_PTR == data_decoder)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cvector: cvector %lx data decoder is null\n", cvector);
        return ((UINT32)-1);
    }

    if(MM_UINT32 == type)
    {
        UINT32 size;

        size = cvector_size(cvector);

        /*re-use the old item to accept the decoded result*/
        for(pos = 0; pos < size && pos < num; pos ++)
        {
            void * data;
            data = (void *)cvector_get_addr(cvector, pos);
            if(NULL_PTR == data)
            {
                data = (void *)cvector_get_addr(cvector, pos);
                //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_decode_cvector: [2] cvector %lx, size %ld, capacity %ld\n", cvector, cvector->size, cvector->capacity);
            }
            data_decoder(comm, in_buff, in_buff_max_len, position, data);
        }

        /*alloc new item to accept the decoded result, and push the new item*/
        for(; pos < num; pos ++)
        {
            void * data;
            cvector_push(cvector, (void *)0);/*add new one*/
            data = (void *)cvector_get_addr(cvector, pos);
            if(NULL_PTR == data)
            {
                dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cvector: cvector %lx, size %ld, capacity %ld, pos = %ld is null\n",
                                    cvector, cvector->size, cvector->capacity, pos);
                return ((UINT32)-1);
            }
            data_decoder(comm, in_buff, in_buff_max_len, position, data);
        }
    }
    else/*non UINT32*/
    {
        UINT32 size;

        CVECTOR_DATA_INIT    data_init;

        data_init = (CVECTOR_DATA_INIT)cvector_codec_get(cvector, CVECTOR_CODEC_INIT);/*data_init may be null pointer*/
        if(NULL_PTR == data_init)
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cvector: cvector %lx data init is null\n", cvector);
            return ((UINT32)-1);
        }

        size = cvector_size(cvector);

        /*re-use the old item to accept the decoded result*/
        for(pos = 0; pos < size && pos < num; pos ++)
        {
            void * data;
            data = cvector_get(cvector, pos);
            if(NULL_PTR == data)
            {
                alloc_static_mem(MD_TBD, 0, type, &data, LOC_CMPIE_0015);
                data_init(CMPI_ANY_MODI, data);
                cvector_set(cvector, pos, (void *)data);/*add new one*/
                //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_decode_cvector: [3] cvector %lx, size %ld, capacity %ld\n", cvector, cvector->size, cvector->capacity);
            }
            data_decoder(comm, in_buff, in_buff_max_len, position, data);
        }

        /*alloc new item to accept the decoded result, and push the new item*/
        for(; pos < num; pos ++)
        {
            void * data;

            alloc_static_mem(MD_TBD, 0, type, &data, LOC_CMPIE_0016);
            if(NULL_PTR == data)
            {
                dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cvector: [3] cvector %lx, size %ld, capacity %ld, pos = %ld failed to alloc\n",
                                    cvector, cvector->size, cvector->capacity, pos);
            }
            data_init(CMPI_ANY_MODI, data);
            data_decoder(comm, in_buff, in_buff_max_len, position, data);
            cvector_push(cvector, (void *)data);/*add new one*/
        }
    }
    CMPI_DBG((LOGSTDOUT, "info:cmpi_decode_cvector: leave: cvector %lx, type = %ld, num = %ld, size = %ld\n", cvector, type, num, cvector->size));
    return (0);
}

UINT32 cmpi_encode_csocket_cnode(const UINT32 comm, const CSOCKET_CNODE *csocket_cnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 sockfd;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csocket_cnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csocket_cnode: csocket_cnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csocket_cnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csocket_cnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    sockfd = INT32_TO_UINT32(CSOCKET_CNODE_SOCKFD(csocket_cnode));

    cmpi_encode_uint32(comm, CSOCKET_CNODE_IPADDR(csocket_cnode) , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSOCKET_CNODE_SRVPORT(csocket_cnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSOCKET_CNODE_TCID(csocket_cnode)   , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSOCKET_CNODE_COMM(csocket_cnode)   , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSOCKET_CNODE_SIZE(csocket_cnode)   , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, sockfd                              , out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csocket_cnode_size(const UINT32 comm, const CSOCKET_CNODE *csocket_cnode, UINT32 *size)
{
    UINT32 sockfd;

    sockfd = INT32_TO_UINT32(CSOCKET_CNODE_SOCKFD(csocket_cnode));

    cmpi_encode_uint32_size(comm, CSOCKET_CNODE_IPADDR(csocket_cnode) , size);
    cmpi_encode_uint32_size(comm, CSOCKET_CNODE_SRVPORT(csocket_cnode), size);
    cmpi_encode_uint32_size(comm, CSOCKET_CNODE_TCID(csocket_cnode)   , size);
    cmpi_encode_uint32_size(comm, CSOCKET_CNODE_COMM(csocket_cnode)   , size);
    cmpi_encode_uint32_size(comm, CSOCKET_CNODE_SIZE(csocket_cnode)   , size);
    cmpi_encode_uint32_size(comm, sockfd                              , size);

    return (0);
}

UINT32 cmpi_decode_csocket_cnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSOCKET_CNODE *csocket_cnode)
{
    UINT32 sockfd;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csocket_cnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csocket_cnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csocket_cnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csocket_cnode: csocket_cnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSOCKET_CNODE_IPADDR(csocket_cnode)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSOCKET_CNODE_SRVPORT(csocket_cnode)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSOCKET_CNODE_TCID(csocket_cnode))   );
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSOCKET_CNODE_COMM(csocket_cnode))   );
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSOCKET_CNODE_SIZE(csocket_cnode))   );
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(sockfd)                               );

    CSOCKET_CNODE_SOCKFD(csocket_cnode) = (int)UINT32_TO_INT32(sockfd);

    return (0);
}

UINT32 cmpi_encode_cmon_obj(const UINT32 comm, const CMON_OBJ *cmon_obj, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cmon_obj )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj: cmon_obj is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CMON_OBJ_OID(cmon_obj) , out_buff, out_buff_max_len, position);

    switch(CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        case MM_UINT32:
            cmpi_encode_uint32(comm, CMON_OBJ_MEAS_DATA_UINT32(cmon_obj), out_buff, out_buff_max_len, position);
            break;

        case MM_REAL:
            cmpi_encode_real(comm, &(CMON_OBJ_MEAS_DATA_REAL(cmon_obj)), out_buff, out_buff_max_len, position);
            break;

        case MM_CVECTOR:
            //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_encode_cmon_obj: beg to encode cvector %lx, type = %ld, item type = %ld\n", CMON_OBJ_MEAS_DATA_VEC(cmon_obj), cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)), CMON_OBJ_ITEM_TYPE(cmon_obj));
            cmpi_encode_cvector(comm, CMON_OBJ_MEAS_DATA_VEC(cmon_obj), out_buff, out_buff_max_len, position);
            break;

        case MM_CRANK_THREAD_STAT:
            cmpi_encode_crank_thread_stat(comm, CMON_OBJ_MEAS_DATA_CRANK_THREAD_STAT(cmon_obj), out_buff, out_buff_max_len, position);
            break;

        default:
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cmon_obj: unknow data type %ld\n", CMON_OBJ_DATA_TYPE(cmon_obj));
            return ((UINT32)-1);
    }

    return (0);
}

UINT32 cmpi_encode_cmon_obj_size(const UINT32 comm, const CMON_OBJ *cmon_obj, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CMON_OBJ_OID(cmon_obj) , size);

    switch(CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        case MM_UINT32:
            cmpi_encode_uint32_size(comm, CMON_OBJ_MEAS_DATA_UINT32(cmon_obj), size);
            break;

        case MM_REAL:
            cmpi_encode_real_size(comm, &(CMON_OBJ_MEAS_DATA_REAL(cmon_obj)), size);
            break;

        case MM_CVECTOR:
            //cvector_codec_set(CMON_OBJ_MEAS_DATA_VEC(cmon_obj), CMON_OBJ_ITEM_TYPE(cmon_obj));
            if(cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)) != CMON_OBJ_ITEM_TYPE(cmon_obj))
            {
                dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cmon_obj_size: before encode size, cvector %lx, type %ld but item type %ld\n",
                                    CMON_OBJ_MEAS_DATA_VEC(cmon_obj),
                                    cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)),
                                    CMON_OBJ_ITEM_TYPE(cmon_obj));
            }
            //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_encode_cmon_obj_size: beg to encode_size cvector %lx, type = %ld, item type = %ld\n", CMON_OBJ_MEAS_DATA_VEC(cmon_obj), cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)), CMON_OBJ_ITEM_TYPE(cmon_obj));
            cmpi_encode_cvector_size(comm, CMON_OBJ_MEAS_DATA_VEC(cmon_obj), size);
            break;

        case MM_CRANK_THREAD_STAT:
            cmpi_encode_crank_thread_stat_size(comm, CMON_OBJ_MEAS_DATA_CRANK_THREAD_STAT(cmon_obj), size);
            break;

        default:
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_cmon_obj_size: unknow data type %ld\n", CMON_OBJ_DATA_TYPE(cmon_obj));
            return ((UINT32)-1);
    }

    return (0);
}

UINT32 cmpi_decode_cmon_obj(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CMON_OBJ *cmon_obj)
{
    UINT32    cmon_oid;
    CMON_MAP *cmon_map;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cmon_obj )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj: cmon_obj is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(cmon_oid));
    if(NULL_PTR == CMON_OBJ_MAP(cmon_obj))
    {
        /*if map is null, we deduce that cmon_obj memory is alloced when decoding task req/rsp, hence initialize it here*/
        cmon_map = cmon_map_fetch(CMPI_ANY_MODI, cmon_oid);
        CMON_OBJ_MAP(cmon_obj) = cmon_map;

        cmon_obj_init(CMPI_ANY_MODI, cmon_obj);
    }

    if(CMON_OBJ_OID(cmon_obj) != cmon_oid)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cmon_obj: cmon_oid %ld mismatched with cmon_obj_oid %ld\n", cmon_oid, CMON_OBJ_OID(cmon_obj));
        return ((UINT32)-1);
    }

    switch(CMON_OBJ_DATA_TYPE(cmon_obj))
    {
        case MM_UINT32:
            cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CMON_OBJ_MEAS_DATA_UINT32(cmon_obj)));
            break;

        case MM_REAL:
            cmpi_decode_real(comm, in_buff, in_buff_max_len, position, &(CMON_OBJ_MEAS_DATA_REAL(cmon_obj)));
            break;

        case MM_CVECTOR:
            //xxxx receiver should initialize cvector,fuck!
            //dbg_log(SEC_0035_CMPIE, 3)(LOGSTDOUT, "info:cmpi_decode_cmon_obj: beg to decode cvector %lx, type = %ld, item type = %ld\n", CMON_OBJ_MEAS_DATA_VEC(cmon_obj), cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)), CMON_OBJ_ITEM_TYPE(cmon_obj));
#if 0
            if(cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)) != CMON_OBJ_ITEM_TYPE(cmon_obj))
            {
                dbg_log(SEC_0035_CMPIE, 5)(LOGSTDOUT, "cmpi_decode_cmon_obj: cvector type %ld ===> %ld\n", cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)), CMON_OBJ_ITEM_TYPE(cmon_obj));
                cvector_codec_set(CMON_OBJ_MEAS_DATA_VEC(cmon_obj), CMON_OBJ_ITEM_TYPE(cmon_obj));/*this line is must when receiver is decoding!*/
            }
#endif
            cmpi_decode_cvector(comm, in_buff, in_buff_max_len, position, CMON_OBJ_MEAS_DATA_VEC(cmon_obj));
            if(cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)) != CMON_OBJ_ITEM_TYPE(cmon_obj))
            {
                dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cmon_obj: afeter decode, cvector %lx, type %ld but item type %ld\n",
                                    CMON_OBJ_MEAS_DATA_VEC(cmon_obj),
                                    cvector_type(CMON_OBJ_MEAS_DATA_VEC(cmon_obj)),
                                    CMON_OBJ_ITEM_TYPE(cmon_obj));
            }

            break;

        case MM_CRANK_THREAD_STAT:
            cmpi_decode_crank_thread_stat(comm, in_buff, in_buff_max_len, position, CMON_OBJ_MEAS_DATA_CRANK_THREAD_STAT(cmon_obj));
            break;

        default:
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cmon_obj: unknow data type %ld\n", CMON_MAP_DATA_TYPE(cmon_map));
            return ((UINT32)-1);
    }

    return (0);
}

UINT32 cmpi_encode_cmon_obj_vec(const UINT32 comm, const CMON_OBJ_VEC *cmon_obj_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cmon_obj_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj_vec: cmon_obj_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj_vec: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmon_obj_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*codec must be set in vector before calling*/
    cmpi_encode_cvector(comm, CMON_OBJ_VEC(cmon_obj_vec), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cmon_obj_vec_size(const UINT32 comm, const CMON_OBJ_VEC *cmon_obj_vec, UINT32 *size)
{
    /*codec must be set in vector before calling*/
    cmpi_encode_cvector_size(comm, CMON_OBJ_VEC(cmon_obj_vec), size);

    return (0);
}

UINT32 cmpi_decode_cmon_obj_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CMON_OBJ_VEC *cmon_obj_vec)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj_vec: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cmon_obj_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmon_obj_vec: cmon_obj_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    /*codec must be set in vector before calling*/
    cmpi_decode_cvector(comm, in_buff, in_buff_max_len, position, CMON_OBJ_VEC(cmon_obj_vec));

    return (0);
}

UINT32 cmpi_encode_csys_cpu_stat(const UINT32 comm, const CSYS_CPU_STAT *csys_cpu_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csys_cpu_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_cpu_stat: csys_cpu_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_cpu_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_cpu_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSYS_CPU_STAT_CSTR(csys_cpu_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_CPU_STAT_USER(csys_cpu_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_CPU_STAT_NICE(csys_cpu_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_CPU_STAT_SYS(csys_cpu_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_CPU_STAT_IDLE(csys_cpu_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_CPU_STAT_TOTAL(csys_cpu_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csys_cpu_stat_size(const UINT32 comm, const CSYS_CPU_STAT *csys_cpu_stat, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSYS_CPU_STAT_CSTR(csys_cpu_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_CPU_STAT_USER(csys_cpu_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_CPU_STAT_NICE(csys_cpu_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_CPU_STAT_SYS(csys_cpu_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_CPU_STAT_IDLE(csys_cpu_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_CPU_STAT_TOTAL(csys_cpu_stat), size);

    return (0);
}

UINT32 cmpi_decode_csys_cpu_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_CPU_STAT *csys_cpu_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_cpu_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_cpu_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csys_cpu_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_cpu_stat: csys_cpu_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSYS_CPU_STAT_CSTR(csys_cpu_stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_CPU_STAT_USER(csys_cpu_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_CPU_STAT_NICE(csys_cpu_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_CPU_STAT_SYS(csys_cpu_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_CPU_STAT_IDLE(csys_cpu_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_CPU_STAT_TOTAL(csys_cpu_stat)));

    return (0);
}


UINT32 cmpi_encode_mm_man_occupy_node(const UINT32 comm, const MM_MAN_OCCUPY_NODE *mm_man_occupy_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == mm_man_occupy_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_occupy_node: mm_man_occupy_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_occupy_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_occupy_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node) , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node) , out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node) , out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_mm_man_occupy_node_size(const UINT32 comm, const MM_MAN_OCCUPY_NODE *mm_man_occupy_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node), size);
    cmpi_encode_uint32_size(comm, MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node) , size);
    cmpi_encode_uint32_size(comm, MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node) , size);
    cmpi_encode_uint32_size(comm, MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node) , size);

    return (0);
}

UINT32 cmpi_decode_mm_man_occupy_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MM_MAN_OCCUPY_NODE *mm_man_occupy_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_occupy_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_occupy_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == mm_man_occupy_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_occupy_node: mm_man_occupy_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MM_MAN_OCCUPY_NODE_TYPE(mm_man_occupy_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MM_MAN_OCCUPY_NODE_SUM(mm_man_occupy_node) ));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MM_MAN_OCCUPY_NODE_MAX(mm_man_occupy_node) ));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MM_MAN_OCCUPY_NODE_CUR(mm_man_occupy_node) ));

    return (0);
}

UINT32 cmpi_encode_mm_man_load_node(const UINT32 comm, const MM_MAN_LOAD_NODE *mm_man_load_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == mm_man_load_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_load_node: mm_man_load_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_load_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_mm_man_load_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, MM_MAN_LOAD_NODE_TYPE(mm_man_load_node), out_buff, out_buff_max_len, position);
    cmpi_encode_real(comm, &(MM_MAN_LOAD_NODE_MAX(mm_man_load_node)), out_buff, out_buff_max_len, position);
    cmpi_encode_real(comm, &(MM_MAN_LOAD_NODE_CUR(mm_man_load_node)), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_mm_man_load_node_size(const UINT32 comm, const MM_MAN_LOAD_NODE *mm_man_load_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, MM_MAN_LOAD_NODE_TYPE(mm_man_load_node), size);
    cmpi_encode_real_size(comm, &(MM_MAN_LOAD_NODE_MAX(mm_man_load_node)), size);
    cmpi_encode_real_size(comm, &(MM_MAN_LOAD_NODE_CUR(mm_man_load_node)), size);

    return (0);
}

UINT32 cmpi_decode_mm_man_load_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MM_MAN_LOAD_NODE *mm_man_load_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_load_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_load_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == mm_man_load_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_mm_man_load_node: mm_man_load_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MM_MAN_LOAD_NODE_TYPE(mm_man_load_node)));
    cmpi_decode_real(comm, in_buff, in_buff_max_len, position, &(MM_MAN_LOAD_NODE_MAX(mm_man_load_node)));
    cmpi_decode_real(comm, in_buff, in_buff_max_len, position, &(MM_MAN_LOAD_NODE_CUR(mm_man_load_node)));

    return (0);
}

UINT32 cmpi_encode_xmod_node(const UINT32 comm, const MOD_NODE *mod_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    cmpi_encode_uint32(comm, MOD_NODE_TCID(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_COMM(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_RANK(mod_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, MOD_NODE_MODI(mod_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_xmod_node_size(const UINT32 comm, const MOD_NODE *mod_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, MOD_NODE_TCID(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_COMM(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_RANK(mod_node), size);
    cmpi_encode_uint32_size(comm, MOD_NODE_MODI(mod_node), size);

    return (0);
}

UINT32 cmpi_decode_xmod_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, MOD_NODE *mod_node)
{
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_TCID(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_COMM(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_RANK(mod_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(MOD_NODE_MODI(mod_node)));

    return (0);
}

UINT32 cmpi_encode_cproc_module_stat(const UINT32 comm, const CPROC_MODULE_STAT *cproc_module_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cproc_module_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cproc_module_stat: cproc_module_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cproc_module_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cproc_module_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CPROC_MODULE_TYPE(cproc_module_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CPROC_MODULE_NUM(cproc_module_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cproc_module_stat_size(const UINT32 comm, const CPROC_MODULE_STAT *cproc_module_stat, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CPROC_MODULE_TYPE(cproc_module_stat), size);
    cmpi_encode_uint32_size(comm, CPROC_MODULE_NUM(cproc_module_stat), size);

    return (0);
}

UINT32 cmpi_decode_cproc_module_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CPROC_MODULE_STAT *cproc_module_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cproc_module_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cproc_module_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cproc_module_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cproc_module_stat: cproc_module_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CPROC_MODULE_TYPE(cproc_module_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CPROC_MODULE_NUM(cproc_module_stat)));

    return (0);
}

UINT32 cmpi_encode_crank_thread_stat(const UINT32 comm, const CRANK_THREAD_STAT *crank_thread_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crank_thread_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crank_thread_stat: crank_thread_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crank_thread_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crank_thread_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CRANK_THREAD_MAX_NUM(crank_thread_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRANK_THREAD_BUSY_NUM(crank_thread_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRANK_THREAD_IDLE_NUM(crank_thread_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_crank_thread_stat_size(const UINT32 comm, const CRANK_THREAD_STAT *crank_thread_stat, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CRANK_THREAD_MAX_NUM(crank_thread_stat), size);
    cmpi_encode_uint32_size(comm, CRANK_THREAD_BUSY_NUM(crank_thread_stat), size);
    cmpi_encode_uint32_size(comm, CRANK_THREAD_IDLE_NUM(crank_thread_stat), size);

    return (0);
}

UINT32 cmpi_decode_crank_thread_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRANK_THREAD_STAT *crank_thread_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crank_thread_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crank_thread_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crank_thread_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crank_thread_stat: crank_thread_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CRANK_THREAD_MAX_NUM(crank_thread_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CRANK_THREAD_BUSY_NUM(crank_thread_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CRANK_THREAD_IDLE_NUM(crank_thread_stat)));

    return (0);
}

UINT32 cmpi_encode_csys_eth_stat(const UINT32 comm, const CSYS_ETH_STAT *csys_eth_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csys_eth_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_eth_stat: csys_eth_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_eth_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_eth_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSYS_ETH_NAME(csys_eth_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_ETH_SPEEDMBS(csys_eth_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_ETH_RXMOCT(csys_eth_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_ETH_TXMOCT(csys_eth_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_ETH_RXTHROUGHPUT(csys_eth_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_ETH_TXTHROUGHPUT(csys_eth_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csys_eth_stat_size(const UINT32 comm, const CSYS_ETH_STAT *csys_eth_stat, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSYS_ETH_NAME(csys_eth_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_ETH_SPEEDMBS(csys_eth_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_ETH_RXMOCT(csys_eth_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_ETH_TXMOCT(csys_eth_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_ETH_RXTHROUGHPUT(csys_eth_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_ETH_TXTHROUGHPUT(csys_eth_stat), size);

    return (0);
}

UINT32 cmpi_decode_csys_eth_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_ETH_STAT *csys_eth_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_eth_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_eth_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csys_eth_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_eth_stat: csys_eth_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSYS_ETH_NAME(csys_eth_stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_ETH_SPEEDMBS(csys_eth_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_ETH_RXMOCT(csys_eth_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_ETH_TXMOCT(csys_eth_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_ETH_RXTHROUGHPUT(csys_eth_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_ETH_TXTHROUGHPUT(csys_eth_stat)));

    return (0);
}

UINT32 cmpi_encode_csys_dsk_stat(const UINT32 comm, const CSYS_DSK_STAT *csys_dsk_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csys_dsk_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_dsk_stat: csys_dsk_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_dsk_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csys_dsk_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSYS_DSK_NAME(csys_dsk_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_DSK_SIZE(csys_dsk_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_DSK_USED(csys_dsk_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSYS_DSK_AVAL(csys_dsk_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_real(comm, &(CSYS_DSK_LOAD(csys_dsk_stat)), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csys_dsk_stat_size(const UINT32 comm, const CSYS_DSK_STAT *csys_dsk_stat, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSYS_DSK_NAME(csys_dsk_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_DSK_SIZE(csys_dsk_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_DSK_USED(csys_dsk_stat), size);
    cmpi_encode_uint32_size(comm, CSYS_DSK_AVAL(csys_dsk_stat), size);
    cmpi_encode_real_size(comm, &(CSYS_DSK_LOAD(csys_dsk_stat)), size);

    return (0);
}

UINT32 cmpi_decode_csys_dsk_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSYS_DSK_STAT *csys_dsk_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_dsk_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_dsk_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csys_dsk_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csys_dsk_stat: csys_dsk_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSYS_DSK_NAME(csys_dsk_stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_DSK_SIZE(csys_dsk_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_DSK_USED(csys_dsk_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CSYS_DSK_AVAL(csys_dsk_stat)));
    cmpi_decode_real(comm, in_buff, in_buff_max_len, position, &(CSYS_DSK_LOAD(csys_dsk_stat)));

    return (0);
}

UINT32 cmpi_encode_task_time_fmt(const UINT32 comm, const TASK_TIME_FMT *task_time_fmt, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == task_time_fmt )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_time_fmt: task_time_fmt is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_time_fmt: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_time_fmt: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, TASK_TIME_FMT_YEAR(task_time_fmt), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_TIME_FMT_MONTH(task_time_fmt), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_TIME_FMT_MDAY(task_time_fmt), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_TIME_FMT_HOUR(task_time_fmt), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_TIME_FMT_MIN(task_time_fmt), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_TIME_FMT_SEC(task_time_fmt), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_task_time_fmt_size(const UINT32 comm, const TASK_TIME_FMT *task_time_fmt, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_YEAR(task_time_fmt), size);
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_MONTH(task_time_fmt), size);
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_MDAY(task_time_fmt), size);
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_HOUR(task_time_fmt), size);
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_MIN(task_time_fmt), size);
    cmpi_encode_uint32_size(comm, TASK_TIME_FMT_SEC(task_time_fmt), size);

    return (0);
}

UINT32 cmpi_decode_task_time_fmt(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASK_TIME_FMT *task_time_fmt)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_time_fmt: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_time_fmt: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == task_time_fmt )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_time_fmt: task_time_fmt is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_YEAR(task_time_fmt)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_MONTH(task_time_fmt)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_MDAY(task_time_fmt)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_HOUR(task_time_fmt)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_MIN(task_time_fmt)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_TIME_FMT_SEC(task_time_fmt)));

    return (0);
}


UINT32 cmpi_encode_task_report_node(const UINT32 comm, const TASK_REPORT_NODE *task_report_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == task_report_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_report_node: task_report_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_report_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_task_report_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_task_time_fmt(comm, TASK_REPORT_NODE_START_TIME(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_task_time_fmt(comm, TASK_REPORT_NODE_END_TIME(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_TCID(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_RANK(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_SEQNO(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_WAIT_FLAG(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_task_report_node_size(const UINT32 comm, const TASK_REPORT_NODE *task_report_node, UINT32 *size)
{
    cmpi_encode_task_time_fmt_size(comm, TASK_REPORT_NODE_START_TIME(task_report_node), size);
    cmpi_encode_task_time_fmt_size(comm, TASK_REPORT_NODE_END_TIME(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_TCID(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_RANK(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_SEQNO(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_WAIT_FLAG(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node), size);
    cmpi_encode_uint32_size(comm, TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node), size);
    return (0);
}

UINT32 cmpi_decode_task_report_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, TASK_REPORT_NODE *task_report_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_report_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_report_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == task_report_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_task_report_node: task_report_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_task_time_fmt(comm, in_buff, in_buff_max_len, position, TASK_REPORT_NODE_START_TIME(task_report_node));
    cmpi_decode_task_time_fmt(comm, in_buff, in_buff_max_len, position, TASK_REPORT_NODE_END_TIME(task_report_node));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_TCID(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_RANK(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_SEQNO(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_TIME_TO_LIVE(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_WAIT_FLAG(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_NEED_RSP_FLAG(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_NEED_RESCHEDULE_FLAG(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_TOTAL_REQ_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_SENT_REQ_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_DISCARD_REQ_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_TIMEOUT_REQ_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_NEED_RSP_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_SUCC_RSP_NUM(task_report_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(TASK_REPORT_NODE_FAIL_RSP_NUM(task_report_node)));
    return (0);
}

UINT32 cmpi_encode_cdfsnp_inode(const UINT32 comm, const CDFSNP_INODE *cdfsnp_inode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_inode: cdfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_inode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CDFSNP_INODE_TCID(cdfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_INODE_PATH(cdfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_INODE_FOFF(cdfsnp_inode), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cdfsnp_inode_size(const UINT32 comm, const CDFSNP_INODE *cdfsnp_inode, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CDFSNP_INODE_TCID(cdfsnp_inode), size);
    cmpi_encode_uint32_size(comm, CDFSNP_INODE_PATH(cdfsnp_inode), size);
    cmpi_encode_uint32_size(comm, CDFSNP_INODE_FOFF(cdfsnp_inode), size);

    return (0);
}

UINT32 cmpi_decode_cdfsnp_inode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_INODE *cdfsnp_inode)
{
    UINT32 path_layout;
    UINT32 file_offset;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_inode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_inode: cdfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CDFSNP_INODE_TCID(cdfsnp_inode)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(path_layout));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_offset));

    CDFSNP_INODE_PATH(cdfsnp_inode) = (path_layout & CDFSNP_32BIT_MASK);
    CDFSNP_INODE_FOFF(cdfsnp_inode) = (file_offset & CDFSNP_32BIT_MASK);

    return (0);
}


UINT32 cmpi_encode_cdfsnp_fnode(const UINT32 comm, const CDFSNP_FNODE *cdfsnp_fnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 cdfsnp_inode_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_fnode: cdfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_fnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CDFSNP_FNODE_FILESZ(cdfsnp_fnode) & CDFSNP_32BIT_MASK, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_FNODE_REPNUM(cdfsnp_fnode) & CDFSNP_32BIT_MASK, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_FNODE_TRUNCF(cdfsnp_fnode) & CDFSNP_32BIT_MASK, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_FNODE_ACTFSZ(cdfsnp_fnode) & CDFSNP_32BIT_MASK, out_buff, out_buff_max_len, position);

    for(cdfsnp_inode_pos = 0; cdfsnp_inode_pos < (CDFSNP_FNODE_REPNUM(cdfsnp_fnode) & CDFSNP_32BIT_MASK) && cdfsnp_inode_pos < CDFSNP_FILE_REPLICA_MAX_NUM; cdfsnp_inode_pos ++)
    {
        CDFSNP_INODE *cdfsnp_inode;

        cdfsnp_inode = (CDFSNP_INODE *)CDFSNP_FNODE_INODE(cdfsnp_fnode, cdfsnp_inode_pos);
        cmpi_encode_cdfsnp_inode(comm, cdfsnp_inode, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_cdfsnp_fnode_size(const UINT32 comm, const CDFSNP_FNODE *cdfsnp_fnode, UINT32 *size)
{
    UINT32 cdfsnp_inode_pos;

    cmpi_encode_uint32_size(comm, CDFSNP_FNODE_FILESZ(cdfsnp_fnode), size);
    cmpi_encode_uint32_size(comm, CDFSNP_FNODE_REPNUM(cdfsnp_fnode), size);
    cmpi_encode_uint32_size(comm, CDFSNP_FNODE_TRUNCF(cdfsnp_fnode), size);
    cmpi_encode_uint32_size(comm, CDFSNP_FNODE_ACTFSZ(cdfsnp_fnode), size);
    for(cdfsnp_inode_pos = 0; cdfsnp_inode_pos < CDFSNP_FNODE_REPNUM(cdfsnp_fnode) && cdfsnp_inode_pos < CDFSNP_FILE_REPLICA_MAX_NUM; cdfsnp_inode_pos ++)
    {
        CDFSNP_INODE *cdfsnp_inode;

        cdfsnp_inode = (CDFSNP_INODE *)CDFSNP_FNODE_INODE(cdfsnp_fnode, cdfsnp_inode_pos);
        cmpi_encode_cdfsnp_inode_size(comm, cdfsnp_inode, size);
    }

    return (0);
}

UINT32 cmpi_decode_cdfsnp_fnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_FNODE *cdfsnp_fnode)
{
    UINT32 cdfsnp_inode_pos;
    UINT32 file_size;
    UINT32 replica_num;
    UINT32 trunc_flag;
    UINT32 actual_fsize;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_fnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_fnode: cdfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_size));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(replica_num));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(trunc_flag));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(actual_fsize));

    if(CDFSNP_FILE_REPLICA_MAX_NUM < (replica_num & CDFSNP_32BIT_MASK))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cdfsnp_fnode: replica num %ld overflow\n", replica_num);
        return ((UINT32)-1);
    }

    CDFSNP_FNODE_FILESZ(cdfsnp_fnode) = (file_size & CDFSNP_32BIT_MASK);
    CDFSNP_FNODE_REPNUM(cdfsnp_fnode) = (replica_num & CDFSNP_32BIT_MASK);
    CDFSNP_FNODE_TRUNCF(cdfsnp_fnode) = (trunc_flag & CDFSNP_32BIT_MASK);
    CDFSNP_FNODE_ACTFSZ(cdfsnp_fnode) = (actual_fsize & CDFSNP_32BIT_MASK);

    for(cdfsnp_inode_pos = 0; cdfsnp_inode_pos < CDFSNP_FNODE_REPNUM(cdfsnp_fnode); cdfsnp_inode_pos ++)
    {
        CDFSNP_INODE *cdfsnp_inode;

        cdfsnp_inode = CDFSNP_FNODE_INODE(cdfsnp_fnode, cdfsnp_inode_pos);
        cmpi_decode_cdfsnp_inode(comm, in_buff, in_buff_max_len, position, cdfsnp_inode);
    }

    return (0);
}

UINT32 cmpi_encode_cdfsnp_item(const UINT32 comm, const CDFSNP_ITEM *cdfsnp_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_item: cdfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_item: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CDFSNP_ITEM_DFLG(cdfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_ITEM_STAT(cdfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_ITEM_KLEN(cdfsnp_item), out_buff, out_buff_max_len, position);

    cmpi_pack(CDFSNP_ITEM_KEY(cdfsnp_item), CDFSNP_ITEM_KLEN(cdfsnp_item), CMPI_UCHAR, out_buff, out_buff_max_len, position, comm);

    cmpi_encode_uint32(comm, CDFSNP_ITEM_PARENT(cdfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSNP_ITEM_SHASH_NEXT(cdfsnp_item), out_buff, out_buff_max_len, position);

    if(CDFSNP_ITEM_FILE_IS_DIR == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        cmpi_encode_uint32(comm, CDFSNP_DNODE_FILE_NUM(CDFSNP_ITEM_DNODE(cdfsnp_item)), out_buff, out_buff_max_len, position);
    }

    if(CDFSNP_ITEM_FILE_IS_REG == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        cmpi_encode_cdfsnp_fnode(comm, CDFSNP_ITEM_FNODE(cdfsnp_item), out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_cdfsnp_item_size(const UINT32 comm, const CDFSNP_ITEM *cdfsnp_item, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CDFSNP_ITEM_DFLG(cdfsnp_item), size);
    cmpi_encode_uint32_size(comm, CDFSNP_ITEM_STAT(cdfsnp_item), size);
    cmpi_encode_uint32_size(comm, CDFSNP_ITEM_KLEN(cdfsnp_item), size);

    cmpi_pack_size(CDFSNP_ITEM_KLEN(cdfsnp_item), CMPI_UCHAR, size,  comm);

    cmpi_encode_uint32_size(comm, CDFSNP_ITEM_PARENT(cdfsnp_item), size);
    cmpi_encode_uint32_size(comm, CDFSNP_ITEM_SHASH_NEXT(cdfsnp_item), size);

    if(CDFSNP_ITEM_FILE_IS_DIR == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        cmpi_encode_uint32_size(comm, CDFSNP_DNODE_FILE_NUM(CDFSNP_ITEM_DNODE(cdfsnp_item)), size);
    }

    if(CDFSNP_ITEM_FILE_IS_REG == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        cmpi_encode_cdfsnp_fnode_size(comm, CDFSNP_ITEM_FNODE(cdfsnp_item), size);
    }

    return (0);
}

UINT32 cmpi_decode_cdfsnp_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSNP_ITEM *cdfsnp_item)
{
    UINT32 dflag;
    UINT32 stat;
    UINT32 klen;

    UINT32 parent_offset;
    UINT32 shash_next_offset;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_item: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsnp_item: cdfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(dflag));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(klen));

    CDFSNP_ITEM_DFLG(cdfsnp_item) = (dflag & CDFSNP_32BIT_MASK);
    CDFSNP_ITEM_STAT(cdfsnp_item) = (stat  & CDFSNP_32BIT_MASK);
    CDFSNP_ITEM_KLEN(cdfsnp_item) = (klen  & CDFSNP_32BIT_MASK);

    cmpi_unpack(in_buff, in_buff_max_len, position, CDFSNP_ITEM_KEY(cdfsnp_item), CDFSNP_ITEM_KLEN(cdfsnp_item), CMPI_UCHAR, comm);

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(parent_offset));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(shash_next_offset));

    CDFSNP_ITEM_PARENT(cdfsnp_item)     = (parent_offset     & CDFSNP_32BIT_MASK);
    CDFSNP_ITEM_SHASH_NEXT(cdfsnp_item) = (shash_next_offset & CDFSNP_32BIT_MASK);

    if(CDFSNP_ITEM_FILE_IS_DIR == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        UINT32 file_num;
        cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_num));
        CDFSNP_DNODE_FILE_NUM(CDFSNP_ITEM_DNODE(cdfsnp_item)) = (file_num & CDFSNP_32BIT_MASK);
    }

    if(CDFSNP_ITEM_FILE_IS_REG == CDFSNP_ITEM_DFLG(cdfsnp_item))
    {
        cmpi_decode_cdfsnp_fnode(comm, in_buff, in_buff_max_len, position, CDFSNP_ITEM_FNODE(cdfsnp_item));
    }

    return (0);
}

UINT32 cmpi_encode_cdfsdn_stat(const UINT32 comm, const CDFSDN_STAT *cdfsdn_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    //UINT32 pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsdn_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_stat: cdfsdn_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CDFSDN_STAT_TCID(cdfsdn_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSDN_STAT_FULL(cdfsdn_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cdfsdn_stat_size(const UINT32 comm, const CDFSDN_STAT *cdfsdn_stat, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CDFSDN_STAT_TCID(cdfsdn_stat), size);
    cmpi_encode_uint32_size(comm, CDFSDN_STAT_FULL(cdfsdn_stat), size);
    return (0);
}

UINT32 cmpi_decode_cdfsdn_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_STAT *cdfsdn_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsdn_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_stat: cdfsdn_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CDFSDN_STAT_TCID(cdfsdn_stat)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CDFSDN_STAT_FULL(cdfsdn_stat)));

    return (0);
}

UINT32 cmpi_encode_cload_stat(const UINT32 comm, const CLOAD_STAT *cload_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cload_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_stat: cload_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint16(comm, CLOAD_STAT_QUE_LOAD(cload_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint16(comm, CLOAD_STAT_OBJ_LOAD(cload_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint8(comm, CLOAD_STAT_CPU_LOAD(cload_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint8(comm, CLOAD_STAT_MEM_LOAD(cload_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint8(comm, CLOAD_STAT_DSK_LOAD(cload_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint8(comm, CLOAD_STAT_NET_LOAD(cload_stat), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cload_stat_size(const UINT32 comm, const CLOAD_STAT *cload_stat, UINT32 *size)
{
    cmpi_encode_uint16_size(comm, CLOAD_STAT_QUE_LOAD(cload_stat), size);
    cmpi_encode_uint16_size(comm, CLOAD_STAT_OBJ_LOAD(cload_stat), size);
    cmpi_encode_uint8_size(comm, CLOAD_STAT_CPU_LOAD(cload_stat), size);
    cmpi_encode_uint8_size(comm, CLOAD_STAT_MEM_LOAD(cload_stat), size);
    cmpi_encode_uint8_size(comm, CLOAD_STAT_DSK_LOAD(cload_stat), size);
    cmpi_encode_uint8_size(comm, CLOAD_STAT_NET_LOAD(cload_stat), size);
    return (0);
}

UINT32 cmpi_decode_cload_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_STAT *cload_stat)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cload_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_stat: cload_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_QUE_LOAD(cload_stat)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_OBJ_LOAD(cload_stat)));
    cmpi_decode_uint8(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_CPU_LOAD(cload_stat)));
    cmpi_decode_uint8(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_MEM_LOAD(cload_stat)));
    cmpi_decode_uint8(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_DSK_LOAD(cload_stat)));
    cmpi_decode_uint8(comm, in_buff, in_buff_max_len, position, &(CLOAD_STAT_NET_LOAD(cload_stat)));

    return (0);
}


UINT32 cmpi_encode_cload_node(const UINT32 comm, const CLOAD_NODE *cload_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cload_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_node: cload_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CLOAD_NODE_TCID(cload_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CLOAD_NODE_COMM(cload_node), out_buff, out_buff_max_len, position);
    cmpi_encode_cvector(comm, CLOAD_NODE_RANK_LOAD_STAT_VEC(cload_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cload_node_size(const UINT32 comm, const CLOAD_NODE *cload_node, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CLOAD_NODE_TCID(cload_node), size);
    cmpi_encode_uint32_size(comm, CLOAD_NODE_COMM(cload_node), size);
    cmpi_encode_cvector_size(comm, CLOAD_NODE_RANK_LOAD_STAT_VEC(cload_node), size);
    return (0);
}

UINT32 cmpi_decode_cload_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_NODE *cload_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cload_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_node: cload_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CLOAD_NODE_TCID(cload_node)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CLOAD_NODE_COMM(cload_node)));
    cmpi_decode_cvector(comm, in_buff, in_buff_max_len, position,  (CLOAD_NODE_RANK_LOAD_STAT_VEC(cload_node)));

    return (0);
}

UINT32 cmpi_encode_cload_mgr(const UINT32 comm, const CLOAD_MGR *cload_mgr, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    CLIST_DATA *clist_data;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cload_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_mgr: cload_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_mgr: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cload_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, clist_size(cload_mgr), out_buff, out_buff_max_len, position);

    CLIST_LOCK(cload_mgr, LOC_CMPIE_0017);
    CLIST_LOOP_NEXT(cload_mgr, clist_data)
    {
        CLOAD_NODE *cload_node;
        cload_node = (CLOAD_NODE *)CLIST_DATA_DATA(clist_data);
        cmpi_encode_cload_node(comm, cload_node, out_buff, out_buff_max_len, position);
    }
    CLIST_UNLOCK(cload_mgr, LOC_CMPIE_0018);
    return (0);
}

UINT32 cmpi_encode_cload_mgr_size(const UINT32 comm, const CLOAD_MGR *cload_mgr, UINT32 *size)
{
    CLIST_DATA *clist_data;

    cmpi_encode_uint32_size(comm, clist_size(cload_mgr), size);

    CLIST_LOCK(cload_mgr, LOC_CMPIE_0019);
    CLIST_LOOP_NEXT(cload_mgr, clist_data)
    {
        CLOAD_NODE *cload_node;
        cload_node = (CLOAD_NODE *)CLIST_DATA_DATA(clist_data);
        cmpi_encode_cload_node_size(comm, cload_node, size);
    }
    CLIST_UNLOCK(cload_mgr, LOC_CMPIE_0020);
    return (0);
}

UINT32 cmpi_decode_cload_mgr(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLOAD_MGR *cload_mgr)
{
    UINT32 pos;
    UINT32 size;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_mgr: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_mgr: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cload_mgr )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cload_mgr: cload_mgr is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(size));
    for(pos = 0; pos < size; pos ++)
    {
        CLOAD_NODE *cload_node;
        cload_node = cload_node_new(CMPI_ERROR_TCID, CMPI_ERROR_COMM, 0);
        cmpi_decode_cload_node(comm, in_buff, in_buff_max_len, position, cload_node);
        clist_push_back(cload_mgr, (void *)cload_node);
    }

    return (0);
}

UINT32 cmpi_encode_cfuse_mode(const UINT32 comm, const CFUSE_MODE *cfuse_mode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 st_mode;
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cfuse_mode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_mode: cfuse_mode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_mode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_mode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    st_mode = 0;
    st_mode = ((st_mode << 1) | (cfuse_mode->suid));
    st_mode = ((st_mode << 1) | (cfuse_mode->sgid));
    st_mode = ((st_mode << 1) | (cfuse_mode->sticky));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.exec));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.exec));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.exec));

    cmpi_encode_uint32(comm, st_mode, out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cfuse_mode_size(const UINT32 comm, const CFUSE_MODE *cfuse_mode, UINT32 *size)
{
    UINT32 st_mode;
    st_mode = 0;
    st_mode = ((st_mode << 1) | (cfuse_mode->suid));
    st_mode = ((st_mode << 1) | (cfuse_mode->sgid));
    st_mode = ((st_mode << 1) | (cfuse_mode->sticky));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->owner.exec));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->group.exec));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.read));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.write));
    st_mode = ((st_mode << 1) | (cfuse_mode->other.exec));

    cmpi_encode_uint32_size(comm, st_mode, size);
    return (0);
}

UINT32 cmpi_decode_cfuse_mode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFUSE_MODE *cfuse_mode)
{
    UINT32 st_mode;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_mode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_mode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cfuse_mode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_mode: cfuse_mode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(st_mode));

    cfuse_mode->other.exec = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->other.write = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->other.read = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->group.exec = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->group.write = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->group.read = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->owner.exec = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->owner.write = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->owner.read = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->sticky = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->sgid = (st_mode & 1);
    st_mode >>= 1;

    cfuse_mode->suid = (st_mode & 1);
    st_mode >>= 1;

    return (0);
}

UINT32 cmpi_encode_cfuse_stat(const UINT32 comm, const CFUSE_STAT *cfuse_stat, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cfuse_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_stat: cfuse_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_stat: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cfuse_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, MINOR(CFUSE_STAT_INO(cfuse_stat)), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CFUSE_STAT_TYPE(cfuse_stat), out_buff, out_buff_max_len, position);

    cmpi_encode_cfuse_mode(comm, CFUSE_STAT_MODE(cfuse_stat), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CFUSE_STAT_SIZE(cfuse_stat), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cfuse_stat_size(const UINT32 comm, const CFUSE_STAT *cfuse_stat, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, MINOR(CFUSE_STAT_INO(cfuse_stat)), size);
    cmpi_encode_uint32_size(comm, CFUSE_STAT_TYPE(cfuse_stat), size);

    cmpi_encode_cfuse_mode_size(comm, CFUSE_STAT_MODE(cfuse_stat), size);
    cmpi_encode_uint32_size(comm, CFUSE_STAT_SIZE(cfuse_stat), size);
    return (0);
}

UINT32 cmpi_decode_cfuse_stat(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CFUSE_STAT *cfuse_stat)
{
    UINT32 st_ino_minor;
    UINT32 st_type;
    UINT32 st_size;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_stat: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_stat: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cfuse_stat )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cfuse_stat: cfuse_stat is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(st_ino_minor));
    CFUSE_STAT_INO(cfuse_stat) = st_ino_minor;

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(st_type));
    CFUSE_STAT_TYPE(cfuse_stat) = st_type;

    cmpi_decode_cfuse_mode(comm, in_buff, in_buff_max_len, position, CFUSE_STAT_MODE(cfuse_stat));

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(st_size));
    CFUSE_STAT_SIZE(cfuse_stat) = st_size;
    return (0);
}

UINT32 cmpi_encode_cdfsdn_record(const UINT32 comm, const CDFSDN_RECORD *cdfsdn_record, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsdn_record )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_record: cdfsdn_record is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_record: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_record: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CDFSDN_RECORD_SIZE(cdfsdn_record), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CDFSDN_RECORD_FIRST_PART_IDX(cdfsdn_record), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cdfsdn_record_size(const UINT32 comm, const CDFSDN_RECORD *cdfsdn_record, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CDFSDN_RECORD_SIZE(cdfsdn_record), size);
    cmpi_encode_uint32_size(comm, CDFSDN_RECORD_FIRST_PART_IDX(cdfsdn_record), size);
    return (0);
}

UINT32 cmpi_decode_cdfsdn_record(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_RECORD *cdfsdn_record)
{
    UINT32 size;
    UINT32 first_part_idx;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_record: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_record: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsdn_record )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_record: cdfsdn_record is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(size));
    CDFSDN_RECORD_SIZE(cdfsdn_record) = size;

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(first_part_idx));
    CDFSDN_RECORD_FIRST_PART_IDX(cdfsdn_record) = first_part_idx;
    return (0);
}

UINT32 cmpi_encode_cdfsdn_block(const UINT32 comm, const CDFSDN_BLOCK *cdfsdn_block, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cdfsdn_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_block: cdfsdn_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_block: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cdfsdn_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint8_array(comm, (UINT8 *)CDFSDN_BLOCK_CACHE(cdfsdn_block), CDFSDN_BLOCK_MAX_SIZE, out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cdfsdn_block_size(const UINT32 comm, const CDFSDN_BLOCK *cdfsdn_block, UINT32 *size)
{
    cmpi_encode_uint8_array_size(comm, (UINT8 *)CDFSDN_BLOCK_CACHE(cdfsdn_block), CDFSDN_BLOCK_MAX_SIZE, size);
    return (0);
}

UINT32 cmpi_decode_cdfsdn_block(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CDFSDN_BLOCK *cdfsdn_block)
{
    UINT32 size;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_block: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_block: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cdfsdn_block )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cdfsdn_block: cdfsdn_block is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    CDFSDN_BLOCK_CACHE(cdfsdn_block) = cdfsdn_cache_new();
    if(NULL_PTR == CDFSDN_BLOCK_CACHE(cdfsdn_block))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cdfsdn_block: block cache is null\n");
        return ((UINT32)-1);
    }

    cmpi_decode_uint8_array(comm, in_buff, in_buff_max_len, position, (UINT8 *)CDFSDN_BLOCK_CACHE(cdfsdn_block), &(size));
    return (0);
}

UINT32 cmpi_encode_cbytes(const UINT32 comm, const CBYTES *cbytes, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
/*
    if ( NULL_PTR == cbytes )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbytes: cbytes is null.\n");
        dbg_exit(MD_TBD, 0);
    }
*/    
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbytes: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbytes: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    if(NULL_PTR == cbytes)
    {
        cmpi_encode_uint32(comm, 0, out_buff, out_buff_max_len, position);
        return (0);
    }

    cmpi_encode_uint8_array(comm, CBYTES_BUF(cbytes), CBYTES_LEN(cbytes), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_cbytes_size(const UINT32 comm, const CBYTES *cbytes, UINT32 *size)
{
    if(NULL_PTR == cbytes)
    {
        cmpi_encode_uint32_size(comm, 0, size);
        return (0);
    }

    cmpi_encode_uint8_array_size(comm, NULL_PTR, CBYTES_LEN(cbytes), size);
    return (0);
}

UINT32 cmpi_decode_cbytes(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CBYTES *cbytes)
{
    UINT32 len;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbytes: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbytes: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cbytes )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbytes: cbytes is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &len);
    if(0 == len)
    {
        CBYTES_LEN(cbytes) = len;
        /*nothing to do*/
        return (0);
    }

    if(NULL_PTR == CBYTES_BUF(cbytes))
    {
        //dbg_log(SEC_0035_CMPIE, 1)(LOGSTDOUT, "warn:cmpi_decode_cbytes: len %ld but buff is null\n", len);
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(len, LOC_CMPIE_0021);
        CBYTES_LEN(cbytes) = len;
    }
    else
    {
        if(CBYTES_LEN(cbytes) < len)
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_cbytes: buff room is %ld bytes, no enough memory to accept %ld bytes\n", CBYTES_LEN(cbytes), len);
            return ((UINT32)-1);
        }
        CBYTES_LEN(cbytes) = len;
    }

    cmpi_unpack(in_buff, in_buff_max_len, position, CBYTES_BUF(cbytes), len, CMPI_UCHAR, comm);
    return (0);
}

UINT32 cmpi_encode_ctimet(const UINT32 comm, const CTIMET *ctimet, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    char time_buf[64];
    CSTRING cstring;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == ctimet )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ctimet: ctimet is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ctimet: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_ctimet: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", c_localtime_r(ctimet));
    cstring_set_str(&cstring, (UINT8 *)time_buf);
    cmpi_encode_cstring(comm, &cstring, out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_ctimet_size(const UINT32 comm, const CTIMET *ctimet, UINT32 *size)
{
    char time_buf[64];
    CSTRING cstring;

    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", c_localtime_r(ctimet));
    cstring_set_str(&cstring, (UINT8 *)time_buf);
    cmpi_encode_cstring_size(comm, &cstring, size);
    return (0);
}

UINT32 cmpi_decode_ctimet(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CTIMET *ctimet)
{
    CSTRING cstring;
    struct tm tm_time;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ctimet: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ctimet: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == ctimet )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_ctimet: ctimet is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cstring_init(&cstring, NULL_PTR);

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, &cstring);
    strptime((char *)cstring_get_str(&cstring), "%Y-%m-%d %H:%M:%S", &tm_time);
    (*ctimet) = mktime(&tm_time);
    cstring_clean(&cstring);
    return (0);
}

UINT32 cmpi_encode_csession_node(const UINT32 comm, const CSESSION_NODE *csession_node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csession_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_node: csession_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_node: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSESSION_NODE_NAME(csession_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSESSION_NODE_ID(csession_node), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSESSION_NODE_EXPIRE_NSEC(csession_node), out_buff, out_buff_max_len, position);
    cmpi_encode_ctimet(comm, &CSESSION_NODE_CREATE_TIME(csession_node), out_buff, out_buff_max_len, position);
    cmpi_encode_ctimet(comm, &CSESSION_NODE_ACCESS_TIME(csession_node), out_buff, out_buff_max_len, position);
    cmpi_encode_clist(comm, CSESSION_NODE_CACHE_TREE(csession_node), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csession_node_size(const UINT32 comm, const CSESSION_NODE *csession_node, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSESSION_NODE_NAME(csession_node), size);
    cmpi_encode_uint32_size(comm, CSESSION_NODE_ID(csession_node), size);
    cmpi_encode_uint32_size(comm, CSESSION_NODE_EXPIRE_NSEC(csession_node), size);
    cmpi_encode_ctimet_size(comm, &CSESSION_NODE_CREATE_TIME(csession_node), size);
    cmpi_encode_ctimet_size(comm, &CSESSION_NODE_ACCESS_TIME(csession_node), size);
    cmpi_encode_clist_size(comm, CSESSION_NODE_CACHE_TREE(csession_node), size);


    return (0);
}

UINT32 cmpi_decode_csession_node(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSESSION_NODE *csession_node)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_node: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_node: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csession_node )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_node: csession_node is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSESSION_NODE_NAME(csession_node));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &CSESSION_NODE_ID(csession_node));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &CSESSION_NODE_EXPIRE_NSEC(csession_node));
    cmpi_decode_ctimet(comm, in_buff, in_buff_max_len, position, &CSESSION_NODE_CREATE_TIME(csession_node));
    cmpi_decode_ctimet(comm, in_buff, in_buff_max_len, position, &CSESSION_NODE_ACCESS_TIME(csession_node));
    cmpi_decode_clist(comm, in_buff, in_buff_max_len, position, CSESSION_NODE_CACHE_TREE(csession_node));

    return (0);
}

UINT32 cmpi_encode_csession_item(const UINT32 comm, const CSESSION_ITEM *csession_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csession_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_item: csession_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_item: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csession_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSESSION_ITEM_KEY(csession_item), out_buff, out_buff_max_len, position);
    cmpi_encode_cbytes(comm, CSESSION_ITEM_VAL(csession_item), out_buff, out_buff_max_len, position);
    cmpi_encode_clist(comm, CSESSION_ITEM_CHILDREN(csession_item), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_csession_item_size(const UINT32 comm, const CSESSION_ITEM *csession_item, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSESSION_ITEM_KEY(csession_item), size);
    cmpi_encode_cbytes_size(comm, CSESSION_ITEM_VAL(csession_item), size);
    cmpi_encode_clist_size(comm, CSESSION_ITEM_CHILDREN(csession_item), size);

    return (0);
}

UINT32 cmpi_decode_csession_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSESSION_ITEM *csession_item)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_item: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csession_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csession_item: csession_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSESSION_ITEM_KEY(csession_item));
    cmpi_decode_cbytes(comm, in_buff, in_buff_max_len, position, CSESSION_ITEM_VAL(csession_item));
    cmpi_decode_clist(comm, in_buff, in_buff_max_len, position, CSESSION_ITEM_CHILDREN(csession_item));

    return (0);
}

UINT32 cmpi_encode_csword(const UINT32 comm, const CSWORD *csword, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csword )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword: csword is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cbytes(comm, CSWORD_CONTENT(csword), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_csword_size(const UINT32 comm, const CSWORD *csword, UINT32 *size)
{
    cmpi_encode_cbytes_size(comm, CSWORD_CONTENT(csword), size);
    return (0);
}

UINT32 cmpi_decode_csword(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSWORD *csword)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csword )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword: csword is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cbytes(comm, in_buff, in_buff_max_len, position, CSWORD_CONTENT(csword));
    return (0);
}

UINT32 cmpi_encode_csdoc(const UINT32 comm, const CSDOC *csdoc, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csdoc )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc: csdoc is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_cstring(comm, CSDOC_NAME(csdoc), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSDOC_ID(csdoc), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSDOC_TYPE(csdoc), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSDOC_CODE(csdoc), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CSDOC_RATE(csdoc), out_buff, out_buff_max_len, position);
    cmpi_encode_cbytes(comm, CSDOC_CONTENT(csdoc), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_csdoc_size(const UINT32 comm, const CSDOC *csdoc, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CSDOC_NAME(csdoc), size);
    cmpi_encode_uint32_size(comm, CSDOC_ID(csdoc), size);
    cmpi_encode_uint32_size(comm, CSDOC_TYPE(csdoc), size);
    cmpi_encode_uint32_size(comm, CSDOC_CODE(csdoc), size);
    cmpi_encode_uint32_size(comm, CSDOC_RATE(csdoc), size);
    cmpi_encode_cbytes_size(comm, CSDOC_CONTENT(csdoc), size);
    return (0);
}

UINT32 cmpi_decode_csdoc(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSDOC *csdoc)
{
    UINT32 doc_id;
    UINT32 doc_type;
    UINT32 doc_code;
    UINT32 doc_rate;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csdoc )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc: csdoc is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CSDOC_NAME(csdoc));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(doc_id));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(doc_type));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(doc_code));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(doc_rate));
    cmpi_decode_cbytes(comm, in_buff, in_buff_max_len, position, CSDOC_CONTENT(csdoc));

    CSDOC_ID(csdoc)   = doc_id  ;
    CSDOC_TYPE(csdoc) = doc_type;
    CSDOC_CODE(csdoc) = doc_code;
    CSDOC_RATE(csdoc) = doc_rate;
    return (0);
}

UINT32 cmpi_encode_csdoc_words(const UINT32 comm, const CSDOC_WORDS *csdoc_words, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csdoc_words )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc_words: csdoc_words is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc_words: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csdoc_words: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_csdoc(comm, CSDOC_WORDS_DOC(csdoc_words), out_buff, out_buff_max_len, position);
    cmpi_encode_clist(comm, CSDOC_WORDS_LIST(csdoc_words), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_csdoc_words_size(const UINT32 comm, const CSDOC_WORDS *csdoc_words, UINT32 *size)
{
    cmpi_encode_csdoc_size(comm, CSDOC_WORDS_DOC(csdoc_words), size);
    cmpi_encode_clist_size(comm, CSDOC_WORDS_LIST(csdoc_words), size);
    return (0);
}

UINT32 cmpi_decode_csdoc_words(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSDOC_WORDS *csdoc_words)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc_words: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc_words: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csdoc_words )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csdoc_words: csdoc_words is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_csdoc(comm, in_buff, in_buff_max_len, position, CSDOC_WORDS_DOC(csdoc_words));
    cmpi_decode_clist(comm, in_buff, in_buff_max_len, position, CSDOC_WORDS_LIST(csdoc_words));
    return (0);
}

UINT32 cmpi_encode_csword_docs(const UINT32 comm, const CSWORD_DOCS *csword_docs, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == csword_docs )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword_docs: csword_docs is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword_docs: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_csword_docs: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_csword(comm, CSWORD_DOCS_WORD(csword_docs), out_buff, out_buff_max_len, position);
    cmpi_encode_clist(comm, CSWORD_DOCS_LIST(csword_docs), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_csword_docs_size(const UINT32 comm, const CSWORD_DOCS *csword_docs, UINT32 *size)
{
    cmpi_encode_csword_size(comm, CSWORD_DOCS_WORD(csword_docs), size);
    cmpi_encode_clist_size(comm, CSWORD_DOCS_LIST(csword_docs), size);
    return (0);
}

UINT32 cmpi_decode_csword_docs(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CSWORD_DOCS *csword_docs)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword_docs: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword_docs: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == csword_docs )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_csword_docs: csword_docs is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_csword(comm, in_buff, in_buff_max_len, position, CSWORD_DOCS_WORD(csword_docs));
    cmpi_decode_clist(comm, in_buff, in_buff_max_len, position, CSWORD_DOCS_LIST(csword_docs));
    return (0);
}


UINT32 cmpi_encode_clist(const UINT32 comm, const CLIST *clist, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 size;
    UINT32 num;
    UINT32 type;

    CLIST_DATA_ENCODER data_encoder;
    CLIST_DATA *clist_data;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == clist )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_clist: clist is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_clist: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_clist: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    type = clist_type(clist);
    size = clist_size(clist);

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_clist: clist %lx, type = %ld, size = %ld, position = %ld\n",
                        clist, type, size, *position));

    cmpi_encode_uint32(comm, type, out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, size, out_buff, out_buff_max_len, position);

    if(0 == size)
    {
        return (0);
    }

    data_encoder = (CLIST_DATA_ENCODER)clist_codec_get(clist, CLIST_CODEC_ENCODER);
    if(NULL_PTR == data_encoder)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_clist: clist data encoder is null\n");
        return ((UINT32)-1);
    }

    num = 0;
    CLIST_LOCK(clist, LOC_CMPIE_0022);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        void *data;
        data = CLIST_DATA_DATA(clist_data);
        data_encoder(comm, data, out_buff, out_buff_max_len, position);
        num ++;
    }
    CLIST_UNLOCK(clist, LOC_CMPIE_0023);

    /*check again*/
    if(size != num)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_clist: clist size = %ld but encoded item num = %ld\n", size, num);
        return ((UINT32)-1);
    }

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_clist: clist %lx, type = %ld, num = %ld ==> position = %ld\n",
                        clist, clist_type(clist), clist_size(clist), *position));

    return (0);
}

UINT32 cmpi_encode_clist_size(const UINT32 comm, const CLIST *clist, UINT32 *size)
{
    UINT32 num;
    UINT32 type;

    CLIST_DATA_ENCODER_SIZE data_encoder_size;

    type = clist_type(clist);
    num = clist_size(clist);

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_clist_size: clist %lx: type = %ld, num = %ld, size = %ld\n", clist, type, num, *size));

    if(MM_END == type)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_clist_size: clist %lx: invalid type = %ld, num = %ld\n", clist, type, num);
    }

    cmpi_encode_uint32_size(comm, type, size);
    cmpi_encode_uint32_size(comm, num, size);

    if(0 == num)
    {
        return (0);
    }

    data_encoder_size = (CLIST_DATA_ENCODER_SIZE)clist_codec_get(clist, CLIST_CODEC_ENCODER_SIZE);
    if(NULL_PTR == data_encoder_size)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_encode_clist_size: clist %lx: type = %ld, num = %ld, data encoder_size is null\n",
                            clist, type, num);
        return ((UINT32)-1);
    }

    if(MM_UINT32 == type)
    {
        CLIST_DATA *clist_data;

        CLIST_LOCK(clist, LOC_CMPIE_0024);
        CLIST_LOOP_NEXT(clist, clist_data)
        {
            void *data;
            data = CLIST_DATA_DATA(clist_data);
            data_encoder_size(comm, data, size);
        }
        CLIST_UNLOCK(clist, LOC_CMPIE_0025);
    }
    else/*non UINT32*/
    {
        CLIST_DATA *clist_data;

        CLIST_LOCK(clist, LOC_CMPIE_0026);
        CLIST_LOOP_NEXT(clist, clist_data)
        {
            void *data;
            data = CLIST_DATA_DATA(clist_data);
            data_encoder_size(comm, data, size);
        }
        CLIST_UNLOCK(clist, LOC_CMPIE_0027);
    }

    CMPI_DBG((LOGSTDOUT, "info:cmpi_encode_clist_size: clist %lx: type = %ld, num = %ld, ==> size %ld\n",
                            clist, clist_type(clist), clist_size(clist), *size));

    return (0);
}

UINT32 cmpi_decode_clist(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CLIST *clist)
{
    UINT32 num;
    UINT32 type;

    UINT32 pos;
    CLIST_DATA_DECODER data_decoder;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_clist: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_clist: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == clist )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_clist: clist is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(type));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));

    dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_clist: enter: clist %lx, type = %ld, num = %ld, size = %ld\n", clist, type, num, clist->size);

    if(type != clist->data_mm_type)
    {
        dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_clist: clist %lx, data type %ld ==> %ld\n", clist, clist->data_mm_type, type);
        clist_codec_set(clist, type);
    }
    dbg_log(SEC_0035_CMPIE, 3)(LOGSTDNULL, "info:cmpi_decode_clist: [0] clist %lx, data type %ld \n", clist, clist->data_mm_type);

    if(0 == num)
    {
        return (0);
    }

    data_decoder = (CLIST_DATA_DECODER)clist_codec_get(clist, CLIST_CODEC_DECODER);
    if(NULL_PTR == data_decoder)
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_clist: clist %lx data decoder is null\n", clist);
        return ((UINT32)-1);
    }

    if(MM_UINT32 == type)
    {
        /*alloc new item to accept the decoded result, and push the new item*/
        for(pos = 0; pos < num; pos ++)
        {
            UINT32 data;

            data_decoder(comm, in_buff, in_buff_max_len, position, &data);
            clist_push_back(clist, (void *)data);/*add new one*/
        }
    }
    else/*non UINT32*/
    {
        CLIST_DATA_INIT    data_init;

        data_init = (CLIST_DATA_INIT)clist_codec_get(clist, CLIST_CODEC_INIT);/*data_init may be null pointer*/
        if(NULL_PTR == data_init)
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_clist: clist %lx data init is null\n", clist);
            return ((UINT32)-1);
        }
        /*alloc new item to accept the decoded result, and push the new item*/
        for(pos = 0; pos < num; pos ++)
        {
            void * data;

            alloc_static_mem(MD_TBD, CMPI_ANY_MODI, type, &data, LOC_CMPIE_0028);
            if(NULL_PTR == data)
            {
                dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_clist: [3] clist %lx, size %ld, pos = %ld failed to alloc\n",
                                    clist, clist->size, pos);
                return ((UINT32)-1);
            }
            data_init(CMPI_ANY_MODI, data);
            data_decoder(comm, in_buff, in_buff_max_len, position, data);
            clist_push_back(clist, (void *)data);/*add new one*/
        }
    }
    CMPI_DBG((LOGSTDOUT, "info:cmpi_decode_clist: leave: clist %lx, type = %ld, num = %ld, size = %ld\n", clist, type, num, clist->size));
    return (0);
}

UINT32 cmpi_encode_crfsnp_inode(const UINT32 comm, const CRFSNP_INODE *crfsnp_inode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_inode: crfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_inode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint16(comm, CRFSNP_INODE_DISK_NO(crfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint16(comm, CRFSNP_INODE_BLOCK_NO(crfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint16(comm, CRFSNP_INODE_PAGE_NO(crfsnp_inode), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_crfsnp_inode_size(const UINT32 comm, const CRFSNP_INODE *crfsnp_inode, UINT32 *size)
{
    cmpi_encode_uint16_size(comm, CRFSNP_INODE_DISK_NO(crfsnp_inode), size);
    cmpi_encode_uint16_size(comm, CRFSNP_INODE_BLOCK_NO(crfsnp_inode), size);
    cmpi_encode_uint16_size(comm, CRFSNP_INODE_PAGE_NO(crfsnp_inode), size);

    return (0);
}

UINT32 cmpi_decode_crfsnp_inode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSNP_INODE *crfsnp_inode)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_inode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_inode: crfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSNP_INODE_DISK_NO(crfsnp_inode)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSNP_INODE_BLOCK_NO(crfsnp_inode)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSNP_INODE_PAGE_NO(crfsnp_inode)));

    return (0);
}


UINT32 cmpi_encode_crfsnp_fnode(const UINT32 comm, const CRFSNP_FNODE *crfsnp_fnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    uint32_t crfsnp_inode_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_fnode: crfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_fnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CRFSNP_FNODE_FILESZ(crfsnp_fnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRFSNP_FNODE_REPNUM(crfsnp_fnode), out_buff, out_buff_max_len, position);
    
    for(crfsnp_inode_pos = 0; crfsnp_inode_pos < (CRFSNP_FNODE_REPNUM(crfsnp_fnode)) && crfsnp_inode_pos < CRFSNP_FILE_REPLICA_MAX_NUM; crfsnp_inode_pos ++)
    {
        CRFSNP_INODE *crfsnp_inode;

        crfsnp_inode = (CRFSNP_INODE *)CRFSNP_FNODE_INODE(crfsnp_fnode, crfsnp_inode_pos);
        cmpi_encode_crfsnp_inode(comm, crfsnp_inode, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_crfsnp_fnode_size(const UINT32 comm, const CRFSNP_FNODE *crfsnp_fnode, UINT32 *size)
{
    uint32_t crfsnp_inode_pos;

    cmpi_encode_uint32_size(comm, CRFSNP_FNODE_FILESZ(crfsnp_fnode), size);
    cmpi_encode_uint32_size(comm, CRFSNP_FNODE_REPNUM(crfsnp_fnode), size);
    for(crfsnp_inode_pos = 0; crfsnp_inode_pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode) && crfsnp_inode_pos < CRFSNP_FILE_REPLICA_MAX_NUM; crfsnp_inode_pos ++)
    {
        CRFSNP_INODE *crfsnp_inode;

        crfsnp_inode = (CRFSNP_INODE *)CRFSNP_FNODE_INODE(crfsnp_fnode, crfsnp_inode_pos);
        cmpi_encode_crfsnp_inode_size(comm, crfsnp_inode, size);
    }

    return (0);
}

UINT32 cmpi_decode_crfsnp_fnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSNP_FNODE *crfsnp_fnode)
{
    UINT32 file_size;
    UINT32 replica_num;
    
    uint32_t crfsnp_inode_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_fnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_fnode: crfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_size));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(replica_num));

    if(CRFSNP_FILE_REPLICA_MAX_NUM < (replica_num))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_crfsnp_fnode: replica num %ld overflow\n", replica_num);
        return ((UINT32)-1);
    }

    CRFSNP_FNODE_FILESZ(crfsnp_fnode) = (uint32_t)(file_size);
    CRFSNP_FNODE_REPNUM(crfsnp_fnode) = (uint32_t)(replica_num);

    for(crfsnp_inode_pos = 0; crfsnp_inode_pos < CRFSNP_FNODE_REPNUM(crfsnp_fnode); crfsnp_inode_pos ++)
    {
        CRFSNP_INODE *crfsnp_inode;

        crfsnp_inode = CRFSNP_FNODE_INODE(crfsnp_fnode, crfsnp_inode_pos);
        cmpi_decode_crfsnp_inode(comm, in_buff, in_buff_max_len, position, crfsnp_inode);
    }

    return (0);
}

UINT32 cmpi_encode_crfsnp_bnode(const UINT32 comm, const CRFSNP_BNODE *crfsnp_bnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsnp_bnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_bnode: crfsnp_bnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_bnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_bnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint64(comm, CRFSNP_BNODE_FILESZ(crfsnp_bnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint64(comm, CRFSNP_BNODE_STORESZ(crfsnp_bnode), out_buff, out_buff_max_len, position);
  
    return (0);
}

UINT32 cmpi_encode_crfsnp_bnode_size(const UINT32 comm, const CRFSNP_BNODE *crfsnp_bnode, UINT32 *size)
{
    cmpi_encode_uint64_size(comm, CRFSNP_BNODE_FILESZ(crfsnp_bnode), size);
    cmpi_encode_uint64_size(comm, CRFSNP_BNODE_STORESZ(crfsnp_bnode), size);

    return (0);
}

UINT32 cmpi_decode_crfsnp_bnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSNP_BNODE *crfsnp_bnode)
{
    uint64_t file_size;
    uint64_t store_size;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_bnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_bnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsnp_bnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_bnode: crfsnp_bnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint64(comm, in_buff, in_buff_max_len, position, &(file_size));
    cmpi_decode_uint64(comm, in_buff, in_buff_max_len, position, &(store_size));


    CRFSNP_BNODE_FILESZ(crfsnp_bnode)  = (uint32_t)(file_size);
    CRFSNP_BNODE_STORESZ(crfsnp_bnode) = (uint32_t)(store_size);

    return (0);
}

UINT32 cmpi_encode_crfsnp_item(const UINT32 comm, const CRFSNP_ITEM *crfsnp_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_item: crfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_item: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CRFSNP_ITEM_DIR_FLAG(crfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRFSNP_ITEM_USED_FLAG(crfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRFSNP_ITEM_KLEN(crfsnp_item), out_buff, out_buff_max_len, position);

    cmpi_pack(CRFSNP_ITEM_KEY(crfsnp_item), CRFSNP_ITEM_KLEN(crfsnp_item), CMPI_UCHAR, out_buff, out_buff_max_len, position, comm);

    cmpi_encode_uint32(comm, CRFSNP_ITEM_PARENT_POS(crfsnp_item), out_buff, out_buff_max_len, position);

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_uint32(comm, CRFSNP_DNODE_FILE_NUM(CRFSNP_ITEM_DNODE(crfsnp_item)), out_buff, out_buff_max_len, position);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_crfsnp_fnode(comm, CRFSNP_ITEM_FNODE(crfsnp_item), out_buff, out_buff_max_len, position);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_crfsnp_bnode(comm, CRFSNP_ITEM_BNODE(crfsnp_item), out_buff, out_buff_max_len, position);
    }    

    return (0);
}

UINT32 cmpi_encode_crfsnp_item_size(const UINT32 comm, const CRFSNP_ITEM *crfsnp_item, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CRFSNP_ITEM_DIR_FLAG(crfsnp_item), size);
    cmpi_encode_uint32_size(comm, CRFSNP_ITEM_USED_FLAG(crfsnp_item), size);
    cmpi_encode_uint32_size(comm, CRFSNP_ITEM_KLEN(crfsnp_item), size);

    cmpi_pack_size(CRFSNP_ITEM_KLEN(crfsnp_item), CMPI_UCHAR, size,  comm);

    cmpi_encode_uint32_size(comm, CRFSNP_ITEM_PARENT_POS(crfsnp_item), size);

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_uint32_size(comm, CRFSNP_DNODE_FILE_NUM(CRFSNP_ITEM_DNODE(crfsnp_item)), size);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_crfsnp_fnode_size(comm, CRFSNP_ITEM_FNODE(crfsnp_item), size);
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_encode_crfsnp_bnode_size(comm, CRFSNP_ITEM_BNODE(crfsnp_item), size);
    }
    return (0);
}

UINT32 cmpi_decode_crfsnp_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSNP_ITEM *crfsnp_item)
{
    UINT32 dflag;
    UINT32 stat;
    UINT32 klen;

    UINT32 parent_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_item: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsnp_item: crfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(dflag));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(klen));

    CRFSNP_ITEM_DIR_FLAG(crfsnp_item) = (uint32_t)(dflag);
    CRFSNP_ITEM_USED_FLAG(crfsnp_item) = (uint32_t)(stat );
    CRFSNP_ITEM_KLEN(crfsnp_item) = (uint32_t)(klen );

    cmpi_unpack(in_buff, in_buff_max_len, position, CRFSNP_ITEM_KEY(crfsnp_item), CRFSNP_ITEM_KLEN(crfsnp_item), CMPI_UCHAR, comm);

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(parent_pos));

    CRFSNP_ITEM_PARENT_POS(crfsnp_item)     = (uint32_t)(parent_pos);

    if(CRFSNP_ITEM_FILE_IS_DIR == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        UINT32 file_num;
        cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_num));
        CRFSNP_DNODE_FILE_NUM(CRFSNP_ITEM_DNODE(crfsnp_item)) = (uint32_t)(file_num);
    }

    if(CRFSNP_ITEM_FILE_IS_REG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_decode_crfsnp_fnode(comm, in_buff, in_buff_max_len, position, CRFSNP_ITEM_FNODE(crfsnp_item));
    }

    if(CRFSNP_ITEM_FILE_IS_BIG == CRFSNP_ITEM_DIR_FLAG(crfsnp_item))
    {
        cmpi_decode_crfsnp_bnode(comm, in_buff, in_buff_max_len, position, CRFSNP_ITEM_BNODE(crfsnp_item));
    }
    
    return (0);
}

UINT32 cmpi_encode_chfsnp_inode(const UINT32 comm, const CHFSNP_INODE *chfsnp_inode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == chfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_inode: chfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_inode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CHFSNP_INODE_DISK_NO(chfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CHFSNP_INODE_BLOCK_NO(chfsnp_inode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CHFSNP_INODE_PAGE_NO(chfsnp_inode), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_chfsnp_inode_size(const UINT32 comm, const CHFSNP_INODE *chfsnp_inode, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CHFSNP_INODE_DISK_NO(chfsnp_inode), size);
    cmpi_encode_uint32_size(comm, CHFSNP_INODE_BLOCK_NO(chfsnp_inode), size);
    cmpi_encode_uint32_size(comm, CHFSNP_INODE_PAGE_NO(chfsnp_inode), size);

    return (0);
}

UINT32 cmpi_decode_chfsnp_inode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CHFSNP_INODE *chfsnp_inode)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_inode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_inode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == chfsnp_inode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_inode: chfsnp_inode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CHFSNP_INODE_DISK_NO(chfsnp_inode)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CHFSNP_INODE_BLOCK_NO(chfsnp_inode)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CHFSNP_INODE_PAGE_NO(chfsnp_inode)));

    return (0);
}


UINT32 cmpi_encode_chfsnp_fnode(const UINT32 comm, const CHFSNP_FNODE *chfsnp_fnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    uint32_t chfsnp_inode_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == chfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_fnode: chfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_fnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CHFSNP_FNODE_FILESZ(chfsnp_fnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CHFSNP_FNODE_REPNUM(chfsnp_fnode), out_buff, out_buff_max_len, position);
    
    for(chfsnp_inode_pos = 0; chfsnp_inode_pos < (CHFSNP_FNODE_REPNUM(chfsnp_fnode)) && chfsnp_inode_pos < CHFSNP_FILE_REPLICA_MAX_NUM; chfsnp_inode_pos ++)
    {
        CHFSNP_INODE *chfsnp_inode;

        chfsnp_inode = (CHFSNP_INODE *)CHFSNP_FNODE_INODE(chfsnp_fnode, chfsnp_inode_pos);
        cmpi_encode_chfsnp_inode(comm, chfsnp_inode, out_buff, out_buff_max_len, position);
    }

    return (0);
}

UINT32 cmpi_encode_chfsnp_fnode_size(const UINT32 comm, const CHFSNP_FNODE *chfsnp_fnode, UINT32 *size)
{
    uint32_t chfsnp_inode_pos;

    cmpi_encode_uint32_size(comm, CHFSNP_FNODE_FILESZ(chfsnp_fnode), size);
    cmpi_encode_uint32_size(comm, CHFSNP_FNODE_REPNUM(chfsnp_fnode), size);
    for(chfsnp_inode_pos = 0; chfsnp_inode_pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode) && chfsnp_inode_pos < CHFSNP_FILE_REPLICA_MAX_NUM; chfsnp_inode_pos ++)
    {
        CHFSNP_INODE *chfsnp_inode;

        chfsnp_inode = (CHFSNP_INODE *)CHFSNP_FNODE_INODE(chfsnp_fnode, chfsnp_inode_pos);
        cmpi_encode_chfsnp_inode_size(comm, chfsnp_inode, size);
    }

    return (0);
}

UINT32 cmpi_decode_chfsnp_fnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CHFSNP_FNODE *chfsnp_fnode)
{
    UINT32 file_size;
    UINT32 replica_num;
    
    uint32_t chfsnp_inode_pos;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_fnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_fnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == chfsnp_fnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_fnode: chfsnp_fnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(file_size));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(replica_num));

    if(CHFSNP_FILE_REPLICA_MAX_NUM < (replica_num))
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_chfsnp_fnode: replica num %ld overflow\n", replica_num);
        return ((UINT32)-1);
    }

    CHFSNP_FNODE_FILESZ(chfsnp_fnode) = (uint32_t)(file_size);
    CHFSNP_FNODE_REPNUM(chfsnp_fnode) = (uint32_t)(replica_num);

    for(chfsnp_inode_pos = 0; chfsnp_inode_pos < CHFSNP_FNODE_REPNUM(chfsnp_fnode); chfsnp_inode_pos ++)
    {
        CHFSNP_INODE *chfsnp_inode;

        chfsnp_inode = CHFSNP_FNODE_INODE(chfsnp_fnode, chfsnp_inode_pos);
        cmpi_decode_chfsnp_inode(comm, in_buff, in_buff_max_len, position, chfsnp_inode);
    }

    return (0);
}

UINT32 cmpi_encode_chfsnp_item(const UINT32 comm, const CHFSNP_ITEM *chfsnp_item, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == chfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_item: chfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_item: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_chfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint32(comm, CHFSNP_ITEM_STAT(chfsnp_item), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CHFSNP_ITEM_K_LEN(chfsnp_item), out_buff, out_buff_max_len, position);

    cmpi_pack(CHFSNP_ITEM_KEY(chfsnp_item), CHFSNP_ITEM_K_LEN(chfsnp_item), CMPI_UCHAR, out_buff, out_buff_max_len, position, comm);

    cmpi_encode_chfsnp_fnode(comm, CHFSNP_ITEM_FNODE(chfsnp_item), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_chfsnp_item_size(const UINT32 comm, const CHFSNP_ITEM *chfsnp_item, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CHFSNP_ITEM_STAT(chfsnp_item), size);
    cmpi_encode_uint32_size(comm, CHFSNP_ITEM_K_LEN(chfsnp_item), size);

    cmpi_pack_size(CHFSNP_ITEM_K_LEN(chfsnp_item), CMPI_UCHAR, size,  comm);

    cmpi_encode_chfsnp_fnode_size(comm, CHFSNP_ITEM_FNODE(chfsnp_item), size);

    return (0);
}

UINT32 cmpi_decode_chfsnp_item(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CHFSNP_ITEM *chfsnp_item)
{
    UINT32 stat;
    UINT32 klen;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_item: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_item: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == chfsnp_item )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_chfsnp_item: chfsnp_item is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(stat));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(klen));

    CHFSNP_ITEM_STAT(chfsnp_item)  = (uint32_t)(stat );
    CHFSNP_ITEM_K_LEN(chfsnp_item) = (uint32_t)(klen );

    cmpi_unpack(in_buff, in_buff_max_len, position, CHFSNP_ITEM_KEY(chfsnp_item), CHFSNP_ITEM_K_LEN(chfsnp_item), CMPI_UCHAR, comm);

    cmpi_decode_chfsnp_fnode(comm, in_buff, in_buff_max_len, position, CHFSNP_ITEM_FNODE(chfsnp_item));

    return (0);
}

UINT32 cmpi_encode_cmd5_digest(const UINT32 comm, const CMD5_DIGEST *cmd5_digest, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cmd5_digest )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmd5_digest: cmd5_digest is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmd5_digest: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cmd5_digest: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_encode_uint8_array(comm, CMD5_DIGEST_SUM(cmd5_digest), CMD5_DIGEST_LEN, out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_cmd5_digest_size(const UINT32 comm, const CMD5_DIGEST *cmd5_digest, UINT32 *size)
{
    cmpi_encode_uint8_array_size(comm, CMD5_DIGEST_SUM(cmd5_digest), CMD5_DIGEST_LEN, size);
    return (0);
}

UINT32 cmpi_decode_cmd5_digest(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CMD5_DIGEST *cmd5_digest)
{
    UINT32 size;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmd5_digest: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmd5_digest: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cmd5_digest )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cmd5_digest: cmd5_digest is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint8_array(comm, in_buff, in_buff_max_len, position, CMD5_DIGEST_SUM(cmd5_digest), &(size));
    ASSERT(CMD5_DIGEST_LEN == size);

    return (0);
}

UINT32 cmpi_encode_crfsop(const UINT32 comm, const CRFSOP *crfsop, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsop )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsop: crfsop is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsop: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsop: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    cmpi_encode_uint16(comm, CRFSOP_OP_TYPE(crfsop), out_buff, out_buff_max_len, position);
    cmpi_encode_uint16(comm, CRFSOP_PATH_TYPE(crfsop), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32(comm, CRFSOP_PATH_HASH(crfsop), out_buff, out_buff_max_len, position);
    cmpi_encode_cstring(comm, CRFSOP_PATH_NAME(crfsop), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_crfsop_size(const UINT32 comm, const CRFSOP *crfsop, UINT32 *size)
{
    cmpi_encode_uint16_size(comm, CRFSOP_OP_TYPE(crfsop), size);
    cmpi_encode_uint16_size(comm, CRFSOP_PATH_TYPE(crfsop), size);
    cmpi_encode_uint32_size(comm, CRFSOP_PATH_HASH(crfsop), size);
    cmpi_encode_cstring_size(comm, CRFSOP_PATH_NAME(crfsop), size);
    return (0);
}

UINT32 cmpi_decode_crfsop(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSOP *crfsop)
{
    UINT32 num;

#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsop: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsop: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsop )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsop: crfsop is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSOP_OP_TYPE(crfsop)));
    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSOP_PATH_TYPE(crfsop)));
    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));    
    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CRFSOP_PATH_NAME(crfsop));

    CRFSOP_PATH_HASH(crfsop) = (uint32_t)num;

    return (0);
}

UINT32 cmpi_encode_crfsconhash_rnode(const UINT32 comm, const CRFSCONHASH_RNODE *crfsconhash_rnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsconhash_rnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode: crfsconhash_rnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    cmpi_encode_uint16(comm, CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32_t(comm, CRFSCONHASH_RNODE_TCID(crfsconhash_rnode), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_crfsconhash_rnode_size(const UINT32 comm, const CRFSCONHASH_RNODE *crfsconhash_rnode, UINT32 *size)
{
    cmpi_encode_uint16_size(comm, CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode), size);
    cmpi_encode_uint32_t_size(comm, CRFSCONHASH_RNODE_TCID(crfsconhash_rnode), size);
    return (0);
}

UINT32 cmpi_decode_crfsconhash_rnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSCONHASH_RNODE *crfsconhash_rnode)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsconhash_rnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode: crfsconhash_rnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint16(comm, in_buff, in_buff_max_len, position, &(CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode)));
    cmpi_decode_uint32_t(comm, in_buff, in_buff_max_len, position, &(CRFSCONHASH_RNODE_TCID(crfsconhash_rnode)));

    return (0);
}

UINT32 cmpi_encode_crfsconhash_vnode(const UINT32 comm, const CRFSCONHASH_VNODE *crfsconhash_vnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsconhash_vnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode: crfsconhash_vnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    cmpi_encode_uint32_t(comm, CRFSCONHASH_VNODE_HASH(crfsconhash_vnode), out_buff, out_buff_max_len, position);
    cmpi_encode_uint32_t(comm, CRFSCONHASH_VNODE_POS(crfsconhash_vnode), out_buff, out_buff_max_len, position);

    return (0);
}

UINT32 cmpi_encode_crfsconhash_vnode_size(const UINT32 comm, const CRFSCONHASH_VNODE *crfsconhash_vnode, UINT32 *size)
{
    cmpi_encode_uint32_t_size(comm, CRFSCONHASH_VNODE_HASH(crfsconhash_vnode), size);
    cmpi_encode_uint32_t_size(comm, CRFSCONHASH_VNODE_POS(crfsconhash_vnode), size);
    return (0);
}

UINT32 cmpi_decode_crfsconhash_vnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSCONHASH_VNODE *crfsconhash_vnode)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsconhash_vnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode: crfsconhash_vnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32_t(comm, in_buff, in_buff_max_len, position, &(CRFSCONHASH_VNODE_HASH(crfsconhash_vnode)));    
    cmpi_decode_uint32_t(comm, in_buff, in_buff_max_len, position, &(CRFSCONHASH_VNODE_POS(crfsconhash_vnode)));    

    return (0);
}

UINT32 cmpi_encode_crfsconhash_rnode_vec(const UINT32 comm, const CVECTOR *rnode_vec, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    UINT32 num;
    UINT32 pos;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == rnode_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode_vec: rnode_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode_vec: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_rnode_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    num = cvector_size(rnode_vec);
    cmpi_encode_uint32(comm, num, out_buff, out_buff_max_len, position);
    
    for(pos = 0; pos < num; pos ++)
    {
        CRFSCONHASH_RNODE *crfsconhash_rnode;
        crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(rnode_vec, pos);
        if(NULL_PTR == crfsconhash_rnode)
        {
            uint32_t     tcid;
            uint16_t     replicas;
        
            replicas = CRFSCONHASH_ERR_REPLICAS;
            tcid     = (uint32_t)CMPI_ERROR_TCID;

            cmpi_encode_uint16(comm, replicas, out_buff, out_buff_max_len, position);
            cmpi_encode_uint32_t(comm, tcid, out_buff, out_buff_max_len, position);
            
            continue;
        }

        cmpi_encode_crfsconhash_rnode(comm, crfsconhash_rnode, out_buff, out_buff_max_len, position);
    }
    return (0);
}

UINT32 cmpi_encode_crfsconhash_rnode_vec_size(const UINT32 comm, const CVECTOR *rnode_vec, UINT32 *size)
{
    UINT32 num;
    UINT32 pos;
   
    num = cvector_size(rnode_vec);
    cmpi_encode_uint32_size(comm, num, size);
    
    for(pos = 0; pos < num; pos ++)
    {
        CRFSCONHASH_RNODE *crfsconhash_rnode;
        crfsconhash_rnode = (CRFSCONHASH_RNODE *)cvector_get(rnode_vec, pos);
        if(NULL_PTR == crfsconhash_rnode)
        {
            uint32_t     tcid;
            uint16_t     replicas;
        
            replicas = CRFSCONHASH_ERR_REPLICAS;
            tcid     = (uint32_t)CMPI_ERROR_TCID;

            cmpi_encode_uint16_size(comm, replicas, size);
            cmpi_encode_uint32_t_size(comm, tcid, size);
            
            continue;
        }

        cmpi_encode_crfsconhash_rnode_size(comm, crfsconhash_rnode, size);
    }
    return (0);
}

UINT32 cmpi_decode_crfsconhash_rnode_vec(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CVECTOR *rnode_vec)
{
    UINT32 num;
    UINT32 pos;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode_vec: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode_vec: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == rnode_vec )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_rnode_vec: rnode_vec is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(num));

    for(pos = 0; pos < num; pos ++)
    {
        CRFSCONHASH_RNODE *crfsconhash_rnode;

        crfsconhash_rnode = crfsconhash_rnode_new();
        if(NULL_PTR == crfsconhash_rnode)
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_crfsconhash_rnode_vec: new rnode %u# failed\n", pos);
            return ((UINT32)-1);
        }
        
        if(EC_FALSE == cmpi_decode_crfsconhash_rnode(comm, in_buff, in_buff_max_len, position, crfsconhash_rnode))
        {
            dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT, "error:cmpi_decode_crfsconhash_rnode_vec: decode rnode %u# failed\n", pos);
            crfsconhash_rnode_free(crfsconhash_rnode);
            return ((UINT32)-1);
        }
        
        if(CRFSCONHASH_ERR_REPLICAS == CRFSCONHASH_RNODE_REPLICAS(crfsconhash_rnode)
        && ((uint32_t)CMPI_ERROR_TCID) == CRFSCONHASH_RNODE_TCID(crfsconhash_rnode))
        {
            cvector_push(rnode_vec, NULL_PTR);
            crfsconhash_rnode_free(crfsconhash_rnode);
            continue;
        }

        cvector_push(rnode_vec, crfsconhash_rnode);
    }

    return (0);
}

static UINT32 __cmpi_encode_crfsconhash_vnode_tree_inorder(const UINT32 comm, const CRB_TREE *vnode_tree, const CRB_NODE *node, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{    
   CRFSCONHASH_VNODE *crfsconhash_vnode;
    if(NULL_PTR == node)
    {
        return (0);
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        if(0 != __cmpi_encode_crfsconhash_vnode_tree_inorder(comm, vnode_tree, CRB_NODE_LEFT(node), out_buff, out_buff_max_len, position))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: encode left subtree %p failed\n", CRB_NODE_LEFT(node));
            return ((UINT32)-1);
        }
    }

    crfsconhash_vnode = (CRFSCONHASH_VNODE *)CRB_NODE_DATA(node);
    if(NULL_PTR == crfsconhash_vnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: data of crb node %p is null\n", node);
        return ((UINT32)-1);
    }
    
    if(0 != cmpi_encode_crfsconhash_vnode(comm, crfsconhash_vnode, out_buff, out_buff_max_len, position))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: encode vnode %p failed\n", crfsconhash_vnode);
        return ((UINT32)-1);
    }

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        if(0 != __cmpi_encode_crfsconhash_vnode_tree_inorder(comm, vnode_tree, CRB_NODE_RIGHT(node), out_buff, out_buff_max_len, position))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__crfsconhash_flush_vnodes_inorder: encode right subtree %p failed\n", CRB_NODE_RIGHT(node));
            return ((UINT32)-1);
        }
    }    
    return (0);
}

UINT32 cmpi_encode_crfsconhash_vnode_tree(const UINT32 comm, const CRB_TREE *vnode_tree, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    uint32_t num;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == vnode_tree )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode_tree: vnode_tree is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode_tree: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash_vnode_tree: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    num = crb_tree_node_num(vnode_tree);
    cmpi_encode_uint32_t(comm, num, out_buff, out_buff_max_len, position);

    __cmpi_encode_crfsconhash_vnode_tree_inorder(comm, vnode_tree, CRB_TREE_ROOT(vnode_tree), out_buff, out_buff_max_len, position);

    return (0);
}

static UINT32 __cmpi_encode_crfsconhash_vnode_tree_size_inorder(const UINT32 comm, const CRB_TREE *vnode_tree, const CRB_NODE *node, UINT32 *size)
{    
   CRFSCONHASH_VNODE *crfsconhash_vnode;
    if(NULL_PTR == node)
    {
        return (0);
    }
    
    if(NULL_PTR != CRB_NODE_LEFT(node))
    {
        if(0 != __cmpi_encode_crfsconhash_vnode_tree_size_inorder(comm, vnode_tree, CRB_NODE_LEFT(node), size))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__cmpi_encode_crfsconhash_vnode_tree_size_inorder: encode left subtree %p failed\n", CRB_NODE_LEFT(node));
            return ((UINT32)-1);
        }
    }

    crfsconhash_vnode = (CRFSCONHASH_VNODE *)CRB_NODE_DATA(node);
    if(NULL_PTR == crfsconhash_vnode)
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__cmpi_encode_crfsconhash_vnode_tree_size_inorder: data of crb node %p is null\n", node);
        return ((UINT32)-1);
    }
    
    if(0 != cmpi_encode_crfsconhash_vnode_size(comm, crfsconhash_vnode, size))
    {
        dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__cmpi_encode_crfsconhash_vnode_tree_size_inorder: encode vnode %p failed\n", crfsconhash_vnode);
        return ((UINT32)-1);
    }

    if(NULL_PTR != CRB_NODE_RIGHT(node))
    {
        if(0 != __cmpi_encode_crfsconhash_vnode_tree_size_inorder(comm, vnode_tree, CRB_NODE_RIGHT(node), size))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:__cmpi_encode_crfsconhash_vnode_tree_size_inorder: encode right subtree %p failed\n", CRB_NODE_RIGHT(node));
            return ((UINT32)-1);
        }
    }    
    return (0);
}

UINT32 cmpi_encode_crfsconhash_vnode_tree_size(const UINT32 comm, const CRB_TREE *vnode_tree, UINT32 *size)
{
    uint32_t num;
   
    num = crb_tree_node_num(vnode_tree);
    cmpi_encode_uint32_t_size(comm, num, size);

    __cmpi_encode_crfsconhash_vnode_tree_size_inorder(comm, vnode_tree, CRB_TREE_ROOT(vnode_tree), size);
  
    return (0);
}

UINT32 cmpi_decode_crfsconhash_vnode_tree(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRB_TREE *vnode_tree)
{
    uint32_t num;
    uint32_t pos;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode_tree: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode_tree: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == vnode_tree )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash_vnode_tree: vnode_tree is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32_t(comm, in_buff, in_buff_max_len, position, &(num));

    for(pos = 0; pos < num; pos ++)
    {
        CRFSCONHASH_VNODE *crfsconhash_vnode;

        crfsconhash_vnode = crfsconhash_vnode_new();
        if(NULL_PTR == crfsconhash_vnode)
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:cmpi_decode_crfsconhash_vnode_tree: new vnode %u# failed\n", pos);
            return ((UINT32)-1);
        }
        
        if(0 != cmpi_decode_crfsconhash_vnode(comm, in_buff, in_buff_max_len, position, crfsconhash_vnode))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:cmpi_decode_crfsconhash_vnode_tree: decode vnode %u# failed\n", pos);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return ((UINT32)-1);
        }        

        if(NULL_PTR == crb_tree_insert_data(vnode_tree, (void *)crfsconhash_vnode))
        {
            dbg_log(SEC_0144_CRFSCONHASH, 0)(LOGSTDOUT, "error:cmpi_decode_crfsconhash_vnode_tree: insert vnode %u# at failed\n", pos);
            crfsconhash_vnode_free(crfsconhash_vnode);
            return ((UINT32)-1);
        }
    }

    return (0);
}


UINT32 cmpi_encode_crfsconhash(const UINT32 comm, const CRFSCONHASH *crfsconhash, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsconhash )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash: crfsconhash is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsconhash: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    cmpi_encode_uint32(comm, CRFSCONHASH_HASH_ID(crfsconhash), out_buff, out_buff_max_len, position);
    cmpi_encode_crfsconhash_rnode_vec(comm, CRFSCONHASH_RNODE_VEC(crfsconhash), out_buff, out_buff_max_len, position);
    cmpi_encode_crfsconhash_vnode_tree(comm, CRFSCONHASH_VNODE_TREE(crfsconhash), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_crfsconhash_size(const UINT32 comm, const CRFSCONHASH *crfsconhash, UINT32 *size)
{
    cmpi_encode_uint32_size(comm, CRFSCONHASH_HASH_ID(crfsconhash), size);
    cmpi_encode_crfsconhash_rnode_vec_size(comm, CRFSCONHASH_RNODE_VEC(crfsconhash), size);
    cmpi_encode_crfsconhash_vnode_tree_size(comm, CRFSCONHASH_VNODE_TREE(crfsconhash), size);
    return (0);
}

UINT32 cmpi_decode_crfsconhash(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSCONHASH *crfsconhash)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsconhash )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsconhash: crfsconhash is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_uint32(comm, in_buff, in_buff_max_len, position, &(CRFSCONHASH_HASH_ID(crfsconhash)));
    cmpi_decode_crfsconhash_rnode_vec(comm, in_buff, in_buff_max_len, position, CRFSCONHASH_RNODE_VEC(crfsconhash));
    cmpi_decode_crfsconhash_vnode_tree(comm, in_buff, in_buff_max_len, position, CRFSCONHASH_VNODE_TREE(crfsconhash));

    return (0);
}


UINT32 cmpi_encode_crfsdt_pnode(const UINT32 comm, const CRFSDT_PNODE *crfsdt_pnode, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == crfsdt_pnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsdt_pnode: crfsdt_pnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsdt_pnode: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_crfsdt_pnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    cmpi_encode_cstring(comm, CRFSDT_PNODE_PATH(crfsdt_pnode), out_buff, out_buff_max_len, position);
    cmpi_encode_crfsconhash(comm, CRFSDT_PNODE_CONHASH(crfsdt_pnode), out_buff, out_buff_max_len, position);
    return (0);
}

UINT32 cmpi_encode_crfsdt_pnode_size(const UINT32 comm, const CRFSDT_PNODE *crfsdt_pnode, UINT32 *size)
{
    cmpi_encode_cstring_size(comm, CRFSDT_PNODE_PATH(crfsdt_pnode), size);
    cmpi_encode_crfsconhash_size(comm, CRFSDT_PNODE_CONHASH(crfsdt_pnode), size);
    return (0);
}

UINT32 cmpi_decode_crfsdt_pnode(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CRFSDT_PNODE *crfsdt_pnode)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsdt_pnode: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsdt_pnode: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == crfsdt_pnode )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_crfsdt_pnode: crfsdt_pnode is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    cmpi_decode_cstring(comm, in_buff, in_buff_max_len, position, CRFSDT_PNODE_PATH(crfsdt_pnode));
    cmpi_decode_crfsconhash(comm, in_buff, in_buff_max_len, position, CRFSDT_PNODE_CONHASH(crfsdt_pnode));

    return (0);
}

UINT32 cmpi_encode_cbuffer(const UINT32 comm, const CBUFFER *cbuffer, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    uint32_t used_size;
    
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == cbuffer )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbuffer: cbuffer is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == out_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbuffer: out_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_encode_cbuffer: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */
   
    used_size = CBUFFER_USED(cbuffer);
    cmpi_pack((UINT8 *)&(used_size), 1, CMPI_U32, out_buff, out_buff_max_len, position,  comm);
    cmpi_pack(CBUFFER_DATA(cbuffer), CBUFFER_USED(cbuffer), CMPI_UCHAR, out_buff, out_buff_max_len, position,  comm);
    return (0);
}

UINT32 cmpi_encode_cbuffer_size(const UINT32 comm, const CBUFFER *cbuffer, UINT32 *size)
{
    (*size) += CBUFFER_USED(cbuffer);
    return (0);
}

UINT32 cmpi_decode_cbuffer(const UINT32 comm, const UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position, CBUFFER *cbuffer)
{
#if ( SWITCH_ON == ENCODE_DEBUG_SWITCH )
    if ( NULL_PTR == in_buff )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbuffer: in_buff is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == position )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbuffer: position is null.\n");
        dbg_exit(MD_TBD, 0);
    }
    if ( NULL_PTR == cbuffer )
    {
        dbg_log(SEC_0035_CMPIE, 0)(LOGSTDOUT,"error:cmpi_decode_cbuffer: cbuffer is null.\n");
        dbg_exit(MD_TBD, 0);
    }
#endif /* ENCODE_DEBUG_SWITCH */

    uint32_t used_size;

    cmpi_unpack(in_buff, in_buff_max_len, position, (UINT8 *)&used_size, 1,   CMPI_U32, comm);
    
    cbuffer_expand_to(cbuffer, used_size);
    cmpi_unpack(in_buff, in_buff_max_len, position, CBUFFER_DATA(cbuffer) + CBUFFER_USED(cbuffer), used_size, CMPI_UCHAR, comm);
    CBUFFER_USED(cbuffer) += used_size;

    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

