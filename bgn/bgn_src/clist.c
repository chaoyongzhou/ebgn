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

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>

#include "type.h"
#include "clist.h"

#include "cstring.h"

#include "mm.h"
#include "log.h"

#include "bgnctrl.h"

#include "cmutex.h"

#include "debug.h"

#if (SWITCH_OFF == CLIST_STATIC_MEM_SWITCH)
#define SAFE_CLIST_DATA_MALLOC(clist_data, __location__)     (clist_data) = (CLIST_DATA *)SAFE_MALLOC(sizeof(CLIST_DATA), __location__)
#define SAFE_CLIST_DATA_FREE(clist_data, __location__)       SAFE_FREE(clist_data, __location__)
#define SAFE_CLIST_MALLOC(clist, __location__)               (clist) = (CLIST *)SAFE_MALLOC(sizeof(CLIST), __location__)
#define SAFE_CLIST_FREE(clist, __location__)                 SAFE_FREE(clist, __location__)
#endif

#if (SWITCH_ON == CLIST_STATIC_MEM_SWITCH)
#define SAFE_CLIST_DATA_MALLOC(clist_data, __location__)     alloc_static_mem(MD_TASK, 0, MM_CLIST_DATA, (void **)&(clist_data), (__location__))
#define SAFE_CLIST_DATA_FREE(clist_data, __location__)       free_static_mem(MD_TASK, 0, MM_CLIST_DATA, (void *)(clist_data), (__location__))
#define SAFE_CLIST_MALLOC(clist, __location__)               alloc_static_mem(MD_TASK, 0, MM_CLIST, (void **)&(clist), (__location__))
#define SAFE_CLIST_FREE(clist, __location__)                 free_static_mem(MD_TASK, 0, MM_CLIST, (void *)(clist), (__location__))
#endif

static CLIST_DATA *clist_data_malloc_default()
{
    CLIST_DATA *clist_data;
    SAFE_CLIST_DATA_MALLOC(clist_data, LOC_CLIST_0001);
    return (clist_data);

}

static void clist_data_free_default(CLIST_DATA *clist_data)
{
    SAFE_CLIST_DATA_FREE(clist_data, LOC_CLIST_0002);
    return;
}

/*for safe reason, when data handler is not given, set to default null function*/
static void clist_null_default(void *data)
{
    return;
}

static EC_BOOL clist_null_default_with_modi(const UINT32 modi, void *data)
{
    return (EC_TRUE);
}

static EC_BOOL clist_cmp_default(const void *data_1, const void *data_2)
{
    if(data_1 == data_2)
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

static EC_BOOL clist_walker_default(const void *data_1, const void *data_2)
{
    if(data_1 <= data_2)
    {
        //dbg_log(SEC_0044_CLIST, 9)(LOGSTDOUT, "[DEBUG] clist_walker_default: %p <= %p => true\n", data_1, data_2);
        return (EC_TRUE);
    }
    //dbg_log(SEC_0044_CLIST, 9)(LOGSTDOUT, "[DEBUG] clist_walker_default: %p > %p => false\n", data_1, data_2);
    return (EC_FALSE);
}

EC_BOOL clist_checker_default(const void * retval)
{
    return ((EC_BOOL)retval);
}

UINT32 clist_type(const CLIST *clist)
{
    return clist->data_mm_type;
}

UINT32 clist_type_set(CLIST *clist, const UINT32 data_mm_type)
{
    clist->data_mm_type = data_mm_type;
    return (0);
}

void clist_codec_set(CLIST *clist, const UINT32 data_mm_type)
{
    TYPE_CONV_ITEM *type_conv_item;

    //CLIST_LOCK(clist, LOC_CLIST_0003);
    clist->data_mm_type = data_mm_type;

    type_conv_item = dbg_query_type_conv_item_by_mm(data_mm_type);
    if(NULL_PTR != type_conv_item)
    {
        clist->data_encoder      = (CLIST_DATA_ENCODER     )TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item);
        clist->data_encoder_size = (CLIST_DATA_ENCODER_SIZE)TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item);
        clist->data_decoder      = (CLIST_DATA_DECODER     )TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item);
        clist->data_init         = (CLIST_DATA_INIT        )TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item);
        clist->data_clean        = (CLIST_DATA_CLEAN       )TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item);
        clist->data_free         = (CLIST_DATA_FREE        )TYPE_CONV_ITEM_VAR_FREE_FUNC(type_conv_item);
    }
    else
    {
        clist->data_encoder      = NULL_PTR;
        clist->data_encoder_size = NULL_PTR;
        clist->data_decoder      = NULL_PTR;
        clist->data_init         = NULL_PTR;
        clist->data_clean        = NULL_PTR;
        clist->data_free         = NULL_PTR;
    }

    //CLIST_UNLOCK(clist, LOC_CLIST_0004);

    return;
}

void *clist_codec_get(const CLIST *clist, const UINT32 choice)
{
    switch(choice)
    {
        case CLIST_CODEC_ENCODER:
            return (void *)clist->data_encoder;
        case CLIST_CODEC_ENCODER_SIZE:
            return (void *)clist->data_encoder_size;
        case CLIST_CODEC_DECODER:
            return (void *)clist->data_decoder;
        case CLIST_CODEC_INIT:
            return (void *)clist->data_init;
        case CLIST_CODEC_CLEAN:
            return (void *)clist->data_clean;
        case CLIST_CODEC_FREE:
            return (void *)clist->data_free;
    }

    dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_codec_get: invalid choice %ld\n", choice);
    return (NULL_PTR);
}

void clist_codec_clone(const CLIST *clist_src, CLIST *clist_des)
{
    /*NOTE: due to function pointer is different depending on processes or machine */
    /*      here ask current process to fetch them locally*/
    clist_codec_set(clist_des, clist_src->data_mm_type);
/*
    clist_des->data_mm_type      = clist_src->data_mm_type     ;
    clist_des->data_encoder      = clist_src->data_encoder     ;
    clist_des->data_encoder_size = clist_src->data_encoder_size;
    clist_des->data_decoder      = clist_src->data_decoder     ;
    clist_des->data_init         = clist_src->data_init        ;
*/
    return;
}

