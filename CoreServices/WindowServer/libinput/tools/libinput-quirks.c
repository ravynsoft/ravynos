/*
 * Copyright © 2018 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>

#include "quirks.h"
#include "shared.h"
#include "builddir.h"

static bool verbose = false;

LIBINPUT_ATTRIBUTE_PRINTF(3, 0)
static void
log_handler(struct libinput *this_is_null,
	    enum libinput_log_priority priority,
	    const char *format,
	    va_list args)
{
	FILE *out = stdout;
	enum quirks_log_priorities p = (enum quirks_log_priorities)priority;
	char buf[256] = {0};
	const char *prefix = NULL;

	switch (p) {
	case QLOG_NOISE:
	case QLOG_DEBUG:
		if (!verbose)
			return;
		prefix = "quirks debug";
		break;
	case QLOG_INFO:
		prefix = "quirks info";
		break;
	case QLOG_ERROR:
		out = stderr;
		prefix = "quirks error";
		break;
	case QLOG_PARSER_ERROR:
		out = stderr;
		prefix = "quirks parser error";
		break;
	}

	snprintf(buf, sizeof(buf), "%s: %s", prefix, format);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
	vfprintf(out, buf, args);
#pragma GCC diagnostic pop
}

static void
usage(void)
{
	printf("Usage:\n"
	       "  libinput quirks list [--data-dir /path/to/quirks/dir] /dev/input/event0\n"
	       "	Print the quirks for the given device\n"
	       "\n"
	       "  libinput quirks validate [--data-dir /path/to/quirks/dir]\n"
	       "	Validate the database\n");
}

static void
simple_printf(void *userdata, const char *val)
{
	printf("%s\n", val);
}

int
main(int argc, char **argv)
{
	struct udev *udev = NULL;
	struct udev_device *device = NULL;
	const char *path;
	const char *data_path = NULL,
	           *override_file = NULL;
	int rc = 1;
	struct quirks_context *quirks;
	bool validate = false;

	while (1) {
		int c;
		int option_index = 0;
		enum {
			OPT_VERBOSE,
			OPT_DATADIR,
		};
		static struct option opts[] = {
			{ "help",     no_argument,       0, 'h' },
			{ "verbose",  no_argument,       0, OPT_VERBOSE },
			{ "data-dir", required_argument, 0, OPT_DATADIR },
			{ 0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch(c) {
		case '?':
			exit(1);
			break;
		case 'h':
			usage();
			exit(0);
			break;
		case OPT_VERBOSE:
			verbose = true;
			break;
		case OPT_DATADIR:
			data_path = optarg;
			break;
		default:
			usage();
			return 1;
		}
	}

	if (optind >= argc) {
		usage();
		return 1;
	}

	if (streq(argv[optind], "list")) {
		optind++;
		if (optind >= argc) {
			usage();
			return 1;
		}
	} else if (streq(argv[optind], "validate")) {
		optind++;
		if (optind < argc) {
			usage();
			return 1;
		}
		validate = true;
	} else {
		fprintf(stderr, "Unnkown action '%s'\n", argv[optind]);
		return 1;
	}

	/* Overriding the data dir means no custom override file */
	if (!data_path) {
		char *builddir = builddir_lookup();
		if (builddir) {
			data_path = LIBINPUT_QUIRKS_SRCDIR;
			free(builddir);
		} else {
			data_path = LIBINPUT_QUIRKS_DIR;
			override_file = LIBINPUT_QUIRKS_OVERRIDE_FILE;
		}
	}

	quirks = quirks_init_subsystem(data_path,
				      override_file,
				      log_handler,
				      NULL,
				      QLOG_CUSTOM_LOG_PRIORITIES);
	if (!quirks) {
		fprintf(stderr,
			"Failed to initialize the device quirks. "
			"Please see the above errors "
			"and/or re-run with --verbose for more details\n");
		return 1;
	}

	if (validate) {
		rc = 0;
		goto out;
	}

	udev = udev_new();
	if (!udev)
		goto out;

	path = argv[optind];
	if (strneq(path, "/sys/", 5)) {
		device = udev_device_new_from_syspath(udev, path);
	} else {
		struct stat st;
		if (stat(path, &st) < 0) {
			fprintf(stderr, "Error: %s: %m\n", path);
			goto out;
		}

		device = udev_device_new_from_devnum(udev, 'c', st.st_rdev);
	}
	if (device) {
		tools_list_device_quirks(quirks, device, simple_printf, NULL);
		rc = 0;
	} else {
		usage();
		rc = 1;
	}

	udev_device_unref(device);
out:
	udev_unref(udev);

	quirks_context_unref(quirks);

	return rc;
}
