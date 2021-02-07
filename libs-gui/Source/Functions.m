/*
   Functions.m

   Generic Functions for the GNUstep GUI Library.

   Copyright (C) 1996,1999 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   
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

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSProcessInfo.h>

#import "GSGuiPrivate.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSBitmapImageRep.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphicsContext.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/DPSOperators.h"
#import "AppKit/NSStoryboard.h"

char **NSArgv = NULL;

@interface NSStoryboard (Private)

+ (void) _setMainStoryboard: (NSStoryboard *)storyboard;
- (void) _instantiateApplicationScene;

@end


/*
 * Main initialization routine for the GNUstep GUI Library Apps
 */
int
NSApplicationMain(int argc, const char **argv)
{
  NSDictionary		*infoDict;
  NSString              *mainModelFile;
  NSString		*className;
  Class			appClass;
  CREATE_AUTORELEASE_POOL(pool);
#if defined(LIB_FOUNDATION_LIBRARY) || defined(GS_PASS_ARGUMENTS)
  extern char		**environ;

  [NSProcessInfo initializeWithArguments: (char**)argv
				   count: argc
			     environment: environ];
#endif

  infoDict = [[NSBundle mainBundle] infoDictionary];
  className = [infoDict objectForKey: @"NSPrincipalClass"];
  appClass = NSClassFromString(className);

  if (appClass == 0)
    {
      NSLog(@"Bad application class '%@' specified", className);
      appClass = [NSApplication class];
    }
  [appClass sharedApplication];

  mainModelFile = [infoDict objectForKey: @"NSMainNibFile"];
  if (mainModelFile != nil && [mainModelFile isEqual: @""] == NO)
    {
      if ([NSBundle loadNibNamed: mainModelFile owner: NSApp] == NO)
	{
	  NSLog (_(@"Cannot load the main model file '%@'"), mainModelFile);
	}
    }
  else
    {
      mainModelFile = [infoDict objectForKey: @"NSMainStoryboardFile"];
      if (mainModelFile != nil && [mainModelFile isEqual: @""] == NO)
        {
          NSStoryboard *storyboard = [NSStoryboard storyboardWithName: mainModelFile
                                                               bundle: [NSBundle mainBundle]];
          if (storyboard == nil)
            {
              NSLog (_(@"Cannot load the main storyboard file '%@'"), mainModelFile);
            }
          else
            {
              [NSStoryboard _setMainStoryboard: storyboard];
              [storyboard _instantiateApplicationScene];
              [storyboard instantiateInitialController];
            }
        }
      else
        {
          NSLog(@"Storyboard is blank or nil, unable to load.");
        }
    }

  RECREATE_AUTORELEASE_POOL(pool);

  [NSApp run];

  DESTROY(NSApp);

  [pool drain];

  return 0;
}

/*
 * Color Functions
 */

/*
 * Get Information About Color Space and Window Depth
 */
const NSWindowDepth*
NSAvailableWindowDepths(void)
{
  /*
   * Perhaps this is the only function which
   * belongs in the backend.   It should be possible
   * to detect which depths the window server is capable
   * of.
   */	
  return (const NSWindowDepth *)_GSWindowDepths;
}

NSWindowDepth
NSBestDepth(NSString *colorSpace, NSInteger bitsPerSample, NSInteger bitsPerPixel,
  BOOL planar, BOOL *exactMatch)
{
  NSInteger		components = NSNumberOfColorComponents(colorSpace);
  NSInteger		index = 0;
  const NSWindowDepth	*depths = NSAvailableWindowDepths();
  NSWindowDepth		bestDepth = NSDefaultDepth;
  
  if (exactMatch != NULL)
    *exactMatch = NO;

  if (components == 1)
    {	
      for (index = 0; depths[index] != 0; index++)
	{
	  NSWindowDepth	depth = depths[index];

	  if (NSPlanarFromDepth(depth))
	    {
	      bestDepth = depth;
	      if (NSBitsPerSampleFromDepth(depth) == bitsPerSample)
		{
                  if (exactMatch != NULL)
                    *exactMatch = YES;
		}
	    }
	}
    }
  else
    {
      for (index = 0; depths[index] != 0; index++)
	{
	  NSWindowDepth	depth = depths[index];

	  if (!NSPlanarFromDepth(depth))
	    {
	      bestDepth = depth;
	      if (NSBitsPerSampleFromDepth(depth) == bitsPerSample)
		{
                  if (exactMatch != NULL)
                    *exactMatch = YES;
		}
	    }
	}
    }
  
  return bestDepth;
}

