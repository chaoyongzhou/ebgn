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

#ifndef _LIB_CMPIC_INC
#define _LIB_CMPIC_INC

#include "lib_type.h"

#define CMPI_COMM_NULL      ((UINT32)( 0))
#define CMPI_COMM_WORLD     ((UINT32)(91))
#define CMPI_MIN_SIZE       ((UINT32)( 1))

#define CMPI_ULONG          ((UINT32)(9))
#define CMPI_USHORT         ((UINT32)(5))
#define CMPI_UCHAR          ((UINT32)(2))
#define CMPI_REAL           ((UINT32)(11))

#define CMPI_ANY_TCID       ((UINT32)( -1))
#define CMPI_ANY_MASK       ((UINT32)( -1))
#define CMPI_ANY_COMM       (CMPI_COMM_NULL)
#define CMPI_ANY_RANK       ((UINT32)( -2))
#define CMPI_ANY_MODI       ((UINT32)( -2))
#define CMPI_ANY_IPADDR     ((UINT32)( -3))
#define CMPI_ANY_SRVPORT    ((UINT32)( -4))
#define CMPI_ANY_TAG        ((UINT32)( -1))
#define CMPI_ANY_SOCKFD     ((UINT32)( -5)) 

#define CMPI_ANY_DBG_TCID   ((UINT32)( -6)) 
#define CMPI_ANY_MON_TCID   ((UINT32)( -7)) 

#define CMPI_ERROR_TCID     ((UINT32)( -8))
#define CMPI_ERROR_MASK     ((UINT32)(  1))
#define CMPI_ERROR_COMM     (CMPI_COMM_NULL)
#define CMPI_ERROR_RANK     ((UINT32)( -9))
#define CMPI_ERROR_MODI     (ERR_MODULE_ID)
#define CMPI_ERROR_IPADDR   ((UINT32)(  0))
#define CMPI_ERROR_SRVPORT  ((UINT32)(  0))
#define CMPI_ERROR_SOCKFD   ((int)   ( -1))
#define CMPI_ERROR_TAG      ((UINT32)(-12))

#endif /*_LIB_CMPIC_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
