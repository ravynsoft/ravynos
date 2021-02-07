/** <title>GSImageMagickImageRep</title>

   <abstract>ImageMagick image representation.</abstract>

   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Author:  Eric Wasylishen <ewasylishen@gmail.com>
   Date: June 2011
   
   This file is part of the GNUstep Application Kit Library.

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

#import <Foundation/NSArray.h>
#import <Foundation/NSAffineTransform.h>
#import <Foundation/NSCoder.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSData.h>
#import <Foundation/NSTask.h>
#import <Foundation/NSProcessInfo.h>
#import "AppKit/NSBitmapImageRep.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSGraphicsContext.h"
#import "GNUstepGUI/GSImageMagickImageRep.h"

#if HAVE_IMAGEMAGICK

#include <magick/MagickCore.h>

@implementation GSImageMagickImageRep 

+ (void) initialize
{
  NSArray *argv = [[NSProcessInfo processInfo] arguments];

  MagickCoreGenesis([[argv objectAtIndex: 0] UTF8String], 0);
}

// Private methods

+ (NSBitmapImageRep *) imageRepWithImageMagickImage: (Image *)image
{
  ExceptionInfo *exception = AcquireExceptionInfo();
  MagickBooleanType success;

  NSBitmapImageRep *bmp = [[[self alloc] 
	  initWithBitmapDataPlanes: NULL
			pixelsWide: image->columns
			pixelsHigh: image->rows
		     bitsPerSample: 8
		   samplesPerPixel: 4
			  hasAlpha: YES
			  isPlanar: NO
		    colorSpaceName: NSDeviceRGBColorSpace
		      bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
		       bytesPerRow: image->columns * 4
		      bitsPerPixel: 32] autorelease];

  // Set the resolution metadata

  if (image->units == PixelsPerInchResolution ||
      image->units == PixelsPerCentimeterResolution)
    {
      NSSize res;
      if (image->units == PixelsPerCentimeterResolution)
	{
	  res = NSMakeSize(image->x_resolution * 2.54, image->y_resolution * 2.54);
	}
      else
	{
	  res = NSMakeSize(image->x_resolution, image->y_resolution);
	}

      if (res.width > 0 && res.height > 0)
	{
	  const NSSize sizeInPoints = NSMakeSize(((CGFloat)image->columns / res.width) * 72.0,
						 ((CGFloat)image->rows / res.height) * 72.0);
	  [bmp setSize: sizeInPoints];
	}
    }

  // Copy the pixel data to the NSBitmapImageRep
  
  success = ExportImagePixels(image, 0, 0, image->columns, image->rows,
			      "RGBA", CharPixel, [bmp bitmapData], exception);      
  if (!success || exception->severity != UndefinedException)
    {
      bmp = nil;
    }

  DestroyExceptionInfo(exception);

  return bmp;
}

+ (NSArray*) imageRepsWithData: (NSData *)data allImages: (BOOL)allImages
{
  NSMutableArray *reps = [NSMutableArray array];

  ExceptionInfo *exception = AcquireExceptionInfo();
  ImageInfo *imageinfo = CloneImageInfo(NULL);
  Image *images, *image;
  
  // Set the background color to transparent
  // (otherwise SVG's are rendered against a white background by default)
  QueryColorDatabase("none", &imageinfo->background_color, exception);

  images = BlobToImage(imageinfo, [data bytes], [data length], exception);

  if (exception->severity != UndefinedException)
    {
      NSWarnLog(@"ImageMagick: %s", GetLocaleExceptionMessage(exception->severity, exception->reason));
    }
  
  for (image = images; image != NULL; image = image->next)
    {
      NSBitmapImageRep *bmp = [[self class] imageRepWithImageMagickImage: image];
      if (bmp != nil)
	{
	  [reps addObject: bmp];
	}
      
      if (!allImages)
	{
	  break;
	}
    }
  
  DestroyExceptionInfo(exception);
  DestroyImageInfo(imageinfo);
  if (images != NULL)
    {
      DestroyImage(images);
    }

  return reps;
}

// NSImageRep overrides

+ (BOOL) canInitWithData: (NSData *)data
{
  char buf[32];
  ExceptionInfo *exception;
  const MagicInfo *info;

  memset(buf, 0, 32);
  [data getBytes: buf length: 32];

  exception = AcquireExceptionInfo();
  info = GetMagicInfo((const unsigned char *)buf, 32, exception);
  DestroyExceptionInfo(exception);

  return (info != NULL);
}

+ (NSArray *) imageUnfilteredFileTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      NSMutableArray *array = [NSMutableArray array];
      size_t size = 0;
      ExceptionInfo *exception = AcquireExceptionInfo();
      const MagickInfo **list = GetMagickInfoList("*", &size, exception);
      size_t i;

      for (i=0; i<size; i++)
	{
	  [array addObject: [[NSString stringWithUTF8String: list[i]->name] lowercaseString]];
	}
      RelinquishMagickMemory(list);
      DestroyExceptionInfo(exception);

      types = [[NSArray alloc] initWithArray: array];
    }
  
  return types;
}

+ (NSArray *) imageUnfilteredPasteboardTypes
{
  static NSArray *types = nil;

  if (types == nil)
    {
      NSMutableArray *array = [NSMutableArray array];
      NSDictionary *fileTypeToPboardType = [NSDictionary dictionaryWithObjectsAndKeys:
							   NSPostScriptPboardType, @"ps",
							 NSPDFPboardType, @"pdf",
							 NSPICTPboardType, @"pict",
							 NSTIFFPboardType, @"tiff",
							 nil];
      
      NSEnumerator *enumerator = [[self imageUnfilteredFileTypes] objectEnumerator];
      NSString *fileType;
      while (nil != (fileType = [enumerator nextObject]))
	{
	  NSString *pboardType = [fileTypeToPboardType objectForKey: fileType];
	  if (pboardType != nil)
	    {
	      [array addObject: pboardType];
	    }
	}
      
      types = [[NSArray alloc] initWithArray: array];
    }
  
  return types;
}

// NSBitmapImageRep overrides

+ (id) imageRepWithData: (NSData *)data
{
  return AUTORELEASE([[self alloc] initWithData: data]);
}

- (id) initWithData: (NSData *)data
{
  NSArray *reps = [[self class] imageRepsWithData: data allImages: NO];

  [self release];

  if ([reps count] != 0)
    {
      return [[reps objectAtIndex: 0] retain];
    }
  else
    {
      return nil;
    }
}

+ (NSArray *) imageRepsWithData: (NSData *)data
{
  return [self imageRepsWithData: data allImages: YES];
}

@end

#endif
