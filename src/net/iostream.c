/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Some simple stream IO functions that work with sockets (separate
   read/write buffers) and provide timeout options on read.
   Also supply a (more or less) transparent interface to SSL wrapped streams*/

#include "cez_net.h"

/* iostream_init() *******************************************************
 *
 * Initialiase iostream subsystem.
 ************************************************************************/

//void iostream_init(struct ssl_config *ssl_config)
//{
//    if (ssl_config->ssl_default_port)       /* Configured to use SSL */
//        ssl_context_init(ssl_config);       /* initialize global SSL context */
//}

/* iostream_ssl_client_init() ********************************************
 *
 *
 * Initialiase iostream ssl client subsystem.
 ************************************************************************/

void iostream_ssl_client_init(void)
{
	ssl_client_context_init();	/* Initialize global SSL client context */
}

/* iostream_check_rsakey() ***********************************************
 *
 * Initialiase iostream subsystem.
 ************************************************************************/

//void iostream_check_rsakey(struct ssl_config *ssl_config)
//{
//    ssl_check_rsakey(ssl_config);
//}

/* iostream_freshen_rsakey() *********************************************
 *
 * Initialiase iostream subsystem.
 ************************************************************************/

//void iostream_freshen_rsakey(struct ssl_config *ssl_config)
//{
//    ssl_freshen_rsakey(ssl_config);
//}

/* iostream_exit() *******************************************************
 *
 * Clear up iostream subsystem.
 ************************************************************************/

void iostream_exit()
{
    ssl_client_context_free();         /* free global SSL context */
}

/* ====================================================================== */

/* iostream_create() *****************************************************
 *
 * Create a new iostream and bind it to socket descriptor.
 *       pool: Target pool for this iostream and its buffers
 *     sockfd: Socket decriptor that we want to bind to
 *  blocksize: Size of I/O buffers on this iostream.
 *
 * Returns: New iostream structure
 ************************************************************************/

struct iostream *iostream_create(struct pool *pool, int sockfd,
                                 unsigned long blocksize)
{
    struct iostream *x = pool_alloc(pool, sizeof(struct iostream));

    x->pool = pool;
    x->blocksize = (blocksize) ? blocksize : IOSTREAM_PREFERRED_BLOCK_SIZE;
    x->fd = sockfd;
    x->ssl = NIL;
    x->bio = NIL;

    x->ibuffer = pool_alloc(pool, x->blocksize);
    x->ibufend = x->ibuffer;
    x->icurrent = x->ibuffer;
    x->itimeout = 0;            /* Default: No timeout */
    x->ieof = NIL;

    x->fd = sockfd;
    x->obuffer = pool_alloc(pool, x->blocksize);
    x->obufend = x->obuffer + x->blocksize;
    x->ocurrent = x->obuffer;
    x->otimeout = 0;            /* Default: No timeout */
    x->oerror = NIL;

    return (x);
}

/* ====================================================================== */

/* iostream_ssl_start_server() *******************************************
 *
 * Start up server side SSL on given iostream
 *    x: iostream.
 ************************************************************************/

//BOOL iostream_ssl_start_server(struct iostream * x)
//{
//    if (!(x->ssl = ssl_start_server(x->fd, x->itimeout))) {
//        x->oerror = T;
//        return (NIL);
//    }
//    return (T);
//}

/* ====================================================================== */

/* iostream_ssl_start_client() *******************************************
 *
 * Start up client side SSL on given iostream
 *    x: iostream.
 ************************************************************************/

BOOL iostream_ssl_start_client(struct iostream * x)
{
    if (!(x->ssl = ssl_start_client(x->fd, x->itimeout))) {
        x->oerror = T;
        return (NIL);
    }
    return (T);
}

/* iostream_ssl_enabled() ************************************************
 *
 * Check with SSL enabled on this stream.
 *    x: iostream.
 ************************************************************************/

BOOL iostream_ssl_enabled(struct iostream * x)
{
    return ((x->ssl) ? T : NIL);
}

/* ====================================================================== */

/* iostream_free() *******************************************************
 *
 * Free iostream.
 ************************************************************************/

