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
#include <string.h>
#include <sys/queue.h>
#include <sys/tree.h>
#include <sys/types.h>

#include "cez_kv.h"
#include "cez_test.h"

int
main(void)
{
	struct kvtree	mytree;
	struct kv	*kv, key;

	cez_test_start();
	kv_init(&mytree);
	key.kv_key = "root";
	assert((kv = kv_find(&mytree, &key)) == NULL);
	assert(kv_add(&mytree, "root", "trunk") != NULL);
	assert((kv = kv_find(&mytree, &key)) != NULL);
	assert(kv_extend(&mytree, kv, "-major") != NULL);
	assert(kv_set(kv, "hollow") != -1);
	assert(kv_setkey(kv, "stub") != -1);
	kv_delete(&mytree, kv);
	assert(kv_add_list(&mytree, "p1=v1;p2=v2;p3=", ';') == 0);
	key.kv_key = "p2";
	assert((kv = kv_find(&mytree, &key)) != NULL);
	assert(strlen(kv->kv_value));
	key.kv_key = "p3";
	assert((kv = kv_find(&mytree, &key)) != NULL);
	assert(strlen(kv->kv_value) == 0);
	kv_purge(&mytree);

	return (0);
}
