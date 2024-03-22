/**********************************************************
 * Copyright 2008-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#ifndef SVGA_BUFFER_H
#define SVGA_BUFFER_H


#include "util/compiler.h"
#include "pipe/p_state.h"
#include "util/u_transfer.h"

#include "svga_screen_cache.h"
#include "svga_screen.h"
#include "svga_cmd.h"
#include "svga_context.h"


/**
 * Maximum number of discontiguous ranges
 */
#define SVGA_BUFFER_MAX_RANGES 32


struct svga_context;
struct svga_winsys_buffer;
struct svga_winsys_surface;

struct svga_buffer_range
{
   unsigned start;
   unsigned end;
};

struct svga_3d_update_gb_image;

/**
 * This structure describes the bind flags and cache key associated
 * with the host surface.
 */
struct svga_buffer_surface
{
   struct list_head list;
   unsigned bind_flags;
   struct svga_host_surface_cache_key key;
   struct svga_winsys_surface *handle;
   enum svga_surface_state surface_state;
};

/**
 * SVGA pipe buffer.
 */
struct svga_buffer
{
   struct pipe_resource b;

   /** This is a superset of b.b.bind */
   unsigned bind_flags;

   /**
    * Regular (non DMA'able) memory.
    *
    * Used for user buffers or for buffers which we know before hand that can
    * never be used by the virtual hardware directly, such as constant buffers.
    */
   void *swbuf;

   /**
    * Whether swbuf was created by the user or not.
    */
   bool user;

   /**
    * Whether swbuf is used for this buffer.
    */
   bool use_swbuf;

   /**
    * Creation key for the host surface handle.
    *
    * This structure describes all the host surface characteristics so that it
    * can be looked up in cache, since creating a host surface is often a slow
    * operation.
    */
   struct svga_host_surface_cache_key key;

   /**
    * Host surface handle.
    *
    * This is a platform independent abstraction for host SID. We create when
    * trying to bind.
    *
    * Only set for non-user buffers.
    */
   struct svga_winsys_surface *handle;

   /**
    * List of surfaces created for this buffer resource to support
    * incompatible bind flags.
    */
   struct list_head surfaces;

   /* Current surface structure */
   struct svga_buffer_surface *bufsurf;

   /**
    * Information about ongoing and past map operations.
    */
   struct {
      /**
       * Number of concurrent mappings.
       */
      unsigned count;

      /**
       * Dirty ranges.
       *
       * Ranges that were touched by the application and need to be uploaded to
       * the host.
       *
       * This information will be copied into dma.boxes, when emiting the
       * SVGA3dCmdSurfaceDMA command.
       */
      struct svga_buffer_range ranges[SVGA_BUFFER_MAX_RANGES];
      unsigned num_ranges;
   } map;

   /**
    * Information about uploaded version of user buffers.
    */
   struct {
      struct pipe_resource *buffer;

      /**
       * We combine multiple user buffers into the same hardware buffer. This
       * is the relative offset within that buffer.
       */
      unsigned offset;

      /**
       * Range of user buffer that is uploaded in @buffer at @offset.
       */
      unsigned start;
      unsigned end;
   } uploaded;

   /**
    * DMA'ble memory.
    *
    * A piece of GMR memory, with the same size of the buffer. It is created
    * when mapping the buffer, and will be used to upload vertex data to the
    * host.
    *
    * Only set for non-user buffers.
    */
   struct svga_winsys_buffer *hwbuf;

   /**
    * Information about pending DMA uploads.
    *
    */
   struct {
      /**
       * Whether this buffer has an unfinished DMA upload command.
       *
       * If not set then the rest of the information is null.
       */
      bool pending;

      SVGA3dSurfaceDMAFlags flags;

      /**
       * Pointer to the DMA copy box *inside* the command buffer.
       */
      SVGA3dCopyBox *boxes;

      /**
       * Pointer to the sequence of update commands
       * *inside* the command buffer.
       */
      struct svga_3d_update_gb_image *updates;

      /**
       * Context that has the pending DMA to this buffer.
       */
      struct svga_context *svga;
   } dma;

   /**
    * Linked list head, used to gather all buffers with pending dma uploads on
    * a context. It is only valid if the dma.pending is set above.
    */
   struct list_head head;

   unsigned size;  /**< Approximate size in bytes */

   bool dirty;  /**< Need to do a readback before mapping? */
   bool uav;    /* Set if the buffer is bound to a uav */

   /** In some cases we try to keep the results of the translate_indices()
    * function from svga_draw_elements.c
    */
   struct {
      enum mesa_prim orig_prim, new_prim;
      struct pipe_resource *buffer;
      unsigned index_size;
      unsigned offset;  /**< first index */
      unsigned count;   /**< num indices */
   } translated_indices;
};


static inline struct svga_buffer *
svga_buffer(struct pipe_resource *resource)
{
   struct svga_buffer *buf = (struct svga_buffer *) resource;
   assert(buf == NULL || buf->b.target == PIPE_BUFFER);
   return buf;
}


/**
 * Returns TRUE for user buffers.  We may
 * decide to use an alternate upload path for these buffers.
 */
static inline bool
svga_buffer_is_user_buffer(struct pipe_resource *buffer)
{
   if (buffer) {
      return svga_buffer(buffer)->user;
   } else {
      return false;
   }
}

