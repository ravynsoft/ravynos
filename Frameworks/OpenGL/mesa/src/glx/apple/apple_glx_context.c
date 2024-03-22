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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

// Get the newer glext.h first
#include <GL/gl.h>
#include <GL/glext.h>

#include <OpenGL/CGLTypes.h>
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/OpenGL.h>

#include "glxclient.h"

#include "apple_glx.h"
#include "apple_glx_context.h"
#include "appledri.h"
#include "apple_visual.h"
#include "apple_cgl.h"
#include "apple_glx_drawable.h"

#include "util/u_debug.h"

static pthread_mutex_t context_lock = PTHREAD_MUTEX_INITIALIZER;

/*
 * This should be locked on creation and destruction of the 
 * apple_glx_contexts.
 *
 * It's also locked when the surface_notify_handler is searching
 * for a uid associated with a surface.
 */
static struct apple_glx_context *context_list = NULL;

/* This guards the context_list above. */
static void
lock_context_list(void)
{
   int err;

   err = pthread_mutex_lock(&context_lock);

   if (err) {
      fprintf(stderr, "pthread_mutex_lock failure in %s: %d\n",
              __func__, err);
      abort();
   }
}

static void
unlock_context_list(void)
{
   int err;

   err = pthread_mutex_unlock(&context_lock);

   if (err) {
      fprintf(stderr, "pthread_mutex_unlock failure in %s: %d\n",
              __func__, err);
      abort();
   }
}

static bool
is_context_valid(struct apple_glx_context *ac)
{
   struct apple_glx_context *i;

   lock_context_list();

   for (i = context_list; i; i = i->next) {
      if (ac == i) {
         unlock_context_list();
         return true;
      }
   }

   unlock_context_list();

   return false;
}

/* This creates an apple_private_context struct.  
 *
 * It's typically called to save the struct in a GLXContext.
 *
 * This is also where the CGLContextObj is created, and the CGLPixelFormatObj.
 */
bool
apple_glx_create_context(void **ptr, Display * dpy, int screen,
                         const void *mode, void *sharedContext,
                         int *errorptr, bool * x11errorptr)
{
   struct apple_glx_context *ac;
   struct apple_glx_context *sharedac = sharedContext;
   CGLError error;

   *ptr = NULL;

   ac = malloc(sizeof *ac);

   if (NULL == ac) {
      *errorptr = BadAlloc;
      *x11errorptr = true;
      return true;
   }

   if (sharedac && !is_context_valid(sharedac)) {
      *errorptr = GLXBadContext;
      *x11errorptr = false;
      free(ac);
      return true;
   }

   ac->context_obj = NULL;
   ac->pixel_format_obj = NULL;
   ac->drawable = NULL;
   ac->thread_id = pthread_self();
   ac->screen = screen;
   ac->double_buffered = false;
   ac->uses_stereo = false;
   ac->need_update = false;
   ac->is_current = false;
   ac->made_current = false;
   ac->last_surface_window = None;

   apple_visual_create_pfobj(&ac->pixel_format_obj, mode,
                             &ac->double_buffered, &ac->uses_stereo,
                             /*offscreen */ false);

   error = apple_cgl.create_context(ac->pixel_format_obj,
                                    sharedac ? sharedac->context_obj : NULL,
                                    &ac->context_obj);


   if (error) {
      (void) apple_cgl.destroy_pixel_format(ac->pixel_format_obj);

      free(ac);

      if (kCGLBadMatch == error) {
         *errorptr = BadMatch;
         *x11errorptr = true;
      }
      else {
         *errorptr = GLXBadContext;
         *x11errorptr = false;
      }

      DebugMessageF("error: %s\n", apple_cgl.error_string(error));

      return true;
   }

   /* The context creation succeeded, so we can link in the new context. */
   lock_context_list();

   if (context_list)
      context_list->previous = ac;

   ac->previous = NULL;
   ac->next = context_list;
   context_list = ac;

   *ptr = ac;

   apple_glx_diagnostic("%s: ac %p ac->context_obj %p\n",
                        __func__, (void *) ac, (void *) ac->context_obj);

   unlock_context_list();

   return false;
}

void
apple_glx_destroy_context(void **ptr, Display * dpy)
{
   struct apple_glx_context *ac = *ptr;

   if (NULL == ac)
      return;

   apple_glx_diagnostic("%s: ac %p ac->context_obj %p\n",
                        __func__, (void *) ac, (void *) ac->context_obj);

   if (apple_cgl.get_current_context() == ac->context_obj) {
      apple_glx_diagnostic("%s: context ac->context_obj %p "
                           "is still current!\n", __func__,
                           (void *) ac->context_obj);
      if (apple_cgl.set_current_context(NULL)) {
         abort();
      }
   }

   /* Remove ac from the context_list as soon as possible. */
   lock_context_list();

   if (ac->previous) {
      ac->previous->next = ac->next;
   }
   else {
      context_list = ac->next;
   }

   if (ac->next) {
      ac->next->previous = ac->previous;
   }

   unlock_context_list();


   if (apple_cgl.clear_drawable(ac->context_obj)) {
      fprintf(stderr, "error: while clearing drawable!\n");
      abort();
   }

   /*
    * This potentially causes surface_notify_handler to be called in
    * apple_glx.c... 
    * We can NOT have a lock held at this point.  It would result in 
    * an abort due to an attempted deadlock.  This is why we earlier
    * removed the ac pointer from the double-linked list.
    */
   if (ac->drawable) {
      ac->drawable->destroy(ac->drawable);
   }

   if (apple_cgl.destroy_pixel_format(ac->pixel_format_obj)) {
      fprintf(stderr, "error: destroying pixel format in %s\n", __func__);
      abort();
   }

   if (apple_cgl.destroy_context(ac->context_obj)) {
      fprintf(stderr, "error: destroying context_obj in %s\n", __func__);
      abort();
   }

   free(ac);

   *ptr = NULL;

   apple_glx_garbage_collect_drawables(dpy);
}


