/*
 * Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net>
 * Copyright (c) 2006 - 2015 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2006, 2007 Pierre-Yves Ritschard <pyr@openbsd.org>
 * Copyright (c) 2003, 2004 Henning Brauer <henning@openbsd.org>
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

//#include <sys/queue.h>
//#include <sys/types.h>

TAILQ_HEAD(kvlist, kv);
RB_HEAD(kvtree, kv);

typedef struct kvtree kvtree;

struct kv {
	char			*kv_key;
	char			*kv_value;

	uint8_t			 kv_flags;

	struct kvlist		 kv_children;
	struct kv		*kv_parent;
	TAILQ_ENTRY(kv)		 kv_entry;

	RB_ENTRY(kv)		 kv_node;
};

void		 kv_init(struct kvtree *);
struct kv	*kv_add(struct kvtree *, char *, char *);
int		 kv_set(struct kv *, char *, ...)
		    __attribute__((__format__ (printf, 2, 3)));
int		 kv_setkey(struct kv *, char *, ...)
		    __attribute__((__format__ (printf, 2, 3)));
void		 kv_delete(struct kvtree *, struct kv *);
struct kv	*kv_extend(struct kvtree *, struct kv *, char *);
void		 kv_purge(struct kvtree *);
void		 kv_free(struct kv *);
struct kv	*kv_inherit(struct kv *, struct kv *);
struct kv	*kv_find(struct kvtree *, struct kv *);
int		 kv_cmp(struct kv *, struct kv *);
RB_PROTOTYPE(kvtree, kv, kv_node, kv_cmp);
