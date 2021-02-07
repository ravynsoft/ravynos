/*
   NSBitmapImageRep+PNG.m

   Methods for loading .png images.

   Copyright (C) 2003-2013 Free Software Foundation, Inc.
   
   Written by: Alexander Malmberg <alexander@malmberg.org>
   Date: 2003-12-07
   
   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import "config.h"

/* we include PNG stuff only if required and before the resto to avoid header and
   redeclaration problems (setjmp, etc) */
#ifdef HAVE_LIBPNG

#include <png.h>

#if defined(PNG_FLOATING_POINT_SUPPORT)
#  define PNG_FLOATING_POINT 1
#else
#  define PNG_FLOATING_POINT 0
#endif
#if defined(PNG_gAMA_SUPPORT)
#  define PNG_gAMA 1
#else
#  define PNG_gAMA 0
#endif

#endif /* HAVE_LIBPNG */

/* we import all the standard headers to allow compilation without PNG */
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSGraphics.h"
#import "NSBitmapImageRepPrivate.h"
#import "NSBitmapImageRep+PNG.h"


#ifdef HAVE_LIBPNG

@implementation NSBitmapImageRep (PNG)

+ (BOOL) _bitmapIsPNG: (NSData *)imageData
{
  if (![imageData length])
    return NO;

  if (!png_sig_cmp((png_bytep)[imageData bytes], 0, [imageData length]))
    return YES;
  return NO;
}

typedef struct
{
  NSData *data;
  unsigned int offset;
} reader_struct_t;

static void reader_func(png_structp png_struct, png_bytep data,
			png_size_t length)
{
  reader_struct_t *r = png_get_io_ptr(png_struct);

  if (r->offset + length > [r->data length])
    {
      png_error(png_struct, "end of buffer");
      return;
    }
  memcpy(data, [r->data bytes] + r->offset, length);
  r->offset += length;
}

