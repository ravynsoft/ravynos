/** <title>NSCachedImageRep</title>

   <abstract>Cached image representation.</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.
   
   Author:  Adam Fedor <fedor@gnu.org>
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
    Keeps a representation of an image in an off-screen window.  If the
    message initFromWindow:rect: is sent with a nil window, one is created
    using the rect information.
*/

// for fabs()
#include <math.h>

#import "config.h"
#import <Foundation/NSString.h>
#import <Foundation/NSException.h>
#import <Foundation/NSUserDefaults.h>

#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSBitmapImageRep.h"
#import "AppKit/NSCachedImageRep.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/PSOperators.h"

@interface GSCacheW : NSWindow
@end

@implementation GSCacheW

- (void) _initDefaults
{
  [super _initDefaults];
  [self setExcludedFromWindowsMenu: YES];
  [self setAutodisplay: NO];
  [self setReleasedWhenClosed: NO];
  [self setMiniwindowImage: nil];
}
- (void) display
{
}
- (void) displayIfNeeded
{
}
- (void) setViewsNeedDisplay: (BOOL)f
{
}
@end

/**<p>Todo Description </p>
 */
@implementation NSCachedImageRep

/** <p>Initializes and returns a new NSCachedImageRep with size 
    and depth specified by <var>aSize</var> and <var>aDepth</var> respectively.
    If seperate is YES,  the image will gets its own unique cache without 
    sharing it with other images. </p>
 */
- (id) initWithSize: (NSSize)aSize
	 pixelsWide: (NSInteger)pixelsWide
	 pixelsHigh: (NSInteger)pixelsHigh
	      depth: (NSWindowDepth)aDepth
	   separate: (BOOL)separate
	      alpha: (BOOL)alpha
{
  NSWindow	*win;

  if (aSize.width <= 0 || aSize.height <= 0 ||
      pixelsWide <= 0 || pixelsHigh <= 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"NSCachedImageRep created with size %@ pixelsWide %li pixelsHigh %li",
		   NSStringFromSize(aSize), (long int) pixelsWide, (long int) pixelsHigh];
    }

  // FIXME: Only create new window when separate is YES
  win = [[GSCacheW alloc] initWithContentRect: NSMakeRect(0, 0, pixelsWide, pixelsHigh)
				    styleMask: NSBorderlessWindowMask | NSUnscaledWindowMask
				      backing: NSBackingStoreRetained
					defer: NO];
 
  [[win contentView] setBoundsSize: NSMakeSize(aSize.width, aSize.height)];
  
  self = [self initWithWindow: win rect: NSMakeRect(0, 0, aSize.width, aSize.height)];
  RELEASE(win);
  if (!self)
    return nil;
  
  [self setAlpha: alpha];
  [self setBitsPerSample: NSBitsPerSampleFromDepth(aDepth)];
  [self setSize: aSize];
  [self setPixelsWide: pixelsWide];
  [self setPixelsHigh: pixelsHigh];
 
  return self;
}

- (id) initWithSize: (NSSize)aSize
	      depth: (NSWindowDepth)aDepth
	   separate: (BOOL)separate
	      alpha: (BOOL)alpha
{
  return [self initWithSize: aSize
		 pixelsWide: aSize.width
		 pixelsHigh: aSize.height
		      depth: aDepth
		   separate: separate
		      alpha: alpha];
}

/** <p>Initializes and returns a new NSCachedImageRep into a NSWindow 
    <var>aWindow</var>. The image will be draw into the rectange aRect of
    this window. aWindow is retained.</p>
    <p>See Also: -rect -window</p>
 */
- (id) initWithWindow: (NSWindow *)aWindow rect: (NSRect)aRect
{
  self = [super init];
  if (!self)
    return nil;

  _window = RETAIN(aWindow);
  _rect   = aRect;

  /* Either win or rect must be non-NULL. If rect is empty, we get the
     frame info from the window. If win is nil we create it from the
     rect information. */
  if (NSIsEmptyRect(_rect))
    {
      if (!_window) 
        {
          [NSException raise: NSInvalidArgumentException
                       format: @"Must specify either window or rect when "
                       @"creating NSCachedImageRep"];
        }

      _rect = [_window frame];
    }
  if (!_window)
    _window = [[GSCacheW alloc] initWithContentRect: _rect
					  styleMask: NSBorderlessWindowMask | NSUnscaledWindowMask
					    backing: NSBackingStoreRetained
					      defer: NO];
  [self setSize: _rect.size];
  [self setAlpha: NO];
  [self setOpaque: YES];
  [self setPixelsHigh: _rect.size.height];
  [self setPixelsWide: _rect.size.width];
  return self;
}

