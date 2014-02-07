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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>

#include "type.h"
#include "log.h"

#include "cstring.h"
#include "cset.h"

#include "cxml.h"
#include "task.h"
#include "taskcfg.inc"
#include "taskcfg.h"
#include "csocket.h"
#include "task.inc"

#include "cmpic.inc"
#include "cmpie.h"
#include "cmisc.h"
#include "crbuff.h"
#include "cbitmap.h"
#include "cdfs.h"

#include "db_internal.h"

static UINT32  g_tmp_encode_size = 0;

static UINT32  g_xmod_node_tmp_encode_size = 0;
static UINT8   g_xmod_node_tmp_encode_buff[256];

#if 0
#define PRINT_BUFF(info, buff, len) do{\
    UINT32 __pos__;\
    sys_log(LOGSTDOUT, "%s: ", info);\
    for(__pos__ = 0; __pos__ < len; __pos__ ++)\
    {\
        sys_print(LOGSTDOUT, "%x,", ((UINT8 *)buff)[ __pos__ ]);\
    }\
    sys_print(LOGSTDOUT, "\n");\
}while(0)
#else
#define PRINT_BUFF(info, buff, len) do{}while(0)
#endif


static const char *csocket_tcpi_stat(const int tcpi_stat)
{
    switch(tcpi_stat)
    {
        case TCP_ESTABLISHED: return "TCP_ESTABLISHED";
        case TCP_SYN_SENT   : return "TCP_SYN_SENT";
        case TCP_SYN_RECV   : return "TCP_SYN_RECV";
        case TCP_FIN_WAIT1  : return "TCP_FIN_WAIT1";
        case TCP_FIN_WAIT2  : return "TCP_FIN_WAIT2";
        case TCP_TIME_WAIT  : return "TCP_TIME_WAIT";
        case TCP_CLOSE      : return "TCP_CLOSE";
        case TCP_CLOSE_WAIT : return "TCP_CLOSE_WAIT";
        case TCP_LAST_ACK   : return "TCP_LAST_ACK";
        case TCP_LISTEN     : return "TCP_LISTEN";
        case TCP_CLOSING    : return "TCP_CLOSING";
        default             :
        sys_log(LOGSTDOUT, "csocket_tcpi_stat: unknown tcpi_stat = %d\n", tcpi_stat);
    }
    return "unknown state";
}

void csocket_tcpi_stat_print(LOG *log, const int sockfd)
{
    struct tcp_info info;
    socklen_t info_len;

    info_len = sizeof(struct tcp_info);
    if(0 != getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, (char *)&info, &info_len))
    {
        sys_log(log, "csocket_is_connected: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return;
    }

    sys_log(log, "csocket_tcpi_stat_print: sockfd %d tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
    return;
}

void csocket_cnode_init(CSOCKET_CNODE *csocket_cnode)
{
    CSOCKET_CNODE_TCID(csocket_cnode   ) = CMPI_ERROR_TCID;
    CSOCKET_CNODE_COMM(csocket_cnode   ) = CMPI_ERROR_COMM;
    CSOCKET_CNODE_SIZE(csocket_cnode   ) = 0              ;
    CSOCKET_CNODE_SOCKFD(csocket_cnode ) = CMPI_ERROR_SOCKFD;

    CSOCKET_CNODE_IPADDR(csocket_cnode ) = CMPI_ERROR_IPADDR;
    CSOCKET_CNODE_SRVPORT(csocket_cnode) = CMPI_ERROR_SRVPORT;

    CSOCKET_CNODE_PKT_POS(csocket_cnode) = 0;
    CSOCKET_CNODE_TASKS_NODE(csocket_cnode) = NULL_PTR;

    CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = NULL_PTR;
    CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode)  = NULL_PTR;
    clist_init(CSOCKET_CNODE_SENDING_LIST(csocket_cnode), MM_IGNORE, LOC_CSOCKET_0001);
    clist_init(CSOCKET_CNODE_RECVING_LIST(csocket_cnode), MM_IGNORE, LOC_CSOCKET_0002);
    return;
}

void csocket_cnode_clean(CSOCKET_CNODE *csocket_cnode)
{
    CSOCKET_CNODE_TCID(csocket_cnode   ) = CMPI_ERROR_TCID;
    CSOCKET_CNODE_COMM(csocket_cnode   ) = CMPI_ERROR_COMM;
    CSOCKET_CNODE_SIZE(csocket_cnode   ) = 0              ;
    CSOCKET_CNODE_SOCKFD(csocket_cnode ) = CMPI_ERROR_SOCKFD;

    CSOCKET_CNODE_IPADDR(csocket_cnode ) = CMPI_ERROR_IPADDR;
    CSOCKET_CNODE_SRVPORT(csocket_cnode) = CMPI_ERROR_SRVPORT;

    CSOCKET_CNODE_TASKS_NODE(csocket_cnode) = NULL_PTR;

    CSOCKET_CNODE_PKT_POS(csocket_cnode) = 0;

    if(NULL_PTR != CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode))
    {
        task_node_free(CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode));
        CSOCKET_CNODE_INCOMING_TASK_NODE(csocket_cnode) = NULL_PTR;
    }

    if(NULL_PTR != CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode))
    {
        task_node_free(CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode));
        CSOCKET_CNODE_INCOMED_TASK_NODE(csocket_cnode) = NULL_PTR;
    }

    if(CSOCKET_CNODE_XCHG_TASKC_NODE == CSOCKET_CNODE_STATUS(csocket_cnode))
    {
        /*when csocket_cnode is in working mode, all task_nodes coming from task_mgr, do not clean up them, just umount from list*/
        clist_clean(CSOCKET_CNODE_SENDING_LIST(csocket_cnode) , NULL_PTR);
    }
    else
    {
         /*when csocket_cnode is in monitoring mode, all task_nodes was created by monitor, clean up them*/
        clist_clean(CSOCKET_CNODE_SENDING_LIST(csocket_cnode) , (CLIST_DATA_DATA_CLEANER)task_node_free);
    }
    clist_clean(CSOCKET_CNODE_RECVING_LIST(csocket_cnode) , (CLIST_DATA_DATA_CLEANER)task_node_free);

    return;
}

void csocket_cnode_init_0(const UINT32 md_id, CSOCKET_CNODE *csocket_cnode)
{
    csocket_cnode_init(csocket_cnode);
    return;
}

void csocket_cnode_clean_0(const UINT32 md_id, CSOCKET_CNODE *csocket_cnode)
{
    csocket_cnode_clean(csocket_cnode);
    return;
}

void csocket_cnode_free_0(const UINT32 md_id, CSOCKET_CNODE *csocket_cnode)
{
    //sys_log(LOGSTDOUT, "info:csocket_cnode_free_0: to free csocket_cnode %lx\n", csocket_cnode);
    csocket_cnode_free(csocket_cnode);
    return;
}

CSOCKET_CNODE * csocket_cnode_new_0()
{
    CSOCKET_CNODE *csocket_cnode;
    alloc_static_mem(MD_TBD, 0, MM_CSOCKET_CNODE, &csocket_cnode, LOC_CSOCKET_0003);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csocket_cnode_new_0: failed to alloc CSOCKET_CNODE\n");
        return (NULL_PTR);
    }
    csocket_cnode_init(csocket_cnode);
    return (csocket_cnode);
}

UINT32 csocket_cnode_clone_0(const CSOCKET_CNODE *csocket_cnode_src, CSOCKET_CNODE *csocket_cnode_des)
{
    UINT32 pos;
    
    CSOCKET_CNODE_TCID(csocket_cnode_des  ) = CSOCKET_CNODE_TCID(csocket_cnode_src  );
    CSOCKET_CNODE_COMM(csocket_cnode_des  ) = CSOCKET_CNODE_COMM(csocket_cnode_src  );
    CSOCKET_CNODE_SIZE(csocket_cnode_des  ) = CSOCKET_CNODE_SIZE(csocket_cnode_src  );
    CSOCKET_CNODE_SOCKFD(csocket_cnode_des) = CSOCKET_CNODE_SOCKFD(csocket_cnode_src);
    CSOCKET_CNODE_STATUS(csocket_cnode_des) = CSOCKET_CNODE_STATUS(csocket_cnode_src);

    CSOCKET_CNODE_IPADDR(csocket_cnode_des) = CSOCKET_CNODE_IPADDR(csocket_cnode_src);
    CSOCKET_CNODE_SRVPORT(csocket_cnode_des)= CSOCKET_CNODE_SRVPORT(csocket_cnode_src);

    CSOCKET_CNODE_LOAD(csocket_cnode_des)   = CSOCKET_CNODE_LOAD(csocket_cnode_src);
   
    for(pos = 0; pos < CSOCKET_CNODE_PKT_POS(csocket_cnode_src); pos ++)
    {
        CSOCKET_CNODE_PKT_HDR_BYTE(csocket_cnode_des, pos) = CSOCKET_CNODE_PKT_HDR_BYTE(csocket_cnode_src, pos);
    }
    CSOCKET_CNODE_PKT_POS(csocket_cnode_des)= CSOCKET_CNODE_PKT_POS(csocket_cnode_src);

    return (0);
}

CSOCKET_CNODE * csocket_cnode_new(const UINT32 tcid, const int sockfd, const UINT32 ipaddr, const UINT32 srvport)
{
    CSOCKET_CNODE *csocket_cnode;

    alloc_static_mem(MD_TBD, 0, MM_CSOCKET_CNODE, &csocket_cnode, LOC_CSOCKET_0004);
    if(NULL_PTR == csocket_cnode)
    {
        sys_log(LOGSTDOUT, "error:csocket_cnode_new: failed to alloc CSOCKET_CNODE\n");
        return (NULL_PTR);
    }

    csocket_cnode_init(csocket_cnode);

    CSOCKET_CNODE_TCID(csocket_cnode  ) = tcid           ;
    CSOCKET_CNODE_COMM(csocket_cnode  ) = CMPI_ERROR_COMM;
    CSOCKET_CNODE_SIZE(csocket_cnode  ) = 0              ;
    CSOCKET_CNODE_SOCKFD(csocket_cnode) = sockfd         ;
    CSOCKET_CNODE_STATUS(csocket_cnode) = CSOCKET_CNODE_NONA_TASKC_NODE;

    CSOCKET_CNODE_IPADDR(csocket_cnode) = ipaddr;
    CSOCKET_CNODE_SRVPORT(csocket_cnode)= srvport;

    CSOCKET_CNODE_LOAD(csocket_cnode)    = 0;
    CSOCKET_CNODE_PKT_POS(csocket_cnode) = 0;

    CSOCKET_CNODE_SET_CONNECTED(csocket_cnode);

    return (csocket_cnode);
}

void csocket_cnode_free(CSOCKET_CNODE *csocket_cnode)
{
    if(NULL_PTR != csocket_cnode)
    {
        csocket_cnode_clean(csocket_cnode);
        free_static_mem(MD_TBD, 0, MM_CSOCKET_CNODE, csocket_cnode, LOC_CSOCKET_0005);
    }
    return;
}

void csocket_cnode_close(CSOCKET_CNODE *csocket_cnode)
{
    if(NULL_PTR != csocket_cnode)
    {
#if (SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)
        cepoll_del_event(task_brd_default_get_cepoll(), CSOCKET_CNODE_SOCKFD(csocket_cnode));
#endif/*(SWITCH_ON == TASK_BRD_CEPOLL_SWITCH)*/
        csocket_close(CSOCKET_CNODE_SOCKFD(csocket_cnode));
        csocket_cnode_free(csocket_cnode);
    }
    return;
}

