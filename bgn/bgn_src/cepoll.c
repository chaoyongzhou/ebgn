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
#include <errno.h>

#include <stdarg.h>

#include <sys/epoll.h>
#include <errno.h>

#include "type.h"
#include "log.h"

#include "cxml.h"
#include "task.h"
#include "csocket.h"
#include "task.inc"

#include "cmpic.inc"
#include "cmpie.h"
#include "cmisc.h"

#include "cepoll.h"
#include "csocket.h"
#include "taskcfg.inc"
#include "tasks.h"

#define CEPOLL_RD_EVENT_CHAR(events)        (((events) & CEPOLL_RD_EVENT) ? 'R':'-')
#define CEPOLL_WR_EVENT_CHAR(events)        (((events) & CEPOLL_WR_EVENT) ? 'W':'-')
#define CEPOLL_ALL_EVENT_CHARS(events)      CEPOLL_RD_EVENT_CHAR(events), CEPOLL_WR_EVENT_CHAR(events)

CEPOLL *cepoll_new(const int epoll_max_event_num)
{
    CEPOLL *cepoll;

    alloc_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, &cepoll, LOC_CEPOLL_0001);
    if(NULL_PTR == cepoll)
    {
        sys_log(LOGSTDOUT, "error:cepoll_new: new cepoll failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cepoll_init(cepoll, epoll_max_event_num))
    {
        sys_log(LOGSTDOUT, "error:cepoll_new: init cepoll with max event num %d failed\n", epoll_max_event_num);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, cepoll, LOC_CEPOLL_0002);
        return (NULL_PTR);
    }
    return (cepoll);
}

EC_BOOL cepoll_init(CEPOLL *cepoll, const int epoll_max_event_num)
{
    CEPOLL_EVENT  *epoll_event_tab;
    CEPOLL_NODE   *epoll_node_tab;
    UINT32 size;
    int epoll_fd;
    
    size = ((UINT32)1) * sizeof(CEPOLL_EVENT) * epoll_max_event_num;
    epoll_event_tab = (CEPOLL_EVENT *)safe_malloc(size, LOC_CEPOLL_0003);
    if(NULL_PTR == epoll_event_tab)
    {
        sys_log(LOGSTDOUT, "error:cepoll_init: malloc %d cepoll events failed\n", epoll_max_event_num);
        return (EC_FALSE);        
    }

    size = ((UINT32)1) * sizeof(CEPOLL_NODE) * CEPOLL_MAX_FD_NUM;
    epoll_node_tab = (CEPOLL_NODE *)safe_calloc(size, LOC_CEPOLL_0004);
    if(NULL_PTR == epoll_node_tab)
    {
        safe_free(epoll_event_tab, LOC_CEPOLL_0005);
        sys_log(LOGSTDOUT, "error:cepoll_init: malloc %d cepoll nodes failed\n", CEPOLL_MAX_FD_NUM);
        return (EC_FALSE);        
    }

    epoll_fd = epoll_create(epoll_max_event_num);
    if(0 > epoll_fd)
    {
        safe_free(epoll_event_tab, LOC_CEPOLL_0006);
        safe_free(epoll_node_tab, LOC_CEPOLL_0007);
        sys_log(LOGSTDOUT, "error:cepoll_init: create epoll with max event num %d failed, errno = %d, errstr = %s\n", 
                           epoll_max_event_num, errno, strerror(errno));
        return (EC_FALSE);
    }

    CEPOLL_FD(cepoll)        = epoll_fd;
    CEPOLL_EVENT_NUM(cepoll) = epoll_max_event_num;
    CEPOLL_EVENT_TAB(cepoll) = epoll_event_tab;
    CEPOLL_NODE_TAB(cepoll)  = epoll_node_tab;
    
    return (EC_TRUE);
}

