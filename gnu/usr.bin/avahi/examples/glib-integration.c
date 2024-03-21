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

#include <glib.h>

#include <avahi-client/client.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>

/* Callback for Avahi API Timeout Event */
static void
avahi_timeout_event (AVAHI_GCC_UNUSED AvahiTimeout *timeout, AVAHI_GCC_UNUSED void *userdata)
{
    g_message ("Avahi API Timeout reached!");
}

/* Callback for GLIB API Timeout Event */
static gboolean
avahi_timeout_event_glib (void *userdata)
{
    GMainLoop *loop = userdata;

    g_message ("GLIB API Timeout reached, quitting main loop!");

    /* Quit the application */
    g_main_loop_quit (loop);

    return FALSE; /* Don't re-schedule timeout event */
}

/* Callback for state changes on the Client */
static void
avahi_client_callback (AVAHI_GCC_UNUSED AvahiClient *client, AvahiClientState state, void *userdata)
{
    GMainLoop *loop = userdata;

    g_message ("Avahi Client State Change: %d", state);

    if (state == AVAHI_CLIENT_FAILURE)
    {
        /* We we're disconnected from the Daemon */
        g_message ("Disconnected from the Avahi Daemon: %s", avahi_strerror(avahi_client_errno(client)));

        /* Quit the application */
        g_main_loop_quit (loop);
    }
}

int
main (AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[])
{
    GMainLoop *loop = NULL;
    const AvahiPoll *poll_api;
    AvahiGLibPoll *glib_poll;
    AvahiClient *client;
    struct timeval tv;
    const char *version;
    int error;

    /* Optional: Tell avahi to use g_malloc and g_free */
    avahi_set_allocator (avahi_glib_allocator ());

    /* Create the GLIB main loop */
    loop = g_main_loop_new (NULL, FALSE);

    /* Create the GLIB Adaptor */
    glib_poll = avahi_glib_poll_new (NULL, G_PRIORITY_DEFAULT);
    poll_api = avahi_glib_poll_get (glib_poll);

    /* Example, schedule a timeout event with the Avahi API */
    avahi_elapse_time (&tv,                         /* timeval structure */
            1000,                                   /* 1 second */
            0);                                     /* "jitter" - Random additional delay from 0 to this value */

    poll_api->timeout_new (poll_api,                /* The AvahiPoll object */
                      &tv,                          /* struct timeval indicating when to go activate */
                      avahi_timeout_event,          /* Pointer to function to call */
                      NULL);                        /* User data to pass to function */

    /* Schedule a timeout event with the glib api */
    g_timeout_add (5000,                            /* 5 seconds */
            avahi_timeout_event_glib,               /* Pointer to function callback */
            loop);                                  /* User data to pass to function */

    /* Create a new AvahiClient instance */
    client = avahi_client_new (poll_api,            /* AvahiPoll object from above */
                               0,
            avahi_client_callback,                  /* Callback function for Client state changes */
            loop,                                   /* User data */
            &error);                                /* Error return */

    /* Check the error return code */
    if (client == NULL)
    {
        /* Print out the error string */
        g_warning ("Error initializing Avahi: %s", avahi_strerror (error));

        goto fail;
    }

    /* Make a call to get the version string from the daemon */
    version = avahi_client_get_version_string (client);

    /* Check if the call suceeded */
    if (version == NULL)
    {
        g_warning ("Error getting version string: %s", avahi_strerror (avahi_client_errno (client)));

        goto fail;
    }

    g_message ("Avahi Server Version: %s", version);

    /* Start the GLIB Main Loop */
    g_main_loop_run (loop);

fail:
    /* Clean up */
    g_main_loop_unref (loop);
    avahi_client_free (client);
    avahi_glib_poll_free (glib_poll);

    return 0;
}
