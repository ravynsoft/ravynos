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

#include <pthread.h>

#include <avahi-common/strlst.h>
#include <avahi-common/malloc.h>
#include <avahi-common/domain.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/llist.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

#include "howl.h"
#include "warn.h"

#define OID_MAX 50

enum {
    COMMAND_POLL = 'p',
    COMMAND_QUIT = 'q',
    COMMAND_POLL_DONE = 'P',
    COMMAND_POLL_FAILED = 'F'
};

typedef enum {
    OID_UNUSED = 0,
    OID_SERVICE_BROWSER,
    OID_SERVICE_RESOLVER,
    OID_DOMAIN_BROWSER,
    OID_ENTRY_GROUP
} oid_type;

typedef struct service_data service_data;

typedef struct oid_data {
    oid_type type;
    sw_opaque extra;
    sw_discovery discovery;
    void *object;
    sw_result (*reply)(void);
    service_data *service_data;
} oid_data;


struct service_data {
    char *name, *regtype, *domain, *host;
    uint16_t port;
    AvahiIfIndex interface;
    AvahiStringList *txt;
    AVAHI_LLIST_FIELDS(service_data, services);
};

struct _sw_discovery {
    int n_ref;
    AvahiSimplePoll *simple_poll;
    AvahiClient *client;

    oid_data oid_table[OID_MAX];
    sw_discovery_oid oid_index;

    int thread_fd, main_fd;

    pthread_t thread;
    int thread_running;

    pthread_mutex_t mutex, salt_mutex;

    AVAHI_LLIST_HEAD(service_data, services);
};

#define ASSERT_SUCCESS(r) { int __ret = (r); assert(__ret == 0); }

#define OID_GET_INDEX(data) ((sw_discovery_oid) (((data) - ((data)->discovery->oid_table))))

static sw_discovery discovery_ref(sw_discovery self);
static void discovery_unref(sw_discovery self);

static const char *add_trailing_dot(const char *s, char *buf, size_t buf_len) {
    if (!s)
        return NULL;

    if (*s == 0)
        return s;

    if (s[strlen(s)-1] == '.')
        return s;

    snprintf(buf, buf_len, "%s.", s);
    return buf;
}

static sw_result map_error(int error) {
    switch (error) {
        case AVAHI_OK:
            return SW_OKAY;

        case AVAHI_ERR_NO_MEMORY:
            return SW_E_MEM;
    }

    return SW_E_UNKNOWN;
}

static int read_command(int fd) {
    ssize_t r;
    char command;

    assert(fd >= 0);

    if ((r = read(fd, &command, 1)) != 1) {
        fprintf(stderr, __FILE__": read() failed: %s\n", r < 0 ? strerror(errno) : "EOF");
        return -1;
    }

    return command;
}

