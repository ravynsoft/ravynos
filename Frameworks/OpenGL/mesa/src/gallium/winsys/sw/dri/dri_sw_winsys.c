/**************************************************************************
 *
 * Copyright 2009, VMware, Inc.
 * All Rights Reserved.
 * Copyright 2010 George Sapountzis <gsapountzis@gmail.com>
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifdef HAVE_SYS_SHM_H
#include <sys/ipc.h>
#include <sys/shm.h>
#ifdef __FreeBSD__
/* sys/ipc.h -> sys/_types.h -> machine/param.h
 * - defines ALIGN which clashes with our ALIGN
 */
#undef ALIGN
#endif
#endif

#include "util/compiler.h"
#include "util/format/u_formats.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"

#include "frontend/sw_winsys.h"
#include "dri_sw_winsys.h"


struct dri_sw_displaytarget
{
   enum pipe_format format;
   unsigned width;
   unsigned height;
   unsigned stride;

   unsigned map_flags;
   int shmid;
   void *data;
   void *mapped;
   const void *front_private;
};

struct dri_sw_winsys
{
   struct sw_winsys base;

   const struct drisw_loader_funcs *lf;
};

static inline struct dri_sw_displaytarget *
dri_sw_displaytarget( struct sw_displaytarget *dt )
{
   return (struct dri_sw_displaytarget *)dt;
}

static inline struct dri_sw_winsys *
dri_sw_winsys( struct sw_winsys *ws )
{
   return (struct dri_sw_winsys *)ws;
}


static bool
dri_sw_is_displaytarget_format_supported( struct sw_winsys *ws,
                                          unsigned tex_usage,
                                          enum pipe_format format )
{
   /* TODO: check visuals or other sensible thing here */
   return true;
}

#ifdef HAVE_SYS_SHM_H
static char *
alloc_shm(struct dri_sw_displaytarget *dri_sw_dt, unsigned size)
{
   char *addr;

   /* 0600 = user read+write */
   dri_sw_dt->shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0600);
   if (dri_sw_dt->shmid < 0)
      return NULL;

   addr = (char *) shmat(dri_sw_dt->shmid, NULL, 0);
   /* mark the segment immediately for deletion to avoid leaks */
   shmctl(dri_sw_dt->shmid, IPC_RMID, NULL);

   if (addr == (char *) -1)
      return NULL;

   return addr;
}
#endif

static struct sw_displaytarget *
dri_sw_displaytarget_create(struct sw_winsys *winsys,
                            unsigned tex_usage,
                            enum pipe_format format,
                            unsigned width, unsigned height,
                            unsigned alignment,
                            const void *front_private,
                            unsigned *stride)
{
   UNUSED struct dri_sw_winsys *ws = dri_sw_winsys(winsys);
   struct dri_sw_displaytarget *dri_sw_dt;
   unsigned nblocksy, size, format_stride;

   dri_sw_dt = CALLOC_STRUCT(dri_sw_displaytarget);
   if(!dri_sw_dt)
      goto no_dt;

   dri_sw_dt->format = format;
   dri_sw_dt->width = width;
   dri_sw_dt->height = height;
   dri_sw_dt->front_private = front_private;

   format_stride = util_format_get_stride(format, width);
   dri_sw_dt->stride = align(format_stride, alignment);

   nblocksy = util_format_get_nblocksy(format, height);
   size = dri_sw_dt->stride * nblocksy;

   dri_sw_dt->shmid = -1;

#ifdef HAVE_SYS_SHM_H
   if (ws->lf->put_image_shm)
      dri_sw_dt->data = alloc_shm(dri_sw_dt, size);
#endif

   if(!dri_sw_dt->data)
      dri_sw_dt->data = align_malloc(size, alignment);

   if(!dri_sw_dt->data)
      goto no_data;

   *stride = dri_sw_dt->stride;
   return (struct sw_displaytarget *)dri_sw_dt;

no_data:
   FREE(dri_sw_dt);
no_dt:
   return NULL;
}

static void
dri_sw_displaytarget_destroy(struct sw_winsys *ws,
                             struct sw_displaytarget *dt)
{
   struct dri_sw_displaytarget *dri_sw_dt = dri_sw_displaytarget(dt);

   if (dri_sw_dt->shmid >= 0) {
#ifdef HAVE_SYS_SHM_H
      shmdt(dri_sw_dt->data);
      shmctl(dri_sw_dt->shmid, IPC_RMID, NULL);
#endif
   } else {
      align_free(dri_sw_dt->data);
   }

   FREE(dri_sw_dt);
}

static void *
dri_sw_displaytarget_map(struct sw_winsys *ws,
                         struct sw_displaytarget *dt,
                         unsigned flags)
{
   struct dri_sw_displaytarget *dri_sw_dt = dri_sw_displaytarget(dt);
   dri_sw_dt->mapped = dri_sw_dt->data;

   if (dri_sw_dt->front_private && (flags & PIPE_MAP_READ)) {
      struct dri_sw_winsys *dri_sw_ws = dri_sw_winsys(ws);
      dri_sw_ws->lf->get_image((void *)dri_sw_dt->front_private, 0, 0, dri_sw_dt->width, dri_sw_dt->height, dri_sw_dt->stride, dri_sw_dt->data);
   }
   dri_sw_dt->map_flags = flags;
   return dri_sw_dt->mapped;
}

