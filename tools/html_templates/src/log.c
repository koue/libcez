/* Copyright (c) 2018 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

void
log_fatal(char *err, ...)
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

void
log_misc(char *err, ...)
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

void
log_debug(char *err, ...)
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

void
log_panic(char *err, ...)
{
    fprintf(stderr, "%s\n", err);
    exit(1);
}

