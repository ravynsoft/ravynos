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

/**
 * This module converts provides a more convenient front-end to u_indices,
 * etc, utils to convert primitive types supported not supported by the
 * hardware.  It handles binding new index buffer state, and restoring
 * previous state after.  To use, put something like this at the front of
 * drivers pipe->draw_vbo():
 *
 *    // emulate unsupported primitives:
 *    if (info->mode needs emulating) {
 *       util_primconvert_save_rasterizer_state(ctx->primconvert, ctx->rasterizer);
 *       util_primconvert_draw_vbo(ctx->primconvert, info);
 *       return;
 *    }
 *
 */

#include "pipe/p_state.h"
#include "util/u_draw.h"
#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_prim.h"
#include "util/u_prim_restart.h"
#include "util/u_upload_mgr.h"

#include "indices/u_indices.h"
#include "indices/u_primconvert.h"

struct primconvert_context
{
   struct pipe_context *pipe;
   struct primconvert_config cfg;
   unsigned api_pv;
};


struct primconvert_context *
util_primconvert_create_config(struct pipe_context *pipe,
                               struct primconvert_config *cfg)
{
   struct primconvert_context *pc = CALLOC_STRUCT(primconvert_context);
   if (!pc)
      return NULL;
   pc->pipe = pipe;
   pc->cfg = *cfg;
   return pc;
}

struct primconvert_context *
util_primconvert_create(struct pipe_context *pipe, uint32_t primtypes_mask)
{
   struct primconvert_config cfg = { .primtypes_mask = primtypes_mask, .restart_primtypes_mask = primtypes_mask };
   return util_primconvert_create_config(pipe, &cfg);
}

void
util_primconvert_destroy(struct primconvert_context *pc)
{
   FREE(pc);
}

void
util_primconvert_save_rasterizer_state(struct primconvert_context *pc,
                                       const struct pipe_rasterizer_state
                                       *rast)
{
   util_primconvert_save_flatshade_first(pc, rast->flatshade_first);
}

void
util_primconvert_save_flatshade_first(struct primconvert_context *pc, bool flatshade_first)
{
   /* if we actually translated the provoking vertex for the buffer,
    * we would actually need to save/restore rasterizer state.  As
    * it is, we just need to make note of the pv.
    */
   pc->api_pv = flatshade_first ? PV_FIRST : PV_LAST;
}

static bool
primconvert_init_draw(struct primconvert_context *pc,
                      const struct pipe_draw_info *info,
                      const struct pipe_draw_start_count_bias *draws,
                      struct pipe_draw_info *new_info,
                      struct pipe_draw_start_count_bias *new_draw)
{
   struct pipe_draw_start_count_bias *direct_draws = NULL;
   unsigned num_direct_draws = 0;
   struct pipe_transfer *src_transfer = NULL;
   u_translate_func trans_func, direct_draw_func;
   u_generate_func gen_func;
   const void *src = NULL;
   void *dst;
   unsigned ib_offset;
   unsigned total_index_count = draws->count;
   void *rewrite_buffer = NULL;

   struct pipe_draw_start_count_bias draw = draws[0];

   /* Filter out degenerate primitives, u_upload_alloc() will assert
    * on size==0 so just bail:
    */
   if (!info->primitive_restart &&
       !u_trim_pipe_prim(info->mode, (unsigned*)&draw.count))
      return false;

   util_draw_init_info(new_info);

   /* Because we've changed the index buffer, the original min_index/max_index
    * for the draw are no longer valid. That's ok, but we need to tell drivers
    * so they don't optimize incorrectly.
    */
   new_info->index_bounds_valid = false;
   new_info->min_index = 0;
   new_info->max_index = ~0;

