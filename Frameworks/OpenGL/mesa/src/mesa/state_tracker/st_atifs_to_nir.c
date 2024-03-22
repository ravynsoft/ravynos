/*
 * Copyright (C) 2016 Miklós Máté
 * Copyright (C) 2020 Google LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "main/mtypes.h"
#include "main/atifragshader.h"
#include "main/errors.h"
#include "program/prog_parameter.h"
#include "program/prog_instruction.h"
#include "program/prog_to_nir.h"

#include "st_program.h"
#include "st_atifs_to_nir.h"
#include "compiler/nir/nir_builder.h"

/**
 * Intermediate state used during shader translation.
 */
struct st_translate {
   nir_builder *b;
   struct ati_fragment_shader *atifs;

   nir_def *temps[MAX_PROGRAM_TEMPS];

   nir_variable *fragcolor;
   nir_variable *constants;
   nir_variable *samplers[MAX_TEXTURE_UNITS];

   nir_def *inputs[VARYING_SLOT_MAX];

   unsigned current_pass;

   bool regs_written[MAX_NUM_PASSES_ATI][MAX_NUM_FRAGMENT_REGISTERS_ATI];

   bool error;
};

static nir_def *
nir_channel_vec4(nir_builder *b, nir_def *src, unsigned channel)
{
   unsigned swizzle[4] = { channel, channel, channel, channel };
   return nir_swizzle(b, src, swizzle, 4);
}

static nir_def *
nir_imm_vec4_float(nir_builder *b, float f)
{
   return nir_imm_vec4(b, f, f, f, f);
}

static nir_def *
get_temp(struct st_translate *t, unsigned index)
{
   if (!t->temps[index])
      t->temps[index] = nir_undef(t->b, 4, 32);
   return t->temps[index];
}

static nir_def *
apply_swizzle(struct st_translate *t,
              struct nir_def *src, GLuint swizzle)
{
   /* From the ATI_fs spec:
    *
    *     "Table 3.20 shows the <swizzle> modes:
    *
    *                           Coordinates Used for 1D or      Coordinates Used for
    *      Swizzle              2D SampleMap and PassTexCoord   3D or cubemap SampleMap
    *      -------              -----------------------------   -----------------------
    *      SWIZZLE_STR_ATI      (s, t, r, undefined)            (s, t, r, undefined)
    *      SWIZZLE_STQ_ATI      (s, t, q, undefined)            (s, t, q, undefined)
    *      SWIZZLE_STR_DR_ATI   (s/r, t/r, 1/r, undefined)      (undefined)
    *      SWIZZLE_STQ_DQ_ATI   (s/q, t/q, 1/q, undefined)      (undefined)
    */
   if (swizzle == GL_SWIZZLE_STR_ATI) {
      return src;
   } else if (swizzle == GL_SWIZZLE_STQ_ATI) {
      static unsigned xywz[4] = { 0, 1, 3, 2 };
      return nir_swizzle(t->b, src, xywz, 4);
   } else {
      nir_def *rcp = nir_frcp(t->b, nir_channel(t->b, src,
                                                    swizzle == GL_SWIZZLE_STR_DR_ATI ? 2 : 3));

      nir_def *st_mul = nir_fmul(t->b, nir_trim_vector(t->b, src, 2), rcp);

      return nir_vec4(t->b,
                      nir_channel(t->b, st_mul, 0),
                      nir_channel(t->b, st_mul, 1),
                      rcp,
                      rcp);
   }
}

static nir_def *
load_input(struct st_translate *t, gl_varying_slot slot)
{
   if (!t->inputs[slot]) {
      nir_variable *var = nir_create_variable_with_location(t->b->shader, nir_var_shader_in, slot,
                                                            glsl_vec4_type());
      var->data.interpolation = INTERP_MODE_NONE;

      t->inputs[slot] = nir_load_var(t->b, var);
   }

   return t->inputs[slot];
}

static nir_def *
atifs_load_uniform(struct st_translate *t, int index)
{
   nir_deref_instr *deref = nir_build_deref_array(t->b,
                                                  nir_build_deref_var(t->b, t->constants),
                                                  nir_imm_int(t->b, index));
   return nir_load_deref(t->b, deref);
}

