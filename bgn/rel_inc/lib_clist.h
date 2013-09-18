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

#ifndef _LIB_CLIST_H
#define _LIB_CLIST_H

#include "lib_type.h"

UINT32 clist_type(const void *clist);

UINT32 clist_type_set(void *clist, const UINT32 data_mm_type);

void   clist_codec_set(void *clist, const UINT32 data_mm_type);

void * clist_codec_get(const void *clist, const UINT32 choice);

void   clist_codec_clone(const void *clist_src, void *clist_des);

void *clist_new(const UINT32 mm_type, const UINT32 location);

void clist_free(void *clist, const UINT32 location);

void clist_free_with_modi(void *clist, const UINT32 modi);

void clist_init(void *clist, const UINT32 mm_type, const UINT32 location);

UINT32 clist_init_0(const UINT32 modi, void *clist);

UINT32 clist_clean_0(const UINT32 modi, void *clist);

UINT32 clist_free_0(const UINT32 modi, void *clist);

void   clist_clone(const void *clist_src, void *clist_des, void *(*clist_data_data_malloc)(), void (*clist_data_data_clone)(const void *, void *));

EC_BOOL clist_is_empty(const void *clist);

void * clist_push_back(void *clist, const void *data);

void * clist_push_front(void *clist, const void *data);

void *clist_pop_back(void *clist);

void *clist_pop_front(void *clist);

void *clist_back(const void *clist);

void *clist_front(const void *clist);

void *clist_data(const void *clist_data);

void *clist_first(const void *clist);

void *clist_last(const void *clist);

void *clist_next(const void *clist, const void *clist_data);

void *clist_prev(const void *clist, const void *clist_data);

void *clist_first_data(const void *clist);

void *clist_last_data(const void *clist);

UINT32 clist_size(const void *clist);

void clist_loop_front(const void *clist, void (*handler)(void *));

void clist_loop_back(const void *clist, void (*handler)(void *));

void clist_print(LOG *log, const void *clist, void (*print)(LOG *, const void *));

void clist_print_level(LOG *log, const void *clist, const UINT32 level, void (*print)(LOG *, const void *, const UINT32));

void clist_sprint(CSTRING *cstring, const void *clist, void (*sprint)(CSTRING *, const void *));

void *clist_vote(const void *clist, EC_BOOL (*voter)(void *, void *));

void * clist_search_front(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_back(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_data_front(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_data_back(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void *clist_insert_front(void *clist, void *clist_data, const void *data);

void *clist_insert_back(void *clist, void *clist_data, const void *data);

void *clist_rmv(void *clist, void *clist_data);
void *clist_del(void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));
void *clist_erase(void *clist, void *clist_data);

void clist_clean(void *clist, void (*cleaner)(void *));

void clist_clean_with_modi(void *clist, const UINT32 modi, EC_BOOL (*cleaner)(const UINT32, void *));

EC_BOOL clist_loop(void *clist, 
                     void *handler_retval_addr, EC_BOOL (*handler_retval_checker)(const void *), 
                     const UINT32 func_para_num, const UINT32 clist_data_pos,
                     const UINT32 handler_func_addr,...);

/*--------------------------------- no lock interface ---------------------------------*/
void *clist_new_no_lock(const UINT32 mm_type, const UINT32 location);

void clist_free_no_lock(void *clist, const UINT32 location);

void clist_free_with_modi_no_lock(void *clist, const UINT32 modi);

void clist_init_no_lock(void *clist, const UINT32 mm_type, const UINT32 location);

/*note: clone clist_src to the tail of clist_des*/
void clist_clone_no_lock(const void *clist_src, void *clist_des, void *(*clist_data_data_malloc)(), void (*clist_data_data_clone)(const void *, void *));

EC_BOOL clist_is_empty_no_lock(const void *clist);

void * clist_push_back_no_lock(void *clist, const void *data);

void * clist_push_front_no_lock(void *clist, const void *data);

void *clist_pop_back_no_lock(void *clist);

void *clist_pop_front_no_lock(void *clist);

void *clist_back_no_lock(const void *clist);

void *clist_front_no_lock(const void *clist);

void *clist_first_no_lock(const void *clist);

void *clist_last_no_lock(const void *clist);

void *clist_next_no_lock(const void *clist, const void *clist_data);

void *clist_prev_no_lock(const void *clist, const void *clist_data);

void clist_loop_front_no_lock(const void *clist, void (*handler)(void *));

void clist_loop_back_no_lock(const void *clist, void (*handler)(void *));

void clist_print_no_lock(LOG *log, const void *clist, void (*print)(LOG *, const void *));

void clist_print_level_no_lock(LOG *log, const void *clist, const UINT32 level, void (*print)(LOG *, const void *, const UINT32));

void clist_sprint_no_lock(CSTRING *cstring, const void *clist, void (*sprint)(CSTRING *, const void *));

/**
*   let clist is c0 < c1 < c2 < ... < ck
* where "<" is a kind of order
*   voter is the justment of the order:
* when ci < cj, voter(ci, cj) return EC_TRUE; otherwise, return EC_FALSE
* then, clist_vote will return the lowest one in the order sequence: c0
*
**/
void *clist_vote_no_lock(const void *clist, EC_BOOL (*voter)(void *, void *));

void * clist_search_front_no_lock(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_back_no_lock(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_data_front_no_lock(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void * clist_search_data_back_no_lock(const void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void *clist_insert_front_no_lock(void *clist, void *clist_data, const void *data);

void *clist_insert_back_no_lock(void *clist, void *clist_data, const void *data);

void *clist_rmv_no_lock(void *clist, void *clist_data);

void *clist_del_no_lock(void *clist, const void *data, EC_BOOL (*cmp)(const void *, const void *));

void *clist_erase_no_lock(void *clist, void *clist_data);

void clist_clean_no_lock(void *clist, void (*cleaner)(void *));

EC_BOOL clist_loop_no_lock(void *clist, 
                                 void *handler_retval_addr, EC_BOOL (*handler_retval_checker)(const void *), 
                                 const UINT32 func_para_num, const UINT32 clist_data_pos,
                                 const UINT32 handler_func_addr,...);

void clist_clean_with_modi_no_lock(void *clist, const UINT32 modi, EC_BOOL (*cleaner)(const UINT32, void *));

EC_BOOL clist_self_check_no_lock(void *clist);

#endif /*_LIB_CLIST_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
