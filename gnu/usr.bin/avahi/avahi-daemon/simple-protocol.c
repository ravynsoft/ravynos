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
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include <avahi-core/log.h>
#include <avahi-core/lookup.h>
#include <avahi-core/dns-srv-rr.h>

#include "simple-protocol.h"
#include "main.h"
#include "sd-daemon.h"

#ifdef ENABLE_CHROOT
#include "chroot.h"
#endif

#ifndef AF_LOCAL
#define AF_LOCAL AF_UNIX
#endif
#ifndef PF_LOCAL
#define PF_LOCAL PF_UNIX
#endif

#define BUFFER_SIZE (20*1024)

#define CLIENTS_MAX 50

typedef struct Client Client;
typedef struct Server Server;

typedef enum {
    CLIENT_IDLE,
    CLIENT_RESOLVE_HOSTNAME,
    CLIENT_RESOLVE_ADDRESS,
    CLIENT_BROWSE_DNS_SERVERS,
    CLIENT_DEAD
} ClientState;

struct Client {
    Server *server;

    ClientState state;

    int fd;
    AvahiWatch *watch;

    char inbuf[BUFFER_SIZE], outbuf[BUFFER_SIZE];
    size_t inbuf_length, outbuf_length;

    AvahiSHostNameResolver *host_name_resolver;
    AvahiSAddressResolver *address_resolver;
    AvahiSDNSServerBrowser *dns_server_browser;

    AvahiProtocol afquery;

    AVAHI_LLIST_FIELDS(Client, clients);
};

struct Server {
    const AvahiPoll *poll_api;
    int fd;
    AvahiWatch *watch;
    AVAHI_LLIST_HEAD(Client, clients);

    unsigned n_clients;
    int remove_socket;
};

static Server *server = NULL;

static void client_work(AvahiWatch *watch, int fd, AvahiWatchEvent events, void *userdata);

static void client_free(Client *c) {
    assert(c);

    assert(c->server->n_clients >= 1);
    c->server->n_clients--;

    if (c->host_name_resolver)
        avahi_s_host_name_resolver_free(c->host_name_resolver);

    if (c->address_resolver)
        avahi_s_address_resolver_free(c->address_resolver);

    if (c->dns_server_browser)
        avahi_s_dns_server_browser_free(c->dns_server_browser);

    c->server->poll_api->watch_free(c->watch);
    close(c->fd);

    AVAHI_LLIST_REMOVE(Client, clients, c->server->clients, c);
    avahi_free(c);
}

static void client_new(Server *s, int fd) {
    Client *c;

    assert(fd >= 0);

    c = avahi_new(Client, 1);
    c->server = s;
    c->fd = fd;
    c->state = CLIENT_IDLE;

    c->inbuf_length = c->outbuf_length = 0;

    c->host_name_resolver = NULL;
    c->address_resolver = NULL;
    c->dns_server_browser = NULL;

    c->watch = s->poll_api->watch_new(s->poll_api, fd, AVAHI_WATCH_IN, client_work, c);

    AVAHI_LLIST_PREPEND(Client, clients, s->clients, c);
    s->n_clients++;
}

static void client_output(Client *c, const uint8_t*data, size_t size) {
    size_t k, m;

    assert(c);
    assert(data);

    if (!size)
        return;

    k = sizeof(c->outbuf) - c->outbuf_length;
    m = size > k ? k : size;

    memcpy(c->outbuf + c->outbuf_length, data, m);
    c->outbuf_length += m;

    server->poll_api->watch_update(c->watch, AVAHI_WATCH_OUT);
}

static void client_output_printf(Client *c, const char *format, ...) {
    char *t;
    va_list ap;

    va_start(ap, format);
    t = avahi_strdup_vprintf(format, ap);
    va_end(ap);

    client_output(c, (uint8_t*) t, strlen(t));
    avahi_free(t);
}

static void host_name_resolver_callback(
    AVAHI_GCC_UNUSED AvahiSHostNameResolver *r,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *hostname,
    const AvahiAddress *a,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    Client *c = userdata;

    assert(c);

    if (event == AVAHI_RESOLVER_FAILURE)
        client_output_printf(c, "%+i %s\n", avahi_server_errno(avahi_server), avahi_strerror(avahi_server_errno(avahi_server)));
    else if (event == AVAHI_RESOLVER_FOUND) {
        char t[AVAHI_ADDRESS_STR_MAX];
        avahi_address_snprint(t, sizeof(t), a);
        client_output_printf(c, "+ %i %u %s %s\n", iface, protocol, hostname, t);
    }

    c->state = CLIENT_DEAD;
}

