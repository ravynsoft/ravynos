/**************************************************************************
 *
 * Copyright 2012 Marek Olšák <maraeo@gmail.com>
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
 * IN NO EVENT SHALL THE AUTHORS AND/OR THEIR SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 **************************************************************************/

#include "util/format/format_utils.h"
#include "util/u_cpu_detect.h"
#include "util/u_helpers.h"
#include "util/u_inlines.h"
#include "util/u_upload_mgr.h"
#include "util/u_thread.h"
#include "util/os_time.h"
#include "util/perf/cpu_trace.h"
#include <inttypes.h>

/**
 * This function is used to copy an array of pipe_vertex_buffer structures,
 * while properly referencing the pipe_vertex_buffer::buffer member.
 *
 * enabled_buffers is updated such that the bits corresponding to the indices
 * of disabled buffers are set to 0 and the enabled ones are set to 1.
 *
 * \sa util_copy_framebuffer_state
 */
void util_set_vertex_buffers_mask(struct pipe_vertex_buffer *dst,
                                  uint32_t *enabled_buffers,
                                  const struct pipe_vertex_buffer *src,
                                  unsigned count,
                                  unsigned unbind_num_trailing_slots,
                                  bool take_ownership)
{
   unsigned i;
   uint32_t bitmask = 0;

   *enabled_buffers &= ~BITFIELD_MASK(count);

   if (src) {
      for (i = 0; i < count; i++) {
         if (src[i].buffer.resource)
            bitmask |= 1 << i;

         pipe_vertex_buffer_unreference(&dst[i]);

         if (!take_ownership && !src[i].is_user_buffer)
            pipe_resource_reference(&dst[i].buffer.resource, src[i].buffer.resource);
      }

      /* Copy over the other members of pipe_vertex_buffer. */
      memcpy(dst, src, count * sizeof(struct pipe_vertex_buffer));

      *enabled_buffers |= bitmask;
   }
   else {
      /* Unreference the buffers. */
      for (i = 0; i < count; i++)
         pipe_vertex_buffer_unreference(&dst[i]);
   }

   for (i = 0; i < unbind_num_trailing_slots; i++)
      pipe_vertex_buffer_unreference(&dst[count + i]);
}

/**
 * Same as util_set_vertex_buffers_mask, but it only returns the number
 * of bound buffers.
 */
void util_set_vertex_buffers_count(struct pipe_vertex_buffer *dst,
                                   unsigned *dst_count,
                                   const struct pipe_vertex_buffer *src,
                                   unsigned count,
                                   unsigned unbind_num_trailing_slots,
                                   bool take_ownership)
{
   unsigned i;
   uint32_t enabled_buffers = 0;

   for (i = 0; i < *dst_count; i++) {
      if (dst[i].buffer.resource)
         enabled_buffers |= (1ull << i);
   }

   util_set_vertex_buffers_mask(dst, &enabled_buffers, src,
                                count, unbind_num_trailing_slots,
                                take_ownership);

   *dst_count = util_last_bit(enabled_buffers);
}

/**
 * This function is used to copy an array of pipe_shader_buffer structures,
 * while properly referencing the pipe_shader_buffer::buffer member.
 *
 * \sa util_set_vertex_buffer_mask
 */
void util_set_shader_buffers_mask(struct pipe_shader_buffer *dst,
                                  uint32_t *enabled_buffers,
                                  const struct pipe_shader_buffer *src,
                                  unsigned start_slot, unsigned count)
{
   unsigned i;

   dst += start_slot;

   if (src) {
      for (i = 0; i < count; i++) {
         pipe_resource_reference(&dst[i].buffer, src[i].buffer);

         if (src[i].buffer)
            *enabled_buffers |= (1ull << (start_slot + i));
         else
            *enabled_buffers &= ~(1ull << (start_slot + i));
      }

      /* Copy over the other members of pipe_shader_buffer. */
      memcpy(dst, src, count * sizeof(struct pipe_shader_buffer));
   }
   else {
      /* Unreference the buffers. */
      for (i = 0; i < count; i++)
         pipe_resource_reference(&dst[i].buffer, NULL);

      *enabled_buffers &= ~(((1ull << count) - 1) << start_slot);
   }
}

/**
 * Given a user index buffer, save the structure to "saved", and upload it.
 */