CLIST *clist_new(const UINT32 mm_type, const UINT32 location)
{
    CLIST *clist;

    SAFE_CLIST_MALLOC(clist, location);

    if(clist)
    {
        clist_init(clist, mm_type, location);
    }
    return (clist);
}

void clist_free(CLIST *clist, const UINT32 location)
{
    CLIST_CLEAN_LOCK(clist, location);
    SAFE_CLIST_FREE(clist, location);
    return;
}

void clist_free_with_modi(CLIST *clist, const UINT32 modi)
{
    clist_clean_with_modi(clist, modi, clist->data_free);

    CLIST_CLEAN_LOCK(clist, LOC_CLIST_0005);
    SAFE_CLIST_FREE(clist, LOC_CLIST_0006);
    return;
}

void clist_init(CLIST *clist, const UINT32 mm_type, const UINT32 location)
{
    CLIST_HEAD_INIT(clist);

    clist->size = 0;

    clist_codec_set(clist, mm_type);

    CLIST_INIT_LOCK(clist, location);
    return;
}

UINT32 clist_init_0(const UINT32 modi, CLIST *clist)
{
    clist_init(clist, MM_IGNORE, LOC_CLIST_0007);
    return (0);
}

UINT32 clist_clean_0(const UINT32 modi, CLIST *clist)
{
    /*seems it is quite NOT necessary to clean codec setting! but if set, we may be in greate trouble*/
    clist_clean_with_modi(clist, modi, clist->data_free);
    return (0);
}

UINT32 clist_free_0(const UINT32 modi, CLIST *clist)
{
    clist_free_with_modi(clist, modi);
    return (0);
}


/*note: clone clist_src to the tail of clist_des*/
void clist_clone(const CLIST *clist_src, CLIST *clist_des, void *(*clist_data_data_malloc)(), void (*clist_data_data_clone)(const void *, void *))
{
    CLIST_DATA *clist_data_src;
    void *data_src;
    void *data_des;

    CLIST_LOCK(clist_src, LOC_CLIST_0008);
    CLIST_LOCK(clist_des, LOC_CLIST_0009);

    CLIST_LOOP_NEXT(clist_src, clist_data_src)
    {
        data_des = clist_data_data_malloc();
        data_src = CLIST_DATA_DATA(clist_data_src);
        clist_data_data_clone(data_src, data_des);
        clist_push_back_no_lock(clist_des, data_des);
    }

    CLIST_UNLOCK(clist_des, LOC_CLIST_0010);
    CLIST_UNLOCK(clist_src, LOC_CLIST_0011);

    return;
}

EC_BOOL clist_is_empty(const CLIST *clist)
{
    CLIST_LOCK(clist, LOC_CLIST_0012);

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0013);
        return (EC_TRUE);
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0014);
    return (EC_FALSE);
}

CLIST_DATA * clist_push_back(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    //sys_log(LOGSTDOUT, "[DEBUG] clist_push_back: push data %p to list %p\n", data, clist);
    CLIST_LOCK(clist, LOC_CLIST_0015);

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_DATA_ADD_BACK(clist, clist_data);

    clist->size ++;

    CLIST_UNLOCK(clist, LOC_CLIST_0016);
    return clist_data;
}

CLIST_DATA * clist_push_front(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CLIST_0017);

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_DATA_ADD_FRONT(clist, clist_data);

    clist->size ++;

    CLIST_UNLOCK(clist, LOC_CLIST_0018);
    return clist_data;
}

void *clist_pop_back(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0019);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0020);
        return (void *)0;
    }

    clist_data = CLIST_LAST_NODE(clist);
    CLIST_DATA_DEL(clist_data);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);

    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0021);
    return (data);
}

void *clist_pop_front(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0022);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0023);
        return (void *)0;
    }

    clist_data = CLIST_FIRST_NODE(clist);
    CLIST_DATA_DEL(clist_data);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);

    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0024);
    return data;
}

CLIST_DATA * clist_push_back1(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_LOCK(clist, LOC_CLIST_0025);
    CLIST_DATA_ADD_BACK(clist, clist_data);

    clist->size ++;

    CLIST_UNLOCK(clist, LOC_CLIST_0026);
    return clist_data;
}

CLIST_DATA * clist_push_front1(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_LOCK(clist, LOC_CLIST_0027);
    CLIST_DATA_ADD_FRONT(clist, clist_data);

    clist->size ++;

    CLIST_UNLOCK(clist, LOC_CLIST_0028);
    return clist_data;
}

void *clist_pop_back1(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0029);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0030);
        return (void *)0;
    }

    clist_data = CLIST_LAST_NODE(clist);
    CLIST_DATA_DEL(clist_data);
    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0031);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);

    return (data);
}

void *clist_pop_front1(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0032);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0033);
        return (void *)0;
    }

    clist_data = CLIST_FIRST_NODE(clist);
    CLIST_DATA_DEL(clist_data);

    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0034);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);
    return data;
}

void *clist_back(const CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0035);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0036);
        return (void *)0;
    }

    clist_data = CLIST_LAST_NODE(clist);
    data = CLIST_DATA_DATA(clist_data);

    CLIST_UNLOCK(clist, LOC_CLIST_0037);
    return data;
}

void *clist_front(const CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0038);
    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0039);
        return (void *)0;
    }

    clist_data = CLIST_FIRST_NODE(clist);
    data = CLIST_DATA_DATA(clist_data);

    CLIST_UNLOCK(clist, LOC_CLIST_0040);
    return data;
}

void *clist_data(const CLIST_DATA *clist_data)
{
    return CLIST_DATA_DATA(clist_data);
}

CLIST_DATA *clist_first(const CLIST *clist)
{
    CLIST_DATA *clist_data_first;

    CLIST_LOCK(clist, LOC_CLIST_0041);
    clist_data_first = CLIST_FIRST_NODE(clist);
    if(clist_data_first == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0042);
        return (CLIST_DATA *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0043);
    return clist_data_first;
}

