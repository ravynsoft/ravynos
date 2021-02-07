/** <title>NSImageRep</title>

   <abstract>Abstract representation of an image.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Adam Fedor <fedor@colorado.edu>
   Date: Feb 1996

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

#include "config.h"
#include <string.h>
#import <Foundation/NSAffineTransform.h>
#import <Foundation/NSArray.h>
#import <Foundation/NSData.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSURL.h>
#import <Foundation/NSException.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSUserDefaults.h>
#import <Foundation/NSDebug.h>
#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSImageRep.h"
#import "AppKit/NSBitmapImageRep.h"
#import "AppKit/NSCachedImageRep.h"
#import "AppKit/NSEPSImageRep.h"
#import "AppKit/NSPDFImageRep.h"
#import "AppKit/NSPICTImageRep.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSScreen.h"
#import "AppKit/DPSOperators.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSImageMagickImageRep.h"

static NSMutableArray *imageReps = nil;
static Class NSImageRep_class = NULL;

@implementation NSImageRep

+ (void) initialize
{
  /* While there are four imageRep subclasses, in practice, only two of
     them can load in data from an external source. */
  if (self == [NSImageRep class])
    {
      NSImageRep_class = self;
      imageReps = [[NSMutableArray alloc] initWithCapacity: 4];
      [imageReps addObject: [NSBitmapImageRep class]];
#if HAVE_IMAGEMAGICK
      [imageReps addObject: [NSPDFImageRep class]];
      [imageReps addObject: [NSEPSImageRep class]];
      [imageReps addObject: [NSPICTImageRep class]];
      [imageReps addObject: [GSImageMagickImageRep class]];
#endif
    }
}

// Managing NSImageRep Subclasses
+ (Class) imageRepClassForData: (NSData *)data
{
  int i, count;

  count = [imageReps count];
  for (i = 0; i < count; i++)
    {
      Class rep = [imageReps objectAtIndex: i];
      if ([rep canInitWithData: data])
	return rep;
    }
  return Nil;
}

+ (Class) imageRepClassForFileType: (NSString *)type
{
  int i, count;

  count = [imageReps count];
  for (i = 0; i < count; i++)
    {
      Class rep = [imageReps objectAtIndex: i];
      if ([[rep imageFileTypes] indexOfObject: type] != NSNotFound)
	{
	  return rep;
	}
    }
  return Nil;
}

+ (Class) imageRepClassForPasteboardType: (NSString *)type
{
  int i, count;

  count = [imageReps count];
  for (i = 0; i < count; i++)
    {
      Class rep = [imageReps objectAtIndex: i];

      if ([[rep imagePasteboardTypes] indexOfObject: type] != NSNotFound)
	{
	  return rep;
	}
    }
  return Nil;
}

+ (void) registerImageRepClass: (Class)imageRepClass
{
  if ([imageReps containsObject: imageRepClass] == NO)
    {
      Class c = imageRepClass;

      while (c != nil && c != [NSObject class] && c != [NSImageRep class])
	{
	  c = [c superclass];
	}
      if (c != [NSImageRep class])
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Attempt to register non-imagerep class"];
	}
      [imageReps addObject: imageRepClass];
    }
  [[NSNotificationCenter defaultCenter]
    postNotificationName: NSImageRepRegistryChangedNotification
		  object: self];
}

+ (NSArray *) registeredImageRepClasses
{
  return (NSArray *)imageReps;
}

+ (void) unregisterImageRepClass: (Class)imageRepClass
{
  [imageReps removeObject: imageRepClass];
  [[NSNotificationCenter defaultCenter]
    postNotificationName: NSImageRepRegistryChangedNotification
		  object: self];
}

// Creating an NSImageRep
+ (id) imageRepWithContentsOfFile: (NSString *)filename
{
  NSArray* array;

  array = [self imageRepsWithContentsOfFile: filename];
  if ([array count])
    return [array objectAtIndex: 0];
  return nil;
}

/* Helper for +imageRepsWithContentsOfFile: and imageRepsWithContentsOfURL:.
It would make sense (and make initialization of images from data more natural
for the user) to make this a real method and call it +imageRepsWithData:, but
that's already the name of one of the method concrete image reps need to
implement, so we can't do that. */
+ (NSArray *) _imageRepsWithData: (NSData *)data
{
  Class rep;

  if (self == NSImageRep_class)
    {
      rep = [self imageRepClassForData: data];
    }
  else if ([self canInitWithData: data])
    {
      rep = self;
    }
  else
    return nil;

  if ([rep respondsToSelector: @selector(imageRepsWithData:)])
    return [rep imageRepsWithData: data];
  else if ([rep respondsToSelector: @selector(imageRepWithData:)])
    {
      NSImageRep *imageRep = [rep imageRepWithData: data];

      if (imageRep != nil)
	return [NSArray arrayWithObject: imageRep];
    }

  return nil;
}

