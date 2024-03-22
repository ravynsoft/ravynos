/**************************************************************************
 *
 * Copyright 2007-2018 VMware, Inc.
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
 * AA line stage:  AA lines are converted triangles (with extra generic)
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

#include "tgsi/tgsi_transform.h"
#include "tgsi/tgsi_dump.h"

#include "draw_context.h"
#include "draw_private.h"
#include "draw_pipe.h"

#include "nir.h"
#include "nir/nir_draw_helpers.h"

/** Approx number of new tokens for instructions in aa_transform_inst() */
#define NUM_NEW_TOKENS 53


/**
 * Subclass of pipe_shader_state to carry extra fragment shader info.
 */
struct aaline_fragment_shader
{
   struct pipe_shader_state state;
   void *driver_fs;
   void *aaline_fs;
   int generic_attrib;  /**< generic used for distance */
};


/**
 * Subclass of draw_stage
 */
struct aaline_stage
{
   struct draw_stage stage;

   float half_line_width;

   /** For AA lines, this is the vertex attrib slot for new generic */
   unsigned coord_slot;
   /** position, not necessarily output zero */
   unsigned pos_slot;


   /*
    * Currently bound state
    */
   struct aaline_fragment_shader *fs;

   /*
    * Driver interface/override functions
    */
   void * (*driver_create_fs_state)(struct pipe_context *,
                                    const struct pipe_shader_state *);
   void (*driver_bind_fs_state)(struct pipe_context *, void *);
   void (*driver_delete_fs_state)(struct pipe_context *, void *);
};



/**
 * Subclass of tgsi_transform_context, used for transforming the
 * user's fragment shader to add the special AA instructions.
 */
struct aa_transform_context {
   struct tgsi_transform_context base;
   uint64_t tempsUsed;  /**< bitmask */
   int colorOutput; /**< which output is the primary color */
   int maxInput, maxGeneric;  /**< max input index found */
   int numImm; /**< number of immediate regsters */
   int colorTemp, aaTemp;  /**< temp registers */
};

/**
 * TGSI declaration transform callback.
 * Look for a free input attrib, and two free temp regs.
 */
static void
aa_transform_decl(struct tgsi_transform_context *ctx,
                  struct tgsi_full_declaration *decl)
{
   struct aa_transform_context *aactx = (struct aa_transform_context *)ctx;

   if (decl->Declaration.File == TGSI_FILE_OUTPUT &&
       decl->Semantic.Name == TGSI_SEMANTIC_COLOR &&
       decl->Semantic.Index == 0) {
      aactx->colorOutput = decl->Range.First;
   }
   else if (decl->Declaration.File == TGSI_FILE_INPUT) {
      if ((int) decl->Range.Last > aactx->maxInput)
         aactx->maxInput = decl->Range.Last;
      if (decl->Semantic.Name == TGSI_SEMANTIC_GENERIC &&
           (int) decl->Semantic.Index > aactx->maxGeneric) {
         aactx->maxGeneric = decl->Semantic.Index;
      }
   }
   else if (decl->Declaration.File == TGSI_FILE_TEMPORARY) {
      unsigned i;
      for (i = decl->Range.First;
           i <= decl->Range.Last; i++) {
         /*
          * XXX this bitfield doesn't really cut it...
          */
         aactx->tempsUsed |= UINT64_C(1) << i;
      }
   }

   ctx->emit_declaration(ctx, decl);
}

/**
 * TGSI immediate declaration transform callback.
 */
static void
aa_immediate(struct tgsi_transform_context *ctx,
                  struct tgsi_full_immediate *imm)
{
   struct aa_transform_context *aactx = (struct aa_transform_context *)ctx;

   ctx->emit_immediate(ctx, imm);
   aactx->numImm++;
}

/**
 * Find the lowest zero bit, or -1 if bitfield is all ones.
 */
static int
free_bit(uint64_t bitfield)
{
   return ffsll(~bitfield) - 1;
}


/**
 * TGSI transform prolog callback.
 */
