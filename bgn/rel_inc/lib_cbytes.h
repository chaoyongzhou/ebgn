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

#ifndef _LIB_CBYTES_H
#define _LIB_CBYTES_H

CBYTES *cbytes_new(const UINT32 len);

EC_BOOL cbytes_init(CBYTES *cbytes);

EC_BOOL cbytes_clean(CBYTES *cbytes, const UINT32 location);

EC_BOOL cbytes_free(CBYTES *cbytes, const UINT32 location);

EC_BOOL cbytes_init_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_clean_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_free_0(const UINT32 md_id, CBYTES *cbytes);

EC_BOOL cbytes_set(CBYTES *cbytes, const UINT32 len, const UINT8 *buf);

EC_BOOL cbytes_clone(const CBYTES *cbytes_src, CBYTES *cbytes_des);

EC_BOOL cbytes_mount(CBYTES *cbytes, const UINT32 len, const UINT8 *buff);

EC_BOOL cbytes_umount(CBYTES *cbytes);

EC_BOOL cbytes_cmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des);

EC_BOOL cbytes_ncmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des, const UINT32 cmp_len);

UINT32  cbytes_len(const CBYTES *cbytes);

UINT8 * cbytes_buf(const CBYTES *cbytes);

EC_BOOL cbytes_print_str(LOG *log, const CBYTES *cbytes);

EC_BOOL cbytes_print_chars(LOG *log, const CBYTES *cbytes);


#endif /*_LIB_CBYTES_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

