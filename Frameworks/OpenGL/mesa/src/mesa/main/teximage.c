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
 * \file teximage.c
 * Texture image-related functions.
 */

#include <stdbool.h>
#include "util/glheader.h"
#include "bufferobj.h"
#include "context.h"
#include "enums.h"
#include "fbobject.h"
#include "framebuffer.h"
#include "hash.h"
#include "image.h"

#include "macros.h"
#include "mipmap.h"
#include "multisample.h"
#include "pixel.h"
#include "pixelstore.h"
#include "state.h"
#include "texcompress.h"
#include "texcompress_cpal.h"
#include "teximage.h"
#include "texobj.h"
#include "texstate.h"
#include "texstorage.h"
#include "textureview.h"
#include "mtypes.h"
#include "glformats.h"
#include "texstore.h"
#include "pbo.h"
#include "api_exec_decl.h"

#include "util/u_memory.h"

#include "program/prog_instruction.h"

#include "state_tracker/st_cb_texture.h"
#include "state_tracker/st_context.h"
#include "state_tracker/st_format.h"
#include "state_tracker/st_gen_mipmap.h"
#include "state_tracker/st_cb_eglimage.h"
#include "state_tracker/st_sampler_view.h"

/**
 * Returns a corresponding internal floating point format for a given base
 * format as specifed by OES_texture_float. In case of GL_FLOAT, the internal
 * format needs to be a 32 bit component and in case of GL_HALF_FLOAT_OES it
 * needs to be a 16 bit component.
 *
 * For example, given base format GL_RGBA, type GL_FLOAT return GL_RGBA32F_ARB.
 */
static GLenum
adjust_for_oes_float_texture(const struct gl_context *ctx,
                             GLenum format, GLenum type)
{
   switch (type) {
   case GL_FLOAT:
      if (ctx->Extensions.OES_texture_float) {
         switch (format) {
         case GL_RGBA:
            return GL_RGBA32F;
         case GL_RGB:
            return GL_RGB32F;
         case GL_ALPHA:
            return GL_ALPHA32F_ARB;
         case GL_LUMINANCE:
            return GL_LUMINANCE32F_ARB;
         case GL_LUMINANCE_ALPHA:
            return GL_LUMINANCE_ALPHA32F_ARB;
         default:
            break;
         }
      }
      break;

   case GL_HALF_FLOAT_OES:
      if (ctx->Extensions.OES_texture_half_float) {
         switch (format) {
         case GL_RGBA:
            return GL_RGBA16F;
         case GL_RGB:
            return GL_RGB16F;
         case GL_ALPHA:
            return GL_ALPHA16F_ARB;
         case GL_LUMINANCE:
            return GL_LUMINANCE16F_ARB;
         case GL_LUMINANCE_ALPHA:
            return GL_LUMINANCE_ALPHA16F_ARB;
         default:
            break;
         }
      }
      break;

   default:
      break;
   }

   return format;
}

/**
 * Returns a corresponding base format for a given internal floating point
 * format as specifed by OES_texture_float.
 */
static GLenum
oes_float_internal_format(const struct gl_context *ctx,
                          GLenum format, GLenum type)
{
   switch (type) {
   case GL_FLOAT:
      if (ctx->Extensions.OES_texture_float) {
         switch (format) {
         case GL_RGBA32F:
            return GL_RGBA;
         case GL_RGB32F:
            return GL_RGB;
         case GL_ALPHA32F_ARB:
            return GL_ALPHA;
         case GL_LUMINANCE32F_ARB:
            return GL_LUMINANCE;
         case GL_LUMINANCE_ALPHA32F_ARB:
            return GL_LUMINANCE_ALPHA;
         default:
            break;
         }
      }
      break;

   case GL_HALF_FLOAT_OES:
      if (ctx->Extensions.OES_texture_half_float) {
         switch (format) {
         case GL_RGBA16F:
            return GL_RGBA;
         case GL_RGB16F:
            return GL_RGB;
         case GL_ALPHA16F_ARB:
            return GL_ALPHA;
         case GL_LUMINANCE16F_ARB:
            return GL_LUMINANCE;
         case GL_LUMINANCE_ALPHA16F_ARB:
            return GL_LUMINANCE_ALPHA;
         default:
            break;
         }
      }
      break;
   }
   return format;
}


/**
 * Install gl_texture_image in a gl_texture_object according to the target
 * and level parameters.
 *
 * \param tObj texture object.
 * \param target texture target.
 * \param level image level.
 * \param texImage texture image.
 */
static void
set_tex_image(struct gl_texture_object *tObj,
              GLenum target, GLint level,
              struct gl_texture_image *texImage)
{
   const GLuint face = _mesa_tex_target_to_face(target);

   assert(tObj);
   assert(texImage);
   if (target == GL_TEXTURE_RECTANGLE_NV || target == GL_TEXTURE_EXTERNAL_OES)
      assert(level == 0);

   tObj->Image[face][level] = texImage;

   /* Set the 'back' pointer */
   texImage->TexObject = tObj;
   texImage->Level = level;
   texImage->Face = face;
}


/**
 * Free a gl_texture_image and associated data.
 * This function is a fallback.
 *
 * \param texImage texture image.
 *
 * Free the texture image structure and the associated image data.
 */
void
_mesa_delete_texture_image(struct gl_context *ctx,
                           struct gl_texture_image *texImage)
{
   /* Free texImage->Data and/or any other driver-specific texture
    * image storage.
    */
   st_FreeTextureImageBuffer( ctx, texImage );
   FREE(texImage);
}


/**
 * Test if a target is a proxy target.
 *
 * \param target texture target.
 *
 * \return GL_TRUE if the target is a proxy target, GL_FALSE otherwise.
 */
GLboolean
_mesa_is_proxy_texture(GLenum target)
{
   unsigned i;
   static const GLenum targets[] = {
      GL_PROXY_TEXTURE_1D,
      GL_PROXY_TEXTURE_2D,
      GL_PROXY_TEXTURE_3D,
      GL_PROXY_TEXTURE_CUBE_MAP,
      GL_PROXY_TEXTURE_RECTANGLE,
      GL_PROXY_TEXTURE_1D_ARRAY,
      GL_PROXY_TEXTURE_2D_ARRAY,
      GL_PROXY_TEXTURE_CUBE_MAP_ARRAY,
      GL_PROXY_TEXTURE_2D_MULTISAMPLE,
      GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY
   };
   /*
    * NUM_TEXTURE_TARGETS should match number of terms above, except there's no
    * proxy for GL_TEXTURE_BUFFER and GL_TEXTURE_EXTERNAL_OES.
    */
   STATIC_ASSERT(NUM_TEXTURE_TARGETS == ARRAY_SIZE(targets) + 2);

   for (i = 0; i < ARRAY_SIZE(targets); ++i)
      if (target == targets[i])
         return GL_TRUE;
   return GL_FALSE;
}


/**
 * Test if a target is an array target.
 *
 * \param target texture target.
 *
 * \return true if the target is an array target, false otherwise.
 */
bool
_mesa_is_array_texture(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return true;
   default:
      return false;
   };
}

/**
 * Test if a target is a cube map.
 *
 * \param target texture target.
 *
 * \return true if the target is a cube map, false otherwise.
 */
bool
_mesa_is_cube_map_texture(GLenum target)
{
   switch(target) {
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      return true;
   default:
      return false;
   }
}

/**
 * Return the proxy target which corresponds to the given texture target
 */
static GLenum
proxy_target(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      return GL_PROXY_TEXTURE_1D;
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      return GL_PROXY_TEXTURE_2D;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      return GL_PROXY_TEXTURE_3D;
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_CUBE_MAP:
      return GL_PROXY_TEXTURE_CUBE_MAP;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return GL_PROXY_TEXTURE_RECTANGLE_NV;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      return GL_PROXY_TEXTURE_1D_ARRAY_EXT;
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return GL_PROXY_TEXTURE_2D_ARRAY_EXT;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      return GL_PROXY_TEXTURE_CUBE_MAP_ARRAY;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return GL_PROXY_TEXTURE_2D_MULTISAMPLE;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY;
   default:
      _mesa_problem(NULL, "unexpected target in proxy_target()");
      return 0;
   }
}




/**
 * Get a texture image pointer from a texture object, given a texture
 * target and mipmap level.  The target and level parameters should
 * have already been error-checked.
 *
 * \param texObj texture unit.
 * \param target texture target.
 * \param level image level.
 *
 * \return pointer to the texture image structure, or NULL on failure.
 */
struct gl_texture_image *
_mesa_select_tex_image(const struct gl_texture_object *texObj,
		                 GLenum target, GLint level)
{
   const GLuint face = _mesa_tex_target_to_face(target);

   assert(texObj);
   assert(level >= 0);
   assert(level < MAX_TEXTURE_LEVELS);

   return texObj->Image[face][level];
}


/**
 * Like _mesa_select_tex_image() but if the image doesn't exist, allocate
 * it and install it.  Only return NULL if passed a bad parameter or run
 * out of memory.
 */
struct gl_texture_image *
_mesa_get_tex_image(struct gl_context *ctx, struct gl_texture_object *texObj,
                    GLenum target, GLint level)
{
   struct gl_texture_image *texImage;

   if (!texObj)
      return NULL;

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      texImage = CALLOC_STRUCT(gl_texture_image);
      if (!texImage) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "texture image allocation");
         return NULL;
      }

      set_tex_image(texObj, target, level, texImage);
   }

   return texImage;
}


/**
 * Return pointer to the specified proxy texture image.
 * Note that proxy textures are per-context, not per-texture unit.
 * \return pointer to texture image or NULL if invalid target, invalid
 *         level, or out of memory.
 */
static struct gl_texture_image *
get_proxy_tex_image(struct gl_context *ctx, GLenum target, GLint level)
{
   struct gl_texture_image *texImage;
   GLuint texIndex;

   if (level < 0)
      return NULL;

   switch (target) {
   case GL_PROXY_TEXTURE_1D:
      texIndex = TEXTURE_1D_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D:
      texIndex = TEXTURE_2D_INDEX;
      break;
   case GL_PROXY_TEXTURE_3D:
      texIndex = TEXTURE_3D_INDEX;
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP:
      texIndex = TEXTURE_CUBE_INDEX;
      break;
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      if (level > 0)
         return NULL;
      texIndex = TEXTURE_RECT_INDEX;
      break;
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      texIndex = TEXTURE_1D_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      texIndex = TEXTURE_2D_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      texIndex = TEXTURE_CUBE_ARRAY_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      texIndex = TEXTURE_2D_MULTISAMPLE_INDEX;
      break;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      texIndex = TEXTURE_2D_MULTISAMPLE_ARRAY_INDEX;
      break;
   default:
      return NULL;
   }

   texImage = ctx->Texture.ProxyTex[texIndex]->Image[0][level];
   if (!texImage) {
      texImage = CALLOC_STRUCT(gl_texture_image);
      if (!texImage) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "proxy texture allocation");
         return NULL;
      }
      ctx->Texture.ProxyTex[texIndex]->Image[0][level] = texImage;
      /* Set the 'back' pointer */
      texImage->TexObject = ctx->Texture.ProxyTex[texIndex];
   }
   return texImage;
}


/**
 * Get the maximum number of allowed mipmap levels.
 *
 * \param ctx GL context.
 * \param target texture target.
 *
 * \return the maximum number of allowed mipmap levels for the given
 * texture target, or zero if passed a bad target.
 *
 * \sa gl_constants.
 */
GLint
_mesa_max_texture_levels(const struct gl_context *ctx, GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      return ffs(util_next_power_of_two(ctx->Const.MaxTextureSize));
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      return !(_mesa_is_gles2(ctx) && !ctx->Extensions.OES_texture_3D)
         ? ctx->Const.Max3DTextureLevels : 0;
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_PROXY_TEXTURE_CUBE_MAP:
      return ctx->Const.MaxCubeTextureLevels;
   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      return ctx->Extensions.NV_texture_rectangle ? 1 : 0;
   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
      return ctx->Extensions.EXT_texture_array
         ? ffs(util_next_power_of_two(ctx->Const.MaxTextureSize)) : 0;
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      return _mesa_has_texture_cube_map_array(ctx)
         ? ctx->Const.MaxCubeTextureLevels : 0;
   case GL_TEXTURE_BUFFER:
      return (_mesa_has_ARB_texture_buffer_object(ctx) ||
              _mesa_has_OES_texture_buffer(ctx)) ? 1 : 0;
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return (_mesa_is_desktop_gl(ctx) || _mesa_is_gles31(ctx))
         && ctx->Extensions.ARB_texture_multisample
         ? 1 : 0;
   case GL_TEXTURE_EXTERNAL_OES:
      return _mesa_has_OES_EGL_image_external(ctx) ? 1 : 0;
   default:
      return 0; /* bad target */
   }
}


/**
 * Return number of dimensions per mipmap level for the given texture target.
 */
GLint
_mesa_get_texture_dimensions(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      return 1;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return 2;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return 3;
   case GL_TEXTURE_BUFFER:
      FALLTHROUGH;
   default:
      _mesa_problem(NULL, "invalid target 0x%x in get_texture_dimensions()",
                    target);
      return 2;
   }
}


/**
 * Check if a texture target can have more than one layer.
 */
GLboolean
_mesa_tex_target_is_layered(GLenum target)
{
   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_EXTERNAL_OES:
      return GL_FALSE;

   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return GL_TRUE;

   default:
      assert(!"Invalid texture target.");
      return GL_FALSE;
   }
}


/**
 * Return the number of layers present in the given level of an array,
 * cubemap or 3D texture.  If the texture is not layered return zero.
 */
GLuint
_mesa_get_texture_layers(const struct gl_texture_object *texObj, GLint level)
{
   assert(level >= 0 && level < MAX_TEXTURE_LEVELS);

   switch (texObj->Target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_BUFFER:
   case GL_TEXTURE_EXTERNAL_OES:
      return 0;

   case GL_TEXTURE_CUBE_MAP:
      return 6;

   case GL_TEXTURE_1D_ARRAY: {
      struct gl_texture_image *img = texObj->Image[0][level];
      return img ? img->Height : 0;
   }

   case GL_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY: {
      struct gl_texture_image *img = texObj->Image[0][level];
      return img ? img->Depth : 0;
   }

   default:
      assert(!"Invalid texture target.");
      return 0;
   }
}


/**
 * Return the maximum number of mipmap levels for the given target
 * and the dimensions.
 * The dimensions are expected not to include the border.
 */
GLsizei
_mesa_get_tex_max_num_levels(GLenum target, GLsizei width, GLsizei height,
                             GLsizei depth)
{
   GLsizei size;

   switch (target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D_ARRAY:
      size = width;
      break;
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      size = width;
      break;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D_ARRAY:
      size = MAX2(width, height);
      break;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      size = MAX3(width, height, depth);
      break;
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return 1;
   default:
      assert(0);
      return 1;
   }

   return util_logbase2(size) + 1;
}


#if 000 /* not used anymore */
/*
 * glTexImage[123]D can accept a NULL image pointer.  In this case we
 * create a texture image with unspecified image contents per the OpenGL
 * spec.
 */
static GLubyte *
make_null_texture(GLint width, GLint height, GLint depth, GLenum format)
{
   const GLint components = _mesa_components_in_format(format);
   const GLint numPixels = width * height * depth;
   GLubyte *data = (GLubyte *) malloc(numPixels * components * sizeof(GLubyte));

#ifdef DEBUG
   /*
    * Let's see if anyone finds this.  If glTexImage2D() is called with
    * a NULL image pointer then load the texture image with something
    * interesting instead of leaving it indeterminate.
    */
   if (data) {
      static const char message[8][32] = {
         "   X   X  XXXXX   XXX     X    ",
         "   XX XX  X      X   X   X X   ",
         "   X X X  X      X      X   X  ",
         "   X   X  XXXX    XXX   XXXXX  ",
         "   X   X  X          X  X   X  ",
         "   X   X  X      X   X  X   X  ",
         "   X   X  XXXXX   XXX   X   X  ",
         "                               "
      };

      GLubyte *imgPtr = data;
      GLint h, i, j, k;
      for (h = 0; h < depth; h++) {
         for (i = 0; i < height; i++) {
            GLint srcRow = 7 - (i % 8);
            for (j = 0; j < width; j++) {
               GLint srcCol = j % 32;
               GLubyte texel = (message[srcRow][srcCol]=='X') ? 255 : 70;
               for (k = 0; k < components; k++) {
                  *imgPtr++ = texel;
               }
            }
         }
      }
   }
#endif

   return data;
}
#endif



/**
 * Set the size and format-related fields of a gl_texture_image struct
 * to zero.  This is used when a proxy texture test fails.
 */
static void
clear_teximage_fields(struct gl_texture_image *img)
{
   assert(img);
   img->_BaseFormat = 0;
   img->InternalFormat = 0;
   img->Border = 0;
   img->Width = 0;
   img->Height = 0;
   img->Depth = 0;
   img->Width2 = 0;
   img->Height2 = 0;
   img->Depth2 = 0;
   img->TexFormat = MESA_FORMAT_NONE;
   img->NumSamples = 0;
   img->FixedSampleLocations = GL_TRUE;
}

/**
 * Given a user-specified texture base format, the actual gallium texture
 * format and the current GL_DEPTH_MODE, return a texture swizzle.
 *
 * Consider the case where the user requests a GL_RGB internal texture
 * format the driver actually uses an RGBA format.  The A component should
 * be ignored and sampling from the texture should always return (r,g,b,1).
 * But if we rendered to the texture we might have written A values != 1.
 * By sampling the texture with a ".xyz1" swizzle we'll get the expected A=1.
 * This function computes the texture swizzle needed to get the expected
 * values.
 *
 * In the case of depth textures, the GL_DEPTH_MODE state determines the
 * texture swizzle.
 *
 * This result must be composed with the user-specified swizzle to get
 * the final swizzle.
 */
static unsigned
compute_texture_format_swizzle(GLenum baseFormat, GLenum depthMode,
                                     bool glsl130_or_later)
{
   switch (baseFormat) {
   case GL_RGBA:
      return SWIZZLE_XYZW;
   case GL_RGB:
      return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_Y, SWIZZLE_Z, SWIZZLE_ONE);
   case GL_RG:
      return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_Y, SWIZZLE_ZERO, SWIZZLE_ONE);
   case GL_RED:
      return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_ZERO,
                           SWIZZLE_ZERO, SWIZZLE_ONE);
   case GL_ALPHA:
      return MAKE_SWIZZLE4(SWIZZLE_ZERO, SWIZZLE_ZERO,
                           SWIZZLE_ZERO, SWIZZLE_W);
   case GL_LUMINANCE:
      return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_ONE);
   case GL_LUMINANCE_ALPHA:
      return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_W);
   case GL_INTENSITY:
      return SWIZZLE_XXXX;
   case GL_STENCIL_INDEX:
   case GL_DEPTH_STENCIL:
   case GL_DEPTH_COMPONENT:
      /* Now examine the depth mode */
      switch (depthMode) {
      case GL_LUMINANCE:
         return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_ONE);
      case GL_INTENSITY:
         return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_X, SWIZZLE_X, SWIZZLE_X);
      case GL_ALPHA:
         /* The texture(sampler*Shadow) functions from GLSL 1.30 ignore
          * the depth mode and return float, while older shadow* functions
          * and ARB_fp instructions return vec4 according to the depth mode.
          *
          * The problem with the GLSL 1.30 functions is that GL_ALPHA forces
          * them to return 0, breaking them completely.
          *
          * A proper fix would increase code complexity and that's not worth
          * it for a rarely used feature such as the GL_ALPHA depth mode
          * in GL3. Therefore, change GL_ALPHA to GL_INTENSITY for all
          * shaders that use GLSL 1.30 or later.
          *
          * BTW, it's required that sampler views are updated when
          * shaders change (check_sampler_swizzle takes care of that).
          */
         if (glsl130_or_later)
            return SWIZZLE_XXXX;
         else
            return MAKE_SWIZZLE4(SWIZZLE_ZERO, SWIZZLE_ZERO,
                                 SWIZZLE_ZERO, SWIZZLE_X);
      case GL_RED:
         return MAKE_SWIZZLE4(SWIZZLE_X, SWIZZLE_ZERO,
                              SWIZZLE_ZERO, SWIZZLE_ONE);
      default:
         assert(!"Unexpected depthMode");
         return SWIZZLE_XYZW;
      }
   default:
      assert(!"Unexpected baseFormat");
      return SWIZZLE_XYZW;
   }
}

void
_mesa_update_teximage_format_swizzle(struct gl_context *ctx,
                                     struct gl_texture_image *img,
                                     GLenum depth_mode)
{
   if (!img)
      return;
   img->FormatSwizzle = compute_texture_format_swizzle(img->_BaseFormat, depth_mode, false);
   img->FormatSwizzleGLSL130 = compute_texture_format_swizzle(img->_BaseFormat, depth_mode, true);
}

/**
 * Initialize basic fields of the gl_texture_image struct.
 *
 * \param ctx GL context.
 * \param img texture image structure to be initialized.
 * \param width image width.
 * \param height image height.
 * \param depth image depth.
 * \param border image border.
 * \param internalFormat internal format.
 * \param format  the actual hardware format (one of MESA_FORMAT_*)
 * \param numSamples  number of samples per texel, or zero for non-MS.
 * \param fixedSampleLocations  are sample locations fixed?
 *
 * Fills in the fields of \p img with the given information.
 * Note: width, height and depth include the border.
 */
void
_mesa_init_teximage_fields_ms(struct gl_context *ctx,
                        struct gl_texture_image *img,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLint border, GLenum internalFormat,
                        mesa_format format,
                        GLuint numSamples, GLboolean fixedSampleLocations)
{
   const GLint base_format =_mesa_base_tex_format(ctx, internalFormat);
   GLenum target;
   assert(img);
   assert(width >= 0);
   assert(height >= 0);
   assert(depth >= 0);

   target = img->TexObject->Target;
   assert(base_format != -1);
   img->_BaseFormat = (GLenum16)base_format;
   img->InternalFormat = internalFormat;
   img->Border = border;
   img->Width = width;
   img->Height = height;
   img->Depth = depth;

   GLenum depth_mode = _mesa_is_desktop_gl_core(ctx) ? GL_RED : GL_LUMINANCE;

   /* In ES 3.0, DEPTH_TEXTURE_MODE is expected to be GL_RED for textures
    * with depth component data specified with a sized internal format.
    */
   if (_mesa_is_gles3(ctx) &&
       (base_format == GL_DEPTH_COMPONENT ||
        base_format == GL_DEPTH_STENCIL ||
        base_format == GL_STENCIL_INDEX)) {
      if (internalFormat != GL_DEPTH_COMPONENT &&
          internalFormat != GL_DEPTH_STENCIL &&
          internalFormat != GL_STENCIL_INDEX)
         depth_mode = GL_RED;
   }
   _mesa_update_teximage_format_swizzle(ctx, img, depth_mode);

   img->Width2 = width - 2 * border;

   switch(target) {
   case GL_TEXTURE_1D:
   case GL_TEXTURE_BUFFER:
   case GL_PROXY_TEXTURE_1D:
      if (height == 0)
         img->Height2 = 0;
      else
         img->Height2 = 1;
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      break;
   case GL_TEXTURE_1D_ARRAY:
   case GL_PROXY_TEXTURE_1D_ARRAY:
      img->Height2 = height; /* no border */
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      break;
   case GL_TEXTURE_2D:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_PROXY_TEXTURE_2D:
   case GL_PROXY_TEXTURE_RECTANGLE:
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      img->Height2 = height - 2 * border;
      if (depth == 0)
         img->Depth2 = 0;
      else
         img->Depth2 = 1;
      break;
   case GL_TEXTURE_2D_ARRAY:
   case GL_PROXY_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      img->Height2 = height - 2 * border;
      img->Depth2 = depth; /* no border */
      break;
   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      img->Height2 = height - 2 * border;
      img->Depth2 = depth - 2 * border;
      break;
   default:
      _mesa_problem(NULL, "invalid target 0x%x in _mesa_init_teximage_fields()",
                    target);
   }

   img->MaxNumLevels =
      _mesa_get_tex_max_num_levels(target,
                                   img->Width2, img->Height2, img->Depth2);
   img->TexFormat = format;
   img->NumSamples = numSamples;
   img->FixedSampleLocations = fixedSampleLocations;
}


void
_mesa_init_teximage_fields(struct gl_context *ctx,
                           struct gl_texture_image *img,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLint border, GLenum internalFormat,
                           mesa_format format)
{
   _mesa_init_teximage_fields_ms(ctx, img, width, height, depth, border,
                                 internalFormat, format, 0, GL_TRUE);
}


/**
 * Free and clear fields of the gl_texture_image struct.
 *
 * \param ctx GL context.
 * \param texImage texture image structure to be cleared.
 *
 * After the call, \p texImage will have no data associated with it.  Its
 * fields are cleared so that its parent object will test incomplete.
 */
void
_mesa_clear_texture_image(struct gl_context *ctx,
                          struct gl_texture_image *texImage)
{
   st_FreeTextureImageBuffer(ctx, texImage);
   clear_teximage_fields(texImage);
}


/**
 * Check the width, height, depth and border of a texture image are legal.
 * Used by all the glTexImage, glCompressedTexImage and glCopyTexImage
 * functions.
 * The target and level parameters will have already been validated.
 * \return GL_TRUE if size is OK, GL_FALSE otherwise.
 */
