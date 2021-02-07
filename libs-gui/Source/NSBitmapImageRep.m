/** <title>NSBitmapImageRep.m</title>

   <abstract>Bitmap image representation.</abstract>

   Copyright (C) 1996-2017 Free Software Foundation, Inc.
   
   Author:  Adam Fedor <fedor@gnu.org>
   Date: Feb 1996
   
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

#include <stdlib.h>
#include <math.h>
#include <tiff.h>

#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSValue.h>
#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSView.h"
#import "AppKit/NSBitmapImageRep.h"

#import "NSBitmapImageRep+GIF.h"
#import "NSBitmapImageRep+JPEG.h"
#import "NSBitmapImageRep+PNG.h"
#import "NSBitmapImageRep+PNM.h"
#import "NSBitmapImageRep+ICNS.h"
#import "NSBitmapImageRepPrivate.h"
#import "GSGuiPrivate.h"

#include "nsimage-tiff.h"

/* Maximum number of planes */
#define MAX_PLANES 5


/**
  <unit>
  <heading>Class Description</heading>
  <p>
  NSBitmapImageRep is an image representation for handling images composed
  of pixels. The standard image format for NSBitmapImageRep is the TIFF
  format. However, through the use of image filters and other methods, many
  other standard image formats can be handled by NSBitmapImageRep.

  Images are typically handled through the NSImage class and there is often
  no need to use the NSBitmapImageRep class directly. However there may
  be cases where you want to manipulate the image bitmap data directly.
  </p>
  </unit>
*/ 
@implementation NSBitmapImageRep 

/** Returns YES if the image stored in data can be read and decoded */
+ (BOOL) canInitWithData: (NSData *)data
{
  if (data == nil)
    {
      return NO;
    }

#if HAVE_LIBPNG
  if ([self _bitmapIsPNG: data])
    return YES;
#endif

  if ([self _bitmapIsPNM: data])
    return YES;

#if HAVE_LIBJPEG
  if ([self _bitmapIsJPEG: data])
    return YES;
#endif

#if HAVE_LIBUNGIF || HAVE_LIBGIF
  if ([self _bitmapIsGIF: data])
    return YES;
#endif

  if ([self _bitmapIsICNS: data])
    return YES;

  if ([self _bitmapIsTIFF: data])
    return YES;

  return NO;
}

/** Returns a list of image filename extensions that are understood by
    NSBitmapImageRep.  */
+ (NSArray *) imageUnfilteredFileTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects:
	@"tiff", @"tif",
	@"pnm", @"ppm",
#if HAVE_LIBUNGIF || HAVE_LIBGIF
	@"gif",
#endif
#if HAVE_LIBJPEG
	@"jpeg", @"jpg",
#endif
#if HAVE_LIBPNG
	@"png",
#endif
	@"icns",
	nil];
    }

  return types;
}

/** Returns a list of image pasteboard types that are understood by
    NSBitmapImageRep.  */
+ (NSArray *) imageUnfilteredPasteboardTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      types = [[NSArray alloc] initWithObjects: NSTIFFPboardType, nil];
    }
  
  return types;
}

/** <p>Returns a newly allocated NSBitmapImageRep object representing the
    image stored in imageData. If the image data contains more than one
    image, the first one is choosen.</p><p>See Also: +imageRepsWithData:</p>  
*/
+ (id) imageRepWithData: (NSData *)imageData
{
  return AUTORELEASE([[self alloc] initWithData: imageData]);
}

/**<p>Returns an array containing newly allocated NSBitmapImageRep
    objects representing the images stored in imageData.</p>
    <p>See Also: +imageRepWithData:</p>
*/
+ (NSArray*) imageRepsWithData: (NSData *)imageData
{
  if (imageData == nil)
    {
      NSLog(@"NSBitmapImageRep: nil image data");
      return [NSArray array];
    }

  if ([self _bitmapIsPNG: imageData])
    {
      NSBitmapImageRep *rep;
      NSArray *a;

      rep = [[self alloc] _initBitmapFromPNG: imageData];
      if (!rep)
        return [NSArray array];
      a = [NSArray arrayWithObject: rep];
      DESTROY(rep);
      return a;
    }

  if ([self _bitmapIsPNM: imageData])
    {
      NSBitmapImageRep *rep;
      NSArray *a;

      rep = [[self alloc] _initBitmapFromPNM: imageData
			      errorMessage: NULL];
      if (!rep)
        return [NSArray array];
      a = [NSArray arrayWithObject: rep];
      DESTROY(rep);
      return a;
    }

  if ([self _bitmapIsJPEG: imageData])
    {
      NSBitmapImageRep *rep;
      NSArray *a;

      rep = [[self alloc] _initBitmapFromJPEG: imageData
			       errorMessage: NULL];
      if (!rep)
        return [NSArray array];
      a = [NSArray arrayWithObject: rep];
      DESTROY(rep);
      return a;
    }

  if ([self _bitmapIsGIF: imageData])
    {
      NSBitmapImageRep *rep;
      NSArray *a;

      rep = [[self alloc] _initBitmapFromGIF: imageData
			      errorMessage: NULL];
      if (!rep)
        return [NSArray array];
      a = [NSArray arrayWithObject: rep];
      DESTROY(rep);
      return a;
    }

  if ([self _bitmapIsICNS: imageData])
    {
      return [self _imageRepsWithICNSData: imageData];
    }

  if ([self _bitmapIsTIFF: imageData])
    {
      return [self _imageRepsWithTIFFData: imageData];
    }

  NSLog(@"NSBitmapImageRep: unable to parse bitmap image data");
  return [NSArray array];
}

/** Loads only the default (first) image from the image contained in
   data. */
- (id) initWithData: (NSData *)imageData
{
  Class class;

  if (imageData == nil)
    {
      RELEASE(self);
      return nil;
    }

  class = [self class];
  if ([class _bitmapIsPNG: imageData])
    return [self _initBitmapFromPNG: imageData];

  if ([class _bitmapIsPNM: imageData])
    return [self _initBitmapFromPNM: imageData
		       errorMessage: NULL];

  if ([class _bitmapIsJPEG: imageData])
    return [self _initBitmapFromJPEG: imageData
			errorMessage: NULL];

  if ([class _bitmapIsGIF: imageData])
    return [self _initBitmapFromGIF: imageData
		       errorMessage: NULL];

  if ([class _bitmapIsICNS: imageData])
    return [self _initBitmapFromICNS: imageData];

  if ([class _bitmapIsTIFF: imageData])
    return [self _initBitmapFromTIFF: imageData];

  DESTROY(self);
  return nil;
}

/** Initialize with bitmap data from a rect within the focused view */
- (id) initWithFocusedViewRect: (NSRect)rect
{
  NSInteger bps, spp, alpha, format;
  NSSize size;
  NSString *space;
  unsigned char *planes[4];
  NSDictionary *dict;

  dict = [GSCurrentContext() GSReadRect: rect];
  if (dict == nil)
    {
      NSLog(@"NSBitmapImageRep initWithFocusedViewRect: failed");
      RELEASE(self);
      return nil;
    }
  _imageData = RETAIN([dict objectForKey: @"Data"]);
  if (_imageData == nil || [_imageData length] == 0)
    {
      NSLog(@"NSBitmapImageRep initWithFocusedViewRect: failed");
      RELEASE(self);
      return nil;
    }
  bps = [[dict objectForKey: @"BitsPerSample"] intValue];
  if (bps == 0)
    bps = 8;
  spp = [[dict objectForKey: @"SamplesPerPixel"] intValue];
  alpha = [[dict objectForKey: @"HasAlpha"] intValue];
  size = [[dict objectForKey: @"Size"] sizeValue];
  space = [dict objectForKey: @"ColorSpace"];
  format = [[dict objectForKey: @"BitmapFormat"] intValue];
  planes[0] = (unsigned char *)[_imageData bytes];
  self = [self initWithBitmapDataPlanes: planes
               pixelsWide: size.width
               pixelsHigh: size.height
               bitsPerSample: bps
               samplesPerPixel: spp
               hasAlpha: (alpha) ? YES : NO
               isPlanar: NO
               colorSpaceName: space
               bitmapFormat: format
               bytesPerRow: 0
               bitsPerPixel: 0];
  return self;
}

