/* 
   tiff.m

   Functions for dealing with tiff images.

   Copyright (C) 1996,1999-2010, 2017 Free Software Foundation, Inc.
   
   Author:  Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996

   Support for writing tiffs: Richard Frith-Macdonald

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

/* Code in NSTiffRead, NSTiffGetInfo, and NSTiffGetColormap 
   is derived from tif_getimage, by Sam Leffler. See the copyright below.
*/

/*
 * Copyright (c) 1991, 1992, 1993, 1994 Sam Leffler
 * Copyright (c) 1991, 1992, 1993, 1994 Silicon Graphics, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and 
 * its documentation for any purpose is hereby granted without fee, provided
 * that (i) the above copyright notices and this permission notice appear in
 * all copies of the software and related documentation, and (ii) the names of
 * Sam Leffler and Silicon Graphics may not be used in any advertising or
 * publicity relating to the software without the specific, prior written
 * permission of Sam Leffler and Silicon Graphics.
 * 
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY 
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  
 * 
 * IN NO EVENT SHALL SAM LEFFLER OR SILICON GRAPHICS BE LIABLE FOR
 * ANY SPECIAL, INCIDENTAL, INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND,
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER OR NOT ADVISED OF THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF 
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE 
 * OF THIS SOFTWARE.
 */

#include "config.h"
#include "nsimage-tiff.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSString.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import "GSGuiPrivate.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>
#ifndef __WIN32__
#include <unistd.h>		/* for L_SET, etc definitions */
#endif /* !__WIN32__ */

#if !defined(TIFF_VERSION_CLASSIC)
// This only got added in version 4 of libtiff, but TIFFLIB_VERSION is unusable to differentiate here
typedef tsize_t tmsize_t;
#endif

typedef struct {
  char* data;
  long  size;
  long  position;
  char  mode;
  char **outdata;
  long *outposition;
} chandle_t;

static int tiff_error_handler_set = 0;

static void
NSTiffError(const char *func, const char *msg, va_list ap)
{
  NSString *format;

  format = [NSString stringWithFormat: @"Tiff Error (%s) %s", func, msg];
  NSLogv (format, ap);
}

static void 
NSTiffWarning(const char *func, const char *msg, va_list ap)
{
  NSString *format;

  format = [NSString stringWithFormat: @"Tiff Warning (%s) %s", func, msg];
  format = [NSString stringWithFormat: format  arguments: ap];
  NSDebugLLog (@"NSTiff", @"%@", format);
}

/* Client functions that provide reading/writing of data for libtiff */
static tmsize_t
TiffHandleRead(thandle_t handle, void* buf, tmsize_t count)
{
  chandle_t* chand = (chandle_t *)handle;
  if (chand->position >= chand->size)
    return 0;
  if (chand->position + count > chand->size)
    count = chand->size - chand->position;
  memcpy(buf, chand->data + chand->position, count);
  return count;
}

static tmsize_t
TiffHandleWrite(thandle_t handle, void* buf, tmsize_t count)
{
  chandle_t* chand = (chandle_t *)handle;
  if (chand->mode == 'r')
    return 0;
  if (chand->position + count > chand->size)
    {
      chand->size = chand->position + count + 1;
      chand->data = realloc(chand->data, chand->size);
      *(chand->outdata) = chand->data;
      if (chand->data == NULL)
	return 0;
    }
  memcpy(chand->data + chand->position, buf, count);
  chand->position += count;
  if (chand->position > *(chand->outposition))
    *(chand->outposition) = chand->position;
  
  return count;
}

static toff_t
TiffHandleSeek(thandle_t handle, toff_t offset, int mode)
{
  chandle_t* chand = (chandle_t *)handle;
  switch(mode) 
    {
    case SEEK_SET: chand->position = offset; break;
    case SEEK_CUR: chand->position += offset; break;
    case SEEK_END: 
      // FIXME: Not sure whether this check is correct
      if (offset > 0 && chand->mode == 'r')
        return 0;
      chand->position = chand->size - ((chand->size > 0) ? 1 : 0) + offset;
      break;
    }
  return chand->position;
}

