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
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>

#ifdef HAVE_INOTIFY
#include <sys/inotify.h>
#endif

#ifdef HAVE_KQUEUE
#include <sys/types.h>
#include <sys/event.h>
#include <unistd.h>
#endif

#include <libdaemon/dfork.h>
#include <libdaemon/dsignal.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>

#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/error.h>
#include <avahi-common/alternative.h>
#include <avahi-common/domain.h>

#include <avahi-core/core.h>
#include <avahi-core/publish.h>
#include <avahi-core/dns-srv-rr.h>
#include <avahi-core/log.h>
#include <avahi-core/util.h>

#ifdef ENABLE_CHROOT
#include "chroot.h"
#include "caps.h"
#endif

#include "setproctitle.h"
#include "main.h"
#include "simple-protocol.h"
#include "static-services.h"
#include "static-hosts.h"
#include "ini-file-parser.h"
#include "sd-daemon.h"

#ifdef HAVE_DBUS
#include "dbus-protocol.h"
#endif

AvahiServer *avahi_server = NULL;
AvahiSimplePoll *simple_poll_api = NULL;
static char *argv0 = NULL;
int nss_support = 0;

typedef enum {
    DAEMON_RUN,
    DAEMON_KILL,
    DAEMON_VERSION,
    DAEMON_HELP,
    DAEMON_RELOAD,
    DAEMON_CHECK
} DaemonCommand;

typedef struct {
    AvahiServerConfig server_config;
    DaemonCommand command;
    int daemonize;
    int use_syslog;
    char *config_file;
#ifdef HAVE_DBUS
    int enable_dbus;
    int fail_on_missing_dbus;
    unsigned n_clients_max;
    unsigned n_objects_per_client_max;
    unsigned n_entries_per_entry_group_max;
#endif
    int drop_root;
    int set_rlimits;
#ifdef ENABLE_CHROOT
    int use_chroot;
#endif
    int modify_proc_title;

    int disable_user_service_publishing;
    int publish_resolv_conf;
    char ** publish_dns_servers;
    int debug;

    int rlimit_as_set, rlimit_core_set, rlimit_data_set, rlimit_fsize_set, rlimit_nofile_set, rlimit_stack_set;
    rlim_t rlimit_as, rlimit_core, rlimit_data, rlimit_fsize, rlimit_nofile, rlimit_stack;

#ifdef RLIMIT_NPROC
    int rlimit_nproc_set;
    rlim_t rlimit_nproc;
#endif
} DaemonConfig;

#define RESOLV_CONF "/etc/resolv.conf"
#define BROWSE_DOMAINS_MAX 16

static AvahiSEntryGroup *dns_servers_entry_group = NULL;
static AvahiSEntryGroup *resolv_conf_entry_group = NULL;

static char **resolv_conf_name_servers = NULL;
static char **resolv_conf_search_domains = NULL;

static DaemonConfig config;

static int has_prefix(const char *s, const char *prefix) {
    size_t l;

    l = strlen(prefix);

    return strlen(s) >= l && strncmp(s, prefix, l) == 0;
}

static int load_resolv_conf(void) {
    int ret = -1;
    FILE *f;
    int i = 0, j = 0;

    avahi_strfreev(resolv_conf_name_servers);
    resolv_conf_name_servers = NULL;

    avahi_strfreev(resolv_conf_search_domains);
    resolv_conf_search_domains = NULL;

#ifdef ENABLE_CHROOT
    f = avahi_chroot_helper_get_file(RESOLV_CONF);
#else
    f = fopen(RESOLV_CONF, "r");
#endif

    if (!f) {
        avahi_log_warn("Failed to open "RESOLV_CONF": %s", strerror(errno));
        goto finish;
    }

    resolv_conf_name_servers = avahi_new0(char*, AVAHI_WIDE_AREA_SERVERS_MAX+1);
    resolv_conf_search_domains = avahi_new0(char*, BROWSE_DOMAINS_MAX+1);

    while (!feof(f)) {
        char ln[128];
        char *p;

        if (!(fgets(ln, sizeof(ln), f)))
            break;

        ln[strcspn(ln, "\r\n#")] = 0;
        p = ln + strspn(ln, "\t ");

        if ((has_prefix(p, "nameserver ") || has_prefix(p, "nameserver\t")) && i < AVAHI_WIDE_AREA_SERVERS_MAX) {
            p += 10;
            p += strspn(p, "\t ");
            p[strcspn(p, "\t ")] = 0;
            resolv_conf_name_servers[i++] = avahi_strdup(p);
        }

        if ((has_prefix(p, "search ") || has_prefix(p, "search\t") ||
             has_prefix(p, "domain ") || has_prefix(p, "domain\t"))) {

            p += 6;

            while (j < BROWSE_DOMAINS_MAX) {
                size_t k;

                p += strspn(p, "\t ");
                k = strcspn(p, "\t ");

                if (k > 0) {
                    resolv_conf_search_domains[j++] = avahi_strndup(p, k);
                    p += k;
                }

                if (!*p)
                    break;
            }
        }
    }

    ret = 0;

finish:

    if (ret != 0) {
        avahi_strfreev(resolv_conf_name_servers);
        resolv_conf_name_servers = NULL;

        avahi_strfreev(resolv_conf_search_domains);
        resolv_conf_search_domains = NULL;
    }

    if (f)
        fclose(f);

    return ret;
}

static AvahiSEntryGroup* add_dns_servers(AvahiServer *s, AvahiSEntryGroup* g, char **l) {
    char **p;

    assert(s);
    assert(l);

    if (!g)
        g = avahi_s_entry_group_new(s, NULL, NULL);

    assert(avahi_s_entry_group_is_empty(g));

    for (p = l; *p; p++) {
        AvahiAddress a;

        if (!avahi_address_parse(*p, AVAHI_PROTO_UNSPEC, &a))
            avahi_log_warn("Failed to parse address '%s', ignoring.", *p);
        else
            if (avahi_server_add_dns_server_address(s, g, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, 0, NULL, AVAHI_DNS_SERVER_RESOLVE, &a, 53) < 0) {
                avahi_s_entry_group_free(g);
                avahi_log_error("Failed to add DNS server address: %s", avahi_strerror(avahi_server_errno(s)));
                return NULL;
            }
    }

    avahi_s_entry_group_commit(g);

    return g;
}

static void remove_dns_server_entry_groups(void) {

    if (resolv_conf_entry_group)
        avahi_s_entry_group_reset(resolv_conf_entry_group);

    if (dns_servers_entry_group)
        avahi_s_entry_group_reset(dns_servers_entry_group);
}

