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

#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include <string.h>
#include "cbytes.h"

#include "type.h"
#include "mm.h"
#include "log.h"
#include "char2int.h"

#include "cmpic.inc"
#include "cstring.h"

CSTRING *cstring_new(const UINT8 *str, const UINT32 location)
{
    CSTRING *cstring;

    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRING, &cstring, location);
    //cstring = (CSTRING *)SAFE_MALLOC(sizeof(CSTRING), location);
    if(cstring)
    {
        cstring->str = NULL_PTR;
        cstring_init(cstring, str);
    }
    return cstring;
}

void cstring_free(CSTRING *cstring)
{
    if(cstring->str)
    {
        SAFE_FREE(cstring->str, LOC_CSTRING_0001);

        cstring->str = (UINT8 *)0;
        cstring->capacity = 0;
        cstring->len      = 0;
    }

    free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CSTRING, cstring, LOC_CSTRING_0002);
    //SAFE_FREE(cstring, LOC_CSTRING_0003);
    return;
}

void cstring_free_0(const UINT32 cstring_md_id, CSTRING *cstring)
{
    cstring_free(cstring);
    return;
}

void cstring_init(CSTRING *cstring, const UINT8 *str)
{
    UINT32 str_len;
    UINT32 pos;

    if(0 == str)
    {
        cstring->str = (UINT8 *)0;
        cstring->len      = 0;
        cstring->capacity = 0;
        return;
    }

    str_len = strlen((char *)str);

    cstring->str = (UINT8 *)SAFE_MALLOC(sizeof(UINT8) * (str_len + 1), LOC_CSTRING_0004);
    if(cstring->str)
    {
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        for(pos = 0; pos <= str_len; pos ++)/*copy the last terminal char of str*/
        {
            cstring->str[ pos ] = str[ pos ];
        }

        cstring->capacity = str_len + 1;
        cstring->len      = str_len;/*the length cover the last terminal char of str*/
        return;
    }

    cstring->capacity = 0;
    cstring->len      = 0;
    return;
}

UINT32 cstring_init_0(const UINT32 cstring_md_id, CSTRING *cstring)
{
    cstring_init(cstring, 0);
    return (0);
}

void cstring_clean(CSTRING *cstring)
{
    if(0 != cstring->capacity)
    {
        SAFE_FREE(cstring->str, LOC_CSTRING_0005);
        cstring->str = (UINT8 *)0;
        cstring->capacity = 0;
        cstring->len      = 0;
    }
    return;
}

UINT32 cstring_clean_0(const UINT32 cstring_md_id, CSTRING *cstring)
{
    cstring_clean(cstring);
    return (0);
}

void cstring_reset(CSTRING *cstring)
{
    cstring->len = 0;
    return;
}

void cstring_clone(const CSTRING *cstring_src, CSTRING *cstring_des)
{
    UINT32 pos;

    cstring_clean(cstring_des);

    if(NULL_PTR == cstring_src || 0 == cstring_src->len)
    {
        return;
    }

    cstring_des->str = (UINT8 *)SAFE_MALLOC(sizeof(UINT8) * (cstring_src->len + 1), LOC_CSTRING_0006);

    for(pos = 0; pos <= cstring_src->len; pos ++)/*clone terminal char*/
    {
        cstring_des->str[ pos ] = cstring_src->str[ pos ];
    }
    cstring_des->capacity = cstring_src->len + 1;
    cstring_des->len      = cstring_src->len;
    return;
}

EC_BOOL cstring_empty(CSTRING *cstring)
{
    cstring->len = 0;
    return (EC_TRUE);
}

