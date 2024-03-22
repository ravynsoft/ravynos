/**************************************************************************
 *
 * Copyright 2015 Advanced Micro Devices, Inc.
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef DD_UTIL_H
#define DD_UTIL_H

#include <stdio.h>
#include <errno.h>

#include "c99_alloca.h"
#include "util/u_atomic.h"
#include "util/u_debug.h"
#include "util/u_string.h"

#include "util/detect.h"
#if DETECT_OS_UNIX
#include <unistd.h>
#include <sys/stat.h>
#elif DETECT_OS_WINDOWS
#include <direct.h>
#include <process.h>
#define mkdir(dir, mode) _mkdir(dir)
#endif

struct pipe_screen;

/* name of the directory in home */
#define DD_DIR "ddebug_dumps"

void
dd_get_debug_filename_and_mkdir(char *buf, size_t buflen, bool verbose);

FILE *
dd_get_debug_file(bool verbose);

void
dd_parse_apitrace_marker(const char *string, int len, unsigned *call_number);

void
dd_write_header(FILE *f, struct pipe_screen *screen, unsigned apitrace_call_number);

#endif /* DD_UTIL_H */