EC_BOOL csocket_cnode_cmp(const CSOCKET_CNODE *csocket_cnode_1, const CSOCKET_CNODE *csocket_cnode_2)
{
    if((NULL_PTR == csocket_cnode_1) && (NULL_PTR == csocket_cnode_2))
    {
        return (EC_TRUE);
    }
    if((NULL_PTR == csocket_cnode_1) || (NULL_PTR == csocket_cnode_2))
    {
        return (EC_FALSE);
    }
    if(CSOCKET_CNODE_TCID(csocket_cnode_1) != CSOCKET_CNODE_TCID(csocket_cnode_2))
    {
        return (EC_FALSE);
    }
    if(CSOCKET_CNODE_SOCKFD(csocket_cnode_1) != CSOCKET_CNODE_SOCKFD(csocket_cnode_2))
    {
        return (EC_FALSE);
    }

    /*when comm not set, then skip comparasion*/
    if(CMPI_ERROR_COMM != (CSOCKET_CNODE_COMM(csocket_cnode_1))
    && CMPI_ERROR_COMM != (CSOCKET_CNODE_COMM(csocket_cnode_2))
    && CSOCKET_CNODE_COMM(csocket_cnode_1) != CSOCKET_CNODE_COMM(csocket_cnode_2))
    {
        return (EC_FALSE);
    }

    /*when size not set, then skip comparasion*/
    if(0 != (CSOCKET_CNODE_SIZE(csocket_cnode_1))
    && 0 != (CSOCKET_CNODE_SIZE(csocket_cnode_2))
    && CSOCKET_CNODE_SIZE(csocket_cnode_1) != CSOCKET_CNODE_SIZE(csocket_cnode_2))
    {
        return (EC_FALSE);
    }

    /*when ipaddr not set, then skip comparasion*/
    if(CMPI_ERROR_IPADDR != (CSOCKET_CNODE_IPADDR(csocket_cnode_1))
    && CMPI_ERROR_IPADDR != (CSOCKET_CNODE_IPADDR(csocket_cnode_2))
    && CSOCKET_CNODE_IPADDR(csocket_cnode_1) != CSOCKET_CNODE_IPADDR(csocket_cnode_2))
    {
        return (EC_FALSE);
    }

    /*when srvport not set, then skip comparasion*/
    if(CMPI_ERROR_SRVPORT != (CSOCKET_CNODE_SRVPORT(csocket_cnode_1))
    && CMPI_ERROR_SRVPORT != (CSOCKET_CNODE_SRVPORT(csocket_cnode_2))
    && (CSOCKET_CNODE_SRVPORT(csocket_cnode_1) != CSOCKET_CNODE_SRVPORT(csocket_cnode_2)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}


EC_BOOL csocket_cnode_is_connected(const CSOCKET_CNODE *csocket_cnode)
{
    if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void csocket_cnode_print(LOG *log, const CSOCKET_CNODE *csocket_cnode)
{
    sys_print(log, "csocket_cnode %lx: sockfd = %d, srvipaddr = %s, srvport = %ld, tcid = %s, comm = %ld, size = %ld, crbuff = %lx\n",
                csocket_cnode, CSOCKET_CNODE_SOCKFD(csocket_cnode),
                CSOCKET_CNODE_IPADDR_STR(csocket_cnode),
                CSOCKET_CNODE_SRVPORT(csocket_cnode),
                CSOCKET_CNODE_TCID_STR(csocket_cnode),
                CSOCKET_CNODE_COMM(csocket_cnode),
                CSOCKET_CNODE_SIZE(csocket_cnode)
                );
    return;
}

void sockfd_print(LOG *log, const void *data)
{
    sys_print(log, "%d\n", UINT32_TO_INT32((UINT32)data));
    csocket_tcpi_stat_print(log, UINT32_TO_INT32((UINT32)data));
}

EC_BOOL csocket_fd_clean(FD_CSET *sockfd_set)
{
    FD_ZERO(SOCKFD_SET(sockfd_set));
    return (EC_TRUE);
}

EC_BOOL csocket_fd_set(const int sockfd, FD_CSET *sockfd_set, int *max_sockfd)
{
    //sys_log(LOGSTDOUT, "csocket_fd_set: sockfd %d add to FD_CSET %lx\n", sockfd, sockfd_set);
    FD_SET(sockfd, SOCKFD_SET(sockfd_set));
    if((*max_sockfd) < sockfd)
    {
        (*max_sockfd) = sockfd;
    }

    return (EC_TRUE);
}

EC_BOOL csocket_fd_isset(const int sockfd, FD_CSET *sockfd_set)
{
    if(FD_ISSET(sockfd, SOCKFD_SET(sockfd_set)))
    {
        //sys_log(LOGSTDOUT, "csocket_fd_isset: sockfd %d was set in FD_CSET %lx\n", sockfd, sockfd_set);
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL csocket_fd_clr(const int sockfd, FD_CSET *sockfd_set)
{
    sys_log(LOGSTDOUT, "csocket_fd_clr: sockfd %d was cleared in FD_CSET %lx\n", sockfd, sockfd_set);
    FD_CLR(sockfd, SOCKFD_SET(sockfd_set));
    return (EC_TRUE);
}

EC_BOOL csocket_fd_clone(FD_CSET *src_sockfd_set, FD_CSET *des_sockfd_set)
{
    //sys_log(LOGSTDOUT, "csocket_fd_clone: clone %lx ---> %lx\n", src_sockfd_set, des_sockfd_set);
    (*SOCKFD_SET(des_sockfd_set)) = (*SOCKFD_SET(src_sockfd_set));

    return (EC_TRUE);
}

static EC_BOOL csocket_srv_addr_init( const UINT32 srv_port, struct sockaddr_in *srv_addr)
{
    srv_addr->sin_family      = AF_INET;
    srv_addr->sin_port        = htons( atoi(c_word_to_port(srv_port)) );
    srv_addr->sin_addr.s_addr = INADDR_ANY;
    bzero(srv_addr->sin_zero, sizeof(srv_addr->sin_zero)/sizeof(srv_addr->sin_zero[0]));

    return  ( EC_TRUE );
}

EC_BOOL csocket_client_addr_init( const UINT32 srv_ipaddr, const UINT32 srv_port, struct sockaddr_in *srv_addr)
{
    struct hostent *hp;

    /* get host addr info*/
    hp = gethostbyname( c_word_to_ipv4(srv_ipaddr) );
    if ( NULL_PTR == hp )
    {
        sys_log(LOGSTDERR, "error:csocket_client_addr_init: unknow server: %s\n", c_word_to_ipv4(srv_ipaddr));
        return ( EC_FALSE );
    }

    if ( AF_INET != hp->h_addrtype )
    {
        sys_log(LOGSTDERR, "error:csocket_client_addr_init: unknow addr type\n");
        return ( EC_FALSE );
    }

    /* fill ip addr and port */
    srv_addr->sin_family = AF_INET;
    srv_addr->sin_port   = htons( atoi(c_word_to_port(srv_port)) );
    srv_addr->sin_addr   = *((struct in_addr *)hp->h_addr);
    bzero(srv_addr->sin_zero, sizeof(srv_addr->sin_zero)/sizeof(srv_addr->sin_zero[0]));

    //sys_log(LOGSTDOUT, "csocket_client_addr_init: %s\n", inet_ntoa(srv_addr->sin_addr.s_addr));

    return  ( EC_TRUE );
}

EC_BOOL csocket_nonblock_enable(int sockfd)
{
    int flag;

    sys_log(LOGSTDOUT, "csocket_nonblock_enable: sockfd %d\n", sockfd);
    flag = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, O_NONBLOCK | flag);
    return (EC_TRUE);
}

EC_BOOL csocket_nonblock_disable(int sockfd)
{
    int flag;

    sys_log(LOGSTDOUT, "csocket_nonblock_disable: sockfd %d\n", sockfd);
    flag = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, (~O_NONBLOCK) & flag);
    return (EC_TRUE);
}

EC_BOOL csocket_is_nonblock(const int sockfd)
{
    int flag;

    flag = fcntl(sockfd, F_GETFL, 0);

    if(flag & O_NONBLOCK)
    {
        //sys_log(LOGSTDOUT, "csocket_is_nonblock: sockfd %d is nonblock\n", sockfd);
        return (EC_TRUE);
    }
    //sys_log(LOGSTDOUT, "csocket_is_nonblock: sockfd %d is NOT nonblock\n", sockfd);
    return (EC_FALSE);
}

EC_BOOL csocket_optimize(int sockfd)
{
    EC_BOOL ret;

    ret = EC_TRUE;
    /* optimization 1: disalbe Nagle Algorithm */
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to disable Nagle Algo\n", sockfd);
            ret = EC_FALSE;
        }
    }

    /*optimization: quick ack*/
    if(1)
    {
        int flag;
        flag = 1;
        if(0 != setsockopt(sockfd, IPPROTO_TCP, TCP_QUICKACK, (char *) &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR,"warn: csocket_optimize: socket %d failed to enable QUICKACK\n", sockfd);
            ret = EC_FALSE;
        }
    }        

    /* optimization 2.1: when flag > 0, set SEND_BUFF size per packet - Flow Control*/
    /* optimization 2.2: when flag = 0, the data buff to send will NOT copy to system buff but send out directly*/
    if(1)
    {
        int flag;
        flag = CSOCKET_SOSNDBUFF_SIZE;
        if( 0 != setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set SEND BUFF to %d\n", sockfd, flag);
            ret = EC_FALSE;
        }
    }

    /* optimization 3.1: when flag > 0, set RECV_BUFF size per packet - Flow Control*/
    /* optimization 3.2: when flag = 0, the data buff to recv will NOT copy from system buff but recv in directly*/
    if(1)
    {
        int flag;
        flag = CSOCKET_SORCVBUFF_SIZE;
        if( 0 != setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, (char *)&flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set RECV BUFF to %d\n", sockfd, flag);
            ret = EC_FALSE;
        }
    }

    /* optimization 4: set KEEPALIVE*/
    /*note: SO_KEEPALIVE is only for TCP protocol but not for UDP, hence some guys need to implement heartbeat mechanism */
    /*in application level to cover both TCP and UDP*/
    if(SWITCH_ON == CSOCKET_SO_KEEPALIVE_SWITCH)
    {
        int flag;
        int keep_idle;
        int keep_interval;
        int keep_count;

        flag = 1;/*1: enable KEEPALIVE, 0: disable KEEPALIVE*/
        keep_idle     = CSOCKET_TCP_KEEPIDLE_NSEC; /*if no data transmission in 10 seconds, start to check socket*/
        keep_interval = CSOCKET_TCP_KEEPINTVL_NSEC;  /*send heartbeat packet in interval 5 seconds*/
        keep_count    = CSOCKET_TCP_KEEPCNT_TIMES;  /*send heartbeat packet up to 3 times, if some heartbeat recv ack, then stop. otherwise, regard socket is disconnected*/
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_KEEPALIVE, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set KEEPALIVE\n", sockfd);
            ret = EC_FALSE;
        }

        if( 1 == flag && 0 != setsockopt( sockfd, SOL_TCP, TCP_KEEPIDLE, (char *)&keep_idle, sizeof(keep_idle) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set KEEPIDLE\n", sockfd);
            ret = EC_FALSE;
        }

        if( 1 == flag && 0 != setsockopt( sockfd, SOL_TCP, TCP_KEEPINTVL, (char *)&keep_interval, sizeof(keep_interval) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set KEEPINTVL\n", sockfd);
            ret = EC_FALSE;
        }

        if( 1 == flag && 0 != setsockopt( sockfd, SOL_TCP, TCP_KEEPCNT, (char *)&keep_count, sizeof(keep_count) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set KEEPCNT\n", sockfd);
            ret = EC_FALSE;
        }
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set REUSEADDR\n", sockfd);
            ret = EC_FALSE;
        }
    }

    /* optimization 6: set SEND TIMEOUT. NOTE: the timeout not working for socket connect op*/
    if(0)
    {
        struct timeval timeout;
        time_t usecs = CSOCKET_SO_SNDTIMEO_NSEC * 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_optimize: socket %d failed to set SEND TIMEOUT to %d usecs\n", sockfd, usecs);
            ret = EC_FALSE;
        }
    }

    /* optimization 7: set RECV TIMEOUT. NOTE: the timeout not working for socket connect op*/
    if(0)
    {
        struct timeval timeout;
        time_t usecs = CSOCKET_SO_RCVTIMEO_NSEC * 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR,"warn: csocket_optimize: socket %d failed to set RECV TIMEOUT to %d usecs\n", sockfd, usecs);
            ret = EC_FALSE;
        }
    }

    /* optimization 8: set NONBLOCK*/
    if(0)
    {
        int flag;

        flag = fcntl(sockfd, F_GETFL, 0);
        fcntl(sockfd, F_SETFL, O_NONBLOCK | flag);
    }

    /*optimization 9: disable linger, i.e., send close socket, stop sending/recving at once*/
    if(1)
    {
        struct linger linger_disable;
        linger_disable.l_onoff  = 0; /*disable*/
        linger_disable.l_linger = 0; /*stop after 0 second, i.e., stop at once*/

        if( 0 != setsockopt(sockfd, SOL_SOCKET, SO_LINGER,(const char*)&linger_disable, sizeof(struct linger)))
        {
            sys_log(LOGSTDERR,"warn: csocket_optimize: socket %d failed to disable linger\n", sockfd);
            ret = EC_FALSE;
        }
    }

    return (ret);
}

EC_BOOL csocket_listen( const UINT32 srv_port, int *srv_sockfd )
{
    struct sockaddr_in srv_addr;
    int sockfd;

    /* create socket */
    sockfd = csocket_open( AF_INET, SOCK_STREAM, 0 );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_listen: tcp socket failed, errno = %d, errstr = %s\n", errno, strerror(errno));
        return ( EC_FALSE );
    }

    /* init socket addr structer */
    csocket_srv_addr_init( srv_port, &srv_addr);

    /* note: optimization must before listen at server side*/
    if(EC_FALSE == csocket_optimize(sockfd))
    {
        sys_log(LOGSTDERR, "warn: csocket_listen: socket %d failed in some optimization\n", sockfd);
    }

    //csocket_nonblock_disable(sockfd);

    if ( 0 !=  bind( sockfd, (struct sockaddr *)&srv_addr, sizeof( srv_addr ) ) )
    {
        sys_log(LOGSTDERR, "error:csocket_listen: bind failed, errno = %d, errstr = %s\n", errno, strerror(errno));
        close(sockfd);
        return ( EC_FALSE );
    }

    /* create listen queues */
    if( 0 !=  listen( sockfd, SOMAXCONN) )/*SOMAXCONN = 128 is a system constant*/
    {
        sys_log(LOGSTDERR,"error:csocket_listen: listen failed, errno = %d, errstr = %s\n", errno, strerror(errno));
        close(sockfd);
        return ( EC_FALSE );
    }

    *srv_sockfd = sockfd;

    return ( EC_TRUE );
}

EC_BOOL csocket_accept(const int srv_sockfd, int *conn_sockfd, UINT32 *client_ipaddr)
{
    int new_sockfd;

    struct sockaddr_in sockaddr_in;
    socklen_t sockaddr_len;

    sockaddr_len = sizeof(struct sockaddr_in);
    new_sockfd = accept(srv_sockfd, (struct sockaddr *)&(sockaddr_in), &(sockaddr_len));
    if( 0 > new_sockfd)
    {
        return (EC_FALSE);
    }
    //csocket_is_nonblock(new_sockfd);

    (*client_ipaddr) = c_ipv4_to_word((char *)c_inet_ntos(sockaddr_in.sin_addr));
    (*conn_sockfd) = new_sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_start_udp_bcast_sender( const UINT32 bcast_fr_ipaddr, const UINT32 bcast_port, int *srv_sockfd )
{
    int sockfd;

    sockfd = csocket_open( AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*//*only recv the port-matched udp pkt*/  );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_sender: udp socket failed, errno = %d, errstr = %s\n",
                            errno, strerror(errno));
        return ( EC_FALSE );
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_bcast_sender: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n",
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_sender: set broadcast flag %d failed, errno = %d, errstr = %s\n",
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        struct timeval timeout;
        time_t usecs = 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR,"warn: csocket_start_udp_bcast_sender: socket %d failed to set RECV TIMEOUT to %d usecs, errno = %d, errstr = %s\n",
                                sockfd, usecs, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }
#if 1
    if(1)
    {
        struct sockaddr_in bcast_addr;
        BSET(&bcast_addr, 0, sizeof(bcast_addr));
        bcast_addr.sin_family      = AF_INET;
        //bcast_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);/*send ok*/
        bcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_fr_ipaddr));
        bcast_addr.sin_port        = htons( atoi(c_word_to_port(bcast_port)) );

        if ( 0 !=  bind( sockfd, (struct sockaddr *)&bcast_addr, sizeof( bcast_addr ) ) )
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_sender: bind to %s:%ld failed, errno = %d, errstr = %s\n",
                                c_word_to_ipv4(bcast_fr_ipaddr), bcast_port, errno, strerror(errno));
            close(sockfd);
            return ( EC_FALSE );
        }
        sys_log(LOGSTDERR, "[DEBUG] csocket_start_udp_bcast_sender: bind to %s:%ld successfully\n",
                            c_word_to_ipv4(bcast_fr_ipaddr), bcast_port);
    }
