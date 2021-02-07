/** <title>NSControl</title>

   <abstract>The abstract control class</abstract>

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996
   Author: Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1998

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

#import <Foundation/NSDebug.h>
#import <Foundation/NSException.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSActionCell.h"
#import "AppKit/NSApplication.h"
#import "AppKit/NSCell.h"
#import "AppKit/NSControl.h"
#import "AppKit/NSColor.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSFont.h"
#import "AppKit/NSFontManager.h"
#import "AppKit/NSKeyValueBinding.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSWindow.h"
#import "GSBindingHelpers.h"
#import "NSViewPrivate.h"
#import "GSGuiPrivate.h"

/*
 * Class variables
 */
static Class usedCellClass;
static Class cellClass;
static Class actionCellClass;
static NSNotificationCenter *nc;

/**<p>TODO Description</p>
 */

@implementation NSControl

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSControl class])
    {
      [self setVersion: 1];
      cellClass = [NSCell class];
      usedCellClass = cellClass;
      actionCellClass = [NSActionCell class];
      // Cache the notifiaction centre for editing notifications
      nc = [NSNotificationCenter defaultCenter];
 
     // expose bindings
      [self exposeBinding: NSValueBinding];
      [self exposeBinding: NSEnabledBinding];
      [self exposeBinding: NSAlignmentBinding];
      [self exposeBinding: NSFontBinding];
      [self exposeBinding: NSFontNameBinding];
      [self exposeBinding: NSFontSizeBinding];
     }
}

/**<p> Returns the cell Class used by NSControl. Used by subclasses.</p>
   <p>See Also: +setCellClass:</p>    
 */
+ (Class) cellClass
{
  return usedCellClass;
}

/**<p> Sets the cell Class used by NSControl to <var>factoryId</var>.
   Used by subclasses.</p> <p>See Also: +setCellClass:</p>    
 */
+ (void) setCellClass: (Class)factoryId
{
  usedCellClass = factoryId ? factoryId : cellClass;
}

/**<p>Initializes and returns a new NSControl into the rectangle 
   frameRect and create a new associated NSCell</p><p>See Also: -setCell:</p> 
 */
- (id) initWithFrame: (NSRect)frameRect
{
  NSCell *cell = [[[self class] cellClass] new];

  [super initWithFrame: frameRect];
  [self setCell: cell];
  RELEASE(cell);
  //_tag = 0;

  return self;
}

- (void) dealloc
{
  RELEASE(_cell);
  [super dealloc];
}

/** <p>Returns the NSControl's cell.</p><p>See Also: -setCell:</p> 
 */
- (id) cell
{
  return _cell;
}

/** <p>Sets the NSControl's cell to aCell, 
    Raises an NSInvalidArgumentException exception if aCell is not nil and
    if it is not a cell class.</p><p>See Also: -cell</p> 
 */
- (void) setCell: (NSCell *)aCell
{
  if (aCell != nil && [aCell isKindOfClass: cellClass] == NO)
    [NSException raise: NSInvalidArgumentException
		format: @"attempt to set non-cell object for control cell"];

  ASSIGN(_cell, aCell);
}

/**<p>Returns whether the selected cell of the NSControl is enabled.</p>
   <p>See Also: -setEnabled: [NSCell-isEnabled]</p>
 */
- (BOOL) isEnabled
{
  return [[self selectedCell] isEnabled];
}

/**<p>Sets whether the NSControl's selected cell is enabled.
   If flag is NO, this method abort the editing. This method marks self for
   display.</p><p>See Also: -isEnabled [NSCell-setEnabled:]</p>
 */
- (void) setEnabled: (BOOL)flag
{
  [[self selectedCell] setEnabled: flag];
  if (!flag)
    [self abortEditing];

  [self setNeedsDisplay: YES];
}

/** <p>Returns the NSControl's selected cell.</p>
 */
- (id) selectedCell
{
  return _cell;
}

/** <p>Returns the tag of the NSControl's selected cell (if exists).
    -1 otherwise.</p><p>See Also: [NSCell-tag]</p>
 */
