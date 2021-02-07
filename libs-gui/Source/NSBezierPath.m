/** <title>NSBezierPath.m</title>

   <abstract>The NSBezierPath class</abstract>

   Copyright (C) 1999, 2005 Free Software Foundation, Inc.

   Author:  Enrico Sersale <enrico@imago.ro>
   Date: Dec 1999
   Modified:  Fred Kiefer <FredKiefer@gmx.de>
   Date: January 2001
   
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

#import <Foundation/NSData.h>
#import <Foundation/NSDebug.h>
#import "AppKit/NSAffineTransform.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSImage.h"
#import "AppKit/PSOperators.h"
#import "GNUstepGUI/GSFontInfo.h"
#import "GSGuiPrivate.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626434
#endif

typedef struct _PathElement
{
  /*NSBezierPathElement*/int type;
  NSPoint points[3];
} PathElement;

//#define GSUNION_TYPES GSUNION_OBJ
#define GSI_ARRAY_TYPES       0
#define GSI_ARRAY_TYPE	PathElement

#define GSI_ARRAY_NO_RETAIN
#define GSI_ARRAY_NO_RELEASE

#ifdef GSIArray
#undef GSIArray
#endif
#include <GNUstepBase/GSIArray.h>

#define	_IN_NSBEZIERPATH_M	1
#import "AppKit/NSBezierPath.h"
#undef	_IN_NSBEZIERPATH_M


// This magic number is 4 *(sqrt(2) -1)/3
#define KAPPA 0.5522847498
#define INVALIDATE_CACHE()   [self _invalidateCache]

static void flatten(NSPoint coeff[], CGFloat flatness, NSBezierPath *path);

static NSWindingRule default_winding_rule = NSNonZeroWindingRule;
static CGFloat default_line_width = 1.0;
static CGFloat default_flatness = 0.6;
static NSLineJoinStyle default_line_join_style = NSMiterLineJoinStyle;
static NSLineCapStyle default_line_cap_style = NSButtLineCapStyle;
static CGFloat default_miter_limit = 10.0;

@interface NSBezierPath (PrivateMethods)
- (void)_invalidateCache;
- (void)_recalculateBounds;
@end


#if 0
@interface GSBezierPath : NSBezierPath
{
  GSIArray pathElements;
  BOOL flat;
}
@end
#endif

@implementation NSBezierPath

+ (void)initialize
{
  if (self == [NSBezierPath class])
    {
      [self setVersion: 2];
    }
}

//
// Creating common paths
//
+ (NSBezierPath *)bezierPath
{
  return AUTORELEASE([[self alloc] init]);
}

+ (NSBezierPath *)bezierPathWithRect: (NSRect)aRect
{
  NSBezierPath *path;
	
  path = [self bezierPath];
  [path appendBezierPathWithRect: aRect];
	
  return path;
}

+ (NSBezierPath *)bezierPathWithOvalInRect: (NSRect)aRect
{
  NSBezierPath *path;
  
  path = [self bezierPath];
  [path appendBezierPathWithOvalInRect: aRect];

  return path;
}

+ (NSBezierPath *)bezierPathWithRoundedRect: (NSRect)aRect
                                    xRadius: (CGFloat)xRadius
                                    yRadius: (CGFloat)yRadius
{
  NSBezierPath *path;

  path = [self bezierPath];
  [path appendBezierPathWithRoundedRect: aRect
                                xRadius: xRadius
                                yRadius: yRadius];

  return path;
}

//
// Immediate mode drawing of common paths
//
+ (void)fillRect: (NSRect)aRect
{
  PSrectfill(NSMinX(aRect), NSMinY(aRect), NSWidth(aRect),  NSHeight(aRect));
}

+ (void)strokeRect: (NSRect)aRect
{
  PSrectstroke(NSMinX(aRect), NSMinY(aRect), NSWidth(aRect),  NSHeight(aRect));
}

+ (void)clipRect: (NSRect)aRect
{
  PSrectclip(NSMinX(aRect), NSMinY(aRect), NSWidth(aRect),  NSHeight(aRect));
}

+ (void)strokeLineFromPoint: (NSPoint)point1  toPoint: (NSPoint)point2
{
  NSBezierPath *path = [[self alloc] init];
  
  [path moveToPoint: point1];
  [path lineToPoint: point2];
  [path stroke];
  RELEASE(path);
}

+ (void)drawPackedGlyphs: (const char *)packedGlyphs  atPoint: (NSPoint)aPoint
{
  NSBezierPath *path = [[self alloc] init];
  
  [path moveToPoint: aPoint];
  [path appendBezierPathWithPackedGlyphs: packedGlyphs];
  [path fill];
  RELEASE(path);
}

//
// Default path rendering parameters
//
+ (void)setDefaultMiterLimit:(CGFloat)limit
{
  default_miter_limit = limit;
  // Do we need this?
  PSsetmiterlimit(limit);
}

+ (CGFloat)defaultMiterLimit
{
  return default_miter_limit;
}

+ (void)setDefaultFlatness:(CGFloat)flatness
{
  default_flatness = flatness;
  PSsetflat(flatness);
}

+ (CGFloat)defaultFlatness
{
  return default_flatness;
}

+ (void)setDefaultWindingRule:(NSWindingRule)windingRule
{
  default_winding_rule = windingRule;
}

+ (NSWindingRule)defaultWindingRule
{
  return default_winding_rule;
}

+ (void)setDefaultLineCapStyle:(NSLineCapStyle)lineCapStyle
{
  default_line_cap_style = lineCapStyle;
  PSsetlinecap(lineCapStyle);
}

+ (NSLineCapStyle)defaultLineCapStyle
{
  return default_line_cap_style;
}

+ (void)setDefaultLineJoinStyle:(NSLineJoinStyle)lineJoinStyle
{
  default_line_join_style = lineJoinStyle;
  PSsetlinejoin(lineJoinStyle);
}

+ (NSLineJoinStyle)defaultLineJoinStyle
{
  return default_line_join_style;
}

+ (void)setDefaultLineWidth:(CGFloat)lineWidth
{
  default_line_width = lineWidth;
  PSsetlinewidth(lineWidth);
}

+ (CGFloat)defaultLineWidth
{
  return default_line_width;
}

- (id) init
{
  NSZone *zone;

  self = [super init];
  if (self == nil)
    return nil;

  // Those values come from the default.
  [self setLineWidth: default_line_width];
  [self setFlatness: default_flatness];
  [self setLineCapStyle: default_line_cap_style];
  [self setLineJoinStyle: default_line_join_style];
  [self setMiterLimit: default_miter_limit];
  [self setWindingRule: default_winding_rule];
  // Set by allocation
  //_bounds = NSZeroRect;
  //_controlPointBounds = NSZeroRect;
  //_cachesBezierPath = NO;
  //_cacheImage = nil;
  //_dash_count = 0;
  //_dash_phase = 0;
  //_dash_pattern = NULL; 

  zone = [self zone];
  _pathElements = NSZoneMalloc(zone, sizeof(GSIArray_t));
  GSIArrayInitWithZoneAndCapacity(_pathElements, zone, 8);
  _flat = YES;

  return self;
}

- (void) dealloc
{
  GSIArrayEmpty(_pathElements);
  NSZoneFree([self zone], _pathElements);

  if (_cacheImage != nil)
    RELEASE(_cacheImage);

  if (_dash_pattern != NULL)
    NSZoneFree([self zone], _dash_pattern);

  [super dealloc];
}

//
// Path construction
//
- (void)moveToPoint:(NSPoint)aPoint
{
  PathElement elem;
  
  elem.type = NSMoveToBezierPathElement;
  elem.points[0] = aPoint;
  elem.points[1] = NSZeroPoint;
  elem.points[2] = NSZeroPoint;
  GSIArrayAddItem(_pathElements, (GSIArrayItem)elem);
  INVALIDATE_CACHE();
}

- (void)lineToPoint:(NSPoint)aPoint
{
  PathElement elem;
  
  elem.type = NSLineToBezierPathElement;
  elem.points[0] = aPoint;
  elem.points[1] = NSZeroPoint;
  elem.points[2] = NSZeroPoint;
  GSIArrayAddItem(_pathElements, (GSIArrayItem)elem);
  INVALIDATE_CACHE();
}

