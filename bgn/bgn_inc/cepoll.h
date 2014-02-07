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

#ifndef _CEPOLL_H
#define _CEPOLL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/epoll.h>
#include <errno.h>

#include "type.h"
#include "csocket.h"
#include "cepoll.inc"

CEPOLL *cepoll_new(const int epoll_max_event_num);

EC_BOOL cepoll_init(CEPOLL *cepoll, const int epoll_max_event_num);

EC_BOOL cepoll_clean(CEPOLL *cepoll);

EC_BOOL cepoll_free(CEPOLL *cepoll);

EC_BOOL cepoll_add(CEPOLL *cepoll, const int sockfd, const uint32_t events);

EC_BOOL cepoll_del(CEPOLL *cepoll, const int sockfd);

EC_BOOL cepoll_mod(CEPOLL *cepoll, const int sockfd, const uint32_t events);

EC_BOOL cepoll_add_client_event(CEPOLL *cepoll, const uint32_t events, CSOCKET_CNODE *csocket_cnode);

EC_BOOL cepoll_add_srv_event(CEPOLL *cepoll, const uint32_t events, TASKS_CFG *tasks_cfg);

EC_BOOL cepoll_mod_client_event(CEPOLL *cepoll, const uint32_t events, CSOCKET_CNODE *csocket_cnode);

EC_BOOL cepoll_mod_srv_event(CEPOLL *cepoll, const uint32_t events, TASKS_CFG *tasks_cfg);

EC_BOOL cepoll_del_event(CEPOLL *cepoll, const int sockfd);

EC_BOOL cepoll_wait(CEPOLL *cepoll, int timeout_ms);



#endif/*_CEPOLL_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
