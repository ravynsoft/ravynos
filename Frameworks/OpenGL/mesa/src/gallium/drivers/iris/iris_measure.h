/*
 * Copyright © 2019 Intel Corporation
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

#ifndef IRIS_MEASURE_H
#define IRIS_MEASURE_H

#include "common/intel_measure.h"
#include "pipe/p_state.h"
struct iris_screen;

struct iris_measure_batch {
   struct iris_bo *bo;
   struct intel_measure_batch base;
};

void iris_init_screen_measure(struct iris_screen *screen);
void iris_init_batch_measure(struct iris_context *ice,
                             struct iris_batch *batch);
void iris_destroy_batch_measure(struct iris_measure_batch *batch);
void iris_destroy_ctx_measure(struct iris_context *ice);
void iris_destroy_screen_measure(struct iris_screen *screen);
void iris_measure_frame_end(struct iris_context *ice);
void iris_measure_batch_end(struct iris_context *ice, struct iris_batch *batch);
void _iris_measure_snapshot(struct iris_context *ice,
                            struct iris_batch *batch,
                            enum intel_measure_snapshot_type type,
                            const struct pipe_draw_info *draw,
                            const struct pipe_draw_indirect_info *indirect,
                            const struct pipe_draw_start_count_bias *sc);

#define iris_measure_snapshot(ice, batch, type, draw, indirect, start_count)        \
   if (unlikely(((struct iris_screen *) ice->ctx.screen)->measure.config)) \
      _iris_measure_snapshot(ice, batch, type, draw, indirect, start_count)

#endif  /* IRIS_MEASURE_H */
