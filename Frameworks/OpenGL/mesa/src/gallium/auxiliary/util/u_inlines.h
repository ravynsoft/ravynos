/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL VMWARE AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#ifndef U_INLINES_H
#define U_INLINES_H

#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_state.h"
#include "pipe/p_screen.h"
#include "util/compiler.h"
#include "util/format/u_format.h"
#include "util/u_debug.h"
#include "util/u_debug_describe.h"
#include "util/u_debug_refcnt.h"
#include "util/u_atomic.h"
#include "util/u_box.h"
#include "util/u_math.h"


#ifdef __cplusplus
extern "C" {
#endif


/*
 * Reference counting helper functions.
 */


static inline void
pipe_reference_init(struct pipe_reference *dst, unsigned count)
{
   dst->count = count;
}

static inline bool
pipe_is_referenced(struct pipe_reference *src)
{
   return p_atomic_read(&src->count) != 0;
}

/**
 * Update reference counting.
 * The old thing pointed to, if any, will be unreferenced.
 * Both 'dst' and 'src' may be NULL.
 * \return TRUE if the object's refcount hits zero and should be destroyed.
 */
static inline bool
pipe_reference_described(struct pipe_reference *dst,
                         struct pipe_reference *src,
                         debug_reference_descriptor get_desc)
{
   if (dst != src) {
      /* bump the src.count first */
      if (src) {
         ASSERTED int count = p_atomic_inc_return(&src->count);
         assert(count != 1); /* src had to be referenced */
         debug_reference(src, get_desc, 1);
      }

      if (dst) {
         int count = p_atomic_dec_return(&dst->count);
         assert(count != -1); /* dst had to be referenced */
         debug_reference(dst, get_desc, -1);
         if (!count)
            return true;
      }
   }

   return false;
}

static inline bool
pipe_reference(struct pipe_reference *dst, struct pipe_reference *src)
{
   return pipe_reference_described(dst, src,
                                   (debug_reference_descriptor)
                                   debug_describe_reference);
}

static inline void
pipe_surface_reference(struct pipe_surface **dst, struct pipe_surface *src)
{
   struct pipe_surface *old_dst = *dst;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL,
                                src ? &src->reference : NULL,
                                (debug_reference_descriptor)
                                debug_describe_surface))
      old_dst->context->surface_destroy(old_dst->context, old_dst);
   *dst = src;
}

/**
 * Similar to pipe_surface_reference() but always set the pointer to NULL
 * and pass in an explicit context.  The explicit context avoids the problem
 * of using a deleted context's surface_destroy() method when freeing a surface
 * that's shared by multiple contexts.
 */
static inline void
pipe_surface_release(struct pipe_context *pipe, struct pipe_surface **ptr)
{
   struct pipe_surface *old = *ptr;

   if (pipe_reference_described(&old->reference, NULL,
                                (debug_reference_descriptor)
                                debug_describe_surface))
      pipe->surface_destroy(pipe, old);
   *ptr = NULL;
}

static inline void
pipe_resource_destroy(struct pipe_resource *res)
{
   /* Avoid recursion, which would prevent inlining this function */
   do {
      struct pipe_resource *next = res->next;

      res->screen->resource_destroy(res->screen, res);
      res = next;
   } while (pipe_reference_described(res ? &res->reference : NULL,
                                     NULL,
                                     (debug_reference_descriptor)
                                     debug_describe_resource));
}

static inline void
pipe_resource_reference(struct pipe_resource **dst, struct pipe_resource *src)
{
   struct pipe_resource *old_dst = *dst;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL,
                                src ? &src->reference : NULL,
                                (debug_reference_descriptor)
                                debug_describe_resource)) {
      pipe_resource_destroy(old_dst);
   }
   *dst = src;
}

/**
 * Subtract the given number of references.
 */
static inline void
pipe_drop_resource_references(struct pipe_resource *dst, int num_refs)
{
   int count = p_atomic_add_return(&dst->reference.count, -num_refs);

   assert(count >= 0);
   /* Underflows shouldn't happen, but let's be safe. */
   if (count <= 0)
      pipe_resource_destroy(dst);
}