/** 
    <init />
    <p>
    Initializes a newly created NSBitmapImageRep object to hold image data
    specified in the planes buffer and organized according to the
    additional arguments passed into the method.
    </p>
    <p>
    The planes argument is an array of char pointers where each array
    holds a single component or plane of data. Note that if data is
    passed into the method via planes, the data is NOT copied and not
    freed when the object is deallocated. It is assumed that the data
    will always be available. If planes is NULL, then a suitable amount
    of memory will be allocated to store the information needed. One can
    then obtain a pointer to the planes data using the -bitmapData or
    -getBitmapDataPlanes: method.
    </p>
    <p>
    Each component of the data is in "standard" order, such as red, green,
    blue for RGB color images. The transparency component, if these is one, should
    always be last.
    </p>
    <p>
    The other arguments to the method consist of:
    </p>
    <deflist>
      <term>width and height</term>
      <desc>The width and height of the image in pixels</desc>
      <term>bps</term>
      <desc>
      The bits per sample or the number of bits used to store a number in
      one component of one pixel of the image. Typically this is 8 (bits)
      but can be 2 or 4, although not all values are supported.
      </desc>
      <term>spp</term>
      <desc>
      Samples per pixel, or the number of components of color in the pixel.
      For instance this would be 4 for an RGB image with transparency.
      </desc>
      <term>alpha</term>
      <desc>
      Set to YES if the image has a transparency component.
      </desc>
      <term>isPlanar</term>
      <desc>
      Set to YES if the data is arranged in planes, i.e. one component
      per buffer as stored in the planes array. If NO, then the image data
      is mixed in one buffer. For instance, for RGB data, the first sample
      would contain red, then next green, then blue, followed by red for the
      next pixel.
      </desc>
      <term>colorSpaceName</term>
      <desc>
      This argument specifies how the data values are to be interpreted.
      Possible values include the typical colorspace names (although
      not all values are currently supported)
      </desc>
      <term>rowBytes</term>
      <desc>
      Specifies the number of bytes contained in a single scan line of the
      data. Normally this can be computed from the width of the image,
      the samples per pixel and the bits per sample. However, if the data
      is aligned along word boundaries, this value may differ from this.
      If rowBytes is 0, the method will calculate the value assuming there
      are no extra bytes at the end of the scan line.
      </desc>
      <term>pixelBits</term>
      <desc>
      This is normally bps for planar data and bps times spp for non-planar
      data, but sometimes images have extra bits. If pixelBits is 0 it
      will be calculated as described above.
      </desc>
      </deflist>
*/
- (id) initWithBitmapDataPlanes: (unsigned char **)planes
                     pixelsWide: (NSInteger)width
                     pixelsHigh: (NSInteger)height
                  bitsPerSample: (NSInteger)bitsPerSample
                samplesPerPixel: (NSInteger)samplesPerPixel
                       hasAlpha: (BOOL)alpha
                       isPlanar: (BOOL)isPlanar
                 colorSpaceName: (NSString *)colorSpaceName
                    bytesPerRow: (NSInteger)rowBytes
                   bitsPerPixel: (NSInteger)pixelBits
{
  return [self initWithBitmapDataPlanes: planes
               pixelsWide: width
               pixelsHigh: height
               bitsPerSample: bitsPerSample
               samplesPerPixel: samplesPerPixel
               hasAlpha: alpha
               isPlanar: isPlanar
               colorSpaceName: colorSpaceName
               bitmapFormat: 0
               bytesPerRow: rowBytes
               bitsPerPixel: pixelBits];
}

- (id) initWithBitmapDataPlanes: (unsigned char**)planes
                     pixelsWide: (NSInteger)width
                     pixelsHigh: (NSInteger)height
                  bitsPerSample: (NSInteger)bps
                samplesPerPixel: (NSInteger)spp
                       hasAlpha: (BOOL)alpha
                       isPlanar: (BOOL)isPlanar
                 colorSpaceName: (NSString*)colorSpaceName
                   bitmapFormat: (NSBitmapFormat)bitmapFormat 
                    bytesPerRow: (NSInteger)rowBytes
                   bitsPerPixel: (NSInteger)pixelBits
{
  NSDebugLLog(@"NSImage", @"Creating bitmap image with pw %d ph %d bps %d spp %d alpha %d, planar %d cs %@",
              (int)width,(int) height, (int)bps, (int)spp, alpha, isPlanar, colorSpaceName);
  if (!bps || !spp || !width || !height) 
    {
      [NSException raise: NSInvalidArgumentException
        format: @"Required arguments not specified creating NSBitmapImageRep"];
    }

  _pixelsWide = width;
  _pixelsHigh = height;
  _size.width  = width;
  _size.height = height;
  _bitsPerSample = bps;
  _numColors  = spp;
  _hasAlpha   = alpha;  
  _isPlanar   = isPlanar;
  _colorSpace = RETAIN(colorSpaceName);
  _format = bitmapFormat;
  if (!pixelBits)
    pixelBits = bps * ((_isPlanar) ? 1 : spp);
  _bitsPerPixel = pixelBits;
  if (!rowBytes) 
    rowBytes = ceil((float)width * _bitsPerPixel / 8);
  _bytesPerRow = rowBytes;

  _imagePlanes = NSAllocateCollectable(sizeof(unsigned char*) * MAX_PLANES, 0);
  if (planes) 
    {
      unsigned int i;

      for (i = 0; i < MAX_PLANES; i++)
 	_imagePlanes[i] = NULL;
      for (i = 0; i < ((_isPlanar) ? _numColors : 1); i++)
 	_imagePlanes[i] = planes[i];
    }
  else
    {
      unsigned char *bits;
      NSUInteger length;
      unsigned int i;

      // No image data was given, allocate it.
      length = (NSUInteger)((_isPlanar) ? _numColors : 1) * _bytesPerRow * 
	  _pixelsHigh * sizeof(unsigned char);
      // Create a mutable data object although we never use it as such
      _imageData = [[NSMutableData alloc] initWithLength: length];
      bits = (unsigned char *)[_imageData bytes];
      _imagePlanes[0] = bits;
      if (_isPlanar) 
	{
	  for (i = 1; i < _numColors; i++) 
	    _imagePlanes[i] = bits + i * _bytesPerRow * _pixelsHigh;
	  for (i = _numColors; i < MAX_PLANES; i++) 
	    _imagePlanes[i] = NULL;
	}
      else
	{
	  for (i = 1; i < MAX_PLANES; i++) 
	    _imagePlanes[i] = NULL;
	}      
    }

  if (alpha)
    {
      unsigned char	*bData = (unsigned char*)[self bitmapData];
      BOOL		allOpaque = YES;
      unsigned		offset = _numColors - 1;
      unsigned		limit = _size.height * _size.width;
      unsigned		i;

      for (i = 0; i < limit; i++)
	{
	  unsigned	a;

	  bData += offset;
	  a = *bData++;
	  if (a != 255)
	    {
	      allOpaque = NO;
	      break;
	    }
	}
      [self setOpaque: allOpaque];
    }
  else
    {
      [self setOpaque: YES];
    }
  _properties = [[NSMutableDictionary alloc] init];

  return self;
}

- (void)colorizeByMappingGray:(CGFloat)midPoint 
		      toColor:(NSColor *)midPointColor 
		 blackMapping:(NSColor *)shadowColor
		 whiteMapping:(NSColor *)lightColor
{
  // TODO
}

- (id)initWithBitmapHandle:(void *)bitmap
{
  // TODO Only needed on MS Windows
  RELEASE(self);
  return nil;
}

- (id)initWithIconHandle:(void *)icon
{
  // TODO Only needed on MS Windows
  RELEASE(self);
  return nil;
}

- (id) initForIncrementalLoad
{
  // FIXME
  return self;
}

- (NSInteger) incrementalLoadFromData: (NSData *)data complete: (BOOL)complete
{
  if (!complete)
    {
      // we don't implement it really
      return NSImageRepLoadStatusWillNeedAllData;
    }
  return [self initWithData: data] ? NSImageRepLoadStatusCompleted : NSImageRepLoadStatusUnexpectedEOF;
}

- (void) dealloc
{
  NSZoneFree([self zone],_imagePlanes);
  RELEASE(_imageData);
  RELEASE(_properties);
  [super dealloc];
}

//
// Getting Information about the Image 
//
/** Returns the number of bits need to contain one pixels worth of data.
    This is normally the number of samples per pixel times the number of
    bits in one sample. */
- (NSInteger) bitsPerPixel
{
  return _bitsPerPixel;
}

/** Returns the number of samples in a pixel. For instance, a normal RGB
    image with transparency would have a samplesPerPixel of 4.  */
- (NSInteger) samplesPerPixel
{
  return _numColors;
}

/** Returns YES if the image components are stored separately. Returns
    NO if the components are meshed (i.e. all the samples for one pixel
    come before the next pixel).  */
- (BOOL) isPlanar
{
  return _isPlanar;
}

/** Returns the number of planes in an image.  Typically this is
    equal to the number of samples in a planar image or 1 for a non-planar
    image.  */
- (NSInteger) numberOfPlanes
{
  return (_isPlanar) ? _numColors : 1;
}

/** Returns the number of bytes in a plane. This is the number of bytes
    in a row times tne height of the image.  */
- (NSInteger) bytesPerPlane
{
  return _bytesPerRow*_pixelsHigh;
}

/** Returns the number of bytes in a row. This is typically based on the
    width of the image and the bits per sample and samples per pixel (if
    in medhed configuration). However it may differ from this if set
    explicitly in -initWithBitmapDataPlanes:pixelsWide:pixelsHigh:bitsPerSample:samplesPerPixel:hasAlpha:isPlanar:colorSpaceName:bytesPerRow:bitsPerPixel:.
*/
- (NSInteger) bytesPerRow
{
  return _bytesPerRow;
}

//
// Getting Image Data 
//
/** Returns the first plane of data representing the image.  */
- (unsigned char *) bitmapData
{
  unsigned char *planes[MAX_PLANES];
  [self getBitmapDataPlanes: planes];
  return planes[0];
}

/** Files the array data with pointers to each of the data planes
    representing the image. The data array must be allocated to contain
    at least -samplesPerPixel pointers.  */
- (void) getBitmapDataPlanes: (unsigned char **)data
{
  unsigned int i;

  if (data)
    {
      for (i = 0; i < _numColors; i++)
        {
          data[i] = _imagePlanes[i];
        }
    }
}

- (NSBitmapFormat) bitmapFormat
{
  return _format;
}

/*
 * This code was copied over from XGBitmap.m
 * Here we extract a value a given number of bits wide from a bit
 * offset into a block of memory starting at "base". The bit numbering
 * is assumed to be such that a bit offset of zero and a width of 4 gives
 * the upper 4 bits of the first byte, *not* the lower 4 bits. We do allow
 * the value to cross a byte boundary, though it is unclear as to whether
 * this is strictly necessary for OpenStep tiffs.
 */
