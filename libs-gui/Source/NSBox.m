/** <title>NSBox</title>

   <abstract>Simple box view that can display a border and title
   </abstract>

   Copyright (C) 1996-2015 Free Software Foundation, Inc.

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

#import "config.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyedArchiver.h>
#import <Foundation/NSString.h>

#import "AppKit/NSBox.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSTheme.h"

#include <math.h>

@interface NSBox (Private)
- (NSRect) calcSizesAllowingNegative: (BOOL)aFlag;
@end

/**<p> TODO : Description</p>
*/
@implementation NSBox

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSBox class])
    {
      // Initial version
      [self setVersion: 1];
    }
}

//
// Instance methods
//
- (id) initWithFrame: (NSRect)frameRect
{
  NSView *cv;

  self = [super initWithFrame: frameRect];
  if (!self)
    return self;

  _cell = [[NSCell alloc] initTextCell: @"Title"];
  [_cell setAlignment: NSCenterTextAlignment];
  [_cell setBordered: NO];
  [_cell setEditable: NO];
  [self setTitleFont: [NSFont systemFontOfSize:
                                [NSFont smallSystemFontSize]]];
  _offsets.width = 5;
  _offsets.height = 5;
  _border_rect = _bounds;
  _border_type = NSGrooveBorder;
  _title_position = NSAtTop;
  _title_rect = NSZeroRect;
  [self setAutoresizesSubviews: NO];

  cv = [NSView new];
  [self setContentView: cv];
  RELEASE(cv);

  return self;
}

- (void) dealloc
{
  TEST_RELEASE(_cell);
  [super dealloc];
}

/**<p>Returns the border rectangle of the box.</p>
 */
- (NSRect) borderRect
{
  return _border_rect;
}

/**<p>Returns the NSBox's border type. See <ref type="type" id="NSBorderType"> 
   NSBorderType</ref>  for more information. The default border type is
   <ref type="type" id="NSBorderType">NSGrooveBorder</ref>.</p>
   <p>See Also: -setBorderType:</p>
 */
- (NSBorderType) borderType
{
  return _border_type;
}

/**<p>Sets the border type to <var>aType</var>, resizes the content view frame
   if needed, and marks self for display. See <ref type="type" 
   id="NSBorderType">NSBorderType</ref> for more informations The default
   boder type is <ref type="type" id="NSBorderType">NSGrooveBorder</ref>.</p>
   <p>See Also: -borderType</p>
 */
