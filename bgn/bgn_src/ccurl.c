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

#include <curl/curl.h>
#include <time.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cstring.h"

#include "carray.h"
#include "cvector.h"

#include "cbc.h"

#include "cmisc.h"

#include "task.h"

#include "csocket.h"

#include "cmpie.h"

#include "ccurl.h"
#include "cload.h"

#include "findex.inc"

#define CCURL_MD_CAPACITY()                  (cbc_md_capacity(MD_CCURL))

#define CCURL_MD_GET(ccurl_md_id)     ((CCURL_MD *)cbc_md_get(MD_CCURL, (ccurl_md_id)))

#define CCURL_MD_ID_CHECK_INVALID(ccurl_md_id)  \
    ((CMPI_ANY_MODI != (ccurl_md_id)) && ((NULL_PTR == CCURL_MD_GET(ccurl_md_id)) || (0 == (CCURL_MD_GET(ccurl_md_id)->usedcounter))))

/**
*   for test only
*
*   to query the status of CCURL Module
*
**/
void ccurl_print_module_status(const UINT32 ccurl_md_id, LOG *log)
{
    CCURL_MD *ccurl_md;
    UINT32 this_ccurl_md_id;

    for( this_ccurl_md_id = 0; this_ccurl_md_id < CCURL_MD_CAPACITY(); this_ccurl_md_id ++ )
    {
        ccurl_md = CCURL_MD_GET(this_ccurl_md_id);

        if ( NULL_PTR != ccurl_md && 0 < ccurl_md->usedcounter )
        {
            sys_log(log,"CCURL Module # %u : %u refered\n",
                    this_ccurl_md_id,
                    ccurl_md->usedcounter);
        }
    }

    return ;
}

/**
*
*   free all static memory occupied by the appointed CCURL module
*
*
**/
UINT32 ccurl_free_module_static_mem(const UINT32 ccurl_md_id)
{
    CCURL_MD  *ccurl_md;

#if ( SWITCH_ON == CCURL_DEBUG_SWITCH )
    if ( CCURL_MD_ID_CHECK_INVALID(ccurl_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ccurl_free_module_static_mem: ccurl module #0x%lx not started.\n",
                ccurl_md_id);
        /*note: here do not exit but return only*/
        return ((UINT32)-1);
    }
#endif/*CCURL_DEBUG_SWITCH*/

    ccurl_md = CCURL_MD_GET(ccurl_md_id);

    free_module_static_mem(MD_CCURL, ccurl_md_id);

    return 0;
}

/**
*
* start CCURL module
*
**/
UINT32 ccurl_start()
{
    CCURL_MD *ccurl_md;
    UINT32   ccurl_md_id;

    ccurl_md_id = cbc_md_new(MD_CCURL, sizeof(CCURL_MD));
    if(ERR_MODULE_ID == ccurl_md_id)
    {
        return (ERR_MODULE_ID);
    }

    /* initilize new one CCURL module */
    ccurl_md = (CCURL_MD *)cbc_md_get(MD_CCURL, ccurl_md_id);
    ccurl_md->usedcounter   = 0;

    /* create a new module node */
    init_static_mem();    

    CCURL_MD_MOD_MGR(ccurl_md)  = mod_mgr_new(ccurl_md_id, /*LOAD_BALANCING_LOOP*//*LOAD_BALANCING_MOD*/LOAD_BALANCING_QUE);

    ccurl_md->usedcounter = 1;

    dbg_log(SEC_0030_CCURL, 5)(LOGSTDOUT, "ccurl_start: start CCURL module #%u\n", ccurl_md_id);

    return ( ccurl_md_id );
}

