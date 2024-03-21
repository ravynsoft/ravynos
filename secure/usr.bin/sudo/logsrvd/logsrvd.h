/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (c) 2019-2022 Todd C. Miller <Todd.Miller@sudo.ws>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SUDO_LOGSRVD_H
#define SUDO_LOGSRVD_H

#include <log_server.pb-c.h>
#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error protobuf-c version 1.30 or higher required
#endif

#include <config.h>

#if defined(HAVE_OPENSSL)
# if defined(HAVE_WOLFSSL)
#  include <wolfssl/options.h>
# endif
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#include "logsrv_util.h"
#include <tls_common.h>

/* Default timeout value for server socket */
#define DEFAULT_SOCKET_TIMEOUT_SEC 30

/* How often to send an ACK to the client (commit point) in seconds */
#define ACK_FREQUENCY	10

/* Shutdown timeout (in seconds) in case client connections time out. */
#define SHUTDOWN_TIMEO	10

/* Template for mkstemp(3) when creating temporary files. */
#define RELAY_TEMPLATE	"relay.XXXXXXXX"

/*
 * Connection status.
 * In the RUNNING state we expect I/O log buffers.
 */
enum connection_status {
    INITIAL,
    CONNECTING,
    RUNNING,
    EXITED,
    SHUTDOWN,
    FINISHED
};

/*
 * Per-connection relay state.
 */
struct relay_closure {
    struct server_address_list *relays;
    struct server_address *relay_addr;
    struct sudo_event *read_ev;
    struct sudo_event *write_ev;
    struct sudo_event *connect_ev;
    struct connection_buffer read_buf;
    struct connection_buffer_list write_bufs;
    struct peer_info relay_name;
#if defined(HAVE_OPENSSL)
    struct tls_client_closure tls_client;
#endif
    int sock;
    bool read_instead_of_write;
    bool write_instead_of_read;
    bool temporary_write_event;
};

/*
 * Per-connection state.
 */
struct connection_closure {
    TAILQ_ENTRY(connection_closure) entries;
    struct client_message_switch *cms;
    struct relay_closure *relay_closure;
    struct eventlog *evlog;
    struct timespec elapsed_time;
    struct connection_buffer read_buf;
    struct connection_buffer_list write_bufs;
    struct connection_buffer_list free_bufs;
    struct sudo_event_base *evbase;
    struct sudo_event *commit_ev;
    struct sudo_event *read_ev;
    struct sudo_event *write_ev;
#if defined(HAVE_OPENSSL)
    struct sudo_event *ssl_accept_ev;
    SSL *ssl;
#endif
    const char *errstr;
    FILE *journal;
    char *journal_path;
    struct iolog_file iolog_files[IOFD_MAX];
    int iolog_dir_fd;
    int sock;
    enum connection_status state;
    bool error;
    bool tls;
    bool log_io;
    bool store_first;
    bool read_instead_of_write;
    bool write_instead_of_read;
    bool temporary_write_event;
#ifdef HAVE_STRUCT_IN6_ADDR
    char ipaddr[INET6_ADDRSTRLEN];
#else
    char ipaddr[INET_ADDRSTRLEN];
#endif
};

