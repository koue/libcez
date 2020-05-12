/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_log.h"

/* Program name */
extern char *__progname;

#define LOG_TMP_SIZE    (255)   /* Space on stack rather than heap */
#define LOG_PREFIX_SIZE (50)    /* Max length of prefix */

/* log_create() ***********************************************************
 *
 * Create new log struct
 *   config: Prayer configuration
 *     pool: Pool to expand filename into
 *
 * Returns: New log structure
 ************************************************************************/

struct log *log_create(char *name, struct pool *pool)
{
    struct log *log;

    log = pool_alloc(pool, sizeof(struct log));
    log->pool = pool;
    log->name = pool_strdup(pool, name);
    log->fd = -1;
    log->inode = 0;
    log->last_ping = 0;
    log->file_perms = 0644;
    log->log_debug = NIL;
    log->fatal_dump_core = NIL;
    log->log_ping_interval = (5 * 60);       /* 5 mins  */
    log->last_ping = 0;

    return (log);
}

/* log_free() ************************************************************
 *
 * Free log structure (unless allocated against pool)
 *   log: Structure to free
 ************************************************************************/

void log_free(struct log *log)
{
    if (log->fd >= 0)
        close(log->fd);

    if (log->pool)
        return;

    if (log->name)
        free(log->name);
    free(log);
}

/* log_open() ************************************************************
 *
 * Open log file
 *      log: Structure which will contain all info related to log file
 *     name: Name of log file to open.
 *
 * Returns: T if log opened successfully.
 ************************************************************************/

BOOL log_open(struct log *log)
{
    int open_flags;
    struct stat sbuf;

    if (!(log->name && log->name[0])) {
        log_panic(log, "log_open() called with empty file name");
        return (NIL);
    }

    open_flags = O_APPEND | O_WRONLY | O_CREAT;

    /* Try to open or create file */
    if ((log->fd = open(log->name, open_flags, log->file_perms)) < 0) {
        log_panic(log, "Couldn't open log file: \"%s\": %s",
                  log->name, strerror(errno));
        return (NIL);
    }

    /* Record file inode and last ping times */
    if (fstat(log->fd, &sbuf) < 0) {
        log_panic(log, "Couldn't fstat() log file: \"%s\": %s",
                  log->name, strerror(errno));
        return (NIL);
    }

    log->inode = sbuf.st_ino;
    log->last_ping = time(NIL);
    return (T);
}

/* log_open() ************************************************************
 *
 * Open log file
 *      log: Structure which will contain all info related to log file
 *
 * Returns: T if log pinged okay.
 ************************************************************************/

BOOL log_ping(struct log * log)
{
    int open_flags;
    struct stat sbuf;
    time_t now = time(NIL);

    /* No log structure allocated. Shouldn't happen! */
    if (log == NIL)
        return (NIL);

    /* Log file already open and pinged recently? */
    if ((log->fd >= 0) && (log->log_ping_interval > 0) &&
        (log->last_ping + log->log_ping_interval) >= now)
        return (T);

    /* Ping log file: has the inode changed? */
    if ((stat(log->name, &sbuf) >= 0) && (sbuf.st_ino == log->inode))
        return (T);

    open_flags = O_APPEND | O_WRONLY | O_CREAT;

    /* Reopen log file */
    close(log->fd);
    if ((log->fd =
        open(log->name, O_APPEND | O_WRONLY | O_CREAT, log->file_perms)) < 0) {
        log_panic(log, "Couldn't open log file: \"%s\": %s", log->name,
                  strerror(errno));
        return (NIL);
    }

    /* Record inode and ping time */
    log->inode = sbuf.st_ino;
    log->last_ping = now;
    return (T);
}

/* ====================================================================== */
/* ====================================================================== */

/* log_timestamp() *******************************************************
 *
 * Generate suitable (fixed length!) prefix for log line.
 *        log:
 *     buffer: Target buffer
 ************************************************************************/

static void log_timestamp(struct log *log, char *buffer)
{
    time_t now;
    struct tm *tm;
    static char *date_month[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
        "Oct",
        "Nov", "Dec"
    };

    time(&now);
    tm = localtime(&now);

    sprintf(buffer, "%s %c%c %c%c:%c%c:%c%c ",
            date_month[tm->tm_mon],
            '0' + ((tm->tm_mday / 10) % 10), '0' + ((tm->tm_mday) % 10),
            '0' + ((tm->tm_hour / 10) % 10), '0' + ((tm->tm_hour) % 10),
            '0' + ((tm->tm_min / 10) % 10), '0' + ((tm->tm_min) % 10),
            '0' + ((tm->tm_sec / 10) % 10), '0' + ((tm->tm_sec) % 10));

    buffer = buffer + strlen(buffer);

    sprintf(buffer, "[%lu] ", (unsigned long) getpid());

    buffer = buffer + strlen(buffer);

}

/* log_terminate() *******************************************************
 *
 * Terminate a log entry: make sure that it finishs with a '\n'
 *    buffer: Working buffer
 *       len: Current length of data in buffer
 ************************************************************************/

static unsigned long log_terminate(char *buffer, unsigned long len)
{
    /* Make sure that string terminates with '\n' */
    if ((len > 2) && (buffer[len - 2] == '\015')
        && (buffer[len - 1] == '\012')) {
        buffer[len - 2] = '\n'; /* CRLF -> LF */
        buffer[len - 1] = '\0';
        return (len - 1);
    }

    if ((len > 1)
        && ((buffer[len - 1] != '\015') && (buffer[len - 1] != '\012'))) {
        /* Add a "\n" to the end of the string */
        buffer[len] = '\n';
        buffer[len + 1] = '\0';
        return (len + 1);
    }

    buffer[len - 1] = '\n';     /* CR or LF -> LF */
    return (len);
}

