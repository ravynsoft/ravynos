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
#include <errno.h>
#include <locale.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/alternative.h>
#include <avahi-common/i18n.h>
#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include "sigint.h"

typedef enum {
    COMMAND_UNSPEC,
    COMMAND_HELP,
    COMMAND_VERSION,
    COMMAND_PUBLISH_SERVICE,
    COMMAND_PUBLISH_ADDRESS
} Command;

typedef struct Config {
    int verbose, no_fail, no_reverse;
    Command command;
    char *name, *stype, *domain, *host;
    uint16_t port;
    AvahiStringList *txt, *subtypes;
    AvahiAddress address;
} Config;

static AvahiSimplePoll *simple_poll = NULL;
static AvahiClient *client = NULL;
static AvahiEntryGroup *entry_group = NULL;

static int register_stuff(Config *config);

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    Config *config = userdata;

    assert(g);
    assert(config);

    switch (state) {

        case AVAHI_ENTRY_GROUP_ESTABLISHED:

            fprintf(stderr, _("Established under name '%s'\n"), config->name);
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:

            fprintf(stderr, _("Failed to register: %s\n"), avahi_strerror(avahi_client_errno(client)));
            break;

        case AVAHI_ENTRY_GROUP_COLLISION: {
            char *n;

            if (config->command == COMMAND_PUBLISH_SERVICE)
                n = avahi_alternative_service_name(config->name);
            else {
                assert(config->command == COMMAND_PUBLISH_ADDRESS);
                n = avahi_alternative_host_name(config->name);
            }

            fprintf(stderr, _("Name collision, picking new name '%s'.\n"), n);
            avahi_free(config->name);
            config->name = n;

            register_stuff(config);

            break;
        }

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static int register_stuff(Config *config) {
    assert(config);

    if (!entry_group) {
        if (!(entry_group = avahi_entry_group_new(client, entry_group_callback, config))) {
            fprintf(stderr, _("Failed to create entry group: %s\n"), avahi_strerror(avahi_client_errno(client)));
            return -1;
        }
    }

    assert(avahi_entry_group_is_empty(entry_group));

    if (config->command == COMMAND_PUBLISH_ADDRESS) {

        if (avahi_entry_group_add_address(entry_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, config->no_reverse ? AVAHI_PUBLISH_NO_REVERSE : 0, config->name, &config->address) < 0) {
            fprintf(stderr, _("Failed to add address: %s\n"), avahi_strerror(avahi_client_errno(client)));
            return -1;
        }

    } else {
        AvahiStringList *i;

        assert(config->command == COMMAND_PUBLISH_SERVICE);

        if (avahi_entry_group_add_service_strlst(entry_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, config->name, config->stype, config->domain, config->host, config->port, config->txt) < 0) {
            fprintf(stderr, _("Failed to add service: %s\n"), avahi_strerror(avahi_client_errno(client)));
            return -1;
        }

        for (i = config->subtypes; i; i = i->next)
            if (avahi_entry_group_add_service_subtype(entry_group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, config->name, config->stype, config->domain, (char*) i->text) < 0) {
                fprintf(stderr, _("Failed to add subtype '%s': %s\n"), i->text, avahi_strerror(avahi_client_errno(client)));
                return -1;
            }
    }

    avahi_entry_group_commit(entry_group);

    return 0;
}

static void client_callback(AvahiClient *c, AvahiClientState state, AVAHI_GCC_UNUSED void * userdata) {
    Config *config = userdata;

    client = c;

    switch (state) {
        case AVAHI_CLIENT_FAILURE:

            if (config->no_fail && avahi_client_errno(c) == AVAHI_ERR_DISCONNECTED) {
                int error;

                /* We have been disconnected, so let reconnect */

                fprintf(stderr, _("Disconnected, reconnecting ...\n"));

                avahi_client_free(client);
                client = NULL;
                entry_group = NULL;

                if (!(client = avahi_client_new(avahi_simple_poll_get(simple_poll), AVAHI_CLIENT_NO_FAIL, client_callback, config, &error))) {
                    fprintf(stderr, _("Failed to create client object: %s\n"), avahi_strerror(error));
                    avahi_simple_poll_quit(simple_poll);
                }

            } else {
                fprintf(stderr, _("Client failure, exiting: %s\n"), avahi_strerror(avahi_client_errno(c)));
                avahi_simple_poll_quit(simple_poll);
            }

            break;

        case AVAHI_CLIENT_S_RUNNING:

            if (register_stuff(config) < 0)
                avahi_simple_poll_quit(simple_poll);

            break;

        case AVAHI_CLIENT_S_COLLISION:

            if (config->verbose)
                fprintf(stderr, _("Host name conflict\n"));

            /* Fall through */

        case AVAHI_CLIENT_S_REGISTERING:

            if (entry_group) {
                avahi_entry_group_free(entry_group);
                entry_group = NULL;
            }
            break;

        case AVAHI_CLIENT_CONNECTING:

            if (config->verbose)
                fprintf(stderr, _("Waiting for daemon ...\n"));

            break;

            ;
    }
}

static void help(FILE *f, const char *argv0) {
    fprintf(f,
            _("%s [options] %s <name> <type> <port> [<txt ...>]\n"
              "%s [options] %s <host-name> <address>\n\n"
              "    -h --help            Show this help\n"
              "    -V --version         Show version\n"
              "    -s --service         Publish service\n"
              "    -a --address         Publish address\n"
              "    -v --verbose         Enable verbose mode\n"
              "    -d --domain=DOMAIN   Domain to publish service in\n"
              "    -H --host=DOMAIN     Host where service resides\n"
              "       --subtype=SUBTYPE An additional subtype to register this service with\n"
              "    -R --no-reverse      Do not publish reverse entry with address\n"
              "    -f --no-fail         Don't fail if the daemon is not available\n"),
              argv0, strstr(argv0, "service") ? "[-s]" : "-s",
              argv0, strstr(argv0, "address") ? "[-a]" : "-a");
}

static int parse_command_line(Config *c, const char *argv0, int argc, char *argv[]) {
    int o;

    enum {
        ARG_SUBTYPE = 256
    };

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "version",        no_argument,       NULL, 'V' },
        { "service",        no_argument,       NULL, 's' },
        { "address",        no_argument,       NULL, 'a' },
        { "verbose",        no_argument,       NULL, 'v' },
        { "domain",         required_argument, NULL, 'd' },
        { "host",           required_argument, NULL, 'H' },
        { "subtype",        required_argument, NULL, ARG_SUBTYPE},
        { "no-reverse",     no_argument,       NULL, 'R' },
        { "no-fail",        no_argument,       NULL, 'f' },
        { NULL, 0, NULL, 0 }
    };

    assert(c);

    c->command = strstr(argv0, "address") ? COMMAND_PUBLISH_ADDRESS : (strstr(argv0, "service") ? COMMAND_PUBLISH_SERVICE : COMMAND_UNSPEC);
    c->verbose = c->no_fail = c->no_reverse = 0;
    c->host = c->name = c->domain = c->stype = NULL;
    c->port = 0;
    c->txt = c->subtypes = NULL;

    while ((o = getopt_long(argc, argv, "hVsavRd:H:f", long_options, NULL)) >= 0) {

        switch(o) {
            case 'h':
                c->command = COMMAND_HELP;
                break;
            case 'V':
                c->command = COMMAND_VERSION;
                break;
            case 's':
                c->command = COMMAND_PUBLISH_SERVICE;
                break;
            case 'a':
                c->command = COMMAND_PUBLISH_ADDRESS;
                break;
            case 'v':
                c->verbose = 1;
                break;
            case 'R':
                c->no_reverse = 1;
                break;
            case 'd':
                avahi_free(c->domain);
                c->domain = avahi_strdup(optarg);
                break;
            case 'H':
                avahi_free(c->host);
                c->host = avahi_strdup(optarg);
                break;
            case 'f':
                c->no_fail = 1;
                break;
            case ARG_SUBTYPE:
                c->subtypes = avahi_string_list_add(c->subtypes, optarg);
                break;
            default:
                return -1;
        }
    }

    if (c->command == COMMAND_PUBLISH_ADDRESS) {
        if (optind+2 !=  argc) {
            fprintf(stderr, _("Bad number of arguments\n"));
            return -1;
        }

        avahi_free(c->name);
        c->name = avahi_strdup(argv[optind]);
        avahi_address_parse(argv[optind+1], AVAHI_PROTO_UNSPEC, &c->address);

    } else if (c->command == COMMAND_PUBLISH_SERVICE) {

        char *e;
        long int p;
        int i;

        if (optind+3 > argc) {
            fprintf(stderr, _("Bad number of arguments\n"));
            return -1;
        }

        c->name = avahi_strdup(argv[optind]);
        c->stype = avahi_strdup(argv[optind+1]);

        errno = 0;
        p = strtol(argv[optind+2], &e, 0);

        if (errno != 0 || *e || p < 0 || p > 0xFFFF) {
            fprintf(stderr, _("Failed to parse port number: %s\n"), argv[optind+2]);
            return -1;
        }

        c->port = p;

        for (i = optind+3; i < argc; i++)
            c->txt = avahi_string_list_add(c->txt, argv[i]);
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

        case COMMAND_PUBLISH_SERVICE:
        case COMMAND_PUBLISH_ADDRESS:

            if (!(simple_poll = avahi_simple_poll_new())) {
                fprintf(stderr, _("Failed to create simple poll object.\n"));
                goto fail;
            }

            if (sigint_install(simple_poll) < 0)
                goto fail;

            if (!(client = avahi_client_new(avahi_simple_poll_get(simple_poll), config.no_fail ? AVAHI_CLIENT_NO_FAIL : 0, client_callback, &config, &error))) {
                fprintf(stderr, _("Failed to create client object: %s\n"), avahi_strerror(error));
                goto fail;
            }

            if (avahi_client_get_state(client) != AVAHI_CLIENT_CONNECTING && config.verbose) {
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

            avahi_simple_poll_loop(simple_poll);
            ret = 0;
            break;
    }

fail:

    if (client)
        avahi_client_free(client);

    sigint_uninstall();

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    avahi_free(config.host);
    avahi_free(config.name);
    avahi_free(config.stype);
    avahi_free(config.domain);
    avahi_string_list_free(config.subtypes);
    avahi_string_list_free(config.txt);

    return ret;
}
