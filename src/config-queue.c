/*
 * Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
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

#include "cez.h"

TAILQ_HEAD(config_queue_head, config_queue_param) config_queue_head = TAILQ_HEAD_INITIALIZER(config_queue_head);

struct config_queue_param {
	TAILQ_ENTRY(config_queue_param) config_queue_entry;
	char *name;
	char *value;
};

void config_queue_cb(const char *name, const char *value){
	struct config_queue_param *current;
	if ((current = calloc(1, sizeof(*current))) == NULL){
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	current->name = strdup(name);
	if (current->name == NULL) {
		free(current);
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	current->value = strdup(value);
	if (current->value == NULL) {
		free(current->name);
		free(current);
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	TAILQ_INSERT_TAIL(&config_queue_head, current, config_queue_entry);
}

void config_queue_print(void){
	struct config_queue_param *current;
	TAILQ_FOREACH(current, &config_queue_head, config_queue_entry){
		printf("name=%s, value=%s\n", current->name, current->value);
	}
}

void config_queue_purge(void){
	struct config_queue_param *current;
	while(!TAILQ_EMPTY(&config_queue_head)){
		current = TAILQ_FIRST(&config_queue_head);
		free(current->name);
		free(current->value);
		TAILQ_REMOVE(&config_queue_head, current, config_queue_entry);
		free(current);
	}
}

char *config_queue_value_get(const char *name) {
	struct config_queue_param *current;

	TAILQ_FOREACH(current, &config_queue_head, config_queue_entry) {
		if (strcmp(name, current->name) == 0) {
			return (current->value);
		}
	}
	return (NULL);
}
