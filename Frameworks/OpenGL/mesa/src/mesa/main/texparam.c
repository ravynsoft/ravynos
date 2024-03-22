/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009  VMware, Inc.  All Rights Reserved.
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
 * \file texparam.c
 *
 * glTexParameter-related functions
 */

#include <stdbool.h>
#include "util/glheader.h"
#include "main/blend.h"
#include "main/context.h"
#include "main/enums.h"
#include "main/formats.h"
#include "main/glformats.h"
#include "main/macros.h"
#include "main/mtypes.h"
#include "main/state.h"
#include "main/texcompress.h"
#include "main/texobj.h"
#include "main/texparam.h"
#include "main/teximage.h"
#include "main/texstate.h"
#include "program/prog_instruction.h"
#include "util/u_math.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_sampler_view.h"

/**
 * Use macro to resolve undefined clamping behaviour when using lroundf
 */
#define LCLAMPF(a, lmin, lmax) ((a) > (float)(lmin) ? ( (a) >= (float)(lmax) ? (lmax) : (lroundf(a)) ) : (lmin))

/**
 * Check if a coordinate wrap mode is supported for the texture target.
 * \return GL_TRUE if legal, GL_FALSE otherwise
 */
static GLboolean
validate_texture_wrap_mode(struct gl_context * ctx, GLenum target, GLenum wrap)
{
   const struct gl_extensions * const e = & ctx->Extensions;
   const bool is_desktop_gl = _mesa_is_desktop_gl(ctx);
   bool supported;

   switch (wrap) {
   case GL_CLAMP:
      /* GL_CLAMP was removed in the core profile, and it has never existed in
       * OpenGL ES.
       */
      supported = _mesa_is_desktop_gl_compat(ctx)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_CLAMP_TO_EDGE:
      supported = true;
      break;

   case GL_CLAMP_TO_BORDER:
      supported = ctx->API != API_OPENGLES
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_REPEAT:
   case GL_MIRRORED_REPEAT:
      supported = (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_MIRROR_CLAMP_EXT:
      supported = is_desktop_gl
         && (e->ATI_texture_mirror_once || e->EXT_texture_mirror_clamp)
         && (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   case GL_MIRROR_CLAMP_TO_EDGE_EXT:
      supported = (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES)
         && (_mesa_has_ARB_texture_mirror_clamp_to_edge(ctx) ||
             _mesa_has_EXT_texture_mirror_clamp_to_edge(ctx) ||
             _mesa_has_ATI_texture_mirror_once(ctx) ||
             _mesa_has_EXT_texture_mirror_clamp(ctx));
      break;

   case GL_MIRROR_CLAMP_TO_BORDER_EXT:
      supported = is_desktop_gl && e->EXT_texture_mirror_clamp
         && (target != GL_TEXTURE_RECTANGLE_NV)
         && (target != GL_TEXTURE_EXTERNAL_OES);
      break;

   default:
      supported = false;
      break;
   }

   if (!supported)
      _mesa_error( ctx, GL_INVALID_ENUM, "glTexParameter(param=0x%x)", wrap );

   return supported;
}


static bool
is_texparameteri_target_valid(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_RECTANGLE:
      return true;
   default:
      return false;
   }
}


/**
 * Get current texture object for given name.
 * Return NULL if any error (and record the error).
 * Note that proxy targets are not accepted.
 * Only the glGetTexLevelParameter() functions accept proxy targets.
 */
static struct gl_texture_object *
get_texobj_by_name(struct gl_context *ctx, GLuint texture, const char *name)
{
   struct gl_texture_object *texObj;

   texObj = _mesa_lookup_texture_err(ctx, texture, name);
   if (!texObj)
      return NULL;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(target)", name);
      return NULL;
   }

   return texObj;
}


/**
 * Convert GL_RED/GREEN/BLUE/ALPHA/ZERO/ONE to SWIZZLE_X/Y/Z/W/ZERO/ONE.
 * \return -1 if error.
 */
static GLint
comp_to_swizzle(GLenum comp)
{
   switch (comp) {
   case GL_RED:
      return SWIZZLE_X;
   case GL_GREEN:
      return SWIZZLE_Y;
   case GL_BLUE:
      return SWIZZLE_Z;
   case GL_ALPHA:
      return SWIZZLE_W;
   case GL_ZERO:
      return SWIZZLE_ZERO;
   case GL_ONE:
      return SWIZZLE_ONE;
   default:
      return -1;
   }
}


static void
set_swizzle_component(GLushort *swizzle, GLuint comp, GLuint swz)
{
   assert(comp < 4);
   assert(swz <= SWIZZLE_NIL);
   {
      GLuint mask = 0x7 << (3 * comp);
      GLuint s = (*swizzle & ~mask) | (swz << (3 * comp));
      *swizzle = s;
   }
}


/**
 * This is called just prior to changing any texture object state which
 * will not affect texture completeness.
 */
static inline void
flush(struct gl_context *ctx)
{
   FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
}


/**
 * This is called just prior to changing any texture object state which
 * could affect texture completeness (texture base level, max level).
 * Any pending rendering will be flushed out, we'll set the _NEW_TEXTURE_OBJECT
 * state flag and then mark the texture object as 'incomplete' so that any
 * per-texture derived state gets recomputed.
 */
static inline void
incomplete(struct gl_context *ctx, struct gl_texture_object *texObj)
{
   FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
   _mesa_dirty_texobj(ctx, texObj);
}


GLboolean
_mesa_target_allows_setting_sampler_parameters(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_FALSE;

   default:
      return GL_TRUE;
   }
}


/**
 * Set an integer-valued texture parameter
 * \return GL_TRUE if legal AND the value changed, GL_FALSE otherwise
 */
