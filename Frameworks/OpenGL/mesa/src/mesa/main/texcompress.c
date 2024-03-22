/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2006  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008 VMware, Inc.
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
 * \file texcompress.c
 * Helper functions for texture compression.
 */


#include "util/glheader.h"

#include "context.h"
#include "formats.h"
#include "mtypes.h"
#include "context.h"
#include "texcompress.h"
#include "texcompress_fxt1.h"
#include "texcompress_rgtc.h"
#include "texcompress_s3tc.h"
#include "texcompress_etc.h"
#include "texcompress_bptc.h"


/**
 * Get the GL base format of a specified GL compressed texture format
 *
 * From page 232 of the OpenGL 3.3 (Compatiblity Profile) spec:
 *
 *     "Compressed Internal Format      Base Internal Format    Type
 *     ---------------------------     --------------------    ---------
 *     COMPRESSED_ALPHA                ALPHA                   Generic
 *     COMPRESSED_LUMINANCE            LUMINANCE               Generic
 *     COMPRESSED_LUMINANCE_ALPHA      LUMINANCE_ALPHA         Generic
 *     COMPRESSED_INTENSITY            INTENSITY               Generic
 *     COMPRESSED_RED                  RED                     Generic
 *     COMPRESSED_RG                   RG                      Generic
 *     COMPRESSED_RGB                  RGB                     Generic
 *     COMPRESSED_RGBA                 RGBA                    Generic
 *     COMPRESSED_SRGB                 RGB                     Generic
 *     COMPRESSED_SRGB_ALPHA           RGBA                    Generic
 *     COMPRESSED_SLUMINANCE           LUMINANCE               Generic
 *     COMPRESSED_SLUMINANCE_ALPHA     LUMINANCE_ALPHA         Generic
 *     COMPRESSED_RED_RGTC1            RED                     Specific
 *     COMPRESSED_SIGNED_RED_RGTC1     RED                     Specific
 *     COMPRESSED_RG_RGTC2             RG                      Specific
 *     COMPRESSED_SIGNED_RG_RGTC2      RG                      Specific"
 *
 * \return
 * The base format of \c format if \c format is a compressed format (either
 * generic or specific.  Otherwise 0 is returned.
 */
GLenum
_mesa_gl_compressed_format_base_format(GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RED:
   case GL_COMPRESSED_R11_EAC:
   case GL_COMPRESSED_RED_RGTC1:
   case GL_COMPRESSED_SIGNED_R11_EAC:
   case GL_COMPRESSED_SIGNED_RED_RGTC1:
      return GL_RED;

   case GL_COMPRESSED_RG:
   case GL_COMPRESSED_RG11_EAC:
   case GL_COMPRESSED_RG_RGTC2:
   case GL_COMPRESSED_SIGNED_RG11_EAC:
   case GL_COMPRESSED_SIGNED_RG_RGTC2:
      return GL_RG;

   case GL_COMPRESSED_RGB:
   case GL_COMPRESSED_SRGB:
   case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB:
   case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB:
   case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGB_FXT1_3DFX:
   case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
   case GL_ETC1_RGB8_OES:
   case GL_COMPRESSED_RGB8_ETC2:
   case GL_COMPRESSED_SRGB8_ETC2:
   case GL_RGB_S3TC:
   case GL_RGB4_S3TC:
   case GL_PALETTE4_RGB8_OES:
   case GL_PALETTE4_R5_G6_B5_OES:
   case GL_PALETTE8_RGB8_OES:
   case GL_PALETTE8_R5_G6_B5_OES:
   case GL_ATC_RGB_AMD:
      return GL_RGB;

   case GL_COMPRESSED_RGBA:
   case GL_COMPRESSED_SRGB_ALPHA:
   case GL_COMPRESSED_RGBA_BPTC_UNORM_ARB:
   case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB:
   case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
   case GL_COMPRESSED_RGBA_FXT1_3DFX:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
   case GL_COMPRESSED_RGBA8_ETC2_EAC:
   case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
   case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case GL_RGBA_S3TC:
   case GL_RGBA4_S3TC:
   case GL_PALETTE4_RGBA8_OES:
   case GL_PALETTE8_RGB5_A1_OES:
   case GL_PALETTE4_RGBA4_OES:
   case GL_PALETTE4_RGB5_A1_OES:
   case GL_PALETTE8_RGBA8_OES:
   case GL_PALETTE8_RGBA4_OES:
   case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
   case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
      return GL_RGBA;

   case GL_COMPRESSED_ALPHA:
      return GL_ALPHA;

   case GL_COMPRESSED_LUMINANCE:
   case GL_COMPRESSED_SLUMINANCE:
   case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
   case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
      return GL_LUMINANCE;

   case GL_COMPRESSED_LUMINANCE_ALPHA:
   case GL_COMPRESSED_SLUMINANCE_ALPHA:
   case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
   case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
   case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      return GL_LUMINANCE_ALPHA;

   case GL_COMPRESSED_INTENSITY:
      return GL_INTENSITY;

   default:
      return 0;
   }
}

