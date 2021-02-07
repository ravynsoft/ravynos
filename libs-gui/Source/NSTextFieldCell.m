/** <title>NSTextFieldCell</title>

   <abstract>Cell class for the text field entry control</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
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

#import "config.h"
#import <Foundation/NSNotification.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSText.h"

@implementation NSTextFieldCell

+ (void) initialize
{
  if (self == [NSTextFieldCell class])
    {
      [self setVersion: 2];
    }
}

//
// Initialization
//
- (id) initTextCell: (NSString *)aString
{
  self = [super initTextCell: aString];
  if (self == nil)
    return self;

  ASSIGN(_text_color, [NSColor textColor]);
  ASSIGN(_background_color, [NSColor textBackgroundColor]);
//  _textfieldcell_draws_background = NO;
  _action_mask = NSKeyUpMask | NSKeyDownMask;
  return self;
}

- (void) dealloc
{
  RELEASE(_background_color);
  RELEASE(_text_color);
  RELEASE(_placeholder);
  [super dealloc];
}

- (id) copyWithZone: (NSZone*)zone
{
  NSTextFieldCell *c = [super copyWithZone: zone];

  RETAIN(_background_color);
  RETAIN(_text_color);
  c->_placeholder = [_placeholder copyWithZone: zone];

  return c;
}

//
// Modifying Graphic Attributes 
//
- (void) setBackgroundColor: (NSColor *)aColor
{
  ASSIGN (_background_color, aColor);
  if (_control_view)
    if ([_control_view isKindOfClass: [NSControl class]])
      [(NSControl *)_control_view updateCell: self];
}

/** <p>Returns the color used to draw the background</p>
    <p>See Also: -setBackgroundColor:</p>
 */
- (NSColor *) backgroundColor
{
  return _background_color;
}


/** <p>Sets whether the NSTextFieldCell draw its background color</p>
    <p>See Also: -drawsBackground</p>
 */
- (void) setDrawsBackground: (BOOL)flag
{
  _textfieldcell_draws_background = flag;
  if (_control_view)
    if ([_control_view isKindOfClass: [NSControl class]])
      [(NSControl *)_control_view updateCell: self];
}

/** <p>Returns whether the NSTextFieldCell draw its background color</p>
    <p>See Also: -setBackgroundColor:</p>
 */
- (BOOL) drawsBackground
{
  return _textfieldcell_draws_background;
}

/** <p>Sets the text color to aColor</p>
    <p>See Also: -textColor</p>
 */
- (void) setTextColor: (NSColor *)aColor
{
  ASSIGN (_text_color, aColor);
  if (_control_view)
    if ([_control_view isKindOfClass: [NSControl class]])
      [(NSControl *)_control_view updateCell: self];
}

/** <p>Returns the text color</p>
    <p>See Also: -setTextColor:</p>
 */
- (NSColor *) textColor
{
  return _text_color;
}

- (void) setBezelStyle: (NSTextFieldBezelStyle)style
{
  _bezelStyle = style;
}

- (NSTextFieldBezelStyle) bezelStyle
{
  return _bezelStyle;
}

- (NSAttributedString*) placeholderAttributedString
{
  if (_textfieldcell_placeholder_is_attributed_string == YES)
    {
      return (NSAttributedString*)_placeholder;
    }
  else
    {
      return nil;
    }
}

- (NSString*) placeholderString
{
  if (_textfieldcell_placeholder_is_attributed_string == YES)
    {
      return nil;
    }
  else
    {
      return (NSString*)_placeholder;
    }
}

- (void) setPlaceholderAttributedString: (NSAttributedString*)string
{
  ASSIGN(_placeholder, string);
  _textfieldcell_placeholder_is_attributed_string = YES;
}

- (void) setPlaceholderString: (NSString*)string
{
  ASSIGN(_placeholder, string);
  _textfieldcell_placeholder_is_attributed_string = NO;
}