static void update_wide_area_servers(void) {
    AvahiAddress a[AVAHI_WIDE_AREA_SERVERS_MAX];
    unsigned n = 0;
    char **p;

    if (!resolv_conf_name_servers) {
        avahi_server_set_wide_area_servers(avahi_server, NULL, 0);
        return;
    }

    for (p = resolv_conf_name_servers; *p && n < AVAHI_WIDE_AREA_SERVERS_MAX; p++) {
        if (!avahi_address_parse(*p, AVAHI_PROTO_UNSPEC, &a[n]))
            avahi_log_warn("Failed to parse address '%s', ignoring.", *p);
        else
            n++;
    }

    avahi_server_set_wide_area_servers(avahi_server, a, n);
}

static AvahiStringList *filter_duplicate_domains(AvahiStringList *l) {
    AvahiStringList *e, *n, *p;

    if (!l)
        return l;

    for (p = l, e = l->next; e; e = n) {
        n = e->next;

        if (avahi_domain_equal((char*) e->text, (char*) l->text)) {
            p->next = e->next;
            avahi_free(e);
        } else
            p = e;
    }

    l->next = filter_duplicate_domains(l->next);
    return l;
}

static void update_browse_domains(void) {
    AvahiStringList *l;
    int n;
    char **p;

    if (!resolv_conf_search_domains) {
        avahi_server_set_browse_domains(avahi_server, NULL);
        return;
    }

    l = avahi_string_list_copy(config.server_config.browse_domains);

    for (p = resolv_conf_search_domains, n = 0; *p && n < BROWSE_DOMAINS_MAX; p++, n++) {
        if (!avahi_is_valid_domain_name(*p))
            avahi_log_warn("'%s' is no valid domain name, ignoring.", *p);
        else
            l = avahi_string_list_add(l, *p);
    }

    l = filter_duplicate_domains(l);

    avahi_server_set_browse_domains(avahi_server, l);
    avahi_string_list_free(l);
}

static void server_callback(AvahiServer *s, AvahiServerState state, void *userdata) {
    DaemonConfig *c = userdata;

    assert(s);
    assert(c);

    /* This function is possibly called before the global variable
     * avahi_server has been set, therefore we do it explicitly */

    avahi_server = s;

#ifdef HAVE_DBUS
    if (c->enable_dbus && state != AVAHI_SERVER_INVALID && state != AVAHI_SERVER_FAILURE)
        dbus_protocol_server_state_changed(state);
#endif

    switch (state) {
        case AVAHI_SERVER_RUNNING:
            avahi_log_info("Server startup complete. Host name is %s. Local service cookie is %u.", avahi_server_get_host_name_fqdn(s), avahi_server_get_local_service_cookie(s));
            sd_notifyf(0, "STATUS=Server startup complete. Host name is %s. Local service cookie is %u.", avahi_server_get_host_name_fqdn(s), avahi_server_get_local_service_cookie(s));
            avahi_set_proc_title(argv0, "%s: running [%s]", argv0, avahi_server_get_host_name_fqdn(s));

            static_service_add_to_server();
            static_hosts_add_to_server();

            remove_dns_server_entry_groups();

            if (c->publish_resolv_conf && resolv_conf_name_servers && resolv_conf_name_servers[0])
                resolv_conf_entry_group = add_dns_servers(s, resolv_conf_entry_group, resolv_conf_name_servers);

            if (c->publish_dns_servers && c->publish_dns_servers[0])
                dns_servers_entry_group = add_dns_servers(s, dns_servers_entry_group, c->publish_dns_servers);

            simple_protocol_restart_queries();
            break;

        case AVAHI_SERVER_COLLISION: {
            char *n;

            static_service_remove_from_server();
            static_hosts_remove_from_server();
            remove_dns_server_entry_groups();

            n = avahi_alternative_host_name(avahi_server_get_host_name(s));

            avahi_log_warn("Host name conflict, retrying with %s", n);
            sd_notifyf(0, "STATUS=Host name conflict, retrying with %s", n);
            avahi_set_proc_title(argv0, "%s: collision [%s]", argv0, n);

            avahi_server_set_host_name(s, n);
            avahi_free(n);

            break;
        }

        case AVAHI_SERVER_FAILURE:

            avahi_log_error("Server error: %s", avahi_strerror(avahi_server_errno(s)));
            sd_notifyf(0, "STATUS=Server error: %s", avahi_strerror(avahi_server_errno(s)));

            avahi_simple_poll_quit(simple_poll_api);
            break;

        case AVAHI_SERVER_REGISTERING:

            sd_notifyf(0, "STATUS=Registering host name %s", avahi_server_get_host_name_fqdn(s));
            avahi_set_proc_title(argv0, "%s: registering [%s]", argv0, avahi_server_get_host_name_fqdn(s));

            static_service_remove_from_server();
            static_hosts_remove_from_server();
            remove_dns_server_entry_groups();

            break;

        case AVAHI_SERVER_INVALID:
            break;

    }
}

static void help(FILE *f) {
    fprintf(f,
            "%s [options]\n"
            "    -h --help          Show this help\n"
            "    -D --daemonize     Daemonize after startup (implies -s)\n"
            "    -s --syslog        Write log messages to syslog(3) instead of STDERR\n"
            "    -k --kill          Kill a running daemon\n"
            "    -r --reload        Request a running daemon to reload static services\n"
            "    -c --check         Return 0 if a daemon is already running\n"
            "    -V --version       Show version\n"
            "    -f --file=FILE     Load the specified configuration file instead of\n"
            "                       "AVAHI_CONFIG_FILE"\n"
            "       --no-rlimits    Don't enforce resource limits\n"
            "       --no-drop-root  Don't drop privileges\n"
#ifdef ENABLE_CHROOT
            "       --no-chroot     Don't chroot()\n"
#endif
            "       --no-proc-title Don't modify process title\n"
            "       --debug         Increase verbosity\n",
            argv0);
}


