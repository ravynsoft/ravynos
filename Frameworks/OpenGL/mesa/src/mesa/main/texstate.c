/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2007  Brian Paul   All Rights Reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file texstate.c
 *
 * Texture state handling.
 */

#include <stdio.h>
#include "util/glheader.h"
#include "bufferobj.h"
#include "context.h"
#include "enums.h"
#include "macros.h"
#include "texobj.h"
#include "teximage.h"
#include "texstate.h"
#include "mtypes.h"
#include "state.h"
#include "util/bitscan.h"
#include "util/bitset.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"

/**
 * Default texture combine environment state.  This is used to initialize
 * a context's texture units and as the basis for converting "classic"
 * texture environmnets to ARB_texture_env_combine style values.
 */
static const struct gl_tex_env_combine_state default_combine_state = {
   GL_MODULATE, GL_MODULATE,
   { GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT, GL_CONSTANT },
   { GL_TEXTURE, GL_PREVIOUS, GL_CONSTANT, GL_CONSTANT },
   { GL_SRC_COLOR, GL_SRC_COLOR, GL_SRC_ALPHA, GL_SRC_ALPHA },
   { GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA, GL_SRC_ALPHA },
   0, 0,
   2, 2
};



/**
 * Used by glXCopyContext to copy texture state from one context to another.
 */
void
_mesa_copy_texture_state( const struct gl_context *src, struct gl_context *dst )
{
   GLuint u, tex;

   assert(src);
   assert(dst);

   dst->Texture.CurrentUnit = src->Texture.CurrentUnit;

   /* per-unit state */
   for (u = 0; u < src->Const.MaxCombinedTextureImageUnits; u++) {
      dst->Texture.Unit[u].LodBias = src->Texture.Unit[u].LodBias;
      dst->Texture.Unit[u].LodBiasQuantized = src->Texture.Unit[u].LodBiasQuantized;

      /*
       * XXX strictly speaking, we should compare texture names/ids and
       * bind textures in the dest context according to id.  For now, only
       * copy bindings if the contexts share the same pool of textures to
       * avoid refcounting bugs.
       */
      if (dst->Shared == src->Shared) {
         /* copy texture object bindings, not contents of texture objects */
         _mesa_lock_context_textures(dst);

         for (tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
            _mesa_reference_texobj(&dst->Texture.Unit[u].CurrentTex[tex],
                                   src->Texture.Unit[u].CurrentTex[tex]);
            if (src->Texture.Unit[u].CurrentTex[tex]) {
               dst->Texture.NumCurrentTexUsed =
                  MAX2(dst->Texture.NumCurrentTexUsed, u + 1);
            }
         }
         dst->Texture.Unit[u]._BoundTextures = src->Texture.Unit[u]._BoundTextures;
         _mesa_unlock_context_textures(dst);
      }
   }

   for (u = 0; u < src->Const.MaxTextureCoordUnits; u++) {
      dst->Texture.FixedFuncUnit[u].Enabled = src->Texture.FixedFuncUnit[u].Enabled;
      dst->Texture.FixedFuncUnit[u].EnvMode = src->Texture.FixedFuncUnit[u].EnvMode;
      COPY_4V(dst->Texture.FixedFuncUnit[u].EnvColor, src->Texture.FixedFuncUnit[u].EnvColor);
      dst->Texture.FixedFuncUnit[u].TexGenEnabled = src->Texture.FixedFuncUnit[u].TexGenEnabled;
      dst->Texture.FixedFuncUnit[u].GenS = src->Texture.FixedFuncUnit[u].GenS;
      dst->Texture.FixedFuncUnit[u].GenT = src->Texture.FixedFuncUnit[u].GenT;
      dst->Texture.FixedFuncUnit[u].GenR = src->Texture.FixedFuncUnit[u].GenR;
      dst->Texture.FixedFuncUnit[u].GenQ = src->Texture.FixedFuncUnit[u].GenQ;
      memcpy(dst->Texture.FixedFuncUnit[u].ObjectPlane,
             src->Texture.FixedFuncUnit[u].ObjectPlane,
             sizeof(src->Texture.FixedFuncUnit[u].ObjectPlane));
      memcpy(dst->Texture.FixedFuncUnit[u].EyePlane,
             src->Texture.FixedFuncUnit[u].EyePlane,
             sizeof(src->Texture.FixedFuncUnit[u].EyePlane));

      /* GL_EXT_texture_env_combine */
      dst->Texture.FixedFuncUnit[u].Combine = src->Texture.FixedFuncUnit[u].Combine;
   }
}


/*
 * For debugging
 */
