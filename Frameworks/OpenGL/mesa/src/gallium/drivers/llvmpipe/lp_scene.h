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


/**
 * Binner data structures and bin-related functions.
 * Note: the "setup" code is concerned with building scenes while
 * The "rast" code is concerned with consuming/executing scenes.
 */

#ifndef LP_SCENE_H
#define LP_SCENE_H

#include "util/u_thread.h"
#include "lp_rast.h"
#include "lp_debug.h"

struct lp_scene_queue;
struct lp_rast_state;

/* We're limited to 2K by 2K for 32bit fixed point rasterization.
 * Will need a 64-bit version for larger framebuffers.
 */
#define TILES_X (LP_MAX_WIDTH / TILE_SIZE)
#define TILES_Y (LP_MAX_HEIGHT / TILE_SIZE)


/* Commands per command block (ideally so sizeof(cmd_block) is a power of
 * two in size.)
 */
#define CMD_BLOCK_MAX 29

/* Bytes per data block.  This effectively limits the maximum constant buffer
 * size.
 */
#define DATA_BLOCK_SIZE (64 * 1024)

/* Scene temporary storage is clamped to this size:
 */
#define LP_SCENE_MAX_SIZE (36*1024*1024)

/* The maximum amount of texture storage referenced by a scene is
 * clamped to this size:
 */
#define LP_SCENE_MAX_RESOURCE_SIZE (64*1024*1024)


/* switch to a non-pointer value for this:
 */
typedef void (*lp_rast_cmd_func)(struct lp_rasterizer_task *,
                                 const union lp_rast_cmd_arg);


struct cmd_block {
   uint8_t cmd[CMD_BLOCK_MAX];  // LP_RAST_OP_x
   union lp_rast_cmd_arg arg[CMD_BLOCK_MAX];
   unsigned count;
   struct cmd_block *next;
};


struct data_block {
   uint8_t data[DATA_BLOCK_SIZE];
   unsigned used;
   struct data_block *next;
};



/**
 * For each screen tile we have one of these bins.
 */
struct cmd_bin {
   const struct lp_rast_state *last_state;  /* most recent state set in bin */
   struct cmd_block *head;
   struct cmd_block *tail;
};


/**
 * This stores bulk data which is used for all memory allocations
 * within a scene.
 *
 * Examples include triangle data and state data.  The commands in
 * the per-tile bins will point to chunks of data in this structure.
 *
 * Include the first block of data statically to ensure we can always
 * initiate a scene without relying on malloc succeeding.
 */
struct data_block_list {
   struct data_block first;
   struct data_block *head;
};

struct resource_ref;

struct shader_ref;

struct lp_scene_surface {
   uint8_t *map;
   unsigned stride;
   unsigned layer_stride;
   unsigned format_bytes;
   unsigned sample_stride;
   unsigned nr_samples;
};


/**
 * All bins and bin data are contained here.
 * Per-bin data goes into the 'tile' bins.
 * Shared data goes into the 'data' buffer.
 *
 * When there are multiple threads, will want to double-buffer between
 * scenes:
 */
struct lp_scene {
   struct pipe_context *pipe;
   struct lp_fence *fence;
   struct lp_setup_context *setup;

   /* The queries still active at end of scene */
   struct llvmpipe_query *active_queries[LP_MAX_ACTIVE_BINNED_QUERIES];
   unsigned num_active_queries;
   /* If queries were either active or there were begin/end query commands */
   bool had_queries;

   /* Framebuffer mappings - valid only between begin_rasterization()
    * and end_rasterization().
    */
   struct lp_scene_surface zsbuf, cbufs[PIPE_MAX_COLOR_BUFS];

   /* The amount of layers in the fb (minimum of all attachments) */
   unsigned fb_max_layer;

   /* fixed point sample positions. */
   int32_t fixed_sample_pos[LP_MAX_SAMPLES][2];

   /* max samples for bound framebuffer */
   unsigned fb_max_samples;

   /** the framebuffer to render the scene into */
   struct pipe_framebuffer_state fb;

   /** list of resources referenced by the scene commands */
   struct resource_ref *resources;

   /** list of writable resources referenced by the scene commands */
   struct resource_ref *writeable_resources;

   /** list of frag shaders referenced by the scene commands */
   struct shader_ref *frag_shaders;

   /** Total memory used by the scene (in bytes).  This sums all the
    * data blocks and counts all bins, state, resource references and
    * other random allocations within the scene.
    */
   unsigned scene_size;

   /** Sum of sizes of all resources referenced by the scene.  Sums
    * all the textures read by the scene:
    */
   unsigned resource_reference_size;

   bool alloc_failed;
   bool permit_linear_rasterizer;

   /**
    * Number of active tiles in each dimension.
    * This basically the framebuffer size divided by tile size
    */
   unsigned tiles_x, tiles_y;

   int curr_x, curr_y;  /**< for iterating over bins */
   mtx_t mutex;

   unsigned num_alloced_tiles;
   struct cmd_bin *tiles;
   struct data_block_list data;
};



struct lp_scene *lp_scene_create(struct lp_setup_context *setup);

void lp_scene_destroy(struct lp_scene *scene);

bool lp_scene_is_empty(struct lp_scene *scene);

bool lp_scene_is_oom(struct lp_scene *scene);

struct data_block *lp_scene_new_data_block(struct lp_scene *scene);

struct cmd_block *lp_scene_new_cmd_block(struct lp_scene *scene,
                                         struct cmd_bin *bin);

