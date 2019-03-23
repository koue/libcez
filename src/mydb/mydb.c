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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cez_queue.h>
#include <cez_mydb.h>

struct cez_mydb *
cez_mydb_init(void)
{
	struct cez_mydb *current;
	if ((current = calloc(1, sizeof(*current))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}
	cez_queue_init(&current->config);
	if ((current->con = mysql_init(NULL)) == NULL) {
		return (NULL);
	}
	return (current);
}

void
cez_mydb_set_option(struct cez_mydb *current, const char *name,
    const char *value)
{
	if (cqa(&current->config, name, value) == -1) {
		printf("%s: fail\n", __func__);
		exit(1);
	}
	return;
}

int
cez_mydb_connect(struct cez_mydb *current)
{
	const char *name;
	const char *cez_mydb_options_list[] = { "hostname", "username",
	    "password", "database", NULL };

	if ((name = cqc(&current->config, cez_mydb_options_list)) != NULL) {
		snprintf(current->error, sizeof(current->error),
		    "Missing option: %s", name);
		return (-1);
	}
	if (mysql_real_connect(current->con, cqg(&current->config, "hostname"),
	    cqg(&current->config, "username"), cqg(&current->config, "password"),
	    cqg(&current->config, "database"), 0, NULL, 0) == NULL) {
		snprintf(current->error, sizeof(current->error), "%s",
		    mysql_error(current->con));
		return (-1);
	}
	return (0);
}

struct cez_mydb_result *
cez_mydb_query(struct cez_mydb *current, const char *query, ...)
{
	char *statement;
	struct cez_mydb_result *Stmt;
	va_list ap;

	if ((Stmt = calloc(1, sizeof(*Stmt))) == NULL) {
		fprintf(stderr, "[ERROR] %s: %s\n", __func__, strerror(errno));
		exit(1);
	}

	va_start(ap, query);
	vasprintf(&statement, query, ap);
	va_end(ap);

	if (mysql_query(current->con, statement)) {
		snprintf(current->error, sizeof(current->error),
		    "%s: %s", __func__, mysql_error(current->con));
		return (NULL);
	}

	Stmt->result = mysql_store_result(current->con);

	if ((Stmt->result == NULL) && mysql_errno(current->con)) {
		return (NULL);
	}

	if (Stmt->result) {
		Stmt->numrows = mysql_num_rows(Stmt->result);
		Stmt->numfields = mysql_num_fields(Stmt->result);
	} else {
		Stmt->numfields = 0;
		Stmt->numrows = 0;
	}
	Stmt->current = 0;
	free(statement);

	return (Stmt);
}

int
cez_mydb_step(struct cez_mydb_result *current)
{
	if ((current->result == NULL) || (current->current == current->numrows))
		return (0);

	current->current++;
	current->_row = mysql_fetch_row(current->result);
	return (1);
}

int
cez_mydb_int(struct cez_mydb_result *current, int field)
{
	if (field >= current->numfields)
		return (0);
	return ((int)strtol(current->_row[field], NULL, 10));
}

void
cez_mydb_finalize(struct cez_mydb_result *current)
{
	mysql_free_result(current->result);
	free(current);
}

void
cez_mydb_close(struct cez_mydb *current)
{
	mysql_close(current->con);
	cez_queue_purge(&current->config);
	free(current);
	return;
}
