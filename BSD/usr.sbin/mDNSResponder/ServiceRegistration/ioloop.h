/* ioloop.c
 *
 * Copyright (c) 2018-2019 Apple Computer, Inc. All rights reserved.
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
 * Definitions for simple dispatch implementation.
 */

#ifndef __IOLOOP_H
#define __IOLOOP_H

#ifndef __DSO_H
typedef struct dso_state dso_state_t;
#endif

typedef union addr addr_t;
union addr {
    struct sockaddr sa;
    struct sockaddr_in sin;
    struct sockaddr_in6 sin6;
};

typedef struct message message_t;
struct message {
    addr_t src;
    int ifindex;
    size_t length;
    dns_wire_t wire;
};

typedef struct dso_transport comm_t;
typedef struct io io_t;
typedef void (*io_callback_t)(io_t *NONNULL io);
typedef void (*comm_callback_t)(comm_t *NONNULL comm);
typedef void (*send_response_t)(comm_t *NONNULL comm, message_t *NONNULL responding_to, struct iovec *NONNULL iov, int count);

#define IOLOOP_SECOND	1000LL
#define IOLOOP_MINUTE	60 * IOLOOP_SECOND
#define IOLOOP_HOUR		60 * IOLOOP_MINUTE
#define IOLOOP_DAY		24 * IOLOOP_HOUR

struct io {
    io_t *NULLABLE next;
    io_callback_t NULLABLE read_callback;
    io_callback_t NULLABLE write_callback;
    io_callback_t NULLABLE finalize;
    io_callback_t NULLABLE wakeup;
    io_callback_t NULLABLE cancel;
    int sock;
    int64_t wakeup_time;
    io_t *NULLABLE cancel_on_close;
    bool want_read : 1;
    bool want_write : 1;
};

struct dso_transport {
    io_t io;
    char *NONNULL name;
    void *NULLABLE context;
    comm_callback_t NULLABLE datagram_callback;
    comm_callback_t NULLABLE close_callback;
    send_response_t NULLABLE send_response;
    comm_callback_t NULLABLE connected;
    message_t *NULLABLE message;
    uint8_t *NULLABLE buf;
    dso_state_t *NULLABLE dso;
    addr_t address;
    size_t message_length_len;
    size_t message_length, message_cur;
    uint8_t message_length_bytes[2];
    bool tcp_stream: 1;
};

extern int64_t ioloop_now;
int getipaddr(addr_t *NONNULL addr, const char *NONNULL p);
int64_t ioloop_timenow(void);
message_t *NULLABLE message_allocate(size_t message_size);
void message_free(message_t *NONNULL message);
void comm_free(comm_t *NONNULL comm);
void ioloop_close(io_t *NONNULL io);
void add_reader(io_t *NONNULL io, io_callback_t NONNULL callback, io_callback_t NULLABLE finalize);
bool ioloop_init(void);
int ioloop_events(int64_t timeout_when);
comm_t *NULLABLE setup_listener_socket(int family, int protocol, uint16_t port, const char *NONNULL name,
                                       comm_callback_t NONNULL datagram_callback,
                                       comm_callback_t NULLABLE connected, void *NULLABLE context);
#endif
// Local Variables:
// mode: C
// tab-width: 4
// c-file-style: "bsd"
// c-basic-offset: 4
// fill-column: 108
// indent-tabs-mode: nil
// End:
