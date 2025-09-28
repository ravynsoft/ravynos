#ifndef LIBEVENT_WATCH_H
#define LIBEVENT_WATCH_H

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

/** \file libevent-watch.h libevent main loop adapter */

#include <event2/event.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/watch.h>

AVAHI_C_DECL_BEGIN

/** libevent main loop adapter */
typedef struct AvahiLibeventPoll AvahiLibeventPoll;

/** Create a new libevent main loop adapter attached to the specified
 event_base. */
AvahiLibeventPoll *avahi_libevent_poll_new(struct event_base *base);

/** Free libevent main loop adapter */
void avahi_libevent_poll_free(AvahiLibeventPoll *ep);

/** Quit libevent main loop adapter's thread if it has one */
void avahi_libevent_poll_quit(AvahiLibeventPoll *ep);

/** Return the abstract poll API structure for this object. This will
 * return the same pointer to an internally allocated structure on each
 * call */
const AvahiPoll *avahi_libevent_poll_get(AvahiLibeventPoll *ep);

AVAHI_C_DECL_END

#endif
