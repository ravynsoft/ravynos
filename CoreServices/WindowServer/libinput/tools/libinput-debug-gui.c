/*
 * Copyright © 2014 Red Hat, Inc.
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
#include <config.h>

#include <linux/input.h>

#include <assert.h>
#include <cairo.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <glib.h>
#include <glib-unix.h>
#include <libevdev/libevdev.h>

#include <libinput.h>
#include "util-strings.h"
#include "util-macros.h"
#include "util-list.h"

#include "shared.h"

#if HAVE_GTK_WAYLAND
	#include <wayland-client.h>
	#include "pointer-constraints-unstable-v1-client-protocol.h"
	#if HAVE_GTK4
		#include <gdk/wayland/gdkwayland.h>
	#else
		#include <gdk/gdkwayland.h>
	#endif
#endif

#if HAVE_GTK_X11
	#include <X11/X.h>
	#include <X11/Xlib.h>
	#if HAVE_GTK4
		#include <gdk/x11/gdkx.h>
	#else
		#include <gdk/gdkx.h>
	#endif
#endif

#define clip(val_, min_, max_) min((max_), max((min_), (val_)))

enum touch_state {
	TOUCH_ACTIVE,
	TOUCH_ENDED,
	TOUCH_CANCELLED,
};

struct touch {
	enum touch_state state;
	int x, y;
};

struct point {
	double x, y;
};

struct evdev_device {
	struct list node;
	struct libevdev *evdev;
	struct libinput_device *libinput_device;
	int fd;
	guint source_id;
};

struct window {
	bool grab;
	struct tools_options options;
	struct list evdev_devices;

	GMainLoop *event_loop;

	GtkWidget *win;
	GtkWidget *area;
	int width, height; /* of window */

	/* sprite position */
	struct point pointer;
	struct point unaccelerated;

	/* these are for the delta coordinates, but they're not
	 * deltas, they are converted into abs positions */
	size_t ndeltas;
	struct point deltas[64];

	/* abs position */
	struct point abs;

	/* Wayland and X11 pointer locking */
	struct {
		bool locked;

#if HAVE_GTK_WAYLAND
		struct zwp_pointer_constraints_v1 *wayland_pointer_constraints;
		struct zwp_locked_pointer_v1 *wayland_locked_pointer;
#endif
	} lock_pointer;

	/* scroll bar positions */
	struct {
		double vx, vy;
		double hx, hy;

		double vx_discrete, vy_discrete;
		double hx_discrete, hy_discrete;
	} scroll;

	/* touch positions */
	struct touch touches[32];

	/* l/m/r mouse buttons */
	struct {
		bool l, m, r;
		bool other;
		const char *other_name;
	} buttons;

	/* touchpad swipe */
	struct {
		int nfingers;
		double x, y;
	} swipe;

	struct {
		int nfingers;
		double scale;
		double angle;
		double x, y;
	} pinch;

	struct {
		int nfingers;
		bool active;
	} hold;

	struct {
		double x, y;
		double x_in, y_in;
		double x_down, y_down;
		double x_up, y_up;
		double pressure;
		double distance;
		double tilt_x, tilt_y;
		double rotation;
		double size_major, size_minor;
		bool is_down;

		/* these are for the delta coordinates, but they're not
		 * deltas, they are converted into abs positions */
		size_t ndeltas;
		struct point deltas[64];
	} tool;

	struct {
		struct {
			double position;
			int number;
		} ring;
		struct {
			double position;
			int number;
		} strip;
	} pad;

	struct {
		int rel_x, rel_y; /* REL_X/Y */
		int x, y;	  /* ABS_X/Y */
		struct {
			int x, y; /* ABS_MT_POSITION_X/Y */
			bool active;
		} slots[16];
		unsigned int slot; /* ABS_MT_SLOT */
		/* So we know when to re-fetch the abs axes */
		uintptr_t device, last_device;
	} evdev;

	struct libinput_device *devices[50];
};

#if HAVE_GTK_WAYLAND
static void
wayland_registry_global(void *data,
			struct wl_registry *registry,
			uint32_t name,
			const char *interface,
			uint32_t version)
{
	struct window *w = data;

	if (!g_strcmp0(interface, "zwp_pointer_constraints_v1")) {
		w->lock_pointer.wayland_pointer_constraints =
			wl_registry_bind(registry,
					 name,
					 &zwp_pointer_constraints_v1_interface,
					 1);
        }
}

static void
wayland_registry_global_remove(void *data,
			       struct wl_registry *wl_registry,
			       uint32_t name)
{

}

struct wl_registry_listener registry_listener = {
	wayland_registry_global,
	wayland_registry_global_remove
};

static bool
wayland_lock_pointer(struct window *w)
{
	GdkDisplay *gdk_display;
	GdkSeat *gdk_seat;
	GdkDevice *gdk_device;
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_pointer *wayland_pointer;
	struct wl_surface *surface;

	w->lock_pointer.wayland_pointer_constraints = NULL;

	gdk_display = gdk_display_get_default();
	display = gdk_wayland_display_get_wl_display(gdk_display);

	gdk_seat = gdk_display_get_default_seat(gdk_display);
	gdk_device = gdk_seat_get_pointer(gdk_seat);
	wayland_pointer = gdk_wayland_device_get_wl_pointer(gdk_device);

	registry = wl_display_get_registry(display);
	wl_registry_add_listener(registry, &registry_listener, w);
	wl_display_roundtrip(display);

	if (!w->lock_pointer.wayland_pointer_constraints)
		return false;

#if HAVE_GTK4
	GtkNative *window = gtk_widget_get_native(w->win);
	GdkSurface *gdk_surface = gtk_native_get_surface(window);
	surface = gdk_wayland_surface_get_wl_surface(gdk_surface);
#else
	GdkWindow *window = gtk_widget_get_window(w->win);
	surface = gdk_wayland_window_get_wl_surface(window);
#endif

	w->lock_pointer.wayland_locked_pointer =
		zwp_pointer_constraints_v1_lock_pointer(w->lock_pointer.wayland_pointer_constraints,
							surface,
							wayland_pointer,
							NULL,
							ZWP_POINTER_CONSTRAINTS_V1_LIFETIME_PERSISTENT);

	return true;
}

static void
wayland_unlock_pointer(struct window *w)
{
	w->lock_pointer.wayland_pointer_constraints = NULL;
	zwp_locked_pointer_v1_destroy(w->lock_pointer.wayland_locked_pointer);
}

static inline bool
backend_is_wayland(void)
{
	return GDK_IS_WAYLAND_DISPLAY(gdk_display_get_default());
}
#endif /* HAVE_GTK_WAYLAND */

#if HAVE_GTK_X11
static bool
x_lock_pointer(struct window *w)
{
	Display *x_display;
	Window x_win;
	int result;

	x_display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

#if HAVE_GTK4
	GtkNative *window = gtk_widget_get_native(w->win);
	GdkSurface *surface = gtk_native_get_surface(window);
	x_win = GDK_SURFACE_XID(surface);
#else
	GdkWindow *window = gtk_widget_get_window(w->win);
	x_win = GDK_WINDOW_XID(window);
#endif

	result = XGrabPointer(x_display, x_win,
			      False, NoEventMask,
			      GrabModeAsync, GrabModeAsync,
			      x_win,
			      None,
			      CurrentTime);
	return (result == GrabSuccess);
}

