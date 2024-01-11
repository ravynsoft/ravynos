#ifndef fooglibwatchhfoo
#define fooglibwatchhfoo

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

/** \file glib-watch.h GLib main loop adapter */

/** \example glib-integration.c Example of how to integrate
 * avahi use with GLIB/GTK applications */

#include <glib.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/watch.h>

AVAHI_C_DECL_BEGIN

/** GLib main loop adapter. You can safely cast this into a GSource */
typedef struct AvahiGLibPoll AvahiGLibPoll;

/** Create a new GLib main loop adapter attached to the specified
 context. If context is NULL, the default main loop context is
 used. You can attach as many AvahiGLibPoll objects to the same context
 as you want. priority takes one of GLib's G_PRIORITY constants. */
AvahiGLibPoll *avahi_glib_poll_new(GMainContext *context, gint priority);

/** Free  GLib main loop adapter */
void avahi_glib_poll_free(AvahiGLibPoll *g);

/** Return the abstract poll API structure for this object. This will
 * return the same pointer to a internally allocated structure on each
 * call */
const AvahiPoll *avahi_glib_poll_get(AvahiGLibPoll *g);

AVAHI_C_DECL_END

#endif
