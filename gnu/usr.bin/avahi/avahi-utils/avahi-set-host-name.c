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
#include <locale.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/i18n.h>
#include <avahi-client/client.h>

#include "sigint.h"

typedef enum {
    COMMAND_UNSPEC,
    COMMAND_HELP,
    COMMAND_VERSION
} Command;

typedef struct Config {
    int verbose;
    Command command;
} Config;

static AvahiSimplePoll *simple_poll = NULL;
static AvahiClient *client = NULL;

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
            _("%s [options] <new host name>\n\n"
              "    -h --help            Show this help\n"
              "    -V --version         Show version\n"
              "    -v --verbose         Enable verbose mode\n"),
            argv0);
}

static int parse_command_line(Config *c, int argc, char *argv[]) {
    int o;

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "version",        no_argument,       NULL, 'V' },
        { "verbose",        no_argument,       NULL, 'v' },
        { NULL, 0, NULL, 0 }
    };

    assert(c);

    c->command = COMMAND_UNSPEC;
    c->verbose = 0;

    while ((o = getopt_long(argc, argv, "hVv", long_options, NULL)) >= 0) {

        switch(o) {
            case 'h':
                c->command = COMMAND_HELP;
                break;
            case 'V':
                c->command = COMMAND_VERSION;
                break;
            case 'v':
                c->verbose = 1;
                break;
            default:
                return -1;
        }
    }

    if (c->command == COMMAND_UNSPEC) {
        if (optind != argc-1) {
            fprintf(stderr, _("Invalid number of arguments, expecting exactly one.\n"));
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

    if (parse_command_line(&config, argc, argv) < 0)
        goto fail;

    switch (config.command) {
        case COMMAND_HELP:
            help(stdout, argv0);
            ret = 0;
            break;

        case COMMAND_VERSION:
            printf("%s "PACKAGE_VERSION"\n", argv0);
            ret = 0;
            break;

        case COMMAND_UNSPEC:

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

            if (avahi_client_set_host_name(client, argv[optind]) < 0) {
                fprintf(stderr, _("Failed to create host name resolver: %s\n"), avahi_strerror(avahi_client_errno(client)));
                goto fail;
            }

            if (config.verbose) {
                const char *hn;

                if (!(hn = avahi_client_get_host_name_fqdn(client))) {
                    fprintf(stderr, _("Failed to query host name: %s\n"), avahi_strerror(avahi_client_errno(client)));
                    goto fail;
                }

                fprintf(stderr, _("Host name successfully changed to %s\n"), hn);
            }

            ret = 0;
            break;
    }

fail:

    if (client)
        avahi_client_free(client);

    sigint_uninstall();

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    return ret;
}
