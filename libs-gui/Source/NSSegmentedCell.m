/* NSSegmentedCell.m
 *
 * Copyright (C) 2007 Free Software Foundation, Inc.
 *
 * Author:	Gregory John Casamento <greg.casamento@gmail.com>
 * Date:	2007
 * 
 * This file is part of GNUstep.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110 
 * USA.
 */

#import <Foundation/NSArray.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSSegmentedCell.h"
#import "AppKit/NSStringDrawing.h"
#import "GNUstepGUI/GSTheme.h"

@interface NSSegmentItem : NSObject
{
  BOOL _selected;
  BOOL _enabled;
  NSInteger _tag;
  CGFloat _width;
  NSMenu *_menu;
  NSString *_label;
  NSString *_tool_tip;
  NSImage *_image;
  NSRect _frame;
}

- (BOOL) isSelected;
- (void) setSelected: (BOOL)flag;
- (BOOL) isEnabled;
- (void) setEnabled: (BOOL)flag;
- (NSMenu *) menu;
- (void) setMenu: (NSMenu *)menu;
- (NSString *) label;
- (void) setLabel: (NSString *)label;
- (NSString *) toolTip;
- (void) setToolTip: (NSString *)toolTip;
- (NSImage *) image;
- (void) setImage: (NSImage *)image;
- (NSInteger) tag;
- (void) setTag: (NSInteger)tag;
- (CGFloat) width;
- (void) setWidth: (CGFloat)width;
- (NSRect) frame;
- (void) setFrame: (NSRect)frame;
@end

@implementation NSSegmentItem

- (id) init
{
  self = [super init];
  if (nil == self)
    return nil;

  _enabled = YES;
 
  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_label);
  TEST_RELEASE(_image);
  TEST_RELEASE(_menu);
  TEST_RELEASE(_tool_tip);

  [super dealloc];
}

- (BOOL) isSelected
{
  return _selected;
}

- (void) setSelected: (BOOL)flag
{
  _selected = flag;
}

- (BOOL) isEnabled
{
  return _enabled;
}

- (void) setEnabled: (BOOL)flag
{
  _enabled = flag;
}

- (NSMenu *) menu
{
  return _menu;
}

- (void) setMenu: (NSMenu *)menu
{
  ASSIGN(_menu, menu);
}

- (NSString *) label
{
  return _label;
}

- (void) setLabel: (NSString *)label
{
  ASSIGN(_label, label);
}

- (NSString *) toolTip
{
  return _tool_tip;
}

- (void) setToolTip: (NSString *)toolTip
{
  ASSIGN(_tool_tip, toolTip);
}

- (NSImage *) image
{
  return _image;
}

- (void) setImage: (NSImage *)image
{
  ASSIGN(_image, image);
}

- (NSInteger) tag
{
  return _tag;
}

- (void) setTag: (NSInteger)tag
{
  _tag = tag;
}

- (CGFloat) width
{
  return _width;
}

- (void) setWidth: (CGFloat)width
{
  _width = width;
}

- (NSRect) frame
{
  return _frame;
}

- (void) setFrame: (NSRect)frame
{
  _frame = frame;
}

- (void) encodeWithCoder:(NSCoder *) aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      if (_label != nil)
        [aCoder encodeObject: _label forKey: @"NSSegmentItemLabel"];
      if (_image != nil)
        [aCoder encodeObject: _image forKey: @"NSSegmentItemImage"];
      if (_menu != nil)
        [aCoder encodeObject: _menu forKey: @"NSSegmentItemMenu"];
      if (_enabled)
        [aCoder encodeBool: YES forKey: @"NSSegmentItemEnabled"];
      else
        [aCoder encodeBool: YES forKey: @"NSSegmentItemDisabled"];
      if (_selected)
        [aCoder encodeBool: YES forKey: @"NSSegmentItemSelected"];
      if (_width != 0.0)
        [aCoder encodeFloat: _width forKey: @"NSSegmentItemWidth"];
      if (_tag != 0)
        [aCoder encodeInt: _tag forKey: @"NSSegmentItemTag"];
    }
  else
    {
      [aCoder encodeObject: _label];
      [aCoder encodeObject: _image];
      [aCoder encodeObject: _menu];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_enabled];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_selected];
      [aCoder encodeValueOfObjCType: @encode(CGFloat) at: &_width];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_tag];
    }
}

