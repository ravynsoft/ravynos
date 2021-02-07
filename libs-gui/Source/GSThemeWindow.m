/** <title>GSThemeWindow</title>

   <abstract>The theme methods for window specific functions</abstract>

   Copyright (C) 2015 Free Software Foundation, Inc.

   Author: Gregory Casamento <greg.casamento@gmail.com>
   Date: Jun 2015
   
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

#import "AppKit/NSWindow.h"
#import "AppKit/NSImage.h"
#import "AppKit/NSButton.h"

#import "GNUstepGUI/GSTheme.h"
#import "GNUstepGUI/GSWindowDecorationView.h"
#import "GSThemePrivate.h"

@implementation GSTheme (NSWindow)
- (NSButton *) standardWindowButton: (NSWindowButton)button
		       forStyleMask: (NSUInteger) mask
{
  NSButton *newButton;

  newButton = [[NSButton alloc] init];
  [newButton setRefusesFirstResponder: YES];
  [newButton setButtonType: NSMomentaryChangeButton];
  [newButton setImagePosition: NSImageOnly];
  [newButton setBordered: YES];
  [newButton setTag: button];
  
  switch (button)
    {
      case NSWindowCloseButton:
        [newButton setImage: [NSImage imageNamed: @"common_Close"]];
        [newButton setAlternateImage: [NSImage imageNamed: @"common_CloseH"]];
        /* TODO: -performClose: should (but doesn't currently) highlight the
           button, which is wrong here. When -performClose: is fixed, we'll need a
           different method here. */
        [newButton setAction: @selector(performClose:)];
        break;

      case NSWindowMiniaturizeButton:
        [newButton setImage: [NSImage imageNamed: @"common_Miniaturize"]];
        [newButton setAlternateImage: [NSImage imageNamed: @"common_MiniaturizeH"]];
        [newButton setAction: @selector(miniaturize:)];
        break;

      case NSWindowZoomButton:
        // FIXME
        [newButton setAction: @selector(zoom:)];
        break;

      case NSWindowToolbarButton:
        // FIXME
        [newButton setAction: @selector(toggleToolbarShown:)];
        break;
      case NSWindowDocumentIconButton:
      default:
        // FIXME
        break;
    }

  return AUTORELEASE(newButton);
}

- (void) didSetDefaultButtonCell: (NSButtonCell *)aCell
{
  // default implementation does nothing...
}
@end
