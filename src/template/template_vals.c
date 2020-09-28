/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */

#include "cez_template.h"

struct template_vals_urlstate *
template_vals_urlstate_create(struct pool *pool)
{
    struct template_vals_urlstate *result
        = pool_alloc(pool, sizeof(struct template_vals_urlstate));

    result->url_prefix_icons = "";
    result->url_prefix_bsession = "";
    result->sequence = 1;
    result->use_short = T;
    result->test_mode = NIL;

    return(result);
}

struct template_vals *
template_vals_create(struct pool *pool,
                     char *dir, char *set, BOOL use_compiled, BOOL html_error,
                     struct template_vals_urlstate *urlstate)
{
    struct template_vals *result = pool_alloc(pool,
                                              sizeof(struct template_vals));

    result->pool  = pool;
    result->vals  = assoc_create(pool, 256, T);
    result->str1  = str_create(pool, 0);
    result->dir   = (dir) ? pool_strdup(pool, dir) : NIL;
    result->set   = (set) ? pool_strdup(pool, set) : NIL;
    result->use_compiled = use_compiled;
    result->html_error = html_error;
    result->error = str_create(pool, 0);
    result->urlstate = urlstate;

    return(result);
}

void template_vals_string(struct template_vals *tvals,
                          char *name, char *value)
{
    if (*name == '$')
        name++;

    if (value == NIL)
        value = "";

    assoc_update(tvals->vals, name, value, T);
}

void template_vals_ulong(struct template_vals *tvals,
                         char *name, unsigned long value)
{
    if (*name == '$')
        name++;

    assoc_update(tvals->vals, name, string_itoa_tmp(value), T);
}

void
template_vals_hash_string(struct template_vals *tvals, char *hash,
                          char *name, char *value)
{
    struct str *str1 = tvals->str1;

    if (*hash == '$')
        hash++;

    if (value == NIL)
        value = "";

    str_rewind(str1, 0);
    str_printf(str1, "%s-%s", hash, name);
    assoc_update(tvals->vals, str_fetch(str1), value, T);
}

void
template_vals_hash_ulong(struct template_vals *tvals, char *hash,
                         char *name, unsigned long value)
{
    struct str *str1 = tvals->str1;

    if (*hash == '$')
        hash++;

    str_rewind(str1, 0);
    str_printf(str1, "%s-%s", hash, name);
    assoc_update(tvals->vals, str_fetch(str1), string_itoa_tmp(value), T);
}

void
template_vals_foreach_init(struct template_vals *tvals,
                           char *array, unsigned long offset)
{
    struct str *str1 = tvals->str1;

    if (*array == '@')
        array++;

    str_rewind(str1, 0);
    str_printf(str1, "@%s-%lu", array, offset);

    assoc_update(tvals->vals, str_fetch(str1), "1", T);
}

void
template_vals_foreach_string(struct template_vals *tvals,
                             char *array, unsigned long offset,
                             char *name, char *value)
{
    struct str *str1 = tvals->str1;

    if (*array == '@')
        array++;

    if (value == NIL)
        value = "";

    str_rewind(str1, 0);
    str_printf(str1, "@%s-%lu-%s", array, offset, name);
    assoc_update(tvals->vals, str_fetch(str1), value, T);
}

void
template_vals_foreach_ulong(struct template_vals *tvals,
                            char *array, unsigned long offset,
                            char *name, unsigned long value)
{
    struct str *str1 = tvals->str1;

    if (*array == '@')
        array++;

    str_rewind(str1, 0);
    str_printf(str1, "@%s-%lu-%s", array, offset, name);
    assoc_update(tvals->vals, str_fetch(str1), string_itoa_tmp(value), T);
}
