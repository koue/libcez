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

#include "cez_core_pool.h"
#include "cez_core_assoc.h"
#include "cez_core_buffer.h"
#include "cez_core_string.h"

#include "cez_net.h"
#include "cez_net_http.h"

#define HTTP_RESPONSE 65536

void
http_connect(struct http_response *response)
{
    struct buffer *b = response->read_buffer;
    struct iostream *stream;
    int c, count = 1, fd;
    char request[1024];

    snprintf(request, sizeof(request),
                  "GET /index.html HTTP/1.0\r\n"
		  "hresp"
		  "Host: chaosophia.net\r\n\r\n");

    fd = os_connect_inet_socket("chaosophia.net", 80);
    printf("fd %d\n", fd);
    stream = iostream_create(response->pool, fd, 0);

    iostream_set_timeout(stream, 10);

    ioprintf(stream, request);
    ioflush(stream);

    while (((c = iogetc(stream)) != EOF) && (++count < HTTP_RESPONSE)) {
      //  printf("%c", c);
        bputc(b, c);
    }
    printf("count %d\n", count);
    iostream_close(stream);
    close(fd);
}

int
main(void)
{
    struct http_response *response;

    response = http_response_create();
    http_connect(response);
    http_response_parse(response);
    http_response_free(response);

    return (0);
}
