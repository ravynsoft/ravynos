/**********************************************************
 * Copyright 2014-2022 VMware, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **********************************************************/

#include "util/u_inlines.h"
#include "util/u_memory.h"
#include "util/u_bitmask.h"
#include "util/u_simple_shaders.h"
#include "tgsi/tgsi_point_sprite.h"
#include "tgsi/tgsi_dynamic_indexing.h"
#include "tgsi/tgsi_vpos.h"
#include "tgsi/tgsi_dump.h"

#include "svga_context.h"
#include "svga_shader.h"
#include "svga_tgsi.h"


/**
 * Bind a new GS.  This updates the derived current gs state, not the
 * user-specified GS state.
 */
static void
bind_gs_state(struct svga_context *svga,
              struct svga_geometry_shader *gs)
{
   svga->curr.gs = gs;
   svga->dirty |= SVGA_NEW_GS;
}


static void
insert_at_head(struct svga_shader *head, struct svga_shader *shader)
{
   shader->parent = head;
   shader->next = head->next;
   head->next = shader;
}


/**
 * Bind shader
 */
static void
bind_shader(struct svga_context *svga,
            const enum pipe_shader_type shader_type,
            struct svga_shader *shader)
{
   switch (shader_type) {
   case PIPE_SHADER_VERTEX:
      svga->pipe.bind_vs_state(&svga->pipe, shader);
      break;
   case PIPE_SHADER_FRAGMENT:
      /**
       * Avoid pipe->bind_fs_state call because it goes through aapoint
       * layer. We loose linked list of all transformed shaders if aapoint
       * is used.
       */
      svga_bind_fs_state(&svga->pipe, shader);
      break;
   case PIPE_SHADER_GEOMETRY:
      svga->pipe.bind_gs_state(&svga->pipe, shader);
      break;
   case PIPE_SHADER_TESS_CTRL:
      svga->pipe.bind_tcs_state(&svga->pipe, shader);
      break;
   case PIPE_SHADER_TESS_EVAL:
      svga->pipe.bind_tes_state(&svga->pipe, shader);
      break;
   default:
      return;
   }
}



/**
 * Create shader
 */
static void *
create_shader(struct svga_context *svga,
              const enum pipe_shader_type shader_type,
              struct pipe_shader_state *state)
{
   switch (shader_type) {
   case PIPE_SHADER_VERTEX:
      return svga->pipe.create_vs_state(&svga->pipe, state);
   case PIPE_SHADER_FRAGMENT:
      /**
       * Avoid pipe->create_fs_state call because it goes through aapoint
       * layer. We loose linked list of all transformed shaders if aapoint
       * is used.
       */
      return svga_create_fs_state(&svga->pipe, state);
   case PIPE_SHADER_GEOMETRY:
      return svga->pipe.create_gs_state(&svga->pipe, state);
   case PIPE_SHADER_TESS_CTRL:
      return svga->pipe.create_tcs_state(&svga->pipe, state);
   case PIPE_SHADER_TESS_EVAL:
      return svga->pipe.create_tes_state(&svga->pipe, state);
   default:
      return NULL;
   }
}


static void
write_vpos(struct svga_context *svga,
           struct svga_shader *shader)
{
   struct svga_token_key key;
   bool use_existing = false;
   struct svga_shader *transform_shader;
   const struct tgsi_shader_info *info = &shader->tgsi_info;

   /* Create a token key */
   memset(&key, 0, sizeof key);
   key.vs.write_position = 1;

   if (shader->next) {
      transform_shader = svga_search_shader_token_key(shader->next, &key);
      if (transform_shader) {
         use_existing = true;
      }
   }

   if (!use_existing) {
      struct pipe_shader_state state = {0};
      struct tgsi_token *new_tokens = NULL;

      new_tokens = tgsi_write_vpos(shader->tokens,
                                   info->immediate_count);
      if (!new_tokens)
         return;

      pipe_shader_state_from_tgsi(&state, new_tokens);

      transform_shader = create_shader(svga, info->processor, &state);
      insert_at_head(shader, transform_shader);
      FREE(new_tokens);
   }
   transform_shader->token_key = key;
   bind_shader(svga, info->processor, transform_shader);
}


