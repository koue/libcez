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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "cez_cgi.h"
#include "cez_util.h"

void
cez_cgi_free(struct cez_cgi *cgi)
{
	cez_queue_purge(&cgi->query_queue);
	pool_free(cgi->pool);
}

static int
query_string_cb(const char *name, const char *value, void *arg)
{
	return (cez_queue_add(arg, name, value));
}

struct cez_cgi *
cez_cgi_create(void)
{
	struct pool *pool = pool_create(1024);
	struct cez_cgi *cgi = pool_alloc(pool, sizeof(struct cez_cgi));

	memset(cgi, 0, sizeof(struct cez_cgi));
	cgi->pool = pool;

	cgi->http_host = getenv("HTTP_HOST");
	cgi->https = getenv("HTTPS");
	cgi->path_info = getenv("PATH_INFO");
	cgi->query_string = getenv("QUERY_STRING");
	cgi->request_method = getenv("REQUEST_METHOD");
	cgi->script_name = getenv("SCRIPT_NAME");
	cgi->server_name = getenv("SERVER_NAME");
	cgi->server_port = getenv("SERVER_PORT");
	cgi->http_cookie = getenv("HTTP_COOKIE");
	cgi->http_referer = getenv("HTTP_REFERER");
	cgi->content_length = getenv("CONTENT_LENGTH") ? strtoul(getenv("CONTENT_LENGTH"), NULL, 10) : 0;

	if (cgi->query_string != NULL) {
		cez_queue_init(&cgi->query_queue);
		if (cez_util_param_list(getenv("QUERY_STRING"), '&', query_string_cb,
		    (void *)&cgi->query_queue) == -1) {
			fprintf(stderr, "%s: Fail to initialize 'query_queue'\n", __func__);
			cez_queue_purge(&cgi->query_queue);
			pool_free(pool);
			return (NULL);
		}
	}

	return (cgi);
}
