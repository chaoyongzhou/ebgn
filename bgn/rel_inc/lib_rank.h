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

#ifndef _LIB_RANK_H
#define _LIB_RANK_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"

EC_BOOL rank_set_new(void **rank_set);

EC_BOOL rank_set_free(void *rank_set);

EC_BOOL rank_set_clean(void *rank_set);

UINT32 rank_set_incl(void *rank_set, const UINT32 rank);

UINT32 rank_set_excl(void *rank_set, const UINT32 rank);

UINT32 rank_set_print(const void *rank_set);

UINT32 rank_set_init(void *rank_set, const UINT32 comm_size);

UINT32 rank_set_default_init(void *rank_set);


#endif /*_LIB_RANK_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
