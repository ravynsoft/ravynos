/** <title>GSThemePanel</title>

   <abstract>Theme management utility</abstract>

   Copyright (C) 2008-2012 Free Software Foundation, Inc.

   Author: Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2007-2008
   
   This file is part of the GNU Objective C User interface library.

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

#import <Foundation/NSFileManager.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSPathUtilities.h>
#import <Foundation/NSSet.h>
#import <Foundation/NSUserDefaults.h>
#import "AppKit/NSButton.h"
#import "AppKit/NSMatrix.h"
#import "AppKit/NSScrollView.h"
#import "GSThemePrivate.h"
#import "GSGuiPrivate.h"


@implementation	GSThemePanel

static GSThemePanel	*sharedPanel = nil;

+ (GSThemePanel*) sharedThemePanel
{
  if (sharedPanel == nil)
    {
      sharedPanel = [self new];
    }
  return sharedPanel;
}

- (id) init
{
  NSRect	winFrame;
  NSRect	sideFrame;
  NSRect	bottomFrame;
  NSRect	frame;
  NSView	*container;
  NSButtonCell	*proto;

  winFrame = NSMakeRect(0, 0, 367, 420);
  sideFrame = NSMakeRect(0, 0, 95, 420);
  bottomFrame = NSMakeRect(95, 0, 272, 32);
  
  self = [super initWithContentRect: winFrame
    styleMask: (NSTitledWindowMask | NSClosableWindowMask
      | NSMiniaturizableWindowMask | NSResizableWindowMask)
    backing: NSBackingStoreBuffered
    defer: NO];
  
  [self setReleasedWhenClosed: NO];
  container = [self contentView];
  sideView = [[NSScrollView alloc] initWithFrame: sideFrame];
  [sideView setHasHorizontalScroller: NO];
  [sideView setHasVerticalScroller: YES];
  [sideView setBorderType: NSBezelBorder];
  [sideView setAutoresizingMask: (NSViewHeightSizable | NSViewMaxXMargin)];
  [container addSubview: sideView];
  RELEASE(sideView);

  proto = [[NSButtonCell alloc] init];
  [proto setBordered: NO];
  [proto setAlignment: NSCenterTextAlignment];
  [proto setImagePosition: NSImageAbove];
  [proto setSelectable: NO];
  [proto setEditable: NO];
  [matrix setPrototype: proto];

  frame = [sideView frame];
  frame.origin = NSZeroPoint;
  matrix = [[NSMatrix alloc] initWithFrame: frame
				      mode: NSRadioModeMatrix
				 prototype: proto
			      numberOfRows: 1
			   numberOfColumns: 1];
  RELEASE(proto);
  [matrix setAutosizesCells: NO];
  [matrix setCellSize: NSMakeSize(72,72)];
  [matrix setIntercellSpacing: NSMakeSize(8,8)];
  [matrix setAutoresizingMask: NSViewNotSizable];
  [matrix setMode: NSRadioModeMatrix];
  [matrix setAction: @selector(changeSelection:)];
  [matrix setTarget: self];

  [sideView setDocumentView: matrix];
  RELEASE(matrix);

  bottomView = [[NSView alloc] initWithFrame: bottomFrame];
  [bottomView setAutoresizingMask: (NSViewWidthSizable | NSViewMaxYMargin)];
  [container addSubview: bottomView];
  RELEASE(bottomView);

  [self setTitle: _(@"Themes")];
  
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(notified:)
    name: GSThemeDidActivateNotification
    object: nil];
  [[NSNotificationCenter defaultCenter]
    addObserver: self
    selector: @selector(notified:)
    name: GSThemeDidDeactivateNotification
    object: nil];

  /* Fake a notification to set the initial value for the inspector.
   */
  [self notified:
    [NSNotification notificationWithName: GSThemeDidActivateNotification
				  object: [GSTheme theme]
				userInfo: nil]];
  return self;
}

- (void) changeSelection: (id)sender
{
  NSButtonCell	*cell = [sender selectedCell];
  NSString	*name = [cell title];

  [GSTheme setTheme: [GSTheme loadThemeNamed: name]];
}

- (void) notified: (NSNotification*)n
{
  NSView		*cView;
  GSThemeInspector	*inspector;

  inspector = (GSThemeInspector*)[[n object] themeInspector];
  cView = [self contentView];

  if ([[n name] isEqualToString: GSThemeDidActivateNotification] == YES)
    {
      NSView	*iView;
      NSRect	frame;
      NSButton	*button;
      NSString	*dName;

      /* Ask the inspector to ensure that it is up to date.
       */
      [inspector update: self];

      /* Move the inspector view into our window.
       */
      iView = RETAIN([inspector contentView]);
      [inspector setContentView: nil];
      [cView addSubview: iView];
      RELEASE(iView);

      /* Set inspector view to fill the frame to the right of our
       * scrollview and above the bottom view
       */
      frame.origin.x = [sideView frame].size.width;
      frame.origin.y = [bottomView frame].size.height;
      frame.size = [cView frame].size;
      frame.size.width -= [sideView frame].size.width;
      frame.size.height -= [bottomView frame].size.height;
      [iView setFrame: frame];

      button = [[bottomView subviews] lastObject];
      if (button == nil)
        {
	  button = [NSButton new];
	  [button setTarget: self];
	  [button setAction: @selector(setDefault:)];
	  [bottomView addSubview: button];
	  RELEASE(button);
	}
      dName = [[NSUserDefaults standardUserDefaults] stringForKey: @"GSTheme"];
      if ([[[n object] name] isEqual: dName] == YES)
        {
	  [button setTitle: _(@"Revert default theme")];
	}
      else
	{
	  [button setTitle: _(@"Make this the default theme")];
	}
      [button sizeToFit];
      frame = [button frame];
      frame.origin.x = ([bottomView frame].size.width - frame.size.width) / 2;
      frame.origin.y = ([bottomView frame].size.height - frame.size.height) / 2;
      frame = [bottomView centerScanRect: frame];
      [button setFrame: frame];
    }
  else
    {
      /* Restore the inspector content view.
       */
      [inspector setContentView: [[cView subviews] lastObject]];
    }
  [cView setNeedsDisplay: YES];
}