static void
x_unlock_pointer(struct window *w)
{
	Display *x_display;

	x_display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());

	XUngrabPointer(x_display, CurrentTime);
}

static inline bool
backend_is_x11(void)
{
	return GDK_IS_X11_DISPLAY(gdk_display_get_default());
}
#endif /* HAVE_GTK_X11 */

static bool
window_lock_pointer(struct window *w)
{
	if (w->lock_pointer.locked)
		return true;

#if HAVE_GTK_WAYLAND
	if (backend_is_wayland())
		w->lock_pointer.locked = wayland_lock_pointer(w);
#endif

#if HAVE_GTK_X11
	if (backend_is_x11())
		w->lock_pointer.locked = x_lock_pointer(w);
#endif

	return w->lock_pointer.locked;
}

static void
window_unlock_pointer(struct window *w)
{
	if (!w->lock_pointer.locked)
		return;

	w->lock_pointer.locked = false;

#if HAVE_GTK_WAYLAND
	if (backend_is_wayland())
		wayland_unlock_pointer(w);
#endif

#if HAVE_GTK_X11
	if (backend_is_x11())
		x_unlock_pointer(w);
#endif
}

LIBINPUT_ATTRIBUTE_PRINTF(1, 2)
static inline void
msg(const char *fmt, ...)
{
	va_list args;
	printf("info: ");

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

static inline void
draw_evdev_rel(struct window *w, cairo_t *cr)
{
	int center_x, center_y;

	cairo_save(cr);
	cairo_set_source_rgb(cr, .2, .2, .8);
	center_x = w->width/2 - 400;
	center_y = w->height/2;

	cairo_arc(cr, center_x, center_y, 10, 0, 2 * M_PI);
	cairo_stroke(cr);

	if (w->evdev.rel_x) {
		int dir = w->evdev.rel_x > 0 ? 1 : -1;
		for (int i = 0; i < abs(w->evdev.rel_x); i++) {
			cairo_move_to(cr,
				      center_x + (i + 1) * 20 * dir,
				      center_y - 20);
			cairo_rel_line_to(cr, 0, 40);
			cairo_rel_line_to(cr, 20 * dir, -20);
			cairo_rel_line_to(cr, -20 * dir, -20);
			cairo_fill(cr);
		}
        }

	if (w->evdev.rel_y) {
		int dir = w->evdev.rel_y > 0 ? 1 : -1;
		for (int i = 0; i < abs(w->evdev.rel_y); i++) {
			cairo_move_to(cr,
				      center_x - 20,
				      center_y + (i + 1) * 20 * dir);
			cairo_rel_line_to(cr, 40, 0);
			cairo_rel_line_to(cr, -20, 20 * dir);
			cairo_rel_line_to(cr, -20, -20 * dir);
			cairo_fill(cr);
		}
        }

	cairo_restore(cr);
}

static inline void
draw_evdev_abs(struct window *w, cairo_t *cr)
{
	static const struct input_absinfo *ax = NULL, *ay = NULL;
	const int normalized_width = 200;
	int outline_width = normalized_width,
	    outline_height = normalized_width * 0.75;
	int center_x, center_y;
	int width, height;
	int x, y;

	cairo_save(cr);

	center_x = w->width/2 + 400;
	center_y = w->height/2;

	/* Always the outline even if we didn't get any abs events yet so it
	 * doesn't just appear out of nowhere */
	if (w->evdev.device == 0)
		goto draw_outline;

	/* device has changed, so the abs proportions/dimensions have
	 * changed. */
	if (w->evdev.device != w->evdev.last_device) {
		struct evdev_device *d;

		ax = NULL;
		ay = NULL;

		list_for_each(d, &w->evdev_devices, node) {
			if ((uintptr_t)d->libinput_device != w->evdev.device)
				continue;

			ax = libevdev_get_abs_info(d->evdev, ABS_X);
			ay = libevdev_get_abs_info(d->evdev, ABS_Y);
			w->evdev.last_device = w->evdev.device;
		}

	}
	if (ax == NULL || ay == NULL)
		goto draw_outline;

	width = ax->maximum - ax->minimum;
	height = ay->maximum - ay->minimum;
	outline_height = 1.0 * height/width * normalized_width;
	outline_width = normalized_width;

	x = 1.0 * (w->evdev.x - ax->minimum)/width * outline_width;
	y = 1.0 * (w->evdev.y - ay->minimum)/height * outline_height;
	x += center_x - outline_width/2;
	y += center_y - outline_height/2;
	cairo_arc(cr, x, y, 10, 0, 2 * M_PI);
	cairo_fill(cr);

	for (size_t i = 0; i < ARRAY_LENGTH(w->evdev.slots); i++) {
		if (!w->evdev.slots[i].active)
			continue;

		cairo_set_source_rgb(cr, .2, .2, .8);
		x = w->evdev.slots[i].x;
		y = w->evdev.slots[i].y;
		x = 1.0 * (x - ax->minimum)/width * outline_width;
		y = 1.0 * (y - ay->minimum)/height * outline_height;
		x += center_x - outline_width/2;
		y += center_y - outline_height/2;
		cairo_arc(cr, x, y, 10, 0, 2 * M_PI);
		cairo_fill(cr);

		char finger_text[3];
		cairo_text_extents_t finger_text_extents;
		snprintf(finger_text, 3, "%zu", i);
		cairo_set_source_rgb(cr, 1.f, 1.f, 1.f);
		cairo_set_font_size(cr, 12.0);
		cairo_text_extents(cr, finger_text, &finger_text_extents);
		cairo_move_to(cr, x - finger_text_extents.width/2,
				  y + finger_text_extents.height/2);
		cairo_show_text(cr, finger_text);
	}

draw_outline:
	/* The touchpad outline */
	cairo_set_source_rgb(cr, .2, .2, .8);
	cairo_rectangle(cr,
			center_x - outline_width/2,
			center_y - outline_height/2,
			outline_width,
			outline_height);
	cairo_stroke(cr);
	cairo_restore(cr);
}

static inline void
draw_gestures(struct window *w, cairo_t *cr)
{
	int offset;

	/* swipe */
	cairo_save(cr);
	cairo_translate(cr, w->swipe.x, w->swipe.y);
	for (int i = 0; i < w->swipe.nfingers; i++) {
		cairo_set_source_rgb(cr, .8, .8, .4);
		cairo_arc(cr, (i - 2) * 40, 0, 20, 0, 2 * M_PI);
		cairo_fill(cr);
	}

	for (int i = 0; i < 4; i++) { /* 4 fg max */
		cairo_set_source_rgb(cr, 0, 0, 0);
		cairo_arc(cr, (i - 2) * 40, 0, 20, 0, 2 * M_PI);
		cairo_stroke(cr);
	}
	cairo_restore(cr);

	/* pinch */
	cairo_save(cr);
	offset = w->pinch.scale * 100;
	cairo_translate(cr, w->pinch.x, w->pinch.y);
	cairo_rotate(cr, w->pinch.angle * M_PI/180.0);
	if (w->pinch.nfingers > 0) {
		cairo_set_source_rgb(cr, .4, .4, .8);
		cairo_arc(cr, offset, -offset, 20, 0, 2 * M_PI);
		cairo_arc(cr, -offset, offset, 20, 0, 2 * M_PI);
		cairo_fill(cr);
	}

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_arc(cr, offset, -offset, 20, 0, 2 * M_PI);
	cairo_stroke(cr);
	cairo_arc(cr, -offset, offset, 20, 0, 2 * M_PI);
	cairo_stroke(cr);

	cairo_restore(cr);

	/* hold */
	cairo_save(cr);
	cairo_translate(cr, w->width/2, w->height/2 + 100);

	for (int i = 4; i > 0; i--) { /* 4 fg max */
		double r, g, b, hold_alpha;

		r = .4 + .2 * (i % 2);
		g = .2;
		b = .2;
		hold_alpha = (w->hold.active && i <= w->hold.nfingers) ? 1 : .5;

		cairo_set_source_rgba(cr, r, g, b, hold_alpha);
		cairo_arc(cr, 0, 0, 20 * i, 0, 2 * M_PI);
		cairo_fill(cr);

		cairo_set_source_rgba(cr, 0, 0, 0, hold_alpha);
		cairo_arc(cr, 0, 0, 20 * i, 0, 2 * M_PI);
		cairo_stroke(cr);
	}

	cairo_restore(cr);
}

static inline void
draw_scrollbars(struct window *w, cairo_t *cr)
{

	/* normal scrollbars */
	cairo_save(cr);
	cairo_set_source_rgb(cr, .4, .8, 0);
	cairo_rectangle(cr, w->scroll.vx - 10, w->scroll.vy - 20, 20, 40);
	cairo_rectangle(cr, w->scroll.hx - 20, w->scroll.hy - 10, 40, 20);
	cairo_fill(cr);

	/* discrete scrollbars */
	cairo_set_source_rgb(cr, .8, .4, 0);
	cairo_rectangle(cr, w->scroll.vx_discrete - 5, w->scroll.vy_discrete - 10, 10, 20);
	cairo_rectangle(cr, w->scroll.hx_discrete - 10, w->scroll.hy_discrete - 5, 20, 10);
	cairo_fill(cr);

	cairo_restore(cr);
}

static inline void
draw_touchpoints(struct window *w, cairo_t *cr)
{
	cairo_save(cr);
	ARRAY_FOR_EACH(w->touches, t) {
		if (t->state == TOUCH_ACTIVE)
			cairo_set_source_rgb(cr, .8, .2, .2);
		else
			cairo_set_source_rgb(cr, .8, .4, .4);
		cairo_arc(cr, t->x, t->y, 10, 0, 2 * M_PI);
		if (t->state == TOUCH_CANCELLED)
			cairo_stroke(cr);
		else
			cairo_fill(cr);
	}
	cairo_restore(cr);
}

static inline void
draw_abs_pointer(struct window *w, cairo_t *cr)
{

	cairo_save(cr);
	cairo_set_source_rgb(cr, .2, .4, .8);
	cairo_arc(cr, w->abs.x, w->abs.y, 10, 0, 2 * M_PI);
	cairo_fill(cr);
	cairo_restore(cr);
}

static inline void
draw_text(cairo_t *cr, const char *text, double x, double y)
{
	cairo_text_extents_t te;
	cairo_font_extents_t fe;

	cairo_text_extents(cr, text, &te);
	cairo_font_extents(cr, &fe);
	/* center of the rectangle */
	cairo_move_to(cr, x, y);
	cairo_rel_move_to(cr, -te.width/2, -fe.descent + te.height/2);
	cairo_show_text(cr, text);
}

static inline void
draw_other_button (struct window *w, cairo_t *cr)
{
	const char *name = w->buttons.other_name;

	cairo_save(cr);

	if (!w->buttons.other)
		goto outline;

	if (!name)
		name = "undefined";

	cairo_set_source_rgb(cr, .2, .8, .8);
	cairo_rectangle(cr, w->width/2 - 40, w->height - 150, 80, 30);
	cairo_fill(cr);

	cairo_set_source_rgb(cr, 0, 0, 0);

	draw_text(cr, name, w->width/2, w->height - 150 + 15);

outline:
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, w->width/2 - 40, w->height - 150, 80, 30);
	cairo_stroke(cr);
	cairo_restore(cr);
}

