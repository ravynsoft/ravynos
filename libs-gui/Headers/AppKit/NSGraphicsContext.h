/** <title>NSGraphicsContext</title>

   <abstract>Abstract drawing context class.</abstract>

   Copyright (C) 1998,1999 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: Feb 1999
   Based on code by:  Adam Fedor <fedor@gnu.org>
   Date: Nov 1998
   
   This file is part of the GNU Objective C User interface library.

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

#ifndef _NSGraphicsContext_h_INCLUDE
#define _NSGraphicsContext_h_INCLUDE
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSMapTable.h>

#import <AppKit/AppKitDefines.h>

@class NSDate;
@class NSDictionary;
@class NSMutableArray;
@class NSMutableData;
@class NSMutableSet;
@class NSString;
@class NSView;
@class NSWindow;
@class NSFont;
@class NSSet;
@class NSBitmapImageRep;
@class NSGradient;

typedef struct CGContext *CGContextRef;

/*
 * Backing Store Types
 */
enum _NSBackingStoreType
{
  NSBackingStoreRetained,
  NSBackingStoreNonretained,
  NSBackingStoreBuffered

};
typedef NSUInteger NSBackingStoreType;

/** NSCompositingOperation defines how an image is drawn or composited onto another.
 * <list>
 *  <item>NSCompositeClear: Cleans out an area</item>
 *  <item> NSCompositeCopy: <em>(Common)</em> Draws over the destination replacing it and carrying over transparency to the destination</item>
 *  <item> NSCompositeSourceOver: <em>(Common)</em> Draws over the destination with transparency</item>
 *  <item> NSCompositeSourceIn</item>
 *  <item> NSCompositeSourceOut</item>
 *  <item> NSCompositeSourceAtop</item>
 *  <item> NSCompositeDestinationOver</item>
 *  <item> NSCompositeDestinationIn</item>
 *  <item> NSCompositeDestinationOut</item>
 *  <item> NSCompositeDestinationAtop</item>
 *  <item> NSCompositeXOR</item>
 *  <item> NSCompositePlusDarker</item>
 *  <item> NSCompositeHighlight</item>
 *  <item> NSCompositePlusLighter</item>
 * </list>
 */
enum _NSCompositingOperation
{
 
  NSCompositeClear,
  NSCompositeCopy,
  NSCompositeSourceOver,
  NSCompositeSourceIn,
  NSCompositeSourceOut,
  NSCompositeSourceAtop,
  NSCompositeDestinationOver,
  NSCompositeDestinationIn,
  NSCompositeDestinationOut,
  NSCompositeDestinationAtop,
  NSCompositeXOR,
  NSCompositePlusDarker,
  NSCompositeHighlight,
  NSCompositePlusLighter

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
  , GSCompositeHighlight = 100
#endif
};
typedef NSUInteger NSCompositingOperation;

typedef int NSWindowDepth;

/* Image interpolation */
typedef enum _NSImageInterpolation
{
  NSImageInterpolationDefault,
  NSImageInterpolationNone,
  NSImageInterpolationLow,
  NSImageInterpolationHigh
} NSImageInterpolation;


/*
 * The following graphics context stuff is needed by inline functions,
 * so it must always be available even when compiling for MACOSX or OPENSTEP
 */


typedef enum _GSTextDrawingMode
{
  GSTextFill,
  GSTextStroke,
  GSTextClip
} GSTextDrawingMode;

// We have to load this after the NSCompositingOperation are defined!!!
#import <GNUstepGUI/GSMethodTable.h>

/*
 * Window ordering
 */
typedef enum _NSWindowOrderingMode
{
  NSWindowAbove,
  NSWindowBelow,
  NSWindowOut

} NSWindowOrderingMode;

/*
 * Window input state
 */
typedef enum _GSWindowInputState
{
  GSTitleBarKey = 0,
  GSTitleBarNormal = 1,
  GSTitleBarMain = 2

} GSWindowInputState;

/* Color spaces */
typedef enum _GSColorSpace
{
  GSDeviceGray,
  GSDeviceRGB,
  GSDeviceCMYK,
  GSCalibratedGray,
  GSCalibratedRGB,
  GSCIELab,
  GSICC
} GSColorSpace;