static int write_command(int fd, char reply) {
    assert(fd >= 0);

    if (write(fd, &reply, 1) != 1) {
        fprintf(stderr, __FILE__": write() failed: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

static int poll_func(struct pollfd *ufds, unsigned int nfds, int timeout, void *userdata) {
    sw_discovery self = userdata;
    int ret;

    assert(self);

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));
    ret = poll(ufds, nfds, timeout);
    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    return ret;
}

static void * thread_func(void *data) {
    sw_discovery self = data;
    sigset_t mask;

    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    self->thread = pthread_self();
    self->thread_running = 1;

    for (;;) {
        char command;

        if ((command = read_command(self->thread_fd)) < 0)
            break;

/*         fprintf(stderr, "Command: %c\n", command); */

        switch (command) {

            case COMMAND_POLL: {
                int ret;

                ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

                for (;;) {
                    errno = 0;

                    if ((ret = avahi_simple_poll_run(self->simple_poll)) < 0) {

                        if (errno == EINTR)
                            continue;

                        fprintf(stderr, __FILE__": avahi_simple_poll_run() failed: %s\n", strerror(errno));
                    }

                    break;
                }

                ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

                if (write_command(self->thread_fd, ret < 0 ? COMMAND_POLL_FAILED : COMMAND_POLL_DONE) < 0)
                    break;

                break;
            }

            case COMMAND_QUIT:
                return NULL;
        }

    }

    return NULL;
}

static int oid_alloc(sw_discovery self, oid_type type) {
    sw_discovery_oid i;
    assert(self);

    for (i = 0; i < OID_MAX; i++) {

        while (self->oid_index >= OID_MAX)
            self->oid_index -= OID_MAX;

        if (self->oid_table[self->oid_index].type == OID_UNUSED) {
            self->oid_table[self->oid_index].type = type;
            self->oid_table[self->oid_index].discovery = self;

            assert(OID_GET_INDEX(&self->oid_table[self->oid_index]) == self->oid_index);

            return self->oid_index ++;
        }

        self->oid_index ++;
    }

    /* No free entry found */

    return (sw_discovery_oid) -1;
}

static void oid_release(sw_discovery self, sw_discovery_oid oid) {
    assert(self);
    assert(oid < OID_MAX);

    assert(self->oid_table[oid].type != OID_UNUSED);

    self->oid_table[oid].type = OID_UNUSED;
    self->oid_table[oid].discovery = NULL;
    self->oid_table[oid].reply = NULL;
    self->oid_table[oid].object = NULL;
    self->oid_table[oid].extra = NULL;
    self->oid_table[oid].service_data = NULL;
}

static oid_data* oid_get(sw_discovery self, sw_discovery_oid oid) {
    assert(self);

    if (oid >= OID_MAX)
        return NULL;

    if (self->oid_table[oid].type == OID_UNUSED)
        return NULL;

    return &self->oid_table[oid];
}

static service_data* service_data_new(sw_discovery self) {
    service_data *sdata;

    assert(self);

    if (!(sdata = avahi_new0(service_data, 1)))
        return NULL;

    AVAHI_LLIST_PREPEND(service_data, services, self->services, sdata);

    return sdata;

}

static void service_data_free(sw_discovery self, service_data* sdata) {
    assert(self);
    assert(sdata);

    AVAHI_LLIST_REMOVE(service_data, services, self->services, sdata);

    avahi_free(sdata->name);
    avahi_free(sdata->regtype);
    avahi_free(sdata->domain);
    avahi_free(sdata->host);
    avahi_string_list_free(sdata->txt);
    avahi_free(sdata);
}

static void reg_client_callback(oid_data *data, AvahiClientState state);

static void client_callback(AvahiClient *s, AvahiClientState state, void* userdata) {
    sw_discovery self = userdata;
    sw_discovery_oid oid;

    assert(s);
    assert(self);

    discovery_ref(self);

    for (oid = 0; oid < OID_MAX; oid++) {

        switch (self->oid_table[oid].type) {

            case OID_ENTRY_GROUP:
                reg_client_callback(&self->oid_table[oid], state);
                break;

            case OID_DOMAIN_BROWSER:
            case OID_SERVICE_BROWSER:
                ((sw_discovery_browse_reply) self->oid_table[oid].reply)(self, oid, SW_DISCOVERY_BROWSE_INVALID, 0, NULL, NULL, NULL, self->oid_table[oid].extra);
                break;

            case OID_SERVICE_RESOLVER:
            case OID_UNUSED:
                ;
        }
    }

    discovery_unref(self);
}

sw_result sw_discovery_init(sw_discovery * self) {
    int fd[2] = { -1, -1};
    sw_result result = SW_E_UNKNOWN;
    pthread_mutexattr_t mutex_attr;
    int error;

    assert(self);

    AVAHI_WARN_LINKAGE;

    *self = NULL;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
        goto fail;

    if (!(*self = avahi_new(struct _sw_discovery, 1))) {
        result = SW_E_MEM;
        goto fail;
    }

    (*self)->n_ref = 1;
    (*self)->thread_fd = fd[0];
    (*self)->main_fd = fd[1];

    (*self)->client = NULL;
    (*self)->simple_poll = NULL;

    memset((*self)->oid_table, 0, sizeof((*self)->oid_table));
    (*self)->oid_index = 0;

    (*self)->thread_running = 0;

    AVAHI_LLIST_HEAD_INIT(service_info, (*self)->services);

    ASSERT_SUCCESS(pthread_mutexattr_init(&mutex_attr));
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    ASSERT_SUCCESS(pthread_mutex_init(&(*self)->mutex, &mutex_attr));
    ASSERT_SUCCESS(pthread_mutex_init(&(*self)->salt_mutex, &mutex_attr));

    if (!((*self)->simple_poll = avahi_simple_poll_new()))
        goto fail;

    avahi_simple_poll_set_func((*self)->simple_poll, poll_func, *self);

    if (!((*self)->client = avahi_client_new(avahi_simple_poll_get((*self)->simple_poll), 0, client_callback, *self, &error))) {
        result = map_error(error);
        goto fail;
    }

    /* Start simple poll */
    if (avahi_simple_poll_prepare((*self)->simple_poll, -1) < 0)
        goto fail;

    /* Queue an initial POLL command for the thread */
    if (write_command((*self)->main_fd, COMMAND_POLL) < 0)
        goto fail;

    if (pthread_create(&(*self)->thread, NULL, thread_func, *self) != 0)
        goto fail;

    (*self)->thread_running = 1;

    return SW_OKAY;

fail:

    if (*self)
        sw_discovery_fina(*self);

    return result;
}

static int stop_thread(sw_discovery self) {
    assert(self);

    if (!self->thread_running)
        return 0;

    if (write_command(self->main_fd, COMMAND_QUIT) < 0)
        return -1;

    avahi_simple_poll_wakeup(self->simple_poll);

    ASSERT_SUCCESS(pthread_join(self->thread, NULL));
    self->thread_running = 0;
    return 0;
}

static sw_discovery discovery_ref(sw_discovery self) {
    assert(self);
    assert(self->n_ref >= 1);

    self->n_ref++;

    return self;
}

static void discovery_unref(sw_discovery self) {
    assert(self);
    assert(self->n_ref >= 1);

    if (--self->n_ref > 0)
        return;

    stop_thread(self);

    if (self->client)
        avahi_client_free(self->client);

    if (self->simple_poll)
        avahi_simple_poll_free(self->simple_poll);

    if (self->thread_fd >= 0)
        close(self->thread_fd);

    if (self->main_fd >= 0)
        close(self->main_fd);

    ASSERT_SUCCESS(pthread_mutex_destroy(&self->mutex));
    ASSERT_SUCCESS(pthread_mutex_destroy(&self->salt_mutex));

    while (self->services)
        service_data_free(self, self->services);

    avahi_free(self);
}

sw_result sw_discovery_fina(sw_discovery self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    stop_thread(self);
    discovery_unref(self);

    return SW_OKAY;
}

sw_result sw_discovery_run(sw_discovery self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    return sw_salt_run((sw_salt) self);
}

sw_result sw_discovery_stop_run(sw_discovery self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    return sw_salt_stop_run((sw_salt) self);
}

int sw_discovery_socket(sw_discovery self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    return self->main_fd;
}

sw_result sw_discovery_read_socket(sw_discovery self) {
    sw_result result = SW_E_UNKNOWN;

    assert(self);

    discovery_ref(self);

    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    /* Cleanup notification socket */
    if (read_command(self->main_fd) != COMMAND_POLL_DONE)
        goto finish;

    if (avahi_simple_poll_dispatch(self->simple_poll) < 0)
        goto finish;

    if (self->n_ref > 1) /* Perhaps we should die */

        /* Dispatch events */
        if (avahi_simple_poll_prepare(self->simple_poll, -1) < 0)
            goto finish;

    if (self->n_ref > 1)

        /* Request the poll */
        if (write_command(self->main_fd, COMMAND_POLL) < 0)
            goto finish;

    result = SW_OKAY;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

    discovery_unref(self);

    return result;
}

sw_result sw_discovery_salt(sw_discovery self, sw_salt *salt) {
    assert(self);
    assert(salt);

    AVAHI_WARN_LINKAGE;

    *salt = (sw_salt) self;

    return SW_OKAY;
}

sw_result sw_salt_step(sw_salt self, sw_uint32 * msec) {
    struct pollfd p;
    int r;
    sw_result result;

    AVAHI_WARN_LINKAGE;

    if (!((sw_discovery) self)->thread_running)
        return SW_E_UNKNOWN;

    memset(&p, 0, sizeof(p));
    p.fd = ((sw_discovery) self)->main_fd;
    p.events = POLLIN;

    if ((r = poll(&p, 1, msec ? (int) *msec : -1)) < 0) {

        /* Don't treat EINTR as error */
        if (errno == EINTR)
            return SW_OKAY;

        return SW_E_UNKNOWN;

    } else if (r == 0) {

        /* Timeoout */
        return SW_OKAY;

    } else {
        /* Success */

        if (p.revents != POLLIN)
            return SW_E_UNKNOWN;

        if ((result = sw_discovery_read_socket((sw_discovery) self)) != SW_OKAY)
            return result;
    }

    return SW_OKAY;
}

sw_result sw_salt_run(sw_salt self) {
    sw_result ret;

    AVAHI_WARN_LINKAGE;

    assert(self);

    for (;;)
        if ((ret = sw_salt_step(self, NULL)) != SW_OKAY)
            return ret;
}

sw_result sw_salt_stop_run(sw_salt self) {
    AVAHI_WARN_LINKAGE;

    assert(self);

    if (stop_thread((sw_discovery) self) < 0)
        return SW_E_UNKNOWN;

    return SW_OKAY;
}

sw_result sw_salt_lock(sw_salt self) {
    AVAHI_WARN_LINKAGE;

    assert(self);
    ASSERT_SUCCESS(pthread_mutex_lock(&((sw_discovery) self)->salt_mutex));

    return SW_OKAY;
}

sw_result sw_salt_unlock(sw_salt self) {
    assert(self);

    AVAHI_WARN_LINKAGE;

    ASSERT_SUCCESS(pthread_mutex_unlock(&((sw_discovery) self)->salt_mutex));

    return SW_OKAY;
}

static void reg_report_status(oid_data *data, sw_discovery_publish_status status) {
    sw_discovery_publish_reply reply;

    assert(data);

    reply = (sw_discovery_publish_reply) data->reply;

    reply(data->discovery,
          OID_GET_INDEX(data),
          status,
          data->extra);
}

static int reg_create_service(oid_data *data) {
    int ret;
    const char *real_type;

    assert(data);

    real_type = avahi_get_type_from_subtype(data->service_data->regtype);

    if ((ret = avahi_entry_group_add_service_strlst(
             data->object,
             data->service_data->interface,
             AVAHI_PROTO_INET,
             0,
             data->service_data->name,
             real_type ? real_type : data->service_data->regtype,
             data->service_data->domain,
             data->service_data->host,
             data->service_data->port,
             data->service_data->txt)) < 0)
        return ret;

    if (real_type) {
        /* Create a subtype entry */

        if (avahi_entry_group_add_service_subtype(
                data->object,
                data->service_data->interface,
                AVAHI_PROTO_INET,
                0,
                data->service_data->name,
                real_type,
                data->service_data->domain,
                data->service_data->regtype) < 0)
            return ret;

    }

    if ((ret = avahi_entry_group_commit(data->object)) < 0)
        return ret;

    return 0;
}

static void reg_client_callback(oid_data *data, AvahiClientState state) {
    assert(data);

    /* We've not been setup completely */
    if (!data->object)
        return;

    switch (state) {
        case AVAHI_CLIENT_FAILURE:
            reg_report_status(data, SW_DISCOVERY_PUBLISH_INVALID);
            break;

        case AVAHI_CLIENT_S_RUNNING: {
            int ret;

            /* Register the service */
            if ((ret = reg_create_service(data)) < 0) {
                reg_report_status(data, SW_DISCOVERY_PUBLISH_INVALID);
                return;
            }

            break;
        }

        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:

            /* Remove our entry */
            avahi_entry_group_reset(data->object);
            break;

        case AVAHI_CLIENT_CONNECTING:
            /* Ignore */
            break;
    }

}

static void reg_entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    oid_data *data = userdata;

    assert(g);
    assert(data);

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:

            reg_report_status(data, SW_DISCOVERY_PUBLISH_STARTED);
            break;

        case AVAHI_ENTRY_GROUP_COLLISION:

            reg_report_status(data, SW_DISCOVERY_PUBLISH_NAME_COLLISION);
            break;

        case AVAHI_ENTRY_GROUP_REGISTERING:
        case AVAHI_ENTRY_GROUP_UNCOMMITED:
            /* Ignore */
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:
            reg_report_status(data, SW_DISCOVERY_PUBLISH_INVALID);
            break;

    }
}

