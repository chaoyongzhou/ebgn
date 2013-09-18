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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>

#include "type.h"
#include "log.h"
#include "task.inc"
#include "task.h"
#include "coroutine.h"

COROUTINE_MUTEX *coroutine_mutex_new(const UINT32 location)
{
    COROUTINE_MUTEX      *coroutine_mutex;

    coroutine_mutex = (COROUTINE_MUTEX *)safe_malloc(sizeof(COROUTINE_MUTEX), LOC_COROUTINE_0001);
    if(NULL_PTR == coroutine_mutex)
    {
        sys_log(LOGSTDOUT, "error:coroutine_mutex_new: failed to alloc COROUTINE_MUTEX, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == coroutine_mutex_init(coroutine_mutex, COROUTINE_MUTEX_IGNORE_FLAG, location))
    {
        sys_log(LOGSTDOUT, "error:coroutine_mutex_init: failed to init coroutine_mutex %lx, called at %s:%ld\n", coroutine_mutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        safe_free(coroutine_mutex, LOC_COROUTINE_0002);
        return (NULL_PTR);
    }

    COROUTINE_MUTEX_INIT_LOCATION(coroutine_mutex);
    COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_NEW, location);

    return (coroutine_mutex);
}

EC_BOOL coroutine_mutex_init(COROUTINE_MUTEX *coroutine_mutex, const UINT32 flag, const UINT32 location)
{
    COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_INIT, location);
    COROUTINE_MUTEX_COUNTER(coroutine_mutex) = 0;    
    return (EC_TRUE);
}

EC_BOOL coroutine_mutex_clean(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location)
{
    COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_CLEAN, location);
    COROUTINE_MUTEX_COUNTER(coroutine_mutex) = 0;
    return (EC_TRUE);
}

void    coroutine_mutex_free(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location)
{
    if(NULL_PTR != coroutine_mutex)
    {
        COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_FREE, location);
        coroutine_mutex_clean(coroutine_mutex, location);
        safe_free(coroutine_mutex, LOC_COROUTINE_0003);
    }
    return;
}

EC_BOOL coroutine_mutex_lock(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location)
{
    COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_LOCK, location);
    ++ COROUTINE_MUTEX_COUNTER(coroutine_mutex);

    while(1 != COROUTINE_MUTEX_COUNTER(coroutine_mutex))
    {
        __COROUTINE_WAIT();
    }    
    
    return (EC_TRUE);
}

EC_BOOL coroutine_mutex_unlock(COROUTINE_MUTEX *coroutine_mutex, const UINT32 location)
{
    COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, COROUTINE_MUTEX_OP_UNLOCK, location);
    //ASSERT(0 < COROUTINE_MUTEX_COUNTER(coroutine_mutex));
    if(0 == COROUTINE_MUTEX_COUNTER(coroutine_mutex))
    {
        sys_log(LOGSTDOUT, "error:coroutine_mutex_unlock: found invalid at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        exit(0);
    }
    -- COROUTINE_MUTEX_COUNTER(coroutine_mutex);
    
    return (EC_TRUE);
}

COROUTINE_RWLOCK *coroutine_rwlock_new(const UINT32 location)
{
    COROUTINE_RWLOCK      *coroutine_rwlock;

    coroutine_rwlock = (COROUTINE_RWLOCK *)safe_malloc(sizeof(COROUTINE_RWLOCK), LOC_COROUTINE_0004);
    if(NULL_PTR == coroutine_rwlock)
    {
        sys_log(LOGSTDOUT, "error:coroutine_rwlock_new: failed to alloc COROUTINE_RWLOCK, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == coroutine_rwlock_init(coroutine_rwlock, COROUTINE_RWLOCK_IGNORE_FLAG, location))
    {
        sys_log(LOGSTDOUT, "error:coroutine_rwlock_init: failed to init coroutine_rwlock %lx, called at %s:%ld\n", coroutine_rwlock, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        safe_free(coroutine_rwlock, LOC_COROUTINE_0005);
        return (NULL_PTR);
    }

    COROUTINE_RWLOCK_INIT_LOCATION(coroutine_rwlock);
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_NEW, location);

    return (coroutine_rwlock);
}

EC_BOOL coroutine_rwlock_init(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 flag, const UINT32 location)
{
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_INIT, location);
    COROUTINE_RWLOCK_READERS(coroutine_rwlock) = 0;    
    COROUTINE_RWLOCK_WRITERS(coroutine_rwlock) = 0;    
    return (EC_TRUE);
}

EC_BOOL coroutine_rwlock_clean(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location)
{
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_CLEAN, location);
    COROUTINE_RWLOCK_READERS(coroutine_rwlock) = 0;    
    COROUTINE_RWLOCK_WRITERS(coroutine_rwlock) = 0;
    return (EC_TRUE);
}

void    coroutine_rwlock_free(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location)
{
    if(NULL_PTR != coroutine_rwlock)
    {
        COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_FREE, location);
        coroutine_rwlock_clean(coroutine_rwlock, location);
        safe_free(coroutine_rwlock, LOC_COROUTINE_0006);
    }
    return;
}

