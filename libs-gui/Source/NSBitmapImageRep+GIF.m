/* NSBitmapImageRep+GIF.m

   Methods for reading GIF images

   Copyright (C) 2003-2014 Free Software Foundation, Inc.
   
   Written by:  Stefan Kleine Stegemann <stefan@wms-network.de>
   Date: Nov 2003

   GIF writing, properties and transparency: Mark Tracy <tracy454@concentric.net>
   Date: Nov 2006

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
#import <Foundation/NSData.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSGraphics.h"
#import "NSBitmapImageRep+GIF.h"
#import "GSGuiPrivate.h"

#if HAVE_LIBUNGIF || HAVE_LIBGIF


/*
gif_lib.h (4.1.0b1, possibly other versions) uses Object as the name of an
argument to a function. This causes a conflict with Object declared by the
objective-c headers.
*/
#define Object GS_GifLib_Object
#include <gif_lib.h>
#undef Object

// GIF 5.0 no longer has this define
#ifndef FALSE
#define FALSE       0
#endif /* FALSE */

// GIF > 5.0
#if GIFLIB_MAJOR >= 5
  #define DGifOpen(s, i) DGifOpen(s, i, NULL)
  #define EGifOpen(s, i) EGifOpen(s, i, NULL)
#endif

// GIF> 5.1
#if GIFLIB_MAJOR >= 5 && GIFLIB_MINOR >= 1
  #define DGifCloseFile(f) DGifCloseFile(f, NULL)
  #define EGifCloseFile(f) EGifCloseFile(f, NULL)
#endif

/* -----------------------------------------------------------
   The following types and functions are for interacting with
   the gif library.
   ----------------------------------------------------------- */

/* settings for reading interlaced images */
static int InterlaceOffset[] = { 0, 4, 2, 1 };
static int InterlaceJumps[]  = { 8, 8, 4, 2 };

/* Holds the information for the input function.  */
typedef struct gs_gif_input_src
{
  const void *data;
  unsigned    length;
  unsigned    pos;
} gs_gif_input_src;

/* Provides data for the gif library.  */
static int gs_gif_input(GifFileType *file, GifByteType *buffer, int len)
{
  /* according the the libungif sources, this functions has
     to act like fread. */
  int bytesRead;
  gs_gif_input_src *src = (gs_gif_input_src *)file->UserData;

  if (src->pos < src->length)
    {
      if ((src->pos + len) > src->length)
	{
	  bytesRead = (src->length - src->pos);
	}
      else
	{
	  bytesRead = len;
	}

      /* We have to copy the data here, looking at
         the libungif source makes this clear.  */
      memcpy(buffer, src->data + src->pos, bytesRead);
      src->pos = src->pos + bytesRead;
    }
  else
    {
      bytesRead = 0;
    }

  return bytesRead;
}


/* Initialze a new input source to be used with
   gs_gif_input. The passed structure has to be
   allocated outside this function. */
static void gs_gif_init_input_source(gs_gif_input_src *src, NSData *data)
{
  src->data   = [data bytes];
  src->length = [data length];
  src->pos    = 0;
}

#if HAVE_QUANTIZEBUFFER || HAVE_GIFQUANTIZEBUFFER
/* Function to write GIF to buffer */
static int gs_gif_output(GifFileType *file, const GifByteType *buffer, int len)
{
  NSMutableData *nsData;
  
  if (len <= 0) return 0;

  nsData = file->UserData;
  [nsData appendBytes: buffer length: len];
  return len;
}
#endif

/* -----------------------------------------------------------
   The gif loading part of NSBitmapImageRep
   ----------------------------------------------------------- */

@implementation NSBitmapImageRep (GIFReading)

