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

#include "cez_net_http.h"

extern int debug;
#define DFPRINTF(x,y)   if (debug >= x) fprintf y

u_char body[32768];
char *useragent = NULL;

ssize_t atomicio(ssize_t (*f)(), int, void *, size_t);

//void (*http_movecb)(struct uri *, char *) = NULL;

struct uri *
uri_new(void)
{
	struct uri *uri;

	if ((uri = calloc(1, sizeof (struct uri))) == NULL)
		return (NULL);

	uri->fd = -1;

	return (uri);
}

void
uri_free(struct uri *uri, int i)
{
	if (uri->fd != -1)
		close(uri->fd);
	if (uri->url_host != NULL)
		free(uri->url_host);
	if (uri->url_file != NULL)
		free(uri->url_file);
	if (uri->header != NULL)
		free(uri->header);
	if (uri->body != NULL)
		free(uri->body);
	if (uri->format != NULL)
		free(uri->format);
	free(uri);
}

int
http_setuseragent(char *name)
{
	char agent[1024];

	if (useragent != NULL)
		free(useragent);

	snprintf(agent, sizeof (agent), "User-Agent: %s\r\n", name);
	useragent = strdup(agent);

	return (useragent != NULL ? 0 : -1);
}

/* The file descriptor needs to be connected */
void
http_fetch(struct uri *uri)
{
	char request[1024];
	char sport[NI_MAXSERV];

	/* fprintf(stdout, "Fetching: %s:%d %s\n", host, port, file); */
	snprintf(sport, sizeof(sport), "%d", uri->url_port);

	snprintf(request, sizeof(request),
	    "%s %s HTTP/1.0\r\n"
	    "%s"
	    "Host: %s%s%s\r\n\r\n",
	    uri->flags & HTTP_REQUEST_GET ? "GET" : "HEAD", uri->url_file,
	    useragent != NULL ? useragent : "",
	    uri->url_host,
	    uri->url_port != HTTP_DEFAULTPORT ? ":" : "",
	    uri->url_port != HTTP_DEFAULTPORT ? sport : "");

	    printf("=======================================\n");
	    printf("%s\n", request);
	    printf("=======================================\n");
	    atomicio(write, uri->fd, request, strlen(request));
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
http_add(u_short type, char *url)
{
	struct uri *uri;
	u_short port;
	char *host, *file;

//	(normalize koue);

	if (http_hostportfile(url, &host, &port, &file) == -1) {
		fprintf(stderr, "%s: illegal url: %s\n", __func__, url);
		return (NULL);
	}

	if ((uri = uri_new()) == NULL) {
		warn("%s: malloc", __func__);
		return (NULL);
	}

	uri->url_host = strdup(host);
	uri->url_file = strdup(file);
	uri->url_port = port;
	if (uri->url_host == NULL || uri->url_file == NULL) {
		warn("%s: malloc", __func__);
		uri_free(uri, 0);
		return (NULL);
	}

	/* GET or HEAD */
	uri->flags = type;

	return (uri);
}

void
http_readheader(struct uri *uri)
{
	char line[20048], *p;
	ssize_t n, offset;

	n = read(uri->fd, line, sizeof(line));
	if (n == -1) {
		if (errno == EINTR || errno == EAGAIN)
			return;
		warn("%s: read", __func__);
		uri_free(uri, URI_CLEANCONNECT);

		return;
	} else if (n == 0) {
		/* Uhm dum */
		fprintf(stderr, "%s: finished read on http://%s%s?\n",
		    __func__, uri->url_host, uri->url_file);

		uri_free(uri, URI_CLEANCONNECT);
		return;
	}

	p = realloc(uri->header, uri->hdlen + n + 1);
	if (p == NULL) {
		warn("%s: realloc", __func__);
		uri_free(uri, URI_CLEANCONNECT);

		return;
	}

	uri->header = p;
	memcpy(uri->header + uri->hdlen, line, n);
	uri->hdlen += n;
	uri->header[uri->hdlen] = '\0';

	p = strstr(uri->header, HTTP_HEADEREND);
	/*
	if (p == NULL)
		goto readmore;
		*/

	offset = p + strlen(HTTP_HEADEREND) - uri->header;

	if (offset < uri->hdlen) {
		uri->bdlen = uri->hdlen - offset;
		uri->body = malloc(uri->bdlen + 1);
		if (uri->body == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri, URI_CLEANCONNECT);

			return;
		}
		memcpy(uri->body, uri->header + offset, uri->bdlen);
		uri->body[uri->bdlen] = '\0';

		/* Adjust header */
		uri->hdlen = offset;
		uri->header[offset] = '\0';
	}

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

		uri_free(uri, URI_CLEANCONNECT);
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

			uri_free(uri, URI_CLEANCONNECT);
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

		uri->format = malloc(end - type + 1);
		if (uri->format == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri, URI_CLEANCONNECT);
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

		if ((location = malloc(end - p + 1)) == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri, URI_CLEANCONNECT);
			return;
		}

		memcpy(location, p, end - p);
		location[end - p] = '\0';

		/* User call back for move */
