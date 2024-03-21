/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

/* Regression test for Avahi bug #84.
 * This program tests whether the avahi_dns_packet_consume_name function
 * returns (rather than spinning forever). For a function as simple as
 * avahi_dns_packet_consume_name, we assume that 1 second of CPU time ≈ forever
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include "dns.h"

#define MAX_CPU_SECONDS 1

#define TEST_NAME "dns-spin-test"

static void fail(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
static void unresolved(const char *fmt, ...) __attribute__((format(printf, 1, 2), noreturn));
static void stdlib_fail(const char *msg) __attribute__((noreturn));
static void handle(int sig) __attribute__((noreturn));

void stdlib_fail(const char *msg) {
    perror(msg);

    printf("UNRESOLVED: " TEST_NAME " (stdlib failure)\n");

    exit(77);
}

void unresolved(const char *fmt, ...) {
    va_list ap;

    printf("UNRESOLVED: " TEST_NAME ": ");
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");

    exit(77);
}

void fail(const char *fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");

    exit(EXIT_FAILURE);
}

void handle(AVAHI_GCC_UNUSED int sig) {
    fail("Interrupted after %d second of CPU time", MAX_CPU_SECONDS);
}

#define TRY_EXCEPT(cmd, badresult) \
    do { \
        if ((cmd) == (badresult)) \
            unresolved("%s returned %s", #cmd, #badresult); \
    } while (0)

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    struct itimerval itval;
    AvahiDnsPacket *packet;
    char name[512];
    int ret;
    uint8_t badrr[] = {
        0xC0, AVAHI_DNS_PACKET_HEADER_SIZE, /* self-referential QNAME pointer */
        0, 1, /* QTYPE A (host addr) */
        0, 1, /* QCLASS IN (internet/ipv4) */
    };

    if (signal(SIGVTALRM, handle) == SIG_ERR)
        stdlib_fail("signal(SIGVTALRM)");

    memset(&itval, 0, sizeof(itval));
    itval.it_value.tv_sec = MAX_CPU_SECONDS;

    if (setitimer(ITIMER_VIRTUAL, &itval, NULL) == -1)
        stdlib_fail("setitimer()");

    TRY_EXCEPT(packet = avahi_dns_packet_new_query(512), NULL);
    TRY_EXCEPT(avahi_dns_packet_append_bytes(packet, badrr, sizeof(badrr)), NULL);

    /* This is expected to fail (if it returns) */
    ret = avahi_dns_packet_consume_name(packet, name, sizeof(name));

    if (ret != -1)
        fail("avahi_dns_packet_consume_name() returned %d; -1 was expected", ret);

    return EXIT_SUCCESS;
}

/* vim:ts=4:sw=4:et
 */