EC_BOOL coroutine_rwlock_rdlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location)
{
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_RDLOCK, location);
    ++ COROUTINE_RWLOCK_READERS(coroutine_rwlock);

    while(0 < COROUTINE_RWLOCK_WRITERS(coroutine_rwlock))
    {
        __COROUTINE_WAIT();
    }
    
    return (EC_TRUE);
}

EC_BOOL coroutine_rwlock_wrlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location)
{
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_WRLOCK, location);    

    while(0 < COROUTINE_RWLOCK_READERS(coroutine_rwlock) || 0 < COROUTINE_RWLOCK_WRITERS(coroutine_rwlock))
    {
        __COROUTINE_WAIT();
    }
    
    ++ COROUTINE_RWLOCK_WRITERS(coroutine_rwlock);
    
    return (EC_TRUE);
}

EC_BOOL coroutine_rwlock_unlock(COROUTINE_RWLOCK *coroutine_rwlock, const UINT32 location)
{
    COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, COROUTINE_RWLOCK_OP_UNLOCK, location);
    if(0 < COROUTINE_RWLOCK_WRITERS(coroutine_rwlock))
    {
        -- COROUTINE_RWLOCK_WRITERS(coroutine_rwlock);
    }
    else
    {
        -- COROUTINE_RWLOCK_READERS(coroutine_rwlock);
    }
    
    return (EC_TRUE);
}

COROUTINE_COND *coroutine_cond_new(const UINT32 location)
{
    COROUTINE_COND      *coroutine_cond;

    coroutine_cond = (COROUTINE_COND *)safe_malloc(sizeof(COROUTINE_COND), LOC_COROUTINE_0007);
    if(NULL_PTR == coroutine_cond)
    {
        sys_log(LOGSTDOUT, "error:coroutine_cond_new: failed to alloc COROUTINE_COND, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == coroutine_cond_init(coroutine_cond, location))
    {
        sys_log(LOGSTDOUT, "error:coroutine_cond_init: failed to init coroutine_cond %lx, called at %s:%ld\n", coroutine_cond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        safe_free(coroutine_cond, LOC_COROUTINE_0008);
        return (NULL_PTR);
    }

    COROUTINE_COND_INIT_LOCATION(coroutine_cond);
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_NEW, location);

    return (coroutine_cond);
}

EC_BOOL coroutine_cond_init(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_INIT, location);
    COROUTINE_COND_COUNTER(coroutine_cond) = 0;    
    return (EC_TRUE);
}

EC_BOOL coroutine_cond_clean(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_CLEAN, location);
    COROUTINE_COND_COUNTER(coroutine_cond) = 0;
    return (EC_TRUE);
}

void    coroutine_cond_free(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    if(NULL_PTR != coroutine_cond)
    {
        COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_FREE, location);
        coroutine_cond_clean(coroutine_cond, location);
        safe_free(coroutine_cond, LOC_COROUTINE_0009);
    }
    return;
}

EC_BOOL coroutine_cond_reserve(COROUTINE_COND *coroutine_cond, const UINT32 counter, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_RESERVE, location);
    COROUTINE_COND_COUNTER(coroutine_cond) += counter;
    
    return (EC_TRUE);
}

EC_BOOL coroutine_cond_release(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_RELEASE, location);
    -- COROUTINE_COND_COUNTER(coroutine_cond);    
    return (EC_TRUE);
}

EC_BOOL coroutine_cond_release_all(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_RELEASE, location);
    COROUTINE_COND_COUNTER(coroutine_cond) = 0;    
    return (EC_TRUE);
}

EC_BOOL coroutine_cond_wait(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    COROUTINE_COND_SET_LOCATION(coroutine_cond, COROUTINE_COND_OP_WAIT, location);
    while(0 < COROUTINE_COND_COUNTER(coroutine_cond))
    {
        __COROUTINE_WAIT();
    }  
    //sys_log(LOGSTDOUT, "[DEBUG] coroutine_cond_wait: now go back\n");
    return (EC_TRUE);
}

UINT32 coroutine_cond_spy(COROUTINE_COND *coroutine_cond, const UINT32 location)
{
    return COROUTINE_COND_COUNTER(coroutine_cond);
}

void coroutine_killme(void *args)
{
    return;
}

void coroutine_cleanup_push(void (*routine)(void*), void *arg)
{
    return;
}

void coroutine_cleanup_pop(int execute)
{
    if(execute)
    {
        //TODO:
        return;
    }
    return;
}

void coroutine_exit(void *value_ptr)
{
    return;
}

void coroutine_cancel()
{
    COROUTINE_NODE *__master = COROUTINE_NODE_MASTER;
    COROUTINE_NODE *__slave = COROUTINE_NODE_CUR;

    COROUTINE_NODE_STATUS(__slave) |= COROUTINE_IS_CANL;
    coroutine_node_swap_task(__slave, __master);
    return;
}

