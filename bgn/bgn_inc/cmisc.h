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

#ifndef _CMISC_H
#define _CMISC_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#include <ucontext.h>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <arpa/inet.h>

#include "type.h"
#include "cvector.h"


/*time value of year, month, day, hour, minute, second. tm is CTM type*/
#define TIME_IN_YMDHMS(__tm)    \
        (__tm)->tm_year + 1900, \
        (__tm)->tm_mon + 1, \
        (__tm)->tm_mday, \
        (__tm)->tm_hour, \
        (__tm)->tm_min, \
        (__tm)->tm_sec


#define SWITCH_ON_STR       ((char *)"on")
#define SWITCH_OFF_STR      ((char *)"off")
#define SWITCH_UNDEF_STR    ((char *)"n.a.")

typedef EC_BOOL (*C_RETVAL_CHECKER)(const void *);

EC_BOOL cmisc_init(UINT32 location);

UINT32 c_chars_to_word(const char *chars, const UINT32 len);

UINT32 c_str_to_word(const char *str);

char *c_word_to_str(const UINT32 num);

char *c_word_to_hex_str(const UINT32 num);

UINT32 c_xmlchar_to_word(const xmlChar *xmlchar);

/*return host order*/
UINT32 c_ipv4_to_word(const char *ipv4_str);

/*ipv4_num is host order*/
char *c_word_to_ipv4(const UINT32 ipv4_num);

uint32_t c_chars_to_uint32(const char *str, const uint32_t len);

uint32_t c_str_to_uint32(const char *str);

char *c_uint32_to_str(const uint32_t num);

uint16_t c_str_to_uint16(const char *str);

char *c_uint16_to_str(const uint16_t num);

char *c_uint8_to_bin_str(const uint8_t num);

char *c_uint16_to_bin_str(const uint16_t num);

char *c_uint32_to_bin_str(const uint32_t num);

char *c_word_to_bin_str(const word_t num);

uint64_t c_chars_to_uint64(const char *str, const uint32_t len);

uint64_t c_str_to_uint64(const char *str);

int c_long_to_str_buf(const long num, char *buf);

char *c_inet_ntos(const struct in_addr in);

EC_BOOL c_inet_ston(const char *ipv4_str, struct in_addr *in);

char *  c_uint32_ntos(const uint32_t ipv4);

uint32_t c_uint32_ston(const char *ipv4_str);

UINT32 c_port_to_word(const char *port_str);

char *c_word_to_port(const UINT32 port_num);

/*note: subnet_mask is in host order*/
uint32_t ipv4_subnet_mask_prefix(uint32_t subnet_mask);

EC_BOOL c_check_is_uint8_t(const UINT32 num);

EC_BOOL c_check_is_uint16_t(const UINT32 num);

EC_BOOL c_check_is_uint32_t(const UINT32 num);

void str_to_lower(UINT8 *str, UINT32 len);

void str_to_upper(UINT8 *str, const UINT32 len);

char *mac_addr_to_str(const UINT8 *mac);

EC_BOOL str_to_mac_addr(const char *mac_str, UINT8 *mac_addr);

UINT32 str_to_switch(const char *str);

char *switch_to_str(const UINT32 switch_choice);

UINT32 c_str_split (char *string, const char *delim, char **fields, const UINT32 size);

char *c_str_join(const char *delim, const char **fields, const UINT32 size);

char *c_str_cat(const char *src_str_1st, const char *src_str_2nd);

char *c_str_dup(const char *str);

EC_BOOL c_str_is_in(const char *string, const char *delim, const char *tags_str);

char *c_str_skip_space(const char *start, const char *end);

char *c_str_fetch_line(char *str);

char *c_str_fetch_next_line(char *str);

char *c_str_move_next(char *str);

char *c_str_seperate (char **stringp, const char *delim);

UINT32 c_line_len(const char *str);

char *uint32_vec_to_str(const CVECTOR *uint32_vec);

char *c_bytes_to_hex_str(const UINT8 *bytes, const UINT32 len);

