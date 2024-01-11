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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <assert.h>
#include <stdio.h>

#include "malloc.h"
#include "timeval.h"
#include "dbus-watch-glue.h"

static AvahiWatchEvent translate_dbus_to_avahi(unsigned int f) {
    AvahiWatchEvent e = 0;

    if (f & DBUS_WATCH_READABLE)
        e |= AVAHI_WATCH_IN;
    if (f & DBUS_WATCH_WRITABLE)
        e |= AVAHI_WATCH_OUT;
    if (f & DBUS_WATCH_ERROR)
        e |= AVAHI_WATCH_ERR;
    if (f & DBUS_WATCH_HANGUP)
        e |= AVAHI_WATCH_HUP;

    return e;
}

static unsigned int translate_avahi_to_dbus(AvahiWatchEvent e) {
    unsigned int f = 0;

    if (e & AVAHI_WATCH_IN)
        f |= DBUS_WATCH_READABLE;
    if (e & AVAHI_WATCH_OUT)
        f |= DBUS_WATCH_WRITABLE;
    if (e & AVAHI_WATCH_ERR)
        f |= DBUS_WATCH_ERROR;
    if (e & AVAHI_WATCH_HUP)
        f |= DBUS_WATCH_HANGUP;

    return f;
}

typedef struct {
    DBusConnection *connection;
    const AvahiPoll *poll_api;
    AvahiTimeout *dispatch_timeout;
    int ref;
} ConnectionData;

static ConnectionData *connection_data_ref(ConnectionData *d) {
    assert(d);
    assert(d->ref >= 1);

    d->ref++;
    return d;
}

static void connection_data_unref(ConnectionData *d) {
    assert(d);
    assert(d->ref >= 1);

    if (--d->ref <= 0) {
        d->poll_api->timeout_free(d->dispatch_timeout);
        avahi_free(d);
    }
}

static void request_dispatch(ConnectionData *d, int enable) {
    static const struct timeval tv = { 0, 0 };
    assert(d);

    if (enable) {
        assert(dbus_connection_get_dispatch_status(d->connection) == DBUS_DISPATCH_DATA_REMAINS);
        d->poll_api->timeout_update(d->dispatch_timeout, &tv);
    } else
        d->poll_api->timeout_update(d->dispatch_timeout, NULL);
}

static void dispatch_timeout_callback(AvahiTimeout *t, void *userdata) {
    ConnectionData *d = userdata;
    assert(t);
    assert(d);

    connection_data_ref(d);
    dbus_connection_ref(d->connection);

    if (dbus_connection_dispatch(d->connection) == DBUS_DISPATCH_DATA_REMAINS)
        /* If there's still data, request that this handler is called again */
        request_dispatch(d, 1);
    else
        request_dispatch(d, 0);

    dbus_connection_unref(d->connection);
    connection_data_unref(d);
}

static void watch_callback(AvahiWatch *avahi_watch, AVAHI_GCC_UNUSED int fd, AvahiWatchEvent events, void *userdata) {
    DBusWatch *dbus_watch = userdata;

    assert(avahi_watch);
    assert(dbus_watch);

    dbus_watch_handle(dbus_watch, translate_avahi_to_dbus(events));
    /* Ignore the return value */
}

static dbus_bool_t update_watch(const AvahiPoll *poll_api, DBusWatch *dbus_watch) {
    AvahiWatch *avahi_watch;
    dbus_bool_t b;

    assert(dbus_watch);

    avahi_watch = dbus_watch_get_data(dbus_watch);

    b = dbus_watch_get_enabled(dbus_watch);

    if (b && !avahi_watch) {

        if (!(avahi_watch = poll_api->watch_new(
                  poll_api,
#if (DBUS_VERSION_MAJOR == 1 && DBUS_VERSION_MINOR == 1 && DBUS_VERSION_MICRO >= 1) || (DBUS_VERSION_MAJOR == 1 && DBUS_VERSION_MINOR > 1) || (DBUS_VERSION_MAJOR > 1)
                  dbus_watch_get_unix_fd(dbus_watch),
#else
                  dbus_watch_get_fd(dbus_watch),
#endif
                  translate_dbus_to_avahi(dbus_watch_get_flags(dbus_watch)),
                  watch_callback,
                  dbus_watch)))
            return FALSE;

        dbus_watch_set_data(dbus_watch, avahi_watch, NULL);

    } else if (!b && avahi_watch) {

        poll_api->watch_free(avahi_watch);
        dbus_watch_set_data(dbus_watch, NULL, NULL);

    } else if (avahi_watch) {

        /* Update flags */
        poll_api->watch_update(avahi_watch, dbus_watch_get_flags(dbus_watch));
    }

    return TRUE;
}

static dbus_bool_t add_watch(DBusWatch *dbus_watch, void *userdata) {
    ConnectionData *d = userdata;

    assert(dbus_watch);
    assert(d);

    return update_watch(d->poll_api, dbus_watch);
}

static void remove_watch(DBusWatch *dbus_watch, void *userdata) {
    ConnectionData *d = userdata;
    AvahiWatch *avahi_watch;

    assert(dbus_watch);
    assert(d);

    if ((avahi_watch = dbus_watch_get_data(dbus_watch))) {
        d->poll_api->watch_free(avahi_watch);
        dbus_watch_set_data(dbus_watch, NULL, NULL);
    }
}

