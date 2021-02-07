/* 
   NSImageRep.h

   Abstract representation of an image.

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

#ifndef _GNUstep_H_NSImageRep
#define _GNUstep_H_NSImageRep
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSGraphicsContext.h>

@class NSString;
@class NSArray;
@class NSData;

@class NSPasteboard;

enum {
  NSImageRepMatchesDevice
};

@interface NSImageRep : NSObject <NSCoding, NSCopying>
{
  // Attributes
  NSString* _colorSpace;
  NSSize _size;
  BOOL   _hasAlpha;
  BOOL   _isOpaque;
  NSInteger _bitsPerSample;
  NSInteger _pixelsWide;
  NSInteger _pixelsHigh;
}

//
// Creating an NSImageRep
//
/** Returns a NSImageRep with the contents of filename.
 */
+ (id)imageRepWithContentsOfFile:(NSString *)filename;

/** Returns an array of newly allocated NSImageRep objects with the
 *  contents of filename.
 */
+ (NSArray *)imageRepsWithContentsOfFile:(NSString *)filename;
+ (id)imageRepWithPasteboard:(NSPasteboard *)pasteboard;
+ (NSArray *)imageRepsWithPasteboard:(NSPasteboard *)pasteboard;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (id)imageRepWithContentsOfURL:(NSURL *)anURL;
+ (NSArray *)imageRepsWithContentsOfURL:(NSURL *)anURL;
#endif

//
// Checking Data Types 
//
/** Returns NO.  It is the subclass' responsibility to implement this method.
 *  <p>See Also:</p>
 *  <list>
 *   <item>[NSBitmapImageRep+canInitWithData:]</item>
 *  </list>
 */
+ (BOOL)canInitWithData:(NSData *)data;
+ (BOOL)canInitWithPasteboard:(NSPasteboard *)pasteboard;

/** Returns array produced by calling -imageUnfilteredFileTypes.
 */
+ (NSArray *)imageFileTypes;
+ (NSArray *)imagePasteboardTypes;

/** Returns nil.  It is the subclass' responsibility to implement this method.
 *  <p>See Also:</p>
 *  <list>
 *   <item>[NSBitmapImageRep+imageUnfilteredFileTypes]</item>
 *  </list>
 */ 
+ (NSArray *)imageUnfilteredFileTypes;
+ (NSArray *)imageUnfilteredPasteboardTypes;

//
// Setting the Size of the Image 
//
/** Sets the size of the image to aSize.
 */
- (void)setSize:(NSSize)aSize;

/** Returns the size of the image.
 */
- (NSSize)size;

//
// Specifying Information about the Representation 
//
/** Returns the bits per sample of the receiver.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-setBitsPerSample:</item>
 *  </list>
 */
- (NSInteger)bitsPerSample;
- (NSString *)colorSpaceName;
- (BOOL)hasAlpha;
- (BOOL)isOpaque;
- (NSInteger)pixelsHigh;
- (NSInteger)pixelsWide;
- (void)setAlpha:(BOOL)flag;

/** Sets the number of bits for each component of a pixel.
 *  <p>See Also:</p>
 *  <list>
 *   <item>-bitsPerSample</item>
 *  </list>
 */
- (void)setBitsPerSample:(NSInteger)anInt;
- (void)setColorSpaceName:(NSString *)aString;
- (void)setOpaque:(BOOL)flag;
- (void)setPixelsHigh:(NSInteger)anInt;
- (void)setPixelsWide:(NSInteger)anInt;

//
// Drawing the Image 
//

/**
 * Primitive method which must be overridden by subclasses to
 * perform their drawing. They should draw inside a rectangle
 * with an origin at (0, 0) and a size equal to the
 * receiver's size, and they should entirely fill this rectangle,
 * overwriting the destination alpha as well.
 */
- (BOOL)draw;
- (BOOL)drawAtPoint:(NSPoint)aPoint;
- (BOOL)drawInRect:(NSRect)aRect;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (BOOL) drawInRect: (NSRect)dstRect
	   fromRect: (NSRect)srcRect
	  operation: (NSCompositingOperation)op
	   fraction: (CGFloat)delta
     respectFlipped: (BOOL)respectFlipped
	      hints: (NSDictionary*)hints;
#endif

//
// Managing NSImageRep Subclasses 
//
+ (Class)imageRepClassForData:(NSData *)data;
+ (Class)imageRepClassForFileType:(NSString *)type;
+ (Class)imageRepClassForPasteboardType:(NSString *)type;
+ (void)registerImageRepClass:(Class)imageRepClass;
+ (NSArray *)registeredImageRepClasses;
+ (void)unregisterImageRepClass:(Class)imageRepClass;

@end

typedef struct CGImage *CGImageRef;
@interface NSImageRep (GSQuartz)
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
- (CGImageRef)CGImageForProposedRect: (NSRect *)proposedDestRect 
                         context: (NSGraphicsContext *)referenceContext 
                           hints: (NSDictionary *)hints;
#endif
@end

APPKIT_EXPORT NSString *NSImageRepRegistryChangedNotification;

#endif // _GNUstep_H_NSImageRep
