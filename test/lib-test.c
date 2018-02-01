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

#include "cez-fossil.h"
#include "cez-misc.h"

Global g;

const char TestSchema[] =
"CREATE TABLE tbl_test("
" id INTEGER PRIMARY KEY,"
" name TEXT"
");";

const char *dbname = "/tmp/testme-xzfadget48.db";
const char *sqltracefile = "/tmp/testme-4trwerfsdfrf89.log";

Stmt q;

int main(void){
  char command[256];

  test_start();

  /* create database */
  db_init_database(dbname, TestSchema, (char*)0);
  test_ok("init database");
  test_ok("create schema");

  /* open database */
  if (sqlite3_open(dbname, &g.db) != SQLITE_OK) {
    fprintf(stderr, "Cannot open database file: %s\n", dbname);
    return (1);
  }
  sqlite3_trace_v2(g.db, SQLITE_TRACE_STMT, db_sql_trace, 0);
  if ((g.sqltrace = fopen(sqltracefile, "w+")) == NULL) {
    fprintf(stderr, "Cannot open sql trace file: %s\n", sqltracefile);
    sqlite3_close(g.db);
    return (1);
  }

  /* insert record */
  db_multi_exec("INSERT INTO tbl_test(name) VALUES (%Q)", "testuser0");
  db_multi_exec("INSERT INTO tbl_test(name) VALUES (%Q)", "testuser1");
  db_multi_exec("INSERT INTO tbl_test(name) VALUES (%Q)", "testuser2");
  db_multi_exec("INSERT INTO tbl_test(name) VALUES (%Q)", "testuser3");
  test_ok("insert");
  /* get text */
  char *zDb_text = db_text(0, "SELECT name FROM tbl_test "
				"WHERE name='testuser1'");
  printf("%20d, %s\n", 0, zDb_text);
  free(zDb_text);
  test_ok("db_text");
  /* get int */
  printf("%20d\n", db_int(0, "SELECT id FROM tbl_test WHERE name='testuser2'"));
  test_ok("db_int");
  /* multiple results */
  db_prepare(&q, "SELECT id, name FROM tbl_test");
  test_ok("db_prepare");
  while(db_step(&q)==SQLITE_ROW){
    printf("%20d, %s\n", db_column_int(&q, 0), db_column_text(&q, 1));
  }
  db_finalize(&q);
  test_ok("db_step");
  test_ok("db_finalize");

  sqlite3_close(g.db);
  snprintf(command, sizeof(command), "rm %s", dbname);
  system(command);
  fclose(g.sqltrace);
  snprintf(command, sizeof(command), "echo && cat %s", sqltracefile);
  system(command);
  snprintf(command, sizeof(command), "rm %s", sqltracefile);
  system(command);

  printf("\n");
  test_ok("sql_trace");

  test_succeed();

  test_end();

  return (0);
}
