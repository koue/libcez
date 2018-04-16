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
** A Blob is a variable-length containers for arbitrary string
** or binary data.
*/

#include "cez_fossil.h"

const Blob empty_blob = BLOB_INITIALIZER;

/*
** Return a pointer to a null-terminated string for a blob.
*/
char *blob_str(Blob *p){
  blob_is_init(p);
  if( p->nUsed==0 ){
    blob_append(p, "", 1); /* NOTE: Changes nUsed. */
    p->nUsed = 0;
  }
  if( p->aData[p->nUsed]!=0 ){
    blob_materialize(p);
  }
  return p->aData;
}

/*
** Make sure a blob is nul-terminated and is not a pointer to unmanaged
** space. Return a pointer to the data.
*/
char *blob_materialize(Blob *pBlob){
  blob_resize(pBlob, pBlob->nUsed);
  return pBlob->aData;
}

/*
** Append text or data to the end of a blob.
*/
void blob_append(Blob *pBlob, const char *aData, int nData){
  assert( aData!=0 || nData==0 );
  blob_is_init(pBlob);
  if( nData<0 ) nData = strlen(aData);
  if( nData==0 ) return;
  if( pBlob->nUsed + nData >= pBlob->nAlloc ){
    pBlob->xRealloc(pBlob, pBlob->nUsed + nData + pBlob->nAlloc + 100);
    if( pBlob->nUsed + nData >= pBlob->nAlloc ){
      blob_panic();
    }
  }
  memcpy(&pBlob->aData[pBlob->nUsed], aData, nData);
  pBlob->nUsed += nData;
  pBlob->aData[pBlob->nUsed] = 0;   /* Blobs are always nul-terminated */
}

/*
** This routine is acalled if a blob operation fails because we
** have run out of memory.
*/
void blob_panic(void){
  static const char zErrMsg[] = "out of memory\n";
  fputs(zErrMsg, stderr);
/*  db_close(1); */
  exit(1);
}

/*
** A reallocation function that assumes that aData came from malloc().
** This function attempts to resize the buffer of the blob to hold
** newSize bytes.
**
** No attempt is made to recover from an out-of-memory error.
** If an OOM error occurs, an error message is printed on stderr
** and the program exists.
*/
void blobReallocMalloc(Blob *pBlob, unsigned int newSize){
  if( newSize==0 ){
    free(pBlob->aData);
    pBlob->aData = 0;
    pBlob->nAlloc = 0;
    pBlob->nUsed = 0;
    pBlob->iCursor = 0;
    pBlob->blobFlags = 0;
  }else if( newSize>pBlob->nAlloc || newSize<pBlob->nAlloc-4000 ){
    /* char *pNew = fossil_realloc(pBlob->aData, newSize); */
    char *pNew = realloc(pBlob->aData, newSize);
    pBlob->aData = pNew;
    pBlob->nAlloc = newSize;
    if( pBlob->nUsed>pBlob->nAlloc ){
      pBlob->nUsed = pBlob->nAlloc;
    }
  }
}

/*
** Attempt to resize a blob so that its internal buffer is
** nByte in size. The blob is truncated if necessary.
*/
void blob_resize(Blob *pBlob, unsigned int newSize){
  pBlob->xRealloc(pBlob, newSize+1);
  pBlob->nUsed = newSize;
  pBlob->aData[newSize] = 0;
}

/*
** A reallocation function for when the initial string is in unmanaged
** space. Copy the string to memory obtained from malloc().
*/
void blobReallocStatic(Blob *pBlob, unsigned int newSize){
  if( newSize==0 ){
    *pBlob = empty_blob;
  }else{
    /* char *pNew = fossil_malloc( newSize ); */
    char *pNew = malloc( newSize );
    if( pBlob->nUsed>newSize ) pBlob->nUsed = newSize;
    memcpy(pNew, pBlob->aData, pBlob->nUsed);
    pBlob->aData = pNew;
    pBlob->xRealloc = blobReallocMalloc;
    pBlob->nAlloc = newSize;
  }
}

/*
** Initialize a blob to the data on an input channel. Return
** the number of bytes read into the blob. Any prior content
** of the blob is discarded, no freed.
*/
int blob_read_from_channel(Blob *pBlob, FILE *in, int nToRead){
  size_t n;
  blob_zero(pBlob);
  if( nToRead<0 ){
    char zBuf[10000];
    while( !feof(in) ){
      n = fread(zBuf, 1, sizeof(zBuf), in);
      if( n>0 ){
        blob_append(pBlob, zBuf, n);
      }
    }
  }else{
    blob_resize(pBlob, nToRead);
    n = fread(blob_buffer(pBlob), 1, nToRead, in);
    blob_resize(pBlob, n);
  }
  return blob_size(pBlob);
}

/*
** Initialize a blob to an empty string.
*/
void blob_zero(Blob *pBlob){
  static const char zEmpty[] = "";
  assert_blob_is_reset(pBlob);
  pBlob->nUsed = 0;
  pBlob->nAlloc = 1;
  pBlob->aData = (char*)zEmpty;
  pBlob->iCursor = 0;
  pBlob->blobFlags = 0;
  pBlob->xRealloc = blobReallocStatic;
}

void blob_vappendf(Blob *pBlob, const char *zFormat, va_list ap){
  if( pBlob ) vxprintf(pBlob, zFormat, ap);
}

/*
** Reset a blob to be an empty container.
*/
void blob_reset(Blob *pBlob){
  blob_is_init(pBlob);
  pBlob->xRealloc(pBlob, 0);
}

/*
** Initialize a blob to a string or byte-array constant of a specified length.
** Any prior data in the blob is discarded.
*/
void blob_init(Blob *pBlob, const char *zData, int size){
  assert_blob_is_reset(pBlob);
  if( zData==0 ){
    *pBlob = empty_blob;
  }else{
    if( size<=0 ) size = strlen(zData);
    pBlob->nUsed = pBlob->nAlloc = size;
    pBlob->aData = (char*)zData;
    pBlob->iCursor = 0;
    pBlob->blobFlags = 0;
    pBlob->xRealloc = blobReallocStatic;
  }
}