/**
 * Return list of (and count of) all specific texture compression
 * formats that are supported.
 *
 * Some formats are \b not returned by this function.  The
 * \c GL_COMPRESSED_TEXTURE_FORMATS query only returns formats that are
 * "suitable for general-purpose usage."  All texture compression extensions
 * have taken this to mean either linear RGB or linear RGBA.
 *
 * The GL_ARB_texture_compress_rgtc spec says:
 *
 *    "19) Should the GL_NUM_COMPRESSED_TEXTURE_FORMATS and
 *        GL_COMPRESSED_TEXTURE_FORMATS queries return the RGTC formats?
 *
 *        RESOLVED:  No.
 *
 *        The OpenGL 2.1 specification says "The only values returned
 *        by this query [GL_COMPRESSED_TEXTURE_FORMATS"] are those
 *        corresponding to formats suitable for general-purpose usage.
 *        The renderer will not enumerate formats with restrictions that
 *        need to be specifically understood prior to use."
 *
 *        Compressed textures with just red or red-green components are
 *        not general-purpose so should not be returned by these queries
 *        because they have restrictions.
 *
 *        Applications that seek to use the RGTC formats should do so
 *        by looking for this extension's name in the string returned by
 *        glGetString(GL_EXTENSIONS) rather than
 *        what GL_NUM_COMPRESSED_TEXTURE_FORMATS and
 *        GL_COMPRESSED_TEXTURE_FORMATS return."
 *
 * There is nearly identical wording in the GL_EXT_texture_compression_rgtc
 * spec.
 *
 * The GL_EXT_texture_rRGB spec says:
 *
 *    "22) Should the new COMPRESSED_SRGB_* formats be listed in an
 *        implementation's GL_COMPRESSED_TEXTURE_FORMATS list?
 *
 *        RESOLVED:  No.  Section 3.8.1 says formats listed by
 *        GL_COMPRESSED_TEXTURE_FORMATS are "suitable for general-purpose
 *        usage."  The non-linear distribution of red, green, and
 *        blue for these sRGB compressed formats makes them not really
 *        general-purpose."
 *
 * The GL_EXT_texture_compression_latc spec says:
 *
 *    "16) Should the GL_NUM_COMPRESSED_TEXTURE_FORMATS and
 *        GL_COMPRESSED_TEXTURE_FORMATS queries return the LATC formats?
 *
 *        RESOLVED:  No.
 *
 *        The OpenGL 2.1 specification says "The only values returned
 *        by this query [GL_COMPRESSED_TEXTURE_FORMATS"] are those
 *        corresponding to formats suitable for general-purpose usage.
 *        The renderer will not enumerate formats with restrictions that
 *        need to be specifically understood prior to use."
 *
 *        Historically, OpenGL implementation have advertised the RGB and
 *        RGBA versions of the S3TC extensions compressed format tokens
 *        through this mechanism.
 *
 *        The specification is not sufficiently clear about what "suitable
 *        for general-purpose usage" means.  Historically that seems to mean
 *        unsigned RGB or unsigned RGBA.  The DXT1 format supporting alpha
 *        (GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) is not exposed in the list (at
 *        least for NVIDIA drivers) because the alpha is always 1.0 expect
 *        when it is 0.0 when RGB is required to be black.  NVIDIA's even
 *        limits itself to true linear RGB or RGBA formats, specifically
 *        not including EXT_texture_sRGB's sRGB S3TC compressed formats.
 *
 *        Adding luminance and luminance-alpha texture formats (and
 *        certainly signed versions of luminance and luminance-alpha
 *        formats!) invites potential comptaibility problems with old
 *        applications using this mechanism since old applications are
 *        unlikely to expect non-RGB or non-RGBA formats to be advertised
 *        through this mechanism.  However no specific misinteractions
 *        with old applications is known.
 *
 *        Applications that seek to use the LATC formats should do so
 *        by looking for this extension's name in the string returned by
 *        glGetString(GL_EXTENSIONS) rather than
 *        what GL_NUM_COMPRESSED_TEXTURE_FORMATS and
 *        GL_COMPRESSED_TEXTURE_FORMATS return."
 *
 * There is no formal spec for GL_ATI_texture_compression_3dc.  Since the
 * formats added by this extension are luminance-alpha formats, it is
 * reasonable to expect them to follow the same rules as
 * GL_EXT_texture_compression_latc.  At the very least, Catalyst 11.6 does not
 * expose the 3dc formats through this mechanism.
 *
 * The spec for GL_ARB_texture_compression_bptc doesn't mention whether it
 * should be included in GL_COMPRESSED_TEXTURE_FORMATS. However as it takes a
 * very long time to compress the textures in this format it's probably not
 * very useful as a general format where the GL will have to compress it on
 * the fly.
 *
 * \param ctx  the GL context
 * \param formats  the resulting format list (may be NULL).
 *
 * \return number of formats.
 */
