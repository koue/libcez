/* $Cambridge: hermes/src/prayer/lib/ipaddr.c,v 1.6 2009/08/20 12:37:55 dpc22 Exp $ */
/************************************************
 *    Prayer - a Webmail Interface              *
 ************************************************/

/* Copyright (c) University of Cambridge 2000 - 2008 */
/* See the file NOTICE for conditions of use and distribution. */

#include "cez_prayer.h"

/* Class for recording and comparing IP addresses. At the moment IPv4
 * only. Shouldn't be to hard to expand to IPv6 as well I hope */

/* ipaddr_create() *******************************************************
 *
 * Create a fresh ipaddr structure.
 ************************************************************************/

struct ipaddr *ipaddr_create(struct pool *pool)
{
    struct ipaddr *result = pool_alloc(pool, sizeof(struct ipaddr));

    return (result);
}

/* ipaddr_copy() *********************************************************
 *
 * Copy ip address.
 *     dst: Target
 *     src: Source
 ************************************************************************/

BOOL ipaddr_copy(struct ipaddr * dst, struct ipaddr * src)
{
    memcpy(dst, src, sizeof(struct ipaddr));

    return (T);
}

/* ipaddr_compare() ******************************************************
 *
 * Compare two ipaddr structures
 *    addr1: First ipaddr
 *    addr2: Second ipaddr
 *
 * Returns: T if ipaddrs match
 ************************************************************************/

BOOL ipaddr_compare(struct ipaddr * addr1, struct ipaddr * addr2)
{
    if (addr1->version != addr2->version)
        return (NIL);

    return ((!memcmp(&addr1->addr, &addr2->addr,
                     (addr1->version == 6) ? 16 : 4)) ? T : NIL);
                    
}

/* ====================================================================== */

/* ipaddr_text() *********************************************************
 *
 * Convert ipaddr to text representation using static buffer
 ************************************************************************/

char *ipaddr_text(struct ipaddr *addr)
{
    static char buf[64];

    os_inet_ntop(addr->addr, addr->version, buf, 64);

    return (buf);
}

/* ipaddr_text() *********************************************************
 *
 * Convert ipaddr to canonical hostname, using named pool
 ************************************************************************/

char *ipaddr_name(struct ipaddr *addr)
{
    char *result;

    if ((result = os_gethostbyaddr(addr->addr, addr->version)))
        return (result);

    return (ipaddr_text(addr));
}


/* ====================================================================== */

/* ipaddr_parse() ********************************************************
 *
 * Parse text representation of IP address
 *     addr: Target ipaddr
 *     text: Text to parse
 *
 * Returns: T if text parsed as correct IP address.
 ************************************************************************/

BOOL ipaddr_parse(struct ipaddr *ipaddr, char *text)
{
    if (text == NIL)
        return (NIL);

    return(os_inet_pton(text, ipaddr));
}

/* ====================================================================== */

/* ipaddr_compare_list() ************************************************
 *
 * Compare IP address to text list of form:
 * ipaddr:
 *   text: Text string of form "131.111.0.0/16 : 192.168.0.0/24 : 2001:12cd:1::/48".
 *         (There has to be a space on either side of the colon for it to 
 *          separate two networks)
 *
 * Returns: T if addr matches list.
 ************************************************************************/

BOOL ipaddr_compare_list(struct ipaddr * ipaddr, char *text)
{
    char *next = NULL, *s, *alloc;
    int i;
    unsigned long bits, mask;
    struct ipaddr parsed;
    BOOL match;

    alloc = text = pool_strdup(NIL, text);

    while (text && *text) {
        next = NULL;
        s = text;
        while ((s = strchr(s, ':'))) {
            if ((s > text) && (*(s - 1) == ' ' || *(s + 1) == ' ')) {
                *s++ = '\0';
                next = s;
                break;
            }
            s++;
        }

        text = string_trim_whitespace(text);

        if ((s = strchr(text, '/'))) {
            *s++ = '\0';
            bits = atoi(s);
        }
        else
            bits = 128;  /* Doesn't matter if it's too big */

        if (ipaddr_parse(&parsed, text)) {
            match = T;

            if (parsed.version != ipaddr->version) {
                text = next; continue;
            }

            for (i = 0; i < (parsed.version == 6 ? 16 : 4); i++) {
                if (bits == 0) mask = 0;
                else if (bits < 8) {
                    mask = ((-1) << (8 - bits)) & 0xff;
                    bits = 0;
                }
                else {
                    mask  = 0xff;
                    bits -= 8;
                }

                if ((parsed.addr[i] & mask) != (ipaddr->addr[i] & mask)) {
                    match = NIL;
                    break;
                }
            }
            if (match) {
                free(alloc);
                return (T);
            }
        }
        text = next;
    }
    free(alloc);
    return (NIL);
}

/* ====================================================================== */

/* ipaddr_send_iostream() ************************************************
 *
 * Send contents an raw ip address into iostream
 *    addr: Address to send
 *  stream: Target stream
 ************************************************************************/

void ipaddr_send_iostream(struct ipaddr *addr, struct iostream *stream)
{
    int i;
    ioputc(addr->version, stream);
    for (i = 0; i < (addr->version == 6 ? 16 : 4); i++) {
        ioputc(addr->addr[i], stream);
    }
}

/* ====================================================================== */

/* ipaddr_fetch_iostream() ***********************************************
 *
 * Parse raw ipaddr from iostream.
 ************************************************************************/

BOOL ipaddr_fetch_iostream(struct ipaddr *addr, struct iostream *stream)
{
    BOOL rc = T;
    int i, c;

    memset(addr->addr, 0, 16);

    if ((c = iogetc(stream)) != EOF) {
        addr->version = (unsigned char) c;
        for (i = 0; i < (addr->version == 6 ? 16 : 4); i++) {
            if ((c = iogetc(stream)) == EOF) {
                rc = NIL;
                break;
            }
            addr->addr[i] = (unsigned char) c;
        }
    }

    if (c == EOF) {
        log_panic
            ("Unexpected disconnect receiving IP address from frontend");
        rc = NIL;
    }

    return (rc);
}

/* ====================================================================== */

/* ipaddr_set() **********************************************************
 *
 * Set IP address in ipaddr structure
 *    ipaddr: Target ipaddr structure
 *    ipvers: IP version
 *      addr: Address string
 ************************************************************************/

void
ipaddr_set(struct ipaddr *ipaddr, unsigned long version,
           unsigned char *addr)
{
    unsigned char ipv4_prefix[] = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0xff, 0xff
    };

    if (version != 4 && version != 6)
        log_fatal("ipaddr_set(): IPv4 and IPv6 only supported!");

    ipaddr->version = version;
    memset(ipaddr->addr, 0, 16);
    memcpy(ipaddr->addr, addr, version == 6 ? 16 : 4);

    if ((ipaddr->version == 6) && !memcmp(ipaddr->addr, ipv4_prefix, 12)) {
        memcpy(ipaddr->addr, &ipaddr->addr[12], 4);
        memset(&ipaddr->addr[4], 0, 12);
        ipaddr->version = 4;
    }

}