static inline void
draw_buttons(struct window *w, cairo_t *cr)
{
	cairo_save(cr);

	if (w->buttons.l || w->buttons.m || w->buttons.r) {
		cairo_set_source_rgb(cr, .2, .8, .8);
		if (w->buttons.l)
			cairo_rectangle(cr, w->width/2 - 100, w->height - 200, 70, 30);
		if (w->buttons.m)
			cairo_rectangle(cr, w->width/2 - 20, w->height - 200, 40, 30);
		if (w->buttons.r)
			cairo_rectangle(cr, w->width/2 + 30, w->height - 200, 70, 30);
		cairo_fill(cr);
	}

	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_rectangle(cr, w->width/2 - 100, w->height - 200, 70, 30);
	cairo_rectangle(cr, w->width/2 - 20, w->height - 200, 40, 30);
	cairo_rectangle(cr, w->width/2 + 30, w->height - 200, 70, 30);
	cairo_stroke(cr);
	cairo_restore(cr);

	draw_other_button(w, cr);
}

static inline void
draw_pad(struct window *w, cairo_t *cr)
{
	double rx, ry;
	double pos;
	char number[3];

	rx = w->width/2 - 200;
	ry = w->height/2 + 100;

	cairo_save(cr);
	/* outer ring */
	cairo_set_source_rgb(cr, .7, .7, .0);
	cairo_arc(cr, rx, ry, 50, 0, 2 * M_PI);
	cairo_fill(cr);

	/* inner ring */
	cairo_set_source_rgb(cr, 1., 1., 1.);
	cairo_arc(cr, rx, ry, 30, 0, 2 * M_PI);
	cairo_fill(cr);

	/* marker */
	/* libinput has degrees and 0 is north, cairo has radians and 0 is
	 * east */
	if (w->pad.ring.position != -1) {
		pos = (w->pad.ring.position + 270) * M_PI/180.0;
		cairo_set_source_rgb(cr, .0, .0, .0);
		cairo_set_line_width(cr, 20);
		cairo_arc(cr, rx, ry, 40, pos - M_PI/8 , pos + M_PI/8);
		cairo_stroke(cr);

		snprintf(number, sizeof(number), "%d", w->pad.ring.number);
		cairo_set_source_rgb(cr, .0, .0, .0);
		draw_text(cr, number, rx, ry);

	}

	cairo_restore(cr);

	rx = w->width/2 - 300;
	ry = w->height/2 + 50;

	cairo_save(cr);
	cairo_set_source_rgb(cr, .7, .7, .0);
	cairo_rectangle(cr, rx, ry, 20, 100);
	cairo_fill(cr);

	if (w->pad.strip.position != -1) {
		pos = w->pad.strip.position * 80;
		cairo_set_source_rgb(cr, .0, .0, .0);
		cairo_rectangle(cr, rx, ry + pos, 20, 20);
		cairo_fill(cr);

		snprintf(number, sizeof(number), "%d", w->pad.strip.number);
		cairo_set_source_rgb(cr, .0, .0, .0);
		draw_text(cr, number, rx + 10, ry - 10);
	}

	cairo_restore(cr);
}