static void
aa_transform_prolog(struct tgsi_transform_context *ctx)
{
   struct aa_transform_context *aactx = (struct aa_transform_context *) ctx;
   uint64_t usedTemps = aactx->tempsUsed;

   /* find two free temp regs */
   aactx->colorTemp = free_bit(usedTemps);
   usedTemps |= UINT64_C(1) << aactx->colorTemp;
   aactx->aaTemp = free_bit(usedTemps);
   assert(aactx->colorTemp >= 0);
   assert(aactx->aaTemp >= 0);

   /* declare new generic input/texcoord */
   tgsi_transform_input_decl(ctx, aactx->maxInput + 1,
                             TGSI_SEMANTIC_GENERIC, aactx->maxGeneric + 1,
                             TGSI_INTERPOLATE_LINEAR);

   /* declare new temp regs */
   tgsi_transform_temp_decl(ctx, aactx->aaTemp);
   tgsi_transform_temp_decl(ctx, aactx->colorTemp);

   /* declare new immediate reg */
   tgsi_transform_immediate_decl(ctx, 2.0, -1.0, 0.0, 0.25);
}


/**
 * TGSI transform epilog callback.
 */
static void
aa_transform_epilog(struct tgsi_transform_context *ctx)
{
   struct aa_transform_context *aactx = (struct aa_transform_context *) ctx;

   if (aactx->colorOutput != -1) {
      struct tgsi_full_instruction inst;
      /* insert distance-based coverage code for antialiasing. */

      /* saturate(linewidth - fabs(interpx), linelength - fabs(interpz) */
      inst = tgsi_default_full_instruction();
      inst.Instruction.Saturate = true;
      inst.Instruction.Opcode = TGSI_OPCODE_ADD;
      inst.Instruction.NumDstRegs = 1;
      tgsi_transform_dst_reg(&inst.Dst[0], TGSI_FILE_TEMPORARY,
                             aactx->aaTemp, TGSI_WRITEMASK_XZ);
      inst.Instruction.NumSrcRegs = 2;
      tgsi_transform_src_reg(&inst.Src[1], TGSI_FILE_INPUT, aactx->maxInput + 1,
                             TGSI_SWIZZLE_X, TGSI_SWIZZLE_X,
                             TGSI_SWIZZLE_Z, TGSI_SWIZZLE_Z);
      tgsi_transform_src_reg(&inst.Src[0], TGSI_FILE_INPUT, aactx->maxInput + 1,
                             TGSI_SWIZZLE_Y, TGSI_SWIZZLE_Y,
                             TGSI_SWIZZLE_W, TGSI_SWIZZLE_W);
      inst.Src[1].Register.Absolute = true;
      inst.Src[1].Register.Negate = true;
      ctx->emit_instruction(ctx, &inst);

      /* linelength * 2 - 1 */
      tgsi_transform_op3_swz_inst(ctx, TGSI_OPCODE_MAD,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_WRITEMASK_Y,
                                  TGSI_FILE_INPUT, aactx->maxInput + 1,
                                  TGSI_SWIZZLE_W, false,
                                  TGSI_FILE_IMMEDIATE, aactx->numImm,
                                  TGSI_SWIZZLE_X,
                                  TGSI_FILE_IMMEDIATE, aactx->numImm,
                                  TGSI_SWIZZLE_Y);

      /* MIN height alpha */
      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MIN,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_WRITEMASK_Z,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_SWIZZLE_Z,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_SWIZZLE_Y, false);

      /* MUL width / height alpha */
      tgsi_transform_op2_swz_inst(ctx, TGSI_OPCODE_MUL,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_WRITEMASK_W,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_SWIZZLE_X,
                                  TGSI_FILE_TEMPORARY, aactx->aaTemp,
                                  TGSI_SWIZZLE_Z, false);

      /* MOV rgb */
      tgsi_transform_op1_inst(ctx, TGSI_OPCODE_MOV,
                              TGSI_FILE_OUTPUT, aactx->colorOutput,
                              TGSI_WRITEMASK_XYZ,
                              TGSI_FILE_TEMPORARY, aactx->colorTemp);

      /* MUL alpha */
      tgsi_transform_op2_inst(ctx, TGSI_OPCODE_MUL,
                              TGSI_FILE_OUTPUT, aactx->colorOutput,
                              TGSI_WRITEMASK_W,
                              TGSI_FILE_TEMPORARY, aactx->colorTemp,
                              TGSI_FILE_TEMPORARY, aactx->aaTemp, false);
   }
}


