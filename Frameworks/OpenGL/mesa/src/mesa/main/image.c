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
 * \file image.c
 * Image handling.
 */


#include "util/glheader.h"
#include "colormac.h"
#include "glformats.h"
#include "image.h"

#include "macros.h"
#include "mtypes.h"



/**
 * Flip the order of the 2 bytes in each word in the given array (src) and
 * store the result in another array (dst). For in-place byte-swapping this
 * function can be called with the same array for src and dst.
 *
 * \param dst the array where byte-swapped data will be stored.
 * \param src the array with the source data we want to byte-swap.
 * \param n number of words.
 */
static void
swap2_copy( GLushort *dst, GLushort *src, GLuint n )
{
   GLuint i;
   for (i = 0; i < n; i++) {
      dst[i] = (src[i] >> 8) | ((src[i] << 8) & 0xff00);
   }
}

void
_mesa_swap2(GLushort *p, GLuint n)
{
   swap2_copy(p, p, n);
}

/*
 * Flip the order of the 4 bytes in each word in the given array (src) and
 * store the result in another array (dst). For in-place byte-swapping this
 * function can be called with the same array for src and dst.
 *
 * \param dst the array where byte-swapped data will be stored.
 * \param src the array with the source data we want to byte-swap.
 * \param n number of words.
 */
static void
swap4_copy( GLuint *dst, GLuint *src, GLuint n )
{
   GLuint i, a, b;
   for (i = 0; i < n; i++) {
      b = src[i];
      a =  (b >> 24)
	| ((b >> 8) & 0xff00)
	| ((b << 8) & 0xff0000)
	| ((b << 24) & 0xff000000);
      dst[i] = a;
   }
}

void
_mesa_swap4(GLuint *p, GLuint n)
{
   swap4_copy(p, p, n);
}

/**
 * Return the byte offset of a specific pixel in an image (1D, 2D or 3D).
 *
 * Pixel unpacking/packing parameters are observed according to \p packing.
 *
 * \param dimensions either 1, 2 or 3 to indicate dimensionality of image
 * \param packing  the pixelstore attributes
 * \param width  the image width
 * \param height  the image height
 * \param format  the pixel format (must be validated beforehand)
 * \param type  the pixel data type (must be validated beforehand)
 * \param img  which image in the volume (0 for 1D or 2D images)
 * \param row  row of pixel in the image (0 for 1D images)
 * \param column column of pixel in the image
 *
 * \return offset of pixel.
 *
 * \sa gl_pixelstore_attrib.
 */
GLintptr
_mesa_image_offset( GLuint dimensions,
                    const struct gl_pixelstore_attrib *packing,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    GLint img, GLint row, GLint column )
{
   GLint alignment;        /* 1, 2 or 4 */
   GLint pixels_per_row;
   GLint rows_per_image;
   GLint skiprows;
   GLint skippixels;
   GLint skipimages;       /* for 3-D volume images */
   GLintptr offset;

   assert(dimensions >= 1 && dimensions <= 3);

   alignment = packing->Alignment;
   if (packing->RowLength > 0) {
      pixels_per_row = packing->RowLength;
   }
   else {
      pixels_per_row = width;
   }
   if (packing->ImageHeight > 0) {
      rows_per_image = packing->ImageHeight;
   }
   else {
      rows_per_image = height;
   }

   skippixels = packing->SkipPixels;
   /* Note: SKIP_ROWS _is_ used for 1D images */
   skiprows = packing->SkipRows;
   /* Note: SKIP_IMAGES is only used for 3D images */
   skipimages = (dimensions == 3) ? packing->SkipImages : 0;

   if (type == GL_BITMAP) {
      /* BITMAP data */
      GLintptr bytes_per_row;
      GLintptr bytes_per_image;
      /* components per pixel for color or stencil index: */
      const GLint comp_per_pixel = 1;

      /* The pixel type and format should have been error checked earlier */
      assert(format == GL_COLOR_INDEX || format == GL_STENCIL_INDEX);

      bytes_per_row = alignment
                    * DIV_ROUND_UP( comp_per_pixel*pixels_per_row, 8*alignment );

      bytes_per_image = bytes_per_row * rows_per_image;

      offset = (skipimages + img) * bytes_per_image
                 + (skiprows + row) * bytes_per_row
                 + (skippixels + column) / 8;
   }
   else {
      /* Non-BITMAP data */
      GLintptr bytes_per_pixel, bytes_per_row, remainder, bytes_per_image;
      GLintptr topOfImage;

      bytes_per_pixel = _mesa_bytes_per_pixel( format, type );

      /* The pixel type and format should have been error checked earlier */
      assert(bytes_per_pixel > 0);

      bytes_per_row = pixels_per_row * bytes_per_pixel;
      remainder = bytes_per_row % alignment;
      if (remainder > 0)
         bytes_per_row += (alignment - remainder);

      assert(bytes_per_row % alignment == 0);

      bytes_per_image = bytes_per_row * rows_per_image;

      if (packing->Invert) {
         /* set pixel_addr to the last row */
         topOfImage = bytes_per_row * (height - 1);
         bytes_per_row = -bytes_per_row;
      }
      else {
         topOfImage = 0;
      }

      /* compute final pixel address */
      offset = (skipimages + img) * bytes_per_image
                 + topOfImage
                 + (skiprows + row) * bytes_per_row
                 + (skippixels + column) * bytes_per_pixel;
   }

   return offset;
}


