/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 2014 Intel Corporation.  All Rights Reserved.
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

#include "context.h"
#include "util/glheader.h"
#include "errors.h"
#include "enums.h"
#include "teximage.h"
#include "texobj.h"
#include "fbobject.h"
#include "textureview.h"
#include "glformats.h"
#include "api_exec_decl.h"

#include "state_tracker/st_cb_copyimage.h"

enum mesa_block_class {
   BLOCK_CLASS_128_BITS,
   BLOCK_CLASS_64_BITS
};

/**
 * Prepare the source or destination resource.  This involves error
 * checking and returning the relevant gl_texture_image or gl_renderbuffer.
 * Note that one of the resulting tex_image or renderbuffer pointers will be
 * NULL and the other will be non-null.
 *
 * \param name  the texture or renderbuffer name
 * \param target  One of GL_TEXTURE_x target or GL_RENDERBUFFER
 * \param level  mipmap level
 * \param z  src or dest Z
 * \param depth  number of slices/faces/layers to copy
 * \param tex_image  returns a pointer to a texture image
 * \param renderbuffer  returns a pointer to a renderbuffer
 * \return true if success, false if error
 */
static bool
prepare_target_err(struct gl_context *ctx, GLuint name, GLenum target,
                   int level, int z, int depth,
                   struct gl_texture_image **tex_image,
                   struct gl_renderbuffer **renderbuffer,
                   mesa_format *format,
                   GLenum *internalFormat,
                   GLuint *width,
                   GLuint *height,
                   GLuint *num_samples,
                   const char *dbg_prefix,
                   bool is_arb_version)
{
   const char *suffix = is_arb_version ? "" : "NV";

   if (name == 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sName = %d)", suffix, dbg_prefix, name);
      return false;
   }

   /*
    * INVALID_ENUM is generated
    *  * if either <srcTarget> or <dstTarget>
    *   - is not RENDERBUFFER or a valid non-proxy texture target
    *   - is TEXTURE_BUFFER, or
    *   - is one of the cubemap face selectors described in table 3.17,
    */
   switch (target) {
   case GL_RENDERBUFFER:
      /* Not a texture target, but valid */
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_RECTANGLE:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      /* These are all valid */
      break;
   case GL_TEXTURE_EXTERNAL_OES:
      /* Only exists in ES */
      if (_mesa_is_gles(ctx))
         break;
      FALLTHROUGH;
   case GL_TEXTURE_BUFFER:
   default:
      _mesa_error(ctx, GL_INVALID_ENUM,
                  "glCopyImageSubData%s(%sTarget = %s)", suffix, dbg_prefix,
                  _mesa_enum_to_string(target));
      return false;
   }

   if (target == GL_RENDERBUFFER) {
      struct gl_renderbuffer *rb = _mesa_lookup_renderbuffer(ctx, name);

      if (!rb) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyImageSubData%s(%sName = %u)", suffix, dbg_prefix, name);
         return false;
      }

      if (!rb->Name) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyImageSubData%s(%sName incomplete)", suffix, dbg_prefix);
         return false;
      }

      if (level != 0) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyImageSubData%s(%sLevel = %u)", suffix, dbg_prefix, level);
         return false;
      }

      *renderbuffer = rb;
      *format = rb->Format;
      *internalFormat = rb->InternalFormat;
      *width = rb->Width;
      *height = rb->Height;
      *num_samples = rb->NumSamples;
      *tex_image = NULL;
   } else {
      struct gl_texture_object *texObj = _mesa_lookup_texture(ctx, name);

      if (!texObj) {
         /*
          * From GL_ARB_copy_image specification:
          * "INVALID_VALUE is generated if either <srcName> or <dstName> does
          * not correspond to a valid renderbuffer or texture object according
          * to the corresponding target parameter."
          */
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyImageSubData%s(%sName = %u)", suffix, dbg_prefix, name);
         return false;
      }

      /* The ARB_copy_image specification says:
       *
       *    "INVALID_OPERATION is generated if either object is a texture and
       *     the texture is not complete (as defined in section 3.9.14)"
       *
       * The cited section says:
       *
       *    "Using the preceding definitions, a texture is complete unless any
       *     of the following conditions hold true: [...]
       *
       *     * The minification filter requires a mipmap (is neither NEAREST
       *       nor LINEAR), and the texture is not mipmap complete."
       *
       * This imposes the bizarre restriction that glCopyImageSubData requires
       * mipmap completion based on the sampler minification filter, even
       * though the call fundamentally ignores the sampler.  Additionally, it
       * doesn't work with texture units, so it can't consider any bound
       * separate sampler objects.  It appears that you're supposed to use
       * the sampler object which is built-in to the texture object.
       *
       * dEQP and the Android CTS mandate this behavior, and the Khronos
       * GL and ES working groups both affirmed that this is unfortunate but
       * correct.  See https://cvs.khronos.org/bugzilla/show_bug.cgi?id=16224.
       *
       * Integer textures with filtering cause another completeness snag:
       *
       *    "Any of:
       *     – The internal format of the texture is integer (see table 8.12).
       *     – The internal format is STENCIL_INDEX.
       *     – The internal format is DEPTH_STENCIL, and the value of
       *       DEPTH_STENCIL_TEXTURE_MODE for the texture is STENCIL_INDEX.
       *     and either the magnification filter is not NEAREST, or the
       *     minification filter is neither NEAREST nor
       *     NEAREST_MIPMAP_NEAREST."
       *
       * However, applications in the wild (such as "Total War: WARHAMMER")
       * appear to call glCopyImageSubData with integer textures and the
       * default mipmap filters of GL_LINEAR and GL_NEAREST_MIPMAP_LINEAR,
       * which would be considered incomplete, but expect this to work.  In
       * fact, until VK-GL-CTS commit fef80039ff875a51806b54d151c5f2d0c12da,
       * the GL 4.5 CTS contained three tests which did the exact same thing
       * by accident, and all conformant implementations allowed it.
       *
       * A proposal was made to amend the spec to say "is not complete (as
       * defined in section <X>, but ignoring format-based completeness
       * rules)" to allow this case.  It makes some sense, given that
       * glCopyImageSubData copies raw data without considering format.
       * While the official edits have not yet been made, the OpenGL
       * working group agreed with the idea of allowing this behavior.
       *
       * To ignore formats, we check texObj->_MipmapComplete directly
       * rather than calling _mesa_is_texture_complete().
       */
      _mesa_test_texobj_completeness(ctx, texObj);
      const bool texture_complete_aside_from_formats =
         _mesa_is_mipmap_filter(&texObj->Sampler) ? texObj->_MipmapComplete
                                                  : texObj->_BaseComplete;
      if (!texture_complete_aside_from_formats) {
         _mesa_error(ctx, GL_INVALID_OPERATION,
                     "glCopyImageSubData%s(%sName incomplete)", suffix, dbg_prefix);
         return false;
      }

      /* Note that target will not be a cube face name */
      if (texObj->Target != target) {
         /*
          * From GL_ARB_copy_image_specification:
          * "INVALID_ENUM is generated if the target does not match the type
          * of the object."
          */
         _mesa_error(ctx, GL_INVALID_ENUM,
                     "glCopyImageSubData%s(%sTarget = %s)", suffix, dbg_prefix,
                     _mesa_enum_to_string(target));
         return false;
      }

      if (level < 0 || level >= MAX_TEXTURE_LEVELS) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyImageSubData%s(%sLevel = %d)", suffix, dbg_prefix, level);
         return false;
      }

      if (target == GL_TEXTURE_CUBE_MAP) {
         int i;

         if (z < 0 || z >= MAX_FACES) {
            _mesa_error(ctx, GL_INVALID_VALUE,
                        "glCopyImageSubData(cube face (%sZ = %d)", dbg_prefix, z);
            return false;
         }

         /* make sure all the cube faces are present */
         for (i = 0; i < depth; i++) {
            if (!texObj->Image[z+i][level]) {
               /* missing cube face */
               _mesa_error(ctx, GL_INVALID_VALUE,
                           "glCopyImageSubData(missing cube face)");
               return false;
            }
         }

         *tex_image = texObj->Image[z][level];
      }
      else {
         *tex_image = _mesa_select_tex_image(texObj, target, level);
      }

      if (!*tex_image) {
         _mesa_error(ctx, GL_INVALID_VALUE,
                     "glCopyImageSubData%s(%sLevel = %u)", suffix, dbg_prefix, level);
         return false;
      }

      *renderbuffer = NULL;
      *format = (*tex_image)->TexFormat;
      *internalFormat = (*tex_image)->InternalFormat;
      *width = (*tex_image)->Width;
      *height = (*tex_image)->Height;
      *num_samples = (*tex_image)->NumSamples;
   }

   return true;
}

