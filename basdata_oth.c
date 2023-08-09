#include "basdata.h"
#include <assert.h>
#include <errno.h>
#include <string.h>

static const char *basdata_msgs[] =
{
    "worked",
    "EOF",
    "unexpected EOF",
    "bad type",
    "out of range"
};

const char *basdata_rmsg(basdata_res res)
{
    assert(res <= BASDATA_IOERR);
    if (res == BASDATA_IOERR)
        return strerror(errno);
    else
        return basdata_msgs[res];
}

static void basdata_reverse(const char *src, char *dest, int len)
{
    int i = 0;
    int j = len;
    while (j)
        dest[i++] = src[--j];
    dest[i] = 0;
}

basdata_res basdata_sread(FILE *fp, char *str, int len)
{
    if (len == 0) {
        str[0] = 0;
        return BASDATA_OK;
    }
    else {
        char buf[256];
        if (fread(buf, len, 1, fp) == 1) {
            basdata_reverse(buf, str, len);
            return BASDATA_OK;
        }
        else if (ferror(fp))
            return BASDATA_IOERR;
        else
            return BASDATA_EOF;
    }
}

basdata_res basdata_reads(FILE *fp, char *str)
{
    unsigned char two[2];
    if (fread(two, 2, 1, fp) == 1) {
        if (two[0] == 0)
            return basdata_sread(fp, str, two[1]);
        else
            return BASDATA_BADTYPE;
    }
    else if (ferror(fp))
        return BASDATA_IOERR;
    else
        return BASDATA_EOF;
}

basdata_res basdata_readi(FILE *fp, int_least32_t *value)
{
    unsigned char buf[5];
    if (fread(buf, 5, 1, fp) == 1) {
        if (buf[0] == 0x40) {
            *value = (buf[1] << 24) | (buf[2] << 16) | (buf[3] << 8) | buf[4];
            return BASDATA_OK;
        }
        else
            return BASDATA_BADTYPE;
    }
    else if (ferror(fp))
        return BASDATA_IOERR;
    else
        return BASDATA_EOF;
}

basdata_res basdata_writes(const char *str, int len, FILE *fp)
{
    unsigned char buf[258];
    buf[0] = 0;
    buf[1] = len;
    basdata_reverse(str, (char *)buf+2, len);
    if (fwrite(buf, len+2, 1, fp) == 1)
        return BASDATA_OK;
    else
        return BASDATA_IOERR;
}

basdata_res basdata_writei(int_least32_t value, FILE *fp)
{
    unsigned char buf[5];
    buf[0] = 0x40;
    buf[1] = value >> 24;
    buf[2] = value >> 16;
    buf[3] = value >> 8;
    buf[4] = value;
    if (fwrite(buf, 5, 1, fp) == 1)
        return BASDATA_OK;
    else
        return BASDATA_IOERR;
}
