#ifndef fooqueryschedhfoo
#define fooqueryschedhfoo

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

typedef struct AvahiQueryScheduler AvahiQueryScheduler;

#include <avahi-common/address.h>
#include "iface.h"

AvahiQueryScheduler *avahi_query_scheduler_new(AvahiInterface *i);
void avahi_query_scheduler_free(AvahiQueryScheduler *s);
void avahi_query_scheduler_clear(AvahiQueryScheduler *s);

int avahi_query_scheduler_post(AvahiQueryScheduler *s, AvahiKey *key, int immediately, unsigned *ret_id);
int avahi_query_scheduler_withdraw_by_id(AvahiQueryScheduler *s, unsigned id);
void avahi_query_scheduler_incoming(AvahiQueryScheduler *s, AvahiKey *key);

#endif