@interface NSGraphicsContext : NSObject
{
  /* Make the one public instance variable first in the object so that, if we
   * add or remove others, we don't necessarily need to recompile everything.
   */
@public
  const gsMethodTable	*methods;

@protected
  NSDictionary		*context_info;
  NSMutableData		*context_data;
  NSMutableArray	*focus_stack;
  NSMutableSet          *usedFonts;
  NSImageInterpolation  _interp;
  BOOL                  _antialias;
  NSPoint _patternPhase;
  void *_graphicsPort;
  BOOL _isFlipped;
  NSCompositingOperation _compositingOperation;
}

+ (BOOL) currentContextDrawingToScreen;
+ (NSGraphicsContext *) graphicsContextWithAttributes: (NSDictionary *)attributes;
+ (NSGraphicsContext *) graphicsContextWithWindow: (NSWindow *)aWindow;

+ (void) restoreGraphicsState;
+ (void) saveGraphicsState;
+ (void) setGraphicsState: (NSInteger)graphicsState;
+ (void) setCurrentContext: (NSGraphicsContext*)context;
+ (NSGraphicsContext*) currentContext;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
+ (NSGraphicsContext *) graphicsContextWithBitmapImageRep: (NSBitmapImageRep *)bitmap;
+ (NSGraphicsContext *) graphicsContextWithGraphicsPort: (void *)port 
                                                flipped: (BOOL)flag;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_10, GS_API_LATEST)
- (CGContextRef) CGContext;
+ (NSGraphicsContext *) graphicsContextWithCGContext: (CGContextRef)ctx
                                             flipped: (BOOL)flipped;
#endif

- (NSDictionary *) attributes;
- (void *) graphicsPort;

- (BOOL) isDrawingToScreen;
- (void) flushGraphics;
- (void) restoreGraphicsState;
- (void) saveGraphicsState;

- (void *) focusStack;
- (void) setFocusStack: (void *)stack;

- (void) setImageInterpolation: (NSImageInterpolation)interpolation;
- (NSImageInterpolation) imageInterpolation;
- (void) setShouldAntialias: (BOOL)antialias;
- (BOOL) shouldAntialias;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSPoint) patternPhase;
- (void) setPatternPhase: (NSPoint)phase;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (NSCompositingOperation) compositingOperation;
- (void) setCompositingOperation:(NSCompositingOperation) operation;

- (BOOL) isFlipped;
#endif

@end

APPKIT_EXPORT NSGraphicsContext	*GSCurrentContext(void);

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)

@interface NSGraphicsContext (GNUstep)
+ (void) setDefaultContextClass: (Class)defaultContextClass;

- (id) initWithContextInfo: (NSDictionary*)info;
- (id) initWithGraphicsPort: (void *)port 
                    flipped: (BOOL)flag;

/*
 * Focus management methods - lock and unlock should only be used by NSView
 * in it's implementation of lockFocus and unlockFocus.
 */
- (NSView*) focusView;
- (void) lockFocusView: (NSView*)aView inRect: (NSRect)rect;
- (void) unlockFocusView: (NSView*)aView needsFlush: (BOOL)flush;

/* Private methods for printing */
- (void) useFont: (NSString *)fontName;
- (void) resetUsedFonts;
- (NSSet *) usedFonts;

/* Private backend methods */
+ (void) handleExposeRect: (NSRect)rect forDriver: (void *)driver;
@end
#endif


/*
 *	GNUstep drawing engine extensions - these are the methods actually
 *	called when one of the inline PostScript functions (like PSlineto())
 *	is called.
 */
@interface NSGraphicsContext (Ops)
/* ----------------------------------------------------------------------- */
/* Color operations */
/* ----------------------------------------------------------------------- */
- (void) DPScurrentalpha: (CGFloat*)a;
- (void) DPScurrentcmykcolor: (CGFloat*)c : (CGFloat*)m : (CGFloat*)y : (CGFloat*)k;
- (void) DPScurrentgray: (CGFloat*)gray;
- (void) DPScurrenthsbcolor: (CGFloat*)h : (CGFloat*)s : (CGFloat*)b;
- (void) DPScurrentrgbcolor: (CGFloat*)r : (CGFloat*)g : (CGFloat*)b;
- (void) DPSsetalpha: (CGFloat)a;
- (void) DPSsetcmykcolor: (CGFloat)c : (CGFloat)m : (CGFloat)y : (CGFloat)k;
- (void) DPSsetgray: (CGFloat)gray;
- (void) DPSsethsbcolor: (CGFloat)h : (CGFloat)s : (CGFloat)b;
- (void) DPSsetrgbcolor: (CGFloat)r : (CGFloat)g : (CGFloat)b;