   new_info->start_instance = info->start_instance;
   new_info->instance_count = info->instance_count;
   new_info->primitive_restart = info->primitive_restart;
   new_info->restart_index = info->restart_index;
   if (info->index_size) {
      enum mesa_prim mode = new_info->mode = u_index_prim_type_convert(pc->cfg.primtypes_mask, info->mode, true);
      unsigned index_size = info->index_size;
      unsigned offset = draw.start * info->index_size;

      new_info->index_size = u_index_size_convert(info->index_size);

      src = info->has_user_indices ? info->index.user : NULL;
      if (!src) {
         /* Map the index range we're interested in (not the whole buffer) */
         src = pipe_buffer_map_range(pc->pipe, info->index.resource,
                                     offset,
                                     draw.count * info->index_size,
                                     PIPE_MAP_READ, &src_transfer);
         offset = 0;
         draw.start = 0;
      }
      const void *restart_src = (const uint8_t *)src  + offset;

      /* if the resulting primitive type is not supported by the driver for primitive restart,
       * or if the original primitive type was not supported by the driver,
       * the draw needs to be rewritten to not use primitive restart
       */
      if (info->primitive_restart &&
          (!(pc->cfg.restart_primtypes_mask & BITFIELD_BIT(mode)) ||
           !(pc->cfg.primtypes_mask & BITFIELD_BIT(info->mode)))) {
         /* step 1: rewrite draw to not use primitive primitive restart;
          *         this pre-filters degenerate primitives
          */
         direct_draws = util_prim_restart_convert_to_direct(restart_src, info, &draw, &num_direct_draws,
                                                            &new_info->min_index, &new_info->max_index, &total_index_count);
         new_info->primitive_restart = false;
         /* step 2: get a translator function which does nothing but handle any index size conversions
          * which may or may not occur (8bit -> 16bit)
          */
         u_index_translator(0xffff,
                            info->mode, index_size, total_index_count,
                            pc->api_pv, pc->api_pv,
                            PR_DISABLE,
                            &mode, &index_size, &new_draw->count,
                            &direct_draw_func);
         /* this should always be a direct translation */
         assert(new_draw->count == total_index_count);
         /* step 3: allocate a temp buffer for an intermediate rewrite step
          *         if no indices were found, this was a single incomplete restart and can be discarded
          */
         if (total_index_count)
            rewrite_buffer = malloc(index_size * total_index_count);
         if (!rewrite_buffer) {
            if (src_transfer)
               pipe_buffer_unmap(pc->pipe, src_transfer);
            return false;
         }
      }
      /* (step 4: get the actual primitive conversion translator function) */
      u_index_translator(pc->cfg.primtypes_mask,
                         info->mode, index_size, total_index_count,
                         pc->api_pv, pc->api_pv,
                         new_info->primitive_restart ? PR_ENABLE : PR_DISABLE,
                         &mode, &index_size, &new_draw->count,
                         &trans_func);
      assert(new_info->mode == mode);
      assert(new_info->index_size == index_size);
   }
   else {
      enum mesa_prim mode = 0;
      unsigned index_size;

      u_index_generator(pc->cfg.primtypes_mask,
                        info->mode, draw.start, draw.count,
                        pc->api_pv, pc->api_pv,
                        &mode, &index_size, &new_draw->count,
                        &gen_func);
      new_info->mode = mode;
      new_info->index_size = index_size;
   }

   /* (step 5: allocate gpu memory sized for the FINAL index count) */
   u_upload_alloc(pc->pipe->stream_uploader, 0, new_info->index_size * new_draw->count, 4,
                  &ib_offset, &new_info->index.resource, &dst);
   new_draw->start = ib_offset / new_info->index_size;
   new_draw->index_bias = info->index_size ? draw.index_bias : 0;

   if (info->index_size) {
      if (num_direct_draws) {
         uint8_t *ptr = rewrite_buffer;
         uint8_t *dst_ptr = dst;
         /* step 6: if rewriting a prim-restart draw to direct draws,
          * loop over all the direct draws in order to rewrite them into a single index buffer
          * and draw in order to match the original call
          */
         for (unsigned i = 0; i < num_direct_draws; i++) {
            /* step 6a: get the index count for this draw, once converted */
            unsigned tmp_count = u_index_count_converted_indices(pc->cfg.primtypes_mask, true, info->mode, direct_draws[i].count);
            /* step 6b: handle index size conversion using the temp buffer; no change in index count
             * TODO: this step can be optimized out if the index size is known to not change
             */
            direct_draw_func(src, direct_draws[i].start, direct_draws[i].count, direct_draws[i].count, info->restart_index, ptr);
            /* step 6c: handle the primitive type conversion rewriting to the converted index count */
            trans_func(ptr, 0, direct_draws[i].count, tmp_count, info->restart_index, dst_ptr);
            /* step 6d: increment the temp buffer and mapped final index buffer pointers */
            ptr += new_info->index_size * direct_draws[i].count;
            dst_ptr += new_info->index_size * tmp_count;
         }
         /* step 7: set the final index count, which is the converted total index count from the original draw rewrite */
         new_draw->count = u_index_count_converted_indices(pc->cfg.primtypes_mask, true, info->mode, total_index_count);
      } else
         trans_func(src, draw.start, draw.count, new_draw->count, info->restart_index, dst);

      if (pc->cfg.fixed_prim_restart && new_info->primitive_restart) {
         new_info->restart_index = (1ull << (new_info->index_size * 8)) - 1;
         if (info->restart_index != new_info->restart_index)
            util_translate_prim_restart_data(new_info->index_size, dst, dst,
                                             new_draw->count,
                                             info->restart_index);
      }
   }
   else {
      gen_func(draw.start, new_draw->count, dst);
   }
   new_info->was_line_loop = info->mode == MESA_PRIM_LINE_LOOP;