bool
util_upload_index_buffer(struct pipe_context *pipe,
                         const struct pipe_draw_info *info,
                         const struct pipe_draw_start_count_bias *draw,
                         struct pipe_resource **out_buffer,
                         unsigned *out_offset, unsigned alignment)
{
   unsigned start_offset = draw->start * info->index_size;

   u_upload_data(pipe->stream_uploader, start_offset,
                 draw->count * info->index_size, alignment,
                 (char*)info->index.user + start_offset,
                 out_offset, out_buffer);
   u_upload_unmap(pipe->stream_uploader);
   *out_offset -= start_offset;
   return *out_buffer != NULL;
}

/**
 * Lower each UINT64 vertex element to 1 or 2 UINT32 vertex elements.
 * 3 and 4 component formats are expanded into 2 slots.
 *
 * @param velems        Original vertex elements, will be updated to contain
 *                      the lowered vertex elements.
 * @param velem_count   Original count, will be updated to contain the count
 *                      after lowering.
 * @param tmp           Temporary array of PIPE_MAX_ATTRIBS vertex elements.
 */
void
util_lower_uint64_vertex_elements(const struct pipe_vertex_element **velems,
                                  unsigned *velem_count,
                                  struct pipe_vertex_element tmp[PIPE_MAX_ATTRIBS])
{
   const struct pipe_vertex_element *input = *velems;
   unsigned count = *velem_count;
   bool has_64bit = false;

   for (unsigned i = 0; i < count; i++) {
      has_64bit |= input[i].src_format >= PIPE_FORMAT_R64_UINT &&
                   input[i].src_format <= PIPE_FORMAT_R64G64B64A64_UINT;
   }

   /* Return the original vertex elements if there is nothing to do. */
   if (!has_64bit)
      return;

   /* Lower 64_UINT to 32_UINT. */
   unsigned new_count = 0;

   for (unsigned i = 0; i < count; i++) {
      enum pipe_format format = input[i].src_format;

      /* If the shader input is dvec2 or smaller, reduce the number of
       * components to 2 at most. If the shader input is dvec3 or larger,
       * expand the number of components to 3 at least. If the 3rd component
       * is out of bounds, the hardware shouldn't skip loading the first
       * 2 components.
       */
      if (format >= PIPE_FORMAT_R64_UINT &&
          format <= PIPE_FORMAT_R64G64B64A64_UINT) {
         if (input[i].dual_slot)
            format = MAX2(format, PIPE_FORMAT_R64G64B64_UINT);
         else
            format = MIN2(format, PIPE_FORMAT_R64G64_UINT);
      }

      switch (format) {
      case PIPE_FORMAT_R64_UINT:
         tmp[new_count] = input[i];
         tmp[new_count].src_format = PIPE_FORMAT_R32G32_UINT;
         new_count++;
         break;

      case PIPE_FORMAT_R64G64_UINT:
         tmp[new_count] = input[i];
         tmp[new_count].src_format = PIPE_FORMAT_R32G32B32A32_UINT;
         new_count++;
         break;

      case PIPE_FORMAT_R64G64B64_UINT:
      case PIPE_FORMAT_R64G64B64A64_UINT:
         assert(new_count + 2 <= PIPE_MAX_ATTRIBS);
         tmp[new_count] = tmp[new_count + 1] = input[i];
         tmp[new_count].src_format = PIPE_FORMAT_R32G32B32A32_UINT;
         tmp[new_count + 1].src_format =
            format == PIPE_FORMAT_R64G64B64_UINT ?
                  PIPE_FORMAT_R32G32_UINT :
                  PIPE_FORMAT_R32G32B32A32_UINT;
         tmp[new_count + 1].src_offset += 16;
         new_count += 2;
         break;

      default:
         tmp[new_count++] = input[i];
         break;
      }
   }

   *velem_count = new_count;
   *velems = tmp;
}

/* This is a helper for hardware bring-up. Don't remove. */
struct pipe_query *
util_begin_pipestat_query(struct pipe_context *ctx)
{
   struct pipe_query *q =
      ctx->create_query(ctx, PIPE_QUERY_PIPELINE_STATISTICS, 0);
   if (!q)
      return NULL;

   ctx->begin_query(ctx, q);
   return q;
}

