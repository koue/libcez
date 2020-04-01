/* $Cambridge: hermes/src/prayer/lib/process.c,v 1.3 2008/09/16 09:59:57 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

#define MAXLENGTH        (8192)

/* Class for communicating with subsiduary process using Pseudo Terminal */

/* process_clear() *******************************************************
 *
 * Clear process state
 ************************************************************************/

void process_clear(struct process *process)
{
    process->childpid = 0;
    process->stream = NIL;
}

/* ====================================================================== */

/* process_makeenv() *****************************************************
 *
 * Utility routine for setting up environment in child process.
 ************************************************************************/

static
char *process_makeenv(char *key, char *value)
{
    char *result;

    if ((result = malloc(strlen(key) + strlen(value) + 2)) == NIL)
        log_fatal("Out of memory");

    sprintf(result, "%s=%s", key, value);
    return (result);
}

/* process_exec() ********************************************************
 *
 * exec new process.
 ************************************************************************/

BOOL process_exec(char *cmdline)
{
    struct passwd *pwd;
    char *path, *s;
    int argc;
    char **argv;
    char *env[6];

    /* Set up environment array for child process */
    if ((pwd = getpwuid(getuid())) == NIL)
        log_fatal("getpwuid() failed");

    env[0] = process_makeenv("HOME", pwd->pw_dir);
    env[1] = process_makeenv("LOGNAME", pwd->pw_name);
    env[2] = process_makeenv("USER", pwd->pw_name);
    env[3] = process_makeenv("PATH", "/bin:/usr/bin:/opt/local/bin");
    env[4] = process_makeenv("SHELL", "/bin/false");
    env[5] = NIL;

    /* Work out correct size for the argv array */
    argc = 0;
    s = cmdline;
    while (*s) {
        if ((*s == ' ') || (*s == '\t')) {
            argc++;
            while ((*s == ' ') || (*s == '\t'))
                s++;
        } else
            s++;
    }

    /* Allocate the argv array */
    if ((argv = malloc((argc + 1) * sizeof(char *))) == NIL) {
        log_fatal("process_exec(): Out of memory");
        /* NOTREACHED */
        exit(1);
    }

    /* Set up argv array */
    argv[0] = path = strdup(cmdline);
    argc = 0;

    s = path;
    while (*s) {
        if ((*s == ' ') || (*s == '\t')) {
            *s++ = '\0';
            while ((*s == ' ') || (*s == '\t'))
                s++;
            if (*s)
                argv[++argc] = s;
        } else
            s++;
    }
    argv[argc + 1] = NIL;

    /* Caculate argv[0] if full pathname was provided */
    if (strrchr(argv[0], '/'))
        argv[0] = strrchr(path, '/') + 1;

    execve(path, argv, env);
    log_fatal("Failed to execve() passwd program");
    /* NOTREACHED */
    exit(1);
}

/* ====================================================================== */

/* process_start() *******************************************************
 *
 * Start up subsiduary process for communication
 *    process:  Wrapper structure for state
 *    cmdline:  Program that we are about to run
 *    use_pty:  Run program inside a Pseudo terminal
 *       argv:  argv array to pass to child.
 *
 * Returns: T if subprocess started successfully. NIL otherwise.
 ************************************************************************/

BOOL
process_start(struct process *process, char *cmdline, BOOL use_pty,
              unsigned long timeout)
{
    int childpid;
    int masterfd;

    if (use_pty) {
        if (!os_run_pty(cmdline, &masterfd, &childpid)) {
            log_misc("Failed to open pseudoterminal");
            return (NIL);
        }
    } else {
        if (!os_run(cmdline, &masterfd, &childpid)) {
            log_misc("Failed to open pseudoterminal");
            return (NIL);
        }
    }

    process->use_pty = use_pty;
    process->childpid = childpid;
    if (!(process->stream = iostream_create(NIL, masterfd, 0)))
        return (NIL);

    iostream_set_timeout(process->stream, timeout);
    return (T);
}

/* ====================================================================== */

/* process_stop() *********************************************************
 *
 * Stop a running process: shuts down iostream connection and waits for
 * process to finish.
 *************************************************************************/

BOOL process_stop(struct process * process)
{
    int status;

    if (process->childpid == 0)
        return (NIL);

    if (process->stream)
        iostream_close(process->stream);

    waitpid(process->childpid, &status, 0);

    return (T);
}

/* ====================================================================== */

/* process_expect_string() ***********************************************
 *
 * Match string on following line of output from child process.
 *
 * Returns:
 *    string matched sucessfully => NIL
 *    Otherwise                  => Line from child as error condition
 ************************************************************************/

