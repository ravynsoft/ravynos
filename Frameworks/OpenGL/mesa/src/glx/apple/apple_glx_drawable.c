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
#include <assert.h>
#include <pthread.h>
#include <string.h>
#include "apple_glx.h"
#include "apple_glx_context.h"
#include "apple_glx_drawable.h"
#include "appledri.h"

static pthread_mutex_t drawables_lock = PTHREAD_MUTEX_INITIALIZER;
static struct apple_glx_drawable *drawables_list = NULL;

static void
lock_drawables_list(void)
{
   int err;

   err = pthread_mutex_lock(&drawables_lock);

   if (err) {
      fprintf(stderr, "pthread_mutex_lock failure in %s: %s\n",
              __func__, strerror(err));
      abort();
   }
}

static void
unlock_drawables_list(void)
{
   int err;

   err = pthread_mutex_unlock(&drawables_lock);

   if (err) {
      fprintf(stderr, "pthread_mutex_unlock failure in %s: %s\n",
              __func__, strerror(err));
      abort();
   }
}

struct apple_glx_drawable *
apple_glx_find_drawable(Display * dpy, GLXDrawable drawable)
{
   struct apple_glx_drawable *i, *agd = NULL;

   lock_drawables_list();

   for (i = drawables_list; i; i = i->next) {
      if (i->drawable == drawable) {
         agd = i;
         break;
      }
   }

   unlock_drawables_list();

   return agd;
}

static void
drawable_lock(struct apple_glx_drawable *agd)
{
   int err;

   err = pthread_mutex_lock(&agd->mutex);

   if (err) {
      fprintf(stderr, "pthread_mutex_lock error: %s\n", strerror(err));
      abort();
   }
}

static void
drawable_unlock(struct apple_glx_drawable *d)
{
   int err;

   err = pthread_mutex_unlock(&d->mutex);

   if (err) {
      fprintf(stderr, "pthread_mutex_unlock error: %s\n", strerror(err));
      abort();
   }
}


static void
reference_drawable(struct apple_glx_drawable *d)
{
   d->lock(d);
   d->reference_count++;
   d->unlock(d);
}

static void
release_drawable(struct apple_glx_drawable *d)
{
   d->lock(d);
   d->reference_count--;
   d->unlock(d);
}

/* The drawables list must be locked prior to calling this. */
/* Return true if the drawable was destroyed. */
static bool
destroy_drawable(struct apple_glx_drawable *d)
{
   int err;

   d->lock(d);

   if (d->reference_count > 0) {
      d->unlock(d);
      return false;
   }

   d->unlock(d);

   if (d->previous) {
      d->previous->next = d->next;
   }
   else {
      /*
       * The item must be at the head of the list, if it
       * has no previous pointer. 
       */
      drawables_list = d->next;
   }

   if (d->next)
      d->next->previous = d->previous;

   unlock_drawables_list();

   if (d->callbacks.destroy) {
      /*
       * Warning: this causes other routines to be called (potentially)
       * from surface_notify_handler.  It's probably best to not have
       * any locks at this point locked.
       */
      d->callbacks.destroy(d->display, d);
   }

   apple_glx_diagnostic("%s: freeing %p\n", __func__, (void *) d);

   /* Stupid recursive locks */
   while (pthread_mutex_unlock(&d->mutex) == 0);

   err = pthread_mutex_destroy(&d->mutex);
   if (err) {
      fprintf(stderr, "pthread_mutex_destroy error: %s\n", strerror(err));
      abort();
   }
   
   free(d);

   /* So that the locks are balanced and the caller correctly unlocks. */
   lock_drawables_list();

   return true;
}

/*
 * This is typically called when a context is destroyed or the current
 * drawable is made None.
 */
