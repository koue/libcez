/*
 * Copyright (c) 2018-2022 Nikola Kolev <koue@chaosophia.net>
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

#include "cez_queue.h"

void
cez_queue_init(struct cez_queue *queue)
{
	TAILQ_INIT(&queue->head);
}

int
cez_queue_add(struct cez_queue *queue, const char *name, const char *value)
{
	struct cez_queue_entry *current;

	if (name == NULL || value == NULL)
		return (-1);
	if (cez_queue_get(queue, name) != NULL) {
		fprintf(stderr, "[ERROR] %s: %s already exists\n", __func__, name);
		return (-1);
	}
	if ((current = calloc(1, sizeof(*current))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	if ((current->name = strdup(name)) == NULL) {
		free(current);
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	if ((current->value = strdup(value)) == NULL) {
		free(current->name);
		free(current);
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	TAILQ_INSERT_TAIL(&queue->head, current, entry);
	return (0);
}

char *
cez_queue_get(struct cez_queue *queue, const char *name)
{
	struct cez_queue_entry *current;

	if (name == NULL)
		return (NULL);
	TAILQ_FOREACH(current, &queue->head, entry) {
		if (strcmp(name, current->name) == 0) {
			return (current->value);
		}
	}
	return (NULL);
}


void
cez_queue_print(struct cez_queue *queue)
{
	struct cez_queue_entry *current;
	TAILQ_FOREACH(current, &queue->head, entry) {
		printf("name=%s, value=%s\n", current->name, current->value);
	}
}

const char *
cez_queue_check(struct cez_queue *queue, const char **params)
{
	if (params == NULL)
		return (NULL);
	for (int i = 0; params[i] != NULL; ++i) {
		if (cez_queue_get(queue, params[i]) == NULL) {
			return (params[i]);
		}
	}
	return (NULL);
}

int
cez_queue_update(struct cez_queue *queue, const char *name, const char *value)
{
	struct cez_queue_entry *current;
	int found = 0;

	if (name == NULL || value == NULL)
		return (-1);
	TAILQ_FOREACH(current, &queue->head, entry) {
		if (strcmp(name, current->name) == 0) {
			free(current->value);
			if ((current->value = strdup(value)) == NULL) {
				free(current->name);
				free(current);
				fprintf(stderr, "[ERROR] %s: %s\n", __func__,
				    strerror(errno));
				exit(1);
			}
			found = 1;
		}
	}
	if (found == 0) {
		fprintf(stderr, "[ERROR] %s: %s doesn't exist\n", __func__, name);
		return (-1);
	}
	return (0);
}

int
cez_queue_remove(struct cez_queue *queue, const char *name)
{
	struct cez_queue_entry *current;

	if (name == NULL)
		return (-1);
	TAILQ_FOREACH(current, &queue->head, entry) {
		if (strcmp(name, current->name) == 0) {
			free(current->name);
			free(current->value);
			TAILQ_REMOVE(&queue->head, current, entry);
			free(current);
			return (0);
		}
	}
	return (-1);
}

void
cez_queue_purge(struct cez_queue *queue)
{
	struct cez_queue_entry *current;
	while (!TAILQ_EMPTY(&queue->head)) {
		current = TAILQ_FIRST(&queue->head);
		free(current->name);
		free(current->value);
		TAILQ_REMOVE(&queue->head, current, entry);
		free(current);
	}
}
