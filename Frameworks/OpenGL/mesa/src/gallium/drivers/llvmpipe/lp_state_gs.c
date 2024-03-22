/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
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

#include "lp_context.h"
#include "lp_state.h"
#include "lp_texture.h"
#include "lp_debug.h"

#include "pipe/p_defines.h"
#include "util/u_memory.h"
#include "util/u_inlines.h"
#include "draw/draw_context.h"
#include "draw/draw_gs.h"
#include "tgsi/tgsi_dump.h"


static void *
llvmpipe_create_gs_state(struct pipe_context *pipe,
                         const struct pipe_shader_state *templ)
{
   llvmpipe_register_shader(pipe, templ, false);

   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);
   struct lp_geometry_shader *state;

   state = CALLOC_STRUCT(lp_geometry_shader);
   if (!state)
      goto no_state;

   /* debug */
   if (LP_DEBUG & DEBUG_TGSI && templ->type == PIPE_SHADER_IR_TGSI) {
      debug_printf("llvmpipe: Create geometry shader %p:\n", (void *)state);
      tgsi_dump(templ->tokens, 0);
   }

   /* copy stream output info */
   if (templ->type == PIPE_SHADER_IR_TGSI)
      state->no_tokens = !templ->tokens;
   else
      state->no_tokens = false;
   memcpy(&state->stream_output, &templ->stream_output, sizeof state->stream_output);

   if (templ->tokens || templ->type == PIPE_SHADER_IR_NIR) {
      state->dgs = draw_create_geometry_shader(llvmpipe->draw, templ);
      if (state->dgs == NULL) {
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
llvmpipe_bind_gs_state(struct pipe_context *pipe, void *gs)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   llvmpipe->gs = (struct lp_geometry_shader *)gs;

   draw_bind_geometry_shader(llvmpipe->draw,
                             (llvmpipe->gs ? llvmpipe->gs->dgs : NULL));

   llvmpipe->dirty |= LP_NEW_GS;
}


static void
llvmpipe_delete_gs_state(struct pipe_context *pipe, void *gs)
{
   struct llvmpipe_context *llvmpipe = llvmpipe_context(pipe);

   struct lp_geometry_shader *state =
      (struct lp_geometry_shader *)gs;

   if (!state) {
      return;
   }

   llvmpipe_register_shader(pipe, &state->dgs->state, true);

   draw_delete_geometry_shader(llvmpipe->draw, state->dgs);
   FREE(state);
}


void
llvmpipe_init_gs_funcs(struct llvmpipe_context *llvmpipe)
{
   llvmpipe->pipe.create_gs_state = llvmpipe_create_gs_state;
   llvmpipe->pipe.bind_gs_state   = llvmpipe_bind_gs_state;
   llvmpipe->pipe.delete_gs_state = llvmpipe_delete_gs_state;
}
