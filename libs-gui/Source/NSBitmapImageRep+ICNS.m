/*
   NSBitmapImageRep+ICNS.m

   Methods for loading .icns images.

   Copyright (C) 2008 Free Software Foundation, Inc.
   
   Written by: Gregory Casamento
   Date: 2008-08-12

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: September 2008
   
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
#import "NSBitmapImageRep+ICNS.h"
#import <Foundation/NSByteOrder.h>
#import <Foundation/NSData.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSGraphics.h"
#import "GSGuiPrivate.h"

#define ICNS_HEADER "icns"

#if HAVE_LIBICNS

#include <icns.h>

#else /* !HAVE_LIBICNS */
/*
  The following code is a drop in replacement for libicns.
  It may be used when the library is not available. 
  This code was mostly build based on the documentation found at 
  http://icns.sourceforge.net/apidocs.html. It also includes icns decoding 
  ideas based on code in mySTEP.
  Only limited formats are implemented and some errors still exist.
*/

typedef unsigned char icns_byte_t;
// must be a 32 bit integer
typedef unsigned int icns_size_t;
typedef struct _icns_type_t {
  char c[4];
} icns_type_t;

typedef struct _icns_element_t {
  icns_type_t elementType;
  icns_size_t elementSize;
} icns_element_t;

typedef struct _icns_icon_info_t {
  unsigned int iconHeight;
  unsigned int iconWidth;
  unsigned int iconDepth;
  unsigned int iconChannels;
} icns_icon_info_t;

typedef struct _icns_image_t {
  unsigned int imageWidth;
  unsigned int imageHeight;
  unsigned int imageChannels;
  unsigned int imagePixelDepth;
  unsigned int imageDataSize;
  icns_byte_t *imageData;
} icns_image_t;

typedef struct _icns_family_t {
  icns_type_t resourceType;
  icns_size_t resourceSize;
  icns_element_t elements[1];
} icns_family_t;

#define ICNS_HEADER_SIZE 8

/*
// ics# 0x69637323
static icns_type_t ICNS_16x16_1BIT_DATA = {{'i', 'c', 's', '#'}};

// ich# 0x69636823
static icns_type_t ICNS_48x48_1BIT_DATA = {{'i', 'c', 'h', '#'}};
*/

// is32 0x69733332
static icns_type_t ICNS_16x16_32BIT_DATA = {{'i', 's', '3', '2'}};

// il32 0x696c3332
static icns_type_t ICNS_32x32_32BIT_DATA = {{'i', 'l', '3', '2'}};

// ih32 0x69683332
static icns_type_t ICNS_48x48_32BIT_DATA = {{'i', 'h', '3', '2'}};

// it32 0x69743332
static icns_type_t ICNS_128X128_32BIT_DATA = {{'i', 't', '3', '2'}};

// s8mk 0x73386d6b
static icns_type_t ICNS_16x16_8BIT_MASK = {{'s', '8', 'm', 'k'}};

// l8mk 0x6c386d6b
static icns_type_t ICNS_32x32_8BIT_MASK = {{'l', '8', 'm', 'k'}};

// h8mk 0x68386d6b
static icns_type_t ICNS_48x48_8BIT_MASK = {{'h', '8', 'm', 'k'}};

// t8mk 0x74386d6b
static icns_type_t ICNS_128X128_8BIT_MASK = {{'t', '8', 'm', 'k'}};

static icns_type_t ICNS_FAMILY_TYPE = {{'i','c','n','s'}};

static icns_type_t ICNS_NULL_TYPE = {{0 , 0 , 0 , 0 }};

#define ICNS_STATUS_OK 0

static int icns_types_equal(icns_type_t type1, icns_type_t type2)
{
  return (strncmp((char*)&type1.c, (char*)&type2.c, 4) == 0);
}