static bool
destroy_drawable_callback(struct apple_glx_drawable *d)
{
   bool result;

   d->lock(d);

   apple_glx_diagnostic("%s: %p ->reference_count before -- %d\n", __func__,
                        (void *) d, d->reference_count);

   d->reference_count--;

   if (d->reference_count > 0) {
      d->unlock(d);
      return false;
   }

   d->unlock(d);

   lock_drawables_list();

   result = destroy_drawable(d);

   unlock_drawables_list();

   return result;
}

static bool
is_pbuffer(struct apple_glx_drawable *d)
{
   return APPLE_GLX_DRAWABLE_PBUFFER == d->type;
}

static bool
is_pixmap(struct apple_glx_drawable *d)
{
   return APPLE_GLX_DRAWABLE_PIXMAP == d->type;
}

static void
common_init(Display * dpy, GLXDrawable drawable, struct apple_glx_drawable *d)
{
   int err;
   pthread_mutexattr_t attr;

   d->display = dpy;
   d->reference_count = 0;
   d->drawable = drawable;
   d->type = -1;

   err = pthread_mutexattr_init(&attr);

   if (err) {
      fprintf(stderr, "pthread_mutexattr_init error: %s\n", strerror(err));
      abort();
   }

   /* 
    * There are some patterns that require a recursive mutex,
    * when working with locks that protect the apple_glx_drawable,
    * and reference functions like ->reference, and ->release.
    */
   err = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

   if (err) {
      fprintf(stderr, "error: setting pthread mutex type: %s\n", strerror(err));
      abort();
   }

   err = pthread_mutex_init(&d->mutex, &attr);

   if (err) {
      fprintf(stderr, "pthread_mutex_init error: %s\n", strerror(err));
      abort();
   }

   (void) pthread_mutexattr_destroy(&attr);

   d->lock = drawable_lock;
   d->unlock = drawable_unlock;

   d->reference = reference_drawable;
   d->release = release_drawable;

   d->destroy = destroy_drawable_callback;

   d->is_pbuffer = is_pbuffer;
   d->is_pixmap = is_pixmap;

   d->width = -1;
   d->height = -1;
   d->row_bytes = 0;
   d->path[0] = '\0';
   d->fd = -1;
   d->buffer = NULL;
   d->buffer_length = 0;

   d->previous = NULL;
   d->next = NULL;
}

static void
link_tail(struct apple_glx_drawable *agd)
{
   lock_drawables_list();

   /* Link the new drawable into the global list. */
   agd->next = drawables_list;

   if (drawables_list)
      drawables_list->previous = agd;

   drawables_list = agd;

   unlock_drawables_list();
}

/*WARNING: this returns a locked and referenced object. */
bool
apple_glx_drawable_create(Display * dpy,
                          int screen,
                          GLXDrawable drawable,
                          struct apple_glx_drawable **agdResult,
                          struct apple_glx_drawable_callbacks *callbacks)
{
   struct apple_glx_drawable *d;

   d = calloc(1, sizeof *d);

   if (NULL == d) {
      perror("malloc");
      return true;
   }

   common_init(dpy, drawable, d);
   d->type = callbacks->type;
   d->callbacks = *callbacks;

   d->reference(d);
   d->lock(d);

   link_tail(d);

   apple_glx_diagnostic("%s: new drawable %p\n", __func__, (void *) d);

   *agdResult = d;

   return false;
}

static int error_count = 0;

static int
error_handler(Display * dpy, XErrorEvent * err)
{
   if (err->error_code == BadWindow) {
      ++error_count;
   }

   return 0;
}

