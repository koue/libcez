/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Top down recursive parse */

typedef enum {
    TEMPLATE_STATE_TOPLEVEL,
    TEMPLATE_STATE_IFDEF,
    TEMPLATE_STATE_IFEQ,
    TEMPLATE_STATE_ELSE,
    TEMPLATE_STATE_ENDIF,
    TEMPLATE_STATE_FOREACH,
    TEMPLATE_STATE_ENDFOREACH,
    TEMPLATE_STATE_LOOP,
    TEMPLATE_STATE_ENDLOOP
} TEMPLATE_STATE;

struct template_block {
    TEMPLATE_STATE state;
    char *name;
    struct pool *pool;
    char **lines;
    int start_line;
    int next_line;
    struct template_item *head;
    struct template *global;        /* Top level global state */
};

/* ====================================================================== */

static void
template_global_add(struct template *global,
                    struct template_item *item)
{
    item->list_next   = NIL;
    item->number      = ++global->count;

    if (global->tail) {
        global->tail->list_next = item;
        global->tail = item;
    } else {
        global->head = item;
        global->tail = item;
    }
}


/* ====================================================================== */

static void template_block_error(struct template_block *block,
                                 char *err, BOOL report_start)
{
    struct str *error = block->global->error;

    if (report_start && (block->start_line != block->next_line)) {
        str_printf(error,
                   "Template: %s, Line %d: %s starting at %d\n", block->name,
                   block->next_line+1, err, block->start_line+1);
    } else {
        char *line = block->lines[block->next_line];

        str_printf(error,
                   "Template: %s, Line %d: %s\n   %s\n", block->name,
                   block->next_line+1, err, (line) ? line : NIL);
    }
}

static void template_block_mismatch(struct template_block *block, char *end)
{
    struct str *error = block->global->error;
    char *type = "block";

    if ((block->state == TEMPLATE_STATE_IFDEF) ||
        (block->state == TEMPLATE_STATE_ELSE)) {
        struct template_ifdef *template_ifdef
            = (struct template_ifdef *)block;

        type = (template_ifdef->positive) ? "IFDEF" : "IFNDEF";
    } else if (block->state == TEMPLATE_STATE_IFEQ) {
        struct template_ifeq *template_ifeq
            = (struct template_ifeq *)block;

        type = (template_ifeq->positive) ? "IFEQ" : "IFNEQ";
    } else if (block->state == TEMPLATE_STATE_FOREACH) {
        type = "FOREACH";
    } else if (block->state == TEMPLATE_STATE_LOOP) {
        type = "LOOP";
    }

    str_printf(error,
               ("Template %s, Line %d: %s doesn't match %s "
                "starting at line %d\n"),
               block->name, block->next_line+1, end, type, block->start_line+1);
}

/* ====================================================================== */

static struct template_item *
template_parse_work(struct template_block *block);

static void
template_block_init(struct template_block *block,
                    struct template_block *parent,
                    TEMPLATE_STATE state)
{
    block->state      = state;
    block->name       = parent->name;
    block->pool       = parent->pool;
    block->lines      = parent->lines;
    block->start_line = parent->next_line;
    block->next_line  = parent->next_line;
    block->head       = NIL;
    block->global     = parent->global;
}

static struct template_item *
template_parse_ifdef(struct template_block *parent, char *expr, BOOL positive)
{
    struct template_block block;
    struct template_ifdef *template_ifdef
        = pool_alloc(parent->pool, sizeof(struct template_ifdef));
    char *var;

    template_block_init(&block, parent, TEMPLATE_STATE_IFDEF);
    template_global_add(block.global,
                        (struct template_item *)template_ifdef);

    if (!(var = template_getvar(&expr, NIL, parent->pool))) {
        template_block_error(&block, "Invalid variable name", NIL);
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(&block, "Junk at end of expression", NIL);
        return(NIL);
    }

    template_ifdef->type = TEMPLATE_ITEM_IFDEF;
    template_ifdef->expr = var;
    block.next_line++;
    template_ifdef->positive    = positive;
    template_ifdef->true_block  = template_parse_work(&block);
    template_ifdef->false_block = NIL;

    if (block.state == TEMPLATE_STATE_ELSE) {
        block.state = TEMPLATE_STATE_IFDEF;
        template_ifdef->false_block = template_parse_work(&block);
    }

    parent->next_line = block.next_line;
    if (block.state != TEMPLATE_STATE_ENDIF) {
        if (positive)
            template_block_error(&block, "Unterminated IFDEF", T);
        else
            template_block_error(&block, "Unterminated IFNDEF", T);
        return(NIL);
    }

    return((struct template_item *)template_ifdef);
}

