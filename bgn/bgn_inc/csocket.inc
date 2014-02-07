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

#ifndef _CSOCKET_INC
#define _CSOCKET_INC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "type.h"
#include "cstring.h"
#include "crbuff.h"
#include "cbytes.h"
#include "task.inc"

#include "taskcfg.h"
#include "taskcfg.inc"
#include "mod.inc"

//#define CSOCKET_RECV_ONCE_MAX_SIZE       (32 * 1024) /*32KB*/
//#define CSOCKET_SEND_ONCE_MAX_SIZE       (32 * 1024) /*32KB*/

#define CSOCKET_RECV_ONCE_MAX_SIZE       (64 * 1024) /*64KB*/
#define CSOCKET_SEND_ONCE_MAX_SIZE       (64 * 1024) /*64KB*/

#if (CSOCKET_RECV_ONCE_MAX_SIZE > CSOCKET_SORCVBUFF_SIZE)
#error "fatal error: CSOCKET_RECV_ONCE_MAX_SIZE > CSOCKET_SORCVBUFF_SIZE. pls check settting again!"
#endif

#if (CSOCKET_SEND_ONCE_MAX_SIZE > CSOCKET_SOSNDBUFF_SIZE)
#error "fatal error: CSOCKET_SEND_ONCE_MAX_SIZE > CSOCKET_SORCVBUFF_SIZE. pls check settting again!"
#endif

/*note: CSOCKET_CNODE_XCHG_TASKC_NODE = CSOCKET_CNODE_SENT_TASKC_NODE | CSOCKET_CNODE_RCVD_TASKC_NODE*/
#define CSOCKET_CNODE_NONA_TASKC_NODE       ((UINT32) 0)
#define CSOCKET_CNODE_SENT_TASKC_NODE       ((UINT32) 1)
#define CSOCKET_CNODE_RCVD_TASKC_NODE       ((UINT32) 2)
#define CSOCKET_CNODE_XCHG_TASKC_NODE       ((UINT32) 3)

#define CSOCKET_IS_BLOCK_MODE               ((UINT32) 1)
#define CSOCKET_IS_NONBLOCK_MODE            ((UINT32) 2)

#define CSOCKET_READING_FLAG                ((UINT32) 1)
#define CSOCKET_WRITING_FLAG                ((UINT32) 2)

#define CSOCKET_UDP_PACKET_DATA_SIZE        ((uint16_t)256)/*256 bytes*/
#define CSOCKET_UDP_PACKET_HEADER_SIZE      (sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint16_t))

struct _CDFS_BUFF;
struct _TASK_NODE;
struct _TASK_FUNC;

typedef struct _CSOCKET_CNODE
{    
    UINT32  ipaddr;
    UINT32  srvport;
    UINT32  tcid;
    UINT32  comm;
    UINT32  size;
    int     sockfd;
    int     rsvd;

    /*the status to exchange taskc_node with remote*/
    UINT32  status; /*counter for monitor client up to 2: send local taskc_node info to remote, recv remote taskc_node info from remote*/

    UINT32  load;  /*load of sending direction*/
    
    UINT8   pkt_hdr[32];
    UINT32  pkt_pos;/*when pos reaches csocket_encode_actual_size, header is ready*/

    struct _TASK_NODE *incoming_task_node; /*be incoming data*/
    struct _TASK_NODE *incomed_task_node; /*when no save_to_list recv it, mount here. used by tasks_monitor*/
    CLIST    sending_list;     /*be sending data*/
    CLIST    recving_list;     /*be recving data */
    
    TASKS_NODE *tasks_node; /*shortcut refer to its tasks node which is be appointed when move from monitor to work*/
}CSOCKET_CNODE;

#define CSOCKET_CNODE_IPADDR(csocket_cnode)              ((csocket_cnode)->ipaddr)
#define CSOCKET_CNODE_SRVPORT(csocket_cnode)             ((csocket_cnode)->srvport)
#define CSOCKET_CNODE_TCID(csocket_cnode)                ((csocket_cnode)->tcid)
#define CSOCKET_CNODE_TCID_STR(csocket_cnode)            (c_word_to_ipv4(CSOCKET_CNODE_TCID(csocket_cnode)))
#define CSOCKET_CNODE_COMM(csocket_cnode)                ((csocket_cnode)->comm)
#define CSOCKET_CNODE_SIZE(csocket_cnode)                ((csocket_cnode)->size)
#define CSOCKET_CNODE_SOCKFD(csocket_cnode)              ((csocket_cnode)->sockfd)
#define CSOCKET_CNODE_STATUS(csocket_cnode)              ((csocket_cnode)->status)

#define CSOCKET_CNODE_LOAD(csocket_cnode)                ((csocket_cnode)->load)

#define CSOCKET_CNODE_IPADDR_STR(csocket_cnode)          (c_word_to_ipv4(CSOCKET_CNODE_IPADDR(csocket_cnode)))
#define CSOCKET_CNODE_PKT_HDR(csocket_cnode)             ((csocket_cnode)->pkt_hdr)
#define CSOCKET_CNODE_PKT_HDR_BYTE(csocket_cnode, pos)   ((csocket_cnode)->pkt_hdr[pos])
#define CSOCKET_CNODE_PKT_POS(csocket_cnode)             ((csocket_cnode)->pkt_pos)
#define CSOCKET_CNODE_TASKS_NODE(csocket_cnode)          ((csocket_cnode)->tasks_node)

#define CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode)  ((csocket_cnode)->incoming_task_node)
#define CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode)   ((csocket_cnode)->incomed_task_node)
#define CSOCKET_CNODE_SENDING_LIST(csocket_cnode)        (&((csocket_cnode)->sending_list))
#define CSOCKET_CNODE_RECVING_LIST(csocket_cnode)        (&((csocket_cnode)->recving_list))

#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
#define CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode)    ((csocket_cnode)->rsvd = 1)
#define CSOCKET_CNODE_SET_CONNECTED(csocket_cnode)       ((csocket_cnode)->rsvd = 0)

#define CSOCKET_CNODE_IS_CONNECTED(csocket_cnode)        (0 == (csocket_cnode)->rsvd? EC_TRUE : EC_FALSE)
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/

#if (SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)
#define CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode)    do{}while(0)
#define CSOCKET_CNODE_SET_CONNECTED(csocket_cnode)       do{}while(0)

#define CSOCKET_CNODE_IS_CONNECTED(csocket_cnode)        csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode))
#endif/*(SWITCH_OFF == TASK_BRD_CEPOLL_SWITCH)*/

#endif/*_CSOCKET_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