/**
 * Return the address of a specific pixel in an image (1D, 2D or 3D).
 *
 * Pixel unpacking/packing parameters are observed according to \p packing.
 *
 * \param dimensions either 1, 2 or 3 to indicate dimensionality of image
 * \param packing  the pixelstore attributes
 * \param image  starting address of image data
 * \param width  the image width
 * \param height  the image height
 * \param format  the pixel format (must be validated beforehand)
 * \param type  the pixel data type (must be validated beforehand)
 * \param img  which image in the volume (0 for 1D or 2D images)
 * \param row  row of pixel in the image (0 for 1D images)
 * \param column column of pixel in the image
 *
 * \return address of pixel.
 *
 * \sa gl_pixelstore_attrib.
 */
GLvoid *
_mesa_image_address( GLuint dimensions,
                     const struct gl_pixelstore_attrib *packing,
                     const GLvoid *image,
                     GLsizei width, GLsizei height,
                     GLenum format, GLenum type,
                     GLint img, GLint row, GLint column )
{
   const GLubyte *addr = (const GLubyte *) image;

   addr += _mesa_image_offset(dimensions, packing, width, height,
                              format, type, img, row, column);

   return (GLvoid *) addr;
}


GLvoid *
_mesa_image_address1d( const struct gl_pixelstore_attrib *packing,
                       const GLvoid *image,
                       GLsizei width,
                       GLenum format, GLenum type,
                       GLint column )
{
   return _mesa_image_address(1, packing, image, width, 1,
                              format, type, 0, 0, column);
}


GLvoid *
_mesa_image_address2d( const struct gl_pixelstore_attrib *packing,
                       const GLvoid *image,
                       GLsizei width, GLsizei height,
                       GLenum format, GLenum type,
                       GLint row, GLint column )
{
   return _mesa_image_address(2, packing, image, width, height,
                              format, type, 0, row, column);
}


GLvoid *
_mesa_image_address3d( const struct gl_pixelstore_attrib *packing,
                       const GLvoid *image,
                       GLsizei width, GLsizei height,
                       GLenum format, GLenum type,
                       GLint img, GLint row, GLint column )
{
   return _mesa_image_address(3, packing, image, width, height,
                              format, type, img, row, column);
}



/**
 * Compute the stride (in bytes) between image rows.
 *
 * \param packing the pixelstore attributes
 * \param width image width.
 * \param format pixel format.
 * \param type pixel data type.
 *
 * \return the stride in bytes for the given parameters, or -1 if error
 */
