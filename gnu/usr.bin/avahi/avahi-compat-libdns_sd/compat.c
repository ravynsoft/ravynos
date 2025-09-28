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

#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>
#include <avahi-common/domain.h>
#include <avahi-common/alternative.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-client/lookup.h>

#include "warn.h"
#include "dns_sd.h"

enum {
    COMMAND_POLL = 'p',
    COMMAND_QUIT = 'q',
    COMMAND_POLL_DONE = 'P',
    COMMAND_POLL_FAILED = 'F'
};

struct type_info {
    char *type;
    AvahiStringList *subtypes;
    int n_subtypes;
};

struct _DNSServiceRef_t {
    int n_ref;

    AvahiSimplePoll *simple_poll;

    int thread_fd, main_fd;

    pthread_t thread;
    int thread_running;

    pthread_mutex_t mutex;

    void *context;
    DNSServiceBrowseReply service_browser_callback;
    DNSServiceResolveReply service_resolver_callback;
    DNSServiceDomainEnumReply domain_browser_callback;
    DNSServiceRegisterReply service_register_callback;
    DNSServiceQueryRecordReply query_resolver_callback;

    AvahiClient *client;
    AvahiServiceBrowser *service_browser;
    AvahiServiceResolver *service_resolver;
    AvahiDomainBrowser *domain_browser;
    AvahiRecordBrowser *record_browser;

    struct type_info type_info;
    char *service_name, *service_name_chosen, *service_domain, *service_host;
    uint16_t service_port;
    AvahiIfIndex service_interface;
    AvahiStringList *service_txt;

    AvahiEntryGroup *entry_group;
};

#define ASSERT_SUCCESS(r) { int __ret = (r); assert(__ret == 0); }

static DNSServiceErrorType map_error(int error) {
    switch (error) {
        case AVAHI_OK :
            return kDNSServiceErr_NoError;

        case AVAHI_ERR_BAD_STATE :
            return kDNSServiceErr_BadState;

        case AVAHI_ERR_INVALID_HOST_NAME:
        case AVAHI_ERR_INVALID_DOMAIN_NAME:
        case AVAHI_ERR_INVALID_TTL:
        case AVAHI_ERR_IS_PATTERN:
        case AVAHI_ERR_INVALID_RECORD:
        case AVAHI_ERR_INVALID_SERVICE_NAME:
        case AVAHI_ERR_INVALID_SERVICE_TYPE:
        case AVAHI_ERR_INVALID_PORT:
        case AVAHI_ERR_INVALID_KEY:
        case AVAHI_ERR_INVALID_ADDRESS:
        case AVAHI_ERR_INVALID_SERVICE_SUBTYPE:
            return kDNSServiceErr_BadParam;


        case AVAHI_ERR_COLLISION:
            return kDNSServiceErr_NameConflict;

        case AVAHI_ERR_TOO_MANY_CLIENTS:
        case AVAHI_ERR_TOO_MANY_OBJECTS:
        case AVAHI_ERR_TOO_MANY_ENTRIES:
        case AVAHI_ERR_ACCESS_DENIED:
            return kDNSServiceErr_Refused;

        case AVAHI_ERR_INVALID_OPERATION:
        case AVAHI_ERR_INVALID_OBJECT:
            return kDNSServiceErr_Invalid;

        case AVAHI_ERR_NO_MEMORY:
            return kDNSServiceErr_NoMemory;

        case AVAHI_ERR_INVALID_INTERFACE:
        case AVAHI_ERR_INVALID_PROTOCOL:
            return kDNSServiceErr_BadInterfaceIndex;

        case AVAHI_ERR_INVALID_FLAGS:
            return kDNSServiceErr_BadFlags;

        case AVAHI_ERR_NOT_FOUND:
            return kDNSServiceErr_NoSuchName;

        case AVAHI_ERR_VERSION_MISMATCH:
            return kDNSServiceErr_Incompatible;

        case AVAHI_ERR_NO_NETWORK:
        case AVAHI_ERR_OS:
        case AVAHI_ERR_INVALID_CONFIG:
        case AVAHI_ERR_TIMEOUT:
        case AVAHI_ERR_DBUS_ERROR:
        case AVAHI_ERR_DISCONNECTED:
        case AVAHI_ERR_NO_DAEMON:
            break;

    }

    return kDNSServiceErr_Unknown;
}

static void type_info_init(struct type_info *i) {
    assert(i);
    i->type = NULL;
    i->subtypes = NULL;
    i->n_subtypes = 0;
}