static struct nir_def *
get_source(struct st_translate *t, GLenum src_type)
{
   if (src_type >= GL_REG_0_ATI && src_type <= GL_REG_5_ATI) {
      if (t->regs_written[t->current_pass][src_type - GL_REG_0_ATI]) {
         return get_temp(t, src_type - GL_REG_0_ATI);
      } else {
         return nir_imm_vec4_float(t->b, 0.0);
      }
   } else if (src_type >= GL_CON_0_ATI && src_type <= GL_CON_7_ATI) {
      int index = src_type - GL_CON_0_ATI;
      if (t->atifs->LocalConstDef & (1 << index)) {
         return nir_imm_vec4(t->b,
                             t->atifs->Constants[index][0],
                             t->atifs->Constants[index][1],
                             t->atifs->Constants[index][2],
                             t->atifs->Constants[index][3]);
      } else {
         return atifs_load_uniform(t, index);
      }
   } else if (src_type == GL_ZERO) {
      return nir_imm_vec4_float(t->b, 0.0);
   } else if (src_type == GL_ONE) {
      return nir_imm_vec4_float(t->b, 1.0);
   } else if (src_type == GL_PRIMARY_COLOR_ARB) {
      return load_input(t, VARYING_SLOT_COL0);
   } else if (src_type == GL_SECONDARY_INTERPOLATOR_ATI) {
      return load_input(t, VARYING_SLOT_COL1);
   } else {
      /* frontend prevents this */
      unreachable("unknown source");
   }
}

static nir_def *
prepare_argument(struct st_translate *t, const struct atifs_instruction *inst,
                 const unsigned argId, bool alpha)
{
   if (argId >= inst->ArgCount[alpha]) {
      _mesa_warning(0, "Using 0 for missing argument %d\n", argId);
      return nir_imm_vec4_float(t->b, 0.0f);
   }

   const struct atifragshader_src_register *srcReg = &inst->SrcReg[alpha][argId];

   nir_def *src = get_source(t, srcReg->Index);

   switch (srcReg->argRep) {
   case GL_NONE:
      break;
   case GL_RED:
      src = nir_channel_vec4(t->b, src, 0);
      break;
   case GL_GREEN:
      src = nir_channel_vec4(t->b, src, 1);
      break;
   case GL_BLUE:
      src = nir_channel_vec4(t->b, src, 2);
      break;
   case GL_ALPHA:
      src = nir_channel_vec4(t->b, src, 3);
      break;
   }

   t->temps[MAX_NUM_FRAGMENT_REGISTERS_ATI + argId] = src;

   if (srcReg->argMod & GL_COMP_BIT_ATI)
      src = nir_fsub_imm(t->b, 1.0, src);
   if (srcReg->argMod & GL_BIAS_BIT_ATI)
      src = nir_fadd_imm(t->b, src, -0.5);
   if (srcReg->argMod & GL_2X_BIT_ATI)
      src = nir_fadd(t->b, src, src);
   if (srcReg->argMod & GL_NEGATE_BIT_ATI)
      src = nir_fneg(t->b, src);

   return src;
}

static nir_def *
emit_arith_inst(struct st_translate *t,
                const struct atifs_instruction *inst,
                bool alpha)
{
   nir_def *src[3] = {0};
   for (int i = 0; i < inst->ArgCount[alpha]; i++)
      src[i] = prepare_argument(t, inst, i, alpha);

   switch (inst->Opcode[alpha]) {
   case GL_MOV_ATI:
      return src[0];

   case GL_ADD_ATI:
      return nir_fadd(t->b, src[0], src[1]);

   case GL_SUB_ATI:
      return nir_fsub(t->b, src[0], src[1]);

   case GL_MUL_ATI:
      return nir_fmul(t->b, src[0], src[1]);

   case GL_MAD_ATI:
      return nir_ffma(t->b, src[0], src[1], src[2]);

   case GL_LERP_ATI:
      return nir_flrp(t->b, src[2], src[1], src[0]);

   case GL_CND_ATI:
      return nir_bcsel(t->b,
                       nir_fle_imm(t->b, src[2], 0.5),
                       src[1],
                       src[0]);

   case GL_CND0_ATI:
      return nir_bcsel(t->b,
                       nir_fge_imm(t->b, src[2], 0.0),
                       src[0],
                       src[1]);

   case GL_DOT2_ADD_ATI:
      return nir_channel_vec4(t->b,
                              nir_fadd(t->b,
                                       nir_fdot2(t->b, src[0], src[1]),
                                       nir_channel(t->b, src[1], 2)),
                              0);

   case GL_DOT3_ATI:
      return nir_channel_vec4(t->b, nir_fdot3(t->b,src[0], src[1]), 0);

   case GL_DOT4_ATI:
      return nir_channel_vec4(t->b, nir_fdot4(t->b,src[0], src[1]), 0);

   default:
      unreachable("Unknown ATI_fs opcode");
   }
}