- (NSInteger) selectedTag
{
  NSCell *selected = [self selectedCell];

  if (selected == nil)
    return -1;
  else
    return [selected tag];
}

/** <p>Returns the value of the NSControl's selected cell as double.</p>
    <p>See Also:  -setDoubleValue: [NSCell-doubleValue] -intValue -floatValue
    -doubleValue -stringValue</p>
 */
- (double) doubleValue
{
  // The validation is performed by the NSActionCell
  return [[self selectedCell] doubleValue];
}

/** <p>Returns the value of the NSControl's selected cell as float.</p>
    <p>See Also: -setFloatValue: [NSCell-floatValue] -intValue -stringValue
    -doubleValue</p>
 */
- (float) floatValue
{
  return [[self selectedCell] floatValue];
}

/** <p>Returns the value of the NSControl's selected cell as int.</p>
    <p>See Also: -setIntValue: [NSCell-intValue] -floatValue -doubleValue
    -stringValue</p>
 */
- (int) intValue
{
  return [[self selectedCell] intValue];
}

/** <p>Returns the value of the NSControl's selected cell as int.</p>
    <p>See Also: -setIntegerValue: [NSCell-integerValue] -floatValue -doubleValue
    -stringValue</p>
 */
- (NSInteger) integerValue
{
  return [[self selectedCell] integerValue];
}

/** <p>Returns the value of the NSControl's selected cell as NSString.</p>
    <p>See Also: -setStringValue: [NSCell-stringValue] -intValue -floatValue
    -doubleValue -stringValue</p>
 */
- (NSString *) stringValue
{
  return [[self selectedCell] stringValue];
}

- (id) objectValue
{
  return [[self selectedCell] objectValue];
}

/** <p>Sets the value of the NSControl's selected cell to double.
    If the selected cell is an action cell, it marks self for display.</p>
    <p>See Also: -doubleValue [NSCell-setDoubleValue:] -setIntValue:
    -setStringValue: -setFloatValue:</p>
 */
- (void) setDoubleValue: (double)aDouble
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setDoubleValue: aDouble];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

/** <p>Sets the value of the NSControl's selected cell to float.
    If the selected cell is an action cell, it marks self for display.</p>
    <p>See Also: -floatValue [NSCell-setFloatValue:] -setIntValue: 
    -setStringValue: -setDoubleValue:</p>
 */
- (void) setFloatValue: (float)aFloat
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setFloatValue: aFloat];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

/** <p>Sets the value of the NSControl's selected cell to int.
    If the selected cell is an action cell, it marks self for display.</p>
    <p>See Also: -intValue [NSCell-setIntValue:] -setDoubleValue: 
    -setFloatValue: -setStringValue:</p>
 */
- (void) setIntValue: (int)anInt
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setIntValue: anInt];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

/** <p>Sets the value of the NSControl's selected cell to int.
    If the selected cell is an action cell, it marks self for display.</p>
    <p>See Also: -integerValue [NSCell-setIntegerValue:] -setDoubleValue: 
    -setFloatValue: -setStringValue:</p>
 */
- (void) setIntegerValue: (NSInteger)anInt
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setIntegerValue: anInt];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

/** <p>Sets the value of the NSControl's selected cell to NSString.
    If the selected cell is an action cell, it marks self for display.</p>
    <p>See Also: -stringValue [NSCell-setStringValue:] -setIntValue:
    -setFloatValue: -setDoubleValue:</p>
 */
- (void) setStringValue: (NSString *)aString
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setStringValue: aString];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

- (void) setObjectValue: (id)anObject
{
  NSCell *selected = [self selectedCell];
  BOOL wasEditing = [self abortEditing];

  [selected setObjectValue: anObject];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];

  if (wasEditing)
    {
      [[self window] makeFirstResponder: self];
    }
}

/** <p>Marks self for display.</p>
*/
- (void) setNeedsDisplay
{
  [super setNeedsDisplay: YES];
}