/* Return YES if this looks like a GIF. */
+ (BOOL) _bitmapIsGIF: (NSData *)imageData
{
  struct gs_gif_input_src src;
  GifFileType*            file;

  if (!imageData || ![imageData length])
    {
      return NO;
    }

  gs_gif_init_input_source(&src, imageData);
  file = DGifOpen(&src, gs_gif_input);
  if (file == NULL)
    {
      /* we do not use giferror here because it doesn't
         seem to be thread-safe (the error code is a global
         variable, so we might get the wrong error here.  */
      return NO;
    }

  DGifCloseFile(file);
  return YES;
}


#define SET_ERROR_MSG(msg) \
   if (errorMsg != NULL) \
     {\
       *errorMsg = msg; \
     }\
   else \
     {\
       NSLog(@"%@", msg);\
     }

#define GIF_CREATE_ERROR(msg) \
   SET_ERROR_MSG(msg); \
   if (file != NULL) \
     {\
       DGifCloseFile(file); \
     }\
   if (imgBuffer != NULL) \
     {\
       NSZoneFree([self zone], imgBuffer); \
     }\
   RELEASE(self); \
   return nil;

#define CALL_CHECKED(f, where) \
   gifrc = f; \
   if (gifrc != GIF_OK) \
     {\
       NSString* msg = [NSString stringWithFormat: @"reading gif failed (%@)", \
						   where]; \
       GIF_CREATE_ERROR(msg);\
     }