bool lp_scene_add_resource_reference(struct lp_scene *scene,
                                     struct pipe_resource *resource,
                                     bool initializing_scene,
                                     bool writeable);

unsigned lp_scene_is_resource_referenced(const struct lp_scene *scene,
                                         const struct pipe_resource *resource);

bool lp_scene_add_frag_shader_reference(struct lp_scene *scene,
                                        struct lp_fragment_shader_variant *variant);



/**
 * Allocate space for a command/data in the bin's data buffer.
 * Grow the block list if needed.
 */
static inline void *
lp_scene_alloc(struct lp_scene *scene, unsigned size)
{
   struct data_block_list *list = &scene->data;
   struct data_block *block = list->head;

   assert(size <= DATA_BLOCK_SIZE);
   assert(block != NULL);

   if (LP_DEBUG & DEBUG_MEM)
      debug_printf("alloc %u block %u/%u tot %u/%u\n",
                   size, block->used, (unsigned)DATA_BLOCK_SIZE,
                   scene->scene_size, LP_SCENE_MAX_SIZE);

   if (block->used + size > DATA_BLOCK_SIZE) {
      block = lp_scene_new_data_block(scene);
      if (!block) {
         /* out of memory */
         return NULL;
      }
   }

   {
      uint8_t *data = block->data + block->used;
      block->used += size;
      return data;
   }
}


/**
 * As above, but with specific alignment.
 */
static inline void *
lp_scene_alloc_aligned(struct lp_scene *scene, unsigned size,
                       unsigned alignment)
{
   struct data_block_list *list = &scene->data;
   struct data_block *block = list->head;

   assert(block != NULL);

   if (LP_DEBUG & DEBUG_MEM)
      debug_printf("alloc %u block %u/%u tot %u/%u\n",
                   size + alignment - 1,
                   block->used, (unsigned)DATA_BLOCK_SIZE,
                   scene->scene_size, LP_SCENE_MAX_SIZE);

   if (block->used + size + alignment - 1 > DATA_BLOCK_SIZE) {
      block = lp_scene_new_data_block(scene);
      if (!block)
         return NULL;
   }

   {
      uint8_t *data = block->data + block->used;
      unsigned offset = (((uintptr_t)data + alignment - 1) & ~(alignment - 1)) - (uintptr_t)data;
      block->used += offset + size;
      return data + offset;
   }
}


/** Return pointer to a particular tile's bin. */
static inline struct cmd_bin *
lp_scene_get_bin(struct lp_scene *scene, unsigned x, unsigned y)
{
   unsigned idx = scene->tiles_x * y + x;
   return &scene->tiles[idx];
}


/** Remove all commands from a bin */
void
lp_scene_bin_reset(struct lp_scene *scene, unsigned x, unsigned y);


/* Add a command to bin[x][y].
 */
static inline bool
lp_scene_bin_command(struct lp_scene *scene,
                     unsigned x, unsigned y,
                     enum lp_rast_op cmd,
                     union lp_rast_cmd_arg arg)
{
   struct cmd_bin *bin = lp_scene_get_bin(scene, x, y);
   struct cmd_block *tail = bin->tail;

   assert(x < scene->tiles_x);
   assert(y < scene->tiles_y);
   assert(cmd < LP_RAST_OP_MAX);

   if (tail == NULL || tail->count == CMD_BLOCK_MAX) {
      tail = lp_scene_new_cmd_block(scene, bin);
      if (!tail) {
         return false;
      }
      assert(tail->count == 0);
   }

   {
      unsigned i = tail->count;
      tail->cmd[i] = cmd & LP_RAST_OP_MASK;
      tail->arg[i] = arg;
      tail->count++;
   }

   return true;
}


static inline bool
lp_scene_bin_cmd_with_state(struct lp_scene *scene,
                            unsigned x, unsigned y,
                            const struct lp_rast_state *state,
                            enum lp_rast_op cmd,
                            union lp_rast_cmd_arg arg)
{
   struct cmd_bin *bin = lp_scene_get_bin(scene, x, y);

   if (state != bin->last_state) {
      bin->last_state = state;
      if (!lp_scene_bin_command(scene, x, y,
                                LP_RAST_OP_SET_STATE,
                                lp_rast_arg_state(state)))
         return false;
   }

   if (!lp_scene_bin_command(scene, x, y, cmd, arg))
      return false;

   return true;
}


/* Add a command to all active bins.
 */
static inline bool
lp_scene_bin_everywhere(struct lp_scene *scene,
                        enum lp_rast_op cmd,
                        const union lp_rast_cmd_arg arg)
{
   for (unsigned i = 0; i < scene->tiles_x; i++) {
      for (unsigned j = 0; j < scene->tiles_y; j++) {
         if (!lp_scene_bin_command(scene, i, j, cmd, arg))
            return false;
      }
   }

   return true;
}


static inline unsigned
lp_scene_get_num_bins(const struct lp_scene *scene)
{
   return scene->tiles_x * scene->tiles_y;
}


void
lp_scene_bin_iter_begin(struct lp_scene *scene);

struct cmd_bin *
lp_scene_bin_iter_next(struct lp_scene *scene, int *x, int *y);



/* Begin/end binning of a scene
 */
void
lp_scene_begin_binning(struct lp_scene *scene,
                       struct pipe_framebuffer_state *fb);

void
lp_scene_end_binning(struct lp_scene *scene);


/* Begin/end rasterization of a scene
 */
void
lp_scene_begin_rasterization(struct lp_scene *scene);

void
lp_scene_end_rasterization(struct lp_scene *scene);


#endif /* LP_SCENE_H */
