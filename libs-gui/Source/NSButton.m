/** <title>NSButton</title>

   <abstract>The button class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
	    Ovidiu Predescu <ovidiu@net-community.com>
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

#include "config.h"

#import <Foundation/NSCharacterSet.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSButtonCell.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSWindow.h"

//
// class variables
//
static id buttonCellClass = nil;

/**
   TODO Description
*/
@implementation NSButton

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSButton class])
    {
      [self setVersion: 1];
      [self setCellClass: [NSButtonCell class]];
    }
}

//
// Initializing the NSButton Factory
//
/** Returns 
   
 */
+ (Class) cellClass
{
  return buttonCellClass;
}

+ (void) setCellClass: (Class)classId
{
  buttonCellClass = classId;
}

//
// Instance methods
//

- (BOOL) acceptsFirstMouse: (NSEvent *)theEvent
{
  return YES;
}

- (BOOL) isFlipped
{
  return YES;
}

/** <p>Sets the NSButtonCell's type to <var>aType</var> and marks self for
    display.See <ref type="type" id="NSButtonType">NSButtonType</ref> for more
    information.</p><p>See Also: [NSButtonCell-setButtonType:]</p>
 */
- (void) setButtonType: (NSButtonType)aType
{
  [_cell setButtonType: aType];
  [self setNeedsDisplay: YES];
}

- (void)setHighlightsBy:(NSInteger)aType
{
  [_cell setHighlightsBy:aType];
}

- (void)setShowsStateBy:(NSInteger)aType
{
  [_cell setShowsStateBy:aType];
}

//
// Setting the State
//
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

/**<p>Sets the NSButtonCell's state to <var>value</var> and marks
   self for display.</p><p>See Also: -state [NSButtonCell-setState:]</p> 
 */
- (void) setState: (NSInteger)value
{
  [_cell setState: value];
  [self setNeedsDisplay: YES];
}

/** <p>Returns the NSButtonCell's state.</p>
   <p>See Also: -setState: [NSButtonCell-state]</p> 
*/
- (NSInteger) state
{
  return [_cell state];
}

- (BOOL) allowsMixedState
{
  return [_cell allowsMixedState];
}

- (void) setAllowsMixedState: (BOOL)flag
{
  [_cell setAllowsMixedState: flag];
}

/**<p>Sets the NSButtonCell to the next state and marks self for display.</p>
   <p>See Also: [NSButtonCell-setNextState]</p>
 */
- (void)setNextState
{
  [_cell setNextState];
  [self setNeedsDisplay: YES];
}

/** <p>Gets the NSButtonCell's <var>delay</var> and the <var>interval</var>
    parameters used when NSButton sends continouly action messages.
    By default <var>delay</var> is 0.4 and <var>interval</var> is 0.075.</p>
    <p>See Also: [NSButtonCell-getPeriodicDelay:interval:] 
    -setPeriodicDelay:interval:
    [NSCell-trackMouse:inRect:ofView:untilMouseUp:]</p>
 */
- (void) getPeriodicDelay: (float *)delay
		interval: (float *)interval
{
  [_cell getPeriodicDelay: delay interval: interval];
}

/** <p>Sets the NSButtonCell's  <var>delay</var> and <var>interval</var> 
    parameters used when NSButton sends continouly action messages.
    By default <var>delay</var> is 0.4 and <var>interval</var> is 0.075.</p>
    <p>See Also: [NSButtonCell-setPeriodicDelay:interval:] 
    -getPeriodicDelay:interval: 
    [NSCell-trackMouse:inRect:ofView:untilMouseUp:]</p>
 */
- (void) setPeriodicDelay: (float)delay
		interval: (float)interval
{
  [_cell setPeriodicDelay: delay interval: interval];
}

/**<p>Returns the NSButtonCell's alternate title.</p>
   <p>See Also: -setAlternateTitle: [NSButtonCell-alternateTitle]</p>
 */
- (NSString *) alternateTitle
{
  return [_cell alternateTitle];
}

/**<p>Sets the NSButtonCell's alternateTitle to <var>aString</var> and marks
   self for display.</p><p>See Also: -alternateTitle</p>
 */