static void coroutine_unbind(COROUTINE_BIND *coroutine_bind)
{
    COROUTINE_NODE *coroutine_node;
    COROUTINE_POOL *coroutine_pool;

    coroutine_node = COROUTINE_BIND_NODE(coroutine_bind);
    coroutine_pool = COROUTINE_BIND_POOL(coroutine_bind);

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0010);
    if(COROUTINE_IS_IDLE & COROUTINE_NODE_STATUS(coroutine_node))
    {        
        clist_rmv_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));        

        sys_log(LOGSTDOUT, "coroutine_unbind: free idle coroutine_node %lx\n", coroutine_node);
        coroutine_node_free(coroutine_node);
    }
    else if(COROUTINE_IS_BUSY & COROUTINE_NODE_STATUS(coroutine_node))
    {
        clist_rmv_no_lock(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));

        sys_log(LOGSTDOUT, "coroutine_unbind: free busy coroutine_node %lx\n", coroutine_node);
        coroutine_node_free(coroutine_node);
    }
    else
    {
        sys_log(LOGSTDOUT, "coroutine_unbind: free coroutine_node %lx but status %lx\n", 
                            coroutine_node, COROUTINE_NODE_STATUS(coroutine_node));
        coroutine_node_free(coroutine_node);    
    }
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0011);

    return;
}

COROUTINE_NODE *coroutine_self()
{
    TASK_BRD *task_brd;
    
    task_brd = task_brd_default_get();
    return coroutine_pool_get_slave(TASK_BRD_CROUTINE_POOL(task_brd));
}

COROUTINE_NODE *coroutine_master()
{
    return COROUTINE_NODE_MASTER;
}

