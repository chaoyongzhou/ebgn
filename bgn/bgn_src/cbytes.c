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

#include "type.h"
#include "mm.h"
#include "log.h"
#include "char2int.h"
#include "cmpic.inc"

#include "cbytes.h"

CBYTES *cbytes_new(const UINT32 len)
{
    CBYTES *cbytes;
    alloc_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CBYTES, &cbytes, LOC_CBYTES_0001);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cbytes_new: alloc memory failed\n");
        return (NULL_PTR);
    }

    cbytes_init(cbytes);

    if(0 < len)
    {
        CBYTES_BUF(cbytes) = (UINT8 *)SAFE_MALLOC(len, LOC_CBYTES_0002);
        if(NULL_PTR == CBYTES_BUF(cbytes))
        {
            sys_log(LOGSTDOUT, "error:cbytes_new: alloc %ld bytes failed\n", len);
            free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CBYTES, cbytes, LOC_CBYTES_0003);
            return (NULL_PTR);
        }
        CBYTES_LEN(cbytes) = len;
    }

    return (cbytes);
}

EC_BOOL cbytes_init(CBYTES *cbytes)
{
    CBYTES_LEN(cbytes) = 0;
    CBYTES_BUF(cbytes) = NULL_PTR;
    return (EC_TRUE);
}

EC_BOOL cbytes_clean(CBYTES *cbytes, const UINT32 location)
{
    if(NULL_PTR != cbytes)
    {
        CBYTES_LEN(cbytes) = 0;
        if(NULL_PTR != CBYTES_BUF(cbytes))
        {
            SAFE_FREE(CBYTES_BUF(cbytes), location);
            CBYTES_BUF(cbytes) = NULL_PTR;
        }
    }
    return (EC_TRUE);
}

EC_BOOL cbytes_free(CBYTES *cbytes, const UINT32 location)
{
    if(NULL_PTR != cbytes)
    {
        cbytes_clean(cbytes, location);
        free_static_mem(MD_TBD, CMPI_ANY_MODI, MM_CBYTES, cbytes, location);
    }
    return (EC_TRUE);
}

EC_BOOL cbytes_init_0(const UINT32 md_id, CBYTES *cbytes)
{
    return cbytes_init(cbytes);
}

EC_BOOL cbytes_clean_0(const UINT32 md_id, CBYTES *cbytes)
{
    return cbytes_clean(cbytes, LOC_CBYTES_0004);
}

EC_BOOL cbytes_free_0(const UINT32 md_id, CBYTES *cbytes)
{
    return cbytes_free(cbytes, LOC_CBYTES_0005);
}

EC_BOOL cbytes_set(CBYTES *cbytes, const UINT32 len, const UINT8 *buf)
{
    UINT8 *des;

    if(0 == len)
    {
        CBYTES_BUF(cbytes) = NULL_PTR;
        CBYTES_LEN(cbytes) = 0;
        return (EC_TRUE);
    }

    des = (UINT8 *)SAFE_MALLOC(len, LOC_CBYTES_0006);
    if(NULL_PTR == des)
    {
        sys_log(LOGSTDOUT, "error:cbytes_set: alloc %ld bytes failed\n", len);
        return (EC_FALSE);
    }

    BCOPY(buf, des, len);
    CBYTES_BUF(cbytes) = des;
    CBYTES_LEN(cbytes) = len;
    return (EC_TRUE);
}

EC_BOOL cbytes_set_word(CBYTES *cbytes, const UINT32 num)
{
    char *str;

    str = uint32_to_str(num);
    return cbytes_set(cbytes, strlen((char *)str), (UINT8 *)str);
}

EC_BOOL cbytes_get_word(CBYTES *cbytes, UINT32 *num)
{
    (*num) = str_to_uint32((char *)cbytes_buf(cbytes));
    return (EC_TRUE);
}

CBYTES *cbytes_make_by_word(const UINT32 num)
{
    CBYTES *cbytes;
    char   *str;
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_word: new cbytes failed\n");
        return (NULL_PTR);
    }

    str = uint32_to_str(num);
    if(EC_FALSE == cbytes_set(cbytes, strlen((char *)str), (UINT8 *)str))
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_word: set %s to cbytes failed\n", str);
        cbytes_free(cbytes, LOC_CBYTES_0007);
        return (NULL_PTR);
    }
    
    return (cbytes);
}

