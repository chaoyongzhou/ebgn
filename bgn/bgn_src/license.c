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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <malloc.h>
#include <unistd.h>

#include <errno.h>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>

#include "type.h"

#include "mm.h"
#include "log.h"
#include "cmisc.h"
#include "bgnz.h"
#include "eccfp.h"
#include "license.h"
#include "print.h"

#if ( 192 <= BIGINTSIZE )
#if (32 == WORDSIZE)
static BIGINT P_192_p = {
     6,{0xffffffff,0xffffffff,0xfffffffe,0xffffffff,0xffffffff,0xffffffff}
};
static ECFP_CURVE P_192_curve = {
    {6,{0xfffffffc,0xffffffff,0xfffffffe,0xffffffff,0xffffffff,0xffffffff}},
    {6,{0xC146B9B1,0xFEB8DEEC,0x72243049,0x0FA7E9AB,0xE59C80E7,0x64210519}},
};
static BIGINT P_192_order = {
    6,{0xB4D22831,0x146BC9B1,0x99DEF836,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
};
static EC_CURVE_POINT P_192_basepoint = {
    {6,{0x8f81d6e4,0x01bccb80,0xbb3705b4,0x98a23123,0xafd3debd,0xedf5e267}},
    {6,{0x1d0535aa,0x6c48a8d3,0x2b81d887,0xd2a4c0a5,0x6e09ffc2,0x202b9c28}},
};
static BIGINT P_192_private_key = {
    6, {0x2355FFB4,0x270B3943,0xD7BFD8BA,0x5044B0B7,0xF5413256,0x0C04B3AB}
};
#endif/*(32 == WORDSIZE)*/
#if (64 == WORDSIZE)
static BIGINT P_192_p = {
     3,
     {0xffffffffffffffff,0xfffffffeffffffff,0xffffffffffffffff}
};
static ECFP_CURVE P_192_curve = {
    {3,{0xfffffffcffffffff,0xfffffffeffffffff,0xffffffffffffffff}},
    {3,{0xC146B9B1FEB8DEEC,0x722430490FA7E9AB,0xE59C80E764210519}},
};
static BIGINT P_192_order = {
    3,{0xB4D22831146BC9B1,0x99DEF836FFFFFFFF,0xFFFFFFFFFFFFFFFF}
};
static EC_CURVE_POINT P_192_basepoint = {
    {3,{0x8f81d6e401bccb80,0xbb3705b498a23123,0xafd3debdedf5e267}},
    {3,{0x1d0535aa6c48a8d3,0x2b81d887d2a4c0a5,0x6e09ffc2202b9c28}},
};
static BIGINT P_192_private_key = {
    3, {0x2355FFB4270B3943,0xD7BFD8BA5044B0B7,0xF54132560C04B3AB}
};
#endif/*(64 == WORDSIZE)*/
#else
#error "fatal error: BIGINTSIZE < 192"
#endif /*( 192 <= BIGINTSIZE )*/

static const char *g_lic_version      = "5.0.0.0";
static const char *g_lic_vendor_name  = "Chaoyong Zhou";
static const char *g_lic_vendor_email = "bgnvednor@gmail.com";

static const char *g_lic_file_name    = "license.dat";

const char *lic_version()
{
    return g_lic_version;
}

EC_BOOL lic_version_verify(const char *version)
{
    if(0 != strcmp(version, lic_version()))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

const char *lic_vendor_name()
{
    return g_lic_vendor_name;
}

EC_BOOL lic_vendor_name_verify(const char *vendor_name)
{
    if(0 != strcmp(vendor_name, lic_vendor_name()))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

const char *lic_vendor_email()
{
    return g_lic_vendor_email;
}

EC_BOOL lic_vendor_email_verify(const char *vendor_email)
{
    if(0 != strcmp(vendor_email, lic_vendor_email()))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL lic_vendor_verify(const char *vendor_name, const char *vendor_email)
{
    if(EC_FALSE == lic_vendor_name_verify(vendor_name) || EC_FALSE == lic_vendor_email_verify(vendor_email))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL lic_buff_load(int fd, UINT32 *offset, const RWSIZE rsize, UINT8 *buff)
{
    RWSIZE csize;/*read completed size*/
    RWSIZE osize;/*read once size*/

    if(-1 == lseek(fd, (*offset), SEEK_SET))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_buff_load: seek offset %ld failed\n", (*offset));
        return (EC_FALSE);
    }

    csize = 0;
    osize = rsize;

    if(osize != read(fd, buff + csize, osize))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_buff_load: load buff from offset %ld failed where rsize %ld, csize %ld, osize %ld, errno %d, errstr %s\n",
                            (*offset), rsize, csize, osize, errno, strerror(errno));
        return (EC_FALSE);
    }

    (*offset) += rsize;

    //dbg_log(SEC_0060_LICENSE, 5)(LOGSTDOUT, "lic_buff_load: load %ld bytes\n", rsize);

    return (EC_TRUE);
}


EC_BOOL lic_buff_flush(int fd, UINT32 *offset, const RWSIZE wsize, const UINT8 *buff)
{
    RWSIZE csize;/*write completed size*/
    RWSIZE osize;/*write once size*/

    if(-1 == lseek(fd, (*offset), SEEK_SET))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_buff_flush: seek offset %ld failed\n", (*offset));
        return (EC_FALSE);
    }

    csize = 0;
    osize = wsize;

    if(osize != write(fd, buff + csize, osize))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_buff_flush: flush buff to offset %ld failed where wsize %ld, csize %ld, osize %ld, errno %d, errstr %s\n",
                            (*offset), wsize, csize, osize, errno, strerror(errno));
        return (EC_FALSE);
    }

    (*offset) += wsize;

    return (EC_TRUE);
}

