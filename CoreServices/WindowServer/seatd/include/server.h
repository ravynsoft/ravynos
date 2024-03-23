#ifndef _SEATD_SERVER_H
#define _SEATD_SERVER_H

#include <stdbool.h>

#include "linked_list.h"
#include "poller.h"

struct client;

struct server {
	bool running;
	struct poller poller;

	struct linked_list seats;
	struct linked_list idle_clients;
};

int server_init(struct server *server);
void server_finish(struct server *server);

struct seat *server_get_seat(struct server *server, const char *seat_name);

int server_handle_connection(int fd, uint32_t mask, void *data);
int server_add_client(struct server *server, int fd);

#endif