NSInteger
NSBitsPerPixelFromDepth(NSWindowDepth depth)
{
  NSInteger bps = NSBitsPerSampleFromDepth(depth);
  NSInteger spp = 0;
  
  if (depth & _GSRGBBitValue)
    {
      spp = 3;	
    }
  else if (depth & _GSCMYKBitValue)
    {
      spp = 4;
    }
  else if (depth & _GSGrayBitValue)
    {
      spp = 1;
    }
  return (spp * bps);
}

NSInteger
NSBitsPerSampleFromDepth(NSWindowDepth depth)
{
  NSWindowDepth	bitValue = 0;

  /*
   * Test against colorspace bit.
   * and out the bit to get the bps value.
   */
  if (depth & _GSRGBBitValue)
    {
      bitValue = _GSRGBBitValue;
    }
  else if (depth & _GSCMYKBitValue)
    {
      bitValue = _GSCMYKBitValue;
    }
  else if (depth & _GSGrayBitValue)
    {
      bitValue = _GSGrayBitValue;
    }
  /*
   * AND against the complement
   * to extract the bps value.	
   */
  return (depth & ~(bitValue));
}

NSString*
NSColorSpaceFromDepth(NSWindowDepth depth)
{
  NSString	*colorSpace = NSCalibratedWhiteColorSpace;
  
  /*
   * Test against each of the possible colorspace bits
   * and return the corresponding colorspace.
   */
  if (depth == 0)
    {
      colorSpace = NSCalibratedBlackColorSpace;
    }
  else if (depth & _GSRGBBitValue)
    {
      colorSpace = NSCalibratedRGBColorSpace;
    }
  else if (depth & _GSCMYKBitValue)
    {
      colorSpace = NSDeviceCMYKColorSpace;
    }
  else if (depth & _GSGrayBitValue)
    {
      colorSpace = NSCalibratedWhiteColorSpace;
    }
  else if (depth & _GSNamedBitValue)
    {
      colorSpace = NSNamedColorSpace;
    }
  else if (depth & _GSCustomBitValue)
    {
      colorSpace = NSCustomColorSpace;
    }

  return colorSpace;
}

NSInteger
NSNumberOfColorComponents(NSString *colorSpaceName)
{
  NSInteger components = 1;
  
  /*
   * These are the only exceptions to the above.
   * All other colorspaces have as many bps as bpp.
   */
  if ([colorSpaceName isEqualToString: NSCalibratedRGBColorSpace]
    || [colorSpaceName isEqualToString: NSDeviceRGBColorSpace])
    {
      components = 3;
    }
  else if ([colorSpaceName isEqualToString: NSDeviceCMYKColorSpace])
    {
      components = 4;
    }
  return components;
}

BOOL
NSPlanarFromDepth(NSWindowDepth depth)
{
  BOOL planar = NO;
  
  /*
   * Only the grayscale depths are planar.
   * All others are interleaved.
   */
  if (depth & _GSGrayBitValue)
    {
      planar = YES;
    }
  return planar;
}

/* Graphic Ops */
NSColor* NSReadPixel(NSPoint location)
{
  NSLog(@"NSReadPixel not implemented");
  return nil;
}

void NSCopyBitmapFromGState(int srcGstate, NSRect srcRect, NSRect destRect)
{
  NSLog(@"NSCopyBitmapFromGState not implemented");
}

void NSCopyBits(NSInteger srcGstate, NSRect srcRect, NSPoint destPoint)
{
  CGFloat x, y, w, h;
  NSGraphicsContext *ctxt = GSCurrentContext();

  x = NSMinX(srcRect);
  y = NSMinY(srcRect);
  w = NSWidth(srcRect);
  h = NSHeight(srcRect);

  DPScomposite(ctxt, x, y, w, h, srcGstate, destPoint.x, destPoint.y,
	       NSCompositeCopy);
}