static void type_info_free(struct type_info *i) {
    assert(i);

    avahi_free(i->type);
    avahi_string_list_free(i->subtypes);

    type_info_init(i);
}

static int type_info_parse(struct type_info *i, const char *t) {
    char *token = NULL;

    assert(i);
    assert(t);

    type_info_init(i);

    for (;;) {
        size_t l;

        if (*t == 0)
            break;

        l = strcspn(t, ",");

        if (l <= 0)
            goto fail;

        token = avahi_strndup(t, l);

        if (!token)
            goto fail;

        if (!i->type) {
            /* This is the first token, hence the main type */

            if (!avahi_is_valid_service_type_strict(token))
                goto fail;

            i->type = token;
            token = NULL;
        } else {
            char *fst;

            /* This is not the first token, hence a subtype */

            if (!(fst = avahi_strdup_printf("%s._sub.%s", token, i->type)))
                goto fail;

            if (!avahi_is_valid_service_subtype(fst)) {
                avahi_free(fst);
                goto fail;
            }

            i->subtypes = avahi_string_list_add(i->subtypes, fst);
            avahi_free(fst);

            avahi_free(token);
            token = NULL;

            i->n_subtypes++;
        }

        t += l;

        if (*t == ',')
            t++;
    }

    if (i->type)
        return 0;

fail:
    type_info_free(i);
    avahi_free(token);
    return -1;
}

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
    DNSServiceRef sdref = userdata;
    int ret;

    assert(sdref);

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

/*     fprintf(stderr, "pre-syscall\n"); */
    ret = poll(ufds, nfds, timeout);
/*     fprintf(stderr, "post-syscall\n"); */

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    return ret;
}

static void * thread_func(void *data) {
    DNSServiceRef sdref = data;
    sigset_t mask;

    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    sdref->thread = pthread_self();
    sdref->thread_running = 1;

    for (;;) {
        char command;

        if ((command = read_command(sdref->thread_fd)) < 0)
            break;

/*         fprintf(stderr, "Command: %c\n", command); */

        switch (command) {

            case COMMAND_POLL: {
                int ret;

                ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

                for (;;) {
                    errno = 0;

                    if ((ret = avahi_simple_poll_run(sdref->simple_poll)) < 0) {

                        if (errno == EINTR)
                            continue;

                        fprintf(stderr, __FILE__": avahi_simple_poll_run() failed: %s\n", strerror(errno));
                    }

                    break;
                }

                ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

                if (write_command(sdref->thread_fd, ret < 0 ? COMMAND_POLL_FAILED : COMMAND_POLL_DONE) < 0)
                    break;

                break;
            }

            case COMMAND_QUIT:
                return NULL;
        }

    }

    return NULL;
}

static DNSServiceRef sdref_new(void) {
    int fd[2] = { -1, -1 };
    DNSServiceRef sdref = NULL;
    pthread_mutexattr_t mutex_attr;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, fd) < 0)
        goto fail;

    if (!(sdref = avahi_new(struct _DNSServiceRef_t, 1)))
        goto fail;

    sdref->n_ref = 1;
    sdref->thread_fd = fd[0];
    sdref->main_fd = fd[1];

    sdref->client = NULL;
    sdref->service_browser = NULL;
    sdref->service_resolver = NULL;
    sdref->domain_browser = NULL;
    sdref->entry_group = NULL;

    sdref->service_name = sdref->service_name_chosen = sdref->service_domain = sdref->service_host = NULL;
    sdref->service_txt = NULL;

    type_info_init(&sdref->type_info);

    ASSERT_SUCCESS(pthread_mutexattr_init(&mutex_attr));
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    ASSERT_SUCCESS(pthread_mutex_init(&sdref->mutex, &mutex_attr));

    sdref->thread_running = 0;

    if (!(sdref->simple_poll = avahi_simple_poll_new()))
        goto fail;

    avahi_simple_poll_set_func(sdref->simple_poll, poll_func, sdref);

    /* Start simple poll */
    if (avahi_simple_poll_prepare(sdref->simple_poll, -1) < 0)
        goto fail;

    /* Queue an initial POLL command for the thread */
    if (write_command(sdref->main_fd, COMMAND_POLL) < 0)
        goto fail;

    if (pthread_create(&sdref->thread, NULL, thread_func, sdref) != 0)
        goto fail;

    sdref->thread_running = 1;

    return sdref;

fail:

    if (sdref)
        DNSServiceRefDeallocate(sdref);

    return NULL;
}

