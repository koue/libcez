/*
 * Copyright (c) 2020-2021 Nikola Kolev <koue@chaosophia.net>
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

#include <stdlib.h>
#include <string.h>

#include "cez_util.h"

int
cez_util_isspace(char c)
{
	return c==' ' || (c<='\r' && c>='\t');
}

int
cez_util_param_list(const char *list, int terminator, int (*fn)(const char *name,
    const char *value, void *arg), void *arg)
{
	char *zList, *zFree;

	if (list == NULL)
		return (-1);
	zFree = zList = strdup(list);
	while (*zList) {
		char *zName;
		char *zValue;
		while (cez_util_isspace(*zList))
			zList++;
		zName = zList;
		while (*zList && *zList != '=' && *zList != terminator)
			zList++;
		if (*zList == '=') {
			*zList = 0;
			zList++;
			zValue = zList;
			while(*zList && *zList != terminator)
				zList++;
			if (*zList) {
				*zList = 0;
				zList++;
			}
		} else {
			if (*zList)
				*zList++ = 0;
			zValue = "";
		}
		if (fn(zName, zValue, arg) != 0) {
			free(zFree);
			return (-1);
		}
	}
	free(zFree);
	return (0);
}

