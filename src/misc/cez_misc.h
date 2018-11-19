/*
** Copyright (c) 2017-2018 Nikola Kolev <koue@chaosophia.net>
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of the Simplified BSD License (also
** known as the "2-Clause License" or "FreeBSD License".)

** This program is distributed in the hope that it will be useful,
** but without any warranty; without even the implied warranty of
** merchantability or fitness for a particular purpose.
**
*******************************************************************************
*/

#ifndef CEZ_MISC_H
#define CEZ_MISC_H

#include <sys/time.h>

/*
** HMAC
*/
#define CEZ_HMAC_INIT "01234567890123456789012345678901234567890123456"
void HMAC_encrypt_me(const char *zSecret, const char *zString, char *zResult,
    size_t zLen);
int HMAC_verify_me(const char *zSecret, const char *zString,
    const char *zResult);

/*
** TIME
*/
double timelapse(struct timeval *t);
const char *rfc822_time(time_t t);
time_t convert_rfc822_time(const char *date);

#endif
