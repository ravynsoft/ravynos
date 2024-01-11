#ifndef fooavahimainhfoo
#define fooavahimainhfoo

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

typedef enum Event {
    EVENT_NULL,
    EVENT_PACKET,
    EVENT_TIMEOUT,
    EVENT_ROUTABLE_ADDR_CONFIGURED,
    EVENT_ROUTABLE_ADDR_UNCONFIGURED,
    EVENT_REFRESH_REQUEST
} Event;

typedef enum State {
    STATE_START,
    STATE_WAITING_PROBE,
    STATE_PROBING,
    STATE_WAITING_ANNOUNCE,
    STATE_ANNOUNCING,
    STATE_RUNNING,
    STATE_SLEEPING,
    STATE_MAX
} State;

int is_ll_address(uint32_t addr);

#endif
