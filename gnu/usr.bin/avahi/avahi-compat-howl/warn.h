#ifndef foowarnhhowlfoo
#define foowarnhhowlfoo

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

/* To avoid symbol name clashes when a process links to both our
 * compatiblity layers, we move the symbols out of the way here */

#define avahi_warn_unsupported avahi_warn_unsupported_HOWL
#define avahi_warn_linkage avahi_warn_linkage_HOWL
#define avahi_warn avahi_warn_HOWL
#define avahi_exe_name avahi_exe_name_HOWL

#include "../avahi-compat-libdns_sd/warn.h"

#endif
