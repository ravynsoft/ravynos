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

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <locale.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/llist.h>
#include <avahi-common/i18n.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include "sigint.h"

typedef enum {
    COMMAND_UNSPEC,
    COMMAND_HELP,
    COMMAND_VERSION,
    COMMAND_RESOLVE_HOST_NAME,
    COMMAND_RESOLVE_ADDRESS
} Command;

typedef struct Config {
    int verbose;
    Command command;
    AvahiProtocol proto;
} Config;

static AvahiSimplePoll *simple_poll = NULL;
static AvahiClient *client = NULL;

static int n_resolving = 0;

static void host_name_resolver_callback(
    AvahiHostNameResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const AvahiAddress *a,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void *userdata) {

    assert(r);

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {
            char address[AVAHI_ADDRESS_STR_MAX];

            avahi_address_snprint(address, sizeof(address), a);

            printf("%s\t%s\n", name, address);

            break;
        }

        case AVAHI_RESOLVER_FAILURE:

            fprintf(stderr, _("Failed to resolve host name '%s': %s\n"), name, avahi_strerror(avahi_client_errno(client)));
            break;
    }


    avahi_host_name_resolver_free(r);

    assert(n_resolving > 0);
    n_resolving--;

    if (n_resolving <= 0)
        avahi_simple_poll_quit(simple_poll);
}

static void address_resolver_callback(
    AvahiAddressResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const AvahiAddress *a,
    const char *name,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    AVAHI_GCC_UNUSED void *userdata) {

    char address[AVAHI_ADDRESS_STR_MAX];
    assert(r);

    avahi_address_snprint(address, sizeof(address), a);

    switch (event) {
        case AVAHI_RESOLVER_FOUND:

            printf("%s\t%s\n", address, name);
            break;

        case AVAHI_RESOLVER_FAILURE:

            fprintf(stderr, _("Failed to resolve address '%s': %s\n"), address, avahi_strerror(avahi_client_errno(client)));
            break;
    }


    avahi_address_resolver_free(r);

    assert(n_resolving > 0);
    n_resolving--;

    if (n_resolving <= 0)
        avahi_simple_poll_quit(simple_poll);
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    switch (state) {
        case AVAHI_CLIENT_FAILURE:
            fprintf(stderr, _("Client failure, exiting: %s\n"), avahi_strerror(avahi_client_errno(c)));
            avahi_simple_poll_quit(simple_poll);
            break;

        case AVAHI_CLIENT_S_REGISTERING:
        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_CONNECTING:
            ;
    }
}

static void help(FILE *f, const char *argv0) {
    fprintf(f,
            _("%s [options] %s <host name ...>\n"
              "%s [options] %s <address ... >\n\n"
              "    -h --help            Show this help\n"
              "    -V --version         Show version\n"
              "    -n --name            Resolve host name\n"
              "    -a --address         Resolve address\n"
              "    -v --verbose         Enable verbose mode\n"
              "    -6                   Lookup IPv6 address\n"
              "    -4                   Lookup IPv4 address\n"),
            argv0, strstr(argv0, "host-name") ? "[-n]" : "-n",
            argv0, strstr(argv0, "address") ? "[-a]" : "-a");
}

