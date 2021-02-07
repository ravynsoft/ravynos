/* NSBitmapImageRep+PNM

   Methods for reading PNM images

   Copyright (C) 2003 Free Software Foundation, Inc.
   
   Written by:  Adam Fedor <fedor@gnu.org>
   Date: Oct 2003
   
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

#import <Foundation/NSData.h>
#import <Foundation/NSString.h>
#import "AppKit/NSGraphics.h"
#import "NSBitmapImageRep+PNM.h"

@implementation NSBitmapImageRep (PNM)

/* Return YES if this is a pgm or ppm file (raw only for now) */
+ (BOOL) _bitmapIsPNM: (NSData *)imageData
{
  const char *bytes = [imageData bytes];

  /*
  3/4 bytes header line, 4/5 bytes size, 2/3 bytes depth. Thus, the image
  must have a least 9 bytes.
  */
  if ([imageData length]<9)
    return NO;

  if (bytes[0] == 'P' && (bytes[2]=='\n' || bytes[2]=='\r'))
    {
      if (bytes[1] == '5' || bytes[1] == '6')
	return YES;
    }
  return NO;
}

#define GET_LINE()							\
  do									\
    {									\
      char *p = buffer;							\
      while (*ptr != '\n' && *ptr != '\r' && (ptr-bytes) < length)	\
	{								\
	  *p++ = *ptr++;						\
	  if (p == &buffer[sizeof(buffer)])				\
	    {								\
	      ERRMSG(@"PNM header line too long");			\
	    }								\
	}								\
      ptr++;								\
      *p = '\0';							\
    } while (0)
      
#define ERRMSG(msg) \
  do { NSLog(@"Error loading PNM: %@", msg); if (error) *error = msg; RELEASE(self); return nil; } while (0)

/* Read the ppm image. Assume it is a ppm file and imageData is not nil */
-(id) _initBitmapFromPNM: (NSData *)imageData  errorMessage: (NSString **)error
{
  int num, xsize, ysize, levels;
  char ptype;
  char buffer[256];
  unsigned char *pchar;
  unsigned length;
  id colorspace;
  const char *ptr;
  const char *bytes = [imageData bytes];

  /* magic number */
  ptr = bytes;
  length = [imageData length];
  GET_LINE();
  if (bytes[0] != 'P')
    ERRMSG(@"Invalid PNM header (magic number)");
  ptype = bytes[1];
  if (ptype != '5' && ptype != '6')
    ERRMSG(@"Unsupported PNM type");

  do
    {
      GET_LINE();
    } while (buffer[0] == '#');
  num = sscanf(buffer, "%d %d", &xsize, &ysize);
  if (num != 2 || xsize <= 0 || ysize <= 0)
    ERRMSG(@"Invalid PNM header (xsize/ysize)");
  if (xsize * ysize > (1 << 31))
    ERRMSG(@"Invalid PNM header (image size:xsize*ysize too large)");
  GET_LINE();
  num = sscanf(buffer, "%d", &levels);
  if (num != 1)
    ERRMSG(@"Invalid PNM header (levels)");

  colorspace = (ptype == '5') ? NSDeviceBlackColorSpace : NSDeviceRGBColorSpace;
  self = [self initWithBitmapDataPlanes: NULL
	       pixelsWide: xsize
	       pixelsHigh: ysize
	       bitsPerSample: 8
	       samplesPerPixel: (ptype == '5') ? 1 : 3
	       hasAlpha: NO
	       isPlanar: NO
	       colorSpaceName: colorspace
	       bytesPerRow: 0
	       bitsPerPixel: 0];

  if ([self bytesPerRow] * ysize > (length - (ptr - bytes)))
    ERRMSG(@"Invalid PNM file (short data)");
  pchar = [self bitmapData];
  if (levels < 256)
    {
      memcpy(pchar, ptr, [self bytesPerRow] * ysize);
    }
  else
    {
      ERRMSG(@"Cannot handle > 256 level PNM files");
    }
  return self;
}

@end

