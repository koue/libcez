//
/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"
//#ifdef SESSION_CACHE_ENABLE
//#include "mydb.h"
//#endif

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

/* Assorted bits stolen straight from Stunnel (ssl.c) that we might need */

/* Global SSL context shared by server iostreams */
//static SSL_CTX *server_ctx;

/* Global SSL context shared by client iostreams */
static SSL_CTX *client_ctx;

/* Identifier string used by both context */
static unsigned char *sid_ctx = (unsigned char *) "libcez SID";

/* Enable full logging? */
//static BOOL ssl_verbose_logging = NIL;

/* ====================================================================== */

/* PRNG stuff for SSL */

/* shortcut to determine if sufficient entropy for PRNG is present */
//static int prng_seeded(int bytes)
//{
//    if (RAND_status()) {
//        log_misc("RAND_status claims sufficient entropy for the PRNG\n");
//        return (1);
//    }
//    return (0);                 /* assume we don't have enough */
//}
//
//static int add_rand_file(char *filename)
//{
//    int readbytes;
//    struct stat sb;
//
//    if (stat(filename, &sb) != 0) {
//        return (0);
//    }
//
//    if ((readbytes = RAND_load_file(filename, 2048))) {
//        log_misc("Snagged %lu random bytes from %s\n",
//                 (unsigned long) readbytes, filename);
//    } else {
//        log_misc("Unable to retrieve any random data from %s\n", filename);
//    }
//    return (readbytes);
//}
//
//static void os_initialize_prng(struct ssl_config *ssl_config)
//{
//    int totbytes = 0;
//
//    /* Try the good-old default /dev/urandom, if available  */
//    totbytes += add_rand_file("/dev/urandom");
//    if (prng_seeded(totbytes)) {
//        goto SEEDED;
//    }
//
//    /* Random file specified during configure */
//
//    log_fatal("PRNG seeded with %lu bytes total (insufficent)\n",
//              (unsigned long) totbytes);
//    exit(1);
//
//  SEEDED:
//    log_misc("PRNG seeded successfully\n");
//    return;
//}

/* ====================================================================== */

//static struct ssl_config *rsa_ssl_config = NIL; /* Configuration        */
//static RSA *rsa_tmp = NIL;      /* temporary RSA key    */
//static time_t rsa_timeout = (time_t) 0; /* Timeout for this key */

/* ssl_make_rsakey() *****************************************************
 *
 * Set up RSAkey
 ************************************************************************/

//static void ssl_make_rsakey(struct ssl_config *ssl_config)
//{
//    log_misc("Generating fresh RSA key");
//
//    if (rsa_tmp)
//        RSA_free(rsa_tmp);
//
//    if (!
//        (rsa_tmp =
//         RSA_generate_key(SSL_RSA_KEYLENGTH, RSA_F4, NULL, NULL)))
//        log_fatal("tmp_rsa_cb");
//
//    log_misc("Generated fresh RSA key");
//
//    if (ssl_config->ssl_rsakey_lifespan) {
//        time_t now = time(NIL);
//        rsa_timeout = now + ssl_config->ssl_rsakey_lifespan;
//    } else
//        rsa_timeout = 0;
//}

/* ssl_init_rsakey() *****************************************************
 *
 * Initialise RSAkey stuff
 ************************************************************************/

//static void ssl_init_rsakey(struct ssl_config *ssl_config)
//{
//    ssl_make_rsakey(ssl_config);
//    rsa_ssl_config = ssl_config;
//}

/* ssl_freshen_rsakey() ***************************************************
 *
 * Extend life of RSA key (unless its already expired)
 *************************************************************************/

//void ssl_freshen_rsakey(struct ssl_config *ssl_config)
//{
//    time_t now = time(NIL);
//
//    if (rsa_tmp && (rsa_timeout != (time_t) 0L) && (rsa_timeout < now))
//        rsa_timeout = now + ssl_config->ssl_rsakey_freshen;
//}

/* ssl_check_rsakey() *****************************************************
 *
 * Generate fresh RSAkey if existing key has expired.
 *************************************************************************/

