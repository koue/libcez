/* $Cambridge: hermes/src/prayer/lib/list.c,v 1.3 2008/09/16 09:59:57 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* list_create() *********************************************************
 *
 * Create a fresh list structure
 *      pool: Target pool for list and all list elements
 *  use_case: T => case is significant in _by_name operations.
 ************************************************************************/

struct list *list_create(struct pool *pool, BOOL use_case)
{
    struct list *result = pool_alloc(pool, sizeof(struct list));

    result->pool = pool;
    result->use_case = use_case;
    result->head = NIL;
    result->tail = NIL;
    result->length = 0L;

    return (result);
}

/* list_free() ***********************************************************
 *
 * Free a list structure, including all list nodes. NOOP unless list
 * has been allocated in the NIL pool.
 ************************************************************************/

void list_free(struct list *list)
{
    struct list_item *current, *next;

    if (list->pool)             /* NOOP if pool */
        return;

    for (current = list->head; current; current = next) {
        next = current->next;
        free(current);
    }
    free(list);
}

/* list_length() *********************************************************
 *
 * Return current length of the list.
 ************************************************************************/

unsigned long list_length(struct list *list)
{
    return (list->length);
}

/* ====================================================================== */

/* list_unshift() ********************************************************
 *
 * Add new entry to start of a list
 *     list:
 *     item: List item to add
 *     name: Name for this list item, if any
 ************************************************************************/

void list_unshift(struct list *list, struct list_item *item, char *name)
{
    if (item == NIL)
        item = pool_alloc(list->pool, sizeof(struct list_item));

    item->next = NIL;

    if (name)
        item->name = pool_strdup(list->pool, name);
    else
        item->name = NIL;

    if (list->head) {
        item->next = list->head;
        list->head = item;
    } else {
        item->next = NIL;
        list->head = list->tail = item;
    }

    list->length++;
}

/* list_shift() **********************************************************
 *
 * Removes (but doesn't free) entry from the start of the list
 *   list:
 *
 * Returns: Ptr to list item which was removed.
 ************************************************************************/

struct list_item *list_shift(struct list *list)
{
    struct list_item *item;

    if (list->head == NIL)
        return (NIL);

    item = list->head;

    if (list->head == list->tail)
        list->head = list->tail = NIL;  /* Remove single item from list */
    else
        list->head = list->head->next;  /* Remove first item from list */

    list->length--;

    return (item);
}

/* ====================================================================== */

/* list_push() ***********************************************************
 *
 * Add an entry to the end of a list
 *    list:
 *    item: List item to push
 *    name: Name for new list item, if any.
 ************************************************************************/

void list_push(struct list *list, struct list_item *item, char *name)
{
    if (item == NIL)
        item = pool_alloc(list->pool, sizeof(struct list_item));

    item->next = NIL;

    if (name)
        item->name = pool_strdup(list->pool, name);
    else
        item->name = NIL;

    if (list->tail)
        list->tail = list->tail->next = item;
    else
        list->head = list->tail = item;

    list->length++;
}

/* list_pop() ************************************************************
 *
 * Removes (but doesn't free) entry from the end of a list
 *   list:
 *
 * Returns: Ptr to list item which was removed.
 ************************************************************************/

struct list_item *list_pop(struct list *list)
{
    struct list_item *item;

    if (list->head == NIL)
        return (NIL);

    if (list->head == list->tail) {
        /* Remove single item from list */
        item = list->head;
        list->head = NIL;
        list->tail = NIL;
    } else {
        struct list_item *last = list->head;

        /* Find penultimate item on list: inefficient with single linked list! */
        while (last->next)
            last = last->next;

        item = last->next;
        last->next = NIL;
    }

    list->length--;
    return (item);
}

/* ====================================================================== */

BOOL
list_insert_sorted(struct list * list, struct list_item * item, char *name)
{
    struct list_item *last, *next;

    if (item == NIL)
        item = pool_alloc(list->pool, sizeof(struct list_item));

    item->next = NIL;

    if (name)
        item->name = pool_strdup(list->pool, name);
    else
        item->name = NIL;

    if (list->use_case) {
        if (((next = list->head) == NIL) || (strcmp(next->name, name) > 0)) {
            item->next = list->head;    /* Insert at start of list */
            list->head = item;
        } else {
            do {
                last = next;    /* (next != NIL) */
                next = next->next;
            }
            while (next && (strcmp(next->name, name) < 0));

            /* Add to this position in list */
            item->next = next;
            last->next = item;
        }
    } else {
        if (((next = list->head) == NIL)
            || (strcasecmp(next->name, name) > 0)) {
            item->next = list->head;    /* Insert at start of list */
            list->head = item;
        } else {
            do {
                last = next;    /* (next != NIL) */
                next = next->next;
            }
            while (next && (strcasecmp(next->name, name) < 0));

            /* Add to this position in list */
            item->next = next;
            last->next = item;
        }
    }
    list->length++;
    return (T);
}

/* ====================================================================== */