static GLboolean
set_tex_parameteri(struct gl_context *ctx,
                   struct gl_texture_object *texObj,
                   GLenum pname, const GLint *params, bool dsa)
{
   const char *suffix = dsa ? "ture" : "";

   if (texObj->HandleAllocated) {
      /* The ARB_bindless_texture spec says:
       *
       * "The error INVALID_OPERATION is generated by TexImage*, CopyTexImage*,
       * CompressedTexImage*, TexBuffer*, TexParameter*, as well as other
       * functions defined in terms of these, if the texture object to be
       * modified is referenced by one or more texture or image handles."
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTex%sParameter(immutable texture)", suffix);
      return GL_FALSE;
   }

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.MinFilter == params[0])
         return GL_FALSE;
      switch (params[0]) {
      case GL_NEAREST:
      case GL_LINEAR:
         flush(ctx);
         texObj->Sampler.Attrib.MinFilter = params[0];
         texObj->Sampler.Attrib.state.min_img_filter = filter_to_gallium(params[0]);
         texObj->Sampler.Attrib.state.min_mip_filter = mipfilter_to_gallium(params[0]);
         _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
         return GL_TRUE;
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
      case GL_LINEAR_MIPMAP_LINEAR:
         if (texObj->Target != GL_TEXTURE_RECTANGLE_NV &&
             texObj->Target != GL_TEXTURE_EXTERNAL_OES) {
            flush(ctx);
            texObj->Sampler.Attrib.MinFilter = params[0];
            texObj->Sampler.Attrib.state.min_img_filter = filter_to_gallium(params[0]);
            texObj->Sampler.Attrib.state.min_mip_filter = mipfilter_to_gallium(params[0]);
            _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
            return GL_TRUE;
         }
         FALLTHROUGH;
      default:
         goto invalid_param;
      }
      return GL_FALSE;

   case GL_TEXTURE_MAG_FILTER:
      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.MagFilter == params[0])
         return GL_FALSE;
      switch (params[0]) {
      case GL_NEAREST:
      case GL_LINEAR:
         flush(ctx); /* does not effect completeness */
         texObj->Sampler.Attrib.MagFilter = params[0];
         texObj->Sampler.Attrib.state.mag_img_filter = filter_to_gallium(params[0]);
         _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
         return GL_TRUE;
      default:
         goto invalid_param;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_S:
      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.WrapS == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         update_sampler_gl_clamp(ctx, &texObj->Sampler, is_wrap_gl_clamp(texObj->Sampler.Attrib.WrapS), is_wrap_gl_clamp(params[0]), WRAP_S);
         texObj->Sampler.Attrib.WrapS = params[0];
         texObj->Sampler.Attrib.state.wrap_s = wrap_to_gallium(params[0]);
         _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_T:
      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.WrapT == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         update_sampler_gl_clamp(ctx, &texObj->Sampler, is_wrap_gl_clamp(texObj->Sampler.Attrib.WrapT), is_wrap_gl_clamp(params[0]), WRAP_T);
         texObj->Sampler.Attrib.WrapT = params[0];
         texObj->Sampler.Attrib.state.wrap_t = wrap_to_gallium(params[0]);
         _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_WRAP_R:
      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.WrapR == params[0])
         return GL_FALSE;
      if (validate_texture_wrap_mode(ctx, texObj->Target, params[0])) {
         flush(ctx);
         update_sampler_gl_clamp(ctx, &texObj->Sampler, is_wrap_gl_clamp(texObj->Sampler.Attrib.WrapR), is_wrap_gl_clamp(params[0]), WRAP_R);
         texObj->Sampler.Attrib.WrapR = params[0];
         texObj->Sampler.Attrib.state.wrap_r = wrap_to_gallium(params[0]);
         _mesa_lower_gl_clamp(ctx, &texObj->Sampler);
         return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_BASE_LEVEL:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (texObj->Attrib.BaseLevel == params[0])
         return GL_FALSE;

      /* Section 8.10 (Texture Parameters) of the OpenGL 4.5 Core Profile spec
       * says:
       *
       *    An INVALID_OPERATION error is generated if the effective target is
       *    TEXTURE_2D_MULTISAMPLE, TEXTURE_2D_MULTISAMPLE_ARRAY, or
       *    TEXTURE_RECTANGLE, and pname TEXTURE_BASE_LEVEL is set to a value
       *    other than zero.
       *
       * Note that section 3.8.8 (Texture Parameters) of the OpenGL 3.3 Core
       * Profile spec said:
       *
       *    The error INVALID_VALUE is generated if TEXTURE_BASE_LEVEL is set
       *    to any value other than zero.
       *
       * We take the 4.5 language as a correction to 3.3, and we implement
       * that on all GL versions.
       */
      if ((texObj->Target == GL_TEXTURE_2D_MULTISAMPLE ||
           texObj->Target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY ||
           texObj->Target == GL_TEXTURE_RECTANGLE) && params[0] != 0)
         goto invalid_operation;

      if (params[0] < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glTex%sParameter(param=%d)", suffix, params[0]);
         return GL_FALSE;
      }
      incomplete(ctx, texObj);

      /** See note about ARB_texture_storage below */
      if (texObj->Immutable)
         texObj->Attrib.BaseLevel = MIN2(texObj->Attrib.ImmutableLevels - 1, params[0]);
      else
         texObj->Attrib.BaseLevel = params[0];

      _mesa_update_teximage_format_swizzle(ctx, _mesa_base_tex_image(texObj), texObj->Attrib.DepthMode);
      _mesa_update_texture_object_swizzle(ctx, texObj);

      return GL_TRUE;

   case GL_TEXTURE_MAX_LEVEL:
      if (texObj->Attrib.MaxLevel == params[0])
         return GL_FALSE;

      if (params[0] < 0 ||
          (texObj->Target == GL_TEXTURE_RECTANGLE_ARB && params[0] > 0)) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glTex%sParameter(param=%d)", suffix,
                     params[0]);
         return GL_FALSE;
      }
      incomplete(ctx, texObj);

      /** From ARB_texture_storage:
       * However, if TEXTURE_IMMUTABLE_FORMAT is TRUE, then level_base is
       * clamped to the range [0, <levels> - 1] and level_max is then clamped to
       * the range [level_base, <levels> - 1], where <levels> is the parameter
       * passed the call to TexStorage* for the texture object.
       */
      if (texObj->Immutable)
          texObj->Attrib.MaxLevel = CLAMP(params[0], texObj->Attrib.BaseLevel,
                                   texObj->Attrib.ImmutableLevels - 1);
      else
         texObj->Attrib.MaxLevel = params[0];

      return GL_TRUE;

   case GL_GENERATE_MIPMAP_SGIS:
      if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
         goto invalid_pname;

      if (params[0] && texObj->Target == GL_TEXTURE_EXTERNAL_OES)
         goto invalid_param;
      if (texObj->Attrib.GenerateMipmap != params[0]) {
         /* no flush() */
	 texObj->Attrib.GenerateMipmap = params[0] ? GL_TRUE : GL_FALSE;
	 return GL_TRUE;
      }
      return GL_FALSE;

   case GL_TEXTURE_COMPARE_MODE_ARB:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_shadow)
          || _mesa_is_gles3(ctx)) {

         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

         if (texObj->Sampler.Attrib.CompareMode == params[0])
            return GL_FALSE;
         if (params[0] == GL_NONE ||
             params[0] == GL_COMPARE_R_TO_TEXTURE_ARB) {
            flush(ctx);
            texObj->Sampler.Attrib.CompareMode = params[0];
            return GL_TRUE;
         }
         goto invalid_param;
      }
      goto invalid_pname;

   case GL_TEXTURE_COMPARE_FUNC_ARB:
      if ((_mesa_is_desktop_gl(ctx) && ctx->Extensions.ARB_shadow)
          || _mesa_is_gles3(ctx)) {

         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

         if (texObj->Sampler.Attrib.CompareFunc == params[0])
            return GL_FALSE;
         switch (params[0]) {
         case GL_LEQUAL:
         case GL_GEQUAL:
         case GL_EQUAL:
         case GL_NOTEQUAL:
         case GL_LESS:
         case GL_GREATER:
         case GL_ALWAYS:
         case GL_NEVER:
            flush(ctx);
            texObj->Sampler.Attrib.CompareFunc = params[0];
            texObj->Sampler.Attrib.state.compare_func = func_to_gallium(params[0]);
            return GL_TRUE;
         default:
            goto invalid_param;
         }
      }
      goto invalid_pname;

   case GL_DEPTH_TEXTURE_MODE_ARB:
      /* GL_DEPTH_TEXTURE_MODE_ARB is removed in core-profile and it has never
       * existed in OpenGL ES.
       */
      if (_mesa_is_desktop_gl_compat(ctx)) {
         if (texObj->Attrib.DepthMode == params[0])
            return GL_FALSE;
         if (params[0] == GL_LUMINANCE ||
             params[0] == GL_INTENSITY ||
             params[0] == GL_ALPHA ||
             (ctx->Extensions.ARB_texture_rg && params[0] == GL_RED)) {
            flush(ctx);
            texObj->Attrib.DepthMode = params[0];
            _mesa_update_teximage_format_swizzle(ctx, _mesa_base_tex_image(texObj), texObj->Attrib.DepthMode);
            _mesa_update_texture_object_swizzle(ctx, texObj);
            return GL_TRUE;
         }
         goto invalid_param;
      }
      goto invalid_pname;

   case GL_DEPTH_STENCIL_TEXTURE_MODE:
      if (_mesa_has_ARB_stencil_texturing(ctx) || _mesa_is_gles31(ctx)) {
         bool stencil = params[0] == GL_STENCIL_INDEX;
         if (!stencil && params[0] != GL_DEPTH_COMPONENT)
            goto invalid_param;

         if (texObj->StencilSampling == stencil)
            return GL_FALSE;

         /* This should not be restored by glPopAttrib. */
         FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, 0);
         texObj->StencilSampling = stencil;
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_CROP_RECT_OES:
      if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
         goto invalid_pname;

      texObj->CropRect[0] = params[0];
      texObj->CropRect[1] = params[1];
      texObj->CropRect[2] = params[2];
      texObj->CropRect[3] = params[3];
      return GL_TRUE;

   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
      if (_mesa_has_EXT_texture_swizzle(ctx) || _mesa_is_gles3(ctx)) {
         const GLuint comp = pname - GL_TEXTURE_SWIZZLE_R_EXT;
         const GLint swz = comp_to_swizzle(params[0]);
         if (swz < 0) {
            _mesa_error(ctx, GL_INVALID_ENUM,
                        "glTex%sParameter(swizzle 0x%x)", suffix, params[0]);
            return GL_FALSE;
         }
         assert(comp < 4);

         flush(ctx);
         texObj->Attrib.Swizzle[comp] = params[0];
         set_swizzle_component(&texObj->Attrib._Swizzle, comp, swz);
         _mesa_update_texture_object_swizzle(ctx, texObj);
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_SWIZZLE_RGBA_EXT:
      if (_mesa_has_EXT_texture_swizzle(ctx) || _mesa_is_gles3(ctx)) {
         flush(ctx);
         for (GLuint comp = 0; comp < 4; comp++) {
            const GLint swz = comp_to_swizzle(params[comp]);
            if (swz >= 0) {
               texObj->Attrib.Swizzle[comp] = params[comp];
               set_swizzle_component(&texObj->Attrib._Swizzle, comp, swz);
               _mesa_update_texture_object_swizzle(ctx, texObj);
            }
            else {
               _mesa_error(ctx, GL_INVALID_ENUM,
                           "glTex%sParameter(swizzle 0x%x)",
                           suffix, params[comp]);
               return GL_FALSE;
            }
         }
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_SRGB_DECODE_EXT:
      if (ctx->Extensions.EXT_texture_sRGB_decode) {
         GLenum decode = params[0];

         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

	 if (decode == GL_DECODE_EXT || decode == GL_SKIP_DECODE_EXT) {
	    if (texObj->Sampler.Attrib.sRGBDecode != decode) {
	       flush(ctx);
	       texObj->Sampler.Attrib.sRGBDecode = decode;
	    }
	    return GL_TRUE;
	 }
      }
      goto invalid_pname;

   case GL_TEXTURE_REDUCTION_MODE_EXT:
      if (ctx->Extensions.EXT_texture_filter_minmax ||
          _mesa_has_ARB_texture_filter_minmax(ctx)) {
         GLenum mode = params[0];

         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

         if (mode == GL_WEIGHTED_AVERAGE_EXT || mode == GL_MIN || mode == GL_MAX) {
            if (texObj->Sampler.Attrib.ReductionMode != mode) {
               flush(ctx);
               texObj->Sampler.Attrib.ReductionMode = mode;
               texObj->Sampler.Attrib.state.reduction_mode = reduction_to_gallium(mode);
            }
            return GL_TRUE;
         }
      }
      goto invalid_pname;

   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
      if (_mesa_has_AMD_seamless_cubemap_per_texture(ctx)) {
         GLenum param = params[0];

         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

         if (param != GL_TRUE && param != GL_FALSE) {
            goto invalid_param;
         }
         if (param != texObj->Sampler.Attrib.CubeMapSeamless) {
            flush(ctx);
            texObj->Sampler.Attrib.CubeMapSeamless = param;
            texObj->Sampler.Attrib.state.seamless_cube_map = param;
         }
         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_TILING_EXT:
      if (ctx->Extensions.EXT_memory_object && !texObj->Immutable) {
            texObj->TextureTiling = params[0];

         return GL_TRUE;
      }
      goto invalid_pname;

   case GL_TEXTURE_SPARSE_ARB:
   case GL_VIRTUAL_PAGE_SIZE_INDEX_ARB:
      if (!_mesa_has_ARB_sparse_texture(ctx))
         goto invalid_pname;

      if (texObj->Immutable)
         goto invalid_operation;

      if (pname == GL_TEXTURE_SPARSE_ARB) {
         /* ARB_sparse_texture spec:
          *
          *   INVALID_VALUE is generated if <pname> is TEXTURE_SPARSE_ARB, <param>
          *   is TRUE and <target> is not one of TEXTURE_2D, TEXTURE_2D_ARRAY,
          *   TEXTURE_CUBE_MAP, TEXTURE_CUBE_MAP_ARRAY, TEXTURE_3D, or
          *   TEXTURE_RECTANGLE.
          *
          * ARB_sparse_texture2 also allow TEXTURE_2D_MULTISAMPLE and
          * TEXTURE_2D_MULTISAMPLE_ARRAY.
          */
         if (params[0] &&
             texObj->Target != GL_TEXTURE_2D &&
             texObj->Target != GL_TEXTURE_2D_ARRAY &&
             texObj->Target != GL_TEXTURE_CUBE_MAP &&
             texObj->Target != GL_TEXTURE_CUBE_MAP_ARRAY &&
             texObj->Target != GL_TEXTURE_3D &&
             texObj->Target != GL_TEXTURE_RECTANGLE &&
             (!_mesa_has_ARB_sparse_texture2(ctx) ||
              (texObj->Target != GL_TEXTURE_2D_MULTISAMPLE &&
               texObj->Target != GL_TEXTURE_2D_MULTISAMPLE_ARRAY))) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glTex%sParameter(target=%d)", suffix, texObj->Target);
            return GL_FALSE;
         }

         texObj->IsSparse = !!params[0];
      } else
         texObj->VirtualPageSizeIndex = params[0];

      return GL_TRUE;

   default:
      goto invalid_pname;
   }

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;

invalid_param:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameter(param=%s)",
               suffix, _mesa_enum_to_string(params[0]));
   return GL_FALSE;

