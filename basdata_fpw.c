#include "basdata.h"
#include <math.h>
#include <stdint.h>

basdata_res basdata_d2fp(double value, unsigned char *bdata)
{
    int exponent;
    uint_least32_t mantissa;

    if (value == 0.0) {
        exponent = 0;
        mantissa = 0;
    }
    else {
        uint_least32_t sign = 0;
        if (value < 0) {
            value = -value;
            sign = 0x80000000;
        }
        mantissa = (lround(frexp(value, &exponent) * 4294967296.0) & 0x7fffffff)|sign;
        //mantissa = ((uint64_t)(frexp(value, &exponent) * 4294967296.0) & 0x7fffffff)|sign;
        if (exponent < -128 || exponent > 127)
            return BASDATA_RANGE;
        exponent += 0x80;
    }
    bdata[0] = mantissa;
    bdata[1] = mantissa >> 8;
    bdata[2] = mantissa >> 16;
    bdata[3] = mantissa >> 24;
    bdata[4] = exponent;
    return BASDATA_OK;

}

basdata_res basdata_writef(double value, FILE *fp)
{
    unsigned char buf[6];
    basdata_res res = basdata_d2fp(value, buf+1);
    if (res == BASDATA_OK) {
        buf[0] = 0xff;
        if (fwrite(buf, 6, 1, fp) != 1)
            res = BASDATA_IOERR;
    }
    return res;
}
