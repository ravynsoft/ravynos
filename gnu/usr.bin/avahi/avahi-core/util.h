#ifndef fooutilhfoo
#define fooutilhfoo

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

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

void avahi_hexdump(const void *p, size_t size);

char *avahi_format_mac_address(char *t, size_t l, const uint8_t* mac, size_t size);

/** Change every character in the string to upper case (ASCII), return a pointer to the string */
char *avahi_strup(char *s);

/** Change every character in the string to lower case (ASCII), return a pointer to the string */
char *avahi_strdown(char *s);

AVAHI_C_DECL_END

#endif
