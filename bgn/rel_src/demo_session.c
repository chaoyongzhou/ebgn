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
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "typeconst.h"
#include "type.h"
#include "cmisc.h"
#include "task.h"
#include "mod.h"
#include "log.h"
#include "debug.h"
#include "rank.h"

#include "cmpic.inc"

#include "cstring.h"
#include "cvector.h"

#include "findex.inc"

#include "super.h"

#include "csession.h"

int main_csession(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        dbg_log(SEC_0137_DEMO, 0)(LOGSTDOUT, "error:main_csession: validity checking failed\n");
        task_brd_default_abort();
        return (-1);
    }

    this_tcid = task_brd_default_get_tcid();
    this_comm = task_brd_default_get_comm();
    this_rank = task_brd_default_get_rank();

    if (EC_TRUE == task_brd_check_is_dbg_tcid(this_tcid) && CMPI_DBG_RANK == this_rank)
    {
        do_cmd_default();
    }
    else if (EC_TRUE == task_brd_check_is_monitor_tcid(this_tcid) && CMPI_MON_RANK == this_rank)
    {
        void * mod_mgr_def;

        mod_mgr_def = mod_mgr_new(CMPI_ERROR_MODI, LOAD_BALANCING_LOOP);
        mod_mgr_default_init(mod_mgr_def, CMPI_ANY_TCID, CMPI_ANY_RANK);

        //mod_mgr_excl(this_tcid, CMPI_ANY_COMM, this_rank, CMPI_ANY_MODI, mod_mgr_def);

        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "======================================================================\n");
        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "                       mod_mgr_default_init finished                  \n");
        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT, "======================================================================\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr_def);

        mod_mgr_free(mod_mgr_def);

        do_slave_wait_default();
    }

    /*fwd rank entrance*/
    else if (CMPI_FWD_RANK == this_rank)
    {
        UINT32 csession_md_id;
        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT,"======================================================================\n");
        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", c_word_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        dbg_log(SEC_0137_DEMO, 5)(LOGSTDOUT,"======================================================================\n");

        csession_md_id = csession_start();
        dbg_log(SEC_0137_DEMO, 9)(LOGSTDOUT, "[DEBUG] main_csession: csession_md_id = %ld\n", csession_md_id);

        do_slave_wait_default();
    }

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

