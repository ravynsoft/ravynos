/**************************************************************************
 *
 * Copyright 2010 LunarG, Inc.
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

#include <inttypes.h>
#include <string.h>

#include "eglcurrent.h"
#include "egldriver.h"
#include "egllog.h"
#include "eglsync.h"

/**
 * Parse the list of sync attributes and return the proper error code.
 */
static EGLint
_eglParseSyncAttribList(_EGLSync *sync, const EGLAttrib *attrib_list)
{
   EGLint i;

   if (!attrib_list)
      return EGL_SUCCESS;

   for (i = 0; attrib_list[i] != EGL_NONE; i++) {
      EGLAttrib attr = attrib_list[i++];
      EGLAttrib val = attrib_list[i];
      EGLint err = EGL_SUCCESS;

      switch (attr) {
      case EGL_CL_EVENT_HANDLE_KHR:
         if (sync->Type == EGL_SYNC_CL_EVENT_KHR) {
            sync->CLEvent = val;
         } else {
            err = EGL_BAD_ATTRIBUTE;
         }
         break;
      case EGL_SYNC_NATIVE_FENCE_FD_ANDROID:
         if (sync->Type == EGL_SYNC_NATIVE_FENCE_ANDROID) {
            /* we take ownership of the native fd, so no dup(): */
            sync->SyncFd = val;
         } else {
            err = EGL_BAD_ATTRIBUTE;
         }
         break;
      default:
         err = EGL_BAD_ATTRIBUTE;
         break;
      }

      if (err != EGL_SUCCESS) {
         _eglLog(_EGL_DEBUG, "bad sync attribute 0x%" PRIxPTR, attr);
         return err;
      }
   }

   return EGL_SUCCESS;
}

EGLBoolean
_eglInitSync(_EGLSync *sync, _EGLDisplay *disp, EGLenum type,
             const EGLAttrib *attrib_list)
{
   EGLint err;

   _eglInitResource(&sync->Resource, sizeof(*sync), disp);
   sync->Type = type;
   sync->SyncStatus = EGL_UNSIGNALED_KHR;
   sync->SyncFd = EGL_NO_NATIVE_FENCE_FD_ANDROID;

   err = _eglParseSyncAttribList(sync, attrib_list);

   switch (type) {
   case EGL_SYNC_CL_EVENT_KHR:
      sync->SyncCondition = EGL_SYNC_CL_EVENT_COMPLETE_KHR;
      break;
   case EGL_SYNC_NATIVE_FENCE_ANDROID:
      if (sync->SyncFd == EGL_NO_NATIVE_FENCE_FD_ANDROID)
         sync->SyncCondition = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
      else
         sync->SyncCondition = EGL_SYNC_NATIVE_FENCE_SIGNALED_ANDROID;
      break;
   default:
      sync->SyncCondition = EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR;
   }

   if (err != EGL_SUCCESS)
      return _eglError(err, "eglCreateSyncKHR");

   if (type == EGL_SYNC_CL_EVENT_KHR && !sync->CLEvent)
      return _eglError(EGL_BAD_ATTRIBUTE, "eglCreateSyncKHR");

   return EGL_TRUE;
}

EGLBoolean
_eglGetSyncAttrib(_EGLDisplay *disp, _EGLSync *sync, EGLint attribute,
                  EGLAttrib *value)
{
   switch (attribute) {
   case EGL_SYNC_TYPE_KHR:
      *value = sync->Type;
      break;
   case EGL_SYNC_STATUS_KHR:
      /* update the sync status */
      if (sync->SyncStatus != EGL_SIGNALED_KHR &&
          (sync->Type == EGL_SYNC_FENCE_KHR ||
           sync->Type == EGL_SYNC_CL_EVENT_KHR ||
           sync->Type == EGL_SYNC_REUSABLE_KHR ||
           sync->Type == EGL_SYNC_NATIVE_FENCE_ANDROID))
         disp->Driver->ClientWaitSyncKHR(disp, sync, 0, 0);

      *value = sync->SyncStatus;
      break;
   case EGL_SYNC_CONDITION_KHR:
      if (sync->Type != EGL_SYNC_FENCE_KHR &&
          sync->Type != EGL_SYNC_CL_EVENT_KHR &&
          sync->Type != EGL_SYNC_NATIVE_FENCE_ANDROID)
         return _eglError(EGL_BAD_ATTRIBUTE, "eglGetSyncAttribKHR");
      *value = sync->SyncCondition;
      break;

   default:
      return _eglError(EGL_BAD_ATTRIBUTE, "eglGetSyncAttribKHR");
      break;
   }

   return EGL_TRUE;
}
