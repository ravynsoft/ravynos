/** <title>GSTextFinder</title>

   Copyright (C) 2010 Free Software Foundation, Inc.

   Author: Wolfgang Lux <wolfgang.lux@gmail.com>
   Date: July 2010

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
#import <Foundation/NSString.h>
#import <Foundation/NSNotification.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSButton.h"
#import "AppKit/NSEvent.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSNib.h"
#import "AppKit/NSNibLoading.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSTextField.h"
#import "AppKit/NSTextView.h"
#import "AppKit/NSWindow.h"
#import "GSGuiPrivate.h"
#import "GSTextFinder.h"


@interface GSTextFinder(PrivateMethods)
- (BOOL) _loadPanel;
- (void) _applicationDidBecomeActive: (NSNotification *)notification;
- (void) _updateFindStringFromPanel: (unsigned *)options
		    putToPasteboard: (BOOL)flag;
- (void) _updateReplaceStringFromPanel;
- (void) _getFindStringFromPasteboard;
- (void) _putFindStringToPasteboard;
@end

@implementation GSTextFinder

static GSTextFinder *sharedTextFinder;

+ (GSTextFinder *) sharedTextFinder
{
  if (sharedTextFinder == nil)
    {
      sharedTextFinder = [[self alloc] init];
    }
  return sharedTextFinder;
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      // make sure our search and replace strings are never nil
      findString = @"";
      replaceString = @"";

      // update find string from pasteboard whenever the application is
      // activated
      [[NSNotificationCenter defaultCenter]
	addObserver: self
	   selector: @selector(_applicationDidBecomeActive:)
	       name: NSApplicationDidBecomeActiveNotification
	     object: NSApp];
      [self _applicationDidBecomeActive: nil];
    }
  return self;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter]
    removeObserver: self
	      name: NSApplicationDidBecomeActiveNotification
	    object: NSApp];

  DESTROY(findString);
  DESTROY(replaceString);
  [super dealloc];
}

// UI actions
- (void) findNext: (id)sender
{
  if ([self findStringInTextView: [self targetView: nil] forward: YES])
    {
      // Special case here: If the user edits the find string and then presses
      // the Return key while the find text field (rather the panel's field
      // editor) is first responder, close the panel when a match was found.
      // This behavior is compatible with OpenStep and Mac OS X. However, in
      // contrast to Mac OS X, this cannot be implemented by associating a
      // dedicated action with the find text field, since the event is already
      // processed by the panel's default button before the field editor has a
      // chance of looking at it.

      // NB I assume here that the only keyboard event that can trigger the
      // Next button's action while the field editor is active is a key down
      // event from the Return key.
      if ([[panel currentEvent] type] == NSKeyDown &&
	  [findText currentEditor] != nil)
	{
	  [panel close];
	}
    }
}

- (void) findPrevious: (id)sender
{
  [self findStringInTextView: [self targetView: nil] forward: NO];
}

- (void) replaceAndFind: (id)sender
{
  NSTextView *targetView = [self targetView: nil];
  [self replaceStringInTextView: targetView];
  [self findStringInTextView: targetView forward: YES];
}

- (void) replace: (id)sender
{
  [self replaceStringInTextView: [self targetView: nil]];
}
  
- (void) replaceAll: (id)sender
{
  // NB In contrast to -performFindPanelAction: this UI action takes the current
  // selection in the Replace All Scope matrix into account
  [self replaceAllInTextView: [self targetView: nil]
	     onlyInSelection: [replaceScopeMatrix selectedTag] != 0];
}

- (void) performFindPanelAction: (id)sender
{
  [self performFindPanelAction: sender withTextView: nil];
}

- (void) performFindPanelAction: (id)sender
		   withTextView: (NSTextView *)aTextView
{
  aTextView = [self targetView: aTextView];
  switch ([sender tag])
    {
    case NSFindPanelActionShowFindPanel:
      [self showFindPanel];
      break;

    case NSFindPanelActionNext:
      [self findStringInTextView: aTextView forward: YES];
      break;
    case NSFindPanelActionPrevious:
      [self findStringInTextView: aTextView forward: NO];
      break;

    case NSFindPanelActionReplaceAll:
      [self replaceAllInTextView: aTextView onlyInSelection: NO];	
      break;

    case NSFindPanelActionReplace:
      [self replaceStringInTextView: aTextView];
      break;

    case NSFindPanelActionReplaceAndFind:
      [self replaceStringInTextView: aTextView];
      [self findStringInTextView: aTextView forward: YES];
      break;

    case NSFindPanelActionSetFindString:
      [self takeFindStringFromTextView: aTextView];
      break;

    case NSFindPanelActionReplaceAllInSelection:
      [self replaceAllInTextView: aTextView onlyInSelection: YES];
      break;

    case NSFindPanelActionSelectAll:
      NSLog(@"NSFindPanelActionSelectAll not supported");
      break;

    case NSFindPanelActionSelectAllInSelection:
      NSLog(@"NSFindPanelActionSelectAllInSelection not supported");
      break;

    default:
      NSLog(@"Unknown find panel action (%ld)", (long)[sender tag]);
    }
}

- (BOOL) validateFindPanelAction: (id)sender
		    withTextView: (NSTextView *)aTextView
{
  aTextView = [self targetView: aTextView];
  switch ([sender tag])
    {
    case NSFindPanelActionShowFindPanel:
      return YES;

    case NSFindPanelActionReplace:
      return aTextView != nil;

    case NSFindPanelActionSetFindString:
      return aTextView && [aTextView selectedRange].length > 0;

    case NSFindPanelActionNext:
    case NSFindPanelActionPrevious:
    case NSFindPanelActionReplaceAll:
    case NSFindPanelActionReplaceAndFind:
#if 0 // NSTextView does not support discontinuous selections at present
    case NSFindPanelActionSelectAll:
#endif
      return aTextView && [findString length] > 0;

    case NSFindPanelActionReplaceAllInSelection:
#if 0 // NSTextView does not support discontinuous selections at present
    case NSFindPanelActionSelectAllInSelection:
#endif
      return [findString length] > 0
	  && aTextView && [aTextView selectedRange].length > 0;

    default:
      break;
    }

  // disable everything else
  return NO;
}

// text finder methods
- (void) showFindPanel
{
  if (panel == nil && [self _loadPanel] == NO)
    {
      return;
    }

  [messageText setStringValue: @""];
  [panel makeKeyAndOrderFront: self];
  [findText selectText: self];
}

- (void) takeFindStringFromTextView: (NSText *)aTextView
{
  [messageText setStringValue: @""];
  if (aTextView != nil)
    {
      NSRange range = [aTextView selectedRange];
      if (range.length)
	{
	  NSString *string = [[aTextView string] substringFromRange: range];
	  ASSIGNCOPY(findString, string);
	  [findText setStringValue: string];
	  [findText selectText: self];
	  [self _putFindStringToPasteboard];
	}
    }
}

- (BOOL) findStringInTextView: (NSText *)aTextView
		      forward: (BOOL)forward
{
  NSRange range;
  NSRange selectedRange;
  NSString *string;
  unsigned int options = NSLiteralSearch | NSCaseInsensitiveSearch;

  [messageText setStringValue: @""];
  if (aTextView == nil)
    {
      return NO;
    }

  string = [aTextView string];
  selectedRange = [aTextView selectedRange];
  [self _updateFindStringFromPanel: &options putToPasteboard: YES];
  if (forward)
    {
      range = NSMakeRange(NSMaxRange(selectedRange),
			  [string length] - NSMaxRange(selectedRange));
      range = [string rangeOfString: findString
			    options: options
			      range: range];
      if (range.location == NSNotFound)
	{
	  range = NSMakeRange(0, selectedRange.location);
	  range = [string rangeOfString: findString
				options: options
				  range: range];
	}
    }
  else
    {
      options |= NSBackwardsSearch;
      range = NSMakeRange(0, selectedRange.location);
      range = [string rangeOfString: findString
			    options: options
			      range: range];
      if (range.location == NSNotFound)
	{
	  range = NSMakeRange(NSMaxRange(selectedRange),
			      [string length] - NSMaxRange(selectedRange));
	  range = [string rangeOfString: findString
				options: options
				  range: range];
	}
    }

  if (range.location != NSNotFound)
    {
      [aTextView setSelectedRange: range];
      [aTextView scrollRangeToVisible: range];
    }
  else
    {
      [messageText setStringValue: _(@"Not found")];
      NSBeep();
    }
  return range.location != NSNotFound;
}

- (void) replaceStringInTextView: (NSTextView *)aTextView
{
  [messageText setStringValue: @""];
  if (aTextView != nil)
    {
      NSRange range = [aTextView selectedRange];

      if ([aTextView shouldChangeTextInRange: range
			   replacementString: replaceString])
	{
	  [self _updateReplaceStringFromPanel];
	  [aTextView replaceCharactersInRange: range
				   withString: replaceString];
	  [aTextView didChangeText];
	  [aTextView scrollRangeToVisible: range];
	}
    }
}

- (void) replaceAllInTextView: (NSTextView *)aTextView
	      onlyInSelection: (BOOL)flag
{
  int n;
  NSRange range, replaceRange;
  NSString *format;
  NSString *string;
  unsigned int options = NSLiteralSearch | NSCaseInsensitiveSearch;
  [messageText setStringValue: @""];
  if (aTextView == nil)
    return;

  [self _updateFindStringFromPanel: &options putToPasteboard: YES];
  [self _updateReplaceStringFromPanel];
  string = [aTextView string];
  replaceRange =
    flag ? [aTextView selectedRange] : NSMakeRange(0, [string length]);

  // look for a first match in the range
  range = [string rangeOfString: findString
			options: options
			  range: replaceRange];
  if (range.location == NSNotFound)
    {
      [messageText setStringValue: _(@"Not found")];
      NSBeep();
      return;
    }

  n = 0;
  do
    {
      if ([aTextView shouldChangeTextInRange: range
			   replacementString: replaceString])
	{
	  [aTextView replaceCharactersInRange: range
				   withString: replaceString];
	  [aTextView didChangeText];
	  n++;
	}

      replaceRange =
	NSMakeRange(range.location + [replaceString length],
		    NSMaxRange(replaceRange) -
		    (range.location + [findString length]));
      range = [string rangeOfString: findString
			    options: options
			      range: replaceRange];
    }
  while (range.location != NSNotFound);

  format = _(@"%d replaced");
  [messageText setStringValue: [NSString stringWithFormat: format, n]];

  // set insertion point to the end of the last match
  range = NSMakeRange(replaceRange.location, 0);
  [aTextView setSelectedRange: range];
  [aTextView scrollRangeToVisible: range];
}

- (NSTextView *) targetView: (NSTextView *)aTextView
{
  // If aTextView is equal to the find panel's field editor use the default
  // target view
  if (aTextView == [panel fieldEditor: NO forObject: [panel firstResponder]])
    {
      aTextView = nil;
    }

  if (aTextView == nil)
    {
      // The default target is the first responder of the main window
      // provided that it is a text view
      id aResponder = [[NSApp mainWindow] firstResponder];
      if ([aResponder isKindOfClass: [NSTextView class]])
	{
	  aTextView = aResponder;
	}
    }
  return aTextView;
}

@end

@implementation GSTextFinder(PrivateMethods)

- (void) _applicationDidBecomeActive: (NSNotification *)notification
{
  [self _getFindStringFromPasteboard];
}

- (BOOL) _loadPanel
{
  NSDictionary *table =
    [NSDictionary dictionaryWithObject: self forKey: NSNibOwner];
  if (![GSGuiBundle() loadNibFile: @"GSFindPanel"
		externalNameTable: table
			 withZone: [self zone]])
    {
      NSLog(@"Model file load failed for GSFindPanel");
      return NO;
    }

  [findText setStringValue: findString];
  [replaceText setStringValue: replaceString];
  [messageText setStringValue: @""];

  // FIXME Setting this in gorm does not have an effect
  [panel setFrameAutosaveName: @"NSFindPanel"];

  // Make sure the Find menu is enabled when the panel's field editor is
  // first responder
  [(NSTextView *)[panel fieldEditor: YES forObject: nil] setUsesFindPanel: YES];

  return YES;
}

- (void) _updateFindStringFromPanel: (unsigned int *)options
		    putToPasteboard: (BOOL)flag
{
  if (panel)
    {
      ASSIGN(findString, [findText stringValue]);
      if ([ignoreCaseButton state] != NSOffState)
	{
	  *options |= NSCaseInsensitiveSearch;
          // literal search is always case-sensitive, so it must be removed in this case
	  *options &= ~NSLiteralSearch;
	}
      else
	{
	  *options &= ~NSCaseInsensitiveSearch;
	}
    }

  if (flag)
    {
      [self _putFindStringToPasteboard];
    }
}

- (void) _updateReplaceStringFromPanel
{
  if (panel)
    {
      ASSIGN(replaceString, [replaceText stringValue]);
    }
}

- (void) _getFindStringFromPasteboard
{
  NSPasteboard *pboard = [NSPasteboard pasteboardWithName:NSFindPboard];
  if ([[pboard types] containsObject:NSStringPboardType])
    {
      NSString *string = [pboard stringForType:NSStringPboardType];
      if ([string length] && ![string isEqualToString:findString])
	{
	  ASSIGN(findString, string);
	  [findText setStringValue: string];
	  [findText selectText: self];
	}
    }
}

- (void) _putFindStringToPasteboard
{
    NSPasteboard *pboard = [NSPasteboard pasteboardWithName:NSFindPboard];
    [pboard declareTypes: [NSArray arrayWithObject:NSStringPboardType]
		   owner: nil];
    [pboard setString: findString forType: NSStringPboardType];
}

@end
