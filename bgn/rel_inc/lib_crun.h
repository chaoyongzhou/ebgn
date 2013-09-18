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

#ifndef _LIB_CRUN_H
#define _LIB_CRUN_H

#include <stdio.h>
#include <stdlib.h>

#include "lib_type.h"
#include "lib_cstring.h"
#include "lib_log.h"

UINT32 usr_run_01(const CSTRING *cstring);
UINT32 usr_run_02(const CSTRING *cstring_01, const CSTRING *cstring_02, CSTRING *cstring_03);


#endif /*_LIB_CRUN_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