/**
 * TGSI instruction transform callback.
 * Replace writes to result.color w/ a temp reg.
 */
static void
aa_transform_inst(struct tgsi_transform_context *ctx,
                  struct tgsi_full_instruction *inst)
{
   struct aa_transform_context *aactx = (struct aa_transform_context *) ctx;
   unsigned i;

   /*
    * Look for writes to result.color and replace with colorTemp reg.
    */
   for (i = 0; i < inst->Instruction.NumDstRegs; i++) {
      struct tgsi_full_dst_register *dst = &inst->Dst[i];
      if (dst->Register.File == TGSI_FILE_OUTPUT &&
          dst->Register.Index == aactx->colorOutput) {
         dst->Register.File = TGSI_FILE_TEMPORARY;
         dst->Register.Index = aactx->colorTemp;
      }
   }

   ctx->emit_instruction(ctx, inst);
}


/**
 * Generate the frag shader we'll use for drawing AA lines.
 * This will be the user's shader plus some arithmetic instructions.
 */
static bool
generate_aaline_fs(struct aaline_stage *aaline)
{
   struct pipe_context *pipe = aaline->stage.draw->pipe;
   const struct pipe_shader_state *orig_fs = &aaline->fs->state;
   struct pipe_shader_state aaline_fs;
   struct aa_transform_context transform;
   const unsigned newLen = tgsi_num_tokens(orig_fs->tokens) + NUM_NEW_TOKENS;

   aaline_fs = *orig_fs; /* copy to init */

   memset(&transform, 0, sizeof(transform));
   transform.colorOutput = -1;
   transform.maxInput = -1;
   transform.maxGeneric = -1;
   transform.colorTemp = -1;
   transform.aaTemp = -1;
   transform.base.prolog = aa_transform_prolog;
   transform.base.epilog = aa_transform_epilog;
   transform.base.transform_instruction = aa_transform_inst;
   transform.base.transform_declaration = aa_transform_decl;
   transform.base.transform_immediate = aa_immediate;

   aaline_fs.tokens = tgsi_transform_shader(orig_fs->tokens, newLen, &transform.base);
   if (!aaline_fs.tokens)
      return false;

#if 0 /* DEBUG */
   debug_printf("draw_aaline, orig shader:\n");
   tgsi_dump(orig_fs->tokens, 0);
   debug_printf("draw_aaline, new shader:\n");
   tgsi_dump(aaline_fs.tokens, 0);
#endif

   aaline->fs->aaline_fs = aaline->driver_create_fs_state(pipe, &aaline_fs);
   if (aaline->fs->aaline_fs != NULL)
      aaline->fs->generic_attrib = transform.maxGeneric + 1;

   FREE((void *)aaline_fs.tokens);
   return aaline->fs->aaline_fs != NULL;
}

static bool
generate_aaline_fs_nir(struct aaline_stage *aaline)
{
   struct pipe_context *pipe = aaline->stage.draw->pipe;
   const struct pipe_shader_state *orig_fs = &aaline->fs->state;
   struct pipe_shader_state aaline_fs;

   aaline_fs = *orig_fs; /* copy to init */
   aaline_fs.ir.nir = nir_shader_clone(NULL, orig_fs->ir.nir);
   if (!aaline_fs.ir.nir)
      return false;

   nir_lower_aaline_fs(aaline_fs.ir.nir, &aaline->fs->generic_attrib, NULL, NULL);
   aaline->fs->aaline_fs = aaline->driver_create_fs_state(pipe, &aaline_fs);
   if (aaline->fs->aaline_fs == NULL)
      return false;

   return true;
}

