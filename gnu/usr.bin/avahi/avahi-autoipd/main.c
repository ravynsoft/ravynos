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

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#include <sys/socket.h>
#include <sys/wait.h>
#ifdef __FreeBSD__
#include <sys/sysctl.h>
#endif

#ifdef __linux__
#include <netpacket/packet.h>
#endif
#include <net/ethernet.h>
#include <net/if.h>
#ifdef __FreeBSD__
#include <net/if_dl.h>
#include <net/route.h>
#endif
#include <arpa/inet.h>

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#include <grp.h>
#include <poll.h>
#include <pwd.h>
#include <unistd.h>

#ifndef __linux__
#include <pcap.h>
#endif

#include <avahi-common/malloc.h>
#include <avahi-common/timeval.h>
#include <avahi-daemon/setproctitle.h>

#include <libdaemon/dfork.h>
#include <libdaemon/dsignal.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>
#include <libdaemon/dexec.h>

#include "main.h"
#include "iface.h"

/* An implementation of RFC 3927 */

/* Constants from the RFC */
#define PROBE_WAIT 1
#define PROBE_NUM 3
#define PROBE_MIN 1
#define PROBE_MAX 2
#define ANNOUNCE_WAIT 2
#define ANNOUNCE_NUM 2
#define ANNOUNCE_INTERVAL 2
#define MAX_CONFLICTS 10
#define RATE_LIMIT_INTERVAL 60
#define DEFEND_INTERVAL 10

#define IPV4LL_NETWORK 0xA9FE0000L
#define IPV4LL_NETMASK 0xFFFF0000L
#define IPV4LL_HOSTMASK 0x0000FFFFL
#define IPV4LL_BROADCAST 0xA9FEFFFFL

#define ETHER_ADDRLEN 6
#define ETHER_HDR_SIZE (2+2*ETHER_ADDRLEN)
#define ARP_PACKET_SIZE (8+4+4+2*ETHER_ADDRLEN)

typedef enum ArpOperation {
    ARP_REQUEST = 1,
    ARP_RESPONSE = 2
} ArpOperation;

typedef struct ArpPacketInfo {
    ArpOperation operation;

    uint32_t sender_ip_address, target_ip_address;
    uint8_t sender_hw_address[ETHER_ADDRLEN], target_hw_address[ETHER_ADDRLEN];
} ArpPacketInfo;

typedef struct ArpPacket {
    uint8_t *ether_header;
    uint8_t *ether_payload;
} ArpPacket;

static State state = STATE_START;
static int n_iteration = 0;
static int n_conflict = 0;

static char *interface_name = NULL;
static char *pid_file_name = NULL;
static uint32_t start_address = 0;
static char *argv0 = NULL;
static int daemonize = 0;
static int wait_for_address = 0;
static int use_syslog = 0;
static int debug = 0;
static int modify_proc_title = 1;
static int force_bind = 0;
#ifdef HAVE_CHROOT
static int no_chroot = 0;
#endif
static int no_drop_root = 0;
static int wrote_pid_file = 0;
static char *action_script = NULL;

static enum {
    DAEMON_RUN,
    DAEMON_KILL,
    DAEMON_REFRESH,
    DAEMON_VERSION,
    DAEMON_HELP,
    DAEMON_CHECK
} command = DAEMON_RUN;

typedef enum CalloutEvent {
    CALLOUT_BIND,
    CALLOUT_CONFLICT,
    CALLOUT_UNBIND,
    CALLOUT_STOP,
    CALLOUT_MAX
} CalloutEvent;

static const char * const callout_event_table[CALLOUT_MAX] = {
    [CALLOUT_BIND] = "BIND",
    [CALLOUT_CONFLICT] = "CONFLICT",
    [CALLOUT_UNBIND] = "UNBIND",
    [CALLOUT_STOP] = "STOP"
};

typedef struct CalloutEventInfo {
    CalloutEvent event;
    uint32_t address;
    int ifindex;
} CalloutEventInfo;

#define RANDOM_DEVICE "/dev/urandom"

#define DEBUG(x)                                \
    do {                                        \
        if (debug) {                            \
            x;                                  \
        }                                       \
    } while (0)

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

    /* If the initialization failed by some reason, we add the time to the seed */
    seed ^= (unsigned) time(NULL);

    srand(seed);
}

static uint32_t pick_addr(uint32_t old_addr) {
    uint32_t addr;

    do {
        unsigned r = (unsigned) rand();

        /* Reduce to 16 bits */
        while (r > 0xFFFF)
            r = (r >> 16) ^ (r & 0xFFFF);

        addr = htonl(IPV4LL_NETWORK | (uint32_t) r);

    } while (addr == old_addr || !is_ll_address(addr));

    return addr;
}

static int load_address(const char *fn, uint32_t *addr) {
    FILE *f;
    unsigned a, b, c, d;

    assert(fn);
    assert(addr);

    if (!(f = fopen(fn, "r"))) {

        if (errno == ENOENT) {
            *addr = 0;
            return 0;
        }

        daemon_log(LOG_ERR, "fopen() failed: %s", strerror(errno));
        goto fail;
    }

    if (fscanf(f, "%u.%u.%u.%u\n", &a, &b, &c, &d) != 4) {
        daemon_log(LOG_ERR, "Parse failure");
        goto fail;
    }

    fclose(f);

    *addr = htonl((a << 24) | (b << 16) | (c << 8) | d);
    return 0;

fail:
    if (f)
        fclose(f);

    return -1;
}

static int save_address(const char *fn, uint32_t addr) {
    FILE *f;
    char buf[32];
    mode_t u;

    assert(fn);

    u = umask(0033);
    if (!(f = fopen(fn, "w"))) {
        daemon_log(LOG_ERR, "fopen() failed: %s", strerror(errno));
        goto fail;
    }
    umask(u);

    fprintf(f, "%s\n", inet_ntop(AF_INET, &addr, buf, sizeof (buf)));
    fclose(f);

    return 0;

fail:
    if (f)
        fclose(f);

    umask(u);

    return -1;
}

