/**************************************************************************
 *
 * Copyright 2009-2010 Chia-I Wu <olvaffe@gmail.com>
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

#ifndef EGLCURRENT_INCLUDED
#define EGLCURRENT_INCLUDED

#include <stdbool.h>

#include "egltypedefs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _EGL_API_ALL_BITS                                                      \
   (EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT |                  \
    EGL_OPENGL_ES3_BIT_KHR | EGL_OPENGL_BIT)

/**
 * Per-thread info
 */
struct _egl_thread_info {
   bool inited;
   EGLint LastError;
   _EGLContext *CurrentContext;
   EGLenum CurrentAPI;
   EGLLabelKHR Label;

   /**
    * The name of the EGL function that's being called at the moment. This is
    * used to report the function name to the EGL_KHR_debug callback.
    */
   const char *CurrentFuncName;
   EGLLabelKHR CurrentObjectLabel;
};

/**
 * Return true if a client API enum is recognized.
 */
static inline EGLBoolean
_eglIsApiValid(EGLenum api)
{
#if HAVE_OPENGL && !defined(ANDROID)
   /* OpenGL is not a valid/supported API on Android */
   if (api == EGL_OPENGL_API)
      return true;
#endif
#if HAVE_OPENGL_ES_1 || HAVE_OPENGL_ES_2
   if (api == EGL_OPENGL_ES_API)
      return true;
#endif
   return false;
}

extern _EGLThreadInfo *
_eglGetCurrentThread(void);

extern void
_eglDestroyCurrentThread(void);

extern _EGLContext *
_eglGetCurrentContext(void);

extern EGLBoolean
_eglError(EGLint errCode, const char *msg);

extern void
_eglDebugReport(EGLenum error, const char *funcName, EGLint type,
                const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif /* EGLCURRENT_INCLUDED */