+ (NSArray *) imageRepsWithContentsOfFile: (NSString *)filename
{
  NSString *ext;
  Class rep;
  NSData* data;

  // Is the file extension already the file type?
  ext = [filename pathExtension];
  if (!ext)
    {
      /* With no extension, we can't tell the type from the file name.
      Use the data instead. */
      data = [NSData dataWithContentsOfFile: filename];
      return [self _imageRepsWithData: data];
    }
  ext = [ext lowercaseString];

  if (self == NSImageRep_class)
    {
      rep = [self imageRepClassForFileType: ext];
    }
  else if ([[self imageFileTypes] containsObject: ext])
    {
      rep = self;
    }
  else
    return nil;

  // filter non-native types to native format
  if (![[rep imageUnfilteredFileTypes] containsObject: ext])
    {
      NSPasteboard *p = [NSPasteboard pasteboardByFilteringFile: filename];
      NSArray *nativeTypes = [rep imageUnfilteredFileTypes];
      NSMutableArray *ptypes = [NSMutableArray arrayWithCapacity: [nativeTypes count]];
      NSEnumerator *enumerator = [nativeTypes objectEnumerator];
      NSString *type;

      // Convert the native types to pasteboard types
      while ((type = [enumerator nextObject]) != nil)
        {
          NSString *type2 = NSCreateFileContentsPboardType(type);
          [ptypes addObject: type2];
        }

      type = [p availableTypeFromArray: ptypes];
      data = [p dataForType: type];
      NSDebugLLog(@"NSImage", @"Filtering data for %@ from %@ of type %@ to %@", filename, p, type, data);
    }
  else
    {
      data = [NSData dataWithContentsOfFile: filename];
    }

  if (nil == data)
    return nil;

  if ([rep respondsToSelector: @selector(imageRepsWithData:)])
    {
      return [rep imageRepsWithData: data];
    }
  else if ([rep respondsToSelector: @selector(imageRepWithData:)])
    {
      NSImageRep *imageRep = [rep imageRepWithData: data];
      
      if (imageRep != nil)
        return [NSArray arrayWithObject: imageRep];
    }

  return nil;
}

+ (id)imageRepWithContentsOfURL:(NSURL *)anURL
{
  NSArray* array;

  array = [self imageRepsWithContentsOfURL: anURL];
  if ([array count])
    return [array objectAtIndex: 0];
  return nil;
}

+ (NSArray *)imageRepsWithContentsOfURL:(NSURL *)anURL
{
  NSData *data;

  // FIXME: Should we use the file type for URLs or only check the data?
  data = [anURL resourceDataUsingCache: YES];

  return [self _imageRepsWithData: data];
}

+ (id) imageRepWithPasteboard: (NSPasteboard *)pasteboard
{
  NSArray* array;

  array = [self imageRepsWithPasteboard: pasteboard];
  if ([array count])
    return [array objectAtIndex: 0];
  return nil;
}

+ (NSArray *) imageRepsWithPasteboard: (NSPasteboard *)pasteboard
{
  int i, count;
  NSMutableArray* array;
  NSArray *reps;

  if (self == NSImageRep_class)
    {
      reps = imageReps;
    }
  else
    {
      reps = [NSArray arrayWithObject: self];
    }

  array = [NSMutableArray arrayWithCapacity: 1];

  count = [reps count];
  for (i = 0; i < count; i++)
    {
      NSString* ptype;
      Class rep = [reps objectAtIndex: i];

      ptype = [pasteboard availableTypeFromArray: [rep imagePasteboardTypes]];
      if (ptype != nil)
	{
	  NSData* data = [pasteboard dataForType: ptype];

	  if ([rep respondsToSelector: @selector(imageRepsWithData:)])
	    [array addObjectsFromArray: [rep imageRepsWithData: data]];
	  else if ([rep respondsToSelector: @selector(imageRepWithData:)])
	    {
	      NSImageRep *imageRep = [rep imageRepWithData: data];

	      if (rep != nil)
		[array addObject: imageRep];
	    }
	}
    }

  if ([array count] == 0)
    return nil;

  return (NSArray *)array;
}


