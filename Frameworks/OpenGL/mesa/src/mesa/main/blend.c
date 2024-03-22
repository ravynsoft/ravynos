/**
 * \file blend.c
 * Blending operations.
 */

/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
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



#include "util/glheader.h"
#include "blend.h"
#include "context.h"
#include "draw_validate.h"
#include "enums.h"
#include "macros.h"
#include "mtypes.h"
#include "state.h"
#include "api_exec_decl.h"



/**
 * Check if given blend source factor is legal.
 * \return GL_TRUE if legal, GL_FALSE otherwise.
 */
static GLboolean
legal_src_factor(const struct gl_context *ctx, GLenum factor)
{
   switch (factor) {
   case GL_SRC_COLOR:
   case GL_ONE_MINUS_SRC_COLOR:
   case GL_ZERO:
   case GL_ONE:
   case GL_DST_COLOR:
   case GL_ONE_MINUS_DST_COLOR:
   case GL_SRC_ALPHA:
   case GL_ONE_MINUS_SRC_ALPHA:
   case GL_DST_ALPHA:
   case GL_ONE_MINUS_DST_ALPHA:
   case GL_SRC_ALPHA_SATURATE:
      return GL_TRUE;
   case GL_CONSTANT_COLOR:
   case GL_ONE_MINUS_CONSTANT_COLOR:
   case GL_CONSTANT_ALPHA:
   case GL_ONE_MINUS_CONSTANT_ALPHA:
      return _mesa_is_desktop_gl(ctx) || _mesa_is_gles2(ctx);
   case GL_SRC1_COLOR:
   case GL_SRC1_ALPHA:
   case GL_ONE_MINUS_SRC1_COLOR:
   case GL_ONE_MINUS_SRC1_ALPHA:
      return ctx->API != API_OPENGLES
         && ctx->Extensions.ARB_blend_func_extended;
   default:
      return GL_FALSE;
   }
}


/**
 * Check if given blend destination factor is legal.
 * \return GL_TRUE if legal, GL_FALSE otherwise.
 */
static GLboolean
legal_dst_factor(const struct gl_context *ctx, GLenum factor)
{
   switch (factor) {
   case GL_DST_COLOR:
   case GL_ONE_MINUS_DST_COLOR:
   case GL_ZERO:
   case GL_ONE:
   case GL_SRC_COLOR:
   case GL_ONE_MINUS_SRC_COLOR:
   case GL_SRC_ALPHA:
   case GL_ONE_MINUS_SRC_ALPHA:
   case GL_DST_ALPHA:
   case GL_ONE_MINUS_DST_ALPHA:
      return GL_TRUE;
   case GL_CONSTANT_COLOR:
   case GL_ONE_MINUS_CONSTANT_COLOR:
   case GL_CONSTANT_ALPHA:
   case GL_ONE_MINUS_CONSTANT_ALPHA:
      return _mesa_is_desktop_gl(ctx) || _mesa_is_gles2(ctx);
   case GL_SRC_ALPHA_SATURATE:
      return (ctx->API != API_OPENGLES
              && ctx->Extensions.ARB_blend_func_extended)
         || _mesa_is_gles3(ctx);
   case GL_SRC1_COLOR:
   case GL_SRC1_ALPHA:
   case GL_ONE_MINUS_SRC1_COLOR:
   case GL_ONE_MINUS_SRC1_ALPHA:
      return ctx->API != API_OPENGLES
         && ctx->Extensions.ARB_blend_func_extended;
   default:
      return GL_FALSE;
   }
}


/**
 * Check if src/dest RGB/A blend factors are legal.  If not generate
 * a GL error.
 * \return GL_TRUE if factors are legal, GL_FALSE otherwise.
 */
static GLboolean
validate_blend_factors(struct gl_context *ctx, const char *func,
                       GLenum sfactorRGB, GLenum dfactorRGB,
                       GLenum sfactorA, GLenum dfactorA)
{
   if (!legal_src_factor(ctx, sfactorRGB)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(sfactorRGB = %s)", func,
                  _mesa_enum_to_string(sfactorRGB));
      return GL_FALSE;
   }

   if (!legal_dst_factor(ctx, dfactorRGB)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(dfactorRGB = %s)", func,
                  _mesa_enum_to_string(dfactorRGB));
      return GL_FALSE;
   }

   if (sfactorA != sfactorRGB && !legal_src_factor(ctx, sfactorA)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(sfactorA = %s)", func,
                  _mesa_enum_to_string(sfactorA));
      return GL_FALSE;
   }

   if (dfactorA != dfactorRGB && !legal_dst_factor(ctx, dfactorA)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "%s(dfactorA = %s)", func,
                  _mesa_enum_to_string(dfactorA));
      return GL_FALSE;
   }

   return GL_TRUE;
}


static GLboolean
blend_factor_is_dual_src(GLenum factor)
{
   return (factor == GL_SRC1_COLOR ||
	   factor == GL_SRC1_ALPHA ||
	   factor == GL_ONE_MINUS_SRC1_COLOR ||
	   factor == GL_ONE_MINUS_SRC1_ALPHA);
}