COROUTINE_NODE *coroutine_node_new(COROUTINE_NODE *coroutine_node_next)
{
    COROUTINE_NODE *coroutine_node;

    alloc_static_mem(MD_TASK, 0, MM_COROUTINE_NODE, &coroutine_node, LOC_COROUTINE_0012);
    if(NULL_PTR == coroutine_node)
    {
        sys_log(LOGSTDOUT, "error:coroutine_node_new: alloc COROUTINE_NODE failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == coroutine_node_init(coroutine_node, coroutine_node_next))
    {
        sys_log(LOGSTDOUT, "error:coroutine_node_new: init COROUTINE_NODE failed\n");
        free_static_mem(MD_TASK, 0, MM_COROUTINE_NODE, coroutine_node, LOC_COROUTINE_0013);
        return (NULL_PTR);
    }

    return (coroutine_node);
}

EC_BOOL coroutine_node_init(COROUTINE_NODE *coroutine_node, COROUTINE_NODE *coroutine_node_next)
{
    coroutine_node_get_task(coroutine_node);
    
    COROUTINE_NODE_STATUS(coroutine_node)  = COROUTINE_IS_IDLE;
    COROUTINE_NODE_MOUNTED(coroutine_node) = NULL_PTR;

    COROUTINE_NODE_STACK_SPACE(coroutine_node) = safe_malloc(COROUTINE_STACK_SIZE_DEFAULT, LOC_COROUTINE_0014);
    if(NULL_PTR == COROUTINE_NODE_STACK_SPACE(coroutine_node))
    {
        sys_log(LOGSTDOUT, "error:coroutine_node_init: malloc %ld bytes failed\n", COROUTINE_STACK_SIZE_DEFAULT);
        return (EC_FALSE);
    }
    COROUTINE_NODE_STACK_SIZE(coroutine_node) = COROUTINE_STACK_SIZE_DEFAULT;
    COROUTINE_NODE_RESUME_POINT(coroutine_node) = COROUTINE_NODE_TASK(coroutine_node_next);

    coroutine_cond_init(COROUTINE_NODE_COND(coroutine_node), LOC_COROUTINE_0015);

    return (EC_TRUE);
}

UINT32 coroutine_node_clean(COROUTINE_NODE *coroutine_node)
{
    if(NULL_PTR != COROUTINE_NODE_STACK_SPACE(coroutine_node))
    {
        safe_free(COROUTINE_NODE_STACK_SPACE(coroutine_node), LOC_COROUTINE_0016);
        COROUTINE_NODE_STACK_SPACE(coroutine_node) = NULL_PTR;
    }    
    COROUTINE_NODE_STACK_SIZE(coroutine_node) = 0;
    COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_DOWN;
    COROUTINE_NODE_MOUNTED(coroutine_node) = NULL_PTR;
    coroutine_cond_clean(COROUTINE_NODE_COND(coroutine_node), LOC_COROUTINE_0017);
    
    return (0);
}

UINT32 coroutine_node_free(COROUTINE_NODE *coroutine_node)
{
    coroutine_node_clean(coroutine_node);
    free_static_mem(MD_TASK, 0, MM_COROUTINE_NODE, coroutine_node, LOC_COROUTINE_0018);
    return (0);
}

EC_BOOL coroutine_node_make_task(COROUTINE_NODE *coroutine_node, const UINT32 start_routine_addr, const UINT32 arg_num, va_list arg_list)
{
    UINT32 _arg_list[16];
    int    _arg_num;
    int    _arg_idx;

    ASSERT(16 >= arg_num);
    _arg_num = (int)arg_num;

    for(_arg_idx = 0; _arg_idx < _arg_num; _arg_idx ++)
    {
        _arg_list[ _arg_idx ] = va_arg(arg_list, UINT32);
    }

    #define PARA_VALUE(arg_list, x)    ((arg_list)[ (x) ])

    #define PARA_LIST_0(arg_list)    /*no parameter*/
    #define PARA_LIST_1(arg_list)    PARA_VALUE(arg_list, 0)
    #define PARA_LIST_2(arg_list)    PARA_LIST_1(arg_list) ,PARA_VALUE(arg_list, 1)
    #define PARA_LIST_3(arg_list)    PARA_LIST_2(arg_list) ,PARA_VALUE(arg_list, 2)
    #define PARA_LIST_4(arg_list)    PARA_LIST_3(arg_list) ,PARA_VALUE(arg_list, 3)
    #define PARA_LIST_5(arg_list)    PARA_LIST_4(arg_list) ,PARA_VALUE(arg_list, 4)
    #define PARA_LIST_6(arg_list)    PARA_LIST_5(arg_list) ,PARA_VALUE(arg_list, 5)
    #define PARA_LIST_7(arg_list)    PARA_LIST_6(arg_list) ,PARA_VALUE(arg_list, 6)
    #define PARA_LIST_8(arg_list)    PARA_LIST_7(arg_list) ,PARA_VALUE(arg_list, 7)
    #define PARA_LIST_9(arg_list)    PARA_LIST_8(arg_list) ,PARA_VALUE(arg_list, 8)
    #define PARA_LIST_10(arg_list)   PARA_LIST_9(arg_list) ,PARA_VALUE(arg_list, 9)
    #define PARA_LIST_11(arg_list)   PARA_LIST_10(arg_list),PARA_VALUE(arg_list, 10)
    #define PARA_LIST_12(arg_list)   PARA_LIST_11(arg_list),PARA_VALUE(arg_list, 11)
    #define PARA_LIST_13(arg_list)   PARA_LIST_12(arg_list),PARA_VALUE(arg_list, 12)
    #define PARA_LIST_14(arg_list)   PARA_LIST_13(arg_list),PARA_VALUE(arg_list, 13)
    #define PARA_LIST_15(arg_list)   PARA_LIST_14(arg_list),PARA_VALUE(arg_list, 14)
    #define PARA_LIST_16(arg_list)   PARA_LIST_15(arg_list),PARA_VALUE(arg_list, 15)

    #define MAKE_CONTEXT_NO_PARA(__x__, start_routine_addr, arg_num, arg_list) \
            makecontext(COROUTINE_NODE_TASK(coroutine_node), ((void (*)(void))start_routine_addr), arg_num)

    #define MAKE_CONTEXT(__x__, start_routine_addr, arg_num, arg_list) \
            makecontext(COROUTINE_NODE_TASK(coroutine_node), ((void (*)(void))start_routine_addr), arg_num, PARA_LIST_##__x__(arg_list))

    switch(arg_num)
    {
        case 0:
            MAKE_CONTEXT_NO_PARA(0, start_routine_addr, _arg_num, _arg_list);
            break;
        case 1:
            MAKE_CONTEXT(1, start_routine_addr, _arg_num, _arg_list);
            break;
        case 2:
            MAKE_CONTEXT(2, start_routine_addr, _arg_num, _arg_list);
            break;
        case 3:
            MAKE_CONTEXT(3, start_routine_addr, _arg_num, _arg_list);
            break;
        case 4:
            MAKE_CONTEXT(4, start_routine_addr, _arg_num, _arg_list);
            break;
        case 5:
            MAKE_CONTEXT(5, start_routine_addr, _arg_num, _arg_list);
            break;
        case 6:
            MAKE_CONTEXT(6, start_routine_addr, _arg_num, _arg_list);
            break;
        case 7:
            MAKE_CONTEXT(7, start_routine_addr, _arg_num, _arg_list);
            break;
        case 8:
            MAKE_CONTEXT(8, start_routine_addr, _arg_num, _arg_list);
            break;
        case 9:
            MAKE_CONTEXT(9, start_routine_addr, _arg_num, _arg_list);
            break;
        case 10:
            MAKE_CONTEXT(10, start_routine_addr, _arg_num, _arg_list);
            break;
        case 11:
            MAKE_CONTEXT(11, start_routine_addr, _arg_num, _arg_list);
            break;
        case 12:
            MAKE_CONTEXT(12, start_routine_addr, _arg_num, _arg_list);
            break;
        case 13:
            MAKE_CONTEXT(13, start_routine_addr, _arg_num, _arg_list);
            break;
        case 14:
            MAKE_CONTEXT(14, start_routine_addr, _arg_num, _arg_list);
            break;
        case 15:
            MAKE_CONTEXT(15, start_routine_addr, _arg_num, _arg_list);
            break;
        case 16:
            MAKE_CONTEXT(16, start_routine_addr, _arg_num, _arg_list);
            break;
        default:
            sys_log(LOGSTDOUT, "error:coroutine_caller: arg num = %ld overflow\n", arg_num);
            return (EC_FALSE);
    }

    #undef PARA_VALUE

    #undef PARA_LIST_0
    #undef PARA_LIST_1
    #undef PARA_LIST_2
    #undef PARA_LIST_3
    #undef PARA_LIST_4
    #undef PARA_LIST_5
    #undef PARA_LIST_6
    #undef PARA_LIST_7
    #undef PARA_LIST_8
    #undef PARA_LIST_9
    #undef PARA_LIST_10
    #undef PARA_LIST_11
    #undef PARA_LIST_12
    #undef PARA_LIST_13
    #undef PARA_LIST_14
    #undef PARA_LIST_15
    #undef PARA_LIST_16

    #undef MAKE_CONTEXT    

    return (EC_TRUE);
}

EC_BOOL coroutine_node_get_task(COROUTINE_NODE *coroutine_node)
{
    if(-1 == getcontext(COROUTINE_NODE_TASK(coroutine_node)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL coroutine_node_set_task(COROUTINE_NODE *coroutine_node)
{
    if(-1 == setcontext(COROUTINE_NODE_TASK(coroutine_node)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL coroutine_node_swap_task(COROUTINE_NODE *coroutine_node_save, COROUTINE_NODE *coroutine_node_to)
{
    ASSERT(NULL_PTR != coroutine_node_save);
    if(NULL_PTR == coroutine_node_save)
    {
        setcontext(COROUTINE_NODE_TASK(coroutine_node_to));
        return (EC_TRUE);
    }
    
    if(0 == swapcontext(COROUTINE_NODE_TASK(coroutine_node_save), COROUTINE_NODE_TASK(coroutine_node_to)))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL coroutine_node_wait_and_swap_task(COROUTINE_NODE *coroutine_node_save, COROUTINE_NODE *coroutine_node_to)
{
    if(NULL_PTR == coroutine_node_save)
    {
        //setcontext(COROUTINE_NODE_TASK(coroutine_node_to));
        return (EC_TRUE);
    }

    COROUTINE_NODE_SET_WAIT_STATUS(coroutine_node_save);
    if(0 == swapcontext(COROUTINE_NODE_TASK(coroutine_node_save), COROUTINE_NODE_TASK(coroutine_node_to)))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

/*note: coroutine_node_shutdown will lock coroutine_pool, so DO NOT call it when coroutine_pool_shutdown*/
UINT32 coroutine_node_shutdown(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool)
{
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0019);

    COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_DOWN;

    if(COROUTINE_IS_IDLE & COROUTINE_NODE_STATUS(coroutine_node))
    {
        clist_rmv_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));
        cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0020);
        return (0);
    }

    if(COROUTINE_IS_BUSY & COROUTINE_NODE_STATUS(coroutine_node))
    {
        clist_rmv_no_lock(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));
        COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_CANL;
        cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0021);
        return (0);
    }

    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0022);
    return (0);
}

UINT32 coroutine_node_busy_to_idle(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool)
{
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0023);
    //sys_log(LOGSTDOUT, "[DEBUG] coroutine_node_busy_to_idle: coroutine_node %lx\n", coroutine_node);
    //coroutine_node_print(LOGSTDOUT, coroutine_node);

    clist_rmv_no_lock(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));
    COROUTINE_NODE_STATUS(coroutine_node)  = ((COROUTINE_NODE_STATUS(coroutine_node) & COROUTINE_HI_MASK) | COROUTINE_IS_IDLE);
    COROUTINE_NODE_MOUNTED(coroutine_node) = clist_push_back_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), coroutine_node);

    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0024);

    return (0);
}