static icns_type_t icns_get_mask_type_for_icon_type(icns_type_t type)
{
  if (icns_types_equal(type, ICNS_16x16_32BIT_DATA))
    {
      return ICNS_16x16_8BIT_MASK;
    }
  else if (icns_types_equal(type, ICNS_32x32_32BIT_DATA))
    {
      return ICNS_32x32_8BIT_MASK;
    }
  else if (icns_types_equal(type, ICNS_48x48_32BIT_DATA))
    {
      return ICNS_48x48_8BIT_MASK;
    }
  else if (icns_types_equal(type, ICNS_128X128_32BIT_DATA))
    {
      return ICNS_128X128_8BIT_MASK;
    }
  else
    {
      return ICNS_NULL_TYPE;
    }  
}

static icns_icon_info_t icns_get_image_info_for_type(icns_type_t type)
{
  icns_icon_info_t info;

  if (icns_types_equal(type, ICNS_16x16_32BIT_DATA))
    {
      info.iconHeight = 16;
      info.iconWidth = 16;
      info.iconDepth = 8;
      info.iconChannels = 4;
    }
  else if (icns_types_equal(type, ICNS_32x32_32BIT_DATA))
    {
      info.iconHeight = 32;
      info.iconWidth = 32;
      info.iconDepth = 8;
      info.iconChannels = 4;
    }
  else if (icns_types_equal(type, ICNS_48x48_32BIT_DATA))
    {
      info.iconHeight = 48;
      info.iconWidth = 48;
      info.iconDepth = 8;
      info.iconChannels = 4;
    }
  else if (icns_types_equal(type, ICNS_128X128_32BIT_DATA))
    {
      info.iconHeight = 128;
      info.iconWidth = 128;
      info.iconDepth = 8;
      info.iconChannels = 4;
    }
  else
    {
      info.iconHeight = 0;
      info.iconWidth = 0;
      info.iconDepth = 0;
      info.iconChannels = 0;
    }

  return info;
}

static int icns_get_element_from_family(icns_family_t *iconFamily,
                                        icns_type_t iconType,
                                        icns_element_t **iconElement)
{
  icns_byte_t *bytes = (icns_byte_t *)iconFamily->elements;
  unsigned long size = iconFamily->resourceSize;
  icns_element_t *element;
  icns_byte_t *data;

  data = bytes;
  element = (icns_element_t *)data;
  while ((bytes + size > data) && element->elementSize)
    {
      if (icns_types_equal(element->elementType, iconType))
        {
          *iconElement = element;
          return ICNS_STATUS_OK;
        }
      data += element->elementSize;
      element = (icns_element_t *)data;
    }

  return 1;
}

static int icns_import_family_data(int size, icns_byte_t *bytes, 
                                   icns_family_t **iconFamily)
{
  icns_element_t *element = NULL;
  icns_family_t *family;
  unsigned long el_size;
  icns_byte_t *data;
  icns_byte_t *end;

  data = bytes;
  family = (icns_family_t *)data;
  while ((bytes + size > data) && family->resourceSize)
    {
      if (icns_types_equal(family->resourceType, ICNS_FAMILY_TYPE))
        {
          element = (icns_element_t *)family;
          break;
        }
      el_size = NSSwapBigIntToHost(family->resourceSize);
      data += el_size;
      family = (icns_family_t *)data;
    }
  if (element == NULL)
    {
      return 1;
    }

  el_size = NSSwapBigIntToHost(element->elementSize);
  family = malloc(el_size);
  if (!family)
    {
      return 1;
    }

  strncpy((char*)&family->resourceType.c, (char*)&element->elementType.c, 4);
  family->resourceSize = el_size;
  memcpy((char*)(family->elements), 
         (char*)element + ICNS_HEADER_SIZE, 
         el_size - ICNS_HEADER_SIZE);

  data = (icns_byte_t *)family->elements;
  end = data + el_size - ICNS_HEADER_SIZE;
  element = family->elements;
  while ((data < end) && element->elementSize)
    {
      el_size = NSSwapBigIntToHost(element->elementSize);
      element->elementSize = el_size;
      data += el_size;
      element = (icns_element_t *)data;
    }
  
  *iconFamily = family;

  return ICNS_STATUS_OK;
}

