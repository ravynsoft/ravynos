/**************************************************************************
 *
 * Copyright 2007 VMware, Inc.
 * All Rights Reserved.
 * Copyright 2009 VMware, Inc.  All Rights Reserved.
 * Copyright © 2010-2011 Intel Corporation
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

#include "util/glheader.h"
#include "main/context.h"

#include "main/macros.h"
#include "main/state.h"
#include "main/texenvprogram.h"
#include "main/texobj.h"
#include "program/program.h"
#include "program/prog_cache.h"
#include "program/prog_statevars.h"
#include "program/prog_to_nir.h"
#include "util/bitscan.h"

#include "state_tracker/st_context.h"
#include "state_tracker/st_program.h"
#include "state_tracker/st_nir.h"

#include "compiler/nir/nir_builder.h"
#include "compiler/nir/nir_builtin_builder.h"

/*
 * Note on texture units:
 *
 * The number of texture units supported by fixed-function fragment
 * processing is MAX_TEXTURE_COORD_UNITS, not MAX_TEXTURE_IMAGE_UNITS.
 * That's because there's a one-to-one correspondence between texture
 * coordinates and samplers in fixed-function processing.
 *
 * Since fixed-function vertex processing is limited to MAX_TEXTURE_COORD_UNITS
 * sets of texcoords, so is fixed-function fragment processing.
 *
 * We can safely use ctx->Const.MaxTextureUnits for loop bounds.
 */


static GLboolean
texenv_doing_secondary_color(struct gl_context *ctx)
{
   if (ctx->Light.Enabled &&
       (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR))
      return GL_TRUE;

   if (ctx->Fog.ColorSumEnabled)
      return GL_TRUE;

   return GL_FALSE;
}

struct state_key {
   GLuint nr_enabled_units:4;
   GLuint separate_specular:1;
   GLuint fog_mode:2;          /**< FOG_x */
   GLuint inputs_available:12;
   GLuint num_draw_buffers:4;

   /* NOTE: This array of structs must be last! (see "keySize" below) */
   struct {
      GLuint enabled:1;
      GLuint source_index:4;   /**< TEXTURE_x_INDEX */
      GLuint shadow:1;

      /***
       * These are taken from struct gl_tex_env_combine_packed
       * @{
       */
      GLuint ModeRGB:4;
      GLuint ModeA:4;
      GLuint ScaleShiftRGB:2;
      GLuint ScaleShiftA:2;
      GLuint NumArgsRGB:3;
      GLuint NumArgsA:3;
      struct gl_tex_env_argument ArgsRGB[MAX_COMBINER_TERMS];
      struct gl_tex_env_argument ArgsA[MAX_COMBINER_TERMS];
      /** @} */
   } unit[MAX_TEXTURE_COORD_UNITS];
};


/**
 * Do we need to clamp the results of the given texture env/combine mode?
 * If the inputs to the mode are in [0,1] we don't always have to clamp
 * the results.
 */
static GLboolean
need_saturate( GLuint mode )
{
   switch (mode) {
   case TEXENV_MODE_REPLACE:
   case TEXENV_MODE_MODULATE:
   case TEXENV_MODE_INTERPOLATE:
      return GL_FALSE;
   case TEXENV_MODE_ADD:
   case TEXENV_MODE_ADD_SIGNED:
   case TEXENV_MODE_SUBTRACT:
   case TEXENV_MODE_DOT3_RGB:
   case TEXENV_MODE_DOT3_RGB_EXT:
   case TEXENV_MODE_DOT3_RGBA:
   case TEXENV_MODE_DOT3_RGBA_EXT:
   case TEXENV_MODE_MODULATE_ADD_ATI:
   case TEXENV_MODE_MODULATE_SIGNED_ADD_ATI:
   case TEXENV_MODE_MODULATE_SUBTRACT_ATI:
   case TEXENV_MODE_ADD_PRODUCTS_NV:
   case TEXENV_MODE_ADD_PRODUCTS_SIGNED_NV:
      return GL_TRUE;
   default:
      assert(0);
      return GL_FALSE;
   }
}

#define VERT_BIT_TEX_ANY    (0xff << VERT_ATTRIB_TEX0)

