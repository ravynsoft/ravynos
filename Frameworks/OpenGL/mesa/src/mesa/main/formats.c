/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008-2009  VMware, Inc.
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


#include "errors.h"

#include "formats.h"
#include "macros.h"
#include "glformats.h"
#include "c11/threads.h"
#include "util/hash_table.h"

/**
 * Information about texture formats.
 */
struct mesa_format_info
{
   mesa_format Name;

   /** text name for debugging */
   const char *StrName;

   enum mesa_format_layout Layout;

   /**
    * Base format is one of GL_RED, GL_RG, GL_RGB, GL_RGBA, GL_ALPHA,
    * GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_INTENSITY, GL_YCBCR_MESA,
    * GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL.
    */
   GLenum BaseFormat;

   /**
    * Logical data type: one of  GL_UNSIGNED_NORMALIZED, GL_SIGNED_NORMALIZED,
    * GL_UNSIGNED_INT, GL_INT, GL_FLOAT.
    */
   GLenum DataType;

   uint8_t RedBits;
   uint8_t GreenBits;
   uint8_t BlueBits;
   uint8_t AlphaBits;
   uint8_t LuminanceBits;
   uint8_t IntensityBits;
   uint8_t DepthBits;
   uint8_t StencilBits;

   bool IsSRGBFormat;

   /**
    * To describe compressed formats.  If not compressed, Width=Height=Depth=1.
    */
   uint8_t BlockWidth, BlockHeight, BlockDepth;
   uint8_t BytesPerBlock;

   uint8_t Swizzle[4];
   mesa_array_format ArrayFormat;
};

#include "format_info.h"

static const struct mesa_format_info *
_mesa_get_format_info(mesa_format format)
{
   const struct mesa_format_info *info = &format_info[format];
   STATIC_ASSERT(ARRAY_SIZE(format_info) == MESA_FORMAT_COUNT);

   /* The MESA_FORMAT_* enums are sparse, don't return a format info
    * for empty entries.
    */
   if (info->Name == MESA_FORMAT_NONE && format != MESA_FORMAT_NONE)
      return NULL;

   assert(info->Name == format);
   return info;
}


/** Return string name of format (for debugging) */
const char *
_mesa_get_format_name(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   if (!info)
      return NULL;
   return info->StrName;
}



/**
 * Return bytes needed to store a block of pixels in the given format.
 * Normally, a block is 1x1 (a single pixel).  But for compressed formats
 * a block may be 4x4 or 8x4, etc.
 *
 * Note: return is signed, so as not to coerce math to unsigned. cf. fdo #37351
 */
int
_mesa_get_format_bytes(mesa_format format)
{
   if (_mesa_format_is_mesa_array_format(format)) {
      return _mesa_array_format_get_type_size(format) *
             _mesa_array_format_get_num_channels(format);
   }

   const struct mesa_format_info *info = _mesa_get_format_info(format);
   assert(info->BytesPerBlock);
   assert(info->BytesPerBlock <= MAX_PIXEL_BYTES ||
          _mesa_is_format_compressed(format));
   return info->BytesPerBlock;
}


/**
 * Return bits per component for the given format.
 * \param format  one of MESA_FORMAT_x
 * \param pname  the component, such as GL_RED_BITS, GL_TEXTURE_BLUE_BITS, etc.
 */
GLint
_mesa_get_format_bits(mesa_format format, GLenum pname)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);

   switch (pname) {
   case GL_RED_BITS:
   case GL_TEXTURE_RED_SIZE:
   case GL_RENDERBUFFER_RED_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
   case GL_INTERNALFORMAT_RED_SIZE:
      return info->RedBits;
   case GL_GREEN_BITS:
   case GL_TEXTURE_GREEN_SIZE:
   case GL_RENDERBUFFER_GREEN_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
   case GL_INTERNALFORMAT_GREEN_SIZE:
      return info->GreenBits;
   case GL_BLUE_BITS:
   case GL_TEXTURE_BLUE_SIZE:
   case GL_RENDERBUFFER_BLUE_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
   case GL_INTERNALFORMAT_BLUE_SIZE:
      return info->BlueBits;
   case GL_ALPHA_BITS:
   case GL_TEXTURE_ALPHA_SIZE:
   case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
   case GL_INTERNALFORMAT_ALPHA_SIZE:
      return info->AlphaBits;
   case GL_TEXTURE_INTENSITY_SIZE:
      return info->IntensityBits;
   case GL_TEXTURE_LUMINANCE_SIZE:
      return info->LuminanceBits;
   case GL_INDEX_BITS:
      return 0;
   case GL_DEPTH_BITS:
   case GL_TEXTURE_DEPTH_SIZE_ARB:
   case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
   case GL_INTERNALFORMAT_DEPTH_SIZE:
      return info->DepthBits;
   case GL_STENCIL_BITS:
   case GL_TEXTURE_STENCIL_SIZE_EXT:
   case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
   case GL_INTERNALFORMAT_STENCIL_SIZE:
      return info->StencilBits;
   default:
      _mesa_problem(NULL, "bad pname in _mesa_get_format_bits()");
      return 0;
   }
}


