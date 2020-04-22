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
#include <stdio.h>

#include "cez_test.h"
#include "cez_net_http.h"
#include "cez_prayer.h"

void
http_connect(struct uri *uri)
{
	uri->fd = os_connect_inet_socket(uri->url_host, uri->url_port);
}

int
main(void)
{
	char *s = "http://chaosophia.net/papers/luks_tpm_and_full_disk_encryption_without_password.html";
	struct uri *uri;

	cez_test_start();

	assert(http_setuseragent("htf 0.1") != -1);
	assert((uri = http_add(HTTP_REQUEST_GET, s)) != NULL);
	http_connect(uri);
	http_fetch(uri);
	http_readheader(uri);
	http_readbody(uri);
//	assert(strcmp(uri->body, "</html>") == 0);
	printf("%s\n", uri->body);
	uri_free(uri, 0);

	return (0);
}