/* list_lookup_byname() **************************************************
 *
 * List lookup by name
 *    list:
 *    name: Name to look up.
 *
 * Returns: list item or NIL if not found
 ************************************************************************/

struct list_item *list_lookup_byname(struct list *list, char *name)
{
    struct list_item *item;

    if (list->use_case) {
        for (item = list->head; item; item = item->next) {
            if (item->name && !strcmp(item->name, name))
                return (item);
        }
    } else {
        for (item = list->head; item; item = item->next) {
            if (item->name && !strcasecmp(item->name, name))
                return (item);
        }
    }

    return (NIL);
}

/* list_lookup_byoffset() ***********************************************
 *
 * List lookup by offset
 *    list:
 *  offset: Offset into list
 *
 * Returns: list item or NIL if offset is out of range
 ************************************************************************/

struct list_item *list_lookup_byoffset(struct list *list,
                                       unsigned long offset)
{
    struct list_item *item;

    for (item = list->head; item; item = item->next) {
        if (offset == 0)
            return (item);
        offset--;
    }
    return (NIL);
}

/* ====================================================================== */

/* list_free_item() ******************************************************
 *
 * Free single list item
 *   list:
 *   item: Item to remove
 ************************************************************************/

static void list_free_item(struct list *list, struct list_item *item)
{
    if (list->pool)
        return;

    if (item->name)
        free(item->name);

    free(item);
}

/* list_remove_byname() **************************************************
 *
 * Remove named entry from list
 *    list:
 *    name: Name of entry to remove
 *
 * Returns: T on sucess, NIL otherwise
 ************************************************************************/

BOOL list_remove_byname(struct list *list, char *name)
{
    struct list_item *last, *current;

    /* Check for empty list? */
    if (list->head == NIL)
        return (NIL);

    /* Remove from start of list? */
    if (list->use_case) {
        if ((list->head->name) && !strcmp(name, list->head->name)) {
            current = list->head;
            list->head = current->next;

            /* Removed sole entry from list? */
            if (current->next == NIL)
                list->tail = NIL;

            list_free_item(list, current);
            list->length--;
            return (T);
        }

        /* Remove from middle or end of list */
        for (last = list->head; (current = last->next); last = current) {
            if (current->name && !strcmp(current->name, name)) {
                if ((last->next = current->next) == NIL)
                    list->tail = last;  /* Removed last item in list */

                list_free_item(list, current);
                list->length--;
                return (T);
            }
        }
        return (NIL);
    }

    if ((list->head->name) && !strcasecmp(name, list->head->name)) {
        current = list->head;
        list->head = current->next;

        /* Removed sole entry from list? */
        if (current->next == NIL)
            list->tail = NIL;

        list_free_item(list, current);
        list->length--;
        return (T);
    }

    /* Remove from middle or end of list */
    for (last = list->head; (current = last->next); last = current) {
        if (current->name && !strcasecmp(current->name, name)) {
            if ((last->next = current->next) == NIL)
                list->tail = last;      /* Removed last item in list */

            list_free_item(list, current);
            list->length--;
            return (T);
        }
    }
    return (NIL);
}

/* list_remove_byoffset() ************************************************
 *
 * Remove list entry by offset
 *    list:
 *  offset: List offset to remove
 *
 * Returns: T on success, NIl otherwise
 ************************************************************************/

BOOL list_remove_byoffset(struct list * list, unsigned long offset)
{
    struct list_item *current, *last;

    /* Check for empty list? */
    if (list->head == NIL)
        return (NIL);

    /* Remove from start of list? */
    if (offset == 0) {
        current = list->head;
        list->head = current->next;

        /* Removed sole entry from list? */
        if (list->head == NIL)
            list->tail = NIL;

        list_free_item(list, current);
        list->length--;
        return (T);
    }

    offset--;

    /* Remove from middle or end of list */
    for (last = list->head; (current = last->next); last = current) {
        if (offset == 0) {
            if ((last->next = current->next) == NIL)
                list->tail = last;      /* Removed last item in list */

            list_free_item(list, current);
            list->length--;
            return (T);
        }
        offset--;
    }

    return (NIL);
}

/* ====================================================================== */

/* list_rename_item() ****************************************************
 *
 * Rename a list item
 *    list:
 * oldname: Name of entry to rename
 * newname: New name for entry
 *
 * Returns: T on success, NIL otherwise
 ************************************************************************/

BOOL list_rename_item(struct list * list, char *oldname, char *newname)
{
    struct list_item *current;

    if (list->use_case) {
        for (current = list->head; current; current = current->next) {
            if (!strcmp(current->name, oldname)) {
                free(current->name);
                current->name = pool_strdup(NIL, newname);
                return (T);
            }
        }
        return (NIL);
    }

    for (current = list->head; current; current = current->next) {
        if (!strcasecmp(current->name, oldname)) {
            free(current->name);
            current->name = pool_strdup(NIL, newname);
            return (T);
        }
    }
    return (NIL);
}
