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

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "cstack.h"

#include "mm.h"
#include "log.h"

#include "bgnctrl.h"

CSTACK *cstack_new(const UINT32 mm_type, const UINT32 location)
{
    return clist_new(mm_type, location);
}

void cstack_free(CSTACK *cstack, const UINT32 location)
{
    clist_free(cstack, location);
    return;
}


void cstack_init(CSTACK *cstack, const UINT32 mm_type, const UINT32 location)
{
   clist_init(cstack, mm_type, location);
}

EC_BOOL cstack_is_empty(const CSTACK *cstack)
{
    return clist_is_empty(cstack);
}

CSTACK_DATA * cstack_push(CSTACK *cstack, void *data)
{
    return clist_push_back(cstack, data);
}

void *cstack_pop(CSTACK *cstack)
{
    return clist_pop_back(cstack);
}

void *cstack_top(const CSTACK *cstack)
{
    return clist_back(cstack);
}

UINT32 cstack_depth(const CSTACK *cstack)
{
    return clist_size(cstack);
}

void cstack_loop(const CSTACK *cstack, void (*handler)(void *))
{
    clist_loop_back(cstack, handler);
    return;
}


void cstack_clean(CSTACK *cstack, void (*cleaner)(void *))
{
   clist_clean(cstack, cleaner);
    return;
}

void cstack_print(LOG *log, const CSTACK *cstack, void (*print)(LOG *, const void *))
{
    /*print from bottom to top*/
    clist_print(log, cstack, print);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