- (NSText *) setUpFieldEditorAttributes: (NSText *)textObject
{
  textObject = [super setUpFieldEditorAttributes: textObject];
  [textObject setDrawsBackground: _textfieldcell_draws_background];
  [textObject setBackgroundColor: _background_color];
  [textObject setTextColor: _text_color];
  return textObject;
}

- (void) _drawBackgroundWithFrame: (NSRect)cellFrame 
                           inView: (NSView*)controlView
{
  if (_textfieldcell_draws_background)
    {
      if ([self isEnabled])
        {
          [_background_color set];
        }
      else
        {
          [[NSColor controlBackgroundColor] set];
        }
      NSRectFill([self drawingRectForBounds: cellFrame]);
    }     
}

- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView
{
  // FIXME: Should use the bezel style if set.
  [super _drawBorderAndBackgroundWithFrame: cellFrame inView: controlView];
  [self _drawBackgroundWithFrame: cellFrame inView: controlView];
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  if (_cell.in_editing)
    [self _drawEditorWithFrame: cellFrame inView: controlView];
  else
    {
      NSRect titleRect;

      /* Make sure we are a text cell; titleRect might return an incorrect
         rectangle otherwise. Note that the type could be different if the
         user has set an image on us, which we just ignore (OS X does so as
         well). */
      _cell.type = NSTextCellType;
      titleRect = [self titleRectForBounds: cellFrame];
      [[self _drawAttributedString] drawInRect: titleRect];
    }
}

/* 
   Attributed string that will be displayed.
 */
- (NSAttributedString*) _drawAttributedString
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

- (BOOL) isOpaque
{
  if (_textfieldcell_draws_background == NO 
      || _background_color == nil 
      || [_background_color alphaComponent] < 1.0)
    return NO;
  else
    return YES;   
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  BOOL tmp;

  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: [self backgroundColor] forKey: @"NSBackgroundColor"];
      [aCoder encodeObject: [self textColor] forKey: @"NSTextColor"];
      [aCoder encodeBool: [self drawsBackground] forKey: @"NSDrawsBackground"];
      if ([self isBezeled])
        {
          [aCoder encodeInt: [self bezelStyle] forKey: @"NSTextBezelStyle"];
        }
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(id) at: &_background_color];
      [aCoder encodeValueOfObjCType: @encode(id) at: &_text_color];
      tmp = _textfieldcell_draws_background;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &tmp];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (self == nil)
    return self;
 
  if ([aDecoder allowsKeyedCoding])
    {
      if ([aDecoder containsValueForKey: @"NSBackgroundColor"])
        {
          [self setBackgroundColor: [aDecoder decodeObjectForKey: 
                                                  @"NSBackgroundColor"]];
        }
      if ([aDecoder containsValueForKey: @"NSTextColor"])
        {
          [self setTextColor: [aDecoder decodeObjectForKey: @"NSTextColor"]];
        }
      if ([aDecoder containsValueForKey: @"NSDrawsBackground"])
        {
          [self setDrawsBackground: [aDecoder decodeBoolForKey: 
                                                  @"NSDrawsBackground"]];
        }
      if ([aDecoder containsValueForKey: @"NSTextBezelStyle"])
        {
          [self setBezelStyle: [aDecoder decodeIntForKey: 
                                             @"NSTextBezelStyle"]];
        }
    }
  else
    {
      BOOL tmp;

      if ([aDecoder versionForClassName:@"NSTextFieldCell"] < 2)
        {
          /* Replace the old default _action_mask with the new default one
             if it's set. There isn't really a way to modify this value
             on an NSTextFieldCell encoded in a .gorm file. The old default value
             causes problems with newer NSTableViews which uses this to discern 
             whether it should trackMouse:inRect:ofView:untilMouseUp: or not.
             This also disables the action from being sent on an uneditable and
             unselectable text fields.
          */
          if (_action_mask == NSLeftMouseUpMask)
            {
              _action_mask = NSKeyUpMask | NSKeyDownMask;
            }
        }

      [aDecoder decodeValueOfObjCType: @encode(id) at: &_background_color];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_text_color];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &tmp];
      _textfieldcell_draws_background = tmp;
    }

  return self;
}

@end
