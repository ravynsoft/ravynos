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

#include <stdlib.h>

#include "rlist.h"
#include "malloc.h"

AvahiRList* avahi_rlist_prepend(AvahiRList *r, void *data) {
    AvahiRList *n;

    if (!(n = avahi_new(AvahiRList, 1)))
        return NULL;

    n->data = data;

    AVAHI_LLIST_PREPEND(AvahiRList, rlist, r, n);
    return r;
}

AvahiRList* avahi_rlist_remove(AvahiRList *r, void *data) {
    AvahiRList *n;

    for (n = r; n; n = n->rlist_next)

        if (n->data == data) {
            AVAHI_LLIST_REMOVE(AvahiRList, rlist, r, n);
            avahi_free(n);
            break;
        }

    return r;
}

AvahiRList* avahi_rlist_remove_by_link(AvahiRList *r, AvahiRList *n) {
    assert(n);

    AVAHI_LLIST_REMOVE(AvahiRList, rlist, r, n);
    avahi_free(n);

    return r;
}
