/*
 * Copyright © 2019 Red Hat, Inc.
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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <getopt.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <libinput.h>
#include <libevdev/libevdev.h>

#include "shared.h"
#include "util-macros.h"
#include "util-input-event.h"

static volatile sig_atomic_t stop = 0;
static struct tools_options options;
static int termwidth = 78;

struct context {
	struct libinput *libinput;
	struct libinput_device *device;
	struct libinput_tablet_tool *tool;
	struct libevdev *evdev;

	/* fd[0] ... libinput fd
	   fd[1] ... libevdev fd */
	struct pollfd fds[2];

	/* libinput device state */
	bool tip_is_down;
	double x, y;
	double x_norm, y_norm;
	double tx, ty;
	double dist, pressure;
	double rotation, slider;
	unsigned int buttons_down[32];
	unsigned int evdev_buttons_down[32];

	/* libevdev device state */
	struct {
		int x, y, z;
		int tilt_x, tilt_y;
		int distance, pressure;
	} abs;
};

LIBINPUT_ATTRIBUTE_PRINTF(1, 2)
static void
print_line(const char *format, ...)
{
	char empty[] = "                                                                                ";
	const int width = 80;
	int n;
	va_list args;

	printf("\r");

	va_start(args, format);
	n = vprintf(format, args);
	va_end(args);
	printf("%.*s\n", width - n, empty);
}

static void
print_buttons(struct context *ctx, unsigned int *buttons, size_t sz)
{
	char buf[256] = {0};
	size_t len = 0;

	for (size_t i = 0; i < sz; i++) {
		const char *name;

		if (buttons[i] == 0)
			continue;

		name = libevdev_event_code_get_name(EV_KEY, buttons[i]);
		len += snprintf(&buf[len], sizeof(buf) - len, "%s ", name);
	}
	print_line("  buttons: %s", buf);
}

static void
print_bar(const char *header, double value, double normalized)
{
	char empty[termwidth];
	bool oob = false;
	/* the bar is minimum 10 chars, otherwise 78 or whatever fits.
	   32 is the manually-added up length of the prefix + [|] */
	const int width = max(10, min(78, termwidth - 32));
	int left_pad, right_pad;

	memset(empty, '-', sizeof empty);

	if (normalized < 0.0 || normalized > 1.0) {
		normalized = min(max(normalized, 0.0), 1.0);
		oob = true;
	}

	left_pad = width * normalized + 0.5;
	right_pad = width - left_pad;

	printf("\r  %s%-16s %8.2f [%.*s|%.*s]%s\n",
	       oob ? ANSI_RED : "",
	       header,
	       value,
	       left_pad, empty,
	       right_pad, empty,
	       oob ? ANSI_NORMAL : "");
}

static double
normalize(struct libevdev *evdev, int code, int value)
{
	const struct input_absinfo *abs;

	if (!evdev)
		return 0.0;

	abs = libevdev_get_abs_info(evdev, code);

	if (!abs)
		return 0.0;

	return 1.0 * (value - abs->minimum)/absinfo_range(abs);
}