/* This is a helper for hardware bring-up. Don't remove. */
void
util_end_pipestat_query(struct pipe_context *ctx, struct pipe_query *q,
                        FILE *f)
{
   static unsigned counter;
   struct pipe_query_data_pipeline_statistics stats;

   ctx->end_query(ctx, q);
   ctx->get_query_result(ctx, q, true, (void*)&stats);
   ctx->destroy_query(ctx, q);

   fprintf(f,
           "Draw call %u:\n"
           "    ia_vertices    = %"PRIu64"\n"
           "    ia_primitives  = %"PRIu64"\n"
           "    vs_invocations = %"PRIu64"\n"
           "    gs_invocations = %"PRIu64"\n"
           "    gs_primitives  = %"PRIu64"\n"
           "    c_invocations  = %"PRIu64"\n"
           "    c_primitives   = %"PRIu64"\n"
           "    ps_invocations = %"PRIu64"\n"
           "    hs_invocations = %"PRIu64"\n"
           "    ds_invocations = %"PRIu64"\n"
           "    cs_invocations = %"PRIu64"\n",
           (unsigned)p_atomic_inc_return(&counter),
           stats.ia_vertices,
           stats.ia_primitives,
           stats.vs_invocations,
           stats.gs_invocations,
           stats.gs_primitives,
           stats.c_invocations,
           stats.c_primitives,
           stats.ps_invocations,
           stats.hs_invocations,
           stats.ds_invocations,
           stats.cs_invocations);
}

/* This is a helper for profiling. Don't remove. */
struct pipe_query *
util_begin_time_query(struct pipe_context *ctx)
{
   struct pipe_query *q =
      ctx->create_query(ctx, PIPE_QUERY_TIME_ELAPSED, 0);
   if (!q)
      return NULL;

   ctx->begin_query(ctx, q);
   return q;
}

/* This is a helper for profiling. Don't remove. */
void
util_end_time_query(struct pipe_context *ctx, struct pipe_query *q, FILE *f,
                    const char *name)
{
   union pipe_query_result result;

   ctx->end_query(ctx, q);
   ctx->get_query_result(ctx, q, true, &result);
   ctx->destroy_query(ctx, q);

   fprintf(f, "Time elapsed: %s - %"PRIu64".%u us\n", name, result.u64 / 1000, (unsigned)(result.u64 % 1000) / 100);
}

/* This is a helper for hardware bring-up. Don't remove. */
void
util_wait_for_idle(struct pipe_context *ctx)
{
   struct pipe_fence_handle *fence = NULL;

   ctx->flush(ctx, &fence, 0);
   ctx->screen->fence_finish(ctx->screen, NULL, fence, OS_TIMEOUT_INFINITE);
}

void
util_throttle_init(struct util_throttle *t, uint64_t max_mem_usage)
{
   t->max_mem_usage = max_mem_usage;
}

void
util_throttle_deinit(struct pipe_screen *screen, struct util_throttle *t)
{
   for (unsigned i = 0; i < ARRAY_SIZE(t->ring); i++)
      screen->fence_reference(screen, &t->ring[i].fence, NULL);
}

static uint64_t
util_get_throttle_total_memory_usage(struct util_throttle *t)
{
   uint64_t total_usage = 0;

   for (unsigned i = 0; i < ARRAY_SIZE(t->ring); i++)
      total_usage += t->ring[i].mem_usage;
   return total_usage;
}

static void util_dump_throttle_ring(struct util_throttle *t)
{
   printf("Throttle:\n");
   for (unsigned i = 0; i < ARRAY_SIZE(t->ring); i++) {
      printf("  ring[%u]: fence = %s, mem_usage = %"PRIu64"%s%s\n",
             i, t->ring[i].fence ? "yes" : " no",
             t->ring[i].mem_usage,
             t->flush_index == i ? " [flush]" : "",
             t->wait_index == i ? " [wait]" : "");
   }
}

/**
 * Notify util_throttle that the next operation allocates memory.
 * util_throttle tracks memory usage and waits for fences until its tracked
 * memory usage decreases.
 *
 * Example:
 *   util_throttle_memory_usage(..., w*h*d*Bpp);
 *   TexSubImage(..., w, h, d, ...);
 *
 * This means that TexSubImage can't allocate more memory its maximum limit
 * set during initialization.
 */
void
util_throttle_memory_usage(struct pipe_context *pipe,
                           struct util_throttle *t, uint64_t memory_size)
{
   (void)util_dump_throttle_ring; /* silence warning */