GLboolean
_mesa_legal_texture_dimensions(struct gl_context *ctx, GLenum target,
                               GLint level, GLint width, GLint height,
                               GLint depth, GLint border)
{
   GLint maxSize;

   switch (target) {
   case GL_TEXTURE_1D:
   case GL_PROXY_TEXTURE_1D:
      maxSize = ctx->Const.MaxTextureSize >> level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      maxSize = ctx->Const.MaxTextureSize >> level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !util_is_power_of_two_nonzero(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_3D:
   case GL_PROXY_TEXTURE_3D:
      maxSize = 1 << (ctx->Const.Max3DTextureLevels - 1);
      maxSize >>= level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 2 * border || depth > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !util_is_power_of_two_nonzero(height - 2 * border))
            return GL_FALSE;
         if (depth > 0 && !util_is_power_of_two_nonzero(depth - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_RECTANGLE_NV:
   case GL_PROXY_TEXTURE_RECTANGLE_NV:
      if (level != 0)
         return GL_FALSE;
      maxSize = ctx->Const.MaxTextureRectSize;
      if (width < 0 || width > maxSize)
         return GL_FALSE;
      if (height < 0 || height > maxSize)
         return GL_FALSE;
      return GL_TRUE;

   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
   case GL_PROXY_TEXTURE_CUBE_MAP:
      maxSize = 1 << (ctx->Const.MaxCubeTextureLevels - 1);
      maxSize >>= level;
      if (width != height)
         return GL_FALSE;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !util_is_power_of_two_nonzero(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_1D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
      maxSize = ctx->Const.MaxTextureSize >> level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 0 || height > ctx->Const.MaxArrayTextureLayers)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_2D_ARRAY_EXT:
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      maxSize = ctx->Const.MaxTextureSize >> level;
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 0 || depth > ctx->Const.MaxArrayTextureLayers)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !util_is_power_of_two_nonzero(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;

   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
      maxSize = 1 << (ctx->Const.MaxCubeTextureLevels - 1);
      if (width < 2 * border || width > 2 * border + maxSize)
         return GL_FALSE;
      if (height < 2 * border || height > 2 * border + maxSize)
         return GL_FALSE;
      if (depth < 0 || depth > ctx->Const.MaxArrayTextureLayers || depth % 6)
         return GL_FALSE;
      if (width != height)
         return GL_FALSE;
      if (level >= ctx->Const.MaxCubeTextureLevels)
         return GL_FALSE;
      if (!ctx->Extensions.ARB_texture_non_power_of_two) {
         if (width > 0 && !util_is_power_of_two_nonzero(width - 2 * border))
            return GL_FALSE;
         if (height > 0 && !util_is_power_of_two_nonzero(height - 2 * border))
            return GL_FALSE;
      }
      return GL_TRUE;
   default:
      _mesa_problem(ctx, "Invalid target in _mesa_legal_texture_dimensions()");
      return GL_FALSE;
   }
}

static bool
error_check_subtexture_negative_dimensions(struct gl_context *ctx,
                                           GLuint dims,
                                           GLsizei subWidth,
                                           GLsizei subHeight,
                                           GLsizei subDepth,
                                           const char *func)
{
   /* Check size */
   if (subWidth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(width=%d)", func, subWidth);
      return true;
   }

   if (dims > 1 && subHeight < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(height=%d)", func, subHeight);
      return true;
   }

   if (dims > 2 && subDepth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(depth=%d)", func, subDepth);
      return true;
   }

   return false;
}

/**
 * Do error checking of xoffset, yoffset, zoffset, width, height and depth
 * for glTexSubImage, glCopyTexSubImage and glCompressedTexSubImage.
 * \param destImage  the destination texture image.
 * \return GL_TRUE if error found, GL_FALSE otherwise.
 */
static GLboolean
error_check_subtexture_dimensions(struct gl_context *ctx, GLuint dims,
                                  const struct gl_texture_image *destImage,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei subWidth, GLsizei subHeight,
                                  GLsizei subDepth, const char *func)
{
   const GLenum target = destImage->TexObject->Target;
   GLuint bw, bh, bd;

   /* check xoffset and width */
   if (xoffset < - (GLint) destImage->Border) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(xoffset)", func);
      return GL_TRUE;
   }

   if (xoffset + subWidth > (GLint) destImage->Width) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(xoffset %d + width %d > %u)", func,
                  xoffset, subWidth, destImage->Width);
      return GL_TRUE;
   }

   /* check yoffset and height */
   if (dims > 1) {
      GLint yBorder = (target == GL_TEXTURE_1D_ARRAY) ? 0 : destImage->Border;
      if (yoffset < -yBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(yoffset)", func);
         return GL_TRUE;
      }
      if (yoffset + subHeight > (GLint) destImage->Height) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(yoffset %d + height %d > %u)",
                     func, yoffset, subHeight, destImage->Height);
         return GL_TRUE;
      }
   }

   /* check zoffset and depth */
   if (dims > 2) {
      GLint depth;
      GLint zBorder = (target == GL_TEXTURE_2D_ARRAY ||
                       target == GL_TEXTURE_CUBE_MAP_ARRAY) ?
                         0 : destImage->Border;

      if (zoffset < -zBorder) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(zoffset)", func);
         return GL_TRUE;
      }

      depth = (GLint) destImage->Depth;
      if (target == GL_TEXTURE_CUBE_MAP)
         depth = 6;
      if (zoffset + subDepth  > depth) {
         _mesa_error(ctx, GL_INVALID_VALUE, "%s(zoffset %d + depth %d > %u)",
                     func, zoffset, subDepth, depth);
         return GL_TRUE;
      }
   }

   /*
    * The OpenGL spec (and GL_ARB_texture_compression) says only whole
    * compressed texture images can be updated.  But, that restriction may be
    * relaxed for particular compressed formats.  At this time, all the
    * compressed formats supported by Mesa allow sub-textures to be updated
    * along compressed block boundaries.
    */
   _mesa_get_format_block_size_3d(destImage->TexFormat, &bw, &bh, &bd);

   if (bw != 1 || bh != 1 || bd != 1) {
      /* offset must be multiple of block size */
      if ((xoffset % bw != 0) || (yoffset % bh != 0) || (zoffset % bd != 0)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(xoffset = %d, yoffset = %d, zoffset = %d)",
                     func, xoffset, yoffset, zoffset);
         return GL_TRUE;
      }

      /* The size must be a multiple of bw x bh, or we must be using a
       * offset+size that exactly hits the edge of the image.  This
       * is important for small mipmap levels (1x1, 2x1, etc) and for
       * NPOT textures.
       */
      if ((subWidth % bw != 0) &&
          (xoffset + subWidth != (GLint) destImage->Width)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(width = %d)", func, subWidth);
         return GL_TRUE;
      }

      if ((subHeight % bh != 0) &&
          (yoffset + subHeight != (GLint) destImage->Height)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(height = %d)", func, subHeight);
         return GL_TRUE;
      }

      if ((subDepth % bd != 0) &&
          (zoffset + subDepth != (GLint) destImage->Depth)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(depth = %d)", func, subDepth);
         return GL_TRUE;
      }
   }

   return GL_FALSE;
}




/**
 * This is the fallback for Driver.TestProxyTexImage() for doing device-
 * specific texture image size checks.
 *
 * A hardware driver might override this function if, for example, the
 * max 3D texture size is 512x512x64 (i.e. not a cube).
 *
 * Note that width, height, depth == 0 is not an error.  However, a
 * texture with zero width/height/depth will be considered "incomplete"
 * and texturing will effectively be disabled.
 *
 * \param target  any texture target/type
 * \param numLevels  number of mipmap levels in the texture or 0 if not known
 * \param level  as passed to glTexImage
 * \param format  the MESA_FORMAT_x for the tex image
 * \param numSamples  number of samples per texel
 * \param width  as passed to glTexImage
 * \param height  as passed to glTexImage
 * \param depth  as passed to glTexImage
 * \return GL_TRUE if the image is acceptable, GL_FALSE if not acceptable.
 */
GLboolean
_mesa_test_proxy_teximage(struct gl_context *ctx, GLenum target,
                          GLuint numLevels, ASSERTED GLint level,
                          mesa_format format, GLuint numSamples,
                          GLint width, GLint height, GLint depth)
{
   uint64_t bytes, mbytes;

   if (numLevels > 0) {
      /* Compute total memory for a whole mipmap.  This is the path
       * taken for glTexStorage(GL_PROXY_TEXTURE_x).
       */
      unsigned l;

      assert(level == 0);

      bytes = 0;

      for (l = 0; l < numLevels; l++) {
         GLint nextWidth, nextHeight, nextDepth;

         bytes += _mesa_format_image_size64(format, width, height, depth);

         if (_mesa_next_mipmap_level_size(target, 0, width, height, depth,
                                          &nextWidth, &nextHeight,
                                          &nextDepth)) {
            width = nextWidth;
            height = nextHeight;
            depth = nextDepth;
         } else {
            break;
         }
      }
   } else {
      /* We just compute the size of one mipmap level.  This is the path
       * taken for glTexImage(GL_PROXY_TEXTURE_x).
       */
      bytes = _mesa_format_image_size64(format, width, height, depth);
   }

   bytes *= _mesa_num_tex_faces(target);
   bytes *= MAX2(1, numSamples);

   mbytes = bytes / (1024 * 1024); /* convert to MB */

   /* We just check if the image size is less than MaxTextureMbytes.
    * Some drivers may do more specific checks.
    */
   return mbytes <= (uint64_t) ctx->Const.MaxTextureMbytes;
}


/**
 * Return true if the format is only valid for glCompressedTexImage.
 */
static bool
compressedteximage_only_format(GLenum format)
{
   switch (format) {
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
   case GL_ATC_RGB_AMD:
   case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
   case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
      return true;
   default:
      return false;
   }
}

/**
 * Return true if the format doesn't support online compression.
 */
bool
_mesa_format_no_online_compression(GLenum format)
{
   return _mesa_is_astc_format(format) ||
          _mesa_is_etc2_format(format) ||
          compressedteximage_only_format(format);
}

/* Writes to an GL error pointer if non-null and returns whether or not the
 * error is GL_NO_ERROR */
static bool
write_error(GLenum *err_ptr, GLenum error)
{
   if (err_ptr)
      *err_ptr = error;

   return error == GL_NO_ERROR;
}

/**
 * Helper function to determine whether a target and specific compression
 * format are supported. The error parameter returns GL_NO_ERROR if the
 * target can be compressed. Otherwise it returns either GL_INVALID_OPERATION
 * or GL_INVALID_ENUM, whichever is more appropriate.
 */
GLboolean
_mesa_target_can_be_compressed(const struct gl_context *ctx, GLenum target,
                               GLenum intFormat, GLenum *error)
{
   GLboolean target_can_be_compresed = GL_FALSE;
   mesa_format format = _mesa_glenum_to_compressed_format(intFormat);
   enum mesa_format_layout layout = _mesa_get_format_layout(format);

   switch (target) {
   case GL_TEXTURE_2D:
   case GL_PROXY_TEXTURE_2D:
      target_can_be_compresed = GL_TRUE; /* true for any compressed format so far */
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      target_can_be_compresed = GL_TRUE;
      break;
   case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
   case GL_TEXTURE_2D_ARRAY_EXT:
      target_can_be_compresed = ctx->Extensions.EXT_texture_array;
      break;
   case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      /* From the KHR_texture_compression_astc_hdr spec:
       *
       *     Add a second new column "3D Tex." which is empty for all non-ASTC
       *     formats. If only the LDR profile is supported by the
       *     implementation, this column is also empty for all ASTC formats. If
       *     both the LDR and HDR profiles are supported only, this column is
       *     checked for all ASTC formats.
       *
       *     Add a third new column "Cube Map Array Tex." which is empty for all
       *     non-ASTC formats, and checked for all ASTC formats.
       *
       * and,
       *
       *     'An INVALID_OPERATION error is generated by CompressedTexImage3D
       *      if <internalformat> is TEXTURE_CUBE_MAP_ARRAY and the
       *      "Cube Map Array" column of table 8.19 is *not* checked, or if
       *      <internalformat> is TEXTURE_3D and the "3D Tex." column of table
       *      8.19 is *not* checked'
       *
       * The instances of <internalformat> above should say <target>.
       *
       * ETC2/EAC formats are the only alternative in GLES and thus such errors
       * have already been handled by normal ETC2/EAC behavior.
       */

      /* From section 3.8.6, page 146 of OpenGL ES 3.0 spec:
       *
       *    "The ETC2/EAC texture compression algorithm supports only
       *     two-dimensional images. If internalformat is an ETC2/EAC format,
       *     glCompressedTexImage3D will generate an INVALID_OPERATION error if
       *     target is not TEXTURE_2D_ARRAY."
       *
       * This should also be applicable for glTexStorage3D(). Other available
       * targets for these functions are: TEXTURE_3D and TEXTURE_CUBE_MAP_ARRAY.
       *
       * Section 8.7, page 179 of OpenGL ES 3.2 adds:
       *
       *      An INVALID_OPERATION error is generated by CompressedTexImage3D
       *      if internalformat is one of the the formats in table 8.17 and target is
       *      not TEXTURE_2D_ARRAY, TEXTURE_CUBE_MAP_ARRAY or TEXTURE_3D.
       *
       *      An INVALID_OPERATION error is generated by CompressedTexImage3D
       *      if internalformat is TEXTURE_CUBE_MAP_ARRAY and the “Cube Map
       *      Array” column of table 8.17 is not checked, or if internalformat
       *      is TEXTURE_- 3D and the “3D Tex.” column of table 8.17 is not
       *      checked.
       *
       * The instances of <internalformat> above should say <target>.
       *
       * Such table 8.17 has checked "Cube Map Array" column for all the
       * cases. So in practice, TEXTURE_CUBE_MAP_ARRAY is now valid for OpenGL ES 3.2
       */
      if (layout == MESA_FORMAT_LAYOUT_ETC2 && _mesa_is_gles3(ctx) &&
          !_mesa_is_gles32(ctx))
            return write_error(error, GL_INVALID_OPERATION);
      target_can_be_compresed = _mesa_has_texture_cube_map_array(ctx);
      break;
   case GL_TEXTURE_3D:
      switch (layout) {
      case MESA_FORMAT_LAYOUT_ETC2:
         /* See ETC2/EAC comment in case GL_TEXTURE_CUBE_MAP_ARRAY. */
         if (_mesa_is_gles3(ctx))
            return write_error(error, GL_INVALID_OPERATION);
         break;
      case MESA_FORMAT_LAYOUT_BPTC:
         target_can_be_compresed = ctx->Extensions.ARB_texture_compression_bptc;
         break;
      case MESA_FORMAT_LAYOUT_S3TC:
         /* From the EXT_texture_compression_s3tc spec:
          *
          *      In extended OpenGL ES 3.0.2 these new tokens are also accepted by the
          *      <internalformat> parameter of TexImage3D, CompressedTexImage3D,
          *      TexStorage2D, TexStorage3D and the <format> parameter of
          *      CompressedTexSubImage3D.
          *
          * The spec does not clarify whether the same applies to newer desktop GL versions
          * (where 3D textures were introduced), check compatibility ext to be safe.
          */
         target_can_be_compresed =
            ctx->Extensions.EXT_texture_compression_s3tc &&
            (_mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility);
         break;
      case MESA_FORMAT_LAYOUT_ASTC:
         target_can_be_compresed =
            ctx->Extensions.KHR_texture_compression_astc_hdr ||
            ctx->Extensions.KHR_texture_compression_astc_sliced_3d;

         /* Throw an INVALID_OPERATION error if the target is TEXTURE_3D and
          * neither of the above extensions are supported. See comment in
          * switch case GL_TEXTURE_CUBE_MAP_ARRAY for more info.
          */
         if (!target_can_be_compresed)
            return write_error(error, GL_INVALID_OPERATION);
         break;
      default:
         break;
      }
      FALLTHROUGH;
   default:
      break;
   }
   return write_error(error,
                      target_can_be_compresed ? GL_NO_ERROR : GL_INVALID_ENUM);
}


/**
 * Check if the given texture target value is legal for a
 * glTexImage1/2/3D call.
 */
static GLboolean
legal_teximage_target(struct gl_context *ctx, GLuint dims, GLenum target)
{
   switch (dims) {
   case 1:
      switch (target) {
      case GL_TEXTURE_1D:
      case GL_PROXY_TEXTURE_1D:
         return _mesa_is_desktop_gl(ctx);
      default:
         return GL_FALSE;
      }
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
         return GL_TRUE;
      case GL_PROXY_TEXTURE_2D:
         return _mesa_is_desktop_gl(ctx);
      case GL_PROXY_TEXTURE_CUBE_MAP:
         return _mesa_is_desktop_gl(ctx);
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return GL_TRUE;
      case GL_TEXTURE_RECTANGLE_NV:
      case GL_PROXY_TEXTURE_RECTANGLE_NV:
         return _mesa_is_desktop_gl(ctx)
            && ctx->Extensions.NV_texture_rectangle;
      case GL_TEXTURE_1D_ARRAY_EXT:
      case GL_PROXY_TEXTURE_1D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      default:
         return GL_FALSE;
      }
   case 3:
      switch (target) {
      case GL_TEXTURE_3D:
         return GL_TRUE;
      case GL_PROXY_TEXTURE_3D:
         return _mesa_is_desktop_gl(ctx);
      case GL_TEXTURE_2D_ARRAY_EXT:
         return (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array)
            || _mesa_is_gles3(ctx);
      case GL_PROXY_TEXTURE_2D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
         return _mesa_has_texture_cube_map_array(ctx);
      default:
         return GL_FALSE;
      }
   default:
      _mesa_problem(ctx, "invalid dims=%u in legal_teximage_target()", dims);
      return GL_FALSE;
   }
}


/**
 * Check if the given texture target value is legal for a
 * glTexSubImage, glCopyTexSubImage or glCopyTexImage call.
 * The difference compared to legal_teximage_target() above is that
 * proxy targets are not supported.
 */
static GLboolean
legal_texsubimage_target(struct gl_context *ctx, GLuint dims, GLenum target,
                         bool dsa)
{
   switch (dims) {
   case 1:
      return _mesa_is_desktop_gl(ctx) && target == GL_TEXTURE_1D;
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
         return GL_TRUE;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         return GL_TRUE;
      case GL_TEXTURE_RECTANGLE_NV:
         return _mesa_is_desktop_gl(ctx)
            && ctx->Extensions.NV_texture_rectangle;
      case GL_TEXTURE_1D_ARRAY_EXT:
         return _mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array;
      default:
         return GL_FALSE;
      }
   case 3:
      switch (target) {
      case GL_TEXTURE_3D:
         return GL_TRUE;
      case GL_TEXTURE_2D_ARRAY_EXT:
         return (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array)
            || _mesa_is_gles3(ctx);
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_PROXY_TEXTURE_CUBE_MAP_ARRAY:
         return _mesa_has_texture_cube_map_array(ctx);

      /* Table 8.15 of the OpenGL 4.5 core profile spec
       * (20141030) says that TEXTURE_CUBE_MAP is valid for TextureSubImage3D
       * and CopyTextureSubImage3D.
       */
      case GL_TEXTURE_CUBE_MAP:
         return dsa;
      default:
         return GL_FALSE;
      }
   default:
      _mesa_problem(ctx, "invalid dims=%u in legal_texsubimage_target()",
                    dims);
      return GL_FALSE;
   }
}


/**
 * Helper function to determine if a texture object is mutable (in terms
 * of GL_ARB_texture_storage/GL_ARB_bindless_texture).
 */
static GLboolean
mutable_tex_object(struct gl_texture_object *texObj)
{
   if (!texObj)
      return GL_FALSE;

   if (texObj->HandleAllocated) {
      /* The ARB_bindless_texture spec says:
       *
       * "The error INVALID_OPERATION is generated by TexImage*, CopyTexImage*,
       *  CompressedTexImage*, TexBuffer*, TexParameter*, as well as other
       *  functions defined in terms of these, if the texture object to be
       *  modified is referenced by one or more texture or image handles."
       */
      return GL_FALSE;
   }

   return !texObj->Immutable;
}


/**
 * Return expected size of a compressed texture.
 */
static GLuint
compressed_tex_size(GLsizei width, GLsizei height, GLsizei depth,
                    GLenum glformat)
{
   mesa_format mesaFormat = _mesa_glenum_to_compressed_format(glformat);
   return _mesa_format_image_size(mesaFormat, width, height, depth);
}

/**
 * Verify that a texture format is valid with a particular target
 *
 * In particular, textures with base format of \c GL_DEPTH_COMPONENT or
 * \c GL_DEPTH_STENCIL are only valid with certain, context dependent texture
 * targets.
 *
 * \param ctx             GL context
 * \param target          Texture target
 * \param internalFormat  Internal format of the texture image
 *
 * \returns true if the combination is legal, false otherwise.
 */
bool
_mesa_legal_texture_base_format_for_target(struct gl_context *ctx,
                                           GLenum target, GLenum internalFormat)
{
   if (_mesa_base_tex_format(ctx, internalFormat) == GL_DEPTH_COMPONENT
       || _mesa_base_tex_format(ctx, internalFormat) == GL_DEPTH_STENCIL
       || _mesa_base_tex_format(ctx, internalFormat) == GL_STENCIL_INDEX) {
      /* Section 3.8.3 (Texture Image Specification) of the OpenGL 3.3 Core
       * Profile spec says:
       *
       *     "Textures with a base internal format of DEPTH_COMPONENT or
       *     DEPTH_STENCIL are supported by texture image specification
       *     commands only if target is TEXTURE_1D, TEXTURE_2D,
       *     TEXTURE_1D_ARRAY, TEXTURE_2D_ARRAY, TEXTURE_RECTANGLE,
       *     TEXTURE_CUBE_MAP, PROXY_TEXTURE_1D, PROXY_TEXTURE_2D,
       *     PROXY_TEXTURE_1D_ARRAY, PROXY_TEXTURE_2D_ARRAY,
       *     PROXY_TEXTURE_RECTANGLE, or PROXY_TEXTURE_CUBE_MAP. Using these
       *     formats in conjunction with any other target will result in an
       *     INVALID_OPERATION error."
       *
       * Cubemaps are only supported with desktop OpenGL version >= 3.0,
       * EXT_gpu_shader4, or, on OpenGL ES 2.0+, OES_depth_texture_cube_map.
       */
      if (target != GL_TEXTURE_1D &&
          target != GL_PROXY_TEXTURE_1D &&
          target != GL_TEXTURE_2D &&
          target != GL_PROXY_TEXTURE_2D &&
          target != GL_TEXTURE_1D_ARRAY &&
          target != GL_PROXY_TEXTURE_1D_ARRAY &&
          target != GL_TEXTURE_2D_ARRAY &&
          target != GL_PROXY_TEXTURE_2D_ARRAY &&
          target != GL_TEXTURE_RECTANGLE_ARB &&
          target != GL_PROXY_TEXTURE_RECTANGLE_ARB &&
         !((_mesa_is_cube_face(target) ||
            target == GL_TEXTURE_CUBE_MAP ||
            target == GL_PROXY_TEXTURE_CUBE_MAP) &&
           (ctx->Version >= 30 || ctx->Extensions.EXT_gpu_shader4
            || (_mesa_is_gles2(ctx) && ctx->Extensions.OES_depth_texture_cube_map))) &&
          !((target == GL_TEXTURE_CUBE_MAP_ARRAY ||
             target == GL_PROXY_TEXTURE_CUBE_MAP_ARRAY) &&
            _mesa_has_texture_cube_map_array(ctx))) {
         return false;
      }
   }

   return true;
}

static bool
texture_formats_agree(GLenum internalFormat,
                      GLenum format)
{
   GLboolean colorFormat;
   GLboolean is_format_depth_or_depthstencil;
   GLboolean is_internalFormat_depth_or_depthstencil;

   /* Even though there are no color-index textures, we still have to support
    * uploading color-index data and remapping it to RGB via the
    * GL_PIXEL_MAP_I_TO_[RGBA] tables.
    */
   const GLboolean indexFormat = (format == GL_COLOR_INDEX);

   is_internalFormat_depth_or_depthstencil =
      _mesa_is_depth_format(internalFormat) ||
      _mesa_is_depthstencil_format(internalFormat);

   is_format_depth_or_depthstencil =
      _mesa_is_depth_format(format) ||
      _mesa_is_depthstencil_format(format);

   colorFormat = _mesa_is_color_format(format);

   if (_mesa_is_color_format(internalFormat) && !colorFormat && !indexFormat)
      return false;

   if (is_internalFormat_depth_or_depthstencil !=
       is_format_depth_or_depthstencil)
      return false;

   if (_mesa_is_ycbcr_format(internalFormat) != _mesa_is_ycbcr_format(format))
      return false;

   return true;
}

/**
 * Test the combination of format, type and internal format arguments of
 * different texture operations on GLES.
 *
 * \param ctx GL context.
 * \param format pixel data format given by the user.
 * \param type pixel data type given by the user.
 * \param internalFormat internal format given by the user.
 * \param callerName name of the caller function to print in the error message
 *
 * \return true if a error is found, false otherwise
 *
 * Currently, it is used by texture_error_check() and texsubimage_error_check().
 */
static bool
texture_format_error_check_gles(struct gl_context *ctx, GLenum format,
                                GLenum type, GLenum internalFormat, const char *callerName)
{
   GLenum err = _mesa_gles_error_check_format_and_type(ctx, format, type,
                                                       internalFormat);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "%s(format = %s, type = %s, internalformat = %s)",
                  callerName, _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type),
                  _mesa_enum_to_string(internalFormat));
      return true;
   }

   return false;
}

/**
 * Test the glTexImage[123]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user (already validated).
 * \param level image level given by the user.
 * \param internalFormat internal format given by the user.
 * \param format pixel data format given by the user.
 * \param type pixel data type given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param depth image depth given by the user.
 * \param border image border given by the user.
 *
 * \return GL_TRUE if a error is found, GL_FALSE otherwise
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 * Note that we don't fully error-check the width, height, depth values
 * here.  That's done in _mesa_legal_texture_dimensions() which is used
 * by several other GL entrypoints.  Plus, texture dims have a special
 * interaction with proxy textures.
 */