void
_mesa_print_texunit_state( struct gl_context *ctx, GLuint unit )
{
   const struct gl_fixedfunc_texture_unit *texUnit = ctx->Texture.FixedFuncUnit + unit;
   printf("Texture Unit %d\n", unit);
   printf("  GL_TEXTURE_ENV_MODE = %s\n", _mesa_enum_to_string(texUnit->EnvMode));
   printf("  GL_COMBINE_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.ModeRGB));
   printf("  GL_COMBINE_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.ModeA));
   printf("  GL_SOURCE0_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceRGB[0]));
   printf("  GL_SOURCE1_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceRGB[1]));
   printf("  GL_SOURCE2_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceRGB[2]));
   printf("  GL_SOURCE0_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceA[0]));
   printf("  GL_SOURCE1_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceA[1]));
   printf("  GL_SOURCE2_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.SourceA[2]));
   printf("  GL_OPERAND0_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandRGB[0]));
   printf("  GL_OPERAND1_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandRGB[1]));
   printf("  GL_OPERAND2_RGB = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandRGB[2]));
   printf("  GL_OPERAND0_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandA[0]));
   printf("  GL_OPERAND1_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandA[1]));
   printf("  GL_OPERAND2_ALPHA = %s\n", _mesa_enum_to_string(texUnit->Combine.OperandA[2]));
   printf("  GL_RGB_SCALE = %d\n", 1 << texUnit->Combine.ScaleShiftRGB);
   printf("  GL_ALPHA_SCALE = %d\n", 1 << texUnit->Combine.ScaleShiftA);
   printf("  GL_TEXTURE_ENV_COLOR = (%f, %f, %f, %f)\n", texUnit->EnvColor[0], texUnit->EnvColor[1], texUnit->EnvColor[2], texUnit->EnvColor[3]);
}



/**********************************************************************/
/*                       Texture Environment                          */
/**********************************************************************/

/**
 * Convert "classic" texture environment to ARB_texture_env_combine style
 * environments.
 *
 * \param state  texture_env_combine state vector to be filled-in.
 * \param mode   Classic texture environment mode (i.e., \c GL_REPLACE,
 *               \c GL_BLEND, \c GL_DECAL, etc.).
 * \param texBaseFormat  Base format of the texture associated with the
 *               texture unit.
 */
static void
calculate_derived_texenv( struct gl_tex_env_combine_state *state,
			  GLenum mode, GLenum texBaseFormat )
{
   GLenum mode_rgb;
   GLenum mode_a;

   *state = default_combine_state;

   switch (texBaseFormat) {
   case GL_ALPHA:
      state->SourceRGB[0] = GL_PREVIOUS;
      break;

   case GL_LUMINANCE_ALPHA:
   case GL_INTENSITY:
   case GL_RGBA:
      break;

   case GL_LUMINANCE:
   case GL_RED:
   case GL_RG:
   case GL_RGB:
   case GL_YCBCR_MESA:
      state->SourceA[0] = GL_PREVIOUS;
      break;

   default:
      _mesa_problem(NULL,
                    "Invalid texBaseFormat 0x%x in calculate_derived_texenv",
                    texBaseFormat);
      return;
   }

   if (mode == GL_REPLACE_EXT)
      mode = GL_REPLACE;

   switch (mode) {
   case GL_REPLACE:
   case GL_MODULATE:
      mode_rgb = (texBaseFormat == GL_ALPHA) ? GL_REPLACE : mode;
      mode_a   = mode;
      break;

   case GL_DECAL:
      mode_rgb = GL_INTERPOLATE;
      mode_a   = GL_REPLACE;

      state->SourceA[0] = GL_PREVIOUS;

      /* Having alpha / luminance / intensity textures replace using the
       * incoming fragment color matches the definition in NV_texture_shader.
       * The 1.5 spec simply marks these as "undefined".
       */
      switch (texBaseFormat) {
      case GL_ALPHA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_INTENSITY:
	 state->SourceRGB[0] = GL_PREVIOUS;
	 break;
      case GL_RED:
      case GL_RG:
      case GL_RGB:
      case GL_YCBCR_MESA:
	 mode_rgb = GL_REPLACE;
	 break;
      case GL_RGBA:
	 state->SourceRGB[2] = GL_TEXTURE;
	 break;
      }
      break;

   case GL_BLEND:
      mode_rgb = GL_INTERPOLATE;
      mode_a   = GL_MODULATE;

      switch (texBaseFormat) {
      case GL_ALPHA:
	 mode_rgb = GL_REPLACE;
	 break;
      case GL_INTENSITY:
	 mode_a = GL_INTERPOLATE;
	 state->SourceA[0] = GL_CONSTANT;
	 state->OperandA[2] = GL_SRC_ALPHA;
	 FALLTHROUGH;
      case GL_LUMINANCE:
      case GL_RED:
      case GL_RG:
      case GL_RGB:
      case GL_LUMINANCE_ALPHA:
      case GL_RGBA:
      case GL_YCBCR_MESA:
	 state->SourceRGB[2] = GL_TEXTURE;
	 state->SourceA[2]   = GL_TEXTURE;
	 state->SourceRGB[0] = GL_CONSTANT;
	 state->OperandRGB[2] = GL_SRC_COLOR;
	 break;
      }
      break;

   case GL_ADD:
      mode_rgb = (texBaseFormat == GL_ALPHA) ? GL_REPLACE : GL_ADD;
      mode_a   = (texBaseFormat == GL_INTENSITY) ? GL_ADD : GL_MODULATE;
      break;

   default:
      _mesa_problem(NULL,
                    "Invalid texture env mode 0x%x in calculate_derived_texenv",
                    mode);
      return;
   }

   state->ModeRGB = (state->SourceRGB[0] != GL_PREVIOUS)
       ? mode_rgb : GL_REPLACE;
   state->ModeA   = (state->SourceA[0]   != GL_PREVIOUS)
       ? mode_a   : GL_REPLACE;
}


/* GL_ARB_multitexture */
static ALWAYS_INLINE void
active_texture(GLenum texture, bool no_error)
{
   const GLuint texUnit = texture - GL_TEXTURE0;

   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glActiveTexture %s\n",
                  _mesa_enum_to_string(texture));

   if (ctx->Texture.CurrentUnit == texUnit)
      return;

   if (!no_error) {
      GLuint k = _mesa_max_tex_unit(ctx);

      assert(k <= ARRAY_SIZE(ctx->Texture.Unit));

      if (texUnit >= k) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glActiveTexture(texture=%s)",
                     _mesa_enum_to_string(texture));
         return;
      }
   }


   /* The below flush call seems useless because
    * gl_context::Texture::CurrentUnit is not used by
    * _mesa_update_texture_state() and friends.
    *
    * However removing the flush
    * introduced some blinking textures in UT2004. More investigation is
    * needed to find the root cause.
    *
    * https://bugs.freedesktop.org/show_bug.cgi?id=105436
    */
   FLUSH_VERTICES(ctx, _NEW_TEXTURE_STATE, GL_TEXTURE_BIT);

   ctx->Texture.CurrentUnit = texUnit;
   if (ctx->Transform.MatrixMode == GL_TEXTURE) {
      /* update current stack pointer */
      ctx->CurrentStack = &ctx->TextureMatrixStack[texUnit];
   }
}


void GLAPIENTRY
_mesa_ActiveTexture_no_error(GLenum texture)
{
   active_texture(texture, true);
}


void GLAPIENTRY
_mesa_ActiveTexture(GLenum texture)
{
   active_texture(texture, false);
}


/* GL_ARB_multitexture */
void GLAPIENTRY
_mesa_ClientActiveTexture(GLenum texture)
{
   GET_CURRENT_CONTEXT(ctx);
   GLuint texUnit = texture - GL_TEXTURE0;

   if (MESA_VERBOSE & (VERBOSE_API | VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glClientActiveTexture %s\n",
                  _mesa_enum_to_string(texture));

   if (ctx->Array.ActiveTexture == texUnit)
      return;

   if (texUnit >= ctx->Const.MaxTextureCoordUnits) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glClientActiveTexture(texture=%s)",
                  _mesa_enum_to_string(texture));
      return;
   }

   /* Don't flush vertices. This is a "latched" state. */
   ctx->Array.ActiveTexture = texUnit;
}



/**********************************************************************/
/*****                    State management                        *****/
/**********************************************************************/


/**
 * \note This routine refers to derived texture attribute values to
 * compute the ENABLE_TEXMAT flags, but is only called on
 * _NEW_TEXTURE_MATRIX.  On changes to _NEW_TEXTURE_OBJECT/STATE,
 * the ENABLE_TEXMAT flags are updated by _mesa_update_textures(), below.
 *
 * \param ctx GL context.
 */
GLbitfield
_mesa_update_texture_matrices(struct gl_context *ctx)
{
   GLuint u;
   GLbitfield old_texmat_enabled = ctx->Texture._TexMatEnabled;

   ctx->Texture._TexMatEnabled = 0x0;

   for (u = 0; u < ctx->Const.MaxTextureCoordUnits; u++) {
      assert(u < ARRAY_SIZE(ctx->TextureMatrixStack));
      if (_math_matrix_is_dirty(ctx->TextureMatrixStack[u].Top)) {
	 _math_matrix_analyse( ctx->TextureMatrixStack[u].Top );

	 if (ctx->Texture.Unit[u]._Current &&
	     ctx->TextureMatrixStack[u].Top->type != MATRIX_IDENTITY)
	    ctx->Texture._TexMatEnabled |= ENABLE_TEXMAT(u);
      }
   }

   if (old_texmat_enabled != ctx->Texture._TexMatEnabled)
      return _NEW_FF_VERT_PROGRAM | _NEW_FF_FRAG_PROGRAM;

   return 0;
}


/**
 * Translate GL combiner state into a MODE_x value
 */
static uint32_t
tex_combine_translate_mode(GLenum envMode, GLenum mode)
{
   switch (mode) {
   case GL_REPLACE: return TEXENV_MODE_REPLACE;
   case GL_MODULATE: return TEXENV_MODE_MODULATE;
   case GL_ADD:
      if (envMode == GL_COMBINE4_NV)
	 return TEXENV_MODE_ADD_PRODUCTS_NV;
      else
	 return TEXENV_MODE_ADD;
   case GL_ADD_SIGNED:
      if (envMode == GL_COMBINE4_NV)
	 return TEXENV_MODE_ADD_PRODUCTS_SIGNED_NV;
      else
	 return TEXENV_MODE_ADD_SIGNED;
   case GL_INTERPOLATE: return TEXENV_MODE_INTERPOLATE;
   case GL_SUBTRACT: return TEXENV_MODE_SUBTRACT;
   case GL_DOT3_RGB: return TEXENV_MODE_DOT3_RGB;
   case GL_DOT3_RGB_EXT: return TEXENV_MODE_DOT3_RGB_EXT;
   case GL_DOT3_RGBA: return TEXENV_MODE_DOT3_RGBA;
   case GL_DOT3_RGBA_EXT: return TEXENV_MODE_DOT3_RGBA_EXT;
   case GL_MODULATE_ADD_ATI: return TEXENV_MODE_MODULATE_ADD_ATI;
   case GL_MODULATE_SIGNED_ADD_ATI: return TEXENV_MODE_MODULATE_SIGNED_ADD_ATI;
   case GL_MODULATE_SUBTRACT_ATI: return TEXENV_MODE_MODULATE_SUBTRACT_ATI;
   default:
      unreachable("Invalid TexEnv Combine mode");
   }
}


static uint8_t
tex_combine_translate_source(GLenum src)
{
   switch (src) {
   case GL_TEXTURE0:
   case GL_TEXTURE1:
   case GL_TEXTURE2:
   case GL_TEXTURE3:
   case GL_TEXTURE4:
   case GL_TEXTURE5:
   case GL_TEXTURE6:
   case GL_TEXTURE7: return TEXENV_SRC_TEXTURE0 + (src - GL_TEXTURE0);
   case GL_TEXTURE: return TEXENV_SRC_TEXTURE;
   case GL_PREVIOUS: return TEXENV_SRC_PREVIOUS;
   case GL_PRIMARY_COLOR: return TEXENV_SRC_PRIMARY_COLOR;
   case GL_CONSTANT: return TEXENV_SRC_CONSTANT;
   case GL_ZERO: return TEXENV_SRC_ZERO;
   case GL_ONE: return TEXENV_SRC_ONE;
   default:
      unreachable("Invalid TexEnv Combine argument source");
   }
}


static uint8_t
tex_combine_translate_operand(GLenum operand)
{
   switch (operand) {
   case GL_SRC_COLOR: return TEXENV_OPR_COLOR;
   case GL_ONE_MINUS_SRC_COLOR: return TEXENV_OPR_ONE_MINUS_COLOR;
   case GL_SRC_ALPHA: return TEXENV_OPR_ALPHA;
   case GL_ONE_MINUS_SRC_ALPHA: return TEXENV_OPR_ONE_MINUS_ALPHA;
   default:
      unreachable("Invalid TexEnv Combine argument source");
   }
}


static void
pack_tex_combine(struct gl_fixedfunc_texture_unit *texUnit)
{
   struct gl_tex_env_combine_state *state = texUnit->_CurrentCombine;
   struct gl_tex_env_combine_packed *packed = &texUnit->_CurrentCombinePacked;

   memset(packed, 0, sizeof *packed);

   packed->ModeRGB = tex_combine_translate_mode(texUnit->EnvMode, state->ModeRGB);
   packed->ModeA = tex_combine_translate_mode(texUnit->EnvMode, state->ModeA);
   packed->ScaleShiftRGB = state->ScaleShiftRGB;
   packed->ScaleShiftA = state->ScaleShiftA;
   packed->NumArgsRGB = state->_NumArgsRGB;
   packed->NumArgsA = state->_NumArgsA;

   for (int i = 0; i < state->_NumArgsRGB; ++i)
   {
      packed->ArgsRGB[i].Source = tex_combine_translate_source(state->SourceRGB[i]);
      packed->ArgsRGB[i].Operand = tex_combine_translate_operand(state->OperandRGB[i]);
   }

   for (int i = 0; i < state->_NumArgsA; ++i)
   {
      packed->ArgsA[i].Source = tex_combine_translate_source(state->SourceA[i]);
      packed->ArgsA[i].Operand = tex_combine_translate_operand(state->OperandA[i]);
   }
}


/**
 * Examine texture unit's combine/env state to update derived state.
 */
static void
update_tex_combine(struct gl_context *ctx,
                   struct gl_texture_unit *texUnit,
                   struct gl_fixedfunc_texture_unit *fftexUnit)
{
   struct gl_tex_env_combine_state *combine;

   /* No combiners will apply to this. */
   if (texUnit->_Current->Target == GL_TEXTURE_BUFFER)
      return;

   /* Set the texUnit->_CurrentCombine field to point to the user's combiner
    * state, or the combiner state which is derived from traditional texenv
    * mode.
    */
   if (fftexUnit->EnvMode == GL_COMBINE ||
       fftexUnit->EnvMode == GL_COMBINE4_NV) {
      fftexUnit->_CurrentCombine = & fftexUnit->Combine;
   }
   else {
      const struct gl_texture_object *texObj = texUnit->_Current;
      GLenum format = _mesa_base_tex_image(texObj)->_BaseFormat;

      if (format == GL_DEPTH_COMPONENT || format == GL_DEPTH_STENCIL_EXT) {
         format = texObj->Attrib.DepthMode;
      }
      calculate_derived_texenv(&fftexUnit->_EnvMode, fftexUnit->EnvMode, format);
      fftexUnit->_CurrentCombine = & fftexUnit->_EnvMode;
   }

   combine = fftexUnit->_CurrentCombine;

   /* Determine number of source RGB terms in the combiner function */
   switch (combine->ModeRGB) {
   case GL_REPLACE:
      combine->_NumArgsRGB = 1;
      break;
   case GL_ADD:
   case GL_ADD_SIGNED:
      if (fftexUnit->EnvMode == GL_COMBINE4_NV)
         combine->_NumArgsRGB = 4;
      else
         combine->_NumArgsRGB = 2;
      break;
   case GL_MODULATE:
   case GL_SUBTRACT:
   case GL_DOT3_RGB:
   case GL_DOT3_RGBA:
   case GL_DOT3_RGB_EXT:
   case GL_DOT3_RGBA_EXT:
      combine->_NumArgsRGB = 2;
      break;
   case GL_INTERPOLATE:
   case GL_MODULATE_ADD_ATI:
   case GL_MODULATE_SIGNED_ADD_ATI:
   case GL_MODULATE_SUBTRACT_ATI:
      combine->_NumArgsRGB = 3;
      break;
   default:
      combine->_NumArgsRGB = 0;
      _mesa_problem(ctx, "invalid RGB combine mode in update_texture_state");
      return;
   }

   /* Determine number of source Alpha terms in the combiner function */
   switch (combine->ModeA) {
   case GL_REPLACE:
      combine->_NumArgsA = 1;
      break;
   case GL_ADD:
   case GL_ADD_SIGNED:
      if (fftexUnit->EnvMode == GL_COMBINE4_NV)
         combine->_NumArgsA = 4;
      else
         combine->_NumArgsA = 2;
      break;
   case GL_MODULATE:
   case GL_SUBTRACT:
      combine->_NumArgsA = 2;
      break;
   case GL_INTERPOLATE:
   case GL_MODULATE_ADD_ATI:
   case GL_MODULATE_SIGNED_ADD_ATI:
   case GL_MODULATE_SUBTRACT_ATI:
      combine->_NumArgsA = 3;
      break;
   default:
      combine->_NumArgsA = 0;
      _mesa_problem(ctx, "invalid Alpha combine mode in update_texture_state");
      break;
   }

   pack_tex_combine(fftexUnit);
}

static void
update_texgen(struct gl_context *ctx)
{
   GLuint unit;

   /* Setup texgen for those texture coordinate sets that are in use */
   for (unit = 0; unit < ctx->Const.MaxTextureCoordUnits; unit++) {
      struct gl_fixedfunc_texture_unit *texUnit =
         &ctx->Texture.FixedFuncUnit[unit];

      texUnit->_GenFlags = 0x0;

      if (!(ctx->Texture._EnabledCoordUnits & (1 << unit)))
	 continue;

      if (texUnit->TexGenEnabled) {
	 if (texUnit->TexGenEnabled & S_BIT) {
	    texUnit->_GenFlags |= texUnit->GenS._ModeBit;
	 }
	 if (texUnit->TexGenEnabled & T_BIT) {
	    texUnit->_GenFlags |= texUnit->GenT._ModeBit;
	 }
	 if (texUnit->TexGenEnabled & R_BIT) {
	    texUnit->_GenFlags |= texUnit->GenR._ModeBit;
	 }
	 if (texUnit->TexGenEnabled & Q_BIT) {
	    texUnit->_GenFlags |= texUnit->GenQ._ModeBit;
	 }

	 ctx->Texture._TexGenEnabled |= ENABLE_TEXGEN(unit);
	 ctx->Texture._GenFlags |= texUnit->_GenFlags;
      }

      assert(unit < ARRAY_SIZE(ctx->TextureMatrixStack));
      if (ctx->TextureMatrixStack[unit].Top->type != MATRIX_IDENTITY)
	 ctx->Texture._TexMatEnabled |= ENABLE_TEXMAT(unit);
   }
}

static struct gl_texture_object *
update_single_program_texture(struct gl_context *ctx, struct gl_program *prog,
                              int unit)
{
   gl_texture_index target_index;
   struct gl_texture_unit *texUnit;
   struct gl_texture_object *texObj;
   struct gl_sampler_object *sampler;

   texUnit = &ctx->Texture.Unit[unit];

   /* Note: If more than one bit was set in TexturesUsed[unit], then we should
    * have had the draw call rejected already.  From the GL 4.4 specification,
    * section 7.10 ("Samplers"):
    *
    *     "It is not allowed to have variables of different sampler types
    *      pointing to the same texture image unit within a program
    *      object. This situation can only be detected at the next rendering
    *      command issued which triggers shader invocations, and an
    *      INVALID_OPERATION error will then be generated."
    */
   target_index = ffs(prog->TexturesUsed[unit]) - 1;
   texObj = texUnit->CurrentTex[target_index];

   sampler = texUnit->Sampler ?
      texUnit->Sampler : &texObj->Sampler;

   if (likely(texObj)) {
      if (_mesa_is_texture_complete(texObj, sampler,
                                    ctx->Const.ForceIntegerTexNearest))
         return texObj;

      _mesa_test_texobj_completeness(ctx, texObj);
      if (_mesa_is_texture_complete(texObj, sampler,
                                    ctx->Const.ForceIntegerTexNearest))
         return texObj;
   }

   /* If we've reached this point, we didn't find a complete texture of the
    * shader's target.  From the GL 4.4 core specification, section 11.1.3.5
    * ("Texture Access"):
    *
    *     "If a sampler is used in a shader and the sampler’s associated
    *      texture is not complete, as defined in section 8.17, (0, 0, 0, 1)
    *      will be returned for a non-shadow sampler and 0 for a shadow
    *      sampler."
    *
    * Mesa implements this by creating a hidden texture object with a pixel of
    * that value.
    */
   texObj = _mesa_get_fallback_texture(ctx, target_index, !!(prog->ShadowSamplers & BITFIELD_BIT(unit)));
   assert(texObj);

   return texObj;
}

static inline void
update_single_program_texture_state(struct gl_context *ctx,
                                    struct gl_program *prog,
                                    int unit,
                                    BITSET_WORD *enabled_texture_units)
{
   struct gl_texture_object *texObj;

   texObj = update_single_program_texture(ctx, prog, unit);

   _mesa_reference_texobj(&ctx->Texture.Unit[unit]._Current, texObj);
   BITSET_SET(enabled_texture_units, unit);
   ctx->Texture._MaxEnabledTexImageUnit =
      MAX2(ctx->Texture._MaxEnabledTexImageUnit, (int)unit);
}

static void
update_program_texture_state(struct gl_context *ctx, struct gl_program **prog,
                             BITSET_WORD *enabled_texture_units)
{
   int i;

   for (i = 0; i < MESA_SHADER_STAGES; i++) {
      GLbitfield mask;
      GLuint s;

      if (!prog[i])
         continue;

      mask = prog[i]->SamplersUsed;

      while (mask) {
         s = u_bit_scan(&mask);

         update_single_program_texture_state(ctx, prog[i],
                                             prog[i]->SamplerUnits[s],
                                             enabled_texture_units);
      }

      if (unlikely(prog[i]->sh.HasBoundBindlessSampler)) {
         /* Loop over bindless samplers bound to texture units.
          */
         for (s = 0; s < prog[i]->sh.NumBindlessSamplers; s++) {
            struct gl_bindless_sampler *sampler =
               &prog[i]->sh.BindlessSamplers[s];

            if (!sampler->bound)
               continue;

            update_single_program_texture_state(ctx, prog[i], sampler->unit,
                                                enabled_texture_units);
         }
      }
   }

   if (prog[MESA_SHADER_FRAGMENT]) {
      const GLuint coordMask = (1 << MAX_TEXTURE_COORD_UNITS) - 1;
      ctx->Texture._EnabledCoordUnits |=
         (prog[MESA_SHADER_FRAGMENT]->info.inputs_read >> VARYING_SLOT_TEX0) &
         coordMask;
   }
}

static void
update_ff_texture_state(struct gl_context *ctx,
                        BITSET_WORD *enabled_texture_units)
{
   int unit;

   for (unit = 0; unit < ctx->Const.MaxTextureUnits; unit++) {
      struct gl_texture_unit *texUnit = &ctx->Texture.Unit[unit];
      struct gl_fixedfunc_texture_unit *fftexUnit =
         &ctx->Texture.FixedFuncUnit[unit];
      GLbitfield mask;
      bool complete;

      if (fftexUnit->Enabled == 0x0)
         continue;

      /* If a shader already dictated what texture target was used for this
       * unit, just go along with it.
       */
      if (BITSET_TEST(enabled_texture_units, unit))
         continue;

      /* From the GL 4.4 compat specification, section 16.2 ("Texture Application"):
       *
       *     "Texturing is enabled or disabled using the generic Enable and
       *      Disable commands, respectively, with the symbolic constants
       *      TEXTURE_1D, TEXTURE_2D, TEXTURE_RECTANGLE, TEXTURE_3D, or
       *      TEXTURE_CUBE_MAP to enable the one-, two-, rectangular,
       *      three-dimensional, or cube map texture, respectively. If more
       *      than one of these textures is enabled, the first one enabled
       *      from the following list is used:
       *
       *      • cube map texture
       *      • three-dimensional texture
       *      • rectangular texture
       *      • two-dimensional texture
       *      • one-dimensional texture"
       *
       * Note that the TEXTURE_x_INDEX values are in high to low priority.
       * Also:
       *
       *     "If a texture unit is disabled or has an invalid or incomplete
       *      texture (as defined in section 8.17) bound to it, then blending
       *      is disabled for that texture unit. If the texture environment
       *      for a given enabled texture unit references a disabled texture
       *      unit, or an invalid or incomplete texture that is bound to
       *      another unit, then the results of texture blending are
       *      undefined."
       */
      complete = false;
      mask = fftexUnit->Enabled;
      while (mask) {
         const int texIndex = u_bit_scan(&mask);
         struct gl_texture_object *texObj = texUnit->CurrentTex[texIndex];
         struct gl_sampler_object *sampler = texUnit->Sampler ?
            texUnit->Sampler : &texObj->Sampler;

         if (!_mesa_is_texture_complete(texObj, sampler,
                                        ctx->Const.ForceIntegerTexNearest)) {
            _mesa_test_texobj_completeness(ctx, texObj);
         }
         if (_mesa_is_texture_complete(texObj, sampler,
                                       ctx->Const.ForceIntegerTexNearest)) {
            _mesa_reference_texobj(&texUnit->_Current, texObj);
            complete = true;
            break;
         }
      }

      if (!complete)
         continue;

      /* if we get here, we know this texture unit is enabled */
      BITSET_SET(enabled_texture_units, unit);
      ctx->Texture._MaxEnabledTexImageUnit =
         MAX2(ctx->Texture._MaxEnabledTexImageUnit, (int)unit);

      ctx->Texture._EnabledCoordUnits |= 1 << unit;

      update_tex_combine(ctx, texUnit, fftexUnit);
   }
}

static void
fix_missing_textures_for_atifs(struct gl_context *ctx,
                               struct gl_program *prog,
                               BITSET_WORD *enabled_texture_units)
{
   GLbitfield mask = prog->SamplersUsed;

   while (mask) {
      const int s = u_bit_scan(&mask);
      const int unit = prog->SamplerUnits[s];
      const gl_texture_index target_index = ffs(prog->TexturesUsed[unit]) - 1;

      if (!ctx->Texture.Unit[unit]._Current) {
         struct gl_texture_object *texObj =
            _mesa_get_fallback_texture(ctx, target_index, false);
         _mesa_reference_texobj(&ctx->Texture.Unit[unit]._Current, texObj);
         BITSET_SET(enabled_texture_units, unit);
         ctx->Texture._MaxEnabledTexImageUnit =
            MAX2(ctx->Texture._MaxEnabledTexImageUnit, (int)unit);
      }
   }
}

/**
 * \note This routine refers to derived texture matrix values to
 * compute the ENABLE_TEXMAT flags, but is only called on
 * _NEW_TEXTURE_OBJECT/STATE.  On changes to _NEW_TEXTURE_MATRIX,
 * the ENABLE_TEXMAT flags are updated by _mesa_update_texture_matrices,
 * above.
 *
 * \param ctx GL context.
 */
GLbitfield
_mesa_update_texture_state(struct gl_context *ctx)
{
   struct gl_program *prog[MESA_SHADER_STAGES];
   int i;
   int old_max_unit = ctx->Texture._MaxEnabledTexImageUnit;
   BITSET_DECLARE(enabled_texture_units, MAX_COMBINED_TEXTURE_IMAGE_UNITS);

   memcpy(prog, ctx->_Shader->CurrentProgram, sizeof(prog));

   if (prog[MESA_SHADER_FRAGMENT] == NULL &&
       _mesa_arb_fragment_program_enabled(ctx)) {
      prog[MESA_SHADER_FRAGMENT] = ctx->FragmentProgram.Current;
   }

   /* TODO: only set this if there are actual changes */
   ctx->NewState |= _NEW_TEXTURE_OBJECT | _NEW_TEXTURE_STATE;

   GLbitfield old_genflags = ctx->Texture._GenFlags;
   GLbitfield old_enabled_coord_units = ctx->Texture._EnabledCoordUnits;
   GLbitfield old_texgen_enabled = ctx->Texture._TexGenEnabled;
   GLbitfield old_texmat_enabled = ctx->Texture._TexMatEnabled;

   ctx->Texture._GenFlags = 0x0;
   ctx->Texture._TexMatEnabled = 0x0;
   ctx->Texture._TexGenEnabled = 0x0;
   ctx->Texture._MaxEnabledTexImageUnit = -1;
   ctx->Texture._EnabledCoordUnits = 0x0;

   memset(&enabled_texture_units, 0, sizeof(enabled_texture_units));

   /* First, walk over our programs pulling in all the textures for them.
    * Programs dictate specific texture targets to be enabled, and for a draw
    * call to be valid they can't conflict about which texture targets are
    * used.
    */
   update_program_texture_state(ctx, prog, enabled_texture_units);

   /* Also pull in any textures necessary for fixed function fragment shading.
    */
   if (!prog[MESA_SHADER_FRAGMENT])
      update_ff_texture_state(ctx, enabled_texture_units);

   /* Now, clear out the _Current of any disabled texture units. */
   for (i = 0; i <= ctx->Texture._MaxEnabledTexImageUnit; i++) {
      if (!BITSET_TEST(enabled_texture_units, i))
         _mesa_reference_texobj(&ctx->Texture.Unit[i]._Current, NULL);
   }
   for (i = ctx->Texture._MaxEnabledTexImageUnit + 1; i <= old_max_unit; i++) {
      _mesa_reference_texobj(&ctx->Texture.Unit[i]._Current, NULL);
   }

   /* add fallback texture for SampleMapATI if there is nothing */
   if (_mesa_ati_fragment_shader_enabled(ctx) &&
       ctx->ATIFragmentShader.Current->Program)
      fix_missing_textures_for_atifs(ctx,
                                     ctx->ATIFragmentShader.Current->Program,
                                     enabled_texture_units);

   if (!prog[MESA_SHADER_FRAGMENT] || !prog[MESA_SHADER_VERTEX])
      update_texgen(ctx);

   GLbitfield new_state = 0;

   if (old_enabled_coord_units != ctx->Texture._EnabledCoordUnits ||
       old_texgen_enabled != ctx->Texture._TexGenEnabled ||
       old_texmat_enabled != ctx->Texture._TexMatEnabled) {
      new_state |= _NEW_FF_VERT_PROGRAM | _NEW_FF_FRAG_PROGRAM;
   }

   if (old_genflags != ctx->Texture._GenFlags)
      new_state |= _NEW_TNL_SPACES;

   return new_state;
}


/**********************************************************************/
/*****                      Initialization                        *****/
/**********************************************************************/

/**
 * Allocate the proxy textures for the given context.
 *
 * \param ctx the context to allocate proxies for.
 *
 * \return GL_TRUE on success, or GL_FALSE on failure
 *
 * If run out of memory part way through the allocations, clean up and return
 * GL_FALSE.
 */
static GLboolean
alloc_proxy_textures( struct gl_context *ctx )
{
   /* NOTE: these values must be in the same order as the TEXTURE_x_INDEX
    * values!
    */
   static const GLenum targets[] = {
      GL_TEXTURE_2D_MULTISAMPLE,
      GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
      GL_TEXTURE_CUBE_MAP_ARRAY,
      GL_TEXTURE_BUFFER,
      GL_TEXTURE_2D_ARRAY_EXT,
      GL_TEXTURE_1D_ARRAY_EXT,
      GL_TEXTURE_EXTERNAL_OES,
      GL_TEXTURE_CUBE_MAP,
      GL_TEXTURE_3D,
      GL_TEXTURE_RECTANGLE_NV,
      GL_TEXTURE_2D,
      GL_TEXTURE_1D,
   };
   GLint tgt;

   STATIC_ASSERT(ARRAY_SIZE(targets) == NUM_TEXTURE_TARGETS);
   assert(targets[TEXTURE_2D_INDEX] == GL_TEXTURE_2D);
   assert(targets[TEXTURE_CUBE_INDEX] == GL_TEXTURE_CUBE_MAP);

   for (tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++) {
      if (!(ctx->Texture.ProxyTex[tgt]
            = _mesa_new_texture_object(ctx, 0, targets[tgt]))) {
         /* out of memory, free what we did allocate */
         while (--tgt >= 0) {
            _mesa_delete_texture_object(ctx, ctx->Texture.ProxyTex[tgt]);
         }
         return GL_FALSE;
      }
   }

   assert(ctx->Texture.ProxyTex[0]->RefCount == 1); /* sanity check */
   return GL_TRUE;
}


/**
 * Initialize texture state for the given context.
 */
GLboolean
_mesa_init_texture(struct gl_context *ctx)
{
   GLuint u;

   /* Texture group */
   ctx->Texture.CurrentUnit = 0;      /* multitexture */

   for (u = 0; u < ARRAY_SIZE(ctx->Texture.Unit); u++) {
      struct gl_texture_unit *texUnit = &ctx->Texture.Unit[u];
      GLuint tex;

      /* initialize current texture object ptrs to the shared default objects */
      for (tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
         _mesa_reference_texobj(&texUnit->CurrentTex[tex],
                                ctx->Shared->DefaultTex[tex]);
      }

      texUnit->_BoundTextures = 0;
   }

   for (u = 0; u < ARRAY_SIZE(ctx->Texture.FixedFuncUnit); u++) {
      struct gl_fixedfunc_texture_unit *texUnit =
         &ctx->Texture.FixedFuncUnit[u];

      texUnit->EnvMode = GL_MODULATE;
      ASSIGN_4V( texUnit->EnvColor, 0.0, 0.0, 0.0, 0.0 );

      texUnit->Combine = default_combine_state;
      texUnit->_EnvMode = default_combine_state;
      texUnit->_CurrentCombine = & texUnit->_EnvMode;

      texUnit->TexGenEnabled = 0x0;
      texUnit->GenS.Mode = GL_EYE_LINEAR;
      texUnit->GenT.Mode = GL_EYE_LINEAR;
      texUnit->GenR.Mode = GL_EYE_LINEAR;
      texUnit->GenQ.Mode = GL_EYE_LINEAR;
      texUnit->GenS._ModeBit = TEXGEN_EYE_LINEAR;
      texUnit->GenT._ModeBit = TEXGEN_EYE_LINEAR;
      texUnit->GenR._ModeBit = TEXGEN_EYE_LINEAR;
      texUnit->GenQ._ModeBit = TEXGEN_EYE_LINEAR;

      /* Yes, these plane coefficients are correct! */
      ASSIGN_4V( texUnit->ObjectPlane[GEN_S], 1.0, 0.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->ObjectPlane[GEN_T], 0.0, 1.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->ObjectPlane[GEN_R], 0.0, 0.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->ObjectPlane[GEN_Q], 0.0, 0.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->EyePlane[GEN_S], 1.0, 0.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->EyePlane[GEN_T], 0.0, 1.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->EyePlane[GEN_R], 0.0, 0.0, 0.0, 0.0 );
      ASSIGN_4V( texUnit->EyePlane[GEN_Q], 0.0, 0.0, 0.0, 0.0 );
   }

   /* After we're done initializing the context's texture state the default
    * texture objects' refcounts should be at least
    * MAX_COMBINED_TEXTURE_IMAGE_UNITS + 1.
    */
   assert(ctx->Shared->DefaultTex[TEXTURE_1D_INDEX]->RefCount
          >= MAX_COMBINED_TEXTURE_IMAGE_UNITS + 1);

   /* Allocate proxy textures */
   if (!alloc_proxy_textures( ctx ))
      return GL_FALSE;

   /* GL_ARB_texture_buffer_object */
   _mesa_reference_buffer_object(ctx, &ctx->Texture.BufferObject, NULL);

   ctx->Texture.NumCurrentTexUsed = 0;

   return GL_TRUE;
}


/**
 * Free dynamically-allocted texture data attached to the given context.
 */
void
_mesa_free_texture_data(struct gl_context *ctx)
{
   GLuint u, tgt;

   /* unreference current textures */
   for (u = 0; u < ARRAY_SIZE(ctx->Texture.Unit); u++) {
      /* The _Current texture could account for another reference */
      _mesa_reference_texobj(&ctx->Texture.Unit[u]._Current, NULL);

      for (tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++) {
         _mesa_reference_texobj(&ctx->Texture.Unit[u].CurrentTex[tgt], NULL);
      }
   }

   /* Free proxy texture objects */
   for (tgt = 0; tgt < NUM_TEXTURE_TARGETS; tgt++)
      _mesa_delete_texture_object(ctx, ctx->Texture.ProxyTex[tgt]);

   /* GL_ARB_texture_buffer_object */
   _mesa_reference_buffer_object(ctx, &ctx->Texture.BufferObject, NULL);

   for (u = 0; u < ARRAY_SIZE(ctx->Texture.Unit); u++) {
      _mesa_reference_sampler_object(ctx, &ctx->Texture.Unit[u].Sampler, NULL);
   }
}


/**
 * Update the default texture objects in the given context to reference those
 * specified in the shared state and release those referencing the old
 * shared state.
 */
void
_mesa_update_default_objects_texture(struct gl_context *ctx)
{
   GLuint u, tex;

   for (u = 0; u < ARRAY_SIZE(ctx->Texture.Unit); u++) {
      struct gl_texture_unit *texUnit = &ctx->Texture.Unit[u];
      for (tex = 0; tex < NUM_TEXTURE_TARGETS; tex++) {
         _mesa_reference_texobj(&texUnit->CurrentTex[tex],
                                ctx->Shared->DefaultTex[tex]);
      }
   }
}
