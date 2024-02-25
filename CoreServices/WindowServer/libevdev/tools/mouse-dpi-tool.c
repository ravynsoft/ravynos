// SPDX-License-Identifier: MIT
/*
 * Copyright © 2014 Red Hat, Inc.
 */

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <poll.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "libevdev/libevdev.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

static int signalled = 0;

struct measurements {
	int distance;
	double max_frequency;
	double *frequencies;
	size_t frequencies_sz;
	size_t nfrequencies;
	uint64_t us;
};

static int
usage(const char *progname) {
	printf("Usage: %s /dev/input/event0\n", progname);
	printf("\n");
	printf("This tool reads relative events from the kernel and calculates\n"
	       "the distance covered and maximum frequency of the incoming events.\n"
	       "Some mouse devices provide dynamic frequencies, it is\n"
	       "recommended to measure multiple times to obtain the highest value.\n");
	return 1;
}

static inline double
get_frequency(uint64_t last, uint64_t current)
{
	return 1000000.0/(current - last);
}

static inline void
push_frequency(struct measurements *m, double freq)
{
	if (m->nfrequencies == m->frequencies_sz) {
		m->frequencies_sz += 100;
		m->frequencies = realloc(m->frequencies,
					 m->frequencies_sz * sizeof *m->frequencies);
		if (!m->frequencies)
			abort();
	}

	m->frequencies[m->nfrequencies] = freq;
	m->nfrequencies++;
}

static int
print_current_values(const struct measurements *m)
{
	static int progress = 0;
	char status = 0;

	switch (progress) {
		case 0: status = '|'; break;
		case 1: status = '/'; break;
		case 2: status = '-'; break;
		case 3: status = '\\'; break;
		default:
			status = '?';
			break;
	}

	progress = (progress + 1) % 4;

	printf("\rCovered distance in device units: %8d at frequency %3.1fHz 	%c",
	       abs(m->distance), m->max_frequency, status);

	return 0;
}

static int
handle_event(struct measurements *m, const struct input_event *ev)
{
	if (ev->type == EV_SYN) {
		const int idle_reset = 3000000; /* us */
		uint64_t last_us = m->us;

		m->us = ev->input_event_sec * 1000000 + ev->input_event_usec;

		/* reset after pause */
		if (last_us + idle_reset < m->us) {
			m->max_frequency = 0.0;
			m->distance = 0;
		} else {
			double freq = get_frequency(last_us, m->us);
			push_frequency(m, freq);
			m->max_frequency = max(freq, m->max_frequency);
			return print_current_values(m);
		}

		return 0;
	}

	if (ev->type != EV_REL)
		return 0;

	switch(ev->code) {
		case REL_X:
			m->distance += ev->value;
			break;
	}

	return 0;
}

static void
signal_handler(__attribute__((__unused__)) int signal)
{
	signalled++;
}

static int
mainloop(struct libevdev *dev, struct measurements *m) {
	struct pollfd fds;

	fds.fd = libevdev_get_fd(dev);
	fds.events = POLLIN;

	signal(SIGINT, signal_handler);

	while (poll(&fds, 1, -1)) {
		struct input_event ev;
		int rc;

		if (signalled)
			break;

		do {
			rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
			if (rc == LIBEVDEV_READ_STATUS_SYNC) {
				fprintf(stderr, "Error: cannot keep up\n");
				return 1;
			}

			if (rc != -EAGAIN && rc < 0) {
				fprintf(stderr, "Error: %s\n", strerror(-rc));
				return 1;
			}

			if (rc == LIBEVDEV_READ_STATUS_SUCCESS)
				handle_event(m, &ev);
		} while (rc != -EAGAIN);
	}

	return 0;
}

static inline double
mean_frequency(struct measurements *m)
{
	int idx;

	idx = m->nfrequencies/2;
	return m->frequencies[idx];
}

static inline const char*
bustype(int bustype)
{
	const char *bus;

	switch(bustype) {
		case BUS_PCI: bus = "pci"; break;
		case BUS_ISAPNP: bus = "isapnp"; break;
		case BUS_USB: bus = "usb"; break;
		case BUS_HIL: bus = "hil"; break;
		case BUS_BLUETOOTH: bus = "bluetooth"; break;
		case BUS_VIRTUAL: bus = "virtual"; break;
		default: bus = "unknown bus type"; break;
	}

	return bus;
}

static void
print_summary(struct libevdev *dev, struct measurements *m)
{
	int res;
	int max_freq, mean_freq;

	if (m->nfrequencies == 0) {
		fprintf(stderr, "Error: no matching events received.\n");
		return;
	}

	max_freq = (int)m->max_frequency;
	mean_freq = (int)mean_frequency(m);

	printf("Estimated sampling frequency: %dHz (mean %dHz)\n",
	       max_freq, mean_freq);

	if (max_freq > mean_freq * 1.3)
		printf("WARNING: Max frequency is more than 30%% higher "
		       "than mean frequency. Manual verification required!\n");

	printf("To calculate resolution, measure physical distance covered\n"
	       "and look up the matching resolution in the table below\n");

	m->distance = abs(m->distance);

	/* If the mouse has more than 2500dpi, the manufacturer usually
	   shows off on their website anyway */
	for (res = 400; res <= 2500; res += 200) {
		double inch = m->distance/(double)res;
		printf("%8dmm	%8.2fin	%8ddpi\n",
		       (int)(inch * 25.4), inch, res);
	}
	printf("If your resolution is not in the list, calculate it with:\n"
	       "\tresolution=%d/inches, or\n"
	       "\tresolution=%d * 25.4/mm\n", m->distance, m->distance);

	printf("\n");
	printf("Entry for hwdb match (replace XXX with the resolution in DPI):\n"
	       "mouse:%s:v%04xp%04x:name:%s:\n"
	       " MOUSE_DPI=XXX@%d\n",
	       bustype(libevdev_get_id_bustype(dev)),
	       libevdev_get_id_vendor(dev),
	       libevdev_get_id_product(dev),
	       libevdev_get_name(dev),
	       (int)m->max_frequency);
}

int
main (int argc, char **argv) {
	int rc;
	int fd;
	const char *path;
	struct libevdev *dev;
	struct measurements measurements = {0};

	if (argc < 2)
		return usage(basename(argv[0]));

	path = argv[1];
	if (path[0] == '-')
		return usage(basename(argv[0]));

	fd = open(path, O_RDONLY|O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "Error opening the device: %s\n", strerror(errno));
		return 1;
	}

	rc = libevdev_new_from_fd(fd, &dev);
	if (rc != 0) {
		fprintf(stderr, "Error fetching the device info: %s\n", strerror(-rc));
		return 1;
	}

	if (libevdev_grab(dev, LIBEVDEV_GRAB) != 0) {
		fprintf(stderr, "Error: cannot grab the device, something else is grabbing it.\n");
		fprintf(stderr, "Use 'fuser -v %s' to find processes with an open fd\n", path);
		return 1;
	}
	libevdev_grab(dev, LIBEVDEV_UNGRAB);

	printf("Mouse %s on %s\n", libevdev_get_name(dev), path);
	printf("Move the device 250mm/10in or more along the x-axis.\n");
	printf("Pause 3 seconds before movement to reset, Ctrl+C to exit.\n");
	setbuf(stdout, NULL);

	rc = mainloop(dev, &measurements);

	printf("\n");

	print_summary(dev, &measurements);

	libevdev_free(dev);
	close(fd);

	return rc;
}