/**
 * Same as pipe_surface_release, but used when pipe_context doesn't exist
 * anymore.
 */
static inline void
pipe_surface_release_no_context(struct pipe_surface **ptr)
{
   struct pipe_surface *surf = *ptr;

   if (pipe_reference_described(&surf->reference, NULL,
                                (debug_reference_descriptor)
                                debug_describe_surface)) {
      /* trivially destroy pipe_surface */
      pipe_resource_reference(&surf->texture, NULL);
      free(surf);
   }
   *ptr = NULL;
}

/**
 * Set *dst to \p src with proper reference counting.
 *
 * The caller must guarantee that \p src and *dst were created in
 * the same context (if they exist), and that this must be the current context.
 */
static inline void
pipe_sampler_view_reference(struct pipe_sampler_view **dst,
                            struct pipe_sampler_view *src)
{
   struct pipe_sampler_view *old_dst = *dst;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL,
                                src ? &src->reference : NULL,
                                (debug_reference_descriptor)
                                debug_describe_sampler_view))
      old_dst->context->sampler_view_destroy(old_dst->context, old_dst);
   *dst = src;
}

static inline void
pipe_so_target_reference(struct pipe_stream_output_target **dst,
                         struct pipe_stream_output_target *src)
{
   struct pipe_stream_output_target *old_dst = *dst;

   if (pipe_reference_described(old_dst ? &old_dst->reference : NULL,
                     src ? &src->reference : NULL,
                     (debug_reference_descriptor)debug_describe_so_target))
      old_dst->context->stream_output_target_destroy(old_dst->context, old_dst);
   *dst = src;
}

static inline void
pipe_vertex_state_reference(struct pipe_vertex_state **dst,
                            struct pipe_vertex_state *src)
{
   struct pipe_vertex_state *old_dst = *dst;

   if (pipe_reference(old_dst ? &old_dst->reference : NULL,
                      src ? &src->reference : NULL))
      old_dst->screen->vertex_state_destroy(old_dst->screen, old_dst);
   *dst = src;
}

static inline void
pipe_vertex_buffer_unreference(struct pipe_vertex_buffer *dst)
{
   if (dst->is_user_buffer)
      dst->buffer.user = NULL;
   else
      pipe_resource_reference(&dst->buffer.resource, NULL);
}

static inline void
pipe_vertex_buffer_reference(struct pipe_vertex_buffer *dst,
                             const struct pipe_vertex_buffer *src)
{
   if (dst->buffer.resource == src->buffer.resource) {
      /* Just copy the fields, don't touch reference counts. */
      dst->is_user_buffer = src->is_user_buffer;
      dst->buffer_offset = src->buffer_offset;
      return;
   }

   pipe_vertex_buffer_unreference(dst);
   /* Don't use memcpy because there is a hole between variables.
    * dst can be used as a hash key.
    */
   dst->is_user_buffer = src->is_user_buffer;
   dst->buffer_offset = src->buffer_offset;

   if (src->is_user_buffer)
      dst->buffer.user = src->buffer.user;
   else
      pipe_resource_reference(&dst->buffer.resource, src->buffer.resource);
}

static inline void
pipe_surface_reset(struct pipe_context *ctx, struct pipe_surface* ps,
                   struct pipe_resource *pt, unsigned level, unsigned layer)
{
   pipe_resource_reference(&ps->texture, pt);
   ps->format = pt->format;
   ps->width = u_minify(pt->width0, level);
   ps->height = u_minify(pt->height0, level);
   ps->u.tex.level = level;
   ps->u.tex.first_layer = ps->u.tex.last_layer = layer;
   ps->context = ctx;
}

static inline void
pipe_surface_init(struct pipe_context *ctx, struct pipe_surface* ps,
                  struct pipe_resource *pt, unsigned level, unsigned layer)
{
   ps->texture = 0;
   pipe_reference_init(&ps->reference, 1);
   pipe_surface_reset(ctx, ps, pt, level, layer);
}