void iostream_free(struct iostream *x)
{
    if (x->ssl)
        ssl_free(x->ssl);

    if (x->pool)
        return;

    if (x->ibuffer)
        free(x->ibuffer);

    if (x->obuffer)
        free(x->obuffer);

    free(x);
}

/* iostream_free_buffers() ***********************************************
 *
 * Free iostream buffers: switching to raw I/O here
 ************************************************************************/

void iostream_free_buffers(struct iostream *x)
{
    ioflush(x);

    if (x->ibuffer)
        free(x->ibuffer);

    if (x->obuffer)
        free(x->obuffer);

    x->ibuffer = x->obuffer = NIL;
}

/* iostream_close() ******************************************************
 *
 * Close down and free iostream
 ************************************************************************/

void iostream_close(struct iostream *x)
{
    int fd = x->fd;

    if (x->obuffer)
        iostream_flush(x);      /* Flush data in write buffer */

    if (x->ssl)
        ssl_shutdown(x->ssl);

    iostream_free(x);
    close(fd);
}

/* iostream_set_timeout() ************************************************
 *
 * Set timeout on this iostream
 *         x: iostream
 *   timeout: timeout in seconds. 0 => disable timeout
 ************************************************************************/

void iostream_set_timeout(struct iostream *x, time_t timeout)
{
    x->itimeout = timeout;
    x->otimeout = timeout;
}

/* ====================================================================== */

/* Static Support functions for iostream_getchar(). */

/* NB: Underlying socket is typically running in non-blocking mode, I/O
 * must be protected by select to provide reliable data and timeouts */

/* iostream_read_wait() **************************************************
 *
 * Wait until unlying socket has data or iostream timeout reached.
 * Returns:   T => Okay to write
 *          NIL => Timeout
 ************************************************************************/

static int iostream_read_wait(struct iostream *x)
{
    fd_set readfds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(x->fd, &readfds);

    if (x->itimeout == 0) {
        /* Wait indefinitely */
        while (select(x->fd + 1, &readfds, NIL, NIL, NIL) < 0) {
            if (errno != EINTR)
                fprintf(stderr, "iostream_getchar(): select() failed: %s",
                          strerror(errno));
		exit(1);
        }
        return ((FD_ISSET(x->fd, &readfds)) ? T : NIL);
    }

    timeout.tv_sec = x->itimeout;
    timeout.tv_usec = 0;

    while (select(x->fd + 1, &readfds, NIL, NIL, &timeout) < 0) {
        if (errno != EINTR)
            fprintf(stderr, "iostream_getchar(): select() failed: %s",
                      strerror(errno));
            exit(1);
    }
    return ((FD_ISSET(x->fd, &readfds)) ? T : NIL);
}

/* iostream_read_ssl() **************************************************
 *
 * Read block of data from iostream, minimum one byte, max, x->blocksize.
 * Convolutions are to protect non blocking socket with minimum number of
 * wasted system calls. iostream_getchar() only wants to deal with
 * three conditions: data ready, EOF and error (treated as EOF + error bit)
 *
 * Returns: Number of bytes read (0 < count) && (count <= x->blocksize)
 *           0 on EOF or timeout
 *          -1 on error (errno not set, need to use SSL_get_error)
 ***********************************************************************/

static int iostream_read_ssl(struct iostream *x)
{
    int len;

    /* Check for data in SSL buffer first */
    if (ssl_pending(x->ssl)) {
        do {
            len = ssl_read(x->ssl, x->ibuffer, x->blocksize);
        }
        while (len == SSL_PRAYER_RETRY);

        return ((len >= 0) ? len : (-1));       /* Aggregate error conditions */
    }

    do {
        if (!iostream_read_wait(x))
            return (0);         /* Timeout */

        len = ssl_read(x->ssl, x->ibuffer, x->blocksize);
    }
    while (len == SSL_PRAYER_RETRY);

    return ((len >= 0) ? len : (-1));   /* Aggregate error conditions */
}

