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

#ifndef _COROUTINE_INC
#define _COROUTINE_INC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ucontext.h>

#include "type.h"
#include "log.h"
#include "clist.h"

#define COROUTINE_MUTEX_IGNORE_FLAG     ((UINT32)0)
#define COROUTINE_RWLOCK_IGNORE_FLAG    ((UINT32)0)

#define COROUTINE_MUTEX_OP_NEW          ((UINT32) 0)
#define COROUTINE_MUTEX_OP_INIT         ((UINT32) 1)
#define COROUTINE_MUTEX_OP_FREE         ((UINT32) 2)
#define COROUTINE_MUTEX_OP_CLEAN        ((UINT32) 3)
#define COROUTINE_MUTEX_OP_LOCK         ((UINT32) 4)
#define COROUTINE_MUTEX_OP_UNLOCK       ((UINT32) 5)
#define COROUTINE_MUTEX_OP_END          ((UINT32) 6)

#define COROUTINE_RWLOCK_OP_NEW         ((UINT32) 0)
#define COROUTINE_RWLOCK_OP_INIT        ((UINT32) 1)
#define COROUTINE_RWLOCK_OP_FREE        ((UINT32) 2)
#define COROUTINE_RWLOCK_OP_CLEAN       ((UINT32) 3)
#define COROUTINE_RWLOCK_OP_RDLOCK      ((UINT32) 4)
#define COROUTINE_RWLOCK_OP_WRLOCK      ((UINT32) 5)
#define COROUTINE_RWLOCK_OP_UNLOCK      ((UINT32) 6)
#define COROUTINE_RWLOCK_OP_END         ((UINT32) 7)

#define COROUTINE_COND_OP_NEW           ((UINT32) 0)
#define COROUTINE_COND_OP_INIT          ((UINT32) 1)
#define COROUTINE_COND_OP_FREE          ((UINT32) 2)
#define COROUTINE_COND_OP_CLEAN         ((UINT32) 3)
#define COROUTINE_COND_OP_WAIT          ((UINT32) 4)
#define COROUTINE_COND_OP_RESERVE       ((UINT32) 5)
#define COROUTINE_COND_OP_RELEASE       ((UINT32) 6)
#define COROUTINE_COND_OP_END           ((UINT32) 7)

#define COROUTINE_STACK_SIZE_DEFAULT    CROUTINE_STACK_MAX_SIZE

#define COROUTINE_EXPAND_MIN_NUM        ((UINT32)16)
#define COROUTINE_SHRINK_THRESHOLD      ((UINT32)32)
#define COROUTINE_SHRINK_MIN_NUM        ((UINT32) 4)

#define COROUTINE_MAX_ARG_NUM           (16)

#define COROUTINE_ERR_FLAG              ((UINT32) -1)
#define ERR_COROUTINE_ID                ((UINT32) -1)

/*bit map*/
#define COROUTINE_IS_IDLE               ((UINT32) 0x0001)
#define COROUTINE_IS_BUSY               ((UINT32) 0x0010)
#define COROUTINE_IS_WAIT               ((UINT32) 0x0100)
#define COROUTINE_IS_CANL               ((UINT32) 0x0200)/*cancel*/
#define COROUTINE_IS_DOWN               ((UINT32) 0x1000)
#define COROUTINE_LO_MASK               ((UINT32) 0x00FF)
#define COROUTINE_HI_MASK               ((UINT32) 0xFF00)
#define COROUTINE_ALL_MASK              ((UINT32) 0xFFFF)

#define COROUTINE_LOCATION_INIT(location) do{ (location) = LOC_NONE_BASE; }while(0)

typedef struct
{
    UINT32        counter;
    UINT32        location[COROUTINE_MUTEX_OP_END];
}COROUTINE_MUTEX;

#define COROUTINE_MUTEX_COUNTER(coroutine_mutex)   ((coroutine_mutex)->counter)

#define COROUTINE_MUTEX_LOCATION(coroutine_mutex, __op__)  ((coroutine_mutex)->location[__op__])

#define COROUTINE_MUTEX_GET_LOCATION(coroutine_mutex, __op__)  \
        COROUTINE_MUTEX_LOCATION(coroutine_mutex, __op__)
        
#define COROUTINE_MUTEX_SET_LOCATION(coroutine_mutex, __op__, __location__)  do{\
        COROUTINE_MUTEX_LOCATION(coroutine_mutex, __op__) = (__location__);\
}while(0)

#define COROUTINE_MUTEX_INIT_LOCATION(coroutine_mutex) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_MUTEX_OP_END; __op__ ++)\
    {\
        COROUTINE_LOCATION_INIT(COROUTINE_MUTEX_LOCATION((coroutine_mutex), __op__));\
    }\
}while(0)