unsigned int
_mesa_get_format_max_bits(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   unsigned int max = MAX2(info->RedBits, info->GreenBits);
   max = MAX2(max, info->BlueBits);
   max = MAX2(max, info->AlphaBits);
   max = MAX2(max, info->LuminanceBits);
   max = MAX2(max, info->IntensityBits);
   max = MAX2(max, info->DepthBits);
   max = MAX2(max, info->StencilBits);
   return max;
}


/**
 * Return the layout type of the given format.
 */
extern enum mesa_format_layout
_mesa_get_format_layout(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return info->Layout;
}


/**
 * Return the data type (or more specifically, the data representation)
 * for the given format.
 * The return value will be one of:
 *    GL_UNSIGNED_NORMALIZED = unsigned int representing [0,1]
 *    GL_SIGNED_NORMALIZED = signed int representing [-1, 1]
 *    GL_UNSIGNED_INT = an ordinary unsigned integer
 *    GL_INT = an ordinary signed integer
 *    GL_FLOAT = an ordinary float
 */
GLenum
_mesa_get_format_datatype(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return info->DataType;
}

static GLenum
get_base_format_for_array_format(mesa_array_format format)
{
   uint8_t swizzle[4];
   int num_channels;

   switch (_mesa_array_format_get_base_format(format)) {
   case MESA_ARRAY_FORMAT_BASE_FORMAT_DEPTH:
      return GL_DEPTH_COMPONENT;
   case MESA_ARRAY_FORMAT_BASE_FORMAT_STENCIL:
      return GL_STENCIL_INDEX;
   case MESA_ARRAY_FORMAT_BASE_FORMAT_RGBA_VARIANTS:
      break;
   }

   _mesa_array_format_get_swizzle(format, swizzle);
   num_channels = _mesa_array_format_get_num_channels(format);

   switch (num_channels) {
   case 4:
      /* FIXME: RGBX formats have 4 channels, but their base format is GL_RGB.
       * This is not really a problem for now because we only create array
       * formats from GL format/type combinations, and these cannot specify
       * RGBX formats.
       */
      return GL_RGBA;
   case 3:
      return GL_RGB;
   case 2:
      if (swizzle[0] == 0 &&
          swizzle[1] == 0 &&
          swizzle[2] == 0 &&
          swizzle[3] == 1)
         return GL_LUMINANCE_ALPHA;
      if (swizzle[0] == 1 &&
          swizzle[1] == 1 &&
          swizzle[2] == 1 &&
          swizzle[3] == 0)
         return GL_LUMINANCE_ALPHA;
      if (swizzle[0] == 0 &&
          swizzle[1] == 1 &&
          swizzle[2] == 4 &&
          swizzle[3] == 5)
         return GL_RG;
      if (swizzle[0] == 1 &&
          swizzle[1] == 0 &&
          swizzle[2] == 4 &&
          swizzle[3] == 5)
         return GL_RG;
      break;
   case 1:
      if (swizzle[0] == 0 &&
          swizzle[1] == 0 &&
          swizzle[2] == 0 &&
          swizzle[3] == 5)
         return GL_LUMINANCE;
      if (swizzle[0] == 0 &&
          swizzle[1] == 0 &&
          swizzle[2] == 0 &&
          swizzle[3] == 0)
         return GL_INTENSITY;
      if (swizzle[0] <= MESA_FORMAT_SWIZZLE_W)
         return GL_RED;
      if (swizzle[1] <= MESA_FORMAT_SWIZZLE_W)
         return GL_GREEN;
      if (swizzle[2] <= MESA_FORMAT_SWIZZLE_W)
         return GL_BLUE;
      if (swizzle[3] <= MESA_FORMAT_SWIZZLE_W)
         return GL_ALPHA;
      break;
   }

   unreachable("Unsupported format");
}

/**
 * Return the basic format for the given type.  The result will be one of
 * GL_RGB, GL_RGBA, GL_ALPHA, GL_LUMINANCE, GL_LUMINANCE_ALPHA, GL_INTENSITY,
 * GL_YCBCR_MESA, GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, GL_DEPTH_STENCIL.
 * This functions accepts a mesa_format or a mesa_array_format.
 */
GLenum
_mesa_get_format_base_format(uint32_t format)
{
   if (!_mesa_format_is_mesa_array_format(format)) {
      const struct mesa_format_info *info = _mesa_get_format_info(format);
      return info->BaseFormat;
   } else {
      return get_base_format_for_array_format(format);
   }
}


/**
 * Return the block size (in pixels) for the given format.  Normally
 * the block size is 1x1.  But compressed formats will have block sizes
 * of 4x4 or 8x4 pixels, etc.
 * \param bw  returns block width in pixels
 * \param bh  returns block height in pixels
 */
void
_mesa_get_format_block_size(mesa_format format,
                            unsigned int *bw, unsigned int *bh)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   /* Use _mesa_get_format_block_size_3d() for 3D blocks. */
   assert(info->BlockDepth == 1);

   *bw = info->BlockWidth;
   *bh = info->BlockHeight;
}


/**
 * Return the block size (in pixels) for the given format. Normally
 * the block size is 1x1x1. But compressed formats will have block
 * sizes of 4x4x4, 3x3x3 pixels, etc.
 * \param bw  returns block width in pixels
 * \param bh  returns block height in pixels
 * \param bd  returns block depth in pixels
 */
