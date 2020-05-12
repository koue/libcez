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

#include <stdarg.h>

#include "cez_core_pool.h"
#include "cez_net.h"
#include "cez_test.h"

#define POOLSIZE 131072

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HTTP_RESPONSE 65536

#define IO_TIMEOUT 10

#define UA "libcez 0.1"

static char request[1024];

void
set_http_request(const char *host)
{
	snprintf(request, sizeof(request),
	    "GET / HTTP/1.0\r\n"
	    "User-Agent: %s\r\n"
	    "Host: %s\r\n\r\n", UA, host);
}

void
test_os_connect_inet_socket_http(struct pool *p, struct buffer *b)
{
	int c, count = 1, sockfd;
	char *host = "chaosophia.net";
	struct iostream *stream;

	assert(sockfd = os_connect_inet_socket(host, HTTP_PORT));
	assert(stream = iostream_create(p, sockfd, 0));

	iostream_set_timeout(stream, IO_TIMEOUT);

	set_http_request(host);

	ioprintf(stream, request);
	ioflush(stream);

	while (((c = iogetc(stream)) != EOF) && (++count < HTTP_RESPONSE))
		bputc(b, c);

	assert(buffer_size(b));
	assert(strstr(buffer_fetch(b, 0, buffer_size(b), NIL), "</html>") != NULL);

	iostream_close(stream);
	close(sockfd);
	buffer_reset(b);
}

#ifdef SSL
void
test_os_connect_inet_socket_https(struct pool *p, struct buffer *b)
{
	int c, count = 1, sockfd;
	char *host = "www.openbsd.org";
	struct iostream *stream;

	assert(sockfd = os_connect_inet_socket(host, HTTPS_PORT));
	assert(stream = iostream_create(p, sockfd, 0));

	iostream_set_timeout(stream, IO_TIMEOUT);

	ssl_client_context_init();
	iostream_ssl_start_client(stream);

	set_http_request(host);

	ioprintf(stream, request);
	ioflush(stream);

	while (((c = iogetc(stream)) != EOF) && (++count < HTTP_RESPONSE))
		bputc(b, c);

	assert(buffer_size(b));

	assert(strstr(buffer_fetch(b, 0, buffer_size(b), NIL), "</footer>") != NULL);

	iostream_close(stream);
	close(sockfd);
	buffer_reset(b);
}
#endif

int
main(void)
{
	struct pool *pool = pool_create(POOLSIZE);
	struct buffer *b = buffer_create(pool, 0L);

	cez_test_start();

	test_os_connect_inet_socket_http(pool, b);
#ifdef SSL
	test_os_connect_inet_socket_https(pool, b);
#endif

	pool_free(pool);

	return (0);
}