UINT32 coroutine_node_busy_to_tail(COROUTINE_NODE *coroutine_node, COROUTINE_POOL *coroutine_pool)
{
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0025);
    if(1 < coroutine_pool_busy_num_no_lock(coroutine_pool))
    {
        CLIST_DATA_DEL(COROUTINE_NODE_MOUNTED(coroutine_node));
        CLIST_DATA_ADD_BACK(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), COROUTINE_NODE_MOUNTED(coroutine_node));
        //sys_log(LOGSTDOUT, "[DEBUG] coroutine_node_busy_to_tail: coroutine_node %lx\n", coroutine_node);
        //coroutine_node_print(LOGSTDOUT, coroutine_node);    
    }
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0026);

    return (0);
}

void coroutine_node_print(LOG *log, const COROUTINE_NODE *coroutine_node)
{
    sys_log(log, "coroutine_node %lx, status %lx, mounted %lx, stack space %lx, stack size %ld\n",
                coroutine_node,
                COROUTINE_NODE_STATUS(coroutine_node),
                COROUTINE_NODE_MOUNTED(coroutine_node),
                COROUTINE_NODE_STACK_SPACE(coroutine_node),
                COROUTINE_NODE_STACK_SIZE(coroutine_node)
                );

    //coroutine_task_print(log, COROUTINE_NODE_TASK(coroutine_node));

    return;
}