- (void)curveToPoint:(NSPoint)aPoint 
       controlPoint1:(NSPoint)controlPoint1
       controlPoint2:(NSPoint)controlPoint2
{
  PathElement elem;
  
  elem.type = NSCurveToBezierPathElement;
  elem.points[0] = controlPoint1;
  elem.points[1] = controlPoint2;
  elem.points[2] = aPoint;
  GSIArrayAddItem(_pathElements, (GSIArrayItem)elem);
  _flat = NO;

  INVALIDATE_CACHE();
}

- (void)closePath
{
  PathElement elem;

  elem.type = NSClosePathBezierPathElement;
  elem.points[0] = NSZeroPoint;
  elem.points[1] = NSZeroPoint;
  elem.points[2] = NSZeroPoint;
  GSIArrayAddItem(_pathElements, (GSIArrayItem)elem);
  INVALIDATE_CACHE();
}

- (void)removeAllPoints
{
  GSIArrayRemoveAllItems(_pathElements);
  _flat = YES;
  INVALIDATE_CACHE();
}

//
// Relative path construction
//
- (void)relativeMoveToPoint:(NSPoint)aPoint
{
  NSPoint p = [self currentPoint];

  p.x = p.x + aPoint.x;
  p.y = p.y + aPoint.y;
  [self moveToPoint: p];
}

- (void)relativeLineToPoint:(NSPoint)aPoint
{
  NSPoint p = [self currentPoint];

  p.x = p.x + aPoint.x;
  p.y = p.y + aPoint.y;
  [self lineToPoint: p];
}

- (void)relativeCurveToPoint:(NSPoint)aPoint
	       controlPoint1:(NSPoint)controlPoint1
	       controlPoint2:(NSPoint)controlPoint2
{
  NSPoint p = [self currentPoint];

  aPoint.x = p.x + aPoint.x;
  aPoint.y = p.y + aPoint.y;
  controlPoint1.x = p.x + controlPoint1.x;
  controlPoint1.y = p.y + controlPoint1.y;
  controlPoint2.x = p.x + controlPoint2.x;
  controlPoint2.y = p.y + controlPoint2.y;
  [self curveToPoint: aPoint
	controlPoint1: controlPoint1
	controlPoint2: controlPoint2];
}

//
// Path rendering parameters
//
- (CGFloat)lineWidth
{
  return _lineWidth;
}

- (void)setLineWidth:(CGFloat)lineWidth
{
  _lineWidth = lineWidth;
}

- (NSLineCapStyle)lineCapStyle
{
  return _lineCapStyle;
}

- (void)setLineCapStyle:(NSLineCapStyle)lineCapStyle
{
  _lineCapStyle = lineCapStyle;
}

- (NSLineJoinStyle)lineJoinStyle
{
  return _lineJoinStyle;
}

- (void)setLineJoinStyle:(NSLineJoinStyle)lineJoinStyle
{
  _lineJoinStyle = lineJoinStyle;
}

- (NSWindingRule)windingRule
{
  return _windingRule;
}

- (void)setWindingRule:(NSWindingRule)windingRule
{
  _windingRule = windingRule;
}

- (void)setFlatness:(CGFloat)flatness
{
  _flatness = flatness;
}

- (CGFloat)flatness
{
  return _flatness;
}

- (void)setMiterLimit:(CGFloat)limit
{
  _miterLimit = limit;
}

- (CGFloat)miterLimit
{
  return _miterLimit;
}

- (void)getLineDash:(CGFloat *)pattern count:(NSInteger *)count phase:(CGFloat *)phase
{
  // FIXME: How big is the pattern array? 
  // We assume that this value is in count!
  if (count != NULL)
    {
      if (*count < _dash_count)
        {
	  *count = _dash_count;
	  return;
	}
      *count = _dash_count;
    }

  if (phase != NULL)
    *phase = _dash_phase;

  memcpy(pattern, _dash_pattern, _dash_count * sizeof(CGFloat));
}

- (void)setLineDash:(const CGFloat *)pattern count:(NSInteger)count phase:(CGFloat)phase
{
  NSZone *myZone = [self zone];
  
  if ((pattern == NULL) || (count == 0))
    {
      if (_dash_pattern != NULL)
        {
	  NSZoneFree(myZone, _dash_pattern);
	  _dash_pattern = NULL;
	}
      _dash_count = 0;
      _dash_phase = 0.0;
      return;
    }

  if (_dash_pattern == NULL)
    _dash_pattern = NSZoneMalloc(myZone, count * sizeof(CGFloat));
  else
    _dash_pattern = NSZoneRealloc(myZone, _dash_pattern, count * sizeof(CGFloat));

  _dash_count = count;
  _dash_phase = phase;
  memcpy(_dash_pattern, pattern, _dash_count * sizeof(CGFloat));
}

//
// Path operations
//
- (void)stroke
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  
  if (_cachesBezierPath) 
    {
      NSRect bounds = [self bounds];
      NSPoint origin = bounds.origin;

      // FIXME: I don't see how this should work with color changes
      if (_cacheImage == nil) 
        {
	  _cacheImage = [[NSImage alloc] initWithSize: bounds.size];
	  [_cacheImage lockFocus];
	  DPStranslate(ctxt, -origin.x, -origin.y);
	  [ctxt GSSendBezierPath: self];
	  DPSstroke(ctxt);
	  [_cacheImage unlockFocus];
	}
      [_cacheImage compositeToPoint: origin operation: NSCompositeCopy];
    } 
  else 
    {
      [ctxt GSSendBezierPath: self];
      DPSstroke(ctxt);
    }
}

- (void)fill
{
  NSGraphicsContext *ctxt = GSCurrentContext();

  if (_cachesBezierPath) 
    {
      NSRect bounds = [self bounds];
      NSPoint origin = bounds.origin;

      // FIXME: I don't see how this should work with color changes
      if (_cacheImage == nil) 
        {
	  _cacheImage = [[NSImage alloc] initWithSize: bounds.size];
	  [_cacheImage lockFocus];
	  DPStranslate(ctxt, -origin.x, -origin.y);
	  [ctxt GSSendBezierPath: self];
	  if ([self windingRule] == NSNonZeroWindingRule)
	    DPSfill(ctxt);
	  else
	    DPSeofill(ctxt);
	  [_cacheImage unlockFocus];
	}
      [_cacheImage compositeToPoint: origin operation: NSCompositeCopy];
    } 
  else 
    {
      [ctxt GSSendBezierPath: self];
      if ([self windingRule] == NSNonZeroWindingRule)
	DPSfill(ctxt);
      else
	DPSeofill(ctxt);
    }
}

- (void)addClip
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  
  [ctxt GSSendBezierPath: self];
  if ([self windingRule] == NSNonZeroWindingRule)
    DPSclip(ctxt);
  else
    DPSeoclip(ctxt);
}

- (void)setClip
{
  NSGraphicsContext *ctxt = GSCurrentContext();
  
  DPSinitclip(ctxt);
  [ctxt GSSendBezierPath: self];
  if ([self windingRule] == NSNonZeroWindingRule)
    DPSclip(ctxt);
  else
    DPSeoclip(ctxt);
}

//
// Path modifications.
//
- (NSBezierPath *)bezierPathByFlatteningPath
{
  NSBezierPath *path;
  NSBezierPathElement type;
  NSPoint pts[3];
  NSPoint coeff[4];
  NSPoint p, last_p;
  NSInteger i, count;
  BOOL first = YES;

  if (_flat)
    return self;

  /* Silence compiler warnings.  */
  p = NSZeroPoint;
  last_p = NSZeroPoint;

  path = [[self class] bezierPath];
  count = [self elementCount];
  for (i = 0; i < count; i++) 
    {
      type = [self elementAtIndex: i associatedPoints: pts];
      switch(type) 
        {
	  case NSMoveToBezierPathElement:
	      [path moveToPoint: pts[0]];
	      last_p = p = pts[0];
	      first = NO;
	      break;
	  case NSLineToBezierPathElement:
	      [path lineToPoint: pts[0]];
	      p = pts[0];
	      if (first)
	        {
		  last_p = pts[0];
		  first = NO;
		}
	      break;
	  case NSCurveToBezierPathElement:
	      coeff[0] = p;
	      coeff[1] = pts[0];
	      coeff[2] = pts[1];
	      coeff[3] = pts[2];
	      flatten(coeff, [self flatness], path);
	      p = pts[2];
	      if (first)
		{
		  last_p = pts[2];
		  first = NO;
		}
	      break;
	  case NSClosePathBezierPathElement:
	      [path closePath];
	      p = last_p;
	      break;
	  default:
	      break;
	}
    }

  return path;
}