static inline void
draw_tablet(struct window *w, cairo_t *cr)
{
	double x, y;
	int first, last;
	size_t mask;
	int rx, ry;

	/* pressure/distance bars */
	rx = w->width/2 + 100;
	ry = w->height/2 + 50;
	cairo_save(cr);
	cairo_set_source_rgb(cr, .2, .6, .6);
	cairo_rectangle(cr, rx, ry, 20, 100);
	cairo_stroke(cr);

	if (w->tool.distance > 0) {
		double pos = w->tool.distance * 100;
		cairo_rectangle(cr, rx, ry + 100 - pos, 20, 5);
		cairo_fill(cr);
	}
	if (w->tool.pressure > 0) {
		double pos = w->tool.pressure * 100;
		if (w->tool.is_down)
			cairo_rectangle(cr, rx + 25, ry + 95, 5, 5);
		cairo_rectangle(cr, rx, ry + 100 - pos, 20, pos);
		cairo_fill(cr);
	}
	cairo_restore(cr);

	/* tablet tool, square for prox-in location */
	cairo_save(cr);
	cairo_set_source_rgb(cr, .2, .6, .6);
	if (w->tool.x_in && w->tool.y_in) {
		cairo_rectangle(cr, w->tool.x_in - 15, w->tool.y_in - 15, 30, 30);
		cairo_stroke(cr);
	}

	if (w->tool.x_down && w->tool.y_down) {
		cairo_rectangle(cr, w->tool.x_down - 10, w->tool.y_down - 10, 20, 20);
		cairo_stroke(cr);
	}

	if (w->tool.x_up && w->tool.y_up) {
		cairo_rectangle(cr, w->tool.x_up - 10, w->tool.y_up - 10, 20, 20);
		cairo_stroke(cr);
	}

	if (w->tool.pressure)
		cairo_set_source_rgb(cr, .2, .8, .8);

	cairo_translate(cr, w->tool.x, w->tool.y);
	/* scale of 2.5 is large enough to make the marker visible around the
	   physical totem */
	cairo_scale(cr,
		    1.0 + w->tool.size_major * 2.5,
		    1.0 + w->tool.size_minor * 2.5);
	cairo_scale(cr, 1.0 + w->tool.tilt_x/30.0, 1.0 + w->tool.tilt_y/30.0);
	if (w->tool.rotation)
		cairo_rotate(cr, w->tool.rotation * M_PI/180.0);
	if (w->tool.pressure)
		cairo_set_source_rgb(cr, .8, .8, .2);
	cairo_arc(cr, 0, 0,
		  1 + 10 * max(w->tool.pressure, w->tool.distance),
		  0, 2 * M_PI);
	cairo_fill(cr);
	cairo_restore(cr);

	/* The line to indicate the origin */
	if (w->tool.size_major) {
		cairo_save(cr);
		cairo_scale(cr, 1.0, 1.0);
		cairo_translate(cr, w->tool.x, w->tool.y);
		if (w->tool.rotation)
			cairo_rotate(cr, w->tool.rotation * M_PI/180.0);
		cairo_set_source_rgb(cr, .0, .0, .0);
		cairo_move_to(cr, 0, 0);
		cairo_rel_line_to(cr, 0, -w->tool.size_major * 2.5);
		cairo_stroke(cr);
		cairo_restore(cr);
	}

	/* tablet deltas */
	mask = ARRAY_LENGTH(w->tool.deltas);
	first = max(w->tool.ndeltas + 1, mask) - mask;
	last = w->tool.ndeltas;

	cairo_save(cr);
	cairo_set_source_rgb(cr, .8, .8, .2);

	x = w->tool.deltas[first % mask].x;
	y = w->tool.deltas[first % mask].y;
	cairo_move_to(cr, x, y);

	for (int i = first + 1; i < last; i++) {
		x = w->tool.deltas[i % mask].x;
		y = w->tool.deltas[i % mask].y;
		cairo_line_to(cr, x, y);
	}

	cairo_stroke(cr);
	cairo_restore(cr);
}

static inline void
draw_pointer(struct window *w, cairo_t *cr)
{
	double x, y;
	int first, last;
	size_t mask;

	/* draw pointer sprite */
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_save(cr);
	cairo_move_to(cr, w->pointer.x, w->pointer.y);
	cairo_rel_line_to(cr, 10, 15);
	cairo_rel_line_to(cr, -10, 0);
	cairo_rel_line_to(cr, 0, -15);
	cairo_fill(cr);

	/* draw unaccelerated sprite */
	cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
	cairo_save(cr);
	cairo_move_to(cr, w->unaccelerated.x, w->unaccelerated.y);
	cairo_rel_line_to(cr, -5, -10);
	cairo_rel_line_to(cr, 10, 0);
	cairo_rel_line_to(cr, -5, 10);
	cairo_fill(cr);

	/* pointer deltas */
	mask = ARRAY_LENGTH(w->deltas);
	first = max(w->ndeltas + 1, mask) - mask;
	last = w->ndeltas;

	cairo_set_source_rgb(cr, .8, .5, .2);

	x = w->deltas[first % mask].x;
	y = w->deltas[first % mask].y;
	cairo_move_to(cr, x, y);

	for (int i = first + 1; i < last; i++) {
		x = w->deltas[i % mask].x;
		y = w->deltas[i % mask].y;
		cairo_line_to(cr, x, y);
	}

	cairo_stroke(cr);
	cairo_restore(cr);
}

static inline void
draw_background(struct window *w, cairo_t *cr)
{
	int x1, x2, y1, y2, x3, y3, x4, y4;
	int cols;

	/* 10px and 5px grids */
	cairo_save(cr);
	cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
	x1 = w->width/2 - 200;
	y1 = w->height/2 - 200;
	x2 = w->width/2 + 200;
	y2 = w->height/2 - 200;
	for (cols = 1; cols < 10; cols++) {
		cairo_move_to(cr, x1 + 10 * cols, y1);
		cairo_rel_line_to(cr, 0, 100);
		cairo_move_to(cr, x1, y1 + 10 * cols);
		cairo_rel_line_to(cr, 100, 0);

		cairo_move_to(cr, x2 + 5 * cols, y2);
		cairo_rel_line_to(cr, 0, 50);
		cairo_move_to(cr, x2, y2 + 5 * cols);
		cairo_rel_line_to(cr, 50, 0);
	}

	/* 3px horiz/vert bar codes */
	x3 = w->width/2 - 200;
	y3 = w->height/2 + 200;
	x4 = w->width/2 + 200;
	y4 = w->height/2 + 100;
	for (cols = 0; cols < 50; cols++) {
		cairo_move_to(cr, x3 + 3 * cols, y3);
		cairo_rel_line_to(cr, 0, 20);

		cairo_move_to(cr, x4, y4 + 3 * cols);
		cairo_rel_line_to(cr, 20, 0);
	}
	cairo_stroke(cr);

	/* round targets */
	for (int i = 0; i <= 3; i++) {
		x1 = w->width * i/4.0;
		x2 = w->width * i/4.0;

		y1 = w->height * 1.0/4.0;
		y2 = w->height * 3.0/4.0;

		cairo_arc(cr, x1, y1, 10, 0, 2 * M_PI);
		cairo_stroke(cr);
		cairo_arc(cr, x2, y2, 10, 0, 2 * M_PI);
		cairo_stroke(cr);
	}

	cairo_restore(cr);
}