static GLboolean
texture_error_check( struct gl_context *ctx,
                     GLuint dimensions, GLenum target,
                     struct gl_texture_object* texObj,
                     GLint level, GLint internalFormat,
                     GLenum format, GLenum type,
                     GLint width, GLint height,
                     GLint depth, GLint border,
                     const GLvoid *pixels )
{
   GLenum err;

   /* Note: for proxy textures, some error conditions immediately generate
    * a GL error in the usual way.  But others do not generate a GL error.
    * Instead, they cause the width, height, depth, format fields of the
    * texture image to be zeroed-out.  The GL spec seems to indicate that the
    * zero-out behaviour is only used in cases related to memory allocation.
    */

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(level=%d)", dimensions, level);
      return GL_TRUE;
   }

   /* Check border */
   if (border < 0 || border > 1 ||
       ((ctx->API != API_OPENGL_COMPAT ||
         target == GL_TEXTURE_RECTANGLE_NV ||
         target == GL_PROXY_TEXTURE_RECTANGLE_NV) && border != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(border=%d)", dimensions, border);
      return GL_TRUE;
   }

   if (width < 0 || height < 0 || depth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(width, height or depth < 0)", dimensions);
      return GL_TRUE;
   }

   /* Check incoming image format and type */
   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      /* Prior to OpenGL-ES 2.0, an INVALID_VALUE is expected instead of
       * INVALID_ENUM. From page 73 OpenGL ES 1.1 spec:
       *
       *     "Specifying a value for internalformat that is not one of the
       *      above (acceptable) values generates the error INVALID VALUE."
       */
      if (err == GL_INVALID_ENUM && _mesa_is_gles(ctx) && ctx->Version < 20)
         err = GL_INVALID_VALUE;

      _mesa_error(ctx, err,
                  "glTexImage%dD(incompatible format = %s, type = %s)",
                  dimensions, _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type));
      return GL_TRUE;
   }

   /* Check internalFormat */
   if (_mesa_base_tex_format(ctx, internalFormat) < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexImage%dD(internalFormat=%s)",
                  dimensions, _mesa_enum_to_string(internalFormat));
      return GL_TRUE;
   }

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * combinations of format, internalFormat, and type that can be used.
    * Formats and types that require additional extensions (e.g., GL_FLOAT
    * requires GL_OES_texture_float) are filtered elsewhere.
    */
   char bufCallerName[20];
   snprintf(bufCallerName, 20, "glTexImage%dD", dimensions);
   if (_mesa_is_gles(ctx) &&
       texture_format_error_check_gles(ctx, format, type,
                                       internalFormat, bufCallerName)) {
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source(ctx, dimensions, &ctx->Unpack,
                                  width, height, depth, format, type,
                                  INT_MAX, pixels, "glTexImage")) {
      return GL_TRUE;
   }

   /* make sure internal format and format basically agree */
   if (!texture_formats_agree(internalFormat, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(incompatible internalFormat = %s, format = %s)",
                  dimensions, _mesa_enum_to_string(internalFormat),
                  _mesa_enum_to_string(format));
      return GL_TRUE;
   }

   /* additional checks for ycbcr textures */
   if (internalFormat == GL_YCBCR_MESA) {
      assert(ctx->Extensions.MESA_ycbcr_texture);
      if (type != GL_UNSIGNED_SHORT_8_8_MESA &&
          type != GL_UNSIGNED_SHORT_8_8_REV_MESA) {
         char message[100];
         snprintf(message, sizeof(message),
                        "glTexImage%dD(format/type YCBCR mismatch)",
                        dimensions);
         _mesa_error(ctx, GL_INVALID_ENUM, "%s", message);
         return GL_TRUE; /* error */
      }
      if (target != GL_TEXTURE_2D &&
          target != GL_PROXY_TEXTURE_2D &&
          target != GL_TEXTURE_RECTANGLE_NV &&
          target != GL_PROXY_TEXTURE_RECTANGLE_NV) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glTexImage%dD(bad target for YCbCr texture)",
                     dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         char message[100];
         snprintf(message, sizeof(message),
                        "glTexImage%dD(format=GL_YCBCR_MESA and border=%d)",
                        dimensions, border);
         _mesa_error(ctx, GL_INVALID_VALUE, "%s", message);
         return GL_TRUE;
      }
   }

   /* additional checks for depth textures */
   if (!_mesa_legal_texture_base_format_for_target(ctx, target, internalFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(bad target for texture)", dimensions);
      return GL_TRUE;
   }

   /* additional checks for compressed textures */
   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      GLenum err;
      if (!_mesa_target_can_be_compressed(ctx, target, internalFormat, &err)) {
         _mesa_error(ctx, err,
                     "glTexImage%dD(target can't be compressed)", dimensions);
         return GL_TRUE;
      }
      if (_mesa_format_no_online_compression(internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTexImage%dD(no compression for format)", dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTexImage%dD(border!=0)", dimensions);
         return GL_TRUE;
      }
   }

   /* additional checks for integer textures */
   if ((ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) &&
       (_mesa_is_enum_format_integer(format) !=
        _mesa_is_enum_format_integer(internalFormat))) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(integer/non-integer format mismatch)",
                  dimensions);
      return GL_TRUE;
   }

   if (!mutable_tex_object(texObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glTexImage%dD(immutable texture)", dimensions);
      return GL_TRUE;
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/**
 * Error checking for glCompressedTexImage[123]D().
 * Note that the width, height and depth values are not fully error checked
 * here.
 * \return GL_TRUE if a error is found, GL_FALSE otherwise
 */
static GLenum
compressed_texture_error_check(struct gl_context *ctx, GLint dimensions,
                               GLenum target, struct gl_texture_object* texObj,
                               GLint level, GLenum internalFormat, GLsizei width,
                               GLsizei height, GLsizei depth, GLint border,
                               GLsizei imageSize, const GLvoid *data)
{
   const GLint maxLevels = _mesa_max_texture_levels(ctx, target);
   GLint expectedSize;
   GLenum error = GL_NO_ERROR;
   char *reason = ""; /* no error */

   if (!_mesa_target_can_be_compressed(ctx, target, internalFormat, &error)) {
      reason = "target";
      goto error;
   }

   /* This will detect any invalid internalFormat value */
   if (!_mesa_is_compressed_format(ctx, internalFormat)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glCompressedTexImage%dD(internalFormat=%s)",
                  dimensions, _mesa_enum_to_string(internalFormat));
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source_compressed(ctx, dimensions, &ctx->Unpack,
                                             imageSize, data,
                                             "glCompressedTexImage")) {
      return GL_TRUE;
   }

   switch (internalFormat) {
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_PALETTE8_RGB5_A1_OES:
      /* check level (note that level should be zero or less!) */
      if (level > 0 || level < -maxLevels) {
         reason = "level";
         error = GL_INVALID_VALUE;
         goto error;
      }

      if (dimensions != 2) {
         reason = "compressed paletted textures must be 2D";
         error = GL_INVALID_OPERATION;
         goto error;
      }

      /* Figure out the expected texture size (in bytes).  This will be
       * checked against the actual size later.
       */
      expectedSize = _mesa_cpal_compressed_size(level, internalFormat,
                                                width, height);

      /* This is for the benefit of the TestProxyTexImage below.  It expects
       * level to be non-negative.  OES_compressed_paletted_texture uses a
       * weird mechanism where the level specified to glCompressedTexImage2D
       * is -(n-1) number of levels in the texture, and the data specifies the
       * complete mipmap stack.  This is done to ensure the palette is the
       * same for all levels.
       */
      level = -level;
      break;

   default:
      /* check level */
      if (level < 0 || level >= maxLevels) {
         reason = "level";
         error = GL_INVALID_VALUE;
         goto error;
      }

      /* Figure out the expected texture size (in bytes).  This will be
       * checked against the actual size later.
       */
      expectedSize = compressed_tex_size(width, height, depth, internalFormat);
      break;
   }

   /* This should really never fail */
   if (_mesa_base_tex_format(ctx, internalFormat) < 0) {
      reason = "internalFormat";
      error = GL_INVALID_ENUM;
      goto error;
   }

   /* No compressed formats support borders at this time */
   if (border != 0) {
      reason = "border != 0";
      error = _mesa_is_desktop_gl(ctx) ? GL_INVALID_OPERATION : GL_INVALID_VALUE;
      goto error;
   }

   /* Check for invalid pixel storage modes */
   if (!_mesa_compressed_pixel_storage_error_check(ctx, dimensions,
                                                   &ctx->Unpack,
                                                   "glCompressedTexImage")) {
      return GL_FALSE;
   }

   /* check image size in bytes */
   if (expectedSize != imageSize) {
      /* Per GL_ARB_texture_compression:  GL_INVALID_VALUE is generated [...]
       * if <imageSize> is not consistent with the format, dimensions, and
       * contents of the specified image.
       */
      reason = "imageSize inconsistent with width/height/format";
      error = GL_INVALID_VALUE;
      goto error;
   }

   if (!mutable_tex_object(texObj)) {
      reason = "immutable texture";
      error = GL_INVALID_OPERATION;
      goto error;
   }

   return GL_FALSE;

error:
   /* Note: not all error paths exit through here. */
   _mesa_error(ctx, error, "glCompressedTexImage%dD(%s)",
               dimensions, reason);
   return GL_TRUE;
}



/**
 * Test glTexSubImage[123]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user (already validated)
 * \param level image level given by the user.
 * \param xoffset sub-image x offset given by the user.
 * \param yoffset sub-image y offset given by the user.
 * \param zoffset sub-image z offset given by the user.
 * \param format pixel data format given by the user.
 * \param type pixel data type given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param depth image depth given by the user.
 *
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 */
static GLboolean
texsubimage_error_check(struct gl_context *ctx, GLuint dimensions,
                        struct gl_texture_object *texObj,
                        GLenum target, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLint width, GLint height, GLint depth,
                        GLenum format, GLenum type, const GLvoid *pixels,
                        const char *callerName)
{
   struct gl_texture_image *texImage;
   GLenum err;

   if (!texObj) {
      /* must be out of memory */
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s()", callerName);
      return GL_TRUE;
   }

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level=%d)", callerName, level);
      return GL_TRUE;
   }

   if (error_check_subtexture_negative_dimensions(ctx, dimensions,
                                                  width, height, depth,
                                                  callerName)) {
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      /* non-existant texture level */
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid texture level %d)",
                  callerName, level);
      return GL_TRUE;
   }

   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "%s(incompatible format = %s, type = %s)",
                  callerName, _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type));
      return GL_TRUE;
   }

   if (!texture_formats_agree(texImage->InternalFormat, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(incompatible internalFormat = %s, format = %s)",
                  callerName,
                  _mesa_enum_to_string(texImage->InternalFormat),
                  _mesa_enum_to_string(format));
      return GL_TRUE;
   }

   GLenum internalFormat = _mesa_is_gles(ctx) ?
      oes_float_internal_format(ctx, texImage->InternalFormat, type) :
      texImage->InternalFormat;

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * combinations of format, internalFormat, and type that can be used.
    * Formats and types that require additional extensions (e.g., GL_FLOAT
    * requires GL_OES_texture_float) are filtered elsewhere.
    */
   if (_mesa_is_gles(ctx) &&
       texture_format_error_check_gles(ctx, format, type,
                                       internalFormat, callerName)) {
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source(ctx, dimensions, &ctx->Unpack,
                                  width, height, depth, format, type,
                                  INT_MAX, pixels, callerName)) {
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dimensions,
                                         texImage, xoffset, yoffset, zoffset,
                                         width, height, depth, callerName)) {
      return GL_TRUE;
   }

   if (_mesa_is_format_compressed(texImage->TexFormat)) {
      if (_mesa_format_no_online_compression(texImage->InternalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "%s(no compression for format)", callerName);
         return GL_TRUE;
      }
   }

   if (ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) {
      /* both source and dest must be integer-valued, or neither */
      if (_mesa_is_format_integer_color(texImage->TexFormat) !=
          _mesa_is_enum_format_integer(format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer/non-integer format mismatch)", callerName);
         return GL_TRUE;
      }
   }

   return GL_FALSE;
}


/**
 * Test glCopyTexImage[12]D() parameters for errors.
 *
 * \param ctx GL context.
 * \param dimensions texture image dimensions (must be 1, 2 or 3).
 * \param target texture target given by the user.
 * \param level image level given by the user.
 * \param internalFormat internal format given by the user.
 * \param width image width given by the user.
 * \param height image height given by the user.
 * \param border texture border.
 *
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 *
 * Verifies each of the parameters against the constants specified in
 * __struct gl_contextRec::Const and the supported extensions, and according
 * to the OpenGL specification.
 */
static GLboolean
copytexture_error_check( struct gl_context *ctx, GLuint dimensions,
                         GLenum target, struct gl_texture_object* texObj,
                         GLint level, GLint internalFormat, GLint border )
{
   GLint baseFormat;
   GLint rb_base_format;
   struct gl_renderbuffer *rb;
   GLenum rb_internal_format;

   /* level check */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyTexImage%dD(level=%d)", dimensions, level);
      return GL_TRUE;
   }

   /* Check that the source buffer is complete */
   if (_mesa_is_user_fbo(ctx->ReadBuffer)) {
      if (ctx->ReadBuffer->_Status == 0) {
         _mesa_test_framebuffer_completeness(ctx, ctx->ReadBuffer);
      }
      if (ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                     "glCopyTexImage%dD(invalid readbuffer)", dimensions);
         return GL_TRUE;
      }

      if (!ctx->st_opts->allow_multisampled_copyteximage &&
          ctx->ReadBuffer->Visual.samples > 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(multisample FBO)", dimensions);
         return GL_TRUE;
      }
   }

   /* Check border */
   if (border < 0 || border > 1 ||
       ((ctx->API != API_OPENGL_COMPAT ||
         target == GL_TEXTURE_RECTANGLE_NV ||
         target == GL_PROXY_TEXTURE_RECTANGLE_NV) && border != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyTexImage%dD(border=%d)", dimensions, border);
      return GL_TRUE;
   }

   /* OpenGL ES 1.x and OpenGL ES 2.0 impose additional restrictions on the
    * internalFormat.
    */
   if (_mesa_is_gles(ctx) && !_mesa_is_gles3(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:

      /* Added by GL_OES_required_internalformat (always enabled) in table 3.4.y.*/
      case GL_ALPHA8:
      case GL_LUMINANCE8:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE4_ALPHA4:
      case GL_RGB565:
      case GL_RGB8:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_DEPTH24_STENCIL8:
      case GL_RGB10:
      case GL_RGB10_A2:
         break;

      default:
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_enum_to_string(internalFormat));
         return GL_TRUE;
      }
   } else {
      /*
       * Section 8.6 (Alternate Texture Image Specification Commands) of the
       * OpenGL 4.5 (Compatibility Profile) spec says:
       *
       *     "Parameters level, internalformat, and border are specified using
       *     the same values, with the same meanings, as the corresponding
       *     arguments of TexImage2D, except that internalformat may not be
       *     specified as 1, 2, 3, or 4."
       */
      if (internalFormat >= 1 && internalFormat <= 4) {
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glCopyTexImage%dD(internalFormat=%d)", dimensions,
                     internalFormat);
         return GL_TRUE;
      }
   }

   baseFormat = _mesa_base_tex_format(ctx, internalFormat);
   if (baseFormat < 0) {
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                  _mesa_enum_to_string(internalFormat));
      return GL_TRUE;
   }

   rb = _mesa_get_read_renderbuffer_for_format(ctx, internalFormat);
   if (rb == NULL) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(read buffer)", dimensions);
      return GL_TRUE;
   }

   rb_internal_format = rb->InternalFormat;
   rb_base_format = _mesa_base_tex_format(ctx, rb->InternalFormat);
   if (_mesa_is_color_format(internalFormat)) {
      if (rb_base_format < 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_enum_to_string(internalFormat));
         return GL_TRUE;
      }
   }

   if (_mesa_is_gles(ctx)) {
      bool valid = true;
      if (_mesa_components_in_format(baseFormat) >
          _mesa_components_in_format(rb_base_format)) {
         valid = false;
      }
      if (baseFormat == GL_DEPTH_COMPONENT ||
          baseFormat == GL_DEPTH_STENCIL ||
          baseFormat == GL_STENCIL_INDEX ||
          rb_base_format == GL_DEPTH_COMPONENT ||
          rb_base_format == GL_DEPTH_STENCIL ||
          rb_base_format == GL_STENCIL_INDEX ||
          ((baseFormat == GL_LUMINANCE_ALPHA ||
            baseFormat == GL_ALPHA) &&
           rb_base_format != GL_RGBA) ||
          internalFormat == GL_RGB9_E5) {
         valid = false;
      }
      if (internalFormat == GL_RGB9_E5) {
         valid = false;
      }
      if (!valid) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_enum_to_string(internalFormat));
         return GL_TRUE;
      }
   }

   if (_mesa_is_gles3(ctx)) {
      bool rb_is_srgb = (ctx->Extensions.EXT_sRGB &&
                         _mesa_is_format_srgb(rb->Format));
      bool dst_is_srgb = false;

      if (_mesa_get_linear_internalformat(internalFormat) != internalFormat) {
         dst_is_srgb = true;
      }

      if (rb_is_srgb != dst_is_srgb) {
         /* Page 137 (page 149 of the PDF) in section 3.8.5 of the
          * OpenGLES 3.0.0 spec says:
          *
          *     "The error INVALID_OPERATION is also generated if the
          *     value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING for the
          *     framebuffer attachment corresponding to the read buffer
          *     is LINEAR (see section 6.1.13) and internalformat is
          *     one of the sRGB formats described in section 3.8.16, or
          *     if the value of FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING is
          *     SRGB and internalformat is not one of the sRGB formats."
          */
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(srgb usage mismatch)", dimensions);
         return GL_TRUE;
      }

      /* Page 139, Table 3.15 of OpenGL ES 3.0 spec does not define ReadPixels
       * types for SNORM formats. Also, conversion to SNORM formats is not
       * allowed by Table 3.2 on Page 110.
       */
      if (!_mesa_has_EXT_render_snorm(ctx) &&
          _mesa_is_enum_format_snorm(internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(internalFormat=%s)", dimensions,
                     _mesa_enum_to_string(internalFormat));
         return GL_TRUE;
      }
   }

   if (!_mesa_source_buffer_exists(ctx, baseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(missing readbuffer)", dimensions);
      return GL_TRUE;
   }

   /* From the EXT_texture_integer spec:
    *
    *     "INVALID_OPERATION is generated by CopyTexImage* and CopyTexSubImage*
    *      if the texture internalformat is an integer format and the read color
    *      buffer is not an integer format, or if the internalformat is not an
    *      integer format and the read color buffer is an integer format."
    */
   if (_mesa_is_color_format(internalFormat)) {
      bool is_int = _mesa_is_enum_format_integer(internalFormat);
      bool is_rbint = _mesa_is_enum_format_integer(rb_internal_format);
      bool is_unorm = _mesa_is_enum_format_unorm(internalFormat);
      bool is_rbunorm = _mesa_is_enum_format_unorm(rb_internal_format);
      if (is_int || is_rbint) {
         if (is_int != is_rbint) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(integer vs non-integer)", dimensions);
            return GL_TRUE;
         } else if (_mesa_is_gles(ctx) &&
                    _mesa_is_enum_format_unsigned_int(internalFormat) !=
                      _mesa_is_enum_format_unsigned_int(rb_internal_format)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(signed vs unsigned integer)",
                        dimensions);
            return GL_TRUE;
         }
      }

      /* From page 138 of OpenGL ES 3.0 spec:
       *    "The error INVALID_OPERATION is generated if floating-point RGBA
       *    data is required; if signed integer RGBA data is required and the
       *    format of the current color buffer is not signed integer; if
       *    unsigned integer RGBA data is required and the format of the
       *    current color buffer is not unsigned integer; or if fixed-point
       *    RGBA data is required and the format of the current color buffer
       *    is not fixed-point.
       */
      if (_mesa_is_gles(ctx) && is_unorm != is_rbunorm)
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%dD(unorm vs non-unorm)", dimensions);
   }

   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      GLenum err;
      if (!_mesa_target_can_be_compressed(ctx, target, internalFormat, &err)) {
         _mesa_error(ctx, err,
                     "glCopyTexImage%dD(target can't be compressed)", dimensions);
         return GL_TRUE;
      }
      if (_mesa_format_no_online_compression(internalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "glCopyTexImage%dD(no compression for format)", dimensions);
         return GL_TRUE;
      }
      if (border != 0) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyTexImage%dD(border!=0)", dimensions);
         return GL_TRUE;
      }
   }

   if (!mutable_tex_object(texObj)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyTexImage%dD(immutable texture)", dimensions);
      return GL_TRUE;
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/**
 * Test glCopyTexSubImage[12]D() parameters for errors.
 * \return GL_TRUE if an error was detected, or GL_FALSE if no errors.
 */
static GLboolean
copytexsubimage_error_check(struct gl_context *ctx, GLuint dimensions,
                            const struct gl_texture_object *texObj,
                            GLenum target, GLint level,
                            GLint xoffset, GLint yoffset, GLint zoffset,
                            GLint width, GLint height, const char *caller)
{
   assert(texObj);

   struct gl_texture_image *texImage;

   /* Check that the source buffer is complete */
   if (_mesa_is_user_fbo(ctx->ReadBuffer)) {
      if (ctx->ReadBuffer->_Status == 0) {
         _mesa_test_framebuffer_completeness(ctx, ctx->ReadBuffer);
      }
      if (ctx->ReadBuffer->_Status != GL_FRAMEBUFFER_COMPLETE_EXT) {
         _mesa_error(ctx, GL_INVALID_FRAMEBUFFER_OPERATION_EXT,
                     "%s(invalid readbuffer)", caller);
         return GL_TRUE;
      }

      /**
       * From the GL_EXT_multisampled_render_to_texture spec:
       *
       * "An INVALID_OPERATION error is generated by CopyTexSubImage3D,
       * CopyTexImage2D, or CopyTexSubImage2D if [...] the value of
       * READ_FRAMEBUFFER_BINDING is non-zero, and:
       *   - the read buffer selects an attachment that has no image attached,
       *     or
       *   - the value of SAMPLE_BUFFERS for the read framebuffer is one."
       *
       * [...]
       * These errors do not apply to textures and renderbuffers that have
       * associated multisample data specified by the mechanisms described in
       * this extension, i.e., the above operations are allowed even when
       * SAMPLE_BUFFERS is non-zero for renderbuffers created via Renderbuffer-
       * StorageMultisampleEXT or textures attached via FramebufferTexture2D-
       * MultisampleEXT.
       */
      if (!ctx->st_opts->allow_multisampled_copyteximage &&
          ctx->ReadBuffer->Visual.samples > 0 &&
          !_mesa_has_rtt_samples(ctx->ReadBuffer)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(multisample FBO)",
                     caller);
         return GL_TRUE;
      }
   }

   /* Check level */
   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level=%d)", caller, level);
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      /* destination image does not exist */
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(invalid texture level %d)", caller, level);
      return GL_TRUE;
   }

   if (error_check_subtexture_negative_dimensions(ctx, dimensions,
                                                  width, height, 1, caller)) {
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dimensions, texImage,
                                         xoffset, yoffset, zoffset,
                                         width, height, 1, caller)) {
      return GL_TRUE;
   }

   if (_mesa_is_format_compressed(texImage->TexFormat)) {
      if (_mesa_format_no_online_compression(texImage->InternalFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
               "%s(no compression for format)", caller);
         return GL_TRUE;
      }
   }

   if (texImage->InternalFormat == GL_YCBCR_MESA) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s()", caller);
      return GL_TRUE;
   }

   /* From OpenGL ES 3.2 spec, section 8.6:
    *
    *     "An INVALID_OPERATION error is generated by CopyTexSubImage3D,
    *      CopyTexImage2D, or CopyTexSubImage2D if the internalformat of the
    *      texture image being (re)specified is RGB9_E5"
    */
   if (texImage->InternalFormat == GL_RGB9_E5 &&
       !_mesa_is_desktop_gl(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(invalid internal format %s)", caller,
                  _mesa_enum_to_string(texImage->InternalFormat));
      return GL_TRUE;
   }

   if (!_mesa_source_buffer_exists(ctx, texImage->_BaseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(missing readbuffer, format=%s)", caller,
                  _mesa_enum_to_string(texImage->_BaseFormat));
      return GL_TRUE;
   }

   /* From the EXT_texture_integer spec:
    *
    *     "INVALID_OPERATION is generated by CopyTexImage* and
    *     CopyTexSubImage* if the texture internalformat is an integer format
    *     and the read color buffer is not an integer format, or if the
    *     internalformat is not an integer format and the read color buffer
    *     is an integer format."
    */
   if (_mesa_is_color_format(texImage->InternalFormat)) {
      struct gl_renderbuffer *rb = ctx->ReadBuffer->_ColorReadBuffer;

      if (_mesa_is_format_integer_color(rb->Format) !=
          _mesa_is_format_integer_color(texImage->TexFormat)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer vs non-integer)", caller);
         return GL_TRUE;
      }
   }

   /* In the ES 3.2 specification's Table 8.13 (Valid CopyTexImage source
    * framebuffer/destination texture base internal format combinations),
    * all the entries for stencil are left blank (unsupported).
    */
   if (_mesa_is_gles(ctx) && _mesa_is_stencil_format(texImage->_BaseFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(stencil disallowed)", caller);
      return GL_TRUE;
   }

   /* if we get here, the parameters are OK */
   return GL_FALSE;
}


/** Callback info for walking over FBO hash table */
struct cb_info
{
   struct gl_context *ctx;
   struct gl_texture_object *texObj;
   GLuint level, face;
};


/**
 * Check render to texture callback.  Called from _mesa_HashWalk().
 */
