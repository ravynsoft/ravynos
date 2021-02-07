/* 
   The NSBezierPath class

   Copyright (C) 1999, 2005 Free Software Foundation, Inc.

   Author:  Enrico Sersale <enrico@imago.ro>
   Date: Dec 1999
   
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

#ifndef BEZIERPATH_H
#define BEZIERPATH_H
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>
#import <Foundation/NSObject.h>
#import <AppKit/NSFont.h>

@class NSAffineTransform;
@class NSImage;

typedef enum {
  NSButtLineCapStyle = 0,
  NSRoundLineCapStyle = 1,
  NSSquareLineCapStyle = 2
} NSLineCapStyle;

typedef enum {
  NSMiterLineJoinStyle = 0,
  NSRoundLineJoinStyle = 1,
  NSBevelLineJoinStyle = 2
} NSLineJoinStyle;

/** A winding rule defines which points are considered inside and which
    points are considered outside a path.
    <deflist>
      <term>NSNonZeroWindingRule</term>
      <desc>A point is inside the path iff the winding count at the point
      is non-zero.</desc>
      <term>NSEvenOddWindingRule</term>
      <desc>A point is inside the path iff the winding count at the point
      is odd.</desc>
    </deflist>
    */
typedef enum {
  NSNonZeroWindingRule,
  NSEvenOddWindingRule
} NSWindingRule;

typedef enum {
  NSMoveToBezierPathElement,
  NSLineToBezierPathElement,
  NSCurveToBezierPathElement,
  NSClosePathBezierPathElement
} NSBezierPathElement;

@interface NSBezierPath : NSObject <NSCopying, NSCoding>
{
@private
  NSWindingRule _windingRule;
  NSLineCapStyle _lineCapStyle;
  NSLineJoinStyle _lineJoinStyle;
  CGFloat _lineWidth;
  CGFloat _flatness;
  CGFloat _miterLimit;
  NSInteger _dash_count;
  CGFloat _dash_phase;
  CGFloat *_dash_pattern;
  NSRect _bounds;
  NSRect _controlPointBounds;
  NSImage *_cacheImage;
#ifndef	_IN_NSBEZIERPATH_M
#define	GSIArray	void*
#endif
  GSIArray _pathElements;
#ifndef	_IN_NSBEZIERPATH_M
#undef	GSIArray
#endif
  BOOL _cachesBezierPath;
  BOOL _shouldRecalculateBounds;
  BOOL _flat;
}

//
// Creating common paths
//
+ (NSBezierPath *)bezierPath;
+ (NSBezierPath *)bezierPathWithRect:(NSRect)aRect;
+ (NSBezierPath *)bezierPathWithOvalInRect:(NSRect)aRect;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
+ (NSBezierPath *)bezierPathWithRoundedRect:(NSRect)aRect
                                    xRadius:(CGFloat)xRadius
                                    yRadius:(CGFloat)yRadius;
#endif

//
// Immediate mode drawing of common paths
//
+ (void)fillRect:(NSRect)aRect;

/** Using default stroke color and default drawing attributes, strokes 
    a rectangle using the specified rect. */
+ (void)strokeRect:(NSRect)aRect;
+ (void)clipRect:(NSRect)aRect;

/** Using default stroke color and default drawing attributes, draws a line 
    between the two points specified. */
+ (void)strokeLineFromPoint:(NSPoint)point1 toPoint:(NSPoint)point2;
+ (void)drawPackedGlyphs: (const char *)packedGlyphs atPoint: (NSPoint)aPoint;

//
// Default path rendering parameters
//
+ (void)setDefaultMiterLimit:(CGFloat)limit;
+ (CGFloat)defaultMiterLimit;
+ (void)setDefaultFlatness:(CGFloat)flatness;
+ (CGFloat)defaultFlatness;
+ (void)setDefaultWindingRule:(NSWindingRule)windingRule;
+ (NSWindingRule)defaultWindingRule;
+ (void)setDefaultLineCapStyle:(NSLineCapStyle)lineCapStyle;
+ (NSLineCapStyle)defaultLineCapStyle;
+ (void)setDefaultLineJoinStyle:(NSLineJoinStyle)lineJoinStyle;
+ (NSLineJoinStyle)defaultLineJoinStyle;
+ (void)setDefaultLineWidth:(CGFloat)lineWidth;
+ (CGFloat)defaultLineWidth;

//
// Path construction
//
- (void)moveToPoint:(NSPoint)aPoint;
- (void)lineToPoint:(NSPoint)aPoint;
- (void)curveToPoint:(NSPoint)aPoint 
       controlPoint1:(NSPoint)controlPoint1
       controlPoint2:(NSPoint)controlPoint2;
