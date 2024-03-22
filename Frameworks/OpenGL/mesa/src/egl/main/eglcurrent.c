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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c11/threads.h"
#include "util/u_string.h"
#include "util/u_thread.h"

#include "eglcurrent.h"
#include "eglglobals.h"
#include "egllog.h"

static __THREAD_INITIAL_EXEC _EGLThreadInfo _egl_TLS = {
   .inited = false,
};

static void
_eglInitThreadInfo(_EGLThreadInfo *t)
{
   t->LastError = EGL_SUCCESS;
   /* default, per EGL spec */
   t->CurrentAPI = EGL_OPENGL_ES_API;
}

/**
 * Return the calling thread's thread info.
 * If the calling thread never calls this function before, or if its thread
 * info was destroyed, reinitialize it.  This function never returns NULL.
 */
_EGLThreadInfo *
_eglGetCurrentThread(void)
{
   _EGLThreadInfo *current = &_egl_TLS;
   if (unlikely(!current->inited)) {
      memset(current, 0, sizeof(current[0]));
      _eglInitThreadInfo(current);
      current->inited = true;
   }
   return current;
}

/**
 * Destroy the calling thread's thread info.
 */
void
_eglDestroyCurrentThread(void)
{
   _EGLThreadInfo *t = _eglGetCurrentThread();
   t->inited = false;
}

/**
 * Return the currently bound context of the current API, or NULL.
 */
_EGLContext *
_eglGetCurrentContext(void)
{
   _EGLThreadInfo *t = _eglGetCurrentThread();
   return t->CurrentContext;
}

/**
 * Record EGL error code and return EGL_FALSE.
 */
static EGLBoolean
_eglInternalError(EGLint errCode, const char *msg)
{
   _EGLThreadInfo *t = _eglGetCurrentThread();

   t->LastError = errCode;

   if (errCode != EGL_SUCCESS) {
      const char *s;

      switch (errCode) {
      case EGL_BAD_ACCESS:
         s = "EGL_BAD_ACCESS";
         break;
      case EGL_BAD_ALLOC:
         s = "EGL_BAD_ALLOC";
         break;
      case EGL_BAD_ATTRIBUTE:
         s = "EGL_BAD_ATTRIBUTE";
         break;
      case EGL_BAD_CONFIG:
         s = "EGL_BAD_CONFIG";
         break;
      case EGL_BAD_CONTEXT:
         s = "EGL_BAD_CONTEXT";
         break;
      case EGL_BAD_CURRENT_SURFACE:
         s = "EGL_BAD_CURRENT_SURFACE";
         break;
      case EGL_BAD_DISPLAY:
         s = "EGL_BAD_DISPLAY";
         break;
      case EGL_BAD_MATCH:
         s = "EGL_BAD_MATCH";
         break;
      case EGL_BAD_NATIVE_PIXMAP:
         s = "EGL_BAD_NATIVE_PIXMAP";
         break;
      case EGL_BAD_NATIVE_WINDOW:
         s = "EGL_BAD_NATIVE_WINDOW";
         break;
      case EGL_BAD_PARAMETER:
         s = "EGL_BAD_PARAMETER";
         break;
      case EGL_BAD_SURFACE:
         s = "EGL_BAD_SURFACE";
         break;
      case EGL_NOT_INITIALIZED:
         s = "EGL_NOT_INITIALIZED";
         break;
      default:
         s = "other EGL error";
      }
      _eglLog(_EGL_DEBUG, "EGL user error 0x%x (%s) in %s\n", errCode, s, msg);
   }

   return EGL_FALSE;
}

EGLBoolean
_eglError(EGLint errCode, const char *msg)
{
   if (errCode != EGL_SUCCESS) {
      EGLint type;
      if (errCode == EGL_BAD_ALLOC)
         type = EGL_DEBUG_MSG_CRITICAL_KHR;
      else
         type = EGL_DEBUG_MSG_ERROR_KHR;

      _eglDebugReport(errCode, NULL, type, msg);
   } else
      _eglInternalError(errCode, msg);

   return EGL_FALSE;
}

void
_eglDebugReport(EGLenum error, const char *funcName, EGLint type,
                const char *message, ...)
{
   _EGLThreadInfo *thr = _eglGetCurrentThread();
   EGLDEBUGPROCKHR callback = NULL;
   va_list args;

   if (funcName == NULL)
      funcName = thr->CurrentFuncName;

   simple_mtx_lock(_eglGlobal.Mutex);
   if (_eglGlobal.debugTypesEnabled & DebugBitFromType(type))
      callback = _eglGlobal.debugCallback;

   simple_mtx_unlock(_eglGlobal.Mutex);

   char *message_buf = NULL;
   if (message != NULL) {
      va_start(args, message);
      if (vasprintf(&message_buf, message, args) < 0)
         message_buf = NULL;
      va_end(args);
   }

   if (callback != NULL) {
      callback(error, funcName, type, thr->Label, thr->CurrentObjectLabel,
               message_buf);
   }

   if (type == EGL_DEBUG_MSG_CRITICAL_KHR || type == EGL_DEBUG_MSG_ERROR_KHR) {
      char *func_message_buf = NULL;
      /* Note: _eglError() is often called with msg == thr->currentFuncName */
      if (message_buf && funcName && strcmp(message_buf, funcName) != 0) {
         if (asprintf(&func_message_buf, "%s: %s", funcName, message_buf) < 0)
            func_message_buf = NULL;
      }
      _eglInternalError(error, func_message_buf ? func_message_buf : funcName);
      free(func_message_buf);
   }
   free(message_buf);
}