//		(*http_movecb)(uri, location);

		free(location);
		uri_free(uri, URI_CLEANCONNECT);
		return;

	default:
		goto error;
	}

	/* If we just had a HEAD request, we are done now */
//	if (uri->flags & HTTP_REQUEST_HEAD) {
//		http_dispatch(uri, HTTPDIS_FREE);
//		return;
//	}

 conturi:

	if (uri->length != -1) {
		size_t len;
		u_char *p;

		len = uri->length;
		if (len > HTTP_MAXMEM)
			len = HTTP_MAXMEM;

		p = realloc(uri->body, len + 1);
		if (p == NULL) {
			warn("%s: malloc", __func__);
			uri_free(uri, URI_CLEANCONNECT);
			return;
		}
		uri->body = p;
		uri->body[len] = '\0';
		uri->bdmemlen = len;
		uri->bdread = uri->bdlen;
	}

	return;

 error:
	uri_free(uri, URI_CLEANCONNECT);
	return;
}

void
http_readbody(struct uri *uri)
{
	ssize_t n;
	u_char *where;
	ssize_t len;

	if (uri->length == -1) {
		where = body;
		len = sizeof(body);
	} else {
		where = uri->body + uri->bdread;
		len = uri->length - uri->bdlen;
		if (len > uri->bdmemlen - uri->bdread)
			len = uri->bdmemlen - uri->bdread;
	}

	n = read(uri->fd, where, len);
	if (n == -1) {
	//	if (errno == EINTR || errno == EAGAIN)
	//		goto readmore;
		warn("%s: read", __func__);
		uri_free(uri, URI_CLEANCONNECT);

		return;
	} else if (n == 0) {
		if (uri->length != -1 &&
		    uri->length != uri->bdlen) {
			fprintf(stderr, "%s: short read on http://%s%s\n",
			    __func__, uri->url_host, uri->url_file);
			uri_free(uri, URI_CLEANCONNECT);
			return;
		}

		uri->length = uri->bdlen;
		goto done;
	}

	if (uri->length == -1) {
		u_char *p;

		p = realloc(uri->body, uri->bdlen + n + 1);
		if (p == NULL) {
			warn("%s: realloc", __func__);
			uri_free(uri, URI_CLEANCONNECT);

			return;
		}

		uri->body = p;
		memcpy(uri->body + uri->bdlen, body, n);
		uri->bdlen += n;
		uri->body[uri->bdlen] = '\0';
	} else {
		uri->bdlen += n;
		uri->bdread += n;
	}

//	if (uri->length == -1 || uri->bdlen < uri->length)
//		goto readmore;

	/* We are done with this document */

 done:
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
