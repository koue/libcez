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

#include "cez_core.h"
#include "cez_test.h"

struct item {
	char *p_strdup;
	char *p_strcat;
	char *p_strcat3;
	char *p_printf;
	char *p_join;
};

int
main(void)
{
	struct pool *pool;
	struct item *item;
	char *str_join[] = {"pool", "join", NULL};

	cez_test_start();

	assert((pool = pool_create(sizeof(struct item))) != NULL);
	assert((item = pool_alloc(pool, sizeof(struct item))) != NULL);

	assert((item->p_strdup = pool_strdup(pool, "p_strdup")) != NULL);
	assert(strcmp(item->p_strdup, "p_strdup") == 0);
	assert(strlen(item->p_strdup) == 8);

	assert((item->p_strcat = pool_strcat(pool, "p_", "strcat")) != NULL);
	assert(strcmp(item->p_strcat, "p_strcat") == 0);
	assert(strlen(item->p_strcat) == 8);

	assert((item->p_strcat3 = pool_strcat3(pool, "p_", "strcat", "3")) != NULL);
	assert(strcmp(item->p_strcat3, "p_strcat3") == 0);
	assert(strlen(item->p_strcat3) == 9);

	assert((item->p_printf = pool_printf(pool, "p_%s %d", "printf", 4)) != NULL);
	assert(strcmp(item->p_printf, "p_printf 4") == 0);
	assert(strlen(item->p_printf) == 10);

	assert((item->p_join = pool_join(pool, '_', str_join)) != NULL);
	assert(strcmp(item->p_join, "pool_join") == 0);
	assert(strlen(item->p_join) == 9);

	pool_free(pool);

	return (0);
}
