/* Copytight (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) Magnus Holmgren <magnus@kibibyte.se>,
 *                               <holmgren@lysator.liu.se>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "cez_template.h"
#include <iconv.h>

#define JNK 0177
#define UNI_REPLACEMENT_CHAR 0x0000FFFD
#define UNI_REPLACEMENT_CHAR_UTF8 "\xEF\xBF\xBD"
#define ICONV_CHUNK_SIZE 1024

/* utf8_from_imaputf7() ***********************************************
 *
 * Convert a string encoded as modified UTF-7 to UTF-8.
 *       pool: Target pool for storage
 *          t: The string to convert
 *
 * Returns: A new, UTF-8-encoded string
 *
 * Note: This function tries hard to return something useful in case
 *       the input string is invalid in some way, rather than bail out
 *       and return NULL.
 **********************************************************************/

char *utf8_from_imaputf7(struct pool *pool, char *t)
{
    struct buffer *b = buffer_create(pool, 64);
    BOOL base64mode = NIL;
    int i = 0, j = 0;
    unsigned char buf[4];
    unsigned char *s;
    unsigned long scalar; /* Unicode scalar value */
    unsigned char c = ' ';

    static char decode_base64[256] = {
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,076,077,JNK,JNK,JNK,
        064,065,066,067,070,071,072,073,074,075,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,000,001,002,003,004,005,006,007,010,011,012,013,014,015,016,
        017,020,021,022,023,024,025,026,027,030,031,JNK,JNK,JNK,JNK,JNK,
        JNK,032,033,034,035,036,037,040,041,042,043,044,045,046,047,050,
        051,052,053,054,055,056,057,060,061,062,063,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,
        JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK,JNK
    };

    if (!t) return NULL;

    for (s = (unsigned char*)t; *s; s++) {
        if (base64mode) {
            if (*s == '-') {
                if (j != 0) /* Some junk left */ {
                    bputs(b, UNI_REPLACEMENT_CHAR_UTF8);
                }
                base64mode = NIL;
                continue;
            }

            if (decode_base64[*s] == JNK) {
                break;  /* Bail out */
            }

            switch (i++) {
            case 0:
                /* Top 6 bits of the first octet */
                c = decode_base64[*s] << 2; break;
            case 1:
                /* Top 2 bits are bottom 2 bits of the first octet */
                buf[j++] = c | (decode_base64[*s] >> 4);
                /* and bottom 4 bits are top 4 bits of the second */
                c = decode_base64[*s] << 4; break;
            case 2:
                /* Top 4 bits are bottom 4 bits of the second octet */
                buf[j++] = c | (decode_base64[*s] >> 2);
                /* and bottom 2 bits are top 2 bits of the third */
                c = decode_base64[*s] << 6; break;
            case 3:
                /* Bottom 6 bits of the third octet */
                buf[j++] = c | decode_base64[*s];
                i = 0;
            }

            /* Check if we have a complete UTF-16 character */
            if (j == 4) { /* We should now have a surrogate pair */
                scalar = ((buf[0] & 3) << 18) + (buf[1] << 10)
                    + ((buf[2] & 3) << 8) + buf[3] + 0x10000;
            }
            else if (j == 2) {
                if (buf[0] < 0xD8 || buf[0] > 0xDF) {
                    scalar = (buf[0] << 8) + buf[1];
                }
                else if (buf[0] > 0xDB) {
                    /* Error - invalid surrogate */
                    scalar = UNI_REPLACEMENT_CHAR;
                }
                else {
                    /* High surrogate found - need low surrogate */
                    continue;
                }
            }
            else continue; /* Odd number of bytes */

            if (scalar >= 0x110000) scalar = UNI_REPLACEMENT_CHAR;

            if (scalar < 0x80) {
                bputc(b, (unsigned char)scalar);
                j = 0;
                continue;
            } else if (scalar < 0x800) {
                bputc(b, 0xC0 | (scalar >> 6));
            } else if (scalar < 0x10000) {
                bputc(b, 0xE0 | (scalar >> 12));
            } else {
                bputc(b, 0xF0 | (scalar >> 18));
            }

            if (scalar >= 0x10000)
                bputc(b, 0x80 | ((scalar >> 12) & 0x3F));
            if (scalar >= 0x800)
                bputc(b, 0x80 | ((scalar >> 6) & 0x3F));
            bputc(b, 0x80 | (scalar & 0x3F));
            j = 0;

        }
        else /* !base64mode */ {
            if (*s == '&') {
                if (*(s+1) == '-') {
                    bputc(b, '&');
                    s++;
                }
                else {
                    base64mode = T;
                    i = 0; j = 0; c = 0;
                }
            }
            else {
                bputc(b, *s);
            }
        }
     }
     return buffer_fetch(b, 0, buffer_size(b), NIL);
}


