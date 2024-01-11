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

#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <signal.h>
#include <sys/wait.h>
#include <getopt.h>
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>

#include <avahi-common/llist.h>
#include <avahi-common/malloc.h>
#include <avahi-common/address.h>

#include <libdaemon/dfork.h>
#include <libdaemon/dsignal.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dexec.h>

#define BROWSE_DNS_SERVERS "BROWSE-DNS-SERVERS\n"

#define ENV_INTERFACE_DNS_SERVERS  "AVAHI_INTERFACE_DNS_SERVERS"
#define ENV_DNS_SERVERS "AVAHI_DNS_SERVERS"
#define ENV_INTERFACE "AVAHI_INTERFACE"

static enum {
    ACKWAIT,
    BROWSING
} state = ACKWAIT;

static enum {
    DAEMON_RUN,
    DAEMON_KILL,
    DAEMON_REFRESH,
    DAEMON_VERSION,
    DAEMON_HELP,
    DAEMON_CHECK
} command = DAEMON_RUN;

static int quit = 0;
static int daemonize = 0;
static int use_syslog = 0;

#if !HAVE_DECL_ENVIRON
extern char **environ;
#endif

typedef struct DNSServerInfo DNSServerInfo;

struct DNSServerInfo {
    AvahiIfIndex interface;
    AvahiProtocol protocol;
    char *address;
    AVAHI_LLIST_FIELDS(DNSServerInfo, servers);
};

static AVAHI_LLIST_HEAD(DNSServerInfo, servers);

static void server_info_free(DNSServerInfo *i) {
    assert(i);

    avahi_free(i->address);

    AVAHI_LLIST_REMOVE(DNSServerInfo, servers, servers, i);
    avahi_free(i);
}

static DNSServerInfo* get_server_info(AvahiIfIndex interface, AvahiProtocol protocol, const char *address) {
    DNSServerInfo *i;
    assert(address);

    for (i = servers; i; i = i->servers_next)
        if (i->interface == interface &&
            i->protocol == protocol &&
            strcmp(i->address, address) == 0)
            return i;

    return NULL;
}

static DNSServerInfo* new_server_info(AvahiIfIndex interface, AvahiProtocol protocol, const char *address) {
    DNSServerInfo *i;

    assert(address);

    i = avahi_new(DNSServerInfo, 1);
    i->interface = interface;
    i->protocol = protocol;
    i->address = avahi_strdup(address);

    AVAHI_LLIST_PREPEND(DNSServerInfo, servers, servers, i);

    return i;
}

static int set_cloexec(int fd) {
    int n;

    assert(fd >= 0);

    if ((n = fcntl(fd, F_GETFD)) < 0)
        return -1;

    if (n & FD_CLOEXEC)
        return 0;

    return fcntl(fd, F_SETFD, n|FD_CLOEXEC);
}

