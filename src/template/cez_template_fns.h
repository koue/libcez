/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

char *
template_gettoken(char **sp, struct pool *pool);

char *
template_getvar(char **sp, char *quotep, struct pool *pool);

char *
template_getlist(char **sp, struct pool *pool);

char *
template_getexpr(char **sp, struct pool *pool);

/* Temporary public, eventually static */
char *
template_parse_read_file(char *filename, struct pool *pool);

/* Temporary public, eventually static */
char **
template_parse_split_lines(char *s, struct pool *pool);

struct template *template_parse(char *dir, char *set,
                                char *name, struct pool *pool);

BOOL
template_expand(char *name, struct template_vals *tvals, struct buffer *b);

BOOL
template_compile(char *prefix, struct template *template, FILE *file);

struct template *
template_find(char *set, char *name, struct pool *pool);