- (void) GSSetPatterColor: (NSImage*)image;

- (void) GSSetFillColorspace: (void *)spaceref;
- (void) GSSetStrokeColorspace: (void *)spaceref;
- (void) GSSetFillColor: (const CGFloat *)values;
- (void) GSSetStrokeColor: (const CGFloat *)values;

/* ----------------------------------------------------------------------- */
/* Text operations */
/* ----------------------------------------------------------------------- */
- (void) DPSashow: (CGFloat)x : (CGFloat)y : (const char*)s;
- (void) DPSawidthshow: (CGFloat)cx : (CGFloat)cy : (int)c
                      : (CGFloat)ax : (CGFloat)ay : (const char*)s;
- (void) DPScharpath: (const char*)s : (int)b;
- (void) appendBezierPathWithPackedGlyphs: (const char *)packedGlyphs
                                     path: (NSBezierPath*)aPath;
- (void) DPSshow: (const char*)s;
- (void) DPSwidthshow: (CGFloat)x : (CGFloat)y : (int)c : (const char*)s;
- (void) DPSxshow: (const char*)s : (const CGFloat*)numarray : (int)size;
- (void) DPSxyshow: (const char*)s : (const CGFloat*)numarray : (int)size;
- (void) DPSyshow: (const char*)s : (const CGFloat*)numarray : (int)size;

- (void) GSSetCharacterSpacing: (CGFloat)extra;
- (void) GSSetFont: (void *)fontref;
- (void) GSSetFontSize: (CGFloat)size;
- (NSAffineTransform *) GSGetTextCTM;
- (NSPoint) GSGetTextPosition;
- (void) GSSetTextCTM: (NSAffineTransform *)ctm;
- (void) GSSetTextDrawingMode: (GSTextDrawingMode)mode;
- (void) GSSetTextPosition: (NSPoint)loc;
- (void) GSShowText: (const char *)string : (size_t) length;
- (void) GSShowGlyphs: (const NSGlyph *)glyphs : (size_t) length;
- (void) GSShowGlyphsWithAdvances: (const NSGlyph *)glyphs : (const NSSize *)advances : (size_t) length;

/* ----------------------------------------------------------------------- */
/* Gstate Handling */
/* ----------------------------------------------------------------------- */
- (void) DPSgrestore;
- (void) DPSgsave;
- (void) DPSinitgraphics;
- (void) DPSsetgstate: (NSInteger)gst;

- (NSInteger)  GSDefineGState;
- (void) GSUndefineGState: (NSInteger)gst;
- (void) GSReplaceGState: (NSInteger)gst;

/* ----------------------------------------------------------------------- */
/* Gstate operations */
/* ----------------------------------------------------------------------- */
- (void) DPScurrentflat: (CGFloat*)flatness;
- (void) DPScurrentlinecap: (int*)linecap;
- (void) DPScurrentlinejoin: (int*)linejoin;
- (void) DPScurrentlinewidth: (CGFloat*)width;
- (void) DPScurrentmiterlimit: (CGFloat*)limit;
- (void) DPScurrentpoint: (CGFloat*)x : (CGFloat*)y;
- (void) DPScurrentstrokeadjust: (int*)b;
- (void) DPSsetdash: (const CGFloat*)pat : (NSInteger)size : (CGFloat)offset;
- (void) DPSsetflat: (CGFloat)flatness;
- (void) DPSsethalftonephase: (CGFloat)x : (CGFloat)y;
- (void) DPSsetlinecap: (int)linecap;
- (void) DPSsetlinejoin: (int)linejoin;
- (void) DPSsetlinewidth: (CGFloat)width;
- (void) DPSsetmiterlimit: (CGFloat)limit;
- (void) DPSsetstrokeadjust: (int)b;

/* ----------------------------------------------------------------------- */
/* Matrix operations */
/* ----------------------------------------------------------------------- */
- (void) DPSconcat: (const CGFloat*)m;
- (void) DPSinitmatrix;
- (void) DPSrotate: (CGFloat)angle;
- (void) DPSscale: (CGFloat)x : (CGFloat)y;
- (void) DPStranslate: (CGFloat)x : (CGFloat)y;

