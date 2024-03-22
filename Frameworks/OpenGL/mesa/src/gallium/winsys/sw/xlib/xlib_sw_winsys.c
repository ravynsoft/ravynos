/**************************************************************************
 * 
 * Copyright 2007 VMware, Inc., Bismarck, ND., USA
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE 
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 * 
 * 
 **************************************************************************/

/*
 * Authors:
 *   Keith Whitwell
 *   Brian Paul
 */

#include "util/format/u_formats.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "frontend/xlibsw_api.h"
#include "xlib_sw_winsys.h"

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>

DEBUG_GET_ONCE_BOOL_OPTION(xlib_no_shm, "XLIB_NO_SHM", false)

/**
 * Display target for Xlib winsys.
 * Low-level OS/window system memory buffer
 */
struct xlib_displaytarget
{
   enum pipe_format format;
   unsigned width;
   unsigned height;
   unsigned stride;

   void *data;
   void *mapped;

   Display *display;
   Visual *visual;
   XImage *tempImage;
   GC gc;

   /* This is the last drawable that this display target was presented
    * against.  May need to recreate gc, tempImage when this changes??
    */
   Drawable drawable;

   XShmSegmentInfo shminfo;
   Bool shm;  /** Using shared memory images? */
};


/**
 * Subclass of sw_winsys for Xlib winsys
 */
struct xlib_sw_winsys
{
   struct sw_winsys base;
   Display *display;
};



/** Cast wrapper */
static inline struct xlib_displaytarget *
xlib_displaytarget(struct sw_displaytarget *dt)
{
   return (struct xlib_displaytarget *) dt;
}


/**
 * X Shared Memory Image extension code
 */

static volatile int XErrorFlag = 0;

/**
 * Catches potential Xlib errors.
 */
static int
handle_xerror(Display *dpy, XErrorEvent *event)
{
   (void) dpy;
   (void) event;
   XErrorFlag = 1;
   return 0;
}


static char *
alloc_shm(struct xlib_displaytarget *buf, unsigned size)
{
   XShmSegmentInfo *const shminfo = & buf->shminfo;

   shminfo->shmid = -1;
   shminfo->shmaddr = (char *) -1;

   /* 0600 = user read+write */
   shminfo->shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0600);
   if (shminfo->shmid < 0) {
      return NULL;
   }

   shminfo->shmaddr = (char *) shmat(shminfo->shmid, 0, 0);
   if (shminfo->shmaddr == (char *) -1) {
      shmctl(shminfo->shmid, IPC_RMID, 0);
      return NULL;
   }

   shmctl(shminfo->shmid, IPC_RMID, 0);
   shminfo->readOnly = False;
   return shminfo->shmaddr;
}


/**
 * Allocate a shared memory XImage back buffer for the given display target.
 */
static void
alloc_shm_ximage(struct xlib_displaytarget *xlib_dt,
                 struct xlib_drawable *xmb,
                 unsigned width, unsigned height)
{
   /*
    * We have to do a _lot_ of error checking here to be sure we can
    * really use the XSHM extension.  It seems different servers trigger
    * errors at different points if the extension won't work.  Therefore
    * we have to be very careful...
    */
   int (*old_handler)(Display *, XErrorEvent *);

   xlib_dt->tempImage = XShmCreateImage(xlib_dt->display,
                                      xmb->visual,
                                      xmb->depth,
                                      ZPixmap,
                                      NULL,
                                      &xlib_dt->shminfo,
                                      width, height);
   if (xlib_dt->tempImage == NULL) {
      shmctl(xlib_dt->shminfo.shmid, IPC_RMID, 0);
      xlib_dt->shm = False;
      return;
   }


   XErrorFlag = 0;
   old_handler = XSetErrorHandler(handle_xerror);
   /* This may trigger the X protocol error we're ready to catch: */
   XShmAttach(xlib_dt->display, &xlib_dt->shminfo);
   XSync(xlib_dt->display, False);

   /* Mark the segment to be destroyed, so that it is automatically destroyed
    * when this process dies.  Needs to be after XShmAttach() for *BSD.
    */
   shmctl(xlib_dt->shminfo.shmid, IPC_RMID, 0);

   if (XErrorFlag) {
      /* we are on a remote display, this error is normal, don't print it */
      XFlush(xlib_dt->display);
      XErrorFlag = 0;
      XDestroyImage(xlib_dt->tempImage);
      xlib_dt->tempImage = NULL;
      xlib_dt->shm = False;
      (void) XSetErrorHandler(old_handler);
      return;
   }

   xlib_dt->shm = True;
}