static int
TiffHandleClose(thandle_t handle)
{
  chandle_t* chand = (chandle_t *)handle;

  /* Presumably, we don't need the handle anymore */
  free(chand);
  return 0;
}

static toff_t
TiffHandleSize(thandle_t handle)
{
  chandle_t* chand = (chandle_t *)handle;
  return chand->size;
}

static int
TiffHandleMap(thandle_t handle, void** data, toff_t* size)
{
  chandle_t* chand = (chandle_t *)handle;
  
  *data = chand->data;
  *size = chand->size;
    
  return 1;
}

static void
TiffHandleUnmap(thandle_t handle, void* data, toff_t size)
{
  /* Nothing to unmap. */
}

/* Open a tiff from a stream. Returns NULL if can't read tiff information.  */
TIFF* 
NSTiffOpenDataRead(const char* data, long size)
{
  chandle_t* handle;

  if (tiff_error_handler_set == 0)
    {
      tiff_error_handler_set = 1;
      TIFFSetErrorHandler(NSTiffError);
      TIFFSetWarningHandler(NSTiffWarning);
    }

  handle = malloc(sizeof(chandle_t));
  handle->data = (char*)data;
  handle->outdata = 0;
  handle->position = 0;
  handle->outposition = 0;
  handle->size = size;
  handle->mode = 'r';
  return TIFFClientOpen("GSTiffReadData", "r",
			(thandle_t)handle,
			TiffHandleRead, TiffHandleWrite,
			TiffHandleSeek, TiffHandleClose,
			TiffHandleSize,
			TiffHandleMap, TiffHandleUnmap);
}

TIFF* 
NSTiffOpenDataWrite(char **data, long *size)
{
  chandle_t* handle;
  handle = malloc(sizeof(chandle_t));
  handle->data = *data;
  handle->outdata = data;
  handle->position = 0;
  handle->outposition = size;
  handle->size = *size;
  handle->mode = 'w';
  return TIFFClientOpen("GSTiffWriteData", "w",
			(thandle_t)handle,
			TiffHandleRead, TiffHandleWrite,
			TiffHandleSeek, TiffHandleClose,
			TiffHandleSize,
			TiffHandleMap, TiffHandleUnmap);
}

int  
NSTiffClose(TIFF* image)
{
  TIFFClose(image);
  return 0;
}

int   
NSTiffGetImageCount(TIFF* image)
{
  int dircount = 1;

  if (image == NULL)
    return 0;

  while (TIFFReadDirectory(image))
    {
      dircount++;
    } 
  TIFFSetDirectory(image, 0);
  return dircount;
}

/* Read some information about the image. Note that currently we don't
   determine numImages. */