static int icns_init_image(unsigned int iconWidth,
                           unsigned int iconHeight,
                           unsigned int iconChannels,
                           unsigned int iconPixelDepth,
                           icns_image_t *imageOut)
{
  imageOut->imageWidth = iconWidth;
  imageOut->imageHeight = iconHeight;
  imageOut->imageChannels = iconChannels;
  imageOut->imagePixelDepth = iconPixelDepth;
  imageOut->imageDataSize = (iconHeight * iconWidth 
                             * iconPixelDepth * iconChannels) / 8;
  imageOut->imageData = malloc(imageOut->imageDataSize);
  if (!imageOut->imageData)
      return 1;
  else
      return ICNS_STATUS_OK;
}

static int icns_init_image_for_type(icns_type_t iconType,
                                    icns_image_t *imageOut)
{
  icns_icon_info_t info;

  info = icns_get_image_info_for_type(iconType);
  if (info.iconChannels == 0)
    {
      return 1;
    }

  return icns_init_image(info.iconWidth, info.iconHeight, info.iconChannels,
                         info.iconDepth, imageOut);
}

static int icns_free_image(icns_image_t *imageIn)
{
  free(imageIn->imageData);
  imageIn->imageData = NULL;
  return ICNS_STATUS_OK;
}

static int icns_get_image32_with_mask_from_family(icns_family_t *iconFamily,
                                                  icns_type_t type, 
                                                  icns_image_t *iconImage)
{
  icns_element_t *element;
  unsigned int samplesPerPixel = 4;
  icns_byte_t *b;
  icns_byte_t *end;
  int j;
  int res;
  icns_type_t mask_type;
  unsigned int imageDataSize;

  if (icns_types_equal(type, ICNS_NULL_TYPE))
    return 1;

  res = icns_get_element_from_family(iconFamily, type, &element);
  if (res != ICNS_STATUS_OK)
    return res;

  res = icns_init_image_for_type(type, iconImage);
  if (res != ICNS_STATUS_OK)
    return res;

  b = (icns_byte_t *)element + ICNS_HEADER_SIZE;
  end = b + element->elementSize - ICNS_HEADER_SIZE;
  // Safety check
  if (end > (icns_byte_t *)iconFamily->elements + iconFamily->resourceSize)
    {
      icns_free_image(iconImage);
      return 1;
    }

  imageDataSize = iconImage->imageDataSize;
  if ((element->elementSize - ICNS_HEADER_SIZE) < 
      3 * iconImage->imageHeight * iconImage->imageWidth)
    {
      unsigned int plane;

      // Run length encoded planar data
      for (plane = 0; plane < 3; plane++)
        {
          unsigned int offset;
    
          offset = 0;
          while ((offset < iconImage->imageHeight * iconImage->imageWidth) 
                 && (b < end))
            {
              icns_byte_t bv = *b++;
              int runLen;
              unsigned int index = samplesPerPixel * offset + plane;
              
              if (bv & 0x80)
                {
                  // Compressed run
                  icns_byte_t val = *b++;

                  runLen = bv - 125;
                  for (j = 0; (j < runLen) && (index < imageDataSize); j++)
                    {
                      iconImage->imageData[index] = val;
                      index += samplesPerPixel;
                    }
                }
              else
                {
                  // Uncompressed run
                  int j;
                  
                  runLen = bv + 1;
                  for (j = 0; (j < runLen) && (index < imageDataSize); j++)
                    {
                      iconImage->imageData[index] = *b++;
                      index += samplesPerPixel;
                    }
                }
              
              offset += runLen;
            }
        }
    }
  else
    {
      for (j = 0; j < iconImage->imageHeight * iconImage->imageWidth; j++)
        {
          iconImage->imageData[samplesPerPixel * j + 0] = *b++;
          iconImage->imageData[samplesPerPixel * j + 1] = *b++;
          iconImage->imageData[samplesPerPixel * j + 2] = *b++;
        }
    }

  // Fill in the mask
  mask_type = icns_get_mask_type_for_icon_type(type);
  res = icns_get_element_from_family(iconFamily, mask_type, &element);
  if (res == ICNS_STATUS_OK)
    {
      b = (icns_byte_t *)element + ICNS_HEADER_SIZE;
      end = b + element->elementSize - ICNS_HEADER_SIZE;
      // Safety check
      if (end > (icns_byte_t *)iconFamily->elements + iconFamily->resourceSize)
        {
            icns_free_image(iconImage);
            return 1;
        }

      for (j = 0; j < iconImage->imageHeight * iconImage->imageWidth; j++)
        {
          iconImage->imageData[samplesPerPixel * j + 3] = *b++;
        }
    }
  else
    {
      for (j = 0; j < iconImage->imageHeight * iconImage->imageWidth; j++)
        {
          iconImage->imageData[samplesPerPixel * j + 3] = 255;
        }
    }

  return ICNS_STATUS_OK;
}