void
_mesa_get_format_block_size_3d(mesa_format format,
                               unsigned int *bw,
                               unsigned int *bh,
                               unsigned int *bd)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   *bw = info->BlockWidth;
   *bh = info->BlockHeight;
   *bd = info->BlockDepth;
}


/**
 * Returns the an array of four numbers representing the transformation
 * from the RGBA or SZ colorspace to the given format.  For array formats,
 * the i'th RGBA component is given by:
 *
 * if (swizzle[i] <= MESA_FORMAT_SWIZZLE_W)
 *    comp = data[swizzle[i]];
 * else if (swizzle[i] == MESA_FORMAT_SWIZZLE_ZERO)
 *    comp = 0;
 * else if (swizzle[i] == MESA_FORMAT_SWIZZLE_ONE)
 *    comp = 1;
 * else if (swizzle[i] == MESA_FORMAT_SWIZZLE_NONE)
 *    // data does not contain a channel of this format
 *
 * For packed formats, the swizzle gives the number of components left of
 * the least significant bit.
 *
 * Compressed formats have no swizzle.
 */
void
_mesa_get_format_swizzle(mesa_format format, uint8_t swizzle_out[4])
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   memcpy(swizzle_out, info->Swizzle, sizeof(info->Swizzle));
}

mesa_array_format
_mesa_array_format_flip_channels(mesa_array_format format)
{
   int num_channels;
   uint8_t swizzle[4];

   num_channels = _mesa_array_format_get_num_channels(format);
   _mesa_array_format_get_swizzle(format, swizzle);

   if (num_channels == 1 || num_channels == 3)
      return format;

   if (num_channels == 2) {
      /* Assert that the swizzle makes sense for 2 channels */
      for (unsigned i = 0; i < 4; i++)
         assert(swizzle[i] != 2 && swizzle[i] != 3);

      static const uint8_t flip_xy[7] = { 1, 0, 2, 3, 4, 5, 6 };
      _mesa_array_format_set_swizzle(&format,
                                     flip_xy[swizzle[0]], flip_xy[swizzle[1]],
                                     flip_xy[swizzle[2]], flip_xy[swizzle[3]]);
      return format;
   }

   if (num_channels == 4) {
      static const uint8_t flip[7] = { 3, 2, 1, 0, 4, 5, 6 };
      _mesa_array_format_set_swizzle(&format,
                                     flip[swizzle[0]], flip[swizzle[1]],
                                     flip[swizzle[2]], flip[swizzle[3]]);
      return format;
   }

   unreachable("Invalid array format");
}

static uint32_t
_mesa_format_info_to_array_format(const struct mesa_format_info *info)
{
#if UTIL_ARCH_BIG_ENDIAN
   if (info->ArrayFormat && info->Layout == MESA_FORMAT_LAYOUT_PACKED)
      return _mesa_array_format_flip_channels(info->ArrayFormat);
   else
#endif
      return info->ArrayFormat;
}

uint32_t
_mesa_format_to_array_format(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return _mesa_format_info_to_array_format(info);
}

static struct hash_table *format_array_format_table;
static once_flag format_array_format_table_exists = ONCE_FLAG_INIT;

static void
format_array_format_table_destroy(void)
{
   _mesa_hash_table_destroy(format_array_format_table, NULL);
}

static bool
array_formats_equal(const void *a, const void *b)
{
   return (intptr_t)a == (intptr_t)b;
}

static void
format_array_format_table_init(void)
{
   const struct mesa_format_info *info;
   mesa_array_format array_format;
   unsigned f;

   format_array_format_table = _mesa_hash_table_create(NULL, NULL,
                                                       array_formats_equal);

   if (!format_array_format_table) {
      _mesa_error_no_memory(__func__);
      return;
   }

   for (f = 1; f < MESA_FORMAT_COUNT; ++f) {
      info = _mesa_get_format_info(f);
      if (!info || !info->ArrayFormat)
         continue;

      /* All sRGB formats should have an equivalent UNORM format, and that's
       * the one we want in the table.
       */
      if (_mesa_is_format_srgb(f))
         continue;

      array_format = _mesa_format_info_to_array_format(info);
      _mesa_hash_table_insert_pre_hashed(format_array_format_table,
                                         array_format,
                                         (void *)(intptr_t)array_format,
                                         (void *)(intptr_t)f);
   }

   atexit(format_array_format_table_destroy);
}

mesa_format
_mesa_format_from_array_format(uint32_t array_format)
{
   struct hash_entry *entry;

   assert(_mesa_format_is_mesa_array_format(array_format));

   call_once(&format_array_format_table_exists, format_array_format_table_init);

   if (!format_array_format_table) {
      static const once_flag once_flag_init = ONCE_FLAG_INIT;
      format_array_format_table_exists = once_flag_init;
      return MESA_FORMAT_NONE;
   }

   entry = _mesa_hash_table_search_pre_hashed(format_array_format_table,
                                              array_format,
                                              (void *)(intptr_t)array_format);
   if (entry)
      return (intptr_t)entry->data;
   else
      return MESA_FORMAT_NONE;
}

/** Is the given format a compressed format? */
bool
_mesa_is_format_compressed(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return info->BlockWidth > 1 || info->BlockHeight > 1;
}


/**
 * Determine if the given format represents a packed depth/stencil buffer.
 */