GLuint
_mesa_get_compressed_formats(struct gl_context *ctx, GLint *formats)
{
   GLint discard_formats[100];
   GLuint n = 0;

   if (!formats) {
      formats = discard_formats;
   }

   if (_mesa_is_desktop_gl(ctx) &&
       ctx->Extensions.TDFX_texture_compression_FXT1) {
      formats[n++] = GL_COMPRESSED_RGB_FXT1_3DFX;
      formats[n++] = GL_COMPRESSED_RGBA_FXT1_3DFX;
   }

   if (ctx->Extensions.EXT_texture_compression_s3tc) {
      formats[n++] = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
      formats[n++] = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
      formats[n++] = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;

      /* The ES and desktop GL specs diverge here.
       *
       * In desktop OpenGL, the driver can perform online compression of
       * uncompressed texture data.  GL_NUM_COMPRESSED_TEXTURE_FORMATS and
       * GL_COMPRESSED_TEXTURE_FORMATS give the application a list of
       * formats that it could ask the driver to compress with some
       * expectation of quality.  The GL_ARB_texture_compression spec
       * calls this "suitable for general-purpose usage."  As noted
       * above, this means GL_COMPRESSED_RGBA_S3TC_DXT1_EXT is not
       * included in the list.
       *
       * In OpenGL ES, the driver never performs compression.
       * GL_NUM_COMPRESSED_TEXTURE_FORMATS and
       * GL_COMPRESSED_TEXTURE_FORMATS give the application a list of
       * formats that the driver can receive from the application.  It
       * is the *complete* list of formats.  The
       * GL_EXT_texture_compression_s3tc spec says:
       *
       *     "New State for OpenGL ES 2.0.25 and 3.0.2 Specifications
       *
       *         The queries for NUM_COMPRESSED_TEXTURE_FORMATS and
       *         COMPRESSED_TEXTURE_FORMATS include
       *         COMPRESSED_RGB_S3TC_DXT1_EXT,
       *         COMPRESSED_RGBA_S3TC_DXT1_EXT,
       *         COMPRESSED_RGBA_S3TC_DXT3_EXT, and
       *         COMPRESSED_RGBA_S3TC_DXT5_EXT."
       *
       * Note that the addition is only to the OpenGL ES specification!
       */
      if (_mesa_is_gles(ctx)) {
         formats[n++] = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
      }
   }

   /* The GL_OES_compressed_ETC1_RGB8_texture spec says:
    *
    *     "New State
    *
    *         The queries for NUM_COMPRESSED_TEXTURE_FORMATS and
    *         COMPRESSED_TEXTURE_FORMATS include ETC1_RGB8_OES."
    */
   if (_mesa_is_gles(ctx)
       && ctx->Extensions.OES_compressed_ETC1_RGB8_texture) {
      formats[n++] = GL_ETC1_RGB8_OES;
   }

   /* Required by EXT_texture_compression_bptc in GLES. */
   if (_mesa_has_EXT_texture_compression_bptc(ctx)) {
      formats[n++] = GL_COMPRESSED_RGBA_BPTC_UNORM;
      formats[n++] = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
      formats[n++] = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
      formats[n++] = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
   }

   /* Required by EXT_texture_compression_rgtc in GLES. */
   if (_mesa_is_gles3(ctx) &&
       _mesa_has_EXT_texture_compression_rgtc(ctx)) {
      formats[n++] = GL_COMPRESSED_RED_RGTC1_EXT;
      formats[n++] = GL_COMPRESSED_SIGNED_RED_RGTC1_EXT;
      formats[n++] = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
      formats[n++] = GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT;
   }

   if (_mesa_is_gles1(ctx)) {
      formats[n++] = GL_PALETTE4_RGB8_OES;
      formats[n++] = GL_PALETTE4_RGBA8_OES;
      formats[n++] = GL_PALETTE4_R5_G6_B5_OES;
      formats[n++] = GL_PALETTE4_RGBA4_OES;
      formats[n++] = GL_PALETTE4_RGB5_A1_OES;
      formats[n++] = GL_PALETTE8_RGB8_OES;
      formats[n++] = GL_PALETTE8_RGBA8_OES;
      formats[n++] = GL_PALETTE8_R5_G6_B5_OES;
      formats[n++] = GL_PALETTE8_RGBA4_OES;
      formats[n++] = GL_PALETTE8_RGB5_A1_OES;
   }

   if (_mesa_is_gles3(ctx) || ctx->Extensions.ARB_ES3_compatibility) {
      formats[n++] = GL_COMPRESSED_RGB8_ETC2;
      formats[n++] = GL_COMPRESSED_RGBA8_ETC2_EAC;
      formats[n++] = GL_COMPRESSED_R11_EAC;
      formats[n++] = GL_COMPRESSED_RG11_EAC;
      formats[n++] = GL_COMPRESSED_SIGNED_R11_EAC;
      formats[n++] = GL_COMPRESSED_SIGNED_RG11_EAC;
      formats[n++] = GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
   }

   if (_mesa_is_gles3(ctx)) {
      formats[n++] = GL_COMPRESSED_SRGB8_ETC2;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
      formats[n++] = GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;
   }

   /* The KHR_texture_compression_astc_hdr spec says:
    *
    *    "Interactions with OpenGL 4.2
    *
    *        OpenGL 4.2 supports the feature that compressed textures can be
    *        compressed online, by passing the compressed texture format enum
    *        as the internal format when uploading a texture using TexImage1D,
    *        TexImage2D or TexImage3D (see Section 3.9.3, Texture Image
    *        Specification, subsection Encoding of Special Internal Formats).
    *
    *        Due to the complexity of the ASTC compression algorithm, it is
    *        not usually suitable for online use, and therefore ASTC support
    *        will be limited to pre-compressed textures only. Where on-device
    *        compression is required, a domain-specific limited compressor
    *        will typically be used, and this is therefore not suitable for
    *        implementation in the driver.
    *
    *        In particular, the ASTC format specifiers will not be added to
    *        Table 3.14, and thus will not be accepted by the TexImage*D
    *        functions, and will not be returned by the (already deprecated)
    *        COMPRESSED_TEXTURE_FORMATS query."
    *
    * The ES and the desktop specs diverge here. In OpenGL ES, the
    * COMPRESSED_TEXTURE_FORMATS query returns the set of supported specific
    * compressed formats.
    */
   if (_mesa_is_gles2(ctx) &&
       ctx->Extensions.KHR_texture_compression_astc_ldr) {
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;
   }

   if (_mesa_is_gles3(ctx) &&
       ctx->Extensions.OES_texture_compression_astc) {
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_3x3x3_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_4x3x3_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_4x4x3_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_4x4x4_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_5x4x4_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_5x5x4_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_5x5x5_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_6x5x5_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_6x6x5_OES;
      formats[n++] = GL_COMPRESSED_RGBA_ASTC_6x6x6_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES;
      formats[n++] = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES;
   }

   /* The GL_AMD_compressed_ATC_texture spec says:
    *
    *     "New State
    *
    *         The queries for NUM_COMPRESSED_TEXTURE_FORMATS and
    *         COMPRESSED_TEXTURE_FORMATS include ATC_RGB_AMD,
    *         ATC_RGBA_EXPLICIT_ALPHA_AMD, and ATC_RGBA_INTERPOLATED_ALPHA_AMD."
    */
   if (_mesa_has_AMD_compressed_ATC_texture(ctx)) {
      formats[n++] = GL_ATC_RGB_AMD;
      formats[n++] = GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
      formats[n++] = GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;
   }

   assert(n <= ARRAY_SIZE(discard_formats));

   return n;
}