static gboolean
draw(GtkWidget *widget, cairo_t *cr, gpointer data)
{
	struct window *w = data;

	cairo_set_font_size(cr, 12.0);
	cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_rectangle(cr, 0, 0, w->width, w->height);
	cairo_fill(cr);

	draw_background(w, cr);
	draw_evdev_rel(w, cr);
	draw_evdev_abs(w, cr);

	draw_pad(w, cr);
	draw_tablet(w, cr);
	draw_gestures(w, cr);
	draw_scrollbars(w, cr);
	draw_touchpoints(w, cr);
	draw_abs_pointer(w, cr);
	draw_buttons(w, cr);
	draw_pointer(w, cr);

	return TRUE;
}

#if HAVE_GTK4
static void
draw_gtk4(GtkDrawingArea *widget,
	  cairo_t *cr,
	  int width,
	  int height,
	  gpointer data)
{
	draw(GTK_WIDGET(widget), cr, data);
}
#endif

static void
window_place_ui_elements(GtkWidget *widget, struct window *w)
{
#if HAVE_GTK4
	w->width = gtk_widget_get_width(w->area);
	w->height = gtk_widget_get_height(w->area);
#else
	gtk_window_get_size(GTK_WINDOW(widget), &w->width, &w->height);
#endif

	w->pointer.x = w->width/2;
	w->pointer.y = w->height/2;
	w->unaccelerated.x = w->width/2;
	w->unaccelerated.y = w->height/2;
	w->deltas[0].x = w->pointer.x;
	w->deltas[0].y = w->pointer.y;

	w->scroll.vx = w->width/2;
	w->scroll.vy = w->height/2;
	w->scroll.hx = w->width/2;
	w->scroll.hy = w->height/2;
	w->scroll.vx_discrete = w->width/2;
	w->scroll.vy_discrete = w->height/2;
	w->scroll.hx_discrete = w->width/2;
	w->scroll.hy_discrete = w->height/2;

	w->swipe.x = w->width/2;
	w->swipe.y = w->height/2;

	w->pinch.scale = 1.0;
	w->pinch.x = w->width/2;
	w->pinch.y = w->height/2;
}

#if HAVE_GTK4
static void
map_event_cb(GtkDrawingArea *widget, int width, int height, gpointer data)
{
	struct window *w = data;

	window_place_ui_elements(GTK_WIDGET(widget), w);

	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(w->area),
				       draw_gtk4,
				       w,
				       NULL);

	gtk_widget_set_cursor_from_name(w->win, "none");

	window_lock_pointer(w);
}
#else
static void
map_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	struct window *w = data;
	GdkDisplay *display;
	GdkWindow *window;

	window_place_ui_elements(widget, w);

	g_signal_connect(G_OBJECT(w->area), "draw", G_CALLBACK(draw), w);

	window = gdk_event_get_window(event);
	display = gdk_window_get_display(window);

	gdk_window_set_cursor(gtk_widget_get_window(w->win),
			      gdk_cursor_new_for_display(display,
							 GDK_BLANK_CURSOR));

	window_lock_pointer(w);
}
#endif

static void
window_quit(struct window *w)
{
	g_main_loop_quit(w->event_loop);
}

#if HAVE_GTK4
static gboolean
window_delete_event_cb(GtkWindow *window, gpointer data)
{
	struct window *w = data;

	window_quit(w);

	return TRUE;
}
#else
static void
window_delete_event_cb(GtkWidget *widget, GdkEvent *event, gpointer data)
{
	struct window *w = data;

	window_quit(w);
}
#endif

static void
window_init(struct window *w)
{
	list_init(&w->evdev_devices);

#if HAVE_GTK4
	w->win = gtk_window_new();
#else
	w->win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif

	if (getenv("LIBINPUT_RUNNING_TEST_SUITE")) {
#if HAVE_GTK4
		gtk_window_minimize(GTK_WINDOW(w->win));
#else
		gtk_window_iconify(GTK_WINDOW(w->win));
#endif
	}

	gtk_window_set_title(GTK_WINDOW(w->win), "libinput debugging tool");
	gtk_window_set_default_size(GTK_WINDOW(w->win), 1024, 768);
	gtk_window_maximize(GTK_WINDOW(w->win));
	gtk_window_set_resizable(GTK_WINDOW(w->win), TRUE);
	gtk_widget_realize(w->win);

	w->area = gtk_drawing_area_new();

#if HAVE_GTK4
	g_signal_connect(G_OBJECT(w->area), "resize", G_CALLBACK(map_event_cb), w);
	g_signal_connect(G_OBJECT(w->win), "close-request", G_CALLBACK(window_delete_event_cb), w);

	gtk_window_set_child(GTK_WINDOW(w->win), w->area);
	gtk_widget_set_visible(w->win, TRUE);
#else
	g_signal_connect(G_OBJECT(w->win), "map-event", G_CALLBACK(map_event_cb), w);
	g_signal_connect(G_OBJECT(w->win), "delete-event", G_CALLBACK(window_delete_event_cb), w);

	gtk_widget_set_events(w->win, 0);
	gtk_widget_set_events(w->area, 0);

	gtk_container_add(GTK_CONTAINER(w->win), w->area);
	gtk_widget_show_all(w->win);
#endif

	w->pad.ring.position = -1;
	w->pad.strip.position = -1;
}

static void
window_cleanup(struct window *w)
{
	ARRAY_FOR_EACH(w->devices, dev) {
		if (*dev)
			libinput_device_unref(*dev);
	}
}

static void
change_ptraccel(struct window *w, double amount)
{
	ARRAY_FOR_EACH(w->devices, dev) {
		double speed;
		enum libinput_config_status status;

		if (*dev == NULL)
			continue;

		if (!libinput_device_config_accel_is_available(*dev))
			continue;

		speed = libinput_device_config_accel_get_speed(*dev);
		speed = clip(speed + amount, -1, 1);

		status = libinput_device_config_accel_set_speed(*dev, speed);

		if (status != LIBINPUT_CONFIG_STATUS_SUCCESS) {
			msg("%s: failed to change accel to %.2f (%s)\n",
			    libinput_device_get_name(*dev),
			    speed,
			    libinput_config_status_to_str(status));
		} else {
			printf("%s: speed is %.2f\n",
			       libinput_device_get_name(*dev),
			       speed);
		}

	}
}