static struct template_item *
template_parse_ifeq(struct template_block *parent, char *expr, BOOL positive)
{
    struct template_block block;
    struct template_ifeq *template_ifeq
        = pool_alloc(parent->pool, sizeof(struct template_ifeq));
    char *name;
    char *value;

    template_block_init(&block, parent, TEMPLATE_STATE_IFEQ);
    template_global_add(block.global,
                        (struct template_item *)template_ifeq);

    if (!(name = template_getexpr(&expr, parent->pool))) {
        template_block_error(&block, "Invalid first value", NIL);
        return(NIL);
    }
    if (!(value = template_getexpr(&expr, parent->pool))) {
        template_block_error(&block, "Invalid second value", NIL);
        return(NIL);
    }

    if (expr && expr[0]) {
        template_block_error(&block, "Junk at end of expression", NIL);
        return(NIL);
    }

    template_ifeq->type  = TEMPLATE_ITEM_IFEQ;
    template_ifeq->name  = name;
    template_ifeq->value = value;
    block.next_line++;
    template_ifeq->positive    = positive;
    template_ifeq->true_block  = template_parse_work(&block);
    template_ifeq->false_block = NIL;

    if (block.state == TEMPLATE_STATE_ELSE) {
        block.state = TEMPLATE_STATE_IFEQ;
        template_ifeq->false_block = template_parse_work(&block);
    }

    parent->next_line = block.next_line;
    if (block.state != TEMPLATE_STATE_ENDIF) {
        template_block_error(&block, "Unterminated IFEQ", T);
        return(NIL);
    }

    return((struct template_item *)template_ifeq);
}

static struct template_item *
template_parse_else(struct template_block *block, char *expr)
{
    if ((block->state != TEMPLATE_STATE_IFDEF) &&
        (block->state != TEMPLATE_STATE_IFEQ)) {
        template_block_mismatch(block, "ELSE");
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(block, "Junk at end of expression", NIL);
        return(NIL);
    }
    block->state = TEMPLATE_STATE_ELSE;
    block->next_line++;
    return(block->head);
}

static struct template_item *
template_parse_endif(struct template_block *block, char *expr)
{
    if ((block->state != TEMPLATE_STATE_IFDEF) &&
        (block->state != TEMPLATE_STATE_IFEQ) &&
        (block->state != TEMPLATE_STATE_ELSE)) {
        template_block_mismatch(block, "ENDIF");
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(block, "Junk at end of expression", NIL);
        return(NIL);
    }
    block->state = TEMPLATE_STATE_ENDIF;
    block->next_line++;
    return(block->head);
}

static struct template_item *
template_parse_foreach(struct template_block *parent, char *expr)
{
    struct template_block block;
    struct template_foreach *template_foreach
        = pool_alloc(parent->pool, sizeof(struct template_foreach));
    char *var;
    char *array;

    template_block_init(&block, parent, TEMPLATE_STATE_FOREACH);
    template_global_add(block.global,
                        (struct template_item *)template_foreach);

    if (!(var = template_getvar(&expr, NIL, parent->pool))) {
        template_block_error(&block, "Invalid variable name", NIL);
        return(NIL);
    }
    if (!(array = template_getlist(&expr, parent->pool))) {
        template_block_error(&block, "Invalid list name", NIL);
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(&block, "Junk at end of expression", NIL);
        return(NIL);
    }

    template_foreach->type  = TEMPLATE_ITEM_FOREACH;
    template_foreach->name  = var;
    template_foreach->array = array;
    block.next_line++;
    template_foreach->block = template_parse_work(&block);

    parent->next_line = block.next_line;
    if (block.state != TEMPLATE_STATE_ENDFOREACH) {
        template_block_error(&block, "Unterminated FOREACH", T);
        return(NIL);
    }
    return((struct template_item *)template_foreach);
}

static struct template_item *
template_parse_endforeach(struct template_block *block, char *expr)
{
    if (block->state != TEMPLATE_STATE_FOREACH) {
        template_block_mismatch(block, "ENDFOREACH");
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(block, "Junk at end of expression", NIL);
        return(NIL);
    }
    block->state = TEMPLATE_STATE_ENDFOREACH;
    block->next_line++;
    return(block->head);
}

