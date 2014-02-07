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

#ifndef _CEPOLL_INC
#define _CEPOLL_INC

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/epoll.h>
#include <errno.h>

#include "type.h"
#include "csocket.inc"

#define CEPOLL_IN        ((uint32_t)0x00000001)
#define CEPOLL_PRI       ((uint32_t)0x00000002)
#define CEPOLL_OUT       ((uint32_t)0x00000004)
#define CEPOLL_RDNORM    ((uint32_t)0x00000040)
#define CEPOLL_RDBAND    ((uint32_t)0x00000080)
#define CEPOLL_WRNORM    ((uint32_t)0x00000100)
#define CEPOLL_WRBAND    ((uint32_t)0x00000200)
#define CEPOLL_MSG       ((uint32_t)0x00000400)
#define CEPOLL_ERR       ((uint32_t)0x00000008)
#define CEPOLL_HUP       ((uint32_t)0x00000010)

#define CEPOLL_ET        ((uint32_t)0x80000000)
#define CEPOLL_ONESHOT   ((uint32_t)0x40000000)

#define CEPOLL_CTL_ADD  ((int)1)
#define CEPOLL_CTL_DEL  ((int)2)
#define CEPOLL_CTL_MOD  ((int)3)

#define CEPOLL_RD_EVENT (CEPOLL_IN)
#define CEPOLL_WR_EVENT (CEPOLL_OUT)

#if 0
typedef union epoll_data
{
  void *ptr;
  int fd;
  uint32_t u32;
  uint64_t u64;
} epoll_data_t;

struct epoll_event
{
  uint32_t events;      /* Epoll events */
  epoll_data_t data;    /* User data variable */
} __attribute__ ((__packed__));
#endif
typedef epoll_data_t CEPOLL_DATA;
typedef struct epoll_event  CEPOLL_EVENT;

#if 0
typedef union {
    void         *ptr;
    int           fd;
    uint32_t      u32;
    uint64_t      u64;
} CEPOLL_DATA;


typedef struct
{
    uint32_t    events;         /* Epoll events/type */
    CEPOLL_DATA data;
}CEPOLL_EVENT;
#endif
#define CEPOLL_EVENT_TYPE(cepoll_event)      ((cepoll_event)->events)
#define CEPOLL_EVENT_FD(cepoll_event)        ((cepoll_event)->data.fd)

#define CEPOLL_MAX_FD_NUM  (4096)

#define CEPOLL_DIR_RD      ((UINT32) 0)
#define CEPOLL_DIR_WR      ((UINT32) 1)
#define CEPOLL_DIR_END     ((UINT32) 2)

#define CEPOLL_NODE_IS_UNDEF   ((uint32_t) 0)
#define CEPOLL_NODE_IS_SERVER  ((uint32_t) 1)
#define CEPOLL_NODE_IS_CLIENT  ((uint32_t) 2)

typedef struct
{
    uint32_t        type;
    uint32_t        rsvd;
    CSOCKET_CNODE  *arg[ CEPOLL_DIR_END ];
}CEPOLL_NODE;

#define CEPOLL_NODE_TYPE(cepoll_node)          ((cepoll_node)->type)
#define CEPOLL_NODE_RD_ARG(cepoll_node)        ((cepoll_node)->arg[ CEPOLL_DIR_RD ])
#define CEPOLL_NODE_WR_ARG(cepoll_node)        ((cepoll_node)->arg[ CEPOLL_DIR_WR ])

typedef struct
{
    int               epoll_fd;
    int               epoll_event_num;/*max epoll event num*/
    CEPOLL_EVENT     *epoll_event_tab;/*used by epoll*/
    CEPOLL_NODE      *epoll_node_tab;
}CEPOLL;

#define CEPOLL_FD(cepoll)                   ((cepoll)->epoll_fd)
#define CEPOLL_EVENT_NUM(cepoll)            ((cepoll)->epoll_event_num)
#define CEPOLL_EVENT_TAB(cepoll)            ((cepoll)->epoll_event_tab)
#define CEPOLL_NODE_TAB(cepoll)             ((cepoll)->epoll_node_tab)

#define CEPOLL_FETCH_EVENT(cepoll, pos)     ((cepoll)->epoll_event_tab + (pos))
#define CEPOLL_FETCH_NODE(cepoll, fd)       ((cepoll)->epoll_node_tab + (fd))


#endif/*_CEPOLL_INC*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/
