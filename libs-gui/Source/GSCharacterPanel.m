/** <title>GSCharacterPanel</title>

   <abstract>Character Panel.</abstract>

   Copyright (C) 2011 Free Software Foundation, Inc.
   
   Author:  Eric Wasylishen <ewasylishen@gmail.com>
   Date: July 2011
   
   This file is part of the GNUstep Application Kit Library.

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

#import <Foundation/NSIndexSet.h>
#import <Foundation/NSBundle.h>
#import <Foundation/NSAutoreleasePool.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSStringDrawing.h"
#import "AppKit/NSPasteboard.h"
#import "AppKit/NSTableView.h"
#import "AppKit/NSTableColumn.h"
#import "AppKit/NSTextFieldCell.h"
#import "AppKit/NSScrollView.h"
#import "AppKit/NSSearchField.h"
#import "GNUstepGUI/GSCharacterPanel.h"
#import "GSGuiPrivate.h"

@implementation NSApplication (CharacterPanel)

- (void) orderFrontCharacterPalette: (id)sender
{
  [[GSCharacterPanel sharedCharacterPanel] orderFront: sender];
}

@end

#if defined(HAVE_UNICODE_UCHAR_H) && defined(HAVE_UNICODE_USTRING_H)
#include <unicode/uchar.h>
#include <unicode/ustring.h>

/*
 * Define TRUE/FALSE to be used with UBool parameters, as these are no longer
 * defined in ICU as of ICU 68.
 */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


@interface GSVerticallyCenteredTextFieldCell : NSTextFieldCell
{
}
@end

@implementation GSVerticallyCenteredTextFieldCell

- (NSRect) titleRectForBounds: (NSRect)aRect
{
  NSRect titleRect = [super titleRectForBounds: aRect];
  NSSize titleSize = [[self attributedStringValue] size];
  titleRect.origin.y = aRect.origin.y + (aRect.size.height - titleSize.height) / 2.0;
  titleRect.size.height = titleSize.height;
  return titleRect;
}

@end

// Enumerating assigned codepoints

static UBool enumCharNamesFn(void *context, UChar32 code, UCharNameChoice nameChoice, const char *name, int32_t length)
{
  [(NSMutableIndexSet*)context addIndex: (NSUInteger)code];
  return TRUE;
}

static NSIndexSet *AssignedCodepoints()
{
  UErrorCode err = U_ZERO_ERROR;
  NSMutableIndexSet *set = [NSMutableIndexSet indexSet];
  u_enumCharNames(UCHAR_MIN_VALUE, UCHAR_MAX_VALUE + 1, enumCharNamesFn, set, U_UNICODE_CHAR_NAME, &err);
  return set;
}

// Searching for codepoints

struct searchContext {
  const char *searchString;
  NSMutableIndexSet *set;
};

static UBool searchCharNamesFn(void *context, UChar32 code, UCharNameChoice nameChoice, const char *name, int32_t length)
{
  struct searchContext *ctx = (struct searchContext *)context;
  if (strstr(name, ctx->searchString) != NULL)
    {
      [ctx->set addIndex: (NSUInteger)code];
    }
  return TRUE;
}

static NSIndexSet *CodepointsWithNameContainingSubstring(NSString *str)
{
  UErrorCode err = U_ZERO_ERROR;
  struct searchContext ctx;
	
  ctx.set = [NSMutableIndexSet indexSet];
  ctx.searchString = [[str uppercaseString] UTF8String];

  u_enumCharNames(UCHAR_MIN_VALUE, UCHAR_MAX_VALUE + 1, searchCharNamesFn, &ctx, U_UNICODE_CHAR_NAME, &err);
	
  return ctx.set;
}


@implementation GSCharacterPanel 

- (void)setVisibleCodepoints: (NSIndexSet*)set
{
  ASSIGN(visibleCodepoints, set);
}


+ (GSCharacterPanel *) sharedCharacterPanel
{
  static GSCharacterPanel *shared = nil;
  if (nil == shared)
    {
      shared = [[self alloc] init];
    }
  return shared;
}

