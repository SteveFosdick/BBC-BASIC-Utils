#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct token {
    char text[9];
    uint8_t flags;
};

#define SPC_BEFORE 0x01
#define SPC_AFTER  0x02
#define INC_INDENT 0x04
#define DEC_INDENT 0x08
#define SKIP_EOL   0x10

static const char low_tokens[8][9] = {
    "Missing",
    "No such",
    "Bad",
    "range",
    "variable",
    "Out of",
    "No",
    "space"
};

static const struct token high_tokens[] = {
    /* Operators */
    { "AND",      SPC_BEFORE|SPC_AFTER }, // 80
    { "DIV",      SPC_BEFORE|SPC_AFTER }, // 81
    { "EOR",      SPC_BEFORE|SPC_AFTER }, // 82
    { "MOD",      SPC_BEFORE|SPC_AFTER }, // 83
    { "OR",       SPC_BEFORE|SPC_AFTER }, // 84
    /* Auxilliary tokens */
    { "ERROR",    SPC_AFTER            }, // 85
    { "LINE",     SPC_AFTER            }, // 86
    { "OFF",      SPC_AFTER            }, // 87
    { "STEP",     SPC_BEFORE|SPC_AFTER }, // 88
    { "SPC",      SPC_AFTER            }, // 89
    { "TAB(",     0                    }, // 8A
    { "ELSE",     SPC_BEFORE|SPC_AFTER }, // 8B
    { "THEN",     SPC_BEFORE|SPC_AFTER }, // 8C
    /* Line number token */
    { "",         SPC_BEFORE|SPC_AFTER }, // 8D
    /* Oddly placed as added with BASIC 2 */
    { "OPENIN",   SPC_AFTER            }, // 8E
    /* Pseudo variable functions */
    { "PTR",      0                    }, // 8F
    { "PAGE",     0                    }, // 90
    { "TIME",     0                    }, // 91
    { "LOMEM",    0                    }, // 92
    { "HIMEM",    0                    }, // 93
    /* Numeric valued functions */
    { "ABS",      0                    }, // 94
    { "ACS",      0                    }, // 95
    { "ADVAL",    0                    }, // 96
    { "ASC",      0                    }, // 97
    { "ASN",      0                    }, // 98
    { "ATN",      0                    }, // 99
    { "BGET",     0                    }, // 9A
    { "COS",      0                    }, // 9B
    { "COUNT",    0                    }, // 9C
    { "DEG",      0                    }, // 9D
    { "ERL",      0                    }, // 9E
    { "ERR",      0                    }, // 9F
    { "EVAL",     0                    }, // A0
    { "EXP",      0                    }, // A1
    { "EXT",      0                    }, // A2
    { "FALSE",    0                    }, // A3
    { "FN",       0                    }, // A4
    { "GET",      0                    }, // A5
    { "INKEY",    0                    }, // A6
    { "INSTR(",   0                    }, // A7
    { "INT",      0                    }, // A8
    { "LEN",      0                    }, // A9
    { "LN",       0                    }, // AA
    { "LOG",      0                    }, // AB
    { "NOT",      0                    }, // AC
    { "OPENUP",   0                    }, // AD
    { "OPENOUT",  0                    }, // AE
    { "PI",       0                    }, // AF
    { "POINT(",   0                    }, // B0
    { "POS",      0                    }, // B1
    { "RAD",      0                    }, // B2
    { "RND",      0                    }, // B3
    { "SGN",      0                    }, // B4
    { "SIN",      0                    }, // B5
    { "SQR",      0                    }, // B6
    { "TAN",      0                    }, // B7
    { "TO",       SPC_BEFORE|SPC_AFTER }, // B8
    { "TRUE",     0                    }, // B9
    { "USR",      0                    }, // BA
    { "VAL",      0                    }, // BB
    { "VPOS",     0                    }, // BC
    { "CHR$",     0                    }, // BD
    /* String-valued functions */
    { "GET$",     0                    }, // BE
    { "INKEY$",   0                    }, // BF
    { "LEFT$(",   0                    }, // C0
    { "MID$(",    0                    }, // C1
    { "RIGHT$(",  0                    }, // C2
    { "STR$",     0                    }, // C3
    { "STRING$(", 0                    }, // C4
    /* EOF is an odd-ball */
    { "EOF",      0                    }, // C5
    /* Commands */
    { "AUTO",     0                    }, // C6
    { "DELETE",   0                    }, // C7
    { "LOAD",     0                    }, // C8
    { "LIST",     0                    }, // C9
    { "NEW",      0                    }, // CA
    { "OLD",      0                    }, // CB
    { "RENUMBER", 0                    }, // CC
    { "SAVE",     0                    }, // CD
    { "",         0                    }, // CE
    /* Pseudo-variable statements */
    { "PTR",      0                    }, // CF
    { "PAGE",     0                    }, // D0
    { "TIME",     0                    }, // D1
    { "LOMEM",    0                    }, // D2
    { "HIMEM",    0                    }, // D3
    /* Statements */
    { "SOUND",    SPC_AFTER            }, // D4
    { "BPUT",     SPC_AFTER            }, // D5
    { "CALL",     SPC_AFTER            }, // D6
    { "CHAIN",    SPC_AFTER            }, // D7
    { "CLEAR",    SPC_AFTER            }, // D8
    { "CLOSE",    SPC_AFTER            }, // D9
    { "CLG",      SPC_AFTER            }, // DA
    { "CLS",      SPC_AFTER            }, // DB
    { "DATA",     SPC_AFTER|SKIP_EOL   }, // DC
    { "DEF",      SPC_AFTER            }, // DD
    { "DIM",      SPC_AFTER            }, // DE
    { "DRAW",     SPC_AFTER            }, // DF
    { "END",      SPC_AFTER            }, // E0
    { "ENDPROC",  SPC_AFTER            }, // E1
    { "ENVELOPE", SPC_AFTER            }, // E2
    { "FOR",      SPC_AFTER|INC_INDENT }, // E3
    { "GOSUB",    SPC_AFTER            }, // E4
    { "GOTO",     SPC_AFTER            }, // E5
    { "GCOL",     SPC_AFTER            }, // E6
    { "IF",       SPC_AFTER            }, // E7
    { "INPUT",    SPC_AFTER            }, // E8
    { "LET",      SPC_AFTER            }, // E9
    { "LOCAL",    SPC_AFTER            }, // EA
    { "MODE",     SPC_AFTER            }, // EB
    { "MOVE",     SPC_AFTER            }, // EC
    { "NEXT",     SPC_AFTER|DEC_INDENT }, // ED
    { "ON",       SPC_AFTER            }, // EE
    { "VDU",      SPC_AFTER            }, // EF
    { "PLOT",     SPC_AFTER            }, // F0
    { "PRINT",    SPC_AFTER            }, // F1
    { "PROC",     0                    }, // F2
    { "READ",     SPC_AFTER            }, // F3
    { "REM",      SPC_AFTER|SKIP_EOL   }, // F4
    { "REPEAT",   SPC_AFTER|INC_INDENT }, // F5
    { "REPORT",   SPC_AFTER            }, // F6
    { "RESTORE",  SPC_AFTER            }, // F7
    { "RETURN",   SPC_AFTER            }, // F8
    { "RUN",      SPC_AFTER            }, // F9
    { "STOP",     SPC_AFTER            }, // FA
    { "COLOUR",   SPC_AFTER            }, // FB
    { "TRACE",    SPC_AFTER            }, // FC
    { "UNTIL",    SPC_AFTER|DEC_INDENT }, // FD
    { "WIDTH",    SPC_AFTER            }, // FE
    { "OSCLI",    SPC_AFTER            }  // FF
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

static unsigned bas2txt(const unsigned char *line, unsigned len, unsigned lineno, unsigned indent, const struct outcfg *ocfg)
{
    printf(ocfg->fmt_lineno, lineno);
    /* pre-scan the line for a decrease in indent. */
    int new_indent = indent;
    bool in_str = false;
    const unsigned char *ptr = line;
    const unsigned char *end = line + len;
    while (ptr < end) {
        int ch = *ptr++;
        if (in_str) {
            if (ch == '"')
                in_str = false;
        }
        else if (ch & 0x80) {
            const struct token *t = high_tokens + (ch & 0x7f);
            unsigned flags = t->flags;
            if (flags & DEC_INDENT)
                --new_indent;
            if (flags & INC_INDENT)
                ++new_indent;
            if (flags & SKIP_EOL)
                break;
        }
        else if (ch == '"')
            in_str = true;
    }
    /* a decrease in indent if applied immediately */
    if (new_indent < indent && new_indent >= 0)
        indent = new_indent;
    for (int i = indent; i; --i) {
        putchar(' ');
        putchar(' ');
    }
    /* now print the line */
    bool did_space = true;
    bool need_space = false;
    in_str = false;
    ptr = line;
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
            const struct token *t = high_tokens + (ch & 0x7f);
            unsigned flags = t->flags;
            if (!did_space && (need_space || (flags & SPC_BEFORE)))
                putchar(' ');
            if (ch == 0x8d) {
                unsigned b1 = ptr[0];
                unsigned lsb = ((b1 & 0x30) << 2) ^ ptr[1];
                unsigned msb = ((b1 & 0x0c) << 4) ^ ptr[2];
                printf("%u", (msb << 8) | lsb);
                ptr += 3;
            }
            else if (flags & SKIP_EOL) {
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
            if (ch >= 0x01 && ch <= 0x08)
                fputs(low_tokens[ch-1], stdout);
            else
                putchar(ch);
        }
    }
    putchar('\n');
    /* an increase in indent is applied afterwards ready for the next line */
    if (new_indent > indent)
        indent = new_indent;
    return indent;
}