invalid_dsa:
   if (!dsa)
      goto invalid_enum;

invalid_operation:
   _mesa_error(ctx, GL_INVALID_OPERATION, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;

invalid_enum:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;
}


/**
 * Set a float-valued texture parameter
 * \return GL_TRUE if legal AND the value changed, GL_FALSE otherwise
 */
static GLboolean
set_tex_parameterf(struct gl_context *ctx,
                   struct gl_texture_object *texObj,
                   GLenum pname, const GLfloat *params, bool dsa)
{
   const char *suffix = dsa ? "ture" : "";

   if (texObj->HandleAllocated) {
      /* The ARB_bindless_texture spec says:
       *
       * "The error INVALID_OPERATION is generated by TexImage*, CopyTexImage*,
       * CompressedTexImage*, TexBuffer*, TexParameter*, as well as other
       * functions defined in terms of these, if the texture object to be
       * modified is referenced by one or more texture or image handles."
       */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTex%sParameter(immutable texture)", suffix);
      return GL_FALSE;
   }

   switch (pname) {
   case GL_TEXTURE_MIN_LOD:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.MinLod == params[0])
         return GL_FALSE;
      flush(ctx);
      texObj->Sampler.Attrib.MinLod = params[0];
      texObj->Sampler.Attrib.state.min_lod = MAX2(params[0], 0.0f); /* only positive vals */
      return GL_TRUE;

   case GL_TEXTURE_MAX_LOD:
      if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
         goto invalid_pname;

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.MaxLod == params[0])
         return GL_FALSE;
      flush(ctx);
      texObj->Sampler.Attrib.MaxLod = params[0];
      texObj->Sampler.Attrib.state.max_lod = params[0];
      return GL_TRUE;

   case GL_TEXTURE_PRIORITY:
      if (ctx->API != API_OPENGL_COMPAT)
         goto invalid_pname;

      flush(ctx);
      texObj->Attrib.Priority = CLAMP(params[0], 0.0F, 1.0F);
      return GL_TRUE;

   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
      if (ctx->Extensions.EXT_texture_filter_anisotropic) {
         if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
            goto invalid_dsa;

         if (texObj->Sampler.Attrib.MaxAnisotropy == params[0])
            return GL_FALSE;
         if (params[0] < 1.0F) {
            _mesa_error(ctx, GL_INVALID_VALUE, "glTex%sParameter(param)",
                        suffix);
            return GL_FALSE;
         }
         flush(ctx);
         /* clamp to max, that's what NVIDIA does */
         texObj->Sampler.Attrib.MaxAnisotropy = MIN2(params[0],
                                      ctx->Const.MaxTextureMaxAnisotropy);
         texObj->Sampler.Attrib.state.max_anisotropy =
            texObj->Sampler.Attrib.MaxAnisotropy == 1 ?
                  0 : texObj->Sampler.Attrib.MaxAnisotropy; /* gallium sets 0 for 1 */
         return GL_TRUE;
      }
      else {
         static GLuint count = 0;
         if (count++ < 10)
            goto invalid_pname;
      }
      return GL_FALSE;

   case GL_TEXTURE_LOD_BIAS:
      /* NOTE: this is really part of OpenGL 1.4, not EXT_texture_lod_bias. */
      if (_mesa_is_gles(ctx))
         goto invalid_pname;

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_dsa;

      if (texObj->Sampler.Attrib.LodBias != params[0]) {
	 flush(ctx);
	 texObj->Sampler.Attrib.LodBias = params[0];
         texObj->Sampler.Attrib.state.lod_bias = util_quantize_lod_bias(params[0]);
	 return GL_TRUE;
      }
      break;

   case GL_TEXTURE_BORDER_COLOR:
      /* Border color exists in desktop OpenGL since 1.0 for GL_CLAMP.  In
       * OpenGL ES 2.0+, it only exists in when GL_OES_texture_border_clamp is
       * enabled.  It is never available in OpenGL ES 1.x.
       */
      if (_mesa_is_gles1(ctx))
         goto invalid_pname;

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target))
         goto invalid_enum;

      flush(ctx);
      /* ARB_texture_float disables clamping */
      if (ctx->Extensions.ARB_texture_float) {
         memcpy(texObj->Sampler.Attrib.state.border_color.f, params, 4 * sizeof(float));
      } else {
         texObj->Sampler.Attrib.state.border_color.f[RCOMP] = CLAMP(params[0], 0.0F, 1.0F);
         texObj->Sampler.Attrib.state.border_color.f[GCOMP] = CLAMP(params[1], 0.0F, 1.0F);
         texObj->Sampler.Attrib.state.border_color.f[BCOMP] = CLAMP(params[2], 0.0F, 1.0F);
         texObj->Sampler.Attrib.state.border_color.f[ACOMP] = CLAMP(params[3], 0.0F, 1.0F);
      }
      _mesa_update_is_border_color_nonzero(&texObj->Sampler);
      return GL_TRUE;

   case GL_TEXTURE_TILING_EXT:
      if (ctx->Extensions.EXT_memory_object) {
         texObj->TextureTiling = params[0];
         return GL_TRUE;
      }
      goto invalid_pname;

   default:
      goto invalid_pname;
   }
   return GL_FALSE;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;

invalid_dsa:
   if (!dsa)
      goto invalid_enum;
   _mesa_error(ctx, GL_INVALID_OPERATION, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;
invalid_enum:
   _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameter(pname=%s)",
               suffix, _mesa_enum_to_string(pname));
   return GL_FALSE;
}

static bool
texparam_invalidates_sampler_views(GLenum pname)
{
   switch (pname) {
      /*
       * Changing any of these texture parameters means we must create
       * new sampler views.
       */
   case GL_ALL_ATTRIB_BITS: /* meaning is all pnames, internal */
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
   case GL_DEPTH_TEXTURE_MODE:
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_SWIZZLE_R:
   case GL_TEXTURE_SWIZZLE_G:
   case GL_TEXTURE_SWIZZLE_B:
   case GL_TEXTURE_SWIZZLE_A:
   case GL_TEXTURE_SWIZZLE_RGBA:
   case GL_TEXTURE_BUFFER_SIZE:
   case GL_TEXTURE_BUFFER_OFFSET:
      return true;
   default:
      return false;
   }
}

static void
_mesa_texture_parameter_invalidate(struct gl_context *ctx,
                                   struct gl_texture_object *texObj,
                                   GLenum pname)
{
   if (texparam_invalidates_sampler_views(pname))
      st_texture_release_all_sampler_views(st_context(ctx), texObj);
}

void
_mesa_texture_parameterf(struct gl_context *ctx,
                         struct gl_texture_object *texObj,
                         GLenum pname, GLfloat param, bool dsa)
{
   GLboolean need_update;

   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
   case GL_GENERATE_MIPMAP_SGIS:
   case GL_TEXTURE_COMPARE_MODE_ARB:
   case GL_TEXTURE_COMPARE_FUNC_ARB:
   case GL_DEPTH_TEXTURE_MODE_ARB:
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_REDUCTION_MODE_EXT:
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
   case GL_TEXTURE_SPARSE_ARB:
   case GL_VIRTUAL_PAGE_SIZE_INDEX_ARB:
      {
         GLint p[4];
         p[0] = (param > 0) ?
                ((param > (float)INT32_MAX) ? INT32_MAX : (GLint) (param + 0.5)) :
                ((param < (float)INT32_MIN) ? INT32_MIN : (GLint) (param - 0.5));

         p[1] = p[2] = p[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, p, dsa);
      }
      break;
   case GL_TEXTURE_BORDER_COLOR:
   case GL_TEXTURE_SWIZZLE_RGBA:
      _mesa_error(ctx, GL_INVALID_ENUM, "glTex%sParameterf(non-scalar pname)",
                  dsa ? "ture" : "");
      return;
   default:
      {
         /* this will generate an error if pname is illegal */
         GLfloat p[4];
         p[0] = param;
         p[1] = p[2] = p[3] = 0.0F;
         need_update = set_tex_parameterf(ctx, texObj, pname, p, dsa);
      }
   }

   if (need_update) {
      _mesa_texture_parameter_invalidate(ctx, texObj, pname);
   }
}