/**
 * Identify all possible varying inputs.  The fragment program will
 * never reference non-varying inputs, but will track them via state
 * constants instead.
 *
 * This function figures out all the inputs that the fragment program
 * has access to and filters input bitmask.
 */
static GLbitfield filter_fp_input_mask( GLbitfield fp_inputs,
                                        struct gl_context *ctx )
{
   if (ctx->VertexProgram._Overriden) {
      /* Somebody's messing with the vertex program and we don't have
       * a clue what's happening.  Assume that it could be producing
       * all possible outputs.
       */
      return fp_inputs;
   }

   if (ctx->RenderMode == GL_FEEDBACK) {
      /* _NEW_RENDERMODE */
      return fp_inputs & (VARYING_BIT_COL0 | VARYING_BIT_TEX0);
   }

   /* _NEW_PROGRAM */
   const GLboolean vertexShader =
         ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX] != NULL;
   const GLboolean vertexProgram = _mesa_arb_vertex_program_enabled(ctx);

   if (!(vertexProgram || vertexShader)) {
      /* Fixed function vertex logic */
      GLbitfield possible_inputs = 0;

      GLbitfield varying_inputs = ctx->VertexProgram._VaryingInputs;
      /* We only update ctx->VertexProgram._VaryingInputs when in VP_MODE_FF _VPMode */
      assert(VP_MODE_FF == ctx->VertexProgram._VPMode);

      /* These get generated in the setup routine regardless of the
       * vertex program:
       */
      /* _NEW_POINT */
      if (ctx->Point.PointSprite) {
         /* All texture varyings are possible to use */
         possible_inputs = VARYING_BITS_TEX_ANY;
      }
      else {
         const GLbitfield possible_tex_inputs =
               ctx->Texture._TexGenEnabled |
               ctx->Texture._TexMatEnabled |
               ((varying_inputs & VERT_BIT_TEX_ANY) >> VERT_ATTRIB_TEX0);

         possible_inputs = (possible_tex_inputs << VARYING_SLOT_TEX0);
      }

      /* First look at what values may be computed by the generated
       * vertex program:
       */
      if (ctx->Light.Enabled) {
         possible_inputs |= VARYING_BIT_COL0;

         if (texenv_doing_secondary_color(ctx))
            possible_inputs |= VARYING_BIT_COL1;
      }

      /* Then look at what might be varying as a result of enabled
       * arrays, etc:
       */
      if (varying_inputs & VERT_BIT_COLOR0)
         possible_inputs |= VARYING_BIT_COL0;
      if (varying_inputs & VERT_BIT_COLOR1)
         possible_inputs |= VARYING_BIT_COL1;

      return fp_inputs & possible_inputs;
   }

   /* calculate from vp->outputs */
   struct gl_program *vprog;

   /* Choose GLSL vertex shader over ARB vertex program.  Need this
    * since vertex shader state validation comes after fragment state
    * validation (see additional comments in state.c).
    */
   if (ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY] != NULL)
      vprog = ctx->_Shader->CurrentProgram[MESA_SHADER_GEOMETRY];
   else if (ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_EVAL] != NULL)
      vprog = ctx->_Shader->CurrentProgram[MESA_SHADER_TESS_EVAL];
   else if (vertexShader)
      vprog = ctx->_Shader->CurrentProgram[MESA_SHADER_VERTEX];
   else
      vprog = ctx->VertexProgram.Current;

   GLbitfield possible_inputs = vprog->info.outputs_written;

   /* These get generated in the setup routine regardless of the
    * vertex program:
    */
   /* _NEW_POINT */
   if (ctx->Point.PointSprite) {
      /* All texture varyings are possible to use */
      possible_inputs |= VARYING_BITS_TEX_ANY;
   }

   return fp_inputs & possible_inputs;
}


/**
 * Examine current texture environment state and generate a unique
 * key to identify it.
 */
