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
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>

#include <libgen.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <arpa/inet.h>

#include <errno.h>

#include "type.h"

#include "mm.h"
#include "log.h"

#include "char2int.h"

#include "ccode.h"

#define CHAR2INT_BUFF_NUM ((UINT32) 256)
#define CHAR2INT_BUFF_LEN ((UINT32)  32)

#define CHAR2INT_TM_NUM   ((UINT32) 256)

#define CHAR2INT_CMD_OUTPUT_LINE_MAX_SIZE       ((UINT32) 1 * 1024 * 1024)

#define CHAR2INT_WRITE_ONCE_MAX_BYTES           ((UINT32)0x7FFFF000)/*2GB - 4KB*/
#define CHAR2INT_READ_ONCE_MAX_BYTES            ((UINT32)0x7FFFF000)/*2GB - 4KB*/

#define CHAR2INT_ERR_SEEK                       (-1)


/*NOTE: use naked/original mutex BUT NOT CMUTEX*/
/*to avoid scenario such as sys_log need c_localtime_r but c_localtime_r need mutex*/
static pthread_mutex_t g_char2int_str_cmutex;
static char g_str_buff[CHAR2INT_BUFF_NUM][CHAR2INT_BUFF_LEN];
static UINT32 g_str_idx = 0;

static pthread_mutex_t g_char2int_tm_cmutex;
static struct tm g_tm_tbl[CHAR2INT_TM_NUM];
static UINT32 g_tm_idx = 0;

static char  g_hex_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static UINT8 g_char_hex[] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*  0 -  15*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/* 16 -  31*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/* 32 -  47*/
     0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,/* 48 -  63*/
    -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,/* 64 -  79*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/* 80 -  95*/
    -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,/* 96 - 111*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*112 - 127*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*128 - 143*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*144 - 159*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*160 - 175*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*176 - 191*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*192 - 207*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*208 - 223*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*224 - 239*/
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,/*240 - 255*/
};

EC_BOOL char2int_init(UINT32 location)
{
    c_mutex_init(&g_char2int_str_cmutex, CMUTEX_PROCESS_PRIVATE, location);
    c_mutex_init(&g_char2int_tm_cmutex, CMUTEX_PROCESS_PRIVATE, location);
    return (EC_TRUE);
}

UINT32 chars_to_uint32(const char *chars, const UINT32 len)
{
    UINT32  c;            /* current char */
    UINT32  total;        /* current total */
    UINT32  pos;
    UINT32  negs;

    if( NULL_PTR == chars)
    {
        return ((UINT32)0);
    }

    total = 0;
    negs  = 1;

    for(pos = 0; pos < len && '\0' != chars[ pos ]; pos ++)
    {
        if(0 == pos && '-' == chars[ pos ])
        {
            negs *= ((UINT32)-1);
            continue;
        }

        c = (UINT32)(chars[ pos ]);

        if(c < '0' || c > '9')
        {
            sys_log(LOGSTDERR, "error:str_to_uint32: str %.*s found not digit char at pos %ld\n", len, chars, pos);
            return ((UINT32)0);
        }
        total = 10 * total + (c - '0');
    }
    return (total * negs);
}

UINT32 str_to_uint32(const char *str)
{
    UINT32  c;            /* current char */
    UINT32  total;        /* current total */
    UINT32  pos;
    UINT32  negs;

    if( NULL_PTR == str)
    {
        return ((UINT32)0);
    }

    total = 0;
    negs  = 1;

    for(pos = 0; pos < strlen(str); pos ++)
    {
        if(0 == pos && '-' == str[ pos ])
        {
            negs *= ((UINT32)-1);
            continue;
        }

        c = (UINT32)(str[ pos ]);

        if(c < '0' || c > '9')
        {
            sys_log(LOGSTDERR, "error:str_to_uint32: str %s found not digit char at pos %ld\n", str, pos);
            return ((UINT32)0);
        }
        total = 10 * total + (c - '0');
    }
    return (total * negs);
}

char *uint32_to_str(const UINT32 num)
{
    char *str_cache;

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0001);
    str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0002);

    snprintf(str_cache, CHAR2INT_BUFF_LEN, "%ld", num);

    return (str_cache);
}

char *uint32_to_hex_str(const UINT32 num)
{
    char *str_cache;

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0003);
    str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0004);

    snprintf(str_cache, CHAR2INT_BUFF_LEN, "%lx", num);

    return (str_cache);
}

