/* dispatch.c
 *
 * Copyright (c) 2018 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Simple event dispatcher for DNS.
 */

#define __APPLE_USE_RFC_3542

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/event.h>
#include <fcntl.h>
#include <sys/time.h>

#include "srp.h"
#include "dns-msg.h"
#include "srp-crypto.h"
#include "ioloop.h"
#include "dnssd-proxy.h"

#define USE_SELECT
#pragma mark Globals
io_t *ios;
int64_t ioloop_now;

#ifdef USE_KQUEUE
int kq;
#endif

int
getipaddr(addr_t *addr, const char *p)
{
    if (inet_pton(AF_INET, p, &addr->sin.sin_addr)) {
        addr->sa.sa_family = AF_INET;
        return sizeof addr->sin;
    }  else if (inet_pton(AF_INET6, p, &addr->sin6.sin6_addr)) {
        addr->sa.sa_family = AF_INET6;
        return sizeof addr->sin6;
    } else {
        return 0;
    }
}                

int64_t
ioloop_timenow()
{
    int64_t now;
    struct timeval tv;
    gettimeofday(&tv, 0);
    now = (int64_t)tv.tv_sec * 1000 + (int64_t)tv.tv_usec / 1000;
    return now;
}

message_t *
message_allocate(size_t message_size)
{
    message_t *message = (message_t *)malloc(message_size + (sizeof (message_t)) - (sizeof (dns_wire_t)));
    if (message)
        memset(message, 0, (sizeof (message_t)) - (sizeof (dns_wire_t)));
    return message;
}

void
message_free(message_t *message)
{
    free(message);
}

void
comm_free(comm_t *comm)
{
    if (comm->name) {
        free(comm->name);
        comm->name = NULL;
    }
    if (comm->message) {
        message_free(comm->message);
        comm->message = NULL;
        comm->buf = NULL;
    }
    free(comm);
}

void
ioloop_close(io_t *io)
{
    close(io->sock);
    io->sock = -1;
}

void
add_reader(io_t *io, io_callback_t callback, io_callback_t finalize)
{
    io->next = ios;
    ios = io;
    io->read_callback = callback;
    io->finalize = finalize;
#ifdef USE_SELECT
    io->want_read = true;
#endif
#ifdef USE_EPOLL
#endif
#ifdef USE_KQUEUE
    struct kevent ev;
    int rv;
    EV_SET(&ev, io->sock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, io);
    rv = kevent(kq, &ev, 1, NULL, 0, NULL);
    if (rv < 0) {
        ERROR("kevent add: %s", strerror(errno));
        return;
    }
#endif // USE_EPOLL
}

bool
ioloop_init(void)
{
    signal(SIGPIPE, SIG_IGN); // because why ever?
#ifdef USE_KQUEUE
    kq = kqueue();
    if (kq < 0) {
        ERROR("kqueue(): %s", strerror(errno));
        return false;
    }
#endif
    return true;
}