sw_result sw_discovery_publish(
    sw_discovery self,
    sw_uint32 interface_index,
    sw_const_string name,
    sw_const_string type,
    sw_const_string domain,
    sw_const_string host,
    sw_port port,
    sw_octets text_record,
    sw_uint32 text_record_len,
    sw_discovery_publish_reply reply,
    sw_opaque extra,
    sw_discovery_oid * oid) {

    oid_data *data;
    sw_result result = SW_E_UNKNOWN;
    service_data *sdata;
    AvahiStringList *txt = NULL;

    assert(self);
    assert(name);
    assert(type);
    assert(reply);
    assert(oid);

    AVAHI_WARN_LINKAGE;

    if (text_record && text_record_len > 0)
        if (avahi_string_list_parse(text_record, text_record_len, &txt) < 0)
            return SW_E_UNKNOWN;

    if ((*oid = oid_alloc(self, OID_ENTRY_GROUP)) == (sw_discovery_oid) -1) {
        avahi_string_list_free(txt);
        return SW_E_UNKNOWN;
    }

    if (!(sdata = service_data_new(self))) {
        avahi_string_list_free(txt);
        oid_release(self, *oid);
        return SW_E_MEM;
    }

    data = oid_get(self, *oid);
    assert(data);
    data->reply = (sw_result (*)(void)) reply;
    data->extra = extra;
    data->service_data = sdata;

    sdata->interface = interface_index == 0 ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface_index;
    sdata->name = avahi_strdup(name);
    sdata->regtype = type ? avahi_normalize_name_strdup(type) : NULL;
    sdata->domain = domain ? avahi_normalize_name_strdup(domain) : NULL;
    sdata->host = host ? avahi_normalize_name_strdup(host) : NULL;
    sdata->port = port;
    sdata->txt = txt;

    /* Some OOM checking would be cool here */

    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    if (!(data->object = avahi_entry_group_new(self->client, reg_entry_group_callback, data))) {
        result = map_error(avahi_client_errno(self->client));
        goto finish;
    }

    if (avahi_client_get_state(self->client) == AVAHI_CLIENT_S_RUNNING) {
        int error;

        if ((error = reg_create_service(data)) < 0) {
            result = map_error(error);
            goto finish;
        }
    }

    result = SW_OKAY;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

    if (result != SW_OKAY)
        if (*oid != (sw_discovery_oid) -1)
            sw_discovery_cancel(self, *oid);

    return result;
}