- (void) dealloc
{
  RELEASE(_window);
  [super dealloc];
}

/** <p>Returns the rectangle where the image is cached.</p>
    <p>See Also: -initWithWindow:rect:</p>
 */
- (NSRect) rect
{
  return _rect;
}

/** <p>Returns the NSWindow where the image is cached.</p>
    <p>See Also: -initWithWindow:rect:</p>
 */
- (NSWindow *) window
{
  return _window;
}

- (BOOL) draw
{
/*
  Horrible hack to get drawing on a scaled or rotated
  context correct. Only used on backends not supporting GSdraw:
  (art/xlib backend).
*/
  NSGraphicsContext *ctxt = GSCurrentContext();
  NSAffineTransform *transform;
  NSAffineTransformStruct ts;
  // Is there anything to draw?
  if (NSIsEmptyRect(_rect))
    return YES;

  transform = [ctxt GSCurrentCTM];
  ts = [transform transformStruct];
      
  if (fabs(ts.m11 - 1.0) < 0.01 && fabs(ts.m12) < 0.01
      && fabs(ts.m21) < 0.01 && fabs(ts.m22 - 1.0) < 0.01)
    {
      PScomposite(NSMinX(_rect), NSMinY(_rect), NSWidth(_rect), NSHeight(_rect),
                  [_window gState], 0, 0, NSCompositeSourceOver);
    }
  else
    {
      NSView *view = [_window contentView];
      NSBitmapImageRep *rep;

      [view lockFocus];
      rep = [[NSBitmapImageRep alloc] initWithFocusedViewRect: _rect];
      [view unlockFocus];
      
      [rep draw];
      RELEASE(rep);
    }

  return YES;
}

/**
 * Overridden private NSImageRep method called for backends supporting
 * drawGState. This is an efficient implementation which boils down to
 * cairo calls to set the source surface, clip, and paint.
 */
- (void) nativeDrawInRect: (NSRect)dstRect
                 fromRect: (NSRect)srcRect
                operation: (NSCompositingOperation)op
                 fraction: (CGFloat)delta
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  NSWindow *window = [self window];
  NSInteger gState = [window gState];
  NSAffineTransform *transform, *backup;
  NSRect winSrcRect;

  // Convert srcRect from rep coordinate space to window base coordinate space
  {
    CGFloat repToWinWidthScaleFactor = [window frame].size.width / [self size].width;
    CGFloat repToWinHeightScaleFactor = [window frame].size.height / [self size].height;
    
    winSrcRect = NSMakeRect(srcRect.origin.x * repToWinWidthScaleFactor,
			    srcRect.origin.y * repToWinHeightScaleFactor,
			    srcRect.size.width * repToWinWidthScaleFactor,
			    srcRect.size.height * repToWinHeightScaleFactor);
  }

  //NSLog(@"Using NSCachedImageRep nativeDrawInRect: fast path for %@", self);
  
  backup = [ctxt GSCurrentCTM];
  
  transform = [NSAffineTransform transform];
  [transform translateXBy: dstRect.origin.x yBy: dstRect.origin.y];
  [transform scaleXBy: dstRect.size.width / winSrcRect.size.width
		  yBy: dstRect.size.height / winSrcRect.size.height];
  [transform concat];
  
  [ctxt GSdraw: gState
       toPoint: NSMakePoint(0,0)
      fromRect: winSrcRect
     operation: op
      fraction: delta];
  
  [ctxt GSSetCTM: backup];
}

// NSCoding protocol
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding] == NO)
    {
      [aCoder encodeObject: _window];
      [aCoder encodeRect: _rect];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding] == NO)
    {
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_window];
      _rect = [aDecoder decodeRect];
    }
  return self;
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone
{
  // Cached images should not be copied
  return nil;
}

@end

