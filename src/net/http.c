/*
 * Copyright 2020 Nikola Kolev <koue@chaosophia.net>
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

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <time.h>

#include "cez_core_pool.h"
#include "cez_core_assoc.h"
#include "cez_core_buffer.h"
#include "cez_core_string.h"

#include "cez_net.h"
#include "cez_net_http.h"

char *
http_request_status_text(unsigned long code)
{
    static struct _RequestStatus {
        unsigned long status;
	char *msg;
    } *msg, msgs[] = {
        {
	1, "OK"}
	, {
	2, "Invalid URL"}
	, {
	3, "Invalid port"}
	, {
        000, NULL}
    };

    msg = msgs;

    while (msg->status <= code)
        if (msg->status == code)
	    return(msg->msg);
	else
	    msg++;

    return "UNKNOWN";
}

static BOOL
http_request_parse_url(struct http_request *request)
{
    char *p;

    if (strncasecmp(request->url, HTTPS_PREFIX, strlen(HTTPS_PREFIX)) == 0) {
        request->isssl = T;
	request->url_port = HTTPS_PORT;
        request->url += strlen(HTTPS_PREFIX);
    } else if (strncasecmp(request->url, HTTP_PREFIX, strlen(HTTP_PREFIX)) == 0) {
        request->isssl = NIL;
	request->url_port = HTTP_PORT;
        request->url += strlen(HTTP_PREFIX);
    } else {
        request->status = REQUEST_INVALID_URL;
	return (NIL);
    }
    // TODO: validation?
    request->url_host = pool_strdup(request->pool, request->url);
    p = strchr(request->url_host, '/');
    if (p != NULL) {
        *p = '\0';
	request->url_path = pool_strdup(request->pool, p + 1);
	if ((p = strchr(request->url_path, '#')) != NULL) {
	    *p = '\0';
	}
    } else {
        request->url_path = NIL;
    }
    p = strchr(request->url_host, ':');
    if (p != NULL) {
        *p = '\0';
	request->url_port = atoi(p + 1);
	if (request->url_port == 0) {
	    request->status = REQUEST_INVALID_PORT;
	}
    }

    return (T);
}

struct http_request *
http_request_create(char *url)
{
    struct pool *pool = pool_create(HTTP_REQUEST_BLOCK_SIZE);
    struct http_request *request = pool_alloc(pool, sizeof(struct http_request));

    /* Make sure cleared out */
    memset(request, 0, sizeof(struct http_request));

    /* Common */
    request->pool = pool;
    request->stream = NIL;
    request->fd = 0;

    request->url = pool_strdup(request->pool, url);
    request->url_host = NIL;
    request->url_port = 0;
    request->url_path = NIL;

    request->timeout = 10;
    request->status = REQUEST_OK;
    request->isssl = NIL;

    request->xerrno = 0;

    http_request_parse_url(request);

    return (request);
}

BOOL
http_request_send(struct http_request *request)
{
    if ((request->fd =
         os_connect_inet_socket(request->url_host, request->url_port)) == -1) {
        request->status = REQUEST_CONNECTION_ERROR;
	request->xerrno = errno;
	return (NIL);
    }
    request->stream = iostream_create(request->pool, request->fd, 0);
    iostream_set_timeout(request->stream, request->timeout);
    if (request->isssl) {
        ssl_client_context_init();
	iostream_ssl_start_client(request->stream);
    }
    ioprintf(request->stream, "GET /%s HTTP/1.0\r\n"
                              "User-Agent: cezhttp\r\n"
			      "Host: %s\r\n\r\n",
			      request->url_path, request->url_host);
    ioflush(request->stream);
    return (T);
}

void
http_request_free(struct http_request *request)
{
    if (request->stream)
        iostream_close(request->stream);
    if (request->fd > 0)
        close(request->fd);
    pool_free(request->pool);
}

struct http_response *
http_response_create(struct http_request *request)
{
    struct pool *p = pool_create(HTTP_RESPONSE_BLOCK_SIZE);
    struct http_response *response = pool_alloc(p, sizeof(struct http_response));

    /* Make sure cleared out */
    memset(response, 0, sizeof(struct http_response));

    /* Common */
    response->pool = p;
    response->stream = request->stream;

    /* Input buffer */
    response->read_buffer = buffer_create(p, PREFERRED_BUFFER_BLOCK_SIZE);
    response->state = RESPONSE_INIT;
    response->hdrs_offset = 0;
    response->hdrs_size = 0;
    response->hdrs_max_size = 8048;
    response->hdrs_crlfs = 1;
    response->body_offset = 0;
    response->body_current = 0;
    response->body_size = 0;
    response->body_max_size = 8048;

    response->preserve = NIL;
    response->iseof = NIL;
    response->error = NIL;

    response->hdrs = assoc_create(p, 16, T);

    response->status = 501;	/* Not implemented */
    response->status_size = 0;
    response->status_max_size = 64;

    return (response);
}

void
http_response_free(struct http_response *response)
{
    pool_free(response->pool);
}

static BOOL
http_response_parse_status(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *b = response->read_buffer;
    int c;
    char *token;
    char *status;
    unsigned long count = response->status_size;
    unsigned long maxsize = response->status_max_size;

    /* Skip over leading whitespace */
    while (((c = iogetc(stream)) == '\015') || (c == '\012'));

    /* Fetch and record characters until end of line */
    while (c != EOF) {
        bputc(b, c);
	if ((maxsize > 0) && (++count >= maxsize)) {
	    response->status = 413;      /* Request Entity too large */
	    return (NIL);
	}
	if (c == '\012')
	    break;
	c = iogetc(stream);
    }

    response->status_size = buffer_size(b);

    if (c == EOF) {
        /* Record permanent end of file */
	response->iseof = T;
	return (NIL);
    }

    /* Status line is now complete: record and then parse */
    if (response->status_size > 0) {
        status = buffer_fetch(b, 0, response->status_size - 1,
	                      response->preserve);
    } else {
        /* Bad response */
	response->status = 400;
	return (NIL);
    }

    if ((token = string_get_token(&status)) == NIL) {
        /* Bad response */
	response->status = 400;
	return (NIL);
    }

    if (strncmp(token, "HTTP", 4) != 0) {
        /* Bad response */
	response->status = 400;
	return (NIL);
    }

    if ((response->status = atoi(string_get_token(&status))) == NIL) {
        /* Bad response */
	response->status = 400;
	return (NIL);
    }

    return (T);
}