EC_BOOL cepoll_clean(CEPOLL *cepoll)
{
    if(ERR_FD != CEPOLL_FD(cepoll))
    {
        close(CEPOLL_FD(cepoll));
        CEPOLL_FD(cepoll) = ERR_FD;
    }

    if(NULL_PTR != CEPOLL_EVENT_TAB(cepoll))
    {
        safe_free(CEPOLL_EVENT_TAB(cepoll), LOC_CEPOLL_0008);
        CEPOLL_EVENT_TAB(cepoll) = NULL_PTR;
    }

    if(NULL_PTR != CEPOLL_NODE_TAB(cepoll))
    {
        safe_free(CEPOLL_NODE_TAB(cepoll), LOC_CEPOLL_0009);
        CEPOLL_NODE_TAB(cepoll) = NULL_PTR;
    }    

    CEPOLL_EVENT_NUM(cepoll) = 0;
    
    return (EC_TRUE);
}

EC_BOOL cepoll_free(CEPOLL *cepoll)
{
    if(NULL_PTR != cepoll)
    {
        cepoll_clean(cepoll);
        free_static_mem(MD_TASK, CMPI_ANY_MODI, MM_CEPOLL, cepoll, LOC_CEPOLL_0010);
    }
    return (EC_TRUE);
}

static CEPOLL_EVENT *cepoll_fetch_event(const CEPOLL *cepoll, const int pos)
{
    if(0 > pos || pos >= CEPOLL_EVENT_NUM(cepoll))
    {
        return (NULL_PTR);
    }

    return CEPOLL_FETCH_EVENT(cepoll, pos);
}

static CEPOLL_NODE *cepoll_fetch_node(const CEPOLL *cepoll, const int fd)
{
    if(0 > fd || fd >= CEPOLL_MAX_FD_NUM)
    {
        return (NULL_PTR);
    }

    return CEPOLL_FETCH_NODE(cepoll, fd);
}

EC_BOOL cepoll_add(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{ 
	CEPOLL_EVENT cepoll_event;

    CEPOLL_EVENT_TYPE(&cepoll_event) = events;
    CEPOLL_EVENT_FD(&cepoll_event)   = sockfd;
    
	if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_ADD, sockfd, &cepoll_event)) 
	{
	    sys_log(LOGSTDOUT, "error:cepoll_add: EPOLL_CTL_ADD failed, sockfd %d, errno = %d, errstr = %s\n", 
	                        sockfd, errno, strerror(errno));
		return (EC_FALSE);
	}

	return (EC_TRUE);
}

EC_BOOL cepoll_del(CEPOLL *cepoll, const int sockfd)
{
	if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_DEL, sockfd, NULL_PTR))
	{
	    sys_log(LOGSTDOUT, "error:cepoll_del: EPOLL_CTL_DEL failed, sockfd %d, errno = %d, errstr = %s\n", 
	                        sockfd, errno, strerror(errno));
		return (EC_FALSE);
	}

	return (EC_TRUE);
}

EC_BOOL cepoll_mod(CEPOLL *cepoll, const int sockfd, const uint32_t events)
{ 
	CEPOLL_EVENT cepoll_event;

    CEPOLL_EVENT_TYPE(&cepoll_event) = events;
    CEPOLL_EVENT_FD(&cepoll_event)   = sockfd;
    
	if(0 != epoll_ctl(CEPOLL_FD(cepoll), EPOLL_CTL_MOD, sockfd, &cepoll_event))
	{
	    sys_log(LOGSTDOUT, "error:cepoll_mod: EPOLL_CTL_MOD failed, sockfd %d, errno = %d, errstr = %s\n", 
	                        sockfd, errno, strerror(errno));
		return (EC_FALSE);
	}

	return (EC_TRUE);
}