/**
 * transform_dynamic_indexing searches shader variant list to see if
 * we have transformed shader for dynamic indexing and reuse/bind it. If we
 * don't have transformed shader, then it will create new shader from which
 * dynamic indexing will be removed. It will also be added to the shader
 * variant list and this new shader will be bind to current svga state.
 */
static void
transform_dynamic_indexing(struct svga_context *svga,
                           struct svga_shader *shader)
{
   struct svga_token_key key;
   bool use_existing = false;
   struct svga_shader *transform_shader;
   const struct tgsi_shader_info *info = &shader->tgsi_info;

   /* Create a token key */
   memset(&key, 0, sizeof key);
   key.dynamic_indexing = 1;

   if (shader->next) {
      transform_shader = svga_search_shader_token_key(shader->next, &key);
      if (transform_shader) {
         use_existing = true;
      }
   }

   struct tgsi_token *new_tokens = NULL;

   if (!use_existing) {
      struct pipe_shader_state state = {0};
      new_tokens = tgsi_remove_dynamic_indexing(shader->tokens,
                                                info->const_buffers_declared,
                                                info->samplers_declared,
                                                info->immediate_count);
      if (!new_tokens)
         return;

      pipe_shader_state_from_tgsi(&state, new_tokens);

      transform_shader = create_shader(svga, info->processor, &state);
      insert_at_head(shader, transform_shader);
   }
   transform_shader->token_key = key;
   bind_shader(svga, info->processor, transform_shader);
   if (new_tokens)
      FREE(new_tokens);
}


/**
 * emulate_point_sprite searches the shader variants list to see it there is
 * a shader variant with a token string that matches the emulation
 * requirement. It there isn't, then it will use a tgsi utility
 * tgsi_add_point_sprite to transform the original token string to support
 * point sprite. A new geometry shader state will be created with the
 * transformed token string and added to the shader variants list of the
 * original geometry shader. The new geometry shader state will then be
 * bound as the current geometry shader.
 */
