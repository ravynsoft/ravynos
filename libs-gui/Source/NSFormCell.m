/** <title>NSFormCell</title>

   <abstract>The cell class for the NSForm control</abstract>

   Copyright (C) 1996, 1999 Free Software Foundation, Inc.

   Author: Ovidiu Predescu <ovidiu@net-community.com>
   Date: March 1997
   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: November 1999
   
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

#include "config.h"
#import <Foundation/NSNotification.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSFormCell.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTextFieldCell.h"
#import "GNUstepGUI/GSTheme.h"

/** <p>TODO Description </p>
 */
@implementation NSFormCell
+ (void) initialize
{
  if (self == [NSFormCell class])
    {
      [self setVersion: 1];
    }
}

/* The title attributes are those inherited from the NSActionCell class. */
- (id) init
{
  return [self initTextCell: @"Field:"];
}

/** <p>Initializes and returns new NSFormCell with aString as its title and
    the text cell with an empty NSString.</p>
    <p>See Also: [NSCell-initTextCell:]</p>
 */
- (id) initTextCell: (NSString *)aString
{
  self = [super initTextCell: @""];
  if (nil == self)
    return nil;
  
  _cell.is_bezeled = YES;
  _cell.is_editable = YES;
  [self setAlignment: NSLeftTextAlignment];
  _titleCell = [[NSCell alloc] initTextCell: aString];
  [_titleCell setAlignment: NSRightTextAlignment];
  _formcell_auto_title_width = YES;
  _displayedTitleWidth = -1;

  return self;
}

- (void)dealloc
{
  RELEASE(_titleCell);
  TEST_RELEASE(_placeholder);
  [super dealloc];
}

/** <p>Returns whether the NSFormCell is Opaque. Returns YES if 
    the textCell and the title cell are both Opaque, NO otherwise</p>
    <p>See Also: [NSCell-isOpaque]</p>
 */
- (BOOL)isOpaque
{
  return [_titleCell isOpaque] && [super isOpaque];
}

- (void)setAttributedTitle: (NSAttributedString *)anAttributedString
{
  [_titleCell setAttributedStringValue: anAttributedString];
  if (_formcell_auto_title_width)
    {
      // Invalidates title width 
      _displayedTitleWidth = -1;
      // Update the control(s)
      [[NSNotificationCenter defaultCenter] 
        postNotificationName: _NSFormCellDidChangeTitleWidthNotification
        object: self];
    }
}

/** <p> Sets the NSFormCell title to aString. 
    TODO => _formcell_auto_title_width / Update the control(s)</p>
 */
- (void)setTitle: (NSString*)aString
{
  [_titleCell setStringValue: aString];
  
  if (_formcell_auto_title_width)
    {
      // Invalidates title width 
      _displayedTitleWidth = -1;
      // Update the control(s)
      [[NSNotificationCenter defaultCenter] 
        postNotificationName: _NSFormCellDidChangeTitleWidthNotification
        object: self];
    }
}

- (void)setTitleWithMnemonic:(NSString *)titleWithAmpersand
{
  [_titleCell setTitleWithMnemonic: titleWithAmpersand];
  if (_formcell_auto_title_width)
    {
      // Invalidates title width 
      _displayedTitleWidth = -1;
      // Update the control(s)
      [[NSNotificationCenter defaultCenter] 
        postNotificationName: _NSFormCellDidChangeTitleWidthNotification
        object: self];
    }
}

/** <p>Sets the text alignment of the NSFormCell's title to mode.
     NSRightTextAlignment by default. See <ref type="type" 
     id="NSTextAlignment">NSTextAlignment</ref> for more informations.
     </p><p>See Also: -titleAlignment [NSCell-setAlignment:]</p>
 */
- (void)setTitleAlignment: (NSTextAlignment)mode
{
  [_titleCell setAlignment: mode];
}

/** <p>Set the text font of the NSFormCell's title to fontObject.</p>
    <p>See Also: -titleFont [NSCell-setFont:]</p>
 */
- (void)setTitleFont: (NSFont*)fontObject
{
  [_titleCell setFont: fontObject];
  if (_formcell_auto_title_width)
    {
      // Invalidates title width 
      _displayedTitleWidth = -1;
      // Update the control(s)
      [[NSNotificationCenter defaultCenter] 
        postNotificationName: _NSFormCellDidChangeTitleWidthNotification
        object: self];
    }
}

