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

#ifndef _LIB_CBC_H
#define _LIB_CBC_H

#include "lib_type.h"

EC_BOOL cbc_new(const UINT32 size);

EC_BOOL cbc_free();

UINT32 cbc_size();

UINT32 cbc_md_size(const UINT32 md_type);

EC_BOOL cbc_md_reg(const UINT32 md_type, const UINT32 md_capaciy);

EC_BOOL cbc_md_unreg(const UINT32 md_type);

EC_BOOL cbc_md_unreg_all();

UINT32 cbc_md_new(const UINT32 md_type, const UINT32 sizeof_md);

EC_BOOL cbc_md_free(const UINT32 md_type, const UINT32 pos);

void *cbc_md_get(const UINT32 md_type, const UINT32 pos);


#endif /*_LIB_CBC_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

