/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 1999-2013  VMware, Inc.  All Rights Reserved.
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


/*
 * glGenerateMipmap function
 */

#include "context.h"
#include "enums.h"
#include "genmipmap.h"
#include "glformats.h"
#include "macros.h"
#include "mtypes.h"
#include "teximage.h"
#include "texobj.h"
#include "hash.h"
#include "api_exec_decl.h"

#include "state_tracker/st_gen_mipmap.h"

bool
_mesa_is_valid_generate_texture_mipmap_target(struct gl_context *ctx,
                                              GLenum target)
{
   bool error;

   switch (target) {
   case GL_TEXTURE_1D:
      error = _mesa_is_gles(ctx);
      break;
   case GL_TEXTURE_2D:
      error = false;
      break;
   case GL_TEXTURE_3D:
      error = _mesa_is_gles1(ctx);
      break;
   case GL_TEXTURE_CUBE_MAP:
      error = false;
      break;
   case GL_TEXTURE_1D_ARRAY:
      error = _mesa_is_gles(ctx) || !ctx->Extensions.EXT_texture_array;
      break;
   case GL_TEXTURE_2D_ARRAY:
      error = (_mesa_is_gles(ctx) && ctx->Version < 30)
         || !ctx->Extensions.EXT_texture_array;
      break;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      error = !_mesa_has_texture_cube_map_array(ctx);
      break;
   default:
      error = true;
   }

   return !error;
}

bool
_mesa_is_valid_generate_texture_mipmap_internalformat(struct gl_context *ctx,
                                                      GLenum internalformat)
{
   if (_mesa_is_gles3(ctx)) {
      /* From the ES 3.2 specification's description of GenerateMipmap():
       * "An INVALID_OPERATION error is generated if the levelbase array was
       *  not specified with an unsized internal format from table 8.3 or a
       *  sized internal format that is both color-renderable and
       *  texture-filterable according to table 8.10."
       *
       * GL_EXT_texture_format_BGRA8888 adds a GL_BGRA_EXT unsized internal
       * format, and includes it in a very similar looking table.  So we
       * include it here as well.
       */
      return internalformat == GL_RGBA || internalformat == GL_RGB ||
             internalformat == GL_LUMINANCE_ALPHA ||
             internalformat == GL_LUMINANCE || internalformat == GL_ALPHA ||
             internalformat == GL_BGRA_EXT ||
             (_mesa_is_es3_color_renderable(ctx, internalformat) &&
              _mesa_is_es3_texture_filterable(ctx, internalformat));
   }

   return (!_mesa_is_enum_format_integer(internalformat) &&
           !_mesa_is_depthstencil_format(internalformat) &&
           !_mesa_is_astc_format(internalformat) &&
           !_mesa_is_stencil_format(internalformat));
}

/**
 * Implements glGenerateMipmap and glGenerateTextureMipmap.
 * Generates all the mipmap levels below the base level.
 * Error-checking is done only if caller is not NULL.
 */
static ALWAYS_INLINE void
generate_texture_mipmap(struct gl_context *ctx,
                        struct gl_texture_object *texObj, GLenum target,
                        const char* caller)
{
   struct gl_texture_image *srcImage;

   FLUSH_VERTICES(ctx, 0, 0);

   if (texObj->Attrib.BaseLevel >= texObj->Attrib.MaxLevel) {
      /* nothing to do */
      return;
   }

   if (caller && texObj->Target == GL_TEXTURE_CUBE_MAP &&
       !_mesa_cube_complete(texObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(incomplete cube map)", caller);
      return;
   }

   _mesa_lock_texture(ctx, texObj);

   texObj->External = GL_FALSE;

   srcImage = _mesa_select_tex_image(texObj, target, texObj->Attrib.BaseLevel);
   if (caller) {
      if (!srcImage) {
         _mesa_unlock_texture(ctx, texObj);
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(zero size base image)", caller);
         return;
      }

      if (!_mesa_is_valid_generate_texture_mipmap_internalformat(ctx,
                                                                 srcImage->InternalFormat)) {
         _mesa_unlock_texture(ctx, texObj);
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(invalid internal format %s)", caller,
                     _mesa_enum_to_string(srcImage->InternalFormat));
         return;
      }

      /* The GLES 2.0 spec says:
       *
       *    "If the level zero array is stored in a compressed internal format,
       *     the error INVALID_OPERATION is generated."
       *
       * and this text is gone from the GLES 3.0 spec.
       */
      if (_mesa_is_gles2(ctx) && ctx->Version < 30 &&
          _mesa_is_format_compressed(srcImage->TexFormat)) {
         _mesa_unlock_texture(ctx, texObj);
         _mesa_error(ctx, GL_INVALID_OPERATION, "generate mipmaps on compressed texture");
         return;
      }
   }

   if (srcImage->Width == 0 || srcImage->Height == 0) {
      _mesa_unlock_texture(ctx, texObj);
      return;
   }

   if (target == GL_TEXTURE_CUBE_MAP) {
      GLuint face;
      for (face = 0; face < 6; face++) {
         st_generate_mipmap(ctx,
                            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, texObj);
      }
   }
   else {
      st_generate_mipmap(ctx, target, texObj);
   }
   _mesa_unlock_texture(ctx, texObj);
}

/**
 * Generate all the mipmap levels below the base level.
 * Note: this GL function would be more useful if one could specify a
 * cube face, a set of array slices, etc.
 */
void GLAPIENTRY
_mesa_GenerateMipmap_no_error(GLenum target)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object *texObj = _mesa_get_current_tex_object(ctx, target);
   generate_texture_mipmap(ctx, texObj, target, NULL);
}

void GLAPIENTRY
_mesa_GenerateMipmap(GLenum target)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_is_valid_generate_texture_mipmap_target(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glGenerateMipmap(target=%s)",
                  _mesa_enum_to_string(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   generate_texture_mipmap(ctx, texObj, target, "glGenerateMipmap");
}

/**
 * Generate all the mipmap levels below the base level.
 */
void GLAPIENTRY
_mesa_GenerateTextureMipmap_no_error(GLuint texture)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object *texObj = _mesa_lookup_texture(ctx, texture);
   generate_texture_mipmap(ctx, texObj, texObj->Target, NULL);
}

static void
validate_params_and_generate_mipmap(struct gl_texture_object *texObj, const char* caller)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!texObj)
      return;

   if (!_mesa_is_valid_generate_texture_mipmap_target(ctx, texObj->Target)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target=%s)",
                  caller,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   generate_texture_mipmap(ctx, texObj, texObj->Target, caller);
}

void GLAPIENTRY
_mesa_GenerateTextureMipmap(GLuint texture)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture, "glGenerateTextureMipmap");
   validate_params_and_generate_mipmap(texObj, "glGenerateTextureMipmap");
}

void GLAPIENTRY
_mesa_GenerateTextureMipmapEXT(GLuint texture, GLenum target)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture,
                                           false, true,
                                           "glGenerateTextureMipmapEXT");
   validate_params_and_generate_mipmap(texObj,
                                       "glGenerateTextureMipmapEXT");
}

void GLAPIENTRY
_mesa_GenerateMultiTexMipmapEXT(GLenum texunit, GLenum target)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glGenerateMultiTexMipmapEXT");
   validate_params_and_generate_mipmap(texObj,
                                       "glGenerateMultiTexMipmapEXT");
}