bool
_mesa_is_format_packed_depth_stencil(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);

   return info->BaseFormat == GL_DEPTH_STENCIL;
}


/**
 * Is the given format a signed/unsigned integer color format?
 */
bool
_mesa_is_format_integer_color(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return (info->DataType == GL_INT || info->DataType == GL_UNSIGNED_INT) &&
      info->BaseFormat != GL_DEPTH_COMPONENT &&
      info->BaseFormat != GL_DEPTH_STENCIL &&
      info->BaseFormat != GL_STENCIL_INDEX;
}


/**
 * Is the given format an unsigned integer format?
 */
bool
_mesa_is_format_unsigned(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return _mesa_is_type_unsigned(info->DataType);
}


/**
 * Does the given format store signed values?
 */
bool
_mesa_is_format_signed(mesa_format format)
{
   if (format == MESA_FORMAT_R11G11B10_FLOAT ||
       format == MESA_FORMAT_R9G9B9E5_FLOAT) {
      /* these packed float formats only store unsigned values */
      return false;
   }
   else {
      const struct mesa_format_info *info = _mesa_get_format_info(format);
      return (info->DataType == GL_SIGNED_NORMALIZED ||
              info->DataType == GL_INT ||
              info->DataType == GL_FLOAT);
   }
}

/**
 * Is the given format an integer format?
 */
bool
_mesa_is_format_integer(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return (info->DataType == GL_INT || info->DataType == GL_UNSIGNED_INT);
}


/**
 * Return true if the given format is a color format.
 */
bool
_mesa_is_format_color_format(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   switch (info->BaseFormat) {
   case GL_DEPTH_COMPONENT:
   case GL_STENCIL_INDEX:
   case GL_DEPTH_STENCIL:
      return false;
   default:
      return true;
   }
}

bool
_mesa_is_format_srgb(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return info->IsSRGBFormat;
}

/**
 * Return TRUE if format is an ETC2 compressed format specified
 * by GL_ARB_ES3_compatibility.
 */
bool
_mesa_is_format_etc2(mesa_format format)
{
   return _mesa_get_format_layout(format) == MESA_FORMAT_LAYOUT_ETC2;
}


/**
 * Return TRUE if format is an ASTC 2D compressed format.
 */
bool
_mesa_is_format_astc_2d(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return info->Layout == MESA_FORMAT_LAYOUT_ASTC && info->BlockDepth == 1;
}


/**
 * Return TRUE if format is an S3TC compressed format.
 */
bool
_mesa_is_format_s3tc(mesa_format format)
{
   return _mesa_get_format_layout(format) == MESA_FORMAT_LAYOUT_S3TC;
}


/**
 * Return TRUE if format is an RGTC compressed format.
 */
bool
_mesa_is_format_rgtc(mesa_format format)
{
   return _mesa_get_format_layout(format) == MESA_FORMAT_LAYOUT_RGTC;
}


/**
 * Return TRUE if format is an LATC compressed format.
 */
bool
_mesa_is_format_latc(mesa_format format)
{
   return _mesa_get_format_layout(format) == MESA_FORMAT_LAYOUT_LATC;
}


/**
 * Return TRUE if format is an BPTC compressed format.
 */
bool
_mesa_is_format_bptc(mesa_format format)
{
   return _mesa_get_format_layout(format) == MESA_FORMAT_LAYOUT_BPTC;
}


/**
 * If the given format is a compressed format, return a corresponding
 * uncompressed format.
 */
mesa_format
_mesa_get_uncompressed_format(mesa_format format)
{
   switch (format) {
   case MESA_FORMAT_RGB_FXT1:
      return MESA_FORMAT_BGR_UNORM8;
   case MESA_FORMAT_RGBA_FXT1:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_RGB_DXT1:
   case MESA_FORMAT_SRGB_DXT1:
      return MESA_FORMAT_BGR_UNORM8;
   case MESA_FORMAT_RGBA_DXT1:
   case MESA_FORMAT_SRGBA_DXT1:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_RGBA_DXT3:
   case MESA_FORMAT_SRGBA_DXT3:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_RGBA_DXT5:
   case MESA_FORMAT_SRGBA_DXT5:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_R_RGTC1_UNORM:
      return MESA_FORMAT_R_UNORM8;
   case MESA_FORMAT_R_RGTC1_SNORM:
      return MESA_FORMAT_R_SNORM8;
   case MESA_FORMAT_RG_RGTC2_UNORM:
      return MESA_FORMAT_RG_UNORM8;
   case MESA_FORMAT_RG_RGTC2_SNORM:
      return MESA_FORMAT_RG_SNORM8;
   case MESA_FORMAT_L_LATC1_UNORM:
      return MESA_FORMAT_L_UNORM8;
   case MESA_FORMAT_L_LATC1_SNORM:
      return MESA_FORMAT_L_SNORM8;
   case MESA_FORMAT_LA_LATC2_UNORM:
      return MESA_FORMAT_LA_UNORM8;
   case MESA_FORMAT_LA_LATC2_SNORM:
      return MESA_FORMAT_LA_SNORM8;
   case MESA_FORMAT_ETC1_RGB8:
   case MESA_FORMAT_ETC2_RGB8:
   case MESA_FORMAT_ETC2_SRGB8:
   case MESA_FORMAT_ATC_RGB:
      return MESA_FORMAT_BGR_UNORM8;
   case MESA_FORMAT_ETC2_RGBA8_EAC:
   case MESA_FORMAT_ETC2_SRGB8_ALPHA8_EAC:
   case MESA_FORMAT_ETC2_RGB8_PUNCHTHROUGH_ALPHA1:
   case MESA_FORMAT_ETC2_SRGB8_PUNCHTHROUGH_ALPHA1:
   case MESA_FORMAT_ATC_RGBA_EXPLICIT:
   case MESA_FORMAT_ATC_RGBA_INTERPOLATED:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_ETC2_R11_EAC:
   case MESA_FORMAT_ETC2_SIGNED_R11_EAC:
      return MESA_FORMAT_R_UNORM16;
   case MESA_FORMAT_ETC2_RG11_EAC:
   case MESA_FORMAT_ETC2_SIGNED_RG11_EAC:
      return MESA_FORMAT_RG_UNORM16;
   case MESA_FORMAT_BPTC_RGBA_UNORM:
   case MESA_FORMAT_BPTC_SRGB_ALPHA_UNORM:
      return MESA_FORMAT_A8B8G8R8_UNORM;
   case MESA_FORMAT_BPTC_RGB_UNSIGNED_FLOAT:
   case MESA_FORMAT_BPTC_RGB_SIGNED_FLOAT:
      return MESA_FORMAT_RGB_FLOAT32;
   default:
      assert(!_mesa_is_format_compressed(format));
      return format;
   }
}