- (id) _initBitmapFromPNG: (NSData *)imageData
{
  png_structp png_struct;
  png_infop png_info, png_end_info;

  int width,height;
  unsigned char *buf = NULL;
  int bytes_per_row;
  int type,channels,depth;

  BOOL alpha;
  int bpp;
  NSString *colorspace;

  reader_struct_t reader;

  if (!(self = [super init]))
    return nil;

  png_struct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_struct)
    {
      RELEASE(self);
      return nil;
    }

  png_info = png_create_info_struct(png_struct);
  if (!png_info)
    {
      png_destroy_read_struct(&png_struct, NULL, NULL);
      RELEASE(self);
      return nil;
    }

  png_end_info = png_create_info_struct(png_struct);
  if (!png_end_info)
    {
      png_destroy_read_struct(&png_struct, &png_info, NULL);
      RELEASE(self);
      return nil;
    }

  if (setjmp(png_jmpbuf(png_struct)))
    {
      // We get here when an error happens during image loading
      png_destroy_read_struct(&png_struct, &png_info, &png_end_info);
      if (buf != NULL)
        {
          NSZoneFree([self zone], buf);
        }
      RELEASE(self);
      return nil;
    }

  reader.data = imageData;
  reader.offset = 0;
  png_set_read_fn(png_struct, &reader, reader_func);

  png_read_info(png_struct, png_info);

  width = png_get_image_width(png_struct, png_info);
  height = png_get_image_height(png_struct, png_info);
  bytes_per_row = png_get_rowbytes(png_struct, png_info);
  type = png_get_color_type(png_struct, png_info);
  channels = png_get_channels(png_struct, png_info);
  depth = png_get_bit_depth(png_struct, png_info);

  switch (type)
    {
      case PNG_COLOR_TYPE_GRAY:
	colorspace = NSCalibratedWhiteColorSpace;
	alpha = NO;
	NSAssert(channels == 1, @"unexpected channel/color_type combination");
	bpp = depth;
	break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
	colorspace = NSCalibratedWhiteColorSpace;
	alpha = YES;
	NSAssert(channels == 2, @"unexpected channel/color_type combination");
	bpp = depth * 2;
	break;

      case PNG_COLOR_TYPE_PALETTE:
	png_set_palette_to_rgb(png_struct);
	channels = 3;
	depth = 8;

	alpha = NO;
	if (png_get_valid(png_struct, png_info, PNG_INFO_tRNS))
	  {
	    alpha = YES;
	    channels++;
	    png_set_tRNS_to_alpha(png_struct);
	  }

	bpp = channels * 8;
	bytes_per_row = channels * width;
	colorspace = NSCalibratedRGBColorSpace;
	break;

      case PNG_COLOR_TYPE_RGB:
	colorspace = NSCalibratedRGBColorSpace;
	alpha = NO;
	bpp = channels * depth; /* channels might be 4 if there's a filler */
	channels = 3;
	break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
	colorspace = NSCalibratedRGBColorSpace;
	alpha = YES;
	NSAssert(channels == 4, @"unexpected channel/color_type combination");
	bpp = 4 * depth;
	break;

      default:
	NSLog(@"NSBitmapImageRep+PNG: unknown color type %i", type);
	RELEASE(self);
	return nil;
    }

  buf = NSZoneMalloc([self zone], bytes_per_row * height);

  {
    png_bytep row_pointers[height];
    int i;

    for (i = 0; i < height; i++)
      {
        row_pointers[i] = buf + i * bytes_per_row;
      }

    png_read_image(png_struct, row_pointers);
  }

  self = [self initWithBitmapDataPlanes: &buf
                             pixelsWide: width
                             pixelsHigh: height
                          bitsPerSample: depth
                        samplesPerPixel: channels
                               hasAlpha: alpha
                               isPlanar: NO
                         colorSpaceName: colorspace
                           bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
                            bytesPerRow: bytes_per_row
                           bitsPerPixel: bpp];
  
  _imageData = [[NSData alloc]
    initWithBytesNoCopy: buf
		 length: bytes_per_row * height];

  if (png_get_valid(png_struct, png_info, PNG_INFO_gAMA))
  {
    double file_gamma = 2.2;
    if (PNG_FLOATING_POINT)
    {
      png_get_gAMA(png_struct, png_info, &file_gamma);
      // remap file_gamma [1.0, 2.5] to property [0.0, 1.0]
      file_gamma = (file_gamma - 1.0)/1.5;
    }
    else	// fixed point
    {
      png_fixed_point int_gamma = 220000;
      png_get_gAMA_fixed(png_struct, png_info, &int_gamma);
      // remap gamma [0.0, 1.0] to [100000, 250000]
      file_gamma = ((double)int_gamma - 100000.0)/150000.0;
    }
    [self setProperty: NSImageGamma
            withValue: [NSNumber numberWithDouble: file_gamma]];
    //NSLog(@"PNG file gamma: %f", file_gamma);
   } 

  if (png_get_valid(png_struct, png_info, PNG_INFO_pHYs))
  {
    png_uint_32 xppm = png_get_x_pixels_per_meter(png_struct, png_info);
    png_uint_32 yppm = png_get_y_pixels_per_meter(png_struct, png_info);

    if (xppm != 0 && yppm != 0)
      {
	const CGFloat pointsPerMeter = 39.3700787 * 72.0;
	NSSize sizeInPoints = NSMakeSize((width / (CGFloat)xppm) * pointsPerMeter,
					 (height / (CGFloat)yppm) * pointsPerMeter);

	// HACK: PNG can not represent 72DPI exactly. If the ppm value is near 72DPI,
	// assume it is exactly 72 DPI. Note that the same problem occurrs at 144DPI...
	// so don't use PNG for resolution independent graphics.
	if (xppm  == 2834 || xppm == 2835)
	  {
	    sizeInPoints.width = width;
	  }
	if (yppm == 2834 || yppm == 2835)
	  {
	    sizeInPoints.height = height;
	  }

	[self setSize: sizeInPoints];
      }
  }

  png_destroy_read_struct(&png_struct, &png_info, &png_end_info);

  return self;
}

/***** PNG writing support ******/
static void writer_func(png_structp png_struct, png_bytep data,
			png_size_t length)
{
  NSMutableData * PNGRep = png_get_io_ptr(png_struct);
  [PNGRep appendBytes: data length: length];
}

