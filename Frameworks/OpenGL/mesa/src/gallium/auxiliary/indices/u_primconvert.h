/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#ifndef U_PRIMCONVERT_H_
#define U_PRIMCONVERT_H_

#include "pipe/p_state.h"

#ifdef __cplusplus
extern "C" {
#endif

struct primconvert_context;

struct primconvert_config {
   uint32_t primtypes_mask;
   uint32_t restart_primtypes_mask;
   bool fixed_prim_restart;
};

struct primconvert_context *util_primconvert_create(struct pipe_context *pipe,
                                                    uint32_t primtypes_mask);
struct primconvert_context *util_primconvert_create_config(struct pipe_context *pipe,
                                                           struct primconvert_config *cfg);

void util_primconvert_destroy(struct primconvert_context *pc);
void util_primconvert_save_rasterizer_state(struct primconvert_context *pc,
                                            const struct pipe_rasterizer_state
                                            *rast);
void
util_primconvert_save_flatshade_first(struct primconvert_context *pc, bool flatshade_first);
void util_primconvert_draw_vbo(struct primconvert_context *pc,
                               const struct pipe_draw_info *info,
                               unsigned drawid_offset,
                               const struct pipe_draw_indirect_info *indirect,
                               const struct pipe_draw_start_count_bias *draws,
                               unsigned num_draws);
void
util_primconvert_draw_vertex_state(struct primconvert_context *pc,
                                   struct pipe_vertex_state *state,
                                   uint32_t partial_velem_mask,
                                   struct pipe_draw_vertex_state_info info,
                                   const struct pipe_draw_start_count_bias *draws,
                                   unsigned num_draws);

#ifdef __cplusplus
}
#endif

#endif /* U_PRIMCONVERT_H_ */
