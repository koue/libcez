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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cez_queue.h>
#include <cez_mydb.h>

cez_mydb *
cez_mydb_init(void)
{
	cez_mydb *db;

	if ((db = calloc(1, sizeof(*db))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	cez_queue_init(&db->config);
	if ((db->conn = mysql_init(NULL)) == NULL) {
		return (NULL);
	}
	return (db);
}

void
cez_mydb_set_option(cez_mydb *db, const char *name, const char *value)
{
	if (cqa(&db->config, name, value) == -1) {
		printf("%s: fail\n", __func__);
		exit(1);
	}
}

int
cez_mydb_connect(cez_mydb *db)
{
	const char *name;
	const char *cez_mydb_options_list[] = { "hostname", "username",
	    "password", "database", NULL };

	if ((name = cqc(&db->config, cez_mydb_options_list)) != NULL) {
		snprintf(db->error, sizeof(db->error),
		    "Missing option: %s", name);
		return (-1);
	}
	if (mysql_real_connect(db->conn, cqg(&db->config, "hostname"),
	    cqg(&db->config, "username"), cqg(&db->config, "password"),
	    cqg(&db->config, "database"), 0, NULL, 0) == NULL) {
		snprintf(db->error, sizeof(db->error), "%s",
		    mysql_error(db->conn));
		return (-1);
	}
	return (0);
}

cez_mydb_res *
cez_mydb_query(cez_mydb *db, const char *query, ...)
{
	char *statement;
	cez_mydb_res *Stmt;
	va_list ap;

	if ((Stmt = calloc(1, sizeof(*Stmt))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}

	va_start(ap, query);
	vasprintf(&statement, query, ap);
	va_end(ap);

	if (mysql_query(db->conn, statement)) {
		snprintf(db->error, sizeof(db->error),
		    "%s: %s", __func__, mysql_error(db->conn));
		return (NULL);
	}

	Stmt->res = mysql_store_result(db->conn);

	if ((Stmt->res == NULL) && mysql_errno(db->conn)) {
		return (NULL);
	}

	if (Stmt->res) {
		Stmt->numrows = mysql_num_rows(Stmt->res);
		Stmt->numfields = mysql_num_fields(Stmt->res);
	} else {
		Stmt->numfields = 0;
		Stmt->numrows = 0;
	}
	Stmt->current = 0;
	free(statement);

	return (Stmt);
}

int
cez_mydb_step(cez_mydb_res *stmt)
{
	if ((stmt->res == NULL) || (stmt->current == stmt->numrows))
		return (0);

	stmt->current++;
	stmt->row = mysql_fetch_row(stmt->res);
	return (1);
}

int
cez_mydb_int(cez_mydb_res *stmt, int field)
{
	if (field >= stmt->numfields)
		return (0);
	return ((int)strtol(stmt->row[field], NULL, 10));
}

void
cez_mydb_finalize(cez_mydb_res *stmt)
{
	mysql_free_result(stmt->res);
	free(stmt);
}

void
cez_mydb_close(cez_mydb *db)
{
	mysql_close(db->conn);
	cez_queue_purge(&db->config);
	free(db);
}