/**
*
* end CCURL module
*
**/
void ccurl_end(const UINT32 ccurl_md_id)
{
    CCURL_MD *ccurl_md;

    ccurl_md = CCURL_MD_GET(ccurl_md_id);
    if(NULL_PTR == ccurl_md)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT,"error:ccurl_end: ccurl_md_id = %u not exist.\n", ccurl_md_id);
        dbg_exit(MD_CCURL, ccurl_md_id);
    }
    /* if the module is occupied by others,then decrease counter only */
    if ( 1 < ccurl_md->usedcounter )
    {
        ccurl_md->usedcounter --;
        return ;
    }

    if ( 0 == ccurl_md->usedcounter )
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT,"error:ccurl_end: ccurl_md_id = %u is not started.\n", ccurl_md_id);
        dbg_exit(MD_CCURL, ccurl_md_id);
    }

    /* if nobody else occupied the module,then free its resource */
    if(NULL_PTR != CCURL_MD_MOD_MGR(ccurl_md))
    {
        mod_mgr_free(CCURL_MD_MOD_MGR(ccurl_md));
        CCURL_MD_MOD_MGR(ccurl_md) = NULL_PTR;
    }

    /* free module : */
    //ccurl_free_module_static_mem(ccurl_md_id);

    ccurl_md->usedcounter = 0;

    dbg_log(SEC_0030_CCURL, 5)(LOGSTDOUT, "ccurl_end: stop CCURL module #%u\n", ccurl_md_id);
    cbc_md_free(MD_CCURL, ccurl_md_id);

    return ;
}

/**
*
* initialize mod mgr of CCURL module
*
**/
UINT32 ccurl_set_mod_mgr(const UINT32 ccurl_md_id, const MOD_MGR * src_mod_mgr)
{
    CCURL_MD *ccurl_md;
    MOD_MGR  *des_mod_mgr;

#if ( SWITCH_ON == CCURL_DEBUG_SWITCH )
    if ( CCURL_MD_ID_CHECK_INVALID(ccurl_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ccurl_set_mod_mgr: ccurl module #0x%lx not started.\n",
                ccurl_md_id);
        ccurl_print_module_status(ccurl_md_id, LOGSTDOUT);
        dbg_exit(MD_CCURL, ccurl_md_id);
    }
#endif/*CCURL_DEBUG_SWITCH*/

    ccurl_md = CCURL_MD_GET(ccurl_md_id);
    des_mod_mgr = CCURL_MD_MOD_MGR(ccurl_md);

    dbg_log(SEC_0030_CCURL, 5)(LOGSTDOUT, "ccurl_set_mod_mgr: md_id %d, input src_mod_mgr %lx\n", ccurl_md_id, src_mod_mgr);
    mod_mgr_print(LOGSTDOUT, src_mod_mgr);

    /*figure out mod_nodes with tcid belong to set of ccurlnp_tcid_vec and ccurlnp_tcid_vec*/
    mod_mgr_limited_clone(ccurl_md_id, src_mod_mgr, des_mod_mgr);

    dbg_log(SEC_0030_CCURL, 5)(LOGSTDOUT, "====================================ccurl_set_mod_mgr: des_mod_mgr %lx beg====================================\n", des_mod_mgr);
    mod_mgr_print(LOGSTDOUT, des_mod_mgr);
    dbg_log(SEC_0030_CCURL, 5)(LOGSTDOUT, "====================================ccurl_set_mod_mgr: des_mod_mgr %lx end====================================\n", des_mod_mgr);

    return (0);
}