void NSDrawBitmap(NSRect rect,
                  NSInteger pixelsWide,
                  NSInteger pixelsHigh,
                  NSInteger bitsPerSample,
                  NSInteger samplesPerPixel,
                  NSInteger bitsPerPixel,
                  NSInteger bytesPerRow,
                  BOOL isPlanar,
                  BOOL hasAlpha,
                  NSString *colorSpaceName,
                  const unsigned char *const data[5])
{
  NSBitmapImageRep *bitmap;
  NSGraphicsContext *ctxt = GSCurrentContext();

  bitmap = [[NSBitmapImageRep alloc] 
               initWithBitmapDataPlanes: (unsigned char **)data
               pixelsWide: pixelsWide
               pixelsHigh: pixelsHigh
               bitsPerSample: bitsPerSample
               samplesPerPixel: samplesPerPixel
               hasAlpha: hasAlpha
               isPlanar: isPlanar
               colorSpaceName: colorSpaceName
               bytesPerRow: bytesPerRow
               bitsPerPixel: bitsPerPixel];

  [ctxt GSDrawImage: rect : bitmap];
  RELEASE(bitmap);
}

/**
 * Tiles an image in a rect, starting from the lower-left-hand corner
 */
static void GSDrawRepeatingImage2D(NSRect aRect, NSImage *image, NSCompositingOperation op,
				   CGFloat fraction, BOOL flipped)
{
  const NSSize imageSize = [image size];
  if (imageSize.width <= 0 ||
      imageSize.height <= 0)
    {
      return;
    }

  [NSGraphicsContext saveGraphicsState];
  NSRectClip(aRect); 
  
  {
    const NSInteger numHorizontal = ceil(aRect.size.width / imageSize.width);
    const NSInteger numVertical = ceil(aRect.size.height / imageSize.height);
    NSInteger x, y;
    
    if (numHorizontal > 0 && numVertical > 0)
      {
	for (x = 0; x < numHorizontal; x++)
	  {
	    for (y = 0; y < numVertical; y++)
	      {
		NSRect drawRect;
		drawRect.size = imageSize;
		drawRect.origin.x = aRect.origin.x + (x * imageSize.width);
		drawRect.origin.y = flipped ? (NSMaxY(aRect) - ((y + 1) * imageSize.height))
		  : (aRect.origin.y + (y * imageSize.height));

		[image drawInRect: drawRect
			 fromRect: NSZeroRect
			operation: op
			 fraction: fraction
		   respectFlipped: YES
			    hints: nil];
	      }
	  }
      }
  }

  [NSGraphicsContext restoreGraphicsState];
}

static void GSDrawRepeatingImage1D(NSRect aRect, NSImage *image, BOOL isVertical,
				   NSCompositingOperation op,
				   CGFloat fraction, BOOL flipped)
{
  const NSSize imageSize = [image size];
  if (imageSize.width <= 0 ||
      imageSize.height <= 0)
    {
      return;
    }

  [NSGraphicsContext saveGraphicsState];
  NSRectClip(aRect); 
  
  {
    const NSInteger num = isVertical ? 
      ceil(aRect.size.height / imageSize.height)
      : ceil(aRect.size.width / imageSize.width);

    NSInteger i;
    
    if (num > 0)
      {
	for (i = 0; i < num; i++)
	  {
	    NSRect drawRect;
	    if (isVertical) 
	      {
		drawRect.size = NSMakeSize(aRect.size.width, imageSize.height);
		drawRect.origin = NSMakePoint(aRect.origin.x,
					      flipped ? (NSMaxY(aRect) - ((i + 1) * imageSize.height))
					      : (aRect.origin.y + (i * imageSize.height)));
	      }
	    else 
	      {
		drawRect.size = NSMakeSize(imageSize.width, aRect.size.height);
		drawRect.origin = NSMakePoint(aRect.origin.x + (i * imageSize.width),
					      aRect.origin.y);
	      }

	    [image drawInRect: drawRect
		     fromRect: NSZeroRect
		    operation: op
		     fraction: fraction
	       respectFlipped: YES
			hints: nil];
	  }
      }
  }

  [NSGraphicsContext restoreGraphicsState];
}

