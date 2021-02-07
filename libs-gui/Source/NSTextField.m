/** <title>NSTextField</title>

   <abstract>Text field control class for text entry</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: August 1998
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

#import <Foundation/NSDictionary.h>
#import <Foundation/NSFormatter.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>

#import "AppKit/NSApplication.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"

static NSNotificationCenter *nc;

/*
 * Class variables
 */
static Class usedCellClass;
static Class textFieldCellClass;

@implementation NSTextField
//
// Class methods
//
+ (void) initialize
{
  if (self == [NSTextField class])
    {
      [self setVersion: 1];
      textFieldCellClass = [NSTextFieldCell class];
      usedCellClass = textFieldCellClass;
      nc = [NSNotificationCenter defaultCenter];

      [self exposeBinding: NSEditableBinding];
      [self exposeBinding: NSTextColorBinding];
    }
}

/*
 * Setting the Cell class
 */
+ (Class) cellClass
{
  return usedCellClass;
}

+ (void) setCellClass: (Class)factoryId
{
  usedCellClass = factoryId ? factoryId : textFieldCellClass;
}

//
// Instance methods
//
- (id) initWithFrame: (NSRect)frameRect
{
  self = [super initWithFrame: frameRect];
  if (self == nil)
    return self;

  [_cell setBezeled: YES];
  [_cell setSelectable: YES];
  [_cell setEditable: YES];
  [_cell setDrawsBackground: YES];
  _text_object = nil;

  return self;
}

- (void) dealloc
{
  if (_delegate != nil)
    {
      [self setDelegate: nil];
    }

  [super dealloc];
}

- (BOOL) isFlipped
{
  return YES;
}

/** <p>Returns whether the NSTextField is editable. By default a NSTextField
    is not editable.</p>
    <p>See Also: -setEditable: [NSCell-isEditable]</p>
*/
- (BOOL) isEditable
{
  return [_cell isEditable];
}

/** <p>Returns whether the NSTextField is selectable.</p>
    <p>See Also: -setSelectable: [NSCell-isSelectable]</p>
*/
- (BOOL) isSelectable
{
  return [_cell isSelectable];
}

/** <p>Sets whether the NSTextField's cell and the NSText object is editable.
    By default a NSTextField is not editable.</p>
    <p>See Also: -isEditable [NSCell-setEditable:] [NSText-setEditable:]</p>
*/
- (void) setEditable: (BOOL)flag
{
  [_cell setEditable: flag];
  if (_text_object)
    [_text_object setEditable: flag];
}

/** <p>Sets whether the NSTextField's cell and the NSText object
    is selectable.</p><p>See Also: -isSelectable 
    [NSTextFieldCell-setSelectable:] [NSText-setSelectable:]</p>
*/
- (void) setSelectable: (BOOL)flag
{
  [_cell setSelectable: flag];
  if (_text_object)
    [_text_object setSelectable: flag];
}

/**<p>Selects all the text of the NSTextField if it's selectable.</p>
 */
- (void) selectText: (id)sender
{
  if ([self isSelectable] && (_super_view != nil))
    {
      if (_text_object)
        [_text_object selectAll: self];
      else
        {
          NSText *text = [_window fieldEditor: YES  forObject: self];
          int length;

          if ([text superview] != nil)
            if ([text resignFirstResponder] == NO)
              return;
          
          //  [NSCursor hide];
          /* [self stringValue] generates a call to validateEditing 
             so we need to call it before setting up the _text_object */
          length = [[self stringValue] length];
          _text_object = [_cell setUpFieldEditorAttributes: text];
	  if (sender == self)
	  {
	    /* Special case: If -selectText: is called in response to making
	       the receiver first responder with a mouse click, don't select
	       the whole text. First of all, this is redundant since the field
	       editor will receive the mouse click and immediately collapse the
	       selection by placing the insertion point at the mouse location.
	       Furthermore, we thus preserve the current selection on X and
	       therefore the user can easily paste it into the text field with
	       a middle mouse click. */
	    NSEvent *event = [_window currentEvent];
	    if ([event type] == NSLeftMouseDown &&
		NSPointInRect([event locationInWindow],
			      [self convertRect:[self bounds] toView:nil]))
	      {
		length = 0;
	      }
	  }
          [_cell selectWithFrame: _bounds
                 inView: self
                 editor: _text_object
                 delegate: self
                 start: 0
                 length: length];
        }
    }
}