/* Client message switch. */
struct client_message_switch {
    bool (*accept)(AcceptMessage *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*reject)(RejectMessage *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*exit)(ExitMessage *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*restart)(RestartMessage *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*alert)(AlertMessage *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*iobuf)(int iofd, IoBuffer *iobuf, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*suspend)(CommandSuspend *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
    bool (*winsize)(ChangeWindowSize *msg, uint8_t *buf, size_t len,
	struct connection_closure *closure);
};

union sockaddr_union {
    struct sockaddr sa;
    struct sockaddr_in sin;
#ifdef HAVE_STRUCT_IN6_ADDR
    struct sockaddr_in6 sin6;
#endif
};

/*
 * List of server addresses.
 */
struct server_address {
    TAILQ_ENTRY(server_address) entries;
    char *sa_host;
    char *sa_str;
    union sockaddr_union sa_un;
    socklen_t sa_size;
    bool tls;
};
TAILQ_HEAD(server_address_list, server_address);

/*
 * List of active network listeners.
 */
struct listener {
    TAILQ_ENTRY(listener) entries;
    struct sudo_event *ev;
    int sock;
    bool tls;
};
TAILQ_HEAD(listener_list, listener);

/*
 * Queue of finished journal files to be relayed.
 */
struct outgoing_journal {
    TAILQ_ENTRY(outgoing_journal) entries;
    char *journal_path;
};
TAILQ_HEAD(outgoing_journal_queue, outgoing_journal);

/* iolog_writer.c */
struct eventlog *evlog_new(TimeSpec *submit_time, InfoMessage **info_msgs, size_t infolen, struct connection_closure *closure);
bool iolog_init(AcceptMessage *msg, struct connection_closure *closure);
bool iolog_create(int iofd, struct connection_closure *closure);
void iolog_close_all(struct connection_closure *closure);
bool iolog_flush_all(struct connection_closure *closure);
bool iolog_rewrite(const struct timespec *target, struct connection_closure *closure);
void update_elapsed_time(TimeSpec *delta, struct timespec *elapsed);

/* logsrvd.c */
extern struct client_message_switch cms_local;
bool start_protocol(struct connection_closure *closure);
void connection_close(struct connection_closure *closure);
bool schedule_commit_point(TimeSpec *commit_point, struct connection_closure *closure);
bool fmt_log_id_message(const char *id, struct connection_closure *closure);
bool schedule_error_message(const char *errstr, struct connection_closure *closure);
struct connection_buffer *get_free_buf(size_t, struct connection_closure *closure);
struct connection_closure *connection_closure_alloc(int fd, bool tls, bool relay_only, struct sudo_event_base *base);

/* logsrvd_conf.c */
bool logsrvd_conf_read(const char *path);
const char *logsrvd_conf_iolog_dir(void);
const char *logsrvd_conf_iolog_file(void);
bool logsrvd_conf_iolog_log_passwords(void);
void *logsrvd_conf_iolog_passprompt_regex(void);
struct server_address_list *logsrvd_conf_server_listen_address(void);
struct server_address_list *logsrvd_conf_relay_address(void);
const char *logsrvd_conf_relay_dir(void);
bool logsrvd_conf_relay_store_first(void);
bool logsrvd_conf_relay_tcp_keepalive(void);
bool logsrvd_conf_server_tcp_keepalive(void);
const char *logsrvd_conf_pid_file(void);
struct timespec *logsrvd_conf_server_timeout(void);
struct timespec *logsrvd_conf_relay_connect_timeout(void);
struct timespec *logsrvd_conf_relay_timeout(void);
time_t logsrvd_conf_relay_retry_interval(void);
#if defined(HAVE_OPENSSL)
bool logsrvd_conf_server_tls_check_peer(void);
SSL_CTX *logsrvd_server_tls_ctx(void);
bool logsrvd_conf_relay_tls_check_peer(void);
SSL_CTX *logsrvd_relay_tls_ctx(void);
#endif
bool logsrvd_conf_log_exit(void);
uid_t logsrvd_conf_iolog_uid(void);
gid_t logsrvd_conf_iolog_gid(void);
mode_t logsrvd_conf_iolog_mode(void);
void address_list_addref(struct server_address_list *);
void address_list_delref(struct server_address_list *);
void logsrvd_conf_cleanup(void);
void logsrvd_warn_stderr(bool enabled);

/* logsrvd_journal.c */
extern struct client_message_switch cms_journal;

/* logsrvd_local.c */
extern struct client_message_switch cms_local;
bool set_random_drop(const char *dropstr);
bool store_accept_local(AcceptMessage *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_reject_local(RejectMessage *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_exit_local(ExitMessage *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_restart_local(RestartMessage *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_alert_local(AlertMessage *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_iobuf_local(int iofd, IoBuffer *iobuf, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_winsize_local(ChangeWindowSize *msg, uint8_t *buf, size_t len, struct connection_closure *closure);
bool store_suspend_local(CommandSuspend *msg, uint8_t *buf, size_t len, struct connection_closure *closure);

/* logsrvd_queue.c */
bool logsrvd_queue_enable(time_t timeout, struct sudo_event_base *evbase);
bool logsrvd_queue_insert(struct connection_closure *closure);
bool logsrvd_queue_scan(struct sudo_event_base *evbase);
void logsrvd_queue_dump(void);

/* logsrvd_relay.c */
extern struct client_message_switch cms_relay;
void relay_closure_free(struct relay_closure *relay_closure);
bool connect_relay(struct connection_closure *closure);
bool relay_shutdown(struct connection_closure *closure);

#endif /* SUDO_LOGSRVD_H */
