#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libseat.h"

static void handle_enable(struct libseat *backend, void *data) {
	(void)backend;
	int *active = (int *)data;
	(*active)++;
}

static void handle_disable(struct libseat *backend, void *data) {
	(void)backend;
	int *active = (int *)data;
	(*active)--;

	libseat_disable_seat(backend);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Specify name of file to open as argument\n");
		return -1;
	}
	char *file = argv[1];

	int active = 0;
	struct libseat_seat_listener listener = {
		.enable_seat = handle_enable,
		.disable_seat = handle_disable,
	};
	libseat_set_log_level(LIBSEAT_LOG_LEVEL_DEBUG);
	struct libseat *backend = libseat_open_seat(&listener, &active);
	fprintf(stderr, "libseat_open_seat(listener: %p, userdata: %p) = %p\n", (void *)&listener,
		(void *)&active, (void *)backend);
	if (backend == NULL) {
		fprintf(stderr, "libseat_open_seat() failed: %s\n", strerror(errno));
		return -1;
	}

	while (active == 0) {
		fprintf(stderr, "waiting for activation...\n");
		if (libseat_dispatch(backend, -1) == -1) {
			libseat_close_seat(backend);
			fprintf(stderr, "libseat_dispatch() failed: %s\n", strerror(errno));
			return -1;
		}
	}
	fprintf(stderr, "active!\n");

	int fd, device;
	device = libseat_open_device(backend, file, &fd);
	fprintf(stderr, "libseat_open_device(backend: %p, path: %s, fd: %p) = %d\n",
		(void *)backend, file, (void *)&fd, device);
	if (device == -1) {
		fprintf(stderr, "libseat_open_device() failed: %s\n", strerror(errno));
		libseat_close_seat(backend);
		return 1;
	}

	libseat_close_device(backend, device);
	close(fd);
	libseat_close_seat(backend);
	return 0;
}
