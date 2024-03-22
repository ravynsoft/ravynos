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

#include "util/u_framebuffer.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/reallocarray.h"
#include "util/u_inlines.h"
#include "util/format/u_format.h"
#include "lp_scene.h"
#include "lp_fence.h"
#include "lp_debug.h"
#include "lp_context.h"
#include "lp_state_fs.h"
#include "lp_setup_context.h"


#define RESOURCE_REF_SZ 32
/** List of resource references */
struct resource_ref {
   struct pipe_resource *resource[RESOURCE_REF_SZ];
   int count;
   struct resource_ref *next;
};


#define SHADER_REF_SZ 32
/** List of shader variant references */
struct shader_ref {
   struct lp_fragment_shader_variant *variant[SHADER_REF_SZ];
   int count;
   struct shader_ref *next;
};


/**
 * Create a new scene object.
 * \param queue  the queue to put newly rendered/emptied scenes into
 */
struct lp_scene *
lp_scene_create(struct lp_setup_context *setup)
{
   struct lp_scene *scene = slab_alloc_st(&setup->scene_slab);
   if (!scene)
      return NULL;

   memset(scene, 0, sizeof(struct lp_scene));
   scene->pipe = setup->pipe;
   scene->setup = setup;
   scene->data.head = &scene->data.first;

   (void) mtx_init(&scene->mutex, mtx_plain);

#ifdef DEBUG
   /* Do some scene limit sanity checks here */
   {
      size_t maxBins = TILES_X * TILES_Y;
      size_t maxCommandBytes = sizeof(struct cmd_block) * maxBins;
      size_t maxCommandPlusData = maxCommandBytes + DATA_BLOCK_SIZE;
      /* We'll need at least one command block per bin.  Make sure that's
       * less than the max allowed scene size.
       */
      assert(maxCommandBytes < LP_SCENE_MAX_SIZE);
      /* We'll also need space for at least one other data block */
      assert(maxCommandPlusData <= LP_SCENE_MAX_SIZE);
   }
#endif

   return scene;
}


/**
 * Free all data associated with the given scene, and the scene itself.
 */
void
lp_scene_destroy(struct lp_scene *scene)
{
   lp_scene_end_rasterization(scene);
   mtx_destroy(&scene->mutex);
   free(scene->tiles);
   assert(scene->data.head == &scene->data.first);
   slab_free_st(&scene->setup->scene_slab, scene);
}


/**
 * Check if the scene's bins are all empty.
 * For debugging purposes.
 */
bool
lp_scene_is_empty(struct lp_scene *scene)
{
   for (unsigned y = 0; y < scene->tiles_y; y++) {
      for (unsigned x = 0; x < scene->tiles_x; x++) {
         const struct cmd_bin *bin = lp_scene_get_bin(scene, x, y);
         if (bin->head) {
            return false;
         }
      }
   }
   return true;
}


/* Returns true if there has ever been a failed allocation attempt in
 * this scene.  Used in triangle/rectangle emit to avoid having to
 * check success at each bin.
 */
bool
lp_scene_is_oom(struct lp_scene *scene)
{
   return scene->alloc_failed;
}


/* Remove all commands from a bin.  Tries to reuse some of the memory
 * allocated to the bin, however.
 */
void
lp_scene_bin_reset(struct lp_scene *scene, unsigned x, unsigned y)
{
   struct cmd_bin *bin = lp_scene_get_bin(scene, x, y);

   bin->last_state = NULL;
   bin->head = bin->tail;
   if (bin->tail) {
      bin->tail->next = NULL;
      bin->tail->count = 0;
   }
}