- (id) initWithCoder: (NSCoder *)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSSegmentItemLabel"])
        [self setLabel: [aDecoder decodeObjectForKey: @"NSSegmentItemLabel"]];
      if ([aDecoder containsValueForKey: @"NSSegmentItemImage"])
        [self setImage: [aDecoder decodeObjectForKey: @"NSSegmentItemImage"]];
      if ([aDecoder containsValueForKey: @"NSSegmentItemMenu"])
        [self setMenu: [aDecoder decodeObjectForKey: @"NSSegmentItemMenu"]];
      if ([aDecoder containsValueForKey: @"NSSegmentItemEnabled"])
          _enabled = [aDecoder decodeBoolForKey: @"NSSegmentItemEnabled"];
      else if ([aDecoder containsValueForKey: @"NSSegmentItemDisabled"])
          _enabled = ![aDecoder decodeBoolForKey: @"NSSegmentItemDisabled"];
      else
          _enabled = YES;
      if ([aDecoder containsValueForKey: @"NSSegmentItemSelected"])
        _selected = [aDecoder decodeBoolForKey: @"NSSegmentItemSelected"];
      if ([aDecoder containsValueForKey: @"NSSegmentItemWidth"])
        _width = [aDecoder decodeFloatForKey: @"NSSegmentItemWidth"];
      if ([aDecoder containsValueForKey: @"NSSegmentItemTag"])
	_tag = [aDecoder decodeIntForKey: @"NSSegmentItemTag"];
    }
  else
    {
      ASSIGN(_label,[aDecoder decodeObject]);
      ASSIGN(_image,[aDecoder decodeObject]);
      ASSIGN(_menu, [aDecoder decodeObject]);
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_enabled];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_selected];
      [aDecoder decodeValueOfObjCType: @encode(CGFloat) at: &_width];
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_tag];
    }

  return self;
}

@end

@implementation NSSegmentedCell 

- (id) init
{
  self = [super initTextCell: @""];
  if (!self)
    return nil;

  _segmentCellFlags._tracking_mode = NSSegmentSwitchTrackingSelectOne;
  _items = [[NSMutableArray alloc] initWithCapacity: 2];
  _selected_segment = -1;
  [self setAlignment: NSCenterTextAlignment];
 
  return self;
}

- (id) initImageCell: (NSImage*)anImage
{
  self = [super initImageCell: anImage];
  if (!self)
    return nil;
  
  _segmentCellFlags._tracking_mode = NSSegmentSwitchTrackingSelectOne;
  _items = [[NSMutableArray alloc] initWithCapacity: 2];
  _selected_segment = -1;
  [self setAlignment: NSCenterTextAlignment];
 
  return self;
}

- (id) initTextCell: (NSString*)aString
{
  self = [super initTextCell: aString];
  if (!self)
    return nil;
  
  _segmentCellFlags._tracking_mode = NSSegmentSwitchTrackingSelectOne;
  _items = [[NSMutableArray alloc] initWithCapacity: 2];
  _selected_segment = -1;
  [self setAlignment: NSCenterTextAlignment];
  
  return self;
}

- (id) copyWithZone: (NSZone *)zone;
{
  NSSegmentedCell *c = (NSSegmentedCell *)[super copyWithZone: zone];
  
  if (c)
    {
      // FIXME: Need a deep copy here
      c->_items = [_items copyWithZone: zone];
    }
  
  return c;
}

- (void) dealloc
{
  TEST_RELEASE(_items);
  [super dealloc];
}

// Specifying number of segments...
- (void) setSegmentCount: (NSInteger)count
{
  NSInteger size;

  if ((count < 0) || (count > 2048))
    {
      [NSException raise: NSRangeException
                   format: @"Illegal segment count."];
    }
  
  size = [_items count];
  if (count < size)
    [_items removeObjectsInRange: NSMakeRange(count, size - count)];
  
  while (count-- > size)
    {
      NSSegmentItem *item = [[NSSegmentItem alloc] init];
      [_items addObject: item];
      RELEASE(item);
    }
}

- (NSInteger) segmentCount
{
  return [_items count];
}

// Specifying selected segment...
- (void) setSelectedSegment: (NSInteger)segment
{
  [self setSelected: YES forSegment: segment];
}

