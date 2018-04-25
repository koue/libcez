/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Compiled template cotains following sections
 *
 * 1) Forward declarations to stop compiler warnings:
 *
 *      static struct template_ifdef _t17;
 *
 * 2) Actual declarations, written as top down structure
 *
 *      static struct template_ifdef _t17 = { ... };
 *
 * 3) External interface:
 *
 *      struct template _template_abook_list = { ... };
 *
 *    which gives us a name which can be linked into the master array.
 */

static void
template_compile_string(char *s, FILE *file, BOOL atend)
{
    char c;

    if (!s) {
        fprintf(file, "  NIL%s\n", (atend) ? "" : ",");
        return;
    }

    fprintf(file, "  \"");
    while ((c=*s++)) {
        if ((c == '"') || (c == '\\'))
            fputc('\\', file);
        fputc(c, file);
    }
    fprintf(file, "\"%s\n", (atend) ? "" : ",");
}

static void
template_compile_block(struct template_item *item, FILE *file, BOOL atend)
{
    if (item)
        fprintf(file, "  (struct template_item *)((void *)&_t%lu)",
                item->number);
    else
        fprintf(file, "  NIL");

    if (!atend)
        fputc(',', file);
    fputc('\n', file);
}

static void
template_compile_common(struct template_item *item, FILE *file)
{
    fprintf(file, "  %lu,\n", item->number);
    template_compile_block(item->list_next, file, NIL);
    template_compile_block(item->tree_next, file, NIL);
}

static void
template_compile_lines(struct template_lines *item, FILE *file)
{
    char **p = item->first;
    int count = item->count;

    fprintf(file, "static char *_t%lu_lines[] = {\n", item->number);
    while (*p && (count > 0)) {
        template_compile_string(*p, file, NIL);
        p++;
        count--;
    }
    fprintf(file, "  NIL\n");
    fprintf(file, "};\n\n");

    fprintf(file, "static struct template_lines _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_LINES,\n");
    fprintf(file, "  _t%lu_lines,\n", item->number);
    fprintf(file, "  %d\n", item->count);
    fprintf(file, "};\n\n");
}

static void
template_compile_ifdef(struct template_ifdef *item, FILE *file)
{

    fprintf(file, "static struct template_ifdef _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_IFDEF,\n");
    fprintf(file, "  %s,\n", (item->positive) ? "T" : "NIL");
    template_compile_string(item->expr, file, NIL);
    template_compile_block(item->true_block, file, NIL);
    template_compile_block(item->false_block, file, T);
    fprintf(file, "};\n\n");
}

static void
template_compile_ifeq(struct template_ifeq *item, FILE *file)
{

    fprintf(file, "static struct template_ifeq _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_IFEQ,\n");
    fprintf(file, "  %s,\n", (item->positive) ? "T" : "NIL");
    template_compile_string(item->name, file, NIL);
    template_compile_string(item->value, file, NIL);
    template_compile_block(item->true_block, file, NIL);
    template_compile_block(item->false_block, file, T);
    fprintf(file, "};\n\n");
}

static void
template_compile_foreach(struct template_foreach *item, FILE *file)
{

    fprintf(file, "static struct template_foreach _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_FOREACH,\n");
    template_compile_string(item->name, file, NIL);
    template_compile_string(item->array, file, NIL);
    template_compile_block(item->block, file, T);
    fprintf(file, "};\n\n");
}

static void
template_compile_loop(struct template_loop *item, FILE *file)
{

    fprintf(file, "static struct template_loop _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_LOOP,\n");
    template_compile_string(item->var, file, NIL);
    template_compile_block(item->block, file, T);
    fprintf(file, "};\n\n");
}

static void
template_compile_call(struct template_call *item, FILE *file)
{
    fprintf(file, "static struct template_call _t%lu = {\n", item->number);
    template_compile_common((struct template_item *)item, file);
    fprintf(file, "  TEMPLATE_ITEM_CALL,\n");
    template_compile_string(item->name, file, NIL);
    template_compile_string(item->params, file, T);
    fprintf(file, "};\n\n");
}

static void
template_compile_data(struct template *template, FILE *file)
{
    struct template_item *item = template->head;

    while (item) {
        switch (item->type) {
        case TEMPLATE_ITEM_LINES:
            template_compile_lines((struct template_lines *)item, file);
            break;
        case TEMPLATE_ITEM_IFDEF:
            template_compile_ifdef((struct template_ifdef *)item, file);
            break;
        case TEMPLATE_ITEM_IFEQ:
            template_compile_ifeq((struct template_ifeq *)item, file);
            break;
        case TEMPLATE_ITEM_FOREACH:
            template_compile_foreach((struct template_foreach *)item,
                                     file);
            break;
        case TEMPLATE_ITEM_LOOP:
            template_compile_loop((struct template_loop *)item, file);
            break;
        case TEMPLATE_ITEM_CALL:
            template_compile_call((struct template_call *)item, file);
            break;
        }
        item = item->list_next;
    }
}

static void
template_compile_decls(struct template *template, FILE *file)
{
    struct template_item *item = template->head;

    while (item) {
        fprintf(file, "static struct");
        switch (item->type) {
        case TEMPLATE_ITEM_LINES:
            fprintf(file, " template_lines ");
            break;
        case TEMPLATE_ITEM_IFDEF:
            fprintf(file, " template_ifdef ");
            break;
        case TEMPLATE_ITEM_IFEQ:
            fprintf(file, " template_ifeq ");
            break;
        case TEMPLATE_ITEM_FOREACH:
            fprintf(file, " template_foreach ");
            break;
        case TEMPLATE_ITEM_LOOP:
            fprintf(file, " template_loop ");
            break;
        case TEMPLATE_ITEM_CALL:
            fprintf(file, " template_call ");
            break;
        }
        fprintf(file, "_t%lu;\n", item->number);
        item = item->list_next;
    }
    fprintf(file, "\n");
}

static void
template_compile_interface(char *prefix, struct template *template, FILE *file)
{
    fprintf(file, "struct template _template_%s_%s = {\n",
            prefix, template->name);
    template_compile_string(template->name, file, NIL);
    template_compile_block(template->head, file, NIL);
    template_compile_block(template->tail, file, NIL);
    fprintf(file, "  %lu,\n", template->count);
    template_compile_block(template->tree, file, NIL);
    fprintf(file, "  NIL\n");
    fprintf(file, "};\n\n");
}

BOOL
template_compile(char *prefix, struct template *template, FILE *file)
{
    fprintf(file, "#include \"cez_prayer_misc.h\"\n");
    fprintf(file, "#include \"cez_prayer_template_structs.h\"\n\n");
    template_compile_decls(template, file);
    template_compile_data(template, file);
    template_compile_interface(prefix, template, file);

    return(T);
}