#endif
    *srv_sockfd = sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_stop_udp_bcast_sender( const int sockfd )
{
    if(CMPI_ERROR_SOCKFD != sockfd)
    {
        close(sockfd);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_start_udp_bcast_recver( const UINT32 bcast_to_ipaddr, const UINT32 bcast_port, int *srv_sockfd )
{
    int sockfd;

    sockfd = csocket_open( AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*//*only recv the port-matched udp pkt*/ );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: udp socket failed, errno = %d, errstr = %s\n",
                            errno, strerror(errno));
        return ( EC_FALSE );
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_bcast_recver: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n",
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: set broadcast flag %d failed, errno = %d, errstr = %s\n",
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        struct timeval timeout;
        time_t usecs = 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR,"warn: csocket_start_udp_bcast_recver: socket %d failed to set RECV TIMEOUT to %d usecs, errno = %d, errstr = %s\n",
                              sockfd, usecs, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }
#if 1
    if(1)
    {
        struct sockaddr_in bcast_addr;
        BSET(&bcast_addr, 0, sizeof(bcast_addr));
        bcast_addr.sin_family      = AF_INET;
        //bcast_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);
        //bcast_addr.sin_addr.s_addr = INADDR_ANY;/*ok,note: if not bind it, socket will recv nothing. faint!*/
        bcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_to_ipaddr));
        bcast_addr.sin_port        = htons( atoi(c_word_to_port(bcast_port)) );

        if ( 0 !=  bind( sockfd, (struct sockaddr *)&bcast_addr, sizeof( bcast_addr ) ) )
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: bind to %s:%ld failed, errno = %d, errstr = %s\n",
                                c_word_to_ipv4(bcast_to_ipaddr), bcast_port, errno, strerror(errno));
            close(sockfd);
            return ( EC_FALSE );
        }
        sys_log(LOGSTDOUT, "[DEBUG] csocket_start_udp_bcast_recver: bind to %s:%ld successfully\n",
                            c_word_to_ipv4(bcast_to_ipaddr), bcast_port);

    }
#endif

    *srv_sockfd = sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_start_udp_bcast_recver1( const UINT32 bcast_to_ipaddr, const UINT32 bcast_port, int *srv_sockfd )
{
    int sockfd;
    const char *net_if = "eth0";

    sockfd = csocket_open( PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: udp socket failed, errno = %d, errstr = %s\n",
                           errno, strerror(errno));
        return ( EC_FALSE );
    }
    if(1)
    {
        struct ifreq ifr;

        strncpy(ifr.ifr_name, net_if, strlen(net_if) + 1);
        if((ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: get %s flags failed, errno = %d, errstr = %s\n",
                                net_if, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }

        ifr.ifr_flags |= IFF_PROMISC;

        if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1 )
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: set %s flags with IFF_PROMISC failed, errno = %d, errstr = %s\n",
                                net_if, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
   }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_bcast_recver: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n",
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: set broadcast flag %d failed, errno = %d, errstr = %s\n",
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        struct timeval timeout;
        time_t usecs = 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR,"warn: csocket_start_udp_bcast_recver: socket %d failed to set RECV TIMEOUT to %d usecs, errno = %d, errstr = %s\n",
                            sockfd, usecs, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }
#if 1
    if(1)
    {
        struct sockaddr_in bcast_addr;
        BSET(&bcast_addr, 0, sizeof(bcast_addr));
        bcast_addr.sin_family      = AF_INET;
        //bcast_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);
        //bcast_addr.sin_addr.s_addr = INADDR_ANY;/*note: if not bind it, socket will recv nothing. faint!*/
        bcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_to_ipaddr));
        bcast_addr.sin_port        = htons( atoi(c_word_to_port(bcast_port)) );

        if ( 0 !=  bind( sockfd, (struct sockaddr *)&bcast_addr, sizeof( bcast_addr ) ) )
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_recver: bind to %s:%ld failed, errno = %d, errstr = %s\n",
                               c_word_to_ipv4(bcast_to_ipaddr), bcast_port, errno, strerror(errno));
            close(sockfd);
            return ( EC_FALSE );
        }
        sys_log(LOGSTDOUT, "[DEBUG] csocket_start_udp_bcast_recver: bind to %s:%ld failed \n",
                           c_word_to_ipv4(bcast_to_ipaddr), bcast_port);
    }
#endif

    *srv_sockfd = sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_stop_udp_bcast_recver( const int sockfd )
{
    if(CMPI_ERROR_SOCKFD != sockfd)
    {
        close(sockfd);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_set_promisc(const char *nif, int sock)
{
    struct ifreq ifr;

    strncpy(ifr.ifr_name, nif,strlen(nif)+1);
    if(-1 == ioctl(sock, SIOCGIFFLAGS, &ifr))
    {
        sys_log(LOGSTDOUT, "error:csocket_set_promisc: get %s flags failed, errno = %d, errstr = %s\n",
                            nif, errno, strerror(errno));
        return (EC_FALSE);
    }

   ifr.ifr_flags |= IFF_PROMISC;

   if(-1 == ioctl(sock, SIOCSIFFLAGS, &ifr))
   {
        sys_log(LOGSTDOUT, "error:csocket_set_promisc: set %s IFF_PROMISC failed, errno = %d, errstr = %s\n",
                            nif, errno, strerror(errno));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG]csocket_set_promisc: set %s IFF_PROMISC successfully\n",
                        nif);

    return (EC_TRUE);
}

EC_BOOL csocket_udp_bcast_send(const UINT32 bcast_fr_ipaddr, const UINT32 bcast_to_ipaddr, const UINT32 bcast_port, const UINT8 *data, const UINT32 dlen)
{
    int sockfd;

    sockfd = csocket_open( AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*//*only recv the port-matched udp pkt*/  );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_send: udp socket failed, errno = %d, errstr = %s\n",
                            errno, strerror(errno));
        return ( EC_FALSE );
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_bcast_send: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n",
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_send: set broadcast flag %d failed, errno = %d, errstr = %s\n",
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        struct timeval timeout;
        time_t usecs = 1000;

        timeout.tv_sec  = usecs / 1000;
        timeout.tv_usec = usecs % 1000;
        if ( 0 != setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval) ) )
        {
            sys_log(LOGSTDERR,"warn: csocket_start_udp_bcast_send: socket %d failed to set RECV TIMEOUT to %d usecs, errno = %d, errstr = %s\n",
                                sockfd, usecs, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    csocket_set_promisc((char *)"eth0", sockfd);
#if 0
    if(1)
    {
        struct sockaddr_in bcast_addr;
        BSET(&bcast_addr, 0, sizeof(bcast_addr));
        bcast_addr.sin_family      = AF_INET;
        //bcast_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);/*send ok*/
        bcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_fr_ipaddr));
        bcast_addr.sin_port        = htons( atoi(c_word_to_port(bcast_port)) );

        if ( 0 !=  bind( sockfd, (struct sockaddr *)&bcast_addr, sizeof( bcast_addr ) ) )
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_bcast_send: bind to %s:%ld failed, errno = %d, errstr = %s\n",
                                c_word_to_ipv4(bcast_fr_ipaddr), bcast_port, errno, strerror(errno));
            close(sockfd);
            return ( EC_FALSE );
        }
        sys_log(LOGSTDERR, "[DEBUG] csocket_start_udp_bcast_send: bind to %s:%ld successfully\n",
                            c_word_to_ipv4(bcast_fr_ipaddr), bcast_port);
    }
#endif

    csocket_udp_bcast_sendto(sockfd, bcast_to_ipaddr, bcast_port, data, dlen);
    close(sockfd);
    return (EC_TRUE);
}

EC_BOOL csocket_udp_bcast_sendto(const int sockfd, const UINT32 bcast_to_ipaddr, const UINT32 bcast_port, const UINT8 *data, const UINT32 dlen)
{
    struct sockaddr_in bcast_to_addr;

    /*make sure dlen < 255*/
    if(dlen != (dlen & 0xFF))
    {
        sys_log(LOGSTDOUT, "error:csocket_udp_bcast_sendto: dlen %ld overflow\n", dlen);
        return (EC_FALSE);
    }

    BSET(&bcast_to_addr, 0, sizeof(bcast_to_addr));
    bcast_to_addr.sin_family = AF_INET;
    //bcast_to_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);
    bcast_to_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_to_ipaddr))/*inet_addr(bcast_to_ipaddr_str)*/;
    bcast_to_addr.sin_port = htons( atoi(c_word_to_port(bcast_port)) );

    if(0 > sendto(sockfd, data, dlen, 0, (struct sockaddr*)&bcast_to_addr,sizeof(bcast_to_addr)))
    {
        sys_log(LOGSTDERR, "error:csocket_udp_bcast_sendto: send %ld bytes to bcast %s:%ld failed, errno = %d, errstr = %s\n",
                            dlen, c_word_to_ipv4(bcast_to_ipaddr), bcast_port, errno, strerror(errno));
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_bcast_sendto: send %ld bytes to bcast %s:%ld\n",
                        dlen, c_word_to_ipv4(bcast_to_ipaddr), bcast_port);

    return (EC_TRUE);
}

EC_BOOL csocket_udp_bcast_recvfrom(const int sockfd, const UINT32 bcast_fr_ipaddr, const UINT32 bcast_port, UINT8 *data, const UINT32 max_dlen, UINT32 *dlen)
{
    struct sockaddr_in bcast_fr_addr;
    socklen_t bcast_fr_addr_len;
    int csize;

    BSET(&bcast_fr_addr, 0, sizeof(bcast_fr_addr));
    bcast_fr_addr.sin_family = AF_INET;
    //bcast_fr_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);
    bcast_fr_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_fr_ipaddr))/*inet_addr(bcast_fr_ipaddr_str)*/;
    bcast_fr_addr.sin_port = htons( atoi(c_word_to_port(bcast_port)) );

    bcast_fr_addr_len = sizeof(bcast_fr_addr);

    csize = recvfrom(sockfd, (void *)data, max_dlen, 0, (struct sockaddr *)&bcast_fr_addr, &bcast_fr_addr_len);
    if(0 > csize)
    {
        sys_log(LOGSTDERR, "error:csocket_udp_bcast_recvfrom: recv from bcast %s:%ld failed, errno = %d, errstr = %s\n",
                            c_word_to_ipv4(bcast_fr_ipaddr), bcast_port, errno, strerror(errno));
        return (EC_FALSE);
    }

    (*dlen) = csize;

    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_bcast_recvfrom: recv %d bytes from bcast %s:%ld\n",
                        csize, c_word_to_ipv4(bcast_fr_ipaddr), bcast_port);
    return (EC_TRUE);
}

EC_BOOL csocket_udp_bcast_recvfrom1(const int sockfd, const UINT32 bcast_fr_ipaddr, const UINT32 bcast_port, UINT8 *data, const UINT32 max_dlen, UINT32 *dlen)
{
    struct sockaddr_in bcast_fr_addr;
    socklen_t bcast_fr_addr_len;
    int csize;

    BSET(&bcast_fr_addr, 0, sizeof(bcast_fr_addr));
#if 0
    bcast_fr_addr.sin_family = AF_INET;
    bcast_fr_addr.sin_addr.s_addr = htonl(/*INADDR_ANY*/INADDR_BROADCAST);
    //bcast_fr_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(bcast_ipaddr))/*inet_addr(bcast_ipaddr_str)*/;
    bcast_fr_addr.sin_port = htons( atoi(c_word_to_port(bcast_port)) );
#endif
    bcast_fr_addr_len = sizeof(bcast_fr_addr);

    csize = recvfrom(sockfd, (void *)data, max_dlen, 0, (struct sockaddr *)&bcast_fr_addr, &bcast_fr_addr_len);
    if(0 > csize)
    {
        sys_log(LOGSTDERR, "error:csocket_udp_bcast_recvfrom: recv from bcast %s:%ld failed, errno = %d, errstr = %s\n",
                            c_word_to_ipv4(bcast_fr_ipaddr), bcast_port, errno, strerror(errno));
        return (EC_FALSE);
    }

    (*dlen) = csize;

    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_bcast_recvfrom: recv %d bytes from bcast %s:%ld\n",
                        csize, c_word_to_ipv4(bcast_fr_ipaddr), bcast_port);
    return (EC_TRUE);
}