COROUTINE_POOL * coroutine_pool_new(const UINT32 coroutine_num, const UINT32 flag)
{
    COROUTINE_POOL *coroutine_pool;

    alloc_static_mem(MD_TASK, 0, MM_COROUTINE_POOL, &coroutine_pool, LOC_COROUTINE_0027);
    if(NULL_PTR == coroutine_pool)
    {
        sys_log(LOGSTDOUT, "error:coroutine_pool_new: alloc COROUTINE_POOL failed\n");
        return (NULL_PTR);
    }

    coroutine_pool_init(coroutine_pool);
    coroutine_pool_create(coroutine_pool, coroutine_num);

    return (coroutine_pool);
}

UINT32 coroutine_pool_init(COROUTINE_POOL *coroutine_pool)
{
    clist_init(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), MM_IGNORE, LOC_COROUTINE_0028);
    clist_init(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), MM_IGNORE, LOC_COROUTINE_0029);
    cmutex_init(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool)  , CMUTEX_PROCESS_PRIVATE, LOC_COROUTINE_0030);

    COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool) = 0;
    return (0);
}

/*create coroutine_num coroutine_nodes and add them to coroutine_pool*/
UINT32 coroutine_pool_create(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num)
{
    UINT32 succ_coroutine_num;

    succ_coroutine_num = coroutine_pool_expand(coroutine_pool, DMIN(coroutine_num, COROUTINE_EXPAND_MIN_NUM));
    COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool) += coroutine_num;

    return (succ_coroutine_num);
}

UINT32 coroutine_pool_expand(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num)
{
    UINT32        coroutine_idx;

    for(coroutine_idx = 0; coroutine_idx < coroutine_num; coroutine_idx ++)
    {
        COROUTINE_NODE *coroutine_node;

        coroutine_node = coroutine_node_new(COROUTINE_POOL_MASTER_OWNER(coroutine_pool));
        if(NULL_PTR == coroutine_node)
        {
            sys_log(LOGSTDOUT, "error:coroutine_pool_expand: failed to new # %ld COROUTINE_NODE\n", coroutine_idx);
            break;
        }

        COROUTINE_NODE_STATUS(coroutine_node)  = ((COROUTINE_NODE_STATUS(coroutine_node) & COROUTINE_HI_MASK) | COROUTINE_IS_IDLE);
        COROUTINE_NODE_MOUNTED(coroutine_node) = clist_push_back_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), (void *)coroutine_node);
    }

    return (coroutine_idx);
}

UINT32 coroutine_pool_shrink(COROUTINE_POOL *coroutine_pool, const UINT32 coroutine_num_to_shrink)
{
    COROUTINE_NODE *coroutine_node;
    UINT32 coroutine_num_shrinked;

    for(
         coroutine_num_shrinked = 0;
         coroutine_num_shrinked < coroutine_num_to_shrink && EC_FALSE == clist_is_empty_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
         coroutine_num_shrinked ++, COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool) --
       )
    {
        coroutine_node = (COROUTINE_NODE *)clist_pop_front_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
        COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_DOWN;
    }

    sys_log(LOGSTDOUT, "coroutine_pool_shrink report: to shrink %ld coroutine_nodes, actually shrinked %ld coroutine_nodes, current support max %ld coroutine_nodes\n",
                        coroutine_num_to_shrink, coroutine_num_shrinked, COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool));
    return (coroutine_num_shrinked);
}

UINT32 coroutine_pool_shutdown(COROUTINE_POOL *coroutine_pool)
{
    COROUTINE_NODE *coroutine_node;

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0031);

    while(EC_FALSE == clist_is_empty_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool)))
    {
        coroutine_node = (COROUTINE_NODE *)clist_pop_front_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
        COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_DOWN;
        sys_log(LOGSTDOUT, "coroutine_pool_shutdown: shutdown idle coroutine_node %lx\n", coroutine_node);
    }

    while(EC_FALSE == clist_is_empty(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool)))
    {
        coroutine_node = (COROUTINE_NODE *)clist_pop_front_no_lock(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));

        COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_DOWN;
        sys_log(LOGSTDOUT, "coroutine_pool_shutdown: shutdown busy coroutine_node %lx\n", coroutine_node);
    }

    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0032);
    return (0);
}


UINT32 coroutine_pool_clean(COROUTINE_POOL *coroutine_pool)
{
    coroutine_pool_shutdown(coroutine_pool);
    cmutex_clean(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0033);
    return (0);
}

UINT32 coroutine_pool_free(COROUTINE_POOL *coroutine_pool)
{
    coroutine_pool_clean(coroutine_pool);
    free_static_mem(MD_TASK, 0, MM_COROUTINE_POOL, coroutine_pool, LOC_COROUTINE_0034);
    return (0);
}

