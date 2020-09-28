/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Data structures for list class */

struct cez_list_item {
    struct cez_list_item *next;     /* Linked list */
    char *name;                 /* Name of list item, if relevant */
    /* List classes extend here */
};

struct cez_list {
    BOOL use_case;              /* Case is significant in list lookups */
    struct pool *pool;          /* Allocation pool */
    struct cez_list_item *head;     /* Start of list   */
    struct cez_list_item *tail;     /* End of list     */
    unsigned long length;       /* Length of list  */
};

/* Function prototypes for list class */

struct cez_list *cez_list_create(struct pool *pool, BOOL use_case);

void cez_list_free(struct cez_list *list);

void cez_list_unshift(struct cez_list *list, struct cez_list_item *item, char *name);
void cez_list_push(struct cez_list *list, struct cez_list_item *item, char *name);

struct cez_list_item *cez_list_shift(struct cez_list *list);
struct cez_list_item *cez_list_pop(struct cez_list *list);

BOOL
cez_list_insert_sorted(struct cez_list *list, struct cez_list_item *item, char *name);

unsigned long cez_list_length(struct cez_list *list);

struct cez_list_item *cez_list_lookup_byname(struct cez_list *list, char *name);

struct cez_list_item *cez_list_lookup_byoffset(struct cez_list *list,
                                       unsigned long offset);

BOOL cez_list_remove_byname(struct cez_list *list, char *name);

BOOL cez_list_remove_byoffset(struct cez_list *list, unsigned long offset);

BOOL cez_list_rename_item(struct cez_list *list, char *oldname, char *newname);