static GLuint make_state_key( struct gl_context *ctx,  struct state_key *key )
{
   GLbitfield inputs_referenced = VARYING_BIT_COL0;
   GLbitfield mask;
   GLuint keySize;

   memset(key, 0, sizeof(*key));

   /* _NEW_TEXTURE_OBJECT | _NEW_TEXTURE_STATE */
   mask = ctx->Texture._EnabledCoordUnits;
   int i = -1;
   while (mask) {
      i = u_bit_scan(&mask);
      const struct gl_texture_unit *texUnit = &ctx->Texture.Unit[i];
      const struct gl_texture_object *texObj = texUnit->_Current;
      const struct gl_tex_env_combine_packed *comb =
         &ctx->Texture.FixedFuncUnit[i]._CurrentCombinePacked;

      if (!texObj)
         continue;

      key->unit[i].enabled = 1;
      inputs_referenced |= VARYING_BIT_TEX(i);

      key->unit[i].source_index = texObj->TargetIndex;

      const struct gl_sampler_object *samp = _mesa_get_samplerobj(ctx, i);
      if (samp->Attrib.CompareMode == GL_COMPARE_R_TO_TEXTURE) {
         const GLenum format = _mesa_texture_base_format(texObj);
         key->unit[i].shadow = (format == GL_DEPTH_COMPONENT ||
                                format == GL_DEPTH_STENCIL_EXT);
      }

      key->unit[i].ModeRGB = comb->ModeRGB;
      key->unit[i].ModeA = comb->ModeA;
      key->unit[i].ScaleShiftRGB = comb->ScaleShiftRGB;
      key->unit[i].ScaleShiftA = comb->ScaleShiftA;
      key->unit[i].NumArgsRGB = comb->NumArgsRGB;
      key->unit[i].NumArgsA = comb->NumArgsA;

      memcpy(key->unit[i].ArgsRGB, comb->ArgsRGB, sizeof comb->ArgsRGB);
      memcpy(key->unit[i].ArgsA, comb->ArgsA, sizeof comb->ArgsA);
   }

   key->nr_enabled_units = i + 1;

   /* _NEW_FOG */
   if (texenv_doing_secondary_color(ctx)) {
      key->separate_specular = 1;
      inputs_referenced |= VARYING_BIT_COL1;
   }

   /* _NEW_FOG */
   key->fog_mode = ctx->Fog._PackedEnabledMode;

   /* _NEW_BUFFERS */
   key->num_draw_buffers = ctx->DrawBuffer->_NumColorDrawBuffers;

   /* _NEW_COLOR */
   if (ctx->Color.AlphaEnabled && key->num_draw_buffers == 0) {
      /* if alpha test is enabled we need to emit at least one color */
      key->num_draw_buffers = 1;
   }

   key->inputs_available = filter_fp_input_mask(inputs_referenced, ctx);

   /* compute size of state key, ignoring unused texture units */
   keySize = sizeof(*key) - sizeof(key->unit)
      + key->nr_enabled_units * sizeof(key->unit[0]);

   return keySize;
}


/** State used to build the fragment program:
 */
struct texenv_fragment_program {
   nir_builder *b;
   struct gl_program_parameter_list *state_params;

   struct state_key *state;

   nir_variable *sampler_vars[MAX_TEXTURE_COORD_UNITS];

   nir_def *src_texture[MAX_TEXTURE_COORD_UNITS];
   /* ssa-def containing each texture unit's sampled texture color,
    * else NULL.
    */

   nir_def *src_previous;   /**< Color from previous stage */
};

static nir_variable *
register_state_var(struct texenv_fragment_program *p,
                   gl_state_index s0,
                   gl_state_index s1,
                   gl_state_index s2,
                   gl_state_index s3,
                   const struct glsl_type *type)
{
   gl_state_index16 tokens[STATE_LENGTH];
   tokens[0] = s0;
   tokens[1] = s1;
   tokens[2] = s2;
   tokens[3] = s3;
   nir_variable *var = nir_find_state_variable(p->b->shader, tokens);
   if (var)
      return var;

   int loc = _mesa_add_state_reference(p->state_params, tokens);

   char *name = _mesa_program_state_string(tokens);
   var = nir_variable_create(p->b->shader, nir_var_uniform, type, name);
   free(name);

   var->num_state_slots = 1;
   var->state_slots = ralloc_array(var, nir_state_slot, 1);
   var->data.driver_location = loc;
   memcpy(var->state_slots[0].tokens, tokens,
          sizeof(var->state_slots[0].tokens));

   p->b->shader->num_uniforms++;
   return var;
}