/* iostream_read_nossl() ************************************************
 *
 * Read block of data from iostream, minimum one byte, max, x->blocksize.
 * Convolutions are to protect non blocking socket with minimum number of
 * wasted system calls. iostream_getchar() only wants to deal with
 * three conditions: data ready, EOF and error (treated as EOF + error bit)
 *
 * Returns: Number of bytes read (0 < count) && (count <= x->blocksize)
 *           0 on EOF or timeout
 *          -1 on error (errno set)
 ***********************************************************************/


static int iostream_read_nossl(struct iostream *x)
{
    int len;

    do {
        if (!iostream_read_wait(x))
            return (0);         /* Timeout */

        do {
            len = read(x->fd, x->ibuffer, x->blocksize);
        }
        while ((len < 0) && (errno == EINTR));

    }
    while ((len < 0) && (errno == EAGAIN));

    return ((len >= 0) ? len : (-1));   /* Aggregate error conditions */
}

/* ====================================================================== */

/* iostream_getchar() ****************************************************
 *
 * Get a single character from I/O stream. Normally called from via
 * iogetc() macro.
 *     x: IOstream
 *
 * Returns: unsigned character.
 *          EOF  => end of file reached
 ************************************************************************/

int iostream_getchar(struct iostream *x)
{
    int len;

    if (x->ieof)
        return (EOF);

    if (x->icurrent < x->ibufend)
        return (*(x->icurrent++));

    if (x->ssl)
        len = iostream_read_ssl(x);
    else
        len = iostream_read_nossl(x);

    if (len > 0) {
        x->ibufend = x->ibuffer + len;
        x->icurrent = x->ibuffer;
        return (*(x->icurrent++));
    }

    x->ieof = T;
    return (EOF);
}

/* ====================================================================== */

/* iostream_ungetchar() **************************************************
 *
 * Unget character from stream. Normally called via ioungetc() macro.
 ************************************************************************/

void iostream_ungetchar(char c, struct iostream *x)
{
    *(--x->icurrent) = c;
}

/* ====================================================================== */

/* Check whether more input pending on this iostream */

/* iostream_have_buffered_input() ****************************************
 *
 * Check for input sitting in iostream or SSL buffers
 *                  x: iostream
 * ignore_white_space: Only trigger if non-whitespace characters queued.
 *
 * Returns: T => have buffered input
 ************************************************************************/

BOOL
iostream_have_buffered_input(struct iostream *x, BOOL ignore_whitespace)
{
    unsigned char *s;

    if (x->icurrent < x->ibufend) {
        if (!ignore_whitespace)
            return (T);

        for (s = x->icurrent; s < x->ibufend; s++) {
            if ((*s != '\015') && (*s != '\012'))
                return (T);
        }
    }

    if (x->ssl && ssl_pending(x->ssl))
        return (T);

    return (NIL);
}

/* iostream_have_buffered_input() ****************************************
 *
 * Check for input sitting in iostream or SSL buffers or pending on socket
 *    x: iostream
 *
 * Returns: T => have buffered input or read() can return without blocking
 ************************************************************************/

BOOL iostream_have_input(struct iostream * x)
{
    fd_set readfds;
    struct timeval timeout;

    if (x->icurrent < x->ibufend) {
        unsigned char *s;

        for (s = x->icurrent; s < x->ibufend; s++) {
            if ((*s != '\015') && (*s != '\012'))
                return (T);
        }
    }

    if (x->ssl && ssl_pending(x->ssl))
        return (T);

    FD_ZERO(&readfds);
    FD_SET(x->fd, &readfds);

    /* Poll for pending input on readfds */
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    while (select(x->fd + 1, &readfds, NIL, NIL, &timeout) < 0) {
        if (errno != EINTR)
            fprintf(stderr, "iostream_getchar(): select() failed");
	    exit(1);
    }

    if (FD_ISSET(x->fd, &readfds))
        return (T);

    return (NIL);
}

/* iostream_is_eof() *****************************************************
 *
 * Check whether iostream has reached EOF on input.
 ************************************************************************/

BOOL iostream_is_eof(struct iostream * x)
{
    return ((x->ieof) ? T : NIL);
}

/* ====================================================================== */

/* Static Support functions for iostream_flush() */

/* NB: Underlying socket is typically running in non-blocking mode, I/O
 * must be protected by select to provide reliable data and timeouts */

