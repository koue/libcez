/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2010 */
/* See the file NOTICE for conditions of use and distribution. */

#include <sys/param.h>

#include "cez_net.h"
#include "cez_net_bsd.h"

/* ====================================================================== */

BOOL os_socketpair(int *sockfd)
{
    int rc;

    do {
        rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd);
    }
    while ((rc < 0) && (errno == EINTR));

    return ((rc == 0) ? T : NIL);
}

/* ====================================================================== */

int os_connect_unix_socket(char *name)
{
    struct sockaddr_un serv_addr;
    int sockfd, servlen;

    /* Open the socket */

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return (-1);

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;

    strncpy(serv_addr.sun_path, name, sizeof(serv_addr.sun_path)-1);
    servlen = sizeof(serv_addr);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, servlen) < 0) {
        close(sockfd);
        return (-1);
    }

    return (sockfd);
}

int os_connect_inet_socket(char *host, unsigned long port)
{
    struct addrinfo *first_ai, *ai;
    char port_str[NI_MAXSERV];
    int sockfd;

    snprintf(port_str, sizeof(port_str), "%lu", port);
    if (getaddrinfo(host, port_str, NULL, &first_ai)) {
        return (-1);
    }
    for (ai = first_ai; ai->ai_next; ai = ai->ai_next) {
        /* Open the socket */
        if ((sockfd = socket(ai->ai_family, SOCK_STREAM, 0)) < 0) {
            break;
        }
        if (connect(sockfd, (struct sockaddr *) ai->ai_addr,
                    ai->ai_addrlen) < 0) {
            close(sockfd);
            break;
        }
        freeaddrinfo(first_ai);
        return (sockfd);
    }
    freeaddrinfo(first_ai);
    return (-1);
}

/* ====================================================================== */

int os_bind_unix_socket(char *name)
{
    struct sockaddr_un serv_addr;
    int sockfd, servlen;
    int i;

    /* Generate well known connect address for frontend servers */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;

    strncpy(serv_addr.sun_path, name, sizeof(serv_addr.sun_path)-1);
    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    unlink(serv_addr.sun_path);

    /* Open the socket */
    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return (-1);
    }

    /* Set socket reuseaddr, otherwise bind will fail after fast stop/start */
    i = 1;
    if (setsockopt
        (sockfd, SOL_SOCKET, SO_REUSEADDR, (void *) &i, sizeof(int))) {
        close(sockfd);
        return (-1);
    }

    /* bind() as UNIX domain socket */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) <
        0) {
        close(sockfd);
        return (-1);
    }

    /* Requests should queue on sockfd until we are ready to serve them */
    if (listen(sockfd, 10) < 0) {
        close(sockfd);
        return (-1);
    }
    return (sockfd);
}

/* ====================================================================== */

int *os_bind_inet_socket(unsigned long port, char *interface)
{
    int optval, sockfd;
    struct addrinfo *ai, *ai_list, ai_hints;
    char port_str[NI_MAXSERV];
    int j;
    int count = 0;
    int *results = NIL;

    snprintf(port_str, sizeof(port_str), "%lu", port);
    bzero((void *) &ai_hints, sizeof(ai_hints));
    ai_hints.ai_flags = AI_PASSIVE|AI_ADDRCONFIG;
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(interface, port_str, &ai_hints, &ai_list)) {
        return(NIL);
    }

    count=0;
    for (ai=ai_list; ai; ai = ai->ai_next)
        count++;

    if (!(results = xmalloc((count+1)*sizeof(int)))) {
        return(NIL);
    }
    results[count] = -1;

    j=0;
    for (ai=ai_list; ai; ai=ai->ai_next) {
        sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sockfd < 0) {
            return(NIL);
        }

        /* set reuseaddr, otherwise bind will fail after fast stop/start */
        optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                       (void *) &optval, sizeof(int))) {
            close(sockfd);
            return(NIL);
        }

