/** <title>NSTextView</title>

   <abstract>Categories which add user actions to NSTextView</abstract>

   Copyright (C) 1996, 1998, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   Originally moved here from NSTextView.m.

   Author: Scott Christley <scottc@net-community.com>
   Date: 1996

   Author: Felipe A. Rodriguez <far@ix.netcom.com>
   Date: July 1998

   Author: Daniel Böhringer <boehring@biomed.ruhr-uni-bochum.de>
   Date: August 1998

   Author: Fred Kiefer <FredKiefer@gmx.de>
   Date: March 2000, September 2000

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: 2000, 2001, 2002

   Author: Pierre-Yves Rivaille <pyrivail@ens-lyon.fr>
   Date: September 2002

   Extensive reworking: Alexander Malmberg <alexander@malmberg.org>
   Date: December 2002 - February 2003

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

#import <Foundation/NSNotification.h>
#import <Foundation/NSValue.h>
#import "AppKit/NSAttributedString.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSLayoutManager.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTextStorage.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSParagraphStyle.h"

/*
These methods are for user actions, ie. they are normally called from
-doCommandBySelector: (which is called by the input manager) in response
to some key press or other user event.

User actions that modify the text must check that a modification is allowed
and make sure all necessary notifications are sent. This is done by sending
-shouldChangeTextInRange:replacementString: before making any changes, and
(if the change is allowed) -didChangeText after the changes have been made.

All actions from NSResponder that make sense for a text view  should be
implemented here, but this is _not_ the place to add new actions.

When changing attributes, the range returned by
rangeForUserCharacterAttributeChange or rangeForUserParagraphAttributeChange
should be used. If the location is NSNotFound, nothing should be done (in
particular, the typing attributes should _not_ be changed). Otherwise,
-shouldChangeTextInRange:replacementString: should be called, and if it
returns YES, the attributes of the range and the typing attributes should be
changed, and -didChangeText should be called.

In a non-rich-text text view, the typing attributes _must_always_ hold the
attributes of the text. Thus, the typing attributes must always be changed
in the same way that the attributes of the text are changed.

TODO: can the selected range's location be NSNotFound? when?

Not all user actions are here. Exceptions:

  -copy:
  -copyFont:
  -copyRuler:
  -paste:
  -pasteFont:
  -pasteRuler:
  -pasteAsPlainText:
  -pasteAsRichText:

  -checkSpelling:
  -showGuessPanel:

  -selectAll: (implemented in NSText)

Not all methods that handle user-induced text modifications are here.
Exceptions:
  (TODO)

  -insertText:
  -changeColor:
  -changeFont: (action method?)
  drag&drop handling methods
  (others?)

All other methods that modify text are for programmatic changes and do not
send -shouldChangeTextInRange:replacementString: or -didChangeText.

*/

/* global kill buffer shared between all text views */
/* Note: I'm not using an attributed string here because Apple apparently is
   using a plain string either. Maybe this is because NeXT was using the X11
   cut buffer for the kill buffer, which can hold only plain strings? */
static NSString *killBuffer = @"";
  
/** First some helpers **/

@interface NSTextView (UserActionHelpers)

-(void) _illegalMovement: (int)textMovement;

-(void) _changeAttribute: (NSString *)name
		 inRange: (NSRange)r
		   using: (NSNumber*(*)(NSNumber*))func;

@end


@implementation NSTextView (UserActionHelpers)

- (void) _illegalMovement: (int)textMovement
{
  /* This is similar to [self resignFirstResponder], with the
     difference that in the notification we need to put the
     NSTextMovement, which resignFirstResponder does not.  Also, if we
     are ending editing, we are going to be removed, so it's useless
     to update any drawing.  Please note that this ends up calling
     resignFirstResponder anyway.  */
  NSNumber *number;
  NSDictionary *uiDictionary;

  if ((_tf.is_editable)
      && ([_delegate respondsToSelector:
		       @selector(textShouldEndEditing:)])
      && ([_delegate textShouldEndEditing: self] == NO))
    return;

  /* TODO: insertion point.
  doesn't the -resignFirstResponder take care of that?
  */

  number = [NSNumber numberWithInt: textMovement];
  uiDictionary = [NSDictionary dictionaryWithObject: number
			       forKey: @"NSTextMovement"];
  [[NSNotificationCenter defaultCenter]
    postNotificationName: NSTextDidEndEditingNotification
		  object: self
		userInfo: uiDictionary];
  /* The TextField will get the notification, and drop our first responder
   * status if it's the case ... in that case, our -resignFirstResponder will
   * be called!  */
  return;
}