// Checking Data Types
+ (BOOL) canInitWithData: (NSData *)data
{
  /* Subclass responsibility */
  return NO;
}

+ (BOOL) canInitWithPasteboard: (NSPasteboard *)pasteboard
{
  NSArray *pbTypes = [pasteboard types];
  NSArray *myTypes = [self imageUnfilteredPasteboardTypes];

  return ([pbTypes firstObjectCommonWithArray: myTypes] != nil);
}

+ (NSArray *) imageFileTypes
{
  NSArray *nativeTypes = [self imageUnfilteredFileTypes];
  NSEnumerator *enumerator = [nativeTypes objectEnumerator];
  NSMutableArray *filteredTypes = [[NSMutableArray alloc] initWithCapacity: 
                                                            [nativeTypes count]];
  NSString *type;
  
  while ((type = [enumerator nextObject]) != nil)
    {
      NSEnumerator *enum2 = [[NSPasteboard typesFilterableTo: 
                                             NSCreateFileContentsPboardType(type)]
                              objectEnumerator];
      NSString *type2;

      while ((type2 = [enum2 nextObject]) != nil)
        {
          NSString *fileType = NSGetFileType(type2);
          
          if (nil != fileType)
            {
              type2 = fileType;
            }
          if ([filteredTypes indexOfObject: type2] == NSNotFound)
            {
              [filteredTypes addObject: type2];
            }
        }
    }

  return AUTORELEASE(filteredTypes);
}

+ (NSArray *) imagePasteboardTypes
{
  // FIXME: We should check what conversions are defined by services.
  return [self imageUnfilteredPasteboardTypes];
}

+ (NSArray *) imageUnfilteredFileTypes
{
  /* Subclass responsibility */
  return nil;
}

+ (NSArray *) imageUnfilteredPasteboardTypes
{
  /* Subclass responsibility */
  return nil;
}


// Instance methods
- (void) dealloc
{
  RELEASE(_colorSpace);
  [super dealloc];
}

// Setting the Size of the Image
- (void) setSize: (NSSize)aSize
{
  _size = aSize;
}

- (NSSize) size
{
  return _size;
}

// Specifying Information about the Representation
- (NSInteger) bitsPerSample
{
  return _bitsPerSample;
}

- (NSString *) colorSpaceName
{
  return _colorSpace;
}

- (BOOL) hasAlpha
{
  return _hasAlpha;
}

- (BOOL) isOpaque
{
  return _isOpaque;
}

- (NSInteger) pixelsWide
{
  return _pixelsWide;
}

- (NSInteger) pixelsHigh
{
  return _pixelsHigh;
}

- (void) setAlpha: (BOOL)flag
{
  _hasAlpha = flag;
}

- (void) setBitsPerSample: (NSInteger)anInt
{
  _bitsPerSample = anInt;
}

- (void) setColorSpaceName: (NSString *)aString
{
  ASSIGN(_colorSpace, aString);
}

- (void) setOpaque: (BOOL)flag
{
  _isOpaque = flag;
}

- (void) setPixelsWide: (NSInteger)anInt
{
  _pixelsWide = anInt;
}

- (void) setPixelsHigh: (NSInteger)anInt
{
  _pixelsHigh = anInt;
}

// Drawing the Image
- (BOOL) draw
{
  /* Subclass should implement this. */
  return YES;
}

- (BOOL) drawAtPoint: (NSPoint)aPoint
{
  BOOL ok, reset;
  NSGraphicsContext *ctxt;
  NSAffineTransform *ctm = nil;

  if (_size.width == 0 && _size.height == 0)
    return NO;

  NSDebugLLog(@"NSImage", @"Drawing at point %f %f\n", aPoint.x, aPoint.y);
  reset = 0;
  ctxt = GSCurrentContext();
  if (aPoint.x != 0 || aPoint.y != 0)
    {
      ctm = GSCurrentCTM(ctxt);
      DPStranslate(ctxt, aPoint.x, aPoint.y);
      reset = 1;
    }
  ok = [self draw];
  if (reset)
    GSSetCTM(ctxt, ctm);
  return ok;
}

- (BOOL) drawInRect: (NSRect)aRect
{
  NSSize scale;
  BOOL ok;
  NSGraphicsContext *ctxt;
  NSAffineTransform *ctm;

  NSDebugLLog(@"NSImage", @"Drawing in rect (%f %f %f %f)\n", 
	      NSMinX(aRect), NSMinY(aRect), NSWidth(aRect), NSHeight(aRect));
  if (_size.width == 0 && _size.height == 0)
    return NO;

  ctxt = GSCurrentContext();
  scale = NSMakeSize(NSWidth(aRect) / _size.width, 
		     NSHeight(aRect) / _size.height);
  ctm = GSCurrentCTM(ctxt);
  DPStranslate(ctxt, NSMinX(aRect), NSMinY(aRect));
  DPSscale(ctxt, scale.width, scale.height);
  ok = [self draw];
  GSSetCTM(ctxt, ctm);
  return ok;
}

/* New code path that delegates as much as possible to the backend and whose 
behavior precisely matches Cocoa. */
- (void) nativeDrawInRect: (NSRect)dstRect
                 fromRect: (NSRect)srcRect
                operation: (NSCompositingOperation)op
                 fraction: (CGFloat)delta
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  /* An intermediate image used to scale the image to be drawn as needed */
  NSCachedImageRep *cache;
  /* The scaled image graphics state we used as the source from which we 
     draw into the destination (the current graphics context)*/
  int gState;
  /* The context of the cache window */
  NSGraphicsContext *cacheCtxt;
  const NSSize repSize = [self size];
  /* The size of the cache window */
  NSSize cacheSize;
  
  CGFloat repToCacheWidthScaleFactor;
  CGFloat repToCacheHeightScaleFactor;
  
  NSRect srcRectInCache;
  NSAffineTransform *transform, *backup;
  
  // FIXME: Revisit this calculation of cache size

  if (([self pixelsWide] == NSImageRepMatchesDevice &&
       [self pixelsHigh] == NSImageRepMatchesDevice) &&
      (dstRect.size.width > repSize.width ||
       dstRect.size.height > repSize.height))
    {
      cacheSize = [[ctxt GSCurrentCTM] transformSize: dstRect.size];
    }
  else
    {
      cacheSize = [[ctxt GSCurrentCTM] transformSize: repSize];
    }
  
  if (cacheSize.width < 0)
    cacheSize.width *= -1;
  if (cacheSize.height < 0)
    cacheSize.height *= -1;
  
  repToCacheWidthScaleFactor = cacheSize.width / repSize.width;
  repToCacheHeightScaleFactor = cacheSize.height / repSize.height;
  
  srcRectInCache = NSMakeRect(srcRect.origin.x * repToCacheWidthScaleFactor, 
			      srcRect.origin.y * repToCacheHeightScaleFactor, 
			      srcRect.size.width * repToCacheWidthScaleFactor, 
			      srcRect.size.height * repToCacheHeightScaleFactor);
  
  cache = [[NSCachedImageRep alloc]
                initWithSize: NSMakeSize(ceil(cacheSize.width), ceil(cacheSize.height))
                       depth: [[NSScreen mainScreen] depth]
                    separate: YES
                       alpha: YES];
  
  [[[cache window] contentView] lockFocus];
  cacheCtxt = GSCurrentContext();
  
  /* Clear the cache window surface */
  DPScompositerect(cacheCtxt, 0, 0, ceil(cacheSize.width), ceil(cacheSize.height), NSCompositeClear);
  gState = [cacheCtxt GSDefineGState];
  
  //NSLog(@"Draw in cache size %@", NSStringFromSize(cacheSize));
  
  [self drawInRect: NSMakeRect(0, 0, cacheSize.width, cacheSize.height)];

  [[[cache window] contentView] unlockFocus];
  
  //NSLog(@"Draw in %@ from %@ from cache rect %@", NSStringFromRect(dstRect), 
  //  NSStringFromRect(srcRect), NSStringFromRect(srcRectInCache));
  
  backup = [ctxt GSCurrentCTM];
  
  transform = [NSAffineTransform transform];
  [transform translateXBy: dstRect.origin.x yBy: dstRect.origin.y];
  [transform scaleXBy: dstRect.size.width / srcRectInCache.size.width
		  yBy: dstRect.size.height / srcRectInCache.size.height];
  [transform concat];
  
  [ctxt GSdraw: gState
       toPoint: NSMakePoint(0,0)
      fromRect: srcRectInCache
     operation: op
      fraction: delta];
  
  [ctxt GSSetCTM: backup];
  
  [ctxt GSUndefineGState: gState];
  DESTROY(cache);
}