static void
check_rtt_cb(void *data, void *userData)
{
   struct gl_framebuffer *fb = (struct gl_framebuffer *) data;
   const struct cb_info *info = (struct cb_info *) userData;
   struct gl_context *ctx = info->ctx;
   const struct gl_texture_object *texObj = info->texObj;
   const GLuint level = info->level, face = info->face;

   /* If this is a user-created FBO */
   if (_mesa_is_user_fbo(fb)) {
      GLuint i;
      /* check if any of the FBO's attachments point to 'texObj' */
      for (i = 0; i < BUFFER_COUNT; i++) {
         struct gl_renderbuffer_attachment *att = fb->Attachment + i;
         if (att->Type == GL_TEXTURE &&
             att->Texture == texObj &&
             att->TextureLevel == level &&
             att->CubeMapFace == face) {
            _mesa_update_texture_renderbuffer(ctx, fb, att);
            assert(att->Renderbuffer->TexImage);
            /* Mark fb status as indeterminate to force re-validation */
            fb->_Status = 0;

            /* Make sure that the revalidation actually happens if this is
             * being done to currently-bound buffers.
             */
            if (fb == ctx->DrawBuffer || fb == ctx->ReadBuffer)
               ctx->NewState |= _NEW_BUFFERS;
         }
      }
   }
}


/**
 * When a texture image is specified we have to check if it's bound to
 * any framebuffer objects (render to texture) in order to detect changes
 * in size or format since that effects FBO completeness.
 * Any FBOs rendering into the texture must be re-validated.
 */
void
_mesa_update_fbo_texture(struct gl_context *ctx,
                         struct gl_texture_object *texObj,
                         GLuint face, GLuint level)
{
   /* Only check this texture if it's been marked as RenderToTexture */
   if (texObj->_RenderToTexture) {
      struct cb_info info;
      info.ctx = ctx;
      info.texObj = texObj;
      info.level = level;
      info.face = face;
      _mesa_HashWalk(ctx->Shared->FrameBuffers, check_rtt_cb, &info);
   }
}


/**
 * If the texture object's GenerateMipmap flag is set and we've
 * changed the texture base level image, regenerate the rest of the
 * mipmap levels now.
 */
static inline void
check_gen_mipmap(struct gl_context *ctx, GLenum target,
                 struct gl_texture_object *texObj, GLint level)
{
   if (texObj->Attrib.GenerateMipmap &&
       level == texObj->Attrib.BaseLevel &&
       level < texObj->Attrib.MaxLevel) {
      st_generate_mipmap(ctx, target, texObj);
   }
}


/** Debug helper: override the user-requested internal format */
static GLenum
override_internal_format(GLenum internalFormat, UNUSED GLint width,
                         UNUSED GLint height)
{
#if 0
   if (internalFormat == GL_RGBA16F_ARB ||
       internalFormat == GL_RGBA32F_ARB) {
      printf("Convert rgba float tex to int %d x %d\n", width, height);
      return GL_RGBA;
   }
   else if (internalFormat == GL_RGB16F_ARB ||
            internalFormat == GL_RGB32F_ARB) {
      printf("Convert rgb float tex to int %d x %d\n", width, height);
      return GL_RGB;
   }
   else if (internalFormat == GL_LUMINANCE_ALPHA16F_ARB ||
            internalFormat == GL_LUMINANCE_ALPHA32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_LUMINANCE_ALPHA;
   }
   else if (internalFormat == GL_LUMINANCE16F_ARB ||
            internalFormat == GL_LUMINANCE32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_LUMINANCE;
   }
   else if (internalFormat == GL_ALPHA16F_ARB ||
            internalFormat == GL_ALPHA32F_ARB) {
      printf("Convert luminance float tex to int %d x %d\n", width, height);
      return GL_ALPHA;
   }
   /*
   else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
      internalFormat = GL_RGBA;
   }
   */
   else {
      return internalFormat;
   }
#else
   return internalFormat;
#endif
}


/**
 * Choose the actual hardware format for a texture image.
 * Try to use the same format as the previous image level when possible.
 * Otherwise, ask the driver for the best format.
 * It's important to try to choose a consistant format for all levels
 * for efficient texture memory layout/allocation.  In particular, this
 * comes up during automatic mipmap generation.
 */
mesa_format
_mesa_choose_texture_format(struct gl_context *ctx,
                            struct gl_texture_object *texObj,
                            GLenum target, GLint level,
                            GLenum internalFormat, GLenum format, GLenum type)
{
   mesa_format f = MESA_FORMAT_NONE;

   /* see if we've already chosen a format for the previous level */
   if (level > 0) {
      struct gl_texture_image *prevImage =
         _mesa_select_tex_image(texObj, target, level - 1);
      /* See if the prev level is defined and has an internal format which
       * matches the new internal format.
       */
      if (prevImage &&
          prevImage->Width > 0 &&
          prevImage->InternalFormat == internalFormat) {
         /* use the same format */
         assert(prevImage->TexFormat != MESA_FORMAT_NONE);
         return prevImage->TexFormat;
      }
   }

   f = st_ChooseTextureFormat(ctx, target, internalFormat,
                              format, type);
   assert(f != MESA_FORMAT_NONE);
   return f;
}


/**
 * Adjust pixel unpack params and image dimensions to strip off the
 * one-pixel texture border.
 *
 * Gallium and intel don't support texture borders.  They've seldem been used
 * and seldom been implemented correctly anyway.
 *
 * \param unpackNew returns the new pixel unpack parameters
 */
static void
strip_texture_border(GLenum target,
                     GLint *width, GLint *height, GLint *depth,
                     const struct gl_pixelstore_attrib *unpack,
                     struct gl_pixelstore_attrib *unpackNew)
{
   assert(width);
   assert(height);
   assert(depth);

   *unpackNew = *unpack;

   if (unpackNew->RowLength == 0)
      unpackNew->RowLength = *width;

   if (unpackNew->ImageHeight == 0)
      unpackNew->ImageHeight = *height;

   assert(*width >= 3);
   unpackNew->SkipPixels++;  /* skip the border */
   *width = *width - 2;      /* reduce the width by two border pixels */

   /* The min height of a texture with a border is 3 */
   if (*height >= 3 && target != GL_TEXTURE_1D_ARRAY) {
      unpackNew->SkipRows++;  /* skip the border */
      *height = *height - 2;  /* reduce the height by two border pixels */
   }

   if (*depth >= 3 &&
       target != GL_TEXTURE_2D_ARRAY &&
       target != GL_TEXTURE_CUBE_MAP_ARRAY) {
      unpackNew->SkipImages++;  /* skip the border */
      *depth = *depth - 2;      /* reduce the depth by two border pixels */
   }
}

static struct gl_texture_object *
lookup_texture_ext_dsa(struct gl_context *ctx, GLenum target, GLuint texture,
                       const char *caller)
{
   GLenum boundTarget;
   switch (target) {
   case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
   case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
   case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      boundTarget = GL_TEXTURE_CUBE_MAP;
      break;
   default:
      boundTarget = target;
      break;
   }

   int targetIndex = _mesa_tex_target_to_index(ctx, boundTarget);
   if (targetIndex < 0) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target = %s)", caller,
                  _mesa_enum_to_string(target));
            return NULL;
   }
   assert(targetIndex < NUM_TEXTURE_TARGETS);

   struct gl_texture_object *texObj;
   if (texture == 0) {
      /* Use a default texture object */
      texObj = ctx->Shared->DefaultTex[targetIndex];
      assert(texObj);
   } else {
      bool isGenName;
      texObj = _mesa_lookup_texture(ctx, texture);
      isGenName = texObj != NULL;
      if (!texObj && _mesa_is_desktop_gl_core(ctx)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(non-gen name)", caller);
         return NULL;
      }

      if (!texObj) {
         texObj = _mesa_new_texture_object(ctx, texture, boundTarget);
         if (!texObj) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
            return NULL;
         }

         /* insert into hash table */
         _mesa_HashInsert(ctx->Shared->TexObjects, texObj->Name, texObj, isGenName);
      }

      if (texObj->Target != boundTarget) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(%s != %s)",
                     caller, _mesa_enum_to_string(texObj->Target),
                     _mesa_enum_to_string(target));
         return NULL;
      }
   }

   return texObj;
}

/**
 * Common code to implement all the glTexImage1D/2D/3D functions,
 * glCompressedTexImage1D/2D/3D and glTextureImage1D/2D/3DEXT
 * \param compressed  only GL_TRUE for glCompressedTexImage1D/2D/3D calls.
 * \param format  the user's image format (only used if !compressed)
 * \param type  the user's image type (only used if !compressed)
 * \param imageSize  only used for glCompressedTexImage1D/2D/3D calls.
 */
static ALWAYS_INLINE void
teximage(struct gl_context *ctx, GLboolean compressed, GLuint dims,
         struct gl_texture_object *texObj,
         GLenum target, GLint level, GLint internalFormat,
         GLsizei width, GLsizei height, GLsizei depth,
         GLint border, GLenum format, GLenum type,
         GLsizei imageSize, const GLvoid *pixels, bool no_error)
{
   const char *func = compressed ? "glCompressedTexImage" : "glTexImage";
   struct gl_pixelstore_attrib unpack_no_border;
   const struct gl_pixelstore_attrib *unpack = &ctx->Unpack;
   mesa_format texFormat;
   bool dimensionsOK = true, sizeOK = true;

   FLUSH_VERTICES(ctx, 0, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE)) {
      if (compressed)
         _mesa_debug(ctx,
                     "glCompressedTexImage%uD %s %d %s %d %d %d %d %p\n",
                     dims,
                     _mesa_enum_to_string(target), level,
                     _mesa_enum_to_string(internalFormat),
                     width, height, depth, border, pixels);
      else
         _mesa_debug(ctx,
                     "glTexImage%uD %s %d %s %d %d %d %d %s %s %p\n",
                     dims,
                     _mesa_enum_to_string(target), level,
                     _mesa_enum_to_string(internalFormat),
                     width, height, depth, border,
                     _mesa_enum_to_string(format),
                     _mesa_enum_to_string(type), pixels);
   }

   internalFormat = override_internal_format(internalFormat, width, height);

   if (!no_error &&
       /* target error checking */
       !legal_teximage_target(ctx, dims, target)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s%uD(target=%s)",
                  func, dims, _mesa_enum_to_string(target));
      return;
   }

   if (!texObj)
      texObj = _mesa_get_current_tex_object(ctx, target);

   if (!no_error) {
      /* general error checking */
      if (compressed) {
         if (compressed_texture_error_check(ctx, dims, target, texObj,
                                            level, internalFormat,
                                            width, height, depth,
                                            border, imageSize, pixels))
            return;
      } else {
         if (texture_error_check(ctx, dims, target, texObj, level, internalFormat,
                                 format, type, width, height, depth, border,
                                 pixels))
            return;
      }
   }
   assert(texObj);

   /* Here we convert a cpal compressed image into a regular glTexImage2D
    * call by decompressing the texture.  If we really want to support cpal
    * textures in any driver this would have to be changed.
    */
   if (_mesa_is_gles1(ctx) && compressed && dims == 2) {
      switch (internalFormat) {
      case GL_PALETTE4_RGB8_OES:
      case GL_PALETTE4_RGBA8_OES:
      case GL_PALETTE4_R5_G6_B5_OES:
      case GL_PALETTE4_RGBA4_OES:
      case GL_PALETTE4_RGB5_A1_OES:
      case GL_PALETTE8_RGB8_OES:
      case GL_PALETTE8_RGBA8_OES:
      case GL_PALETTE8_R5_G6_B5_OES:
      case GL_PALETTE8_RGBA4_OES:
      case GL_PALETTE8_RGB5_A1_OES:
         _mesa_cpal_compressed_teximage2d(target, level, internalFormat,
                                          width, height, imageSize, pixels);
         return;
      }
   }

   if (compressed) {
      /* For glCompressedTexImage() the driver has no choice about the
       * texture format since we'll never transcode the user's compressed
       * image data.  The internalFormat was error checked earlier.
       */
      texFormat = _mesa_glenum_to_compressed_format(internalFormat);
   }
   else {
      /* In case of HALF_FLOAT_OES or FLOAT_OES, find corresponding sized
       * internal floating point format for the given base format.
       */
      if (_mesa_is_gles(ctx) && format == internalFormat) {
         if (type == GL_FLOAT) {
            texObj->_IsFloat = GL_TRUE;
         } else if (type == GL_HALF_FLOAT_OES || type == GL_HALF_FLOAT) {
            texObj->_IsHalfFloat = GL_TRUE;
         }

         internalFormat = adjust_for_oes_float_texture(ctx, format, type);
      }

      texFormat = _mesa_choose_texture_format(ctx, texObj, target, level,
                                              internalFormat, format, type);
   }

   assert(texFormat != MESA_FORMAT_NONE);

   if (!no_error) {
      /* check that width, height, depth are legal for the mipmap level */
      dimensionsOK = _mesa_legal_texture_dimensions(ctx, target, level, width,
                                                    height, depth, border);

      /* check that the texture won't take too much memory, etc */
      sizeOK = st_TestProxyTexImage(ctx, proxy_target(target),
                                    0, level, texFormat, 1,
                                    width, height, depth);
   }

   if (_mesa_is_proxy_texture(target)) {
      /* Proxy texture: just clear or set state depending on error checking */
      struct gl_texture_image *texImage =
         get_proxy_tex_image(ctx, target, level);

      if (!texImage)
         return;  /* GL_OUT_OF_MEMORY already recorded */

      if (dimensionsOK && sizeOK) {
         _mesa_init_teximage_fields(ctx, texImage, width, height, depth,
                                    border, internalFormat, texFormat);
      }
      else {
         clear_teximage_fields(texImage);
      }
   }
   else {
      /* non-proxy target */
      const GLuint face = _mesa_tex_target_to_face(target);
      struct gl_texture_image *texImage;

      if (!dimensionsOK) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s%uD(invalid width=%d or height=%d or depth=%d)",
                     func, dims, width, height, depth);
         return;
      }

      if (!sizeOK) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY,
                     "%s%uD(image too large: %d x %d x %d, %s format)",
                     func, dims, width, height, depth,
                     _mesa_enum_to_string(internalFormat));
         return;
      }

      /* Allow a hardware driver to just strip out the border, to provide
       * reliable but slightly incorrect hardware rendering instead of
       * rarely-tested software fallback rendering.
       */
      if (border) {
         strip_texture_border(target, &width, &height, &depth, unpack,
                              &unpack_no_border);
         border = 0;
         unpack = &unpack_no_border;
      }

      _mesa_update_pixel(ctx);

      _mesa_lock_texture(ctx, texObj);
      {
         texObj->External = GL_FALSE;

         texImage = _mesa_get_tex_image(ctx, texObj, target, level);

         if (!texImage) {
            _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s%uD", func, dims);
         }
         else {
            st_FreeTextureImageBuffer(ctx, texImage);

            _mesa_init_teximage_fields(ctx, texImage,
                                       width, height, depth,
                                       border, internalFormat, texFormat);

            /* Give the texture to the driver.  <pixels> may be null. */
            if (width > 0 && height > 0 && depth > 0) {
               if (compressed) {
                  st_CompressedTexImage(ctx, dims, texImage,
                                        imageSize, pixels);
               }
               else {
                  st_TexImage(ctx, dims, texImage, format,
                              type, pixels, unpack);
               }
            }

            check_gen_mipmap(ctx, target, texObj, level);

            _mesa_update_fbo_texture(ctx, texObj, face, level);

            _mesa_dirty_texobj(ctx, texObj);
            /* only apply depthMode swizzle if it was explicitly changed */
            GLenum depth_mode = _mesa_is_desktop_gl_core(ctx) ? GL_RED : GL_LUMINANCE;
            if (texObj->Attrib.DepthMode != depth_mode)
               _mesa_update_teximage_format_swizzle(ctx, texObj->Image[0][texObj->Attrib.BaseLevel], texObj->Attrib.DepthMode);
            _mesa_update_texture_object_swizzle(ctx, texObj);
         }
      }
      _mesa_unlock_texture(ctx, texObj);
   }
}


/* This is a wrapper around teximage() so that we can force the KHR_no_error
 * logic to be inlined without inlining the function into all the callers.
 */
static void
teximage_err(struct gl_context *ctx, GLboolean compressed, GLuint dims,
             GLenum target, GLint level, GLint internalFormat,
             GLsizei width, GLsizei height, GLsizei depth,
             GLint border, GLenum format, GLenum type,
             GLsizei imageSize, const GLvoid *pixels)
{
   teximage(ctx, compressed, dims, NULL, target, level, internalFormat, width, height,
            depth, border, format, type, imageSize, pixels, false);
}


static void
teximage_no_error(struct gl_context *ctx, GLboolean compressed, GLuint dims,
                  GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLsizei depth,
                  GLint border, GLenum format, GLenum type,
                  GLsizei imageSize, const GLvoid *pixels)
{
   teximage(ctx, compressed, dims, NULL, target, level, internalFormat, width, height,
            depth, border, format, type, imageSize, pixels, true);
}


/*
 * Called from the API.  Note that width includes the border.
 */
void GLAPIENTRY
_mesa_TexImage1D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLint border, GLenum format,
                  GLenum type, const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_FALSE, 1, target, level, internalFormat, width, 1, 1,
                border, format, type, 0, pixels);
}

void GLAPIENTRY
_mesa_TextureImage1DEXT(GLuint texture, GLenum target, GLint level,
                      GLint internalFormat, GLsizei width, GLint border,
                      GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureImage1DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 1, texObj, target, level, internalFormat,
            width, 1, 1, border, format, type, 0, pixels, false);
}

void GLAPIENTRY
_mesa_MultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level,
                         GLint internalFormat, GLsizei width, GLint border,
                         GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexImage1DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 1, texObj, target, level, internalFormat, width, 1, 1,
                border, format, type, 0, pixels, false);
}

void GLAPIENTRY
_mesa_TexImage2D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type,
                  const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_FALSE, 2, target, level, internalFormat, width, height, 1,
                border, format, type, 0, pixels);
}

void GLAPIENTRY
_mesa_TextureImage2DEXT(GLuint texture, GLenum target, GLint level,
                      GLint internalFormat, GLsizei width, GLsizei height,
                      GLint border,
                      GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureImage2DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 2, texObj, target, level, internalFormat,
            width, height, 1, border, format, type, 0, pixels, false);
}

void GLAPIENTRY
_mesa_MultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level,
                         GLint internalFormat, GLsizei width, GLsizei height,
                         GLint border,
                         GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexImage2DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 2, texObj, target, level, internalFormat, width, height, 1,
                border, format, type, 0, pixels, false);
}

/*
 * Called by the API or display list executor.
 * Note that width and height include the border.
 */
void GLAPIENTRY
_mesa_TexImage3D( GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLsizei depth,
                  GLint border, GLenum format, GLenum type,
                  const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_FALSE, 3, target, level, internalFormat,
                width, height, depth, border, format, type, 0, pixels);
}

void GLAPIENTRY
_mesa_TextureImage3DEXT(GLuint texture, GLenum target, GLint level,
                      GLint internalFormat, GLsizei width, GLsizei height,
                      GLsizei depth, GLint border,
                      GLenum format, GLenum type, const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureImage3DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 3, texObj, target, level, internalFormat,
            width, height, depth, border, format, type, 0, pixels, false);
}


void GLAPIENTRY
_mesa_MultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level,
                         GLint internalFormat, GLsizei width, GLsizei height,
                         GLsizei depth, GLint border, GLenum format, GLenum type,
                         const GLvoid *pixels )
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexImage3DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_FALSE, 3, texObj, target, level, internalFormat,
                width, height, depth, border, format, type, 0, pixels, false);
}


void GLAPIENTRY
_mesa_TexImage1D_no_error(GLenum target, GLint level, GLint internalFormat,
                          GLsizei width, GLint border, GLenum format,
                          GLenum type, const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_FALSE, 1, target, level, internalFormat, width, 1,
                     1, border, format, type, 0, pixels);
}


void GLAPIENTRY
_mesa_TexImage2D_no_error(GLenum target, GLint level, GLint internalFormat,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_FALSE, 2, target, level, internalFormat, width,
                     height, 1, border, format, type, 0, pixels);
}


void GLAPIENTRY
_mesa_TexImage3D_no_error(GLenum target, GLint level, GLint internalFormat,
                          GLsizei width, GLsizei height, GLsizei depth,
                          GLint border, GLenum format, GLenum type,
                          const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_FALSE, 3, target, level, internalFormat,
                     width, height, depth, border, format, type, 0, pixels);
}

/*
 * Helper used by __mesa_EGLImageTargetTexture2DOES and
 * _mesa_EGLImageTargetTexStorageEXT.
 */
static void
egl_image_target_texture(struct gl_context *ctx,
                         struct gl_texture_object *texObj, GLenum target,
                         GLeglImageOES image, bool tex_storage,
                         const char *caller)
{
   struct gl_texture_image *texImage;
   FLUSH_VERTICES(ctx, 0, 0);

   if (!texObj)
      texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (!image || (ctx->Driver.ValidateEGLImage &&
                  !ctx->Driver.ValidateEGLImage(ctx, image))) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(image=%p)", caller, image);
      return;
   }

   _mesa_lock_texture(ctx, texObj);

   if (texObj->Immutable) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(texture is immutable)", caller);
      _mesa_unlock_texture(ctx, texObj);
      return;
   }

   texImage = _mesa_get_tex_image(ctx, texObj, target, 0);
   if (!texImage) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s", caller);
   } else {
      st_FreeTextureImageBuffer(ctx, texImage);

      texObj->External = GL_TRUE;

      struct st_egl_image stimg;
      bool native_supported;
      if (!st_get_egl_image(ctx, image, PIPE_BIND_SAMPLER_VIEW, caller,
                            &stimg, &native_supported)) {
         _mesa_unlock_texture(ctx, texObj);
         return;
      }

      if (tex_storage) {
         /* EXT_EGL_image_storage
          * If the EGL image was created using EGL_EXT_image_dma_buf_import,
          * then the following applies:
          *    - <target> must be GL_TEXTURE_2D or GL_TEXTURE_EXTERNAL_OES.
          *    Otherwise, the error INVALID_OPERATION is generated.
          */
         if (stimg.imported_dmabuf &&
             !(target == GL_TEXTURE_2D || target == GL_TEXTURE_EXTERNAL_OES)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(texture is imported from dmabuf)", caller);
            pipe_resource_reference(&stimg.texture, NULL);
            _mesa_unlock_texture(ctx, texObj);
            return;
         }
         st_bind_egl_image(ctx, texObj, texImage, &stimg, true, native_supported);
      } else {
         st_bind_egl_image(ctx, texObj, texImage, &stimg,
                           target != GL_TEXTURE_EXTERNAL_OES, native_supported);
      }

      pipe_resource_reference(&stimg.texture, NULL);

      _mesa_dirty_texobj(ctx, texObj);
   }

   if (tex_storage)
      _mesa_set_texture_view_state(ctx, texObj, target, 1);

   _mesa_update_fbo_texture(ctx, texObj, 0, 0);

   _mesa_unlock_texture(ctx, texObj);
}

void GLAPIENTRY
_mesa_EGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
   const char *func = "glEGLImageTargetTexture2D";
   GET_CURRENT_CONTEXT(ctx);

   bool valid_target;
   switch (target) {
   case GL_TEXTURE_2D:
      valid_target = _mesa_has_OES_EGL_image(ctx);
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      valid_target = _mesa_has_OES_EGL_image_external(ctx);
      break;
   default:
      valid_target = false;
      break;
   }

   if (valid_target) {
      egl_image_target_texture(ctx, NULL, target, image, false, func);
   } else {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(target=%d)", func, target);
   }
}

static void
egl_image_target_texture_storage(struct gl_context *ctx,
                                 struct gl_texture_object *texObj, GLenum target,
                                 GLeglImageOES image, const GLint *attrib_list,
                                 const char *caller)
{
   /*
    * EXT_EGL_image_storage:
    *
    * "<attrib_list> must be NULL or a pointer to the value GL_NONE."
    */
   if (attrib_list && attrib_list[0] != GL_NONE) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(image=%p)", caller, image);
      return;
   }

   /*
    * <target> must be one of GL_TEXTURE_2D, GL_TEXTURE_2D_ARRAY,
    * GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY. On
    * OpenGL implementations (non-ES), <target> can also be GL_TEXTURE_1D or
    * GL_TEXTURE_1D_ARRAY.
    * If the implementation supports OES_EGL_image_external, <target> can be
    * GL_TEXTURE_EXTERNAL_OES.
    */
   bool valid_target;
   switch (target) {
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
      valid_target = true;
      break;
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
      /* No eglImageOES for 1D textures */
      valid_target = !_mesa_is_gles(ctx);
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      valid_target = _mesa_has_OES_EGL_image_external(ctx);
      break;
   default:
      valid_target = false;
      break;
   }

   if (valid_target) {
      egl_image_target_texture(ctx, texObj, target, image, true, caller);
   } else {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(target=%d)", caller, target);
   }

}


void GLAPIENTRY
_mesa_EGLImageTargetTexStorageEXT(GLenum target, GLeglImageOES image,
                                  const GLint *attrib_list)
{
   const char *func = "glEGLImageTargetTexStorageEXT";
   GET_CURRENT_CONTEXT(ctx);

   if (!(_mesa_is_desktop_gl(ctx) && ctx->Version >= 42) &&
       !_mesa_is_gles3(ctx) && !_mesa_has_ARB_texture_storage(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
         "OpenGL 4.2, OpenGL ES 3.0 or ARB_texture_storage required");
      return;
   }

   egl_image_target_texture_storage(ctx, NULL, target, image, attrib_list,
                                    func);
}

void GLAPIENTRY
_mesa_EGLImageTargetTextureStorageEXT(GLuint texture, GLeglImageOES image,
                                      const GLint *attrib_list)
{
   struct gl_texture_object *texObj;
   const char *func = "glEGLImageTargetTextureStorageEXT";
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_has_ARB_direct_state_access(ctx) &&
       !_mesa_has_EXT_direct_state_access(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "direct access not supported");
      return;
   }

   if (!(_mesa_is_desktop_gl(ctx) && ctx->Version >= 42) &&
       !_mesa_is_gles3(ctx) && !_mesa_has_ARB_texture_storage(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
         "OpenGL 4.2, OpenGL ES 3.0 or ARB_texture_storage required");
      return;
   }

   texObj = _mesa_lookup_texture_err(ctx, texture, func);
   if (!texObj)
      return;

   egl_image_target_texture_storage(ctx, texObj, texObj->Target, image,
                                    attrib_list, func);
}

