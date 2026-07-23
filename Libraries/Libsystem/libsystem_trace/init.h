/**
 * init.h
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

#ifndef __LIBTRACE__INIT_H_
#define __LIBTRACE__INIT_H_

#include <CoreFoundation/CFString.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void _libtrace_init(void);
extern CFStringRef (*_CFStringCreateWithCString)(CFAllocatorRef, const char*, CFStringEncoding);
extern CFTypeID (*_CFGetTypeID)(CFTypeRef);
extern CFTypeID (*_CFDataGetTypeID)(void);
extern CFIndex (*_CFDataGetLength)(CFDataRef);
extern const uint8_t* (*_CFDataGetBytePtr)(CFDataRef);

#define CFStringCreateWithCString _CFStringCreateWithCString
#define CFGetTypeID      _CFGetTypeID
#define CFDataGetTypeID  _CFDataGetTypeID
#define CFDataGetLength  _CFDataGetLength
#define CFDataGetBytePtr _CFDataGetBytePtr

#ifdef __cplusplus
}
#endif
#endif // __LIBTRACE__INIT_H_
