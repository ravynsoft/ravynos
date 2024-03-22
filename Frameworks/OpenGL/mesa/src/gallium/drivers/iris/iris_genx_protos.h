/*
 * Copyright © 2019 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* GenX-specific function declarations.
 *
 * Don't include this directly, it will be included by iris_context.h.
 *
 * NOTE: This header can be included multiple times, from the same file.
 */

/* iris_state.c */
void genX(init_state)(struct iris_context *ice);
void genX(init_screen_state)(struct iris_screen *screen);
void genX(emit_hashing_mode)(struct iris_context *ice,
                             struct iris_batch *batch,
                             unsigned width, unsigned height,
                             unsigned scale);
void genX(emit_depth_state_workarounds)(struct iris_context *ice,
                                        struct iris_batch *batch,
                                        const struct isl_surf *surf);
void genX(update_pma_fix)(struct iris_context *ice,
                          struct iris_batch *batch,
                          bool enable);

void genX(invalidate_aux_map_state)(struct iris_batch *batch);

void genX(emit_breakpoint)(struct iris_batch *batch, bool emit_before_draw);
void genX(emit_3dprimitive_was)(struct iris_batch *batch,
                                const struct pipe_draw_indirect_info *indirect,
                                uint32_t primitive_topology,
                                uint32_t vertex_count);

static inline void
genX(maybe_emit_breakpoint)(struct iris_batch *batch,
                            bool emit_before_draw)
{
   if (INTEL_DEBUG(DEBUG_DRAW_BKP))
      genX(emit_breakpoint)(batch, emit_before_draw);
}


/* iris_blorp.c */
void genX(init_blorp)(struct iris_context *ice);

/* iris_query.c */
void genX(init_query)(struct iris_context *ice);
void genX(math_add32_gpr0)(struct iris_context *ice,
                           struct iris_batch *batch,
                           uint32_t x);
void genX(math_div32_gpr0)(struct iris_context *ice,
                           struct iris_batch *batch,
                           uint32_t D);

