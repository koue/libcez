/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

BOOL os_socketpair(int *sockfd);

int os_connect_unix_socket(char *name);

int os_connect_inet_socket(char *host, unsigned long port);

int os_bind_unix_socket(char *name);

int *os_bind_inet_socket(unsigned long port, char *interface);

int os_accept_unix(int sockfd);

int os_accept_inet(int sockfd, struct ipaddr *ipaddr);

int os_socket_blocking(int sockfd);

int os_socket_nonblocking(int sockfd);

char *os_gethostbyaddr(void *addr, unsigned int version);

int os_inet_ntop(void *addr, unsigned long version, char *buf, unsigned long buflen);

int os_inet_pton(char *str, struct ipaddr *addr);

void os_child_reaper(void);

pid_t os_waitpid_nohang(void);

BOOL os_signal_child_init(void (*fn) (int));

BOOL os_signal_child_clear(void);

BOOL os_signal_alarm_init(void (*fn) (int));

BOOL os_signal_alarm_clear(void);

BOOL os_signal_init(void);

BOOL os_lock_exclusive(int fd);

BOOL os_lock_shared(int fd);

BOOL os_lock_release(int fd);

BOOL os_lock_exclusive_allow_break(int fd);

BOOL os_lock_shared_allow_break(int fd);

BOOL os_lock_release_allow_break(int fd);

BOOL os_random(struct ssl_config *ssl_config, void *buffer, unsigned long count);

void os_limit_vm(unsigned long x);

void os_prctl_set_dumpable(void);

BOOL os_run(char *cmdline, int *fdp, int *childpidp);
BOOL os_run_pty(char *cmdline, int *fdp, int *childpidp);