- (NSAffineTransform *) GSCurrentCTM;
- (void) GSSetCTM: (NSAffineTransform *)ctm;
- (void) GSConcatCTM: (NSAffineTransform *)ctm;

/* ----------------------------------------------------------------------- */
/* Paint operations */
/* ----------------------------------------------------------------------- */
- (void) DPSarc: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
	       : (CGFloat)angle2;
- (void) DPSarcn: (CGFloat)x : (CGFloat)y : (CGFloat)r : (CGFloat)angle1 
		: (CGFloat)angle2;
- (void) DPSarct: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 : (CGFloat)r;
- (void) DPSclip;
- (void) DPSclosepath;
- (void) DPScurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		   : (CGFloat)x3 : (CGFloat)y3;
- (void) DPSeoclip;
- (void) DPSeofill;
- (void) DPSfill;
- (void) DPSflattenpath;
- (void) DPSinitclip;
- (void) DPSlineto: (CGFloat)x : (CGFloat)y;
- (void) DPSmoveto: (CGFloat)x : (CGFloat)y;
- (void) DPSnewpath;
- (void) DPSpathbbox: (CGFloat*)llx : (CGFloat*)lly : (CGFloat*)urx : (CGFloat*)ury;
- (void) DPSrcurveto: (CGFloat)x1 : (CGFloat)y1 : (CGFloat)x2 : (CGFloat)y2 
		    : (CGFloat)x3 : (CGFloat)y3;
- (void) DPSrectclip: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSrectfill: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSrectstroke: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h;
- (void) DPSreversepath;
- (void) DPSrlineto: (CGFloat)x : (CGFloat)y;
- (void) DPSrmoveto: (CGFloat)x : (CGFloat)y;
- (void) DPSstroke;
- (void) DPSshfill: (NSDictionary *)shaderDictionary;

- (void) GSSendBezierPath: (NSBezierPath *)path;
- (void) GSRectClipList: (const NSRect *)rects : (int) count;
- (void) GSRectFillList: (const NSRect *)rects : (int) count;

/* ----------------------------------------------------------------------- */
/* Window system ops */
/* ----------------------------------------------------------------------- */
- (void) GSCurrentDevice: (void**)device : (int*)x : (int*)y;
- (void) GSSetDevice: (void*)device : (int)x : (int)y;
- (void) DPScurrentoffset: (int*)x : (int*)y;
- (void) DPSsetoffset: (short int)x : (short int)y;

/*-------------------------------------------------------------------------*/
/* Graphics Extensions Ops */
/*-------------------------------------------------------------------------*/
- (void) DPScomposite: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
		     : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (NSCompositingOperation)op;
- (void) DPScompositerect: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h : (NSCompositingOperation)op;
- (void) DPSdissolve: (CGFloat)x : (CGFloat)y : (CGFloat)w : (CGFloat)h 
		    : (NSInteger)gstateNum : (CGFloat)dx : (CGFloat)dy : (CGFloat)delta;

- (void) GScomposite: (NSInteger)gstateNum 
             toPoint: (NSPoint)aPoint
	    fromRect: (NSRect)srcRect
	   operation: (NSCompositingOperation)op
	    fraction: (CGFloat)delta;
- (BOOL) supportsDrawGState;
- (void) GSdraw: (NSInteger)gstateNum
        toPoint: (NSPoint)aPoint
       fromRect: (NSRect)srcRect
      operation: (NSCompositingOperation)op
       fraction: (CGFloat)delta;
- (void) GSDrawImage: (NSRect)rect : (void *)imageref;

/* ----------------------------------------------------------------------- */
/* Postscript Client functions */
/* ----------------------------------------------------------------------- */
- (void) DPSPrintf: (const char *)fmt : (va_list)args;
- (void) DPSWriteData: (const char *)buf : (unsigned int)count;

@end

/* ----------------------------------------------------------------------- */
/* NSGraphics Ops */	
/* ----------------------------------------------------------------------- */
@interface NSGraphicsContext (NSGraphics)