unsigned int
_mesa_format_num_components(mesa_format format)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   return ((info->RedBits > 0) +
           (info->GreenBits > 0) +
           (info->BlueBits > 0) +
           (info->AlphaBits > 0) +
           (info->LuminanceBits > 0) +
           (info->IntensityBits > 0) +
           (info->DepthBits > 0) +
           (info->StencilBits > 0));
}


/**
 * Returns true if a color format has data stored in the R/G/B/A channels,
 * given an index from 0 to 3.
 */
bool
_mesa_format_has_color_component(mesa_format format, int component)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);

   assert(info->BaseFormat != GL_DEPTH_COMPONENT &&
          info->BaseFormat != GL_DEPTH_STENCIL &&
          info->BaseFormat != GL_STENCIL_INDEX);

   switch (component) {
   case 0:
      return (info->RedBits + info->IntensityBits + info->LuminanceBits) > 0;
   case 1:
      return (info->GreenBits + info->IntensityBits + info->LuminanceBits) > 0;
   case 2:
      return (info->BlueBits + info->IntensityBits + info->LuminanceBits) > 0;
   case 3:
      return (info->AlphaBits + info->IntensityBits) > 0;
   default:
      assert(!"Invalid color component: must be 0..3");
      return false;
   }
}


/**
 * Return number of bytes needed to store an image of the given size
 * in the given format.
 */
uint32_t
_mesa_format_image_size(mesa_format format, int width,
                        int height, int depth)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   uint32_t sz;
   /* Strictly speaking, a conditional isn't needed here */
   if (info->BlockWidth > 1 || info->BlockHeight > 1 || info->BlockDepth > 1) {
      /* compressed format (2D only for now) */
      const uint32_t bw = info->BlockWidth;
      const uint32_t bh = info->BlockHeight;
      const uint32_t bd = info->BlockDepth;
      const uint32_t wblocks = (width + bw - 1) / bw;
      const uint32_t hblocks = (height + bh - 1) / bh;
      const uint32_t dblocks = (depth + bd - 1) / bd;
      sz = wblocks * hblocks * dblocks * info->BytesPerBlock;
   } else
      /* non-compressed */
      sz = width * height * depth * info->BytesPerBlock;

   return sz;
}


/**
 * Same as _mesa_format_image_size() but returns a 64-bit value to
 * accommodate very large textures.
 */
uint64_t
_mesa_format_image_size64(mesa_format format, int width,
                          int height, int depth)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   uint64_t sz;
   /* Strictly speaking, a conditional isn't needed here */
   if (info->BlockWidth > 1 || info->BlockHeight > 1 || info->BlockDepth > 1) {
      /* compressed format (2D only for now) */
      const uint64_t bw = info->BlockWidth;
      const uint64_t bh = info->BlockHeight;
      const uint64_t bd = info->BlockDepth;
      const uint64_t wblocks = (width + bw - 1) / bw;
      const uint64_t hblocks = (height + bh - 1) / bh;
      const uint64_t dblocks = (depth + bd - 1) / bd;
      sz = wblocks * hblocks * dblocks * info->BytesPerBlock;
   } else
      /* non-compressed */
      sz = ((uint64_t) width * (uint64_t) height *
            (uint64_t) depth * info->BytesPerBlock);

   return sz;
}



int32_t
_mesa_format_row_stride(mesa_format format, int width)
{
   const struct mesa_format_info *info = _mesa_get_format_info(format);
   /* Strictly speaking, a conditional isn't needed here */
   if (info->BlockWidth > 1 || info->BlockHeight > 1) {
      /* compressed format */
      const uint32_t bw = info->BlockWidth;
      const uint32_t wblocks = (width + bw - 1) / bw;
      const int32_t stride = wblocks * info->BytesPerBlock;
      return stride;
   }
   else {
      const int32_t stride = width * info->BytesPerBlock;
      return stride;
   }
}