GLint
_mesa_image_row_stride( const struct gl_pixelstore_attrib *packing,
                        GLint width, GLenum format, GLenum type )
{
   GLint bytesPerRow, remainder;

   assert(packing);

   if (type == GL_BITMAP) {
      if (packing->RowLength == 0) {
         bytesPerRow = (width + 7) / 8;
      }
      else {
         bytesPerRow = (packing->RowLength + 7) / 8;
      }
   }
   else {
      /* Non-BITMAP data */
      const GLint bytesPerPixel = _mesa_bytes_per_pixel(format, type);
      if (bytesPerPixel <= 0)
         return -1;  /* error */
      if (packing->RowLength == 0) {
         bytesPerRow = bytesPerPixel * width;
      }
      else {
         bytesPerRow = bytesPerPixel * packing->RowLength;
      }
   }

   remainder = bytesPerRow % packing->Alignment;
   if (remainder > 0) {
      bytesPerRow += (packing->Alignment - remainder);
   }

   if (packing->Invert) {
      /* negate the bytes per row (negative row stride) */
      bytesPerRow = -bytesPerRow;
   }

   return bytesPerRow;
}


/*
 * Compute the stride between images in a 3D texture (in bytes) for the given
 * pixel packing parameters and image width, format and type.
 */
intptr_t
_mesa_image_image_stride( const struct gl_pixelstore_attrib *packing,
                          GLint width, GLint height,
                          GLenum format, GLenum type )
{
   GLint bytesPerRow, remainder;
   intptr_t bytesPerImage;

   assert(packing);

   if (type == GL_BITMAP) {
      if (packing->RowLength == 0) {
         bytesPerRow = (width + 7) / 8;
      }
      else {
         bytesPerRow = (packing->RowLength + 7) / 8;
      }
   }
   else {
      const GLint bytesPerPixel = _mesa_bytes_per_pixel(format, type);

      if (bytesPerPixel <= 0)
         return -1;  /* error */
      if (packing->RowLength == 0) {
         bytesPerRow = bytesPerPixel * width;
      }
      else {
         bytesPerRow = bytesPerPixel * packing->RowLength;
      }
   }

   remainder = bytesPerRow % packing->Alignment;
   if (remainder > 0)
      bytesPerRow += (packing->Alignment - remainder);

   if (packing->ImageHeight == 0)
      bytesPerImage = (intptr_t)bytesPerRow * height;
   else
      bytesPerImage = (intptr_t)bytesPerRow * packing->ImageHeight;

   return bytesPerImage;
}



/**
 * "Expand" a bitmap from 1-bit per pixel to 8-bits per pixel.
 * This is typically used to convert a bitmap into a GLubyte/pixel texture.
 * "On" bits will set texels to \p onValue.
 * "Off" bits will not modify texels.
 * \param width  src bitmap width in pixels
 * \param height  src bitmap height in pixels
 * \param unpack  bitmap unpacking state
 * \param bitmap  the src bitmap data
 * \param destBuffer  start of dest buffer
 * \param destStride  row stride in dest buffer
 * \param onValue  if bit is 1, set destBuffer pixel to this value
 */
void
_mesa_expand_bitmap(GLsizei width, GLsizei height,
                    const struct gl_pixelstore_attrib *unpack,
                    const GLubyte *bitmap,
                    GLubyte *destBuffer, GLint destStride,
                    GLubyte onValue)
{
   const GLubyte *srcRow = (const GLubyte *)
      _mesa_image_address2d(unpack, bitmap, width, height,
                            GL_COLOR_INDEX, GL_BITMAP, 0, 0);
   const GLint srcStride = _mesa_image_row_stride(unpack, width,
                                                  GL_COLOR_INDEX, GL_BITMAP);
   GLint row, col;
   GLubyte *dstRow = destBuffer;

   for (row = 0; row < height; row++) {
      const GLubyte *src = srcRow;

      if (unpack->LsbFirst) {
         /* Lsb first */
         GLubyte mask = 1U << (unpack->SkipPixels & 0x7);
         for (col = 0; col < width; col++) {

            if (*src & mask) {
               dstRow[col] = onValue;
            }

            if (mask == 128U) {
               src++;
               mask = 1U;
            }
            else {
               mask = mask << 1;
            }
         }

         /* get ready for next row */
         if (mask != 1)
            src++;
      }
      else {
         /* Msb first */
         GLubyte mask = 128U >> (unpack->SkipPixels & 0x7);
         for (col = 0; col < width; col++) {

            if (*src & mask) {
               dstRow[col] = onValue;
            }

            if (mask == 1U) {
               src++;
               mask = 128U;
            }
            else {
               mask = mask >> 1;
            }
         }

         /* get ready for next row */
         if (mask != 128)
            src++;
      }

      srcRow += srcStride;
      dstRow += destStride;
   } /* row */
}