/* Read a gif image. Assume it is from a gif file. */
- (id) _initBitmapFromGIF: (NSData *)imageData
	     errorMessage: (NSString **)errorMsg
{
  struct gs_gif_input_src src;
  GifFileType            *file = NULL;
  GifRecordType           recordType;
  GifByteType            *extension;
  GifPixelType           *imgBuffer = NULL;
  GifPixelType           *imgBufferPos;  /* a position inside imgBuffer */
  unsigned char          *rgbBuffer; /* image converted to rgb */
  unsigned                rgbBufferPos;
  unsigned                rgbBufferSize;
  ColorMapObject         *colorMap;
  GifColorType           *color;
  unsigned char           colorIndex;
  unsigned                pixelSize, rowSize;
  int                     extCode;
  int                     gifrc; /* required by CALL_CHECKED */
  int                     i, j;  /* counters */
  int                     imgHeight = 0, imgWidth = 0, imgRow = 0, imgCol = 0;
  BOOL                    hasAlpha = NO;
  unsigned char           transparentColor = 0;
  int                     sPP = 3;	/* samples per pixel */
  unsigned short          duration = 0;

  /* open the image */
  gs_gif_init_input_source(&src, imageData);
  file = DGifOpen(&src, gs_gif_input);
  if (file == NULL)
    {
      /* we do not use giferror here because it doesn't
         seem to be thread-safe (the error code is a global
         variable, so we might get the wrong error here.  */
      GIF_CREATE_ERROR(@"unable to open gif from data");
      /* Not reached. */
    }


  /* allocate a buffer for the decoded image */
  pixelSize = sizeof(GifPixelType);
  rowSize   = file->SWidth * pixelSize;
  imgBuffer = NSZoneMalloc([self zone], file->SHeight * rowSize);
  if (imgBuffer == NULL)
    {
      GIF_CREATE_ERROR(@"could not allocate input buffer");
      /* Not reached. */
    }


  /* set the background color */
  memset(imgBuffer, file->SBackGroundColor, file->SHeight * rowSize);


  /* read the image 
   * this delivers the first image in a multi-image gif
   */
  do
    {
      CALL_CHECKED(DGifGetRecordType(file, &recordType), @"GetRecordType");
      switch (recordType)
	{
	  case IMAGE_DESC_RECORD_TYPE:
	    {
	      CALL_CHECKED(DGifGetImageDesc(file), @"GetImageDesc");
	     
	      imgWidth  = file->Image.Width;
	      imgHeight = file->Image.Height;
	      imgRow    = file->Image.Top;
	      imgCol    = file->Image.Left;

	      if ((file->Image.Left + file->Image.Width > file->SWidth)
		  || (file->Image.Top + file->Image.Height > file->SHeight))
		{
		   GIF_CREATE_ERROR(@"image does not fit into screen dimensions");
		}

	      if (file->Image.Interlace)
		{
		  for (i = 0; i < 4; i++)
		    {
		      for (j = imgRow + InterlaceOffset[i]; j < imgRow + imgHeight;
			   j = j + InterlaceJumps[i])
			{
			  imgBufferPos =
			    imgBuffer + (j * rowSize) + (imgCol * pixelSize);
			  CALL_CHECKED(DGifGetLine(file, imgBufferPos, imgWidth),
				       @"GetLine(Interlaced)");
			}      
		    }
		}
	      else
		{
		  for (i = 0; i < imgHeight; i++)
		    {
		      imgBufferPos =
			imgBuffer + ((imgRow++) * rowSize) + (imgCol * pixelSize);
		      CALL_CHECKED(DGifGetLine(file, imgBufferPos, imgWidth),
				   @"GetLine(Non-Interlaced)");
		    }
		}

	      break;
	    }

	  case EXTENSION_RECORD_TYPE:
	    {
	      /* transparency support */
	      CALL_CHECKED(DGifGetExtension(file, &extCode, &extension), @"GetExtension");
              if (extCode == GRAPHICS_EXT_FUNC_CODE)
                {
                   hasAlpha = (extension[1] & 0x01);
                   transparentColor = extension[4];
                   duration = extension[3];
                   duration = (duration << 8) + extension[2];
                }
	      while (extension != NULL)
		{
		  CALL_CHECKED(DGifGetExtensionNext(file, &extension), @"GetExtensionNext");
                }
	      break;
	    }

	  case TERMINATE_RECORD_TYPE:
	  default:
	    {
	      break;
	    }
	}
    } while ((recordType != IMAGE_DESC_RECORD_TYPE)
            && (recordType != TERMINATE_RECORD_TYPE));


  /* convert the image to rgb */
  sPP = hasAlpha? 4 : 3;
  rgbBufferSize = file->SHeight * (file->SWidth * sizeof(unsigned char) * sPP);
  rgbBuffer = NSZoneMalloc([self zone],  rgbBufferSize);
  if (rgbBuffer == NULL)
    {
      GIF_CREATE_ERROR(@"could not allocate image buffer");
      /* Not reached. */
    }

  colorMap = (file->Image.ColorMap ? file->Image.ColorMap : file->SColorMap);
  rgbBufferPos = 0;

  for (i = 0; i < file->SHeight; i++)
    {
      imgBufferPos = imgBuffer + (i * rowSize);
      for (j = 0; j < file->SWidth; j++)
	{
          colorIndex = *(imgBufferPos + j*pixelSize);
	  color = &colorMap->Colors[colorIndex];
	  rgbBuffer[rgbBufferPos++] = color->Red;
	  rgbBuffer[rgbBufferPos++] = color->Green;
	  rgbBuffer[rgbBufferPos++] = color->Blue;
          if (hasAlpha)
            rgbBuffer[rgbBufferPos++] = (transparentColor == colorIndex)? 0 : 255;
	}
    }

  NSZoneFree([self zone], imgBuffer);


  /* initialize self */
  [self initWithBitmapDataPlanes: &rgbBuffer
	pixelsWide: file->SWidth
	pixelsHigh: file->SHeight
	bitsPerSample: 8
	samplesPerPixel: sPP
	hasAlpha: hasAlpha
	isPlanar: NO
	colorSpaceName: NSCalibratedRGBColorSpace
	bytesPerRow: file->SWidth * sPP
	bitsPerPixel: 8 * sPP];

  _imageData = [[NSData alloc] initWithBytesNoCopy: rgbBuffer
			       length: rgbBufferSize];
  [self setProperty: NSImageRGBColorTable
        withValue: [NSData dataWithBytes: colorMap->Colors
                                  length: sizeof(GifColorType)*colorMap->ColorCount]];
  if (duration > 0)
    {
      [self setProperty: NSImageCurrentFrameDuration
              withValue: [NSNumber numberWithFloat: (100.0 * duration)]];
    }
  [self setProperty: NSImageCurrentFrame
          withValue: [NSNumber numberWithInt: 0]];

  /* don't forget to close the gif */
  DGifCloseFile(file);

  return self;
}

