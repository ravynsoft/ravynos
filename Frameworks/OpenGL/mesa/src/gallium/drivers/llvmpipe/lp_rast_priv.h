/**************************************************************************
 *
 * Copyright 2009 VMware, Inc.
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

#ifndef LP_RAST_PRIV_H
#define LP_RAST_PRIV_H

#include "util/format/u_format.h"
#include "util/u_thread.h"
#include "gallivm/lp_bld_debug.h"
#include "lp_memory.h"
#include "lp_rast.h"
#include "lp_scene.h"
#include "lp_state.h"
#include "lp_texture.h"
#include "lp_limits.h"


#define TILE_VECTOR_HEIGHT 4
#define TILE_VECTOR_WIDTH 4

/* If we crash in a jitted function, we can examine jit_line and jit_state
 * to get some info.  This is not thread-safe, however.
 */
#ifdef DEBUG

struct lp_rasterizer_task;
extern int jit_line;
extern const struct lp_rast_state *jit_state;
extern const struct lp_rasterizer_task *jit_task;

#define BEGIN_JIT_CALL(state, task) \
   do { \
      jit_line = __LINE__; \
      jit_state = state; \
      jit_task = task; \
   } while (0)

#define END_JIT_CALL() \
   do { \
      jit_line = 0; \
      jit_state = NULL; \
   } while (0)

#else

#define BEGIN_JIT_CALL(X, Y)
#define END_JIT_CALL()

#endif


struct lp_rasterizer;
struct cmd_bin;

/**
 * Per-thread rasterization state
 */
struct lp_rasterizer_task
{
   const struct cmd_bin *bin;
   const struct lp_rast_state *state;

   struct lp_scene *scene;
   unsigned x, y;          /**< Pos of this tile in framebuffer, in pixels */
   unsigned width, height; /**< width, height of current tile, in pixels */

   uint8_t *color_tiles[PIPE_MAX_COLOR_BUFS];
   uint8_t *depth_tile;

   /** "back" pointer */
   struct lp_rasterizer *rast;

   /** "my" index */
   unsigned thread_index;

   /** Non-interpolated passthru state and occlude counter for visible pixels */
   struct lp_jit_thread_data thread_data;

   util_semaphore work_ready;
   util_semaphore work_done;
};


/**
 * This is the state required while rasterizing tiles.
 * Note that this contains per-thread information too.
 * The tile size is TILE_SIZE x TILE_SIZE pixels.
 */
struct lp_rasterizer
{
   bool exit_flag;
   bool no_rast;  /**< For debugging/profiling */

   /** The incoming queue of scenes ready to rasterize */
   struct lp_scene_queue *full_scenes;

   /** The scene currently being rasterized by the threads */
   struct lp_scene *curr_scene;

   /** A task object for each rasterization thread */
   struct lp_rasterizer_task tasks[LP_MAX_THREADS];

   unsigned num_threads;
   thrd_t threads[LP_MAX_THREADS];

   /** For synchronizing the rasterization threads */
   util_barrier barrier;

   struct lp_fence *last_fence;
};


void
lp_rast_shade_quads_mask_sample(struct lp_rasterizer_task *task,
                                const struct lp_rast_shader_inputs *inputs,
                                unsigned x, unsigned y,
                                uint64_t mask);

void
lp_rast_shade_quads_mask(struct lp_rasterizer_task *task,
                         const struct lp_rast_shader_inputs *inputs,
                         unsigned x, unsigned y,
                         unsigned mask);


/**
 * Get the pointer to a 4x4 color block (within a 64x64 tile).
 * \param x, y location of 4x4 block in window coords
 */
static inline uint8_t *
lp_rast_get_color_block_pointer(struct lp_rasterizer_task *task,
                                unsigned buf, unsigned x, unsigned y,
                                unsigned layer)
{
   assert(x < task->scene->tiles_x * TILE_SIZE);
   assert(y < task->scene->tiles_y * TILE_SIZE);
   assert((x % TILE_VECTOR_WIDTH) == 0);
   assert((y % TILE_VECTOR_HEIGHT) == 0);
   assert(buf < task->scene->fb.nr_cbufs);
   assert(task->color_tiles[buf]);

   /*
    * We don't actually benefit from having per tile cbuf/zsbuf pointers,
    * it's just extra work - the mul/add would be exactly the same anyway.
    * Fortunately the extra work (modulo) here is very cheap at least...
    */
   unsigned px = x % TILE_SIZE;
   unsigned py = y % TILE_SIZE;

   unsigned pixel_offset = px * task->scene->cbufs[buf].format_bytes +
                           py * task->scene->cbufs[buf].stride;
   uint8_t *color = task->color_tiles[buf] + pixel_offset;

   if (layer) {
      assert(layer <= task->scene->fb_max_layer);
      color += layer * task->scene->cbufs[buf].layer_stride;
   }

   assert(lp_check_alignment(color, llvmpipe_get_format_alignment(task->scene->fb.cbufs[buf]->format)));
   return color;
}


/**
 * Get the pointer to a 4x4 depth block (within a 64x64 tile).
 * \param x, y location of 4x4 block in window coords
 */
