/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

struct template_vals_urlstate {
    char *url_prefix_icons;
    char *url_prefix_bsession;
    unsigned long sequence;
    BOOL use_short;
    BOOL test_mode;
};

struct template_vals {
    struct assoc *vals;
    struct pool *pool;
    struct str *str1;
    char *dir;
    char *set;
    BOOL use_compiled;
    BOOL html_error;
    struct str *error;
    struct template_vals_urlstate *urlstate;
};

struct template_vals_urlstate *
template_vals_urlstate_create(struct pool *pool);

struct template_vals *
template_vals_create(struct pool *pool,
                     char *dir, char *set, BOOL use_compiled, BOOL html_error,
                     struct template_vals_urlstate *urlstate);

void template_vals_string(struct template_vals *tvals,
                          char *name, char *value);

void template_vals_ulong(struct template_vals *tbals,
                         char *name, unsigned long value);

void 
template_vals_hash_string(struct template_vals *tvals, char *hash,
                          char *name, char *value);

void 
template_vals_hash_ulong(struct template_vals *tvals, char *hash,
                         char *name, unsigned long value);

void 
template_vals_foreach_init(struct template_vals *tvals,
                           char *array, unsigned long offset);

void 
template_vals_foreach_string(struct template_vals *tvals,
                             char *array, unsigned long offset,
                             char *name, char *value);


void 
template_vals_foreach_ulong(struct template_vals *tvals,
                            char *array, unsigned long offset,
                            char *name, unsigned long value);
