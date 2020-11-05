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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cez_test.h"

#include "cez_render.h"

typedef struct {
	char *p1;
	char *p2;
	int pi;
} mydata;

static
void render_data(const char *m, void *arg)
{
	mydata *item = (void *)(mydata *)arg;

	assert((strcmp(m, "TOP") == 0) ||
		(strcmp(m, "MIDDLE") == 0) ||
		(strcmp(m, "BOTTOM") == 0));
	assert(strcmp(item->p1, "param1") == 0);
	assert(strcmp(item->p2, "param2") == 0);
	assert(item->pi == 10);
}

int
main(void)
{
	struct cez_render render;
	struct cez_render_entry *entry;

	mydata testdata;
	testdata.p1 = "param1";
	testdata.p2 = "param2";
	testdata.pi = 10;

	cez_test_start();

	cez_render_init(&render);
	assert(cez_render_get(&render, "macro1") == NULL);
	assert(cez_render_add(&render, "macro1", "RENDER.template",
	    (mydata *)render_data) == 0);
	assert((entry = cez_render_get(&render, "macro1")) != NULL);
	cez_render_call(entry->filepath, entry->render_cb, (void *)&testdata);
	assert(cez_render_remove(&render, "macro1") == 0);
	assert(cez_render_remove(&render, "notexist") == -1);
	assert(cez_render_get(&render, "macro1") == NULL);
	cez_render_purge(&render);

	return (0);
}
