#ifndef foosimplewatchhfoo
#define foosimplewatchhfoo

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

/** \file simple-watch.h Simple poll() based main loop implementation */

#include <poll.h>
#include <avahi-common/cdecl.h>
#include <avahi-common/watch.h>

AVAHI_C_DECL_BEGIN

/** A main loop object. Main loops of this type aren't very flexible
 * since they only support a single wakeup type. Nevertheless it
 * should suffice for small test and example applications.  */
typedef struct AvahiSimplePoll AvahiSimplePoll;

/** Create a new main loop object */
AvahiSimplePoll *avahi_simple_poll_new(void);

/** Free a main loop object */
void avahi_simple_poll_free(AvahiSimplePoll *s);

/** Return the abstracted poll API object for this main loop
 * object. The is will return the same pointer each time it is
 * called. */
const AvahiPoll* avahi_simple_poll_get(AvahiSimplePoll *s);

/** Run a single main loop iteration of this main loop. If sleep_time
is < 0 this will block until any of the registered events happens,
then it will execute the attached callback function. If sleep_time is
0 the routine just checks if any event is pending. If yes the attached
callback function is called, otherwise the function returns
immediately. If sleep_time > 0 the function will block for at most the
specified time in msecs. Returns -1 on error, 0 on success and 1 if a
quit request has been scheduled. Usually this function should be called
in a loop until it returns a non-zero value*/
int avahi_simple_poll_iterate(AvahiSimplePoll *s, int sleep_time);

/** Request that the main loop quits. If this is called the next
 call to avahi_simple_poll_iterate() will return 1 */
void avahi_simple_poll_quit(AvahiSimplePoll *s);

/** Prototype for a poll() type function */
typedef int (*AvahiPollFunc)(struct pollfd *ufds, unsigned int nfds, int timeout, void *userdata);

/** Replace the internally used poll() function. By default the system's poll() will be used */
void avahi_simple_poll_set_func(AvahiSimplePoll *s, AvahiPollFunc func, void *userdata);

/** The first stage of avahi_simple_poll_iterate(), use this function only if you know what you do */
int avahi_simple_poll_prepare(AvahiSimplePoll *s, int timeout);

/** The second stage of avahi_simple_poll_iterate(), use this function only if you know what you do */
int avahi_simple_poll_run(AvahiSimplePoll *s);

/** The third and final stage of avahi_simple_poll_iterate(), use this function only if you know what you do */
int avahi_simple_poll_dispatch(AvahiSimplePoll *s);

/** Call avahi_simple_poll_iterate() in a loop and return if it returns non-zero */
int avahi_simple_poll_loop(AvahiSimplePoll *s);

/** Wakeup the main loop. (for threaded environments) */
void avahi_simple_poll_wakeup(AvahiSimplePoll *s);

AVAHI_C_DECL_END

#endif