static void domain_browser_callback(
    AvahiDomainBrowser *b,
    AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    oid_data* data = userdata;
    sw_discovery_browse_reply reply;
    static char domain_fixed[AVAHI_DOMAIN_NAME_MAX];

    assert(b);
    assert(data);

    reply = (sw_discovery_browse_reply) data->reply;

    domain  = add_trailing_dot(domain, domain_fixed, sizeof(domain_fixed));

    switch (event) {
        case AVAHI_BROWSER_NEW:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_ADD_DOMAIN, interface, NULL, NULL, domain, data->extra);
            break;

        case AVAHI_BROWSER_REMOVE:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_REMOVE_DOMAIN, interface, NULL, NULL, domain, data->extra);
            break;

        case AVAHI_BROWSER_FAILURE:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_INVALID, interface, NULL, NULL, domain, data->extra);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;
    }
}

sw_result sw_discovery_browse_domains(
    sw_discovery self,
    sw_uint32 interface_index,
    sw_discovery_browse_reply reply,
    sw_opaque extra,
    sw_discovery_oid * oid) {

    oid_data *data;
    AvahiIfIndex ifindex;
    sw_result result = SW_E_UNKNOWN;

    assert(self);
    assert(reply);
    assert(oid);

    AVAHI_WARN_LINKAGE;

    if ((*oid = oid_alloc(self, OID_DOMAIN_BROWSER)) == (sw_discovery_oid) -1)
        return SW_E_UNKNOWN;

    data = oid_get(self, *oid);
    assert(data);
    data->reply = (sw_result (*)(void)) reply;
    data->extra = extra;

    ifindex = interface_index == 0 ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface_index;

    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    if (!(data->object = avahi_domain_browser_new(self->client, ifindex, AVAHI_PROTO_INET, NULL, AVAHI_DOMAIN_BROWSER_BROWSE, 0, domain_browser_callback, data))) {
        result = map_error(avahi_client_errno(self->client));
        goto finish;
    }

    result = SW_OKAY;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

    if (result != SW_OKAY)
        if (*oid != (sw_discovery_oid) -1)
            sw_discovery_cancel(self, *oid);

    return result;
}

