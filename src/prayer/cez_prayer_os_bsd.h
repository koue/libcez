/* $Cambridge: hermes/src/prayer/lib/os_bsd.h,v 1.1 2010/07/08 09:34:25 dpc22 Exp $ */

#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netdb.h>
#ifdef __GLIBC__
#include <sys/file.h>
#include <pty.h>
#else
#include <libutil.h>
#endif
