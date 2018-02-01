/*
** Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
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
};
extern Global g;

/*
** PRINTF
*/

char *mprintf(const char *zFormat, ...);
char *vmprintf(const char *zFormat, va_list ap);
int vxprintf(Blob *pBlob, const char *fmt, va_list ap);

/*
** BLOB
*/

struct Blob {
  unsigned int nUsed;            /* Number of bytes used in aData[] */
  unsigned int nAlloc;           /* Number of bytes allocated for aData[] */
  unsigned int iCursor;          /* Next character of input to parse */
  unsigned int blobFlags;        /* One or more BLOBFLAG_* bits */
  char *aData;                   /* Where the information is stored */
  void (*xRealloc)(Blob*, unsigned int); /* Function to reallocate the buffer */
};

#define assert_blob_is_reset(x)

#define blob_size(X)  ((X)->nUsed)

#define blob_buffer(X)  ((X)->aData)

#define blob_is_init(x) \
  assert((x)->xRealloc==blobReallocMalloc || (x)->xRealloc==blobReallocStatic)

#define BLOB_INITIALIZER  {0,0,0,0,0,blobReallocMalloc}

void blob_panic(void);
void blob_append(Blob *pBlob, const char *aData, int nData);
char *blob_materialize(Blob *pBlob);
char *blob_str(Blob *p);
void blobReallocMalloc(Blob *pBlob, unsigned int newSize);
void blob_resize(Blob *pBlob, unsigned int newSize);
void blobReallocMalloc(Blob *pBlob, unsigned int newSize);
void blobReallocStatic(Blob *pBlob, unsigned int newSize);
extern const Blob empty_blob;
int blob_read_from_channel(Blob *pBlob, FILE *in, int nToRead);
void blob_zero(Blob *pBlob);
void blob_vappendf(Blob *pBlob, const char *zFormat, va_list ap);
void blob_reset(Blob *pBlob);
void blob_init(Blob *pBlob, const char *zData, int size);

struct Stmt {
  Blob sql;               /* The SQL for this statement */
  sqlite3_stmt *pStmt;    /* The results of sqlite3_prepare_v2() */
  Stmt *pNext, *pPrev;    /* List of all unfinalized statements */
  int nStep;              /* Number of sqlite3_step() calls */
};

/*
** DB
*/

typedef sqlite3_int64 i64;

char *db_text(const char *zDefault, const char *zSql, ...);
int db_finalize(Stmt *pStmt);
void db_check_result(int rc);
void db_err(const char *zFormat, ...);
void db_close(int reportErrors);
int db_vprepare(Stmt *pStmt, int errOk, const char *zFormat, va_list ap);
int db_step(Stmt *pStmt);
void db_stats(Stmt *pStmt);
void db_end_transaction(int rollbackFlag);
int db_int(int iDflt, const char *zSql, ...);
int db_multi_exec(const char *zSql, ...);
int db_column_int(Stmt *pStmt, int N);
sqlite3_int64 db_column_int64(Stmt *pStmt, int N);
int db_database_slot(const char *zLabel);
const char *db_column_text(Stmt *pStmt, int N);
int db_prepare_ignore_error(Stmt *pStmt, const char *zFormat, ...);
int db_prepare(Stmt *pStmt, const char *zFormat, ...);
void db_init_database(const char *zFileName, const char *zSchema, ...);
int db_sql_trace(unsigned m, void *notUsed, void *pP, void *pX);

/*
** CGI
*/
#define P(x)		cgi_parameter((x),0)
#define PD(x,y)		cgi_parameter((x),(y))

void cgi_setenv(const char *zName, const char *zValue);
void cgi_set_parameter_nocopy(const char *zName, const char *zValue, int isQP);
int qparam_compare(const void *a, const void *b);
const char *cgi_parameter(const char *zName, const char *zDefault);
int AsciiToHex(int c);
int dehttpize(char *z);
void add_param_list(char *z, int terminator);
void cgi_set_parameter(const char *zName, const char *zValue);
void cgi_replace_parameter(const char *zName, const char *zValue);

#endif
