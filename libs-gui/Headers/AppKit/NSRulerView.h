/* -*-objc-*-
   NSRulerView.h

   The NSRulerView class.

   Copyright (C) 1999-2002 Free Software Foundation, Inc.

   Author: Michael Hanni <mhanni@sprintmail.com>
   Date: Feb 1999
   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: Sept 2001
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

#ifndef _GNUstep_H_NSRulerView
#define _GNUstep_H_NSRulerView

#import <AppKit/NSView.h>

/* Declaring classes, rather than #including the full class header,
 * results in much faster compilations.  */
@class NSScrollView;
@class NSString;
@class NSArray;
@class NSRulerMarker;
@class NSCoder;
@class NSEvent;

typedef enum {
  NSHorizontalRuler,
  NSVerticalRuler
} NSRulerOrientation;

@class GSRulerUnit;

@interface NSRulerView : NSView
{
  GSRulerUnit *_unit;
  NSScrollView *_scrollView;
  NSView *_clientView;		// Not retained
  NSView *_accessoryView;
  CGFloat _originOffset;
  NSMutableArray *_markers;
  NSRulerOrientation _orientation;
  CGFloat _ruleThickness;
  CGFloat _reservedThicknessForAccessoryView;
  CGFloat _reservedThicknessForMarkers;

  /* Cached values. It's a little expensive to calculate them and they
   * change only when the unit or the originOffset is changed or when
   * clientView changes it's size or zooming factor.  This cache is
   * invalidated by -invalidateHashMarks method.  */
  BOOL  _cacheIsValid;
  float _markDistance;
  float _labelDistance;
  int   _marksToBigMark;
  int   _marksToMidMark;
  int   _marksToLabel;
  float _UNUSED;
  float _unitToRuler;
  NSString *_labelFormat;
}

- (id) initWithScrollView:(NSScrollView *)aScrollView
	     orientation:(NSRulerOrientation)o; 

+ (void) registerUnitWithName:(NSString *)uName
		 abbreviation:(NSString *)abbreviation
 unitToPointsConversionFactor:(CGFloat)conversionFactor
		  stepUpCycle:(NSArray *)stepUpCycle
		stepDownCycle:(NSArray *)stepDownCycle;

- (void) setMeasurementUnits: (NSString *)uName; 
- (NSString *) measurementUnits; 

- (void) setClientView: (NSView *)aView; 
- (NSView *) clientView; 

- (void) setAccessoryView: (NSView *)aView; 
- (NSView *) accessoryView; 

- (void) setOriginOffset: (CGFloat)offset; 
- (CGFloat) originOffset; 

- (void) setMarkers: (NSArray *)newMarkers; 
- (NSArray *) markers;
- (void) addMarker: (NSRulerMarker *)aMarker; 
- (void) removeMarker: (NSRulerMarker *)aMarker; 
- (BOOL) trackMarker: (NSRulerMarker *)aMarker 
      withMouseEvent: (NSEvent *)theEvent; 

- (void) moveRulerlineFromLocation: (CGFloat)oldLoc toLocation: (CGFloat)newLoc; 

- (void) drawHashMarksAndLabelsInRect: (NSRect)aRect; 
- (void) drawMarkersInRect: (NSRect)aRect; 
- (void) invalidateHashMarks; 

- (void) setScrollView:(NSScrollView *) scrollView;
- (NSScrollView *) scrollView; 

- (void) setOrientation: (NSRulerOrientation)o; 
- (NSRulerOrientation) orientation; 
- (void) setReservedThicknessForAccessoryView: (CGFloat)thickness; 
- (CGFloat) reservedThicknessForAccessoryView; 
- (void) setReservedThicknessForMarkers: (CGFloat)thickness; 
- (CGFloat) reservedThicknessForMarkers; 
- (void) setRuleThickness: (CGFloat)thickness; 
- (CGFloat) ruleThickness; 
- (CGFloat) requiredThickness;
- (CGFloat) baselineLocation; 
- (BOOL) isFlipped; 

@end

/*
 * Methods Implemented by the client view ... FIXME/TODO: we currently
 * do not send all these messages to the client view ... while we
 * should!
 */
@interface NSObject (NSRulerViewClientView)

- (void)rulerView: (NSRulerView *)aRulerView
     didAddMarker: (NSRulerMarker *)aMarker;

- (void)rulerView: (NSRulerView *)aRulerView 
    didMoveMarker: (NSRulerMarker *)aMarker; 

- (void)rulerView: (NSRulerView *)aRulerView 
  didRemoveMarker: (NSRulerMarker *)aMarker; 

- (void)rulerView: (NSRulerView *)aRulerView 
  handleMouseDown: (NSEvent *)theEvent; 

- (BOOL)rulerView: (NSRulerView *)aRulerView 
  shouldAddMarker: (NSRulerMarker *)aMarker; 

- (BOOL)rulerView: (NSRulerView *)aRulerView
 shouldMoveMarker: (NSRulerMarker *)aMarker; 

- (BOOL)rulerView: (NSRulerView *)aRulerView 
   shouldRemoveMarker: (NSRulerMarker *)aMarker;

- (CGFloat)rulerView: (NSRulerView *)aRulerView
       willAddMarker: (NSRulerMarker *)aMarker
          atLocation: (CGFloat)location; 

- (CGFloat)rulerView: (NSRulerView *)aRulerView
      willMoveMarker: (NSRulerMarker *)aMarker
          toLocation: (CGFloat)location; 

- (void)rulerView: (NSRulerView *)aRulerView
willSetClientView: (NSView *)newClient; 

@end

#endif /* _GNUstep_H_NSRulerView */

