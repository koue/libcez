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

#include "cez_util.h"
#include "cez_test.h"

int
param_list_false(const char *name, const char *value)
{
	if ((strcmp(name, "p1") == 0) && (strcmp(value, "v1false") != 0))
		return (-1);
	return (0);
}

int
param_list_true(const char *name, const char *value)
{
	if ((strcmp(name, "p1") == 0) && (strcmp(value, "v1") == 0))
		return (0);
	if ((strcmp(name, "p2") == 0) && (strcmp(value, "v2") == 0))
		return (0);
	if ((strcmp(name, "p3") == 0) && (strcmp(value, "") == 0))
		return (0);
	return (-1);
}

int
main(void)
{
	int i;
	static const char *list = "p1=v1;p2=v2;p3=";

	cez_test_start();
	for (i = 0; i <= 255; i++) {
		if(i==' ' || i=='\n' || i=='\t' || i=='\v' || i=='\f' || i=='\r')
			assert(cez_util_isspace((char)i) == 1);
		else
			assert(cez_util_isspace((char)i) == 0);
	}
	assert(cez_util_param_list(list, ';', param_list_true) == 0);
	assert(cez_util_param_list(list, ';', param_list_false) == -1);

	return (0);
}