void
_mesa_texture_parameterfv(struct gl_context *ctx,
                          struct gl_texture_object *texObj,
                          GLenum pname, const GLfloat *params, bool dsa)
{
   GLboolean need_update;
   switch (pname) {
   case GL_TEXTURE_MIN_FILTER:
   case GL_TEXTURE_MAG_FILTER:
   case GL_TEXTURE_WRAP_S:
   case GL_TEXTURE_WRAP_T:
   case GL_TEXTURE_WRAP_R:
   case GL_TEXTURE_BASE_LEVEL:
   case GL_TEXTURE_MAX_LEVEL:
   case GL_GENERATE_MIPMAP_SGIS:
   case GL_TEXTURE_COMPARE_MODE_ARB:
   case GL_TEXTURE_COMPARE_FUNC_ARB:
   case GL_DEPTH_TEXTURE_MODE_ARB:
   case GL_DEPTH_STENCIL_TEXTURE_MODE:
   case GL_TEXTURE_SRGB_DECODE_EXT:
   case GL_TEXTURE_REDUCTION_MODE_EXT:
   case GL_TEXTURE_CUBE_MAP_SEAMLESS:
   case GL_TEXTURE_SPARSE_ARB:
   case GL_VIRTUAL_PAGE_SIZE_INDEX_ARB:
      {
         /* convert float param to int */
         GLint p[4];
         p[0] = (GLint) params[0];
         p[1] = p[2] = p[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, p, dsa);
      }
      break;
   case GL_TEXTURE_CROP_RECT_OES:
      {
         /* convert float params to int */
         GLint iparams[4];
         iparams[0] = (GLint) params[0];
         iparams[1] = (GLint) params[1];
         iparams[2] = (GLint) params[2];
         iparams[3] = (GLint) params[3];
         need_update = set_tex_parameteri(ctx, texObj, pname, iparams, dsa);
      }
      break;
   case GL_TEXTURE_SWIZZLE_R_EXT:
   case GL_TEXTURE_SWIZZLE_G_EXT:
   case GL_TEXTURE_SWIZZLE_B_EXT:
   case GL_TEXTURE_SWIZZLE_A_EXT:
   case GL_TEXTURE_SWIZZLE_RGBA_EXT:
      {
         GLint p[4] = {0, 0, 0, 0};
         p[0] = (GLint) params[0];
         if (pname == GL_TEXTURE_SWIZZLE_RGBA_EXT) {
            p[1] = (GLint) params[1];
            p[2] = (GLint) params[2];
            p[3] = (GLint) params[3];
         }
         need_update = set_tex_parameteri(ctx, texObj, pname, p, dsa);
      }
      break;
   default:
      /* this will generate an error if pname is illegal */
      need_update = set_tex_parameterf(ctx, texObj, pname, params, dsa);
   }

   if (need_update) {
      _mesa_texture_parameter_invalidate(ctx, texObj, pname);
   }
}


void
_mesa_texture_parameteri(struct gl_context *ctx,
                         struct gl_texture_object *texObj,
                         GLenum pname, GLint param, bool dsa)
{
   GLboolean need_update;
   switch (pname) {
   case GL_TEXTURE_MIN_LOD:
   case GL_TEXTURE_MAX_LOD:
   case GL_TEXTURE_PRIORITY:
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
   case GL_TEXTURE_LOD_BIAS:
      {
         GLfloat fparam[4];
         fparam[0] = (GLfloat) param;
         fparam[1] = fparam[2] = fparam[3] = 0.0F;
         /* convert int param to float */
         need_update = set_tex_parameterf(ctx, texObj, pname, fparam, dsa);
      }
      break;
   case GL_TEXTURE_BORDER_COLOR:
   case GL_TEXTURE_SWIZZLE_RGBA:
      {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glTex%sParameteri(non-scalar pname)",
                     dsa ? "ture" : "");
         return;
      }
   default:
      /* this will generate an error if pname is illegal */
      {
         GLint iparam[4];
         iparam[0] = param;
         iparam[1] = iparam[2] = iparam[3] = 0;
         need_update = set_tex_parameteri(ctx, texObj, pname, iparam, dsa);
      }
   }

   if (need_update) {
      _mesa_texture_parameter_invalidate(ctx, texObj, pname);
   }
}


void
_mesa_texture_parameteriv(struct gl_context *ctx,
                          struct gl_texture_object *texObj,
                          GLenum pname, const GLint *params, bool dsa)
{
   GLboolean need_update;

   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      {
         /* convert int params to float */
         GLfloat fparams[4];
         fparams[0] = INT_TO_FLOAT(params[0]);
         fparams[1] = INT_TO_FLOAT(params[1]);
         fparams[2] = INT_TO_FLOAT(params[2]);
         fparams[3] = INT_TO_FLOAT(params[3]);
         need_update = set_tex_parameterf(ctx, texObj, pname, fparams, dsa);
      }
      break;
   case GL_TEXTURE_MIN_LOD:
   case GL_TEXTURE_MAX_LOD:
   case GL_TEXTURE_PRIORITY:
   case GL_TEXTURE_MAX_ANISOTROPY_EXT:
   case GL_TEXTURE_LOD_BIAS:
      {
         /* convert int param to float */
         GLfloat fparams[4];
         fparams[0] = (GLfloat) params[0];
         fparams[1] = fparams[2] = fparams[3] = 0.0F;
         need_update = set_tex_parameterf(ctx, texObj, pname, fparams, dsa);
      }
      break;
   default:
      /* this will generate an error if pname is illegal */
      need_update = set_tex_parameteri(ctx, texObj, pname, params, dsa);
   }

   if (need_update) {
      _mesa_texture_parameter_invalidate(ctx, texObj, pname);
   }
}

void
_mesa_texture_parameterIiv(struct gl_context *ctx,
                           struct gl_texture_object *texObj,
                           GLenum pname, const GLint *params, bool dsa)
{
   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      if (texObj->HandleAllocated) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTextureParameterIiv(immutable texture)");
         return;
      }

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target)) {
         _mesa_error(ctx, dsa ? GL_INVALID_OPERATION : GL_INVALID_ENUM, "glTextureParameterIiv(texture)");
         return;
      }
      FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
      /* set the integer-valued border color */
      COPY_4V(texObj->Sampler.Attrib.state.border_color.i, params);
      _mesa_update_is_border_color_nonzero(&texObj->Sampler);
      break;
   default:
      _mesa_texture_parameteriv(ctx, texObj, pname, params, dsa);
      break;
   }
   /* XXX no driver hook for TexParameterIiv() yet */
}

void
_mesa_texture_parameterIuiv(struct gl_context *ctx,
                            struct gl_texture_object *texObj,
                            GLenum pname, const GLuint *params, bool dsa)
{
   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      if (texObj->HandleAllocated) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTextureParameterIuiv(immutable texture)");
         return;
      }

      if (!_mesa_target_allows_setting_sampler_parameters(texObj->Target)) {
         _mesa_error(ctx, dsa ? GL_INVALID_OPERATION : GL_INVALID_ENUM, "glTextureParameterIuiv(texture)");
         return;
      }
      FLUSH_VERTICES(ctx, _NEW_TEXTURE_OBJECT, GL_TEXTURE_BIT);
      /* set the unsigned integer-valued border color */
      COPY_4V(texObj->Sampler.Attrib.state.border_color.ui, params);
      _mesa_update_is_border_color_nonzero(&texObj->Sampler);
      break;
   default:
      _mesa_texture_parameteriv(ctx, texObj, pname, (const GLint *) params,
                                dsa);
      break;
   }
   /* XXX no driver hook for TexParameterIuiv() yet */
}

void GLAPIENTRY
_mesa_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameterf");
   if (!texObj)
      return;

   _mesa_texture_parameterf(ctx, texObj, pname, param, false);
}

void GLAPIENTRY
_mesa_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameterfv");
   if (!texObj)
      return;

   _mesa_texture_parameterfv(ctx, texObj, pname, params, false);
}

void GLAPIENTRY
_mesa_TexParameteri(GLenum target, GLenum pname, GLint param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameteri");
   if (!texObj)
      return;

   _mesa_texture_parameteri(ctx, texObj, pname, param, false);
}

void GLAPIENTRY
_mesa_TexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameteriv");
   if (!texObj)
      return;

   _mesa_texture_parameteriv(ctx, texObj, pname, params, false);
}

/**
 * Set tex parameter to integer value(s).  Primarily intended to set
 * integer-valued texture border color (for integer-valued textures).
 * New in GL 3.0.
 */
void GLAPIENTRY
_mesa_TexParameterIiv(GLenum target, GLenum pname, const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameterIiv");
   if (!texObj)
      return;

   _mesa_texture_parameterIiv(ctx, texObj, pname, params, false);
}

/**
 * Set tex parameter to unsigned integer value(s).  Primarily intended to set
 * uint-valued texture border color (for integer-valued textures).
 * New in GL 3.0
 */
void GLAPIENTRY
_mesa_TexParameterIuiv(GLenum target, GLenum pname, const GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   ctx->Texture.CurrentUnit,
                                                   false,
                                                   "glTexParameterIuiv");
   if (!texObj)
      return;

   _mesa_texture_parameterIuiv(ctx, texObj, pname, params, false);
}

