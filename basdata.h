#ifndef BASDATA_INC
#define BASDATA_INC

#include <stdio.h>
#include <stdint.h>

typedef enum {
    BASDATA_OK,
    BASDATA_EOF,
    BASDATA_BADEOF,
    BASDATA_BADTYPE,
    BASDATA_RANGE,
    BASDATA_IOERR
} basdata_res;

typedef enum {
    BASDATA_STRING,
    BASDATA_INTEGER,
    BASDATA_FLOAT,
    BASDATA_INVALID
} basdata_type;

typedef struct {
    basdata_type type;
    union {
        struct {
            uint8_t len;
            char str[256];
        } s;
        int_least32_t i;
        double f;
    } u;
} basdata_var;

extern double basdata_fp2d(const unsigned char *bdata);
extern basdata_res basdata_d2fp(double value, unsigned char *bdata);

extern basdata_res basdata_sread(FILE *fp, char *str, int len);
extern basdata_res basdata_reads(FILE *fp, char *str);
extern basdata_res basdata_readi(FILE *fp, int_least32_t *value);
extern basdata_res basdata_readf(FILE *fp, double *value);
extern basdata_res basdata_readv(FILE *fp, basdata_var *var);

extern basdata_res basdata_writes(const char *str, int len, FILE *fp);
extern basdata_res basdata_writei(int_least32_t value, FILE *fp);
extern basdata_res basdata_writef(double value, FILE *fp);
extern basdata_res basdata_writev(basdata_var *var, FILE *fp);

extern const char *basdata_rmsg(basdata_res res);

#endif
