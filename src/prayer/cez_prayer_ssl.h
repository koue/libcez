/* $Cambridge: hermes/src/prayer/lib/ssl.h,v 1.4 2012/06/30 14:30:08 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Error codes that we wont to export to clients */
#define SSL_PRAYER_RETRY (-2)
#define SSL_PRAYER_ERROR (-1)

/* RSA key length */
#define SSL_RSA_KEYLENGTH   (1024)

/* Default Cipher list */
#define SSLCIPHERLIST_DEFAULT \
    "ECDHE-RSA-AES128-SHA256:AES128-GCM-SHA256:RC4:HIGH:!MD5:!aNULL:!EDH:!EXP"

/* Subset of struct config which is relevant to SSL */

struct ssl_config {
    char *ssl_cipher_list;       /* List of ciphers */
    BOOL  ssl_server_preference; /* Server selects preferred cipher */
    char *ssl_session_dir;      /* SSL session directory */
    char *ssl_cert_file;        /* SSL Certificate file              */
    char *ssl_privatekey_file;  /* SSL Privatekey file               */
    char *ssl_dh_file;          /* SSL DH file                       */
    unsigned long ssl_session_timeout;  /* Timeout for SSL sessions          */
    unsigned long ssl_rsakey_lifespan;  /* Master server regenerates RSA key */
    unsigned long ssl_rsakey_freshen;   /* Keys last this long after 1st use */
    unsigned long ssl_default_port;     /* Default HTTPS port, if any        */
    char *egd_socket;           /* Path for EGD socket               */
    BOOL log_debug;             /* T => Enable debug logging    */
};

/* Prototypes for ssl.c */

BOOL ssl_is_available(void);

void ssl_check_rsakey(struct ssl_config *ssl_config);

void ssl_freshen_rsakey(struct ssl_config *ssl_config);

void ssl_context_init(struct ssl_config *ssl_config);

void ssl_context_free(void);

void ssl_shutdown(void *ssl);

int ssl_get_error(void *ssl, int count);

void ssl_free(void *ssl);

void *ssl_start_server(int fd, unsigned long timeout);

void *ssl_start_client(int fd, unsigned long timeout);

int ssl_read(void *ssl, unsigned char *buffer, unsigned long blocksize);

int ssl_write(void *ssl, unsigned char *buffer, unsigned long bytes);

int ssl_pending(void *ssl);

int ssl_prune_sessions(struct ssl_config *ssl_config);

void ssl_client_context_init(void);

void ssl_client_context_free(void);