- (void) setBorderType: (NSBorderType)aType
{
  if (_border_type != aType)
    {
      _border_type = aType;
      [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
      [self setNeedsDisplay: YES];
    }
}

- (NSBoxType) boxType
{
  return _box_type;
}

- (void) setBoxType: (NSBoxType)aType
{
  if (_box_type != aType)
    {
      _box_type = aType;
      [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
      [self setNeedsDisplay: YES];
    }
}

/**<p>Sets the title cell to <var>aString</var>, resizes the content
   view frame if needed and marks self for display.</p>
   <p>Warning: This method does not implement the Cocoa behaviour</p>
   <p>See Also: -title [NSCell-setStringValue:]</p>
 */ 
- (void) setTitle: (NSString *)aString
{
  // TODO: implement the macosx behaviour for setTitle:
  [_cell setStringValue: aString];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
  [self setNeedsDisplay: YES];
}

- (void) setTitleWithMnemonic: (NSString *)aString
{
  [_cell setTitleWithMnemonic: aString];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the title cell font to <var>fontObj</var>, resizes 
 * the content view frame if needed and marks self for display.</p>
 *<p>See Also: -titleFont [NSCell-setFont:]</p>
 */
- (void) setTitleFont: (NSFont *)fontObj
{
  [_cell setFont: fontObj];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the title cell position to <var>aPosition</var>, resizes the
   content view frame if needed and marks self for display.
   See <ref type="type" id="NSTitlePosition">NSTitlePosition</ref> for more
   information. The default position is <ref type="type" id="NSTitlePosition">
   NSAtTop</ref>.</p>
   <p>See Also: -titlePosition</p>
 */
- (void) setTitlePosition: (NSTitlePosition)aPosition
{
  if (_title_position != aPosition)
    {
      _title_position = aPosition;
      [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
      [self setNeedsDisplay: YES];
    }
}

/**<p>Returns title cell string value.</p>
 *<p>See Also: -setTitle: [NSCell-stringValue]</p>
 */
- (NSString*) title
{
  return [_cell stringValue];
}

/**<p>Returns the title cell</p>
 */
- (id) titleCell
{
  return _cell;
}

/**<p>Returns the title cell font.</p>
 *<p>See Also: -setTitleFont: [NSCell-font]</p>
 */
- (NSFont*) titleFont
{
  return [_cell font];
}

/**<p>Returns the title position. See <ref type="type" id="NSTitlePosition">
   NSTitlePosition</ref> for more information. The default position is 
   <ref type="type" id="NSTitlePosition">NSAtTop</ref></p>
   <p>See Also: -setTitlePosition:</p>
 */
- (NSTitlePosition) titlePosition
{
  return _title_position;
}

/**<p>Returns the title rectangle</p>
 */
- (NSRect) titleRect
{
  return _title_rect;
}

/**<p>Returns the NSBox's content view. The content view is created as NSView
   when the box is initialized. The contentView is resizes when needed.</p>
   <p>See Also: -setContentView:</p>
 */
- (id) contentView
{
  return _content_view;
}

/**<p>Returns an NSSize containing the interior margins of the receiver. 
  An NSBox's content view margins are empty space that is subtracted 
  from the top, bottom, and sides as padding between the inside of the box
  and the frame of its content view.</p>
  <p>See Also: -setContentViewMargins:</p>
 */
- (NSSize) contentViewMargins
{
  return _offsets;
}

/**<p>Sets the content view to aView. The current content view is replaced
   by -replaceSubview:with:. So you should -retain the current
   view if you want to use it later. The contentView frame is resized if 
   needed.</p><p>See Also: -contentView</p>
 */
- (void) setContentView: (NSView*)aView
{
  if (aView)
    {
      [super replaceSubview: _content_view with: aView];
      _content_view = aView;
      [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
    }
}

/**<p>Sets the NSSize containing the interior margins to offsetSize. 
  An NSBox's content view margins are empty space that is subtracted 
  from the top, bottom, and sides as padding between the inside of the box
  and the frame of its content view</p>
  <p> See Also: -contentViewMargins</p>
 */
- (void) setContentViewMargins: (NSSize)offsetSize
{
  NSAssert(offsetSize.width >= 0 && offsetSize.height >= 0,
        @"illegal margins supplied");

  _offsets = offsetSize;
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
  [self setNeedsDisplay: YES];

}

//
// Resizing the Box 
//
- (void) setFrame: (NSRect)frameRect
{
  [super setFrame: frameRect];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
}

- (void) setFrameSize: (NSSize)newSize
{
  [super setFrameSize: newSize];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
}

/**<p>Resizes the NSBox to fit the content view frame 
   <var>contentFrame</var>.</p>
 */
- (void) setFrameFromContentFrame: (NSRect)contentFrame
{
  // First calc the sizes to see how much we are off by
  NSRect r = [self calcSizesAllowingNegative: YES];
  NSRect f = _frame;

  NSAssert(contentFrame.size.width >= 0 && contentFrame.size.height >= 0,
        @"illegal content frame supplied");

  if (_super_view)
    r = [_super_view convertRect:r fromView: self];
  
  // Add the difference to the frame
  f.size.width = f.size.width + (contentFrame.size.width - r.size.width);
  f.size.height = f.size.height + (contentFrame.size.height - r.size.height);
  f.origin.x = f.origin.x + (contentFrame.origin.x - r.origin.x);
  f.origin.y = f.origin.y + (contentFrame.origin.y - r.origin.y);

  [self setFrame: f];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];  
}

-(NSSize) minimumSize
{
  NSRect rect;
  NSSize borderSize = [[GSTheme theme] sizeForBorderType: _border_type];

  if ([_content_view respondsToSelector: @selector(minimumSize)])
    {
      rect.size = [_content_view minimumSize];
    }
  else
    {   
      NSArray *subviewArray = [_content_view subviews];

      if ([subviewArray count])
        {
          id subview;
          NSEnumerator *enumerator;
          enumerator = [subviewArray objectEnumerator];
          
          rect = [[enumerator nextObject] frame];
          
          // Loop through subviews and calculate rect
          // to encompass all
          while ((subview = [enumerator nextObject]))
            {
              rect = NSUnionRect(rect, [subview frame]);
            }
        }
      else // _content_view has no subviews
        {
          rect = NSZeroRect;
        }
    }

  rect.size = [self convertSize: rect.size fromView:_content_view];
  rect.size.width += (2 * _offsets.width) + (2 * borderSize.width);
  rect.size.height += (2 * _offsets.height) + (2 * borderSize.height);
  return rect.size;
}

/**<p>Resizes the NSBox and its content view to fit its subviews.</p>
 */
- (void) sizeToFit
{
  NSRect f;

  if ([_content_view respondsToSelector: @selector(sizeToFit)])
    {
      [_content_view sizeToFit];
    }
  else // _content_view !respondsToSelector: sizeToFit
    {   
      NSArray *subviewArray = [_content_view subviews];
      if ([subviewArray count])
        {
          id o, e = [subviewArray objectEnumerator];
          NSRect r = [[e nextObject] frame];
          // Loop through subviews and calculate rect to encompass all
          while ((o = [e nextObject]))
            {
              r = NSUnionRect(r, [o frame]);
            }
          [_content_view setBoundsOrigin: r.origin];
          r.size = [self convertSize: r.size fromView: _content_view];
          [_content_view setAutoresizesSubviews: NO];
          [_content_view setFrameSize: r.size];
          [_content_view setAutoresizesSubviews: YES];
        }
      else // _content_view has no subviews
        {
            [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
        }
    }

  f = [_content_view frame];

  // The box width should be enough to display the title
  if (_title_position != NSNoTitle)
    {
      NSSize titleSize = [_cell cellSize];
      titleSize.width += 6;
      if (f.size.width < titleSize.width)
        f.size.width = titleSize.width;
    }
  
  if (_super_view != nil)
    [self setFrameFromContentFrame: [self convertRect: f toView: _super_view]];
  else // _super_view == nil
    [self setFrameFromContentFrame: f]; 
}

- (void) resizeWithOldSuperviewSize: (NSSize)oldSize
{
  [super resizeWithOldSuperviewSize: oldSize];
  [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
}

//
// Managing the NSView Hierarchy 
//
- (void) addSubview: (NSView*)aView
{
  [_content_view addSubview: aView];
}

- (void) addSubview: (NSView*)aView
         positioned: (NSWindowOrderingMode)place
         relativeTo: (NSView*)otherView
{
  [_content_view addSubview: aView positioned: place relativeTo: otherView];
}

- (void) replaceSubview: (NSView *)aView with: (NSView*) newView
{
  [_content_view replaceSubview: aView with: newView];
}

//
// Displaying
//
- (void) drawRect: (NSRect)rect
{
  rect = NSIntersectionRect(_bounds, rect);

  [[GSTheme theme] drawBoxInClipRect: rect
			     boxType: _box_type
			  borderType: _border_type
			      inView: self];
}

- (BOOL) isOpaque
{
  // FIXME: Depends on theme; if always returning NO is a performance hit
  // we can check if GSTheme is going to draw an old-style opaque box
  // or not.
  return NO;
  // if (_box_type == NSBoxCustom)
  //   {
  //     return !_transparent;
  //   }
  // else
  //   {
  //     return YES;
  //   }
}

- (NSColor*) fillColor
{
  return _fill_color;
}

- (void) setFillColor: (NSColor*)newFillColor
{
  ASSIGN(_fill_color, newFillColor);
}

- (NSColor*) borderColor
{
  return _border_color;
}

- (void) setBorderColor: (NSColor*)newBorderColor
{
  ASSIGN(_border_color, newBorderColor);
}

- (CGFloat) borderWidth
{
  return _border_width;
}

- (void) setBorderWidth: (CGFloat)borderWidth
{
  _border_width = borderWidth;
}

- (CGFloat) cornerRadius
{
  return _corner_radius;
}

- (void) setCornerRadius: (CGFloat)cornerRadius
{
  _corner_radius = cornerRadius;
}

- (BOOL) isTransparent
{
  return _transparent;
}

- (void) setTransparent: (BOOL)transparent
{
  _transparent = transparent;
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: [self contentView] forKey: @"NSContentView"];
      [aCoder encodeObject: _cell forKey: @"NSTitleCell"];
      [aCoder encodeInt: [self borderType] forKey: @"NSBorderType"];
      [aCoder encodeInt: [self boxType] forKey: @"NSBoxType"];
      [aCoder encodeInt: [self titlePosition] forKey: @"NSTitlePosition"];
      [aCoder encodeBool: _transparent forKey: @"NSFullyTransparent"];
      [aCoder encodeSize: [self contentViewMargins] forKey: @"NSOffsets"];
    }
  else
    {
      [aCoder encodeObject: _cell];
      [aCoder encodeSize: _offsets];
      [aCoder encodeValueOfObjCType: @encode(int) at: &_border_type];
      [aCoder encodeValueOfObjCType: @encode(int) at: &_title_position];
      // NB: the content view is our (only) subview, so it is already 
      // encoded by NSView.
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];

  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSBoxType"])
        {
          int boxType = [aDecoder decodeIntForKey: @"NSBoxType"];
          
          [self setBoxType: boxType];
        }
      if ([aDecoder containsValueForKey: @"NSBorderType"])
        {
          NSBorderType borderType = [aDecoder decodeIntForKey: @"NSBorderType"];

          [self setBorderType: borderType];
        }
      else
        {
          _border_type = NSGrooveBorder;
        }
      if ([aDecoder containsValueForKey: @"NSTitlePosition"])
        {
          NSTitlePosition titlePosition = [aDecoder decodeIntForKey: 
                                                        @"NSTitlePosition"];
          
          [self setTitlePosition: titlePosition];
        }
      else
        {
          _title_position = NSAtTop;
        }
      if ([aDecoder containsValueForKey: @"NSTransparent"])
        {
          // On Apple this is always NO, we keep it for old GNUstep archives
          _transparent = [aDecoder decodeBoolForKey: @"NSTransparent"];
        }
      if ([aDecoder containsValueForKey: @"NSFullyTransparent"])
        {
          _transparent = [aDecoder decodeBoolForKey: @"NSFullyTransparent"];
        }
     if ([aDecoder containsValueForKey: @"NSOffsets"])
        {
          [self setContentViewMargins: [aDecoder decodeSizeForKey: @"NSOffsets"]];
        }
      if ([aDecoder containsValueForKey: @"NSTitleCell"])
        {
          NSCell *titleCell = [aDecoder decodeObjectForKey: @"NSTitleCell"];
          
          ASSIGN(_cell, titleCell);
        }
      else
        {
          _cell = [[NSCell alloc] initTextCell: @"Title"];
          [_cell setAlignment: NSCenterTextAlignment];
          [_cell setBordered: NO];
          [_cell setEditable: NO];
          [self setTitleFont: [NSFont systemFontOfSize:
                                        [NSFont smallSystemFontSize]]];
        }
      if ([aDecoder containsValueForKey: @"NSContentView"])
        {
          NSView *contentView = [aDecoder decodeObjectForKey: @"NSContentView"];

          [self setContentView: contentView];
        }
      else
        {
          NSView *cv = [NSView new];
          [self setContentView: cv];
          RELEASE(cv);
        }
    }
  else
    {
        [aDecoder decodeValueOfObjCType: @encode(id) at: &_cell];
        _offsets = [aDecoder decodeSize];
        [aDecoder decodeValueOfObjCType: @encode(int)
                                     at: &_border_type];
        [aDecoder decodeValueOfObjCType: @encode(int) 
                                     at: &_title_position];

        // The content view is our only sub_view
        if ([_sub_views count] == 0)
          {
            NSDebugLLog(@"NSBox", @"NSBox: decoding without content view\n");
            // No content view
            _content_view = nil;
            [self calcSizesAllowingNegative: NO];
          }
        else 
          {
            if ([_sub_views count] != 1)
              {
                NSLog (@"Warning: Encoded NSBox with more than one content view!");
              }
            _content_view = [_sub_views objectAtIndex: 0];
            // The following also computes _title_rect and _border_rect.
            [_content_view setFrame: [self calcSizesAllowingNegative: NO]];
          }
    }
  return self;
}

@end

@implementation NSBox (Private)

- (NSRect) calcSizesAllowingNegative: (BOOL)aFlag
{
  GSTheme	*theme = [GSTheme theme];
  NSRect r = NSZeroRect;

  if (_box_type == NSBoxSeparator)
    {
      _title_rect = NSZeroRect;
      _border_rect = _bounds;
      if (_bounds.size.width > _bounds.size.height)
	{
	  _border_rect.origin.y = (int)(_border_rect.size.height / 2);
	  _border_rect.size.height = 1;
	}
      else
	{
	  _border_rect.origin.x = (int)(_border_rect.size.width / 2);
	  _border_rect.size.width = 1;
	}
      return r;
    }				

  // Don't try to compute anything while the title cell hasn't been set.
  if (_cell == nil)
    {
      return r;
    }

  switch (_title_position)
    {
      case NSNoTitle: 
	{
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  _border_rect = _bounds;
	  _title_rect = NSZeroRect;

	  // Add the offsets to the border rect
	  r.origin.x = _offsets.width + borderSize.width;
	  r.origin.y = _offsets.height + borderSize.height;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  r.size.height = _border_rect.size.height - (2 * _offsets.height)
	    - (2 * borderSize.height);

	  break;
	}
      case NSAboveTop: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  // Adjust border rect by title cell
	  _border_rect = _bounds;
	  _border_rect.size.height -= titleSize.height + borderSize.height;

	  // Add the offsets to the border rect
	  r.origin.x
	    = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.origin.y
	    = _border_rect.origin.y + _offsets.height + borderSize.height;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  r.size.height = _border_rect.size.height - (2 * _offsets.height)
	    - (2 * borderSize.height);

	  // center the title cell
	  c = floor((_bounds.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = _bounds.origin.x + c;
	  _title_rect.origin.y = _bounds.origin.y + _border_rect.size.height
	    + borderSize.height;
	  _title_rect.size = titleSize;

	  break;
	}
      case NSBelowTop: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  // Adjust border rect by title cell
	  _border_rect = _bounds;

	  // Add the offsets to the border rect
	  r.origin.x
	    = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.origin.y
	    = _border_rect.origin.y + _offsets.height + borderSize.height;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  r.size.height = _border_rect.size.height - (2 * _offsets.height)
	    - (2 * borderSize.height);

	  // Adjust by the title size
	  r.size.height -= titleSize.height + borderSize.height;

	  // center the title cell
	  c = floor((_border_rect.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = _border_rect.origin.x + c;
	  _title_rect.origin.y
	    = _border_rect.origin.y + _border_rect.size.height
	    - titleSize.height - borderSize.height;
	  _title_rect.size = titleSize;

	  break;
	}
      case NSAtTop: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;
	  float topMargin;
	  float topOffset;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  _border_rect = _bounds;

	  topMargin = ceil(titleSize.height / 2);
	  topOffset = titleSize.height - topMargin;
	  
	  // Adjust by the title size
	  _border_rect.size.height -= topMargin;
	  
	  // Add the offsets to the border rect
	  r.origin.x
	    = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  
	  if (topOffset > _offsets.height)
	    {
	      r.origin.y
		= _border_rect.origin.y + _offsets.height + borderSize.height;
	      r.size.height = _border_rect.size.height - _offsets.height
		- (2 * borderSize.height) - topOffset;
	    }
	  else
	    {
	      r.origin.y
		= _border_rect.origin.y + _offsets.height + borderSize.height;
	      r.size.height = _border_rect.size.height - (2 * _offsets.height)
		- (2 * borderSize.height);
	    }

	  // Adjust by the title size
	  //	r.size.height -= titleSize.height + borderSize.height;

	  // center the title cell
	  c = floor((_border_rect.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = _border_rect.origin.x + c;
	  _title_rect.origin.y
	    = _border_rect.origin.y + _border_rect.size.height - topMargin;
	  _title_rect.size = titleSize;

	  break;
	}
      case NSAtBottom: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;
	  float bottomMargin;
	  float bottomOffset;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  _border_rect = _bounds;

	  bottomMargin = ceil(titleSize.height / 2);
	  bottomOffset = titleSize.height - bottomMargin;

	  // Adjust by the title size
	  _border_rect.origin.y += bottomMargin;
	  _border_rect.size.height -= bottomMargin;

	  // Add the offsets to the border rect
	  r.origin.x = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);

	  if (bottomOffset > _offsets.height)
	    {
	      r.origin.y
		= _border_rect.origin.y + bottomOffset + borderSize.height;
	      r.size.height = _border_rect.size.height - _offsets.height
		- bottomOffset
		- (2 * borderSize.height);
	    }
	  else
	    {
	      r.origin.y
		= _border_rect.origin.y + _offsets.height + borderSize.height;
	      r.size.height = _border_rect.size.height - (2 * _offsets.height)
		- (2 * borderSize.height);
	    }

	  // Adjust by the title size
	  /*
	  r.origin.y += (titleSize.height / 2) + borderSize.height;
	  r.size.height -= (titleSize.height / 2) + borderSize.height;
	  */
	  // center the title cell
	  c = floor((_border_rect.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = c;
	  _title_rect.origin.y = 0;
	  _title_rect.size = titleSize;

	  break;
	}
      case NSBelowBottom: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  // Adjust by the title
	  _border_rect = _bounds;
	  _border_rect.origin.y += titleSize.height + borderSize.height;
	  _border_rect.size.height -= titleSize.height + borderSize.height;

	  // Add the offsets to the border rect
	  r.origin.x
	    = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.origin.y
	    = _border_rect.origin.y + _offsets.height + borderSize.height;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  r.size.height = _border_rect.size.height - (2 * _offsets.height)
	    - (2 * borderSize.height);

	  // center the title cell
	  c = floor((_border_rect.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = c;
	  _title_rect.origin.y = 0;
	  _title_rect.size = titleSize;

	  break;
	}
      case NSAboveBottom: 
	{
	  NSSize titleSize = [_cell cellSize];
	  NSSize borderSize = [theme sizeForBorderType: _border_type];
	  float c;

	  // Add spacer around title
	  titleSize.width += 6;
	  titleSize.height += 2;

	  _border_rect = _bounds;

	  // Add the offsets to the border rect
	  r.origin.x
	    = _border_rect.origin.x + _offsets.width + borderSize.width;
	  r.origin.y
	    = _border_rect.origin.y + _offsets.height + borderSize.height;
	  r.size.width = _border_rect.size.width - (2 * _offsets.width)
	    - (2 * borderSize.width);
	  r.size.height = _border_rect.size.height - (2 * _offsets.height)
	    - (2 * borderSize.height);

	  // Adjust by the title size
	  r.origin.y += titleSize.height + borderSize.height;
	  r.size.height -= titleSize.height + borderSize.height;

	  // center the title cell
	  c = floor((_border_rect.size.width - titleSize.width) / 2);
	  if (c < 0) c = 0;
	  _title_rect.origin.x = _border_rect.origin.x + c;
	  _title_rect.origin.y = _border_rect.origin.y + borderSize.height;
	  _title_rect.size = titleSize;

	  break;
	}
    }

  if (!aFlag)
    {
      if (r.size.width < 0)
	{
	  r.size.width = 0;
	}
      if (r.size.height < 0)
	{
	  r.size.height = 0;
	}
    }
  
  return r;
}

@end