static void
alloc_ximage(struct xlib_displaytarget *xlib_dt,
             struct xlib_drawable *xmb,
             unsigned width, unsigned height)
{
   /* try allocating a shared memory image first */
   if (xlib_dt->shm) {
      alloc_shm_ximage(xlib_dt, xmb, width, height);
      if (xlib_dt->tempImage)
         return; /* success */
   }

   /* try regular (non-shared memory) image */
   xlib_dt->tempImage = XCreateImage(xlib_dt->display,
                                   xmb->visual,
                                   xmb->depth,
                                   ZPixmap, 0,
                                   NULL, width, height,
                                   8, 0);
}

static bool
xlib_is_displaytarget_format_supported(struct sw_winsys *ws,
                                       unsigned tex_usage,
                                       enum pipe_format format)
{
   /* TODO: check visuals or other sensible thing here */
   return true;
}


static void *
xlib_displaytarget_map(struct sw_winsys *ws,
                       struct sw_displaytarget *dt,
                       unsigned flags)
{
   struct xlib_displaytarget *xlib_dt = xlib_displaytarget(dt);
   xlib_dt->mapped = xlib_dt->data;
   return xlib_dt->mapped;
}


static void
xlib_displaytarget_unmap(struct sw_winsys *ws,
                         struct sw_displaytarget *dt)
{
   struct xlib_displaytarget *xlib_dt = xlib_displaytarget(dt);
   xlib_dt->mapped = NULL;
}


static void
xlib_displaytarget_destroy(struct sw_winsys *ws,
                           struct sw_displaytarget *dt)
{
   struct xlib_displaytarget *xlib_dt = xlib_displaytarget(dt);

   if (xlib_dt->data) {
      if (xlib_dt->shminfo.shmid >= 0) {
         shmdt(xlib_dt->shminfo.shmaddr);
         shmctl(xlib_dt->shminfo.shmid, IPC_RMID, 0);
         
         xlib_dt->shminfo.shmid = -1;
         xlib_dt->shminfo.shmaddr = (char *) -1;

         xlib_dt->data = NULL;
         if (xlib_dt->tempImage)
            xlib_dt->tempImage->data = NULL;
      }
      else {
         align_free(xlib_dt->data);
         if (xlib_dt->tempImage && xlib_dt->tempImage->data == xlib_dt->data) {
            xlib_dt->tempImage->data = NULL;
         }
         xlib_dt->data = NULL;
      }
   }

   if (xlib_dt->tempImage) {
      XDestroyImage(xlib_dt->tempImage);
      xlib_dt->tempImage = NULL;
   }

   if (xlib_dt->gc)
      XFreeGC(xlib_dt->display, xlib_dt->gc);

   FREE(xlib_dt);
}


/**
 * Display/copy the image in the surface into the X window specified
 * by the display target.
 */
static void
xlib_sw_display(struct xlib_drawable *xlib_drawable,
                struct sw_displaytarget *dt,
                struct pipe_box *box)
{
   static bool no_swap = false;
   static bool firsttime = true;
   struct xlib_displaytarget *xlib_dt = xlib_displaytarget(dt);
   Display *display = xlib_dt->display;
   XImage *ximage;
   struct pipe_box _box = {};

   if (firsttime) {
      no_swap = getenv("SP_NO_RAST") != NULL;
      firsttime = 0;
   }

   if (no_swap)
      return;

   if (!box) {
      _box.width = xlib_dt->width;
      _box.height = xlib_dt->height;
      box = &_box;
   }

   if (xlib_dt->drawable != xlib_drawable->drawable) {
      if (xlib_dt->gc) {
         XFreeGC(display, xlib_dt->gc);
         xlib_dt->gc = NULL;
      }

      if (xlib_dt->tempImage) {
         XDestroyImage(xlib_dt->tempImage);
         xlib_dt->tempImage = NULL;
      }

      xlib_dt->drawable = xlib_drawable->drawable;
   }

   if (xlib_dt->tempImage == NULL) {
      assert(util_format_get_blockwidth(xlib_dt->format) == 1);
      assert(util_format_get_blockheight(xlib_dt->format) == 1);
      alloc_ximage(xlib_dt, xlib_drawable,
                   xlib_dt->stride / util_format_get_blocksize(xlib_dt->format),
                   xlib_dt->height);
      if (!xlib_dt->tempImage)
         return;
   }

   if (xlib_dt->gc == NULL) {
      xlib_dt->gc = XCreateGC(display, xlib_drawable->drawable, 0, NULL);
      XSetFunction(display, xlib_dt->gc, GXcopy);
   }

