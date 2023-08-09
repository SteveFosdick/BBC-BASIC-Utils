#include "basdata.h"
#include <errno.h>
#include <string.h>

int main(int argc, char **argv)
{
    int status = 0;
    while (--argc) {
        const char *fn = *++argv;
        FILE *fp = fopen(fn, "rb");
        if (fp) {
            basdata_var var;
            basdata_res res;
            while ((res = basdata_readv(fp, &var)) == BASDATA_OK) {
                switch(var.type) {
                    case BASDATA_STRING:
                        if (var.u.s.len)
                            printf("S: %.*s\n", var.u.s.len, var.u.s.str);
                        else
                            fputs("S:\n", stdout);
                        break;
                    case BASDATA_INTEGER:
                        printf("I: %12d 0x%08X\n", var.u.i, var.u.i);
                        break;
                    case BASDATA_FLOAT:
                        printf("F: %g\n", var.u.f);
                    default:
                        break;
                }
            }
            if (res != BASDATA_EOF) {
                status = 1;
                fprintf(stderr, "basdata2txt: %s on %s\n", basdata_rmsg(res), fn);
            }
            fclose(fp);
        }
        else {
            fprintf(stderr, "basdata2txt: unable to open '%s' for reading: %s\n", fn, strerror(errno));
            status = 1;
        }
    }
    return status;
}