EC_BOOL csocket_start_udp_mcast_sender( const UINT32 mcast_ipaddr, const UINT32 srv_port, int *srv_sockfd )
{
    int sockfd;

    sockfd = csocket_open( AF_INET, SOCK_DGRAM, 0 );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_sender: udp socket failed\n");
        return ( EC_FALSE );
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_mcast_sender: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n", 
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_sender: set multicast loop flag %d failed, errno = %d, errstr = %s\n", 
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    *srv_sockfd = sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_stop_udp_mcast_sender( const int sockfd, const UINT32 mcast_ipaddr )
{
    if(CMPI_ERROR_SOCKFD != sockfd)
    {
        close(sockfd);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_start_udp_mcast_recver( const UINT32 mcast_ipaddr, const UINT32 srv_port, int *srv_sockfd )
{
    struct sockaddr_in srv_addr;
    int sockfd;

    sockfd = csocket_open( AF_INET, SOCK_DGRAM, 0 );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_recver: udp socket failed\n");
        return ( EC_FALSE );
    }

    /* optimization 5: set REUSEADDR*/
    if(1)
    {
        int flag;
        flag = 1;
        if( 0 != setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&flag, sizeof(flag) ) )
        {
            sys_log(LOGSTDERR, "warn: csocket_start_udp_mcast_recver: socket %d failed to set REUSEADDR, errno = %d, errstr = %s\n", 
                                sockfd, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    if(1)
    {
        int flag;
        flag = 1;
        if(0 > setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &flag, sizeof(flag)))
        {
            sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_recver: set multicast loop flag %d failed, errno = %d, errstr = %s\n", 
                                flag, errno, strerror(errno));
            close(sockfd);
            return (EC_FALSE);
        }
    }

    /*join multicast*/
    if(EC_FALSE == csocket_join_mcast(sockfd, mcast_ipaddr))
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_recver: join mcast %s failed\n", c_word_to_ipv4(mcast_ipaddr));
        close(sockfd);
        return (EC_FALSE);
    }

    /* init socket addr structer */
    csocket_srv_addr_init( srv_port, &srv_addr);

    /*udp receiver must bind mcast ipaddr & port*/
    if ( 0 !=  bind( sockfd, (struct sockaddr *)&srv_addr, sizeof( srv_addr ) ) )
    {
        sys_log(LOGSTDERR, "error:csocket_start_udp_mcast_recver: bind failed, errno = %d, errstr = %s\n",
                            errno, strerror(errno));
        close(sockfd);
        return ( EC_FALSE );
    }

    *srv_sockfd = sockfd;

    return (EC_TRUE);
}

EC_BOOL csocket_stop_udp_mcast_recver( const int sockfd, const UINT32 mcast_ipaddr )
{
    if(EC_FALSE == csocket_drop_mcast(sockfd, mcast_ipaddr))
    {
        sys_log(LOGSTDERR, "error:csocket_stop_udp_mcast_recver: drop mcast %s failed\n", c_word_to_ipv4(mcast_ipaddr));
        close(sockfd);
        return (EC_FALSE);
    }

    close(sockfd);
    return (EC_TRUE);
}

EC_BOOL csocket_join_mcast(const int sockfd, const UINT32 mcast_ipaddr)
{
    struct ip_mreq mreq;

    mreq.imr_multiaddr.s_addr = htonl(UINT32_TO_INT32(mcast_ipaddr));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(0 > setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        sys_log(LOGSTDERR, "error:csocket_join_mcast: add to mcast %s failed, errno = %d, errstr = %s\n", 
                            c_word_to_ipv4(mcast_ipaddr), errno, strerror(errno));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csocket_drop_mcast(const int sockfd, const UINT32 mcast_ipaddr)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = htonl(UINT32_TO_INT32(mcast_ipaddr));
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(0 > setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)))
    {
        sys_log(LOGSTDERR, "error:csocket_drop_mcast: drop from mcast %s failed, errno = %d, errstr = %s\n", 
                            c_word_to_ipv4(mcast_ipaddr), errno, strerror(errno));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_udp_sendto(const int sockfd, const UINT32 mcast_ipaddr, const UINT32 mcast_port, const UINT8 *data, const UINT32 dlen)
{
    struct sockaddr_in mcast_addr;
    ssize_t o_len;

    BSET(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(mcast_ipaddr));
    mcast_addr.sin_port = htons( atoi(c_word_to_port(mcast_port)) );

    if(0 > (o_len = sendto(sockfd, data, dlen, 0, (struct sockaddr*)&mcast_addr,sizeof(mcast_addr))))
    {
        sys_log(LOGSTDERR, "error:csocket_udp_sendto: send %ld bytes to mcast %s:%ld failed, errno = %d, errstr = %s\n",
                            dlen, c_word_to_ipv4(mcast_ipaddr), mcast_port, errno, strerror(errno));
        return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_sendto: send %ld of %ld bytes  to mcast %s:%ld failed\n",
                        o_len, dlen, c_word_to_ipv4(mcast_ipaddr), mcast_port);
    return (EC_TRUE);
}

EC_BOOL csocket_udp_mcast_sendto(const int sockfd, const UINT32 mcast_ipaddr, const UINT32 mcast_port, const UINT8 *data, const UINT32 dlen)
{
    struct sockaddr_in mcast_addr;

    uint32_t tlen;/*total length*/
    uint32_t csize;/*send completed size*/
    uint16_t osize;/*send once size*/
    uint16_t seqno;

    /*make sure dlen is 32 bits only*/
    if(dlen != (dlen & 0xFFFFFFFF))
    {
        sys_log(LOGSTDOUT, "error:csocket_udp_mcast_sendto: dlen %ld overflow\n", dlen);
        return (EC_FALSE);
    }

    tlen = UINT32_TO_INT32(dlen);

    BSET(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(mcast_ipaddr));
    mcast_addr.sin_port = htons( atoi(c_word_to_port(mcast_port)) );

    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_mcast_sendto: send %ld bytes to mcast %s:%ld\n",
                        dlen, c_word_to_ipv4(mcast_ipaddr), mcast_port);

    /*one udp packet format: total len (4B)| packet len (2B) | seqno (2B) | data (packet len bytes)*/
    for(csize = 0, seqno = 0, osize = CSOCKET_UDP_PACKET_DATA_SIZE; csize < tlen; csize += osize, seqno ++)
    {
        uint8_t  udp_packet[CSOCKET_UDP_PACKET_HEADER_SIZE + CSOCKET_UDP_PACKET_DATA_SIZE];
        uint32_t counter;

        if(csize + osize > tlen)
        {
            osize = tlen - csize;
        }

        counter = 0;
        gdbPut32(udp_packet, &counter, tlen);
        gdbPut16(udp_packet, &counter, osize);
        gdbPut16(udp_packet, &counter, seqno);
        gdbPut8s(udp_packet, &counter, data + csize, osize);

        sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_mcast_sendto: tlen %d, osize %d, seqno %d\n", tlen, osize, seqno);

        if(0 > sendto(sockfd, udp_packet, counter, 0, (struct sockaddr*)&mcast_addr,sizeof(mcast_addr)))
        {
            sys_log(LOGSTDERR, "error:csocket_udp_mcast_sendto: send %ld bytes to mcast %s:%ld failed, errno = %d, errstr = %s\n",
                                counter, c_word_to_ipv4(mcast_ipaddr), mcast_port, errno, strerror(errno));
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL csocket_udp_mcast_recvfrom(const int sockfd, const UINT32 mcast_ipaddr, const UINT32 mcast_port, UINT8 *data, const UINT32 max_dlen, UINT32 *dlen)
{
    struct sockaddr_in mcast_addr;
    socklen_t mcast_addr_len;

    CBITMAP  *cbitmap;
    uint32_t  csize;/*send completed size*/


    BSET(&mcast_addr, 0, sizeof(mcast_addr));
    mcast_addr.sin_family = AF_INET;
    mcast_addr.sin_addr.s_addr = htonl(UINT32_TO_INT32(mcast_ipaddr));
    //mcast_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    mcast_addr.sin_port = htons( atoi(c_word_to_port(mcast_port)) );

    mcast_addr_len = sizeof(mcast_addr);

    cbitmap = NULL_PTR;
    csize   = 0;

    do
    {
        uint8_t   udp_packet[CSOCKET_UDP_PACKET_HEADER_SIZE + CSOCKET_UDP_PACKET_DATA_SIZE];

        uint32_t tlen;/*total length*/
        uint16_t osize;/*send once size*/
        uint16_t seqno;

        uint32_t counter;
        uint32_t offset;

        if(0 > recvfrom(sockfd, (void *)udp_packet, sizeof(udp_packet)/sizeof(udp_packet[0]), 0, (struct sockaddr *)&mcast_addr, &mcast_addr_len))
        {
            sys_log(LOGSTDERR, "error:csocket_udp_mcast_recvfrom: recv %ld bytes from mcast %s:%ld failed, errno = %d, errstr = %s\n",
                                osize, c_word_to_ipv4(mcast_ipaddr), mcast_port, errno, strerror(errno));
            cbitmap_free(cbitmap);/*also works when cbitmap is NULL_PTR*/
            return (EC_FALSE);
        }

        counter = 0;
        tlen  = gdbGet32(udp_packet, &counter);
        osize = gdbGet16(udp_packet, &counter);
        seqno = gdbGet16(udp_packet, &counter);

        if(NULL_PTR != cbitmap && EC_TRUE == cbitmap_check(cbitmap, seqno))
        {
            continue;
        }

        sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_mcast_recvfrom: tlen %d, osize %d, seqno %d\n", tlen, osize, seqno);

        if(NULL_PTR == cbitmap)
        {
            UINT32 max_bits;

            max_bits = ((tlen + CSOCKET_UDP_PACKET_DATA_SIZE - 1)/ CSOCKET_UDP_PACKET_DATA_SIZE);
            cbitmap = cbitmap_new(max_bits);
            if(NULL_PTR == cbitmap)
            {
                sys_log(LOGSTDERR, "error:csocket_udp_mcast_recvfrom: new cbitmap with max bits %d failed\n", max_bits);
                return (EC_FALSE);
            }

            /*so sorry for adjustment due to cbitmap_new will align the max bits to multiple of WORDSIZE :-(*/
            CBITMAP_MAX_BITS(cbitmap) = max_bits;
        }

        offset = seqno * CSOCKET_UDP_PACKET_DATA_SIZE;
        if(offset + osize > max_dlen)
        {
            sys_log(LOGSTDERR, "error:csocket_udp_mcast_recvfrom: no enough room to accept %d bytes at offset %d\n", osize, offset);
            cbitmap_free(cbitmap);
            return (EC_FALSE);
        }

        BCOPY(udp_packet + counter, data + offset, osize);
        csize += osize;

        cbitmap_set(cbitmap, seqno);

        //cbitmap_print(cbitmap, LOGSTDOUT);
    }while(EC_FALSE == cbitmap_is_full(cbitmap));

    sys_log(LOGSTDOUT, "[DEBUG] csocket_udp_mcast_recvfrom: recv %d bytes from mcast %s:%ld\n",
                        csize, c_word_to_ipv4(mcast_ipaddr), mcast_port);

    cbitmap_free(cbitmap);
    (*dlen) = csize;
    return (EC_TRUE);
}

EC_BOOL csocket_send_confirm(const int sockfd)
{
    sys_log(LOGSTDOUT, "csocket_send_confirm: start to send data on sockfd %d\n", sockfd);
    csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    if(-1 == send(sockfd, NULL_PTR, 0, 0) && EAGAIN == errno)
    {
        sys_log(LOGSTDOUT, "csocket_send_confirm: sockfd %d connection confirmed\n", sockfd);
        return (EC_TRUE);
    }
    sys_log(LOGSTDOUT, "csocket_send_confirm: sockfd %d errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
    return (EC_FALSE);
}

EC_BOOL csocket_recv_confirm(const int sockfd)
{
    sys_log(LOGSTDOUT, "csocket_recv_confirm: start to recv data on sockfd %d\n", sockfd);
    csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    if(-1 == recv(sockfd, NULL_PTR, 0, 0) && EAGAIN == errno)
    {
        sys_log(LOGSTDOUT, "csocket_read_confirm: sockfd %d connection confirmed\n", sockfd);
        return (EC_TRUE);
    }
    sys_log(LOGSTDOUT, "csocket_recv_confirm: errno = %d, errstr = %s\n", errno, strerror(errno));
    return (EC_FALSE);
}

/*get local ipaddr of sockfd*/
EC_BOOL csocket_name(const int sockfd, CSTRING *ipaddr)
{
    struct sockaddr_in sockaddr_in;
    socklen_t sockaddr_len;

    sockaddr_len = sizeof(struct sockaddr_in);
    if(0 != getsockname(sockfd, (struct sockaddr *)&(sockaddr_in), &(sockaddr_len)))
    {
        sys_log(LOGSTDOUT, "error:csocket_name: failed to get ipaddr of sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cstring_init(ipaddr, (UINT8 *)c_inet_ntos(sockaddr_in.sin_addr));
    return (EC_TRUE);
}

EC_BOOL csocket_connect_wait_ready(int sockfd)
{
    FD_CSET fd_cset;
    int max_sockfd;
    int ret;
    int len;
    int err;
    struct timeval timeout;
      
    timeout.tv_sec  = 5;
    timeout.tv_usec = 0;    

    max_sockfd = 0;
    csocket_fd_clean(&fd_cset);
    csocket_fd_set(sockfd, &fd_cset, &max_sockfd);
    if(EC_FALSE == csocket_select(max_sockfd + 1, &fd_cset, NULL_PTR, NULL_PTR, &timeout, &ret))
    {
        return (EC_FALSE);
    }

    len = sizeof(int);
    if(0 != getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&err, (socklen_t *)&len))
    {
        sys_log(LOGSTDOUT, "error:csocket_connect_wait_ready: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    if(0 != err)
    {
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csocket_connect_0( const UINT32 srv_ipaddr, const UINT32 srv_port, const UINT32 csocket_block_mode, int *client_sockfd )
{
    struct sockaddr_in srv_addr;

    int sockfd;

    /* initialize the ip addr and port of server */
    if( EC_FALSE == csocket_client_addr_init( srv_ipaddr, srv_port, &srv_addr ) )
    {
        sys_log(LOGSTDERR,"error:csocket_connect: csocket_client_addr_init failed\n");
        return ( EC_FALSE );
    }

    /* create socket */
    sockfd = csocket_open( AF_INET, SOCK_STREAM, 0 );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_connect: socket error\n");
        return ( EC_FALSE );
    }

    /* note: optimization must before connect at server side*/
    if(EC_FALSE == csocket_optimize(sockfd))
    {
        sys_log(LOGSTDERR, "warn:csocket_connect: socket %d failed in some optimization\n", sockfd);
    }

    csocket_nonblock_enable(sockfd);

    /* connect to server, connect timeout is default 75s */
    if(0 > connect(sockfd, (struct sockaddr *) &srv_addr, sizeof(struct sockaddr)) /*&& EINPROGRESS != errno && EINTR != errno*/)
    {
        int errcode;

        errcode = errno;

        switch(errcode)
        {
            case EACCES:
                sys_log(LOGSTDOUT, "error:csocket_connect: write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix\n");
                break;

            case EPERM:
                sys_log(LOGSTDOUT, "error:csocket_connect: tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule\n");
                break;

            case EADDRINUSE:
                sys_log(LOGSTDOUT, "error:csocket_connect: local address is already in use\n");
                break;

            case EAFNOSUPPORT:
                sys_log(LOGSTDOUT, "error:csocket_connect: The passed address not have the correct address family in its sa_family fiel\n");
                break;

            case EADDRNOTAVAIL:
                sys_log(LOGSTDOUT, "error:csocket_connect: non-existent interface was requested or the requested address was not local\n");
                break;

            case EALREADY:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket is non-blocking and a previous connection attempt has not yet been completed\n");
                break;

            case EBADF:
                sys_log(LOGSTDOUT, "error:csocket_connect: the file descriptor is not a valid index in the descriptor tabl\n");
                break;

            case ECONNREFUSED:
                sys_log(LOGSTDOUT, "error:csocket_connect: no one listening on the remote address\n");
                break;

            case EFAULT:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket structure address is outside the user address space\n");
                break;

            case EINPROGRESS:
                sys_log(LOGSTDOUT, "warn:csocket_connect: the socket is non-blocking and the connection cannot be completed immediately\n");
                if(EC_FALSE == csocket_connect_wait_ready(sockfd))
                {
                    sys_log(LOGSTDOUT, "error:csocket_connect: wait socket %d connection complete failed\n", sockfd);
                    break;
                }
                
                if(CSOCKET_IS_BLOCK_MODE == csocket_block_mode)
                {
                    csocket_nonblock_disable(sockfd);
                }
                *client_sockfd = sockfd;
                return ( EC_TRUE );

            case EINTR:
                sys_log(LOGSTDOUT, "warn:csocket_connect: the system call was interrupted by a signal that was caugh\n");
                if(CSOCKET_IS_BLOCK_MODE == csocket_block_mode)
                {
                    csocket_nonblock_disable(sockfd);
                }
                *client_sockfd = sockfd;
                return ( EC_TRUE );

            case EISCONN:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket is already connected\n");
                break;

            case ENETUNREACH:
                sys_log(LOGSTDOUT, "error:csocket_connect: network is unreachabl\n");
                break;

            case ENOTSOCK:
                sys_log(LOGSTDOUT, "error:csocket_connect: the file descriptor is not associated with a socket\n");
                break;

            case ETIMEDOUT:
                sys_log(LOGSTDOUT, "error:csocket_connect: timeout while attempting connection. The server may be too busy to accept new connection\n");
                break;

            default:
                sys_log(LOGSTDOUT, "error:csocket_connect: unknown errno = %d\n", errcode);
        }

        //sys_log(LOGSTDERR,"error:csocket_connect: sockfd %d connect error, errno = %d, errstr = %s\n", sockfd, errcode, strerror(errcode));
        //csocket_tcpi_stat_print(LOGSTDERR, sockfd);
        close(sockfd);
        return ( EC_FALSE );
    }

    if(CSOCKET_IS_BLOCK_MODE == csocket_block_mode)
    {
        csocket_nonblock_disable(sockfd);
    }
    *client_sockfd = sockfd;
    return ( EC_TRUE );
}

EC_BOOL csocket_connect( const UINT32 srv_ipaddr, const UINT32 srv_port, const UINT32 csocket_block_mode, int *client_sockfd )
{
    struct sockaddr_in srv_addr;

    int sockfd;

    /* initialize the ip addr and port of server */
    if( EC_FALSE == csocket_client_addr_init( srv_ipaddr, srv_port, &srv_addr ) )
    {
        sys_log(LOGSTDERR,"error:csocket_connect: csocket_client_addr_init failed\n");
        return ( EC_FALSE );
    }

    /* create socket */
    sockfd = csocket_open( AF_INET, SOCK_STREAM, 0 );
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_connect: socket error\n");
        return ( EC_FALSE );
    }

    /* note: optimization must before connect at server side*/
    if(EC_FALSE == csocket_optimize(sockfd))
    {
        sys_log(LOGSTDERR, "warn:csocket_connect: socket %d failed in some optimization\n", sockfd);
    }

    csocket_nonblock_disable(sockfd);

    /* connect to server, connect timeout is default 75s */
    if(0 > connect(sockfd, (struct sockaddr *) &srv_addr, sizeof(struct sockaddr)) /*&& EINPROGRESS != errno && EINTR != errno*/)
    {
        int errcode;

        errcode = errno;

        switch(errcode)
        {
            case EACCES:
                sys_log(LOGSTDOUT, "error:csocket_connect: write permission is denied on the socket file, or search permission is denied for one of the directories in the path prefix\n");
                break;

            case EPERM:
                sys_log(LOGSTDOUT, "error:csocket_connect: tried to connect to a broadcast address without having the socket broadcast flag enabled or the connection request failed because of a local firewall rule\n");
                break;

            case EADDRINUSE:
                sys_log(LOGSTDOUT, "error:csocket_connect: local address is already in use\n");
                break;

            case EAFNOSUPPORT:
                sys_log(LOGSTDOUT, "error:csocket_connect: The passed address not have the correct address family in its sa_family fiel\n");
                break;

            case EADDRNOTAVAIL:
                sys_log(LOGSTDOUT, "error:csocket_connect: non-existent interface was requested or the requested address was not local\n");
                break;

            case EALREADY:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket is non-blocking and a previous connection attempt has not yet been completed\n");
                break;

            case EBADF:
                sys_log(LOGSTDOUT, "error:csocket_connect: the file descriptor is not a valid index in the descriptor tabl\n");
                break;

            case ECONNREFUSED:
                sys_log(LOGSTDOUT, "error:csocket_connect: no one listening on the remote address\n");
                break;

            case EFAULT:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket structure address is outside the user address space\n");
                break;

            case EINPROGRESS:
                sys_log(LOGSTDOUT, "warn:csocket_connect: the socket is non-blocking and the connection cannot be completed immediatel\n");
                if(CSOCKET_IS_NONBLOCK_MODE == csocket_block_mode)
                {
                    csocket_nonblock_enable(sockfd);
                }
                *client_sockfd = sockfd;
                return ( EC_TRUE );

            case EINTR:
                sys_log(LOGSTDOUT, "warn:csocket_connect: the system call was interrupted by a signal that was caugh\n");
                if(CSOCKET_IS_NONBLOCK_MODE == csocket_block_mode)
                {
                    csocket_nonblock_enable(sockfd);
                }
                *client_sockfd = sockfd;
                return ( EC_TRUE );

            case EISCONN:
                sys_log(LOGSTDOUT, "error:csocket_connect: the socket is already connected\n");
                break;

            case ENETUNREACH:
                sys_log(LOGSTDOUT, "error:csocket_connect: network is unreachabl\n");
                break;

            case ENOTSOCK:
                sys_log(LOGSTDOUT, "error:csocket_connect: the file descriptor is not associated with a socket\n");
                break;

            case ETIMEDOUT:
                sys_log(LOGSTDOUT, "error:csocket_connect: timeout while attempting connection. The server may be too busy to accept new connection\n");
                break;

            default:
                sys_log(LOGSTDOUT, "error:csocket_connect: unknown errno = %d\n", errcode);
        }

        //sys_log(LOGSTDERR,"error:csocket_connect: sockfd %d connect error, errno = %d, errstr = %s\n", sockfd, errcode, strerror(errcode));
        //csocket_tcpi_stat_print(LOGSTDERR, sockfd);
        close(sockfd);
        return ( EC_FALSE );
    }

    if(CSOCKET_IS_NONBLOCK_MODE == csocket_block_mode)
    {
        csocket_nonblock_enable(sockfd);
    }
    *client_sockfd = sockfd;
    return ( EC_TRUE );
}

UINT32 csocket_state(const int sockfd)
{
    struct tcp_info info;
    socklen_t info_len;

    UINT32 state;

    if(-1 == sockfd)
    {
        return ((UINT32)-1);
    }

    info_len = sizeof(struct tcp_info);
    if(0 != getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, (char *)&info, &info_len))
    {
        sys_log(LOGSTDOUT, "csocket_is_established: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return ((UINT32)-1);
    }

    state = info.tcpi_state;
    return (state);
}

EC_BOOL csocket_is_established(const int sockfd)
{
    struct tcp_info info;
    socklen_t info_len;

    if(-1 == sockfd)
    {
        return (EC_FALSE);
    }

    info_len = sizeof(struct tcp_info);
    if(0 != getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, (char *)&info, &info_len))
    {
        sys_log(LOGSTDOUT, "csocket_is_established: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    switch(info.tcpi_state)
    {
        case TCP_ESTABLISHED:
            //sys_log(LOGSTDOUT, "csocket_is_established: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_TRUE);
        case TCP_SYN_SENT   :
        case TCP_SYN_RECV   :
        case TCP_FIN_WAIT1  :
        case TCP_FIN_WAIT2  :
        case TCP_TIME_WAIT  :
        case TCP_CLOSE      :
        case TCP_CLOSE_WAIT :
        case TCP_LAST_ACK   :
        case TCP_LISTEN     :
        case TCP_CLOSING    :
            //sys_log(LOGSTDOUT, "csocket_is_established: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "csocket_is_established: unknown tcpi_stat = %d\n", info.tcpi_state);
    return (EC_FALSE);

}

EC_BOOL csocket_is_connected(const int sockfd)
{
    struct tcp_info info;
    socklen_t info_len;

    if(-1 == sockfd)
    {
        return (EC_FALSE);
    }

    info_len = sizeof(struct tcp_info);
    if(0 != getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, (char *)&info, &info_len))
    {
        sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "[DEBUG] csocket_is_connected: called for sockfd %d\n", sockfd);
    switch(info.tcpi_state)
    {
        case TCP_ESTABLISHED:
            //sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_TRUE);
        case TCP_SYN_SENT   :
            //sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_TRUE);
        case TCP_SYN_RECV   :
            //sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_TRUE);
        case TCP_FIN_WAIT1  :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_FIN_WAIT2  :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_TIME_WAIT  :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_CLOSE      :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_CLOSE_WAIT :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_LAST_ACK   :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
        case TCP_LISTEN     :
            //sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_TRUE);
        case TCP_CLOSING    :
            sys_log(LOGSTDOUT, "csocket_is_connected: sockfd %d, tcpi state = %s\n", sockfd, csocket_tcpi_stat(info.tcpi_state));
            return (EC_FALSE);
    }
    sys_log(LOGSTDOUT, "csocket_is_connected: unknown tcpi_stat = %d\n", info.tcpi_state);
    return (EC_FALSE);
}

EC_BOOL csocket_is_closed(const int sockfd)
{
    struct tcp_info info;
    socklen_t info_len;

    if(-1 == sockfd)
    {
        return (EC_FALSE);
    }

    info_len = sizeof(struct tcp_info);
    if(0 != getsockopt(sockfd, IPPROTO_TCP, TCP_INFO, (char *)&info, &info_len))
    {
        sys_log(LOGSTDOUT, "csocket_is_closed: sockfd %d, errno = %d, errstr = %s\n", sockfd, errno, strerror(errno));
        return (EC_FALSE);
    }

    switch(info.tcpi_state)
    {
        case TCP_ESTABLISHED:
        case TCP_SYN_SENT   :
        case TCP_SYN_RECV   :
            return (EC_FALSE);
        case TCP_FIN_WAIT1  :
        case TCP_FIN_WAIT2  :
        case TCP_TIME_WAIT  :
        case TCP_CLOSE      :
        case TCP_CLOSE_WAIT :
        case TCP_LAST_ACK   :
            return (EC_TRUE);
        case TCP_LISTEN     :
            return (EC_FALSE);
        case TCP_CLOSING    :
            return (EC_TRUE);
    }
    sys_log(LOGSTDOUT, "csocket_is_closed: unknown tcpi_stat = %d\n", info.tcpi_state);
    return (EC_FALSE);

}

EC_BOOL csocket_select(const int sockfd_boundary, FD_CSET *read_sockfd_set, FD_CSET *write_sockfd_set, FD_CSET *except_sockfd_set, struct timeval *timeout, int *retval)
{
    int ret;

    ret = select(sockfd_boundary,
                (NULL_PTR == read_sockfd_set)   ? NULL_PTR : SOCKFD_SET(read_sockfd_set),
                (NULL_PTR == write_sockfd_set)  ? NULL_PTR : SOCKFD_SET(write_sockfd_set),
                (NULL_PTR == except_sockfd_set) ? NULL_PTR : SOCKFD_SET(except_sockfd_set),
                timeout);

    (*retval) = ret;
    if(0 > ret)/*error occur*/
    {
        return csocket_no_ierror();
    }
    return (EC_TRUE);
}

EC_BOOL csocket_shutdown( const int sockfd, const int flag )
{
    int ret;

    ret = shutdown( sockfd, flag );
    if ( 0 > ret )
    {
        sys_log(LOGSTDERR,"error:csocket_shutdown: failed to close socket %d direction %d\n", sockfd, flag);
        return ( EC_FALSE );
    }

    return ( EC_TRUE );
}

int csocket_open(int domain, int type, int protocol)
{
    int sockfd;

    sockfd = socket(domain, type, protocol);
    if ( 0 > sockfd )
    {
        sys_log(LOGSTDERR, "error:csocket_open: open socket failed, errno = %d, errstr = %s\n",
                            errno, strerror(errno));
        return (-1);
    }

    if(1)
    {
        if( 0 > fcntl(sockfd, F_SETFD, FD_CLOEXEC))
        {
            sys_log(LOGSTDOUT, "error:csocket_open: set socket %d to FD_CLOEXEC failed, errno = %d, errstr = %s\n",
                               sockfd, errno, strerror(errno));
            close(sockfd);
            return (-1);
        }
    }
    return ( sockfd );
}

EC_BOOL csocket_close( const int sockfd )
{
    if(EC_TRUE == csocket_is_connected(sockfd))
    {
        close( sockfd );
        sys_log(LOGSTDOUT,"csocket_close: close socket %d\n", sockfd);
        return (EC_TRUE);
    }

    if(-1 == sockfd)
    {
        sys_log(LOGSTDERR, "error:csocket_close: why try to close sockfd %d ?\n", sockfd);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "csocket_close: tcpi stat is: \n");
    csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    close(sockfd);
    sys_log(LOGSTDOUT,"warn: csocket_close: force close socket %d\n", sockfd);

    return ( EC_TRUE );
}

int csocket_errno()
{
    return errno;
}

EC_BOOL csocket_is_eagain()
{
    if(EAGAIN == errno)
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL csocket_no_ierror()
{
    int err;

    err = errno;
    if(EINPROGRESS == err || EAGAIN == err || EWOULDBLOCK == err || EINTR == err)
    {
        return (EC_TRUE); /*no error*/
    }
    sys_log(LOGSTDOUT, "warn:csocket_no_ierror: errno = %d, errstr = %s\n", err, strerror(err));
    return (EC_FALSE);/*found error*/
}

EC_BOOL csocket_wait_for_io(const int sockfd, const UINT32 io_flag)
{
    struct pollfd pfd;
    int ret;

    pfd.fd     = sockfd;

    if(CSOCKET_READING_FLAG == io_flag)
    {
        pfd.events = POLLIN;
    }
    else if(CSOCKET_WRITING_FLAG == io_flag)
    {
        pfd.events = POLLOUT;
    }

    do 
    {
        ret = poll(&pfd, 1/*one sockfd*/, 0/*timeout is zero*/);
    } while (-1 == ret && EINTR == errno);
    
    if (0 == ret) 
    {
        /*timeout, no event*/
        return (EC_FALSE);
    }
    
    if (ret > 0) 
    {
        /*can recv or send*/
        return (EC_TRUE);
    }
    
    return (EC_FALSE);
}

EC_BOOL csocket_isend(const int sockfd, const UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    ssize_t ret;
    UINT32  once_sent_len;

    if(out_buff_max_len < (*position))/*debug*/
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:csocket_isend: out_buff_max_len %ld < pos %ld\n", out_buff_max_len, (*position));
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "csocket_isend: start to send data on sockfd %d: beg: max len = %ld, position = %ld\n", sockfd, out_buff_max_len, *position);
    //csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    once_sent_len = out_buff_max_len - (*position);
    if(0 == once_sent_len)
    {
        return (EC_TRUE);
    }

    once_sent_len = DMIN(CSOCKET_SEND_ONCE_MAX_SIZE, once_sent_len);    

    ret = write(sockfd, (void *)(out_buff + (*position)), once_sent_len);
    if(0 <= ret )/*when ret = 0, no data was sent*/
    {
        (*position) += (ret);
        //sys_log(LOGSTDOUT, "csocket_isend: end to send data on sockfd %d: end: max len = %ld, position = %ld\n", sockfd, out_buff_max_len, *position);
        return (EC_TRUE);
    }
    /*when ret = -1, error happen*/
    sys_log(LOGSTDNULL, "warn:csocket_isend: sockfd %d, errno = %d, errstr = %s, max len %ld, pos %ld\n",
                        sockfd, errno, strerror(errno), out_buff_max_len, (*position));
    return csocket_no_ierror();
}

EC_BOOL csocket_isend_0(const int sockfd, const UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *position)
{
    ssize_t ret;
    UINT32  once_sent_len;

    if(out_buff_max_len < (*position))/*debug*/
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:csocket_isend: out_buff_max_len %ld < pos %ld\n", out_buff_max_len, (*position));
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "csocket_isend: start to send data on sockfd %d: beg: max len = %ld, position = %ld\n", sockfd, out_buff_max_len, *position);
    //csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    if(EC_FALSE == csocket_wait_for_io(sockfd, CSOCKET_WRITING_FLAG))
    {
        return (EC_TRUE);
    }

    once_sent_len = out_buff_max_len - (*position);
    if(0 == once_sent_len)
    {
        return (EC_TRUE);
    }

    once_sent_len = DMIN(CSOCKET_SEND_ONCE_MAX_SIZE, once_sent_len);    

    //ret = send(sockfd, (void *)(out_buff + (*position)), once_sent_len, 0);
    do{
        ret = write(sockfd, (void *)(out_buff + (*position)), once_sent_len);
    }while(-1 == ret && EINTR == errno);
    if(0 <= ret )/*when ret = 0, no data was sent*/
    {
        (*position) += (ret);
        //sys_log(LOGSTDOUT, "csocket_isend: end to send data on sockfd %d: end: max len = %ld, position = %ld\n", sockfd, out_buff_max_len, *position);
        return (EC_TRUE);
    }
    /*when ret = -1, error happen*/
    sys_log(LOGSTDNULL, "warn:csocket_isend: sockfd %d, errno = %d, errstr = %s, max len %ld, pos %ld\n",
                        sockfd, errno, strerror(errno), out_buff_max_len, (*position));
    return csocket_no_ierror();
}

EC_BOOL csocket_irecv(const int sockfd, UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position)
{
    ssize_t ret;
    UINT32  once_recv_len;

    if(in_buff_max_len < (*position))/*debug*/
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:csocket_irecv: out_buff_max_len %ld < pos %ld\n", in_buff_max_len, (*position));
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "csocket_irecv: start to send data on sockfd %d: beg: max len = %ld, position = %ld\n", sockfd, in_buff_max_len, *position);
    //csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    once_recv_len = in_buff_max_len - (*position);
    if(0 == once_recv_len)/*no free space to recv*/
    {
        return (EC_TRUE);
    }

    once_recv_len = DMIN(CSOCKET_RECV_ONCE_MAX_SIZE, once_recv_len);  
    ret = read(sockfd, (void *)(in_buff + (*position)), once_recv_len);
    if(0 <= ret )
    {
        (*position) += (ret);
        //sys_log(LOGSTDOUT, "csocket_irecv: end to recv data on sockfd %d: end: max len = %ld, position = %ld\n", sockfd, in_buff_max_len, *position);
        return (EC_TRUE);
    }
    sys_log(LOGSTDNULL, "warn:csocket_irecv: sockfd %d, errno = %d, errstr = %s, max len %ld, pos %ld\n",
                        sockfd, errno, strerror(errno), in_buff_max_len, (*position));
    return csocket_no_ierror();
}

EC_BOOL csocket_irecv_0(const int sockfd, UINT8 *in_buff, const UINT32 in_buff_max_len, UINT32 *position)
{
    ssize_t ret;
    UINT32  once_recv_len;

    if(in_buff_max_len < (*position))/*debug*/
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:csocket_irecv: out_buff_max_len %ld < pos %ld\n", in_buff_max_len, (*position));
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "csocket_irecv: start to send data on sockfd %d: beg: max len = %ld, position = %ld\n", sockfd, in_buff_max_len, *position);
    //csocket_tcpi_stat_print(LOGSTDOUT, sockfd);

    if(EC_FALSE == csocket_wait_for_io(sockfd, CSOCKET_READING_FLAG))
    {
        return (EC_TRUE);
    }

    once_recv_len = in_buff_max_len - (*position);
    if(0 == once_recv_len)/*no free space to recv*/
    {
        return (EC_TRUE);
    }

    once_recv_len = DMIN(CSOCKET_RECV_ONCE_MAX_SIZE, once_recv_len);    

    //ret = recv(sockfd, (void *)(in_buff + (*position)), once_recv_len, 0);
    do{
        ret = read(sockfd, (void *)(in_buff + (*position)), once_recv_len);
    }while(-1 == ret && EINTR == errno);
    if(0 <= ret )
    {
        (*position) += (ret);
        //sys_log(LOGSTDOUT, "csocket_irecv: end to recv data on sockfd %d: end: max len = %ld, position = %ld\n", sockfd, in_buff_max_len, *position);
        return (EC_TRUE);
    }
    sys_log(LOGSTDNULL, "warn:csocket_irecv: sockfd %d, errno = %d, errstr = %s, max len %ld, pos %ld\n",
                        sockfd, errno, strerror(errno), in_buff_max_len, (*position));
    return csocket_no_ierror();
}

EC_BOOL csocket_send(const int sockfd, const UINT8 *out_buff, const UINT32 out_buff_expect_len)
{
    UINT32 pos;

    pos = 0;

    while(pos < out_buff_expect_len)
    {
        if(EC_FALSE == csocket_is_connected(sockfd))
        {
            sys_log(LOGSTDOUT, "error:csocket_send: sockfd %d was broken\n", sockfd);
            return (EC_FALSE);
        }

        if(EC_FALSE == csocket_isend(sockfd, out_buff, out_buff_expect_len, &pos))
        {
            sys_log(LOGSTDOUT, "error:csocket_send: isend on sockfd %d failed where expect %ld, pos %ld\n",
                               sockfd, out_buff_expect_len, pos);
            return (EC_FALSE);
        }
        //sys_log(LOGSTDOUT, "[DEBUG] csocket_send: out_buff_expect_len %ld, pos %ld\n", out_buff_expect_len, pos);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_recv(const int sockfd, UINT8 *in_buff, const UINT32 in_buff_expect_len)
{
    UINT32 pos;

    pos = 0;

    while(pos < in_buff_expect_len)
    {
        if(EC_FALSE == csocket_is_connected(sockfd))
        {
            sys_log(LOGSTDOUT, "error:csocket_recv: sockfd %d was broken\n", sockfd);
            return (EC_FALSE);
        }

        if(EC_FALSE == csocket_irecv(sockfd, in_buff, in_buff_expect_len, &pos))
        {
            sys_log(LOGSTDOUT, "error:csocket_recv: irecv on sockfd %d failed where expect %ld, pos %ld\n",
                                sockfd, in_buff_expect_len, pos);
            return (EC_FALSE);
        }
        //sys_log(LOGSTDOUT, "[DEBUG] csocket_recv: in_buff_expect_len %ld, pos %ld\n", in_buff_expect_len, pos);
        //PRINT_BUFF("[DEBUG] csocket_recv: ", in_buff, pos);
    }
    return (EC_TRUE);
}

/*write until all data out or no further data can be sent out at present*/
EC_BOOL csocket_write(const int sockfd, const UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *pos)
{
    for(;;)
    {
        UINT32   once_sent_len;
        ssize_t  sent_num;

        once_sent_len = out_buff_max_len - (*pos);
        if(0 == once_sent_len)
        {
            return (EC_TRUE);
        }

        once_sent_len = DMIN(CSOCKET_SEND_ONCE_MAX_SIZE, once_sent_len);

        sent_num = write(sockfd, (void *)(out_buff + (*pos)), once_sent_len);
        if(0 > sent_num)
        {
            return csocket_no_ierror();
        }
        
        if(0 == sent_num )/*when ret = 0, no data was sent*/
        {
            return (EC_TRUE);
        }
        //sys_log(LOGSTDOUT, "[DEBUG] csocket_write: sent out %ld bytes\n", sent_num);
        (*pos) += (UINT32)sent_num;
    }
    return (EC_TRUE);
}
EC_BOOL csocket_write_0(const int sockfd, const UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *pos)
{
    for(;;)
    {
        UINT32   once_sent_len;
        ssize_t  sent_num;

        if(EC_FALSE == csocket_wait_for_io(sockfd, CSOCKET_WRITING_FLAG))
        {
            return (EC_TRUE);
        }

        once_sent_len = out_buff_max_len - (*pos);
        if(0 == once_sent_len)
        {
            break;
        }

        once_sent_len = DMIN(CSOCKET_SEND_ONCE_MAX_SIZE, once_sent_len);

        //sent_num = send(sockfd, (void *)(out_buff + (*pos)), once_sent_len, 0);
        sent_num = write(sockfd, (void *)(out_buff + (*pos)), once_sent_len);
        if(0 > sent_num)
        {
            return csocket_no_ierror();
        }
        
        if(0 == sent_num )/*when ret = 0, no data was sent*/
        {
            return (EC_TRUE);
        }
        //sys_log(LOGSTDOUT, "[DEBUG] csocket_write: sent out %ld bytes\n", sent_num);
        (*pos) += (UINT32)sent_num;
    }
    return (EC_TRUE);
}

/*read until all data ready or no further data to recv at present*/
EC_BOOL csocket_read(const int sockfd, UINT8 *in_buff, const UINT32 in_buff_expect_len, UINT32 *pos)
{
    for(;;)
    {
        UINT32 once_recv_len;
        ssize_t  recved_num;
        
        once_recv_len = in_buff_expect_len - (*pos);
        if(0 == once_recv_len)/*no free space to recv*/
        {
            return (EC_TRUE);
        }

        once_recv_len = DMIN(CSOCKET_RECV_ONCE_MAX_SIZE, once_recv_len);

        recved_num = read(sockfd, (void *)(in_buff + (*pos)), once_recv_len);
        if(0 > recved_num)
        {
            /*no data to recv or found error*/
            return csocket_no_ierror();
        }

        if(0 == recved_num)
        {
            return (EC_TRUE);
        }

        //sys_log(LOGSTDOUT, "[DEBUG] csocket_read: read in %ld bytes\n", recved_num);
        (*pos) += (UINT32)recved_num;
    }

    return (EC_TRUE);
}

EC_BOOL csocket_read_0(const int sockfd, UINT8 *in_buff, const UINT32 in_buff_expect_len, UINT32 *pos)
{
    for(;;)
    {
        UINT32 once_recv_len;
        ssize_t  recved_num;

        if(EC_FALSE == csocket_wait_for_io(sockfd, CSOCKET_READING_FLAG))
        {
            return (EC_TRUE);
        }        
        
        once_recv_len = in_buff_expect_len - (*pos);
        if(0 == once_recv_len)/*no free space to recv*/
        {
            break;
        }

        once_recv_len = DMIN(CSOCKET_RECV_ONCE_MAX_SIZE, once_recv_len);

        //recved_num = recv(sockfd, (void *)(in_buff + (*pos)), once_recv_len, 0);
        recved_num = read(sockfd, (void *)(in_buff + (*pos)), once_recv_len);
        if(0 > recved_num)
        {
            /*no data to recv or found error*/
            return csocket_no_ierror();
        }

        if(0 == recved_num)
        {
            return (EC_TRUE);
        }

        //sys_log(LOGSTDOUT, "[DEBUG] csocket_read: read in %ld bytes\n", recved_num);
        (*pos) += (UINT32)recved_num;
    }

    return (EC_TRUE);
}

EC_BOOL csocket_send_uint32(const int sockfd, const UINT32 num)
{
    UINT32 data;

    data = hton_uint32(num);
    return csocket_send(sockfd, (UINT8 *)&data, sizeof(data));
}

EC_BOOL csocket_recv_uint32(const int sockfd, UINT32 *num)
{
    UINT32 data;

    if(EC_TRUE == csocket_recv(sockfd, (UINT8 *)&data, sizeof(data)))
    {
        (*num) = ntoh_uint32(data);
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL csocket_send_uint8(const int sockfd, const UINT8 num)
{
    UINT8 data;

    data = (num);
    return csocket_send(sockfd, (UINT8 *)&data, sizeof(data));
}

EC_BOOL csocket_recv_uint8(const int sockfd, UINT8 *num)
{
    UINT8 data;

    if(EC_TRUE == csocket_recv(sockfd, (UINT8 *)&data, sizeof(data)))
    {
        (*num) = (data);
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL csocket_send_cstring(const int sockfd, const CSTRING *cstring)
{
    if(EC_FALSE == csocket_send_uint32(sockfd, cstring_get_len(cstring)))
    {
        sys_log(LOGSTDOUT, "error:csocket_send_cstring: send cstring len %ld failed\n", cstring_get_len(cstring));
        return (EC_FALSE);
    }
    return csocket_send(sockfd, cstring_get_str(cstring), cstring_get_len(cstring));
}

EC_BOOL csocket_recv_cstring(const int sockfd, CSTRING *cstring)
{
    UINT32 len;
    if(EC_FALSE == csocket_recv_uint32(sockfd, &len))
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cstring: recv cstring len failed\n");
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "[DEBUG] csocket_recv_cstring: len = %ld\n", len);

    if(EC_FALSE == cstring_expand_to(cstring, len))
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cstring: expand cstring to size %ld failed\n", len);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv(sockfd, cstring->str, len))
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cstring: recv cstring with len %ld failed\n", len);
        return (EC_FALSE);
    }

    cstring->len = len;/*fuck!*/
    return (EC_TRUE);
}

EC_BOOL csocket_send_cbytes(const int sockfd, const CBYTES *cbytes)
{
    if(EC_FALSE == csocket_send_uint32(sockfd, CBYTES_LEN(cbytes)))
    {
        sys_log(LOGSTDOUT, "error:csocket_send_cbytes: send cdfs buff len %ld\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_send(sockfd, CBYTES_BUF(cbytes), CBYTES_LEN(cbytes)))
    {
        sys_log(LOGSTDOUT, "error:csocket_send_cbytes: send cdfs buff with len %ld failed\n", CBYTES_LEN(cbytes));
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csocket_recv_cbytes(const int sockfd, CBYTES *cbytes)
{
    UINT32 len;
    UINT8 *data;

    if(EC_FALSE == csocket_recv_uint32(sockfd, &len))
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cbytes: recv data len failed\n");
        return (EC_FALSE);
    }

    if(0 == len)
    {
        return (EC_TRUE);
    }

    data = (UINT8 *)SAFE_MALLOC(len, LOC_CSOCKET_0006);
    if(NULL_PTR == data)
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cbytes: alloc %ld bytes failed\n", len);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv(sockfd, data, len))
    {
        sys_log(LOGSTDOUT, "error:csocket_recv_cbytes: recv %ld bytes\n", len);
        SAFE_FREE(data, LOC_CSOCKET_0007);
        return (EC_FALSE);
    }

    CBYTES_LEN(cbytes) = len;
    CBYTES_BUF(cbytes) = data;

    return (EC_TRUE);
}


/*to fix a incomplete task_node, when complete, return EC_TRUE, otherwise, return EC_FALSE yet*/
EC_BOOL csocket_fix_task_node(const int sockfd, TASK_NODE *task_node)
{
    csocket_read(sockfd, TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &(TASK_NODE_BUFF_POS(task_node)));
    
    /*when complete csocket_request, return EC_TRUE*/
    if(TASK_NODE_BUFF_LEN(task_node) == TASK_NODE_BUFF_POS(task_node))
    {
        return (EC_TRUE);
    }
    /*otherwise*/
    return (EC_FALSE);
}

UINT32 csocket_encode_actual_size()
{
    if(0 == g_tmp_encode_size)
    {
        TASK_BRD        *task_brd;

        task_brd = task_brd_default_get();
        /**
        *
        *   note: here we cannot call cmpi_encode_csocket_status_size to determine the actual encode size
        *   because xxx_size functions just give a estimated size but not the actual encoded size!
        *
        *   if make size = cmpi_encode_csocket_status_size(), the size may be greater than the crbuff_total_read_len,
        *   thus, terrible things happen: you would never fetch the data from crbuff until next data stream from remote
        *   reaches local and meet the size length ...
        *
        **/
        cmpi_encode_uint32_size(TASK_BRD_COMM(task_brd), (UINT32)0, &g_tmp_encode_size);/*len*/
        cmpi_encode_uint32_size(TASK_BRD_COMM(task_brd), (UINT32)0, &g_tmp_encode_size);/*tag*/
    }

    return (g_tmp_encode_size);
}

UINT32 xmod_node_encode_actual_size()
{
    if(0 == g_xmod_node_tmp_encode_size)
    {
        MOD_NODE  xmod_node_tmp;
        TASK_BRD *task_brd;

        task_brd = task_brd_default_get();
        /**
        *
        *   note: here we cannot call cmpi_encode_xmod_node_size to determine the actual encode size
        *   because xxx_size functions just give a estimated size but not the actual encoded size!
        *
        *   if make size = cmpi_encode_xmod_node_size(), the size may be greater than the crbuff_total_read_len,
        *   thus, terrible things happen: you would never fetch the data from crbuff until next data stream from remote
        *   reaches local and meet the size length ...
        *
        **/
        cmpi_encode_xmod_node(TASK_BRD_COMM(task_brd),
                            &xmod_node_tmp,
                            g_xmod_node_tmp_encode_buff,
                            sizeof(g_xmod_node_tmp_encode_buff)/sizeof(g_xmod_node_tmp_encode_buff[0]),
                            &g_xmod_node_tmp_encode_size);
    }

    return (g_xmod_node_tmp_encode_size);
}

/*fetch a complete or incomplete csocket_request, caller should check the result*/
TASK_NODE *csocket_fetch_task_node(CSOCKET_CNODE *csocket_cnode)
{
    UINT32    size;
    TASK_BRD *task_brd;

    task_brd = task_brd_default_get();

    size = csocket_encode_actual_size();

    if(CSOCKET_CNODE_PKT_POS(csocket_cnode) < size)
    {
        if(EC_FALSE == csocket_irecv(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                      CSOCKET_CNODE_PKT_HDR(csocket_cnode), 
                                      size, 
                                      &(CSOCKET_CNODE_PKT_POS(csocket_cnode)))
          )
        {
            sys_log(LOGSTDOUT, "error:csocket_fetch_task_node: csocket irecv failed on socket %d where pkt pos %ld\n", 
                                CSOCKET_CNODE_SOCKFD(csocket_cnode), CSOCKET_CNODE_PKT_POS(csocket_cnode));
            return (NULL_PTR);
        }
    }

    if(CSOCKET_CNODE_PKT_POS(csocket_cnode) >= size)
    {
        UINT32  pos;

        UINT32  len;
        UINT32  tag;
        UINT8  *out_buff;
        UINT32  out_size;

        TASK_NODE *task_node;

        out_buff = CSOCKET_CNODE_PKT_HDR(csocket_cnode);
        out_size = CSOCKET_CNODE_PKT_POS(csocket_cnode);

        pos = 0;
        cmpi_decode_uint32(TASK_BRD_COMM(task_brd), out_buff, out_size, &pos, &len);
        cmpi_decode_uint32(TASK_BRD_COMM(task_brd), out_buff, out_size, &pos, &tag);

        //sys_log(LOGSTDOUT, "[DEBUG] csocket_fetch_task_node: len = %ld, tag = %ld\n", len, tag);
        //PRINT_BUFF("[DEBUG] csocket_fetch_task_node: ", out_buff, out_size);
#if 0
        if(len > 0x10000)/*debug!*/
        {
            exit(0);
        }
#endif
        task_node = task_node_new(len, LOC_CSOCKET_0008);
        if(NULL_PTR == task_node)
        {
            sys_log(LOGSTDOUT, "error:csocket_fetch_task_node: new task_node with %ld bytes failed\n", len);
            return (NULL_PTR);
        }

        TASK_NODE_TAG(task_node) = tag;

        /*move the probed buff to task_node*/
        BCOPY(out_buff, TASK_NODE_BUFF(task_node), out_size);
        TASK_NODE_BUFF_POS(task_node)        = out_size;
        CSOCKET_CNODE_PKT_POS(csocket_cnode) = 0;

        csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &(TASK_NODE_BUFF_POS(task_node)));

        return (task_node);
    }
    return (NULL_PTR);
}

EC_BOOL csocket_isend_task_node(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    if(EC_FALSE == CSOCKET_CNODE_IS_CONNECTED(csocket_cnode))
    {
        sys_log(LOGSTDERR, "error:csocket_request_isend: sockfd %d is disconnected\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }
#if 0
    sys_log(LOGSTDOUT, "[DEBUG]csocket_isend_task_node: before isend, task_node %lx, buff %lx, pos %ld, len %ld\n", task_node,
                    TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_POS(task_node), TASK_NODE_BUFF_LEN(task_node));
    PRINT_BUFF("[DEBUG]csocket_isend_task_node:will send: ", TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node));
#endif
    if(EC_FALSE == csocket_write(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &(TASK_NODE_BUFF_POS(task_node))))
    {
        sys_log(LOGSTDERR, "error:csocket_request_isend: sockfd %d isend failed\n", CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }
#if 0
    sys_log(LOGSTDOUT, "[DEBUG]csocket_isend_task_node: after isend, task_node %lx, buff %lx, pos %ld, len %ld\n", task_node,
                    TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_POS(task_node), TASK_NODE_BUFF_LEN(task_node));
    PRINT_BUFF("[DEBUG]csocket_isend_task_node:was sent: ", TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_POS(task_node));
#endif
    return (EC_TRUE);
}

EC_BOOL csocket_irecv_task_node(CSOCKET_CNODE *csocket_cnode, TASK_NODE *task_node)
{
    if(EC_FALSE == csocket_is_connected(CSOCKET_CNODE_SOCKFD(csocket_cnode)))
    {
        sys_log(LOGSTDERR, "error:csocket_request_irecv: tcid %s sockfd %d is disconnected\n",
                            TASK_NODE_RECV_TCID_STR(task_node),
                            CSOCKET_CNODE_SOCKFD(csocket_cnode));
        return (EC_FALSE);
    }

    /*debug*/
    //csocket_is_nonblock(CSOCKET_CNODE_SOCKFD(csocket_cnode));

    if(EC_FALSE == csocket_read(CSOCKET_CNODE_SOCKFD(csocket_cnode), 
                                  TASK_NODE_BUFF(task_node), TASK_NODE_BUFF_LEN(task_node), &(TASK_NODE_BUFF_POS(task_node))))
    {
        sys_log(LOGSTDERR, "error:csocket_request_irecv: tcid %s sockfd %d irecv failed where data addr = %lx, len = %ld, pos = %ld\n",
                        TASK_NODE_RECV_TCID_STR(task_node),
                        CSOCKET_CNODE_SOCKFD(csocket_cnode),
                        TASK_NODE_BUFF(task_node),
                        TASK_NODE_BUFF_LEN(task_node),
                        TASK_NODE_BUFF_POS(task_node));
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_discard_task_node_from(CLIST *clist, const UINT32 broken_tcid)
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CSOCKET_0009);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        if(broken_tcid == TASK_NODE_SEND_TCID(task_node))
        {
            CLIST_DATA *clist_data_rmv;

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);
            clist_rmv_no_lock(clist, clist_data_rmv);

            sys_log(LOGSTDOUT, "lost node: from broken tcid %s\n", TASK_NODE_SEND_TCID_STR(task_node));

            task_node_free(task_node);
        }
    }
    CLIST_UNLOCK(clist, LOC_CSOCKET_0010);
    return (EC_TRUE);
}

EC_BOOL csocket_discard_task_node_to(CLIST *clist, const UINT32 broken_tcid)
{
    CLIST_DATA *clist_data;

    CLIST_LOCK(clist, LOC_CSOCKET_0011);
    CLIST_LOOP_NEXT(clist, clist_data)
    {
        TASK_NODE *task_node;

        task_node = (TASK_NODE *)CLIST_DATA_DATA(clist_data);

        /*check tasks_work_isend_request function, here should be CSOCKET_STATUS_RECV_TCID*/
        if(broken_tcid == TASK_NODE_RECV_TCID(task_node)/*CSOCKET_STATUS_TCID(csocket_status)*/)
        {
            CLIST_DATA *clist_data_rmv;

            clist_data_rmv = clist_data;
            clist_data = CLIST_DATA_PREV(clist_data);
            clist_rmv_no_lock(clist, clist_data_rmv);

            sys_log(LOGSTDOUT, "lost node: to broken tcid %s\n", TASK_NODE_RECV_TCID_STR(task_node));

            task_node_free(task_node);
        }
    }
    CLIST_UNLOCK(clist, LOC_CSOCKET_0012);
    return (EC_TRUE);
}

EC_BOOL csocket_srv_start( const UINT32 srv_port, const UINT32 csocket_block_mode, int *srv_sockfd )
{
    if(EC_FALSE == csocket_listen(srv_port, srv_sockfd))
    {
        sys_log(LOGSTDERR, "error:csocket_srv_start: failed to listen on port %ld\n",srv_port);
        return (EC_FALSE);
    }

    sys_log(LOGSTDOUT, "csocket_srv_start: start server %d on port %ld\n", (*srv_sockfd), srv_port);

    if(CSOCKET_IS_NONBLOCK_MODE == csocket_block_mode && EC_FALSE == csocket_is_nonblock(*srv_sockfd))
    {
        csocket_nonblock_enable(*srv_sockfd);
    }

    if(CSOCKET_IS_BLOCK_MODE == csocket_block_mode && EC_TRUE == csocket_is_nonblock(*srv_sockfd))
    {
        csocket_nonblock_disable(*srv_sockfd);
    }
    return (EC_TRUE);
}

EC_BOOL csocket_srv_end(const int srv_sockfd)
{
    sys_log(LOGSTDOUT, "csocket_srv_end: stop server %d\n", srv_sockfd);
    csocket_close(srv_sockfd);

    return (EC_TRUE);
}

EC_BOOL csocket_client_start( const UINT32 srv_ipaddr, const UINT32 srv_port, const UINT32 csocket_block_mode, int *client_sockfd )
{
    if(EC_FALSE == csocket_connect( srv_ipaddr, srv_port , csocket_block_mode, client_sockfd ))
    {
        sys_log(LOGSTDNULL, "error:csocket_client_start: client failed to connect server %s:%ld\n",
                            c_word_to_ipv4(srv_ipaddr), srv_port);
        return (EC_FALSE);
    }

    sys_log(LOGSTDNULL, "csocket_client_start: start client %d connecting to server %s:%ld\n",
                        (*client_sockfd), c_word_to_ipv4(srv_ipaddr), srv_port);
    return (EC_TRUE);
}

EC_BOOL csocket_client_end(const int client_sockfd)
{
    csocket_close(client_sockfd);
    sys_log(LOGSTDOUT, "csocket_client_end: stop client %d\n", client_sockfd);
    return (EC_TRUE);
}

static EC_BOOL __csocket_task_req_func_encode_size(const struct _TASK_FUNC *task_req_func, UINT32 *size)
{
    FUNC_ADDR_NODE *func_addr_node;

    UINT32 send_comm;

    /*clear size*/
    *size = 0;

    send_comm = CMPI_COMM_WORLD;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_encode_size: failed to fetch func addr node by func id %lx\n",
                            task_req_func->func_id);
        return (EC_FALSE);
    }

    cmpi_encode_uint32_size(send_comm, (task_req_func->func_id), size);
    cmpi_encode_uint32_size(send_comm, (task_req_func->func_para_num), size);

    if(EC_FALSE == task_req_func_para_encode_size(send_comm,
                                                  func_addr_node->func_para_num,
                                                  (FUNC_PARA *)task_req_func->func_para,
                                                  func_addr_node,
                                                  size))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_encode_size: encode size of req func paras failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

static EC_BOOL __csocket_task_req_func_encode(const struct _TASK_FUNC *task_req_func, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len)
{
    FUNC_ADDR_NODE *func_addr_node;

    UINT32 send_comm;

    UINT32  position;

    send_comm = CMPI_COMM_WORLD;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_encode: failed to fetch func addr node by func id %lx\n",
                            task_req_func->func_id);
        return (EC_FALSE);
    }

    cmpi_encode_uint32(send_comm, (task_req_func->func_id), out_buff, out_buff_max_len, &(position));
    cmpi_encode_uint32(send_comm, (task_req_func->func_para_num), out_buff, out_buff_max_len, &(position));

    if(EC_FALSE == task_req_func_para_encode(send_comm,
                                              task_req_func->func_para_num,
                                              (FUNC_PARA *)task_req_func->func_para,
                                              func_addr_node,
                                              out_buff,
                                              out_buff_max_len,
                                              &(position)))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_encode: encode req func para failed\n");
        return (EC_FALSE);
    }

    (*out_buff_len) = position;/*set to real length*/

    return (EC_TRUE);
}

static EC_BOOL __csocket_task_req_func_decode(const UINT8 *in_buff, const UINT32 in_buff_len, struct _TASK_FUNC *task_req_func)
{
    UINT32 recv_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 position;

    recv_comm = CMPI_COMM_WORLD;

    position = 0;

    cmpi_decode_uint32(recv_comm, in_buff, in_buff_len, &(position), &(task_req_func->func_id));
    cmpi_decode_uint32(recv_comm, in_buff, in_buff_len, &(position), &(task_req_func->func_para_num));

    //sys_log(LOGSTDOUT, "[DEBUG] __csocket_task_req_func_decode: func id %lx\n", task_req_func->func_id);
    //sys_log(LOGSTDOUT, "[DEBUG] __csocket_task_req_func_decode: func para num %ld\n", task_req_func->func_para_num);

    if(0 != dbg_fetch_func_addr_node_by_index(task_req_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_decode: failed to fetch func addr node by func id %lx\n", task_req_func->func_id);
        return (EC_FALSE);
    }

    type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
    if( NULL_PTR == type_conv_item )
    {
        sys_log(LOGSTDOUT,"error:__csocket_task_req_func_decode: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
        return (EC_FALSE);
    }
    if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item))
    {
        alloc_static_mem(MD_TASK, 0, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void **)&(task_req_func->func_ret_val), LOC_CSOCKET_0013);
        dbg_tiny_caller(2, TYPE_CONV_ITEM_VAR_INIT_FUNC(type_conv_item), CMPI_ANY_MODI, task_req_func->func_ret_val);
    }

    if(EC_FALSE == task_req_func_para_decode(recv_comm,
                                             in_buff,
                                             in_buff_len,
                                             &(position),
                                             &(task_req_func->func_para_num),
                                             (FUNC_PARA *)task_req_func->func_para,
                                             func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_req_func_decode: decode func paras failed\n");

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_req_func->func_ret_val)
        {
            free_static_mem(MD_TASK, 0, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)(task_req_func->func_ret_val), LOC_CSOCKET_0014);
        }
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL __csocket_task_rsp_func_encode_size(const struct _TASK_FUNC *task_rsp_func, UINT32 *size)
{
    UINT32 send_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    send_comm = CMPI_COMM_WORLD;

    *size = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_encode_size: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:__csocket_task_rsp_func_encode_size: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }
        dbg_tiny_caller(3,
            TYPE_CONV_ITEM_VAR_ENCODE_SIZE(type_conv_item),
            send_comm,
            task_rsp_func->func_ret_val,
            size);
    }

    if(EC_FALSE == task_rsp_func_para_encode_size(send_comm,
                                                  task_rsp_func->func_para_num,
                                                  (FUNC_PARA *)task_rsp_func->func_para,
                                                  func_addr_node,
                                                  size))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_encode_size: encode size of rsp func failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL __csocket_task_rsp_func_encode(struct _TASK_FUNC *task_rsp_func, UINT8 *out_buff, const UINT32 out_buff_max_len, UINT32 *out_buff_len)
{
    UINT32 send_comm;

    FUNC_ADDR_NODE *func_addr_node;
    TYPE_CONV_ITEM *type_conv_item;

    UINT32 position;

    send_comm = CMPI_COMM_WORLD;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_encode: failed to fetch func addr node by func id %lx\n", task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:__csocket_task_rsp_func_encode: ret type %ld conv item is not defined\n", func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        dbg_tiny_caller(5,
            TYPE_CONV_ITEM_VAR_ENCODE_FUNC(type_conv_item),
            send_comm,
            task_rsp_func->func_ret_val,
            out_buff,
            out_buff_max_len,
            &(position));

        if(EC_TRUE == TYPE_CONV_ITEM_VAR_POINTER_FLAG(type_conv_item) && 0 != task_rsp_func->func_ret_val)
        {
            dbg_tiny_caller(2, TYPE_CONV_ITEM_VAR_CLEAN_FUNC(type_conv_item), CMPI_ANY_MODI, task_rsp_func->func_ret_val);/*WARNING: SHOULD NOT BE 0*/
            free_static_mem(MD_TASK, CMPI_ANY_MODI, TYPE_CONV_ITEM_VAR_MM_TYPE(type_conv_item), (void *)task_rsp_func->func_ret_val, LOC_CSOCKET_0015);/*clean up*/
            task_rsp_func->func_ret_val = 0;
        }
    }
    //PRINT_BUFF("[DEBUG] __csocket_task_rsp_func_encode[3]: send rsp buff", out_buff, position);
    if(EC_FALSE == task_rsp_func_para_encode(send_comm,
                                              task_rsp_func->func_para_num,
                                              (FUNC_PARA *)task_rsp_func->func_para,
                                              func_addr_node,
                                              out_buff,
                                              out_buff_max_len,
                                              &(position)))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_encode: encode rsp func paras failed\n");
        return (EC_FALSE);
    }
    //PRINT_BUFF("[DEBUG] __csocket_task_rsp_func_encode[4]: send rsp buff", out_buff, position);

    (*out_buff_len) = position;/*set to real length*/
    return (EC_TRUE);
}

EC_BOOL __csocket_task_rsp_func_decode(const UINT8 *in_buff, const UINT32 in_buff_len, struct _TASK_FUNC *task_rsp_func)
{
    UINT32 recv_comm;

    FUNC_ADDR_NODE *func_addr_node;

    TYPE_CONV_ITEM *type_conv_item;

    UINT32  position;

    recv_comm = CMPI_COMM_WORLD;

    position = 0;

    if(0 != dbg_fetch_func_addr_node_by_index(task_rsp_func->func_id, &func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_decode: failed to fetch func addr node by func id %lx\n",
                            task_rsp_func->func_id);
        return (EC_FALSE);
    }

    if(e_dbg_void != func_addr_node->func_ret_type)
    {
        if(0 == task_rsp_func->func_ret_val)
        {
            sys_log(LOGSTDOUT, "error:__csocket_task_rsp_func_decode: func id %lx func_ret_val should not be null\n",
                                task_rsp_func->func_id);
            return (EC_FALSE);
        }

        type_conv_item = dbg_query_type_conv_item_by_type(func_addr_node->func_ret_type);
        if( NULL_PTR == type_conv_item )
        {
            sys_log(LOGSTDOUT,"error:__csocket_task_rsp_func_decode: ret type %ld conv item is not defined\n",
                                func_addr_node->func_ret_type);
            return (EC_FALSE);
        }

        dbg_tiny_caller(5,
                TYPE_CONV_ITEM_VAR_DECODE_FUNC(type_conv_item),
                recv_comm,
                in_buff,
                in_buff_len,
                &(position),
                task_rsp_func->func_ret_val);
    }

    if(EC_FALSE == task_rsp_func_para_decode(recv_comm, in_buff, in_buff_len, &(position),
                              task_rsp_func->func_para_num,
                              (FUNC_PARA *)task_rsp_func->func_para,
                              func_addr_node))
    {
        sys_log(LOGSTDOUT, "error:cextsrv_rsp_decode: decode rsp func paras failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL csocket_task_req_func_send(const int sockfd, struct _TASK_FUNC *task_req_func)
{
    UINT32 size;

    UINT8 *out_buff;
    UINT32 out_buff_len;

    /*encode size task_req_func*/
    if(EC_FALSE == __csocket_task_req_func_encode_size(task_req_func, &size))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_send: encode size failed\n");
        return (EC_FALSE);
    }

    /*encode task_req_func*/
    out_buff = (UINT8 *)SAFE_MALLOC(size, LOC_CSOCKET_0016);
    if(NULL_PTR == out_buff)
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_send: alloc %ld bytes failed\n", size);
        return (EC_FALSE);
    }

    if(EC_FALSE == __csocket_task_req_func_encode(task_req_func, out_buff, size, &out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_send: encode failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0017);
        return (EC_FALSE);
    }

    /*send task_req_func*/
    if(EC_FALSE == csocket_send_uint32(sockfd, out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_send: send buff len %ld failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0018);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_send(sockfd, out_buff, out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_send: send %ld bytes failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0019);
        return (EC_FALSE);
    }

    SAFE_FREE(out_buff, LOC_CSOCKET_0020);
    return (EC_TRUE);
}

EC_BOOL csocket_task_req_func_recv(const int sockfd, struct _TASK_FUNC *task_req_func)
{
    UINT8 *in_buff;
    UINT32 in_buff_len;

    /*recv task_req_func*/
    if(EC_FALSE == csocket_recv_uint32(sockfd, &in_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_recv: recv len failed\n");
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "[DEBUG]csocket_task_req_func_recv: in_buff_len = %ld\n", in_buff_len);

    in_buff = (UINT8 *)SAFE_MALLOC(in_buff_len, LOC_CSOCKET_0021);
    if(NULL_PTR == in_buff)
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_recv: alloc %ld bytes failed\n", in_buff_len);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv(sockfd, in_buff, in_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_recv: recv %ld bytes failed\n");
        SAFE_FREE(in_buff, LOC_CSOCKET_0022);
        return (EC_FALSE);
    }

    //PRINT_BUFF("[DEBUG]csocket_task_req_func_recv: recv buff:\n", in_buff, in_buff_len);

    if(EC_FALSE == __csocket_task_req_func_decode(in_buff, in_buff_len, task_req_func))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_req_func_recv: encoding failed\n");
        SAFE_FREE(in_buff, LOC_CSOCKET_0023);
        return (EC_FALSE);
    }

    SAFE_FREE(in_buff, LOC_CSOCKET_0024);
    return (EC_TRUE);
}

EC_BOOL csocket_task_rsp_func_send(const int sockfd, struct _TASK_FUNC *task_rsp_func)
{
    UINT32 size;

    UINT8 *out_buff;
    UINT32 out_buff_len;

    /*encode size task_rsp_func*/
    if(EC_FALSE == __csocket_task_rsp_func_encode_size(task_rsp_func, &size))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_send: encode size failed\n");
        return (EC_FALSE);
    }

    /*encode task_rsp_func*/
    out_buff = (UINT8 *)SAFE_MALLOC(size, LOC_CSOCKET_0025);
    if(NULL_PTR == out_buff)
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_send: alloc %ld bytes failed\n", size);
        return (EC_FALSE);
    }

    if(EC_FALSE == __csocket_task_rsp_func_encode(task_rsp_func, out_buff, size, &out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_send: encode failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0026);
        return (EC_FALSE);
    }

    //PRINT_BUFF("[DEBUG] csocket_task_rsp_func_send[1]: send rsp buff", out_buff, out_buff_len);

    /*send task_rsp_func*/
    if(EC_FALSE == csocket_send_uint32(sockfd, out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_send: send len %ld failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0027);
        return (EC_FALSE);
    }

    //sys_log(LOGSTDOUT, "[DEBUG] csocket_task_rsp_func_send: send rsp len %ld, size %ld\n", out_buff_len, size);

    if(EC_FALSE == csocket_send(sockfd, out_buff, out_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_send: send %ld bytes failed\n");
        SAFE_FREE(out_buff, LOC_CSOCKET_0028);
        return (EC_FALSE);
    }

    //PRINT_BUFF("[DEBUG] csocket_task_rsp_func_send[2]: send rsp buff", out_buff, out_buff_len);

    SAFE_FREE(out_buff, LOC_CSOCKET_0029);
    return (EC_TRUE);
}

EC_BOOL csocket_task_rsp_func_recv(const int sockfd, struct _TASK_FUNC *task_rsp_func)
{
    UINT8 *in_buff;
    UINT32 in_buff_len;

    /*recv task_rsp_func*/
    if(EC_FALSE == csocket_recv_uint32(sockfd, &in_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_recv: recv len failed\n");
        return (EC_FALSE);
    }

    in_buff = (UINT8 *)SAFE_MALLOC(in_buff_len, LOC_CSOCKET_0030);
    if(NULL_PTR == in_buff)
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_recv: alloc %ld bytes failed\n", in_buff_len);
        return (EC_FALSE);
    }

    if(EC_FALSE == csocket_recv(sockfd, in_buff, in_buff_len))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_recv: recv %ld bytes failed\n");
        SAFE_FREE(in_buff, LOC_CSOCKET_0031);
        return (EC_FALSE);
    }

    if(EC_FALSE == __csocket_task_rsp_func_decode(in_buff, in_buff_len, task_rsp_func))
    {
        sys_log(LOGSTDOUT, "error:csocket_task_rsp_func_recv: encode failed\n");
        SAFE_FREE(in_buff, LOC_CSOCKET_0032);
        return (EC_FALSE);
    }

    SAFE_FREE(in_buff, LOC_CSOCKET_0033);
    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

