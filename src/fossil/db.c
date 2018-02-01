/*
** Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
** Copyright (c) 2006 D. Richard Hipp
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)
**
** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
** Author contact information:
**   drh@hwaci.com
**   http://www.hwaci.com/drh/
**
*******************************************************************************
**
** Code for interfacing to the various databases.
**
*/

#include "cez-fossil.h"

/*
** All static variable that a used by only this file are gathered into
** the following structure.
*/
static struct DbLocalData {
  int nBegin;               /* Nesting depth of BEGIN */
  int doRollback;           /* True to force a rollback */
  int nCommitHook;          /* Number of commit hooks */
  Stmt *pAllStmt;           /* List of all unfinalized statements */
  int nPrepare;             /* Number of calls to sqlite3_prepare_v2() */
  int nDeleteOnFail;        /* Number of entries in azDeleteOnFail[] */
  struct sCommitHook {
    int (*xHook)(void);         /* Functions to call at db_end_transaction() */
    int sequence;               /* Call functions in sequence order */
  } aHook[5];
  char *azDeleteOnFail[3];  /* Files to delete on a failure */
  char *azBeforeCommit[5];  /* Commands to run prior to COMMIT */
  int nBeforeCommit;        /* Number of entries in azBeforeCommit */
  int nPriorChanges;        /* sqlite3_total_changes() at transaction start */
} db = {0, 0, 0, 0, 0, 0, };

/*
** Execute a query. Return the first column of the first row
** of the result set as a string. Space to hold the string is
** obtained from malloc(). If the result set is empty, return
** zDefault instead.
*/
char *db_text(const char *zDefault, const char *zSql, ...){
  va_list ap;
  Stmt s;
  char *z;
  va_start(ap, zSql);
  db_vprepare(&s, 0, zSql, ap);
  va_end(ap);
  if( db_step(&s)==SQLITE_ROW ){
    z = mprintf("%s", sqlite3_column_text(s.pStmt, 0));
  }else if( zDefault ){
    z = mprintf("%s", zDefault);
  }else{
    z = 0;
  }
  db_finalize(&s);
  return z;
}

/*
** Finalize a statement.
*/
int db_finalize(Stmt *pStmt){
  int rc;
  db_stats(pStmt);
  blob_reset(&pStmt->sql);
  rc = sqlite3_finalize(pStmt->pStmt);
  db_check_result(rc);
  pStmt->pStmt = 0;
  if( pStmt->pNext ){
    pStmt->pNext->pPrev = pStmt->pPrev;
  }
  if( pStmt->pPrev ){
    pStmt->pPrev->pNext = pStmt->pNext;
  }else if( db.pAllStmt==pStmt ){
    db.pAllStmt = pStmt->pNext;
  }
  pStmt->pNext = 0;
  pStmt->pPrev = 0;
  return rc;
}

/*
** check a result code. If it is not SQLITE_OK, print the
** corresponding error message and exit.
*/
void db_check_result(int rc){
  if( rc!=SQLITE_OK ){
    db_err("SQL error: %s", sqlite3_errmsg(g.db));
  }
}

/*
** Call this routine when a database error occurs.
*/
void db_err(const char *zFormat, ...){
  va_list ap;
  char *z;
/* koue  int rc = 1; */
  va_start(ap, zFormat);
  z = vmprintf(zFormat, ap);
  va_end(ap);
#if 0 /* koue */
#ifdef FOSSIL_ENABLE_JSON
  if( g.json.isJsonMode ){
    json_err( 0, z, 1 );
    if( g.isHTTP ){
      rc = 0 /* avoid HTTP 500 */;
    }
  }
  else
#endif /* FOSSIL_ENABLE_JSON */
  if( g.xferPanic ){
    cgi_reset_content();
    @ error Database\serror:\s%F(z)
      cgi_reply();
  }
  else if( g.cgiOutput ){
    g.cgiOutput = 0;
    cgi_printf("<h1>Database Error</h1>\n<p>%h</p>\n", z);
    cgi_reply();
  }else{
    fprintf(stderr, "%s: %s\n", g.argv[0], z);
  }
#endif /* koue */
  fprintf(stderr, "%s: %s\n", __func__, z);
  free(z);
/* koue  db_force_rollback(); */
  db_close(1);
  exit(1);
}

