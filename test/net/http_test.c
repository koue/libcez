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
#include <sys/time.h>

#include "cez_net.h"
#include "cez_net_http.h"

#include "cez_test.h"

int
main(void)
{
    struct http_request *request, *rq1, *rq2;
    struct http_response *response, *rs1, *rs2;

    time_t now = time(NULL);
    char timestr[32];

    cez_test_start();

    // Valid HTTP URL
    assert((request = http_request_create("http://chaosophia.net/", "myagent")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(strcmp(request->url_path, "") == 0);
    assert(request->state == HTTP_REQUEST_OK);
    http_request_free(request);

    assert((request = http_request_create("http://chaosophia.net:8080/my.html", "myagent")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == 8080);
    assert(strcmp(request->url_path, "my.html") == 0);
    assert(request->state == HTTP_REQUEST_OK);
    http_request_free(request);

    // Valid HTTPS URL
    assert((request = http_request_create("https://chaosophia.net/", "myagent")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == HTTPS_PORT);
    assert(strlen(request->url_path) == 0);
    assert(request->state == HTTP_REQUEST_OK);
    http_request_free(request);

    assert((request = http_request_create("HTTPS://chaosophia.net:8443/1/2.html", "myagent")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(request->url_port == 8443);
    assert(strcmp(request->url_path, "1/2.html") == 0);
    assert(request->state == HTTP_REQUEST_OK);
    http_request_free(request);

    // Invalid HTTP URL
    assert((request = http_request_create("xhttp://chaosophia.net", "myagent")) != NULL);
    assert(request->state == HTTP_REQUEST_URL_INVALID);
    assert(strcmp(http_request_state_text(request->state), "Invalid URL") == 0);
    http_request_free(request);

    assert((request = http_request_create("http://chaosophia.net::", "myagent")) != NULL);
    assert(request->state == HTTP_REQUEST_PORT_INVALID);
    assert(strcmp(http_request_state_text(request->state), "Invalid port") == 0);
    http_request_free(request);

    // Connection refused
    assert((request = http_request_create("http://koue.chaosophia.net/index.html", "myagent")) != NULL);
    assert(strcmp(request->url_host, "koue.chaosophia.net") == 0);
    assert(strcmp(request->url_path, "index.html") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(request->state == HTTP_REQUEST_OK);
    assert(http_request_send(request) == NIL);
    assert(request->state == HTTP_REQUEST_CONNECTION_ERROR);
    http_request_free(request);

    // Response 200
    assert((request = http_request_create("http://chaosophia.net/index.html", "myagent")) != NULL);
    assert(strcmp(request->url_host, "chaosophia.net") == 0);
    assert(strcmp(request->url_path, "index.html") == 0);
    assert(request->url_port == HTTP_PORT);
    assert(request->state == HTTP_REQUEST_OK);
    assert(http_request_send(request) == T);
    assert(request->state == HTTP_REQUEST_OK);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 200);
    assert(strcmp(http_response_body_print(response), "</html>"));
    http_response_free(response);
    http_request_free(request);

    // Response 304
    strftime(timestr, sizeof(timestr), "%a, %d %b %Y %T %Z", localtime(&now));
    assert((request = http_request_create("http://chaosophia.net/index.html", "myagent")) != NULL);
    http_request_header_add(request, "If-Modified-Since", timestr);
    assert(http_request_send(request) == T);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 304);
    http_response_free(response);
    http_request_free(request);

    // Response 404
    assert((request = http_request_create("http://chaosophia.net/notexistzaq.html", "myagent")) != NULL);
    assert(http_request_send(request) == T);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 404);
    http_response_free(response);
    http_request_free(request);

    // HTTPS Response 200
    // 1 - single connection
    assert((request = http_request_create("https://www.openbsd.org/index.html", "myagent")) != NULL);
    assert(strcmp(request->url_host, "www.openbsd.org") == 0);
    assert(strcmp(request->url_path, "index.html") == 0);
    assert(request->url_port == HTTPS_PORT);
    assert(request->state == HTTP_REQUEST_OK);
    assert(http_request_send(request) == T);
    assert(request->state == HTTP_REQUEST_OK);
    assert((response = http_response_create(request)) != NULL);
    assert(http_response_parse(response) == T);
    assert(response->status == 200);
    assert(strcmp(http_response_body_print(response), "</footer>"));
    http_response_free(response);
    http_request_free(request);

    // 2 - two parallel connections
    assert((rq1 = http_request_create("https://raw.githubusercontent.com/koue/rssroll/develop/tests/atom.xml", "myagent")) != NULL);
    assert((rq2 = http_request_create("https://raw.githubusercontent.com:443/koue/libcez/develop/README.md", "myagent")) != NULL);
    assert(strcmp(rq1->url_host, "raw.githubusercontent.com") == 0);
    assert(strcmp(rq2->url_host, "raw.githubusercontent.com") == 0);
    assert(rq1->url_port == HTTPS_PORT);
    assert(rq2->url_port == HTTPS_PORT);
    assert(rq1->state == HTTP_REQUEST_OK);
    assert(rq2->state == HTTP_REQUEST_OK);
    assert(http_request_send(rq1) == T);
    assert(http_request_send(rq2) == T);
    assert(rq1->state == HTTP_REQUEST_OK);
    assert(rq2->state == HTTP_REQUEST_OK);
    assert((rs1 = http_response_create(rq1)) != NULL);
    assert((rs2 = http_response_create(rq2)) != NULL);
    assert(http_response_parse(rs1) == T);
    assert(http_response_parse(rs2) == T);
    assert(rs1->status == 200);
    assert(rs2->status == 200);
    assert(strcmp(http_response_body_print(rs1), "</feed>"));
    assert(strcmp(http_response_body_print(rs2), "# libcez"));
    http_response_free(rs1);
    http_response_free(rs2);
    http_request_free(rq1);
    http_request_free(rq2);

    return (0);
}
