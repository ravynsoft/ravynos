/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 * Copyright (C) 2009-2010  VMware, Inc.  All Rights Reserved.
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
 * THEA AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


/**
 * \file pack.c
 * Image and pixel span packing and unpacking.
 */


/*
 * XXX: MSVC takes forever to compile this module for x86_64 unless we disable
 * this global optimization.
 *
 * See also:
 * - http://msdn.microsoft.com/en-us/library/1yk3ydd7.aspx
 * - http://msdn.microsoft.com/en-us/library/chh3fb0k.aspx
 */
#if defined(_MSC_VER) && defined(_M_X64)
#  pragma optimize( "g", off )
#endif


#include "errors.h"
#include "util/glheader.h"
#include "enums.h"
#include "image.h"

#include "macros.h"
#include "mtypes.h"
#include "pack.h"
#include "pixeltransfer.h"

#include "glformats.h"
#include "format_utils.h"
#include "format_pack.h"
#include "format_unpack.h"
#include "util/format/u_format.h"

/**
 * Flip the 8 bits in each byte of the given array.
 *
 * \param p array.
 * \param n number of bytes.
 *
 * \todo try this trick to flip bytes someday:
 * \code
 *  v = ((v & 0x55555555) << 1) | ((v >> 1) & 0x55555555);
 *  v = ((v & 0x33333333) << 2) | ((v >> 2) & 0x33333333);
 *  v = ((v & 0x0f0f0f0f) << 4) | ((v >> 4) & 0x0f0f0f0f);
 * \endcode
 */
static void
flip_bytes( GLubyte *p, GLuint n )
{
   GLuint i, a, b;
   for (i = 0; i < n; i++) {
      b = (GLuint) p[i];        /* words are often faster than bytes */
      a = ((b & 0x01) << 7) |
	  ((b & 0x02) << 5) |
	  ((b & 0x04) << 3) |
	  ((b & 0x08) << 1) |
	  ((b & 0x10) >> 1) |
	  ((b & 0x20) >> 3) |
	  ((b & 0x40) >> 5) |
	  ((b & 0x80) >> 7);
      p[i] = (GLubyte) a;
   }
}



/*
 * Unpack a 32x32 pixel polygon stipple from user memory using the
 * current pixel unpack settings.
 */
void
_mesa_unpack_polygon_stipple( const GLubyte *pattern, GLuint dest[32],
                              const struct gl_pixelstore_attrib *unpacking )
{
   GLubyte *ptrn = (GLubyte *) _mesa_unpack_image(2, 32, 32, 1, GL_COLOR_INDEX,
                                                  GL_BITMAP, pattern, unpacking);
   if (ptrn) {
      /* Convert pattern from GLubytes to GLuints and handle big/little
       * endian differences
       */
      GLubyte *p = ptrn;
      GLint i;
      for (i = 0; i < 32; i++) {
         dest[i] = (p[0] << 24)
                 | (p[1] << 16)
                 | (p[2] <<  8)
                 | (p[3]      );
         p += 4;
      }
      free(ptrn);
   }
}


/*
 * Pack polygon stipple into user memory given current pixel packing
 * settings.
 */
void
_mesa_pack_polygon_stipple( const GLuint pattern[32], GLubyte *dest,
                            const struct gl_pixelstore_attrib *packing )
{
   /* Convert pattern from GLuints to GLubytes to handle big/little
    * endian differences.
    */
   GLubyte ptrn[32*4];
   GLint i;
   for (i = 0; i < 32; i++) {
      ptrn[i * 4 + 0] = (GLubyte) ((pattern[i] >> 24) & 0xff);
      ptrn[i * 4 + 1] = (GLubyte) ((pattern[i] >> 16) & 0xff);
      ptrn[i * 4 + 2] = (GLubyte) ((pattern[i] >> 8 ) & 0xff);
      ptrn[i * 4 + 3] = (GLubyte) ((pattern[i]      ) & 0xff);
   }

   _mesa_pack_bitmap(32, 32, ptrn, dest, packing);
}


/*
 * Pack bitmap data.
 */
void
_mesa_pack_bitmap( GLint width, GLint height, const GLubyte *source,
                   GLubyte *dest, const struct gl_pixelstore_attrib *packing )
{
   GLint row, width_in_bytes;
   const GLubyte *src;

   if (!source)
      return;

   width_in_bytes = DIV_ROUND_UP( width, 8 );
   src = source;
   for (row = 0; row < height; row++) {
      GLubyte *dst = (GLubyte *) _mesa_image_address2d(packing, dest,
                       width, height, GL_COLOR_INDEX, GL_BITMAP, row, 0);
      if (!dst)
         return;

      if ((packing->SkipPixels & 7) == 0) {
         memcpy( dst, src, width_in_bytes );
         if (packing->LsbFirst) {
            flip_bytes( dst, width_in_bytes );
         }
      }
      else {
         /* handling SkipPixels is a bit tricky (no pun intended!) */
         GLint i;
         if (packing->LsbFirst) {
            GLubyte srcMask = 128;
            GLubyte dstMask = 1 << (packing->SkipPixels & 0x7);
            const GLubyte *s = src;
            GLubyte *d = dst;
            *d = 0;
            for (i = 0; i < width; i++) {
               if (*s & srcMask) {
                  *d |= dstMask;
               }
               if (srcMask == 1) {
                  srcMask = 128;
                  s++;
               }
               else {
                  srcMask = srcMask >> 1;
               }
               if (dstMask == 128) {
                  dstMask = 1;
                  d++;
                  *d = 0;
               }
               else {
                  dstMask = dstMask << 1;
               }
            }
         }
         else {
            GLubyte srcMask = 128;
            GLubyte dstMask = 128 >> (packing->SkipPixels & 0x7);
            const GLubyte *s = src;
            GLubyte *d = dst;
            *d = 0;
            for (i = 0; i < width; i++) {
               if (*s & srcMask) {
                  *d |= dstMask;
               }
               if (srcMask == 1) {
                  srcMask = 128;
                  s++;
               }
               else {
                  srcMask = srcMask >> 1;
               }
               if (dstMask == 1) {
                  dstMask = 128;
                  d++;
                  *d = 0;
               }
               else {
                  dstMask = dstMask >> 1;
               }
            }
         }
      }
      src += width_in_bytes;
   }
}


#define SWAP2BYTE(VALUE)			\
   {						\
      GLubyte *bytes = (GLubyte *) &(VALUE);	\
      GLubyte tmp = bytes[0];			\
      bytes[0] = bytes[1];			\
      bytes[1] = tmp;				\
   }

