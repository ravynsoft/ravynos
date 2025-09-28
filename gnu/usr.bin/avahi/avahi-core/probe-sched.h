#ifndef fooprobeschedhfoo
#define fooprobeschedhfoo

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

typedef struct AvahiProbeScheduler AvahiProbeScheduler;

#include <avahi-common/address.h>
#include "iface.h"

AvahiProbeScheduler *avahi_probe_scheduler_new(AvahiInterface *i);
void avahi_probe_scheduler_free(AvahiProbeScheduler *s);
void avahi_probe_scheduler_clear(AvahiProbeScheduler *s);

int avahi_probe_scheduler_post(AvahiProbeScheduler *s, AvahiRecord *record, int immediately);

#endif
