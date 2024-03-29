/*
 * Copyright (c) 2020-2021 Nikola Kolev <koue@chaosophia.net>
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

#include "cez_cgi.h"
#include "cez_test.h"

static char *test_head = "HEAD";
static char *test_query_string = "a=show&b=this-is-it&c=3";

void
set_context(void)
{
	setenv("REQUEST_METHOD", test_head, 1);
	setenv("QUERY_STRING", test_query_string, 1);
}

int
main(void)
{
	struct cez_cgi *cgi = cez_cgi_create();

	cez_test_start();

	assert((cgi = cez_cgi_create()) != NULL);
	assert(cgi->request_method == NULL);
	assert(cgi->query_string == NULL);
	cez_cgi_free(cgi);

	set_context();
	assert((cgi = cez_cgi_create()) != NULL);
	assert(strcmp(cgi->request_method, test_head) == 0);
	assert(strcmp(cgi->query_string, test_query_string) == 0);
	assert(strcmp(cez_queue_get(&cgi->query_queue, "a"), "show") == 0);
	assert(strcmp(cez_queue_get(&cgi->query_queue, "a"), "nottrue") != 0);
	assert(strcmp(cez_queue_get(&cgi->query_queue, "b"), "this-is-it") == 0);
	assert(strcmp(cez_queue_get(&cgi->query_queue, "c"), "3") == 0);
	assert(strcmp(cez_queue_get(&cgi->query_queue, "c"), "0123") != -1);
	assert(cez_queue_get(&cgi->query_queue, "notexist") == NULL);

	cez_cgi_free(cgi);

	return (0);
}
