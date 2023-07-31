#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct token {
    char text[14];
    uint8_t flags;
};

#define SPC_BEFORE 0x01
#define SPC_AFTER  0x02
#define INC_INDENT 0x04
#define DEC_INDENT 0x08
#define SKIP_EOL   0x10
#define SKIP_TWO   0x20

static const struct token high_tokens[] = {
    { "AND",           SPC_BEFORE|SPC_AFTER }, // 85
    { "DIV",           SPC_BEFORE|SPC_AFTER }, // 86
    { "EOR",           SPC_BEFORE|SPC_AFTER }, // 87
    { "MOD",           SPC_BEFORE|SPC_AFTER }, // 88
    { "OR",            SPC_BEFORE|SPC_AFTER }, // 89
    { "IN",            SPC_BEFORE|SPC_AFTER }, // 8A
    { "APPEND",        SPC_BEFORE|SPC_AFTER }, // 8B
    { "DO",            SPC_BEFORE|SPC_AFTER }, // 8C
    { "FILE",          SPC_BEFORE|SPC_AFTER }, // 8D
    { "OF",            SPC_BEFORE|SPC_AFTER }, // 8E
    { "RANDOM",        SPC_BEFORE|SPC_AFTER }, // 8F
    { "REF",           SPC_BEFORE|SPC_AFTER }, // 90
    { "STEP",          SPC_BEFORE|SPC_AFTER }, // 91
    { "TAB(",          0                    }, // 92
    { "THEN",          SPC_BEFORE|SPC_AFTER }, // 93
    { "TO",            SPC_BEFORE|SPC_AFTER }, // 94
    { "USING",         SPC_BEFORE|SPC_AFTER }, // 95
    { ":",             0                    }, // 96
// Functions with no argument/
    { "FALSE",         0                    }, // 97
    { "PI",            0                    }, // 98
    { "TRUE",          0                    }, // 99
    { "COUNT",         0                    }, // 9A
    { "EOD",           0                    }, // 9B
    { "GET",           0                    }, // 9C
    { "POS",           0                    }, // 9D
    { "SIZE",          0                    }, // 9E
    { "FREE",          0                    }, // 9F
    { "VPOS",          0                    }, // A0
    { "GET$",          0                    }, // A1
// Functions with one string argument.
    { "LEN",           SPC_AFTER            }, // A2
    { "ORD",           0                    }, // A3
    { "VAL",           0                    }, // A4
// Functions with one numeric argument.
    { "ACS",           0                    }, // A5
    { "ASN",           0                    }, // A6
    { "ATN",           0                    }, // A7
    { "COS",           0                    }, // A8
    { "DEG",           0                    }, // A9
    { "EXP",           0                    }, // AA
    { "LN",            0                    }, // AB
    { "LOG",           0                    }, // AC
    { "RAD",           0                    }, // AD
    { "SIN",           0                    }, // AE
    { "SQR",           0                    }, // AF
    { "TAN",           0                    }, // B0
    { "INT",           0                    }, // B1
    { "SGN",           0                    }, // B2
    { "ABS",           0                    }, // B3
    { "ADVAL",         0                    }, // B4
    { "EOF",           0                    }, // B5
    { "EXT",           0                    }, // B6
    { "INKEY",         0                    }, // B7
    { "NOT",           0                    }, // B8
    { "USR",           0                    }, // B9
    { "CHR$",          0                    }, // BA
    { "INKEY$",        0                    }, // BB
    { "STR$",          0                    }, // BC
    { "RND",           0                    }, // BD
    { "POINT(",        0                    }, // BE
    { "MODE",          0                    }, // BF
    { "PAGE",          0                    }, // C0
    { "TIME",          0                    }, // C1
    { "WIDTH",         SPC_AFTER            }, // C2
    { "ZONE",          SPC_AFTER            }, // C3
    { "CLEAR",         SPC_AFTER            }, // C4
    { "NULL",          SPC_AFTER            }, // C5
    { "CLG",           SPC_AFTER            }, // C6
    { "CLS",           SPC_AFTER            }, // C7
    { "NEW",           0                    }, // C8
    { "STOP",          SPC_AFTER            }, // C9
    { "RESTORE",       SPC_AFTER            }, // CA
    { "EXEC",          SPC_AFTER            }, // CB
    { "GOTO",          SPC_AFTER            }, // CC
    { "DEL",           0                    }, // CD
    { "//",            SPC_AFTER|SKIP_EOL   }, // CE
    { "DATA",          SPC_AFTER|SKIP_EOL   }, // CF
    { "RUN",           SPC_AFTER            }, // D0
    { "SAVE",          0                    }, // D1
    { "LOAD",          0                    }, // D2
    { "DELETE",        0                    }, // D3
    { "SELECT OUTPUT", SPC_AFTER            }, // D4
    { "VDU",           SPC_AFTER            }, // D5
    { "DIM",           SPC_AFTER            }, // D6
    { "OSCLI",         SPC_AFTER            }, // D7
    { "OPEN",          SPC_AFTER            }, // D8
    { "INPUT",         SPC_AFTER            }, // D9
    { "WRITE",         SPC_AFTER            }, // DA
    { "READ",          SPC_AFTER            }, // DB
    { "CLOSE",         SPC_AFTER            }, // DC
    { "DRAW",          SPC_AFTER            }, // DD
    { "GCOL",          SPC_AFTER            }, // DE
    { "MOVE",          SPC_AFTER            }, // DF
    { "PLOT",          SPC_AFTER            }, // E0
    { "SOUND",         SPC_AFTER            }, // E1
    { "ENVELOPE",      SPC_AFTER            }, // E2
    { "COLOUR",        SPC_AFTER            }, // E3
    { "UNTIL",         SPC_AFTER|DEC_INDENT }, // E4
    { "END",           SPC_AFTER            }, // E5
    { "NEXT",          SPC_AFTER|DEC_INDENT }, // E6
    { "RETURN",        SPC_AFTER            }, // E7
    { "IMPORT",        SPC_AFTER            }, // E8
    { "OTHERWISE",     SPC_AFTER            }, // E9
    { "ELSE",          SPC_BEFORE|SPC_AFTER }, // EA
    { "WHEN",          SPC_BEFORE|SPC_AFTER }, // EB
    { "ELIF",          SPC_BEFORE|SPC_AFTER }, // EC
    { "FUNC",          SPC_AFTER|SKIP_TWO   }, // ED
    { "PROC",          SPC_AFTER|SKIP_TWO   }, // EE
    { "CASE",          SPC_AFTER            }, // EF
    { "REPEAT",        SPC_AFTER|INC_INDENT }, // F0
    { "IF",            SPC_AFTER            }, // F1
    { "WHILE",         0                    }, // F2
    { "FOR",           SPC_AFTER|INC_INDENT }, // F3
    { "PRINT",         SPC_AFTER            }, // F4
    { "AUTO",          0                    }, // F5
    { "RENUMBER",      0                    }, // F6
    { "EDIT",          0                    }, // F7
    { "LIST",          0                    }, // F8
    { "CONT",          0                    }, // F9
    { "DEBUG",         0                    }, // FA
    { "OLD",           0                    }, // FB
    { "READ ONLY",     SPC_AFTER            }, // FC
    { "CLOSED",        SPC_AFTER            }, // FD
};