- (id) init
{
  const NSRect contentRect = NSMakeRect(100, 100, 276, 420);
  self = [super initWithContentRect: contentRect
			  styleMask: NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask | NSUtilityWindowMask
			    backing: NSBackingStoreBuffered
			      defer: YES];
  if (nil != self)
    {
      // Setup assignedCodepoints and  visibleCodepointsArray
      assignedCodepoints = [AssignedCodepoints() retain];
      [self setVisibleCodepoints: assignedCodepoints];

      [self setTitle: _(@"Character Panel")];
		
      // Set up the table view
      table = [[[NSTableView alloc] initWithFrame: NSMakeRect(0, 0, contentRect.size.width - 18, contentRect.size.height - 52)] autorelease];

      // Set up table columns
      {
	NSTableColumn *col = [[[NSTableColumn alloc] initWithIdentifier: @"char"] autorelease];
	[col setDataCell: [[[GSVerticallyCenteredTextFieldCell alloc] init] autorelease]];
	[[col dataCell] setFont:[NSFont systemFontOfSize: 24]];
	[[col dataCell] setAlignment: NSCenterTextAlignment];
	[col setMinWidth: 40];
	[col setWidth: 40];
	[table addTableColumn: col];
      }
      {
	NSTableColumn *col = [[[NSTableColumn alloc] initWithIdentifier: @"name"] autorelease];
	[col setDataCell: [[[GSVerticallyCenteredTextFieldCell alloc] init] autorelease]];
	[[col dataCell] setFont:[NSFont systemFontOfSize: 10]];
	[[col headerCell] setStringValue: _(@"Name")];
	[col setWidth: 195];
	[table addTableColumn: col];
      }
      {
	NSTableColumn *col = [[[NSTableColumn alloc] initWithIdentifier: @"code"] autorelease];
	[col setDataCell: [[[GSVerticallyCenteredTextFieldCell alloc] init] autorelease]];
	[[col dataCell] setFont:[NSFont systemFontOfSize: 10]];
	[[col dataCell] setAlignment: NSCenterTextAlignment];
	[[col headerCell] setStringValue: _(@"Code Point")];
	[col setMinWidth: 80];
	[col setWidth: 80];
	[table addTableColumn: col];
      }
      {
	NSTableColumn *col = [[[NSTableColumn alloc] initWithIdentifier: @"block"] autorelease];
	[col setDataCell: [[[GSVerticallyCenteredTextFieldCell alloc] init] autorelease]];
	[[col dataCell] setFont:[NSFont systemFontOfSize: 10]];
	[[col headerCell] setStringValue: _(@"Unicode Block")];
	[col setMinWidth: 140];
	[table addTableColumn: col];
      }      

      [table setRowHeight: 32];
      [table setDataSource: self];
      [table setDelegate: self];
      [table setTarget: self];
      [table setDoubleAction: @selector(doubleClickRow:)];

      // Allow dragging out of the application
      [table setDraggingSourceOperationMask:NSDragOperationCopy forLocal:NO];
      
      // Set up scroll view
      {
	NSScrollView *scrollView = [[NSScrollView alloc] initWithFrame: NSMakeRect(9, 41, contentRect.size.width - 18, contentRect.size.height - 52)];
	[scrollView setDocumentView: table];
	[scrollView setHasHorizontalScroller: YES];
	[scrollView setHasVerticalScroller: YES];
	[scrollView setBorderType: NSBezelBorder];
	[scrollView setAutoresizingMask: (NSViewWidthSizable | NSViewHeightSizable)];
	[[self contentView] addSubview: scrollView];
	[scrollView release];
      }

      // Set up search field
      {
	searchfield = [[NSSearchField alloc] initWithFrame: NSMakeRect(9,9,186,22)];
	[searchfield setTarget: self];
	[searchfield setAction: @selector(search:)];
	[[self contentView] addSubview: searchfield];
	[searchfield release];
      }
    }

  return self;
}

- (void)dealloc
{
  [assignedCodepoints release];
  [visibleCodepoints release];
  [super dealloc];
}