- (void) _changeAttribute: (NSString *)name
		  inRange: (NSRange)r
		    using: (NSNumber*(*)(NSNumber*))func
{
  NSUInteger i;
  NSRange e, r2;
  id current, new;

  if (![self shouldChangeTextInRange: r replacementString: nil])
    return;

  [_textStorage beginEditing];
  for (i = r.location; i < NSMaxRange(r);)
    {
      current = [_textStorage attribute: name
				atIndex: i
			 effectiveRange: &e];

      r2 = NSMakeRange(i, NSMaxRange(e) - i);
      r2 = NSIntersectionRange(r2, r);
      i = NSMaxRange(e);

      new = func(current);
      if (new != current)
	{
	  if (!new)
	    {
	      [_textStorage removeAttribute: name
				      range: r2];
	    }
	  else
	    {
	      [_textStorage addAttribute: name
				   value: new
				   range: r2];
	    }
	}
    }
  [_textStorage endEditing];

  current = [_layoutManager->_typingAttributes objectForKey: name];
  new = func(current);
  if (new != current)
    {
      if (!new)
	{
	  [_layoutManager->_typingAttributes removeObjectForKey: name];
	}
      else
	{
	  [_layoutManager->_typingAttributes setObject: new  forKey: name];
	}
    }

  [self didChangeText];
}

@end


@implementation NSTextView (UserActions)

/* Helpers used with _changeAttribute:inRange:using:. */
static NSNumber *int_minus_one(NSNumber *cur)
{
  int value;

  if (cur)
    value = [cur intValue] - 1;
  else
    value = -1;

  if (value)
    return [NSNumber numberWithInt: value];
  else
    return nil;
}

static NSNumber *int_plus_one(NSNumber *cur)
{
  int value;

  if (cur)
    value = [cur intValue] + 1;
  else
    value = 1;

  if (value)
    return [NSNumber numberWithInt: value];
  else
    return nil;
}

static NSNumber *float_minus_one(NSNumber *cur)
{
  float value;

  if (cur)
    value = [cur floatValue] - 1;
  else
    value = -1;

  if (value)
    return [NSNumber numberWithFloat: value];
  else
    return nil;
}

static NSNumber *float_plus_one(NSNumber *cur)
{
  int value;

  if (cur)
    value = [cur floatValue] + 1;
  else
    value = 1;

  if (value)
    return [NSNumber numberWithFloat: value];
  else
    return nil;
}


- (void) subscript: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSSuperscriptAttributeName
		 inRange: r
		   using: int_minus_one];
}

- (void) superscript: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSSuperscriptAttributeName
		 inRange: r
		   using: int_plus_one];
}

- (void) lowerBaseline: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSBaselineOffsetAttributeName
		 inRange: r
		   using: float_plus_one];
}

- (void) raiseBaseline: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSBaselineOffsetAttributeName
		 inRange: r
		   using: float_minus_one];
}

- (void) unscript: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange
		   replacementString: nil])
    return;

  if (aRange.length)
    {
      [_textStorage beginEditing];
      [_textStorage removeAttribute: NSSuperscriptAttributeName
			      range: aRange];
      [_textStorage removeAttribute: NSBaselineOffsetAttributeName
			      range: aRange];
      [_textStorage endEditing];
    }

  [_layoutManager->_typingAttributes removeObjectForKey: NSSuperscriptAttributeName];
  [_layoutManager->_typingAttributes removeObjectForKey: NSBaselineOffsetAttributeName];
  [self didChangeText];
}


- (void) underline: (id)sender
{
  BOOL doUnderline = YES;
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;

  if ([[_textStorage attribute: NSUnderlineStyleAttributeName
		     atIndex: aRange.location
		     effectiveRange: NULL] intValue])
    doUnderline = NO;

  if (aRange.length)
    {
      if (![self shouldChangeTextInRange: aRange
		 replacementString: nil])
	return;
      [_textStorage beginEditing];
      [_textStorage addAttribute: NSUnderlineStyleAttributeName
		    value: [NSNumber numberWithInt: doUnderline]
		    range: aRange];
      [_textStorage endEditing];
      [self didChangeText];
    }

  [_layoutManager->_typingAttributes
      setObject: [NSNumber numberWithInt: doUnderline]
      forKey: NSUnderlineStyleAttributeName];
}


- (void) useStandardKerning: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;
  if (![self shouldChangeTextInRange: aRange
	    replacementString: nil])
    return;

  [_textStorage removeAttribute: NSKernAttributeName
		range: aRange];
  [_layoutManager->_typingAttributes removeObjectForKey: NSKernAttributeName];
  [self didChangeText];
}

- (void) turnOffKerning: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;
  if (![self shouldChangeTextInRange: aRange
	    replacementString: nil])
    return;

  [_textStorage addAttribute: NSKernAttributeName
		value: [NSNumber numberWithFloat: 0.0]
		range: aRange];
  [_layoutManager->_typingAttributes setObject: [NSNumber numberWithFloat: 0.0]
    forKey: NSKernAttributeName];
  [self didChangeText];
}

- (void) loosenKerning: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSKernAttributeName
		 inRange: r
		   using: float_plus_one];
}

- (void) tightenKerning: (id)sender
{
  NSRange r = [self rangeForUserCharacterAttributeChange];

  if (r.location == NSNotFound)
    return;

  [self _changeAttribute: NSKernAttributeName
		 inRange: r
		   using: float_minus_one];
}