static unsigned int
_get_bit_value(unsigned char *base, long msb_off, int bit_width)
{
  long lsb_off, byte1, byte2;
  int shift, value;

  /*
   * Firstly we calculate the position of the msb and lsb in terms
   * of bit offsets and thus byte offsets. The shift is the number of
   * spare bits left in the byte containing the lsb
   */
  lsb_off= msb_off+bit_width-1;
  byte1= msb_off/8;
  byte2= lsb_off/8;
  shift= 7-(lsb_off%8);

  /*
   * We now get the value from the byte array, possibly using two bytes if
   * the required set of bits crosses the byte boundary. This is then shifted
   * down to it's correct position and extraneous bits masked off before
   * being returned.
   */
  value=base[byte2];
  if (byte1!=byte2)
    value|= base[byte1]<<8;
  value >>= shift;

  return value & ((1<<bit_width)-1);
}

/**
 * Returns the values of the components of pixel (x,y), where (0,0) is the 
 * top-left pixel in the image, by storing them in the array pixelData.
 */
- (void) getPixel: (NSUInteger[])pixelData atX: (NSInteger)x y: (NSInteger)y
{
  NSInteger i;
  NSInteger offset;
  NSInteger line_offset;

  if (x < 0 || y < 0 || x >= _pixelsWide || y >= _pixelsHigh)
    {
      // outside
      return;
    }

  line_offset = _bytesPerRow * y;
  if (_isPlanar)
    {
      if (_bitsPerSample == 8)
        {
          offset = x + line_offset;
          for (i = 0; i < _numColors; i++)
            {
              pixelData[i] = _imagePlanes[i][offset];
            }
        }
      else
        {
          offset = _bitsPerPixel * x;
          for (i = 0; i < _numColors; i++)
            {
              pixelData[i] = _get_bit_value(_imagePlanes[i] + line_offset, 
                                            offset, _bitsPerSample);
            }
        }
    }
  else
    {
      if (_bitsPerSample == 8)
        {
          offset = (_bitsPerPixel * x) / 8 + line_offset;
          for (i = 0; i < _numColors; i++)
            {
              pixelData[i] = _imagePlanes[0][offset + i];
            }
        }
      else
        {
          offset = _bitsPerPixel * x;
          for (i = 0; i < _numColors; i++)
            {
              pixelData[i] = _get_bit_value(_imagePlanes[0] + line_offset, 
                                            offset, _bitsPerSample);
              offset += _bitsPerSample;
            }
        }
    }
}

static void
_set_bit_value(unsigned char *base, long msb_off, int bit_width, 
               unsigned int value)
{
  long lsb_off, byte1, byte2;
  int shift;
  int all;

  /*
   * Firstly we calculate the position of the msb and lsb in terms
   * of bit offsets and thus byte offsets. The shift is the number of
   * spare bits left in the byte containing the lsb
   */
  lsb_off= msb_off+bit_width-1;
  byte1= msb_off/8;
  byte2= lsb_off/8;
  shift= 7-(lsb_off%8);

  /*
   * We now set the value in the byte array, possibly using two bytes if
   * the required set of bits crosses the byte boundary. This value is 
   * first shifted up to it's correct position and extraneous bits are 
   * masked off.
   */
  value &= ((1<<bit_width)-1);
  value <<= shift;
  all = ((1<<bit_width)-1) << shift;

  if (byte1 != byte2)
    base[byte1] = (value >> 8) | (base[byte1] & ~(all >> 8));
  base[byte2] = (value & 255) | (base[byte2] & ~(all & 255));
}

/**
 * Sets the components of pixel (x,y), where (0,0) is the top-left pixel in
 * the image, to the given array of pixel components. 
 */
- (void) setPixel: (NSUInteger[])pixelData atX: (NSInteger)x y: (NSInteger)y
{
  NSInteger i;
  NSInteger offset;
  NSInteger line_offset;

  if (x < 0 || y < 0 || x >= _pixelsWide || y >= _pixelsHigh)
    {
      // outside
      return;
    }

  if (!_imagePlanes || !_imagePlanes[0])
    {
      // allocate plane memory
      [self bitmapData];
    }

  line_offset = _bytesPerRow * y;
  if (_isPlanar)
    {
      if (_bitsPerSample == 8)
        {
          offset = x + line_offset;
          for (i = 0; i < _numColors; i++)
            {
              _imagePlanes[i][offset] = pixelData[i];
            }
        }
      else
        {
          offset = _bitsPerPixel * x;
          for (i = 0; i < _numColors; i++)
            {
              _set_bit_value(_imagePlanes[i] + line_offset, 
                             offset, _bitsPerSample, pixelData[i]);
            }
        }
    }
  else
    {
      if (_bitsPerSample == 8)
        {
          offset = (_bitsPerPixel * x) / 8 + line_offset;
          for (i = 0; i < _numColors; i++)
            {
              _imagePlanes[0][offset + i] = pixelData[i];
            }
        }
      else
        {
          offset = _bitsPerPixel * x;
          for (i = 0; i < _numColors; i++)
            {
              _set_bit_value(_imagePlanes[0] + line_offset, 
                             offset, _bitsPerSample, pixelData[i]);
              offset += _bitsPerSample;
            }
        }
    }
}

/**
 * Returns an NSColor object representing the color of the pixel (x,y), where
 * (0,0) is the top-left pixel in the image.
 */
- (NSColor*) colorAtX: (NSInteger)x y: (NSInteger)y
{
  NSUInteger pixelData[5];

  if (x < 0 || y < 0 || x >= _pixelsWide || y >= _pixelsHigh)
    {
      // outside
      return nil;
    }

  [self getPixel: pixelData atX: x y: y];
  if ([_colorSpace isEqualToString: NSCalibratedRGBColorSpace]
      || [_colorSpace isEqualToString: NSDeviceRGBColorSpace])
    {
      NSUInteger ir, ig, ib, ia;
      CGFloat fr, fg, fb, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      if (_hasAlpha)
        {
          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              ia = pixelData[0];
              ir = pixelData[1];
              ig = pixelData[2];
              ib = pixelData[3];
            }
          else
            {
              ir = pixelData[0];
              ig = pixelData[1];
              ib = pixelData[2];
              ia = pixelData[3];
            }

          // Scale to [0.0 ... 1.0] and undo premultiplication
          fa = ia / scale;
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              fr = ir / scale;
              fg = ig / scale;
              fb = ib / scale;
            }
          else
            {
              fr = ir / (scale * fa);
              fg = ig / (scale * fa);
              fb = ib / (scale * fa);
            }
        }
      else
        {
          ir = pixelData[0];
          ig = pixelData[1];
          ib = pixelData[2];
          // Scale to [0.0 ... 1.0]
          fr = ir / scale;
          fg = ig / scale;
          fb = ib / scale;
          fa = 1.0;
        }
      if ([_colorSpace isEqualToString: NSCalibratedRGBColorSpace])
        {
          return [NSColor colorWithCalibratedRed: fr
                          green: fg
                          blue: fb
                          alpha: fa];
        }
      else
        {
          return [NSColor colorWithDeviceRed: fr
                          green: fg
                          blue: fb
                          alpha: fa];
        }
    }
  else if ([_colorSpace isEqual: NSDeviceWhiteColorSpace]
           || [_colorSpace isEqual: NSCalibratedWhiteColorSpace])
    {
      NSUInteger iw, ia;
      CGFloat fw, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      if (_hasAlpha)
        {
          // FIXME: This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
                ia = pixelData[0];
                iw = pixelData[1];
            }
          else
            {
                iw = pixelData[0];
                ia = pixelData[1];
            }

          // Scale to [0.0 ... 1.0] and undo premultiplication
          fa = ia / scale;
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              fw = iw / scale;
            }
          else
            {
              fw = iw / (scale * fa);
            }
        }
      else
        {
          // FIXME: This order depends on the bitmap format
          iw = pixelData[0];
          // Scale to [0.0 ... 1.0]
          fw = iw / scale;
          fa = 1.0;
        }
      if ([_colorSpace isEqualToString: NSCalibratedWhiteColorSpace])
        {
          return [NSColor colorWithCalibratedWhite: fw
                          alpha: fa];
        }
      else
        {
          return [NSColor colorWithDeviceWhite: fw
                          alpha: fa];
        }
    }
  else if ([_colorSpace isEqual: NSDeviceBlackColorSpace]
           || [_colorSpace isEqual: NSCalibratedBlackColorSpace])
    {
      NSUInteger ib, ia;
      CGFloat fw, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      if (_hasAlpha)
        {
          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              ia = pixelData[0];
              ib = pixelData[1];
            }
          else
            {
              ib = pixelData[0];
              ia = pixelData[1];
            }
          // Scale to [0.0 ... 1.0] and undo premultiplication
          fa = ia / scale;
         if (_format & NSAlphaNonpremultipliedBitmapFormat)
           {
             fw = 1.0 - ib / scale;
           }
         else
           {
             fw = 1.0 - ib / (scale * fa);
           }
        }
      else
        {
          ib = pixelData[0];
          // Scale to [0.0 ... 1.0]
          fw = 1.0 - ib / scale;
          fa = 1.0;
        }
      if ([_colorSpace isEqualToString: NSCalibratedBlackColorSpace])
        {
          return [NSColor colorWithCalibratedWhite: fw
                          alpha: fa];
        }
      else
        {
          return [NSColor colorWithDeviceWhite: fw
                          alpha: fa];
        }
		}
  else if ([_colorSpace isEqual: NSDeviceCMYKColorSpace])
    {
      NSUInteger ic, im, iy, ib, ia;
      CGFloat fc, fm, fy, fb, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      if (_hasAlpha)
        {
          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              ia = pixelData[0];
              ic = pixelData[1];
              im = pixelData[2];
              iy = pixelData[3];
              ib = pixelData[4];
            }
          else
            {
              ic = pixelData[0];
              im = pixelData[1];
              iy = pixelData[2];
              ib = pixelData[3];
              ia = pixelData[4];
            }

          // Scale to [0.0 ... 1.0] and undo premultiplication
          fa = ia / scale;
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              fc = ic / scale;
              fm = im / scale;
              fy = iy / scale;
              fb = ib / scale;
            }
          else
            {
              fc = ic / (scale * fa);
              fm = im / (scale * fa);
              fy = iy / (scale * fa);
              fb = ib / (scale * fa);
            }
        }
      else
        {
          ic = pixelData[0];
          im = pixelData[1];
          iy = pixelData[2];
          ib = pixelData[3];
          // Scale to [0.0 ... 1.0]
          fc = ic / scale;
          fm = im / scale;
          fy = iy / scale;
          fb = ib / scale;
          fa = 1.0;
        }

      return [NSColor colorWithDeviceCyan: fc
                      magenta: fm
                      yellow: fy
                      black: fb
                      alpha: fa];
    }

  return nil;
}

