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

#include <sys/cdefs.h>
#include <asl.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "glxclient.h"
#include "apple_glx_log.h"
#include "util/u_debug.h"

static aslclient aslc;

void apple_glx_log_init(void) {
    aslc = asl_open(NULL, NULL, 0);
}

void _apple_glx_log(int level, const char *file, const char *function,
                    int line, const char *fmt, ...) {
    va_list v;
    va_start(v, fmt);
    _apple_glx_vlog(level, file, function, line, fmt, v);
    va_end(v);
}

static const char *
_asl_level_string(int level)
{
        if (level == ASL_LEVEL_EMERG) return ASL_STRING_EMERG;
        if (level == ASL_LEVEL_ALERT) return ASL_STRING_ALERT;
        if (level == ASL_LEVEL_CRIT) return ASL_STRING_CRIT;
        if (level == ASL_LEVEL_ERR) return ASL_STRING_ERR;
        if (level == ASL_LEVEL_WARNING) return ASL_STRING_WARNING;
        if (level == ASL_LEVEL_NOTICE) return ASL_STRING_NOTICE;
        if (level == ASL_LEVEL_INFO) return ASL_STRING_INFO;
        if (level == ASL_LEVEL_DEBUG) return ASL_STRING_DEBUG;
        return "unknown";
}

void _apple_glx_vlog(int level, const char *file, const char *function,
                     int line, const char *fmt, va_list args) {
    aslmsg msg;
    uint64_t thread = 0;

    if (pthread_is_threaded_np()) {
#if MAC_OS_X_VERSION_MAX_ALLOWED < 1060
        thread = (uint64_t)(uintptr_t)pthread_self();
#elif MAC_OS_X_VERSION_MIN_REQUIRED < 1060
        if (&pthread_threadid_np) {
            pthread_threadid_np(NULL, &thread);
        } else {
            thread = (uint64_t)(uintptr_t)pthread_self();
        }
#else
        pthread_threadid_np(NULL, &thread);
#endif
    }

    DebugMessageF("%-9s %24s:%-4d %s(%"PRIu64"): ",
                  _asl_level_string(level), file, line, function, thread);

    msg = asl_new(ASL_TYPE_MSG);
    if (msg) {
        if (file)
            asl_set(msg, "File", file);
        if (function)
            asl_set(msg, "Function", function);
        if (line) {
            char *_line;
            asprintf(&_line, "%d", line);
            if (_line) {
                asl_set(msg, "Line", _line);
                free(_line);
            }
        }
        if (pthread_is_threaded_np()) {
            char *_thread;
            asprintf(&_thread, "%"PRIu64, thread);
            if (_thread) {
                asl_set(msg, "Thread", _thread);
                free(_thread);
            }
        }
    }

    asl_vlog(aslc, msg, level, fmt, args);
    if (msg)
        asl_free(msg);
}