/**
 * When we're about to draw our first AA line in a batch, this function is
 * called to tell the driver to bind our modified fragment shader.
 */
static bool
bind_aaline_fragment_shader(struct aaline_stage *aaline)
{
   struct draw_context *draw = aaline->stage.draw;
   struct pipe_context *pipe = draw->pipe;

   if (!aaline->fs->aaline_fs) {
      if (aaline->fs->state.type == PIPE_SHADER_IR_NIR) {
         if (!generate_aaline_fs_nir(aaline))
            return false;
      } else
         if (!generate_aaline_fs(aaline))
            return false;
   }

   draw->suspend_flushing = true;
   aaline->driver_bind_fs_state(pipe, aaline->fs->aaline_fs);
   draw->suspend_flushing = false;

   return true;
}



static inline struct aaline_stage *
aaline_stage(struct draw_stage *stage)
{
   return (struct aaline_stage *) stage;
}


/**
 * Draw a wide line by drawing a quad, using geometry which will
 * fullfill GL's antialiased line requirements.
 */
static void
aaline_line(struct draw_stage *stage, struct prim_header *header)
{
   const struct aaline_stage *aaline = aaline_stage(stage);
   const float half_width = aaline->half_line_width;
   struct prim_header tri;
   struct vertex_header *v[8];
   unsigned coordPos = aaline->coord_slot;
   unsigned posPos = aaline->pos_slot;
   float *pos, *tex;
   float dx = header->v[1]->data[posPos][0] - header->v[0]->data[posPos][0];
   float dy = header->v[1]->data[posPos][1] - header->v[0]->data[posPos][1];
   float length = sqrtf(dx * dx + dy * dy);
   float c_a = dx / length, s_a = dy / length;
   float half_length = 0.5 * length;
   float t_l, t_w;
   unsigned i;

   half_length = half_length + 0.5f;

   t_w = half_width;
   t_l = 0.5f;

   /* allocate/dup new verts */
   for (i = 0; i < 4; i++) {
      v[i] = dup_vert(stage, header->v[i/2], i);
   }

   /*
    * Quad strip for line from v0 to v1 (*=endpoints):
    *
    *  1                             3
    *  +-----------------------------+
    *  |                             |
    *  | *v0                     v1* |
    *  |                             |
    *  +-----------------------------+
    *  0                             2
    */

   /*
    * We increase line length by 0.5 pixels (at each endpoint),
    * and calculate the tri endpoints by moving them half-width
    * distance away perpendicular to the line.
    * XXX: since we change line endpoints (by 0.5 pixel), should
    * actually re-interpolate all other values?
    */

   /* new verts */
   pos = v[0]->data[posPos];
   pos[0] += (-t_l * c_a -  t_w * s_a);
   pos[1] += (-t_l * s_a +  t_w * c_a);

   pos = v[1]->data[posPos];
   pos[0] += (-t_l * c_a - -t_w * s_a);
   pos[1] += (-t_l * s_a + -t_w * c_a);

   pos = v[2]->data[posPos];
   pos[0] += (t_l * c_a -  t_w * s_a);
   pos[1] += (t_l * s_a +  t_w * c_a);

   pos = v[3]->data[posPos];
   pos[0] += (t_l * c_a - -t_w * s_a);
   pos[1] += (t_l * s_a + -t_w * c_a);

   /* new texcoords */
   tex = v[0]->data[coordPos];
   ASSIGN_4V(tex, -half_width, half_width, -half_length, half_length);

   tex = v[1]->data[coordPos];
   ASSIGN_4V(tex, half_width, half_width, -half_length, half_length);

   tex = v[2]->data[coordPos];
   ASSIGN_4V(tex, -half_width, half_width, half_length, half_length);

   tex = v[3]->data[coordPos];
   ASSIGN_4V(tex, half_width, half_width, half_length, half_length);

   tri.v[0] = v[2];  tri.v[1] = v[1];  tri.v[2] = v[0];
   stage->next->tri(stage->next, &tri);

   tri.v[0] = v[3];  tri.v[1] = v[1];  tri.v[2] = v[2];
   stage->next->tri(stage->next, &tri);
}