/* Return true if the surfaces are equal. */
static inline bool
pipe_surface_equal(struct pipe_surface *s1, struct pipe_surface *s2)
{
   return s1->texture == s2->texture &&
          s1->format == s2->format &&
          (s1->texture->target != PIPE_BUFFER ||
           (s1->u.buf.first_element == s2->u.buf.first_element &&
            s1->u.buf.last_element == s2->u.buf.last_element)) &&
          (s1->texture->target == PIPE_BUFFER ||
           (s1->u.tex.level == s2->u.tex.level &&
            s1->u.tex.first_layer == s2->u.tex.first_layer &&
            s1->u.tex.last_layer == s2->u.tex.last_layer));
}

/*
 * Convenience wrappers for screen buffer functions.
 */


static inline unsigned
pipe_buffer_size(const struct pipe_resource *buffer)
{
    return buffer->width0;
}


/**
 * Create a new resource.
 * \param bind  bitmask of PIPE_BIND_x flags
 * \param usage  a PIPE_USAGE_x value
 */
static inline struct pipe_resource *
pipe_buffer_create(struct pipe_screen *screen,
                   unsigned bind,
                   enum pipe_resource_usage usage,
                   unsigned size)
{
   struct pipe_resource buffer;
   memset(&buffer, 0, sizeof buffer);
   buffer.target = PIPE_BUFFER;
   buffer.format = PIPE_FORMAT_R8_UNORM; /* want TYPELESS or similar */
   buffer.bind = bind;
   buffer.usage = usage;
   buffer.flags = 0;
   buffer.width0 = size;
   buffer.height0 = 1;
   buffer.depth0 = 1;
   buffer.array_size = 1;
   return screen->resource_create(screen, &buffer);
}


static inline struct pipe_resource *
pipe_buffer_create_const0(struct pipe_screen *screen,
                          unsigned bind,
                          enum pipe_resource_usage usage,
                          unsigned size)
{
   struct pipe_resource buffer;
   memset(&buffer, 0, sizeof buffer);
   buffer.target = PIPE_BUFFER;
   buffer.format = PIPE_FORMAT_R8_UNORM;
   buffer.bind = bind;
   buffer.usage = usage;
   buffer.flags = screen->get_param(screen, PIPE_CAP_CONSTBUF0_FLAGS);
   buffer.width0 = size;
   buffer.height0 = 1;
   buffer.depth0 = 1;
   buffer.array_size = 1;
   return screen->resource_create(screen, &buffer);
}


/**
 * Map a range of a resource.
 * \param offset  start of region, in bytes
 * \param length  size of region, in bytes
 * \param access  bitmask of PIPE_MAP_x flags
 * \param transfer  returns a transfer object
 */
static inline void *
pipe_buffer_map_range(struct pipe_context *pipe,
                      struct pipe_resource *buffer,
                      unsigned offset,
                      unsigned length,
                      unsigned access,
                      struct pipe_transfer **transfer)
{
   struct pipe_box box;
   void *map;

   assert(offset < buffer->width0);
   assert(offset + length <= buffer->width0);
   assert(length);

   u_box_1d(offset, length, &box);

   map = pipe->buffer_map(pipe, buffer, 0, access, &box, transfer);
   if (!map) {
      return NULL;
   }

   return map;
}


/**
 * Map whole resource.
 * \param access  bitmask of PIPE_MAP_x flags
 * \param transfer  returns a transfer object
 */
static inline void *
pipe_buffer_map(struct pipe_context *pipe,
                struct pipe_resource *buffer,
                unsigned access,
                struct pipe_transfer **transfer)
{
   return pipe_buffer_map_range(pipe, buffer, 0, buffer->width0,
                                access, transfer);
}


static inline void
pipe_buffer_unmap(struct pipe_context *pipe,
                  struct pipe_transfer *transfer)
{
   pipe->buffer_unmap(pipe, transfer);
}