/* Old code path that can probably partially be merged with the new native implementation.
Fallback for backends other than Cairo. */
- (void) guiDrawInRect: (NSRect)dstRect
              fromRect: (NSRect)srcRect
             operation: (NSCompositingOperation)op
              fraction: (CGFloat)delta
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  NSAffineTransform *transform;
  NSSize repSize;

  repSize = [self size];

  if (![ctxt isDrawingToScreen])
    {
      /* We can't composite or dissolve if we aren't drawing to a screen,
         so we'll just draw the right part of the image in the right
         place. This code will only get used by the GSStreamContext. */
      NSPoint p;
      double fx, fy;

      fx = dstRect.size.width / srcRect.size.width;
      fy = dstRect.size.height / srcRect.size.height;

      p.x = dstRect.origin.x / fx - srcRect.origin.x;
      p.y = dstRect.origin.y / fy - srcRect.origin.y;

      DPSgsave(ctxt);
      DPSrectclip(ctxt, dstRect.origin.x, dstRect.origin.y,
                  dstRect.size.width, dstRect.size.height);
      DPSscale(ctxt, fx, fy);
      [self drawInRect: NSMakeRect(p.x, p.y, repSize.width, repSize.height)];
      DPSgrestore(ctxt);

      return;
    }

  /* Figure out what the effective transform from rep space to
     'window space' is.  */
  transform = [ctxt GSCurrentCTM];

  [transform scaleXBy: dstRect.size.width / srcRect.size.width
                  yBy: dstRect.size.height / srcRect.size.height];

  /* We can't composite or dissolve directly from the image reps, so we
     create a temporary off-screen window large enough to hold the
     transformed image, draw the image rep there, and composite from there
     to the destination.

     Optimization: Since we do the entire image at once, we might need a
     huge buffer.  If this starts hurting too much, there are a couple of
     things we could do to:


     1. Take srcRect into account and only process the parts of the image
     we really need.
     2. Take the clipping path into account.  Desirable, especially if we're
     being drawn as lots of small strips in a scrollview.  We don't have
     the clipping path here, though.
     3. Allocate a permanent but small buffer and process the image
     piecewise.

     */
  {
    NSCachedImageRep *cache;
    NSAffineTransformStruct ts;
    NSPoint p;
    CGFloat x0, y0, x1, y1, w, h;
    NSInteger gState;
    NSGraphicsContext *ctxt1;

    /* Figure out how big we need to make the window that'll hold the
       transformed image.  */
    p = [transform transformPoint: NSMakePoint(0, repSize.height)];
    x0 = x1 = p.x;
    y0 = y1 = p.y;

    p = [transform transformPoint: NSMakePoint(repSize.width, 0)];
    x0 = MIN(x0, p.x);
    y0 = MIN(y0, p.y);
    x1 = MAX(x1, p.x);
    y1 = MAX(y1, p.y);

    p = [transform transformPoint: NSMakePoint(repSize.width, repSize.height)];
    x0 = MIN(x0, p.x);
    y0 = MIN(y0, p.y);
    x1 = MAX(x1, p.x);
    y1 = MAX(y1, p.y);

    p = [transform transformPoint: NSMakePoint(0, 0)];
    x0 = MIN(x0, p.x);
    y0 = MIN(y0, p.y);
    x1 = MAX(x1, p.x);
    y1 = MAX(y1, p.y);

    x0 = floor(x0);
    y0 = floor(y0);
    x1 = ceil(x1);
    y1 = ceil(y1);

    w = x1 - x0;
    h = y1 - y0;

    /* This is where we want the origin of image space to be in our
       window.  */
    p.x -= x0;
    p.y -= y0;

    cache = [[NSCachedImageRep alloc]
                initWithSize: NSMakeSize(w, h)
                       depth: [[NSScreen mainScreen] depth]
                    separate: YES
                       alpha: YES];

    [[[cache window] contentView] lockFocus];
    // The context of the cache window
    ctxt1 = GSCurrentContext();
    DPScompositerect(ctxt1, 0, 0, w, h, NSCompositeClear);

    /* Set up the effective transform.  We also save a gState with this
       transform to make it easier to do the final composite.  */
    ts = [transform transformStruct];
    ts.tX = p.x;
    ts.tY = p.y;
    [transform setTransformStruct: ts];
    [ctxt1 GSSetCTM: transform];
    gState = [ctxt1 GSDefineGState];

    {
      // Hack for xlib. Without it, transparent parts of images are black
      [[NSColor clearColor] set];
      NSRectFill(NSMakeRect(0, 0, repSize.width, repSize.height));
    }

    [self drawInRect: NSMakeRect(0, 0, repSize.width, repSize.height)];

    /* If we're doing a dissolve, use a DestinationIn composite to lower
       the alpha of the pixels.  */
    if (delta != 1.0)
      {
        DPSsetalpha(ctxt1, delta);
        DPScompositerect(ctxt1, 0, 0, repSize.width, repSize.height,
                         NSCompositeDestinationIn);
      }

    [[[cache window] contentView] unlockFocus];

    DPScomposite(ctxt, srcRect.origin.x, srcRect.origin.y,
                 srcRect.size.width, srcRect.size.height, gState,
                 dstRect.origin.x, dstRect.origin.y, op);

    [ctxt GSUndefineGState: gState];

    DESTROY(cache);
  }
}