static void address_resolver_callback(
    AVAHI_GCC_UNUSED AvahiSAddressResolver *r,
    AvahiIfIndex iface,
    AvahiProtocol protocol,
    AvahiResolverEvent event,
    AVAHI_GCC_UNUSED const AvahiAddress *a,
    const char *hostname,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    Client *c = userdata;

    assert(c);

    if (event == AVAHI_RESOLVER_FAILURE)
        client_output_printf(c, "%+i %s\n", avahi_server_errno(avahi_server), avahi_strerror(avahi_server_errno(avahi_server)));
    else if (event == AVAHI_RESOLVER_FOUND)
        client_output_printf(c, "+ %i %u %s\n", iface, protocol, hostname);

    c->state = CLIENT_DEAD;
}

static void dns_server_browser_callback(
    AVAHI_GCC_UNUSED AvahiSDNSServerBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    AVAHI_GCC_UNUSED const char *host_name,
    const AvahiAddress *a,
    uint16_t port,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {

    Client *c = userdata;
    char t[AVAHI_ADDRESS_STR_MAX];

    assert(c);

    if (!a)
        return;

    switch (event) {
        case AVAHI_BROWSER_FAILURE:
            client_output_printf(c, "%+i %s\n", avahi_server_errno(avahi_server), avahi_strerror(avahi_server_errno(avahi_server)));
            c->state = CLIENT_DEAD;
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            break;

        case AVAHI_BROWSER_NEW:
        case AVAHI_BROWSER_REMOVE:

            avahi_address_snprint(t, sizeof(t), a);
            client_output_printf(c, "%c %i %u %s %u\n", event == AVAHI_BROWSER_NEW ? '>' : '<',  interface, protocol, t, port);
            break;
    }
}

static void handle_line(Client *c, const char *s) {
    char cmd[64], arg[64];
    int n_args;

    assert(c);
    assert(s);

    if (c->state != CLIENT_IDLE)
        return;

    if ((n_args = sscanf(s, "%63s %63s", cmd, arg)) < 1 ) {
        client_output_printf(c, "%+i Failed to parse command, try \"HELP\".\n", AVAHI_ERR_INVALID_OPERATION);
        c->state = CLIENT_DEAD;
        return;
    }

    if (strcmp(cmd, "HELP") == 0) {
        client_output_printf(c,
                             "+ Available commands are:\n"
                             "+      RESOLVE-HOSTNAME <hostname>\n"
                             "+      RESOLVE-HOSTNAME-IPV6 <hostname>\n"
                             "+      RESOLVE-HOSTNAME-IPV4 <hostname>\n"
                             "+      RESOLVE-ADDRESS <address>\n"
                             "+      BROWSE-DNS-SERVERS\n"
                             "+      BROWSE-DNS-SERVERS-IPV4\n"
                             "+      BROWSE-DNS-SERVERS-IPV6\n");
        c->state = CLIENT_DEAD; }
    else if (strcmp(cmd, "FUCK") == 0 && n_args == 1) {
        client_output_printf(c, "+ FUCK: Go fuck yourself!\n");
        c->state = CLIENT_DEAD;
    } else if (strcmp(cmd, "RESOLVE-HOSTNAME-IPV4") == 0 && n_args == 2) {
        c->state = CLIENT_RESOLVE_HOSTNAME;
        if (!(c->host_name_resolver = avahi_s_host_name_resolver_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, arg, c->afquery = AVAHI_PROTO_INET, AVAHI_LOOKUP_USE_MULTICAST, host_name_resolver_callback, c)))
            goto fail;

        avahi_log_debug(__FILE__": Got %s request for '%s'.", cmd, arg);
    } else if (strcmp(cmd, "RESOLVE-HOSTNAME-IPV6") == 0 && n_args == 2) {
        c->state = CLIENT_RESOLVE_HOSTNAME;
        if (!(c->host_name_resolver = avahi_s_host_name_resolver_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, arg, c->afquery = AVAHI_PROTO_INET6, AVAHI_LOOKUP_USE_MULTICAST, host_name_resolver_callback, c)))
            goto fail;

        avahi_log_debug(__FILE__": Got %s request for '%s'.", cmd, arg);
    } else if (strcmp(cmd, "RESOLVE-HOSTNAME") == 0 && n_args == 2) {
        c->state = CLIENT_RESOLVE_HOSTNAME;
        if (!(c->host_name_resolver = avahi_s_host_name_resolver_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, arg, c->afquery = AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_USE_MULTICAST, host_name_resolver_callback, c)))
            goto fail;

        avahi_log_debug(__FILE__": Got %s request for '%s'.", cmd, arg);
    } else if (strcmp(cmd, "RESOLVE-ADDRESS") == 0 && n_args == 2) {
        AvahiAddress addr;

        if (!(avahi_address_parse(arg, AVAHI_PROTO_UNSPEC, &addr))) {
            client_output_printf(c, "%+i Failed to parse address \"%s\".\n", AVAHI_ERR_INVALID_ADDRESS, arg);
            c->state = CLIENT_DEAD;
        } else {
            c->state = CLIENT_RESOLVE_ADDRESS;
            if (!(c->address_resolver = avahi_s_address_resolver_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, &addr, AVAHI_LOOKUP_USE_MULTICAST, address_resolver_callback, c)))
                goto fail;
        }

        avahi_log_debug(__FILE__": Got %s request for '%s'.", cmd, arg);

    } else if (strcmp(cmd, "BROWSE-DNS-SERVERS-IPV4") == 0 && n_args == 1) {
        c->state = CLIENT_BROWSE_DNS_SERVERS;
        if (!(c->dns_server_browser = avahi_s_dns_server_browser_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DNS_SERVER_RESOLVE, c->afquery = AVAHI_PROTO_INET, AVAHI_LOOKUP_USE_MULTICAST, dns_server_browser_callback, c)))
            goto fail;
        client_output_printf(c, "+ Browsing ...\n");

        avahi_log_debug(__FILE__": Got %s request.", cmd);

    } else if (strcmp(cmd, "BROWSE-DNS-SERVERS-IPV6") == 0 && n_args == 1) {
        c->state = CLIENT_BROWSE_DNS_SERVERS;
        if (!(c->dns_server_browser = avahi_s_dns_server_browser_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DNS_SERVER_RESOLVE, c->afquery = AVAHI_PROTO_INET6, AVAHI_LOOKUP_USE_MULTICAST, dns_server_browser_callback, c)))
            goto fail;
        client_output_printf(c, "+ Browsing ...\n");

        avahi_log_debug(__FILE__": Got %s request.", cmd);

    } else if (strcmp(cmd, "BROWSE-DNS-SERVERS") == 0 && n_args == 1) {
        c->state = CLIENT_BROWSE_DNS_SERVERS;
        if (!(c->dns_server_browser = avahi_s_dns_server_browser_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DNS_SERVER_RESOLVE, c->afquery = AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_USE_MULTICAST, dns_server_browser_callback, c)))
            goto fail;
        client_output_printf(c, "+ Browsing ...\n");

        avahi_log_debug(__FILE__": Got %s request.", cmd);

    } else {
        client_output_printf(c, "%+i Invalid command \"%s\", try \"HELP\".\n", AVAHI_ERR_INVALID_OPERATION, cmd);
        c->state = CLIENT_DEAD;

        avahi_log_debug(__FILE__": Got invalid request '%s'.", cmd);
    }

    return;

