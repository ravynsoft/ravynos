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

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-core/log.h>
#include <avahi-core/publish.h>

#include "main.h"
#include "static-hosts.h"

typedef struct StaticHost StaticHost;

struct StaticHost {
    AvahiSEntryGroup *group;
    int iteration;

    char *host;
    AvahiAddress address;

    AVAHI_LLIST_FIELDS(StaticHost, hosts);
};

static AVAHI_LLIST_HEAD(StaticHost, hosts) = NULL;
static int current_iteration = 0;

static void add_static_host_to_server(StaticHost *h);
static void remove_static_host_from_server(StaticHost *h);

static void entry_group_callback(AvahiServer *s, AVAHI_GCC_UNUSED AvahiSEntryGroup *eg, AvahiEntryGroupState state, void* userdata) {
    StaticHost *h;

    assert(s);
    assert(eg);

    h = userdata;

    switch (state) {

        case AVAHI_ENTRY_GROUP_COLLISION:
            avahi_log_error("Host name conflict for \"%s\", not established.", h->host);
            break;

        case AVAHI_ENTRY_GROUP_ESTABLISHED:
            avahi_log_notice ("Static host name \"%s\" successfully established.", h->host);
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:
            avahi_log_notice ("Failed to establish static host name \"%s\": %s.", h->host, avahi_strerror (avahi_server_errno (s)));
            break;

        case AVAHI_ENTRY_GROUP_UNCOMMITED:
        case AVAHI_ENTRY_GROUP_REGISTERING:
            ;
    }
}

static StaticHost *static_host_new(void) {
    StaticHost *s;

    s = avahi_new(StaticHost, 1);

    s->group = NULL;
    s->host = NULL;
    s->iteration = current_iteration;

    AVAHI_LLIST_PREPEND(StaticHost, hosts, hosts, s);

    return s;
}

static void static_host_free(StaticHost *s) {
    assert(s);

    AVAHI_LLIST_REMOVE(StaticHost, hosts, hosts, s);

    if (s->group)
        avahi_s_entry_group_free (s->group);

    avahi_free(s->host);

    avahi_free(s);
}

static StaticHost *static_host_find(const char *host, const AvahiAddress *a) {
    StaticHost *h;

    assert(host);
    assert(a);

    for (h = hosts; h; h = h->hosts_next)
        if (!strcmp(h->host, host) && !avahi_address_cmp(a, &h->address))
            return h;

    return NULL;
}

static void add_static_host_to_server(StaticHost *h)
{

    if (!h->group)
        if (!(h->group = avahi_s_entry_group_new (avahi_server, entry_group_callback, h))) {
            avahi_log_error("avahi_s_entry_group_new() failed: %s", avahi_strerror(avahi_server_errno(avahi_server)));
            return;
        }

    if (avahi_s_entry_group_is_empty(h->group)) {
        AvahiProtocol p;
        int err;
        const AvahiServerConfig *config;
        config = avahi_server_get_config(avahi_server);

        p = (h->address.proto == AVAHI_PROTO_INET && config->publish_a_on_ipv6) ||
            (h->address.proto == AVAHI_PROTO_INET6 && config->publish_aaaa_on_ipv4) ? AVAHI_PROTO_UNSPEC : h->address.proto;

        if ((err = avahi_server_add_address(avahi_server, h->group, AVAHI_IF_UNSPEC, p, 0, h->host, &h->address)) < 0) {
            avahi_log_error ("Static host name %s: avahi_server_add_address failure: %s", h->host, avahi_strerror(err));
            return;
        }

        avahi_s_entry_group_commit (h->group);
    }
}

static void remove_static_host_from_server(StaticHost *h)
{
    if (h->group)
        avahi_s_entry_group_reset (h->group);
}

void static_hosts_add_to_server(void) {
    StaticHost *h;

    for (h = hosts; h; h = h->hosts_next)
        add_static_host_to_server(h);
}

void static_hosts_remove_from_server(void) {
    StaticHost *h;

    for (h = hosts; h; h = h->hosts_next)
        remove_static_host_from_server(h);
}

void static_hosts_load(int in_chroot) {
    FILE *f;
    unsigned int line = 0;
    StaticHost *h, *next;
    const char *filename = in_chroot ? "/hosts" : AVAHI_CONFIG_DIR "/hosts";

    if (!(f = fopen(filename, "r"))) {
        if (errno != ENOENT)
            avahi_log_error ("Failed to open static hosts file: %s", strerror (errno));
        return;
    }

    current_iteration++;

    while (!feof(f)) {
        unsigned int len;
        char ln[256], *s;
        char *host, *ip;
        AvahiAddress a;

        if (!fgets(ln, sizeof (ln), f))
            break;

        line++;

        /* Find the start of the line, ignore whitespace */
        s = ln + strspn(ln, " \t");
        /* Set the end of the string to NULL */
        s[strcspn(s, "#\r\n")] = 0;

        /* Ignore blank lines */
        if (*s == 0)
            continue;

        /* Read the first string (ip) up to the next whitespace */
        len = strcspn(s, " \t");
        ip = avahi_strndup(s, len);

        /* Skip past it */
        s += len;

        /* Find the next token */
        s += strspn(s, " \t");
        len = strcspn(s, " \t");
        host = avahi_strndup(s, len);

        if (*host == 0)
        {
            avahi_log_error("%s:%d: Error, unexpected end of line!", filename, line);
            avahi_free(host);
            avahi_free(ip);
            goto fail;
        }

        /* Skip over the host */
        s += len;

        /* Skip past any more spaces */
        s += strspn(s, " \t");

        /* Anything left? */
        if (*s != 0) {
            avahi_log_error ("%s:%d: Junk on the end of the line!", filename, line);
            avahi_free(host);
            avahi_free(ip);
            goto fail;
        }

        if (!avahi_address_parse(ip, AVAHI_PROTO_UNSPEC, &a)) {
            avahi_log_error("Static host name %s: failed to parse address %s", host, ip);
            avahi_free(host);
            avahi_free(ip);
            goto fail;
        }

        avahi_free(ip);

        if ((h = static_host_find(host, &a)))
            avahi_free(host);
        else {
            h = static_host_new();
            h->host = host;
            h->address = a;

            avahi_log_info("Loading new static hostname %s.", h->host);
        }

        h->iteration = current_iteration;
    }

    for (h = hosts; h; h = next) {
        next = h->hosts_next;

        if (h->iteration != current_iteration) {
            avahi_log_info("Static hostname %s vanished, removing.", h->host);
            static_host_free(h);
        }
    }

fail:

    fclose(f);
}

void static_hosts_free_all (void)
{
    while(hosts)
        static_host_free(hosts);
}
