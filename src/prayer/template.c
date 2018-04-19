/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Support routines for locating and parsing templates */

/* token is \S+ */

#include "cez_prayer.h"

#define MAX_TOKEN (63)

char *
template_gettoken(char **sp, struct pool *pool)
{
    static char buf[MAX_TOKEN+1];
    char *s = *sp;
    char *start;
    int len;

    while (*s && Uisspace(*s))
        s++;

    start = s;
    while (*s && !Uisspace(*s))
        s++;

    if ((len = s-start) <= MAX_TOKEN) {
        memcpy(buf, start, len);
        buf[len] = '\0';
    } else
        buf[0] = '\0';

    while (*s && Uisspace(*s))
        s++;
    *sp = s;

    return((pool) ? pool_strdup(pool, buf) : buf);
}


/* Example Variable names:
 *  $a
 *  $a->b
 *  $a->{b}
 *  ${a->{b}}
 *  $list[3]->foo
 *
 */

#define MAX_VAR_LEN (63)

char *
template_getvar(char **sp, char *quotep, struct pool *pool)
{
    static char buf[MAX_VAR_LEN+1];
    char *s = *sp;
    int depth = 0;
    int len = 0;

    /* Quoted variable: |n : none, |c : canon, |u : url */
    if (quotep)
        *quotep = 'n';

    while (*s && Uisspace(*s))
        s++;

    if (!(s && (*s == '$')))
        return(NIL);

    buf[len++] = *s++;
    do {
        if (s[0] == '[') {
            s++;
            buf[0] = '@';
            buf[len++] = '-';
            while (isdigit(*s) && (len < MAX_VAR_LEN))
                buf[len++] = *s++;
            if (*s++ != ']')
                return(NIL);
            continue;
        }
        if ((s[0] == '-') && (s[1] == '>')) {
            buf[len++] = '-';
            s += 2;
            continue;
        }
        if (*s == '{') {
            s++;
            depth++;
        } else if (s[0] == '|' && s[1] != '\0'  && s[2] == '}') {
            if (quotep)
                *quotep = s[1];
            s += 3;
            if (depth <= 0)
                return(NIL);
            if (--depth == 0)
                break;
        } else if (*s == '}') {
            s++;
            if (depth <= 0)
                return(NIL);
            if (--depth == 0)
                break;
        } else if (isalnum(*s)) {
            buf[len++] = *s++;
        } else if (*s == '_') {
            buf[len++] = *s++;
        } else
            break;
    } while (*s && (len < MAX_VAR_LEN));

    if ((len == 0) || (len >= MAX_VAR_LEN) || (depth < 0))
        return(NIL);

    *sp = s;

    buf[len] = '\0';
    s = (buf[0] == '$') ? (buf+1) : buf;

    return((pool) ? pool_strdup(pool, s) : s);
}

char *
template_getlist(char **sp, struct pool *pool)
{
    static char buf[MAX_VAR_LEN+1];
    char *s = *sp;
    int len = 0;

    while (*s && Uisspace(*s))
        s++;

    if (!(s && (*s == '@')))
        return(NIL);

    buf[len++] = *s++;
    while ((isalnum(*s) || (*s == '_')) && (len < MAX_VAR_LEN)) {
        buf[len++] = *s++;
    }

    *sp = s;

    buf[len] = '\0';
    return((pool) ? pool_strdup(pool, buf) : buf);
}

/* expr is token or "quoted text" */

char *
template_getexpr(char **sp, struct pool *pool)
{
    char *result;
    char *s = *sp;
    char *start;
    int len;

    while (*s && Uisspace(*s))
        s++;

    if (*s == '"') {
        start = ++s;
        while (*s && (*s != '"'))
            s++;
    } else {
        start = s;
        while (*s && !Uisspace(*s))
            s++;
    }

    len = s-start;
    result = pool_alloc(pool, len+1);
    memcpy(result, start, len);
    result[len] = '\0';

    if (*s == '"')
        s++;

    while (*s && Uisspace(*s))
        s++;
    *sp = s;

    return(result);
}

/* ====================================================================== */

extern struct template_map_index template_map_index[];

struct template *
template_find(char *set, char *name, struct pool *pool)
{
    struct template_map_index *tmi = &template_map_index[0];
    struct template_map *tm;
    unsigned long count;
    unsigned long first, last, middle;
    int rc;

    while (tmi->name && (strcmp(tmi->name, set) != 0))
        tmi++;

    if (!tmi->name)
        return(NIL);

    tm    = tmi->template_map;
    count = *(tmi->count);

    first = 0;
    last = count;

    /* Binary chop */
    while (first < last) {
        middle = (first + last) / 2;
        rc = strcmp(tm[middle].name, name);

        if (rc == 0)
            return(tm[middle].template);
        else if (rc < 0)
            first = middle + 1;
        else
            last = middle;
    }
    /* Not found */
    return (NIL);
}