static inline void
pipe_buffer_flush_mapped_range(struct pipe_context *pipe,
                               struct pipe_transfer *transfer,
                               unsigned offset,
                               unsigned length)
{
   struct pipe_box box;
   int transfer_offset;

   assert(length);
   assert(transfer->box.x <= (int) offset);
   assert((int) (offset + length) <= transfer->box.x + transfer->box.width);

   /* Match old screen->buffer_flush_mapped_range() behaviour, where
    * offset parameter is relative to the start of the buffer, not the
    * mapped range.
    */
   transfer_offset = offset - transfer->box.x;

   u_box_1d(transfer_offset, length, &box);

   pipe->transfer_flush_region(pipe, transfer, &box);
}

static inline void
pipe_buffer_write(struct pipe_context *pipe,
                  struct pipe_resource *buf,
                  unsigned offset,
                  unsigned size,
                  const void *data)
{
   /* Don't set any other usage bits. Drivers should derive them. */
   pipe->buffer_subdata(pipe, buf, PIPE_MAP_WRITE, offset, size, data);
}

/**
 * Special case for writing non-overlapping ranges.
 *
 * We can avoid GPU/CPU synchronization when writing range that has never
 * been written before.
 */
static inline void
pipe_buffer_write_nooverlap(struct pipe_context *pipe,
                            struct pipe_resource *buf,
                            unsigned offset, unsigned size,
                            const void *data)
{
   pipe->buffer_subdata(pipe, buf,
                        (PIPE_MAP_WRITE |
                         PIPE_MAP_UNSYNCHRONIZED),
                        offset, size, data);
}

/**
 * Utility for simplifying pipe_context::resource_copy_region calls
 */
static inline void
pipe_buffer_copy(struct pipe_context *pipe,
                 struct pipe_resource *dst,
                 struct pipe_resource *src,
                 unsigned dst_offset,
                 unsigned src_offset,
                 unsigned size)
{
   struct pipe_box box;
   /* only these fields are used */
   box.x = (int)src_offset;
   box.width = (int)size;
   pipe->resource_copy_region(pipe, dst, 0, dst_offset, 0, 0, src, 0, &box);
}

/**
 * Create a new resource and immediately put data into it
 * \param bind  bitmask of PIPE_BIND_x flags
 * \param usage  bitmask of PIPE_USAGE_x flags
 */
static inline struct pipe_resource *
pipe_buffer_create_with_data(struct pipe_context *pipe,
                             unsigned bind,
                             enum pipe_resource_usage usage,
                             unsigned size,
                             const void *ptr)
{
   struct pipe_resource *res = pipe_buffer_create(pipe->screen,
                                                  bind, usage, size);
   pipe_buffer_write_nooverlap(pipe, res, 0, size, ptr);
   return res;
}

static inline void
pipe_buffer_read(struct pipe_context *pipe,
                 struct pipe_resource *buf,
                 unsigned offset,
                 unsigned size,
                 void *data)
{
   struct pipe_transfer *src_transfer;
   uint8_t *map;

   map = (uint8_t *) pipe_buffer_map_range(pipe,
                                         buf,
                                         offset, size,
                                         PIPE_MAP_READ,
                                         &src_transfer);
   if (!map)
      return;

   memcpy(data, map, size);
   pipe_buffer_unmap(pipe, src_transfer);
}


/**
 * Map a resource for reading/writing.
 * \param access  bitmask of PIPE_MAP_x flags
 */
static inline void *
pipe_texture_map(struct pipe_context *context,
                 struct pipe_resource *resource,
                 unsigned level, unsigned layer,
                 unsigned access,
                 unsigned x, unsigned y,
                 unsigned w, unsigned h,
                 struct pipe_transfer **transfer)
{
   struct pipe_box box;
   u_box_2d_zslice(x, y, layer, w, h, &box);
   return context->texture_map(context, resource, level, access,
                               &box, transfer);
}


/**
 * Map a 3D (texture) resource for reading/writing.
 * \param access  bitmask of PIPE_MAP_x flags
 */
