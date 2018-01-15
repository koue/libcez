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
** This file contains C functions and procedures that provide useful
** services to CGI programs.  There are procedures for parsing and
** dispensing QUERY_STRING parameters and cookies, the "mprintf()"
** formatting function and its cousins, and routines to encode and
** decode strings in HTML or HTTP.
*/

#include "cez.h"

/*
** Information about all query parameters and cookies are stored
** in these variables.
*/
static int nAllocQP = 0;	/* Space allocated for aParamQP[] */
static int nUsedQP = 0;		/* Space actually used in aParamQP[] */
static int sortQP = 0;		/* True if aParamQP[] needs sorting */
static int seqQP = 0;		/* Sequence numbers */
static struct QParam {		/* One entry for eachquery parameter or cookie */
  const char *zName;		/* Parameter or cookie name */
  const char *zValue;		/* Value of the query parameter or cookie */
  int seq;			/* Order of insertion */
  char isQP;			/* True for query parameters */
  char cTag;			/* Tag on query parameters */
} *aParamQP;			/* An array of all parameters and cookies */

/*
** Add a query parameter.  The zName portion is fixed but a copy
** must be made of zValue.
*/
void cgi_setenv(const char *zName, const char *zValue){
  cgi_set_parameter_nocopy(zName, mprintf("%s",zValue), 0);
}

/*
** Add another query parameter or cookie to the parameter set.
** zName is the name of the query parameter or cookie and zValue
** is its fully decoded value.
**
** zName and zValue are not copied and must not change or be
** deallocated after this routine returns.
*/
void cgi_set_parameter_nocopy(const char *zName, const char *zValue, int isQP){
  if( nAllocQP<=nUsedQP ){
    nAllocQP = nAllocQP*2 + 10;
    if( nAllocQP>1000 ){
      /* Prevent a DOS service attack against the framework */
      //fossil_fatal("Too many query parameters");
      fprintf(stderr, "Too many query parameters");
      exit(1);
    }
    aParamQP = realloc( aParamQP, nAllocQP*sizeof(aParamQP[0]) );
  }
  aParamQP[nUsedQP].zName = zName;
  aParamQP[nUsedQP].zValue = zValue;
/*
  if( g.fHttpTrace ){
    fprintf(stderr, "# cgi: %s = [%s]\n", zName, zValue);
  }
*/
  aParamQP[nUsedQP].seq = seqQP++;
  aParamQP[nUsedQP].isQP = isQP;
  aParamQP[nUsedQP].cTag = 0;
  nUsedQP++;
  sortQP = 1;
}

/*
** This is the comparison function used to sort the aParamQP[] array of
** query parameters and cookies.
*/
int qparam_compare(const void *a, const void *b){
  struct QParam *pA = (struct QParam*)a;
  struct QParam *pB = (struct QParam*)b;
  int c;
  c = strcmp(pA->zName, pB->zName);
  if( c==0 ){
    c = pA->seq - pB->seq;
  }
  return c;
}

/*
** Return the value of a query parameter or cookie whose name is zName.
** If there is no query parameter or cookie named zName and the first
** character of zName is uppercase, then check to see if there is an
** environment variable by that name and return it if there is.  As
** a last resort when nothing else matches, return zDefault.
*/
const char *cgi_parameter(const char *zName, const char *zDefault){
  int lo, hi, mid, c;

  /* The sortQP flag is set whenever a new query parameter is inserted.
  ** It indicates that we need to resort the query parameters.
  */
  if( sortQP ){
    int i, j;
    qsort(aParamQP, nUsedQP, sizeof(aParamQP[0]), qparam_compare);
    sortQP = 0;
    /* After sorting, remove duplicate parameters.  The secondary sort
    ** key is aParamQP[].seq and we keep the first entry.  That means
    ** with duplicate calls to cgi_set_parameter() the second and
    ** subsequent calls are effectively no-ops. */
    for(i=j=1; i<nUsedQP; i++){
      if( strcmp(aParamQP[i].zName,aParamQP[i-1].zName)==0 ){
        continue;
      }
      if( j<i ){
        memcpy(&aParamQP[j], &aParamQP[i], sizeof(aParamQP[j]));
      }
      j++;
    }
    nUsedQP = j;
  }

  /* Do a binary search for a matching query parameter */
  lo = 0;
  hi = nUsedQP-1;
  while( lo<=hi ){
    mid = (lo+hi)/2;
    c = strcmp(aParamQP[mid].zName, zName);
    if( c==0 ){
      //CGIDEBUG(("mem-match [%s] = [%s]\n", zName, aParamQP[mid].zValue));
      return aParamQP[mid].zValue;
    }else if( c>0 ){
      hi = mid-1;
    }else{
      lo = mid+1;
    }
  }

  /* If no match is found and the name begins with an upper-case
  ** letter, then check to see if there is an environment variable
  ** with the given name.
  */
  //if( zName && fossil_isupper(zName[0]) ){
  if( zName ){
    const char *zValue = getenv(zName);
    if( zValue ){
      cgi_set_parameter_nocopy(zName, zValue, 0);
      //CGIDEBUG(("env-match [%s] = [%s]\n", zName, zValue));
      return zValue;
    }
  }
  //CGIDEBUG(("no-match [%s]\n", zName));
  return zDefault;
}

