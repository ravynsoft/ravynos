/** <title>NSCell</title>

   <abstract>The abstract cell class</abstract>

   Copyright (C) 1996-2012,2019 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Modifications:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
   Rewrite:  Multiple authors
   Date: 1999
   Editing, formatters: Nicola Pero <nicola@brainstorm.co.uk>
   Date: 2000

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <math.h>

#import "config.h"
#import <Foundation/NSCoder.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSFormatter.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSNumberFormatter.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSString.h>
#import <Foundation/NSGeometry.h>

#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSClipView.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSMenu.h"
#import "AppKit/NSParagraphStyle.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSTextContainer.h"
#import "AppKit/NSView.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSGuiPrivate.h"

static Class colorClass;
static Class cellClass;
static Class fontClass;
static Class imageClass;

static NSColor *txtCol;
static NSColor *dtxtCol;

@interface NSCell (PrivateColor)
+ (void) _systemColorsChanged: (NSNotification*)n;
@end


@implementation NSCell (PrivateColor)
+ (void) _systemColorsChanged: (NSNotification*)n
{
  ASSIGN (txtCol, [colorClass controlTextColor]);
  ASSIGN (dtxtCol, [colorClass disabledControlTextColor]);
}
@end


/**
 *<p> TODO Desctiption</p>
 */

@implementation NSCell

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSCell class])
    {
      [self setVersion: 4];
      colorClass = [NSColor class];
      cellClass = [NSCell class];
      fontClass = [NSFont class];
      imageClass = [NSImage class];
      /*
       * Watch for changes to system colors, and simulate an initial change
       * in order to set up our defaults.
       */
      [[NSNotificationCenter defaultCenter]
        addObserver: self
           selector: @selector(_systemColorsChanged:)
               name: NSSystemColorsDidChangeNotification
             object: nil];
      [self _systemColorsChanged: nil];
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
      [self exposeBinding: NSTitleBinding];
#endif
    }
}

+ (NSMenu*) defaultMenu
{
  return nil;
}

+ (NSFocusRingType) defaultFocusRingType
{
  return NSFocusRingTypeDefault;
}

/**<p>This class method returns NO. This method should be overrided by
   subclasses.</p>
 */
+ (BOOL) prefersTrackingUntilMouseUp
{
  return NO;
}

/*
 * Instance methods
 */
- (id) init
{
  return [self initTextCell: @""];
}

/**<p>Initializes and returns a new NSCell with a NSImage <var>anImage</var>.
   This method sets the image position to <ref type="type"
   id="NSCellImagePosition">NSImageOnly</ref> and the cell's type to
   <ref type="type" id="NSCellType">NSImageCellType</ref>.</p>
   <p>See Also: -initTextCell: </p>
 */
- (id) initImageCell: (NSImage*)anImage
{
  _cell.type = NSImageCellType;
  _cell_image = RETAIN (anImage);
  _cell.image_position = NSImageOnly;
  _font = RETAIN ([fontClass systemFontOfSize: 0]);

  // Implicitly set by allocation:
  //
  //_font = nil;
  //_cell.contents_is_attributed_string = NO;
  //_cell.is_highlighted = NO;
  //_cell.is_disabled = NO;
  //_cell.is_editable = NO;
  //_cell.is_rich_text = NO;
  //_cell.imports_graphics = NO;
  //_cell.shows_first_responder = NO;
  //_cell.refuses_first_responder = NO;
  //_cell.sends_action_on_end_editing = NO;
  //_cell.is_bordered = NO;
  //_cell.is_bezeled = NO;
  //_cell.is_scrollable = NO;
  //_cell.is_selectable = NO;
  //_cell.state = 0;
  //_cell.line_break_mode = NSLineBreakByWordWrapping;
  _action_mask = NSLeftMouseUpMask;
  _menu = [object_getClass(self) defaultMenu];
  [self setFocusRingType: [object_getClass(self) defaultFocusRingType]];

  return self;
}
/**<p>Initializes and returns a new NSCell with a NSString aString.
   This method sets the cell's type to <ref type="type" id="NSCellType">
   NSTextCellType</ref>.</p>
   <p>See Also: -initImageCell: </p>
 */
- (id) initTextCell: (NSString*)aString
{
  _cell.type = NSTextCellType;
  _contents = RETAIN (aString);
  _font = RETAIN ([fontClass systemFontOfSize: 0]);

  // Implicitly set by allocation:
  //
  //_cell.contents_is_attributed_string = NO;
  //_cell_image = nil;
  //_cell.image_position = NSNoImage;
  //_cell.is_disabled = NO;
  //_cell.state = 0;
  //_cell.is_highlighted = NO;
  //_cell.is_editable = NO;
  //_cell.is_bordered = NO;
  //_cell.is_bezeled = NO;
  //_cell.is_scrollable = NO;
  //_cell.is_selectable = NO;
  //_cell.line_break_mode = NSLineBreakByWordWrapping;
  _action_mask = NSLeftMouseUpMask;
  _menu = [object_getClass(self) defaultMenu];
  [self setFocusRingType: [object_getClass(self) defaultFocusRingType]];

  return self;
}

- (void) dealloc
{
  // Remove all key value bindings for this object.
  [GSKeyValueBinding unbindAllForObject: self];
  TEST_RELEASE (_contents);
  TEST_RELEASE (_cell_image);
  TEST_RELEASE (_font);
  TEST_RELEASE (_represented_object);
  TEST_RELEASE (_object_value);
  TEST_RELEASE (_formatter);
  TEST_RELEASE (_menu);

  [super dealloc];
}

/*
 * Setting the NSCell's Value
 */
- (id) objectValue
{
  if (_cell.has_valid_object_value)
    {
      return _object_value;
    }
  else
    {
      return nil;
    }
}

- (BOOL) hasValidObjectValue
{
  return _cell.has_valid_object_value;
}

/**<p>Returns the NSCell's value as a double. </p>
 *<p>See Also: -setDoubleValue: </p>
 */
- (double) doubleValue
{
  if ((_cell.has_valid_object_value == YES) &&
      ([_object_value respondsToSelector: @selector(doubleValue)]))
    {
      return [_object_value doubleValue];
    }
  else
    {
      return [[self stringValue] doubleValue];
    }
}

/**<p>Returns the cell's value as a float. </p>
 *<p>See Also: -setFloatValue: </p>
 */
- (float) floatValue
{
  if ((_cell.has_valid_object_value == YES) &&
      ([_object_value respondsToSelector: @selector(floatValue)]))
    {
      return [_object_value floatValue];
    }
  else
    {
      return [[self stringValue] floatValue];
    }
}

/**<p>Returns the cell's value as an int. </p>
 *<p>See Also: -setIntValue:</p>
 */
- (int) intValue
{
  if ((_cell.has_valid_object_value == YES) &&
      ([_object_value respondsToSelector: @selector(intValue)]))
    {
      return [_object_value intValue];
    }
  else
    {
      return [[self stringValue] intValue];
    }
}

/**<p>Returns the cell's value as an NSInteger. </p>
 *<p>See Also: -setIntegerValue:</p>
 */
- (NSInteger) integerValue
{
  if ((_cell.has_valid_object_value == YES) &&
      ([_object_value respondsToSelector: @selector(integerValue)]))
    {
      return [_object_value integerValue];
    }
  else
    {
      return [[self stringValue] integerValue];
    }
}

/**<p>Returns the cell's value as a NSString.</p>
 *<p>See Also: -setStringValue: </p>
 */
- (NSString*) stringValue
{
  if (nil == _contents)
    {
      return @"";
    }

  if (_cell.contents_is_attributed_string == NO)
    {
      // If we have a formatter this is also the string of the _object_value
      return (NSString *)_contents;
    }
  else
    {
      return [(NSAttributedString *)_contents string];
    }
}

- (void) setObjectValue: (id)object
{
  id newContents;

  ASSIGN (_object_value, object);
  if (_formatter == nil)
    {
      if (object == nil || [object isKindOfClass: [NSString class]] == YES)
        {
          newContents = object;
          _cell.contents_is_attributed_string = NO;
          _cell.has_valid_object_value = YES;

	  // If we are in single line mode, trim the new line characters
	  if(_cell.uses_single_line_mode == YES)
	    {
	      newContents = [object stringByTrimmingCharactersInSet:
				      [NSCharacterSet newlineCharacterSet]];
	    }
        }
      else if ([object isKindOfClass: [NSAttributedString class]] == YES)
        {
          newContents = object;
          _cell.contents_is_attributed_string = YES;
          _cell.has_valid_object_value = YES;

	  // If we are in single line mode, trim the new line characters
	  if(_cell.uses_single_line_mode == YES)
	    {
	      newContents = [object stringByTrimmingCharactersInSet:
				      [NSCharacterSet newlineCharacterSet]];
	    }
	}
      else if ([_object_value respondsToSelector: @selector(attributedStringValue)])
        {
          newContents = [_object_value attributedStringValue];
          _cell.contents_is_attributed_string = YES;
          _cell.has_valid_object_value = YES;
        }
      else if ([_object_value respondsToSelector: @selector(stringValue)])
        {
          // If the thing that was assigned is not a string, but
          // responds to stringValue then get that.
          newContents = [_object_value stringValue];
          _cell.contents_is_attributed_string = NO;
          _cell.has_valid_object_value = YES;
        }
      else
        {
          newContents = [_object_value description];
          _cell.contents_is_attributed_string = NO;
          _cell.has_valid_object_value = YES;
        }
    }
  else
    {
      newContents = [_formatter stringForObjectValue: _object_value];
      _cell.contents_is_attributed_string = NO;
      if (newContents != nil)
        {
          _cell.has_valid_object_value = YES;
        }
      else
        {
          _cell.has_valid_object_value = NO;
        }
    }

  ASSIGNCOPY(_contents, newContents);
}


/**<p>Sets the NSCell's value to aDouble.</p>
 *<p>See Also: -doubleValue</p>
 */
- (void) setDoubleValue: (double)aDouble
{
  NSNumber *number;

  // NB: GNUstep can set a double value for an image cell

  number = [NSNumber numberWithDouble: aDouble];
  [self setObjectValue: number];
}

/**
 *<p>Sets the NSCell's value to a aFloat. This used for example in
 NSSliderCell</p>
 *<p>See Also: -floatValue</p>
 */
- (void) setFloatValue: (float)aFloat
{
  NSNumber *number;

  // NB: GNUstep can set a float value for an image cell.
  // NSSliderCell is an example of it!

  number = [NSNumber numberWithFloat: aFloat];
  [self setObjectValue: number];
}


/**
 *<p>Sets the NSCell's value to anInt.</p>
 *<p>See Also: -intValue</p>
 */
- (void) setIntValue: (int)anInt
{
  NSNumber *number;

  // NB: GNUstep can set an int value for an image cell.

  number = [NSNumber numberWithInt: anInt];
  [self setObjectValue: number];
}

/**
 *<p>Sets the NSCell's value to anInt.</p>
 *<p>See Also: -integerValue</p>
 */
