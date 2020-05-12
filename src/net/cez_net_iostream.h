/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* Some simple stream IO functions that work with sockets (separate
   read/write buffers) and provide timeout options on read */

struct iostream {
    struct pool *pool;          /* Allocation pool */
    unsigned long blocksize;    /* Size of IO buffers */
    int fd;                     /* File descriptor associated with this iostream */
    void *ssl;                  /* Actually SSL * */
    void *bio;                  /* SSL bio stream */
    BOOL ieof;                  /* Input stream has reached end of file */

    /* Input buffer */
    unsigned char *ibuffer;     /* Input buffer */
    unsigned char *ibufend;     /* End of buffer (simplies ptr arithmethic) */
    unsigned char *icurrent;    /* Current read location in input buffer */
    time_t itimeout;            /* Timeout on input streams */

    /* Output buffer */
    unsigned char *obuffer;     /* Output buffer */
    unsigned char *obufend;     /* End of buffer (simplies ptr arithmethic) */
    unsigned char *ocurrent;    /* Current read location in input buffer */
    time_t otimeout;            /* Timeout on input streams */
    BOOL oerror;                /* Error occured during write */
};

#define IOSTREAM_PREFERRED_BLOCK_SIZE (16384)

void iostream_check_rsakey(struct ssl_config *ssl_config);

void iostream_freshen_rsakey(struct ssl_config *ssl_config);

void iostream_init(struct ssl_config *ssl_config);
struct iostream *iostream_create(struct pool *p, int sockfd,
                                 unsigned long blocksize);

void iostream_free(struct iostream *x);

void iostream_free_buffers(struct iostream *x);

void iostream_close(struct iostream *x);

BOOL iostream_ssl_start_server(struct iostream *x);
BOOL iostream_ssl_start_client(struct iostream *x);

BOOL iostream_ssl_enabled(struct iostream *x);

void iostream_set_timeout(struct iostream *x, time_t timeout);
int iostream_getchar(struct iostream *x);
void iostream_ungetchar(char c, struct iostream *x);
BOOL
iostream_have_buffered_input(struct iostream *x, BOOL ignore_whitespace);

BOOL iostream_have_input(struct iostream *x);

BOOL iostream_is_eof(struct iostream *x);

BOOL iostream_putchar(char c, struct iostream *x);
BOOL iostream_flush(struct iostream *x);
BOOL iostream_puts(struct iostream *x, unsigned char *s);
BOOL iostream_printf(struct iostream *x, char *format, ...);
BOOL iostream_getline(struct iostream *stream, char *s, int length);

BOOL
iostream_getline_overflow(struct iostream *stream,
                          char *s, int length, BOOL * overflowp);

#define iogetc(x)      ((x->icurrent < x->ibufend) \
                        ? (int)*(x->icurrent++) : iostream_getchar(x))

#define ioungetc(c, x) (*(--x->icurrent) = c);

#define ioputc(c, x)                    \
{                                       \
  unsigned char _c = (unsigned char)c;  \
                                        \
  if (x->ocurrent < x->obufend)         \
    *(x->ocurrent++) = _c;              \
  else                                  \
    iostream_putchar(c, x);             \
}

#define ioflush(x)     (iostream_flush(x))

#define ioputs(a, b)   iostream_puts(a, (unsigned char *)b)
#define ioprintf       iostream_printf
