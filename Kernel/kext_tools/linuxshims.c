/**
 * linuxshims.c
 * Author: Zoe Knox      Created: 2026-03-01
 *
 * Copyright (C) 2026 ravynOS Project. All rights reserved.
 * SPDX: BSD-2-Clause
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/kext/OSKext.h>
#include "FILES/linuxshims.h"

LS_PRIVATE void
os_log_with_args(os_log_t kextlog, int logtype, char *fmt, va_list *args, void *addr)
{
    vfprintf(stdout, fmt, args);
}

/* NOT thread safe unless your underlying basename() is! */
LS_PRIVATE int
basename_r(char *path, char **out)
{
    *out = basename(path);
    if(*out)
        return 0;
    return -1;
}

/* NOT thread safe unless your underlying dirname() is! */
LS_PRIVATE int
dirname_r(char *path, char **out)
{
    *out = dirname(path);
    if(*out)
	return 0;
    return -1;
}