static int parse_command_line(DaemonConfig *c, int argc, char *argv[]) {
    int o;

    enum {
        OPTION_NO_RLIMITS = 256,
        OPTION_NO_DROP_ROOT,
#ifdef ENABLE_CHROOT
        OPTION_NO_CHROOT,
#endif
        OPTION_NO_PROC_TITLE,
        OPTION_DEBUG
    };

    static const struct option long_options[] = {
        { "help",           no_argument,       NULL, 'h' },
        { "daemonize",      no_argument,       NULL, 'D' },
        { "kill",           no_argument,       NULL, 'k' },
        { "version",        no_argument,       NULL, 'V' },
        { "file",           required_argument, NULL, 'f' },
        { "reload",         no_argument,       NULL, 'r' },
        { "check",          no_argument,       NULL, 'c' },
        { "syslog",         no_argument,       NULL, 's' },
        { "no-rlimits",     no_argument,       NULL, OPTION_NO_RLIMITS },
        { "no-drop-root",   no_argument,       NULL, OPTION_NO_DROP_ROOT },
#ifdef ENABLE_CHROOT
        { "no-chroot",      no_argument,       NULL, OPTION_NO_CHROOT },
#endif
        { "no-proc-title",  no_argument,       NULL, OPTION_NO_PROC_TITLE },
        { "debug",          no_argument,       NULL, OPTION_DEBUG },
        { NULL, 0, NULL, 0 }
    };

    assert(c);

    while ((o = getopt_long(argc, argv, "hDkVf:rcs", long_options, NULL)) >= 0) {

        switch(o) {
            case 's':
                c->use_syslog = 1;
                break;
            case 'h':
                c->command = DAEMON_HELP;
                break;
            case 'D':
                c->daemonize = 1;
                break;
            case 'k':
                c->command = DAEMON_KILL;
                break;
            case 'V':
                c->command = DAEMON_VERSION;
                break;
            case 'f':
                avahi_free(c->config_file);
                c->config_file = avahi_strdup(optarg);
                break;
            case 'r':
                c->command = DAEMON_RELOAD;
                break;
            case 'c':
                c->command = DAEMON_CHECK;
                break;
            case OPTION_NO_RLIMITS:
                c->set_rlimits = 0;
                break;
            case OPTION_NO_DROP_ROOT:
                c->drop_root = 0;
                break;
#ifdef ENABLE_CHROOT
            case OPTION_NO_CHROOT:
                c->use_chroot = 0;
                break;
#endif
            case OPTION_NO_PROC_TITLE:
                c->modify_proc_title = 0;
                break;
            case OPTION_DEBUG:
                c->debug = 1;
#ifdef DAEMON_SET_VERBOSITY_AVAILABLE
                daemon_set_verbosity(LOG_DEBUG);
#endif
                break;
            default:
                return -1;
        }
    }

    if (optind < argc) {
        fprintf(stderr, "Too many arguments\n");
        return -1;
    }

    return 0;
}

static int is_yes(const char *s) {
    assert(s);

    return *s == 'y' || *s == 'Y' || *s == '1' || *s == 't' || *s == 'T';
}

static int parse_unsigned(const char *s, unsigned *u) {
    char *e = NULL;
    unsigned long ul;
    unsigned k;

    errno = 0;
    ul = strtoul(s, &e, 0);

    if (!e || *e || errno != 0)
        return -1;

    k = (unsigned) ul;

    if ((unsigned long) k != ul)
        return -1;

    *u = k;
    return 0;
}

static int parse_usec(const char *s, AvahiUsec *u) {
    char *e = NULL;
    unsigned long long ull;
    AvahiUsec k;

    errno = 0;
    ull = strtoull(s, &e, 0);

    if (!e || *e || errno != 0)
        return -1;

    k = (AvahiUsec) ull;

    if ((unsigned long long) k != ull)
        return -1;

    *u = k;
    return 0;
}

static char *get_machine_id(void) {
    int fd;
    char buf[32];

    fd = open("/etc/machine-id", O_RDONLY|O_CLOEXEC|O_NOCTTY);
    if (fd == -1 && errno == ENOENT)
        fd = open("/var/lib/dbus/machine-id", O_RDONLY|O_CLOEXEC|O_NOCTTY);
    if (fd == -1)
        return NULL;

    /* File is on a filesystem so we never get EINTR or partial reads */
    if (read(fd, buf, sizeof buf) != sizeof buf) {
        close(fd);
        return NULL;
    }
    close(fd);

    /* Contents can be lower, upper and even mixed case so normalize */
    avahi_strdown(buf);

    return avahi_strndup(buf, sizeof buf);
}

