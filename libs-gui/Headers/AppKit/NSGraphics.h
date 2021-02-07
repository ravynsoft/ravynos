/*
   NSGraphics.h

   Copyright (C) 1996, 2005 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: February 1997
   
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

#ifndef __NSGraphics_h__
#define __NSGraphics_h__
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>

#import <AppKit/NSGraphicsContext.h>
#import <AppKit/AppKitDefines.h>

#if defined(__cplusplus)
extern "C" {
#endif

@class NSString;
@class NSColor;
@class NSGraphicsContext;

/*
 * Colorspace Names 
 */
APPKIT_EXPORT NSString *NSCalibratedWhiteColorSpace; 
APPKIT_EXPORT NSString *NSCalibratedBlackColorSpace; 
APPKIT_EXPORT NSString *NSCalibratedRGBColorSpace;
APPKIT_EXPORT NSString *NSDeviceWhiteColorSpace;
APPKIT_EXPORT NSString *NSDeviceBlackColorSpace;
APPKIT_EXPORT NSString *NSDeviceRGBColorSpace;
APPKIT_EXPORT NSString *NSDeviceCMYKColorSpace;
APPKIT_EXPORT NSString *NSNamedColorSpace;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
APPKIT_EXPORT NSString *NSPatternColorSpace;
#endif
APPKIT_EXPORT NSString *NSCustomColorSpace;


/*
 * Color function APPKIT_EXPORTs
 */
APPKIT_EXPORT const NSWindowDepth _GSGrayBitValue;
APPKIT_EXPORT const NSWindowDepth _GSRGBBitValue;
APPKIT_EXPORT const NSWindowDepth _GSCMYKBitValue;
APPKIT_EXPORT const NSWindowDepth _GSCustomBitValue;
APPKIT_EXPORT const NSWindowDepth _GSNamedBitValue;
APPKIT_EXPORT const NSWindowDepth *_GSWindowDepths[7];
APPKIT_EXPORT const NSWindowDepth NSDefaultDepth;
APPKIT_EXPORT const NSWindowDepth NSTwoBitGrayDepth;
APPKIT_EXPORT const NSWindowDepth NSEightBitGrayDepth;
APPKIT_EXPORT const NSWindowDepth NSEightBitRGBDepth;
APPKIT_EXPORT const NSWindowDepth NSTwelveBitRGBDepth;
APPKIT_EXPORT const NSWindowDepth GSSixteenBitRGBDepth;
APPKIT_EXPORT const NSWindowDepth NSTwentyFourBitRGBDepth;

/*
 * Gray Values 
 */
APPKIT_EXPORT const CGFloat NSBlack;
APPKIT_EXPORT const CGFloat NSDarkGray;
APPKIT_EXPORT const CGFloat NSWhite;
APPKIT_EXPORT const CGFloat NSLightGray;
APPKIT_EXPORT const CGFloat NSGray;

/*
 * Device Dictionary Keys 
 */
APPKIT_EXPORT NSString *NSDeviceResolution;
APPKIT_EXPORT NSString *NSDeviceColorSpaceName;
APPKIT_EXPORT NSString *NSDeviceBitsPerSample;
APPKIT_EXPORT NSString *NSDeviceIsScreen;
APPKIT_EXPORT NSString *NSDeviceIsPrinter;
APPKIT_EXPORT NSString *NSDeviceSize;

/*
 * Get Information About Color Space and Window Depth
 */
APPKIT_EXPORT const NSWindowDepth *NSAvailableWindowDepths(void);
APPKIT_EXPORT NSWindowDepth NSBestDepth(NSString *colorSpace, 
			  NSInteger bitsPerSample, NSInteger bitsPerPixel, 
			  BOOL planar, BOOL *exactMatch);