static struct svga_shader *
emulate_point_sprite(struct svga_context *svga,
                     struct svga_shader *shader,
                     const struct tgsi_token *tokens)
{
   struct svga_token_key key;
   struct tgsi_token *new_tokens;
   const struct tgsi_token *orig_tokens;
   struct svga_geometry_shader *orig_gs = (struct svga_geometry_shader *)shader;
   struct svga_geometry_shader *gs = NULL;
   struct pipe_shader_state templ = {0};
   struct svga_stream_output *streamout = NULL;
   int pos_out_index = -1;
   int aa_point_coord_index = -1;
   struct pipe_screen *screen = svga->pipe.screen;
   bool has_texcoord_semantic =
      screen->get_param(screen, PIPE_CAP_TGSI_TEXCOORD);

   assert(tokens != NULL);

   orig_tokens = tokens;

   /* Create a token key */
   memset(&key, 0, sizeof key);
   key.gs.writes_psize = 1;
   key.gs.sprite_coord_enable = svga->curr.rast->templ.sprite_coord_enable;
   if (has_texcoord_semantic)
      key.gs.sprite_coord_enable |= 0x1;   /* For TGSI_SEMANTIC_PCOORD */

   key.gs.sprite_origin_upper_left =
      !(svga->curr.rast->templ.sprite_coord_mode == PIPE_SPRITE_COORD_LOWER_LEFT);

   key.gs.aa_point = svga->curr.rast->templ.point_smooth;

   if (orig_gs) {

      /* Check if the original geometry shader has stream output and
       * if position is one of the outputs.
       */
      streamout = orig_gs->base.stream_output;
      if (streamout) {
         pos_out_index = streamout->pos_out_index;
         key.gs.point_pos_stream_out = pos_out_index != -1;
      }

      /* Search the shader lists to see if there is a variant that matches
       * this token key.
       */
      gs = (struct svga_geometry_shader *)
              svga_search_shader_token_key(&orig_gs->base, &key);
   }

   /* If there isn't, then call the tgsi utility tgsi_add_point_sprite
    * to transform the original tokens to support point sprite.
    * Flip the sprite origin as SVGA3D device only supports an
    * upper-left origin.
    */
   if (!gs) {
      new_tokens = tgsi_add_point_sprite(orig_tokens,
                                         key.gs.sprite_coord_enable,
                                         key.gs.sprite_origin_upper_left,
                                         key.gs.point_pos_stream_out,
					 has_texcoord_semantic,
                                         key.gs.aa_point ?
                                            &aa_point_coord_index : NULL);

      if (!new_tokens) {
         /* if no new tokens are generated for whatever reason, just return */
         return NULL;
      }

      if (0) {
         debug_printf("Before tgsi_add_point_sprite ---------------\n");
         tgsi_dump(orig_tokens, 0);
         debug_printf("After tgsi_add_point_sprite --------------\n");
         tgsi_dump(new_tokens, 0);
      }

      pipe_shader_state_from_tgsi(&templ, new_tokens);
      templ.stream_output.num_outputs = 0;

      if (streamout) {
         templ.stream_output = streamout->info;
         /* The tgsi_add_point_sprite utility adds an extra output
          * for the original point position for stream output purpose.
          * We need to replace the position output register index in the
          * stream output declaration with the new register index.
          */
         if (pos_out_index != -1) {
            assert(orig_gs != NULL);
            templ.stream_output.output[pos_out_index].register_index =
               orig_gs->base.tgsi_info.num_outputs;
         }
      }

      /* Create a new geometry shader state with the new tokens */
      gs = svga->pipe.create_gs_state(&svga->pipe, &templ);

      /* Don't need the token string anymore. There is a local copy
       * in the shader state.
       */
      FREE(new_tokens);

      if (!gs) {
         return NULL;
      }

      gs->wide_point = true;
      gs->aa_point_coord_index = aa_point_coord_index;
      gs->base.token_key = key;
      gs->base.parent = &orig_gs->base;
      gs->base.next = NULL;

      /* Add the new geometry shader to the head of the shader list
       * pointed to by the original geometry shader.
       */
      if (orig_gs) {
         gs->base.next = orig_gs->base.next;
         orig_gs->base.next = &gs->base;
      }
   }

   /* Bind the new geometry shader state */
   bind_gs_state(svga, gs);

   return &gs->base;
}

/**
 * Generate a geometry shader that emits a wide point by drawing a quad.
 * This function first creates a passthrough geometry shader and then
 * calls emulate_point_sprite() to transform the geometry shader to
 * support point sprite.
 */
static struct svga_shader *
add_point_sprite_shader(struct svga_context *svga)
{
   struct svga_vertex_shader *vs = svga->curr.vs;
   struct svga_geometry_shader *orig_gs = vs->gs;
   struct svga_geometry_shader *new_gs;
   const struct tgsi_token *tokens;

   if (orig_gs == NULL) {

      /* If this is the first time adding a geometry shader to this
       * vertex shader to support point sprite, then create
       * a passthrough geometry shader first.
       */
      orig_gs = (struct svga_geometry_shader *)
                   util_make_geometry_passthrough_shader(
                      &svga->pipe, vs->base.tgsi_info.num_outputs,
                      vs->base.tgsi_info.output_semantic_name,
                      vs->base.tgsi_info.output_semantic_index);

      if (!orig_gs)
         return NULL;
   }
   else {
      if (orig_gs->base.parent)
         orig_gs = (struct svga_geometry_shader *)orig_gs->base.parent;
   }
   tokens = orig_gs->base.tokens;

   /* Call emulate_point_sprite to find or create a transformed
    * geometry shader for supporting point sprite.
    */
   new_gs = (struct svga_geometry_shader *)
               emulate_point_sprite(svga, &orig_gs->base, tokens);

   /* If this is the first time creating a geometry shader to
    * support vertex point size, then add the new geometry shader
    * to the vertex shader.
    */
   if (vs->gs == NULL) {
      vs->gs = new_gs;
   }

   return &new_gs->base;
}