void NSDrawThreePartImage(NSRect aRect, NSImage *start, NSImage *middle,
			  NSImage *end, BOOL isVertical, NSCompositingOperation op,
			  CGFloat fraction, BOOL flipped)
{
  NSRect startRect, middleRect, endRect;
  NSView *focusView = [NSView focusView];

  if (nil != focusView)
    {
      aRect = [focusView centerScanRect: aRect];
    }
  
  [NSGraphicsContext saveGraphicsState];

  // Protects against the case when the smallest source image is larger than aRect
  NSRectClip(aRect); 

  if (isVertical)
    {
      startRect.size = NSMakeSize(aRect.size.width, 
				  [start size].height);
      endRect.size = NSMakeSize(aRect.size.width,
				[end size].height);
      middleRect.size = NSMakeSize(aRect.size.width,
				   MAX(0, aRect.size.height - 
				       startRect.size.height -
				       endRect.size.height));
      
      endRect.origin = aRect.origin;
      middleRect.origin = NSMakePoint(aRect.origin.x, NSMaxY(endRect));
      startRect.origin = NSMakePoint(aRect.origin.x, NSMaxY(middleRect));
    }
  else
    {
      startRect.size = NSMakeSize([start size].width,
				  aRect.size.height);
      endRect.size = NSMakeSize([end size].width, 
				aRect.size.height);
      middleRect.size = NSMakeSize(MAX(0, aRect.size.width - 
				       startRect.size.width -
				       endRect.size.width),
				   aRect.size.height);
      
      startRect.origin = aRect.origin;
      middleRect.origin = NSMakePoint(NSMaxX(startRect), aRect.origin.y);
      endRect.origin = NSMakePoint(NSMaxX(middleRect), aRect.origin.y);
    }

  [start drawInRect: startRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];
  GSDrawRepeatingImage1D(middleRect, middle, isVertical, op, fraction, flipped);
  [end drawInRect: endRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];   

  [NSGraphicsContext restoreGraphicsState]; // Restore clipping region
}


void NSDrawNinePartImage(NSRect aRect, NSImage *topLeft, NSImage *topMiddle,
			 NSImage *topRight, NSImage *centerLeft,
			 NSImage *centerMiddle, NSImage *centerRight,
			 NSImage *bottomLeft, NSImage *bottomMiddle,
			 NSImage *bottomRight, NSCompositingOperation op,
			 CGFloat fraction, BOOL flipped)
{
  NSView *focusView = [NSView focusView];

  if (nil != focusView)
    {
      aRect = [focusView centerScanRect: aRect];
    }
  
  [NSGraphicsContext saveGraphicsState];

  // Protects against the case when the smallest source image is larger than aRect
  NSRectClip(aRect); 

  {
    NSRect topLeftRect;
    NSRect topMiddleRect;
    NSRect topRightRect;
    NSRect centerLeftRect;
    NSRect centerMiddleRect;
    NSRect centerRightRect;
    NSRect bottomLeftRect; 
    NSRect bottomMiddleRect;
    NSRect bottomRightRect;

    // These two images are the only sizes we use in addition to aRect
    topLeftRect.size = [topLeft size];
    bottomRightRect.size = [bottomRight size];

    // Fill in the rest of the sizes
    topMiddleRect.size = NSMakeSize(MAX(0, aRect.size.width - 
					topLeftRect.size.width - 
					bottomRightRect.size.width),
				    topLeftRect.size.height);
    topRightRect.size = NSMakeSize(bottomRightRect.size.width,
				   topLeftRect.size.height);
    centerLeftRect.size = NSMakeSize(topLeftRect.size.width,
				     MAX(0, aRect.size.height - 
					 topLeftRect.size.height - 
					 bottomRightRect.size.height));
    centerMiddleRect.size = NSMakeSize(topMiddleRect.size.width,
				       centerLeftRect.size.height);
    centerRightRect.size = NSMakeSize(topRightRect.size.width,
				      centerLeftRect.size.height);
    bottomLeftRect.size = NSMakeSize(topLeftRect.size.width,
				     bottomRightRect.size.height);
    bottomMiddleRect.size = NSMakeSize(centerMiddleRect.size.width,
				       bottomRightRect.size.height);

    // Now fill in the positions

    if (flipped)
      {
	topLeftRect.origin = aRect.origin;
	topMiddleRect.origin = NSMakePoint(NSMaxX(topLeftRect),
					   aRect.origin.y);
	topRightRect.origin = NSMakePoint(NSMaxX(aRect) - NSWidth(topRightRect),
					  aRect.origin.y);
	centerLeftRect.origin = NSMakePoint(aRect.origin.x,
					    NSMaxY(topLeftRect));
	centerMiddleRect.origin = NSMakePoint(NSMaxX(topLeftRect),
					      NSMaxY(topLeftRect));
	centerRightRect.origin = NSMakePoint(NSMaxX(topMiddleRect),
					     NSMaxY(topMiddleRect));
	bottomLeftRect.origin = NSMakePoint(aRect.origin.x,
					    NSMaxY(aRect) - NSHeight(bottomLeftRect));
	bottomMiddleRect.origin = NSMakePoint(NSMaxX(centerLeftRect),
					      NSMaxY(centerLeftRect));
	bottomRightRect.origin = NSMakePoint(NSMaxX(aRect) - NSWidth(bottomRightRect),
					     NSMaxY(aRect) - NSHeight(bottomRightRect));
      }
    else
      {
	bottomLeftRect.origin = aRect.origin;
	bottomMiddleRect.origin = NSMakePoint(NSMaxX(bottomLeftRect),
					      aRect.origin.y);
	bottomRightRect.origin = NSMakePoint(NSMaxX(aRect) - NSWidth(bottomRightRect),
					     aRect.origin.y);
	centerLeftRect.origin = NSMakePoint(aRect.origin.x,
					    NSMaxY(bottomLeftRect));
	centerMiddleRect.origin = NSMakePoint(NSMaxX(bottomLeftRect),
					      NSMaxY(bottomLeftRect));
	centerRightRect.origin = NSMakePoint(NSMaxX(bottomMiddleRect),
					     NSMaxY(bottomMiddleRect));
	topLeftRect.origin = NSMakePoint(aRect.origin.x,
					 NSMaxY(aRect) - NSHeight(topLeftRect));
	topMiddleRect.origin = NSMakePoint(NSMaxX(centerLeftRect),
					   NSMaxY(centerLeftRect));
	topRightRect.origin = NSMakePoint(NSMaxX(aRect) - NSWidth(topRightRect),
					  NSMaxY(aRect) - NSHeight(topRightRect));
      }
	
    // Draw the images left-to-right, bottom-to-top

    [bottomLeft drawInRect: bottomLeftRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];
    GSDrawRepeatingImage2D(bottomMiddleRect, bottomMiddle, op, fraction, flipped);
    [bottomRight drawInRect: bottomRightRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];   
    GSDrawRepeatingImage2D(centerLeftRect, centerLeft, op, fraction, flipped);
    GSDrawRepeatingImage2D(centerMiddleRect, centerMiddle, op, fraction, flipped);
    GSDrawRepeatingImage2D(centerRightRect, centerRight, op, fraction, flipped);
    [topLeft drawInRect: topLeftRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];
    GSDrawRepeatingImage2D(topMiddleRect, topMiddle, op, fraction, flipped);
    [topRight drawInRect: topRightRect fromRect: NSZeroRect operation: op fraction: fraction respectFlipped: YES hints: nil];
  }

  [NSGraphicsContext restoreGraphicsState]; // Restore clipping region
}