EC_BOOL cepoll_add_client_event(CEPOLL *cepoll, const uint32_t events, CSOCKET_CNODE *csocket_cnode)
{
    CEPOLL_NODE *cepoll_node;
    uint32_t events_t;
    int sockfd;

    sockfd = CSOCKET_CNODE_SOCKFD(csocket_cnode);

    sys_log(LOGSTDOUT, "[DEBUG] cepoll_add_client_event: sockfd %d try to add events %c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events));

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        sys_log(LOGSTDOUT, "error:cepoll_add_client_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    events_t = 0;    
    if(CEPOLL_RD_EVENT & events)
    {   
        events_t |= (CEPOLL_IN /*| CEPOLL_ET*/);

        if(NULL_PTR != CEPOLL_NODE_RD_ARG(cepoll_node))
        {
            sys_log(LOGSTDOUT, "error:cepoll_add_client_event:sockfd %d has set CEPOLL_NODE_RD_ARG %p\n", 
                                sockfd, CEPOLL_NODE_RD_ARG(cepoll_node));
            return (EC_FALSE);
        }    
        CEPOLL_NODE_RD_ARG(cepoll_node) = (void *)csocket_cnode;
    }
    
    if(CEPOLL_WR_EVENT & events)
    {   
        events_t |= (CEPOLL_OUT/* | CEPOLL_ET*/);

        if(NULL_PTR != CEPOLL_NODE_WR_ARG(cepoll_node))
        {
            sys_log(LOGSTDOUT, "error:cepoll_add_client_event:sockfd %d has set CEPOLL_NODE_WR_ARG %p\n", 
                                sockfd, CEPOLL_NODE_WR_ARG(cepoll_node));
            return (EC_FALSE);
        }    
        CEPOLL_NODE_WR_ARG(cepoll_node) = (void *)csocket_cnode;
    }
    
    if(0 == events_t)
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:cepoll_add_client_event: sockfd %d refuse to add invalid events %c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events));
        return (EC_FALSE);
    }

    CEPOLL_NODE_TYPE(cepoll_node) = CEPOLL_NODE_IS_CLIENT;

    sys_log(LOGSTDOUT, "[DEBUG] cepoll_add_client_event: sockfd %d add events_t %c%c\n", sockfd, CEPOLL_ALL_EVENT_CHARS(events_t));
    return cepoll_add(cepoll, sockfd, events_t);
}

