/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
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
 * Polygon stipple stage:  implement polygon stipple with texture map and
 * fragment program.  The fragment program samples the texture using the
 * fragment window coordinate register and does a fragment kill for the
 * stipple-failing fragments.
 *
 * Authors:  Brian Paul
 */


#include "pipe/p_context.h"
#include "pipe/p_defines.h"
#include "pipe/p_shader_tokens.h"
#include "util/u_inlines.h"

#include "util/format/u_format.h"
#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/u_pstipple.h"
#include "util/u_sampler.h"

#include "tgsi/tgsi_parse.h"

#include "draw_context.h"
#include "draw_pipe.h"

#include "nir.h"
#include "nir/nir_draw_helpers.h"

/** Approx number of new tokens for instructions in pstip_transform_inst() */
#define NUM_NEW_TOKENS 53


/**
 * Subclass of pipe_shader_state to carry extra fragment shader info.
 */
struct pstip_fragment_shader
{
   struct pipe_shader_state state;
   void *driver_fs;
   void *pstip_fs;
   unsigned sampler_unit;
};


/**
 * Subclass of draw_stage
 */
struct pstip_stage
{
   struct draw_stage stage;

   void *sampler_cso;
   struct pipe_resource *texture;
   struct pipe_sampler_view *sampler_view;
   unsigned num_samplers;
   unsigned num_sampler_views;

   /*
    * Currently bound state
    */
   struct pstip_fragment_shader *fs;
   struct {
      void *samplers[PIPE_MAX_SAMPLERS];
      struct pipe_sampler_view *sampler_views[PIPE_MAX_SHADER_SAMPLER_VIEWS];
      const struct pipe_poly_stipple *stipple;
   } state;

   /*
    * Driver interface/override functions
    */
   void * (*driver_create_fs_state)(struct pipe_context *,
                                    const struct pipe_shader_state *);
   void (*driver_bind_fs_state)(struct pipe_context *, void *);
   void (*driver_delete_fs_state)(struct pipe_context *, void *);

   void (*driver_bind_sampler_states)(struct pipe_context *,
                                      enum pipe_shader_type,
                                      unsigned, unsigned, void **);

   void (*driver_set_sampler_views)(struct pipe_context *,
                                    enum pipe_shader_type shader,
                                    unsigned start, unsigned count,
                                    unsigned unbind_num_trailing_slots,
                                    bool take_ownership,
                                    struct pipe_sampler_view **);

   void (*driver_set_polygon_stipple)(struct pipe_context *,
                                      const struct pipe_poly_stipple *);

   struct pipe_context *pipe;
};


/**
 * Generate the frag shader we'll use for doing polygon stipple.
 * This will be the user's shader prefixed with a TEX and KIL instruction.
 */
static bool
generate_pstip_fs(struct pstip_stage *pstip)
{
   struct pipe_context *pipe = pstip->pipe;
   struct pipe_screen *screen = pipe->screen;
   const struct pipe_shader_state *orig_fs = &pstip->fs->state;
   /*struct draw_context *draw = pstip->stage.draw;*/
   struct pipe_shader_state pstip_fs;
   enum tgsi_file_type wincoord_file;

   wincoord_file = screen->get_param(screen, PIPE_CAP_FS_POSITION_IS_SYSVAL) ?
                   TGSI_FILE_SYSTEM_VALUE : TGSI_FILE_INPUT;

   pstip_fs = *orig_fs; /* copy to init */
   if (orig_fs->type == PIPE_SHADER_IR_TGSI) {
      pstip_fs.tokens = util_pstipple_create_fragment_shader(orig_fs->tokens,
                                                             &pstip->fs->sampler_unit,
                                                             0,
                                                             wincoord_file);
      if (pstip_fs.tokens == NULL)
         return false;
   } else {
      pstip_fs.ir.nir = nir_shader_clone(NULL, orig_fs->ir.nir);
      nir_lower_pstipple_fs(pstip_fs.ir.nir,
                            &pstip->fs->sampler_unit, 0, wincoord_file == TGSI_FILE_SYSTEM_VALUE,
                            nir_type_bool32);
   }

   assert(pstip->fs->sampler_unit < PIPE_MAX_SAMPLERS);

   pstip->fs->pstip_fs = pstip->driver_create_fs_state(pipe, &pstip_fs);

   FREE((void *)pstip_fs.tokens);

   if (!pstip->fs->pstip_fs)
      return false;

   return true;
}