static nir_def *
load_state_var(struct texenv_fragment_program *p,
               gl_state_index s0,
               gl_state_index s1,
               gl_state_index s2,
               gl_state_index s3,
               const struct glsl_type *type)
{
   nir_variable *var = register_state_var(p, s0, s1, s2, s3, type);
   return nir_load_var(p->b, var);
}

static nir_def *
load_input(struct texenv_fragment_program *p, gl_varying_slot slot,
           const struct glsl_type *type)
{
   nir_variable *var =
      nir_get_variable_with_location(p->b->shader,
                                     nir_var_shader_in,
                                     slot,
                                     type);
   var->data.interpolation = INTERP_MODE_NONE;
   return nir_load_var(p->b, var);
}

static nir_def *
get_current_attrib(struct texenv_fragment_program *p, GLuint attrib)
{
   return load_state_var(p, STATE_CURRENT_ATTRIB_MAYBE_VP_CLAMPED,
                         attrib, 0, 0,
                         glsl_vec4_type());
}

static nir_def *
get_gl_Color(struct texenv_fragment_program *p)
{
   if (p->state->inputs_available & VARYING_BIT_COL0) {
      return load_input(p, VARYING_SLOT_COL0, glsl_vec4_type());
   } else {
      return get_current_attrib(p, VERT_ATTRIB_COLOR0);
   }
}

static nir_def *
get_source(struct texenv_fragment_program *p,
           GLuint src, GLuint unit)
{
   switch (src) {
   case TEXENV_SRC_TEXTURE:
      return p->src_texture[unit];

   case TEXENV_SRC_TEXTURE0:
   case TEXENV_SRC_TEXTURE1:
   case TEXENV_SRC_TEXTURE2:
   case TEXENV_SRC_TEXTURE3:
   case TEXENV_SRC_TEXTURE4:
   case TEXENV_SRC_TEXTURE5:
   case TEXENV_SRC_TEXTURE6:
   case TEXENV_SRC_TEXTURE7:
      return p->src_texture[src - TEXENV_SRC_TEXTURE0];

   case TEXENV_SRC_CONSTANT:
      return load_state_var(p, STATE_TEXENV_COLOR,
                            unit, 0, 0,
                            glsl_vec4_type());

   case TEXENV_SRC_PRIMARY_COLOR:
      return get_gl_Color(p);

   case TEXENV_SRC_ZERO:
      return nir_imm_zero(p->b, 4, 32);

   case TEXENV_SRC_ONE:
      return nir_imm_vec4(p->b, 1.0f, 1.0f, 1.0f, 1.0f);

   case TEXENV_SRC_PREVIOUS:
      if (!p->src_previous) {
         return get_gl_Color(p);
      } else {
         return p->src_previous;
      }

   default:
      assert(0);
      return NULL;
   }
}

static nir_def *
emit_combine_source(struct texenv_fragment_program *p,
                    GLuint unit,
                    GLuint source,
                    GLuint operand)
{
   nir_def *src;

   src = get_source(p, source, unit);

   switch (operand) {
   case TEXENV_OPR_ONE_MINUS_COLOR:
      return nir_fsub_imm(p->b, 1.0, src);

   case TEXENV_OPR_ALPHA:
      return src->num_components == 1 ? src : nir_channel(p->b, src, 3);

   case TEXENV_OPR_ONE_MINUS_ALPHA: {
      nir_def *scalar =
         src->num_components == 1 ? src : nir_channel(p->b, src, 3);

      return nir_fsub_imm(p->b, 1.0, scalar);
   }

   case TEXENV_OPR_COLOR:
      return src;

   default:
      assert(0);
      return src;
   }
}

/**
 * Check if the RGB and Alpha sources and operands match for the given
 * texture unit's combinder state.  When the RGB and A sources and
 * operands match, we can emit fewer instructions.
 */