/* Return true if an error occurred. */
bool
apple_glx_make_current_context(Display * dpy, void *oldptr, void *ptr,
                               GLXDrawable drawable)
{
   struct apple_glx_context *oldac = oldptr;
   struct apple_glx_context *ac = ptr;
   struct apple_glx_drawable *newagd = NULL;
   CGLError cglerr;
   bool same_drawable = false;

#if 0
   apple_glx_diagnostic("%s: oldac %p ac %p drawable 0x%lx\n",
                        __func__, (void *) oldac, (void *) ac, drawable);

   apple_glx_diagnostic("%s: oldac->context_obj %p ac->context_obj %p\n",
                        __func__,
                        (void *) (oldac ? oldac->context_obj : NULL),
                        (void *) (ac ? ac->context_obj : NULL));
#endif

   /* This a common path for GLUT and other apps, so special case it. */
   if (ac && ac->drawable && ac->drawable->drawable == drawable) {
      same_drawable = true;

      if (ac->is_current)
         return false;
   }

   /* Reset the is_current state of the old context, if non-NULL. */
   if (oldac && (ac != oldac))
      oldac->is_current = false;

   if (NULL == ac) {
      /*Clear the current context for this thread. */
      apple_cgl.set_current_context(NULL);

      if (oldac) {
         oldac->is_current = false;

         if (oldac->drawable) {
            oldac->drawable->destroy(oldac->drawable);
            oldac->drawable = NULL;
         }

         /* Invalidate this to prevent surface recreation. */
         oldac->last_surface_window = None;
      }

      return false;
   }

   if (None == drawable) {
      bool error = false;

      /* Clear the current drawable for this context_obj. */

      if (apple_cgl.set_current_context(ac->context_obj))
         error = true;

      if (apple_cgl.clear_drawable(ac->context_obj))
         error = true;

      if (ac->drawable) {
         ac->drawable->destroy(ac->drawable);
         ac->drawable = NULL;
      }

      /* Invalidate this to prevent surface recreation. */
      ac->last_surface_window = None;

      apple_glx_diagnostic("%s: drawable is None, error is: %d\n",
                           __func__, error);

      return error;
   }

   /* This is an optimisation to avoid searching for the current drawable. */
   if (ac->drawable && ac->drawable->drawable == drawable) {
      newagd = ac->drawable;
   }
   else {
      /* Find the drawable if possible, and retain a reference to it. */
      newagd =
         apple_glx_drawable_find(drawable, APPLE_GLX_DRAWABLE_REFERENCE);
   }

   /*
    * Try to destroy the old drawable, so long as the new one
    * isn't the old. 
    */
   if (ac->drawable && !same_drawable) {
      ac->drawable->destroy(ac->drawable);
      ac->drawable = NULL;
   }

   if (NULL == newagd) {
      if (apple_glx_surface_create(dpy, ac->screen, drawable, &newagd))
         return true;

      /* The drawable is referenced once by apple_glx_surface_create. */

      /*
       * FIXME: We actually need 2 references to prevent premature surface 
       * destruction.  The problem is that the surface gets destroyed in 
       * the case of the context being reused for another window, and
       * we then lose the surface contents.  Wait for destruction of a
       * window to destroy a surface.
       *
       * Note: this may leave around surfaces we don't want around, if
       * say we are using X for raster drawing after OpenGL rendering, 
       * but it will be compatible with the old libGL's behavior.
       *
       * Someday the X11 and OpenGL rendering must be unified at some
       * layer.  I suspect we can do that via shared memory and 
       * multiple threads in the X server (1 for each context created
       * by a client).  This would also allow users to render from 
       * multiple clients to the same OpenGL surface.  In fact it could
       * all be OpenGL.
       *
       */
      newagd->reference(newagd);

      /* Save the new drawable with the context structure. */
      ac->drawable = newagd;
   }
   else {
      /* We are reusing an existing drawable structure. */

      if (same_drawable) {
         assert(ac->drawable == newagd);
         /* The drawable_find above retained a reference for us. */
      }
      else {
         ac->drawable = newagd;
      }
   }

   /* 
    * Avoid this costly path if this is the same drawable and the
    * context is already current. 
    */

   if (same_drawable && ac->is_current) {
      apple_glx_diagnostic("same_drawable and ac->is_current\n");
      return false;
   }

   cglerr = apple_cgl.set_current_context(ac->context_obj);

   if (kCGLNoError != cglerr) {
      fprintf(stderr, "set current error: %s\n",
              apple_cgl.error_string(cglerr));
      return true;
   }

   ac->is_current = true;

   assert(NULL != ac->context_obj);
   assert(NULL != ac->drawable);

   ac->thread_id = pthread_self();

   /* This will be set if the pending_destroy code indicates it should be: */
   ac->last_surface_window = None;

   switch (ac->drawable->type) {
   case APPLE_GLX_DRAWABLE_PBUFFER:
   case APPLE_GLX_DRAWABLE_SURFACE:
   case APPLE_GLX_DRAWABLE_PIXMAP:
      if (ac->drawable->callbacks.make_current) {
         if (ac->drawable->callbacks.make_current(ac, ac->drawable))
            return true;
      }
      break;

   default:
      fprintf(stderr, "internal error: invalid drawable type: %d\n",
              ac->drawable->type);
      abort();
   }

   return false;
}

