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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <avahi-common/domain.h>
#include <avahi-common/defs.h>
#include <avahi-common/malloc.h>

#include "dns.h"
#include "log.h"
#include "util.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    char t[AVAHI_DOMAIN_NAME_MAX], *m;
    const char *a, *b, *c, *d;
    AvahiDnsPacket *p;
    AvahiRecord *r, *r2;
    uint8_t rdata[AVAHI_DNS_RDATA_MAX];
    size_t l;

    p = avahi_dns_packet_new(0);

    assert(avahi_dns_packet_append_name(p, a = "Ahello.hello.hello.de."));
    assert(avahi_dns_packet_append_name(p, b = "Bthis is a test.hello.de."));
    assert(avahi_dns_packet_append_name(p, c = "Cthis\\.is\\.a\\.test\\.with\\.dots.hello.de."));
    assert(avahi_dns_packet_append_name(p, d = "Dthis\\\\is another test.hello.de."));

    avahi_hexdump(AVAHI_DNS_PACKET_DATA(p), p->size);

    assert(avahi_dns_packet_consume_name(p, t, sizeof(t)) == 0);
    avahi_log_debug(">%s<", t);
    assert(avahi_domain_equal(a, t));

    assert(avahi_dns_packet_consume_name(p, t, sizeof(t)) == 0);
    avahi_log_debug(">%s<", t);
    assert(avahi_domain_equal(b, t));

    assert(avahi_dns_packet_consume_name(p, t, sizeof(t)) == 0);
    avahi_log_debug(">%s<", t);
    assert(avahi_domain_equal(c, t));

    assert(avahi_dns_packet_consume_name(p, t, sizeof(t)) == 0);
    avahi_log_debug(">%s<", t);
    assert(avahi_domain_equal(d, t));

    avahi_dns_packet_free(p);

    /* RDATA PARSING AND SERIALIZATION */

    /* Create an AvahiRecord with some usful data */
    r = avahi_record_new_full("foobar.local", AVAHI_DNS_CLASS_IN, AVAHI_DNS_TYPE_HINFO, AVAHI_DEFAULT_TTL);
    assert(r);
    r->data.hinfo.cpu = avahi_strdup("FOO");
    r->data.hinfo.os = avahi_strdup("BAR");

    /* Serialize it into a blob */
    assert((l = avahi_rdata_serialize(r, rdata, sizeof(rdata))) != (size_t) -1);

    /* Print it */
    avahi_hexdump(rdata, l);

    /* Create a new record and fill in the data from the blob */
    r2 = avahi_record_new(r->key, AVAHI_DEFAULT_TTL);
    assert(r2);
    assert(avahi_rdata_parse(r2, rdata, l) >= 0);

    /* Compare both versions */
    assert(avahi_record_equal_no_ttl(r, r2));

    /* Free the records */
    avahi_record_unref(r);
    avahi_record_unref(r2);

    r = avahi_record_new_full("foobar", 77, 77, AVAHI_DEFAULT_TTL);
    assert(r);

    assert(r->data.generic.data = avahi_memdup("HALLO", r->data.generic.size = 5));

    m = avahi_record_to_string(r);
    assert(m);

    avahi_log_debug(">%s<", m);

    avahi_free(m);
    avahi_record_unref(r);

    return 0;
}