static int parse_command_line(Config *c, const char *argv0, int argc, char *argv[]) {
    int o;

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "version",        no_argument,       NULL, 'V' },
        { "name",           no_argument,       NULL, 'n' },
        { "address",        no_argument,       NULL, 'a' },
        { "verbose",        no_argument,       NULL, 'v' },
        { NULL, 0, NULL, 0 }
    };

    assert(c);

    c->command = strstr(argv0, "address") ? COMMAND_RESOLVE_ADDRESS : (strstr(argv0, "host-name") ? COMMAND_RESOLVE_HOST_NAME : COMMAND_UNSPEC);
    c->proto = AVAHI_PROTO_UNSPEC;
    c->verbose = 0;

    while ((o = getopt_long(argc, argv, "hVnav46", long_options, NULL)) >= 0) {

        switch(o) {
            case 'h':
                c->command = COMMAND_HELP;
                break;
            case 'V':
                c->command = COMMAND_VERSION;
                break;
            case 'n':
                c->command = COMMAND_RESOLVE_HOST_NAME;
                break;
            case 'a':
                c->command = COMMAND_RESOLVE_ADDRESS;
                break;
            case 'v':
                c->verbose = 1;
                break;
            case '4':
                c->proto = AVAHI_PROTO_INET;
                break;
            case '6':
                c->proto = AVAHI_PROTO_INET6;
                break;
            default:
                return -1;
        }
    }

    if (c->command == COMMAND_RESOLVE_ADDRESS || c->command == COMMAND_RESOLVE_HOST_NAME) {
        if (optind >= argc) {
            fprintf(stderr, _("Too few arguments\n"));
            return -1;
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 1, error;
    Config config;
    const char *argv0;

    avahi_init_i18n();
    setlocale(LC_ALL, "");

    if ((argv0 = strrchr(argv[0], '/')))
        argv0++;
    else
        argv0 = argv[0];

    if (parse_command_line(&config, argv0, argc, argv) < 0)
        goto fail;

    switch (config.command) {
        case COMMAND_UNSPEC:
            ret = 1;
            fprintf(stderr, _("No command specified.\n"));
            break;

        case COMMAND_HELP:
            help(stdout, argv0);
            ret = 0;
            break;

        case COMMAND_VERSION:
            printf("%s "PACKAGE_VERSION"\n", argv0);
            ret = 0;
            break;

        case COMMAND_RESOLVE_HOST_NAME:
        case COMMAND_RESOLVE_ADDRESS: {
            int i;

            if (!(simple_poll = avahi_simple_poll_new())) {
                fprintf(stderr, _("Failed to create simple poll object.\n"));
                goto fail;
            }

            if (sigint_install(simple_poll) < 0)
                goto fail;

            if (!(client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, client_callback, NULL, &error))) {
                fprintf(stderr, _("Failed to create client object: %s\n"), avahi_strerror(error));
                goto fail;
            }

            if (config.verbose) {
                const char *version, *hn;

                if (!(version = avahi_client_get_version_string(client))) {
                    fprintf(stderr, _("Failed to query version string: %s\n"), avahi_strerror(avahi_client_errno(client)));
                    goto fail;
                }

                if (!(hn = avahi_client_get_host_name_fqdn(client))) {
                    fprintf(stderr, _("Failed to query host name: %s\n"), avahi_strerror(avahi_client_errno(client)));
                    goto fail;
                }

                fprintf(stderr, _("Server version: %s; Host name: %s\n"), version, hn);
            }

            n_resolving = 0;

            for (i = optind; i < argc; i++) {

                if (config.command == COMMAND_RESOLVE_HOST_NAME) {

                    if (!(avahi_host_name_resolver_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, argv[i], config.proto, 0, host_name_resolver_callback, NULL))) {
                        fprintf(stderr, _("Failed to create host name resolver: %s\n"), avahi_strerror(avahi_client_errno(client)));
                        goto fail;
                    }

                } else {
                    AvahiAddress a;

                    assert(config.command == COMMAND_RESOLVE_ADDRESS);

                    if (!avahi_address_parse(argv[i], AVAHI_PROTO_UNSPEC, &a)) {
                        fprintf(stderr, _("Failed to parse address '%s'\n"), argv[i]);
                        goto fail;
                    }

                    if (!(avahi_address_resolver_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, &a, 0, address_resolver_callback, NULL))) {
                        fprintf(stderr, _("Failed to create address resolver: %s\n"), avahi_strerror(avahi_client_errno(client)));
                        goto fail;
                    }
                }

                n_resolving++;
            }

            avahi_simple_poll_loop(simple_poll);
            ret = 0;
            break;
        }
    }

fail:

    if (client)
        avahi_client_free(client);

    sigint_uninstall();

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    return ret;
}
