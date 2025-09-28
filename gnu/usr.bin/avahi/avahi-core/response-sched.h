#ifndef fooresponseschedhfoo
#define fooresponseschedhfoo

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

typedef struct AvahiResponseScheduler AvahiResponseScheduler;

#include <avahi-common/address.h>
#include "iface.h"

AvahiResponseScheduler *avahi_response_scheduler_new(AvahiInterface *i);
void avahi_response_scheduler_free(AvahiResponseScheduler *s);
void avahi_response_scheduler_clear(AvahiResponseScheduler *s);
void avahi_response_scheduler_force(AvahiResponseScheduler *s);

int avahi_response_scheduler_post(AvahiResponseScheduler *s, AvahiRecord *record, int flush_cache, const AvahiAddress *querier, int immediately);
void avahi_response_scheduler_incoming(AvahiResponseScheduler *s, AvahiRecord *record, int flush_cache);
void avahi_response_scheduler_suppress(AvahiResponseScheduler *s, AvahiRecord *record, const AvahiAddress *querier);

#endif
