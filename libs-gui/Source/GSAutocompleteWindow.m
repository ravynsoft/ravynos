/*
   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: German A. Arias <german@xelalug.org>
   Date: 2013

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

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSNotification.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSBox.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSScreen.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSText.h"
#import "AppKit/NSTextView.h"
#import "GNUstepGUI/GSTheme.h"
#import "GSAutocompleteWindow.h"

static GSAutocompleteWindow *gsWindow = nil;

@interface NSTextView (Additions)
- (NSRect) rectForCharacterRange: (NSRange)aRange;
@end

@interface GSAutocompleteView : NSTableView
{
}
@end

@implementation GSAutocompleteView
- (BOOL) acceptsFirstMouse: (NSEvent *)event
{
  return YES;
}
@end

@implementation GSAutocompleteWindow

+ (GSAutocompleteWindow *) defaultWindow
{
  if (gsWindow == nil)
    gsWindow = [[self alloc] initWithContentRect: NSMakeRect(0,0,200,200)
			               styleMask: NSBorderlessWindowMask
			                 backing: NSBackingStoreNonretained
			                   defer: YES];

  return gsWindow;
}

- (id) initWithContentRect: (NSRect)contentRect
		 styleMask: (NSUInteger)aStyle
		   backing: (NSBackingStoreType)bufferingType
		     defer: (BOOL)flag
{
  NSBox *box;
  NSScrollView *scrollView;
  NSTableColumn *column;
  NSCell *cell;
   
  self = [super initWithContentRect: contentRect
		          styleMask: aStyle
		            backing: bufferingType
		              defer: flag];	  
  if (nil == self)
    return self;

  // Init vars
  _words = nil;
  _originalWord = nil;

  [self setLevel: NSPopUpMenuWindowLevel];
  [self setBecomesKeyOnlyIfNeeded: YES];
  
  box = [[NSBox alloc] initWithFrame: contentRect];
  [box setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [box setBorderType: NSLineBorder];
  [box setTitlePosition: NSNoTitle];
  [box setContentViewMargins: NSMakeSize(0, 0)];
  [self setContentView: box];
  [box release];
  
  _tableView = [[GSAutocompleteView alloc] 
                     initWithFrame: NSMakeRect(0, 0, 100, 100)];
  [_tableView setAutoresizingMask: NSViewWidthSizable | NSViewHeightSizable];
  [_tableView setDrawsGrid: NO];
  [_tableView setAllowsEmptySelection: YES];
  [_tableView setAllowsMultipleSelection: NO];
  [_tableView setAutoresizesAllColumnsToFit: YES];
  [_tableView setHeaderView: nil];
  [_tableView setCornerView: nil];
  
  column = [[NSTableColumn alloc] initWithIdentifier: @"content"];
  cell = [[NSCell alloc] initTextCell: @""];
  [column setDataCell: cell];
  [cell release];
  [_tableView addTableColumn: column];
  [column release];
  
  [_tableView setDataSource: self];
  [_tableView setDelegate: self];
  [_tableView setAction: @selector(clickItem:)];
  [_tableView setTarget: self];
  
  scrollView = [[NSScrollView alloc] initWithFrame:
				       NSMakeRect(contentRect.origin.x, 
						  contentRect.origin.y,
						  contentRect.size.width, 
						  contentRect.size.height)];
  [scrollView setHasVerticalScroller: YES];
  [scrollView setDocumentView: _tableView];
  [_tableView release];
  [box setContentView: scrollView];
  [scrollView release];

  return self;
}

- (void) dealloc
{
  /* Don't release _words and _originalWord, since these are
   * released when the autocomplete is final or canceled.
   */
  [super dealloc];
}

- (BOOL) canBecomeKeyWindow 
{ 
  return YES; 
}

- (void) layout
{
  NSSize bsize = [[GSTheme theme] sizeForBorderType: NSLineBorder];
  CGFloat windowWidth, windowHeight;
  NSInteger num = [_words count];
  NSUInteger widest = 0;
  NSCell *cell;
  NSString *word, *widestWord = nil;
  NSEnumerator *enumerator;

  /* If the suggested words are more than 10,
   * we limit the window to show 10.
   */
  if (num > 10)
    {
      num = 10;
    }

  /* Lookup the widest word to calculate the width
   * of the window.
   */
  enumerator = [_words objectEnumerator];
  while ((word = [enumerator nextObject]))
    {
      if ([word length] > widest)
	{
	  widest = [word length];
	  widestWord = word;
	}
    }

  // Width
  cell = [[_tableView tableColumnWithIdentifier: @"content"] dataCell];
  windowWidth = 1.1*[cell _sizeText: widestWord].width
    + [NSScroller scrollerWidth] + 2*bsize.width;

  //Height
  windowHeight = 2*bsize.height + [_tableView rowHeight]*num
    + [_tableView intercellSpacing].height;

  [self setFrame: NSMakeRect(0, 0, windowWidth, windowHeight) display: NO];
}

