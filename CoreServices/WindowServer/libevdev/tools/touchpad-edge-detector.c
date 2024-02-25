// SPDX-License-Identifier: MIT
/*
 * Copyright © 2014 Red Hat, Inc.
 */

#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <math.h>
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

static int
usage(const char *progname) {
	printf("Usage: %s 12x34 /dev/input/eventX\n", progname);
	printf("\n");
	printf("This tool reads the touchpad events from the kernel and calculates\n "
	       "the minimum and maximum for the x and y coordinates, respectively.\n"
	       "The first argument is the physical size of the touchpad in mm (WIDTHxHEIGHT).\n");
	return 1;
}

struct dimensions {
	int top, bottom, left, right;
};

struct size {
	int w, h;
};

static int
print_current_values(const struct dimensions *d)
{
	static int progress;
	char status = 0;

	switch (progress) {
		case 0: status = '|'; break;
		case 1: status = '/'; break;
		case 2: status = '-'; break;
		case 3: status = '\\'; break;
	}

	progress = (progress + 1) % 4;

	printf("\rTouchpad sends:	x [%d..%d], y [%d..%d] %c",
			d->left, d->right, d->top, d->bottom, status);
	return 0;
}

static int
handle_event(struct dimensions *d, const struct input_event *ev) {
	if (ev->type == EV_SYN)
		return print_current_values(d);

	if (ev->type != EV_ABS)
		return 0;

	switch(ev->code) {
		case ABS_X:
		case ABS_MT_POSITION_X:
			d->left = min(d->left, ev->value);
			d->right = max(d->right, ev->value);
			break;
		case ABS_Y:
		case ABS_MT_POSITION_Y:
			d->top = min(d->top, ev->value);
			d->bottom = max(d->bottom, ev->value);
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
mainloop(struct libevdev *dev, struct dimensions *dim) {
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
				handle_event(dim, &ev);
		} while (rc != -EAGAIN);
	}

	return 0;
}

static inline void
pid_vid_matchstr(struct libevdev *dev, char *match, size_t sz)
{
	snprintf(match, sz, "input:b%04Xv%04Xp%04X",
		libevdev_get_id_bustype(dev),
		libevdev_get_id_vendor(dev),
		libevdev_get_id_product(dev));
}

static inline void
dmi_matchstr(struct libevdev *dev, char *match, size_t sz)
{
	char modalias[PATH_MAX];
	FILE *fp;

	fp = fopen("/sys/class/dmi/id/modalias", "r");
	if (!fp || fgets(modalias, sizeof(modalias), fp) == NULL) {
		sprintf(match, "ERROR READING DMI MODALIAS");
		if (fp)
			fclose(fp);
		return;
	}

	fclose(fp);

	modalias[strlen(modalias) - 1] = '\0'; /* drop \n */
	snprintf(match, sz, "name:%s:%s", libevdev_get_name(dev), modalias);
}

static void
print_udev_override_rule(struct libevdev *dev,
			 const struct dimensions *dim,
			 const struct size *size) {
	const struct input_absinfo *x, *y;
	char match[PATH_MAX];
	int w, h;
	int xres, yres;

	x = libevdev_get_abs_info(dev, ABS_X);
	y = libevdev_get_abs_info(dev, ABS_Y);
	w = dim->right - dim->left;
	h = dim->bottom - dim->top;
	xres = round((double)w/size->w);
	yres = round((double)h/size->h);

	if (x->resolution && y->resolution) {
		int width = x->maximum - x->minimum,
		    height = y->maximum - y->minimum;
		printf("Touchpad size as listed by the kernel: %dx%dmm\n",
		       width/x->resolution, height/y->resolution);
	} else {
		printf("Touchpad has no resolution, size unknown\n");
	}

	printf("User-specified touchpad size: %dx%dmm\n", size->w, size->h);
	printf("Calculated ranges: %d/%d\n", w, h);
	printf("\n");
	printf("Suggested udev rule:\n");

	switch(libevdev_get_id_bustype(dev)) {
	case BUS_USB:
	case BUS_BLUETOOTH:
		pid_vid_matchstr(dev, match, sizeof(match));
		break;
	default:
		dmi_matchstr(dev, match, sizeof(match));
		break;
	}

	printf("# <Laptop model description goes here>\n"
	       "evdev:%s*\n"
	       " EVDEV_ABS_00=%d:%d:%d\n"
	       " EVDEV_ABS_01=%d:%d:%d\n",
	       match,
	       dim->left, dim->right, xres,
	       dim->top, dim->bottom, yres);
	if (libevdev_has_event_code(dev, EV_ABS, ABS_MT_POSITION_X))
		printf(" EVDEV_ABS_35=%d:%d:%d\n"
		       " EVDEV_ABS_36=%d:%d:%d\n",
		       dim->left, dim->right, xres,
		       dim->top, dim->bottom, yres);
}

int main (int argc, char **argv) {
	int rc;
	int fd;
	const char *path;
	struct libevdev *dev;
	struct dimensions dim;
	struct size size;

	if (argc < 3)
		return usage(basename(argv[0]));

	if (sscanf(argv[1], "%dx%d", &size.w, &size.h) != 2 ||
	    size.w <= 0 || size.h <= 0)
		return usage(basename(argv[0]));

	if (size.w < 30 || size.h < 30) {
		fprintf(stderr,
			"%dx%dmm is too small for a touchpad.\n"
			"Please specify the touchpad size in mm.\n",
			size.w, size.h);
		return 1;
	}

	path = argv[2];
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

	if (!libevdev_has_event_code(dev, EV_ABS, ABS_X) ||
	    !libevdev_has_event_code(dev, EV_ABS, ABS_Y)) {
		fprintf(stderr, "Error: this device does not have abs axes\n");
		rc = EXIT_FAILURE;
		goto out;
	}

	dim.left = INT_MAX;
	dim.right = INT_MIN;
	dim.top = INT_MAX;
	dim.bottom = INT_MIN;

	printf("Touchpad %s on %s\n", libevdev_get_name(dev), path);
	printf("Move one finger around the touchpad to detect the actual edges\n");
	printf("Kernel says:	x [%d..%d], y [%d..%d]\n",
			libevdev_get_abs_minimum(dev, ABS_X),
			libevdev_get_abs_maximum(dev, ABS_X),
			libevdev_get_abs_minimum(dev, ABS_Y),
			libevdev_get_abs_maximum(dev, ABS_Y));

	setbuf(stdout, NULL);

	rc = mainloop(dev, &dim);
	printf("\n\n");

	print_udev_override_rule(dev, &dim, &size);

out:
	libevdev_free(dev);
	close(fd);

	return rc;
}