/* log_out_of_memory() ***************************************************
 *
 * Out of memory while trying to report error. Send something to
 * syslog and paniclog so we have some idea what is going on.
 ************************************************************************/

static void log_out_of_memory(void)
{
    fprintf(stderr, "%s PANICLOG:\n", __progname);
    fprintf(stderr, "  [log_panic] Out of memory\n");

    openlog(__progname, LOG_PID | LOG_CONS, LOG_MAIL);

    syslog(LOG_ERR, "[log_panic] Out of memory");
    closelog();
}

/* ====================================================================== */

/* log_panic_ap() ********************************************************
 *
 * Log string to panic log. If panic file cannot be opened then the message
 * is sent to syslog and stderr.
 *    config: Configuration.
 *       len: Length of (variable part of) log entry
 *       fmt: Format string
 *        ap: va_list
 ************************************************************************/

void
log_panic_ap(struct log *log, unsigned long len, char *fmt, va_list ap)
{
    char *buffer, *error;
    int fd;
    int timelen, maxlen;

    maxlen = len + LOG_PREFIX_SIZE;

    /* Alloc temporary buffer with space for Date + Expanded String + \n */
    /* Important that _something_ reported to user */
    if ((buffer = malloc(maxlen)) == NIL) {
        log_out_of_memory();
        return;
    }

    log_timestamp(NIL, buffer);
    error = buffer + (timelen = strlen(buffer));
    pool_vprintf(error, fmt, ap);
    len = log_terminate(error, len);

    if (log->name && log->name[0]) {
        int open_flags;

        open_flags = O_CREAT | O_APPEND | O_WRONLY;

        if ((fd = open(log->name, open_flags, log->file_perms)) >= 0) {
            write(fd, buffer, timelen + len);
            close(fd);
            /* Copy error to stderr (noop after stderr redirected to /dev/null) */
            fprintf(stderr, "%s: %s", __progname, buffer);
            free(buffer);
            return;
        }
    }

    /* Failed to write to named file: write to stderr and syslog */
    fprintf(stderr, "%s PANICLOG:\n", __progname);
    fprintf(stderr, "  Failed to open panic log file: \"paniclog\"\n");
    fprintf(stderr, "  Error was: %s", buffer);

    openlog(__progname, LOG_PID | LOG_CONS, LOG_MAIL);
    if (log->name && log->name[0])
        syslog(LOG_ERR, "Failed to write log entry to \"%s\": %s", log->name,
               error);
    else
        syslog(LOG_ERR, "Failed to write log entry: %s", error);
    closelog();

    free(buffer);
}

/* ====================================================================== */

/* log_ap() ***************************************************************
 *
 * Log string to open file descriptor
 *        fd: Target file descriptor
 *      pool: Scratch pool (NIL => malloc and free own memory)
 *       len: Length of (variable part of) log entry
 *       fmt: Format string
 *        ap: va_list
 ************************************************************************/

BOOL
log_ap(struct log *log, struct pool *pool,
       unsigned long len, char *fmt, va_list ap)
{
    char *output, *tmp;
    unsigned long timelen, maxlen;
    char log_tmp[LOG_TMP_SIZE + 1];

    if (!log_ping(log))
        return (NIL);

    /* Log file isn't open */
    if (log->fd < 0)
        return (NIL);

    maxlen = len + LOG_PREFIX_SIZE;

    /* Alloc temporary buffer with space for Date + Expanded String + \n */
    if (pool)
        output = pool_alloc(pool, maxlen);
    else if ((maxlen) < LOG_TMP_SIZE) {
        output = log_tmp;
    } else if ((output = malloc(maxlen)) == NIL) {
        log_out_of_memory();
        return (NIL);
    }

    /* Write timestamp, then log entry into the buffer */
    log_timestamp(log, output);
    tmp = output + (timelen = strlen(output));
    pool_vprintf(tmp, fmt, ap);
    len = log_terminate(tmp, len);

    /* Then punt the log entry to the file as a single (O_APPEND) write */
    write(log->fd, output, timelen + len);

    if ((pool == NIL) && (output != log_tmp))
        free(output);

    return (T);
}

/* log_here() *************************************************************
 *
 * Log message to open log file
 *     fd: File descriptor
 *    fmt: Format string, followed by arguments.
 *
 * Returns: T on sucess
 ************************************************************************/

BOOL
log_here(struct log * log, char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_ap(log, log->pool, len, fmt, ap);
    va_end(ap);

    return (T);
}

/* ====================================================================== */

/* log_debug() ***********************************************************
 *
 * Log message to miscellaneous log if it is open and debugging set
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_debug(struct log *log, char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    if ((log->fd < 0))
        return;

    if (log->log_debug == NIL)
        return;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_ap(log, log->pool, len, fmt, ap);
    va_end(ap);
}

/* log_panic() ***********************************************************
 *
 * Log message to panic log
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_panic(struct log *log, char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_panic_ap(log, len, fmt, ap);
    va_end(ap);
}

/* log_fatal() ***********************************************************
 *
 * Log message to panic log and exit.
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_fatal(struct log *log, char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_panic_ap(log, len, fmt, ap);
    va_end(ap);

    if (log->fatal_dump_core && (getuid() != 0))
        abort();

    exit(1);
}
