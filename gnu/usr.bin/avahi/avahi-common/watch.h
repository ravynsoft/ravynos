#ifndef foowatchhfoo
#define foowatchhfoo

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

/** \file watch.h Simplistic main loop abstraction */

#include <poll.h>
#include <sys/time.h>

#include <avahi-common/cdecl.h>

AVAHI_C_DECL_BEGIN

/** An I/O watch object */
typedef struct AvahiWatch AvahiWatch;

/** A timeout watch object */
typedef struct AvahiTimeout AvahiTimeout;

/** An event polling abstraction object */
typedef struct AvahiPoll AvahiPoll;

/** Type of watch events */
typedef enum {
    AVAHI_WATCH_IN = POLLIN,      /**< Input event */
    AVAHI_WATCH_OUT = POLLOUT,    /**< Output event */
    AVAHI_WATCH_ERR = POLLERR,    /**< Error event */
    AVAHI_WATCH_HUP = POLLHUP     /**< Hangup event */
} AvahiWatchEvent;

/** Called whenever an I/O event happens  on an I/O watch */
typedef void (*AvahiWatchCallback)(AvahiWatch *w, int fd, AvahiWatchEvent event, void *userdata);

/** Called when the timeout is reached */
typedef void (*AvahiTimeoutCallback)(AvahiTimeout *t, void *userdata);

/** Defines an abstracted event polling API. This may be used to
 connect Avahi to other main loops. This is loosely based on Unix
 poll(2). A consumer will call watch_new() for all file descriptors it
 wants to listen for events on. In addition he can call timeout_new()
 to define time based events .*/
struct AvahiPoll {

    /** Some abstract user data usable by the provider of the API */
    void* userdata;

    /** Create a new watch for the specified file descriptor and for
     * the specified events. The API will call the callback function
     * whenever any of the events happens. */
    AvahiWatch* (*watch_new)(const AvahiPoll *api, int fd, AvahiWatchEvent event, AvahiWatchCallback callback, void *userdata);

    /** Update the events to wait for. It is safe to call this function from an AvahiWatchCallback */
    void (*watch_update)(AvahiWatch *w, AvahiWatchEvent event);

    /** Return the events that happened. It is safe to call this function from an AvahiWatchCallback  */
    AvahiWatchEvent (*watch_get_events)(AvahiWatch *w);

    /** Free a watch. It is safe to call this function from an AvahiWatchCallback */
    void (*watch_free)(AvahiWatch *w);

    /** Set a wakeup time for the polling loop. The API will call the
    callback function when the absolute time *tv is reached. If tv is
    NULL, the timeout is disabled. After the timeout expired the
    callback function will be called and the timeout is disabled. You
    can reenable it by calling timeout_update()  */
    AvahiTimeout* (*timeout_new)(const AvahiPoll *api, const struct timeval *tv, AvahiTimeoutCallback callback, void *userdata);

    /** Update the absolute expiration time for a timeout, If tv is
     * NULL, the timeout is disabled. It is safe to call this function from an AvahiTimeoutCallback */
    void (*timeout_update)(AvahiTimeout *, const struct timeval *tv);

    /** Free a timeout. It is safe to call this function from an AvahiTimeoutCallback */
    void (*timeout_free)(AvahiTimeout *t);
};

AVAHI_C_DECL_END

#endif

