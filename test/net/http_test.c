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
#include <string.h>

#include "cez_net.h"
#include "cez_net_http.h"

#include "cez_test.h"

int
main(void)
{
    struct http_request *request;
    struct http_response *response;

    cez_test_start();

    // Valid HTTP URL
    assert((request = http_request_create("http://chaosophia.net/")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(strcmp(request->url_path, "") == 0);
    assert(request->status == REQUEST_OK);
    http_request_free(request);

    assert((request = http_request_create("http://chaosophia.net:8080/my.html")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == 8080);
    assert(strcmp(request->url_path, "my.html") == 0);
    assert(request->status == REQUEST_OK);
    http_request_free(request);

    // Valid HTTPS URL
    assert((request = http_request_create("https://chaosophia.net/")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == HTTPS_PORT);
    assert(strlen(request->url_path) == 0);
    assert(request->status == REQUEST_OK);
    http_request_free(request);

    assert((request = http_request_create("HTTPS://chaosophia.net:8443/1/2.html")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == 8443);
    assert(strcmp(request->url_path, "1/2.html") == 0);
    assert(request->status == REQUEST_OK);
    http_request_free(request);

    // Invalid HTTP URL
    assert((request = http_request_create("xhttp://chaosophia.net")) != NULL);
    assert(request->status == REQUEST_INVALID_URL);
    http_request_free(request);

    assert((request = http_request_create("http://chaosophia.net::")) != NULL);
    assert(request->status == REQUEST_INVALID_PORT);
    http_request_free(request);

    // Connection refused
    assert((request = http_request_create("http://koue.chaosophia.net/index.html")) != NULL);
    assert(strcmp(request->url_host, "koue.chaosophia.net") == 0);
    assert(strcmp(request->url_path, "index.html") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(request->status == REQUEST_OK);
    assert(http_request_send(request) == NIL);
    assert(request->status == REQUEST_CONNECTION_ERROR);
    assert(strcmp(http_request_status_text(request->status), "Connection refused"));
    http_request_free(request);

    // Response 200
    assert((request = http_request_create("http://chaosophia.net/index.html")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(strcmp(request->url_path, "index.html") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(request->status == REQUEST_OK);
    assert(http_request_send(request) == T);
    assert(request->status == REQUEST_OK);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 200);
    assert(strcmp(http_response_print_body(response), "</html>"));
    http_response_free(response);
    http_request_free(request);

    // Response 404
    assert((request = http_request_create("http://chaosophia.net/notexistzaq.html")) != NULL);
    assert(http_request_send(request) == T);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 404);
    http_response_free(response);
    http_request_free(request);

    return (0);
}