#define SWAP4BYTE(VALUE)			\
   {						\
      GLubyte *bytes = (GLubyte *) &(VALUE);	\
      GLubyte tmp = bytes[0];			\
      bytes[0] = bytes[3];			\
      bytes[3] = tmp;				\
      tmp = bytes[1];				\
      bytes[1] = bytes[2];			\
      bytes[2] = tmp;				\
   }


static void
extract_uint_indexes(GLuint n, GLuint indexes[],
                     GLenum srcFormat, GLenum srcType, const GLvoid *src,
                     const struct gl_pixelstore_attrib *unpack )
{
   assert(srcFormat == GL_COLOR_INDEX || srcFormat == GL_STENCIL_INDEX);

   assert(srcType == GL_BITMAP ||
          srcType == GL_UNSIGNED_BYTE ||
          srcType == GL_BYTE ||
          srcType == GL_UNSIGNED_SHORT ||
          srcType == GL_SHORT ||
          srcType == GL_UNSIGNED_INT ||
          srcType == GL_INT ||
          srcType == GL_UNSIGNED_INT_24_8_EXT ||
          srcType == GL_HALF_FLOAT_ARB ||
          srcType == GL_HALF_FLOAT_OES ||
          srcType == GL_FLOAT ||
          srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   switch (srcType) {
      case GL_BITMAP:
         {
            GLubyte *ubsrc = (GLubyte *) src;
            if (unpack->LsbFirst) {
               GLubyte mask = 1 << (unpack->SkipPixels & 0x7);
               GLuint i;
               for (i = 0; i < n; i++) {
                  indexes[i] = (*ubsrc & mask) ? 1 : 0;
                  if (mask == 128) {
                     mask = 1;
                     ubsrc++;
                  }
                  else {
                     mask = mask << 1;
                  }
               }
            }
            else {
               GLubyte mask = 128 >> (unpack->SkipPixels & 0x7);
               GLuint i;
               for (i = 0; i < n; i++) {
                  indexes[i] = (*ubsrc & mask) ? 1 : 0;
                  if (mask == 1) {
                     mask = 128;
                     ubsrc++;
                  }
                  else {
                     mask = mask >> 1;
                  }
               }
            }
         }
         break;
      case GL_UNSIGNED_BYTE:
         {
            GLuint i;
            const GLubyte *s = (const GLubyte *) src;
            for (i = 0; i < n; i++)
               indexes[i] = s[i];
         }
         break;
      case GL_BYTE:
         {
            GLuint i;
            const GLbyte *s = (const GLbyte *) src;
            for (i = 0; i < n; i++)
               indexes[i] = s[i];
         }
         break;
      case GL_UNSIGNED_SHORT:
         {
            GLuint i;
            const GLushort *s = (const GLushort *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLushort value = s[i];
                  SWAP2BYTE(value);
                  indexes[i] = value;
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i];
            }
         }
         break;
      case GL_SHORT:
         {
            GLuint i;
            const GLshort *s = (const GLshort *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLshort value = s[i];
                  SWAP2BYTE(value);
                  indexes[i] = value;
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i];
            }
         }
         break;
      case GL_UNSIGNED_INT:
         {
            GLuint i;
            const GLuint *s = (const GLuint *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLuint value = s[i];
                  SWAP4BYTE(value);
                  indexes[i] = value;
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i];
            }
         }
         break;
      case GL_INT:
         {
            GLuint i;
            const GLint *s = (const GLint *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLint value = s[i];
                  SWAP4BYTE(value);
                  indexes[i] = value;
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i];
            }
         }
         break;
      case GL_FLOAT:
         {
            GLuint i;
            const GLfloat *s = (const GLfloat *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLfloat value = s[i];
                  SWAP4BYTE(value);
                  indexes[i] = (GLuint) value;
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = (GLuint) s[i];
            }
         }
         break;
      case GL_HALF_FLOAT_ARB:
      case GL_HALF_FLOAT_OES:
         {
            GLuint i;
            const GLhalfARB *s = (const GLhalfARB *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLhalfARB value = s[i];
                  SWAP2BYTE(value);
                  indexes[i] = (GLuint) _mesa_half_to_float(value);
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = (GLuint) _mesa_half_to_float(s[i]);
            }
         }
         break;
      case GL_UNSIGNED_INT_24_8_EXT:
         {
            GLuint i;
            const GLuint *s = (const GLuint *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLuint value = s[i];
                  SWAP4BYTE(value);
                  indexes[i] = value & 0xff;  /* lower 8 bits */
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i] & 0xff;  /* lower 8 bits */
            }
         }
         break;
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         {
            GLuint i;
            const GLuint *s = (const GLuint *) src;
            if (unpack->SwapBytes) {
               for (i = 0; i < n; i++) {
                  GLuint value = s[i*2+1];
                  SWAP4BYTE(value);
                  indexes[i] = value & 0xff;  /* lower 8 bits */
               }
            }
            else {
               for (i = 0; i < n; i++)
                  indexes[i] = s[i*2+1] & 0xff;  /* lower 8 bits */
            }
         }
         break;

      default:
         unreachable("bad srcType in extract_uint_indexes");
   }
}


/*
 * Unpack a row of stencil data from a client buffer according to
 * the pixel unpacking parameters.
 * This is (or will be) used by glDrawPixels
 *
 * Args:  ctx - the context
 *        n - number of pixels
 *        dstType - destination data type
 *        dest - destination array
 *        srcType - source pixel type
 *        source - source data pointer
 *        srcPacking - pixel unpacking parameters
 *        transferOps - apply offset/bias/lookup ops?
 */