EC_BOOL cepoll_add_srv_event(CEPOLL *cepoll, const uint32_t events, TASKS_CFG *tasks_cfg)
{
    CEPOLL_NODE *cepoll_node;
    
    uint32_t events_t;
    int sockfd;

    sockfd = TASKS_CFG_SRVSOCKFD(tasks_cfg);

    sys_log(LOGSTDOUT, "[DEBUG] cepoll_add_srv_event: sockfd %d try to add events %c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events));

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        sys_log(LOGSTDOUT, "error:cepoll_add_srv_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    events_t = 0;    
    if(CEPOLL_RD_EVENT & events)
    {   
        events_t |= (CEPOLL_IN /*| CEPOLL_ET*/);

        CEPOLL_NODE_RD_ARG(cepoll_node) = (void *)tasks_cfg;
    }
    
    if(CEPOLL_WR_EVENT & events)
    {
        sys_log(LOGSTDOUT, "error:cepoll_add_srv_event:sockfd %d should never reach here\n", sockfd);
        return (EC_FALSE);
    }
    
    if(0 == events_t)
    {
        sys_log(LOGSTDOUT, "[DEBUG] error:cepoll_add_srv_event: sockfd %d refuse to add invalid events %c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events));
        return (EC_FALSE);
    }

    CEPOLL_NODE_TYPE(cepoll_node) = CEPOLL_NODE_IS_SERVER;

    sys_log(LOGSTDOUT, "[DEBUG] cepoll_add_srv_event: sockfd %d add events_t %c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_t));
    return cepoll_add(cepoll, sockfd, events_t);
}

EC_BOOL cepoll_mod_client_event(CEPOLL *cepoll, const uint32_t events, CSOCKET_CNODE *csocket_cnode)
{
    uint32_t events_des;
    uint32_t events_src;
    int      sockfd;

    CEPOLL_NODE *cepoll_node;

    sockfd = CSOCKET_CNODE_SOCKFD(csocket_cnode);

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        sys_log(LOGSTDOUT, "error:cepoll_mod_client_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    events_src = 0;
    events_des = 0;
    if(CEPOLL_RD_EVENT & events)
    {   
        events_des |= (CEPOLL_IN/* | CEPOLL_ET*/);

        if(csocket_cnode != CEPOLL_NODE_RD_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_IN/* | CEPOLL_ET*/);
            CEPOLL_NODE_RD_ARG(cepoll_node) = (void *)csocket_cnode;
        }
        else
        {
            sys_log(LOGSTDOUT, "info:cepoll_mod_client_event:sockfd %d has set CEPOLL_NODE_RD_ARG %p\n", 
                                sockfd, CEPOLL_NODE_RD_ARG(cepoll_node));
        }    
    }
    else
    {
        if(NULL_PTR != CEPOLL_NODE_RD_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_IN/* | CEPOLL_ET*/);
            CEPOLL_NODE_RD_ARG(cepoll_node) = NULL_PTR;
        }
    }

    if(CEPOLL_WR_EVENT & events)
    {   
        events_des |= (CEPOLL_OUT/* | CEPOLL_ET*/);

        if(csocket_cnode != CEPOLL_NODE_WR_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_OUT/* | CEPOLL_ET*/);
            CEPOLL_NODE_WR_ARG(cepoll_node) = (void *)csocket_cnode;
        }
        else
        {
            sys_log(LOGSTDOUT, "info:cepoll_mod_client_event:sockfd %d has set CEPOLL_NODE_WR_ARG %p\n", 
                                sockfd, CEPOLL_NODE_WR_ARG(cepoll_node));
        }    
    }
    else
    {
        if(NULL_PTR != CEPOLL_NODE_WR_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_OUT/* | CEPOLL_ET*/);
            CEPOLL_NODE_WR_ARG(cepoll_node) = NULL_PTR;
        }
    }    

    if(events_des == events_src)
    {
        sys_log(LOGSTDOUT, "[DEBUG] cepoll_mod_client_event: ignore sockfd %d modification epoll event to %c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_des));    
        return (EC_TRUE);                            
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] cepoll_mod_client_event: sockfd %d modify epoll event to %c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_des));
    
    return cepoll_mod(cepoll, sockfd, events_des);
}

EC_BOOL cepoll_mod_srv_event(CEPOLL *cepoll, const uint32_t events, TASKS_CFG *tasks_cfg)
{
    uint32_t events_des;
    uint32_t events_src;
    int      sockfd;

    CEPOLL_NODE *cepoll_node;

    sockfd = TASKS_CFG_SRVSOCKFD(tasks_cfg);

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        sys_log(LOGSTDOUT, "error:cepoll_mod_srv_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);

    events_des = 0;
    if(CEPOLL_RD_EVENT & events)
    {   
        events_des |= (CEPOLL_IN/* | CEPOLL_ET*/);
        if(tasks_cfg != CEPOLL_NODE_RD_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_IN/* | CEPOLL_ET*/);
            CEPOLL_NODE_RD_ARG(cepoll_node) = (void *)tasks_cfg;
        }
        else
        {
            sys_log(LOGSTDOUT, "info:cepoll_mod_srv_event:sockfd %d has set CEPOLL_NODE_RD_ARG %p\n", 
                                sockfd, CEPOLL_NODE_RD_ARG(cepoll_node));
        }    
    }
    else
    {
        if(NULL_PTR != CEPOLL_NODE_RD_ARG(cepoll_node))
        {
            events_src |= (CEPOLL_IN/* | CEPOLL_ET*/);
            CEPOLL_NODE_RD_ARG(cepoll_node) = NULL_PTR;
        }
    }

    if(CEPOLL_WR_EVENT & events)
    {   
        sys_log(LOGSTDOUT, "error:cepoll_mod_srv_event:sockfd %d should never reach here\n", sockfd);
        return (EC_FALSE);
    }

    if(events_des == events_src)
    {
        sys_log(LOGSTDOUT, "[DEBUG] cepoll_mod_srv_event: ignore sockfd %d modification epoll event to %c%c\n", 
                            sockfd, CEPOLL_ALL_EVENT_CHARS(events_des));    
        return (EC_TRUE);                            
    }
    
    sys_log(LOGSTDOUT, "[DEBUG] cepoll_mod_srv_event: sockfd %d modify epoll event to %c%c\n", 
                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_des));
    
    return cepoll_mod(cepoll, sockfd, events_des);
}