/**
 * Convert GLenum to a compressed MESA_FORMAT_x.
 */
mesa_format
_mesa_glenum_to_compressed_format(GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RGB_FXT1_3DFX:
      return MESA_FORMAT_RGB_FXT1;
   case GL_COMPRESSED_RGBA_FXT1_3DFX:
      return MESA_FORMAT_RGBA_FXT1;

   case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
   case GL_RGB_S3TC:
   case GL_RGB4_S3TC:
      return MESA_FORMAT_RGB_DXT1;
   case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      return MESA_FORMAT_RGBA_DXT1;
   case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
   case GL_RGBA_S3TC:
   case GL_RGBA4_S3TC:
      return MESA_FORMAT_RGBA_DXT3;
   case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      return MESA_FORMAT_RGBA_DXT5;

   case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
      return MESA_FORMAT_SRGB_DXT1;
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
      return MESA_FORMAT_SRGBA_DXT1;
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
      return MESA_FORMAT_SRGBA_DXT3;
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      return MESA_FORMAT_SRGBA_DXT5;

   case GL_COMPRESSED_RED_RGTC1:
      return MESA_FORMAT_R_RGTC1_UNORM;
   case GL_COMPRESSED_SIGNED_RED_RGTC1:
      return MESA_FORMAT_R_RGTC1_SNORM;
   case GL_COMPRESSED_RG_RGTC2:
      return MESA_FORMAT_RG_RGTC2_UNORM;
   case GL_COMPRESSED_SIGNED_RG_RGTC2:
      return MESA_FORMAT_RG_RGTC2_SNORM;

   case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
      return MESA_FORMAT_L_LATC1_UNORM;
   case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
      return MESA_FORMAT_L_LATC1_SNORM;
   case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
   case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      return MESA_FORMAT_LA_LATC2_UNORM;
   case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
      return MESA_FORMAT_LA_LATC2_SNORM;

   case GL_ETC1_RGB8_OES:
      return MESA_FORMAT_ETC1_RGB8;
   case GL_COMPRESSED_RGB8_ETC2:
      return MESA_FORMAT_ETC2_RGB8;
   case GL_COMPRESSED_SRGB8_ETC2:
      return MESA_FORMAT_ETC2_SRGB8;
   case GL_COMPRESSED_RGBA8_ETC2_EAC:
      return MESA_FORMAT_ETC2_RGBA8_EAC;
   case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      return MESA_FORMAT_ETC2_SRGB8_ALPHA8_EAC;
   case GL_COMPRESSED_R11_EAC:
      return MESA_FORMAT_ETC2_R11_EAC;
   case GL_COMPRESSED_RG11_EAC:
      return MESA_FORMAT_ETC2_RG11_EAC;
   case GL_COMPRESSED_SIGNED_R11_EAC:
      return MESA_FORMAT_ETC2_SIGNED_R11_EAC;
   case GL_COMPRESSED_SIGNED_RG11_EAC:
      return MESA_FORMAT_ETC2_SIGNED_RG11_EAC;
   case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      return MESA_FORMAT_ETC2_RGB8_PUNCHTHROUGH_ALPHA1;
   case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      return MESA_FORMAT_ETC2_SRGB8_PUNCHTHROUGH_ALPHA1;

   case GL_COMPRESSED_RGBA_BPTC_UNORM:
      return MESA_FORMAT_BPTC_RGBA_UNORM;
   case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
      return MESA_FORMAT_BPTC_SRGB_ALPHA_UNORM;
   case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
      return MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT;
   case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      return MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT;

   case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
      return MESA_FORMAT_RGBA_ASTC_4x4;
   case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
      return MESA_FORMAT_RGBA_ASTC_5x4;
   case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
      return MESA_FORMAT_RGBA_ASTC_5x5;
   case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
      return MESA_FORMAT_RGBA_ASTC_6x5;
   case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
      return MESA_FORMAT_RGBA_ASTC_6x6;
   case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
      return MESA_FORMAT_RGBA_ASTC_8x5;
   case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
      return MESA_FORMAT_RGBA_ASTC_8x6;
   case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
      return MESA_FORMAT_RGBA_ASTC_8x8;
   case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
      return MESA_FORMAT_RGBA_ASTC_10x5;
   case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
      return MESA_FORMAT_RGBA_ASTC_10x6;
   case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
      return MESA_FORMAT_RGBA_ASTC_10x8;
   case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
      return MESA_FORMAT_RGBA_ASTC_10x10;
   case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
      return MESA_FORMAT_RGBA_ASTC_12x10;
   case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
      return MESA_FORMAT_RGBA_ASTC_12x12;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x4;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x6;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x8;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x6;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x8;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x10;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_12x10;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_12x12;
   case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:
      return MESA_FORMAT_RGBA_ASTC_3x3x3;
   case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:
      return MESA_FORMAT_RGBA_ASTC_4x3x3;
   case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:
      return MESA_FORMAT_RGBA_ASTC_4x4x3;
   case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:
      return MESA_FORMAT_RGBA_ASTC_4x4x4;
   case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:
      return MESA_FORMAT_RGBA_ASTC_5x4x4;
   case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:
      return MESA_FORMAT_RGBA_ASTC_5x5x4;
   case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:
      return MESA_FORMAT_RGBA_ASTC_5x5x5;
   case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:
      return MESA_FORMAT_RGBA_ASTC_6x5x5;
   case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:
      return MESA_FORMAT_RGBA_ASTC_6x6x5;
   case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:
      return MESA_FORMAT_RGBA_ASTC_6x6x6;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_3x3x3;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x3x3;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4x3;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4x4;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x4x4;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5x4;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x5x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6x5;
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:
      return MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6x6;

   case GL_ATC_RGB_AMD:
      return MESA_FORMAT_ATC_RGB;
   case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
      return MESA_FORMAT_ATC_RGBA_EXPLICIT;
   case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
      return MESA_FORMAT_ATC_RGBA_INTERPOLATED;

   default:
      return MESA_FORMAT_NONE;
   }
}


