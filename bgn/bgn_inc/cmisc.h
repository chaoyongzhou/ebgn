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

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <arpa/inet.h>

#include "type.h"
#include "cvector.h"

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

uint32_t c_str_to_uint32(const char *str);

char *c_uint32_to_str(const uint32_t num);

uint16_t c_str_to_uint16(const char *str);

char *c_uint16_to_str(const uint16_t num);

char *c_uint8_to_bin_str(const uint8_t num);

char *c_uint16_to_bin_str(const uint16_t num);

char *c_uint32_to_bin_str(const uint32_t num);

char *c_word_to_bin_str(const word_t num);

uint64_t c_str_to_uint64(const char *str);

char *c_inet_ntos(const struct in_addr in);

EC_BOOL c_inet_ston(const char *ipv4_str, struct in_addr *in);

char *  c_uint32_ntos(const uint32_t ipv4);

uint32_t c_uint32_ston(const char *ipv4_str);

UINT32 c_port_to_word(const char *port_str);

char *c_word_to_port(const UINT32 port_num);

/*note: subnet_mask is in host order*/
uint32_t ipv4_subnet_mask_prefix(uint32_t subnet_mask);

void str_to_lower(UINT8 *mac_str, UINT32 len);

char *mac_addr_to_str(const UINT8 *mac);

EC_BOOL str_to_mac_addr(const char *mac_str, UINT8 *mac_addr);

UINT32 str_to_switch(const char *str);

char *switch_to_str(const UINT32 switch_choice);

UINT32 c_str_split (char *string, const char *delim, char **fields, const UINT32 size);

char *c_str_join(const char *delim, const char **fields, const UINT32 size);

char *c_str_cat(const char *src_str_1st, const char *src_str_2nd);

char *c_str_dup(const char *str);

EC_BOOL c_str_is_in(const char *string, const char *delim, const char *tags_str);

char *c_str_fetch_line(char *str);

char *c_str_fetch_next_line(char *str);

char *c_str_move_next(char *str);

char *uint32_vec_to_str(const CVECTOR *uint32_vec);

char *bytes_to_hex_str(const UINT8 *bytes, const UINT32 len);

EC_BOOL hex_str_to_bytes(const char *str, UINT8 **bytes, UINT32 *len);

char   *c_dirname(const char *path_name);

EC_BOOL c_dir_create(const char *dir_name);

EC_BOOL c_basedir_create(const char *file_name);

EC_BOOL exec_shell(const char *cmd_str, char *cmd_output, const UINT32 max_size);

EC_BOOL c_file_flush(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 *buff);

EC_BOOL c_file_pad(int fd, UINT32 *offset, const UINT32 wsize, const UINT8 ch);

EC_BOOL c_file_load(int fd, UINT32 *offset, const UINT32 rsize, UINT8 *buff);

EC_BOOL c_file_size(int fd, UINT32 *fsize);

EC_BOOL c_file_access(const char *pathname, int mode);

EC_BOOL c_file_truncate(int fd, const UINT32 fsize);

EC_BOOL c_file_unlink(const char *filename);

int mem_ncmp(const UINT8 *src, const UINT32 slen, const UINT8 *des, const UINT32 dlen);

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

int c_file_open(const char *pathname, const int flags, const mode_t mode);

int c_file_close(int fd);

struct tm *c_localtime_r(const time_t *timestamp);

ctime_t c_time();

EC_BOOL c_usleep(const UINT32 msec);
EC_BOOL c_sleep(const UINT32 nsec);

EC_BOOL c_checker_default(const void * retval);

void c_mutex_print(pthread_mutex_t *mutex);

pthread_mutex_t *c_mutex_new(const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_init(pthread_mutex_t *mutex, const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_clean(pthread_mutex_t *mutex, const UINT32 location);

void c_mutex_free(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_lock(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_unlock(pthread_mutex_t *mutex, const UINT32 location);



#endif /*_CMISC_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