/**
 * Sets the color of pixel (x,y), where (0,0) is the top-left pixel in the
 * image.
 */
- (void) setColor: (NSColor*)color atX: (NSInteger)x y: (NSInteger)y
{
  NSUInteger pixelData[5];
  NSColor *conv;

  if (x < 0 || y < 0 || x >= _pixelsWide || y >= _pixelsHigh)
    {
      // outside
      return;
    }

  conv = [color colorUsingColorSpaceName: _colorSpace];
  if (!conv)
    {
      return;
    }
      
  if ([_colorSpace isEqualToString: NSCalibratedRGBColorSpace]
      || [_colorSpace isEqualToString: NSDeviceRGBColorSpace])
    {
      NSUInteger ir, ig, ib, ia;
      CGFloat fr, fg, fb, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      [conv getRed: &fr green: &fg blue: &fb alpha: &fa];
      if(_hasAlpha)
        {
          // Scale and premultiply alpha
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              ir = scale * fr;
              ig = scale * fg;
              ib = scale * fb;
            }
          else
            {
              ir = scale * fr * fa;
              ig = scale * fg * fa;
              ib = scale * fb * fa;
            }
          ia = scale * fa;

          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              pixelData[0] = ia;
              pixelData[1] = ir;
              pixelData[2] = ig;
              pixelData[3] = ib;
            }
          else
            {
              pixelData[0] = ir;
              pixelData[1] = ig;
              pixelData[2] = ib;
              pixelData[3] = ia;
            }
        }
      else
        {
          // Scale
          ir = scale * fr;
          ig = scale * fg;
          ib = scale * fb;
          // This order depends on the bitmap format
          pixelData[0] = ir;
          pixelData[1] = ig;
          pixelData[2] = ib;
        }
    }
  else if ([_colorSpace isEqual: NSDeviceWhiteColorSpace]
           || [_colorSpace isEqual: NSCalibratedWhiteColorSpace])
    {
      NSUInteger iw, ia;
      CGFloat fw, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      [conv getWhite: &fw alpha: &fa];
      if (_hasAlpha)
        {
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              iw = scale * fw;
            }
          else
            {
              iw = scale * fw * fa;
            }
          ia = scale * fa;

          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              pixelData[0] = ia;
              pixelData[1] = iw;
            }
          else
            {
              pixelData[0] = iw;
              pixelData[1] = ia;
            }
        }
      else
        {
          iw = scale * fw;
          pixelData[0] = iw;
        }
    }
  else if ([_colorSpace isEqual: NSDeviceBlackColorSpace]
           || [_colorSpace isEqual: NSCalibratedBlackColorSpace])
    {
      NSUInteger iw, ia;
      CGFloat fw, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      [conv getWhite: &fw alpha: &fa];
      if (_hasAlpha)
        {
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              iw = scale * (1 - fw);
            }
          else
            {
              iw = scale * (1 - fw) * fa;
            }
          ia = scale * fa;

          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              pixelData[0] = ia;
              pixelData[1] = iw;
            }
          else
            {
              pixelData[0] = iw;
              pixelData[1] = ia;
            }
        }
      else
        {
          iw = scale * (1 - fw);
          pixelData[0] = iw;
        }
    }
  else if ([_colorSpace isEqual: NSDeviceCMYKColorSpace])
    {
      NSUInteger ic, im, iy, ib, ia;
      CGFloat fc, fm, fy, fb, fa;
      CGFloat scale;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      [conv getCyan: &fc magenta: &fm yellow: &fy black: &fb alpha: &fa];
      if(_hasAlpha)
        {
          if (_format & NSAlphaNonpremultipliedBitmapFormat)
            {
              ic = scale * fc;
              im = scale * fm;
              iy = scale * fy;
              ib = scale * fb;
            }
          else
            {
              ic = scale * fc * fa;
              im = scale * fm * fa;
              iy = scale * fy * fa;
              ib = scale * fb * fa;
            }
          ia = scale * fa;

          // This order depends on the bitmap format
          if (_format & NSAlphaFirstBitmapFormat)
            {
              pixelData[0] = ia;
              pixelData[1] = ic;
              pixelData[2] = im;
              pixelData[3] = iy;
              pixelData[4] = ib;
            }
          else
            {
              pixelData[0] = ic;
              pixelData[1] = im;
              pixelData[2] = iy;
              pixelData[3] = ib;
              pixelData[4] = ia;
            }
        }
      else
        {
          ic = scale * fc;
          im = scale * fm;
          iy = scale * fy;
          ib = scale * fb;
          // This order depends on the bitmap format
          pixelData[0] = ic;
          pixelData[1] = im;
          pixelData[2] = iy;
          pixelData[3] = ib;
        }          
    }
  else
    {
      // FIXME: Other colour spaces not implemented
      return;
    }

  [self setPixel: pixelData atX: x y: y];
}

/** Draws the image in the current window according the information
    from the current gState, including information about the current
    point, scaling, etc.  */
- (BOOL) draw
{
  NSRect irect = NSMakeRect(0, 0, _size.width, _size.height);
  NSGraphicsContext *ctxt = GSCurrentContext();

  [self _premultiply];
  [ctxt GSDrawImage: irect : self];
  return YES;
}

//
// Producing a TIFF Representation of the Image 
//
/** Produces an NSData object containing a TIFF representation of all
   the images stored in anArray.  BUGS: Currently this only works if the
   images are NSBitmapImageRep objects.  */
+ (NSData*) TIFFRepresentationOfImageRepsInArray: (NSArray *)anArray
{
  NSEnumerator *enumerator = [anArray objectEnumerator];
  NSImageRep *rep;
  TIFF *image;
  NSTiffInfo info;
  char *bytes = 0;
  long length = 0;
  int num = 0;
  NSData *data;

  image = NSTiffOpenDataWrite(&bytes, &length);
  if (image == 0)
    {
      [NSException raise: NSTIFFException 
		   format: @"Opening data stream for writing"];
    }

  while ((rep = [enumerator nextObject]) != nil)
    {
      if ([rep isKindOfClass: self])
        {
          NSTIFFCompression compression;
          float factor;
          NSBitmapImageRep *bitmap = (NSBitmapImageRep*)rep;

          [bitmap getCompression: &compression
                          factor: &factor];
          [bitmap _fillTIFFInfo: &info
               usingCompression: compression
                         factor: factor];
          info.imageNumber = num++;
          info.numImages = [anArray count];
          info.subfileType = FILETYPE_PAGE;
          if (NSTiffWrite(image, &info, [bitmap bitmapData]) != 0)
            {
              [NSException raise: NSTIFFException format: @"Writing data"];
            }
	}
    }

  NSTiffClose(image);
  data = [NSData dataWithBytesNoCopy: bytes length: length];
  if (num > 0)
    {
      return data;
    }
  else
    {
      // FIXME: Not sure wether this is the correct behaviour, at least it was
      // the old one of this method.
      return nil;
    }
}

/** Produces an NSData object containing a TIFF representation of all
   the images stored in anArray. The image is compressed according to
   the compression type and factor. BUGS: Currently this only works if
   the images are NSBitmapImageRep objects. */
+ (NSData*) TIFFRepresentationOfImageRepsInArray: (NSArray *)anArray
				usingCompression: (NSTIFFCompression)compression
					  factor: (float)factor
{
  NSEnumerator *enumerator = [anArray objectEnumerator];
  NSImageRep *rep;
  NSTiffInfo info;
  TIFF *image;
  char *bytes = 0;
  long length = 0;
  int num = 0;
  NSData *data;

  image = NSTiffOpenDataWrite(&bytes, &length);
  if (image == 0)
    {
      [NSException raise: NSTIFFException 
		   format: @"Opening data stream for writing"];
    }

  while ((rep = [enumerator nextObject]) != nil)
    {
      if ([rep isKindOfClass: self])
        {
          [(NSBitmapImageRep*)rep _fillTIFFInfo: &info
                               usingCompression: compression
                                         factor: factor];
          info.imageNumber = num++;
          info.numImages = [anArray count];
          info.subfileType = FILETYPE_PAGE;
          if (NSTiffWrite(image, &info, [(NSBitmapImageRep*)rep bitmapData]) != 0)
            {
              [NSException raise: NSTIFFException format: @"Writing data"];
            }
	}
    }

  NSTiffClose(image);
  data = [NSData dataWithBytesNoCopy: bytes length: length];
  if (num > 0)
    {
      return data;
    }
  else
    {
      // FIXME: Not sure wether this is the correct behaviour, at least it was
      // the old one of this method.
      return nil;
    }
}