/**
 * Given a compressed MESA_FORMAT_x value, return the corresponding
 * GLenum for that format.
 * This is needed for glGetTexLevelParameter(GL_TEXTURE_INTERNAL_FORMAT)
 * which must return the specific texture format used when the user might
 * have originally specified a generic compressed format in their
 * glTexImage2D() call.
 * For non-compressed textures, we always return the user-specified
 * internal format unchanged.
 */
GLenum
_mesa_compressed_format_to_glenum(struct gl_context *ctx,
                                  mesa_format mesaFormat)
{
   switch (mesaFormat) {
   case MESA_FORMAT_RGB_FXT1:
      return GL_COMPRESSED_RGB_FXT1_3DFX;
   case MESA_FORMAT_RGBA_FXT1:
      return GL_COMPRESSED_RGBA_FXT1_3DFX;
   case MESA_FORMAT_RGB_DXT1:
      return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
   case MESA_FORMAT_RGBA_DXT1:
      return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
   case MESA_FORMAT_RGBA_DXT3:
      return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
   case MESA_FORMAT_RGBA_DXT5:
      return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
   case MESA_FORMAT_SRGB_DXT1:
      return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
   case MESA_FORMAT_SRGBA_DXT1:
      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
   case MESA_FORMAT_SRGBA_DXT3:
      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
   case MESA_FORMAT_SRGBA_DXT5:
      return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
   case MESA_FORMAT_R_RGTC1_UNORM:
      return GL_COMPRESSED_RED_RGTC1;
   case MESA_FORMAT_R_RGTC1_SNORM:
      return GL_COMPRESSED_SIGNED_RED_RGTC1;
   case MESA_FORMAT_RG_RGTC2_UNORM:
      return GL_COMPRESSED_RG_RGTC2;
   case MESA_FORMAT_RG_RGTC2_SNORM:
      return GL_COMPRESSED_SIGNED_RG_RGTC2;

   case MESA_FORMAT_L_LATC1_UNORM:
      return GL_COMPRESSED_LUMINANCE_LATC1_EXT;
   case MESA_FORMAT_L_LATC1_SNORM:
      return GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
   case MESA_FORMAT_LA_LATC2_UNORM:
      return GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
   case MESA_FORMAT_LA_LATC2_SNORM:
      return GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;

   case MESA_FORMAT_ETC1_RGB8:
      return GL_ETC1_RGB8_OES;
   case MESA_FORMAT_ETC2_RGB8:
      return GL_COMPRESSED_RGB8_ETC2;
   case MESA_FORMAT_ETC2_SRGB8:
      return GL_COMPRESSED_SRGB8_ETC2;
   case MESA_FORMAT_ETC2_RGBA8_EAC:
      return GL_COMPRESSED_RGBA8_ETC2_EAC;
   case MESA_FORMAT_ETC2_SRGB8_ALPHA8_EAC:
      return GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
   case MESA_FORMAT_ETC2_R11_EAC:
      return GL_COMPRESSED_R11_EAC;
   case MESA_FORMAT_ETC2_RG11_EAC:
      return GL_COMPRESSED_RG11_EAC;
   case MESA_FORMAT_ETC2_SIGNED_R11_EAC:
      return GL_COMPRESSED_SIGNED_R11_EAC;
   case MESA_FORMAT_ETC2_SIGNED_RG11_EAC:
      return GL_COMPRESSED_SIGNED_RG11_EAC;
   case MESA_FORMAT_ETC2_RGB8_PUNCHTHROUGH_ALPHA1:
      return GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
   case MESA_FORMAT_ETC2_SRGB8_PUNCHTHROUGH_ALPHA1:
      return GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2;

   case MESA_FORMAT_BPTC_RGBA_UNORM:
      return GL_COMPRESSED_RGBA_BPTC_UNORM;
   case MESA_FORMAT_BPTC_SRGB_ALPHA_UNORM:
      return GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
   case MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT:
      return GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
   case MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT:
      return GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;

   case MESA_FORMAT_RGBA_ASTC_4x4:
      return GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
   case MESA_FORMAT_RGBA_ASTC_5x4:
      return GL_COMPRESSED_RGBA_ASTC_5x4_KHR;
   case MESA_FORMAT_RGBA_ASTC_5x5:
      return GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
   case MESA_FORMAT_RGBA_ASTC_6x5:
      return GL_COMPRESSED_RGBA_ASTC_6x5_KHR;
   case MESA_FORMAT_RGBA_ASTC_6x6:
      return GL_COMPRESSED_RGBA_ASTC_6x6_KHR;
   case MESA_FORMAT_RGBA_ASTC_8x5:
      return GL_COMPRESSED_RGBA_ASTC_8x5_KHR;
   case MESA_FORMAT_RGBA_ASTC_8x6:
      return GL_COMPRESSED_RGBA_ASTC_8x6_KHR;
   case MESA_FORMAT_RGBA_ASTC_8x8:
      return GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
   case MESA_FORMAT_RGBA_ASTC_10x5:
      return GL_COMPRESSED_RGBA_ASTC_10x5_KHR;
   case MESA_FORMAT_RGBA_ASTC_10x6:
      return GL_COMPRESSED_RGBA_ASTC_10x6_KHR;
   case MESA_FORMAT_RGBA_ASTC_10x8:
      return GL_COMPRESSED_RGBA_ASTC_10x8_KHR;
   case MESA_FORMAT_RGBA_ASTC_10x10:
      return GL_COMPRESSED_RGBA_ASTC_10x10_KHR;
   case MESA_FORMAT_RGBA_ASTC_12x10:
      return GL_COMPRESSED_RGBA_ASTC_12x10_KHR;
   case MESA_FORMAT_RGBA_ASTC_12x12:
      return GL_COMPRESSED_RGBA_ASTC_12x12_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x4:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x6:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_8x8:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x6:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x8:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_10x10:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_12x10:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_12x12:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR;

   case MESA_FORMAT_RGBA_ASTC_3x3x3:
      return GL_COMPRESSED_RGBA_ASTC_3x3x3_OES;
   case MESA_FORMAT_RGBA_ASTC_4x3x3:
      return GL_COMPRESSED_RGBA_ASTC_4x3x3_OES;
   case MESA_FORMAT_RGBA_ASTC_4x4x3:
      return GL_COMPRESSED_RGBA_ASTC_4x4x3_OES;
   case MESA_FORMAT_RGBA_ASTC_4x4x4:
      return GL_COMPRESSED_RGBA_ASTC_4x4x4_OES;
   case MESA_FORMAT_RGBA_ASTC_5x4x4:
      return GL_COMPRESSED_RGBA_ASTC_5x4x4_OES;
   case MESA_FORMAT_RGBA_ASTC_5x5x4:
      return GL_COMPRESSED_RGBA_ASTC_5x5x4_OES;
   case MESA_FORMAT_RGBA_ASTC_5x5x5:
      return GL_COMPRESSED_RGBA_ASTC_5x5x5_OES;
   case MESA_FORMAT_RGBA_ASTC_6x5x5:
      return GL_COMPRESSED_RGBA_ASTC_6x5x5_OES;
   case MESA_FORMAT_RGBA_ASTC_6x6x5:
      return GL_COMPRESSED_RGBA_ASTC_6x6x5_OES;
   case MESA_FORMAT_RGBA_ASTC_6x6x6:
      return GL_COMPRESSED_RGBA_ASTC_6x6x6_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_3x3x3:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x3x3:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4x3:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_4x4x4:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x4x4:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5x4:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_5x5x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x5x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6x5:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES;
   case MESA_FORMAT_SRGB8_ALPHA8_ASTC_6x6x6:
      return GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES;

   case MESA_FORMAT_ATC_RGB:
      return GL_ATC_RGB_AMD;
   case MESA_FORMAT_ATC_RGBA_EXPLICIT:
      return GL_ATC_RGBA_EXPLICIT_ALPHA_AMD;
   case MESA_FORMAT_ATC_RGBA_INTERPOLATED:
      return GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD;

   default:
      _mesa_problem(ctx, "Unexpected mesa texture format in"
                    " _mesa_compressed_format_to_glenum()");
      return 0;
   }
}