static int
handle_event_evdev(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct libinput_device *dev = data;
	struct libinput *li = libinput_device_get_context(dev);
	struct window *w = libinput_get_user_data(li);
	struct evdev_device *d,
			    *device = NULL;
	struct input_event e;
	int rc;

	list_for_each(d, &w->evdev_devices, node) {
		if (d->libinput_device == dev) {
			device = d;
			break;
		}
	}

	if (device == NULL) {
		msg("Unknown device: %s\n", libinput_device_get_name(dev));
		return FALSE;
	}

	do {
		rc = libevdev_next_event(device->evdev,
					 LIBEVDEV_READ_FLAG_NORMAL,
					 &e);
		if (rc == -EAGAIN) {
			break;
		} else if (rc == LIBEVDEV_READ_STATUS_SYNC) {
			msg("SYN_DROPPED received\n");
			goto out;
		} else if (rc != LIBEVDEV_READ_STATUS_SUCCESS) {
			msg("Error reading event: %s\n", strerror(-rc));
			goto out;
		}

#define EVENT(t_, c_) (t_ << 16 | c_)
		switch (EVENT(e.type, e.code)) {
		case EVENT(EV_REL, REL_X):
			w->evdev.rel_x = e.value;
			break;
		case EVENT(EV_REL, REL_Y):
			w->evdev.rel_y = e.value;
			break;
		case EVENT(EV_ABS, ABS_MT_SLOT):
			w->evdev.slot = min((unsigned int)e.value,
					  ARRAY_LENGTH(w->evdev.slots) - 1);
			w->evdev.device = (uintptr_t)dev;
			break;
		case EVENT(EV_ABS, ABS_MT_TRACKING_ID):
			w->evdev.slots[w->evdev.slot].active = (e.value != -1);
			w->evdev.device = (uintptr_t)dev;
			break;
		case EVENT(EV_ABS, ABS_X):
			w->evdev.x = e.value;
			w->evdev.device = (uintptr_t)dev;
			break;
		case EVENT(EV_ABS, ABS_Y):
			w->evdev.y = e.value;
			w->evdev.device = (uintptr_t)dev;
			break;
		case EVENT(EV_ABS, ABS_MT_POSITION_X):
			w->evdev.slots[w->evdev.slot].x = e.value;
			w->evdev.device = (uintptr_t)dev;
			break;
		case EVENT(EV_ABS, ABS_MT_POSITION_Y):
			w->evdev.slots[w->evdev.slot].y = e.value;
			w->evdev.device = (uintptr_t)dev;
			break;
		}
	} while (rc == LIBEVDEV_READ_STATUS_SUCCESS);

	gtk_widget_queue_draw(w->area);
out:
	return TRUE;
}

static void
register_evdev_device(struct window *w, struct libinput_device *dev)
{
	GIOChannel *c;
	struct udev_device *ud;
	struct libevdev *evdev;
	const char *device_node;
	int fd;
	struct evdev_device *d;

	ud = libinput_device_get_udev_device(dev);
	device_node = udev_device_get_devnode(ud);

	fd = open(device_node, O_RDONLY|O_NONBLOCK);
	if (fd == -1) {
		msg("failed to open %s, evdev events unavailable\n", device_node);
		goto out;
	}

	if (libevdev_new_from_fd(fd, &evdev) != 0) {
		msg("failed to create context for %s, evdev events unavailable\n",
		    device_node);
		goto out;
	}

	d = zalloc(sizeof *d);
	list_append(&w->evdev_devices, &d->node);
	d->fd = fd;
	d->evdev = evdev;
	d->libinput_device =libinput_device_ref(dev);

	c = g_io_channel_unix_new(fd);
	g_io_channel_set_encoding(c, NULL, NULL);
	d->source_id = g_io_add_watch(c, G_IO_IN,
				      handle_event_evdev,
				      d->libinput_device);
	fd = -1;
out:
	close(fd);
	udev_device_unref(ud);
}

static void
unregister_evdev_device(struct window *w, struct libinput_device *dev)
{
	struct evdev_device *d;

	list_for_each_safe(d, &w->evdev_devices, node) {
		if (d->libinput_device != dev)
			continue;

		list_remove(&d->node);
		g_source_remove(d->source_id);
		free(libinput_device_get_user_data(d->libinput_device));
		libinput_device_unref(d->libinput_device);
		libevdev_free(d->evdev);
		close(d->fd);
		free(d);
		w->evdev.last_device = 0;
		break;
	}
}

static void
handle_event_device_notify(struct libinput_event *ev)
{
	struct libinput_device *dev = libinput_event_get_device(ev);
	struct libinput *li;
	struct window *w;
	const char *type;

	li = libinput_event_get_context(ev);
	w = libinput_get_user_data(li);

	if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
		type = "added";
		register_evdev_device(w, dev);
		tools_device_apply_config(libinput_event_get_device(ev),
					  &w->options);
	} else {
		type = "removed";
		unregister_evdev_device(w, dev);
	}

	msg("%s %-30s %s\n",
	    libinput_device_get_sysname(dev),
	    libinput_device_get_name(dev),
	    type);

	if (libinput_event_get_type(ev) == LIBINPUT_EVENT_DEVICE_ADDED) {
		ARRAY_FOR_EACH(w->devices, device) {
			if (*device == NULL) {
				*device = libinput_device_ref(dev);
				break;
			}
		}
	} else  {
		ARRAY_FOR_EACH(w->devices, device) {
			if (*device == dev) {
				libinput_device_unref(*device);
				*device = NULL;
				break;
			}
		}
	}
}

static void
handle_event_motion(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double dx = libinput_event_pointer_get_dx(p),
	       dy = libinput_event_pointer_get_dy(p);
	double dx_unaccel = libinput_event_pointer_get_dx_unaccelerated(p),
	       dy_unaccel = libinput_event_pointer_get_dy_unaccelerated(p);
	struct point point;
	const int mask = ARRAY_LENGTH(w->deltas);
	size_t idx;

	w->pointer.x = clip(w->pointer.x + dx, 0.0, w->width);
	w->pointer.y = clip(w->pointer.y + dy, 0.0, w->height);
	w->unaccelerated.x = clip(w->unaccelerated.x + dx_unaccel, 0.0, w->width);
	w->unaccelerated.y = clip(w->unaccelerated.y + dy_unaccel, 0.0, w->height);

	idx = w->ndeltas % mask;
	point = w->deltas[idx];
	idx = (w->ndeltas + 1) % mask;
	point.x += dx_unaccel;
	point.y += dy_unaccel;
	w->deltas[idx] = point;
	w->ndeltas++;
}

static void
handle_event_absmotion(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double x = libinput_event_pointer_get_absolute_x_transformed(p, w->width),
	       y = libinput_event_pointer_get_absolute_y_transformed(p, w->height);

	w->abs.x = x;
	w->abs.y = y;
}

static void
handle_event_touch(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_touch *t = libinput_event_get_touch_event(ev);
	int slot = libinput_event_touch_get_seat_slot(t);
	struct touch *touch;
	double x, y;

	if (slot == -1 || slot >= (int) ARRAY_LENGTH(w->touches))
		return;

	touch = &w->touches[slot];

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_TOUCH_UP:
		touch->state = TOUCH_ENDED;
		return;
	case LIBINPUT_EVENT_TOUCH_CANCEL:
		touch->state = TOUCH_CANCELLED;
		return;
	default:
		break;
	}

	x = libinput_event_touch_get_x_transformed(t, w->width),
	y = libinput_event_touch_get_y_transformed(t, w->height);

	touch->state = TOUCH_ACTIVE;
	touch->x = (int)x;
	touch->y = (int)y;
}

static void
handle_event_axis(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	double value;
	enum libinput_pointer_axis axis;
	enum libinput_event_type type;

	type = libinput_event_get_type(ev);

	axis = LIBINPUT_POINTER_AXIS_SCROLL_VERTICAL;
	if (libinput_event_pointer_has_axis(p, axis)) {
		value = libinput_event_pointer_get_scroll_value(p, axis);
		w->scroll.vy += value;
		w->scroll.vy = clip(w->scroll.vy, 0, w->height);

		if (type == LIBINPUT_EVENT_POINTER_SCROLL_WHEEL) {
			w->scroll.vy_discrete += value;
			w->scroll.vy_discrete = clip(w->scroll.vy_discrete, 0, w->height);
		}
	}

	axis = LIBINPUT_POINTER_AXIS_SCROLL_HORIZONTAL;
	if (libinput_event_pointer_has_axis(p, axis)) {
		value = libinput_event_pointer_get_scroll_value(p, axis);
		w->scroll.hx += value;
		w->scroll.hx = clip(w->scroll.hx, 0, w->width);

		if (type == LIBINPUT_EVENT_POINTER_SCROLL_WHEEL) {
			w->scroll.hx_discrete += value;
			w->scroll.hx_discrete = clip(w->scroll.hx_discrete, 0, w->width);
		}
	}
}

