/*
 * Copyright © 2017 Red Hat, Inc.
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

#include <getopt.h>
#include <stdio.h>

#include <libinput-version.h>

#include "shared.h"

static void
usage(void)
{
	printf("Usage: libinput [--help|--version] <command> [<args>]\n"
	       "\n"
	       "Global options:\n"
	       "  --help ...... show this help and exit\n"
	       "  --version ... show version information and exit\n"
	       "\n"
	       "Commands:\n"
	       "  list-devices\n"
	       "	List all devices with their default configuration options\n"
	       "\n"
	       "  debug-events\n"
	       "	Print events to stdout\n"
	       "\n"
#if HAVE_DEBUG_GUI
	       "  debug-gui\n"
	       "	Display a simple GUI to visualize libinput's events.\n"
	       "\n"
#endif
	       "  measure <feature>\n"
	       "	Measure various device properties. See the man page for more info\n"
	       "\n"
	       "  analyze <feature>\n"
	       "	Analyze device events. See the man page for more info\n"
	       "\n"
	       "  record\n"
	       "	Record event stream from a device node. See the man page for more info\n"
	       "\n"
	       "  replay\n"
	       "	Replay a previously recorded event stream. See the man page for more info\n"
	       "\n");
}

enum global_opts {
	GOPT_HELP = 1,
	GOPT_VERSION,
};

int
main(int argc, char **argv)
{
	int option_index = 0;

	while (1) {
		int c;
		static struct option opts[] = {
			{ "help",	no_argument,	0, GOPT_HELP },
			{ "version",	no_argument,	0, GOPT_VERSION },
			{ 0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "+h", opts, &option_index);
		if (c == -1)
			break;

		switch(c) {
		case 'h':
		case GOPT_HELP:
			usage();
			return EXIT_SUCCESS;
		case GOPT_VERSION:
			printf("%s\n", LIBINPUT_VERSION);
			return EXIT_SUCCESS;
		default:
			usage();
			return EXIT_INVALID_USAGE;
		}
	}

	if (optind >= argc) {
		usage();
		return EXIT_INVALID_USAGE;
	}

	argv += optind;
	argc -= optind;

	return tools_exec_command("libinput", argc, argv);
}
