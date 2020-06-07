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

void
http_request_timeout_set(struct http_request *request, long timeout)
{
    if (timeout > 0)
        request->timeout = timeout;
}

char *
http_request_state_text(long code)
{
    if (code < 0)
        return "UNKNOWN";

    static struct _RequestState {
        unsigned long state;
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

    while (msg->state <= code)
        if (msg->state == code)
	    return(msg->msg);
	else
	    msg++;

    return "UNKNOWN";
}

static BOOL
http_request_url_parse(struct http_request *request)
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
        request->state = HTTP_REQUEST_URL_INVALID;
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
	    request->state = HTTP_REQUEST_PORT_INVALID;
	}
    }

    return (T);
}

static void
http_request_header_start(struct http_request *request)
{
    struct buffer *buffer = request->hdrs;

    bprintf(buffer, "GET /%s HTTP/1.0" CRLF, request->url_path);
    bprintf(buffer, "User-Agent: %s" CRLF, request->user_agent);
    bprintf(buffer, "Host: %s" CRLF, request->url_host);
}

void
http_request_header_add(struct http_request *request, const char *key,
    const char *value)
{
    struct buffer *buffer = request->hdrs;

    bprintf(buffer, "%s: %s" CRLF, key, value);
}

static void
http_request_header_end(struct http_request *request)
{
    struct buffer *buffer = request->hdrs;

    bputs(buffer, "" CRLF);
}

void
http_request_header_dump(struct http_request *request)
{
    struct buffer *buffer = request->hdrs;
    int c;

    buffer_rewind(buffer);
    while ((c = bgetc(buffer)) != EOF)
        putc(c, stdout);
}

struct http_request *
http_request_create(const char *url, const char *user_agent)
{
    struct pool *pool = pool_create(HTTP_REQUEST_BLOCK_SIZE);
    struct http_request *request = pool_alloc(pool, sizeof(struct http_request));

    /* Make sure cleared out */
    memset(request, 0, sizeof(struct http_request));

    /* Common */
    request->pool = pool;
    request->stream = NIL;
    request->fd = 0;

    request->hdrs = buffer_create(request->pool, HTTP_REQUEST_HDR_BLOCK_SIZE);

    request->url = pool_strdup(request->pool, url);
    request->url_host = NIL;
    request->url_port = 0;
    request->url_path = NIL;
    if (user_agent == NULL)
        request->user_agent = pool_strdup(request->pool, "cezhttp");
    else
        request->user_agent = pool_strdup(request->pool, user_agent);

    request->timeout = HTTP_REQUEST_TIMEOUT;
    request->state = HTTP_REQUEST_OK;
    request->status = 501;	/* Not implemented */
    request->isssl = NIL;

    request->xerrno = 0;

    http_request_url_parse(request);
    http_request_header_start(request);

    return (request);
}