static GLboolean args_match( const struct state_key *key, GLuint unit )
{
   GLuint i, numArgs = key->unit[unit].NumArgsRGB;

   for (i = 0; i < numArgs; i++) {
      if (key->unit[unit].ArgsA[i].Source != key->unit[unit].ArgsRGB[i].Source)
         return GL_FALSE;

      switch (key->unit[unit].ArgsA[i].Operand) {
      case TEXENV_OPR_ALPHA:
         switch (key->unit[unit].ArgsRGB[i].Operand) {
         case TEXENV_OPR_COLOR:
         case TEXENV_OPR_ALPHA:
            break;
         default:
            return GL_FALSE;
         }
         break;
      case TEXENV_OPR_ONE_MINUS_ALPHA:
         switch (key->unit[unit].ArgsRGB[i].Operand) {
         case TEXENV_OPR_ONE_MINUS_COLOR:
         case TEXENV_OPR_ONE_MINUS_ALPHA:
            break;
         default:
            return GL_FALSE;
         }
         break;
      default:
         return GL_FALSE;        /* impossible */
      }
   }

   return GL_TRUE;
}

static nir_def *
smear(nir_builder *b, nir_def *val)
{
   if (val->num_components != 1)
      return val;

   return nir_replicate(b, val, 4);
}

static nir_def *
emit_combine(struct texenv_fragment_program *p,
             GLuint unit,
             GLuint nr,
             GLuint mode,
             const struct gl_tex_env_argument *opt)
{
   nir_def *src[MAX_COMBINER_TERMS];
   nir_def *tmp0, *tmp1;
   GLuint i;

   assert(nr <= MAX_COMBINER_TERMS);

   for (i = 0; i < nr; i++)
      src[i] = emit_combine_source( p, unit, opt[i].Source, opt[i].Operand );

   switch (mode) {
   case TEXENV_MODE_REPLACE:
      return src[0];

   case TEXENV_MODE_MODULATE:
      return nir_fmul(p->b, src[0], src[1]);

   case TEXENV_MODE_ADD:
      return nir_fadd(p->b, src[0], src[1]);

   case TEXENV_MODE_ADD_SIGNED:
      return nir_fadd_imm(p->b, nir_fadd(p->b, src[0], src[1]), -0.5f);

   case TEXENV_MODE_INTERPOLATE:
      return nir_flrp(p->b, src[1], src[0], src[2]);

   case TEXENV_MODE_SUBTRACT:
      return nir_fsub(p->b, src[0], src[1]);

   case TEXENV_MODE_DOT3_RGBA:
   case TEXENV_MODE_DOT3_RGBA_EXT:
   case TEXENV_MODE_DOT3_RGB_EXT:
   case TEXENV_MODE_DOT3_RGB:
      tmp0 = nir_fadd_imm(p->b, nir_fmul_imm(p->b, src[0], 2.0f), -1.0f);
      tmp1 = nir_fadd_imm(p->b, nir_fmul_imm(p->b, src[1], 2.0f), -1.0f);
      return nir_fdot3(p->b, smear(p->b, tmp0), smear(p->b, tmp1));

   case TEXENV_MODE_MODULATE_ADD_ATI:
      return nir_fmad(p->b, src[0], src[2], src[1]);

   case TEXENV_MODE_MODULATE_SIGNED_ADD_ATI:
      return nir_fadd_imm(p->b,
                          nir_fadd(p->b,
                                   nir_fmul(p->b, src[0], src[2]),
                                   src[1]),
                          -0.5f);

   case TEXENV_MODE_MODULATE_SUBTRACT_ATI:
      return nir_fsub(p->b, nir_fmul(p->b, src[0], src[2]), src[1]);

   case TEXENV_MODE_ADD_PRODUCTS_NV:
      return nir_fadd(p->b, nir_fmul(p->b, src[0], src[1]),
                            nir_fmul(p->b, src[2], src[3]));

   case TEXENV_MODE_ADD_PRODUCTS_SIGNED_NV:
      return nir_fadd_imm(p->b,
                          nir_fadd(p->b,
                                   nir_fmul(p->b, src[0], src[1]),
                                   nir_fmul(p->b, src[2], src[3])),
                          -0.5f);
   default:
      assert(0);
      return src[0];
   }
}

/**
 * Generate instructions for one texture unit's env/combiner mode.
 */