static bool
update_uses_dual_src(struct gl_context *ctx, int buf)
{
   bool uses_dual_src =
      (blend_factor_is_dual_src(ctx->Color.Blend[buf].SrcRGB) ||
       blend_factor_is_dual_src(ctx->Color.Blend[buf].DstRGB) ||
       blend_factor_is_dual_src(ctx->Color.Blend[buf].SrcA) ||
       blend_factor_is_dual_src(ctx->Color.Blend[buf].DstA));

   if (((ctx->Color._BlendUsesDualSrc >> buf) & 0x1) != uses_dual_src) {
      if (uses_dual_src)
         ctx->Color._BlendUsesDualSrc |= 1 << buf;
      else
         ctx->Color._BlendUsesDualSrc &= ~(1 << buf);
      return true; /* changed state */
   }
   return false; /* no change */
}


/**
 * Return the number of per-buffer blend states to update in
 * glBlendFunc, glBlendFuncSeparate, glBlendEquation, etc.
 */
static inline unsigned
num_buffers(const struct gl_context *ctx)
{
   return ctx->Extensions.ARB_draw_buffers_blend
      ? ctx->Const.MaxDrawBuffers : 1;
}


/* Returns true if there was no change */
static bool
skip_blend_state_update(const struct gl_context *ctx,
                        GLenum sfactorRGB, GLenum dfactorRGB,
                        GLenum sfactorA, GLenum dfactorA)
{
   /* Check if we're really changing any state.  If not, return early. */
   if (ctx->Color._BlendFuncPerBuffer) {
      const unsigned numBuffers = num_buffers(ctx);

      /* Check all per-buffer states */
      for (unsigned buf = 0; buf < numBuffers; buf++) {
         if (ctx->Color.Blend[buf].SrcRGB != sfactorRGB ||
             ctx->Color.Blend[buf].DstRGB != dfactorRGB ||
             ctx->Color.Blend[buf].SrcA != sfactorA ||
             ctx->Color.Blend[buf].DstA != dfactorA) {
            return false;
         }
      }
   }
   else {
      /* only need to check 0th per-buffer state */
      if (ctx->Color.Blend[0].SrcRGB != sfactorRGB ||
          ctx->Color.Blend[0].DstRGB != dfactorRGB ||
          ctx->Color.Blend[0].SrcA != sfactorA ||
          ctx->Color.Blend[0].DstA != dfactorA) {
         return false;
      }
   }

   return true;
}


static void
blend_func_separate(struct gl_context *ctx,
                    GLenum sfactorRGB, GLenum dfactorRGB,
                    GLenum sfactorA, GLenum dfactorA)
{
   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;

   const unsigned numBuffers = num_buffers(ctx);
   for (unsigned buf = 0; buf < numBuffers; buf++) {
      ctx->Color.Blend[buf].SrcRGB = sfactorRGB;
      ctx->Color.Blend[buf].DstRGB = dfactorRGB;
      ctx->Color.Blend[buf].SrcA = sfactorA;
      ctx->Color.Blend[buf].DstA = dfactorA;
   }

   GLbitfield old_blend_uses_dual_src = ctx->Color._BlendUsesDualSrc;
   update_uses_dual_src(ctx, 0);
   /* We have to replicate the bit to all color buffers. */
   if (ctx->Color._BlendUsesDualSrc & 0x1)
      ctx->Color._BlendUsesDualSrc |= BITFIELD_RANGE(1, numBuffers - 1);
   else
      ctx->Color._BlendUsesDualSrc = 0;

   if (ctx->Color._BlendUsesDualSrc != old_blend_uses_dual_src)
      _mesa_update_valid_to_render_state(ctx);

   ctx->Color._BlendFuncPerBuffer = GL_FALSE;
}


/**
 * Specify the blending operation.
 *
 * \param sfactor source factor operator.
 * \param dfactor destination factor operator.
 *
 * \sa glBlendFunc, glBlendFuncSeparateEXT
 */
void GLAPIENTRY
_mesa_BlendFunc( GLenum sfactor, GLenum dfactor )
{
   GET_CURRENT_CONTEXT(ctx);

   if (skip_blend_state_update(ctx, sfactor, dfactor, sfactor, dfactor))
      return;

   if (!validate_blend_factors(ctx, "glBlendFunc",
                               sfactor, dfactor, sfactor, dfactor)) {
      return;
   }

   blend_func_separate(ctx, sfactor, dfactor, sfactor, dfactor);
}


void GLAPIENTRY
_mesa_BlendFunc_no_error(GLenum sfactor, GLenum dfactor)
{
   GET_CURRENT_CONTEXT(ctx);

   if (skip_blend_state_update(ctx, sfactor, dfactor, sfactor, dfactor))
      return;

   blend_func_separate(ctx, sfactor, dfactor, sfactor, dfactor);
}


/**
 * Set the separate blend source/dest factors for all draw buffers.
 *
 * \param sfactorRGB RGB source factor operator.
 * \param dfactorRGB RGB destination factor operator.
 * \param sfactorA alpha source factor operator.
 * \param dfactorA alpha destination factor operator.
 */