CLIST_DATA *clist_last(const CLIST *clist)
{
    CLIST_DATA *clist_data_last;

    CLIST_LOCK(clist, LOC_CLIST_0044);
    clist_data_last = CLIST_LAST_NODE(clist);
    if(clist_data_last == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0045);
        return (CLIST_DATA *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0046);
    return clist_data_last;
}

CLIST_DATA *clist_next(const CLIST *clist, const CLIST_DATA *clist_data)
{
    CLIST_DATA *clist_data_next;

    if(0 == clist_data)
    {
        return (CLIST_DATA *)0;
    }

    CLIST_LOCK(clist, LOC_CLIST_0047);
    clist_data_next = CLIST_DATA_NEXT(clist_data);
    if(clist_data_next == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0048);
        return (CLIST_DATA *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0049);
    return clist_data_next;
}

CLIST_DATA *clist_prev(const CLIST *clist, const CLIST_DATA *clist_data)
{
    CLIST_DATA *clist_data_prev;

    if(0 == clist_data)
    {
        return (CLIST_DATA *)0;
    }

    CLIST_LOCK(clist, LOC_CLIST_0050);
    clist_data_prev = CLIST_DATA_PREV(clist_data);
    if(clist_data_prev == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0051);
        return (CLIST_DATA *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0052);
    return clist_data_prev;
}

void *clist_first_data(const CLIST *clist)
{
    CLIST_DATA *clist_data_first;

    CLIST_LOCK(clist, LOC_CLIST_0053);
    clist_data_first = CLIST_FIRST_NODE(clist);
    if(clist_data_first == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0054);
        return (void *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0055);
    return (void *)CLIST_DATA_DATA(clist_data_first);
}

void *clist_last_data(const CLIST *clist)
{
    CLIST_DATA *clist_data_last;

    CLIST_LOCK(clist, LOC_CLIST_0056);
    clist_data_last = CLIST_LAST_NODE(clist);
    if(clist_data_last == CLIST_NULL_NODE(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0057);
        return (void *)0;
    }

    CLIST_UNLOCK(clist, LOC_CLIST_0058);
    return (void *)CLIST_DATA_DATA(clist_data_last);
}

UINT32 clist_size(const CLIST *clist)
{
    return clist->size;
}

void clist_loop_front(const CLIST *clist, void (*handler)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0059);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        (handler)( data );
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0060);
    return;
}

void clist_loop_back(const CLIST *clist, void (*handler)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0061);
    CLIST_LOOP_PREV(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        (handler)( data );
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0062);
    return;
}

EC_BOOL clist_loop_front_with_modi(const CLIST *clist, const UINT32 modi, EC_BOOL (*handler)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0063);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(EC_FALSE == (handler)( modi, data ))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0064);
            return (EC_FALSE);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0065);
    return (EC_TRUE);
}

EC_BOOL clist_loop_back_with_modi(const CLIST *clist, const UINT32 modi, EC_BOOL (*handler)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0066);
    CLIST_LOOP_PREV(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(EC_FALSE == (handler)( modi, data ))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0067);
            return (EC_FALSE);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0068);
    return (EC_TRUE);
}

void clist_print(LOG *log, const CLIST *clist, void (*print)(LOG *, const void *))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    CLIST_LOCK(clist, LOC_CLIST_0069);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        sys_log(log, "(null)\n");

        CLIST_UNLOCK(clist, LOC_CLIST_0070);
        return;
    }

    sys_log(log, "size = %ld\n", clist->size);

    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        sys_log(log, "No. %ld: [%p]", pos ++, clist_data);

        if(0 != print)
        {
            (print)( log, data );
        }
        else
        {
            sys_print(log, " %lx\n", data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0071);
    return;
}

void clist_print_level(LOG *log, const CLIST *clist, const UINT32 level, void (*print)(LOG *, const void *, const UINT32))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    CLIST_LOCK(clist, LOC_CLIST_0072);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        sys_log(log, "(null)\n");

        CLIST_UNLOCK(clist, LOC_CLIST_0073);
        return;
    }

    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(0 != print)
        {
            (print)( log, data, level );
        }
        else
        {
            sys_log(log, "No. %ld: ", pos ++);
            sys_print(log, " %lx\n", data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0074);
    return;
}

void clist_print_plain(LOG *log, const CLIST *clist, void (*print)(LOG *, const void *))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    CLIST_LOCK(clist, LOC_CLIST_0075);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0076);
        return;
    }

    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        if(0 != print)
        {
            (print)( log, data );
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0077);
    return;
}

void clist_print_plain_level(LOG *log, const CLIST *clist, const UINT32 level, void (*print)(LOG *, const void *, const UINT32))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    CLIST_LOCK(clist, LOC_CLIST_0078);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0079);
        return;
    }

    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(0 != print)
        {
            (print)( log, data, level );
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0080);
    return;
}

void clist_sprint(CSTRING *cstring, const CLIST *clist, void (*sprint)(CSTRING *, const void *))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    CLIST_LOCK(clist, LOC_CLIST_0081);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        cstring_format(cstring, "(null)\n");

        CLIST_UNLOCK(clist, LOC_CLIST_0082);
        return;
    }
    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        cstring_format(cstring, "No. %ld: ", pos ++);

        if(0 != sprint)
        {
            (sprint)( cstring, data );
        }
        else
        {
            cstring_format(cstring, " %lx\n", data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0083);
    return;
}

/**
*   let clist is c0 < c1 < c2 < ... < ck
* where "<" is a kind of order
*   voter is the justment of the order:
* when ci < cj, voter(ci, cj) return EC_TRUE; otherwise, return EC_FALSE
* then, clist_vote will return the lowest one in the order sequence: c0
*
**/
void *clist_vote(const CLIST *clist, EC_BOOL (*voter)(void *, void *))
{
    CLIST_DATA *clist_data_cur;
    CLIST_DATA *clist_data_best;

    CLIST_LOCK(clist, LOC_CLIST_0084);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0085);
        return (void *)0;
    }

    clist_data_best = CLIST_FIRST_NODE(clist);
    CLIST_LOOP_NEXT(clist, clist_data_cur)
    {
        /*when cur is better than the best, set best = cur*/
        if(EC_TRUE == voter(CLIST_DATA_DATA(clist_data_cur), CLIST_DATA_DATA(clist_data_best)))
        {
            clist_data_best = clist_data_cur;
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0086);
    return CLIST_DATA_DATA(clist_data_best);
}

