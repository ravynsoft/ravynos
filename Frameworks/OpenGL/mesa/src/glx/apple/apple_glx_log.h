/*
 * Copyright (c) 2012 Apple Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 * HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without
 * prior written authorization.
 */

#ifndef APPLE_GLX_LOG_H
#define APPLE_GLX_LOG_H

#include <sys/cdefs.h>
#include <asl.h>

void apple_glx_log_init(void);

__printflike(5, 6)
void _apple_glx_log(int level, const char *file, const char *function,
                    int line, const char *fmt, ...);
#define apple_glx_log(l, f, args ...) \
    _apple_glx_log(l, __FILE__, __func__, __LINE__, f, ## args)


__printflike(5, 0)
void _apple_glx_vlog(int level, const char *file, const char *function,
                     int line, const char *fmt, va_list v);
#define apple_glx_vlog(l, f, v) \
    _apple_glx_vlog(l, __FILE__, __func__, __LINE__, f, v)

/* This is just here to help the transition.
 * TODO: Replace calls to apple_glx_diagnostic
 */
#define apple_glx_diagnostic(f, args ...) \
    apple_glx_log(ASL_LEVEL_DEBUG, f, ## args)

#endif