bool
apple_glx_is_current_drawable(Display * dpy, void *ptr, GLXDrawable drawable)
{
   struct apple_glx_context *ac = ptr;

   if (ac->drawable && ac->drawable->drawable == drawable) {
      return true;
   }
   else if (NULL == ac->drawable && None != ac->last_surface_window) {
      apple_glx_context_update(dpy, ac);

      return (ac->drawable && ac->drawable->drawable == drawable);
   }

   return false;
}

bool
apple_glx_copy_context(void *currentptr, void *srcptr, void *destptr,
                       unsigned long mask, int *errorptr, bool * x11errorptr)
{
   struct apple_glx_context *src, *dest;
   CGLError err;

   src = srcptr;
   dest = destptr;

   if (src->screen != dest->screen) {
      *errorptr = BadMatch;
      *x11errorptr = true;
      return true;
   }

   if (dest == currentptr || dest->is_current) {
      *errorptr = BadAccess;
      *x11errorptr = true;
      return true;
   }

   /* 
    * If srcptr is the current context then we should do an implicit glFlush.
    */
   if (currentptr == srcptr)
      glFlush();

   err = apple_cgl.copy_context(src->context_obj, dest->context_obj,
                                (GLbitfield) mask);

   if (kCGLNoError != err) {
      *errorptr = GLXBadContext;
      *x11errorptr = false;
      return true;
   }

   return false;
}

/* 
 * The value returned is the total number of contexts set to update. 
 * It's meant for debugging/introspection.
 */
int
apple_glx_context_surface_changed(unsigned int uid, pthread_t caller)
{
   struct apple_glx_context *ac;
   int updated = 0;

   lock_context_list();

   for (ac = context_list; ac; ac = ac->next) {
      if (ac->drawable && APPLE_GLX_DRAWABLE_SURFACE == ac->drawable->type
          && ac->drawable->types.surface.uid == uid) {

         if (caller == ac->thread_id) {
            apple_glx_diagnostic("caller is the same thread for uid %u\n",
                                 uid);

            xp_update_gl_context(ac->context_obj);
         }
         else {
            ac->need_update = true;
            ++updated;
         }
      }
   }

   unlock_context_list();

   return updated;
}

void
apple_glx_context_update(Display * dpy, void *ptr)
{
   struct apple_glx_context *ac = ptr;

   if (NULL == ac->drawable && None != ac->last_surface_window) {
      bool failed;

      /* Attempt to recreate the surface for a destroyed drawable. */
      failed =
         apple_glx_make_current_context(dpy, ac, ac, ac->last_surface_window);

      apple_glx_diagnostic("%s: surface recreation failed? %s\n", __func__,
                           failed ? "YES" : "NO");
   }

   if (ac->need_update) {
      xp_update_gl_context(ac->context_obj);
      ac->need_update = false;

      apple_glx_diagnostic("%s: updating context %p\n", __func__, ptr);
   }

   if (ac->drawable && APPLE_GLX_DRAWABLE_SURFACE == ac->drawable->type
       && ac->drawable->types.surface.pending_destroy) {
      apple_glx_diagnostic("%s: clearing drawable %p\n", __func__, ptr);
      apple_cgl.clear_drawable(ac->context_obj);

      if (ac->drawable) {
         struct apple_glx_drawable *d;

         apple_glx_diagnostic("%s: attempting to destroy drawable %p\n",
                              __func__, ptr);
         apple_glx_diagnostic("%s: ac->drawable->drawable is 0x%lx\n",
                              __func__, ac->drawable->drawable);

         d = ac->drawable;

         ac->last_surface_window = d->drawable;

         ac->drawable = NULL;

         /* 
          * This will destroy the surface drawable if there are 
          * no references to it.  
          * It also subtracts 1 from the reference_count.
          * If there are references to it, then it's probably made
          * current in another context.
          */
         d->destroy(d);
      }
   }
}

bool
apple_glx_context_uses_stereo(void *ptr)
{
   struct apple_glx_context *ac = ptr;

   return ac->uses_stereo;
}
