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

#ifndef U_HELPERS_H
#define U_HELPERS_H

#include "pipe/p_state.h"
#include "c11/threads.h"
#include "compiler/shader_enums.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void util_set_vertex_buffers_mask(struct pipe_vertex_buffer *dst,
                                  uint32_t *enabled_buffers,
                                  const struct pipe_vertex_buffer *src,
                                  unsigned count,
                                  unsigned unbind_num_trailing_slots,
                                  bool take_ownership);

void util_set_vertex_buffers_count(struct pipe_vertex_buffer *dst,
                                   unsigned *dst_count,
                                   const struct pipe_vertex_buffer *src,
                                   unsigned count,
                                   unsigned unbind_num_trailing_slots,
                                   bool take_ownership);

void util_set_shader_buffers_mask(struct pipe_shader_buffer *dst,
                                  uint32_t *enabled_buffers,
                                  const struct pipe_shader_buffer *src,
                                  unsigned start_slot, unsigned count);

bool util_upload_index_buffer(struct pipe_context *pipe,
                              const struct pipe_draw_info *info,
                              const struct pipe_draw_start_count_bias *draw,
                              struct pipe_resource **out_buffer,
                              unsigned *out_offset, unsigned alignment);

void
util_lower_uint64_vertex_elements(const struct pipe_vertex_element **velems,
                                  unsigned *velem_count,
                                  struct pipe_vertex_element tmp[PIPE_MAX_ATTRIBS]);

/* Helper function to determine if the varying should contain the point
 * coordinates, given the sprite_coord_enable mask.  Requires
 * PIPE_CAP_TGSI_TEXCOORD to be enabled.
 */
static inline bool
util_varying_is_point_coord(gl_varying_slot slot, uint32_t sprite_coord_enable)
{
   if (slot == VARYING_SLOT_PNTC)
      return true;

   if (slot >= VARYING_SLOT_TEX0 && slot <= VARYING_SLOT_TEX7 &&
       (sprite_coord_enable & (1 << (slot - VARYING_SLOT_TEX0)))) {
      return true;
   }

   return false;
}

struct pipe_query *
util_begin_pipestat_query(struct pipe_context *ctx);

void
util_end_pipestat_query(struct pipe_context *ctx, struct pipe_query *q,
                        FILE *f);

struct pipe_query *
util_begin_time_query(struct pipe_context *ctx);
void
util_end_time_query(struct pipe_context *ctx, struct pipe_query *q, FILE *f,
                    const char *name);

void
util_wait_for_idle(struct pipe_context *ctx);

/* A utility for throttling execution based on memory usage. */
struct util_throttle {
   struct {
      struct pipe_fence_handle *fence;
      uint64_t mem_usage;
   } ring[10];

   unsigned flush_index;
   unsigned wait_index;
   uint64_t max_mem_usage;
};

void util_throttle_init(struct util_throttle *t, uint64_t max_mem_usage);
void util_throttle_deinit(struct pipe_screen *screen, struct util_throttle *t);
void util_throttle_memory_usage(struct pipe_context *pipe,
                                struct util_throttle *t, uint64_t memory_size);
void util_sw_query_memory_info(struct pipe_screen *pscreen,
                          struct pipe_memory_info *info);

bool
util_lower_clearsize_to_dword(const void *clearValue, int *clearValueSize, uint32_t *clamped);

void
util_init_pipe_vertex_state(struct pipe_screen *screen,
                            struct pipe_vertex_buffer *buffer,
                            const struct pipe_vertex_element *elements,
                            unsigned num_elements,
                            struct pipe_resource *indexbuf,
                            uint32_t full_velem_mask,
                            struct pipe_vertex_state *state);

union pipe_color_union util_clamp_color(enum pipe_format format,
                                        const union pipe_color_union *color);

struct pipe_sampler_view
util_image_to_sampler_view(struct pipe_image_view *v);

#ifdef __cplusplus
}
#endif

#endif