   if (xlib_dt->shm) {
      ximage = xlib_dt->tempImage;
      ximage->data = xlib_dt->data;

      /* _debug_printf("XSHM\n"); */
      XShmPutImage(xlib_dt->display, xlib_drawable->drawable, xlib_dt->gc,
                   ximage, box->x, box->y, box->x, box->y,
                   box->width, box->height, False);
   }
   else {
      /* display image in Window */
      ximage = xlib_dt->tempImage;
      ximage->data = xlib_dt->data;

      /* check that the XImage has been previously initialized */
      assert(ximage->format);
      assert(ximage->bitmap_unit);

      /* update XImage's fields */
      ximage->width = xlib_dt->width;
      ximage->height = xlib_dt->height;
      ximage->bytes_per_line = xlib_dt->stride;

      /* _debug_printf("XPUT\n"); */
      XPutImage(xlib_dt->display, xlib_drawable->drawable, xlib_dt->gc,
                ximage, box->x, box->y, box->x, box->y,
                box->width, box->height);
   }

   XFlush(xlib_dt->display);
}


/**
 * Display/copy the image in the surface into the X window specified
 * by the display target.
 */
static void
xlib_displaytarget_display(struct sw_winsys *ws,
                           struct sw_displaytarget *dt,
                           void *context_private,
                           struct pipe_box *box)
{
   struct xlib_drawable *xlib_drawable = (struct xlib_drawable *)context_private;
   xlib_sw_display(xlib_drawable, dt, box);
}


static struct sw_displaytarget *
xlib_displaytarget_create(struct sw_winsys *winsys,
                          unsigned tex_usage,
                          enum pipe_format format,
                          unsigned width, unsigned height,
                          unsigned alignment,
                          const void *front_private,
                          unsigned *stride)
{
   struct xlib_displaytarget *xlib_dt;
   unsigned nblocksy, size;
   int ignore;

   xlib_dt = CALLOC_STRUCT(xlib_displaytarget);
   if (!xlib_dt)
      goto no_xlib_dt;

   xlib_dt->display = ((struct xlib_sw_winsys *)winsys)->display;
   xlib_dt->format = format;
   xlib_dt->width = width;
   xlib_dt->height = height;

   nblocksy = util_format_get_nblocksy(format, height);
   xlib_dt->stride = align(util_format_get_stride(format, width), alignment);
   size = xlib_dt->stride * nblocksy;

   if (!debug_get_option_xlib_no_shm() &&
       XQueryExtension(xlib_dt->display, "MIT-SHM", &ignore, &ignore, &ignore)) {
      xlib_dt->data = alloc_shm(xlib_dt, size);
      if (xlib_dt->data) {
         xlib_dt->shm = True;
      }
   }

   if (!xlib_dt->data) {
      xlib_dt->data = align_malloc(size, alignment);
      if (!xlib_dt->data)
         goto no_data;
   }

   *stride = xlib_dt->stride;
   return (struct sw_displaytarget *)xlib_dt;

no_data:
   FREE(xlib_dt);
no_xlib_dt:
   return NULL;
}


static struct sw_displaytarget *
xlib_displaytarget_from_handle(struct sw_winsys *winsys,
                               const struct pipe_resource *templet,
                               struct winsys_handle *whandle,
                               unsigned *stride)
{
   assert(0);
   return NULL;
}


static bool
xlib_displaytarget_get_handle(struct sw_winsys *winsys,
                              struct sw_displaytarget *dt,
                              struct winsys_handle *whandle)
{
   assert(0);
   return false;
}


static void
xlib_destroy(struct sw_winsys *ws)
{
   FREE(ws);
}


struct sw_winsys *
xlib_create_sw_winsys(Display *display)
{
   struct xlib_sw_winsys *ws;

   ws = CALLOC_STRUCT(xlib_sw_winsys);
   if (!ws)
      return NULL;

   ws->display = display;
   ws->base.destroy = xlib_destroy;

   ws->base.is_displaytarget_format_supported = xlib_is_displaytarget_format_supported;

   ws->base.displaytarget_create = xlib_displaytarget_create;
   ws->base.displaytarget_from_handle = xlib_displaytarget_from_handle;
   ws->base.displaytarget_get_handle = xlib_displaytarget_get_handle;
   ws->base.displaytarget_map = xlib_displaytarget_map;
   ws->base.displaytarget_unmap = xlib_displaytarget_unmap;
   ws->base.displaytarget_destroy = xlib_displaytarget_destroy;

   ws->base.displaytarget_display = xlib_displaytarget_display;

   return &ws->base;
}