static int
handle_event_keyboard(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_keyboard *k = libinput_event_get_keyboard_event(ev);
	unsigned int key = libinput_event_keyboard_get_key(k);

	if (libinput_event_keyboard_get_key_state(k) ==
	    LIBINPUT_KEY_STATE_RELEASED)
		return 0;

	switch(key) {
	case KEY_ESC:
		return 1;
	case KEY_UP:
		change_ptraccel(w, 0.1);
		break;
	case KEY_DOWN:
		change_ptraccel(w, -0.1);
		break;
	default:
		break;
	}

	return 0;
}

static void
handle_event_button(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_pointer *p = libinput_event_get_pointer_event(ev);
	unsigned int button = libinput_event_pointer_get_button(p);
	bool is_press;

	is_press = libinput_event_pointer_get_button_state(p) == LIBINPUT_BUTTON_STATE_PRESSED;

	switch (button) {
	case BTN_LEFT:
		w->buttons.l = is_press;
		break;
	case BTN_RIGHT:
		w->buttons.r = is_press;
		break;
	case BTN_MIDDLE:
		w->buttons.m = is_press;
		break;
	default:
		w->buttons.other = is_press;
		w->buttons.other_name = libevdev_event_code_get_name(EV_KEY,
								     button);
	}

}

static void
handle_event_swipe(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_gesture *g = libinput_event_get_gesture_event(ev);
	int nfingers;
	double dx, dy;

	nfingers = libinput_event_gesture_get_finger_count(g);

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
		w->swipe.nfingers = nfingers;
		w->swipe.x = w->width/2;
		w->swipe.y = w->height/2;
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
		dx = libinput_event_gesture_get_dx(g);
		dy = libinput_event_gesture_get_dy(g);
		w->swipe.x += dx;
		w->swipe.y += dy;
		break;
	case LIBINPUT_EVENT_GESTURE_SWIPE_END:
		w->swipe.nfingers = 0;
		w->swipe.x = w->width/2;
		w->swipe.y = w->height/2;
		break;
	default:
		abort();
	}
}

static void
handle_event_pinch(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_gesture *g = libinput_event_get_gesture_event(ev);
	int nfingers;
	double dx, dy;

	nfingers = libinput_event_gesture_get_finger_count(g);

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
		w->pinch.nfingers = nfingers;
		w->pinch.x = w->width/2;
		w->pinch.y = w->height/2;
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
		dx = libinput_event_gesture_get_dx(g);
		dy = libinput_event_gesture_get_dy(g);
		w->pinch.x += dx;
		w->pinch.y += dy;
		w->pinch.scale = libinput_event_gesture_get_scale(g);
		w->pinch.angle += libinput_event_gesture_get_angle_delta(g);
		break;
	case LIBINPUT_EVENT_GESTURE_PINCH_END:
		w->pinch.nfingers = 0;
		w->pinch.x = w->width/2;
		w->pinch.y = w->height/2;
		w->pinch.angle = 0.0;
		w->pinch.scale = 1.0;
		break;
	default:
		abort();
	}
}

static void
handle_event_hold(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_gesture *g = libinput_event_get_gesture_event(ev);
	int nfingers;

	nfingers = libinput_event_gesture_get_finger_count(g);

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
		w->hold.nfingers = nfingers;
		w->hold.active = true;
		break;
	case LIBINPUT_EVENT_GESTURE_HOLD_END:
		w->hold.nfingers = nfingers;
		w->hold.active = false;
		break;
	default:
		abort();
	}
}

static void
handle_event_tablet(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_tablet_tool *t = libinput_event_get_tablet_tool_event(ev);
	double x, y;
	struct point point;
	int idx;
	const int mask = ARRAY_LENGTH(w->tool.deltas);
	bool is_press;
	unsigned int button;

	x = libinput_event_tablet_tool_get_x_transformed(t, w->width);
	y = libinput_event_tablet_tool_get_y_transformed(t, w->height);

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		if (libinput_event_tablet_tool_get_proximity_state(t) ==
		    LIBINPUT_TABLET_TOOL_PROXIMITY_STATE_OUT) {
			w->tool.x_in = 0;
			w->tool.y_in = 0;
			w->tool.x_down = 0;
			w->tool.y_down = 0;
			w->tool.x_up = 0;
			w->tool.y_up = 0;
		} else {
			w->tool.x_in = x;
			w->tool.y_in = y;
			w->tool.ndeltas = 0;
			w->tool.deltas[0].x = w->width/2;
			w->tool.deltas[0].y = w->height/2;
		}
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		w->tool.pressure = libinput_event_tablet_tool_get_pressure(t);
		w->tool.distance = libinput_event_tablet_tool_get_distance(t);
		w->tool.tilt_x = libinput_event_tablet_tool_get_tilt_x(t);
		w->tool.tilt_y = libinput_event_tablet_tool_get_tilt_y(t);
		if (libinput_event_tablet_tool_get_tip_state(t) ==
		    LIBINPUT_TABLET_TOOL_TIP_DOWN) {
			w->tool.x_down = x;
			w->tool.y_down = y;
			w->tool.is_down = true;
		} else {
			w->tool.x_up = x;
			w->tool.y_up = y;
			w->tool.is_down = false;
		}
		_fallthrough_;
	case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
		w->tool.x = x;
		w->tool.y = y;
		w->tool.pressure = libinput_event_tablet_tool_get_pressure(t);
		w->tool.distance = libinput_event_tablet_tool_get_distance(t);
		w->tool.tilt_x = libinput_event_tablet_tool_get_tilt_x(t);
		w->tool.tilt_y = libinput_event_tablet_tool_get_tilt_y(t);
		w->tool.rotation = libinput_event_tablet_tool_get_rotation(t);
		w->tool.size_major = libinput_event_tablet_tool_get_size_major(t);
		w->tool.size_minor = libinput_event_tablet_tool_get_size_minor(t);

		/* Add the delta to the last position and store them as abs
		 * coordinates */
		idx = w->tool.ndeltas % mask;
		point = w->tool.deltas[idx];

		idx = (w->tool.ndeltas + 1) % mask;
		point.x += libinput_event_tablet_tool_get_dx(t);
		point.y += libinput_event_tablet_tool_get_dy(t);
		w->tool.deltas[idx] = point;
		w->tool.ndeltas++;
		break;
	case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
		is_press = libinput_event_tablet_tool_get_button_state(t) == LIBINPUT_BUTTON_STATE_PRESSED;
		button = libinput_event_tablet_tool_get_button(t);

		w->buttons.other = is_press;
		w->buttons.other_name = libevdev_event_code_get_name(EV_KEY,
								     button);
		break;
	default:
		abort();
	}
}