void GLAPIENTRY
_mesa_BlendFuncSeparate( GLenum sfactorRGB, GLenum dfactorRGB,
                            GLenum sfactorA, GLenum dfactorA )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBlendFuncSeparate %s %s %s %s\n",
                  _mesa_enum_to_string(sfactorRGB),
                  _mesa_enum_to_string(dfactorRGB),
                  _mesa_enum_to_string(sfactorA),
                  _mesa_enum_to_string(dfactorA));



   if (skip_blend_state_update(ctx, sfactorRGB, dfactorRGB, sfactorA, dfactorA))
      return;

   if (!validate_blend_factors(ctx, "glBlendFuncSeparate",
                               sfactorRGB, dfactorRGB,
                               sfactorA, dfactorA)) {
      return;
   }

   blend_func_separate(ctx, sfactorRGB, dfactorRGB, sfactorA, dfactorA);
}


void GLAPIENTRY
_mesa_BlendFuncSeparate_no_error(GLenum sfactorRGB, GLenum dfactorRGB,
                                 GLenum sfactorA, GLenum dfactorA)
{
   GET_CURRENT_CONTEXT(ctx);

   if (skip_blend_state_update(ctx, sfactorRGB, dfactorRGB, sfactorA, dfactorA))
      return;

   blend_func_separate(ctx, sfactorRGB, dfactorRGB, sfactorA, dfactorA);
}


void GLAPIENTRY
_mesa_BlendFunciARB_no_error(GLuint buf, GLenum sfactor, GLenum dfactor)
{
   _mesa_BlendFuncSeparateiARB_no_error(buf, sfactor, dfactor, sfactor,
                                        dfactor);
}


/**
 * Set blend source/dest factors for one color buffer/target.
 */
void GLAPIENTRY
_mesa_BlendFunciARB(GLuint buf, GLenum sfactor, GLenum dfactor)
{
   _mesa_BlendFuncSeparateiARB(buf, sfactor, dfactor, sfactor, dfactor);
}


static ALWAYS_INLINE void
blend_func_separatei(GLuint buf, GLenum sfactorRGB, GLenum dfactorRGB,
                     GLenum sfactorA, GLenum dfactorA, bool no_error)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!no_error) {
      if (!ctx->Extensions.ARB_draw_buffers_blend) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "glBlendFunc[Separate]i()");
         return;
      }

      if (buf >= ctx->Const.MaxDrawBuffers) {
         _mesa_error(ctx, GL_INVALID_VALUE, "glBlendFuncSeparatei(buffer=%u)",
                     buf);
         return;
      }
   }

   if (ctx->Color.Blend[buf].SrcRGB == sfactorRGB &&
       ctx->Color.Blend[buf].DstRGB == dfactorRGB &&
       ctx->Color.Blend[buf].SrcA == sfactorA &&
       ctx->Color.Blend[buf].DstA == dfactorA)
      return; /* no change */

   if (!no_error && !validate_blend_factors(ctx, "glBlendFuncSeparatei",
                                            sfactorRGB, dfactorRGB,
                                            sfactorA, dfactorA)) {
      return;
   }

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;

   ctx->Color.Blend[buf].SrcRGB = sfactorRGB;
   ctx->Color.Blend[buf].DstRGB = dfactorRGB;
   ctx->Color.Blend[buf].SrcA = sfactorA;
   ctx->Color.Blend[buf].DstA = dfactorA;
   if (update_uses_dual_src(ctx, buf))
      _mesa_update_valid_to_render_state(ctx);
   ctx->Color._BlendFuncPerBuffer = GL_TRUE;
}


void GLAPIENTRY
_mesa_BlendFuncSeparateiARB_no_error(GLuint buf, GLenum sfactorRGB,
                                     GLenum dfactorRGB, GLenum sfactorA,
                                     GLenum dfactorA)
{
   blend_func_separatei(buf, sfactorRGB, dfactorRGB, sfactorA, dfactorA,
                        true);
}


/**
 * Set separate blend source/dest factors for one color buffer/target.
 */
void GLAPIENTRY
_mesa_BlendFuncSeparateiARB(GLuint buf, GLenum sfactorRGB, GLenum dfactorRGB,
                            GLenum sfactorA, GLenum dfactorA)
{
   blend_func_separatei(buf, sfactorRGB, dfactorRGB, sfactorA, dfactorA,
                        false);
}


/**
 * Return true if \p mode is a legal blending equation, excluding
 * GL_KHR_blend_equation_advanced modes.
 */