- (void) turnOffLigatures: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange
	    replacementString: nil])
    return;
  [_textStorage addAttribute: NSLigatureAttributeName
		value: [NSNumber numberWithInt: 0]
		range: aRange];
  [_layoutManager->_typingAttributes setObject: [NSNumber numberWithInt: 0]
    forKey: NSLigatureAttributeName];
  [self didChangeText];
}

- (void) useStandardLigatures: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange
	    replacementString: nil])
    return;
    
  [_textStorage removeAttribute: NSLigatureAttributeName
		range: aRange];
  [_layoutManager->_typingAttributes removeObjectForKey: NSLigatureAttributeName];
  [self didChangeText];
}

- (void) useAllLigatures: (id)sender
{
  NSRange aRange = [self rangeForUserCharacterAttributeChange];

  if (aRange.location == NSNotFound)
    return;

  if (![self shouldChangeTextInRange: aRange
	    replacementString: nil])
    return;
  [_textStorage addAttribute: NSLigatureAttributeName
		value: [NSNumber numberWithInt: 2]
		range: aRange];
  [_layoutManager->_typingAttributes setObject: [NSNumber numberWithInt: 2]
    forKey: NSLigatureAttributeName];
  [self didChangeText];
}

- (void) toggleTraditionalCharacterShape: (id)sender
{
  // TODO
  NSLog(@"Method %s is not implemented for class %s",
	"toggleTraditionalCharacterShape:", "NSTextView");
}


- (void) insertNewline: (id)sender
{
  if (_tf.is_field_editor)
    {
      [self _illegalMovement: NSReturnTextMovement];
      return;
    }

  [self insertText: @"\n"];
}

- (void) insertTab: (id)sender
{
  if (_tf.is_field_editor)
    {
      [self _illegalMovement: NSTabTextMovement];
      return;
    }

  [self insertText: @"\t"];
}

- (void) insertBacktab: (id)sender
{
  if (_tf.is_field_editor)
    {
      [self _illegalMovement: NSBacktabTextMovement];
      return;
    }

  /* TODO */
  //[self insertText: @"\t"];
}

- (void) insertNewlineIgnoringFieldEditor: (id)sender
{
  [self insertText: @"\n"];
}

- (void) insertTabIgnoringFieldEditor: (id)sender
{
  [self insertText: @"\t"];
}

- (void) insertContainerBreak: (id)sender
{
  unichar ch = NSFormFeedCharacter;
  [self insertText: [NSString stringWithCharacters: &ch length: 1]];
}

- (void) insertLineBreak: (id)sender
{
  unichar ch = NSLineSeparatorCharacter;
  [self insertText: [NSString stringWithCharacters: &ch length: 1]];
}

- (void) deleteForward: (id)sender
{
  NSRange range = [self rangeForUserTextChange];
  NSDictionary *attributes;
  
  if (range.location == NSNotFound)
    {
      return;
    }
  
  /* Manage case of insertion point - implicitly means to delete following 
     character */
  if (range.length == 0)
    {
      if (range.location != [_textStorage length])
	{
	  /* Not at the end of text -- delete following character */
	  range.length = 1;
	}
      else
	{
	  /* At the end of text - TODO: Make beeping or not beeping
	     configurable vie User Defaults */
	  NSBeep ();
	  return;
	}
    }
  else if ([self smartInsertDeleteEnabled] &&
	   [self selectionGranularity] == NSSelectByWord)
    {
      range = [self smartDeleteRangeForProposedRange: range];
    }
  
  if (![self shouldChangeTextInRange: range  replacementString: @""])
    {
      return;
    }

  attributes = RETAIN([_textStorage attributesAtIndex: range.location
				       effectiveRange: NULL]);
  [_textStorage beginEditing];
  [_textStorage deleteCharactersInRange: range];
  [_textStorage endEditing];
  [self setTypingAttributes: attributes];
  RELEASE(attributes);
  [self didChangeText];
}

- (void) deleteBackward: (id)sender
{
  NSRange range = [self rangeForUserTextChange];
  NSDictionary *attributes;
  
  if (range.location == NSNotFound)
    {
      return;
    }
  
  /* Manage case of insertion point - implicitly means to delete
     previous character */
  if (range.length == 0)
    {
      if (range.location != 0)
	{
	  /* Not at the beginning of text -- delete previous character */
	  range.location -= 1;
	  range.length = 1;
	}
      else
	{
	  /* At the beginning of text - TODO: Make beeping or not
	     beeping configurable via User Defaults */
	  NSBeep ();
	  return;
	}
    }
  else if ([self smartInsertDeleteEnabled] &&
	   [self selectionGranularity] == NSSelectByWord)
    {
      range = [self smartDeleteRangeForProposedRange: range];
    }
  
  if (![self shouldChangeTextInRange: range  replacementString: @""])
    {
      return;
    }

  attributes = RETAIN([_textStorage attributesAtIndex: range.location
				       effectiveRange: NULL]);
  [_textStorage beginEditing];
  [_textStorage deleteCharactersInRange: range];
  [_textStorage endEditing];
  [self setTypingAttributes: attributes];
  RELEASE(attributes);
  [self didChangeText];
}

