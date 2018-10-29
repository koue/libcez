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

/*
** cc -c hmac.c
** cc -o program program.c hmac.o -lcrypto
*/

#include <stdio.h>
#include <string.h>
#include <openssl/hmac.h>

#include "cez_misc.h"

/*
** HMAC encrypt string.
*/
void
HMAC_encrypt_me(const char *zSecret, const char *zString, char *zResult,
    size_t zLen){
  unsigned char *result;
  unsigned int len = 20;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX ctx;
#else
  HMAC_CTX *ctx = HMAC_CTX_new();
#endif

  result = (unsigned char*)malloc(sizeof(char) * len);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_init(&ctx);
  HMAC_Init_ex(&ctx, zSecret, strlen(zSecret), EVP_sha1(), NULL);
  HMAC_Update(&ctx, (unsigned char*)zString, strlen(zString));
  HMAC_Final(&ctx, result, &len);
#else
  HMAC_Init_ex(ctx, zSecret, strlen(zSecret), EVP_sha1(), NULL);
  HMAC_Update(ctx, (unsigned char*)zString, strlen(zString));
  HMAC_Final(ctx, result, &len);
#endif

  for (int i = 0; i < len; i++)
    snprintf(&zResult[i*2], zLen, "%02x", (unsigned int)result[i]);
  free(result);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_cleanup(&ctx);
#else
  HMAC_CTX_free(ctx);
#endif
}

/*
** HMAC verify string.
*/
int
HMAC_verify_me(const char *zSecret, const char *zString, const char *zResult){
  char hmac_result[64];
  unsigned char *result;
  unsigned int len = 20;
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX ctx;
#else
  HMAC_CTX *ctx = HMAC_CTX_new();
#endif

  result = (unsigned char *)malloc(sizeof(char) * len);

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_init(&ctx);
  HMAC_Init_ex(&ctx, zSecret, strlen(zSecret), EVP_sha1(), NULL);
  HMAC_Update(&ctx, (unsigned char*)zString, strlen(zString));
  HMAC_Final(&ctx, result, &len);
#else
  HMAC_Init_ex(ctx, zSecret, strlen(zSecret), EVP_sha1(), NULL);
  HMAC_Update(ctx, (unsigned char*)zString, strlen(zString));
  HMAC_Final(ctx, result, &len);
#endif

  for (int i = 0; i < len; i++)
    snprintf(&hmac_result[i*2], sizeof(hmac_result), "%02x",
      (unsigned int)result[i]);
  free(result);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
  HMAC_CTX_cleanup(&ctx);
#else
  HMAC_CTX_free(ctx);
#endif

  return(strncmp(zResult, hmac_result, 40));
}