static bool
legal_simple_blend_equation(const struct gl_context *ctx, GLenum mode)
{
   switch (mode) {
   case GL_FUNC_ADD:
   case GL_FUNC_SUBTRACT:
   case GL_FUNC_REVERSE_SUBTRACT:
   case GL_MIN:
   case GL_MAX:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

static enum gl_advanced_blend_mode
advanced_blend_mode_from_gl_enum(GLenum mode)
{
   switch (mode) {
   case GL_MULTIPLY_KHR:
      return BLEND_MULTIPLY;
   case GL_SCREEN_KHR:
      return BLEND_SCREEN;
   case GL_OVERLAY_KHR:
      return BLEND_OVERLAY;
   case GL_DARKEN_KHR:
      return BLEND_DARKEN;
   case GL_LIGHTEN_KHR:
      return BLEND_LIGHTEN;
   case GL_COLORDODGE_KHR:
      return BLEND_COLORDODGE;
   case GL_COLORBURN_KHR:
      return BLEND_COLORBURN;
   case GL_HARDLIGHT_KHR:
      return BLEND_HARDLIGHT;
   case GL_SOFTLIGHT_KHR:
      return BLEND_SOFTLIGHT;
   case GL_DIFFERENCE_KHR:
      return BLEND_DIFFERENCE;
   case GL_EXCLUSION_KHR:
      return BLEND_EXCLUSION;
   case GL_HSL_HUE_KHR:
      return BLEND_HSL_HUE;
   case GL_HSL_SATURATION_KHR:
      return BLEND_HSL_SATURATION;
   case GL_HSL_COLOR_KHR:
      return BLEND_HSL_COLOR;
   case GL_HSL_LUMINOSITY_KHR:
      return BLEND_HSL_LUMINOSITY;
   default:
      return BLEND_NONE;
   }
}

/**
 * If \p mode is one of the advanced blending equations defined by
 * GL_KHR_blend_equation_advanced (and the extension is supported),
 * return the corresponding BLEND_* enum.  Otherwise, return BLEND_NONE
 * (which can also be treated as false).
 */
static enum gl_advanced_blend_mode
advanced_blend_mode(const struct gl_context *ctx, GLenum mode)
{
   return _mesa_has_KHR_blend_equation_advanced(ctx) ?
          advanced_blend_mode_from_gl_enum(mode) : BLEND_NONE;
}

static void
set_advanced_blend_mode(struct gl_context *ctx,
                        enum gl_advanced_blend_mode advanced_mode)
{
   if (ctx->Color._AdvancedBlendMode != advanced_mode) {
      ctx->Color._AdvancedBlendMode = advanced_mode;
      _mesa_update_valid_to_render_state(ctx);
   }
}

/* This is really an extension function! */
void GLAPIENTRY
_mesa_BlendEquation( GLenum mode )
{
   GET_CURRENT_CONTEXT(ctx);
   const unsigned numBuffers = num_buffers(ctx);
   unsigned buf;
   bool changed = false;
   enum gl_advanced_blend_mode advanced_mode = advanced_blend_mode(ctx, mode);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBlendEquation(%s)\n",
                  _mesa_enum_to_string(mode));

   if (ctx->Color._BlendEquationPerBuffer) {
      /* Check all per-buffer states */
      for (buf = 0; buf < numBuffers; buf++) {
         if (ctx->Color.Blend[buf].EquationRGB != mode ||
             ctx->Color.Blend[buf].EquationA != mode) {
            changed = true;
            break;
         }
      }
   }
   else {
      /* only need to check 0th per-buffer state */
      if (ctx->Color.Blend[0].EquationRGB != mode ||
          ctx->Color.Blend[0].EquationA != mode) {
         changed = true;
      }
   }

   if (!changed)
      return;


   if (!legal_simple_blend_equation(ctx, mode) && !advanced_mode) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBlendEquation");
      return;
   }

   _mesa_flush_vertices_for_blend_adv(ctx, ctx->Color.BlendEnabled,
                                      advanced_mode);

   for (buf = 0; buf < numBuffers; buf++) {
      ctx->Color.Blend[buf].EquationRGB = mode;
      ctx->Color.Blend[buf].EquationA = mode;
   }
   ctx->Color._BlendEquationPerBuffer = GL_FALSE;
   set_advanced_blend_mode(ctx, advanced_mode);
}


/**
 * Set blend equation for one color buffer/target.
 */
static void
blend_equationi(struct gl_context *ctx, GLuint buf, GLenum mode,
                enum gl_advanced_blend_mode advanced_mode)
{
   if (ctx->Color.Blend[buf].EquationRGB == mode &&
       ctx->Color.Blend[buf].EquationA == mode)
      return;  /* no change */

   _mesa_flush_vertices_for_blend_adv(ctx, ctx->Color.BlendEnabled,
                                      advanced_mode);
   ctx->Color.Blend[buf].EquationRGB = mode;
   ctx->Color.Blend[buf].EquationA = mode;
   ctx->Color._BlendEquationPerBuffer = GL_TRUE;

   if (buf == 0)
      set_advanced_blend_mode(ctx, advanced_mode);
}


void GLAPIENTRY
_mesa_BlendEquationiARB_no_error(GLuint buf, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);

   enum gl_advanced_blend_mode advanced_mode = advanced_blend_mode(ctx, mode);
   blend_equationi(ctx, buf, mode, advanced_mode);
}


