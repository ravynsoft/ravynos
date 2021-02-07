/** <title>NSDataLinkPanel</title>

   Copyright (C) 1996, 2003, 2004 Free Software Foundation, Inc.

   Author: Gregory John Casamento <greg_casamento@yahoo.com>
   Author: Scott Christley <scottc@net-community.com>
   Date: 1996, 2003, 2004
   
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
#import <Foundation/NSDictionary.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSDataLinkPanel.h"
#import "AppKit/NSDataLinkManager.h"
#import "AppKit/NSDataLink.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSView.h"
#import "AppKit/NSNibLoading.h"
#import "GSGuiPrivate.h"

static NSDataLinkPanel *_sharedDataLinkPanel;

@interface GSDataLinkPanelController : NSObject
{
  id panel;
}
- (id) panel;
@end

@implementation GSDataLinkPanelController
- (id) init
{
  if ((self = [super init]) != nil)
    {
      if ([NSBundle loadNibNamed: @"GSDataLinkPanel" owner: self] == NO)
	{
	  NSRunAlertPanel(@"Error", @"Could not load data link panel resource", 
			  @"OK", NULL, NULL);
	  return nil;
	}
    }

  return self;
}

- (id) panel
{
  return panel;
}

- (void) dealloc
{
  RELEASE(panel);
  [super dealloc];
}
@end

@implementation NSApplication (NSDataLinkPanel)

/**
 * Order the data link panel to the front.  If it has not already 
 * been instantiated, instantiate it.
 */
- (void) orderFrontDataLinkPanel: sender
{
  NSDataLinkPanel *dataLinkPanel = [NSDataLinkPanel sharedDataLinkPanel];

  if (dataLinkPanel)
    [dataLinkPanel orderFront: nil];
  else
    NSBeep();
}

@end

@implementation NSDataLinkPanel

+ (void)initialize
{
  if (self == [NSDataLinkPanel class])
    {
      // Initial version
      [self setVersion: 0];
    }
}

/**
 * Initializes and returns the shared panel.
 */
+ (NSDataLinkPanel *)sharedDataLinkPanel
{
  if (_sharedDataLinkPanel == nil)
    {
      id controller = [[GSDataLinkPanelController alloc] init];
      ASSIGN(_sharedDataLinkPanel, [controller panel]);
      RELEASE(controller);
    }
  return _sharedDataLinkPanel;
}

/**
 * Get the currently selected array of links and thier respective managers. 
 * Return the whether or not multiple links are selected in flag.
 */
+ (void)getLink:(NSDataLink **)link
	manager:(NSDataLinkManager **)linkManager
     isMultiple:(BOOL *)flag
{
  [[NSDataLinkPanel sharedDataLinkPanel]
    getLink: link
    manager: linkManager
    isMultiple: flag];
}

/**
 * Set the currently selected array of links and their respective managers.
 * If all of the given links should be selected flag should be YES.
 */
+ (void)setLink:(NSDataLink *)link
	manager:(NSDataLinkManager *)linkManager
     isMultiple:(BOOL)flag
{
  [[NSDataLinkPanel sharedDataLinkPanel]
    setLink: link
    manager: linkManager
    isMultiple: flag];
}

//
// Instance methods
//

/**
 * Get the currently selected array of links and thier respective managers. 
 * Return the whether or not multiple links are selected in flag.
 */
- (void)getLink:(NSDataLink **)link
	manager:(NSDataLinkManager **)linkManager
     isMultiple:(BOOL *)flag
{
  ASSIGN(*link, _currentDataLink);
  ASSIGN(*linkManager, _currentDataLinkManager);
  *flag = _multipleSelection;
}

/**
 * Set the currently selected array of links and their respective managers.
 * If all of the given links should be selected flag should be YES.
 */
- (void)setLink:(NSDataLink *)link
	manager:(NSDataLinkManager *)linkManager
     isMultiple:(BOOL)flag
{
  ASSIGN(_currentDataLink, link);
  ASSIGN(_currentDataLinkManager, linkManager);
  _multipleSelection = flag;
}

//
// Customizing the Panel
//

/**
 * Add an accessory view to the panel.
 */
- (NSView *)accessoryView
{
  // not yet implemented.
  return nil;
}

/**
 * Get the accessory view.
 */
- (void)setAccessoryView:(NSView *)aView
{
  // not yet implemented.
}

//
// Responding to User Input
//

/**
 * Called when the user presses the Break All Links button.
 * Invokes breakAllLinks on the current link manager.
 */
- (void)pickedBreakAllLinks:(id)sender
{
  [_currentDataLinkManager breakAllLinks];
}

/**
 * Called when the user presses the Break button.
 * Invokes break on the current link.
 */
- (void)pickedBreakLink:(id)sender
{
  [_currentDataLink break];
}

/**
 * Called when the user presses the Open Source button.
 * Invokes openSource on the current link.
 */
- (void)pickedOpenSource:(id)sender
{
  [_currentDataLink openSource];
}

/**
 * Called when the Update Destination button
 * Invokes updateDestination on the current link.
 */
- (void)pickedUpdateDestination:(id)sender
{
  [_currentDataLink updateDestination];
}

/**
 * Called when the user selects an update mode from the pull down.
 * Invokes setUpdateMode: on the current link.
 */
- (void)pickedUpdateMode:(id)sender
{
  NSDataLinkUpdateMode mode = (NSDataLinkUpdateMode)[sender tag];
  [_currentDataLink setUpdateMode: mode];
}

@end