static void sdref_free(DNSServiceRef sdref) {
    assert(sdref);

    if (sdref->thread_running) {
        ASSERT_SUCCESS(write_command(sdref->main_fd, COMMAND_QUIT));
        avahi_simple_poll_wakeup(sdref->simple_poll);
        ASSERT_SUCCESS(pthread_join(sdref->thread, NULL));
    }

    if (sdref->client)
        avahi_client_free(sdref->client);

    if (sdref->simple_poll)
        avahi_simple_poll_free(sdref->simple_poll);

    if (sdref->thread_fd >= 0)
        close(sdref->thread_fd);

    if (sdref->main_fd >= 0)
        close(sdref->main_fd);

    ASSERT_SUCCESS(pthread_mutex_destroy(&sdref->mutex));

    avahi_free(sdref->service_name);
    avahi_free(sdref->service_name_chosen);
    avahi_free(sdref->service_domain);
    avahi_free(sdref->service_host);

    type_info_free(&sdref->type_info);

    avahi_string_list_free(sdref->service_txt);

    avahi_free(sdref);
}

static void sdref_ref(DNSServiceRef sdref) {
    assert(sdref);
    assert(sdref->n_ref >= 1);

    sdref->n_ref++;
}

static void sdref_unref(DNSServiceRef sdref) {
    assert(sdref);
    assert(sdref->n_ref >= 1);

    if (--(sdref->n_ref) <= 0)
        sdref_free(sdref);
}

int DNSSD_API DNSServiceRefSockFD(DNSServiceRef sdref) {

    AVAHI_WARN_LINKAGE;

    if (!sdref || sdref->n_ref <= 0)
        return -1;

    return sdref->main_fd;
}

DNSServiceErrorType DNSSD_API DNSServiceProcessResult(DNSServiceRef sdref) {
    DNSServiceErrorType ret = kDNSServiceErr_Unknown;

    AVAHI_WARN_LINKAGE;

    if (!sdref || sdref->n_ref <= 0)
        return kDNSServiceErr_BadParam;

    sdref_ref(sdref);

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    /* Cleanup notification socket */
    if (read_command(sdref->main_fd) != COMMAND_POLL_DONE)
        goto finish;

    if (avahi_simple_poll_dispatch(sdref->simple_poll) < 0)
        goto finish;

    if (sdref->n_ref > 1) /* Perhaps we should die */

        /* Dispatch events */
        if (avahi_simple_poll_prepare(sdref->simple_poll, -1) < 0)
            goto finish;

    if (sdref->n_ref > 1)

        /* Request the poll */
        if (write_command(sdref->main_fd, COMMAND_POLL) < 0)
            goto finish;

    ret = kDNSServiceErr_NoError;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    sdref_unref(sdref);

    return ret;
}

void DNSSD_API DNSServiceRefDeallocate(DNSServiceRef sdref) {
    AVAHI_WARN_LINKAGE;

    if (sdref)
        sdref_unref(sdref);
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

    DNSServiceRef sdref = userdata;
    char type_fixed[AVAHI_DOMAIN_NAME_MAX], domain_fixed[AVAHI_DOMAIN_NAME_MAX];
    assert(b);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    type = add_trailing_dot(type, type_fixed, sizeof(type_fixed));
    domain  = add_trailing_dot(domain, domain_fixed, sizeof(domain_fixed));

    switch (event) {
        case AVAHI_BROWSER_NEW:
            sdref->service_browser_callback(sdref, kDNSServiceFlagsAdd, interface, kDNSServiceErr_NoError, name, type, domain, sdref->context);
            break;

        case AVAHI_BROWSER_REMOVE:
            sdref->service_browser_callback(sdref, 0, interface, kDNSServiceErr_NoError, name, type, domain, sdref->context);
            break;

        case AVAHI_BROWSER_FAILURE:
            sdref->service_browser_callback(sdref, 0, interface, map_error(avahi_client_errno(sdref->client)), NULL, NULL, NULL, sdref->context);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;
    }
}

static void generic_client_callback(AvahiClient *s, AvahiClientState state, void* userdata) {
    DNSServiceRef sdref = userdata;
    int error = kDNSServiceErr_Unknown;

    assert(s);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    switch (state) {

        case AVAHI_CLIENT_FAILURE:

            if (sdref->service_browser_callback)
                sdref->service_browser_callback(sdref, 0, 0, error, NULL, NULL, NULL, sdref->context);
            else if (sdref->service_resolver_callback)
                sdref->service_resolver_callback(sdref, 0, 0, error, NULL, NULL, 0, 0, NULL, sdref->context);
            else if (sdref->domain_browser_callback)
                sdref->domain_browser_callback(sdref, 0, 0, error, NULL, sdref->context);
            else if (sdref->query_resolver_callback)
                sdref->query_resolver_callback(sdref, 0, 0, error, NULL, 0, 0, 0, NULL, 0, sdref->context);

            break;

        case AVAHI_CLIENT_S_RUNNING:
        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:
        case AVAHI_CLIENT_CONNECTING:
            break;
    }
}