static void
prepare_target(struct gl_context *ctx, GLuint name, GLenum target,
               int level, int z,
               struct gl_texture_image **texImage,
               struct gl_renderbuffer **renderbuffer)
{
   if (target == GL_RENDERBUFFER) {
      struct gl_renderbuffer *rb = _mesa_lookup_renderbuffer(ctx, name);

      *renderbuffer = rb;
      *texImage = NULL;
   } else {
      struct gl_texture_object *texObj = _mesa_lookup_texture(ctx, name);

      if (target == GL_TEXTURE_CUBE_MAP) {
         *texImage = texObj->Image[z][level];
      }
      else {
         *texImage = _mesa_select_tex_image(texObj, target, level);
      }

      *renderbuffer = NULL;
   }
}

/**
 * Check that the x,y,z,width,height,region is within the texture image
 * dimensions.
 * \return true if bounds OK, false if regions is out of bounds
 */
static bool
check_region_bounds(struct gl_context *ctx,
                    GLenum target,
                    const struct gl_texture_image *tex_image,
                    const struct gl_renderbuffer *renderbuffer,
                    int x, int y, int z, int width, int height, int depth,
                    const char *dbg_prefix,
                    bool is_arb_version)
{
   int surfWidth, surfHeight, surfDepth;
   const char *suffix = is_arb_version ? "" : "NV";

   if (width < 0 || height < 0 || depth < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sWidth, %sHeight, or %sDepth is negative)",
                  suffix, dbg_prefix, dbg_prefix, dbg_prefix);
      return false;
   }

   if (x < 0 || y < 0 || z < 0) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sX, %sY, or %sZ is negative)",
                  suffix, dbg_prefix, dbg_prefix, dbg_prefix);
      return false;
   }

   /* Check X direction */
   if (target == GL_RENDERBUFFER) {
      surfWidth = renderbuffer->Width;
   }
   else {
      surfWidth = tex_image->Width;
   }

   if (x + width > surfWidth) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sX or %sWidth exceeds image bounds)",
                  suffix, dbg_prefix, dbg_prefix);
      return false;
   }

   /* Check Y direction */
   switch (target) {
   case GL_RENDERBUFFER:
      surfHeight = renderbuffer->Height;
      break;
   case GL_TEXTURE_1D:
   case GL_TEXTURE_1D_ARRAY:
      surfHeight = 1;
      break;
   default:
      surfHeight = tex_image->Height;
   }

   if (y + height > surfHeight) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sY or %sHeight exceeds image bounds)",
                  suffix, dbg_prefix, dbg_prefix);
      return false;
   }

   /* Check Z direction */
   switch (target) {
   case GL_RENDERBUFFER:
   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_RECTANGLE:
      surfDepth = 1;
      break;
   case GL_TEXTURE_CUBE_MAP:
      surfDepth = 6;
      break;
   case GL_TEXTURE_1D_ARRAY:
      surfDepth = tex_image->Height;
      break;
   default:
      surfDepth = tex_image->Depth;
   }

   if (z < 0 || z + depth > surfDepth) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData%s(%sZ or %sDepth exceeds image bounds)",
                  suffix, dbg_prefix, dbg_prefix);
      return false;
   }

   return true;
}