static void service_resolver_callback(
    AvahiServiceResolver *r,
    AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    oid_data* data = userdata;
    sw_discovery_resolve_reply reply;

    assert(r);
    assert(data);

    reply = (sw_discovery_resolve_reply) data->reply;

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {

            char host_name_fixed[AVAHI_DOMAIN_NAME_MAX];
            uint8_t *p = NULL;
            size_t l = 0;
            sw_ipv4_address addr;

            sw_ipv4_address_init_from_saddr(&addr, a->data.ipv4.address);

            host_name = add_trailing_dot(host_name, host_name_fixed, sizeof(host_name_fixed));

            if ((p = avahi_new0(uint8_t, (l = avahi_string_list_serialize(txt, NULL, 0))+1)))
                avahi_string_list_serialize(txt, p, l);

            reply(data->discovery, OID_GET_INDEX(data), interface, name, type, domain, addr, port, p, l, data->extra);

            avahi_free(p);
            break;
        }

        case AVAHI_RESOLVER_FAILURE:

            /* Apparently there is no way in HOWL to inform about failed resolvings ... */

            avahi_warn("A service failed to resolve in the HOWL compatiblity layer of Avahi which is used by '%s'. "
                       "Since the HOWL API doesn't offer any means to inform the application about this, we have to ignore the failure. "
                       "Please fix your application to use the native API of Avahi!",
                       avahi_exe_name());

            break;
    }
}

