/*
 * Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
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
#include <stdlib.h>

#include "cez_config.h"
#include "cez_misc.h"

int main(void) {
	const char *paramsok[] = { "param1", "param2", "param4", NULL };
	const char *paramsfail[] = { "param1", "param2", "param3", "param4",
	    NULL };
	const char *key;
	char *value;

	test_start();

	if (configfile_parse("./configrc", config_queue_cb) == -1)
		exit(1);
	test_ok("configfile_parse");
	if ((key = config_queue_check(paramsok)) == NULL)
		test_ok("config_queue_check");
	else
		test_fail("config_queue_check");
	if ((key = config_queue_check(paramsfail)) != NULL)
		test_ok("config_queue_check false");
	else
		test_fail("config_queue_check false");
	config_queue_print();
	test_ok("config_queue_print");
	value = config_queue_value_get("param4");
	printf("param4 = %s\n", value);
	value = config_queue_value_get("param5");
	printf("param5 = %s\n", value);
	test_ok("config_queue_value_get");
	config_queue_purge();
	test_ok("config_queue_purge");
	test_succeed();
	test_end();

	return (0);
}