- (NSBezierPath *) bezierPathByReversingPath
{
  NSBezierPath *path = [object_getClass(self) bezierPath];
  NSBezierPathElement type, last_type;
  NSPoint pts[3];
  NSPoint p, cp1, cp2;
  NSInteger i, count;
  BOOL closed = NO;

  /* Silence compiler warnings.  */
  p = NSZeroPoint;

  last_type = NSMoveToBezierPathElement;
  count = [self elementCount];
  for (i = count - 1; i >= 0; i--) 
    {
      type = [self elementAtIndex: i associatedPoints: pts];
      switch(type) 
        {
	  case NSMoveToBezierPathElement:
	      p = pts[0];
	      break;
	  case NSLineToBezierPathElement:
	      p = pts[0];
	      break;
	  case NSCurveToBezierPathElement:
	      cp1 = pts[0];
	      cp2 = pts[1];
	      p = pts[2];      
	      break;
	  case NSClosePathBezierPathElement:
	      p = pts[0];
	      break;
	  default:
	      break;
	}

      switch(last_type) 
        {
	  case NSMoveToBezierPathElement:
	      if (closed)
	        {
		  [path closePath];
		  closed = NO;
		}
	      [path moveToPoint: p];
	      break;
	  case NSLineToBezierPathElement:
	      [path lineToPoint: p];
	      break;
	  case NSCurveToBezierPathElement:
	      [path curveToPoint: p 
		    controlPoint1: cp2 
		    controlPoint2: cp1];	      
	      break;
	  case NSClosePathBezierPathElement:
	      closed = YES;
	      break;
	  default:
	      break;
	}
      last_type = type;
    }

  if (closed)
    [path closePath];
  return path;
}

//
// Applying transformations.
//
- (void) transformUsingAffineTransform: (NSAffineTransform *)transform
{
  NSBezierPathElement type;
  NSPoint pts[3];
  NSInteger i, count;
  SEL transformPointSel = @selector(transformPoint:); 
  NSPoint (*transformPointImp)(NSAffineTransform*, SEL, NSPoint);

  transformPointImp = (NSPoint (*)(NSAffineTransform*, SEL, NSPoint))
    [transform methodForSelector: transformPointSel];

  count = [self elementCount];
  for (i = 0; i < count; i++) 
    {
      type = [self elementAtIndex: i associatedPoints: pts];
      switch(type) 
        {
	  case NSMoveToBezierPathElement:
	  case NSLineToBezierPathElement:
	      pts[0] = (*transformPointImp)(transform,
                                            transformPointSel, pts[0]);
	      [self setAssociatedPoints: pts atIndex: i];
	      break;
	  case NSCurveToBezierPathElement:
	      pts[0] = (*transformPointImp)(transform,
                                            transformPointSel, pts[0]);
	      pts[1] = (*transformPointImp)(transform,
                                            transformPointSel, pts[1]);
	      pts[2] = (*transformPointImp)(transform,
                                            transformPointSel, pts[2]);
	      [self setAssociatedPoints: pts atIndex: i];
	      break;
	  case NSClosePathBezierPathElement:
	      break;
	  default:
	      break;
	}
    }
  INVALIDATE_CACHE();
}

//
// Path info
//
- (BOOL) isEmpty
{
  return ([self elementCount] == 0);
}

- (NSPoint) currentPoint
{
  NSBezierPathElement type;
  NSPoint points[3];
  NSInteger count;

  count = [self elementCount];
  if (!count) 
    [NSException raise: NSGenericException
		 format: @"No current Point in NSBezierPath"];

  type = [self elementAtIndex: count - 1 associatedPoints: points];
  switch(type) 
    {
      case NSMoveToBezierPathElement:
      case NSLineToBezierPathElement:
	  return points[0];
	  break;
      case NSCurveToBezierPathElement:
	  return points[2];
	  break;
      case NSClosePathBezierPathElement:
	  return points[0];
	  break;
      default:
	  break;
    }

  return NSZeroPoint;
}

- (NSRect) controlPointBounds
{
  if (_shouldRecalculateBounds)
     [self _recalculateBounds];
  return _controlPointBounds;
}

- (NSRect) bounds
{
  if (_shouldRecalculateBounds)
     [self _recalculateBounds];
  return _bounds;
}

//
// Elements
//
- (NSInteger) elementCount
{
  return GSIArrayCount(_pathElements);
}

- (NSBezierPathElement) elementAtIndex: (NSInteger)index
		      associatedPoints: (NSPoint *)points
{
  PathElement elm = GSIArrayItemAtIndex(_pathElements, index).ext;
  NSBezierPathElement type = elm.type;
  NSInteger i;
	
  if (points != NULL) 
    {
      switch(type) 
        {
        case NSMoveToBezierPathElement:
        case NSLineToBezierPathElement:
	  points[0] = elm.points[0];
          break;
        case NSCurveToBezierPathElement:
	  points[0] = elm.points[0];
	  points[1] = elm.points[1];
	  points[2] = elm.points[2];
          break;
        case NSClosePathBezierPathElement:
  	  // We have to find the last move element and take its point
	  for (i = index - 1; i >= 0; i--)
	    {
	      elm = GSIArrayItemAtIndex(_pathElements, i).ext;
	      if (elm.type == NSMoveToBezierPathElement)
                {
                  points[0] = elm.points[0];
                  break;
                }
	    }
          // FIXME: What to do if we don't find a move element?
          break;
        default:
	  break;
        }
    }
  
  return type;
}

- (NSBezierPathElement) elementAtIndex: (NSInteger)index
{
  return [self elementAtIndex: index associatedPoints: NULL];
}

- (void)setAssociatedPoints:(NSPoint *)points atIndex:(NSInteger)index
{
  PathElement elm = GSIArrayItemAtIndex(_pathElements, index).ext;
  NSBezierPathElement type = elm.type;
  
  switch(type) 
    {
      case NSMoveToBezierPathElement:
      case NSLineToBezierPathElement:
	  elm.points[0] = points[0];
	  break;
      case NSCurveToBezierPathElement:
	  elm.points[0] = points[0];
	  elm.points[1] = points[1];
	  elm.points[2] = points[2];
	  break;
      case NSClosePathBezierPathElement:
	  break;
      default:
	  break;
    }

  GSIArraySetItemAtIndex(_pathElements, (GSIArrayItem)elm, index);
  INVALIDATE_CACHE();
}

//
// Appending common paths
//
- (void) appendBezierPath: (NSBezierPath *)aPath
{
  NSBezierPathElement type;
  NSPoint points[3];
  NSInteger i, count;

  count = [aPath elementCount];
  for (i = 0; i < count; i++)
    {
      type = [aPath elementAtIndex: i associatedPoints: points];
      switch(type) 
        {
	  case NSMoveToBezierPathElement:
	      [self moveToPoint: points[0]];
	      break;
	  case NSLineToBezierPathElement:
	      [self lineToPoint: points[0]];
	      break;
	  case NSCurveToBezierPathElement:
	      [self curveToPoint: points[2] 
		    controlPoint1: points[0]
		    controlPoint2: points[1]];
	      break;
	  case NSClosePathBezierPathElement:
	      [self closePath];
	      break;
	  default:
	      break;
	}
    }
}

- (void)appendBezierPathWithRect:(NSRect)aRect
{
  NSPoint p;
	
  [self moveToPoint: aRect.origin];
  p.x = aRect.origin.x + aRect.size.width;
  p.y = aRect.origin.y;
  [self lineToPoint: p];
  p.x = aRect.origin.x + aRect.size.width;
  p.y = aRect.origin.y + aRect.size.height;
  [self lineToPoint: p];
  p.x = aRect.origin.x;
  p.y = aRect.origin.y + aRect.size.height;
  [self lineToPoint: p];
  [self closePath];
}

- (void)appendBezierPathWithPoints:(NSPoint *)points count:(NSInteger)count
{
  NSInteger i;

  if (!count)
    return;

  if ([self isEmpty])
    {
      [self moveToPoint: points[0]];
    }
  else
    {
      [self lineToPoint: points[0]];
    }
  
  for (i = 1; i < count; i++)
    {
      [self lineToPoint: points[i]];
    }
}

