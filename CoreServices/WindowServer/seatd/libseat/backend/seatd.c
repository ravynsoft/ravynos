#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include "backend.h"
#include "connection.h"
#include "libseat.h"
#include "linked_list.h"
#include "log.h"
#include "protocol.h"

#ifdef BUILTIN_ENABLED
#include "poller.h"
#include "server.h"
#endif

const struct seat_impl seatd_impl;
const struct seat_impl builtin_impl;

struct pending_event {
	struct linked_list link; // backend_seat::link
	int opcode;
};

struct backend_seatd {
	struct libseat base;
	struct connection connection;
	const struct libseat_seat_listener *seat_listener;
	void *seat_listener_data;
	struct linked_list pending_events;
	bool awaiting_pong;
	bool error;

	char seat_name[MAX_SEAT_LEN];
};

static int seatd_connect(void) {
	union {
		struct sockaddr_un unix;
		struct sockaddr generic;
	} addr = {{0}};
	int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd == -1) {
		log_errorf("Could not create socket: %s", strerror(errno));
		return -1;
	}
	const char *path = getenv("SEATD_SOCK");
	if (path == NULL) {
		path = SEATD_DEFAULTPATH;
	}
	addr.unix.sun_family = AF_UNIX;
	strncpy(addr.unix.sun_path, path, sizeof addr.unix.sun_path - 1);
	socklen_t size = offsetof(struct sockaddr_un, sun_path) + strlen(addr.unix.sun_path);
	if (connect(fd, &addr.generic, size) == -1) {
		if (errno == ENOENT) {
			log_infof("Could not connect to socket %s: %s", path, strerror(errno));
		} else {
			log_errorf("Could not connect to socket %s: %s", path, strerror(errno));
		}
		close(fd);
		return -1;
	};
	return fd;
}

static struct backend_seatd *backend_seatd_from_libseat_backend(struct libseat *base) {
	assert(base);
#ifdef BUILTIN_ENABLED
	assert(base->impl == &seatd_impl || base->impl == &builtin_impl);
#else
	assert(base->impl == &seatd_impl);
#endif
	return (struct backend_seatd *)base;
}

static void cleanup(struct backend_seatd *backend) {
	if (backend->connection.fd != -1) {
		close(backend->connection.fd);
		backend->connection.fd = -1;
	}
	connection_close_fds(&backend->connection);
	while (!linked_list_empty(&backend->pending_events)) {
		struct pending_event *ev = (struct pending_event *)backend->pending_events.next;
		linked_list_remove(&ev->link);
		free(ev);
	}
}

static void destroy(struct backend_seatd *backend) {
	cleanup(backend);
	free(backend);
}

static void set_error(struct backend_seatd *backend) {
	if (backend->error) {
		return;
	}

	backend->error = true;
	cleanup(backend);
}

static inline int conn_put(struct backend_seatd *backend, const void *data, const size_t data_len) {
	if (connection_put(&backend->connection, data, data_len) == -1) {
		log_errorf("Could not send request: %s", strerror(errno));
		set_error(backend);
		return -1;
	}
	return 0;
}

static inline int conn_flush(struct backend_seatd *backend) {
	if (connection_flush(&backend->connection) == -1) {
		log_errorf("Could not flush connection: %s", strerror(errno));
		set_error(backend);
		return -1;
	}
	return 0;
}

static inline int conn_get(struct backend_seatd *backend, void *target, const size_t target_len) {
	if (connection_get(&backend->connection, target, target_len) == -1) {
		log_error("Invalid message: insufficient data received");
		set_error(backend);
		errno = EBADMSG;
		return -1;
	}
	return 0;
}

static inline int conn_get_fd(struct backend_seatd *backend, int *fd) {
	if (connection_get_fd(&backend->connection, fd) == -1) {
		log_error("Invalid message: insufficient data received");
		set_error(backend);
		errno = EBADMSG;
		return -1;
	}
	return 0;
}

static size_t read_header(struct backend_seatd *backend, uint16_t expected_opcode,
			  size_t expected_size, bool variable) {
	struct proto_header header;
	if (conn_get(backend, &header, sizeof header) == -1) {
		set_error(backend);
		return SIZE_MAX;
	}
	if (header.opcode != expected_opcode) {
		struct proto_server_error msg;
		if (header.opcode != SERVER_ERROR) {
			log_errorf("Unexpected response: expected opcode %d, received opcode %d",
				   expected_opcode, header.opcode);
			set_error(backend);
			errno = EBADMSG;
		} else if (header.size != sizeof msg || conn_get(backend, &msg, sizeof msg) == -1) {
			set_error(backend);
			errno = EBADMSG;
		} else {
			errno = msg.error_code;
		}
		return SIZE_MAX;
	}

	if ((!variable && header.size != expected_size) || (variable && header.size < expected_size)) {
		log_errorf("Invalid message: does not match expected size: variable: %d, header.size: %d, expected size: %zd",
			   variable, header.size, expected_size);
		set_error(backend);
		errno = EBADMSG;
		return SIZE_MAX;
	}
	return header.size;
}

