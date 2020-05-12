/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Appendable string class which can be allocated from pool: less
 * memory management hassle than memblock. In fact should replace
 * memblock in many places. */

#include "cez_core_pool.h"

struct str *str_create(struct pool *pool, unsigned long blocksize)
{
    struct str *str = pool_alloc(pool, sizeof(struct str));

    str->len = 0;
    str->alloc = (blocksize > 0) ? blocksize : PREFERRED_STR_BLOCK_SIZE;
    str->s = xmalloc(str->alloc);
    str->s[0] = '\0';
    str->next = NULL;

    if (pool) {
        str->next = pool->str_list;
	pool->str_list = str;
    }

    return(str);
}

/* Return a string which is at least large enough for size */

void *
str_reserve(struct str *str, unsigned long size)
{
    if (size > str->alloc) {
        str->alloc = size;
        str->s = xrealloc(str->s, str->alloc);
    }

    return(str->s);
}

void str_free(struct str *str)
{
    free(str->s);
    str->s = NULL;
}

void str_free_chain(struct str *str)
{
    struct str *next;

    while (str) {
        next = str->next;
	str_free(str);
	str = next;
    }
}

void
str_putchar(struct str *str, unsigned char c)
{
    if (str->len < str->alloc) {
        str->s[str->len++] = c;
        return;
    }

    str->alloc *= 2;
    str->s = xrealloc(str->s, str->alloc);
    str->s[str->len++] = c;
}

static void str_print_ulong(struct str *str, unsigned long value)
{
    unsigned long tmp, weight;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    for (weight = 1, tmp = value / 10; tmp > 0; tmp /= 10)
        weight *= 10;

    for (tmp = value; weight > 0; weight /= 10) {
        if (value >= weight) {
            str_putc(str, '0' + (value / weight));
            value -= weight * (value / weight); /* Calculate remainder */
        } else
            str_putc(str, '0');
    }
}

static void str_print_hex(struct str *str, unsigned long value)
{
    unsigned long tmp, weight;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    for (weight = 1, tmp = value / 16; tmp > 0; tmp /= 16)
        weight *= 16;

    for (tmp = value; weight > 0; weight /= 16) {
        unsigned long digit = value / weight;
        unsigned char c =
            (digit > 9) ? ('a' + (digit - 10)) : ('0' + digit);

        str_putc(str, c);

        value -= weight * digit;
    }
}

void str_vaprintf(struct str *str, char *fmt, va_list ap)
{
    unsigned char *s, c;

    while ((c = *fmt++)) {
        if (c != '%') {
            str_putc(str, c);
        } else
            switch (*fmt++) {
            case 's':          /* string */
                if ((s = (unsigned char *) va_arg(ap, char *))) {
                    while ((c = *s++))
                        str_putc(str, c);
                } else
                    str_puts(str, "(nil)");
                break;
            case 'l':
                if (*fmt == 'u') {
                    str_print_ulong(str, va_arg(ap, unsigned long));
                    fmt++;
                } else if (*fmt == 'x') {
                    str_print_hex(str, va_arg(ap, unsigned long));
                    fmt++;
                } else
                    str_print_ulong(str, va_arg(ap, long));
                break;
            case 'd':
                if (*fmt == 'u') {
                    str_print_ulong(str, va_arg(ap, unsigned int));
                    fmt++;
                } else
                    str_print_ulong(str, va_arg(ap, int));
                break;
            case 'c':
                str_putc(str, (unsigned char) va_arg(ap, int));
                break;
            case 'x':
                str_print_hex(str, va_arg(ap, unsigned long));
                break;
            case '%':
                str_putc(str, '%');
                break;
            default:
                fprintf(stderr, "Bad format string to buffer_printf\n");
		exit(1);
            }
    }
}

void str_printf(struct str *str, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    str_vaprintf(str, fmt, ap);
    va_end(ap);
}

void str_puts(struct str *str, char *t)
{
    unsigned char *s = (unsigned char *) t;
    char c;

    if (!s)
        str_puts(str, "(nil)");
    else
        while ((c = *s++))
            str_putc(str, c);
}

static void
str_encode_common(struct str *str, char *t, char quote)
{
    unsigned char *s = (unsigned char *) t;
    static char hex[] = "0123456789abcdef";
    unsigned char c;

    while ((c=*s++)) {
        if (Uisalnum(c))
            str_putc(str, c);
        else {
            switch (c) {
            case '_':
            case '.':
            case '!':
            case '*':
            case '\'':
            case '(':
            case ')':
            case '-':
                str_putc(str, c);
                break;
            default:
                str_putc(str, quote);
                str_putc(str, hex[c >> 4]);
                str_putc(str, hex[c & 15]);
                break;
            }
        }
    }
}

void
str_encode_url(struct str *str, char *s)
{
    str_encode_common(str, s, '%');
}

void
str_encode_canon(struct str *str, char *s)
{
    str_encode_common(str, s, '*');
}

unsigned long str_len(struct str *str)
{
    return(str->len);
}

void
str_rewind(struct str *str, unsigned long offset)
{
    if (offset < str->len)
        str->len = offset;
}

void *
str_fetch(struct str *str)
{
    str_putc(str, '\0');
    str->len--;

    return(str->s);
}
