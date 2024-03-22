/**************************************************************************
 *
 * Copyright 2009-2021 VMware, Inc.
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

#include <limits.h>
#include "util/u_memory.h"
#include "util/u_math.h"
#include "util/u_rect.h"
#include "util/u_surface.h"
#include "util/u_pack_color.h"

#include "lp_scene_queue.h"
#include "lp_debug.h"
#include "lp_fence.h"
#include "lp_perf.h"
#include "lp_query.h"
#include "lp_rast.h"
#include "lp_rast_priv.h"
#include "lp_scene.h"


static void
lp_rast_linear_clear(struct lp_rasterizer_task *task,
                     const union lp_rast_cmd_arg arg)
{
   LP_DBG(DEBUG_RAST, "%s\n", __func__);

   union util_color uc = arg.clear_rb->color_val;

   const struct lp_scene *scene = task->scene;
   util_fill_rect(scene->cbufs[0].map,
                  PIPE_FORMAT_B8G8R8A8_UNORM,
                  scene->cbufs[0].stride,
                  task->x,
                  task->y,
                  task->width,
                  task->height,
                  &uc);
}


/* Run the scanline version of the shader across the whole tile.
 */
static void
lp_rast_linear_tile(struct lp_rasterizer_task *task,
                    const union lp_rast_cmd_arg arg)
{
   const struct lp_rast_shader_inputs *inputs = arg.shade_tile;
   if (inputs->disable)
      return;

   const struct lp_rast_state *state = task->state;
   assert(state);
   if (!state) {
      return;
   }

   const struct lp_fragment_shader_variant *variant = state->variant;
   const struct lp_scene *scene = task->scene;

   if (variant->jit_linear_blit && inputs->is_blit) {
      if (variant->jit_linear_blit(state,
                                   task->x,
                                   task->y,
                                   task->width,
                                   task->height,
                                   GET_A0(inputs),
                                   GET_DADX(inputs),
                                   GET_DADY(inputs),
                                   scene->cbufs[0].map,
                                   scene->cbufs[0].stride))
         return;
   }

   if (variant->jit_linear) {
      if (variant->jit_linear(state,
                              task->x,
                              task->y,
                              task->width,
                              task->height,
                              GET_A0(inputs),
                              GET_DADX(inputs),
                              GET_DADY(inputs),
                              scene->cbufs[0].map,
                              scene->cbufs[0].stride))
         return;
   }

   {
      struct u_rect box;
      box.x0 = task->x;
      box.x1 = task->x + task->width - 1;
      box.y0 = task->y;
      box.y1 = task->y + task->height - 1;
      lp_rast_linear_rect_fallback(task, inputs, &box);
   }
}


/* Run the scanline version of the shader on a rectangle within the
 * tile.
 */
static void
lp_rast_linear_rect(struct lp_rasterizer_task *task,
                    const union lp_rast_cmd_arg arg)
{
   const struct lp_scene *scene = task->scene;
   const struct lp_rast_rectangle *rect = arg.rectangle;
   const struct lp_rast_shader_inputs *inputs = &rect->inputs;

   if (inputs->disable)
      return;

   struct u_rect box;
   box.x0 = task->x;
   box.y0 = task->y;
   box.x1 = task->x + task->width - 1;
   box.y1 = task->y + task->height - 1;

   u_rect_find_intersection(&rect->box, &box);

   const int width  = box.x1 - box.x0 + 1;
   const int height = box.y1 - box.y0 + 1;

   /* Note that blit primitives can end up in the non-full-tile path,
    * the binner currently doesn't try to classify sub-tile
    * primitives.  Can detect them here though.
    */
   const struct lp_rast_state *state = task->state;
   struct lp_fragment_shader_variant *variant = state->variant;
   if (variant->jit_linear_blit && inputs->is_blit) {
      if (variant->jit_linear_blit(state,
                                   box.x0, box.y0,
                                   width, height,
                                   GET_A0(inputs),
                                   GET_DADX(inputs),
                                   GET_DADY(inputs),
                                   scene->cbufs[0].map,
                                   scene->cbufs[0].stride)) {
         return;
      }
   }

   if (variant->jit_linear) {
      if (variant->jit_linear(state,
                              box.x0, box.y0,
                              width, height,
                              GET_A0(inputs),
                              GET_DADX(inputs),
                              GET_DADY(inputs),
                              scene->cbufs[0].map,
                              scene->cbufs[0].stride)) {
         return;
      }
   }

   lp_rast_linear_rect_fallback(task, inputs, &box);
}


