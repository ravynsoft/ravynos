#ifndef fooglibmallochfoo
#define fooglibmallochfoo

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

/** \file glib-malloc.h GLib's memory allocator for Avahi */

#include <glib.h>

#include <avahi-common/cdecl.h>
#include <avahi-common/malloc.h>

AVAHI_C_DECL_BEGIN

/** Return a pointer to a memory allocator that uses GLib's g_malloc()
 and friends. The returned structure is statically allocated, and needs
 not to be copied or freed. Pass this directly to avahi_set_allocator(). */
const AvahiAllocator * avahi_glib_allocator(void);

AVAHI_C_DECL_END

#endif
