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

#ifndef _CSIG_H
#define _CSIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/time.h>
#include <signal.h>


#define CSIG_SHELL_CMD_LINE_BUFF_SIZE   (1024)
#define CSIG_SHELL_CMD_OUTPUT_BUFF_SIZE (1024)

#define CSIG_MAX_NUM    (256)

typedef struct
{
    int count;
    int rsvd;
    void (*handler)(int signo);    
}CSIG_ACTION;

typedef struct
{
    int signal_queue_len;            /* length of signal queue, <= MAX_SIGNAL (1 entry per signal max) */
    int signal_queue[ CSIG_MAX_NUM ];/* in-order queue of received signals */

    CSIG_ACTION signal_action[ CSIG_MAX_NUM ];
    sigset_t    blocked_sig;    
}CSIG;

EC_BOOL csig_init(CSIG *csig);

void csig_handler(int signo);

void csig_register(int signo, void (*handler)(int));

void csigaction_register(int signo, void (*handler)(int));

EC_BOOL csig_takeover(CSIG *csig);

void csig_process_queue();

void csig_print_queue(LOG *log);

void csig_set_itimer(const int which_timer, const long useconds);
void csig_reg_action(const int which_sig, void(*sig_handle) (int));

#if 0
void csig_takeover_mpi();
#endif

void csig_gdb_gcore_dump(pid_t pid);

void csig_core_dump(int signo);
void csig_os_default(int signo);
void csig_ignore(int signo);
void csig_stop(int signo);
void csig_interrupt(int signo);
void csig_terminate(int signo);

#endif /*_CSIG_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

