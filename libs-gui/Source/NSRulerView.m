/*
   NSRulerView.m

   Copyright (C) 2002 Free Software Foundation, Inc.

   Author: Diego Kreutz (kreutz@inf.ufsm.br)
   Date: January 2002

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

#include <math.h>
#import "config.h"

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSRulerMarker.h"
#import "AppKit/NSRulerView.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSStringDrawing.h"
#import "GSGuiPrivate.h"

#define MIN_LABEL_DISTANCE 40
#define MIN_MARK_DISTANCE 5

#define MARK_SIZE 2
#define MID_MARK_SIZE 4
#define BIG_MARK_SIZE 6
#define LABEL_MARK_SIZE 11

#define RULER_THICKNESS 16
#define MARKER_THICKNESS 15

#define DRAW_HASH_MARK(path, size)			\
		do {					\
		  if (_orientation == NSHorizontalRuler)\
		    {					\
		      [path relativeLineToPoint: NSMakePoint(0, size)];	\
		    }					\
		  else					\
		    {					\
		      [path relativeLineToPoint: NSMakePoint(size, 0)];	\
		    }					\
		} while (0)

@interface GSRulerUnit : NSObject
{
  NSString *_unitName;
  NSString *_abbreviation;
  CGFloat   _conversionFactor;
  NSArray  *_stepUpCycle;
  NSArray  *_stepDownCycle;
}

+ (GSRulerUnit *) unitWithName: (NSString *)uName 
                  abbreviation: (NSString *)abbrev 
  unitToPointsConversionFactor: (CGFloat)factor 
                   stepUpCycle: (NSArray *)upCycle 
                 stepDownCycle: (NSArray *)downCycle;
- (id) initWithUnitName: (NSString *)uName 
                abbreviation: (NSString *)abbrev
unitToPointsConversionFactor: (CGFloat)factor 
                 stepUpCycle: (NSArray *)upCycle 
               stepDownCycle: (NSArray *)downCycle;
- (NSString *) unitName;
- (NSString *) abbreviation;
- (CGFloat) conversionFactor;
- (NSArray *) stepUpCycle;
- (NSArray *) stepDownCycle;

@end

@implementation GSRulerUnit

+ (GSRulerUnit *) unitWithName: (NSString *)uName 
                  abbreviation: (NSString *)abbrev 
  unitToPointsConversionFactor: (CGFloat)factor 
                   stepUpCycle: (NSArray *)upCycle 
                 stepDownCycle: (NSArray *)downCycle
{
  return [[[self alloc] initWithUnitName: uName 
                            abbreviation: abbrev 
            unitToPointsConversionFactor: factor 
                             stepUpCycle: upCycle 
                           stepDownCycle: downCycle] autorelease];
}

- (id) initWithUnitName: (NSString *)uName 
	   abbreviation: (NSString *)abbrev 
unitToPointsConversionFactor: (CGFloat)factor 
	    stepUpCycle: (NSArray *)upCycle 
	  stepDownCycle: (NSArray *)downCycle
{
  self = [super init];
  if (self != nil) 
    {
      ASSIGN(_unitName, uName);
      ASSIGN(_abbreviation, abbrev);
      _conversionFactor = factor;
      ASSIGN(_stepUpCycle, upCycle);
      ASSIGN(_stepDownCycle, downCycle);
    }
  
  return self;
}

- (NSString *) unitName
{
  return _unitName;
}

- (NSString *) abbreviation
{
  return _abbreviation;
}

- (CGFloat) conversionFactor
{
  return _conversionFactor;
}

- (NSArray *) stepUpCycle
{
  return _stepUpCycle;
}

- (NSArray *) stepDownCycle
{
  return _stepDownCycle;
}

- (void) dealloc
{
  [_unitName release];
  [_abbreviation release];
  [_stepUpCycle release];
  [_stepDownCycle release];
  [super dealloc];
}

@end


@implementation NSRulerView

/*
 * Class variables
 */