static int queue_event(struct backend_seatd *backend, int opcode) {
	struct pending_event *ev = calloc(1, sizeof(struct pending_event));
	if (ev == NULL) {
		log_errorf("Allocation failed: %s", strerror(errno));
		return -1;
	}

	ev->opcode = opcode;
	linked_list_insert(&backend->pending_events, &ev->link);
	return 0;
}

static int execute_events(struct backend_seatd *backend) {
	struct linked_list list;
	linked_list_init(&list);
	linked_list_take(&list, &backend->pending_events);
	int executed = 0;
	while (!linked_list_empty(&list)) {
		struct pending_event *ev = (struct pending_event *)list.next;
		int opcode = ev->opcode;
		linked_list_remove(&ev->link);
		free(ev);

		switch (opcode) {
		case SERVER_DISABLE_SEAT:
			log_info("Disabling seat");
			backend->seat_listener->disable_seat(&backend->base,
							     backend->seat_listener_data);
			break;
		case SERVER_ENABLE_SEAT:
			log_info("Enabling seat");
			backend->seat_listener->enable_seat(&backend->base,
							    backend->seat_listener_data);
			break;
		default:
			log_errorf("Invalid opcode: %d", opcode);
			abort();
		}
		executed++;
	}
	return executed;
}

static int dispatch_pending(struct backend_seatd *backend, int *opcode) {
	int packets = 0;
	struct proto_header header;
	while (connection_get(&backend->connection, &header, sizeof header) != -1) {
		packets++;
		switch (header.opcode) {
		case SERVER_PONG:
			// We care about whether or not the answer has been
			// read from the connection, so handle it here instead
			// of pushing it to the pending event list.
			backend->awaiting_pong = false;
			break;
		case SERVER_DISABLE_SEAT:
		case SERVER_ENABLE_SEAT:
			if (queue_event(backend, header.opcode) == -1) {
				set_error(backend);
				return -1;
			}
			break;
		default:
			if (opcode != NULL &&
			    connection_pending(&backend->connection) >= header.size) {
				*opcode = header.opcode;
			}
			connection_restore(&backend->connection, sizeof header);
			return packets;
		}
	}
	return packets;
}

static int dispatch_pending_and_execute(struct backend_seatd *backend) {
	int dispatched = dispatch_pending(backend, NULL);
	if (dispatched == -1) {
		return -1;
	}
	dispatched += execute_events(backend);
	return dispatched;
}

static int poll_connection(struct backend_seatd *backend, int timeout) {
	struct pollfd fd = {
		.fd = backend->connection.fd,
		.events = POLLIN,
	};

	if (poll(&fd, 1, timeout) == -1) {
		return (errno == EAGAIN || errno == EINTR) ? 0 : -1;
	}

	if (fd.revents & (POLLERR | POLLHUP)) {
		errno = EPIPE;
		return -1;
	}

	int len = 0;
	if (fd.revents & POLLIN) {
		len = connection_read(&backend->connection);
		if (len == 0) {
			errno = EIO;
			return -1;
		} else if (len == -1 && errno != EAGAIN) {
			return -1;
		}
	}

	return len;
}

static int dispatch(struct backend_seatd *backend) {
	if (conn_flush(backend) == -1) {
		return -1;
	}
	while (true) {
		int opcode = 0;
		if (dispatch_pending(backend, &opcode) == -1) {
			log_errorf("Could not dispatch pending messages: %s", strerror(errno));
			return -1;
		}
		if (opcode != 0) {
			break;
		}
		if (poll_connection(backend, -1) == -1) {
			log_errorf("Could not poll connection: %s", strerror(errno));
			return -1;
		}
	}
	return 0;
}

static int get_fd(struct libseat *base) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	return backend->connection.fd;
}

