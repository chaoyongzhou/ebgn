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

#ifndef _LICENSE_H
#define _LICENSE_H

#include "type.h"
#include "eccfp.h"

#define LIC_MAC_ADDR_SIZE           (6)
#define LIC_MAC_ADDR_RSVD_SIZE      (2)
#define LIC_INTERFACE_MAX_NUM       ((UINT32) 16)/*support network card num up to 16*/

#define LIC_VERSION_MAX_SIZE        ( 32)
#define LIC_USER_NAME_MAX_SIZE      (128)
#define LIC_USER_EMAIL_MAX_SIZE     ( 64)
#define LIC_VENDOR_NAME_MAX_SIZE    (128)
#define LIC_VENDOR_EMAIL_MAX_SIZE   ( 64)

typedef struct
{
    UINT8   addr[LIC_MAC_ADDR_SIZE];
    UINT8   rsvd[LIC_MAC_ADDR_RSVD_SIZE];
}LIC_MAC;
#define LIC_MAC_ADDR_TBL(lic_mac)       ((lic_mac)->addr)
#define LIC_MAC_ADDR(lic_mac, idx)      ((lic_mac)->addr[ (idx) ])

typedef struct
{
    UINT32      start_year :12;
    UINT32      start_month:4;
    UINT32      start_day  :5;
    UINT32      start_rsvd :11;
    UINT32      end_year   :12;
    UINT32      end_month  :4;
    UINT32      end_day    :5;
    UINT32      end_rsvd   :11;
}LIC_DATE;

#define LIC_DATE_START_YEAR(lic_date)     ((lic_date)->start_year)
#define LIC_DATE_START_MONTH(lic_date)    ((lic_date)->start_month)
#define LIC_DATE_START_DAY(lic_date)      ((lic_date)->start_day)
#define LIC_DATE_START_RSVD(lic_date)     ((lic_date)->start_rsvd)

#define LIC_DATE_END_YEAR(lic_date)       ((lic_date)->end_year)
#define LIC_DATE_END_MONTH(lic_date)      ((lic_date)->end_month)
#define LIC_DATE_END_DAY(lic_date)        ((lic_date)->end_day)
#define LIC_DATE_END_RSVD(lic_date)       ((lic_date)->end_rsvd)

typedef struct
{
    UINT8       version[LIC_VERSION_MAX_SIZE];
    LIC_MAC     lic_mac;

    LIC_DATE    lic_date;

    UINT8       user_name   [LIC_USER_NAME_MAX_SIZE   ];
    UINT8       user_email  [LIC_USER_EMAIL_MAX_SIZE  ];
    UINT8       vendor_name [LIC_VENDOR_NAME_MAX_SIZE ];
    UINT8       vendor_email[LIC_VENDOR_EMAIL_MAX_SIZE];

    BIGINT         ecc_private_key;
    ECC_SIGNATURE  ecc_signature;
}LIC_CFG;

#define LIC_CFG_MSG_LEN        (sizeof(LIC_CFG) /*- sizeof(BIGINT)*/ - sizeof(ECC_SIGNATURE))

#define LIC_CFG_VERSION(lic_cfg)        ((lic_cfg)->version)
#define LIC_CFG_MAC(lic_cfg)            (&((lic_cfg)->lic_mac))

#define LIC_CFG_DATE(lic_cfg)           (&((lic_cfg)->lic_date))

#define LIC_CFG_USER_NAME(lic_cfg)      ((lic_cfg)->user_name)
#define LIC_CFG_USER_EMAIL(lic_cfg)     ((lic_cfg)->user_email)

#define LIC_CFG_VENDOR_NAME(lic_cfg)    ((lic_cfg)->vendor_name)
#define LIC_CFG_VENDOR_EMAIL(lic_cfg)   ((lic_cfg)->vendor_email)

#define LIC_CFG_PRIVATE_KEY(lic_cfg)    (&((lic_cfg)->ecc_private_key))
#define LIC_CFG_SIGNATURE(lic_cfg)      (&((lic_cfg)->ecc_signature))


const char *lic_version();

const char *lic_vendor_name();

const char *lic_vendor_email();

EC_BOOL lic_version_verify(const char *version);

EC_BOOL lic_vendor_name_verify(const char *vendor_name);

EC_BOOL lic_vendor_email_verify(const char *vendor_email);

EC_BOOL lic_vendor_verify(const char *vendor_name, const char *vendor_email);

EC_BOOL lic_buff_load(int fd, UINT32 *offset, const RWSIZE rsize, UINT8 *buff);

EC_BOOL lic_buff_flush(int fd, UINT32 *offset, const RWSIZE wsize, const UINT8 *buff);

EC_BOOL lic_mac_collect(LIC_MAC *lic_mac_tbl, const UINT32 lic_mac_tbl_max_size, UINT32 *lic_mac_num);

EC_BOOL lic_mac_cmp(const LIC_MAC *lic_mac_1st, const LIC_MAC *lic_mac_2nd);

EC_BOOL lic_mac_exist(const LIC_MAC *lic_mac_tbl, const UINT32 lic_mac_num, const LIC_MAC *lic_mac);

EC_BOOL lic_mac_check(const LIC_MAC *lic_mac);

EC_BOOL lic_mac_verify(const LIC_MAC *lic_mac);

EC_BOOL lic_mac_init(LIC_MAC *lic_mac);

EC_BOOL lic_mac_make(LIC_MAC *lic_mac, const char *mac_str);

void    lic_mac_print(LOG *log, const LIC_MAC *lic_mac);

EC_BOOL lic_chars_init(UINT8 *str, const UINT32 len);

EC_BOOL lic_chars_make(UINT8 *des_str, const UINT32 des_max_size, const char *src_str);

EC_BOOL lic_date_init(LIC_DATE *lic_date);

EC_BOOL lic_date_clean(LIC_DATE *lic_date);

EC_BOOL lic_date_check(const LIC_DATE *lic_date);

EC_BOOL lic_date_make(LIC_DATE *lic_date, const char *start_date_str, const char *end_date_str);

void    lic_date_print(LOG *log, const LIC_DATE *lic_date);

EC_BOOL lic_prikey_make(BIGINT *prikey);

void    lic_prikey_print(LOG *log, const BIGINT *prikey);

void    lic_signature_print(LOG *log, const ECC_SIGNATURE *ecc_signature);

EC_BOOL lic_cfg_init(LIC_CFG *lic_cfg);

EC_BOOL lic_cfg_clean(LIC_CFG *lic_cfg);

EC_BOOL lic_cfg_load(int fd, LIC_CFG *lic_cfg);

EC_BOOL lic_cfg_flush(int fd, const LIC_CFG *lic_cfg);

EC_BOOL lic_cfg_make(LIC_CFG *lic_cfg, const char *mac_str,
                            const char *start_date_str, const char *end_date_str,
                            const char *user_name_str, const char *user_email_str);

EC_BOOL lic_cfg_signate(LIC_CFG *lic_cfg);

EC_BOOL lic_cfg_verify(const LIC_CFG *lic_cfg);

void   lic_cfg_print(LOG *log, const LIC_CFG *lic_cfg);

EC_BOOL lic_make(const char *mac_str,
                    const char *start_date_str, const char *end_date_str,
                    const char *user_name_str, const char *user_email_str);

EC_BOOL lic_check();

void lic_print(LOG *log);

#endif/* _LICENSE_H */

#ifdef __cplusplus
}
#endif/*__cplusplus*/
