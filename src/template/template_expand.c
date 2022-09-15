/* Copyright (c) 2018-2022 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_template.h"

/* ====================================================================== */

/* Note really awful feature of GET URLs: Web server needs to quote "&" as
 * "&amp;" to stop browser from interpreting & sequences. Browser will send
 * the URL back as "&" */

static void
template_expand_session(struct template_vals_urlstate *urlstate,
                        struct buffer *b, char *t, BOOL seq)
{
    unsigned char *s = (unsigned char *) t;
    unsigned char c;
    BOOL use_short = (urlstate->use_short && seq) ? T : NIL;
    unsigned char src[3];
    unsigned char dst[5];
    char *v =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-";

    src[0] = (urlstate->sequence >> 16) & 255;     /* Bits 23->16 */
    src[1] = (urlstate->sequence >> 8) & 255;      /* Bits 15->8  */
    src[2] = (urlstate->sequence) & 255;           /* Bits  7->0  */

    /* Quick (URL friendly) base64 encode of three byte sequence */
    dst[0] = v[src[0] >> 2];
    dst[1] = v[((src[0] << 4) + (src[1] >> 4)) & 0x3f];
    dst[2] = v[((src[1] << 2) + (src[2] >> 6)) & 0x3f];
    dst[3] = v[src[2] & 0x3f];
    dst[4] = '\0';

    if (!s) {
        bputs(b, "(nil)");
        return;
    }

    if (urlstate->test_mode) {
        while ((c = *s++) && (Uisalnum(c) || (c == '_')))
            bputc(b, c);
        bputs(b, ".html");
        return;
    }

    if (!urlstate->use_short) {
        bputs(b, urlstate->url_prefix_bsession);
        bputc(b, '/');
    }

    if (seq) {
        bputs(b, dst);
    } else
        bputs(b, "NOSEQ");

    bputc(b, (use_short) ? '@' : '/');
    while ((c = *s++)) {
        switch (c) {
        case '/':
            bputc(b, (use_short) ? '@' : '/');
            break;
        case '&':
            bputs(b, "&amp;");
            break;
        default:
            bputc(b, c);
            break;
        }
    }
}

static void
template_expand_icon(struct template_vals_urlstate *urlstate,
                     struct buffer *b, char *t)
{
    unsigned char *s = (unsigned char *) t;

    bprintf(b, "%s/%s.gif", urlstate->url_prefix_icons, s);
}

/* ====================================================================== */

/* Forward decleration */

/* foreach_active_item is list of simple substititions to make:
 *
 * $i->{foo} becomes lookup on @list-0-foo, @list-1-foo , etc
 */

struct foreach_active_item {
    struct foreach_active_item *next;
    char *var_prefix;
    char *array;
};

struct template_expand_state {
    char          *name;
    struct template_vals *tvals;
    struct pool   *pool;
    struct str    *str;
    struct str    *str2;
    struct str    *str3;
    struct str    *expbuf;
    struct str    *error;
    struct buffer *buffer;
    struct assoc  *params;
    struct assoc  *vars;
    struct foreach_active_item *foreach_list;
};

static BOOL
template_expand_work(struct template_item *item,
                     struct template_expand_state *state);

static char *
template_fetch_var(char *s, struct template_expand_state *state)
{
    struct assoc *params = state->params;
    struct assoc *h = state->vars;
    struct foreach_active_item *f = state->foreach_list;
    char *result;

    while (f) {
        int len = strlen(f->var_prefix);

        if (!strncmp(s, f->var_prefix, len)) {
            char buf[64];

            snprintf(buf, sizeof(buf), "%s%s", f->array, s+len);
            if (params && (result = assoc_lookup(params, buf)))
                return(result);
            else
                return (assoc_lookup(h, buf));
        }
        f = f->next;
    }
    if (params && (result = assoc_lookup(params, s)))
        return(result);
    else
        return (assoc_lookup(h, s));
}

static BOOL
template_test_var(char *s, struct template_expand_state *state)
{
    char *val = template_fetch_var(s, state);

    return ((val) ? T : NIL);
}

static void
template_expand_var(char *s, struct str *str,
                    struct template_expand_state *state, char quote)
{
//    struct pool *pool = state->pool;
    char *val = template_fetch_var(s, state);

