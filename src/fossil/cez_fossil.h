/*
** Copyright (c) 2017-2019 Nikola Kolev <koue@chaosophia.net>
** Copyright (c) 2006 D. Richard Hipp
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)

** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
** Author contact information:
**   drh@hwaci.com
**   http://www.hwaci.com/drh/
**
*******************************************************************************
*/

#ifndef _CEZ_FOSSIL_H
#define _CEZ_FOSSIL_H

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <time.h>

typedef struct Global Global;
typedef struct Stmt Stmt;
typedef struct Blob Blob;

struct Global {
  sqlite3 *db;
  FILE *sqltrace;
  int dbIgnoreErrors;
};
extern Global g;

/*
** PRINTF
*/

int vxprintf(Blob *pBlob, const char *fmt, va_list ap);
char *mprintf(const char *zFormat, ...);
char *vmprintf(const char *zFormat, va_list ap);

/*
** BLOB
*/

/*
** A Blob can hold a string or a binary object of arbitrary size.  The
** size changes as necessary.
*/
struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  unsigned int blobFlags;        /* One or more BLOBFLAG_* bits */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};

/*
** Make sure a blob does not contain malloced memory.
**
** This might fail if we are unlucky and x is uninitialized.  For that
** reason it should only be used locally for debugging.  Leave it turned
** off for production.
*/
#if 0  /* Enable for debugging only */
#define assert_blob_is_reset(x) assert(blob_is_reset(x))
#else
#define assert_blob_is_reset(x)
#endif

/*
** The current size of a Blob
*/
#define blob_size(X)  ((X)->nUsed)

/*
** The buffer holding the blob data
*/
#define blob_buffer(X)  ((X)->aData)

/*
** Make sure a blob is initialized
*/
#define blob_is_init(x) \
  assert((x)->xRealloc==blobReallocMalloc || (x)->xRealloc==blobReallocStatic)

#define BLOB_INITIALIZER  {0,0,0,0,0,blobReallocMalloc}

extern const Blob empty_blob;

char *blob_str(Blob *p);
char *blob_materialize(Blob *pBlob);
void blob_append(Blob *pBlob, const char *aData, int nData);
void blob_append_char(Blob *pBlob, char c);
void blobReallocMalloc(Blob *pBlob, unsigned int newSize);
void blob_resize(Blob *pBlob, unsigned int newSize);
int blob_read_from_channel(Blob *pBlob, FILE *in, int nToRead);
void blob_zero(Blob *pBlob);
void blob_vappendf(Blob *pBlob, const char *zFormat, va_list ap);
void blob_reset(Blob *pBlob);
void blob_init(Blob *pBlob, const char *zData, int size);

/*
** DB
*/

/*
** An single SQL statement is represented as an instance of the following
** structure.
*/
struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare_v2() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
  int rc;                 /* Error from db_vprepare() */
};

typedef sqlite3_int64 i64;

#define DB_PREPARE_IGNORE_ERROR  0x001  /* Suppress errors */
#define DB_PREPARE_PERSISTENT    0x002  /* Stmt will stick around for a while */

char *db_text(const char *zDefault, const char *zSql, ...);
int db_finalize(Stmt *pStmt);
void db_check_result(int rc);
void db_close(int reportErrors);
int db_vprepare(Stmt *pStmt, int flags, const char *zFormat, va_list ap);
int db_step(Stmt *pStmt);
void db_end_transaction(int rollbackFlag);
i64 db_int64(i64 iDflt, const char *zSql, ...);
int db_int(int iDflt, const char *zSql, ...);
int db_multi_exec(const char *zSql, ...);
int db_column_int(Stmt *pStmt, int N);
i64 db_column_int64(Stmt *pStmt, int N);
int db_database_slot(const char *zLabel);
const char *db_column_text(Stmt *pStmt, int N);
int db_prepare_ignore_error(Stmt *pStmt, const char *zFormat, ...);
int db_prepare(Stmt *pStmt, const char *zFormat, ...);
void db_init_database(const char *zFileName, const char *zSchema, ...);
int db_sql_trace(unsigned m, void *notUsed, void *pP, void *pX);
#endif