- (NSData *) _PNGRepresentationWithProperties: (NSDictionary *) properties
{
  png_structp png_struct;
  png_infop png_info;

  int width, height, depth;
  unsigned char * bitmapData;
  int bytes_per_row;
  NSString * colorspace;
  NSMutableData * PNGRep = nil;
  int type = -1;	// illegal value
  int interlace = PNG_INTERLACE_NONE;
  int transforms = PNG_TRANSFORM_IDENTITY;	// no transformations
  NSNumber * gammaNumber = nil;
  double gamma = 0.0;
  
  // Need to convert to non-pre-multiplied format
  if ([self isPlanar] || !(_format & NSAlphaNonpremultipliedBitmapFormat))
  {
    NSBitmapImageRep *converted = [self _convertToFormatBitsPerSample: _bitsPerSample
                                                      samplesPerPixel: _numColors
                                                             hasAlpha: _hasAlpha
                                                             isPlanar: NO
                                                       colorSpaceName: _colorSpace
                                                         bitmapFormat: _format | NSAlphaNonpremultipliedBitmapFormat 
                                                          bytesPerRow: _bytesPerRow
                                                         bitsPerPixel: _bitsPerPixel];

    return [converted _PNGRepresentationWithProperties: properties];
  } 
  // get the image parameters
  width = [self pixelsWide];
  height = [self pixelsHigh];
  bytes_per_row = [self bytesPerRow];
  colorspace = [self colorSpaceName];
  depth = [self bitsPerSample];
  gammaNumber = [properties objectForKey: NSImageGamma];
  gamma = [gammaNumber doubleValue];
  if ([[properties objectForKey: NSImageInterlaced] boolValue])
    interlace = PNG_INTERLACE_ADAM7;

  if ([colorspace isEqualToString: NSCalibratedWhiteColorSpace] ||
      [colorspace isEqualToString: NSDeviceWhiteColorSpace])
    type = PNG_COLOR_TYPE_GRAY;
  if ([colorspace isEqualToString: NSCalibratedRGBColorSpace] ||
      [colorspace isEqualToString: NSDeviceRGBColorSpace])
    type = PNG_COLOR_TYPE_RGB;
  if ([self hasAlpha]) type = type | PNG_COLOR_MASK_ALPHA;

  // make the PNG structures
  // ignore errors until I write the handlers
  png_struct = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_struct)
    {
      return nil;
    }

  png_info = png_create_info_struct(png_struct);
  if (!png_info)
    {
      png_destroy_write_struct(&png_struct, NULL);
      return nil;
    }

  if (setjmp(png_jmpbuf(png_struct)))
    {
      png_destroy_write_struct(&png_struct, &png_info);
      return nil;
    }

  // init structures
  PNGRep = [NSMutableData dataWithLength: 0];
#if PNG_LIBPNG_VER < 10500
  // I don't think this was ever needed as png_create_info_struct()
  // sets up the structure correctly and we rely on that in all other places.
  png_info_init_3(&png_info, png_sizeof(png_info));
#endif
  png_set_write_fn(png_struct, PNGRep, writer_func, NULL);
  png_set_IHDR(png_struct, png_info, width, height, depth,
   type, interlace, PNG_COMPRESSION_TYPE_BASE,
   PNG_FILTER_TYPE_BASE);

  if (gammaNumber)
  {
    NSLog(@"PNGRepresentation: gamma support is experimental");
    if (PNG_FLOATING_POINT)
    {
      // remap gamma [0.0, 1.0] to [1.0, 2.5]
      png_set_gAMA(png_struct, png_info, (gamma * 1.5 + 1.0));
    }
    else	// fixed point
    {
      // remap gamma [0.0, 1.0] to [100000, 250000]
      int int_gamma = (int)(gamma * 150000.0 + 100000.0);
      png_set_gAMA_fixed(png_struct, png_info, int_gamma);
    }
   } 

  // get rgb data and row pointers and
  // write PNG out to NSMutableData
  bitmapData = [self bitmapData];
  {
    unsigned char *row_pointers[height];
    int i;
    for (i = 0 ; i < height ; i++)
      row_pointers[i] = bitmapData + i * bytes_per_row;
    png_set_rows(png_struct, png_info, row_pointers);

    png_write_png(png_struct, png_info, transforms, NULL);
  }
               
  //NSLog(@"PNG representation is experimental: %i bytes written", [PNGRep length]);
  png_destroy_write_struct(&png_struct, &png_info);
  return PNGRep;
}
@end

#else /* !HAVE_LIBPNG */

@implementation NSBitmapImageRep (PNG)
+ (BOOL) _bitmapIsPNG: (NSData *)imageData
{
  return NO;
}
- (id) _initBitmapFromPNG: (NSData *)imageData
{
  RELEASE(self);
  return nil;
}
- (NSData *) _PNGRepresentationWithProperties: (NSDictionary *) properties
{
  return nil;
}
@end

#endif /* !HAVE_LIBPNG */