static inline uint8_t *
lp_rast_get_depth_block_pointer(struct lp_rasterizer_task *task,
                                unsigned x, unsigned y, unsigned layer)
{
   assert(x < task->scene->tiles_x * TILE_SIZE);
   assert(y < task->scene->tiles_y * TILE_SIZE);
   assert((x % TILE_VECTOR_WIDTH) == 0);
   assert((y % TILE_VECTOR_HEIGHT) == 0);
   assert(task->depth_tile);

   unsigned px = x % TILE_SIZE;
   unsigned py = y % TILE_SIZE;

   unsigned pixel_offset = px * task->scene->zsbuf.format_bytes +
                           py * task->scene->zsbuf.stride;
   uint8_t *depth = task->depth_tile + pixel_offset;

   if (layer) {
      depth += layer * task->scene->zsbuf.layer_stride;
   }

   assert(lp_check_alignment(depth, llvmpipe_get_format_alignment(task->scene->fb.zsbuf->format)));
   return depth;
}


/**
 * Shade all pixels in a 4x4 block.  The fragment code omits the
 * triangle in/out tests.
 * \param x, y location of 4x4 block in window coords
 */
static inline void
lp_rast_shade_quads_all(struct lp_rasterizer_task *task,
                        const struct lp_rast_shader_inputs *inputs,
                        unsigned x, unsigned y)
{
   const struct lp_scene *scene = task->scene;
   const struct lp_rast_state *state = task->state;
   struct lp_fragment_shader_variant *variant = state->variant;
   uint8_t *color[PIPE_MAX_COLOR_BUFS];
   unsigned stride[PIPE_MAX_COLOR_BUFS];
   unsigned sample_stride[PIPE_MAX_COLOR_BUFS];
   uint8_t *depth = NULL;
   unsigned depth_stride = 0;
   unsigned depth_sample_stride = 0;

   /* color buffer */
   for (unsigned i = 0; i < scene->fb.nr_cbufs; i++) {
      if (scene->fb.cbufs[i]) {
         stride[i] = scene->cbufs[i].stride;
         sample_stride[i] = scene->cbufs[i].sample_stride;
         color[i] = lp_rast_get_color_block_pointer(task, i, x, y,
                                                    inputs->layer + inputs->view_index);
      } else {
         stride[i] = 0;
         sample_stride[i] = 0;
         color[i] = NULL;
      }
   }

   if (scene->zsbuf.map) {
      depth = lp_rast_get_depth_block_pointer(task, x, y, inputs->layer + inputs->view_index);
      depth_sample_stride = scene->zsbuf.sample_stride;
      depth_stride = scene->zsbuf.stride;
   }

   uint64_t mask = 0;
   for (unsigned i = 0; i < scene->fb_max_samples; i++)
      mask |= (uint64_t)0xffff << (16 * i);

   /*
    * The rasterizer may produce fragments outside our
    * allocated 4x4 blocks hence need to filter them out here.
    */
   if ((x % TILE_SIZE) < task->width && (y % TILE_SIZE) < task->height) {
      /* Propagate non-interpolated raster state. */
      task->thread_data.raster_state.viewport_index = inputs->viewport_index;
      task->thread_data.raster_state.view_index = inputs->view_index;

      /* run shader on 4x4 block */
      BEGIN_JIT_CALL(state, task);
      variant->jit_function[RAST_WHOLE](&state->jit_context,
                                        &state->jit_resources,
                                        x, y,
                                        inputs->frontfacing,
                                        GET_A0(inputs),
                                        GET_DADX(inputs),
                                        GET_DADY(inputs),
                                        color,
                                        depth,
                                        mask,
                                        &task->thread_data,
                                        stride,
                                        depth_stride,
                                        sample_stride,
                                        depth_sample_stride);
      END_JIT_CALL();
   }
}

void
lp_rast_triangle_1(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_2(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_3(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_4(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_5(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_6(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_7(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_8(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_3_4(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_3_16(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_4_16(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_1(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_2(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_3(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_4(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_5(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_6(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_7(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_8(struct lp_rasterizer_task *, const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_3_4(struct lp_rasterizer_task *,
                        const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_3_16(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_32_4_16(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_rectangle(struct lp_rasterizer_task *,
                  const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_1(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_2(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_3(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_4(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_5(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_6(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_7(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_8(struct lp_rasterizer_task *,
                      const union lp_rast_cmd_arg);


void
lp_rast_triangle_ms_3_4(struct lp_rasterizer_task *,
                        const union lp_rast_cmd_arg);


void
lp_rast_triangle_ms_3_16(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);


void
lp_rast_triangle_ms_4_16(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);


void
lp_rast_triangle_ms_32_1(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_2(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_3(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_4(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_5(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_6(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_7(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_8(struct lp_rasterizer_task *,
                         const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_3_4(struct lp_rasterizer_task *,
                           const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_3_16(struct lp_rasterizer_task *,
                            const union lp_rast_cmd_arg);

void
lp_rast_triangle_ms_32_4_16(struct lp_rasterizer_task *,
                            const union lp_rast_cmd_arg);

void
lp_rast_set_state(struct lp_rasterizer_task *task,
                  const union lp_rast_cmd_arg arg);

void
lp_debug_bin(const struct cmd_bin *bin, int x, int y);

void
lp_linear_rasterize_bin(struct lp_rasterizer_task *task,
                        const struct cmd_bin *bin);

void
lp_rast_linear_rect_fallback(struct lp_rasterizer_task *task,
                             const struct lp_rast_shader_inputs *inputs,
                             const struct u_rect *box);

#endif
