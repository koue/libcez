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

#ifndef _CEZ_RENDER_H
#define _CEZ_RENDER_H

#include <sys/queue.h>

typedef void (*cez_render_cb)(const char *macro, void *arg);

struct cez_render_entry {
	TAILQ_ENTRY(cez_render_entry) entry;
	char *macro;
	char *filepath;
	cez_render_cb render_cb;
};

struct cez_render {
	TAILQ_HEAD(, cez_render_entry) head;
};

void cez_render_init(struct cez_render *render);
int cez_render_add(struct cez_render *render, const char *macro,
    const char *filepath, void *arg);
struct cez_render_entry *cez_render_get(struct cez_render *render,
    const char *macro);
int cez_render_remove(struct cez_render *render, const char *macro);
void cez_render_purge(struct cez_render *render);
int cez_render_call(struct cez_render *render, const char *macro, void *arg);

#endif