CLIST_DATA * clist_search_front(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CLIST_0087);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0088);
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0089);
            return clist_data;
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0090);
    return (NULL_PTR);
}

CLIST_DATA * clist_search_back(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CLIST_0091);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0092);
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_PREV(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0093);
            return clist_data;
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0094);
    return (NULL_PTR);
}

void * clist_search_data_front(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CLIST_0095);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0096);
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0097);
            return CLIST_DATA_DATA(clist_data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0098);
    return (NULL_PTR);
}

void * clist_search_data_back(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CLIST_0099);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0100);
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_PREV(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0101);
            return CLIST_DATA_DATA(clist_data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0102);
    return (NULL_PTR);
}

CLIST_DATA *clist_insert_front(CLIST *clist, CLIST_DATA *clist_data, const void *data)
{
    CLIST_DATA *clist_data_new;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    CLIST_LOCK(clist, LOC_CLIST_0103);
    clist_data_new = clist_data_malloc_default();
    if(clist_data_new)
    {
        CLIST_DATA_DATA(clist_data_new) = (void *)data;
        list_base_add_tail(CLIST_DATA_NODE(clist_data_new), CLIST_DATA_NODE(clist_data));
        clist->size ++;
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0104);
    return clist_data_new;
}

CLIST_DATA *clist_insert_back(CLIST *clist, CLIST_DATA *clist_data, const void *data)
{
    CLIST_DATA *clist_data_new;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    CLIST_LOCK(clist, LOC_CLIST_0105);
    clist_data_new = clist_data_malloc_default();
    if(clist_data_new)
    {
        CLIST_DATA_DATA(clist_data_new) = (void *)data;
        list_base_add(CLIST_DATA_NODE(clist_data_new), CLIST_DATA_NODE(clist_data));
        clist->size ++;
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0106);
    return clist_data_new;
}

void *clist_rmv(CLIST *clist, CLIST_DATA *clist_data)
{
    void *data;

    CLIST_LOCK(clist, LOC_CLIST_0107);
    data = CLIST_DATA_DATA(clist_data);
    CLIST_DATA_DEL(clist_data);
    clist_data_free_default(clist_data);
    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0108);
    return (data);
}

void *clist_del(CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;
    void *cur_data;

    CLIST_LOCK(clist, LOC_CLIST_0109);
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        CLIST_UNLOCK(clist, LOC_CLIST_0110);
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            cur_data = CLIST_DATA_DATA(clist_data);
            CLIST_DATA_DEL(clist_data);
            clist_data_free_default(clist_data);
            clist->size --;

            CLIST_UNLOCK(clist, LOC_CLIST_0111);
            return (cur_data);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0112);
    return (NULL_PTR);
}

void *clist_erase(CLIST *clist, CLIST_DATA *clist_data)
{
    void *data;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    CLIST_LOCK(clist, LOC_CLIST_0113);
    CLIST_DATA_DEL(clist_data);
    data = CLIST_DATA_DATA(clist_data);

    clist_data_free_default(clist_data);

    /*WARNING: if clist_data does not belong to this clist, clist->size will be changed exceptionally*/
    /*the solution maybe discard size field and count size by run-through the clist*/
    clist->size --;

    CLIST_UNLOCK(clist, LOC_CLIST_0114);
    return data;
}

/*move from current position to tail*/
EC_BOOL clist_move_back(CLIST *clist, CLIST_DATA *clist_data)
{
    CLIST_LOCK(clist, LOC_CLIST_0115);
    CLIST_DATA_DEL(clist_data);
    CLIST_DATA_ADD_BACK(clist, clist_data);
    CLIST_UNLOCK(clist, LOC_CLIST_0116);
    return (EC_TRUE);
}

/*move from current position to head*/
EC_BOOL clist_move_front(CLIST *clist, CLIST_DATA *clist_data)
{
    CLIST_LOCK(clist, LOC_CLIST_0117);
    CLIST_DATA_DEL(clist_data);
    CLIST_DATA_ADD_FRONT(clist, clist_data);
    CLIST_UNLOCK(clist, LOC_CLIST_0118);
    return (EC_TRUE);
}

void clist_clean(CLIST *clist, void (*cleaner)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    /*accept null cleaner*/
    if(0 == cleaner)
    {
        cleaner = clist_null_default;
    }

    CLIST_LOCK(clist, LOC_CLIST_0119);
    while( EC_FALSE == CLIST_IS_EMPTY(clist) )
    {
        clist_data = CLIST_LAST_NODE(clist);
        CLIST_DATA_DEL(clist_data);

        data = CLIST_DATA_DATA(clist_data);
        cleaner(data);

        clist_data_free_default(clist_data);
    }

    clist->size = 0;

    CLIST_UNLOCK(clist, LOC_CLIST_0120);
    return;
}

void clist_clean_with_modi(CLIST *clist, const UINT32 modi, EC_BOOL (*cleaner)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    /*accept null cleaner*/
    if(0 == cleaner)
    {
        if(0 == clist->data_free)
        {
            cleaner = clist_null_default_with_modi;
        }
        else
        {
            cleaner = clist->data_free;
        }
    }

    CLIST_LOCK(clist, LOC_CLIST_0121);
    while( EC_FALSE == CLIST_IS_EMPTY(clist) )
    {
        clist_data = CLIST_LAST_NODE(clist);
        CLIST_DATA_DEL(clist_data);

        data = CLIST_DATA_DATA(clist_data);
        cleaner(modi, data);

        clist_data_free_default(clist_data);
    }

    clist->size = 0;

    CLIST_UNLOCK(clist, LOC_CLIST_0122);
    return;
}

