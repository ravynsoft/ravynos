#ifndef fooannouncehfoo
#define fooannouncehfoo

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

typedef struct AvahiAnnouncer AvahiAnnouncer;

#include <avahi-common/llist.h>
#include "iface.h"
#include "internal.h"
#include "timeeventq.h"
#include "publish.h"

typedef enum {
    AVAHI_PROBING,         /* probing phase */
    AVAHI_WAITING,         /* wait for other records in group */
    AVAHI_ANNOUNCING,      /* announcing phase */
    AVAHI_ESTABLISHED      /* we'e established */
} AvahiAnnouncerState;

struct AvahiAnnouncer {
    AvahiServer *server;
    AvahiInterface *interface;
    AvahiEntry *entry;

    AvahiTimeEvent *time_event;

    AvahiAnnouncerState state;
    unsigned n_iteration;
    unsigned sec_delay;

    AVAHI_LLIST_FIELDS(AvahiAnnouncer, by_interface);
    AVAHI_LLIST_FIELDS(AvahiAnnouncer, by_entry);
};

void avahi_announce_interface(AvahiServer *s, AvahiInterface *i);
void avahi_announce_entry(AvahiServer *s, AvahiEntry *e);
void avahi_announce_group(AvahiServer *s, AvahiSEntryGroup *g);

void avahi_entry_return_to_initial_state(AvahiServer *s, AvahiEntry *e, AvahiInterface *i);

void avahi_s_entry_group_check_probed(AvahiSEntryGroup *g, int immediately);

int avahi_entry_is_registered(AvahiServer *s, AvahiEntry *e, AvahiInterface *i);
int avahi_entry_is_probing(AvahiServer *s, AvahiEntry *e, AvahiInterface *i);

void avahi_goodbye_interface(AvahiServer *s, AvahiInterface *i, int send_goodbye, int rem);
void avahi_goodbye_entry(AvahiServer *s, AvahiEntry *e, int send_goodbye, int rem);

void avahi_reannounce_entry(AvahiServer *s, AvahiEntry *e);

#endif