/*
** Close the database connection.
**
** Check for unfinalized statements and report errors if the reportErrors
** argument is true. Ignore unfinalized statements when false.
*/
void db_close(int reportErrors){
  sqlite3_stmt *pStmt;
  if( g.db==0 ) return;
#if 0 /* koue */
  if( g.fSqlStats ){
    int cur, hiwtr;
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_USED, &cur, &hiwtr, 0);
    fprintf(stderr, "-- LOOKASIDE_USED         %10d %10d\n", cur, hiwtr);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_HIT, &cur, &hiwtr, 0);
    fprintf(stderr, "-- LOOKASIDE_HIT                     %10d\n", hiwtr);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE, &cur,&hiwtr,0);
    fprintf(stderr, "-- LOOKASIDE_MISS_SIZE               %10d\n", hiwtr);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL, &cur,&hiwtr,0);
    fprintf(stderr, "-- LOOKASIDE_MISS_FULL               %10d\n", hiwtr);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_CACHE_USED, &cur, &hiwtr, 0);
    fprintf(stderr, "-- CACHE_USED             %10d\n", cur);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_SCHEMA_USED, &cur, &hiwtr, 0);
    fprintf(stderr, "-- SCHEMA_USED            %10d\n", cur);
    sqlite3_db_status(g.db, SQLITE_DBSTATUS_STMT_USED, &cur, &hiwtr, 0);
    fprintf(stderr, "-- STMT_USED              %10d\n", cur);
    sqlite3_status(SQLITE_STATUS_MEMORY_USED, &cur, &hiwtr, 0);
    fprintf(stderr, "-- MEMORY_USED            %10d %10d\n", cur, hiwtr);
    sqlite3_status(SQLITE_STATUS_MALLOC_SIZE, &cur, &hiwtr, 0);
    fprintf(stderr, "-- MALLOC_SIZE                       %10d\n", hiwtr);
    sqlite3_status(SQLITE_STATUS_MALLOC_COUNT, &cur, &hiwtr, 0);
    fprintf(stderr, "-- MALLOC_COUNT           %10d %10d\n", cur, hiwtr);
    sqlite3_status(SQLITE_STATUS_PAGECACHE_OVERFLOW, &cur, &hiwtr, 0);
    fprintf(stderr, "-- PCACHE_OVFLOW          %10d %10d\n", cur, hiwtr);
    fprintf(stderr, "-- prepared statements    %10d\n", db.nPrepare);
  }
#endif /* koue */
  while( db.pAllStmt ){
    db_finalize(db.pAllStmt);
  }
  db_end_transaction(1);
  pStmt = 0;
#if 0 /* koue */
  db_close_config();
#endif /* koue */

  /* If the localdb has a lot of unused free space,
  ** then VACUUM it as we shut down.
  */
  if( db_database_slot("localdb")>=0 ){
    int nFree = db_int(0, "PRAGMA localdb.freelist_count");
    int nTotal = db_int(0, "PRAGMA localdb.page_count");
    if( nFree>nTotal/4 ){
      db_multi_exec("VACUUM localdb;");
    }
  }

  if( g.db ){
    int rc;
    sqlite3_wal_checkpoint(g.db, 0);
    rc = sqlite3_close(g.db);
    if( rc==SQLITE_BUSY && reportErrors ){
      while( (pStmt = sqlite3_next_stmt(g.db, pStmt))!=0 ){
        /* koue fossil_warning("unfinalized SQL statement: [%s]", sqlite3_sql(pStmt)); */
        fprintf(stderr, "unfinalized SQL statement: [%s]", sqlite3_sql(pStmt));
      }
    }
    g.db = 0;
  }
#if 0 /* koue */
  g.repositoryOpen = 0;
  g.localOpen = 0;
  assert( g.dbConfig==0 );
  assert( g.zConfigDbName==0 );
#endif /* koue */
}

/*
** Prepare a Stmt. Assume that the Stmt is previously uninitialized.
** If the input tring contains multiple SQL statements, only the first
** one is processed. All statements beyond the first are silently ignored.
*/
int db_vprepare(Stmt *pStmt, int errOk, const char *zFormat, va_list ap){
  int rc;
  char *zSql;
  blob_zero(&pStmt->sql);
  blob_vappendf(&pStmt->sql, zFormat, ap);
  va_end(ap);
  zSql = blob_str(&pStmt->sql);
  db.nPrepare++;
  rc = sqlite3_prepare_v2(g.db, zSql, -1, &pStmt->pStmt, 0);
  if( rc!=0 && !errOk ){
    db_err("%s\n%s", sqlite3_errmsg(g.db), zSql);
  }
  pStmt->pNext = pStmt->pPrev = 0;
  pStmt->nStep = 0;
  return rc;
}