static inline void *
pipe_texture_map_3d(struct pipe_context *context,
                    struct pipe_resource *resource,
                    unsigned level,
                    unsigned access,
                    unsigned x, unsigned y, unsigned z,
                    unsigned w, unsigned h, unsigned d,
                    struct pipe_transfer **transfer)
{
   struct pipe_box box;
   u_box_3d(x, y, z, w, h, d, &box);
   return context->texture_map(context, resource, level, access,
                               &box, transfer);
}

static inline void
pipe_texture_unmap(struct pipe_context *context,
                   struct pipe_transfer *transfer)
{
   context->texture_unmap(context, transfer);
}

static inline void
pipe_set_constant_buffer(struct pipe_context *pipe,
                         enum pipe_shader_type shader, uint index,
                         struct pipe_resource *buf)
{
   if (buf) {
      struct pipe_constant_buffer cb;
      cb.buffer = buf;
      cb.buffer_offset = 0;
      cb.buffer_size = buf->width0;
      cb.user_buffer = NULL;
      pipe->set_constant_buffer(pipe, shader, index, false, &cb);
   } else {
      pipe->set_constant_buffer(pipe, shader, index, false, NULL);
   }
}


/**
 * Get the polygon offset enable/disable flag for the given polygon fill mode.
 * \param fill_mode  one of PIPE_POLYGON_MODE_POINT/LINE/FILL
 */
static inline bool
util_get_offset(const struct pipe_rasterizer_state *templ,
                unsigned fill_mode)
{
   switch(fill_mode) {
   case PIPE_POLYGON_MODE_POINT:
      return templ->offset_point;
   case PIPE_POLYGON_MODE_LINE:
      return templ->offset_line;
   case PIPE_POLYGON_MODE_FILL:
      return templ->offset_tri;
   default:
      assert(0);
      return false;
   }
}

static inline float
util_get_min_point_size(const struct pipe_rasterizer_state *state)
{
   /* The point size should be clamped to this value at the rasterizer stage.
    */
   return !state->point_quad_rasterization &&
          !state->point_smooth &&
          !state->multisample ? 1.0f : 0.0f;
}

static inline void
util_query_clear_result(union pipe_query_result *result, unsigned type)
{
   switch (type) {
   case PIPE_QUERY_OCCLUSION_PREDICATE:
   case PIPE_QUERY_OCCLUSION_PREDICATE_CONSERVATIVE:
   case PIPE_QUERY_SO_OVERFLOW_PREDICATE:
   case PIPE_QUERY_SO_OVERFLOW_ANY_PREDICATE:
   case PIPE_QUERY_GPU_FINISHED:
      result->b = false;
      break;
   case PIPE_QUERY_OCCLUSION_COUNTER:
   case PIPE_QUERY_TIMESTAMP:
   case PIPE_QUERY_TIME_ELAPSED:
   case PIPE_QUERY_PRIMITIVES_GENERATED:
   case PIPE_QUERY_PRIMITIVES_EMITTED:
      result->u64 = 0;
      break;
   case PIPE_QUERY_SO_STATISTICS:
      memset(&result->so_statistics, 0, sizeof(result->so_statistics));
      break;
   case PIPE_QUERY_TIMESTAMP_DISJOINT:
      memset(&result->timestamp_disjoint, 0, sizeof(result->timestamp_disjoint));
      break;
   case PIPE_QUERY_PIPELINE_STATISTICS:
      memset(&result->pipeline_statistics, 0, sizeof(result->pipeline_statistics));
      break;
   default:
      memset(result, 0, sizeof(*result));
   }
}

