/*
 * Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net>
 * Copyright (c) 2014 Reyk Floeter <reyk@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/types.h>

#include "cez_kv.h"

struct kv *
kv_add(struct kvtree *keys, char *key, char *value)
{
	struct kv	*kv, *oldkv;

	if (key == NULL)
		return (NULL);
	if ((kv = calloc(1, sizeof(*kv))) == NULL)
		return (NULL);
	if ((kv->kv_key = strdup(key)) == NULL) {
		free(kv);
		return (NULL);
	}
	if (value != NULL &&
	    (kv->kv_value = strdup(value)) == NULL) {
		free(kv->kv_key);
		free(kv);
		return (NULL);
	}
	TAILQ_INIT(&kv->kv_children);

	if ((oldkv = RB_INSERT(kvtree, keys, kv)) != NULL) {
		TAILQ_INSERT_TAIL(&oldkv->kv_children, kv, kv_entry);
		kv->kv_parent = oldkv;
	}

	return (kv);
}

static int
kv_isspace(char c)
{
	return c==' ' || (c<='\r' && c>='\t');
}

int
kv_add_list(struct kvtree *keys, char *list, int terminator)
{
	struct kv	*kv;
	char		*zList, *zFree;

	zFree = zList = strdup(list);
	while (*zList) {
		char *zName;
		char *zValue;
		while (kv_isspace(*zList))
			zList++;
		zName = zList;
		while (*zList && *zList != '=' && *zList != terminator)
			zList++;
		if (*zList == '=') {
			*zList = 0;
			zList++;
			zValue = zList;
			while(*zList && *zList != terminator)
				zList++;
			if (*zList) {
				*zList = 0;
				zList++;
			}
		} else {
			if (*zList)
				*zList++ = 0;
			zValue = "";
		}
		if ((kv = kv_add(keys, zName, zValue)) == NULL) {
			free(zFree);
			return (-1);
		}
	}
	free(zFree);
	return (0);
}


int
kv_set(struct kv *kv, char *fmt, ...)
{
	va_list		  ap;
	char		*value = NULL;
	struct kv	*ckv;
	int		ret;

	va_start(ap, fmt);
	ret = vasprintf(&value, fmt, ap);
	va_end(ap);
	if (ret == -1)
		return (-1);

	/* Remove all children */
	while ((ckv = TAILQ_FIRST(&kv->kv_children)) != NULL) {
		TAILQ_REMOVE(&kv->kv_children, ckv, kv_entry);
		kv_free(ckv);
		free(ckv);
	}

	/* Set the new value */
	free(kv->kv_value);
	kv->kv_value = value;

	return (0);
}

int
kv_setkey(struct kv *kv, char *fmt, ...)
{
	va_list  ap;
	char	*key = NULL;
	int	ret;

	va_start(ap, fmt);
	ret = vasprintf(&key, fmt, ap);
	va_end(ap);
	if (ret == -1)
		return (-1);

	free(kv->kv_key);
	kv->kv_key = key;

	return (0);
}

void
kv_delete(struct kvtree *keys, struct kv *kv)
{
	struct kv	*ckv;

	RB_REMOVE(kvtree, keys, kv);

	/* Remove all children */
	while ((ckv = TAILQ_FIRST(&kv->kv_children)) != NULL) {
		TAILQ_REMOVE(&kv->kv_children, ckv, kv_entry);
		kv_free(ckv);
		free(ckv);
	}

	kv_free(kv);
	free(kv);
}

struct kv *
kv_extend(struct kvtree *keys, struct kv *kv, char *value)
{
	char		*newvalue;

	if (kv == NULL) {
		return (NULL);
	} else if (kv->kv_value != NULL) {
		if (asprintf(&newvalue, "%s%s", kv->kv_value, value) == -1)
			return (NULL);

		free(kv->kv_value);
		kv->kv_value = newvalue;
	} else if ((kv->kv_value = strdup(value)) == NULL)
		return (NULL);

	return (kv);
}

void
kv_purge(struct kvtree *keys)
{
	struct kv	*kv;

	while ((kv = RB_MIN(kvtree, keys)) != NULL)
		kv_delete(keys, kv);
}

void
kv_free(struct kv *kv)
{
	free(kv->kv_key);
	kv->kv_key = NULL;
	free(kv->kv_value);
	kv->kv_value = NULL;
	memset(kv, 0, sizeof(*kv));
}

struct kv *
kv_inherit(struct kv *dst, struct kv *src)
{
	memset(dst, 0, sizeof(*dst));
	memcpy(dst, src, sizeof(*dst));
	TAILQ_INIT(&dst->kv_children);

	if (src->kv_key != NULL) {
		if ((dst->kv_key = strdup(src->kv_key)) == NULL) {
			kv_free(dst);
			return (NULL);
		}
	}
	if (src->kv_value != NULL) {
		if ((dst->kv_value = strdup(src->kv_value)) == NULL) {
			kv_free(dst);
			return (NULL);
		}
	}

	return (dst);
}

struct kv *
kv_find(struct kvtree *keys, struct kv *kv)
{
	struct kv	*match;
#if 0
	const char	*key;

	if (kv->kv_flags & KV_FLAG_GLOBBING) {
		/* Test header key using shell globbing rules */
		key = kv->kv_key == NULL ? "" : kv->kv_key;
		RB_FOREACH(match, kvtree, keys) {
			if (fnmatch(key, match->kv_key, FNM_CASEFOLD) == 0)
				break;
		}
	} else {
#endif
		/* Fast tree-based lookup only works without globbing */
		match = RB_FIND(kvtree, keys, kv);
#if 0
	}
#endif

	return (match);
}

int
kv_cmp(struct kv *a, struct kv *b)
{
	return (strcasecmp(a->kv_key, b->kv_key));
}

void
kv_init(struct kvtree *keys)
{
	RB_INIT(keys);
}

RB_GENERATE(kvtree, kv, kv_node, kv_cmp);
