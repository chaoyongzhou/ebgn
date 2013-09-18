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

#ifndef _LIB_CHAR2INT_H
#define _LIB_CHAR2INT_H

#include <stdio.h>
#include <stdlib.h>

UINT32 chars_to_uint32(const char *chars, const UINT32 len);

UINT32 str_to_uint32(const char *str);

char *uint32_to_str(const UINT32 num);

char *uint32_to_hex_str(const UINT32 num);

/*return host order*/
UINT32 ipv4_to_uint32(const char *ipv4_str);

/*ipv4_num is host order*/
char *uint32_to_ipv4(const UINT32 ipv4_num);

UINT32 port_to_uint32(const char *port_str);

char *uint32_to_port(const UINT32 port_num);

int c_open(const char *pathname, const int flags, const mode_t mode);

#endif /*_LIB_CHAR2INT_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