static int
print_state(struct context *ctx)
{
	const char *tool_str;
	double w, h;
	int lines_printed = 0;

	if (!ctx->device) {
		print_line(ANSI_RED "No device connected" ANSI_NORMAL);
		lines_printed++;
	} else {
		libinput_device_get_size(ctx->device, &w, &h);
		print_line("Device: %s (%s)",
			   libinput_device_get_name(ctx->device),
			   libinput_device_get_sysname(ctx->device));
		lines_printed++;
	}

	if (!ctx->tool) {
		print_line(ANSI_RED "No tool in proximity " ANSI_NORMAL);
		lines_printed++;
	} else {
		switch (libinput_tablet_tool_get_type(ctx->tool)) {
		case LIBINPUT_TABLET_TOOL_TYPE_PEN:
			tool_str = "pen";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_ERASER:
			tool_str = "eraser";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_BRUSH:
			tool_str = "brush";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_PENCIL:
			tool_str = "pencil";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_AIRBRUSH:
			tool_str = "airbrush";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_MOUSE:
			tool_str = "mouse";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_LENS:
			tool_str = "lens";
			break;
		case LIBINPUT_TABLET_TOOL_TYPE_TOTEM:
			tool_str = "totem";
			break;
		default:
			abort();
		}

		printf("\rTool: %s serial %#" PRIx64 ", id %#" PRIx64 "\n",
		       tool_str,
		       libinput_tablet_tool_get_serial(ctx->tool),
		       libinput_tablet_tool_get_tool_id(ctx->tool));
		lines_printed++;
	}
	printf("libinput:\n");
	print_line("tip: %s", ctx->tip_is_down ? "down" : "up");
	print_bar("x:", ctx->x, ctx->x_norm);
	print_bar("y:", ctx->y, ctx->y_norm);
	print_bar("tilt x:", ctx->tx, (ctx->tx + 90)/180);
	print_bar("tilt y:", ctx->ty, (ctx->ty + 90)/180);
	print_bar("dist:", ctx->dist, ctx->dist);
	print_bar("pressure:", ctx->pressure, ctx->pressure);
	print_bar("rotation:", ctx->rotation, ctx->rotation/360.0);
	print_bar("slider:", ctx->slider, (ctx->slider + 1.0)/2.0);
	print_buttons(ctx,
		      ctx->buttons_down,
		      ARRAY_LENGTH(ctx->buttons_down));
	lines_printed += 11;

	printf("evdev:\n");
	print_bar("ABS_X:", ctx->abs.x, normalize(ctx->evdev, ABS_X, ctx->abs.x));
	print_bar("ABS_Y:", ctx->abs.y, normalize(ctx->evdev, ABS_Y, ctx->abs.y));
	print_bar("ABS_Z:", ctx->abs.z, normalize(ctx->evdev, ABS_Z, ctx->abs.z));
	print_bar("ABS_TILT_X:", ctx->abs.tilt_x, normalize(ctx->evdev, ABS_TILT_X, ctx->abs.tilt_x));
	print_bar("ABS_TILT_Y:", ctx->abs.tilt_y, normalize(ctx->evdev, ABS_TILT_Y, ctx->abs.tilt_y));
	print_bar("ABS_DISTANCE:", ctx->abs.distance, normalize(ctx->evdev, ABS_DISTANCE, ctx->abs.distance));
	print_bar("ABS_PRESSURE:", ctx->abs.pressure, normalize(ctx->evdev, ABS_PRESSURE, ctx->abs.pressure));
	print_buttons(ctx,
		      ctx->evdev_buttons_down,
		      ARRAY_LENGTH(ctx->evdev_buttons_down));
	lines_printed += 9;

	return lines_printed;
}

static void
handle_device_added(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_device *device = libinput_event_get_device(ev);
	struct udev_device *udev_device;
	const char *devnode;

	if (ctx->device)
		return;

	if (!libinput_device_has_capability(device, LIBINPUT_DEVICE_CAP_TABLET_TOOL))
		return;

	ctx->device = libinput_device_ref(device);

	udev_device = libinput_device_get_udev_device(device);
	if (!udev_device)
		return;

	devnode = udev_device_get_devnode(udev_device);
	if (devnode) {
		int fd = open(devnode, O_RDONLY|O_NONBLOCK);
		assert(fd != -1);
		assert(libevdev_new_from_fd(fd, &ctx->evdev) == 0);
	}

	udev_device_unref(udev_device);
}

static void
handle_device_removed(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_device *device = libinput_event_get_device(ev);

	if (ctx->device != device)
		return;

	libinput_device_unref(ctx->device);
	ctx->device = NULL;

	libevdev_free(ctx->evdev);
	ctx->evdev = NULL;

	close(ctx->fds[1].fd);
	ctx->fds[1].fd = -1;
}

static void
update_tablet_axes(struct context *ctx, struct libinput_event_tablet_tool *t)
{
	ctx->x = libinput_event_tablet_tool_get_x(t);
	ctx->y = libinput_event_tablet_tool_get_y(t);
	ctx->x_norm = libinput_event_tablet_tool_get_x_transformed(t, 1.0);
	ctx->y_norm = libinput_event_tablet_tool_get_y_transformed(t, 1.0);
	ctx->tx = libinput_event_tablet_tool_get_tilt_x(t);
	ctx->ty = libinput_event_tablet_tool_get_tilt_y(t);
	ctx->dist = libinput_event_tablet_tool_get_distance(t);
	ctx->pressure = libinput_event_tablet_tool_get_pressure(t);
	ctx->rotation = libinput_event_tablet_tool_get_rotation(t);
	ctx->slider = libinput_event_tablet_tool_get_slider_position(t);
}