#endif /* !HAVE_LIBICNS */

// Define the pixel
typedef struct pixel_t
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pixel_t;

@implementation NSBitmapImageRep (ICNS)

+ (BOOL) _bitmapIsICNS: (NSData *)imageData
{
  char header[5];

  /*
   * If the data is 0, return immediately.
   */
  if ([imageData length] < 8)
    {
      return NO;
    }

  /*
   * Check the beginning of the data for 
   * the string "icns".
   */
  [imageData getBytes: header length: 4];
  if(strncmp(header, ICNS_HEADER, 4) == 0)
    {
      return YES;
    }

  return NO;
}

- (id) _initBitmapFromICNSImage: (icns_image_t*)iconImage
{
  unsigned int iconWidth = 0, iconHeight = 0;
  unsigned int rgbBufferPos = 0;
  unsigned int rgbBufferSize = 0;
  unsigned char *rgbBuffer = NULL; /* image converted to rgb */
  int i = 0, j = 0;
  int imageChannels = 0;
  int sPP = 4;

  iconWidth = iconImage->imageWidth;
  iconHeight = iconImage->imageHeight;

  // allocate the buffer...
  rgbBufferSize = iconHeight * (iconWidth * sizeof(unsigned char) * sPP);
  rgbBuffer = NSZoneMalloc([self zone],  rgbBufferSize); 
  if (rgbBuffer == NULL)
    {
      NSLog(@"Couldn't allocate memory for image data from ICNS.");
      RELEASE(self);
      return nil;
    }

  imageChannels = iconImage->imageChannels;
  rgbBufferPos = 0;
  for (i = 0; i < iconHeight; i++)
    {
      for (j = 0; j < iconWidth; j++)
        {
          pixel_t *src_rgb_pixel;
	  
          src_rgb_pixel = (pixel_t *)&(iconImage->imageData[i*iconWidth*imageChannels+j*imageChannels]);
          
          rgbBuffer[rgbBufferPos++] = src_rgb_pixel->r;
          rgbBuffer[rgbBufferPos++] = src_rgb_pixel->g;
          rgbBuffer[rgbBufferPos++] = src_rgb_pixel->b;
          rgbBuffer[rgbBufferPos++] = src_rgb_pixel->a;
        }
    }
  
  /* initialize self */
  [self initWithBitmapDataPlanes: &rgbBuffer
        pixelsWide: iconWidth
        pixelsHigh: iconHeight
        bitsPerSample: 8
        samplesPerPixel: sPP
        hasAlpha: YES
        isPlanar: NO
        colorSpaceName: NSCalibratedRGBColorSpace
        // FIXME: Not sure whether this format is pre-multiplied
        bitmapFormat: NSAlphaNonpremultipliedBitmapFormat
        bytesPerRow: iconWidth * sPP
        bitsPerPixel: 8 * sPP];
  
  _imageData = [[NSData alloc] initWithBytesNoCopy: rgbBuffer
                               length: rgbBufferSize];

  return self;
}