DNSServiceErrorType DNSSD_API DNSServiceBrowse(
        DNSServiceRef *ret_sdref,
        DNSServiceFlags flags,
        uint32_t interface,
        const char *regtype,
        const char *domain,
        DNSServiceBrowseReply callback,
        void *context) {

    DNSServiceErrorType ret = kDNSServiceErr_Unknown;
    int error;
    DNSServiceRef sdref = NULL;
    AvahiIfIndex ifindex;
    struct type_info type_info;

    AVAHI_WARN_LINKAGE;

    if (!ret_sdref || !regtype)
        return kDNSServiceErr_BadParam;
    *ret_sdref = NULL;

    if (interface == kDNSServiceInterfaceIndexLocalOnly || flags != 0) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    type_info_init(&type_info);

    if (type_info_parse(&type_info, regtype) < 0 || type_info.n_subtypes > 1) {
        type_info_free(&type_info);

        if (!avahi_is_valid_service_type_generic(regtype))
            return kDNSServiceErr_Unsupported;
    } else
        regtype = type_info.subtypes ? (char*) type_info.subtypes->text : type_info.type;

    if (!(sdref = sdref_new())) {
        type_info_free(&type_info);
        return kDNSServiceErr_Unknown;
    }

    sdref->context = context;
    sdref->service_browser_callback = callback;

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!(sdref->client = avahi_client_new(avahi_simple_poll_get(sdref->simple_poll), 0, generic_client_callback, sdref, &error))) {
        ret =  map_error(error);
        goto finish;
    }

    ifindex = interface == kDNSServiceInterfaceIndexAny ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface;

    if (!(sdref->service_browser = avahi_service_browser_new(sdref->client, ifindex, AVAHI_PROTO_UNSPEC, regtype, domain, 0, service_browser_callback, sdref))) {
        ret = map_error(avahi_client_errno(sdref->client));
        goto finish;
    }

    ret = kDNSServiceErr_NoError;
    *ret_sdref = sdref;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    if (ret != kDNSServiceErr_NoError)
        DNSServiceRefDeallocate(sdref);

    type_info_free(&type_info);

    return ret;
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
    AVAHI_GCC_UNUSED const AvahiAddress *a,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    DNSServiceRef sdref = userdata;

    assert(r);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    switch (event) {
        case AVAHI_RESOLVER_FOUND: {

            char host_name_fixed[AVAHI_DOMAIN_NAME_MAX];
            char full_name[AVAHI_DOMAIN_NAME_MAX];
            int ret;
            char *p = NULL;
            size_t l = 0;

            host_name = add_trailing_dot(host_name, host_name_fixed, sizeof(host_name_fixed));

            if ((p = avahi_new0(char, (l = avahi_string_list_serialize(txt, NULL, 0))+1)))
                avahi_string_list_serialize(txt, p, l);

            ret = avahi_service_name_join(full_name, sizeof(full_name), name, type, domain);
            assert(ret == AVAHI_OK);

            strcat(full_name, ".");

            sdref->service_resolver_callback(sdref, 0, interface, kDNSServiceErr_NoError, full_name, host_name, htons(port), l, (unsigned char*) p, sdref->context);

            avahi_free(p);
            break;
        }

        case AVAHI_RESOLVER_FAILURE:
            sdref->service_resolver_callback(sdref, 0, interface, map_error(avahi_client_errno(sdref->client)), NULL, NULL, 0, 0, NULL, sdref->context);
            break;
    }
}