- (void) setIntegerValue: (NSInteger)anInt
{
  NSNumber *number;

  // NB: GNUstep can set an int value for an image cell.

  number = [NSNumber numberWithInteger: anInt];
  [self setObjectValue: number];
}

/**<p>Sets the cell's value to a NSString.
   The NSCell's type is set to NSTextCellType if needed</p>
   <p>See Also: -stringValue</p>
 */
- (void) setStringValue: (NSString*)aString
{
  /* We warn about nil for compatibiliy with MacOS X, which refuses
     nil.  */
  if (aString == nil)
    {
      NSDebugMLLog (@"MacOSXCompatibility",
                    @"Attempt to use nil as string value");
    }

  if (_cell.type != NSTextCellType)
    {
      [self setType: NSTextCellType];
    }

  if (_formatter == nil)
    {
      [self setObjectValue: aString];
    }
  else
    {
      id newObjectValue;

      if ([_formatter getObjectValue: &newObjectValue
                      forString: aString
                      errorDescription: NULL])
        {
          [self setObjectValue: newObjectValue];
        }
      else
        {
          ASSIGNCOPY(_contents, aString);
          _cell.contents_is_attributed_string = NO;
          _cell.has_valid_object_value = NO;
        }
    }
}

/**<p>Returns some NSCell's attributes for the specified <ref type="type"
   id="NSCellAttribute">NSCellAttribute</ref></p>
   <p>See Also: -setCellAttribute:to:</p>
 */
- (NSInteger) cellAttribute: (NSCellAttribute)aParameter
{
  switch (aParameter)
    {
    case NSCellDisabled: return _cell.is_disabled;
    case NSCellState: return _cell.state;
    case NSCellEditable: return _cell.is_editable;
    case NSCellHighlighted: return _cell.is_highlighted;
    case NSCellIsBordered: return _cell.is_bordered;
    case NSCellAllowsMixedState: return _cell.allows_mixed_state;

      /*
        case NSPushInCell: return 0;
        case NSChangeGrayCell: return 0;
        case NSCellLightsByContents: return 0;
        case NSCellLightsByGray: return 0;
        case NSChangeBackgroundCell: return 0;
        case NSCellLightsByBackground: return 0;
        case NSCellChangesContents: return 0;
        case NSCellIsInsetButton: return 0;
      */
    case NSCellHasOverlappingImage:
      {
        return _cell.image_position == NSImageOverlaps;
      }
    case NSCellHasImageHorizontal:
      {
        return (_cell.image_position == NSImageRight)
          || (_cell.image_position == NSImageLeft);
      }
    case NSCellHasImageOnLeftOrBottom:
      {
        return (_cell.image_position == NSImageBelow)
          || (_cell.image_position == NSImageLeft);
      }
    default:
      {
        NSWarnLog (@"cell attribute %d not supported", (int)aParameter);
        break;
      }
    }

  return 0;
}


/**<p>TODO</p>
 *<p>See Also: -cellAttribute:</p>
 */
- (void) setCellAttribute: (NSCellAttribute)aParameter to: (NSInteger)value
{
  switch (aParameter)
    {
    case NSCellDisabled:
      {
        _cell.is_disabled = value;
        break;
      }
    case NSCellState:
      {
        _cell.state = value;
        break;
      }
    case NSCellEditable:
      {
        _cell.is_editable = value;
        break;
      }
    case NSCellHighlighted:
      {
        _cell.is_highlighted = value;
        break;
      }
    case NSCellHasOverlappingImage:
      {
        if (value)
          {
            _cell.image_position = NSImageOverlaps;
          }
        else
          {
            if (_cell.image_position == NSImageOverlaps)
              {
                _cell.image_position = NSImageLeft;
              }
          }
        break;
      }
    case NSCellHasImageHorizontal:
      {
        if (value)
          {
            if (_cell.image_position != NSImageLeft
                && _cell.image_position != NSImageRight)
              {
                _cell.image_position = NSImageLeft;
              }
          }
        else
          {
            if (_cell.image_position == NSImageLeft)
              {
                _cell.image_position = NSImageAbove;
              }
            else if (_cell.image_position == NSImageRight)
              {
                _cell.image_position = NSImageBelow;
              }
          }
        break;
      }
    case NSCellHasImageOnLeftOrBottom:
      {
        if (value)
          {
            if (_cell.image_position == NSImageAbove)
              {
                _cell.image_position = NSImageBelow;
              }
            else
              {
                _cell.image_position = NSImageLeft;
              }
          }
        else
          {
            if (_cell.image_position == NSImageBelow)
              {
                _cell.image_position = NSImageAbove;
              }
            else
              {
                _cell.image_position = NSImageRight;
              }
          }
        break;
      }
      /*
    case NSCellChangesContents:
      _cell. = value;
      break;
    case NSCellIsInsetButton:
      _cell. = value;
      break;
*/
    case NSCellIsBordered:
      {
        _cell.is_bordered = value;
        break;
      }
    case NSCellAllowsMixedState:
      {
        _cell.allows_mixed_state = value;
        break;
      }
    default:
      {
        NSWarnLog (@"cell attribute %d not supported", (int)aParameter);
        break;
      }
    }
}

/**<p>Sets the NSCell's type. See <ref type="type" id="NSCellType">NSCellType
   </ref>.If the cell is set to NSTextCellType, the cell is given
   a default title and is reset to the default system font.</p>
   <p>See Also: -type</p>
*/
- (void) setType: (NSCellType)aType
{
  if (_cell.type == aType)
    {
      return;
    }

  _cell.type = aType;
  switch (_cell.type)
    {
      case NSTextCellType:
        {
          ASSIGN (_contents, @"title");
          _cell.contents_is_attributed_string = NO;
          /* Doc says we have to reset the font too. */
          ASSIGN (_font, [fontClass systemFontOfSize: 0]);
          break;
        }
      case NSImageCellType:
        {
          TEST_RELEASE (_cell_image);
          _cell_image = nil;
          break;
        }
    }
}

/**<p>Returns the cell's type. Returns NSNullCellType if the
  cell's type flag is set to NSImageCellType and if the cell's image
  is nil. See <ref type="type" id="NSCellType">NSCellType</ref> for more
  information.</p><p>See Also -setType:</p>
 */
- (NSCellType) type
{
  if (_cell.type == NSImageCellType && _cell_image == nil)
    return NSNullCellType;

  return _cell.type;
}


/**<p>Returns whether the NSCell can respond to mouse events.</p>
 *<p>See Also: -setEnabled:</p>
 */
- (BOOL) isEnabled
{
  return !_cell.is_disabled;
}

/**<p>Sets whether the NSCell can respond to mouse events</p>
 <p>See Also: -isEnabled</p>
 */
- (void) setEnabled: (BOOL)flag
{
  _cell.is_disabled = !flag;
}

/**<p>Returns whether the NSCell has a bezeled border. By default a NSCell
   has no bezeled border</p><p>See Also: -setBezeled:</p>
 */
- (BOOL) isBezeled
{
  return _cell.is_bezeled;
}

/**<p>Returns whether the NSCell has a border. By default a NSCell has
   border</p><p>See Also: -setBordered: -setBezeled: -isBezeled</p>
 */
- (BOOL) isBordered
{
  return _cell.is_bordered;
}

/**<p>Returns whether the cell is opaque. Return NO by default</p>
 */
- (BOOL) isOpaque
{
  return NO;
}

/**<p>Sets whether the cell has a bezeled border.
 If this method is called, the bordered flag is turn off.
 By default a NSCell has no bezeled border</p>
 <p>See Also: -isBezeled -setBordered: -isBordered</p>
 */
- (void) setBezeled: (BOOL)flag
{
  _cell.is_bezeled = flag;
  _cell.is_bordered = NO;
}

/**<p>Sets whether the cell has a border.  If this method is called,
 the bezeled flag is turn off. By default a NSCell has no border</p>
 <p>See Also: -isBordered -setBezeled: -isBezeled</p>
 */
- (void) setBordered: (BOOL)flag
{
  _cell.is_bordered = flag;
  _cell.is_bezeled = NO;
}

- (NSFocusRingType) focusRingType
{
  return _cell.focus_ring_type;
}

- (void) setFocusRingType: (NSFocusRingType)type
{
  _cell.focus_ring_type = type;
}

/**<p>Sets the NSCell's state.  Please use always symbolic constants when
   calling this method. The integer values could be changed in the this
   implementation. (Currently they match the Cocoa values but they are
   quite strange)</p> <p>See Also: -state</p>
 */
- (void) setState: (NSInteger)value
{
  /* We do exactly as in macosx when value is not NSOnState,
   * NSOffState, NSMixedState, even if their behaviour (value < 0 ==>
   * NSMixedState) is a bit strange.  We could decide to do
   * differently in the future, so please use always symbolic
   * constants when calling this method, this way your code won't be
   * broken by changes. */
  if (value > 0 || (value < 0 && _cell.allows_mixed_state == NO))
    {
      _cell.state = NSOnState;
    }
  else if (value == 0)
    {
      _cell.state = NSOffState;
    }
  else
    {
      _cell.state = NSMixedState;
    }
}

/**<p>Returns the NSCell's state</p>
 <p>See Also: -setState: </p>
*/
- (NSInteger) state
{
  return _cell.state;
}

- (BOOL) allowsMixedState
{
  return _cell.allows_mixed_state;
}

- (void) setAllowsMixedState: (BOOL)flag
{
  _cell.allows_mixed_state = flag;
  if (!flag && _cell.state == NSMixedState)
    {
      [self setNextState];
    }
}

- (NSInteger) nextState
{
  switch (_cell.state)
    {
      case NSOnState:
        {
          return NSOffState;
        }
      case NSOffState:
        {
          if (_cell.allows_mixed_state)
            {
              return NSMixedState;
            }
          else
            {
              return NSOnState;
            }
        }
      case NSMixedState:
      default:
        {
          return NSOnState;
        }
    }
}

- (void) setNextState
{
  [self setState: [self nextState]];
}

/**<p>Returns the alignment of the text used in the NSCell. See
   <ref type="type" id="NSTextAlignment">NSTextAlignment</ref> for more
   informations. By default the text alignment is <ref type="type"
   id="NSTextAlignment">NSJustifiedTextAlignment</ref></p>
   <p>See Also: -setAlignment:</p>
 */
- (NSTextAlignment) alignment
{
  return _cell.text_align;
}

/** <p>Returns the font of the text used in the NSCell</p>
    <p>See Also: -setFont:</p>
 */
- (NSFont*) font
{
  return _font;
}

/**<p>Returns whether the cell is editable.By default a NSCell is not editable.
   </p><p>See Also: -setEditable:</p>
 */
- (BOOL) isEditable
{
  return _cell.is_editable;
}

/**<p>Returns whether the cell is selectable. This method returns YES if
   the cell is selectable or editable. NO otherwise</p>
   <p>See Also: -setSelectable: -isEditable -setEditable: </p>
 */
- (BOOL) isSelectable
{
  return _cell.is_selectable || _cell.is_editable;
}

/**<p>Returns whether the NSCell is scrollable. By default a NSCell is not
   scrollable</p><p>See Also: -setScrollable:</p>
 */