static void
init_scene_texture(struct lp_scene_surface *ssurf, struct pipe_surface *psurf)
{
   if (!psurf) {
      ssurf->stride = 0;
      ssurf->layer_stride = 0;
      ssurf->sample_stride = 0;
      ssurf->nr_samples = 0;
      ssurf->map = NULL;
      return;
   }

   if (llvmpipe_resource_is_texture(psurf->texture)) {
      ssurf->stride = llvmpipe_resource_stride(psurf->texture,
                                               psurf->u.tex.level);
      ssurf->layer_stride = llvmpipe_layer_stride(psurf->texture,
                                                           psurf->u.tex.level);
      ssurf->sample_stride = llvmpipe_sample_stride(psurf->texture);

      ssurf->map = llvmpipe_resource_map(psurf->texture,
                                         psurf->u.tex.level,
                                         psurf->u.tex.first_layer,
                                         LP_TEX_USAGE_READ_WRITE);
      ssurf->format_bytes = util_format_get_blocksize(psurf->format);
      ssurf->nr_samples = util_res_sample_count(psurf->texture);
   } else {
      struct llvmpipe_resource *lpr = llvmpipe_resource(psurf->texture);
      unsigned pixstride = util_format_get_blocksize(psurf->format);
      ssurf->stride = psurf->texture->width0;
      ssurf->layer_stride = 0;
      ssurf->sample_stride = 0;
      ssurf->nr_samples = 1;
      ssurf->map = lpr->data;
      ssurf->map += psurf->u.buf.first_element * pixstride;
      ssurf->format_bytes = util_format_get_blocksize(psurf->format);
   }
}


void
lp_scene_begin_rasterization(struct lp_scene *scene)
{
   const struct pipe_framebuffer_state *fb = &scene->fb;

   //LP_DBG(DEBUG_RAST, "%s\n", __func__);

   for (unsigned i = 0; i < scene->fb.nr_cbufs; i++) {
      struct pipe_surface *cbuf = scene->fb.cbufs[i];
      init_scene_texture(&scene->cbufs[i], cbuf);
   }

   if (fb->zsbuf) {
      struct pipe_surface *zsbuf = scene->fb.zsbuf;
      init_scene_texture(&scene->zsbuf, zsbuf);
   }
}


/**
 * Free all the temporary data in a scene.
 */
void
lp_scene_end_rasterization(struct lp_scene *scene)
{
   mtx_lock(&scene->mutex);

   /* Unmap color buffers */
   for (unsigned i = 0; i < scene->fb.nr_cbufs; i++) {
      if (scene->cbufs[i].map) {
         struct pipe_surface *cbuf = scene->fb.cbufs[i];
         if (llvmpipe_resource_is_texture(cbuf->texture)) {
            llvmpipe_resource_unmap(cbuf->texture,
                                    cbuf->u.tex.level,
                                    cbuf->u.tex.first_layer);
         }
         scene->cbufs[i].map = NULL;
      }
   }

   /* Unmap z/stencil buffer */
   if (scene->zsbuf.map) {
      struct pipe_surface *zsbuf = scene->fb.zsbuf;
      llvmpipe_resource_unmap(zsbuf->texture,
                              zsbuf->u.tex.level,
                              zsbuf->u.tex.first_layer);
      scene->zsbuf.map = NULL;
   }

   /* Reset all command lists:
    */
   memset(scene->tiles, 0, sizeof(struct cmd_bin) * scene->num_alloced_tiles);

   /* Decrement texture ref counts
    */
   int j = 0;
   for (struct resource_ref *ref = scene->resources; ref; ref = ref->next) {
      for (int i = 0; i < ref->count; i++) {
         if (LP_DEBUG & DEBUG_SETUP)
            debug_printf("resource %d: %p %dx%d sz %d\n",
                         j,
                         (void *) ref->resource[i],
                         ref->resource[i]->width0,
                         ref->resource[i]->height0,
                         llvmpipe_resource_size(ref->resource[i]));
         j++;
         llvmpipe_resource_unmap(ref->resource[i], 0, 0);
         pipe_resource_reference(&ref->resource[i], NULL);
      }
   }

   for (struct resource_ref *ref = scene->writeable_resources; ref;
        ref = ref->next) {
      for (int i = 0; i < ref->count; i++) {
         if (LP_DEBUG & DEBUG_SETUP)
            debug_printf("resource %d: %p %dx%d sz %d\n",
                         j,
                         (void *) ref->resource[i],
                         ref->resource[i]->width0,
                            ref->resource[i]->height0,
                         llvmpipe_resource_size(ref->resource[i]));
         j++;
         llvmpipe_resource_unmap(ref->resource[i], 0, 0);
         pipe_resource_reference(&ref->resource[i], NULL);
      }
   }

   if (LP_DEBUG & DEBUG_SETUP) {
      debug_printf("scene %d resources, sz %d\n",
                   j, scene->resource_reference_size);
   }

   /* Decrement shader variant ref counts
    */
   j = 0;
   for (struct shader_ref *ref = scene->frag_shaders; ref; ref = ref->next) {
      for (int i = 0; i < ref->count; i++) {
         if (LP_DEBUG & DEBUG_SETUP)
            debug_printf("shader %d: %p\n", j, (void *) ref->variant[i]);
         j++;
         lp_fs_variant_reference(llvmpipe_context(scene->pipe),
                                 &ref->variant[i], NULL);
      }
   }

   /* Free all scene data blocks:
    */
   {
      struct data_block_list *list = &scene->data;
      struct data_block *block, *tmp;

      for (block = list->head; block; block = tmp) {
         tmp = block->next;
         if (block != &list->first)
            FREE(block);
      }

      list->head = &list->first;
      list->head->next = NULL;
   }

   lp_fence_reference(&scene->fence, NULL);

   scene->resources = NULL;
   scene->writeable_resources = NULL;
   scene->frag_shaders = NULL;
   scene->scene_size = 0;
   scene->resource_reference_size = 0;

   scene->alloc_failed = false;

   util_unreference_framebuffer_state(&scene->fb);

   mtx_unlock(&scene->mutex);
}


