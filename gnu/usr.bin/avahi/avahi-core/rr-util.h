#ifndef foorrutilhfoo
#define foorrutilhfoo

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

#include "rr.h"

AVAHI_C_DECL_BEGIN

/** Creaze new AvahiKey object based on an existing key but replaceing the type by CNAME */
AvahiKey *avahi_key_new_cname(AvahiKey *key);

/** Match a key to a key pattern. The pattern has a type of
AVAHI_DNS_CLASS_ANY, the classes are taken to be equal. Same for the
type. If the pattern has neither class nor type with ANY constants,
this function is identical to avahi_key_equal(). In contrast to
avahi_equal() this function is not commutative. */
int avahi_key_pattern_match(const AvahiKey *pattern, const AvahiKey *k);

/** Check whether a key is a pattern key, i.e. the class/type has a
 * value of AVAHI_DNS_CLASS_ANY/AVAHI_DNS_TYPE_ANY */
int avahi_key_is_pattern(const AvahiKey *k);

/** Returns a maximum estimate for the space that is needed to store
 * this key in a DNS packet. */
size_t avahi_key_get_estimate_size(AvahiKey *k);

/** Returns a maximum estimate for the space that is needed to store
 * the record in a DNS packet. */
size_t avahi_record_get_estimate_size(AvahiRecord *r);

/** Do a mDNS spec conforming lexicographical comparison of the two
 * records. Return a negative value if a < b, a positive if a > b,
 * zero if equal. */
int avahi_record_lexicographical_compare(AvahiRecord *a, AvahiRecord *b);

/** Return 1 if the specified record is an mDNS goodbye record. i.e. TTL is zero. */
int avahi_record_is_goodbye(AvahiRecord *r);

/** Make a deep copy of an AvahiRecord object */
AvahiRecord *avahi_record_copy(AvahiRecord *r);

AVAHI_C_DECL_END

#endif
