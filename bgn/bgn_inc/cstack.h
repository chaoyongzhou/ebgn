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

#ifndef _CSTACK_H
#define _CSTACK_H

#include "clist.h"

typedef CLIST_DATA CSTACK_DATA;
typedef CLIST CSTACK;

CSTACK *cstack_new(const UINT32 mm_type, const UINT32 location);
void    cstack_free(CSTACK *cstack, const UINT32 location);

void    cstack_init(CSTACK *cstack, const UINT32 mm_type, const UINT32 location);
EC_BOOL cstack_is_empty(const CSTACK *cstack);

CSTACK_DATA * cstack_push(CSTACK *cstack, void *data);
void  * cstack_pop(CSTACK *cstack);
void  * cstack_top(const CSTACK *cstack);
UINT32  cstack_depth(const CSTACK *cstack);

void cstack_loop(const CSTACK *cstack, void (*handler)(void *));
void cstack_clean(CSTACK *cstack, void (*cleaner)(void *));

void cstack_print(LOG *log, const CSTACK *cstack, void (*print)(LOG *, const void *));
#endif/* _CSTACK_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/