- (void) setDefault: (id)sender
{
  NSButton		*button = (NSButton*)sender;
  NSUserDefaults	*defs = [NSUserDefaults standardUserDefaults];
  NSString		*cName;
  NSString		*dName;
  NSRect		frame;

  dName = [defs stringForKey: @"GSTheme"];
  cName = [[GSTheme theme] name];

  if ([cName isEqual: dName] == YES)
    {
      [defs removeObjectForKey: @"GSTheme"];
      [button setTitle: _(@"Make this the default theme")];
    }
  else
    {
      [defs setObject: cName forKey: @"GSTheme"];
      [button setTitle: _(@"Revert default theme")];
    }
  [defs synchronize];
  [button sizeToFit];
  frame = [button frame];
  frame.origin.x = ([bottomView frame].size.width - frame.size.width) / 2;
  frame.origin.y = ([bottomView frame].size.height - frame.size.height) / 2;
  [button setFrame: frame];
  [bottomView setNeedsDisplay: YES];
}

- (void) update: (id)sender
{
  NSArray		*array;
  GSTheme		*theme = [GSTheme loadThemeNamed: @"GNUstep.theme"];

  /* Avoid [NSMutableSet set] that confuses GCC 3.3.3.  It seems to confuse
   * this static +(id)set method with the instance -(void)set, so it would
   * refuse to compile saying
   * GSTheme.m:1565: error: void value not ignored as it ought to be
   */
  NSMutableSet		*set = AUTORELEASE([NSMutableSet new]);

  NSString		*selected = RETAIN([[matrix selectedCell] title]);
  unsigned		existing = [[matrix cells] count];
  NSFileManager		*mgr = [NSFileManager defaultManager];
  NSEnumerator		*enumerator;
  NSString		*path;
  NSString		*name;
  NSButtonCell		*cell;
  unsigned		count = 0;

  /* Ensure the first cell contains the default theme.
   */
  cell = [matrix cellAtRow: count++ column: 0];
  [cell setImage: [theme icon]];
  [cell setTitle: [theme name]];

  /* Go through all the themes in the standard locations and find their names.
   */
  enumerator = [NSSearchPathForDirectoriesInDomains
    (NSAllLibrariesDirectory, NSAllDomainsMask, YES) objectEnumerator];
  while ((path = [enumerator nextObject]) != nil)
    {
      NSEnumerator	*files;
      NSString		*file;

      path = [path stringByAppendingPathComponent: @"Themes"];
      files = [[mgr directoryContentsAtPath: path] objectEnumerator];
      while ((file = [files nextObject]) != nil)
        {
	  NSString	*ext = [file pathExtension];

	  name = [file stringByDeletingPathExtension];
	  if ([ext isEqualToString: @"theme"] == YES
	    && [name isEqualToString: @"GNUstep"] == NO
	    && [[name pathExtension] isEqual: @"backup"] == NO)
	    {
	      [set addObject: name];
	    }
	}
    }

  /* Sort theme names alphabetically, and add each theme to the matrix.
   */
  array = [[set allObjects] sortedArrayUsingSelector:
    @selector(caseInsensitiveCompare:)];
  enumerator = [array objectEnumerator];
  while ((name = [enumerator nextObject]) != nil)
    {
      GSTheme	*loaded;

      loaded = [GSTheme loadThemeNamed: name];
      if (loaded != nil)
	{
	  if (count >= existing)
	    {
	      [matrix addRow];
	      existing++;
	    }
	  cell = [matrix cellAtRow: count column: 0];
	  [cell setImage: [loaded icon]];
	  [cell setTitle: [loaded name]];
	  count++;
	}
    }

  /* Empty any unused cells.
   */
  while (count < existing)
    {
      cell = [matrix cellAtRow: count column: 0];
      [cell setImage: nil];
      [cell setTitle: @""];
      count++;
    }

  /* Restore the selected cell.
   */
  array = [matrix cells];
  count = [array count];
  while (count-- > 0)
    {
      cell = [matrix cellAtRow: count column: 0];
      if ([[cell title] isEqual: selected])
        {
	  [matrix selectCellAtRow: count column: 0];
	  break;
	}
    }
  RELEASE(selected);
  [matrix sizeToCells];
  [matrix setNeedsDisplay: YES];
}

@end

