/*
 * Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net>
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

#include "cez-prayer.h"

#include "cez-misc.h"

struct item {
	struct pool *pool;	/* Allocation pool */
	struct assoc *assoc;	/* Associative array for fast lookups */
};

int main(void){
	struct pool *pool = pool_create(1024);
	struct item *i;

	test_start();

	i = pool_alloc(pool, sizeof(struct item));
	i->pool = pool;
	i->assoc = assoc_create(pool, 16, T);
	test_ok("assoc_create");
	/* Add word to assoc chain */
	assoc_update(i->assoc, "first", "1", NIL);
	assoc_update(i->assoc, "second", "1", NIL);
	assoc_update(i->assoc, "third", "1", NIL);
	test_ok("assoc_update");

	if(assoc_lookup(i->assoc, "second"))
		test_ok("assoc_lookup found");
	else
		test_fail("assoc_lookup found");

	assoc_delete(i->assoc, "second");
	test_ok("assoc_delete");

	if(assoc_lookup(i->assoc, "second"))
		test_fail("assoc_lookup missing");
	else
		test_ok("assoc_lookup missing");

	pool_free(i->pool);

	test_succeed();
	test_end();

	return (0);
}
