#ifndef foowarnhfoo
#define foowarnhfoo

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

/* This routine works on Linux only, so don't rely on it */
const char *avahi_exe_name(void);

void avahi_warn_unsupported(const char *function);

void avahi_warn_linkage(void);

void avahi_warn(const char *fmt, ...);

#define AVAHI_WARN_LINKAGE do { avahi_warn_linkage(); } while(0)
#define AVAHI_WARN_UNSUPPORTED do { avahi_warn_linkage(); avahi_warn_unsupported(__FUNCTION__); } while(0)
#define AVAHI_WARN_UNSUPPORTED_ABORT do { AVAHI_WARN_UNSUPPORTED; abort(); } while(0)

#endif