UINT32 xmlchar_to_uint32(const xmlChar *xmlchar)
{
    return str_to_uint32((const char *)xmlchar);
}

/*return host order*/
UINT32 ipv4_to_uint32(const char *ipv4_str)
{
    /*network order is big endian*/
    /*network order to host order. e.g., "1.2.3.4" -> 0x01020304*/

    UINT32 a,b,c,d;
    sscanf(ipv4_str, "%ld.%ld.%ld.%ld",&a, &b, &c, &d);

    return ((a << 24) | (b << 16) | (c << 8) | (d));
    //return (ntohl(inet_addr(ipv4_str)));
}


/*ipv4_num is host order*/
char *uint32_to_ipv4(const UINT32 ipv4_num)
{
    char *ipv4_str_cache;
    UINT32 a,b,c,d;

    a = ((ipv4_num >> 24) & 0xFF);/*high bits*/
    b = ((ipv4_num >> 16) & 0xFF);
    c = ((ipv4_num >>  8) & 0xFF);
    d = ((ipv4_num >>  0) & 0xFF);/*low bits*/

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0005);
    ipv4_str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0006);

    snprintf(ipv4_str_cache, CHAR2INT_BUFF_LEN, "%ld.%ld.%ld.%ld", a, b, c, d);
    return (ipv4_str_cache);
}

char *inet_ntos(const struct in_addr in)
{
    char *ipv4_str_cache;

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0007);
    ipv4_str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0008);

    inet_ntop(AF_INET, &in, ipv4_str_cache, CHAR2INT_BUFF_LEN);
    return (ipv4_str_cache);
}

/*
   inet_aton()  converts  the  Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order) and stores it in the structure
   that inp points to.  inet_aton() returns non-zero if the address is valid, zero if not.  The address supplied in cp can have one of the following forms:

   a.b.c.d   Each of the four numeric parts specifies a byte of the address; the bytes are assigned in left-to-right order to produce the binary address.

   a.b.c     Parts a and b specify the first two bytes of the binary address.  Part c is interpreted as a 16-bit value that defines the  rightmost  two  bytes  of  the
             binary address.  This notation is suitable for specifying (outmoded) Class B network addresses.

   a.b       Part  a  specifies  the  first  byte  of the binary address.  Part b is interpreted as a 24-bit value that defines the rightmost three bytes of the binary
             address.  This notation is suitable for specifying (outmoded) Class C network addresses.

   a         The value a is interpreted as a 32-bit value that is stored directly into the binary address without any byte rearrangement.

   In all of the above forms, components of the dotted address can be specified in decimal, octal (with a leading 0), or hexadecimal, with a leading 0X).  Addresses in
   any  of  these  forms are collectively termed IPV4 numbers-and-dots notation.  The form that uses exactly four decimal numbers is referred to as IPv4 dotted-decimal
   notation (or sometimes: IPv4 dotted-quad notation).
*/
EC_BOOL inet_ston(const char *ipv4_str, struct in_addr *in)
{
    if(inet_aton(ipv4_str, in))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}


char *uint32_t_ntos(const uint32_t ipv4)
{
    struct in_addr in;
    in.s_addr = ipv4;
    return inet_ntos(in);
}

uint32_t  uint32_t_ston(const char *ipv4_str)
{
    struct in_addr in;
    if(EC_FALSE == inet_ston(ipv4_str, &in))
    {
        return (0);
    }
    return (in.s_addr);
}

UINT32 port_to_uint32(const char *port_str)
{
    return ((UINT32)(atoi(port_str)));
}

char *uint32_to_port(const UINT32 port_num)
{
    char *port_str_cache;

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0009);
    port_str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0010);

    snprintf(port_str_cache, CHAR2INT_BUFF_LEN, "%ld", port_num);

    return (port_str_cache);
}

/*note: subnet_mask is in host order*/
uint32_t ipv4_subnet_mask_prefix(uint32_t subnet_mask)
{
    uint32_t e;
    uint32_t prefix;

    if(0 == subnet_mask)
    {
        return ((uint32_t)0);
    }

    for(prefix = 32, e = 1; 0 == (e & subnet_mask); prefix --, e <<= 1)
    {
        /*do nothing*/
    }
    return (prefix);
}

void str_to_lower(UINT8 *mac_str, const UINT32 len)
{
    UINT32 pos;
    for(pos = 0; pos < len; pos ++)
    {
        if('A' <= (mac_str[ pos ]) && (mac_str[ pos ]) <= 'Z')
        {
            mac_str[ pos ] = (mac_str[ pos ] - 'A' + 'a');
        }
    }
    return;
}