/*
** Convert a single HEX digit to an integer
*/
int AsciiToHex(int c){
  if( c>='a' && c<='f' ){
    c += 10 - 'a';
  }else if( c>='A' && c<='F' ){
    c += 10 - 'A';
  }else if( c>='0' && c<='9' ){
    c -= '0';
  }else{
    c = 0;
  }
  return c;
}

/*
** Remove the HTTP encodings from a string.  The conversion is done
** in-place.  Return the length of the string after conversion.
*/
int dehttpize(char *z){
  int i, j;

  /* Treat a null pointer as a zero-length string. */
  if( !z ) return 0;

  i = j = 0;
  while( z[i] ){
    switch( z[i] ){
      case '%':
        if( z[i+1] && z[i+2] ){
          z[j] = AsciiToHex(z[i+1]) << 4;
          z[j] |= AsciiToHex(z[i+2]);
          i += 2;
        }
        break;
      case '+':
        z[j] = ' ';
        break;
      default:
        z[j] = z[i];
        break;
    }
    i++;
    j++;
  }
  z[j] = 0;
  return j;
}

/* fossil */
static int fossil_isspace(char c){
  return c==' ' || (c<='\r' && c>='\t');
}

static int fossil_islower(char c){ return c>='a' && c<='z'; }

/*
** Add a list of query parameters or cookies to the parameter set.
**
** Each parameter is of the form NAME=VALUE.  Both the NAME and the
** VALUE may be url-encoded ("+" for space, "%HH" for other special
** characters).  But this routine assumes that NAME contains no
** special character and therefore does not decode it.
**
** If NAME begins with another other than a lower-case letter then
** the entire NAME=VALUE term is ignored.  Hence:
**
**      *  cookies and query parameters that have uppercase names
**         are ignored.
**
**      *  it is impossible for a cookie or query parameter to
**         override the value of an environment variable since
**         environment variables always have uppercase names.
**
** Parameters are separated by the "terminator" character.  Whitespace
** before the NAME is ignored.
**
** The input string "z" is modified but no copies is made.  "z"
** should not be deallocated or changed again after this routine
** returns or it will corrupt the parameter table.
*/
void add_param_list(char *z, int terminator){
  int isQP = terminator=='&';
  while( *z ){
    char *zName;
    char *zValue;
    while( fossil_isspace(*z) ){ z++; }
    zName = z;
    while( *z && *z!='=' && *z!=terminator ){ z++; }
    if( *z=='=' ){
      *z = 0;
      z++;
      zValue = z;
      while( *z && *z!=terminator ){ z++; }
      if( *z ){
        *z = 0;
        z++;
      }
      dehttpize(zValue);
    }else{
      if( *z ){ *z++ = 0; }
      zValue = "";
    }
    if( fossil_islower(zName[0]) ){
      cgi_set_parameter_nocopy(zName, zValue, isQP);
    }
#ifdef FOSSIL_ENABLE_JSON
    json_setenv( zName, cson_value_new_string(zValue,strlen(zValue)) );
#endif /* FOSSIL_ENABLE_JSON */
  }
}

/*
** Add another query parameter or cookie to the parameter set.
** zName is the name of the query parameter or cookie and zValue
** is its fully decoded value.
**
** Copies are made of both the zName and zValue parameters.
*/
void cgi_set_parameter(const char *zName, const char *zValue){
  cgi_set_parameter_nocopy(mprintf("%s",zName), mprintf("%s",zValue), 0);
}

/*
** Replace a parameter with a new value.
*/
void cgi_replace_parameter(const char *zName, const char *zValue){
  int i;
  for(i=0; i<nUsedQP; i++){
    if( strcmp(aParamQP[i].zName,zName)==0 ){
      aParamQP[i].zValue = zValue;
      return;
    }
  }
  cgi_set_parameter_nocopy(zName, zValue, 0);
}