static void
handle_tablet_button_event(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);
	unsigned int button = libinput_event_tablet_tool_get_button(t);
	enum libinput_button_state state = libinput_event_tablet_tool_get_button_state(t);

	ARRAY_FOR_EACH(ctx->buttons_down, btn) {
		if (state == LIBINPUT_BUTTON_STATE_PRESSED) {
		    if (*btn == 0) {
				*btn = button;
				break;
		    }
		} else {
			if (*btn == button) {
				*btn = 0;
				break;
			}
		}
	}
}

static void
handle_tablet_axis_event(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);

	update_tablet_axes(ctx, t);
}

static void
handle_tablet_proximity_event(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);
	struct libinput_tablet_tool *tool = libinput_event_tablet_tool_get_tool(t);

	if (ctx->tool) {
		libinput_tablet_tool_unref(ctx->tool);
		ctx->tool = NULL;
	}

	if (libinput_event_tablet_tool_get_proximity_state(t) == LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_IN)
		ctx->tool = libinput_tablet_tool_ref(tool);
}

static void
handle_tablet_tip_event(struct context *ctx, struct libinput_event *ev)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);

	ctx->tip_is_down = libinput_event_tablet_tool_get_tip_state(t) == LIBINPUT_TABLET_TOOL_TIP_DOWN;
}