static unsigned char *is_wilson(unsigned char *prog, unsigned char *file_end)
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

static void wilson2txt(unsigned char *prog, unsigned char *prog_end, const struct outcfg *ocfg)
{
    unsigned indent = 0;
    while (prog < prog_end) {
        unsigned lineno = (prog[1] << 8) | prog[2];
        unsigned len = prog[3];
        indent = bas2txt(prog+4, len-4, lineno, indent, ocfg);
        prog += len;
    }
}

static unsigned char *is_russell(unsigned char *prog, unsigned char *file_end)
{
    file_end -= 3;
    while (prog < file_end) {
        if (prog[0] == 0x00 && prog[1] == 0xff && prog[2] == 0xff)
            return prog;
        prog += prog[0];
        if (prog[-1] != 0x0d)
            return NULL;
    }
    return NULL;
}

static void russell2txt(unsigned char *prog, unsigned char *prog_end, const struct outcfg *ocfg)
{
    unsigned indent = 0;
    while (prog < prog_end) {
        unsigned len = prog[0];
        unsigned lineno = prog[1] | (prog[2] << 8);
        indent = bas2txt(prog + 3, len - 4, lineno, indent, ocfg);
        prog += len;
    }
}

static unsigned char *load_file(const char *fn, unsigned char **end)
{
    FILE *fp = fopen(fn, "rb");
    if (fp) {
        if (!fseek(fp, 0L, SEEK_END)) {
            long size = ftell(fp);
            if (size == 0)
                fprintf(stderr, "bas2txt: %s is an empty file\n", fn);
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
                        fprintf(stderr, "bas2txt: read error on %s: %s", fn, strerror(errno));
                }
                else
                    fprintf(stderr, "bas2txt: out of memory reading %s\n", fn);
            }
        }
        else
            fprintf(stderr, "bas2txt: seek error on %s: %s", fn, strerror(errno));
        fclose(fp);
    }
    else
        fprintf(stderr, "bas2txt: unable to open '%s' for reading: %s", fn, strerror(errno));
    return NULL;
}