int
ioloop_events(int64_t timeout_when)
{
    io_t *io, **iop;
    int nev = 0, rv;
    int64_t now = ioloop_timenow();
    int64_t next_event = timeout_when;
    int64_t timeout = 0;

    INFO("%qd.%03qd seconds have passed on entry to ioloop_events", (now - ioloop_now) / 1000, (now - ioloop_now) % 1000);
    ioloop_now = now;

    // A timeout of zero means don't time out.
    if (timeout_when == 0) {
        next_event = INT64_MAX;
    } else {
        next_event = timeout_when;
    }

#ifdef USE_SELECT
    int nfds = 0;
    fd_set reads, writes, errors;
    struct timeval tv;

    FD_ZERO(&reads);
    FD_ZERO(&writes);
    FD_ZERO(&errors);
    
#endif
    iop = &ios;
    while (*iop) {
        io = *iop;
        if (io->sock != -1 && io->wakeup_time != 0) {
            if (io->wakeup_time <= ioloop_now) {
                io->wakeup_time = 0;
                io->wakeup(io);
                ++nev;
            } else if (io->wakeup_time < next_event) {
                next_event = io->wakeup_time;
            }
        }

        if (io->sock == -1) {
            *iop = io->next;
            if (io->finalize) {
                io->finalize(io);
            } else {
                free(io);
            }
            continue;
        }

        // INFO("now: %qd  io %d wakeup_time %qd  next_event %qd", ioloop_now, io->sock, io->wakeup_time, next_event);

        // If we were given a timeout in the future, or told to wait indefinitely, wait until the next event.
        if (timeout_when == 0 || timeout_when > ioloop_now) {
            timeout = next_event - ioloop_now;
            // Don't choose a time so far in the future that it might overflow some math in the kernel.
            if (timeout > IOLOOP_DAY * 100) {
                timeout = IOLOOP_DAY * 100;
            }
#ifdef USE_SELECT
            tv.tv_sec = timeout / 1000;
            tv.tv_usec = (timeout % 1000) * 1000;
#endif
#ifdef USE_KQUEUE
            ts.tv_sec = timeout / 1000;
            ts.tv_nsec = (timeout % 1000) * 1000 * 1000;
#endif
        }
        iop = &io->next;
    }

#ifdef USE_SELECT
    for (io = ios; io; io = io->next) {
        if (io->sock != -1 && (io->want_read || io->want_write)) {
            if (io->sock >= nfds) {
                nfds = io->sock + 1;
            }
            if (io->want_read) {
                FD_SET(io->sock, &reads);
            }
            if (io->want_write) {
                FD_SET(io->sock, &writes);
            }
        }
    }
#endif

#ifdef USE_SELECT
    INFO("waiting %ld %d seconds", tv.tv_sec, tv.tv_usec);
    rv = select(nfds, &reads, &writes, &writes, &tv);
    if (rv < 0) {
        ERROR("select: %s", strerror(errno));
        exit(1);
    }
    now = ioloop_timenow();
    INFO("%qd.%03qd seconds passed waiting, got %d events", (now - ioloop_now) / 1000, (now - ioloop_now) % 1000, rv);
    ioloop_now = now;
    for (io = ios; io; io = io->next) {
        if (io->sock != -1) {
            if (FD_ISSET(io->sock, &reads)) {
                io->read_callback(io);
            } else if (FD_ISSET(io->sock, &writes)) {
                io->write_callback(io);
            }
        }
    }
    nev += rv;
#endif // USE_SELECT
#ifdef USE_KQUEUE
#define KEV_MAX 20
    struct kevent evs[KEV_MAX];
    int i, rv;
    struct timespec ts;

    INFO("waiting %qd/%qd seconds", ts.tv_sec, ts.tv_nsec);
    do {
        rv = kevent(kq, NULL, 0, evs, KEV_MAX, &ts);
        now = ioloop_timenow();
        INFO("%qd.%03qd seconds passed waiting, got %d events", (now - ioloop_now) / 1000, (now - ioloop_now) % 1000, rv);
        ioloop_now = now;
        ts.tv_sec = 0;
        ts.tv_nsec = 0;
        if (rv < 0) {
            ERROR("kevent poll: %s", strerror(errno));
            exit(1);
        }
        for (i = 0; i < nev; i++) {
            io = evs[i].udata;
            if (evs[i].filter == EVFILT_WRITE) {
                io->write_callback(io);
            } else if (evs[i].filter == EVFILT_READ) {
                io->read_callback(io);
            }
        }
        nev += rv;
    } while (rv == KEV_MAX);
#endif
    return nev;
}

static void
udp_read_callback(io_t *io)
{
    comm_t *connection = (comm_t *)io;
    addr_t src;
    int rv;
    struct msghdr msg;
    struct iovec bufp;
    uint8_t msgbuf[DNS_MAX_UDP_PAYLOAD];
    char cmsgbuf[128];
    struct cmsghdr *cmh;
    message_t *message;

    bufp.iov_base = msgbuf;
    bufp.iov_len = DNS_MAX_UDP_PAYLOAD;
    msg.msg_iov = &bufp;
    msg.msg_iovlen = 1;
    msg.msg_name = &src;
    msg.msg_namelen = sizeof src;
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof cmsgbuf;
    
    rv = recvmsg(connection->io.sock, &msg, 0);
    if (rv < 0) {
        ERROR("udp_read_callback: %s", strerror(errno));
        return;
    }
    message = message_allocate(rv);
    if (!message) {
        ERROR("udp_read_callback: out of memory");
        return;
    }
    memcpy(&message->src, &src, sizeof src);
    message->length = rv;
    memcpy(&message->wire, msgbuf, rv);
    
    // For UDP, we use the interface index as part of the validation strategy, so go get
    // the interface index.
    for (cmh = CMSG_FIRSTHDR(&msg); cmh; cmh = CMSG_NXTHDR(&msg, cmh)) {
        if (cmh->cmsg_level == IPPROTO_IPV6 && cmh->cmsg_type == IPV6_PKTINFO) {
            struct in6_pktinfo pktinfo;    

            memcpy(&pktinfo, CMSG_DATA(cmh), sizeof pktinfo);
            message->ifindex = pktinfo.ipi6_ifindex;
        } else if (cmh->cmsg_level == IPPROTO_IP && cmh->cmsg_type == IP_PKTINFO) { 
            struct in_pktinfo pktinfo;
          
            memcpy(&pktinfo, CMSG_DATA(cmh), sizeof pktinfo);
            message->ifindex = pktinfo.ipi_ifindex;
        }
    }
    connection->message = message;
    connection->datagram_callback(connection);
}

