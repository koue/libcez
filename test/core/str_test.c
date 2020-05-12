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

#include "cez_core_pool.h"
#include "cez_test.h"

int
main(void)
{
	struct pool *pool = pool_create(1024);
	struct str *str;
	char *my, *s1 = "string";
	char *s2 = "zaq1`~!@#$%^&*()_+|}{\\][;:<>,./?'";

	cez_test_start();

	assert((str = str_create(pool, 0)) != NULL);
	assert(str_len(str) == 0);
	assert((my = str_reserve(str, 8)) != NULL);
	assert(sizeof(my) == 8);
	str_rewind(str, 0);
	str_printf(str, "my %s", s1);
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, "my string") == 0);
	assert(str_len(str) == strlen(my));
	str_rewind(str, 0);
	assert(str_len(str) == 0);
	str_putc(str, 'a');
	str_putc(str, 'b');
	str_putc(str, 'c');
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, "abc") == 0);
	assert(str_len(str) == 3);
	str_rewind(str, 2);
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, "ab") == 0);
	assert(str_len(str) == 2);
	str_rewind(str, 0);
	str_puts(str, s1);
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, s1) == 0);
	str_rewind(str, 0);
	str_encode_url(str, s2);
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, "zaq1%60%7e!%40%23%24%25%5e%26*()_%2b%7c%7d%7b%5c%5d%5b%3b%3a%3c%3e%2c.%2f%3f'") == 0);
	str_rewind(str, 0);
	str_encode_canon(str, s2);
	assert((my = str_fetch(str)) != NULL);
	assert(strcmp(my, "zaq1*60*7e!*40*23*24*25*5e*26*()_*2b*7c*7d*7b*5c*5d*5b*3b*3a*3c*3e*2c.*2f*3f'") == 0);

	pool_free(pool);

	return (0);
}