/** Returns an NSData object containing a TIFF representation of the
    receiver.  */
- (NSData*) TIFFRepresentation
{
  if ([self canBeCompressedUsing: _compression] == NO)
    {
      [self setCompression: NSTIFFCompressionNone factor: 0];
    }
  return [self TIFFRepresentationUsingCompression: _compression 
	       factor: _comp_factor];
}

/** Returns an NSData object containing a TIFF representation of the
    receiver. The TIFF data is compressed using compresssion type
    and factor.  */
- (NSData*) TIFFRepresentationUsingCompression: (NSTIFFCompression)compression
					factor: (float)factor
{
  NSTiffInfo	info;
  TIFF		*image;
  char		*bytes = 0;
  long		length = 0;

  image = NSTiffOpenDataWrite(&bytes, &length);
  if (image == 0)
    {
      [NSException raise: NSTIFFException 
		   format: @"Opening data stream for writing"];
    }

  [self _fillTIFFInfo: &info
     usingCompression: compression
               factor: factor];
  if (NSTiffWrite(image, &info, [self bitmapData]) != 0)
    {
      [NSException raise: NSTIFFException format: @"Writing data"];
    }
  NSTiffClose(image);
  return [NSData dataWithBytesNoCopy: bytes length: length];
}

/** <p> Returns a data object in the selected format with multiple images.</p>
  <p> See Also: -setProperty:withValue: for the options supported in the properties.</p>
  <p> FIXME: returns only the first image in the array, and only works for
  NSBitmapImageRep or subclasses thereof. </p>
*/
+ (NSData *)representationOfImageRepsInArray:(NSArray *)imageReps 
				   usingType:(NSBitmapImageFileType)storageType
				  properties:(NSDictionary *)properties
{
  // Partial implementation only returns data for the first imageRep in the array
  // and only works for NSBitmapImageRep or subclasses thereof. 
  //FIXME: This only outputs one of the ImageReps
  NSEnumerator *enumerator = [imageReps objectEnumerator];
  NSImageRep *rep;

  if (storageType == NSTIFFFileType)
    {
      NSNumber *comp_property = [properties objectForKey: NSImageCompressionMethod];
      NSNumber *factor_property = [properties objectForKey: NSImageCompressionFactor];
      
      if ((comp_property != nil) && (factor_property != nil))
        {
          float factor = [factor_property floatValue];
          NSTIFFCompression compression = [comp_property unsignedShortValue];

          return [self TIFFRepresentationOfImageRepsInArray: imageReps
                                           usingCompression: compression
                                                     factor: factor];
        }
      else
        {
          return [self TIFFRepresentationOfImageRepsInArray: imageReps];
        }
    }
  else
    {
      while ((rep = [enumerator nextObject]) != nil)
        {
          if ([rep isKindOfClass: self])
            {
              return [(NSBitmapImageRep*)rep representationUsingType: storageType
                                                          properties: properties];
            }
        }
    }

  return nil;
}

/** <p> Returns a data object in one of the supported bitmap graphics file types. 
  A limited set of options may be passed via the properties. If the passed in properties is nil,
  it falls back to the options set with -setProperty:withValue:. File types not yet
  implemented return nil and log an error message.</p>
  <p> See Also: -setProperty:withValue: for supported options in the properties. </p>
*/
- (NSData *)representationUsingType:(NSBitmapImageFileType)storageType 
			 properties:(NSDictionary *)properties
{
  // if it exists, the passed in properties takes precedence over the internal _properties
  NSDictionary * __properties;
  __properties = (properties)? properties : (NSDictionary *)_properties;

  switch (storageType)
  {
    case NSTIFFFileType:
    {
      NSNumber *property;
      float factor = _comp_factor;
      NSTIFFCompression compression = _compression;
      if ((property = [__properties objectForKey: NSImageCompressionMethod]))
        compression =  [property unsignedShortValue];
      if ((property = [__properties objectForKey: NSImageCompressionFactor]))
        factor = [property floatValue];
      if ([self canBeCompressedUsing: compression] == NO)
        {
          factor = 0.0;
          compression = NSTIFFCompressionNone;
        }
      return [self TIFFRepresentationUsingCompression: compression factor: factor];
    }

    case NSBMPFileType:
      NSLog(@"BMP representation is not yet implemented");
      return nil;

    case NSGIFFileType:
      return [self _GIFRepresentationWithProperties: __properties
                                       errorMessage: NULL];

    case NSJPEGFileType:
      return [self _JPEGRepresentationWithProperties: __properties
                                        errorMessage: NULL];

    case NSPNGFileType:
      return [self _PNGRepresentationWithProperties: __properties];
    
    case NSJPEG2000FileType:
      NSLog(@"JPEG2000 representation is not yet implemented");
      return nil;
  }
  return nil;
}

//
// Setting and Checking Compression Types 
//
/** Returns a C-array of available TIFF compression types.
 */
+ (void) getTIFFCompressionTypes: (const NSTIFFCompression **)list
			   count: (NSInteger *)numTypes
{
  // the GNUstep supported types
  static NSTIFFCompression	types[] = {
    NSTIFFCompressionNone,
    NSTIFFCompressionCCITTFAX3,
    NSTIFFCompressionCCITTFAX4,
    NSTIFFCompressionLZW,
    NSTIFFCompressionJPEG,
    NSTIFFCompressionNEXT,
    NSTIFFCompressionPackBits,
    NSTIFFCompressionOldJPEG
  };
  
  // check with libtiff to see what is really available
  NSInteger i, j;
  static NSTIFFCompression checkedTypes[8];
  for (i = 0, j = 0; i < 8; i++)
  {
    if (NSTiffIsCodecConfigured([NSBitmapImageRep _localFromCompressionType: types[i]]))
    {
      checkedTypes[j] = types[i];
      j++;
    }
  }
  if (list)
    *list = checkedTypes;
  if (numTypes)
    *numTypes = j;
}

/** Returns a localized string describing a TIFF compression type. */
+ (NSString*) localizedNameForTIFFCompressionType: (NSTIFFCompression)type
{
  switch (type)
    {
      case NSTIFFCompressionNone: return _(@"No Compression");
      case NSTIFFCompressionCCITTFAX3: return _(@"CCITTFAX3 Compression");
      case NSTIFFCompressionCCITTFAX4: return _(@"CCITTFAX4 Compression");
      case NSTIFFCompressionLZW: return _(@"LZW Compression");
      case NSTIFFCompressionJPEG: return _(@"JPEG Compression");
      case NSTIFFCompressionNEXT: return _(@"NEXT Compression");
      case NSTIFFCompressionPackBits: return _(@"PackBits Compression");
      case NSTIFFCompressionOldJPEG: return _(@"Old JPEG Compression");
      default: return nil;
    }
}

/** Returns YES if the receiver can be stored in a representation
    compressed using the compression type.  */
- (BOOL) canBeCompressedUsing: (NSTIFFCompression)compression
{
  BOOL does;
  int codecConf =
    NSTiffIsCodecConfigured([NSBitmapImageRep _localFromCompressionType: compression]);
  switch (compression)
    {
      case NSTIFFCompressionCCITTFAX3:
      case NSTIFFCompressionCCITTFAX4:
	if (_numColors == 1 && _bitsPerSample == 1 && codecConf != 0)
	  does = YES;
	else
	  does = NO;
	break;

      case NSTIFFCompressionLZW: 
      case NSTIFFCompressionNone:
      case NSTIFFCompressionJPEG:	// this is a GNUstep extension; Cocoa does not support
      case NSTIFFCompressionPackBits:
      case NSTIFFCompressionOldJPEG:
      case NSTIFFCompressionNEXT:
      default:
	does = (codecConf != 0);
    }
  return does;
}

/** Returns the receivers compression and compression factor, which is
    set either when the image is read in or by -setCompression:factor:.
    Factor is ignored in many compression schemes. For JPEG compression,
    factor can be any value from 0 to 1, with 1 being the maximum
    quality.  */
- (void) getCompression: (NSTIFFCompression*)compression
		 factor: (float*)factor
{
  *compression = _compression;
  *factor = _comp_factor;
}

- (void) setCompression: (NSTIFFCompression)compression
		 factor: (float)factor
{
  _compression = compression;
  _comp_factor = factor;
}

