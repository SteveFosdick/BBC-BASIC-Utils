#include "basdata.h"
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define S "The quick brown fox jumps over the lazy dog "
#define Q "Now is the time to bury the hatchet"

static const char s_str[] = S;
static const char l_str[] = S S S S S Q;
static const int_least32_t integers[] = { -1, 0x12345678, 0x98765432, 0 };
static const double floats[] = { 1,-1,2,3,-4343,4578321, -1.521640152e-36, 2.37977495e21, 0 };
static bool check_str(const char *fn, FILE *fp, const char *expected)
{
    char str[258];
    basdata_res res = basdata_reads(fp, str);
    if (res == BASDATA_OK) {
        if (strcmp(str, expected)) {
            printf("String mismatch in %s\nExpected: %s\nGot:      %s\n\n", fn, expected, str);
            return false;
        }
        return true;
    }
    else {
        const char *msg = basdata_rmsg(res);
        fprintf(stderr, "basdata_test: unable to read string from %s: %s\n", fn, msg);
        return false;
    }
}

static bool check_strings(const char *fn, FILE *fp)
{
    bool worked = true;
    if (!check_str(fn, fp, s_str))
        worked = false;
    if (!check_str(fn, fp, l_str))
        worked = false;
    if (!check_str(fn, fp, ""))
        worked = false;
    return worked;
}

static bool check_integers(const char *fn, FILE *fp)
{
    bool worked = true;
    int i = 0;
    int_least32_t expected;
    do {
        int_least32_t value;
        expected = integers[i++];
        basdata_res res = basdata_readi(fp, &value);
        if (res == BASDATA_OK) {
            if ((uint_least32_t)value != expected) {
                printf("Integer mismatch in %s\nExpected: %12d %08X\nGot:      %12d %08X\n\n", fn, expected, expected, value, value);
                worked = false;
            }
        }
        else {
            const char *msg = basdata_rmsg(res);
            fprintf(stderr, "basdata_test: unable to read integer from %s: %s\n", fn, msg);
            worked = false;
        }
    } while (expected != 0);
    return worked;
}

static bool check_floats(const char *fn, FILE *fp)
{
    bool worked = true;
    int i = 0;
    double expected;
    do {
        double value;
        expected = floats[i++];
        basdata_res res = basdata_readf(fp, &value);
        if (res == BASDATA_OK) {
            if (expected != 0) {
                double ratio = fabs(fabs(value/expected)-1.0);
                if (ratio > 3e-10) {
                    printf("Float mismatch in %s\nExpected: %.17lg\nGot:      %.17lg\nRatio:    %.17lg\n\n", fn, expected, value, ratio);
                    worked = false;
                }
            }
        }
        else {
            const char *msg = basdata_rmsg(res);
            fprintf(stderr, "basdata_test: unable to read float from %s: %s\n", fn, msg);
            return false;
        }
    } while (expected != 0.0);
    return worked;
}

static int check_file(const char *fn)
{
    FILE *fp = fopen(fn, "rb");
    if (fp) {
        int status = 0;
        if (!check_strings(fn, fp))
            status = 1;
        if (!check_integers(fn, fp))
            status = 1;
        if (!check_floats(fn, fp))
            status = 1;
        fclose(fp);
        return status;
    }
    else {
        fprintf(stderr, "basdata_test: unable to open %s for reading: %s\n", fn, strerror(errno));
        return 1;
    }
}

static bool write_strings(const char *fn, FILE *fp)
{
    bool worked = true;
    basdata_res res;
    if ((res = basdata_writes(s_str, sizeof(s_str)-1, fp)) != BASDATA_OK) {
        fprintf(stderr, "basdata_test: unable to write string to %s: %s\n", fn, basdata_rmsg(res));
        worked = false;
    }
    if ((res = basdata_writes(l_str, sizeof(l_str)-1, fp)) != BASDATA_OK) {
        fprintf(stderr, "basdata_test: unable to write string to %s: %s\n", fn, basdata_rmsg(res));
        worked = false;
    }
    if ((res = basdata_writes("", 0, fp)) != BASDATA_OK) {
        fprintf(stderr, "basdata_test: unable to write string to %s: %s\n", fn, basdata_rmsg(res));
        worked = false;
    }
    return worked;
}

static bool write_integers(const char *fn, FILE *fp)
{
    bool worked = true;
    int i = 0;
    int_least32_t value;
    do {
        value = integers[i++];
        basdata_res res = basdata_writei(value, fp);
        if (res != BASDATA_OK) {
            fprintf(stderr, "basdata_test: unable to write integer to %s: %s\n", fn, basdata_rmsg(res));
            worked = false;
        }
    } while (value != 0);
    return worked;
}

static bool write_floats(const char *fn, FILE *fp)
{
    bool worked = true;
    int i = 0;
    double value = 0.0;
    do {
        value = floats[i++];
        basdata_res res = basdata_writef(value, fp);
        if (res != BASDATA_OK) {
            fprintf(stderr, "basdata_test: unable to write float to %s: %s\n", fn, basdata_rmsg(res));
            worked = false;
        }
    } while (value != 0.0);
    return worked;
}

static int write_file(const char *fn)
{
    FILE *fp = fopen(fn, "wb");
    if (fp) {
        int status = 0;
        if (!write_strings(fn, fp))
            status = 1;
        if (!write_integers(fn, fp))
            status = 1;
        if (!write_floats(fn, fp))
            status = 1;
        fclose(fp);
        return status;
    }
    else {
        fprintf(stderr, "basdata_test: unable to open %s for writing: %s\n", fn, strerror(errno));
        return 1;
    }
}

int main(int argc, char **argv)
{
    int status = 0;
    status += check_file("bdata");
    status += write_file("cdata");
    status += check_file("cdata");
    return status;
}
