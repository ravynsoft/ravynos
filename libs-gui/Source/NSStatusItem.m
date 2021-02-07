/*
   NSStatusItem.h

   The status item class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Gregory Casamento <greg.casamento@gmail.com>
   Date: 2013
   Author: Dr. H. Nikolaus Schaller

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

// FIXME: This class is currently a placeholder to allow compilation of
// apps which require NSStatusItem.  Currently there is not a clean,
// cross-platform way to implement this functionality.

#import <Foundation/NSAttributedString.h>
#import <Foundation/NSString.h>

#import <AppKit/NSStatusItem.h>
#import <AppKit/NSMenuItem.h>
#import <AppKit/NSView.h>

@implementation NSStatusItem 

- (id) _initForStatusBar: (NSStatusBar*)bar
              withLength: (CGFloat)len
{
  if ((self = [super init]))
    {
      _statusBar = bar;
      _menuItem = [[NSMenuItem alloc] initWithTitle: @"?"
                                             action: NULL
                                      keyEquivalent: @""];
      [_menuItem setRepresentedObject: self];
      [self setLength: len];
    }

  return self;
}

- (void) dealloc
{
  RELEASE(_menuItem);
  [super dealloc];
}

- (SEL) action
{
  return [_menuItem action];
}

- (NSAttributedString*) attributedTitle
{
  return [_menuItem attributedTitle];
}

- (SEL) doubleAction
{
  // NIMP
  return NULL;
}

- (void) drawStatusBarBackgroundInRect: (NSRect)rect withHighlight: (BOOL)flag
{
  // NIMP
}

- (BOOL) highlightMode
{
  return _highlightMode;
}

- (NSImage*) image
{
  return [_menuItem image];
}

- (BOOL) isEnabled
{
  return [_menuItem isEnabled];
}

- (CGFloat) length
{
  return _length;
}

- (NSMenu *) menu
{
  return [_menuItem submenu];
}

- (void) popUpStatusItemMenu: (NSMenu*)menu
{
  // NIMP
}

- (NSInteger) sendActionOn: (NSInteger)mask
{
  //NIMP
  return 0;
}

- (void) setAction: (SEL)action
{
  [_menuItem setAction: action];
}

- (void) setAttributedTitle: (NSAttributedString*) title
{
  [_menuItem setAttributedTitle: title];
}

- (void) setDoubleAction: (SEL)sel
{
  // NIMP
}

- (void) setEnabled: (BOOL)flag
{
  [_menuItem setEnabled: flag];
}

- (void) setHighlightMode: (BOOL)highlightMode
{
  _highlightMode = highlightMode;
}

- (void) setImage: (NSImage*)image
{
  [_menuItem setImage: image];
}

- (void) setLength: (CGFloat)len
{
  _length = len;
  //[_menuItem _changed];
}

- (void) setMenu: (NSMenu*)menu
{
  [_menuItem setSubmenu: menu]; 
}

- (void) setTarget: (id)target
{
  [_menuItem setTarget: target];
}

- (void) setTitle: (NSString*)title
{
  [_menuItem setTitle: title];
}

- (void) setToolTip: (NSString*)toolTip
{
  [_menuItem setToolTip: toolTip];
}

- (void) setView: (NSView*)view
{
  ASSIGN(_view, view);
}

- (NSStatusBar*) statusBar
{
  return _statusBar;
}

- (id) target
{
  return [_menuItem target];
}

- (NSString*) title
{
  return [_menuItem title];
}

- (NSString*) toolTip
{
  return [_menuItem toolTip];
}

- (NSView*) view
{
  return _view;
}

- (NSImage*) alternateImage
{
  //NIMP
  return nil;
}

- (void) setAlternateImage: (NSImage*)img
{
  //NIMP
}

@end
