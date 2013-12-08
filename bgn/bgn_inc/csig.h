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

#define CSIG_SHELL_CMD_LINE_BUFF_SIZE   (1024)
#define CSIG_SHELL_CMD_OUTPUT_BUFF_SIZE (1024)

typedef void (*CSIG_HANDLER)(int); 

typedef struct
{
    int signal;
    CSIG_HANDLER handler;    
}CSIG_ACTION;

#define CSIG_ACTION_SIGNAL(csig_action)     ((csig_action)->signal)
#define CSIG_ACTION_HANDLER(csig_action)    ((csig_action)->handler)


void csig_set_itimer(const int which_timer, const long useconds);
void csig_reg_action(const int which_sig, void(*sig_handle) (int));
void csig_takeover_mpi();

void csig_gdb_gcore_dump(pid_t pid);

void csig_core_dump(int signo);
void csig_os_default(int signo);
void csig_ignore(int signo);
void csig_stop(int signo);
void csig_terminate(int signo);

#endif /*_CSIG_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

