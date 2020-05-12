/* Copyright (c) 2018-2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_core_pool.h"
#include "cez_core_assoc.h"

/* Class which provides simple associative arrays (aka hash tables). Used
 * to be called "hash", however c-client in IMAP 2001 toolkit defines its
 * own hash class (isn't namespace polution in C fun boys and girls?).
 * You will still see lots of "struct assoc *h" scattered around Prayer */

/* ====================================================================== */

/* assoc_create() *******************************************************
 *
 * Create a new associative array.
 * pool:     Pool to use for allocations
 * size:     Number of magazines in the hash table
 * use_case: Case is significant in hash table lookups
 *
 * Returns: Ptr to new assocative array
 ***********************************************************************/

struct assoc *assoc_create(struct pool *pool, unsigned long size,
                           BOOL use_case)
{
    struct assoc *h;
    unsigned long i;

    if (size < 16)
        fprintf(stderr, "size argument to create_assoc() too small\n");

    h = pool_alloc(pool, (sizeof(struct assoc) +
                          (size - 1) * sizeof(struct assoc_list)));

    h->pool = pool;
    h->size = size;

    for (i = 0; i < size; i++)
        h->list[i] = NIL;

    h->use_case = use_case;
    h->scanlist = 0;
    h->scan_elt = NIL;

    return (h);
}

/* assoc_free() *********************************************************
 *
 * Free associative array. NOOP if pool defined.
 *   h: Associative array to free.
 ***********************************************************************/

void assoc_free(struct assoc *h)
{
    struct assoc_list *current, *next;
    unsigned long i;

    if (h->pool)                /* NOOP if assoc allocated from pool */
        return;

    /* Free each linked list chain */
    for (i = 0; i < h->size; i++) {
        for (current = h->list[i]; current; current = next) {
            next = current->next;
            free(current);
        }
    }

    /* Free central assoc value */
    free(h);
}

/* ====================================================================== */

/* assoc_make_value() ***************************************************
 *
 * Convert string into hash value: defines linked list magazine to use
 *  key:     Key to hash
 * size:     Number of magazines in the hash table
 * use_case: Case is significant in hash table lookups
 *
 * Returns: Hash table magazine that will/should contain this string
 ***********************************************************************/

static unsigned long
assoc_make_value(char *key, unsigned long size, BOOL use_case)
{
    unsigned long value = 0;
    char c;

    while ((c = *key++)) {
        if (!use_case)
            c = tolower(c);

        value *= ASSOC_MULT;
        value += c;
    }

    /* Return value % size, optimise some common cases */
    switch (size) {
    case 16:
        return (value & 15);
    case 256:
        return (value & 255);
    case 1024:
        return (value & 1023);
    default:
        return (value % size);
    }
}

/* ====================================================================== */

/* assoc_update() *******************************************************
 *
 * Insert entry into associative array. Replaces any existing value
 *     h: Associative array
 *   key: Key for hash entry
 * value: Value for hash entry
 *   dup: Duplicate key and value to ensure unique reference
 ***********************************************************************/

void assoc_update(struct assoc *h, char *key, void *value, BOOL dup)
{
    unsigned long offset;
    struct assoc_list *current, *newelt;

    /* Generate new assoc_list element */

    newelt = pool_alloc(h->pool, sizeof(struct assoc_list));
    newelt->next = NIL;

    if (dup) {
        newelt->key = pool_strdup(h->pool, key);
        newelt->value = pool_strdup(h->pool, value);
    } else {
        newelt->key = key;
        newelt->value = value;
    }

    offset = assoc_make_value(key, h->size, h->use_case);

    for (current = h->list[offset]; current; current = current->next) {
        if (h->use_case) {
            if (!strcmp(key, current->key)) {
                /* Replace existing key in this linked list */
                current->key = newelt->key;
                current->value = newelt->value;
                return;
            }
        } else {
            if (!strcasecmp(key, current->key)) {
                /* Replace existing key in this linked list */
                current->key = newelt->key;
                current->value = newelt->value;
                return;
            }
        }
    }

    /* Not found: Add new key to front of linked list */

    newelt->next = h->list[offset];
    h->list[offset] = newelt;
}

