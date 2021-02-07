/* 
   NSBitmapImageRep.h

   Bitmap image representations

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

#ifndef _GNUstep_H_NSBitmapImageRep
#define _GNUstep_H_NSBitmapImageRep
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSImageRep.h>

@class NSArray;
@class NSString;
@class NSData;
@class NSDictionary;
@class NSMutableData;
@class NSMutableDictionary;
@class NSColor;

/** Describes the type of compression used on an image.  Possible compressions:
  <list>
   <item> NSTIFFCompressionNone; </item>
   <item> NSTIFFCompressionCCITTFAX3; </item>
   <item> NSTIFFCompressionCCITFAX4; </item>
   <item> NSTIFFCompressionLZW; </item>
   <item> NSTIFFCompressionJPEG; </item>
   <item> NSTIFFCompressionNEXT; </item>
   <item> NSTIFFCompressionPackBits. </item>
   <item> NSTIFFCompressionOldJPEG; </item>
  </list>
 */
typedef enum _NSTIFFCompression {
  NSTIFFCompressionNone  = 1,
  NSTIFFCompressionCCITTFAX3 = 3,
  NSTIFFCompressionCCITTFAX4 = 4,
  NSTIFFCompressionLZW = 5,
  NSTIFFCompressionJPEG = 6,
  NSTIFFCompressionNEXT = 32766,
  NSTIFFCompressionPackBits = 32773,
  NSTIFFCompressionOldJPEG = 32865
} NSTIFFCompression;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/** Type of image
  <list>
   <item> NSTIFFFileType; </item>
   <item> NSBMPFileType; Not implemented </item>
   <item> NSGIFFileType; </item>
   <item> NSJPEGFileType; </item>
   <item> NSPNGFileType; </item>
   <item> NSJPEG2000FileType. Not implemented </item>
  </list>
 */
typedef enum _NSBitmapImageFileType {
    NSTIFFFileType,
    NSBMPFileType,
    NSGIFFileType,
    NSJPEGFileType,
    NSPNGFileType,
    NSJPEG2000FileType  // available in Mac OS X v10.4
} NSBitmapImageFileType;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)

typedef enum _NSBitmapFormat
{
  NSAlphaFirstBitmapFormat = 1,
  NSAlphaNonpremultipliedBitmapFormat = 2,
  NSFloatingPointSamplesBitmapFormat = 4
} NSBitmapFormat;

typedef enum _NSImageRepLoadStatus
{
  NSImageRepLoadStatusUnknownType = -1,
  NSImageRepLoadStatusReadingHeader = -2,
  NSImageRepLoadStatusWillNeedAllData = -3,
  NSImageRepLoadStatusInvalidData = -4,
  NSImageRepLoadStatusUnexpectedEOF = -5,
  NSImageRepLoadStatusCompleted = -6
} NSImageRepLoadStatus;

#endif

APPKIT_EXPORT NSString *NSImageCompressionMethod;  // NSNumber; only for TIFF files
APPKIT_EXPORT NSString *NSImageCompressionFactor;  // NSNumber 0.0 to 255.0; only for JPEG files (GNUstep extension: JPEG-compressed TIFFs too)
APPKIT_EXPORT NSString *NSImageDitherTranparency;  // NSNumber boolean; only for writing GIF files
APPKIT_EXPORT NSString *NSImageRGBColorTable;  // NSData; only for reading & writing GIF files
APPKIT_EXPORT NSString *NSImageInterlaced;  // NSNumber boolean; only for writing PNG files
APPKIT_EXPORT NSString *NSImageColorSyncProfileData; // Mac OX X only
//APPKIT_EXPORT NSString *GSImageICCProfileData;  // if & when color management comes to GNUstep
APPKIT_EXPORT NSString *NSImageFrameCount;  // NSNumber integer; only for reading animated GIF files
APPKIT_EXPORT NSString *NSImageCurrentFrame; // NSNumber integer; only for animated GIF files
APPKIT_EXPORT NSString *NSImageCurrentFrameDuration;  // NSNumber float; only for reading animated GIF files
APPKIT_EXPORT NSString *NSImageLoopCount; // NSNumber integer; only for reading animated GIF files
APPKIT_EXPORT NSString *NSImageGamma; // NSNumber 0.0 to 1.0; only for reading & writing PNG files
APPKIT_EXPORT NSString *NSImageProgressive; // NSNumber boolean; only for reading & writing JPEG files
APPKIT_EXPORT NSString *NSImageEXIFData; // No GNUstep support yet; for reading & writing JPEG