DNSServiceErrorType DNSSD_API DNSServiceResolve(
    DNSServiceRef *ret_sdref,
    DNSServiceFlags flags,
    uint32_t interface,
    const char *name,
    const char *regtype,
    const char *domain,
    DNSServiceResolveReply callback,
    void *context) {

    DNSServiceErrorType ret = kDNSServiceErr_Unknown;
    int error;
    DNSServiceRef sdref = NULL;
    AvahiIfIndex ifindex;

    AVAHI_WARN_LINKAGE;

    if (!ret_sdref || !name || !regtype || !domain || !callback)
        return kDNSServiceErr_BadParam;
    *ret_sdref = NULL;

    if (interface == kDNSServiceInterfaceIndexLocalOnly || flags != 0) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    if (!(sdref = sdref_new()))
        return kDNSServiceErr_Unknown;

    sdref->context = context;
    sdref->service_resolver_callback = callback;

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!(sdref->client = avahi_client_new(avahi_simple_poll_get(sdref->simple_poll), 0, generic_client_callback, sdref, &error))) {
        ret =  map_error(error);
        goto finish;
    }

    ifindex = interface == kDNSServiceInterfaceIndexAny ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface;

    if (!(sdref->service_resolver = avahi_service_resolver_new(sdref->client, ifindex, AVAHI_PROTO_UNSPEC, name, regtype, domain, AVAHI_PROTO_UNSPEC, 0, service_resolver_callback, sdref))) {
        ret = map_error(avahi_client_errno(sdref->client));
        goto finish;
    }


    ret = kDNSServiceErr_NoError;
    *ret_sdref = sdref;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    if (ret != kDNSServiceErr_NoError)
        DNSServiceRefDeallocate(sdref);

    return ret;
}

int DNSSD_API DNSServiceConstructFullName (
    char *fullName,
    const char *service,
    const char *regtype,
    const char *domain) {

    AVAHI_WARN_LINKAGE;

    if (!fullName || !regtype || !domain)
        return -1;

    if (avahi_service_name_join(fullName, kDNSServiceMaxDomainName, service, regtype, domain) < 0)
        return -1;

    return 0;
}

static void domain_browser_callback(
    AvahiDomainBrowser *b,
    AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void *userdata) {

    DNSServiceRef sdref = userdata;
    static char domain_fixed[AVAHI_DOMAIN_NAME_MAX];

    assert(b);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    domain  = add_trailing_dot(domain, domain_fixed, sizeof(domain_fixed));

    switch (event) {
        case AVAHI_BROWSER_NEW:
            sdref->domain_browser_callback(sdref, kDNSServiceFlagsAdd, interface, kDNSServiceErr_NoError, domain, sdref->context);
            break;

        case AVAHI_BROWSER_REMOVE:
            sdref->domain_browser_callback(sdref, 0, interface, kDNSServiceErr_NoError, domain, sdref->context);
            break;

        case AVAHI_BROWSER_FAILURE:
            sdref->domain_browser_callback(sdref, 0, interface, map_error(avahi_client_errno(sdref->client)), domain, sdref->context);
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
        case AVAHI_BROWSER_ALL_FOR_NOW:
            break;
    }
}

DNSServiceErrorType DNSSD_API DNSServiceEnumerateDomains(
    DNSServiceRef *ret_sdref,
    DNSServiceFlags flags,
    uint32_t interface,
    DNSServiceDomainEnumReply callback,
    void *context) {

    DNSServiceErrorType ret = kDNSServiceErr_Unknown;
    int error;
    DNSServiceRef sdref = NULL;
    AvahiIfIndex ifindex;

    AVAHI_WARN_LINKAGE;

    if (!ret_sdref || !callback)
        return kDNSServiceErr_BadParam;
    *ret_sdref = NULL;

    if (interface == kDNSServiceInterfaceIndexLocalOnly ||
        (flags != kDNSServiceFlagsBrowseDomains &&  flags != kDNSServiceFlagsRegistrationDomains)) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    if (!(sdref = sdref_new()))
        return kDNSServiceErr_Unknown;

    sdref->context = context;
    sdref->domain_browser_callback = callback;

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!(sdref->client = avahi_client_new(avahi_simple_poll_get(sdref->simple_poll), 0, generic_client_callback, sdref, &error))) {
        ret =  map_error(error);
        goto finish;
    }

    ifindex = interface == kDNSServiceInterfaceIndexAny ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface;

    if (!(sdref->domain_browser = avahi_domain_browser_new(sdref->client, ifindex, AVAHI_PROTO_UNSPEC, "local",
                                                           flags == kDNSServiceFlagsRegistrationDomains ? AVAHI_DOMAIN_BROWSER_REGISTER : AVAHI_DOMAIN_BROWSER_BROWSE,
                                                           0, domain_browser_callback, sdref))) {
        ret = map_error(avahi_client_errno(sdref->client));
        goto finish;
    }

    ret = kDNSServiceErr_NoError;
    *ret_sdref = sdref;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    if (ret != kDNSServiceErr_NoError)
        DNSServiceRefDeallocate(sdref);

    return ret;
}

