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

#ifndef _CEZ_NET_HTTP_H
#define _CEZ_NET_HTTP_H

#define HTTP_PREFIX "http://"
#define HTTPS_PREFIX "https://"

#define HTTP_PORT 80
#define HTTPS_PORT 443

#define HTTP_REQUEST_BLOCK_SIZE 8192
#define HTTP_REQUEST_HDR_BLOCK_SIZE 1024
#define HTTP_REQUEST_TIMEOUT 10

typedef enum {
    HTTP_REQUEST_OK = 1,
    HTTP_REQUEST_URL_INVALID,
    HTTP_REQUEST_PORT_INVALID,
    HTTP_REQUEST_CONNECTION_ERROR
} HTTP_REQUEST_STATE;

struct http_request {
    /* Common */
    struct pool *pool;
    struct iostream *stream;
    int fd;

    /* Request header */
    struct buffer *hdrs;	/* Headers used in request		*/

    /* Request */
    char *url;			/* Request URL				*/
    char *url_host;		/* Host component in request URL	*/
    int url_port;		/* Port component in request URL	*/
    char *url_path;		/* Path component in request URL	*/
    char *user_agent;           /* User agent				*/

    HTTP_REQUEST_STATE state;   /* Request state			*/
    unsigned long timeout;	/* Request timeout			*/
    unsigned long status;	/* Request status			*/

    int xerrno;
    BOOL isssl;
};

void http_request_timeout_set(struct http_request *request, long timeout);
char *http_request_state_text(long code);
struct http_request *http_request_create(char *url, char *user_agent);
void http_request_header_add(struct http_request *request,
                                const char *key, const char *value);
void http_request_header_dump(struct http_request *request);
BOOL http_request_send(struct http_request *request);
void http_request_free(struct http_request *request);

#define HTTP_RESPONSE_BLOCK_SIZE 8192
#define HTTP_RESPONSE_STATUS_MAX_SIZE 256
#define HTTP_RESPONSE_HEADER_MAX_SIZE 1024
#define HTTP_RESPONSE_BODY_MAX_SIZE 131072

typedef enum {
    HTTP_RESPONSE_INIT,
    HTTP_RESPONSE_HDRS,
    HTTP_RESPONSE_BODY,
    HTTP_RESPONSE_COMPLETE
} HTTP_RESPONSE_STATE;

struct http_response {
    /* Common */
    struct pool *pool;		/* Allocate memory from this pool	*/
    struct iostream *stream;	/* Bidirection pipe 			*/

    /* Input buffer */
    struct buffer *read_buffer;	/* Incoming response 			*/

    HTTP_RESPONSE_STATE state;	/* State of current response 		*/
    unsigned long hdrs_offset;	/* Offset to hdrs in read_buffer	*/
    unsigned long hdrs_size;	/* Size of hdrs in read_buffer		*/
    unsigned long hdrs_max_size;/* Max size of hdrs in read_buffer	*/
    unsigned long hdrs_crlfs;	/* Number of CRLF in header		*/
    unsigned long body_offset;	/* Offset to body in read_buffer	*/
    unsigned long body_current; /* Current offset of body in read_buffer*/
    unsigned long body_size;	/* Size of body in read_buffer		*/
    unsigned long body_max_size;/* Max size of body in read_buffer	*/
    BOOL preserve;		/* Preserve headers			*/
    BOOL iseof;			/* Reached EOF				*/
    BOOL error;			/* Response generated error		*/

    /* Response header */
    struct assoc *hdrs;		/* Headers used in this response	*/

    /* Response information */
    unsigned long status;	/* Status of this response 		*/
    unsigned long status_size;  /* Size of status line 			*/
    unsigned long status_max_size; /* Max size of status line 		*/
};

void http_response_status_size_set(struct http_response *response, long size);
void http_response_header_size_set(struct http_response *response, long size);
void http_response_body_size_set(struct http_response *response, long size);
struct http_response *http_response_create(struct http_request *request);
void http_response_free(struct http_response *response);
BOOL http_response_parse(struct http_response *response);
void http_response_header_dump(struct http_response *response);
char *http_response_body_print(struct http_response *response);
BOOL http_response_complete(struct http_response *response);

#endif /* _HTTP_H_ */