static CLIST_DATA *__clist_get_most_small_no_lock(CLIST *clist, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data_des;/*to return*/
    CLIST_DATA *clist_data_cur;

    clist_data_des = CLIST_FIRST_NODE(clist);
    if(clist_data_des == CLIST_NULL_NODE(clist))
    {
        return (NULL_PTR);
    }
    
    for(clist_data_cur = CLIST_DATA_NEXT(clist_data_des);  clist_data_cur != CLIST_NULL_NODE(clist); clist_data_cur = CLIST_DATA_NEXT(clist_data_cur))
    {
        if(EC_FALSE == cmp(CLIST_DATA_DATA(clist_data_des), CLIST_DATA_DATA(clist_data_cur)))
        {
            clist_data_des = clist_data_cur;
        }
    }

    return (clist_data_des);
}

static CLIST_DATA *__clist_get_most_small_with_modi_no_lock(CLIST *clist, const UINT32 modi, EC_BOOL (*cmp)(const UINT32, const void *, const void *))
{
    CLIST_DATA *clist_data_des;/*to return*/
    CLIST_DATA *clist_data_cur;

    clist_data_des = CLIST_FIRST_NODE(clist);
    if(clist_data_des == CLIST_NULL_NODE(clist))
    {
        return (NULL_PTR);
    }
    
    for(clist_data_cur = CLIST_DATA_NEXT(clist_data_des);  clist_data_cur != CLIST_NULL_NODE(clist); clist_data_cur = CLIST_DATA_NEXT(clist_data_cur))
    {
        if(EC_FALSE == cmp(modi, CLIST_DATA_DATA(clist_data_des), CLIST_DATA_DATA(clist_data_cur)))
        {
            clist_data_des = clist_data_cur;
        }
    }

    return (clist_data_des);
}

static void __clist_move(CLIST *clist_src, CLIST *clist_des)
{
    LIST_NODE *head_src;
    LIST_NODE *head_des;

    head_src = CLIST_HEAD(clist_src);
    head_des = CLIST_HEAD(clist_des);

    head_des->next = head_src->next;
    head_des->prev = head_src->prev;

    head_des->next->prev = head_des;
    head_des->prev->next = head_des;

    INIT_LIST_BASE_HEAD(head_src);
    return;
}

void clist_bubble_sort(CLIST *clist, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST clist_t;/*tmp list*/
    CLIST_DATA *clist_data;

    CLIST_HEAD_INIT(&clist_t);
    
    CLIST_LOCK(clist, LOC_CLIST_0123);
    __clist_move(clist, &clist_t);
    CLIST_UNLOCK(clist, LOC_CLIST_0124);

    while(NULL_PTR != (clist_data = __clist_get_most_small_no_lock(&clist_t, cmp)))
    {
        CLIST_DATA_DEL(clist_data);
        CLIST_DATA_ADD_BACK(clist, clist_data);  
    }  

    return;
}

void clist_bubble_sort_with_modi(CLIST *clist, const UINT32 modi, EC_BOOL (*cmp)(const UINT32, const void *, const void *))
{
    CLIST clist_t;/*tmp list*/
    CLIST_DATA *clist_data;

    CLIST_HEAD_INIT(&clist_t);
    
    CLIST_LOCK(clist, LOC_CLIST_0125);
    __clist_move(clist, &clist_t);
    CLIST_UNLOCK(clist, LOC_CLIST_0126);

    while(NULL_PTR != (clist_data = __clist_get_most_small_with_modi_no_lock(&clist_t, modi, cmp)))
    {
        CLIST_DATA_DEL(clist_data);
        CLIST_DATA_ADD_BACK(clist, clist_data);  
    }  

    return;
}

/*handler_func_addr format: void *func(xx,clist data,yy, zz)*/
/*clist_data_pos range from 0 to func_para_num - 1*/
EC_BOOL clist_loop(CLIST *clist, 
                     void *handler_retval_addr, EC_BOOL (*handler_retval_checker)(const void *), 
                     const UINT32 func_para_num, const UINT32 clist_data_pos,
                     const UINT32 handler_func_addr,...)
{
    UINT32 func_para_value[ MAX_NUM_OF_FUNC_PARAS ];
    UINT32 index;

    CLIST_DATA *clist_data;

    va_list ap;

    if(0 == handler_func_addr)
    {
        return (EC_TRUE);
    }

    if(0 == func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop: func_para_num must be larger than 1\n");
        return (EC_FALSE);
    }

    if(MAX_NUM_OF_FUNC_PARAS < func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop: func_para_num %ld overflow which must be smaller than %ld\n", 
                           func_para_num, MAX_NUM_OF_FUNC_PARAS);
        return (EC_FALSE);
    }

    if(clist_data_pos >= func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop: invalid setting where clist_data_pos %ld >= func_para_num %ld\n", 
                           clist_data_pos, func_para_num);
        return (EC_FALSE);
    }

    va_start(ap, handler_func_addr);
    for( index = 0; index < func_para_num; index ++ )
    {
        func_para_value[ index ] = va_arg(ap, UINT32);
    }
    va_end(ap);

    CLIST_LOCK(clist, LOC_CLIST_0127);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        void *data;
        data = CLIST_DATA_DATA(clist_data);

        func_para_value[ clist_data_pos ] = (UINT32)data;

        if(EC_FALSE == dbg_caller(handler_func_addr, func_para_num, func_para_value, (UINT32 *)handler_retval_addr))
        {
            dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop: dbg_caller failed\n");
            CLIST_UNLOCK(clist, LOC_CLIST_0128);
            return (EC_FALSE);
        }

        if(NULL_PTR != handler_retval_checker 
        && NULL_PTR != handler_retval_addr
        && EC_FALSE == handler_retval_checker(handler_retval_addr))
        {
            CLIST_UNLOCK(clist, LOC_CLIST_0129);
            return (EC_FALSE);
        }
    }
    CLIST_UNLOCK(clist, LOC_CLIST_0130);

    return ( EC_TRUE );
}


