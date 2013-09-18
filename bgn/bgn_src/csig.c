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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <sys/time.h>
#include <signal.h>

#include "type.h"
#include "mm.h"
#include "log.h"
#include "csig.h"
#include "char2int.h"

#include "debug.h"
#include "ccoredumper.h"
#include "task.inc"
#include "task.h"
#include "croutine.h"

/*********************************************************************************************************************************************************************
man 7 signal
=============

IGNAL(7)                                                             Linux Programmer Manual                                                             SIGNAL(7)

NAME
       signal - list of available signals

DESCRIPTION
       Linux supports both POSIX reliable signals (hereinafter "standard signals") and POSIX real-time signals.

   Standard Signals
       Linux  supports the standard signals listed below. Several signal numbers are architecture dependent, as indicated in the "Value" column.  (Where three values
       are given, the first one is usually valid for alpha and sparc, the middle one for i386, ppc and sh, and the last one for mips.  A - denotes that a  signal  is
       absent on the corresponding architecture.)

       The entries in the "Action" column of the table specify the default action for the signal, as follows:

       Term   Default action is to terminate the process.

       Ign    Default action is to ignore the signal.

       Core   Default action is to terminate the process and dump core.

       Stop   Default action is to stop the process.

       First the signals described in the original POSIX.1 standard.

       Signal     Value     Action   Comment
       -------------------------------------------------------------------------
       SIGHUP        1       Term    Hangup detected on controlling terminal
                                     or death of controlling process
       SIGINT        2       Term    Interrupt from keyboard
       SIGQUIT       3       Core    Quit from keyboard
       SIGILL        4       Core    Illegal Instruction
       SIGABRT       6       Core    Abort signal from abort(3)
       SIGFPE        8       Core    Floating point exception
       SIGKILL       9       Term    Kill signal
       SIGSEGV      11       Core    Invalid memory reference
       SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
       SIGALRM      14       Term    Timer signal from alarm(2)
       SIGTERM      15       Term    Termination signal
       SIGUSR1   30,10,16    Term    User-defined signal 1
       SIGUSR2   31,12,17    Term    User-defined signal 2
       SIGCHLD   20,17,18    Ign     Child stopped or terminated
       SIGCONT   19,18,25            Continue if stopped
       SIGSTOP   17,19,23    Stop    Stop process
       SIGTSTP   18,20,24    Stop    Stop typed at tty
       SIGTTIN   21,21,26    Stop    tty input for background process
       SIGTTOU   22,22,27    Stop    tty output for background process

       The signals SIGKILL and SIGSTOP cannot be caught, blocked, or ignored.

       Next the signals not in the POSIX.1 standard but described in SUSv2 and SUSv3 / POSIX 1003.1-2001.

       Signal       Value     Action   Comment
       -------------------------------------------------------------------------
       SIGBUS      10,7,10     Core    Bus error (bad memory access)
       SIGPOLL                 Term    Pollable event (Sys V). Synonym of SIGIO
       SIGPROF     27,27,29    Term    Profiling timer expired
       SIGSYS      12,-,12     Core    Bad argument to routine (SVID)
       SIGTRAP        5        Core    Trace/breakpoint trap
       SIGURG      16,23,21    Ign     Urgent condition on socket (4.2 BSD)
       SIGVTALRM   26,26,28    Term    Virtual alarm clock (4.2 BSD)
       SIGXCPU     24,24,30    Core    CPU time limit exceeded (4.2 BSD)
       SIGXFSZ     25,25,31    Core    File size limit exceeded (4.2 BSD)

       Up  to  and  including Linux 2.2, the default behaviour for SIGSYS, SIGXCPU, SIGXFSZ, and (on architectures other than SPARC and MIPS) SIGBUS was to terminate
       the process (without a core dump).  (On some other Unices the default action for SIGXCPU and SIGXFSZ is to terminate the process without a core dump.)   Linux
       2.4 conforms to the POSIX 1003.1-2001 requirements for these signals, terminating the process with a core dump.

       Next various other signals.

       Signal       Value     Action   Comment
       --------------------------------------------------------------------
       SIGIOT         6        Core    IOT trap. A synonym for SIGABRT
       SIGEMT       7,-,7      Term
       SIGSTKFLT    -,16,-     Term    Stack fault on coprocessor (unused)
       SIGIO       23,29,22    Term    I/O now possible (4.2 BSD)
       SIGCLD       -,-,18     Ign     A synonym for SIGCHLD
       SIGPWR      29,30,19    Term    Power failure (System V)
       SIGINFO      29,-,-             A synonym for SIGPWR
       SIGLOST      -,-,-      Term    File lock lost
       SIGWINCH    28,28,20    Ign     Window resize signal (4.3 BSD, Sun)
       SIGUNUSED    -,31,-     Term    Unused signal (will be SIGSYS)

       (Signal 29 is SIGINFO / SIGPWR on an alpha but SIGLOST on a sparc.)

       SIGEMT  is  not  specified  in POSIX 1003.1-2001, but neverthless appears on most other Unices, where its default action is typically to terminate the process
       with a core dump.
*********************************************************************************************************************************************************************/