- (BOOL) isScrollable
{
  return _cell.is_scrollable;
}

/**<p>Sets the alignment of the text. See <ref type="type"
   id="NSTextAlignment">NSTextAlignment</ref>.</p><p>See Also: -alignment </p>
 */
- (void) setAlignment: (NSTextAlignment)mode
{
  // This does not have any influence on attributed strings
  _cell.text_align = mode;
}

/**<p>Sets whether the NSCell's text is editable.</p>
   <p>See Also: -isEditable -setSelectable: -isSelectable</p>
*/
- (void) setEditable: (BOOL)flag
{
  /*
   * The cell_editable flag is also checked to see if the cell is
   * selectable so turning edit on also turns selectability on (until
   * edit is turned off again).
   */
  _cell.is_editable = flag;
}

/**<p>Sets the text font. The NSCell's type is set to NSTextCellType if needed
   </p><p>See Also: -font -setType: -type</p>
 */
- (void) setFont: (NSFont*)fontObject
{
  if (_cell.type != NSTextCellType)
    {
      [self setType: NSTextCellType];
    }

  // This does not have any influence on attributed strings
  ASSIGN (_font, fontObject);
}

/**<p>Sets whether the cell selectable. Making a cell unselectable also
 * makes it uneditable until a -setEditable: re-enables it.</p>
 *<p>See Also: -isSelectable -setEditable: -isEditable</p>
 */
- (void) setSelectable: (BOOL)flag
{
  _cell.is_selectable = flag;

  if (!flag)
    _cell.is_editable = NO;
}

/**<p>Sets whether the NCell is scrollable. By default a NSCell is not
   scrollable</p><p>See Also: -isSelectable</p>
 */
- (void) setScrollable: (BOOL)flag
{
  _cell.is_scrollable = flag;
  if (flag)
    {
      [self setWraps: NO];
    }
}

- (void) setWraps: (BOOL)flag
{
  if (flag)
    {
      if (![self wraps])
	[self setLineBreakMode: NSLineBreakByWordWrapping];
    }
  else
    {
      if ([self wraps])
	[self setLineBreakMode: NSLineBreakByClipping];
    }
}

- (BOOL) wraps
{
  return _cell.line_break_mode == NSLineBreakByWordWrapping
      || _cell.line_break_mode == NSLineBreakByCharWrapping;
}

- (void) setAttributedStringValue: (NSAttributedString*)attribStr
{
  /* Hmm.  FIXME.  Not sure what to do here. */
  if (_formatter != nil)
    {
      id newObjectValue;

      if ([_formatter getObjectValue: &newObjectValue
                      forString: [attribStr string]
                      errorDescription: NULL] == YES)
        {
          [self setObjectValue: newObjectValue];
          /* What about the attributed string ?  We are loosing it. */
          return;
        }
      _cell.has_valid_object_value = NO;
    }
  else
    {
      _cell.has_valid_object_value = YES;
      ASSIGN (_object_value, attribStr);
    }

  ASSIGN (_contents, attribStr);
  _cell.contents_is_attributed_string = YES;
}

- (NSAttributedString*) attributedStringValue
{
  if (_formatter != nil)
    {
      NSDictionary *attributes;
      NSAttributedString *attrStr;

      attributes = [self _nonAutoreleasedTypingAttributes];
      attrStr = [_formatter attributedStringForObjectValue: _object_value
                            withDefaultAttributes: attributes];
      RELEASE(attributes);
      if (attrStr != nil)
        {
          return attrStr;
        }
    }

  /* In all other cases */
  if (_cell.contents_is_attributed_string &&
      nil != _contents)
    {
      return (NSAttributedString *)_contents;
    }
  else
    {
      NSDictionary *dict;
      NSAttributedString *attrStr;

      dict = [self _nonAutoreleasedTypingAttributes];
      attrStr = [[NSAttributedString alloc] initWithString: [self stringValue]
                                            attributes: dict];
      RELEASE(dict);
      return AUTORELEASE(attrStr);
    }
}

- (void) setAllowsEditingTextAttributes: (BOOL)flag
{
  _cell.is_rich_text = flag;
  if (!flag)
    _cell.imports_graphics = NO;
}

- (BOOL) allowsEditingTextAttributes
{
  return _cell.is_rich_text;
}

- (void) setImportsGraphics: (BOOL)flag
{
  _cell.imports_graphics = flag;
  if (flag)
    _cell.is_rich_text = YES;
}

- (BOOL) importsGraphics
{
  return _cell.imports_graphics;
}

- (NSString*) title
{
  return [self stringValue];
}

- (void) setTitle: (NSString*)aString
{
  [self setStringValue: aString];
}

- (NSLineBreakMode) lineBreakMode
{
  return _cell.line_break_mode;
}

- (void) setLineBreakMode: (NSLineBreakMode)mode
{
  if (mode == NSLineBreakByCharWrapping || mode == NSLineBreakByWordWrapping)
    {
      _cell.is_scrollable = NO;
    }
  _cell.line_break_mode = mode;
}

- (NSWritingDirection) baseWritingDirection
{
  return _cell.base_writing_direction;
}

- (void) setBaseWritingDirection: (NSWritingDirection)direction
{
  _cell.base_writing_direction = direction;
}

/**<p>Implemented by subclasses to return the action method.
   The NSCell implementaiton returns NULL.</p>
 <p>See Also: -setAction: -setTarget: -target</p>
 */
- (SEL) action
{
  return NULL;
}

/** <p>Implemented by subclasses to set the action method.
    The NSCell implementation raises a NSInternalInconsistencyException</p>
 <p>See Also: -action -setTarget: -target</p>
*/
- (void) setAction: (SEL)aSelector
{
  [NSException raise: NSInternalInconsistencyException
              format: @"attempt to set an action in an NSCell"];
}

/**<p>Implemented by subclasses to set the target object.
   The NSCell implementation raises a NSInternalInconsistencyException</p>
   <p>See Also: -target -setAction: -action</p>
 */
- (void) setTarget: (id)anObject
{
  [NSException raise: NSInternalInconsistencyException
              format: @"attempt to set a target in an NSCell"];
}

/**<p>Implemented by subclass to return the target object.
   The NSCell implementation returns nil</p>
   <p>See Also: -setTarget: -setAction: -action</p>
 */
- (id) target
{
  return nil;
}

/**<p>Returns whether the cell can continuously send its action messages.</p>
   <p>See Also: -setContinuous:</p>
 */
- (BOOL) isContinuous
{
  // Some subclasses should redefine this with NSLeftMouseDraggedMask
  return (_action_mask & NSPeriodicMask) != 0;
}

/**<p>Sets whether the cell can continuously send its action messages.</p>
 *<p>See Also: -isContinuous</p>
 */
- (void) setContinuous: (BOOL)flag
{
  // Some subclasses should redefine this with NSLeftMouseDraggedMask
  if (flag)
    {
      _action_mask |= NSPeriodicMask;
    }
  else
    {
      _action_mask &= ~NSPeriodicMask;
    }
}

/**<p>TODO Explain</p>
 */
- (NSInteger) sendActionOn: (NSInteger)mask
{
  NSUInteger previousMask = _action_mask;

  _action_mask = mask;

  return previousMask;
}

/**<p>Returns the NSCell's image if the NSCell's type is <ref type="type"
   id="NSCellType">NSImageCellType</ref>,
   returns nil otherwise.</p>
   <p>See Also: -setImage: -setType: -type</p>
 */
- (NSImage*) image
{
  if (_cell.type == NSImageCellType)
    {
      return _cell_image;
    }
  else
    return nil;
}

/**<p>Sets the NSCell's image to anImage. This method sets the cell's type
   to NSImageCellType if needed. Raises an NSInvalidArgumentException if
   the anImage is not an NSImage (sub)class. The new image is retained and the
   old one is released</p><p>See Also: -image</p>
 */
- (void) setImage: (NSImage*)anImage
{
  if (anImage)
    {
      NSAssert ([anImage isKindOfClass: imageClass],
                NSInvalidArgumentException);
    }

  if (_cell.type != NSImageCellType)
    {
      [self setType: NSImageCellType];
    }

  ASSIGN (_cell_image, anImage);
}

/**<p>Implemented by sublclasses to assigns the tag <var>anInt</var>.
    The NSCell implementation raises an NSInvalidArgumentException.</p>
    <p>See Also: -tag</p>
 */
- (void) setTag: (NSInteger)anInt
{
  [NSException raise: NSInternalInconsistencyException
              format: @"attempt to set a tag in an NSCell"];
}

/**<p>Implemented by subclasses to Return the tag.
   The NSCell implementation returns -1 </p><p>See Also: -setTag:</p>
 */
- (NSInteger) tag
{
  return -1;
}

/*
 * Formatting Data
 */
- (void) setFloatingPointFormat: (BOOL)autoRange
                           left: (NSUInteger)leftDigits
                          right: (NSUInteger)rightDigits
{
  NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
  NSMutableString *format = [[NSMutableString alloc] init];

  if (autoRange)
    {
      NSUInteger fieldWidth = leftDigits + rightDigits + 1;

      // FIXME: this does not fully match the documentation.
      while (fieldWidth--)
        {
          [format appendString: @"#"];
        }
    }
  else
    {
      while (leftDigits--)
        {
          [format appendString: @"#"];
        }
      [format appendString: @"."];
      while (rightDigits--)
        {
          [format appendString: @"0"];
        }
    }

  [formatter setFormat: format];
  RELEASE(format);
  [self setFormatter: formatter];
  RELEASE(formatter);
}

- (void) setFormatter: (NSFormatter*)newFormatter
{
  ASSIGN(_formatter, newFormatter);
}

- (id) formatter
{
  return _formatter;
}

/**<p> TODO</p>
 */
- (NSInteger) entryType
{
  return _cell.entry_type;
}

/** <p>TODO</p>
 */
- (void) setEntryType: (NSInteger)aType
{
  [self setType: NSTextCellType];
  // TODO: This should select a suitable formatter
  _cell.entry_type = aType;
}

- (BOOL) isEntryAcceptable: (NSString*)aString
{
  if ((_formatter != nil) && ![aString isEqualToString: @""])
    {
      id newObjectValue;

      return [_formatter getObjectValue: &newObjectValue
                         forString: aString
                         errorDescription: NULL];
    }
  else
    {
      return YES;
    }
}

/*
 * Menu
 */
- (void) setMenu: (NSMenu*)aMenu
{
  ASSIGN (_menu, aMenu);
}

- (NSMenu*) menu
{
  return _menu;
}

- (NSMenu*) menuForEvent: (NSEvent*)anEvent
                  inRect: (NSRect)cellFrame
                  ofView: (NSView*)aView
{
  return [self menu];
}

/**
 * Compares the reciever to another to another NSCell.
 * The argument must be an NSCell sublclass and have
 * the NSCellType NSTextCellType.  Returns the result
 * of the comparison of each cell's stringValue.
 */