static int dispatch_and_execute(struct libseat *base, int timeout) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	if (backend->error) {
		errno = ENOTCONN;
		return -1;
	}

	int predispatch = dispatch_pending_and_execute(backend);
	if (predispatch == -1) {
		return -1;
	}

	// We don't want to block if we dispatched something, as the
	// caller might be waiting for the result. However, we'd also
	// like to read anything pending.
	int read = 0;
	if (predispatch > 0 || timeout == 0) {
		read = connection_read(&backend->connection);
	} else {
		read = poll_connection(backend, timeout);
	}

	if (read == 0) {
		return predispatch;
	} else if (read == -1 && errno != EAGAIN) {
		log_errorf("Could not read from connection: %s", strerror(errno));
		return -1;
	}

	int postdispatch = dispatch_pending_and_execute(backend);
	if (postdispatch == -1) {
		return -1;
	}

	return predispatch + postdispatch;
}

static struct libseat *_open_seat(const struct libseat_seat_listener *listener, void *data, int fd) {
	assert(listener != NULL);
	assert(listener->enable_seat != NULL && listener->disable_seat != NULL);
	struct backend_seatd *backend = calloc(1, sizeof(struct backend_seatd));
	if (backend == NULL) {
		log_errorf("Allocation failed: %s", strerror(errno));
		goto alloc_error;
	}

	backend->seat_listener = listener;
	backend->seat_listener_data = data;
	backend->connection.fd = fd;
	backend->base.impl = &seatd_impl;
	linked_list_init(&backend->pending_events);

	struct proto_header header = {
		.opcode = CLIENT_OPEN_SEAT,
		.size = 0,
	};
	if (conn_put(backend, &header, sizeof header) == -1 || dispatch(backend) == -1) {
		goto backend_error;
	}

	struct proto_server_seat_opened rmsg;
	size_t size = read_header(backend, SERVER_SEAT_OPENED, sizeof rmsg, true);
	if (size == SIZE_MAX || conn_get(backend, &rmsg, sizeof rmsg) == -1) {
		goto backend_error;
	}
	if (rmsg.seat_name_len != size - sizeof rmsg) {
		log_errorf("Invalid message: seat_name_len does not match remaining message size (%d != %zd)",
			   rmsg.seat_name_len, size);
		errno = EBADMSG;
		goto backend_error;
	}
	if (conn_get(backend, backend->seat_name, rmsg.seat_name_len) == -1) {
		goto backend_error;
	}

	execute_events(backend);
	return &backend->base;

backend_error:
	destroy(backend);
alloc_error:
	close(fd);
	return NULL;
}

static struct libseat *open_seat(const struct libseat_seat_listener *listener, void *data) {
	int fd = seatd_connect();
	if (fd == -1) {
		return NULL;
	}

	return _open_seat(listener, data, fd);
}

static int close_seat(struct libseat *base) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);

	struct proto_header header = {
		.opcode = CLIENT_CLOSE_SEAT,
		.size = 0,
	};
	if (conn_put(backend, &header, sizeof header) == -1 || dispatch(backend) == -1) {
		goto error;
	}

	if (read_header(backend, SERVER_SEAT_CLOSED, 0, false) == SIZE_MAX) {
		goto error;
	}

	execute_events(backend);
	destroy(backend);
	return 0;

error:
	execute_events(backend);
	destroy(backend);
	return -1;
}

static const char *seat_name(struct libseat *base) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	return backend->seat_name;
}

static int send_ping(struct backend_seatd *backend) {
	struct proto_header header = {
		.opcode = CLIENT_PING,
		.size = 0,
	};
	if (conn_put(backend, &header, sizeof header) == -1 || conn_flush(backend) == -1) {
		return -1;
	}
	return 0;
}

static void check_pending_events(struct backend_seatd *backend) {
	if (linked_list_empty(&backend->pending_events)) {
		return;
	}
	if (backend->awaiting_pong) {
		return;
	}

	// We have events pending execution, so a dispatch is required.
	// However, we likely already drained our socket, so there will not be
	// anything to read. Instead, send a ping request to seatd, so that the
	// user will be woken up by its response.
	if (send_ping(backend) == -1) {
		log_errorf("Could not send ping request: %s", strerror(errno));
		return;
	}
	backend->awaiting_pong = true;
}

static int open_device(struct libseat *base, const char *path, int *fd) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	if (backend->error) {
		errno = ENOTCONN;
		return -1;
	}
	size_t pathlen = strlen(path) + 1;
	if (pathlen > MAX_PATH_LEN) {
		errno = EINVAL;
		return -1;
	}

	struct proto_client_open_device msg = {
		.path_len = (uint16_t)pathlen,
	};
	struct proto_header header = {
		.opcode = CLIENT_OPEN_DEVICE,
		.size = sizeof msg + pathlen,
	};
	if (conn_put(backend, &header, sizeof header) == -1 ||
	    conn_put(backend, &msg, sizeof msg) == -1 || conn_put(backend, path, pathlen) == -1 ||
	    dispatch(backend) == -1) {
		goto error;
	}

	struct proto_server_device_opened rmsg;
	if (read_header(backend, SERVER_DEVICE_OPENED, sizeof rmsg, false) == SIZE_MAX ||
	    conn_get(backend, &rmsg, sizeof rmsg) == -1 || conn_get_fd(backend, fd)) {
		goto error;
	}

	check_pending_events(backend);
	return rmsg.device_id;

