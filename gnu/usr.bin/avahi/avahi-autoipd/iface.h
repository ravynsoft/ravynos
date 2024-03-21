#ifndef fooavahiifacehfoo
#define fooavahiifacehfoo

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

#include <inttypes.h>

#include "main.h"

/* Subscribe to network configuration changes. The only events we are
 * interested in are when routable addresses are removed/added to the
 * monitored interface and when our monitored interface disappears. */


/* Return a valid fd that we listen on for events */
int iface_init(int ifindex);

/* Process events */
int iface_process(Event *event);
void iface_done(void);

/* Deduce the initial state of our state machine. If a routable
 * address is configured for the interface, *state should be set to
 * STATE_SLEEPING, otherwise STATE_START */

int iface_get_initial_state(State *state);

#endif