COROUTINE_NODE * coroutine_pool_reserve_no_lock(COROUTINE_POOL *coroutine_pool)
{
    COROUTINE_NODE *coroutine_node;
    UINT32        total_num;
    UINT32        idle_num;
    UINT32        busy_num;

    coroutine_node = (COROUTINE_NODE *)clist_pop_back_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    if(NULL_PTR == coroutine_node)
    {
        idle_num = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
        busy_num = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));

        total_num = idle_num + busy_num;

        if(total_num < COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool))
        {
            UINT32 coroutine_num;

            coroutine_num = DMIN(COROUTINE_EXPAND_MIN_NUM, COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool) - total_num);
            
            sys_log(LOGSTDOUT, "coroutine_pool_reserve_no_lock: try to expand coroutine num from %ld to %ld\n", 
                                total_num, coroutine_num + total_num);
            coroutine_pool_expand(coroutine_pool, coroutine_num);
        }

        coroutine_node = (COROUTINE_NODE *)clist_pop_back_no_lock(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    }

    idle_num = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    busy_num = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));

    total_num = idle_num + busy_num;        
    if(total_num > COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool))
    {
        sys_log(LOGSTDOUT, "coroutine_pool_reserve_no_lock: try to shrink coroutine num from %ld to %ld\n", 
                            total_num, COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool));
    
        coroutine_pool_shrink(coroutine_pool, total_num - COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool));
    }
    return (coroutine_node);
}

COROUTINE_NODE * coroutine_pool_reserve(COROUTINE_POOL *coroutine_pool)
{
    COROUTINE_NODE *coroutine_node;

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0035);
    coroutine_node = coroutine_pool_reserve_no_lock(coroutine_pool);
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0036);

    return (coroutine_node);
}

COROUTINE_NODE * coroutine_pool_load_no_lock(COROUTINE_POOL *coroutine_pool, const UINT32 start_routine_addr, const UINT32 para_num, va_list para_list)
{
    COROUTINE_NODE *coroutine_node;

    coroutine_node = coroutine_pool_reserve_no_lock(coroutine_pool);
    if(NULL_PTR == coroutine_node)
    {
        UINT32 idle_num;
        UINT32 busy_num;
        UINT32 total_num;
        UINT32 max_num;

        coroutine_pool_num_info_no_lock(coroutine_pool, &idle_num, &busy_num, &total_num);
        max_num = COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool);

        sys_log(LOGSTDOUT, "warn:coroutine_pool_load_no_lock: failed to reserve one coroutine_node where idle %ld, busy %ld, total %ld, max %ld\n",
                            idle_num, busy_num, total_num, max_num);
        return (NULL_PTR);
    }

    coroutine_node_make_task(coroutine_node, start_routine_addr, para_num, para_list);
    COROUTINE_NODE_COND_RESERVE(coroutine_node, 1, LOC_COROUTINE_0037);

    COROUTINE_NODE_STATUS(coroutine_node)  = ((COROUTINE_NODE_STATUS(coroutine_node) & COROUTINE_HI_MASK) | COROUTINE_IS_BUSY);
    COROUTINE_NODE_MOUNTED(coroutine_node) = clist_push_back_no_lock(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), (void *)coroutine_node);

    return (coroutine_node);
}

COROUTINE_NODE * coroutine_pool_load(COROUTINE_POOL *coroutine_pool, const UINT32 start_routine_addr, const UINT32 para_num, ...)
{
    COROUTINE_NODE *coroutine_node;
    va_list para_list;

    va_start(para_list, para_num);
    
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0038);    
    coroutine_node = coroutine_pool_load_no_lock(coroutine_pool, start_routine_addr, para_num, para_list);
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0039);
    
    va_end(para_list);

    return (coroutine_node);
}

COROUTINE_NODE *coroutine_pool_get_master(COROUTINE_POOL *coroutine_pool)
{
    return COROUTINE_POOL_MASTER_OWNER(coroutine_pool);
}

COROUTINE_NODE *coroutine_pool_get_slave(COROUTINE_POOL *coroutine_pool)
{
    COROUTINE_NODE *crounte_node;    
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0040);
    crounte_node = (COROUTINE_NODE *)clist_first_data(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0041);
    return (crounte_node);
}