NSTiffInfo *      
NSTiffGetInfo(int imageNumber, TIFF* image)
{
  NSTiffInfo* info;
  uint16 *sample_info = NULL;

  if (image == NULL)
    return NULL;

  info = malloc(sizeof(NSTiffInfo));
  memset(info, 0, sizeof(NSTiffInfo));
  if (imageNumber >= 0)
    {
      if (TIFFSetDirectory(image, imageNumber) == 0)
	return NULL;
      info->imageNumber = imageNumber;
    }
  
  TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &info->width);
  TIFFGetField(image, TIFFTAG_IMAGELENGTH, &info->height);
  TIFFGetField(image, TIFFTAG_COMPRESSION, &info->compression);
  if (info->compression == COMPRESSION_JPEG)
    TIFFGetField(image, TIFFTAG_JPEGQUALITY, &info->quality);
  TIFFGetField(image, TIFFTAG_SUBFILETYPE, &info->subfileType);
  TIFFGetField(image, TIFFTAG_EXTRASAMPLES, &info->extraSamples, &sample_info);
  info->extraSamples = (info->extraSamples == 1 
			&& ((sample_info[0] == EXTRASAMPLE_ASSOCALPHA) 
			    || (sample_info[0] == EXTRASAMPLE_UNASSALPHA)));
  info->assocAlpha = (info->extraSamples == 1 
		      && sample_info[0] == EXTRASAMPLE_ASSOCALPHA);

  /* If the following tags aren't present then use the TIFF defaults. */
  TIFFGetFieldDefaulted(image, TIFFTAG_BITSPERSAMPLE, &info->bitsPerSample);
  TIFFGetFieldDefaulted(image, TIFFTAG_SAMPLESPERPIXEL, 
			&info->samplesPerPixel);
  TIFFGetFieldDefaulted(image, TIFFTAG_PLANARCONFIG, 
			&info->planarConfig);

  /* If TIFFTAG_PHOTOMETRIC is not present then assign a reasonable default.
     The TIFF 5.0 specification doesn't give a default. */
  if (!TIFFGetField(image, TIFFTAG_PHOTOMETRIC, &info->photoInterp)) 
    {
      switch (info->samplesPerPixel) 
	{
	case 1:
	  info->photoInterp = PHOTOMETRIC_MINISBLACK;
	  break;
	case 3: case 4:
	  info->photoInterp = PHOTOMETRIC_RGB;
	  break;
	default:
	  TIFFError(TIFFFileName(image),
		    "Missing needed \"PhotometricInterpretation\" tag");
	  return NULL;
	}
      TIFFError(TIFFFileName(image),
		"No \"PhotometricInterpretation\" tag, assuming %s\n",
		info->photoInterp == PHOTOMETRIC_RGB ? "RGB" : "min-is-black");
    }

  {
    uint16 resolution_unit;
    float xres, yres;
    if (TIFFGetField(image, TIFFTAG_XRESOLUTION, &xres) 
	&& TIFFGetField(image, TIFFTAG_YRESOLUTION, &yres))
      {
	TIFFGetFieldDefaulted(image, TIFFTAG_RESOLUTIONUNIT, &resolution_unit);
	if (resolution_unit == 2) // Inch
	  {
	    info->xdpi = xres;
	    info->ydpi = yres;
	  }
	else if (resolution_unit == 3) // Centimeter
	  {
	    info->xdpi = xres * 2.54;
	    info->ydpi = yres * 2.54;
	  }
      }   
  }

  return info;
}

#define READ_SCANLINE(sample)				\
  if (TIFFReadScanline(image, buf, row, sample) != 1)	\
    {							\
      error = 1;					\
      break;						\
    }

/* Read an image into a data array.  The data array is assumed to have been
   already allocated to the correct size.

   Note that palette images are implicitly coverted to 24-bit contig
   direct color images. Thus the data array should be large 
   enough to hold this information. */
int
NSTiffRead(TIFF *image, NSTiffInfo *info, unsigned char *data)
{
  int     i;
  unsigned int row, col;
  int	  error = 0;
  uint8* outP;
  uint8* buf;
  uint8* raster;
  NSTiffColormap* map;
  tmsize_t scan_line_size;

  if (data == NULL)
    return -1;
	
  map = NULL;
  if (info->photoInterp == PHOTOMETRIC_PALETTE) 
    {
      map = NSTiffGetColormap(image);
      if (!map)
	return -1;
    }

  scan_line_size = TIFFScanlineSize(image);
  buf = _TIFFmalloc(scan_line_size);
  
  raster = (uint8 *)data;
  outP = raster;
  switch (info->photoInterp) 
    {
    case PHOTOMETRIC_MINISBLACK:
    case PHOTOMETRIC_MINISWHITE:
      if (info->planarConfig == PLANARCONFIG_CONTIG) 
	{
	  for (row = 0; row < info->height; ++row) 
	    {
	      READ_SCANLINE(0);
	      memcpy(outP, buf, scan_line_size);
	      outP += scan_line_size;
	    }
	} 
      else 
	{
	  for (i = 0; i < info->samplesPerPixel; i++)
	    for (row = 0; row < info->height; ++row) 
	      {
		READ_SCANLINE(i);
		memcpy(outP, buf, scan_line_size);
		outP += scan_line_size;
	      }
	}
      break;
    case PHOTOMETRIC_PALETTE:
      {
	for (row = 0; row < info->height; ++row) 
	  {
	    uint8 *inP;
	    READ_SCANLINE(0);
	    inP = buf;
	    for (col = 0; col < info->width; col++) 
	      {
		*outP++ = map->red[*inP] / 256;
		*outP++ = map->green[*inP] / 256;
		*outP++ = map->blue[*inP] / 256;
		inP++;
	      }
	  }
	free(map);
      }
      break;
    case PHOTOMETRIC_RGB:
      if (info->planarConfig == PLANARCONFIG_CONTIG) 
	{
	  for (row = 0; row < info->height; ++row) 
	    {
	      READ_SCANLINE(0);
	      memcpy(outP, buf, scan_line_size);
	      outP += scan_line_size;
	    }
	} 
      else 
	{
	  for (i = 0; i < info->samplesPerPixel; i++)
	    for (row = 0; row < info->height; ++row) 
	      {
		READ_SCANLINE(i);
		memcpy(outP, buf, scan_line_size);
		outP += scan_line_size;
	      }
	}
      break;
    default:
      NSLog(@"Tiff: reading photometric %d not supported", info->photoInterp);
      error = 1;
      break;
    }
    
  _TIFFfree(buf);
  return error;
}