- (NSComparisonResult) compare: (id)otherCell
{
  if ([otherCell isKindOfClass: cellClass] == NO)
    {
      [NSException raise: NSBadComparisonException
                   format: @"NSCell comparison with non-NSCell"];
    }
  if (_cell.type != NSTextCellType
      || ((NSCell*)otherCell)->_cell.type != NSTextCellType)
    {
      [NSException raise: NSBadComparisonException
                   format: @"Comparison between non-text cells"];
    }
  /* We shouldn't access instance variables directly as subclasses
     may override stringValue to retrieve the value from somewhere else.  */
  return [[self stringValue] compare: [(NSCell*)otherCell stringValue]];
}

/*
 * Should this cell respond to keyboard input?
 */
- (BOOL) acceptsFirstResponder
{
  return _cell.is_disabled == NO && _cell.refuses_first_responder == NO;
}

- (void) setShowsFirstResponder: (BOOL)flag
{
  _cell.shows_first_responder = flag;
}

- (BOOL) showsFirstResponder
{
  return _cell.shows_first_responder;
}

- (void) setTitleWithMnemonic: (NSString*)aString
{
  NSRange r = [aString rangeOfString: @"&"];

  if (r.length > 0)
    {
      NSUInteger location = r.location;

      [self setTitle: [[aString substringToIndex: location]
                        stringByAppendingString:
                          [aString substringFromIndex: NSMaxRange(r)]]];
      // TODO: We should underline this character
      [self setMnemonicLocation: location];
    }
}

- (NSString*) mnemonic
{
  NSUInteger location = [self mnemonicLocation];
  NSString *c = [self title];

  if ((location == NSNotFound) || location >= [c length])
    return @"";

  return [c substringWithRange: NSMakeRange (location, 1)];
}

- (void) setMnemonicLocation: (NSUInteger)location
{
  _cell.mnemonic_location = location;
}

- (NSUInteger) mnemonicLocation
{
  return _cell.mnemonic_location;
}

- (BOOL) refusesFirstResponder
{
  return _cell.refuses_first_responder;
}

- (void) setRefusesFirstResponder: (BOOL)flag
{
  _cell.refuses_first_responder = flag;
}

/**
 * Simulates a single click in the cell (only works with controls which have
 * no more than one cell). This method is deprecated,
 * performClickWithFrame:inView: is the right method to use now.
 */
- (void) performClick: (id)sender
{
  NSView *cv = [self controlView];

  if (cv != nil)
    [self performClickWithFrame: [cv bounds] inView: cv];
}

/*
 * Helper method used to send actions. Sender normally is [self controlView].
 */
- (BOOL) _sendActionFrom: (id)sender
{
  SEL action = [self action];

  if ([sender respondsToSelector: @selector(sendAction:to:)])
    {
      return [sender sendAction: action to: [self target]];
    }
  else
    {
      if (sender == nil)
        sender = self;

      if (action)
        {
          return [NSApp sendAction: action to: [self target] from: sender];
        }
    }

  return NO;
}

/**
 * Simulates a single click in the cell.
 * The display of the cell with this event
 * occurs in the area delimited by <var>cellFrame</var> within
 * <var>controlView</var>.
 */
- (void) performClickWithFrame: (NSRect)cellFrame inView: (NSView *)controlView
{
  if (_cell.is_disabled == YES)
    {
      return;
    }

  [self setNextState];

  if ((controlView != nil) && [controlView canDraw])
    {
      NSWindow *cvWin = [controlView window];
      NSDate *limit = [NSDate dateWithTimeIntervalSinceNow: 0.1];

      [controlView lockFocus];
      [self highlight: YES withFrame: cellFrame inView: controlView];
      [cvWin flushWindow];

      // Wait approx 1/10 seconds
      [[NSRunLoop currentRunLoop] runUntilDate: limit];

      [self highlight: NO withFrame: cellFrame inView: controlView];
      [cvWin flushWindow];
      [controlView unlockFocus];
    }

  [self _sendActionFrom: controlView];
}

/*
 * Deriving values from other objects (not necessarily cells)
 */
- (void) takeObjectValueFrom: (id)sender
{
  [self setObjectValue: [sender objectValue]];
}

/** <p>Sets the NSCell's double value to sender's double value</p>
    <p>See Also: -setDoubleValue:</p>
 */
- (void) takeDoubleValueFrom: (id)sender
{
  [self setDoubleValue: [sender doubleValue]];
}

/** <p>Sets the NSCell's float value to sender's float value</p>
    <p>See Also: -setFloatValue:</p>
 */
- (void) takeFloatValueFrom: (id)sender
{
  [self setFloatValue: [sender floatValue]];
}

/** <p>Sets the NSCell's int value to sender's int value</p>
    <p>See Also: -setIntValue:</p>
 */
- (void) takeIntValueFrom: (id)sender
{
  [self setIntValue: [sender intValue]];
}

/** <p>Sets the NSCell's NSInteger value to sender's NSInteger value</p>
    <p>See Also: -setIntegerValue:</p>
 */
- (void) takeIntegerValueFrom: (id)sender
{
  [self setIntegerValue: [sender integerValue]];
}

/** <p>Sets the NSCell's NSString value to sender's NSSting value</p>
    <p>See Also: -setStringValue:</p>
 */
- (void) takeStringValueFrom: (id)sender
{
  [self setStringValue: [sender stringValue]];
}

/** <p>Returns the NSCell's represented object</p>
    <p>See Also: -setRepresentedObject:</p>
 */
- (id) representedObject
{
  return _represented_object;
}

/** <p>Sets the NSCell's represented object to <var>anObject</var>.
    anObject will be retain.</p><p>See Also: -representedObject</p>
 */
- (void) setRepresentedObject: (id)anObject
{
  /* Ahm - not nice - the RETAIN here could cause retain cycles - anyway. */
  ASSIGN (_represented_object, anObject);
}

- (NSBackgroundStyle)backgroundStyle
{
  return(_cell.background_style);
}

- (void)setBackgroundStyle:(NSBackgroundStyle)backgroundStyle
{
  _cell.background_style = backgroundStyle;
}

/**<p>Returns the mouse flags. This flags are usally sets in
   the -trackMouse:inRect:ofView:untilMouseUp: method</p>
 */
- (NSInteger) mouseDownFlags
{
  return _mouse_down_flags;
}

/**<p>Gets the NSCell's <var>delay</var> and the <var>interval</var>
   parameters used when NSCell sends continouly action messages.
   The NSCell implementation sets <var>delay</var> to 0.2 and <var>interval</var>
   to 0.025.</p>
   <p>See Also: -trackMouse:inRect:ofView:untilMouseUp:</p>
 */
- (void) getPeriodicDelay: (float*)delay interval: (float*)interval
{
  *delay = 0.2;
  *interval = 0.025;
}

/**<p>Returns whether tracking starts. The NSCell implementation
   returns YES when the <var>startPoint</var> is into the control view
   retangle, NO otherwise. This method is call at the early stage of
   -trackMouse:inRect:ofView:untilMouseUp:</p><p>See Also:
   [NSView-mouse:inRect:] -trackMouse:inRect:ofView:untilMouseUp:
   </p>
 */