#endif

@interface NSBitmapImageRep : NSImageRep
{
  // Attributes
  NSInteger _bytesPerRow;
  NSInteger _numColors;
  NSInteger _bitsPerPixel;   
  unsigned short _compression;
  float	_comp_factor;
  NSMutableDictionary *_properties;
  BOOL _isPlanar;
  unsigned char **_imagePlanes;
  NSData *_imageData;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
  NSBitmapFormat _format;
#else
  unsigned int    _format;
#endif
}

//
// Allocating and Initializing a New NSBitmapImageRep Object 
//
+ (id) imageRepWithData: (NSData*)imageData;
+ (NSArray*) imageRepsWithData: (NSData*)imageData;
- (id) initWithData: (NSData*)imageData;
- (id) initWithFocusedViewRect: (NSRect)rect;
- (id) initWithBitmapDataPlanes: (unsigned char**)planes
		     pixelsWide: (NSInteger)width
		     pixelsHigh: (NSInteger)height
		  bitsPerSample: (NSInteger)bitsPerSample
		samplesPerPixel: (NSInteger)samplesPerPixel
		       hasAlpha: (BOOL)alpha
		       isPlanar: (BOOL)isPlanar
		 colorSpaceName: (NSString*)colorSpaceName
		    bytesPerRow: (NSInteger)rowBytes
		   bitsPerPixel: (NSInteger)pixelBits;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)colorizeByMappingGray:(CGFloat)midPoint 
		      toColor:(NSColor *)midPointColor 
		 blackMapping:(NSColor *)shadowColor
		 whiteMapping:(NSColor *)lightColor;
- (id)initWithBitmapHandle:(void *)bitmap;
- (id)initWithIconHandle:(void *)icon;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSInteger) incrementalLoadFromData: (NSData *)data complete: (BOOL)complete;
- (id) initForIncrementalLoad;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
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
                   bitsPerPixel: (NSInteger)pixelBits;
#endif

//
// Getting Information about the Image 
//
- (NSInteger) bitsPerPixel;
- (NSInteger) samplesPerPixel;
- (BOOL) isPlanar;
- (NSInteger) numberOfPlanes;
- (NSInteger) bytesPerPlane;
- (NSInteger) bytesPerRow;

//
// Getting Image Data 
//
- (unsigned char*) bitmapData;
- (void) getBitmapDataPlanes: (unsigned char**)data;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSBitmapFormat) bitmapFormat;
- (void) getPixel: (NSUInteger[])pixelData atX: (NSInteger)x y: (NSInteger)y;
- (void) setPixel: (NSUInteger[])pixelData atX: (NSInteger)x y: (NSInteger)y;
- (NSColor*) colorAtX: (NSInteger)x y: (NSInteger)y;
- (void) setColor: (NSColor*)color atX: (NSInteger)x y: (NSInteger)y;
#endif 

//
// Producing a TIFF Representation of the Image 
//
+ (NSData*) TIFFRepresentationOfImageRepsInArray: (NSArray*)anArray;
+ (NSData*) TIFFRepresentationOfImageRepsInArray: (NSArray*)anArray
				usingCompression: (NSTIFFCompression)type
					  factor: (float)factor;
- (NSData*) TIFFRepresentation;
- (NSData*) TIFFRepresentationUsingCompression: (NSTIFFCompression)type
					factor: (float)factor;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (NSData *)representationOfImageRepsInArray:(NSArray *)imageReps 
				   usingType:(NSBitmapImageFileType)storageType
				  properties:(NSDictionary *)properties;
- (NSData *)representationUsingType:(NSBitmapImageFileType)storageType 
			 properties:(NSDictionary *)properties;
#endif

//
// Setting and Checking Compression Types 
//
+ (void) getTIFFCompressionTypes: (const NSTIFFCompression**)list
			   count: (NSInteger*)numTypes;
+ (NSString*) localizedNameForTIFFCompressionType: (NSTIFFCompression)type;
- (BOOL) canBeCompressedUsing: (NSTIFFCompression)compression;
- (void) getCompression: (NSTIFFCompression*)compression
		 factor: (float*)factor;
- (void) setCompression: (NSTIFFCompression)compression
		 factor: (float)factor;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
- (void)setProperty:(NSString *)property withValue:(id)value;
- (id)valueForProperty:(NSString *)property;
#endif

@end

@interface NSBitmapImageRep (GNUstepExtension)
+ (NSArray*) imageRepsWithFile: (NSString *)filename;
@end

#endif // _GNUstep_H_NSBitmapImageRep
