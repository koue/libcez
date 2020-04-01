/* $Cambridge: hermes/src/prayer/shared/log.h,v 1.3 2008/09/16 09:59:58 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

struct log {
    struct pool *pool;
    struct config *config;
    char *name;
    int fd;
    ino_t inode;
    time_t last_ping;
    pid_t peer_pid;
};

struct log *log_create(struct config *config, struct pool *pool);

void log_free(struct log *log);

BOOL log_open(struct log *log, char *name);

BOOL log_ping(struct log *log);

/* ====================================================================== */

unsigned long log_entry_size(char *fmt, va_list ap);

void
log_panic_ap(struct config *config,
             char *username, unsigned long len, char *fmt, va_list ap);

BOOL
log_ap(struct log *log, struct pool *pool, char *username,
       unsigned long len, char *fmt, va_list ap);

BOOL log_here(struct log *log, char *fmt, ...);

void log_record_peer_pid(struct log *log, pid_t pid);

/* ====================================================================== */

BOOL
log_misc_init(struct config *config, char *progname, char *misc_log_name);

BOOL log_misc_ping(void);

void log_misc(char *fmt, ...);

void log_debug(char *fmt, ...);

void log_panic(char *fmt, ...);

void log_fatal(char *fmt, ...);