/**
*
* assume in list: a_1 > a_2 > a_3 > ... > a_n, given b, insert it to list in order
*  a_i > a_j  <--> walker(a_i, a_j) = EC_TRUE
*
**/
CLIST_DATA *clist_walk_and_insert(CLIST *clist, const void *data, EC_BOOL (*walker)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(NULL_PTR == walker)
    {
        walker = clist_walker_default;
    }

    CLIST_LOCK(clist, LOC_CLIST_0131);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_FALSE == walker(CLIST_DATA_DATA(clist_data), data))
        {   
            break;
        }
    }
    clist_data = clist_insert_front_no_lock(clist, clist_data, data);
    CLIST_UNLOCK(clist, LOC_CLIST_0132);
    return (clist_data);
}

/*--------------------------------- no lock interface ---------------------------------*/
CLIST *clist_new_no_lock(const UINT32 mm_type, const UINT32 location)
{
    CLIST *clist;

    SAFE_CLIST_MALLOC(clist, location);

    if(clist)
    {
        clist_init_no_lock(clist, mm_type, location);
    }
    return clist;
}

void clist_free_no_lock(CLIST *clist, const UINT32 location)
{
    SAFE_CLIST_FREE(clist, location);
    return;
}

void clist_free_with_modi_no_lock(CLIST *clist, const UINT32 modi)
{
    clist_clean_with_modi_no_lock(clist, modi, clist->data_free);

    SAFE_CLIST_FREE(clist, LOC_CLIST_0133);
    return;
}

void clist_init_no_lock(CLIST *clist, const UINT32 mm_type, const UINT32 location)
{
    CLIST_HEAD_INIT(clist);

    clist->size = 0;

    clist_codec_set(clist, mm_type);

    return;
}

/*note: clone clist_src to the tail of clist_des*/
void clist_clone_no_lock(const CLIST *clist_src, CLIST *clist_des, void *(*clist_data_data_malloc)(), void (*clist_data_data_clone)(const void *, void *))
{
    CLIST_DATA *clist_data_src;
    void *data_src;
    void *data_des;

    CLIST_LOOP_NEXT(clist_src, clist_data_src)
    {
        data_des = clist_data_data_malloc();
        data_src = CLIST_DATA_DATA(clist_data_src);
        clist_data_data_clone(data_src, data_des);
        clist_push_back_no_lock(clist_des, data_des);
    }

    return;
}

EC_BOOL clist_is_empty_no_lock(const CLIST *clist)
{
    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

CLIST_DATA * clist_push_back_no_lock(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_DATA_ADD_BACK(clist, clist_data);

    clist->size ++;

    return clist_data;
}

CLIST_DATA * clist_push_front_no_lock(CLIST *clist, const void *data)
{
    CLIST_DATA *clist_data;

    clist_data = clist_data_malloc_default();
    CLIST_DATA_DATA(clist_data) = (void *)data;

    CLIST_DATA_ADD_FRONT(clist, clist_data);

    clist->size ++;

    return clist_data;
}

void *clist_pop_back_no_lock(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        return (void *)0;
    }

    clist_data = CLIST_LAST_NODE(clist);
    CLIST_DATA_DEL(clist_data);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);

    clist->size --;

    return (data);
}

void *clist_pop_front_no_lock(CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        return (void *)0;
    }

    clist_data = CLIST_FIRST_NODE(clist);
    CLIST_DATA_DEL(clist_data);

    data = CLIST_DATA_DATA(clist_data);
    clist_data_free_default(clist_data);

    clist->size --;

    return data;
}

void *clist_back_no_lock(const CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        return (void *)0;
    }

    clist_data = CLIST_LAST_NODE(clist);
    data = CLIST_DATA_DATA(clist_data);

    return data;
}

void *clist_front_no_lock(const CLIST *clist)
{
    CLIST_DATA *clist_data;
    void *data;

    if(EC_TRUE == clist_is_empty_no_lock(clist))
    {
        return (void *)0;
    }

    clist_data = CLIST_FIRST_NODE(clist);
    data = CLIST_DATA_DATA(clist_data);

    return data;
}

CLIST_DATA *clist_first_no_lock(const CLIST *clist)
{
    CLIST_DATA *clist_data_first;

    clist_data_first = CLIST_FIRST_NODE(clist);
    if(clist_data_first == CLIST_NULL_NODE(clist))
    {
        return (CLIST_DATA *)0;
    }

    return clist_data_first;
}

CLIST_DATA *clist_last_no_lock(const CLIST *clist)
{
    CLIST_DATA *clist_data_last;

    clist_data_last = CLIST_LAST_NODE(clist);
    if(clist_data_last == CLIST_NULL_NODE(clist))
    {
        return (CLIST_DATA *)0;
    }

    return clist_data_last;
}

CLIST_DATA *clist_next_no_lock(const CLIST *clist, const CLIST_DATA *clist_data)
{
    CLIST_DATA *clist_data_next;

    if(0 == clist_data)
    {
        return (CLIST_DATA *)0;
    }

    clist_data_next = CLIST_DATA_NEXT(clist_data);
    if(clist_data_next == CLIST_NULL_NODE(clist))
    {
        return (CLIST_DATA *)0;
    }

    return clist_data_next;
}

CLIST_DATA *clist_prev_no_lock(const CLIST *clist, const CLIST_DATA *clist_data)
{
    CLIST_DATA *clist_data_prev;

    if(0 == clist_data)
    {
        return (CLIST_DATA *)0;
    }

    clist_data_prev = CLIST_DATA_PREV(clist_data);
    if(clist_data_prev == CLIST_NULL_NODE(clist))
    {
        return (CLIST_DATA *)0;
    }

    return clist_data_prev;
}

void clist_loop_front_no_lock(const CLIST *clist, void (*handler)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        (handler)( data );
    }
    return;
}

void clist_loop_back_no_lock(const CLIST *clist, void (*handler)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOOP_PREV(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        (handler)( data );
    }
    return;
}

EC_BOOL clist_loop_front_with_modi_no_lock(const CLIST *clist, const UINT32 modi, EC_BOOL (*handler)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(EC_FALSE == (handler)( modi, data ))
        {
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL clist_loop_back_with_modi_no_lock(const CLIST *clist, const UINT32 modi, EC_BOOL (*handler)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    CLIST_LOOP_PREV(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);
        if(EC_FALSE == (handler)( modi, data ))
        {
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}


void clist_print_no_lock(LOG *log, const CLIST *clist, void (*print)(LOG *, const void *))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        sys_log(log, "(null)\n");

        return;
    }
    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        sys_log(log, "No. %ld: ", pos ++);

        if(0 != print)
        {
            (print)( log, data );
        }
        else
        {
            sys_print(log, " %lx\n", data);
        }
    }
    return;
}