/**<p>Returns the object selected when the user presses the TAB key.</p>
   <p>See Also: -setNextText: [NSView-nextKeyView]</p>
 */
- (id) nextText
{
  return [self nextKeyView];
}

/**<p>Returns the object selected when the user presses the Shift-TAB key.</p>
   <p>See Also: -setPreviousText: [NSView-previousKeyView]</p>
 */
- (id) previousText
{
  return [self previousKeyView];
}

/**<p>Sets the object selected when the user presses the TAB key to
   <var>anObject</var>.</p>
   <p>See Also: -nextText [NSView-setNextKeyView:]</p>
 */
- (void) setNextText: (id)anObject
{
  [self setNextKeyView: anObject];
}

/**<p>Sets the object selected when the user presses the shift-TAB key to
   <var>anObject</var>.</p>
   <p>See Also: -previousText [NSView-setPreviousKeyView:]</p>
 */
- (void) setPreviousText: (id)anObject
{
  [self setPreviousKeyView: anObject];
}

/** <p>Sets the delegate to <var>anObject</var>.</p>
    <p>See Also: -delegate</p>
*/
- (void) setDelegate: (id<NSTextFieldDelegate>)anObject
{
  if (_delegate)
    [nc removeObserver: _delegate name: nil object: self];
  _delegate = anObject;

#define SET_DELEGATE_NOTIFICATION(notif_name) \
  if ([_delegate respondsToSelector: @selector(controlText##notif_name:)]) \
    [nc addObserver: _delegate \
      selector: @selector(controlText##notif_name:) \
      name: NSControlText##notif_name##Notification object: self]

  SET_DELEGATE_NOTIFICATION(DidBeginEditing);
  SET_DELEGATE_NOTIFICATION(DidEndEditing);
  SET_DELEGATE_NOTIFICATION(DidChange);
}

/** <p>Returns the delegate object.</p>
    <p>See Also: -setDelegate:</p>
*/
- (id<NSTextFieldDelegate>) delegate
{
  return _delegate;
}

/**<p>Sets the color used to draw the background to <var>aColor</var>.</p>
   <p>See Also: -backgroundColor -setDrawsBackground: -drawsBackground
   [NSTextFieldCell-setBackgroundColor:]</p>
 */
- (void) setBackgroundColor: (NSColor *)aColor
{
  [_cell setBackgroundColor: aColor];
}

/** <p>Returns the color used to draw the background.</p>
    <p>See Also: -setBackgroundColor: setDrawsBackground: -drawsBackground
    [NSTextFieldCell-backgroundColor]</p>
 */
- (NSColor *) backgroundColor
{
  return [_cell backgroundColor];
}

/** <p>Returns whether the NSTextField draws the background. By default NO.</p>
    <p>See Also: -setDrawsBackground: [NSTextFieldCell-drawsBackground]</p>
 */
- (BOOL) drawsBackground
{
  return [_cell drawsBackground];
}

/** <p>Returns whether the NSTextField's cell has bezeled border.</p>
    <p>See Also: -setBezeled: [NSTextFieldCell-isBezeled]</p>
 */
- (BOOL) isBezeled
{
  return [_cell isBezeled];
}

/** <p>Returns whether the NSTextField's cell has border.</p>
    <p>See Also: -setBordered: [NSTextFieldCell-isBordered]</p>
 */
- (BOOL) isBordered
{
  return [_cell isBordered];
}

/** <p>Sets whether the NSTextField's cell has bezeled border.</p>
    <p>See Also: -isBezeled [NSTextFieldCell-setBezeled:]</p>
 */
- (void) setBezeled: (BOOL)flag
{
  [_cell setBezeled: flag];
}

/** <p>Sets whether the NSTextField's cell has border.</p>
    <p>See Also: -isBordered [NSTextFieldCell-setBordered:]</p>
 */
- (void) setBordered: (BOOL)flag
{
  [_cell setBordered: flag];
}

/** <p>Sets whether the NSTextField draws the background. By default NO.</p>
    <p>See Also: -drawsBackground [NSTextFieldCell-setDrawsBackground:]</p>
 */
- (void) setDrawsBackground: (BOOL)flag
{
  [_cell setDrawsBackground: flag];
}

/** <p>Sets the  color with which the text will be draw to aColor.</p>
    <p>See Also: -textColor [NSTextFieldCell-setTextColor:]</p>
 */
- (void) setTextColor: (NSColor *)aColor
{
  [_cell setTextColor: aColor];
}

/** <p>Returns the colour used to draw the text.</p>
    <p>See Also: -setTextColor: [NSTextFieldCell-textColor]</p>
 */
- (NSColor *) textColor
{
  return [_cell textColor];
}

//
// Target and Action
//
- (SEL) errorAction
{
  return _error_action;
}

- (void) setErrorAction: (SEL)aSelector
{
  _error_action = aSelector;
}

//
// Handling Events
//
- (void) mouseDown: (NSEvent*)theEvent
{
  if ([self isSelectable] == NO || [self isEnabled] == NO)
    {
      if (_next_responder)
        [_next_responder mouseDown: theEvent];
      else
        [super mouseDown: theEvent];
      return;
    }

  /* NB: If we're receiving this click from the NSWindow, we expect
     _text_object to never be nil here, since NSWindow makes the
     NSTextField the first responder (which invokes its
     -becomeFirstResponder:, which invokes its -selectText:, which, if
     it is selectable, sets up the _text_object, then makes the
     _text_object first responder!) before calling its -mouseDown:.
     Only the first click should go via here; further clicks will be
     sent directly by the NSWindow to the _text_object.
  */
  if (_text_object)
    {
      [_text_object mouseDown: theEvent];
      return;
    }
  else
    {
      /* I suppose you could get here in subclasses which override
       * -becomeFirstResponder not to select the text.  In that case,
       * we set up the _text_object manually to start editing here.
       */

      /* Make sure we have first responder status when we start edit.
       * This does nothing if we are already first responder; but
       * (important!) it implicitly should free the fieldEditor if it
       * was in use by another control.
       */
      if ([_window makeFirstResponder: self])
        {
          NSText *t = [_window fieldEditor: YES forObject: self];
          
          if ([t superview] != nil)
            {
              /* Can't take the field editor ... give up.  */
                return;
            }
          
          _text_object = [_cell setUpFieldEditorAttributes: t];
          [_cell editWithFrame: _bounds
                 inView: self
                 editor: _text_object
                 delegate: self
                 event: theEvent];
        }
    }
}

- (BOOL) acceptsFirstMouse: (NSEvent *)aEvent
{
  return [self isEditable] && [self isEnabled];
}

/** <p>Returns whether the NSTextField accepts to be the first responder.
    This method returns YES if the NSTextField is selectable and if
    there is no NSText object</p><p>See Also: -becomeFirstResponder 
    -isSelectable [NSView-acceptsFirstResponder]</p>
 */
- (BOOL) acceptsFirstResponder
{
  // we do not accept first responder if there is already a 
  // _text_object, else it would make the _text_object resign
  // and end editing
  return (_text_object == nil) && [self isSelectable] && [self isEnabled];
}

- (BOOL) becomeFirstResponder
{
  if ([self acceptsFirstResponder])
    {
      [self selectText: self];
      return YES;
    }
  else
    {
      return NO;
    }
}

-(BOOL) needsPanelToBecomeKey
{
  return [self isEditable] || [self isSelectable];
}

- (BOOL) abortEditing
{
  if (_text_object)
    {
      [_cell endEditing: _text_object];
      _text_object = nil;
      return YES;
    }
  else 
    return NO;
}

- (NSText *) currentEditor
{
  if (_text_object && ([_window firstResponder] == _text_object))
    return _text_object;
  else
    return nil;
}

- (void) validateEditing
{
  if (_text_object)
    {
      NSFormatter *formatter;
      NSString *string;
      BOOL validatedOK = YES;

      formatter = [_cell formatter];
      string = AUTORELEASE ([[_text_object text] copy]);

      if (formatter != nil)
        {
          id newObjectValue;
          NSString *error;
          
          if ([formatter getObjectValue: &newObjectValue 
                         forString: string 
                         errorDescription: &error] == YES)
            {
              [_cell setObjectValue: newObjectValue];
              return;
            }
          else
            {
              SEL sel = @selector(control:didFailToFormatString:errorDescription:);
              
              if ([_delegate respondsToSelector: sel])
                {
                  validatedOK = [_delegate control: self 
                                           didFailToFormatString: string 
                                           errorDescription: error];
                }
              else if (![string isEqualToString: @""])
                {
                  validatedOK = NO;
                }
            }
        }

      if (validatedOK)
        {
          [_cell setStringValue: string];
        }
    }
}

- (void) textDidBeginEditing: (NSNotification *)aNotification
{
  [super textDidBeginEditing: aNotification];
}

- (void) textDidChange: (NSNotification *)aNotification
{

  NSFormatter *formatter;

  [super textDidChange: aNotification];

  formatter = [_cell formatter];
  if (formatter != nil)
    {
      /*
       * FIXME: This part needs heavy interaction with the yet to finish 
       * text system.
       *
       */
      NSString *partialString;
      NSString *newString = nil;
      NSString *error = nil;
      BOOL wasAccepted;
      
      partialString = [_text_object string];
      wasAccepted = [formatter isPartialStringValid: partialString 
                               newEditingString: &newString 
                               errorDescription: &error];

      if (wasAccepted == NO)
        {
          SEL sel = @selector(control:didFailToValidatePartialString:errorDescription:);
          
          if ([_delegate respondsToSelector: sel])
            {
              [_delegate control: self 
                         didFailToValidatePartialString: partialString 
                         errorDescription: error];
            }
        }

      if (newString != nil)
        {
          // This resets editing with insertion point after the string, which is what Cocoa does
          [_text_object setString: newString];
        }
      else
        {
          if (wasAccepted == NO)
            {
              // FIXME: Need to delete last typed character (?!)
              NSLog (@"Unimplemented: should delete last typed character");
            }
        }
    }
}

- (void) textDidEndEditing: (NSNotification *)aNotification
{
  id textMovement;
  int movement;

  [super textDidEndEditing: aNotification];

  textMovement = [[aNotification userInfo] objectForKey: @"NSTextMovement"];

  if (textMovement)
    {
      movement = [(NSNumber *)textMovement intValue];
    }
  else
    {
      movement = 0;
    }

  if (movement == NSReturnTextMovement)
    {
      if ([self sendAction: [self action] to: [self target]] == NO)
        {
          [self performKeyEquivalent: [_window currentEvent]];
        }
      if ([_window firstResponder] == _window)
	[self selectText: self];
    }
  else
    {
      if ([[self cell] sendsActionOnEndEditing])
        [self sendAction: [self action] to: [self target]];

      if (movement == NSTabTextMovement)
        {
          [_window selectKeyViewFollowingView: self];
          if ([_window firstResponder] == _window)
            [self selectText: self];
        }
      else if (movement == NSBacktabTextMovement)
        {
          [_window selectKeyViewPrecedingView: self];
          if ([_window firstResponder] == _window)
            [self selectText: self];
        }
    }
}

- (BOOL) textShouldBeginEditing: (NSText *)textObject
{
  if ([self isEditable] == NO)
    return NO;
  
  if (_delegate && [_delegate respondsToSelector: 
                                @selector(control:textShouldBeginEditing:)])
    return [_delegate control: self 
                      textShouldBeginEditing: textObject];
  else 
    return YES;
}

- (BOOL) textShouldEndEditing: (NSText*)textObject
{
  if ([_cell isEntryAcceptable: [textObject text]] == NO)
    {
      [self sendAction: _error_action to: [self target]];
      return NO;
    }
  
  if ([_delegate respondsToSelector: 
                   @selector(control:textShouldEndEditing:)])
    {
      if ([_delegate control: self textShouldEndEditing: textObject] == NO)
        {
          NSBeep ();
          return NO;
        }
    }

  if ([_delegate respondsToSelector: @selector(control:isValidObject:)] == YES)
    {
      NSFormatter *formatter;
      id newObjectValue;
      
      formatter = [_cell formatter];
      
      if ([formatter getObjectValue: &newObjectValue 
                          forString: [_text_object text] 
                   errorDescription: NULL] == YES)
        {
          if ([_delegate control: self isValidObject: newObjectValue] == NO)
            {
              return NO;
            }
        }
    }

  // In all other cases
  return YES;
}

- (BOOL)textView: (NSTextView*)textView doCommandBySelector: (SEL)command
{
  if (_delegate && 
      [_delegate respondsToSelector: @selector(control:textView:doCommandBySelector:)])
    {
      return [_delegate control: self 
                        textView: textView 
                        doCommandBySelector: command];
    }

   return NO;
}


//
// Rich Text
//
- (void)setAllowsEditingTextAttributes:(BOOL)flag
{
  [_cell setAllowsEditingTextAttributes: flag];
}

- (BOOL)allowsEditingTextAttributes
{
  return [_cell allowsEditingTextAttributes];
}

- (void)setImportsGraphics:(BOOL)flag
{
  [_cell setImportsGraphics: flag];
}

- (BOOL)importsGraphics
{
  return [_cell importsGraphics];
}

- (void)setTitleWithMnemonic:(NSString *)aString
{
  [_cell setTitleWithMnemonic: aString];
}

- (void)setBezelStyle:(NSTextFieldBezelStyle)style
{
  [_cell setBezelStyle: style];
}

- (NSTextFieldBezelStyle)bezelStyle
{
  return [_cell bezelStyle];
}

//
// NSCoding protocol
//
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if (![aCoder allowsKeyedCoding])
    {
      [aCoder encodeConditionalObject: _delegate];
      [aCoder encodeValueOfObjCType: @encode(SEL) at: &_error_action];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if ([aDecoder allowsKeyedCoding])
    {
      // do nothing for now...
    }
  else
    {
      [self setDelegate: [aDecoder decodeObject]];
      [aDecoder decodeValueOfObjCType: @encode(SEL) at: &_error_action];
    }
  _text_object = nil;

  return self;
}

//
// Bindings
//

/*
 * Bindings implemented:
 * alignment, hidden, editable, enabled, font, toolTip
 *
 * Bindings left to implement:
 * other font bindings, value, displayPatternValue1
 */

- (void) bind: (NSString *)binding
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding hasPrefix: NSEditableBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueAndBinding alloc] initWithBinding: NSEditableBinding 
                                                 withName: binding 
                                                 toObject: anObject
                                              withKeyPath: keyPath
                                                  options: options
                                               fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      [super bind: binding
         toObject: anObject
      withKeyPath: keyPath
          options: options];
    }
}

@end

