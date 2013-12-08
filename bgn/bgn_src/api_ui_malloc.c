/******************************************************************************
*
* Copyright (C) Chaoyong Zhou
* Email: bgnvendor@gmail.com 
* QQ: 312230917
*
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif/*__cplusplus*/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "api_ui_log.h"

void *api_ui_malloc(size_t size, const UINT32 location)
{
    void*           mem_allocated_ptr = NULL;
    mem_allocated_ptr = SAFE_MALLOC(size, location);
    //sys_log(LOGSTDOUT, "api_ui_malloc: mem_allocated_ptr = %lx\n", mem_allocated_ptr);
    return(void*)(mem_allocated_ptr);
}

void api_ui_free(void *ptr, const UINT32 location)
{
    if(ptr)
    {
        //sys_log(LOGSTDOUT, "api_ui_free: ptr = %lx\n", ptr);
        SAFE_FREE(ptr, location);
    }

    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

