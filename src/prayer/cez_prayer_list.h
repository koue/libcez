/* $Cambridge: hermes/src/prayer/lib/list.h,v 1.3 2008/09/16 09:59:57 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Data structures for list class */

struct list_item {
    struct list_item *next;     /* Linked list */
    char *name;                 /* Name of list item, if relevant */
    /* List classes extend here */
};

struct list {
    BOOL use_case;              /* Case is significant in list lookups */
    struct pool *pool;          /* Allocation pool */
    struct list_item *head;     /* Start of list   */
    struct list_item *tail;     /* End of list     */
    unsigned long length;       /* Length of list  */
};

/* Function prototypes for list class */

struct list *list_create(struct pool *pool, BOOL use_case);

void list_free(struct list *list);

void list_unshift(struct list *list, struct list_item *item, char *name);
void list_push(struct list *list, struct list_item *item, char *name);

struct list_item *list_shift(struct list *list);
struct list_item *list_pop(struct list *list);

BOOL
list_insert_sorted(struct list *list, struct list_item *item, char *name);

unsigned long list_length(struct list *list);

struct list_item *list_lookup_byname(struct list *list, char *name);

struct list_item *list_lookup_byoffset(struct list *list,
                                       unsigned long offset);

BOOL list_remove_byname(struct list *list, char *name);

BOOL list_remove_byoffset(struct list *list, unsigned long offset);

BOOL list_rename_item(struct list *list, char *oldname, char *newname);