void
_mesa_unpack_stencil_span( struct gl_context *ctx, GLuint n,
                           GLenum dstType, GLvoid *dest,
                           GLenum srcType, const GLvoid *source,
                           const struct gl_pixelstore_attrib *srcPacking,
                           GLbitfield transferOps )
{
   assert(srcType == GL_BITMAP ||
          srcType == GL_UNSIGNED_BYTE ||
          srcType == GL_BYTE ||
          srcType == GL_UNSIGNED_SHORT ||
          srcType == GL_SHORT ||
          srcType == GL_UNSIGNED_INT ||
          srcType == GL_INT ||
          srcType == GL_UNSIGNED_INT_24_8_EXT ||
          srcType == GL_HALF_FLOAT_ARB ||
          srcType == GL_HALF_FLOAT_OES ||
          srcType == GL_FLOAT ||
          srcType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   assert(dstType == GL_UNSIGNED_BYTE ||
          dstType == GL_UNSIGNED_SHORT ||
          dstType == GL_UNSIGNED_INT ||
          dstType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

   /* only shift and offset apply to stencil */
   transferOps &= IMAGE_SHIFT_OFFSET_BIT;

   /*
    * Try simple cases first
    */
   if (transferOps == 0 &&
       !ctx->Pixel.MapStencilFlag &&
       srcType == GL_UNSIGNED_BYTE &&
       dstType == GL_UNSIGNED_BYTE) {
      memcpy(dest, source, n * sizeof(GLubyte));
   }
   else if (transferOps == 0 &&
            !ctx->Pixel.MapStencilFlag &&
            srcType == GL_UNSIGNED_INT &&
            dstType == GL_UNSIGNED_INT &&
            !srcPacking->SwapBytes) {
      memcpy(dest, source, n * sizeof(GLuint));
   }
   else {
      /*
       * general solution
       */
      GLuint *indexes = malloc(n * sizeof(GLuint));

      if (!indexes) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "stencil unpacking");
         return;
      }

      extract_uint_indexes(n, indexes, GL_STENCIL_INDEX, srcType, source,
                           srcPacking);

      if (transferOps & IMAGE_SHIFT_OFFSET_BIT) {
         /* shift and offset indexes */
         _mesa_shift_and_offset_ci(ctx, n, indexes);
      }

      if (ctx->Pixel.MapStencilFlag) {
         /* Apply stencil lookup table */
         const GLuint mask = ctx->PixelMaps.StoS.Size - 1;
         GLuint i;
         for (i = 0; i < n; i++) {
            indexes[i] = (GLuint)ctx->PixelMaps.StoS.Map[ indexes[i] & mask ];
         }
      }

      /* convert to dest type */
      switch (dstType) {
         case GL_UNSIGNED_BYTE:
            {
               GLubyte *dst = (GLubyte *) dest;
               GLuint i;
               for (i = 0; i < n; i++) {
                  dst[i] = (GLubyte) (indexes[i] & 0xff);
               }
            }
            break;
         case GL_UNSIGNED_SHORT:
            {
               GLuint *dst = (GLuint *) dest;
               GLuint i;
               for (i = 0; i < n; i++) {
                  dst[i] = (GLushort) (indexes[i] & 0xffff);
               }
            }
            break;
         case GL_UNSIGNED_INT:
            memcpy(dest, indexes, n * sizeof(GLuint));
            break;
         case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
            {
               GLuint *dst = (GLuint *) dest;
               GLuint i;
               for (i = 0; i < n; i++) {
                  dst[i*2+1] = indexes[i] & 0xff; /* lower 8 bits */
               }
            }
            break;
         default:
            unreachable("bad dstType in _mesa_unpack_stencil_span");
      }

      free(indexes);
   }
}