- (void)search: (id)sender
{
  NSString *str = [searchfield stringValue];
	
  if ([str length] == 0)
    {
      [self setVisibleCodepoints: assignedCodepoints];
    }
  else
    {
      NSIndexSet *set = CodepointsWithNameContainingSubstring(str);
      [self setVisibleCodepoints: set];
    }
	
  [table reloadData];
}

- (NSUInteger) codepointAtVisibleRow:(NSUInteger)row
{
  //FIXME: Use a binary search
  NSUInteger curr = 0;
  NSUInteger currValue = [visibleCodepoints firstIndex];
	
  while (currValue != NSNotFound)
    {
      if (curr == row)
	{
	  return currValue;
	}
      currValue = [visibleCodepoints indexGreaterThanIndex: currValue];
      curr++;
    }
  return NSNotFound;
}

- (NSString *)characterForRow: (NSInteger)row
{
  if (row >= 0 && row < [visibleCodepoints count])
    {
      UChar32 utf32 = [self codepointAtVisibleRow: row];
      UChar utf16buf[2];
      int32_t utf16bufLength = 0;
      UErrorCode error = U_ZERO_ERROR;
      u_strFromUTF32(utf16buf, 2, &utf16bufLength, &utf32, 1, &error);
	  
      return [[[NSString alloc] initWithCharacters: utf16buf
					    length: utf16bufLength] autorelease];
    }
  return @"";
}

- (void) doubleClickRow: (id)sender
{
  NSWindow *mainWindow = [NSApp mainWindow];
  NSResponder *firstResponder = [mainWindow firstResponder];
  NSString *str = [self characterForRow: [table clickedRow]];

  [firstResponder insertText: str];
}

- (BOOL) tableView: (NSTableView *)aTable shouldEditTableColumn: (NSTableColumn *)aColumn row: (NSInteger)row
{
  return NO;
}

// NSTableViewDataSource protocol

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView
{
  return [visibleCodepoints count];
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
  UChar32 utf32 = [self codepointAtVisibleRow: row];
  
  if ([[tableColumn identifier] isEqualToString: @"char"])
    {
      return [self characterForRow: row];
    }
  else if ([[tableColumn identifier] isEqualToString: @"name"])
    {
      UErrorCode error = U_ZERO_ERROR;
      int32_t size = u_charName(utf32, U_UNICODE_CHAR_NAME, NULL, 0, &error);

      if (size > 0)
	{
	  char name[512];
	  error = U_ZERO_ERROR;
	  u_charName(utf32, U_UNICODE_CHAR_NAME, name, 512, &error);

	  NSString *nameObj = [[[NSString alloc] initWithBytes: name
							length: size
						      encoding: NSASCIIStringEncoding] autorelease];
	  return [[nameObj lowercaseString] capitalizedString];
	}
      return @"";
    }
  else if ([[tableColumn identifier] isEqualToString: @"code"])
    {
      return [NSString stringWithFormat:@"U+%04X", (int)utf32];
    }
  else if ([[tableColumn identifier] isEqualToString: @"block"])
    {
      int32_t val = u_getIntPropertyValue(utf32, UCHAR_BLOCK);
      const char *name = u_getPropertyValueName(UCHAR_BLOCK, val, U_LONG_PROPERTY_NAME);
      if (name != NULL)
	{
	  return [[[[NSString alloc] initWithBytes: name
					    length: strlen(name)
					  encoding: NSASCIIStringEncoding] autorelease] stringByReplacingOccurrencesOfString: @"_" withString: @" "];
	}
    }  
  return nil; 
}

- (BOOL)tableView:(NSTableView *)aTableView writeRowsWithIndexes:(NSIndexSet *)rowIndexes toPasteboard:(NSPasteboard *)pboard
{
  NSString *str = [self characterForRow: [rowIndexes firstIndex]];
	
	[pboard declareTypes: [NSArray arrayWithObject: NSStringPboardType]
			owner: nil];
	[pboard setString: str
		forType: NSStringPboardType];
	
	return YES;
}

@end

#else // !(defined(HAVE_UNICODE_UCHAR_H) && defined(HAVE_UNICODE_USTRING_H))

@implementation GSCharacterPanel

+ (GSCharacterPanel *) sharedCharacterPanel
{
  return nil;
}

@end

#endif
