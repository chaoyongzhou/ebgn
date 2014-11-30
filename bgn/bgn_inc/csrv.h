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

#ifndef _CSRV_H
#define _CSRV_H

#include "type.h"
#include "mm.h"
#include "log.h"

#include "csocket.h"

struct _CSOCKET_CNODE;
struct _TASK_FUNC;

typedef  EC_BOOL (*CSRV_ADD_CSOCKET_CNODE)(const UINT32, struct _CSOCKET_CNODE *);
typedef  EC_BOOL (*CSRV_DEL_CSOCKET_CNODE)(const UINT32, struct _CSOCKET_CNODE *);
typedef  EC_BOOL (*CSRV_READ_HANDLER)(struct _CSOCKET_CNODE *);
typedef  EC_BOOL (*CSRV_WRITE_HANDLER)(struct _CSOCKET_CNODE *);
typedef  EC_BOOL (*CSRV_TIMEOUT_HANDLER)(struct _CSOCKET_CNODE *);
typedef  EC_BOOL (*CSRV_CLOSE_HANDLER)(struct _CSOCKET_CNODE *);

typedef struct
{
    UINT32      srv_ipaddr;
    UINT32      srv_port;
    int         srv_sockfd;
    uint32_t    timeout;/*in second*/
    UINT32      md_id;

    CSRV_ADD_CSOCKET_CNODE    add_csocket_cnode;
    CSRV_DEL_CSOCKET_CNODE    del_csocket_cnode;
    
    CSRV_READ_HANDLER         rd_handler;
    CSRV_WRITE_HANDLER        wr_handler;
    CSRV_TIMEOUT_HANDLER      timeout_handler;
    CSRV_CLOSE_HANDLER        close_handler;
}CSRV;

#define CSRV_IPADDR(csrv)               ((csrv)->srv_ipaddr)
#define CSRV_PORT(csrv)                 ((csrv)->srv_port)
#define CSRV_SOCKFD(csrv)               ((csrv)->srv_sockfd)
#define CSRV_MD_ID(csrv)                ((csrv)->md_id)
#define CSRV_TIMEOUT_NSEC(csrv)         ((csrv)->timeout)
#define CSRV_ADD_CSOCKET_CNODE(csrv)    ((csrv)->add_csocket_cnode)
#define CSRV_DEL_CSOCKET_CNODE(csrv)    ((csrv)->del_csocket_cnode)
#define CSRV_RD_HANDLER(csrv)           ((csrv)->rd_handler)
#define CSRV_WR_HANDLER(csrv)           ((csrv)->wr_handler)
#define CSRV_OT_HANDLER(csrv)           ((csrv)->timeout_handler)
#define CSRV_CL_HANDLER(csrv)           ((csrv)->close_handler)

CSRV *csrv_new(const UINT32 srv_ipaddr, const UINT32 srv_port, const int srv_sockfd);

EC_BOOL csrv_init(CSRV *csrv, const UINT32 srv_ipaddr, const UINT32 srv_port, const int srv_sockfd);

EC_BOOL csrv_clean(CSRV *csrv);

EC_BOOL csrv_free(CSRV *csrv);

CSRV * csrv_start(const UINT32 srv_ipaddr, const UINT32 srv_port, const UINT32 md_id, 
                     const uint32_t timeout_nsec,
                     CSRV_ADD_CSOCKET_CNODE    add_csocket_cnode, 
                     CSRV_DEL_CSOCKET_CNODE    del_csocket_cnode,
                     CSRV_READ_HANDLER         rd_handler,
                     CSRV_WRITE_HANDLER        wr_handler,
                     CSRV_TIMEOUT_HANDLER      timeout_handler,
                     CSRV_CLOSE_HANDLER        close_handler
                     );


EC_BOOL csrv_end(CSRV *csrv);

EC_BOOL csrv_accept_once(CSRV *csrv, EC_BOOL *continue_flag);

EC_BOOL csrv_accept(CSRV *csrv);



EC_BOOL csrv_select(CSRV *csrv, int *ret);

EC_BOOL csrv_req_clean(struct _TASK_FUNC *task_req_func);

EC_BOOL csrv_rsp_clean(struct _TASK_FUNC *task_rsp_func);

EC_BOOL csrv_process(CSRV *csrv, struct _CSOCKET_CNODE *csocket_cnode);

EC_BOOL csrv_handle(CSRV *csrv, struct _CSOCKET_CNODE *csocket_cnode);

EC_BOOL csrv_do_once(CSRV *csrv);

#endif/*_CSRV_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
