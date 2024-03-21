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

#ifdef HAVE_DLOPEN
#include <dlfcn.h>
#endif
#include <stdlib.h>

#include "client.h"

int avahi_nss_support(void) {
    int b = 0;

#ifdef HAVE_DLOPEN
    static const char * const libs[] = {
        "libnss_mdns.so.2",
        "libnss_mdns4.so.2",
        "libnss_mdns6.so.2",
        NULL };

    const char * const *l;

    for (l = libs; *l; l++) {
        void *dl;

        if ((dl = dlopen(*l, RTLD_LAZY))) {
            b = 1;
            dlclose(dl);
            break;
        }
    }
#endif

    return b;
}