static nir_def *
emit_dstmod(struct st_translate *t,
            struct nir_def *dst, GLuint dstMod)
{
   switch (dstMod & ~GL_SATURATE_BIT_ATI) {
   case GL_2X_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 2.0f);
      break;
   case GL_4X_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 4.0f);
      break;
   case GL_8X_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 8.0f);
      break;
   case GL_HALF_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 0.5f);
      break;
   case GL_QUARTER_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 0.25f);
      break;
   case GL_EIGHTH_BIT_ATI:
      dst = nir_fmul_imm(t->b, dst, 0.125f);
      break;
   default:
      break;
   }

   if (dstMod & GL_SATURATE_BIT_ATI)
      dst = nir_fsat(t->b, dst);

   return dst;
}

/**
 * Compile one setup instruction to NIR instructions.
 */
static void
compile_setupinst(struct st_translate *t,
                  const unsigned r,
                  const struct atifs_setupinst *texinst)
{
   if (!texinst->Opcode)
      return;

   GLuint pass_tex = texinst->src;

   nir_def *coord;

   if (pass_tex >= GL_TEXTURE0_ARB && pass_tex <= GL_TEXTURE7_ARB) {
      unsigned attr = pass_tex - GL_TEXTURE0_ARB;

      coord = load_input(t, VARYING_SLOT_TEX0 + attr);
   } else if (pass_tex >= GL_REG_0_ATI && pass_tex <= GL_REG_5_ATI) {
      unsigned reg = pass_tex - GL_REG_0_ATI;

      /* the frontend already validated that REG is only allowed in second pass */
      if (t->regs_written[0][reg]) {
         coord = t->temps[reg];
      } else {
         coord = nir_imm_vec4_float(t->b, 0.0f);
      }
   } else {
      coord = nir_undef(t->b, 4, 32);
   }
   coord = apply_swizzle(t, coord, texinst->swizzle);

   if (texinst->Opcode == ATI_FRAGMENT_SHADER_SAMPLE_OP) {
      nir_variable *tex_var = t->samplers[r];
      if (!tex_var) {
         /* The actual sampler dim will be determined at draw time and lowered
          * by st_nir_update_atifs_samplers. Setting it to 3D for now means we
          * don't optimize out coordinate channels we may need later.
          */
         const struct glsl_type *sampler_type =
             glsl_sampler_type(GLSL_SAMPLER_DIM_3D, false, false, GLSL_TYPE_FLOAT);

         tex_var = nir_variable_create(t->b->shader, nir_var_uniform, sampler_type, "tex");
         tex_var->data.binding = r;
         tex_var->data.explicit_binding = true;
         t->samplers[r] = tex_var;
      }
      nir_deref_instr *tex_deref = nir_build_deref_var(t->b, t->samplers[r]);

      nir_tex_instr *tex = nir_tex_instr_create(t->b->shader, 3);
      tex->op = nir_texop_tex;
      tex->sampler_dim = glsl_get_sampler_dim(tex_var->type);
      tex->dest_type = nir_type_float32;
      tex->coord_components =
         glsl_get_sampler_dim_coordinate_components(tex->sampler_dim);

      tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                        &tex_deref->def);
      tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref,
                                        &tex_deref->def);
      tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_coord,
                                        nir_trim_vector(t->b, coord, tex->coord_components));

      nir_def_init(&tex->instr, &tex->def, 4, 32);
      nir_builder_instr_insert(t->b, &tex->instr);

      t->temps[r] = &tex->def;
   } else if (texinst->Opcode == ATI_FRAGMENT_SHADER_PASS_OP) {
      t->temps[r] = coord;
   }

   t->regs_written[t->current_pass][r] = true;
}

/**
 * Compile one arithmetic operation COLOR&ALPHA pair into NIR instructions.
 */
static void
compile_instruction(struct st_translate *t,
                    const struct atifs_instruction *inst)
{
   unsigned optype;

   for (optype = 0; optype < 2; optype++) { /* color, alpha */
      unsigned dstreg = inst->DstReg[optype].Index - GL_REG_0_ATI;

      if (!inst->Opcode[optype])
         continue;

      /* Execute the op */
      nir_def *result = emit_arith_inst(t, inst, optype);
      result = emit_dstmod(t, result, inst->DstReg[optype].dstMod);

      /* Do the writemask */
      nir_const_value wrmask[4];
      for (int i = 0; i < 4; i++) {
         bool bit = inst->DstReg[optype].dstMask & (1 << i);
         wrmask[i] = nir_const_value_for_bool(bit, 1);
      }

      t->temps[dstreg] = nir_bcsel(t->b,
                                   nir_build_imm(t->b, 4, 1, wrmask),
                                   result,
                                   get_temp(t, dstreg));
      t->regs_written[t->current_pass][dstreg] = true;
   }
}


/* Creates the uniform variable referencing the ATI_fragment_shader constants.
 */