static void reg_report_error(DNSServiceRef sdref, DNSServiceErrorType error) {
    char regtype_fixed[AVAHI_DOMAIN_NAME_MAX], domain_fixed[AVAHI_DOMAIN_NAME_MAX];
    const char *regtype, *domain;
    assert(sdref);
    assert(sdref->n_ref >= 1);

    if (!sdref->service_register_callback)
        return;

    regtype = add_trailing_dot(sdref->type_info.type, regtype_fixed, sizeof(regtype_fixed));
    domain = add_trailing_dot(sdref->service_domain, domain_fixed, sizeof(domain_fixed));

    sdref->service_register_callback(
        sdref, 0, error,
        sdref->service_name_chosen ? sdref->service_name_chosen : sdref->service_name,
        regtype,
        domain,
        sdref->context);
}

static int reg_create_service(DNSServiceRef sdref) {
    int ret;
    AvahiStringList *l;

    assert(sdref);
    assert(sdref->n_ref >= 1);

    if ((ret = avahi_entry_group_add_service_strlst(
        sdref->entry_group,
        sdref->service_interface,
        AVAHI_PROTO_UNSPEC,
        0,
        sdref->service_name_chosen,
        sdref->type_info.type,
        sdref->service_domain,
        sdref->service_host,
        sdref->service_port,
        sdref->service_txt)) < 0)
        return ret;

    for (l = sdref->type_info.subtypes; l; l = l->next) {
        /* Create a subtype entry */

        if (avahi_entry_group_add_service_subtype(
                sdref->entry_group,
                sdref->service_interface,
                AVAHI_PROTO_UNSPEC,
                0,
                sdref->service_name_chosen,
                sdref->type_info.type,
                sdref->service_domain,
                (const char*) l->text) < 0)
            return ret;
    }

    if ((ret = avahi_entry_group_commit(sdref->entry_group)) < 0)
        return ret;

    return 0;
}

static void reg_client_callback(AvahiClient *s, AvahiClientState state, void* userdata) {
    DNSServiceRef sdref = userdata;

    assert(s);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    /* We've not been setup completely */
    if (!sdref->entry_group)
        return;

    switch (state) {
        case AVAHI_CLIENT_FAILURE:
            reg_report_error(sdref, kDNSServiceErr_Unknown);
            break;

        case AVAHI_CLIENT_S_RUNNING: {
            int ret;

            if (!sdref->service_name) {
                const char *n;
                /* If the service name is taken from the host name, copy that */

                avahi_free(sdref->service_name_chosen);
                sdref->service_name_chosen = NULL;

                if (!(n = avahi_client_get_host_name(sdref->client))) {
                    reg_report_error(sdref, map_error(avahi_client_errno(sdref->client)));
                    return;
                }

                if (!(sdref->service_name_chosen = avahi_strdup(n))) {
                    reg_report_error(sdref, kDNSServiceErr_NoMemory);
                    return;
                }
            }

            if (!sdref->service_name_chosen) {

                assert(sdref->service_name);

                if (!(sdref->service_name_chosen = avahi_strdup(sdref->service_name))) {
                    reg_report_error(sdref, kDNSServiceErr_NoMemory);
                    return;
                }
            }

            /* Register the service */

            if ((ret = reg_create_service(sdref)) < 0) {
                reg_report_error(sdref, map_error(ret));
                return;
            }

            break;
        }

        case AVAHI_CLIENT_S_COLLISION:
        case AVAHI_CLIENT_S_REGISTERING:

            /* Remove our entry */
            avahi_entry_group_reset(sdref->entry_group);

            break;

        case AVAHI_CLIENT_CONNECTING:
            /* Ignore */
            break;
    }

}

static void reg_entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state, void *userdata) {
    DNSServiceRef sdref = userdata;

    assert(g);

    switch (state) {
        case AVAHI_ENTRY_GROUP_ESTABLISHED:

            /* Inform the user */
            reg_report_error(sdref, kDNSServiceErr_NoError);

            break;

        case AVAHI_ENTRY_GROUP_COLLISION: {
            char *n;
            int ret;

            /* Remove our entry */
            avahi_entry_group_reset(sdref->entry_group);

            assert(sdref->service_name_chosen);

            /* Pick a new name */
            if (!(n = avahi_alternative_service_name(sdref->service_name_chosen))) {
                reg_report_error(sdref, kDNSServiceErr_NoMemory);
                return;
            }
            avahi_free(sdref->service_name_chosen);
            sdref->service_name_chosen = n;

            /* Register the service with that new name */
            if ((ret = reg_create_service(sdref)) < 0) {
                reg_report_error(sdref, map_error(ret));
                return;
            }

            break;
        }

        case AVAHI_ENTRY_GROUP_REGISTERING:
        case AVAHI_ENTRY_GROUP_UNCOMMITED:
            /* Ignore */
            break;

        case AVAHI_ENTRY_GROUP_FAILURE:
            /* Inform the user */
            reg_report_error(sdref, map_error(avahi_client_errno(sdref->client)));
            break;

    }
}

