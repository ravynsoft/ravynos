/**********************************************************
 * Copyright 2008-2023 VMware, Inc.  All rights reserved.
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

#ifndef SVGA_TEXTURE_H
#define SVGA_TEXTURE_H


#include "util/compiler.h"
#include "pipe/p_state.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_transfer.h"
#include "svga_screen_cache.h"
#include "svga_context.h"

struct pipe_context;
struct pipe_screen;
struct svga_context;
struct svga_winsys_surface;
enum SVGA3dSurfaceFormat;


#define SVGA_MAX_TEXTURE_LEVELS 16

struct svga_texture
{
   struct pipe_resource b;

   uint16_t *defined;

   struct svga_sampler_view *cached_view;

   unsigned view_age[SVGA_MAX_TEXTURE_LEVELS];
   unsigned age;

   bool views_modified;

   /**
    * Creation key for the host surface handle.
    *
    * This structure describes all the host surface characteristics so that it
    * can be looked up in cache, since creating a host surface is often a slow
    * operation.
    */
   struct svga_host_surface_cache_key key;

   /**
    * Handle for the host side surface.
    *
    * This handle is owned by this texture. Views should hold on to a reference
    * to this texture and never destroy this handle directly.
    */
   struct svga_winsys_surface *handle;

   /**
    * Whether the host side surface is imported and not created by this
    * driver.
    */
   bool imported;

   /**
    * Whether texture upload buffer can be used on this texture
    */
   bool can_use_upload;

   /**
    * Whether texture is modified.  Set if any of the dirty bits is set.
    */
   bool modified;

   unsigned size;  /**< Approximate size in bytes */

   /** array indexed by cube face or 3D/array slice, one bit per mipmap level */
   uint16_t *rendered_to;

   /** array indexed by cube face or 3D/array slice, one bit per mipmap level.
    *  Set if the level is marked as dirty.
    */
   uint16_t *dirty;

   enum svga_surface_state surface_state;

   /**
    * A cached backing host side surface to be used if this texture is being
    * used for rendering and sampling at the same time.
    * Currently we only cache one handle. If needed, we can extend this to
    * support multiple handles.
    */
   struct svga_host_surface_cache_key backed_key;
   struct svga_winsys_surface *backed_handle;
   unsigned backed_age;
};



/* Note this is only used for texture (not buffer) transfers:
 */
struct svga_transfer
{
   struct pipe_transfer base;

   unsigned slice;  /**< array slice or cube face */
   SVGA3dBox box;   /* The adjusted box with slice index removed from z */

   struct svga_winsys_buffer *hwbuf;

   /* Height of the hardware buffer in pixel blocks */
   unsigned hw_nblocksy;

   /* Temporary malloc buffer when we can't allocate a hardware buffer
    * big enough */
   void *swbuf;

   /* True if guest backed surface is supported and we can directly map
    * to the surface for this transfer.
    */
   bool use_direct_map;

   struct {
      struct pipe_resource *buf;  /* points to the upload buffer if this
                                   * transfer is done via the upload buffer
                                   * instead of directly mapping to the
                                   * resource's surface.
                                   */
      void *map;
      unsigned offset;
      SVGA3dBox box;
      unsigned nlayers;
   } upload;
};


static inline struct svga_texture *
svga_texture(struct pipe_resource *resource)
{
   struct svga_texture *tex = (struct svga_texture *)resource;
   assert(tex == NULL || tex->b.target != PIPE_BUFFER);
   return tex;
}


static inline struct svga_transfer *
svga_transfer(struct pipe_transfer *transfer)
{
   assert(transfer);
   return (struct svga_transfer *)transfer;
}


/**
 * Increment the age of a view into a texture
 * This is used to track updates to textures when we draw into
 * them via a surface.
 */
static inline void
svga_age_texture_view(struct svga_texture *tex, unsigned level)
{
   assert(level < ARRAY_SIZE(tex->view_age));
   tex->view_age[level] = ++(tex->age);
}


