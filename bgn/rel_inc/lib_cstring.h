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

#ifndef _LIB_CSTRING_H
#define _LIB_CSTRING_H

#include "lib_type.h"

CSTRING *cstring_new(const UINT8 *str, const UINT32 location);

void cstring_free(CSTRING *cstring);

void cstring_init(CSTRING *cstring, const UINT8 *str);

void cstring_clean(CSTRING *cstring);

void cstring_clone(const CSTRING *cstring_src, CSTRING *cstring_des);

void cstring_reset(CSTRING *cstring);

EC_BOOL cstring_is_empty(const CSTRING *cstring);

EC_BOOL cstring_cmp(const CSTRING *cstring_src, const CSTRING *cstring_des);

EC_BOOL cstring_expand(CSTRING *cstring, const UINT32 location);

EC_BOOL cstring_expand_to(CSTRING *cstring, const UINT32 size);

UINT32 cstring_get_capacity(const CSTRING *cstring);

UINT32 cstring_get_len(const CSTRING *cstring);

UINT8 * cstring_get_str(const CSTRING *cstring);

EC_BOOL cstring_get_char(const CSTRING *cstring, const UINT32 pos, UINT8 *pch);

EC_BOOL cstring_set_word(CSTRING *cstring, const UINT32 num);

UINT32 cstring_get_word(const CSTRING *cstring);

EC_BOOL cstring_set_str(CSTRING *cstring, const UINT8 *str);
EC_BOOL cstring_set_char(CSTRING *cstring, const UINT8 ch, const UINT32 pos);

EC_BOOL cstring_get_cstr(const CSTRING *cstring_src, const UINT32 from, const UINT32 to, CSTRING *cstring_des);
EC_BOOL cstring_set_chars(CSTRING *cstring, const UINT8 *pchs, const UINT32 len);

EC_BOOL cstring_erase_char(CSTRING *cstring, const UINT32 pos);

EC_BOOL cstring_append_char(CSTRING *cstring, const UINT8 ch);

EC_BOOL cstring_append_chars(CSTRING *cstring, const UINT32 ch_num, const UINT8 *chars);

EC_BOOL cstring_append_str(CSTRING *cstring, const UINT8 *str);

EC_BOOL cstring_append_cstr(const CSTRING *cstring_src, CSTRING *cstring_des);

EC_BOOL cstring_erase_sub_str(CSTRING *cstring, const UINT32 from, const UINT32 to);

EC_BOOL cstring_erase_tail_str(CSTRING *cstring, const UINT32 len);

void cstring_format(CSTRING *cstring, const char *format, ...);

EC_BOOL cstring_regex(const CSTRING *cstring, const CSTRING *pattern, void *cvector_cstring);

UINT32 cstring_print(LOG *log, const CSTRING *cstring);


#endif/* _LIB_CSTRING_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