struct outcfg {
    const char *fmt_lineno;
    const char *fmt_token;
    const char *str_prefix;
    const char *fmt_skipeol;
    const char *gen_suffix;
    const char *eol;
};

static const struct outcfg cfg_plain =
{
    "%5u ",
    "%s",
    "",
    "%s",
    "",
    "\n"
};

static const struct outcfg cfg_colour =
{
    "\e[38;5;160m%5u\e[0m ",
    "\e[38;5;45m%s\e[0m",
    "\e[38;5;166m",
    "\e[38;5;128m%s",
    "\e[0m"
};

static const struct outcfg cfg_dark =
{
    "\e[38;5;124m%5u\e[0m ",
    "\e[38;5;20m%s\e[0m",
    "\e[38;5;94m",
    "\e[38;5;128m%s",
    "\e[0m"
};

static const struct outcfg cfg_html =
{
    "<span class=\"lineno\">%5u</span> ",
    "<span class=\"token\">%s</span>",
    "<span class=\"string\">",
    "<span class=\"skipeol\">%s",
    "</span>"
};

static unsigned char *check_program(unsigned char *prog, unsigned char *file_end)
{
    file_end -= 2;
    while (prog <= file_end) {
        if (prog[0] != 0x0d)
            return NULL;
        if (prog[1] == 0xff)
            return prog;
        prog += prog[3];
    }
    return NULL;
}