/** <p> Properties are key-value pairs associated with the representation. Arbitrary
  key-value pairs may be set. If the value is nil, the key is erased from properties.
  There are standard keys that are used to pass information
  and options related to the standard file types that may be read from or written to.
  Certain properties are automatically set when reading in image data.
  Certain properties may be set by the user prior to writing image data in order to set options
  for the data format. </p>
  <deflist>
    <term> NSImageCompressionMethod </term>
    <desc> NSNumber; automatically set when reading TIFF data; writing TIFF data </desc>
    <term> NSImageCompressionFactor </term>
    <desc> NSNumber 0.0 to 1.0; writing JPEG data 
    (GNUstep extension: JPEG-compressed TIFFs too) </desc>
    <term> NSImageProgressive </term>
    <desc> NSNumber boolean; automatically set when reading JPEG data; writing JPEG data.
    Note: progressive display is not supported in GNUstep at this time. </desc>
    <term> NSImageInterlaced </term>
    <desc> NSNumber boolean; only for writing PNG data </desc>
    <term> NSImageGamma </term>
    <desc> NSNumber 0.0 to 1.0; only for reading or writing PNG data </desc>
    <term> NSImageRGBColorTable </term>
    <desc> NSData; automatically set when reading GIF data; writing GIF data </desc>
    <term> NSImageFrameCount </term>
    <desc> NSNumber integer; automatically set when reading animated GIF data.
    Not currently implemented. </desc>
    <term> NSImageCurrentFrame </term>
    <desc> NSNumber integer; only for animated GIF files. Not currently implemented. </desc>
    <term> NSImageCurrentFrameDuration </term>
    <desc> NSNumber float; automatically set when reading animated GIF data </desc>
    <term> NSImageLoopCount </term>
    <desc> NSNumber integer; automatically set when reading animated GIF data </desc>
    <term> NSImageDitherTranparency </term>
    <desc> NSNumber boolean; only for writing GIF data. Not currently supported. </desc>
  </deflist>
*/
- (void)setProperty:(NSString *)property withValue:(id)value
{
  if (value)
  {
    [_properties setObject: value forKey: property];
  }
  else  // clear the property
  {
    [_properties removeObjectForKey: property];
  }
}

/** Returns the value of a property */
- (id)valueForProperty:(NSString *)property
{
  return [_properties objectForKey: property];
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone
{
  NSBitmapImageRep	*copy;

  copy = (NSBitmapImageRep*)[super copyWithZone: zone];

  copy->_properties = [_properties mutableCopyWithZone: zone];
  copy->_imageData = [_imageData mutableCopyWithZone: zone];
  copy->_imagePlanes = NSZoneMalloc(zone, sizeof(unsigned char*) * MAX_PLANES);
  if (_imageData == nil)
    {
      memcpy(copy->_imagePlanes, _imagePlanes, sizeof(unsigned char*) * MAX_PLANES);
    }
  else
    {
      unsigned char *bits;
      unsigned int i;

      bits = (unsigned char *)[copy->_imageData bytes];
      copy->_imagePlanes[0] = bits;
      if (_isPlanar) 
	{
	  for (i = 1; i < _numColors; i++) 
	    copy->_imagePlanes[i] = bits + i * _bytesPerRow * _pixelsHigh;
	  for (i = _numColors; i < MAX_PLANES; i++) 
	    copy->_imagePlanes[i] = NULL;
	}
      else
	{
	  for (i = 1; i < MAX_PLANES; i++) 
	    copy->_imagePlanes[i] = NULL;
	}
    }

  return copy;
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  NSData *data = [self TIFFRepresentation];

  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: data forKey: @"NSTIFFRepresentation"];
    }
  else
    {
      [aCoder encodeObject: data];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  NSData	*data;

  self = [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      data = [aDecoder decodeObjectForKey: @"NSTIFFRepresentation"];	
    }
  else
    {
      data = [aDecoder decodeObject];
    }
  return [self initWithData: data];
}

@end

@implementation NSBitmapImageRep (GSPrivate)

+ (int) _localFromCompressionType: (NSTIFFCompression)type
{
  switch (type)
    {
    case NSTIFFCompressionNone: return COMPRESSION_NONE;
    case NSTIFFCompressionCCITTFAX3: return COMPRESSION_CCITTFAX3;
    case NSTIFFCompressionCCITTFAX4: return COMPRESSION_CCITTFAX4;
    case NSTIFFCompressionLZW: return COMPRESSION_LZW;
    case NSTIFFCompressionJPEG: return COMPRESSION_JPEG;
    case NSTIFFCompressionNEXT: return COMPRESSION_NEXT;
    case NSTIFFCompressionPackBits: return COMPRESSION_PACKBITS;
    case NSTIFFCompressionOldJPEG: return COMPRESSION_OJPEG;
    default:
      break;
    }
  return COMPRESSION_NONE;
}

+ (NSTIFFCompression) _compressionTypeFromLocal: (int)type
{
  switch (type)
    {
    case COMPRESSION_NONE: return NSTIFFCompressionNone;
    case COMPRESSION_CCITTFAX3: return NSTIFFCompressionCCITTFAX3;
    case COMPRESSION_CCITTFAX4: return NSTIFFCompressionCCITTFAX4;
    case COMPRESSION_LZW: return NSTIFFCompressionLZW;
    case COMPRESSION_JPEG: return NSTIFFCompressionJPEG;
    case COMPRESSION_NEXT: return NSTIFFCompressionNEXT;
    case COMPRESSION_PACKBITS: return NSTIFFCompressionPackBits;
    case COMPRESSION_OJPEG: return NSTIFFCompressionOldJPEG;
    default:
      break;
   }
  return NSTIFFCompressionNone;
}

+ (BOOL) _bitmapIsTIFF: (NSData *)data
{
  TIFF *image = NSTiffOpenDataRead((char *)[data bytes], [data length]);

  if (image != NULL)
    {
      NSTiffClose(image);
      return YES;
    }
  else
    {
      return NO;
    }
}

+ (NSArray*) _imageRepsWithTIFFData: (NSData *)imageData
{
  int		 i, images;
  TIFF		 *image;
  NSMutableArray *array;

  image = NSTiffOpenDataRead((char *)[imageData bytes], [imageData length]);
  if (image == NULL)
    {
      NSLog(@"NSBitmapImageRep: unable to parse TIFF data");
      return [NSArray array];
    }

  images = NSTiffGetImageCount(image);
  NSDebugLLog(@"NSImage", @"Image contains %d directories", images);
  array = [NSMutableArray arrayWithCapacity: images];
  for (i = 0; i < images; i++)
    {
      NSBitmapImageRep* imageRep;
      imageRep = [[self alloc] _initFromTIFFImage: image number: i];
      if (imageRep)
	{
	  [array addObject: imageRep];
          RELEASE(imageRep);
	}
    }
  NSTiffClose(image);

  return array;
}

- (NSBitmapImageRep *) _initBitmapFromTIFF: (NSData *)imageData
{
  TIFF *image = NSTiffOpenDataRead((char *)[imageData bytes], [imageData length]);

  if (image == NULL)
    {
      RELEASE(self);
      NSLog(@"Tiff read invalid TIFF info from data");
      return nil;
    }

  [self _initFromTIFFImage: image number: -1];
  NSTiffClose(image);
  return self;
}

/* Given a TIFF image (from the libtiff library), load the image information
   into our data structure.  Reads the specified image. */
- (NSBitmapImageRep *) _initFromTIFFImage: (TIFF *)image number: (int)imageNumber
{
  NSString* space;
  NSTiffInfo* info;

  /* Seek to the correct image and get the dictionary information */
  info = NSTiffGetInfo(imageNumber, image);
  if (!info) 
    {
      RELEASE(self);
      NSLog(@"Tiff read invalid TIFF info in directory %d", imageNumber);
      return nil;
    }

  /* 8-bit RGB will be converted to 24-bit by the tiff routines, so account
     for this. */
  space = nil;
  switch(info->photoInterp) 
    {
    case PHOTOMETRIC_MINISBLACK: space = NSDeviceWhiteColorSpace; break;
    case PHOTOMETRIC_MINISWHITE: space = NSDeviceBlackColorSpace; break;
    case PHOTOMETRIC_RGB: space = NSDeviceRGBColorSpace; break;
    case PHOTOMETRIC_PALETTE: 
      space = NSDeviceRGBColorSpace; 
      info->samplesPerPixel = 3;
      break;
    default:
      break;
    }

  [self initWithBitmapDataPlanes: NULL
        pixelsWide: info->width
        pixelsHigh: info->height
        bitsPerSample: info->bitsPerSample
        samplesPerPixel: info->samplesPerPixel
        hasAlpha: (info->extraSamples > 0)
        isPlanar: (info->planarConfig == PLANARCONFIG_SEPARATE)
        colorSpaceName: space
        bitmapFormat: (info->assocAlpha ? 0 : 
                       NSAlphaNonpremultipliedBitmapFormat)
        bytesPerRow: 0
        bitsPerPixel: 0];
  _compression = [NSBitmapImageRep _compressionTypeFromLocal: info->compression];
  _comp_factor = (((float)info->quality)/100.0);

  // Note that Cocoa does not do this, even though the docs say it should
  [_properties setObject: [NSNumber numberWithUnsignedShort: _compression]
                  forKey: NSImageCompressionMethod];
  [_properties setObject: [NSNumber numberWithFloat: _comp_factor]
                  forKey: NSImageCompressionFactor];

  if (info->xdpi > 0 && info->xdpi != 72 &&
      info->ydpi > 0 && info->ydpi != 72)
    {
      NSSize pointSize = NSMakeSize((double)info->width * (72.0 / (double)info->xdpi),
				    (double)info->height * (72.0 / (double)info->ydpi));
      [self setSize: pointSize];
    }

  if (NSTiffRead(image, info, [self bitmapData]))
    {
      free(info);
      RELEASE(self);
      NSLog(@"Tiff read invalid TIFF image data in directory %d", imageNumber);
      return nil;
    }
  free(info);

  return self;
}