struct cmd_block *
lp_scene_new_cmd_block(struct lp_scene *scene,
                       struct cmd_bin *bin)
{
   struct cmd_block *block = lp_scene_alloc(scene, sizeof(struct cmd_block));
   if (block) {
      if (bin->tail) {
         bin->tail->next = block;
         bin->tail = block;
      } else {
         bin->head = block;
         bin->tail = block;
      }
      //memset(block, 0, sizeof *block);
      block->next = NULL;
      block->count = 0;
   }
   return block;
}


struct data_block *
lp_scene_new_data_block(struct lp_scene *scene)
{
   if (scene->scene_size + DATA_BLOCK_SIZE > LP_SCENE_MAX_SIZE) {
      if (0) debug_printf("%s: failed\n", __func__);
      scene->alloc_failed = true;
      return NULL;
   } else {
      struct data_block *block = MALLOC_STRUCT(data_block);
      if (!block)
         return NULL;

      scene->scene_size += sizeof *block;

      block->used = 0;
      block->next = scene->data.head;
      scene->data.head = block;

      return block;
   }
}


/**
 * Return number of bytes used for all bin data within a scene.
 * This does not include resources (textures) referenced by the scene.
 */
static unsigned
lp_scene_data_size(const struct lp_scene *scene)
{
   unsigned size = 0;
   const struct data_block *block;
   for (block = scene->data.head; block; block = block->next) {
      size += block->used;
   }
   return size;
}



/**
 * Add a reference to a resource by the scene.
 */
