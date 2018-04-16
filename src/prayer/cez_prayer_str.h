/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

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