- (void) computePosition
{
  NSRect  screenFrame;
  NSRect  rect;
  NSRect  stringRect;
  NSPoint point;
  NSInterfaceStyle style;

  style = NSInterfaceStyleForKey(@"NSScrollViewInterfaceStyle", nil);

  rect = [self frame];
  screenFrame = [[[_textView window] screen] frame];

  // Get the rectangle to draw the current word.
  stringRect = [_textView rectForCharacterRange: _range];

  // Convert the origin point to screen coordinates.
  point = [[_textView window] convertBaseToScreen:
	   [_textView convertRect: stringRect toView: nil].origin];

  // Calculate the origin point to the window.
  if (style == NSMacintoshInterfaceStyle
      || style == NSWindows95InterfaceStyle)
    {
      rect.origin.x = point.x - 4;
    }
  else
    {
      rect.origin.x = point.x - [NSScroller scrollerWidth] - 4;
    }
  rect.origin.y = point.y - rect.size.height;

  // If part of the window is off screen, change the origin point.
  if (screenFrame.size.width < (rect.origin.x + rect.size.width))
    {
      rect.origin.x = screenFrame.size.width - rect.size.width;
    }
  else if (rect.origin.x < 0)
    {
      rect.origin.x = 0;
    }

  // If no space under the string, we display the window over this.
  if (rect.origin.y < 0)
    {
      rect.origin.y = point.y + stringRect.size.height;
    }

  [self setFrame: rect display: NO];
}

- (NSArray *) words
{
  return _words;
}

- (void) displayForTextView: (NSTextView *)textView
{
  _textView = textView;
  _range = [_textView rangeForUserCompletion];
  [self reloadData];

  if ([_words count] > 0)
    {
      [self runModalWindow];
    }
  else
    {
      [self close];
    }
}

- (void) runModalWindow
{
  NSWindow	*onWindow;
  NSNotificationCenter *notificationCenter;

  onWindow = [_textView window];
  notificationCenter = [NSNotificationCenter defaultCenter];

  // Get the appropriate notifications to cancel the autocomplete.

  [notificationCenter addObserver: self selector: @selector(onWindowEdited:)
    name: NSWindowWillCloseNotification object: onWindow];
  [notificationCenter addObserver: self selector: @selector(onWindowEdited:)
    name: NSWindowWillMiniaturizeNotification object: onWindow];
  // The notification below don't seems to work.
  [notificationCenter addObserver: self selector: @selector(onWindowEdited:)
    name: NSWindowWillMoveNotification object: onWindow];
    
  // FIX ME: The notification below doesn't exist currently
  // [nc addObserver: self selector: @selector(onWindowEdited:) 
  //   name: NSWindowWillResizeNotification object: onWindow];
  
  // FIXME: The code below must be removed when the notifications over will work
  [notificationCenter addObserver: self selector: @selector(onWindowEdited:) 
    name: NSWindowDidMoveNotification object: onWindow];
  [notificationCenter addObserver: self selector: @selector(onWindowEdited:)
    name: NSWindowDidResizeNotification object: onWindow];
  // End of the code to remove
  
  [self orderFront: self];
  [self makeFirstResponder: _tableView];

  [self runLoop];
  
  [notificationCenter removeObserver: self name: nil object: onWindow];
  [self close];
  [onWindow makeFirstResponder: _textView];
}

