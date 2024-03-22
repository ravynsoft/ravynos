/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

/**
 * Internal EGL defines
 */

#ifndef EGLDEFINES_INCLUDED
#define EGLDEFINES_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define _EGL_MAX_EXTENSIONS_LEN 2048

/* Hardcoded, conservative default for EGL_LARGEST_PBUFFER,
 * this is used to implement EGL_LARGEST_PBUFFER.
 */
#define _EGL_MAX_PBUFFER_WIDTH  4096
#define _EGL_MAX_PBUFFER_HEIGHT 4096

#define _EGL_VENDOR_STRING "Mesa Project"

#ifdef __cplusplus
}
#endif

#endif /* EGLDEFINES_INCLUDED */