- (void)closePath;
- (void)removeAllPoints;

//
// Relative path construction
//
- (void)relativeMoveToPoint:(NSPoint)aPoint;
- (void)relativeLineToPoint:(NSPoint)aPoint;
- (void)relativeCurveToPoint:(NSPoint)aPoint
	       controlPoint1:(NSPoint)controlPoint1
	       controlPoint2:(NSPoint)controlPoint2;

//
// Path rendering parameters
//
- (CGFloat)lineWidth;
- (void)setLineWidth:(CGFloat)lineWidth;
- (NSLineCapStyle)lineCapStyle;
- (void)setLineCapStyle:(NSLineCapStyle)lineCapStyle;
- (NSLineJoinStyle)lineJoinStyle;
- (void)setLineJoinStyle:(NSLineJoinStyle)lineJoinStyle;
- (NSWindingRule)windingRule;
- (void)setWindingRule:(NSWindingRule)windingRule;
- (void)setFlatness:(CGFloat)flatness;
- (CGFloat)flatness;
- (void)setMiterLimit:(CGFloat)limit;
- (CGFloat)miterLimit;
- (void)getLineDash:(CGFloat *)pattern count:(NSInteger *)count phase:(CGFloat *)phase;
- (void)setLineDash:(const CGFloat *)pattern count:(NSInteger)count phase:(CGFloat)phase;

//
// Path operations
//
- (void)stroke;
- (void)fill;
- (void)addClip;
- (void)setClip;

//
// Path modifications.
//
- (NSBezierPath *)bezierPathByFlatteningPath;
- (NSBezierPath *)bezierPathByReversingPath;

//
// Applying transformations.
//
- (void)transformUsingAffineTransform:(NSAffineTransform *)transform;

//
// Path info
//
- (BOOL)isEmpty;
- (NSPoint)currentPoint;
- (NSRect)controlPointBounds;
- (NSRect)bounds;

//
// Elements
//
- (NSInteger)elementCount;
- (NSBezierPathElement)elementAtIndex:(NSInteger)index
		     associatedPoints:(NSPoint *)points;
- (NSBezierPathElement)elementAtIndex:(NSInteger)index;
- (void)setAssociatedPoints:(NSPoint *)points atIndex:(NSInteger)index;

//
// Appending common paths
//
- (void)appendBezierPath:(NSBezierPath *)aPath;
- (void)appendBezierPathWithRect:(NSRect)aRect;
- (void)appendBezierPathWithPoints:(NSPoint *)points count:(NSInteger)count;
- (void)appendBezierPathWithOvalInRect:(NSRect)aRect;
- (void)appendBezierPathWithArcWithCenter:(NSPoint)center  
				   radius:(CGFloat)radius
			       startAngle:(CGFloat)startAngle
				 endAngle:(CGFloat)endAngle
				clockwise:(BOOL)clockwise;
- (void)appendBezierPathWithArcWithCenter:(NSPoint)center  
				   radius:(CGFloat)radius
			       startAngle:(CGFloat)startAngle
				 endAngle:(CGFloat)endAngle;
- (void)appendBezierPathWithArcFromPoint:(NSPoint)point1
				 toPoint:(NSPoint)point2
				  radius:(CGFloat)radius;
- (void)appendBezierPathWithGlyph:(NSGlyph)glyph inFont:(NSFont *)font;
- (void)appendBezierPathWithGlyphs:(NSGlyph *)glyphs 
			     count:(NSInteger)count
			    inFont:(NSFont *)font;
- (void)appendBezierPathWithPackedGlyphs:(const char *)packedGlyphs;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void)appendBezierPathWithRoundedRect:(NSRect)aRect
                                xRadius:(CGFloat)xRadius
                                yRadius:(CGFloat)yRadius;
#endif

//
// Hit detection  
//

#if OS_API_VERSION(GS_API_NONE, GS_API_NONE)
/** Returns the winding count, according to the PostScript definition,
    at the given point.  */
- (int) windingCountAtPoint: (NSPoint)point;
#endif

/** Returns YES iff the path contains, according to the current
    <ref type="type" id="NSWindingRule">winding rule</ref>, the given point.
    */
- (BOOL)containsPoint:(NSPoint)point;

//
// Caching
// 
- (BOOL)cachesBezierPath;
- (void)setCachesBezierPath:(BOOL)flag;

@end

#endif // BEZIERPATH_H