- (void) appendBezierPathWithOvalInRect: (NSRect)aRect
{
  NSPoint p, p1, p2;
  double originx = aRect.origin.x;
  double originy = aRect.origin.y;
  double width = aRect.size.width;
  double height = aRect.size.height;
  double hdiff = width / 2 * KAPPA;
  double vdiff = height / 2 * KAPPA;
  
  p = NSMakePoint(originx + width / 2, originy + height);
  [self moveToPoint: p];
  
  p = NSMakePoint(originx, originy + height / 2);
  p1 = NSMakePoint(originx + width / 2 - hdiff, originy + height);
  p2 = NSMakePoint(originx, originy + height / 2 + vdiff);
  [self curveToPoint: p controlPoint1: p1 controlPoint2: p2];
  
  p = NSMakePoint(originx + width / 2, originy);
  p1 = NSMakePoint(originx, originy + height / 2 - vdiff);
  p2 = NSMakePoint(originx + width / 2 - hdiff, originy);
  [self curveToPoint: p controlPoint1: p1 controlPoint2: p2];	
  
  p = NSMakePoint(originx + width, originy + height / 2);
  p1 = NSMakePoint(originx + width / 2 + hdiff, originy);
  p2 = NSMakePoint(originx + width, originy + height / 2 - vdiff);
  [self curveToPoint: p controlPoint1: p1 controlPoint2: p2];	
  
  p = NSMakePoint(originx + width / 2, originy + height);
  p1 = NSMakePoint(originx + width, originy + height / 2 + vdiff);
  p2 = NSMakePoint(originx + width / 2 + hdiff, originy + height);
  [self curveToPoint: p controlPoint1: p1 controlPoint2: p2];	
}

/* startAngle and endAngle are in degrees, counterclockwise, from the
   x axis */
- (void) appendBezierPathWithArcWithCenter: (NSPoint)center  
				    radius: (CGFloat)radius
				startAngle: (CGFloat)startAngle
				  endAngle: (CGFloat)endAngle
				 clockwise: (BOOL)clockwise
{
  CGFloat startAngle_rad, endAngle_rad, diff;
  NSPoint p0, p1, p2, p3;

  /* We use the Postscript prescription for managing the angles and
     drawing the arc.  See the documentation for `arc' and `arcn' in
     the Postscript Reference. */

  if (clockwise)
    {
      /* This modification of the angles is the postscript
         prescription. */
      while (startAngle < endAngle)
        endAngle -= 360;

      /* This is used when we draw a clockwise quarter of
	 circumference.  By adding diff at the starting angle of the
	 quarter, we get the ending angle.  diff is negative because
	 we draw clockwise. */
      diff = - M_PI / 2;
    }
  else
    {
      /* This modification of the angles is the postscript
         prescription. */
      while (endAngle < startAngle)
        endAngle += 360;

      /* This is used when we draw a counterclockwise quarter of
	 circumference.  By adding diff at the starting angle of the
	 quarter, we get the ending angle.  diff is positive because
	 we draw counterclockwise. */
      diff = M_PI / 2;
    }

  /* Convert the angles to radians */
  startAngle_rad = M_PI * startAngle / 180;
  endAngle_rad = M_PI * endAngle / 180;

  /* Start point */
  p0 = NSMakePoint (center.x + radius * cos (startAngle_rad), 
		    center.y + radius * sin (startAngle_rad));
  if ([self elementCount] == 0)
    {
      [self moveToPoint: p0];
    }
  else
    {
      NSPoint ps = [self currentPoint];
      
      if (p0.x != ps.x  ||  p0.y != ps.y)
	{
	  [self lineToPoint: p0];
	}
    }
  
  while ((clockwise) ? (startAngle_rad > endAngle_rad) 
	 : (startAngle_rad < endAngle_rad))
    {
    /* Add a quarter circle */
    if ((clockwise) ? (startAngle_rad + diff >= endAngle_rad) 
	: (startAngle_rad + diff <= endAngle_rad))
      {
	CGFloat sin_start = sin (startAngle_rad);
	CGFloat cos_start = cos (startAngle_rad);
	CGFloat sign = (clockwise) ? -1.0 : 1.0;
	
	p1 = NSMakePoint (center.x 
                           + radius * (cos_start - KAPPA * sin_start * sign), 
			  center.y 
                           + radius * (sin_start + KAPPA * cos_start * sign));
	p2 = NSMakePoint (center.x 
                           + radius * (-sin_start * sign + KAPPA * cos_start),
			  center.y 
                           + radius * (cos_start * sign + KAPPA * sin_start));
	p3 = NSMakePoint (center.x + radius * (-sin_start * sign),
			  center.y + radius *   cos_start * sign);
	
	[self curveToPoint: p3  controlPoint1: p1  controlPoint2: p2];
	startAngle_rad += diff;
      }
    else
      {
	/* Add the missing bit
	 * We require that the arc be less than a semicircle.
	 * The arc may go either clockwise or counterclockwise.
	 * The approximation is a very simple one: a single curve
	 * whose middle two control points are a fraction F of the way
	 * to the intersection of the tangents, where
	 *      F = (4/3) / (1 + sqrt (1 + (d / r)^2))
	 * where r is the radius and d is the distance from either tangent
	 * point to the intersection of the tangents. This produces
	 * a curve whose center point, as well as its ends, lies on
	 * the desired arc.
	 */
	NSPoint ps = [self currentPoint];
	/* tangent is the tangent of half the angle */
	CGFloat tangent = tan ((endAngle_rad - startAngle_rad) / 2);
	/* trad is the distance from either tangent point to the
	   intersection of the tangents */
	CGFloat trad = radius * tangent;
	/* pt is the intersection of the tangents */
	NSPoint pt = NSMakePoint (ps.x - trad * sin (startAngle_rad),
				  ps.y + trad * cos (startAngle_rad));
	/* This is F - in this expression we need to compute 
	   (trad/radius)^2, which is simply tangent^2 */
	CGFloat f = (4.0 / 3.0) / (1.0 + sqrt (1.0 +  (tangent * tangent)));
	
	p1 = NSMakePoint (ps.x + (pt.x - ps.x) * f, ps.y + (pt.y - ps.y) * f);
	p3 = NSMakePoint(center.x + radius * cos (endAngle_rad),
			 center.y + radius * sin (endAngle_rad));
	p2 = NSMakePoint (p3.x + (pt.x - p3.x) * f, p3.y + (pt.y - p3.y) * f);
	[self curveToPoint: p3  controlPoint1: p1  controlPoint2: p2];
	break;
      }
  }
}

- (void) appendBezierPathWithArcWithCenter: (NSPoint)center  
				    radius: (CGFloat)radius
				startAngle: (CGFloat)startAngle
				  endAngle: (CGFloat)endAngle
{
  [self appendBezierPathWithArcWithCenter: center  radius: radius
	startAngle: startAngle  endAngle: endAngle  clockwise: NO];
}

- (void) appendBezierPathWithArcFromPoint: (NSPoint)point1
				  toPoint: (NSPoint)point2
				   radius: (CGFloat)radius
{
  CGFloat x1, y1;
  CGFloat dx1, dy1, dx2, dy2;
  CGFloat l, a1, a2;
  NSPoint p;

  p = [self currentPoint];
 
  x1 = point1.x;
  y1 = point1.y;
  dx1 = p.x - x1;
  dy1 = p.y - y1;
  
  l= dx1*dx1 + dy1*dy1;
  if (l <= 0)
    {
      [self lineToPoint: point1];
      return;
    }
  l = 1/sqrt(l);
  dx1 *= l;
  dy1 *= l;
  
  dx2 = point2.x - x1;
  dy2 = point2.y - y1;

  l = dx2*dx2 + dy2*dy2;
  if (l <= 0)
    {
      [self lineToPoint: point1];
      return;
    }
	
  l = 1/sqrt(l);
  dx2 *= l; 
  dy2 *= l;
  
  l = dx1*dx2 + dy1*dy2;
  if (l < -0.999)
    {
      [self lineToPoint: point1];
      return;
    }

  l = radius/sin(acos(l));
  p.x = x1 + (dx1 + dx2)*l;
  p.y = y1 + (dy1 + dy2)*l;

  if (dx1 < -1)
    a1 = 180;
  else if (dx1 > 1)
    a1 = 0;
  else
    a1 = acos(dx1) / M_PI*180;
  if (dy1 < 0)
    {   
      a1 = -a1;
    }

  if (dx2 < -1)
    a2 = 180;
  else if (dx2 > 1)
    a2 = 0;
  else
    a2 = acos(dx2) / M_PI*180;
  if (dy2 < 0)
    {   
      a2 = -a2;
    }

  l = dx1*dy2 - dx2*dy1;
  if (l < 0)
    {
      a2 = a2 - 90;
      a1 = a1 + 90;
      [self appendBezierPathWithArcWithCenter: p  
	    radius: radius
	    startAngle: a1  
	    endAngle: a2  
	    clockwise: NO];
    }
  else
    {
      a2 = a2 + 90;
      a1 = a1 - 90;
      [self appendBezierPathWithArcWithCenter: p  
	    radius: radius
	    startAngle: a1  
	    endAngle: a2  
	    clockwise: YES];
    }
}