#define WRITE_SCANLINE(sample) \
	if (TIFFWriteScanline(image, buf, row, sample) != 1) { \
	    error = 1; \
	    break; \
	}

int  
NSTiffWrite(TIFF *image, NSTiffInfo *info, unsigned char *data)
{
  void*	buf = (void*)data;
  uint16        sample_info[1];
  int		i;
  unsigned int 	row;
  int           error = 0;
  tmsize_t      scan_line_size;

  if (info->numImages > 1)
    {
      /* Set the page number */
      TIFFSetField(image, TIFFTAG_PAGENUMBER, info->imageNumber, info->numImages);
    }

  TIFFSetField(image, TIFFTAG_IMAGEWIDTH, info->width);
  TIFFSetField(image, TIFFTAG_IMAGELENGTH, info->height);
  if (info->xdpi && info->ydpi)
    {
      TIFFSetField(image, TIFFTAG_XRESOLUTION, info->xdpi);
      TIFFSetField(image, TIFFTAG_YRESOLUTION, info->ydpi);
      TIFFSetField(image, TIFFTAG_RESOLUTIONUNIT, 2);
    }
  TIFFSetField(image, TIFFTAG_COMPRESSION, info->compression);
  if (info->compression == COMPRESSION_JPEG)
    {
      TIFFSetField(image, TIFFTAG_JPEGQUALITY, info->quality);
    }
  TIFFSetField(image, TIFFTAG_SUBFILETYPE, info->subfileType);
  TIFFSetField(image, TIFFTAG_BITSPERSAMPLE, info->bitsPerSample);
  TIFFSetField(image, TIFFTAG_SAMPLESPERPIXEL, info->samplesPerPixel);
  TIFFSetField(image, TIFFTAG_PLANARCONFIG, info->planarConfig);
  TIFFSetField(image, TIFFTAG_PHOTOMETRIC, info->photoInterp);

  if (info->assocAlpha)
    sample_info[0] = EXTRASAMPLE_ASSOCALPHA;
  else
    sample_info[0] = EXTRASAMPLE_UNASSALPHA;
  TIFFSetField(image, TIFFTAG_EXTRASAMPLES, info->extraSamples, sample_info);
  scan_line_size = TIFFScanlineSize(image);

  switch (info->photoInterp) 
    {
      case PHOTOMETRIC_MINISBLACK:
      case PHOTOMETRIC_MINISWHITE:
	if (info->planarConfig == PLANARCONFIG_CONTIG) 
	  {
	    for (row = 0; row < info->height; ++row) 
	      {
		WRITE_SCANLINE(0)
		buf += scan_line_size;
	      }
	  } 
	else 
	  {
	    for (i = 0; i < info->samplesPerPixel; i++)
	      {
		for (row = 0; row < info->height; ++row) 
		  {
		    WRITE_SCANLINE(i)
		    buf += scan_line_size;
		  }
	      }
	  }
	break;

      case PHOTOMETRIC_RGB:
	if (info->planarConfig == PLANARCONFIG_CONTIG) 
	  {
	    for (row = 0; row < info->height; ++row) 
	      {
		WRITE_SCANLINE(0)
		buf += scan_line_size;
	      }
	  } 
	else 
	  {
	    for (i = 0; i < info->samplesPerPixel; i++)
	      {
		for (row = 0; row < info->height; ++row) 
		  {
		    WRITE_SCANLINE(i)
		    buf += scan_line_size;
		  }
	      }
	  }
	break;

      default:
	NSLog(@"Tiff: photometric %d for image %s not supported", 
	      info->photoInterp, TIFFFileName(image));
	return -1;
	break;
    }

  // Write out the directory as there may be more images comming
  TIFFWriteDirectory(image);
  TIFFFlush(image);

  return error;
}