/* iostream_write_wait() *************************************************
 *
 * Wait until socket is ready for nonblocking write or iostream timeout
 * reached.
 *
 * Returns:   T => Okay to write
 *          NIL => Timeout
 ************************************************************************/

static int iostream_write_wait(struct iostream *x)
{
    fd_set writefds;
    struct timeval timeout;

    FD_ZERO(&writefds);
    FD_SET(x->fd, &writefds);

    if (x->otimeout == 0) {
        /* Wait indefinitely */
        while (select(x->fd + 1, NIL, &writefds, NIL, NIL) < 0) {
            if (errno != EINTR)
                fprintf(stderr, "iostream_write_wait(): select() failed: %s",
                          strerror(errno));
		exit(1);
        }
        return ((FD_ISSET(x->fd, &writefds)) ? T : NIL);
    }

    timeout.tv_sec = x->otimeout;
    timeout.tv_usec = 0;

    while (select(x->fd + 1, NIL, &writefds, NIL, &timeout) < 0) {
        if (errno != EINTR)
            fprintf(stderr, "iostream_write_wait(): select() failed: %s",
                      strerror(errno));
            exit(1);
    }
    return ((FD_ISSET(x->fd, &writefds)) ? T : NIL);
}

/* iostream_write_ssl() **************************************************
 *
 * Flush buffered data to iostream,
 *
 * Returns: T on success, NIL on error
 *          
 ***********************************************************************/

static BOOL iostream_write_ssl(struct iostream *x)
{
    unsigned char *current = x->obuffer;
    int bytes = x->ocurrent - x->obuffer;
    int rc;

    while (bytes > 0) {
        if (!iostream_write_wait(x))
            break;              /* Timeout */

        rc = ssl_write(x->ssl, current, bytes);

        if (rc == SSL_PRAYER_RETRY)
            continue;
        if (rc <= 0)
            break;

        current += rc;
        bytes -= rc;
    }

    x->ocurrent = x->obuffer;   /* Clear buffer */

    return ((bytes == 0) ? T : NIL);
}

/* iostream_write_nossl() ***********************************************
 *
 * Flush buffered data to iostream,
 *
 * Returns: T on success, NIL on error
 *          
 ***********************************************************************/

static int iostream_write_nossl(struct iostream *x)
{
    unsigned char *current = x->obuffer;
    int bytes = x->ocurrent - x->obuffer;
    int rc;

    while (bytes > 0) {
        if (!iostream_write_wait(x))
            break;              /* Timeout */

        do {
            rc = write(x->fd, current, bytes);
        }
        while ((rc < 0) && (errno == EINTR));

        if ((rc < 0) && (errno == EAGAIN))
            continue;

        if (rc <= 0)
            break;

        current += rc;
        bytes -= rc;
    }

    x->ocurrent = x->obuffer;   /* Clear buffer */

    return ((bytes == 0) ? T : NIL);
}

/* iostream_flush() ******************************************************
 *
 * Flush buffered data to iostream.
 ************************************************************************/

BOOL iostream_flush(struct iostream * x)
{
    int rc;

    if (x->oerror)              /* Blackhole all output after error */
        return (NIL);

    if (x->obuffer == NIL)      /* No output buffer on stream */
        return (T);

    if (x->obuffer == x->ocurrent)      /* NOOP */
        return (T);

    if (x->ssl)
        rc = iostream_write_ssl(x);
    else
        rc = iostream_write_nossl(x);

    if (!rc)
        x->oerror = T;

    return (rc);
}

/* ====================================================================== */

/* iostream_putchar() ****************************************************
 *
 * Push character through iostream output buffer. Normally called via
 * ioputc macro.
 *      c: Character to push
 *      x: IOstream
 ************************************************************************/

BOOL iostream_putchar(char c, struct iostream * x)
{

    if ((x->ocurrent == x->obufend) && !iostream_flush(x))
        return (NIL);           /* Flush failed */

    *(x->ocurrent++) = c;
    return (T);
}

/* iostream_puts() *******************************************************
 *
 * Push string through iostream output buffer. Normally called via
 * ioputs macro.
 *      x: IOstream
 *      s: String to print
 ************************************************************************/