/**
 * Return a texel-fetch function for the given format, or NULL if
 * invalid format.
 */
compressed_fetch_func
_mesa_get_compressed_fetch_func(mesa_format format)
{
   switch (_mesa_get_format_layout(format)) {
   case MESA_FORMAT_LAYOUT_S3TC:
      return _mesa_get_dxt_fetch_func(format);
   case MESA_FORMAT_LAYOUT_FXT1:
      return _mesa_get_fxt_fetch_func(format);
   case MESA_FORMAT_LAYOUT_RGTC:
   case MESA_FORMAT_LAYOUT_LATC:
      return _mesa_get_compressed_rgtc_func(format);
   case MESA_FORMAT_LAYOUT_ETC1:
      return _mesa_get_etc_fetch_func(format);
   case MESA_FORMAT_LAYOUT_BPTC:
      return _mesa_get_bptc_fetch_func(format);
   default:
      return NULL;
   }
}


/**
 * Decompress a compressed texture image, returning a GL_RGBA/GL_FLOAT image.
 * \param srcRowStride  stride in bytes between rows of blocks in the
 *                      compressed source image.
 */
void
_mesa_decompress_image(mesa_format format, GLuint width, GLuint height,
                       const GLubyte *src, GLint srcRowStride,
                       GLfloat *dest)
{
   compressed_fetch_func fetch;
   GLuint i, j;
   GLuint bytes, bw, bh;
   GLint stride;

   bytes = _mesa_get_format_bytes(format);
   _mesa_get_format_block_size(format, &bw, &bh);

   fetch = _mesa_get_compressed_fetch_func(format);
   if (!fetch) {
      _mesa_problem(NULL, "Unexpected format in _mesa_decompress_image()");
      return;
   }

   stride = srcRowStride * bh / bytes;

   for (j = 0; j < height; j++) {
      for (i = 0; i < width; i++) {
         fetch(src, stride, i, j, dest);
         dest += 4;
      }
   }
}