DNSServiceErrorType DNSSD_API DNSServiceRegister (
        DNSServiceRef *ret_sdref,
        DNSServiceFlags flags,
        uint32_t interface,
        const char *name,
        const char *regtype,
        const char *domain,
        const char *host,
        uint16_t port,
        uint16_t txtLen,
        const void *txtRecord,
        DNSServiceRegisterReply callback,
        void *context) {

    DNSServiceErrorType ret = kDNSServiceErr_Unknown;
    int error;
    DNSServiceRef sdref = NULL;
    AvahiStringList *txt = NULL;
    struct type_info type_info;

    AVAHI_WARN_LINKAGE;

    if (!ret_sdref || !regtype)
        return kDNSServiceErr_BadParam;
    *ret_sdref = NULL;

    if (!txtRecord) {
        txtLen = 1;
        txtRecord = "";
    }

    if (interface == kDNSServiceInterfaceIndexLocalOnly || flags) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    if (txtLen > 0)
        if (avahi_string_list_parse(txtRecord, txtLen, &txt) < 0)
            return kDNSServiceErr_Invalid;

    if (type_info_parse(&type_info, regtype) < 0) {
        avahi_string_list_free(txt);
        return kDNSServiceErr_Invalid;
    }

    if (!(sdref = sdref_new())) {
        avahi_string_list_free(txt);
        type_info_free(&type_info);
        return kDNSServiceErr_Unknown;
    }

    sdref->context = context;
    sdref->service_register_callback = callback;

    sdref->type_info = type_info;
    sdref->service_name = avahi_strdup(name);
    sdref->service_domain = domain ? avahi_normalize_name_strdup(domain) : NULL;
    sdref->service_host = host ? avahi_normalize_name_strdup(host) : NULL;
    sdref->service_interface = interface == kDNSServiceInterfaceIndexAny ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface;
    sdref->service_port = ntohs(port);
    sdref->service_txt = txt;

    /* Some OOM checking would be cool here */

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!(sdref->client = avahi_client_new(avahi_simple_poll_get(sdref->simple_poll), 0, reg_client_callback, sdref, &error))) {
        ret =  map_error(error);
        goto finish;
    }

    if (!sdref->service_domain) {
        const char *d;

        if (!(d = avahi_client_get_domain_name(sdref->client))) {
            ret = map_error(avahi_client_errno(sdref->client));
            goto finish;
        }

        if (!(sdref->service_domain = avahi_strdup(d))) {
            ret = kDNSServiceErr_NoMemory;
            goto finish;
        }
    }

    if (!(sdref->entry_group = avahi_entry_group_new(sdref->client, reg_entry_group_callback, sdref))) {
        ret = map_error(avahi_client_errno(sdref->client));
        goto finish;
    }

    if (avahi_client_get_state(sdref->client) == AVAHI_CLIENT_S_RUNNING) {
        const char *n;

        if (sdref->service_name)
            n = sdref->service_name;
        else {
            if (!(n = avahi_client_get_host_name(sdref->client))) {
                ret = map_error(avahi_client_errno(sdref->client));
                goto finish;
            }
        }

        if (!(sdref->service_name_chosen = avahi_strdup(n))) {
            ret = kDNSServiceErr_NoMemory;
            goto finish;
        }


        if ((error = reg_create_service(sdref)) < 0) {
            ret = map_error(error);
            goto finish;
        }
    }

    ret = kDNSServiceErr_NoError;
    *ret_sdref = sdref;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    if (ret != kDNSServiceErr_NoError)
        DNSServiceRefDeallocate(sdref);

    return ret;
}