- (void) setSelected: (BOOL)flag forSegment: (NSInteger)seg
{
  NSSegmentItem *segment = [_items objectAtIndex: seg];
  NSSegmentItem *previous = nil;

  if (_selected_segment != -1)
    {
      previous = [_items objectAtIndex: _selected_segment];
      if(_segmentCellFlags._tracking_mode == NSSegmentSwitchTrackingSelectOne)
	{
	  [previous setSelected: NO];
	}
    }

  if ([segment isEnabled])
    {
      [segment setSelected: flag];
      if (flag)
        {
          _selected_segment = seg;
        }
      else if (seg == _selected_segment)
        {
          _selected_segment = -1;
        }
    }
}

- (NSInteger) selectedSegment
{
  return _selected_segment;
}

- (void) selectSegmentWithTag: (NSInteger)tag
{
  NSEnumerator *en = [_items objectEnumerator];
  id o = nil;
  NSInteger segment = 0;

  while ((o = [en nextObject]) != nil)
    {
      if([o tag] == tag)
        {
          break;
        }
      segment++;
    }
  
  [self setSelected: YES forSegment: segment];
}

- (void) makeNextSegmentKey
{
  NSInteger next;

  if (_selected_segment < [_items count])
    {
      next = _selected_segment + 1;
    }
  else
    {
      next = 0;
    }
  [self setSelected: NO forSegment: _selected_segment];
  [self setSelected: YES forSegment: next];
}

- (void) makePreviousSegmentKey
{
  NSInteger prev;

  if (_selected_segment > 0)
    {
      prev = _selected_segment - 1;
    }
  else
    {
      prev = 0;
    }
  [self setSelected: NO forSegment: _selected_segment];
  [self setSelected: YES forSegment: prev];
}


// Specify tracking mode...
- (void) setTrackingMode: (NSSegmentSwitchTracking)mode
{
  _segmentCellFlags._tracking_mode = mode;
}

- (NSSegmentSwitchTracking) trackingMode
{
  return _segmentCellFlags._tracking_mode;
}


// Working with individual segments...
- (void) setWidth: (CGFloat)width forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  [segment setWidth: width];
}

- (CGFloat) widthForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment width];
}

- (void) setImage: (NSImage *)image forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  [segment setImage: image];
}

- (NSImage *) imageForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment image];
}

- (void) setLabel: (NSString *)label forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  [segment setLabel: label];
}

- (NSString *) labelForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment label];
}

- (BOOL) isSelectedForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment isSelected];
}

- (void) setEnabled: (BOOL)flag forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment setEnabled: flag];
}

- (BOOL) isEnabledForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment isEnabled];
}

- (void) setMenu: (NSMenu *)menu forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment setMenu: menu];
}

- (NSMenu *) menuForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment menu];
}

- (void) setToolTip: (NSString *) toolTip forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment setToolTip: toolTip];
}

- (NSString *) toolTipForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment toolTip];
}

- (void) setTag: (NSInteger)tag forSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment setTag: tag];
}

- (NSInteger) tagForSegment: (NSInteger)seg
{
  id segment = [_items objectAtIndex: seg];
  return [segment tag];
}

