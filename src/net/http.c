/*
 * Copyright 2020 Nikola Kolev <koue@chaosophia.net>
 * Copyright 2001 Niels Provos <provos@citi.umich.edu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Niels Provos.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

#include "cez_net_http.h"

/*
extern int debug;
#define DFPRINTF(x,y)   if (debug >= x) fprintf y

u_char body[32768];

//void (*http_movecb)(struct uri *, char *) = NULL;

*/
// prayer

struct http_response *
http_response_create(void)
{
    struct pool *p = pool_create(RESPONSE_PREFERRED_BLOCK_SIZE);
    struct http_response *response = pool_alloc(p, sizeof(struct http_response));

    /* Make sure cleared out */
    memset(response, 0, sizeof(struct http_response));

    /* Common */
    response->pool = p;

    /* Input buffer */
    response->read_buffer = buffer_create(p, PREFERRED_BUFFER_BLOCK_SIZE);
    response->state = RESPONSE_INIT;
    response->hdrs_offset = 0;
    response->hdrs_size = 0;
    response->hdrs_max_size = 0;
    response->hdrs_crlfs = 1;
    response->body_offset = 0;
    response->body_size = 0;
    response->body_max_size = 0;

    response->preserve = NIL;
    response->iseof = NIL;
    response->error = NIL;

    response->url = NIL;
    response->url_host = NIL;
    response->url_port = NIL;
    response->url_path = NIL;

    response->hdrs = assoc_create(p, 16, T);

    response->status = 501;	/* Not implemented */

    return (response);
}

void
http_response_free(struct http_response *response)
{
    pool_free(response->pool);
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

    printf("%s\n", data);
#if 0
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
#endif
    return (T);
}

static BOOL
http_response_parse_headers(struct http_response *response)
{
    struct buffer *b = response->read_buffer;
    //unsigned long crlf_count = response->hdrs_crlfs;
    char *data;
    //int c = EOF;
    //unsigned long count = response->hdrs_size;
    //unsigned long maxsize = response->hdrs_max_size;

    /* Record hdrs location first time into loop */
    if (response->hdrs_offset == 0)
        response->hdrs_offset = buffer_size(b);

//    data = buffer_fetch(b, response->hdrs_offset, response->hdrs_size,
//                        response->preserve);
    data = buffer_fetch(b, 0, buffer_size(b), NIL);

    http_response_process_headers(response, data);
    return (T);
}

#if 0
static BOOL http_response_parse_body_init(struct http_response *response)
{
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
    }
}
#endif

BOOL
http_response_parse(struct http_response *response)
{
    while (response->state != RESPONSE_COMPLETE) {
        switch (response->state) {
	case RESPONSE_INIT:
            http_response_parse_headers_init(response);
	    break;

	case RESPONSE_HDRS:
	    if (http_response_parse_headers(response) == NIL)
	        return NIL;
	    // temporary test, remove me
	    response->state = RESPONSE_COMPLETE;
	    //if (http_response_parse_body_init(response) == NIL)
	    //    return NIL;
	    break;

	case RESPONSE_BODY:
	   // if (http_response_parse_body(response) == NIL)
	   //     return NIL;
	   // response->state = RESPONSE_COMPLETE;
	    break;

	case RESPONSE_COMPLETE:
	    break;
	}
    }

    response->state = RESPONSE_COMPLETE;
    return (T);
}
// prayer

#if 0
struct uri *
uri_create(struct pool *pool)
{
	struct uri *uri = pool_alloc(pool, sizeof(struct uri));

	memset(uri, 0, sizeof(struct uri));
	uri->pool = pool;

	uri->fd = -1;

	return (uri);
}

void
uri_free(struct uri *uri)
{
	if (uri->fd != -1)
		close(uri->fd);
	pool_free(uri->pool);
}

int
http_setuseragent(struct uri *uri, char *name)
{
	struct str *str = str_create(uri->pool, 0);

	str_printf(str, "User-Agent: %s\r\n", name);
	if ((uri->user_agent = str_fetch(str)) == NULL)
		return (-1);
	return (0);
}