void GLAPIENTRY
_mesa_BlendEquationiARB(GLuint buf, GLenum mode)
{
   GET_CURRENT_CONTEXT(ctx);
   enum gl_advanced_blend_mode advanced_mode = advanced_blend_mode(ctx, mode);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBlendEquationi(%u, %s)\n",
                  buf, _mesa_enum_to_string(mode));

   if (buf >= ctx->Const.MaxDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBlendEquationi(buffer=%u)",
                  buf);
      return;
   }

   if (!legal_simple_blend_equation(ctx, mode) && !advanced_mode) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glBlendEquationi");
      return;
   }

   blend_equationi(ctx, buf, mode, advanced_mode);
}


static void
blend_equation_separate(struct gl_context *ctx, GLenum modeRGB, GLenum modeA,
                        bool no_error)
{
   const unsigned numBuffers = num_buffers(ctx);
   unsigned buf;
   bool changed = false;

   if (ctx->Color._BlendEquationPerBuffer) {
      /* Check all per-buffer states */
      for (buf = 0; buf < numBuffers; buf++) {
         if (ctx->Color.Blend[buf].EquationRGB != modeRGB ||
             ctx->Color.Blend[buf].EquationA != modeA) {
            changed = true;
            break;
         }
      }
   } else {
      /* only need to check 0th per-buffer state */
      if (ctx->Color.Blend[0].EquationRGB != modeRGB ||
          ctx->Color.Blend[0].EquationA != modeA) {
         changed = true;
      }
   }

   if (!changed)
      return;

   if (!no_error) {
      if ((modeRGB != modeA) && !ctx->Extensions.EXT_blend_equation_separate) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glBlendEquationSeparateEXT not supported by driver");
         return;
      }

      /* Only allow simple blending equations.
       * The GL_KHR_blend_equation_advanced spec says:
       *
       *    "NOTE: These enums are not accepted by the <modeRGB> or <modeAlpha>
       *     parameters of BlendEquationSeparate or BlendEquationSeparatei."
       */
      if (!legal_simple_blend_equation(ctx, modeRGB)) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glBlendEquationSeparateEXT(modeRGB)");
         return;
      }

      if (!legal_simple_blend_equation(ctx, modeA)) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBlendEquationSeparateEXT(modeA)");
         return;
      }
   }

   _mesa_flush_vertices_for_blend_state(ctx);

   for (buf = 0; buf < numBuffers; buf++) {
      ctx->Color.Blend[buf].EquationRGB = modeRGB;
      ctx->Color.Blend[buf].EquationA = modeA;
   }
   ctx->Color._BlendEquationPerBuffer = GL_FALSE;
   set_advanced_blend_mode(ctx, BLEND_NONE);
}


void GLAPIENTRY
_mesa_BlendEquationSeparate_no_error(GLenum modeRGB, GLenum modeA)
{
   GET_CURRENT_CONTEXT(ctx);
   blend_equation_separate(ctx, modeRGB, modeA, true);
}


void GLAPIENTRY
_mesa_BlendEquationSeparate(GLenum modeRGB, GLenum modeA)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBlendEquationSeparateEXT(%s %s)\n",
                  _mesa_enum_to_string(modeRGB),
                  _mesa_enum_to_string(modeA));

   blend_equation_separate(ctx, modeRGB, modeA, false);
}


static ALWAYS_INLINE void
blend_equation_separatei(struct gl_context *ctx, GLuint buf, GLenum modeRGB,
                         GLenum modeA, bool no_error)
{
   if (ctx->Color.Blend[buf].EquationRGB == modeRGB &&
       ctx->Color.Blend[buf].EquationA == modeA)
      return;  /* no change */

   if (!no_error) {
      /* Only allow simple blending equations.
       * The GL_KHR_blend_equation_advanced spec says:
       *
       *    "NOTE: These enums are not accepted by the <modeRGB> or <modeAlpha>
       *     parameters of BlendEquationSeparate or BlendEquationSeparatei."
       */
      if (!legal_simple_blend_equation(ctx, modeRGB)) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBlendEquationSeparatei(modeRGB)");
         return;
      }

      if (!legal_simple_blend_equation(ctx, modeA)) {
         _mesa_error(ctx, GL_INVALID_ENUM, "glBlendEquationSeparatei(modeA)");
         return;
      }
   }

   _mesa_flush_vertices_for_blend_state(ctx);
   ctx->Color.Blend[buf].EquationRGB = modeRGB;
   ctx->Color.Blend[buf].EquationA = modeA;
   ctx->Color._BlendEquationPerBuffer = GL_TRUE;
   set_advanced_blend_mode(ctx, BLEND_NONE);
}


void GLAPIENTRY
_mesa_BlendEquationSeparateiARB_no_error(GLuint buf, GLenum modeRGB,
                                         GLenum modeA)
{
   GET_CURRENT_CONTEXT(ctx);
   blend_equation_separatei(ctx, buf, modeRGB, modeA, true);
}


/**
 * Set separate blend equations for one color buffer/target.
 */
void GLAPIENTRY
_mesa_BlendEquationSeparateiARB(GLuint buf, GLenum modeRGB, GLenum modeA)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glBlendEquationSeparatei(%u, %s %s)\n", buf,
                  _mesa_enum_to_string(modeRGB),
                  _mesa_enum_to_string(modeA));

   if (buf >= ctx->Const.MaxDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glBlendEquationSeparatei(buffer=%u)",
                  buf);
      return;
   }

   blend_equation_separatei(ctx, buf, modeRGB, modeA, false);
}