APPKIT_EXPORT NSInteger NSBitsPerPixelFromDepth(NSWindowDepth depth);
APPKIT_EXPORT NSInteger NSBitsPerSampleFromDepth(NSWindowDepth depth);
APPKIT_EXPORT NSString *NSColorSpaceFromDepth(NSWindowDepth depth);
APPKIT_EXPORT NSInteger NSNumberOfColorComponents(NSString *colorSpaceName);
APPKIT_EXPORT BOOL NSPlanarFromDepth(NSWindowDepth depth);


/*
 * Functions for getting information about windows.
 */
APPKIT_EXPORT void NSCountWindows(NSInteger *count);
APPKIT_EXPORT void NSWindowList(NSInteger size, NSInteger list[]);

APPKIT_EXPORT void NSEraseRect(NSRect aRect);
APPKIT_EXPORT void NSHighlightRect(NSRect aRect);
APPKIT_EXPORT void NSRectClip(NSRect aRect);
APPKIT_EXPORT void NSRectClipList(const NSRect *rects, NSInteger count);
APPKIT_EXPORT void NSRectFill(NSRect aRect);
APPKIT_EXPORT void NSRectFillList(const NSRect *rects, NSInteger count);
APPKIT_EXPORT void NSRectFillListWithGrays(const NSRect *rects,
					   const CGFloat *grays,
                                           NSInteger count);

/** Draws a set of edges of aRect.  The sides array should contain
    count edges, and grays the corresponding color.  Edges are drawn
    in the order given in the array, and subsequent edges are drawn
    inside previous edges (thus, they will never overlap).  */
APPKIT_EXPORT NSRect NSDrawTiledRects(NSRect aRect, const NSRect clipRect,
                                      const NSRectEdge *sides,
                                      const CGFloat *grays,
                                      NSInteger count);

APPKIT_EXPORT void NSDrawButton(const NSRect aRect, const NSRect clipRect);
APPKIT_EXPORT void NSDrawGrayBezel(const NSRect aRect, const NSRect clipRect);
APPKIT_EXPORT void NSDrawGroove(const NSRect aRect, const NSRect clipRect);
APPKIT_EXPORT void NSDrawWhiteBezel(const NSRect aRect, const NSRect clipRect);
APPKIT_EXPORT void NSDrawFramePhoto(const NSRect aRect, const NSRect clipRect);

// This is from an old version of the specification 
static inline void
NSDrawBezel(const NSRect aRect, const NSRect clipRect)
{
  NSDrawGrayBezel(aRect, clipRect);
}

/** Draws a rectangle along the inside of aRect.  The rectangle will be
    black, dotted (using 1 point dashes), and will have a line width
    of 1 point.  */
APPKIT_EXPORT void NSDottedFrameRect(NSRect aRect);
/** <p>Draws a rectangle using the current color along the inside of aRect.
    NSFrameRectWithWidth uses the frameWidth as the line width, while
    NSFrameRect always uses 1 point wide lines.  The functions do not
    change the line width of the current graphics context.
    </p><p>
    'Inside' here means that no part of the stroked rectangle will extend
    outside the given rectangle.
    </p>  */
APPKIT_EXPORT void NSFrameRect(const NSRect aRect); 
APPKIT_EXPORT void NSFrameRectWithWidth(const NSRect aRect, CGFloat frameWidth);
APPKIT_EXPORT void NSFrameRectWithWidthUsingOperation(const NSRect aRect, CGFloat frameWidth, 
						      NSCompositingOperation op);

APPKIT_EXPORT NSColor* NSReadPixel(NSPoint location);

APPKIT_EXPORT void NSCopyBitmapFromGState(int srcGstate, NSRect srcRect, 
					  NSRect destRect);
APPKIT_EXPORT void NSCopyBits(NSInteger srcGstate, NSRect srcRect, 
			      NSPoint destPoint);

APPKIT_EXPORT void NSDrawBitmap(NSRect rect,
                                NSInteger pixelsWide,
                                NSInteger pixelsHigh,
                                NSInteger bitsPerSample,
                                NSInteger samplesPerPixel,
                                NSInteger bitsPerPixel,
                                NSInteger bytesPerRow,
                                BOOL isPlanar,
                                BOOL hasAlpha,
                                NSString *colorSpaceName,
                                const unsigned char *const data[5]);