static NSMutableDictionary *units = nil;

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSRulerView class]) 
    {
      NSArray *array05;
      NSArray *array052;
      NSArray *array2;
      NSArray *array10;

      [self setVersion: 1];

      units = [[NSMutableDictionary alloc] init];
      array05 = [NSArray arrayWithObject: [NSNumber numberWithFloat: 0.5]];
      array052 = [NSArray arrayWithObjects: [NSNumber numberWithFloat: 0.5], 
                 [NSNumber numberWithFloat: 0.2], nil];
      array2 = [NSArray arrayWithObject: [NSNumber numberWithFloat: 2.0]];
      array10 = [NSArray arrayWithObject: [NSNumber numberWithFloat: 10.0]];
      [self registerUnitWithName: @"Inches" 
                    abbreviation: @"in" 
    unitToPointsConversionFactor: 72.0 
                     stepUpCycle: array2
                   stepDownCycle: array05];
      [self registerUnitWithName: @"Centimeters" 
                    abbreviation: @"cm" 
    unitToPointsConversionFactor: 28.35 
                     stepUpCycle: array2
                   stepDownCycle: array052];
      [self registerUnitWithName: @"Points" 
                    abbreviation: @"pt" 
    unitToPointsConversionFactor: 1.0 
                     stepUpCycle: array10
                   stepDownCycle: array05];
      [self registerUnitWithName: @"Picas" 
                    abbreviation: @"pc" 
    unitToPointsConversionFactor: 12.0 
                     stepUpCycle: array2
                   stepDownCycle: array05];
    }
}

- (id) initWithScrollView: (NSScrollView *)aScrollView 
              orientation: (NSRulerOrientation)o
{
  self = [super initWithFrame: NSZeroRect];
  if (self != nil)
    {
      [self setScrollView: aScrollView];
      [self setOrientation: o];
      [self setMeasurementUnits: @"Points"]; /* FIXME: should be user's pref */
      [self setRuleThickness: RULER_THICKNESS];
      [self setOriginOffset: 0.0];
      [self setReservedThicknessForAccessoryView: 0.0];
      if (o == NSHorizontalRuler)
        {
          [self setReservedThicknessForMarkers: MARKER_THICKNESS];
	}
      else
        {
          [self setReservedThicknessForMarkers: 0.0];
	}
      [self invalidateHashMarks];
    }
  return self;
}
  
+ (void) registerUnitWithName: (NSString *)uName
                 abbreviation: (NSString *)abbreviation
 unitToPointsConversionFactor: (CGFloat)conversionFactor 
                  stepUpCycle: (NSArray *)stepUpCycle
                stepDownCycle: (NSArray *)stepDownCycle
{
  GSRulerUnit *u = [GSRulerUnit unitWithName: uName
				abbreviation: abbreviation
				unitToPointsConversionFactor: conversionFactor
				stepUpCycle: stepUpCycle
				stepDownCycle: stepDownCycle];
  [units setObject: u forKey: uName];
}

- (void) setMeasurementUnits: (NSString *)uName
{
  GSRulerUnit *newUnit;
  
  newUnit = [units objectForKey: uName];
  if (newUnit == nil) 
    {
      [NSException raise: NSInvalidArgumentException
		   format: @"Unknown measurement unit %@", uName];
    }
  ASSIGN(_unit, newUnit);
  [self invalidateHashMarks];
}

- (NSString *) measurementUnits
{
  return [_unit unitName];
}

- (void) setClientView: (NSView *)aView
{
  if (_clientView == aView)
    return;

  if (_clientView != nil  
      && [_clientView respondsToSelector: 
			@selector(rulerView:willSetClientView:)]) 
    {
      [_clientView rulerView: self willSetClientView: aView];
    }
  /* NB: We should not RETAIN the clientView.  */
  _clientView = aView;
  [self setMarkers: nil];
  [self invalidateHashMarks];
}

- (BOOL) isOpaque
{
  return YES;
}

- (NSView *) clientView
{
  return _clientView;
}

- (void) setAccessoryView: (NSView *)aView
{
  /* FIXME/TODO: support for accessory views is not implemented */
  ASSIGN(_accessoryView, aView);
  [self setNeedsDisplay: YES];
}

- (NSView *) accessoryView
{
  return _accessoryView;
}

- (void) setOriginOffset: (CGFloat)offset
{
  _originOffset = offset;
  [self invalidateHashMarks];
}