char *mac_addr_to_str(const UINT8 *mac)
{
    char *mac_str_cache;

    c_mutex_lock(&g_char2int_str_cmutex, LOC_CHAR2INT_0011);
    mac_str_cache = (char *)(g_str_buff[g_str_idx]);
    g_str_idx = ((g_str_idx + 1) % (CHAR2INT_BUFF_NUM));
    c_mutex_unlock(&g_char2int_str_cmutex, LOC_CHAR2INT_0012);

    snprintf(mac_str_cache, CHAR2INT_BUFF_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return (mac_str_cache);
}

EC_BOOL str_to_mac_addr(const char *mac_str, UINT8 *mac_addr)
{
    char mac_str_tmp[32];
    char *fields[16];
    UINT32 len;
    UINT32 pos;

    len = DMIN(32, strlen(mac_str) + 1);
    sys_log(LOGSTDNULL, "[DEBUG] str_to_mac_addr: len %ld\n", len);

    BCOPY(mac_str, mac_str_tmp, len);
    if(6 != str_split(mac_str_tmp, ":", fields, sizeof(fields)/sizeof(fields[0])))
    {
        sys_log(LOGSTDNULL, "error:str_to_mac_addr:invalid mac addr %s\n", mac_str);
        return (EC_FALSE);
    }

    for(pos = 0; pos < 6; pos ++)
    {
        mac_addr[ pos ] = (UINT8)(0xff & strtol(fields[ pos ], NULL_PTR, 16));
    }
    return (EC_TRUE);
}

UINT32 str_to_switch(const char *str)
{
    if(0 == strcasecmp(str, (char *)"SWITCH_ON") || 0 == strcasecmp(str, (char *)"ON"))
    {
        return (SWITCH_ON);
    }
    return (SWITCH_OFF);
}

char *switch_to_str(const UINT32 switch_choice)
{
    if(SWITCH_ON == switch_choice)
    {
        return (char *)"SWITCH_ON";
    }
    if(SWITCH_OFF == switch_choice)
    {
        return (char *)"SWITCH_OFF";
    }

    return (char *)"SWITCH_UNKNOWN";
}

UINT32 str_split (char *string, const char *delim, char **fields, const UINT32 size)
{
    UINT32 idx;
    char *saveptr;

    idx = 0;
    saveptr = string;
    while ((fields[ idx ] = strtok_r(NULL_PTR, delim, &saveptr)) != NULL_PTR)
    {
        idx ++;

        if (idx >= size)
        {
            break;
        }
    }

    return (idx);
}

char *str_join(const char *delim, const char **fields, const UINT32 size)
{
    UINT32 total_len;
    UINT32 delim_len;
    UINT32 idx;
    char  *ptr;
    char  *str;

    if( 0 == size)
    {
        return (NULL_PTR);
    }

    delim_len = strlen(delim);

    for(idx = 0, total_len = 0; idx < size; idx ++)
    {
        total_len += strlen(fields[ idx ]);
    }

    total_len += delim_len * (size - 1);
    total_len ++; /*string terminate char*/

    str = (char *)safe_malloc(total_len, LOC_CHAR2INT_0013);
    if(NULL_PTR == str)
    {
        sys_log(LOGSTDOUT, "error:str_join: malloc str with len %ld failed\n", total_len);
        return (NULL_PTR);
    }

    BSET(str, 0, total_len);

    ptr = str;
    for(idx = 0; idx < size; idx ++)
    {
        UINT32 cur_len;

        cur_len = strlen(fields[ idx ]);

        BCOPY(fields[ idx ], ptr, cur_len);
        ptr += cur_len;

        if(idx + 1 < size)
        {
            BCOPY(delim, ptr, delim_len);
            ptr += delim_len;
        }
    }

    (*ptr) = '\0';

    return (str);
}

char *str_dup(const char *str)
{
    char *dup_str;
    
    dup_str = (char *)safe_malloc(strlen(str) + 1, LOC_CHAR2INT_0014);
    if(NULL_PTR == dup_str)
    {
        sys_log(LOGSTDOUT, "error:str_dup: dup str %s failed\n", str);
        return (NULL_PTR);
    }
    BCOPY(str, dup_str, strlen(str) + 1);
    return (dup_str);
}

EC_BOOL str_is_in(const char *string, const char *delim, const char *tags_str)
{
    char *str_tmp;
    char *tag[32];
    UINT32 tag_num;
    UINT32 tag_idx;

    if(NULL_PTR == string)
    {
        return (EC_FALSE);
    }
    
    str_tmp = str_dup(tags_str);
    if(NULL_PTR == str_tmp)
    {
        sys_log(LOGSTDOUT, "error:str_is_in: dup %s failed\n", tags_str);
        return (EC_FALSE);
    }
    tag_num = str_split(str_tmp, delim, tag, sizeof(tag)/sizeof(tag[0]));

    for(tag_idx = 0; tag_idx < tag_num; tag_idx ++)
    {
        if(0 == strcasecmp(string, tag[ tag_idx ]))
        {
            safe_free(str_tmp, LOC_CHAR2INT_0015);
            return (EC_TRUE);
        }
    }
    safe_free(str_tmp, LOC_CHAR2INT_0016);
    return (EC_FALSE);
}

char *str_fetch_line(char *str)
{
    char *pch;

    if('\0' == *str)
    {
        return (NULL_PTR);
    }

    for(pch = str; '\0' != (*pch); pch ++)
    {
        if('\n' == (*pch))
        {
            (*pch) = '\0';
            break;
        }
    }
    return (str);
}

char *str_move_next(char *str)
{
    return (str + strlen(str) + 1);
}

char *uint32_vec_to_str(const CVECTOR *uint32_vec)
{
    char  *buff;
    UINT32 len;
    UINT32 char_pos;
    
    UINT32 beg_pos;
    UINT32 cur_pos;
    
    if(EC_TRUE == cvector_is_empty(uint32_vec))
    {
        return (NULL_PTR);
    }

    len = 1 * 1024;
    buff = (char *)safe_malloc(len, LOC_CHAR2INT_0017);
    if(NULL_PTR == buff)
    {
        sys_log(LOGSTDOUT, "error:uint32_vec_to_str: malloc %ld bytes failed\n", len);
        return (NULL_PTR);
    }

    char_pos = 0;

    CVECTOR_LOCK(uint32_vec, LOC_CHAR2INT_0018);
    for(beg_pos = 0; beg_pos < cvector_size(uint32_vec); beg_pos ++)
    {
        UINT32 beg_num;
        UINT32 end_num;
        
        beg_num = (UINT32)cvector_get_no_lock(uint32_vec, beg_pos);
        end_num = beg_num;

        for(cur_pos = beg_pos + 1; cur_pos < cvector_size(uint32_vec); cur_pos ++)
        {
            UINT32 cur_num;
            cur_num = (UINT32)cvector_get_no_lock(uint32_vec, cur_pos);

            if(end_num + 1 == cur_num)
            {
                end_num = cur_num;
            }
            else
            {
                break;/*terminate inner loop*/
            }            
        }

        /*okay, we obtain the [beg_num, end_num]*/
        if(0 < char_pos)
        {
            char_pos += snprintf(buff + char_pos, len - char_pos, ",");
        }

        if(beg_num == end_num)
        {
            char_pos += snprintf(buff + char_pos, len - char_pos, "%ld", beg_num);
        }
        else if(beg_num + 1 == end_num)
        {
            char_pos += snprintf(buff + char_pos, len - char_pos, "%ld,%ld", beg_num, end_num);
        }        
        else
        {
            char_pos += snprintf(buff + char_pos, len - char_pos, "%ld-%ld", beg_num, end_num);
        }

        /*move forward*/
        beg_pos = cur_pos;
    }
    CVECTOR_UNLOCK(uint32_vec, LOC_CHAR2INT_0019);

    return (buff);
}

char *bytes_to_hex_str(const UINT8 *bytes, const UINT32 len)
{    
    char *str;
    UINT32 byte_pos;
    UINT32 char_pos;

    str = (char *)safe_malloc(2 * len + 1, LOC_CHAR2INT_0020);
    if(NULL_PTR == str)
    {
        sys_log(LOGSTDOUT, "error:bytes_to_hex_str: malloc %ld bytes failed\n", 2 * len + 1);
        return (NULL_PTR);
    }

    char_pos = 0;
    for(byte_pos = 0; byte_pos < len; byte_pos ++)
    {
        str[ char_pos ++ ] = g_hex_char[(bytes[ byte_pos ] >> 4) & 0xF];/*high 4 bytes*/
        str[ char_pos ++ ] = g_hex_char[(bytes[ byte_pos ]) & 0xF];/*high 4 bytes*/
    }
    str[ char_pos ] = '\0';

    return (str);
}

EC_BOOL hex_str_to_bytes(const char *str, UINT8 **bytes, UINT32 *len)
{
    UINT32 byte_pos;
    UINT32 char_pos;
    UINT32 str_len;
    UINT32 bytes_len;
    UINT8  *buff;

    str_len = strlen(str);

    if(str_len & 1)/*should never be odd*/
    {
        sys_log(LOGSTDOUT, "error:hex_str_to_bytes: str len %ld, but it should never is odd!\n", str_len);
        return (EC_FALSE);
    }

    bytes_len = (str_len >> 1);
    buff = (UINT8 *)safe_malloc(bytes_len, LOC_CHAR2INT_0021);
    if(NULL_PTR == buff)
    {
        sys_log(LOGSTDOUT, "error:hex_str_to_bytes: malloc %ld bytes failed\n", bytes_len);
        return (EC_FALSE);
    }

    char_pos = 0;
    for(byte_pos = 0; byte_pos < bytes_len; byte_pos ++)
    {
        UINT8 hi;
        UINT8 lo;
        UINT8 des;

        hi = g_char_hex[ (UINT8)(str[ char_pos ++ ]) ];
        lo = g_char_hex[ (UINT8)(str[ char_pos ++ ]) ];

        //ASSERT((UINT8)-1 != hi);
        //ASSERT((UINT8)-1 != lo);

        des = ((hi << 4) | lo);
        buff[ byte_pos ] = des;
    }

    (*bytes) = buff;
    (*len)   = bytes_len;
    return (EC_TRUE);
}

EC_BOOL create_dir(const char *dir_name)
{
    char *pstr;

    int   len;
    int   pos;

    if(0 == access(dir_name, F_OK))/*exist*/
    {
        return (EC_TRUE);
    }

    pstr = strdup(dir_name);
    if(NULL_PTR == pstr)
    {
        sys_log(LOGSTDOUT, "error:create_dir: strdup failed\n");
        return (EC_FALSE);
    }

    len  = strlen(pstr);

    for(pos = 1; pos < len; pos ++)
    {
        UINT32 loop;

        if('/' != pstr[ pos ])
        {
            continue;
        }

        pstr[ pos ] = '\0';

        for(loop = 0; loop < 3 && 0 != access(pstr, F_OK) && 0 != mkdir(pstr, 0755); loop ++)/*try 3 times*/
        {
            /*do nothing*/
        }

        if(3 <= loop)
        {
            sys_log(LOGSTDOUT, "error:create_dir: create dir %s failed\n", pstr);
            pstr[ pos ] = '/';

            free(pstr);
            return (EC_FALSE);
        }
        pstr[ pos ] = '/';
    }

    if(0 != access(dir_name, F_OK) && 0 != mkdir(dir_name, 0755))
    {
        sys_log(LOGSTDOUT, "error:create_dir: create dir %s failed\n", dir_name);
        free(pstr);
        return (EC_FALSE);
    }

    free(pstr);
    return (EC_TRUE);
}

EC_BOOL create_basedir(const char *file_name)
{
    char *dir_name;
    EC_BOOL ret;

    dir_name = str_dup(file_name);
    dir_name = dirname(dir_name);
    ret = create_dir(dir_name);
    safe_free(dir_name, LOC_CHAR2INT_0022);
    return (ret);
}

char *dup_filename(const char *file_name)
{
    char *duped_file_name;
    duped_file_name = (char *)SAFE_MALLOC(strlen(file_name) + 1, LOC_CHAR2INT_0023);
    if(NULL_PTR == duped_file_name)
    {
        sys_log(LOGSTDOUT, "error:dup_filename: dup file name %s failed\n", file_name);
        return (NULL_PTR);
    }
    sprintf(duped_file_name, "%s", file_name);
    return (duped_file_name);
}

EC_BOOL exec_shell(const char *cmd_str, char *cmd_output, const UINT32 max_size)
{
    FILE    *rstream;
    char    *cmd_ostream;
    char    *cmd_ostr;
    UINT32   cmd_osize;

    //sys_log(LOGSTDNULL, "exec_shell beg: %s\n", cmd_str);

    if(NULL_PTR == cmd_output)
    {
        cmd_osize   = CHAR2INT_CMD_OUTPUT_LINE_MAX_SIZE;
        cmd_ostream = (char *)SAFE_MALLOC(cmd_osize, LOC_CHAR2INT_0024);
    }
    else
    {
        cmd_osize   = max_size;
        cmd_ostream = cmd_output;
    }

    rstream = popen(cmd_str, "r");
    if(NULL_PTR == rstream)
    {
        sys_log(LOGSTDOUT, "error:exec_shell: popen %s failed\n", cmd_str);
        return (EC_FALSE);
    }
    for(cmd_ostr = cmd_ostream;
        1 < cmd_osize && NULL_PTR != (cmd_ostr = fgets(cmd_ostr, cmd_osize, rstream));
        cmd_osize -= strlen(cmd_ostr), cmd_ostr += strlen(cmd_ostr))
    {
        /*do nothing*/
    }
    pclose( rstream );

    if(cmd_ostream != cmd_output)
    {
        SAFE_FREE(cmd_ostream, LOC_CHAR2INT_0025);
    }
    //sys_log(LOGSTDNULL, "exec_shell end: %s\n", cmd_output);
    return (EC_TRUE);
}

EC_BOOL file_buff_flush(int fd, const UINT32 offset, const UINT32 wsize, const UINT8 *buff)
{
    UINT32 csize;/*write completed size*/
    UINT32 osize;/*write once size*/

    if(CHAR2INT_ERR_SEEK == lseek(fd, offset, SEEK_SET))
    {
        sys_log(LOGSTDOUT, "error:file_buff_flush: seek offset %ld failed\n", offset);
        return (EC_FALSE);
    }

    for(csize = 0, osize = CHAR2INT_WRITE_ONCE_MAX_BYTES; csize < wsize; csize += osize)
    {
        if(csize + osize > wsize)
        {
            osize = wsize - csize;
        }

        if((ssize_t)osize != write(fd, buff + csize, osize))
        {
            sys_log(LOGSTDOUT, "error:file_buff_flush: flush buff to offset %ld failed where wsize %ld, csize %ld, osize %ld, errno %d, errstr %s\n",
                                offset, wsize, csize, osize, errno, strerror(errno));
            return (EC_FALSE);
        }
    }

    return (EC_TRUE);
}

EC_BOOL file_buff_load(int fd, const UINT32 offset, const UINT32 rsize, UINT8 *buff)
{
    UINT32 csize;/*read completed size*/
    UINT32 osize;/*read once size*/

    if(CHAR2INT_ERR_SEEK == lseek(fd, offset, SEEK_SET))
    {
        sys_log(LOGSTDOUT, "error:file_buff_load: seek offset %ld failed\n", offset);
        return (EC_FALSE);
    }

    for(csize = 0, osize = CHAR2INT_READ_ONCE_MAX_BYTES; csize < rsize; csize += osize)
    {
        if(csize + osize > rsize)
        {
            osize = rsize - csize;
        }

        if((ssize_t)osize != read(fd, buff + csize, osize))
        {
            sys_log(LOGSTDOUT, "error:file_buff_load: load buff from offset %ld failed where rsize %ld, csize %ld, osize %ld, errno %d, errstr %s\n",
                                offset, rsize, csize, osize, errno, strerror(errno));
            return (EC_FALSE);
        }
    }

    //sys_log(LOGSTDOUT, "cdfsnp_buff_load: load %ld bytes\n", rsize);
    return (EC_TRUE);
}

EC_BOOL file_get_size(int fd, UINT32 *fsize)
{
    (*fsize) = lseek(fd, 0, SEEK_END);
    if(((UINT32)-1) == (*fsize))
    {
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL file_truncate(int fd, const UINT32 fsize)
{
    if(0 != ftruncate(fd, fsize))
    {
        sys_log(LOGSTDOUT, "error:file_truncate: truncate %ld bytes failed\n", fsize);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

int mem_ncmp(const UINT8 *src, const UINT32 slen, const UINT8 *des, const UINT32 dlen)
{
    UINT32 len;

    int result;

    len = DMIN(slen, dlen);
    result = BCMP(src, des, len);
    if(0 != result)
    {
        return (result);
    }

    if(slen < dlen)
    {
        return (-1);
    }

    if(slen > dlen)
    {
        return (1);
    }
    return (0);
}

void ident_print(LOG *log, const UINT32 level)
{
    UINT32 idx;

    for(idx = 0; idx < level; idx ++)
    {
        sys_print(log, "    ");
    }
    return;
}

int c_open(const char *pathname, const int flags, const mode_t mode)
{
    int fd;

    if(flags & O_CREAT)
    {
        if(EC_FALSE == create_basedir(pathname))
        {
            sys_log(LOGSTDOUT, "error:c_open: create basedir of file %s failed\n", pathname);
            return (-1);
        }
    }

    fd = open(pathname, flags, mode);
    if(-1 == fd)
    {
        sys_log(LOGSTDOUT,"error:c_open: open %s failed\n", pathname);
        return (-1);
    }

    if(1)
    {
        if( 0 > fcntl(fd, F_SETFD, FD_CLOEXEC))
        {
            sys_log(LOGSTDOUT, "error:c_open: set fd %d to FD_CLOEXEC failed, errno = %d, errstr = %s\n",
                               fd, errno, strerror(errno));
            close(fd);
            return (-1);
        }
    }
    return (fd);
}

int c_close(int fd)
{
    if(-1 != fd)
    {
        return close(fd);
    }
    return (0);
}

struct tm *c_localtime_r(const time_t *timestamp)
{
    struct tm *ptime;
    c_mutex_lock(&g_char2int_tm_cmutex, LOC_CHAR2INT_0026);
    ptime = &(g_tm_tbl[g_tm_idx]);
    g_tm_idx = ((g_tm_idx + 1) % (CHAR2INT_TM_NUM));
    c_mutex_unlock(&g_char2int_tm_cmutex, LOC_CHAR2INT_0027);

    localtime_r(timestamp, ptime);
    return (ptime);
}

ctime_t c_time()
{
    return (ctime_t)time(NULL_PTR);
}

EC_BOOL c_checker_default(const void * val)
{
    return ((EC_BOOL)val);
}

void c_mutex_print(pthread_mutex_t *mutex)
{
#if 0
    fprintf(stdout, "c_mutex_print: mutex %lx: __m_lock = %d, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d\n",
                    mutex,
                    mutex->__m_lock,
                    mutex->__m_reserved,
                    mutex->__m_count,
                    mutex->__m_owner,
                    mutex->__m_kind
            );
    fflush(stdout);            
#endif    
#if 1
    fprintf(stdout, "c_mutex_print: mutex %lx: __m_lock = %d, __m_reserved = %d, __m_count = %d, __m_owner = %d, __m_kind = %d\n",
                    (UINT32)((void *)mutex),
                    mutex->__data.__lock,
                    mutex->__data.__nusers,
                    mutex->__data.__count,
                    mutex->__data.__owner,
                    mutex->__data.__kind
            );
    fflush(stdout);            
#endif    

    return;
}

pthread_mutex_t *c_mutex_new(const UINT32 flag, const UINT32 location)
{
    pthread_mutex_t *mutex;

    mutex = (pthread_mutex_t *)safe_malloc(sizeof(pthread_mutex_t), location);
    if(NULL_PTR == mutex)
    {
        fprintf(stdout, "error:c_mutex_new: malloc mutex failed, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        fflush(stdout);
        return (NULL_PTR);
    }

    if(EC_FALSE == c_mutex_init(mutex, flag, location))
    {
        fprintf(stdout, "error:c_mutex_new: init mutex failed, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
        fflush(stdout);
        safe_free(mutex, location);
        return (NULL_PTR);
    }

    return (mutex);
}

EC_BOOL c_mutex_clean(pthread_mutex_t *mutex, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_destroy(mutex);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                fprintf(stdout, "error:c_mutex_clean - EINVAL: mutex doesn't refer to an initialized mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EBUSY:
            {
                fprintf(stdout, "error:c_mutex_clean - EBUSY: mutex is locked or in use by another thread, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            default:
            {
                /* Unknown error */
                fprintf(stdout, "error:c_mutex_clean - UNKNOWN: mutex detect error, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

void c_mutex_free(pthread_mutex_t *mutex, const UINT32 location)
{
    if(NULL_PTR != mutex)
    {
        if(EC_FALSE == c_mutex_clean(mutex, location))
        {
            fprintf(stdout, "error:c_mutex_free: clean mutex failed, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
            fflush(stdout);
        }
        safe_free(mutex, location);
    }
    return;
}

EC_BOOL c_mutex_lock(pthread_mutex_t *mutex, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_lock(mutex);
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                fprintf(stdout, "error:c_mutex_lock - EINVAL: mutex NOT an initialized object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EDEADLK:
            {
                fprintf(stdout, "error:c_mutex_lock - EDEADLK: deadlock is detected or current thread already owns the mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case ETIMEDOUT:
            {
                fprintf(stdout, "error:c_mutex_lock - ETIMEDOUT: failed to lock mutex before the specified timeout expired, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EBUSY:
            {
                fprintf(stdout, "error:c_mutex_lock - EBUSY: failed to lock mutex due to busy, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            default:
            {
                fprintf(stdout, "error:c_mutex_lock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL c_mutex_unlock(pthread_mutex_t *mutex, const UINT32 location)
{
    int ret_val;

    ret_val = pthread_mutex_unlock(mutex);
    if(0 != ret_val)
    {
        switch(ret_val)
        {
            case EINVAL:
            {
                fprintf(stdout, "error:c_mutex_unlock - EINVAL: mutex NOT an initialized object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EPERM:
            {
                fprintf(stdout, "error:c_mutex_unlock - EPERM: current thread does not hold a lock on mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            default:
            {
                fprintf(stdout, "error:c_mutex_unlock - UNKNOWN: error detected, errno %d, errstr %s, called at %s:%ld\n", ret_val, strerror(ret_val), MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }
        return (EC_FALSE);
    }

    return (EC_TRUE);
}

EC_BOOL c_mutex_init(pthread_mutex_t *mutex, const UINT32 flag, const UINT32 location)
{
    pthread_mutexattr_t  mutex_attr;
    int ret_val;

    ret_val = pthread_mutexattr_init(&mutex_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case ENOMEM:
            {
                fprintf(stdout, "error:c_mutex_init - ENOMEM: Insufficient memory to create the mutex attributes object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
            default:
            {
                /* Unknown error */
                fprintf(stdout, "error:c_mutex_init - UNKNOWN: Error detected when mutexattr init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }
        return (EC_FALSE);
    }

    if(CMUTEX_PROCESS_PRIVATE == flag)
    {
        ret_val = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_PRIVATE);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    fprintf(stdout, "error:c_mutex_init - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    fflush(stdout);
                    break;
                }

                default:
                {
                    fprintf(stdout, "error:c_mutex_init - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    fflush(stdout);
                    break;
                }
            }
            return (EC_FALSE);
        }
    }

    if(CMUTEX_PROCESS_SHARED == flag)
    {
        ret_val = pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
        if( 0 != ret_val )
        {
            switch( ret_val )
            {
                case EINVAL:
                {
                    fprintf(stdout, "error:c_mutex_init - EINVAL: value specified for argument -pshared- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    fflush(stdout);
                    break;
                }

                default:
                {
                    fprintf(stdout, "error:c_mutex_init - UNKNOWN: error detected when setpshared, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                    fflush(stdout);
                    break;
                }
            }

            return (ret_val);
        }
    }

    /*Initialize the mutex attribute called 'type' to PTHREAD_MUTEX_RECURSIVE_NP,
    so that a thread can recursively lock a mutex if needed. */
    ret_val = pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EINVAL:
            {
                fprintf(stdout, "error:c_mutex_init - EINVAL: value specified for argument -type- is INCORRECT, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            default:
            {
                fprintf(stdout, "error:c_mutex_init - UNKNOWN: error detected when settype, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }
        return (EC_FALSE);
    }

    /* Creating and Initializing the mutex with the above stated mutex attributes */
    ret_val = pthread_mutex_init(mutex, &mutex_attr);
    if( 0 != ret_val )
    {
        switch( ret_val )
        {
            case EAGAIN:
            {
                fprintf(stdout, "error:mutex_new - EAGAIN: System resources(other than memory) are unavailable, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EPERM:
            {
                fprintf(stdout, "error:mutex_new - EPERM: Doesn't have privilige to perform this operation, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EINVAL:
            {
                fprintf(stdout, "error:mutex_new - EINVAL: mutex_attr doesn't refer a valid condition variable attribute object, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case EFAULT:
            {
                fprintf(stdout, "error:mutex_new - EFAULT: Mutex or mutex_attr is an invalid pointer, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            case ENOMEM:
            {
                fprintf(stdout, "error:mutex_new - ENOMEM: Insufficient memory exists to initialize the mutex, called at %s:%ld\n", MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }

            default:
            {
                /* Unknown error */
                fprintf(stdout, "error:mutex_new - UNKNOWN: Error detected when mutex init, error no: %d, called at %s:%ld\n", ret_val, MM_LOC_FILE_NAME(location), MM_LOC_LINE_NO(location));
                fflush(stdout);
                break;
            }
        }

        return (EC_FALSE);
    }

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

