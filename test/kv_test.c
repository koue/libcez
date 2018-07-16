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

#include <stdio.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/types.h>

#include "cez_kv.h"
#include "cez_misc.h"

int
main(void){
	struct kvtree	mytree;
	struct kv	*kv, key;

	test_start();
	key.kv_key = "root";
	if ((kv = kv_find(&mytree, &key)) != NULL) {
		test_fail("kv_find, not exist");
	} else {
		test_ok("kv_find, not exist");
	}
	if (kv_add(&mytree, "root", "trunk") == NULL) {
		test_fail("kv_add");
		goto fail;
	} else {
		test_ok("kv_add");
	}
	if ((kv = kv_find(&mytree, &key)) != NULL) {
		test_ok("kv_find");
		printf("%20s->%s\n", kv->kv_key, kv->kv_value);
	} else {
		test_fail("kv_find");
	}
	if (kv_extend(&mytree, kv, "-major") == NULL) {
		test_fail("kv_extend");
		goto fail;
	} else {
		test_ok("kv_extend");
		printf("%20s->%s\n", kv->kv_key, kv->kv_value);
	}
	if (kv_set(kv, "hollow") == -1) {
		test_fail("kv_set");
	} else {
		test_ok("kv_set");
		printf("%20s->%s\n", kv->kv_key, kv->kv_value);
	}
	if (kv_setkey(kv, "stub") == -1) {
		test_fail("kv_setkey");
	} else {
		test_ok("kv_setkey");
		printf("%20s->%s\n", kv->kv_key, kv->kv_value);
	}
	kv_delete(&mytree, kv);
	test_ok("kv_delete");

	test_succeed();

fail:
	test_end();

	return (0);
}