- (void)appendBezierPathWithGlyph:(NSGlyph)glyph inFont:(NSFont *)font
{
  [[font fontInfo] appendBezierPathWithGlyphs: &glyph
    count: 1
    toBezierPath: self];
}

- (void)appendBezierPathWithGlyphs:(NSGlyph *)glyphs 
			     count:(NSInteger)count
			    inFont:(NSFont *)font
{
  [[font fontInfo] appendBezierPathWithGlyphs: glyphs
    count: count
    toBezierPath: self];
}

- (void)appendBezierPathWithPackedGlyphs:(const char *)packedGlyphs
{
  [GSCurrentContext() appendBezierPathWithPackedGlyphs: packedGlyphs
                   path: self];
}

- (void) appendBezierPathWithRoundedRect: (NSRect)aRect
                                 xRadius: (CGFloat)xRadius
                                 yRadius: (CGFloat)yRadius
{
  NSPoint startp, endp, controlp1, controlp2, topLeft, topRight, bottomRight;

  xRadius = MIN(xRadius, aRect.size.width / 2.0);
  yRadius = MIN(yRadius, aRect.size.height / 2.0);

  if (xRadius == 0.0 || yRadius == 0.0)
    {
      [self appendBezierPathWithRect: aRect];
      return;
    }

  topLeft = NSMakePoint(NSMinX(aRect), NSMaxY(aRect));
  topRight = NSMakePoint(NSMaxX(aRect), NSMaxY(aRect));
  bottomRight = NSMakePoint(NSMaxX(aRect), NSMinY(aRect));

  startp = NSMakePoint(topLeft.x + xRadius, topLeft.y);
  endp = NSMakePoint(topLeft.x, topLeft.y - yRadius);
  controlp1 = NSMakePoint(startp.x - (KAPPA * xRadius), startp.y);
  controlp2 = NSMakePoint(endp.x, endp.y + (KAPPA * yRadius));
  [self moveToPoint: startp];
  [self curveToPoint: endp controlPoint1: controlp1 controlPoint2: controlp2];

  startp = NSMakePoint(aRect.origin.x, aRect.origin.y + yRadius);
  endp = NSMakePoint(aRect.origin.x + xRadius, aRect.origin.y);
  controlp1 = NSMakePoint(startp.x, startp.y - (KAPPA * yRadius));
  controlp2 = NSMakePoint(endp.x - (KAPPA * xRadius), endp.y);
  [self lineToPoint: startp];
  [self curveToPoint: endp controlPoint1: controlp1 controlPoint2: controlp2];

  startp = NSMakePoint(bottomRight.x - xRadius, bottomRight.y);
  endp = NSMakePoint(bottomRight.x, bottomRight.y + yRadius);
  controlp1 = NSMakePoint(startp.x + (KAPPA * xRadius), startp.y);
  controlp2 = NSMakePoint(endp.x, endp.y - (KAPPA * yRadius));
  [self lineToPoint: startp];
  [self curveToPoint: endp controlPoint1: controlp1 controlPoint2: controlp2];

  startp = NSMakePoint(topRight.x, topRight.y - yRadius);
  endp = NSMakePoint(topRight.x - xRadius, topRight.y);
  controlp1 = NSMakePoint(startp.x, startp.y + (KAPPA * yRadius));
  controlp2 = NSMakePoint(endp.x + (KAPPA * xRadius), endp.y);
  [self lineToPoint: startp];
  [self curveToPoint: endp controlPoint1: controlp1 controlPoint2: controlp2];

  [self closePath];
}

/* We use our own point structure with double elements while recursing to
   avoid losing accuracy at really fine subdivisions of curves.  */
typedef struct
{
  double x, y;
} double_point;

static int winding_line(double_point from, double_point to, double_point p)
{
  int y_dir;
  double k, x;

  if (from.y == to.y)
    return 0;

  if (to.y < from.y)
    {
      y_dir = -2;
      if (p.y < to.y)
	return 0;
      if (p.y > from.y)
	return 0;
    }
  else
    {
      y_dir = 2;
      if (p.y < from.y)
	return 0;
      if (p.y > to.y)
	return 0;
    }

  if (p.y == from.y || p.y == to.y)
    y_dir /= 2;

  /* The line is intersected.  Check if the intersection is outside the
     line's bounding box.  */
  if (to.x < from.x)
    {
      if (p.x < to.x)
	return 0;
      if (p.x > from.x)
	return y_dir;
    }
  else
    {
      if (p.x < from.x)
	return 0;
      if (p.x > to.x)
	return y_dir;
    }

  /* Determine the exact x coordinate of the intersection.  */
  k = (double)(from.x - to.x) / (double)(from.y - to.y);
  x = to.x + k * (double)(p.y - to.y);
  if (x < p.x)
    return y_dir;

  return 0;
}

static int winding_curve(double_point from, double_point to, double_point c1,
			 double_point c2, double_point p, int depth)
{
  double x0, x1;
  double y0, y1;
  double scale;

  /* Get the vertical extents of the convex hull.  */
  y0 = y1 = from.y;
  if (to.y < y0)
    y0 = to.y;
  else if (to.y > y1)
    y1 = to.y;
  if (c1.y < y0)
    y0 = c1.y;
  else if (c1.y > y1)
    y1 = c1.y;
  if (c2.y < y0)
    y0 = c2.y;
  else if (c2.y > y1)
    y1 = c2.y;

  /* If the point is outside the convex hull, the line can't intersect the
     curve.  */
  if (p.y < y0 || p.y > y1)
    return 0;

  /* Get the horizontal convex hull.  */
  x0 = x1 = from.x;
  if (to.x < x0)
    x0 = to.x;
  else if (to.x > x1)
    x1 = to.x;
  if (c1.x < x0)
    x0 = c1.x;
  else if (c1.x > x1)
    x1 = c1.x;
  if (c2.x < x0)
    x0 = c2.x;
  else if (c2.x > x1)
    x1 = c2.x;

  /* If the point is left of the convex hull, the line doesn't intersect
     the curve.  */
  if (p.x < x0)
    return 0;

  /* If the point is right of the convex hull, the net winding count is 0,
     1, or -1, and it depends only on how the end-points are placed in
     relation to the point.  Essentially, it's equivalent to a line.  */
  if (p.x > x1)
    return winding_line(from, to, p);

  /* Limit the recursion, just to be safe.  */
  if (depth >= 40)
    return winding_line(from, to, p);

  /* The line possibly intersects the curve in some interesting way.  If the
     curve is flat enough, we can pretend it's a line.  Otherwise, we
     subdivide and recurse.

     First, calculate a suitable scale based on the coordinates of the
     convex hull.  This is used to get a good cutoff for the subdivision.
     Since it's based on the coordinates in the curve, scaling the curve
     up or down won't affect relative accuracy.  Note that if the scale is
     zero, the convex hull, and thus the curve, has no extent.  */

  scale = fabs(x0) + fabs(x1) + fabs(y0) + fabs(y1);
  if (!scale)
    return 0;

  scale /= 40000000.0;

  /* Deal with the degenerate case to == from.  */
  if (to.x == from.x && to.y == from.y)
    {
      if (x1 - x0 < scale && y1 - y0 < scale)
	return winding_line(from, to, p);
    }
  else
    {
      double dx, dy;
      double nx, ny;
      double d0, d1, d2, d3;

      /* Get the direction vector and the normal vector.  */
      dx = to.x - from.x;
      dy = to.y - from.y;
      d0 = sqrt(dx * dx + dy * dy);
      dx /= d0;
      dy /= d0;
      nx = dy;
      ny = -dx;

      /* Check that the distances along the direction vector are
	 monotone.  */

      d0 = from.x * dx + from.y * dy;
      d1 = c1.x * dx + c1.y * dy;
      d2 = c2.x * dx + c2.y * dy;
      d3 = to.x * dx + to.y * dy;

      if ((d3 > d2 && d2 > d1 && d1 > d0)
	  || (d3 < d2 && d2 < d1 && d1 < d0))
	{
	  /* Check that the control points are close to the straigt line
	     between from and to.  */
	  d0 = to.x * nx + to.y * ny;
	  d1 = c1.x * nx + c1.y * ny;
	  d2 = c2.x * nx + c2.y * ny;

	  if (fabs(d0 - d1) < scale && fabs(d0 - d2) < scale)
	    {
	      /* It's flat enough.  */
	      return winding_line(from, to, p);
	    }
	}
    }

  {
    /* Subdivide.  */
    double_point m, l1, l2, r1, r2;

    m.x = (from.x + to.x + 3 * (c1.x + c2.x)) / 8;
    m.y = (from.y + to.y + 3 * (c1.y + c2.y)) / 8;

    l1.x = (from.x + c1.x) / 2;
    l1.y = (from.y + c1.y) / 2;

    l2.x = (from.x + 2 * c1.x + c2.x) / 4;
    l2.y = (from.y + 2 * c1.y + c2.y) / 4;

    r2.x = (to.x + c2.x) / 2;
    r2.y = (to.y + c2.y) / 2;

    r1.x = (to.x + 2 * c2.x + c1.x) / 4;
    r1.y = (to.y + 2 * c2.y + c1.y) / 4;

    return winding_curve(from, m, l1, l2, p, depth + 1)
	   + winding_curve(m, to, r1, r2, p, depth + 1);
  }
}