EC_BOOL lic_mac_collect(LIC_MAC *lic_mac_tbl, const UINT32 lic_mac_tbl_max_size, UINT32 *lic_mac_num)
{
    int fd;
    UINT32 net_interface;
    struct ifreq buf[ LIC_INTERFACE_MAX_NUM ];
    struct ifconf ifc;

    UINT32 lic_mac_idx;
    LIC_MAC *lic_mac;

    lic_mac_idx = 0;

    if ((fd = csocket_open(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t) buf;

        if (ioctl(fd, SIOCGIFCONF, (char *) &ifc))
        {
            close(fd);
            return (EC_FALSE);
        }

        net_interface = sizeof(buf) / sizeof(buf[0]);
        if(net_interface > lic_mac_tbl_max_size)
        {
            net_interface = lic_mac_tbl_max_size;
        }

        while (net_interface -- > 0)
        {
            /*Jugde whether the net card status is promisc */
            if (ioctl(fd, SIOCGIFFLAGS, (char *) &buf[net_interface]))
            {
                continue;
            }

            if (!(buf[net_interface].ifr_flags & IFF_UP))
            {
                continue;
            }

            /*Get HW ADDRESS of the net card */
            if (ioctl(fd, SIOCGIFHWADDR, (char *) &buf[net_interface]))
            {
                continue;
            }

            lic_mac = (lic_mac_tbl + lic_mac_idx);
#if 0
            sys_log(stdout, "HW address is:");

            sys_print(stdout, "%02x:%02x:%02x:%02x:%02x:%02x\n",
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[0],
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[1],
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[2],
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[3],
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[4],
                    (unsigned char) buf[net_interface].ifr_hwaddr.sa_data[5]);
#endif
            LIC_MAC_ADDR(lic_mac, 0) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[0];
            LIC_MAC_ADDR(lic_mac, 1) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[1];
            LIC_MAC_ADDR(lic_mac, 2) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[2];
            LIC_MAC_ADDR(lic_mac, 3) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[3];
            LIC_MAC_ADDR(lic_mac, 4) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[4];
            LIC_MAC_ADDR(lic_mac, 5) = (UINT8)buf[net_interface].ifr_hwaddr.sa_data[5];
            lic_mac_idx ++;
        }
    }

    close(fd);

    (*lic_mac_num) = lic_mac_idx;
    return (EC_TRUE);
}

EC_BOOL lic_mac_cmp(const LIC_MAC *lic_mac_1st, const LIC_MAC *lic_mac_2nd)
{
    UINT32 idx;

    for(idx = 0; idx < LIC_MAC_ADDR_SIZE; idx ++)
    {
        if(LIC_MAC_ADDR(lic_mac_1st, idx) != LIC_MAC_ADDR(lic_mac_2nd, idx))
        {
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL lic_mac_exist(const LIC_MAC *lic_mac_tbl, const UINT32 lic_mac_num, const LIC_MAC *lic_mac)
{
    UINT32 lic_mac_idx;

    for(lic_mac_idx = 0; lic_mac_idx < lic_mac_num; lic_mac_idx ++)
    {
        if(EC_TRUE == lic_mac_cmp(lic_mac, lic_mac_tbl + lic_mac_idx))
        {
            return (EC_TRUE);
        }
    }
    return (EC_FALSE);
}

EC_BOOL lic_mac_check(const LIC_MAC *lic_mac)
{
    if(0 == LIC_MAC_ADDR(lic_mac, 0)
    && 0 == LIC_MAC_ADDR(lic_mac, 1)
    && 0 == LIC_MAC_ADDR(lic_mac, 2)
    && 0 == LIC_MAC_ADDR(lic_mac, 3)
    && 0 == LIC_MAC_ADDR(lic_mac, 4)
    && 0 == LIC_MAC_ADDR(lic_mac, 5)
    )
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL lic_mac_verify(const LIC_MAC *lic_mac)
{
    LIC_MAC lic_mac_tbl[LIC_INTERFACE_MAX_NUM];
    UINT32  lic_mac_tbl_max_size;
    UINT32  lic_mac_num;

    lic_mac_tbl_max_size = sizeof(lic_mac_tbl)/sizeof(lic_mac_tbl[0]);
    if(EC_FALSE == lic_mac_collect(lic_mac_tbl, lic_mac_tbl_max_size, &lic_mac_num))
    {
        return (EC_FALSE);
    }

    return lic_mac_exist(lic_mac_tbl, lic_mac_num, lic_mac);
}

EC_BOOL lic_mac_init(LIC_MAC *lic_mac)
{
    UINT32 idx;

    for(idx = 0; idx < LIC_MAC_ADDR_SIZE; idx ++)
    {
        LIC_MAC_ADDR(lic_mac, idx) = 0;
    }

    return (EC_TRUE);
}

EC_BOOL lic_mac_make(LIC_MAC *lic_mac, const char *mac_str)
{
    if(EC_FALSE == str_to_mac_addr(mac_str, &(LIC_MAC_ADDR(lic_mac, 0))))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_mac_make: invalid mac addr %s\n", mac_str);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_mac_check(lic_mac))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_mac_make: illegal mac addr %s\n", mac_str);
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

void lic_mac_print(LOG *log, const LIC_MAC *lic_mac)
{
    sys_log(log, "mac addr    : %02x:%02x:%02x:%02x:%02x:%02x\n",
                             (LIC_MAC_ADDR(lic_mac, 0)),
                             (LIC_MAC_ADDR(lic_mac, 1)),
                             (LIC_MAC_ADDR(lic_mac, 2)),
                             (LIC_MAC_ADDR(lic_mac, 3)),
                             (LIC_MAC_ADDR(lic_mac, 4)),
                             (LIC_MAC_ADDR(lic_mac, 5))
                             );
    return;
}

EC_BOOL lic_chars_init(UINT8 *str, const UINT32 len)
{
    UINT32 idx;

    for(idx = 0; idx < len; idx ++)
    {
        str[ idx ] = '\0';
    }
    return (EC_TRUE);
}

EC_BOOL lic_chars_make(UINT8 *des_str, const UINT32 des_max_size, const char *src_str)
{
    UINT32 src_str_len;
    UINT32 idx;

    src_str_len = strlen(src_str);
    if(src_str_len > des_max_size)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_chars_make: src str %s size = %ld overflow where max size = %ld\n",
                            src_str, src_str_len, des_max_size);
        return (EC_FALSE);
    }

    for(idx = 0; idx < src_str_len; idx ++)
    {
        des_str[ idx ] = src_str[ idx ];
    }
    return (EC_TRUE);
}

EC_BOOL lic_date_init(LIC_DATE *lic_date)
{
    LIC_DATE_START_YEAR(lic_date)  = 0;
    LIC_DATE_START_MONTH(lic_date) = 0;
    LIC_DATE_START_DAY(lic_date)   = 0;
    LIC_DATE_START_RSVD(lic_date)  = 0;

    LIC_DATE_END_YEAR(lic_date)  = 0;
    LIC_DATE_END_MONTH(lic_date) = 0;
    LIC_DATE_END_DAY(lic_date)   = 0;
    LIC_DATE_END_RSVD(lic_date)  = 0;

    return (EC_TRUE);
}

EC_BOOL lic_date_clean(LIC_DATE *lic_date)
{
    LIC_DATE_START_YEAR(lic_date)  = 0;
    LIC_DATE_START_MONTH(lic_date) = 0;
    LIC_DATE_START_DAY(lic_date)   = 0;
    LIC_DATE_START_RSVD(lic_date)  = 0;

    LIC_DATE_END_YEAR(lic_date)  = 0;
    LIC_DATE_END_MONTH(lic_date) = 0;
    LIC_DATE_END_DAY(lic_date)   = 0;
    LIC_DATE_END_RSVD(lic_date)  = 0;

    return (EC_TRUE);
}

EC_BOOL lic_date_check(const LIC_DATE *lic_date)
{
    if(LIC_DATE_START_YEAR(lic_date) > LIC_DATE_END_YEAR(lic_date))
    {
        return (EC_FALSE);
    }

    if(LIC_DATE_START_YEAR(lic_date) < LIC_DATE_END_YEAR(lic_date))
    {
        return (EC_TRUE);
    }

    /*now start year == end year*/

    if(LIC_DATE_START_MONTH(lic_date) > LIC_DATE_END_MONTH(lic_date))
    {
        return (EC_FALSE);
    }

    if(LIC_DATE_START_MONTH(lic_date) < LIC_DATE_END_MONTH(lic_date))
    {
        return (EC_TRUE);
    }

    /*now start month == end month*/

    if(LIC_DATE_START_DAY(lic_date) > LIC_DATE_END_DAY(lic_date))
    {
        return (EC_FALSE);
    }

    if(LIC_DATE_START_DAY(lic_date) < LIC_DATE_END_DAY(lic_date))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL lic_date_make(LIC_DATE *lic_date, const char *start_date_str, const char *end_date_str)
{
    char  start_date_str_tmp[32];
    char  end_date_str_tmp[32];
    char  *fields[8];
    UINT32 field_num;

    UINT32 year;
    UINT32 month;
    UINT32 day;

    snprintf(start_date_str_tmp, sizeof(start_date_str_tmp)/sizeof(start_date_str_tmp[0]), "%s", start_date_str);
    snprintf(end_date_str_tmp, sizeof(end_date_str_tmp)/sizeof(end_date_str_tmp[0]), "%s", end_date_str);

    /*start date*/
    field_num = c_str_split(start_date_str_tmp, "-/", fields, sizeof(fields)/sizeof(fields[ 0 ]));
    if(3 != field_num)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_date_make: invalid start date: %s\n", start_date_str);
        return (EC_FALSE);
    }

    year  = c_str_to_word(fields[ 0 ]);
    month = c_str_to_word(fields[ 1 ]);
    day   = c_str_to_word(fields[ 2 ]);

    if(2012 > year || year > 4095 || 1 > month || month > 12 || 1 > day || day > 31)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_date_make: illegal start date: %s\n", start_date_str);
        return (EC_FALSE);
    }

    LIC_DATE_START_YEAR(lic_date)  = year;
    LIC_DATE_START_MONTH(lic_date) = month;
    LIC_DATE_START_DAY(lic_date)   = day;

    /*end date*/
    field_num = c_str_split(end_date_str_tmp, "-/", fields, sizeof(fields)/sizeof(fields[ 0 ]));
    if(3 != field_num)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_date_make: invalid end date: %s\n", end_date_str);
        return (EC_FALSE);
    }

    year  = c_str_to_word(fields[ 0 ]);
    month = c_str_to_word(fields[ 1 ]);
    day   = c_str_to_word(fields[ 2 ]);

    if(2012 > year || year > 4095 || 1 > month || month > 12 || 1 > day || day > 31)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_date_make: illegal end date: %s\n", end_date_str);
        return (EC_FALSE);
    }

    LIC_DATE_END_YEAR(lic_date)  = year;
    LIC_DATE_END_MONTH(lic_date) = month;
    LIC_DATE_END_DAY(lic_date)   = day;

    if(EC_FALSE == lic_date_check(lic_date))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_date_make: illegal issued expiration: from %s to %s\n", start_date_str, end_date_str);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void lic_date_print(LOG *log, const LIC_DATE *lic_date)
{
    sys_log(log, "start date  : %4ld-%02ld-%02d\n",
                LIC_DATE_START_YEAR(lic_date),
                LIC_DATE_START_MONTH(lic_date),
                LIC_DATE_START_DAY(lic_date)
                );

    sys_log(log, "end date    : %4ld-%02ld-%02d\n",
                LIC_DATE_END_YEAR(lic_date),
                LIC_DATE_END_MONTH(lic_date),
                LIC_DATE_END_DAY(lic_date)
                );
    return;
}

EC_BOOL lic_prikey_make(BIGINT *prikey)
{
    UINT32 bgnz_md_id;
    bgnz_md_id = bgn_z_start();
    bgn_z_clone(bgnz_md_id, &P_192_private_key, prikey);
    bgn_z_end(bgnz_md_id);
    return (EC_TRUE);
}

void lic_prikey_print(LOG *log, const BIGINT *prikey)
{
    sys_log(log, "private key : ");
    print_bigint_dec(log, prikey);
    return;
}

void lic_signature_print(LOG *log, const ECC_SIGNATURE *ecc_signature)
{
    sys_log(log, "signature[r]: ");
    print_bigint_dec(log, &(ecc_signature->r));

    sys_log(log, "signature[s]: ");
    print_bigint_dec(log, &(ecc_signature->s));
    return;
}

EC_BOOL lic_cfg_init(LIC_CFG *lic_cfg)
{
    lic_chars_init(LIC_CFG_VERSION(lic_cfg)     , LIC_VERSION_MAX_SIZE);
    lic_mac_init(LIC_CFG_MAC(lic_cfg));

    lic_date_init(LIC_CFG_DATE(lic_cfg));

    lic_chars_init(LIC_CFG_USER_NAME(lic_cfg)   , LIC_USER_NAME_MAX_SIZE);
    lic_chars_init(LIC_CFG_USER_EMAIL(lic_cfg)  , LIC_USER_EMAIL_MAX_SIZE);
    lic_chars_init(LIC_CFG_VENDOR_NAME(lic_cfg) , LIC_VENDOR_NAME_MAX_SIZE);
    lic_chars_init(LIC_CFG_VENDOR_EMAIL(lic_cfg), LIC_VENDOR_EMAIL_MAX_SIZE);

    return (EC_TRUE);
}

EC_BOOL lic_cfg_clean(LIC_CFG *lic_cfg)
{
    lic_chars_init(LIC_CFG_VERSION(lic_cfg)     , LIC_VERSION_MAX_SIZE);
    lic_mac_init(LIC_CFG_MAC(lic_cfg));

    lic_date_clean(LIC_CFG_DATE(lic_cfg));

    lic_chars_init(LIC_CFG_USER_NAME(lic_cfg)   , LIC_USER_NAME_MAX_SIZE);
    lic_chars_init(LIC_CFG_USER_EMAIL(lic_cfg)  , LIC_USER_EMAIL_MAX_SIZE);
    lic_chars_init(LIC_CFG_VENDOR_NAME(lic_cfg) , LIC_VENDOR_NAME_MAX_SIZE);
    lic_chars_init(LIC_CFG_VENDOR_EMAIL(lic_cfg), LIC_VENDOR_EMAIL_MAX_SIZE);

    //TODO: ecc

    return (EC_TRUE);
}

EC_BOOL lic_cfg_load(int fd, LIC_CFG *lic_cfg)
{
    UINT32 offset;

    offset = 0;

    if(EC_FALSE == lic_buff_load(fd, &offset, LIC_VERSION_MAX_SIZE, LIC_CFG_VERSION(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load version failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, sizeof(LIC_MAC), (UINT8 *)LIC_CFG_MAC(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load mac failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, sizeof(LIC_DATE), (UINT8 *)LIC_CFG_DATE(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load date failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, LIC_USER_NAME_MAX_SIZE, LIC_CFG_USER_NAME(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load user name failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, LIC_USER_EMAIL_MAX_SIZE, LIC_CFG_USER_EMAIL(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load user email failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, LIC_VENDOR_NAME_MAX_SIZE, LIC_CFG_VENDOR_NAME(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load vendor name failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, LIC_VENDOR_EMAIL_MAX_SIZE, LIC_CFG_VENDOR_EMAIL(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load vendor email failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, sizeof(BIGINT), (UINT8 *)LIC_CFG_PRIVATE_KEY(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load private key failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_load(fd, &offset, sizeof(ECC_SIGNATURE), (UINT8 *)LIC_CFG_SIGNATURE(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_load: load signature failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL lic_cfg_flush(int fd, const LIC_CFG *lic_cfg)
{
    UINT32 offset;

    offset = 0;

    if(EC_FALSE == lic_buff_flush(fd, &offset, LIC_VERSION_MAX_SIZE, LIC_CFG_VERSION(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush version failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, sizeof(LIC_MAC), (UINT8 *)LIC_CFG_MAC(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush mac failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, sizeof(LIC_DATE), (UINT8 *)LIC_CFG_DATE(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush date failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, LIC_USER_NAME_MAX_SIZE, LIC_CFG_USER_NAME(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush user name failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, LIC_USER_EMAIL_MAX_SIZE, LIC_CFG_USER_EMAIL(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush user email failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, LIC_VENDOR_NAME_MAX_SIZE, LIC_CFG_VENDOR_NAME(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush vendor name failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, LIC_VENDOR_EMAIL_MAX_SIZE, LIC_CFG_VENDOR_EMAIL(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush vendor email failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, sizeof(BIGINT), (UINT8 *)LIC_CFG_PRIVATE_KEY(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush private key failed\n");
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_buff_flush(fd, &offset, sizeof(ECC_SIGNATURE), (UINT8 *)LIC_CFG_SIGNATURE(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_flush: flush signature failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL lic_cfg_make(LIC_CFG *lic_cfg, const char *mac_str,
                            const char *start_date_str, const char *end_date_str,
                            const char *user_name_str, const char *user_email_str)
{
    if(EC_FALSE == lic_mac_make(LIC_CFG_MAC(lic_cfg), mac_str))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make mac from %s failed\n", mac_str);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_date_make(LIC_CFG_DATE(lic_cfg), start_date_str, end_date_str))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make date from start date %s to end date %s failed\n", start_date_str, end_date_str);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_chars_make(LIC_CFG_USER_NAME(lic_cfg), LIC_USER_NAME_MAX_SIZE, user_name_str))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make user name from %s failed\n", user_name_str);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_chars_make(LIC_CFG_USER_EMAIL(lic_cfg), LIC_USER_EMAIL_MAX_SIZE, user_email_str))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make user email from %s failed\n", user_name_str);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_chars_make(LIC_CFG_VENDOR_NAME(lic_cfg), LIC_VENDOR_NAME_MAX_SIZE, g_lic_vendor_name))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make vendor name from %s failed\n", g_lic_vendor_name);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_chars_make(LIC_CFG_VENDOR_EMAIL(lic_cfg), LIC_VENDOR_EMAIL_MAX_SIZE, g_lic_vendor_email))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make vendor email from %s failed\n", g_lic_vendor_email);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_chars_make(LIC_CFG_VERSION(lic_cfg), LIC_VERSION_MAX_SIZE, g_lic_version))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make software version from %s failed\n", g_lic_version);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_prikey_make(LIC_CFG_PRIVATE_KEY(lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_cfg_make: make private key failed\n");
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

void lic_cfg_print(LOG *log, const LIC_CFG *lic_cfg)
{
    sys_log(log, "version     : %s\n", LIC_CFG_VERSION(lic_cfg));
    lic_mac_print(log, LIC_CFG_MAC(lic_cfg));
    lic_date_print(log, LIC_CFG_DATE(lic_cfg));
    sys_log(log, "user name   : %s\n", LIC_CFG_USER_NAME(lic_cfg));
    sys_log(log, "user email  : %s\n", LIC_CFG_USER_EMAIL(lic_cfg));

    sys_log(log, "vendor name : %s\n", LIC_CFG_VENDOR_NAME(lic_cfg));
    sys_log(log, "vendor email: %s\n", LIC_CFG_VENDOR_EMAIL(lic_cfg));

    lic_prikey_print(log, LIC_CFG_PRIVATE_KEY(lic_cfg));
    lic_signature_print(log, LIC_CFG_SIGNATURE(lic_cfg));
    return;
}

EC_BOOL lic_cfg_signate(LIC_CFG *lic_cfg)
{
    UINT32 eccfp_md_id;

    BIGINT *p ;
    ECFP_CURVE *curve;
    BIGINT *order;
    EC_CURVE_POINT *basepoint;

    BIGINT *prikey;
    ECC_SIGNATURE *signature;

    UINT8 *message;
    UINT32 messagelen;

    UINT32 ret;

#if ( 192 <= BIGINTSIZE )
    p = &P_192_p;
    curve = &P_192_curve;
    order = &P_192_order;
    basepoint = &P_192_basepoint;
#endif/*( 192 <= BIGINTSIZE )*/

    prikey = LIC_CFG_PRIVATE_KEY(lic_cfg);
    signature = LIC_CFG_SIGNATURE(lic_cfg);

    eccfp_md_id = ecc_fp_start(p, curve, order, basepoint, 0, 0);

    message = (UINT8 *)lic_cfg;
    messagelen = LIC_CFG_MSG_LEN;
    ret = ecc_fp_signate(eccfp_md_id, prikey, message, messagelen, signature);

    ecc_fp_end(eccfp_md_id);
    return ((0 == ret) ? EC_TRUE : EC_FALSE);
}

EC_BOOL lic_cfg_verify(const LIC_CFG *lic_cfg)
{
    UINT32 eccfp_md_id;
    BIGINT *p ;
    ECFP_CURVE *curve;
    BIGINT *order;
    EC_CURVE_POINT *basepoint;

    EC_CURVE_POINT buf_1;

    BIGINT *prikey;
    EC_CURVE_POINT *pubkey;
    ECC_SIGNATURE *signature;

    UINT8 *message;
    UINT32 messagelen;

    EC_BOOL ret;

#if ( 192 <= BIGINTSIZE )
    p = &P_192_p;
    curve = &P_192_curve;
    order = &P_192_order;
    basepoint = &P_192_basepoint;
#endif/*( 192 <= BIGINTSIZE )*/

    prikey = (BIGINT *)LIC_CFG_PRIVATE_KEY(lic_cfg);
    pubkey = &buf_1;
    signature = (ECC_SIGNATURE *)LIC_CFG_SIGNATURE(lic_cfg);

    eccfp_md_id = ecc_fp_start(p, curve, order, basepoint, 0, 0);
    ecc_fp_get_public_key(eccfp_md_id, prikey, pubkey);

    message = (UINT8 *)lic_cfg;
    messagelen = LIC_CFG_MSG_LEN;

    ret = ecc_fp_verify(eccfp_md_id, pubkey, message, messagelen, signature);
#if 0
    if ( EC_TRUE == ret )
    {
        dbg_log(SEC_0060_LICENSE, 5)(LOGSTDOUT,"ECDSA verification passed\n");
    }
    else
    {
        dbg_log(SEC_0060_LICENSE, 5)(LOGSTDOUT,"ECDSA verification failed\n");
    }
#endif
    ecc_fp_end(eccfp_md_id);

    return (ret);
}

EC_BOOL lic_make(const char *mac_str,
                    const char *start_date_str, const char *end_date_str,
                    const char *user_name_str, const char *user_email_str)
{
    LIC_CFG lic_cfg;
    int fd;

    lic_cfg_init(&lic_cfg);

    if(EC_FALSE == lic_cfg_make(&lic_cfg, mac_str, start_date_str, end_date_str, user_name_str, user_email_str))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_make: make license cfg failed\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_cfg_signate(&lic_cfg))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_make: make signature failed\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    fd = c_file_open(g_lic_file_name, O_RDWR | O_CREAT, 0666);
    if(-1 == fd)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_make: open license file %s failed\n", g_lic_file_name);
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_cfg_flush(fd, &lic_cfg))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_make: generate license file %s failed\n", g_lic_file_name);

        close(fd);
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    close(fd);
    lic_cfg_clean(&lic_cfg);
    return (EC_TRUE);
}

EC_BOOL lic_check()
{
    LIC_CFG lic_cfg;

    struct tm *cur_time;

    UINT32 year;
    UINT32 month;
    UINT32 day;

    LIC_DATE   *lic_date;

    int fd;

    lic_cfg_init(&lic_cfg);

    fd = c_file_open(g_lic_file_name, O_RDONLY, 0666);
    if(-1 == fd)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: open license file %s failed\n", g_lic_file_name);
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    if(EC_FALSE == lic_cfg_load(fd, &lic_cfg))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: load license file %s failed\n", g_lic_file_name);
        close(fd);
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*check signature*/
    if(EC_FALSE == lic_cfg_verify(&lic_cfg))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: verify signature failed\n");
        close(fd);
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    close(fd);

    /*check mac addr*/
    if(EC_FALSE == lic_mac_verify(LIC_CFG_MAC(&lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: verify mac addr failed\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*check version*/
    if(EC_FALSE == lic_version_verify((char *)LIC_CFG_VERSION(&lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: verify version failed\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*check vendor*/
    if(EC_FALSE == lic_vendor_verify((char *)LIC_CFG_VENDOR_NAME(&lic_cfg), (char *)LIC_CFG_VENDOR_EMAIL(&lic_cfg)))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_check: verify vendor failed\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*check expiration*/
    cur_time = c_localtime_r(NULL_PTR);

    year  = cur_time->tm_year + 1900;
    month = cur_time->tm_mon + 1;
    day   = cur_time->tm_mday;

    lic_date = LIC_CFG_DATE(&lic_cfg);

    if(LIC_DATE_END_YEAR(lic_date) > year)
    {
        lic_cfg_clean(&lic_cfg);
        return (EC_TRUE);
    }

    if(LIC_DATE_END_YEAR(lic_date) < year)
    {
        dbg_log(SEC_0060_LICENSE, 1)(LOGSTDOUT, "warn:lic_check: license is expired\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*now year == end year*/
    if(LIC_DATE_END_MONTH(lic_date) > month)
    {
        lic_cfg_clean(&lic_cfg);
        return (EC_TRUE);
    }

    if(LIC_DATE_END_MONTH(lic_date) < month)
    {
        dbg_log(SEC_0060_LICENSE, 1)(LOGSTDOUT, "warn:lic_check: license is expired\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    /*now month == end month*/
    if(LIC_DATE_END_DAY(lic_date) < day)
    {
        dbg_log(SEC_0060_LICENSE, 1)(LOGSTDOUT, "warn:lic_check: license is expired\n");
        lic_cfg_clean(&lic_cfg);
        return (EC_FALSE);
    }

    if(LIC_DATE_END_DAY(lic_date) < day + 15)
    {
        dbg_log(SEC_0060_LICENSE, 1)(LOGSTDOUT, "warn:lic_check: license will be expired in %ld days. please contact %s/%s to renew.\n",
                            LIC_DATE_END_DAY(lic_date) - day,
                            g_lic_vendor_name, g_lic_vendor_email);
    }

    lic_cfg_clean(&lic_cfg);
    return (EC_TRUE);
}

void lic_print(LOG *log)
{
    LIC_CFG lic_cfg;

    int fd;

    lic_cfg_init(&lic_cfg);

    fd = c_file_open(g_lic_file_name, O_RDONLY, 0666);
    if(-1 == fd)
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_print: open license file %s failed\n", g_lic_file_name);
        lic_cfg_clean(&lic_cfg);
        return;
    }

    if(EC_FALSE == lic_cfg_load(fd, &lic_cfg))
    {
        dbg_log(SEC_0060_LICENSE, 0)(LOGSTDOUT, "error:lic_print: load license file %s failed\n", g_lic_file_name);
        close(fd);
        lic_cfg_clean(&lic_cfg);
        return;
    }

    close(fd);

    lic_cfg_print(log, &lic_cfg);

    lic_cfg_clean(&lic_cfg);
    return;
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/