- (BOOL) startTrackingAt: (NSPoint)startPoint inView: (NSView*)controlView
{
  if ([self isContinuous] || (_action_mask & NSLeftMouseDraggedMask))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/** <p>Returns whether the mouse dragging should continue for the cell.
    Subclasses should overrided this method if you want
    stop tracking the mouse. This method is call in the
    -trackMouse:inRect:ofView:untilMouseUp: main loop.</p>
    <p>See Also: -trackMouse:inRect:ofView:untilMouseUp:</p>
 */
- (BOOL) continueTracking: (NSPoint)lastPoint
                       at: (NSPoint)currentPoint
                   inView: (NSView*)controlView
{
  if ([self isContinuous] || (_action_mask & NSLeftMouseDraggedMask))
    {
      return YES;
    }
  else
    {
      return NO;
    }
}

/**<p>Default implementation of this method in NSCell does nothing.</p>
 */
- (void) stopTracking: (NSPoint)lastPoint
                   at: (NSPoint)stopPoint
               inView: (NSView*)controlView
            mouseIsUp: (BOOL)flag
{
}

- (BOOL) trackMouse: (NSEvent*)theEvent
             inRect: (NSRect)cellFrame
             ofView: (NSView*)controlView
       untilMouseUp: (BOOL)flag
{
  NSApplication *theApp = [NSApplication sharedApplication];
  NSUInteger event_mask = NSLeftMouseDownMask | NSLeftMouseUpMask
    | NSMouseMovedMask | NSLeftMouseDraggedMask | NSOtherMouseDraggedMask
    | NSRightMouseDraggedMask;
  NSPoint location = [theEvent locationInWindow];
  NSPoint point = [controlView convertPoint: location fromView: nil];
  NSPoint last_point = point;
  BOOL mouseWentUp = NO;
  BOOL tracking;
  unsigned periodCount = 0;

  NSDebugLLog(@"NSCell", @"cell start tracking in rect %@ initial point %f %f",
             NSStringFromRect(cellFrame), point.x, point.y);

  _mouse_down_flags = [theEvent modifierFlags];

  if (![self isEnabled])
    {
      return NO;
    }

  if (![controlView mouse: point inRect: cellFrame])
    {
      // point is not in cell
      return NO;
    }

  tracking = [self startTrackingAt: point inView: controlView];

  if (_action_mask & NSEventMaskFromType([theEvent type]))
    {
      [self _sendActionFrom: controlView];
    }

  if ([self isContinuous])
    {
      float delay;
      float interval;

      [self getPeriodicDelay: &delay interval: &interval];
      [NSEvent startPeriodicEventsAfterDelay: delay withPeriod: interval];
      event_mask |= NSPeriodicMask;
    }

  NSDebugLLog(@"NSCell", @"cell get mouse events\n");
  if (theEvent != [NSApp currentEvent])
    {
      theEvent = [NSApp currentEvent];
    }
  else
    {
      theEvent = [theApp nextEventMatchingMask: event_mask
                                     untilDate: [NSDate distantFuture]
                                        inMode: NSEventTrackingRunLoopMode
                                       dequeue: YES];
    }

  while (YES)
    {
      NSEventType eventType;

      eventType = [theEvent type];

      // Did the mouse go up?
      if (eventType == NSLeftMouseUp)
        {
          NSDebugLLog(@"NSCell", @"cell mouse went up\n");
          mouseWentUp = YES;
          break;
        }
      else if (eventType == NSPeriodic)
        {
          NSDebugLLog (@"NSCell", @"cell got a periodic event");
          if (periodCount == 4)
            {
              NSWindow *w = [controlView window];

              /*
               * Too many periodic events in succession -
               * update the mouse location and reset the counter.
               */
              location = [w mouseLocationOutsideOfEventStream];
              last_point = point;
              point = [controlView convertPoint: location fromView: nil];
              periodCount = 0;
            }
          else
            {
              periodCount++;
            }
        }
      else
        {
          location = [theEvent locationInWindow];
          last_point = point;
          point = [controlView convertPoint: location fromView: nil];
        }

      if (!flag && ![controlView mouse: point inRect: cellFrame])
        {
          NSDebugLLog(@"NSCell", @"point not in cell frame\n");
          break;
        }

      if (tracking)
        {
          // should continue tracking?
          tracking = [self continueTracking: last_point
                                         at: point
                                     inView: controlView];
          NSDebugLLog(@"NSCell", @"cell continue tracking %d\n", tracking);
        }

      if (_action_mask & NSEventMaskFromType([theEvent type]))
        {
          [self _sendActionFrom: controlView];
        }

      theEvent = [theApp nextEventMatchingMask: event_mask
                                     untilDate: [NSDate distantFuture]
                                        inMode: NSEventTrackingRunLoopMode
                                       dequeue: YES];
    }

  if (tracking)
    {
      // Hook called when stop tracking
      [self stopTracking: last_point
                      at: point
                  inView: controlView
               mouseIsUp: mouseWentUp];
    }

  if ([self isContinuous])
    {
      [NSEvent stopPeriodicEvents];
    }

  if (mouseWentUp)
    {
      [self setNextState];
      if (_action_mask & NSEventMaskFromType([theEvent type]))
        {
          [self _sendActionFrom: controlView];
        }
    }

  // Return YES only if the mouse went up within the cell or flag was true
  if (mouseWentUp && (flag || [controlView mouse: point inRect: cellFrame]))
    {
      NSDebugLLog(@"NSCell", @"mouse went up in cell\n");
      return YES;
    }
  else
    {
      // Otherwise return NO
      NSDebugLLog(@"NSCell", @"mouse did not go up in cell\n");
      return NO;
    }
}

- (NSUInteger) hitTestForEvent: (NSEvent*)event
                       inRect: (NSRect)cellFrame
                       ofView: (NSView*)controlView
{
  if (_cell.type == NSImageCellType)
    {
      if ((_cell_image != nil) &&
          NSMouseInRect([controlView convertPoint: [event locationInWindow]
                                         fromView: nil],
                        [self imageRectForBounds: [controlView bounds]],
                        [controlView isFlipped]))
        {
          return NSCellHitContentArea;
        }
      else
        {
          return NSCellHitNone;
        }
    }
  else if (_cell.type == NSTextCellType)
    {
      if (_contents == nil)
        {
          return NSCellHitNone;
        }
      else if (_cell.is_disabled == NO)
        {
          return NSCellHitContentArea | NSCellHitEditableTextArea;
        }
      else
        {
          return NSCellHitContentArea;
        }
    }
  else
    {
      if (_cell.is_disabled == NO)
        {
          return NSCellHitContentArea | NSCellHitTrackableArea;
        }
      else
        {
          return NSCellHitContentArea;
        }
    }
}

/** <p>TODO</p>
 */
- (void) resetCursorRect: (NSRect)cellFrame inView: (NSView*)controlView
{
  if (_cell.type == NSTextCellType && _cell.is_disabled == NO
    && (_cell.is_selectable == YES || _cell.is_editable == YES))
    {
      static NSCursor        *cursor = nil;
      NSRect        rect;

      if (cursor== nil)
        {
          cursor = RETAIN([NSCursor IBeamCursor]);
        }
      rect = NSIntersectionRect(cellFrame, [controlView visibleRect]);
      /*
       * Here we depend on an undocumented feature of NSCursor which may or
       * may not exist in OPENSTEP or MacOS-X ...
       * If we add a cursor rect to a view and don't set it to be set on
       * either entry to or exit from the view, we push it on entry and
       * pop it from the cursor stack on exit.
       */
      [controlView addCursorRect: rect cursor: cursor];
    }
}

/**<p>Implemented by subclasses to returns the key equivalent.
   The NSCell implementation returns an empty NSString. </p>
 */
- (NSString*) keyEquivalent
{
  return @"";
}

/**<p>Does nothing. This method is used by subclasses to recalculate sizes</p>
   <p>It is usally called from a NSControl object</p>
   <p>See Also: [NSControl-calcSize]</p>
 */
- (void) calcDrawInfo: (NSRect)aRect
{
}

/**Returns the minimun size needed to display the NSCell.
   This size is calculate by adding :
   <list>
   <item> the borders (plain or bezeled) size</item>
   <item> the spacing between the border and inside the cell</item>
   <item> the TODO ... if the cell is type  of NSTextCellType
   or the image size if the cell has a NSImageCellType type.</item>
   </list>
  <p>This method  returns NSZeroSize if the cell has a NSNullCellType type
   (Cocoa returns a very big size instead).
   </p>
 */
- (NSSize) cellSize
{
  NSSize borderSize, s;
  NSBorderType aType;

  // Get border size
  if (_cell.is_bordered)
    aType = NSLineBorder;
  else if (_cell.is_bezeled)
    aType = NSBezelBorder;
  else
    aType = NSNoBorder;

  borderSize = [[GSTheme theme] sizeForBorderType: aType];

  // Add spacing between border and inside
  if (_cell.is_bordered || _cell.is_bezeled)
    {
      borderSize.height += 1;
      borderSize.width  += 3;
    }

  // Get Content Size
  switch (_cell.type)
    {
      case NSTextCellType:
        {
          NSAttributedString *attrStr;

          attrStr = [self attributedStringValue];
          if ([attrStr length] != 0)
            {
              s = [attrStr size];
            }
          else
            {
              s = [self _sizeText: @"A"];
            }
        }
        break;

      case NSImageCellType:
        if (_cell_image == nil)
          {
            s = NSZeroSize;
          }
        else
          {
            s = [_cell_image size];
          }
        break;

      default:
      case NSNullCellType:
        //  macosx instead returns a 'very big size' here; we return NSZeroSize
        s = NSZeroSize;
        break;
    }

  // Add in border size
  s.width += 2 * borderSize.width;
  s.height += 2 * borderSize.height;

  return s;
}

/**<p>TODO. Currently the GNUstep implementation returns -cellSize</p>
   <p>See Also: -cellSize</p>
 */
- (NSSize) cellSizeForBounds: (NSRect)aRect
{
  if (_cell.type == NSTextCellType)
    {
      // TODO: Resize the text to fit
    }

  return [self cellSize];
}

/**<p>TODO</p>
 */
- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  NSSize borderSize;
  NSBorderType aType;

  // Get border size
  if (_cell.is_bordered)
    aType = NSLineBorder;
  else if (_cell.is_bezeled)
    aType = NSBezelBorder;
  else
    aType = NSNoBorder;

  borderSize = [[GSTheme theme] sizeForBorderType: aType];
  return NSInsetRect(theRect, borderSize.width, borderSize.height);
}

/**<p>Frame the image gets drawn in</p>
 */
- (NSRect) imageRectForBounds: (NSRect)theRect
{
  if (_cell.type == NSImageCellType)
    {
      NSRect frame = [self drawingRectForBounds: theRect];

      // Add spacing between border and inside
      if (_cell.is_bordered || _cell.is_bezeled)
        {
          frame.origin.x += 3;
          frame.size.width -= 6;
          frame.origin.y += 1;
          frame.size.height -= 2;
        }
      return frame;
    }
  else
    {
      return theRect;
    }
}

/** <p>Frame the title gets drawn in</p>
 */
- (NSRect) titleRectForBounds: (NSRect)theRect
{
  if (_cell.type == NSTextCellType)
    {
      NSRect frame = [self drawingRectForBounds: theRect];

      // Add spacing between border and inside
      if (_cell.is_bordered || _cell.is_bezeled)
        {
          frame.origin.x += 3;
          frame.size.width -= 6;
          frame.origin.y += 1;
          frame.size.height -= 2;
        }
      return frame;
    }
  else
    {
      return theRect;
    }
}

- (void) setControlSize: (NSControlSize)controlSize
{
  _cell.control_size = controlSize;
}

- (NSControlSize) controlSize
{
  return _cell.control_size;
}

- (void) setControlTint: (NSControlTint)controlTint
{
  _cell.control_tint = controlTint;
}

- (NSControlTint) controlTint
{
  return _cell.control_tint;
}

/**<p>This method is used by subclasses to get the control view.
   This method returns nil.</p>
 */
- (NSView*) controlView
{
  return nil;
}

/**<p>This method is used by subclasses to specify the control view.</p>
 */
- (void) setControlView: (NSView*)view
{
  // Do nothing
}

/** <p>This drawing is minimal and with no background,
 * to make it easier for subclass to customize drawing. </p>
 */
- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  switch (_cell.type)
    {
      case NSTextCellType:
	if (_cell.in_editing)
	  [self _drawEditorWithFrame: cellFrame inView: controlView];
	else
	  [self _drawAttributedText: [self _drawAttributedString]
			    inFrame: [self titleRectForBounds: cellFrame]];
        break;

      case NSImageCellType:
        if (_cell_image)
          {
            NSSize size;
            NSPoint position;
            NSRect drawingRect = [self imageRectForBounds: cellFrame];
            NSRect rect;

            size = [_cell_image size];
            position.x = MAX(NSMidX(drawingRect) - (size.width/2.),0.);
            position.y = MAX(NSMidY(drawingRect) - (size.height/2.),0.);
            rect = NSMakeRect(position.x, position.y, size.width, size.height);

            if (nil != controlView)
              {
                rect = [controlView centerScanRect: rect];
              }

            [_cell_image drawInRect: rect
			   fromRect: NSZeroRect
			  operation: NSCompositeSourceOver
			   fraction: 1.0
		     respectFlipped: YES
			      hints: nil];
          }
        break;

      case NSNullCellType:
        break;
    }

  // NB: We don't do any highlighting to make it easier for subclasses
  // to reuse this code while doing their own custom highlighting and
  // prettyfying
}

/**<p>Draws the cell in <var>controlView</var></p>
 */
- (void) drawWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  // do nothing if cell's frame rect is zero
  if (NSIsEmptyRect(cellFrame))
    return;

  // draw the border if needed
  [self _drawBorderAndBackgroundWithFrame: cellFrame inView: controlView];
  // draw interior
  [self drawInteriorWithFrame: cellFrame inView: controlView];
  // Draw first responder
  [self _drawFocusRingWithFrame: cellFrame inView: controlView];
}

/**<p>Sets whether the NSCell is highlighted.</p>
   <p>See Also: -isHighlighted</p>
 */
- (void) setHighlighted: (BOOL) flag
{
  _cell.is_highlighted = flag;
}

/**<p>Returns whether the cell is highlighted. By default NO</p>
   <p>See Also: -setHighlighted:</p>
 */
- (BOOL) isHighlighted
{
  return _cell.is_highlighted;
}

/**
 *<p>TODO explain</p>
 */

- (void) highlight: (BOOL)lit
         withFrame: (NSRect)cellFrame
            inView: (NSView*)controlView
{
  if (_cell.is_highlighted != lit)
    {
      _cell.is_highlighted = lit;

      // Disabling this because when combined with making
      // -[NSButtonCell isOpaque] return NO, it was causing scroller buttons
      // to stay stuck down. --Eric (2013-09-28)
#if 0
      /*
       * NB: This has a visible effect only if subclasses override
       * drawWithFrame:inView: to draw something special when the
       * cell is highlighted.
       * NSCell simply draws border+text/image and makes no highlighting,
       * for easier subclassing.
       */
      if ([self isOpaque] == NO)
        {
          /* FIXME - This looks like potentially generating an
           * infinite loop!  The control asking the cell to draw
           * itself in the rect, the cell asking the control to draw
           * the rect, the control asking the cell to draw itself in
           * the rect, the cell ...
           *
           * I think we should remove it.  The control is responsible
           * for using the cell to draw, not vice versa.
           */
          [controlView displayRect: cellFrame];
        }
#endif
      [self drawWithFrame: cellFrame inView: controlView];
    }
}

