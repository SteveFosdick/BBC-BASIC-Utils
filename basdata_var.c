#include "basdata.h"

basdata_res basdata_readv(FILE *fp, basdata_var *var)
{
    unsigned char buf[6];
    if (fread(buf, 2, 1, fp) == 1) {
        unsigned vtype = buf[0];
        if (vtype == 0) {
            int len = buf[1];
            var->type = BASDATA_STRING;
            var->u.s.len = len;
            return basdata_sread(fp, var->u.s.str, len);
        }
        else if (vtype == 0x40) {
            var->type = BASDATA_INTEGER;
            if (fread(buf+2, 3, 1, fp) == 1) {
                var->u.i = (buf[1] << 24) | (buf[2] << 16) | (buf[3] << 8) | buf[4];
                return BASDATA_OK;
            }
            else if (ferror(fp))
                return BASDATA_IOERR;
            else
                return BASDATA_EOF;
        }
        else if (vtype == 0xff) {
            var->type = BASDATA_FLOAT;
            if (fread(buf+2, 4, 1, fp) == 1) {
                var->u.f = basdata_fp2d(buf+1);
                return BASDATA_OK;
            }
            else if (ferror(fp))
                return BASDATA_IOERR;
            else
                return BASDATA_EOF;
        }
        else
            return BASDATA_BADTYPE;
    }
    else if (ferror(fp))
        return BASDATA_IOERR;
    else
        return BASDATA_EOF;
}

basdata_res basdata_writev(basdata_var *var, FILE *fp)
{
    switch(var->type) {
        case BASDATA_STRING:
            return basdata_writes(var->u.s.str, var->u.s.len, fp);
        case BASDATA_INTEGER:
            return basdata_writei(var->u.i, fp);
        case BASDATA_FLOAT:
            return basdata_writef(var->u.f, fp);
        default:
            return BASDATA_BADTYPE;
    }
}