static nir_def *
emit_texenv(struct texenv_fragment_program *p, GLuint unit)
{
   const struct state_key *key = p->state;
   GLboolean rgb_saturate, alpha_saturate;
   GLuint rgb_shift, alpha_shift;

   if (!key->unit[unit].enabled) {
      return get_source(p, TEXENV_SRC_PREVIOUS, 0);
   }

   switch (key->unit[unit].ModeRGB) {
   case TEXENV_MODE_DOT3_RGB_EXT:
      alpha_shift = key->unit[unit].ScaleShiftA;
      rgb_shift = 0;
      break;
   case TEXENV_MODE_DOT3_RGBA_EXT:
      alpha_shift = 0;
      rgb_shift = 0;
      break;
   default:
      rgb_shift = key->unit[unit].ScaleShiftRGB;
      alpha_shift = key->unit[unit].ScaleShiftA;
      break;
   }

   /* If we'll do rgb/alpha shifting don't saturate in emit_combine().
    * We don't want to clamp twice.
    */
   if (rgb_shift)
      rgb_saturate = GL_FALSE;  /* saturate after rgb shift */
   else if (need_saturate(key->unit[unit].ModeRGB))
      rgb_saturate = GL_TRUE;
   else
      rgb_saturate = GL_FALSE;

   if (alpha_shift)
      alpha_saturate = GL_FALSE;  /* saturate after alpha shift */
   else if (need_saturate(key->unit[unit].ModeA))
      alpha_saturate = GL_TRUE;
   else
      alpha_saturate = GL_FALSE;

   nir_def *val;

   /* Emit the RGB and A combine ops
    */
   if (key->unit[unit].ModeRGB == key->unit[unit].ModeA &&
       args_match(key, unit)) {
      val = emit_combine(p, unit,
                         key->unit[unit].NumArgsRGB,
                         key->unit[unit].ModeRGB,
                         key->unit[unit].ArgsRGB);
      val = smear(p->b, val);
      if (rgb_saturate)
         val = nir_fsat(p->b, val);
   }
   else if (key->unit[unit].ModeRGB == TEXENV_MODE_DOT3_RGBA_EXT ||
            key->unit[unit].ModeRGB == TEXENV_MODE_DOT3_RGBA) {
      val = emit_combine(p, unit,
                         key->unit[unit].NumArgsRGB,
                         key->unit[unit].ModeRGB,
                         key->unit[unit].ArgsRGB);
      val = smear(p->b, val);
      if (rgb_saturate)
         val = nir_fsat(p->b, val);
   }
   else {
      /* Need to do something to stop from re-emitting identical
       * argument calculations here:
       */
      val = emit_combine(p, unit,
                         key->unit[unit].NumArgsRGB,
                         key->unit[unit].ModeRGB,
                         key->unit[unit].ArgsRGB);
      val = smear(p->b, val);
      if (rgb_saturate)
         val = nir_fsat(p->b, val);
      nir_def *rgb = val;

      val = emit_combine(p, unit,
                         key->unit[unit].NumArgsA,
                         key->unit[unit].ModeA,
                         key->unit[unit].ArgsA);

      if (val->num_components != 1)
         val = nir_channel(p->b, val, 3);

      if (alpha_saturate)
         val = nir_fsat(p->b, val);
      nir_def *a = val;

      val = nir_vector_insert_imm(p->b, rgb, a, 3);
   }

   /* Deal with the final shift:
    */
   if (alpha_shift || rgb_shift) {
      nir_def *shift;

      if (rgb_shift == alpha_shift) {
         shift = nir_imm_float(p->b, (float)(1 << rgb_shift));
      }
      else {
         shift = nir_imm_vec4(p->b,
            (float)(1 << rgb_shift),
            (float)(1 << rgb_shift),
            (float)(1 << rgb_shift),
            (float)(1 << alpha_shift));
      }

      return nir_fsat(p->b, nir_fmul(p->b, val, shift));
   }
   else
      return val;
}


/**
 * Generate instruction for getting a texture source term.
 */