static bool
compressed_format_compatible(const struct gl_context *ctx,
                             GLenum compressedFormat, GLenum otherFormat)
{
   enum mesa_block_class compressedClass, otherClass;

   /* Two view-incompatible compressed formats are never compatible. */
   if (_mesa_is_compressed_format(ctx, otherFormat)) {
      return false;
   }

   /*
    * From ARB_copy_image spec:
    *    Table 4.X.1 (Compatible internal formats for copying between
    *                 compressed and uncompressed internal formats)
    *    ---------------------------------------------------------------------
    *    | Texel / | Uncompressed      |                                     |
    *    | Block   | internal format   | Compressed internal format          |
    *    | size    |                   |                                     |
    *    ---------------------------------------------------------------------
    *    | 128-bit | RGBA32UI,         | COMPRESSED_RGBA_S3TC_DXT3_EXT,      |
    *    |         | RGBA32I,          | COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,|
    *    |         | RGBA32F           | COMPRESSED_RGBA_S3TC_DXT5_EXT,      |
    *    |         |                   | COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,|
    *    |         |                   | COMPRESSED_RG_RGTC2,                |
    *    |         |                   | COMPRESSED_SIGNED_RG_RGTC2,         |
    *    |         |                   | COMPRESSED_RGBA_BPTC_UNORM,         |
    *    |         |                   | COMPRESSED_SRGB_ALPHA_BPTC_UNORM,   |
    *    |         |                   | COMPRESSED_RGB_BPTC_SIGNED_FLOAT,   |
    *    |         |                   | COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT  |
    *    ---------------------------------------------------------------------
    *    | 64-bit  | RGBA16F, RG32F,   | COMPRESSED_RGB_S3TC_DXT1_EXT,       |
    *    |         | RGBA16UI, RG32UI, | COMPRESSED_SRGB_S3TC_DXT1_EXT,      |
    *    |         | RGBA16I, RG32I,   | COMPRESSED_RGBA_S3TC_DXT1_EXT,      |
    *    |         | RGBA16,           | COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,|
    *    |         | RGBA16_SNORM      | COMPRESSED_RED_RGTC1,               |
    *    |         |                   | COMPRESSED_SIGNED_RED_RGTC1         |
    *    ---------------------------------------------------------------------
    */

   switch (compressedFormat) {
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_RG_RGTC2:
      case GL_COMPRESSED_SIGNED_RG_RGTC2:
      case GL_COMPRESSED_RGBA_BPTC_UNORM:
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
         compressedClass = BLOCK_CLASS_128_BITS;
         break;
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RED_RGTC1:
      case GL_COMPRESSED_SIGNED_RED_RGTC1:
         compressedClass = BLOCK_CLASS_64_BITS;
         break;
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      case GL_COMPRESSED_RG11_EAC:
      case GL_COMPRESSED_SIGNED_RG11_EAC:
         if (_mesa_is_gles(ctx))
            compressedClass = BLOCK_CLASS_128_BITS;
         else
            return false;
         break;
      case GL_COMPRESSED_RGB8_ETC2:
      case GL_COMPRESSED_SRGB8_ETC2:
      case GL_COMPRESSED_R11_EAC:
      case GL_COMPRESSED_SIGNED_R11_EAC:
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
         if (_mesa_is_gles(ctx))
            compressedClass = BLOCK_CLASS_64_BITS;
         else
            return false;
         break;
      default:
         if (_mesa_is_gles(ctx) && _mesa_is_astc_format(compressedFormat))
            compressedClass = BLOCK_CLASS_128_BITS;
         else
            return false;
         break;
   }

   switch (otherFormat) {
      case GL_RGBA32UI:
      case GL_RGBA32I:
      case GL_RGBA32F:
         otherClass = BLOCK_CLASS_128_BITS;
         break;
      case GL_RGBA16F:
      case GL_RG32F:
      case GL_RGBA16UI:
      case GL_RG32UI:
      case GL_RGBA16I:
      case GL_RG32I:
      case GL_RGBA16:
      case GL_RGBA16_SNORM:
         otherClass = BLOCK_CLASS_64_BITS;
         break;
      default:
         return false;
   }

   return compressedClass == otherClass;
}

