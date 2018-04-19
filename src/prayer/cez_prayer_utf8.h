/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */

char *utf8_from_imaputf7(struct pool *pool, char *t);
char *utf8_to_imaputf7(struct pool *pool, char *t);
char *utf8_from_string(struct pool *pool, char *charset, char *t, unsigned long len);
BOOL utf8_print(char *charset, char *fallback_charset,
                unsigned char **dst, unsigned long dst_size,
                unsigned char **src, unsigned long src_size);
char *utf8_prune(struct pool *pool, char *s, unsigned long maxlen);

unsigned long utf8_count_chars(char *s, unsigned long bytes);

BOOL utf8_is_8859_1(char *s0);
void utf8_to_8859_1(char *s0);

void utf8_skip_char(char **sp);
void utf8_find_offset(char **sp, unsigned long offset);
unsigned long utf8_len(char *s0);
void utf8_print_char(struct buffer *b, char **sp);