CBYTES *cbytes_make_by_str(const UINT8 *str)
{
    CBYTES *cbytes;
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_str: new cbytes failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_set(cbytes, strlen((char *)str), (UINT8 *)str))
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_str: set str %s to cbytes failed\n", str);
        cbytes_free(cbytes, LOC_CBYTES_0008);
        return (NULL_PTR);
    }
    
    return (cbytes);
}

CBYTES *cbytes_make_by_cstr(const CSTRING *cstr)
{
    CBYTES *cbytes;
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_cstr: new cbytes failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_set(cbytes, cstring_get_len(cstr), cstring_get_str(cstr)))
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_cstr: set cstr %.*s to cbytes failed\n", 
                            cstring_get_len(cstr), cstring_get_str(cstr));
        cbytes_free(cbytes, LOC_CBYTES_0009);
        return (NULL_PTR);
    }
    
    return (cbytes);
}

CBYTES *cbytes_make_by_ctimet(const CTIMET *ctimet)
{
    UINT32  ts_num;
    char   *ts_hex_str;

    ts_num = (UINT32)((*ctimet) & (~(UINT32_ZERO)));
    ts_hex_str = uint32_to_hex_str(ts_num);

    return cbytes_make_by_str((UINT8 *)ts_hex_str);
}

CBYTES *cbytes_make_by_bytes(const UINT32 len, const UINT8 *bytes)
{
    CBYTES *cbytes;
    
    cbytes = cbytes_new(0);
    if(NULL_PTR == cbytes)
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_bytes: new cbytes failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_set(cbytes, len, bytes))
    {
        sys_log(LOGSTDOUT, "error:cbytes_make_by_bytes: set %ld bytes to cbytes failed\n", len);
        cbytes_free(cbytes, LOC_CBYTES_0010);
        return (NULL_PTR);
    }
    
    return (cbytes);
}

CBYTES *cbytes_dup(const CBYTES *cbytes_src)
{
    CBYTES *cbytes_des;

    cbytes_des = cbytes_new(0);
    if(NULL_PTR == cbytes_des)
    {
        sys_log(LOGSTDOUT, "error:cbytes_dup: new cbytes failed\n");
        return (NULL_PTR);
    }

    if(EC_FALSE == cbytes_clone(cbytes_src, cbytes_des))
    {
        sys_log(LOGSTDOUT, "error:cbytes_dup: clone cbytes failed\n");
        cbytes_free(cbytes_des, LOC_CBYTES_0011);
        return (NULL_PTR);
    }
    return (cbytes_des);
}

EC_BOOL cbytes_clone(const CBYTES *cbytes_src, CBYTES *cbytes_des)
{
    UINT32 data_len;
    UINT8 *buf;

    data_len = CBYTES_LEN(cbytes_src);

    buf = (UINT8 *)SAFE_MALLOC(data_len, LOC_CBYTES_0012);
    if(NULL_PTR == buf)
    {
        sys_log(LOGSTDOUT, "error:cbytes_clone: alloc %ld bytes failed\n", data_len);
        return (EC_FALSE);
    }

    CBYTES_BUF(cbytes_des) = buf;
    BCOPY(CBYTES_BUF(cbytes_src), CBYTES_BUF(cbytes_des), data_len);
    CBYTES_LEN(cbytes_des) = data_len;

    return (EC_TRUE);
}

EC_BOOL cbytes_mount(CBYTES *cbytes, const UINT32 len, const UINT8 *buff)
{
    CBYTES_BUF(cbytes) = (UINT8 *)buff;
    CBYTES_LEN(cbytes) = len;
    return (EC_TRUE);
}

EC_BOOL cbytes_umount(CBYTES *cbytes)
{
    CBYTES_BUF(cbytes) = NULL_PTR;
    CBYTES_LEN(cbytes) = 0;
    return (EC_TRUE);
}

EC_BOOL cbytes_cat(const CBYTES *cbytes_src_1st, const CBYTES *cbytes_src_2nd, CBYTES *cbytes_des)
{
    UINT32 data_len;
    UINT32 offset;
    UINT8 *buf;

    data_len = CBYTES_LEN(cbytes_src_1st) + CBYTES_LEN(cbytes_src_2nd);
    buf = (UINT8 *)SAFE_MALLOC(data_len, LOC_CBYTES_0013);
    if(NULL_PTR == buf)
    {
        sys_log(LOGSTDOUT, "error:cbytes_cat: alloc %ld bytes failed\n", data_len);
        return (EC_FALSE);
    }

    offset = 0;
    BCOPY(CBYTES_BUF(cbytes_src_1st), buf + offset, CBYTES_LEN(cbytes_src_1st));

    offset += CBYTES_LEN(cbytes_src_1st);
    BCOPY(CBYTES_BUF(cbytes_src_2nd), buf + offset, CBYTES_LEN(cbytes_src_2nd));

    CBYTES_BUF(cbytes_des) = (UINT8 *)buf;
    CBYTES_LEN(cbytes_des) = data_len;

    return (EC_TRUE);
}