/*
 * Allocate a buffer with two pointers in front, one of which is
 * guaranteed to point ETHER_HDR_SIZE bytes into it.
 */
static ArpPacket* packet_new(size_t packet_len) {
    ArpPacket *p;
    uint8_t *b;

    assert(packet_len > 0);

#ifdef __linux__
    b = avahi_new0(uint8_t, sizeof(struct ArpPacket) + packet_len);
    p = (ArpPacket*) b;
    p->ether_header = NULL;
    p->ether_payload = b + sizeof(struct ArpPacket);

#else
    b = avahi_new0(uint8_t, sizeof(struct ArpPacket) + ETHER_HDR_SIZE + packet_len);
    p = (ArpPacket*) b;
    p->ether_header = b + sizeof(struct ArpPacket);
    p->ether_payload = b + sizeof(struct ArpPacket) + ETHER_HDR_SIZE;
#endif

    return p;
}

static ArpPacket* packet_new_with_info(const ArpPacketInfo *info, size_t *packet_len) {
    ArpPacket *p = NULL;
    uint8_t *r;

    assert(info);
    assert(info->operation == ARP_REQUEST || info->operation == ARP_RESPONSE);
    assert(packet_len != NULL);

    *packet_len = ARP_PACKET_SIZE;
    p = packet_new(*packet_len);
    r = p->ether_payload;

    r[1] = 1; /* HTYPE */
    r[2] = 8; /* PTYPE */
    r[4] = ETHER_ADDRLEN; /* HLEN */
    r[5] = 4; /* PLEN */
    r[7] = (uint8_t) info->operation;

    memcpy(r+8, info->sender_hw_address, ETHER_ADDRLEN);
    memcpy(r+14, &info->sender_ip_address, 4);
    memcpy(r+18, info->target_hw_address, ETHER_ADDRLEN);
    memcpy(r+24, &info->target_ip_address, 4);

    return p;
}

static ArpPacket *packet_new_probe(uint32_t ip_address, const uint8_t*hw_address, size_t *packet_len) {
    ArpPacketInfo info;

    memset(&info, 0, sizeof(info));
    info.operation = ARP_REQUEST;
    memcpy(info.sender_hw_address, hw_address, ETHER_ADDRLEN);
    info.target_ip_address = ip_address;

    return packet_new_with_info(&info, packet_len);
}

static ArpPacket *packet_new_announcement(uint32_t ip_address, const uint8_t* hw_address, size_t *packet_len) {
    ArpPacketInfo info;

    memset(&info, 0, sizeof(info));
    info.operation = ARP_REQUEST;
    memcpy(info.sender_hw_address, hw_address, ETHER_ADDRLEN);
    info.target_ip_address = ip_address;
    info.sender_ip_address = ip_address;

    return packet_new_with_info(&info, packet_len);
}

static int packet_parse(const ArpPacket *packet, size_t packet_len, ArpPacketInfo *info) {
    const uint8_t *p;

    assert(packet);
    p = (uint8_t *)packet->ether_payload;
    assert(p);

    if (packet_len < ARP_PACKET_SIZE)
        return -1;

    /* Check HTYPE and PTYPE */
    if (p[0] != 0 || p[1] != 1 || p[2] != 8 || p[3] != 0)
        return -1;

    /* Check HLEN, PLEN, OPERATION */
    if (p[4] != ETHER_ADDRLEN || p[5] != 4 || p[6] != 0 || (p[7] != 1 && p[7] != 2))
        return -1;

    info->operation = p[7];
    memcpy(info->sender_hw_address, p+8, ETHER_ADDRLEN);
    memcpy(&info->sender_ip_address, p+14, 4);
    memcpy(info->target_hw_address, p+18, ETHER_ADDRLEN);
    memcpy(&info->target_ip_address, p+24, 4);

    return 0;
}

static void set_state(State st, int reset_counter, uint32_t address) {
    static const char* const state_table[] = {
        [STATE_START] = "START",
        [STATE_WAITING_PROBE] = "WAITING_PROBE",
        [STATE_PROBING] = "PROBING",
        [STATE_WAITING_ANNOUNCE] = "WAITING_ANNOUNCE",
        [STATE_ANNOUNCING] = "ANNOUNCING",
        [STATE_RUNNING] = "RUNNING",
        [STATE_SLEEPING] = "SLEEPING"
    };
    char buf[64];

    assert(st < STATE_MAX);

    if (st == state && !reset_counter) {
        n_iteration++;
        DEBUG(daemon_log(LOG_DEBUG, "State iteration %s-%i", state_table[state], n_iteration));
    } else {
        DEBUG(daemon_log(LOG_DEBUG, "State transition %s-%i -> %s-0", state_table[state], n_iteration, state_table[st]));
        state = st;
        n_iteration = 0;
    }

    if (state == STATE_SLEEPING)
        avahi_set_proc_title(argv0, "%s: [%s] sleeping", argv0, interface_name);
    else if (state == STATE_ANNOUNCING)
        avahi_set_proc_title(argv0, "%s: [%s] announcing %s", argv0, interface_name, inet_ntop(AF_INET, &address, buf, sizeof(buf)));
    else if (state == STATE_RUNNING)
        avahi_set_proc_title(argv0, "%s: [%s] bound %s", argv0, interface_name, inet_ntop(AF_INET, &address, buf, sizeof(buf)));
    else
        avahi_set_proc_title(argv0, "%s: [%s] probing %s", argv0, interface_name, inet_ntop(AF_INET, &address, buf, sizeof(buf)));
}

static int interface_up(int iface) {
    int fd = -1;
    struct ifreq ifreq;

    if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        daemon_log(LOG_ERR, "socket() failed: %s", strerror(errno));
        goto fail;
    }

    memset(&ifreq, 0, sizeof(ifreq));
    if (!if_indextoname(iface, ifreq.ifr_name)) {
        daemon_log(LOG_ERR, "if_indextoname() failed: %s", strerror(errno));
        goto fail;
    }

    if (ioctl(fd, SIOCGIFFLAGS, &ifreq) < 0) {
        daemon_log(LOG_ERR, "SIOCGIFFLAGS failed: %s", strerror(errno));
        goto fail;
    }

    ifreq.ifr_flags |= IFF_UP;

    if (ioctl(fd, SIOCSIFFLAGS, &ifreq) < 0) {
        daemon_log(LOG_ERR, "SIOCSIFFLAGS failed: %s", strerror(errno));
        goto fail;
    }

    close(fd);

    return 0;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