- (CGFloat) originOffset
{
  return _originOffset;
}

- (void) _verifyReservedThicknessForMarkers
{
  NSEnumerator *en;
  NSRulerMarker *marker;
  CGFloat maxThickness = _reservedThicknessForMarkers;
  
  if (_markers == nil)
    {
      return;
    }
  en = [_markers objectEnumerator];
  while ((marker = [en nextObject]) != nil)
    {
      CGFloat markerThickness;
      markerThickness = [marker thicknessRequiredInRuler];
      if (markerThickness > maxThickness)
        {
	  maxThickness = markerThickness;
	}
    }
  if (maxThickness > _reservedThicknessForMarkers)
    {
      [self setReservedThicknessForMarkers: maxThickness];
    }
}

- (void) setMarkers: (NSArray *)newMarkers
{
  if (newMarkers != nil && _clientView == nil)
    {
      [NSException raise: NSInternalInconsistencyException
                  format: @"Cannot set markers without a client view"];
    }
  if (newMarkers != nil)
    {
      ASSIGN(_markers, [NSMutableArray arrayWithArray: newMarkers]);
      [self _verifyReservedThicknessForMarkers];
    }
  else
    {
      ASSIGN(_markers, nil);
    }
  [self setNeedsDisplay: YES];
}

- (NSArray *) markers
{
  return _markers;
}

- (void) addMarker: (NSRulerMarker *)aMarker
{
  CGFloat markerThickness = [aMarker thicknessRequiredInRuler];

  if (_clientView == nil)
    {
      [NSException raise: NSInternalInconsistencyException
                 format: @"Cannot add a marker without a client view"];
    }
   
  if (markerThickness > [self reservedThicknessForMarkers])
    {
      [self setReservedThicknessForMarkers: markerThickness];
    }
  if (_markers == nil) 
    {
      _markers = [[NSMutableArray alloc] initWithObjects: aMarker, nil];
    }
  else 
    {
      [_markers addObject: aMarker];
    }
  
  [self setNeedsDisplay: YES];
}

- (void) removeMarker: (NSRulerMarker *)aMarker
{
  [_markers removeObject: aMarker];
  [self setNeedsDisplay: YES];
}

- (BOOL) trackMarker: (NSRulerMarker *)aMarker
      withMouseEvent: (NSEvent *)theEvent
{
  NSParameterAssert(aMarker != nil);
  
  return [aMarker trackMouse: theEvent adding: YES];
}

- (NSRect) _rulerRect
{
  NSRect rect = [self bounds]; 
  if (_orientation == NSHorizontalRuler)
    {
      rect.size.height = _ruleThickness;
      if ([self isFlipped])
        {
	  rect.origin.y = [self baselineLocation];
	}
      else
        {
	  rect.origin.y = [self baselineLocation] - _ruleThickness;
	}
    }
  else
    {
      rect.size.width = _ruleThickness;
      rect.origin.x = [self baselineLocation];
    }
  return rect;
}

- (NSRect) _markersRect
{
  NSRect rect = [self bounds]; 
  if (_orientation == NSHorizontalRuler)
    {
      rect.size.height = _reservedThicknessForMarkers;
      if ([self isFlipped])
        {
	  rect.origin.y = _reservedThicknessForAccessoryView;
	}
      else
        {
	  rect.origin.y = _ruleThickness;
	}
    }
  else
    {
      rect.size.width = _reservedThicknessForMarkers;
      rect.origin.x = _reservedThicknessForAccessoryView;
    }
  return rect;
}

- (NSRulerMarker *) _markerAtPoint: (NSPoint)point
{
  NSEnumerator *markerEnum;
  NSRulerMarker *marker;
  BOOL flipped = [self isFlipped];
  
  /* test markers in reverse order so that markers drawn on top
     are tested before those underneath */
  markerEnum = [_markers reverseObjectEnumerator];
  while ((marker = [markerEnum nextObject]) != nil)
    {
      if (NSMouseInRect (point, [marker imageRectInRuler], flipped))
        {
	  return marker;
	}
    }
  return nil;
}

