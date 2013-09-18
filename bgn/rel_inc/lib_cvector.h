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

#ifndef _LIB_CVECTOR_H
#define _LIB_CVECTOR_H

#include "lib_type.h"

#define CVECTOR_CODEC_ENCODER         ((UINT32) 1)
#define CVECTOR_CODEC_ENCODER_SIZE    ((UINT32) 2)
#define CVECTOR_CODEC_DECODER         ((UINT32) 3)
#define CVECTOR_CODEC_INIT            ((UINT32) 4)


typedef  void *(*CVECTOR_DATA_MALLOC)();
typedef  void (*CVECTOR_DATA_CLONE)(const void *, void *);
typedef  EC_BOOL (*CVECTOR_DATA_CMP)(const void *, const void *);
typedef  EC_BOOL (*CVECTOR_DATA_PREV_FILTER)(const void *, const void *);
typedef  EC_BOOL (*CVECTOR_DATA_POST_FILTER)(const void *, const void *);
typedef  EC_BOOL (*CVECTOR_DATA_VOTER)(const void *, const void *);
typedef void (*CVECTOR_DATA_CLEANER)(void *);
typedef void (*CVECTOR_DATA_LOCATION_CLEANER)(void *, const UINT32);
typedef void (*CVECTOR_DATA_HANDLER)(void *);
typedef  void (*CVECTOR_DATA_PRINT)(LOG *, const void *);
typedef  void (*CVECTOR_DATA_LEVEL_PRINT)(LOG *, const void *, const UINT32);

typedef UINT32 (*CVECTOR_DATA_ENCODER)(const UINT32, const void *, UINT8 *, const UINT32, UINT32 *);
typedef UINT32 (*CVECTOR_DATA_ENCODER_SIZE)(const UINT32, const void *, UINT32 *);
typedef UINT32 (*CVECTOR_DATA_DECODER)(const UINT32, const UINT8 *, const UINT32, UINT32 *, void *);
typedef UINT32 (*CVECTOR_DATA_INIT)(const UINT32, void *);
typedef UINT32 (*CVECTOR_DATA_CLEAN)(const UINT32, void *);
typedef UINT32 (*CVECTOR_DATA_FREE)(const UINT32, void *);


void *cvector_new(const UINT32 capacity, const UINT32 mm_type, const UINT32 location);

void cvector_free(void *cvector, const UINT32 location);

void cvector_init(void *cvector, const UINT32 capacity, const UINT32 mm_type, const UINT32 lock_enable_flag, const UINT32 location);

void cvector_clone(const void *cvector_src, void *cvector_des, void *(*cvector_data_malloc)(), void (*cvector_data_clone)(const void *, void *));

void cvector_clone_with_prev_filter(const void *cvector_src, void *cvector_des, const void *condition, EC_BOOL (*filter)(const void *, const void *), void *(*cvector_data_malloc)(), void (*cvector_data_clone)(const void *, void *));
void cvector_clone_with_post_filter(const void *cvector_src, void *cvector_des, const void *condition, EC_BOOL (*filter)(const void *, const void *), void *(*cvector_data_malloc)(), void (*cvector_data_clone)(const void *, void *));

EC_BOOL cvector_is_empty(const void *cvector);

EC_BOOL cvector_expand(void *cvector);

EC_BOOL cvector_cmp(const void *cvector_1st, const void *cvector_2nd, EC_BOOL (*cmp)(const void *, const void *));

UINT32 cvector_push(void *cvector, const void *data);

void *cvector_pop(void *cvector);

void *cvector_get(const void *cvector, const UINT32 pos);

void *cvector_set(void *cvector, const UINT32 pos, const void *data);

UINT32 cvector_capacity(const void *cvector);

UINT32 cvector_size(const void *cvector);

void cvector_codec_set(void *cvector, const UINT32 data_mem_type);

void *cvector_codec_get(const void *cvector, const UINT32 choice);

void cvector_codec_clone(const void *cvector_src, void *cvector_des);

void cvector_loop_front(const void *cvector, void (*handler)(void *));

void cvector_loop_back(const void *cvector, void (*handler)(void *));

UINT32 cvector_search_front(const void *cvector, const void *data, EC_BOOL (*cmp)(const void *, const void *));

UINT32 cvector_search_back(const void *cvector, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void *cvector_vote(const void *cvector, EC_BOOL (*voter)(const void *, const void *));

void *cvector_vote_with_prev_filter(const void *cvector, const void *condition, EC_BOOL (*filter)(const void *, const void *), EC_BOOL (*voter)(const void *, const void *));
void *cvector_vote_with_post_filter(const void *cvector, const void *condition, EC_BOOL (*filter)(const void *, const void *), EC_BOOL (*voter)(const void *, const void *));

/*note: clone cvector_src to the tail of cvector_des*/
void cvector_clone(const void *cvector_src, void *cvector_des, void *(*cvector_data_malloc)(), void (*cvector_data_clone)(const void *, void *));

void *cvector_erase(void *cvector, const UINT32 pos);

void cvector_clean(void *cvector, void (*cleaner)(void *), const UINT32 location);

void cvector_clean_with_location(void *cvector, void (*cleaner)(void *, const UINT32), const UINT32 location);

void cvector_clean_with_modi(void *cvector, const UINT32 modi, UINT32 (*cleaner)(const UINT32, void *), const UINT32 location);

void cvector_print(LOG *log, const void *cvector, void (*handler)(LOG *, const void *));

EC_BOOL cvector_loop(void *cvector, 
                         void *handler_retval_addr, EC_BOOL (*handler_retval_checker)(const void *), 
                         const UINT32 func_para_num, const UINT32 cvector_data_pos,
                         const UINT32 handler_func_addr,...);

#endif /*_LIB_CVECTOR_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