/*
** Step the SQL statement. Return either SQLITE_ROW or an error code
** or SQLITE_OK if the statement finishes successfully.
*/
int db_step(Stmt *pStmt){
  int rc;
  rc = sqlite3_step(pStmt->pStmt);
  pStmt->nStep++;
  return rc;
}

/*
** Print warnings if a query is inefficient.
*/
void db_stats(Stmt *pStmt){
#ifdef FOSSIL_DEBUG
  int c1, c2, c3;
  const char *zSql = sqlite3_sql(pStmt->pStmt);
  if( zSql==0 ) return;
  c1 = sqlite3_stmt_status(pStmt->pStmt, SQLITE_STMTSTATUS_FULLSCAN_STEP, 1);
  c2 = sqlite3_stmt_status(pStmt->pStmt, SQLITE_STMTSTATUS_AUTOINDEX, 1);
  c3 = sqlite3_stmt_status(pStmt->pStmt, SQLITE_STMTSTATUS_SORT, 1);
  if( c1>pStmt->nStep*4 && strstr(zSql,"/*scan*/")==0 ){
    fossil_warning("%d scan steps for %d rows in [%s]", c1, pStmt->nStep, zSql);
  }else if( c2 ){
    fossil_warning("%d automatic index rows in [%s]", c2, zSql);
  }else if( c3 && strstr(zSql,"/*sort*/")==0 && strstr(zSql,"/*scan*/")==0 ){
    fossil_warning("sort w/o index in [%s]", zSql);
  }
  pStmt->nStep = 0;
#endif
}

/*
** End a nested transaction
*/
void db_end_transaction(int rollbackFlag){
  if( g.db==0 ) return;
  if( db.nBegin<=0 ) return;
  if( rollbackFlag ) db.doRollback = 1;
  db.nBegin--;
  if( db.nBegin==0 ){
    int i;
    if( db.doRollback==0 && db.nPriorChanges<sqlite3_total_changes(g.db) ){
      i = 0;
      while( db.nBeforeCommit ){
        db.nBeforeCommit--;
        sqlite3_exec(g.db, db.azBeforeCommit[i], 0, 0, 0);
        sqlite3_free(db.azBeforeCommit[i]);
        i++;
      }
/* koue
      leaf_do_pending_checks();
*/
    }
    for(i=0; db.doRollback==0 && i<db.nCommitHook; i++){
      db.doRollback |= db.aHook[i].xHook();
    }
    while( db.pAllStmt ){
      db_finalize(db.pAllStmt);
    }
    db_multi_exec("%s", db.doRollback ? "ROLLBACK" : "COMMIT");
    db.doRollback = 0;
  }
}

/*
** Execute a query and return a single integer value.
*/
i64 db_int64(i64 iDflt, const char *zSql, ...){
  va_list ap;
  Stmt s;
  i64 rc;
  va_start(ap, zSql);
  db_vprepare(&s, 0, zSql, ap);
  va_end(ap);
  if( db_step(&s)!=SQLITE_ROW ){
    rc = iDflt;
  }else{
    rc = db_column_int64(&s, 0);
  }
  db_finalize(&s);
  return rc;
}

int db_int(int iDflt, const char *zSql, ...){
  va_list ap;
  Stmt s;
  int rc;
  va_start(ap, zSql);
  db_vprepare(&s, 0, zSql, ap);
  va_end(ap);
  if( db_step(&s)!=SQLITE_ROW ){
    rc = iDflt;
  }else{
    rc = db_column_int(&s, 0);
  }
  db_finalize(&s);
  return rc;
}

/*
** Execute multiple SQL statements.
*/
int db_multi_exec(const char *zSql, ...){
  Blob sql;
  int rc = SQLITE_OK;
  va_list ap;
  const char *z, *zEnd;
  sqlite3_stmt *pStmt;
  blob_init(&sql, 0, 0);
  va_start(ap, zSql);
  blob_vappendf(&sql, zSql, ap);
  va_end(ap);
  z = blob_str(&sql);
  while( rc==SQLITE_OK && z[0] ){
    pStmt = 0;
    rc = sqlite3_prepare_v2(g.db, z, -1, &pStmt, &zEnd);
    if( rc ){
      db_err("%s: {%s}", sqlite3_errmsg(g.db), z);
    }else if( pStmt ){
      db.nPrepare++;
      while( sqlite3_step(pStmt)==SQLITE_ROW ){}
      rc = sqlite3_finalize(pStmt);
      if( rc ) db_err("%s: {%.*s}", sqlite3_errmsg(g.db), (int)(zEnd-z), z);
    }
    z = zEnd;
  }
  blob_reset(&sql);
  return rc;
}

