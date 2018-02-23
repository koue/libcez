/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Associative Arrays */

struct assoc_list {             /* Single magazine for hash table */
    struct assoc_list *next;    /* Linked list */
    char *key;
    void *value;
};

struct assoc {
    struct pool *pool;          /* Allocate in this pool */
    BOOL use_case;              /* T => lookups are case dependant */
    unsigned long size;         /* Number of magazines for hash table */
    unsigned long scanlist;     /* For assoc scan stuff */
    struct assoc_list *scan_elt;
    struct assoc_list *list[1]; /* Placeholder for array */
};

#define ASSOC_MULT (29)         /* Strange prime to distribute keys better */

struct assoc *assoc_create(struct pool *p, unsigned long size,
                           BOOL use_case);

void assoc_free(struct assoc *h);

void assoc_update(struct assoc *h, char *key, void *value, BOOL dup);

BOOL assoc_delete(struct assoc *h, char *key);

void *assoc_lookup(struct assoc *h, char *key);

void assoc_scan_reset(struct assoc *h);

BOOL assoc_scan_next(struct assoc *h, char **keyp, void **valuep);