#define COROUTINE_MUTEX_PRINT_LOCATION(fp, fname, coroutine_mutex) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_MUTEX_OP_END; __op__ ++)\
    {\
        if(LOC_NONE_BASE != COROUTINE_MUTEX_LOCATION((coroutine_mutex), __op__))\
        {\
            sys_log(fp, "\t COROUTINE_MUTEX %lx: %s report: op = %ld happen at %s:%ld\n", \
                        coroutine_mutex, fname, __op__, \
                        MM_LOC_FILE_NAME(COROUTINE_MUTEX_LOCATION((coroutine_mutex), __op__)), \
                        MM_LOC_LINE_NO(COROUTINE_MUTEX_LOCATION((coroutine_mutex), __op__)));\
        }\
    }\
}while(0)

typedef struct
{
    UINT32        readers;
    UINT32        writers;
    UINT32        location[COROUTINE_RWLOCK_OP_END];
}COROUTINE_RWLOCK;

#define COROUTINE_RWLOCK_READERS(coroutine_rwlock)   ((coroutine_rwlock)->readers)
#define COROUTINE_RWLOCK_WRITERS(coroutine_rwlock)   ((coroutine_rwlock)->writers)

#define COROUTINE_RWLOCK_LOCATION(coroutine_rwlock, __op__)  ((coroutine_rwlock)->location[__op__])

#define COROUTINE_RWLOCK_GET_LOCATION(coroutine_rwlock, __op__)  \
        COROUTINE_RWLOCK_LOCATION(coroutine_rwlock, __op__)
        
#define COROUTINE_RWLOCK_SET_LOCATION(coroutine_rwlock, __op__, __location__)  do{\
        COROUTINE_RWLOCK_LOCATION(coroutine_rwlock, __op__) = (__location__);\
}while(0)

#define COROUTINE_RWLOCK_INIT_LOCATION(coroutine_rwlock) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_RWLOCK_OP_END; __op__ ++)\
    {\
        COROUTINE_LOCATION_INIT(COROUTINE_RWLOCK_LOCATION((coroutine_rwlock), __op__));\
    }\
}while(0)

#define COROUTINE_RWLOCK_PRINT_LOCATION(fp, fname, coroutine_rwlock) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_RWLOCK_OP_END; __op__ ++)\
    {\
        if(LOC_NONE_BASE != COROUTINE_RWLOCK_LOCATION((coroutine_rwlock), __op__))\
        {\
            sys_log(fp, "\t COROUTINE_RWLOCK %lx: %s report: op = %ld happen at %s:%ld\n", \
                        coroutine_rwlock, fname, __op__, \
                        MM_LOC_FILE_NAME(COROUTINE_RWLOCK_LOCATION((coroutine_rwlock), __op__)), \
                        MM_LOC_LINE_NO(COROUTINE_RWLOCK_LOCATION((coroutine_rwlock), __op__)));\
        }\
    }\
}while(0)

typedef UINT32 COROUTINE_ID;

typedef struct
{
    UINT32        counter;
    UINT32        location[COROUTINE_COND_OP_END];
}COROUTINE_COND;

#define COROUTINE_COND_COUNTER(coroutine_cond)   ((coroutine_cond)->counter)

#define COROUTINE_COND_LOCATION(coroutine_cond, __op__)  ((coroutine_cond)->location[__op__])

#define COROUTINE_COND_GET_LOCATION(coroutine_cond, __op__)  \
        COROUTINE_COND_LOCATION(coroutine_cond, __op__)
        
#define COROUTINE_COND_SET_LOCATION(coroutine_cond, __op__, __location__)  do{\
        COROUTINE_COND_LOCATION(coroutine_cond, __op__) = (__location__);\
}while(0)

#define COROUTINE_COND_INIT_LOCATION(coroutine_cond) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_COND_OP_END; __op__ ++)\
    {\
        COROUTINE_LOCATION_INIT(COROUTINE_COND_LOCATION((coroutine_cond), __op__));\
    }\
}while(0)

#define COROUTINE_COND_PRINT_LOCATION(fp, fname, coroutine_cond) do{\
    UINT32 __op__;\
    for(__op__ = 0; __op__ < COROUTINE_COND_OP_END; __op__ ++)\
    {\
        if(LOC_NONE_BASE != COROUTINE_COND_LOCATION((coroutine_cond), __op__))\
        {\
            sys_log(fp, "\t COROUTINE_COND %lx: %s report: op = %ld happen at %s:%ld\n", \
                        coroutine_cond, fname, __op__, \
                        MM_LOC_FILE_NAME(COROUTINE_COND_LOCATION((coroutine_cond), __op__)), \
                        MM_LOC_LINE_NO(COROUTINE_COND_LOCATION((coroutine_cond), __op__)));\
        }\
    }\
}while(0)