static void
st_atifs_setup_uniforms(struct st_translate *t, struct gl_program *program)
{
   const struct glsl_type *type =
      glsl_array_type(glsl_vec4_type(), program->Parameters->NumParameters, 0);
   t->constants =
      nir_variable_create(t->b->shader, nir_var_uniform, type,
                          "gl_ATI_fragment_shader_constants");
}

/**
 * Called when a new variant is needed, we need to translate
 * the ATI fragment shader to NIR
 */
nir_shader *
st_translate_atifs_program(struct ati_fragment_shader *atifs,
                           struct gl_program *program,
                           const nir_shader_compiler_options *options)
{
   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT, options, "ATI_fs");

   struct st_translate translate = {
      .atifs = atifs,
      .b = &b,
   };
   struct st_translate *t = &translate;

   /* Copy the shader_info from the gl_program */
   t->b->shader->info = program->info;

   nir_shader *s = t->b->shader;
   s->info.name = ralloc_asprintf(s, "ATIFS%d", program->Id);
   s->info.internal = false;

   t->fragcolor = nir_create_variable_with_location(b.shader, nir_var_shader_out,
                                                    FRAG_RESULT_COLOR, glsl_vec4_type());

   st_atifs_setup_uniforms(t, program);

   /* emit instructions */
   for (unsigned pass = 0; pass < atifs->NumPasses; pass++) {
      t->current_pass = pass;
      for (unsigned r = 0; r < MAX_NUM_FRAGMENT_REGISTERS_ATI; r++) {
         struct atifs_setupinst *texinst = &atifs->SetupInst[pass][r];
         compile_setupinst(t, r, texinst);
      }
      for (unsigned i = 0; i < atifs->numArithInstr[pass]; i++) {
         struct atifs_instruction *inst = &atifs->Instructions[pass][i];
         compile_instruction(t, inst);
      }
   }

   if (t->regs_written[atifs->NumPasses-1][0])
      nir_store_var(t->b, t->fragcolor, t->temps[0], 0xf);

   return b.shader;
}

static bool
st_nir_lower_atifs_samplers_instr(nir_builder *b, nir_instr *instr, void *data)
{
   const uint8_t *texture_index = data;

   /* Can't just do this in tex handling below, as llvmpipe leaves dead code
    * derefs around.
    */
   if (instr->type == nir_instr_type_deref) {
      nir_deref_instr *deref = nir_instr_as_deref(instr);
      nir_variable *var = nir_deref_instr_get_variable(deref);
      if (glsl_type_is_sampler(var->type))
         deref->type = var->type;
   }

   if (instr->type != nir_instr_type_tex)
      return false;

   b->cursor = nir_before_instr(instr);

   nir_tex_instr *tex = nir_instr_as_tex(instr);

   unsigned unit;
   int sampler_src_idx = nir_tex_instr_src_index(tex, nir_tex_src_sampler_deref);
   if (sampler_src_idx >= 0) {
      nir_deref_instr *deref = nir_src_as_deref(tex->src[sampler_src_idx].src);
      nir_variable *var = nir_deref_instr_get_variable(deref);
      unit = var->data.binding;
   } else {
      unit = tex->sampler_index;
   }

   bool is_array;
   tex->sampler_dim =
       _mesa_texture_index_to_sampler_dim(texture_index[unit], &is_array);

   int coords_idx = nir_tex_instr_src_index(tex, nir_tex_src_coord);
   assert(coords_idx >= 0);
   int coord_components =
       glsl_get_sampler_dim_coordinate_components(tex->sampler_dim);
   /* Trim unused coords, or append undefs as necessary (if someone
    * accidentally enables a cube array).
    */
   if (coord_components != tex->coord_components) {
      nir_def *coords = tex->src[coords_idx].src.ssa;
      nir_src_rewrite(&tex->src[coords_idx].src,
                      nir_resize_vector(b, coords, coord_components));
      tex->coord_components = coord_components;
   }

   return true;
}

/**
 * Rewrites sampler dimensions and coordinate components for the currently
 * active texture unit at draw time.
 */
bool
st_nir_lower_atifs_samplers(struct nir_shader *s, const uint8_t *texture_index)
{
   nir_foreach_uniform_variable(var, s) {
      if (!glsl_type_is_sampler(var->type))
         continue;
      bool is_array;
      enum glsl_sampler_dim sampler_dim =
          _mesa_texture_index_to_sampler_dim(texture_index[var->data.binding], &is_array);
      var->type = glsl_sampler_type(sampler_dim, false, is_array, GLSL_TYPE_FLOAT);
   }

   return nir_shader_instructions_pass(s, st_nir_lower_atifs_samplers_instr,\
                                       nir_metadata_block_index |
                                       nir_metadata_dominance, (void *)texture_index);
}