- (NSColor*) highlightColorWithFrame: (NSRect)cellFrame
                              inView: (NSView *)controlView
{
  return [NSColor selectedControlColor];
}

- (NSText*) setUpFieldEditorAttributes: (NSText*)textObject
{
  NSDictionary *attr;

  // Reset the string to have a well defined state. The real string gets set later on.
  [textObject setString: @""];

  [textObject setTextColor: [self textColor]];
  if ([self isBezeled])
    {
      [textObject setBackgroundColor: [NSColor textBackgroundColor]];
      [textObject setDrawsBackground: YES];
    }
  else
    {
      [textObject setDrawsBackground: NO];
    }
  [textObject setFont: [self font]];
  [textObject setAlignment: [self alignment]];
  // FIXME: Add base writing direction

  [textObject setEditable: [self isEditable]];
  [textObject setSelectable: [self isSelectable]];
  [textObject setRichText: [self allowsEditingTextAttributes]];
  [textObject setImportsGraphics: [self importsGraphics]];
  [(NSTextView*)textObject setAllowsUndo: [self allowsUndo]];
  attr = [self _nonAutoreleasedTypingAttributes];
  [(NSTextView*)textObject setTypingAttributes: attr];
  RELEASE(attr);

  return textObject;
}

- (void) _setupTextWithFrame: (NSRect)aRect
                      inView: (NSView*)controlView
                      editor: (NSText*)textObject
                    delegate: (id)anObject
                       range: (NSRange)selection
{
  BOOL needsClipView;
  BOOL wraps = [self wraps];
  NSTextContainer *ct;
  NSSize maxSize;
  NSRect titleRect = [self titleRectForBounds: aRect];

  /* We always add a clip view if the cell is editable so that the user
     can edit the whole contents even if the cell's contents normally is
     clipped. */
  needsClipView = [self isScrollable] || [self isEditable];
  if (needsClipView)
    {
      NSClipView *cv;

      cv = [[NSClipView alloc] initWithFrame: titleRect];
      [cv setDocumentView: textObject];
      [controlView addSubview: cv];
      RELEASE(cv);
    }
  else
    [controlView addSubview: textObject];

  /* Note: The order of statements matters here. We must set the text object's
     horizontallyResizable and verticallyResizable attributes before setting
     its frame size. Otherwise, the text object's width and/or height might
     incorrectly be reduced to zero (since the text object has no contents at
     this point) if the text object was resizable by its text container before.
     Of course we could also have set the text object's minimum size to the
     intended frame size, but then we must update the minimum size whenever
     the field editor's frame is changed in -_drawEditorWithFrame:inView:.
     Note that the minimum size is not relevant when a clip view is used. */
  [textObject setMinSize: NSZeroSize];
  [textObject setMaxSize: NSMakeSize(1e6, 1e6)];
  [textObject setHorizontallyResizable: needsClipView && !wraps];
  [textObject setVerticallyResizable: needsClipView];
  [textObject setFrame: titleRect];
  if (needsClipView)
    [textObject setAutoresizingMask: NSViewWidthSizable + NSViewHeightSizable];
  else
    [textObject setAutoresizingMask: NSViewNotSizable];

  /* Note: Order of statements matters again. The heightTracksTextView
     and widthTracksTextView container attributes must be set after setting
     the horizontallyResizable and verticallyResizable text view attributes
     because NSTextView's setter methods include "safety code", which
     always updates the container attributes along with the text view
     attributes.
     FIXME Fix NSTextView to only reset the text container attributes, but
     never set them. However note that this may break some sloppily written
     code which forgets to set the text container attributes. */
  /* See comments in NSStringDrawing.m about the choice of maximum size. */
  ct = [(NSTextView*)textObject textContainer];
  if (wraps)
    maxSize = NSMakeSize(NSWidth(titleRect), 1e6);
  else
    maxSize = NSMakeSize(1e6, 1e6);
  [ct setContainerSize: maxSize];
  [ct setWidthTracksTextView: wraps];
  [ct setHeightTracksTextView: NO];

  [self _updateFieldEditor: textObject];

  [textObject setSelectedRange: selection];
  [textObject scrollRangeToVisible: selection];

  [textObject setDelegate: anObject];
  [[controlView window] makeFirstResponder: textObject];
  _cell.in_editing = YES;
}

/**<p>Ends any text editing. This method sets the text object's delegate
   to nil, and remove the NSClipView and the text object used for editing</p>
 <p>See Also:  -editWithFrame:inView:editor:delegate:event:</p>
 */
- (void) endEditing: (NSText*)textObject
{
  NSClipView *clipView;

  _cell.in_editing = NO;
  [textObject setString: @""];
  [textObject setDelegate: nil];

  clipView = (NSClipView*)[textObject superview];
  if ([clipView isKindOfClass: [NSClipView class]])
    {
      [clipView setDocumentView: nil];
      [clipView removeFromSuperview];
    }
  else
    [textObject removeFromSuperview];
}

/*
 * Editing Text
 */
/** <p>.This method does nothing if a the <var>controlView</var> is nil,
    if text object does not exist or if the cell's type is not <ref type="type"
    id="NSCellType">NSTextCellType</ref></p>
 */
- (void) editWithFrame: (NSRect)aRect
                inView: (NSView*)controlView
                editor: (NSText*)textObject
              delegate: (id)anObject
                 event: (NSEvent*)theEvent
{
  if (!controlView || !textObject || (_cell.type != NSTextCellType))
    return;

  [self _setupTextWithFrame: aRect
        inView: controlView
        editor: textObject
        delegate: anObject
        range: NSMakeRange(0, 0)];

  if ([theEvent type] == NSLeftMouseDown)
    {
      [textObject mouseDown: theEvent];
    }
}

/** <p> This method does nothing if the <var>controlView</var> is nil,
    if text object does not exist or if the cell's type is not <ref type="type"
    id="NSCellType">NSTextCellType</ref> </p>
 */
- (void) selectWithFrame: (NSRect)aRect
                  inView: (NSView*)controlView
                  editor: (NSText*)textObject
                delegate: (id)anObject
                   start: (NSInteger)selStart
                  length: (NSInteger)selLength
{
  if (!controlView || !textObject || (_cell.type != NSTextCellType))
    return;

  [self _setupTextWithFrame: aRect
        inView: controlView
        editor: textObject
        delegate: anObject
        range: NSMakeRange(selStart, selLength)];
}

- (BOOL) sendsActionOnEndEditing
{
  return _cell.sends_action_on_end_editing;
}

- (void) setSendsActionOnEndEditing: (BOOL)flag
{
  _cell.sends_action_on_end_editing = flag;
}

- (BOOL) allowsUndo
{
  return _cell.allows_undo;
}

- (void) setAllowsUndo: (BOOL)flag
{
  _cell.allows_undo = flag;
}

/*
 * Copying
 */
