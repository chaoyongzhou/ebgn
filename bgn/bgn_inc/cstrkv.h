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

#ifndef _CSTRKV_H
#define _CSTRKV_H

#include "type.h"
#include "mm.h"
#include "log.h"

#include "clist.h"

#include "cstring.h"

typedef struct
{
    CSTRING  key;
    CSTRING  val;
}CSTRKV;

#define CSTRKV_KEY(cstrkv)      (&((cstrkv)->key))
#define CSTRKV_VAL(cstrkv)      (&((cstrkv)->val))

#define CSTRKV_KEY_STR(cstrkv)  (cstring_get_str(CSTRKV_KEY(cstrkv)))
#define CSTRKV_VAL_STR(cstrkv)  (cstring_get_str(CSTRKV_VAL(cstrkv)))

typedef struct
{
    CLIST    kv_list;
}CSTRKV_MGR;

#define CSTRKV_MGR_LIST(cstrkv_mgr)         (&((cstrkv_mgr)->kv_list))


CSTRKV *cstrkv_new(const char *key, const char *val);

EC_BOOL cstrkv_init(CSTRKV *cstrkv, const char *key, const char *val); 

EC_BOOL cstrkv_clean(CSTRKV *cstrkv); 

EC_BOOL cstrkv_free(CSTRKV *cstrkv); 

EC_BOOL cstrkv_set_key_str(CSTRKV *cstrkv, const char *key);

EC_BOOL cstrkv_set_key_bytes(CSTRKV *cstrkv, const uint8_t *key, const uint32_t key_len);

EC_BOOL cstrkv_set_val_str(CSTRKV *cstrkv, const char *val);

EC_BOOL cstrkv_set_val_bytes(CSTRKV *cstrkv, const uint8_t *val, const uint32_t val_len);

void cstrkv_print(LOG *log, const CSTRKV *cstrkv);

CSTRKV_MGR *cstrkv_mgr_new();

EC_BOOL cstrkv_mgr_init(CSTRKV_MGR *cstrkv_mgr); 

EC_BOOL cstrkv_mgr_clean(CSTRKV_MGR *cstrkv_mgr); 

EC_BOOL cstrkv_mgr_free(CSTRKV_MGR *cstrkv_mgr); 

EC_BOOL cstrkv_mgr_add_kv(CSTRKV_MGR *cstrkv_mgr, const CSTRKV *cstrkv);

EC_BOOL cstrkv_mgr_add_kv_str(CSTRKV_MGR *cstrkv_mgr, const char *key, const char *val);

CSTRING *cstrkv_mgr_get_val_cstr(CSTRKV_MGR *cstrkv_mgr, const char *key);

char *cstrkv_mgr_get_val_str(CSTRKV_MGR *cstrkv_mgr, const char *key);

CSTRING *cstrkv_mgr_fetch_key_cstr(CSTRKV_MGR *cstrkv_mgr, const char *val);

char *cstrkv_mgr_fetch_key_str(CSTRKV_MGR *cstrkv_mgr, const char *val);

EC_BOOL cstrkv_mgr_exist_key(CSTRKV_MGR *cstrkv_mgr, const char *key);

EC_BOOL cstrkv_mgr_exist_val(CSTRKV_MGR *cstrkv_mgr, const char *val);

CSTRKV *cstrkv_mgr_last_kv(const CSTRKV_MGR *cstrkv_mgr);

CSTRKV *cstrkv_mgr_first_kv(const CSTRKV_MGR *cstrkv_mgr);

void cstrkv_mgr_print(LOG *log, const CSTRKV_MGR *cstrkv_mgr);

#endif/*_CSTRKV_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/