/**
 * Return datatype and number of components per texel for the given
 * uncompressed mesa_format. Only used for mipmap generation code.
 */
void
_mesa_uncompressed_format_to_type_and_comps(mesa_format format,
                               GLenum *datatype, GLuint *comps)
{
   switch (format) {
   case MESA_FORMAT_A8B8G8R8_UNORM:
   case MESA_FORMAT_R8G8B8A8_UNORM:
   case MESA_FORMAT_B8G8R8A8_UNORM:
   case MESA_FORMAT_A8R8G8B8_UNORM:
   case MESA_FORMAT_X8B8G8R8_UNORM:
   case MESA_FORMAT_R8G8B8X8_UNORM:
   case MESA_FORMAT_B8G8R8X8_UNORM:
   case MESA_FORMAT_X8R8G8B8_UNORM:
   case MESA_FORMAT_A8B8G8R8_UINT:
   case MESA_FORMAT_R8G8B8A8_UINT:
   case MESA_FORMAT_B8G8R8A8_UINT:
   case MESA_FORMAT_A8R8G8B8_UINT:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 4;
      return;
   case MESA_FORMAT_BGR_UNORM8:
   case MESA_FORMAT_RGB_UNORM8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_B5G6R5_UNORM:
   case MESA_FORMAT_R5G6B5_UNORM:
   case MESA_FORMAT_B5G6R5_UINT:
   case MESA_FORMAT_R5G6B5_UINT:
      *datatype = GL_UNSIGNED_SHORT_5_6_5;
      *comps = 3;
      return;

   case MESA_FORMAT_B4G4R4A4_UNORM:
   case MESA_FORMAT_A4R4G4B4_UNORM:
   case MESA_FORMAT_B4G4R4X4_UNORM:
   case MESA_FORMAT_B4G4R4A4_UINT:
   case MESA_FORMAT_A4R4G4B4_UINT:
      *datatype = GL_UNSIGNED_SHORT_4_4_4_4;
      *comps = 4;
      return;

   case MESA_FORMAT_B5G5R5A1_UNORM:
   case MESA_FORMAT_A1R5G5B5_UNORM:
   case MESA_FORMAT_B5G5R5X1_UNORM:
   case MESA_FORMAT_B5G5R5A1_UINT:
   case MESA_FORMAT_A1R5G5B5_UINT:
      *datatype = GL_UNSIGNED_SHORT_1_5_5_5_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_B10G10R10A2_UNORM:
      *datatype = GL_UNSIGNED_INT_2_10_10_10_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_A1B5G5R5_UNORM:
   case MESA_FORMAT_A1B5G5R5_UINT:
   case MESA_FORMAT_X1B5G5R5_UNORM:
      *datatype = GL_UNSIGNED_SHORT_5_5_5_1;
      *comps = 4;
      return;

   case MESA_FORMAT_L4A4_UNORM:
      *datatype = MESA_UNSIGNED_BYTE_4_4;
      *comps = 2;
      return;

   case MESA_FORMAT_LA_UNORM8:
   case MESA_FORMAT_RG_UNORM8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;

   case MESA_FORMAT_LA_UNORM16:
   case MESA_FORMAT_RG_UNORM16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 2;
      return;

   case MESA_FORMAT_R_UNORM16:
   case MESA_FORMAT_A_UNORM16:
   case MESA_FORMAT_L_UNORM16:
   case MESA_FORMAT_I_UNORM16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 1;
      return;

   case MESA_FORMAT_R3G3B2_UNORM:
   case MESA_FORMAT_R3G3B2_UINT:
      *datatype = GL_UNSIGNED_BYTE_2_3_3_REV;
      *comps = 3;
      return;
   case MESA_FORMAT_A4B4G4R4_UNORM:
   case MESA_FORMAT_A4B4G4R4_UINT:
      *datatype = GL_UNSIGNED_SHORT_4_4_4_4;
      *comps = 4;
      return;

   case MESA_FORMAT_R4G4B4A4_UNORM:
   case MESA_FORMAT_R4G4B4A4_UINT:
      *datatype = GL_UNSIGNED_SHORT_4_4_4_4;
      *comps = 4;
      return;
   case MESA_FORMAT_R5G5B5A1_UNORM:
   case MESA_FORMAT_R5G5B5A1_UINT:
      *datatype = GL_UNSIGNED_SHORT_1_5_5_5_REV;
      *comps = 4;
      return;
   case MESA_FORMAT_A2B10G10R10_UNORM:
   case MESA_FORMAT_A2B10G10R10_UINT:
      *datatype = GL_UNSIGNED_INT_10_10_10_2;
      *comps = 4;
      return;
   case MESA_FORMAT_A2R10G10B10_UNORM:
   case MESA_FORMAT_A2R10G10B10_UINT:
      *datatype = GL_UNSIGNED_INT_10_10_10_2;
      *comps = 4;
      return;

   case MESA_FORMAT_B2G3R3_UNORM:
   case MESA_FORMAT_B2G3R3_UINT:
      *datatype = GL_UNSIGNED_BYTE_3_3_2;
      *comps = 3;
      return;

   case MESA_FORMAT_A_UNORM8:
   case MESA_FORMAT_L_UNORM8:
   case MESA_FORMAT_I_UNORM8:
   case MESA_FORMAT_R_UNORM8:
   case MESA_FORMAT_S_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;

   case MESA_FORMAT_YCBCR:
   case MESA_FORMAT_YCBCR_REV:
   case MESA_FORMAT_RG_RB_UNORM8:
   case MESA_FORMAT_RB_RG_UNORM8:
   case MESA_FORMAT_GR_BR_UNORM8:
   case MESA_FORMAT_BR_GR_UNORM8:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 2;
      return;

   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      *datatype = GL_UNSIGNED_INT_24_8_MESA;
      *comps = 2;
      return;

   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      *datatype = GL_UNSIGNED_INT_8_24_REV_MESA;
      *comps = 2;
      return;

   case MESA_FORMAT_Z_UNORM16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 1;
      return;

   case MESA_FORMAT_Z24_UNORM_X8_UINT:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;

   case MESA_FORMAT_X8_UINT_Z24_UNORM:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;

   case MESA_FORMAT_Z_UNORM32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;

   case MESA_FORMAT_Z_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 1;
      return;

   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      *datatype = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
      *comps = 1;
      return;

   case MESA_FORMAT_R_SNORM8:
   case MESA_FORMAT_A_SNORM8:
   case MESA_FORMAT_L_SNORM8:
   case MESA_FORMAT_I_SNORM8:
      *datatype = GL_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_SNORM8:
   case MESA_FORMAT_LA_SNORM8:
      *datatype = GL_BYTE;
      *comps = 2;
      return;
   case MESA_FORMAT_A8B8G8R8_SNORM:
   case MESA_FORMAT_R8G8B8A8_SNORM:
   case MESA_FORMAT_X8B8G8R8_SNORM:
      *datatype = GL_BYTE;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBA_UNORM16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 4;
      return;

   case MESA_FORMAT_R_SNORM16:
   case MESA_FORMAT_A_SNORM16:
   case MESA_FORMAT_L_SNORM16:
   case MESA_FORMAT_I_SNORM16:
      *datatype = GL_SHORT;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_SNORM16:
   case MESA_FORMAT_LA_SNORM16:
      *datatype = GL_SHORT;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_SNORM16:
      *datatype = GL_SHORT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_SNORM16:
      *datatype = GL_SHORT;
      *comps = 4;
      return;

   case MESA_FORMAT_BGR_SRGB8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_A8B8G8R8_SRGB:
   case MESA_FORMAT_B8G8R8A8_SRGB:
   case MESA_FORMAT_A8R8G8B8_SRGB:
   case MESA_FORMAT_R8G8B8A8_SRGB:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 4;
      return;
   case MESA_FORMAT_L_SRGB8:
   case MESA_FORMAT_R_SRGB8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_SRGB8:
   case MESA_FORMAT_RG_SRGB8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;

   case MESA_FORMAT_RGBA_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 4;
      return;
   case MESA_FORMAT_RGBA_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 4;
      return;
   case MESA_FORMAT_RGB_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGB_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 3;
      return;
   case MESA_FORMAT_LA_FLOAT32:
   case MESA_FORMAT_RG_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 2;
      return;
   case MESA_FORMAT_LA_FLOAT16:
   case MESA_FORMAT_RG_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 2;
      return;
   case MESA_FORMAT_A_FLOAT32:
   case MESA_FORMAT_L_FLOAT32:
   case MESA_FORMAT_I_FLOAT32:
   case MESA_FORMAT_R_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 1;
      return;
   case MESA_FORMAT_A_FLOAT16:
   case MESA_FORMAT_L_FLOAT16:
   case MESA_FORMAT_I_FLOAT16:
   case MESA_FORMAT_R_FLOAT16:
      *datatype = GL_HALF_FLOAT_ARB;
      *comps = 1;
      return;

   case MESA_FORMAT_A_UINT8:
   case MESA_FORMAT_L_UINT8:
   case MESA_FORMAT_I_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;

   case MESA_FORMAT_A_UINT16:
   case MESA_FORMAT_L_UINT16:
   case MESA_FORMAT_I_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 2;
      return;
   case MESA_FORMAT_A_UINT32:
   case MESA_FORMAT_L_UINT32:
   case MESA_FORMAT_I_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 2;
      return;
   case MESA_FORMAT_A_SINT8:
   case MESA_FORMAT_L_SINT8:
   case MESA_FORMAT_I_SINT8:
      *datatype = GL_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_SINT8:
      *datatype = GL_BYTE;
      *comps = 2;
      return;

   case MESA_FORMAT_A_SINT16:
   case MESA_FORMAT_L_SINT16:
   case MESA_FORMAT_I_SINT16:
      *datatype = GL_SHORT;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_SINT16:
      *datatype = GL_SHORT;
      *comps = 2;
      return;

   case MESA_FORMAT_A_SINT32:
   case MESA_FORMAT_L_SINT32:
   case MESA_FORMAT_I_SINT32:
      *datatype = GL_INT;
      *comps = 1;
      return;
   case MESA_FORMAT_LA_SINT32:
      *datatype = GL_INT;
      *comps = 2;
      return;

   case MESA_FORMAT_R_SINT8:
      *datatype = GL_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_SINT8:
      *datatype = GL_BYTE;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_SINT8:
      *datatype = GL_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_SINT8:
      *datatype = GL_BYTE;
      *comps = 4;
      return;
   case MESA_FORMAT_R_SINT16:
      *datatype = GL_SHORT;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_SINT16:
      *datatype = GL_SHORT;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_SINT16:
      *datatype = GL_SHORT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_SINT16:
      *datatype = GL_SHORT;
      *comps = 4;
      return;
   case MESA_FORMAT_R_SINT32:
      *datatype = GL_INT;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_SINT32:
      *datatype = GL_INT;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_SINT32:
      *datatype = GL_INT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_SINT32:
      *datatype = GL_INT;
      *comps = 4;
      return;

   /**
    * \name Non-normalized unsigned integer formats.
    */
   case MESA_FORMAT_R_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 3;
      return;
   case MESA_FORMAT_R_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 4;
      return;
   case MESA_FORMAT_R_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 1;
      return;
   case MESA_FORMAT_RG_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 2;
      return;
   case MESA_FORMAT_RGB_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 3;
      return;
   case MESA_FORMAT_RGBA_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 4;
      return;

   case MESA_FORMAT_R9G9B9E5_FLOAT:
      *datatype = GL_UNSIGNED_INT_5_9_9_9_REV;
      *comps = 3;
      return;

   case MESA_FORMAT_R11G11B10_FLOAT:
      *datatype = GL_UNSIGNED_INT_10F_11F_11F_REV;
      *comps = 3;
      return;

   case MESA_FORMAT_B10G10R10A2_UINT:
   case MESA_FORMAT_R10G10B10A2_UINT:
      *datatype = GL_UNSIGNED_INT_2_10_10_10_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_R8G8B8X8_SRGB:
   case MESA_FORMAT_X8B8G8R8_SRGB:
   case MESA_FORMAT_RGBX_UINT8:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 4;
      return;

   case MESA_FORMAT_R8G8B8X8_SNORM:
   case MESA_FORMAT_RGBX_SINT8:
      *datatype = GL_BYTE;
      *comps = 4;
      return;

   case MESA_FORMAT_B10G10R10X2_UNORM:
   case MESA_FORMAT_R10G10B10X2_UNORM:
      *datatype = GL_UNSIGNED_INT_2_10_10_10_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_UNORM16:
   case MESA_FORMAT_RGBX_UINT16:
      *datatype = GL_UNSIGNED_SHORT;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_SNORM16:
   case MESA_FORMAT_RGBX_SINT16:
      *datatype = GL_SHORT;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_FLOAT16:
      *datatype = GL_HALF_FLOAT;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_FLOAT32:
      *datatype = GL_FLOAT;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_UINT32:
      *datatype = GL_UNSIGNED_INT;
      *comps = 4;
      return;

   case MESA_FORMAT_RGBX_SINT32:
      *datatype = GL_INT;
      *comps = 4;
      return;

   case MESA_FORMAT_R10G10B10A2_UNORM:
      *datatype = GL_UNSIGNED_INT_2_10_10_10_REV;
      *comps = 4;
      return;

   case MESA_FORMAT_B8G8R8X8_SRGB:
   case MESA_FORMAT_X8R8G8B8_SRGB:
      *datatype = GL_UNSIGNED_BYTE;
      *comps = 4;
      return;

   case MESA_FORMAT_COUNT:
      assert(0);
      return;
   default: {
      const char *name = _mesa_get_format_name(format);
      /* Warn if any formats are not handled */
      _mesa_problem(NULL, "bad format %s in _mesa_uncompressed_format_to_type_and_comps",
                    name ? name : "???");
      assert(format == MESA_FORMAT_NONE ||
             _mesa_is_format_compressed(format));
      *datatype = 0;
      *comps = 1;
   }
   }
}