/**
 * When we're about to draw our first stipple polygon in a batch, this function
 * is called to tell the driver to bind our modified fragment shader.
 */
static bool
bind_pstip_fragment_shader(struct pstip_stage *pstip)
{
   struct draw_context *draw = pstip->stage.draw;
   if (!pstip->fs->pstip_fs &&
       !generate_pstip_fs(pstip))
      return false;

   draw->suspend_flushing = true;
   pstip->driver_bind_fs_state(pstip->pipe, pstip->fs->pstip_fs);
   draw->suspend_flushing = false;
   return true;
}


static inline struct pstip_stage *
pstip_stage(struct draw_stage *stage)
{
   return (struct pstip_stage *) stage;
}


static void
pstip_first_tri(struct draw_stage *stage, struct prim_header *header)
{
   struct pstip_stage *pstip = pstip_stage(stage);
   struct pipe_context *pipe = pstip->pipe;
   struct draw_context *draw = stage->draw;
   unsigned num_samplers;
   unsigned num_sampler_views;

   assert(stage->draw->rasterizer->poly_stipple_enable);

   /* bind our fragprog */
   if (!bind_pstip_fragment_shader(pstip)) {
      stage->tri = draw_pipe_passthrough_tri;
      stage->tri(stage, header);
      return;
   }

   /* how many samplers? */
   /* we'll use sampler/texture[pstip->sampler_unit] for the stipple */
   num_samplers = MAX2(pstip->num_samplers, pstip->fs->sampler_unit + 1);
   num_sampler_views = MAX2(pstip->num_sampler_views, num_samplers);

   /* plug in our sampler, texture */
   pstip->state.samplers[pstip->fs->sampler_unit] = pstip->sampler_cso;
   pipe_sampler_view_reference(&pstip->state.sampler_views[pstip->fs->sampler_unit],
                               pstip->sampler_view);

   assert(num_samplers <= PIPE_MAX_SAMPLERS);

   draw->suspend_flushing = true;

   pstip->driver_bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0,
                                     num_samplers, pstip->state.samplers);

   pstip->driver_set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0,
                                   num_sampler_views, 0, false,
                                   pstip->state.sampler_views);

   draw->suspend_flushing = false;

   /* now really draw first triangle */
   stage->tri = draw_pipe_passthrough_tri;
   stage->tri(stage, header);
}


static void
pstip_flush(struct draw_stage *stage, unsigned flags)
{
   struct draw_context *draw = stage->draw;
   struct pstip_stage *pstip = pstip_stage(stage);
   struct pipe_context *pipe = pstip->pipe;

   stage->tri = pstip_first_tri;
   stage->next->flush(stage->next, flags);

   /* restore original frag shader, texture, sampler state */
   draw->suspend_flushing = true;
   pstip->driver_bind_fs_state(pipe, pstip->fs ? pstip->fs->driver_fs : NULL);

   pstip->driver_bind_sampler_states(pipe, PIPE_SHADER_FRAGMENT, 0,
                                     pstip->num_samplers,
                                     pstip->state.samplers);

   pstip->driver_set_sampler_views(pipe, PIPE_SHADER_FRAGMENT, 0,
                                   pstip->num_sampler_views, 0, false,
                                   pstip->state.sampler_views);

   draw->suspend_flushing = false;
}