static struct template_item *
template_parse_loop(struct template_block *parent, char *expr)
{
    struct template_block block;
    struct template_loop *template_loop
        = pool_alloc(parent->pool, sizeof(struct template_loop));
    char *var;

    template_block_init(&block, parent, TEMPLATE_STATE_LOOP);
    template_global_add(block.global,
                        (struct template_item *)template_loop);

    if (!(var = template_getvar(&expr, NIL, parent->pool))) {
        template_block_error(&block, "Invalid variable name", NIL);
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(&block, "Junk at end of expression", NIL);
        return(NIL);
    }

    template_loop->type = TEMPLATE_ITEM_LOOP;
    template_loop->var  = var;
    block.next_line++;
    template_loop->block = template_parse_work(&block);

    parent->next_line = block.next_line;
    if (block.state != TEMPLATE_STATE_ENDLOOP) {
        template_block_error(&block, "Unterminated LOOP", T);
        return(NIL);
    }
    return((struct template_item *)template_loop);
}

static struct template_item *
template_parse_endloop(struct template_block *block, char *expr)
{
    if (block->state != TEMPLATE_STATE_LOOP) {
        template_block_mismatch(block, "ENDLOOP");
        return(NIL);
    }
    if (expr && expr[0]) {
        template_block_error(block, "Junk at end of expression", NIL);
        return(NIL);
    }
    block->state = TEMPLATE_STATE_ENDLOOP;
    block->next_line++;
    return(block->head);
}

static BOOL
call_has_continuation(char *s)
{
    char c;
    BOOL result = T;

    while ((c=*s++)) {
        if (c == ',') {
            result = T;
        } else if (!Uisspace(c)) {
            result = NIL;
        }
    }
    return(result);
}

static struct template_item *
template_parse_call(struct template_block *block, char *expr)
{
    struct template_call *template_call
        = pool_alloc(block->pool, sizeof(struct template_call));
    char *name, *s;
    int len;
    BOOL params = NIL;

    template_global_add(block->global,
                        (struct template_item *)template_call);

    if (!(name = template_getexpr(&expr, block->pool))) {
        template_block_error(block, "Invalid name", NIL);
        return(NIL);
    }
    /* Prune trailing , */
    if (((len=strlen(name)) > 1) && name[len-1] == ',') {
        name[len-1] = '\0';
        params = T;
    }

    while (*expr && (Uisspace(*expr) || (*expr == ','))) {
        if (*expr == ',')
            params = T;
        expr++;
    }
    s = expr = pool_strdup(block->pool, expr);

    if (params) {
        while (call_has_continuation(s)) {
            s = block->lines[block->next_line+1];
            if (!s) {
                template_block_error(block,
                                     "Unexpected end of file in CALL block",
                                     NIL);
                return(NIL);
            }
            if (*s != '%') {
                template_block_error(block, "Unterminated CALL block", NIL);
                return(NIL);
            }
            s++;
            block->next_line++;
            while (*s && (Uisspace(*s) || (*expr == ',')))
                s++;
            expr = pool_strcat(block->pool, expr, s);
        }
    }

    template_call->type   = TEMPLATE_ITEM_CALL;
    template_call->name   = name;
    template_call->params = expr;
    block->next_line++;

    return((struct template_item *)template_call);
}

static BOOL
template_parse_check_escapes(struct template_block *block, char *s)
{
    char *t, *u;

    while ((s=strstr(s, "<%"))) {
        if (!(t=strstr(s+2, "%>"))) {
            template_block_error(block, "Unterminated <% escape sequence", NIL);
            return(NIL);
        }
        if (!(u=strchr(s, '|')) || (u > t)) {
            template_block_error(block, "Missing | in <% escape sequence", NIL);
            return(NIL);
        }

        s = t + 2;
    }
    return(T);
}

static struct template_item *
template_parse_lines(struct template_block *block)
{
    struct template_lines *template_lines
        = pool_alloc(block->pool, sizeof(struct template_lines));
    int count = 0;
    char **lines = block->lines;

    template_global_add(block->global,
                        (struct template_item *)template_lines);

    template_lines->type  = TEMPLATE_ITEM_LINES;
    template_lines->first = &block->lines[block->next_line];

    while (lines[block->next_line] && (lines[block->next_line][0] != '%')) {
        if (!template_parse_check_escapes(block, lines[block->next_line])) {
            block->next_line++;
            return(NIL);
        }
        count++;
        block->next_line++;
    }
    template_lines->count = count;

    return((struct template_item *)template_lines);
}

