/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_net.h"

/* Headers files for OpenSSL */

#include <openssl/lhash.h>
#include <openssl/opensslv.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

/* ====================================================================== */

BOOL ssl_is_available(void)
{
    return (T);
}

/* ====================================================================== */

/* Identifier string used by both context */
static unsigned char *sid_ctx = (unsigned char *) "libcez SID";

void *ssl_client_context_init(void)
{
    SSL_CTX *client_ctx;

    SSL_library_init();

    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */

    client_ctx = (void *) SSL_CTX_new(TLSv1_2_client_method());   /* Create new context */
    return ((void *) client_ctx);
}

void ssl_client_context_free(void *client_ctx)
{
    SSL_CTX_free((SSL_CTX *)client_ctx);
}

void ssl_shutdown(void *ssl)
{
    SSL_shutdown((SSL *) ssl);
}

int ssl_get_error(void *ssl, int code)
{
    return (SSL_get_error((SSL *) ssl, code));
}

void ssl_free(void *ssl)
{
    SSL_free((SSL *) ssl);
    ERR_remove_state(0);
}

/* ssl_server_client() ***************************************************
 *
 * Start client side SSL
 ************************************************************************/

void *ssl_start_client(int fd, void *client_ctx, unsigned long timeout)
{
    SSL *ssl;
    const SSL_CIPHER *c;
    char *ver;
    int bits;

    if (!(ssl = (void *) SSL_new((SSL_CTX *)client_ctx)))
        return (NIL);

    SSL_set_session_id_context((SSL *) ssl, sid_ctx,
                               strlen((char *) sid_ctx));

    SSL_set_fd((SSL *) ssl, fd);
    SSL_set_connect_state((SSL *) ssl);

    if (SSL_connect((SSL *) ssl) <= 0)
        return (NIL);

    /* Verify certificate here? Need local context to play with? */

    switch (SSL_version(ssl)) {
    case SSL2_VERSION:
        ver = "SSLv2";
        break;
    case SSL3_VERSION:
        ver = "SSLv3";
        break;
    case TLS1_VERSION:
        ver = "TLSv1";
        break;
    default:
        ver = "UNKNOWN";
    }
    c = SSL_get_current_cipher((SSL *) ssl);
    SSL_CIPHER_get_bits(c, &bits);

    /* Put underlying socket in non-blocking mode: stops occasional
     * deadlocks where select() timeout preferred */
    os_socket_nonblocking(fd);

    return ((void *) ssl);
}

/* ====================================================================== */

/* ssl_read() ************************************************************
 *
 * read() from SSL pipe:
 *    ssl     - SSL abstraction
 *  buffer    - Buffer to read into
 *  blocksize - Size of buffer
 *
 * Returns: Numbers of bytes read. 0 => EOF,
 *          -1 => error (SSL_PRAYER_RETRY or SSL_PRAYER_ERROR)
 ************************************************************************/

int ssl_read(void *ssl, unsigned char *buffer, unsigned long blocksize)
{
    int rc = SSL_read((SSL *) ssl, (char *) buffer, blocksize);

    switch (SSL_get_error((SSL *) ssl, rc)) {
    case SSL_ERROR_NONE:
        return (rc);
    case SSL_ERROR_ZERO_RETURN:
        return (0);
    case SSL_ERROR_WANT_READ:
        return (SSL_PRAYER_RETRY);
    default:
        return (SSL_PRAYER_ERROR);
    }
}

/* ssl_write() ***********************************************************
 *
 * write() to SSL pipe:
 *    ssl  - SSL abstraction
 *  buffer - Buffer to write from
 *  bytes  - Number of bytes to write
 *
 * Returns: Numbers of bytes written. -1 => error
 ************************************************************************/

int ssl_write(void *ssl, unsigned char *buffer, unsigned long bytes)
{
    int rc = SSL_write((SSL *) ssl, (char *) buffer, bytes);

    switch (SSL_get_error((SSL *) ssl, rc)) {
    case SSL_ERROR_NONE:
        return (rc);
    case SSL_ERROR_ZERO_RETURN:
        return (0);
    case SSL_ERROR_WANT_WRITE:
        return (SSL_PRAYER_RETRY);
    default:
        return (SSL_PRAYER_ERROR);
    }
}

/* ssl_pending() *********************************************************
 *
 * Check for pending input on SSL pipe.
 ************************************************************************/

int ssl_pending(void *ssl)
{
    return (SSL_pending((SSL *) ssl));
}
