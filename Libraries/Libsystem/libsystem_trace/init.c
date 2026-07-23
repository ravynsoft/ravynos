/**
 * init.c
 * Author: Zoe Knox      Created: 2026-7-10
 *
 * Copyright (C) 2026 Zoe Knox. All rights reserved.
 * SPDX: BSD-2-Clause
 *
 * This is an original implementation of Apple's libsystem_trace.dylib based on
 * open-source code, including ASL, XNU, Swift, and other sources, and the API
 * specs on developer.apple.com. It is not based on decompilation or diassembly
 * of any closed-source object files.
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

#include <stdio.h>
#include <dlfcn.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFData.h>
#include "init.h"

/* Libsystem init glue */

static void *cf_handle = NULL;
CFStringRef (*_CFStringCreateWithCString)(CFAllocatorRef, const char*, CFStringEncoding) = NULL;
CFTypeID (*_CFGetTypeID)(CFTypeRef) = NULL;
CFTypeID (*_CFDataGetTypeID)(void) = NULL;
CFIndex (*_CFDataGetLength)(CFDataRef) = NULL;
const uint8_t* (*_CFDataGetBytePtr)(CFDataRef) = NULL;

void _libtrace_init(void)
{
    if (!cf_handle)
        cf_handle = dlopen("/System/Library/Frameworks/CoreFoundation.framework/CoreFoundation",
                           RTLD_LAZY);
    if (cf_handle) { // did we open it?
        _CFStringCreateWithCString = dlsym(cf_handle, "CFStringCreateWithCString");
        _CFGetTypeID = dlsym(cf_handle, "CFGetTypeID");
        _CFDataGetTypeID = dlsym(cf_handle, "CFDataGetTypeID");
        _CFDataGetBytePtr = dlsym(cf_handle, "CFDataGetBytePtr");
    }
    printf("initialized libsystem_trace: cf_handle=%p, pfunc=%p\n", cf_handle, _CFStringCreateWithCString);
}


