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

#ifndef _CEXTSRV_H
#define _CEXTSRV_H

#include "type.h"
#include "mm.h"
#include "log.h"
#include "cmutex.h"
#include "char2int.h"

#include "csocket.h"
#include "task.inc"
#include "cextsrv.inc"

EC_BOOL cextclnt_init(CEXTCLNT *cextclnt);

EC_BOOL cextclnt_clean(CEXTCLNT *cextclnt);

EC_BOOL cextclnt_recv(CEXTCLNT *cextclnt, UINT8 **in_buff, UINT32 *in_buff_size);

EC_BOOL cextclnt_send(CEXTCLNT *cextclnt, const UINT8 *out_buff, const UINT32 out_buff_size);

CEXTSRV *cextsrv_new(const UINT32 srv_port, const int srv_sockfd);

EC_BOOL cextsrv_init(CEXTSRV *cextsrv, const UINT32 srv_port, const int srv_sockfd);

EC_BOOL cextsrv_clean(CEXTSRV *cextsrv);

EC_BOOL cextsrv_free(CEXTSRV *cextsrv);

CEXTSRV * cextsrv_start(const UINT32 srv_port);

EC_BOOL cextsrv_end(CEXTSRV *cextsrv);

EC_BOOL cextsrv_req_encode(CEXTSRV *cextsrv, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len, TASK_FUNC *task_req_func);

EC_BOOL cextsrv_req_encode_size(CEXTSRV *cextsrv, const TASK_FUNC *task_req_func, UINT32 *size);

EC_BOOL cextsrv_req_decode(CEXTSRV *cextsrv, const UINT8 *in_buff, const UINT32 in_buff_len, TASK_FUNC *task_req_func);

EC_BOOL cextsrv_rsp_encode(CEXTSRV *cextsrv, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len, TASK_FUNC *task_rsp_func);

EC_BOOL cextsrv_rsp_encode_size(CEXTSRV *cextsrv, const TASK_FUNC *task_rsp_func, UINT32 *size);

EC_BOOL cextsrv_rsp_decode(CEXTSRV *cextsrv, const UINT8 *in_buff, const UINT32 in_buff_len, TASK_FUNC *task_req_func);

EC_BOOL cextsrv_req_clean(TASK_FUNC *task_req_func);

EC_BOOL cextsrv_rsp_clean(TASK_FUNC *task_rsp_func);

EC_BOOL cextsrv_process(CEXTSRV *cextsrv, CEXTCLNT *cextclnt);

/*when thread accept one client connection, then process it and close the connection when reaching end*/
EC_BOOL cextsrv_thread(CEXTSRV *cextsrv);


#endif/*_CEXTSRV_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