- (int) windingCountAtPoint: (NSPoint)point
{
  int total;
  NSBezierPathElement type;
  NSInteger count;
  BOOL first;
  NSPoint pts[3];
  NSPoint first_p, last_p;
  NSInteger i;

  /* We trace a line from (-INF, point.y) to (point) and count the
     intersections.  Simple, really. ;)

     Lines are trivially checked with a few complications:

     a. Tangent lines, i.e. horizontal lines.  These can be ignored since
	the winding count is undefined on edges.

     b. Lines whose endpoints are touched by our infinite line.  To get
	these right, we return half a winding for such intersections.
	Except for intermediate horizontal lines, which are ignored, the
	next line will also be intersected in one endpoint.  If it's going
	in the same direction as the first line, the two half intersections
	will add up to one real intersection.  If it isn't, the two half
	intersections with opposite signs will cancel out.  Either way, we
	get the right results.

	(If this happens for the first element, s/next/previous/ in the
	explanation.  In practice, we double the winding counts while
	working and divide by 2 just before returning.)

     Curves are checked first using the convex hull, and if necessary, by
     subdividing until they are flat enough to check as lines.  We use a
     very fine subdivision, and thus get good accuracy.  This is possible
     because only the parts of the curve that might intersect the line are
     subdivided (due to the convex hull checks).  */

  total = 0;
  count = [self elementCount];

  if (count == 0)
    return 0;

  /* 'Unroll' the first element to avoid compiler warnings.  It has to be
     a MoveTo, anyway.  */
  type = [self elementAtIndex: 0 associatedPoints: pts];
  if (type != NSMoveToBezierPathElement)
    {
      NSWarnLog(@"Invalid path, first element isn't MoveTo.");
      return 0;
    }
  last_p = first_p = pts[0];
  first = NO;

#define D(a) (double_point){a.x,a.y}
  for (i = 1; i < count; i++)
    {
      type = [self elementAtIndex: i associatedPoints: pts];
      switch(type)
	{
	  case NSMoveToBezierPathElement:
	    if (!first)
	      {
		total += winding_line(D(last_p), D(first_p), D(point));
	      }
	    last_p = first_p = pts[0];
	    first = NO;
	    break;
	  case NSLineToBezierPathElement:
	    if (first)
	      {
		NSWarnLog(@"Invalid path, LineTo without MoveTo.");
		return 0;
	      }
	    total += winding_line(D(last_p), D(pts[0]), D(point));
	    last_p = pts[0];
	    break;
	  case NSCurveToBezierPathElement:
	    if (first)
	      {
		NSWarnLog(@"Invalid path, CurveTo without MoveTo.");
		return 0;
	      }
	    total += winding_curve(D(last_p), D(pts[2]), D(pts[0]), D(pts[1]), D(point), 0);
	    last_p = pts[2];
	    break;
	  case NSClosePathBezierPathElement:
	    if (first)
	      {
		NSWarnLog(@"Invalid path, ClosePath with no open subpath.");
		return 0;
	      }
	    first = YES;
	    total += winding_line(D(last_p), D(first_p), D(point));
	    break;
	  default:
	    NSWarnLog(@"Invalid element in path.");
	    return 0;
	}
    }

  if (!first)
    total += winding_line(D(last_p), D(first_p), D(point));
#undef D

  if (total & 1)
    {
      /* This should only happen for points on edges, and the winding
	 count is undefined at such points.  */
      NSDebugLLog(@"NSBezierPath", @"winding count total is odd");
    }
  return total / 2;
}

- (BOOL) containsPoint: (NSPoint)point
{
  int sum;

  if (![self elementCount])
    return NO;

  if (!NSPointInRect(point, [self bounds]))
    return NO;

  sum = [self windingCountAtPoint: point];
  if ([self windingRule] == NSNonZeroWindingRule)
    {
      if (sum == 0)
	return NO;
      else	
	return YES;
    }
  else 
    {
      if ((sum % 2) == 0)
	return NO;
      else	
	return YES;
    }
}

//
// Caching paths 
// 
//
// Caching
// 
- (BOOL)cachesBezierPath
{
  return _cachesBezierPath;
}

- (void)setCachesBezierPath:(BOOL)flag
{
  _cachesBezierPath = flag;

  if (!flag)
    INVALIDATE_CACHE();
}

//
// NSCoding protocol
//
- (void)encodeWithCoder:(NSCoder *)aCoder
{
  NSBezierPathElement type;
  NSPoint pts[3];

  if ([aCoder allowsKeyedCoding])
    {
      NSUInteger count, i;
      NSMutableData *d;
      float x,y;
      char ctype;

      [aCoder encodeFloat: [self flatness] forKey: @"NSFlatness"];
      [aCoder encodeFloat: [self lineWidth] forKey: @"NSLineWidth"];
      [aCoder encodeInt: [self lineCapStyle] forKey: @"NSLineCapStyle"];
      [aCoder encodeInt: [self lineJoinStyle] forKey: @"NSLineJoinStyle"];
      [aCoder encodeInt: [self windingRule] forKey: @"NSWindingRule"];
      [aCoder encodeFloat: [self miterLimit] forKey: @"NSMiterLimit"];

      count = [self elementCount];
      d = [[NSMutableData alloc] init];
      for (i = 0; i < count; i++)
        {
          type = [self elementAtIndex: i associatedPoints: pts];
          ctype = type; 
          [d serializeDataAt: &ctype
                  ofObjCType: "c"
                     context: nil];

          switch (type)
            {
            case NSMoveToBezierPathElement:
            case NSLineToBezierPathElement:
              x = pts[0].x;
              y = pts[0].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              break;
            case NSCurveToBezierPathElement:
              x = pts[0].x;
              y = pts[0].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &ctype
                      ofObjCType: "c"
                         context: nil];
              x = pts[1].x;
              y = pts[1].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &ctype
                      ofObjCType: "c"
                         context: nil];
              x = pts[2].x;
              y = pts[2].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              break;
            case NSClosePathBezierPathElement:
              x = pts[0].x;
              y = pts[0].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &ctype
                      ofObjCType: "c"
                         context: nil];
              x = pts[0].x;
              y = pts[0].y;
              [d serializeDataAt: &x
                      ofObjCType: "f"
                         context: nil];
              [d serializeDataAt: &y
                      ofObjCType: "f"
                         context: nil];
              break;
           default:
              break;
           }
        }
      [aCoder encodeBytes: [d bytes]
                   length: [d length]
                   forKey: @"NSSegments"];
      RELEASE(d);
    }
  else
    {
      int i, count;
      float f;
      
      f = [self lineWidth];
      [aCoder encodeValueOfObjCType: @encode(float) at: &f];
      i = [self lineCapStyle];
      [aCoder encodeValueOfObjCType: @encode(int) at: &i];
      i = [self lineJoinStyle];
      [aCoder encodeValueOfObjCType: @encode(int) at: &i];
      i = [self windingRule];
      [aCoder encodeValueOfObjCType: @encode(int) at: &i];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_cachesBezierPath];

      // version 2
      f = [self flatness];
      [aCoder encodeValueOfObjCType: @encode(float) at: &f];
      f = [self miterLimit];
      [aCoder encodeValueOfObjCType: @encode(float) at: &f];
      
      count = [self elementCount];
      [aCoder encodeValueOfObjCType: @encode(int) at: &count];
      
      for (i = 0; i < count; i++) 
        {
          type = [self elementAtIndex: i associatedPoints: pts];
          [aCoder encodeValueOfObjCType: @encode(int) at: &type];
          switch(type) 
            {
            case NSMoveToBezierPathElement:
            case NSLineToBezierPathElement:
	      [aCoder encodePoint: pts[0]];
	      break;
            case NSCurveToBezierPathElement:
	      [aCoder encodePoint: pts[0]];
	      [aCoder encodePoint: pts[1]];
	      [aCoder encodePoint: pts[2]];
	      break;
            case NSClosePathBezierPathElement:
	      break;
            default:
	      break;
            }
        }
    }
}