//void ssl_check_rsakey(struct ssl_config *ssl_config)
//{
//    time_t now = time(NIL);
//
//    if (rsa_tmp && (rsa_timeout != (time_t) 0L) && (rsa_timeout < now))
//        ssl_make_rsakey(ssl_config);
//}

/* ====================================================================== */

/* A pair of OpenSSL callbacks */

//static void info_callback(const SSL * s, int where, int ret)
//{
//}

/* ====================================================================== */

//#ifdef SESSION_CACHE_ENABLE
/* SSL Session database, stolen from Cyrus */

//#define DB (&mydb_db3_nosync)

//static struct db *sessdb = NULL;
//static int sess_dbopen = 0;

/*
 * The new_session_cb() is called, whenever a new session has been
 * negotiated and session caching is enabled.  We save the session in
 * a database so that we can share sessions between processes.
 */
//static int new_session_cb(SSL * ssl, SSL_SESSION * sess)
//{
//    int len;
//    unsigned char *data = NULL, *asn;
//    time_t expire;
//    int ret = -1;
//    unsigned int session_id_length;
//    unsigned char *session_id = SSL_SESSION_get_id(sess, &session_id_length);
//
//    if (!sess_dbopen)
//        return 0;
//
//    /* find the size of the ASN1 representation of the session */
//    len = i2d_SSL_SESSION(sess, NULL);
//
//    /*
//     * create the data buffer.  the data is stored as:
//     * <expire time><ASN1 data>
//     */
//    data = (unsigned char *)
//        pool_alloc(NIL, sizeof(time_t) + len * sizeof(unsigned char));

    /* transform the session into its ASN1 representation */
//    if (data) {
//        asn = data + sizeof(time_t);
//        len = i2d_SSL_SESSION(sess, &asn);
//        if (!len)
//           log_panic("i2d_SSL_SESSION failed");
//    }

    /* set the expire time for the external cache, and prepend it to data */
//    expire = SSL_SESSION_get_time(sess) + SSL_SESSION_get_timeout(sess);
//    memcpy(data, &expire, sizeof(time_t));

//    if (data && len) {
        /* store the session in our database */
//        do {
//	    ret = DB->store(sessdb, (void *) session_id, session_id_length,
//                           (void *) data, len + sizeof(time_t), NULL);
//        }
//        while (ret == MYDB_AGAIN);
//    }

//    if (data)
//        free(data);
//
//    /* log this transaction */
//    if (ssl_verbose_logging) {
//        int i;
//        char idstr[SSL_MAX_SSL_SESSION_ID_LENGTH * 2 + 1];
//	for (i = 0; i < session_id_length; i++)
//	    sprintf(idstr + i * 2, "%02X", session_id[i]);
//
//        log_debug("new SSL session: id=%s, expire=%s, status=%s",
//                  idstr, ctime(&expire), ret ? "failed" : "ok");
//    }
//    return (ret == 0);
//}

/*
 * Function for removing session from our database.
 */
//static void remove_session(unsigned char *id, int idlen)
//{
//    int ret;
//
//    if (!sess_dbopen)
//        return;
//
//    do {
//        ret = DB->delete(sessdb, (void *) id, idlen, NULL, 1);
//    }
//    while (ret == MYDB_AGAIN);
//
//    /* log this transaction */
//    if (ssl_verbose_logging) {
//        int i;
//        char idstr[SSL_MAX_SSL_SESSION_ID_LENGTH * 2 + 1];
//        for (i = 0; i < idlen; i++)
//            sprintf(idstr + i * 2, "%02X", id[i]);
//
//        log_debug("remove SSL session: id=%s", idstr);
//    }
//}

/*
 * The remove_session_cb() is called, whenever the SSL engine removes
 * a session from the internal cache. This happens if the session is
 * removed because it is expired or when a connection was not shutdown
 * cleanly.
 */
//static void remove_session_cb(SSL_CTX * ctx, SSL_SESSION * sess)
//{
//	unsigned int session_id_length;
//	unsigned char *session_id = SSL_SESSION_get_id(sess, &session_id_length);
//
//	remove_session(session_id, session_id_length);
//}