sw_result sw_discovery_resolve(
    sw_discovery self,
    sw_uint32 interface_index,
    sw_const_string name,
    sw_const_string type,
    sw_const_string domain,
    sw_discovery_resolve_reply reply,
    sw_opaque extra,
    sw_discovery_oid * oid) {

    oid_data *data;
    AvahiIfIndex ifindex;
    sw_result result = SW_E_UNKNOWN;

    assert(self);
    assert(name);
    assert(type);
    assert(reply);
    assert(oid);

    AVAHI_WARN_LINKAGE;

    if ((*oid = oid_alloc(self, OID_SERVICE_RESOLVER)) == (sw_discovery_oid) -1)
        return SW_E_UNKNOWN;

    data = oid_get(self, *oid);
    assert(data);
    data->reply = (sw_result (*)(void)) reply;
    data->extra = extra;

    ifindex = interface_index == 0 ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface_index;

    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    if (!(data->object = avahi_service_resolver_new(self->client, ifindex, AVAHI_PROTO_INET, name, type, domain, AVAHI_PROTO_INET, 0, service_resolver_callback, data))) {
        result = map_error(avahi_client_errno(self->client));
        goto finish;
    }

    result = SW_OKAY;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

    if (result != SW_OKAY)
        if (*oid != (sw_discovery_oid) -1)
            sw_discovery_cancel(self, *oid);

    return result;
}