EC_BOOL c_hex_str_to_bytes(const char *str, UINT8 **bytes, UINT32 *len);

char *c_md5_to_hex_str(const uint8_t *md5, char *str, const uint32_t max_len);

char   *c_dirname(const char *path_name);

EC_BOOL c_dir_create(const char *dir_name);

EC_BOOL c_basedir_create(const char *file_name);

EC_BOOL exec_shell(const char *cmd_str, char *cmd_output, const UINT32 max_size);

EC_BOOL c_file_flush(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 *buff);

EC_BOOL c_file_write(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 *buff);

EC_BOOL c_file_pad(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 ch);

EC_BOOL c_file_load(int fd, UINT32 *offset, const UINT32 rsize, UINT8 *buff);

EC_BOOL c_file_read(int fd, UINT32 *offset, const UINT32 rsize, UINT8 *buff);

EC_BOOL c_file_pwrite(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 *buff);

EC_BOOL c_file_ppad(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 ch);

EC_BOOL c_file_pread(int fd, UINT32 *offset, const UINT32 rsize, UINT8 *buff);

EC_BOOL c_file_size(int fd, UINT32 *fsize);

EC_BOOL c_file_size_b(int fd, uint64_t *fsize);

EC_BOOL c_file_pos(int fd, UINT32 *fpos);

EC_BOOL c_file_pos_b(int fd, uint64_t *fpos);

EC_BOOL c_file_access(const char *pathname, int mode);

EC_BOOL c_file_truncate(int fd, const UINT32 fsize);

EC_BOOL c_file_unlink(const char *filename);

int c_mem_ncmp(const UINT8 *src, const UINT32 slen, const UINT8 *des, const UINT32 dlen);

void c_ident_print(LOG * log, const UINT32 level);

void c_usage_print(LOG *log, const char **usage, const int size);

void c_history_init(char **history, const int max, int *size);

void c_history_push(char **history, const int max, int *size, const char *str);

void c_history_clean(char **history, const int max, const int size);

void c_history_print(char **history, const int max, const int size);

void c_uint16_lo2hi_header_print(LOG *log);

void c_uint16_lo2hi_bits_print(LOG *log, const uint16_t num);

void c_uint16_hi2lo_header_print(LOG *log);

void c_uint16_hi2lo_bits_print(LOG *log, const uint16_t num);

void c_buff_print_char(LOG *log, const UINT8 *buff, const UINT32 len);

void c_buff_print_hex(LOG *log, const UINT8 *buff, const UINT32 len);

void c_buff_print_str(LOG *log, const UINT8 *buff, const UINT32 len);

EC_BOOL c_isdigit(int c); 

EC_BOOL c_isxdigit(int c); 

EC_BOOL c_isalpha(int c); 

EC_BOOL c_isalnum(int c); 

EC_BOOL c_memcmp(const uint8_t *s1, const uint8_t *s2, const uint32_t len);

const char *c_bool_str(const EC_BOOL flag);

EC_BOOL c_str_to_bool(const char *str);

int c_file_open(const char *pathname, const int flags, const mode_t mode);

int c_file_close(int fd);

struct tm *c_localtime_r(const time_t *timestamp);

ctime_t c_time(ctime_t *timestamp);

EC_BOOL c_usleep(const UINT32 msec, const UINT32 location);
EC_BOOL c_sleep(const UINT32 nsec, const UINT32 location);

EC_BOOL c_checker_default(const void * retval);

void c_mutex_print(pthread_mutex_t *mutex);

pthread_mutex_t *c_mutex_new(const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_init(pthread_mutex_t *mutex, const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_clean(pthread_mutex_t *mutex, const UINT32 location);

void c_mutex_free(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_lock(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_unlock(pthread_mutex_t *mutex, const UINT32 location);

void c_backtrace_print(LOG *log, ucontext_t *ucontext);

void c_backtrace_dump(LOG *log);

void c_indent_print(LOG *log, const UINT32 level);

#endif /*_CMISC_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