/**
 * Fallback implementation for subclasses which don't implement their
 * own direct drawing
 * TODO: explain how -draw, -drawInRect:, -drawAtPoint: clear their background
 */
- (BOOL) drawInRect: (NSRect)dstRect
	   fromRect: (NSRect)srcRect
	  operation: (NSCompositingOperation)op
	   fraction: (CGFloat)delta
     respectFlipped: (BOOL)respectFlipped
	      hints: (NSDictionary*)hints
{
  NSAffineTransform *backup = nil;
  NSGraphicsContext *ctx = GSCurrentContext();
  const BOOL compensateForFlip = (respectFlipped && [ctx isFlipped]);
  const NSSize repSize = [self size];

  // Handle abbreviated parameters

  if (NSEqualRects(srcRect, NSZeroRect))
    {
      srcRect.size = repSize;
    }
  if (NSEqualSizes(dstRect.size, NSZeroSize))
    {
      dstRect.size = repSize;
    }

  if (dstRect.size.width <= 0 || dstRect.size.height <= 0
    || srcRect.size.width <= 0 || srcRect.size.height <= 0)
    return NO;

  // Clip to image bounds

  if (srcRect.origin.x < 0)
    srcRect.origin.x = 0;
  if (srcRect.origin.y < 0)
    srcRect.origin.y = 0;
  if (NSMaxX(srcRect) > repSize.width)
    srcRect.size.width = repSize.width - srcRect.origin.x;
  if (NSMaxY(srcRect) > repSize.height)
    srcRect.size.height = repSize.height - srcRect.origin.y;

  // FIXME: Hints are currently ignored

  // Compensate for flip

  if (compensateForFlip)
    {
      NSAffineTransform *newXform;

      backup = [[ctx GSCurrentCTM] retain];

      newXform = [backup copy];
      [newXform translateXBy: dstRect.origin.x yBy: dstRect.origin.y + dstRect.size.height];
      [newXform scaleXBy: 1 yBy: -1];
      [ctx GSSetCTM: newXform];
      [newXform release];

      dstRect.origin = NSMakePoint(0, 0);
    }

  // Draw

  if ([ctx supportsDrawGState])
  {
    [self nativeDrawInRect: dstRect fromRect: srcRect operation: op fraction: delta];
  }
  else
  {
    [self guiDrawInRect: dstRect fromRect: srcRect operation: op fraction: delta];
  }

  // Undo flip compensation

  if (compensateForFlip)
    {
      [ctx GSSetCTM: backup];
      [backup release];
    }

  return YES;
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone
{
  NSImageRep	*copy;

  copy = (NSImageRep*)NSCopyObject(self, 0, zone);
  copy->_colorSpace = [_colorSpace copyWithZone: zone];

  return copy;
}

// NSCoding protocol
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aCoder encodeObject: _colorSpace];
      [aCoder encodeSize: _size];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_hasAlpha];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_isOpaque];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_bitsPerSample];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_pixelsWide];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_pixelsHigh];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      // FIXME
    }
  else
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_colorSpace];
      _size = [aDecoder decodeSize];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_hasAlpha];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_isOpaque];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_bitsPerSample];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_pixelsWide];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_pixelsHigh];
    }
  return self;
}

- (NSString*)description
{
  return [NSString stringWithFormat: @"<%@: %p size: %@ pixelsWide: %d pixelsHigh: %d colorSpaceName: %@ bps: %d>",
		   [self class],
		   self,
		   NSStringFromSize([self size]),
		   (int)[self pixelsWide],
		   (int)[self pixelsHigh],
		   [self colorSpaceName],
		   (int)[self bitsPerSample]];
}

@end

