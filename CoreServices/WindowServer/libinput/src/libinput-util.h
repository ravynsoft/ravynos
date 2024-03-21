/*
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2013-2015 Red Hat, Inc.
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

#ifndef LIBINPUT_UTIL_H
#define LIBINPUT_UTIL_H

#include "config.h"

#ifdef NDEBUG
#warning "libinput relies on assert(). #defining NDEBUG is not recommended"
#endif

#include "libinput.h"

#include "util-bits.h"
#include "util-macros.h"
#include "util-list.h"
#include "util-matrix.h"
#include "util-strings.h"
#include "util-ratelimit.h"
#include "util-prop-parsers.h"
#include "util-time.h"

#define VENDOR_ID_APPLE 0x5ac
#define VENDOR_ID_CHICONY 0x4f2
#define VENDOR_ID_LOGITECH 0x46d
#define VENDOR_ID_WACOM 0x56a
#define VENDOR_ID_SYNAPTICS_SERIAL 0x002
#define PRODUCT_ID_APPLE_KBD_TOUCHPAD 0x273
#define PRODUCT_ID_APPLE_APPLETOUCH 0x21a
#define PRODUCT_ID_SYNAPTICS_SERIAL 0x007
#define PRODUCT_ID_WACOM_EKR 0x0331

/* The HW DPI rate we normalize to before calculating pointer acceleration */
#define DEFAULT_MOUSE_DPI 1000
#define DEFAULT_TRACKPOINT_SENSITIVITY 128

#define trace(...) \
	do { \
	printf("%s() - \033[0;31m", __func__); \
	printf(__VA_ARGS__); \
	printf("\033[0m"); \
	} while (0)

#define LIBINPUT_EXPORT __attribute__ ((visibility("default")))
#define LIBINPUT_UNUSED __attribute__ ((unused))

#endif /* LIBINPUT_UTIL_H */