// Drawing custom content
- (void) drawSegment: (NSInteger)seg 
             inFrame: (NSRect)frame 
            withView: (NSView *)view
{
  id segment = [_items objectAtIndex: seg];
  NSString *label = [self labelForSegment: seg];
  NSImage *segmentImage = [self imageForSegment: seg];
  GSThemeControlState state = GSThemeNormalState;
  BOOL roundedLeft = NO;
  BOOL roundedRight = NO;

  [segment setFrame: frame];

  if ([segment isSelected])
    {
      state = GSThemeSelectedState;
    }

  if (seg == 0)
    {
      roundedLeft = YES;
    }

  if (seg == ([_items count] - 1))
    {
      roundedRight = YES;
    }

   [[GSTheme theme] drawSegmentedControlSegment: self
                                      withFrame: frame
                                         inView: [self controlView]
                                          style: [self segmentStyle]
                                          state: state
                                    roundedLeft: roundedLeft
                                   roundedRight: roundedRight];
   if (label)
     {
       NSSize textSize = [label sizeWithAttributes: [self _nonAutoreleasedTypingAttributes]];
       NSRect textFrame = frame;
       CGFloat x_offset = (frame.size.width - textSize.width) / 2;

       textFrame.origin.x += x_offset;
       textFrame.size.width -= x_offset;
       [self _drawText: label inFrame: textFrame];
     }

  if (segmentImage)
    {
      NSSize size = [segmentImage size];
      NSPoint position;
      NSRect destinationRect;
 
      position.x = MAX(NSMidX(frame) - (size.width/2.), 0.);
      position.y = MAX(NSMidY(frame) - (size.height/2.), 0.);
      destinationRect = NSMakeRect(position.x, position.y, size.width, size.height);

      if (nil != view)
        {
          destinationRect = [view centerScanRect: destinationRect];
        }

      [segmentImage drawInRect: destinationRect
                      fromRect: NSZeroRect
                     operation: NSCompositeSourceOver
                      fraction: 1.0];
    }
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame 
                        inView: (NSView*)controlView
{
  NSInteger i;
  NSUInteger count = [_items count];
  NSRect frame = cellFrame;
  NSRect controlFrame = [controlView frame];

  for (i = 0; i < count;i++)
    {
      frame.size.width = [[_items objectAtIndex: i] width];
      if(frame.size.width == 0.0)
	{
	  frame.size.width = (controlFrame.size.width - frame.origin.x) / (count - i); 
	}

      [self drawSegment: i inFrame: frame withView: controlView];
      frame.origin.x += frame.size.width;
      if (frame.origin.x >= cellFrame.size.width)
        break;
    }
}

// Setting the style of the segments
- (void) setSegmentStyle: (NSSegmentStyle)style
{
  _segmentCellFlags._style = style;
}

- (NSSegmentStyle) segmentStyle
{
  return _segmentCellFlags._style;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _items forKey: @"NSSegmentImages"];
      if (_selected_segment != -1)
        [aCoder encodeInt: _selected_segment forKey: @"NSSelectedSegment"];
      [aCoder encodeInt: _segmentCellFlags._style forKey: @"NSSegmentStyle"];
    }
  else
    {
      int style;
      [aCoder encodeObject: _items];
      [aCoder encodeValueOfObjCType: @encode(NSInteger) at: &_selected_segment];
      style = _segmentCellFlags._style;
      [aCoder encodeValueOfObjCType: @encode(int) at: &style];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      NSUInteger i;

      _selected_segment = -1;
      _segmentCellFlags._tracking_mode = NSSegmentSwitchTrackingSelectOne;
      if ([aDecoder containsValueForKey: @"NSSegmentImages"])
        ASSIGN(_items, [aDecoder decodeObjectForKey: @"NSSegmentImages"]);
      else
	_items = [[NSMutableArray alloc] initWithCapacity: 2];

      for (i = 0; i < [_items count]; i++)
	{
	  if ([self isSelectedForSegment: i])
	    _selected_segment = i;
	}

      if ([aDecoder containsValueForKey: @"NSSelectedSegment"])
        {
          _selected_segment = [aDecoder decodeIntForKey: @"NSSelectedSegment"];
          if (_selected_segment != -1)
            [self setSelectedSegment: _selected_segment];
        }

      _segmentCellFlags._style = [aDecoder decodeIntForKey: @"NSSegmentStyle"];
    }
  else
    {
      int style;

      _segmentCellFlags._tracking_mode = NSSegmentSwitchTrackingSelectOne;
      ASSIGN(_items,[aDecoder decodeObject]);
      [aDecoder decodeValueOfObjCType: @encode(NSInteger) at: &_selected_segment];
      if (_selected_segment != -1)
	[self setSelectedSegment: _selected_segment];
      [aDecoder decodeValueOfObjCType: @encode(int) at: &style];
      _segmentCellFlags._style = style;
    }
  return self;
}

- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView
{
  return YES;
}

- (BOOL) continueTracking: (NSPoint)lastPoint
                       at: (NSPoint)currentPoint
                   inView: (NSView*)controlView
{
  return YES;
}

- (void) stopTracking: (NSPoint)lastPoint
		   at: (NSPoint)stopPoint
	       inView: (NSView*)controlView
	    mouseIsUp: (BOOL)flag
{
  NSInteger count = [self segmentCount];
  NSInteger i = 0;

  [super stopTracking: lastPoint
	 at: stopPoint
	 inView: controlView
	 mouseIsUp: (BOOL)flag];

  for (i = 0; i < count; i++)
    {
      id segment = [_items objectAtIndex: i];
      NSRect frame = [segment frame];
      if(NSPointInRect(lastPoint,frame))
	{
	  [self setSelectedSegment: i];
	  break;
	}
    }
}
@end
