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

#ifndef _LIB_CDFSNP_H
#define _LIB_CDFSNP_H

#define CDFSNP_4K_MODE               ((UINT32) 0)
#define CDFSNP_64K_MODE              ((UINT32) 1)
#define CDFSNP_1M_MODE               ((UINT32) 2)
#define CDFSNP_2M_MODE               ((UINT32) 3)
#define CDFSNP_128M_MODE             ((UINT32) 4)
#define CDFSNP_256M_MODE             ((UINT32) 5)
#define CDFSNP_512M_MODE             ((UINT32) 6)
#define CDFSNP_1G_MODE               ((UINT32) 7)
#define CDFSNP_2G_MODE               ((UINT32) 8)

#if (64 == WORDSIZE)
#define CDFSNP_4G_MODE               ((UINT32) 9)
#endif/*(64 == WORDSIZE)*/

#define CDFSNP_ERR_MODE              ((UINT32)0xF)  /*4 bits*/


#endif/* _LIB_CDFSNP_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

