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

#include "type.h"
#include "mm.h"
#include "log.h"

#include "clist.h"

#include "cstring.h"
#include "cstrkv.h"

#include "cmpic.inc"

CSTRKV *cstrkv_new(const char *key, const char *val)
{
    CSTRKV *cstrkv;

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRKV, &cstrkv, LOC_CSTRKV_0001);
    ASSERT(NULL_PTR != cstrkv);

    cstrkv_init(cstrkv, key, val);
    return(cstrkv);
}

EC_BOOL cstrkv_init(CSTRKV *cstrkv, const char *key, const char *val) 
{
    cstring_init(CSTRKV_KEY(cstrkv), (const uint8_t *)key);
    cstring_init(CSTRKV_VAL(cstrkv), (const uint8_t *)val);

    return (EC_TRUE);
}

EC_BOOL cstrkv_clean(CSTRKV *cstrkv) 
{
    cstring_clean(CSTRKV_KEY(cstrkv));
    cstring_clean(CSTRKV_VAL(cstrkv));

    return (EC_TRUE);
} 

EC_BOOL cstrkv_free(CSTRKV *cstrkv) 
{
    if (NULL_PTR != cstrkv) 
    {
        cstrkv_clean(cstrkv);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRKV, cstrkv, LOC_CSTRKV_0002);
    }

    return (EC_TRUE);
}

EC_BOOL cstrkv_set_key_str(CSTRKV *cstrkv, const char *key)
{
    cstring_append_str(CSTRKV_KEY(cstrkv), (const uint8_t *)key);
    return (EC_TRUE);
}

EC_BOOL cstrkv_set_key_bytes(CSTRKV *cstrkv, const uint8_t *key, const uint32_t key_len)
{
    cstring_append_chars(CSTRKV_KEY(cstrkv), key_len, key);
    return (EC_TRUE);
}

EC_BOOL cstrkv_set_val_str(CSTRKV *cstrkv, const char *val)
{
    cstring_append_str(CSTRKV_VAL(cstrkv), (const uint8_t *)val);
    return (EC_TRUE);
}

EC_BOOL cstrkv_set_val_bytes(CSTRKV *cstrkv, const uint8_t *val, const uint32_t val_len)
{
    cstring_append_chars(CSTRKV_VAL(cstrkv), val_len, val);
    return (EC_TRUE);
}

void cstrkv_print(LOG *log, const CSTRKV *cstrkv)
{
    sys_log(log, "[key] %s, [val] %s\n", CSTRKV_KEY_STR(cstrkv), CSTRKV_VAL_STR(cstrkv));
    return;
}

CSTRKV_MGR *cstrkv_mgr_new()
{
    CSTRKV_MGR *cstrkv_mgr;

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRKV_MGR, &cstrkv_mgr, LOC_CSTRKV_0003);
    ASSERT(NULL_PTR != cstrkv_mgr);

    cstrkv_mgr_init(cstrkv_mgr);
    return(cstrkv_mgr);
}

EC_BOOL cstrkv_mgr_init(CSTRKV_MGR *cstrkv_mgr) 
{
    clist_init(CSTRKV_MGR_LIST(cstrkv_mgr), MM_CSTRKV, LOC_CSTRKV_0004);

    return (EC_TRUE);
}

EC_BOOL cstrkv_mgr_clean(CSTRKV_MGR *cstrkv_mgr) 
{
    clist_clean(CSTRKV_MGR_LIST(cstrkv_mgr), (CLIST_DATA_DATA_CLEANER)cstrkv_free);

    return (EC_TRUE);
} 

EC_BOOL cstrkv_mgr_free(CSTRKV_MGR *cstrkv_mgr) 
{
    if (NULL_PTR != cstrkv_mgr) 
    {
        cstrkv_mgr_clean(cstrkv_mgr);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRKV_MGR, cstrkv_mgr, LOC_CSTRKV_0005);
    }

    return (EC_TRUE);
}

EC_BOOL cstrkv_mgr_add_kv(CSTRKV_MGR *cstrkv_mgr, const CSTRKV *cstrkv)
{
    clist_push_back(CSTRKV_MGR_LIST(cstrkv_mgr), (void *)cstrkv);
    return (EC_TRUE);
}