/*------------------------------------------------------------------------*/

/* Many programs get TIFF colormaps wrong.  They use 8-bit colormaps
   instead of 16-bit colormaps.  This function is a heuristic to
   detect and correct this. */
static int
CheckAndCorrectColormap(NSTiffColormap* map)
{
  register unsigned int i;

  for (i = 0; i < map->size; i++)
    if ((map->red[i] > 255)||(map->green[i] > 255)||(map->blue[i] > 255))
      return 16;

#define	CVT(x)		(((x) * 255) / ((1L<<16)-1))
  for (i = 0; i < map->size; i++) 
    {
      map->red[i] = CVT(map->red[i]);
      map->green[i] = CVT(map->green[i]);
      map->blue[i] = CVT(map->blue[i]);
    }
  return 8;
}

/* Gets the colormap for the image if there is one. Returns a
   NSTiffColormap if one was found.
*/
NSTiffColormap *
NSTiffGetColormap(TIFF* image)
{
  NSTiffInfo* info;
  NSTiffColormap* map;

  /* Re-read the tiff information. We pass -1 as the image number which
     means just read the current image. */
  info = NSTiffGetInfo(-1, image);
  if (info->photoInterp != PHOTOMETRIC_PALETTE)
    return NULL;

  map = malloc(sizeof(NSTiffColormap));
  map->size = 1 << info->bitsPerSample;

  if (!TIFFGetField(image, TIFFTAG_COLORMAP,
		    &map->red, &map->green, &map->blue)) 
    {
      TIFFError(TIFFFileName(image), "Missing required \"Colormap\" tag");
      free(map);
      return NULL;
    }
  if (CheckAndCorrectColormap(map) == 8)
    TIFFWarning(TIFFFileName(image), "Assuming 8-bit colormap");

  free(info);
  return map;
}

int NSTiffIsCodecConfigured(unsigned int codec)
{
#if (TIFFLIB_VERSION >= 20041016)
  // starting with version 3.7.0 we can ask libtiff what it is configured to do
  return TIFFIsCODECConfigured(codec);
#else
  // we check the tiffconf.h
#include <tiffconf.h>
#ifndef CCITT_SUPPORT
#  define CCITT_SUPPORT 0
#else
#  define CCITT_SUPPORT 1
#endif
#ifndef PACKBITS_SUPPORT
#  define PACKBITS_SUPPORT 0
#else
#  define PACKBITS_SUPPORT 1
#endif
#ifndef OJPEG_SUPPORT
#  define OJPEG_SUPPORT 0
#else
#  define OJPEG_SUPPORT 1
#endif
#ifndef LZW_SUPPORT
#  define LZW_SUPPORT 0
#else
#  define LZW_SUPPORT 1
#endif
#ifndef NEXT_SUPPORT
#  define NEXT_SUPPORT 0
#else
#  define NEXT_SUPPORT 1
#endif
#ifndef JPEG_SUPPORT
#  define JPEG_SUPPORT 0
#else
#  define JPEG_SUPPORT 1
#endif
/* If this fails, your libtiff is obsolete! Come to think of it
 * if you even are compiling this part your libtiff is obsolete. */
  switch (codec)
  {
    case COMPRESSION_NONE: return 1;
    case COMPRESSION_CCITTFAX3: return CCITT_SUPPORT;
    case COMPRESSION_CCITTFAX4: return CCITT_SUPPORT;
    case COMPRESSION_JPEG: return JPEG_SUPPORT;
    case COMPRESSION_PACKBITS: return PACKBITS_SUPPORT;
    case COMPRESSION_OJPEG: return OJPEG_SUPPORT;
    case COMPRESSION_LZW: return LZW_SUPPORT;
    case COMPRESSION_NEXT: return NEXT_SUPPORT;
    default:
      return 0;
  }
#endif
}


