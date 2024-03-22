/*
 Copyright (c) 2008, 2009 Apple Inc.
 
 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation files
 (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 DEALINGS IN THE SOFTWARE.
 
 Except as contained in this notice, the name(s) of the above
 copyright holders shall not be used in advertising or otherwise to
 promote the sale, use or other dealings in this Software without
 prior written authorization.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <dlfcn.h>
#include <pthread.h>
#include <inttypes.h>
#include "appledri.h"
#include "apple_glx.h"
#include "apple_glx_context.h"
#include "apple_cgl.h"

static bool initialized = false;
static int dri_event_base = 0;

int
apple_get_dri_event_base(void)
{
   if (!initialized) {
      fprintf(stderr,
              "error: dri_event_base called before apple_init_glx!\n");
      abort();
   }
   return dri_event_base;
}

static void
surface_notify_handler(Display * dpy, unsigned int uid, int kind)
{

   switch (kind) {
   case AppleDRISurfaceNotifyDestroyed:
      apple_glx_diagnostic("%s: surface destroyed %u\n", __func__, uid);
      apple_glx_surface_destroy(uid);
      break;

   case AppleDRISurfaceNotifyChanged:{
         int updated;

         updated = apple_glx_context_surface_changed(uid, pthread_self());

         apple_glx_diagnostic("surface notify updated %d\n", updated);
      }
      break;

   default:
      fprintf(stderr, "unhandled kind of event: %d in %s\n", kind, __func__);
   }
}

xp_client_id
apple_glx_get_client_id(void)
{
   static xp_client_id id;

   if (0 == id) {
      if ((XP_Success != xp_init(XP_IN_BACKGROUND)) ||
          (Success != xp_get_client_id(&id))) {
         return 0;
      }
   }

   return id;
}

/* Return true if an error occurred. */
bool
apple_init_glx(Display * dpy)
{
   int eventBase, errorBase;
   int major, minor, patch;

   if (!XAppleDRIQueryExtension(dpy, &eventBase, &errorBase))
      return true;

   if (!XAppleDRIQueryVersion(dpy, &major, &minor, &patch))
      return true;

   if (initialized)
      return false;

   apple_glx_log_init();

   apple_glx_log(ASL_LEVEL_INFO, "Initializing libGL.");

   apple_cgl_init();
   (void) apple_glx_get_client_id();

   XAppleDRISetSurfaceNotifyHandler(surface_notify_handler);

   /* This should really be per display. */
   dri_event_base = eventBase;
   initialized = true;

   return false;
}

void
apple_glx_swap_buffers(void *ptr)
{
   struct apple_glx_context *ac = ptr;

   apple_cgl.flush_drawable(ac->context_obj);
}

void
apple_glx_waitx(Display * dpy, void *ptr)
{
   struct apple_private_context *ac = ptr;

   (void) ac;

   glFlush();
   glFinish();
   XSync(dpy, False);
}
