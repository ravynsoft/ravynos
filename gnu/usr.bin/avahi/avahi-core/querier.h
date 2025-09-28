#ifndef fooquerierhfoo
#define fooquerierhfoo

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

typedef struct AvahiQuerier AvahiQuerier;

#include "iface.h"

/** Add querier for the specified key to the specified interface */
void avahi_querier_add(AvahiInterface *i, AvahiKey *key, struct timeval *ret_ctime);

/** Remove a querier for the specified key from the specified interface */
void avahi_querier_remove(AvahiInterface *i, AvahiKey *key);

/** Add a querier for the specified key on all interfaces that mach */
void avahi_querier_add_for_all(AvahiServer *s, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key, struct timeval *ret_ctime);

/** Remove a querier for the specified key on all interfaces that mach */
void avahi_querier_remove_for_all(AvahiServer *s, AvahiIfIndex idx, AvahiProtocol protocol, AvahiKey *key);

/** Free all queriers */
void avahi_querier_free(AvahiQuerier *q);

/** Free all queriers on the specified interface */
void avahi_querier_free_all(AvahiInterface *i);

/** Return 1 if there is a querier for the specified key on the specified interface */
int avahi_querier_shall_refresh_cache(AvahiInterface *i, AvahiKey *key);

#endif