void
_mesa_pack_stencil_span( struct gl_context *ctx, GLuint n,
                         GLenum dstType, GLvoid *dest, const GLubyte *source,
                         const struct gl_pixelstore_attrib *dstPacking )
{
   GLubyte *stencil = malloc(n * sizeof(GLubyte));

   if (!stencil) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "stencil packing");
      return;
   }

   if (ctx->Pixel.IndexShift || ctx->Pixel.IndexOffset ||
       ctx->Pixel.MapStencilFlag) {
      /* make a copy of input */
      memcpy(stencil, source, n * sizeof(GLubyte));
      _mesa_apply_stencil_transfer_ops(ctx, n, stencil);
      source = stencil;
   }

   switch (dstType) {
   case GL_UNSIGNED_BYTE:
      memcpy(dest, source, n);
      break;
   case GL_BYTE:
      {
         GLbyte *dst = (GLbyte *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLbyte) (source[i] & 0x7f);
         }
      }
      break;
   case GL_UNSIGNED_SHORT:
      {
         GLushort *dst = (GLushort *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLushort) source[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   case GL_SHORT:
      {
         GLshort *dst = (GLshort *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLshort) source[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   case GL_UNSIGNED_INT:
      {
         GLuint *dst = (GLuint *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLuint) source[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_INT:
      {
         GLint *dst = (GLint *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLint) source[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_FLOAT:
      {
         GLfloat *dst = (GLfloat *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = (GLfloat) source[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_HALF_FLOAT_ARB:
   case GL_HALF_FLOAT_OES:
      {
         GLhalfARB *dst = (GLhalfARB *) dest;
         GLuint i;
         for (i=0;i<n;i++) {
            dst[i] = _mesa_float_to_half( (float) source[i] );
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   case GL_BITMAP:
      if (dstPacking->LsbFirst) {
         GLubyte *dst = (GLubyte *) dest;
         GLint shift = 0;
         GLuint i;
         for (i = 0; i < n; i++) {
            if (shift == 0)
               *dst = 0;
            *dst |= ((source[i] != 0) << shift);
            shift++;
            if (shift == 8) {
               shift = 0;
               dst++;
            }
         }
      }
      else {
         GLubyte *dst = (GLubyte *) dest;
         GLint shift = 7;
         GLuint i;
         for (i = 0; i < n; i++) {
            if (shift == 7)
               *dst = 0;
            *dst |= ((source[i] != 0) << shift);
            shift--;
            if (shift < 0) {
               shift = 7;
               dst++;
            }
         }
      }
      break;
   default:
      unreachable("bad type in _mesa_pack_index_span");
   }

   free(stencil);
}

#define DEPTH_VALUES(GLTYPE, GLTYPE2FLOAT)                              \
    do {                                                                \
        GLuint i;                                                       \
        const GLTYPE *src = (const GLTYPE *)source;                     \
        for (i = 0; i < n; i++) {                                       \
            GLTYPE value = src[i];                                      \
            if (srcPacking->SwapBytes) {                                \
                if (sizeof(GLTYPE) == 2) {                              \
                    SWAP2BYTE(value);                                   \
                } else if (sizeof(GLTYPE) == 4) {                       \
                    SWAP4BYTE(value);                                   \
                }                                                       \
            }                                                           \
            depthValues[i] = GLTYPE2FLOAT(value);                       \
        }                                                               \
    } while (0)


/**
 * Unpack a row of depth/z values from memory, returning GLushort, GLuint
 * or GLfloat values.
 * The glPixelTransfer (scale/bias) params will be applied.
 *
 * \param dstType  one of GL_UNSIGNED_SHORT, GL_UNSIGNED_INT, GL_FLOAT
 * \param depthMax  max value for returned GLushort or GLuint values
 *                  (ignored for GLfloat).
 */
void
_mesa_unpack_depth_span( struct gl_context *ctx, GLuint n,
                         GLenum dstType, GLvoid *dest, GLuint depthMax,
                         GLenum srcType, const GLvoid *source,
                         const struct gl_pixelstore_attrib *srcPacking )
{
   GLfloat *depthTemp = NULL, *depthValues;
   GLboolean needClamp = GL_FALSE;

   /* Look for special cases first.
    * Not only are these faster, they're less prone to numeric conversion
    * problems.  Otherwise, converting from an int type to a float then
    * back to an int type can introduce errors that will show up as
    * artifacts in things like depth peeling which uses glCopyTexImage.
    */
   if (ctx->Pixel.DepthScale == 1.0F && ctx->Pixel.DepthBias == 0.0F) {
      if (srcType == GL_UNSIGNED_INT && dstType == GL_UNSIGNED_SHORT) {
         const GLuint *src = (const GLuint *) source;
         GLushort *dst = (GLushort *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = src[i] >> 16;
         }
         return;
      }
      if (srcType == GL_UNSIGNED_SHORT
          && dstType == GL_UNSIGNED_INT
          && depthMax == 0xffffffff) {
         const GLushort *src = (const GLushort *) source;
         GLuint *dst = (GLuint *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = src[i] | (src[i] << 16);
         }
         return;
      }
      if (srcType == GL_UNSIGNED_INT_24_8
          && dstType == GL_UNSIGNED_INT
          && depthMax == 0xffffff) {
         const GLuint *src = (const GLuint *) source;
         GLuint *dst = (GLuint *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = src[i] >> 8;
         }
         return;
      }
      /* XXX may want to add additional cases here someday */
   }

   /* general case path follows */

   if (dstType == GL_FLOAT) {
      depthValues = (GLfloat *) dest;
   }
   else {
      depthTemp = malloc(n * sizeof(GLfloat));
      if (!depthTemp) {
         _mesa_error(ctx, GL_OUT_OF_MEMORY, "pixel unpacking");
         return;
      }

      depthValues = depthTemp;
   }

   /* Convert incoming values to GLfloat.  Some conversions will require
    * clamping, below.
    */
   switch (srcType) {
      case GL_BYTE:
         DEPTH_VALUES(GLbyte, BYTE_TO_FLOATZ);
         needClamp = GL_TRUE;
         break;
      case GL_UNSIGNED_BYTE:
         DEPTH_VALUES(GLubyte, UBYTE_TO_FLOAT);
         break;
      case GL_SHORT:
         DEPTH_VALUES(GLshort, SHORT_TO_FLOATZ);
         needClamp = GL_TRUE;
         break;
      case GL_UNSIGNED_SHORT:
         DEPTH_VALUES(GLushort, USHORT_TO_FLOAT);
         break;
      case GL_INT:
         DEPTH_VALUES(GLint, INT_TO_FLOAT);
         needClamp = GL_TRUE;
         break;
      case GL_UNSIGNED_INT:
         DEPTH_VALUES(GLuint, UINT_TO_FLOAT);
         break;
      case GL_UNSIGNED_INT_24_8_EXT: /* GL_EXT_packed_depth_stencil */
         if (dstType == GL_UNSIGNED_INT_24_8_EXT &&
             depthMax == 0xffffff &&
             ctx->Pixel.DepthScale == 1.0F &&
             ctx->Pixel.DepthBias == 0.0F) {
            const GLuint *src = (const GLuint *) source;
            GLuint *zValues = (GLuint *) dest;
            GLuint i;
            for (i = 0; i < n; i++) {
                GLuint value = src[i];
                if (srcPacking->SwapBytes) {
                    SWAP4BYTE(value);
                }
                zValues[i] = value & 0xffffff00;
            }
            free(depthTemp);
            return;
         }
         else {
            const GLuint *src = (const GLuint *) source;
            const GLfloat scale = 1.0f / 0xffffff;
            GLuint i;
            for (i = 0; i < n; i++) {
                GLuint value = src[i];
                if (srcPacking->SwapBytes) {
                    SWAP4BYTE(value);
                }
                depthValues[i] = (value >> 8) * scale;
            }
         }
         break;
      case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
         {
            GLuint i;
            const GLfloat *src = (const GLfloat *)source;
            for (i = 0; i < n; i++) {
               GLfloat value = src[i * 2];
               if (srcPacking->SwapBytes) {
                  SWAP4BYTE(value);
               }
               depthValues[i] = value;
            }
            needClamp = GL_TRUE;
         }
         break;
      case GL_FLOAT:
         DEPTH_VALUES(GLfloat, 1*);
         needClamp = GL_TRUE;
         break;
      case GL_HALF_FLOAT_ARB:
      case GL_HALF_FLOAT_OES:
         {
            GLuint i;
            const GLhalfARB *src = (const GLhalfARB *) source;
            for (i = 0; i < n; i++) {
               GLhalfARB value = src[i];
               if (srcPacking->SwapBytes) {
                  SWAP2BYTE(value);
               }
               depthValues[i] = _mesa_half_to_float(value);
            }
            needClamp = GL_TRUE;
         }
         break;
      default:
         _mesa_problem(NULL, "bad type in _mesa_unpack_depth_span()");
         free(depthTemp);
         return;
   }

   /* apply depth scale and bias */
   {
      const GLfloat scale = ctx->Pixel.DepthScale;
      const GLfloat bias = ctx->Pixel.DepthBias;
      if (scale != 1.0F || bias != 0.0F) {
         GLuint i;
         for (i = 0; i < n; i++) {
            depthValues[i] = depthValues[i] * scale + bias;
         }
         needClamp = GL_TRUE;
      }
   }

   /* clamp to [0, 1] */
   if (needClamp) {
      GLuint i;
      for (i = 0; i < n; i++) {
         depthValues[i] = CLAMP(depthValues[i], 0.0F, 1.0F);
      }
   }

   /*
    * Convert values to dstType
    */
   if (dstType == GL_UNSIGNED_INT) {
      GLuint *zValues = (GLuint *) dest;
      GLuint i;
      if (depthMax <= 0xffffff) {
         /* no overflow worries */
         for (i = 0; i < n; i++) {
            zValues[i] = (GLuint) (depthValues[i] * (GLfloat) depthMax);
         }
      }
      else {
         /* need to use double precision to prevent overflow problems */
         for (i = 0; i < n; i++) {
            GLdouble z = depthValues[i] * (GLdouble) depthMax;
            if (z >= (GLdouble) 0xffffffff)
               zValues[i] = 0xffffffff;
            else
               zValues[i] = (GLuint) z;
         }
      }
   }
   else if (dstType == GL_UNSIGNED_SHORT) {
      GLushort *zValues = (GLushort *) dest;
      GLuint i;
      assert(depthMax <= 0xffff);
      for (i = 0; i < n; i++) {
         zValues[i] = (GLushort) (depthValues[i] * (GLfloat) depthMax);
      }
   }
   else if (dstType == GL_FLOAT) {
      /* Nothing to do. depthValues is pointing to dest. */
   }
   else if (dstType == GL_FLOAT_32_UNSIGNED_INT_24_8_REV) {
      GLfloat *zValues = (GLfloat*) dest;
      GLuint i;
      for (i = 0; i < n; i++) {
         zValues[i*2] = depthValues[i];
      }
   }
   else {
      assert(0);
   }

   free(depthTemp);
}


/*
 * Pack an array of depth values.  The values are floats in [0,1].
 */
void
_mesa_pack_depth_span( struct gl_context *ctx, GLuint n, GLvoid *dest,
                       GLenum dstType, const GLfloat *depthSpan,
                       const struct gl_pixelstore_attrib *dstPacking )
{
   GLfloat *depthCopy = malloc(n * sizeof(GLfloat));
   if (!depthCopy) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "pixel packing");
      return;
   }

   if (ctx->Pixel.DepthScale != 1.0F || ctx->Pixel.DepthBias != 0.0F) {
      memcpy(depthCopy, depthSpan, n * sizeof(GLfloat));
      _mesa_scale_and_bias_depth(ctx, n, depthCopy);
      depthSpan = depthCopy;
   }

   switch (dstType) {
   case GL_UNSIGNED_BYTE:
      {
         GLubyte *dst = (GLubyte *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = FLOAT_TO_UBYTE( depthSpan[i] );
         }
      }
      break;
   case GL_BYTE:
      {
         GLbyte *dst = (GLbyte *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = FLOAT_TO_BYTE( depthSpan[i] );
         }
      }
      break;
   case GL_UNSIGNED_SHORT:
      {
         GLushort *dst = (GLushort *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            CLAMPED_FLOAT_TO_USHORT(dst[i], depthSpan[i]);
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   case GL_SHORT:
      {
         GLshort *dst = (GLshort *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = FLOAT_TO_SHORT( depthSpan[i] );
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   case GL_UNSIGNED_INT_24_8:
      {
         const GLdouble scale = (GLdouble) 0xffffff;
         GLuint *dst = (GLuint *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            GLuint z = (GLuint) (depthSpan[i] * scale);
            assert(z <= 0xffffff);
            dst[i] = (z << 8);
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
         break;
      }
   case GL_UNSIGNED_INT:
      {
         GLuint *dst = (GLuint *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = FLOAT_TO_UINT( depthSpan[i] );
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_INT:
      {
         GLint *dst = (GLint *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = FLOAT_TO_INT( depthSpan[i] );
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_FLOAT:
      {
         GLfloat *dst = (GLfloat *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = depthSpan[i];
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap4( (GLuint *) dst, n );
         }
      }
      break;
   case GL_HALF_FLOAT_ARB:
   case GL_HALF_FLOAT_OES:
      {
         GLhalfARB *dst = (GLhalfARB *) dest;
         GLuint i;
         for (i = 0; i < n; i++) {
            dst[i] = _mesa_float_to_half(depthSpan[i]);
         }
         if (dstPacking->SwapBytes) {
            _mesa_swap2( (GLushort *) dst, n );
         }
      }
      break;
   default:
      unreachable("bad type in _mesa_pack_depth_span()");
   }

   free(depthCopy);
}



/**
 * Pack depth and stencil values as GL_DEPTH_STENCIL (GL_UNSIGNED_INT_24_8 etc)
 */
void
_mesa_pack_depth_stencil_span(struct gl_context *ctx,GLuint n,
                              GLenum dstType, GLuint *dest,
                              const GLfloat *depthVals,
                              const GLubyte *stencilVals,
                              const struct gl_pixelstore_attrib *dstPacking)
{
   GLfloat *depthCopy = malloc(n * sizeof(GLfloat));
   GLubyte *stencilCopy = malloc(n * sizeof(GLubyte));
   GLuint i;

   if (!depthCopy || !stencilCopy) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "pixel packing");
      free(depthCopy);
      free(stencilCopy);
      return;
   }

   if (ctx->Pixel.DepthScale != 1.0F || ctx->Pixel.DepthBias != 0.0F) {
      memcpy(depthCopy, depthVals, n * sizeof(GLfloat));
      _mesa_scale_and_bias_depth(ctx, n, depthCopy);
      depthVals = depthCopy;
   }

   if (ctx->Pixel.IndexShift ||
       ctx->Pixel.IndexOffset ||
       ctx->Pixel.MapStencilFlag) {
      memcpy(stencilCopy, stencilVals, n * sizeof(GLubyte));
      _mesa_apply_stencil_transfer_ops(ctx, n, stencilCopy);
      stencilVals = stencilCopy;
   }

   switch (dstType) {
   case GL_UNSIGNED_INT_24_8:
      for (i = 0; i < n; i++) {
         GLuint z = (GLuint) (depthVals[i] * 0xffffff);
         dest[i] = (z << 8) | (stencilVals[i] & 0xff);
      }
      break;
   case GL_FLOAT_32_UNSIGNED_INT_24_8_REV:
      for (i = 0; i < n; i++) {
         ((GLfloat*)dest)[i*2] = depthVals[i];
         dest[i*2+1] = stencilVals[i] & 0xff;
      }
      break;
   }

   if (dstPacking->SwapBytes) {
      _mesa_swap4(dest, n);
   }

   free(depthCopy);
   free(stencilCopy);
}



/**
 * Unpack image data.  Apply byte swapping, byte flipping (bitmap).
 * Return all image data in a contiguous block.  This is used when we
 * compile glDrawPixels, glTexImage, etc into a display list.  We
 * need a copy of the data in a standard format.
 */
void *
_mesa_unpack_image( GLuint dimensions,
                    GLsizei width, GLsizei height, GLsizei depth,
                    GLenum format, GLenum type, const GLvoid *pixels,
                    const struct gl_pixelstore_attrib *unpack )
{
   GLint bytesPerRow, compsPerRow;
   GLboolean flipBytes, swap2, swap4;

   if (!pixels)
      return NULL;  /* not necessarily an error */

   if (width <= 0 || height <= 0 || depth <= 0)
      return NULL;  /* generate error later */

   if (type == GL_BITMAP) {
      bytesPerRow = (width + 7) >> 3;
      flipBytes = unpack->LsbFirst;
      swap2 = swap4 = GL_FALSE;
      compsPerRow = 0;
   }
   else {
      const GLint bytesPerPixel = _mesa_bytes_per_pixel(format, type);
      GLint components = _mesa_components_in_format(format);
      GLint bytesPerComp;

      if (_mesa_type_is_packed(type))
          components = 1;

      if (bytesPerPixel <= 0 || components <= 0)
         return NULL;   /* bad format or type.  generate error later */
      bytesPerRow = bytesPerPixel * width;
      bytesPerComp = bytesPerPixel / components;
      flipBytes = GL_FALSE;
      swap2 = (bytesPerComp == 2) && unpack->SwapBytes;
      swap4 = (bytesPerComp == 4) && unpack->SwapBytes;
      compsPerRow = components * width;
      assert(compsPerRow >= width);
   }

   {
      GLubyte *destBuffer
         = malloc(bytesPerRow * height * depth);
      GLubyte *dst;
      GLint img, row;
      if (!destBuffer)
         return NULL;   /* generate GL_OUT_OF_MEMORY later */

      dst = destBuffer;
      for (img = 0; img < depth; img++) {
         for (row = 0; row < height; row++) {
            const GLvoid *src = _mesa_image_address(dimensions, unpack, pixels,
                               width, height, format, type, img, row, 0);

            if ((type == GL_BITMAP) && (unpack->SkipPixels & 0x7)) {
               GLint i;
               flipBytes = GL_FALSE;
               if (unpack->LsbFirst) {
                  GLubyte srcMask = 1 << (unpack->SkipPixels & 0x7);
                  GLubyte dstMask = 128;
                  const GLubyte *s = src;
                  GLubyte *d = dst;
                  *d = 0;
                  for (i = 0; i < width; i++) {
                     if (*s & srcMask) {
                        *d |= dstMask;
                     }
                     if (srcMask == 128) {
                        srcMask = 1;
                        s++;
                     }
                     else {
                        srcMask = srcMask << 1;
                     }
                     if (dstMask == 1) {
                        dstMask = 128;
                        d++;
                        *d = 0;
                     }
                     else {
                        dstMask = dstMask >> 1;
                     }
                  }
               }
               else {
                  GLubyte srcMask = 128 >> (unpack->SkipPixels & 0x7);
                  GLubyte dstMask = 128;
                  const GLubyte *s = src;
                  GLubyte *d = dst;
                  *d = 0;
                  for (i = 0; i < width; i++) {
                     if (*s & srcMask) {
                        *d |= dstMask;
                     }
                     if (srcMask == 1) {
                        srcMask = 128;
                        s++;
                     }
                     else {
                        srcMask = srcMask >> 1;
                     }
                     if (dstMask == 1) {
                        dstMask = 128;
                        d++;
                        *d = 0;
                     }
                     else {
                        dstMask = dstMask >> 1;
                     }
                  }
               }
            }
            else {
               memcpy(dst, src, bytesPerRow);
            }

            /* byte flipping/swapping */
            if (flipBytes) {
               flip_bytes((GLubyte *) dst, bytesPerRow);
            }
            else if (swap2) {
               _mesa_swap2((GLushort*) dst, compsPerRow);
            }
            else if (swap4) {
               _mesa_swap4((GLuint*) dst, compsPerRow);
            }
            dst += bytesPerRow;
         }
      }
      return destBuffer;
   }
}

void
_mesa_pack_luminance_from_rgba_float(GLuint n, GLfloat rgba[][4],
                                     GLvoid *dstAddr, GLenum dst_format,
                                     GLbitfield transferOps)
{
   int i;
   GLfloat *dst = (GLfloat *) dstAddr;

   switch (dst_format) {
   case GL_LUMINANCE:
      if (transferOps & IMAGE_CLAMP_BIT) {
         for (i = 0; i < n; i++) {
            GLfloat sum = rgba[i][RCOMP] + rgba[i][GCOMP] + rgba[i][BCOMP];
            dst[i] = CLAMP(sum, 0.0F, 1.0F);
         }
      } else {
         for (i = 0; i < n; i++) {
            dst[i] = rgba[i][RCOMP] + rgba[i][GCOMP] + rgba[i][BCOMP];
         }
      }
      return;
   case GL_LUMINANCE_ALPHA:
      if (transferOps & IMAGE_CLAMP_BIT) {
         for (i = 0; i < n; i++) {
            GLfloat sum = rgba[i][RCOMP] + rgba[i][GCOMP] + rgba[i][BCOMP];
            dst[2*i] = CLAMP(sum, 0.0F, 1.0F);
            dst[2*i+1] = rgba[i][ACOMP];
         }
      } else {
         for (i = 0; i < n; i++) {
            dst[2*i] = rgba[i][RCOMP] + rgba[i][GCOMP] + rgba[i][BCOMP];
            dst[2*i+1] = rgba[i][ACOMP];
         }
      }
      return;
   default:
      assert(!"Unsupported format");
   }
}

static int32_t
clamp_sint64_to_sint32(int64_t src)
{
   return CLAMP(src, INT32_MIN, INT32_MAX);
}

static int32_t
clamp_sint64_to_uint32(int64_t src)
{
   return CLAMP(src, 0, UINT32_MAX);
}

static int32_t
clamp_uint64_to_uint32(uint64_t src)
{
   return MIN2(src, UINT32_MAX);
}

static int32_t
clamp_uint64_to_sint32(uint64_t src)
{
   return MIN2(src, INT32_MAX);
}

static int32_t
convert_integer_luminance64(int64_t src64, int bits,
                            bool dst_is_signed, bool src_is_signed)
{
   int32_t src32;

   /* Clamp Luminance value from 64-bit to 32-bit. Consider if we need
    * any signed<->unsigned conversion too.
    */
   if (src_is_signed && dst_is_signed)
      src32 = clamp_sint64_to_sint32(src64);
   else if (src_is_signed && !dst_is_signed)
      src32 = clamp_sint64_to_uint32(src64);
   else if (!src_is_signed && dst_is_signed)
      src32 = clamp_uint64_to_sint32(src64);
   else
      src32 = clamp_uint64_to_uint32(src64);

   /* If the dst type is < 32-bit, we need an extra clamp */
   if (bits == 32) {
      return src32;
   } else {
      if (dst_is_signed)
         return _mesa_signed_to_signed(src32, bits);
      else
         return _mesa_unsigned_to_unsigned(src32, bits);
   }
}

static int32_t
convert_integer(int32_t src, int bits, bool dst_is_signed, bool src_is_signed)
{
   if (src_is_signed && dst_is_signed)
      return _mesa_signed_to_signed(src, bits);
   else if (src_is_signed && !dst_is_signed)
      return _mesa_signed_to_unsigned(src, bits);
   else if (!src_is_signed && dst_is_signed)
      return _mesa_unsigned_to_signed(src, bits);
   else
      return _mesa_unsigned_to_unsigned(src, bits);
}

void
_mesa_pack_luminance_from_rgba_integer(GLuint n,
                                       GLuint rgba[][4], bool rgba_is_signed,
                                       GLvoid *dstAddr,
                                       GLenum dst_format,
                                       GLenum dst_type)
{
   int i;
   int64_t lum64;
   int32_t lum32, alpha;
   bool dst_is_signed;
   int dst_bits;

   assert(dst_format == GL_LUMINANCE_INTEGER_EXT ||
          dst_format == GL_LUMINANCE_ALPHA_INTEGER_EXT);

   /* We first compute luminance values as a 64-bit addition of the
    * 32-bit R,G,B components, then we clamp the result to the dst type size.
    *
    * Notice that this operation involves casting the 32-bit R,G,B components
    * to 64-bit before the addition. Since rgba is defined as a GLuint array
    * we need to be careful when rgba packs signed data and make sure
    * that we cast to a 32-bit signed integer values before casting them to
    * 64-bit signed integers.
    */
   dst_is_signed = (dst_type == GL_BYTE || dst_type == GL_SHORT ||
                    dst_type == GL_INT);

   dst_bits = _mesa_sizeof_type(dst_type) * 8;
   assert(dst_bits > 0);

   switch (dst_format) {
   case GL_LUMINANCE_INTEGER_EXT:
      for (i = 0; i < n; i++) {
         if (!rgba_is_signed) {
            lum64 = (uint64_t) rgba[i][RCOMP] +
                    (uint64_t) rgba[i][GCOMP] +
                    (uint64_t) rgba[i][BCOMP];
         } else {
            lum64 = (int64_t) ((int32_t) rgba[i][RCOMP]) +
                    (int64_t) ((int32_t) rgba[i][GCOMP]) +
                    (int64_t) ((int32_t) rgba[i][BCOMP]);
         }
         lum32 = convert_integer_luminance64(lum64, dst_bits,
                                             dst_is_signed, rgba_is_signed);
         switch (dst_type) {
         case GL_BYTE:
         case GL_UNSIGNED_BYTE: {
            GLbyte *dst = (GLbyte *) dstAddr;
            dst[i] = lum32;
            break;
         }
         case GL_SHORT:
         case GL_UNSIGNED_SHORT: {
            GLshort *dst = (GLshort *) dstAddr;
            dst[i] = lum32;
            break;
         }
         case GL_INT:
         case GL_UNSIGNED_INT: {
            GLint *dst = (GLint *) dstAddr;
            dst[i] = lum32;
            break;
         }
         }
      }
      return;
   case GL_LUMINANCE_ALPHA_INTEGER_EXT:
      for (i = 0; i < n; i++) {
         if (!rgba_is_signed) {
            lum64 = (uint64_t) rgba[i][RCOMP] +
                    (uint64_t) rgba[i][GCOMP] +
                    (uint64_t) rgba[i][BCOMP];
         } else {
            lum64 = (int64_t) ((int32_t) rgba[i][RCOMP]) +
                    (int64_t) ((int32_t) rgba[i][GCOMP]) +
                    (int64_t) ((int32_t) rgba[i][BCOMP]);
         }
         lum32 = convert_integer_luminance64(lum64, dst_bits,
                                             dst_is_signed, rgba_is_signed);
         alpha = convert_integer(rgba[i][ACOMP], dst_bits,
                                 dst_is_signed, rgba_is_signed);
         switch (dst_type) {
         case GL_BYTE:
         case GL_UNSIGNED_BYTE: {
            GLbyte *dst = (GLbyte *) dstAddr;
            dst[2*i] = lum32;
            dst[2*i+1] = alpha;
            break;
         }
         case GL_SHORT:
         case GL_UNSIGNED_SHORT: {
            GLshort *dst = (GLshort *) dstAddr;
            dst[i] = lum32;
            dst[2*i+1] = alpha;
            break;
         }
         case GL_INT:
         case GL_UNSIGNED_INT: {
            GLint *dst = (GLint *) dstAddr;
            dst[i] = lum32;
            dst[2*i+1] = alpha;
            break;
         }
         }
      }
      return;
   }
}

GLfloat *
_mesa_unpack_color_index_to_rgba_float(struct gl_context *ctx, GLuint dims,
                                       const void *src, GLenum srcFormat, GLenum srcType,
                                       int srcWidth, int srcHeight, int srcDepth,
                                       const struct gl_pixelstore_attrib *srcPacking,
                                       GLbitfield transferOps)
{
   int count, img;
   GLuint *indexes;
   GLfloat *rgba, *dstPtr;

   count = srcWidth * srcHeight;
   indexes = malloc(count * sizeof(GLuint));
   if (!indexes) {
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "pixel unpacking");
      return NULL;
   }

   rgba = malloc(4 * count * srcDepth * sizeof(GLfloat));
   if (!rgba) {
      free(indexes);
      _mesa_error(ctx, GL_OUT_OF_MEMORY, "pixel unpacking");
      return NULL;
   }

   /* Convert indexes to RGBA float */
   dstPtr = rgba;
   for (img = 0; img < srcDepth; img++) {
      const GLubyte *srcPtr =
         (const GLubyte *) _mesa_image_address(dims, srcPacking, src,
                                               srcWidth, srcHeight,
                                               srcFormat, srcType,
                                               img, 0, 0);

      extract_uint_indexes(count, indexes, srcFormat, srcType, srcPtr, srcPacking);

      if (transferOps & IMAGE_SHIFT_OFFSET_BIT)
         _mesa_shift_and_offset_ci(ctx, count, indexes);

      _mesa_map_ci_to_rgba(ctx, count, indexes, (float (*)[4])dstPtr);

      /* Don't do RGBA scale/bias or RGBA->RGBA mapping if starting
       * with color indexes.
       */
      transferOps &= ~(IMAGE_SCALE_BIAS_BIT | IMAGE_MAP_COLOR_BIT);
      _mesa_apply_rgba_transfer_ops(ctx, transferOps, count, (float (*)[4])dstPtr);

      dstPtr += srcHeight * srcWidth * 4;
   }

   free(indexes);

   return rgba;
}

GLubyte *
_mesa_unpack_color_index_to_rgba_ubyte(struct gl_context *ctx, GLuint dims,
                                       const void *src, GLenum srcFormat, GLenum srcType,
                                       int srcWidth, int srcHeight, int srcDepth,
                                       const struct gl_pixelstore_attrib *srcPacking,
                                       GLbitfield transferOps)
{
   GLfloat *rgba;
   GLubyte *dst;
   int count, i;

   transferOps |= IMAGE_CLAMP_BIT;
   rgba = _mesa_unpack_color_index_to_rgba_float(ctx, dims,
                                                 src, srcFormat, srcType,
                                                 srcWidth, srcHeight, srcDepth,
                                                 srcPacking, transferOps);

   count = srcWidth * srcHeight * srcDepth;
   dst = malloc(count * 4 * sizeof(GLubyte));
   for (i = 0; i < count; i++) {
      CLAMPED_FLOAT_TO_UBYTE(dst[i * 4 + 0], rgba[i * 4 + 0]);
      CLAMPED_FLOAT_TO_UBYTE(dst[i * 4 + 1], rgba[i * 4 + 1]);
      CLAMPED_FLOAT_TO_UBYTE(dst[i * 4 + 2], rgba[i * 4 + 2]);
      CLAMPED_FLOAT_TO_UBYTE(dst[i * 4 + 3], rgba[i * 4 + 3]);
   }

   free(rgba);

   return dst;
}

void
_mesa_unpack_ubyte_rgba_row(mesa_format format, uint32_t n,
                            const void *src, uint8_t dst[][4])
{
   const struct util_format_unpack_description *unpack =
      util_format_unpack_description((enum pipe_format)format);

   if (unpack->unpack_rgba_8unorm) {
      unpack->unpack_rgba_8unorm((uint8_t *)dst, src, n);
   } else {
      /* get float values, convert to ubyte */
      {
         float *tmp = malloc(n * 4 * sizeof(float));
         if (tmp) {
            uint32_t i;
            _mesa_unpack_rgba_row(format, n, src, (float (*)[4]) tmp);
            for (i = 0; i < n; i++) {
               dst[i][0] = _mesa_float_to_unorm(tmp[i*4+0], 8);
               dst[i][1] = _mesa_float_to_unorm(tmp[i*4+1], 8);
               dst[i][2] = _mesa_float_to_unorm(tmp[i*4+2], 8);
               dst[i][3] = _mesa_float_to_unorm(tmp[i*4+3], 8);
            }
            free(tmp);
         }
      }
   }
}

/** Helper struct for MESA_FORMAT_Z32_FLOAT_S8X24_UINT */
struct z32f_x24s8
{
   float z;
   uint32_t x24s8;
};


static void
unpack_uint_24_8_depth_stencil_Z24_UNORM_S8_UINT(const uint32_t *src, uint32_t *dst, uint32_t n)
{
   uint32_t i;

   for (i = 0; i < n; i++) {
      uint32_t val = src[i];
      dst[i] = val >> 24 | val << 8;
   }
}

static void
unpack_uint_24_8_depth_stencil_Z32_S8X24(const uint32_t *src,
                                         uint32_t *dst, uint32_t n)
{
   uint32_t i;

   for (i = 0; i < n; i++) {
      /* 8 bytes per pixel (float + uint32) */
      float zf = ((float *) src)[i * 2 + 0];
      uint32_t z24 = (uint32_t) (zf * (float) 0xffffff);
      uint32_t s = src[i * 2 + 1] & 0xff;
      dst[i] = (z24 << 8) | s;
   }
}

static void
unpack_uint_24_8_depth_stencil_S8_UINT_Z24_UNORM(const uint32_t *src, uint32_t *dst, uint32_t n)
{
   memcpy(dst, src, n * 4);
}

/**
 * Unpack depth/stencil returning as GL_UNSIGNED_INT_24_8.
 * \param format  the source data format
 */
void
_mesa_unpack_uint_24_8_depth_stencil_row(mesa_format format, uint32_t n,
                                         const void *src, uint32_t *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      unpack_uint_24_8_depth_stencil_S8_UINT_Z24_UNORM(src, dst, n);
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      unpack_uint_24_8_depth_stencil_Z24_UNORM_S8_UINT(src, dst, n);
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      unpack_uint_24_8_depth_stencil_Z32_S8X24(src, dst, n);
      break;
   default:
      unreachable("bad format %s in _mesa_unpack_uint_24_8_depth_stencil_row");
   }
}

static void
unpack_float_32_uint_24_8_Z24_UNORM_S8_UINT(const uint32_t *src,
                                            uint32_t *dst, uint32_t n)
{
   uint32_t i;
   struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
   const double scale = 1.0 / (double) 0xffffff;

   for (i = 0; i < n; i++) {
      const uint32_t z24 = src[i] & 0xffffff;
      d[i].z = z24 * scale;
      d[i].x24s8 = src[i] >> 24;
      assert(d[i].z >= 0.0f);
      assert(d[i].z <= 1.0f);
   }
}

static void
unpack_float_32_uint_24_8_Z32_FLOAT_S8X24_UINT(const uint32_t *src,
                                               uint32_t *dst, uint32_t n)
{
   memcpy(dst, src, n * sizeof(struct z32f_x24s8));
}

static void
unpack_float_32_uint_24_8_S8_UINT_Z24_UNORM(const uint32_t *src,
                                            uint32_t *dst, uint32_t n)
{
   uint32_t i;
   struct z32f_x24s8 *d = (struct z32f_x24s8 *) dst;
   const double scale = 1.0 / (double) 0xffffff;

   for (i = 0; i < n; i++) {
      const uint32_t z24 = src[i] >> 8;
      d[i].z = z24 * scale;
      d[i].x24s8 = src[i] & 0xff;
      assert(d[i].z >= 0.0f);
      assert(d[i].z <= 1.0f);
   }
}

/**
 * Unpack depth/stencil returning as GL_FLOAT_32_UNSIGNED_INT_24_8_REV.
 * \param format  the source data format
 *
 * In GL_FLOAT_32_UNSIGNED_INT_24_8_REV lower 4 bytes contain float
 * component and higher 4 bytes contain packed 24-bit and 8-bit
 * components.
 *
 *    31 30 29 28 ... 4 3 2 1 0    31 30 29 ... 9 8 7 6 5 ... 2 1 0
 *    +-------------------------+  +--------------------------------+
 *    |    Float Component      |  | Unused         | 8 bit stencil |
 *    +-------------------------+  +--------------------------------+
 *          lower 4 bytes                  higher 4 bytes
 */
void
_mesa_unpack_float_32_uint_24_8_depth_stencil_row(mesa_format format, uint32_t n,
                                                  const void *src, uint32_t *dst)
{
   switch (format) {
   case MESA_FORMAT_S8_UINT_Z24_UNORM:
      unpack_float_32_uint_24_8_S8_UINT_Z24_UNORM(src, dst, n);
      break;
   case MESA_FORMAT_Z24_UNORM_S8_UINT:
      unpack_float_32_uint_24_8_Z24_UNORM_S8_UINT(src, dst, n);
      break;
   case MESA_FORMAT_Z32_FLOAT_S8X24_UINT:
      unpack_float_32_uint_24_8_Z32_FLOAT_S8X24_UINT(src, dst, n);
      break;
   default:
      unreachable("bad format %s in _mesa_unpack_uint_24_8_depth_stencil_row");
   }
}