static bool
copy_format_compatible(const struct gl_context *ctx,
                       GLenum srcFormat, GLenum dstFormat)
{
   /*
    * From ARB_copy_image spec:
    *    For the purposes of CopyImageSubData, two internal formats
    *    are considered compatible if any of the following conditions are
    *    met:
    *    * the formats are the same,
    *    * the formats are considered compatible according to the
    *      compatibility rules used for texture views as defined in
    *      section 3.9.X. In particular, if both internal formats are listed
    *      in the same entry of Table 3.X.2, they are considered compatible, or
    *    * one format is compressed and the other is uncompressed and
    *      Table 4.X.1 lists the two formats in the same row.
    */

   if (_mesa_texture_view_compatible_format(ctx, srcFormat, dstFormat)) {
      /* Also checks if formats are equal. */
      return true;
   } else if (_mesa_is_compressed_format(ctx, srcFormat)) {
      return compressed_format_compatible(ctx, srcFormat, dstFormat);
   } else if (_mesa_is_compressed_format(ctx, dstFormat)) {
      return compressed_format_compatible(ctx, dstFormat, srcFormat);
   }

   return false;
}

static void
copy_image_subdata(struct gl_context *ctx,
                   struct gl_texture_image *srcTexImage,
                   struct gl_renderbuffer *srcRenderbuffer,
                   int srcX, int srcY, int srcZ, int srcLevel,
                   struct gl_texture_image *dstTexImage,
                   struct gl_renderbuffer *dstRenderbuffer,
                   int dstX, int dstY, int dstZ, int dstLevel,
                   int srcWidth, int srcHeight, int srcDepth)
{
   /* loop over 2D slices/faces/layers */
   for (int i = 0; i < srcDepth; ++i) {
      int newSrcZ = srcZ + i;
      int newDstZ = dstZ + i;

      if (srcTexImage &&
          srcTexImage->TexObject->Target == GL_TEXTURE_CUBE_MAP) {
         /* need to update srcTexImage pointer for the cube face */
         assert(srcZ + i < MAX_FACES);
         srcTexImage = srcTexImage->TexObject->Image[srcZ + i][srcLevel];
         assert(srcTexImage);
         newSrcZ = 0;
      }

      if (dstTexImage &&
          dstTexImage->TexObject->Target == GL_TEXTURE_CUBE_MAP) {
         /* need to update dstTexImage pointer for the cube face */
         assert(dstZ + i < MAX_FACES);
         dstTexImage = dstTexImage->TexObject->Image[dstZ + i][dstLevel];
         assert(dstTexImage);
         newDstZ = 0;
      }

      st_CopyImageSubData(ctx,
                          srcTexImage, srcRenderbuffer,
                          srcX, srcY, newSrcZ,
                          dstTexImage, dstRenderbuffer,
                          dstX, dstY, newDstZ,
                          srcWidth, srcHeight);
   }
}