APPKIT_EXPORT void
NSDrawThreePartImage(NSRect aRect, NSImage *start, NSImage *middle,
		     NSImage *end, BOOL isVertical, NSCompositingOperation op,
		     CGFloat fraction, BOOL flipped);

APPKIT_EXPORT void
NSDrawNinePartImage(NSRect aRect, NSImage *topLeft, NSImage *topMiddle,
		    NSImage *topRight, NSImage *centerLeft,
		    NSImage *centerMiddle, NSImage *centerRight,
		    NSImage *bottomLeft, NSImage *bottomMiddle,
		    NSImage *bottomRight, NSCompositingOperation op,
		    CGFloat fraction, BOOL flipped);

static inline void
NSBeep(void)
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  if (ctxt != nil) {
    (ctxt->methods->NSBeep)
      (ctxt, @selector(NSBeep));
  }
}

static inline void
GSWSetViewIsFlipped(NSGraphicsContext *ctxt, BOOL flipped)
{
  (ctxt->methods->GSWSetViewIsFlipped_)
    (ctxt, @selector(GSWSetViewIsFlipped:), flipped);
}

static inline BOOL
GSWViewIsFlipped(NSGraphicsContext *ctxt)
{
  return (ctxt->methods->GSWViewIsFlipped)
    (ctxt, @selector(GSWViewIsFlipped));
}

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
@class	NSArray;
@class	NSWindow;

APPKIT_EXPORT NSArray* GSOrderedWindows(void);
APPKIT_EXPORT NSArray* GSAllWindows(void);
APPKIT_EXPORT NSWindow* GSWindowWithNumber(NSInteger num);
#endif

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
// Window operations
APPKIT_EXPORT void NSConvertGlobalToWindowNumber(int globalNum, unsigned int *winNum);
APPKIT_EXPORT void NSConvertWindowNumberToGlobal(int winNum, unsigned int *globalNum);

// Rectangle drawing
APPKIT_EXPORT NSRect NSDrawColorTiledRects(NSRect boundsRect, NSRect clipRect, 
                                           const NSRectEdge *sides, 
                                           NSColor **colors, 
                                           NSInteger count);
APPKIT_EXPORT void NSDrawDarkBezel(NSRect aRect, NSRect clipRect);
APPKIT_EXPORT void NSDrawLightBezel(NSRect aRect, NSRect clipRect);
APPKIT_EXPORT void NSRectFillListWithColors(const NSRect *rects, 
                                            NSColor **colors, NSInteger count);

APPKIT_EXPORT void NSRectFillUsingOperation(NSRect aRect, 
					     NSCompositingOperation op);
APPKIT_EXPORT void NSRectFillListUsingOperation(const NSRect *rects, 
                                                NSInteger count, 
                                                NSCompositingOperation op);
APPKIT_EXPORT void NSRectFillListWithColorsUsingOperation(const NSRect *rects,
                                                          NSColor **colors, 
                                                          NSInteger num, 
                                                          NSCompositingOperation op);

APPKIT_EXPORT void NSDrawWindowBackground(NSRect aRect);

// Context information
APPKIT_EXPORT void NSCountWindowsForContext(NSInteger context, NSInteger *count);
APPKIT_EXPORT void NSWindowListForContext(NSInteger context, NSInteger size, NSInteger **list);
APPKIT_EXPORT int NSGetWindowServerMemory(int context, int *virtualMemory, 
                                          int *windowBackingMemory, 
                                          NSString **windowDumpStream);

#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_1, GS_API_LATEST)
typedef enum _NSFocusRingPlacement
{
    NSFocusRingOnly=0,
    NSFocusRingBelow,
    NSFocusRingAbove
} NSFocusRingPlacement;

void NSSetFocusRingStyle(NSFocusRingPlacement placement);
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __NSGraphics_h__ */
