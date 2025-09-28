#ifndef footimeeventqhfoo
#define footimeeventqhfoo

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

#include <sys/types.h>

typedef struct AvahiTimeEventQueue AvahiTimeEventQueue;
typedef struct AvahiTimeEvent AvahiTimeEvent;

#include <avahi-common/watch.h>

#include "prioq.h"

typedef void (*AvahiTimeEventCallback)(AvahiTimeEvent *e, void* userdata);

AvahiTimeEventQueue* avahi_time_event_queue_new(const AvahiPoll *poll_api);
void avahi_time_event_queue_free(AvahiTimeEventQueue *q);

AvahiTimeEvent* avahi_time_event_new(
    AvahiTimeEventQueue *q,
    const struct timeval *timeval,
    AvahiTimeEventCallback callback,
    void* userdata);

void avahi_time_event_free(AvahiTimeEvent *e);
void avahi_time_event_update(AvahiTimeEvent *e, const struct timeval *timeval);

#endif