static void
handle_event_tablet_pad(struct libinput_event *ev, struct window *w)
{
	struct libinput_event_tablet_pad *p = libinput_event_get_tablet_pad_event(ev);
	bool is_press;
	unsigned int button;
	static const char *pad_buttons[] = {
		"Pad 0", "Pad 1", "Pad 2", "Pad 3", "Pad 4", "Pad 5",
		"Pad 6", "Pad 7", "Pad 8", "Pad 9", "Pad >= 10"
	};
	double position;
	double number;

	switch (libinput_event_get_type(ev)) {
	case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		is_press = libinput_event_tablet_pad_get_button_state(p) == LIBINPUT_BUTTON_STATE_PRESSED;
		button = libinput_event_tablet_pad_get_button_number(p);
		w->buttons.other = is_press;
		w->buttons.other_name = pad_buttons[min(button, 10)];
		break;
	case LIBINPUT_EVENT_TABLET_PAD_RING:
		position = libinput_event_tablet_pad_get_ring_position(p);
		number = libinput_event_tablet_pad_get_ring_number(p);
		w->pad.ring.number = number;
		w->pad.ring.position = position;
		break;
	case LIBINPUT_EVENT_TABLET_PAD_STRIP:
		position = libinput_event_tablet_pad_get_strip_position(p);
		number = libinput_event_tablet_pad_get_strip_number(p);
		w->pad.strip.number = number;
		w->pad.strip.position = position;
		break;
	default:
		abort();
	}
}

static gboolean
handle_event_libinput(GIOChannel *source, GIOCondition condition, gpointer data)
{
	struct libinput *li = data;
	struct window *w = libinput_get_user_data(li);
	struct libinput_event *ev;

	tools_dispatch(li);

	while ((ev = libinput_get_event(li))) {
		switch (libinput_event_get_type(ev)) {
		case LIBINPUT_EVENT_NONE:
			abort();
		case LIBINPUT_EVENT_DEVICE_ADDED:
		case LIBINPUT_EVENT_DEVICE_REMOVED:
			handle_event_device_notify(ev);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION:
			handle_event_motion(ev, w);
			break;
		case LIBINPUT_EVENT_POINTER_MOTION_ABSOLUTE:
			handle_event_absmotion(ev, w);
			break;
		case LIBINPUT_EVENT_TOUCH_DOWN:
		case LIBINPUT_EVENT_TOUCH_MOTION:
		case LIBINPUT_EVENT_TOUCH_UP:
		case LIBINPUT_EVENT_TOUCH_CANCEL:
			handle_event_touch(ev, w);
			break;
		case LIBINPUT_EVENT_TOUCH_FRAME:
			break;
		case LIBINPUT_EVENT_POINTER_AXIS:
			/* ignore */
			break;
		case LIBINPUT_EVENT_POINTER_SCROLL_WHEEL:
		case LIBINPUT_EVENT_POINTER_SCROLL_FINGER:
		case LIBINPUT_EVENT_POINTER_SCROLL_CONTINUOUS:
			handle_event_axis(ev, w);
			break;
		case LIBINPUT_EVENT_POINTER_BUTTON:
			handle_event_button(ev, w);
			break;
		case LIBINPUT_EVENT_KEYBOARD_KEY:
			if (handle_event_keyboard(ev, w)) {
				libinput_event_destroy(ev);
				window_quit(w);
				return FALSE;
			}
			break;
		case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
		case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
		case LIBINPUT_EVENT_GESTURE_SWIPE_END:
			handle_event_swipe(ev, w);
			break;
		case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
		case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
		case LIBINPUT_EVENT_GESTURE_PINCH_END:
			handle_event_pinch(ev, w);
			break;
		case LIBINPUT_EVENT_GESTURE_HOLD_BEGIN:
		case LIBINPUT_EVENT_GESTURE_HOLD_END:
			handle_event_hold(ev, w);
			break;
		case LIBINPUT_EVENT_TABLET_TOOL_AXIS:
		case LIBINPUT_EVENT_TABLET_TOOL_PROXIMITY:
		case LIBINPUT_EVENT_TABLET_TOOL_TIP:
		case LIBINPUT_EVENT_TABLET_TOOL_BUTTON:
			handle_event_tablet(ev, w);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_BUTTON:
		case LIBINPUT_EVENT_TABLET_PAD_RING:
		case LIBINPUT_EVENT_TABLET_PAD_STRIP:
			handle_event_tablet_pad(ev, w);
			break;
		case LIBINPUT_EVENT_TABLET_PAD_KEY:
			break;
		case LIBINPUT_EVENT_SWITCH_TOGGLE:
			break;
		}

		libinput_event_destroy(ev);
	}
	gtk_widget_queue_draw(w->area);

	return TRUE;
}

static void
sockets_init(struct libinput *li)
{
	GIOChannel *c = g_io_channel_unix_new(libinput_get_fd(li));

	g_io_channel_set_encoding(c, NULL, NULL);
	g_io_add_watch(c, G_IO_IN, handle_event_libinput, li);
}

static void
usage(void) {
	printf("Usage: libinput debug-gui [options] [--udev <seat>|[--device] /dev/input/event0]\n");
}

static gboolean
signal_handler(void *data)
{
	struct libinput *li = data;
	struct window *w = libinput_get_user_data(li);

	window_quit(w);

	return FALSE;
}

int
main(int argc, char **argv)
{
	struct window w = {0};
	struct tools_options options;
	struct libinput *li;
	enum tools_backend backend = BACKEND_NONE;
	const char *seat_or_device[2] = {"seat0", NULL};
	bool verbose = false;
	bool gtk_init = false;

#if HAVE_GTK4
	gtk_init = gtk_init_check();
#else
	gtk_init = gtk_init_check(&argc, &argv);
#endif

	if (!gtk_init)
		return 77;

	tools_init_options(&options);

	while (1) {
		int c;
		int option_index = 0;
		enum {
			OPT_DEVICE = 1,
			OPT_UDEV,
			OPT_GRAB,
			OPT_VERBOSE,
		};
		static struct option opts[] = {
			CONFIGURATION_OPTIONS,
			{ "help",                      no_argument,       0, 'h' },
			{ "device",                    required_argument, 0, OPT_DEVICE },
			{ "udev",                      required_argument, 0, OPT_UDEV },
			{ "grab",                      no_argument,       0, OPT_GRAB },
			{ "verbose",                   no_argument,       0, OPT_VERBOSE },
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
			exit(0);
			break;
		case OPT_DEVICE:
			backend = BACKEND_DEVICE;
			seat_or_device[0] = optarg;
			break;
		case OPT_UDEV:
			backend = BACKEND_UDEV;
			seat_or_device[0] = optarg;
			break;
		case OPT_GRAB:
			w.grab = true;
			break;
		case OPT_VERBOSE:
			verbose = true;
			break;
		default:
			if (tools_parse_option(c, optarg, &options) != 0) {
				usage();
				return EXIT_INVALID_USAGE;
			}
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

	li = tools_open_backend(backend, seat_or_device, verbose, &w.grab);
	if (!li)
		return EXIT_FAILURE;

	libinput_set_user_data(li, &w);

	g_unix_signal_add(SIGINT, signal_handler, li);

	window_init(&w);
	w.options = options;
	sockets_init(li);
	handle_event_libinput(NULL, 0, li);

	w.event_loop = g_main_loop_new(NULL, FALSE);
	g_main_loop_run(w.event_loop);

	window_unlock_pointer(&w);
	window_cleanup(&w);
	libinput_unref(li);

	return EXIT_SUCCESS;
}