/**
 * Helper that implements the glTexSubImage1/2/3D()
 * and glTextureSubImage1/2/3D() functions.
 */
static void
texture_sub_image(struct gl_context *ctx, GLuint dims,
                  struct gl_texture_object *texObj,
                  struct gl_texture_image *texImage,
                  GLenum target, GLint level,
                  GLint xoffset, GLint yoffset, GLint zoffset,
                  GLsizei width, GLsizei height, GLsizei depth,
                  GLenum format, GLenum type, const GLvoid *pixels)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _mesa_update_pixel(ctx);

   _mesa_lock_texture(ctx, texObj);
   {
      if (width > 0 && height > 0 && depth > 0) {
         /* If we have a border, offset=-1 is legal.  Bias by border width. */
         switch (dims) {
         case 3:
            if (target != GL_TEXTURE_2D_ARRAY)
               zoffset += texImage->Border;
            FALLTHROUGH;
         case 2:
            if (target != GL_TEXTURE_1D_ARRAY)
               yoffset += texImage->Border;
            FALLTHROUGH;
         case 1:
            xoffset += texImage->Border;
         }

         st_TexSubImage(ctx, dims, texImage,
                        xoffset, yoffset, zoffset,
                        width, height, depth,
                        format, type, pixels, &ctx->Unpack);

         check_gen_mipmap(ctx, target, texObj, level);

         /* NOTE: Don't signal _NEW_TEXTURE_OBJECT since we've only changed
          * the texel data, not the texture format, size, etc.
          */
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}

/**
 * Implement all the glTexSubImage1/2/3D() functions.
 * Must split this out this way because of GL_TEXTURE_CUBE_MAP.
 */
static void
texsubimage_err(struct gl_context *ctx, GLuint dims, GLenum target, GLint level,
                GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type, const GLvoid *pixels,
                const char *callerName)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   /* check target (proxies not allowed) */
   if (!legal_texsubimage_target(ctx, dims, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glTexSubImage%uD(target=%s)",
                  dims, _mesa_enum_to_string(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   if (texsubimage_error_check(ctx, dims, texObj, target, level,
                               xoffset, yoffset, zoffset,
                               width, height, depth, format, type,
                               pixels, callerName)) {
      return;   /* error was detected */
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   /* texsubimage_error_check ensures that texImage is not NULL */

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glTexSubImage%uD %s %d %d %d %d %d %d %d %s %s %p\n",
                  dims,
                  _mesa_enum_to_string(target), level,
                  xoffset, yoffset, zoffset, width, height, depth,
                  _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type), pixels);

   texture_sub_image(ctx, dims, texObj, texImage, target, level,
                     xoffset, yoffset, zoffset, width, height, depth,
                     format, type, pixels);
}


static void
texsubimage(struct gl_context *ctx, GLuint dims, GLenum target, GLint level,
            GLint xoffset, GLint yoffset, GLint zoffset,
            GLsizei width, GLsizei height, GLsizei depth,
            GLenum format, GLenum type, const GLvoid *pixels)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   texObj = _mesa_get_current_tex_object(ctx, target);
   texImage = _mesa_select_tex_image(texObj, target, level);

   texture_sub_image(ctx, dims, texObj, texImage, target, level,
                     xoffset, yoffset, zoffset, width, height, depth,
                     format, type, pixels);
}


/**
 * Implement all the glTextureSubImage1/2/3D() functions.
 * Must split this out this way because of GL_TEXTURE_CUBE_MAP.
 */
static ALWAYS_INLINE void
texturesubimage(struct gl_context *ctx, GLuint dims,
                GLuint texture, GLenum target, GLint level,
                GLint xoffset, GLint yoffset, GLint zoffset,
                GLsizei width, GLsizei height, GLsizei depth,
                GLenum format, GLenum type, const GLvoid *pixels,
                const char *callerName, bool no_error, bool ext_dsa)
{
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;
   int i;

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx,
                  "glTextureSubImage%uD %d %d %d %d %d %d %d %d %s %s %p\n",
                  dims, texture, level,
                  xoffset, yoffset, zoffset, width, height, depth,
                  _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type), pixels);

   /* Get the texture object by Name. */
   if (!no_error) {
      if (!ext_dsa) {
         texObj = _mesa_lookup_texture_err(ctx, texture, callerName);
      } else {
         texObj = lookup_texture_ext_dsa(ctx, target, texture, callerName);
      }
      if (!texObj)
         return;
   } else {
      texObj = _mesa_lookup_texture(ctx, texture);
   }

   if (!no_error) {
      /* check target (proxies not allowed) */
      if (!legal_texsubimage_target(ctx, dims, texObj->Target, true)) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(target=%s)",
                     callerName, _mesa_enum_to_string(texObj->Target));
         return;
      }

      if (texsubimage_error_check(ctx, dims, texObj, texObj->Target, level,
                                  xoffset, yoffset, zoffset,
                                  width, height, depth, format, type,
                                  pixels, callerName)) {
         return;   /* error was detected */
      }
   }

   /* Must handle special case GL_TEXTURE_CUBE_MAP. */
   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      intptr_t imageStride;

      /*
       * What do we do if the user created a texture with the following code
       * and then called this function with its handle?
       *
       *    GLuint tex;
       *    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &tex);
       *    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, ...);
       *    // Note: GL_TEXTURE_CUBE_MAP_NEGATIVE_Y not set, or given the
       *    // wrong format, or given the wrong size, etc.
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, ...);
       *    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, ...);
       *
       * A bug has been filed against the spec for this case.  In the
       * meantime, we will check for cube completeness.
       *
       * According to Section 8.17 Texture Completeness in the OpenGL 4.5
       * Core Profile spec (30.10.2014):
       *    "[A] cube map texture is cube complete if the
       *    following conditions all hold true: The [base level] texture
       *    images of each of the six cube map faces have identical, positive,
       *    and square dimensions. The [base level] images were each specified
       *    with the same internal format."
       *
       * It seems reasonable to check for cube completeness of an arbitrary
       * level here so that the image data has a consistent format and size.
       */
      if (!no_error && !_mesa_cube_level_complete(texObj, level)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glTextureSubImage%uD(cube map incomplete)",
                     dims);
         return;
      }

      imageStride = _mesa_image_image_stride(&ctx->Unpack, width, height,
                                             format, type);
      /* Copy in each face. */
      for (i = zoffset; i < zoffset + depth; ++i) {
         texImage = texObj->Image[i][level];
         assert(texImage);

         texture_sub_image(ctx, 3, texObj, texImage, texObj->Target,
                           level, xoffset, yoffset, 0,
                           width, height, 1, format,
                           type, pixels);
         pixels = (GLubyte *) pixels + imageStride;
      }
   }
   else {
      texImage = _mesa_select_tex_image(texObj, texObj->Target, level);
      assert(texImage);

      texture_sub_image(ctx, dims, texObj, texImage, texObj->Target,
                        level, xoffset, yoffset, zoffset,
                        width, height, depth, format,
                        type, pixels);
   }
}


static void
texturesubimage_error(struct gl_context *ctx, GLuint dims,
                      GLuint texture, GLenum target, GLint level,
                      GLint xoffset, GLint yoffset, GLint zoffset,
                      GLsizei width, GLsizei height, GLsizei depth,
                      GLenum format, GLenum type, const GLvoid *pixels,
                      const char *callerName, bool ext_dsa)
{
   texturesubimage(ctx, dims, texture, target, level, xoffset, yoffset,
                   zoffset, width, height, depth, format, type, pixels,
                   callerName, false, ext_dsa);
}


static void
texturesubimage_no_error(struct gl_context *ctx, GLuint dims,
                         GLuint texture, GLenum target, GLint level,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         GLsizei width, GLsizei height, GLsizei depth,
                         GLenum format, GLenum type, const GLvoid *pixels,
                         const char *callerName, bool ext_dsa)
{
   texturesubimage(ctx, dims, texture, target, level, xoffset, yoffset,
                   zoffset, width, height, depth, format, type, pixels,
                   callerName, true, ext_dsa);
}


void GLAPIENTRY
_mesa_TexSubImage1D_no_error(GLenum target, GLint level,
                             GLint xoffset, GLsizei width,
                             GLenum format, GLenum type,
                             const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 1, target, level,
               xoffset, 0, 0,
               width, 1, 1,
               format, type, pixels);
}


void GLAPIENTRY
_mesa_TexSubImage1D( GLenum target, GLint level,
                     GLint xoffset, GLsizei width,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage_err(ctx, 1, target, level,
                   xoffset, 0, 0,
                   width, 1, 1,
                   format, type, pixels, "glTexSubImage1D");
}


void GLAPIENTRY
_mesa_TexSubImage2D_no_error(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset,
                             GLsizei width, GLsizei height,
                             GLenum format, GLenum type,
                             const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 2, target, level,
               xoffset, yoffset, 0,
               width, height, 1,
               format, type, pixels);
}


void GLAPIENTRY
_mesa_TexSubImage2D( GLenum target, GLint level,
                     GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage_err(ctx, 2, target, level,
                   xoffset, yoffset, 0,
                   width, height, 1,
                   format, type, pixels, "glTexSubImage2D");
}


void GLAPIENTRY
_mesa_TexSubImage3D_no_error(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset, GLint zoffset,
                             GLsizei width, GLsizei height, GLsizei depth,
                             GLenum format, GLenum type,
                             const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage(ctx, 3, target, level,
               xoffset, yoffset, zoffset,
               width, height, depth,
               format, type, pixels);
}


void GLAPIENTRY
_mesa_TexSubImage3D( GLenum target, GLint level,
                     GLint xoffset, GLint yoffset, GLint zoffset,
                     GLsizei width, GLsizei height, GLsizei depth,
                     GLenum format, GLenum type,
                     const GLvoid *pixels )
{
   GET_CURRENT_CONTEXT(ctx);
   texsubimage_err(ctx, 3, target, level,
                   xoffset, yoffset, zoffset,
                   width, height, depth,
                   format, type, pixels, "glTexSubImage3D");
}


void GLAPIENTRY
_mesa_TextureSubImage1D_no_error(GLuint texture, GLint level, GLint xoffset,
                                 GLsizei width, GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_no_error(ctx, 1, texture, 0, level, xoffset, 0, 0, width,
                            1, 1, format, type, pixels, "glTextureSubImage1D",
                            false);
}


void GLAPIENTRY
_mesa_TextureSubImage1DEXT(GLuint texture, GLenum target, GLint level,
                        GLint xoffset, GLsizei width,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 1, texture, target, level, xoffset, 0, 0, width, 1,
                         1, format, type, pixels, "glTextureSubImage1DEXT",
                         false);
}


void GLAPIENTRY
_mesa_MultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level,
                            GLint xoffset, GLsizei width,
                            GLenum format, GLenum type,
                            const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexImage1DEXT");
   texImage = _mesa_select_tex_image(texObj, target, level);

   texture_sub_image(ctx, 1, texObj, texImage, target, level,
                     xoffset, 0, 0, width, 1, 1,
                     format, type, pixels);
}


void GLAPIENTRY
_mesa_TextureSubImage1D(GLuint texture, GLint level,
                        GLint xoffset, GLsizei width,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 1, texture, 0, level, xoffset, 0, 0, width, 1,
                         1, format, type, pixels, "glTextureSubImage1D",
                         false);
}


void GLAPIENTRY
_mesa_TextureSubImage2D_no_error(GLuint texture, GLint level, GLint xoffset,
                                 GLint yoffset, GLsizei width, GLsizei height,
                                 GLenum format, GLenum type,
                                 const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_no_error(ctx, 2, texture, 0, level, xoffset, yoffset, 0,
                            width, height, 1, format, type, pixels,
                            "glTextureSubImage2D", false);
}


void GLAPIENTRY
_mesa_TextureSubImage2DEXT(GLuint texture, GLenum target, GLint level,
                           GLint xoffset, GLint yoffset, GLsizei width,
                           GLsizei height, GLenum format, GLenum type,
                           const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 2, texture, target, level, xoffset, yoffset, 0,
                         width, height, 1, format, type, pixels,
                         "glTextureSubImage2DEXT", true);
}


void GLAPIENTRY
_mesa_MultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level,
                            GLint xoffset, GLint yoffset, GLsizei width,
                            GLsizei height, GLenum format, GLenum type,
                            const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexImage2DEXT");
   texImage = _mesa_select_tex_image(texObj, target, level);

   texture_sub_image(ctx, 2, texObj, texImage, target, level,
                     xoffset, yoffset, 0, width, height, 1,
                     format, type, pixels);
}


void GLAPIENTRY
_mesa_TextureSubImage2D(GLuint texture, GLint level,
                        GLint xoffset, GLint yoffset,
                        GLsizei width, GLsizei height,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 2, texture, 0, level, xoffset, yoffset, 0,
                         width, height, 1, format, type, pixels,
                         "glTextureSubImage2D", false);
}


void GLAPIENTRY
_mesa_TextureSubImage3D_no_error(GLuint texture, GLint level, GLint xoffset,
                                 GLint yoffset, GLint zoffset, GLsizei width,
                                 GLsizei height, GLsizei depth, GLenum format,
                                 GLenum type, const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_no_error(ctx, 3, texture, 0, level, xoffset, yoffset,
                            zoffset, width, height, depth, format, type,
                            pixels, "glTextureSubImage3D", false);
}


void GLAPIENTRY
_mesa_TextureSubImage3DEXT(GLuint texture, GLenum target, GLint level,
                           GLint xoffset, GLint yoffset, GLint zoffset,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLenum format, GLenum type, const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 3, texture, target, level, xoffset, yoffset,
                         zoffset, width, height, depth, format, type,
                         pixels, "glTextureSubImage3DEXT", true);
}


void GLAPIENTRY
_mesa_MultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level,
                           GLint xoffset, GLint yoffset, GLint zoffset,
                           GLsizei width, GLsizei height, GLsizei depth,
                           GLenum format, GLenum type, const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImage;

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false,
                                                   "glMultiTexImage3DEXT");
   texImage = _mesa_select_tex_image(texObj, target, level);

   texture_sub_image(ctx, 3, texObj, texImage, target, level,
                     xoffset, yoffset, zoffset, width, height, depth,
                     format, type, pixels);
}


void GLAPIENTRY
_mesa_TextureSubImage3D(GLuint texture, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLsizei width, GLsizei height, GLsizei depth,
                        GLenum format, GLenum type,
                        const GLvoid *pixels)
{
   GET_CURRENT_CONTEXT(ctx);
   texturesubimage_error(ctx, 3, texture, 0, level, xoffset, yoffset, zoffset,
                         width, height, depth, format, type, pixels,
                         "glTextureSubImage3D", false);
}


/**
 * For glCopyTexSubImage, return the source renderbuffer to copy texel data
 * from.  This depends on whether the texture contains color or depth values.
 */
static struct gl_renderbuffer *
get_copy_tex_image_source(struct gl_context *ctx, mesa_format texFormat)
{
   if (_mesa_get_format_bits(texFormat, GL_DEPTH_BITS) > 0) {
      /* reading from depth/stencil buffer */
      return ctx->ReadBuffer->Attachment[BUFFER_DEPTH].Renderbuffer;
   } else if (_mesa_get_format_bits(texFormat, GL_STENCIL_BITS) > 0) {
      return ctx->ReadBuffer->Attachment[BUFFER_STENCIL].Renderbuffer;
   } else {
      /* copying from color buffer */
      return ctx->ReadBuffer->_ColorReadBuffer;
   }
}


static void
copytexsubimage_by_slice(struct gl_context *ctx,
                         struct gl_texture_image *texImage,
                         GLuint dims,
                         GLint xoffset, GLint yoffset, GLint zoffset,
                         struct gl_renderbuffer *rb,
                         GLint x, GLint y,
                         GLsizei width, GLsizei height)
{
   if (texImage->TexObject->Target == GL_TEXTURE_1D_ARRAY) {
      int slice;

      /* For 1D arrays, we copy each scanline of the source rectangle into the
       * next array slice.
       */
      assert(zoffset == 0);

      for (slice = 0; slice < height; slice++) {
         assert(yoffset + slice < texImage->Height);
         st_CopyTexSubImage(ctx, 2, texImage,
                            xoffset, 0, yoffset + slice,
                            rb, x, y + slice, width, 1);
      }
   } else {
      st_CopyTexSubImage(ctx, dims, texImage,
                         xoffset, yoffset, zoffset,
                         rb, x, y, width, height);
   }
}


static GLboolean
formats_differ_in_component_sizes(mesa_format f1, mesa_format f2)
{
   GLint f1_r_bits = _mesa_get_format_bits(f1, GL_RED_BITS);
   GLint f1_g_bits = _mesa_get_format_bits(f1, GL_GREEN_BITS);
   GLint f1_b_bits = _mesa_get_format_bits(f1, GL_BLUE_BITS);
   GLint f1_a_bits = _mesa_get_format_bits(f1, GL_ALPHA_BITS);

   GLint f2_r_bits = _mesa_get_format_bits(f2, GL_RED_BITS);
   GLint f2_g_bits = _mesa_get_format_bits(f2, GL_GREEN_BITS);
   GLint f2_b_bits = _mesa_get_format_bits(f2, GL_BLUE_BITS);
   GLint f2_a_bits = _mesa_get_format_bits(f2, GL_ALPHA_BITS);

   if ((f1_r_bits && f2_r_bits && f1_r_bits != f2_r_bits)
       || (f1_g_bits && f2_g_bits && f1_g_bits != f2_g_bits)
       || (f1_b_bits && f2_b_bits && f1_b_bits != f2_b_bits)
       || (f1_a_bits && f2_a_bits && f1_a_bits != f2_a_bits))
      return GL_TRUE;

   return GL_FALSE;
}


/**
 * Check if the given texture format and size arguments match those
 * of the texture image.
 * \param return true if arguments match, false otherwise.
 */
static bool
can_avoid_reallocation(const struct gl_texture_image *texImage,
                       GLenum internalFormat,
                       mesa_format texFormat, GLsizei width,
                       GLsizei height, GLint border)
{
   if (texImage->InternalFormat != internalFormat)
      return false;
   if (texImage->TexFormat != texFormat)
      return false;
   if (texImage->Border != border)
      return false;
   if (texImage->Width2 != width)
      return false;
   if (texImage->Height2 != height)
      return false;
   return true;
}


/**
 * Implementation for glCopyTex(ture)SubImage1/2/3D() functions.
 */
static void
copy_texture_sub_image(struct gl_context *ctx, GLuint dims,
                       struct gl_texture_object *texObj,
                       GLenum target, GLint level,
                       GLint xoffset, GLint yoffset, GLint zoffset,
                       GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_image *texImage;

   _mesa_lock_texture(ctx, texObj);

   texImage = _mesa_select_tex_image(texObj, target, level);

   /* If we have a border, offset=-1 is legal.  Bias by border width. */
   switch (dims) {
   case 3:
      if (target != GL_TEXTURE_2D_ARRAY)
         zoffset += texImage->Border;
      FALLTHROUGH;
   case 2:
      if (target != GL_TEXTURE_1D_ARRAY)
         yoffset += texImage->Border;
      FALLTHROUGH;
   case 1:
      xoffset += texImage->Border;
   }

   if (ctx->Const.NoClippingOnCopyTex ||
       _mesa_clip_copytexsubimage(ctx, &xoffset, &yoffset, &x, &y,
                                  &width, &height)) {
      struct gl_renderbuffer *srcRb =
         get_copy_tex_image_source(ctx, texImage->TexFormat);

      copytexsubimage_by_slice(ctx, texImage, dims, xoffset, yoffset, zoffset,
                               srcRb, x, y, width, height);

      check_gen_mipmap(ctx, target, texObj, level);

      /* NOTE: Don't signal _NEW_TEXTURE_OBJECT since we've only changed
       * the texel data, not the texture format, size, etc.
       */
   }

   _mesa_unlock_texture(ctx, texObj);
}


static void
copy_texture_sub_image_err(struct gl_context *ctx, GLuint dims,
                           struct gl_texture_object *texObj,
                           GLenum target, GLint level,
                           GLint xoffset, GLint yoffset, GLint zoffset,
                           GLint x, GLint y, GLsizei width, GLsizei height,
                           const char *caller)
{
   FLUSH_VERTICES(ctx, 0, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "%s %s %d %d %d %d %d %d %d %d\n", caller,
                  _mesa_enum_to_string(target),
                  level, xoffset, yoffset, zoffset, x, y, width, height);

   _mesa_update_pixel(ctx);

   if (ctx->NewState & _NEW_BUFFERS)
      _mesa_update_state(ctx);

   if (copytexsubimage_error_check(ctx, dims, texObj, target, level,
                                   xoffset, yoffset, zoffset,
                                   width, height, caller)) {
      return;
   }

   copy_texture_sub_image(ctx, dims, texObj, target, level, xoffset, yoffset,
                          zoffset, x, y, width, height);
}


static void
copy_texture_sub_image_no_error(struct gl_context *ctx, GLuint dims,
                                struct gl_texture_object *texObj,
                                GLenum target, GLint level,
                                GLint xoffset, GLint yoffset, GLint zoffset,
                                GLint x, GLint y, GLsizei width, GLsizei height)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _mesa_update_pixel(ctx);

   if (ctx->NewState & _NEW_BUFFERS)
      _mesa_update_state(ctx);

   copy_texture_sub_image(ctx, dims, texObj, target, level, xoffset, yoffset,
                          zoffset, x, y, width, height);
}


/**
 * Implement the glCopyTexImage1/2D() functions.
 */
static ALWAYS_INLINE void
copyteximage(struct gl_context *ctx, GLuint dims, struct gl_texture_object *texObj,
             GLenum target, GLint level, GLenum internalFormat,
             GLint x, GLint y, GLsizei width, GLsizei height, GLint border,
             bool no_error)
{
   struct gl_texture_image *texImage;
   mesa_format texFormat;

   FLUSH_VERTICES(ctx, 0, 0);

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE))
      _mesa_debug(ctx, "glCopyTexImage%uD %s %d %s %d %d %d %d %d\n",
                  dims,
                  _mesa_enum_to_string(target), level,
                  _mesa_enum_to_string(internalFormat),
                  x, y, width, height, border);

   _mesa_update_pixel(ctx);

   if (ctx->NewState & _NEW_BUFFERS)
      _mesa_update_state(ctx);

   /* check target */
   if (!no_error && !legal_texsubimage_target(ctx, dims, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "glCopyTexImage%uD(target=%s)",
                  dims, _mesa_enum_to_string(target));
      return;
   }

   if (!texObj)
      texObj = _mesa_get_current_tex_object(ctx, target);

   if (!no_error) {
      if (copytexture_error_check(ctx, dims, target, texObj, level,
                                  internalFormat, border))
         return;

      if (!_mesa_legal_texture_dimensions(ctx, target, level, width, height,
                                          1, border)) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyTexImage%uD(invalid width=%d or height=%d)",
                     dims, width, height);
         return;
      }
   }

   assert(texObj);

   texFormat = _mesa_choose_texture_format(ctx, texObj, target, level,
                                           internalFormat, GL_NONE, GL_NONE);

   /* First check if reallocating the texture buffer can be avoided.
    * Without the realloc the copy can be 20x faster.
    */
   _mesa_lock_texture(ctx, texObj);
   {
      texImage = _mesa_select_tex_image(texObj, target, level);
      if (texImage && can_avoid_reallocation(texImage, internalFormat, texFormat,
                                             width, height, border)) {
         _mesa_unlock_texture(ctx, texObj);
         if (no_error) {
            copy_texture_sub_image_no_error(ctx, dims, texObj, target, level, 0,
                                            0, 0, x, y, width, height);
         } else {
            copy_texture_sub_image_err(ctx, dims, texObj, target, level, 0, 0,
                                       0, x, y, width, height,"CopyTexImage");
         }
         return;
      }
   }
   _mesa_unlock_texture(ctx, texObj);
   _mesa_perf_debug(ctx, MESA_DEBUG_SEVERITY_LOW, "glCopyTexImage "
                    "can't avoid reallocating texture storage\n");

   if (!no_error && _mesa_is_gles3(ctx)) {
      struct gl_renderbuffer *rb =
         _mesa_get_read_renderbuffer_for_format(ctx, internalFormat);

      if (_mesa_is_enum_format_unsized(internalFormat)) {
      /* Conversion from GL_RGB10_A2 source buffer format is not allowed in
       * OpenGL ES 3.0. Khronos bug# 9807.
       */
         if (rb->InternalFormat == GL_RGB10_A2) {
               _mesa_error(ctx, GL_INVALID_OPERATION,
                           "glCopyTexImage%uD(Reading from GL_RGB10_A2 buffer"
                           " and writing to unsized internal format)", dims);
               return;
         }
      }
      else {
         /* From Page 139 of OpenGL ES 3.0 spec:
         *    "If internalformat is sized, the internal format of the new texel
         *    array is internalformat, and this is also the new texel array’s
         *    effective internal format. If the component sizes of internalformat
         *    do not exactly match the corresponding component sizes of the source
         *    buffer’s effective internal format, described below, an
         *    INVALID_OPERATION error is generated. If internalformat is unsized,
         *    the internal format of the new texel array is the effective internal
         *    format of the source buffer, and this is also the new texel array’s
         *    effective internal format.
         */
         enum pipe_format rb_format = st_choose_format(ctx->st, rb->InternalFormat,
                                                       GL_NONE, GL_NONE,
                                                       PIPE_TEXTURE_2D, 0, 0, 0,
                                                       false, false);
         enum pipe_format new_format = st_choose_format(ctx->st, internalFormat,
                                                        GL_NONE, GL_NONE,
                                                        PIPE_TEXTURE_2D, 0, 0, 0,
                                                        false, false);
         /* this comparison must be done on the API format, not the driver format */
         if (formats_differ_in_component_sizes (new_format, rb_format)) {
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "glCopyTexImage%uD(component size changed in"
                        " internal format)", dims);
            return;
         }
      }
   }

   assert(texFormat != MESA_FORMAT_NONE);

   if (!st_TestProxyTexImage(ctx, proxy_target(target),
                             0, level, texFormat, 1,
                             width, height, 1)) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY,
                  "glCopyTexImage%uD(image too large)", dims);
      return;
   }

   if (border) {
      x += border;
      width -= border * 2;
      if (dims == 2) {
         y += border;
         height -= border * 2;
      }
      border = 0;
   }

   _mesa_lock_texture(ctx, texObj);
   {
      texObj->External = GL_FALSE;
      texImage = _mesa_get_tex_image(ctx, texObj, target, level);

      if (!texImage) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "glCopyTexImage%uD", dims);
      }
      else {
         GLint srcX = x, srcY = y, dstX = 0, dstY = 0, dstZ = 0;
         const GLuint face = _mesa_tex_target_to_face(target);

         /* Free old texture image */
         st_FreeTextureImageBuffer(ctx, texImage);

         _mesa_init_teximage_fields(ctx, texImage, width, height, 1,
                                    border, internalFormat, texFormat);

         if (width && height) {
            /* Allocate texture memory (no pixel data yet) */
            st_AllocTextureImageBuffer(ctx, texImage);

            if (ctx->Const.NoClippingOnCopyTex ||
                _mesa_clip_copytexsubimage(ctx, &dstX, &dstY, &srcX, &srcY,
                                           &width, &height)) {
               struct gl_renderbuffer *srcRb =
                  get_copy_tex_image_source(ctx, texImage->TexFormat);

               copytexsubimage_by_slice(ctx, texImage, dims,
                                        dstX, dstY, dstZ,
                                        srcRb, srcX, srcY, width, height);
            }

            check_gen_mipmap(ctx, target, texObj, level);
         }

         _mesa_update_fbo_texture(ctx, texObj, face, level);

         _mesa_dirty_texobj(ctx, texObj);
         _mesa_update_texture_object_swizzle(ctx, texObj);
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}