static int open_socket(void) {
    int fd = -1;
    struct sockaddr_un sa;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        daemon_log(LOG_ERR, "socket(): %s", strerror(errno));
        goto fail;
    }

    if (set_cloexec(fd) < 0) {
        daemon_log(LOG_ERR, "fcntl(): %s", strerror(errno));
        goto fail;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sun_family = AF_UNIX;
    strncpy(sa.sun_path, AVAHI_SOCKET, sizeof(sa.sun_path)-1);
    sa.sun_path[sizeof(sa.sun_path)-1] = 0;

    if (connect(fd, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
        daemon_log(LOG_ERR, "connect(): %s", strerror(errno));
        daemon_log(LOG_INFO, "Failed to connect to the daemon. This probably means that you");
        daemon_log(LOG_INFO, "didn't start avahi-daemon before avahi-dnsconfd.");
        goto fail;
    }

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

static ssize_t loop_write(int fd, const void*data, size_t size) {

    ssize_t ret = 0;
    assert(fd >= 0 && data && size);

    while (size > 0) {
        ssize_t r;

        if ((r = write(fd, data, size)) < 0)
            return r;

        if (r == 0)
            break;

        ret += r;
        data = (const uint8_t*) data + r;
        size -= r;
    }

    return ret;
}

static char *concat_dns_servers(AvahiIfIndex interface) {
    DNSServerInfo *i;
    char *r = NULL;

    for (i = servers; i; i = i->servers_next)
        if (i->interface == interface || interface <= 0) {
            DNSServerInfo *j;
            char *t;

            /* Filter out double entries */
            for (j = servers; j != i; j = j->servers_next)
                if (j->interface == interface || interface <= 0)
                    if (strcmp(i->address, j->address) == 0)
                        break;

            if (j != i)
                continue;

            if (!r)
                t = avahi_strdup(i->address);
            else
                t = avahi_strdup_printf("%s %s", r, i->address);

            avahi_free(r);
            r = t;
        }

    return r;
}

static void set_env(const char *name, const char *value) {
    char **e;
    size_t l;

    assert(name);
    assert(value);

    l = strlen(name);

    for (e = environ; *e; e++) {
        /* Search for the variable */
        if (strlen(*e) < l+1)
            continue;

        if (strncmp(*e, name, l) != 0 || (*e)[l] != '=')
            continue;

        /* We simply free the record, sicne we know that we created it previously */
        avahi_free(*e);
        *e = avahi_strdup_printf("%s=%s", name, value);
        return;
    }

    assert(0);
}

static void run_script(int new, AvahiIfIndex interface, AvahiProtocol protocol, const char *address) {
    char *p;
    int ret;
    char ia[16], pa[16];
    char name[IF_NAMESIZE];

    assert(interface > 0);

    if (!if_indextoname(interface, name))
        return;

    p = concat_dns_servers(interface);
    set_env(ENV_INTERFACE_DNS_SERVERS, p ? p : "");
    avahi_free(p);

    p = concat_dns_servers(-1);
    set_env(ENV_DNS_SERVERS, p ? p : "");
    avahi_free(p);

    set_env(ENV_INTERFACE, name);

    snprintf(ia, sizeof(ia), "%i", (int) interface);
    snprintf(pa, sizeof(pa), "%i", (int) protocol);

    if (daemon_exec("/", &ret, AVAHI_DNSCONF_SCRIPT, AVAHI_DNSCONF_SCRIPT, new ? "+" : "-", address, ia, pa, avahi_proto_to_string(protocol), NULL) < 0)
        daemon_log(LOG_WARNING, "Failed to run script");
    else if (ret != 0)
        daemon_log(LOG_WARNING, "Script returned with non-zero exit code %i", ret);
}

static int new_line(const char *l) {
    assert(l);

    if (state == ACKWAIT) {
        if (*l != '+') {
            daemon_log(LOG_ERR, "Avahi command failed: %s", l);
            return -1;
        }

        daemon_log(LOG_INFO, "Successfully connected to Avahi daemon.");
        state = BROWSING;
    } else {
        AvahiIfIndex interface;
        AvahiProtocol protocol;
        int i_interface, i_protocol, port;
        char a[AVAHI_ADDRESS_STR_MAX];

        assert(state == BROWSING);

        if (*l != '<' && *l != '>') {
            daemon_log(LOG_ERR, "Avahi sent us an invalid browsing line: %s", l);
            return -1;
        }

        if (sscanf(l+1, "%i %i %39s %i", &i_interface, &i_protocol, a, &port) != 4) {
            daemon_log(LOG_ERR, "Failed to parse browsing line: %s", l);
            return -1;
        }

        interface = (AvahiIfIndex) i_interface;
        protocol = (AvahiProtocol) i_protocol;

        if (*l == '>') {
            if (port != 53)
                daemon_log(LOG_WARNING, "DNS server with port address != 53 found, ignoring");
            else {
                daemon_log(LOG_INFO, "New DNS Server %s (interface: %i.%s)", a, interface, avahi_proto_to_string(protocol));
                new_server_info(interface, protocol, a);
                run_script(1, interface, protocol, a);
            }
        } else {
            DNSServerInfo *i;

            if (port == 53)
                if ((i = get_server_info(interface, protocol, a))) {
                    daemon_log(LOG_INFO, "DNS Server %s removed (interface: %i.%s)", a, interface, avahi_proto_to_string(protocol));
                    server_info_free(i);
                    run_script(0, interface, protocol, a);
                }
        }

    }

    return 0;
}

static int do_connect(void) {
    int fd = -1;

    if ((fd = open_socket()) < 0)
        goto fail;

    if (loop_write(fd, BROWSE_DNS_SERVERS, sizeof(BROWSE_DNS_SERVERS)-1) < 0) {
        daemon_log(LOG_ERR, "write(): %s", strerror(errno));
        goto fail;
    }

    state = ACKWAIT;
    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

static void free_dns_server_info_list(void) {
    while (servers) {
        AvahiIfIndex interface = servers->interface;
        AvahiProtocol protocol = servers->protocol;
        char *address = avahi_strdup(servers->address);
        server_info_free(servers);

        run_script(0, interface, protocol, address);
        avahi_free(address);
    }
}

static void help(FILE *f, const char *argv0) {
    fprintf(f,
            "%s [options]\n"
            "    -h --help        Show this help\n"
            "    -D --daemonize   Daemonize after startup\n"
            "    -s --syslog      Write log messages to syslog(3) instead of STDERR\n"
            "    -k --kill        Kill a running daemon\n"
            "    -r --refresh     Request a running daemon to refresh DNS server data\n"
            "    -c --check       Return 0 if a daemon is already running\n"
            "    -V --version     Show version\n",
            argv0);
}

static int parse_command_line(int argc, char *argv[]) {
    int c;

    static const struct option long_options[] = {
        { "help",      no_argument,       NULL, 'h' },
        { "daemonize", no_argument,       NULL, 'D' },
        { "syslog",    no_argument,       NULL, 's' },
        { "kill",      no_argument,       NULL, 'k' },
        { "version",   no_argument,       NULL, 'V' },
        { "refresh",   no_argument,       NULL, 'r' },
        { "check",     no_argument,       NULL, 'c' },
        { NULL, 0, NULL, 0 }
    };

    while ((c = getopt_long(argc, argv, "hDkVrcs", long_options, NULL)) >= 0) {

        switch(c) {
            case 'h':
                command = DAEMON_HELP;
                break;
            case 'D':
                daemonize = 1;
                break;
            case 's':
                use_syslog = 1;
                break;
            case 'k':
                command = DAEMON_KILL;
                break;
            case 'V':
                command = DAEMON_VERSION;
                break;
            case 'r':
                command = DAEMON_REFRESH;
                break;
            case 'c':
                command = DAEMON_CHECK;
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

static int run_daemon(void) {
    int fd = -1, ret = -1;
    char buf[1024];
    size_t buflen = 0;

    AVAHI_LLIST_HEAD_INIT(DNSServerInfo, servers);

    daemon_signal_init(SIGINT, SIGTERM, SIGCHLD, SIGHUP, 0);

    /* Allocate some memory for our environment variables */
    putenv(avahi_strdup(ENV_INTERFACE"="));
    putenv(avahi_strdup(ENV_DNS_SERVERS"="));
    putenv(avahi_strdup(ENV_INTERFACE_DNS_SERVERS"="));

    if ((fd = do_connect()) < 0)
        goto finish;

    if (daemonize)
        daemon_retval_send(0);

    ret = 0;

    while (!quit) {
        fd_set rfds, wfds;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);

        FD_SET(fd, &rfds);
        FD_SET(daemon_signal_fd(), &rfds);

        for (;;) {
            if (select(fd+1, &rfds, NULL, NULL, NULL) < 0) {
                if (errno == EINTR)
                    continue;

                daemon_log(LOG_ERR, "select(): %s", strerror(errno));
                goto finish;
            }

            break;
        }

        if (FD_ISSET(daemon_signal_fd(), &rfds)) {

            int sig;

            if ((sig = daemon_signal_next()) <= 0) {
                daemon_log(LOG_ERR, "daemon_signal_next() failed");
                goto finish;
            }

            switch(sig) {
                case SIGINT:
                case SIGTERM:
                    daemon_log(LOG_INFO, "Got %s, quitting.", sig == SIGINT ? "SIGINT" : "SIGTERM");
                    ret = 0;
                    goto finish;

                case SIGCHLD:
                    waitpid(-1, NULL, WNOHANG);
                    break;

                case SIGHUP:
                    daemon_log(LOG_INFO, "Refreshing DNS Server list");

                    close(fd);
                    free_dns_server_info_list();

                    if ((fd = do_connect()) < 0)
                        goto finish;

                    break;
            }

        } else if (FD_ISSET(fd, &rfds)) {
            ssize_t r;
            char *n;

            if ((r = read(fd, buf, sizeof(buf) - buflen - 1)) <= 0) {
                daemon_log(LOG_ERR, "read(): %s", r < 0 ? strerror(errno) : "EOF");
                goto finish;
            }

            buflen += r;
            assert(buflen <= sizeof(buf)-1);

            while ((n = memchr(buf, '\n', buflen))) {
                *(n++) = 0;

                if (new_line(buf) < 0)
                    goto finish;

                buflen -= (n - buf);
                memmove(buf, n, buflen);
            }

            if (buflen >= sizeof(buf)-1) {
                /* The incoming line is horribly long */
                buf[sizeof(buf)-1] = 0;

                if (new_line(buf) < 0)
                    goto finish;

                buflen = 0;
            }
        }
    }

finish:

    free_dns_server_info_list();

    if (fd >= 0)
        close(fd);

    daemon_signal_done();

    if (ret != 0 && daemonize)
        daemon_retval_send(1);

    return ret;
}

static const char* pid_file_proc(void) {
    return AVAHI_RUNTIME_DIR"/avahi-dnsconfd.pid";
}

int main(int argc, char *argv[]) {
    char *argv0;
    int r = 1;
    int wrote_pid_file = 0;

    if ((argv0 = strrchr(argv[0], '/')))
        argv0++;
    else
        argv0 = argv[0];

    daemon_pid_file_ident = daemon_log_ident = argv0;
    daemon_pid_file_proc = pid_file_proc;

    if (parse_command_line(argc, argv) < 0)
        goto finish;

    if (command == DAEMON_RUN) {
        pid_t pid;

        if (getuid() != 0) {
            daemon_log(LOG_ERR, "This program is intended to be run as root.");
            goto finish;
        }

        if ((pid = daemon_pid_file_is_running()) >= 0) {
            daemon_log(LOG_ERR, "Daemon already running on PID %u", pid);
            goto finish;
        }

        if (daemonize) {
            daemon_retval_init();

            if ((pid = daemon_fork()) < 0)
                goto finish;
            else if (pid != 0) {
                int ret;
                /** Parent **/

                if ((ret = daemon_retval_wait(20)) < 0) {
                    daemon_log(LOG_ERR, "Could not receive return value from daemon process.");
                    goto finish;
                }

                r = ret;
                goto finish;
            }

            /* Child */
        }

        if (use_syslog || daemonize)
            daemon_log_use = DAEMON_LOG_SYSLOG;

        chdir("/");

        if (daemon_pid_file_create() < 0) {
            daemon_log(LOG_ERR, "Failed to create PID file: %s", strerror(errno));

            if (daemonize)
                daemon_retval_send(1);
            goto finish;
        } else
            wrote_pid_file = 1;

        if (run_daemon() < 0)
            goto finish;

        r = 0;
    } else if (command == DAEMON_HELP) {
        help(stdout, argv0);

        r = 0;
    } else if (command == DAEMON_VERSION) {
        printf("%s "PACKAGE_VERSION"\n", argv0);

        r = 0;
    } else if (command == DAEMON_KILL) {
        if (daemon_pid_file_kill_wait(SIGTERM, 5) < 0) {
            daemon_log(LOG_WARNING, "Failed to kill daemon: %s", strerror(errno));
            goto finish;
        }

        r = 0;
    } else if (command == DAEMON_REFRESH) {
        if (daemon_pid_file_kill(SIGHUP) < 0) {
            daemon_log(LOG_WARNING, "Failed to kill daemon: %s", strerror(errno));
            goto finish;
        }

        r = 0;
    } else if (command == DAEMON_CHECK)
        r = (daemon_pid_file_is_running() >= 0) ? 0 : 1;


finish:

    if (daemonize)
        daemon_retval_done();

    if (wrote_pid_file)
        daemon_pid_file_remove();

    return r;
}
