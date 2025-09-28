#ifndef foothreadedwatchhfoo
#define foothreadedwatchhfoo

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

/** \file thread-watch.h Threaded poll() based main loop implementation */

#include <poll.h>
#include <avahi-common/cdecl.h>
#include <avahi-common/watch.h>

AVAHI_C_DECL_BEGIN

/** A main loop object that runs an AvahiSimplePoll in its own thread. \since 0.6.4 */
typedef struct AvahiThreadedPoll AvahiThreadedPoll;

/** Create a new event loop object. This will allocate the internal
 * AvahiSimplePoll, but will not start the helper thread. \since 0.6.4 */
AvahiThreadedPoll *avahi_threaded_poll_new(void);

/** Free an event loop object. This will stop the associated event loop
 * thread (if it is running). \since 0.6.4 */
void avahi_threaded_poll_free(AvahiThreadedPoll *p);

/** Return the abstracted poll API object for this event loop
 * object. The will return the same pointer each time it is
 * called. \since 0.6.4 */
const AvahiPoll* avahi_threaded_poll_get(AvahiThreadedPoll *p);

/** Start the event loop helper thread. After the thread has started
 * you must make sure to access the event loop object
 * (AvahiThreadedPoll, AvahiPoll and all its associated objects)
 * synchronized, i.e. with proper locking. You may want to use
 * avahi_threaded_poll_lock()/avahi_threaded_poll_unlock() for this,
 * which will lock the the entire event loop. Please note that event
 * loop callback functions are called from the event loop helper thread
 * with that lock held, i.e. avahi_threaded_poll_lock() calls are not
 * required from event callbacks. \since 0.6.4 */
int avahi_threaded_poll_start(AvahiThreadedPoll *p);

/** Request that the event loop quits and the associated thread
 stops. Call this from outside the helper thread if you want to shut
 it down. \since 0.6.4 */
int avahi_threaded_poll_stop(AvahiThreadedPoll *p);

/** Request that the event loop quits and the associated thread
 stops. Call this from inside the helper thread if you want to shut it
 down. \since 0.6.4  */
void avahi_threaded_poll_quit(AvahiThreadedPoll *p);

/** Lock the main loop object. Use this if you want to access the event
 * loop objects (such as creating a new event source) from anything
 * else but the event loop helper thread, i.e. from anything else but event
 * loop callbacks \since 0.6.4  */
void avahi_threaded_poll_lock(AvahiThreadedPoll *p);

/** Unlock the event loop object, use this as counterpart to
 * avahi_threaded_poll_lock() \since 0.6.4 */
void avahi_threaded_poll_unlock(AvahiThreadedPoll *p);

AVAHI_C_DECL_END

#endif
