#ifndef _SEATD_CONNECTION_H
#define _SEATD_CONNECTION_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CONNECTION_BUFFER_SIZE 256

#define MAX_FDS (CONNECTION_BUFFER_SIZE / sizeof(int))

struct connection_buffer {
	uint32_t head, tail;
	char data[CONNECTION_BUFFER_SIZE];
};

struct connection {
	struct connection_buffer in, out;
	struct connection_buffer fds_in, fds_out;
	int fd;
	bool want_flush;
};

int connection_read(struct connection *connection);
int connection_flush(struct connection *connection);

int connection_put(struct connection *connection, const void *data, size_t count);
int connection_put_fd(struct connection *connection, int fd);

size_t connection_pending(struct connection *connection);
int connection_get(struct connection *connection, void *dst, size_t count);
int connection_get_fd(struct connection *connection, int *fd);
void connection_restore(struct connection *connection, size_t count);

void connection_close_fds(struct connection *connection);

#endif