static const lp_rast_cmd_func
dispatch_linear[] = {
   lp_rast_linear_clear,        /* clear_color */
   NULL,                        /* clear_zstencil */
   NULL,                        /* triangle_1 */
   NULL,                        /* triangle_2 */
   NULL,                        /* triangle_3 */
   NULL,                        /* triangle_4 */
   NULL,                        /* triangle_5 */
   NULL,                        /* triangle_6 */
   NULL,                        /* triangle_7 */
   NULL,                        /* triangle_8 */
   NULL,                        /* triangle_3_4 */
   NULL,                        /* triangle_3_16 */
   NULL,                        /* triangle_4_16 */
   lp_rast_linear_tile,         /* shade_tile */
   lp_rast_linear_tile,         /* shade_tile_opaque */
   NULL,                        /* begin_query */
   NULL,                        /* end_query */
   lp_rast_set_state,           /* set_state */
   NULL,                        /* lp_rast_triangle_32_1 */
   NULL,                        /* lp_rast_triangle_32_2 */
   NULL,                        /* lp_rast_triangle_32_3 */
   NULL,                        /* lp_rast_triangle_32_4 */
   NULL,                        /* lp_rast_triangle_32_5 */
   NULL,                        /* lp_rast_triangle_32_6 */
   NULL,                        /* lp_rast_triangle_32_7 */
   NULL,                        /* lp_rast_triangle_32_8 */
   NULL,                        /* lp_rast_triangle_32_3_4 */
   NULL,                        /* lp_rast_triangle_32_3_16 */
   NULL,                        /* lp_rast_triangle_32_4_16 */

   NULL,                        /* lp_rast_triangle_ms_1 */
   NULL,                        /* lp_rast_triangle_ms_2 */
   NULL,                        /* lp_rast_triangle_ms_3 */
   NULL,                        /* lp_rast_triangle_ms_4 */
   NULL,                        /* lp_rast_triangle_ms_5 */
   NULL,                        /* lp_rast_triangle_ms_6 */
   NULL,                        /* lp_rast_triangle_ms_7 */
   NULL,                        /* lp_rast_triangle_ms_8 */
   NULL,                        /* lp_rast_triangle_ms_3_4 */
   NULL,                        /* lp_rast_triangle_ms_3_16 */
   NULL,                        /* lp_rast_triangle_ms_4_16 */

   lp_rast_linear_rect,         /* rect */
   lp_rast_linear_tile,         /* blit */
};


/* Assumptions for this path:
 *   - Single color buffer, PIPE_FORMAT_B8G8R8A8_UNORM
 *   - No depth buffer
 *   - All primitives in bins are rect, tile, blit or clear.
 *   - All shaders have a linear variant.
 */
void
lp_linear_rasterize_bin(struct lp_rasterizer_task *task,
                        const struct cmd_bin *bin)
{
   STATIC_ASSERT(ARRAY_SIZE(dispatch_linear) == LP_RAST_OP_MAX);

   if (0) debug_printf("%s\n", __func__);

   const struct cmd_block *block;
   for (block = bin->head; block; block = block->next) {
      for (unsigned k = 0; k < block->count; k++) {
         assert(dispatch_linear[block->cmd[k]]);
         dispatch_linear[block->cmd[k]](task, block->arg[k]);
      }
   }
}
