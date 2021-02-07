/* 
   NSScroller.h

   The scroller class

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   A completely rewritten version of the original source by Scott Christley.
   Date: July 1997
   
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

#ifndef _GNUstep_H_NSScroller
#define _GNUstep_H_NSScroller
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSControl.h>
#import <AppKit/NSCell.h>

@class NSEvent;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
enum {
  NSScrollerStyleLegacy = 0,
  NSScrollerStyleOverlay = 1
};
typedef NSInteger NSScrollerStyle;

enum {
  NSScrollerKnobStyleDefault = 0,
  NSScrollerKnobStyleDark = 1,
  NSScrollerKnobStyleLight = 2
};
typedef NSInteger NSScrollerKnobStyle;
#endif

enum _NSScrollArrowPosition {
#if OS_API_VERSION(MAC_OS_X_VERSION_10_1, GS_API_LATEST)
  NSScrollerArrowsDefaultSetting = 0,
#endif
  NSScrollerArrowsMaxEnd = 0,
  NSScrollerArrowsMinEnd,
  NSScrollerArrowsNone 
};
typedef NSUInteger NSScrollArrowPosition;

enum _NSScrollerPart {
  NSScrollerNoPart = 0,
  NSScrollerDecrementPage,
  NSScrollerKnob,
  NSScrollerIncrementPage,
  NSScrollerDecrementLine,
  NSScrollerIncrementLine,
  NSScrollerKnobSlot
};
typedef NSUInteger NSScrollerPart;

enum _NSScrollerUsablePart {
  NSNoScrollerParts = 0,
  NSOnlyScrollerArrows,
  NSAllScrollerParts  
};
typedef NSUInteger NSUsableScrollerParts;

enum _NSScrollerArrow {
  NSScrollerIncrementArrow = 0,
  NSScrollerDecrementArrow
};
typedef NSUInteger NSScrollerArrow;

@interface NSScroller : NSControl <NSCoding>
{
  double _doubleValue;
  CGFloat _knobProportion;
  CGFloat _pendingKnobProportion;
  id _target;
  SEL _action;
  NSScrollerPart _hitPart;
  NSScrollArrowPosition _arrowsPosition;
  NSUsableScrollerParts _usableParts;
  struct _scFlagsType { 
    // total 7 bits.  25 bits left.
    unsigned isHorizontal: 1;
    unsigned isEnabled: 1;
    unsigned control_tint: 3;
    unsigned control_size: 2;
  } _scFlags;
}

#if OS_API_VERSION(MAC_OS_X_VERSION_10_7, GS_API_LATEST)
+ (NSScrollerStyle)preferredScrollerStyle;
#endif

//
// Laying out the NSScroller 
//
+ (CGFloat)scrollerWidth;
- (NSScrollArrowPosition)arrowsPosition;
- (void)checkSpaceForParts;
- (NSRect)rectForPart:(NSScrollerPart)partCode;
- (void)setArrowsPosition:(NSScrollArrowPosition)where;
- (NSUsableScrollerParts)usableParts;

//
// Setting the NSScroller's Values
//
- (CGFloat)knobProportion;
#if OS_API_VERSION(GS_API_MACOSX, MAC_OS_X_VERSION_10_5)
- (void)setFloatValue:(float)aFloat knobProportion:(CGFloat)ratio;
#endif

//
// Displaying 
//
- (void)drawArrow:(NSScrollerArrow)whichButton highlight:(BOOL)flag;
- (void)drawKnobSlot;
- (void)drawKnob;
- (void)drawParts;
- (void)highlight:(BOOL)flag;

//
// Handling Events 
//
- (NSScrollerPart)hitPart;

/* Return the part of the scroller which contains thePoint.  thePoint
 * is in the window's coordinate system.
 */
- (NSScrollerPart)testPart:(NSPoint)thePoint;
- (void)trackKnob:(NSEvent *)theEvent;
- (void)trackScrollButtons:(NSEvent *)theEvent;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
+ (CGFloat)scrollerWidthForControlSize:(NSControlSize)controlSize;
- (void)setControlSize:(NSControlSize)controlSize;
- (NSControlSize)controlSize;
- (void)setControlTint:(NSControlTint)controlTint;
- (NSControlTint)controlTint;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void)setKnobProportion:(CGFloat)proportion;
- (void)drawKnobSlotInRect:(NSRect)slotRect highlight:(BOOL)flag;
#endif

@end

APPKIT_EXPORT NSString *NSPreferredScrollerStyleDidChangeNotification;

#endif // _GNUstep_H_NSScroller