/*
 * The get_session_cb() is only called on SSL/TLS servers with the
 * session id proposed by the client. The get_session_cb() is always
 * called, also when session caching was disabled.  We lookup the
 * session in our database in case it was stored by another process.
 */
//static SSL_SESSION *get_session_cb(SSL * ssl, unsigned char *id, int idlen,
//                                   int *copy)
//{
//    int ret;
//    const char *data = NULL;
//    unsigned char *asn;
//    int len = 0;
//    time_t expire = 0, now = time(0);
//    SSL_SESSION *sess = NULL;
//
//    if (!sess_dbopen)
//        return NULL;
//
//    do {
//        ret =
//            DB->fetch(sessdb, (void *) id, idlen, (void *) &data, &len,
//                      NULL);
//    }
//    while (ret == MYDB_AGAIN);
//
//    if (data) {
//        /* grab the expire time */
//        memcpy(&expire, data, sizeof(time_t));
//
//        /* check if the session has expired */
//        if (expire < now) {
//            remove_session(id, idlen);
//        } else {
//            /* transform the ASN1 representation of the session
//               into an SSL_SESSION object */
//            asn = (unsigned char *) data + sizeof(time_t);
//            sess = d2i_SSL_SESSION(NULL, &asn, len - sizeof(time_t));
//            if (!sess)
//                log_panic("d2i_SSL_SESSION failed");
//        }
//    }
//
//    /* log this transaction */
//    if (ssl_verbose_logging) {
//        int i;
//        char idstr[SSL_MAX_SSL_SESSION_ID_LENGTH * 2 + 1];
//        for (i = 0; i < idlen; i++)
//            sprintf(idstr + i * 2, "%02X", id[i]);
//
//        log_debug("get SSL session: id=%s, expire=%s, status=%s",
//                  idstr, ctime(&expire),
//                  !data ? "not found" : expire < now ? "expired" : "ok");
//    }
//
//    *copy = 0;
//    return sess;
//}
//#endif

/* ====================================================================== */

/* ssl_context_init() ****************************************************
 *
 * Initialise SSL "context"es: one for server size activity and one for
 * client side activity.
 ************************************************************************/