static size_t __ccurl_write_data( void *buffer, size_t size, size_t nmemb, void *userp) 
{
    CSTRING *curl_reply_body;
    size_t seg_size;

    curl_reply_body = userp;
    seg_size = size * nmemb;

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDNULL, "[DEBUG] __ccurl_write_data enter where curl_reply_body %p\n", curl_reply_body);

    if(NULL_PTR == curl_reply_body)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:__ccurl_write_data: curl_reply_body is null\n");
        return (0);
    }

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDNULL, "[DEBUG] __ccurl_write_data: [1] body %p: str %p, capacity %ld, len %ld\n",
                        curl_reply_body, curl_reply_body->str, curl_reply_body->capacity, curl_reply_body->len);

    if(EC_TRUE == cstring_is_empty(curl_reply_body))
    {
        cstring_expand_to(curl_reply_body, CCURL_BUFF_INIT_SIZE);
    }

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDNULL, "[DEBUG] __ccurl_write_data: [2] body %p: str %p, capacity %ld, len %ld\n",
                        curl_reply_body, curl_reply_body->str, curl_reply_body->capacity, curl_reply_body->len);
    

    if(EC_FALSE == cstring_append_chars(curl_reply_body, seg_size, buffer))
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:__ccurl_write_data: append %d bytes to curl_reply_body %p failed\n", seg_size, curl_reply_body);
        return (0);
    }

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDNULL, "[DEBUG] __ccurl_write_data: [3] body %p: str %p, capacity %ld, len %ld\n",
                        curl_reply_body, curl_reply_body->str, curl_reply_body->capacity, curl_reply_body->len);
    

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDNULL, "[DEBUG] __ccurl_write_data leave where curl_reply_body %p len = %ld\n", 
                        curl_reply_body, cstring_get_len(curl_reply_body));
    
    /* Return the number of bytes received, indicating to curl that all is okay */
    return (seg_size);
}

EC_BOOL ccurl_get(const UINT32 ccurl_md_id, const CSTRING *url_str, const CSTRING *proxy_ip_port, CSTRING *curl_reply_body)
{
    CURL    *curl;
    CURLcode res;
    long     status;

#if ( SWITCH_ON == CCURL_DEBUG_SWITCH )
    if ( CCURL_MD_ID_CHECK_INVALID(ccurl_md_id) )
    {
        sys_log(LOGSTDOUT,
                "error:ccurl_get: ccurl module #0x%lx not started.\n",
                ccurl_md_id);
        ccurl_print_module_status(ccurl_md_id, LOGSTDOUT);
        dbg_exit(MD_CCURL, ccurl_md_id);
    }
#endif/*CCURL_DEBUG_SWITCH*/    

    curl = curl_easy_init();
    if(NULL_PTR == curl)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:ccurl_get: curl_easy_init failed\n");
        return (EC_FALSE);
    }    

    curl_easy_setopt(curl, CURLOPT_URL, (char *)cstring_get_str(url_str));
    /* example.com is redirected, so we tell libcurl to follow redirection */ 
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_PROXY, (char *)cstring_get_str(proxy_ip_port));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, __ccurl_write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_reply_body);
    curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L); /*total download time in seconds*/
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 30L); /*total connect time in seconds*/

    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    
    /* Check for errors */ 
    if(CURLE_OK != res)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:ccurl_get:curl_easy_perform() failed: %s where url '%s'\n", 
                           curl_easy_strerror(res), (char *)cstring_get_str(url_str));
        curl_easy_cleanup(curl);
        return (EC_FALSE);
    }

    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &status); 
    if(CURLE_OK != res)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:ccurl_get:curl_easy_getinfo() failed: %s where url '%s'\n", 
                            curl_easy_strerror(res), (char *)cstring_get_str(url_str));
        curl_easy_cleanup(curl);
        return (EC_FALSE);
    }
    
    if(CCURL_HTTP_OK != status)
    {
        dbg_log(SEC_0030_CCURL, 0)(LOGSTDOUT, "error:ccurl_get:reply status %d not OK! where url '%s'\n", 
                           status, (char *)cstring_get_str(url_str));
        curl_easy_cleanup(curl);
        return (EC_FALSE);
    }
    
    /* always cleanup */ 
    curl_easy_cleanup(curl);    

    dbg_log(SEC_0030_CCURL, 9)(LOGSTDOUT, "[DEBUG] ccurl_get: GET '%s' from '%s' done\n", 
                        (char *)cstring_get_str(url_str), 
                        (char *)cstring_get_str(proxy_ip_port));
    
    return (EC_TRUE);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/
