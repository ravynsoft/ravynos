/*
 * Copyright 2014, 2015 Red Hat.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef VIRGL_RESOURCE_H
#define VIRGL_RESOURCE_H

#include "util/u_resource.h"
#include "util/u_range.h"
#include "util/list.h"
#include "util/u_transfer.h"

#include "virtio-gpu/virgl_hw.h"
#include "virgl_screen.h"
#define VR_MAX_TEXTURE_2D_LEVELS 15

#define VIRGL_BLOB_MEM_GUEST 1
#define VIRGL_BLOB_MEM_HOST3D 2
#define VIRGL_BLOB_MEM_HOST3D_GUEST 3

struct winsys_handle;
struct virgl_screen;
struct virgl_context;

struct virgl_resource_metadata
{
   unsigned long level_offset[VR_MAX_TEXTURE_2D_LEVELS];
   unsigned stride[VR_MAX_TEXTURE_2D_LEVELS];
   unsigned layer_stride[VR_MAX_TEXTURE_2D_LEVELS];
   uint32_t plane, plane_offset, total_size;
   uint64_t modifier;
};

struct virgl_resource {
   struct pipe_resource b;
   struct virgl_hw_res *hw_res;
   struct virgl_resource_metadata metadata;

   /* For PIPE_BUFFER only.  Data outside of this range are uninitialized. */
   struct util_range valid_buffer_range;

   /* This mask indicates where the resource has been bound to, excluding
    * pipe_surface binds.
    *
    * This is more accurate than pipe_resource::bind.  Besides,
    * pipe_resource::bind can be 0 with direct state access, and is not
    * usable.
    */
   unsigned bind_history;
   uint32_t blob_mem;

   uint16_t clean_mask;
   uint16_t use_staging : 1;
   uint16_t reserved : 15;
};

struct virgl_transfer {
   struct pipe_transfer base;
   uint32_t offset, l_stride;
   struct util_range range;
   struct list_head queue_link;
   struct pipe_transfer *resolve_transfer;

   struct virgl_hw_res *hw_res;
   void *hw_res_map;
   /* If not NULL, denotes that this is a copy transfer, i.e.,
    * that the transfer source data should be taken from this
    * resource instead of the original transfer resource.
    */
   struct virgl_hw_res *copy_src_hw_res;
   /* The offset in the copy source resource to copy data from. */
   uint32_t copy_src_offset;
   /* copy transfers can be performed to and from host */
   uint32_t direction;
};

void virgl_resource_destroy(struct pipe_screen *screen,
                            struct pipe_resource *resource);

void virgl_init_screen_resource_functions(struct pipe_screen *screen);

void virgl_init_context_resource_functions(struct pipe_context *ctx);

void virgl_texture_init(struct virgl_resource *res);

static inline struct virgl_resource *virgl_resource(struct pipe_resource *r)
{
   return (struct virgl_resource *)r;
}

static inline struct virgl_transfer *virgl_transfer(struct pipe_transfer *trans)
{
   return (struct virgl_transfer *)trans;
}

void virgl_buffer_transfer_flush_region(struct pipe_context *ctx,
                                        struct pipe_transfer *transfer,
                                        const struct pipe_box *box);

void virgl_buffer_transfer_unmap(struct pipe_context *ctx,
                                 struct pipe_transfer *transfer);

void virgl_buffer_init(struct virgl_resource *res);

