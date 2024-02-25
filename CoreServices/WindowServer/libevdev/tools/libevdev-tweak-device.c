// SPDX-License-Identifier: MIT
/*
 * Copyright © 2014 Red Hat, Inc.
 */

#include "config.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <limits.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "libevdev/libevdev.h"

static void
usage(const char *progname)
{
	printf("%s --abs <axis> [--min min] [--max max] [--res res] [--fuzz fuzz] [--flat flat] /dev/input/eventXYZ\n"
	       "\tChange the absinfo struct for the named axis\n"
	       "%s --resolution res[,yres] /dev/input/eventXYZ\n"
	       "\tChange the x/y resolution on the given device\n"
	       "%s --led <led> --on|--off /dev/input/eventXYZ\n"
	       "\tEnable or disable the named LED\n",
	       progname,
	       progname,
	       progname);
}

enum mode {
	MODE_NONE = 0,
	MODE_ABS,
	MODE_LED,
	MODE_RESOLUTION,
	MODE_HELP,
};

enum opts {
	OPT_ABS = 1 << 0,
	OPT_MIN = 1 << 1,
	OPT_MAX = 1 << 2,
	OPT_FUZZ = 1 << 3,
	OPT_FLAT = 1 << 4,
	OPT_RES = 1 << 5,
	OPT_LED = 1 << 6,
	OPT_ON = 1 << 7,
	OPT_OFF = 1 << 8,
	OPT_RESOLUTION = 1 << 9,
	OPT_HELP = 1 << 10,
};

static bool
parse_resolution_argument(const char *arg, int *xres, int *yres)
{
	int matched;

	matched = sscanf(arg, "%d,%d", xres, yres);

	switch(matched) {
		case 2:
			break;
		case 1:
			*yres = *xres;
			break;
		default:
			return false;
	}

	return true;
}

static inline bool
safe_atoi(const char *str, int *val)
{
        char *endptr;
        long v;

        v = strtol(str, &endptr, 10);
        if (str == endptr)
                return false;
        if (*str != '\0' && *endptr != '\0')
                return false;

        if (v > INT_MAX || v < INT_MIN)
                return false;

        *val = v;
        return true;
}

static int
parse_event_code(int type, const char *str)
{
	int code;

	code = libevdev_event_code_from_name(type, str);
	if (code != -1)
		return code;

	if (safe_atoi(str, &code))
		return code;

	return -1;
}

static int
parse_options_abs(int argc, char **argv, unsigned int *changes,
		  int *axis, struct input_absinfo *absinfo)
{
	int rc = 1;
	int c;
	int option_index = 0;
	static struct option opts[] = {
		{ "abs", 1, 0, OPT_ABS },
		{ "min", 1, 0, OPT_MIN },
		{ "max", 1, 0, OPT_MAX },
		{ "fuzz", 1, 0, OPT_FUZZ },
		{ "flat", 1, 0, OPT_FLAT },
		{ "res", 1, 0, OPT_RES },
		{ NULL, 0, 0, 0 },
	};

	if (argc < 2)
		goto error;

	optind = 1;
	while (1) {
		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case OPT_ABS:
				*axis = parse_event_code(EV_ABS, optarg);
				if (*axis == -1)
					goto error;
				break;
			case OPT_MIN:
				absinfo->minimum = atoi(optarg);
				break;
			case OPT_MAX:
				absinfo->maximum = atoi(optarg);
				break;
			case OPT_FUZZ:
				absinfo->fuzz = atoi(optarg);
				break;
			case OPT_FLAT:
				absinfo->flat = atoi(optarg);
				break;
			case OPT_RES:
				absinfo->resolution = atoi(optarg);
				break;
			default:
				goto error;
		}
		*changes |= c;
	}
	rc = 0;
error:
	return rc;
}

static int
parse_options_led(int argc, char **argv, int *led, int *led_state)
{
	int rc = 1;
	int c;
	int option_index = 0;
	static struct option opts[] = {
		{ "led", 1, 0, OPT_LED },
		{ "on", 0, 0, OPT_ON },
		{ "off", 0, 0, OPT_OFF },
		{ NULL, 0, 0, 0 },
	};

	if (argc < 2)
		goto error;

	optind = 1;
	while (1) {
		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case OPT_LED:
				*led = parse_event_code(EV_LED, optarg);
				if (*led == -1)
					goto error;
				break;
			case OPT_ON:
				if (*led_state != -1)
					goto error;
				*led_state = 1;
				break;
			case OPT_OFF:
				if (*led_state != -1)
					goto error;
				*led_state = 0;
				break;
			default:
				goto error;
		}
	}

	rc = 0;
error:
	return rc;
}

static int
parse_options_resolution(int argc, char **argv, int *xres, int *yres)
{
	int rc = 1;
	int c;
	int option_index = 0;
	static struct option opts[] = {
		{ "resolution", 1, 0, OPT_RESOLUTION },
		{ NULL, 0, 0, 0 },
	};

	if (argc < 2)
		goto error;

	optind = 1;
	while (1) {
		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case OPT_RESOLUTION:
				if (!parse_resolution_argument(optarg,
							       xres, yres))
					goto error;
				break;
			default:
				goto error;
		}
	}

	rc = 0;
error:
	return rc;
}