static void
load_texture(struct texenv_fragment_program *p, GLuint unit)
{
   if (p->src_texture[unit])
      return;

   const GLuint texTarget = p->state->unit[unit].source_index;
   nir_def *texcoord;

   if (!(p->state->inputs_available & (VARYING_BIT_TEX0 << unit))) {
      texcoord = get_current_attrib(p, VERT_ATTRIB_TEX0 + unit);
   } else {
      texcoord = load_input(p,
         VARYING_SLOT_TEX0 + unit,
         glsl_vec4_type());
   }

   if (!p->state->unit[unit].enabled) {
      p->src_texture[unit] = nir_imm_zero(p->b, 4, 32);
      return ;
   }

   unsigned num_srcs = 4;
   if (p->state->unit[unit].shadow)
      num_srcs++;

   nir_tex_instr *tex = nir_tex_instr_create(p->b->shader, num_srcs);
   tex->op = nir_texop_tex;
   tex->dest_type = nir_type_float32;
   tex->texture_index = unit;
   tex->sampler_index = unit;

   tex->sampler_dim =
      _mesa_texture_index_to_sampler_dim(texTarget,
                                         &tex->is_array);

   tex->coord_components =
      glsl_get_sampler_dim_coordinate_components(tex->sampler_dim);
   if (tex->is_array)
      tex->coord_components++;

   nir_variable *var = p->sampler_vars[unit];
   if (!var) {
      const struct glsl_type *sampler_type =
         glsl_sampler_type(tex->sampler_dim,
                           p->state->unit[unit].shadow,
                           tex->is_array, GLSL_TYPE_FLOAT);

      var = nir_variable_create(p->b->shader, nir_var_uniform,
                                sampler_type,
                                ralloc_asprintf(p->b->shader,
                                                "sampler_%d", unit));
      var->data.binding = unit;
      var->data.explicit_binding = true;

      p->sampler_vars[unit] = var;
   }

   nir_deref_instr *deref = nir_build_deref_var(p->b, var);
   tex->src[0] = nir_tex_src_for_ssa(nir_tex_src_texture_deref,
                                     &deref->def);
   tex->src[1] = nir_tex_src_for_ssa(nir_tex_src_sampler_deref,
                                     &deref->def);

   nir_def *src2 =
      nir_channels(p->b, texcoord,
                   nir_component_mask(tex->coord_components));
   tex->src[2] = nir_tex_src_for_ssa(nir_tex_src_coord, src2);

   tex->src[3] = nir_tex_src_for_ssa(nir_tex_src_projector,
                                     nir_channel(p->b, texcoord, 3));

   if (p->state->unit[unit].shadow) {
      tex->is_shadow = true;
      nir_def *src4 =
         nir_channel(p->b, texcoord, tex->coord_components);
      tex->src[4] = nir_tex_src_for_ssa(nir_tex_src_comparator, src4);
   }

   nir_def_init(&tex->instr, &tex->def, 4, 32);
   p->src_texture[unit] = &tex->def;

   nir_builder_instr_insert(p->b, &tex->instr);
   BITSET_SET(p->b->shader->info.textures_used, unit);
   BITSET_SET(p->b->shader->info.samplers_used, unit);
}

static void
load_texenv_source(struct texenv_fragment_program *p,
                   GLuint src, GLuint unit)
{
   switch (src) {
   case TEXENV_SRC_TEXTURE:
      load_texture(p, unit);
      break;

   case TEXENV_SRC_TEXTURE0:
   case TEXENV_SRC_TEXTURE1:
   case TEXENV_SRC_TEXTURE2:
   case TEXENV_SRC_TEXTURE3:
   case TEXENV_SRC_TEXTURE4:
   case TEXENV_SRC_TEXTURE5:
   case TEXENV_SRC_TEXTURE6:
   case TEXENV_SRC_TEXTURE7:
      load_texture(p, src - TEXENV_SRC_TEXTURE0);
      break;

   default:
      /* not a texture src - do nothing */
      break;
   }
}


/**
 * Generate instructions for loading all texture source terms.
 */
static GLboolean
load_texunit_sources(struct texenv_fragment_program *p, GLuint unit)
{
   const struct state_key *key = p->state;
   GLuint i;

   for (i = 0; i < key->unit[unit].NumArgsRGB; i++) {
      load_texenv_source( p, key->unit[unit].ArgsRGB[i].Source, unit );
   }

   for (i = 0; i < key->unit[unit].NumArgsA; i++) {
      load_texenv_source( p, key->unit[unit].ArgsA[i].Source, unit );
   }

   return GL_TRUE;
}