/* assoc_delete() *******************************************************
 *
 * Delete entry from associative array. Frees key and value if no pool
 * is defined for this associative array.
 *     h: Associative array
 *   key: Key for entry to be removed.
 *
 * Returns: T if entry existed. NIL => operation was NOOP.
 ***********************************************************************/

BOOL assoc_delete(struct assoc * h, char *key)
{
    unsigned long offset;
    struct assoc_list *l;
    struct assoc_list *l2;

    if (!h)
        return (NIL);

    offset = assoc_make_value(key, h->size, h->use_case);
    l = h->list[offset];

    if (l == NIL)               /* Hash slot was empty */
        return (NIL);

    if (h->use_case) {
        if (!strcmp(key, l->key)) {
            /* Remove item from front of list */
            h->list[offset] = l->next;

            if (!h->pool) {
                /* Free key and value strings unless pool allocated */
                free(l->key);
                free(l->value);
            }
            return (T);
        }
    } else {
        if (!strcasecmp(key, l->key)) {
            /* Remove item from front of list */
            h->list[offset] = l->next;

            if (!h->pool) {
                /* Free key and value strings unless pool allocated */
                free(l->key);
                free(l->value);
            }
            return (T);
        }
    }

    while ((l2 = l->next)) {
        if (h->use_case) {
            if (!strcmp(key, l2->key)) {
                l->next = l2->next;     /* Remove next link from chain */

                if (!h->pool) {
                    free(l2->key);      /* free key and value strings */
                    free(l2->value);
                }
                return (T);
            }
        } else {
            if (!strcasecmp(key, l2->key)) {
                l->next = l2->next;     /* Remove next link from chain */

                if (!h->pool) {
                    free(l2->key);      /* free key and value strings */
                    free(l2->value);
                }
                return (T);
            }
        }
        l = l->next;
    }

    return (NIL);               /* Couldn't find value in assoc table */
}

/* assoc_lookup() *******************************************************
 *
 * Lookup entry in associative array.
 *     h: Associative array
 *   key: Key to be looked up
 *
 * Returns: value which corresponds to key. NIL => none.
 ***********************************************************************/

void *assoc_lookup(struct assoc *h, char *key)
{
    unsigned long offset;
    struct assoc_list *l;

    if (!h)
        return (NIL);

    offset = assoc_make_value(key, h->size, h->use_case);
    l = h->list[offset];

    if (h->use_case) {
        while (l) {
            if (!strcmp(key, l->key))
                return (l->value);
            l = l->next;
        }
    } else {
        while (l) {
            if (!strcasecmp(key, l->key))
                return (l->value);
            l = l->next;
        }
    }

    /* Couldn't find value in assoc table */
    return (NIL);
}

/* ====================================================================== */

/* assoc_scan_reset() ****************************************************
 *
 * Prepare hash for sequential scan
 ************************************************************************/

void assoc_scan_reset(struct assoc *h)
{
    h->scanlist = 0;
    h->scan_elt = h->list[0];
}

/* assoc_scan_next() *****************************************************
 *
 * Find next (key, value) pair in associative array
 *      h:
 *   keyp: Record next key here if non-NIL
 * valuep: Record next value here if non-NIL
 ************************************************************************/

BOOL assoc_scan_next(struct assoc *h, char **keyp, void **valuep)
{
    if (h->scan_elt == NIL) {
        /* Current magazine empty: find next magazine with data */
        while (++h->scanlist < h->size) {
            if ((h->scan_elt = h->list[h->scanlist]) != NIL)
                break;
        }
    }

    /* No next (key, value) pair */
    if (!h->scan_elt)
        return (NIL);

    /* Found next (key, value) pair */
    if (keyp)
        *keyp = h->scan_elt->key;

    if (valuep)
        *valuep = h->scan_elt->value;

    h->scan_elt = h->scan_elt->next;

    return (T);
}
