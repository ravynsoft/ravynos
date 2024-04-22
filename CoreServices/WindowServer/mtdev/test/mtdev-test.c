/*****************************************************************************
 *
 * mtdev - Multitouch Protocol Translation Library (MIT license)
 *
 * Copyright (C) 2010 Henrik Rydberg <rydberg@euromail.se>
 * Copyright (C) 2010 Canonical Ltd.
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
 *
 ****************************************************************************/

#include <mtdev.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>

#ifndef input_event_sec
#define input_event_sec time.tv_sec
#define input_event_usec time.tv_usec
#endif

/* year-proof millisecond event time */
typedef uint64_t mstime_t;

static int use_event(const struct input_event *ev)
{
#if 0
	return ev->type == EV_ABS && mtdev_is_absmt(ev->code);
#else
	return 1;
#endif
}

static void print_event(const struct input_event *ev)
{
	static const mstime_t ms = 1000;
	static int slot;
	mstime_t evtime = ev->input_event_usec / ms + ev->input_event_sec * ms;
	if (ev->type == EV_ABS && ev->code == ABS_MT_SLOT)
		slot = ev->value;
	fprintf(stderr, "%012llx %02d %01d %04x %d\n",
		evtime, slot, ev->type, ev->code, ev->value);
}

#define CHECK(dev, name)			\
	if (mtdev_has_mt_event(dev, name))	\
		fprintf(stderr, "   %s\n", #name)

static void show_props(const struct mtdev *dev)
{
	fprintf(stderr, "supported mt events:\n");
	CHECK(dev, ABS_MT_SLOT);
	CHECK(dev, ABS_MT_TOUCH_MAJOR);
	CHECK(dev, ABS_MT_TOUCH_MINOR);
	CHECK(dev, ABS_MT_WIDTH_MAJOR);
	CHECK(dev, ABS_MT_WIDTH_MINOR);
	CHECK(dev, ABS_MT_ORIENTATION);
	CHECK(dev, ABS_MT_POSITION_X);
	CHECK(dev, ABS_MT_POSITION_Y);
	CHECK(dev, ABS_MT_TOOL_TYPE);
	CHECK(dev, ABS_MT_BLOB_ID);
	CHECK(dev, ABS_MT_TRACKING_ID);
	CHECK(dev, ABS_MT_PRESSURE);
	CHECK(dev, ABS_MT_DISTANCE);
}

static void loop_device(int fd)
{
	struct mtdev dev;
	struct input_event ev;
	int ret = mtdev_open(&dev, fd);
	if (ret) {
		fprintf(stderr, "error: could not open device: %d\n", ret);
		return;
	}
	show_props(&dev);
	/* while the device has not been inactive for five seconds */
	while (!mtdev_idle(&dev, fd, 5000)) {
		/* extract all available processed events */
		while (mtdev_get(&dev, fd, &ev, 1) > 0) {
			if (use_event(&ev))
				print_event(&ev);
		}
	}
	mtdev_close(&dev);
}

int main(int argc, char *argv[])
{
	int fd;
	if (argc < 2) {
		fprintf(stderr, "Usage: mtdev <device>\n");
		return -1;
	}
	fd = open(argv[1], O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		fprintf(stderr, "error: could not open device\n");
		return -1;
	}
	if (ioctl(fd, EVIOCGRAB, 1)) {
		fprintf(stderr, "error: could not grab the device\n");
		return -1;
	}
	loop_device(fd);
	ioctl(fd, EVIOCGRAB, 0);
	close(fd);
	return 0;
}