- (void) deleteToEndOfLine: (id)sender
{
  NSRange range = [self rangeForUserTextChange];
  NSDictionary *attributes;

  if (range.location == NSNotFound)
    {
      return;
    }

  /* If the selection is not empty delete it, otherwise delete up to the
     next line end from the insertion point or the delete the line end
     itself when the insertion point is already at the end of the line. */
  if (range.length == 0)
    {
      NSUInteger start, end, contentsEnd;

      [[_textStorage string] getLineStart: &start
				      end: &end
			      contentsEnd: &contentsEnd
				 forRange: range];
      if (range.location == contentsEnd)
	{
	  range = NSMakeRange(contentsEnd, end - contentsEnd);
	}
      else
	{
	  range.length = contentsEnd - range.location;
	}
      if (range.length == 0)
	{
	  return;
	}
    }

  if (![self shouldChangeTextInRange: range  replacementString: @""])
    {
      return;
    }

  ASSIGN(killBuffer, [[_textStorage string] substringWithRange: range]);
  attributes = RETAIN([_textStorage attributesAtIndex: range.location
				       effectiveRange: NULL]);
  [_textStorage beginEditing];
  [_textStorage deleteCharactersInRange: range];
  [_textStorage endEditing];
  [self setTypingAttributes: attributes];
  RELEASE(attributes);
  [self didChangeText];
}

- (void) deleteToEndOfParagraph: (id)sender
{
  NSRange range = [self rangeForUserTextChange];
  NSDictionary *attributes;

  if (range.location == NSNotFound)
    {
      return;
    }

  /* If the selection is not empty delete it, otherwise delete up to
     the next paragraph end from the insertion point or the delete the
     paragraph end itself when the insertion point is already at the
     end of the paragraph. */
  if (range.length == 0)
    {
      NSUInteger start, end, contentsEnd;

      [[_textStorage string] getParagraphStart: &start
					   end: &end
				   contentsEnd: &contentsEnd
				      forRange: range];
      if (range.location == contentsEnd)
	{
	  range = NSMakeRange(contentsEnd, end - contentsEnd);
	}
      else
	{
	  range.length = contentsEnd - range.location;
	}
      if (range.length == 0)
	{
	  return;
	}
    }

  if (![self shouldChangeTextInRange: range  replacementString: @""])
    {
      return;
    }

  ASSIGN(killBuffer, [[_textStorage string] substringWithRange: range]);
  attributes = RETAIN([_textStorage attributesAtIndex: range.location
				       effectiveRange: NULL]);
  [_textStorage beginEditing];
  [_textStorage deleteCharactersInRange: range];
  [_textStorage endEditing];
  [self setTypingAttributes: attributes];
  RELEASE(attributes);
  [self didChangeText];
}

- (void) yank: (id)sender
{
  if ([killBuffer length] > 0)
    {
      [self insertText: killBuffer];
    }
}


/*
TODO: find out what affinity is supposed to mean

My current assumption:

Affinity deals with which direction we are selecting in, ie. which end of
the selected range is the moving end, and which is the anchor.

NSSelectionAffinityUpstream means that the minimum index of the selected
range is moving (ie. _selected_range.location).

NSSelectionAffinityDownstream means that the maximum index of the selected
range is moving (ie. _selected_range.location+_selected_range.length).

Thus, when moving and selecting, we use the affinity to find out which end
of the selected range to move, and after moving, we compare the character
index we moved to with the anchor and set the range and affinity.


The affinity is important when making keyboard selection have sensible
behavior. Example:

If, in the string "abcd", the insertion point is between the "c" and the "d"
(selected range is (3,0)), and the user hits shift-left twice, we select
the "c" and "b" (1,2) and set the affinity to NSSelectionAffinityUpstream.
If the user hits shift-right, only the "c" will be selected (2,1).

If the insertion point is between the "a" and the "b" (1,0) and the user hits
shift-right twice, we again select the "b" and "c" (1,2), but the affinity
is NSSelectionAffinityDownstream. If the user hits shift-right, the "d" is
added to the selection (1,3).

*/

- (unsigned int) _movementOrigin
{
  NSRange range = [self selectedRange];

  if ([self selectionAffinity] == NSSelectionAffinityUpstream)
    return range.location;
  else
    return NSMaxRange(range);
}

- (NSUInteger) _movementEnd
{
  NSRange range = [self selectedRange];

  if ([self selectionAffinity] == NSSelectionAffinityDownstream)
    return range.location;
  else
    return NSMaxRange(range);
}

- (void) _moveTo: (NSUInteger)cindex
	  select: (BOOL)select
{
  if (select)
    {
      NSUInteger anchor = [self _movementEnd];

      if (anchor < cindex)
	{
	  [self setSelectedRange: NSMakeRange(anchor, cindex - anchor)
			affinity: NSSelectionAffinityDownstream
		  stillSelecting: NO];
	}
      else
 	{
	  [self setSelectedRange: NSMakeRange(cindex, anchor - cindex)
			affinity: NSSelectionAffinityUpstream
		  stillSelecting: NO];
	}
    }
  else
    {
      [self setSelectedRange: NSMakeRange(cindex, 0)];
    }
  [self scrollRangeToVisible: NSMakeRange(cindex, 0)];
}

