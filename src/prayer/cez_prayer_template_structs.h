/* $Cambridge: hermes/src/prayer/lib/template_structs.h,v 1.1 2010/07/07 08:52:14 dpc22 Exp $ */

/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

typedef enum {
    TEMPLATE_ITEM_LINES,
    TEMPLATE_ITEM_IFDEF,
    TEMPLATE_ITEM_IFEQ,
    TEMPLATE_ITEM_FOREACH,
    TEMPLATE_ITEM_LOOP,
    TEMPLATE_ITEM_CALL,
} TEMPLATE_ITEM_TYPE;

/* Any changes to the following must be mirrored in template_compile */

struct template_map_index {
    char *name;
    struct template_map *template_map;
    unsigned long *count;
};

struct template_map {
    char *name;
    struct template *template;
};

struct template {
    char *name;
    struct template_item *head;
    struct template_item *tail;
    unsigned long count;
    struct template_item *tree;
    struct str *error;
};

struct template_item {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    /* ... */
};

struct template_lines {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    char **first;
    int count;
};

struct template_ifdef {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    BOOL positive;
    char *expr;
    struct template_item *true_block;
    struct template_item *false_block;
};

struct template_ifeq {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    BOOL positive;
    char *name;
    char *value;
    struct template_item *true_block;
    struct template_item *false_block;
};

struct template_foreach {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    char *name;
    char *array;
    struct template_item *block;
};

struct template_loop {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    char *var;
    struct template_item *block;
};

struct template_call {
    unsigned long number;
    struct template_item *list_next;
    struct template_item *tree_next;
    TEMPLATE_ITEM_TYPE type;
    char *name;
    char *params;
};