/**
 * Returns a pointer to a struct svga_winsys_screen given a
 * struct svga_buffer.
 */
static inline struct svga_winsys_screen *
svga_buffer_winsys_screen(struct svga_buffer *sbuf)
{
   return svga_screen(sbuf->b.screen)->sws;
}


/**
 * Returns whether a buffer has hardware storage that is
 * visible to the GPU.
 */
static inline bool
svga_buffer_has_hw_storage(struct svga_buffer *sbuf)
{
   if (svga_buffer_winsys_screen(sbuf)->have_gb_objects)
      return (sbuf->handle ? true : false);
   else
      return (sbuf->hwbuf ? true : false);
}

/**
 * Map the hardware storage of a buffer.
 * \param flags  bitmask of PIPE_MAP_* flags
 */
static inline void *
svga_buffer_hw_storage_map(struct svga_context *svga,
                           struct svga_buffer *sbuf,
                           unsigned flags, bool *retry)
{
   struct svga_winsys_screen *sws = svga_buffer_winsys_screen(sbuf);

   svga->hud.num_buffers_mapped++;

   if (sws->have_gb_objects) {
      struct svga_winsys_context *swc = svga->swc;
      bool rebind;
      void *map;

      if (swc->force_coherent) {
         flags |= PIPE_MAP_PERSISTENT | PIPE_MAP_COHERENT;
      }
      map = swc->surface_map(swc, sbuf->handle, flags, retry, &rebind);
      if (map && rebind) {
         enum pipe_error ret;

         ret = SVGA3D_BindGBSurface(swc, sbuf->handle);
         if (ret != PIPE_OK) {
            svga_context_flush(svga, NULL);
            ret = SVGA3D_BindGBSurface(swc, sbuf->handle);
            assert(ret == PIPE_OK);
         }
         svga_context_flush(svga, NULL);
      }
      return map;
   } else {
      *retry = false;
      return sws->buffer_map(sws, sbuf->hwbuf, flags);
   }
}

/**
 * Unmap the hardware storage of a buffer.
 */
static inline void
svga_buffer_hw_storage_unmap(struct svga_context *svga,
                             struct svga_buffer *sbuf)
{
   struct svga_winsys_screen *sws = svga_buffer_winsys_screen(sbuf);

   if (sws->have_gb_objects) {
      struct svga_winsys_context *swc = svga->swc;
      bool rebind;

      swc->surface_unmap(swc, sbuf->handle, &rebind);
      if (rebind) {
         SVGA_RETRY(svga, SVGA3D_BindGBSurface(swc, sbuf->handle));
      }
   } else
      sws->buffer_unmap(sws, sbuf->hwbuf);

   /* Mark the buffer surface as UPDATED */
   assert(sbuf->bufsurf);
   sbuf->bufsurf->surface_state = SVGA_SURFACE_STATE_UPDATED;
}


static inline void
svga_set_buffer_rendered_to(struct svga_buffer_surface *bufsurf)
{
   bufsurf->surface_state = SVGA_SURFACE_STATE_RENDERED;
}


static inline bool
svga_was_buffer_rendered_to(const struct svga_buffer_surface *bufsurf)
{
   return (bufsurf->surface_state == SVGA_SURFACE_STATE_RENDERED);
}


static inline bool
svga_has_raw_buffer_view(struct svga_buffer *sbuf)
{
   return (sbuf->uav ||
           (sbuf->key.persistent &&
            (sbuf->key.flags & SVGA3D_SURFACE_BIND_RAW_VIEWS) != 0));
}


struct pipe_resource *
svga_user_buffer_create(struct pipe_screen *screen,
                        void *ptr,
                        unsigned bytes,
                        unsigned usage);

struct pipe_resource *
svga_buffer_create(struct pipe_screen *screen,
                   const struct pipe_resource *template);



/**
 * Get the host surface handle for this buffer.
 *
 * This will ensure the host surface is updated, issuing DMAs as needed.
 *
 * NOTE: This may insert new commands in the context, so it *must* be called
 * before reserving command buffer space. And, in order to insert commands
 * it may need to call svga_context_flush().
 */
struct svga_winsys_surface *
svga_buffer_handle(struct svga_context *svga,
                   struct pipe_resource *buf,
                   unsigned tobind_flags);

void
svga_context_flush_buffers(struct svga_context *svga);

struct svga_winsys_buffer *
svga_winsys_buffer_create(struct svga_context *svga,
                          unsigned alignment,
                          unsigned usage,
                          unsigned size);

void
svga_buffer_transfer_flush_region(struct pipe_context *pipe,
                                  struct pipe_transfer *transfer,
                                  const struct pipe_box *box);

void
svga_resource_destroy(struct pipe_screen *screen,
                      struct pipe_resource *buf);

void *
svga_buffer_transfer_map(struct pipe_context *pipe,
                         struct pipe_resource *resource,
                         unsigned level,
                         unsigned usage,
                         const struct pipe_box *box,
                         struct pipe_transfer **ptransfer);

void
svga_buffer_transfer_unmap(struct pipe_context *pipe,
                           struct pipe_transfer *transfer);

#endif /* SVGA_BUFFER_H */
