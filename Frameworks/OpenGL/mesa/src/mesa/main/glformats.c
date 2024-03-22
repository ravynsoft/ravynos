/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (c) 2008-2009  VMware, Inc.
 * Copyright (c) 2012 Intel Corporation
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
#include "glformats.h"
#include "formats.h"
#include "texcompress.h"
#include "enums.h"

enum {
   ZERO = 4,
   ONE = 5
};

enum {
   IDX_LUMINANCE = 0,
   IDX_ALPHA,
   IDX_INTENSITY,
   IDX_LUMINANCE_ALPHA,
   IDX_RGB,
   IDX_RGBA,
   IDX_RED,
   IDX_GREEN,
   IDX_BLUE,
   IDX_BGR,
   IDX_BGRA,
   IDX_ABGR,
   IDX_RG,
   MAX_IDX
};

#define MAP1(x)       MAP4(x, ZERO, ZERO, ZERO)
#define MAP2(x,y)     MAP4(x, y, ZERO, ZERO)
#define MAP3(x,y,z)   MAP4(x, y, z, ZERO)
#define MAP4(x,y,z,w) { x, y, z, w, ZERO, ONE }

static const struct {
   GLubyte format_idx;
   GLubyte to_rgba[6];
   GLubyte from_rgba[6];
} mappings[MAX_IDX] =
{
   {
      IDX_LUMINANCE,
      MAP4(0,0,0,ONE),
      MAP1(0)
   },

   {
      IDX_ALPHA,
      MAP4(ZERO, ZERO, ZERO, 0),
      MAP1(3)
   },

   {
      IDX_INTENSITY,
      MAP4(0, 0, 0, 0),
      MAP1(0),
   },

   {
      IDX_LUMINANCE_ALPHA,
      MAP4(0,0,0,1),
      MAP2(0,3)
   },

   {
      IDX_RGB,
      MAP4(0,1,2,ONE),
      MAP3(0,1,2)
   },

   {
      IDX_RGBA,
      MAP4(0,1,2,3),
      MAP4(0,1,2,3),
   },

   {
      IDX_RED,
      MAP4(0, ZERO, ZERO, ONE),
      MAP1(0),
   },

   {
      IDX_GREEN,
      MAP4(ZERO, 0, ZERO, ONE),
      MAP1(1),
   },

   {
      IDX_BLUE,
      MAP4(ZERO, ZERO, 0, ONE),
      MAP1(2),
   },

   {
      IDX_BGR,
      MAP4(2,1,0,ONE),
      MAP3(2,1,0)
   },

   {
      IDX_BGRA,
      MAP4(2,1,0,3),
      MAP4(2,1,0,3)
   },

   {
      IDX_ABGR,
      MAP4(3,2,1,0),
      MAP4(3,2,1,0)
   },

   {
      IDX_RG,
      MAP4(0, 1, ZERO, ONE),
      MAP2(0, 1)
   },
};

/**
 * Convert a GL image format enum to an IDX_* value (see above).
 */
static int
get_map_idx(GLenum value)
{
   switch (value) {
   case GL_LUMINANCE:
   case GL_LUMINANCE_INTEGER_EXT:
      return IDX_LUMINANCE;
   case GL_ALPHA:
   case GL_ALPHA_INTEGER:
      return IDX_ALPHA;
   case GL_INTENSITY:
      return IDX_INTENSITY;
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      return IDX_LUMINANCE_ALPHA;
   case GL_RGB:
   case GL_RGB_INTEGER:
      return IDX_RGB;
   case GL_RGBA:
   case GL_RGBA_INTEGER:
      return IDX_RGBA;
   case GL_RED:
   case GL_RED_INTEGER:
      return IDX_RED;
   case GL_GREEN:
      return IDX_GREEN;
   case GL_BLUE:
      return IDX_BLUE;
   case GL_BGR:
   case GL_BGR_INTEGER:
      return IDX_BGR;
   case GL_BGRA:
   case GL_BGRA_INTEGER:
      return IDX_BGRA;
   case GL_ABGR_EXT:
      return IDX_ABGR;
   case GL_RG:
   case GL_RG_INTEGER:
      return IDX_RG;
   default:
      _mesa_problem(NULL, "Unexpected inFormat %s",
                    _mesa_enum_to_string(value));
      return 0;
   }
}

/**
 * When promoting texture formats (see below) we need to compute the
 * mapping of dest components back to source components.
 * This function does that.
 * \param inFormat  the incoming format of the texture
 * \param outFormat  the final texture format
 * \return map[6]  a full 6-component map
 */
void
_mesa_compute_component_mapping(GLenum inFormat, GLenum outFormat, GLubyte *map)
{
   const int inFmt = get_map_idx(inFormat);
   const int outFmt = get_map_idx(outFormat);
   const GLubyte *in2rgba = mappings[inFmt].to_rgba;
   const GLubyte *rgba2out = mappings[outFmt].from_rgba;
   int i;

   for (i = 0; i < 4; i++)
      map[i] = in2rgba[rgba2out[i]];

   map[ZERO] = ZERO;
   map[ONE] = ONE;

#if 0
   printf("from %x/%s to %x/%s map %d %d %d %d %d %d\n",
	  inFormat, _mesa_enum_to_string(inFormat),
	  outFormat, _mesa_enum_to_string(outFormat),
	  map[0],
	  map[1],
	  map[2],
	  map[3],
	  map[4],
	  map[5]);
#endif
}

/**
 * \return GL_TRUE if type is packed pixel type, GL_FALSE otherwise.
 */
GLboolean
_mesa_type_is_packed(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case MESA_UNSIGNED_BYTE_4_4:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
   case GL_UNSIGNED_INT_24_8_EXT:
   case GL_UNSIGNED_INT_5_9_9_9_REV:
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      return GL_TRUE;
   }

   return GL_FALSE;
}


/**
 * Get the size of a GL data type.
 *
 * \param type GL data type.
 *
 * \return the size, in bytes, of the given data type, 0 if a GL_BITMAP, or -1
 * if an invalid type enum.
 */
GLint
_mesa_sizeof_type(GLenum type)
{
   switch (type) {
   case GL_BITMAP:
      return 0;
   case GL_UNSIGNED_BYTE:
      return sizeof(GLubyte);
   case GL_BYTE:
      return sizeof(GLbyte);
   case GL_UNSIGNED_SHORT:
      return sizeof(GLushort);
   case GL_SHORT:
      return sizeof(GLshort);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   case GL_INT:
      return sizeof(GLint);
   case GL_FLOAT:
      return sizeof(GLfloat);
   case GL_DOUBLE:
      return sizeof(GLdouble);
   case GL_HALF_FLOAT_ARB:
   case GL_HALF_FLOAT_OES:
      return sizeof(GLhalfARB);
   case GL_FIXED:
      return sizeof(GLfixed);
   default:
      return -1;
   }
}


/**
 * Same as _mesa_sizeof_type() but also accepting the packed pixel
 * format data types.
 */
GLint
_mesa_sizeof_packed_type(GLenum type)
{
   switch (type) {
   case GL_BITMAP:
      return 0;
   case GL_UNSIGNED_BYTE:
      return sizeof(GLubyte);
   case GL_BYTE:
      return sizeof(GLbyte);
   case GL_UNSIGNED_SHORT:
      return sizeof(GLushort);
   case GL_SHORT:
      return sizeof(GLshort);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   case GL_INT:
      return sizeof(GLint);
   case GL_HALF_FLOAT_ARB:
   case GL_HALF_FLOAT_OES:
      return sizeof(GLhalfARB);
   case GL_FLOAT:
      return sizeof(GLfloat);
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case MESA_UNSIGNED_BYTE_4_4:
      return sizeof(GLubyte);
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      return sizeof(GLushort);
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
   case GL_UNSIGNED_INT_24_8_EXT:
   case GL_UNSIGNED_INT_5_9_9_9_REV:
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      return sizeof(GLuint);
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      return 8;
   default:
      return -1;
   }
}


/**
 * Get the number of components in a pixel format.
 *
 * \param format pixel format.
 *
 * \return the number of components in the given format, or -1 if a bad format.
 */
GLint
_mesa_components_in_format(GLenum format)
{
   switch (format) {
   case GL_COLOR_INDEX:
   case GL_STENCIL_INDEX:
   case GL_DEPTH_COMPONENT:
   case GL_RED:
   case GL_RED_INTEGER_EXT:
   case GL_GREEN:
   case GL_GREEN_INTEGER_EXT:
   case GL_BLUE:
   case GL_BLUE_INTEGER_EXT:
   case GL_ALPHA:
   case GL_ALPHA_INTEGER_EXT:
   case GL_LUMINANCE:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_INTENSITY:
      return 1;

   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
   case GL_RG:
   case GL_YCBCR_MESA:
   case GL_DEPTH_STENCIL_EXT:
   case GL_RG_INTEGER:
      return 2;

   case GL_RGB:
   case GL_BGR:
   case GL_RGB_INTEGER_EXT:
   case GL_BGR_INTEGER_EXT:
      return 3;

   case GL_RGBA:
   case GL_BGRA:
   case GL_ABGR_EXT:
   case GL_RGBA_INTEGER_EXT:
   case GL_BGRA_INTEGER_EXT:
      return 4;

   default:
      return -1;
   }
}


/**
 * Get the bytes per pixel of pixel format type pair.
 *
 * \param format pixel format.
 * \param type pixel type.
 *
 * \return bytes per pixel, or -1 if a bad format or type was given.
 */