- (void) _fillTIFFInfo: (NSTiffInfo*)info
      usingCompression: (NSTIFFCompression)type
                factor: (float)factor
{
  info->numImages = 1;
  info->imageNumber = 0;
  info->subfileType = 0;
  info->width = _pixelsWide;
  info->height = _pixelsHigh;
  info->bitsPerSample = _bitsPerSample;
  info->samplesPerPixel = _numColors;

  // resolution/density
  info->xdpi = 0;
  info->ydpi = 0;
  if (_pixelsWide != (int)(_size.width) || _pixelsHigh != (int)(_size.height))
    {
      float x_density, y_density;
      x_density = _pixelsWide * 72 / _size.width;
      y_density = _pixelsHigh * 72 / _size.height;
      info->xdpi = x_density;
      info->ydpi = y_density;
    }

  if (_isPlanar)
    info->planarConfig = PLANARCONFIG_SEPARATE;
  else
    info->planarConfig = PLANARCONFIG_CONTIG;

  if ([_colorSpace isEqual: NSDeviceRGBColorSpace]
      || [_colorSpace isEqual: NSCalibratedRGBColorSpace])
    info->photoInterp = PHOTOMETRIC_RGB;
  else if ([_colorSpace isEqual: NSDeviceWhiteColorSpace]
	   || [_colorSpace isEqual: NSCalibratedWhiteColorSpace])
    info->photoInterp = PHOTOMETRIC_MINISBLACK;
  else if ([_colorSpace isEqual: NSDeviceBlackColorSpace]
	   || [_colorSpace isEqual: NSCalibratedBlackColorSpace])
    info->photoInterp = PHOTOMETRIC_MINISWHITE;
  else
    {
      NSWarnMLog(@"Unknown colorspace %@.", _colorSpace);
      info->photoInterp = PHOTOMETRIC_RGB;
    }

  info->extraSamples = (_hasAlpha) ? 1 : 0;
  info->assocAlpha = (_format & NSAlphaNonpremultipliedBitmapFormat) ? 0 : 1;

  if ([self canBeCompressedUsing: type] == NO)
    {
      type = NSTIFFCompressionNone;
      factor = 0;
    }

  info->compression = [NSBitmapImageRep _localFromCompressionType: type];
  if (factor < 0)
    factor = 0;
  if (factor > 1)
    factor = 1;
  info->quality = factor * 100;
  info->error = 0;
}

- (void) _premultiply
{
  NSInteger x, y;
  NSUInteger pixelData[5];
  NSInteger start, end, i, ai;
  SEL getPSel = @selector(getPixel:atX:y:);
  SEL setPSel = @selector(setPixel:atX:y:);
  IMP getP = [self methodForSelector: getPSel];
  IMP setP = [self methodForSelector: setPSel];

  if (!_hasAlpha || !(_format & NSAlphaNonpremultipliedBitmapFormat))
    return;

  if (_format & NSAlphaFirstBitmapFormat)
    {
      ai = 0;
      start = 1;
      end = _numColors;
    }
  else
    {
      ai = _numColors - 1;
      start = 0;
      end = _numColors - 1;
    }

  if (_bitsPerSample == 8)
    {
      if (!_isPlanar)
        {
          // Optimize for the most common case
          NSUInteger a;
          NSInteger offset;
          NSInteger line_offset;

          for (y = 0; y < _pixelsHigh; y++)
            {
              line_offset = _bytesPerRow * y;
              for (x = 0; x < _pixelsWide; x++)
                {
                  offset = (_bitsPerPixel * x) / 8 + line_offset;
                  a = _imagePlanes[0][offset + ai];
                  if (a != 255)
                    {
                      if (a == 0)
                        {
                          for (i = start; i < end; i++)
                            {
                              _imagePlanes[0][offset + i] = 0;
                            }
                        }
                      else
                        {
                          for (i = start; i < end; i++)
                            {
                              NSUInteger v = _imagePlanes[0][offset + i];
                              NSUInteger t = a * v + 0x80;
                              
                              v = ((t >> 8) + t) >> 8;
                              _imagePlanes[0][offset + i] = v;
                            }
                        }
                    }
                }
            }
        }
      else
        {
          NSUInteger a;

          for (y = 0; y < _pixelsHigh; y++)
            {
              for (x = 0; x < _pixelsWide; x++)
                {
                  //[self getPixel: pixelData atX: x y: y];
                  getP(self, getPSel, pixelData, x, y);
                  a = pixelData[ai];
                  if (a != 255)
                    {
                      for (i = start; i < end; i++)
                        {
                          NSUInteger t = a * pixelData[i] + 0x80;
                          
                          pixelData[i] = ((t >> 8) + t) >> 8;
                        }
                      //[self setPixel: pixelData atX: x y: y];
                      setP(self, setPSel, pixelData, x, y);
                    }
                }
            }
        }
    }
  else
    {
      CGFloat scale;
      CGFloat alpha;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      for (y = 0; y < _pixelsHigh; y++)
        {
          for (x = 0; x < _pixelsWide; x++)
            {
              //[self getPixel: pixelData atX: x y: y];
              getP(self, getPSel, pixelData, x, y);
              alpha = pixelData[ai] / scale;
              for (i = start; i < end; i++)
                {
                  pixelData[i] *= alpha;
                }
              //[self setPixel: pixelData atX: x y: y];
              setP(self, setPSel, pixelData, x, y);
            }
        }
    }

  _format &= ~NSAlphaNonpremultipliedBitmapFormat;
}

- (void) _unpremultiply
{
  NSInteger x, y;
  NSUInteger pixelData[5];
  NSInteger start, end, i, ai;
  SEL getPSel = @selector(getPixel:atX:y:);
  SEL setPSel = @selector(setPixel:atX:y:);
  IMP getP = [self methodForSelector: getPSel];
  IMP setP = [self methodForSelector: setPSel];

  if (!_hasAlpha || (_format & NSAlphaNonpremultipliedBitmapFormat))
    return;

  if (_format & NSAlphaFirstBitmapFormat)
    {
      ai = 0;
      start = 1;
      end = _numColors;
    }
  else
    {
      ai = _numColors - 1;
      start = 0;
      end = _numColors - 1;
    }

  if (_bitsPerSample == 8)
    {
      if (!_isPlanar)
        {
          // Optimize for the most common case
          NSUInteger a;
          NSInteger offset;
          NSInteger line_offset;

          for (y = 0; y < _pixelsHigh; y++)
            {
              line_offset = _bytesPerRow * y;
              for (x = 0; x < _pixelsWide; x++)
                {
                  offset = (_bitsPerPixel * x) / 8 + line_offset;
                  a = _imagePlanes[0][offset + ai];
                  if ((a != 0) && (a != 255))
                    {
                      for (i = start; i < end; i++)
                        {
                          NSUInteger v = _imagePlanes[0][offset + i];
                          NSUInteger c;
                          
                          c = (v * 255) / a;
                          if (c >= 255)
                            {
                              v = 255;
                            }
                          else
                            {
                              v = c;
                            }
                          
                          _imagePlanes[0][offset + i] = v;
                        }
                    }
                }
            }
        }
      else
        {
          NSUInteger a;
          
          for (y = 0; y < _pixelsHigh; y++)
            {
              for (x = 0; x < _pixelsWide; x++)
                {
                  //[self getPixel: pixelData atX: x y: y];
                  getP(self, getPSel, pixelData, x, y);
                  a = pixelData[ai];
                  if ((a != 0) && (a != 255))
                    {
                      for (i = start; i < end; i++)
                        {
                          NSUInteger c;
                          
                          c = (pixelData[i] * 255) / a;
                          if (c >= 255)
                            {
                              pixelData[i] = 255;
                            }
                          else
                            {
                              pixelData[i] = c;
                            }
                        }
                      //[self setPixel: pixelData atX: x y: y];
                      setP(self, setPSel, pixelData, x, y);
                    }
                }
            }
        }
    }
  else
    {
      CGFloat scale;
      CGFloat alpha;

      scale = (CGFloat)((1 << _bitsPerSample) - 1);
      for (y = 0; y < _pixelsHigh; y++)
        {
          NSUInteger a;

          for (x = 0; x < _pixelsWide; x++)
            {
              //[self getPixel: pixelData atX: x y: y];
              getP(self, getPSel, pixelData, x, y);
              a = pixelData[ai];
              if (a != 0)
                {
                    alpha = scale / a;
                    for (i = start; i < end; i++)
                      {
                        CGFloat new = pixelData[i] * alpha;
                        
                        if (new > scale)
                          {
                            pixelData[i] = scale;
                          }
                        else
                          {
                            pixelData[i] = new;
                          }
                      }
                    //[self setPixel: pixelData atX: x y: y];
                    setP(self, setPSel, pixelData, x, y);
                }
            }
        }
    }

  _format |= NSAlphaNonpremultipliedBitmapFormat;
}