    if (!val)
        return;

    if (quote == 'u') {
        str_encode_url(str, val);
    } else if (quote == 'c') {
        str_encode_canon(str, val);
//    } else if (quote == '7') {
//        str_puts(str, utf8_from_imaputf7(pool, val));
    } else {
        str_puts(str, val);
    }
}

static char *
template_expand_vars(struct str *str, char *s,
                     struct template_expand_state *state)
{
    char *var;
    char varquote = 'n';

    /* Reuse single str variable */
    str_rewind(str, 0);

    while (*s) {
        if (*s != '$') {
            str_putc(str, *s++);
        } else if ((var = template_getvar(&s, &varquote, NIL))) {
            template_expand_var(var, str, state, varquote);
        } else
            return(NIL);
    }
    return(str_fetch(str));
}

static BOOL
template_test_value(char *s, char *t, struct template_expand_state *state)
{
    char *val  = template_expand_vars(state->str2, s, state);
    char *val2 = template_expand_vars(state->str3, t, state);

    return((val && val2 && !strcmp(val, val2)) ? T : NIL);
}

static void
template_expand_vars_buffer(char *s, struct template_expand_state *state)
{
    struct buffer *b = state->buffer;
    char *value;
    char *var, *val1, *val2;
    char quoting = 'h';

    value = template_getexpr(&s, state->pool);
    value = template_expand_vars(state->str, value, state);

    if (!strncasecmp(s, "IFDEF", 5) && Uisspace(s[5])) {
        s += 6;
        if (!((var = template_getvar(&s, NIL, NIL)) &&
              template_test_var(var, state)))
            return;
    } else if (!strncasecmp(s, "IFEQ", 4) && Uisspace(s[4])) {
        s += 5;
        if (!((val1 = template_getexpr(&s, state->pool)) &&
              (val2 = template_getexpr(&s, state->pool)) &&
              (template_test_value(val1, val2, state))))
            return;
    }
    while (Uisspace(*s))
        s++;

    if (*s == '|') {
        s++;
        quoting = *s++;
    }

    switch (quoting) {
    case 'n':
        bputs(b, value);
        break;
    case 's':
        template_expand_session(state->tvals->urlstate, b, value, T);
        break;
    case 'S':
        template_expand_session(state->tvals->urlstate, b, value, NIL);
        break;
    case 'i':
        template_expand_icon(state->tvals->urlstate, b, value);
        break;
    case 'c':
        buffer_encode_canon(b, value);
        break;
    case 'u':
        buffer_encode_url(b, value);
        break;
    case 'h':
    default:
        html_quote_string(b, value);
        break;
    }
}

/* ====================================================================== */

static BOOL
template_expand_line(char *s, struct template_expand_state *state)
{
    struct buffer *b = state->buffer;
    char *t;
    char c;

    while (s && (t = strstr(s, "<%"))) {
        /* Expand a <% ... %> escape */
        while (s < t)
            bputc(b, *s++);
        s += 2;
        if ((t = strstr(s, "%>"))) {
            int   len = t-s;
            char *buf = str_reserve(state->expbuf, len+1);

            memcpy(buf, s, len);
            buf[len] = '\0';
            template_expand_vars_buffer(buf, state);
        } else
            return(NIL);
        s = (t) ? t+2 : NIL;
    }
    if (s) {
        /* Print the rest of the line as is. \<sp>+\n blocks CRLF */
        while ((c=*s++)) {
            if (c == '\\') {
                t = s ;
                while (Uisspace(*t))
                    t++;
                if (*t == '\0')
                    return(T);
            }
            bputc(b, c);
        }
    }
    bputs(b, ""CRLF);
    return(T);
}

static BOOL
template_expand_lines(struct template_lines *item,
                      struct template_expand_state *state)
{
    char **lines = item->first;
    int    count = item->count;
    int rc = T;

    while (rc && count) {
        rc = template_expand_line(*lines, state);
        lines++;
        count--;
    }
    return(rc);
}

static BOOL
template_expand_ifdef(struct template_ifdef *item,
                      struct template_expand_state *state)
{
    int rc;
    struct template_item *true_block
        = (item->positive) ? item->true_block : item->false_block;
    struct template_item *false_block
        = (item->positive) ? item->false_block : item->true_block;