#ifdef IPV6_V6ONLY
        if (ai->ai_family == AF_INET6) {
            /* Linux refuses to bind to both '0.0.0.0' and '::' without
               IPV6_V6ONLY option. Alternative approach would be to reorder
               the list so that '::' appears first and only try binding to
               '0.0.0.0' if '::' fails. Exim daemon.c has long explanation.

               IPV6_V6ONLY is good enough for our needs on Hermes */

            /* Ignore errors here */
            optval = 1;
            setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY,
                       (void *) &optval, sizeof(int));
        }
#endif

        if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) < 0) {
            close(sockfd);
            return(NIL);
        }

        if (listen(sockfd, 10) < 0) {
            close(sockfd);
            return(NIL);
        }
        results[j++] = sockfd;
    }
    results[j] = -1;

    freeaddrinfo(ai_list);

    return(results);
}

/* ====================================================================== */

int os_accept_unix(int sockfd)
{
    struct sockaddr_un addr;
    socklen_t len = (socklen_t) sizeof(struct sockaddr_un);
    int newsockfd;

    do {
        newsockfd = accept(sockfd, (struct sockaddr *) &addr, &len);
    }
    while ((newsockfd < 0) && (errno == EINTR));

    if (newsockfd < 0) {
        close(newsockfd);
        return (-1);
    }

    /* Set close on exec so subprocesses can't interfere */
    if (fcntl(newsockfd, F_SETFD, FD_CLOEXEC) < 0) {
        close(newsockfd);
        return (-1);
    }

    return (newsockfd);
}

int os_accept_inet(int sockfd, struct ipaddr *ipaddr)
{
    struct sockaddr_storage addr;
    socklen_t len = (socklen_t) sizeof(addr);
    int newsockfd;

    do {
        newsockfd = accept(sockfd, (struct sockaddr *) &addr, &len);
    }
    while ((newsockfd < 0) && (errno == EINTR));

    if (newsockfd < 0) {
        close(newsockfd);
        return (-1);
    }

    if (ipaddr) {
        if (addr.ss_family == AF_INET6)
            ipaddr_set(ipaddr, 6, (unsigned char *)
                       &((struct sockaddr_in6*)&addr)->sin6_addr);
        else
            ipaddr_set(ipaddr, 4, (unsigned char *)
                       &((struct sockaddr_in*)&addr)->sin_addr);
    }

    /* Set close on exec so subprocesses can't interfere */
    if (fcntl(newsockfd, F_SETFD, FD_CLOEXEC) < 0) {
        close(newsockfd);
        return (-1);
    }

    return (newsockfd);
}

/* ====================================================================== */

int os_socket_blocking(int sockfd)
{
    int mode;

    mode = fcntl(sockfd, F_GETFL, 0);
    mode &= ~O_NDELAY;

    if (fcntl(sockfd, F_SETFL, mode) != 0) {
        return (NIL);
    }
    return (T);
}


int os_socket_nonblocking(int sockfd)
{
    int mode;

    mode = fcntl(sockfd, F_GETFL, 0);
    mode |= O_NDELAY;

    if (fcntl(sockfd, F_SETFL, mode) != 0) {
        return (NIL);
    }
    return (T);
}

/* ====================================================================== */

/* Convert IP address into text form */

char *os_gethostbyaddr(void *opaque, unsigned int version)
{
    struct hostent *hostent;

    if (version == 6)
        hostent = gethostbyaddr(opaque, 16, AF_INET6);
    else
        hostent = gethostbyaddr(opaque, 4, AF_INET);

    if (hostent && hostent->h_name && hostent->h_name[0])
        return (hostent->h_name);

    return (NIL);
}

int os_inet_ntop(void *addr, unsigned long version,
		 char *buf, unsigned long buflen) {
    if (version == 6) {
        if (inet_ntop(AF_INET6, addr, buf, buflen))
            return (T);
    }
    else {
        if (inet_ntop(AF_INET, addr, buf, buflen))
            return (T);
    }
    return (NIL);
}

int os_inet_pton(char *str, struct ipaddr *addr) {
    unsigned char buf[16];

    if (inet_pton(AF_INET6, str, buf)) {
        ipaddr_set(addr, 6, buf);
        return (T);
    }
    else if (inet_pton(AF_INET, str, buf)) {
        ipaddr_set(addr, 4, buf);
        return (T);
    }
    return (NIL);
}