   if (!t->max_mem_usage)
      return;

   MESA_TRACE_FUNC();

   struct pipe_screen *screen = pipe->screen;
   struct pipe_fence_handle **fence = NULL;
   unsigned ring_size = ARRAY_SIZE(t->ring);
   uint64_t total = util_get_throttle_total_memory_usage(t);

   /* If there is not enough memory, walk the list of fences and find
    * the latest one that we need to wait for.
    */
   while (t->wait_index != t->flush_index &&
          total && total + memory_size > t->max_mem_usage) {
      assert(t->ring[t->wait_index].fence);

      /* Release an older fence if we need to wait for a newer one. */
      if (fence)
         screen->fence_reference(screen, fence, NULL);

      fence = &t->ring[t->wait_index].fence;
      t->ring[t->wait_index].mem_usage = 0;
      t->wait_index = (t->wait_index + 1) % ring_size;

      total = util_get_throttle_total_memory_usage(t);
   }

   /* Wait for the fence to decrease memory usage. */
   if (fence) {
      screen->fence_finish(screen, pipe, *fence, OS_TIMEOUT_INFINITE);
      screen->fence_reference(screen, fence, NULL);
   }

   /* Flush and get a fence if we've exhausted memory usage for the current
    * slot.
    */
   if (t->ring[t->flush_index].mem_usage &&
       t->ring[t->flush_index].mem_usage + memory_size >
       t->max_mem_usage / (ring_size / 2)) {
      struct pipe_fence_handle **fence =
         &t->ring[t->flush_index].fence;

      /* Expect that the current flush slot doesn't have a fence yet. */
      assert(!*fence);

      pipe->flush(pipe, fence, PIPE_FLUSH_ASYNC);
      t->flush_index = (t->flush_index + 1) % ring_size;

      /* Vacate the next slot if it's occupied. This should be rare. */
      if (t->flush_index == t->wait_index) {
         struct pipe_fence_handle **fence =
            &t->ring[t->wait_index].fence;

         t->ring[t->wait_index].mem_usage = 0;
         t->wait_index = (t->wait_index + 1) % ring_size;

         assert(*fence);
         screen->fence_finish(screen, pipe, *fence, OS_TIMEOUT_INFINITE);
         screen->fence_reference(screen, fence, NULL);
      }

      assert(!t->ring[t->flush_index].mem_usage);
      assert(!t->ring[t->flush_index].fence);
   }

   t->ring[t->flush_index].mem_usage += memory_size;
}

void
util_sw_query_memory_info(struct pipe_screen *pscreen,
                          struct pipe_memory_info *info)
{
   /* Provide query_memory_info from CPU reported memory */
   uint64_t size;

   if (!os_get_available_system_memory(&size))
      return;
   info->avail_staging_memory = size / 1024;
   if (!os_get_total_physical_memory(&size))
      return;
   info->total_staging_memory = size / 1024;
}

bool
util_lower_clearsize_to_dword(const void *clearValue, int *clearValueSize, uint32_t *clamped)
{
   /* Reduce a large clear value size if possible. */
   if (*clearValueSize > 4) {
      bool clear_dword_duplicated = true;
      const uint32_t *clear_value = clearValue;

      /* See if we can lower large fills to dword fills. */
      for (unsigned i = 1; i < *clearValueSize / 4; i++) {
         if (clear_value[0] != clear_value[i]) {
            clear_dword_duplicated = false;
            break;
         }
      }
      if (clear_dword_duplicated) {
         *clamped = *clear_value;
         *clearValueSize = 4;
      }
      return clear_dword_duplicated;
   }

   /* Expand a small clear value size. */
   if (*clearValueSize <= 2) {
      if (*clearValueSize == 1) {
         *clamped = *(uint8_t *)clearValue;
         *clamped |=
            (*clamped << 8) | (*clamped << 16) | (*clamped << 24);
      } else {
         *clamped = *(uint16_t *)clearValue;
         *clamped |= *clamped << 16;
      }
      *clearValueSize = 4;
      return true;
   }
   return false;
}

void
util_init_pipe_vertex_state(struct pipe_screen *screen,
                            struct pipe_vertex_buffer *buffer,
                            const struct pipe_vertex_element *elements,
                            unsigned num_elements,
                            struct pipe_resource *indexbuf,
                            uint32_t full_velem_mask,
                            struct pipe_vertex_state *state)
{
   assert(num_elements == util_bitcount(full_velem_mask));