    if (template_test_var(item->expr, state))
        rc = template_expand_work(true_block, state);
    else
        rc = template_expand_work(false_block, state);

    return(rc);
}

static BOOL
template_expand_ifeq(struct template_ifeq *item,
                      struct template_expand_state *state)
{
    int rc;
    struct template_item *true_block
        = (item->positive) ? item->true_block : item->false_block;
    struct template_item *false_block
        = (item->positive) ? item->false_block : item->true_block;


    if (template_test_value(item->name, item->value, state))
        rc = template_expand_work(true_block, state);
    else
        rc = template_expand_work(false_block, state);

    return(rc);
}

static BOOL
template_expand_foreach(struct template_foreach *item,
                        struct template_expand_state *state)
{
    struct pool  *pool = state->pool;
    struct assoc *h    = state->vars;
    struct foreach_active_item *oldf = state->foreach_list;
    struct foreach_active_item *f;
    char buf[64];                  /* Variables limited to 64 chars */
    int i = 0;
    BOOL ret = T;

    f = pool_alloc(pool, sizeof (struct foreach_active_item));
    f->next       = oldf;
    f->var_prefix = pool_strcat(pool, item->name, "-");

    state->foreach_list = f;

    snprintf(buf, sizeof(buf), "%s-%d", item->array, i);
    while (assoc_lookup(h, buf)) {
        strncat(buf, "-", sizeof(buf) - strlen(buf) - 1);
        f->array = buf;

        if (!(ret = template_expand_work(item->block, state)))
            break;
        i++;
        snprintf(buf, sizeof(buf), "%s-%d", item->array, i);
    }
    state->foreach_list = oldf;

    return(ret);
}

static BOOL
template_expand_loop(struct template_loop *item,
                     struct template_expand_state *state)
{
    char *s = template_fetch_var(item->var, state);
    int count = (s) ? atoi(s) : 0;
    int rc = T;

    while (rc && (count > 0)) {
        rc = template_expand_work(item->block, state);
        count--;
    }

    return(rc);
}

/* ====================================================================== */

static BOOL
template_expand_recurse(struct template_item *template,
                        struct template_vals *tvals,
                        struct buffer *b,
                        struct assoc *params,
                        struct template_expand_state *oldstate)
{
    struct template_expand_state state;
    struct pool *pool = tvals->pool;

    state.tvals        = tvals;
    state.pool         = pool;
    state.vars         = tvals->vals;
    state.params       = params;
    state.buffer       = b;
    state.foreach_list = NIL;

    /* Recursive calls can reuse scratch, which saves lots of str_create()s */
    state.str          = oldstate->str;
    state.str2         = oldstate->str2;
    state.str3         = oldstate->str3;
    state.expbuf       = oldstate->expbuf;

    return(template_expand_work(template, &state));
}

static BOOL
template_expand_call_add_param(struct assoc *params,
                               struct str *str,
                               char *s,
                               struct template_expand_state *state)
{
    char *t;
    char *var, *value = NIL;

    while (Uisspace(*s))
        s++;

    if ((t = strstr(s, "=>"))) {
        *t = '\0';
        t += 2;
        value = template_getexpr(&t, state->pool);
        value = template_expand_vars(str, value, state);
    } else
        value = "1";  /* Default unspecified value */

    if (!(var = template_getvar(&s, NIL, NIL)))
        return(NIL);

    assoc_update(params, var, value, T);
    return(T);
}