/*
 * Rectangle Drawing 
 */
void NSEraseRect(NSRect aRect)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSWhite);
  NSRectFill(aRect);
  DPSgrestore(ctxt);
}

void NSHighlightRect(NSRect aRect)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPScompositerect(ctxt, NSMinX(aRect), NSMinY(aRect), 
		   NSWidth(aRect), NSHeight(aRect), 
		   GSCompositeHighlight);
}

void NSRectClip(NSRect aRect)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSrectclip(ctxt, NSMinX(aRect), NSMinY(aRect), 
	      NSWidth(aRect), NSHeight(aRect));
  DPSnewpath(ctxt);
}

void NSRectClipList(const NSRect *rects, NSInteger count)
{
  NSInteger i;
  NSRect union_rect;

  if (count == 0)
    return;

  /* 
     The specification is not clear if the union of the rects 
     should produce the new clip rect or if the outline of all rects 
     should be used as clip path.
  */
  union_rect = rects[0];
  for (i = 1; i < count; i++)
    union_rect = NSUnionRect(union_rect, rects[i]);

  NSRectClip(union_rect);
}

void NSRectFill(NSRect aRect)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSrectfill(ctxt, NSMinX(aRect), NSMinY(aRect), 
	      NSWidth(aRect), NSHeight(aRect));
}

void NSRectFillList(const NSRect *rects, NSInteger count)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  GSRectFillList(ctxt, rects, count);
}

void 
NSRectFillListWithColors(const NSRect *rects, NSColor **colors, NSInteger count)
{
  NSInteger i;
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);

  for (i = 0; i < count; i++)
    {
      [colors[i] set];
      NSRectFill(rects[i]);
    }

  DPSgrestore(ctxt);
}

void NSRectFillListWithGrays(const NSRect *rects, const CGFloat *grays, 
			     NSInteger count)
{
  NSInteger i;
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);

  for (i = 0; i < count; i++)
    {
      DPSsetgray(ctxt, grays[i]);
      DPSrectfill(ctxt,  NSMinX(rects[i]), NSMinY(rects[i]), 
		  NSWidth(rects[i]), NSHeight(rects[i]));
    }

  DPSgrestore(ctxt);
}