static void
pstip_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
pstip_destroy(struct draw_stage *stage)
{
   struct pstip_stage *pstip = pstip_stage(stage);

   for (unsigned i = 0; i < PIPE_MAX_SHADER_SAMPLER_VIEWS; i++) {
      pipe_sampler_view_reference(&pstip->state.sampler_views[i], NULL);
   }

   pstip->pipe->delete_sampler_state(pstip->pipe, pstip->sampler_cso);

   pipe_resource_reference(&pstip->texture, NULL);

   if (pstip->sampler_view) {
      pipe_sampler_view_reference(&pstip->sampler_view, NULL);
   }

   draw_free_temp_verts(stage);
   FREE(stage);
}


/** Create a new polygon stipple drawing stage object */
static struct pstip_stage *
draw_pstip_stage(struct draw_context *draw, struct pipe_context *pipe)
{
   struct pstip_stage *pstip = CALLOC_STRUCT(pstip_stage);
   if (!pstip)
      goto fail;

   pstip->pipe = pipe;

   pstip->stage.draw = draw;
   pstip->stage.name = "pstip";
   pstip->stage.next = NULL;
   pstip->stage.point = draw_pipe_passthrough_point;
   pstip->stage.line = draw_pipe_passthrough_line;
   pstip->stage.tri = pstip_first_tri;
   pstip->stage.flush = pstip_flush;
   pstip->stage.reset_stipple_counter = pstip_reset_stipple_counter;
   pstip->stage.destroy = pstip_destroy;

   if (!draw_alloc_temp_verts(&pstip->stage, 8))
      goto fail;

   return pstip;

fail:
   if (pstip)
      pstip->stage.destroy(&pstip->stage);

   return NULL;
}


static struct pstip_stage *
pstip_stage_from_pipe(struct pipe_context *pipe)
{
   struct draw_context *draw = (struct draw_context *) pipe->draw;
   return pstip_stage(draw->pipeline.pstipple);
}


/**
 * This function overrides the driver's create_fs_state() function and
 * will typically be called by the gallium frontend.
 */
static void *
pstip_create_fs_state(struct pipe_context *pipe,
                       const struct pipe_shader_state *fs)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);
   struct pstip_fragment_shader *pstipfs = CALLOC_STRUCT(pstip_fragment_shader);

   if (pstipfs) {
      pstipfs->state.type = fs->type;
      if (fs->type == PIPE_SHADER_IR_TGSI)
         pstipfs->state.tokens = tgsi_dup_tokens(fs->tokens);
      else
         pstipfs->state.ir.nir = nir_shader_clone(NULL, fs->ir.nir);

      /* pass-through */
      pstipfs->driver_fs = pstip->driver_create_fs_state(pstip->pipe, fs);
   }

   return pstipfs;
}


static void
pstip_bind_fs_state(struct pipe_context *pipe, void *fs)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);
   struct pstip_fragment_shader *pstipfs = (struct pstip_fragment_shader *) fs;
   /* save current */
   pstip->fs = pstipfs;
   /* pass-through */
   pstip->driver_bind_fs_state(pstip->pipe,
                               (pstipfs ? pstipfs->driver_fs : NULL));
}


static void
pstip_delete_fs_state(struct pipe_context *pipe, void *fs)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);
   struct pstip_fragment_shader *pstipfs = (struct pstip_fragment_shader *) fs;
   /* pass-through */
   pstip->driver_delete_fs_state(pstip->pipe, pstipfs->driver_fs);

   if (pstipfs->pstip_fs)
      pstip->driver_delete_fs_state(pstip->pipe, pstipfs->pstip_fs);

   if (pstipfs->state.type == PIPE_SHADER_IR_TGSI)
      FREE((void*)pstipfs->state.tokens);
   else
      ralloc_free(pstipfs->state.ir.nir);
   FREE(pstipfs);
}


