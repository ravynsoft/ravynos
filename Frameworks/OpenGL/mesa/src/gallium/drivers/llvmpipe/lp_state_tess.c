/**************************************************************************
 *
 * Copyright 2019 Red Hat.
 * All Rights Reserved.
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
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 **************************************************************************/

#include "lp_context.h"
#include "lp_state.h"
#include "lp_texture.h"
#include "lp_debug.h"

#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "draw/draw_context.h"
#include "draw/draw_tess.h"
#include "tgsi/tgsi_dump.h"


static void *
llvmpipe_create_tcs_state(struct pipe_context *pipe,
                          const struct pipe_shader_state *templ)
{
   llvmpipe_register_shader(pipe, templ, false);

   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   struct lp_tess_ctrl_shader *state;

   state = CALLOC_STRUCT(lp_tess_ctrl_shader);
   if (!state)
      goto no_state;

   /* debug */
   if (LP_DEBUG & DEBUG_TGSI && templ->type == PIPE_SHADER_IR_TGSI) {
      debug_printf("llvmpipe: Create tess ctrl shader %p:\n", (void *)state);
      tgsi_dump(templ->tokens, 0);
   }

   /* copy stream output info */
   state->no_tokens = !templ->tokens;
   memcpy(&state->stream_output, &templ->stream_output, sizeof state->stream_output);

   if (templ->tokens || templ->type == PIPE_SHADER_IR_NIR) {
      state->dtcs = draw_create_tess_ctrl_shader(llvmpipe->draw, templ);
      if (state->dtcs == NULL) {
         goto no_dgs;
      }
   }

   return state;

no_dgs:
   FREE(state);
no_state:
   return NULL;
}


static void
llvmpipe_bind_tcs_state(struct pipe_context *pipe, void *tcs)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->tcs = (struct lp_tess_ctrl_shader *)tcs;

   draw_bind_tess_ctrl_shader(llvmpipe->draw,
                              (llvmpipe->tcs ? llvmpipe->tcs->dtcs : NULL));

   llvmpipe->dirty |= LP_NEW_TCS;
}


static void
llvmpipe_delete_tcs_state(struct pipe_context *pipe, void *tcs)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   struct lp_tess_ctrl_shader *state =
      (struct lp_tess_ctrl_shader *)tcs;

   if (!state) {
      return;
   }

   llvmpipe_register_shader(pipe, &state->dtcs->state, true);

   draw_delete_tess_ctrl_shader(llvmpipe->draw, state->dtcs);
   FREE(state);
}


static void *
llvmpipe_create_tes_state(struct pipe_context *pipe,
                          const struct pipe_shader_state *templ)
{
   llvmpipe_register_shader(pipe, templ, false);

   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   struct lp_tess_eval_shader *state;

   state = CALLOC_STRUCT(lp_tess_eval_shader);
   if (!state)
      goto no_state;

   /* debug */
   if (LP_DEBUG & DEBUG_TGSI) {
      debug_printf("llvmpipe: Create tess eval shader %p:\n", (void *)state);
      tgsi_dump(templ->tokens, 0);
   }

   /* copy stream output info */
   state->no_tokens = !templ->tokens;
   memcpy(&state->stream_output, &templ->stream_output, sizeof state->stream_output);

   if (templ->tokens || templ->type == PIPE_SHADER_IR_NIR) {
      state->dtes = draw_create_tess_eval_shader(llvmpipe->draw, templ);
      if (state->dtes == NULL) {
         goto no_dgs;
      }
   }

   return state;

no_dgs:
   FREE(state);
no_state:
   return NULL;
}


static void
llvmpipe_bind_tes_state(struct pipe_context *pipe, void *tes)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->tes = (struct lp_tess_eval_shader *)tes;

   draw_bind_tess_eval_shader(llvmpipe->draw,
                              (llvmpipe->tes ? llvmpipe->tes->dtes : NULL));

   llvmpipe->dirty |= LP_NEW_TES;
}


static void
llvmpipe_delete_tes_state(struct pipe_context *pipe, void *tes)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   struct lp_tess_eval_shader *state =
      (struct lp_tess_eval_shader *)tes;

   if (!state) {
      return;
   }

   llvmpipe_register_shader(pipe, &state->dtes->state, true);

   draw_delete_tess_eval_shader(llvmpipe->draw, state->dtes);
   FREE(state);
}

static void
llvmpipe_set_tess_state(struct pipe_context *pipe,
                        const float default_outer_level[4],
                        const float default_inner_level[2])
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   draw_set_tess_state(llvmpipe->draw, default_outer_level, default_inner_level);
}

static void
llvmpipe_set_patch_vertices(struct pipe_context *pipe, uint8_t patch_vertices)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->patch_vertices = patch_vertices;
}

void
llvmpipe_init_tess_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.create_tcs_state = llvmpipe_create_tcs_state;
   llvmpipe->pipe.bind_tcs_state   = llvmpipe_bind_tcs_state;
   llvmpipe->pipe.delete_tcs_state = llvmpipe_delete_tcs_state;

   llvmpipe->pipe.create_tes_state = llvmpipe_create_tes_state;
   llvmpipe->pipe.bind_tes_state   = llvmpipe_bind_tes_state;
   llvmpipe->pipe.delete_tes_state = llvmpipe_delete_tes_state;

   llvmpipe->pipe.set_tess_state = llvmpipe_set_tess_state;
   llvmpipe->pipe.set_patch_vertices = llvmpipe_set_patch_vertices;
}