/** For debugging, check that face and level are legal */
static inline void
check_face_level(const struct svga_texture *tex,
                 unsigned face, unsigned level)
{
   if (tex->b.target == PIPE_TEXTURE_CUBE) {
      assert(face < 6);
   }
   else if (tex->b.target == PIPE_TEXTURE_3D) {
      assert(face < tex->b.depth0);
   }
   else {
      assert(face < tex->b.array_size);
   }

   assert(level < 8 * sizeof(tex->rendered_to[0]));
}


/**
 * Mark the given texture face/level as being defined.
 */
static inline void
svga_define_texture_level(struct svga_texture *tex,
                          unsigned face,unsigned level)
{
   check_face_level(tex, face, level);
   tex->defined[face] |= 1 << level;
}


static inline bool
svga_is_texture_level_defined(const struct svga_texture *tex,
                              unsigned face, unsigned level)
{
   check_face_level(tex, face, level);
   return (tex->defined[face] & (1 << level)) != 0;
}


static inline void
svga_set_texture_rendered_to(struct svga_texture *tex)
{
   tex->surface_state = SVGA_SURFACE_STATE_RENDERED;
}


static inline void
svga_clear_texture_rendered_to(struct svga_texture *tex)
{
   tex->surface_state = SVGA_SURFACE_STATE_UPDATED;
}

static inline bool
svga_was_texture_rendered_to(const struct svga_texture *tex)
{
   return (tex->surface_state == SVGA_SURFACE_STATE_RENDERED);
}

static inline void
svga_set_texture_dirty(struct svga_texture *tex,
                       unsigned face, unsigned level)
{
   check_face_level(tex, face, level);
   tex->dirty[face] |= 1 << level;
   tex->modified = true;
}

static inline void
svga_clear_texture_dirty(struct svga_texture *tex)
{
   unsigned i;
   for (i = 0; i < tex->b.depth0 * tex->b.array_size; i++) {
      tex->dirty[i] = 0;
   }
   tex->modified = false;
}

static inline bool
svga_is_texture_level_dirty(const struct svga_texture *tex,
                            unsigned face, unsigned level)
{
   check_face_level(tex, face, level);
   return !!(tex->dirty[face] & (1 << level));
}

static inline bool
svga_is_texture_dirty(const struct svga_texture *tex)
{
   return tex->modified;
}

struct pipe_resource *
svga_texture_create(struct pipe_screen *screen,
                    const struct pipe_resource *template);

bool
svga_resource_get_handle(struct pipe_screen *screen,
                         struct pipe_context *context,
                         struct pipe_resource *texture,
                         struct winsys_handle *whandle,
                         unsigned usage);

struct pipe_resource *
svga_texture_from_handle(struct pipe_screen * screen,
                         const struct pipe_resource *template,
                         struct winsys_handle *whandle);

bool
svga_texture_generate_mipmap(struct pipe_context *pipe,
                             struct pipe_resource *pt,
                             enum pipe_format format,
                             unsigned base_level,
                             unsigned last_level,
                             unsigned first_layer,
                             unsigned last_layer);

bool
svga_texture_transfer_map_upload_create(struct svga_context *svga);

void
svga_texture_transfer_map_upload_destroy(struct svga_context *svga);

bool
svga_texture_transfer_map_can_upload(const struct svga_screen *svgascreen,
                                     const struct pipe_resource *pt);

void *
svga_texture_transfer_map_upload(struct svga_context *svga,
                                 struct svga_transfer *st);

void
svga_texture_transfer_unmap_upload(struct svga_context *svga,
                                   struct svga_transfer *st);

bool
svga_texture_device_format_has_alpha(struct pipe_resource *texture);

void *
svga_texture_transfer_map(struct pipe_context *pipe,
                          struct pipe_resource *texture,
                          unsigned level,
                          unsigned usage,
                          const struct pipe_box *box,
                          struct pipe_transfer **ptransfer);

void
svga_texture_transfer_unmap(struct pipe_context *pipe,
                            struct pipe_transfer *transfer);

#endif /* SVGA_TEXTURE_H */