void GLAPIENTRY
_mesa_TextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, const GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameterfvEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTextureParameterfvEXT");
      return;
   }

   _mesa_texture_parameterfv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameterfv(GLuint texture, GLenum pname, const GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameterfv");
   if (!texObj)
      return;

   _mesa_texture_parameterfv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_MultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexParameterfvEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glMultiTexParameterifvEXT(target)");
      return;
   }

   _mesa_texture_parameterfv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameterfEXT(GLuint texture, GLenum target, GLenum pname, GLfloat param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameterfEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTextureParameterfEXT");
      return;
   }

   _mesa_texture_parameterf(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_MultiTexParameterfEXT(GLenum texunit, GLenum target, GLenum pname,
                            GLfloat param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexParameterfEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glMultiTexParameterfEXT");
      return;
   }

   _mesa_texture_parameterf(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_TextureParameterf(GLuint texture, GLenum pname, GLfloat param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameterf");
   if (!texObj)
      return;

   _mesa_texture_parameterf(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_TextureParameteriEXT(GLuint texture, GLenum target, GLenum pname, GLint param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameteriEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTextureParameteriEXT(target)");
      return;
   }

   _mesa_texture_parameteri(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_MultiTexParameteriEXT(GLenum texunit, GLenum target, GLenum pname,
                            GLint param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexParameteriEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glMultiTexParameteriEXT(target)");
      return;
   }

   _mesa_texture_parameteri(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_TextureParameteri(GLuint texture, GLenum pname, GLint param)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameteri");
   if (!texObj)
      return;

   _mesa_texture_parameteri(ctx, texObj, pname, param, true);
}

void GLAPIENTRY
_mesa_TextureParameterivEXT(GLuint texture, GLenum target, GLenum pname,
                         const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameterivEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glTextureParameterivEXT(target)");
      return;
   }

   _mesa_texture_parameteriv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_MultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname,
                             const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexParameterivEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glMultiTexParameterivEXT(target)");
      return;
   }

   _mesa_texture_parameteriv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameteriv(GLuint texture, GLenum pname,
                         const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameteriv");
   if (!texObj)
      return;

   _mesa_texture_parameteriv(ctx, texObj, pname, params, true);
}


void GLAPIENTRY
_mesa_TextureParameterIiv(GLuint texture, GLenum pname, const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameterIiv");
   if (!texObj)
      return;

   _mesa_texture_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname,
                             const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameterIivEXT");
   if (!texObj)
      return;

   _mesa_texture_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_MultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname,
                              const GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexParameterIivEXT");
   if (!texObj)
      return;

   _mesa_texture_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameterIuiv(GLuint texture, GLenum pname, const GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glTextureParameterIuiv");
   if (!texObj)
      return;

   _mesa_texture_parameterIuiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_TextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname,
                              const GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureParameterIuivEXT");
   if (!texObj)
      return;

   _mesa_texture_parameterIuiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_MultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname,
                               const GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexParameterIuivEXT");
   if (!texObj)
      return;

   _mesa_texture_parameterIuiv(ctx, texObj, pname, params, true);
}

GLboolean
_mesa_legal_get_tex_level_parameter_target(struct gl_context *ctx, GLenum target,
                                           bool dsa)
{
   /* Common targets for desktop GL and GLES 3.1. */
   switch (target) {
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
      return GL_TRUE;
   case GL_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      return GL_TRUE;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return ctx->Extensions.ARB_texture_multisample;
   case GL_TEXTURE_BUFFER:
      /* GetTexLevelParameter accepts GL_TEXTURE_BUFFER in GL 3.1+ contexts,
       * but not in earlier versions that expose ARB_texture_buffer_object.
       *
       * From the ARB_texture_buffer_object spec:
       * "(7) Do buffer textures support texture parameters (TexParameter) or
       *      queries (GetTexParameter, GetTexLevelParameter, GetTexImage)?
       *
       *    RESOLVED:  No. [...] Note that the spec edits above don't add
       *    explicit error language for any of these cases.  That is because
       *    each of the functions enumerate the set of valid <target>
       *    parameters.  Not editing the spec to allow TEXTURE_BUFFER_ARB in
       *    these cases means that target is not legal, and an INVALID_ENUM
       *    error should be generated."
       *
       * From the OpenGL 3.1 spec:
       * "target may also be TEXTURE_BUFFER, indicating the texture buffer."
       *
       * From ARB_texture_buffer_range, GL_TEXTURE is a valid target in
       * GetTexLevelParameter.
       */
      return (_mesa_is_desktop_gl(ctx) && ctx->Version >= 31) ||
             _mesa_has_OES_texture_buffer(ctx) ||
             _mesa_has_ARB_texture_buffer_range(ctx);
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return _mesa_has_texture_cube_map_array(ctx);
   }

   if (!_mesa_is_desktop_gl(ctx))
      return GL_FALSE;

   /* Rest of the desktop GL targets. */
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_3D:
   case GL_PROXY_TEXTURE_CUBE_MAP:
      return GL_TRUE;
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      return ctx->Extensions.ARB_texture_cube_map_array;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return ctx->Extensions.NV_texture_rectangle;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return ctx->Extensions.ARB_texture_multisample;

   /*  This is a valid target for dsa, but the OpenGL 4.5 core spec
    *  (30.10.2014) Section 8.11 Texture Queries says:
    *       "For GetTextureLevelParameter* only, texture may also be a cube
    *       map texture object.  In this case the query is always performed
    *       for face zero (the TEXTURE_CUBE_MAP_POSITIVE_X face), since there
    *       is no way to specify another face."
    */
   case GL_TEXTURE_CUBE_MAP:
      return dsa;
   default:
      return GL_FALSE;
   }
}


static void
get_tex_level_parameter_image(struct gl_context *ctx,
                              const struct gl_texture_object *texObj,
                              GLenum target, GLint level,
                              GLenum pname, GLint *params,
                              bool dsa)
{
   const struct gl_texture_image *img = NULL;
   struct gl_texture_image dummy_image;
   mesa_format texFormat;
   const char *suffix = dsa ? "ture" : "";

   img = _mesa_select_tex_image(texObj, target, level);
   if (!img || img->TexFormat == MESA_FORMAT_NONE) {
      /* In case of undefined texture image return the default values.
       *
       * From OpenGL 4.0 spec, page 398:
       *    "The initial internal format of a texel array is RGBA
       *     instead of 1. TEXTURE_COMPONENTS is deprecated; always
       *     use TEXTURE_INTERNAL_FORMAT."
       */
      memset(&dummy_image, 0, sizeof(dummy_image));
      dummy_image.TexFormat = MESA_FORMAT_NONE;
      dummy_image.InternalFormat = GL_RGBA;
      dummy_image._BaseFormat = GL_NONE;
      dummy_image.FixedSampleLocations = GL_TRUE;

      img = &dummy_image;
   }

   texFormat = img->TexFormat;

   switch (pname) {
      case GL_TEXTURE_WIDTH:
         *params = img->Width;
         break;
      case GL_TEXTURE_HEIGHT:
         *params = img->Height;
         break;
      case GL_TEXTURE_DEPTH:
         *params = img->Depth;
         break;
      case GL_TEXTURE_INTERNAL_FORMAT:
         if (_mesa_is_format_compressed(texFormat)) {
            /* need to return the actual compressed format */
            *params = _mesa_compressed_format_to_glenum(ctx, texFormat);
         }
         else {
	    /* If the true internal format is not compressed but the user
	     * requested a generic compressed format, we have to return the
	     * generic base format that matches.
	     *
	     * From page 119 (page 129 of the PDF) of the OpenGL 1.3 spec:
	     *
	     *     "If no specific compressed format is available,
	     *     internalformat is instead replaced by the corresponding base
	     *     internal format."
	     *
	     * Otherwise just return the user's requested internal format
	     */
	    const GLenum f =
	       _mesa_gl_compressed_format_base_format(img->InternalFormat);

	    *params = (f != 0) ? f : img->InternalFormat;
	 }
         break;
      case GL_TEXTURE_BORDER:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = img->Border;
         break;
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
         if (_mesa_base_format_has_channel(img->_BaseFormat, pname))
            *params = _mesa_get_format_bits(texFormat, pname);
         else
            *params = 0;
         break;
      case GL_TEXTURE_INTENSITY_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         if (_mesa_base_format_has_channel(img->_BaseFormat, pname)) {
            *params = _mesa_get_format_bits(texFormat, pname);
            if (*params == 0) {
               /* intensity or luminance is probably stored as RGB[A] */
               *params = MIN2(_mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_RED_SIZE),
                              _mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_GREEN_SIZE));
            }
            if (*params == 0 && pname == GL_TEXTURE_INTENSITY_SIZE) {
               /* Gallium may store intensity as LA */
               *params = _mesa_get_format_bits(texFormat,
                                               GL_TEXTURE_ALPHA_SIZE);
            }
         }
         else {
            *params = 0;
         }
         break;
      case GL_TEXTURE_DEPTH_SIZE_ARB:
         *params = _mesa_get_format_bits(texFormat, pname);
         break;
      case GL_TEXTURE_STENCIL_SIZE:
         *params = _mesa_get_format_bits(texFormat, pname);
         break;
      case GL_TEXTURE_SHARED_SIZE:
         if (ctx->Version < 30 &&
             !ctx->Extensions.EXT_texture_shared_exponent)
            goto invalid_pname;
         *params = texFormat == MESA_FORMAT_R9G9B9E5_FLOAT ? 5 : 0;
         break;

      /* GL_ARB_texture_compression */
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
         if (_mesa_is_format_compressed(texFormat) &&
             !_mesa_is_proxy_texture(target)) {
            *params = _mesa_format_image_size(texFormat, img->Width,
                                              img->Height, img->Depth);
         } else {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glGetTex%sLevelParameter[if]v(pname=%s)", suffix,
                        _mesa_enum_to_string(pname));
         }
         break;
      case GL_TEXTURE_COMPRESSED:
         *params = (GLint) _mesa_is_format_compressed(texFormat);
         break;

      /* GL_ARB_texture_float */
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         FALLTHROUGH;
      case GL_TEXTURE_RED_TYPE_ARB:
      case GL_TEXTURE_GREEN_TYPE_ARB:
      case GL_TEXTURE_BLUE_TYPE_ARB:
      case GL_TEXTURE_ALPHA_TYPE_ARB:
      case GL_TEXTURE_DEPTH_TYPE_ARB:
         if (!ctx->Extensions.ARB_texture_float)
            goto invalid_pname;
	 if (_mesa_base_format_has_channel(img->_BaseFormat, pname))
	    *params = _mesa_get_format_datatype(texFormat);
	 else
	    *params = GL_NONE;
         break;

      /* GL_ARB_texture_multisample */
      case GL_TEXTURE_SAMPLES:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = img->NumSamples;
         break;

      case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = img->FixedSampleLocations;
         break;

      /* There is never a buffer data store here, but these pnames still have
       * to work.
       */

      /* GL_ARB_texture_buffer_object */
      case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
         if (!ctx->Extensions.ARB_texture_buffer_object)
            goto invalid_pname;
         *params = 0;
         break;

      /* GL_ARB_texture_buffer_range */
      case GL_TEXTURE_BUFFER_OFFSET:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = 0;
         break;
      case GL_TEXTURE_BUFFER_SIZE:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = 0;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM,
               "glGetTex%sLevelParameter[if]v(pname=%s)", suffix,
               _mesa_enum_to_string(pname));
}


/**
 * Handle a glGetTexLevelParamteriv() call for a texture buffer.
 */
static void
get_tex_level_parameter_buffer(struct gl_context *ctx,
                               const struct gl_texture_object *texObj,
                               GLenum pname, GLint *params, bool dsa)
{
   const struct gl_buffer_object *bo = texObj->BufferObject;
   mesa_format texFormat = texObj->_BufferObjectFormat;
   int bytes = MAX2(1, _mesa_get_format_bytes(texFormat));
   GLenum internalFormat = texObj->BufferObjectFormat;
   GLenum baseFormat = _mesa_get_format_base_format(texFormat);
   const char *suffix = dsa ? "ture" : "";

   assert(texObj->Target == GL_TEXTURE_BUFFER);

   if (!bo) {
      /* undefined texture buffer object */
      switch (pname) {
      case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
         *params = GL_TRUE;
         break;
      case GL_TEXTURE_INTERNAL_FORMAT:
         *params = internalFormat;
         break;
      default:
         *params = 0;
         break;
      }
      return;
   }

   switch (pname) {
      case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
         *params = bo->Name;
         break;
      case GL_TEXTURE_WIDTH:
         *params = ((texObj->BufferSize == -1) ? bo->Size : texObj->BufferSize)
            / bytes;
         break;
      case GL_TEXTURE_HEIGHT:
      case GL_TEXTURE_DEPTH:
         *params = 1;
         break;
      case GL_TEXTURE_BORDER:
      case GL_TEXTURE_SHARED_SIZE:
      case GL_TEXTURE_COMPRESSED:
         *params = 0;
         break;
      case GL_TEXTURE_INTERNAL_FORMAT:
         *params = internalFormat;
         break;
      case GL_TEXTURE_RED_SIZE:
      case GL_TEXTURE_GREEN_SIZE:
      case GL_TEXTURE_BLUE_SIZE:
      case GL_TEXTURE_ALPHA_SIZE:
         if (_mesa_base_format_has_channel(baseFormat, pname))
            *params = _mesa_get_format_bits(texFormat, pname);
         else
            *params = 0;
         break;
      case GL_TEXTURE_INTENSITY_SIZE:
      case GL_TEXTURE_LUMINANCE_SIZE:
         if (_mesa_base_format_has_channel(baseFormat, pname)) {
            *params = _mesa_get_format_bits(texFormat, pname);
            if (*params == 0) {
               /* intensity or luminance is probably stored as RGB[A] */
               *params = MIN2(_mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_RED_SIZE),
                              _mesa_get_format_bits(texFormat,
                                                    GL_TEXTURE_GREEN_SIZE));
            }
         } else {
            *params = 0;
         }
         break;
      case GL_TEXTURE_DEPTH_SIZE_ARB:
      case GL_TEXTURE_STENCIL_SIZE_EXT:
         *params = _mesa_get_format_bits(texFormat, pname);
         break;

      /* GL_ARB_texture_buffer_range */
      case GL_TEXTURE_BUFFER_OFFSET:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = texObj->BufferOffset;
         break;
      case GL_TEXTURE_BUFFER_SIZE:
         if (!ctx->Extensions.ARB_texture_buffer_range)
            goto invalid_pname;
         *params = (texObj->BufferSize == -1) ? bo->Size : texObj->BufferSize;
         break;

      /* GL_ARB_texture_multisample */
      case GL_TEXTURE_SAMPLES:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = 0;
         break;

      case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
         if (!ctx->Extensions.ARB_texture_multisample)
            goto invalid_pname;
         *params = GL_TRUE;
         break;

      /* GL_ARB_texture_compression */
      case GL_TEXTURE_COMPRESSED_IMAGE_SIZE:
         /* Always illegal for GL_TEXTURE_BUFFER */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glGetTex%sLevelParameter[if]v(pname=%s)", suffix,
                     _mesa_enum_to_string(pname));
         break;

      /* GL_ARB_texture_float */
      case GL_TEXTURE_RED_TYPE_ARB:
      case GL_TEXTURE_GREEN_TYPE_ARB:
      case GL_TEXTURE_BLUE_TYPE_ARB:
      case GL_TEXTURE_ALPHA_TYPE_ARB:
      case GL_TEXTURE_LUMINANCE_TYPE_ARB:
      case GL_TEXTURE_INTENSITY_TYPE_ARB:
      case GL_TEXTURE_DEPTH_TYPE_ARB:
         if (!ctx->Extensions.ARB_texture_float)
            goto invalid_pname;
         if (_mesa_base_format_has_channel(baseFormat, pname))
            *params = _mesa_get_format_datatype(texFormat);
         else
            *params = GL_NONE;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   return;

invalid_pname:
   _mesa_error(ctx, GL_INVALID_ENUM,
               "glGetTex%sLevelParameter[if]v(pname=%s)", suffix,
               _mesa_enum_to_string(pname));
}