void NSRectFillUsingOperation(NSRect aRect, NSCompositingOperation op)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPScompositerect(ctxt, NSMinX(aRect), NSMinY(aRect), 
		   NSWidth(aRect), NSHeight(aRect), op);
}


void 
NSRectFillListUsingOperation(const NSRect *rects, NSInteger count, 
			     NSCompositingOperation op)
{
  NSInteger i;

  for (i = 0; i < count; i++)
    {
      NSRectFillUsingOperation(rects[i], op);
    }
}

void 
NSRectFillListWithColorsUsingOperation(const NSRect *rects, 
				       NSColor **colors, 
				       NSInteger num, 
				       NSCompositingOperation op)
{
  NSInteger i;
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);

  for (i = 0; i < num; i++)
    {
      [colors[i] set];
      NSRectFillUsingOperation(rects[i], op);
    }

  DPSgrestore(ctxt);
}


/* Various functions for drawing bordered rectangles.  */

void NSDottedFrameRect(const NSRect aRect)
{
  CGFloat dot_dash[] = {1.0, 1.0};
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSBlack);
  DPSsetlinewidth(ctxt, 1.0);
  // FIXME
  DPSsetdash(ctxt, dot_dash, 2, 0.0);
  DPSrectstroke(ctxt,  NSMinX(aRect) + 0.5, NSMinY(aRect) + 0.5,
		NSWidth(aRect) - 1.0, NSHeight(aRect) - 1.0);
  DPSgrestore(ctxt);
}

void NSFrameRect(const NSRect aRect)
{
  NSFrameRectWithWidth(aRect, 1.0);
}

void NSFrameRectWithWidth(const NSRect aRect, CGFloat frameWidth)
{
  NSRectEdge sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRect remainder = aRect;
  NSRect rects[4];
  int i;

  if (frameWidth == 0.0)
    {
      NSView *view = [GSCurrentContext() focusView];
      NSSize aSize = [view convertSize: NSMakeSize(1.0, 1.0) fromView: nil];

      frameWidth = (aSize.width + aSize.height) / 2.0;
    }

  for (i = 0; i < 4; i++) 
    {
      NSDivideRect(remainder, &rects[i], &remainder, frameWidth, sides[i]);
    }
  NSRectFillList(rects, 4);
}

void 
NSFrameRectWithWidthUsingOperation(NSRect aRect, CGFloat frameWidth, 
				   NSCompositingOperation op)
{
  NSRectEdge sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRect remainder = aRect;
  NSRect rects[4];
  int i;

  if (frameWidth == 0.0)
    {
      NSView *view = [GSCurrentContext() focusView];
      NSSize aSize = [view convertSize: NSMakeSize(1.0, 1.0) fromView: nil];

      frameWidth = (aSize.width + aSize.height) / 2.0;
    }

  for (i = 0; i < 4; i++) 
    {
      NSDivideRect(remainder, &rects[i], &remainder, frameWidth, sides[i]);
    }
  NSRectFillListUsingOperation(rects, 4, op);
}

NSRect 
NSDrawTiledRects(NSRect aRect, const NSRect clipRect,
		 const NSRectEdge *sides,
		 const CGFloat *grays, NSInteger count)
{
  NSInteger i;
  NSRect slice;
  NSRect remainder = aRect;
  NSRect rects[count];
  BOOL hasClip = !NSIsEmptyRect(clipRect);

  if (hasClip && NSIntersectsRect(aRect, clipRect) == NO)
    return remainder;

  for (i = 0; i < count; i++)
    {
      NSDivideRect(remainder, &slice, &remainder, 1.0, sides[i]);
      if (hasClip)
	rects[i] = NSIntersectionRect(slice, clipRect);
      else
	rects[i] = slice;
    }

  NSRectFillListWithGrays(rects, grays, count);

  return remainder;
}

NSRect 
NSDrawColorTiledRects(NSRect boundsRect, NSRect clipRect, 
		      const NSRectEdge *sides, NSColor **colors, 
		      NSInteger count)
{
  NSInteger i;
  NSRect slice;
  NSRect remainder = boundsRect;
  NSRect rects[count];
  BOOL hasClip = !NSIsEmptyRect(clipRect);

  if (hasClip && NSIntersectsRect(boundsRect, clipRect) == NO)
    return remainder;

  for (i = 0; i < count; i++)
    {
      NSDivideRect(remainder, &slice, &remainder, 1.0, sides[i]);
      if (hasClip)
	rects[i] = NSIntersectionRect(slice, clipRect);
      else
	rects[i] = slice;
    }

  NSRectFillListWithColors(rects, colors, count);

  return remainder;
}