static void
copyteximage_err(struct gl_context *ctx, GLuint dims,
                 GLenum target,
                 GLint level, GLenum internalFormat, GLint x, GLint y,
                 GLsizei width, GLsizei height, GLint border)
{
   copyteximage(ctx, dims, NULL, target, level, internalFormat, x, y, width, height,
                border, false);
}


static void
copyteximage_no_error(struct gl_context *ctx, GLuint dims, GLenum target,
                      GLint level, GLenum internalFormat, GLint x, GLint y,
                      GLsizei width, GLsizei height, GLint border)
{
   copyteximage(ctx, dims, NULL, target, level, internalFormat, x, y, width, height,
                border, true);
}


void GLAPIENTRY
_mesa_CopyTexImage1D( GLenum target, GLint level,
                      GLenum internalFormat,
                      GLint x, GLint y,
                      GLsizei width, GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage_err(ctx, 1, target, level, internalFormat, x, y, width, 1,
                    border);
}


void GLAPIENTRY
_mesa_CopyTextureImage1DEXT( GLuint texture, GLenum target, GLint level,
                             GLenum internalFormat,
                             GLint x, GLint y,
                             GLsizei width, GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object* texObj =
      _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                     "glCopyTextureImage1DEXT");
   if (!texObj)
      return;
   copyteximage(ctx, 1, texObj, target, level, internalFormat, x, y, width, 1,
                border, false);
}


void GLAPIENTRY
_mesa_CopyMultiTexImage1DEXT( GLenum texunit, GLenum target, GLint level,
                              GLenum internalFormat,
                              GLint x, GLint y,
                              GLsizei width, GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object* texObj =
      _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                             texunit - GL_TEXTURE0,
                                             false,
                                             "glCopyMultiTexImage1DEXT");
   if (!texObj)
      return;
   copyteximage(ctx, 1, texObj, target, level, internalFormat, x, y, width, 1,
                border, false);
}


void GLAPIENTRY
_mesa_CopyTexImage2D( GLenum target, GLint level, GLenum internalFormat,
                      GLint x, GLint y, GLsizei width, GLsizei height,
                      GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage_err(ctx, 2, target, level, internalFormat,
                    x, y, width, height, border);
}


void GLAPIENTRY
_mesa_CopyTextureImage2DEXT( GLuint texture, GLenum target, GLint level,
                             GLenum internalFormat,
                             GLint x, GLint y,
                             GLsizei width, GLsizei height,
                             GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object* texObj =
      _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                     "glCopyTextureImage2DEXT");
   if (!texObj)
      return;
   copyteximage(ctx, 2, texObj, target, level, internalFormat, x, y, width, height,
                border, false);
}


void GLAPIENTRY
_mesa_CopyMultiTexImage2DEXT( GLenum texunit, GLenum target, GLint level,
                              GLenum internalFormat,
                              GLint x, GLint y,
                              GLsizei width, GLsizei height, GLint border )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object* texObj =
      _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                             texunit - GL_TEXTURE0,
                                             false,
                                             "glCopyMultiTexImage2DEXT");
   if (!texObj)
      return;
   copyteximage(ctx, 2, texObj, target, level, internalFormat, x, y, width, height,
                border, false);
}


void GLAPIENTRY
_mesa_CopyTexImage1D_no_error(GLenum target, GLint level, GLenum internalFormat,
                              GLint x, GLint y, GLsizei width, GLint border)
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage_no_error(ctx, 1, target, level, internalFormat, x, y, width, 1,
                         border);
}


void GLAPIENTRY
_mesa_CopyTexImage2D_no_error(GLenum target, GLint level, GLenum internalFormat,
                              GLint x, GLint y, GLsizei width, GLsizei height,
                              GLint border)
{
   GET_CURRENT_CONTEXT(ctx);
   copyteximage_no_error(ctx, 2, target, level, internalFormat,
                         x, y, width, height, border);
}


void GLAPIENTRY
_mesa_CopyTexSubImage1D(GLenum target, GLint level,
                        GLint xoffset, GLint x, GLint y, GLsizei width)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage1D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 1, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   copy_texture_sub_image_err(ctx, 1, texObj, target, level, xoffset, 0, 0,
                              x, y, width, 1, self);
}


void GLAPIENTRY
_mesa_CopyTexSubImage2D(GLenum target, GLint level,
                        GLint xoffset, GLint yoffset,
                        GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage2D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 2, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   copy_texture_sub_image_err(ctx, 2, texObj, target, level, xoffset, yoffset,
                              0, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyTexSubImage3D(GLenum target, GLint level,
                        GLint xoffset, GLint yoffset, GLint zoffset,
                        GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTexSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   /* Check target (proxies not allowed). Target must be checked prior to
    * calling _mesa_get_current_tex_object.
    */
   if (!legal_texsubimage_target(ctx, 3, target, false)) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(target));
      return;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   copy_texture_sub_image_err(ctx, 3, texObj, target, level, xoffset, yoffset,
                              zoffset, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage1D(GLuint texture, GLint level,
                            GLint xoffset, GLint x, GLint y, GLsizei width)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage1D";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 1, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   copy_texture_sub_image_err(ctx, 1, texObj, texObj->Target, level, xoffset, 0,
                              0, x, y, width, 1, self);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage1DEXT(GLuint texture, GLenum target, GLint level,
                               GLint xoffset, GLint x, GLint y, GLsizei width)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage1DEXT";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 1, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   copy_texture_sub_image_err(ctx, 1, texObj, texObj->Target, level, xoffset, 0,
                              0, x, y, width, 1, self);
}


void GLAPIENTRY
_mesa_CopyMultiTexSubImage1DEXT(GLenum texunit, GLenum target, GLint level,
                                GLint xoffset, GLint x, GLint y, GLsizei width)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyMultiTexSubImage1DEXT";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false, self);
   if (!texObj)
      return;

   copy_texture_sub_image_err(ctx, 1, texObj, texObj->Target, level, xoffset, 0,
                              0, x, y, width, 1, self);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage2D(GLuint texture, GLint level,
                            GLint xoffset, GLint yoffset,
                            GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage2D";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 2, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   copy_texture_sub_image_err(ctx, 2, texObj, texObj->Target, level, xoffset,
                              yoffset, 0, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage2DEXT(GLuint texture, GLenum target, GLint level,
                               GLint xoffset, GLint yoffset,
                               GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage2DEXT";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 2, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   copy_texture_sub_image_err(ctx, 2, texObj, texObj->Target, level, xoffset,
                              yoffset, 0, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyMultiTexSubImage2DEXT(GLenum texunit, GLenum target, GLint level,
                               GLint xoffset, GLint yoffset,
                               GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyMultiTexSubImage2DEXT";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false, self);
   if (!texObj)
      return;

   copy_texture_sub_image_err(ctx, 2, texObj, texObj->Target, level, xoffset,
                              yoffset, 0, x, y, width, height, self);
}

void GLAPIENTRY
_mesa_CopyTextureSubImage3D(GLuint texture, GLint level,
                            GLint xoffset, GLint yoffset, GLint zoffset,
                            GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 3, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      /* Act like CopyTexSubImage2D */
      copy_texture_sub_image_err(ctx, 2, texObj,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset,
                                level, xoffset, yoffset, 0, x, y, width, height,
                                self);
   }
   else
      copy_texture_sub_image_err(ctx, 3, texObj, texObj->Target, level, xoffset,
                                 yoffset, zoffset, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage3DEXT(GLuint texture, GLenum target, GLint level,
                               GLint xoffset, GLint yoffset, GLint zoffset,
                               GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyTextureSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true, self);
   if (!texObj)
      return;

   /* Check target (proxies not allowed). */
   if (!legal_texsubimage_target(ctx, 3, texObj->Target, true)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", self,
                  _mesa_enum_to_string(texObj->Target));
      return;
   }

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      /* Act like CopyTexSubImage2D */
      copy_texture_sub_image_err(ctx, 2, texObj,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset,
                                level, xoffset, yoffset, 0, x, y, width, height,
                                self);
   }
   else
      copy_texture_sub_image_err(ctx, 3, texObj, texObj->Target, level, xoffset,
                                 yoffset, zoffset, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyMultiTexSubImage3DEXT(GLenum texunit, GLenum target, GLint level,
                                GLint xoffset, GLint yoffset, GLint zoffset,
                                GLint x, GLint y, GLsizei width, GLsizei height)
{
   struct gl_texture_object* texObj;
   const char *self = "glCopyMultiTexSubImage3D";
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   false, self);
   if (!texObj)
      return;

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      /* Act like CopyTexSubImage2D */
      copy_texture_sub_image_err(ctx, 2, texObj,
                                GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset,
                                level, xoffset, yoffset, 0, x, y, width, height,
                                self);
   }
   else
      copy_texture_sub_image_err(ctx, 3, texObj, texObj->Target, level, xoffset,
                                 yoffset, zoffset, x, y, width, height, self);
}


void GLAPIENTRY
_mesa_CopyTexSubImage1D_no_error(GLenum target, GLint level, GLint xoffset,
                                 GLint x, GLint y, GLsizei width)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_get_current_tex_object(ctx, target);
   copy_texture_sub_image_no_error(ctx, 1, texObj, target, level, xoffset, 0, 0,
                                   x, y, width, 1);
}


void GLAPIENTRY
_mesa_CopyTexSubImage2D_no_error(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLint x, GLint y, GLsizei width,
                                 GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_get_current_tex_object(ctx, target);
   copy_texture_sub_image_no_error(ctx, 2, texObj, target, level, xoffset,
                                   yoffset, 0, x, y, width, height);
}


void GLAPIENTRY
_mesa_CopyTexSubImage3D_no_error(GLenum target, GLint level, GLint xoffset,
                                 GLint yoffset, GLint zoffset, GLint x, GLint y,
                                 GLsizei width, GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_get_current_tex_object(ctx, target);
   copy_texture_sub_image_no_error(ctx, 3, texObj, target, level, xoffset,
                                   yoffset, zoffset, x, y, width, height);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage1D_no_error(GLuint texture, GLint level, GLint xoffset,
                                     GLint x, GLint y, GLsizei width)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_lookup_texture(ctx, texture);
   copy_texture_sub_image_no_error(ctx, 1, texObj, texObj->Target, level,
                                   xoffset, 0, 0, x, y, width, 1);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage2D_no_error(GLuint texture, GLint level, GLint xoffset,
                                     GLint yoffset, GLint x, GLint y,
                                     GLsizei width, GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_lookup_texture(ctx, texture);
   copy_texture_sub_image_no_error(ctx, 2, texObj, texObj->Target, level,
                                   xoffset, yoffset, 0, x, y, width, height);
}


void GLAPIENTRY
_mesa_CopyTextureSubImage3D_no_error(GLuint texture, GLint level, GLint xoffset,
                                     GLint yoffset, GLint zoffset, GLint x,
                                     GLint y, GLsizei width, GLsizei height)
{
   GET_CURRENT_CONTEXT(ctx);

   struct gl_texture_object* texObj = _mesa_lookup_texture(ctx, texture);
   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      /* Act like CopyTexSubImage2D */
      copy_texture_sub_image_no_error(ctx, 2, texObj,
                                      GL_TEXTURE_CUBE_MAP_POSITIVE_X + zoffset,
                                      level, xoffset, yoffset, 0, x, y, width,
                                      height);
   }
   else
      copy_texture_sub_image_no_error(ctx, 3, texObj, texObj->Target, level,
                                      xoffset, yoffset, zoffset, x, y, width,
                                      height);
}


static bool
check_clear_tex_image(struct gl_context *ctx,
                      const char *function,
                      struct gl_texture_image *texImage,
                      GLenum format, GLenum type,
                      const void *data,
                      GLubyte *clearValue)
{
   struct gl_texture_object *texObj = texImage->TexObject;
   static const GLubyte zeroData[MAX_PIXEL_BYTES];
   GLenum internalFormat = texImage->InternalFormat;
   GLenum err;

   if (texObj->Target == GL_TEXTURE_BUFFER) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(buffer texture)", function);
      return false;
   }

   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(compressed texture)", function);
      return false;
   }

   err = _mesa_error_check_format_and_type(ctx, format, type);
   if (err != GL_NO_ERROR) {
      _mesa_error(ctx, err,
                  "%s(incompatible format = %s, type = %s)",
                  function,
                  _mesa_enum_to_string(format),
                  _mesa_enum_to_string(type));
      return false;
   }

   /* make sure internal format and format basically agree */
   if (!texture_formats_agree(internalFormat, format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(incompatible internalFormat = %s, format = %s)",
                  function,
                  _mesa_enum_to_string(internalFormat),
                  _mesa_enum_to_string(format));
      return false;
   }

   if (ctx->Version >= 30 || ctx->Extensions.EXT_texture_integer) {
      /* both source and dest must be integer-valued, or neither */
      if (_mesa_is_format_integer_color(texImage->TexFormat) !=
          _mesa_is_enum_format_integer(format)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "%s(integer/non-integer format mismatch)",
                     function);
         return false;
      }
   }

   if (!_mesa_texstore(ctx,
                       1, /* dims */
                       texImage->_BaseFormat,
                       texImage->TexFormat,
                       0, /* dstRowStride */
                       &clearValue,
                       1, 1, 1, /* srcWidth/Height/Depth */
                       format, type,
                       data ? data : zeroData,
                       &ctx->DefaultPacking)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid format)", function);
      return false;
   }

   return true;
}


static struct gl_texture_object *
get_tex_obj_for_clear(struct gl_context *ctx,
                      const char *function,
                      GLuint texture)
{
   struct gl_texture_object *texObj;

   texObj = _mesa_lookup_texture_err(ctx, texture, function);
   if (!texObj)
      return NULL;

   if (texObj->Target == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unbound tex)", function);
      return NULL;
   }

   return texObj;
}


/**
 * For clearing cube textures, the zoffset and depth parameters indicate
 * which cube map faces are to be cleared.  This is the one case where we
 * need to be concerned with multiple gl_texture_images.  This function
 * returns the array of texture images to clear for cube maps, or one
 * texture image otherwise.
 * \return number of texture images, 0 for error, 6 for cube, 1 otherwise.
 */
static int
get_tex_images_for_clear(struct gl_context *ctx,
                         const char *function,
                         struct gl_texture_object *texObj,
                         GLint level,
                         struct gl_texture_image **texImages)
{
   GLenum target;
   int numFaces, i;

   if (level < 0 || level >= MAX_TEXTURE_LEVELS) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid level)", function);
      return 0;
   }

   if (texObj->Target == GL_TEXTURE_CUBE_MAP) {
      target = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
      numFaces = MAX_FACES;
   }
   else {
      target = texObj->Target;
      numFaces = 1;
   }

   for (i = 0; i < numFaces; i++) {
      texImages[i] = _mesa_select_tex_image(texObj, target + i, level);
      if (texImages[i] == NULL) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid level)", function);
         return 0;
      }
   }

   return numFaces;
}


void GLAPIENTRY
_mesa_ClearTexSubImage(GLuint texture, GLint level,
                       GLint xoffset, GLint yoffset, GLint zoffset,
                       GLsizei width, GLsizei height, GLsizei depth,
                       GLenum format, GLenum type, const void *data)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImages[MAX_FACES];
   GLubyte clearValue[MAX_FACES][MAX_PIXEL_BYTES];
   int i, numImages;
   int minDepth, maxDepth;

   texObj = get_tex_obj_for_clear(ctx, "glClearTexSubImage", texture);

   if (texObj == NULL)
      return;

   _mesa_lock_texture(ctx, texObj);

   numImages = get_tex_images_for_clear(ctx, "glClearTexSubImage",
                                        texObj, level, texImages);
   if (numImages == 0)
      goto out;

   if (numImages == 1) {
      minDepth = -(int) texImages[0]->Border;
      maxDepth = texImages[0]->Depth;
   } else {
      assert(numImages == MAX_FACES);
      minDepth = 0;
      maxDepth = numImages;
   }

   if (xoffset < -(GLint) texImages[0]->Border ||
       yoffset < -(GLint) texImages[0]->Border ||
       zoffset < minDepth ||
       width < 0 ||
       height < 0 ||
       depth < 0 ||
       xoffset + width > texImages[0]->Width ||
       yoffset + height > texImages[0]->Height ||
       zoffset + depth > maxDepth) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glClearSubTexImage(invalid dimensions)");
      goto out;
   }

   if (numImages == 1) {
      if (check_clear_tex_image(ctx, "glClearTexSubImage", texImages[0],
                                format, type, data, clearValue[0])) {
         st_ClearTexSubImage(ctx,
                             texImages[0],
                             xoffset, yoffset, zoffset,
                             width, height, depth,
                             data ? clearValue[0] : NULL);
      }
   } else {
      /* loop over cube face images */
      for (i = zoffset; i < zoffset + depth; i++) {
         assert(i < MAX_FACES);
         if (!check_clear_tex_image(ctx, "glClearTexSubImage", texImages[i],
                                    format, type, data, clearValue[i]))
            goto out;
      }
      for (i = zoffset; i < zoffset + depth; i++) {
         st_ClearTexSubImage(ctx,
                             texImages[i],
                             xoffset, yoffset, 0,
                             width, height, 1,
                             data ? clearValue[i] : NULL);
      }
   }

 out:
   _mesa_unlock_texture(ctx, texObj);
}


void GLAPIENTRY
_mesa_ClearTexImage( GLuint texture, GLint level,
                     GLenum format, GLenum type, const void *data )
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_object *texObj;
   struct gl_texture_image *texImages[MAX_FACES];
   GLubyte clearValue[MAX_FACES][MAX_PIXEL_BYTES];
   int i, numImages;

   texObj = get_tex_obj_for_clear(ctx, "glClearTexImage", texture);

   if (texObj == NULL)
      return;

   _mesa_lock_texture(ctx, texObj);

   numImages = get_tex_images_for_clear(ctx, "glClearTexImage",
                                        texObj, level, texImages);

   for (i = 0; i < numImages; i++) {
      if (!check_clear_tex_image(ctx, "glClearTexImage", texImages[i], format,
                                 type, data, clearValue[i]))
         goto out;
   }

   for (i = 0; i < numImages; i++) {
      st_ClearTexSubImage(ctx, texImages[i],
                          -(GLint) texImages[i]->Border, /* xoffset */
                          -(GLint) texImages[i]->Border, /* yoffset */
                          -(GLint) texImages[i]->Border, /* zoffset */
                          texImages[i]->Width,
                          texImages[i]->Height,
                          texImages[i]->Depth,
                          data ? clearValue[i] : NULL);
   }

out:
   _mesa_unlock_texture(ctx, texObj);
}




/**********************************************************************/
/******                   Compressed Textures                    ******/
/**********************************************************************/


/**
 * Target checking for glCompressedTexSubImage[123]D().
 * \return GL_TRUE if error, GL_FALSE if no error
 * Must come before other error checking so that the texture object can
 * be correctly retrieved using _mesa_get_current_tex_object.
 */
static GLboolean
compressed_subtexture_target_check(struct gl_context *ctx, GLenum target,
                                   GLint dims, GLenum intFormat, bool dsa,
                                   const char *caller)
{
   GLboolean targetOK;
   mesa_format format;
   enum mesa_format_layout layout;

   if (dsa && target == GL_TEXTURE_RECTANGLE) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid target %s)", caller,
                  _mesa_enum_to_string(target));
      return GL_TRUE;
   }

   switch (dims) {
   case 2:
      switch (target) {
      case GL_TEXTURE_2D:
         targetOK = GL_TRUE;
         break;
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         targetOK = GL_TRUE;
         break;
      default:
         targetOK = GL_FALSE;
         break;
      }
      break;
   case 3:
      switch (target) {
      case GL_TEXTURE_CUBE_MAP:
         targetOK = dsa;
         break;
      case GL_TEXTURE_2D_ARRAY:
         targetOK = _mesa_is_gles3(ctx) ||
            (_mesa_is_desktop_gl(ctx) && ctx->Extensions.EXT_texture_array);
         break;
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         targetOK = _mesa_has_texture_cube_map_array(ctx);
         break;
      case GL_TEXTURE_3D:
         targetOK = GL_TRUE;
         /*
          * OpenGL 4.5 spec (30.10.2014) says in Section 8.7 Compressed Texture
          * Images:
          *    "An INVALID_OPERATION error is generated by
          *    CompressedTex*SubImage3D if the internal format of the texture
          *    is one of the EAC, ETC2, or RGTC formats and either border is
          *    non-zero, or the effective target for the texture is not
          *    TEXTURE_2D_ARRAY."
          *
          * NOTE: that's probably a spec error.  It should probably say
          *    "... or the effective target for the texture is not
          *    TEXTURE_2D_ARRAY, TEXTURE_CUBE_MAP, nor
          *    GL_TEXTURE_CUBE_MAP_ARRAY."
          * since those targets are 2D images and they support all compression
          * formats.
          *
          * Instead of listing all these, just list those which are allowed,
          * which is (at this time) only bptc. Otherwise we'd say s3tc (and
          * more) are valid here, which they are not, but of course not
          * mentioned by core spec.
          *
          * Also, from GL_KHR_texture_compression_astc_{hdr,ldr}:
          *
          *    "Add a second new column "3D Tex." which is empty for all non-ASTC
          *     formats. If only the LDR profile is supported by the implementation,
          *     this column is also empty for all ASTC formats. If both the LDR and HDR
          *     profiles are supported, this column is checked for all ASTC formats."
          *
          *    "An INVALID_OPERATION error is generated by CompressedTexSubImage3D if
          *     <format> is one of the formats in table 8.19 and <target> is not
          *     TEXTURE_2D_ARRAY, TEXTURE_CUBE_MAP_ARRAY, or TEXTURE_3D.
          *
          *     An INVALID_OPERATION error is generated by CompressedTexSubImage3D if
          *     <format> is TEXTURE_CUBE_MAP_ARRAY and the "Cube Map Array" column of
          *     table 8.19 is *not* checked, or if <format> is TEXTURE_3D and the "3D
          *     Tex." column of table 8.19 is *not* checked"
          *
          * And from GL_KHR_texture_compression_astc_sliced_3d:
          *
          *    "Modify the "3D Tex." column to be checked for all ASTC formats."
          */
         format = _mesa_glenum_to_compressed_format(intFormat);
         layout = _mesa_get_format_layout(format);
         switch (layout) {
         case MESA_FORMAT_LAYOUT_BPTC:
            /* valid format */
            break;
         case MESA_FORMAT_LAYOUT_S3TC:
            targetOK =
               ctx->Extensions.EXT_texture_compression_s3tc &&
               (_mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility);
            break;
         case MESA_FORMAT_LAYOUT_ASTC:
            targetOK =
               ctx->Extensions.KHR_texture_compression_astc_hdr ||
               ctx->Extensions.KHR_texture_compression_astc_sliced_3d;
            break;
         default:
            /* invalid format */
            _mesa_error(ctx, GL_INVALID_OPERATION,
                        "%s(invalid target %s for format %s)", caller,
                        _mesa_enum_to_string(target),
                        _mesa_enum_to_string(intFormat));
            return GL_TRUE;
         }
         break;
      default:
         targetOK = GL_FALSE;
      }

      break;
   default:
      assert(dims == 1);
      /* no 1D compressed textures at this time */
      targetOK = GL_FALSE;
      break;
   }

   if (!targetOK) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(invalid target %s)", caller,
                  _mesa_enum_to_string(target));
      return GL_TRUE;
   }

   return GL_FALSE;
}

/**
 * Error checking for glCompressedTexSubImage[123]D().
 * \return GL_TRUE if error, GL_FALSE if no error
 */
