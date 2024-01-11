#ifndef foocdeclhfoo
#define foocdeclhfoo

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

/** \file cdecl.h C++ compatibility */
#ifdef __cplusplus
/** If using C++ this macro enables C mode, otherwise does nothing */
#define AVAHI_C_DECL_BEGIN extern "C" {
/** If using C++ this macro switches back to C++ mode, otherwise does nothing */
#define AVAHI_C_DECL_END }

#else
/** If using C++ this macro enables C mode, otherwise does nothing */
#define AVAHI_C_DECL_BEGIN
/** If using C++ this macro switches back to C++ mode, otherwise does nothing */
#define AVAHI_C_DECL_END

#endif

#endif
