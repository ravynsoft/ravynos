/**
 * log.c
 * Author: Zoe Knox      Created: 2026-02-15
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * SPDX: BSD-2-Clause
 *
 * This is an original implementation of Apple's libsystem_trace.dylib based on
 * open-source code, including ASL, XNU, Swift, and other sources, and the API
 * specs on developer.apple.com.
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

#include "log_internal.h"
#include <objc/runtime.h>
#include <stdio.h>

struct os_log_s _os_log_default;

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW OS_WARN_RESULT OS_OBJECT_RETURNS_RETAINED
os_log_t
os_log_create(const char *subsystem, const char *category)
{
    Class cls = objc_lookUpClass("NSObject");
    id obj = class_createInstance(cls, 0);
    os_retain(obj);
    return (os_log_t)obj;
}

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool
os_log_info_enabled(
os_log_t log
)
{
    return true; /* FIXME: Implement this */
}

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool
os_log_debug_enabled(os_log_t log)
{
    return true; /* FIXME: Implement this */
}

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
bool
os_log_shim_enabled(void *ret_addr)
{
    (void)ret_addr;
    return true;
}

__OSX_AVAILABLE_STARTING(__MAC_10_12, __IPHONE_10_0)
OS_EXPORT OS_NOTHROW
void
os_log_with_args(os_log_t oslog, os_log_type_t type, const char *format, va_list args, void *ret_addr)
{
    (void)oslog;
    (void)ret_addr;
    if (format == NULL)
        return;
    fprintf(stderr, "[OS LOG %d] ", type);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW
void
_os_log_internal(void *dso, os_log_t log, os_log_type_t type, const char *message, ...)
{
    /* FIXME: this is just a placeholder for now */
    va_list args;
    va_start(args, message);
    fprintf(stderr, "[OS LOG 0x%p %d] ", dso, type);
    vfprintf(stderr, message, args);
}

__WATCHOS_AVAILABLE(3.0) __OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0) __TVOS_AVAILABLE(10.0)
OS_EXPORT OS_NOTHROW
int
_os_log_internal_driverKit(void *dso, os_log_t log, os_log_type_t type, const char *message, ...)
{
    /* FIXME: this is just a placeholder for now */
    va_list args;
    va_start(args, message);
    fprintf(stderr, "[OS LOG 0x%p %d] ", dso, type);
    vfprintf(stderr, message, args);
}

size_t os_log_pack_size(const char* format, ...)
{
    /* FIXME: this is just a placeholder for now */
    static char buf[4096];
    va_list args;
    va_start(args, format);
    int n = vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    if (n < 0 || n >= (int)sizeof(buf))
        return (size_t)-1;
    return (size_t)n;
}

void os_log_pack_fill(void* pack,
    size_t pack_size,
    int saved_errno,
    const char* format,
    ...)
{
    /* FIXME: this is just a placeholder for now */
    (void)saved_errno;
    if ( pack == NULL || pack_size == 0 )
        return;
    va_list args;
    va_start(args, format);
    vsnprintf((char*)pack, pack_size, format, args);
    va_end(args);
}

