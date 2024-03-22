/*
 * (C) Copyright 2016, NVIDIA CORPORATION.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * IBM AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Kyle Brenneman <kbrenneman@nvidia.com>
 */

#ifndef EGLDISPATCHSTUBS_H
#define EGLDISPATCHSTUBS_H

#include "glvnd/libeglabi.h"

// These variables are all generated along with the dispatch stubs.
extern const int __EGL_DISPATCH_FUNC_COUNT;
extern const char *const __EGL_DISPATCH_FUNC_NAMES[];
extern int __EGL_DISPATCH_FUNC_INDICES[];
extern const __eglMustCastToProperFunctionPointerType __EGL_DISPATCH_FUNCS[];

void
__eglInitDispatchStubs(const __EGLapiExports *exportsTable);
void
__eglSetDispatchIndex(const char *name, int index);

/**
 * Returns the dispatch function for the given name, or \c NULL if the function
 * isn't supported.
 */
void *
__eglDispatchFindDispatchFunction(const char *name);

// Helper functions used by the generated stubs.
__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByDisplay(EGLDisplay dpy, int index);
__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByDevice(EGLDeviceEXT dpy, int index);
__eglMustCastToProperFunctionPointerType
__eglDispatchFetchByCurrent(int index);

#endif // EGLDISPATCHSTUBS_H