static void
pstip_bind_sampler_states(struct pipe_context *pipe,
                          enum pipe_shader_type shader,
                          unsigned start, unsigned num, void **sampler)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);

   assert(start == 0);

   if (shader == PIPE_SHADER_FRAGMENT) {
      /* save current */
      memcpy(pstip->state.samplers, sampler, num * sizeof(void *));
      for (unsigned i = num; i < PIPE_MAX_SAMPLERS; i++) {
         pstip->state.samplers[i] = NULL;
      }
      pstip->num_samplers = num;
   }

   /* pass-through */
   pstip->driver_bind_sampler_states(pstip->pipe, shader, start, num, sampler);
}


static void
pstip_set_sampler_views(struct pipe_context *pipe,
                        enum pipe_shader_type shader,
                        unsigned start, unsigned num,
                        unsigned unbind_num_trailing_slots,
                        bool take_ownership,
                        struct pipe_sampler_view **views)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);

   if (shader == PIPE_SHADER_FRAGMENT) {
      /* save current */
      unsigned i;
      for (i = 0; i < num; i++) {
         pipe_sampler_view_reference(&pstip->state.sampler_views[start + i],
                                     views[i]);
      }
      for (; i < num + unbind_num_trailing_slots; i++) {
         pipe_sampler_view_reference(&pstip->state.sampler_views[start + i],
                                     NULL);
      }
      pstip->num_sampler_views = num;
   }

   /* pass-through */
   pstip->driver_set_sampler_views(pstip->pipe, shader, start, num,
                                   unbind_num_trailing_slots, take_ownership, views);
}


static void
pstip_set_polygon_stipple(struct pipe_context *pipe,
                          const struct pipe_poly_stipple *stipple)
{
   struct pstip_stage *pstip = pstip_stage_from_pipe(pipe);

   /* save current */
   pstip->state.stipple = stipple;

   /* pass-through */
   pstip->driver_set_polygon_stipple(pstip->pipe, stipple);

   util_pstipple_update_stipple_texture(pstip->pipe, pstip->texture,
                                        pstip->state.stipple->stipple);
}


/**
 * Called by drivers that want to install this polygon stipple stage
 * into the draw module's pipeline.  This will not be used if the
 * hardware has native support for polygon stipple.
 */
bool
draw_install_pstipple_stage(struct draw_context *draw,
                            struct pipe_context *pipe)
{
   pipe->draw = (void *) draw;

   /*
    * Create / install pgon stipple drawing / prim stage
    */
   struct pstip_stage *pstip = draw_pstip_stage(draw, pipe);
   if (!pstip)
      goto fail;

   draw->pipeline.pstipple = &pstip->stage;

   /* save original driver functions */
   pstip->driver_create_fs_state = pipe->create_fs_state;
   pstip->driver_bind_fs_state = pipe->bind_fs_state;
   pstip->driver_delete_fs_state = pipe->delete_fs_state;

   pstip->driver_bind_sampler_states = pipe->bind_sampler_states;
   pstip->driver_set_sampler_views = pipe->set_sampler_views;
   pstip->driver_set_polygon_stipple = pipe->set_polygon_stipple;

   /* create special texture, sampler state */
   pstip->texture = util_pstipple_create_stipple_texture(pipe, NULL);
   if (!pstip->texture)
      goto fail;

   pstip->sampler_view = util_pstipple_create_sampler_view(pipe,
                                                           pstip->texture);
   if (!pstip->sampler_view)
      goto fail;

   pstip->sampler_cso = util_pstipple_create_sampler(pipe);
   if (!pstip->sampler_cso)
      goto fail;

   /* override the driver's functions */
   pipe->create_fs_state = pstip_create_fs_state;
   pipe->bind_fs_state = pstip_bind_fs_state;
   pipe->delete_fs_state = pstip_delete_fs_state;

   pipe->bind_sampler_states = pstip_bind_sampler_states;
   pipe->set_sampler_views = pstip_set_sampler_views;
   pipe->set_polygon_stipple = pstip_set_polygon_stipple;

   return true;

 fail:
   if (pstip)
      pstip->stage.destroy(&pstip->stage);

   return false;
}