static inline unsigned pipe_to_virgl_bind(const struct virgl_screen *vs,
                                          unsigned pbind)
{
   unsigned outbind = 0;
   if (pbind & PIPE_BIND_DEPTH_STENCIL)
      outbind |= VIRGL_BIND_DEPTH_STENCIL;
   if (pbind & PIPE_BIND_RENDER_TARGET)
      outbind |= VIRGL_BIND_RENDER_TARGET;
   if (pbind & PIPE_BIND_SAMPLER_VIEW)
      outbind |= VIRGL_BIND_SAMPLER_VIEW;
   if (pbind & PIPE_BIND_VERTEX_BUFFER)
      outbind |= VIRGL_BIND_VERTEX_BUFFER;
   if (pbind & PIPE_BIND_INDEX_BUFFER)
      outbind |= VIRGL_BIND_INDEX_BUFFER;
   if (pbind & PIPE_BIND_CONSTANT_BUFFER)
      outbind |= VIRGL_BIND_CONSTANT_BUFFER;
   if (pbind & PIPE_BIND_DISPLAY_TARGET)
      outbind |= VIRGL_BIND_DISPLAY_TARGET;
   if (pbind & PIPE_BIND_STREAM_OUTPUT)
      outbind |= VIRGL_BIND_STREAM_OUTPUT;
   if (pbind & PIPE_BIND_CURSOR)
      outbind |= VIRGL_BIND_CURSOR;
   if (pbind & PIPE_BIND_CUSTOM)
      outbind |= VIRGL_BIND_CUSTOM;
   if (pbind & PIPE_BIND_SCANOUT)
      outbind |= VIRGL_BIND_SCANOUT;
   if (pbind & PIPE_BIND_SHARED)
      outbind |= VIRGL_BIND_SHARED;
   if (pbind & PIPE_BIND_SHADER_BUFFER)
      outbind |= VIRGL_BIND_SHADER_BUFFER;
   if (pbind & PIPE_BIND_QUERY_BUFFER)
      outbind |= VIRGL_BIND_QUERY_BUFFER;
   if (pbind & PIPE_BIND_COMMAND_ARGS_BUFFER)
      if (vs->caps.caps.v2.capability_bits & VIRGL_CAP_BIND_COMMAND_ARGS)
         outbind |= VIRGL_BIND_COMMAND_ARGS;

   /* Staging resources should only be created directly through the winsys,
    * not using pipe_resources.
    */
   assert(!(outbind & VIRGL_BIND_STAGING));

   return outbind;
}

static inline unsigned pipe_to_virgl_flags(const struct virgl_screen *vs,
                                           unsigned pflags)
{
   unsigned out_flags = 0;
   if (pflags & PIPE_RESOURCE_FLAG_MAP_PERSISTENT)
      out_flags |= VIRGL_RESOURCE_FLAG_MAP_PERSISTENT;

   if (pflags & PIPE_RESOURCE_FLAG_MAP_COHERENT)
      out_flags |= VIRGL_RESOURCE_FLAG_MAP_COHERENT;

   return out_flags;
}

void *
virgl_resource_transfer_map(struct pipe_context *ctx,
                            struct pipe_resource *resource,
                            unsigned level,
                            unsigned usage,
                            const struct pipe_box *box,
                            struct pipe_transfer **transfer);

struct virgl_transfer *
virgl_resource_create_transfer(struct virgl_context *vctx,
                               struct pipe_resource *pres,
                               const struct virgl_resource_metadata *metadata,
                               unsigned level, unsigned usage,
                               const struct pipe_box *box);

void virgl_resource_destroy_transfer(struct virgl_context *vctx,
                                     struct virgl_transfer *trans);

void virgl_resource_destroy(struct pipe_screen *screen,
                            struct pipe_resource *resource);

bool virgl_resource_get_handle(struct pipe_screen *screen,
                               struct pipe_context *context,
                               struct pipe_resource *resource,
                               struct winsys_handle *whandle,
                               unsigned usage);

void virgl_resource_dirty(struct virgl_resource *res, uint32_t level);

void *virgl_texture_transfer_map(struct pipe_context *ctx,
                                 struct pipe_resource *resource,
                                 unsigned level,
                                 unsigned usage,
                                 const struct pipe_box *box,
                                 struct pipe_transfer **transfer);

void virgl_texture_transfer_unmap(struct pipe_context *ctx,
                                  struct pipe_transfer *transfer);

#endif