- (void) _moveFrom: (NSUInteger)cindex
	 direction: (GSInsertionPointMovementDirection)direction
          distance: (CGFloat)distance
	    select: (BOOL)select
{
  int new_direction;

  if (direction == GSInsertionPointMoveUp ||
      direction == GSInsertionPointMoveDown)
    {
      new_direction = 2;
    }
  else if (direction == GSInsertionPointMoveLeft ||
	   direction == GSInsertionPointMoveRight)
    {
      new_direction = 1;
    }
  else
    {
      new_direction = 0;
    }

  if (new_direction != _currentInsertionPointMovementDirection ||
      !new_direction)
    {
      _originalInsertionPointCharacterIndex = cindex;
    }

  cindex = [_layoutManager characterIndexMoving: direction
	      fromCharacterIndex: cindex
	      originalCharacterIndex: _originalInsertionPointCharacterIndex
	      distance: distance];
  [self _moveTo: cindex
	 select: select];
  /* Setting the selected range will clear out the current direction, but
  not the index. Thus, we always set the direction here. */
  _currentInsertionPointMovementDirection = new_direction;
}

- (void) _move: (GSInsertionPointMovementDirection)direction
      distance: (CGFloat)distance
	select: (BOOL)select
{
  [self _moveFrom: [self _movementOrigin]
	direction: direction
	 distance: distance
	   select: select];
}


/*
 * returns the character index for the left or right side of the selected text
 * based upon the writing direction of the paragraph style.
 * it should only be used when moving a literal direction such as left right
 * up or down, not directions like forward, backward, beginning or end
 */
- (NSUInteger) _characterIndexForSelectedRange: (NSRange)range
       direction: (GSInsertionPointMovementDirection)direction
{
  NSUInteger cIndex;
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  switch (writingDirection)
    {
      case NSWritingDirectionLeftToRight:
        cIndex = (direction == GSInsertionPointMoveLeft
		  || direction == GSInsertionPointMoveUp)
	         ? range.location
		 : NSMaxRange(range);
        break;
      case NSWritingDirectionRightToLeft:
        cIndex = (direction == GSInsertionPointMoveLeft
		  || direction == GSInsertionPointMoveUp)
        	 ? NSMaxRange(range)
		 : range.location;
        break;
      case NSWritingDirectionNaturalDirection:
        // not sure if we should see this as it should resolve to either
        // LeftToRight or RightToLeft in NSParagraphStyle
        // for the users language.
	//
	// currently falls back to default..
      default:
	/* default to LeftToRight */
        cIndex = (direction == GSInsertionPointMoveLeft
		  || direction == GSInsertionPointMoveUp)
	         ? range.location
		 : NSMaxRange(range);
        break;
     }
  return cIndex;
}

/*
Insertion point movement actions.

TODO: some of these used to do nothing if self is a field editor. should
check if there was a reason for that.
*/

- (void) moveUp: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger cIndex = [self _characterIndexForSelectedRange:range
	  			direction:GSInsertionPointMoveUp];
  
  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveUp
	distance: 0.0
	select: NO];
}

- (void) moveUpAndModifySelection: (id)sender
{
  [self _move: GSInsertionPointMoveUp
	distance: 0.0
	select: YES];
}

- (void) moveDown: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger cIndex = [self _characterIndexForSelectedRange: range
	  			direction: GSInsertionPointMoveDown];
  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveDown
	distance: 0.0
	select: NO];
}

- (void) moveDownAndModifySelection: (id)sender
{
  [self _move: GSInsertionPointMoveDown
	distance: 0.0
	select: YES];
}

- (void) moveLeft: (id)sender
{
  NSRange range = [self selectedRange];

  if (range.length)
    {
      NSUInteger cIndex;
      
      cIndex = [self _characterIndexForSelectedRange: range
	      		direction:GSInsertionPointMoveLeft];
      [self _moveTo: cIndex select: NO];
    }
  else 
    {
      [self _move: GSInsertionPointMoveLeft
 	    distance: 0.0
	    select: NO];
    }
}

- (void) moveLeftAndModifySelection: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
        [self moveForwardAndModifySelection: sender];
    }
  else
    {
        [self moveBackwardAndModifySelection: sender];
    }
}

- (void) moveRight: (id)sender
{
  NSRange range = [self selectedRange];
  
  if (range.length)
    {
      NSUInteger cIndex;
      
      cIndex = [self _characterIndexForSelectedRange: range
	       		direction: GSInsertionPointMoveRight];
      [self _moveTo: cIndex select: NO];
    }
  else
    {
      [self _move: GSInsertionPointMoveRight
 	    distance: 0.0
	    select: NO];
    }
}

- (void) moveRightAndModifySelection: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
      [self moveBackwardAndModifySelection: sender];
    }
  else
    {
      [self moveForwardAndModifySelection: sender];
    }
}

- (void) moveBackward: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger to = range.location;

  if (range.length == 0 && to)
    {
      to--;
    }
  
  [self _moveTo: to
	 select: NO];
}

