#ifndef footimevalhfoo
#define footimevalhfoo

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

/** \file timeval.h Functions to facilitate timeval handling */

#include <inttypes.h>
#include <sys/time.h>

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

/** A numeric data type for storing microsecond values. (signed 64bit integer) */
typedef int64_t AvahiUsec;

/** Compare two timeval structures and return a negative value when a < b, 0 when a == b and a positive value otherwise */
int avahi_timeval_compare(const struct timeval *a, const struct timeval *b);

/** Calculate the difference between two timeval structures as microsecond value */
AvahiUsec avahi_timeval_diff(const struct timeval *a, const struct timeval *b);

/** Add a number of microseconds to the specified timeval structure and return it. *a is modified. */
struct timeval* avahi_timeval_add(struct timeval *a, AvahiUsec usec);

/** Return the difference between the current time and *a. Positive if *a was earlier */
AvahiUsec avahi_age(const struct timeval *a);

/** Fill *tv with the current time plus "ms" milliseconds plus an
 * extra jitter of "j" milliseconds. Pass 0 for j if you don't want
 * the jitter */
struct timeval *avahi_elapse_time(struct timeval *tv, unsigned ms, unsigned j);

AVAHI_C_DECL_END

#endif
