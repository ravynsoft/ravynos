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

 /*
  * Authors:
  *   Keith Whitwell <keithw@vmware.com>
  *   Brian Paul
  */

#include "util/u_math.h"
#include "util/u_memory.h"
#include "util/ralloc.h"
#include "pipe/p_shader_tokens.h"
#include "pipe/p_context.h"
#include "nir/nir_to_tgsi.h"

#include "draw_private.h"
#include "draw_context.h"
#include "draw_vs.h"

#include "tgsi/tgsi_parse.h"
#include "tgsi/tgsi_scan.h"
#include "tgsi/tgsi_exec.h"


struct exec_vertex_shader {
   struct draw_vertex_shader base;
   struct tgsi_exec_machine *machine;
};


static struct exec_vertex_shader *
exec_vertex_shader(struct draw_vertex_shader *vs)
{
   return (struct exec_vertex_shader *)vs;
}


/* Not required for run_linear.
 */
static void
vs_exec_prepare(struct draw_vertex_shader *shader,
                struct draw_context *draw)
{
   struct exec_vertex_shader *evs = exec_vertex_shader(shader);

   assert(!draw->llvm);
   /* Specify the vertex program to interpret/execute.
    * Avoid rebinding when possible.
    */
   if (evs->machine->Tokens != shader->state.tokens) {
      tgsi_exec_machine_bind_shader(evs->machine,
                                    shader->state.tokens,
                                    draw->vs.tgsi.sampler,
                                    draw->vs.tgsi.image,
                                    draw->vs.tgsi.buffer);
   }
}



/**
 * Simplified vertex shader interface for the pt paths.  Given the
 * complexity of code-generating all the above operations together,
 * it's time to try doing all the other stuff separately.
 */
