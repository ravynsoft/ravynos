/* 
   nsimage-tiff.h 

   Functions for dealing with tiff images

   Copyright (C) 1996 Free Software Foundation, Inc.
   
   Written by:  Adam Fedor <fedor@colorado.edu>
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

/*
    Warning:  This header file should not be used for reading and
    writing tiff files.  You should use the NSImage and NSBitmapImageRep
    classes for general reading/writing of tiff files.
*/

#ifndef _GNUstep_H_tiff
#define _GNUstep_H_tiff

#include <tiffio.h>
#include <sys/types.h>

/* Structure to store common information about a tiff. */
typedef struct {
    uint16  numImages;	      /* number of images in tiff */
    uint16  imageNumber;      /* number of current image */
    uint32  subfileType;
    uint32  width;
    uint32  height;
    uint16 bitsPerSample;    /* number of bits per data channel */
    uint16 samplesPerPixel;  /* number of channels per pixel */
    uint16 planarConfig;     /* meshed or separate */
    uint16 photoInterp;      /* photometric interpretation of bitmap data, */
    uint16 compression;
    uint16 extraSamples;     /* Alpha */
    int     assocAlpha;
    int     quality;	      /* compression quality (for jpeg) 1 to 255 */
    int     error;
    float   xdpi;
    float   ydpi;
} NSTiffInfo; 

typedef struct {
    uint32 size;
    uint16 *red;
    uint16 *green;
    uint16 *blue;
} NSTiffColormap;

typedef char* realloc_data_callback(char* data, long size);

extern TIFF* NSTiffOpenDataRead(const char* data, long size);
extern TIFF* NSTiffOpenDataWrite(char **data, long *size);
extern int   NSTiffClose(TIFF* image);

extern int   NSTiffGetImageCount(TIFF* image);
extern int   NSTiffWrite(TIFF *image, NSTiffInfo *info, unsigned char *data);
extern int   NSTiffRead(TIFF *image, NSTiffInfo *info, unsigned char *data);
extern NSTiffInfo* NSTiffGetInfo(int imageNumber, TIFF* image);

extern NSTiffColormap* NSTiffGetColormap(TIFF* image);

extern int NSTiffIsCodecConfigured(unsigned int codec);

#endif // _GNUstep_H_tiff

