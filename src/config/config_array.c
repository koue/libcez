/*
 * Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
 * Copyright (c) 2006 D. Richard Hipp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cez_config.h"

static int nAllocQP = 0;        /* Space allocated for aParamQP[] */
static int nUsedQP = 0;         /* Space actually used in aParamQP[] */
static int sortQP = 0;          /* True if aParamQP[] needs sorting */
static int seqQP = 0;           /* Sequence numbers */
static struct QParam {		/* One entry for eachquery parameter or cookie */
	char *zName;	/* Parameter or cookie name */
	char *zValue;	/* Value of the query parameter or cookie */
	int seq;		/* Order of insertion */
} *aParamQP;			/* An array of all parameters and cookies */

void config_array_cb(const char *zName, const char *zValue){
	if( nAllocQP<=nUsedQP ){
		nAllocQP = nAllocQP*2 + 10;
		if( nAllocQP>1000 ){
			/* Prevent a DOS service attack against the framework */
			fprintf(stderr, "Too many query parameters");
			exit(1);
		}
		aParamQP = realloc( aParamQP, nAllocQP*sizeof(aParamQP[0]) );
	}
	aParamQP[nUsedQP].zName = strdup(zName);
	aParamQP[nUsedQP].zValue = strdup(zValue);
	aParamQP[nUsedQP].seq = seqQP++;
	nUsedQP++;
	sortQP = 1;
}

void config_array_print(void){
	for(int i = 0; i < nUsedQP; i++){
		printf("name=%s, value=%s\n", aParamQP[i].zName, aParamQP[i].zValue);
	}
}

void config_array_purge(void){
	for(int i = 0; i < nUsedQP; i++){
		free(aParamQP[i].zName);
		free(aParamQP[i].zValue);
	}
	free(aParamQP);
}

/* This is the comparison function used to sort the aParamQP[] array */
int qparam_compare(const void *a, const void *b){
	struct QParam *pA = (struct QParam*)a;
	struct QParam *pB = (struct QParam*)b;
	int c;
	c = strcmp(pA->zName, pB->zName);
	if( c==0 ){
		c = pA->seq - pB->seq;
	}
	return (c);
}

char *config_array_value_get(const char *zName){
	int lo, hi, mid, c;
	/* The sortQP flag is set whenever a new parameter is inserted.
	** It indicates that we need to resort the parameters
	*/
	if(sortQP){
		int i, j;
		qsort(aParamQP, nUsedQP, sizeof(aParamQP[0]), qparam_compare);
		sortQP = 0;
		/* After sorting, remove duplicate parameters. */
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
			return aParamQP[mid].zValue;
		}else if( c>0 ){
			hi = mid-1;
		}else{
			lo = mid+1;
		}
	}
	return (NULL);
}

const char *
config_array_check(const char **params)
{
	for (int i = 0; params[i] != NULL; ++i) {
		if (config_array_value_get(params[i]) == NULL) {
			return (params[i]);
		}
	}
	return (NULL);
}
