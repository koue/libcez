/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#ifndef _CEZ_CORE_H
#define _CEZ_CORE_H

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pool:
 *  Memory allocated in pools which can be freed as a single operation.
 *  Idea is to make memory management for transient data structures
 *  a bit easier. Does use more memory than direct malloc/free.
 *
 * Methods:
 *   pool_create (initialise pool data structure)
 *   pool_alloc  (allocate memory using nominated pool)
 *   pool_free   (free all memory allocated in this pool).
 */

/* Malloc pool in 4Kbyte chunks by default */
#define PREFERRED_POOL_BLOCK_SIZE  (4096)

struct pool_elt {
    struct pool_elt *next;      /* Linked list of data blocks */
    char data[1];
};

struct pool {
    struct pool_elt *first;     /* First data block */
    struct pool_elt *last;      /* Last data block */
    unsigned long blocksize;    /* Default block size for small allocations */
    unsigned long avail;        /* Available space in current (last) block */
    struct str *str_list;	/* prayer str -> templates */
};

struct pool *pool_create(unsigned long blocksize);
void pool_free(struct pool *p);
void *pool_alloc(struct pool *p, unsigned long size);
char *pool_strdup(struct pool *p, char *value);
char *pool_strcat(struct pool *p, char *s1, char *s2);
char *pool_strcat3(struct pool *p, char *s1, char *s2, char *s3);

unsigned long pool_vprintf_size(char *fmt, va_list ap);
void pool_vprintf(char *target, char *fmt, va_list ap);
char *pool_printf(struct pool *p, char *fmt, ...);

char *pool_join(struct pool *pool, char join_char, char *argv[]);

#ifndef CORE_POOL_ONLY
/* Misc */

typedef int BOOL;

#ifndef NIL
#define NIL (0)
#endif

#ifndef T
#define T (1)
#endif

#define CRLF "\015\012"

/* Fix ctype.h macros. */
#define UC (unsigned char)
#define Uisspace(c) isspace(UC(c))
#define Uisalpha(c) isalpha(UC(c))
#define Uisalnum(c) isalnum(UC(c))
#define Uisdigit(c) isdigit(UC(c))
#define Uisxdigit(c) isxdigit(UC(c))
#define Uiscntrl(c) iscntrl(UC(c))
#define Utoupper(c) toupper(UC(c))
#define Utolower(c) tolower(UC(c))

void *xmalloc(unsigned long size);
void *xrealloc(void *old, unsigned long size);

/* Str */

struct str {
    struct str *next;
    unsigned char *s;
    unsigned long len;
    unsigned long alloc;
};

#define PREFERRED_STR_BLOCK_SIZE (32)

struct str *str_create(struct pool *p, unsigned long blocksize);
void *str_reserve(struct str *str, unsigned long size);

void str_free(struct str *str);
void str_free_chain(struct str *str);

void str_putchar(struct str *str, unsigned char c);
void str_vaprintf(struct str *str, char *format, va_list ap);
void str_printf(struct str *str, char *format, ...);
void str_puts(struct str *str, char *string);
void str_encode_url(struct str *str, char *s);
void str_encode_canon(struct str *str, char *s);


#define str_putc(str, c)                        \
do {                                            \
  unsigned char _c = (unsigned char)c;          \
                                                \
  if (str->len < str->alloc) {                  \
      str->s[str->len++] = _c;                  \
  } else                                        \
    str_putchar(str, _c);                       \
} while (0)

/* Fetch methods */
unsigned long str_len(struct str *str);

void str_rewind(struct str *str, unsigned long offset);
void *str_fetch(struct str *s);

#endif // CORE_POOL_ONLY

#endif
