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
	MYSQL	*conn;
	struct	cez_queue config;
	char	error[1024];
};

typedef struct cez_mydb cez_mydb;

struct cez_mydb_res {
	MYSQL_RES	*res;
	MYSQL_ROW	row;
	long		numrows;
	long		numfields;
	long		current;
};

typedef struct cez_mydb_res cez_mydb_res;

cez_mydb *cez_mydb_init(void);
void cez_mydb_set_option(cez_mydb *db, const char *name, const char *value);
int cez_mydb_connect(cez_mydb *db);
int cez_mydb_exec(cez_mydb *db, const char *zSql, ...);
cez_mydb_res *cez_mydb_query(cez_mydb *db, const char *query, ...);
int cez_mydb_step(cez_mydb_res *stmt);
int cez_mydb_column_int(cez_mydb_res *stmt, int field);
long cez_mydb_column_long(cez_mydb_res *stmt, int field);
float cez_mydb_column_float(cez_mydb_res *stmt, int field);
double cez_mydb_column_double(cez_mydb_res *stmt, int field);
const char *cez_mydb_column_text(cez_mydb_res *stmt, int field);
void cez_mydb_finalize(cez_mydb_res *stmt);
void cez_mydb_close(cez_mydb *stmt);

#endif

