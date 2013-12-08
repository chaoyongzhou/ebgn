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

#ifndef _CEXTSRV_INC
#define _CEXTSRV_INC

#include "type.h"
#include "mm.h"
#include "log.h"
#include "croutine.h"
#include "cmisc.h"

#include "csocket.h"
#include "task.inc"

typedef struct
{
    int client_fd;
    int rsvd;

    UINT32 client_ipaddr;
}CEXTCLNT;

#define CEXTCLNT_SOCKFD(cextclnt)       ((cextclnt)->client_fd)
#define CEXTCLNT_IPADDR(cextclnt)       ((cextclnt)->client_ipaddr)
#define CEXTCLNT_IPADDR_STR(cextclnt)   (c_word_to_ipv4(CEXTCLNT_IPADDR(cextclnt)))


typedef struct
{
    UINT32      srv_port;
    int         srv_sockfd;
    int         rsvd;
    UINT32      md_id;

    CROUTINE_MUTEX      cmutex;

}CEXTSRV;

#define CEXTSRV_PORT(cextsrv)                 ((cextsrv)->srv_port)
#define CEXTSRV_SOCKFD(cextsrv)               ((cextsrv)->srv_sockfd)
#define CEXTSRV_MD_ID(cextsrv)                ((cextsrv)->md_id)
#define CEXTSRV_CMUTEX(cextsrv)               (&((cextsrv)->cmutex))

#define CEXTSRV_CMUTEX_INIT(cextsrv, location)          (croutine_mutex_init(CEXTSRV_CMUTEX(cextsrv), CMUTEX_PROCESS_PRIVATE, location))
#define CEXTSRV_CMUTEX_LOCK(cextsrv, location)          (croutine_mutex_lock(CEXTSRV_CMUTEX(cextsrv), location))
#define CEXTSRV_CMUTEX_UNLOCK(cextsrv, location)        (croutine_mutex_unlock(CEXTSRV_CMUTEX(cextsrv), location))
#define CEXTSRV_CMUTEX_CLEAN(cextsrv, location)         (croutine_mutex_clean(CEXTSRV_CMUTEX(cextsrv), location))


#endif/*_CEXTSRV_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
