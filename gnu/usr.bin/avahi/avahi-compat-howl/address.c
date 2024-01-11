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
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <avahi-common/gccmacro.h>

#include "howl.h"
#include "warn.h"

sw_ipv4_address sw_ipv4_address_any(void) {
    sw_ipv4_address a;

    AVAHI_WARN_LINKAGE;

    a.m_addr = htonl(INADDR_ANY);
    return a;
}

sw_ipv4_address sw_ipv4_address_loopback(void) {
    sw_ipv4_address a;

    AVAHI_WARN_LINKAGE;

    a.m_addr = htonl(INADDR_LOOPBACK);
    return a;
}

sw_result sw_ipv4_address_init(sw_ipv4_address * self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    self->m_addr = htonl(INADDR_ANY);
    return SW_OKAY;
}

sw_result sw_ipv4_address_init_from_saddr(
    sw_ipv4_address *self,
    sw_saddr addr) {

    assert(self);

    AVAHI_WARN_LINKAGE;

    self->m_addr = addr;
    return SW_OKAY;
}

sw_result sw_ipv4_address_init_from_name(
    sw_ipv4_address *self,
    sw_const_string name) {

    struct hostent *he;

    assert(self);
    assert(name);

    AVAHI_WARN_LINKAGE;

    if (!(he = gethostbyname(name)))
        return SW_E_UNKNOWN;

    self->m_addr = *(uint32_t*) he->h_addr;
    return SW_OKAY;
}

sw_result sw_ipv4_address_init_from_address(
    sw_ipv4_address *self,
    sw_ipv4_address addr) {

    assert(self);

    AVAHI_WARN_LINKAGE;

    self->m_addr = addr.m_addr;
    return SW_OKAY;
}

sw_result sw_ipv4_address_init_from_this_host(sw_ipv4_address *self) {
    struct sockaddr_in sa;
    int fd;
    socklen_t l = sizeof(sa);

    assert(self);

    AVAHI_WARN_LINKAGE;

    /* This is so fucked up ... */

    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("192.168.1.1"); /* Ouch */
    sa.sin_port = htons(5555);

    if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0 ||
        connect(fd, (struct sockaddr*) &sa, sizeof(sa)) < 0 ||
        getsockname(fd, (struct sockaddr*) &sa, &l) < 0) {
        if (fd >= 0)
            close(fd);

        perror("fuck");
        return SW_E_UNKNOWN;
    }

    assert(l == sizeof(sa));
    close(fd);

    self->m_addr = sa.sin_addr.s_addr;

    return SW_OKAY;
}

sw_result sw_ipv4_address_fina(AVAHI_GCC_UNUSED sw_ipv4_address self) {

    AVAHI_WARN_LINKAGE;

    /* This is ridiculous ... */

    return SW_OKAY;
}

sw_bool sw_ipv4_address_is_any(sw_ipv4_address self) {
    AVAHI_WARN_LINKAGE;
    return self.m_addr == htonl(INADDR_ANY);
}

sw_saddr sw_ipv4_address_saddr(sw_ipv4_address self) {
    AVAHI_WARN_LINKAGE;
    return self.m_addr;
}

sw_string sw_ipv4_address_name(
    sw_ipv4_address self,
    sw_string name,
    sw_uint32 len) {

    assert(name);
    assert(len > 0);

    AVAHI_WARN_LINKAGE;

    if (len < INET_ADDRSTRLEN)
        return NULL;

    if (!(inet_ntop(AF_INET, &self.m_addr, name, len)))
        return NULL;

    return name;
}

sw_result sw_ipv4_address_decompose(
    sw_ipv4_address self,
    sw_uint8 * a1,
    sw_uint8 * a2,
    sw_uint8 * a3,
    sw_uint8 * a4) {

    uint32_t a;

    AVAHI_WARN_LINKAGE;

    a = ntohl(self.m_addr);

    assert(a1);
    assert(a2);
    assert(a3);
    assert(a4);

    *a1 = (uint8_t) (a >> 24);
    *a2 = (uint8_t) (a >> 16);
    *a3 = (uint8_t) (a >> 8);
    *a4 = (uint8_t) (a);

    return SW_OKAY;
}

sw_bool sw_ipv4_address_equals(
    sw_ipv4_address self,
    sw_ipv4_address addr) {

    AVAHI_WARN_LINKAGE;

    return self.m_addr == addr.m_addr;
}

