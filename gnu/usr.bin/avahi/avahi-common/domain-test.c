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

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "domain.h"
#include "malloc.h"

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
    char *s;
    char t[256], r[256];
    const char *p;
    size_t size;
    char name[64], type[AVAHI_DOMAIN_NAME_MAX], domain[AVAHI_DOMAIN_NAME_MAX];

    printf("%s\n", s = avahi_normalize_name_strdup("foo.foo\\046."));
    avahi_free(s);

    printf("%s\n", s = avahi_normalize_name_strdup("foo.foo\\.foo."));
    avahi_free(s);


    printf("%s\n", s = avahi_normalize_name_strdup("fo\\\\o\\..f oo."));
    avahi_free(s);

    printf("%i\n", avahi_domain_equal("\\065aa bbb\\.\\046cc.cc\\\\.dee.fff.", "Aaa BBB\\.\\.cc.cc\\\\.dee.fff"));
    printf("%i\n", avahi_domain_equal("A", "a"));

    printf("%i\n", avahi_domain_equal("a", "aaa"));

    printf("%u = %u\n", avahi_domain_hash("ccc\\065aa.aa\\.b\\\\."), avahi_domain_hash("cccAaa.aa\\.b\\\\"));


    avahi_service_name_join(t, sizeof(t), "foo.foo.foo \\.", "_http._tcp", "test.local");
    printf("<%s>\n", t);

    avahi_service_name_split(t, name, sizeof(name), type, sizeof(type), domain, sizeof(domain));
    printf("name: <%s>; type: <%s>; domain <%s>\n", name, type, domain);

    avahi_service_name_join(t, sizeof(t), NULL, "_http._tcp", "one.two\\. .local");
    printf("<%s>\n", t);

    avahi_service_name_split(t, NULL, 0, type, sizeof(type), domain, sizeof(domain));
    printf("name: <>; type: <%s>; domain <%s>\n", type, domain);


    p = "--:---\\\\\\123\\065_äöü\\064\\.\\\\sjöödfhh.sdfjhskjdf";
    printf("unescaped: <%s>, rest: %s\n", avahi_unescape_label(&p, t, sizeof(t)), p);

    size = sizeof(r);
    s = r;

    printf("escaped: <%s>\n", avahi_escape_label(t, strlen(t), &s, &size));

    p = r;
    printf("unescaped: <%s>\n", avahi_unescape_label(&p, t, sizeof(t)));

    assert(avahi_is_valid_service_type_generic("_foo._bar._waldo"));
    assert(!avahi_is_valid_service_type_strict("_foo._bar._waldo"));
    assert(!avahi_is_valid_service_subtype("_foo._bar._waldo"));

    assert(avahi_is_valid_service_type_generic("_foo._tcp"));
    assert(avahi_is_valid_service_type_strict("_foo._tcp"));
    assert(!avahi_is_valid_service_subtype("_foo._tcp"));

    assert(!avahi_is_valid_service_type_generic("_foo._bar.waldo"));
    assert(!avahi_is_valid_service_type_strict("_foo._bar.waldo"));
    assert(!avahi_is_valid_service_subtype("_foo._bar.waldo"));

    assert(!avahi_is_valid_service_type_generic(""));
    assert(!avahi_is_valid_service_type_strict(""));
    assert(!avahi_is_valid_service_subtype(""));

    assert(avahi_is_valid_service_type_generic("_foo._sub._bar._tcp"));
    assert(!avahi_is_valid_service_type_strict("_foo._sub._bar._tcp"));
    assert(avahi_is_valid_service_subtype("_foo._sub._bar._tcp"));

    printf("%s\n", avahi_get_type_from_subtype("_foo._sub._bar._tcp"));

    assert(!avahi_is_valid_host_name("sf.ooo."));
    assert(avahi_is_valid_host_name("sfooo."));
    assert(avahi_is_valid_host_name("sfooo"));

    assert(avahi_is_valid_domain_name("."));
    assert(avahi_is_valid_domain_name(""));

    assert(avahi_normalize_name(".", t, sizeof(t)));
    assert(avahi_normalize_name("", t, sizeof(t)));

    assert(!avahi_is_valid_fqdn("."));
    assert(!avahi_is_valid_fqdn(""));
    assert(!avahi_is_valid_fqdn("foo"));
    assert(avahi_is_valid_fqdn("foo.bar"));
    assert(avahi_is_valid_fqdn("foo.bar."));
    assert(avahi_is_valid_fqdn("gnurz.foo.bar."));
    assert(!avahi_is_valid_fqdn("192.168.50.1"));
    assert(!avahi_is_valid_fqdn("::1"));
    assert(!avahi_is_valid_fqdn(".192.168.50.1."));

    return 0;
}