EC_BOOL cbytes_cmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des)
{
    if(CBYTES_LEN(cbytes_src) != CBYTES_LEN(cbytes_des))
    {
        return (EC_FALSE);
    }

    if(NULL_PTR == CBYTES_BUF(cbytes_src) && NULL_PTR != CBYTES_BUF(cbytes_des))
    {
        return (EC_FALSE);
    }

    if(NULL_PTR != CBYTES_BUF(cbytes_src) && NULL_PTR == CBYTES_BUF(cbytes_des))
    {
        return (EC_FALSE);
    }

    if(0 == BCMP(CBYTES_BUF(cbytes_src), CBYTES_BUF(cbytes_des), CBYTES_LEN(cbytes_src)))
    {
        return (EC_TRUE);
    }
    return (EC_FALSE);
}

EC_BOOL cbytes_ncmp(const CBYTES *cbytes_src, const CBYTES *cbytes_des, const UINT32 cmp_len)
{
    UINT32 len;

    if(NULL_PTR == CBYTES_BUF(cbytes_src) && NULL_PTR != CBYTES_BUF(cbytes_des))
    {
        return (EC_FALSE);
    }

    if(NULL_PTR != CBYTES_BUF(cbytes_src) && NULL_PTR == CBYTES_BUF(cbytes_des))
    {
        return (EC_FALSE);
    }

    len = cmp_len;
    len = DMIN(CBYTES_LEN(cbytes_src), len);
    len = DMIN(CBYTES_LEN(cbytes_des), len);
    if(0 == BCMP(CBYTES_BUF(cbytes_src), CBYTES_BUF(cbytes_des), len))
    {
        return (EC_TRUE);
    }

    return (EC_FALSE);
}

UINT32 cbytes_len(const CBYTES *cbytes)
{
    return CBYTES_LEN(cbytes);
}

UINT8 * cbytes_buf(const CBYTES *cbytes)
{
    return (UINT8 *)CBYTES_BUF(cbytes);
}