EC_BOOL cstring_is_empty(const CSTRING *cstring)
{
    if(NULL_PTR == cstring || 0 == cstring->len)
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cstring_cmp(const CSTRING *cstring_src, const CSTRING *cstring_des)
{
    if(cstring_src == cstring_des)
    {
        return (EC_TRUE);
    }

    if(cstring_src->len != cstring_des->len)
    {
        return (EC_FALSE);
    }

    if(0 == BCMP(cstring_src->str, cstring_des->str, cstring_src->len))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

EC_BOOL cstring_ncmp(const CSTRING *cstring_src, const CSTRING *cstring_des, const UINT32 n)
{
    UINT32 pos;
    UINT32 len_to_cmp;

    if(cstring_src == cstring_des)
    {
        return (EC_TRUE);
    }

    if((cstring_src->len >= n && cstring_des->len < n)
    || (cstring_des->len >= n && cstring_src->len < n))
    {
        return (EC_FALSE);
    }

    if(cstring_src->len >= n && cstring_des->len >= n)
    {
        if(cstring_src->len > cstring_des->len)
        {
            len_to_cmp = cstring_des->len;
        }
        else
        {
            len_to_cmp = cstring_src->len;
        }
    }
    else
    {
        len_to_cmp = n;
    }

    for(pos = 0; pos < len_to_cmp; pos ++)
    {
        if(cstring_src->str[ pos ] != cstring_des->str[ pos ])
        {
            return (EC_FALSE);
        }
    }
    return (EC_TRUE);
}


EC_BOOL cstring_expand(CSTRING *cstring, const UINT32 location)
{
    UINT32 capacity;
    UINT8 *str;

    if(0 == cstring->capacity)
    {
        capacity = CSTRING_MIN_CAPACITY; /*default*/
        str = (UINT8 *)SAFE_MALLOC(capacity, location);
    }
    else
    {
        capacity = 2 * (cstring->capacity);/*double the old capacity*/
        str = (UINT8 *)SAFE_REALLOC(cstring->str, cstring->capacity, capacity, location);
    }

    if(str)
    {
        cstring->str = str;
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        BSET(cstring->str + cstring->capacity, '\0', capacity - cstring->capacity);
#if 0
        for(pos = cstring->capacity; pos < capacity; pos ++)
        {
            cstring->str[ pos ] = '\0';
        }
#endif
        cstring->capacity = capacity;

        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:cstring_expand: failed to expand cstring with capacity %ld and len %ld\n", cstring->capacity, cstring->len);

    return (EC_FALSE);
}

EC_BOOL cstring_expand_to(CSTRING *cstring, const UINT32 size)
{
    UINT32 capacity;
    UINT8 *str;

    capacity = (size < CSTRING_MIN_CAPACITY)? CSTRING_MIN_CAPACITY : size;

    if(capacity <= cstring->capacity)
    {
        /*nothing to do*/
        return (EC_TRUE);
    }

    if(0 == cstring->capacity)
    {
        str = (UINT8 *)SAFE_MALLOC(capacity, LOC_CSTRING_0007);
    }
    else
    {
        str = (UINT8 *)SAFE_REALLOC(cstring->str, cstring->capacity, capacity, LOC_CSTRING_0008);
    }

    if(str)
    {
        //UINT32 pos;
        cstring->str = str;
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        BSET(cstring->str + cstring->capacity, '\0', capacity - cstring->capacity);
#if 0
        for(pos = cstring->capacity; pos < capacity; pos ++)
        {
            cstring->str[ pos ] = '\0';
        }
#endif
        cstring->capacity = capacity;

        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:cstring_expand_to: failed to expand cstring with capacity %ld and len %ld to capacity %ld\n",
                        cstring->capacity, cstring->len, capacity);

    return (EC_FALSE);
}

EC_BOOL cstring_set_capacity(CSTRING *cstring, const UINT32 capacity)
{
    UINT8 *str;
    if(0 != cstring->capacity && capacity != cstring->capacity)
    {
        SAFE_FREE(cstring->str, LOC_CSTRING_0009);
        cstring->str = (UINT8 *)0;
        cstring->capacity = 0;
    }

    cstring->len = 0;
    str = (UINT8 *)SAFE_MALLOC(sizeof(UINT8) * capacity, LOC_CSTRING_0010);
    if(str)
    {
        cstring->str = str;
        cstring->capacity = capacity;

        return (EC_TRUE);
    }
    return (EC_FALSE);
}

UINT32 cstring_get_capacity(const CSTRING *cstring)
{
    return cstring->capacity;
}

UINT32 cstring_get_len(const CSTRING *cstring)
{
    return cstring->len;
}

UINT32 cstring_get_room(const CSTRING *cstring)
{
    return (cstring->capacity - cstring->len);
}

UINT8 * cstring_get_str(const CSTRING *cstring)
{
    if(NULL_PTR == cstring || 0 == cstring->len)
    {
        return (UINT8 *)0;
    }
    return cstring->str;
}

EC_BOOL cstring_get_char(const CSTRING *cstring, const UINT32 pos, UINT8 *pch)
{
    if(pos >= cstring->len)
    {
        sys_log(LOGSTDOUT, "error:cstring_get_char: failed to get char at %ld due to overflow where cstring capaciy %ld and len %ld\n",
                        pos, cstring->capacity, cstring->len);
        return (EC_FALSE);
    }
    (*pch) = cstring->str[ pos ];
    return (EC_TRUE);
}

EC_BOOL cstring_set_str(CSTRING *cstring, const UINT8 *str)
{
    //cstring_clean(cstring);

    if(NULL_PTR != str)
    {
        cstring->str = (UINT8 *)str;
        cstring->len = (UINT32)strlen((char *)str);
        cstring->capacity = cstring->len + 1;
    }
    return (EC_TRUE);
}

CBYTES *cstring_get_cbytes(const CSTRING *cstring)
{
    if(NULL_PTR != cstring->str && 0 != cstring->len)
    {
        CBYTES *cbytes;
        cbytes = cbytes_new(cstring->len/* + 1*/);
        if(NULL_PTR == cbytes)
        {
            sys_log(LOGSTDOUT, "error:cstring_get_cbytes: new cbytes with len %ld failed\n", cstring->len + 1);
            return (NULL_PTR);
        }
        BCOPY(cstring->str, CBYTES_BUF(cbytes), cstring->len/* + 1*/);
        return (cbytes);
    }
    return (NULL_PTR);
}

EC_BOOL cstring_set_char(CSTRING *cstring, const UINT8 ch, const UINT32 pos)
{
    if(pos >= cstring->len)
    {
        sys_log(LOGSTDOUT, "error:cstring_set_char: failed to set %c at %ld due to overflow where cstring capaciy %ld and len %ld\n",
                        ch, pos, cstring->capacity, cstring->len);
        return (EC_FALSE);
    }

   cstring->str[ pos ] = ch; /*due to cstring_expand will set the new space to '\0', here ignore pos at end of string*/
   return (EC_TRUE);
}

EC_BOOL cstring_get_cstr(const CSTRING *cstring_src, const UINT32 from, const UINT32 to, CSTRING *cstring_des)
{
    UINT32 beg_pos;
    UINT32 end_pos;
    UINT32 pos;

    UINT8 *src_pch;
    UINT8 *des_pch;

    if(from >= to)
    {
        sys_log(LOGSTDOUT, "error:cstring_get_cstr: invalid sub range [%ld, %ld)\n", from, to);
        return (EC_FALSE);
    }

    beg_pos = from;
    end_pos = ((to >= cstring_src->len) ? cstring_src->len : to);

    if(beg_pos >= end_pos)
    {
        sys_log(LOGSTDOUT, "error:cstring_get_cstr: sub range [%ld, %ld) overflow where cstring_src len %ld, capacity %ld\n",
                         from, to, cstring_src->len, cstring_src->capacity);
        return (EC_FALSE);
    }

    if(EC_FALSE == cstring_expand_to(cstring_des, end_pos - beg_pos + 1))
    {
        sys_log(LOGSTDOUT, "error:cstring_get_cstr: failed to expand cstring with capaciy %ld and len %ld to size %ld\n",
                        cstring_des->capacity, cstring_des->len, end_pos - beg_pos + 1);
        return (EC_FALSE);
    }

    src_pch = cstring_src->str + beg_pos;
    des_pch = cstring_des->str;
    for(pos = beg_pos; pos < end_pos; pos ++)
    {
        (*des_pch ++) = (*src_pch ++);
    }
    (*des_pch) = '\0';
    cstring_des->len = end_pos - beg_pos;

    return (EC_TRUE);
}

EC_BOOL cstring_set_word(CSTRING *cstring, const UINT32 num)
{
    cstring_format(cstring, "%ld", num);
    return (EC_TRUE);
}

UINT32 cstring_get_word(const CSTRING *cstring)
{
    UINT32 c;            /* current char */
    UINT32 total;        /* current total */
    UINT8 pos;

    if(0 == cstring->len || NULL_PTR == cstring->str)
    {
        return ((UINT32)0);
    }
    total = 0;
    for(pos = 0; pos < cstring->len; pos ++)
    {
        c = (UINT32)(cstring->str[ pos ]);
        if(c < '0' || c > '9')
        {
            sys_log(LOGSTDERR, "error:cstring_get_word: cstring %.*s found not digit char at pos %ld\n",
                            cstring->len, cstring->str, pos);
            return ((UINT32)0);
        }
        total = 10 * total + (c - '0');
    }
    return (total);
}
EC_BOOL cstring_set_chars(CSTRING *cstring, const UINT8 *pchs, const UINT32 len)
{
    UINT32 pos;

    if(len < cstring->capacity)
    {
        for(pos = 0; pos < len; pos ++)
        {
            cstring->str[ pos ] = pchs[ pos ];
        }
        cstring->len = pos;
        return (EC_TRUE);
    }

    cstring_clean(cstring);

    cstring->str = (UINT8 *)SAFE_MALLOC(len, LOC_CSTRING_0011);
    if(NULL_PTR == cstring->str)
    {
        sys_log(LOGSTDOUT, "error:cstring_set_chars: failed to malloc memory %ld bytes\n", len);
        return (EC_FALSE);
    }
    cstring->capacity = len;
    cstring->len      = 0;

    for(pos = 0; pos < len; pos ++)
    {
        cstring->str[ pos ] = pchs[ pos ];
    }
    cstring->len = pos;
    cstring->str[ cstring->len ] = '\0';

    return (EC_TRUE);
}

CSTRING *cstring_make_by_word(const UINT32 num)
{
    char *str;

    str = uint32_to_str(num);
    return cstring_new((UINT8 *)str, LOC_CSTRING_0012);
}

CSTRING *cstring_make_by_ctimet(const CTIMET *ctimet)
{
    UINT32  ts_num;
    char   *ts_hex_str;

    ts_num = (UINT32)((*ctimet) & (~(UINT32_ZERO)));
    ts_hex_str = uint32_to_hex_str(ts_num);

    return cstring_new((UINT8 *)ts_hex_str, LOC_CSTRING_0013);
}

CSTRING *cstring_make_by_bytes(const UINT32 len, const UINT8 *bytes)
{
    CSTRING * cstring;

    cstring = cstring_new(NULL_PTR, LOC_CSTRING_0014);
    if(NULL_PTR == cstring)
    {
        sys_log(LOGSTDOUT, "error:cstring_make_by_bytes: new cstring failed\n");
        return (NULL_PTR);
    }
    
    if(EC_FALSE == cstring_set_chars(cstring, bytes, len))
    {
        sys_log(LOGSTDOUT, "error:cstring_make_by_bytes: sett chars to cstring failed\n");
        cstring_free(cstring);
        return (NULL_PTR);
    }
    return (cstring);
}

CSTRING *cstring_dup(const CSTRING *cstring_src)
{
    CSTRING *cstring_des;
    
    cstring_des = cstring_new(NULL_PTR, LOC_CSTRING_0015);
    if(NULL_PTR == cstring_des)
    {
        sys_log(LOGSTDOUT, "error:cstring_dup: new cstring failed\n");
        return (NULL_PTR);
    }

    cstring_clone(cstring_src, cstring_des);
    return (cstring_des);
}

EC_BOOL cstring_erase_char(CSTRING *cstring, const UINT32 pos)
{
    UINT32 cur_pos;

    UINT8 *src_pch;
    UINT8 *des_pch;

    if(pos >= cstring->len)
    {
        sys_log(LOGSTDOUT, "error:cstring_erase_char: failed to erase char at %ld due to overflow where cstring capaciy %ld and len %ld\n",
                        pos, cstring->capacity, cstring->len);
        return (EC_FALSE);
    }

    src_pch = cstring->str + pos + 1;
    des_pch = cstring->str + pos;
    for(cur_pos = pos; cur_pos < cstring->len; cur_pos ++)
    {
        (*des_pch ++) = (*src_pch ++);/*okay, the terminal char of string is moved forward too*/
    }
    cstring->len --;
    cstring->str[ cstring->len ] = '\0';
    return (EC_TRUE);
}


EC_BOOL cstring_append_char(CSTRING *cstring, const UINT8 ch)
{
    if(cstring->len + 1 >= cstring->capacity)
    {
        if(EC_FALSE == cstring_expand(cstring, LOC_CSTRING_0016))
        {
            sys_log(LOGSTDOUT, "error:cstring_append_char: failed to expand cstring with capaciy %ld and len %ld\n",
                            cstring->capacity, cstring->len);
            return (EC_FALSE);
        }
        //sys_log(LOGSTDOUT, "[LOG] cstring_append_char: expand to capacity %ld, len %ld\n",  cstring->capacity, cstring->len);
    }

    cstring->str[ cstring->len ++ ] = ch;
    cstring->str[ cstring->len ] = '\0';

    return (EC_TRUE);
}

EC_BOOL cstring_append_chars(CSTRING *cstring, const UINT32 ch_num, const UINT8 *chars)
{
    UINT32 pos;
    if(cstring->len + ch_num >= cstring->capacity)
    {
        if(EC_FALSE == cstring_expand_to(cstring, cstring->len + ch_num))
        {
            sys_log(LOGSTDOUT, "error:cstring_append_chars: failed to expand cstring with capaciy %ld and len %ld to size %ld\n",
                            cstring->capacity, cstring->len, cstring->len + ch_num);
            return (EC_FALSE);
        }
    }

    for(pos = 0; pos < ch_num; pos ++)
    {
        cstring->str[ cstring->len ++ ] = chars[ pos ];
    }

    cstring->str[ cstring->len ] = '\0';

    return (EC_TRUE);
}

EC_BOOL cstring_append_str(CSTRING *cstring, const UINT8 *str)
{
    UINT32 str_len;
    UINT32 pos;
    UINT8 *src_pch;
    UINT8 *des_pch;

    str_len = strlen((char *)str);

    if(EC_FALSE == cstring_expand_to(cstring, cstring->len + str_len + 1))
    {
        sys_log(LOGSTDOUT, "error:cstring_append_str: failed to expand cstring with capaciy %ld and len %ld to size %ld\n",
                        cstring->capacity, cstring->len, cstring->len + str_len + 1);
        return (EC_FALSE);
    }

    src_pch = (UINT8 *)str;
    des_pch = cstring->str + cstring->len;

    for(pos = 0; pos < str_len; pos ++)
    {
        (*des_pch ++) = (*src_pch ++);
    }

    cstring->len += str_len;
    cstring->str[ cstring->len ] = '\0';
    return (EC_TRUE);
}

EC_BOOL cstring_append_cstr(const CSTRING *cstring_src, CSTRING *cstring_des)
{
    return cstring_append_str(cstring_des, cstring_src->str);
}

EC_BOOL cstring_erase_sub_str(CSTRING *cstring, const UINT32 from, const UINT32 to)
{
    UINT32 src_pos;
    UINT32 des_pos;

    UINT32 str_len;
    UINT32 ret_len;

    str_len = cstring->len;
    des_pos = ((from >= str_len) ? str_len : from);
    src_pos = ((to >= str_len) ? str_len : to);

    if( src_pos <= des_pos )
    {
        return (EC_FALSE);
    }

    ret_len = str_len - (src_pos - des_pos);

    /*note: src_pos <= str_len means it will copy the terminal char of string to right position*/
    for(; src_pos <= str_len && des_pos < str_len; src_pos ++, des_pos ++)
    {
        cstring->str[ des_pos ] = cstring->str[ src_pos];
    }
    cstring->len = ret_len;
    cstring->str[ cstring->len ] = '\0';
    return (EC_TRUE);
}

EC_BOOL cstring_erase_tail_str(CSTRING *cstring, const UINT32 len)
{
    UINT32 str_len;

    str_len = cstring->len;

    if(0 == str_len)
    {
        return (EC_TRUE);
    }

    if(str_len < len)
    {
        sys_log(LOGSTDOUT, "warn:cstring_erase_tail_str: force cstring to empty due to its len %ld < len %ld to erase\n", str_len, len);
        cstring->str[ 0 ] = '\0';
        cstring->len = 0;
        return (EC_TRUE);
    }

    //sys_log(LOGSTDOUT, "\n");
    //sys_log(LOGSTDOUT, "cstring_erase_tail_str: %s => \n", cstring->str);

    cstring->str[ str_len - len ] = '\0';
    cstring->len = str_len - len;

    //sys_log(LOGSTDOUT, "cstring_erase_tail_str: %s\n", cstring->str);
    return (EC_TRUE);
}

/*read content from fp and write into cstring, note: when have no chance to get file length when fp is pipe*/
UINT32 cstring_fread(CSTRING *cstring, FILE *fp)
{
    for(;;)
    {
        if(cstring->len == cstring->capacity)
        {
            /*expand cstring to accept left bytes in file*/
            if(EC_FALSE == cstring_expand(cstring, LOC_CSTRING_0017))
            {
                sys_log(LOGSTDOUT, "error:cstring_fread: failed to expand cstring with capaciy %ld and len %ld\n",
                                cstring->capacity, cstring->len);
                return ((UINT32)-1);
            }
        }

        cstring->len += fread((char *)(cstring->str + cstring->len), sizeof(char), cstring->capacity - cstring->len, fp);
        if(cstring->len < cstring->capacity)/*ok, no more to read*/
        {
            cstring->str[ cstring->len ++ ] = '\0';
            break;
        }
    }
    return (0);
}

void cstring_print(LOG *log, const CSTRING *cstring)
{
    if(NULL_PTR == cstring)
    {
        sys_log(log, "(null)\n");
    }
    else if(0 == cstring->str)
    {
        sys_log(log, "cstring %lx: capacity = %ld, len = %ld, str = <error:undefined string>\n",
                        cstring,
                        cstring->capacity,
                        cstring->len);
    }
    else if(0 == cstring->len)
    {
        sys_log(log, "cstring %lx: capacity = %ld, len = %ld, str = <null>\n",
                        cstring,
                        cstring->capacity,
                        cstring->len);
    }
    else
    {
        sys_log(log, "cstring %lx: capacity = %ld, len = %ld, str = %s\n",
                        cstring,
                        cstring->capacity,
                        cstring->len,
                        cstring->str);
    }
    return ;
}

void cstring_format(CSTRING *cstring, const char *format, ...)
{
    va_list ap;
    UINT32 len;

    va_list params;

    //cstring_print(LOGSTDOUT, cstring);

    va_start(ap, format);

    va_copy(params, ap);

    len = (UINT32)vsnprintf((char *)0, 0, format, params);

    cstring_expand_to(cstring, len + cstring->len + 1);

    len = vsnprintf((char *)(cstring->str + cstring->len), cstring->capacity - cstring->len, format, ap);
    cstring->len += len;
    //cstring->str[ len ] = '\0';
    va_end(ap);

    return;
}

void cstring_vformat(CSTRING *cstring, const char *format, va_list ap)
{
    UINT32 len;
    va_list params;

    //cstring_print(LOGSTDOUT, cstring);

    va_copy(params, ap);

    len = (UINT32)vsnprintf((char *)0, 0, format, params);

    cstring_expand_to(cstring, len + cstring->len + 1);

    len = vsnprintf((char *)(cstring->str + cstring->len), cstring->capacity - cstring->len, format, ap);
    cstring->len += len;

    return;
}
#if 0
/***********************************************************************************************************************\
SYNOPSIS

       #include <pcre.h>

       pcre *pcre_compile(const char *pattern, int options,
            const char **errptr, int *erroffset,
            const unsigned char *tableptr);

DESCRIPTION

       This function compiles a regular expression into an internal form. Its arguments are:

         pattern       A zero-terminated string containing the
                         regular expression to be compiled
         options       Zero or more option bits
         errptr        Where to put an error message
         erroffset     Offset in pattern where error was found
         tableptr      Pointer to character tables, or NULL to
                         use the built-in default

       The option bits are:

         PCRE_ANCHORED         Force pattern anchoring
         PCRE_CASELESS         Do caseless matching
         PCRE_DOLLAR_ENDONLY   $ not to match newline at end
         PCRE_DOTALL           . matches anything including NL
         PCRE_EXTENDED         Ignore whitespace and # comments
         PCRE_EXTRA            PCRE extra features
                                 (not much use currently)
         PCRE_MULTILINE        ^ and $ match newlines within data
         PCRE_NO_AUTO_CAPTURE  Disable numbered capturing paren-
                                 theses (named ones available)
         PCRE_UNGREEDY         Invert greediness of quantifiers
         PCRE_UTF8             Run in UTF-8 mode
         PCRE_NO_UTF8_CHECK    Do not check the pattern for UTF-8
                                 validity (only relevant if
                                 PCRE_UTF8 is set)

       PCRE must be compiled with UTF-8 support in order to use PCRE_UTF8 (or PCRE_NO_UTF8_CHECK).

       The yield of the function is a pointer to a private data structure that contains the compiled pattern, or NULL if an error
       was detected.

       There is a complete description of the PCRE API in the pcreapi page.
\***********************************************************************************************************************/
/***********************************************************************************************************************\
NAME
       PCRE - Perl-compatible regular expressions

SYNOPSIS

       #include <pcre.h>

       int pcre_exec(const pcre *code, const pcre_extra *extra,
            const char *subject, int length, int startoffset,
            int options, int *ovector, int ovecsize);

DESCRIPTION

       This  function  matches  a  compiled  regular  expression against a given subject string, and returns offsets to capturing
       subexpressions. Its arguments are:

         code         Points to the compiled pattern
         extra        Points to an associated pcre_extra structure,
                        or is NULL
         subject      Points to the subject string
         length       Length of the subject string, in bytes
         startoffset  Offset in bytes in the subject at which to
                        start matching
         options      Option bits
         ovector      Points to a vector of ints for result offsets
         ovecsize     Size of the vector (a multiple of 3)

       The options are:

         PCRE_ANCHORED      Match only at the first position
         PCRE_NOTBOL        Subject is not the beginning of a line
         PCRE_NOTEOL        Subject is not the end of a line
         PCRE_NOTEMPTY      An empty string is not a valid match
         PCRE_NO_UTF8_CHECK Do not check the subject for UTF-8
                              validity (only relevant if PCRE_UTF8
                              was set at compile time)

       There is a complete description of the PCRE API in the pcreapi page.
\***********************************************************************************************************************/
EC_BOOL cstring_regex(const CSTRING *cstring, const CSTRING *pattern, CVECTOR *cvector_cstring)
{
    pcre            *re;
    const char      *error;
    int             erroffset;
    int             startoffset;
    int             option;
    int             ovector[CSTRING_OVEC_COUNT];
    int             rc;
    int             idx;

    startoffset = 0;
    option      = 0;

    re = pcre_compile((char *)(pattern->str), 0, &error, &erroffset, NULL_PTR);
    if (NULL_PTR == re)
    {
        sys_log(LOGSTDOUT, "error:cstring_regex: PCRE compilation failed at offset %d: %s\n", erroffset, error);
        return (EC_FALSE);
    }

    rc = pcre_exec(re, NULL_PTR, (char *)(cstring->str), cstring->len, startoffset, option, ovector, CSTRING_OVEC_COUNT);
    if(PCRE_ERROR_NOMATCH == rc)
    {
        sys_log(LOGSTDOUT, "warn:cstring_regex: no matched\n");
        free(re);
        return (EC_TRUE);
    }

    if(0 > rc)
    {
        sys_log(LOGSTDOUT, "error:cstring_regex: matching error with str %s and pattern %s\n", (char *)(cstring->str), (char *)(pattern->str));
        free(re);
        return (EC_FALSE);
    }

    for (idx = 0; idx < rc; idx ++)
    {
        UINT32 from;
        UINT32 to;

        CSTRING *sub_cstring;

        from = ovector[ 2 * idx ];
        to   = ovector[ 2 * idx + 1];

        sub_cstring = cstring_new(NULL_PTR, LOC_CSTRING_0018);

        cstring_get_cstr(cstring, from, to, sub_cstring);

        cvector_push(cvector_cstring, (void *)sub_cstring);
    }

    return (EC_TRUE);
}
#endif

#ifdef __cplusplus
}
#endif/*__cplusplus*/