static GLboolean
compressed_subtexture_error_check(struct gl_context *ctx, GLint dims,
                                  const struct gl_texture_object *texObj,
                                  GLenum target, GLint level,
                                  GLint xoffset, GLint yoffset, GLint zoffset,
                                  GLsizei width, GLsizei height, GLsizei depth,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data, const char *callerName)
{
   struct gl_texture_image *texImage;
   GLint expectedSize;

   GLenum is_generic_compressed_token =
      _mesa_generic_compressed_format_to_uncompressed_format(format) !=
      format;

   /* OpenGL 4.6 and OpenGL ES 3.2 spec:
    *
    *   "An INVALID_OPERATION error is generated if format does not match the
    *    internal format of the texture image being modified, since these commands do
    *    not provide for image format conversion."
    *
    *  Desktop spec has an additional rule for GL_INVALID_ENUM:
    *
    *   "An INVALID_ENUM error is generated if format is one of the generic
    *    compressed internal formats."
    */
   /* this will catch any invalid compressed format token */
   if (!_mesa_is_compressed_format(ctx, format)) {
      GLenum error = _mesa_is_desktop_gl(ctx) && is_generic_compressed_token ?
         GL_INVALID_ENUM : GL_INVALID_OPERATION;
      _mesa_error(ctx, error, "%s(format)", callerName);
      return GL_TRUE;
   }

   if (level < 0 || level >= _mesa_max_texture_levels(ctx, target)) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(level=%d)", callerName, level);
      return GL_TRUE;
   }

   /* validate the bound PBO, if any */
   if (!_mesa_validate_pbo_source_compressed(ctx, dims, &ctx->Unpack,
                                     imageSize, data, callerName)) {
      return GL_TRUE;
   }

   /* Check for invalid pixel storage modes */
   if (!_mesa_compressed_pixel_storage_error_check(ctx, dims,
                                                   &ctx->Unpack, callerName)) {
      return GL_TRUE;
   }

   expectedSize = compressed_tex_size(width, height, depth, format);
   if (expectedSize != imageSize) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size=%d)", callerName, imageSize);
      return GL_TRUE;
   }

   texImage = _mesa_select_tex_image(texObj, target, level);
   if (!texImage) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(invalid texture level %d)",
                  callerName, level);
      return GL_TRUE;
   }

   if ((GLint) format != texImage->InternalFormat) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(format=%s)",
                  callerName, _mesa_enum_to_string(format));
      return GL_TRUE;
   }

   if (compressedteximage_only_format(format)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(format=%s cannot be updated)",
                  callerName, _mesa_enum_to_string(format));
      return GL_TRUE;
   }

   if (error_check_subtexture_negative_dimensions(ctx, dims, width, height,
                                                  depth, callerName)) {
      return GL_TRUE;
   }

   if (error_check_subtexture_dimensions(ctx, dims, texImage, xoffset, yoffset,
                                         zoffset, width, height, depth,
                                         callerName)) {
      return GL_TRUE;
   }

   return GL_FALSE;
}


void GLAPIENTRY
_mesa_CompressedTexImage1D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLint border, GLsizei imageSize,
                              const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_TRUE, 1, target, level, internalFormat,
                width, 1, 1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTextureImage1DEXT(GLuint texture, GLenum target, GLint level,
                                  GLenum internalFormat, GLsizei width,
                                  GLint border, GLsizei imageSize,
                                  const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glCompressedTextureImage1DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 1, texObj, target, level, internalFormat,
            width, 1, 1, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedMultiTexImage1DEXT(GLenum texunit, GLenum target, GLint level,
                                   GLenum internalFormat, GLsizei width,
                                   GLint border, GLsizei imageSize,
                                   const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glCompressedMultiTexImage1DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 1, texObj, target, level, internalFormat,
            width, 1, 1, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedTexImage2D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLsizei height, GLint border, GLsizei imageSize,
                              const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_TRUE, 2, target, level, internalFormat,
                width, height, 1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTextureImage2DEXT(GLuint texture, GLenum target, GLint level,
                                  GLenum internalFormat, GLsizei width,
                                  GLsizei height, GLint border, GLsizei imageSize,
                                  const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glCompressedTextureImage2DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 2, texObj, target, level, internalFormat,
            width, height, 1, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedMultiTexImage2DEXT(GLenum texunit, GLenum target, GLint level,
                                   GLenum internalFormat, GLsizei width,
                                   GLsizei height, GLint border, GLsizei imageSize,
                                   const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glCompressedMultiTexImage2DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 2, texObj, target, level, internalFormat,
            width, height, 1, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedTexImage3D(GLenum target, GLint level,
                              GLenum internalFormat, GLsizei width,
                              GLsizei height, GLsizei depth, GLint border,
                              GLsizei imageSize, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_err(ctx, GL_TRUE, 3, target, level, internalFormat, width, height,
                depth, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTextureImage3DEXT(GLuint texture, GLenum target, GLint level,
                                  GLenum internalFormat, GLsizei width,
                                  GLsizei height, GLsizei depth, GLint border,
                                  GLsizei imageSize, const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glCompressedTextureImage3DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 3, texObj, target, level, internalFormat,
            width, height, depth, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedMultiTexImage3DEXT(GLenum texunit, GLenum target, GLint level,
                                   GLenum internalFormat, GLsizei width,
                                   GLsizei height, GLsizei depth, GLint border,
                                   GLsizei imageSize, const GLvoid *pixels)
{
   struct gl_texture_object*  texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glCompressedMultiTexImage3DEXT");
   if (!texObj)
      return;
   teximage(ctx, GL_TRUE, 3, texObj, target, level, internalFormat,
            width, height, depth, border, GL_NONE, GL_NONE, imageSize, pixels, false);
}


void GLAPIENTRY
_mesa_CompressedTexImage1D_no_error(GLenum target, GLint level,
                                    GLenum internalFormat, GLsizei width,
                                    GLint border, GLsizei imageSize,
                                    const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_TRUE, 1, target, level, internalFormat, width, 1,
                     1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTexImage2D_no_error(GLenum target, GLint level,
                                    GLenum internalFormat, GLsizei width,
                                    GLsizei height, GLint border,
                                    GLsizei imageSize, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_TRUE, 2, target, level, internalFormat, width,
                     height, 1, border, GL_NONE, GL_NONE, imageSize, data);
}


void GLAPIENTRY
_mesa_CompressedTexImage3D_no_error(GLenum target, GLint level,
                                    GLenum internalFormat, GLsizei width,
                                    GLsizei height, GLsizei depth, GLint border,
                                    GLsizei imageSize, const GLvoid *data)
{
   GET_CURRENT_CONTEXT(ctx);
   teximage_no_error(ctx, GL_TRUE, 3, target, level, internalFormat, width,
                     height, depth, border, GL_NONE, GL_NONE, imageSize, data);
}


/**
 * Common helper for glCompressedTexSubImage1/2/3D() and
 * glCompressedTextureSubImage1/2/3D().
 */
static void
compressed_texture_sub_image(struct gl_context *ctx, GLuint dims,
                             struct gl_texture_object *texObj,
                             struct gl_texture_image *texImage,
                             GLenum target, GLint level, GLint xoffset,
                             GLint yoffset, GLint zoffset, GLsizei width,
                             GLsizei height, GLsizei depth, GLenum format,
                             GLsizei imageSize, const GLvoid *data)
{
   FLUSH_VERTICES(ctx, 0, 0);

   _mesa_lock_texture(ctx, texObj);
   {
      if (width > 0 && height > 0 && depth > 0) {
         st_CompressedTexSubImage(ctx, dims, texImage,
                                  xoffset, yoffset, zoffset,
                                  width, height, depth,
                                  format, imageSize, data);

         check_gen_mipmap(ctx, target, texObj, level);

         /* NOTE: Don't signal _NEW_TEXTURE_OBJECT since we've only changed
          * the texel data, not the texture format, size, etc.
          */
      }
   }
   _mesa_unlock_texture(ctx, texObj);
}


enum tex_mode {
   /* Use bound texture to current unit */
   TEX_MODE_CURRENT_NO_ERROR = 0,
   TEX_MODE_CURRENT_ERROR,
   /* Use the specified texture name */
   TEX_MODE_DSA_NO_ERROR,
   TEX_MODE_DSA_ERROR,
   /* Use the specified texture name + target */
   TEX_MODE_EXT_DSA_TEXTURE,
   /* Use the specified texture unit + target */
   TEX_MODE_EXT_DSA_TEXUNIT,
};


static void
compressed_tex_sub_image(unsigned dim, GLenum target, GLuint textureOrIndex,
                         GLint level, GLint xoffset, GLint yoffset,
                         GLint zoffset, GLsizei width, GLsizei height,
                         GLsizei depth, GLenum format, GLsizei imageSize,
                         const GLvoid *data, enum tex_mode mode,
                         const char *caller)
{
   struct gl_texture_object *texObj = NULL;
   struct gl_texture_image *texImage;
   bool no_error = false;
   GET_CURRENT_CONTEXT(ctx);

   switch (mode) {
      case TEX_MODE_DSA_ERROR:
         assert(target == 0);
         texObj = _mesa_lookup_texture_err(ctx, textureOrIndex, caller);
         if (texObj)
            target = texObj->Target;
         break;
      case TEX_MODE_DSA_NO_ERROR:
         assert(target == 0);
         texObj = _mesa_lookup_texture(ctx, textureOrIndex);
         if (texObj)
            target = texObj->Target;
         no_error = true;
         break;
      case TEX_MODE_EXT_DSA_TEXTURE:
         texObj = _mesa_lookup_or_create_texture(ctx, target, textureOrIndex,
                                                 false, true, caller);
         break;
      case TEX_MODE_EXT_DSA_TEXUNIT:
         texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                         textureOrIndex,
                                                         false,
                                                         caller);
         break;
      case TEX_MODE_CURRENT_NO_ERROR:
         no_error = true;
         FALLTHROUGH;
      case TEX_MODE_CURRENT_ERROR:
      default:
         assert(textureOrIndex == 0);
         break;
   }

   if (!no_error &&
       compressed_subtexture_target_check(ctx, target, dim, format,
                                          mode == TEX_MODE_DSA_ERROR,
                                          caller)) {
      return;
   }

   if (mode == TEX_MODE_CURRENT_NO_ERROR ||
       mode == TEX_MODE_CURRENT_ERROR) {
      texObj = _mesa_get_current_tex_object(ctx, target);
   }

   if (!texObj)
      return;

   if (!no_error &&
       compressed_subtexture_error_check(ctx, dim, texObj, target, level,
                                         xoffset, yoffset, zoffset, width,
                                         height, depth, format,
                                         imageSize, data, caller)) {
      return;
   }

   /* Must handle special case GL_TEXTURE_CUBE_MAP. */
   if (dim == 3 &&
       (mode == TEX_MODE_DSA_ERROR || mode == TEX_MODE_DSA_NO_ERROR) &&
       texObj->Target == GL_TEXTURE_CUBE_MAP) {
      const char *pixels = data;
      GLint image_stride;

      /* Make sure the texture object is a proper cube.
       * (See texturesubimage in teximage.c for details on why this check is
       * performed.)
       */
      if (!no_error && !_mesa_cube_level_complete(texObj, level)) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCompressedTextureSubImage3D(cube map incomplete)");
         return;
      }

      /* Copy in each face. */
      for (int i = zoffset; i < zoffset + depth; ++i) {
         texImage = texObj->Image[i][level];
         assert(texImage);

         compressed_texture_sub_image(ctx, 3, texObj, texImage,
                                      texObj->Target, level, xoffset, yoffset,
                                      0, width, height, 1, format,
                                      imageSize, pixels);

         /* Compressed images don't have a client format */
         image_stride = _mesa_format_image_size(texImage->TexFormat,
                                                texImage->Width,
                                                texImage->Height, 1);

         pixels += image_stride;
         imageSize -= image_stride;
      }
   } else {
      texImage = _mesa_select_tex_image(texObj, target, level);
      assert(texImage);

      compressed_texture_sub_image(ctx, dim, texObj, texImage, target, level,
                                   xoffset, yoffset, zoffset, width, height,
                                   depth, format, imageSize, data);
   }
}


void GLAPIENTRY
_mesa_CompressedTexSubImage1D_no_error(GLenum target, GLint level,
                                       GLint xoffset, GLsizei width,
                                       GLenum format, GLsizei imageSize,
                                       const GLvoid *data)
{
   compressed_tex_sub_image(1, target, 0,
                            level, xoffset, 0, 0, width,
                            1, 1, format, imageSize, data,
                            TEX_MODE_CURRENT_NO_ERROR,
                            "glCompressedTexSubImage1D");
}


void GLAPIENTRY
_mesa_CompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset,
                              GLsizei width, GLenum format,
                              GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(1, target, 0,
                            level, xoffset, 0, 0, width,
                            1, 1, format, imageSize, data,
                            TEX_MODE_CURRENT_ERROR,
                            "glCompressedTexSubImage1D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage1D_no_error(GLuint texture, GLint level,
                                           GLint xoffset, GLsizei width,
                                           GLenum format, GLsizei imageSize,
                                           const GLvoid *data)
{
   compressed_tex_sub_image(1, 0, texture,
                            level, xoffset, 0, 0,
                            width, 1, 1, format, imageSize, data,
                            TEX_MODE_DSA_NO_ERROR,
                            "glCompressedTextureSubImage1D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage1D(GLuint texture, GLint level, GLint xoffset,
                                  GLsizei width, GLenum format,
                                  GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(1, 0, texture,
                            level, xoffset, 0, 0,
                            width, 1, 1, format, imageSize, data,
                            TEX_MODE_DSA_ERROR,
                            "glCompressedTextureSubImage1D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage1DEXT(GLuint texture, GLenum target,
                                     GLint level, GLint xoffset,
                                     GLsizei width, GLenum format,
                                     GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(1, target, texture, level, xoffset, 0,
                            0, width, 1, 1, format, imageSize,
                            data,
                            TEX_MODE_EXT_DSA_TEXTURE,
                            "glCompressedTextureSubImage1DEXT");
}


void GLAPIENTRY
_mesa_CompressedMultiTexSubImage1DEXT(GLenum texunit, GLenum target,
                                      GLint level, GLint xoffset,
                                      GLsizei width, GLenum format,
                                      GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(1, target, texunit - GL_TEXTURE0, level,
                            xoffset, 0, 0, width, 1, 1, format, imageSize,
                            data,
                            TEX_MODE_EXT_DSA_TEXUNIT,
                            "glCompressedMultiTexSubImage1DEXT");
}


void GLAPIENTRY
_mesa_CompressedTexSubImage2D_no_error(GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLsizei imageSize,
                                       const GLvoid *data)
{
   compressed_tex_sub_image(2, target, 0, level,
                            xoffset, yoffset, 0,
                            width, height, 1, format, imageSize, data,
                            TEX_MODE_CURRENT_NO_ERROR,
                            "glCompressedTexSubImage2D");
}


void GLAPIENTRY
_mesa_CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLsizei width, GLsizei height,
                              GLenum format, GLsizei imageSize,
                              const GLvoid *data)
{
   compressed_tex_sub_image(2, target, 0, level,
                            xoffset, yoffset, 0,
                            width, height, 1, format, imageSize, data,
                            TEX_MODE_CURRENT_ERROR,
                            "glCompressedTexSubImage2D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage2DEXT(GLuint texture, GLenum target,
                                     GLint level, GLint xoffset,
                                     GLint yoffset, GLsizei width,
                                     GLsizei height, GLenum format,
                                     GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(2, target, texture, level, xoffset,
                            yoffset, 0, width, height, 1, format,
                            imageSize, data,
                            TEX_MODE_EXT_DSA_TEXTURE,
                            "glCompressedTextureSubImage2DEXT");
}


void GLAPIENTRY
_mesa_CompressedMultiTexSubImage2DEXT(GLenum texunit, GLenum target,
                                      GLint level, GLint xoffset, GLint yoffset,
                                      GLsizei width, GLsizei height, GLenum format,
                                      GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(2, target, texunit - GL_TEXTURE0, level,
                            xoffset, yoffset, 0, width, height, 1, format,
                            imageSize, data,
                            TEX_MODE_EXT_DSA_TEXUNIT,
                            "glCompressedMultiTexSubImage2DEXT");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage2D_no_error(GLuint texture, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLsizei width, GLsizei height,
                                           GLenum format, GLsizei imageSize,
                                           const GLvoid *data)
{
   compressed_tex_sub_image(2, 0, texture, level, xoffset, yoffset, 0,
                            width, height, 1, format, imageSize, data,
                            TEX_MODE_DSA_NO_ERROR,
                            "glCompressedTextureSubImage2D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage2D(GLuint texture, GLint level, GLint xoffset,
                                  GLint yoffset,
                                  GLsizei width, GLsizei height,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data)
{
   compressed_tex_sub_image(2, 0, texture, level, xoffset, yoffset, 0,
                            width, height, 1, format, imageSize, data,
                            TEX_MODE_DSA_ERROR,
                            "glCompressedTextureSubImage2D");
}

void GLAPIENTRY
_mesa_CompressedTexSubImage3D_no_error(GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLint zoffset, GLsizei width,
                                       GLsizei height, GLsizei depth,
                                       GLenum format, GLsizei imageSize,
                                       const GLvoid *data)
{
   compressed_tex_sub_image(3, target, 0, level, xoffset, yoffset,
                            zoffset, width, height, depth, format,
                            imageSize, data,
                            TEX_MODE_CURRENT_NO_ERROR,
                            "glCompressedTexSubImage3D");
}

void GLAPIENTRY
_mesa_CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset,
                              GLint yoffset, GLint zoffset, GLsizei width,
                              GLsizei height, GLsizei depth, GLenum format,
                              GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(3, target, 0, level, xoffset, yoffset,
                            zoffset, width, height, depth, format,
                            imageSize, data,
                            TEX_MODE_CURRENT_ERROR,
                            "glCompressedTexSubImage3D");
}

void GLAPIENTRY
_mesa_CompressedTextureSubImage3D_no_error(GLuint texture, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint zoffset, GLsizei width,
                                           GLsizei height, GLsizei depth,
                                           GLenum format, GLsizei imageSize,
                                           const GLvoid *data)
{
   compressed_tex_sub_image(3, 0, texture, level, xoffset, yoffset,
                            zoffset, width, height, depth, format,
                            imageSize, data,
                            TEX_MODE_DSA_NO_ERROR,
                            "glCompressedTextureSubImage3D");
}

void GLAPIENTRY
_mesa_CompressedTextureSubImage3D(GLuint texture, GLint level, GLint xoffset,
                                  GLint yoffset, GLint zoffset, GLsizei width,
                                  GLsizei height, GLsizei depth,
                                  GLenum format, GLsizei imageSize,
                                  const GLvoid *data)
{
   compressed_tex_sub_image(3, 0, texture, level, xoffset, yoffset,
                            zoffset, width, height, depth, format,
                            imageSize, data,
                            TEX_MODE_DSA_ERROR,
                            "glCompressedTextureSubImage3D");
}


void GLAPIENTRY
_mesa_CompressedTextureSubImage3DEXT(GLuint texture, GLenum target,
                                     GLint level, GLint xoffset,
                                     GLint yoffset, GLint zoffset,
                                     GLsizei width, GLsizei height,
                                     GLsizei depth, GLenum format,
                                     GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(3, target, texture, level, xoffset, yoffset,
                            zoffset, width, height, depth, format,
                            imageSize, data,
                            TEX_MODE_EXT_DSA_TEXTURE,
                            "glCompressedTextureSubImage3DEXT");
}


void GLAPIENTRY
_mesa_CompressedMultiTexSubImage3DEXT(GLenum texunit, GLenum target,
                                      GLint level, GLint xoffset, GLint yoffset,
                                      GLint zoffset, GLsizei width, GLsizei height,
                                      GLsizei depth, GLenum format,
                                      GLsizei imageSize, const GLvoid *data)
{
   compressed_tex_sub_image(3, target, texunit - GL_TEXTURE0, level,
                            xoffset, yoffset, zoffset, width, height, depth,
                            format, imageSize, data,
                            TEX_MODE_EXT_DSA_TEXUNIT,
                            "glCompressedMultiTexSubImage3DEXT");
}


mesa_format
_mesa_get_texbuffer_format(const struct gl_context *ctx, GLenum internalFormat)
{
   if (_mesa_is_desktop_gl_compat(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA8:
         return MESA_FORMAT_A_UNORM8;
      case GL_ALPHA16:
         return MESA_FORMAT_A_UNORM16;
      case GL_ALPHA16F_ARB:
         return MESA_FORMAT_A_FLOAT16;
      case GL_ALPHA32F_ARB:
         return MESA_FORMAT_A_FLOAT32;
      case GL_ALPHA8I_EXT:
         return MESA_FORMAT_A_SINT8;
      case GL_ALPHA16I_EXT:
         return MESA_FORMAT_A_SINT16;
      case GL_ALPHA32I_EXT:
         return MESA_FORMAT_A_SINT32;
      case GL_ALPHA8UI_EXT:
         return MESA_FORMAT_A_UINT8;
      case GL_ALPHA16UI_EXT:
         return MESA_FORMAT_A_UINT16;
      case GL_ALPHA32UI_EXT:
         return MESA_FORMAT_A_UINT32;
      case GL_LUMINANCE8:
         return MESA_FORMAT_L_UNORM8;
      case GL_LUMINANCE16:
         return MESA_FORMAT_L_UNORM16;
      case GL_LUMINANCE16F_ARB:
         return MESA_FORMAT_L_FLOAT16;
      case GL_LUMINANCE32F_ARB:
         return MESA_FORMAT_L_FLOAT32;
      case GL_LUMINANCE8I_EXT:
         return MESA_FORMAT_L_SINT8;
      case GL_LUMINANCE16I_EXT:
         return MESA_FORMAT_L_SINT16;
      case GL_LUMINANCE32I_EXT:
         return MESA_FORMAT_L_SINT32;
      case GL_LUMINANCE8UI_EXT:
         return MESA_FORMAT_L_UINT8;
      case GL_LUMINANCE16UI_EXT:
         return MESA_FORMAT_L_UINT16;
      case GL_LUMINANCE32UI_EXT:
         return MESA_FORMAT_L_UINT32;
      case GL_LUMINANCE8_ALPHA8:
         return MESA_FORMAT_LA_UNORM8;
      case GL_LUMINANCE16_ALPHA16:
         return MESA_FORMAT_LA_UNORM16;
      case GL_LUMINANCE_ALPHA16F_ARB:
         return MESA_FORMAT_LA_FLOAT16;
      case GL_LUMINANCE_ALPHA32F_ARB:
         return MESA_FORMAT_LA_FLOAT32;
      case GL_LUMINANCE_ALPHA8I_EXT:
         return MESA_FORMAT_LA_SINT8;
      case GL_LUMINANCE_ALPHA16I_EXT:
         return MESA_FORMAT_LA_SINT16;
      case GL_LUMINANCE_ALPHA32I_EXT:
         return MESA_FORMAT_LA_SINT32;
      case GL_LUMINANCE_ALPHA8UI_EXT:
         return MESA_FORMAT_LA_UINT8;
      case GL_LUMINANCE_ALPHA16UI_EXT:
         return MESA_FORMAT_LA_UINT16;
      case GL_LUMINANCE_ALPHA32UI_EXT:
         return MESA_FORMAT_LA_UINT32;
      case GL_INTENSITY8:
         return MESA_FORMAT_I_UNORM8;
      case GL_INTENSITY16:
         return MESA_FORMAT_I_UNORM16;
      case GL_INTENSITY16F_ARB:
         return MESA_FORMAT_I_FLOAT16;
      case GL_INTENSITY32F_ARB:
         return MESA_FORMAT_I_FLOAT32;
      case GL_INTENSITY8I_EXT:
         return MESA_FORMAT_I_SINT8;
      case GL_INTENSITY16I_EXT:
         return MESA_FORMAT_I_SINT16;
      case GL_INTENSITY32I_EXT:
         return MESA_FORMAT_I_SINT32;
      case GL_INTENSITY8UI_EXT:
         return MESA_FORMAT_I_UINT8;
      case GL_INTENSITY16UI_EXT:
         return MESA_FORMAT_I_UINT16;
      case GL_INTENSITY32UI_EXT:
         return MESA_FORMAT_I_UINT32;
      default:
         break;
      }
   }

   if (_mesa_has_ARB_texture_buffer_object_rgb32(ctx) ||
       _mesa_has_OES_texture_buffer(ctx)) {
      switch (internalFormat) {
      case GL_RGB32F:
         return MESA_FORMAT_RGB_FLOAT32;
      case GL_RGB32UI:
         return MESA_FORMAT_RGB_UINT32;
      case GL_RGB32I:
         return MESA_FORMAT_RGB_SINT32;
      default:
         break;
      }
   }

   switch (internalFormat) {
   case GL_RGBA8:
      return MESA_FORMAT_R8G8B8A8_UNORM;
   case GL_RGBA16:
      if (_mesa_is_gles(ctx) && !_mesa_has_EXT_texture_norm16(ctx))
         return MESA_FORMAT_NONE;
      return MESA_FORMAT_RGBA_UNORM16;
   case GL_RGBA16F_ARB:
      return MESA_FORMAT_RGBA_FLOAT16;
   case GL_RGBA32F_ARB:
      return MESA_FORMAT_RGBA_FLOAT32;
   case GL_RGBA8I_EXT:
      return MESA_FORMAT_RGBA_SINT8;
   case GL_RGBA16I_EXT:
      return MESA_FORMAT_RGBA_SINT16;
   case GL_RGBA32I_EXT:
      return MESA_FORMAT_RGBA_SINT32;
   case GL_RGBA8UI_EXT:
      return MESA_FORMAT_RGBA_UINT8;
   case GL_RGBA16UI_EXT:
      return MESA_FORMAT_RGBA_UINT16;
   case GL_RGBA32UI_EXT:
      return MESA_FORMAT_RGBA_UINT32;

   case GL_RG8:
      return MESA_FORMAT_RG_UNORM8;
   case GL_RG16:
      if (_mesa_is_gles(ctx) && !_mesa_has_EXT_texture_norm16(ctx))
         return MESA_FORMAT_NONE;
      return MESA_FORMAT_RG_UNORM16;
   case GL_RG16F:
      return MESA_FORMAT_RG_FLOAT16;
   case GL_RG32F:
      return MESA_FORMAT_RG_FLOAT32;
   case GL_RG8I:
      return MESA_FORMAT_RG_SINT8;
   case GL_RG16I:
      return MESA_FORMAT_RG_SINT16;
   case GL_RG32I:
      return MESA_FORMAT_RG_SINT32;
   case GL_RG8UI:
      return MESA_FORMAT_RG_UINT8;
   case GL_RG16UI:
      return MESA_FORMAT_RG_UINT16;
   case GL_RG32UI:
      return MESA_FORMAT_RG_UINT32;

   case GL_R8:
      return MESA_FORMAT_R_UNORM8;
   case GL_R16:
      if (_mesa_is_gles(ctx) && !_mesa_has_EXT_texture_norm16(ctx))
         return MESA_FORMAT_NONE;
      return MESA_FORMAT_R_UNORM16;
   case GL_R16F:
      return MESA_FORMAT_R_FLOAT16;
   case GL_R32F:
      return MESA_FORMAT_R_FLOAT32;
   case GL_R8I:
      return MESA_FORMAT_R_SINT8;
   case GL_R16I:
      return MESA_FORMAT_R_SINT16;
   case GL_R32I:
      return MESA_FORMAT_R_SINT32;
   case GL_R8UI:
      return MESA_FORMAT_R_UINT8;
   case GL_R16UI:
      return MESA_FORMAT_R_UINT16;
   case GL_R32UI:
      return MESA_FORMAT_R_UINT32;

   default:
      return MESA_FORMAT_NONE;
   }
}


mesa_format
_mesa_validate_texbuffer_format(const struct gl_context *ctx,
                                GLenum internalFormat)
{
   mesa_format format = _mesa_get_texbuffer_format(ctx, internalFormat);
   GLenum datatype;

   if (format == MESA_FORMAT_NONE)
      return MESA_FORMAT_NONE;

   datatype = _mesa_get_format_datatype(format);

   /* The GL_ARB_texture_buffer_object spec says:
    *
    *     "If ARB_texture_float is not supported, references to the
    *     floating-point internal formats provided by that extension should be
    *     removed, and such formats may not be passed to TexBufferARB."
    *
    * As a result, GL_HALF_FLOAT internal format depends on both
    * GL_ARB_texture_float and GL_ARB_half_float_pixel.
    */
   if ((datatype == GL_FLOAT || datatype == GL_HALF_FLOAT) &&
       !ctx->Extensions.ARB_texture_float)
      return MESA_FORMAT_NONE;

   if (!ctx->Extensions.ARB_texture_rg) {
      GLenum base_format = _mesa_get_format_base_format(format);
      if (base_format == GL_R || base_format == GL_RG)
         return MESA_FORMAT_NONE;
   }

   if (!ctx->Extensions.ARB_texture_buffer_object_rgb32) {
      GLenum base_format = _mesa_get_format_base_format(format);
      if (base_format == GL_RGB)
         return MESA_FORMAT_NONE;
   }
   return format;
}


/**
 * Do work common to glTexBuffer, glTexBufferRange, glTextureBuffer
 * and glTextureBufferRange, including some error checking.
 */
static void
texture_buffer_range(struct gl_context *ctx,
                     struct gl_texture_object *texObj,
                     GLenum internalFormat,
                     struct gl_buffer_object *bufObj,
                     GLintptr offset, GLsizeiptr size,
                     const char *caller)
{
   GLintptr oldOffset = texObj->BufferOffset;
   GLsizeiptr oldSize = texObj->BufferSize;
   mesa_format format;
   mesa_format old_format;

   /* NOTE: ARB_texture_buffer_object might not be supported in
    * the compatibility profile.
    */
   if (!_mesa_has_ARB_texture_buffer_object(ctx) &&
       !_mesa_has_OES_texture_buffer(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "%s(ARB_texture_buffer_object is not"
                  " implemented for the compatibility profile)", caller);
      return;
   }

   if (texObj->HandleAllocated) {
      /* The ARB_bindless_texture spec says:
       *
       * "The error INVALID_OPERATION is generated by TexImage*, CopyTexImage*,
       *  CompressedTexImage*, TexBuffer*, TexParameter*, as well as other
       *  functions defined in terms of these, if the texture object to be
       *  modified is referenced by one or more texture or image handles."
       */
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable texture)", caller);
      return;
   }

   format = _mesa_validate_texbuffer_format(ctx, internalFormat);
   if (format == MESA_FORMAT_NONE) {
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(internalFormat %s)",
                  caller, _mesa_enum_to_string(internalFormat));
      return;
   }

   FLUSH_VERTICES(ctx, 0, GL_TEXTURE_BIT);

   _mesa_lock_texture(ctx, texObj);
   {
      _mesa_reference_buffer_object_shared(ctx, &texObj->BufferObject, bufObj);
      texObj->BufferObjectFormat = internalFormat;
      old_format = texObj->_BufferObjectFormat;
      texObj->_BufferObjectFormat = format;
      texObj->BufferOffset = offset;
      texObj->BufferSize = size;
   }
   _mesa_unlock_texture(ctx, texObj);

   if (old_format != format) {
      st_texture_release_all_sampler_views(st_context(ctx), texObj);
   } else {
      if (offset != oldOffset) {
         st_texture_release_all_sampler_views(st_context(ctx), texObj);
      }
      if (size != oldSize) {
         st_texture_release_all_sampler_views(st_context(ctx), texObj);
      }
   }

   ctx->NewDriverState |= ST_NEW_SAMPLER_VIEWS;

   if (bufObj) {
      bufObj->UsageHistory |= USAGE_TEXTURE_BUFFER;
   }
}


/**
 * Make sure the texture buffer target is GL_TEXTURE_BUFFER.
 * Return true if it is, and return false if it is not
 * (and throw INVALID ENUM as dictated in the OpenGL 4.5
 * core spec, 02.02.2015, PDF page 245).
 */
static bool
check_texture_buffer_target(struct gl_context *ctx, GLenum target,
                            const char *caller, bool dsa)
{
   if (target != GL_TEXTURE_BUFFER_ARB) {
      _mesa_error(ctx, dsa ? GL_INVALID_OPERATION : GL_INVALID_ENUM,
                  "%s(texture target is not GL_TEXTURE_BUFFER)", caller);
      return false;
   }
   else
      return true;
}

/**
 * Check for errors related to the texture buffer range.
 * Return false if errors are found, true if none are found.
 */
static bool
check_texture_buffer_range(struct gl_context *ctx,
                           struct gl_buffer_object *bufObj,
                           GLintptr offset, GLsizeiptr size,
                           const char *caller)
{
   /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
    * Textures (PDF page 245):
    *    "An INVALID_VALUE error is generated if offset is negative, if
    *    size is less than or equal to zero, or if offset + size is greater
    *    than the value of BUFFER_SIZE for the buffer bound to target."
    */
   if (offset < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(offset=%d < 0)", caller,
                  (int) offset);
      return false;
   }

   if (size <= 0) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(size=%d <= 0)", caller,
                  (int) size);
      return false;
   }

   if (offset + size > bufObj->Size) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(offset=%d + size=%d > buffer_size=%d)", caller,
                  (int) offset, (int) size, (int) bufObj->Size);
      return false;
   }

   /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
    * Textures (PDF page 245):
    *    "An INVALID_VALUE error is generated if offset is not an integer
    *    multiple of the value of TEXTURE_BUFFER_OFFSET_ALIGNMENT."
    */
   if (offset % ctx->Const.TextureBufferOffsetAlignment) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "%s(invalid offset alignment)", caller);
      return false;
   }

   return true;
}


