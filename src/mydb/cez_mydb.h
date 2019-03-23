/*
 * Copyright (c) 2019 Nikola Kolev <koue@chaosophia.net>
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

#ifndef _CEZ_MYDB_H
#define _CEZ_MYDB_H

#include <cez_queue.h>
#include <mysql/mysql.h>

struct cez_mydb {
	MYSQL	*con;
	struct	cez_queue config;
	char	error[128];
};

struct cez_mydb_result {
	MYSQL_RES	*result;
	MYSQL_ROW	_row;
	long		numrows;
	long		numfields;
	long		current;
};

struct cez_mydb *cez_mydb_init(void);
void cez_mydb_set_option(struct cez_mydb *current, const char *name,
    const char *value);
int cez_mydb_connect(struct cez_mydb *current);
struct cez_mydb_result *cez_mydb_query(struct cez_mydb *current,
    const char *query, ...);
int cez_mydb_step(struct cez_mydb_result *current);
int cez_mydb_int(struct cez_mydb_result *current, int field);
void cez_mydb_finalize(struct cez_mydb_result *current);
void cez_mydb_close(struct cez_mydb *current);

#endif