fail:
    client_output_printf(c, "%+i %s\n", avahi_server_errno(avahi_server), avahi_strerror(avahi_server_errno(avahi_server)));
    c->state = CLIENT_DEAD;
}

static void handle_input(Client *c) {
    assert(c);

    for (;;) {
        char *e;
        size_t k;

        if (!(e = memchr(c->inbuf, '\n', c->inbuf_length)))
            break;

        k = e - (char*) c->inbuf;
        *e = 0;

        handle_line(c, c->inbuf);
        c->inbuf_length -= k + 1;
        memmove(c->inbuf, e+1, c->inbuf_length);
    }
}

static void client_work(AvahiWatch *watch, AVAHI_GCC_UNUSED int fd, AvahiWatchEvent events, void *userdata) {
    Client *c = userdata;

    assert(c);

    if ((events & AVAHI_WATCH_IN) && c->inbuf_length < sizeof(c->inbuf)) {
        ssize_t r;

        if ((r = read(c->fd, c->inbuf + c->inbuf_length, sizeof(c->inbuf) - c->inbuf_length)) <= 0) {
            if (r < 0)
                avahi_log_warn("read(): %s", strerror(errno));
            client_free(c);
            return;
        }

        c->inbuf_length += r;
        assert(c->inbuf_length <= sizeof(c->inbuf));

        handle_input(c);
    }

    if ((events & AVAHI_WATCH_OUT) && c->outbuf_length > 0) {
        ssize_t r;

        if ((r = write(c->fd, c->outbuf, c->outbuf_length)) < 0) {
            avahi_log_warn("write(): %s", strerror(errno));
            client_free(c);
            return;
        }

        assert((size_t) r <= c->outbuf_length);
        c->outbuf_length -= r;

        if (c->outbuf_length)
            memmove(c->outbuf, c->outbuf + r, c->outbuf_length - r);

        if (c->outbuf_length == 0 && c->state == CLIENT_DEAD) {
            client_free(c);
            return;
        }
    }

    c->server->poll_api->watch_update(
        watch,
        (c->outbuf_length > 0 ? AVAHI_WATCH_OUT : 0) |
        (c->inbuf_length < sizeof(c->inbuf) ? AVAHI_WATCH_IN : 0));
}