/* The file descriptor needs to be connected */
int
http_request(struct uri *uri)
{
	struct str *str = str_create(uri->pool, 0);
	char sport[NI_MAXSERV];

	/* fprintf(stdout, "Fetching: %s:%d %s\n", host, port, file); */
	snprintf(sport, sizeof(sport), "%d", uri->url_port);

	str_printf(str, "%s %s HTTP/1.0\r\n"
	    "%s"
	    "Host: %s%s%s\r\n\r\n",
	    uri->flags & HTTP_REQUEST_GET ? "GET" : "HEAD", uri->url_file,
	    uri->user_agent,
	    uri->url_host,
	    uri->url_port != HTTP_DEFAULTPORT ? ":" : "",
	    uri->url_port != HTTP_DEFAULTPORT ? sport : "");

	if ((uri->request = str_fetch(str)) == NULL)
		return (-1);
	return (0);
}

/* Separated host, port and file from URI */

int
http_hostportfile(char *url, char **phost, u_short *pport, char **pfile)
{
	static char host[1024];
	static char file[1024];
	char *p, *p2;
	int len;
	u_short port;

	len = strlen(HTTP_PREFIX);
	if (strncasecmp(url, HTTP_PREFIX, len))
		return (-1);

	url += len;

	/* We might overrun */
	if (strlcpy(host, url, sizeof (host)) >= sizeof(host))
		return (-1);

	p = strchr(host, '/');
	if (p != NULL) {
		*p = '\0';
		p2 = p + 1;
	} else
		p2 = NULL;

	if (pfile != NULL) {
		/* Generate request file */
		if (p2 == NULL)
			p2 = "";
		snprintf(file, sizeof(file), "/%s", p2);
	}

	p = strchr(host, ':');
	if (p != NULL) {
		*p = '\0';
		port = atoi(p + 1);

		if (port == 0)
			return (-1);
	} else
		port = HTTP_DEFAULTPORT;

	if (phost != NULL)
		*phost = host;
	if (pport != NULL)
		*pport = port;
	if (pfile != NULL)
		*pfile = file;

	return (0);
}

struct uri *
http_add(u_short type, char *url, char *useragent)
{
	struct pool *pool = pool_create(1024);

	struct uri *uri;
	u_short port;
	char *host, *file;

//	(normalize koue);

	if (http_hostportfile(url, &host, &port, &file) == -1) {
		fprintf(stderr, "%s: illegal url: %s\n", __func__, url);
		pool_free(pool);
		return (NULL);
	}

	if ((uri = uri_create(pool)) == NULL) {
		warn("%s: malloc", __func__);
		return (NULL);
	}

	if (http_setuseragent(uri, useragent) == -1) {
		goto fail;
	}

	uri->url_host = host;
	uri->url_file = file;
	uri->url_port = port;

	/* GET or HEAD */
	uri->flags = type;

	return (uri);

fail:
	warn("%s: malloc", __func__);
	uri_free(uri);
	return (NULL);
}

void
http_readresponse(struct uri *uri)
{
	char *p;

	p = strstr(uri->header, HTTP_HEADEREND);

	uri->body = pool_alloc(uri->pool, strlen(p) + 1);
	memcpy(uri->body, p, strlen(p));
	uri->body[strlen(p) + 1] = '\0';

	*p = '\0';

	http_parseheader(uri);
	return;
}

void
http_parseheader(struct uri *uri)
{
	char *p, *end;
	int major, minor, code;
	char *type = NULL, *length = NULL, *location = NULL;
	struct header parse[] = {
		{"Content-Type: ", &type},
		{"Content-Length: ", &length},
		{"Location: ", &location},
		{NULL, NULL}
	};
	struct header *hdr;

	if (sscanf(uri->header, "HTTP/%d.%d %d",
		   &major, &minor, &code) != 3 ||
	    major != 1 ||
	    (minor != 0 && minor != 1)) {
		fprintf(stderr, "%s: illegal header in http://%s%s\n",
		    __func__, uri->url_host, uri->url_file);

		uri_free(uri);
		return;
	}

	/* Parse header */
	p = uri->header;
	end = p + uri->hdlen;
	while(p < end) {
		char *lend;

		lend = strstr(p, "\r\n");
		if (lend == NULL) {
			fprintf(stderr, "%s: illegal header in http://%s%s\n",
			    __func__, uri->url_host, uri->url_file);

			uri_free(uri);
			return;
		}

		for (hdr = &parse[0]; hdr->name; hdr++) {
			if (strncasecmp(p, hdr->name,
					strlen(hdr->name)) == 0) {
				*hdr->where = p + strlen(hdr->name);
				break;
			}
		}

		p = lend + 2;
	}

	if (length == NULL)
		uri->length = -1;
	else
		uri->length = atoi(length);

	if (type != NULL) {
		end = strstr(type, "\r\n");
/* koue
		uri->format = malloc(end - type + 1);
		*/
		uri->format = pool_alloc(uri->pool, end - type + 1);

		if (uri->format == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri);
			return;
		}

		memcpy(uri->format, type, end - type);
		uri->format[end - type] = '\0';
	}

	uri->code = code;

	switch (code) {
	case HTTP_OK:
		break;

	case HTTP_MOVETEMP:
	case HTTP_MOVEPERM:
/*		if (location == NULL || http_movecb == NULL ||
		    (use_robots && http_isrobotstxt(uri)))
			goto error;
			*/

		p = location;
		end = strstr(p, "\r\n");
/* koue
		if ((location = malloc(end - p + 1)) == NULL) {
		*/
		if ((location = pool_alloc(uri->pool, end - p + 1)) == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri);
			return;
		}

		memcpy(location, p, end - p);
		location[end - p] = '\0';

		/* User call back for move */