static void
dri_sw_displaytarget_unmap(struct sw_winsys *ws,
                           struct sw_displaytarget *dt)
{
   struct dri_sw_displaytarget *dri_sw_dt = dri_sw_displaytarget(dt);
   if (dri_sw_dt->front_private && (dri_sw_dt->map_flags & PIPE_MAP_WRITE)) {
      struct dri_sw_winsys *dri_sw_ws = dri_sw_winsys(ws);
      dri_sw_ws->lf->put_image2((void *)dri_sw_dt->front_private, dri_sw_dt->data, 0, 0, dri_sw_dt->width, dri_sw_dt->height, dri_sw_dt->stride);
   }
   dri_sw_dt->map_flags = 0;
   dri_sw_dt->mapped = NULL;
}

static struct sw_displaytarget *
dri_sw_displaytarget_from_handle(struct sw_winsys *winsys,
                                 const struct pipe_resource *templ,
                                 struct winsys_handle *whandle,
                                 unsigned *stride)
{
   assert(0);
   return NULL;
}

static bool
dri_sw_displaytarget_get_handle(struct sw_winsys *winsys,
                                struct sw_displaytarget *dt,
                                struct winsys_handle *whandle)
{
   struct dri_sw_displaytarget *dri_sw_dt = dri_sw_displaytarget(dt);

   if (whandle->type == WINSYS_HANDLE_TYPE_SHMID) {
      if (dri_sw_dt->shmid < 0)
         return false;
      whandle->handle = dri_sw_dt->shmid;
      return true;
   }

   return false;
}

static void
dri_sw_displaytarget_display(struct sw_winsys *ws,
                             struct sw_displaytarget *dt,
                             void *context_private,
                             struct pipe_box *box)
{
   struct dri_sw_winsys *dri_sw_ws = dri_sw_winsys(ws);
   struct dri_sw_displaytarget *dri_sw_dt = dri_sw_displaytarget(dt);
   struct dri_drawable *dri_drawable = (struct dri_drawable *)context_private;
   unsigned width, height, x = 0, y = 0;
   unsigned blsize = util_format_get_blocksize(dri_sw_dt->format);
   unsigned offset = 0;
   unsigned offset_x = 0;
   char *data = dri_sw_dt->data;
   bool is_shm = dri_sw_dt->shmid != -1;
   /* Set the width to 'stride / cpp'.
    *
    * PutImage correctly clips to the width of the dst drawable.
    */
   if (box) {
      offset = dri_sw_dt->stride * box->y;
      offset_x = box->x * blsize;
      data += offset;
      /* don't add x offset for shm, the put_image_shm will deal with it */
      if (!is_shm)
         data += offset_x;
      x = box->x;
      y = box->y;
      width = box->width;
      height = box->height;
   } else {
      width = dri_sw_dt->stride / blsize;
      height = dri_sw_dt->height;
   }

   if (is_shm) {
      dri_sw_ws->lf->put_image_shm(dri_drawable, dri_sw_dt->shmid, dri_sw_dt->data, offset, offset_x,
                                   x, y, width, height, dri_sw_dt->stride);
      return;
   }

   if (box)
      dri_sw_ws->lf->put_image2(dri_drawable, data,
                                x, y, width, height, dri_sw_dt->stride);
   else
      dri_sw_ws->lf->put_image(dri_drawable, data, width, height);
}

static void
dri_destroy_sw_winsys(struct sw_winsys *winsys)
{
   FREE(winsys);
}

struct sw_winsys *
dri_create_sw_winsys(const struct drisw_loader_funcs *lf)
{
   struct dri_sw_winsys *ws;

   ws = CALLOC_STRUCT(dri_sw_winsys);
   if (!ws)
      return NULL;

   ws->lf = lf;
   ws->base.destroy = dri_destroy_sw_winsys;

   ws->base.is_displaytarget_format_supported = dri_sw_is_displaytarget_format_supported;

   /* screen texture functions */
   ws->base.displaytarget_create = dri_sw_displaytarget_create;
   ws->base.displaytarget_destroy = dri_sw_displaytarget_destroy;
   ws->base.displaytarget_from_handle = dri_sw_displaytarget_from_handle;
   ws->base.displaytarget_get_handle = dri_sw_displaytarget_get_handle;

   /* texture functions */
   ws->base.displaytarget_map = dri_sw_displaytarget_map;
   ws->base.displaytarget_unmap = dri_sw_displaytarget_unmap;

   ws->base.displaytarget_display = dri_sw_displaytarget_display;

   return &ws->base;
}

/* vim: set sw=3 ts=8 sts=3 expandtab: */
