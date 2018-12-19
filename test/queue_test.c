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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cez_queue.h"
#include "cez_test.h"

int
main(void)
{
	struct cez_queue cgi, config;
	static const char *params[] = { "name1", "name2", NULL };
	static char *list = "p1=v1;p2=v2;p3=";

	cez_test_start();
	cez_queue_init(&cgi);
	cez_queue_init(&config);
	assert(cez_queue_get(&cgi, "name1") == NULL);
	assert(cez_queue_get(&config, "param4") == NULL);
	assert(cez_queue_add(&cgi, "name1", "black sheep wall") == 0);
	assert(cez_queue_add(&cgi, "name2", "power overwhelming") == 0);
	assert(configfile_parse("./configrc", &config) == 0);
	assert(cez_queue_check(&cgi, params) == NULL);
	assert(cez_queue_check(&config, params) != NULL);
	assert(cez_queue_add_list(&cgi, list, ';') == 0);
	assert(strcmp(cez_queue_get(&cgi, "name1"), "black sheep wall") == 0);
	assert(strcmp(cez_queue_get(&cgi, "p2"), "v2") == 0);
	assert(strlen(cez_queue_get(&cgi, "p3")) == 0);
	assert(cez_queue_get(&config, "param3") == NULL);
	assert(strcmp(cez_queue_get(&config, "param4"), "power 'overwhelming'") == 0);
	assert(cez_queue_update(&config, "param4", "my new value") == 0);
	assert(strcmp(cez_queue_get(&config, "param4"), "my new value") == 0);
	assert(cez_queue_remove(&config, "param4") == 0);
	assert(cez_queue_get(&config, "param4") == NULL);
	cez_queue_purge(&cgi);
	cez_queue_purge(&config);

	return (0);
}
