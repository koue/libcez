/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#ifndef _CEZ_LOG_H
#define _CEZ_LOG_H

#include <errno.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <syslog.h>
#include <unistd.h>

#include <cez_core_pool.h>

struct log {
    struct pool *pool;
    unsigned long file_perms;
    char *name;
    int fd;
    ino_t inode;
    unsigned long log_ping_interval;
    time_t last_ping;
    BOOL log_debug;
    BOOL fatal_dump_core;
};

struct log *log_create(char *name, struct pool *pool);

void log_free(struct log *log);

BOOL log_open(struct log *log);

BOOL log_ping(struct log *log);

/* ====================================================================== */

void
log_panic_ap(struct log *log, unsigned long len, char *fmt, va_list ap);

BOOL
log_ap(struct log *log, struct pool *pool,
       unsigned long len, char *fmt, va_list ap);

BOOL log_here(struct log *log, char *fmt, ...);

/* ====================================================================== */

void log_debug(struct log *log, char *fmt, ...);

void log_panic(struct log *log, char *fmt, ...);

void log_fatal(struct log *log, char *fmt, ...);

#endif

