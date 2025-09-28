/*
 * Copyright © 2008-2011 Kristian Høgsberg
 * Copyright © 2011 Intel Corporation
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

#pragma once

#include "config.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])
/**
 * Iterate through the array _arr, assigning the variable elem to each
 * element. elem only exists within the loop.
 */
#define ARRAY_FOR_EACH(_arr, _elem) \
	for (__typeof__((_arr)[0]) *_elem = _arr; \
	     _elem < (_arr) + ARRAY_LENGTH(_arr); \
	     _elem++)

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define ANSI_HIGHLIGHT		"\x1B[0;1;39m"
#define ANSI_RED		"\x1B[0;31m"
#define ANSI_GREEN		"\x1B[0;32m"
#define ANSI_YELLOW		"\x1B[0;33m"
#define ANSI_BLUE		"\x1B[0;34m"
#define ANSI_MAGENTA		"\x1B[0;35m"
#define ANSI_CYAN		"\x1B[0;36m"
#define ANSI_BRIGHT_RED		"\x1B[0;31;1m"
#define ANSI_BRIGHT_GREEN	"\x1B[0;32;1m"
#define ANSI_BRIGHT_YELLOW	"\x1B[0;33;1m"
#define ANSI_BRIGHT_BLUE	"\x1B[0;34;1m"
#define ANSI_BRIGHT_MAGENTA	"\x1B[0;35;1m"
#define ANSI_BRIGHT_CYAN	"\x1B[0;36;1m"
#define ANSI_NORMAL		"\x1B[0m"

#define ANSI_UP			"\x1B[%dA"
#define ANSI_DOWN		"\x1B[%dB"
#define ANSI_RIGHT		"\x1B[%dC"
#define ANSI_LEFT		"\x1B[%dD"

#define CASE_RETURN_STRING(a) case a: return #a

#define _fallthrough_ __attribute__((fallthrough))
