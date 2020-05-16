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

#define RESPONSE_PREFERRED_BLOCK_SIZE (8192)

typedef enum {
    RESPONSE_INIT,
    RESPONSE_HDRS,
    RESPONSE_BODY,
    RESPONSE_COMPLETE
} RESPONSE_STATE;

struct http_response {
    /* Common */
    struct pool *pool;		/* Allocate memory from this pool	*/
    struct iostream *stream;	/* Bidirection pipe 			*/

    /* Input buffer */
    struct buffer *read_buffer;	/* Incoming response 			*/

    RESPONSE_STATE state;	/* State of current response 		*/
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

    /* Request */
    char *url;			/* Request URL				*/
    char *url_host;		/* Host component in request URL	*/
    char *url_port;		/* Port component in request URL	*/
    char *url_path;		/* Path component in request URL	*/

    /* Response header */
    struct assoc *hdrs;		/* Headers used in this response	*/

    /* Response information */
    unsigned long status;	/* Status of this response */
    unsigned long status_size;  /* Size of status line */
    unsigned long status_max_size; /* Max size of status line */
};

struct http_response *http_response_create(struct iostream *stream);

void http_response_free(struct http_response *response);

BOOL http_response_parse(struct http_response *response);

BOOL http_response_complete(struct http_response *response);

#endif /* _HTTP_H_ */