- (void) mouseDown: (NSEvent*)theEvent
{
  NSPoint clickPoint;
  BOOL flipped = [self isFlipped];
  
  clickPoint = [self convertPoint: [theEvent locationInWindow]
                         fromView: nil];
  if (NSMouseInRect (clickPoint, [self _rulerRect], flipped))
    {
      if (_clientView != nil  
          && [_clientView respondsToSelector: 
			    @selector(rulerView:handleMouseDown:)]) 
        {
          [_clientView rulerView: self handleMouseDown: theEvent];
        }
    }
  else if (NSMouseInRect (clickPoint, [self _markersRect], flipped))
    {
      NSRulerMarker *clickedMarker;
      
      clickedMarker = [self _markerAtPoint: clickPoint];
      if (clickedMarker != nil)
        {
	  [clickedMarker trackMouse: theEvent adding: NO];
	}
    }
}

- (void) moveRulerlineFromLocation: (CGFloat)oldLoc 
                        toLocation: (CGFloat)newLoc
{
  /* FIXME/TODO: not implemented */
}

- (void)drawRect: (NSRect)aRect
{
  [[NSColor controlColor] set];
  NSRectFill(aRect);
  [self drawHashMarksAndLabelsInRect: aRect];
  [self drawMarkersInRect: aRect];
}

- (float) _stepForIndex: (int)index
{
  int newindex;
  NSArray *stepCycle;

  if (index > 0) 
    {
      stepCycle = [_unit stepUpCycle];
      newindex = (index - 1) % [stepCycle count];
      return [[stepCycle objectAtIndex: newindex] floatValue];
    } 
  else 
    {
      stepCycle = [_unit stepDownCycle];
      newindex = (-index) % [stepCycle count];
      return 1 / [[stepCycle objectAtIndex: newindex] floatValue];
    }
}

- (void) _verifyCachedValues
{
  if (! _cacheIsValid)
    {
      NSSize unitSize;
      CGFloat cf; 
      int convIndex;

      /* calculate the size one unit in document view has in the ruler */
      cf = [_unit conversionFactor];
      unitSize = [self convertSize: NSMakeSize(cf, cf)  
		       fromView: [_scrollView documentView]];

      if (_orientation == NSHorizontalRuler) 
        {
          _unitToRuler = unitSize.width;
        }
      else 
        {
          _unitToRuler = unitSize.height;
        }

      /* Calculate distance between marks.  */
      /* It must not be less than MIN_MARK_DISTANCE in ruler units
       * and must obey the current unit's step cycles.  */
      _markDistance = _unitToRuler;
      convIndex = 0;
      /* make sure it's smaller than MIN_MARK_DISTANCE */
      while ((_markDistance) > MIN_MARK_DISTANCE) 
        {
          _markDistance /= [self _stepForIndex: convIndex];
          convIndex--;
        }
      /* find the first size that's not < MIN_MARK_DISTANCE */
      while ((_markDistance) < MIN_MARK_DISTANCE) 
        {
          convIndex++;
          _markDistance *= [self _stepForIndex: convIndex];
        }
    
      /* calculate number of small marks in each bigger mark */
      _marksToMidMark = GSRoundTowardsInfinity([self _stepForIndex: convIndex + 1]);
      _marksToBigMark = _marksToMidMark 
                      * GSRoundTowardsInfinity([self _stepForIndex: convIndex + 2]);
    
      /* Calculate distance between labels.
         It must not be less than MIN_LABEL_DISTANCE. */
      _labelDistance = _markDistance;
      while (_labelDistance < MIN_LABEL_DISTANCE) 
        {
          convIndex++;
          _labelDistance *= [self _stepForIndex: convIndex];
        }

      /* number of small marks between two labels */
      _marksToLabel = GSRoundTowardsInfinity(_labelDistance / _markDistance);

      /* format of labels */
      if (_labelDistance / _unitToRuler >= 1)
        {
          ASSIGN(_labelFormat, @"%1.f");
        }
      else 
        {
          /* smallest integral value not less than log10(1/labelDistInUnits) */
          int log = ceil(log10(1 / (_labelDistance / _unitToRuler)));
          NSString *string = [NSString stringWithFormat: @"%%.%df", (int)log];
          ASSIGN(_labelFormat, string);
        }

      _cacheIsValid = YES;
    }
}