/** Convert PIPE_TEXTURE_x to TGSI_TEXTURE_x */
static inline enum tgsi_texture_type
util_pipe_tex_to_tgsi_tex(enum pipe_texture_target pipe_tex_target,
                          unsigned nr_samples)
{
   switch (pipe_tex_target) {
   case PIPE_BUFFER:
      return TGSI_TEXTURE_BUFFER;

   case PIPE_TEXTURE_1D:
      assert(nr_samples <= 1);
      return TGSI_TEXTURE_1D;

   case PIPE_TEXTURE_2D:
      return nr_samples > 1 ? TGSI_TEXTURE_2D_MSAA : TGSI_TEXTURE_2D;

   case PIPE_TEXTURE_RECT:
      assert(nr_samples <= 1);
      return TGSI_TEXTURE_RECT;

   case PIPE_TEXTURE_3D:
      assert(nr_samples <= 1);
      return TGSI_TEXTURE_3D;

   case PIPE_TEXTURE_CUBE:
      assert(nr_samples <= 1);
      return TGSI_TEXTURE_CUBE;

   case PIPE_TEXTURE_1D_ARRAY:
      assert(nr_samples <= 1);
      return TGSI_TEXTURE_1D_ARRAY;

   case PIPE_TEXTURE_2D_ARRAY:
      return nr_samples > 1 ? TGSI_TEXTURE_2D_ARRAY_MSAA :
                              TGSI_TEXTURE_2D_ARRAY;

   case PIPE_TEXTURE_CUBE_ARRAY:
      return TGSI_TEXTURE_CUBE_ARRAY;

   default:
      assert(0 && "unexpected texture target");
      return TGSI_TEXTURE_UNKNOWN;
   }
}


static inline void
util_copy_constant_buffer(struct pipe_constant_buffer *dst,
                          const struct pipe_constant_buffer *src,
                          bool take_ownership)
{
   if (src) {
      if (take_ownership) {
         pipe_resource_reference(&dst->buffer, NULL);
         dst->buffer = src->buffer;
      } else {
         pipe_resource_reference(&dst->buffer, src->buffer);
      }
      dst->buffer_offset = src->buffer_offset;
      dst->buffer_size = src->buffer_size;
      dst->user_buffer = src->user_buffer;
   }
   else {
      pipe_resource_reference(&dst->buffer, NULL);
      dst->buffer_offset = 0;
      dst->buffer_size = 0;
      dst->user_buffer = NULL;
   }
}

static inline void
util_copy_shader_buffer(struct pipe_shader_buffer *dst,
                        const struct pipe_shader_buffer *src)
{
   if (src) {
      pipe_resource_reference(&dst->buffer, src->buffer);
      dst->buffer_offset = src->buffer_offset;
      dst->buffer_size = src->buffer_size;
   }
   else {
      pipe_resource_reference(&dst->buffer, NULL);
      dst->buffer_offset = 0;
      dst->buffer_size = 0;
   }
}

static inline void
util_copy_image_view(struct pipe_image_view *dst,
                     const struct pipe_image_view *src)
{
   if (src) {
      pipe_resource_reference(&dst->resource, src->resource);
      dst->format = src->format;
      dst->access = src->access;
      dst->shader_access = src->shader_access;
      dst->u = src->u;
   } else {
      pipe_resource_reference(&dst->resource, NULL);
      dst->format = PIPE_FORMAT_NONE;
      dst->access = 0;
      dst->shader_access = 0;
      memset(&dst->u, 0, sizeof(dst->u));
   }
}

static inline unsigned
util_max_layer(const struct pipe_resource *r, unsigned level)
{
   switch (r->target) {
   case PIPE_TEXTURE_3D:
      return u_minify(r->depth0, level) - 1;
   case PIPE_TEXTURE_CUBE:
      assert(r->array_size == 6);
      FALLTHROUGH;
   case PIPE_TEXTURE_1D_ARRAY:
   case PIPE_TEXTURE_2D_ARRAY:
   case PIPE_TEXTURE_CUBE_ARRAY:
      return r->array_size - 1;
   default:
      return 0;
   }
}

static inline unsigned
util_num_layers(const struct pipe_resource *r, unsigned level)
{
   return util_max_layer(r, level) + 1;
}

static inline bool
util_texrange_covers_whole_level(const struct pipe_resource *tex,
                                 unsigned level, unsigned x, unsigned y,
                                 unsigned z, unsigned width,
                                 unsigned height, unsigned depth)
{
   return x == 0 && y == 0 && z == 0 &&
          width == u_minify(tex->width0, level) &&
          height == u_minify(tex->height0, level) &&
          depth == util_num_layers(tex, level);
}