- (void) setAlternateTitle: (NSString *)aString
{
  [_cell setAlternateTitle: aString];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the NSButtonCell's title to <var>aString</var> and marks self for
   display.</p><p>See Also: -title</p>
 */
- (void) setTitle: (NSString *)aString
{
  [_cell setTitle: aString];
  [self setNeedsDisplay: YES];
}

/** <p>Returns the NSButtonCell's title.</p>
    <p>See Also: [NSButtonCell-title] -setTitle:</p>
 */
- (NSString *) title
{
  return [_cell title];
}

- (NSAttributedString *) attributedAlternateTitle
{
  return [_cell attributedAlternateTitle];
}

- (NSAttributedString *) attributedTitle
{
  return [_cell attributedTitle];
}

- (void) setAttributedAlternateTitle: (NSAttributedString *)aString
{
  [_cell setAttributedAlternateTitle: aString];
  [self setNeedsDisplay: YES];
}

- (void) setAttributedTitle: (NSAttributedString *)aString
{
  [_cell setAttributedTitle: aString];
  [self setNeedsDisplay: YES];
}

- (void) setTitleWithMnemonic: (NSString *)aString
{
  [_cell setTitleWithMnemonic: aString];
  [self setNeedsDisplay: YES];
}

//
// Setting the Images
//
/** <p>Returns the NSButtonCell's alternate image.</p>
    <p>See Also: -setAlternateImage: [NSButtonCell-alternateImage]</p> 
*/
- (NSImage *) alternateImage
{
  return [_cell alternateImage];
}

/** <p>Returns the NSButtonCell's image.</p>
    <p>See Also: -setImage: [NSButtonCell-image]</p> 
*/
- (NSImage *) image
{
  return [_cell image];
}

/** <p>Returns the position of the NSButtonCell's image. See 
    <ref type="type" id="NSCellImagePosition">NSCellImagePosition</ref>
    for more information.</p>
    <p>See Also: -setImagePosition: [NSButtonCell-imagePosition]</p> 
*/
- (NSCellImagePosition) imagePosition
{
  return [_cell imagePosition];
}

/**<p>Sets the NSButtonCell's alternate image to <var>anImage</var> and marks
   self for display.</p>
   <p>See Also: -alternateImage [NSButtonCell-setAlternateImage:]</p>
*/
- (void) setAlternateImage: (NSImage *)anImage
{
  [_cell setAlternateImage: anImage];
  [self setNeedsDisplay: YES];
}

/** <p>Sets the NSButtonCell's image to <var>anImage</var> and marks self
    for display.</p><p>See Also: -image [NSButtonCell-setImage:]</p>
*/
- (void) setImage: (NSImage *)anImage
{
  [_cell setImage: anImage];
  [self setNeedsDisplay: YES];
}

/** <p>Sets the postion of the NSButtonCell's image to <var>aPosition</var>
    and marks self for display. See <ref type="type" id="NSCellImagePosition">
    NSCellImagePosition</ref>for more information.</p>
    <p>See Also: -imagePosition [NSButtonCell-setImagePosition:] </p>
*/
- (void) setImagePosition: (NSCellImagePosition)aPosition
{
  [_cell setImagePosition: aPosition];
  [self setNeedsDisplay: YES];
}

//
// Modifying Graphic Attributes
//
/** <p>Returns whether the NSButton's cell has border.</p>
    <p>See Also: -setBordered: [NSButtonCell-isBordered]</p>
*/
- (BOOL) isBordered
{
  return [_cell isBordered];
}

/** <p>Returns whether the NSButton's cell is transparent.</p>
    <p>See Also: -setTransparent: [NSButtonCell-isTransparent]</p>
*/
- (BOOL) isTransparent
{
  return [_cell isTransparent];
}

/** <p>Sets whether the NSButton's cell has border and marks self for 
    display.</p><p>See Also: -isBordered [NSButtonCell-setBordered:]</p>
*/
- (void) setBordered: (BOOL)flag
{
  [_cell setBordered: flag];
  [self setNeedsDisplay: YES];
}

/** <p>Sets whether the NSButton's cell is transparent and marks self for 
    display.</p><p>See Also: -isTransparent [NSButtonCell-setTransparent:]</p>
*/
- (void) setTransparent: (BOOL)flag
{
  [_cell setTransparent: flag];
  [self setNeedsDisplay: YES];
}

/** <p>Returns the style of the NSButtonCell's bezeled border. 
    See <ref type="type" id="NSBezelStyle">NSBezelStyle</ref> for more
    information.</p><p>See Also: -setBezelStyle: [NSButtonCell-bezelStyle]</p>
*/
- (NSBezelStyle)bezelStyle
{
  return [_cell bezelStyle];
}

/**<p>Sets the style of the NSButtonCell's bezeled border and marks self for
   display. See <ref type="type" id="NSBezelStyle">NSBezelStyle</ref>
   for more information.</p>
   <p>See Also: -bezelStyle [NSButtonCell-setBezelStyle:]</p>
*/
- (void)setBezelStyle:(NSBezelStyle)bezelStyle
{
  [_cell setBezelStyle: bezelStyle];
  [self setNeedsDisplay: YES];
}

- (BOOL)showsBorderOnlyWhileMouseInside
{
  return [_cell showsBorderOnlyWhileMouseInside];
}

- (void)setShowsBorderOnlyWhileMouseInside:(BOOL)show
{
  [_cell setShowsBorderOnlyWhileMouseInside: show];
  [self setNeedsDisplay: YES];
}

//
// Displaying
//
/** <p>(Un)Highlights the NSButtonCell.</p>
    <p>See Also: [NSButtonCell-highlight:withFrame:inView:]</p>
 */
- (void) highlight: (BOOL)flag
{
  [_cell highlight: flag withFrame: _bounds inView: self];
}

//
// Setting the Key Equivalent
//
/**<p>Returns the NSButtonCell's key equivalent. The key equivalent and its
   modifier mask are used to simulate the click of the button in
   -performKeyEquivalent:. Returns an empty string if no key equivalent is
   defined. By default NSButton hasn't key equivalent.</p>
   <p>See Also: -setKeyEquivalent: -performKeyEquivalent: 
   -keyEquivalentModifierMask [NSButtonCell-keyEquivalent]</p>
 */
- (NSString*) keyEquivalent
{
  return [_cell keyEquivalent];
}

/** <p>Returns the modifier mask of the NSButtonCell's key equivalent. 
    The key equivalent and its modifier mask are used to simulate the click
    of the button in  -performKeyEquivalent: . The default mask is 
    NSCommandKeyMask.</p><p>See Also: -setKeyEquivalentModifierMask:
    -keyEquivalent [NSButtonCell-setKeyEquivalentModifierMask:]</p>
 */
- (NSUInteger) keyEquivalentModifierMask
{
  return [_cell keyEquivalentModifierMask];
}

/** <p>Sets the NSButtonCell's key equivalent to <var>aKeyEquivalent</var>.
    The key equivalent and its modifier mask are used to simulate the click
    of the button in  -performKeyEquivalent:. By default NSButton hasn't 
    key equivalent.</p><p>See Also: -keyEquivalent 
    -setKeyEquivalentModifierMask: -performKeyEquivalent: 
    [NSButtonCell-setKeyEquivalent:]</p>
*/
- (void) setKeyEquivalent: (NSString*)aKeyEquivalent
{
  [_cell setKeyEquivalent: aKeyEquivalent];
}

/** <p>Sets the modifier mask of the NSButtonCell's key equivalent to
    <var>mask</var>. The key equivalent and its modifier mask are used to
    simulate the click of the button in  -performKeyEquivalent:. By default
    the mask is NSCommandKeyMask.</p><p>See Also: -keyEquivalentModifierMask  
    -setKeyEquivalent: -performKeyEquivalent:
    [NSButtonCell-setKeyEquivalentModifierMask:]</p>
*/
- (void) setKeyEquivalentModifierMask: (NSUInteger)mask
{
  [_cell setKeyEquivalentModifierMask: mask];
}

//
// Determining the first responder
//
- (BOOL) becomeFirstResponder
{
  [_window disableKeyEquivalentForDefaultButtonCell];
  [_cell setShowsFirstResponder: YES];
  [self setNeedsDisplay: YES];

  return YES;
}

- (BOOL) resignFirstResponder
{
  [_window enableKeyEquivalentForDefaultButtonCell];
  [_cell setShowsFirstResponder: NO];
  [self setNeedsDisplay: YES];

  return YES;
}

- (void) becomeKeyWindow
{
  [_cell setShowsFirstResponder: YES];
  [self setNeedsDisplay: YES];
}

- (void) resignKeyWindow
{
  [_cell setShowsFirstResponder: NO];
  [self setNeedsDisplay: YES];
}

- (void) keyDown: (NSEvent*)theEvent
{
  if ([self isEnabled])
    {
      NSString *characters = [theEvent characters];

      /* Handle SPACE to perform a click */
      if ([characters isEqualToString: @" "])
        {
          [self performClick: self];
          return;
        }
    }
  
  [super keyDown: theEvent];
}

//
// Handling Events and Action Messages
//
/**<p>Simulates the click of the button if the key equivalent and 
   its modifier mask match the event <var>anEvent</var>.
   By default the key modifier mask is NSCommandKeyMask and there is no
   key equivalent</p>
   <p>See Also: -keyEquivalent -keyEquivalentModifierMask 
   [NSControl-performClick:]
   [NSEvent-charactersIgnoringModifiers] [NSEvent-modifierFlags]</p>

  Does nothing and returns NO if the receiver is disabled or if it is
  blocked by a modal window being run.
  
 */
- (BOOL) performKeyEquivalent: (NSEvent *)anEvent
{
  NSWindow *w = [self window];
  NSWindow *mw = [NSApp modalWindow];

  if ([self isEnabled] && (mw == nil || [w worksWhenModal] || mw == w))
    {
      NSString	*keyEquivalent = [self keyEquivalent];

      if ([keyEquivalent length] > 0 && [keyEquivalent isEqualToString: [anEvent charactersIgnoringModifiers]])
        {
          NSUInteger	mask = [self keyEquivalentModifierMask];
          NSUInteger modifiers = [anEvent modifierFlags];
          NSUInteger relevantModifiersMask = NSCommandKeyMask | NSAlternateKeyMask | NSControlKeyMask;
          /* Take shift key into account only for control keys and arrow and function keys */
          if ((modifiers & NSFunctionKeyMask)
              || [[NSCharacterSet controlCharacterSet] characterIsMember:[keyEquivalent characterAtIndex:0]])
            relevantModifiersMask |= NSShiftKeyMask;
          
          if ((modifiers & relevantModifiersMask) == (mask & relevantModifiersMask))
            {
              [self performClick: self];
              return YES;
            }
        }
    }
  return NO;
}

- (void) setSound: (NSSound *)aSound
{
  [_cell setSound: aSound];
}

- (NSSound *) sound
{
  return [_cell sound];
}

@end