/**
<p>
Read raw pixels from the device and return the information as a bitmap.
Pixels are read from the smallest device-pixel aligned rectangle
containing rect (defined in the current graphics state and clipped to
the current window, but not against the clipping path). If the resulting
device rectangle is degenerate, Size will be (0,0) and Data will be nil,
but the other entries in the dictionary will be filled in.
</p><p>
If the device does not support the operation, returns nil.
</p><p>
The returned dictionary contains at least the following keys:
</p>
<deflist>
<term>Data</term><desc>An NSData-instance with the image data.</desc>

<term>Size</term><desc>An NSValue/NSSize with the size in pixels of the
returned image data.</desc>

<term>BitsPerSample</term><desc>An NSValue/unsigned int.</desc>

<term>SamplesPerPixel</term><desc>An NSValue/unsigned int.</desc>

<term>ColorSpace</term><desc>An NSString with the name of the color space the
data is in.</desc>

<term>HasAlpha</term><desc>An NSValue/unsigned int. 0 if the returned image
does not have an alpha channel, 1 if it does.</desc>

<term>Matrix</term><desc>An NSAffineTransform-instance that contains the
transform between current user space and image space for this image.</desc>
</deflist>
*/
- (NSDictionary *) GSReadRect: (NSRect)rect;

/* Soon to be obsolete */
- (void) NSDrawBitmap: (NSRect) rect : (NSInteger) pixelsWide : (NSInteger) pixelsHigh
		     : (NSInteger) bitsPerSample : (NSInteger) samplesPerPixel 
		     : (NSInteger) bitsPerPixel : (NSInteger) bytesPerRow : (BOOL) isPlanar
		     : (BOOL) hasAlpha : (NSString *) colorSpaceName
		     : (const unsigned char *const [5]) data;

- (void) NSBeep;

/* Context helper wraps */
- (void) GSWSetViewIsFlipped: (BOOL) flipped;
- (BOOL) GSWViewIsFlipped;

@end

/* ----------------------------------------------------------------------- */
/* Printing Ops */	
/* ----------------------------------------------------------------------- */
@interface NSGraphicsContext (Printing)

- (void) beginPage: (int)ordinalNum
             label: (NSString*)aString
              bBox: (NSRect)pageRect
             fonts: (NSString*)fontNames;
- (void) beginPrologueBBox: (NSRect)boundingBox
              creationDate: (NSString*)dateCreated
                 createdBy: (NSString*)anApplication
                     fonts: (NSString*)fontNames
                   forWhom: (NSString*)user
                     pages: (int)numPages
                     title: (NSString*)aTitle;
- (void) beginSetup;
- (void) beginTrailer;
- (void) endDocumentPages: (int)pages
            documentFonts: (NSSet*)fontNames;
- (void) endHeaderComments;
- (void) endPageSetup;
- (void) endPrologue;
- (void) endSetup;
- (void) endSheet;
- (void) endTrailer;
- (void) printerProlog;
- (void) showPage;

@end

@interface NSGraphicsContext (NSGradient)
- (void) drawGradient: (NSGradient*)gradient
           fromCenter: (NSPoint)startCenter
               radius: (CGFloat)startRadius
             toCenter: (NSPoint)endCenter 
               radius: (CGFloat)endRadius
              options: (NSUInteger)options;

- (void) drawGradient: (NSGradient*)gradient
            fromPoint: (NSPoint)startPoint
              toPoint: (NSPoint)endPoint
              options: (NSUInteger)options;
@end

/* NSGraphicContext constants */
APPKIT_EXPORT NSString *NSGraphicsContextDestinationAttributeName;
APPKIT_EXPORT NSString *NSGraphicsContextPDFFormat;
APPKIT_EXPORT NSString *NSGraphicsContextPSFormat;
APPKIT_EXPORT NSString *NSGraphicsContextRepresentationFormatAttributeName;

/* Colorspace constants */
APPKIT_EXPORT NSString *GSColorSpaceName;
APPKIT_EXPORT NSString *GSColorSpaceWhitePoint;
APPKIT_EXPORT NSString *GSColorSpaceBlackPoint;
APPKIT_EXPORT NSString *GSColorSpaceGamma;
APPKIT_EXPORT NSString *GSColorSpaceMatrix;
APPKIT_EXPORT NSString *GSColorSpaceRange;
APPKIT_EXPORT NSString *GSColorSpaceComponents;
APPKIT_EXPORT NSString *GSColorSpaceProfile;
APPKIT_EXPORT NSString *GSAlternateColorSpace;
APPKIT_EXPORT NSString *GSBaseColorSpace;
APPKIT_EXPORT NSString *GSColorSpaceColorTable;

#endif /* _NSGraphicsContext_h_INCLUDE */