- (void) moveBackwardAndModifySelection: (id)sender
{
  NSUInteger to = [self _movementOrigin];

  if (to == 0)
    return;
  to--;
  [self _moveTo: to
	 select: YES];
}

- (void) moveForward: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger to = NSMaxRange(range);
  
  if (range.length == 0 && to != [_textStorage length])
    {
      to++;
    }
  
  [self _moveTo: to
	 select: NO];
}

- (void) moveForwardAndModifySelection: (id)sender
{
  NSUInteger to = [self _movementOrigin];

  if (to == [_textStorage length])
    return;
  to++;
  [self _moveTo: to
	 select: YES];
}

- (void) moveWordBackward: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger newLocation;
  NSUInteger cIndex = range.location;
  
  newLocation = [_textStorage nextWordFromIndex: cIndex
			      forward: NO];
  [self _moveTo: newLocation
	 select: NO];
}

- (void) moveWordBackwardAndModifySelection: (id)sender
{
  NSUInteger newLocation;
  
  newLocation = [_textStorage nextWordFromIndex: [self _movementOrigin]
			      forward: NO];
  [self _moveTo: newLocation
	 select: YES];
}

- (void) moveWordForward: (id)sender
{
  NSUInteger newLocation;
  NSUInteger cIndex = NSMaxRange([self selectedRange]);

  newLocation = [_textStorage nextWordFromIndex: cIndex
			      forward: YES];
  [self _moveTo: newLocation
	 select: NO];
}

- (void) moveWordForwardAndModifySelection: (id)sender
{
  NSUInteger newLocation;

  newLocation = [_textStorage nextWordFromIndex: [self _movementOrigin]
			      forward: YES];
  [self _moveTo: newLocation
	 select: YES];
}

- (void) moveWordLeft: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
        [self moveWordForward: sender];
    }
  else
    {
        [self moveWordBackward: sender];
    }
}

- (void) moveWordLeftAndModifySelection: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
        [self moveWordForwardAndModifySelection: sender];
    }
  else
    {
        [self moveWordBackwardAndModifySelection: sender];
    }
}

- (void) moveWordRight: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
      [self moveWordBackward: sender];
    }
  else
    {
      [self moveWordForward: sender];
    }
}

- (void) moveWordRightAndModifySelection: (id)sender
{
  NSParagraphStyle *parStyle;
  NSWritingDirection writingDirection;
  
  parStyle = [[self typingAttributes]
	  	objectForKey: NSParagraphStyleAttributeName];
  writingDirection = [parStyle baseWritingDirection];
  
  if (writingDirection == NSWritingDirectionRightToLeft)
    {
      [self moveWordBackwardAndModifySelection: sender];
    }
  else
    {
      [self moveWordForwardAndModifySelection: sender];
    }
}

- (void) moveToBeginningOfDocument: (id)sender
{
  [self _moveTo: 0
	 select: NO];
}

- (void) moveToBeginningOfDocumentAndModifySelection: (id)sender
{
  [self _moveTo: 0
	  select:YES];
}

- (void) moveToEndOfDocument: (id)sender
{
  [self _moveTo: [_textStorage length]
	 select: NO];
}

- (void) moveToEndOfDocumentAndModifySelection: (id)sender
{
  [self _moveTo: [_textStorage length]
	  select:YES];
}

- (void) moveToBeginningOfParagraph: (id)sender
{
  NSRange aRange = [self selectedRange];
 
  aRange = [[_textStorage string] lineRangeForRange: 
				      NSMakeRange(aRange.location, 0)];
  [self _moveTo: aRange.location
	 select: NO];
}

- (void) moveToBeginningOfParagraphAndModifySelection: (id)sender
{
  NSRange aRange;
  
  aRange = [[_textStorage string] lineRangeForRange: 
				      NSMakeRange([self _movementOrigin], 0)];
  [self _moveTo: aRange.location
	 select: YES];
}

- (void) _moveToEndOfParagraph: (id)sender modify:(BOOL)flag
{
  NSRange aRange;
  NSUInteger newLocation;
  NSUInteger maxRange;
  NSUInteger cIndex;

  if (flag)
    {
      cIndex = [self _movementOrigin];
    }
  else
    {
      cIndex = NSMaxRange([self selectedRange]);
    }

  aRange = [[_textStorage string] lineRangeForRange: 
				      NSMakeRange(cIndex, 0)];
  maxRange = NSMaxRange (aRange);

  if (maxRange == 0)
    {
      /* Beginning of text is special only for technical reasons -
	 since maxRange is an unsigned, we can't safely subtract 1
	 from it if it is 0.  */
      newLocation = maxRange;
    }
  else if (maxRange == [_textStorage length])
    {
      /* End of text is special - we want the insertion point to
	 appear *after* the last character, which means as if before
	 the next (virtual) character after the end of text ... unless
	 the last character is a newline, and we are trying to go to
	 the end of the line which is displayed as the
	 one-before-the-last.  Please note (maxRange - 1) is a valid
	 char since the maxRange == 0 case has already been
	 eliminated.  */
      unichar u = [[_textStorage string] characterAtIndex: (maxRange - 1)];
      if (u == '\n'  ||  u == '\r')
	{
	  newLocation = maxRange - 1;
	}
      else
	{
	  newLocation = maxRange;
	}
    }
  else
    {
      /* Else, we want the insertion point to appear before the last
	 character in the paragraph range.  Normally the last
	 character in the paragraph range is a newline.  */
      newLocation = maxRange - 1;
    }

  if (newLocation < aRange.location)
    {
      newLocation = aRange.location;
    }

  [self _moveTo: newLocation
	 select: flag];
}