- (id) copyWithZone: (NSZone*)zone
{
  NSCell *c = (NSCell*)NSCopyObject (self, 0, zone);

  /* Hmmm. */
  c->_contents = [_contents copyWithZone: zone];
  /* Because of performance issues (and because so the doc says) only
     pointers to the objects are copied.  We need to RETAIN them all
     though. */
  _font = TEST_RETAIN (_font);
  _object_value = TEST_RETAIN (_object_value);
  _menu = TEST_RETAIN (_menu);
  _cell_image = TEST_RETAIN (_cell_image);
  _formatter = TEST_RETAIN (_formatter);
  _represented_object = TEST_RETAIN (_represented_object);

  return c;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      unsigned long cFlags = 0;
      unsigned int cFlags2 = 0;
      id contents = _contents;

      // encode contents
      [aCoder encodeObject: contents forKey: @"NSContents"];

      // flags
      cFlags |= [self focusRingType];
      cFlags |= [self showsFirstResponder] ?  0x4 : 0;
      cFlags |= (_action_mask & NSLeftMouseUpMask) ?  0 : 0x20;
      cFlags |= [self wraps] ?  0x40 : 0;
      cFlags |= (_action_mask & NSLeftMouseDraggedMask) ?  0x100 : 0;
      cFlags |= (_action_mask & NSLeftMouseDownMask) ?  0x40000 : 0;
      cFlags |= [self isContinuous] ? 0x80000 : 0;
      cFlags |= [self isScrollable] ? 0x100000 : 0;
      cFlags |= [self isSelectable] ? 0x200000 : 0;
      cFlags |= [self isBezeled] ? 0x400000 : 0;
      cFlags |= [self isBordered] ? 0x800000 : 0;
      cFlags |= ([self type] << 26);
      cFlags |= [self isEditable] ? 0x10000000 : 0;
      cFlags |= ([self isEnabled] == NO) ? 0x20000000 : 0;
      cFlags |= [self isHighlighted] ? 0x40000000 : 0;
      cFlags |= ([self state] == NSOnState) ? 0x80000000 : 0;
      [aCoder encodeInt: cFlags forKey: @"NSCellFlags"];

      // flags part 2
      cFlags2 |= ([self usesSingleLineMode] ? 0x40 : 0);
      cFlags2 |= (([self allowsUndo] == NO) ? 0x1000 : 0);
      cFlags2 |= ([self controlTint] << 5);
      cFlags2 |= ([self lineBreakMode] << 9);
      cFlags2 |= ([self controlSize] << 17);
      cFlags2 |= [self sendsActionOnEndEditing] ? 0x400000 : 0;
      cFlags2 |= [self allowsMixedState] ? 0x1000000 : 0;
      cFlags2 |= [self refusesFirstResponder] ? 0x2000000 : 0;
      cFlags2 |= ([self alignment] << 26);
      cFlags2 |= [self importsGraphics] ? 0x20000000 : 0;
      cFlags2 |= [self allowsEditingTextAttributes] ? 0x40000000 : 0;
      [aCoder encodeInt: cFlags2 forKey: @"NSCellFlags2"];

      if (_cell.type == NSTextCellType)
        {
          // font and formatter.
          if ([self font])
            {
              [aCoder encodeObject: [self font] forKey: @"NSSupport"];
            }

          if ([self formatter])
            {
              [aCoder encodeObject: [self formatter] forKey: @"NSFormatter"];
            }
        }
      else if ([self image])
        {
            [aCoder encodeObject: [self image] forKey: @"NSSupport"];
        }
    }
  else
    {
      BOOL flag;
      NSUInteger tmp_uint;

      [aCoder encodeObject: _contents];
      [aCoder encodeObject: _cell_image];
      [aCoder encodeObject: _font];
      [aCoder encodeObject: _object_value];
      flag = _cell.contents_is_attributed_string;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_highlighted;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_disabled;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_editable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_rich_text;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.imports_graphics;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.shows_first_responder;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.refuses_first_responder;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.sends_action_on_end_editing;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_bordered;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_bezeled;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_scrollable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.is_selectable;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      // This used to be is_continuous, which has been replaced.
      /* Ayers 20.03.2003: But we must continue to encode it for backward
         compatibility or current releases will have undefined behavior when
         decoding archives (i.e. .gorm files) encoded by this version. */
      flag = [self isContinuous];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = _cell.allows_mixed_state;
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      flag = [self wraps];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &flag];
      tmp_uint = _cell.text_align;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.type;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.image_position;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.entry_type;
      encode_NSUInteger(aCoder, &tmp_uint);
      // FIXME: State may be -1, why do we encode it as unsigned?
      tmp_uint = _cell.state;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.mnemonic_location;
      encode_NSUInteger(aCoder, &tmp_uint);
      encode_NSUInteger(aCoder, &_mouse_down_flags);
      encode_NSUInteger(aCoder, &_action_mask);
      [aCoder encodeValueOfObjCType: @encode(id) at: &_formatter];
      [aCoder encodeValueOfObjCType: @encode(id) at: &_menu];
      [aCoder encodeValueOfObjCType: @encode(id) at: &_represented_object];

      tmp_uint = _cell.allows_undo;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.line_break_mode;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.control_tint;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.control_size;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.focus_ring_type;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.base_writing_direction;
      encode_NSUInteger(aCoder, &tmp_uint);
      tmp_uint = _cell.uses_single_line_mode;
      encode_NSUInteger(aCoder, &tmp_uint);
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  if ([aDecoder allowsKeyedCoding])
    {
      id contents = [aDecoder decodeObjectForKey: @"NSContents"];

      // initialize based on content...
      if ([contents isKindOfClass: [NSString class]])
        {
          self = [self initTextCell: contents];
        }
      else if ([contents isKindOfClass: [NSImage class]])
        {
          self = [self initImageCell: contents];
        }
      else
        {
          self = [self init];
          [self setObjectValue: contents];
        }

      if ([aDecoder containsValueForKey: @"NSCellFlags"])
        {
          unsigned long cFlags;
          NSUInteger mask = 0;
          cFlags = [aDecoder decodeIntForKey: @"NSCellFlags"];

          [self setFocusRingType: (cFlags & 0x3)];
          [self setShowsFirstResponder: ((cFlags & 0x4) == 0x4)];
          // This bit flag is the other way around!
          if ((cFlags & 0x20) != 0x20)
              mask |= NSLeftMouseUpMask;
          // This bit flag is the other way around!
          [self setWraps: ((cFlags & 0x40) != 0x40)];
          if ((cFlags & 0x100) == 0x100)
            mask |= NSLeftMouseDraggedMask;
          if ((cFlags & 0x40000) == 0x40000)
            mask |= NSLeftMouseDownMask;
          if ((cFlags & 0x80000) == 0x80000)
            mask |= NSPeriodicMask;
          [self sendActionOn: mask];
          [self setScrollable: ((cFlags & 0x100000) == 0x100000)];
          [self setSelectable: ((cFlags & 0x200000) == 0x200000)];
          [self setBezeled: ((cFlags & 0x400000) == 0x400000)];
          [self setBordered: ((cFlags & 0x800000) == 0x800000)];
	  if (contents == nil)
	    {
	      //
	      // If the contents aren't set (the contents determine the type), 
	      // get it from the flags.  This prevents the type from being
	      // accidentally reset on some platforms (mainly WIN32) after 
	      // the contents are set.
	      //
	      [self setType: ((cFlags & 0xC000000) >> 26)];
	    }
          [self setEditable: ((cFlags & 0x10000000) == 0x10000000)];
          // This bit flag is the other way around!
          [self setEnabled: ((cFlags & 0x20000000) != 0x20000000)];
          [self setHighlighted: ((cFlags & 0x40000000) == 0x40000000)];
          [self setState: ((cFlags & 0x80000000) == 0x80000000) ? NSOnState : NSOffState];
        }
      if ([aDecoder containsValueForKey: @"NSCellFlags2"])
        {
          int cFlags2;

          cFlags2 = [aDecoder decodeIntForKey: @"NSCellFlags2"];
	  [self setUsesSingleLineMode: (cFlags2 & 0x40)];
          [self setControlTint: ((cFlags2 & 0xE0) >> 5)];
	  [self setLineBreakMode: ((cFlags2 & 0xE00) >> 9)];
          [self setControlSize: ((cFlags2 & 0xE0000) >> 17)];
          [self setSendsActionOnEndEditing: ((cFlags2 & 0x400000) == 0x400000)];
          [self setAllowsMixedState: ((cFlags2 & 0x1000000) == 0x1000000)];
          [self setRefusesFirstResponder: ((cFlags2 & 0x2000000) == 0x2000000)];
          [self setAlignment: ((cFlags2 & 0x1C000000) >> 26)];
          [self setImportsGraphics: ((cFlags2 & 0x20000000) == 0x20000000)];
          [self setAllowsEditingTextAttributes: ((cFlags2 & 0x40000000) == 0x40000000)];
        }
      if ([aDecoder containsValueForKey: @"NSSupport"])
        {
          id support = [aDecoder decodeObjectForKey: @"NSSupport"];

          if ([support isKindOfClass: [NSFont class]])
            {
              [self setFont: support];
            }
          else if ([support isKindOfClass: [NSImage class]])
            {
              [self setImage: support];
            }
        }
      if ([aDecoder containsValueForKey: @"NSFormatter"])
        {
          NSFormatter *formatter = [aDecoder decodeObjectForKey: @"NSFormatter"];

          [self setFormatter: formatter];
        }
    }
  else
    {
      BOOL flag, wraps;
      NSUInteger tmp_uint;
      id formatter, menu;
      int version = [aDecoder versionForClassName: @"NSCell"];

      [aDecoder decodeValueOfObjCType: @encode(id) at: &_contents];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_cell_image];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_font];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_object_value];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.contents_is_attributed_string = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_highlighted = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_disabled = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_editable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_rich_text = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.imports_graphics = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.shows_first_responder = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.refuses_first_responder = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.sends_action_on_end_editing = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_bordered = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_bezeled = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_scrollable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.is_selectable = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      // This used to be is_continuous, which has been replaced.
      //_cell.is_continuous = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      _cell.allows_mixed_state = flag;
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &flag];
      /* The wraps attribute has been superseded by lineBreakMode. However,
	 we may need it to set lineBreakMode when reading old archives. */
      wraps = flag;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.text_align = tmp_uint;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.type = tmp_uint;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.image_position = tmp_uint;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.entry_type = tmp_uint;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.state = tmp_uint;
      decode_NSUInteger(aDecoder, &tmp_uint);
      _cell.mnemonic_location = tmp_uint;
      decode_NSUInteger(aDecoder, &_mouse_down_flags);
      decode_NSUInteger(aDecoder, &_action_mask);
      if (version < 3)
        {
          unsigned int mask = 0;

          // Convert old GNUstep mask value to Cocoa values
          if ((_action_mask & 0x1) == 0x1)
            {
              mask |= NSLeftMouseDownMask;
            }
          if ((_action_mask & 0x2) == 0x2)
            {
              mask |= NSLeftMouseUpMask;
            }
          if ((_action_mask & 0x4) == 0x4)
            {
              mask |= NSOtherMouseDownMask;
            }
          if ((_action_mask & 0x8) == 0x8)
            {
              mask |= NSOtherMouseUpMask;
            }
          if ((_action_mask & 0x10) == 0x10)
            {
              mask |= NSRightMouseDownMask;
            }
          if ((_action_mask & 0x20) == 0x20)
            {
              mask |= NSRightMouseUpMask;
            }
          if ((_action_mask & 0x40) == 0x40)
            {
              mask |= NSMouseMovedMask;
            }
          if ((_action_mask & 0x80) == 0x80)
            {
              mask |= NSLeftMouseDraggedMask;
            }
          if ((_action_mask & 0x100) == 0x100)
            {
              mask |= NSOtherMouseDraggedMask;
            }
          if ((_action_mask & 0x200) == 0x200)
            {
              mask |= NSRightMouseDraggedMask;
            }
          if ((_action_mask & 0x400) == 0x400)
            {
              mask |= NSMouseEnteredMask;
            }
          if ((_action_mask & 0x800) == 0x800)
            {
              mask |= NSMouseExitedMask;
            }
          if ((_action_mask & 0x1000) == 0x1000)
            {
              mask |= NSKeyDownMask;
            }
          if ((_action_mask & 0x2000) == 0x2000)
            {
              mask |= NSKeyUpMask;
            }
          if ((_action_mask & 0x4000) == 0x4000)
            {
              mask |= NSFlagsChangedMask;
            }
          if ((_action_mask & 0x8000) == 0x8000)
            {
              mask |= NSAppKitDefinedMask;
            }
          if ((_action_mask & 0x10000) == 0x10000)
            {
              mask |= NSSystemDefinedMask;
            }
          if ((_action_mask & 0x20000) == 0x20000)
            {
              mask |= NSApplicationDefinedMask;
            }
          if ((_action_mask & 0x40000) == 0x40000)
            {
              mask |= NSPeriodicMask;
            }
          if ((_action_mask & 0x80000) == 0x80000)
            {
              mask |= NSCursorUpdateMask;
            }
          if ((_action_mask & 0x100000) == 0x100000)
            {
              mask |= NSScrollWheelMask;
            }
          _action_mask = mask;
        }
      _action_mask |= NSLeftMouseUpMask;
      [aDecoder decodeValueOfObjCType: @encode(id) at: &formatter];
      [self setFormatter: formatter];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &menu];
      [self setMenu: menu];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_represented_object];

      if (_formatter != nil)
        {
          NSString *contents;

          contents = [_formatter stringForObjectValue: _object_value];
          if (contents != nil)
            {
              _cell.has_valid_object_value = YES;
              ASSIGN (_contents, contents);
              _cell.contents_is_attributed_string = NO;
            }
        }

      if (version >= 2)
        {
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.allows_undo = tmp_uint;
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.line_break_mode = tmp_uint;
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.control_tint = tmp_uint;
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.control_size = tmp_uint;
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.focus_ring_type = tmp_uint;
          decode_NSUInteger(aDecoder, &tmp_uint);
          _cell.base_writing_direction = tmp_uint;
        }
      else
	{
	  /* Backward compatibility: Derive lineBreakMode from the superseded
	     wraps attribute. */
	  [self setWraps: wraps];
	}

      if (version >= 4)
	{
          decode_NSUInteger(aDecoder, &tmp_uint);
	  _cell.uses_single_line_mode = tmp_uint;
	}

    }
  return self;
}

- (NSUserInterfaceLayoutDirection) userInterfaceLayoutDirection
{
  // FIXME
  return NSUserInterfaceLayoutDirectionLeftToRight;
}

- (void) setUserInterfaceLayoutDirection: (NSUserInterfaceLayoutDirection)dir
{
  // FIXME: implement this
  return;
}