/**
 * Perform basic clipping for glDrawPixels.  The image's position and size
 * and the unpack SkipPixels and SkipRows are adjusted so that the image
 * region is entirely within the window and scissor bounds.
 * NOTE: this will only work when glPixelZoom is (1, 1) or (1, -1).
 * If Pixel.ZoomY is -1, *destY will be changed to be the first row which
 * we'll actually write.  Beforehand, *destY-1 is the first drawing row.
 *
 * \return  GL_TRUE if image is ready for drawing or
 *          GL_FALSE if image was completely clipped away (draw nothing)
 */
GLboolean
_mesa_clip_drawpixels(const struct gl_context *ctx,
                      GLint *destX, GLint *destY,
                      GLsizei *width, GLsizei *height,
                      struct gl_pixelstore_attrib *unpack)
{
   const struct gl_framebuffer *buffer = ctx->DrawBuffer;

   if (unpack->RowLength == 0) {
      unpack->RowLength = *width;
   }

   assert(ctx->Pixel.ZoomX == 1.0F);
   assert(ctx->Pixel.ZoomY == 1.0F || ctx->Pixel.ZoomY == -1.0F);

   /* left clipping */
   if (*destX < buffer->_Xmin) {
      unpack->SkipPixels += (buffer->_Xmin - *destX);
      *width -= (buffer->_Xmin - *destX);
      *destX = buffer->_Xmin;
   }
   /* right clipping */
   if (*destX + *width > buffer->_Xmax)
      *width -= (*destX + *width - buffer->_Xmax);

   if (*width <= 0)
      return GL_FALSE;

   if (ctx->Pixel.ZoomY == 1.0F) {
      /* bottom clipping */
      if (*destY < buffer->_Ymin) {
         unpack->SkipRows += (buffer->_Ymin - *destY);
         *height -= (buffer->_Ymin - *destY);
         *destY = buffer->_Ymin;
      }
      /* top clipping */
      if (*destY + *height > buffer->_Ymax)
         *height -= (*destY + *height - buffer->_Ymax);
   }
   else { /* upside down */
      /* top clipping */
      if (*destY > buffer->_Ymax) {
         unpack->SkipRows += (*destY - buffer->_Ymax);
         *height -= (*destY - buffer->_Ymax);
         *destY = buffer->_Ymax;
      }
      /* bottom clipping */
      if (*destY - *height < buffer->_Ymin)
         *height -= (buffer->_Ymin - (*destY - *height));
      /* adjust destY so it's the first row to write to */
      (*destY)--;
   }

   if (*height <= 0)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Perform clipping for glReadPixels.  The image's window position
 * and size, and the pack skipPixels, skipRows and rowLength are adjusted
 * so that the image region is entirely within the window bounds.
 * Note: this is different from _mesa_clip_drawpixels() in that the
 * scissor box is ignored, and we use the bounds of the current readbuffer
 * surface or the attached image.
 *
 * \return  GL_TRUE if region to read is in bounds
 *          GL_FALSE if region is completely out of bounds (nothing to read)
 */
GLboolean
_mesa_clip_readpixels(const struct gl_context *ctx,
                      GLint *srcX, GLint *srcY,
                      GLsizei *width, GLsizei *height,
                      struct gl_pixelstore_attrib *pack)
{
   const struct gl_framebuffer *buffer = ctx->ReadBuffer;
   struct gl_renderbuffer *rb = buffer->_ColorReadBuffer;
   GLsizei clip_width;
   GLsizei clip_height;