   pipe_reference_init(&state->reference, 1);
   state->screen = screen;

   pipe_vertex_buffer_reference(&state->input.vbuffer, buffer);
   pipe_resource_reference(&state->input.indexbuf, indexbuf);
   state->input.num_elements = num_elements;
   for (unsigned i = 0; i < num_elements; i++)
      state->input.elements[i] = elements[i];
   state->input.full_velem_mask = full_velem_mask;
}

/**
 * Clamp color value to format range.
 */
union pipe_color_union
util_clamp_color(enum pipe_format format,
                 const union pipe_color_union *color)
{
   union pipe_color_union clamp_color = *color;
   int i;

   for (i = 0; i < 4; i++) {
      uint8_t bits = util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, i);

      if (!bits)
         continue;

      if (util_format_is_unorm(format))
         clamp_color.f[i] = SATURATE(clamp_color.f[i]);
      else if (util_format_is_snorm(format))
         clamp_color.f[i] = CLAMP(clamp_color.f[i], -1.0, 1.0);
      else if (util_format_is_pure_uint(format))
         clamp_color.ui[i] = _mesa_unsigned_to_unsigned(clamp_color.ui[i], bits);
      else if (util_format_is_pure_sint(format))
         clamp_color.i[i] = _mesa_signed_to_signed(clamp_color.i[i], bits);
   }

   return clamp_color;
}

/*
 * Some hardware does not use a distinct descriptor for images, so it is
 * convenient for drivers to reuse their texture descriptor packing for shader
 * images. This helper constructs a synthetic, non-reference counted
 * pipe_sampler_view corresponding to a given pipe_image_view for drivers'
 * internal convenience.
 *
 * The returned descriptor is "synthetic" in the sense that it is not reference
 * counted and the context field is ignored. Otherwise it's complete.
 */
struct pipe_sampler_view
util_image_to_sampler_view(struct pipe_image_view *v)
{
   struct pipe_sampler_view out = {
      .format = v->format,
      .is_tex2d_from_buf = v->access & PIPE_IMAGE_ACCESS_TEX2D_FROM_BUFFER,
      .target = v->resource->target,
      .swizzle_r = PIPE_SWIZZLE_X,
      .swizzle_g = PIPE_SWIZZLE_Y,
      .swizzle_b = PIPE_SWIZZLE_Z,
      .swizzle_a = PIPE_SWIZZLE_W,
      .texture = v->resource,
   };

   if (out.target == PIPE_BUFFER) {
      out.u.buf.offset = v->u.buf.offset;
      out.u.buf.size = v->u.buf.size;
   } else if (out.is_tex2d_from_buf) {
      out.u.tex2d_from_buf.offset = v->u.tex2d_from_buf.offset;
      out.u.tex2d_from_buf.row_stride = v->u.tex2d_from_buf.row_stride;
      out.u.tex2d_from_buf.width = v->u.tex2d_from_buf.width;
      out.u.tex2d_from_buf.height = v->u.tex2d_from_buf.height;
   } else {
      /* For a single layer view of a multilayer image, we need to swap in the
       * non-layered texture target to match the texture instruction.
       */
      if (v->u.tex.single_layer_view) {
         switch (out.target) {
         case PIPE_TEXTURE_1D_ARRAY:
            /* A single layer is a 1D image */
            out.target = PIPE_TEXTURE_1D;
            break;

         case PIPE_TEXTURE_3D:
         case PIPE_TEXTURE_CUBE:
         case PIPE_TEXTURE_2D_ARRAY:
         case PIPE_TEXTURE_CUBE_ARRAY:
            /* A single layer/face is a 2D image.
             *
             * Note that OpenGL does not otherwise support 2D views of 3D.
             * Drivers that use this helper must support that anyway.
             */
            out.target = PIPE_TEXTURE_2D;
            break;

         default:
            /* Other texture targets already only have 1 layer, nothing to do */
            break;
         }
      }

      out.u.tex.first_layer = v->u.tex.first_layer;
      out.u.tex.last_layer = v->u.tex.last_layer;

      /* Single level only */
      out.u.tex.first_level = v->u.tex.level;
      out.u.tex.last_level = v->u.tex.level;
   }

   return out;
}