- (id)initWithCoder:(NSCoder *)aCoder
{
  // We have to init the place to store the elements
  [self init];
  _cacheImage = nil;
  _shouldRecalculateBounds = YES;

  if ([aCoder allowsKeyedCoding])
    {
      float f;
      int i;

      if ([aCoder containsValueForKey: @"NSFlatness"])
        {
          f = [aCoder decodeFloatForKey: @"NSFlatness"];
          [self setFlatness: f];
        }
      if ([aCoder containsValueForKey: @"NSLineWidth"])
        {
          f = [aCoder decodeFloatForKey: @"NSLineWidth"];
          [self setLineWidth: f];
        }
      if ([aCoder containsValueForKey: @"NSLineCapStyle"])
        {
          i = [aCoder decodeIntForKey: @"NSLineCapStyle"];
          [self setLineCapStyle: i];
        }
      if ([aCoder containsValueForKey: @"NSLineJoinStyle"])
        {
          i = [aCoder decodeIntForKey: @"NSLineJoinStyle"];
          [self setLineJoinStyle: i];
        }
      if ([aCoder containsValueForKey: @"NSWindingRule"])
        {
          i = [aCoder decodeIntForKey: @"NSWindingRule"];
          [self setWindingRule: i];
        }
      if ([aCoder containsValueForKey: @"NSMiterLimit"])
        {
          f = [aCoder decodeFloatForKey: @"NSMiterLimit"];
          [self setMiterLimit: f];
        }

      if ([aCoder containsValueForKey: @"NSSegments"])
        {
	  NSUInteger length;
	  const uint8_t *data;
          NSData *d;
          unsigned int cursor = 0;

          data = [aCoder decodeBytesForKey: @"NSSegments"
                              returnedLength: &length]; 
          d = [NSData dataWithBytes: data length: length];
          //NSLog(@"decoded segments %@", d);
          while (cursor < length)
            {
              char c;
              float f, g;
              NSPoint p, cp1, cp2;

              [d deserializeDataAt: &c
                        ofObjCType: "c"
                          atCursor: &cursor
                           context: nil];
              switch (c) 
                {
                case NSMoveToBezierPathElement:
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  p = NSMakePoint(f, g);
                  [self moveToPoint: p];
                  //NSLog(@"Decoded move %@", NSStringFromPoint(p));
                  break;
                case NSLineToBezierPathElement:

                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  p = NSMakePoint(f, g);
                  [self lineToPoint: p];
                  //NSLog(@"Decoded line %@", NSStringFromPoint(p));
                  break;
                case NSCurveToBezierPathElement:
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  cp1 = NSMakePoint(f, g);
                  [d deserializeDataAt: &c
                            ofObjCType: "c"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  cp2 = NSMakePoint(f, g);
                  [d deserializeDataAt: &c
                            ofObjCType: "c"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  p = NSMakePoint(f, g);
                  [self curveToPoint: p
                       controlPoint1: cp1
                       controlPoint2: cp2];
                  //NSLog(@"Decoded curve %@ %@ %@", NSStringFromPoint(p), NSStringFromPoint(cp1), NSStringFromPoint(cp2));
                  break;
                case NSClosePathBezierPathElement:
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  p = NSMakePoint(f, g);
                  [d deserializeDataAt: &c
                            ofObjCType: "c"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &f
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  [d deserializeDataAt: &g
                            ofObjCType: "f"
                              atCursor: &cursor
                               context: nil];
                  cp1 = NSMakePoint(f, g);
                  //NSLog(@"Decoded close %@ %@", NSStringFromPoint(p), NSStringFromPoint(cp1));
                  [self closePath];
                  break;
                default:
                  //NSLog(@"Unable to decode unknown bezier path element type %d", c);
                  break;
                }
            }
        }
    }
  else
    {
      NSBezierPathElement type;
      NSPoint pts[3];
      int i, count;
      float f;
      int version = [aCoder versionForClassName: @"NSBezierPath"];
      
      [aCoder decodeValueOfObjCType: @encode(float) at: &f];
      [self setLineWidth: f];
      [aCoder decodeValueOfObjCType: @encode(int) at: &i];
      [self setLineCapStyle: i];
      [aCoder decodeValueOfObjCType: @encode(int) at: &i];
      [self setLineJoinStyle: i];
      [aCoder decodeValueOfObjCType: @encode(int) at: &i];
      [self setWindingRule: i];
      [aCoder decodeValueOfObjCType: @encode(BOOL) at: &_cachesBezierPath];

      if (version >= 2)
        {
          [aCoder decodeValueOfObjCType: @encode(float) at: &f];
          [self setFlatness: f];
          [aCoder decodeValueOfObjCType: @encode(float) at: &f];
          [self setMiterLimit: f];
        }

      [aCoder decodeValueOfObjCType: @encode(int) at: &count];
      
      for (i = 0; i < count; i++) 
        {
          [aCoder decodeValueOfObjCType: @encode(int) at: &type];
          switch(type) 
            {
            case NSMoveToBezierPathElement:
	      pts[0] = [aCoder decodePoint];
	      [self moveToPoint: pts[0]];
	      break;
            case NSLineToBezierPathElement:
	      pts[0] = [aCoder decodePoint];
	      [self lineToPoint: pts[0]];
	      break;
            case NSCurveToBezierPathElement:
	      pts[0] = [aCoder decodePoint];
	      pts[1] = [aCoder decodePoint];
	      pts[2] = [aCoder decodePoint];
	      [self curveToPoint: pts[2] controlPoint1: pts[0] controlPoint2: pts[1]];
	      break;
            case NSClosePathBezierPathElement:
	      [self closePath];
	      break;
            default:
	      break;
            }
        }
    }

  return self;
}

//
// NSCopying Protocol
//
- (id)copyWithZone:(NSZone *)zone
{
  NSBezierPath *path = (NSBezierPath*)NSCopyObject (self, 0, zone);

  if (_cachesBezierPath && _cacheImage)
      path->_cacheImage = [_cacheImage copy];

  if (_dash_pattern != NULL)
    {
      CGFloat *pattern = NSZoneMalloc(zone, _dash_count * sizeof(CGFloat));

      memcpy(pattern, _dash_pattern, _dash_count * sizeof(CGFloat));
      _dash_pattern = pattern;
    }

  path->_pathElements = GSIArrayCopyWithZone(_pathElements, zone);

  return path;
}

@end


@implementation NSBezierPath (PrivateMethods)

- (void) _invalidateCache
{
  _shouldRecalculateBounds = YES;
  DESTROY(_cacheImage);
}


/* Helper for -_recalculateBounds. */
static NSPoint point_on_curve(double t, NSPoint a, NSPoint b, NSPoint c,
			      NSPoint d)
{
  double ti = 1.0 - t;
  return NSMakePoint(ti * ti * ti * a.x + 3 * ti * ti * t * b.x
		       + 3 * ti * t * t * c.x + t * t * t * d.x,
		     ti * ti * ti * a.y + 3 * ti * ti * t * b.y
		       + 3 * ti * t * t * c.y + t * t * t * d.y);
}

- (void)_recalculateBounds
{
  NSBezierPathElement type;
  NSPoint p; /* Current point. */
  NSPoint pts[3];
  NSPoint min, max;   /* Path bounding box. */
  NSPoint cmin, cmax; /* Control-point bounding box. */
  int i, count, num_curves;
  
  count = [self elementCount];
  if (!count)
    {
      _bounds = NSZeroRect;
      _controlPointBounds = NSZeroRect;
      _shouldRecalculateBounds = NO;
      return;
    }

  p = min = max = cmin = cmax = NSMakePoint(0, 0);

#define CHECK_MAX(max, p) \
  if (p.x > max.x) max.x = p.x; \
  if (p.y > max.y) max.y = p.y;
#define CHECK_MIN(min, p) \
  if (p.x < min.x) min.x = p.x; \
  if (p.y < min.y) min.y = p.y;

  num_curves = 0;
  for (i = 0; i < count; i++)
    {
      type = [self elementAtIndex: i  associatedPoints: pts];

      if (i == 0)
	{
	  cmin = cmax = min = max = p = pts[0];
	}

      switch (type)
	{
          case NSClosePathBezierPathElement:
	    p = pts[0];
	    continue;

	  case NSMoveToBezierPathElement:
	  case NSLineToBezierPathElement:
	    CHECK_MAX(max, pts[0])
	    CHECK_MIN(min, pts[0])
	    p = pts[0];
	    break;

	  case NSCurveToBezierPathElement:
	    {
	      double t0, t1, t;
	      NSPoint q;

	      num_curves++;
	      CHECK_MAX(cmax, pts[0])
	      CHECK_MIN(cmin, pts[0])
	      CHECK_MAX(cmax, pts[1])
	      CHECK_MIN(cmin, pts[1])

	      CHECK_MAX(max, pts[2])
	      CHECK_MIN(min, pts[2])

#define CHECK_CURVE_EXTREMES(x) \
	      t = (p.x * (pts[2].x - pts[1].x) \
		   + pts[0].x * (-pts[2].x - pts[1].x) \
		   + pts[1].x * pts[1].x + pts[0].x * pts[0].x); \
	      if (t >= 0.0) \
		{ \
		  t = sqrt(t); \
		  t0 = (pts[1].x - 2 * pts[0].x + p.x + t) \
		       / (-pts[2].x + 3 * pts[1].x - 3 * pts[0].x + p.x); \
		  t1 = (pts[1].x - 2 * pts[0].x + p.x - t) \
		       / (-pts[2].x + 3 * pts[1].x - 3 * pts[0].x + p.x); \
\
		  if (t0 > 0.0 && t0 < 1.0) \
		    { \
		      q = point_on_curve(t0, p, pts[0], pts[1], pts[2]); \
		      CHECK_MAX(max, q) \
		      CHECK_MIN(min, q) \
		    } \
		  if (t1 > 0.0 && t1 < 1.0) \
		    { \
		      q = point_on_curve(t1, p, pts[0], pts[1], pts[2]); \
		      CHECK_MAX(max, q) \
		      CHECK_MIN(min, q) \
		    } \
		}

	      CHECK_CURVE_EXTREMES(x)
	      CHECK_CURVE_EXTREMES(y)

#undef CHECK_CURVE_EXTREMES

	      p = pts[2];
	      break;
	    }
	}
    }

  /* If there were no curve elements, the control-point bounding box is the
  same as the path bounding box. Otherwise, the control-point bounding box
  is the union of the path bounding box and the bounding box of the curve
  control points. */
  if (num_curves)
    {
      CHECK_MAX(cmax, max)
      CHECK_MIN(cmin, min)
    }
  else
    {
      cmin = min;
      cmax = max;
    }

  _bounds = NSMakeRect(min.x, min.y, max.x - min.x, max.y - min.y);
  _controlPointBounds = NSMakeRect(cmin.x, cmin.y,
				   cmax.x - cmin.x, cmax.y - cmin.y);
  _shouldRecalculateBounds = NO;
#undef CHECK_MAX
#undef CHECK_MIN
}

@end

#if 0
@implementation GSBezierPath

- (void) appendBezierPath: (NSBezierPath *)aPath
{
  PathElement elem;
  int i, count;

  if (![aPath isKindOfClass: object_getClass(self)])
    {
      [super appendBezierPath: aPath];
      return;
    }

  flat = flat && ((GSBezierPath*)aPath)->flat;
  count = [aPath elementCount];

  for (i = 0; i < count; i++)
    {
      elem = GSIArrayItemAtIndex(((GSBezierPath*)aPath)->pathElements, i).ext;
      GSIArrayAddItem(pathElements, (GSIArrayItem)elem);
    }
  INVALIDATE_CACHE();
}

@end // GSBezierPath
#endif

static void flatten(NSPoint coeff[], CGFloat flatness, NSBezierPath *path)
{
  // Check if the Bezier path defined by the four points has the given flatness.
  // If not split it up in the middle and recurse. 
  // Otherwise add the end point to the path.
  BOOL flat = YES;

  // This criteria for flatness is based on code from Libart which has the 
  // following copyright:
/* Libart_LGPL - library of basic graphic primitives
 * Copyright (C) 1998 Raph Levien
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

  double x1_0, y1_0;
  double x3_2, y3_2;
  double x3_0, y3_0;
  double z3_0_dot;
  double z1_dot, z2_dot;
  double z1_perp, z2_perp;
  double max_perp_sq;

  x3_0 = coeff[3].x - coeff[0].x;
  y3_0 = coeff[3].y - coeff[0].y;
  x3_2 = coeff[3].x - coeff[2].x;
  y3_2 = coeff[3].y - coeff[2].y;
  x1_0 = coeff[1].x - coeff[0].x;
  y1_0 = coeff[1].y - coeff[0].y;
  z3_0_dot = x3_0 * x3_0 + y3_0 * y3_0;

  if (z3_0_dot < 0.001)
    flat = YES;
  else
    {
      max_perp_sq = flatness * flatness * z3_0_dot;

      z1_perp = y1_0 * x3_0 - x1_0 * y3_0;
      if (z1_perp * z1_perp > max_perp_sq)
	flat = NO;
      else
        {
	  z2_perp = y3_2 * x3_0 - x3_2 * y3_0;
	  if (z2_perp * z2_perp > max_perp_sq)
	      flat = NO;
	  else
	    {
	      z1_dot = x1_0 * x3_0 + y1_0 * y3_0;
	      if (z1_dot < 0 && z1_dot * z1_dot > max_perp_sq)
		flat = NO;
	      else
	        {
		  z2_dot = x3_2 * x3_0 + y3_2 * y3_0;
		  if (z2_dot < 0 && z2_dot * z2_dot > max_perp_sq)
		      flat = NO;
		  else
		    {
		      if ((z1_dot + z1_dot > z3_0_dot) ||
			  (z2_dot + z2_dot > z3_0_dot))
			flat = NO;
		    }
		}
	    }
	}
    }

  if (!flat)
    {
      NSPoint bleft[4], bright[4];
	
      bleft[0] = coeff[0];
      bleft[1].x = (coeff[0].x + coeff[1].x) / 2;
      bleft[1].y = (coeff[0].y + coeff[1].y) / 2;
      bleft[2].x = (coeff[0].x + 2*coeff[1].x + coeff[2].x) / 4;
      bleft[2].y = (coeff[0].y + 2*coeff[1].y + coeff[2].y) / 4;
      bleft[3].x = (coeff[0].x + 3*(coeff[1].x + coeff[2].x) + coeff[3].x) / 8;
      bleft[3].y = (coeff[0].y + 3*(coeff[1].y + coeff[2].y) + coeff[3].y) / 8;
      bright[0].x =  bleft[3].x;
      bright[0].y =  bleft[3].y;
      bright[1].x = (coeff[3].x + 2*coeff[2].x + coeff[1].x) / 4;
      bright[1].y = (coeff[3].y + 2*coeff[2].y + coeff[1].y) / 4;
      bright[2].x = (coeff[3].x + coeff[2].x) / 2;
      bright[2].y = (coeff[3].y + coeff[2].y) / 2;
      bright[3] = coeff[3];

      flatten(bleft, flatness, path);
      flatten(bright, flatness, path);
    }
  else
    {
      //[path lineToPoint: coeff[1]];
      //[path lineToPoint: coeff[2]];
      [path lineToPoint: coeff[3]];
    }
}