static void
handle_libinput_events(struct context *ctx)
{
	struct libinput *li = ctx->libinput;
	struct libinput_event *ev;

	libinput_dispatch(li);
	while ((ev = libinput_get_event(li))) {
		switch (libinput_event_get_type(ev)) {
		case LIBINPUT_EVENT_NONE:
			abort();
		case LIBINPUT_EVENT_DEVICE_ADDED:
			handle_device_added(ctx, ev);
			tools_device_apply_config(libinput_event_get_device(ev),
						  &options);
			break;
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			handle_device_removed(ctx, ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
			handle_tablet_button_event(ctx, ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
			handle_tablet_axis_event(ctx, ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
			handle_tablet_proximity_event(ctx, ev);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_TIP:
			handle_tablet_tip_event(ctx, ev);
			break;
		default:
			break;
		}

		libinput_event_destroy(ev);
	}
}

static void
handle_libevdev_events(struct context *ctx)
{
	struct libevdev *evdev = ctx->evdev;
	struct input_event event;

#define evbit(_t, _c) (((_t) << 16) | (_c))

	if (!evdev)
		return;

	while (libevdev_next_event(evdev, LIBEVDEV_READ_FLAG_NORMAL, &event)
	       == LIBEVDEV_READ_STATUS_SUCCESS)
	{
		switch(evbit(event.type, event.code)) {
		case evbit(EV_KEY, BTN_TOOL_PEN):
		case evbit(EV_KEY, BTN_TOOL_RUBBER):
		case evbit(EV_KEY, BTN_TOOL_BRUSH):
		case evbit(EV_KEY, BTN_TOOL_PENCIL):
		case evbit(EV_KEY, BTN_TOOL_AIRBRUSH):
		case evbit(EV_KEY, BTN_TOOL_MOUSE):
		case evbit(EV_KEY, BTN_TOOL_LENS):
			ctx->evdev_buttons_down[event.code - BTN_TOOL_PEN] = event.value ?  event.code : 0;
			break;
		/* above tools should be mutually exclusive but let's leave
		 * enough space */
		case evbit(EV_KEY, BTN_TOUCH):
			ctx->evdev_buttons_down[7] = event.value ? event.code : 0;
			break;
		case evbit(EV_KEY, BTN_STYLUS):
			ctx->evdev_buttons_down[8] = event.value ? event.code : 0;
			break;
		case evbit(EV_KEY, BTN_STYLUS2):
			ctx->evdev_buttons_down[9] = event.value ? event.code : 0;
			break;
		case evbit(EV_KEY, BTN_STYLUS3):
			ctx->evdev_buttons_down[10] = event.value ? event.code : 0;
			break;
		case evbit(EV_ABS, ABS_X):
			ctx->abs.x = event.value;
			break;
		case evbit(EV_ABS, ABS_Y):
			ctx->abs.y = event.value;
			break;
		case evbit(EV_ABS, ABS_Z):
			ctx->abs.z = event.value;
			break;
		case evbit(EV_ABS, ABS_PRESSURE):
			ctx->abs.pressure = event.value;
			break;
		case evbit(EV_ABS, ABS_TILT_X):
			ctx->abs.tilt_x = event.value;
			break;
		case evbit(EV_ABS, ABS_TILT_Y):
			ctx->abs.tilt_y = event.value;
			break;
		case evbit(EV_ABS, ABS_DISTANCE):
			ctx->abs.distance = event.value;
			break;
		}
	}
}

static void
sighandler(int signal, siginfo_t *siginfo, void *userdata)
{
	stop = 1;
}

static void
mainloop(struct context *ctx)
{
	unsigned int lines_printed = 20;

	ctx->fds[0].fd = libinput_get_fd(ctx->libinput);

	/* pre-load the lines */
	for (unsigned int i = 0; i < lines_printed; i++)
		printf("\n");

	do {
		handle_libinput_events(ctx);
		handle_libevdev_events(ctx);

		printf(ANSI_LEFT, 1000);
		printf(ANSI_UP, lines_printed);
		lines_printed = print_state(ctx);
	} while (!stop && poll(ctx->fds, 2, -1) > -1);

	printf("\n");
}

static void
usage(void) {
	printf("Usage: libinput debug-tablet [options] [--udev <seat>|--device /dev/input/event0]\n");
}

static void
init_context(struct context *ctx)
{

	memset(ctx, 0, sizeof *ctx);

	ctx->fds[0].fd = -1; /* libinput fd */
	ctx->fds[0].events = POLLIN;
	ctx->fds[0].revents = 0;
	ctx->fds[1].fd = -1; /* libevdev fd */
	ctx->fds[1].events = POLLIN;
	ctx->fds[1].revents = 0;
}

int
main(int argc, char **argv)
{
	struct context ctx;
	struct libinput *li;
	enum tools_backend backend = BACKEND_NONE;
	const char *seat_or_device[2] = {"seat0", NULL};
	struct sigaction act;
	bool grab = false;

	init_context(&ctx);

	tools_init_options(&options);

	while (1) {
		int c;
		int option_index = 0;
		enum {
			OPT_DEVICE = 1,
			OPT_UDEV,
		};
		static struct option opts[] = {
			CONFIGURATION_OPTIONS,
			{ "help",                      no_argument,       0, 'h' },
			{ "device",                    required_argument, 0, OPT_DEVICE },
			{ "udev",                      required_argument, 0, OPT_UDEV },
			{ 0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "h", opts, &option_index);
		if (c == -1)
			break;

		switch(c) {
		case '?':
			exit(EXIT_INVALID_USAGE);
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
			break;
		case OPT_DEVICE:
			backend = BACKEND_DEVICE;
			seat_or_device[0] = optarg;
			break;
		case OPT_UDEV:
			backend = BACKEND_UDEV;
			seat_or_device[0] = optarg;
			break;
		}

	}

	if (optind < argc) {
		if (optind < argc - 1 || backend != BACKEND_NONE) {
			usage();
			return EXIT_INVALID_USAGE;
		}
		backend = BACKEND_DEVICE;
		seat_or_device[0] = argv[optind];
	} else if (backend == BACKEND_NONE) {
		backend = BACKEND_UDEV;
	}

	memset(&act, 0, sizeof(act));
	act.sa_sigaction = sighandler;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGINT, &act, NULL) == -1) {
		fprintf(stderr, "Failed to set up signal handling (%s)\n",
				strerror(errno));
		return EXIT_FAILURE;
	}

	li = tools_open_backend(backend, seat_or_device, false, &grab);
	if (!li)
		return EXIT_FAILURE;

	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
		termwidth = w.ws_col;

	ctx.libinput = li;
	mainloop(&ctx);

	libinput_unref(li);

	return EXIT_SUCCESS;
}