- (NSBitmapImageRep *) _convertToFormatBitsPerSample: (NSInteger)bps
                                     samplesPerPixel: (NSInteger)spp
                                            hasAlpha: (BOOL)alpha
                                            isPlanar: (BOOL)isPlanar
                                      colorSpaceName: (NSString*)colorSpaceName
                                        bitmapFormat: (NSBitmapFormat)bitmapFormat 
                                         bytesPerRow: (NSInteger)rowBytes
                                        bitsPerPixel: (NSInteger)pixelBits
{
  if (!pixelBits)
    pixelBits = bps * ((isPlanar) ? 1 : spp);
  if (!rowBytes) 
    rowBytes = ceil((float)_pixelsWide * pixelBits / 8);

  // Do we already have the correct format?
  if ((bps == _bitsPerSample) && (spp == _numColors)
      && (alpha == _hasAlpha) && (isPlanar == _isPlanar)
      && (bitmapFormat == _format) && (rowBytes == _bytesPerRow) 
      && (pixelBits == _bitsPerPixel)
      && [_colorSpace isEqualToString: colorSpaceName])
    {
      return self;
    }
  else
    {
      NSBitmapImageRep* new;
      
      new = [[NSBitmapImageRep alloc]
                initWithBitmapDataPlanes: NULL
                pixelsWide: _pixelsWide
                pixelsHigh: _pixelsHigh
                bitsPerSample: bps
                samplesPerPixel: spp
                hasAlpha: alpha
                isPlanar: isPlanar
                colorSpaceName: colorSpaceName
                bitmapFormat: bitmapFormat
                bytesPerRow: rowBytes
                bitsPerPixel: pixelBits];

      if ([_colorSpace isEqualToString: colorSpaceName] ||
          ([_colorSpace isEqualToString: NSDeviceRGBColorSpace] &&
           [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace]) ||
          ([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] &&
           [_colorSpace isEqualToString: NSCalibratedRGBColorSpace]))
        {
          SEL getPSel = @selector(getPixel:atX:y:);
          SEL setPSel = @selector(setPixel:atX:y:);
          IMP getP = [self methodForSelector: getPSel];
          IMP setP = [new methodForSelector: setPSel];
          NSUInteger pixelData[5];
          NSInteger x, y;
          CGFloat _scale;
          CGFloat scale;

          NSDebugLLog(@"NSImage", @"Converting %@ bitmap data", _colorSpace);

          if (_bitsPerSample != bps)
            {
              _scale = (CGFloat)((1 << _bitsPerSample) - 1);
              scale = (CGFloat)((1 << bps) - 1);
            }
          else
            {
              _scale = 1.0;
              scale = 1.0;
            }

          for (y = 0; y < _pixelsHigh; y++)
            {
              for (x = 0; x < _pixelsWide; x++)
                {
                  NSUInteger iv[4], ia;
                  CGFloat fv[4], fa;
                  NSInteger i;

                 //[self getPixel: pixelData atX: x y: y];
                  getP(self, getPSel, pixelData, x, y);

                  if (_hasAlpha)
                    {
                      // This order depends on the bitmap format
                      if (_format & NSAlphaFirstBitmapFormat)
                        {
                          ia = pixelData[0];
                          for (i = 0; i < _numColors - 1; i++)
                            {
                              iv[i] = pixelData[i + 1];
                            }
                        }
                      else
                        {
                          for (i = 0; i < _numColors - 1; i++)
                            {
                              iv[i] = pixelData[i];
                            }
                          ia = pixelData[_numColors - 1];
                        }

                      // Scale to [0.0 ... 1.0]
                      for (i = 0; i < _numColors - 1; i++)
                        {
                          fv[i] = iv[i] / _scale;
                        }
                      fa = ia / _scale;

                      if ((ia != 0 && fa < 1.0) &&
                          (_format & NSAlphaNonpremultipliedBitmapFormat) !=
                          (bitmapFormat & NSAlphaNonpremultipliedBitmapFormat))
                        {
                          if (_format & NSAlphaNonpremultipliedBitmapFormat)
                            {
                              for (i = 0; i < _numColors - 1; i++)
                                {
                                  fv[i] = fv[i] * fa;
                                }
                            }
                          else
                            {
                              for (i = 0; i < _numColors - 1; i++)
                                {
                                  fv[i] = fv[i] / fa;
                                }
                            }
                        }
                    }
                  else 
                    {
                      for (i = 0; i < _numColors; i++)
                        {
                          iv[i] = pixelData[i];
                        }
                      // Scale to [0.0 ... 1.0]
                      for (i = 0; i < _numColors; i++)
                        {
                          fv[i] = iv[i] / _scale;
                        }
                      fa = 1.0;
                    }
                  
                  if (alpha)
                    {
                      // Scale from [0.0 ... 1.0]
                      for (i = 0; i < spp - 1; i++)
                        {
                          iv[i] = fv[i] * scale;
                        }
                      ia = fa * scale;

                      if (bitmapFormat & NSAlphaFirstBitmapFormat)
                        {
                          pixelData[0] = ia;
                          for (i = 0; i < spp - 1; i++)
                            {
                              pixelData[i + 1] = iv[i];
                            }
                        }
                      else
                        {
                          for (i = 0; i < spp - 1; i++)
                            {
                              pixelData[i] = iv[i];
                            }
                          pixelData[spp -1] = ia;
                        }
                    }
                  else
                    {
                      // Scale from [0.0 ... 1.0]
                      for (i = 0; i < spp; i++)
                        {
                          pixelData[i] = fv[i] * scale;
                        }
                    }

                  //[new setPixel: pixelData atX: x y: y];
                  setP(new, setPSel, pixelData, x, y);
                }
            }
        }
      else if (([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] ||
               [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace])
          && ([_colorSpace isEqualToString: NSCalibratedWhiteColorSpace] ||
              [_colorSpace isEqualToString: NSCalibratedBlackColorSpace] ||
              [_colorSpace isEqualToString: NSDeviceWhiteColorSpace] ||
              [_colorSpace isEqualToString: NSDeviceBlackColorSpace]))
        {
          SEL getPSel = @selector(getPixel:atX:y:);
          SEL setPSel = @selector(setPixel:atX:y:);
          IMP getP = [self methodForSelector: getPSel];
          IMP setP = [new methodForSelector: setPSel];
          NSUInteger pixelData[4];
          NSInteger x, y;
          CGFloat _scale;
          CGFloat scale;
          NSInteger max = (1 << _bitsPerSample) - 1;
          BOOL isWhite = [_colorSpace isEqualToString: NSCalibratedWhiteColorSpace] 
              || [_colorSpace isEqualToString: NSDeviceWhiteColorSpace];

          NSDebugLLog(@"NSImage", @"Converting black/white bitmap data");

          if (_bitsPerSample != bps)
            {
              _scale = (CGFloat)((1 << _bitsPerSample) - 1);
              scale = (CGFloat)((1 << bps) - 1);
            }
          else
            {
              _scale = 1.0;
              scale = 1.0;
            }

          for (y = 0; y < _pixelsHigh; y++)
            {
              for (x = 0; x < _pixelsWide; x++)
                {
                  NSUInteger iv, ia;
                  CGFloat fv, fa;

                 //[self getPixel: pixelData atX: x y: y];
                  getP(self, getPSel, pixelData, x, y);

                  if (_hasAlpha)
                    {
                      // This order depends on the bitmap format
                      if (_format & NSAlphaFirstBitmapFormat)
                        {
                          ia = pixelData[0];
                          if (isWhite)
                            iv = pixelData[1];
                          else
                            iv = max - pixelData[1];
                        }
                      else
                        {
                          if (isWhite)
                            iv = pixelData[0];
                          else
                            iv = max - pixelData[0];
                          ia = pixelData[1];
                        }

                      // Scale to [0.0 ... 1.0]
                      fv = iv / _scale;
                      fa = ia / _scale;

                      if ((ia != 0 && fa < 1.0) &&
                          (_format & NSAlphaNonpremultipliedBitmapFormat) !=
                          (bitmapFormat & NSAlphaNonpremultipliedBitmapFormat))
                        {
                          if (_format & NSAlphaNonpremultipliedBitmapFormat)
                            {
                              fv = fv * fa;
                            }
                          else
                            {
                              fv = fv / fa;
                            }
                        }
                    }
                  else 
                    {
                      if (isWhite)
                        iv = pixelData[0];
                      else
                        iv = max - pixelData[0];
                      // Scale to [0.0 ... 1.0]
                      fv = iv / _scale;
                      fa = 1.0;
                    }
                  
                  if (alpha)
                    {
                      // Scale from [0.0 ... 1.0]
                      iv = fv * scale;
                      ia = fa * scale;

                      if (bitmapFormat & NSAlphaFirstBitmapFormat)
                        {
                          pixelData[0] = ia;
                          pixelData[1] = iv;
                          pixelData[2] = iv;
                          pixelData[3] = iv;
                        }
                      else
                        {
                          pixelData[0] = iv;
                          pixelData[1] = iv;
                          pixelData[2] = iv;
                          pixelData[3] = ia;
                        }
                    }
                  else
                    {
                      // Scale from [0.0 ... 1.0]
                      iv = fv * scale;
                      pixelData[0] = iv;
                      pixelData[1] = iv;
                      pixelData[2] = iv;
                    }

                  //[new setPixel: pixelData atX: x y: y];
                  setP(new, setPSel, pixelData, x, y);
                }
            }
        }
      else
        {
          SEL getCSel = @selector(colorAtX:y:);
          SEL setCSel = @selector(setColor:atX:y:);
          IMP getC = [self methodForSelector: getCSel];
          IMP setC = [new methodForSelector: setCSel];
          NSInteger i, j;

          NSDebugLLog(@"NSImage", @"Slow converting %@ bitmap data to %@", 
                      _colorSpace, colorSpaceName);
          for (j = 0; j < _pixelsHigh; j++)
            {
              CREATE_AUTORELEASE_POOL(pool);
              
              for (i = 0; i < _pixelsWide; i++)
                {
                  NSColor *c;
                  
                  //c = [self colorAtX: i y: j];
                  c = getC(self, getCSel, i, j);
                  //[new setColor: c atX: i y: j];
                  setC(new, setCSel, c, i, j);
                }
              [pool drain];
            }
        }

      return AUTORELEASE(new);
    }  
}

@end