static void watch_toggled(DBusWatch *dbus_watch, void *userdata) {
    ConnectionData *d = userdata;

    assert(dbus_watch);
    assert(d);

    update_watch(d->poll_api, dbus_watch);
}

typedef struct TimeoutData {
    const AvahiPoll *poll_api;
    AvahiTimeout *avahi_timeout;
    DBusTimeout *dbus_timeout;
    int ref;
} TimeoutData;

static TimeoutData* timeout_data_ref(TimeoutData *t) {
    assert(t);
    assert(t->ref >= 1);

    t->ref++;
    return t;
}

static void timeout_data_unref(TimeoutData *t) {
    assert(t);
    assert(t->ref >= 1);

    if (--t->ref <= 0) {
        if (t->avahi_timeout)
            t->poll_api->timeout_free(t->avahi_timeout);

        avahi_free(t);
    }
}

static void update_timeout(TimeoutData *timeout) {
    assert(timeout);
    assert(timeout->ref >= 1);

    if (dbus_timeout_get_enabled(timeout->dbus_timeout)) {
        struct timeval tv;
        avahi_elapse_time(&tv, dbus_timeout_get_interval(timeout->dbus_timeout), 0);
        timeout->poll_api->timeout_update(timeout->
                                      avahi_timeout, &tv);
    } else
        timeout->poll_api->timeout_update(timeout->avahi_timeout, NULL);

}

static void timeout_callback(AvahiTimeout *avahi_timeout, void *userdata) {
    TimeoutData *timeout = userdata;

    assert(avahi_timeout);
    assert(timeout);

    timeout_data_ref(timeout);

    dbus_timeout_handle(timeout->dbus_timeout);
    /* Ignore the return value */

    if (timeout->avahi_timeout)
        update_timeout(timeout);

    timeout_data_unref(timeout);
}

static dbus_bool_t add_timeout(DBusTimeout *dbus_timeout, void *userdata) {
    TimeoutData *timeout;
    ConnectionData *d = userdata;
    struct timeval tv;
    dbus_bool_t b;

    assert(dbus_timeout);
    assert(d);

    if (!(timeout = avahi_new(TimeoutData, 1)))
        return FALSE;

    timeout->dbus_timeout = dbus_timeout;
    timeout->poll_api = d->poll_api;
    timeout->ref = 1;

    if ((b = dbus_timeout_get_enabled(dbus_timeout)))
        avahi_elapse_time(&tv, dbus_timeout_get_interval(dbus_timeout), 0);

    if (!(timeout->avahi_timeout = d->poll_api->timeout_new(
              d->poll_api,
              b ? &tv : NULL,
              timeout_callback,
              timeout))) {
        avahi_free(timeout);
        return FALSE;
    }

    dbus_timeout_set_data(dbus_timeout, timeout, (DBusFreeFunction) timeout_data_unref);
    return TRUE;
}

static void remove_timeout(DBusTimeout *dbus_timeout, void *userdata) {
    ConnectionData *d = userdata;
    TimeoutData *timeout;

    assert(dbus_timeout);
    assert(d);

    timeout = dbus_timeout_get_data(dbus_timeout);
    assert(timeout);

    d->poll_api->timeout_free(timeout->avahi_timeout);
    timeout->avahi_timeout = NULL;
}

static void timeout_toggled(DBusTimeout *dbus_timeout, AVAHI_GCC_UNUSED void *userdata) {
    TimeoutData *timeout;

    assert(dbus_timeout);
    timeout = dbus_timeout_get_data(dbus_timeout);
    assert(timeout);

    update_timeout(timeout);
}

static void dispatch_status(AVAHI_GCC_UNUSED DBusConnection *connection, DBusDispatchStatus new_status, void *userdata) {
    ConnectionData *d = userdata;

    if (new_status == DBUS_DISPATCH_DATA_REMAINS)
        request_dispatch(d, 1);
 }

int avahi_dbus_connection_glue(DBusConnection *c, const AvahiPoll *poll_api) {
    ConnectionData *d = NULL;

    assert(c);
    assert(poll_api);

    if (!(d = avahi_new(ConnectionData, 1)))
        goto fail;

    d->poll_api = poll_api;
    d->connection = c;
    d->ref = 1;

    if (!(d->dispatch_timeout = poll_api->timeout_new(poll_api, NULL, dispatch_timeout_callback, d)))
        goto fail;

    if (!(dbus_connection_set_watch_functions(c, add_watch, remove_watch, watch_toggled, connection_data_ref(d), (DBusFreeFunction)connection_data_unref)))
        goto fail;

    if (!(dbus_connection_set_timeout_functions(c, add_timeout, remove_timeout, timeout_toggled, connection_data_ref(d), (DBusFreeFunction)connection_data_unref)))
        goto fail;

    dbus_connection_set_dispatch_status_function(c, dispatch_status, connection_data_ref(d), (DBusFreeFunction)connection_data_unref);

    if (dbus_connection_get_dispatch_status(c) == DBUS_DISPATCH_DATA_REMAINS)
        request_dispatch(d, 1);

    connection_data_unref(d);

    return 0;

fail:

    if (d) {
        d->poll_api->timeout_free(d->dispatch_timeout);

        avahi_free(d);
    }

    return -1;
}
