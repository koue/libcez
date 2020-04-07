/*
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

#ifndef _HTTP_H_
#define _HTTP_H_

#define HTTP_MAXRETRY		3
#define HTTP_MAXCONNECTS	20
#define HTTP_CONNECT_TIMEOUT	50
#define HTTP_READTIMEOUT	180

#define HTTP_MAXPARALLEL	3	/* number of connections per host */

#define HTTP_PREFIX		"http://"
#define HTTP_DEFAULTPORT	80

#define HTTP_OK			200
#define HTTP_MOVEPERM		301
#define HTTP_MOVETEMP		302
#define HTTP_NOTFOUND		404

#define HTTP_HEADEREND		"\r\n\r\n"

#define HTTP_REQUEST_GET	0x0001
#define HTTP_REQUEST_HEAD	0x0002

#define HTTP_MAXMEM		128*1024

#define URI_CLEANCONNECT	0x01	/* reduces number of connects */

#define url_host	url.host
#define url_file	url.file
#define url_port	url.port

struct url {
	char *host;
	char *file;
	unsigned short port;
};

struct uri {
	unsigned short flags;
	unsigned short retry;

	int code;

	struct url url;

	char *format;
	ssize_t length;

	char *header;
	size_t hdlen;

	char *body;
	size_t bdmemlen;
	size_t bdread;
	size_t bdlen;

	int fd;
};

struct header {
	char *name;
	char **where;
};

struct uri *uri_new(void);
void uri_free(struct uri *, int);

struct uri *http_add(unsigned short, char *);

int http_setuseragent(char *);

void http_fetch(struct uri *uri);
int http_hostportfile(char *, char **, unsigned short *, char **);
void http_parseheader(struct uri *);
void http_readheader(struct uri *);
void http_readbody(struct uri *);
void http_dispatch(struct uri *, int);
#define HTTPDIS_KEEP	0
#define HTTPDIS_FREE	1

char *http_normalize_uri(char *);
char *http_basename(struct uri *);
char *http_make_uri(char *, char *);

char *http_make_url(struct url *);

#endif /* _HTTP_H_ */