bool
lp_scene_add_resource_reference(struct lp_scene *scene,
                                struct pipe_resource *resource,
                                bool initializing_scene,
                                bool writeable)
{
   struct resource_ref *ref;
   int i;
   struct resource_ref **list = writeable ? &scene->writeable_resources : &scene->resources;
   struct resource_ref **last = list;

   mtx_lock(&scene->mutex);

   /* Look at existing resource blocks:
    */
   for (ref = *list; ref; ref = ref->next) {
      last = &ref->next;

      /* Search for this resource:
       */
      for (i = 0; i < ref->count; i++)
         if (ref->resource[i] == resource) {
            mtx_unlock(&scene->mutex);
            return true;
      }

      if (ref->count < RESOURCE_REF_SZ) {
         /* If the block is half-empty, then append the reference here.
          */
         break;
      }
   }

   /* Create a new block if no half-empty block was found.
    */
   if (!ref) {
      assert(*last == NULL);
      *last = lp_scene_alloc(scene, sizeof *ref);
      if (*last == NULL) {
          mtx_unlock(&scene->mutex);
          return false;
      }

      ref = *last;
      memset(ref, 0, sizeof *ref);
   }

   /* Map resource again to increment the map count. We likely use the
    * already-mapped pointer in a texture of the jit context, and that pointer
    * needs to stay mapped during rasterization. This map is unmap'ed when
    * finalizing scene rasterization. */
   llvmpipe_resource_map(resource, 0, 0, LP_TEX_USAGE_READ);

   /* Append the reference to the reference block.
    */
   pipe_resource_reference(&ref->resource[ref->count++], resource);
   scene->resource_reference_size += llvmpipe_resource_size(resource);

   /* Heuristic to advise scene flushes.  This isn't helpful in the
    * initial setup of the scene, but after that point flush on the
    * next resource added which exceeds 64MB in referenced texture
    * data.
    */
   int flush = (initializing_scene || scene->resource_reference_size < LP_SCENE_MAX_RESOURCE_SIZE);
   mtx_unlock(&scene->mutex);
   return flush;
}

/**
 * Add a reference to a fragment shader variant
 * Return FALSE if out of memory, TRUE otherwise.
 */
bool
lp_scene_add_frag_shader_reference(struct lp_scene *scene,
                                   struct lp_fragment_shader_variant *variant)
{
   struct shader_ref *ref, **last = &scene->frag_shaders;

   /* Look at existing resource blocks:
    */
   for (ref = scene->frag_shaders; ref; ref = ref->next) {
      last = &ref->next;

      /* Search for this resource:
       */
      for (int i = 0; i < ref->count; i++)
         if (ref->variant[i] == variant)
            return true;

      if (ref->count < SHADER_REF_SZ) {
         /* If the block is half-empty, then append the reference here.
          */
         break;
      }
   }

   /* Create a new block if no half-empty block was found.
    */
   if (!ref) {
      assert(*last == NULL);
      *last = lp_scene_alloc(scene, sizeof *ref);
      if (*last == NULL)
          return false;

      ref = *last;
      memset(ref, 0, sizeof *ref);
   }

   /* Append the reference to the reference block.
    */
   lp_fs_variant_reference(llvmpipe_context(scene->pipe),
                           &ref->variant[ref->count++], variant);

   return true;
}


/**
 * Does this scene have a reference to the given resource?
 * Returns bitmask of LP_REFERENCED_FOR_READ/WRITE bits.
 */
unsigned
lp_scene_is_resource_referenced(const struct lp_scene *scene,
                                const struct pipe_resource *resource)
{
   const struct resource_ref *ref;

   /* check the render targets */
   for (unsigned j = 0; j < scene->fb.nr_cbufs; j++) {
     if (scene->fb.cbufs[j] && scene->fb.cbufs[j]->texture == resource)
       return LP_REFERENCED_FOR_READ | LP_REFERENCED_FOR_WRITE;
   }
   if (scene->fb.zsbuf && scene->fb.zsbuf->texture == resource) {
     return LP_REFERENCED_FOR_READ | LP_REFERENCED_FOR_WRITE;
   }

   for (ref = scene->resources; ref; ref = ref->next) {
      for (int i = 0; i < ref->count; i++)
         if (ref->resource[i] == resource)
            return LP_REFERENCED_FOR_READ;
   }

   for (ref = scene->writeable_resources; ref; ref = ref->next) {
      for (int i = 0; i < ref->count; i++)
         if (ref->resource[i] == resource)
            return LP_REFERENCED_FOR_READ | LP_REFERENCED_FOR_WRITE;
   }

   return 0;
}


/** advance curr_x,y to the next bin */
static bool
next_bin(struct lp_scene *scene)
{
   scene->curr_x++;
   if (scene->curr_x >= scene->tiles_x) {
      scene->curr_x = 0;
      scene->curr_y++;
   }
   if (scene->curr_y >= scene->tiles_y) {
      /* no more bins */
      return false;
   }
   return true;
}


