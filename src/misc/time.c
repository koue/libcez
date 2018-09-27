/*
 * Copyright (c) 2011-2018 Nikola Kolev <koue@chaosophia.net>
 * Copyright (c) 2004-2006 Daniel Hartmeier. All rights reserved.
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

#include "cez_misc.h"

double
timelapse(struct timeval *t)
{
	struct timeval u;
	double d;

	gettimeofday(&u, NULL);
	d = (double)((u.tv_sec * 1000000 + u.tv_usec) -
	    (t->tv_sec * 1000000 + t->tv_usec)) / 1000.0;
	memcpy(t, &u, sizeof(*t));
	return (d);
}

const char *
rfc822_time(time_t t)
{
	static char s[30], *p;

	p = ctime(&t);
	if (p == NULL || strlen(p) != 25) {
		strlcpy(s, "<invalid-time>", sizeof(s));
		return (s);
	}
	/* Thu Nov 24 18:22:48 1986\n */
	/* Wed, 02 Oct 2002 13:00:00 GMT */
	strlcpy(s, p, 4);
	strlcat(s, ", ", 6);
	strlcat(s, p + 8, 9);
	strlcat(s, p + 4, 13);
	strlcat(s, p + 20, 17);
	strlcat(s, " ", 18);
	strlcat(s, p + 11, 26);
	strlcat(s, " GMT", 30);
	return (s);
}

time_t
convert_rfc822_time(const char *date)
{
/*
	I found few bugs in strptime on FreeBSD already: wday, yday.
	Using sscanf instead.

	struct tm tm;
	time_t t;

	strptime(date, "%a, %e %h %Y %H:%M:%S %z", &tm);
	t = mktime(&tm);
*/
	const char *mns[13] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
	    "Aug", "Sep", "Oct", "Nov", "Dec", NULL };
	char wd[4], mn[4], zone[16];
	int d, h, m, s, y, i;
	struct tm tm;
	time_t t;

	if (sscanf(date, "%3s, %d %3s %d %d:%d:%d %15s",
	    wd, &d, mn, &y, &h, &m, &s, zone) != 8) {
		return (0);
	}
	for (i = 0; mns[i] != NULL; ++i) {
		if (strcmp(mns[i], mn) == 0) {
			break;
		}
	}
	if (mns[i] == NULL) {
		return (0);
	}
	memset(&tm, 0, sizeof(tm));
	tm.tm_year = y - 1900;
	tm.tm_mon = i;
	tm.tm_mday = d;
	tm.tm_hour = h;
	tm.tm_min = m;
	tm.tm_sec = s;
	tm.tm_zone = zone;
	t = mktime(&tm);
	return (t);
}