//void ssl_context_init(struct ssl_config *ssl_config)
//{
//    char *ssl_cipher_list = SSLCIPHERLIST_DEFAULT;
//
//    /* Set cipherlist */
//    if (ssl_config->ssl_cipher_list && ssl_config->ssl_cipher_list[0])
//        ssl_cipher_list = ssl_config->ssl_cipher_list;
//
//    /* Set up random number generator */
//    os_initialize_prng(ssl_config);
//
//    /* Set up debug flag */
//    ssl_verbose_logging = ssl_config->log_debug;
//
//    SSLeay_add_ssl_algorithms();
//    SSL_load_error_strings();
//
//    /* Set up client context: only used by accountd */
//#if OPENSSL_VERSION_NUMBER < 0x10100000L
//    client_ctx = SSL_CTX_new(SSLv3_client_method());
//#else
//    client_ctx = SSL_CTX_new(TLS_client_method());
//#endif
//    SSL_CTX_set_session_cache_mode(client_ctx, SSL_SESS_CACHE_BOTH);
//    SSL_CTX_set_info_callback(client_ctx, info_callback);
//#ifdef SSL_MODE_AUTO_RETRY
//    SSL_CTX_set_mode(client_ctx, SSL_MODE_AUTO_RETRY);
//#endif
//
//    /* Don't bother with session cache for client side: not enough
//     * connections to worry about caching */
//    SSL_CTX_set_session_cache_mode(client_ctx, SSL_SESS_CACHE_OFF);
//    SSL_CTX_set_timeout(client_ctx, 0);
//
//    if (!SSL_CTX_set_cipher_list(client_ctx, ssl_cipher_list))
//        log_fatal("SSL_CTX_set_cipher_list");
//
//
//    /* Set up server context */
//    server_ctx = SSL_CTX_new(SSLv23_server_method());
//
//    /* Enable all (sensible) bug fixes. A few others are not recommended */
//    /* See ssl.h for details */
//    SSL_CTX_set_options(server_ctx, SSL_OP_ALL);
//#if 0
//    /* Following appears to break Netscape? */
//    SSL_CTX_set_options(server_ctx,
//                        SSL_OP_NETSCAPE_DEMO_CIPHER_CHANGE_BUG);
//#endif
//
//    /* SSLv2 now obsolete */
//    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);
//
//    /* Start off with the session cache disabled */
//    SSL_CTX_set_session_cache_mode(server_ctx, SSL_SESS_CACHE_OFF);
//    SSL_CTX_sess_set_cache_size(server_ctx, 0);
//    SSL_CTX_set_timeout(server_ctx, 0);
//
//    SSL_CTX_set_info_callback(server_ctx, info_callback);
//#ifdef SSL_MODE_AUTO_RETRY
//    SSL_CTX_set_mode(server_ctx, SSL_MODE_AUTO_RETRY);
//#endif
//    SSL_CTX_set_quiet_shutdown(server_ctx, 1);
//
//#ifdef SESSION_CACHE_ENABLE
//    if (ssl_config->ssl_session_timeout > 0) {
//        int r;
//
//        /* Set the callback functions for the external session cache */
//        SSL_CTX_set_session_cache_mode(server_ctx, SSL_SESS_CACHE_BOTH);
//        SSL_CTX_sess_set_cache_size(server_ctx, 128);
//        SSL_CTX_set_timeout(server_ctx, ssl_config->ssl_session_timeout);
//
//        /* Initialise the session cache */
//        /* Initialize DB environment */
//        r = DB->init(ssl_config->ssl_session_dir, 0);
//        if (r != 0)
//            log_fatal("DBERROR init: %s", mydb_strerror(r));
//
//        /* create the name of the db file */
//        r = DB->open("sessions.db", &sessdb);
//        if (r != 0)
//            log_fatal("DBERROR: opening %s: %s",
//                      "sessions.db", mydb_strerror(r));
//        sess_dbopen = 1;
//
//        /* Set the callback functions for the external session cache */
//        SSL_CTX_sess_set_new_cb(server_ctx, new_session_cb);
//        SSL_CTX_sess_set_remove_cb(server_ctx, remove_session_cb);
//        SSL_CTX_sess_set_get_cb(server_ctx, get_session_cb);
//    }
//#endif
//
//    /* Set up DH file, if required */
//    if (ssl_config->ssl_dh_file) {
//        static DH *dh = NULL;
//        BIO *bio = NULL;
//
//        if (!(bio = BIO_new_file(ssl_config->ssl_dh_file, "r")))
//            log_fatal("Error reading DH file: %s\n", ssl_config->ssl_dh_file);
//
//        if (!(dh = PEM_read_bio_DHparams(bio, NULL, NULL, NULL)))
//            log_fatal("Could not load DH parameters from: %s\n",
//                      ssl_config->ssl_dh_file);
//
//        SSL_CTX_set_tmp_dh(server_ctx, dh);
//        if (bio)
//            BIO_free(bio);
//        if (dh)
//            DH_free(dh);
//    }
//
//    /* Set up certificate chain */
//    if (!SSL_CTX_use_certificate_chain_file
//        (server_ctx, ssl_config->ssl_cert_file))
//        log_fatal("Error reading certificate chains from file: %s\n",
//                  ssl_config->ssl_cert_file);
//
//    /* Set up PrivateKey file */
//    if (!SSL_CTX_use_PrivateKey_file
//        (server_ctx, ssl_config->ssl_privatekey_file, SSL_FILETYPE_PEM))
//        log_fatal
//            ("SSL_CTX_use_RSAPrivateKey_file: failed to use file %s\n",
//             ssl_config->ssl_privatekey_file);
//
//    /* Set cipherlist */
//    if (!SSL_CTX_set_cipher_list(server_ctx, ssl_cipher_list))
//        log_fatal("SSL_CTX_set_cipher_list() failed");
//
//    /* Make server rather than client pick preferred cipher */
//    if (ssl_config->ssl_server_preference &&
//        !SSL_CTX_set_options(server_ctx, SSL_OP_CIPHER_SERVER_PREFERENCE))
//        log_fatal("SSL_CTX_set_options(SSL_OP_CIPHER_SERVER_PREFERENCE)"
//                  "failed");
//
//    /* Initialise RSA temporary key (will take a couple of secs to complete) */
//    ssl_init_rsakey(ssl_config);
//}
//
//void ssl_context_free()
//{
//    SSL_CTX_free(server_ctx);
//}

