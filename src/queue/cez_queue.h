/*
 * Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net>
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

#ifndef _CEZ_QUEUE_H_
#define _CEZ_QUEUE_H_

#include <sys/queue.h>

struct cez_queue_entry {
	TAILQ_ENTRY(cez_queue_entry) entry;
	char *name;
	char *value;
};

struct cez_queue {
	TAILQ_HEAD(, cez_queue_entry) head;
};


/*
** CONFIGFILE
*/
int configfile_parse(const char *filename, struct cez_queue *queue);

/*
** QUEUE
*/
void cez_queue_init(struct cez_queue *queue);
#define cqi cez_queue_init
int cez_queue_add(struct cez_queue *queue, const char *name, const char
    *value);
#define cqa cez_queue_add
int cez_queue_add_list(struct cez_queue *queue, char *list, int terminator);
#define cqal cez_queue_add_list
char *cez_queue_get(struct cez_queue *queue, const char *name);
#define cqg cez_queue_get
const char *cez_queue_check(struct cez_queue *queue, const char **params);
#define cqc cez_queue_check
int cez_queue_update(struct cez_queue *queue, const char *name, const char
    *value);
#define cqu cez_queue_update
int cez_queue_remove(struct cez_queue *queue, const char *name);
#define cqr cez_queue_remove
void cez_queue_print(struct cez_queue *queue);
void cez_queue_purge(struct cez_queue *queue);
#define cqp cez_queue_purge

#endif