EC_BOOL cepoll_del_event(CEPOLL *cepoll, const int sockfd)
{
    CEPOLL_NODE *cepoll_node;

    if(0 > sockfd || CEPOLL_MAX_FD_NUM <= sockfd)
    {
        sys_log(LOGSTDOUT, "error:cepoll_del_event:invalid sockfd %d\n", sockfd);
        return (EC_FALSE);
    }

    cepoll_node = CEPOLL_FETCH_NODE(cepoll, sockfd);
    if(NULL_PTR != CEPOLL_NODE_RD_ARG(cepoll_node)
    || NULL_PTR != CEPOLL_NODE_WR_ARG(cepoll_node))
    {
        CEPOLL_NODE_RD_ARG(cepoll_node) = NULL_PTR;
        CEPOLL_NODE_WR_ARG(cepoll_node) = NULL_PTR;
        CEPOLL_NODE_TYPE(cepoll_node)   = CEPOLL_NODE_IS_UNDEF;

        sys_log(LOGSTDOUT, "[DEBUG] cepoll_del_event: sockfd %d del epoll events\n", sockfd);    
        return cepoll_del(cepoll, sockfd);
    }
    return (EC_TRUE);
}

static EC_BOOL __cepoll_sfd_handle(CEPOLL *cepoll,  const uint32_t events, TASKS_CFG *tasks_cfg)
{
    if(events & (CEPOLL_IN | CEPOLL_HUP | CEPOLL_ERR))
    {    
        tasks_srv_accept(tasks_cfg);
    }

    if(events & (CEPOLL_OUT | CEPOLL_ERR))
    {
        sys_log(LOGSTDOUT, "error:__cepoll_sfd_handle: sockfd %d WR events should never be triggered\n", 
                            TASKS_CFG_SRVSOCKFD(tasks_cfg));
        return (EC_FALSE);                            
    }
    return (EC_TRUE);
}

static EC_BOOL __cepoll_cfd_handle(CEPOLL *cepoll,  const uint32_t events, CSOCKET_CNODE *csocket_cnode_rd, CSOCKET_CNODE *csocket_cnode_wr)
{
    if(events & (CEPOLL_IN | CEPOLL_HUP | CEPOLL_ERR))
    {    
        TASK_BRD *task_brd;
        CLIST *save_to_list;
        
        task_brd     = task_brd_default_get();
        save_to_list = TASK_BRD_QUEUE(task_brd, TASK_RECVING_QUEUE);

        if(events & CEPOLL_ERR)
        {
            if(NULL_PTR != csocket_cnode_rd)
            {
                CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_rd);
            }
            return (EC_FALSE);
        }

        if(NULL_PTR != csocket_cnode_rd && ERR_FD == CSOCKET_CNODE_SOCKFD(csocket_cnode_rd))
        {
            sys_log(LOGSTDOUT, "[DEBUG] __cepoll_cfd_handle: invalid sockfd %d RD events trigger disconnected\n", 
                                CSOCKET_CNODE_SOCKFD(csocket_cnode_rd));
                                
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_rd);
            return (EC_FALSE);
        }         
        
        if(EC_FALSE == tasks_work_irecv_on_csocket_cnode(NULL_PTR, csocket_cnode_rd, save_to_list))
        {
            sys_log(LOGSTDOUT, "[DEBUG] __cepoll_cfd_handle: sockfd %d RD events trigger disconnected\n", 
                                CSOCKET_CNODE_SOCKFD(csocket_cnode_rd));
                                
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_rd);
            return (EC_FALSE);
        }
    }

    if(events & (CEPOLL_OUT | CEPOLL_ERR))
    {
        if(events & CEPOLL_ERR)
        {
            if(NULL_PTR != csocket_cnode_wr)
            {
                CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_wr);
            }
            return (EC_FALSE);
        }

        if(NULL_PTR != csocket_cnode_wr && ERR_FD == CSOCKET_CNODE_SOCKFD(csocket_cnode_wr))
        {
            sys_log(LOGSTDOUT, "[DEBUG] __cepoll_cfd_handle: invalid sockfd %d WR events trigger disconnected\n", 
                                CSOCKET_CNODE_SOCKFD(csocket_cnode_wr));        
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_wr);
            return (EC_FALSE);
        }        
        
        if(EC_FALSE == tasks_work_isend_on_csocket_cnode(NULL_PTR, csocket_cnode_wr))
        {
            sys_log(LOGSTDOUT, "[DEBUG] __cepoll_cfd_handle: sockfd %d WR events trigger disconnected\n", 
                                CSOCKET_CNODE_SOCKFD(csocket_cnode_wr));
        
            CSOCKET_CNODE_SET_DISCONNECTED(csocket_cnode_wr);
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}