void
apple_glx_garbage_collect_drawables(Display * dpy)
{
   struct apple_glx_drawable *d, *dnext;
   Window root;
   int x, y;
   unsigned int width, height, bd, depth;
   int (*old_handler) (Display *, XErrorEvent *);


   if (NULL == drawables_list)
      return;

   old_handler = XSetErrorHandler(error_handler);

   XSync(dpy, False);

   lock_drawables_list();

   for (d = drawables_list; d;) {
      dnext = d->next;

      d->lock(d);

      if (d->reference_count > 0) {
         /* 
          * Skip this, because some context still retains a reference 
          * to the drawable.
          */
         d->unlock(d);
         d = dnext;
         continue;
      }

      d->unlock(d);

      error_count = 0;

      /* 
       * Mesa uses XGetWindowAttributes, but some of these things are 
       * most definitely not Windows, and that's against the rules.
       * XGetGeometry on the other hand is legal with a Pixmap and Window.
       */
      XGetGeometry(dpy, d->drawable, &root, &x, &y, &width, &height, &bd,
                   &depth);

      if (error_count > 0) {
         /*
          * Note: this may not actually destroy the drawable.
          * If another context retains a reference to the drawable
          * after the reference count test above. 
          */
         (void) destroy_drawable(d);
         error_count = 0;
      }

      d = dnext;
   }

   XSetErrorHandler(old_handler);

   unlock_drawables_list();
}

unsigned int
apple_glx_get_drawable_count(void)
{
   unsigned int result = 0;
   struct apple_glx_drawable *d;

   lock_drawables_list();

   for (d = drawables_list; d; d = d->next)
      ++result;

   unlock_drawables_list();

   return result;
}

struct apple_glx_drawable *
apple_glx_drawable_find_by_type(GLXDrawable drawable, int type, int flags)
{
   struct apple_glx_drawable *d;

   lock_drawables_list();

   for (d = drawables_list; d; d = d->next) {
      if (d->type == type && d->drawable == drawable) {
         if (flags & APPLE_GLX_DRAWABLE_REFERENCE)
            d->reference(d);

         if (flags & APPLE_GLX_DRAWABLE_LOCK)
            d->lock(d);

         unlock_drawables_list();

         return d;
      }
   }

   unlock_drawables_list();

   return NULL;
}

struct apple_glx_drawable *
apple_glx_drawable_find(GLXDrawable drawable, int flags)
{
   struct apple_glx_drawable *d;

   lock_drawables_list();

   for (d = drawables_list; d; d = d->next) {
      if (d->drawable == drawable) {
         if (flags & APPLE_GLX_DRAWABLE_REFERENCE)
            d->reference(d);

         if (flags & APPLE_GLX_DRAWABLE_LOCK)
            d->lock(d);

         unlock_drawables_list();

         return d;
      }
   }

   unlock_drawables_list();

   return NULL;
}

/* Return true if the type is valid for the drawable. */
bool
apple_glx_drawable_destroy_by_type(Display * dpy,
                                   GLXDrawable drawable, int type)
{
   struct apple_glx_drawable *d;

   lock_drawables_list();

   for (d = drawables_list; d; d = d->next) {
      if (drawable == d->drawable && type == d->type) {
         /*
          * The user has requested that we destroy this resource.
          * However, there may be references in the contexts to it, so
          * release it, and call destroy_drawable which doesn't destroy
          * if the reference_count is > 0.
          */
         d->release(d);

         apple_glx_diagnostic("%s d->reference_count %d\n",
                              __func__, d->reference_count);

         destroy_drawable(d);
         unlock_drawables_list();
         return true;
      }
   }

   unlock_drawables_list();

   return false;
}

struct apple_glx_drawable *
apple_glx_drawable_find_by_uid(unsigned int uid, int flags)
{
   struct apple_glx_drawable *d;

   lock_drawables_list();

   for (d = drawables_list; d; d = d->next) {
      /* Only surfaces have a uid. */
      if (APPLE_GLX_DRAWABLE_SURFACE == d->type) {
         if (d->types.surface.uid == uid) {
            if (flags & APPLE_GLX_DRAWABLE_REFERENCE)
               d->reference(d);

            if (flags & APPLE_GLX_DRAWABLE_LOCK)
               d->lock(d);

            unlock_drawables_list();

            return d;
         }
      }
   }

   unlock_drawables_list();

   return NULL;
}
