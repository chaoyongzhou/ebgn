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
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_typeconst.h"
#include "lib_type.h"
#include "lib_char2int.h"
#include "lib_task.h"
#include "lib_mod.h"
#include "lib_log.h"
#include "lib_debug.h"
#include "lib_rank.h"

#include "lib_cmpic.inc"

#include "lib_cstring.h"
#include "lib_cvector.h"

#include "lib_findex.inc"

#include "lib_super.h"

#include "lib_csession.h"

int main_csession(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_csession: validity checking failed\n");
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

        sys_log(LOGSTDOUT, "======================================================================\n");
        sys_log(LOGSTDOUT, "                       mod_mgr_default_init finished                  \n");
        sys_log(LOGSTDOUT, "======================================================================\n");
        mod_mgr_print(LOGSTDOUT, mod_mgr_def);

        mod_mgr_free(mod_mgr_def);

        do_slave_wait_default();
    }

    /*fwd rank entrance*/
    else if (CMPI_FWD_RANK == this_rank)
    {
        UINT32 csession_md_id;
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        csession_md_id = csession_start();
        sys_log(LOGSTDOUT, "[DEBUG] main_csession: csession_md_id = %ld\n", csession_md_id);

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