/*
typedef struct
{   
    UINT32  start_routine_addr;
    UINT32  arg_num;
    UINT32  arg_list[CTHREAD_MAX_ARG_NUM];
}COROUTINE_TASK;

#define COROUTINE_TASK_ROUTINE(coroutine_task)              ((coroutine_task)->start_routine_addr)
#define COROUTINE_TASK_ARG_NUM(coroutine_task)              ((coroutine_task)->arg_num)
#define COROUTINE_TASK_ARG_LIST(coroutine_task)             ((coroutine_task)->arg_list)
#define COROUTINE_TASK_ARG_VAL(coroutine_task, arg_idx)     ((coroutine_task)->arg_list[(arg_idx)])
*/

typedef ucontext_t COROUTINE_TASK;

typedef struct
{
    UINT32         status;
    CLIST_DATA    *routine_mounted;
    COROUTINE_TASK  routine_task;
    COROUTINE_COND  routine_cond;
}COROUTINE_NODE;

#define COROUTINE_NODE_STACK_SPACE(coroutine_node)       ((coroutine_node)->routine_task.uc_stack.ss_sp)
#define COROUTINE_NODE_STACK_SIZE(coroutine_node)        ((coroutine_node)->routine_task.uc_stack.ss_size)
#define COROUTINE_NODE_RESUME_POINT(coroutine_node)      ((coroutine_node)->routine_task.uc_link)
#define COROUTINE_NODE_STATUS(coroutine_node)            ((coroutine_node)->status)
#define COROUTINE_NODE_MOUNTED(coroutine_node)           ((coroutine_node)->routine_mounted)
#define COROUTINE_NODE_TASK(coroutine_node)              (&((coroutine_node)->routine_task))
#define COROUTINE_NODE_COND(coroutine_node)              (&((coroutine_node)->routine_cond))

#define COROUTINE_NODE_COND_INIT(coroutine_node, location)              (coroutine_cond_init(COROUTINE_NODE_COND(coroutine_node), location))
#define COROUTINE_NODE_COND_CLEAN(coroutine_node, location)             (coroutine_cond_clean(COROUTINE_NODE_COND(coroutine_node), location))
#define COROUTINE_NODE_COND_RESERVE(coroutine_node, counter, location)  (coroutine_cond_reserve(COROUTINE_NODE_COND(coroutine_node), counter, location))
#define COROUTINE_NODE_COND_RELEASE(coroutine_node, location)           (coroutine_cond_release(COROUTINE_NODE_COND(coroutine_node), location))
#define COROUTINE_NODE_COND_RELEASE_ALL(coroutine_node, location)       (coroutine_cond_release_all(COROUTINE_NODE_COND(coroutine_node), location))
#define COROUTINE_NODE_COND_WAIT(coroutine_node, location)              (coroutine_cond_wait(COROUTINE_NODE_COND(coroutine_node), location))
#define COROUTINE_NODE_COND_SPY(coroutine_node, location)               (coroutine_cond_spy(COROUTINE_NODE_COND(coroutine_node), location))

#define COROUTINE_NODE_IS_RESERVED(coroutine_node, location)            (1 <= COROUTINE_NODE_COND_SPY(coroutine_node, location))

#define COROUTINE_NODE_SET_WAIT_STATUS(coroutine_node)    (COROUTINE_NODE_STATUS(coroutine_node) |= COROUTINE_IS_WAIT)
#define COROUTINE_NODE_CLR_WAIT_STATUS(coroutine_node)    (COROUTINE_NODE_STATUS(coroutine_node) &= ((~COROUTINE_IS_WAIT) & COROUTINE_ALL_MASK))

typedef struct
{
    CLIST           worker_idle_list;  /*idle coroutine_node list*/
    CLIST           worker_busy_list;  /*busy coroutine_node list*/
    CMUTEX          worker_cmutex;     /*cmutex of cthread_pool*/

    UINT32          worker_max_num;    /*max supported number of coroutine_nodes*/
    COROUTINE_NODE   master_owner;
}COROUTINE_POOL;

#define COROUTINE_POOL_WORKER_IDLE_LIST(coroutine_pool)     (&((coroutine_pool)->worker_idle_list))
#define COROUTINE_POOL_WORKER_BUSY_LIST(coroutine_pool)     (&((coroutine_pool)->worker_busy_list))
#define COROUTINE_POOL_WORKER_CMUTEX(coroutine_pool)        (&((coroutine_pool)->worker_cmutex))
#define COROUTINE_POOL_WORKER_MAX_NUM(coroutine_pool)       ((coroutine_pool)->worker_max_num)
#define COROUTINE_POOL_MASTER_OWNER(coroutine_pool)         (&((coroutine_pool)->master_owner))

typedef struct
{
    COROUTINE_NODE  *coroutine_node;
    COROUTINE_POOL  *coroutine_pool;
}COROUTINE_BIND;

#define COROUTINE_BIND_NODE(coroutine_bind)                 ((coroutine_bind)->coroutine_node)
#define COROUTINE_BIND_POOL(coroutine_bind)                 ((coroutine_bind)->coroutine_pool)

#endif/*_COROUTINE_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