static void
vs_exec_run_linear(struct draw_vertex_shader *shader,
                   const float (*input)[4],
                   float (*output)[4],
                   const struct draw_buffer_info *constants,
                   unsigned count,
                   unsigned input_stride,
                   unsigned output_stride,
                   const unsigned *fetch_elts)
{
   struct exec_vertex_shader *evs = exec_vertex_shader(shader);
   struct tgsi_exec_machine *machine = evs->machine;
   unsigned int i, j;
   unsigned slot;
   bool clamp_vertex_color = shader->draw->rasterizer->clamp_vertex_color;

   assert(!shader->draw->llvm);
   tgsi_exec_set_constant_buffers(machine, PIPE_MAX_CONSTANT_BUFFERS,
                                  (const struct tgsi_exec_consts_info *)constants);

   if (shader->info.uses_instanceid) {
      unsigned i = machine->SysSemanticToIndex[TGSI_SEMANTIC_INSTANCEID];
      assert(i < ARRAY_SIZE(machine->SystemValue));
      for (j = 0; j < TGSI_QUAD_SIZE; j++)
         machine->SystemValue[i].xyzw[0].i[j] = shader->draw->instance_id;
   }

   for (i = 0; i < count; i += MAX_TGSI_VERTICES) {
      unsigned int max_vertices = MIN2(MAX_TGSI_VERTICES, count - i);

      /* Swizzle inputs.
       */
      for (j = 0; j < max_vertices; j++) {
#if 0
         debug_printf("%d) Input vert:\n", i + j);
         for (slot = 0; slot < shader->info.num_inputs; slot++) {
            debug_printf("\t%d: %f %f %f %f\n", slot,
                         input[slot][0],
                         input[slot][1],
                         input[slot][2],
                         input[slot][3]);
         }
#endif
	 int basevertex = shader->draw->pt.user.eltSize ? shader->draw->pt.user.eltBias : shader->draw->start_index;

         if (shader->info.uses_vertexid) {
            unsigned vid = machine->SysSemanticToIndex[TGSI_SEMANTIC_VERTEXID];
            assert(vid < ARRAY_SIZE(machine->SystemValue));
            machine->SystemValue[vid].xyzw[0].i[j] = fetch_elts ? fetch_elts[i + j] : (i + j + basevertex);
         }
         if (shader->info.uses_basevertex) {
            unsigned vid = machine->SysSemanticToIndex[TGSI_SEMANTIC_BASEVERTEX];
            assert(vid < ARRAY_SIZE(machine->SystemValue));
            machine->SystemValue[vid].xyzw[0].i[j] = basevertex;
         }
         if (shader->info.uses_vertexid_nobase) {
            unsigned vid = machine->SysSemanticToIndex[TGSI_SEMANTIC_VERTEXID_NOBASE];
            assert(vid < ARRAY_SIZE(machine->SystemValue));
            machine->SystemValue[vid].xyzw[0].i[j] = fetch_elts ? (fetch_elts[i + j] - basevertex) : (i + j);
         }

         for (slot = 0; slot < shader->info.num_inputs; slot++) {
#if 0
            assert(!util_is_inf_or_nan(input[slot][0]));
            assert(!util_is_inf_or_nan(input[slot][1]));
            assert(!util_is_inf_or_nan(input[slot][2]));
            assert(!util_is_inf_or_nan(input[slot][3]));
#endif
            machine->Inputs[slot].xyzw[0].f[j] = input[slot][0];
            machine->Inputs[slot].xyzw[1].f[j] = input[slot][1];
            machine->Inputs[slot].xyzw[2].f[j] = input[slot][2];
            machine->Inputs[slot].xyzw[3].f[j] = input[slot][3];
         }

         input = (const float (*)[4])((const char *)input + input_stride);
      }

      machine->NonHelperMask = (1 << max_vertices) - 1;
      /* run interpreter */
      tgsi_exec_machine_run(machine, 0);

      /* Unswizzle all output results.
       */
      for (j = 0; j < max_vertices; j++) {
         for (slot = 0; slot < shader->info.num_outputs; slot++) {
            enum tgsi_semantic name = shader->info.output_semantic_name[slot];
            if (clamp_vertex_color &&
                (name == TGSI_SEMANTIC_COLOR || name == TGSI_SEMANTIC_BCOLOR)) {
               output[slot][0] = SATURATE(machine->Outputs[slot].xyzw[0].f[j]);
               output[slot][1] = SATURATE(machine->Outputs[slot].xyzw[1].f[j]);
               output[slot][2] = SATURATE(machine->Outputs[slot].xyzw[2].f[j]);
               output[slot][3] = SATURATE(machine->Outputs[slot].xyzw[3].f[j]);
            } else {
               output[slot][0] = machine->Outputs[slot].xyzw[0].f[j];
               output[slot][1] = machine->Outputs[slot].xyzw[1].f[j];
               output[slot][2] = machine->Outputs[slot].xyzw[2].f[j];
               output[slot][3] = machine->Outputs[slot].xyzw[3].f[j];
            }
         }

#if 0
         debug_printf("%d) Post xform vert:\n", i + j);
         for (slot = 0; slot < shader->info.num_outputs; slot++) {
            debug_printf("\t%d: %f %f %f %f\n", slot,
                         output[slot][0],
                         output[slot][1],
                         output[slot][2],
                         output[slot][3]);
            assert(!util_is_inf_or_nan(output[slot][0]));
         }
#endif

         output = (float (*)[4])((char *)output + output_stride);
      }
   }
}


static void
vs_exec_delete(struct draw_vertex_shader *dvs)
{
   FREE((void*) dvs->state.tokens);
   FREE(dvs);
}


struct draw_vertex_shader *
draw_create_vs_exec(struct draw_context *draw,
                    const struct pipe_shader_state *state)
{
   struct exec_vertex_shader *vs = CALLOC_STRUCT(exec_vertex_shader);

   if (!vs)
      return NULL;

   if (state->type == PIPE_SHADER_IR_NIR) {
      vs->base.state.type = PIPE_SHADER_IR_TGSI;
      vs->base.state.tokens = nir_to_tgsi(state->ir.nir, draw->pipe->screen);
   } else {
      assert(state->type == PIPE_SHADER_IR_TGSI);
      vs->base.state.type = state->type;

      /* we need to keep a local copy of the tokens */
      vs->base.state.tokens = tgsi_dup_tokens(state->tokens);
      if (!vs->base.state.tokens) {
         FREE(vs);
         return NULL;
      }
   }

   tgsi_scan_shader(vs->base.state.tokens, &vs->base.info);

   vs->base.state.stream_output = state->stream_output;
   vs->base.draw = draw;
   vs->base.prepare = vs_exec_prepare;
   vs->base.run_linear = vs_exec_run_linear;
   vs->base.delete = vs_exec_delete;
   vs->base.create_variant = draw_vs_create_variant_generic;
   vs->machine = draw->vs.tgsi.machine;

   return &vs->base;
}