//		(*http_movecb)(uri, location);

		free(location);
		uri_free(uri);
		return;

	default:
		goto error;
	}

	/* If we just had a HEAD request, we are done now */
//	if (uri->flags & HTTP_REQUEST_HEAD) {
//		http_dispatch(uri, HTTPDIS_FREE);
//		return;
//	}

	return;

 error:
	uri_free(uri);
	return;
}

char *
http_normalize_uri(char *uri)
{
	static char normal[1024];
	char *host, *file, *p;
	u_short port;

	if (http_hostportfile(uri, &host, &port, &file) == -1)
		return (NULL);

	if ((p = strchr(file, '#')) != NULL)
		*p = '\0';

	/* Remove identities */
	p = file;
	while ((p = strstr(p, "/./")) != NULL) {
		memmove(p, p + 2, strlen(p + 2) + 1);
	}

	p = file;
	while ((p = strstr(p, "//")) != NULL) {
		char *p2 = p + strspn(p, "/");

		memmove(p + 1, p2, strlen(p2) + 1);
	}

	/* Deal with ../ */
	while ((p = strstr(file, "/..")) != NULL) {
		char *p2;

		for (p2 = p - 1; p2 > file; p2--)
			if (*p2 == '/')
				break;

		if (p2 <= file)
			memmove(file, p + 3, strlen(p + 3) + 1);
		else
			memmove(p2, p + 3, strlen(p + 3) + 1);
	}

	if (port != HTTP_DEFAULTPORT)
		snprintf(normal, sizeof(normal), "http://%s:%d", host, port);
	else
		snprintf(normal, sizeof(normal), "http://%s", host);

	/* Unix file names should not be lowered */
	p = normal;
	while (*p) {
		*p = tolower(*p);
		p++;
	}

	/* Append file and check for overrun */
	if (strlcat(normal, file, sizeof (normal)) >= sizeof (normal))
		return (NULL);

	return (normal);
}

/* Depends on http_normalize_uri */

char *
http_basename(struct uri *uri)
{
	char *p, *url, *normal;

	url = http_make_url(&uri->url);
	normal = http_normalize_uri(url);

	if (normal == NULL)
		return (NULL);

	p = strrchr(normal, '/');

	/* This should never happen */
	if (p == NULL)
		return (NULL);

	if (p[1] != '\0')
		p[1] = '\0';

	return (normal);
}

char *
http_make_uri(char *base, char *rel)
{
	static char normal[1024];

	if (!strncasecmp(rel, HTTP_PREFIX, strlen(HTTP_PREFIX)) ||
	    strchr(rel, ':') != NULL)
		return (rel);

	if (rel[0] == '/') {
		char *host, *file;
		u_short port;

		if (http_hostportfile(base, &host, &port, &file) == -1)
			return (NULL);

		if (port != HTTP_DEFAULTPORT)
			snprintf(normal, sizeof (normal), "http://%s:%d%s",
				 host, port, rel);
		else
			snprintf(normal, sizeof (normal), "http://%s%s",
				 host, rel);
	} else if (rel[0] == '#')
		return (NULL);
	else
		snprintf(normal, sizeof (normal), "%s%s", base, rel);

	return (http_normalize_uri(normal));
}

char *
http_make_url(struct url *url)
{
	static char output[1024];

	if (url->port != HTTP_DEFAULTPORT)
		snprintf(output, sizeof(output), "http://%s:%d%s",
		    url->host, url->port, url->file);
	else
		snprintf(output, sizeof(output), "http://%s%s",
		    url->host, url->file);

	return (output);
}

#endif