static enum mode
parse_options_mode(int argc, char **argv)
{
	int c;
	int option_index = 0;
	static const struct option opts[] = {
		{ "abs", 1, 0, OPT_ABS },
		{ "led", 1, 0, OPT_LED },
		{ "resolution", 1, 0, OPT_RESOLUTION },
		{ "help", 0, 0, OPT_HELP },
		{ NULL, 0, 0, 0 },
	};
	enum mode mode = MODE_NONE;

	if (argc < 2)
		return mode;

	while (mode == MODE_NONE) {
		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch (c) {
			case 'h':
			case OPT_HELP:
				mode = MODE_HELP;
				break;
			case OPT_ABS:
				mode = MODE_ABS;
				break;
			case OPT_LED:
				mode = MODE_LED;
				break;
			case OPT_RESOLUTION:
				mode = MODE_RESOLUTION;
				break;
			default:
				break;
		}
	}

	if (optind >= argc && mode != MODE_HELP)
		return MODE_NONE;

	return mode;
}

static void
set_abs(struct libevdev *dev, unsigned int changes,
	unsigned int axis, struct input_absinfo *absinfo)
{
	int rc;
	struct input_absinfo abs;
	const struct input_absinfo *a;

	if ((a = libevdev_get_abs_info(dev, axis)) == NULL) {
		fprintf(stderr,
			"Device '%s' doesn't have axis %s\n",
			libevdev_get_name(dev),
			libevdev_event_code_get_name(EV_ABS, axis));
		return;
	}

	abs = *a;
	if (changes & OPT_MIN)
		abs.minimum = absinfo->minimum;
	if (changes & OPT_MAX)
		abs.maximum = absinfo->maximum;
	if (changes & OPT_FUZZ)
		abs.fuzz = absinfo->fuzz;
	if (changes & OPT_FLAT)
		abs.flat = absinfo->flat;
	if (changes & OPT_RES)
		abs.resolution = absinfo->resolution;

	rc = libevdev_kernel_set_abs_info(dev, axis, &abs);
	if (rc != 0)
		fprintf(stderr,
			"Failed to set absinfo %s: %s",
			libevdev_event_code_get_name(EV_ABS, axis),
			strerror(-rc));
}

static void
set_led(struct libevdev *dev, unsigned int led, int led_state)
{
	int rc;
	enum libevdev_led_value state =
		led_state ? LIBEVDEV_LED_ON : LIBEVDEV_LED_OFF;

	if (!libevdev_has_event_code(dev, EV_LED, led)) {
		fprintf(stderr,
			"Device '%s' doesn't have %s\n",
			libevdev_get_name(dev),
			libevdev_event_code_get_name(EV_LED, led));
		return;
	}

	rc = libevdev_kernel_set_led_value(dev, led, state);
	if (rc != 0)
		fprintf(stderr,
			"Failed to set LED %s: %s",
			libevdev_event_code_get_name(EV_LED, led),
			strerror(-rc));
}

static void
set_resolution(struct libevdev *dev, int xres, int yres)
{
	struct input_absinfo abs;

	abs.resolution = xres;
	if (libevdev_has_event_code(dev, EV_ABS, ABS_X))
		set_abs(dev, OPT_RES, ABS_X, &abs);
	if (libevdev_has_event_code(dev, EV_ABS, ABS_MT_POSITION_X))
		set_abs(dev, OPT_RES, ABS_MT_POSITION_X, &abs);

	abs.resolution = yres;
	if (libevdev_has_event_code(dev, EV_ABS, ABS_Y))
		set_abs(dev, OPT_RES, ABS_Y, &abs);
	if (libevdev_has_event_code(dev, EV_ABS, ABS_MT_POSITION_Y))
		set_abs(dev, OPT_RES, ABS_MT_POSITION_Y, &abs);
}

int
main(int argc, char **argv)
{
	struct libevdev *dev = NULL;
	int fd = -1;
	int rc = EXIT_FAILURE;
	enum mode mode;
	const char *path;
	struct input_absinfo absinfo;
	int axis = -1;
	int led = -1;
	int led_state = -1;
	unsigned int changes = 0; /* bitmask of changes */
	int xres = 0,
	    yres = 0;

	mode = parse_options_mode(argc, argv);
	switch (mode) {
		case MODE_HELP:
			rc = EXIT_SUCCESS;
			/* fallthrough */
		case MODE_NONE:
			usage(basename(argv[0]));
			goto out;
		case MODE_ABS:
			rc = parse_options_abs(argc, argv, &changes, &axis,
					       &absinfo);
			break;
		case MODE_LED:
			rc = parse_options_led(argc, argv, &led, &led_state);
			break;
		case MODE_RESOLUTION:
			rc = parse_options_resolution(argc, argv, &xres,
						      &yres);
			break;
		default:
			fprintf(stderr,
				"++?????++ Out of Cheese Error. Redo From Start.\n");
			goto out;
	}

	if (rc != EXIT_SUCCESS)
		goto out;

	if (optind >= argc) {
		rc = EXIT_FAILURE;
		usage(basename(argv[0]));
		goto out;
	}

	path = argv[optind];

	fd = open(path, O_RDWR);
	if (fd < 0) {
		rc = EXIT_FAILURE;
		perror("Failed to open device");
		goto out;
	}

	rc = libevdev_new_from_fd(fd, &dev);
	if (rc < 0) {
		fprintf(stderr, "Failed to init libevdev (%s)\n", strerror(-rc));
		goto out;
	}

	switch (mode) {
		case MODE_ABS:
			set_abs(dev, changes, axis, &absinfo);
			break;
		case MODE_LED:
			set_led(dev, led, led_state);
			break;
		case MODE_RESOLUTION:
			set_resolution(dev, xres, yres);
			break;
		default:
			break;
	}

out:
	libevdev_free(dev);
	if (fd != -1)
		close(fd);

	return rc;
}
