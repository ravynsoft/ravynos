/** <title>NSButtonCell</title>

   <abstract>The button cell class</abstract>

   Copyright (C) 1996-1999 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
            Ovidiu Predescu <ovidiu@net-community.com>
   Date: 1996
   Author:  Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998

   Modified: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   
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
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

#import "AppKit/AppKitExceptions.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSBezierPath.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSSound.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSWindow.h"
#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSNibLoading.h"
#import "GSGuiPrivate.h"
#import "GSCodingFlags.h"

#include <math.h>

@interface NSCell (Private)
- (NSSize) _scaleImageWithSize: (NSSize)imageSize
                   toFitInSize: (NSSize)canvasSize
                   scalingType: (NSImageScaling)scalingType;
@end

/**<p> TODO Description</p>
 */
@implementation NSButtonCell

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSButtonCell class])
    [self setVersion: 3];
}

/*
 * Instance methods
 */
- (id) _init
{
  // Implicitly performed by allocation:
  //
  //_buttoncell_is_transparent = NO;

  [self setAlignment: NSCenterTextAlignment];
  _cell.is_bordered = YES;
  [self setButtonType: NSMomentaryPushInButton];
  _delayInterval = 0.4;
  _repeatInterval = 0.075;
  _keyEquivalentModifierMask = 0;
  _keyEquivalent = @"";
  _altContents = @"";
  _gradient_type = NSGradientNone;
  [self setImageScaling: NSImageScaleNone];

  return self;
}

- (id) init
{
  self = [self initTextCell: @"Button"];

  return self;
}

- (id) initImageCell: (NSImage*)anImage
{
  self = [super initImageCell: anImage];
  if (!self)
    return nil;

  return [self _init];
}

- (id) initTextCell: (NSString*)aString
{
  self = [super initTextCell: aString];
  if (!self)
    return nil;

  return [self _init];
}

- (void) dealloc
{
  RELEASE(_altContents);
  RELEASE(_altImage);
  RELEASE(_keyEquivalent);
  RELEASE(_keyEquivalentFont);
  RELEASE(_sound);
  RELEASE(_backgroundColor);

  [super dealloc];
}

/** 
 *<p>The GNUstep implementation does nothing here
 *  (to match the Mac OS X behavior) because with
 * NSButtonCell GNUstep implementation the cell type is bound to the image
 * position. We implemented this behavior because it permits to have
 * -setFont: -setTitle -setImage: methods which are symetrical by not altering
 * directly the cell type and to validate the fact that the cell type is more
 * characterized by the potential visibility of the image (which is under the
 * control of the method -setImagePosition:) than by the value of the image
 * ivar itself (related to -setImage: method).  
 * On Mac OS X, the NSButtonCell cell type is NSTextCellType by default or
 * NSImageCellType if the initialization has been done with -initImageCell:,
 * it should be noted that the cell type never changes later.</p>
 */
- (void) setType: (NSCellType)aType
{
}

/** <p>Returns the NSButtonCell's title.</p>
 */
- (NSString*) title
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

/** <p>Returns the NSButtonCell's alternate title ( used when highlighted ).
    </p><p>See Also: -setAlternateTitle:</p>
 */
- (NSString*) alternateTitle
{
  if (_altContents != nil)
    {
      return _altContents;
    }
  else
    {
      return @"";
    }
}

- (NSInteger) cellAttribute: (NSCellAttribute)aParameter
{
  NSInteger value = 0;

  switch (aParameter)
    {
    case NSPushInCell:
      if (_highlightsByMask & NSPushInCellMask)
        value = 1;
      break;
    case NSChangeGrayCell:
      if (_showAltStateMask & NSChangeGrayCellMask)
        value = 1;
      break;
    case NSCellLightsByGray:
      if (_highlightsByMask & NSChangeGrayCellMask)
        value = 1;
      break;
    case NSChangeBackgroundCell:
      if (_showAltStateMask & NSChangeBackgroundCellMask)
        value = 1;
      break;
    case NSCellLightsByBackground:
      if (_highlightsByMask & NSChangeBackgroundCellMask)
        value = 1;
      break;
    case NSCellChangesContents:
      if (_showAltStateMask & NSContentsCellMask)
        value = 1;
      break;
    case NSCellLightsByContents:
      if (_highlightsByMask & NSContentsCellMask)
        value = 1;
      break;
    default:
      value = [super cellAttribute: aParameter];
      break;
    }

  return value;
}

- (void) setCellAttribute: (NSCellAttribute)aParameter to: (NSInteger)value
{
  switch (aParameter)
    {
    case NSPushInCell:
      if (value)
        _highlightsByMask |= NSPushInCellMask;
      else
        _highlightsByMask &= ~NSPushInCellMask;
      break;
    case NSChangeGrayCell:
      if (value)
        _showAltStateMask |= NSChangeGrayCellMask;
      else
        _showAltStateMask &= ~NSChangeGrayCellMask;
      break;
    case NSChangeBackgroundCell:
      if (value)
        _showAltStateMask |= NSChangeBackgroundCellMask;
      else
        _showAltStateMask &= ~NSChangeBackgroundCellMask;
      break;
    case NSCellChangesContents:
      if (value)
        _showAltStateMask |= NSContentsCellMask;
      else
        _showAltStateMask &= ~NSContentsCellMask;
      break;
    case NSCellLightsByGray:
      if (value)
        _highlightsByMask |= NSChangeGrayCellMask;
      else
        _highlightsByMask &= ~NSChangeGrayCellMask;
      break;
    case NSCellLightsByBackground:
      if (value)
        _highlightsByMask |= NSChangeBackgroundCellMask;
      else
        _highlightsByMask &= ~NSChangeBackgroundCellMask;
      break;
    case NSCellLightsByContents:
      if (value)
        _highlightsByMask |= NSContentsCellMask;
      else
        _highlightsByMask &= ~NSContentsCellMask;
      break;
    default:
      [super setCellAttribute: aParameter to: value];
    }
}

/** <p>Sets the NSButtonCell's font to <var>fontObject</var>.
    The key equivalent font size is changed to match the <var>fontObject</var>
    if needed.</p><p>See Also: [NSCell-font] -keyEquivalentFont 
    -setKeyEquivalentFont: -setKeyEquivalentFont:size:</p>
 */
- (void) setFont: (NSFont*)fontObject
{
  int size;

  [super setFont: fontObject];

  if ((_keyEquivalentFont != nil) && (fontObject != nil)
    && ((size = [fontObject pointSize]) != [_keyEquivalentFont pointSize]))
    {
      [self setKeyEquivalentFont: [_keyEquivalentFont fontName] 
            size: size];       
    }
}

/**<p>Sets the NSButtonCell's title to <var>aString</var>.</p>
 */