static void
emit_instructions(struct texenv_fragment_program *p)
{
   struct state_key *key = p->state;
   GLuint unit;

   if (key->nr_enabled_units) {
      /* First pass - to support texture_env_crossbar, first identify
       * all referenced texture sources and emit texld instructions
       * for each:
       */
      for (unit = 0; unit < key->nr_enabled_units; unit++)
         if (key->unit[unit].enabled) {
            load_texunit_sources(p, unit);
         }

      /* Second pass - emit combine instructions to build final color:
       */
      for (unit = 0; unit < key->nr_enabled_units; unit++) {
         if (key->unit[unit].enabled) {
            p->src_previous = emit_texenv(p, unit);
         }
      }
   }

   nir_def *cf = get_source(p, TEXENV_SRC_PREVIOUS, 0);

   if (key->separate_specular) {
      nir_def *spec_result = cf;

      nir_def *secondary;
      if (p->state->inputs_available & VARYING_BIT_COL1)
         secondary = load_input(p, VARYING_SLOT_COL1, glsl_vec4_type());
      else
         secondary = get_current_attrib(p, VERT_ATTRIB_COLOR1);

      secondary = nir_vector_insert_imm(p->b, secondary,
                                        nir_imm_zero(p->b, 1, 32), 3);
      cf = nir_fadd(p->b, spec_result, secondary);
   }

   const char *name =
      gl_frag_result_name(FRAG_RESULT_COLOR);
   nir_variable *var =
      nir_variable_create(p->b->shader, nir_var_shader_out,
                          glsl_vec4_type(), name);

   var->data.location = FRAG_RESULT_COLOR;
   var->data.driver_location = p->b->shader->num_outputs++;

   p->b->shader->info.outputs_written |= BITFIELD64_BIT(FRAG_RESULT_COLOR);

   nir_store_var(p->b, var, cf, 0xf);
}

/**
 * Generate a new fragment program which implements the context's
 * current texture env/combine mode.
 */
static nir_shader *
create_new_program(struct state_key *key,
                   struct gl_program *program,
                   const nir_shader_compiler_options *options)
{
   struct texenv_fragment_program p;

   memset(&p, 0, sizeof(p));
   p.state = key;

   program->Parameters = _mesa_new_parameter_list();
   p.state_params = _mesa_new_parameter_list();

   nir_builder b = nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,
                                                  options,
                                                  "ff-fs");

   nir_shader *s = b.shader;

   s->info.separate_shader = true;
   s->info.subgroup_size = SUBGROUP_SIZE_UNIFORM;

   p.b = &b;

   if (key->num_draw_buffers)
      emit_instructions(&p);

   nir_validate_shader(b.shader, "after generating ff-vertex shader");

   if (key->fog_mode)
      NIR_PASS_V(b.shader, st_nir_lower_fog, key->fog_mode, p.state_params);

   _mesa_add_separate_state_parameters(program, p.state_params);
   _mesa_free_parameter_list(p.state_params);

   return s;
}

/**
 * Return a fragment program which implements the current
 * fixed-function texture, fog and color-sum operations.
 */
struct gl_program *
_mesa_get_fixed_func_fragment_program(struct gl_context *ctx)
{
   struct gl_program *prog;
   struct state_key key;
   GLuint keySize;

   keySize = make_state_key(ctx, &key);

   prog = (struct gl_program *)
      _mesa_search_program_cache(ctx->FragmentProgram.Cache,
                                 &key, keySize);

   if (!prog) {
      prog = ctx->Driver.NewProgram(ctx, MESA_SHADER_FRAGMENT, 0, false);
      if (!prog)
         return NULL;

      const struct nir_shader_compiler_options *options =
         st_get_nir_compiler_options(ctx->st, MESA_SHADER_FRAGMENT);

      nir_shader *s =
         create_new_program(&key, prog, options);

      prog->state.type = PIPE_SHADER_IR_NIR;
      prog->nir = s;

      prog->SamplersUsed = s->info.samplers_used[0];

      /* default mapping from samplers to texture units */
      for (unsigned i = 0; i < MAX_SAMPLERS; i++)
         prog->SamplerUnits[i] = i;

      st_program_string_notify(ctx, GL_FRAGMENT_PROGRAM_ARB, prog);

      _mesa_program_cache_insert(ctx, ctx->FragmentProgram.Cache,
                                 &key, keySize, prog);
   }

   return prog;
}