static bool
valid_tex_level_parameteriv_target(struct gl_context *ctx, GLenum target,
                                   bool dsa)
{
   const char *suffix = dsa ? "ture" : "";
   if (!_mesa_legal_get_tex_level_parameter_target(ctx, target, dsa)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glGetTex%sLevelParameter[if]v(target=%s)", suffix,
                  _mesa_enum_to_string(target));
      return false;
   }
   return true;
}

/**
 * This isn't exposed to the rest of the driver because it is a part of the
 * OpenGL API that is rarely used.
 */
static void
get_tex_level_parameteriv(struct gl_context *ctx,
                          struct gl_texture_object *texObj,
                          GLenum target, GLint level,
                          GLenum pname, GLint *params,
                          bool dsa)
{
   GLint maxLevels;
   const char *suffix = dsa ? "ture" : "";

   /* Check for errors */
   if (ctx->Texture.CurrentUnit >= ctx->Const.MaxCombinedTextureImageUnits) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glGetTex%sLevelParameter[if]v("
                  "current unit >= max combined texture units)", suffix);
      return;
   }

   maxLevels = _mesa_max_texture_levels(ctx, target);
   assert(maxLevels != 0);

   if (level < 0 || level >= maxLevels) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glGetTex%sLevelParameter[if]v(level out of range)", suffix);
      return;
   }

   /* Get the level parameter */
   if (target == GL_TEXTURE_BUFFER) {
      get_tex_level_parameter_buffer(ctx, texObj, pname, params, dsa);
   }
   else {
      get_tex_level_parameter_image(ctx, texObj, target,
                                    level, pname, params, dsa);
   }
}

void GLAPIENTRY
_mesa_GetTexLevelParameterfv( GLenum target, GLint level,
                              GLenum pname, GLfloat *params )
{
   struct gl_texture_object *texObj;
   GLint iparam;
   GET_CURRENT_CONTEXT(ctx);

   if (!valid_tex_level_parameteriv_target(ctx, target, false))
      return;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   get_tex_level_parameteriv(ctx, texObj, target, level,
                             pname, &iparam, false);

   *params = (GLfloat) iparam;
}

void GLAPIENTRY
_mesa_GetTexLevelParameteriv( GLenum target, GLint level,
                              GLenum pname, GLint *params )
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   if (!valid_tex_level_parameteriv_target(ctx, target, false))
      return;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   get_tex_level_parameteriv(ctx, texObj, target, level,
                             pname, params, false);
}

void GLAPIENTRY
_mesa_GetTextureLevelParameterfv(GLuint texture, GLint level,
                                 GLenum pname, GLfloat *params)
{
   struct gl_texture_object *texObj;
   GLint iparam;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glGetTextureLevelParameterfv");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, &iparam, true);

   *params = (GLfloat) iparam;
}

void GLAPIENTRY
_mesa_GetTextureLevelParameterfvEXT(GLuint texture, GLenum target, GLint level,
                                    GLenum pname, GLfloat *params)
{
   struct gl_texture_object *texObj;
   GLint iparam;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureLevelParameterfvEXT");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, &iparam, true);

   *params = (GLfloat) iparam;
}

void GLAPIENTRY
_mesa_GetMultiTexLevelParameterfvEXT(GLenum texunit, GLenum target, GLint level,
                                     GLenum pname, GLfloat *params)
{
   struct gl_texture_object *texObj;
   GLint iparam;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glGetMultiTexLevelParameterfvEXT");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, &iparam, true);

   *params = (GLfloat) iparam;
}

void GLAPIENTRY
_mesa_GetTextureLevelParameteriv(GLuint texture, GLint level,
                                 GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glGetTextureLevelParameteriv");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureLevelParameterivEXT(GLuint texture, GLenum target, GLint level,
                                    GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureLevelParameterivEXT");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, params, true);
}

void GLAPIENTRY
_mesa_GetMultiTexLevelParameterivEXT(GLenum texunit, GLenum target, GLint level,
                                     GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glGetMultiTexLevelParameterivEXT");
   if (!texObj)
      return;

   if (!valid_tex_level_parameteriv_target(ctx, texObj->Target, true))
      return;

   get_tex_level_parameteriv(ctx, texObj, texObj->Target, level,
                             pname, params, true);
}


/**
 * This isn't exposed to the rest of the driver because it is a part of the
 * OpenGL API that is rarely used.
 */