BOOL
http_request_send(struct http_request *request)
{
    struct buffer *buffer;
    struct iostream *stream;
    int c;

    if ((request->fd =
         os_connect_inet_socket(request->url_host, request->url_port)) == -1) {
        request->state = HTTP_REQUEST_CONNECTION_ERROR;
	request->xerrno = errno;
	return (NIL);
    }
    stream = request->stream = iostream_create(request->pool, request->fd, 0);
    iostream_set_timeout(stream, request->timeout);
    if (request->isssl) {
        ssl_client_context_init();
	iostream_ssl_start_client(stream);
    }

    http_request_header_end(request);

    buffer = request->hdrs;
    buffer_rewind(buffer);
    while ((c = bgetc(buffer)) != EOF)
        ioputc(c, stream);

    ioflush(stream);
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

void
http_response_header_size_set(struct http_response *response, long size)
{
    if (size > 0)
        response->hdrs_max_size = size;
}

void
http_response_body_size_set(struct http_response *response, long size)
{
    if (size > 0)
        response->body_max_size = size;
}

void
http_response_status_size_set(struct http_response *response, long size)
{
    if (size > 0)
        response->status_max_size = size;
}

struct http_response *
http_response_create(struct http_request *request)
{
    struct pool *pool = pool_create(HTTP_RESPONSE_BLOCK_SIZE);
    struct http_response *response = pool_alloc(pool, sizeof(struct http_response));

    /* Make sure cleared out */
    memset(response, 0, sizeof(struct http_response));

    /* Common */
    response->pool = pool;
    response->stream = request->stream;

    /* Input buffer */
    response->read_buffer = buffer_create(pool, PREFERRED_BUFFER_BLOCK_SIZE);
    response->state = HTTP_RESPONSE_INIT;
    response->hdrs_offset = 0;
    response->hdrs_size = 0;
    response->hdrs_max_size = HTTP_RESPONSE_HEADER_MAX_SIZE;
    response->hdrs_crlfs = 1;
    response->body_offset = 0;
    response->body_current = 0;
    response->body_size = 0;
    response->body_max_size = HTTP_RESPONSE_BODY_MAX_SIZE;

    response->preserve = NIL;
    response->iseof = NIL;
    response->error = NIL;

    response->hdrs = assoc_create(pool, 16, T);

    response->status = 501;	/* Not implemented */
    response->status_size = 0;
    response->status_max_size = HTTP_RESPONSE_STATUS_MAX_SIZE;

    return (response);
}

void
http_response_free(struct http_response *response)
{
    pool_free(response->pool);
}

static BOOL
http_response_status_parse(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *buffer = response->read_buffer;
    int c;
    char *token;
    char *status;
    unsigned long count = response->status_size;
    unsigned long maxsize = response->status_max_size;

    /* Skip over leading whitespace */
    while (((c = iogetc(stream)) == '\015') || (c == '\012'));

    /* Fetch and record characters until end of line */
    while (c != EOF) {
        bputc(buffer, c);
	if ((maxsize > 0) && (++count >= maxsize)) {
	    response->status = 413;      /* Request Entity too large */
	    return (NIL);
	}
	if (c == '\012')
	    break;
	c = iogetc(stream);
    }

    response->status_size = buffer_size(buffer);

    if (c == EOF) {
        /* Record permanent end of file */
	response->iseof = T;
	return (NIL);
    }

    /* Status line is now complete: record and then parse */
    if (response->status_size > 0) {
        status = buffer_fetch(buffer, 0, response->status_size - 1,
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
http_response_header_parse_init(struct http_response *response)
{
    struct buffer *buffer = response->read_buffer;

    response->state = HTTP_RESPONSE_HDRS;
    response->hdrs_offset = buffer_size(buffer);
    response->hdrs_size = 0;
    response->hdrs_crlfs = 1;

    return (T);
}

static BOOL
http_response_header_process(struct http_response *response, char *data)
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
http_response_header_parse(struct http_response *response)
{
    struct buffer *buffer = response->read_buffer;
    struct iostream *stream = response->stream;
    unsigned long crlf_count = response->hdrs_crlfs;
    char *data;
    int c = EOF;
    unsigned long count = response->hdrs_size;
    unsigned long maxsize = response->hdrs_max_size;

    /* Record hdrs location first time into loop */
    if (response->hdrs_offset == 0)
        response->hdrs_offset = buffer_size(buffer);

    /* Read in data until end of header block located (CRLFCRLF or just LFLF) */

    while ((crlf_count < 2) && ((c = iogetc(stream)) != EOF)) {
        bputc(buffer, c);
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
    data = buffer_fetch(buffer, response->hdrs_offset, response->hdrs_size,
                        response->preserve);

    http_response_header_process(response, data);
    return (T);
}

void
http_response_header_dump(struct http_response *response)
{
    struct assoc *hdrs = response->hdrs;
    char *key;

    assoc_scan_reset(hdrs);

    while (assoc_scan_next(hdrs, &key, NIL)) {
        printf("key: %s, value: %s\n", key, assoc_lookup(hdrs, key));
    }
}

static BOOL
http_response_body_parse_init(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *buffer = response->read_buffer;
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
	response->state = HTTP_RESPONSE_BODY;
	response->body_offset = buffer_size(buffer);
	response->body_current = 0L;
	response->body_size = atoi(value);
	return (T);
    }

    /* Response has no body */
    response->state = HTTP_RESPONSE_COMPLETE;
    return (T);
}

static BOOL
http_response_body_parse(struct http_response *response)
{
    struct iostream *stream = response->stream;
    struct buffer *buffer = response->read_buffer;
    unsigned long current = response->body_current;
    unsigned long size = response->body_size;
    int c = EOF;

    /* Record body location and size */
    if (response->body_offset == 0)
        response->body_offset = buffer_size(buffer);

    while ((current < size) && ((c = iogetc(stream)) != EOF)) {
        bputc(buffer, c);
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
http_response_body_print(struct http_response *response)
{
    return (buffer_fetch(response->read_buffer, response->body_offset,
                        response->body_size, 0));
}

BOOL
http_response_parse(struct http_response *response)
{
    while (response->state != HTTP_RESPONSE_COMPLETE) {
        switch (response->state) {
	case HTTP_RESPONSE_INIT:
	    http_response_status_parse(response);
            http_response_header_parse_init(response);
	    break;

	case HTTP_RESPONSE_HDRS:
	    if (http_response_header_parse(response) == NIL)
	        return NIL;
	    if (http_response_body_parse_init(response) == NIL)
	        return NIL;
	    break;

	case HTTP_RESPONSE_BODY:
	    if (http_response_body_parse(response) == NIL)
	        return NIL;
	    response->state = HTTP_RESPONSE_COMPLETE;
	    break;

	case HTTP_RESPONSE_COMPLETE:
	    break;
	}
    }

    response->state = HTTP_RESPONSE_COMPLETE;
    return (T);
}