/*define signals which will be taken over*/
static CSIG_ACTION g_csig_action_tbl[] = {
   // {SIGABRT , csig_core_dump,},
    {SIGFPE  , csig_core_dump,},
    {SIGILL  , csig_core_dump,},
    {SIGQUIT , csig_core_dump,},
    {SIGSEGV , csig_core_dump,},
    {SIGTRAP , csig_core_dump,},
    {SIGSYS  , csig_core_dump,},
    {SIGBUS  , csig_core_dump,},
    {SIGXCPU , csig_core_dump,},
    //{SIGPIPE , csig_core_dump,},
};

void csig_set_itimer(const int which_timer, const long useconds)
{
    struct itimerval itimer;

    itimer.it_value.tv_sec = (long)(useconds / 1000);
    itimer.it_value.tv_usec = (long)(useconds % 1000);
    itimer.it_interval = itimer.it_value;

    setitimer(/*ITIMER_REAL*/which_timer, &itimer, NULL_PTR); /*ITIMER_REAL timer, when timer is reached, send out SIGALRM*/
    return;
}

void csig_reg_action(const int which_sig, void(*sig_handle) (int))
{
    struct sigaction sigact;

    sigact.sa_handler = sig_handle; /*register signal handler*/
    sigact.sa_flags = 0;

    sigemptyset(&sigact.sa_mask); /*initialize signal set*/

    sigaction(which_sig, &sigact, NULL_PTR); /*register signal*/

    /**
     * unblock all the signals, because if the current process is
     * spawned in the previous signal handler, all the signals are
     * blocked. In order to make it sense of signals, we should
     * unblock them. Certainly, you should call this function as
     * early as possible. :)
     **/
    sigprocmask(SIG_UNBLOCK, &sigact.sa_mask, NULL_PTR);

    return;
}

/**
*   core filename format parameters:
*   %%	%
*   %p	pid
*   %u	uid
*   %g	gid
*   %s	signal which triggered the core dump
*   %t	time of core dump
*   %h	hostname
*   %e	executable filename
*   %c	ulimit -c
*
* core filename format definition:
*   echo "/tmp/cores/core-%e-%p-%t-%s " > /proc/sys/kernel/core_pattern
*
* append pid to core filename enabling:
*   echo "1" > /proc/sys/kernel/core_uses_pid
*
* set core file size to unlimited
*   ulimit -c unlimited
*
* gcore is one part of gdb tool suite
* gcore create a core dump of certain process by
*   gcore -o <core filename> <pid>
*
* gdb check the core file by
*   gdb <executable filename> <core filename>
*
**/

void csig_gdb_gcore_dump(pid_t pid)
{
    FILE * rstream;
    char   cmd_line[CSIG_SHELL_CMD_LINE_BUFF_SIZE];
    char   cmd_output[CSIG_SHELL_CMD_OUTPUT_BUFF_SIZE];

    struct tm *cur_time;
    time_t timestamp;

    /*check gcore of gdb existing*/
    snprintf(cmd_line, CSIG_SHELL_CMD_LINE_BUFF_SIZE - 4, "which gcore 2>/dev/null");
    sys_log(LOGSTDOUT, "csig_gcore_dump: execute shell command: %s\n", cmd_line);

    rstream = popen((char *)cmd_line, "r");
    fgets(cmd_output, CSIG_SHELL_CMD_OUTPUT_BUFF_SIZE - 4, rstream);
    pclose( rstream );

    if(0 == strlen(cmd_output))
    {
        sys_log(LOGSTDERR, "csig_gcore_dump: no result for shell command: %s\n", cmd_line);
        return;
    }

    /*get time string*/
    time(&timestamp);
    cur_time = c_localtime_r(&timestamp);

    /*encapsulate cmd line*/
    snprintf(cmd_line, CSIG_SHELL_CMD_LINE_BUFF_SIZE - 4, "gcore -o core-%d-%4d%02d%02d-%02d%02d%02d %d",
                        pid,
                        cur_time->tm_year + 1900,
                        cur_time->tm_mon + 1,
                        cur_time->tm_mday,
                        cur_time->tm_hour,
                        cur_time->tm_min,
                        cur_time->tm_sec,
                        pid
           );
    sys_log(LOGSTDOUT, "csig_gcore_dump: execute shell command: %s\n", cmd_line);

    /*execute cmd line*/
    rstream = popen((char *)cmd_line, "r");
    pclose( rstream );

    return;
}