static int load_config_file(DaemonConfig *c) {
    int r = -1;
    AvahiIniFile *f;
    AvahiIniFileGroup *g;

    assert(c);

    if (!(f = avahi_ini_file_load(c->config_file ? c->config_file : AVAHI_CONFIG_FILE)))
        goto finish;

    for (g = f->groups; g; g = g->groups_next) {

        if (strcasecmp(g->name, "server") == 0) {
            AvahiIniFilePair *p;

            for (p = g->pairs; p; p = p->pairs_next) {

                if (strcasecmp(p->key, "host-name") == 0) {
                    avahi_free(c->server_config.host_name);
                    c->server_config.host_name = avahi_strdup(p->value);
                } else if (strcasecmp(p->key, "domain-name") == 0) {
                    avahi_free(c->server_config.domain_name);
                    c->server_config.domain_name = avahi_strdup(p->value);
                } else if (strcasecmp(p->key, "browse-domains") == 0) {
                    char **e, **t;

                    e = avahi_split_csv(p->value);

                    for (t = e; *t; t++) {
                        char cleaned[AVAHI_DOMAIN_NAME_MAX];

                        if (!avahi_normalize_name(*t, cleaned, sizeof(cleaned))) {
                            avahi_log_error("Invalid domain name \"%s\" for key \"%s\" in group \"%s\"\n", *t, p->key, g->name);
                            avahi_strfreev(e);
                            goto finish;
                        }

                        c->server_config.browse_domains = avahi_string_list_add(c->server_config.browse_domains, cleaned);
                    }

                    avahi_strfreev(e);

                    c->server_config.browse_domains = filter_duplicate_domains(c->server_config.browse_domains);
                } else if (strcasecmp(p->key, "use-ipv4") == 0)
                    c->server_config.use_ipv4 = is_yes(p->value);
                else if (strcasecmp(p->key, "use-ipv6") == 0)
                    c->server_config.use_ipv6 = is_yes(p->value);
                else if (strcasecmp(p->key, "check-response-ttl") == 0)
                    c->server_config.check_response_ttl = is_yes(p->value);
                else if (strcasecmp(p->key, "allow-point-to-point") == 0)
                    c->server_config.allow_point_to_point = is_yes(p->value);
                else if (strcasecmp(p->key, "use-iff-running") == 0)
                    c->server_config.use_iff_running = is_yes(p->value);
                else if (strcasecmp(p->key, "disallow-other-stacks") == 0)
                    c->server_config.disallow_other_stacks = is_yes(p->value);
                else if (strcasecmp(p->key, "host-name-from-machine-id") == 0) {
                    if (*(p->value) == 'y' || *(p->value) == 'Y') {
                        char *machine_id = get_machine_id();
                        if (machine_id != NULL) {
                            avahi_free(c->server_config.host_name);
                            c->server_config.host_name = machine_id;
                        }
                    }
                }
#ifdef HAVE_DBUS
                else if (strcasecmp(p->key, "enable-dbus") == 0) {

                    if (*(p->value) == 'w' || *(p->value) == 'W') {
                        c->fail_on_missing_dbus = 0;
                        c->enable_dbus = 1;
                    } else if (*(p->value) == 'y' || *(p->value) == 'Y') {
                        c->fail_on_missing_dbus = 1;
                        c->enable_dbus = 1;
                    } else {
                        c->enable_dbus = 0;
                    }
                }
#endif
                else if (strcasecmp(p->key, "allow-interfaces") == 0) {
                    char **e, **t;

                    avahi_string_list_free(c->server_config.allow_interfaces);
                    c->server_config.allow_interfaces = NULL;
                    e = avahi_split_csv(p->value);

                    for (t = e; *t; t++)
                        c->server_config.allow_interfaces = avahi_string_list_add(c->server_config.allow_interfaces, *t);

                    avahi_strfreev(e);
                } else if (strcasecmp(p->key, "deny-interfaces") == 0) {
                    char **e, **t;

                    avahi_string_list_free(c->server_config.deny_interfaces);
                    c->server_config.deny_interfaces = NULL;
                    e = avahi_split_csv(p->value);

                    for (t = e; *t; t++)
                        c->server_config.deny_interfaces = avahi_string_list_add(c->server_config.deny_interfaces, *t);

                    avahi_strfreev(e);
                } else if (strcasecmp(p->key, "ratelimit-interval-usec") == 0) {
                    AvahiUsec k;

                    if (parse_usec(p->value, &k) < 0) {
                        avahi_log_error("Invalid ratelimit-interval-usec setting %s", p->value);
                        goto finish;
                    }

                    c->server_config.ratelimit_interval = k;

                } else if (strcasecmp(p->key, "ratelimit-burst") == 0) {
                    unsigned k;

                    if (parse_unsigned(p->value, &k) < 0) {
                        avahi_log_error("Invalid ratelimit-burst setting %s", p->value);
                        goto finish;
                    }

                    c->server_config.ratelimit_burst = k;

                } else if (strcasecmp(p->key, "cache-entries-max") == 0) {
                    unsigned k;

                    if (parse_unsigned(p->value, &k) < 0) {
                        avahi_log_error("Invalid cache-entries-max setting %s", p->value);
                        goto finish;
                    }

                    c->server_config.n_cache_entries_max = k;
#ifdef HAVE_DBUS
                } else if (strcasecmp(p->key, "clients-max") == 0) {
                    unsigned k;

                    if (parse_unsigned(p->value, &k) < 0) {
                        avahi_log_error("Invalid clients-max setting %s", p->value);
                        goto finish;
                    }

                    c->n_clients_max = k;
                } else if (strcasecmp(p->key, "objects-per-client-max") == 0) {
                    unsigned k;

                    if (parse_unsigned(p->value, &k) < 0) {
                        avahi_log_error("Invalid objects-per-client-max setting %s", p->value);
                        goto finish;
                    }

                    c->n_objects_per_client_max = k;
                } else if (strcasecmp(p->key, "entries-per-entry-group-max") == 0) {
                    unsigned k;

                    if (parse_unsigned(p->value, &k) < 0) {
                        avahi_log_error("Invalid entries-per-entry-group-max setting %s", p->value);
                        goto finish;
                    }

                    c->n_entries_per_entry_group_max = k;
#endif
                } else {
                    avahi_log_error("Invalid configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
                    goto finish;
                }
            }

        } else if (strcasecmp(g->name, "publish") == 0) {
            AvahiIniFilePair *p;

            for (p = g->pairs; p; p = p->pairs_next) {

                if (strcasecmp(p->key, "publish-addresses") == 0)
                    c->server_config.publish_addresses = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-hinfo") == 0)
                    c->server_config.publish_hinfo = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-workstation") == 0)
                    c->server_config.publish_workstation = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-domain") == 0)
                    c->server_config.publish_domain = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-resolv-conf-dns-servers") == 0)
                    c->publish_resolv_conf = is_yes(p->value);
                else if (strcasecmp(p->key, "disable-publishing") == 0)
                    c->server_config.disable_publishing = is_yes(p->value);
                else if (strcasecmp(p->key, "disable-user-service-publishing") == 0)
                    c->disable_user_service_publishing = is_yes(p->value);
                else if (strcasecmp(p->key, "add-service-cookie") == 0)
                    c->server_config.add_service_cookie = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-dns-servers") == 0) {
                    avahi_strfreev(c->publish_dns_servers);
                    c->publish_dns_servers = avahi_split_csv(p->value);
                } else if (strcasecmp(p->key, "publish-a-on-ipv6") == 0)
                    c->server_config.publish_a_on_ipv6 = is_yes(p->value);
                else if (strcasecmp(p->key, "publish-aaaa-on-ipv4") == 0)
                    c->server_config.publish_aaaa_on_ipv4 = is_yes(p->value);
                else {
                    avahi_log_error("Invalid configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
                    goto finish;
                }
            }

        } else if (strcasecmp(g->name, "wide-area") == 0) {
            AvahiIniFilePair *p;

            for (p = g->pairs; p; p = p->pairs_next) {

                if (strcasecmp(p->key, "enable-wide-area") == 0)
                    c->server_config.enable_wide_area = is_yes(p->value);
                else {
                    avahi_log_error("Invalid configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
                    goto finish;
                }
            }

        } else if (strcasecmp(g->name, "reflector") == 0) {
            AvahiIniFilePair *p;

            for (p = g->pairs; p; p = p->pairs_next) {

                if (strcasecmp(p->key, "enable-reflector") == 0)
                    c->server_config.enable_reflector = is_yes(p->value);
                else if (strcasecmp(p->key, "reflect-ipv") == 0)
                    c->server_config.reflect_ipv = is_yes(p->value);
                else if (strcasecmp(p->key, "reflect-filters") == 0) {
                    char **e, **t;

                    avahi_string_list_free(c->server_config.reflect_filters);
                    c->server_config.reflect_filters = NULL;
                    e = avahi_split_csv(p->value);

                    for (t = e; *t; t++)
                        c->server_config.reflect_filters = avahi_string_list_add(c->server_config.reflect_filters, *t);

                    avahi_strfreev(e);
                }
                else {
                    avahi_log_error("Invalid configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
                    goto finish;
                }
            }

        } else if (strcasecmp(g->name, "rlimits") == 0) {
            AvahiIniFilePair *p;

            for (p = g->pairs; p; p = p->pairs_next) {

                if (strcasecmp(p->key, "rlimit-as") == 0) {
                    c->rlimit_as_set = 1;
                    c->rlimit_as = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-core") == 0) {
                    c->rlimit_core_set = 1;
                    c->rlimit_core = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-data") == 0) {
                    c->rlimit_data_set = 1;
                    c->rlimit_data = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-fsize") == 0) {
                    c->rlimit_fsize_set = 1;
                    c->rlimit_fsize = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-nofile") == 0) {
                    c->rlimit_nofile_set = 1;
                    c->rlimit_nofile = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-stack") == 0) {
                    c->rlimit_stack_set = 1;
                    c->rlimit_stack = atoi(p->value);
                } else if (strcasecmp(p->key, "rlimit-nproc") == 0) {
#ifdef RLIMIT_NPROC
                    c->rlimit_nproc_set = 1;
                    c->rlimit_nproc = atoi(p->value);
#else
                    avahi_log_error("Ignoring configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
#endif
                } else {
                    avahi_log_error("Invalid configuration key \"%s\" in group \"%s\"\n", p->key, g->name);
                    goto finish;
                }

            }

        } else {
            avahi_log_error("Invalid configuration file group \"%s\".\n", g->name);
            goto finish;
        }
    }

    r = 0;

finish:

    if (f)
        avahi_ini_file_free(f);

    return r;
}

static void log_function(AvahiLogLevel level, const char *txt) {

    static const int log_level_map[] = {
        LOG_ERR,
        LOG_WARNING,
        LOG_NOTICE,
        LOG_INFO,
        LOG_DEBUG
    };

    assert(level < AVAHI_LOG_LEVEL_MAX);
    assert(txt);

    if (!config.debug && level == AVAHI_LOG_DEBUG)
        return;

    daemon_log(log_level_map[level], "%s", txt);
}

static void dump(const char *text, AVAHI_GCC_UNUSED void* userdata) {
    avahi_log_info("%s", text);
}

#ifdef HAVE_INOTIFY

static int inotify_fd = -1;

static void add_inotify_watches(void) {
    int c = 0;
    /* We ignore the return values, because one or more of these files
     * might not exist and we're OK with that. In addition we never
     * want to remove these watches, hence we keep their ids? */

#ifdef ENABLE_CHROOT
    c = config.use_chroot;
#endif

    inotify_add_watch(inotify_fd, c ? "/services" : AVAHI_SERVICE_DIR, IN_CLOSE_WRITE|IN_DELETE|IN_DELETE_SELF|IN_MOVED_FROM|IN_MOVED_TO|IN_MOVE_SELF
#ifdef IN_ONLYDIR
                      |IN_ONLYDIR
#endif
    );
    inotify_add_watch(inotify_fd, c ? "/" : AVAHI_CONFIG_DIR, IN_CLOSE_WRITE|IN_DELETE|IN_DELETE_SELF|IN_MOVED_FROM|IN_MOVED_TO|IN_MOVE_SELF
#ifdef IN_ONLYDIR
                      |IN_ONLYDIR
#endif
    );
}

#endif

#ifdef HAVE_KQUEUE

#define NUM_WATCHES 2

static int kq = -1;
static int kfds[NUM_WATCHES];
static int num_kfds = 0;

static void add_kqueue_watch(const char *dir);

static void add_kqueue_watches(void) {
    int c = 0;

#ifdef ENABLE_CHROOT
    c = config.use_chroot;
#endif

    add_kqueue_watch(c ? "/" : AVAHI_CONFIG_DIR);
    add_kqueue_watch(c ? "/services" : AVAHI_SERVICE_DIR);
}

static void add_kqueue_watch(const char *dir) {
    int fd;
    struct kevent ev;

    if (kq < 0)
        return;

    if (num_kfds >= NUM_WATCHES)
        return;

    fd = open(dir, O_RDONLY);
    if (fd < 0)
        return;
    EV_SET(&ev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
           NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_RENAME,
           0, 0);
    if (kevent(kq, &ev, 1, NULL, 0, NULL) == -1) {
        close(fd);
        return;
    }

    kfds[num_kfds++] = fd;
}

#endif

static void reload_config(void) {

#ifdef HAVE_INOTIFY
    /* Refresh in case the config dirs have been removed */
    add_inotify_watches();
#endif

#ifdef HAVE_KQUEUE
    add_kqueue_watches();
#endif

#ifdef ENABLE_CHROOT
    static_service_load(config.use_chroot);
    static_hosts_load(config.use_chroot);
#else
    static_service_load(0);
    static_hosts_load(0);
#endif
    static_service_add_to_server();
    static_hosts_add_to_server();

    if (resolv_conf_entry_group)
        avahi_s_entry_group_reset(resolv_conf_entry_group);

    load_resolv_conf();

    update_wide_area_servers();
    update_browse_domains();

    if (config.publish_resolv_conf && resolv_conf_name_servers && resolv_conf_name_servers[0])
        resolv_conf_entry_group = add_dns_servers(avahi_server, resolv_conf_entry_group, resolv_conf_name_servers);
}

#ifdef HAVE_INOTIFY

static void inotify_callback(AvahiWatch *watch, int fd, AVAHI_GCC_UNUSED AvahiWatchEvent event, AVAHI_GCC_UNUSED void *userdata) {
    char* buffer;
    int n = 0;

    assert(fd == inotify_fd);
    assert(watch);

    ioctl(inotify_fd, FIONREAD, &n);
    if (n <= 0)
        n = 128;

    buffer = avahi_malloc(n);
    if (read(inotify_fd, buffer, n) < 0 ) {
        avahi_free(buffer);
        avahi_log_error("Failed to read inotify event: %s", avahi_strerror(errno));
        return;
    }
    avahi_free(buffer);

    avahi_log_info("Files changed, reloading.");
    reload_config();
}

#endif

#ifdef HAVE_KQUEUE

static void kqueue_callback(AvahiWatch *watch, int fd, AVAHI_GCC_UNUSED AvahiWatchEvent event, AVAHI_GCC_UNUSED void *userdata) {
    struct kevent ev;
    struct timespec nullts = { 0, 0 };
    int res;

    assert(fd == kq);
    assert(watch);

    res = kevent(kq, NULL, 0, &ev, 1, &nullts);

    if (res > 0) {
        /* Sleep for a half-second to avoid potential races
         * during install/uninstall. */
        usleep(500000);
        avahi_log_info("Files changed, reloading.");
        reload_config();
    } else {
        avahi_log_error("Failed to read kqueue event: %s", avahi_strerror(errno));
    }
}

#endif

static void signal_callback(AvahiWatch *watch, AVAHI_GCC_UNUSED int fd, AVAHI_GCC_UNUSED AvahiWatchEvent event, AVAHI_GCC_UNUSED void *userdata) {
    int sig;
    const AvahiPoll *poll_api;

    assert(watch);
    assert(simple_poll_api);

    poll_api = avahi_simple_poll_get(simple_poll_api);

    if ((sig = daemon_signal_next()) <= 0) {
        avahi_log_error("daemon_signal_next() failed");
        poll_api->watch_free(watch);
        return;
    }

    switch (sig) {
        case SIGINT:
        case SIGTERM:
            avahi_log_info(
                    "Got %s, quitting.",
                    sig == SIGINT ? "SIGINT" : "SIGTERM");
            avahi_simple_poll_quit(simple_poll_api);
            break;

        case SIGHUP:
            avahi_log_info("Got SIGHUP, reloading.");

            reload_config();
            break;

        case SIGUSR1:
            avahi_log_info("Got SIGUSR1, dumping record data.");
            avahi_server_dump(avahi_server, dump, NULL);
            break;

        default:
            avahi_log_warn("Got spurious signal, ignoring.");
            break;
    }
}

/* Imported from ../avahi-client/nss-check.c */
int avahi_nss_support(void);

static void ignore_signal(int sig)  {
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;

    sigaction(sig, &sa, NULL);
}

static int run_server(DaemonConfig *c) {
    int r = -1;
    int error;
    const AvahiPoll *poll_api = NULL;
    AvahiWatch *sig_watch = NULL;
    int retval_is_sent = 0;
#ifdef HAVE_INOTIFY
    AvahiWatch *inotify_watch = NULL;
#endif
#ifdef HAVE_KQUEUE
    int i;
    AvahiWatch *kqueue_watch = NULL;
#endif

    assert(c);

    ignore_signal(SIGPIPE);

    if (!(nss_support = avahi_nss_support()))
        avahi_log_warn("WARNING: No NSS support for mDNS detected, consider installing nss-mdns!");

    if (!(simple_poll_api = avahi_simple_poll_new())) {
        avahi_log_error("Failed to create main loop object.");
        goto finish;
    }

    poll_api = avahi_simple_poll_get(simple_poll_api);

    if (daemon_signal_init(SIGINT, SIGHUP, SIGTERM, SIGUSR1, 0) < 0) {
        avahi_log_error("Could not register signal handlers (%s).", strerror(errno));
        goto finish;
    }

    if (!(sig_watch = poll_api->watch_new(poll_api, daemon_signal_fd(), AVAHI_WATCH_IN, signal_callback, simple_poll_api))) {
        avahi_log_error( "Failed to create signal watcher");
        goto finish;
    }

    if (simple_protocol_setup(poll_api) < 0)
        goto finish;

#ifdef HAVE_DBUS
    if (c->enable_dbus) {
        if (dbus_protocol_setup(poll_api,
                                config.disable_user_service_publishing,
                                config.n_clients_max,
                                config.n_objects_per_client_max,
                                config.n_entries_per_entry_group_max,
                                !c->fail_on_missing_dbus
#ifdef ENABLE_CHROOT
                                && !config.use_chroot
#endif
            ) < 0) {

            avahi_log_warn("WARNING: Failed to contact D-Bus daemon.");

            if (c->fail_on_missing_dbus)
                goto finish;
        }
    }
#endif

#ifdef ENABLE_CHROOT

    if (config.drop_root && config.use_chroot) {
        if (chroot(AVAHI_CONFIG_DIR) < 0) {
            avahi_log_error("Failed to chroot(): %s", strerror(errno));
            goto finish;
        }

        avahi_log_info("Successfully called chroot().");
        chdir("/");

        if (avahi_caps_drop_all() < 0) {
            avahi_log_error("Failed to drop capabilities.");
            goto finish;
        }
        avahi_log_info("Successfully dropped remaining capabilities.");
    }

#endif

#ifdef HAVE_INOTIFY
    if ((inotify_fd = inotify_init()) < 0)
        avahi_log_warn( "Failed to initialize inotify: %s", strerror(errno));
    else {
        add_inotify_watches();

        if (!(inotify_watch = poll_api->watch_new(poll_api, inotify_fd, AVAHI_WATCH_IN, inotify_callback, NULL))) {
            avahi_log_error( "Failed to create inotify watcher");
            goto finish;
        }
    }
#endif

#ifdef HAVE_KQUEUE
    if ((kq = kqueue()) < 0)
        avahi_log_warn( "Failed to initialize kqueue: %s", strerror(errno));
    else {
        add_kqueue_watches();

        if (!(kqueue_watch = poll_api->watch_new(poll_api, kq, AVAHI_WATCH_IN, kqueue_callback, NULL))) {
            avahi_log_error( "Failed to create kqueue watcher");
            goto finish;
        }
    }
#endif

    load_resolv_conf();
#ifdef ENABLE_CHROOT
    static_service_load(config.use_chroot);
    static_hosts_load(config.use_chroot);
#else
    static_service_load(0);
    static_hosts_load(0);
#endif

    if (!(avahi_server = avahi_server_new(poll_api, &c->server_config, server_callback, c, &error))) {
        avahi_log_error("Failed to create server: %s", avahi_strerror(error));
        goto finish;
    }

    update_wide_area_servers();
    update_browse_domains();

    if (c->daemonize) {
        daemon_retval_send(0);
        retval_is_sent = 1;
    }

    for (;;) {
        if ((r = avahi_simple_poll_iterate(simple_poll_api, -1)) < 0) {

            /* We handle signals through an FD, so let's continue */
            if (errno == EINTR)
                continue;

            avahi_log_error("poll(): %s", strerror(errno));
            goto finish;
        } else if (r > 0)
            /* Quit */
            break;
    }

    r = 0;

finish:

    static_service_remove_from_server();
    static_service_free_all();

    static_hosts_remove_from_server();
    static_hosts_free_all();

    remove_dns_server_entry_groups();

    simple_protocol_shutdown();

#ifdef HAVE_DBUS
    if (c->enable_dbus)
        dbus_protocol_shutdown();
#endif

    if (avahi_server) {
        avahi_server_free(avahi_server);
        avahi_server = NULL;
    }

    daemon_signal_done();

    if (sig_watch)
        poll_api->watch_free(sig_watch);

#ifdef HAVE_INOTIFY
    if (inotify_watch)
        poll_api->watch_free(inotify_watch);
    if (inotify_fd >= 0)
        close(inotify_fd);
#endif

#ifdef HAVE_KQUEUE
    if (kqueue_watch)
        poll_api->watch_free(kqueue_watch);
    if (kq >= 0)
        close(kq);
    for (i = 0; i < num_kfds; i++) {
        if (kfds[i] >= 0)
            close(kfds[i]);
    }
#endif

    if (simple_poll_api) {
        avahi_simple_poll_free(simple_poll_api);
        simple_poll_api = NULL;
    }

    if (!retval_is_sent && c->daemonize)
        daemon_retval_send(1);

    return r;
}

#define set_env(key, value) putenv(avahi_strdup_printf("%s=%s", (key), (value)))

static int drop_root(void) {
    struct passwd *pw;
    struct group * gr;
    int r;

    if (!(pw = getpwnam(AVAHI_USER))) {
        avahi_log_error( "Failed to find user '"AVAHI_USER"'.");
        return -1;
    }

    if (!(gr = getgrnam(AVAHI_GROUP))) {
        avahi_log_error( "Failed to find group '"AVAHI_GROUP"'.");
        return -1;
    }

    avahi_log_info("Found user '"AVAHI_USER"' (UID %lu) and group '"AVAHI_GROUP"' (GID %lu).", (unsigned long) pw->pw_uid, (unsigned long) gr->gr_gid);

    if (initgroups(AVAHI_USER, gr->gr_gid) != 0) {
        avahi_log_error("Failed to change group list: %s", strerror(errno));
        return -1;
    }

#if defined(HAVE_SETRESGID)
    r = setresgid(gr->gr_gid, gr->gr_gid, gr->gr_gid);
#elif defined(HAVE_SETEGID)
    if ((r = setgid(gr->gr_gid)) >= 0)
        r = setegid(gr->gr_gid);
#elif defined(HAVE_SETREGID)
    r = setregid(gr->gr_gid, gr->gr_gid);
#else
#error "No API to drop privileges"
#endif

    if (r < 0) {
        avahi_log_error("Failed to change GID: %s", strerror(errno));
        return -1;
    }

#if defined(HAVE_SETRESUID)
    r = setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid);
#elif defined(HAVE_SETEUID)
    if ((r = setuid(pw->pw_uid)) >= 0)
        r = seteuid(pw->pw_uid);
#elif defined(HAVE_SETREUID)
    r = setreuid(pw->pw_uid, pw->pw_uid);
#else
#error "No API to drop privileges"
#endif

    if (r < 0) {
        avahi_log_error("Failed to change UID: %s", strerror(errno));
        return -1;
    }

    set_env("USER", pw->pw_name);
    set_env("LOGNAME", pw->pw_name);
    set_env("HOME", pw->pw_dir);

    avahi_log_info("Successfully dropped root privileges.");

    return 0;
}

static const char* pid_file_proc(void) {
    return AVAHI_DAEMON_RUNTIME_DIR"/pid";
}

static int make_runtime_dir(void) {
    int r = -1;
    mode_t u;
    int reset_umask = 0;
    struct passwd *pw;
    struct group * gr;
    struct stat st;

    if (!(pw = getpwnam(AVAHI_USER))) {
        avahi_log_error( "Failed to find user '"AVAHI_USER"'.");
        goto fail;
    }

    if (!(gr = getgrnam(AVAHI_GROUP))) {
        avahi_log_error( "Failed to find group '"AVAHI_GROUP"'.");
        goto fail;
    }

    u = umask(0000);
    reset_umask = 1;

    if (mkdir(AVAHI_DAEMON_RUNTIME_DIR, 0755) < 0 && errno != EEXIST) {
        avahi_log_error("mkdir(\""AVAHI_DAEMON_RUNTIME_DIR"\"): %s", strerror(errno));
        goto fail;
    }

    chown(AVAHI_DAEMON_RUNTIME_DIR, pw->pw_uid, gr->gr_gid);

    if (stat(AVAHI_DAEMON_RUNTIME_DIR, &st) < 0) {
        avahi_log_error("stat(): %s\n", strerror(errno));
        goto fail;
    }

    if (!S_ISDIR(st.st_mode) || st.st_uid != pw->pw_uid || st.st_gid != gr->gr_gid) {
        avahi_log_error("Failed to create runtime directory "AVAHI_DAEMON_RUNTIME_DIR".");
        goto fail;
    }

    r = 0;

fail:
    if (reset_umask)
        umask(u);
    return r;
}

static void set_one_rlimit(int resource, rlim_t limit, const char *name) {
    struct rlimit rl;
    rl.rlim_cur = rl.rlim_max = limit;

    if (setrlimit(resource, &rl) < 0)
        avahi_log_warn("setrlimit(%s, {%u, %u}) failed: %s", name, (unsigned) limit, (unsigned) limit, strerror(errno));
}

static void enforce_rlimits(void) {
#ifdef RLIMIT_AS
    if (config.rlimit_as_set)
        set_one_rlimit(RLIMIT_AS, config.rlimit_as, "RLIMIT_AS");
#endif
    if (config.rlimit_core_set)
        set_one_rlimit(RLIMIT_CORE, config.rlimit_core, "RLIMIT_CORE");
    if (config.rlimit_data_set)
        set_one_rlimit(RLIMIT_DATA, config.rlimit_data, "RLIMIT_DATA");
    if (config.rlimit_fsize_set)
        set_one_rlimit(RLIMIT_FSIZE, config.rlimit_fsize, "RLIMIT_FSIZE");
    if (config.rlimit_nofile_set)
        set_one_rlimit(RLIMIT_NOFILE, config.rlimit_nofile, "RLIMIT_NOFILE");
    if (config.rlimit_stack_set)
        set_one_rlimit(RLIMIT_STACK, config.rlimit_stack, "RLIMIT_STACK");
#ifdef RLIMIT_NPROC
    if (config.rlimit_nproc_set)
        set_one_rlimit(RLIMIT_NPROC, config.rlimit_nproc, "RLIMIT_NPROC");
#endif

    /* the sysctl() call from iface-pfroute.c needs locked memory on FreeBSD */
#if defined(RLIMIT_MEMLOCK) && !defined(__FreeBSD__) && !defined(__FreeBSD_kernel__)
    /* We don't need locked memory */
    set_one_rlimit(RLIMIT_MEMLOCK, 0, "RLIMIT_MEMLOCK");
#endif
}

#define RANDOM_DEVICE "/dev/urandom"

static void init_rand_seed(void) {
    int fd;
    unsigned seed = 0;

    /* Try to initialize seed from /dev/urandom, to make it a little
     * less predictable, and to make sure that multiple machines
     * booted at the same time choose different random seeds.  */
    if ((fd = open(RANDOM_DEVICE, O_RDONLY)) >= 0) {
        read(fd, &seed, sizeof(seed));
        close(fd);
    }

    /* If the initialization failed by some reason, we add the time to the seed*/
    seed ^= (unsigned) time(NULL);

    srand(seed);
}

int main(int argc, char *argv[]) {
    int r = 255;
    int wrote_pid_file = 0;

    avahi_set_log_function(log_function);

    init_rand_seed();

    avahi_server_config_init(&config.server_config);
    config.command = DAEMON_RUN;
    config.daemonize = 0;
    config.config_file = NULL;
#ifdef HAVE_DBUS
    config.enable_dbus = 1;
    config.fail_on_missing_dbus = 1;
    config.n_clients_max = 0;
    config.n_objects_per_client_max = 0;
    config.n_entries_per_entry_group_max = 0;
#endif

    config.drop_root = 1;
    config.set_rlimits = 1;
#ifdef ENABLE_CHROOT
    config.use_chroot = 1;
#endif
    config.modify_proc_title = 1;

    config.disable_user_service_publishing = 0;
    config.publish_dns_servers = NULL;
    config.publish_resolv_conf = 0;
    config.use_syslog = 0;
    config.debug = 0;
    config.rlimit_as_set = 0;
    config.rlimit_core_set = 0;
    config.rlimit_data_set = 0;
    config.rlimit_fsize_set = 0;
    config.rlimit_nofile_set = 0;
    config.rlimit_stack_set = 0;
#ifdef RLIMIT_NPROC
    config.rlimit_nproc_set = 0;
#endif

    if ((argv0 = strrchr(argv[0], '/')))
        argv0 = avahi_strdup(argv0 + 1);
    else
        argv0 = avahi_strdup(argv[0]);

    daemon_pid_file_ident = (const char *) argv0;
    daemon_log_ident = (char*) argv0;
    daemon_pid_file_proc = pid_file_proc;

    if (parse_command_line(&config, argc, argv) < 0)
        goto finish;

    if (config.modify_proc_title)
        avahi_init_proc_title(argc, argv);

#ifdef ENABLE_CHROOT
    config.use_chroot = config.use_chroot && config.drop_root;
#endif

    if (config.command == DAEMON_HELP) {
        help(stdout);
        r = 0;
    } else if (config.command == DAEMON_VERSION) {
        printf("%s "PACKAGE_VERSION"\n", argv0);
        r = 0;
    } else if (config.command == DAEMON_KILL) {
        if (daemon_pid_file_kill_wait(SIGTERM, 5) < 0) {
            avahi_log_warn("Failed to kill daemon: %s", strerror(errno));
            goto finish;
        }

        r = 0;

    } else if (config.command == DAEMON_RELOAD) {
        if (daemon_pid_file_kill(SIGHUP) < 0) {
            avahi_log_warn("Failed to kill daemon: %s", strerror(errno));
            goto finish;
        }

        r = 0;

    } else if (config.command == DAEMON_CHECK)
        r = (daemon_pid_file_is_running() >= 0) ? 0 : 1;
    else if (config.command == DAEMON_RUN) {
        pid_t pid;

        if (getuid() != 0 && config.drop_root) {
            avahi_log_error("This program is intended to be run as root.");
            goto finish;
        }

        if ((pid = daemon_pid_file_is_running()) >= 0) {
            avahi_log_error("Daemon already running on PID %u", pid);
            goto finish;
        }

        if (load_config_file(&config) < 0)
            goto finish;

        if (config.daemonize) {
            daemon_retval_init();

            if ((pid = daemon_fork()) < 0)
                goto finish;
            else if (pid != 0) {
                int ret;
                /** Parent **/

                if ((ret = daemon_retval_wait(20)) < 0) {
                    avahi_log_error("Could not receive return value from daemon process.");
                    goto finish;
                }

                r = ret;
                goto finish;
            }

            /* Child */
        }

        if (config.use_syslog || config.daemonize)
            daemon_log_use = DAEMON_LOG_SYSLOG;

        if (sd_listen_fds(0) <= 0)
            if (daemon_close_all(-1) < 0)
                avahi_log_warn("Failed to close all remaining file descriptors: %s", strerror(errno));

        daemon_reset_sigs(-1);
        daemon_unblock_sigs(-1);

        if (make_runtime_dir() < 0)
            goto finish;

        if (config.drop_root) {
#ifdef ENABLE_CHROOT
            if (config.use_chroot)
                if (avahi_caps_reduce() < 0)
                    goto finish;
#endif

            if (drop_root() < 0)
                goto finish;

#ifdef ENABLE_CHROOT
            if (config.use_chroot)
                if (avahi_caps_reduce2() < 0)
                    goto finish;
#endif
        }

        if (daemon_pid_file_create() < 0) {
            avahi_log_error("Failed to create PID file: %s", strerror(errno));

            if (config.daemonize)
                daemon_retval_send(1);
            goto finish;
        } else
            wrote_pid_file = 1;

        if (config.set_rlimits)
            enforce_rlimits();

        chdir("/");

#ifdef ENABLE_CHROOT
        if (config.drop_root && config.use_chroot)
            if (avahi_chroot_helper_start(argv0) < 0) {
                avahi_log_error("failed to start chroot() helper daemon.");
                goto finish;
            }
#endif
        avahi_log_info("%s "PACKAGE_VERSION" starting up.", argv0);
        sd_notifyf(0, "STATUS=%s "PACKAGE_VERSION" starting up.", argv0);
        avahi_set_proc_title(argv0, "%s: starting up", argv0);

        if (run_server(&config) == 0)
            r = 0;

        avahi_log_info("%s "PACKAGE_VERSION" exiting.", argv0);
        sd_notifyf(0, "STATUS=%s "PACKAGE_VERSION" exiting.", argv0);
    }

finish:

    if (config.daemonize)
        daemon_retval_done();

    avahi_server_config_free(&config.server_config);
    avahi_free(config.config_file);
    avahi_strfreev(config.publish_dns_servers);
    avahi_strfreev(resolv_conf_name_servers);
    avahi_strfreev(resolv_conf_search_domains);

    if (wrote_pid_file) {
#ifdef ENABLE_CHROOT
        avahi_chroot_helper_unlink(pid_file_proc());
#else
        daemon_pid_file_remove();
#endif
    }

#ifdef ENABLE_CHROOT
    avahi_chroot_helper_shutdown();
#endif

    avahi_free(argv0);

    return r;
}