- (void) moveToEndOfParagraph: (id)sender
{
  [self _moveToEndOfParagraph:sender modify:NO];  
}

- (void) moveToEndOfParagraphAndModifySelection: (id)sender
{
  [self _moveToEndOfParagraph:sender modify:YES];  
}

/* TODO: this is only the beginning and end of lines if lines are horizontal
and layout is left-to-right */
- (void) moveToBeginningOfLine: (id)sender
{
  NSRange range = [self selectedRange];
  NSUInteger cIndex = range.location;

  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveLeft
	distance: 1e8
	select: NO];
}

- (void) moveToBeginningOfLineAndModifySelection: (id)sender
{
  [self _move: GSInsertionPointMoveLeft
	distance: 1e8
	select: YES];
}

- (void) moveToEndOfLine: (id)sender
{
  NSUInteger cIndex = NSMaxRange([self selectedRange]);

  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveRight
	distance: 1e8
	select: NO];
}

- (void) moveToEndOfLineAndModifySelection: (id)sender
{
  [self _move: GSInsertionPointMoveRight
	distance: 1e8
	select: YES];
}

/**
 * Tries to move the selection/insertion point down one page of the
 * visible rect in the receiver while trying to maintain the
 * horizontal position of the last vertical movement.
 * If the receiver is a field editor, this method returns immediatly. 
 */
- (void) _pageDown: (id)sender modify: (BOOL)flag
{
  CGFloat    scrollDelta;
  CGFloat    oldOriginY;
  CGFloat    newOriginY;
  NSUInteger cIndex;
  
  if (flag)
    {
      cIndex = [self _movementOrigin];
    }
  else
    {
      cIndex = [self _characterIndexForSelectedRange: [self selectedRange]
	      		direction: GSInsertionPointMoveDown];
    }

  /*
   * Scroll; also determine how far to move the insertion point.
   */
  oldOriginY = NSMinY([self visibleRect]);
  [[self enclosingScrollView] pageDown: sender];
  newOriginY = NSMinY([self visibleRect]);
  scrollDelta = newOriginY - oldOriginY;

  if (scrollDelta == 0)
    {
      [self _moveTo:[_textStorage length] select:flag];
      return;
    }

  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveDown
	distance: scrollDelta
	select: flag];
}

- (void) pageDown:(id)sender
{
  [self _pageDown:sender modify:NO];
}

- (void) pageDownAndModifySelection:(id)sender
{
  [self _pageDown:sender modify:YES];
}

/**
 * Tries to move the selection/insertion point up one page of the
 * visible rect in the receiver while trying to maintain the
 * horizontal position of the last vertical movement.
 * If the receiver is a field editor, this method returns immediatly. 
 */
- (void) _pageUp: (id)sender modify:(BOOL)flag
{
  CGFloat    scrollDelta;
  CGFloat    oldOriginY;
  CGFloat    newOriginY;
  NSUInteger cIndex;
  
  if (flag)
    {
      cIndex = [self _movementOrigin];
    }
  else
    {
      cIndex = [self _characterIndexForSelectedRange:[self selectedRange]
		        direction: GSInsertionPointMoveUp];
    }
  /*
   * Scroll; also determine how far to move the insertion point.
   */
  oldOriginY = NSMinY([self visibleRect]);
  [[self enclosingScrollView] pageUp: sender];
  newOriginY = NSMinY([self visibleRect]);
  scrollDelta = newOriginY - oldOriginY;

  if (scrollDelta == 0)
    {
      [self _moveTo:0 select:flag];
      return; 
    }

  [self _moveFrom: cIndex
	direction: GSInsertionPointMoveUp
	distance: -scrollDelta
	select: flag];
}

- (void) pageUp:(id)sender
{
  [self _pageUp:sender modify:NO];
}

- (void) pageUpAndModifySelection:(id)sender
{
  [self _pageUp:sender modify:YES];
}

- (void) scrollLineDown: (id)sender
{
  [[self enclosingScrollView] scrollLineDown: sender];
}

- (void) scrollLineUp: (id)sender
{
  [[self enclosingScrollView] scrollLineUp: sender];
}

- (void) scrollPageDown: (id)sender
{
  [[self enclosingScrollView] scrollPageDown: sender];
}

- (void) scrollPageUp: (id)sender
{
  [[self enclosingScrollView] scrollPageUp: sender];
}

- (void) scrollToBeginningOfDocument: (id)sender
{
  [[self enclosingScrollView] scrollToBeginningOfDocument: sender];
}

- (void) scrollToEndOfDocument: (id)sender
{
  [[self enclosingScrollView] scrollToEndOfDocument: sender];
}