/** GL_ARB_texture_buffer_object */
void GLAPIENTRY
_mesa_TexBuffer(GLenum target, GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   /* Need to catch a bad target before it gets to
    * _mesa_get_current_tex_object.
    */
   if (!check_texture_buffer_target(ctx, target, "glTexBuffer", false))
      return;

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTexBuffer");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   texture_buffer_range(ctx, texObj, internalFormat, bufObj, 0,
                        buffer ? -1 : 0, "glTexBuffer");
}


/** GL_ARB_texture_buffer_range */
void GLAPIENTRY
_mesa_TexBufferRange(GLenum target, GLenum internalFormat, GLuint buffer,
                     GLintptr offset, GLsizeiptr size)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   /* Need to catch a bad target before it gets to
    * _mesa_get_current_tex_object.
    */
   if (!check_texture_buffer_target(ctx, target, "glTexBufferRange", false))
      return;

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTexBufferRange");
      if (!bufObj)
         return;

      if (!check_texture_buffer_range(ctx, bufObj, offset, size,
          "glTexBufferRange"))
         return;

   } else {
      /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
       * Textures (PDF page 254):
       *    "If buffer is zero, then any buffer object attached to the buffer
       *    texture is detached, the values offset and size are ignored and
       *    the state for offset and size for the buffer texture are reset to
       *    zero."
       */
      offset = 0;
      size = 0;
      bufObj = NULL;
   }

   texObj = _mesa_get_current_tex_object(ctx, target);
   if (!texObj)
      return;

   texture_buffer_range(ctx, texObj, internalFormat, bufObj,
                        offset, size, "glTexBufferRange");
}


/** GL_ARB_texture_buffer_range + GL_EXT_direct_state_access */
void GLAPIENTRY
_mesa_TextureBufferRangeEXT(GLuint texture, GLenum target, GLenum internalFormat,
                            GLuint buffer, GLintptr offset, GLsizeiptr size)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_or_create_texture(ctx, target, texture, false, true,
                                           "glTextureBufferRangeEXT");
   if (!texObj)
      return;

   if (!check_texture_buffer_target(ctx, target, "glTextureBufferRangeEXT", true))
      return;

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTextureBufferRangeEXT");
      if (!bufObj)
         return;

      if (!check_texture_buffer_range(ctx, bufObj, offset, size,
          "glTextureBufferRangeEXT"))
         return;

   } else {
      /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
       * Textures (PDF page 254):
       *    "If buffer is zero, then any buffer object attached to the buffer
       *    texture is detached, the values offset and size are ignored and
       *    the state for offset and size for the buffer texture are reset to
       *    zero."
       */
      offset = 0;
      size = 0;
      bufObj = NULL;
   }

   texture_buffer_range(ctx, texObj, internalFormat, bufObj,
                        offset, size, "glTextureBufferRangeEXT");
}


void GLAPIENTRY
_mesa_TextureBuffer(GLuint texture, GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTextureBuffer");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture, "glTextureBuffer");
   if (!texObj)
      return;

   if (!check_texture_buffer_target(ctx, texObj->Target, "glTextureBuffer", true))
      return;

   texture_buffer_range(ctx, texObj, internalFormat,
                        bufObj, 0, buffer ? -1 : 0, "glTextureBuffer");
}

void GLAPIENTRY
_mesa_TextureBufferEXT(GLuint texture, GLenum target,
                       GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glTextureBuffer");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_or_create_texture(ctx, target, texture,
                                           false, true,
                                           "glTextureBufferEXT");

   if (!texObj ||
       !check_texture_buffer_target(ctx, texObj->Target, "glTextureBufferEXT", true))
      return;

   texture_buffer_range(ctx, texObj, internalFormat,
                        bufObj, 0, buffer ? -1 : 0, "glTextureBufferEXT");
}

void GLAPIENTRY
_mesa_MultiTexBufferEXT(GLenum texunit, GLenum target,
                        GLenum internalFormat, GLuint buffer)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer, "glMultiTexBufferEXT");
      if (!bufObj)
         return;
   } else
      bufObj = NULL;

   /* Get the texture object */
   texObj = _mesa_get_texobj_by_target_and_texunit(ctx, target,
                                                   texunit - GL_TEXTURE0,
                                                   true,
                                                   "glMultiTexBufferEXT");

   if (!texObj ||
       !check_texture_buffer_target(ctx, texObj->Target, "glMultiTexBufferEXT", false))
      return;

   texture_buffer_range(ctx, texObj, internalFormat,
                        bufObj, 0, buffer ? -1 : 0, "glMultiTexBufferEXT");
}

void GLAPIENTRY
_mesa_TextureBufferRange(GLuint texture, GLenum internalFormat, GLuint buffer,
                         GLintptr offset, GLsizeiptr size)
{
   struct gl_texture_object *texObj;
   struct gl_buffer_object *bufObj;

   GET_CURRENT_CONTEXT(ctx);

   if (buffer) {
      bufObj = _mesa_lookup_bufferobj_err(ctx, buffer,
                                          "glTextureBufferRange");
      if (!bufObj)
         return;

      if (!check_texture_buffer_range(ctx, bufObj, offset, size,
          "glTextureBufferRange"))
         return;

   } else {
      /* OpenGL 4.5 core spec (02.02.2015) says in Section 8.9 Buffer
       * Textures (PDF page 254):
       *    "If buffer is zero, then any buffer object attached to the buffer
       *    texture is detached, the values offset and size are ignored and
       *    the state for offset and size for the buffer texture are reset to
       *    zero."
       */
      offset = 0;
      size = 0;
      bufObj = NULL;
   }

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture, "glTextureBufferRange");
   if (!texObj)
      return;

   if (!check_texture_buffer_target(ctx, texObj->Target,
       "glTextureBufferRange", true))
      return;

   texture_buffer_range(ctx, texObj, internalFormat,
                        bufObj, offset, size, "glTextureBufferRange");
}

GLboolean
_mesa_is_renderable_texture_format(const struct gl_context *ctx,
                                   GLenum internalformat)
{
   /* Everything that is allowed for renderbuffers,
    * except for a base format of GL_STENCIL_INDEX, unless supported.
    */
   GLenum baseFormat = _mesa_base_fbo_format(ctx, internalformat);
   if (ctx->Extensions.ARB_texture_stencil8)
      return baseFormat != 0;
   else
      return baseFormat != 0 && baseFormat != GL_STENCIL_INDEX;
}


/** GL_ARB_texture_multisample */
static GLboolean
check_multisample_target(GLuint dims, GLenum target, bool dsa)
{
   switch(target) {
   case GL_TEXTURE_2D_MULTISAMPLE:
      return dims == 2;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE:
      return dims == 2 && !dsa;
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return dims == 3;
   case GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY:
      return dims == 3 && !dsa;
   default:
      return GL_FALSE;
   }
}


static void
texture_image_multisample(struct gl_context *ctx, GLuint dims,
                          struct gl_texture_object *texObj,
                          struct gl_memory_object *memObj,
                          GLenum target, GLsizei samples,
                          GLint internalformat, GLsizei width,
                          GLsizei height, GLsizei depth,
                          GLboolean fixedsamplelocations,
                          GLboolean immutable, GLuint64 offset,
                          const char *func)
{
   struct gl_texture_image *texImage;
   GLboolean sizeOK, dimensionsOK, samplesOK;
   mesa_format texFormat;
   GLenum sample_count_error;
   bool dsa = strstr(func, "ture") ? true : false;

   if (MESA_VERBOSE & (VERBOSE_API|VERBOSE_TEXTURE)) {
      _mesa_debug(ctx, "%s(target=%s, samples=%d, internalformat=%s)\n", func,
                  _mesa_enum_to_string(target), samples, _mesa_enum_to_string(internalformat));
   }

   if (!((ctx->Extensions.ARB_texture_multisample
         && _mesa_is_desktop_gl(ctx))) && !_mesa_is_gles31(ctx)) {
      _mesa_error(ctx, GL_INVALID_OPERATION, "%s(unsupported)", func);
      return;
   }

   if (samples < 1) {
      _mesa_error(ctx, GL_INVALID_VALUE, "%s(samples < 1)", func);
      return;
   }

   if (!check_multisample_target(dims, target, dsa)) {
      GLenum err = dsa ? GL_INVALID_OPERATION : GL_INVALID_ENUM;
      _mesa_error(ctx, err, "%s(target=%s)", func,
                  _mesa_enum_to_string(target));
      return;
   }

   /* check that the specified internalformat is color/depth/stencil-renderable;
    * refer GL3.1 spec 4.4.4
    */

   if (immutable && !_mesa_is_legal_tex_storage_format(ctx, internalformat)) {
      _mesa_error(ctx, GL_INVALID_ENUM,
            "%s(internalformat=%s not legal for immutable-format)",
            func, _mesa_enum_to_string(internalformat));
      return;
   }

   if (!_mesa_is_renderable_texture_format(ctx, internalformat)) {
      /* Page 172 of OpenGL ES 3.1 spec says:
       *   "An INVALID_ENUM error is generated if sizedinternalformat is not
       *   color-renderable, depth-renderable, or stencil-renderable (as
       *   defined in section 9.4).
       *
       *  (Same error is also defined for desktop OpenGL for multisample
       *  teximage/texstorage functions.)
       */
      _mesa_error(ctx, GL_INVALID_ENUM, "%s(internalformat=%s)", func,
                  _mesa_enum_to_string(internalformat));
      return;
   }

   sample_count_error = _mesa_check_sample_count(ctx, target,
         internalformat, samples, samples);
   samplesOK = sample_count_error == GL_NO_ERROR;

   /* Page 254 of OpenGL 4.4 spec says:
    *   "Proxy arrays for two-dimensional multisample and two-dimensional
    *    multisample array textures are operated on in the same way when
    *    TexImage2DMultisample is called with target specified as
    *    PROXY_TEXTURE_2D_MULTISAMPLE, or TexImage3DMultisample is called
    *    with target specified as PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY.
    *    However, if samples is not supported, then no error is generated.
    */
   if (!samplesOK && !_mesa_is_proxy_texture(target)) {
      _mesa_error(ctx, sample_count_error, "%s(samples=%d)", func, samples);
      return;
   }

   if (!texObj) {
      texObj = _mesa_get_current_tex_object(ctx, target);
      if (!texObj)
         return;
   }

   if (immutable && texObj->Name == 0) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
            "%s(texture object 0)",
            func);
      return;
   }

   texImage = _mesa_get_tex_image(ctx, texObj, 0, 0);

   if (texImage == NULL) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s()", func);
      return;
   }

   texFormat = _mesa_choose_texture_format(ctx, texObj, target, 0,
         internalformat, GL_NONE, GL_NONE);
   assert(texFormat != MESA_FORMAT_NONE);

   dimensionsOK = _mesa_legal_texture_dimensions(ctx, target, 0,
         width, height, depth, 0);

   sizeOK = st_TestProxyTexImage(ctx, target, 0, 0, texFormat,
                                 samples, width, height, depth);

   if (_mesa_is_proxy_texture(target)) {
      if (samplesOK && dimensionsOK && sizeOK) {
         _mesa_init_teximage_fields_ms(ctx, texImage, width, height, depth, 0,
                                       internalformat, texFormat,
                                       samples, fixedsamplelocations);
      }
      else {
         /* clear all image fields */
         clear_teximage_fields(texImage);
      }
   }
   else {
      if (!dimensionsOK) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "%s(invalid width=%d or height=%d)", func, width, height);
         return;
      }

      if (!sizeOK) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "%s(texture too large)", func);
         return;
      }

      /* Check if texObj->Immutable is set */
      if (texObj->Immutable) {
         _mesa_error(ctx, GL_INVALID_OPERATION, "%s(immutable)", func);
         return;
      }

      if (texObj->IsSparse &&
          _mesa_sparse_texture_error_check(ctx, dims, texObj, texFormat, target, 0,
                                           width, height, depth, func))
         return; /* error was recorded */

      st_FreeTextureImageBuffer(ctx, texImage);

      _mesa_init_teximage_fields_ms(ctx, texImage, width, height, depth, 0,
                                    internalformat, texFormat,
                                    samples, fixedsamplelocations);

      if (width > 0 && height > 0 && depth > 0) {
         if (memObj) {
            if (!st_SetTextureStorageForMemoryObject(ctx, texObj,
                                                     memObj, 1, width,
                                                     height, depth,
                                                     offset, func)) {

               _mesa_init_teximage_fields(ctx, texImage, 0, 0, 0, 0,
                                          internalformat, texFormat);
            }
         } else {
            if (!st_AllocTextureStorage(ctx, texObj, 1,
                                        width, height, depth, func)) {
               /* tidy up the texture image state. strictly speaking,
                * we're allowed to just leave this in whatever state we
                * like, but being tidy is good.
                */
               _mesa_init_teximage_fields(ctx, texImage, 0, 0, 0, 0,
                                          internalformat, texFormat);
            }
         }
      }

      texObj->External = GL_FALSE;
      texObj->Immutable |= immutable;

      if (immutable) {
         _mesa_set_texture_view_state(ctx, texObj, target, 1);
      }

      _mesa_update_fbo_texture(ctx, texObj, 0, 0);
   }
   _mesa_update_texture_object_swizzle(ctx, texObj);
}


void GLAPIENTRY
_mesa_TexImage2DMultisample(GLenum target, GLsizei samples,
                            GLenum internalformat, GLsizei width,
                            GLsizei height, GLboolean fixedsamplelocations)
{
   GET_CURRENT_CONTEXT(ctx);

   texture_image_multisample(ctx, 2, NULL, NULL, target, samples,
                             internalformat, width, height, 1,
                             fixedsamplelocations, GL_FALSE, 0,
                             "glTexImage2DMultisample");
}


void GLAPIENTRY
_mesa_TexImage3DMultisample(GLenum target, GLsizei samples,
                            GLenum internalformat, GLsizei width,
                            GLsizei height, GLsizei depth,
                            GLboolean fixedsamplelocations)
{
   GET_CURRENT_CONTEXT(ctx);

   texture_image_multisample(ctx, 3, NULL, NULL, target, samples,
                             internalformat, width, height, depth,
                             fixedsamplelocations, GL_FALSE, 0,
                             "glTexImage3DMultisample");
}

static bool
valid_texstorage_ms_parameters(GLsizei width, GLsizei height, GLsizei depth,
                               unsigned dims)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!_mesa_valid_tex_storage_dim(width, height, depth)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glTexStorage%uDMultisample(width=%d,height=%d,depth=%d)",
                  dims, width, height, depth);
      return false;
   }
   return true;
}

void GLAPIENTRY
_mesa_TexStorage2DMultisample(GLenum target, GLsizei samples,
                              GLenum internalformat, GLsizei width,
                              GLsizei height, GLboolean fixedsamplelocations)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!valid_texstorage_ms_parameters(width, height, 1, 2))
      return;

   texture_image_multisample(ctx, 2, NULL, NULL, target, samples,
                             internalformat, width, height, 1,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTexStorage2DMultisample");
}

void GLAPIENTRY
_mesa_TexStorage3DMultisample(GLenum target, GLsizei samples,
                              GLenum internalformat, GLsizei width,
                              GLsizei height, GLsizei depth,
                              GLboolean fixedsamplelocations)
{
   GET_CURRENT_CONTEXT(ctx);

   if (!valid_texstorage_ms_parameters(width, height, depth, 3))
      return;

   texture_image_multisample(ctx, 3, NULL, NULL, target, samples,
                             internalformat, width, height, depth,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTexStorage3DMultisample");
}

void GLAPIENTRY
_mesa_TextureStorage2DMultisample(GLuint texture, GLsizei samples,
                                  GLenum internalformat, GLsizei width,
                                  GLsizei height,
                                  GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glTextureStorage2DMultisample");
   if (!texObj)
      return;

   if (!valid_texstorage_ms_parameters(width, height, 1, 2))
      return;

   texture_image_multisample(ctx, 2, texObj, NULL, texObj->Target,
                             samples, internalformat, width, height, 1,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTextureStorage2DMultisample");
}

void GLAPIENTRY
_mesa_TextureStorage3DMultisample(GLuint texture, GLsizei samples,
                                  GLenum internalformat, GLsizei width,
                                  GLsizei height, GLsizei depth,
                                  GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   /* Get the texture object by Name. */
   texObj = _mesa_lookup_texture_err(ctx, texture,
                                     "glTextureStorage3DMultisample");
   if (!texObj)
      return;

   if (!valid_texstorage_ms_parameters(width, height, depth, 3))
      return;

   texture_image_multisample(ctx, 3, texObj, NULL, texObj->Target, samples,
                             internalformat, width, height, depth,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTextureStorage3DMultisample");
}

void GLAPIENTRY
_mesa_TextureStorage2DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples,
                                     GLenum internalformat, GLsizei width,
                                     GLsizei height,
                                     GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = lookup_texture_ext_dsa(ctx, target, texture,
                                   "glTextureStorage2DMultisampleEXT");
   if (!texObj)
      return;

   if (!valid_texstorage_ms_parameters(width, height, 1, 2))
      return;

   texture_image_multisample(ctx, 2, texObj, NULL, texObj->Target,
                             samples, internalformat, width, height, 1,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTextureStorage2DMultisampleEXT");
}

void GLAPIENTRY
_mesa_TextureStorage3DMultisampleEXT(GLuint texture, GLenum target, GLsizei samples,
                                     GLenum internalformat, GLsizei width,
                                     GLsizei height, GLsizei depth,
                                     GLboolean fixedsamplelocations)
{
   struct gl_texture_object *texObj;
   GET_CURRENT_CONTEXT(ctx);

   texObj = lookup_texture_ext_dsa(ctx, target, texture,
                                   "glTextureStorage3DMultisampleEXT");
   if (!texObj)
      return;

   if (!valid_texstorage_ms_parameters(width, height, depth, 3))
      return;

   texture_image_multisample(ctx, 3, texObj, NULL, texObj->Target, samples,
                             internalformat, width, height, depth,
                             fixedsamplelocations, GL_TRUE, 0,
                             "glTextureStorage3DMultisampleEXT");
}

void
_mesa_texture_storage_ms_memory(struct gl_context *ctx, GLuint dims,
                                struct gl_texture_object *texObj,
                                struct gl_memory_object *memObj,
                                GLenum target, GLsizei samples,
                                GLenum internalFormat, GLsizei width,
                                GLsizei height, GLsizei depth,
                                GLboolean fixedSampleLocations,
                                GLuint64 offset, const char* func)
{
   assert(memObj);

   texture_image_multisample(ctx, dims, texObj, memObj, target, samples,
                             internalFormat, width, height, depth,
                             fixedSampleLocations, GL_TRUE, offset,
                             func);
}