- (id) _initBitmapFromICNS: (NSData *)imageData
{
  int                     error = 0;
  int                     size = [imageData length];
  icns_byte_t            *bytes = (icns_byte_t *)[imageData bytes];
  icns_family_t          *iconFamily = NULL;
  unsigned long           dataOffset = 0;
  icns_byte_t            *data = NULL;
  icns_type_t             typeStr = ICNS_NULL_TYPE;
  icns_image_t            iconImage;

  error = icns_import_family_data(size, bytes, &iconFamily);
  if (error != ICNS_STATUS_OK)
    {
      NSLog(@"Error reading ICNS data.");
      RELEASE(self);
      return nil;
    }

  // skip the header...
  dataOffset = sizeof(icns_type_t) + sizeof(icns_size_t);
  data = (icns_byte_t *)iconFamily;

  // read each icon...
  while (((dataOffset + 8) < iconFamily->resourceSize))
    {
      icns_element_t   element;
      
      memcpy(&element, (data + dataOffset), 8);
      
      // Temporarily limit to 48 until we can find a way to 
      // utilize the other representations in the icns file.
      if (icns_types_equal(element.elementType, ICNS_48x48_32BIT_DATA) 
          || (icns_types_equal(typeStr, ICNS_NULL_TYPE) 
               && (icns_types_equal(element.elementType, ICNS_32x32_32BIT_DATA) 
                   || icns_types_equal(element.elementType, ICNS_128X128_32BIT_DATA))))
        {
          memcpy(&typeStr, &(element.elementType), 4);
        }

      // next...
      dataOffset += element.elementSize;
    }

  // extract the image...
  memset(&iconImage, 0, sizeof(icns_image_t));
  error = icns_get_image32_with_mask_from_family(iconFamily,
                                                 typeStr,
                                                 &iconImage);
  if (error)
    {
      NSLog(@"Error while extracting image from ICNS data.");
      RELEASE(self);
      free(iconFamily);
      return nil;
    }

  self = [self _initBitmapFromICNSImage: &iconImage];
  icns_free_image(&iconImage);
  free(iconFamily);

  return self;
}

+ (NSArray*) _imageRepsWithICNSData: (NSData *)imageData
{
  NSMutableArray *array = [NSMutableArray array];
  int             error = 0;
  int             size = [imageData length];
  icns_byte_t    *bytes = (icns_byte_t *)[imageData bytes];
  icns_family_t  *iconFamily = NULL;
  unsigned long   dataOffset = 0;
  icns_byte_t    *data = NULL;
  int best = 0;

  error = icns_import_family_data(size, bytes, &iconFamily);
  if (error != ICNS_STATUS_OK)
    {
      NSLog(@"Error reading ICNS data.");
      RELEASE(self);
      return array;
    }

  // skip the header...
  dataOffset = sizeof(icns_type_t) + sizeof(icns_size_t);
  data = (icns_byte_t *)iconFamily;

  // read each icon...
  while (((dataOffset + 8) < iconFamily->resourceSize))
    {
      icns_element_t element;
      icns_type_t typeStr = ICNS_NULL_TYPE;
      icns_image_t iconImage;
      
      memcpy(&element, (data + dataOffset), 8);
      memcpy(&typeStr, &(element.elementType), 4);

      // extract the image...
      memset(&iconImage, 0, sizeof(icns_image_t));
      error = icns_get_image32_with_mask_from_family(iconFamily,
                                                     typeStr,
                                                     &iconImage);
      //NSLog(@"Read image %c %c %c %c result %d size %d", typeStr.c[0], typeStr.c[1], typeStr.c[2], typeStr.c[3], error, element.elementSize);

      if (!error)
        {
          NSBitmapImageRep* imageRep;

          imageRep = [[self alloc] _initBitmapFromICNSImage: &iconImage];
          if (imageRep)
            {
              // If it exists, put the 48 icon as the first element in the array
              if (icns_types_equal(typeStr, ICNS_48x48_32BIT_DATA))
                {
                  [array insertObject: imageRep atIndex: 0];
                  best = 48;
                }
              else if (icns_types_equal(typeStr, ICNS_32x32_32BIT_DATA) && best != 48)
                {
                  [array insertObject: imageRep atIndex: 0];
                  best = 32;
                }
              else if ((icns_types_equal(typeStr, ICNS_16x16_32BIT_DATA) ||
                        icns_types_equal(typeStr, ICNS_128X128_32BIT_DATA)) &&
                       best != 48 && best != 32)
                {
                  [array insertObject: imageRep atIndex: 0];
                }
              else
                {
                  [array addObject: imageRep];
                }
              RELEASE(imageRep);
            }
          
          icns_free_image(&iconImage);
        }

      // next...
      dataOffset += element.elementSize;
    }
  
  free(iconFamily);

  return array;
}

@end