struct template_item *
template_parse_control(struct template_block *block)
{
    char *s = block->lines[block->next_line];
    char *t;

    assert(*s == '%');
    s++;
    t = template_gettoken(&s, NIL);

    if (!strcasecmp(t, "ifdef"))
        return(template_parse_ifdef(block, s, T));

    if (!strcasecmp(t, "ifndef"))
        return(template_parse_ifdef(block, s, NIL));

    if (!strcasecmp(t, "ifeq"))
        return(template_parse_ifeq(block, s, T));

    if (!strcasecmp(t, "ifneq"))
        return(template_parse_ifeq(block, s, NIL));

    if (!strcasecmp(t, "else"))
        return(template_parse_else(block, s));

    if (!strcasecmp(t, "endif"))
        return(template_parse_endif(block, s));

    if (!strcasecmp(t, "foreach"))
        return(template_parse_foreach(block, s));

    if (!strcasecmp(t, "endforeach"))
        return(template_parse_endforeach(block, s));

    if (!strcasecmp(t, "loop"))
        return(template_parse_loop(block, s));

    if (!strcasecmp(t, "endloop"))
        return(template_parse_endloop(block, s));

    if (!strcasecmp(t, "call"))
        return(template_parse_call(block, s));

    template_block_error(block, "Unknown keyword", NIL);
    return(NIL);
}

static BOOL template_parse_work_isfinished(struct template_block *block)
{
    if ((block->state == TEMPLATE_STATE_ELSE) ||
        (block->state == TEMPLATE_STATE_ENDIF) ||
        (block->state == TEMPLATE_STATE_ENDFOREACH) ||
        (block->state == TEMPLATE_STATE_ENDLOOP))
        return(T);

    return(NIL);
}

static struct template_item *
template_parse_work(struct template_block *block)
{
    struct template_item *tail, *item;
    char *s;

    block->head = tail = NIL;

    while ((s = block->lines[block->next_line])) {
        if (*s++ == '%') {
            while (Uisspace(*s))
                s++;
            if (*s == '\0' || *s == '#') {
                block->next_line++;
                continue;
            }
            item = template_parse_control(block);

            if (template_parse_work_isfinished(block))
                return(item);  /* block->head */
        } else
            item = template_parse_lines(block);

        if (!item)
            return(NIL);   /* Propagate syntax error up the tree */

        item->tree_next = NIL;
        if (tail) {
            tail = tail->tree_next = item;
        } else {
            block->head = tail = item;
        }
    }
    if (block->state != TEMPLATE_STATE_TOPLEVEL)
        return(NIL);

    return(block->head);
}

char *
template_parse_read_file(char *filename, struct pool *pool)
{
    char *data;
    size_t size;
    FILE *file;
    struct stat sbuf;

    if ((file=fopen(filename, "r")) == NULL)
        return(NIL);

    if ((fstat(fileno(file), &sbuf)) < 0)
        return(NIL);

    size = sbuf.st_size;
    data=pool_alloc(pool, size+1);

    if (fread(data, 1, size, file) != size)
        return(NIL);

    fclose(file);
    data[size] = '\0';

    return(data);
}

static int
template_parse_count_lines(char *s)
{
    int result = 0;

    while (s && *s) {
        result++;
        if ((s = strchr(s, '\n')))
            s++;

    }
    return(result);
}

char **
template_parse_split_lines(char *s, struct pool *pool)
{
    int lines = template_parse_count_lines(s);
    char **result = pool_alloc(pool, (lines+1) * sizeof(char *));
    int i = 0;

    while (s && *s) {
        result[i++] = s;

        if ((s = strchr(s, '\n')))
            *s++ = '\0';
    }
    result[i] = NIL;

    return(result);
}

struct template *
template_parse(char *dir, char *set, char *name, struct pool *pool)
{
    struct template *template = pool_alloc(pool, sizeof(struct template));
    struct template_block  block;
    char *data;
    char **lines;
    char *s = strrchr(name, '/');

    template->name  = pool_strdup(pool, (s) ? (s+1) : name);
    template->error = str_create(pool, 0);
    template->tree  = NIL;

    if (dir && set)
        name = pool_printf(pool, "%s/%s/%s.t", dir, set, name);
    else
        name = pool_strcat(pool, name, ".t");

    if (!(data = template_parse_read_file(name, pool)))
        return(NIL);

    lines = template_parse_split_lines(data, pool);

    template->head       = NIL;
    template->tail       = NIL;
    template->count      = 0;

    block.state      = TEMPLATE_STATE_TOPLEVEL;
    block.name       = name;
    block.pool       = pool;
    block.lines      = lines;
    block.start_line = 0;
    block.next_line  = 0;
    block.head       = NIL;
    block.global     = template;

    template->tree = template_parse_work(&block);

    return(template);
}
