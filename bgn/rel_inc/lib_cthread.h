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

#ifndef _LIB_CTHREAD_H
#define _LIB_CTHREAD_H

#include <pthread.h>
#include "lib_type.h"

typedef pthread_t           CTHREAD_ID;

EC_BOOL cthread_wait(CTHREAD_ID cthread_id);

#endif/* _LIB_CTHREAD_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

