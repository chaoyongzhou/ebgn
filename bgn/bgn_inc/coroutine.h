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

#ifndef _COROUTINE_H
#define _COROUTINE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>

#include "type.h"
#include "log.h"
#include "coroutine.inc"

#define COROUTINE_NODE_MASTER         (coroutine_pool_get_master(TASK_BRD_CROUTINE_POOL(task_brd_default_get())))
#define COROUTINE_NODE_CUR            (coroutine_pool_get_slave(TASK_BRD_CROUTINE_POOL(task_brd_default_get())))

#define __COROUTINE_WAIT() do{\
    COROUTINE_NODE *__master = COROUTINE_NODE_MASTER;\
    COROUTINE_NODE *__slave = COROUTINE_NODE_CUR;\
    coroutine_node_wait_and_swap_task(__slave, __master);\
}while(0)



#define COROUTINE_CLEANUP_PUSH(coroutine_cancel_routine, coroutine_cancel_para) do{}while(0)

#define COROUTINE_CLEANUP_POP(flag)   do{}while(0)

#define COROUTINE_EXIT(ptr) do{}while(0)

#define COROUTINE_TEST_CANCEL() do{}while(0)

COROUTINE_MUTEX *coroutine_mutex_new(const UINT32 location);

EC_BOOL coroutine_mutex_init(COROUTINE_MUTEX *coroutine_mutex, const UINT32 flag, const UINT32 location);

EC_BOOL coroutine_mutex_clean(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location);

void    coroutine_mutex_free(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location);

EC_BOOL coroutine_mutex_lock(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location);

EC_BOOL coroutine_mutex_unlock(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location);

COROUTINE_RWLOCK *coroutine_rwlock_new(const UINT32 location);

EC_BOOL coroutine_rwlock_init(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 flag, const UINT32 location);

EC_BOOL coroutine_rwlock_clean(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location);

void    coroutine_rwlock_free(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location);

EC_BOOL coroutine_rwlock_rdlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location);

EC_BOOL coroutine_rwlock_wrlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location);

EC_BOOL coroutine_rwlock_unlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location);

COROUTINE_COND *coroutine_cond_new(const UINT32 location);

EC_BOOL coroutine_cond_init(COROUTINE_COND *coroutine_cond, const UINT32 location);

EC_BOOL coroutine_cond_clean(COROUTINE_COND *coroutine_cond, const UINT32 location);

void    coroutine_cond_free(COROUTINE_COND *coroutine_cond, const UINT32 location);

EC_BOOL coroutine_cond_reserve(COROUTINE_COND *coroutine_cond, const UINT32 counter, const UINT32 location);

EC_BOOL coroutine_cond_release(COROUTINE_COND *coroutine_cond, const UINT32 location);

EC_BOOL coroutine_cond_release_all(COROUTINE_COND *coroutine_cond, const UINT32 location);

EC_BOOL coroutine_cond_wait(COROUTINE_COND *coroutine_cond, const UINT32 location);

UINT32  coroutine_cond_spy(COROUTINE_COND *coroutine_cond, const UINT32 location);

void coroutine_cancel();

COROUTINE_NODE *coroutine_self();

COROUTINE_NODE *coroutine_master();

COROUTINE_NODE *coroutine_node_new(COROUTINE_NODE *coroutine_node_next);

EC_BOOL coroutine_node_init(COROUTINE_NODE *coroutine_node, COROUTINE_NODE *coroutine_node_next);

UINT32 coroutine_node_clean(COROUTINE_NODE *coroutine_node);

UINT32 coroutine_node_free(COROUTINE_NODE *coroutine_node);

EC_BOOL coroutine_node_make_task(COROUTINE_NODE *coroutine_node, const UINT32 start_routine_addr, const UINT32 arg_num, va_list arg_list);

EC_BOOL coroutine_node_get_task(COROUTINE_NODE *coroutine_node);

EC_BOOL coroutine_node_set_task(COROUTINE_NODE *coroutine_node);

EC_BOOL coroutine_node_swap_task(COROUTINE_NODE *coroutine_node_save, COROUTINE_NODE *coroutine_node_to);

EC_BOOL coroutine_node_wait_and_swap_task(COROUTINE_NODE *coroutine_node_save, COROUTINE_NODE *coroutine_node_to);

UINT32 coroutine_node_shutdown(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_node_busy_to_idle(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_node_busy_to_tail(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool);

void coroutine_node_print(LOG *log, const COROUTINE_NODE *coroutine_node);

COROUTINE_POOL * coroutine_pool_new(const UINT32 coroutine_num, const UINT32 flag);

UINT32 coroutine_pool_init(COROUTINE_POOL *coroutine_pool);

/*create coroutine_num coroutine_nodes and add them to coroutine_pool*/
UINT32 coroutine_pool_create(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num);

UINT32 coroutine_pool_expand(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num);

UINT32 coroutine_pool_shrink(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num_to_shrink);

UINT32 coroutine_pool_shutdown(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_clean(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_free(COROUTINE_POOL *coroutine_pool);

COROUTINE_NODE * coroutine_pool_reserve_no_lock(COROUTINE_POOL *coroutine_pool);

COROUTINE_NODE * coroutine_pool_reserve(COROUTINE_POOL *coroutine_pool);

COROUTINE_NODE * coroutine_pool_load_no_lock(COROUTINE_POOL *coroutine_pool, const UINT32 start_routine_addr, const UINT32 para_num, va_list para_list);

COROUTINE_NODE * coroutine_pool_load(COROUTINE_POOL *coroutine_pool, const UINT32 start_routine_addr, const UINT32 para_num, ...);

COROUTINE_NODE *coroutine_pool_get_master(COROUTINE_POOL *coroutine_pool);

COROUTINE_NODE *coroutine_pool_get_slave(COROUTINE_POOL *coroutine_pool);

/*endless loop*/
void coroutine_pool_run(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_size_no_lock(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_idle_num_no_lock(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_busy_num_no_lock(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_num_info_no_lock(COROUTINE_POOL *coroutine_pool, UINT32 *idle_num, UINT32 *busy_num, UINT32 *total_num);

UINT32 coroutine_pool_size(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_idle_num(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_busy_num(COROUTINE_POOL *coroutine_pool);

UINT32 coroutine_pool_num_info(COROUTINE_POOL *coroutine_pool, UINT32 *idle_num, UINT32 *busy_num, UINT32 *total_num);

void coroutine_pool_print(LOG *log, COROUTINE_POOL *coroutine_pool);


#endif/*_COROUTINE_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