EC_BOOL cbytes_expand(CBYTES *cbytes, const UINT32 location)
{
    UINT8 *buf;
    UINT32 len;

    if(0 == cbytes->len)
    {
        len = CBYTES_DEFAULT_LEN * sizeof(UINT8);
        buf = (UINT8 *)SAFE_MALLOC(len, location);
    }
    else
    {
        len = 2 * (cbytes->len) * sizeof(UINT8);/*double the old capacity*/
        buf = (UINT8 *)SAFE_REALLOC(cbytes->buf, cbytes->len, len , location);
    }

    if(buf)
    {
        cbytes->buf = buf;
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        BSET(cbytes->buf + cbytes->len, 0, len - cbytes->len);
        cbytes->len = len;

        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:cbytes_expand: failed to expand cbytes with len %ld\n", cbytes->len);

    return (EC_FALSE);
}

EC_BOOL cbytes_expand_to(CBYTES *cbytes, const UINT32 size)
{
    UINT32 len;
    UINT8 *buf;

    len = DMAX(size, CBYTES_DEFAULT_LEN);

    if(len <= cbytes->len)
    {
        /*nothing to do*/
        return (EC_TRUE);
    }

    if(0 == cbytes->len)
    {
        buf = (UINT8 *)SAFE_MALLOC(len, LOC_CBYTES_0014);
    }
    else
    {
        buf = (UINT8 *)SAFE_REALLOC(cbytes->buf, cbytes->len, len, LOC_CBYTES_0015);
    }

    if(buf)
    {
        cbytes->buf = buf;
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        BSET(cbytes->buf + cbytes->len, 0, len - cbytes->len);
        cbytes->len = len;

        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:cbytes_expand_to: failed to expand cbytes with len %ld to len %ld\n",
                        cbytes->len, len);

    return (EC_FALSE);
}

EC_BOOL cbytes_resize(CBYTES *cbytes, const UINT32 size)
{
    UINT8 *buf;

    if(cbytes->len >= size)
    {
        cbytes->len = size;
        return (EC_TRUE);
    }

    if(0 == cbytes->len)
    {
        buf = (UINT8 *)SAFE_MALLOC(size, LOC_CBYTES_0016);
    }
    else
    {
        buf = (UINT8 *)SAFE_REALLOC(cbytes->buf, cbytes->len, size, LOC_CBYTES_0017);
    }

    if(buf)
    {
        cbytes->buf = buf;
        /*note: here not call memset to set data area to zero due to finding its unstable*/
        BSET(cbytes->buf + cbytes->len, 0, size - cbytes->len);
        cbytes->len = size;

        return (EC_TRUE);
    }

    sys_log(LOGSTDOUT, "error:cbytes_resize: failed to resize cbytes with len %ld to size %ld\n",
                        cbytes->len, size);
    return (EC_FALSE);
}


/*note: no chance to get file length when fp is pipe*/
EC_BOOL cbytes_fread3(CBYTES *cbytes, FILE *fp)
{
    UINT32 len;

    len = 0;
    for(;;)
    {
        if(len + 1 >= cbytes->len)
        {
            /*expand cbytes to accept left bytes in file*/
            if(EC_FALSE == cbytes_expand(cbytes, LOC_CBYTES_0018))
            {
                sys_log(LOGSTDOUT, "error:cbytes_fread: failed to expand cbytes with len %ld\n", cbytes->len);
                return (EC_FALSE);
            }
        }

        if(NULL_PTR == fgets((char *)(cbytes->buf + len), cbytes->len - len, fp))
        {
            break;
        }

        len += strlen((char *)(cbytes->buf + len));
    }
    cbytes->len = len;/*reset len*/
    return (EC_TRUE);
}


/*note: no chance to get file length when fp is pipe*/
EC_BOOL cbytes_fread(CBYTES *cbytes, FILE *fp)
{
    UINT32 len;

    len = 0;
    for(;;)
    {
        if(len == cbytes->len)
        {
            /*expand cbytes to accept left bytes in file*/
            if(EC_FALSE == cbytes_expand(cbytes, LOC_CBYTES_0019))
            {
                sys_log(LOGSTDOUT, "error:cbytes_fread: failed to expand cbytes with len %ld\n", cbytes->len);
                return (EC_FALSE);
            }
        }

        len += fread((char *)(cbytes->buf + len), sizeof(char), cbytes->len - len, fp);
        if(len < cbytes->len)/*ok, no more to read*/
        {
            break;
        }
    }
    cbytes->len = len;/*reset len*/
    return (EC_TRUE);
}

EC_BOOL cbytes_fread0(CBYTES *cbytes, FILE *fp)
{
    UINT32 flen;
    UINT32 fcur;
    UINT32 rsize;
    UINT32 csize;

    fcur = ftell(fp);
    flen = fseek(fp, 0, SEEK_END);
    fseek(fp, fcur, SEEK_SET);

    sys_log(LOGSTDOUT, "[DEBUG] cbytes_fread: fcur = %ld, flen %ld\n", fcur, flen);

    rsize = flen - fcur;

    if(cbytes_len(cbytes) < rsize)
    {
        cbytes_expand_to(cbytes, rsize);
    }

    for(csize = 0; 0 == feof(fp) && csize < rsize;)
    {
        csize += fread(cbytes->buf + csize, sizeof(char), rsize - csize, fp);
    }

    if(0 == feof(fp))/*not reach end of file*/
    {
        sys_log(LOGSTDOUT, "error:cbytes_fread: read %ld bytes from file at offset %ld failed\n", rsize, fcur);
        return (EC_FALSE);
    }
    return (EC_TRUE);
}

EC_BOOL cbytes_print_str(LOG *log, const CBYTES *cbytes)
{
    sys_print(log, "size = %ld, buff = %.*s\n", CBYTES_LEN(cbytes), CBYTES_LEN(cbytes), (char *)CBYTES_BUF(cbytes));
    return (EC_TRUE);
}

EC_BOOL cbytes_print_chars(LOG *log, const CBYTES *cbytes)
{
    UINT32 pos;

    for(pos = 0; pos < CBYTES_LEN(cbytes); pos ++)
    {
        sys_print(log, "%02x ", CBYTES_BUF(cbytes)[pos]);
    }
    sys_print(log, "\n");

    return (EC_TRUE);
}

#ifdef __cplusplus
}
#endif/*__cplusplus*/