static void
aaline_first_line(struct draw_stage *stage, struct prim_header *header)
{
   auto struct aaline_stage *aaline = aaline_stage(stage);
   struct draw_context *draw = stage->draw;
   struct pipe_context *pipe = draw->pipe;
   const struct pipe_rasterizer_state *rast = draw->rasterizer;
   void *r;

   assert(draw->rasterizer->line_smooth && !draw->rasterizer->multisample);

   if (draw->rasterizer->line_width <= 1.0)
      aaline->half_line_width = 1.0;
   else
      aaline->half_line_width = 0.5f * draw->rasterizer->line_width + 0.5f;

   if (!draw->rasterizer->half_pixel_center)
      /*
       * The tex coords probably would need adjustments?
       */
      debug_printf("aa lines without half pixel center may be wrong\n");

   /*
    * Bind (generate) our fragprog
    */
   if (!bind_aaline_fragment_shader(aaline)) {
      stage->line = draw_pipe_passthrough_line;
      stage->line(stage, header);
      return;
   }

   draw_aaline_prepare_outputs(draw, draw->pipeline.aaline);

   draw->suspend_flushing = true;

   /* Disable triangle culling, stippling, unfilled mode etc. */
   r = draw_get_rasterizer_no_cull(draw, rast);
   pipe->bind_rasterizer_state(pipe, r);

   draw->suspend_flushing = false;

   /* now really draw first line */
   stage->line = aaline_line;
   stage->line(stage, header);
}


static void
aaline_flush(struct draw_stage *stage, unsigned flags)
{
   struct draw_context *draw = stage->draw;
   struct aaline_stage *aaline = aaline_stage(stage);
   struct pipe_context *pipe = draw->pipe;

   stage->line = aaline_first_line;
   stage->next->flush(stage->next, flags);

   /* restore original frag shader */
   draw->suspend_flushing = true;
   aaline->driver_bind_fs_state(pipe, aaline->fs ? aaline->fs->driver_fs : NULL);

   /* restore original rasterizer state */
   if (draw->rast_handle) {
      pipe->bind_rasterizer_state(pipe, draw->rast_handle);
   }

   draw->suspend_flushing = false;

   draw_remove_extra_vertex_attribs(draw);
}


static void
aaline_reset_stipple_counter(struct draw_stage *stage)
{
   stage->next->reset_stipple_counter(stage->next);
}


static void
aaline_destroy(struct draw_stage *stage)
{
   struct aaline_stage *aaline = aaline_stage(stage);
   struct pipe_context *pipe = stage->draw->pipe;

   draw_free_temp_verts(stage);

   /* restore the old entry points */
   pipe->create_fs_state = aaline->driver_create_fs_state;
   pipe->bind_fs_state = aaline->driver_bind_fs_state;
   pipe->delete_fs_state = aaline->driver_delete_fs_state;

   FREE(stage);
}


static struct aaline_stage *
draw_aaline_stage(struct draw_context *draw)
{
   struct aaline_stage *aaline = CALLOC_STRUCT(aaline_stage);
   if (!aaline)
      return NULL;

   aaline->stage.draw = draw;
   aaline->stage.name = "aaline";
   aaline->stage.next = NULL;
   aaline->stage.point = draw_pipe_passthrough_point;
   aaline->stage.line = aaline_first_line;
   aaline->stage.tri = draw_pipe_passthrough_tri;
   aaline->stage.flush = aaline_flush;
   aaline->stage.reset_stipple_counter = aaline_reset_stipple_counter;
   aaline->stage.destroy = aaline_destroy;

   if (!draw_alloc_temp_verts(&aaline->stage, 8)) {
      aaline->stage.destroy(&aaline->stage);
      return NULL;
   }

   return aaline;
}


static struct aaline_stage *
aaline_stage_from_pipe(struct pipe_context *pipe)
{
   struct draw_context *draw = (struct draw_context *) pipe->draw;

   if (draw) {
      return aaline_stage(draw->pipeline.aaline);
   } else {
      return NULL;
   }
}