void
NSDrawButton(const NSRect aRect, const NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge, 
			   NSMaxXEdge, NSMinYEdge};
  NSRectEdge down_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			     NSMinXEdge, NSMinYEdge, 
			     NSMaxXEdge, NSMaxYEdge};
  CGFloat grays[] = {NSBlack, NSBlack, 
                     NSWhite, NSWhite, 
                     NSDarkGray, NSDarkGray};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 6);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 6);
    }
  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSLightGray);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void
NSDrawGrayBezel(const NSRect aRect, const NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge,
			   NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRectEdge down_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge,
			     NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge};
  CGFloat grays[] = {NSWhite, NSWhite, NSDarkGray, NSDarkGray,
                     NSLightGray, NSLightGray, NSBlack, NSBlack};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();

  DPSgsave(ctxt);

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 8);
      // to give a really clean look we add 2 dark gray points
      DPSsetgray(ctxt, NSDarkGray);
      DPSrectfill(ctxt, NSMinX(aRect) + 1., NSMaxY(aRect) - 2., 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 2., NSMinY(aRect) + 1., 1., 1.);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 8);
      // to give a really clean look we add 2 dark gray points
      DPSsetgray(ctxt, NSDarkGray);
      DPSrectfill(ctxt, NSMinX(aRect) + 1., NSMinY(aRect) + 1., 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 2., NSMaxY(aRect) - 2., 1., 1.);
    }

  DPSsetgray(ctxt, NSLightGray);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void 
NSDrawGroove(const NSRect aRect, const NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMinXEdge, NSMaxYEdge, NSMinXEdge, NSMaxYEdge, 
			   NSMaxXEdge, NSMinYEdge, NSMaxXEdge, NSMinYEdge};
  NSRectEdge down_sides[] = {NSMinXEdge, NSMinYEdge, NSMinXEdge, NSMinYEdge, 
			     NSMaxXEdge, NSMaxYEdge, NSMaxXEdge, NSMaxYEdge};
  CGFloat grays[] = {NSDarkGray, NSDarkGray, NSWhite, NSWhite,
                     NSWhite, NSWhite, NSDarkGray, NSDarkGray};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 8);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 8);
    }

  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSLightGray);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void 
NSDrawWhiteBezel(const NSRect aRect,  const NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge,
  			   NSMaxYEdge, NSMaxXEdge, NSMinYEdge, NSMinXEdge};
  NSRectEdge down_sides[] = {NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge, 
  			     NSMinYEdge, NSMaxXEdge, NSMaxYEdge, NSMinXEdge};
  CGFloat grays[] = {NSDarkGray, NSWhite, NSWhite, NSDarkGray, 
                     NSDarkGray, NSLightGray, NSLightGray, NSDarkGray};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 8);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 8);
    }

  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSWhite);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void 
NSDrawDarkBezel(NSRect aRect, NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge,
			   NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRectEdge down_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge,
			     NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge};
  // FIXME: The actual colour used for the 3 + 4 line 
  // (and the two additional points) is a bit darker. 
  CGFloat grays[] = {NSWhite, NSWhite, NSLightGray, NSLightGray,
                     NSLightGray, NSLightGray, NSBlack, NSBlack};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 8);
      // to give a really clean look we add 2 light gray points
      DPSsetgray(ctxt, NSLightGray);
      DPSrectfill(ctxt, NSMinX(aRect) + 1., NSMaxY(aRect) - 2., 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 2., NSMinY(aRect) + 1., 1., 1.);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 8);
      // to give a really clean look we add 2 light gray points
      DPSsetgray(ctxt, NSLightGray);
      DPSrectfill(ctxt, NSMinX(aRect) + 1., NSMinY(aRect) + 1., 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 2., NSMaxY(aRect) - 2., 1., 1.);
    }

  DPSsetgray(ctxt, NSLightGray);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void 
