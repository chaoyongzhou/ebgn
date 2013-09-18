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

#ifndef _LIB_CTIMER_H
#define _LIB_CTIMER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 

#include <signal.h>
#include <time.h>  
#include <sys/time.h>

#include "lib_type.h"

/**
*
* start CTIMER module
*
**/
UINT32 ctimer_start(const UINT32 ctimer_expire_delta);

/**
*
* end CTIMER module
*
**/
void ctimer_end();

/**
*
* new CTIMER_NODE
*
**/
void *ctimer_node_new();

/**
*
* init CTIMER_NODE
*
**/
UINT32 ctimer_node_init(void *ctimer_node);

/**
*
* clean CTIMER_NODE
*
**/
UINT32 ctimer_node_clean(void *ctimer_node);

/**
*
* free CTIMER_NODE
*
**/
UINT32 ctimer_node_free(void *ctimer_node);

/**
*
* add CTIMER_NODE
*
**/
UINT32 ctimer_node_add(void *ctimer_node, const UINT32 timeout, const void * func_retval_addr, const UINT32 func_id, ...);

/**
*
* delete CTIMER_NODE
*
**/
UINT32 ctimer_node_del(void *ctimer_node);

/**
*
* num of CTIMER_NODE
*
**/
UINT32 ctimer_node_num();

/**
*
* start CTIMER_NODE (start a timer)
*
**/
UINT32 ctimer_node_start(void *ctimer_node);

/**
*
* stop CTIMER_NODE (stop a timer)
*
**/
UINT32 ctimer_node_stop(void *ctimer_node);

/**
*
* update all CTIMER_NODE and trigger event if timer expired
*
**/
void ctimer_update(int signal);

/**
*
* create a event trigger
*
**/
UINT32 ctimer_create();

/**
*
* stop and clean up all CTIMER_NODE
*
**/
UINT32 ctimer_clean();

/**
*
* get current time in msec
*
**/
UINT32 ctimer_cur_msec();

/**
*
* sleep msec
*
**/
UINT32 ctimer_sleep_msec(const UINT32 msec);

void ctimer_node_print(LOG *log, const void *ctimer_node);
void ctimer_citimer_print(LOG *log, const void *citimer);
void ctimer_print(LOG *log, const void *ctimer_md);

#endif/* _LIB_CTIMER_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

