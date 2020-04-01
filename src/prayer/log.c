/* $Cambridge: hermes/src/prayer/shared/log.c,v 1.3 2008/09/16 09:59:58 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Program name */
static char *log_progname = NIL;

#define LOG_TMP_SIZE    (255)   /* Space on stack rather than heap */
#define LOG_PREFIX_SIZE (50)    /* Max length of prefix without username */

/* log_create() ***********************************************************
 *
 * Create new log struct
 *   config: Prayer configuration
 *     pool: Pool to expand filename into
 *
 * Returns: New log structure
 ************************************************************************/

struct log *log_create(struct config *config, struct pool *pool)
{
    struct log *log;

    log = pool_alloc(pool, sizeof(struct log));
    log->config = config;
    log->pool = pool;
    log->name = NIL;
    log->fd = -1;
    log->inode = 0;
    log->last_ping = 0;
    log->peer_pid = 0;

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

BOOL log_open(struct log *log, char *name)
{
    struct pool *pool = log->pool;
    struct config *config = log->config;
    int open_flags;
    int open_perms;
    struct stat sbuf;

    if (!(name && name[0])) {
        log_panic("log_open() called with empty file name");
        return (NIL);
    }

    if (config && config->log_dir && config->log_dir[0] &&
        name && (name[0] != '/'))
        log->name = pool_strcat3(pool, config->log_dir, "/", name);
    else
        log->name = pool_strdup(pool, name);

    /* Calculate arguments for open() */
    if (getuid() != 0)
        open_flags = O_APPEND | O_WRONLY | O_CREAT;     /* Non-root can create files */
    else
        open_flags = O_APPEND | O_WRONLY;       /* Don't create files as root */

    open_perms =
        ((config && config->file_perms) ? config->file_perms : 0644);

    /* Try to open or create file */
    if ((log->fd = open(log->name, open_flags, open_perms)) < 0) {
        log_panic("Couldn't open log file: \"%s\": %s",
                  log->name, strerror(errno));
        return (NIL);
    }

    /* Record file inode and last ping times */
    if (fstat(log->fd, &sbuf) < 0) {
        log_panic("Couldn't fstat() log file: \"%s\": %s",
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
    struct config *config = log->config;
    int open_flags;
    int open_perms;
    struct stat sbuf;
    time_t now = time(NIL);

    /* No log structure allocated. Shouldn't happen! */
    if (log == NIL)
        return (NIL);

    /* Log file already open and pinged recently? */
    if ((log->fd >= 0) && (config->log_ping_interval > 0) &&
        (log->last_ping + config->log_ping_interval) >= now)
        return (T);

    /* Ping log file: has the inode changed? */
    if ((stat(log->name, &sbuf) >= 0) && (sbuf.st_ino == log->inode))
        return (T);

    /* Calculate arguments for open(2) */
    if (getuid() != 0)
        open_flags = O_APPEND | O_WRONLY | O_CREAT;     /* Non-root can create files */
    else
        open_flags = O_APPEND | O_WRONLY;       /* Don't create files as root */

    open_perms =
        ((config && config->file_perms) ? config->file_perms : 0644);

    /* Reopen log file */
    close(log->fd);
    if ((log->fd =
         open(log->name, O_APPEND | O_WRONLY | O_CREAT, open_perms)) < 0) {
        log_panic("Couldn't open log file: \"%s\": %s", log->name,
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

/* Routines for logging messages to log files, syslog and sterr */

/* log_entry_size() ******************************************************
 *
 * Calculate size of log entry item. Doesn't include timestamps and other
 * (fixed length) padding that that the log function may want to add
 *     fmt: Format string
 *      ap: va_list
 ************************************************************************/

unsigned long log_entry_size(char *fmt, va_list ap)
{
    return (pool_vprintf_size(fmt, ap));
}

/* ====================================================================== */

/* log_timestamp() *******************************************************
 *
 * Generate suitable (fixed length!) prefix for log line.
 *        log: 
 *     buffer: Target buffer
 *   username: Username to log. NIL => username unknown or no significant
 ************************************************************************/

static void log_timestamp(struct log *log, char *buffer, char *username)
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

    if (log && log->peer_pid)
        sprintf(buffer, "[%lu:%lu] ",
                (unsigned long) log->peer_pid, (unsigned long) getpid());
    else
        sprintf(buffer, "[%lu] ", (unsigned long) getpid());

    buffer = buffer + strlen(buffer);

    if (username && username[0])
        sprintf(buffer, "(%s) ", username);

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
    if (log_progname)
        fprintf(stderr, "%s PANICLOG:\n", log_progname);
    else
        fprintf(stderr, "Prayer PANICLOG:\n");
    fprintf(stderr, "  [log_panic] Out of memory\n");

    if (log_progname)
        openlog(log_progname, LOG_PID | LOG_CONS, LOG_MAIL);
    else
        openlog("prayer", LOG_PID | LOG_CONS, LOG_MAIL);

    syslog(LOG_ERR, "[log_panic] Out of memory");
    closelog();
}

/* ====================================================================== */

/* log_panic_ap() ********************************************************
 *
 * Log string to panic log. If panic file cannot be opened then the message
 * is sent to syslog and stderr.
 *    config: Configuration.
 *  username: Username to put in log file. (NIL => none)
 *       len: Length of (variable part of) log entry
 *       fmt: Format string
 *        ap: va_list
 ************************************************************************/

void
log_panic_ap(struct config *config,
             char *username, unsigned long len, char *fmt, va_list ap)
{
    char *buffer, *error;
    int fd;
    int timelen, maxlen;
    char *name;
    char *progname = (log_progname) ? log_progname : "prayer";

    /* Only try ro write to file if config->log_dir defined and expanded */

    if (config && config->log_dir && config->log_dir[0] &&
        (config->log_dir[0] != '$'))
        name = pool_strcat3(NIL, config->log_dir, "/", "paniclog");
    else
        name = NIL;

    /* Username string can be arbitrarily long */
    maxlen = len + LOG_PREFIX_SIZE;
    if (username)
        maxlen += strlen(username) + 3;  /* "(%s) " */

    /* Alloc temporary buffer with space for Date + Expanded String + \n */
    /* Important that _something_ reported to user */
    if ((buffer = malloc(maxlen)) == NIL) {
        log_out_of_memory();
        return;
    }

    log_timestamp(NIL, buffer, username);
    error = buffer + (timelen = strlen(buffer));
    pool_vprintf(error, fmt, ap);
    len = log_terminate(error, len);

    if (name) {
        int open_flags;
        int open_perms;

        open_perms =
            ((config && config->file_perms) ? config->file_perms : 0644);

        if (getuid() != 0)
            open_flags = O_CREAT | O_APPEND | O_WRONLY;
        else
            open_flags = O_APPEND | O_WRONLY;

        if ((fd = open(name, open_flags, open_perms)) >= 0) {
            write(fd, buffer, timelen + len);
            close(fd);
            /* Copy error to stderr (noop after stderr redirected to /dev/null) */
            fprintf(stderr, "%s: %s", progname, buffer);
            free(buffer);
            return;
        }
    }

    /* Failed to write to named file: write to stderr and syslog */
    fprintf(stderr, "%s PANICLOG:\n", progname);
    fprintf(stderr, "  Failed to open panic log file: \"paniclog\"\n");
    fprintf(stderr, "  Error was: %s", buffer);

    openlog(progname, LOG_PID | LOG_CONS, LOG_MAIL);
    if (name)
        syslog(LOG_ERR, "Failed to write log entry to \"%s\": %s", name,
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
 *  username: Username to put in log file.
 *       len: Length of (variable part of) log entry
 *       fmt: Format string
 *        ap: va_list
 ************************************************************************/

BOOL
log_ap(struct log *log, struct pool *pool, char *username,
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

    /* Username string can be arbitrarily long */
    maxlen = len + LOG_PREFIX_SIZE;
    if (username)
        maxlen += strlen(username) + 3;  /* "(%s) " */

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
    log_timestamp(log, output, username);
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
    log_ap(log, NIL, NIL, len, fmt, ap);
    va_end(ap);

    return (T);
}

/* log_record_peer_pid() *************************************************
 *
 * Record process ID of peer (used by session access log only)
 *    log:
 *    pid:
 ************************************************************************/

void log_record_peer_pid(struct log *log, pid_t pid)
{
    log->peer_pid = pid;
}

/* ====================================================================== */
/* ====================================================================== */

/* Support for process wide miscellaneous, debug and panic logs which
 * Don't require us to pass around global state handles. Seemed like a
 * good idea at the time, however this doesn't fit sit terribly well
 * with the rest of the prayer design. The routines are used mostly
 * by iostream classes and other. May want to come back and quietly
 * review this situation later */

/* A little bit of global state */
static struct log *log_misc_ptr = NIL;

/* log_init() ************************************************************
 *
 * Initialise the log subsystem: sets up the global state above.
 *         config: Prayer configuration
 *       progname: Program name
 *  misc_log_name: Name for miscellaneous log entries (typically function
 *                 of progname)
 *
 * Returns:  T => everything okay
 ************************************************************************/

BOOL log_misc_init(struct config *config, char *progname, char *log_name)
{
    char *s;

    if ((s = strrchr(progname, '/')))
        log_progname = strdup(s + 1);
    else
        log_progname = strdup(progname);

    log_misc_ptr = log_create(config, NIL);

    return (log_open(log_misc_ptr, log_name));
}

/* log_misc_ping() *******************************************************
 *
 * Reopen misc log file if required
 *
 ************************************************************************/

BOOL log_misc_ping(void)
{
    return (log_ping(log_misc_ptr));
}

/* ====================================================================== */

/* log_misc() *************************************************************
 *
 * Log message to miscellaneous log if it is open
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_misc(char *fmt, ...)
{
    unsigned long len;
    va_list ap;

    if ((log_misc_ptr == NIL) || (log_misc_ptr->fd < 0))
        return;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_ap(log_misc_ptr, NIL, NIL, len, fmt, ap);
    va_end(ap);
}

/* log_debug() ***********************************************************
 *
 * Log message to miscellaneous log if it is open and debugging set
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_debug(char *fmt, ...)
{
    struct config *config = NIL;
    unsigned long len;
    va_list ap;

    if ((log_misc_ptr == NIL) || (log_misc_ptr->fd < 0))
        return;

    if (log_misc_ptr->config)
        config = log_misc_ptr->config;

    if (config->log_debug == NIL)
        return;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_ap(log_misc_ptr, NIL, NIL, len, fmt, ap);
    va_end(ap);
}

/* log_panic() ***********************************************************
 *
 * Log message to panic log
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_panic(char *fmt, ...)
{
    struct config *config = NIL;
    unsigned long len;
    va_list ap;

    if (log_misc_ptr && log_misc_ptr->config)
        config = log_misc_ptr->config;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    va_start(ap, fmt);
    log_panic_ap(config, NIL, len, fmt, ap);
    va_end(ap);
}

/* log_fatal() ***********************************************************
 *
 * Log message to panic log and exit.
 *    fmt: Format string, followed by arguments.
 ************************************************************************/

void log_fatal(char *fmt, ...)
{
    struct config *config = NIL;
    unsigned long len;
    va_list ap;

    va_start(ap, fmt);
    len = pool_vprintf_size(fmt, ap);
    va_end(ap);

    if (log_misc_ptr && log_misc_ptr->config)
        config = log_misc_ptr->config;

    va_start(ap, fmt);
    log_panic_ap(config, NIL, len, fmt, ap);
    va_end(ap);

    if (config && config->fatal_dump_core && (getuid() != 0))
        abort();

    exit(1);
}