DNSServiceErrorType DNSSD_API DNSServiceUpdateRecord(
         DNSServiceRef sdref,
         DNSRecordRef rref,
         DNSServiceFlags flags,
         uint16_t rdlen,
         const void *rdata,
         AVAHI_GCC_UNUSED uint32_t ttl) {

    int ret = kDNSServiceErr_Unknown;
    AvahiStringList *txt = NULL;

    AVAHI_WARN_LINKAGE;

    if (!sdref || sdref->n_ref <= 0)
        return kDNSServiceErr_BadParam;

    if (flags || rref) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    if (rdlen > 0)
        if (avahi_string_list_parse(rdata, rdlen, &txt) < 0)
            return kDNSServiceErr_Invalid;

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!avahi_string_list_equal(txt, sdref->service_txt)) {

        avahi_string_list_free(sdref->service_txt);
        sdref->service_txt = txt;

        if (avahi_client_get_state(sdref->client) == AVAHI_CLIENT_S_RUNNING &&
            sdref->entry_group &&
            (avahi_entry_group_get_state(sdref->entry_group) == AVAHI_ENTRY_GROUP_ESTABLISHED ||
            avahi_entry_group_get_state(sdref->entry_group) == AVAHI_ENTRY_GROUP_REGISTERING))

            if (avahi_entry_group_update_service_txt_strlst(
                        sdref->entry_group,
                        sdref->service_interface,
                        AVAHI_PROTO_UNSPEC,
                        0,
                        sdref->service_name_chosen,
                        sdref->type_info.type,
                        sdref->service_domain,
                        sdref->service_txt) < 0) {

                ret = map_error(avahi_client_errno(sdref->client));
                goto finish;
            }

    } else
        avahi_string_list_free(txt);

    ret = kDNSServiceErr_NoError;

finish:
    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    return ret;
}

static void query_resolver_callback(
        AvahiRecordBrowser *r,
        AvahiIfIndex interface,
        AVAHI_GCC_UNUSED AvahiProtocol protocol,
        AvahiBrowserEvent event,
        const char *name,
        uint16_t clazz,
        uint16_t type,
        const void* rdata,
        size_t size,
        AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
        void *userdata) {

    DNSServiceRef sdref = userdata;

    assert(r);
    assert(sdref);
    assert(sdref->n_ref >= 1);

    switch (event) {

    case AVAHI_BROWSER_NEW:
    case AVAHI_BROWSER_REMOVE: {

        DNSServiceFlags qflags = 0;
        if (event == AVAHI_BROWSER_NEW)
            qflags |= kDNSServiceFlagsAdd;

        sdref->query_resolver_callback(sdref, qflags, interface, kDNSServiceErr_NoError, name, type, clazz, size, rdata, 0, sdref->context);
        break;
    }

    case AVAHI_BROWSER_ALL_FOR_NOW:
    case AVAHI_BROWSER_CACHE_EXHAUSTED:
        /* not implemented */
        break;

    case AVAHI_BROWSER_FAILURE:
        sdref->query_resolver_callback(sdref, 0, interface, map_error(avahi_client_errno(sdref->client)), NULL, 0, 0, 0, NULL, 0, sdref->context);
        break;
    }
}

DNSServiceErrorType DNSSD_API DNSServiceQueryRecord (
    DNSServiceRef *ret_sdref,
    DNSServiceFlags flags,
    uint32_t interface,
    const char *fullname,
    uint16_t type,
    uint16_t clazz,
    DNSServiceQueryRecordReply callback,
    void *context) {

    DNSServiceErrorType ret = kDNSServiceErr_Unknown;
    int error;
    DNSServiceRef sdref = NULL;
    AvahiIfIndex ifindex;

    AVAHI_WARN_LINKAGE;

    if (!ret_sdref || !fullname)
        return kDNSServiceErr_BadParam;
    *ret_sdref = NULL;

    if (interface == kDNSServiceInterfaceIndexLocalOnly || flags != 0) {
        AVAHI_WARN_UNSUPPORTED;
        return kDNSServiceErr_Unsupported;
    }

    if (!(sdref = sdref_new()))
        return kDNSServiceErr_Unknown;

    sdref->context = context;
    sdref->query_resolver_callback = callback;

    ASSERT_SUCCESS(pthread_mutex_lock(&sdref->mutex));

    if (!(sdref->client = avahi_client_new(avahi_simple_poll_get(sdref->simple_poll), 0, generic_client_callback, sdref, &error))) {
        ret =  map_error(error);
        goto finish;
    }

    ifindex = interface == kDNSServiceInterfaceIndexAny ? AVAHI_IF_UNSPEC : (AvahiIfIndex) interface;

    if (!(sdref->record_browser = avahi_record_browser_new(sdref->client, ifindex, AVAHI_PROTO_UNSPEC, fullname, clazz, type, 0, query_resolver_callback, sdref))) {
        ret = map_error(avahi_client_errno(sdref->client));
        goto finish;
    }

    ret = kDNSServiceErr_NoError;
    *ret_sdref = sdref;

finish:

    ASSERT_SUCCESS(pthread_mutex_unlock(&sdref->mutex));

    if (ret != kDNSServiceErr_NoError)
        DNSServiceRefDeallocate(sdref);

    return ret;
}