/**<p>Sets the NSControl's selected cell to the sender's double value.</p>
   <p>See Also: [NSCell-takeDoubleValueFrom:] -takeFloatValueFrom: 
   takeIntValueFrom: takeStringValueFrom:</p> 
*/
- (void) takeDoubleValueFrom: (id)sender
{
  [[self selectedCell] takeDoubleValueFrom: sender];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the NSControl's selected cell to the sender's float value.</p>
   <p>See Also: [NSCell-takeDoubleValueFrom:] -takeDoubleValueFrom: 
   -takeIntValueFrom: -takeStringValueFrom:</p> 
*/
- (void) takeFloatValueFrom: (id)sender
{
  [[self selectedCell] takeFloatValueFrom: sender];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the NSControl's selected cell to the sender's float int.</p>
   <p>See Also: [NSCell-takeIntValueFrom:] -takeDoubleValueFrom: 
   -takeFloatValueFrom: -takeStringValueFrom:</p> 
*/
- (void) takeIntValueFrom: (id)sender
{
  [[self selectedCell] takeIntValueFrom: sender];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the NSControl's selected cell to the sender's float int.</p>
   <p>See Also: [NSCell-takeIntegerValueFrom:] -takeDoubleValueFrom: 
   -takeFloatValueFrom: -takeStringValueFrom:</p> 
*/
- (void) takeIntegerValueFrom: (id)sender
{
  [[self selectedCell] takeIntegerValueFrom: sender];
  [self setNeedsDisplay: YES];
}

- (void) takeObjectValueFrom: (id)sender
{
  [[self selectedCell] takeObjectValueFrom: sender];
  [self setNeedsDisplay: YES];
}

/**<p>Sets the NSControl's selected cell to the sender's float int.</p>
   <p>See Also: [NSCell-takeDoubleValueFrom:] -takeDoubleValueFrom: 
   -takeFloatValueFrom: -takeIntValueFrom:</p> 
*/
- (void) takeStringValueFrom: (id)sender
{
  [[self selectedCell] takeStringValueFrom: sender];
  [self setNeedsDisplay: YES];
}

/**<p>Returns the alignment of the text in the NSControl's cell.
   Returns NSNaturalTextAlignment if the cell does not exists.
   See <ref type="type" id="NSTextAlignment">NSTextAlignment</ref> for
   more informations.</p><p>See Also: -setAlignment: [NSCell-alignment] </p>
 */
- (NSTextAlignment) alignment
{
  if (_cell)
    return [_cell alignment];
  else
    return NSNaturalTextAlignment;
}

/**<p>Returns the font of the text in the NSControl's cell. Returns nil if
   the cell does not exists.</p><p>See Also: -setFont: [NSCell-font]</p>
 */
- (NSFont *) font
{
  if (_cell)
    return [_cell font];
  else
    return nil;
}

/**<p>Sets the alignment of the text in the NSControl's cell to 
   <var>mode</var>. This method abort the editing and  marks self for display 
   if the cell is an NSActionCell. See <ref type="type" id="NSTextAlignment">
   NSTextAlignment</ref> for more informations.</p>
   <p>See Also: -alignment [NSCell-setAlignment:] -abortEditing</p>
 */
- (void) setAlignment: (NSTextAlignment)mode
{
  if (_cell)
    {
      [self abortEditing];

      [_cell setAlignment: mode];
      if (![_cell isKindOfClass: actionCellClass])
	[self setNeedsDisplay: YES];
    }
}

/**<p>Sets the font of the text in the NSControl's cell and the
   editor object (if exists) to fontObject.</p>
   <p>See Also: -font [NSCell-setFont:] -currentEditor</p>
 */
- (void) setFont: (NSFont *)fontObject
{
  if (_cell)
    {
      NSText *editor = [self currentEditor];
      
      [_cell setFont: fontObject];
      if (editor != nil)
	[editor setFont: fontObject];
    }
}

- (void) setFloatingPointFormat: (BOOL)autoRange
			   left: (NSUInteger)leftDigits
			  right: (NSUInteger)rightDigits
{
  [self abortEditing];

  [_cell setFloatingPointFormat: autoRange  left: leftDigits
	 right: rightDigits];
  if (![_cell isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];
}

- (void) setFormatter: (NSFormatter*)newFormatter 
{
  if (_cell)
    {
      [_cell setFormatter: newFormatter];
      if (![_cell isKindOfClass: actionCellClass])
	[self setNeedsDisplay: YES];
    }
}

- (id) formatter
{
  return [_cell formatter];
}

- (NSWritingDirection) baseWritingDirection
{
  return [_cell baseWritingDirection];
}

- (void) setBaseWritingDirection: (NSWritingDirection)direction
{
  if (_cell)
    {
      [_cell setBaseWritingDirection: direction];
      if (![_cell isKindOfClass: actionCellClass])
	[self setNeedsDisplay: YES];
    }
}

/**<p>Sends an [NSCell-endEditing:] message to the current object used to
   edit the NSControl. Returns NO if the the currentEditor does not exists, 
   YES otherwise.</p>
 */
- (BOOL) abortEditing
{
  NSText *text;

  text = [self currentEditor];
  if (text == nil)
    {
      return NO;
    }

  [[self selectedCell] endEditing: text];
  return YES;
}

/**<p>Returns the NSText object used when editing the NSControl.</p>
 */
- (NSText *) currentEditor
{
  if (_cell != nil)
    {
      NSText *text;

      text = [_window fieldEditor: NO forObject: self];
      if (([text delegate] == self) && ([_window firstResponder] == text))
        {
	  return text;
	}
    }

  return nil;
}

/**
 */
- (void) validateEditing
{
  NSText *text;

  text = [self currentEditor];
  if (text == nil)
    {
      return;
    }

  if ([text isRichText])
    {
      NSAttributedString *attr;
      NSTextStorage *storage;
      int len;
      
      storage = [(NSTextView*)text textStorage];
      len = [storage length];
      attr = [storage attributedSubstringFromRange: NSMakeRange(0, len)];
      [[self selectedCell] setAttributedStringValue: attr];
    }
  else
    {
      NSString *string;

      string = AUTORELEASE([[text string] copy]);
      [[self selectedCell] setStringValue: string];
    }
}

/* 
 * Text delegate methods 
 */

/**<p>Invokes when the text cell starts to be editing.This methods posts 
   a NSControlTextDidBeginEditingNotification with a dictionary containing 
   the NSFieldEditor as user info </p><p>See Also:
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidBeginEditing: (NSNotification *)aNotification
{
  NSMutableDictionary *dict;

  dict = [[NSMutableDictionary alloc] initWithDictionary: 
					[aNotification userInfo]];
  [dict setObject: [aNotification object] forKey: @"NSFieldEditor"];

  [nc postNotificationName: NSControlTextDidBeginEditingNotification
      object: self
      userInfo: dict];
  RELEASE(dict);
}

/**<p>Invokes when the text cell is changed. This methods posts a 
   NSControlTextDidChangeNotification with a dictionary containing the
   NSFieldEditor as user info </p><p>See Also: 
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidChange: (NSNotification *)aNotification
{
  NSMutableDictionary *dict;

  dict = [[NSMutableDictionary alloc] initWithDictionary: 
				     [aNotification userInfo]];
  [dict setObject: [aNotification object] forKey: @"NSFieldEditor"];

  [nc postNotificationName: NSControlTextDidChangeNotification
      object: self
      userInfo: dict];
  RELEASE(dict);
}

/**<p>Invokes when the text cell is changed.
   This methods posts a NSControlTextDidEndEditingNotification
   a dictionary containing the NSFieldEditor as user info </p><p>See Also:
   [NSNotificationCenter-postNotificationName:object:userInfo:]</p>
*/
- (void) textDidEndEditing: (NSNotification *)aNotification
{
  NSMutableDictionary *dict;

  [self validateEditing];
  [self abortEditing];

  dict = [[NSMutableDictionary alloc] initWithDictionary: 
					[aNotification userInfo]];
  [dict setObject: [aNotification object] forKey: @"NSFieldEditor"];

  [nc postNotificationName: NSControlTextDidEndEditingNotification
      object: self
      userInfo: dict];
  RELEASE(dict);
}

/**<p>Recalculates the internal size by sending [NSCell-calcDrawInfo:] 
   to the cell.</p>
 */
- (void) calcSize
{
  [_cell calcDrawInfo: [self bounds]];
}

/**<p>Resizes the NSControl to fits the NSControl's cell size.</p> 
  <p>See Also: [NSCell-cellSize]</p>
 */
- (void) sizeToFit
{
  [self setFrameSize: [_cell cellSize]];
}

/** <p>Returns whether the NSControl's cell is opaque</p>
 */
- (BOOL) isOpaque
{
  return [_cell isOpaque];
}

- (void) drawRect: (NSRect)aRect
{
  [self drawCell: _cell];
}

/**<p>Redraws a <var>aCell</var> if it is the NSControl's cell.</p>
   <p>See Also: -setCell: [NSCell-drawWithFrame:inView:]</p>
 */
- (void) drawCell: (NSCell *)aCell
{
  if (_cell == aCell)
    {
      [_cell drawWithFrame: _bounds
             inView: self];
    }
}

/**<p>Redraws a <var>aCell</var>'s inside if it is the NSControl's cell.</p>
   <p>See Also: -setCell: [NSCell-drawInteriorWithFrame:inView:]</p>
 */
- (void) drawCellInside: (NSCell *)aCell
{
  if (_cell == aCell)
    {
      [_cell drawInteriorWithFrame: _bounds
             inView: self];
    }
}

/** <p>Sets the aCell's state to NSOnState and marks self for display
    if it is the NSControl's cell.</p>
 */
- (void) selectCell: (NSCell *)aCell
{
  if (_cell == aCell)
    {
      [_cell setState: NSOnState];
      [self setNeedsDisplay: YES];
    }
}

/** <p>Marks self for display.</p>
 */
- (void) updateCell: (NSCell *)aCell
{
  [self setNeedsDisplay: YES];
}

/** <p>Marks self for display.</p>
 */
- (void) updateCellInside: (NSCell *)aCell
{
  [self setNeedsDisplay: YES];
}

/** <p>Returns the NSControl's cell action method.</p>
    <p>See Also: -setAction: [NSCell-action] </p>
 */
- (SEL) action
{
  return [_cell action];
}

/** <p>Returns whether the NSControl's cell can continuously sends its action
    message.</p><p>See Also: -setContinuous: [NSCell-isContinuous]</p>
 */
- (BOOL) isContinuous
{
  return [_cell isContinuous];
}

/**<p>Asks the NSApplication to send an action <var>theAction</var> 
   with <var>theTarget</var> as target to NSControl.
   Returns NO if <var>theAction</var> is nil or if NSApplication can 
   not send the action.</p>
   <p>See Also: [NSApplication-sendAction:to:from:]</p>
 */
- (BOOL) sendAction: (SEL)theAction to: (id)theTarget
{
  GSKeyValueBinding *theBinding;

  theBinding = [GSKeyValueBinding getBinding: NSValueBinding 
                                  forObject: self];
  if (theBinding != nil)
    [theBinding reverseSetValueFor: @"objectValue"];

  if (theAction)
    return [NSApp sendAction: theAction to: theTarget from: self];
  else
    return NO;
}

- (NSInteger) sendActionOn: (NSInteger)mask
{
  return [_cell sendActionOn: mask];
}

/**<p>Sets the NSControl's cell action method.</p>
   <p>See Also: -action [NSCell-setAction:]</p>
*/
- (void) setAction: (SEL)aSelector
{
  [_cell setAction: aSelector];
}

/** <p>Sets whether the NSControl's cell can continuously sends its action 
    message.</p><p>See Also: -isContinuous [NSCell-setContinuous:]</p>
*/ 
- (void) setContinuous: (BOOL)flag
{
  [_cell setContinuous: flag];
}

/** <p>Sets the target object of the NSControl's cell to anObject.</p>
    <p>See Also: -target [NSCell-setTarget:]</p>
 */
- (void) setTarget: (id)anObject
{
  [_cell setTarget: anObject];
}

/**<p>Returns the target object of the NSControl's cell.</p>
   <p>See Also: -setTarget: [NSCell-target]</p>
 */
- (id) target
{
  return [_cell target];
}

/*
 * Attributed string handling
 */
- (void) setAttributedStringValue: (NSAttributedString*)attribStr
{
  NSCell *selected = [self selectedCell];

  [self abortEditing];

  [selected setAttributedStringValue: attribStr];
  if (![selected isKindOfClass: actionCellClass])
    [self setNeedsDisplay: YES];
}

- (NSAttributedString*) attributedStringValue
{
  NSCell *selected = [self selectedCell];

  if (selected == nil)
    {
      return AUTORELEASE([NSAttributedString new]);
    }

  // As this method is not defined for NSActionCell, we have 
  // to do the validation here.
  [self validateEditing];

  return [selected attributedStringValue];
}

/** Assigning a Tag
 */
- (void) setTag: (NSInteger)anInt
{
  _tag = anInt;
}

/**<p>Returns the NSControl tag</p>
   <p>See Also: -setTag:</p>
 */
- (NSInteger) tag
{
  return _tag;
}

/*
 * Activation
 */

/**
 * Simulates a single mouse click on the control. This method calls the cell's
 * method performClickWithFrame:inView:. Take note that <var>sender</var> is not
 * used. 
 */
- (void) performClick: (id)sender
{
  [_cell performClickWithFrame: [self bounds] inView: self];
}

- (BOOL) refusesFirstResponder
{
  return [[self selectedCell] refusesFirstResponder];
}

- (void) setRefusesFirstResponder:(BOOL)flag
{
  [[self selectedCell] setRefusesFirstResponder: flag];
}

- (BOOL) acceptsFirstResponder
{
  return [[self selectedCell] acceptsFirstResponder];
}


- (void) mouseDown: (NSEvent *)theEvent
{
  unsigned int event_mask = NSLeftMouseDownMask | NSLeftMouseUpMask
    | NSMouseMovedMask | NSLeftMouseDraggedMask | NSOtherMouseDraggedMask
    | NSRightMouseDraggedMask;
  NSEvent *e = nil;

  // If not enabled ignore mouse clicks
  if (![self isEnabled])
    {  
      [super mouseDown: theEvent];
      return;
    }

  // Ignore multiple clicks, if configured to do so
  if (_ignoresMultiClick && ([theEvent clickCount] > 1))
    {  
      [super mouseDown: theEvent];
      return;
    }

  // Make sure self does not go away during the processing of the event
  RETAIN(self); 

  // loop until mouse goes up
  e = theEvent;
  while (1)
    {
      NSPoint location = [self convertPoint: [e locationInWindow] 
			       fromView: nil];

      // ask the cell to track the mouse only,
      // if the mouse is within the cell
      if ([self mouse: location inRect: _bounds])
	{
	  BOOL done;

	  [_cell setHighlighted: YES];
	  [self setNeedsDisplay: YES];
	  done = [_cell trackMouse: e
			inRect: _bounds	
			ofView: self	
			untilMouseUp: [[_cell class] prefersTrackingUntilMouseUp]];
	  [_cell setHighlighted: NO];
	  [self setNeedsDisplay: YES];

	  if (done)
	    {
	      break;
	    }
	}

      e = [NSApp nextEventMatchingMask: event_mask
		 untilDate: [NSDate distantFuture]
		 inMode: NSEventTrackingRunLoopMode
		 dequeue: YES];
      if ([e type] == NSLeftMouseUp)
        {
	  break;
        }
    }

  // undo initial retain
  RELEASE(self); 
}

- (BOOL) shouldBeTreatedAsInkEvent: (NSEvent *)theEvent
{
  return NO;
}

- (void) resetCursorRects
{
  [_cell resetCursorRect: _bounds inView: self];
}

/**<p>Returns wheter multiple clicks are ignored.</p>
   <p>See Also: -setIgnoresMultiClick: -mouseDown:</p>
 */
- (BOOL) ignoresMultiClick
{
  return _ignoresMultiClick;
}


/** <p>Sets wheter multiple clicks are ignored.</p>
    <p>See Also: -ignoresMultiClick -mouseDown:</p>
 */
- (void) setIgnoresMultiClick: (BOOL)flag
{
  _ignoresMultiClick = flag;
}

/**<p>Returns the mouse flags. This flags are usally sets in 
   the NSCell-trackMouse:inRect:ofView:untilMouseUp: method.</p>
   <p>This is a NeXTStep 3.3 method, no longer officially supported.</p>
 */
- (NSInteger) mouseDownFlags
{ 
  return [[self selectedCell] mouseDownFlags];
}

/*
 * NSCoding protocol
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];
  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: [self cell] forKey: @"NSCell"];
      [aCoder encodeBool: [self isEnabled] forKey: @"NSEnabled"];
       if (_tag)
        {
          [aCoder encodeInt: [self tag] forKey: @"NSTag"];
        }
    }
  else
    {
      encode_NSInteger(aCoder, &_tag);
      [aCoder encodeObject: _cell];
      [aCoder encodeValueOfObjCType: @encode(BOOL) at: &_ignoresMultiClick];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (self == nil)
    {
      return nil;
    }

  if ([aDecoder allowsKeyedCoding])
    {
      NSCell *cell = [aDecoder decodeObjectForKey: @"NSCell"];
      
      if (cell != nil)
        {
          [self setCell: cell];
        }
      else
        {
          // This is needed for subclasses without cells, like NSColorWeel
          // as we store some properties only on the cell.
          cell = [[[self class] cellClass] new];

          [self setCell: cell];
          RELEASE(cell);
        }
      if ([aDecoder containsValueForKey: @"NSEnabled"])
        {
          // Don't use this information as it also comes from the cell
          // and NSComboBox has always YES here, even when disabled
          //[self setEnabled: [aDecoder decodeBoolForKey: @"NSEnabled"]];
        }
      if ([aDecoder containsValueForKey: @"NSTag"])
        {
          int tag = [aDecoder decodeIntForKey: @"NSTag"];
          [self setTag: tag];
        }
    }
  else 
    {
      decode_NSInteger(aDecoder, &_tag);
      [aDecoder decodeValueOfObjCType: @encode(id) at: &_cell];
      [aDecoder decodeValueOfObjCType: @encode(BOOL) at: &_ignoresMultiClick];
    }

  return self;
}

- (void) bind: (NSString *)binding
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ([binding isEqual: NSValueBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueBinding alloc] initWithBinding: @"objectValue"
                                              withName: NSValueBinding
                                              toObject: anObject
                                           withKeyPath: keyPath
                                               options: options
                                            fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else if ([binding hasPrefix: NSEnabledBinding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueAndBinding alloc] initWithBinding: NSEnabledBinding 
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

- (void) setValue: (id)anObject forKey: (NSString*)aKey
{
  if ([aKey isEqual: NSFontNameBinding])
    {
      [self setFont: [[NSFontManager sharedFontManager] convertFont: [self font] 
                                                             toFace: anObject]];
    }
  else if ([aKey isEqual: NSFontSizeBinding])
    {
      [self setFont: [[NSFontManager sharedFontManager] convertFont: [self font]
                                                             toSize: [anObject doubleValue]]];
    }
  else
    {
      [super setValue: anObject forKey: aKey];
    }
}

- (id) valueForKey: (NSString*)aKey
{
  if ([aKey isEqual: NSFontNameBinding])
    {
      return [[self font] fontName];
    }
  else if ([aKey isEqual: NSFontSizeBinding])
    {
      return [NSNumber numberWithDouble: (double)[[self font] pointSize]];
    }
  else
    {
      return [super valueForKey: aKey];
    }
}

- (NSSize) sizeThatFits: (NSSize)size
{
  // FIXME: This is a stub
  return size;
}

@end

@implementation NSControl(KeyViewLoop)

- (void) _setUpKeyViewLoopWithNextKeyView: (NSView *)nextKeyView
{
  // Controls are expected to have no subviews
  //NSLog(@"%@@%p -_setUpKeyViewLoopWithKeyKeyView:%@@%p", [self class], self, [nextKeyView class], nextKeyView);
  [self setNextKeyView: nextKeyView];
}

@end