#ifdef __linux__

/* Linux 'packet socket' specific implementation */

static int open_socket(int iface, uint8_t *hw_address) {
    int fd = -1;
    struct sockaddr_ll sa;
    socklen_t sa_len;

    if (interface_up(iface) < 0)
        goto fail;

    if ((fd = socket(PF_PACKET, SOCK_DGRAM, 0)) < 0) {
        daemon_log(LOG_ERR, "socket() failed: %s", strerror(errno));
        goto fail;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = iface;

    if (bind(fd, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
        daemon_log(LOG_ERR, "bind() failed: %s", strerror(errno));
        goto fail;
    }

    sa_len = sizeof(sa);
    if (getsockname(fd, (struct sockaddr*) &sa, &sa_len) < 0) {
        daemon_log(LOG_ERR, "getsockname() failed: %s", strerror(errno));
        goto fail;
    }

    if (sa.sll_halen != ETHER_ADDRLEN) {
        daemon_log(LOG_ERR, "getsockname() returned invalid hardware address.");
        goto fail;
    }

    memcpy(hw_address, sa.sll_addr, ETHER_ADDRLEN);

    return fd;

fail:
    if (fd >= 0)
        close(fd);

    return -1;
}

static int send_packet(int fd, int iface, ArpPacket *packet, size_t packet_len) {
    struct sockaddr_ll sa;

    assert(fd >= 0);
    assert(packet);
    assert(packet_len > 0);

    memset(&sa, 0, sizeof(sa));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = iface;
    sa.sll_halen = ETHER_ADDRLEN;
    memset(sa.sll_addr, 0xFF, ETHER_ADDRLEN);

    if (sendto(fd, packet->ether_payload, packet_len, 0, (struct sockaddr*) &sa, sizeof(sa)) < 0) {
        daemon_log(LOG_ERR, "sendto() failed: %s", strerror(errno));
        return -1;
    }

    return 0;
}

static int recv_packet(int fd, ArpPacket **packet, size_t *packet_len) {
    int s;
    struct sockaddr_ll sa;
    socklen_t sa_len;
    ssize_t r;

    assert(fd >= 0);
    assert(packet);
    assert(packet_len);

    *packet = NULL;

    if (ioctl(fd, FIONREAD, &s) < 0) {
        daemon_log(LOG_ERR, "FIONREAD failed: %s", strerror(errno));
        goto fail;
    }

    if (s <= 0)
        s = 4096;

    *packet = packet_new(s);

    sa_len = sizeof(sa);
    if ((r = recvfrom(fd, (*packet)->ether_payload, s, 0, (struct sockaddr*) &sa, &sa_len)) < 0) {
        daemon_log(LOG_ERR, "recvfrom() failed: %s", strerror(errno));
        goto fail;
    }

    *packet_len = (size_t) r;

    return 0;

fail:
    if (*packet) {
        avahi_free(*packet);
        *packet = NULL;
    }

    return -1;
}

static void close_socket(int fd) {
    close(fd);
}

#else /* !__linux__ */
/* PCAP-based implementation */

static pcap_t *__pp;
static char __pcap_errbuf[PCAP_ERRBUF_SIZE];
static uint8_t __lladdr[ETHER_ADDRLEN];

#ifndef elementsof
#define elementsof(array)       (sizeof(array)/sizeof(array[0]))
#endif

static int __get_ether_addr(int ifindex, u_char *lladdr) {
    int mib[6];
    char *buf;
    struct if_msghdr *ifm;
    char *lim;
    char *next;
    struct sockaddr_dl *sdl;
    size_t len;

    mib[0] = CTL_NET;
    mib[1] = PF_ROUTE;
    mib[2] = 0;
    mib[3] = 0;
    mib[4] = NET_RT_IFLIST;
    mib[5] = ifindex;

    if (sysctl(mib, elementsof(mib), NULL, &len, NULL, 0) != 0) {
        daemon_log(LOG_ERR, "sysctl(NET_RT_IFLIST): %s",
                   strerror(errno));
        return -1;
    }

    buf = avahi_malloc(len);
    if (sysctl(mib, elementsof(mib), buf, &len, NULL, 0) != 0) {
        daemon_log(LOG_ERR, "sysctl(NET_RT_IFLIST): %s",
                   strerror(errno));
        free(buf);
        return -1;
    }

    lim = buf + len;
    for (next = buf; next < lim; next += ifm->ifm_msglen) {
        ifm = (struct if_msghdr *)next;
        if (ifm->ifm_type == RTM_IFINFO) {
            sdl = (struct sockaddr_dl *)(ifm + 1);
            memcpy(lladdr, LLADDR(sdl), ETHER_ADDRLEN);
        }
    }
    avahi_free(buf);

    return 0;
}

#define PCAP_TIMEOUT 500 /* 0.5s */

static int open_socket(int iface, uint8_t *hw_address) {
    struct bpf_program bpf;
    char *filter;
    char ifname[IFNAMSIZ];
    pcap_t *pp;
    int err;
    int fd;

    assert(__pp == NULL);

    if (interface_up(iface) < 0)
        return -1;

    if (__get_ether_addr(iface, __lladdr) == -1)
        return -1;

    if (if_indextoname(iface, ifname) == NULL)
        return -1;

    /*
     * Using a timeout for BPF is fairly portable across BSDs. On most
     * modern versions, using the timeout/nonblock/poll method results in
     * fairly sane behavior, with the timeout only coming into play during
     * the next_ex() call itself (so, for us, that's only when there's
     * data). On older versions, it may result in a PCAP_TIMEOUT busy-wait
     * on some versions, though, as the poll() may terminate at the
     * PCAP_TIMEOUT instead of the poll() timeout.
     */
    pp = pcap_open_live(ifname, 1500, 0, PCAP_TIMEOUT, __pcap_errbuf);
    if (pp == NULL) {
        return (-1);
    }
    err = pcap_set_datalink(pp, DLT_EN10MB);
    if (err == -1) {
        daemon_log(LOG_ERR, "pcap_set_datalink: %s", pcap_geterr(pp));
        pcap_close(pp);
        return (-1);
    }
    err = pcap_setdirection(pp, PCAP_D_IN);
    if (err == -1) {
        daemon_log(LOG_ERR, "pcap_setdirection: %s", pcap_geterr(pp));
        pcap_close(pp);
        return (-1);
    }

    fd = pcap_get_selectable_fd(pp);
    if (fd == -1) {
        pcap_close(pp);
        return (-1);
    }

    /*
     * Using setnonblock is a portability stop-gap. Using the timeout in
     * combination with setnonblock will ensure on most BSDs that the
     * next_ex call returns in a timely fashion.
     */
    err = pcap_setnonblock(pp, 1, __pcap_errbuf);
    if (err == -1) {
        pcap_close(pp);
        return (-1);
    }

    filter = avahi_strdup_printf("arp and (ether dst ff:ff:ff:ff:ff:ff or "
                                 "%02x:%02x:%02x:%02x:%02x:%02x)",
                                 __lladdr[0], __lladdr[1],
                                 __lladdr[2], __lladdr[3],
                                 __lladdr[4], __lladdr[5]);
    DEBUG(daemon_log(LOG_DEBUG, "Using pcap filter '%s'", filter));

    err = pcap_compile(pp, &bpf, filter, 1, 0);
    avahi_free(filter);
    if (err == -1) {
        daemon_log(LOG_ERR, "pcap_compile: %s", pcap_geterr(pp));
        pcap_close(pp);
        return (-1);
    }
    err = pcap_setfilter(pp, &bpf);
    if (err == -1) {
        daemon_log(LOG_ERR, "pcap_setfilter: %s", pcap_geterr(pp));
        pcap_close(pp);
        return (-1);
    }
    pcap_freecode(&bpf);

    /* Stash pcap-specific context away. */
    memcpy(hw_address, __lladdr, ETHER_ADDRLEN);
    __pp = pp;

    return (fd);
}

static void close_socket(int fd AVAHI_GCC_UNUSED) {
    assert(__pp != NULL);
    pcap_close(__pp);
    __pp = NULL;
}

/*
 * We trick avahi into allocating sizeof(packet) + sizeof(ether_header),
 * and prepend the required ethernet header information before sending.
 */
static int send_packet(int fd AVAHI_GCC_UNUSED, int iface AVAHI_GCC_UNUSED, ArpPacket *packet, size_t packet_len) {
    struct ether_header *eh;

    assert(__pp != NULL);
    assert(packet != NULL);

    eh = (struct ether_header *)packet->ether_header;
    memset(eh->ether_dhost, 0xFF, ETHER_ADDRLEN);
    memcpy(eh->ether_shost, __lladdr, ETHER_ADDRLEN);
    eh->ether_type = htons(0x0806);

    return (pcap_inject(__pp, (void *)eh, packet_len + sizeof(*eh)));
}

static int recv_packet(int fd AVAHI_GCC_UNUSED, ArpPacket **packet, size_t *packet_len) {
    struct pcap_pkthdr *ph;
    u_char *pd;
    ArpPacket *ap;
    int err;
    int retval;

    assert(__pp != NULL);
    assert(packet != NULL);
    assert(packet_len != NULL);

    *packet = NULL;
    *packet_len = 0;
    retval = -1;

    err = pcap_next_ex(__pp, &ph, (const u_char **)&pd);
    if (err == 1 && ph->caplen <= ph->len) {
        ap = packet_new(ph->caplen);
        memcpy(ap->ether_header, pd, ph->caplen);
        *packet = ap;
        *packet_len = (ph->caplen - sizeof(struct ether_header));
        retval = 0;
    } else if (err >= 0) {
        /*
         * err == 1: Just drop bogus packets (>1500 for an arp packet!?)
         * on the floor.
         *
         * err == 0: We might have had traffic on the pcap fd that
         * didn't match the filter, in which case we'll get 0 packets.
         */
        retval = 0;
    } else
        daemon_log(LOG_ERR, "pcap_next_ex(%d): %s",
                   err, pcap_geterr(__pp));

    return (retval);
}
#endif /* __linux__ */

int is_ll_address(uint32_t addr) {
    return
        ((ntohl(addr) & IPV4LL_NETMASK) == IPV4LL_NETWORK) &&
        ((ntohl(addr) & 0x0000FF00) != 0x0000) &&
        ((ntohl(addr) & 0x0000FF00) != 0xFF00);
}

static struct timeval *elapse_time(struct timeval *tv, unsigned msec, unsigned jitter) {
    assert(tv);

    gettimeofday(tv, NULL);

    if (msec)
        avahi_timeval_add(tv, (AvahiUsec) msec*1000);

    if (jitter)
        avahi_timeval_add(tv, (AvahiUsec) (jitter*1000.0*rand()/(RAND_MAX+1.0)));

    return tv;
}

static FILE* fork_dispatcher(void) {
    FILE *ret;
    int fds[2];
    pid_t pid;

    if (pipe(fds) < 0) {
        daemon_log(LOG_ERR, "pipe() failed: %s", strerror(errno));
        goto fail;
    }

    if ((pid = fork()) < 0)
        goto fail;
    else if (pid == 0) {
        FILE *f = NULL;
        int r = 1;

        /* Please note that the signal pipe is not closed at this
         * point, signals will thus be dispatched in the main
         * process. */

        daemon_retval_done();

        avahi_set_proc_title(argv0, "%s: [%s] callout dispatcher", argv0, interface_name);

        close(fds[1]);

        if (!(f = fdopen(fds[0], "r"))) {
            daemon_log(LOG_ERR, "fdopen() failed: %s", strerror(errno));
            goto dispatcher_fail;
        }

        for (;;) {
            CalloutEventInfo info;
            char name[IFNAMSIZ], buf[64];
            int k;

            if (fread(&info, sizeof(info), 1, f) != 1) {
                if (feof(f))
                    break;

                daemon_log(LOG_ERR, "fread() failed: %s", strerror(errno));
                goto dispatcher_fail;
            }

            assert(info.event <= CALLOUT_MAX);

            if (!if_indextoname(info.ifindex, name)) {
                daemon_log(LOG_ERR, "if_indextoname() failed: %s", strerror(errno));
                continue;
            }

            if (daemon_exec("/", &k,
                            action_script, action_script,
                            callout_event_table[info.event],
                            name,
                            inet_ntop(AF_INET, &info.address, buf, sizeof(buf)), NULL) < 0) {

                daemon_log(LOG_ERR, "Failed to run script: %s", strerror(errno));
                continue;
            }

            if (k != 0)
                daemon_log(LOG_WARNING, "Script execution failed with return value %i", k);
        }

        r = 0;

    dispatcher_fail:

        if (f)
            fclose(f);

#ifdef HAVE_CHROOT
        /* If the main process is trapped inside a chroot() we have to
         * remove the PID file for it */

        if (!no_chroot && wrote_pid_file)
            daemon_pid_file_remove();
#endif

        _exit(r);
    }

    /* parent */

    close(fds[0]);
    fds[0] = -1;

    if (!(ret = fdopen(fds[1], "w"))) {
        daemon_log(LOG_ERR, "fdopen() failed: %s", strerror(errno));
        goto fail;
    }

    return ret;

fail:
    if (fds[0] >= 0)
        close(fds[0]);
    if (fds[1] >= 0)
        close(fds[1]);

    return NULL;
}

static int do_callout(FILE *f, CalloutEvent event, int iface, uint32_t addr) {
    CalloutEventInfo info;
    char buf[64], ifname[IFNAMSIZ];

    daemon_log(LOG_INFO, "Callout %s, address %s on interface %s",
               callout_event_table[event],
               inet_ntop(AF_INET, &addr, buf, sizeof(buf)),
               if_indextoname(iface, ifname));

    info.event = event;
    info.ifindex = iface;
    info.address = addr;

    if (fwrite(&info, sizeof(info), 1, f) != 1 || fflush(f) != 0) {
        daemon_log(LOG_ERR, "Failed to write callout event: %s", strerror(errno));
        return -1;
    }

    return 0;
}

#define set_env(key, value) putenv(avahi_strdup_printf("%s=%s", (key), (value)))

static int drop_privs(void) {
    struct passwd *pw;
    struct group * gr;
    int r;
    mode_t u;

    pw = NULL;
    gr = NULL;

    /* Get user/group ID */

    if (!no_drop_root) {

        if (!(pw = getpwnam(AVAHI_AUTOIPD_USER))) {
            daemon_log(LOG_ERR, "Failed to find user '"AVAHI_AUTOIPD_USER"'.");
            return -1;
        }

        if (!(gr = getgrnam(AVAHI_AUTOIPD_GROUP))) {
            daemon_log(LOG_ERR, "Failed to find group '"AVAHI_AUTOIPD_GROUP"'.");
            return -1;
        }

        daemon_log(LOG_INFO, "Found user '"AVAHI_AUTOIPD_USER"' (UID %lu) and group '"AVAHI_AUTOIPD_GROUP"' (GID %lu).", (unsigned long) pw->pw_uid, (unsigned long) gr->gr_gid);
    }

    /* Create directory */
    u = umask(0000);
    r = mkdir(AVAHI_IPDATA_DIR, 0755);
    umask(u);

    if (r < 0 && errno != EEXIST) {
        daemon_log(LOG_ERR, "mkdir(\""AVAHI_IPDATA_DIR"\"): %s", strerror(errno));
        return -1;
    }

    /* Convey working directory */

    if (!no_drop_root) {
        struct stat st;

        chown(AVAHI_IPDATA_DIR, pw->pw_uid, gr->gr_gid);

        if (stat(AVAHI_IPDATA_DIR, &st) < 0) {
            daemon_log(LOG_ERR, "stat(): %s\n", strerror(errno));
            return -1;
        }

        if (!S_ISDIR(st.st_mode) || st.st_uid != pw->pw_uid || st.st_gid != gr->gr_gid) {
            daemon_log(LOG_ERR, "Failed to create runtime directory "AVAHI_IPDATA_DIR".");
            return -1;
        }
    }

#ifdef HAVE_CHROOT

    if (!no_chroot) {
        if (chroot(AVAHI_IPDATA_DIR) < 0) {
            daemon_log(LOG_ERR, "Failed to chroot(): %s", strerror(errno));
            return -1;
        }

        daemon_log(LOG_INFO, "Successfully called chroot().");
        chdir("/");

        /* Since we are now trapped inside a chroot we cannot remove
         * the pid file anymore, the helper process will do that for us. */
        wrote_pid_file = 0;
    }

#endif

    if (!no_drop_root) {

        if (initgroups(AVAHI_AUTOIPD_USER, gr->gr_gid) != 0) {
            daemon_log(LOG_ERR, "Failed to change group list: %s", strerror(errno));
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
            daemon_log(LOG_ERR, "Failed to change GID: %s", strerror(errno));
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
            daemon_log(LOG_ERR, "Failed to change UID: %s", strerror(errno));
            return -1;
        }

        set_env("USER", pw->pw_name);
        set_env("LOGNAME", pw->pw_name);
        set_env("HOME", pw->pw_dir);

        daemon_log(LOG_INFO, "Successfully dropped root privileges.");
    }

    return 0;
}

static int loop(int iface, uint32_t addr) {
    enum {
        FD_ARP,
        FD_IFACE,
        FD_SIGNAL,
        FD_MAX
    };

    int fd = -1, ret = -1;
    struct timeval next_wakeup;
    int next_wakeup_valid = 0;
    char buf[64];
    ArpPacket *in_packet = NULL;
    size_t in_packet_len = 0;
    ArpPacket *out_packet = NULL;
    size_t out_packet_len;
    uint8_t hw_address[ETHER_ADDRLEN];
    struct pollfd pollfds[FD_MAX];
    int iface_fd = -1;
    Event event = EVENT_NULL;
    int retval_sent = !daemonize;
    State st;
    FILE *dispatcher = NULL;
    char *address_fn = NULL;
    const char *p;

    daemon_signal_init(SIGINT, SIGTERM, SIGCHLD, SIGHUP, 0);

    if (!(dispatcher = fork_dispatcher()))
        goto fail;

    if ((fd = open_socket(iface, hw_address)) < 0)
        goto fail;

    if ((iface_fd = iface_init(iface)) < 0)
        goto fail;

    if (drop_privs() < 0)
        goto fail;

    if (force_bind)
        st = STATE_START;
    else if (iface_get_initial_state(&st) < 0)
        goto fail;

#ifdef HAVE_CHROOT
    if (!no_chroot)
        p = "";
    else
#endif
        p = AVAHI_IPDATA_DIR;

    address_fn = avahi_strdup_printf(
            "%s/%02x:%02x:%02x:%02x:%02x:%02x", p,
            hw_address[0], hw_address[1],
            hw_address[2], hw_address[3],
            hw_address[4], hw_address[5]);

    if (!addr)
        load_address(address_fn, &addr);

    if (addr && !is_ll_address(addr)) {
        daemon_log(LOG_WARNING, "Requested address %s is not from IPv4LL range 169.254/16 or a reserved address, ignoring.", inet_ntop(AF_INET, &addr, buf, sizeof(buf)));
        addr = 0;
    }

    if (!addr) {
        int i;
        uint32_t a = 1;

        for (i = 0; i < ETHER_ADDRLEN; i++)
            a += hw_address[i]*(i+1);

        a = (a % 0xFE00) + 0x0100;

        addr = htonl(IPV4LL_NETWORK | (uint32_t) a);
    }

    assert(is_ll_address(addr));

    set_state(st, 1, addr);

    daemon_log(LOG_INFO, "Starting with address %s", inet_ntop(AF_INET, &addr, buf, sizeof(buf)));

    if (state == STATE_SLEEPING)
        daemon_log(LOG_INFO, "Routable address already assigned, sleeping.");

    if (!retval_sent && (!wait_for_address || state == STATE_SLEEPING)) {
        daemon_retval_send(0);
        retval_sent = 1;
    }

    memset(pollfds, 0, sizeof(pollfds));
    pollfds[FD_ARP].fd = fd;
    pollfds[FD_ARP].events = POLLIN;
    pollfds[FD_IFACE].fd = iface_fd;
    pollfds[FD_IFACE].events = POLLIN;
    pollfds[FD_SIGNAL].fd = daemon_signal_fd();
    pollfds[FD_SIGNAL].events = POLLIN;

    for (;;) {
        int r, timeout;
        AvahiUsec usec;

        if (state == STATE_START) {

            /* First, wait a random time */
            set_state(STATE_WAITING_PROBE, 1, addr);

            elapse_time(&next_wakeup, 0, PROBE_WAIT*1000);
            next_wakeup_valid = 1;

        } else if ((state == STATE_WAITING_PROBE && event == EVENT_TIMEOUT) ||
                   (state == STATE_PROBING && event == EVENT_TIMEOUT && n_iteration < PROBE_NUM-2)) {

            /* Send a probe */
            out_packet = packet_new_probe(addr, hw_address, &out_packet_len);
            set_state(STATE_PROBING, 0, addr);

            elapse_time(&next_wakeup, PROBE_MIN*1000, (PROBE_MAX-PROBE_MIN)*1000);
            next_wakeup_valid = 1;

        } else if (state == STATE_PROBING && event == EVENT_TIMEOUT && n_iteration >= PROBE_NUM-2) {

            /* Send the last probe */
            out_packet = packet_new_probe(addr, hw_address, &out_packet_len);
            set_state(STATE_WAITING_ANNOUNCE, 1, addr);

            elapse_time(&next_wakeup, ANNOUNCE_WAIT*1000, 0);
            next_wakeup_valid = 1;

        } else if ((state == STATE_WAITING_ANNOUNCE && event == EVENT_TIMEOUT) ||
                   (state == STATE_ANNOUNCING && event == EVENT_TIMEOUT && n_iteration < ANNOUNCE_NUM-1)) {

            /* Send announcement packet */
            out_packet = packet_new_announcement(addr, hw_address, &out_packet_len);
            set_state(STATE_ANNOUNCING, 0, addr);

            elapse_time(&next_wakeup, ANNOUNCE_INTERVAL*1000, 0);
            next_wakeup_valid = 1;

            if (n_iteration == 0) {
                if (do_callout(dispatcher, CALLOUT_BIND, iface, addr) < 0)
                    goto fail;

                n_conflict = 0;
            }

        } else if ((state == STATE_ANNOUNCING && event == EVENT_TIMEOUT && n_iteration >= ANNOUNCE_NUM-1)) {

            daemon_log(LOG_INFO, "Successfully claimed IP address %s", inet_ntop(AF_INET, &addr, buf, sizeof(buf)));
            set_state(STATE_RUNNING, 0, addr);

            next_wakeup_valid = 0;

            save_address(address_fn, addr);

            if (!retval_sent) {
                daemon_retval_send(0);
                retval_sent = 1;
            }

        } else if (event == EVENT_PACKET) {
            ArpPacketInfo info;

            assert(in_packet);

            if (packet_parse(in_packet, in_packet_len, &info) < 0)
                daemon_log(LOG_WARNING, "Failed to parse incoming ARP packet.");
            else {
                int conflict = 0;

                if (info.sender_ip_address == addr) {

                    if (memcmp(hw_address, info.sender_hw_address, ETHER_ADDRLEN)) {
                        /* Normal conflict */
                        conflict = 1;
                        daemon_log(LOG_INFO, "Received conflicting normal ARP packet.");
                    } else
                        daemon_log(LOG_DEBUG, "Received ARP packet back on source interface. Ignoring.");

                } else if (state == STATE_WAITING_PROBE || state == STATE_PROBING || state == STATE_WAITING_ANNOUNCE) {
                    /* Probe conflict */
                    conflict = info.target_ip_address == addr &&
                               info.sender_ip_address == 0 &&
                               memcmp(hw_address, info.sender_hw_address, ETHER_ADDRLEN);

                    if (conflict)
                        daemon_log(LOG_INFO, "Received conflicting probe ARP packet.");
                }

                if (conflict) {

                    if (state == STATE_RUNNING || state == STATE_ANNOUNCING)
                        if (do_callout(dispatcher, CALLOUT_CONFLICT, iface, addr) < 0)
                            goto fail;

                    /* Pick a new address */
                    addr = pick_addr(addr);

                    daemon_log(LOG_INFO, "Trying address %s", inet_ntop(AF_INET, &addr, buf, sizeof(buf)));

                    n_conflict++;

                    set_state(STATE_WAITING_PROBE, 1, addr);

                    if (n_conflict >= MAX_CONFLICTS) {
                        daemon_log(LOG_WARNING, "Got too many conflicts, rate limiting new probes.");
                        elapse_time(&next_wakeup, RATE_LIMIT_INTERVAL*1000, PROBE_WAIT*1000);
                    } else
                        elapse_time(&next_wakeup, 0, PROBE_WAIT*1000);

                    next_wakeup_valid = 1;
                } else
                    DEBUG(daemon_log(LOG_DEBUG, "Ignoring irrelevant ARP packet."));
            }

        } else if (event == EVENT_ROUTABLE_ADDR_CONFIGURED && !force_bind) {

            daemon_log(LOG_INFO, "A routable address has been configured.");

            if (state == STATE_RUNNING || state == STATE_ANNOUNCING)
                if (do_callout(dispatcher, CALLOUT_UNBIND, iface, addr) < 0)
                    goto fail;

            if (!retval_sent) {
                daemon_retval_send(0);
                retval_sent = 1;
            }

            set_state(STATE_SLEEPING, 1, addr);
            next_wakeup_valid = 0;

        } else if (event == EVENT_ROUTABLE_ADDR_UNCONFIGURED && state == STATE_SLEEPING && !force_bind) {

            daemon_log(LOG_INFO, "No longer a routable address configured, restarting probe process.");

            set_state(STATE_WAITING_PROBE, 1, addr);

            elapse_time(&next_wakeup, 0, PROBE_WAIT*1000);
            next_wakeup_valid = 1;

        } else if (event == EVENT_REFRESH_REQUEST && state == STATE_RUNNING) {

            /* The user requested a reannouncing of the address by a SIGHUP */
            daemon_log(LOG_INFO, "Reannouncing address.");

            /* Send announcement packet */
            out_packet = packet_new_announcement(addr, hw_address, &out_packet_len);
            set_state(STATE_ANNOUNCING, 1, addr);

            elapse_time(&next_wakeup, ANNOUNCE_INTERVAL*1000, 0);
            next_wakeup_valid = 1;
        }

        if (out_packet) {
            DEBUG(daemon_log(LOG_DEBUG, "sending..."));

            if (send_packet(fd, iface, out_packet, out_packet_len) < 0)
                goto fail;

            avahi_free(out_packet);
            out_packet = NULL;
        }

        if (in_packet) {
            avahi_free(in_packet);
            in_packet = NULL;
        }

        event = EVENT_NULL;
        timeout = -1;

        if (next_wakeup_valid) {
            usec = avahi_age(&next_wakeup);
            timeout = usec < 0 ? (int) (-usec/1000) : 0;
        }

        DEBUG(daemon_log(LOG_DEBUG, "sleeping %ims", timeout));

        while ((r = poll(pollfds, FD_MAX, timeout)) < 0 && errno == EINTR)
            ;

        if (r < 0) {
            daemon_log(LOG_ERR, "poll() failed: %s", strerror(r));
            goto fail;
        } else if (r == 0) {
            event = EVENT_TIMEOUT;
            next_wakeup_valid = 0;
        } else {


            if (pollfds[FD_ARP].revents) {

                if (pollfds[FD_ARP].revents == POLLERR) {
                    /* The interface is probably down, let's recreate our socket */

                    close_socket(fd);

                    if ((fd = open_socket(iface, hw_address)) < 0)
                        goto fail;

                    pollfds[FD_ARP].fd = fd;

                } else {

                    assert(pollfds[FD_ARP].revents == POLLIN);

                    if (recv_packet(fd, &in_packet, &in_packet_len) < 0)
                        goto fail;

                    if (in_packet)
                        event = EVENT_PACKET;
                }
            }

            if (event == EVENT_NULL &&
                pollfds[FD_IFACE].revents) {

                assert(pollfds[FD_IFACE].revents == POLLIN);

                if (iface_process(&event) < 0)
                    goto fail;
            }

            if (event == EVENT_NULL &&
                pollfds[FD_SIGNAL].revents) {

                int sig;
                assert(pollfds[FD_SIGNAL].revents == POLLIN);

                if ((sig = daemon_signal_next()) <= 0) {
                    daemon_log(LOG_ERR, "daemon_signal_next() failed");
                    goto fail;
                }

                switch(sig) {
                    case SIGINT:
                    case SIGTERM:
                        daemon_log(LOG_INFO, "Got %s, quitting.", sig == SIGINT ? "SIGINT" : "SIGTERM");
                        ret = 0;
                        goto fail;

                    case SIGCHLD:
                        waitpid(-1, NULL, WNOHANG);
                        break;

                    case SIGHUP:
                        event = EVENT_REFRESH_REQUEST;
                        break;
                }

            }
        }
    }

    ret = 0;

fail:

    if (state == STATE_RUNNING || state == STATE_ANNOUNCING)
        do_callout(dispatcher, CALLOUT_STOP, iface, addr);

    avahi_free(out_packet);
    avahi_free(in_packet);

    if (fd >= 0)
        close_socket(fd);

    if (iface_fd >= 0)
        iface_done();

    if (daemonize && !retval_sent)
        daemon_retval_send(ret);

    if (dispatcher)
        fclose(dispatcher);

    if (address_fn)
        avahi_free(address_fn);

    return ret;
}


static void help(FILE *f, const char *a0) {
    fprintf(f,
            "%s [options] INTERFACE\n"
            "    -h --help           Show this help\n"
            "    -D --daemonize      Daemonize after startup\n"
            "    -s --syslog         Write log messages to syslog(3) instead of STDERR\n"
            "    -k --kill           Kill a running daemon\n"
            "    -r --refresh        Request a running daemon refresh its IP address\n"
            "    -c --check          Return 0 if a daemon is already running\n"
            "    -V --version        Show version\n"
            "    -S --start=ADDRESS  Start with this address from the IPv4LL range\n"
            "                        169.254.0.0/16\n"
            "    -t --script=script  Action script to run (defaults to\n"
            "                        "AVAHI_IPCONF_SCRIPT")\n"
            "    -w --wait           Wait until an address has been acquired before\n"
            "                        daemonizing\n"
            "       --force-bind     Assign an IPv4LL address even if a routable address\n"
            "                        is already assigned\n"
            "       --no-drop-root   Don't drop privileges\n"
#ifdef HAVE_CHROOT
            "       --no-chroot      Don't chroot()\n"
#endif
            "       --no-proc-title  Don't modify process title\n"
            "       --debug          Increase verbosity\n",
            a0);
}

static int parse_command_line(int argc, char *argv[]) {
    int c;

    enum {
        OPTION_NO_PROC_TITLE = 256,
        OPTION_FORCE_BIND,
        OPTION_DEBUG,
        OPTION_NO_DROP_ROOT,
#ifdef HAVE_CHROOT
        OPTION_NO_CHROOT
#endif
    };

    static const struct option long_options[] = {
        { "help",          no_argument,       NULL, 'h' },
        { "daemonize",     no_argument,       NULL, 'D' },
        { "syslog",        no_argument,       NULL, 's' },
        { "kill",          no_argument,       NULL, 'k' },
        { "refresh",       no_argument,       NULL, 'r' },
        { "check",         no_argument,       NULL, 'c' },
        { "version",       no_argument,       NULL, 'V' },
        { "start",         required_argument, NULL, 'S' },
        { "script",        required_argument, NULL, 't' },
        { "wait",          no_argument,       NULL, 'w' },
        { "force-bind",    no_argument,       NULL, OPTION_FORCE_BIND },
        { "no-drop-root",  no_argument,       NULL, OPTION_NO_DROP_ROOT },
#ifdef HAVE_CHROOT
        { "no-chroot",     no_argument,       NULL, OPTION_NO_CHROOT },
#endif
        { "no-proc-title", no_argument,       NULL, OPTION_NO_PROC_TITLE },
        { "debug",         no_argument,       NULL, OPTION_DEBUG },
        { NULL, 0, NULL, 0 }
    };

    while ((c = getopt_long(argc, argv, "hDskrcVS:t:w", long_options, NULL)) >= 0) {

        switch(c) {
            case 's':
                use_syslog = 1;
                break;
            case 'h':
                command = DAEMON_HELP;
                break;
            case 'D':
                daemonize = 1;
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
            case 'S':

                if ((start_address = inet_addr(optarg)) == (uint32_t) -1) {
                    fprintf(stderr, "Failed to parse IP address '%s'.", optarg);
                    return -1;
                }
                break;
            case 't':
                avahi_free(action_script);
                action_script = avahi_strdup(optarg);
                break;
            case 'w':
                wait_for_address = 1;
                break;

            case OPTION_NO_PROC_TITLE:
                modify_proc_title = 0;
                break;

            case OPTION_DEBUG:
                debug = 1;
#ifdef DAEMON_SET_VERBOSITY_AVAILABLE
                daemon_set_verbosity(LOG_DEBUG);
#endif
                break;

            case OPTION_FORCE_BIND:
                force_bind = 1;
                break;

            case OPTION_NO_DROP_ROOT:
                no_drop_root = 1;
                break;

#ifdef HAVE_CHROOT
            case OPTION_NO_CHROOT:
                no_chroot = 1;
                break;
#endif

            default:
                return -1;
        }
    }

    if (command == DAEMON_RUN ||
        command == DAEMON_KILL ||
        command == DAEMON_REFRESH ||
        command == DAEMON_CHECK) {

        if (optind >= argc) {
            fprintf(stderr, "Missing interface name.\n");
            return -1;
        }

        interface_name = avahi_strdup(argv[optind++]);
    }

    if (optind != argc) {
        fprintf(stderr, "Too many arguments\n");
        return -1;
    }

    if (!action_script)
        action_script = avahi_strdup(AVAHI_IPCONF_SCRIPT);

    return 0;
}

static const char* pid_file_proc(void) {
    return pid_file_name;
}

int main(int argc, char*argv[]) {
    int r = 1;
    char *log_ident = NULL;

    signal(SIGPIPE, SIG_IGN);

    if ((argv0 = strrchr(argv[0], '/')))
        argv0 = avahi_strdup(argv0 + 1);
    else
        argv0 = avahi_strdup(argv[0]);

    daemon_log_ident = argv0;

    if (parse_command_line(argc, argv) < 0)
        goto finish;

    if (modify_proc_title)
        avahi_init_proc_title(argc, argv);

    daemon_log_ident = log_ident = avahi_strdup_printf("%s(%s)", argv0, interface_name);
    daemon_pid_file_proc = pid_file_proc;
    pid_file_name = avahi_strdup_printf(AVAHI_RUNTIME_DIR"/avahi-autoipd.%s.pid", interface_name);

    if (command == DAEMON_RUN) {
        pid_t pid;
        int ifindex;

        init_rand_seed();

        if ((ifindex = if_nametoindex(interface_name)) <= 0) {
            daemon_log(LOG_ERR, "Failed to get index for interface name '%s': %s", interface_name, strerror(errno));
            goto finish;
        }

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

        avahi_set_proc_title(argv0, "%s: [%s] starting up", argv0, interface_name);

        if (loop(ifindex, start_address) < 0)
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

    avahi_free(log_ident);
    avahi_free(pid_file_name);
    avahi_free(argv0);
    avahi_free(interface_name);
    avahi_free(action_script);

    return r;
}