void GLAPIENTRY
_mesa_CopyImageSubData_no_error(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                GLint srcX, GLint srcY, GLint srcZ,
                                GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                GLint dstX, GLint dstY, GLint dstZ,
                                GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   struct gl_texture_image *srcTexImage, *dstTexImage;
   struct gl_renderbuffer *srcRenderbuffer, *dstRenderbuffer;

   GET_CURRENT_CONTEXT(ctx);

   prepare_target(ctx, srcName, srcTarget, srcLevel, srcZ, &srcTexImage,
                  &srcRenderbuffer);

   prepare_target(ctx, dstName, dstTarget, dstLevel, dstZ, &dstTexImage,
                  &dstRenderbuffer);

   copy_image_subdata(ctx, srcTexImage, srcRenderbuffer, srcX, srcY, srcZ,
                      srcLevel, dstTexImage, dstRenderbuffer, dstX, dstY, dstZ,
                      dstLevel, srcWidth, srcHeight, srcDepth);
}

void GLAPIENTRY
_mesa_CopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                       GLint srcX, GLint srcY, GLint srcZ,
                       GLuint dstName, GLenum dstTarget, GLint dstLevel,
                       GLint dstX, GLint dstY, GLint dstZ,
                       GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_image *srcTexImage, *dstTexImage;
   struct gl_renderbuffer *srcRenderbuffer, *dstRenderbuffer;
   mesa_format srcFormat, dstFormat;
   GLenum srcIntFormat, dstIntFormat;
   GLuint src_w, src_h, dst_w, dst_h;
   GLuint src_bw, src_bh, dst_bw, dst_bh;
   GLuint src_num_samples, dst_num_samples;
   int dstWidth, dstHeight, dstDepth;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glCopyImageSubData(%u, %s, %d, %d, %d, %d, "
                                          "%u, %s, %d, %d, %d, %d, "
                                          "%d, %d, %d)\n",
                  srcName, _mesa_enum_to_string(srcTarget), srcLevel,
                  srcX, srcY, srcZ,
                  dstName, _mesa_enum_to_string(dstTarget), dstLevel,
                  dstX, dstY, dstZ,
                  srcWidth, srcHeight, srcDepth);

   if (!ctx->Extensions.ARB_copy_image) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubData(extension not available)");
      return;
   }

   if (!prepare_target_err(ctx, srcName, srcTarget, srcLevel, srcZ, srcDepth,
                           &srcTexImage, &srcRenderbuffer, &srcFormat,
                           &srcIntFormat, &src_w, &src_h, &src_num_samples,
                           "src",true))
      return;

   if (!prepare_target_err(ctx, dstName, dstTarget, dstLevel, dstZ, srcDepth,
                           &dstTexImage, &dstRenderbuffer, &dstFormat,
                           &dstIntFormat, &dst_w, &dst_h, &dst_num_samples,
                           "dst",true))
      return;

   _mesa_get_format_block_size(srcFormat, &src_bw, &src_bh);

   /* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core Profile
    * spec says:
    *
    *    An INVALID_VALUE error is generated if the dimensions of either
    *    subregion exceeds the boundaries of the corresponding image object,
    *    or if the image format is compressed and the dimensions of the
    *    subregion fail to meet the alignment constraints of the format.
    *
    * and Section 8.7 (Compressed Texture Images) says:
    *
    *    An INVALID_OPERATION error is generated if any of the following
    *    conditions occurs:
    *
    *      * width is not a multiple of four, and width + xoffset is not
    *        equal to the value of TEXTURE_WIDTH.
    *      * height is not a multiple of four, and height + yoffset is not
    *        equal to the value of TEXTURE_HEIGHT.
    *
    * so we take that to mean that you can copy the "last" block of a
    * compressed texture image even if it's smaller than the minimum block
    * dimensions.
    */
   if ((srcX % src_bw != 0) || (srcY % src_bh != 0) ||
       (srcWidth % src_bw != 0 && (srcX + srcWidth) != src_w) ||
       (srcHeight % src_bh != 0 && (srcY + srcHeight) != src_h)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData(unaligned src rectangle)");
      return;
   }

   _mesa_get_format_block_size(dstFormat, &dst_bw, &dst_bh);
   if ((dstX % dst_bw != 0) || (dstY % dst_bh != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubData(unaligned dst rectangle)");
      return;
   }

   /* From the GL_ARB_copy_image spec:
    *
    * "The dimensions are always specified in texels, even for compressed
    * texture formats. But it should be noted that if only one of the
    * source and destination textures is compressed then the number of
    * texels touched in the compressed image will be a factor of the
    * block size larger than in the uncompressed image."
    *
    * So, if copying from compressed to uncompressed, the dest region is
    * shrunk by the src block size factor.  If copying from uncompressed
    * to compressed, the dest region is grown by the dest block size factor.
    * Note that we're passed the _source_ width, height, depth and those
    * dimensions are never changed.
    */
   dstWidth = srcWidth * dst_bw / src_bw;
   dstHeight = srcHeight * dst_bh / src_bh;
   dstDepth = srcDepth;

   if (!check_region_bounds(ctx, srcTarget, srcTexImage, srcRenderbuffer,
                            srcX, srcY, srcZ, srcWidth, srcHeight, srcDepth,
                            "src", true))
      return;

   if (!check_region_bounds(ctx, dstTarget, dstTexImage, dstRenderbuffer,
                            dstX, dstY, dstZ, dstWidth, dstHeight, dstDepth,
                            "dst", true))
      return;

   /* Section 18.3.2 (Copying Between Images) of the OpenGL 4.5 Core Profile
    * spec says:
    *
    *    An INVALID_OPERATION error is generated if either object is a texture
    *    and the texture is not complete, if the source and destination internal
    *    formats are not compatible, or if the number of samples do not match.
    */
   if (!copy_format_compatible(ctx, srcIntFormat, dstIntFormat)) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubData(internalFormat mismatch)");
      return;
   }

   if (src_num_samples != dst_num_samples) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubData(number of samples mismatch)");
      return;
   }

   copy_image_subdata(ctx, srcTexImage, srcRenderbuffer, srcX, srcY, srcZ,
                      srcLevel, dstTexImage, dstRenderbuffer, dstX, dstY, dstZ,
                      dstLevel, srcWidth, srcHeight, srcDepth);
}