static void template(const char *fn, unsigned char *tmpl, unsigned char *tmpl_end, unsigned char *prog, unsigned char *prog_end,
                     const struct outcfg *ocfg, void (*func)(unsigned char *prog, unsigned char *prog_end, const struct outcfg *ocfg))
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
                func(prog, prog_end, ocfg);
            else
                putchar(ch);
            tmpl = ptr;
        }
    }
    fwrite(tmpl, ptr-tmpl, 1, stdout);
}

static const char usage[]  = "Usage: bas2txt [-c] [-d] [-h] <file> [ ... ]\n";

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
                    fprintf(stderr, "bas2txt: missing option\n%s", usage);
                    return 1;
                default:
                    fprintf(stderr, "bas2txt: unrecognised option '%c'\n%s", opt, usage);
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
            if ((prog_end = is_wilson(file, file_end)))
                template(fn, tmpl_data, tmpl_end, file, prog_end, ocfg, wilson2txt);
            else if ((prog_end = is_russell(file, file_end)))
                template(fn, tmpl_data, tmpl_end, file, prog_end, ocfg, russell2txt);
            else {
                fprintf(stderr, "bas2txt: %s is not a BBC BASIC program or is corrupt\n", fn);
                status = 3;
            }
            free(file);
        }
        else
            status = 2;
    }
    return status;
}