- (void) drawHashMarksAndLabelsInRect: (NSRect)aRect
{
  NSView *docView;
  NSRect docBounds;
  NSRect baselineRect;
  NSRect visibleBaselineRect;
  CGFloat firstBaselineLocation;
  CGFloat firstVisibleLocation;
  CGFloat lastVisibleLocation;
  int firstVisibleMark;
  int lastVisibleMark;
  int mark;
  int firstVisibleLabel;
  int lastVisibleLabel;
  int label;
  CGFloat baselineLocation = [self baselineLocation];
  NSPoint zeroPoint;
  CGFloat zeroLocation;
  NSBezierPath *path;
  NSFont *font = [NSFont systemFontOfSize: [NSFont smallSystemFontSize]];
  NSDictionary *attr = [[NSDictionary alloc] 
			   initWithObjectsAndKeys: 
			       font, NSFontAttributeName,
			       [NSColor blackColor], NSForegroundColorAttributeName,
			       nil];

  docView = [_scrollView documentView];
  docBounds = [docView bounds];

  /* Calculate the location of 'zero' hash mark */
  // _originOffset is an offset from document bounds origin, in doc coords
  zeroPoint.x = docBounds.origin.x + _originOffset;
  zeroPoint.y = docBounds.origin.y + _originOffset;
  zeroPoint = [self convertPoint: zeroPoint fromView: docView];
  if (_orientation == NSHorizontalRuler)
    {
      zeroLocation = zeroPoint.x;
    }
  else
    {
      zeroLocation = zeroPoint.y;
    }

  [self _verifyCachedValues];

  /* Calculate the base line (corresponds to the document bounds) */
  baselineRect = [self convertRect: docBounds  fromView: docView];
  if (_orientation == NSHorizontalRuler) 
    {
      baselineRect.origin.y = baselineLocation; 
      baselineRect.size.height = 1;
      firstBaselineLocation = NSMinX(baselineRect);
      visibleBaselineRect = NSIntersectionRect(baselineRect, aRect);
      firstVisibleLocation = NSMinX(visibleBaselineRect);
      lastVisibleLocation = NSMaxX(visibleBaselineRect);
    }
  else 
    {
      baselineRect.origin.x = baselineLocation;
      baselineRect.size.width = 1;
      firstBaselineLocation = NSMinY(baselineRect);
      visibleBaselineRect = NSIntersectionRect(baselineRect, aRect);
      firstVisibleLocation = NSMinY(visibleBaselineRect);
      lastVisibleLocation = NSMaxY(visibleBaselineRect);
    }

  /* draw the base line */
  [[NSColor blackColor] set];
  NSRectFill(visibleBaselineRect);

  /* draw hash marks */
  firstVisibleMark = ceil((firstVisibleLocation - zeroLocation)
                          / _markDistance);
  lastVisibleMark = floor((lastVisibleLocation - zeroLocation) 
                          / _markDistance);
  path = [NSBezierPath new];
  
  for (mark = firstVisibleMark; mark <= lastVisibleMark; mark++)
    {
      CGFloat markLocation;

      markLocation = zeroLocation + mark * _markDistance;
      if (_orientation == NSHorizontalRuler)
        {
	  [path moveToPoint: NSMakePoint(markLocation, baselineLocation)];
        }
      else
        {
	  [path moveToPoint: NSMakePoint(baselineLocation, markLocation)];
        }

      if ((mark % _marksToLabel) == 0) 
        {
          DRAW_HASH_MARK(path, LABEL_MARK_SIZE);
        }
      else if ((mark % _marksToBigMark) == 0) 
        {
          DRAW_HASH_MARK(path, BIG_MARK_SIZE);
        }
      else if ((mark % _marksToMidMark) == 0)
        {
          DRAW_HASH_MARK(path, MID_MARK_SIZE);
        }
      else 
        {
          DRAW_HASH_MARK(path, MARK_SIZE);
        }
    }
  [path stroke];
  RELEASE(path);

  /* draw labels */
  /* FIXME: shouldn't be using NSCell to draw labels? */
  firstVisibleLabel = floor((firstVisibleLocation - zeroLocation)
                          / (_marksToLabel * _markDistance));
  lastVisibleLabel = floor((lastVisibleLocation - zeroLocation) 
                          / (_marksToLabel * _markDistance));
  /* firstVisibleLabel can be to the left of the visible ruler area.
     This is OK because just part of the label can be visible to the left
     when scrolling. However, it should not be drawn if outside of the
     baseline. */
  if (zeroLocation + firstVisibleLabel * _marksToLabel * _markDistance
      < firstBaselineLocation)
    {
      firstVisibleLabel++;
    }
  
  for (label = firstVisibleLabel; label <= lastVisibleLabel; label++)
    {
      CGFloat labelLocation = zeroLocation + label * _marksToLabel * _markDistance;
      // This has to be a float or we need to change the label format
      float labelValue = (labelLocation - zeroLocation) / _unitToRuler;
      NSString *labelString = [NSString stringWithFormat: _labelFormat, labelValue];
      NSSize size = [labelString sizeWithAttributes: attr];
      NSPoint labelPosition;

      if (_orientation == NSHorizontalRuler)
        {
	  labelPosition.x = labelLocation + 1;
	  labelPosition.y = baselineLocation + LABEL_MARK_SIZE + 4 - size.height;
        }
      else
        {
	  labelPosition.x = baselineLocation + _ruleThickness - size.width;
	  labelPosition.y = labelLocation + 1;
        }
      [labelString drawAtPoint: labelPosition withAttributes: attr];
    }

  RELEASE(attr);
}

