/*
 Copyright (c) 2009 Apple Inc.
 
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
#include <pthread.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include "apple_glx.h"
#include "apple_cgl.h"
#include "apple_visual.h"
#include "apple_glx_drawable.h"
#include "appledri.h"
#include "glxconfig.h"

static bool pixmap_make_current(struct apple_glx_context *ac,
                                struct apple_glx_drawable *d);

static void pixmap_destroy(Display * dpy, struct apple_glx_drawable *d);

static struct apple_glx_drawable_callbacks callbacks = {
   .type = APPLE_GLX_DRAWABLE_PIXMAP,
   .make_current = pixmap_make_current,
   .destroy = pixmap_destroy
};

static bool
pixmap_make_current(struct apple_glx_context *ac,
                    struct apple_glx_drawable *d)
{
   CGLError cglerr;
   struct apple_glx_pixmap *p = &d->types.pixmap;

   assert(APPLE_GLX_DRAWABLE_PIXMAP == d->type);

   cglerr = apple_cgl.set_current_context(p->context_obj);

   if (kCGLNoError != cglerr) {
      fprintf(stderr, "set current context: %s\n",
              apple_cgl.error_string(cglerr));
      return true;
   }

   cglerr = apple_cgl.set_off_screen(p->context_obj, p->width, p->height,
                                     p->pitch, p->buffer);

   if (kCGLNoError != cglerr) {
      fprintf(stderr, "set off screen: %s\n", apple_cgl.error_string(cglerr));

      return true;
   }

   if (!ac->made_current) {
      apple_glapi_oglfw_viewport_scissor(0, 0, p->width, p->height);
      ac->made_current = true;
   }

   return false;
}

static void
pixmap_destroy(Display * dpy, struct apple_glx_drawable *d)
{
   struct apple_glx_pixmap *p = &d->types.pixmap;

   if (p->pixel_format_obj)
      (void) apple_cgl.destroy_pixel_format(p->pixel_format_obj);

   if (p->context_obj)
      (void) apple_cgl.destroy_context(p->context_obj);

   XAppleDRIDestroyPixmap(dpy, p->xpixmap);

   if (p->buffer) {
      if (munmap(p->buffer, p->size))
         perror("munmap");

      if (-1 == close(p->fd))
         perror("close");

      if (shm_unlink(p->path))
         perror("shm_unlink");
   }

   apple_glx_diagnostic("destroyed pixmap buffer for: 0x%lx\n", d->drawable);
}

/* Return true if an error occurred. */
bool
apple_glx_pixmap_create(Display * dpy, int screen, Pixmap pixmap,
                        const void *mode)
{
   struct apple_glx_drawable *d;
   struct apple_glx_pixmap *p;
   bool double_buffered;
   bool uses_stereo;
   CGLError error;
   const struct glx_config *cmodes = mode;

   if (apple_glx_drawable_create(dpy, screen, pixmap, &d, &callbacks))
      return true;

   /* d is locked and referenced at this point. */

   p = &d->types.pixmap;

   p->xpixmap = pixmap;
   p->buffer = NULL;

   if (!XAppleDRICreatePixmap(dpy, screen, pixmap,
                              &p->width, &p->height, &p->pitch, &p->bpp,
                              &p->size, p->path, PATH_MAX)) {
      d->unlock(d);
      d->destroy(d);
      return true;
   }

   p->fd = shm_open(p->path, O_RDWR, 0);

   if (p->fd < 0) {
      perror("shm_open");
      d->unlock(d);
      d->destroy(d);
      return true;
   }

   p->buffer = mmap(NULL, p->size, PROT_READ | PROT_WRITE,
                    MAP_FILE | MAP_SHARED, p->fd, 0);

   if (MAP_FAILED == p->buffer) {
      perror("mmap");
      d->unlock(d);
      d->destroy(d);
      return true;
   }

   apple_visual_create_pfobj(&p->pixel_format_obj, mode, &double_buffered,
                             &uses_stereo, /*offscreen */ true);

   error = apple_cgl.create_context(p->pixel_format_obj, NULL,
                                    &p->context_obj);

   if (kCGLNoError != error) {
      d->unlock(d);
      d->destroy(d);
      return true;
   }

   p->fbconfigID = cmodes->fbconfigID;

   d->unlock(d);

   apple_glx_diagnostic("created: pixmap buffer for 0x%lx\n", d->drawable);

   return false;
}

bool
apple_glx_pixmap_query(GLXPixmap pixmap, int attr, unsigned int *value)
{
   struct apple_glx_drawable *d;
   struct apple_glx_pixmap *p;
   bool result = false;

   d = apple_glx_drawable_find_by_type(pixmap, APPLE_GLX_DRAWABLE_PIXMAP,
                                       APPLE_GLX_DRAWABLE_LOCK);

   if (d) {
      p = &d->types.pixmap;

      switch (attr) {
      case GLX_WIDTH:
         *value = p->width;
         result = true;
         break;

      case GLX_HEIGHT:
         *value = p->height;
         result = true;
         break;

      case GLX_FBCONFIG_ID:
         *value = p->fbconfigID;
         result = true;
         break;
      }

      d->unlock(d);
   }

   return result;
}

/* Return true if the type is valid for pixmap. */
bool
apple_glx_pixmap_destroy(Display * dpy, GLXPixmap pixmap)
{
   return !apple_glx_drawable_destroy_by_type(dpy, pixmap,
                                              APPLE_GLX_DRAWABLE_PIXMAP);
}
