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

#ifndef _CHAR2INT_H
#define _CHAR2INT_H

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

EC_BOOL char2int_init(UINT32 location);

UINT32 chars_to_uint32(const char *chars, const UINT32 len);

UINT32 str_to_uint32(const char *str);

char *uint32_to_str(const UINT32 num);

char *uint32_to_hex_str(const UINT32 num);

UINT32 xmlchar_to_uint32(const xmlChar *xmlchar);

/*return host order*/
UINT32 ipv4_to_uint32(const char *ipv4_str);

/*ipv4_num is host order*/
char *uint32_to_ipv4(const UINT32 ipv4_num);

char *inet_ntos(const struct in_addr in);

EC_BOOL inet_ston(const char *ipv4_str, struct in_addr *in);

char *  uint32_t_ntos(const uint32_t ipv4);

uint32_t uint32_t_ston(const char *ipv4_str);

UINT32 port_to_uint32(const char *port_str);

char *uint32_to_port(const UINT32 port_num);

/*note: subnet_mask is in host order*/
uint32_t ipv4_subnet_mask_prefix(uint32_t subnet_mask);

void str_to_lower(UINT8 *mac_str, UINT32 len);

char *mac_addr_to_str(const UINT8 *mac);

EC_BOOL str_to_mac_addr(const char *mac_str, UINT8 *mac_addr);

UINT32 str_to_switch(const char *str);

char *switch_to_str(const UINT32 switch_choice);

UINT32 str_split (char *string, const char *delim, char **fields, const UINT32 size);

char *str_join(const char *delim, const char **fields, const UINT32 size);

char *str_dup(const char *str);

EC_BOOL str_is_in(const char *string, const char *delim, const char *tags_str);

char *str_fetch_line(char *str);

char *str_move_next(char *str);

char *uint32_vec_to_str(const CVECTOR *uint32_vec);

char *bytes_to_hex_str(const UINT8 *bytes, const UINT32 len);

EC_BOOL hex_str_to_bytes(const char *str, UINT8 **bytes, UINT32 *len);

EC_BOOL create_dir(const char *dir_name);

EC_BOOL create_basedir(const char *file_name);

char *dup_filename(const char *file_name);

EC_BOOL exec_shell(const char *cmd_str, char *cmd_output, const UINT32 max_size);

EC_BOOL file_buff_flush(int fd, const UINT32 offset, const UINT32 wsize, const UINT8 *buff);

EC_BOOL file_buff_load(int fd, const UINT32 offset, const UINT32 rsize, UINT8 *buff);

EC_BOOL file_get_size(int fd, UINT32 *fsize);

EC_BOOL file_truncate(int fd, const UINT32 fsize);

int mem_ncmp(const UINT8 *src, const UINT32 slen, const UINT8 *des, const UINT32 dlen);

void ident_print(LOG * log, const UINT32 level);

int c_open(const char *pathname, const int flags, const mode_t mode);

int c_close(int fd);

struct tm *c_localtime_r(const time_t *timestamp);

ctime_t c_time();

EC_BOOL c_checker_default(const void * retval);

void c_mutex_print(pthread_mutex_t *mutex);

pthread_mutex_t *c_mutex_new(const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_init(pthread_mutex_t *mutex, const UINT32 flag, const UINT32 location);

EC_BOOL c_mutex_clean(pthread_mutex_t *mutex, const UINT32 location);

void c_mutex_free(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_lock(pthread_mutex_t *mutex, const UINT32 location);

EC_BOOL c_mutex_unlock(pthread_mutex_t *mutex, const UINT32 location);



#endif /*_CHAR2INT_H*/

#ifdef __cplusplus
}
#endif/*__cplusplus*/