NSDrawLightBezel(NSRect aRect, NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge, 
  			   NSMaxXEdge, NSMinYEdge, NSMinXEdge, NSMaxYEdge};
  NSRectEdge down_sides[] = {NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge, 
  			     NSMaxXEdge, NSMaxYEdge, NSMinXEdge, NSMinYEdge};
  CGFloat grays[] = {NSWhite, NSWhite, NSGray, NSGray,
                     NSBlack, NSBlack, NSBlack, NSBlack};
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();
  DPSgsave(ctxt);

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 8);
      // to give a really clean look we add 2 light gray points
      DPSsetgray(ctxt, NSLightGray);
      DPSrectfill(ctxt, NSMinX(aRect), NSMaxY(aRect) - 1., 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 1., NSMinY(aRect), 1., 1.);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 8);
      // to give a really clean look we add 2 light gray points
      DPSsetgray(ctxt, NSLightGray);
      DPSrectfill(ctxt, NSMinX(aRect), NSMinY(aRect), 1., 1.);
      DPSrectfill(ctxt, NSMaxX(aRect) - 1., NSMaxY(aRect) - 1., 1., 1.);
    }

  DPSsetgray(ctxt, NSWhite);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void
NSDrawFramePhoto(const NSRect aRect, const NSRect clipRect)
{
  NSRectEdge up_sides[] = {NSMaxXEdge, NSMinYEdge, 
			   NSMinXEdge, NSMaxYEdge, 
			   NSMaxXEdge, NSMinYEdge};
  NSRectEdge down_sides[] = {NSMaxXEdge, NSMaxYEdge, 
			     NSMinXEdge, NSMinYEdge, 
			     NSMaxXEdge, NSMaxYEdge};
  CGFloat grays[] = {NSDarkGray, NSDarkGray, 
                     NSDarkGray, NSDarkGray,
                     NSBlack, NSBlack};
 
  NSRect rect;
  NSGraphicsContext *ctxt = GSCurrentContext();

  if (GSWViewIsFlipped(ctxt) == YES)
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       down_sides, grays, 6);
    }
  else
    {
      rect = NSDrawTiledRects(aRect, clipRect,
			       up_sides, grays, 6);
    }

  DPSgsave(ctxt);
  DPSsetgray(ctxt, NSLightGray);
  DPSrectfill(ctxt, NSMinX(rect), NSMinY(rect), 
	      NSWidth(rect), NSHeight(rect));
  DPSgrestore(ctxt);
}

void 
NSDrawWindowBackground(NSRect aRect)
{
  NSGraphicsContext *ctxt = GSCurrentContext();  
  DPSgsave(ctxt);
  [[NSColor windowBackgroundColor] set];
  NSRectFill(aRect);
  DPSgrestore(ctxt);
}

CGFloat 
NSLinkFrameThickness(void)
{
  return 1.0;
}

void 
NSFrameLinkRect(NSRect aRect, BOOL isDestination)
{
  NSGraphicsContext *ctxt = GSCurrentContext();  
  DPSgsave(ctxt);

  if (isDestination)
    {
      [[NSColor redColor] set];
    }
  else
    {
      [[NSColor greenColor] set];
    }

  NSFrameRectWithWidth(aRect, NSLinkFrameThickness());
  DPSgrestore(ctxt);
}

void NSSetFocusRingStyle(NSFocusRingPlacement placement)
{
  // FIXME: NIMP
  NSLog(@"*** NSSetFocusRingStyle not implemented ***");
}

void 
NSConvertGlobalToWindowNumber(int globalNum, unsigned int *winNum)
{
  NSArray *windows = GSAllWindows();
  NSUInteger count = [windows count];
  NSUInteger i;

  for (i = 0; i < count; i++)
    {
      NSWindow *win = [windows objectAtIndex: i];

      if (((int)(intptr_t)[win windowRef]) == globalNum)
        {
          *winNum = [win windowNumber];
          return;
        }
    }
  *winNum = 0;
}

void 
NSConvertWindowNumberToGlobal(int winNum, unsigned int *globalNum)
{
  *globalNum = (int)(intptr_t)[GSWindowWithNumber(winNum) windowRef];
}

void 
NSCountWindowsForContext(NSInteger context, NSInteger *count)
{
// TODO
  *count = 0;
}

void 
NSShowSystemInfoPanel(NSDictionary *options)
{
  [NSApp orderFrontStandardInfoPanelWithOptions: options];
}

void 
NSWindowListForContext(NSInteger context, NSInteger size, NSInteger **list)
{
// TODO
}

int 
NSGetWindowServerMemory(int context, int *virtualMemory, 
			int *windowBackingMemory, NSString **windowDumpStream)
{
// TODO
  return -1;
}