static BOOL
template_expand_call(struct template_call *item,
                     struct template_expand_state *state)
{
    struct template_vals *tvals = state->tvals;
    struct pool   *pool   = state->pool;
    struct buffer *b      = state->buffer;
    struct assoc  *params = NIL;
    struct str    *str    = NIL;
    struct template *template;
    char *s, *t;
    char *err = NIL;

#ifdef PRAYER_TEMPLATE_COMPILE
    if (tvals->use_compiled)
        template = template_find(tvals->set, item->name, tvals->pool);
    else
#endif
        template = template_parse(tvals->dir, tvals->set,
                                  item->name, tvals->pool);
    if (!template) {
        str_printf(state->error,
                   "Template \"%s\" not found (CALL from template \"%s\")\n",
                   item->name, state->name);
        return(NIL);
    }
    /* NB: template->error only used by interpreted templates */
    if (template->error &&
        (err = str_fetch(template->error)) && err[0]) {
        if (state->error)
            str_puts(state->error, err);
        else
            fputs(err, stderr);
        return(NIL);
    }
    if (item->params && item->params[0]) {
        s      = pool_strdup(pool, item->params);   /* Scratch copy */
        str    = str_create(pool, 0);
        params = assoc_create(pool, 16, T);

        while ((t = strchr(s, ','))) {
            *t++ = '\0';
            template_expand_call_add_param(params, str, s, state);
            s = t;
        }
        template_expand_call_add_param(params, str, s, state);
    }
    return(template_expand_recurse(template->tree, tvals, b, params, state));
}

/* ====================================================================== */

static BOOL
template_expand_work(struct template_item *item,
                     struct template_expand_state *state)
{
    BOOL rc = T;

    while (rc && item) {
        switch (item->type) {
        case TEMPLATE_ITEM_LINES:
            rc = template_expand_lines((struct template_lines *)item, state);
            break;
        case TEMPLATE_ITEM_IFDEF:
            rc = template_expand_ifdef((struct template_ifdef *)item, state);
            break;
        case TEMPLATE_ITEM_IFEQ:
            rc = template_expand_ifeq((struct template_ifeq *)item, state);
            break;
        case TEMPLATE_ITEM_FOREACH:
            rc = template_expand_foreach((struct template_foreach *)item,
                                         state);
            break;
        case TEMPLATE_ITEM_LOOP:
            rc = template_expand_loop((struct template_loop *)item,
                                      state);
            break;
        case TEMPLATE_ITEM_CALL:
            rc = template_expand_call((struct template_call *)item, state);
            break;
        default:
            /* log_fatal("template_expand_work() with unknown type"); */
	    fprintf(stderr, "template_expand_work() with unknown type\n");
	    exit (1);
        }

        item = item->tree_next;
    }
    return(rc);
}

BOOL
template_expand(char *name, struct template_vals *tvals, struct buffer *b)
{
    struct template_expand_state state;
    struct template *template;
    struct pool *pool = tvals->pool;
    struct str *error = tvals->error;
    char *err;

#ifdef PRAYER_TEMPLATE_COMPILE
    if (tvals->use_compiled)
        template = template_find(tvals->set, name, tvals->pool);
    else
#endif
        template = template_parse(tvals->dir, tvals->set, name, tvals->pool);

    if (!template) {
        str_printf(tvals->error,
                   "Template %s not found (top level template_expand())\n",
                   name);
        err = str_fetch(tvals->error);
        buffer_reset(b);
        if (tvals->html_error) {
            bputs(b, "<html><pre>"CRLF);
            html_quote_string(b, err);
            bputs(b, "</pre></html>"CRLF);
        } else
            bputs(b, err);
        return(NIL);
    }
    if (template->error && (err=str_fetch(template->error)) && err[0]) {
        buffer_reset(b);
        if (tvals->html_error) {
            bputs(b, "<html><pre>"CRLF);
            html_quote_string(b, err);
            bputs(b, "</pre></html>"CRLF);
        } else
            bputs(b, err);
        return(NIL);
    }

    state.name         = name;
    state.tvals        = tvals;
    state.pool         = pool;
    state.vars         = tvals->vals;
    state.params       = NIL;
    state.buffer       = b;
    state.foreach_list = NIL;
    state.str          = str_create(pool, 0);
    state.str2         = str_create(pool, 0);
    state.str3         = str_create(pool, 0);
    state.expbuf       = str_create(pool, 64);
    state.error        = tvals->error;

    if (!template_expand_work(template->tree, &state)) {
        buffer_reset(b);
        if (tvals->html_error) {
            bputs(b, "<html><pre>"CRLF);
            html_quote_string(b, str_fetch(error));
            bputs(b, "</pre></html>"CRLF);
        } else
            bputs(b, str_fetch(error));

        return(NIL);
    }
    return(T);
}