EC_BOOL cstrkv_mgr_add_kv_str(CSTRKV_MGR *cstrkv_mgr, const char *key, const char *val)
{
    CSTRKV *cstrkv;
    
    cstrkv = cstrkv_new(key, val);
    if(NULL_PTR == cstrkv)
    {
        dbg_log(SEC_0008_CSTRKV, 0)(LOGSTDOUT, "error:cstrkv_mgr_add_kv_str: new cstrkv of key %s, val %s failed\n", key, val);
        return (EC_FALSE);
    }

    return cstrkv_mgr_add_kv(cstrkv_mgr, cstrkv);
}

CSTRING *cstrkv_mgr_get_val_cstr(CSTRKV_MGR *cstrkv_mgr, const char *key)
{
    CLIST_DATA *clist_data;
    CSTRING     key_cstr;

    cstring_set_str(&key_cstr, key);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_KEY(cstrkv), &key_cstr))
        {
            return (CSTRKV_VAL(cstrkv));
        }
    }

    return (NULL_PTR);
}

char *cstrkv_mgr_get_val_str(CSTRKV_MGR *cstrkv_mgr, const char *key)
{
    CLIST_DATA *clist_data;
    CSTRING     key_cstr;

    cstring_set_str(&key_cstr, key);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_KEY(cstrkv), &key_cstr))
        {
            return ((char *)CSTRKV_VAL_STR(cstrkv));
        }
    }

    return (NULL_PTR);
}

CSTRING *cstrkv_mgr_fetch_key_cstr(CSTRKV_MGR *cstrkv_mgr, const char *val)
{
    CLIST_DATA *clist_data;
    CSTRING     val_cstr;

    cstring_set_str(&val_cstr, val);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_VAL(cstrkv), &val_cstr))
        {
            return (CSTRKV_KEY(cstrkv));
        }
    }

    return (NULL_PTR);
}

char *cstrkv_mgr_fetch_key_str(CSTRKV_MGR *cstrkv_mgr, const char *val)
{
    CLIST_DATA *clist_data;
    CSTRING     val_cstr;

    cstring_set_str(&val_cstr, val);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_VAL(cstrkv), &val_cstr))
        {
            return (CSTRKV_KEY_STR(cstrkv));
        }
    }

    return (NULL_PTR);
}

EC_BOOL cstrkv_mgr_exist_key(CSTRKV_MGR *cstrkv_mgr, const char *key)
{
    CLIST_DATA *clist_data;
    CSTRING     key_cstr;

    cstring_set_str(&key_cstr, key);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_KEY(cstrkv), &key_cstr))
        {
            return (EC_TRUE);
        }
    }

    return (EC_FALSE);
}


EC_BOOL cstrkv_mgr_exist_val(CSTRKV_MGR *cstrkv_mgr, const char *val)
{
    CLIST_DATA *clist_data;
    CSTRING     val_cstr;

    cstring_set_str(&val_cstr, val);

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        if(EC_TRUE == cstring_is_equal(CSTRKV_VAL(cstrkv), &val_cstr))
        {
            return (EC_TRUE);
        }
    }

    return (EC_FALSE);
}

CSTRKV *cstrkv_mgr_last_kv(const CSTRKV_MGR *cstrkv_mgr)
{
    return (CSTRKV *)clist_back(CSTRKV_MGR_LIST(cstrkv_mgr));
}

CSTRKV *cstrkv_mgr_first_kv(const CSTRKV_MGR *cstrkv_mgr)
{
    return (CSTRKV *)clist_front(CSTRKV_MGR_LIST(cstrkv_mgr));
}

void cstrkv_mgr_print(LOG *log, const CSTRKV_MGR *cstrkv_mgr)
{
    CLIST_DATA *clist_data;

    CLIST_LOOP_NEXT(CSTRKV_MGR_LIST(cstrkv_mgr), clist_data)
    {
        CSTRKV *cstrkv;

        cstrkv = (CSTRKV *)CLIST_DATA_DATA(clist_data);
        cstrkv_print(log, cstrkv);
    }

    return;
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