- (void) centerSelectionInVisibleArea: (id)sender
{
  NSRange range;
  NSPoint new;
  NSRect rect, vRect;

  vRect = [self visibleRect];
  range = [self selectedRange];
  if (range.length == 0)
    {
      rect =
          [_layoutManager insertionPointRectForCharacterIndex: range.location
                          inTextContainer: _textContainer];
    }
  else
    {
      range = [_layoutManager glyphRangeForCharacterRange: range
                              actualCharacterRange: NULL];
      rect = [_layoutManager boundingRectForGlyphRange: range
                             inTextContainer: _textContainer];
    }

  if (NSWidth(_bounds) <= NSWidth(vRect))
    new.x = 0;
  else if (NSWidth(rect) > NSWidth(vRect))
    new.x = NSMinX(rect);
  else
    new.x = NSMinX(rect) - (NSWidth(vRect) - NSWidth(rect)) / 2;

  if (NSHeight(_bounds) <= NSHeight(vRect))
    new.y = 0;
  else if (NSHeight(rect) > NSHeight(vRect))
    new.y = NSMinY(rect);
  else
    new.y = NSMinY(rect) - (NSHeight(vRect) - NSHeight(rect)) / 2;

  [self scrollPoint: new];
}


/* -selectAll: inherited from NSText  */

- (void) selectLine: (id)sender
{
  NSUInteger start, end, cindex;

  cindex = [self _movementOrigin];
  start = [_layoutManager characterIndexMoving: GSInsertionPointMoveLeft
	     fromCharacterIndex: cindex
	     originalCharacterIndex: cindex
	     distance: 1e8];
  end = [_layoutManager characterIndexMoving: GSInsertionPointMoveRight
	   fromCharacterIndex: cindex
	   originalCharacterIndex: cindex
	   distance: 1e8];
  [self setSelectedRange: NSMakeRange(start, end - start)];
}


/* The following method is bound to 'Control-t', and works exactly like
 * pressing 'Control-t' inside Emacs, i.e., in general it swaps the
 * character immediately before and after the insertion point and moves
 * the insertion point forward by one character.  If, however, the
 * insertion point is at the end of a line, it swaps the two characters
 * before the insertion point and does not move the insertion point.
 * Note that Mac OS X does not implement the special case at the end
 * of a line, but I consider Emacs' behavior more useful.
 */
- (void) transpose: (id)sender
{
  NSRange range = [self selectedRange];
  NSString *string;
  NSString *replacementString;
  unichar chars[2];

  /* Do nothing if the selection is not empty or if we are at the
   * beginning of text.  */
  if (range.length > 0 || range.location < 1)
    {
      return;
    }

  range = NSMakeRange(range.location - 1, 2);

  /* Eventually adjust the range if we are at the end of a line. */
  string = [_textStorage string];
  if (range.location + 1 == [string length]
      || [string characterAtIndex: range.location + 1] == '\n')
    {
      if (range.location == 0)
	return;
      range.location -= 1;
    }

  /* Get the two chars and swap them.  */
  chars[1] = [string characterAtIndex: range.location];
  chars[0] = [string characterAtIndex: (range.location + 1)];

  /* Replace the original chars with the swapped ones.  */
  replacementString = [NSString stringWithCharacters: chars  length: 2];

  if ([self shouldChangeTextInRange: range
	replacementString: replacementString])
    {
      [self replaceCharactersInRange: range
        withString: replacementString];
      [self setSelectedRange: NSMakeRange(range.location + 2, 0)];
      [self didChangeText];
    }
}

- (void) delete: (id)sender
{
  [self deleteForward: sender];
}


/* Helper for -align*: */
- (void) _alignUser: (NSTextAlignment)alignment
{
  NSRange r = [self rangeForUserParagraphAttributeChange];

  if (r.location == NSNotFound)
    return;
  if (![self shouldChangeTextInRange: r
	 replacementString: nil])
    return;

  [self setAlignment: alignment
    range: r];
  [self didChangeText];
}

- (void) alignCenter: (id)sender
{
  [self _alignUser: NSCenterTextAlignment];
}

- (void) alignLeft: (id)sender
{
  [self _alignUser: NSLeftTextAlignment];
}

- (void) alignRight: (id)sender
{
  [self _alignUser: NSRightTextAlignment];
}

- (void) alignJustified: (id)sender
{
  [self _alignUser: NSJustifiedTextAlignment];
}

- (void) toggleContinuousSpellChecking: (id)sender
{
  [self setContinuousSpellCheckingEnabled: 
	    ![self isContinuousSpellCheckingEnabled]];
}

- (void) toggleRuler: (id)sender
{
  [self setRulerVisible: !_tf.is_ruler_visible];
}

- (void) outline: (id)sender
{
  // FIXME
}

- (void) setBaseWritingDirection: (NSWritingDirection)direction
                           range: (NSRange)range
{
  if (!_tf.is_rich_text)
    return;

  [_textStorage setBaseWritingDirection: direction range: range];
}

- (void) toggleBaseWritingDirection: (id)sender
{
  // FIXME
}

@end