static bool
has_dynamic_indexing(const struct tgsi_shader_info *info)
{
   return (info->dim_indirect_files & (1u << TGSI_FILE_CONSTANT)) ||
      (info->indirect_files & (1u << TGSI_FILE_SAMPLER));
}


/* update_tgsi_transform provides a hook to transform a shader if needed.
 */
static enum pipe_error
update_tgsi_transform(struct svga_context *svga, uint64_t dirty)
{
   struct svga_geometry_shader *gs = svga->curr.user_gs;   /* current gs */
   struct svga_vertex_shader *vs = svga->curr.vs;     /* currently bound vs */
   struct svga_fragment_shader *fs = svga->curr.fs;   /* currently bound fs */
   struct svga_tcs_shader *tcs = svga->curr.tcs;      /* currently bound tcs */
   struct svga_tes_shader *tes = svga->curr.tes;      /* currently bound tes */
   struct svga_shader *orig_gs;                       /* original gs */
   struct svga_shader *new_gs;                        /* new gs */

   assert(svga_have_vgpu10(svga));

   if (vs->base.tgsi_info.num_outputs == 0) {
      write_vpos(svga, &vs->base);
   }

   if (vs && has_dynamic_indexing(&vs->base.tgsi_info)) {
      transform_dynamic_indexing(svga, &vs->base);
   }
   if (fs && has_dynamic_indexing(&fs->base.tgsi_info)) {
      transform_dynamic_indexing(svga, &fs->base);
   }
   if (gs && has_dynamic_indexing(&gs->base.tgsi_info)) {
      transform_dynamic_indexing(svga, &gs->base);
   }
   if (tcs && has_dynamic_indexing(&tcs->base.tgsi_info)) {
      transform_dynamic_indexing(svga, &tcs->base);
   }
   if (tes && has_dynamic_indexing(&tes->base.tgsi_info)) {
      transform_dynamic_indexing(svga, &tes->base);
   }

   if (svga->curr.reduced_prim == MESA_PRIM_POINTS) {
      /* If the current prim type is POINTS and the current geometry shader
       * emits wide points, transform the shader to emulate wide points using
       * quads. NOTE: we don't do emulation of wide points in GS when
       * transform feedback is enabled.
       */
      if (gs != NULL && !gs->base.stream_output &&
          (gs->base.tgsi_info.writes_psize || gs->wide_point)) {
         orig_gs = gs->base.parent ? gs->base.parent : &gs->base;
         new_gs = emulate_point_sprite(svga, orig_gs, orig_gs->tokens);
      }

      /* If there is not an active geometry shader and the current vertex
       * shader emits wide point then create a new geometry shader to emulate
       * wide point.
       */
      else if (gs == NULL && !vs->base.stream_output &&
               (svga->curr.rast->pointsize > 1.0 ||
                vs->base.tgsi_info.writes_psize)) {
         new_gs = add_point_sprite_shader(svga);
      }
      else {
         /* use the user's GS */
         bind_gs_state(svga, svga->curr.user_gs);
      }
   }
   else if (svga->curr.gs != svga->curr.user_gs) {
      /* If current primitive type is not POINTS, then make sure
       * we don't bind to any of the generated geometry shader
       */
      bind_gs_state(svga, svga->curr.user_gs);
   }
   (void) new_gs;    /* silence the unused var warning */

   return PIPE_OK;
}

struct svga_tracked_state svga_need_tgsi_transform =
{
   "transform shader for optimization",
   (SVGA_NEW_VS |
    SVGA_NEW_FS |
    SVGA_NEW_GS |
    SVGA_NEW_REDUCED_PRIMITIVE |
    SVGA_NEW_RAST),
   update_tgsi_transform
};