static void
get_tex_parameterfv(struct gl_context *ctx,
                    struct gl_texture_object *obj,
                    GLenum pname, GLfloat *params, bool dsa)
{
   _mesa_lock_context_textures(ctx);
   switch (pname) {
      case GL_TEXTURE_MAG_FILTER:
	 *params = ENUM_TO_FLOAT(obj->Sampler.Attrib.MagFilter);
	 break;
      case GL_TEXTURE_MIN_FILTER:
         *params = ENUM_TO_FLOAT(obj->Sampler.Attrib.MinFilter);
         break;
      case GL_TEXTURE_WRAP_S:
         *params = ENUM_TO_FLOAT(obj->Sampler.Attrib.WrapS);
         break;
      case GL_TEXTURE_WRAP_T:
         *params = ENUM_TO_FLOAT(obj->Sampler.Attrib.WrapT);
         break;
      case GL_TEXTURE_WRAP_R:
         *params = ENUM_TO_FLOAT(obj->Sampler.Attrib.WrapR);
         break;
      case GL_TEXTURE_BORDER_COLOR:
         if (_mesa_is_gles1(ctx))
            goto invalid_pname;

         if (_mesa_get_clamp_fragment_color(ctx, ctx->DrawBuffer)) {
            params[0] = CLAMP(obj->Sampler.Attrib.state.border_color.f[0], 0.0F, 1.0F);
            params[1] = CLAMP(obj->Sampler.Attrib.state.border_color.f[1], 0.0F, 1.0F);
            params[2] = CLAMP(obj->Sampler.Attrib.state.border_color.f[2], 0.0F, 1.0F);
            params[3] = CLAMP(obj->Sampler.Attrib.state.border_color.f[3], 0.0F, 1.0F);
         }
         else {
            params[0] = obj->Sampler.Attrib.state.border_color.f[0];
            params[1] = obj->Sampler.Attrib.state.border_color.f[1];
            params[2] = obj->Sampler.Attrib.state.border_color.f[2];
            params[3] = obj->Sampler.Attrib.state.border_color.f[3];
         }
         break;
      case GL_TEXTURE_RESIDENT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = 1.0F;
         break;
      case GL_TEXTURE_PRIORITY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = obj->Attrib.Priority;
         break;
      case GL_TEXTURE_MIN_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->Sampler.Attrib.MinLod;
         break;
      case GL_TEXTURE_MAX_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->Sampler.Attrib.MaxLod;
         break;
      case GL_TEXTURE_BASE_LEVEL:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = (GLfloat) obj->Attrib.BaseLevel;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         *params = (GLfloat) obj->Attrib.MaxLevel;
         break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
         if (!ctx->Extensions.EXT_texture_filter_anisotropic)
            goto invalid_pname;
         *params = obj->Sampler.Attrib.MaxAnisotropy;
         break;
      case GL_GENERATE_MIPMAP_SGIS:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;

	 *params = (GLfloat) obj->Attrib.GenerateMipmap;
         break;
      case GL_TEXTURE_COMPARE_MODE_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.Attrib.CompareMode;
         break;
      case GL_TEXTURE_COMPARE_FUNC_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.Attrib.CompareFunc;
         break;
      case GL_DEPTH_TEXTURE_MODE_ARB:
         /* GL_DEPTH_TEXTURE_MODE_ARB is removed in core-profile and it has
          * never existed in OpenGL ES.
          */
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.DepthMode;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         if (!_mesa_has_ARB_stencil_texturing(ctx) && !_mesa_is_gles31(ctx))
            goto invalid_pname;
         *params = (GLfloat)
            (obj->StencilSampling ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
         break;
      case GL_TEXTURE_LOD_BIAS:
         if (_mesa_is_gles(ctx))
            goto invalid_pname;

         *params = obj->Sampler.Attrib.LodBias;
         break;
      case GL_TEXTURE_CROP_RECT_OES:
         if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
            goto invalid_pname;

         params[0] = (GLfloat) obj->CropRect[0];
         params[1] = (GLfloat) obj->CropRect[1];
         params[2] = (GLfloat) obj->CropRect[2];
         params[3] = (GLfloat) obj->CropRect[3];
         break;

      case GL_TEXTURE_SWIZZLE_R_EXT:
      case GL_TEXTURE_SWIZZLE_G_EXT:
      case GL_TEXTURE_SWIZZLE_B_EXT:
      case GL_TEXTURE_SWIZZLE_A_EXT:
         if (!_mesa_has_EXT_texture_swizzle(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.Swizzle[pname - GL_TEXTURE_SWIZZLE_R_EXT];
         break;

      case GL_TEXTURE_SWIZZLE_RGBA_EXT:
         if (!_mesa_has_EXT_texture_swizzle(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         for (GLuint comp = 0; comp < 4; comp++)
            params[comp] = (GLfloat) obj->Attrib.Swizzle[comp];
         break;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (!_mesa_has_AMD_seamless_cubemap_per_texture(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.Attrib.CubeMapSeamless;
         break;

      case GL_TEXTURE_IMMUTABLE_FORMAT:
         *params = (GLfloat) obj->Immutable;
         break;

      case GL_TEXTURE_IMMUTABLE_LEVELS:
         if (_mesa_is_gles3(ctx) || _mesa_has_texture_view(ctx))
            *params = (GLfloat) obj->Attrib.ImmutableLevels;
         else
            goto invalid_pname;
         break;

      case GL_TEXTURE_VIEW_MIN_LEVEL:
         if (!_mesa_has_texture_view(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.MinLevel;
         break;

      case GL_TEXTURE_VIEW_NUM_LEVELS:
         if (!_mesa_has_texture_view(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.NumLevels;
         break;

      case GL_TEXTURE_VIEW_MIN_LAYER:
         if (!_mesa_has_texture_view(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.MinLayer;
         break;

      case GL_TEXTURE_VIEW_NUM_LAYERS:
         if (!_mesa_has_texture_view(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.NumLayers;
         break;

      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         if (!_mesa_is_gles(ctx) || !ctx->Extensions.OES_EGL_image_external)
            goto invalid_pname;
         *params = (GLfloat) obj->RequiredTextureImageUnits;
         break;

      case GL_TEXTURE_SRGB_DECODE_EXT:
         if (!ctx->Extensions.EXT_texture_sRGB_decode)
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.Attrib.sRGBDecode;
         break;

      case GL_TEXTURE_REDUCTION_MODE_EXT:
         if (!ctx->Extensions.EXT_texture_filter_minmax &&
             !_mesa_has_ARB_texture_filter_minmax(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Sampler.Attrib.ReductionMode;
         break;

      case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
         if (!ctx->Extensions.ARB_shader_image_load_store &&
             !_mesa_is_gles31(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->Attrib.ImageFormatCompatibilityType;
         break;

      case GL_TEXTURE_TARGET:
         if (ctx->API != API_OPENGL_CORE)
            goto invalid_pname;
         *params = ENUM_TO_FLOAT(obj->Target);
         break;

      case GL_TEXTURE_TILING_EXT:
         if (!ctx->Extensions.EXT_memory_object)
            goto invalid_pname;
         *params = ENUM_TO_FLOAT(obj->TextureTiling);
         break;

      case GL_TEXTURE_SPARSE_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->IsSparse;
         break;

      case GL_VIRTUAL_PAGE_SIZE_INDEX_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->VirtualPageSizeIndex;
         break;

      case GL_NUM_SPARSE_LEVELS_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = (GLfloat) obj->NumSparseLevels;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   _mesa_unlock_context_textures(ctx);
   return;

invalid_pname:
   _mesa_unlock_context_textures(ctx);
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetTex%sParameterfv(pname=0x%x)",
               dsa ? "ture" : "", pname);
}


static void
get_tex_parameteriv(struct gl_context *ctx,
                    struct gl_texture_object *obj,
                    GLenum pname, GLint *params, bool dsa)
{
   _mesa_lock_texture(ctx, obj);
   switch (pname) {
      case GL_TEXTURE_MAG_FILTER:
         *params = (GLint) obj->Sampler.Attrib.MagFilter;
         break;
      case GL_TEXTURE_MIN_FILTER:
         *params = (GLint) obj->Sampler.Attrib.MinFilter;
         break;
      case GL_TEXTURE_WRAP_S:
         *params = (GLint) obj->Sampler.Attrib.WrapS;
         break;
      case GL_TEXTURE_WRAP_T:
         *params = (GLint) obj->Sampler.Attrib.WrapT;
         break;
      case GL_TEXTURE_WRAP_R:
         *params = (GLint) obj->Sampler.Attrib.WrapR;
         break;
      case GL_TEXTURE_BORDER_COLOR:
         if (_mesa_is_gles1(ctx))
            goto invalid_pname;

         {
            GLfloat b[4];
            b[0] = CLAMP(obj->Sampler.Attrib.state.border_color.f[0], 0.0F, 1.0F);
            b[1] = CLAMP(obj->Sampler.Attrib.state.border_color.f[1], 0.0F, 1.0F);
            b[2] = CLAMP(obj->Sampler.Attrib.state.border_color.f[2], 0.0F, 1.0F);
            b[3] = CLAMP(obj->Sampler.Attrib.state.border_color.f[3], 0.0F, 1.0F);
            params[0] = FLOAT_TO_INT(b[0]);
            params[1] = FLOAT_TO_INT(b[1]);
            params[2] = FLOAT_TO_INT(b[2]);
            params[3] = FLOAT_TO_INT(b[3]);
         }
         break;
      case GL_TEXTURE_RESIDENT:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = 1;
         break;
      case GL_TEXTURE_PRIORITY:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;

         *params = FLOAT_TO_INT(obj->Attrib.Priority);
         break;
      case GL_TEXTURE_MIN_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         /* GL spec 'Data Conversions' section specifies that floating-point
          * value in integer Get function is rounded to nearest integer
          *
          * Section 2.2.2 (Data Conversions For State Query Commands) of the
          * OpenGL 4.5 spec says:
          *
          *   Following these steps, if a value is so large in magnitude that
          *   it cannot be represented by the returned data type, then the
          *   nearest value representable using that type is returned.
          */
         *params = LCLAMPF(obj->Sampler.Attrib.MinLod, INT32_MIN, INT32_MAX);
         break;
      case GL_TEXTURE_MAX_LOD:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         /* GL spec 'Data Conversions' section specifies that floating-point
          * value in integer Get function is rounded to nearest integer
          *
          * Section 2.2.2 (Data Conversions For State Query Commands) of the
          * OpenGL 4.5 spec says:
          *
          *   Following these steps, if a value is so large in magnitude that
          *   it cannot be represented by the returned data type, then the
          *   nearest value representable using that type is returned.
          */
         *params = LCLAMPF(obj->Sampler.Attrib.MaxLod, INT32_MIN, INT32_MAX);
         break;
      case GL_TEXTURE_BASE_LEVEL:
         if (!_mesa_is_desktop_gl(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;

         *params = obj->Attrib.BaseLevel;
         break;
      case GL_TEXTURE_MAX_LEVEL:
         *params = obj->Attrib.MaxLevel;
         break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
         if (!ctx->Extensions.EXT_texture_filter_anisotropic)
            goto invalid_pname;
         /* GL spec 'Data Conversions' section specifies that floating-point
          * value in integer Get function is rounded to nearest integer
          *
          * Section 2.2.2 (Data Conversions For State Query Commands) of the
          * OpenGL 4.5 spec says:
          *
          *   Following these steps, if a value is so large in magnitude that
          *   it cannot be represented by the returned data type, then the
          *   nearest value representable using that type is returned.
          */
         *params = LCLAMPF(obj->Sampler.Attrib.MaxAnisotropy, INT32_MIN, INT32_MAX);
         break;
      case GL_GENERATE_MIPMAP_SGIS:
         if (ctx->API != API_OPENGL_COMPAT && ctx->API != API_OPENGLES)
            goto invalid_pname;

	 *params = (GLint) obj->Attrib.GenerateMipmap;
         break;
      case GL_TEXTURE_COMPARE_MODE_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLint) obj->Sampler.Attrib.CompareMode;
         break;
      case GL_TEXTURE_COMPARE_FUNC_ARB:
         if ((!_mesa_is_desktop_gl(ctx) || !ctx->Extensions.ARB_shadow)
             && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = (GLint) obj->Sampler.Attrib.CompareFunc;
         break;
      case GL_DEPTH_TEXTURE_MODE_ARB:
         if (ctx->API != API_OPENGL_COMPAT)
            goto invalid_pname;
         *params = (GLint) obj->Attrib.DepthMode;
         break;
      case GL_DEPTH_STENCIL_TEXTURE_MODE:
         if (!_mesa_has_ARB_stencil_texturing(ctx) && !_mesa_is_gles31(ctx))
            goto invalid_pname;
         *params = (GLint)
            (obj->StencilSampling ? GL_STENCIL_INDEX : GL_DEPTH_COMPONENT);
         break;
      case GL_TEXTURE_LOD_BIAS:
         if (_mesa_is_gles(ctx))
            goto invalid_pname;

         /* GL spec 'Data Conversions' section specifies that floating-point
          * value in integer Get function is rounded to nearest integer
          *
          * Section 2.2.2 (Data Conversions For State Query Commands) of the
          * OpenGL 4.5 spec says:
          *
          *   Following these steps, if a value is so large in magnitude that
          *   it cannot be represented by the returned data type, then the
          *   nearest value representable using that type is returned.
          */
         *params = LCLAMPF(obj->Sampler.Attrib.LodBias, INT32_MIN, INT32_MAX);
         break;
      case GL_TEXTURE_CROP_RECT_OES:
         if (ctx->API != API_OPENGLES || !ctx->Extensions.OES_draw_texture)
            goto invalid_pname;

         params[0] = obj->CropRect[0];
         params[1] = obj->CropRect[1];
         params[2] = obj->CropRect[2];
         params[3] = obj->CropRect[3];
         break;
      case GL_TEXTURE_SWIZZLE_R_EXT:
      case GL_TEXTURE_SWIZZLE_G_EXT:
      case GL_TEXTURE_SWIZZLE_B_EXT:
      case GL_TEXTURE_SWIZZLE_A_EXT:
         if (!_mesa_has_EXT_texture_swizzle(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         *params = obj->Attrib.Swizzle[pname - GL_TEXTURE_SWIZZLE_R_EXT];
         break;

      case GL_TEXTURE_SWIZZLE_RGBA_EXT:
         if (!_mesa_has_EXT_texture_swizzle(ctx) && !_mesa_is_gles3(ctx))
            goto invalid_pname;
         COPY_4V(params, obj->Attrib.Swizzle);
         break;

      case GL_TEXTURE_CUBE_MAP_SEAMLESS:
         if (_mesa_has_AMD_seamless_cubemap_per_texture(ctx))
            goto invalid_pname;
         *params = (GLint) obj->Sampler.Attrib.CubeMapSeamless;
         break;

      case GL_TEXTURE_IMMUTABLE_FORMAT:
         *params = (GLint) obj->Immutable;
         break;

      case GL_TEXTURE_IMMUTABLE_LEVELS:
         if (_mesa_has_ARB_texture_view(ctx) || _mesa_is_gles3(ctx))
            *params = obj->Attrib.ImmutableLevels;
         else
            goto invalid_pname;
         break;

      case GL_TEXTURE_VIEW_MIN_LEVEL:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->Attrib.MinLevel;
         break;

      case GL_TEXTURE_VIEW_NUM_LEVELS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->Attrib.NumLevels;
         break;

      case GL_TEXTURE_VIEW_MIN_LAYER:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->Attrib.MinLayer;
         break;

      case GL_TEXTURE_VIEW_NUM_LAYERS:
         if (!ctx->Extensions.ARB_texture_view)
            goto invalid_pname;
         *params = (GLint) obj->Attrib.NumLayers;
         break;

      case GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES:
         if (!_mesa_is_gles(ctx) || !ctx->Extensions.OES_EGL_image_external)
            goto invalid_pname;
         *params = obj->RequiredTextureImageUnits;
         break;

      case GL_TEXTURE_SRGB_DECODE_EXT:
         if (!ctx->Extensions.EXT_texture_sRGB_decode)
            goto invalid_pname;
         *params = obj->Sampler.Attrib.sRGBDecode;
         break;

      case GL_TEXTURE_REDUCTION_MODE_EXT:
         if (!ctx->Extensions.EXT_texture_filter_minmax &&
             !_mesa_has_ARB_texture_filter_minmax(ctx))
            goto invalid_pname;
         *params = obj->Sampler.Attrib.ReductionMode;
         break;

      case GL_IMAGE_FORMAT_COMPATIBILITY_TYPE:
         if (!ctx->Extensions.ARB_shader_image_load_store &&
             !_mesa_is_gles31(ctx))
            goto invalid_pname;
         *params = obj->Attrib.ImageFormatCompatibilityType;
         break;

      case GL_TEXTURE_TARGET:
         if (ctx->API != API_OPENGL_CORE)
            goto invalid_pname;
         *params = (GLint) obj->Target;
         break;

      case GL_TEXTURE_TILING_EXT:
         if (!ctx->Extensions.EXT_memory_object)
            goto invalid_pname;
         *params = (GLint) obj->TextureTiling;
         break;

      case GL_TEXTURE_SPARSE_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = obj->IsSparse;
         break;

      case GL_VIRTUAL_PAGE_SIZE_INDEX_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = obj->VirtualPageSizeIndex;
         break;

      case GL_NUM_SPARSE_LEVELS_ARB:
         if (!_mesa_has_ARB_sparse_texture(ctx))
            goto invalid_pname;
         *params = obj->NumSparseLevels;
         break;

      default:
         goto invalid_pname;
   }

   /* no error if we get here */
   _mesa_unlock_texture(ctx, obj);
   return;

invalid_pname:
   _mesa_unlock_texture(ctx, obj);
   _mesa_error(ctx, GL_INVALID_ENUM, "glGetTex%sParameteriv(pname=0x%x)",
               dsa ? "ture" : "", pname);
}

static void
get_tex_parameterIiv(struct gl_context *ctx,
                     struct gl_texture_object *obj,
                     GLenum pname, GLint *params, bool dsa)
{
   switch (pname) {
   case GL_TEXTURE_BORDER_COLOR:
      COPY_4V(params, obj->Sampler.Attrib.state.border_color.i);
      break;
   default:
      get_tex_parameteriv(ctx, obj, pname, params, dsa);
   }
}

void GLAPIENTRY
_mesa_GetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                ctx->Texture.CurrentUnit,
                                                false,
                                                "glGetTexParameterfv");
   if (!obj)
      return;

   get_tex_parameterfv(ctx, obj, pname, params, false);
}

void GLAPIENTRY
_mesa_GetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                ctx->Texture.CurrentUnit,
                                                false,
                                                "glGetTexParameteriv");
   if (!obj)
      return;

   get_tex_parameteriv(ctx, obj, pname, params, false);
}

/** New in GL 3.0 */
void GLAPIENTRY
_mesa_GetTexParameterIiv(GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                ctx->Texture.CurrentUnit,
                                                false,
                                                "glGetTexParameterIiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, params, false);
}


/** New in GL 3.0 */
void GLAPIENTRY
_mesa_GetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                ctx->Texture.CurrentUnit,
                                                false,
                                                "glGetTexParameterIuiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, (GLint *) params, false);
}

void GLAPIENTRY
_mesa_GetTextureParameterfvEXT(GLuint texture, GLenum target, GLenum pname, GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureParameterfvEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetTextureParameterfvEXT");
      return;
   }

   get_tex_parameterfv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetMultiTexParameterfvEXT(GLenum texunit, GLenum target, GLenum pname, GLfloat *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glGetMultiTexParameterfvEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetMultiTexParameterfvEXT");
      return;
   }
   get_tex_parameterfv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterfv(GLuint texture, GLenum pname, GLfloat *params)
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = get_texobj_by_name(ctx, texture, "glGetTextureParameterfv");
   if (!obj)
      return;

   get_tex_parameterfv(ctx, obj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterivEXT(GLuint texture, GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureParameterivEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetTextureParameterivEXT");
      return;
   }
   get_tex_parameteriv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetMultiTexParameterivEXT(GLenum texunit, GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glGetMultiTexParameterivEXT");
   if (!texObj)
      return;

   if (!is_texparameteri_target_valid(texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "glGetMultiTexParameterivEXT");
      return;
   }
   get_tex_parameteriv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameteriv(GLuint texture, GLenum pname, GLint *params)
{
   struct gl_texture_object *obj;
   GET_CURRENT_CONTEXT(ctx);

   obj = get_texobj_by_name(ctx, texture, "glGetTextureParameteriv");
   if (!obj)
      return;

   get_tex_parameteriv(ctx, obj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterIiv(GLuint texture, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glGetTextureParameterIiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterIivEXT(GLuint texture, GLenum target, GLenum pname, GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureParameterIivEXT");
   if (!texObj)
      return;


   get_tex_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetMultiTexParameterIivEXT(GLenum texunit, GLenum target, GLenum pname,
                                 GLint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glGetMultiTexParameterIiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterIuiv(GLuint texture, GLenum pname, GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = get_texobj_by_name(ctx, texture, "glGetTextureParameterIuiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, (GLint *) params, true);
}

void GLAPIENTRY
_mesa_GetTextureParameterIuivEXT(GLuint texture, GLenum target, GLenum pname,
                                 GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glGetTextureParameterIuvEXT");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, (GLint *) params, true);
}

void GLAPIENTRY
_mesa_GetMultiTexParameterIuivEXT(GLenum texunit, GLenum target, GLenum pname,
                               GLuint *params)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glGetMultiTexParameterIuiv");
   if (!texObj)
      return;

   get_tex_parameterIiv(ctx, texObj, pname, (GLint *) params, true);
}
