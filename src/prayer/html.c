/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */

#include "cez_prayer.h"

void
html_quote_char(struct buffer *b, unsigned char c)
{
    if (c > 127) {
        bputc(b, c);
    } else
        switch (c) {
        case '"':
            bputs(b, "&quot;");
            break;
        case '&':
            bputs(b, "&amp;");
            break;
        case '<':
            bputs(b, "&lt;");
            break;
        case '>':
            bputs(b, "&gt;");
            break;
        default:
            bputc(b, c);
        }
}

void
html_quote_string(struct buffer *b, char *t)
{
    unsigned char *s = (unsigned char *) t;
    unsigned char c;

    if (!s)
        bputs(b, "(nil)");
    else
        while ((c = *s++))
            html_quote_char(b, c);
}