static void
tcp_read_callback(io_t *context)
{
    comm_t *connection = (comm_t *)context;
    int rv;
    if (connection->message_length_len < 2) {
        rv = read(connection->io.sock, &connection->message_length_bytes[connection->message_length_len],
                  2 - connection->message_length_len);
        if (rv < 0) {
        read_error:
            ERROR("tcp_read_callback: %s", strerror(errno));
            close(connection->io.sock);
            connection->io.sock = -1;
            // connection->io.finalize() will be called from the io loop.
            return;
        }
        // If we read zero here, the remote endpoint has closed or shutdown the connection.  Either case is
        // effectively the same--if we are sensitive to read events, that means that we are done processing
        // the previous message.
        if (rv == 0) {
        eof:
            ERROR("tcp_read_callback: remote end (%s) closed connection on %d", connection->name, connection->io.sock);
            close(connection->io.sock);
            connection->io.sock = -1;
            // connection->io.finalize() will be called from the io loop.
            return;
        }
        connection->message_length_len += rv;
        if (connection->message_length_len == 2) {
            connection->message_length = (((uint16_t)connection->message_length_bytes[0] << 8) |
                                          ((uint16_t)connection->message_length_bytes[1]));
        }
        return;
    }

    // If we only just got the length, we need to allocate a message
    if (connection->message == NULL) {
        connection->message = message_allocate(connection->message_length);
        if (!connection->message) {
            ERROR("udp_read_callback: out of memory");
            return;
        }
        connection->buf = (uint8_t *)&connection->message->wire;
        connection->message->length = connection->message_length;
        memset(&connection->message->src, 0, sizeof connection->message->src);
    }

    rv = read(connection->io.sock, &connection->buf[connection->message_cur],
              connection->message_length - connection->message_cur);
    if (rv < 0) {
        goto read_error;
    }
    if (rv == 0) {
        goto eof;
    }

    connection->message_cur += rv;
    if (connection->message_cur == connection->message_length) {
        connection->datagram_callback(connection);
        // Caller is expected to consume the message, we are immediately ready for the next read.
        connection->message_length = connection->message_length_len = 0;
    }
}

static void
tcp_send_response(comm_t *comm, message_t *responding_to, struct iovec *iov, int iov_len)
{
    struct msghdr mh;
    struct iovec iovec[4];
    char lenbuf[2];
    ssize_t status;
    size_t payload_length = 0;
    int i;

    // We don't anticipate ever needing more than four hunks, but if we get more, handle then?
    if (iov_len > 3) {
        ERROR("tcp_send_response: too many io buffers");
        close(comm->io.sock);
        comm->io.sock = -1;
        return;
    }

    iovec[0].iov_base = &lenbuf[0];
    iovec[0].iov_len = 2;
    for (i = 0; i < iov_len; i++) {
        iovec[i + 1] = iov[i];
        payload_length += iov[i].iov_len;
    }
    lenbuf[0] = payload_length / 256;
    lenbuf[1] = payload_length & 0xff;
    payload_length += 2;

    memset(&mh, 0, sizeof mh);
    mh.msg_iov = &iovec[0];
    mh.msg_iovlen = iov_len + 1;
    mh.msg_name = 0;

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
    status = sendmsg(comm->io.sock, &mh, MSG_NOSIGNAL);
    if (status < 0 || status != payload_length) {
        if (status < 0) {
            ERROR("tcp_send_response: write failed: %s", strerror(errno));
        } else {
            ERROR("tcp_send_response: short write (%zd out of %zu bytes)", status, payload_length);
        }
        close(comm->io.sock);
        comm->io.sock = -1;
    }
}

static void
udp_send_response(comm_t *comm, message_t *responding_to, struct iovec *iov, int iov_len)
{
    struct msghdr mh;
    memset(&mh, 0, sizeof mh);
    mh.msg_iov = iov;
    mh.msg_iovlen = iov_len;
    mh.msg_name = &responding_to->src;
    if (responding_to->src.sa.sa_family == AF_INET) {
        mh.msg_namelen = sizeof (struct sockaddr_in);
    } else if (responding_to->src.sa.sa_family == AF_INET6) {
        mh.msg_namelen = sizeof (struct sockaddr_in6);
    } else {
        ERROR("send_udp_response: unknown family %d", responding_to->src.sa.sa_family);
        abort();
    }
    sendmsg(comm->io.sock, &mh, 0);
}

// When a communication is closed, scan the io event list to see if any other ios are referencing this one.
void
comm_finalize(io_t *io_in) {
    io_t *io;

    for (io = ios; io; io = io->next) {
        if (io->cancel_on_close == io_in && io->cancel != NULL) {
            io->cancel(io);
        }
    }
}