/**
 * Returns true if the blit will fully initialize all pixels in the resource.
 */
static inline bool
util_blit_covers_whole_resource(const struct pipe_blit_info *info)
{
   /* No conditional rendering or scissoring.  (We assume that the caller would
    * have dropped any redundant scissoring)
    */
   if (info->scissor_enable || info->window_rectangle_include || info->render_condition_enable || info->alpha_blend)
      return false;

   const struct pipe_resource *dst = info->dst.resource;
   /* A single blit can't initialize a miptree. */
   if (dst->last_level != 0)
      return false;

   assert(info->dst.level == 0);

   /* Make sure the dst box covers the whole resource. */
   if (!(util_texrange_covers_whole_level(dst, 0,
         0, 0, 0,
         info->dst.box.width, info->dst.box.height, info->dst.box.depth))) {
      return false;
   }

   /* Make sure the mask actually updates all the channels present in the dst format. */
   if (info->mask & PIPE_MASK_RGBA) {
      if ((info->mask & PIPE_MASK_RGBA) != PIPE_MASK_RGBA)
         return false;
   }

   if (info->mask & PIPE_MASK_ZS) {
      const struct util_format_description *format_desc = util_format_description(info->dst.format);
      uint32_t dst_has = 0;
      if (util_format_has_depth(format_desc))
         dst_has |= PIPE_MASK_Z;
      if (util_format_has_stencil(format_desc))
         dst_has |= PIPE_MASK_S;
      if (dst_has & ~(info->mask & PIPE_MASK_ZS))
         return false;
   }

   return true;
}

static inline bool
util_logicop_reads_dest(enum pipe_logicop op)
{
   switch (op) {
   case PIPE_LOGICOP_NOR:
   case PIPE_LOGICOP_AND_INVERTED:
   case PIPE_LOGICOP_AND_REVERSE:
   case PIPE_LOGICOP_INVERT:
   case PIPE_LOGICOP_XOR:
   case PIPE_LOGICOP_NAND:
   case PIPE_LOGICOP_AND:
   case PIPE_LOGICOP_EQUIV:
   case PIPE_LOGICOP_NOOP:
   case PIPE_LOGICOP_OR_INVERTED:
   case PIPE_LOGICOP_OR_REVERSE:
   case PIPE_LOGICOP_OR:
      return true;
   case PIPE_LOGICOP_CLEAR:
   case PIPE_LOGICOP_COPY_INVERTED:
   case PIPE_LOGICOP_COPY:
   case PIPE_LOGICOP_SET:
      return false;
   }
   unreachable("bad logicop");
}

static inline bool
util_writes_stencil(const struct pipe_stencil_state *s)
{
   return s->enabled && s->writemask &&
        ((s->fail_op != PIPE_STENCIL_OP_KEEP) ||
         (s->zpass_op != PIPE_STENCIL_OP_KEEP) ||
         (s->zfail_op != PIPE_STENCIL_OP_KEEP));
}

static inline bool
util_writes_depth(const struct pipe_depth_stencil_alpha_state *zsa)
{
   return zsa->depth_enabled && zsa->depth_writemask &&
         (zsa->depth_func != PIPE_FUNC_NEVER);
}

static inline bool
util_writes_depth_stencil(const struct pipe_depth_stencil_alpha_state *zsa)
{
   return util_writes_depth(zsa) ||
          util_writes_stencil(&zsa->stencil[0]) ||
          util_writes_stencil(&zsa->stencil[1]);
}

static inline struct pipe_context *
pipe_create_multimedia_context(struct pipe_screen *screen)
{
   unsigned flags = 0;

   if (!screen->get_param(screen, PIPE_CAP_GRAPHICS))
      flags |= PIPE_CONTEXT_COMPUTE_ONLY;

   return screen->context_create(screen, NULL, flags);
}

static inline unsigned util_res_sample_count(struct pipe_resource *res)
{
   return res->nr_samples > 0 ? res->nr_samples : 1;
}

#ifdef __cplusplus
}
#endif

#endif /* U_INLINES_H */