void
lp_scene_bin_iter_begin(struct lp_scene *scene)
{
   scene->curr_x = scene->curr_y = -1;
}


/**
 * Return pointer to next bin to be rendered.
 * The lp_scene::curr_x and ::curr_y fields will be advanced.
 * Multiple rendering threads will call this function to get a chunk
 * of work (a bin) to work on.
 */
struct cmd_bin *
lp_scene_bin_iter_next(struct lp_scene *scene , int *x, int *y)
{
   struct cmd_bin *bin = NULL;

   mtx_lock(&scene->mutex);

   if (scene->curr_x < 0) {
      /* first bin */
      scene->curr_x = 0;
      scene->curr_y = 0;
   } else if (!next_bin(scene)) {
      /* no more bins left */
      goto end;
   }

   bin = lp_scene_get_bin(scene, scene->curr_x, scene->curr_y);
   *x = scene->curr_x;
   *y = scene->curr_y;

end:
   /*printf("return bin %p at %d, %d\n", (void *) bin, *bin_x, *bin_y);*/
   mtx_unlock(&scene->mutex);
   return bin;
}


void
lp_scene_begin_binning(struct lp_scene *scene,
                       struct pipe_framebuffer_state *fb)
{
   assert(lp_scene_is_empty(scene));

   util_copy_framebuffer_state(&scene->fb, fb);

   scene->tiles_x = align(fb->width, TILE_SIZE) / TILE_SIZE;
   scene->tiles_y = align(fb->height, TILE_SIZE) / TILE_SIZE;
   assert(scene->tiles_x <= TILES_X);
   assert(scene->tiles_y <= TILES_Y);

   unsigned num_required_tiles = scene->tiles_x * scene->tiles_y;
   if (scene->num_alloced_tiles < num_required_tiles) {
      scene->tiles = reallocarray(scene->tiles, num_required_tiles,
                                  sizeof(struct cmd_bin));
      if (!scene->tiles)
         return;
      memset(scene->tiles, 0, sizeof(struct cmd_bin) * num_required_tiles);
      scene->num_alloced_tiles = num_required_tiles;
   }

   /*
    * Determine how many layers the fb has (used for clamping layer value).
    * OpenGL (but not d3d10) permits different amount of layers per rt,
    * however results are undefined if layer exceeds the amount of layers of
    * ANY attachment hence don't need separate per cbuf and zsbuf max.
    */
   unsigned max_layer = ~0;
   for (unsigned i = 0; i < scene->fb.nr_cbufs; i++) {
      struct pipe_surface *cbuf = scene->fb.cbufs[i];
      if (cbuf) {
         if (llvmpipe_resource_is_texture(cbuf->texture)) {
            max_layer = MIN2(max_layer,
                             cbuf->u.tex.last_layer - cbuf->u.tex.first_layer);
         } else {
            max_layer = 0;
         }
      }
   }

   if (fb->zsbuf) {
      struct pipe_surface *zsbuf = scene->fb.zsbuf;
      max_layer = MIN2(max_layer, zsbuf->u.tex.last_layer - zsbuf->u.tex.first_layer);
   }

   scene->fb_max_layer = max_layer;
   scene->fb_max_samples = util_framebuffer_get_num_samples(fb);
   if (scene->fb_max_samples == 4) {
      for (unsigned i = 0; i < 4; i++) {
         scene->fixed_sample_pos[i][0] = util_iround(lp_sample_pos_4x[i][0] * FIXED_ONE);
         scene->fixed_sample_pos[i][1] = util_iround(lp_sample_pos_4x[i][1] * FIXED_ONE);
      }
   }
}


void
lp_scene_end_binning(struct lp_scene *scene)
{
   if (LP_DEBUG & DEBUG_SCENE) {
      debug_printf("rasterize scene:\n");
      debug_printf("  scene_size: %u\n",
                   scene->scene_size);
      debug_printf("  data size: %u\n",
                   lp_scene_data_size(scene));

      if (0)
         lp_debug_bins(scene);
   }
}