- (void) runLoop
{
  NSEvent *event;
  NSDate *limit = [NSDate distantFuture];
  unichar key;
  CREATE_AUTORELEASE_POOL (pool);

  _stopped = NO;

  while (YES)
    {
      event = [NSApp nextEventMatchingMask: NSAnyEventMask
		                 untilDate: limit
		                    inMode: NSDefaultRunLoopMode
		                   dequeue: YES];

      if ([event type] == NSLeftMouseDown
	  || [event type] == NSRightMouseDown)
        {
          if ([event window] != self)
	    {
	      [self updateTextViewWithMovement: NSCancelTextMovement
				       isFinal: NO];
              break;
	    }
	  else
	    {
	      [NSApp sendEvent: event];
	    }
        }
      else if ([event type] == NSKeyDown)
        {
	  key = [[event characters] characterAtIndex: 0];

          if (key == NSUpArrowFunctionKey)
            {
              [self moveUpSelection];
	      [self updateTextViewWithMovement: NSUpTextMovement
				       isFinal: NO];
            }
          else if (key == NSDownArrowFunctionKey)
            {
              [self moveDownSelection];
	      [self updateTextViewWithMovement: NSDownTextMovement
				       isFinal: NO];
            }
          else if (key == NSEnterCharacter ||
		   key == NSCarriageReturnCharacter ||
		   key == NSNewlineCharacter)
	    {
	      [self clickItem: self];
	      break;
	    }
	  else if (key == 0x001b ||
		   key == NSRightArrowFunctionKey ||
		   key == NSLeftArrowFunctionKey)
	    {
	      [self updateTextViewWithMovement: NSCancelTextMovement
				       isFinal: NO];
	      break;
	    }
          else
            {
	      // First remove the selected text.
	      [_textView replaceCharactersInRange: [_textView selectedRange]
				       withString: @""];
	      // Send the even to update the text container.
	      [NSApp sendEvent: event];
	      // Reload data.
	      [self reloadData];
	    }
	}
      else
        {
	  [NSApp sendEvent: event];
	}
      
      if (_stopped)
        break;      
    }

  [pool drain];
}

- (void) onWindowEdited: (NSNotification *)notification
{
  _stopped = YES;
  [self updateTextViewWithMovement: NSCancelTextMovement
			   isFinal: NO];
}

- (void) reloadData
{
  _range = [_textView rangeForUserCompletion];

  if (_range.location == NSNotFound || _range.length == 0)
    {
      _stopped = YES;
    }
  else
    {
      NSInteger index = 0;
      NSString *word;
      NSArray *newWords;

      word = [[_textView string] substringWithRange: _range];
      ASSIGN(_originalWord, word);

      newWords = [_textView completionsForPartialWordRange: _range
				       indexOfSelectedItem: &index];

      if ([newWords count] > 0)
	{
	  ASSIGN(_words, newWords);
	  [_tableView reloadData];
	  [self layout];
	  [self computePosition];
	  [_tableView selectRow: index byExtendingSelection: NO];
	  [_tableView scrollRowToVisible: index];
	  [self updateTextViewWithMovement: NSOtherTextMovement
				   isFinal: NO];
	}
      else
	{
	  [_tableView reloadData];
	  _stopped = YES;
	  [self updateTextViewWithMovement: NSCancelTextMovement
				   isFinal: NO];
	}
    }
}

- (void) updateTextViewWithMovement: (NSInteger)movement
			    isFinal: (BOOL)flag
{
  NSString *word;

  if (movement != NSCancelTextMovement)
    {
      NSInteger rowIndex = [_tableView selectedRow];
      word = [[_words objectAtIndex: rowIndex] description];
    }
  else
    {
      word = _originalWord;
    }

  [_textView insertCompletion: word
	  forPartialWordRange: _range
		     movement: movement
		      isFinal: flag];

  // Release _words and _originalWords if
  // autocomplete is final or canceled.
  if ( (flag) ||
       (movement == NSCancelTextMovement) )
    {
      ASSIGN(_originalWord, nil);
      ASSIGN(_words, nil);
    }
}

// Action method
- (void) clickItem: (id)sender
{
  [self updateTextViewWithMovement: NSOtherTextMovement
			   isFinal: YES];

  _stopped = YES;
}

// Key actions methods
- (void) moveUpSelection
{
  NSInteger index = [_tableView selectedRow] - 1;

  if (index > -1 && index < [_tableView numberOfRows])
    {
      [_tableView selectRow: index byExtendingSelection: NO];
      [_tableView scrollRowToVisible: index];
    }
}

- (void) moveDownSelection
{
  NSInteger index = [_tableView selectedRow] + 1;
  
  if (index > -1 && index < [_tableView numberOfRows])
    {
      [_tableView selectRow: index byExtendingSelection: NO];
      [_tableView scrollRowToVisible: index];
    }
}

// Delegate
- (int) numberOfRowsInTableView: (NSTableView *)aTableView
{
  return [_words count];
}

- (id) tableView: (NSTableView *)aTableView
  objectValueForTableColumn: (NSTableColumn *)aTableColumn
	     row: (int)rowIndex
{
  return [[_words objectAtIndex: rowIndex] description];
}
@end

