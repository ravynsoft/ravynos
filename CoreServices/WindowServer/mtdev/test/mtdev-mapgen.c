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

#define MTDEV_NO_LEGACY_API
#include <mtdev.h>
#include <stdio.h>

#define MT_ABS_SIZE	12
#define MT_SLOT_ABS_EVENTS {	\
	ABS_MT_TOUCH_MAJOR,	\
	ABS_MT_TOUCH_MINOR,	\
	ABS_MT_WIDTH_MAJOR,	\
	ABS_MT_WIDTH_MINOR,	\
	ABS_MT_ORIENTATION,	\
	ABS_MT_POSITION_X,	\
	ABS_MT_POSITION_Y,	\
	ABS_MT_TOOL_TYPE,	\
	ABS_MT_BLOB_ID,		\
	ABS_MT_TRACKING_ID,	\
	ABS_MT_PRESSURE,	\
	ABS_MT_DISTANCE,	\
}

static unsigned int cabs2mt[ABS_CNT];
static unsigned int cmt2abs[MT_ABS_SIZE];

static void init_caps()
{
	static const int init_abs_map[MT_ABS_SIZE] = MT_SLOT_ABS_EVENTS;
	int i;
	for (i = 0; i < MT_ABS_SIZE; i++) {
		cabs2mt[init_abs_map[i]] = i + 1;
		cmt2abs[i] = init_abs_map[i];
	}
}

static inline const char *newln(int i, int n)
{
	return i == n - 1 || i % 8 == 7 ? "\n" : "";
}

int main(int argc, char *argv[])
{
	int i;
	init_caps();
	printf("static const unsigned int mtdev_map_abs2mt[ABS_CNT] = {\n");
	for (i = 0; i < ABS_CNT; i++)
		printf(" 0x%04x,%s", cabs2mt[i], newln(i, ABS_CNT));
	printf("};\n\n");
	printf("static const unsigned int mtdev_map_mt2abs[MT_ABS_SIZE] = {\n");
	for (i = 0; i < MT_ABS_SIZE; i++)
		printf(" 0x%04x,%s", cmt2abs[i], newln(i, MT_ABS_SIZE));
	printf("};\n\n");
	return 0;
}