static void server_work(AVAHI_GCC_UNUSED AvahiWatch *watch, int fd, AvahiWatchEvent events, void *userdata) {
    Server *s = userdata;

    assert(s);

    if (events & AVAHI_WATCH_IN) {
        int cfd;

        if ((cfd = accept(fd, NULL, NULL)) < 0)
            avahi_log_error("accept(): %s", strerror(errno));
        else
            client_new(s, cfd);
    }
}

int simple_protocol_setup(const AvahiPoll *poll_api) {
    struct sockaddr_un sa;
    mode_t u;
    int n;

    assert(!server);

    server = avahi_new(Server, 1);
    server->poll_api = poll_api;
    server->remove_socket = 0;
    server->fd = -1;
    server->n_clients = 0;
    AVAHI_LLIST_HEAD_INIT(Client, server->clients);
    server->watch = NULL;

    u = umask(0000);

    if ((n = sd_listen_fds(1)) < 0) {
        avahi_log_warn("Failed to acquire systemd file descriptors: %s", strerror(-n));
        goto fail;
    }

    if (n > 1) {
        avahi_log_warn("Too many systemd file descriptors passed.");
        goto fail;
    }

    if (n == 1) {
        int r;

        if ((r = sd_is_socket(SD_LISTEN_FDS_START, AF_LOCAL, SOCK_STREAM, 1)) < 0) {
            avahi_log_warn("Passed systemd file descriptor is of wrong type: %s", strerror(-r));
            goto fail;
        }

        server->fd = SD_LISTEN_FDS_START;

    } else {

        if ((server->fd = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0) {
            avahi_log_warn("socket(AF_LOCAL, SOCK_STREAM, 0): %s", strerror(errno));
            goto fail;
        }

        memset(&sa, 0, sizeof(sa));
        sa.sun_family = AF_LOCAL;
        strncpy(sa.sun_path, AVAHI_SOCKET, sizeof(sa.sun_path)-1);

        /* We simply remove existing UNIX sockets under this name. The
           Avahi daemon makes sure that it runs only once on a host,
           therefore sockets that already exist are stale and may be
           removed without any ill effects */

        unlink(AVAHI_SOCKET);

        if (bind(server->fd, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
            avahi_log_warn("bind(): %s", strerror(errno));
            goto fail;
        }

        server->remove_socket = 1;

        if (listen(server->fd, SOMAXCONN) < 0) {
            avahi_log_warn("listen(): %s", strerror(errno));
            goto fail;
        }
    }

    umask(u);

    server->watch = poll_api->watch_new(poll_api, server->fd, AVAHI_WATCH_IN, server_work, server);

    return 0;

fail:

    umask(u);
    simple_protocol_shutdown();

    return -1;
}

void simple_protocol_shutdown(void) {

    if (server) {

        if (server->remove_socket)
#ifdef ENABLE_CHROOT
            avahi_chroot_helper_unlink(AVAHI_SOCKET);
#else
            unlink(AVAHI_SOCKET);
#endif

        while (server->clients)
            client_free(server->clients);

        if (server->watch)
            server->poll_api->watch_free(server->watch);

        if (server->fd >= 0)
            close(server->fd);

        avahi_free(server);

        server = NULL;
    }
}

void simple_protocol_restart_queries(void) {
    Client *c;

    /* Restart queries in case of local domain name changes */

    assert(server);

    for (c = server->clients; c; c = c->clients_next)
        if (c->state == CLIENT_BROWSE_DNS_SERVERS && c->dns_server_browser) {
            avahi_s_dns_server_browser_free(c->dns_server_browser);
            c->dns_server_browser = avahi_s_dns_server_browser_new(avahi_server, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, NULL, AVAHI_DNS_SERVER_RESOLVE, c->afquery, AVAHI_LOOKUP_USE_MULTICAST, dns_server_browser_callback, c);
        }
}
