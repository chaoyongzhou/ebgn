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
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <errno.h>

#include "type.h"

#include "mm.h"
#include "log.h"

#include "bgnctrl.h"


/**********************************************************************************************************************************************\
test_mutext.c
=============

scenario 1:
==========
usage: <init|clean|lock|unlock|print|quit>
choice> clean
error:mutex_clean - EBUSY: mutex bff0ce60 is locked or in use by another thread
mutex_print: mutex bff0ce60: __m_reserved = 134513750, __m_count = -1074737352, __m_owner = 12343474, __m_kind = 134513336
choice> print
mutex_print: mutex bff0ce60: __m_reserved = 134513750, __m_count = -1074737352, __m_owner = 12343474, __m_kind = 134513336
choice> init
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice> unlock
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice> lock
mutex_print: mutex bff0ce60: __m_reserved = 1, __m_count = 1, __m_owner = 15045, __m_kind = 1
choice> lock
mutex_print: mutex bff0ce60: __m_reserved = 1, __m_count = 2, __m_owner = 15045, __m_kind = 1
choice> lock
mutex_print: mutex bff0ce60: __m_reserved = 1, __m_count = 3, __m_owner = 15045, __m_kind = 1
choice> lock
mutex_print: mutex bff0ce60: __m_reserved = 1, __m_count = 4, __m_owner = 15045, __m_kind = 1
choice> clean
error:mutex_clean - EBUSY: mutex bff0ce60 is locked or in use by another thread
mutex_print: mutex bff0ce60: __m_reserved = 1, __m_count = 4, __m_owner = 15045, __m_kind = 1
choice> unlock
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice> unlock
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice> unlock
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice> clean
mutex_print: mutex bff0ce60: __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1
choice>

scenario 2:
===========
usage: <init|clean|lock|unlock|print|quit>
choice> lock
==> hung on without info

result notes:
============
1. mutex MUST be initialized and then be used
    1.1 if lock some un-initialized mutex, fatal error will happen and the program is hang on
    1.2 after initialize, mutex is set as
            __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1

2. mutex can be locked many times (recursive attribute) as well as __m_count increase and __m_reserved keep 1
            __m_reserved = 1, __m_count = 4, __m_owner = 15045, __m_kind = 1

3. mutex unlock will clean up __m_reserved, __m_count and __m_owner, i.e., restore to the state of initialization
    before unlock:
            __m_reserved = 1, __m_count = 4, __m_owner = 15045, __m_kind = 1
    after unlock:
            __m_reserved = 0, __m_count = 0, __m_owner = 0, __m_kind = 1

4. mutex can be unlocked many times after it is initialized

5. mutex is able to clean only if it is not locked


summary:
=======
1. after initialize, __m_kind =1
2. when  lock, must __m_kind = 1
3. after lock, __m_reserved = 1
4. when  unlock, must __m_kind = 1
5. after unlock, __m_reserved = 0, __m_count = 0
6. when clean, must __m_reserved = 0
\**********************************************************************************************************************************************/

static CMUTEX_POOL g_cmutex_pool;

EC_BOOL cmutex_log_switch = EC_FALSE;
static EC_BOOL cmutex_check(const CMUTEX *cmutex, const UINT32 op, const UINT32 location)
{
    switch(op)
    {
        case CMUTEX_OP_NEW:
            break;
        case CMUTEX_OP_INIT:
            if(0 != CMUTEX_COUNT(cmutex))
            {
                sys_log(LOGSTDOUT, "error: cmutex %lx : op = %ld, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d, at %s:%ld\n",
                                    cmutex, op,
                                    CMUTEX_RESERVED(cmutex),
                                    CMUTEX_COUNT(cmutex),
                                    CMUTEX_OWNER(cmutex),
                                    CMUTEX_KIND(cmutex),
                                    MM_LOC_FILE_NAME(location),
                                    MM_LOC_LINE_NO(location)
                        );
                CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_check", cmutex);
                return (EC_FALSE);
            }
            break;
        case CMUTEX_OP_FREE:
            if(0 != CMUTEX_COUNT(cmutex))
            {
                sys_log(LOGSTDOUT, "error: cmutex %lx : op = %ld, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d, at %s:%ld\n",
                                    cmutex, op,
                                    CMUTEX_RESERVED(cmutex),
                                    CMUTEX_COUNT(cmutex),
                                    CMUTEX_OWNER(cmutex),
                                    CMUTEX_KIND(cmutex),
                                    MM_LOC_FILE_NAME(location),
                                    MM_LOC_LINE_NO(location)
                        );
                CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_check", cmutex);
                return (EC_FALSE);
            }
            break;
        case CMUTEX_OP_CLEAN:
            if(0 != CMUTEX_COUNT(cmutex))
            {
                sys_log(LOGSTDOUT, "error: cmutex %lx : op = %ld, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d, at %s:%ld\n",
                                    cmutex, op,
                                    CMUTEX_RESERVED(cmutex),
                                    CMUTEX_COUNT(cmutex),
                                    CMUTEX_OWNER(cmutex),
                                    CMUTEX_KIND(cmutex),
                                    MM_LOC_FILE_NAME(location),
                                    MM_LOC_LINE_NO(location)
                        );
                CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_check", cmutex);
                return (EC_FALSE);
            }
            break;
        case CMUTEX_OP_LOCK:
            if(0 != CMUTEX_COUNT(cmutex))
            {
                sys_log(LOGSTDOUT, "error: cmutex %lx : op = %ld, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d, at %s:%ld\n",
                                    cmutex, op,
                                    CMUTEX_RESERVED(cmutex),
                                    CMUTEX_COUNT(cmutex),
                                    CMUTEX_OWNER(cmutex),
                                    CMUTEX_KIND(cmutex),
                                    MM_LOC_FILE_NAME(location),
                                    MM_LOC_LINE_NO(location)
                        );
                CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_check", cmutex);
                return (EC_FALSE);
            }
            break;
        case CMUTEX_OP_UNLOCK:
            if(0 == CMUTEX_COUNT(cmutex))
            {
                sys_log(LOGSTDOUT, "error: cmutex %lx : op = %ld, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d, at %s:%ld\n",
                                    cmutex, op,
                                    CMUTEX_RESERVED(cmutex),
                                    CMUTEX_COUNT(cmutex),
                                    CMUTEX_OWNER(cmutex),
                                    CMUTEX_KIND(cmutex),
                                    MM_LOC_FILE_NAME(location),
                                    MM_LOC_LINE_NO(location)
                        );
                CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_check", cmutex);
                return (EC_FALSE);
            }
            break;
    }

    return (EC_TRUE);
}