BOOL iostream_puts(struct iostream * x, unsigned char *s)
{
    unsigned char c;

    if (!s)
        ioputs(x, "(nil)");
    else
        while ((c = *s++))
            ioputc(c, x);

    return (T);
}

/* Static support routine for iostream_printf */

static void iostream_print_ulong(struct iostream *x, unsigned long value)
{
    unsigned long tmp, weight;

    /* All numbers contain at least one digit.
     * Find weight of most significant digit. */
    for (weight = 1, tmp = value / 10; tmp > 0; tmp /= 10)
        weight *= 10;

    for (tmp = value; weight > 0; weight /= 10) {
        if (value >= weight) {  /* Strictly speaking redundant... */
            ioputc('0' + (value / weight), x);  /* Digit other than zero */
            value -= weight * (value / weight); /* Calculate remainder */
        } else
            ioputc('0', x);
    }
}

/* iostream_printf() *****************************************************
 *
 * Print string through iostream output buffer. Normally called via
 * ioprintf macro.
 *      x: IOstream
 *    fmt: String to print, followed by arguments.
 ************************************************************************/

BOOL iostream_printf(struct iostream *x, char *fmt, ...)
{
    va_list ap;
    char *s;
    char c;

    va_start(ap, fmt);

    while ((c = *fmt++)) {
        if (c != '%') {
            ioputc(c, x);
        } else
            switch (*fmt++) {
            case 's':          /* string */
                if ((s = va_arg(ap, char *))) {
                    while ((c = *s++))
                        ioputc(c, x);
                } else
                    ioprintf(x, "(nil)");
                break;
            case 'l':
                if (*fmt == 'u') {
                    iostream_print_ulong(x, va_arg(ap, unsigned long));
                    fmt++;
                } else
                    iostream_print_ulong(x, va_arg(ap, long));
                break;
            case 'd':
                if (*fmt == 'u') {
                    iostream_print_ulong(x, va_arg(ap, unsigned int));
                    fmt++;
                } else
                    iostream_print_ulong(x, va_arg(ap, int));
                break;
            case 'c':
                ioputc((char) va_arg(ap, int), x);
                break;
            case '%':
                ioputc('%', x);
                break;
            default:
                fprintf(stderr, "Bad format string to iostream_printf");
		exit(1);
            }
    }
    va_end(ap);

    return (T);
}

/* ====================================================================== */

/* iostream_getline() ****************************************************
 *
 * Get line from iostream with known upper bound. Most client routines
 * dealing with arbitary amounts of data won't use this, instead they
 * copy data to a temporary buffer and buffer_fetch() from that.
 *    stream: iostream
 *         s: Target buffer
 *    length: Size of target buffer.
 ************************************************************************/

BOOL iostream_getline(struct iostream * stream, char *s, int length)
{
    int c = EOF;

    length--;                   /* Leave space for trailing '\0' */

    while ((length > 0) && ((c = iogetc(stream)) != EOF)) {
        if (c == '\015')
            continue;

        if (c == '\012')
            break;

        *s++ = c;
        length--;
    }

    *s = '\0';

    return (((c != EOF) && (length > 0)) ? T : NIL);
}

/* iostream_getline_overflow() ********************************************
 *
 * Get line from iostream with known upper bound. Most client routines
 * dealing with arbitary amounts of data won't use this, instead they
 * copy data to a temporary buffer and buffer_fetch() from that.
 *    stream: iostream
 *         s: Target buffer
 *    length: Size of target buffer.
 *   toolong: Input line was too long
 ************************************************************************/

BOOL
iostream_getline_overflow(struct iostream * stream,
                          char *s, int length, BOOL * overflowp)
{
    int c = EOF;

    length--;                   /* Leave space for trailing '\0' */

    while ((length > 0) && ((c = iogetc(stream)) != EOF)) {
        if (c == '\015')
            continue;

        if (c == '\012')
            break;

        *s++ = c;
        length--;
    }

    *s = '\0';

    if (length == 0) {
        if (overflowp)
            *overflowp = T;
        return (NIL);
    }

    if (overflowp)
        *overflowp = NIL;
    return ((c != EOF) ? T : NIL);
}