static void service_browser_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    oid_data* data = userdata;
    char type_fixed[AVAHI_DOMAIN_NAME_MAX], domain_fixed[AVAHI_DOMAIN_NAME_MAX];
    sw_discovery_browse_reply reply;

    assert(b);
    assert(data);

    reply = (sw_discovery_browse_reply) data->reply;

    type = add_trailing_dot(type, type_fixed, sizeof(type_fixed));
    domain = add_trailing_dot(domain, domain_fixed, sizeof(domain_fixed));

    switch (event) {
        case AVAHI_BROWSER_NEW:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_ADD_SERVICE, interface, name, type, domain, data->extra);
            break;

        case AVAHI_BROWSER_REMOVE:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_REMOVE_SERVICE, interface, name, type, domain, data->extra);
            break;

        case AVAHI_BROWSER_FAILURE:
            reply(data->discovery, OID_GET_INDEX(data), SW_DISCOVERY_BROWSE_INVALID, interface, name, type, domain, data->extra);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;
    }
}

sw_result sw_discovery_browse(
    sw_discovery self,
    sw_uint32 interface_index,
    sw_const_string type,
    sw_const_string domain,
    sw_discovery_browse_reply reply,
    sw_opaque extra,
    sw_discovery_oid * oid) {

    oid_data *data;
    AvahiIfIndex ifindex;
    sw_result result = SW_E_UNKNOWN;

    assert(self);
    assert(type);
    assert(reply);
    assert(oid);

    AVAHI_WARN_LINKAGE;

    if ((*oid = oid_alloc(self, OID_SERVICE_BROWSER)) == (sw_discovery_oid) -1)
        return SW_E_UNKNOWN;

    data = oid_get(self, *oid);
    assert(data);
    data->reply = (sw_result (*)(void)) reply;
    data->extra = extra;

    ifindex = interface_index == 0 ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface_index;

    ASSERT_SUCCESS(pthread_mutex_lock(&self->mutex));

    if (!(data->object = avahi_service_browser_new(self->client, ifindex, AVAHI_PROTO_INET, type, domain, 0, service_browser_callback, data))) {
        result = map_error(avahi_client_errno(self->client));
        goto finish;
    }

    result = SW_OKAY;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&self->mutex));

    if (result != SW_OKAY)
        if (*oid != (sw_discovery_oid) -1)
            sw_discovery_cancel(self, *oid);

    return result;
}

sw_result sw_discovery_cancel(sw_discovery self, sw_discovery_oid oid) {
    oid_data *data;
    assert(self);

    AVAHI_WARN_LINKAGE;

    if (!(data = oid_get(self, oid)))
        return SW_E_UNKNOWN;

    if (data->object) {
        switch (data->type) {
            case OID_SERVICE_BROWSER:
                avahi_service_browser_free(data->object);
                break;

            case OID_SERVICE_RESOLVER:
                avahi_service_resolver_free(data->object);
                break;

            case OID_DOMAIN_BROWSER:
                avahi_domain_browser_free(data->object);
                break;

            case OID_ENTRY_GROUP:
                avahi_entry_group_free(data->object);
                break;

            case OID_UNUSED:
            ;
        }
    }

    if (data->service_data) {
        assert(data->type == OID_ENTRY_GROUP);
        service_data_free(self, data->service_data);
    }

    oid_release(self, oid);

    return SW_OKAY;
}

sw_result sw_discovery_init_with_flags(
    sw_discovery * self,
    sw_discovery_init_flags flags) {

    assert(self);

    AVAHI_WARN_LINKAGE;

    if (flags != SW_DISCOVERY_USE_SHARED_SERVICE)
        return SW_E_NO_IMPL;

    return sw_discovery_init(self);
}