   if (rb) {
      clip_width = rb->Width;
      clip_height = rb->Height;
   } else {
      clip_width = buffer->Width;
      clip_height = buffer->Height;
   }


   if (pack->RowLength == 0) {
      pack->RowLength = *width;
   }

   /* left clipping */
   if (*srcX < 0) {
      pack->SkipPixels += (0 - *srcX);
      *width -= (0 - *srcX);
      *srcX = 0;
   }
   /* right clipping */
   if (*srcX + *width > clip_width)
      *width -= (*srcX + *width - clip_width);

   if (*width <= 0)
      return GL_FALSE;

   /* bottom clipping */
   if (*srcY < 0) {
      pack->SkipRows += (0 - *srcY);
      *height -= (0 - *srcY);
      *srcY = 0;
   }
   /* top clipping */
   if (*srcY + *height > clip_height)
      *height -= (*srcY + *height - clip_height);

   if (*height <= 0)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Do clipping for a glCopyTexSubImage call.
 * The framebuffer source region might extend outside the framebuffer
 * bounds.  Clip the source region against the framebuffer bounds and
 * adjust the texture/dest position and size accordingly.
 *
 * \return GL_FALSE if region is totally clipped, GL_TRUE otherwise.
 */
GLboolean
_mesa_clip_copytexsubimage(const struct gl_context *ctx,
                           GLint *destX, GLint *destY,
                           GLint *srcX, GLint *srcY,
                           GLsizei *width, GLsizei *height)
{
   const struct gl_framebuffer *fb = ctx->ReadBuffer;
   const GLint srcX0 = *srcX, srcY0 = *srcY;

   if (_mesa_clip_to_region(0, 0, fb->Width, fb->Height,
                            srcX, srcY, width, height)) {
      *destX = *destX + *srcX - srcX0;
      *destY = *destY + *srcY - srcY0;

      return GL_TRUE;
   }
   else {
      return GL_FALSE;
   }
}



/**
 * Clip the rectangle defined by (x, y, width, height) against the bounds
 * specified by [xmin, xmax) and [ymin, ymax).
 * \return GL_FALSE if rect is totally clipped, GL_TRUE otherwise.
 */
GLboolean
_mesa_clip_to_region(GLint xmin, GLint ymin,
                     GLint xmax, GLint ymax,
                     GLint *x, GLint *y,
                     GLsizei *width, GLsizei *height )
{
   /* left clipping */
   if (*x < xmin) {
      *width -= (xmin - *x);
      *x = xmin;
   }

   /* right clipping */
   if (*x + *width > xmax)
      *width -= (*x + *width - xmax);

   if (*width <= 0)
      return GL_FALSE;

   /* bottom (or top) clipping */
   if (*y < ymin) {
      *height -= (ymin - *y);
      *y = ymin;
   }

   /* top (or bottom) clipping */
   if (*y + *height > ymax)
      *height -= (*y + *height - ymax);

   if (*height <= 0)
      return GL_FALSE;

   return GL_TRUE;
}


/**
 * Clip dst coords against Xmax (or Ymax).
 */
static inline void
clip_right_or_top(GLint *srcX0, GLint *srcX1,
                  GLint *dstX0, GLint *dstX1,
                  GLint maxValue)
{
   GLfloat t, bias;

   if (*dstX1 > maxValue) {
      /* X1 outside right edge */
      assert(*dstX0 < maxValue); /* X0 should be inside right edge */
      t = (GLfloat) (maxValue - *dstX0) / (GLfloat) (*dstX1 - *dstX0);
      /* chop off [t, 1] part */
      assert(t >= 0.0 && t <= 1.0);
      *dstX1 = maxValue;
      bias = (*srcX0 < *srcX1) ? 0.5F : -0.5F;
      *srcX1 = *srcX0 + (GLint) (t * (*srcX1 - *srcX0) + bias);
   }
   else if (*dstX0 > maxValue) {
      /* X0 outside right edge */
      assert(*dstX1 < maxValue); /* X1 should be inside right edge */
      t = (GLfloat) (maxValue - *dstX1) / (GLfloat) (*dstX0 - *dstX1);
      /* chop off [t, 1] part */
      assert(t >= 0.0 && t <= 1.0);
      *dstX0 = maxValue;
      bias = (*srcX0 < *srcX1) ? -0.5F : 0.5F;
      *srcX0 = *srcX1 + (GLint) (t * (*srcX0 - *srcX1) + bias);
   }
}


/**
 * Clip dst coords against Xmin (or Ymin).
 */
static inline void
clip_left_or_bottom(GLint *srcX0, GLint *srcX1,
                    GLint *dstX0, GLint *dstX1,
                    GLint minValue)
{
   GLfloat t, bias;

   if (*dstX0 < minValue) {
      /* X0 outside left edge */
      assert(*dstX1 > minValue); /* X1 should be inside left edge */
      t = (GLfloat) (minValue - *dstX0) / (GLfloat) (*dstX1 - *dstX0);
      /* chop off [0, t] part */
      assert(t >= 0.0 && t <= 1.0);
      *dstX0 = minValue;
      bias = (*srcX0 < *srcX1) ? 0.5F : -0.5F;
      *srcX0 = *srcX0 + (GLint) (t * (*srcX1 - *srcX0) + bias);
   }
   else if (*dstX1 < minValue) {
      /* X1 outside left edge */
      assert(*dstX0 > minValue); /* X0 should be inside left edge */
      t = (GLfloat) (minValue - *dstX1) / (GLfloat) (*dstX0 - *dstX1);
      /* chop off [0, t] part */
      assert(t >= 0.0 && t <= 1.0);
      *dstX1 = minValue;
      bias = (*srcX0 < *srcX1) ? -0.5F : 0.5F;
      *srcX1 = *srcX1 + (GLint) (t * (*srcX0 - *srcX1) + bias);
   }
}


/**
 * Do clipping of blit src/dest rectangles.
 * The dest rect is clipped against both the buffer bounds and scissor bounds.
 * The src rect is just clipped against the buffer bounds.
 *
 * When either the src or dest rect is clipped, the other is also clipped
 * proportionately!
 *
 * Note that X0 need not be less than X1 (same for Y) for either the source
 * and dest rects.  That makes the clipping a little trickier.
 *
 * \return GL_TRUE if anything is left to draw, GL_FALSE if totally clipped
 */
GLboolean
_mesa_clip_blit(struct gl_context *ctx,
                const struct gl_framebuffer *readFb,
                const struct gl_framebuffer *drawFb,
                GLint *srcX0, GLint *srcY0, GLint *srcX1, GLint *srcY1,
                GLint *dstX0, GLint *dstY0, GLint *dstX1, GLint *dstY1)
{
   const GLint srcXmin = 0;
   const GLint srcXmax = readFb->Width;
   const GLint srcYmin = 0;
   const GLint srcYmax = readFb->Height;

   /* these include scissor bounds */
   const GLint dstXmin = drawFb->_Xmin;
   const GLint dstXmax = drawFb->_Xmax;
   const GLint dstYmin = drawFb->_Ymin;
   const GLint dstYmax = drawFb->_Ymax;

   /*
   printf("PreClipX:  src: %d .. %d  dst: %d .. %d\n",
          *srcX0, *srcX1, *dstX0, *dstX1);
   printf("PreClipY:  src: %d .. %d  dst: %d .. %d\n",
          *srcY0, *srcY1, *dstY0, *dstY1);
   */

   /* trivial rejection tests */
   if (*dstX0 == *dstX1)
      return GL_FALSE; /* no width */
   if (*dstX0 <= dstXmin && *dstX1 <= dstXmin)
      return GL_FALSE; /* totally out (left) of bounds */
   if (*dstX0 >= dstXmax && *dstX1 >= dstXmax)
      return GL_FALSE; /* totally out (right) of bounds */

   if (*dstY0 == *dstY1)
      return GL_FALSE;
   if (*dstY0 <= dstYmin && *dstY1 <= dstYmin)
      return GL_FALSE;
   if (*dstY0 >= dstYmax && *dstY1 >= dstYmax)
      return GL_FALSE;

   if (*srcX0 == *srcX1)
      return GL_FALSE;
   if (*srcX0 <= srcXmin && *srcX1 <= srcXmin)
      return GL_FALSE;
   if (*srcX0 >= srcXmax && *srcX1 >= srcXmax)
      return GL_FALSE;

   if (*srcY0 == *srcY1)
      return GL_FALSE;
   if (*srcY0 <= srcYmin && *srcY1 <= srcYmin)
      return GL_FALSE;
   if (*srcY0 >= srcYmax && *srcY1 >= srcYmax)
      return GL_FALSE;

   /*
    * dest clip
    */
   clip_right_or_top(srcX0, srcX1, dstX0, dstX1, dstXmax);
   clip_right_or_top(srcY0, srcY1, dstY0, dstY1, dstYmax);
   clip_left_or_bottom(srcX0, srcX1, dstX0, dstX1, dstXmin);
   clip_left_or_bottom(srcY0, srcY1, dstY0, dstY1, dstYmin);

   /*
    * src clip (just swap src/dst values from above)
    */
   clip_right_or_top(dstX0, dstX1, srcX0, srcX1, srcXmax);
   clip_right_or_top(dstY0, dstY1, srcY0, srcY1, srcYmax);
   clip_left_or_bottom(dstX0, dstX1, srcX0, srcX1, srcXmin);
   clip_left_or_bottom(dstY0, dstY1, srcY0, srcY1, srcYmin);

   /*
   printf("PostClipX: src: %d .. %d  dst: %d .. %d\n",
          *srcX0, *srcX1, *dstX0, *dstX1);
   printf("PostClipY: src: %d .. %d  dst: %d .. %d\n",
          *srcY0, *srcY1, *dstY0, *dstY1);
   */

   assert(*dstX0 >= dstXmin);
   assert(*dstX0 <= dstXmax);
   assert(*dstX1 >= dstXmin);
   assert(*dstX1 <= dstXmax);

   assert(*dstY0 >= dstYmin);
   assert(*dstY0 <= dstYmax);
   assert(*dstY1 >= dstYmin);
   assert(*dstY1 <= dstYmax);

   assert(*srcX0 >= srcXmin);
   assert(*srcX0 <= srcXmax);
   assert(*srcX1 >= srcXmin);
   assert(*srcX1 <= srcXmax);

   assert(*srcY0 >= srcYmin);
   assert(*srcY0 <= srcYmax);
   assert(*srcY1 >= srcYmin);
   assert(*srcY1 <= srcYmax);

   return GL_TRUE;
}

/**
 * Swap the bytes in a 2D image.
 *
 * using the packing information this swaps the bytes
 * according to the format and type of data being input.
 * It takes into a/c various packing parameters like
 * Alignment and RowLength.
 */
void
_mesa_swap_bytes_2d_image(GLenum format, GLenum type,
                          const struct gl_pixelstore_attrib *packing,
                          GLsizei width, GLsizei height,
                          GLvoid *dst, const GLvoid *src)
{
   GLint swapSize = _mesa_sizeof_packed_type(type);

   assert(packing->SwapBytes);

   if (swapSize == 2 || swapSize == 4) {
      int swapsPerPixel = _mesa_bytes_per_pixel(format, type) / swapSize;
      int stride = _mesa_image_row_stride(packing, width, format, type);
      int row;
      uint8_t *dstrow;
      const uint8_t *srcrow;
      assert(swapsPerPixel > 0);
      assert(_mesa_bytes_per_pixel(format, type) % swapSize == 0);
      dstrow = dst;
      srcrow = src;
      for (row = 0; row < height; row++) {
         if (swapSize == 2)
            swap2_copy((GLushort *)dstrow, (GLushort *)srcrow, width * swapsPerPixel);
         else if (swapSize == 4)
            swap4_copy((GLuint *)dstrow, (GLuint *)srcrow, width * swapsPerPixel);
         dstrow += stride;
         srcrow += stride;
      }
   }
}
