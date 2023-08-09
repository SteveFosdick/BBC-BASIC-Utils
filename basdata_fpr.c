#include "basdata.h"
#include <math.h>
#include <stdint.h>

double basdata_fp2d(const unsigned char *bdata)
{
    uint_least32_t mantissa = bdata[0] | (bdata[1] << 8) | (bdata[2] << 16) | (bdata[3] << 24);
    int exponent = bdata[4];
    if (mantissa || exponent) {
        uint_least32_t sign = mantissa & 0x80000000;
        exponent -= 0x80;
        mantissa |= 0x80000000;
        double value = ldexp((double)mantissa, exponent-32);
        if (sign)
            value = -value;
        return value;
    }
    return 0.0;
}

basdata_res basdata_readf(FILE *fp, double *vptr)
{
    unsigned char bdata[6];
    if (fread(bdata, sizeof(bdata), 1, fp) == 1) {
        if (bdata[0] == 0xff) {
            *vptr = basdata_fp2d(bdata+1);
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