void clist_print_level_no_lock(LOG *log, const CLIST *clist, const UINT32 level, void (*print)(LOG *, const void *, const UINT32))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        sys_log(log, "(null)\n");

        return;
    }
    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        if(0 != print)
        {
            (print)( log, data, level );
        }
        else
        {
            sys_log(log, "No. %ld: ", pos ++);
            sys_print(log, " %lx\n", data);
        }
    }
    return;
}

void clist_sprint_no_lock(CSTRING *cstring, const CLIST *clist, void (*sprint)(CSTRING *, const void *))
{
    CLIST_DATA *clist_data;
    void *data;

    UINT32 pos;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        cstring_format(cstring, "(null)\n");

        return;
    }
    pos = 0;
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        data = CLIST_DATA_DATA(clist_data);

        cstring_format(cstring, "No. %ld: ", pos ++);

        if(0 != sprint)
        {
            (sprint)( cstring, data );
        }
        else
        {
            cstring_format(cstring, " %lx\n", data);
        }
    }
    return;
}

/**
*   let clist is c0 < c1 < c2 < ... < ck
* where "<" is a kind of order
*   voter is the justment of the order:
* when ci < cj, voter(ci, cj) return EC_TRUE; otherwise, return EC_FALSE
* then, clist_vote will return the lowest one in the order sequence: c0
*
**/
void *clist_vote_no_lock(const CLIST *clist, EC_BOOL (*voter)(void *, void *))
{
    CLIST_DATA *clist_data_cur;
    CLIST_DATA *clist_data_best;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (void *)0;
    }

    clist_data_best = CLIST_FIRST_NODE(clist);
    CLIST_LOOP_NEXT(clist, clist_data_cur)
    {
        /*when cur is better than the best, set best = cur*/
        if(EC_TRUE == voter(CLIST_DATA_DATA(clist_data_cur), CLIST_DATA_DATA(clist_data_best)))
        {
            clist_data_best = clist_data_cur;
        }
    }
    return CLIST_DATA_DATA(clist_data_best);
}

CLIST_DATA * clist_search_front_no_lock(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            return clist_data;
        }
    }

    return (NULL_PTR);
}

CLIST_DATA * clist_search_back_no_lock(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_PREV(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            return clist_data;
        }
    }
    return (NULL_PTR);
}

void * clist_search_data_front_no_lock(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            return CLIST_DATA_DATA(clist_data);
        }
    }

    return (NULL_PTR);
}

void * clist_search_data_back_no_lock(const CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_PREV(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            return CLIST_DATA_DATA(clist_data);
        }
    }
    return (NULL_PTR);
}


CLIST_DATA *clist_insert_front_no_lock(CLIST *clist, CLIST_DATA *clist_data, const void *data)
{
    CLIST_DATA *clist_data_new;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    clist_data_new = clist_data_malloc_default();
    if(clist_data_new)
    {
        CLIST_DATA_DATA(clist_data_new) = (void *)data;
        list_base_add_tail(CLIST_DATA_NODE(clist_data_new), CLIST_DATA_NODE(clist_data));
        clist->size ++;
    }
    return clist_data_new;
}

CLIST_DATA *clist_insert_back_no_lock(CLIST *clist, CLIST_DATA *clist_data, const void *data)
{
    CLIST_DATA *clist_data_new;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    clist_data_new = clist_data_malloc_default();
    if(clist_data_new)
    {
        CLIST_DATA_DATA(clist_data_new) = (void *)data;
        list_base_add(CLIST_DATA_NODE(clist_data_new), CLIST_DATA_NODE(clist_data));
        clist->size ++;
    }

    return clist_data_new;
}

void *clist_rmv_no_lock(CLIST *clist, CLIST_DATA *clist_data)
{
    void *data;

    data = CLIST_DATA_DATA(clist_data);
    CLIST_DATA_DEL(clist_data);
    clist_data_free_default(clist_data);
    clist->size --;

    return (data);
}

void *clist_del_no_lock(CLIST *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST_DATA *clist_data;
    void *cur_data;

    if(EC_TRUE == CLIST_IS_EMPTY(clist))
    {
        return (NULL_PTR);
    }

    if(NULL_PTR == cmp)
    {
        cmp = clist_cmp_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_TRUE == cmp(CLIST_DATA_DATA(clist_data), data))
        {
            cur_data = CLIST_DATA_DATA(clist_data);
            CLIST_DATA_DEL(clist_data);
            clist_data_free_default(clist_data);
            clist->size --;

            return (cur_data);
        }
    }
    return (NULL_PTR);
}

void *clist_erase_no_lock(CLIST *clist, CLIST_DATA *clist_data)
{
    void *data;

    if(NULL_PTR == clist_data)
    {
        return (NULL_PTR);
    }

    CLIST_DATA_DEL(clist_data);
    data = CLIST_DATA_DATA(clist_data);

    clist_data_free_default(clist_data);

    /*WARNING: if clist_data does not belong to this clist, clist->size will be changed exceptionally*/
    /*the solution maybe discard size field and count size by run-through the clist*/
    clist->size --;

    return data;
}


/*move from current position to tail*/
EC_BOOL clist_move_back_no_lock(CLIST *clist, CLIST_DATA *clist_data)
{
    CLIST_DATA_DEL(clist_data);
    CLIST_DATA_ADD_BACK(clist, clist_data);
    return (EC_TRUE);
}

/*move from current position to head*/
EC_BOOL clist_move_front_no_lock(CLIST *clist, CLIST_DATA *clist_data)
{
    CLIST_DATA_DEL(clist_data);
    CLIST_DATA_ADD_FRONT(clist, clist_data);
    return (EC_TRUE);
}