/**
 * Set the blending color.
 *
 * \param red red color component.
 * \param green green color component.
 * \param blue blue color component.
 * \param alpha alpha color component.
 *
 * \sa glBlendColor().
 *
 * Clamps the parameters and updates gl_colorbuffer_attrib::BlendColor.  On a
 * change, flushes the vertices and notifies the driver via
 * dd_function_table::BlendColor callback.
 */
void GLAPIENTRY
_mesa_BlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
   GLfloat tmp[4];
   GET_CURRENT_CONTEXT(ctx);

   tmp[0] = red;
   tmp[1] = green;
   tmp[2] = blue;
   tmp[3] = alpha;

   if (TEST_EQ_4V(tmp, ctx->Color.BlendColorUnclamped))
      return;

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND_COLOR;
   COPY_4FV( ctx->Color.BlendColorUnclamped, tmp );

   ctx->Color.BlendColor[0] = CLAMP(tmp[0], 0.0F, 1.0F);
   ctx->Color.BlendColor[1] = CLAMP(tmp[1], 0.0F, 1.0F);
   ctx->Color.BlendColor[2] = CLAMP(tmp[2], 0.0F, 1.0F);
   ctx->Color.BlendColor[3] = CLAMP(tmp[3], 0.0F, 1.0F);
}


/**
 * Specify the alpha test function.
 *
 * \param func alpha comparison function.
 * \param ref reference value.
 *
 * Verifies the parameters and updates gl_colorbuffer_attrib. 
 * On a change, flushes the vertices and notifies the driver via
 * dd_function_table::AlphaFunc callback.
 */
void GLAPIENTRY
_mesa_AlphaFunc( GLenum func, GLclampf ref )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glAlphaFunc(%s, %f)\n",
                  _mesa_enum_to_string(func), ref);

   if (ctx->Color.AlphaFunc == func && ctx->Color.AlphaRefUnclamped == ref)
      return; /* no change */

   switch (func) {
   case GL_NEVER:
   case GL_LESS:
   case GL_EQUAL:
   case GL_LEQUAL:
   case GL_GREATER:
   case GL_NOTEQUAL:
   case GL_GEQUAL:
   case GL_ALWAYS:
      FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
      ctx->NewDriverState |= ctx->DriverFlags.NewAlphaTest;
      ctx->Color.AlphaFunc = func;
      ctx->Color.AlphaRefUnclamped = ref;
      ctx->Color.AlphaRef = CLAMP(ref, 0.0F, 1.0F);
      return;

   default:
      _mesa_error( ctx, GL_INVALID_ENUM, "glAlphaFunc(func)" );
      return;
   }
}

static const enum gl_logicop_mode color_logicop_mapping[16] = {
   COLOR_LOGICOP_CLEAR,
   COLOR_LOGICOP_AND,
   COLOR_LOGICOP_AND_REVERSE,
   COLOR_LOGICOP_COPY,
   COLOR_LOGICOP_AND_INVERTED,
   COLOR_LOGICOP_NOOP,
   COLOR_LOGICOP_XOR,
   COLOR_LOGICOP_OR,
   COLOR_LOGICOP_NOR,
   COLOR_LOGICOP_EQUIV,
   COLOR_LOGICOP_INVERT,
   COLOR_LOGICOP_OR_REVERSE,
   COLOR_LOGICOP_COPY_INVERTED,
   COLOR_LOGICOP_OR_INVERTED,
   COLOR_LOGICOP_NAND,
   COLOR_LOGICOP_SET
};

static ALWAYS_INLINE void
logic_op(struct gl_context *ctx, GLenum opcode, bool no_error)
{
   if (ctx->Color.LogicOp == opcode)
      return;

   if (!no_error) {
      switch (opcode) {
         case GL_CLEAR:
         case GL_SET:
         case GL_COPY:
         case GL_COPY_INVERTED:
         case GL_NOOP:
         case GL_INVERT:
         case GL_AND:
         case GL_NAND:
         case GL_OR:
         case GL_NOR:
         case GL_XOR:
         case GL_EQUIV:
         case GL_AND_REVERSE:
         case GL_AND_INVERTED:
         case GL_OR_REVERSE:
         case GL_OR_INVERTED:
            break;
         default:
            _mesa_error( ctx, GL_INVALID_ENUM, "glLogicOp" );
            return;
      }
   }

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   ctx->Color.LogicOp = opcode;
   ctx->Color._LogicOp = color_logicop_mapping[opcode & 0x0f];
   _mesa_update_allow_draw_out_of_order(ctx);
}


/**
 * Specify a logic pixel operation for color index rendering.
 *
 * \param opcode operation.
 *
 * Verifies that \p opcode is a valid enum and updates
 * gl_colorbuffer_attrib::LogicOp.
 * On a change, flushes the vertices and notifies the driver via the
 * dd_function_table::LogicOpcode callback.
 */