void GLAPIENTRY
_mesa_CopyImageSubDataNV_no_error(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                                  GLint srcX, GLint srcY, GLint srcZ,
                                  GLuint dstName, GLenum dstTarget, GLint dstLevel,
                                  GLint dstX, GLint dstY, GLint dstZ,
                                  GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   struct gl_texture_image *srcTexImage, *dstTexImage;
   struct gl_renderbuffer *srcRenderbuffer, *dstRenderbuffer;

   GET_CURRENT_CONTEXT(ctx);

   prepare_target(ctx, srcName, srcTarget, srcLevel, srcZ, &srcTexImage,
                  &srcRenderbuffer);

   prepare_target(ctx, dstName, dstTarget, dstLevel, dstZ, &dstTexImage,
                  &dstRenderbuffer);

   copy_image_subdata(ctx, srcTexImage, srcRenderbuffer, srcX, srcY, srcZ,
                      srcLevel, dstTexImage, dstRenderbuffer, dstX, dstY, dstZ,
                      dstLevel, srcWidth, srcHeight, srcDepth);
}

void GLAPIENTRY
_mesa_CopyImageSubDataNV(GLuint srcName, GLenum srcTarget, GLint srcLevel,
                         GLint srcX, GLint srcY, GLint srcZ,
                         GLuint dstName, GLenum dstTarget, GLint dstLevel,
                         GLint dstX, GLint dstY, GLint dstZ,
                         GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)
{
   GET_CURRENT_CONTEXT(ctx);
   struct gl_texture_image *srcTexImage, *dstTexImage;
   struct gl_renderbuffer *srcRenderbuffer, *dstRenderbuffer;
   mesa_format srcFormat, dstFormat;
   GLenum srcIntFormat, dstIntFormat;
   GLuint src_w, src_h, dst_w, dst_h;
   GLuint src_bw, src_bh, dst_bw, dst_bh;
   GLuint src_num_samples, dst_num_samples;

   if (MESA_VERBOSE & VERBOSE_API)
      _mesa_debug(ctx, "glCopyImageSubDataNV(%u, %s, %d, %d, %d, %d, "
                                            "%u, %s, %d, %d, %d, %d, "
                                            "%d, %d, %d)\n",
                  srcName, _mesa_enum_to_string(srcTarget), srcLevel,
                  srcX, srcY, srcZ,
                  dstName, _mesa_enum_to_string(dstTarget), dstLevel,
                  dstX, dstY, dstZ,
                  srcWidth, srcHeight, srcDepth);

   if (!ctx->Extensions.NV_copy_image) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubDataNV(extension not available)");
      return;
   }

   if (!prepare_target_err(ctx, srcName, srcTarget, srcLevel, srcZ, srcDepth,
                           &srcTexImage, &srcRenderbuffer, &srcFormat,
                           &srcIntFormat, &src_w, &src_h, &src_num_samples,
                           "src", false))
      return;

   if (!prepare_target_err(ctx, dstName, dstTarget, dstLevel, dstZ, srcDepth,
                           &dstTexImage, &dstRenderbuffer, &dstFormat,
                           &dstIntFormat, &dst_w, &dst_h, &dst_num_samples,
                           "dst", false))
      return;

   /*
    * The NV_copy_image spec says:
    *
    *    INVALID_OPERATION is generated if either object is a texture
    *    and the texture is not consistent, or if the source and destination
    *    internal formats or number of samples do not match.
    *
    * In the absence of any definition of texture consistency the texture
    * completeness check, which is affected in the prepare_target_err function,
    * is used instead in keeping with the ARB version.
    * The check related to the internal format here is different from the ARB
    * version which adds the ability to copy between images which have
    * different formats where the formats are compatible for texture views.
    */
   if (srcIntFormat != dstIntFormat) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubDataNV(internalFormat mismatch)");
      return;
   }

   if (src_num_samples != dst_num_samples) {
      _mesa_error(ctx, GL_INVALID_OPERATION,
                  "glCopyImageSubDataNV(number of samples mismatch)");
      return;
   }

   /*
    * The NV_copy_image spec says:
    *
    *    INVALID_VALUE is generated if the image format is compressed
    *    and the dimensions of the subregion fail to meet the alignment
    *    constraints of the format.
    *
    * The check here is identical to the ARB version.
    */
   _mesa_get_format_block_size(srcFormat, &src_bw, &src_bh);
   if ((srcX % src_bw != 0) || (srcY % src_bh != 0) ||
       (srcWidth % src_bw != 0 && (srcX + srcWidth) != src_w) ||
       (srcHeight % src_bh != 0 && (srcY + srcHeight) != src_h)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubDataNV(unaligned src rectangle)");
      return;
   }

   _mesa_get_format_block_size(dstFormat, &dst_bw, &dst_bh);
   if ((dstX % dst_bw != 0) || (dstY % dst_bh != 0)) {
      _mesa_error(ctx, GL_INVALID_VALUE,
                  "glCopyImageSubDataNV(unaligned dst rectangle)");
      return;
   }

   /*
    * The NV_copy_image spec says:
    *
    *    INVALID_VALUE is generated if the dimensions of the either subregion
    *    exceeds the boundaries of the corresponding image object.
    *
    * The check here is similar to the ARB version except for the fact that
    * block sizes are not considered owing to the fact that copying across
    * compressed and uncompressed formats is not supported.
    */
   if (!check_region_bounds(ctx, srcTarget, srcTexImage, srcRenderbuffer,
                            srcX, srcY, srcZ, srcWidth, srcHeight, srcDepth,
                            "src", false))
      return;

   if (!check_region_bounds(ctx, dstTarget, dstTexImage, dstRenderbuffer,
                            dstX, dstY, dstZ, srcWidth, srcHeight, srcDepth,
                            "dst", false))
      return;

   copy_image_subdata(ctx, srcTexImage, srcRenderbuffer, srcX, srcY, srcZ,
                      srcLevel, dstTexImage, dstRenderbuffer, dstX, dstY, dstZ,
                      dstLevel, srcWidth, srcHeight, srcDepth);
}