/*
** Extract integer value from the N-th column of the
** current row.
*/
int db_column_int(Stmt *pStmt, int N){
  return sqlite3_column_int(pStmt->pStmt, N);
}

i64 db_column_int64(Stmt *pStmt, int N){
  return sqlite3_column_int64(pStmt->pStmt, N);
}

/*
** Return the slot number for database zLabel. The first database
** opened is slot 0. The "temp" database is slot 1. Attached databases
** are slots 2 and higher.
**
** Return -1 if zLabel does not match any open databases.
*/
int db_database_slot(const char *zLabel){
  int iSlot = -1;
  int rc;
  Stmt q;
  if( g.db==0 ) return iSlot;
  rc = db_prepare_ignore_error(&q, "PRAGMA database_list");
  if( rc!=SQLITE_OK ) return iSlot;
  while( db_step(&q)==SQLITE_ROW ){
    /* koue if( fossil_strcmp(db_column_text(&q,1),zLabel)==0 ){ */
    if( strcmp(db_column_text(&q,1),zLabel)==0 ){
      iSlot = db_column_int(&q, 0);
      break;
    }
  }
  db_finalize(&q);
  return iSlot;
}

/*
** Extract text value from the N-th column of the
** current row.
*/
const char *db_column_text(Stmt *pStmt, int N){
  return (char*)sqlite3_column_text(pStmt->pStmt, N);
}

/*
** Prepare a Stmt. Assume that the Stmt is previously uninitialized.
** If the input string contains multiple SQL statements, only the first
** one is processed. All statements beyond the first are silently ignored.
*/
int db_prepare_ignore_error(Stmt *pStmt, const char *zFormat, ...){
  int rc;
  va_list ap;
  va_start(ap, zFormat);
  rc = db_vprepare(pStmt, 1, zFormat, ap);
  va_end(ap);
  return rc;
}

int db_prepare(Stmt *pStmt, const char *zFormat, ...){
  int rc;
  va_list ap;
  va_start(ap, zFormat);
  rc = db_vprepare(pStmt, 0, zFormat, ap);
  va_end(ap);
  return rc;
}

/*
** Initialize a new database file with the given schema. If anything
** goes wrong, call db_err() to exit.
*/
void db_init_database(
  const char *zFileName,   /* Name of database file to create */
  const char *zSchema,     /* First part of schema */
  ...                      /* Additional SQL to run.  Terminate with NULL. */
){
  sqlite3 *db;
  int rc;
  const char *zSql;
  va_list ap;

/* koue  db = db_open(zFileName); */
  if (sqlite3_open(zFileName, &db) != SQLITE_OK) {
    fprintf(stderr, "Cannot open database file: %s\n", zFileName);
    exit(1);
  }

  sqlite3_exec(db, "BEGIN EXCLUSIVE", 0, 0, 0);
  rc = sqlite3_exec(db, zSchema, 0, 0, 0);
  if( rc!=SQLITE_OK ){
    db_err("%s", sqlite3_errmsg(db));
  }
  va_start(ap, zSchema);
  while( (zSql = va_arg(ap, const char*))!=0 ){
    rc = sqlite3_exec(db, zSql, 0, 0, 0);
    if( rc!=SQLITE_OK ){
      db_err("%s", sqlite3_errmsg(db));
    }
  }
  va_end(ap);
  sqlite3_exec(db, "COMMIT", 0, 0, 0);
  sqlite3_close(db);
}

/* sql trace */
int db_sql_trace(unsigned m, void *notUsed, void *pP, void *pX){
  sqlite3_stmt *pStmt = (sqlite3_stmt*)pP;
  char *zSql;
  int n;
  const char *zArg = (const char*)pX;
  if( zArg[0]=='-' ) return 0;
  zSql = sqlite3_expanded_sql(pStmt);
  n = (int)strlen(zSql);
  if (g.sqltrace != NULL)
    fprintf(g.sqltrace, "%s%s\n", zSql, (n>0 && zSql[n-1]==';') ? "" : ";");
  else
    fprintf(stderr, "%s%s\n", zSql, (n>0 && zSql[n-1]==';') ? "" : ";");
  sqlite3_free(zSql);
  return (0);
}
