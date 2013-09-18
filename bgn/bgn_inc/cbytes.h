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

#ifndef _CBYTES_H
#define _CBYTES_H

#include "type.h"
#include "log.h"

#define CBYTES_LEN(cbytes)        ((cbytes)->len)
#define CBYTES_BUF(cbytes)        ((cbytes)->buf)

#define CBYTES_DEFAULT_LEN        (128)

CBYTES *cbytes_new(const UINT32 len);

EC_BOOL cbytes_init(CBYTES *cbytes);

EC_BOOL cbytes_clean(CBYTES *cbytes, const UINT32 location);

EC_BOOL cbytes_free(CBYTES *cbytes, const UINT32 location);

EC_BOOL cbytes_init_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_clean_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_free_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_set(CBYTES *cbytes, const UINT32 len, const UINT8 *buf);

EC_BOOL cbytes_set_word(CBYTES *cbytes, const UINT32 num);

EC_BOOL cbytes_get_word(CBYTES *cbytes, UINT32 *num);

CBYTES *cbytes_make_by_word(const UINT32 num);

CBYTES *cbytes_make_by_str(const UINT8 *str);

CBYTES *cbytes_make_by_cstr(const CSTRING *cstr);

CBYTES *cbytes_make_by_ctimet(const CTIMET *ctimet);

CBYTES *cbytes_make_by_bytes(const UINT32 len, const UINT8 *bytes);

CBYTES *cbytes_dup(const CBYTES *cbytes_src);

EC_BOOL cbytes_clone(const CBYTES *cbytes_src, CBYTES *cbytes_des);

EC_BOOL cbytes_mount(CBYTES *cbytes, const UINT32 len, const UINT8 *buff);

EC_BOOL cbytes_umount(CBYTES *cbytes);

EC_BOOL cbytes_cat(const CBYTES *cbytes_src_1st, const CBYTES *cbytes_src_2nd, CBYTES *cbytes_des);

EC_BOOL cbytes_cmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des);

EC_BOOL cbytes_ncmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des, const UINT32 cmp_len);

UINT32  cbytes_len(const CBYTES *cbytes);

UINT8 * cbytes_buf(const CBYTES *cbytes);

EC_BOOL cbytes_expand(CBYTES *cbytes, const UINT32 location);

EC_BOOL cbytes_expand_to(CBYTES *cbytes, const UINT32 size);

EC_BOOL cbytes_resize(CBYTES *cbytes, const UINT32 size);

EC_BOOL cbytes_fread(CBYTES *cbytes, FILE *fp);

EC_BOOL cbytes_print_str(LOG *log, const CBYTES *cbytes);

EC_BOOL cbytes_print_chars(LOG *log, const CBYTES *cbytes);


#endif /*_CBYTES_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