void clist_clean_no_lock(CLIST *clist, void (*cleaner)(void *))
{
    CLIST_DATA *clist_data;
    void *data;

    /*accept null cleaner*/
    if(0 == cleaner)
    {
        cleaner = clist_null_default;
    }

    while( EC_FALSE == CLIST_IS_EMPTY(clist) )
    {
        clist_data = CLIST_LAST_NODE(clist);
        CLIST_DATA_DEL(clist_data);

        data = CLIST_DATA_DATA(clist_data);
        cleaner(data);

        clist_data_free_default(clist_data);
    }

    clist->size = 0;

    return;
}

void clist_clean_with_modi_no_lock(CLIST *clist, const UINT32 modi, EC_BOOL (*cleaner)(const UINT32, void *))
{
    CLIST_DATA *clist_data;
    void *data;

    /*accept null cleaner*/
    if(0 == cleaner)
    {
        if(0 == clist->data_free)
        {
            cleaner = clist_null_default_with_modi;
        }
        else
        {
            cleaner = clist->data_free;
        }
    }

    while( EC_FALSE == CLIST_IS_EMPTY(clist) )
    {
        clist_data = CLIST_LAST_NODE(clist);
        CLIST_DATA_DEL(clist_data);

        data = CLIST_DATA_DATA(clist_data);
        cleaner(modi, data);

        clist_data_free_default(clist_data);
    }

    clist->size = 0;
    return;
}

void clist_bubble_sort_no_lock(CLIST *clist, EC_BOOL (*cmp)(const void *, const void *))
{
    CLIST clist_t;/*tmp list*/
    CLIST_DATA *clist_data;

    CLIST_HEAD_INIT(&clist_t);
    
    __clist_move(clist, &clist_t);

    while(NULL_PTR != (clist_data = __clist_get_most_small_no_lock(&clist_t, cmp)))
    {
        CLIST_DATA_DEL(clist_data);
        CLIST_DATA_ADD_BACK(clist, clist_data);  
    }  

    return;
}

void clist_bubble_sort_with_modi_no_lock(CLIST *clist, const UINT32 modi, EC_BOOL (*cmp)(const UINT32, const void *, const void *))
{
    CLIST clist_t;/*tmp list*/
    CLIST_DATA *clist_data;

    CLIST_HEAD_INIT(&clist_t);
    
    __clist_move(clist, &clist_t);

    while(NULL_PTR != (clist_data = __clist_get_most_small_with_modi_no_lock(&clist_t, modi, cmp)))
    {
        CLIST_DATA_DEL(clist_data);
        CLIST_DATA_ADD_BACK(clist, clist_data);  
    }  

    return;
}

/*handler_func_addr format: void *func(xx,clist data,yy, zz)*/
/*clist_data_pos range from 0 to func_para_num - 1*/
EC_BOOL clist_loop_no_lock(CLIST *clist, 
                                 void *handler_retval_addr, EC_BOOL (*handler_retval_checker)(const void *), 
                                 const UINT32 func_para_num, const UINT32 clist_data_pos,
                                 const UINT32 handler_func_addr,...)
{
    UINT32 func_para_value[ MAX_NUM_OF_FUNC_PARAS ];
    UINT32 index;

    CLIST_DATA *clist_data;

    va_list ap;

    if(0 == handler_func_addr)
    {
        return (EC_TRUE);
    }

    if(0 == func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop_no_lock: func_para_num must be larger than 1\n");
        return (EC_FALSE);
    }

    if(MAX_NUM_OF_FUNC_PARAS < func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop_no_lock: func_para_num %ld overflow which must be smaller than %ld\n", 
                           func_para_num, MAX_NUM_OF_FUNC_PARAS);
        return (EC_FALSE);
    }

    if(clist_data_pos >= func_para_num)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop_no_lock: invalid setting where clist_data_pos %ld >= func_para_num %ld\n", 
                           clist_data_pos, func_para_num);
        return (EC_FALSE);
    }

    va_start(ap, handler_func_addr);
    for( index = 0; index < func_para_num; index ++ )
    {
        func_para_value[ index ] = va_arg(ap, UINT32);
    }
    va_end(ap);

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        void *data;
        data = CLIST_DATA_DATA(clist_data);

        func_para_value[ clist_data_pos ] = (UINT32)data;

        if(EC_FALSE == dbg_caller(handler_func_addr, func_para_num, func_para_value, (UINT32 *)handler_retval_addr))
        {
            dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_loop_no_lock: dbg_caller failed\n");
            return (EC_FALSE);
        }

        if(NULL_PTR != handler_retval_checker 
        && NULL_PTR != handler_retval_addr
        && EC_FALSE == handler_retval_checker(handler_retval_addr))
        {
            return (EC_FALSE);
        }
    }

    return ( EC_TRUE );
}

/**
*
* assume in list: a_1 > a_2 > a_3 > ... > a_n, given b, insert it to list in order
*  a_i > a_j  <--> walker(a_i, a_j) = EC_TRUE
*
**/
CLIST_DATA *clist_walk_and_insert_no_lock(CLIST *clist, const void *data, EC_BOOL (*walker)(const void *, const void *))
{
    CLIST_DATA *clist_data;

    if(NULL_PTR == walker)
    {
        walker = clist_walker_default;
    }

    CLIST_LOOP_NEXT(clist, clist_data)
    {
        if(EC_FALSE == walker(CLIST_DATA_DATA(clist_data), data))
        {   
            break;
        }
    }

    clist_data = clist_insert_front_no_lock(clist, clist_data, data);
    return (clist_data);
}

EC_BOOL clist_self_check_no_lock(CLIST *clist)
{
    UINT32 size;
    UINT32 count;
    CLIST_DATA *clist_data;

    size = clist->size;
    count = 0;
    for((clist_data) = CLIST_FIRST_NODE(clist);  (clist_data) != CLIST_NULL_NODE(clist) && count < 2048; (clist_data) = CLIST_DATA_NEXT(clist_data))
    {
        count ++;
    }

    if(size != count)
    {
        dbg_log(SEC_0044_CLIST, 0)(LOGSTDOUT, "error:clist_self_check_no_lock: size = %ld but count = %ld\n", size, count);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
