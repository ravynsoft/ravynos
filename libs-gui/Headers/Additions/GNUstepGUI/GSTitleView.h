/** <title>GSTitleView</title>

   Copyright (C) 2003 Free Software Foundation, Inc.

   Author: Serg Stoyan <stoyan@on.com.ua>
   Date:   Mar 2003
   
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

#ifndef _GNUstep_H_GSTitleView
#define _GNUstep_H_GSTitleView

#import <AppKit/NSView.h>

@class NSMutableDictionary;
@class NSButton;
@class NSImage;

@interface GSTitleView : NSView
{
  NSButton            *closeButton;
  NSButton            *miniaturizeButton;
  NSMutableDictionary *textAttributes;
  NSColor             *titleColor;

  @private
    id       _owner;
    unsigned _ownedByMenu;
    unsigned _isKeyWindow;
    unsigned _isMainWindow;
    unsigned _isActiveApplication;
}

// ============================================================================
// ==== Initialization & deallocation
// ============================================================================

+ (float) height;
- (void) setOwner: (id)owner;
- (id) initWithOwner: (id)owner;
- (id) owner;
- (NSSize) titleSize;

// ============================================================================
// ==== Buttons
// ============================================================================

- (void) addCloseButtonWithAction: (SEL)closeAction;
- (NSButton *) closeButton;
- (void) removeCloseButton;
- (void) addMiniaturizeButtonWithAction: (SEL)miniaturizeAction;
- (NSButton *)miniaturizeButton;
- (void) removeMiniaturizeButton;

@end

#endif