#if 0
void csig_core_dump(int signo)
{
    char   core_file[CSIG_SHELL_CMD_LINE_BUFF_SIZE];

    struct tm *cur_time;
    time_t timestamp;

    //struct CoredumperCompressor *compressor;
    sys_log(LOGSTDOUT, "error:csig_core_dump: signal %d trigger core dump ......\n", signo);

    /*get time string*/
    time(&timestamp);
    cur_time = c_localtime_r(&timestamp);

    /*encapsulate cmd line*/
    snprintf(core_file, CSIG_SHELL_CMD_LINE_BUFF_SIZE - 4, "core-%d-%4d%02d%02d-%02d:%02d:%02d",
                        getpid(),
                        cur_time->tm_year + 1900,
                        cur_time->tm_mon + 1,
                        cur_time->tm_mday,
                        cur_time->tm_hour,
                        cur_time->tm_min,
                        cur_time->tm_sec,
           );


    //csig_gdb_gcore_dump(getpid());
    //signal(signo, SIG_DFL);/*restore to OS default handler!*/
    //ccoredumper_write_compressed("corexx", -1, COREDUMPER_COMPRESSED, &compressor);
    ccoredumper_write(core_file);
    raise(signo);

    return;
}
#endif

#if 0
/*ok*/
void csig_core_dump(int signo)
{
    sys_log(LOGSTDOUT, "error:csig_core_dump: signal %d trigger core dump ......\n", signo);

    csig_gdb_gcore_dump(getpid());
    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);

    return;
}
#endif

void csig_core_dump(int signo)
{
    sys_log(LOGSTDOUT, "error:csig_core_dump: signal %d trigger core dump ......\n", signo);

    if(0)
    {
        TASK_BRD *task_brd;
        task_brd = task_brd_default_get();
        croutine_pool_print(LOGSTDOUT, TASK_BRD_CROUTINE_POOL(task_brd));
    }

    if(0)/*debug*/
    {
        for(;;)
        {
            sys_log(LOGSTDOUT, "[DEBUG] csig_core_dump: wait for gdb ...\n");
            sleep(300);
        }
    }
    
    //dbg_exit(MD_END, ERR_MODULE_ID);

    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);

    return;
}


void csig_os_default(int signo)
{
    sys_log(LOGSTDOUT, "warn:csig_os_default: process %d, signal %d restore default action\n", getpid(), signo);

    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);
    return;
}
void csig_ignore(int signo)
{
    sys_log(LOGSTDOUT, "warn:csig_ignore: process %d, signal %d was ignored\n", getpid(), signo);

    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);
    return;
}

void csig_stop(int signo)
{
    sys_log(LOGSTDOUT, "error:csig_stop: process %d, signal %d trigger stopping ......\n", getpid(), signo);

    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);
    return;
}

void csig_terminate(int signo)
{
    sys_log(LOGSTDOUT, "error:csig_terminate: process %d, signal %d trigger terminating ......\n", getpid(), signo);

    signal(signo, SIG_DFL);/*restore to OS default handler!*/
    raise(signo);
    return;
}

#if 0
/*taskover sig handler of MPI!*/
void csig_takeover_mpi()
{
    UINT32 csig_idx;

    for (csig_idx = 0; csig_idx < sizeof(g_csig_action_tbl)/sizeof(g_csig_action_tbl[0]); csig_idx ++)
    {
        CSIG_ACTION *csig_action;

        csig_action = &(g_csig_action_tbl[ csig_idx ]);
        csig_reg_action(CSIG_ACTION_SIGNAL(csig_action), CSIG_ACTION_HANDLER(csig_action));
    }
    return;
}
#endif

#if 1
/*taskover sig handler of MPI!*/
void csig_takeover_mpi()
{
    struct sigaction sig_act;
    UINT32 csig_idx;

    memset(&sig_act, 0, sizeof(sig_act));
    sig_act.sa_handler = csig_core_dump;
    //sigfillset(&act.sa_mask);
    sigemptyset(&sig_act.sa_mask);

    for (csig_idx = 0; csig_idx < sizeof(g_csig_action_tbl)/sizeof(g_csig_action_tbl[0]); csig_idx ++)
    {
        CSIG_ACTION *csig_action;

        csig_action = &(g_csig_action_tbl[ csig_idx ]);
        sigaction(CSIG_ACTION_SIGNAL(csig_action), &sig_act, NULL_PTR);
    }
    /* unblock all the signals, because if the current process is
     * spawned in the previous signal handler, all the signals are
     * blocked. In order to make it sense of signals, we should
     * unblock them. Certainly, you should call this function as
     * early as possible. :) */
    sigprocmask(SIG_UNBLOCK, &sig_act.sa_mask, NULL_PTR);
    return;
}
#endif

#ifdef __cplusplus
}
#endif/*__cplusplus*/