void GLAPIENTRY
_mesa_LogicOp( GLenum opcode )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glLogicOp(%s)\n", _mesa_enum_to_string(opcode));

   logic_op(ctx, opcode, false);
}


void GLAPIENTRY
_mesa_LogicOp_no_error(GLenum opcode)
{
   GET_CURRENT_CONTEXT(ctx);
   logic_op(ctx, opcode, true);
}


void GLAPIENTRY
_mesa_IndexMask( GLuint mask )
{
   GET_CURRENT_CONTEXT(ctx);

   if (ctx->Color.IndexMask == mask)
      return;

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   ctx->Color.IndexMask = mask;
}


/**
 * Enable or disable writing of frame buffer color components.
 *
 * \param red whether to mask writing of the red color component.
 * \param green whether to mask writing of the green color component.
 * \param blue whether to mask writing of the blue color component.
 * \param alpha whether to mask writing of the alpha color component.
 *
 * \sa glColorMask().
 *
 * Sets the appropriate value of gl_colorbuffer_attrib::ColorMask.  On a
 * change, flushes the vertices and notifies the driver via the
 * dd_function_table::ColorMask callback.
 */
void GLAPIENTRY
_mesa_ColorMask( GLboolean red, GLboolean green,
                 GLboolean blue, GLboolean alpha )
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glColorMask(%d, %d, %d, %d)\n",
                  red, green, blue, alpha);

   GLbitfield mask = (!!red) |
                     ((!!green) << 1) |
                     ((!!blue) << 2) |
                     ((!!alpha) << 3);
   mask = _mesa_replicate_colormask(mask, ctx->Const.MaxDrawBuffers);

   if (ctx->Color.ColorMask == mask)
      return;

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   ctx->Color.ColorMask = mask;
   _mesa_update_allow_draw_out_of_order(ctx);
}


/**
 * For GL_EXT_draw_buffers2 and GL3
 */
void GLAPIENTRY
_mesa_ColorMaski(GLuint buf, GLboolean red, GLboolean green,
                 GLboolean blue, GLboolean alpha)
{
   GET_CURRENT_CONTEXT(ctx);

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glColorMaski %u %d %d %d %d\n",
                  buf, red, green, blue, alpha);

   if (buf >= ctx->Const.MaxDrawBuffers) {
      _mesa_error(ctx, GL_INVALID_VALUE, "glColorMaski(buf=%u)", buf);
      return;
   }

   GLbitfield mask = (!!red) |
                     ((!!green) << 1) |
                     ((!!blue) << 2) |
                     ((!!alpha) << 3);

   if (GET_COLORMASK(ctx->Color.ColorMask, buf) == mask)
      return;

   FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT);
   ctx->NewDriverState |= ST_NEW_BLEND;
   ctx->Color.ColorMask &= ~(0xf << (4 * buf));
   ctx->Color.ColorMask |= mask << (4 * buf);
   _mesa_update_allow_draw_out_of_order(ctx);
}


void GLAPIENTRY
_mesa_ClampColor(GLenum target, GLenum clamp)
{
   GET_CURRENT_CONTEXT(ctx);

   /* Check for both the extension and the GL version, since the Intel driver
    * does not advertise the extension in core profiles.
    */
   if (ctx->Version <= 30 && !ctx->Extensions.ARB_color_buffer_float) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glClampColor()");
      return;
   }

   if (clamp != GL_TRUE && clamp != GL_FALSE && clamp != GL_FIXED_ONLY_ARB) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glClampColorARB(clamp)");
      return;
   }

   switch (target) {
   case GL_CLAMP_VERTEX_COLOR_ARB:
      if (_mesa_is_desktop_gl_core(ctx))
         goto invalid_enum;
      FLUSH_VERTICES(ctx, _NEW_LIGHT_STATE, GL_LIGHTING_BIT | GL_ENABLE_BIT);
      ctx->Light.ClampVertexColor = clamp;
      _mesa_update_clamp_vertex_color(ctx, ctx->DrawBuffer);
      break;
   case GL_CLAMP_FRAGMENT_COLOR_ARB:
      if (_mesa_is_desktop_gl_core(ctx))
         goto invalid_enum;
      if (ctx->Color.ClampFragmentColor != clamp) {
         FLUSH_VERTICES(ctx, 0, GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT);
         ctx->Color.ClampFragmentColor = clamp;
         _mesa_update_clamp_fragment_color(ctx, ctx->DrawBuffer);
      }
      break;
   case GL_CLAMP_READ_COLOR_ARB:
      ctx->Color.ClampReadColor = clamp;
      ctx->PopAttribState |= GL_COLOR_BUFFER_BIT | GL_ENABLE_BIT;
      break;
   default:
      goto invalid_enum;
   }
   return;

invalid_enum:
   _mesa_error(ctx, GL_INVALID_ENUM, "glClampColor(%s)",
               _mesa_enum_to_string(target));
}

static GLboolean
get_clamp_color(const struct gl_framebuffer *fb, GLenum clamp)
{
   if (clamp == GL_TRUE || clamp == GL_FALSE)
      return clamp;

   assert(clamp == GL_FIXED_ONLY);
   if (!fb)
      return GL_TRUE;

   return fb->_AllColorBuffersFixedPoint;
}