EC_BOOL cepoll_wait(CEPOLL *cepoll, int timeout_ms)
{
	int sockfd_idx;
	int sockfd_num;

	sockfd_num = epoll_wait(CEPOLL_FD(cepoll), CEPOLL_EVENT_TAB(cepoll), CEPOLL_EVENT_NUM(cepoll), timeout_ms);
	if (0 > sockfd_num)
	{
	    sys_log(LOGSTDOUT, "error:cepoll_wait: errno = %d, errstr = %s\n", errno, strerror(errno));
		return (EC_FALSE);
	} 

	/*no file descriptor became ready during the requested timeout milliseconds*/
	if (0 == sockfd_num)
	{
	    return (EC_TRUE);
	}

	sys_log(LOGSTDOUT, "[DEBUG] cepoll_wait: return sockfd_num %d\n", sockfd_num);

	for(sockfd_idx = 0 ; sockfd_idx < sockfd_num; sockfd_idx ++)
	{
	    CEPOLL_EVENT *cepoll_event;
	    CEPOLL_NODE  *cepoll_node;
	    int      sockfd;
	    uint32_t events_t;
	    
	    cepoll_event = CEPOLL_FETCH_EVENT(cepoll, sockfd_idx);
	    sockfd       = CEPOLL_EVENT_FD(cepoll_event);
	    events_t     = CEPOLL_EVENT_TYPE(cepoll_event);
	    
	    cepoll_node  = CEPOLL_FETCH_NODE(cepoll, sockfd);	    
	    if(NULL_PTR == cepoll_node)
	    {
            sys_log(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd_idx %d: sockfd %d overflow\n", sockfd_idx, sockfd);
            continue;                                
	    }

	    sys_log(LOGSTDOUT, "[DEBUG] cepoll_wait: sockfd %d triggered by events_t %c%c\n", 
	                        sockfd, CEPOLL_ALL_EVENT_CHARS(events_t));

        if(CEPOLL_NODE_IS_SERVER == CEPOLL_NODE_TYPE(cepoll_node))
	    {
	        __cepoll_sfd_handle(cepoll, events_t,(TASKS_CFG *)CEPOLL_NODE_RD_ARG(cepoll_node));
	        continue;
	    }

	    if(CEPOLL_NODE_IS_CLIENT == CEPOLL_NODE_TYPE(cepoll_node))
	    {
    	    if(EC_FALSE == __cepoll_cfd_handle(cepoll, events_t,
                    	                    (CSOCKET_CNODE *)CEPOLL_NODE_RD_ARG(cepoll_node), 
                    	                    (CSOCKET_CNODE *)CEPOLL_NODE_WR_ARG(cepoll_node)))
            {
                sys_log(LOGSTDOUT, "[DEBUG] cepoll_wait: trigger sockfd %d del epoll events\n", sockfd);    
                cepoll_del_event(cepoll, sockfd);
            }
            continue;
        }
	}

	return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
