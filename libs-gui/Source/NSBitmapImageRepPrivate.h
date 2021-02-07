/** <title>NSBitmapImageRepPrivate.h</title>

   <abstract>The private methods of the NSBitmapImageRep classes.</abstract>

   Copyright (C) 2016 Free Software Foundation, Inc.
   
   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: Jun 2016
   
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
#import "AppKit/NSBitmapImageRep.h"
#include "nsimage-tiff.h"

@interface NSBitmapImageRep (GSPrivate)
// GNUstep extension
+ (BOOL) _bitmapIsTIFF: (NSData *)data;
+ (NSArray*) _imageRepsWithTIFFData: (NSData *)imageData;
- (NSBitmapImageRep *) _initBitmapFromTIFF: (NSData *)imageData;
- (NSBitmapImageRep *) _initFromTIFFImage: (TIFF *)image number: (int)imageNumber;
- (void) _fillTIFFInfo: (NSTiffInfo*)info
      usingCompression: (NSTIFFCompression)type
                factor: (float)factor;

// Internal
+ (int) _localFromCompressionType: (NSTIFFCompression)type;
+ (NSTIFFCompression) _compressionTypeFromLocal: (int)type;
- (void) _premultiply;
- (void) _unpremultiply;
- (NSBitmapImageRep *) _convertToFormatBitsPerSample: (NSInteger)bps
                                     samplesPerPixel: (NSInteger)spp
                                            hasAlpha: (BOOL)alpha
                                            isPlanar: (BOOL)isPlanar
                                      colorSpaceName: (NSString*)colorSpaceName
                                        bitmapFormat: (NSBitmapFormat)bitmapFormat 
                                         bytesPerRow: (NSInteger)rowBytes
                                        bitsPerPixel: (NSInteger)pixelBits;
@end