/**<p>Sets the width of the NSFormCell's title to width. All NSFormCell
   of the NSForm are updated</p><p>See Also: -titleWidth</p>
*/
- (void)setTitleWidth: (CGFloat)width
{
  if (width >= 0)
    {
      _formcell_auto_title_width = NO;
      _displayedTitleWidth = width;
    }
  else 
    {
      _formcell_auto_title_width = YES;
      _displayedTitleWidth = -1;
    }
  // TODO: Don't updated the control if nothing changed.

  // Update the control(s)
  [[NSNotificationCenter defaultCenter] 
    postNotificationName: _NSFormCellDidChangeTitleWidthNotification
    object: self];
}

- (NSAttributedString *)attributedTitle
{
  return [_titleCell attributedStringValue];
}

/** <p>Returns the NSFormCell's title.</p>
    <p>See Also: -setTitle: [NSCell-stringValue]</p>
 */
- (NSString*)title
{
  return [_titleCell stringValue];
}

/** <p>Returns the text alignment of the NSFormCell's title. 
    NSRightTextAlignment by default. See NSTextAlignment for more informations
    </p><p>See Also: -setTitleAlignment:</p>
 */
- (NSTextAlignment)titleAlignment
{
  return [_titleCell alignment];
}

/** <p>Returns the text font of the NSFormCell's title</p>
    <p>See Also: -setTitleFont: [NSCell-font]</p>
 */
- (NSFont*)titleFont
{
  return [_titleCell font];
}

/** <p>Returns the writing direction of the NSFormCell's title</p>
    <p>See Also: -setTitleBaseWritingDirection: [NSCell-baseWritingDirection]</p>
 */
- (NSWritingDirection)titleBaseWritingDirection
{
  return [_titleCell baseWritingDirection];
}

/** <p>Sets the writing direction of the NSFormCell's title</p>
    <p>See Also: -titleBaseWritingDirection: [NSCell-setBaseWritingDirection]</p>
 */
- (void)setTitleBaseWritingDirection: (NSWritingDirection)writingDirection
{
  [_titleCell setBaseWritingDirection: writingDirection];
}

//
// Warning: this method returns the width of the title; the width the
// title would have if the cell was the only cell in the form.  This
// is used by NSForm to align all the cells in its form.  This is to
// say that this title width is *not* what you are going to see on the
// screen if more than one cell is present.  Setting a titleWidth
// manually with setTitleWidth: disables any alignment with other
// cells.
//
- (CGFloat)titleWidth
{
  if (_formcell_auto_title_width == NO)
    return _displayedTitleWidth;
  else
    {
      NSSize titleSize = [_titleCell cellSize];
      return titleSize.width;
    }
}

- (CGFloat)titleWidth: (NSSize)aSize
{
  if (_formcell_auto_title_width == NO)
    return _displayedTitleWidth;
  else
    {
      NSSize titleSize = [_titleCell cellSize];

      if (aSize.width > titleSize.width)
        return titleSize.width;
      else
        return aSize.width;
    }
}

- (NSAttributedString*)placeholderAttributedString
{
  if (_formcell_placeholder_is_attributed_string == YES)
    {
      return (NSAttributedString*)_placeholder;
    }
  else
    {
      return nil;
    }
}

- (NSString*)placeholderString
{
  if (_formcell_placeholder_is_attributed_string == YES)
    {
      return nil;
    }
  else
    {
      return (NSString*)_placeholder;
    }
}

- (void)setPlaceholderAttributedString: (NSAttributedString*)string
{
  ASSIGN(_placeholder, string);
  _formcell_placeholder_is_attributed_string = YES;
}

- (void)setPlaceholderString: (NSString*)string
{
  ASSIGN(_placeholder, string);
  _formcell_placeholder_is_attributed_string = NO;
}

// Updates the title width.  The width of aRect is the new title width
// to display.  Invoked by NSForm to align the editable parts of the
// cells.
- (void) calcDrawInfo: (NSRect)aRect
{
  if (_formcell_auto_title_width == NO)
    return;
  
  _displayedTitleWidth = aRect.size.width;
}


- (NSSize)cellSize
{
  NSSize returnedSize;
  NSSize titleSize = [_titleCell cellSize];
  NSSize textSize;
  
  if (_contents != nil)
    textSize = [super cellSize];
  else
    {
      ASSIGN (_contents, @"Minimum");
      _cell.contents_is_attributed_string = NO;
      textSize = [super cellSize];
      DESTROY (_contents);
    }

  returnedSize.width = titleSize.width + 3 + textSize.width;

  if (titleSize.height > textSize.height)
    returnedSize.height = titleSize.height;
  else
    returnedSize.height = textSize.height; 
  
  return returnedSize;
}

- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  // Safety check
  if (_displayedTitleWidth == -1)
    _displayedTitleWidth = [self titleWidth];

  theRect.origin.x   += _displayedTitleWidth + 3;
  theRect.size.width -= _displayedTitleWidth + 3;
  
  return [super drawingRectForBounds: theRect];
}

- (void) resetCursorRect: (NSRect)cellFrame inView: (NSView *)controlView
{
  NSRect rect = NSMakeRect(cellFrame.origin.x + 3 + [self titleWidth],
			   NSMinY(cellFrame),
			   NSWidth(cellFrame) - 3 - [self titleWidth],
			   NSHeight(cellFrame));
  
  [super resetCursorRect: rect
		  inView: controlView];
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  NSRect borderedFrame = cellFrame;

  //
  // Draw border
  //
  borderedFrame.origin.x   += _displayedTitleWidth + 3;
  borderedFrame.size.width -= _displayedTitleWidth + 3;

  [super _drawBorderAndBackgroundWithFrame: borderedFrame inView: controlView];
  // Draw text background
  [[NSColor textBackgroundColor] set];
  NSRectFill([self drawingRectForBounds: cellFrame]);
}

- (void) drawWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  NSRect titleFrame = cellFrame;

  // Safety check
  if (_displayedTitleWidth == -1)
    _displayedTitleWidth = [self titleWidth];

  // Draw title
  titleFrame.size.width = _displayedTitleWidth;
  [_titleCell drawWithFrame: titleFrame inView: controlView];

  // Draw text
  [super drawWithFrame: cellFrame inView: controlView];
}

/* 
   Attributed string that will be displayed.
 */
- (NSAttributedString*)_drawAttributedString
{
  NSAttributedString *attrStr;

  attrStr = [super _drawAttributedString];
  if (attrStr == nil)
    {
      attrStr = [self placeholderAttributedString];
      if (attrStr == nil)
        {
          NSString *string;
          NSDictionary *attributes;
          NSMutableDictionary *newAttribs;
      
          string = [self placeholderString];
          if (string == nil)
            {
              return nil;
            }

          attributes = [self _nonAutoreleasedTypingAttributes];
          newAttribs = [NSMutableDictionary 
                           dictionaryWithDictionary: attributes];
          [newAttribs setObject: [NSColor disabledControlTextColor]
                      forKey: NSForegroundColorAttributeName];
          
          return AUTORELEASE([[NSAttributedString alloc]
                                 initWithString: string
                                 attributes: newAttribs]);
        }
      else
        {
          return attrStr;
        }
    }
  else
    {
      return attrStr;
    }
}

/*
 * Copying
 */
- (id) copyWithZone: (NSZone*)zone
{
  NSFormCell *c = (NSFormCell *)[super copyWithZone:zone];
  
  /* We need to copy the title cell (as opposed to simply copying the
     pointer to it), otherwise if eg we change the string value of the
     title cell of the copied cell, the string value of the title cell
     of the original cell would be changed too ! */
  c->_titleCell = [_titleCell copyWithZone: zone];
  c->_placeholder = [_placeholder copyWithZone: zone];
  
  return c;
}


- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      /*
      if ([self stringValue] != nil)
        {
          [aCoder encodeObject: [self stringValue] forKey: @"NSContents"];
        }
      */
      [aCoder encodeFloat: [self titleWidth] forKey: @"NSTitleWidth"];
      [aCoder encodeObject: _titleCell forKey: @"NSTitleCell"];
    }
  else
    {
      BOOL tmp = _formcell_auto_title_width;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &tmp];
      [aCoder encodeValueOfObjCType: @encode(float) at: &_displayedTitleWidth];
      [aCoder encodeObject: _titleCell];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSContents"])
        {
          [self setStringValue: [aDecoder decodeObjectForKey: @"NSContents"]];
        }
      if ([aDecoder containsValueForKey: @"NSTitleWidth"])
        {
          [self setTitleWidth: [aDecoder decodeFloatForKey: @"NSTitleWidth"]];
        }
      if ([aDecoder containsValueForKey: @"NSTitleCell"])
        {
          ASSIGN(_titleCell, [aDecoder decodeObjectForKey: @"NSTitleCell"]);
        }
    }
  else
    {
      BOOL tmp;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &tmp];
      _formcell_auto_title_width = tmp;
      [aDecoder decodeValueOfObjCType: @encode(float) at: &_displayedTitleWidth];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_titleCell];
    }
  return self;
}

@end