   if (src_transfer)
      pipe_buffer_unmap(pc->pipe, src_transfer);

   u_upload_unmap(pc->pipe->stream_uploader);

   free(direct_draws);
   free(rewrite_buffer);
   return true;
}

static void
util_primconvert_draw_single_vbo(struct primconvert_context *pc,
                                 const struct pipe_draw_info *info,
                                 unsigned drawid_offset,
                                 const struct pipe_draw_start_count_bias *draw)
{
   struct pipe_draw_info new_info;
   struct pipe_draw_start_count_bias new_draw;

   if (!primconvert_init_draw(pc, info, draw, &new_info, &new_draw))
      return;
   /* to the translated draw: */
   pc->pipe->draw_vbo(pc->pipe, &new_info, drawid_offset, NULL, &new_draw, 1);

   pipe_resource_reference(&new_info.index.resource, NULL);
}

void
util_primconvert_draw_vbo(struct primconvert_context *pc,
                          const struct pipe_draw_info *info,
                          unsigned drawid_offset,
                          const struct pipe_draw_indirect_info *indirect,
                          const struct pipe_draw_start_count_bias *draws,
                          unsigned num_draws)
{
   if (indirect && indirect->buffer) {
      /* this is stupid, but we're already doing a readback,
       * so this thing may as well get the rest of the job done
       */
      unsigned draw_count = 0;
      struct u_indirect_params *new_draws = util_draw_indirect_read(pc->pipe, info, indirect, &draw_count);
      if (!new_draws)
         goto cleanup;

      for (unsigned i = 0; i < draw_count; i++)
         util_primconvert_draw_single_vbo(pc, &new_draws[i].info, drawid_offset + i, &new_draws[i].draw);
      free(new_draws);
   } else {
      unsigned drawid = drawid_offset;
      for (unsigned i = 0; i < num_draws; i++) {
         if (draws[i].count && info->instance_count)
            util_primconvert_draw_single_vbo(pc, info, drawid, &draws[i]);
         if (info->increment_draw_id)
            drawid++;
      }
   }

cleanup:
   if (info->take_index_buffer_ownership) {
      struct pipe_resource *buffer = info->index.resource;
      pipe_resource_reference(&buffer, NULL);
   }
}

void
util_primconvert_draw_vertex_state(struct primconvert_context *pc,
                                   struct pipe_vertex_state *vstate,
                                   uint32_t partial_velem_mask,
                                   struct pipe_draw_vertex_state_info info,
                                   const struct pipe_draw_start_count_bias *draws,
                                   unsigned num_draws)
{
   struct pipe_draw_info new_info;
   struct pipe_draw_start_count_bias new_draw;

   if (pc->cfg.primtypes_mask & BITFIELD_BIT(info.mode)) {
      pc->pipe->draw_vertex_state(pc->pipe, vstate, partial_velem_mask, info, draws, num_draws);
      return;
   }

   if (num_draws > 1) {
      for (unsigned i = 0; i < num_draws; i++) {
         if (draws[i].count)
            util_primconvert_draw_vertex_state(pc, vstate, partial_velem_mask, info, &draws[i], 1);
      }
      return;
   }

   struct pipe_draw_info dinfo = {0};
   dinfo.mode = info.mode;
   dinfo.index_size = 4;
   dinfo.instance_count = 1;
   dinfo.index.resource = vstate->input.indexbuf;
   if (!primconvert_init_draw(pc, &dinfo, draws, &new_info, &new_draw))
      return;

   struct pipe_vertex_state *new_state = pc->pipe->screen->create_vertex_state(pc->pipe->screen,
                                                                               &vstate->input.vbuffer,
                                                                               vstate->input.elements,
                                                                               vstate->input.num_elements,
                                                                               new_info.index.resource,
                                                                               vstate->input.full_velem_mask);
   if (new_state) {
      struct pipe_draw_vertex_state_info new_vinfo;
      new_vinfo.mode = new_info.mode;
      new_vinfo.take_vertex_state_ownership = true;
      /* to the translated draw: */
      pc->pipe->draw_vertex_state(pc->pipe, new_state, partial_velem_mask, new_vinfo, &new_draw, 1);
   }
   if (info.take_vertex_state_ownership)
      pipe_vertex_state_reference(&vstate, NULL);

   pipe_resource_reference(&new_info.index.resource, NULL);
}
