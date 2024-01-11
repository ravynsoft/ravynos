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

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <avahi-common/gccmacro.h>
#include <dns_sd.h>

static void reply(
        AVAHI_GCC_UNUSED DNSServiceRef sdRef,
        AVAHI_GCC_UNUSED DNSServiceFlags flags,
        AVAHI_GCC_UNUSED uint32_t interfaceIndex,
        AVAHI_GCC_UNUSED DNSServiceErrorType errorCode,
        AVAHI_GCC_UNUSED const char *serviceName,
        AVAHI_GCC_UNUSED const char *regtype,
        AVAHI_GCC_UNUSED const char *replyDomain,
        AVAHI_GCC_UNUSED void *context) {
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char*argv[]) {

    DNSServiceRef ref1, ref2, ref3, ref4 = NULL;

    DNSServiceRegister(&ref1, 0, 0, "simple", "_simple._tcp", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    DNSServiceRegister(&ref2, 0, 0, "subtype #1", "_simple._tcp,_subtype1", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    DNSServiceRegister(&ref3, 0, 0, "subtype #2", "_simple._tcp,_subtype1,_subtype2", NULL, NULL, 4711, 0, NULL, NULL, NULL);

    DNSServiceRegister(&ref4, 0, 0, "subtype #3", "_simple._tcp,,", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    assert(!ref4);
    DNSServiceRegister(&ref4, 0, 0, "subtype #3", "", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    assert(!ref4);
    DNSServiceRegister(&ref4, 0, 0, "subtype #3", ",", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    assert(!ref4);
    DNSServiceRegister(&ref4, 0, 0, "subtype #3", ",,", NULL, NULL, 4711, 0, NULL, NULL, NULL);
    assert(!ref4);

    DNSServiceBrowse(&ref4, 0, 0, "_simple._tcp,_gurke", NULL, reply, NULL);

    sleep(20);

    DNSServiceRefDeallocate(ref1);
    DNSServiceRefDeallocate(ref2);
    DNSServiceRefDeallocate(ref3);
    DNSServiceRefDeallocate(ref4);

    return 0;
}