- (void) setTitle: (NSString*)aString
{
  ASSIGNCOPY(_contents, aString);
  _cell.contents_is_attributed_string = NO;

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

/**<p>Sets the NSButtonCell's alternate title ( used when highlighted ) 
   to <var>aString</var> and update the cell if it contains 
   a NSControl view.</p><p>See Also: -alternateTitle</p>
 */
- (void) setAlternateTitle: (NSString*)aString
{
  ASSIGNCOPY(_altContents, aString);

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

- (NSAttributedString *)attributedAlternateTitle
{
  // TODO
  NSDictionary *dict;
  NSAttributedString *attrStr;

  dict = [self _nonAutoreleasedTypingAttributes];
  attrStr = [[NSAttributedString alloc] initWithString: [self alternateTitle]
                                        attributes: dict];
  RELEASE(dict);

  return AUTORELEASE(attrStr);
}

- (void)setAttributedAlternateTitle:(NSAttributedString *)aString
{
  // TODO
  [self setAlternateTitle: [aString string]];
}

- (NSAttributedString *)attributedTitle
{
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
      attrStr = [[NSAttributedString alloc] initWithString: [self title]
                                            attributes: dict];
      RELEASE(dict);
      return AUTORELEASE(attrStr);
    }
}

- (void)setAttributedTitle:(NSAttributedString *)aString
{
  ASSIGNCOPY(_contents, aString);
  _cell.contents_is_attributed_string = YES;

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

- (void)setTitleWithMnemonic:(NSString *)aString
{
  // TODO
  [super setTitleWithMnemonic: aString];
}

- (NSString *)alternateMnemonic
{
  // TODO
  return @"";
}

- (NSUInteger)alternateMnemonicLocation
{
  // TODO
  return NSNotFound;
}

- (void)setAlternateMnemonicLocation:(NSUInteger)location
{
  // TODO
}

- (void)setAlternateTitleWithMnemonic:(NSString *)aString
{
  NSUInteger location = [aString rangeOfString: @"&"].location;

  [self setAlternateTitle: [aString stringByReplacingString: @"&"
                                    withString: @""]];
  // TODO: We should underline this character
  [self setAlternateMnemonicLocation: location];
}

/**<p>Returns the NSButtonCell's alternate image.</p>
   <p>See Also: -setAlternateImage:</p> 
 */
- (NSImage*) alternateImage
{
  return _altImage;
}

/** <p>Returns the NSButtonCell's image position. See <ref type="type" 
    id="NSCellImagePosition">NSCellImagePosition</ref> for more information.
    </p><p>See Also: -setImagePosition:</p>
 */
- (NSCellImagePosition) imagePosition
{
  return _cell.image_position;
}

- (NSImageScaling) imageScaling
{
  return _imageScaling;
}

- (void) setImageScaling: (NSImageScaling)scaling
{
  _imageScaling = scaling;
}

- (void) setImage: (NSImage *)anImage
{
  if (_cell.image_position == NSNoImage)
    {
      [self setImagePosition: NSImageOnly];
    }
  
  [super setImage: anImage];
}

/**<p>Sets the NSButtonCell's alternate image to <var>anImage</var>.</p>
   <p>See Also: -alternateImage</p>
 */
- (void) setAlternateImage: (NSImage*)anImage
{
  ASSIGN(_altImage, anImage);

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

/**<p>Sets the image position. The GNUstep implementation depends only on 
   the image position. If the image position is set to <ref type="type"
   id="NSCellImagePosition">NSNoImage</ref> then the type is set to
   <ref type="type" id="NSCellImagePosition">NSTextCellType</ref>, to
   <ref type="type" id="NSCellImagePosition">NSImageCellType</ref> otherwise
   </p><p>See Also: -imagePosition</p>
*/
- (void) setImagePosition: (NSCellImagePosition)aPosition
{
  _cell.image_position = aPosition;
   
  // In the GNUstep NSButtonCell implementation, the cell type depends only on
  // the image position. 
  /* NOTE: We set the cell type attribute directly here instead of calling
     NSCell's -setType: method because it may change the title or image of
     the button cell. This is to make our implementation compatible with
     the behavior of Mac OS X, which does not change the cell's type and
     hence does not involve any of the side effects of -setType: either. */
  if (_cell.image_position == NSNoImage)
    {
      _cell.type = NSTextCellType;
    }
  else
    {
      _cell.type = NSImageCellType;
    }

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

/**<p>Gets the NSButtonCell's <var>delay</var> and the <var>interval</var>
   parameters used when NSButton sends continouly action messages.
   By default <var>delay</var> is 0.4 and <var>interval</var> is 0.075.</p>
   <p>See Also: -setPeriodicDelay:interval:
   [NSCell-trackMouse:inRect:ofView:untilMouseUp:]</p>
 */
- (void) getPeriodicDelay: (float*)delay interval: (float*)interval
{
  *delay = _delayInterval;
  *interval = _repeatInterval;
}

/** <p>Sets the NSButtonCell's  <var>delay</var> and <var>interval</var> 
    parameters used when NSButton sends continouly action messages.
    By default <var>delay</var> is 0.4 and <var>interval</var> is 0.075.</p>
    <p>See Also: -getPeriodicDelay:interval: 
    [NSCell-trackMouse:inRect:ofView:untilMouseUp:]</p>
 */
- (void) setPeriodicDelay: (float)delay interval: (float)interval
{
  _delayInterval = delay;
  _repeatInterval = interval;
}

/**<p>Returns the NSButtonCell's key equivalent. The key equivalent and its
   modifier mask are used to simulate the click of the button in
   [NSButton-performKeyEquivalent:]. Returns an empty string if no key
   equivalent is defined. By default NSButtonCell hasn't key equivalent.</p>
   <p>See Also: -setKeyEquivalent: [NSButton-performKeyEquivalent:]
   -keyEquivalentModifierMask [NSButtonCell-keyEquivalent]</p>
 */
- (NSString*) keyEquivalent
{
  if (nil == _keyEquivalent)
    {
      return @"";
    }
  else
    {
      return _keyEquivalent;
    }
}

/**<p>Returns the NSFont of the key equivalent.</p>
 *<p>See Also: -setKeyEquivalentFont:</p>
 */
- (NSFont*) keyEquivalentFont
{
  return _keyEquivalentFont;
}

/** <p>Returns the modifier mask of the NSButtonCell's key equivalent. 
    The key equivalent and its modifier mask are used to simulate the click
    of the button in  [NSButton-performKeyEquivalent:]. The default mask is 
    0.</p><p>See Also: -setKeyEquivalentModifierMask:
    -keyEquivalent [NSButton-performKeyEquivalent:]</p>
 */
- (NSUInteger) keyEquivalentModifierMask
{
  return _keyEquivalentModifierMask;
}

/** <p>Sets the NSButtonCell's key equivalent to <var>key</var>. The key
    equivalent and its modifier mask are used to simulate the click
    of the button in  [NSButton-performKeyEquivalent:]. By default NSButton
    hasn't   key equivalent.</p><p>See Also: -keyEquivalent 
    -setKeyEquivalentModifierMask: [NSButton-performKeyEquivalent:]</p>
*/
- (void) setKeyEquivalent: (NSString*)key
{
  [[GSTheme theme] setKeyEquivalent: key
		      forButtonCell: self];
  ASSIGNCOPY(_keyEquivalent, key);
}

/** <p>Sets the modifier mask of the NSButtonCell's key equivalent to
    <var>mask</var>. The key equivalent and its modifier mask are used to
    simulate the click of the button in [NSButton-performKeyEquivalent:]. 
    By default the mask is 0.</p>
    <p>See Also: -keyEquivalentModifierMask  
    -setKeyEquivalent: [NSButton-performKeyEquivalent:]</p>
*/
- (void) setKeyEquivalentModifierMask: (NSUInteger)mask
{
  _keyEquivalentModifierMask = mask;
}

/**<p>Sets the NSFont of the key equivalent to <var>fontObject</var>.</p>
 *<p>See Also: -keyEquivalentFont -setFont:</p>
 */
- (void) setKeyEquivalentFont: (NSFont*)fontObj
{
  ASSIGN(_keyEquivalentFont, fontObj);
}

/**<p>Sets the NSFont with size <var>fontSize</var> of the key equivalent 
   to <var>fontName</var>.</p>
   <p>See Also: -keyEquivalentFont -setKeyEquivalentFont: -setFont:</p>
 */
- (void) setKeyEquivalentFont: (NSString*)fontName size: (float)fontSize
{
  ASSIGN(_keyEquivalentFont, [NSFont fontWithName: fontName size: fontSize]);
}

/**<p>Returns whether the button cell is transparent.</p>
   <p>See Also: -setTransparent:</p>
 */
- (BOOL) isTransparent
{
  return _buttoncell_is_transparent;
}

/**<p>Sets whether the button cell is transparent.</p>
   <p>See Also: -isTransparent </p>
 */
- (void) setTransparent: (BOOL)flag
{
  _buttoncell_is_transparent = flag;
}

/**<p>Returns whether the NSButtonCell is opaque. Currently always
   returns NO</p>*/
- (BOOL) isOpaque
{
  // May not be opaque, due to themes
  return NO;

  //  return !_buttoncell_is_transparent && _cell.is_bordered &&
  //    _bezel_style == 0;
}

- (NSBezelStyle) bezelStyle
{
  return _bezel_style;
}

- (void) setBezelStyle: (NSBezelStyle)bezelStyle
{
  _bezel_style = bezelStyle;
}

- (BOOL) showsBorderOnlyWhileMouseInside
{
  return _shows_border_only_while_mouse_inside;
}

- (void) setShowsBorderOnlyWhileMouseInside: (BOOL)show
{
  if (_shows_border_only_while_mouse_inside == show)
    {
      return;
    }

  _shows_border_only_while_mouse_inside = show;
  // FIXME Switch mouse tracking on
}

- (NSGradientType) gradientType
{
  return _gradient_type;
}

- (void) setGradientType: (NSGradientType)gradientType
{
  _gradient_type = gradientType;
}

- (BOOL) imageDimsWhenDisabled
{
  return _image_dims_when_disabled;
}

- (void) setImageDimsWhenDisabled:(BOOL)flag
{
  _image_dims_when_disabled = flag;
}

/**<p>Returns a mask describing how the button cell is highlighted : </p>
   <p> NSNoCellMask, NSContentsCellMask,NSPushInCellMask,NSChangeGrayCellMask,
   NSChangeBackgroundCellMask</p>
   <p>See Also : -setHighlightsBy:</p>
 */
- (NSInteger) highlightsBy
{
  return _highlightsByMask;
}

/**<p>Sets a mask describing how the button cell is highlighted :  </p>
   <p> NSNoCellMask, NSContentsCellMask,NSPushInCellMask,NSChangeGrayCellMask,
   NSChangeBackgroundCellMask</p>
   <p>See Also : -highlightsBy</p>
 */
- (void) setHighlightsBy: (NSInteger)mask
{
  _highlightsByMask = mask;
}

- (void) setShowsStateBy: (NSInteger)mask
{
  _showAltStateMask = mask;
}

- (void) setButtonType: (NSButtonType)buttonType
{
  // Don't store the button type anywhere

  switch (buttonType)
    {
      case NSMomentaryLightButton: 
        [self setHighlightsBy: NSChangeBackgroundCellMask];
        [self setShowsStateBy: NSNoCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSMomentaryPushInButton: 
        [self setHighlightsBy: NSPushInCellMask | NSChangeGrayCellMask];
        [self setShowsStateBy: NSNoCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSMomentaryChangeButton: 
        [self setHighlightsBy: NSContentsCellMask];
        [self setShowsStateBy: NSNoCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSPushOnPushOffButton: 
        [self setHighlightsBy: NSPushInCellMask | NSChangeGrayCellMask];
        [self setShowsStateBy: NSChangeBackgroundCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSOnOffButton: 
        [self setHighlightsBy: NSChangeBackgroundCellMask];
        [self setShowsStateBy: NSChangeBackgroundCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSToggleButton: 
        [self setHighlightsBy: NSPushInCellMask | NSContentsCellMask];
        [self setShowsStateBy: NSContentsCellMask];
        [self setImageDimsWhenDisabled: YES];
        break;
      case NSSwitchButton: 
        [self setHighlightsBy: NSContentsCellMask];
        [self setShowsStateBy: NSContentsCellMask];
        [self setImage: [NSImage imageNamed: @"NSSwitch"]];
        [self setAlternateImage: [NSImage imageNamed: @"NSHighlightedSwitch"]];
        [self setImagePosition: NSImageLeft];
        [self setAlignment: NSLeftTextAlignment];
        [self setBordered: NO];
        [self setBezeled: NO];
        [self setImageDimsWhenDisabled: NO];
        break;
      case NSRadioButton: 
        [self setHighlightsBy: NSContentsCellMask];
        [self setShowsStateBy: NSContentsCellMask];
        [self setImage: [NSImage imageNamed: @"NSRadioButton"]];
        [self setAlternateImage: [NSImage imageNamed: @"NSHighlightedRadioButton"]];
        [self setImagePosition: NSImageLeft];
        [self setAlignment: NSLeftTextAlignment];
        [self setBordered: NO];
        [self setBezeled: NO];
        [self setImageDimsWhenDisabled: NO];
        break;
      default:
        NSLog(@"Using unsupported button type %d", buttonType);
        break;
    }
}

- (NSInteger) showsStateBy
{
  return _showAltStateMask;
}

- (void) setIntValue: (int)anInt
{
  [self setState: (anInt != 0)];
}

- (void) setFloatValue: (float)aFloat
{
  [self setState: (aFloat != 0)];
}

- (void) setDoubleValue: (double)aDouble
{
  [self setState: (aDouble != 0)];
}

- (int) intValue
{
  return _cell.state;
}

- (float) floatValue
{
  return _cell.state;
}

- (double) doubleValue
{
  return _cell.state;
}

- (void) setObjectValue: (id)object
{
  if (object == nil)
    {
      [self setState: NSOffState];
    }
  else if ([object respondsToSelector: @selector(intValue)])
    {
      [self setState: [object intValue]];
    }
  else
    {
      [self setState: NSOnState];
    }
}

- (id) objectValue
{
  if (_cell.state == NSOffState)
    {        
      return [NSNumber numberWithBool: NO];
    }
  else if (_cell.state == NSOnState)
    {        
      return [NSNumber numberWithBool: YES];
    }
  else // NSMixedState
    {        
      return [NSNumber numberWithInt: -1];
   }
}

- (void) setStringValue: (NSString *)aString
{
  [self setState: ([aString length] != 0)];
}

- (NSString *) stringValue
{
  return _cell.state ? @"1" : @"";
}

- (void) setAttributedStringValue: (NSAttributedString *)attrString
{
  [self setState: ([attrString length] != 0)];
}

- (NSAttributedString *) attributedStringValue
{
  return AUTORELEASE([[NSAttributedString alloc] 
                         initWithString: [self stringValue]]); 
}

/*
 * Displaying
 */
- (NSColor*) textColor
{
  if (_cell.is_disabled == YES)
    return [NSColor disabledControlTextColor];
  if ((_cell.state && (_showAltStateMask & NSChangeGrayCellMask))
      || (_cell.is_highlighted && (_highlightsByMask & NSChangeGrayCellMask)))
    return [NSColor selectedControlTextColor];
  return [NSColor controlTextColor];
}

- (NSColor *) backgroundColor
{
  return _backgroundColor;
}

- (void) setBackgroundColor: (NSColor *)color
{
  ASSIGN(_backgroundColor, color);

  if (_control_view)
    {
      if ([_control_view isKindOfClass: [NSControl class]])
        {
          [(NSControl*)_control_view updateCell: self];
        }
    }
}

- (GSThemeControlState) themeControlState
{
  unsigned mask;
  GSThemeControlState buttonState = GSThemeNormalState;

  // set the mask
  if (_cell.is_highlighted)
    {
      mask = _highlightsByMask;
      if (_cell.state)
        {
          mask &= ~_showAltStateMask;
        }
    }
  else if (_cell.state)
    mask = _showAltStateMask;
  else
    mask = NSNoCellMask;

  /* Determine the background color. 
     We draw when there is a border or when highlightsByMask
     is NSChangeBackgroundCellMask or NSChangeGrayCellMask,
     as required by our nextstep-like look and feel.  */
  if (mask & (NSChangeGrayCellMask | NSChangeBackgroundCellMask))
    {
      buttonState = GSThemeHighlightedState;
    }

  /* Pushed in buttons contents are displaced to the bottom right 1px.  */
  if (mask & NSPushInCellMask)
    {
      buttonState = GSThemeSelectedState;
    }

  if (_cell.is_disabled && buttonState != GSThemeHighlightedState)
    {
      buttonState = GSThemeDisabledState;
    }

  /* If we are first responder, change to the corresponding
     first responder state. Note that GSThemeDisabledState
     doesn't have a first responder variant, currently. */
  if (_cell.shows_first_responder
      && [[[self controlView] window] firstResponder] == [self controlView]
      && [self controlView] != nil)
    {
      if (buttonState == GSThemeSelectedState)
	buttonState = GSThemeSelectedFirstResponderState;
      else if (buttonState == GSThemeHighlightedState)
	buttonState = GSThemeHighlightedFirstResponderState;
      else if (buttonState == GSThemeNormalState)
	buttonState = GSThemeFirstResponderState;
    }

  return buttonState;
}

- (void) drawBezelWithFrame: (NSRect)cellFrame inView: (NSView *)controlView
{
  GSThemeControlState buttonState = [self themeControlState];

  [[GSTheme theme] drawButton: cellFrame 
                   in: self 
                   view: controlView
                   style: _bezel_style
                   state: buttonState];
}

- (void) drawImage: (NSImage*)imageToDisplay 
         withFrame: (NSRect)cellFrame 
            inView: (NSView*)controlView
{
  // Draw image
  if (imageToDisplay != nil)
    {
      NSPoint offset;
      NSRect rect;
      CGFloat fraction;
      NSSize size = [self _scaleImageWithSize: [imageToDisplay size]
				  toFitInSize: cellFrame.size
				  scalingType: _imageScaling];

      /* Calculate an offset from the cellFrame origin */
      offset = NSMakePoint((NSWidth(cellFrame) - size.width) / 2.0,
                           (NSHeight(cellFrame) - size.height) / 2.0);

      rect = NSMakeRect(cellFrame.origin.x + offset.x,
   		        cellFrame.origin.y + offset.y,
			size.width,
			size.height);

      /* Pixel-align */
      if (nil != controlView)
        {
          rect = [controlView centerScanRect: rect];
        }

      /* Draw the image */

      fraction = (![self isEnabled] &&
		  [self imageDimsWhenDisabled]) ? 0.5 : 1.0;
      
      [imageToDisplay drawInRect: rect
			fromRect: NSZeroRect
		       operation: NSCompositeSourceOver
			fraction: fraction
		  respectFlipped: YES
			   hints: nil];
    }
}

- (NSRect) drawTitle: (NSAttributedString*)titleToDisplay 
	   withFrame: (NSRect)cellFrame 
	      inView: (NSView*)controlView
{
  [self _drawAttributedText: titleToDisplay
		    inFrame: cellFrame];

  return [titleToDisplay
	   boundingRectWithSize: cellFrame.size
			options: NSStringDrawingUsesLineFragmentOrigin];   
}

// Private helper method overridden in subclasses
- (void) _drawBorderAndBackgroundWithFrame: (NSRect)cellFrame 
                                    inView: (NSView*)controlView
{
  /* The background color is used for borderless cells (the MacOS-X
   * documentation of the NSButtonCell -backgroundColor method says
   * it's only used for borderless cells).
   */
  if (!_cell.is_bordered)
    {
      NSColor	*c = [self backgroundColor];

      if (c != nil)
	{
	  [c set];
	  NSRectFill(cellFrame);
	}
    }

  // Draw gradient
  if (!_cell.is_highlighted)
    {
      [[GSTheme theme] drawGradientBorder: _gradient_type 
                       inRect: cellFrame 
                       withClip: NSZeroRect];
    }

  // The inside check could also be done via a track rect, but then this would
  // only work with specially prepared controls. Therefore we dont use 
  // _mouse_inside here.
  if ((_cell.is_bordered)
      && (!_shows_border_only_while_mouse_inside 
          || [controlView mouse: [[controlView window] mouseLocationOutsideOfEventStream] 
                          inRect: cellFrame]))
    {
      [self drawBezelWithFrame: cellFrame inView: controlView];
    }
}

- (void) drawInteriorWithFrame: (NSRect)cellFrame inView: (NSView*)controlView
{
  unsigned mask;
  NSImage *imageToDisplay;
  NSRect imageRect;
  NSAttributedString *titleToDisplay;
  NSRect titleRect;
  NSSize imageSize = {0, 0};
  NSSize titleSize = {0, 0};
  BOOL flippedView = [controlView isFlipped];
  NSCellImagePosition ipos = _cell.image_position;

  // transparent buttons never draw
  if (_buttoncell_is_transparent)
    return;

  _control_view = controlView;

  cellFrame = [self drawingRectForBounds: cellFrame];

  if (_cell.is_highlighted)
    {
      mask = _highlightsByMask;

      if (_cell.state)
        mask &= ~_showAltStateMask;
    }
  else if (_cell.state)
    mask = _showAltStateMask;
  else
    mask = NSNoCellMask;

  /*
   * Determine the image and the title that will be
   * displayed. If the NSContentsCellMask is set the
   * image and title are swapped only if state is 1 or
   * if highlighting is set (when a button is pushed it's
   * content is changed to the face of reversed state).
   */
  if (mask & NSContentsCellMask)
    {
      imageToDisplay = _altImage;
      if (!imageToDisplay)
        {
          imageToDisplay = _cell_image;
        }
      titleToDisplay = [self attributedAlternateTitle];
      if (titleToDisplay == nil || [titleToDisplay length] == 0)
        {
          titleToDisplay = [self attributedTitle];
        }
    }
  else
    {
      imageToDisplay = _cell_image;
      titleToDisplay = [self attributedTitle];
    }

  if (imageToDisplay && ipos != NSNoImage)
    {
      imageSize = [imageToDisplay size];
    }
  else
    {
      // When there is no image to display, ignore it in the calculations
      imageToDisplay = nil;
      ipos = NSNoImage;
    }

  if (titleToDisplay && ipos != NSImageOnly)
    {
      titleSize = [titleToDisplay size];
    }
  else
    {
      // When there is no text to display, ignore it in the calculations
      titleToDisplay = nil;
      ipos = NSImageOnly;
    }

  if (flippedView == YES)
    {
      if (ipos == NSImageAbove)
        {
          ipos = NSImageBelow;
        }
      else if (ipos == NSImageBelow)
        {
          ipos = NSImageAbove;
        }
    }
  
  /*
  The size calculations here should be changed very carefully, and _must_ be
  kept in sync with -cellSize. Changing the calculations to require more
  space isn't OK; this breaks interfaces designed using the old sizes by
  clipping away parts of the title.

  The current size calculations ensure that for bordered or bezeled cells,
  there's always at least a three point margin between the size returned by
  -cellSize and the minimum size required not to clip text. (In other words,
  the text can become three points wider (due to eg. font mismatches) before
  you lose the last character.)
  */
  switch (ipos)
    {
      default:
      case NSNoImage: 
        imageToDisplay = nil;
        titleRect = cellFrame;
         imageRect = NSZeroRect;
        if (titleSize.width + 6 <= titleRect.size.width)
          {
            titleRect.origin.x += 3;
            titleRect.size.width -= 6;
          }
        break;

      case NSImageOnly: 
        titleToDisplay = nil;
        imageRect = cellFrame;
        titleRect = NSZeroRect;
        break;

      case NSImageLeft: 
        imageRect.origin = cellFrame.origin;
        imageRect.size.width = imageSize.width;
        imageRect.size.height = cellFrame.size.height;
        if (_cell.is_bordered || _cell.is_bezeled) 
          {
            imageRect.origin.x += 3;
          }
        titleRect = imageRect;
        titleRect.origin.x += imageSize.width + GSCellTextImageXDist;
        titleRect.size.width = NSMaxX(cellFrame) - titleRect.origin.x;
        if (titleSize.width + 3 <= titleRect.size.width)
          {
            titleRect.size.width -= 3;
          }
        break;

      case NSImageRight: 
        imageRect.origin.x = NSMaxX(cellFrame) - imageSize.width;
        imageRect.origin.y = cellFrame.origin.y;
        imageRect.size.width = imageSize.width;
        imageRect.size.height = cellFrame.size.height;
        if (_cell.is_bordered || _cell.is_bezeled) 
          {
            imageRect.origin.x -= 3;
          }
        titleRect.origin = cellFrame.origin;
        titleRect.size.width = imageRect.origin.x - titleRect.origin.x
                               - GSCellTextImageXDist;
        titleRect.size.height = cellFrame.size.height;
        if (titleSize.width + 3 <= titleRect.size.width)
          {
            titleRect.origin.x += 3;
            titleRect.size.width -= 3;
          }
        break;

      case NSImageAbove: 
        /*
         * In this case, imageRect is all the space we can allocate
         * above the text. 
         * The drawing code below will then center the image in imageRect.
         */
        titleRect.origin = cellFrame.origin;
        titleRect.size.width = cellFrame.size.width;
        titleRect.size.height = titleSize.height;
        if (_cell.is_bordered || _cell.is_bezeled) 
          {
            titleRect.origin.y += 3;
          }

        imageRect.origin.x = cellFrame.origin.x;
        imageRect.origin.y = NSMaxY(titleRect) + GSCellTextImageYDist;
        imageRect.size.width = cellFrame.size.width;
        imageRect.size.height = NSMaxY(cellFrame) - imageRect.origin.y;

        if (_cell.is_bordered || _cell.is_bezeled) 
          {
            imageRect.size.height -= 3;
          }
        if (titleSize.width + 6 <= titleRect.size.width)
          {
            titleRect.origin.x += 3;
            titleRect.size.width -= 6;
          }
        break;

      case NSImageBelow: 
        /*
         * In this case, imageRect is all the space we can allocate
         * below the text. 
         * The drawing code below will then center the image in imageRect.
         */
        titleRect.origin.x = cellFrame.origin.x;
        titleRect.origin.y = NSMaxY(cellFrame) - titleSize.height;
        titleRect.size.width = cellFrame.size.width;
        titleRect.size.height = titleSize.height;
        if (_cell.is_bordered || _cell.is_bezeled)
          {
            titleRect.origin.y -= 3;
          }

        imageRect.origin.x = cellFrame.origin.x;
        imageRect.origin.y = cellFrame.origin.y;
        imageRect.size.width = cellFrame.size.width;
        imageRect.size.height
          = titleRect.origin.y - GSCellTextImageYDist - imageRect.origin.y;

        if (_cell.is_bordered || _cell.is_bezeled) 
          {
            imageRect.origin.y += 3;
            imageRect.size.height -= 3;
          }
        if (titleSize.width + 6 <= titleRect.size.width)
          {
            titleRect.origin.x += 3;
            titleRect.size.width -= 6;
          }
        break;

      case NSImageOverlaps: 
        imageRect = cellFrame;
        titleRect = cellFrame;
        if (titleSize.width + 6 <= titleRect.size.width)
          {
            titleRect.origin.x += 3;
            titleRect.size.width -= 6;
          }
        break;
    }

  // Draw image
  if (imageToDisplay != nil)
    {
      [self drawImage: imageToDisplay
            withFrame: imageRect
               inView: controlView];
    }

  // Draw title
  if (titleToDisplay != nil)
    {
      [self drawTitle: titleToDisplay withFrame: titleRect inView: controlView];
    }
}

- (NSSize) cellSize
{
  NSSize s;
  GSThemeMargins border;
  unsigned mask;
  NSImage *imageToDisplay;
  NSAttributedString *titleToDisplay;
  NSSize imageSize = NSZeroSize;
  NSSize titleSize = NSZeroSize;
  
  /* The size calculations here must be kept in sync with
  -drawInteriorWithFrame. */

  if (_cell.is_highlighted)
    {
      mask = _highlightsByMask;

      if (_cell.state)
        mask &= ~_showAltStateMask;
    }
  else if (_cell.state)
    mask = _showAltStateMask;
  else
    mask = NSNoCellMask;
  
  if (mask & NSContentsCellMask)
    {
      imageToDisplay = _altImage;
      if (!imageToDisplay)
        {
          imageToDisplay = _cell_image;
        }
      titleToDisplay = [self attributedAlternateTitle];
      if (titleToDisplay == nil || [titleToDisplay length] == 0)
        {
          titleToDisplay = [self attributedTitle];
        }
    }
  else
    {
      imageToDisplay = _cell_image;
      titleToDisplay = [self attributedTitle];
    }
  
  if (imageToDisplay)
    {
      imageSize = [imageToDisplay size];
    }

  if (titleToDisplay != nil)
    {
      titleSize = [titleToDisplay size];
    }
  
  switch (_cell.image_position)
    {
      default:
      case NSNoImage: 
        s = titleSize;
        break;
        
      case NSImageOnly: 
        s = imageSize;
        break;
        
      case NSImageLeft: 
      case NSImageRight: 
        s.width = imageSize.width + titleSize.width + GSCellTextImageXDist;
        s.height = MAX(imageSize.height, titleSize.height);
        break;
        
      case NSImageBelow: 
      case NSImageAbove: 
        s.width = MAX(imageSize.width, titleSize.width);
        s.height = imageSize.height + titleSize.height + GSCellTextImageYDist;
        break;
        
      case NSImageOverlaps: 
        s.width = MAX(imageSize.width, titleSize.width);
        s.height = MAX(imageSize.height, titleSize.height);
        break;
    }
  
  // Get border size
  if (_cell.is_bordered)
    {
      GSThemeControlState        buttonState = GSThemeNormalState;

      /* Determine the background color. 
         We draw when there is a border or when highlightsByMask
         is NSChangeBackgroundCellMask or NSChangeGrayCellMask,
         as required by our nextstep-like look and feel.  */
      if (mask & (NSChangeGrayCellMask | NSChangeBackgroundCellMask))
        {
          buttonState = GSThemeHighlightedState;
        }
      
      /* Pushed in buttons contents are displaced to the bottom right 1px.  */
      if (mask & NSPushInCellMask)
        {
          buttonState = GSThemeSelectedState;
        }

      border = [[GSTheme theme] buttonMarginsForCell: self
					       style: _bezel_style 
					       state: buttonState];
    }
  else
    {
      border.left = 0;
      border.top = 0;
      border.right = 0;
      border.bottom = 0;
    }
      
  /* Add an additional 6 pixels horizontally so that the text is not
   * too near the boundaries of the button.  Without them, autosized
   * buttons look too tiny and crammed.  This might be made
   * configurable by the theme, but most likely only because themes
   * might want to have even more space here (to make buttons more
   * clear and readable) rather than less!  Eg. Apple by default has
   * huge amounts of empty space between the text and the borders of
   * their push buttons.
   */
  if ((_cell.is_bordered && (_cell.image_position != NSImageOnly))
      || _cell.is_bezeled)
    {
      border.left += 6;
      border.right += 6;
    }

  // Add border size
  s.width += border.left + border.right;
  s.height += border.top + border.bottom;

  return s;
}

- (NSRect) drawingRectForBounds: (NSRect)theRect
{
  if (_cell.is_bordered)
    {
      GSThemeMargins border;
      unsigned mask;
      GSThemeControlState buttonState = GSThemeNormalState;
      NSRect interiorFrame;

      if (_cell.is_highlighted)
        {
          mask = _highlightsByMask;

          if (_cell.state)
            mask &= ~_showAltStateMask;
        }
      else if (_cell.state)
        mask = _showAltStateMask;
      else
        mask = NSNoCellMask;
  
      /* Determine the background color. 
         We draw when there is a border or when highlightsByMask
         is NSChangeBackgroundCellMask or NSChangeGrayCellMask,
         as required by our nextstep-like look and feel.  */
      if (mask & (NSChangeGrayCellMask | NSChangeBackgroundCellMask))
        {
          buttonState = GSThemeHighlightedState;
        }
      
      if (mask & NSPushInCellMask)
        {
          buttonState = GSThemeSelectedState;
        }

      border = [[GSTheme theme] buttonMarginsForCell: self
					       style: _bezel_style 
					       state: buttonState];

      interiorFrame = theRect;
      interiorFrame.origin.x += border.left;
      interiorFrame.size.width -= border.left + border.right;
      interiorFrame.origin.y += ([_control_view isFlipped] ? 
				 border.top : border.bottom);
      interiorFrame.size.height -= border.bottom + border.top;

      /* Pushed in buttons contents are displaced to the bottom right 1px.  */
      if (mask & NSPushInCellMask)
        {
          interiorFrame = NSOffsetRect(interiorFrame, 1.0,
	    [_control_view isFlipped] ? 1.0 : -1.0);
        }
      return interiorFrame;
    }
  else
    {
      return theRect;
    }
}

- (void) setSound: (NSSound *)aSound
{
  ASSIGN(_sound, aSound);
}

- (NSSound *) sound
{
  return _sound;
}

- (void) mouseEntered: (NSEvent *)event
{
  _mouse_inside = YES;
  [(NSView *)[event userData] setNeedsDisplay: YES];
}

- (void) mouseExited: (NSEvent *)event
{
  _mouse_inside = NO;
  [(NSView *)[event userData] setNeedsDisplay: YES];
}

/**Simulates a single mouse click on the button cell. This method overrides the
  cell method performClickWithFrame:inView: to add the possibility to
  play a sound associated with the click. 
 */
- (void) performClickWithFrame: (NSRect)cellFrame inView: (NSView *)controlView
{
  if (_sound != nil)
    {
      [_sound play];
    }

  [super performClickWithFrame: cellFrame inView: controlView];
}

/*
 * Comparing to Another NSButtonCell
 */
- (NSComparisonResult) compare: (id)otherCell
{
  if ([otherCell isKindOfClass: [NSButtonCell class]] == NO)
    {
      [NSException raise: NSBadComparisonException
                   format: @"NSButtonCell comparison with non-NSButtonCell"];
    }
  return [super compare: otherCell];
}

/*
 * NSCopying protocol
 */
- (id) copyWithZone: (NSZone*)zone
{
  NSButtonCell *c = [super copyWithZone: zone];
  
  c->_altContents = [_altContents copyWithZone: zone];
  _altImage = TEST_RETAIN(_altImage);
  _keyEquivalent = TEST_RETAIN(_keyEquivalent);
  _keyEquivalentFont = TEST_RETAIN(_keyEquivalentFont);
  _sound = TEST_RETAIN(_sound);
  _backgroundColor = TEST_RETAIN(_backgroundColor);

  return c;
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      GSButtonCellFlags buttonCellFlags;
      unsigned int bFlags = 0;
      unsigned int bFlags2 = 0;
      NSImage *image = [self image];
      NSButtonImageSource *bi = nil;

      if ([[self keyEquivalent] length] > 0)
        {
          [aCoder encodeObject: [self keyEquivalent] forKey: @"NSKeyEquivalent"];
        }
      if ([self image] != nil)
        {
          [aCoder encodeObject: [self image] forKey: @"NSNormalImage"];
        }
      if ([[self alternateTitle] length] > 0)
        {
          [aCoder encodeObject: [self alternateTitle] forKey: @"NSAlternateContents"];
        }

      buttonCellFlags.useButtonImageSource = (([NSImage imageNamed: @"NSSwitch"] == image) ||
                                              ([NSImage imageNamed: @"NSRadioButton"] == image));
      buttonCellFlags.isTransparent = [self isTransparent];
      buttonCellFlags.isBordered = [self isBordered]; 
      buttonCellFlags.imageDoesOverlap = 
          (_cell.image_position == NSImageOverlaps) 
          || (_cell.image_position == NSImageOnly);
      buttonCellFlags.isHorizontal = (_cell.image_position == NSImageLeft) 
          || (_cell.image_position == NSImageRight);
      buttonCellFlags.isBottomOrLeft = (_cell.image_position == NSImageLeft) 
          || (_cell.image_position == NSImageBelow);
      buttonCellFlags.isImageAndText = (image != nil) 
          && (_cell.image_position != NSImageOnly);
      buttonCellFlags.hasKeyEquiv = ([self keyEquivalent] != nil);

      // cell attributes...
      buttonCellFlags.isPushin = [self cellAttribute: NSPushInCell]; 
      buttonCellFlags.highlightByBackground = [self cellAttribute: NSCellLightsByBackground];
      buttonCellFlags.highlightByContents = [self cellAttribute: NSCellLightsByContents];
      buttonCellFlags.highlightByGray = [self cellAttribute: NSCellLightsByGray];
      buttonCellFlags.changeBackground = [self cellAttribute: NSChangeBackgroundCell];
      buttonCellFlags.changeContents = [self cellAttribute: NSCellChangesContents];
      buttonCellFlags.changeGray = [self cellAttribute: NSChangeGrayCell];

      // set these to zero...
      buttonCellFlags.inset = 0;
      buttonCellFlags.doesNotDimImage = 0;
      buttonCellFlags.gradient = 0;
      buttonCellFlags.unused2 = 0;
      buttonCellFlags.lastState = 0;
      buttonCellFlags.isImageSizeDiff = 0;
      buttonCellFlags.drawing = 0;

      memcpy((void *)&bFlags, (void *)&buttonCellFlags,sizeof(unsigned int));
      [aCoder encodeInt: bFlags forKey: @"NSButtonFlags"];

      // style and border.
      bFlags2 |= [self showsBorderOnlyWhileMouseInside] ? 0x8 : 0;
      bFlags2 |= (([self bezelStyle] & 0x7) | (([self bezelStyle] & 0x18) << 2));
      bFlags2 |= [self keyEquivalentModifierMask] << 8;

      switch ([self imageScaling])
	{
	case NSImageScaleProportionallyDown:
	  bFlags2 |= (2 << 6);
	  break;
	case NSImageScaleAxesIndependently:
	  bFlags2 |= (3 << 6);
	  break;
	case NSImageScaleNone:
	default:
	  break;
	case NSImageScaleProportionallyUpOrDown:
	  bFlags2 |= (1 << 6);
	  break;
	}

      [aCoder encodeInt: bFlags2 forKey: @"NSButtonFlags2"];

      // alternate image encoding...
      if (image != nil)
        {
          if ([image isKindOfClass: [NSImage class]] && buttonCellFlags.useButtonImageSource)
            {
              if ([NSImage imageNamed: @"NSSwitch"] == image)
                {
                  bi = [[NSButtonImageSource alloc] initWithImageNamed: @"NSHighlightedSwitch"];
                }
              else if ([NSImage imageNamed: @"NSRadioButton"] == image)
                {
                  bi = [[NSButtonImageSource alloc] initWithImageNamed: @"NSHighlightedRadioButton"];
                }
            }
        }

      // encode button image source, if it exists...
      if (bi != nil)
        {
          [aCoder encodeObject: bi forKey: @"NSAlternateImage"];
          RELEASE(bi);
        }
      else if (_altImage != nil)
        {
          [aCoder encodeObject: _altImage forKey: @"NSAlternateImage"];
        }

      // repeat and delay
      [aCoder encodeInt: (int)_delayInterval forKey: @"NSPeriodicDelay"];
      [aCoder encodeInt: (int)_repeatInterval forKey: @"NSPeriodicInterval"];
    }
  else
    {
      BOOL tmp;
      NSUInteger tmp2;

      [aCoder encodeObject: _keyEquivalent];
      [aCoder encodeObject: _keyEquivalentFont];
      [aCoder encodeObject: _altContents];
      [aCoder encodeObject: _altImage];
      tmp = _buttoncell_is_transparent;
      [aCoder encodeValueOfObjCType: @encode(BOOL)
              at: &tmp];

      encode_NSUInteger(aCoder, &_keyEquivalentModifierMask);
      tmp2 = _highlightsByMask;
      encode_NSUInteger(aCoder, &tmp2);
      tmp2 = _showAltStateMask;
      encode_NSUInteger(aCoder, &tmp2);

      [aCoder encodeObject: _sound];
      [aCoder encodeObject: _backgroundColor];
      [aCoder encodeValueOfObjCType: @encode(float)
                                 at: &_delayInterval];
      [aCoder encodeValueOfObjCType: @encode(float)
                                 at: &_repeatInterval];
      tmp2 = _bezel_style;
      encode_NSUInteger(aCoder, &tmp2);
      tmp2 = _gradient_type;
      encode_NSUInteger(aCoder, &tmp2);
      tmp = _image_dims_when_disabled;
      [aCoder encodeValueOfObjCType: @encode(BOOL)
                                 at: &tmp];
      tmp = _shows_border_only_while_mouse_inside;
      [aCoder encodeValueOfObjCType: @encode(BOOL)
                                 at: &tmp];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (!self)
    return nil;

  if ([aDecoder allowsKeyedCoding])
    {
      int delay = 0;
      int interval = 0;      
      // NSControl *control = [aDecoder decodeObjectForKey: @"NSControlView"];

      if ([aDecoder containsValueForKey: @"NSKeyEquivalent"])
        {
          [self setKeyEquivalent: [aDecoder decodeObjectForKey: @"NSKeyEquivalent"]];
        }
      if ([aDecoder containsValueForKey: @"NSNormalImage"])
        {
          [self setImage: [aDecoder decodeObjectForKey: @"NSNormalImage"]];
        }
      if ([aDecoder containsValueForKey: @"NSAlternateContents"])
        {
          [self setAlternateTitle: [aDecoder decodeObjectForKey: @"NSAlternateContents"]];
        }
      if ([aDecoder containsValueForKey: @"NSButtonFlags"])
        {
          unsigned int bFlags = [aDecoder decodeIntForKey: @"NSButtonFlags"];
          GSButtonCellFlags buttonCellFlags;
          memcpy((void *)&buttonCellFlags,(void *)&bFlags,sizeof(struct _GSButtonCellFlags));

          [self setTransparent: buttonCellFlags.isTransparent];
          [self setBordered: buttonCellFlags.isBordered];
          
          [self setCellAttribute: NSPushInCell
                to: buttonCellFlags.isPushin];
          [self setCellAttribute: NSCellLightsByBackground 
                to: buttonCellFlags.highlightByBackground];
          [self setCellAttribute: NSCellLightsByContents   
                to: buttonCellFlags.highlightByContents];
          [self setCellAttribute: NSCellLightsByGray
                to: buttonCellFlags.highlightByGray]; 
          [self setCellAttribute: NSChangeBackgroundCell   
                to: buttonCellFlags.changeBackground];
          [self setCellAttribute: NSCellChangesContents    
                to: buttonCellFlags.changeContents];
          [self setCellAttribute: NSChangeGrayCell
                to: buttonCellFlags.changeGray]; 
          
          if (buttonCellFlags.imageDoesOverlap)
            if (buttonCellFlags.isImageAndText)
              [self setImagePosition: NSImageOverlaps];
            else
              [self setImagePosition: NSImageOnly];
          else if (buttonCellFlags.isImageAndText)
            if (buttonCellFlags.isHorizontal)
              if (buttonCellFlags.isBottomOrLeft)
                [self setImagePosition: NSImageLeft];
              else
                [self setImagePosition: NSImageRight];
            else
              if (buttonCellFlags.isBottomOrLeft)
                [self setImagePosition: NSImageBelow];
              else
                [self setImagePosition: NSImageAbove];
          else
            [self setImagePosition: NSNoImage];
        }
      if ([aDecoder containsValueForKey: @"NSButtonFlags2"])
        {
	  NSUInteger imageScale;
          int bFlags2;

          bFlags2 = [aDecoder decodeIntForKey: @"NSButtonFlags2"];
          [self setShowsBorderOnlyWhileMouseInside: (bFlags2 & 0x8)];
          [self setBezelStyle: (bFlags2 & 0x7) | ((bFlags2 & 0x20) >> 2)];
          [self setKeyEquivalentModifierMask: ((bFlags2 >> 8) & 
                                               NSDeviceIndependentModifierFlagsMask)];

	  switch ((bFlags2 >> 6) & 3)
	    {
	    case 2:
	      imageScale = NSImageScaleProportionallyDown;
	      break;
	    case 3:
	      imageScale = NSImageScaleAxesIndependently;
	      break;
	    case 0:
	    default:
	      imageScale = NSImageScaleNone;
	      break;
	    case 1:
	      imageScale = NSImageScaleProportionallyUpOrDown;
	      break;
	    }
	  [self setImageScaling: imageScale];
        }
      if ([aDecoder containsValueForKey: @"NSAlternateImage"])
        {
          id image;

          //
          // NOTE: Okay... this is a humongous kludge.   It seems as though
          // Cocoa is doing something very odd here.  It doesn't seem to 
          // encode system images for buttons normally, if it is using 
          // images at all. Until I figure out what, this will stay.  
          // Danger, Will Robinson! :)
          //
          image = [aDecoder decodeObjectForKey: @"NSAlternateImage"];
          if ([image isKindOfClass: [NSImage class]])
            {
              if ([NSImage imageNamed: @"NSSwitch"] == image)
                {
                  image = [NSImage imageNamed: @"NSHighlightedSwitch"];
                  if ([self image] == nil)
                    {
                      [self setImage: [NSImage imageNamed: @"NSSwitch"]];
                    }                    
                }
              else if ([NSImage imageNamed: @"NSRadioButton"] == image)
                {
                  image = [NSImage imageNamed: @"NSHighlightedRadioButton"];
                  if ([self image] == nil)
                    {
                      [self setImage: [NSImage imageNamed: @"NSRadioButton"]];
                    }                    
                }
              
              [self setAlternateImage: image];
            }
        }
      if ([aDecoder containsValueForKey: @"NSPeriodicDelay"])
        {
          delay = [aDecoder decodeIntForKey: @"NSPeriodicDelay"];
        }
      if ([aDecoder containsValueForKey: @"NSPeriodicInterval"])
        {
          interval = [aDecoder decodeIntForKey: @"NSPeriodicInterval"];
        }
      [self setPeriodicDelay: delay interval: interval];
    }
  else
    {
      BOOL tmp;
      NSUInteger tmp2;
      int version = [aDecoder versionForClassName: @"NSButtonCell"];
      NSString *key = nil;

      [aDecoder decodeValueOfObjCType: @encode(id) at: &key];
      // Hack to correct a Gorm problem, there "\n" is used instead of "\r".
      if ([key isEqualToString: @"\n" ])
        {
          key = @"\r";
        }
      [self setKeyEquivalent: key];

      [aDecoder decodeValueOfObjCType: @encode(id) at: &_keyEquivalentFont];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_altContents];
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_altImage];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &tmp];
      _buttoncell_is_transparent = tmp;
      decode_NSUInteger(aDecoder, &_keyEquivalentModifierMask);
      if (version <= 2)
        {
          _keyEquivalentModifierMask = _keyEquivalentModifierMask << 16;
        }
      decode_NSUInteger(aDecoder, &tmp2);
      _highlightsByMask = (NSInteger)tmp2;
      decode_NSUInteger(aDecoder, &tmp2);
      _showAltStateMask = (NSInteger)tmp2;

      if (version >= 2)
        {
          [aDecoder decodeValueOfObjCType: @encode(id) at: &_sound];
          [aDecoder decodeValueOfObjCType: @encode(id) at: &_backgroundColor];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_delayInterval];
          [aDecoder decodeValueOfObjCType: @encode(float) at: &_repeatInterval];
          decode_NSUInteger(aDecoder, &tmp2);
          _bezel_style = (NSBezelStyle)tmp2;
          decode_NSUInteger(aDecoder, &tmp2);
          _gradient_type = (NSGradientType)tmp2;
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &tmp];
          _image_dims_when_disabled = tmp;
          [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &tmp];
          _shows_border_only_while_mouse_inside = tmp;
        }
      // Not encoded in non-keyed archive
      _imageScaling = NSImageScaleNone;
    }

  return self;
}

@end
