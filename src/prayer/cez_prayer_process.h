/* $Cambridge: hermes/src/prayer/lib/process.h,v 1.3 2008/09/16 09:59:57 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

/* External Interfere Prototypes for process.c */

struct process {
    struct pool *pool;
    pid_t childpid;
    struct iostream *stream;
    BOOL use_pty;
};

BOOL process_exec(char *cmdline);

void process_clear(struct process *process);
BOOL
process_start(struct process *process, char *cmdline, BOOL use_pty,
              unsigned long timeout);
BOOL process_stop(struct process *process);

char *process_run_script(struct process *process, struct pool *pool,
                         struct assoc *h, char *script, char *result,
                         unsigned long result_length);
