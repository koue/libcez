typedef int BOOL;

#ifndef NIL
#define NIL (0)
#endif

#ifndef T
#define T (1)
#endif

#define CRLF "\015\012"

/* Fix ctype.h macros. */
#define UC (unsigned char)
#define Uisspace(c) isspace(UC(c))
#define Uisalpha(c) isalpha(UC(c))
#define Uisalnum(c) isalnum(UC(c))
#define Uisdigit(c) isdigit(UC(c))
#define Uisxdigit(c) isxdigit(UC(c))
#define Uiscntrl(c) iscntrl(UC(c))
#define Utoupper(c) toupper(UC(c))
#define Utolower(c) tolower(UC(c))

void *xmalloc(unsigned long size);
void *xrealloc(void *old, unsigned long size);

/*
void log_fatal(char *fmt, ...);
void log_misc(char *fmt, ...);
void log_debug(char *fmt, ...);
void log_panic(char *fmt, ...);
*/
