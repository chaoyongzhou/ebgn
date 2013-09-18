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

#include "lib_cstring.h"
#include "lib_cvector.h"

#include "lib_super.h"
#include "lib_tbd.h"
#include "lib_crun.h"

#include "lib_cthread.h"

#include "lib_cmpic.inc"
#include "lib_findex.inc"

#include "lib_chashalgo.h"

#include "lib_cbgt.h"
#include "lib_cdfs.h"
#include "lib_cdfsnp.h"
#include "lib_csolr.h"
#include "lib_cbytes.h"

#include "demo.h"


EC_BOOL csocket_accept(const int srv_sockfd, int *conn_sockfd, UINT32 *client_ipaddr);

EC_BOOL csocket_start_udp_mcast_sender( const UINT32 mcast_ipaddr, const UINT32 srv_port, int *srv_sockfd );

EC_BOOL csocket_stop_udp_mcast_sender( const int sockfd, const UINT32 mcast_ipaddr );

EC_BOOL csocket_start_udp_mcast_recver( const UINT32 mcast_ipaddr, const UINT32 srv_port, int *srv_sockfd );

EC_BOOL csocket_stop_udp_mcast_recver( const int sockfd, const UINT32 mcast_ipaddr );

EC_BOOL csocket_join_mcast(const int sockfd, const UINT32 mcast_ipaddr);

EC_BOOL csocket_drop_mcast(const int sockfd, const UINT32 mcast_ipaddr);

EC_BOOL csocket_udp_mcast_sendto(const int sockfd, const UINT32 mcast_ipaddr, const UINT32 mcast_port, const UINT8 *data, const UINT32 dlen);

EC_BOOL csocket_udp_mcast_recvfrom(const int sockfd, const UINT32 mcast_ipaddr, const UINT32 mcast_port, UINT8 *data, const UINT32 max_dlen, UINT32 *dlen);


int main_exec(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_exec: validity checking failed\n");
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
#if 1
    /*fwd rank entrance*/
    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }
#endif
    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

int main_cextsrv(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_cextsrv: validity checking failed\n");
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
    else if (ipv4_to_uint32("10.10.10.1") == this_tcid && CMPI_FWD_RANK == this_rank)
    {
        UINT32 cextsrv_port;
        UINT32 thread_num;

        cextsrv_port = 69001;
        thread_num   = 3;

        task_brd_default_start_cextsrv(cextsrv_port, thread_num);

        do_slave_wait_default();
    }

    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

int main_csrv(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_csrv: validity checking failed\n");
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
    else if (CMPI_FWD_RANK == this_rank && EC_TRUE == task_brd_default_check_csrv_enabled())
    {
        task_brd_default_start_csrv();

        do_slave_wait_default();
    }

    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

int main_trans(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_trans: validity checking failed\n");
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
    else if (ipv4_to_uint32("10.10.10.2") == this_tcid && CMPI_FWD_RANK == this_rank)
    {
        CSTRING *src_fname;
        CSTRING *des_fname;
        UINT32   des_tcid;

        src_fname = cstring_new((UINT8 *)"/home/ezhocha/bgn/bin/CentOS_4.4_DVD.iso", 0);
        des_fname = cstring_new((UINT8 *)"/home/ezhocha/bgn/bin/CentOS_4.4_DVD_des.iso", 0);

        des_tcid = ipv4_to_uint32("10.10.10.1");

        super_transfer(0, src_fname, des_tcid, des_fname);
        do_slave_wait_default();
    }

    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

        do_slave_wait_default();
    }

    /*work process*/
    else
    {
        do_slave_wait_default();
    }

    return (0);
}

int main_udp(int argc, char **argv)
{
    UINT32 this_tcid;
    UINT32 this_comm;
    UINT32 this_rank;

    task_brd_default_init(argc, argv);
    if(EC_FALSE == task_brd_default_check_validity())
    {
        sys_log(LOGSTDOUT, "error:main_trans: validity checking failed\n");
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
    else if (ipv4_to_uint32("10.10.10.1") == this_tcid && CMPI_FWD_RANK == this_rank)
    {
        int sockfd;
        const char *mcast_ipaddr_str = "239.2.11.71";/*239.0.0.0бл239.255.255.255*/
        UINT32 mcast_ipaddr;
        UINT32 mcast_port = 8888;

        mcast_ipaddr = ipv4_to_uint32(mcast_ipaddr_str);

        if(EC_FALSE == csocket_start_udp_mcast_recver(mcast_ipaddr, mcast_port, &sockfd))
        {
            sys_log(LOGCONSOLE, "error:start udp server %s:%ld failed\n", mcast_ipaddr_str, mcast_port);
        }
        else
        {
            UINT8 data[256];
            UINT32 dlen;

            sys_log(LOGCONSOLE, "start udp server %s:%ld\n", mcast_ipaddr_str, mcast_port);
            for(;EC_TRUE == csocket_udp_mcast_recvfrom(sockfd, mcast_ipaddr, mcast_port, data, sizeof(data)/sizeof(data[0]), &dlen);)
            {
                sys_log(LOGCONSOLE, "[DEBUG] recv udp data: %.*s\n", dlen, (char *)data);
            }
            csocket_stop_udp_mcast_recver(sockfd, mcast_ipaddr);
        }
        do_slave_wait_default();
    }

    else if (ipv4_to_uint32("10.10.10.7") == this_tcid && CMPI_FWD_RANK == this_rank)
    {
        int sockfd;
        const char *mcast_ipaddr_str = "239.2.11.71";
        UINT32 mcast_ipaddr;
        UINT32 mcast_port = 8888;

        UINT32 loop;

        mcast_ipaddr = ipv4_to_uint32(mcast_ipaddr_str);

        csocket_start_udp_mcast_sender(mcast_ipaddr, mcast_port, &sockfd);
        for(loop = 0; loop < 5; loop ++)
        {
            UINT8  data[256];
            UINT32 dlen;

            snprintf((char *)data, sizeof(data)/sizeof(data[0]), "[loop %ld] hello world!", loop);
            dlen = strlen((char *)data);
            //dlen = sizeof(data)/sizeof(data[0]);
            if(EC_FALSE == csocket_udp_mcast_sendto(sockfd, mcast_ipaddr, mcast_port, data, dlen))
            {
                sys_log(LOGSTDOUT, "error:send udp data to %s:%ld failed\n", mcast_ipaddr_str, mcast_port);
                break;
            }
            sys_log(LOGCONSOLE, "send udp data: %.*s\n", dlen, (char *)data);
            sleep(5);
        }
        csocket_stop_udp_mcast_sender(sockfd, ipv4_to_uint32(mcast_ipaddr_str));

        do_slave_wait_default();
    }

    else if (CMPI_FWD_RANK == this_rank)
    {
        sys_log(LOGSTDOUT,"======================================================================\n");
        sys_log(LOGSTDOUT,"                taskc_mgr in (tcid %s, rank %ld)                     \n", uint32_to_ipv4(this_tcid), this_rank);
        super_show_work_client(task_brd_default_get_super(), LOGSTDOUT);/*debug only*/
        sys_log(LOGSTDOUT,"======================================================================\n");

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

