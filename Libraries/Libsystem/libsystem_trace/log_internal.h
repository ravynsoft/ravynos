/**
 * log_internal.h
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

#ifndef __LOG_INTERNAL_H
#define __LOG_INTERNAL_H

#include <sys/param.h>
#include <sys/types.h>
#include <Block.h>
#include <os/base.h>
#include <os/object.h>
#include <os/log.h>

/* This is an overlay on the NSObject. Leave room for refcount and class info */
struct os_log_s {
    uintptr_t _opaque[2];
};

/* typedef void (^)(void) os_block_t; */
/* typedef void (*)(void *) os_function_t; */

extern size_t os_proc_available_memory();
extern void * os_retain(void * object);
extern void os_release(void * object);

/*
oslog
The OS_LOG_DEFAULT constant or a custom log object that you create with the os_log_create function.

type
A log type constant, such as OS_LOG_TYPE_DEFAULT, OS_LOG_TYPE_INFO, OS_LOG_TYPE_DEBUG, OS_LOG_TYPE_ERROR, or OS_LOG_TYPE_FAULT, that specifies the level of logging to check.

Return Value
true if logging at the specified level is in an enabled state; otherwise, false.
*/
extern bool os_log_type_enabled(os_log_t oslog, os_log_type_t type);

extern bool os_signpost_enabled(os_log_t log);


#endif /* __LOG_INTERNAL_H */