error:
	check_pending_events(backend);
	return -1;
}

static int close_device(struct libseat *base, int device_id) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	if (backend->error) {
		errno = ENOTCONN;
		return -1;
	}
	if (device_id < 0) {
		errno = EINVAL;
		return -1;
	}

	struct proto_client_close_device msg = {
		.device_id = device_id,
	};
	struct proto_header header = {
		.opcode = CLIENT_CLOSE_DEVICE,
		.size = sizeof msg,
	};
	if (conn_put(backend, &header, sizeof header) == -1 ||
	    conn_put(backend, &msg, sizeof msg) == -1 || dispatch(backend) == -1) {
		goto error;
	}

	if (read_header(backend, SERVER_DEVICE_CLOSED, 0, false) == SIZE_MAX) {
		goto error;
	}

	check_pending_events(backend);
	return 0;

error:
	check_pending_events(backend);
	return -1;
}

static int switch_session(struct libseat *base, int session) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	if (backend->error) {
		errno = ENOTCONN;
		return -1;
	}
	if (session < 0) {
		return -1;
	}

	struct proto_client_switch_session msg = {
		.session = session,
	};
	struct proto_header header = {
		.opcode = CLIENT_SWITCH_SESSION,
		.size = sizeof msg,
	};
	if (conn_put(backend, &header, sizeof header) == -1 ||
	    conn_put(backend, &msg, sizeof msg) == -1 || conn_flush(backend) == -1) {
		return -1;
	}

	return 0;
}

static int disable_seat(struct libseat *base) {
	struct backend_seatd *backend = backend_seatd_from_libseat_backend(base);
	if (backend->error) {
		errno = ENOTCONN;
		return -1;
	}
	struct proto_header header = {
		.opcode = CLIENT_DISABLE_SEAT,
		.size = 0,
	};
	if (conn_put(backend, &header, sizeof header) == -1 || conn_flush(backend) == -1) {
		return -1;
	}

	return 0;
}

const struct seat_impl seatd_impl = {
	.open_seat = open_seat,
	.disable_seat = disable_seat,
	.close_seat = close_seat,
	.seat_name = seat_name,
	.open_device = open_device,
	.close_device = close_device,
	.switch_session = switch_session,
	.get_fd = get_fd,
	.dispatch = dispatch_and_execute,
};

#ifdef BUILTIN_ENABLED

static struct libseat *builtin_open_seat(const struct libseat_seat_listener *listener, void *data) {
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds) == -1) {
		log_errorf("Could not create socket pair: %s", strerror(errno));
		return NULL;
	}

	pid_t pid = fork();
	if (pid == -1) {
		log_errorf("Could not fork: %s", strerror(errno));
		close(fds[0]);
		close(fds[1]);
		return NULL;
	} else if (pid == 0) {
		close(fds[1]);
		int fd = fds[0];
		int res = 0;
		struct server server = {0};
		if (server_init(&server) == -1) {
			log_errorf("Could not init embedded seatd server: %s", strerror(errno));
			res = 1;
			goto error;
		}
		if (server_add_client(&server, fd) == -1) {
			log_errorf("Could not add client to embedded seatd server: %s",
				   strerror(errno));
			res = 1;
			goto server_error;
		}
		log_info("Started embedded seatd");
		while (server.running) {
			if (poller_poll(&server.poller) == -1) {
				log_errorf("Could not poll server socket: %s", strerror(errno));
				res = 1;
				break;
			}
		}
	server_error:
		server_finish(&server);
	error:
		close(fd);
		log_info("Stopped embedded seatd");
		exit(res);
	} else {
		close(fds[0]);
		int fd = fds[1];
		return _open_seat(listener, data, fd);
	}
}

const struct seat_impl builtin_impl = {
	.open_seat = builtin_open_seat,
	.disable_seat = disable_seat,
	.close_seat = close_seat,
	.seat_name = seat_name,
	.open_device = open_device,
	.close_device = close_device,
	.switch_session = switch_session,
	.get_fd = get_fd,
	.dispatch = dispatch_and_execute,
};
#endif
