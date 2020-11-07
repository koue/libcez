/*
 * Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cez_render.h"

void
cez_render_init(struct cez_render *render)
{
	TAILQ_INIT(&render->head);
}

int
cez_render_add(struct cez_render *render, const char *macro,
    const char *filepath, void *arg)
{
	struct cez_render_entry *current;

	if (macro == NULL) {
		fprintf(stderr, "%s: error: empty macro\n", __func__);
		return (-1);
	}

	if ((current = calloc(1, sizeof(*current))) == NULL) {
		fprintf(stderr, "%s: error: %s\n", __func__, strerror(errno));
		exit(1);
	}
	if ((current->macro = strdup(macro)) == NULL) {
		free(current);
		fprintf(stderr, "%s: error: %s\n", __func__, strerror(errno));
		exit(1);
	}
	if (filepath == NULL) {
		current->filepath = NULL;
	}
	else if ((current->filepath = strdup(filepath)) == NULL) {
		free(current->macro);
		free(current);
		fprintf(stderr, "%s: error: %s\n", __func__, strerror(errno));
		exit(1);
	}
	current->render_cb = arg;
	TAILQ_INSERT_TAIL(&render->head, current, entry);
	return (0);
}

struct cez_render_entry *
cez_render_get(struct cez_render *render, const char *macro)
{
	struct cez_render_entry *current;
	TAILQ_FOREACH(current, &render->head, entry) {
		if (strcmp(macro, current->macro) == 0) {
			return (current);
		}
	}
	return (NULL);
}

int
cez_render_remove(struct cez_render *render, const char *macro)
{
	struct cez_render_entry *current;
	TAILQ_FOREACH(current, &render->head, entry) {
		if (strcmp(macro, current->macro) == 0) {
			free(current->macro);
			free(current->filepath);
			TAILQ_REMOVE(&render->head, current, entry);
			free(current);
			return (0);
		}
	}
	return (-1);
}

void
cez_render_purge(struct cez_render *render)
{
	struct cez_render_entry *current;
	while (!TAILQ_EMPTY(&render->head)) {
		current = TAILQ_FIRST(&render->head);
		free(current->macro);
		free(current->filepath);
		TAILQ_REMOVE(&render->head, current, entry);
		free(current);
	}
}

int
cez_render_call(struct cez_render *render, const char *macro, void *data)
{
	struct cez_render_entry *entry;
	FILE *f;
	char s[8192];

	if ((entry = cez_render_get(render, macro)) == NULL) {
		fprintf(stderr, "%s: error: unknow macro %s\n", __func__, macro);
		return (-1);
	}

	if (entry->filepath == NULL) {
		if (entry->render_cb != NULL) {
			(*entry->render_cb)(macro, data);
		}
		return (0);
	}

	if ((f = fopen(entry->filepath, "re")) == NULL) {
		fprintf(stderr, "%s: error: fopen: %s: %s\n", __func__,
		    entry->filepath, strerror(errno));
		return (-1);
	}
	while (fgets(s, sizeof(s), f)) {
		char *a, *b;
		for (a = s; (b = strstr(a, "%%")) != NULL;) {
			*b = 0;
			printf("%s", a);
			a = b + 2;
			if ((b = strstr(a, "%%")) != NULL) {
				*b = 0;
				if (entry->render_cb != NULL) {
					(*entry->render_cb)(a, data);
				}
				a = b + 2;
			}
		}
		printf("%s", a);
	}
	fclose(f);
	return (0);
}