GLint
_mesa_bytes_per_pixel(GLenum format, GLenum type)
{
   GLint comps = _mesa_components_in_format(format);
   if (comps < 0)
      return -1;

   switch (type) {
   case GL_BITMAP:
      return 0;  /* special case */
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return comps * sizeof(GLubyte);
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
      return comps * sizeof(GLshort);
   case GL_INT:
   case GL_UNSIGNED_INT:
      return comps * sizeof(GLint);
   case GL_FLOAT:
      return comps * sizeof(GLfloat);
   case GL_HALF_FLOAT_ARB:
   case GL_HALF_FLOAT_OES:
      return comps * sizeof(GLhalfARB);
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
      if (format == GL_RGB || format == GL_BGR ||
          format == GL_RGB_INTEGER_EXT || format == GL_BGR_INTEGER_EXT)
         return sizeof(GLubyte);
      else
         return -1;  /* error */
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
      if (format == GL_RGB || format == GL_BGR ||
          format == GL_RGB_INTEGER_EXT || format == GL_BGR_INTEGER_EXT)
         return sizeof(GLushort);
      else
         return -1;  /* error */
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
      if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT)
         return sizeof(GLushort);
      else
         return -1;
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      if (format == GL_RGBA || format == GL_BGRA ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT)
         return sizeof(GLushort);
      else
         return -1;
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
      if (format == GL_RGBA || format == GL_BGRA || format == GL_ABGR_EXT ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT ||
          format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (format == GL_RGBA || format == GL_BGRA ||
          format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT ||
          format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      if (format == GL_YCBCR_MESA)
         return sizeof(GLushort);
      else
         return -1;
   case GL_UNSIGNED_INT_24_8_EXT:
      if (format == GL_DEPTH_COMPONENT ||
          format == GL_DEPTH_STENCIL_EXT)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_5_9_9_9_REV:
      if (format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (format == GL_RGB)
         return sizeof(GLuint);
      else
         return -1;
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (format == GL_DEPTH_STENCIL)
         return 8;
      else
         return -1;
   default:
      return -1;
   }
}


/**
 * Test if the given format is unsized.
 */
GLboolean
_mesa_is_enum_format_unsized(GLenum format)
{
   switch (format) {
   case GL_RGBA:
   case GL_BGRA:
   case GL_ABGR_EXT:
   case GL_RGB:
   case GL_BGR:
   case GL_RG:
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_ALPHA:
   case GL_INTENSITY:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:

   case GL_SRGB:
   case GL_SRGB_ALPHA:
   case GL_SLUMINANCE:
   case GL_SLUMINANCE_ALPHA:

   case GL_RGBA_SNORM:
   case GL_RGB_SNORM:
   case GL_RG_SNORM:
   case GL_RED_SNORM:
   case GL_ALPHA_SNORM:
   case GL_INTENSITY_SNORM:
   case GL_LUMINANCE_SNORM:
   case GL_LUMINANCE_ALPHA_SNORM:

   case GL_RED_INTEGER:
   case GL_GREEN_INTEGER:
   case GL_BLUE_INTEGER:
   case GL_ALPHA_INTEGER:
   case GL_RGB_INTEGER:
   case GL_RGBA_INTEGER:
   case GL_BGR_INTEGER:
   case GL_BGRA_INTEGER:
   case GL_RG_INTEGER:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:

   case GL_DEPTH_COMPONENT:
   case GL_DEPTH_STENCIL:
   case GL_STENCIL_INDEX:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format is a UNORM (unsigned-normalized) format.
 */
GLboolean
_mesa_is_enum_format_unorm(GLenum format)
{
      switch(format) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
      case 1:
      case GL_LUMINANCE:
      case GL_SLUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
      case 2:
      case GL_LUMINANCE_ALPHA:
      case GL_SLUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
      case GL_R8:
      case GL_R16:
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case 3:
      case GL_RGB:
      case GL_BGR:
      case GL_SRGB:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB565:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
      case 4:
      case GL_ABGR_EXT:
      case GL_RGBA:
      case GL_BGRA:
      case GL_SRGB_ALPHA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}

/**
 * Test if the given format is a SNORM (signed-normalized) format.
 */
GLboolean
_mesa_is_enum_format_snorm(GLenum format)
{
   switch (format) {
   /* signed, normalized texture formats */
   case GL_RED_SNORM:
   case GL_R8_SNORM:
   case GL_R16_SNORM:
   case GL_RG_SNORM:
   case GL_RG8_SNORM:
   case GL_RG16_SNORM:
   case GL_RGB_SNORM:
   case GL_RGB8_SNORM:
   case GL_RGB16_SNORM:
   case GL_RGBA_SNORM:
   case GL_RGBA8_SNORM:
   case GL_RGBA16_SNORM:
   case GL_ALPHA_SNORM:
   case GL_ALPHA8_SNORM:
   case GL_ALPHA16_SNORM:
   case GL_LUMINANCE_SNORM:
   case GL_LUMINANCE8_SNORM:
   case GL_LUMINANCE16_SNORM:
   case GL_LUMINANCE_ALPHA_SNORM:
   case GL_LUMINANCE8_ALPHA8_SNORM:
   case GL_LUMINANCE16_ALPHA16_SNORM:
   case GL_INTENSITY_SNORM:
   case GL_INTENSITY8_SNORM:
   case GL_INTENSITY16_SNORM:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_unsigned_int(GLenum format)
{
   switch (format) {
   /* specific integer formats */
   case GL_RGBA32UI_EXT:
   case GL_RGB32UI_EXT:
   case GL_RG32UI:
   case GL_R32UI:
   case GL_ALPHA32UI_EXT:
   case GL_INTENSITY32UI_EXT:
   case GL_LUMINANCE32UI_EXT:
   case GL_LUMINANCE_ALPHA32UI_EXT:
   case GL_RGBA16UI_EXT:
   case GL_RGB16UI_EXT:
   case GL_RG16UI:
   case GL_R16UI:
   case GL_ALPHA16UI_EXT:
   case GL_INTENSITY16UI_EXT:
   case GL_LUMINANCE16UI_EXT:
   case GL_LUMINANCE_ALPHA16UI_EXT:
   case GL_RGBA8UI_EXT:
   case GL_RGB8UI_EXT:
   case GL_RG8UI:
   case GL_R8UI:
   case GL_ALPHA8UI_EXT:
   case GL_INTENSITY8UI_EXT:
   case GL_LUMINANCE8UI_EXT:
   case GL_LUMINANCE_ALPHA8UI_EXT:
   case GL_RGB10_A2UI:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}


/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_signed_int(GLenum format)
{
   switch (format) {
   /* generic integer formats */
   case GL_RED_INTEGER_EXT:
   case GL_GREEN_INTEGER_EXT:
   case GL_BLUE_INTEGER_EXT:
   case GL_ALPHA_INTEGER_EXT:
   case GL_RGB_INTEGER_EXT:
   case GL_RGBA_INTEGER_EXT:
   case GL_BGR_INTEGER_EXT:
   case GL_BGRA_INTEGER_EXT:
   case GL_LUMINANCE_INTEGER_EXT:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
   case GL_RG_INTEGER:
   /* specific integer formats */
   case GL_RGBA32I_EXT:
   case GL_RGB32I_EXT:
   case GL_RG32I:
   case GL_R32I:
   case GL_ALPHA32I_EXT:
   case GL_INTENSITY32I_EXT:
   case GL_LUMINANCE32I_EXT:
   case GL_LUMINANCE_ALPHA32I_EXT:
   case GL_RGBA16I_EXT:
   case GL_RGB16I_EXT:
   case GL_RG16I:
   case GL_R16I:
   case GL_ALPHA16I_EXT:
   case GL_INTENSITY16I_EXT:
   case GL_LUMINANCE16I_EXT:
   case GL_LUMINANCE_ALPHA16I_EXT:
   case GL_RGBA8I_EXT:
   case GL_RGB8I_EXT:
   case GL_RG8I:
   case GL_R8I:
   case GL_ALPHA8I_EXT:
   case GL_INTENSITY8I_EXT:
   case GL_LUMINANCE8I_EXT:
   case GL_LUMINANCE_ALPHA8I_EXT:
      return GL_TRUE;
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format is an ASTC 2D format.
 */
static bool
is_astc_2d_format(GLenum internalFormat)
{
   switch (internalFormat) {
   case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
   case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
   case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
   case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
   case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
   case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
   case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
   case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
   case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
   case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
   case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
   case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
   case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
   case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
      return true;
   default:
      return false;
   }
}

/**
 * Test if the given format is an ASTC 3D format.
 */
static bool
is_astc_3d_format(GLenum internalFormat)
{
   switch (internalFormat) {
   case GL_COMPRESSED_RGBA_ASTC_3x3x3_OES:
   case GL_COMPRESSED_RGBA_ASTC_4x3x3_OES:
   case GL_COMPRESSED_RGBA_ASTC_4x4x3_OES:
   case GL_COMPRESSED_RGBA_ASTC_4x4x4_OES:
   case GL_COMPRESSED_RGBA_ASTC_5x4x4_OES:
   case GL_COMPRESSED_RGBA_ASTC_5x5x4_OES:
   case GL_COMPRESSED_RGBA_ASTC_5x5x5_OES:
   case GL_COMPRESSED_RGBA_ASTC_6x5x5_OES:
   case GL_COMPRESSED_RGBA_ASTC_6x6x5_OES:
   case GL_COMPRESSED_RGBA_ASTC_6x6x6_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES:
      return true;
   default:
      return false;
   }
}

/**
 * Test if the given format is an ASTC format.
 */
GLboolean
_mesa_is_astc_format(GLenum internalFormat)
{
   return is_astc_2d_format(internalFormat) ||
          is_astc_3d_format(internalFormat);
}

/**
 * Test if the given format is an ETC2 format.
 */
GLboolean
_mesa_is_etc2_format(GLenum internalFormat)
{
   switch (internalFormat) {
   case GL_COMPRESSED_RGB8_ETC2:
   case GL_COMPRESSED_SRGB8_ETC2:
   case GL_COMPRESSED_RGBA8_ETC2_EAC:
   case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
   case GL_COMPRESSED_R11_EAC:
   case GL_COMPRESSED_RG11_EAC:
   case GL_COMPRESSED_SIGNED_R11_EAC:
   case GL_COMPRESSED_SIGNED_RG11_EAC:
   case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      return true;
   default:
      return false;
   }
}

/**
 * Test if the given format is an integer (non-normalized) format.
 */
GLboolean
_mesa_is_enum_format_integer(GLenum format)
{
   return _mesa_is_enum_format_unsigned_int(format) ||
          _mesa_is_enum_format_signed_int(format);
}


GLboolean
_mesa_is_type_unsigned(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_INT:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:

   case GL_UNSIGNED_SHORT:
   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_SHORT_8_8_MESA:
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:

   case GL_UNSIGNED_BYTE:
   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
      return GL_TRUE;

   default:
      return GL_FALSE;
   }
}


/**
 * Test if the given image format is a color/RGBA format (i.e., not color
 * index, depth, stencil, etc).
 * \param format  the image format value (may by an internal texture format)
 * \return GL_TRUE if its a color/RGBA format, GL_FALSE otherwise.
 */
GLboolean
_mesa_is_color_format(GLenum format)
{
   switch (format) {
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_ALPHA4:
      case GL_ALPHA8:
      case GL_ALPHA12:
      case GL_ALPHA16:
      case 1:
      case GL_LUMINANCE:
      case GL_LUMINANCE4:
      case GL_LUMINANCE8:
      case GL_LUMINANCE12:
      case GL_LUMINANCE16:
      case 2:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:
      case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:
      case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:
      case GL_LUMINANCE16_ALPHA16:
      case GL_INTENSITY:
      case GL_INTENSITY4:
      case GL_INTENSITY8:
      case GL_INTENSITY12:
      case GL_INTENSITY16:
      case GL_R8:
      case GL_R16:
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case 3:
      case GL_RGB:
      case GL_BGR:
      case GL_R3_G3_B2:
      case GL_RGB4:
      case GL_RGB5:
      case GL_RGB565:
      case GL_RGB8:
      case GL_RGB10:
      case GL_RGB12:
      case GL_RGB16:
      case 4:
      case GL_ABGR_EXT:
      case GL_RGBA:
      case GL_BGRA:
      case GL_RGBA2:
      case GL_RGBA4:
      case GL_RGB5_A1:
      case GL_RGBA8:
      case GL_RGB10_A2:
      case GL_RGBA12:
      case GL_RGBA16:
      /* float texture formats */
      case GL_ALPHA16F_ARB:
      case GL_ALPHA32F_ARB:
      case GL_LUMINANCE16F_ARB:
      case GL_LUMINANCE32F_ARB:
      case GL_LUMINANCE_ALPHA16F_ARB:
      case GL_LUMINANCE_ALPHA32F_ARB:
      case GL_INTENSITY16F_ARB:
      case GL_INTENSITY32F_ARB:
      case GL_R16F:
      case GL_R32F:
      case GL_RG16F:
      case GL_RG32F:
      case GL_RGB16F_ARB:
      case GL_RGB32F_ARB:
      case GL_RGBA16F_ARB:
      case GL_RGBA32F_ARB:
      /* compressed formats */
      case GL_COMPRESSED_ALPHA:
      case GL_COMPRESSED_LUMINANCE:
      case GL_COMPRESSED_LUMINANCE_ALPHA:
      case GL_COMPRESSED_INTENSITY:
      case GL_COMPRESSED_RED:
      case GL_COMPRESSED_RG:
      case GL_COMPRESSED_RGB:
      case GL_COMPRESSED_RGBA:
      case GL_RGB_S3TC:
      case GL_RGB4_S3TC:
      case GL_RGBA_S3TC:
      case GL_RGBA4_S3TC:
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_RGB_FXT1_3DFX:
      case GL_COMPRESSED_RGBA_FXT1_3DFX:
      case GL_SR8_EXT:
      case GL_SRG8_EXT:
      case GL_SRGB_EXT:
      case GL_SRGB8_EXT:
      case GL_SRGB_ALPHA_EXT:
      case GL_SRGB8_ALPHA8_EXT:
      case GL_SLUMINANCE_ALPHA_EXT:
      case GL_SLUMINANCE8_ALPHA8_EXT:
      case GL_SLUMINANCE_EXT:
      case GL_SLUMINANCE8_EXT:
      case GL_COMPRESSED_SRGB_EXT:
      case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      case GL_COMPRESSED_SLUMINANCE_EXT:
      case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
      case GL_COMPRESSED_RED_RGTC1:
      case GL_COMPRESSED_SIGNED_RED_RGTC1:
      case GL_COMPRESSED_RG_RGTC2:
      case GL_COMPRESSED_SIGNED_RG_RGTC2:
      case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
      case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
      case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      case GL_ETC1_RGB8_OES:
      case GL_COMPRESSED_RGB8_ETC2:
      case GL_COMPRESSED_SRGB8_ETC2:
      case GL_COMPRESSED_RGBA8_ETC2_EAC:
      case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
      case GL_COMPRESSED_R11_EAC:
      case GL_COMPRESSED_RG11_EAC:
      case GL_COMPRESSED_SIGNED_R11_EAC:
      case GL_COMPRESSED_SIGNED_RG11_EAC:
      case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
      case GL_COMPRESSED_RGBA_BPTC_UNORM:
      case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
      case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
      case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      case GL_COMPRESSED_RGBA_ASTC_4x4_KHR:
      case GL_COMPRESSED_RGBA_ASTC_5x4_KHR:
      case GL_COMPRESSED_RGBA_ASTC_5x5_KHR:
      case GL_COMPRESSED_RGBA_ASTC_6x5_KHR:
      case GL_COMPRESSED_RGBA_ASTC_6x6_KHR:
      case GL_COMPRESSED_RGBA_ASTC_8x5_KHR:
      case GL_COMPRESSED_RGBA_ASTC_8x6_KHR:
      case GL_COMPRESSED_RGBA_ASTC_8x8_KHR:
      case GL_COMPRESSED_RGBA_ASTC_10x5_KHR:
      case GL_COMPRESSED_RGBA_ASTC_10x6_KHR:
      case GL_COMPRESSED_RGBA_ASTC_10x8_KHR:
      case GL_COMPRESSED_RGBA_ASTC_10x10_KHR:
      case GL_COMPRESSED_RGBA_ASTC_12x10_KHR:
      case GL_COMPRESSED_RGBA_ASTC_12x12_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
      case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
      case GL_ATC_RGB_AMD:
      case GL_ATC_RGBA_EXPLICIT_ALPHA_AMD:
      case GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD:
      /* generic integer formats */
      case GL_RED_INTEGER_EXT:
      case GL_GREEN_INTEGER_EXT:
      case GL_BLUE_INTEGER_EXT:
      case GL_ALPHA_INTEGER_EXT:
      case GL_RGB_INTEGER_EXT:
      case GL_RGBA_INTEGER_EXT:
      case GL_BGR_INTEGER_EXT:
      case GL_BGRA_INTEGER_EXT:
      case GL_RG_INTEGER:
      case GL_LUMINANCE_INTEGER_EXT:
      case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      /* sized integer formats */
      case GL_RGBA32UI_EXT:
      case GL_RGB32UI_EXT:
      case GL_RG32UI:
      case GL_R32UI:
      case GL_ALPHA32UI_EXT:
      case GL_INTENSITY32UI_EXT:
      case GL_LUMINANCE32UI_EXT:
      case GL_LUMINANCE_ALPHA32UI_EXT:
      case GL_RGBA16UI_EXT:
      case GL_RGB16UI_EXT:
      case GL_RG16UI:
      case GL_R16UI:
      case GL_ALPHA16UI_EXT:
      case GL_INTENSITY16UI_EXT:
      case GL_LUMINANCE16UI_EXT:
      case GL_LUMINANCE_ALPHA16UI_EXT:
      case GL_RGBA8UI_EXT:
      case GL_RGB8UI_EXT:
      case GL_RG8UI:
      case GL_R8UI:
      case GL_ALPHA8UI_EXT:
      case GL_INTENSITY8UI_EXT:
      case GL_LUMINANCE8UI_EXT:
      case GL_LUMINANCE_ALPHA8UI_EXT:
      case GL_RGBA32I_EXT:
      case GL_RGB32I_EXT:
      case GL_RG32I:
      case GL_R32I:
      case GL_ALPHA32I_EXT:
      case GL_INTENSITY32I_EXT:
      case GL_LUMINANCE32I_EXT:
      case GL_LUMINANCE_ALPHA32I_EXT:
      case GL_RGBA16I_EXT:
      case GL_RGB16I_EXT:
      case GL_RG16I:
      case GL_R16I:
      case GL_ALPHA16I_EXT:
      case GL_INTENSITY16I_EXT:
      case GL_LUMINANCE16I_EXT:
      case GL_LUMINANCE_ALPHA16I_EXT:
      case GL_RGBA8I_EXT:
      case GL_RGB8I_EXT:
      case GL_RG8I:
      case GL_R8I:
      case GL_ALPHA8I_EXT:
      case GL_INTENSITY8I_EXT:
      case GL_LUMINANCE8I_EXT:
      case GL_LUMINANCE_ALPHA8I_EXT:
      /* signed, normalized texture formats */
      case GL_RED_SNORM:
      case GL_R8_SNORM:
      case GL_R16_SNORM:
      case GL_RG_SNORM:
      case GL_RG8_SNORM:
      case GL_RG16_SNORM:
      case GL_RGB_SNORM:
      case GL_RGB8_SNORM:
      case GL_RGB16_SNORM:
      case GL_RGBA_SNORM:
      case GL_RGBA8_SNORM:
      case GL_RGBA16_SNORM:
      case GL_ALPHA_SNORM:
      case GL_ALPHA8_SNORM:
      case GL_ALPHA16_SNORM:
      case GL_LUMINANCE_SNORM:
      case GL_LUMINANCE8_SNORM:
      case GL_LUMINANCE16_SNORM:
      case GL_LUMINANCE_ALPHA_SNORM:
      case GL_LUMINANCE8_ALPHA8_SNORM:
      case GL_LUMINANCE16_ALPHA16_SNORM:
      case GL_INTENSITY_SNORM:
      case GL_INTENSITY8_SNORM:
      case GL_INTENSITY16_SNORM:
      case GL_RGB9_E5:
      case GL_R11F_G11F_B10F:
      case GL_RGB10_A2UI:
         return GL_TRUE;
      case GL_YCBCR_MESA:  /* not considered to be RGB */
         FALLTHROUGH;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth component format.
 */
GLboolean
_mesa_is_depth_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_DEPTH_COMPONENT32F:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a stencil format.
 */
GLboolean
_mesa_is_stencil_format(GLenum format)
{
   switch (format) {
      case GL_STENCIL_INDEX:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a YCbCr format.
 */
GLboolean
_mesa_is_ycbcr_format(GLenum format)
{
   switch (format) {
      case GL_YCBCR_MESA:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth+stencil format.
 */
GLboolean
_mesa_is_depthstencil_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH24_STENCIL8_EXT:
      case GL_DEPTH_STENCIL_EXT:
      case GL_DEPTH32F_STENCIL8:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}


/**
 * Test if the given image format is a depth or stencil format.
 */
GLboolean
_mesa_is_depth_or_stencil_format(GLenum format)
{
   switch (format) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
      case GL_STENCIL_INDEX:
      case GL_STENCIL_INDEX1_EXT:
      case GL_STENCIL_INDEX4_EXT:
      case GL_STENCIL_INDEX8_EXT:
      case GL_STENCIL_INDEX16_EXT:
      case GL_DEPTH_STENCIL_EXT:
      case GL_DEPTH24_STENCIL8_EXT:
      case GL_DEPTH_COMPONENT32F:
      case GL_DEPTH32F_STENCIL8:
         return GL_TRUE;
      default:
         return GL_FALSE;
   }
}

/**
 * Test if the given image format has a floating-point depth component.
 */
GLboolean
_mesa_has_depth_float_channel(GLenum internalFormat)
{
   return internalFormat == GL_DEPTH32F_STENCIL8 ||
          internalFormat == GL_DEPTH_COMPONENT32F;
}

/**
 * Test if an image format is a supported compressed format.
 * \param format the internal format token provided by the user.
 * \return GL_TRUE if compressed, GL_FALSE if uncompressed
 */
GLboolean
_mesa_is_compressed_format(const struct gl_context *ctx, GLenum format)
{
   mesa_format m_format = _mesa_glenum_to_compressed_format(format);

   /* Some formats in this switch have an equivalent mesa_format_layout
    * to the compressed formats in the layout switch below and thus
    * must be handled first.
    */
   switch (format) {
   case GL_RGB_S3TC:
   case GL_RGB4_S3TC:
   case GL_RGBA_S3TC:
   case GL_RGBA4_S3TC:
      return _mesa_has_S3_s3tc(ctx);
   case GL_COMPRESSED_LUMINANCE_ALPHA_3DC_ATI:
      return _mesa_has_ATI_texture_compression_3dc(ctx);
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
      return _mesa_is_gles1(ctx);
   }

   switch (_mesa_get_format_layout(m_format)) {
   case MESA_FORMAT_LAYOUT_S3TC:
      if (!_mesa_is_format_srgb(m_format)) {
         return _mesa_has_EXT_texture_compression_s3tc(ctx);
      } else {
         return (_mesa_has_EXT_texture_sRGB(ctx) ||
            _mesa_has_EXT_texture_compression_s3tc_srgb(ctx)) &&
            _mesa_has_EXT_texture_compression_s3tc(ctx);
      }
   case MESA_FORMAT_LAYOUT_FXT1:
      return _mesa_has_3DFX_texture_compression_FXT1(ctx);
   case MESA_FORMAT_LAYOUT_RGTC:
      return _mesa_has_ARB_texture_compression_rgtc(ctx) ||
             _mesa_has_EXT_texture_compression_rgtc(ctx);
   case MESA_FORMAT_LAYOUT_LATC:
      return _mesa_has_EXT_texture_compression_latc(ctx);
   case MESA_FORMAT_LAYOUT_ETC1:
      return _mesa_has_OES_compressed_ETC1_RGB8_texture(ctx);
   case MESA_FORMAT_LAYOUT_ETC2:
      return _mesa_is_gles3(ctx) || _mesa_has_ARB_ES3_compatibility(ctx);
   case MESA_FORMAT_LAYOUT_BPTC:
      return _mesa_has_ARB_texture_compression_bptc(ctx) ||
             _mesa_has_EXT_texture_compression_bptc(ctx);
   case MESA_FORMAT_LAYOUT_ASTC:
      return _mesa_has_KHR_texture_compression_astc_ldr(ctx);
   case MESA_FORMAT_LAYOUT_ATC:
      return _mesa_has_AMD_compressed_ATC_texture(ctx);
   default:
      return GL_FALSE;
   }
}

/**
 * Test if the given format represents an sRGB format.
 * \param format the GL format (can be an internal format)
 * \return GL_TRUE if format is sRGB, GL_FALSE otherwise
 */
GLboolean
_mesa_is_srgb_format(GLenum format)
{
   switch (format) {
   case GL_SR8_EXT:
   case GL_SRG8_EXT:
   case GL_SRGB:
   case GL_SRGB8:
   case GL_SRGB_ALPHA:
   case GL_SRGB8_ALPHA8:
   case GL_COMPRESSED_SRGB:
   case GL_COMPRESSED_SRGB_ALPHA:
   case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
   case GL_COMPRESSED_SRGB8_ETC2:
   case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
   case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:
   case GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:
      return GL_TRUE;
   default:
      break;
   }

   return GL_FALSE;
}

/**
 * Convert various unpack formats to the corresponding base format.
 */
GLenum
_mesa_unpack_format_to_base_format(GLenum format)
{
   switch(format) {
   case GL_RED_INTEGER:
      return GL_RED;
   case GL_GREEN_INTEGER:
      return GL_GREEN;
   case GL_BLUE_INTEGER:
      return GL_BLUE;
   case GL_ALPHA_INTEGER:
      return GL_ALPHA;
   case GL_RG_INTEGER:
      return GL_RG;
   case GL_RGB_INTEGER:
      return GL_RGB;
   case GL_RGBA_INTEGER:
      return GL_RGBA;
   case GL_BGR_INTEGER:
      return GL_BGR;
   case GL_BGRA_INTEGER:
      return GL_BGRA;
   case GL_LUMINANCE_INTEGER_EXT:
      return GL_LUMINANCE;
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      return GL_LUMINANCE_ALPHA;
   case GL_RED:
   case GL_GREEN:
   case GL_BLUE:
   case GL_RG:
   case GL_RGB:
   case GL_RGBA:
   case GL_BGR:
   case GL_BGRA:
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
   default:
      return format;
   }
}

/**
 * Convert various base formats to the corresponding integer format.
 */
GLenum
_mesa_base_format_to_integer_format(GLenum format)
{
   switch(format) {
   case GL_RED:
      return GL_RED_INTEGER;
   case GL_GREEN:
      return GL_GREEN_INTEGER;
   case GL_BLUE:
      return GL_BLUE_INTEGER;
   case GL_RG:
      return GL_RG_INTEGER;
   case GL_RGB:
      return GL_RGB_INTEGER;
   case GL_RGBA:
      return GL_RGBA_INTEGER;
   case GL_BGR:
      return GL_BGR_INTEGER;
   case GL_BGRA:
      return GL_BGRA_INTEGER;
   case GL_ALPHA:
      return GL_ALPHA_INTEGER;
   case GL_LUMINANCE:
      return GL_LUMINANCE_INTEGER_EXT;
   case GL_LUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA_INTEGER_EXT;
   }

   return format;
}


/**
 * Does the given base texture/renderbuffer format have the channel
 * named by 'pname'?
 */
GLboolean
_mesa_base_format_has_channel(GLenum base_format, GLenum pname)
{
   switch (pname) {
   case GL_TEXTURE_RED_SIZE:
   case GL_TEXTURE_RED_TYPE:
   case GL_RENDERBUFFER_RED_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
   case GL_INTERNALFORMAT_RED_SIZE:
   case GL_INTERNALFORMAT_RED_TYPE:
      if (base_format == GL_RED ||
	  base_format == GL_RG ||
	  base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_GREEN_SIZE:
   case GL_TEXTURE_GREEN_TYPE:
   case GL_RENDERBUFFER_GREEN_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
   case GL_INTERNALFORMAT_GREEN_SIZE:
   case GL_INTERNALFORMAT_GREEN_TYPE:
      if (base_format == GL_RG ||
	  base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_BLUE_SIZE:
   case GL_TEXTURE_BLUE_TYPE:
   case GL_RENDERBUFFER_BLUE_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
   case GL_INTERNALFORMAT_BLUE_SIZE:
   case GL_INTERNALFORMAT_BLUE_TYPE:
      if (base_format == GL_RGB ||
	  base_format == GL_RGBA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_ALPHA_SIZE:
   case GL_TEXTURE_ALPHA_TYPE:
   case GL_RENDERBUFFER_ALPHA_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
   case GL_INTERNALFORMAT_ALPHA_SIZE:
   case GL_INTERNALFORMAT_ALPHA_TYPE:
      if (base_format == GL_RGBA ||
	  base_format == GL_ALPHA ||
	  base_format == GL_LUMINANCE_ALPHA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_LUMINANCE_SIZE:
   case GL_TEXTURE_LUMINANCE_TYPE:
      if (base_format == GL_LUMINANCE ||
	  base_format == GL_LUMINANCE_ALPHA) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_INTENSITY_SIZE:
   case GL_TEXTURE_INTENSITY_TYPE:
      if (base_format == GL_INTENSITY) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_TEXTURE_DEPTH_SIZE:
   case GL_TEXTURE_DEPTH_TYPE:
   case GL_RENDERBUFFER_DEPTH_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
   case GL_INTERNALFORMAT_DEPTH_SIZE:
   case GL_INTERNALFORMAT_DEPTH_TYPE:
      if (base_format == GL_DEPTH_STENCIL ||
	  base_format == GL_DEPTH_COMPONENT) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   case GL_RENDERBUFFER_STENCIL_SIZE_EXT:
   case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
   case GL_INTERNALFORMAT_STENCIL_SIZE:
   case GL_INTERNALFORMAT_STENCIL_TYPE:
      if (base_format == GL_DEPTH_STENCIL ||
	  base_format == GL_STENCIL_INDEX) {
	 return GL_TRUE;
      }
      return GL_FALSE;
   default:
      _mesa_warning(NULL, "%s: Unexpected channel token 0x%x\n",
		    __func__, pname);
      return GL_FALSE;
   }

   return GL_FALSE;
}


/**
 * If format is a generic compressed format, return the corresponding
 * non-compressed format.  For other formats, return the format as-is.
 */
GLenum
_mesa_generic_compressed_format_to_uncompressed_format(GLenum format)
{
   switch (format) {
   case GL_COMPRESSED_RED:
      return GL_RED;
   case GL_COMPRESSED_RG:
      return GL_RG;
   case GL_COMPRESSED_RGB:
      return GL_RGB;
   case GL_COMPRESSED_RGBA:
      return GL_RGBA;
   case GL_COMPRESSED_ALPHA:
      return GL_ALPHA;
   case GL_COMPRESSED_LUMINANCE:
      return GL_LUMINANCE;
   case GL_COMPRESSED_LUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA;
   case GL_COMPRESSED_INTENSITY:
      return GL_INTENSITY;
   /* sRGB formats */
   case GL_COMPRESSED_SRGB:
      return GL_SRGB;
   case GL_COMPRESSED_SRGB_ALPHA:
      return GL_SRGB_ALPHA;
   case GL_COMPRESSED_SLUMINANCE:
      return GL_SLUMINANCE;
   case GL_COMPRESSED_SLUMINANCE_ALPHA:
      return GL_SLUMINANCE_ALPHA;
   default:
      return format;
   }
}


/**
 * Return the equivalent non-generic internal format.
 * This is useful for comparing whether two internal formats are equivalent.
 */
GLenum
_mesa_get_nongeneric_internalformat(GLenum format)
{
   switch (format) {
   /* GL 1.1 formats. */
   case 4:
   case GL_RGBA:
      return GL_RGBA8;
   case 3:
   case GL_RGB:
      return GL_RGB8;
   case 2:
   case GL_LUMINANCE_ALPHA:
      return GL_LUMINANCE8_ALPHA8;
   case 1:
   case GL_LUMINANCE:
      return GL_LUMINANCE8;
   case GL_ALPHA:
      return GL_ALPHA8;
   case GL_INTENSITY:
      return GL_INTENSITY8;

   /* GL_ARB_texture_rg */
   case GL_RED:
      return GL_R8;
   case GL_RG:
      return GL_RG8;

   /* GL_EXT_texture_sRGB */
   case GL_SRGB:
      return GL_SRGB8;
   case GL_SRGB_ALPHA:
      return GL_SRGB8_ALPHA8;
   case GL_SLUMINANCE:
      return GL_SLUMINANCE8;
   case GL_SLUMINANCE_ALPHA:
      return GL_SLUMINANCE8_ALPHA8;

   /* GL_EXT_texture_snorm */
   case GL_RGBA_SNORM:
      return GL_RGBA8_SNORM;
   case GL_RGB_SNORM:
      return GL_RGB8_SNORM;
   case GL_RG_SNORM:
      return GL_RG8_SNORM;
   case GL_RED_SNORM:
      return GL_R8_SNORM;
   case GL_LUMINANCE_ALPHA_SNORM:
      return GL_LUMINANCE8_ALPHA8_SNORM;
   case GL_LUMINANCE_SNORM:
      return GL_LUMINANCE8_SNORM;
   case GL_ALPHA_SNORM:
      return GL_ALPHA8_SNORM;
   case GL_INTENSITY_SNORM:
      return GL_INTENSITY8_SNORM;

   default:
      return format;
   }
}


/**
 * Convert an sRGB internal format to linear.
 */
GLenum
_mesa_get_linear_internalformat(GLenum format)
{
   switch (format) {
   case GL_SRGB:
      return GL_RGB;
   case GL_SRGB_ALPHA:
      return GL_RGBA;
   case GL_SRGB8:
      return GL_RGB8;
   case GL_SRGB8_ALPHA8:
      return GL_RGBA8;
   case GL_SLUMINANCE8:
      return GL_LUMINANCE8;
   case GL_SLUMINANCE:
      return GL_LUMINANCE;
   case GL_SLUMINANCE_ALPHA:
      return GL_LUMINANCE_ALPHA;
   case GL_SLUMINANCE8_ALPHA8:
      return GL_LUMINANCE8_ALPHA8;
   default:
      return format;
   }
}


/**
 * Do error checking of format/type combinations for glReadPixels,
 * glDrawPixels and glTex[Sub]Image.  Note that depending on the format
 * and type values, we may either generate GL_INVALID_OPERATION or
 * GL_INVALID_ENUM.
 *
 * \param format pixel format.
 * \param type pixel type.
 *
 * \return GL_INVALID_ENUM, GL_INVALID_OPERATION or GL_NO_ERROR
 */
GLenum
_mesa_error_check_format_and_type(const struct gl_context *ctx,
                                  GLenum format, GLenum type)
{
   /* From OpenGL 3.3 spec, page 220:
    *    "If the format is DEPTH_STENCIL, then values are taken from
    *    both the depth buffer and the stencil buffer. If there is no
    *    depth buffer or if there is no stencil buffer, then the error
    *    INVALID_OPERATION occurs. If the type parameter is not
    *    UNSIGNED_INT_24_8 or FLOAT_32_UNSIGNED_INT_24_8_REV, then the
    *    error INVALID_ENUM occurs."
    *
    * OpenGL ES still generates GL_INVALID_OPERATION because glReadPixels
    * cannot be used to read depth or stencil in that API.
    */
   if (_mesa_is_desktop_gl(ctx) && format == GL_DEPTH_STENCIL
       && type != GL_UNSIGNED_INT_24_8
       && type != GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
      return GL_INVALID_ENUM;

   /* special type-based checks (see glReadPixels, glDrawPixels error lists) */
   switch (type) {
   case GL_BITMAP:
      if (format != GL_COLOR_INDEX && format != GL_STENCIL_INDEX) {
         return GL_INVALID_ENUM;
      }
      break;

   case GL_UNSIGNED_BYTE_3_3_2:
   case GL_UNSIGNED_BYTE_2_3_3_REV:
   case GL_UNSIGNED_SHORT_5_6_5:
   case GL_UNSIGNED_SHORT_5_6_5_REV:
      if (format == GL_RGB) {
         break; /* OK */
      }
      if (format == GL_RGB_INTEGER_EXT &&
          _mesa_has_texture_rgb10_a2ui(ctx)) {
         break; /* OK */
      }
      return GL_INVALID_OPERATION;

   case GL_UNSIGNED_SHORT_4_4_4_4:
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
   case GL_UNSIGNED_INT_8_8_8_8:
   case GL_UNSIGNED_INT_8_8_8_8_REV:
      if (format == GL_RGBA ||
          format == GL_BGRA ||
          format == GL_ABGR_EXT) {
         break; /* OK */
      }
      if ((format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT) &&
          _mesa_has_texture_rgb10_a2ui(ctx)) {
         break; /* OK */
      }
      return GL_INVALID_OPERATION;

   case GL_UNSIGNED_SHORT_5_5_5_1:
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
   case GL_UNSIGNED_INT_10_10_10_2:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (format == GL_RGBA ||
          format == GL_BGRA) {
         break; /* OK */
      }
      if ((format == GL_RGBA_INTEGER_EXT || format == GL_BGRA_INTEGER_EXT) &&
          _mesa_has_texture_rgb10_a2ui(ctx)) {
         break; /* OK */
      }
      if (type == GL_UNSIGNED_INT_2_10_10_10_REV && format == GL_RGB &&
          _mesa_is_gles2(ctx)) {
         break; /* OK by GL_EXT_texture_type_2_10_10_10_REV */
      }
      return GL_INVALID_OPERATION;

   case GL_UNSIGNED_INT_24_8:
      /* Depth buffer OK to read in OpenGL ES (NV_read_depth). */
      if (_mesa_is_gles2(ctx) && format == GL_DEPTH_COMPONENT)
         return GL_NO_ERROR;

      if (format != GL_DEPTH_STENCIL) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (!_mesa_has_float_depth_buffer(ctx)) {
         return GL_INVALID_ENUM;
      }
      if (format != GL_DEPTH_STENCIL) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (!_mesa_has_packed_float(ctx)) {
         return GL_INVALID_ENUM;
      }
      if (format != GL_RGB) {
         return GL_INVALID_OPERATION;
      }
      return GL_NO_ERROR;

   case GL_HALF_FLOAT_OES:
      switch (format) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE:
      case GL_ALPHA:
         return GL_NO_ERROR;
      case GL_RG:
      case GL_RED:
         if (_mesa_has_rg_textures(ctx))
            return GL_NO_ERROR;
         FALLTHROUGH;
      default:
         return GL_INVALID_OPERATION;
      }

   default:
      ; /* fall-through */
   }

   /* now, for each format, check the type for compatibility */
   switch (format) {
      case GL_COLOR_INDEX:
      case GL_STENCIL_INDEX:
         switch (type) {
            case GL_BITMAP:
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
#if 0 /* not legal!  see table 3.6 of the 1.5 spec */
      case GL_INTENSITY:
#endif
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_DEPTH_COMPONENT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RG:
         if (!_mesa_has_rg_textures(ctx))
            return GL_INVALID_ENUM;
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGB:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_UNSIGNED_BYTE_3_3_2:
            case GL_UNSIGNED_BYTE_2_3_3_REV:
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_5_6_5_REV:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            case GL_UNSIGNED_INT_2_10_10_10_REV:
               /* OK by GL_EXT_texture_type_2_10_10_10_REV */
               return _mesa_is_gles2(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_INT_5_9_9_9_REV:
               return _mesa_has_texture_shared_exponent(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_INT_10F_11F_11F_REV:
               return _mesa_has_packed_float(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_BGR:
         switch (type) {
            /* NOTE: no packed types are supported with BGR.  That's
             * intentional, according to the GL spec.
             */
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGBA:
      case GL_BGRA:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            case GL_UNSIGNED_SHORT_5_5_5_1:
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            case GL_UNSIGNED_INT_8_8_8_8:
            case GL_UNSIGNED_INT_8_8_8_8_REV:
            case GL_UNSIGNED_INT_10_10_10_2:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_ABGR_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            case GL_FLOAT:
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            case GL_UNSIGNED_INT_8_8_8_8:
            case GL_UNSIGNED_INT_8_8_8_8_REV:
            case GL_HALF_FLOAT:
               return GL_NO_ERROR;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_YCBCR_MESA:
         if (!_mesa_has_MESA_ycbcr_texture(ctx))
            return GL_INVALID_ENUM;
         if (type == GL_UNSIGNED_SHORT_8_8_MESA ||
             type == GL_UNSIGNED_SHORT_8_8_REV_MESA)
            return GL_NO_ERROR;
         else
            return GL_INVALID_OPERATION;

      case GL_DEPTH_STENCIL:
         if (type == GL_UNSIGNED_INT_24_8)
            return GL_NO_ERROR;
         else if (_mesa_has_float_depth_buffer(ctx) &&
             type == GL_FLOAT_32_UNSIGNED_INT_24_8_REV)
            return GL_NO_ERROR;
         else
            return GL_INVALID_ENUM;

      /* integer-valued formats */
      case GL_RED_INTEGER_EXT:
      case GL_GREEN_INTEGER_EXT:
      case GL_BLUE_INTEGER_EXT:
      case GL_ALPHA_INTEGER_EXT:
      case GL_RG_INTEGER:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return _mesa_has_integer_textures(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGB_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return _mesa_has_integer_textures(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_BYTE_3_3_2:
            case GL_UNSIGNED_BYTE_2_3_3_REV:
            case GL_UNSIGNED_SHORT_5_6_5:
            case GL_UNSIGNED_SHORT_5_6_5_REV:
               return _mesa_has_texture_rgb10_a2ui(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_BGR_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
            /* NOTE: no packed formats w/ BGR format */
               return _mesa_has_integer_textures(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_RGBA_INTEGER_EXT:
      case GL_BGRA_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return _mesa_has_integer_textures(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            case GL_UNSIGNED_SHORT_4_4_4_4:
            case GL_UNSIGNED_SHORT_4_4_4_4_REV:
            case GL_UNSIGNED_SHORT_5_5_5_1:
            case GL_UNSIGNED_SHORT_1_5_5_5_REV:
            case GL_UNSIGNED_INT_8_8_8_8:
            case GL_UNSIGNED_INT_8_8_8_8_REV:
            case GL_UNSIGNED_INT_10_10_10_2:
            case GL_UNSIGNED_INT_2_10_10_10_REV:
               return _mesa_has_texture_rgb10_a2ui(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      case GL_LUMINANCE_INTEGER_EXT:
      case GL_LUMINANCE_ALPHA_INTEGER_EXT:
         switch (type) {
            case GL_BYTE:
            case GL_UNSIGNED_BYTE:
            case GL_SHORT:
            case GL_UNSIGNED_SHORT:
            case GL_INT:
            case GL_UNSIGNED_INT:
               return _mesa_has_integer_textures(ctx)
                  ? GL_NO_ERROR : GL_INVALID_ENUM;
            default:
               return GL_INVALID_ENUM;
         }

      default:
         return GL_INVALID_ENUM;
   }
   return GL_NO_ERROR;
}


/**
 * Do error checking of format/type combinations for OpenGL ES glReadPixels
 * and glTex[Sub]Image.
 * \return error code, or GL_NO_ERROR.
 */
GLenum
_mesa_es_error_check_format_and_type(const struct gl_context *ctx,
                                     GLenum format, GLenum type,
                                     unsigned dimensions)
{
   GLboolean type_valid = GL_TRUE;

   switch (format) {
   case GL_RED:
   case GL_RG:
      if (!_mesa_has_rg_textures(ctx))
         return GL_INVALID_VALUE;
      FALLTHROUGH;
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES);
      break;

   case GL_RGB:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_UNSIGNED_SHORT_5_6_5
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES);
      break;

   case GL_RGBA:
      type_valid = (type == GL_UNSIGNED_BYTE
                    || type == GL_UNSIGNED_SHORT_4_4_4_4
                    || type == GL_UNSIGNED_SHORT_5_5_5_1
                    || type == GL_FLOAT
                    || type == GL_HALF_FLOAT_OES
                    || (_mesa_has_texture_type_2_10_10_10_REV(ctx) &&
                        type == GL_UNSIGNED_INT_2_10_10_10_REV));
      break;

   case GL_DEPTH_COMPONENT:
      /* This format is filtered against invalid dimensionalities elsewhere.
       */
      type_valid = (type == GL_UNSIGNED_SHORT
                    || type == GL_UNSIGNED_INT);
      break;

   case GL_DEPTH_STENCIL:
      /* This format is filtered against invalid dimensionalities elsewhere.
       */
      type_valid = (type == GL_UNSIGNED_INT_24_8);
      break;

   case GL_BGRA_EXT:
      type_valid = (type == GL_UNSIGNED_BYTE);

      /* This feels like a bug in the EXT_texture_format_BGRA8888 spec, but
       * the format does not appear to be allowed for 3D textures in OpenGL
       * ES.
       */
      if (dimensions != 2)
         return GL_INVALID_VALUE;

      break;

   default:
      return GL_INVALID_VALUE;
   }

   return type_valid ? GL_NO_ERROR : GL_INVALID_OPERATION;
}

/**
 * Return the simple base format for a given internal texture format.
 * For example, given GL_LUMINANCE12_ALPHA4, return GL_LUMINANCE_ALPHA.
 *
 * \param ctx GL context.
 * \param internalFormat the internal texture format token or 1, 2, 3, or 4.
 *
 * \return the corresponding \u base internal format (GL_ALPHA, GL_LUMINANCE,
 * GL_LUMANCE_ALPHA, GL_INTENSITY, GL_RGB, or GL_RGBA), or -1 if invalid enum.
 *
 * This is the format which is used during texture application (i.e. the
 * texture format and env mode determine the arithmetic used.
 */
GLint
_mesa_base_tex_format(const struct gl_context *ctx, GLint internalFormat)
{
   switch (internalFormat) {
   case GL_ALPHA:
   case GL_ALPHA4:
   case GL_ALPHA8:
   case GL_ALPHA12:
   case GL_ALPHA16:
      return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
   case 1:
   case GL_LUMINANCE:
   case GL_LUMINANCE4:
   case GL_LUMINANCE8:
   case GL_LUMINANCE12:
   case GL_LUMINANCE16:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
   case 2:
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE4_ALPHA4:
   case GL_LUMINANCE6_ALPHA2:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE12_ALPHA4:
   case GL_LUMINANCE12_ALPHA12:
   case GL_LUMINANCE16_ALPHA16:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
   case GL_INTENSITY:
   case GL_INTENSITY4:
   case GL_INTENSITY8:
   case GL_INTENSITY12:
   case GL_INTENSITY16:
      return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
   case 3:
      return (ctx->API != API_OPENGL_CORE) ? GL_RGB : -1;
   case GL_RGB:
   case GL_R3_G3_B2:
   case GL_RGB4:
   case GL_RGB5:
   case GL_RGB8:
   case GL_RGB10:
   case GL_RGB12:
   case GL_RGB16:
      return GL_RGB;
   case 4:
      return (ctx->API != API_OPENGL_CORE) ? GL_RGBA : -1;
   case GL_RGBA:
   case GL_RGBA2:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGBA12:
   case GL_RGBA16:
      return GL_RGBA;
   default:
      ; /* fallthrough */
   }

   /* GL_BGRA can be an internal format *only* in OpenGL ES (1.x or 2.0).
    */
   if (_mesa_is_gles(ctx)) {
      switch (internalFormat) {
      case GL_BGRA:
         return GL_RGBA;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_ARB_ES2_compatibility(ctx) ||
       _mesa_has_OES_framebuffer_object(ctx) ||
       _mesa_is_gles2(ctx)) {
      switch (internalFormat) {
      case GL_RGB565:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (ctx->API != API_OPENGLES) {
      switch (internalFormat) {
      case GL_DEPTH_COMPONENT:
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT24:
      case GL_DEPTH_COMPONENT32:
         return GL_DEPTH_COMPONENT;
      case GL_DEPTH_STENCIL:
      case GL_DEPTH24_STENCIL8:
         return GL_DEPTH_STENCIL;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_ARB_texture_stencil8(ctx) ||
       _mesa_has_OES_texture_stencil8(ctx)) {
      switch (internalFormat) {
      case GL_STENCIL_INDEX:
      case GL_STENCIL_INDEX1:
      case GL_STENCIL_INDEX4:
      case GL_STENCIL_INDEX8:
      case GL_STENCIL_INDEX16:
         return GL_STENCIL_INDEX;
      default:
         ; /* fallthrough */
      }
   }

   switch (internalFormat) {
   case GL_COMPRESSED_ALPHA:
      return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
   case GL_COMPRESSED_LUMINANCE:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
   case GL_COMPRESSED_LUMINANCE_ALPHA:
      return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
   case GL_COMPRESSED_INTENSITY:
      return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
   case GL_COMPRESSED_RGB:
      return GL_RGB;
   case GL_COMPRESSED_RGBA:
      return GL_RGBA;
   default:
      ; /* fallthrough */
   }

   if (_mesa_is_compressed_format(ctx, internalFormat)) {
      GLenum base_compressed =
         _mesa_gl_compressed_format_base_format(internalFormat);
      if (base_compressed)
            return base_compressed;
   }

   if ((_mesa_has_KHR_texture_compression_astc_ldr(ctx) &&
        is_astc_2d_format(internalFormat)) ||
       (_mesa_has_OES_texture_compression_astc(ctx) &&
        is_astc_3d_format(internalFormat)))
        return GL_RGBA;

   if (_mesa_has_MESA_ycbcr_texture(ctx)) {
      if (internalFormat == GL_YCBCR_MESA)
         return GL_YCBCR_MESA;
   }

   if (_mesa_has_half_float_textures(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA16F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
      case GL_RGBA16F_ARB:
         return GL_RGBA;
      case GL_RGB16F_ARB:
         return GL_RGB;
      case GL_INTENSITY16F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
      case GL_LUMINANCE16F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
      case GL_LUMINANCE_ALPHA16F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
      }
   }

   if (_mesa_has_float_textures(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA32F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
      case GL_RGBA32F_ARB:
         return GL_RGBA;
      case GL_RGB32F_ARB:
         return GL_RGB;
      case GL_INTENSITY32F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
      case GL_LUMINANCE32F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
      case GL_LUMINANCE_ALPHA32F_ARB:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
      }
   }

   if (_mesa_has_EXT_texture_snorm(ctx) || _mesa_is_gles3(ctx)) {
      switch (internalFormat) {
      case GL_RED_SNORM:
      case GL_R8_SNORM:
      case GL_R16_SNORM:
         return GL_RED;
      case GL_RG_SNORM:
      case GL_RG8_SNORM:
      case GL_RG16_SNORM:
         return GL_RG;
      case GL_RGB_SNORM:
      case GL_RGB8_SNORM:
      case GL_RGB16_SNORM:
         return GL_RGB;
      case GL_RGBA_SNORM:
      case GL_RGBA8_SNORM:
      case GL_RGBA16_SNORM:
         return GL_RGBA;
      case GL_ALPHA_SNORM:
      case GL_ALPHA8_SNORM:
      case GL_ALPHA16_SNORM:
         return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
      case GL_LUMINANCE_SNORM:
      case GL_LUMINANCE8_SNORM:
      case GL_LUMINANCE16_SNORM:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
      case GL_LUMINANCE_ALPHA_SNORM:
      case GL_LUMINANCE8_ALPHA8_SNORM:
      case GL_LUMINANCE16_ALPHA16_SNORM:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
      case GL_INTENSITY_SNORM:
      case GL_INTENSITY8_SNORM:
      case GL_INTENSITY16_SNORM:
         return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_EXT_texture_sRGB(ctx) || _mesa_is_gles3(ctx)) {
      switch (internalFormat) {
      case GL_SRGB_EXT:
      case GL_SRGB8_EXT:
      case GL_COMPRESSED_SRGB_EXT:
         return GL_RGB;
      case GL_SRGB_ALPHA_EXT:
      case GL_SRGB8_ALPHA8_EXT:
      case GL_COMPRESSED_SRGB_ALPHA_EXT:
         return GL_RGBA;
      case GL_SLUMINANCE_ALPHA_EXT:
      case GL_SLUMINANCE8_ALPHA8_EXT:
      case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
      case GL_SLUMINANCE_EXT:
      case GL_SLUMINANCE8_EXT:
      case GL_COMPRESSED_SLUMINANCE_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_EXT_texture_sRGB_R8(ctx)) {
      switch (internalFormat) {
      case GL_SR8_EXT:
         return GL_RED;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_EXT_texture_sRGB_RG8(ctx)) {
      switch (internalFormat) {
      case GL_SRG8_EXT:
         return GL_RG;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_integer_textures(ctx)) {
      switch (internalFormat) {
      case GL_RGBA8UI_EXT:
      case GL_RGBA16UI_EXT:
      case GL_RGBA32UI_EXT:
      case GL_RGBA8I_EXT:
      case GL_RGBA16I_EXT:
      case GL_RGBA32I_EXT:
         return GL_RGBA;
      case GL_RGB8UI_EXT:
      case GL_RGB16UI_EXT:
      case GL_RGB32UI_EXT:
      case GL_RGB8I_EXT:
      case GL_RGB16I_EXT:
      case GL_RGB32I_EXT:
         return GL_RGB;
      }
   }

   if (_mesa_has_texture_rgb10_a2ui(ctx)) {
      switch (internalFormat) {
      case GL_RGB10_A2UI:
         return GL_RGBA;
      }
   }

   if (_mesa_has_integer_textures(ctx)) {
      switch (internalFormat) {
      case GL_ALPHA8UI_EXT:
      case GL_ALPHA16UI_EXT:
      case GL_ALPHA32UI_EXT:
      case GL_ALPHA8I_EXT:
      case GL_ALPHA16I_EXT:
      case GL_ALPHA32I_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_ALPHA : -1;
      case GL_INTENSITY8UI_EXT:
      case GL_INTENSITY16UI_EXT:
      case GL_INTENSITY32UI_EXT:
      case GL_INTENSITY8I_EXT:
      case GL_INTENSITY16I_EXT:
      case GL_INTENSITY32I_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_INTENSITY : -1;
      case GL_LUMINANCE8UI_EXT:
      case GL_LUMINANCE16UI_EXT:
      case GL_LUMINANCE32UI_EXT:
      case GL_LUMINANCE8I_EXT:
      case GL_LUMINANCE16I_EXT:
      case GL_LUMINANCE32I_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE : -1;
      case GL_LUMINANCE_ALPHA8UI_EXT:
      case GL_LUMINANCE_ALPHA16UI_EXT:
      case GL_LUMINANCE_ALPHA32UI_EXT:
      case GL_LUMINANCE_ALPHA8I_EXT:
      case GL_LUMINANCE_ALPHA16I_EXT:
      case GL_LUMINANCE_ALPHA32I_EXT:
         return (ctx->API != API_OPENGL_CORE) ? GL_LUMINANCE_ALPHA : -1;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_rg_textures(ctx)) {
      switch (internalFormat) {
      case GL_R16F:
         if (!_mesa_has_half_float_textures(ctx))
            break;
         return GL_RED;
      case GL_R32F:
         if (!_mesa_has_float_textures(ctx))
            break;
         return GL_RED;
      case GL_R8I:
      case GL_R8UI:
      case GL_R16I:
      case GL_R16UI:
      case GL_R32I:
      case GL_R32UI:
         if (!_mesa_has_integer_textures(ctx))
            break;
         FALLTHROUGH;
      case GL_R8:
      case GL_R16:
      case GL_RED:
      case GL_COMPRESSED_RED:
         return GL_RED;

      case GL_RG16F:
         if (!_mesa_has_half_float_textures(ctx))
            break;
         return GL_RG;
      case GL_RG32F:
         if (!_mesa_has_float_textures(ctx))
            break;
         return GL_RG;
      case GL_RG8I:
      case GL_RG8UI:
      case GL_RG16I:
      case GL_RG16UI:
      case GL_RG32I:
      case GL_RG32UI:
         if (!_mesa_has_integer_textures(ctx))
            break;
         FALLTHROUGH;
      case GL_RG:
      case GL_RG8:
      case GL_RG16:
      case GL_COMPRESSED_RG:
         return GL_RG;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_texture_shared_exponent(ctx)) {
      switch (internalFormat) {
      case GL_RGB9_E5_EXT:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_packed_float(ctx)) {
      switch (internalFormat) {
      case GL_R11F_G11F_B10F_EXT:
         return GL_RGB;
      default:
         ; /* fallthrough */
      }
   }

   if (_mesa_has_float_depth_buffer(ctx)) {
      switch (internalFormat) {
      case GL_DEPTH_COMPONENT32F:
         return GL_DEPTH_COMPONENT;
      case GL_DEPTH32F_STENCIL8:
         return GL_DEPTH_STENCIL;
      default:
         ; /* fallthrough */
      }
   }

   return -1; /* error */
}

/**
 * Returns the effective internal format from a texture format and type.
 * This is used by texture image operations internally for validation, when
 * the specified internal format is a base (unsized) format.
 *
 * This method will only return a valid effective internal format if the
 * combination of format, type and internal format in base form, is acceptable.
 *
 * If a single sized internal format is defined in the spec (OpenGL-ES 3.0.4) or
 * in extensions, to unambiguously correspond to the given base format, then
 * that internal format is returned as the effective. Otherwise, if the
 * combination is accepted but a single effective format is not defined, the
 * passed base format will be returned instead.
 *
 * \param format the texture format
 * \param type the texture type
 */
static GLenum
gles_effective_internal_format_for_format_and_type(GLenum format,
                                                   GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
      switch (format) {
      case GL_RGBA:
         return GL_RGBA8;
      case GL_RGB:
         return GL_RGB8;
      case GL_RG:
         return GL_RG8;
      case GL_RED:
         return GL_R8;
      /* Although LUMINANCE_ALPHA, LUMINANCE and ALPHA appear in table 3.12,
       * (section 3.8 Texturing, page 128 of the OpenGL-ES 3.0.4) as effective
       * internal formats, they do not correspond to GL constants, so the base
       * format is returned instead.
       */
      case GL_BGRA_EXT:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE:
      case GL_ALPHA:
         return format;
      }
      break;

   case GL_UNSIGNED_SHORT_4_4_4_4:
      if (format == GL_RGBA)
         return GL_RGBA4;
      break;

   case GL_UNSIGNED_SHORT_5_5_5_1:
      if (format == GL_RGBA)
         return GL_RGB5_A1;
      break;

   case GL_UNSIGNED_SHORT_5_6_5:
      if (format == GL_RGB)
         return GL_RGB565;
      break;

   /* OES_packed_depth_stencil */
   case GL_UNSIGNED_INT_24_8:
      if (format == GL_DEPTH_STENCIL)
         return GL_DEPTH24_STENCIL8;
      break;

   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (format == GL_DEPTH_STENCIL)
         return GL_DEPTH32F_STENCIL8;
      break;

   case GL_UNSIGNED_SHORT:
      if (format == GL_DEPTH_COMPONENT)
         return GL_DEPTH_COMPONENT16;
      break;

   case GL_UNSIGNED_INT:
      /* It can be DEPTH_COMPONENT16 or DEPTH_COMPONENT24, so just return
       * the format.
       */
      if (format == GL_DEPTH_COMPONENT)
         return format;
      break;

   /* OES_texture_float and OES_texture_half_float */
   case GL_FLOAT:
      if (format == GL_DEPTH_COMPONENT)
         return GL_DEPTH_COMPONENT32F;
      FALLTHROUGH;
   case GL_HALF_FLOAT_OES:
      switch (format) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE:
      case GL_ALPHA:
      case GL_RED:
      case GL_RG:
         return format;
      }
      break;
   case GL_HALF_FLOAT:
      switch (format) {
      case GL_RG:
      case GL_RED:
         return format;
      }
      break;

   /* GL_EXT_texture_type_2_10_10_10_REV */
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      switch (format) {
      case GL_RGBA:
      case GL_RGB:
         return format;
      }
      break;

   default:
      /* fall through and return NONE */
      break;
   }

   return GL_NONE;
}

/**
 * Error checking if internalformat for glTex[Sub]Image is valid
 * within OpenGL ES 3.2 (or introduced by an ES extension).
 *
 * Note, further checks in _mesa_gles_error_check_format_and_type
 * are required for complete checking between format and type.
 */
static GLenum
_mesa_gles_check_internalformat(const struct gl_context *ctx,
                                GLenum internalFormat)
{
   switch (internalFormat) {
   /* OpenGL ES 2.0 */
   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
   case GL_RGB:
   case GL_RGBA:

   /* GL_OES_depth_texture */
   case GL_DEPTH_COMPONENT:

   /* GL_EXT_texture_format_BGRA8888 */
   case GL_BGRA:

   /* GL_OES_required_internalformat */
   case GL_RGB565:
   case GL_RGB8:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_DEPTH_COMPONENT16:
   case GL_DEPTH_COMPONENT24:
   case GL_DEPTH_COMPONENT32:
   case GL_DEPTH24_STENCIL8:
   case GL_RGB10_EXT:
   case GL_RGB10_A2_EXT:
   case GL_ALPHA8:
   case GL_LUMINANCE8:
   case GL_LUMINANCE8_ALPHA8:
   case GL_LUMINANCE4_ALPHA4:
      return GL_NO_ERROR;

   case GL_R8:
   case GL_RG8:
   case GL_RED:
   case GL_RG:
      if (!_mesa_has_rg_textures(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   /* GL_OES_texture_stencil8 */
   case GL_STENCIL_INDEX8:
      if (!_mesa_has_OES_texture_stencil8(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_COMPRESSED_RGBA_BPTC_UNORM:
   case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
   case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
   case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
      if (!_mesa_has_EXT_texture_compression_bptc(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_COMPRESSED_RED_RGTC1:
   case GL_COMPRESSED_SIGNED_RED_RGTC1:
   case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
   case GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT:
      if (!_mesa_has_EXT_texture_compression_rgtc(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
      if (!_mesa_has_EXT_texture_compression_s3tc(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
   case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
      if (!_mesa_has_EXT_texture_compression_s3tc_srgb(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_R16:
   case GL_RG16:
   case GL_RGB16:
   case GL_RGBA16:
      if (!_mesa_has_EXT_texture_norm16(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_R16_SNORM:
   case GL_RG16_SNORM:
   case GL_RGB16_SNORM:
   case GL_RGBA16_SNORM:
      if (!_mesa_has_EXT_texture_norm16(ctx) &&
          !_mesa_has_EXT_texture_snorm(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_SR8_EXT:
      if (!_mesa_has_EXT_texture_sRGB_R8(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   case GL_SRG8_EXT:
      if (!_mesa_has_EXT_texture_sRGB_RG8(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;

   /* OpenGL ES 3.0 */
   case GL_SRGB8_ALPHA8:
   case GL_RGBA8_SNORM:
   case GL_RGBA16F:
   case GL_RGBA32F:
   case GL_RGBA8UI:
   case GL_RGBA8I:
   case GL_RGBA16UI:
   case GL_RGBA16I:
   case GL_RGBA32UI:
   case GL_RGBA32I:
   case GL_RGB10_A2UI:
   case GL_SRGB8:
   case GL_RGB8_SNORM:
   case GL_R11F_G11F_B10F:
   case GL_RGB9_E5:
   case GL_RGB16F:
   case GL_RGB32F:
   case GL_RGB8UI:
   case GL_RGB8I:
   case GL_RGB16UI:
   case GL_RGB16I:
   case GL_RGB32UI:
   case GL_RGB32I:
   case GL_RG8_SNORM:
   case GL_RG16F:
   case GL_RG32F:
   case GL_RG8UI:
   case GL_RG8I:
   case GL_RG16UI:
   case GL_RG16I:
   case GL_RG32UI:
   case GL_RG32I:
   case GL_R8_SNORM:
   case GL_R16F:
   case GL_R32F:
   case GL_R8UI:
   case GL_R8I:
   case GL_R16UI:
   case GL_R16I:
   case GL_R32UI:
   case GL_R32I:
   case GL_DEPTH_COMPONENT32F:
   case GL_DEPTH32F_STENCIL8:
      if (!_mesa_is_gles3(ctx))
         return GL_INVALID_VALUE;
      return GL_NO_ERROR;
   default:
      return GL_INVALID_VALUE;
   }
}

/**
 * Do error checking of format/type combinations for OpenGL ES 3
 * glTex[Sub]Image, or ES1/ES2 with GL_OES_required_internalformat.
 * \return error code, or GL_NO_ERROR.
 */
GLenum
_mesa_gles_error_check_format_and_type(const struct gl_context *ctx,
                                       GLenum format, GLenum type,
                                       GLenum internalFormat)
{
   /* If internalFormat is an unsized format, then the effective internal
    * format derived from format and type should be used instead. Page 127,
    * section "3.8 Texturing" of the GLES 3.0.4 spec states:
    *
    *    "if internalformat is a base internal format, the effective
    *     internal format is a sized internal format that is derived
    *     from the format and type for internal use by the GL.
    *     Table 3.12 specifies the mapping of format and type to effective
    *     internal formats. The effective internal format is used by the GL
    *     for purposes such as texture completeness or type checks for
    *     CopyTex* commands. In these cases, the GL is required to operate
    *     as if the effective internal format was used as the internalformat
    *     when specifying the texture data."
    */
   if (_mesa_is_enum_format_unsized(internalFormat)) {
      GLenum effectiveInternalFormat =
         gles_effective_internal_format_for_format_and_type(format, type);

      if (effectiveInternalFormat == GL_NONE)
         return GL_INVALID_OPERATION;

      GLenum baseInternalFormat;
      if (internalFormat == GL_BGRA_EXT) {
         /* Unfortunately, _mesa_base_tex_format returns a base format of
          * GL_RGBA for GL_BGRA_EXT.  This makes perfect sense if you're
          * asking the question, "what channels does this format have?"
          * However, if we're trying to determine if two internal formats
          * match in the ES3 sense, we actually want GL_BGRA.
          */
         baseInternalFormat = GL_BGRA_EXT;
      } else {
         baseInternalFormat =
            _mesa_base_tex_format(ctx, effectiveInternalFormat);
      }

      if (internalFormat != baseInternalFormat)
         return GL_INVALID_OPERATION;

      internalFormat = effectiveInternalFormat;
   }

   /* The GLES variant of EXT_texture_compression_s3tc is very vague and
    * doesn't list valid types. Just do exactly what the spec says.
    */
   if (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ||
       internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
       internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT ||
       internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
      return format == GL_RGB || format == GL_RGBA ? GL_NO_ERROR :
                                                     GL_INVALID_OPERATION;

   /* Before checking for the combination, verify that
    * given internalformat is legal for OpenGL ES.
    */
   GLenum internal_format_error =
      _mesa_gles_check_internalformat(ctx, internalFormat);

   if (internal_format_error != GL_NO_ERROR)
      return internal_format_error;

   switch (format) {
   case GL_BGRA_EXT:
      if (type != GL_UNSIGNED_BYTE ||
              (internalFormat != GL_BGRA &&
               internalFormat != GL_RGBA8 &&
               internalFormat != GL_SRGB8_ALPHA8))
         return GL_INVALID_OPERATION;
      break;

   case GL_BGR_EXT:
      if (type != GL_UNSIGNED_BYTE ||
              (internalFormat != GL_RGB8 &&
               internalFormat != GL_SRGB8))
         return GL_INVALID_OPERATION;
      break;

   case GL_RGBA:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGBA8:
         case GL_RGB5_A1:
         case GL_RGBA4:
            break;
         case GL_SRGB8_ALPHA8_EXT:
            if (ctx->Version <= 20)
               return GL_INVALID_OPERATION;
            break;
         case GL_COMPRESSED_RGBA_BPTC_UNORM:
         case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_BYTE:
         if (ctx->Version <= 20 || internalFormat != GL_RGBA8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGBA16)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGBA16_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT_4_4_4_4:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGBA4:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_SHORT_5_5_5_1:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGB5_A1:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         switch (internalFormat) {
         case GL_RGBA:
         case GL_RGB10_A2:
         case GL_RGB5_A1:
            if (!_mesa_has_texture_type_2_10_10_10_REV(ctx))
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_HALF_FLOAT:
         if (ctx->Version <= 20 || internalFormat != GL_RGBA16F)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RGBA16F:
         case GL_RGBA32F:
            if (ctx->Version <= 20)
               return GL_INVALID_OPERATION;
            break;
         case GL_RGBA:
            if (!_mesa_has_OES_texture_float(ctx) || internalFormat != format)
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_HALF_FLOAT_OES:
         if (!_mesa_has_OES_texture_half_float(ctx) || internalFormat != format)
            return GL_INVALID_OPERATION;
         break;
      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGBA_INTEGER:
      if (ctx->Version <= 20)
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RGBA8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGBA8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGBA16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGBA16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RGBA32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RGBA32I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         if (internalFormat != GL_RGB10_A2UI)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGB:
      switch (type) {
      case GL_UNSIGNED_BYTE:
         switch (internalFormat) {
         case GL_RGB:
         case GL_RGB8:
         case GL_RGB565:
            break;
         case GL_SRGB8:
            if (ctx->Version <= 20)
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_BYTE:
         if (ctx->Version <= 20 || internalFormat != GL_RGB8_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGB16)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGB16_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT_5_6_5:
         switch (internalFormat) {
         case GL_RGB:
         case GL_RGB565:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_UNSIGNED_INT_10F_11F_11F_REV:
         if (ctx->Version <= 20 || internalFormat != GL_R11F_G11F_B10F)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT_5_9_9_9_REV:
         if (ctx->Version <= 20 || internalFormat != GL_RGB9_E5)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
         if (ctx->Version <= 20)
            return GL_INVALID_OPERATION;
         switch (internalFormat) {
         case GL_RGB16F:
         case GL_R11F_G11F_B10F:
         case GL_RGB9_E5:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RGB16F:
         case GL_RGB32F:
         case GL_R11F_G11F_B10F:
         case GL_RGB9_E5:
            if (ctx->Version <= 20)
               return GL_INVALID_OPERATION;
            break;
         case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
         case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
            break;
         case GL_RGB:
            if (!_mesa_has_OES_texture_float(ctx) || internalFormat != format)
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_HALF_FLOAT_OES:
         if (!_mesa_has_OES_texture_half_float(ctx) || internalFormat != format)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT_2_10_10_10_REV:
         switch (internalFormat) {
         case GL_RGB:
         case GL_RGB10:
         case GL_RGB8:
         case GL_RGB565:
            /* GL_EXT_texture_type_2_10_10_10_REV allows GL_RGB even though
             * GLES3 doesn't, and GL_OES_required_internalformat extends that
             * to allow the sized RGB internalformats as well.
             */
            if (!_mesa_has_texture_type_2_10_10_10_REV(ctx))
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RGB_INTEGER:
      if (ctx->Version <= 20)
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RGB8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RGB8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RGB16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RGB16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RGB32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RGB32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RG:
      if (!_mesa_has_rg_textures(ctx))
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RG8 &&
             internalFormat != GL_COMPRESSED_RED_GREEN_RGTC2_EXT &&
             internalFormat != GL_SRG8_EXT)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RG8_SNORM &&
             internalFormat != GL_COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RG16)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RG16_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
      case GL_HALF_FLOAT_OES:
         switch (internalFormat) {
            case GL_RG16F:
               if (ctx->Version <= 20)
                  return GL_INVALID_OPERATION;
               break;
            case GL_RG:
               if (!_mesa_has_OES_texture_half_float(ctx))
                  return GL_INVALID_OPERATION;
               break;
            default:
               return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_RG16F:
         case GL_RG32F:
            break;
         case GL_RG:
            if (!_mesa_has_OES_texture_float(ctx))
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RG_INTEGER:
      if (ctx->Version <= 20)
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_RG8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_RG8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_RG16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_RG16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_RG32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_RG32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RED:
      if (!_mesa_has_rg_textures(ctx))
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_R8 &&
             internalFormat != GL_SR8_EXT &&
             internalFormat != GL_COMPRESSED_RED_RGTC1_EXT) {
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_BYTE:
         if (internalFormat != GL_R8_SNORM &&
             internalFormat != GL_COMPRESSED_SIGNED_RED_RGTC1_EXT)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_R16)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_R16_SNORM)
            return GL_INVALID_OPERATION;
         break;

      case GL_HALF_FLOAT:
      case GL_HALF_FLOAT_OES:
         switch (internalFormat) {
         case GL_R16F:
            if (ctx->Version <= 20)
               return GL_INVALID_OPERATION;
            break;
         case GL_RG:
         case GL_RED:
            if (!_mesa_has_OES_texture_half_float(ctx))
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         switch (internalFormat) {
         case GL_R16F:
         case GL_R32F:
            break;
         case GL_RED:
            if (!_mesa_has_OES_texture_float(ctx))
               return GL_INVALID_OPERATION;
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_RED_INTEGER:
      if (ctx->Version <= 20)
         return GL_INVALID_OPERATION;
      switch (type) {
      case GL_UNSIGNED_BYTE:
         if (internalFormat != GL_R8UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_BYTE:
         if (internalFormat != GL_R8I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_R16UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_SHORT:
         if (internalFormat != GL_R16I)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         if (internalFormat != GL_R32UI)
            return GL_INVALID_OPERATION;
         break;

      case GL_INT:
         if (internalFormat != GL_R32I)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_DEPTH_COMPONENT:
      switch (type) {
      case GL_UNSIGNED_SHORT:
         if (internalFormat != GL_DEPTH_COMPONENT &&
             internalFormat != GL_DEPTH_COMPONENT16)
            return GL_INVALID_OPERATION;
         break;

      case GL_UNSIGNED_INT:
         switch (internalFormat) {
         case GL_DEPTH_COMPONENT:
         case GL_DEPTH_COMPONENT16:
         case GL_DEPTH_COMPONENT24:
            break;
         default:
            return GL_INVALID_OPERATION;
         }
         break;

      case GL_FLOAT:
         if (ctx->Version <= 20 || internalFormat != GL_DEPTH_COMPONENT32F)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_DEPTH_STENCIL:
      switch (type) {
      case GL_UNSIGNED_INT_24_8:
         if (internalFormat != GL_DEPTH_STENCIL &&
             internalFormat != GL_DEPTH24_STENCIL8)
            return GL_INVALID_OPERATION;
         break;

      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         if (ctx->Version <= 20 || internalFormat != GL_DEPTH32F_STENCIL8)
            return GL_INVALID_OPERATION;
         break;

      default:
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_STENCIL_INDEX:
      if (type != GL_UNSIGNED_BYTE ||
          internalFormat != GL_STENCIL_INDEX8) {
         return GL_INVALID_OPERATION;
      }
      break;

   case GL_ALPHA:
   case GL_LUMINANCE:
   case GL_LUMINANCE_ALPHA:
      switch (type) {
      case GL_FLOAT:
         if (!_mesa_has_OES_texture_float(ctx) || internalFormat != format)
            return GL_INVALID_OPERATION;
         break;
      case GL_HALF_FLOAT_OES:
         if (!_mesa_has_OES_texture_half_float(ctx) || internalFormat != format)
            return GL_INVALID_OPERATION;
         break;
      case GL_UNSIGNED_BYTE:
         if (!(format == internalFormat ||
               (format == GL_ALPHA && internalFormat == GL_ALPHA8) ||
               (format == GL_LUMINANCE && internalFormat == GL_LUMINANCE8) ||
               (format == GL_LUMINANCE_ALPHA &&
                ((internalFormat == GL_LUMINANCE8_ALPHA8) ||
                 (internalFormat == GL_LUMINANCE4_ALPHA4))))) {
            return GL_INVALID_OPERATION;
         }
         break;
      default:
         return GL_INVALID_OPERATION;
      }
      break;
   }

   return GL_NO_ERROR;
}

static void
set_swizzle(uint8_t *swizzle, int x, int y, int z, int w)
{
   swizzle[MESA_FORMAT_SWIZZLE_X] = x;
   swizzle[MESA_FORMAT_SWIZZLE_Y] = y;
   swizzle[MESA_FORMAT_SWIZZLE_Z] = z;
   swizzle[MESA_FORMAT_SWIZZLE_W] = w;
}

static bool
get_swizzle_from_gl_format(GLenum format, uint8_t *swizzle)
{
   switch (format) {
   case GL_RGBA:
   case GL_RGBA_INTEGER_EXT:
      set_swizzle(swizzle, 0, 1, 2, 3);
      return true;
   case GL_BGRA:
   case GL_BGRA_INTEGER_EXT:
      set_swizzle(swizzle, 2, 1, 0, 3);
      return true;
   case GL_ABGR_EXT:
      set_swizzle(swizzle, 3, 2, 1, 0);
      return true;
   case GL_RGB:
   case GL_RGB_INTEGER_EXT:
      set_swizzle(swizzle, 0, 1, 2, 5);
      return true;
   case GL_BGR:
   case GL_BGR_INTEGER_EXT:
      set_swizzle(swizzle, 2, 1, 0, 5);
      return true;
   case GL_LUMINANCE_ALPHA:
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      set_swizzle(swizzle, 0, 0, 0, 1);
      return true;
   case GL_RG:
   case GL_RG_INTEGER:
      set_swizzle(swizzle, 0, 1, 4, 5);
      return true;
   case GL_RED:
   case GL_RED_INTEGER_EXT:
      set_swizzle(swizzle, 0, 4, 4, 5);
      return true;
   case GL_GREEN:
   case GL_GREEN_INTEGER_EXT:
      set_swizzle(swizzle, 4, 0, 4, 5);
      return true;
   case GL_BLUE:
   case GL_BLUE_INTEGER_EXT:
      set_swizzle(swizzle, 4, 4, 0, 5);
      return true;
   case GL_ALPHA:
   case GL_ALPHA_INTEGER_EXT:
      set_swizzle(swizzle, 4, 4, 4, 0);
      return true;
   case GL_LUMINANCE:
   case GL_LUMINANCE_INTEGER_EXT:
      set_swizzle(swizzle, 0, 0, 0, 5);
      return true;
   case GL_INTENSITY:
      set_swizzle(swizzle, 0, 0, 0, 0);
      return true;
   case GL_DEPTH_COMPONENT:
      set_swizzle(swizzle, 0, 6, 6, 6);
      return true;
   case GL_STENCIL_INDEX:
      set_swizzle(swizzle, 6, 0, 6, 6);
      return true;
   default:
      return false;
   }
}

bool
_mesa_swap_bytes_in_type_enum(GLenum *type)
{
   switch (*type) {
   case GL_UNSIGNED_INT_8_8_8_8:
      *type = GL_UNSIGNED_INT_8_8_8_8_REV;
      return true;
   case GL_UNSIGNED_INT_8_8_8_8_REV:
      *type = GL_UNSIGNED_INT_8_8_8_8;
      return true;
   case GL_UNSIGNED_SHORT_8_8_MESA:
      *type = GL_UNSIGNED_SHORT_8_8_REV_MESA;
      return true;
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      *type = GL_UNSIGNED_SHORT_8_8_MESA;
      return true;
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      /* format/types that are arrays of 8-bit values are unaffected by
       * swapBytes.
       */
      return true;
   default:
      /* swapping bytes on 4444, 1555, or >8 bit per channel types etc. will
       * never match a Mesa format.
       */
      return false;
   }
}

/**
* Take an OpenGL format (GL_RGB, GL_RGBA, etc), OpenGL data type (GL_INT,
* GL_FOAT, etc) and return a matching mesa_array_format or a mesa_format
* otherwise (for non-array formats).
*
* This function will typically be used to compute a mesa format from a GL type
* so we can then call _mesa_format_convert. This function does
* not consider byte swapping, so it returns types assuming that no byte
* swapping is involved. If byte swapping is involved then clients are supposed
* to handle that on their side before calling _mesa_format_convert.
*
* This function returns an uint32_t that can pack a mesa_format or a
* mesa_array_format. Clients must check the mesa array format bit
* (MESA_ARRAY_FORMAT_BIT) on the return value to know if the returned
* format is a mesa_array_format or a mesa_format.
*/
uint32_t
_mesa_format_from_format_and_type(GLenum format, GLenum type)
{
   bool is_array_format = true;
   uint8_t swizzle[4];
   bool normalized = false, is_float = false, is_signed = false;
   int num_channels = 0, type_size = 0;

   if (format == GL_COLOR_INDEX)
      return MESA_FORMAT_NONE;

   /* Extract array format type information from the OpenGL data type */
   switch (type) {
   case GL_UNSIGNED_BYTE:
      type_size = 1;
      break;
   case GL_BYTE:
      type_size = 1;
      is_signed = true;
      break;
   case GL_UNSIGNED_SHORT:
      type_size = 2;
      break;
   case GL_SHORT:
      type_size = 2;
      is_signed = true;
      break;
   case GL_UNSIGNED_INT:
      type_size = 4;
      break;
   case GL_INT:
      type_size = 4;
      is_signed = true;
      break;
   case GL_HALF_FLOAT:
   case GL_HALF_FLOAT_OES:
      type_size = 2;
      is_signed = true;
      is_float = true;
      break;
   case GL_FLOAT:
      type_size = 4;
      is_signed = true;
      is_float = true;
      break;
   default:
      is_array_format = false;
      break;
   }

   /* Extract array format swizzle information from the OpenGL format */
   if (is_array_format)
      is_array_format = get_swizzle_from_gl_format(format, swizzle);

   /* If this is an array format type after checking data type and format,
    * create the array format
    */
   if (is_array_format) {
      enum mesa_array_format_base_format bf;
      switch (format) {
      case GL_DEPTH_COMPONENT:
         bf = MESA_ARRAY_FORMAT_BASE_FORMAT_DEPTH;
         break;
      case GL_STENCIL_INDEX:
         bf = MESA_ARRAY_FORMAT_BASE_FORMAT_STENCIL;
         break;
      default:
         bf = MESA_ARRAY_FORMAT_BASE_FORMAT_RGBA_VARIANTS;
         break;
      }

      normalized = !(_mesa_is_enum_format_integer(format) ||
                     format == GL_STENCIL_INDEX);
      num_channels = _mesa_components_in_format(format);

      return MESA_ARRAY_FORMAT(bf, type_size, is_signed, is_float,
                               normalized, num_channels,
                               swizzle[0], swizzle[1], swizzle[2], swizzle[3]);
   }

   /* Otherwise this is not an array format, so return the mesa_format
    * matching the OpenGL format and data type
    */
   switch (type) {
   case GL_UNSIGNED_SHORT_5_6_5:
     if (format == GL_RGB)
         return MESA_FORMAT_B5G6R5_UNORM;
      else if (format == GL_BGR)
         return MESA_FORMAT_R5G6B5_UNORM;
      else if (format == GL_RGB_INTEGER)
         return MESA_FORMAT_B5G6R5_UINT;
      break;
   case GL_UNSIGNED_SHORT_5_6_5_REV:
      if (format == GL_RGB)
         return MESA_FORMAT_R5G6B5_UNORM;
      else if (format == GL_BGR)
         return MESA_FORMAT_B5G6R5_UNORM;
      else if (format == GL_RGB_INTEGER)
         return MESA_FORMAT_R5G6B5_UINT;
      break;
   case GL_UNSIGNED_SHORT_4_4_4_4:
      if (format == GL_RGBA)
         return MESA_FORMAT_A4B4G4R4_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_A4R4G4B4_UNORM;
      else if (format == GL_ABGR_EXT)
         return MESA_FORMAT_R4G4B4A4_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_A4B4G4R4_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_A4R4G4B4_UINT;
      break;
   case GL_UNSIGNED_SHORT_4_4_4_4_REV:
      if (format == GL_RGBA)
         return MESA_FORMAT_R4G4B4A4_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_B4G4R4A4_UNORM;
      else if (format == GL_ABGR_EXT)
         return MESA_FORMAT_A4B4G4R4_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_R4G4B4A4_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_B4G4R4A4_UINT;
      break;
   case GL_UNSIGNED_SHORT_5_5_5_1:
      if (format == GL_RGBA)
         return MESA_FORMAT_A1B5G5R5_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_A1R5G5B5_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_A1B5G5R5_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_A1R5G5B5_UINT;
      break;
   case GL_UNSIGNED_SHORT_1_5_5_5_REV:
      if (format == GL_RGBA)
         return MESA_FORMAT_R5G5B5A1_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_B5G5R5A1_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_R5G5B5A1_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_B5G5R5A1_UINT;
      break;
   case GL_UNSIGNED_BYTE_3_3_2:
      if (format == GL_RGB)
         return MESA_FORMAT_B2G3R3_UNORM;
      else if (format == GL_RGB_INTEGER)
         return MESA_FORMAT_B2G3R3_UINT;
      break;
   case GL_UNSIGNED_BYTE_2_3_3_REV:
      if (format == GL_RGB)
         return MESA_FORMAT_R3G3B2_UNORM;
      else if (format == GL_RGB_INTEGER)
         return MESA_FORMAT_R3G3B2_UINT;
      break;
   case GL_UNSIGNED_INT_5_9_9_9_REV:
      if (format == GL_RGB)
         return MESA_FORMAT_R9G9B9E5_FLOAT;
      break;
   case GL_UNSIGNED_INT_10_10_10_2:
      if (format == GL_RGBA)
         return MESA_FORMAT_A2B10G10R10_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_A2B10G10R10_UINT;
      else if (format == GL_BGRA)
         return MESA_FORMAT_A2R10G10B10_UNORM;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_A2R10G10B10_UINT;
      break;
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      if (format == GL_RGB)
         return MESA_FORMAT_R10G10B10X2_UNORM;
      if (format == GL_RGBA)
         return MESA_FORMAT_R10G10B10A2_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_R10G10B10A2_UINT;
      else if (format == GL_BGRA)
         return MESA_FORMAT_B10G10R10A2_UNORM;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_B10G10R10A2_UINT;
      break;
   case GL_UNSIGNED_INT_8_8_8_8:
      if (format == GL_RGBA)
         return MESA_FORMAT_A8B8G8R8_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_A8R8G8B8_UNORM;
      else if (format == GL_ABGR_EXT)
         return MESA_FORMAT_R8G8B8A8_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_A8B8G8R8_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_A8R8G8B8_UINT;
      break;
   case GL_UNSIGNED_INT_8_8_8_8_REV:
      if (format == GL_RGBA)
         return MESA_FORMAT_R8G8B8A8_UNORM;
      else if (format == GL_BGRA)
         return MESA_FORMAT_B8G8R8A8_UNORM;
      else if (format == GL_ABGR_EXT)
         return MESA_FORMAT_A8B8G8R8_UNORM;
      else if (format == GL_RGBA_INTEGER)
         return MESA_FORMAT_R8G8B8A8_UINT;
      else if (format == GL_BGRA_INTEGER)
         return MESA_FORMAT_B8G8R8A8_UINT;
      break;
   case GL_UNSIGNED_SHORT_8_8_MESA:
      if (format == GL_YCBCR_MESA)
         return MESA_FORMAT_YCBCR;
      break;
   case GL_UNSIGNED_SHORT_8_8_REV_MESA:
      if (format == GL_YCBCR_MESA)
         return MESA_FORMAT_YCBCR_REV;
      break;
   case GL_UNSIGNED_INT_10F_11F_11F_REV:
      if (format == GL_RGB)
         return MESA_FORMAT_R11G11B10_FLOAT;
      break;
   case GL_FLOAT:
      if (format == GL_DEPTH_COMPONENT)
         return MESA_FORMAT_Z_FLOAT32;
      break;
   case GL_UNSIGNED_INT:
      if (format == GL_DEPTH_COMPONENT)
         return MESA_FORMAT_Z_UNORM32;
      break;
   case GL_UNSIGNED_SHORT:
      if (format == GL_DEPTH_COMPONENT)
         return MESA_FORMAT_Z_UNORM16;
      break;
   case GL_UNSIGNED_INT_24_8:
      if (format == GL_DEPTH_STENCIL)
         return MESA_FORMAT_S8_UINT_Z24_UNORM;
      else if (format == GL_DEPTH_COMPONENT)
         return MESA_FORMAT_X8_UINT_Z24_UNORM;
      break;
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      if (format == GL_DEPTH_STENCIL)
         return MESA_FORMAT_Z32_FLOAT_S8X24_UINT;
      break;
   default:
      break;
   }

   fprintf(stderr, "Unsupported format/type: %s/%s\n",
           _mesa_enum_to_string(format),
           _mesa_enum_to_string(type));

   /* If we got here it means that we could not find a Mesa format that
    * matches the GL format/type provided. We may need to add a new Mesa
    * format in that case.
    */
   unreachable("Unsupported format");
}

uint32_t
_mesa_tex_format_from_format_and_type(const struct gl_context *ctx,
                                      GLenum gl_format, GLenum type)
{
   mesa_format format = _mesa_format_from_format_and_type(gl_format, type);

   if (_mesa_format_is_mesa_array_format(format))
      format = _mesa_format_from_array_format(format);
      
   if (format == MESA_FORMAT_NONE || !ctx->TextureFormatSupported[format])
      return MESA_FORMAT_NONE;

   return format;
}

/**
 * Returns true if \p internal_format is a sized internal format that
 * is marked "Color Renderable" in Table 8.10 of the ES 3.2 specification.
 */
bool
_mesa_is_es3_color_renderable(const struct gl_context *ctx,
                              GLenum internal_format)
{
   switch (internal_format) {
   case GL_R8:
   case GL_RG8:
   case GL_RGB8:
   case GL_RGB565:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGB10_A2UI:
   case GL_SRGB8_ALPHA8:
   case GL_R11F_G11F_B10F:
   case GL_R8I:
   case GL_R8UI:
   case GL_R16I:
   case GL_R16UI:
   case GL_R32I:
   case GL_R32UI:
   case GL_RG8I:
   case GL_RG8UI:
   case GL_RG16I:
   case GL_RG16UI:
   case GL_RG32I:
   case GL_RG32UI:
   case GL_RGBA8I:
   case GL_RGBA8UI:
   case GL_RGBA16I:
   case GL_RGBA16UI:
   case GL_RGBA32I:
   case GL_RGBA32UI:
      return true;
   case GL_R16F:
   case GL_RG16F:
   case GL_RGB16F:
   case GL_RGBA16F:
      return _mesa_has_EXT_color_buffer_half_float(ctx);
   case GL_R32F:
   case GL_RG32F:
   case GL_RGBA32F:
      return _mesa_has_EXT_color_buffer_float(ctx);
   case GL_R16:
   case GL_RG16:
   case GL_RGBA16:
      return _mesa_has_EXT_texture_norm16(ctx);
   case GL_R8_SNORM:
   case GL_RG8_SNORM:
   case GL_RGBA8_SNORM:
      return _mesa_has_EXT_render_snorm(ctx);
   case GL_R16_SNORM:
   case GL_RG16_SNORM:
   case GL_RGBA16_SNORM:
      return _mesa_has_EXT_texture_norm16(ctx) &&
             _mesa_has_EXT_render_snorm(ctx);
   default:
      return false;
   }
}

/**
 * Returns true if \p internal_format is a sized internal format that
 * is marked "Texture Filterable" in Table 8.10 of the ES 3.2 specification.
 */
bool
_mesa_is_es3_texture_filterable(const struct gl_context *ctx,
                                GLenum internal_format)
{
   switch (internal_format) {
   case GL_R8:
   case GL_R8_SNORM:
   case GL_RG8:
   case GL_RG8_SNORM:
   case GL_RGB8:
   case GL_RGB8_SNORM:
   case GL_RGB565:
   case GL_RGBA4:
   case GL_RGB5_A1:
   case GL_RGBA8:
   case GL_RGBA8_SNORM:
   case GL_RGB10_A2:
   case GL_SRGB8:
   case GL_SRGB8_ALPHA8:
   case GL_R16F:
   case GL_RG16F:
   case GL_RGB16F:
   case GL_RGBA16F:
   case GL_R11F_G11F_B10F:
   case GL_RGB9_E5:
      return true;
   case GL_R16:
   case GL_R16_SNORM:
   case GL_RG16:
   case GL_RG16_SNORM:
   case GL_RGB16:
   case GL_RGB16_SNORM:
   case GL_RGBA16:
   case GL_RGBA16_SNORM:
      return _mesa_has_EXT_texture_norm16(ctx);
   case GL_R32F:
   case GL_RG32F:
   case GL_RGB32F:
   case GL_RGBA32F:
      /* The OES_texture_float_linear spec says:
       *
       *    "When implemented against OpenGL ES 3.0 or later versions, sized
       *     32-bit floating-point formats become texture-filterable. This
       *     should be noted by, for example, checking the ``TF'' column of
       *     table 8.13 in the ES 3.1 Specification (``Correspondence of sized
       *     internal formats to base internal formats ... and use cases ...'')
       *     for the R32F, RG32F, RGB32F, and RGBA32F formats."
       */
      return _mesa_has_OES_texture_float_linear(ctx);
   default:
      return false;
   }
}