- (NSData *) _GIFRepresentationWithProperties: (NSDictionary *) properties
                                 errorMessage: (NSString **)errorMsg
{
#if HAVE_QUANTIZEBUFFER || HAVE_GIFQUANTIZEBUFFER
  NSMutableData         * GIFRep = nil;	// our return value
  GifFileType           * GIFFile = NULL;
  GifByteType           * rgbPlanes = NULL;	// giflib needs planar RGB
  GifByteType           * redPlane = NULL;
  GifByteType           * greenPlane = NULL;
  GifByteType           * bluePlane = NULL;
  int                   width, height;
  GifByteType           * GIFImage = NULL;	// intermediate image storage
  GifByteType           * GIFImageP = NULL;
  int                   h;	// general-purpose loop counter
  ColorMapObject        * GIFColorMap = NULL;
  int                   colorMapSize = 256;
  int                   status;	// return status for giflib calls
  NSString              * colorSpaceName;
  BOOL                  isRGB, hasAlpha;
  unsigned char         * bitmapData = NULL;
  unsigned char         * planes[5];	// MAX_PLANES = 5
  NSData                * colorTable = NULL;	// passed in from properties
  
  NSLog(@"GIF representation is experimental");

  width = [self pixelsWide];
  height = [self pixelsHigh];
  if (!width || !height)
  {
    SET_ERROR_MSG(@"GIFRepresentation: image is zero size");
    return nil;
  }

  // Giflib wants planar RGB so convert as necessary
  colorSpaceName = [self colorSpaceName];
  isRGB = ([colorSpaceName isEqualToString: NSDeviceRGBColorSpace] ||
           [colorSpaceName isEqualToString: NSCalibratedRGBColorSpace]);
  if (!isRGB)
    {  
      SET_ERROR_MSG(@"GIFRepresentation: Only RGB is supported at this time.");
      return nil;
    }
  hasAlpha = [self hasAlpha];
  if ([self isPlanar])
    {
      [self getBitmapDataPlanes: planes];
      redPlane = planes[0];
      greenPlane = planes[1];
      bluePlane = planes[2];
    }
  else	// interleaved RGB or RGBA
    {
      rgbPlanes = malloc(sizeof(GifByteType)*width*height*3);
      if (!rgbPlanes)
	{
	  SET_ERROR_MSG(@"GIFRepresentation: malloc out of memory.");
	  return nil;
	}
      redPlane = rgbPlanes;
      greenPlane = redPlane + width*height;
      bluePlane = greenPlane + width*height;
      bitmapData = [self bitmapData];
      for (h = 0; h < width*height; h++)
	{
	  *redPlane++ = *bitmapData++;
	  *greenPlane++ = *bitmapData++;
	  *bluePlane++ = *bitmapData++;
	  if (hasAlpha) bitmapData++;	// ignore alpha channel
	}
      redPlane = rgbPlanes;
      greenPlane = redPlane + width*height;
      bluePlane = greenPlane + width*height;
    }

  // If you have a color table, you must be certain that it is GIF format
  colorTable = [self valueForProperty: NSImageRGBColorTable];	// nil is OK
  colorMapSize = (colorTable)? [colorTable length]/sizeof(GifColorType) : 256;
#if GIFLIB_MAJOR >= 5
  GIFColorMap = GifMakeMapObject(colorMapSize, [colorTable bytes]);
#else
  GIFColorMap = MakeMapObject(colorMapSize, [colorTable bytes]);
#endif
  if (!GIFColorMap)
    {
      SET_ERROR_MSG(@"GIFRepresentation (giflib): MakeMapObject() failed.");
      free(rgbPlanes);
      return nil;
    }

  GIFImage = malloc(sizeof(GifByteType)*height*width);
  if (!GIFImage)
    {
      SET_ERROR_MSG(@"GIFRepresentation: malloc out of memory.");
      free(rgbPlanes);
    }
#if GIFLIB_MAJOR >= 5
  status = GifQuantizeBuffer(width, height, &colorMapSize,
                             redPlane, greenPlane, bluePlane,
                             GIFImage, GIFColorMap->Colors);
#else
  status = QuantizeBuffer(width, height, &colorMapSize,
		       redPlane, greenPlane, bluePlane,
		       GIFImage, GIFColorMap->Colors);
#endif
  if (status == GIF_ERROR)
    {
      free(GIFImage);
      free(rgbPlanes);
      return nil;
    }

  // QuantizeBuffer returns an optimized colorMapSize,
  // but we must round up to nearest power of 2
  // otherwise MakeColorMap() fails
  for (h = 0; h < 8; h++)
    if ((1<<h) >= colorMapSize) break;
  colorMapSize = 1<<h;
  GIFColorMap->ColorCount = colorMapSize;
  GIFColorMap->BitsPerPixel = h;

  if (![self isPlanar]) free(rgbPlanes);

  // Write the converted image out to the NSData
  GIFRep = [NSMutableData dataWithLength: 0];
  if (!GIFRep) 
    {
      free(GIFImage);
      return nil;
    }

  GIFFile = EGifOpen(GIFRep, gs_gif_output);
  status = EGifPutScreenDesc(GIFFile, width, height, 8, 0, NULL);
  if (status == GIF_ERROR)
    {
      SET_ERROR_MSG(@"GIFRepresentation (giflib): EGifPutScreenDesc() failed.");
      free(GIFImage);
      return nil;
    }

  // note we are not supporting interlaced mode
  status = EGifPutImageDesc(GIFFile, 0, 0, width, height, FALSE, GIFColorMap);
  if (status == GIF_ERROR)
    {
      SET_ERROR_MSG(@"GIFRepresentation (giflib): EGifPutImageDesc() failed.");
      free(GIFImage);
      return nil;
    }

  GIFImageP = GIFImage;
  for (h = 0; h < height ; h++)
    {
      status = EGifPutLine(GIFFile, GIFImageP, width);
      if (status == GIF_ERROR)
	{
	  SET_ERROR_MSG(@"GIFRepresentation (giflib): EGifPutLine() failed.");
	  free(GIFImage);
	  return nil;
	}
      GIFImageP += width;
    }
  status = EGifCloseFile(GIFFile);

  free(GIFImage);

  return GIFRep;
#else
  SET_ERROR_MSG(@"GIFRepresentation: not available on this system");
  return nil;
#endif
}

@end

#else /* !HAVE_LIBUNGIF || !HAVE_LIBGIF */

@implementation NSBitmapImageRep (GIFReading)
+ (BOOL) _bitmapIsGIF: (NSData *)imageData
{
  return NO;
}
- (id) _initBitmapFromGIF: (NSData *)imageData
	     errorMessage: (NSString **)errorMsg
{
  if (errorMsg != NULL)
    {
      *errorMsg = @"gif images not supported on this system";
    }
  RELEASE(self);
  return nil;
}

- (NSData *) _GIFRepresentationWithProperties: (NSDictionary *) properties
                                 errorMessage: (NSString **)errorMsg
{
  if (errorMsg != NULL)
    {
      *errorMsg = @"GIFRepresentation: not supported on this system";
    }
  return nil;
}

@end

#endif /* !HAVE_LIBUNGIF || !HAVE_LIBGIF */