- (void) setUsesSingleLineMode: (BOOL)flag
{
  _cell.uses_single_line_mode = flag;
}

- (BOOL) usesSingleLineMode
{
  return _cell.uses_single_line_mode;
}

@end

@implementation NSCell (PrivateMethods)

- (NSColor*) textColor
{
  if (_cell.is_disabled)
    return dtxtCol;
  else
    return txtCol;
}

/* This method is an exception and returns a non-autoreleased
   dictionary, so that calling methods can deallocate it immediately
   using release.  Otherwise if many cells are drawn/their size
   computed, we pile up hundreds or thousands of these objects before they
   are deallocated at the end of the run loop. */
- (NSDictionary*) _nonAutoreleasedTypingAttributes
{
  NSDictionary *attr;
  NSColor *color;
  NSMutableParagraphStyle *paragraphStyle;

  color = [self textColor];
  /* Note: There are only a few possible paragraph styles for cells.
     TODO: Cache them and reuse them for the whole app lifetime. */
  paragraphStyle = [[NSParagraphStyle defaultParagraphStyle] mutableCopy];
  [paragraphStyle setLineBreakMode: [self lineBreakMode]];
  [paragraphStyle setBaseWritingDirection: [self baseWritingDirection]];
  [paragraphStyle setAlignment: [self alignment]];

  attr = [[NSDictionary alloc] initWithObjectsAndKeys:
                               _font, NSFontAttributeName,
                               color, NSForegroundColorAttributeName,
                               paragraphStyle, NSParagraphStyleAttributeName,
                               nil];
  RELEASE (paragraphStyle);
  return attr;
}

- (NSSize) _sizeText: (NSString*)title
{
  NSSize size;
  NSDictionary *dict;

  if (title == nil)
    {
      return NSMakeSize (0,0);
    }

  dict = [self _nonAutoreleasedTypingAttributes];
  size = [title sizeWithAttributes: dict];
  RELEASE (dict);
  return size;
}

/**
 * Private internal method, returns an attributed string to display.
 */
- (NSAttributedString*) _drawAttributedString
{
  if (!_cell.is_disabled)
    {
      return [self attributedStringValue];
    }
  else
    {
      NSAttributedString *attrStr = [self attributedStringValue];
      NSDictionary *attribs;
      NSMutableDictionary *newAttribs;

      attribs = [attrStr attributesAtIndex: 0
                         effectiveRange: NULL];
      newAttribs = [NSMutableDictionary dictionaryWithDictionary: attribs];
      [newAttribs setObject: [NSColor disabledControlTextColor]
                  forKey: NSForegroundColorAttributeName];

      return AUTORELEASE([[NSAttributedString alloc]
                             initWithString: [attrStr string]
                             attributes: newAttribs]);
    }
}


- (BOOL) _shouldShortenStringForRect: (NSRect)titleRect size: (NSSize)titleSize length: (NSUInteger)length
{
  NSLineBreakMode mode = [self lineBreakMode];

  return ((titleSize.width > titleRect.size.width) && (length > 4) &&
          (mode == NSLineBreakByTruncatingHead ||
           mode == NSLineBreakByTruncatingTail ||
           mode == NSLineBreakByTruncatingMiddle));
}

- (NSAttributedString*) _resizeAttributedString: (NSAttributedString*)attrstring forRect: (NSRect)titleRect
{
  // Redo string based on selected truncation mask...
  NSMutableAttributedString *mutableString = AUTORELEASE([attrstring mutableCopy]);
  NSString *ellipsis = @"...";
  NSLineBreakMode mode = [self lineBreakMode];
  // This code shortens the string one character at a time.
  // To speed it up we start off proportional:
  CGFloat width = [mutableString size].width;
  int cut = MAX(floor([mutableString length] * (width - titleRect.size.width) / width), 4);

  do
    {
      NSRange replaceRange;
      if (mode == NSLineBreakByTruncatingHead)
        replaceRange = NSMakeRange(0, cut);
      else if (mode == NSLineBreakByTruncatingTail)
        replaceRange = NSMakeRange([mutableString length] - cut, cut);
      else
        replaceRange = NSMakeRange(([mutableString length] / 2) - (cut / 2), cut);
      [mutableString replaceCharactersInRange: replaceRange withString: ellipsis];
      cut = 4;
    } while ([mutableString length] > 4 && [mutableString size].width > titleRect.size.width);

  // Return the modified attributed string...
  return mutableString;
}

/**
 * Private internal method to display an attributed string.
 */
- (void) _drawAttributedText: (NSAttributedString*)aString
                     inFrame: (NSRect)aRect
{
  NSSize titleSize;

  if (aString == nil)
    return;

  titleSize = [aString size];
  if ([self _shouldShortenStringForRect: aRect size: titleSize length: [aString length]])
    {
      aString = [self _resizeAttributedString: aString forRect: aRect];
      titleSize = [aString size];
    }

  /** Important: text should always be vertically centered without
   * considering descender [as if descender did not exist].
   * This is particularly important for single line texts.
   * Please make sure the output remains always correct.
   */
  aRect.origin.y = NSMidY (aRect) - titleSize.height/2;
  aRect.size.height = titleSize.height;

  [aString drawInRect: aRect];
}

- (void) _drawText: (NSString*)aString  inFrame: (NSRect)cellFrame
{
  NSSize titleSize;
  NSDictionary *attributes;

  if (aString == nil)
    return;

  attributes = [self _nonAutoreleasedTypingAttributes];
  titleSize = [aString sizeWithAttributes: attributes];
  if ([self _shouldShortenStringForRect: cellFrame size: titleSize length: [aString length]])
    {
      NSAttributedString *attrstring = AUTORELEASE([[NSAttributedString alloc] initWithString: aString
                                                                                   attributes: attributes]);
      return [self _drawAttributedText: attrstring inFrame: cellFrame];
    }

  /** Important: text should always be vertically centered without
   * considering descender [as if descender did not exist].
   * This is particularly important for single line texts.
   * Please make sure the output remains always correct.
   */
  cellFrame.origin.y = NSMidY (cellFrame) - titleSize.height/2;
  cellFrame.size.height = titleSize.height;

  [aString drawInRect: cellFrame  withAttributes: attributes];
  RELEASE (attributes);
}

// Private helper method overridden in subclasses
- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame
                                    inView: (NSView*)controlView
{
  NSBorderType aType;

  // Get border size
  if (_cell.is_bordered)
    aType = NSLineBorder;
  else if (_cell.is_bezeled)
    aType = NSBezelBorder;
  else
    aType = NSNoBorder;

  [[GSTheme theme] drawBorderType: aType frame: cellFrame view: controlView];
}

// Private helper method
- (void) _drawFocusRingWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  if (_cell.shows_first_responder
    && [[controlView window] firstResponder] == controlView)
    {
      switch (_cell.focus_ring_type)
        {
          case NSFocusRingTypeDefault:
            [[GSTheme theme] drawFocusFrame: [self drawingRectForBounds:
                                                       cellFrame]
                             view: controlView];
            break;
          case NSFocusRingTypeExterior:
            [[GSTheme theme] drawFocusFrame: cellFrame
                             view: controlView];
            break;
          case NSFocusRingTypeNone:
          default:
            break;
        }
    }
}

- (void) _drawEditorWithFrame: (NSRect)cellFrame
		       inView: (NSView *)controlView
{
  /* Look Ma', no drawing here... */

  if ([controlView isKindOfClass: [NSControl class]])
    {
      /* Adjust the text editor's frame to match cell's frame (w/o border) */
      NSRect titleRect = [self titleRectForBounds: cellFrame];
      NSText *textObject = [(NSControl*)controlView currentEditor];
      NSView *clipView = [textObject superview];

      if ([clipView isKindOfClass: [NSClipView class]])
	{
	  [clipView setFrame: titleRect];
	}
      else
	{
	  [textObject setFrame: titleRect];
	}
    }
}

- (BOOL) _sendsActionOn:(NSUInteger)eventTypeMask
{
  return (_action_mask & eventTypeMask);
}

- (void) _setInEditing: (BOOL)flag
{
  _cell.in_editing = flag;
}

- (BOOL) _inEditing
{
  return _cell.in_editing;
}

- (void) _updateFieldEditor: (NSText*)textObject
{
  if (_formatter != nil)
    {
      NSString *contents;

      contents = [_formatter editingStringForObjectValue: _object_value];
      if (contents == nil)
        {
          // We cannot call the stringValue method as this will call
          // validateEditing in NSActionCell subclasses
          if (nil == _contents)
            {
              contents = @"";
            }
	  else
	    {
	      if (_cell.contents_is_attributed_string == NO)
		{
		  contents = (NSString *)_contents;
		}
	      else
		{
		  contents = [(NSAttributedString *)_contents string];
		}
	    }
        }
      if (![contents isEqualToString: [textObject string]])
	[textObject setText: contents];
    }
  else
    {
      NSString *contents;

      if (nil == _contents)
        {
          contents = @"";
        }
      else
	{
	  if (_cell.contents_is_attributed_string == NO)
	    {
	      contents = (NSString *)_contents;
	    }
	  else
	    {
	      contents = [(NSAttributedString *)_contents string];
	    }
	}
      if (![contents isEqualToString: [textObject string]])
        {
          if (_cell.contents_is_attributed_string == NO)
            {
              [textObject setText: contents];
            }
          else
            {
              // FIXME what about attribute changes?
              NSRange range = NSMakeRange(0, [[textObject string] length]);
              [textObject replaceCharactersInRange: range
                              withAttributedString: (NSAttributedString *)_contents];
            }
        }
    }
}

/**
 * Private method used by NSImageCell and NSButtonCell for calculating
 * scaled image size
 */

static inline NSSize
scaleProportionally(NSSize imageSize, NSSize canvasSize, BOOL scaleUpOrDown)
{
  CGFloat ratio;

  if (imageSize.width <= 0
      || imageSize.height <= 0)
    {
      return NSMakeSize(0, 0);
    }

  /* Get the smaller ratio and scale the image size by it.  */
  ratio = MIN(canvasSize.width / imageSize.width,
	      canvasSize.height / imageSize.height);

  /* Only scale down, unless scaleUpOrDown is YES */
  if (ratio < 1.0 || scaleUpOrDown)
    {
      imageSize.width *= ratio;
      imageSize.height *= ratio;
    }

  return imageSize;
}

- (NSSize) _scaleImageWithSize: (NSSize)imageSize
                   toFitInSize: (NSSize)canvasSize
		   scalingType: (NSImageScaling)scalingType
{
  NSSize result;
  switch (scalingType)
    {
    case NSImageScaleProportionallyDown: // == NSScaleProportionally
      {
	result = scaleProportionally (imageSize, canvasSize, NO);
	break;
      }
    case NSImageScaleAxesIndependently: // == NSScaleToFit
      {
	result = canvasSize;
	break;
      }
    default:
    case NSImageScaleNone: // == NSScaleNone
      {
	result = imageSize;
	break;
      }
    case NSImageScaleProportionallyUpOrDown:
      {
	result = scaleProportionally (imageSize, canvasSize, YES);
	break;
      }
    }
  return result;
}

@end