void ssl_client_context_init(void)
{
    SSL_library_init();

    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */

    client_ctx = SSL_CTX_new(TLSv1_2_client_method());   /* Create new context */
//    SSL_CTX_set_session_cache_mode(client_ctx, SSL_SESS_CACHE_BOTH);
//    SSL_CTX_set_info_callback(client_ctx, info_callback);
//    #ifdef SSL_MODE_AUTO_RETRY
//        SSL_CTX_set_mode(client_ctx, SSL_MODE_AUTO_RETRY);
//    #endif

//    if (SSL_CTX_need_tmp_RSA(client_ctx))
//        SSL_CTX_set_tmp_rsa_callback(client_ctx, rsa_callback);

    /* Don't bother with session cache for client side: not enough
     * connections to worry about caching */
//    SSL_CTX_set_session_cache_mode(client_ctx, SSL_SESS_CACHE_OFF);
//    SSL_CTX_set_timeout(client_ctx, 0);
//    if (!SSL_CTX_set_cipher_list(client_ctx, ssl_cipher_list))
//         printf("SSL_CTX_set_cipher_list\n");
}

void ssl_client_context_free()
{
    SSL_CTX_free(client_ctx);
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

/* ====================================================================== */

//static void my_alarm_stub()
//{
//    /* NOOP */
//}
//
///* ssl_server_server() ***************************************************
// *
// * Start server side SSL
// ************************************************************************/
//
//void *ssl_start_server(int fd, unsigned long timeout)
//{
//    SSL *ssl;
//    const SSL_CIPHER *c;
//    X509 *client_cert;
//    char *ver;
//    int bits;
//    int rc;
//
//    if (!(ssl = SSL_new(server_ctx)))
//        return (NIL);
//
//    SSL_set_session_id_context(ssl, sid_ctx, strlen((char *) sid_ctx));
//
//    SSL_set_fd(ssl, fd);
//    SSL_set_accept_state(ssl);
//
//    if (timeout > 0) {
//        fd_set readfds;
//        struct timeval timeval;
//
//        FD_ZERO(&readfds);
//        FD_SET(fd, &readfds);
//
//        /* Check for SSL negotiation */
//        timeval.tv_sec = timeout;
//        timeval.tv_usec = 0;
//
//        while (select(fd + 1, &readfds, NIL, NIL, &timeval) < 0) {
//            if (errno != EINTR) {
//                SSL_shutdown(ssl);      /* Safe? */
//                SSL_free(ssl);
//                ERR_remove_state(0);
//                return (NIL);
//            }
//        }
//
//        if (!FD_ISSET(fd, &readfds)) {
//            SSL_shutdown(ssl);  /* Safe? */
//            SSL_free(ssl);
//            ERR_remove_state(0);
//            return (NIL);
//        }
//    }
//
//    /* SSL_accept can spin. Use alarm to force read() syscall buried inside
//     * SSL_accept() to break with an EINTR. XXX Is this safe? */
//    if (timeout > 0) {
//        os_signal_alarm_init(my_alarm_stub);
//        alarm(timeout);
//    }
//
//    rc = SSL_accept(ssl);
//
//    if (timeout > 0) {
//        alarm(0);
//        os_signal_alarm_clear();
//    }
//
//    if (rc <= 0) {
//        SSL_shutdown(ssl);
//        SSL_free(ssl);
//        ERR_remove_state(0);
//        return (NIL);
//    }
//
//    if ((client_cert = SSL_get_peer_certificate(ssl)))
//        log_debug("SSL: Have client certificate");
//    else
//        log_debug("SSL: No client certificate");
//
//    switch (SSL_version(ssl)) {
//    case SSL2_VERSION:
//        ver = "SSLv2";
//        break;
//    case SSL3_VERSION:
//        ver = "SSLv3";
//        break;
//    case TLS1_VERSION:
//        ver = "TLSv1";
//        break;
//    default:
//        ver = "UNKNOWN";
//    }
//    c = SSL_get_current_cipher(ssl);
//
//    SSL_CIPHER_get_bits(c, &bits);
//    log_debug("Opened with %s, cipher %s (%lu bits)\n",
//              ver, SSL_CIPHER_get_name(c), (unsigned long) bits);
//
//
//    /* Put underlying socket in non-blocking mode: stops occasional
//     * deadlocks where select() timeout preferred */
//    os_socket_nonblocking(fd);
//
//    return ((void *) ssl);
//}
//
/* ssl_server_client() ***************************************************
 *
 * Start client side SSL
 ************************************************************************/

void *ssl_start_client(int fd, unsigned long timeout)
{
    SSL *ssl;
    const SSL_CIPHER *c;
    char *ver;
    int bits;

    if (!(ssl = (void *) SSL_new(client_ctx)))
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

///* ====================================================================== */
//
//#ifdef SESSION_CACHE_ENABLE
///*
// * Delete expired sessions: again stolen from Cyrus.
// */
//struct prunerock {
//    struct db *db;
//    int count;
//    int deletions;
//};
//
//static int
//prune_p(void *rock, const char *id, int idlen, const char *data,
//        int datalen)
//{
//    struct prunerock *prock = (struct prunerock *) rock;
//    time_t expire;
//
//    prock->count++;
//
//    /* grab the expire time */
//    memcpy(&expire, data, sizeof(time_t));
//
//    /* log this transaction */
//    if (ssl_verbose_logging) {
//        int i;
//        char idstr[SSL_MAX_SSL_SESSION_ID_LENGTH * 2 + 1];
//        for (i = 0; i < idlen; i++)
//            sprintf(idstr + i * 2, "%02X", id[i]);
//
//        log_debug("found SSL session: id=%s, expire=%s",
//                  idstr, ctime(&expire));
//    }
//
//    /* check if the session has expired */
//    return (expire < time(0));
//}
//
//static int
//prune_cb(void *rock, const char *id, int idlen,
//         const char *data, int datalen)
//{
//    struct prunerock *prock = (struct prunerock *) rock;
//    int ret;
//
//    prock->deletions++;
//
//    do {
//        ret = DB->delete(prock->db, id, idlen, NULL, 1);
//    }
//    while (ret == MYDB_AGAIN);
//
//    /* log this transaction */
//    if (ssl_verbose_logging) {
//        int i;
//        char idstr[SSL_MAX_SSL_SESSION_ID_LENGTH * 2 + 1];
//        for (i = 0; i < idlen; i++)
//            sprintf(idstr + i * 2, "%02X", id[i]);
//
//        log_debug("expiring SSL session: id=%s", idstr);
//    }
//
//    return 0;
//}
//
//int ssl_prune_sessions(struct ssl_config *ssl_config)
//{
//    int ret;
//    struct prunerock prock;
//
//    /* initialize DB environment */
//    DB->init(ssl_config->ssl_session_dir, 0);
//
//    /* create the name of the db file */
//
//    ret = DB->open("sessions.db", &sessdb);
//    if (ret != MYDB_OK)
//        log_fatal("DBERROR: opening %s: %s", "sessions.db",
//                  mydb_strerror(ret));
//
//    /* check each session in our database */
//    prock.db = sessdb;
//    prock.count = prock.deletions = 0;
//    DB->foreach(sessdb, "", 0, &prune_p, &prune_cb, &prock, NULL);
//    DB->close(sessdb);
//    sessdb = NULL;
//
//    log_debug("tls_prune: purged %d out of %d entries",
//              prock.deletions, prock.count);
//
//    DB->archive(NULL, NULL);
//    DB->done();
//
//    return (0);
//}
//#else
//int ssl_prune_sessions(struct ssl_config *ssl_config)
//{
//    return (0);
//}
//#endif