static char *process_expect_string(struct process *process, char *string,
                                   char *warning)
{
    unsigned long len = strlen(string);
    unsigned long count = 0;
    struct iostream *stream = process->stream;
    char c;
    static char buffer[MAXLENGTH];

    buffer[0] = '\0';

    while ((c = iogetc(stream)) != EOF) {
        if (c == '\r')
            continue;

        if (count == MAXLENGTH)
            return ("Line from child process too long");

        buffer[count++] = c;
        if ((count >= len)
            && !strncmp(buffer + (count - len), string, len))
            return (NIL);

        if (c == '\n') {
            if (warning && !strncmp(buffer, warning, strlen(warning))) {
                count = 0;      /* Ignore warning lines */
                buffer[0] = '\0';
            } else {
                buffer[count - 1] = '\0';       /* Return line from child as error */
                return (buffer);
            }
        }
    }
    return ("End of file or timeout waiting for input");
}

/* process_read_line() ***************************************************
 *
 * Fetch a line from child process
 *  process:
 *      buf: Target buffer (if defined)
 *     blen: Length of target buffer (if defined)
 * 
 * Returns: T => got line. NIL => EOF.
 ************************************************************************/

BOOL
process_read_line(struct process * process, char *buf, unsigned long blen)
{
    struct iostream *stream = process->stream;
    char c;
    unsigned long i = 0;

    if (blen > 0)
        blen--;                 /* Leave space for trailing '\0' */

    while ((c = iogetc(stream)) != EOF) {
        if (c == '\r')
            continue;

        if (c == '\n') {
            if (buf && (i < blen))
                buf[i] = '\0';
            return (T);
        }

        if (buf && (i < blen))
            buf[i++] = c;
    }
    return (NIL);
}

/* process_get_token() **************************************************
 *
 * Isolate next token in string
 *    sp: Ptr to current string location
 *        (updated to point to following token)
 *
 * Returns: Ptr to next token, NIL if none.
 ***********************************************************************/

static char *process_get_token(char **sp)
{
    char *s = *sp, *result;

    if (!(s = string_next_token(sp)))
        return (NIL);

    if (*s == '"') {
        /* Deal with quoted strings */
        result = s + 1;
        s += 2;

        while (*s && (*s != '"'))
            s++;

        if (*s == '\0')
            return (NIL);

        *s++ = '\0';
        *sp = s;
        return (result);
    }

    /* Record position of this token */
    result = s;

    /* Find next whitespace character or end of string */
    while ((*s) && (*s != ' ') && (*s != '\t'))
        s++;

    /* Tie off the string unless \0 already reached */
    if (*s) {
        *s++ = '\0';

        while ((*s == ' ') || (*s == '\t'))
            s++;
    }

    /* Record position of first non-whitespace character for next caller */
    *sp = s;

    return (result);
}

/* ====================================================================== */

/* process_run_script() **************************************************
 *
 * Run script against process
 *
 * Returns:
 *  NIL   => script matched successfully
 *  ""    => unexpected EOF
 *  Other => Last line of output from child as an error condition
 ************************************************************************/

char *process_run_script(struct process *process,
                         struct pool *pool, struct assoc *h,
                         char *script, char *result,
                         unsigned long result_length)
{
    char *type, *value, *ret;
    char *warning = NIL;
    struct iostream *stream = process->stream;

    while (*script) {
        if (!(type = string_get_token(&script)))
            return (NIL);

        if (!strcasecmp(type, "readline")) {
            if (!process_read_line(process, NIL, 0L))
                return ("");
            continue;
        }

        if (!strcasecmp(type, "result")) {
            if (!process_read_line(process, result, result_length))
                return ("");
            return (NIL);
        }

        if (!(value = process_get_token(&script)))
            return (NIL);

        value = string_expand(pool, h, value);

        if (!strcasecmp(type, "warning")) {
            warning = value;
            continue;
        }

        if (!strcasecmp(type, "expect")) {
            if ((ret = process_expect_string(process, value, warning)))
                return (ret);
            continue;
        }

        if (!strcasecmp(type, "sendline")) {
            ioprintf(stream, "%s\n", value);
            ioflush(stream);

            if (process->use_pty && !process_read_line(process, NIL, 0L))
                return ("");
            continue;
        }

        if (!strcasecmp(type, "send")) {
            ioputs(stream, value);
            ioflush(stream);
            continue;
        }

        log_panic("Unexpected command in script: %s %s", type, value);
        return ("Internal server error");
    }
    /* Script finished sucessfully */
    return (NIL);
}