/**
 * Check if a mesa_format exactly matches a GL format/type combination
 * such that we can use memcpy() from one to the other.
 * \param mesa_format  a MESA_FORMAT_x value
 * \param format  the user-specified image format
 * \param type  the user-specified image datatype
 * \param swapBytes  typically the current pixel pack/unpack byteswap state
 * \param[out] error GL_NO_ERROR if format is an expected input.
 *                   GL_INVALID_ENUM if format is an unexpected input.
 * \return true if the formats match, false otherwise.
 */
bool
_mesa_format_matches_format_and_type(mesa_format mformat,
				     GLenum format, GLenum type,
				     bool swapBytes, GLenum *error)
{
   if (error)
      *error = GL_NO_ERROR;

   if (_mesa_is_format_compressed(mformat)) {
      if (error)
         *error = GL_INVALID_ENUM;
      return false;
   }

   if (swapBytes && !_mesa_swap_bytes_in_type_enum(&type))
      return false;

   /* format/type don't include srgb and should match regardless of it. */
   mformat = _mesa_get_srgb_format_linear(mformat);

   /* intensity formats are uploaded with GL_RED, and we want to find
    * memcpy matches for them.
    */
   mformat = _mesa_get_intensity_format_red(mformat);

   if (format == GL_COLOR_INDEX)
      return false;

   mesa_format other_format = _mesa_format_from_format_and_type(format, type);
   if (_mesa_format_is_mesa_array_format(other_format))
      other_format = _mesa_format_from_array_format(other_format);

   return other_format == mformat;
}