/**
 * This function overrides the driver's create_fs_state() function and
 * will typically be called by the gallium frontend.
 */
static void *
aaline_create_fs_state(struct pipe_context *pipe,
                       const struct pipe_shader_state *fs)
{
   struct aaline_stage *aaline = aaline_stage_from_pipe(pipe);
   struct aaline_fragment_shader *aafs = NULL;

   if (!aaline)
      return NULL;

   aafs = CALLOC_STRUCT(aaline_fragment_shader);

   if (!aafs)
      return NULL;

   aafs->state.type = fs->type;
   if (fs->type == PIPE_SHADER_IR_TGSI)
      aafs->state.tokens = tgsi_dup_tokens(fs->tokens);
   else
      aafs->state.ir.nir = nir_shader_clone(NULL, fs->ir.nir);

   /* pass-through */
   aafs->driver_fs = aaline->driver_create_fs_state(pipe, fs);

   return aafs;
}


static void
aaline_bind_fs_state(struct pipe_context *pipe, void *fs)
{
   struct aaline_stage *aaline = aaline_stage_from_pipe(pipe);
   struct aaline_fragment_shader *aafs = (struct aaline_fragment_shader *) fs;

   if (!aaline) {
      return;
   }

   /* save current */
   aaline->fs = aafs;
   /* pass-through */
   aaline->driver_bind_fs_state(pipe, (aafs ? aafs->driver_fs : NULL));
}


static void
aaline_delete_fs_state(struct pipe_context *pipe, void *fs)
{
   struct aaline_stage *aaline = aaline_stage_from_pipe(pipe);
   struct aaline_fragment_shader *aafs = (struct aaline_fragment_shader *) fs;

   if (!aafs) {
      return;
   }

   if (aaline) {
      /* pass-through */
      aaline->driver_delete_fs_state(pipe, aafs->driver_fs);

      if (aafs->aaline_fs)
         aaline->driver_delete_fs_state(pipe, aafs->aaline_fs);
   }

   if (aafs->state.type == PIPE_SHADER_IR_TGSI)
      FREE((void*)aafs->state.tokens);
   else
      ralloc_free(aafs->state.ir.nir);
   FREE(aafs);
}


void
draw_aaline_prepare_outputs(struct draw_context *draw,
                            struct draw_stage *stage)
{
   struct aaline_stage *aaline = aaline_stage(stage);
   const struct pipe_rasterizer_state *rast = draw->rasterizer;

   /* update vertex attrib info */
   aaline->pos_slot = draw_current_shader_position_output(draw);

   if (!rast->line_smooth || rast->multisample)
      return;

   /* allocate the extra post-transformed vertex attribute */
   if (aaline->fs && aaline->fs->aaline_fs)
      aaline->coord_slot = draw_alloc_extra_vertex_attrib(draw,
                                                          TGSI_SEMANTIC_GENERIC,
                                                          aaline->fs->generic_attrib);
   else
      aaline->coord_slot = -1;
}

/**
 * Called by drivers that want to install this AA line prim stage
 * into the draw module's pipeline.  This will not be used if the
 * hardware has native support for AA lines.
 */
bool
draw_install_aaline_stage(struct draw_context *draw, struct pipe_context *pipe)
{
   struct aaline_stage *aaline;

   pipe->draw = (void *) draw;

   /*
    * Create / install AA line drawing / prim stage
    */
   aaline = draw_aaline_stage(draw);
   if (!aaline)
      return false;

   /* save original driver functions */
   aaline->driver_create_fs_state = pipe->create_fs_state;
   aaline->driver_bind_fs_state = pipe->bind_fs_state;
   aaline->driver_delete_fs_state = pipe->delete_fs_state;

   /* override the driver's functions */
   pipe->create_fs_state = aaline_create_fs_state;
   pipe->bind_fs_state = aaline_bind_fs_state;
   pipe->delete_fs_state = aaline_delete_fs_state;

   /* Install once everything is known to be OK:
    */
   draw->pipeline.aaline = &aaline->stage;

   return true;
}