static void
listen_callback(io_t *context)
{
    comm_t *listener = (comm_t *)context;
    int rv;
    addr_t addr;
    socklen_t addr_len = sizeof addr;
    comm_t *comm;
    char addrbuf[INET6_ADDRSTRLEN + 7];
    int addrlen;

    rv = accept(listener->io.sock, &addr.sa, &addr_len);
    if (rv < 0) {
        ERROR("accept: %s", strerror(errno));
        close(listener->io.sock);
        listener->io.sock = -1;
        return;
    }
    inet_ntop(addr.sa.sa_family, (addr.sa.sa_family == AF_INET
                                  ? (void *)&addr.sin.sin_addr
                                  : (void *)&addr.sin6.sin6_addr), addrbuf, sizeof addrbuf);
    addrlen = strlen(addrbuf);
    snprintf(&addrbuf[addrlen], (sizeof addrbuf) - addrlen, "%%%d",
             (addr.sa.sa_family == AF_INET ? addr.sin.sin_port : addr.sin6.sin6_port));
    comm = calloc(1, sizeof *comm);
    comm->name = strdup(addrbuf);
    comm->io.sock = rv;
    comm->address = addr;
    comm->datagram_callback = listener->datagram_callback;
    comm->send_response = tcp_send_response;
    comm->tcp_stream = true;

    if (listener->connected) {
        listener->connected(comm);
    }
    add_reader(&comm->io, tcp_read_callback, NULL);
    comm->io.finalize = comm_finalize;

#ifdef SO_NOSIGPIPE
    int one = 1;
    rv = setsockopt(comm->io.sock, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof one);
    if (rv < 0) {
        ERROR("SO_NOSIGPIPE failed: %s", strerror(errno));
    }
#endif
}

comm_t *
setup_listener_socket(int family, int protocol, uint16_t port, const char *name,
                      comm_callback_t datagram_callback,
                      comm_callback_t connected, void *context)
{
    comm_t *listener;
    socklen_t sl;
    int rv;
    int flag = 1;
    
    listener = calloc(1, sizeof *listener);
    if (listener == NULL) {
        return NULL;
    }
    listener->name = strdup(name);
    if (!listener->name) {
        free(listener);
        return NULL;
    }
    listener->io.sock = socket(family, protocol == IPPROTO_UDP ? SOCK_DGRAM : SOCK_STREAM, protocol);
    if (listener->io.sock < 0) {
        ERROR("Can't get socket: %s", strerror(errno));
        comm_free(listener);
        return NULL;
    }
    rv = setsockopt(listener->io.sock, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof flag);
    if (rv < 0) {
        ERROR("SO_REUSEPORT failed: %s", strerror(errno));
        comm_free(listener);
        return NULL;
    }

    if (family == AF_INET) {
        sl = sizeof listener->address.sin;
        listener->address.sin.sin_port = port ? port : htons(53);
    } else {
        sl = sizeof listener->address.sin6;
        listener->address.sin6.sin6_port = port ? port : htons(53);
    }
    listener->address.sa.sa_family = family;
    listener->address.sa.sa_len = sl;
    if (bind(listener->io.sock, &listener->address.sa, sl) < 0) {
        ERROR("Can't bind to 0#53/%s%s: %s",
                protocol == IPPROTO_UDP ? "udp" : "tcp", family == AF_INET ? "v4" : "v6",
                strerror(errno));
    out:
        close(listener->io.sock);
        free(listener);
        return NULL;
    }

    if (protocol == IPPROTO_TCP) {
        if (listen(listener->io.sock, 5 /* xxx */) < 0) {
            ERROR("Can't listen on 0#53/%s%s: %s.",
                    protocol == IPPROTO_UDP ? "udp" : "tcp", family == AF_INET ? "v4" : "v6",
                    strerror(errno));
            goto out;
        }                
        add_reader(&listener->io, listen_callback, NULL);
    } else {
        rv = setsockopt(listener->io.sock, family == AF_INET ? IPPROTO_IP : IPPROTO_IPV6,
                        family == AF_INET ? IP_PKTINFO : IPV6_RECVPKTINFO, &flag, sizeof flag);
        if (rv < 0) {
            ERROR("Can't set %s: %s.", family == AF_INET ? "IP_PKTINFO" : "IPV6_RECVPKTINFO",
                    strerror(errno));
            goto out;
        }
        add_reader(&listener->io, udp_read_callback, NULL);
        listener->send_response = udp_send_response;
    }
    listener->datagram_callback = datagram_callback;
    listener->connected = connected;
    return listener;
}

// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
