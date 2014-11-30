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
#include <stdarg.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "typeconst.h"
#include "type.h"
#include "mm.h"
#include "cmisc.h"
#include "task.h"
#include "mod.h"
#include "log.h"
#include "debug.h"
#include "rank.h"

#include "cstring.h"
#include "cvector.h"

#include "super.h"
#include "tbd.h"
#include "crun.h"

#include "cthread.h"

#include "cmpic.inc"
#include "findex.inc"



int main_default(int argc, char **argv)
{
    task_brd_default_init(argc, argv);
    //c_sleep(3, LOC_DEMO_0026);
    //dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] main_default: sleep to wait tcp enter established ... shit!\n");
    if(EC_FALSE == task_brd_default_check_validity())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:main_default: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "[DEBUG] main_default: validity checking done\n");

    
    /*start the defined runner on current (tcid, rank)*/
    task_brd_default_start_runner();   

    return (0);
}


#ifdef __cplusplus
}
#endif/*__cplusplus*/