EC_BOOL cmutex_attr_set(CMUTEX_ATTR  *mutex_attr, const UINT32 flag, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutexattr_init(mutex_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:cmutex_attr_set - ENOMEM: Insufficient memory to create the mutex attributes object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:cmutex_attr_set - UNKNOWN: Error detected when mutexattr init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (ret_val);
    }

    if(CMUTEX_PROCESS_PRIVATE == flag)
    {
        ret_val = pthread_mutexattr_setpshared(mutex_attr, PTHREAD_PROCESS_PRIVATE);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    sys_log(LOGSTDOUT, "error:cmutex_attr_set - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }

                default:
                {
                    sys_log(LOGSTDOUT, "error:cmutex_attr_set - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }
            }

            return (ret_val);
        }
    }

    if(CMUTEX_PROCESS_SHARED == flag)
    {
        ret_val = pthread_mutexattr_setpshared(mutex_attr, PTHREAD_PROCESS_SHARED);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    sys_log(LOGSTDOUT, "error:cmutex_attr_set - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }

                default:
                {
                    sys_log(LOGSTDOUT, "error:cmutex_attr_set - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }
            }

            return (ret_val);
        }
    }

    /*Initialize the mutex attribute called 'type' to PTHREAD_MUTEX_RECURSIVE_NP,
    so that a thread can recursively lock a mutex if needed. */
    ret_val = pthread_mutexattr_settype(mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:cmutex_attr_set - EINVAL: value specified for argument -type- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:cmutex_attr_set - UNKNOWN: error detected when settype, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (ret_val);
    }

    return (ret_val);

}

CMUTEX *cmutex_new(const UINT32 flag, const UINT32 location)
{
    CMUTEX      *cmutex;

    cmutex = (CMUTEX *)SAFE_MALLOC(sizeof(CMUTEX), LOC_CMUTEX_0001);
    if(NULL_PTR == cmutex)
    {
        sys_log(LOGSTDOUT, "error:cmutex_new: failed to alloc CMUTEX, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == cmutex_init(cmutex, flag, location))
    {
        sys_log(LOGSTDOUT, "error:cmutex_init: failed to init cmutex %lx, called at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        SAFE_FREE(cmutex, LOC_CMUTEX_0002);
        return (NULL_PTR);
    }

    //CMUTEX_INIT_LOCATION(cmutex);
    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_NEW, location);

    CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_NEW, location);

    return (cmutex);
}

EC_BOOL cmutex_init(CMUTEX *cmutex, const UINT32 flag, const UINT32 location)
{
    CMUTEX_ATTR  mutex_attr;
    int ret_val;

    CMUTEX_INIT_LOCATION(cmutex);
    //CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_INIT, location);
    

    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_INIT, location);

    ret_val = cmutex_attr_set(&mutex_attr, flag, location);
    if( 0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:cmutex_init: failed to set mutex attribute\n");
        return (EC_FALSE);
    }

    /* Creating and Initializing the mutex with the above stated mutex attributes */
    ret_val = pthread_mutex_init(CMUTEX_MUTEX(cmutex), &mutex_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EAGAIN:
            {
                sys_log(LOGSTDOUT, "error:cmutex_new - EAGAIN: System resources(other than memory) are unavailable, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:cmutex_new - EPERM: Doesn't have privilige to perform this operation, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:cmutex_new - EINVAL: mutex_attr doesn't refer a valid condition variable attribute object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EFAULT:
            {
                sys_log(LOGSTDOUT, "error:cmutex_new - EFAULT: Mutex or mutex_attr is an invalid pointer, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:cmutex_new - ENOMEM: Insufficient memory exists to initialize the mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:cmutex_new - UNKNOWN: Error detected when mutex init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (EC_FALSE);
    }

    //cmutex_pool_add(cmutex_pool_default_get(), cmutex);

    return (EC_TRUE);
}

void cmutex_free(CMUTEX *cmutex, const UINT32 location)
{
    CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_FREE, location);

    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_FREE, location);
    cmutex_clean(cmutex, LOC_CMUTEX_0003);
    SAFE_FREE(cmutex, LOC_CMUTEX_0004);
}

void cmutex_clean(CMUTEX *cmutex, const UINT32 location)
{
    int ret_val;

    CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_CLEAN, location);

    /*when clean, must __m_reserved = 0*/
    if(0 != CMUTEX_RESERVED(cmutex))
    {
        sys_log(LOGSTDOUT, "error:cmutex_clean: cmutex %lx:invalid reserved value found at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        CMUTEX_PRINT_LOCK_INFO(LOGSTDOUT, CMUTEX_OP_CLEAN, cmutex);
        return;
    }

    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_CLEAN, location);

    ret_val = pthread_mutex_destroy(CMUTEX_MUTEX(cmutex));
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:cmutex_clean - EINVAL: cmutex %lx doesn't refer to an initialized mutex, called at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:cmutex_clean - EBUSY: cmutex %lx is locked or in use by another thread, called at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:cmutex_clean - UNKNOWN: cmutex %lx detect error, error no: %d, called at %s:%ld\n", cmutex, ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
    }

    //cmutex_pool_rmv(cmutex_pool_default_get(), cmutex);

    return;
}

EC_BOOL cmutex_lock(CMUTEX *cmutex, const UINT32 location)
{
    int ret_val;

    CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_LOCK, location);
#if 1
    /*when  lock, must __m_kind = 1*/
    if(PTHREAD_MUTEX_RECURSIVE_NP != CMUTEX_KIND(cmutex))
    {
        sys_log(LOGSTDOUT, "error:cmutex_lock: cmutex %lx: invalid kind value %d found at %s:%ld\n", cmutex, CMUTEX_KIND(cmutex), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        CMUTEX_PRINT_LOCK_INFO(LOGSTDOUT, CMUTEX_OP_LOCK, cmutex);
        CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_lock", cmutex);
        return (EC_FALSE);
    }
#endif
    if(NULL_PTR == cmutex)
    {
        sys_log(LOGSTDOUT, "error:cmutex_lock: refuse to lock null cmutex, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    if(EC_TRUE == cmutex_log_switch)
    {
        sys_log(LOGSTDOUT, "cmutex_lock:lock %lx at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
    }

    if(1 < CMUTEX_COUNT(cmutex))
    {
        sys_log(LOGSTDOUT, "warn:cmutex_lock: lock %lx recursively at %s:%ld, depth = %ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location), CMUTEX_COUNT(cmutex));
        CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_lock", cmutex);
    }

    ret_val = pthread_mutex_lock(CMUTEX_MUTEX(cmutex));
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:cmutex_lock - EINVAL: cmutex NOT an initialized object, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EDEADLK:
            {
                sys_log(LOGSTDOUT, "error:cmutex_lock - EDEADLK: deadlock is detected or current thread already owns the cmutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case ETIMEDOUT:
            {
                sys_log(LOGSTDOUT, "error:cmutex_lock - ETIMEDOUT: failed to lock cmutex before the specified timeout expired , called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:cmutex_lock - EBUSY: failed to lock cmutex due to busy , called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:cmutex_lock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        return (EC_FALSE);
    }

    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_LOCK, location);

    return (EC_TRUE);
}

EC_BOOL cmutex_unlock(CMUTEX *cmutex, const UINT32 location)
{
    int ret_val;

    CMUTEX_CHECK_LOCK_VALIDITY(cmutex, CMUTEX_OP_UNLOCK, location);
#if 1
    /*when  unlock, must __m_kind = 1*/
    if(1 != CMUTEX_KIND(cmutex))
    {
        sys_log(LOGSTDOUT, "error:cmutex_unlock: cmutex %lx: invalid kind value found at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        CMUTEX_PRINT_LOCK_INFO(LOGSTDOUT, CMUTEX_OP_UNLOCK, cmutex);
        CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_unlock", cmutex);
        return (EC_FALSE);
    }
#endif
    if(NULL_PTR == cmutex)
    {
        sys_log(LOGSTDOUT, "error:cmutex_unlock: refuse to unlock null cmutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    if(EC_TRUE == cmutex_log_switch)
    {
        sys_log(LOGSTDOUT, "cmutex_unlock:unlock %lx at %s:%ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
    }

    if(0 == CMUTEX_COUNT(cmutex))
    {
        sys_log(LOGSTDOUT, "error:cmutex_unlock: lock %lx found conflict at %s:%ld, depth = %ld\n", cmutex, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location), CMUTEX_COUNT(cmutex));
        CMUTEX_PRINT_LOCATION(LOGSTDOUT, "cmutex_unlock", cmutex);
    }

    ret_val = pthread_mutex_unlock(CMUTEX_MUTEX(cmutex));
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:cmutex_unlock - EINVAL: cmutex NOT an initialized object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:cmutex_unlock - EPERM: current thread does not hold a lock on cmutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:cmutex_unlock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        return (EC_FALSE);
    }

    //CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_UNLOCK, location);
    CMUTEX_SET_LOCATION(cmutex, CMUTEX_OP_UNLOCK, LOC_NONE_BASE);
    return (EC_TRUE);
}

CCOND *ccond_new(const UINT32 location)
{
    CCOND      *ccond;

    ccond = (CCOND *)SAFE_MALLOC(sizeof(CCOND), LOC_CMUTEX_0005);
    if(NULL_PTR == ccond)
    {
        sys_log(LOGSTDOUT, "error:ccond_new: failed to alloc CCOND, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == ccond_init(ccond, location))
    {
        sys_log(LOGSTDOUT, "error:ccond_init: failed to init ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        SAFE_FREE(ccond, LOC_CMUTEX_0006);
        return (NULL_PTR);
    }

    CCOND_INIT_LOCATION(ccond);
    CCOND_SET_LOCATION(ccond, CCOND_OP_NEW, location);
    return (ccond);
}

EC_BOOL ccond_init(CCOND *ccond, const UINT32 location)
{
    CMUTEX_ATTR mutex_attr;
    int ret_val;

    CCOND_SET_LOCATION(ccond, CCOND_OP_INIT, location);

    ret_val = cmutex_attr_set(&mutex_attr, CMUTEX_PROCESS_PRIVATE, location);
    if( 0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_init: failed to set mutex attribute\n");
        return (EC_FALSE);
    }

    ret_val = pthread_cond_init(CCOND_VAR(ccond), NULL_PTR);
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EINVAL: cmutex NOT an initialized object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EBUSY: failed to lock cmutex due to busy, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        return (EC_FALSE);
    }

    ret_val = pthread_mutex_init(CCOND_MUTEX(ccond), &mutex_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EAGAIN:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EAGAIN: System resources(other than memory) are unavailable, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EPERM: Doesn't have privilige to perform this operation, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EINVAL: mutex_attr doesn't refer a valid condition variable attribute object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EFAULT:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - EFAULT: Mutex or mutex_attr is an invalid pointer, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:ccond_init - ENOMEM: Insufficient memory exists to initialize the mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:ccond_init - UNKNOWN: Error detected when mutex init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (EC_FALSE);
    }

    CCOND_COUNTER(ccond) = 0;

    return (EC_TRUE);
}

void ccond_free(CCOND *ccond, const UINT32 location)
{
    CCOND_SET_LOCATION(ccond, CCOND_OP_FREE, location);
    ccond_clean(ccond, LOC_CMUTEX_0007);
    SAFE_FREE(ccond, LOC_CMUTEX_0008);
}

EC_BOOL ccond_clean(CCOND *ccond, const UINT32 location)
{
    int ret_val;

    CCOND_SET_LOCATION(ccond, CCOND_OP_CLEAN, location);

    ret_val = pthread_mutex_destroy(CCOND_MUTEX(ccond));
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:ccond_clean - EINVAL: ccond %lx mutex doesn't refer to an initialized mutex, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:ccond_clean - EBUSY: ccond %lx mutex is locked or in use by another thread, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:ccond_clean - UNKNOWN: ccond %lx mutex detect error, error no: %d, called at %s:%ld\n", ccond, ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
    }

    ret_val = pthread_cond_destroy(CCOND_VAR(ccond));
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:ccond_clean - EINVAL: ccond %lx var doesn't refer to an initialized cond var, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:ccond_clean - EBUSY: ccond %lx var is locked or in use by another thread, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:ccond_clean - UNKNOWN: ccond %lx var detect error, error no: %d, called at %s:%ld\n", ccond, ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
    }

    CCOND_COUNTER(ccond) = 0;

    return (EC_TRUE);
}

EC_BOOL ccond_wait(CCOND *ccond, const UINT32 location)
{
    int ret_val;
#if 1
    ret_val = pthread_mutex_lock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_wait: failed to lock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }
#endif

    CCOND_SET_LOCATION(ccond, CCOND_OP_WAIT, location);

    /*when reserved*/
    while(0 < CCOND_COUNTER(ccond))
    {
        ret_val = pthread_cond_wait(CCOND_VAR(ccond), CCOND_MUTEX(ccond));
        if(0 != ret_val)
        {
            sys_log(LOGSTDOUT, "error:ccond_wait: something wrong, error no: %d, error info: %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        }
    }
#if 1
    ret_val = pthread_mutex_unlock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_wait: failed to unlock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }
 #endif
    return (EC_TRUE);
}

EC_BOOL ccond_reserve(CCOND *ccond, const UINT32 counter, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_lock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_reserve: failed to lock mutex of ccond %lx with counter %ld, called at %s:%ld\n", ccond, counter, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    CCOND_SET_LOCATION(ccond, CCOND_OP_RESERVE, location);

    //CCOND_COUNTER(ccond) = counter;
    CCOND_COUNTER(ccond) += counter;

    ret_val = pthread_mutex_unlock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_reserve: failed to unlock mutex of ccond %lx with counter %ld, called at %s:%ld\n", ccond, counter, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL ccond_release(CCOND *ccond, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_lock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_release: failed to lock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    CCOND_SET_LOCATION(ccond, CCOND_OP_RELEASE, location);

    -- CCOND_COUNTER(ccond);

    if(0 == CCOND_COUNTER(ccond))
    {
        ret_val = pthread_cond_signal(CCOND_VAR(ccond));
        if(0 != ret_val)
        {
            sys_log(LOGSTDOUT, "error:ccond_release: something wrong, error no: %d, error info: %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        }
    }

    ret_val = pthread_mutex_unlock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_release: failed to unlock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL ccond_release_all(CCOND *ccond, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_lock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_release_all: failed to lock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    CCOND_SET_LOCATION(ccond, CCOND_OP_RELEASE, location);

    -- CCOND_COUNTER(ccond);

    if(0 == CCOND_COUNTER(ccond))
    {
        ret_val = pthread_cond_broadcast(CCOND_VAR(ccond));/*broadcast to all*/
        if(0 != ret_val)
        {
            sys_log(LOGSTDOUT, "error:ccond_release_all: something wrong, error no: %d, error info: %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        }
    }

    ret_val = pthread_mutex_unlock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_release_all: failed to unlock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

/*spy on the current times*/
UINT32 ccond_spy(CCOND *ccond, const UINT32 location)
{
    UINT32 times;
    int ret_val;

    ret_val = pthread_mutex_lock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_spy: failed to lock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (ERR_CCOND_TIMES);
    }

    times = CCOND_COUNTER(ccond);

    ret_val = pthread_mutex_unlock(CCOND_MUTEX(ccond));
    if(0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:ccond_spy: failed to unlock mutex of ccond %lx, called at %s:%ld\n", ccond, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (ERR_CCOND_TIMES);
    }
    return (times);
}

EC_BOOL cmutex_node_init(CMUTEX_NODE *cmutex_node)
{
    CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node) = NULL_PTR;
    CMUTEX_NODE_USED_FLAG(cmutex_node) = CMUTEX_NODE_IS_NOT_USED;
    return (EC_TRUE);
}

EC_BOOL cmutex_node_clean(CMUTEX_NODE *cmutex_node)
{
    CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node) = NULL_PTR;
    CMUTEX_NODE_USED_FLAG(cmutex_node) = CMUTEX_NODE_IS_NOT_USED;
    return (EC_TRUE);
}

void cmutex_node_print(LOG *log, CMUTEX_NODE *cmutex_node)
{
    sys_print(log, "cmutex %lx, owner %ld\n",
                    CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node),
                    CMUTEX_OWNER((CMUTEX *)CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node)));
    return;
}

EC_BOOL cmutex_bucket_init(CMUTEX_BUCKET *cmutex_bucket)
{
    UINT32 cmutex_node_idx;

    SPINLOCK_INIT(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));

    for(cmutex_node_idx = 0; cmutex_node_idx < CMUTEX_NODE_MAX_NUM; cmutex_node_idx ++)
    {
        CMUTEX_NODE *cmutex_node;
        cmutex_node = CMUTEX_BUCKET_NODE(cmutex_bucket, cmutex_node_idx);
        cmutex_node_init(cmutex_node);
    }
    return (EC_TRUE);
}

EC_BOOL cmutex_bucket_clean(CMUTEX_BUCKET *cmutex_bucket)
{
    UINT32 cmutex_node_idx;

    for(cmutex_node_idx = 0; cmutex_node_idx < CMUTEX_NODE_MAX_NUM; cmutex_node_idx ++)
    {
        CMUTEX_NODE *cmutex_node;
        cmutex_node = CMUTEX_BUCKET_NODE(cmutex_bucket, cmutex_node_idx);
        cmutex_node_clean(cmutex_node);
    }

    SPINLOCK_CLEAN(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    return (EC_TRUE);
}

EC_BOOL cmutex_bucket_add(CMUTEX_BUCKET *cmutex_bucket, CMUTEX *cmutex)
{
    UINT32 cmutex_node_idx;

    SPINLOCK_LOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    for(cmutex_node_idx = 0; cmutex_node_idx < CMUTEX_NODE_MAX_NUM; cmutex_node_idx ++)
    {
        CMUTEX_NODE *cmutex_node;
        cmutex_node = CMUTEX_BUCKET_NODE(cmutex_bucket, cmutex_node_idx);

        if(CMUTEX_NODE_IS_NOT_USED == CMUTEX_NODE_USED_FLAG(cmutex_node))
        {
            CMUTEX_NODE_USED_FLAG(cmutex_node) = CMUTEX_NODE_IS_USED;
            CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node)  = cmutex;
            CMUTEX_RECORD_NODE(cmutex) = cmutex_node;

            SPINLOCK_UNLOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
            return (EC_TRUE);
        }
    }
    SPINLOCK_UNLOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    return (EC_FALSE);
}

void cmutex_bucket_print(LOG *log, CMUTEX_BUCKET *cmutex_bucket)
{
    UINT32 cmutex_node_idx;

    SPINLOCK_LOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    for(cmutex_node_idx = 0; cmutex_node_idx < CMUTEX_NODE_MAX_NUM; cmutex_node_idx ++)
    {
        CMUTEX_NODE *cmutex_node;
        cmutex_node = CMUTEX_BUCKET_NODE(cmutex_bucket, cmutex_node_idx);
        if(CMUTEX_NODE_IS_USED == CMUTEX_NODE_USED_FLAG(cmutex_node))
        {
            sys_print(log, "\t %8ld# ");
            cmutex_node_print(log, cmutex_node);
        }
    }
    SPINLOCK_UNLOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    return;
}

EC_BOOL cmutex_pool_init(CMUTEX_POOL *cmutex_pool)
{
    UINT32 cmutex_bucket_idx;

    for(cmutex_bucket_idx = 0; cmutex_bucket_idx < CMUTEX_BUCKET_MAX_NUM; cmutex_bucket_idx ++)
    {
        CMUTEX_BUCKET *cmutex_bucket;
        cmutex_bucket = CMUTEX_POOL_BUCKET_BY_IDX(cmutex_pool, cmutex_bucket_idx);
        cmutex_bucket_init(cmutex_bucket);
    }
    return (EC_TRUE);
}

EC_BOOL cmutex_pool_clean(CMUTEX_POOL *cmutex_pool)
{
    UINT32 cmutex_bucket_idx;

    for(cmutex_bucket_idx = 0; cmutex_bucket_idx < CMUTEX_BUCKET_MAX_NUM; cmutex_bucket_idx ++)
    {
        CMUTEX_BUCKET *cmutex_bucket;
        cmutex_bucket = CMUTEX_POOL_BUCKET_BY_IDX(cmutex_pool, cmutex_bucket_idx);
        cmutex_bucket_clean(cmutex_bucket);
    }
    return (EC_TRUE);
}

EC_BOOL cmutex_pool_add(CMUTEX_POOL *cmutex_pool, CMUTEX *cmutex)
{
    CMUTEX_BUCKET *cmutex_bucket;

    cmutex_bucket = CMUTEX_POOL_BUCKET_BY_OWNER(cmutex_pool, CMUTEX_OWNER(cmutex));
    return cmutex_bucket_add(cmutex_bucket, cmutex);
}

EC_BOOL cmutex_pool_rmv(CMUTEX_POOL *cmutex_pool, CMUTEX *cmutex)
{
    CMUTEX_NODE *cmutex_node;

    cmutex_node = CMUTEX_RECORD_NODE(cmutex);
    if(NULL_PTR != cmutex_node)
    {
        cmutex_node_clean(cmutex_node);
        CMUTEX_RECORD_NODE(cmutex) = NULL_PTR;
    }
    return (EC_TRUE);
}

EC_BOOL cmutex_pool_reset_one(CMUTEX_POOL *cmutex_pool, CMUTEX *cmutex)
{
    cmutex_pool_rmv(cmutex_pool, cmutex);
    CMUTEX_RESET(cmutex, CMUTEX_PROCESS_PRIVATE);/*not change the init location info*/
    return (EC_TRUE);
}

EC_BOOL cmutex_pool_reset_all(CMUTEX_POOL *cmutex_pool, const UINT32 old_owner)
{
    CMUTEX_BUCKET *cmutex_bucket;
    UINT32 cmutex_node_idx;

    cmutex_bucket = CMUTEX_POOL_BUCKET_BY_OWNER(cmutex_pool, old_owner);
    SPINLOCK_LOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));

    for(cmutex_node_idx = 0; cmutex_node_idx < CMUTEX_NODE_MAX_NUM; cmutex_node_idx ++)
    {
        CMUTEX_NODE *cmutex_node;
        cmutex_node = CMUTEX_BUCKET_NODE(cmutex_bucket, cmutex_node_idx);

        if(CMUTEX_NODE_IS_USED == CMUTEX_NODE_USED_FLAG(cmutex_node) && old_owner == CMUTEX_OWNER((CMUTEX *)CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node)))
        {
            CMUTEX *cmutex;
            cmutex = (CMUTEX *)CMUTEX_NODE_RECORDED_CMUTEX(cmutex_node);

            cmutex_node_clean(cmutex_node);
            CMUTEX_RECORD_NODE(cmutex) = NULL_PTR;

            CMUTEX_RESET(cmutex, CMUTEX_PROCESS_PRIVATE); /*not change the init location info*/
        }
    }
    SPINLOCK_UNLOCK(CMUTEX_BUCKET_SPINLOCK(cmutex_bucket));
    return (EC_TRUE);
}

void cmutex_pool_print(LOG *log, CMUTEX_POOL *cmutex_pool)
{
    UINT32 cmutex_bucket_idx;

    for(cmutex_bucket_idx = 0; cmutex_bucket_idx < CMUTEX_BUCKET_MAX_NUM; cmutex_bucket_idx ++)
    {
        CMUTEX_BUCKET *cmutex_bucket;
        cmutex_bucket = CMUTEX_POOL_BUCKET_BY_IDX(cmutex_pool, cmutex_bucket_idx);
        cmutex_bucket_print(log, cmutex_bucket);
    }
    return;
}

CMUTEX_POOL *cmutex_pool_default_get()
{
    return (&g_cmutex_pool);
}

EC_BOOL crwlock_attr_set(CRWLOCK_ATTR  *rwlock_attr, const UINT32 flag, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_rwlockattr_init(rwlock_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_attr_set - ENOMEM: Insufficient memory to create the rwlock attributes object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_attr_set - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:crwlock_attr_set - UNKNOWN: Error detected when rwlockattr init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (ret_val);
    }

    if(CRWLOCK_PROCESS_PRIVATE == flag)
    {
        ret_val = pthread_rwlockattr_setpshared(rwlock_attr, PTHREAD_PROCESS_PRIVATE);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    sys_log(LOGSTDOUT, "error:crwlock_attr_set - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }

                default:
                {
                    sys_log(LOGSTDOUT, "error:crwlock_attr_set - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }
            }

            return (ret_val);
        }
    }

    if(CRWLOCK_PROCESS_SHARED == flag)
    {
        ret_val = pthread_rwlockattr_setpshared(rwlock_attr, PTHREAD_PROCESS_SHARED);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    sys_log(LOGSTDOUT, "error:crwlock_attr_set - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }

                default:
                {
                    sys_log(LOGSTDOUT, "error:crwlock_attr_set - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    break;
                }
            }

            return (ret_val);
        }
    }

    return (ret_val);

}

CRWLOCK *crwlock_new(const UINT32 flag, const UINT32 location)
{
    CRWLOCK      *crwlock;

    crwlock = (CRWLOCK *)SAFE_MALLOC(sizeof(CRWLOCK), LOC_CMUTEX_0009);
    if(NULL_PTR == crwlock)
    {
        sys_log(LOGSTDOUT, "error:crwlock_new: failed to alloc CRWLOCK, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (NULL_PTR);
    }

    if(EC_FALSE == crwlock_init(crwlock, flag, location))
    {
        sys_log(LOGSTDOUT, "error:crwlock_init: failed to init crwlock %lx, called at %s:%ld\n", crwlock, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        SAFE_FREE(crwlock, LOC_CMUTEX_0010);
        return (NULL_PTR);
    }

    CRWLOCK_INIT_LOCATION(crwlock);
    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_NEW, location);

    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_NEW, location);

    return (crwlock);
}

EC_BOOL crwlock_init(CRWLOCK *crwlock, const UINT32 flag, const UINT32 location)
{
    CRWLOCK_ATTR  rwlock_attr;
    int ret_val;

    //CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_INIT, location);

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_INIT, location);

    ret_val = crwlock_attr_set(&rwlock_attr, flag, location);
    if( 0 != ret_val)
    {
        sys_log(LOGSTDOUT, "error:crwlock_init: failed to set rwlock attribute\n");
        return (EC_FALSE);
    }

    /* Creating and Initializing the rwlock with the above stated rwlock attributes */
    ret_val = pthread_rwlock_init(CRWLOCK_RWLOCK(crwlock), &rwlock_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EAGAIN:
            {
                sys_log(LOGSTDOUT, "error:crwlock_new - EAGAIN: System resources(other than memory) are unavailable, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_new - EPERM: Doesn't have privilige to perform this operation, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_new - EINVAL: rwlock_attr doesn't refer a valid condition variable attribute object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:crwlock_new - EBUSY: The implementation has detected an attempt to destroy the object referenced by rwlock while it is locked., called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_new - ENOMEM: Insufficient memory exists to initialize the rwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:crwlock_new - UNKNOWN: Error detected when rwlock init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        return (EC_FALSE);
    }

    //crwlock_pool_add(crwlock_pool_default_get(), crwlock);

    return (EC_TRUE);
}

void crwlock_free(CRWLOCK *crwlock, const UINT32 location)
{
    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_FREE, location);

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_FREE, location);
    crwlock_clean(crwlock, LOC_CMUTEX_0011);
    SAFE_FREE(crwlock, LOC_CMUTEX_0012);
}

void crwlock_clean(CRWLOCK *crwlock, const UINT32 location)
{
    int ret_val;

    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_CLEAN, location);

    /*when clean, must __m_reserved = 0*/
    if(0 != CRWLOCK_NR_READER(crwlock) || 0 != CRWLOCK_NR_READER_QUEUED(crwlock) || 0 != CRWLOCK_NR_WRITER_QUEUED(crwlock))
    {
        sys_log(LOGSTDOUT, "error:crwlock_clean: crwlock %lx:invalid status found at %s:%ld\n", crwlock, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        CRWLOCK_PRINT_LOCK_INFO(LOGSTDOUT, CRWLOCK_OP_CLEAN, crwlock);
        return;
    }

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_CLEAN, location);

    ret_val = pthread_rwlock_destroy(CRWLOCK_RWLOCK(crwlock));
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_clean - EINVAL: crwlock %lx doesn't refer to an initialized rwlock, called at %s:%ld\n", crwlock, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:crwlock_clean - EBUSY: crwlock %lx is locked or in use by another thread, called at %s:%ld\n", crwlock, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EAGAIN:
            {
                sys_log(LOGSTDOUT, "error:crwlock_clean - EAGAIN: System resources(other than memory) are unavailable, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_clean - EPERM: Doesn't have privilige to perform this operation, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case ENOMEM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_clean - ENOMEM: Insufficient memory exists to initialize the rwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                /* Unknown error */
                sys_log(LOGSTDOUT, "error:crwlock_clean - UNKNOWN: crwlock %lx detect error, error no: %d, called at %s:%ld\n", crwlock, ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        CRWLOCK_PRINT_LOCATION(LOGSTDOUT, "crwlock_clean", crwlock);
    }

    //crwlock_pool_rmv(crwlock_pool_default_get(), crwlock);

    return;
}

EC_BOOL crwlock_rdlock(CRWLOCK *crwlock, const UINT32 location)
{
    int ret_val;

    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_RDLOCK, location);

    if(NULL_PTR == crwlock)
    {
        sys_log(LOGSTDOUT, "error:crwlock_rdlock: refuse to lock null crwlock, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }


    ret_val = pthread_rwlock_rdlock(CRWLOCK_RWLOCK(crwlock));
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_rdlock - EINVAL: crwlock NOT an initialized object, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EDEADLK:
            {
                sys_log(LOGSTDOUT, "error:crwlock_rdlock - EDEADLK: deadlock is detected or current thread already owns the crwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EAGAIN:
            {
                sys_log(LOGSTDOUT, "error:crwlock_rdlock - EAGAIN: The read lock could not be acquired because the maximum number of read locks for rwlock has been exceeded, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:crwlock_rdlock - EBUSY: failed to lock crwlock due to busy , called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:crwlock_rdlock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        CRWLOCK_PRINT_LOCATION(LOGSTDOUT, "crwlock_rdlock", crwlock);
        return (EC_FALSE);
    }

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_RDLOCK, location);

    return (EC_TRUE);
}

EC_BOOL crwlock_wrlock(CRWLOCK *crwlock, const UINT32 location)
{
    int ret_val;

    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_WRLOCK, location);

    if(NULL_PTR == crwlock)
    {
        sys_log(LOGSTDOUT, "error:crwlock_wrlock: refuse to lock null crwlock, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    ret_val = pthread_rwlock_wrlock(CRWLOCK_RWLOCK(crwlock));
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_wrlock - EINVAL: crwlock NOT an initialized object, called at %s:%ld, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EDEADLK:
            {
                sys_log(LOGSTDOUT, "error:crwlock_wrlock - EDEADLK: deadlock is detected or current thread already owns the crwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EBUSY:
            {
                sys_log(LOGSTDOUT, "error:crwlock_wrlock - EBUSY: failed to lock crwlock due to busy , called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:crwlock_wrlock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }

        CRWLOCK_PRINT_LOCATION(LOGSTDOUT, "crwlock_wrlock", crwlock);
        return (EC_FALSE);
    }

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_WRLOCK, location);

    return (EC_TRUE);
}

EC_BOOL crwlock_unlock(CRWLOCK *crwlock, const UINT32 location)
{
    int ret_val;

    CRWLOCK_CHECK_LOCK_VALIDITY(crwlock, CRWLOCK_OP_UNLOCK, location);

    if(NULL_PTR == crwlock)
    {
        sys_log(LOGSTDOUT, "error:crwlock_unlock: refuse to unlock null crwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        return (EC_FALSE);
    }

    ret_val = pthread_rwlock_unlock(CRWLOCK_RWLOCK(crwlock));
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                sys_log(LOGSTDOUT, "error:crwlock_unlock - EINVAL: crwlock NOT an initialized object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            case EPERM:
            {
                sys_log(LOGSTDOUT, "error:crwlock_unlock - EPERM: current thread does not hold a lock on crwlock, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }

            default:
            {
                sys_log(LOGSTDOUT, "error:crwlock_unlock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                break;
            }
        }
        CRWLOCK_PRINT_LOCATION(LOGSTDOUT, "crwlock_unlock", crwlock);
        return (EC_FALSE);
    }

    CRWLOCK_SET_LOCATION(crwlock, CRWLOCK_OP_UNLOCK, location);
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

