/* $Cambridge: hermes/src/prayer/lib/no_ssl.c,v 1.3 2008/09/16 09:59:57 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Stub routines if SSL not defined */

BOOL ssl_is_available()
{
    return (NIL);
}

void ssl_check_rsakey(struct ssl_config *ssl_config)
{
}

void ssl_freshen_rsakey(struct ssl_config *ssl_config)
{
}

void ssl_context_init(struct ssl_config *ssl_config)
{
}

void ssl_context_free()
{
}

void ssl_shutdown(void *ssl)
{
}

int ssl_get_error(void *ssl, int code)
{
    return (0);
}

void ssl_free(void *ssl)
{
}

void *ssl_start_server(int fd, unsigned long timeout)
{
    return (NIL);
}

void *ssl_start_client(int fd, unsigned long timeout)
{
    return (NIL);
}

int ssl_read(void *ssl, unsigned char *buffer, unsigned long blocksize)
{
    return (-1);
}

int ssl_write(void *ssl, unsigned char *buffer, unsigned long bytes)
{
    return (-1);
}

int ssl_pending(void *ssl)
{
    return (0);
}

int ssl_prune_sessions(struct ssl_config *ssl_config)
{
    return (NIL);
}

void ssl_client_context_init(void)
{
}

void ssl_client_context_free(void)
{
}
