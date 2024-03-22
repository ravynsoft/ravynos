/**************************************************************************
 *
 * Copyright 2008 VMware, Inc.
 * All Rights Reserved.
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

/**
 * @file
 * Functions to produce packed colors/Z from floats.
 */


#ifndef U_PACK_COLOR_H
#define U_PACK_COLOR_H


#include "util/compiler.h"
#include "util/format/u_formats.h"
#include "util/format/u_format.h"
#include "util/u_math.h"


/**
 * Helper union for packing pixel values.
 * Will often contain values in formats which are too complex to be described
 * in simple terms, hence might just effectively contain a number of bytes.
 * Must be big enough to hold data for all formats (currently 256 bits).
 */
union util_color {
   uint8_t ub;
   uint16_t us;
   uint32_t ui[4];
   uint16_t h[4]; /* half float */
   float f[4];
   double d[4];
};

/**
 * Pack uint8 R,G,B,A into dest pixel.
 */
static inline void
util_pack_color_ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a,
                   enum pipe_format format, union util_color *uc)
{
   switch (format) {
   case PIPE_FORMAT_ABGR8888_UNORM:
      {
         uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | a;
      }
      return;
   case PIPE_FORMAT_XBGR8888_UNORM:
      {
         uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | 0xff;
      }
      return;
   case PIPE_FORMAT_BGRA8888_UNORM:
      {
         uc->ui[0] = (a << 24) | (r << 16) | (g << 8) | b;
      }
      return;
   case PIPE_FORMAT_BGRX8888_UNORM:
      {
         uc->ui[0] = (0xff << 24) | (r << 16) | (g << 8) | b;
      }
      return;
   case PIPE_FORMAT_ARGB8888_UNORM:
      {
         uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | a;
      }
      return;
   case PIPE_FORMAT_XRGB8888_UNORM:
      {
         uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | 0xff;
      }
      return;
   case PIPE_FORMAT_B5G6R5_UNORM:
      {
         uc->us = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      {
         uc->us = ((0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      {
         uc->us = ((a & 0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      {
         uc->us = ((a & 0xf0) << 8) | ((r & 0xf0) << 4) | ((g & 0xf0) << 0) | (b >> 4);
      }
      return;
   case PIPE_FORMAT_A8_UNORM:
      {
         uc->ub = a;
      }
      return;
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
      {
         uc->ub = r;
      }
      return;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      {
         uc->f[0] = (float)r / 255.0f;
         uc->f[1] = (float)g / 255.0f;
         uc->f[2] = (float)b / 255.0f;
         uc->f[3] = (float)a / 255.0f;
      }
      return;
   case PIPE_FORMAT_R32G32B32_FLOAT:
      {
         uc->f[0] = (float)r / 255.0f;
         uc->f[1] = (float)g / 255.0f;
         uc->f[2] = (float)b / 255.0f;
      }
      return;

   /* Handle other cases with a generic function.
    */
   default:
      {
         uint8_t src[4];

         src[0] = r;
         src[1] = g;
         src[2] = b;
         src[3] = a;
         util_format_write_4ub(format, src, 0, uc, 0, 0, 0, 1, 1);
      }
   }
}
 

/**
 * Unpack RGBA from a packed pixel, returning values as uint8_ts in [0,255].
 */
static inline void
util_unpack_color_ub(enum pipe_format format, union util_color *uc,
                     uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)
{
   switch (format) {
   case PIPE_FORMAT_ABGR8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >> 24) & 0xff);
         *g = (uint8_t) ((p >> 16) & 0xff);
         *b = (uint8_t) ((p >>  8) & 0xff);
         *a = (uint8_t) ((p >>  0) & 0xff);
      }
      return;
   case PIPE_FORMAT_XBGR8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >> 24) & 0xff);
         *g = (uint8_t) ((p >> 16) & 0xff);
         *b = (uint8_t) ((p >>  8) & 0xff);
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_BGRA8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >> 16) & 0xff);
         *g = (uint8_t) ((p >>  8) & 0xff);
         *b = (uint8_t) ((p >>  0) & 0xff);
         *a = (uint8_t) ((p >> 24) & 0xff);
      }
      return;
   case PIPE_FORMAT_BGRX8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >> 16) & 0xff);
         *g = (uint8_t) ((p >>  8) & 0xff);
         *b = (uint8_t) ((p >>  0) & 0xff);
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_ARGB8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >>  8) & 0xff);
         *g = (uint8_t) ((p >> 16) & 0xff);
         *b = (uint8_t) ((p >> 24) & 0xff);
         *a = (uint8_t) ((p >>  0) & 0xff);
      }
      return;
   case PIPE_FORMAT_XRGB8888_UNORM:
      {
         uint32_t p = uc->ui[0];
         *r = (uint8_t) ((p >>  8) & 0xff);
         *g = (uint8_t) ((p >> 16) & 0xff);
         *b = (uint8_t) ((p >> 24) & 0xff);
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_B5G6R5_UNORM:
      {
         uint16_t p = uc->us;
         *r = (uint8_t) (((p >> 8) & 0xf8) | ((p >> 13) & 0x7));
         *g = (uint8_t) (((p >> 3) & 0xfc) | ((p >>  9) & 0x3));
         *b = (uint8_t) (((p << 3) & 0xf8) | ((p >>  2) & 0x7));
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      {
         uint16_t p = uc->us;
         *r = (uint8_t) (((p >>  7) & 0xf8) | ((p >> 12) & 0x7));
         *g = (uint8_t) (((p >>  2) & 0xf8) | ((p >>  7) & 0x7));
         *b = (uint8_t) (((p <<  3) & 0xf8) | ((p >>  2) & 0x7));
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      {
         uint16_t p = uc->us;
         *r = (uint8_t) (((p >>  7) & 0xf8) | ((p >> 12) & 0x7));
         *g = (uint8_t) (((p >>  2) & 0xf8) | ((p >>  7) & 0x7));
         *b = (uint8_t) (((p <<  3) & 0xf8) | ((p >>  2) & 0x7));
         *a = (uint8_t) (0xff * (p >> 15));
      }
      return;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      {
         uint16_t p = uc->us;
         *r = (uint8_t) (((p >> 4) & 0xf0) | ((p >>  8) & 0xf));
         *g = (uint8_t) (((p >> 0) & 0xf0) | ((p >>  4) & 0xf));
         *b = (uint8_t) (((p << 4) & 0xf0) | ((p >>  0) & 0xf));
         *a = (uint8_t) (((p >> 8) & 0xf0) | ((p >> 12) & 0xf));
      }
      return;
   case PIPE_FORMAT_A8_UNORM:
      {
         uint8_t p = uc->ub;
         *r = *g = *b = (uint8_t) 0xff;
         *a = p;
      }
      return;
   case PIPE_FORMAT_L8_UNORM:
      {
         uint8_t p = uc->ub;
         *r = *g = *b = p;
         *a = (uint8_t) 0xff;
      }
      return;
   case PIPE_FORMAT_I8_UNORM:
      {
         uint8_t p = uc->ub;
         *r = *g = *b = *a = p;
      }
      return;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      {
         const float *p = &uc->f[0];
         *r = float_to_ubyte(p[0]);
         *g = float_to_ubyte(p[1]);
         *b = float_to_ubyte(p[2]);
         *a = float_to_ubyte(p[3]);
      }
      return;
   case PIPE_FORMAT_R32G32B32_FLOAT:
      {
         const float *p = &uc->f[0];
         *r = float_to_ubyte(p[0]);
         *g = float_to_ubyte(p[1]);
         *b = float_to_ubyte(p[2]);
         *a = (uint8_t) 0xff;
      }
      return;

   case PIPE_FORMAT_R32G32_FLOAT:
      {
         const float *p = &uc->f[0];
         *r = float_to_ubyte(p[0]);
         *g = float_to_ubyte(p[1]);
         *b = *a = (uint8_t) 0xff;
      }
      return;

   case PIPE_FORMAT_R32_FLOAT:
      {
         const float *p = &uc->f[0];
         *r = float_to_ubyte(p[0]);
         *g = *b = *a = (uint8_t) 0xff;
      }
      return;

   /* Handle other cases with a generic function.
    */
   default:
      {
         uint8_t dst[4];

         util_format_read_4ub(format, dst, 0, uc, 0, 0, 0, 1, 1);
         *r = dst[0];
         *g = dst[1];
         *b = dst[2];
         *a = dst[3];
      }
   }
}


/**
 * Note rgba outside [0,1] will be clamped for int pixel formats.
 * This will not work (and might not really be useful with float input)
 * for pure integer formats (which lack the pack_rgba_float function).
 */
static inline void
util_pack_color(const float rgba[4], enum pipe_format format, union util_color *uc)
{
   uint8_t r = 0;
   uint8_t g = 0;
   uint8_t b = 0;
   uint8_t a = 0;

   if (util_format_get_component_bits(format, UTIL_FORMAT_COLORSPACE_RGB, 0) <= 8) {
      /* format uses 8-bit components or less */
      r = float_to_ubyte(rgba[0]);
      g = float_to_ubyte(rgba[1]);
      b = float_to_ubyte(rgba[2]);
      a = float_to_ubyte(rgba[3]);
   }

   switch (format) {
   case PIPE_FORMAT_ABGR8888_UNORM:
      {
         uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | a;
      }
      return;
   case PIPE_FORMAT_XBGR8888_UNORM:
      {
         uc->ui[0] = (r << 24) | (g << 16) | (b << 8) | 0xff;
      }
      return;
   case PIPE_FORMAT_BGRA8888_UNORM:
      {
         uc->ui[0] = (a << 24) | (r << 16) | (g << 8) | b;
      }
      return;
   case PIPE_FORMAT_BGRX8888_UNORM:
      {
         uc->ui[0] = (0xffu << 24) | (r << 16) | (g << 8) | b;
      }
      return;
   case PIPE_FORMAT_ARGB8888_UNORM:
      {
         uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | a;
      }
      return;
   case PIPE_FORMAT_XRGB8888_UNORM:
      {
         uc->ui[0] = (b << 24) | (g << 16) | (r << 8) | 0xff;
      }
      return;
   case PIPE_FORMAT_B5G6R5_UNORM:
      {
         uc->us = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B5G5R5X1_UNORM:
      {
         uc->us = ((0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B5G5R5A1_UNORM:
      {
         uc->us = ((a & 0x80) << 8) | ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | (b >> 3);
      }
      return;
   case PIPE_FORMAT_B4G4R4A4_UNORM:
      {
         uc->us = ((a & 0xf0) << 8) | ((r & 0xf0) << 4) | ((g & 0xf0) << 0) | (b >> 4);
      }
      return;
   case PIPE_FORMAT_A8_UNORM:
      {
         uc->ub = a;
      }
      return;
   case PIPE_FORMAT_L8_UNORM:
   case PIPE_FORMAT_I8_UNORM:
      {
         uc->ub = r;
      }
      return;
   case PIPE_FORMAT_R32G32B32A32_FLOAT:
      {
         uc->f[0] = rgba[0];
         uc->f[1] = rgba[1];
         uc->f[2] = rgba[2];
         uc->f[3] = rgba[3];
      }
      return;
   case PIPE_FORMAT_R32G32B32_FLOAT:
      {
         uc->f[0] = rgba[0];
         uc->f[1] = rgba[1];
         uc->f[2] = rgba[2];
      }
      return;

   /* Handle other cases with a generic function.
    */
   default:
      util_format_pack_rgba(format, uc, rgba, 1);
   }
}

static inline void
util_pack_color_union(enum pipe_format format,
                      union util_color *dst,
                      const union pipe_color_union *src)
{
   util_format_pack_rgba(format, dst, &src->ui, 1);
}

/* Integer versions of util_pack_z and util_pack_z_stencil - useful for
 * constructing clear masks.
 */
static inline uint32_t
util_pack_mask_z(enum pipe_format format, uint32_t z)
{
   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      return z & 0xffff;
   case PIPE_FORMAT_Z32_UNORM:
   case PIPE_FORMAT_Z32_FLOAT:
      return z;
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
      return z & 0xffffff;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
   case PIPE_FORMAT_X8Z24_UNORM:
      return (z & 0xffffff) << 8;
   case PIPE_FORMAT_S8_UINT:
      return 0;
   default:
      debug_printf("gallium: unhandled format in util_pack_mask_z(): %s\n",
                   util_format_name(format));
      assert(0);
      return 0;
   }
}


static inline uint64_t
util_pack64_mask_z(enum pipe_format format, uint32_t z)
{
   switch (format) {
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      return z;
   default:
      return util_pack_mask_z(format, z);
   }
}


static inline uint32_t
util_pack_mask_z_stencil(enum pipe_format format, uint32_t z, uint8_t s)
{
   uint32_t packed = util_pack_mask_z(format, z);

   switch (format) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      packed |= (uint32_t)s << 24;
      break;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      packed |= s;
      break;
   case PIPE_FORMAT_S8_UINT:
      packed |= s;
      break;
   default:
      break;
   }

   return packed;
}


static inline uint64_t
util_pack64_mask_z_stencil(enum pipe_format format, uint32_t z, uint8_t s)
{
   uint64_t packed;

   switch (format) {
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      packed = util_pack64_mask_z(format, z);
      packed |= (uint64_t)s << 32ull;
      return packed;
   default:
      return util_pack_mask_z_stencil(format, z, s);
   }
}


/**
 * Note: it's assumed that z is in [0,1]
 */
static inline uint32_t
util_pack_z(enum pipe_format format, double z)
{
   union fi fui;

   if (format == PIPE_FORMAT_Z32_FLOAT) {
      fui.f = (float)z;
      return fui.ui;
   }

   if (z <= 0.0)
      return 0;

   switch (format) {
   case PIPE_FORMAT_Z16_UNORM:
      if (z >= 1.0)
         return 0xffff;
      return (uint32_t) lrint(z * 0xffff);
   case PIPE_FORMAT_Z32_UNORM:
      /* special-case to avoid overflow */
      if (z >= 1.0)
         return 0xffffffff;
      return (uint32_t) llrint(z * 0xffffffff);
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
   case PIPE_FORMAT_Z24X8_UNORM:
      if (z >= 1.0)
         return 0xffffff;
      return (uint32_t) lrint(z * 0xffffff);
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
   case PIPE_FORMAT_X8Z24_UNORM:
      if (z >= 1.0)
         return 0xffffff00;
      return ((uint32_t) lrint(z * 0xffffff)) << 8;
   case PIPE_FORMAT_S8_UINT:
      /* this case can get it via util_pack_z_stencil() */
      return 0;
   default:
      debug_printf("gallium: unhandled format in util_pack_z(): %s\n",
                   util_format_name(format));
      assert(0);
      return 0;
   }
}


static inline uint64_t
util_pack64_z(enum pipe_format format, double z)
{
   union fi fui;

   if (z == 0)
      return 0;

   switch (format) {
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      fui.f = (float)z;
      return fui.ui;
   default:
      return util_pack_z(format, z);
   }
}
 

/**
 * Pack Z and/or stencil values into a 32-bit value described by format.
 * Note: it's assumed that z is in [0,1] and s in [0,255]
 */
static inline uint32_t
util_pack_z_stencil(enum pipe_format format, double z, uint8_t s)
{
   uint32_t packed = util_pack_z(format, z);

   switch (format) {
   case PIPE_FORMAT_Z24_UNORM_S8_UINT:
      packed |= (uint32_t)s << 24;
      break;
   case PIPE_FORMAT_S8_UINT_Z24_UNORM:
      packed |= s;
      break;
   case PIPE_FORMAT_S8_UINT:
      packed |= s;
      break;
   default:
      break;
   }

   return packed;
}


static inline uint64_t
util_pack64_z_stencil(enum pipe_format format, double z, uint8_t s)
{
   uint64_t packed;

   switch (format) {
   case PIPE_FORMAT_Z32_FLOAT_S8X24_UINT:
      packed = util_pack64_z(format, z);
      packed |= (uint64_t)s << 32ull;
      break;
   default:
      return util_pack_z_stencil(format, z, s);
   }

   return packed;
}


/**
 * Pack 4 uint8_ts into a 4-byte word
 */
static inline uint32_t
pack_ub4(uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3)
{
   return ((((uint32_t)b0) << 0) |
	   (((uint32_t)b1) << 8) |
	   (((uint32_t)b2) << 16) |
	   (((uint32_t)b3) << 24));
}


/**
 * Pack/convert 4 floats into one 4-byte word.
 */
static inline uint32_t
pack_ui32_float4(float a, float b, float c, float d)
{
   return pack_ub4( float_to_ubyte(a),
		    float_to_ubyte(b),
		    float_to_ubyte(c),
		    float_to_ubyte(d) );
}



#endif /* U_PACK_COLOR_H */