/* utf8_to_imaputf7() *************************************************
 *
 * Convert a string encoded as UTF-8 to modified UTF-7.
 *       pool: Target pool for storage
 *          t: The string to convert
 *
 * Returns: A new string encoded as modified UTF-7
 *
 * Note: This function tries hard to return something useful in case
 *       the input string is invalid in some way, rather than bail out
 *       and return NULL.
 **********************************************************************/

char *utf8_to_imaputf7(struct pool *pool, char *t)
{
    unsigned char *s;
    struct buffer *b = buffer_create(pool, 64);
    BOOL base64mode = NIL;
    unsigned long scalar;
    unsigned int L, i = 0, j;
    unsigned char buf[4];
    unsigned char c = 0;

    static char encode_base64[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+,";
    if (!t) return NULL;

    for (s = (unsigned char *)t; *s; s++) {
        if (*s < 0x7f && *s >= 0x20) {
            if (base64mode) {
                switch (i) {
                case 1:
                    /* Remaining bottom 2 bits of the last octet */
                    bputc(b, encode_base64[c << 4]); break;
                case 2:
                    /* Remaining bottom 4 bits of the last octet */
                    bputc(b, encode_base64[c << 2]);
                }
                bputc(b, '-');
                base64mode = NIL;
            }
            bputc(b, *s);
            if (*s == '&') {
                bputc(b, '-');
            }
        }
        else {
            if (*s < 0x80) {
                L = 0; scalar = *s;
            } else if ((*s & 0xE0) == 0xC0) {
                L = 1; scalar = (*s & 0x1F);
            } else if ((*s & 0xF0) == 0xE0) {
                L = 2; scalar = (*s & 0x0F);
            } else if ((*s & 0xF8) == 0xF0) {
                L = 3; scalar = (*s & 0x07);
            } else if ((*s & 0xFC) == 0xF8) {
                L = 4; scalar = (*s & 0x03);
            } else if ((*s & 0xFE) == 0xFC) {
                L = 5; scalar = (*s & 0x01);
            } else {
                L = 0; scalar = UNI_REPLACEMENT_CHAR;
            }

            for (j = 0; j < L; j++) {
                s++;
                if ((*s & 0xC0) == 0x80) {
                    scalar <<= 6;
                    scalar |= (*s & 0x3F);
                }
                else {
                    s--;
                    scalar = UNI_REPLACEMENT_CHAR;
                }
            }

            if (!base64mode) {
                bputc(b, '&');
                base64mode = T;
                i = 0;
            }
            if (scalar <= 0xFFFF) {
                buf[1] = scalar & 0xFF;
                scalar >>= 8;
                buf[0] = scalar & 0xFF;
                L = 2;
            }
            else {
                scalar -= 0x10000UL;
                buf[3] = scalar & 0xFF;
                scalar >>= 8;
                buf[2] = 0xDC | (scalar & 0x03);
                scalar >>= 2;
                buf[1] = scalar & 0xFF;
                scalar >>= 8;
                buf[0] = 0xD8 | (scalar & 0x03);
                L = 4;
            }

            for (j = 0; j < L; j++) {
                switch (i++) {
                case 0:
                    /* Top 6 bits of the first octet */
                    bputc(b, encode_base64[(buf[j] >> 2) & 0x3F]);
                    c = (buf[j] & 0x03); break;
                case 1:
                    /* Bottom 2 bits of the first octet, and top 4 bits of the second */
                    bputc(b, encode_base64[(c << 4) |
                                           ((buf[j] >> 4) & 0x0F)]);
                    c = (buf[j] & 0x0F); break;
                case 2:
                    /* Bottom 4 bits of the second octet and top 2 bits of the third */
                    bputc(b, encode_base64[(c << 2) |
                                           ((buf[j] >> 6) & 0x03)]);
                    /* Bottom 6 bits of the third octet */
                    bputc(b, encode_base64[buf[j] & 0x3F]);
                    i = 0;
                }
            }

        }
    }
    if (base64mode) {
        switch (i) {
        case 1:
            /* Remaining bottom 2 bits of the last octet */
            bputc(b, encode_base64[c << 4]); break;
        case 2:
            /* Remaining bottom 4 bits of the last octet */
            bputc(b, encode_base64[c << 2]);
        }
        bputc(b, '-');
        base64mode = NIL;
    }
    return buffer_fetch(b, 0, buffer_size(b), NIL);
}


/* utf8_from_string() ************************************************
 *
 * Convert a string with given character encoding to UTF-8
 *
 *    pool: Pool to allocate memory from
 * charset: Charset of input string
 *       t: String to convert
 *     len: Length of string
 ***********************************************************************/

char *utf8_from_string(struct pool *pool, char *charset, char *t, unsigned long len)
{
    struct buffer *b;
    char chunk[ICONV_CHUNK_SIZE];
    char *outbuf;
    size_t outbytesleft;
    size_t result;
    int i;
    iconv_t cd = iconv_open("UTF-8", charset);
    b = buffer_create(pool, 1024);

    if (cd == (iconv_t)(-1))
        cd = iconv_open("UTF-8", "ISO-8859-1");

    if (cd == (iconv_t)(-1))
        /* Should now be impossible */
        return buffer_fetch(b, 0, buffer_size(b), NIL);

    while (len) {
        outbuf = chunk;
        outbytesleft = ICONV_CHUNK_SIZE;
        result = iconv(cd, &t, (size_t*)&len, &outbuf, &outbytesleft);
        for (i = 0; i < ICONV_CHUNK_SIZE - outbytesleft; i++) {
            bputc(b, chunk[i]);
        }
        if (result == (size_t)(-1)) switch (errno) {
        case E2BIG:
            break;
        case EILSEQ:
        case EINVAL:
            /* Try skipping a byte */
            t++;
            len--;
            bputs(b, UNI_REPLACEMENT_CHAR_UTF8);
            break;
        default:
            iconv_close(cd);
            return NULL;
        }
    }

    iconv_close(cd);
    return buffer_fetch(b, 0, buffer_size(b), NIL);
}

BOOL utf8_print(char *charset, char *fallback_charset,
                unsigned char **dst, unsigned long dst_size,
                unsigned char **src, unsigned long src_size)
{

    iconv_t cd = iconv_open("UTF-8", charset);
    if (cd == (iconv_t)(-1) && fallback_charset)
        cd = iconv_open("UTF-8", fallback_charset);

    if (cd == (iconv_t)(-1))
        return(NIL);

    while (iconv(cd, (char**)src, (size_t*)&src_size,
                     (char**)dst, (size_t*)&dst_size) == (size_t)(-1)) {
        switch (errno) {
        case EILSEQ:
        case EINVAL:
            /* Try skipping a byte */
            (*src)++;
            src_size--;
            if (dst_size >= sizeof(UNI_REPLACEMENT_CHAR_UTF8)) {
                strcpy((char*)*dst, UNI_REPLACEMENT_CHAR_UTF8);
            }
            if (errno == EILSEQ) break;
        case E2BIG:
        default:
            iconv_close(cd);
            return NIL;
        }
    }
    iconv_close(cd);
    return T;
}

/* utf8_prune() ********************************************************
 *
 * Like string_prune, but counts UTF-8 multibyte sequences as units.
 *   pool: Target pool
 *      s: UTF-8 string to prune
 * maxlen: Maximum length of string before pruning applies.
 *
 * Returns: Pruned string, maximum maxlen characters (not bytes), not
 *          counting the terminating null.
 ***********************************************************************/

char *utf8_prune(struct pool *pool, char *s, unsigned long maxlen)
{
    char *result;
    unsigned long cutoff = 0;
    unsigned long L;
    unsigned long i = 0;

    if (maxlen < (strlen("...") + 1))
        return (s);

    for (L = 0; L < maxlen; L++) {
        if (s[i] == '\0') return s;
        if (L == maxlen - strlen("...")) cutoff = i;

        if (s[i] & 0x80)
            while (s[i] & 0x80) i++;
        else
            i++;
    }

    result = pool_alloc(pool, cutoff + 4);
    memcpy(result, s, cutoff);
    strcpy(result + cutoff, "...");

    return (result);

}

/* utf8_count_chars() *****************************************************
 *
 * Calculate number of characters in UTF-8 byte sequence.
 *      s: Input string
 *  bytes: Number of bytes to consider
 *
 * Returns: Number of characters
 *
 *************************************************************************/

unsigned long utf8_count_chars(char *t, unsigned long bytes)
{
    unsigned char c, *s = (unsigned char *)t;
    unsigned long chars = 0;

    /* Count everything apart from continuation characters */
    while ((bytes > 0) && ((c = *s++) != '\0')) {
        if ((c < 0x80) || (c > 0xbf))
            chars++;
        bytes--;
    }

    return(chars);
}

/* ====================================================================== */

/* Windows codepage 1252 is a superset of (the useful part of) ISO-8859-1.
 *
 * Characters in the range 0x80 -> 0x9f are reserved for magic control
 * characters in 8859-1 (similar to 0x00 -> 0x1f). Windows CP 1252 uses
 * this range for some additional glyphs including "smartquote" characters.
 *
 *   http://en.wikipedia.org/wiki/ISO_8859-1
 *   http://en.wikipedia.org/wiki/Windows-1252
 *   http://czyborra.com/charsets/codepages.html
 *
 * Windows treats the two as synonymous and will happily send documents
 * signed ISO-8859-1 containing smart quote characters. We hypothesise this
 * happens when people try to cut and paste from a Word document into a Web
 * browser form: it doesn't seem to happen in just normal use.
 *
 * Firefox on my Linux box renders ISO-8859-1 characters 0x80 -> 0x9f using
 * the appropriate CP1252 glyphs. Presumably to ensure compability with
 * broken Websites. Other MUAs will not be as cooperative, so lets try some
 * transliteration into strict ISO-8859-1, at least until someone moans.
 */

static void
transliterate_1252(unsigned char *s)
{
    unsigned char c, table[] = {
        /* 33 characters from 0x80 to 0xa0 inclusive (0xa0 is &nbsp;) */

        0x80,   /* 0x80  U+20AC  EURO SIGN */
        0x81,   /* 0x81          Undefined */
        '\'',   /* 0x82  U+201A  SINGLE LOW-9 QUOTATION MARK */
        0x83,   /* 0x83  U+0192  LATIN SMALL LETTER F WITH HOOK */

        '"',    /* 0x84  U+201E  DOUBLE LOW-9 QUOTATION MARK */
        0x85,   /* 0x85  U+2026  HORIZONTAL ELLIPSIS */
        0x86,   /* 0x86  U+2020  DAGGER */
        0x87,   /* 0x87  U+2021  DOUBLE DAGGER */

        0x88,   /* 0x88  U+02C6  MODIFIER LETTER CIRCUMFLEX ACCENT */
        0x89,   /* 0x89  U+2030  PER MILLE SIGN */
        0x8a,   /* 0x8a  U+0160  LATIN CAPITAL LETTER S WITH CARON */
        '<',    /* 0x8b  U+2039  SINGLE LEFT-POINTING ANGLE QUOTATION MARK */

        0x8c,   /* 0x8c  U+0152  LATIN CAPITAL LIGATURE OE */
        0x8d,   /* 0x8d          Undefined */
        0x8e,   /* 0x8e  U+017D  LATIN CAPITAL LETTER Z WITH CARON */
        0x8f,   /* 0x8f          Undefined */

        0x90,   /* 0x90          Undefined */
        '\'',   /* 0x91  U+2018  LEFT SINGLE QUOTATION MARK */
        '\'',   /* 0x92  U+2019  RIGHT SINGLE QUOTATION MARK */
        '"',    /* 0x93  U+201C  LEFT DOUBLE QUOTATION MARK */

        '"',    /* 0x94  U+201D  RIGHT DOUBLE QUOTATION MARK */
        0x95,   /* 0x95  U+2022  BULLET */
        '-',    /* 0x96  U+2013  EN DASH */
        '-',    /* 0x97  U+2014  EM DASH */

        '~',    /* 0x98  U+02DC  SMALL TILDE */
        0x99,   /* 0x99  U+2122  TRADE MARK SIGN */
        0x9a,   /* 0x9a  U+0161  LATIN SMALL LETTER S WITH CARON */
        '>',    /* 0x9b  U+203A  SINGLE RIGHT-POINTING ANGLE QUOTATION MARK */

        0x9c,   /* 0x9c  U+0153  LATIN SMALL LIGATURE OE */
        0x9d,   /* 0x9d          Undefined */
        0x9e,   /* 0x9e  U+017E  LATIN SMALL LETTER Z WITH CARON */
        0x9f,   /* 0x9f  U+0178  LATIN CAPITAL LETTER Y WITH DIAERESIS */

        ' '     /* 0xa0  U+00A0  NO-BREAK SPACE */
    };

    while ((c=*s)) {
        if ((c >= 0x80) && (c <= 0xa0))
            *s = table[(c-0x80)];
        s++;
    }
}

/* Transliterate U+2010 through U+201F (subset of the range above)
 * into single byte ISO-8859-1. UTF-8 codes are: 0xe2 0x80 [0x90->0x9f]
 */

static unsigned char utf8_1252_char(unsigned char c)
{
    unsigned char table[] = {
        0x00,   /* U+2010                              */
        0x00,   /* U+2011                              */
        0x00,   /* U+2012                              */
         '-',   /* U+2013  EN DASH                     */
         '-',   /* U+2014  EM DASH                     */
        0x00,   /* U+2015                              */
        0x00,   /* U+2016                              */
        0x00,   /* U+2017                              */
        '\'',   /* U+2018  LEFT SINGLE QUOTATION MARK  */
        '\'',   /* U+2019  RIGHT SINGLE QUOTATION MARK */
        '\'',   /* U+201A  SINGLE LOW-9 QUOTATION MARK */
        0x00,   /* U+201B                              */
        '"',    /* U+201C  LEFT DOUBLE QUOTATION MARK  */
        '"',    /* U+201D  RIGHT DOUBLE QUOTATION MARK */
        '"',    /* U+201E  DOUBLE LOW-9 QUOTATION MARK */
        0x00    /* U+201F                              */
    };

    if ((c >= 0x90) || (c <= 0x9f))
        return table[c - 0x90];

    return(0x00);
}

/* Unicode characters 0x80 through 0x7ff encoding using following rule:
 *
 *   U-00000080 - U-000007FF:  110xxxxx 10xxxxxx
 *
 * which means that the correct range for 0x80 to 0xff is:
 *
 *   U-00000080 - U-000000FF:  1100001x 10xxxxxx
 *
 * (NB: the top bit is not stripped before encoding).
 */

/* utf8_is_8859_1() *****************************************************
 *
 * Check whether UTF-8 script is actually just ISO-8859-1 or Windows
 * code page 1252 characters.
 *
 ***********************************************************************/

BOOL
utf8_is_8859_1(char *s0)
{
    unsigned char *s = (unsigned char *)s0;

    if (!(s && s[0]))
        return(T);

    while (*s) {
        /* Transliterate smartquote characters into Latin-1 approximations */
        if ((s[0] == 0xe2) && (s[1] == 0x80) && utf8_1252_char(s[2])) {
            s += 3;
        } else if (*s & 0x80) {
            if (!(((s[0] & 0xfe) == (0xc2)) && ((s[1] & 0xc0) == (0x80))))
                return(NIL); /* UTF-8 char outside the ISO-8859-1 range */
            s += 2;
        } else
            s++;
    }
    return(T);
}

/* utf8_to_8859_1() *****************************************************
 *
 * Convert UTF-8 string containing only characters in the ISO-8859-1
 * range into ISO-8859-1. Transliterate illegal ISO-8859-1 characters
 * from Windows codepage 1252 into simple ASCII alternatives.
 *
 ***********************************************************************/

void
utf8_to_8859_1(char *s0)
{
    unsigned char *s = (unsigned char *)s0;
    unsigned char *d = s;
    unsigned char c;

    if (!(s && s[0]))
        return;

    while (*s) {
        /* Transliterate smartquote characters into Latin-1 approximations */
        if ((s[0] == 0xe2) && (s[1] == 0x80) && (c=utf8_1252_char(s[2]))) {
            *d++ = c;
            s += 3;
        } else if (*s & 0x80) {
            if (((s[0] & 0xfe) == (0xc2)) && ((s[1] & 0xc0) == (0x80))) {
                /* Falls within ISO-8859-1 range */
                if (s[0] & 0x01)
                    *d++ = 0x80 + 0x40 + (s[1] & 0x3f);
                else
                    *d++ = 0x80        + (s[1] & 0x3f);
                s += 2;
            } else {
                while (*s & 0x80)    /* Skip until next ascii character */
                    s++;
            }
        } else if (s != d) {         /* Copy or skip as appropriate */
            *d++ = *s++;
        } else {
            s++; d ++;
        }
    }
    if (*d != '\0')
        *d = '\0';

    /* Deal with illegal control characters in the Latin-1 range */
    /* Should never happen with browser sending UTF-8 */
    transliterate_1252((unsigned char *)s0);
}

/* ====================================================================== */

/* Some small utility routines to process UTF-8 string char by char */

void utf8_skip_char(char **sp)
{
    unsigned char *s = (unsigned char *)(*sp);

    if (*s & 0x80) {
        s++;
        while ((*s & 0xc0) == 0x80)
            s++;
    } else if (*s)
        s++;

    *sp = (char *)s;
}

void utf8_find_offset(char **sp, unsigned long offset)
{
    unsigned long i;

    for (i=0; i < offset; i++)
        utf8_skip_char(sp);
}

unsigned long utf8_len(char *s)
{
    unsigned long result = 0;

    while (*s) {
        utf8_skip_char(&s);
        result++;
    }

    return(result);
}

void utf8_print_char(struct buffer *b, char **sp)
{
    unsigned char *s = (unsigned char *)(*sp);

    if (*s & 0x80) {
        bputc(b, *s++);
        while ((*s & 0xc0) == 0x80)
            bputc(b, *s++);
    } else if (*s)
        bputc(b, *s++);

    *sp = (char *)s;
}