static void comal2txt(const unsigned char *prog, const unsigned char *prog_end, const struct outcfg *ocfg)
{
    while (prog < prog_end) {
        unsigned lineno = (prog[1] << 8) | prog[2];
        unsigned len = prog[3];
        unsigned indent = prog[4];
        const unsigned char *end = prog + len;
        const unsigned char *ptr = prog + 5;
        bool did_space = true;
        bool need_space = false;
        bool in_str = false;
        printf(ocfg->fmt_lineno, lineno);
        while (indent--) {
            putchar(' ');
            putchar(' ');
        }
        while (ptr < end) {
            int ch = *ptr++;
            if (in_str) {
                putchar(ch);
                if (ch == '"') {
                    in_str = false;
                    fputs(ocfg->gen_suffix, stdout);
                }
            }
            else if (ch & 0x80) {
                if (ch >= 0x85 && ch <= 0xfd) {
                    const struct token *t = high_tokens + (ch - 0x85);
                    unsigned flags = t->flags;
                    if (!did_space && (need_space || (flags & SPC_BEFORE)))
                        putchar(' ');
                    if (flags & SKIP_EOL) {
                        printf(ocfg->fmt_skipeol, t->text);
                        fwrite(ptr, end-ptr, 1, stdout);
                        fputs(ocfg->gen_suffix, stdout);
                        break;
                    }
                    else
                        printf(ocfg->fmt_token, t->text);
                    did_space = need_space = false;
                    if (flags & SPC_AFTER)
                        need_space = true;
                    if (flags & SKIP_TWO)
						ptr += 2;
                }
            }
            else {
                if (ch == '"') {
                    in_str = true;
                    need_space = false;
                    fputs(ocfg->str_prefix, stdout);
                }
                else if (ch == ' ' || ch == ':') {
                    did_space = true;
                    need_space = false;
                }
                else
                    did_space = false;
                if (need_space && !did_space) {
                    need_space = false;
                    did_space = true;
                    putchar(' ');
                }
                putchar(ch);
            }
        }
        putchar('\n');
        prog = end;
    }
}

static unsigned char *load_file(const char *fn, unsigned char **end)
{
    FILE *fp = fopen(fn, "rb");
    if (fp) {
        if (!fseek(fp, 0L, SEEK_END)) {
            long size = ftell(fp);
            if (size == 0)
                fprintf(stderr, "comal2txt: %s is an empty file\n", fn);
            else {
                unsigned char *data = malloc(size);
                if (data) {
                    rewind(fp);
                    if (fread(data, size, 1, fp) == 1) {
                        fclose(fp);
                        *end = data + size;
                        return data;
                    }
                    else
                        fprintf(stderr, "comal2txt: read error on %s: %s", fn, strerror(errno));
                }
                else
                    fprintf(stderr, "comal2txt: out of memory reading %s\n", fn);
            }
        }
        else
            fprintf(stderr, "comal2txt: seek error on %s: %s", fn, strerror(errno));
        fclose(fp);
    }
    else
        fprintf(stderr, "comal2txt: unable to open '%s' for reading: %s", fn, strerror(errno));
    return NULL;
}

static void template(const char *fn, unsigned char *tmpl, unsigned char *tmpl_end, unsigned char *prog, unsigned char *prog_end, const struct outcfg *ocfg)
{
    unsigned char *ptr = tmpl;
    while (ptr < tmpl_end) {
        int ch = *ptr++;
        if (ch == '%') {
            fwrite(tmpl, ptr-tmpl-1, 1, stdout);
            ch = *ptr++;
            if (ch == 'f')
                fputs(fn, stdout);
            else if (ch == 'p')
                comal2txt(prog, prog_end, ocfg);
            else
                putchar(ch);
            tmpl = ptr;
        }
    }
    fwrite(tmpl, ptr-tmpl, 1, stdout);
}

static const char usage[]  = "Usage: comal2txt [-c] [-d] [-h] <file> [ ... ]\n";

int main(int argc, char **argv)
{
    const struct outcfg *ocfg = &cfg_plain;
    bool tmpl_next = false;
    const char *tmpl_name = NULL;
    while (--argc) {
        const char *arg = *++argv;
        if (tmpl_next) {
            tmpl_name = arg;
            tmpl_next = false;
        }
        else {
            if (arg[0] != '-')
                break;
            int opt = arg[1];
            switch(opt) {
                case 'c':
                    ocfg = &cfg_colour;
                    break;
                case 'd':
                    ocfg = &cfg_dark;
                    break;
                case 'h':
                    ocfg = &cfg_html;
                    break;
                case 't':
                    tmpl_next = true;
                    break;
                case 0:
                    fprintf(stderr, "comal2txt: missing option\n%s", usage);
                    return 1;
                default:
                    fprintf(stderr, "comal2txt: unrecognised option '%c'\n%s", opt, usage);
                    return 1;
            }
        }
    }
    if (argc == 0) {
        fputs(usage, stderr);
        return 1;
    }
    unsigned char *tmpl_data, *tmpl_end;
    if (tmpl_name) {
        if (!(tmpl_data = load_file(tmpl_name, &tmpl_end)))
            return 2;
    }
    else {
        tmpl_data = (unsigned char *)"%p";
        tmpl_end = tmpl_data + 2;
    }
    int status = 0;
    while (argc--) {
        const char *fn = *argv++;
        unsigned char *file_end;
        unsigned char *file = load_file(fn, &file_end);
        if (file) {
            unsigned char *prog_end;
            if ((prog_end = check_program(file, file_end)))
                template(fn, tmpl_data, tmpl_end, file, prog_end, ocfg);
            else {
                fprintf(stderr, "comal2txt: %s is not a COMAL program or is corrupt\n", fn);
                status = 3;
            }
            free(file);
        }
        else
            status = 2;
    }
    return status;
}
