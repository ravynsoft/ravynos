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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <avahi-core/core.h>
#include <avahi-core/publish.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/alternative.h>
#include <avahi-common/error.h>

static AvahiSEntryGroup *group = NULL;
static AvahiSimplePoll *simple_poll = NULL;
static char *name = NULL;

static void create_services(AvahiServer *s);

static void entry_group_callback(AvahiServer *s, AvahiSEntryGroup *g, AvahiEntryGroupState state, AVAHI_GCC_UNUSED void *userdata) {
    assert(s);
    assert(g == group);

    /* Called whenever the entry group state changes */

    switch (state) {

        case AVAHI_ENTRY_GROUP_ESTABLISHED:

            /* The entry group has been established successfully */
            fprintf(stderr, "Service '%s' successfully established.\n", name);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION: {
            char *n;

            /* A service name collision happened. Let's pick a new name */
            n = avahi_alternative_service_name(name);
            avahi_free(name);
            name = n;

            fprintf(stderr, "Service name collision, renaming service to '%s'\n", name);

            /* And recreate the services */
            create_services(s);
            break;
        }

        case AVAHI_ENTRY_GROUP_FAILURE :

            fprintf(stderr, "Entry group failure: %s\n", avahi_strerror(avahi_server_errno(s)));

            /* Some kind of failure happened while we were registering our services */
            avahi_simple_poll_quit(simple_poll);
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static void create_services(AvahiServer *s) {
    char r[128];
    int ret;
    assert(s);

    /* If this is the first time we're called, let's create a new entry group */
    if (!group)
        if (!(group = avahi_s_entry_group_new(s, entry_group_callback, NULL))) {
            fprintf(stderr, "avahi_entry_group_new() failed: %s\n", avahi_strerror(avahi_server_errno(s)));
            goto fail;
        }

    fprintf(stderr, "Adding service '%s'\n", name);

    /* Create some random TXT data */
    snprintf(r, sizeof(r), "random=%i", rand());

    /* Add the service for IPP */
    if ((ret = avahi_server_add_service(s, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_ipp._tcp", NULL, NULL, 651, "test=blah", r, NULL)) < 0) {
        fprintf(stderr, "Failed to add _ipp._tcp service: %s\n", avahi_strerror(ret));
        goto fail;
    }

    /* Add the same service for BSD LPR */
    if ((ret = avahi_server_add_service(s, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_printer._tcp", NULL, NULL, 515, NULL)) < 0) {
        fprintf(stderr, "Failed to add _printer._tcp service: %s\n", avahi_strerror(ret));
        goto fail;
    }

    /* Add an additional (hypothetic) subtype */
    if ((ret = avahi_server_add_service_subtype(s, group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, name, "_printer._tcp", NULL, "_magic._sub._printer._tcp") < 0)) {
        fprintf(stderr, "Failed to add subtype _magic._sub._printer._tcp: %s\n", avahi_strerror(ret));
        goto fail;
    }

    /* Tell the server to register the service */
    if ((ret = avahi_s_entry_group_commit(group)) < 0) {
        fprintf(stderr, "Failed to commit entry_group: %s\n", avahi_strerror(ret));
        goto fail;
    }

    return;

fail:
    avahi_simple_poll_quit(simple_poll);
}

static void server_callback(AvahiServer *s, AvahiServerState state, AVAHI_GCC_UNUSED void * userdata) {
    assert(s);

    /* Called whenever the server state changes */

    switch (state) {

        case AVAHI_SERVER_RUNNING:
            /* The serve has startup successfully and registered its host
             * name on the network, so it's time to create our services */

            if (!group)
                create_services(s);

            break;

        case AVAHI_SERVER_COLLISION: {
            char *n;
            int r;

            /* A host name collision happened. Let's pick a new name for the server */
            n = avahi_alternative_host_name(avahi_server_get_host_name(s));
            fprintf(stderr, "Host name collision, retrying with '%s'\n", n);
            r = avahi_server_set_host_name(s, n);
            avahi_free(n);

            if (r < 0) {
                fprintf(stderr, "Failed to set new host name: %s\n", avahi_strerror(r));

                avahi_simple_poll_quit(simple_poll);
                return;
            }

        }

            /* Fall through */

        case AVAHI_SERVER_REGISTERING:

	    /* Let's drop our registered services. When the server is back
             * in AVAHI_SERVER_RUNNING state we will register them
             * again with the new host name. */
            if (group)
                avahi_s_entry_group_reset(group);

            break;

        case AVAHI_SERVER_FAILURE:

            /* Terminate on failure */

            fprintf(stderr, "Server failure: %s\n", avahi_strerror(avahi_server_errno(s)));
            avahi_simple_poll_quit(simple_poll);
            break;

        case AVAHI_SERVER_INVALID:
            ;
    }
}

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char*argv[]) {
    AvahiServerConfig config;
    AvahiServer *server = NULL;
    int error;
    int ret = 1;

    /* Initialize the pseudo-RNG */
    srand(time(NULL));

    /* Allocate main loop object */
    if (!(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        goto fail;
    }

    name = avahi_strdup("MegaPrinter");

    /* Let's set the host name for this server. */
    avahi_server_config_init(&config);
    config.host_name = avahi_strdup("gurkiman");
    config.publish_workstation = 0;

    /* Allocate a new server */
    server = avahi_server_new(avahi_simple_poll_get(simple_poll), &config, server_callback, NULL, &error);

    /* Free the configuration data */
    avahi_server_config_free(&config);

    /* Check wether creating the server object succeeded */
    if (!server) {
        fprintf(stderr, "Failed to create server: %s\n", avahi_strerror(error));
        goto fail;
    }

    /* Run the main loop */
    avahi_simple_poll_loop(simple_poll);

    ret = 0;

fail:

    /* Cleanup things */

    if (server)
        avahi_server_free(server);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    avahi_free(name);

    return ret;
}
