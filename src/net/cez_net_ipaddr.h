/* Copyright (c) 2020 Nikola Kolev <koue@chaosophia.net> */
/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

struct ipaddr {
    unsigned long version;
    unsigned char addr[16];
};

struct ipaddr *ipaddr_create(struct pool *pool);

BOOL ipaddr_copy(struct ipaddr *dst, struct ipaddr *src);

BOOL ipaddr_compare(struct ipaddr *addr1, struct ipaddr *addr2);

char *ipaddr_text(struct ipaddr *addr);

char *ipaddr_name(struct ipaddr *addr);

BOOL ipaddr_parse(struct ipaddr *addr, char *text);

BOOL ipaddr_compare_list(struct ipaddr *ipaddr, char *text);

void ipaddr_send_iostream(struct ipaddr *addr, struct iostream *stream);

BOOL ipaddr_fetch_iostream(struct ipaddr *addr, struct iostream *stream);

void
ipaddr_set(struct ipaddr *ipaddr, unsigned long ipvers,
           unsigned char *addr);
