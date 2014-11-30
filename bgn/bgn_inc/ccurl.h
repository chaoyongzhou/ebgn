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

#ifndef _CCURL_H
#define _CCURL_H

#include "type.h"
#include "log.h"
#include "cstring.h"
#include "cmutex.h"

#define CCURL_BUFF_INIT_SIZE   ((UINT32) 64 * 1024) /*64KB*/

typedef struct
{
    /* used counter >= 0 */
    UINT32      usedcounter;

    MOD_MGR    *curl_mod_mgr;
}CCURL_MD;

#define CCURL_MD_MOD_MGR(ccurl_md)         ((ccurl_md)->curl_mod_mgr)

#define CCURL_HTTP_STATUS_NONE                                  ((long)   0)
#define CCURL_HTTP_CONTINUE                                     ((long) 100)
#define CCURL_HTTP_SWITCHING_PROTOCOLS                          ((long) 101)
#define CCURL_HTTP_PROCESSING                                   ((long) 102)  /* RFC2518 section 10.1 */
#define CCURL_HTTP_OK                                           ((long) 200)
#define CCURL_HTTP_CREATED                                      ((long) 201)
#define CCURL_HTTP_ACCEPTED                                     ((long) 202)
#define CCURL_HTTP_NON_AUTHORITATIVE_INFORMATION                ((long) 203)
#define CCURL_HTTP_NO_CONTENT                                   ((long) 204)
#define CCURL_HTTP_RESET_CONTENT                                ((long) 205)
#define CCURL_HTTP_PARTIAL_CONTENT                              ((long) 206)
#define CCURL_HTTP_MULTI_STATUS                                 ((long) 207)  /* RFC2518 section 10.2 */
#define CCURL_HTTP_MULTIPLE_CHOICES                             ((long) 300)
#define CCURL_HTTP_MOVED_PERMANENTLY                            ((long) 301)
#define CCURL_HTTP_MOVED_TEMPORARILY                            ((long) 302)
#define CCURL_HTTP_SEE_OTHER                                    ((long) 303)
#define CCURL_HTTP_NOT_MODIFIED                                 ((long) 304)
#define CCURL_HTTP_USE_PROXY                                    ((long) 305)
#define CCURL_HTTP_TEMPORARY_REDIRECT                           ((long) 307)
#define CCURL_HTTP_BAD_REQUEST                                  ((long) 400)
#define CCURL_HTTP_UNAUTHORIZED                                 ((long) 401)
#define CCURL_HTTP_PAYMENT_REQUIRED                             ((long) 402)
#define CCURL_HTTP_FORBIDDEN                                    ((long) 403)
#define CCURL_HTTP_NOT_FOUND                                    ((long) 404)
#define CCURL_HTTP_METHOD_NOT_ALLOWED                           ((long) 405)
#define CCURL_HTTP_NOT_ACCEPTABLE                               ((long) 406)
#define CCURL_HTTP_PROXY_AUTHENTICATION_REQUIRED                ((long) 407)
#define CCURL_HTTP_REQUEST_TIMEOUT                              ((long) 408)
#define CCURL_HTTP_CONFLICT                                     ((long) 409)
#define CCURL_HTTP_GONE                                         ((long) 410)
#define CCURL_HTTP_LENGTH_REQUIRED                              ((long) 411)
#define CCURL_HTTP_PRECONDITION_FAILED                          ((long) 412)
#define CCURL_HTTP_REQUEST_ENTITY_TOO_LARGE                     ((long) 413)
#define CCURL_HTTP_REQUEST_URI_TOO_LONG                         ((long) 414)
#define CCURL_HTTP_UNSUPPORTED_MEDIA_TYPE                       ((long) 415)
#define CCURL_HTTP_RANGE_NOT_SATISFIABLE                        ((long) 416)
#define CCURL_HTTP_EXPECTATION_FAILED                           ((long) 417)
#define CCURL_HTTP_UNPROCESSABLE_ENTITY                         ((long) 422) /* RFC2518 section 10.3 */
#define CCURL_HTTP_LOCKED                                       ((long) 423) /* RFC2518 section 10.4 */
#define CCURL_HTTP_FAILED_DEPENDENCY                            ((long) 424) /* RFC2518 section 10.5 */
#define CCURL_HTTP_INTERNAL_SERVER_ERROR                        ((long) 500)
#define CCURL_HTTP_NOT_IMPLEMENTED                              ((long) 501)
#define CCURL_HTTP_BAD_GATEWAY                                  ((long) 502)
#define CCURL_HTTP_SERVICE_UNAVAILABLE                          ((long) 503)
#define CCURL_HTTP_GATEWAY_TIMEOUT                              ((long) 504)
#define CCURL_HTTP_CCURL_HTTP_VERSION_NOT_SUPPORTED             ((long) 505)
#define CCURL_HTTP_INSUFFICIENT_STORAGE                         ((long) 507)/* RFC2518 section 10.6 */
#define CCURL_HTTP_HEADER_TOO_LARGE                             ((long) 601)/* Header too large to process */


/**
*   for test only
*
*   to query the status of CCURL Module
*
**/
void ccurl_print_module_status(const UINT32 ccurl_md_id, LOG *log);

/**
*
*   free all static memory occupied by the appointed CCURL module
*
*
**/
UINT32 ccurl_free_module_static_mem(const UINT32 ccurl_md_id);

/**
*
* start CCURL module
*
**/
UINT32 ccurl_start();

/**
*
* end CCURL module
*
**/
void ccurl_end(const UINT32 ccurl_md_id);

/**
*
* initialize mod mgr of CCURL module
*
**/
UINT32 ccurl_set_mod_mgr(const UINT32 ccurl_md_id, const MOD_MGR * src_mod_mgr);

/**
*
* get body from specific url via specific proxy
*
**/
EC_BOOL ccurl_get(const UINT32 ccurl_md_id, const CSTRING *url_str, const CSTRING *proxy_ip_port, CSTRING *curl_reply_body);


#endif /*_CCURL_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
