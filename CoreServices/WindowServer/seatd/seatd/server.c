#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "client.h"
#include "log.h"
#include "poller.h"
#include "seat.h"
#include "server.h"
#include "terminal.h"

static int server_handle_vt_acq(int signal, void *data);
static int server_handle_vt_rel(int signal, void *data);
static int server_handle_kill(int signal, void *data);

int server_init(struct server *server) {
	if (poller_init(&server->poller) == -1) {
		log_errorf("could not initialize poller: %s", strerror(errno));
		return -1;
	}

	linked_list_init(&server->seats);
	linked_list_init(&server->idle_clients);

	if (poller_add_signal(&server->poller, SIGUSR1, server_handle_vt_rel, server) == NULL ||
	    poller_add_signal(&server->poller, SIGUSR2, server_handle_vt_acq, server) == NULL ||
	    poller_add_signal(&server->poller, SIGINT, server_handle_kill, server) == NULL ||
	    poller_add_signal(&server->poller, SIGTERM, server_handle_kill, server) == NULL) {
		server_finish(server);
		return -1;
	}

	char *vtenv = getenv("SEATD_VTBOUND");

	// TODO: create more seats:
	struct seat *seat = seat_create("seat0", vtenv == NULL || strcmp(vtenv, "1") == 0);
	if (seat == NULL) {
		server_finish(server);
		return -1;
	}

	linked_list_insert(&server->seats, &seat->link);
	server->running = true;
	return 0;
}

void server_finish(struct server *server) {
	assert(server);
	while (!linked_list_empty(&server->idle_clients)) {
		struct client *client = (struct client *)server->idle_clients.next;
		client_destroy(client);
	}
	while (!linked_list_empty(&server->seats)) {
		struct seat *seat = (struct seat *)server->seats.next;
		seat_destroy(seat);
	}
	poller_finish(&server->poller);
}

struct seat *server_get_seat(struct server *server, const char *seat_name) {
	for (struct linked_list *elem = server->seats.next; elem != &server->seats;
	     elem = elem->next) {
		struct seat *seat = (struct seat *)elem;
		if (strcmp(seat->seat_name, seat_name) == 0) {
			return seat;
		}
	}
	return NULL;
}

static int server_handle_vt_acq(int signal, void *data) {
	(void)signal;
	struct server *server = data;
	struct seat *seat = server_get_seat(server, "seat0");
	if (seat == NULL) {
		return -1;
	}

	seat_vt_activate(seat);
	return 0;
}

static int server_handle_vt_rel(int signal, void *data) {
	(void)signal;
	struct server *server = data;
	struct seat *seat = server_get_seat(server, "seat0");
	if (seat == NULL) {
		return -1;
	}

	seat_vt_release(seat);
	return 0;
}

static int server_handle_kill(int signal, void *data) {
	(void)signal;
	struct server *server = data;
	server->running = false;
	return 0;
}

static int set_nonblock(int fd) {
	int flags;
	if ((flags = fcntl(fd, F_GETFD)) == -1 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
		log_errorf("Could not set FD_CLOEXEC on socket: %s", strerror(errno));
		return -1;
	}
	if ((flags = fcntl(fd, F_GETFL)) == -1 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		log_errorf("Could not set O_NONBLOCK on socket: %s", strerror(errno));
		return -1;
	}
	return 0;
}

int server_add_client(struct server *server, int fd) {
	if (set_nonblock(fd) != 0) {
		log_errorf("Could not prepare new client socket: %s", strerror(errno));
		close(fd);
		return -1;
	}

	struct client *client = client_create(server, fd);
	if (client == NULL) {
		log_errorf("Could not create client: %s", strerror(errno));
		close(fd);
		return -1;
	}

	client->event_source =
		poller_add_fd(&server->poller, fd, EVENT_READABLE, client_handle_connection, client);
	if (client->event_source == NULL) {
		log_errorf("Could not add client socket to poller: %s", strerror(errno));
		client_destroy(client);
		return -1;
	}
	log_infof("New client connected (pid: %d, uid: %d, gid: %d)", client->pid, client->uid,
		  client->gid);
	return 0;
}

int server_handle_connection(int fd, uint32_t mask, void *data) {
	struct server *server = data;
	if (mask & (EVENT_ERROR | EVENT_HANGUP)) {
		shutdown(fd, SHUT_RDWR);
		server->running = false;
		log_error("Server socket received an error");
		return -1;
	}

	if (mask & EVENT_READABLE) {
		int new_fd = accept(fd, NULL, NULL);
		if (fd == -1) {
			log_errorf("Could not accept client connection: %s", strerror(errno));
			return 0;
		}

		if (server_add_client(server, new_fd) == -1) {
			return 0;
		}
	}
	return 0;
}