static BOOL
http_response_parse_headers_init(struct http_response *response)
{
    struct buffer *b = response->read_buffer;

    response->state = RESPONSE_HDRS;
    response->hdrs_offset = buffer_size(b);
    response->hdrs_size = 0;
    response->hdrs_crlfs = 1;

    return (T);
}

static BOOL
http_response_process_headers(struct http_response *response, char *data)
{
    char *header, *key, *oldvalue, *value, *s;

    while ((header = string_get_lws_line(&data, T))) {
        /* Fetch one (possibly folded) header line at a time */
	if (header[0] == '\0')
	    continue;

	if (!((key = string_get_token(&header)) && ((value = string_next_token(&header))))) {
            /* Bad request */
	    response->status = 400;
	    return (NIL);
	}

	/* Convert string to lower case */
	for (s = key; *s; s++)
	    *s = tolower(*s);

	if ((s == key) || (s[-1] != ':')) {
	    /* Bad request */
	    response->status = 400;
	    return (NIL);
	}

	s[-1] = '\0';

	if ((oldvalue = assoc_lookup(response->hdrs, key))) {
	    s = pool_alloc(response->hdrs->pool,
	                   strlen(value) + strlen(oldvalue) + 3);

            strcpy(s, oldvalue);        /* Should be able to improve this */
	    strcat(s, ", ");
	    strcat(s, value);
	    value = s;
	}
	/* Generate assoc entry. Don't need to copy key and value */
	assoc_update(response->hdrs, key, value, NIL);
    }
    return (T);
}

static BOOL
http_response_parse_headers(struct http_response *response)
{
    struct buffer *b = response->read_buffer;
    struct iostream *stream = response->stream;
    unsigned long crlf_count = response->hdrs_crlfs;
    char *data;
    int c = EOF;
    unsigned long count = response->hdrs_size;
    unsigned long maxsize = response->hdrs_max_size;

    /* Record hdrs location first time into loop */
    if (response->hdrs_offset == 0)
        response->hdrs_offset = buffer_size(b);

    /* Read in data until end of header block located (CRLFCRLF or just LFLF) */

    while ((crlf_count < 2) && ((c = iogetc(stream)) != EOF)) {
        bputc(b, c);
	if ((maxsize > 0) && (++count >= maxsize)) {
	    response->status = 413;      /* Request Entity too large */
	    return (NIL);
	}

	if (c == '\012')
	    crlf_count++;
	else if (c != '\015')
            crlf_count = 0;
    }
    response->hdrs_size = count;
    response->hdrs_crlfs = crlf_count;

    /* Hdrs now complete */

    /* Extract copy of entire header block from buffer */
    data = buffer_fetch(b, response->hdrs_offset, response->hdrs_size,
                        response->preserve);

    http_response_process_headers(response, data);
    return (T);
}

static BOOL
http_response_parse_body_init(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *b = response->read_buffer;
    char *value;
    unsigned long len;

    if ((value = assoc_lookup(response->hdrs, "content-length"))) {
        if (((len = atoi(value)) > response->body_max_size)) {
	    /* Eat the body */
	    if (len < (5 * response->body_max_size)) {
	        while ((len > 0) && (iogetc(stream) != EOF))
		    len--;
	    }

            response->status = 413; 	/* Response too large 	*/
	    return (NIL);
	}
	response->state = RESPONSE_BODY;
	response->body_offset = buffer_size(b);
	response->body_current = 0L;
	response->body_size = atoi(value);
	return (T);
    }

    /* Response has no body */
    response->state = RESPONSE_COMPLETE;
    return (T);
}

static BOOL
http_response_parse_body(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *b = response->read_buffer;
    unsigned long current = response->body_current;
    unsigned long size = response->body_size;
    int c = EOF;

    /* Record body location and size */
    if (response->body_offset == 0)
        response->body_offset = buffer_size(b);

    while ((current < size) && ((c = iogetc(stream)) != EOF)) {
        bputc(b, c);
	current++;
    }
    response->body_current = current;

    if (current < size) {
        response->status = 400; /* Insufficient data */
	return (NIL);
    }

    return (T);
}

char *
http_response_print_body(struct http_response *response)
{
    return (buffer_fetch(response->read_buffer, response->body_offset,
                        response->body_size, 0));
}

BOOL
http_response_parse(struct http_response *response)
{
    while (response->state != RESPONSE_COMPLETE) {
        switch (response->state) {
	case RESPONSE_INIT:
	    http_response_parse_status(response);
            http_response_parse_headers_init(response);
	    break;

	case RESPONSE_HDRS:
	    if (http_response_parse_headers(response) == NIL)
	        return NIL;
	    if (http_response_parse_body_init(response) == NIL)
	        return NIL;
	    break;

	case RESPONSE_BODY:
	    if (http_response_parse_body(response) == NIL)
	        return NIL;
	    response->state = RESPONSE_COMPLETE;
	    break;

	case RESPONSE_COMPLETE:
	    break;
	}
    }

    response->state = RESPONSE_COMPLETE;
    return (T);
}