GLboolean
_mesa_get_clamp_fragment_color(const struct gl_context *ctx,
                               const struct gl_framebuffer *drawFb)
{
   return get_clamp_color(drawFb, ctx->Color.ClampFragmentColor);
}

GLboolean
_mesa_get_clamp_vertex_color(const struct gl_context *ctx,
                             const struct gl_framebuffer *drawFb)
{
   return get_clamp_color(drawFb, ctx->Light.ClampVertexColor);
}

GLboolean
_mesa_get_clamp_read_color(const struct gl_context *ctx,
                           const struct gl_framebuffer *readFb)
{
   return get_clamp_color(readFb, ctx->Color.ClampReadColor);
}

/**
 * Update the ctx->Color._ClampFragmentColor field
 */
void
_mesa_update_clamp_fragment_color(struct gl_context *ctx,
                                  const struct gl_framebuffer *drawFb)
{
   GLboolean clamp;

   /* Don't clamp if:
    * - there is no colorbuffer
    * - all colorbuffers are unsigned normalized, so clamping has no effect
    * - there is an integer colorbuffer
    */
   if (!drawFb || !drawFb->_HasSNormOrFloatColorBuffer ||
       drawFb->_IntegerBuffers)
      clamp = GL_FALSE;
   else
      clamp = _mesa_get_clamp_fragment_color(ctx, drawFb);

   if (ctx->Color._ClampFragmentColor == clamp)
      return;

   ctx->NewState |= _NEW_FRAG_CLAMP; /* for state constants */
   ctx->NewDriverState |= ctx->DriverFlags.NewFragClamp;
   ctx->Color._ClampFragmentColor = clamp;
}

/**
 * Update the ctx->Color._ClampVertexColor field
 */
void
_mesa_update_clamp_vertex_color(struct gl_context *ctx,
                                const struct gl_framebuffer *drawFb)
{
   ctx->Light._ClampVertexColor =
         _mesa_get_clamp_vertex_color(ctx, drawFb);
}

/**********************************************************************/
/** \name Initialization */
/*@{*/

/**
 * Initialization of the context's Color attribute group.
 *
 * \param ctx GL context.
 *
 * Initializes the related fields in the context color attribute group,
 * __struct gl_contextRec::Color.
 */
void _mesa_init_color( struct gl_context * ctx )
{
   GLuint i;

   /* Color buffer group */
   ctx->Color.IndexMask = ~0u;
   ctx->Color.ColorMask = 0xffffffff;
   ctx->Color.ClearIndex = 0;
   ASSIGN_4V( ctx->Color.ClearColor.f, 0, 0, 0, 0 );
   ctx->Color.AlphaEnabled = GL_FALSE;
   ctx->Color.AlphaFunc = GL_ALWAYS;
   ctx->Color.AlphaRef = 0;
   ctx->Color.BlendEnabled = 0x0;
   for (i = 0; i < ARRAY_SIZE(ctx->Color.Blend); i++) {
      ctx->Color.Blend[i].SrcRGB = GL_ONE;
      ctx->Color.Blend[i].DstRGB = GL_ZERO;
      ctx->Color.Blend[i].SrcA = GL_ONE;
      ctx->Color.Blend[i].DstA = GL_ZERO;
      ctx->Color.Blend[i].EquationRGB = GL_FUNC_ADD;
      ctx->Color.Blend[i].EquationA = GL_FUNC_ADD;
   }
   ASSIGN_4V( ctx->Color.BlendColor, 0.0, 0.0, 0.0, 0.0 );
   ASSIGN_4V( ctx->Color.BlendColorUnclamped, 0.0, 0.0, 0.0, 0.0 );
   ctx->Color.IndexLogicOpEnabled = GL_FALSE;
   ctx->Color.ColorLogicOpEnabled = GL_FALSE;
   ctx->Color.LogicOp = GL_COPY;
   ctx->Color._LogicOp = COLOR_LOGICOP_COPY;
   ctx->Color.DitherFlag = GL_TRUE;

   /* GL_FRONT is not possible on GLES. Instead GL_BACK will render to either
    * the front or the back buffer depending on the config */
   if (ctx->Visual.doubleBufferMode || _mesa_is_gles(ctx)) {
      ctx->Color.DrawBuffer[0] = GL_BACK;
   }
   else {
      ctx->Color.DrawBuffer[0] = GL_FRONT;
   }

   ctx->Color.ClampFragmentColor = _mesa_is_desktop_gl_compat(ctx) ?
                                   GL_FIXED_ONLY_ARB : GL_FALSE;
   ctx->Color._ClampFragmentColor = GL_FALSE;
   ctx->Color.ClampReadColor = GL_FIXED_ONLY_ARB;

   /* GLES 1/2/3 behaves as though GL_FRAMEBUFFER_SRGB is always enabled
    * if EGL_KHR_gl_colorspace has been used to request sRGB.
    */
   ctx->Color.sRGBEnabled = _mesa_is_gles(ctx);

   ctx->Color.BlendCoherent = true;
}

/*@}*/