- (void) drawMarkersInRect: (NSRect)aRect
{
  NSRulerMarker *marker;
  NSEnumerator *en;

  en = [_markers objectEnumerator];
  while ((marker = [en nextObject]) != nil)
    {
      [marker drawRect: aRect];
    }
}

- (void) invalidateHashMarks
{
  _cacheIsValid = NO;
  [self setNeedsDisplay:YES];
}

- (void) setScrollView: (NSScrollView *)scrollView
{
  /* We do NOT retain the scrollView; the scrollView is retaining us.  */
  _scrollView = scrollView;
}

- (NSScrollView *) scrollView
{
  return _scrollView;
}

- (void) setOrientation: (NSRulerOrientation)o
{
  _orientation = o;
}

- (NSRulerOrientation)orientation
{
  return _orientation;
}

- (void) setReservedThicknessForAccessoryView: (CGFloat)thickness
{
  _reservedThicknessForAccessoryView = thickness;
  [_scrollView tile];
}

- (CGFloat) reservedThicknessForAccessoryView
{
  return _reservedThicknessForAccessoryView;
}

- (void) setReservedThicknessForMarkers: (CGFloat)thickness
{
  _reservedThicknessForMarkers = thickness;
  [_scrollView tile];
}

- (CGFloat) reservedThicknessForMarkers
{
  return _reservedThicknessForMarkers;
}

- (void) setRuleThickness: (CGFloat)thickness
{
  _ruleThickness = thickness;
  [_scrollView tile];
}

- (CGFloat) ruleThickness
{
  return _ruleThickness;
}

- (CGFloat) requiredThickness
{
  return [self ruleThickness]
    + [self reservedThicknessForAccessoryView]
    + [self reservedThicknessForMarkers];
}

- (CGFloat) baselineLocation
{
  return [self reservedThicknessForAccessoryView]
    + [self reservedThicknessForMarkers];
}

- (BOOL) isFlipped
{
  if (_orientation == NSVerticalRuler)
    {
      return [[_scrollView documentView] isFlipped]; 
    }
  return YES;
}

- (void) encodeWithCoder: (NSCoder *)encoder
{
  [super encodeWithCoder: encoder];
  /* FIXME/TODO: not implemented */
  return;
}

- (id) initWithCoder: (NSCoder *)decoder
{
  self = [super initWithCoder: decoder];
  if (self == nil)
    return nil;

  /* FIXME/TODO: not implemented */
  return self;
}

- (void) dealloc
{
  RELEASE(_unit);
  RELEASE(_accessoryView);
  RELEASE(_markers);
  RELEASE(_labelFormat);
  [super dealloc];
}

@end