/*endless loop*/
void coroutine_pool_run(COROUTINE_POOL *coroutine_pool)
{
    COROUTINE_NODE *coroutine_node_master;
    COROUTINE_NODE *coroutine_node_slave;

    coroutine_node_master = COROUTINE_POOL_MASTER_OWNER(coroutine_pool);   
    coroutine_node_get_task(coroutine_node_master);
    
    for(;;)
    {   
        coroutine_node_slave = coroutine_pool_get_slave(coroutine_pool);
        if(NULL_PTR == coroutine_node_slave)
        {
            usleep(10);
            continue;
        }

        if(COROUTINE_NODE_IS_RESERVED(coroutine_node_slave, LOC_COROUTINE_0042))
        {
            if(1 == coroutine_pool_busy_num(coroutine_pool))
            {
                usleep(10);
            }
            else
            {
                /*usleep(1);*//*force thread to switch*/
                coroutine_node_busy_to_tail(coroutine_node_slave, coroutine_pool);
            }
            continue;
        }

        //coroutine_pool_print(LOGSTDOUT, coroutine_pool);
        
        //sys_log(LOGSTDOUT, "[DEBUG] coroutine_pool_run: before swap, master %lx, slave %lx status %lx\n", coroutine_node_master, coroutine_node_slave, COROUTINE_NODE_STATUS(coroutine_node_slave));
        COROUTINE_NODE_CLR_WAIT_STATUS(coroutine_node_slave);
        coroutine_node_swap_task(coroutine_node_master, coroutine_node_slave);
        //sys_log(LOGSTDOUT, "[DEBUG] coroutine_pool_run: after  swap, master %lx, slave %lx status %lx\n", coroutine_node_master, coroutine_node_slave, COROUTINE_NODE_STATUS(coroutine_node_slave));

        /*check slave status*/
        if(COROUTINE_NODE_STATUS(coroutine_node_slave) & COROUTINE_IS_CANL)
        {
            COROUTINE_NODE_STATUS(coroutine_node_slave) |= COROUTINE_IS_DOWN;
        }
        
        if(COROUTINE_NODE_STATUS(coroutine_node_slave) & COROUTINE_IS_DOWN)
        {
            COROUTINE_NODE_STATUS(coroutine_node_slave) = COROUTINE_IS_IDLE;
            coroutine_node_busy_to_idle(coroutine_node_slave, coroutine_pool);
        }
        else if(COROUTINE_NODE_STATUS(coroutine_node_slave) & COROUTINE_IS_IDLE)
        {
            coroutine_node_busy_to_idle(coroutine_node_slave, coroutine_pool);
        }
        else if(COROUTINE_NODE_STATUS(coroutine_node_slave) & COROUTINE_IS_WAIT)
        {
            coroutine_node_busy_to_tail(coroutine_node_slave, coroutine_pool);
        }
        else
        {
            COROUTINE_NODE_STATUS(coroutine_node_slave) = COROUTINE_IS_IDLE;
            coroutine_node_busy_to_idle(coroutine_node_slave, coroutine_pool);        
        }
    }
    
    return;
}

UINT32 coroutine_pool_size_no_lock(COROUTINE_POOL *coroutine_pool)
{
    UINT32 size;
    size = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool))
         + clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    return (size);
}

UINT32 coroutine_pool_idle_num_no_lock(COROUTINE_POOL *coroutine_pool)
{
    UINT32 num;
    num = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    return (num);
}

UINT32 coroutine_pool_busy_num_no_lock(COROUTINE_POOL *coroutine_pool)
{
    UINT32 num;
    num = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    return (num);
}

UINT32 coroutine_pool_num_info_no_lock(COROUTINE_POOL *coroutine_pool, UINT32 *idle_num, UINT32 *busy_num, UINT32 *total_num)
{
    (*idle_num) = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    (*busy_num) = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    (*total_num) = (*idle_num) + (*busy_num);

    return (0);
}

UINT32 coroutine_pool_size(COROUTINE_POOL *coroutine_pool)
{
    UINT32 size;

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0043);
    size = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool))
         + clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0044);

    return (size);
}

UINT32 coroutine_pool_idle_num(COROUTINE_POOL *coroutine_pool)
{
    UINT32 num;

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0045);
    num = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0046);

    return (num);
}

UINT32 coroutine_pool_busy_num(COROUTINE_POOL *coroutine_pool)
{
    UINT32 num;

    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0047);
    num = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0048);

    return (num);
}

UINT32 coroutine_pool_num_info(COROUTINE_POOL *coroutine_pool, UINT32 *idle_num, UINT32 *busy_num, UINT32 *total_num)
{
    cmutex_lock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0049);
    (*idle_num) = clist_size(COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool));
    (*busy_num) = clist_size(COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool));
    (*total_num) = (*idle_num) + (*busy_num);
    cmutex_unlock(COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0050);

    return (0);
}

void coroutine_pool_print(LOG *log, COROUTINE_POOL *coroutine_pool)
{
    UINT32 idle_num;
    UINT32 busy_num;
    UINT32 total_num;
    
    cmutex_lock((CMUTEX *)COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0051);

    coroutine_pool_num_info_no_lock((COROUTINE_POOL *)coroutine_pool, &idle_num, &busy_num, &total_num);

    sys_log(log, "coroutine_pool %lx: size %ld, idle %ld, busy %ld\n",
                 coroutine_pool, total_num, idle_num, busy_num                
               );

    //sys_log(log, "idle worker list:\n");
    //clist_print(log, COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool), (CLIST_DATA_DATA_PRINT)coroutine_node_print);

    sys_log(log, "busy worker list:\n");
    clist_print_no_lock(log, COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool), (CLIST_DATA_DATA_PRINT)coroutine_node_print);

    sys_log(log, "master woker:\n");
    coroutine_node_print(log, COROUTINE_POOL_MASTER_OWNER(coroutine_pool));

    cmutex_unlock((CMUTEX *)COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool), LOC_COROUTINE_0052);
    return;
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

