/** <title>GSThemePanel</title>

   <abstract>Theme management utility</abstract>

   Copyright (C) 2010 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg.casamento@gmail.com>
   Date: 2010
   
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

#import <Foundation/NSString.h>
#import <Foundation/NSArchiver.h>
#import "AppKit/NSMenu.h"
#import "AppKit/NSPanel.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSMenuView.h"
#import "AppKit/NSApplication.h"

#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSWindowDecorationView.h"

#import "NSToolbarFrameworkPrivate.h"
#import "GSGuiPrivate.h"


@interface NSWindow (Private)
- (GSWindowDecorationView *) windowView;
- (void) _setMenu: (NSMenu *)menu;
@end

@implementation NSWindow (Private)
- (GSWindowDecorationView *) windowView
{
  return _wv;
}

- (void) _setMenu: (NSMenu *)menu
{
  [super setMenu: menu];
}
@end

@implementation	GSTheme (Menu)
- (void) setMenu: (NSMenu *)menu
       forWindow: (NSWindow *)window
{
  GSWindowDecorationView *wv = [window windowView];

  // protect against stupid calls from updateAllWindowsWithMenu:
  if ([window menu] == menu)
    return;

  // Prevent recursion
  [window _setMenu: menu];

  // Remove any possible old menu view
  [wv removeMenuView];

  //NSLog(@"Adding menu %@ to window %@", menu, window);
  if (menu != nil)
  {
    NSMenuView *menuView = [[NSMenuView alloc] initWithFrame: NSZeroRect];

    [menuView setMenu: menu];
    [menuView setHorizontal: YES];
    [menuView setInterfaceStyle: NSWindows95InterfaceStyle];
    [wv addMenuView: menuView];
    [menuView sizeToFit];
    RELEASE(menuView);
  } 
}

- (void) rightMouseDisplay: (NSMenu *)menu
		  forEvent: (NSEvent *)theEvent
{
  NSMenuView *mv = [menu menuRepresentation];
  if ([mv isHorizontal] == NO)
    {
      [menu displayTransient];
      [mv mouseDown: theEvent];
      [menu closeTransient];
    }
}

- (void) displayPopUpMenu: (NSMenuView *)mr
	    withCellFrame: (NSRect)cellFrame
	controlViewWindow: (NSWindow *)cvWin
	    preferredEdge: (NSRectEdge)edge
	     selectedItem: (int)selectedItem
{
  BOOL pe = [[GSTheme theme] doesProcessEventsForPopUpMenu];

  /* Ensure the window responds when run in modal and should
   * process events. Or revert this if theme has changed.
   */
  if (pe && ![[mr window] worksWhenModal])
    {
      [(NSPanel *)[mr window] setWorksWhenModal: YES];
    }

  if (!pe && [[mr window] worksWhenModal])
    {
      [(NSPanel *)[mr window] setWorksWhenModal: NO];
    }

  // Ask the MenuView to attach the menu to this rect
  [mr setWindowFrameForAttachingToRect: cellFrame
			      onScreen: [cvWin screen]
			 preferredEdge: edge
		     popUpSelectedItem: selectedItem];
  
  // Set to be above the main window
  [cvWin addChildWindow: [mr window] ordered: NSWindowAbove];

  // Last, display the window
  [[mr window] orderFrontRegardless];
}

- (void) processCommand: (void *)context
{
  // this is only implemented when we handle native menus.
  // put code in here to handle commands from the native menu structure.
}

- (float) menuHeightForWindow: (NSWindow *)window
{
  return [NSMenuView menuBarHeight];
}

- (void) updateMenu: (NSMenu *)menu forWindow: (NSWindow *)window
{
  [self setMenu: menu 
	forWindow: window];
}

- (void) updateAllWindowsWithMenu: (NSMenu *) menu
{
  NSEnumerator *en = [[NSApp windows] objectEnumerator];
  id            o = nil;

  while ((o = [en nextObject]) != nil)
    {
      if([o canBecomeMainWindow])
	{
	  [self updateMenu: menu forWindow: o];
	}
    }
}

- (BOOL) doesProcessEventsForPopUpMenu
{
  return NO; // themes that handle events in a popUpMenu should return YES
}

- (BOOL) menuShouldShowIcon
{
  return YES; // override whether or not to show the icon in the menu.
}

@end

