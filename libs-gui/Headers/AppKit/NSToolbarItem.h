/* 
   NSToolbarItem.h

   The toolbar item class.
   
   Copyright (C) 2002 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>,
            Fabien Vallon <fabien.vallon@fr.alcove.com>,
            Quentin Mathe <qmathe@club-internet.fr>
   Date: May 2002
   
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

#ifndef _GNUstep_H_NSToolbarItem
#define _GNUstep_H_NSToolbarItem
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <Foundation/NSGeometry.h>
#import <AppKit/AppKitDefines.h>
#import <AppKit/NSUserInterfaceValidation.h>

@class NSArray;
@class NSString;
@class NSDictionary;
@class NSMutableDictionary;
@class NSImage;
@class NSMenuItem;
@class NSView;
@class NSToolbar;

/*
 * Constants
 */
APPKIT_EXPORT NSString *NSToolbarSeparatorItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarSpaceItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarFlexibleSpaceItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarShowColorsItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarShowFontsItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarCustomizeToolbarItemIdentifier;
APPKIT_EXPORT NSString *NSToolbarPrintItemIdentifier;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
enum _NSToolbarItemVisibilityPriority {
  NSToolbarItemVisibilityPriorityStandard = 0,
  NSToolbarItemVisibilityPriorityLow = -1000,
  NSToolbarItemVisibilityPriorityHigh = 1000,
  NSToolbarItemVisibilityPriorityUser = 2000
};
#endif

@interface NSToolbarItem : NSObject <NSCopying, NSValidatedUserInterfaceItem>
{
  // externally visible variables
  BOOL _autovalidates;
  NSString *_itemIdentifier;
  NSString *_label;
  NSString *_paletteLabel;
  NSImage *_image;
  id _view;
  NSMenuItem *_menuFormRepresentation;
  NSString *_toolTip;
  NSInteger _tag;
  NSInteger _visibilityPriority;

  // toolbar
  NSToolbar *_toolbar;
  NSView *_backView;
  BOOL _modified;
  BOOL _selectable;

  // size
  NSSize _maxSize;
  NSSize _minSize;
}

// Instance methods
- (id)initWithItemIdentifier: (NSString *)itemIdentifier;

- (void)validate;

// Accessors
- (SEL) action;
- (BOOL) allowsDuplicatesInToolbar;
- (NSImage *) image;
- (BOOL) isEnabled;
- (NSString *) itemIdentifier;
- (NSString *) label;
- (NSSize) maxSize;
- (NSMenuItem *) menuFormRepresentation;
- (NSSize) minSize;
- (NSString *) paletteLabel;
- (NSInteger) tag;
- (id) target;
- (NSString *) toolTip;
- (NSToolbar *) toolbar;
- (NSView *) view;
- (void) setAction: (SEL)action;
- (void) setEnabled: (BOOL)enabled;
- (void) setImage: (NSImage *)image;
- (void) setLabel: (NSString *)label;
- (void) setMaxSize: (NSSize)maxSize;
- (void) setMenuFormRepresentation: (NSMenuItem *)menuItem;
- (void) setMinSize: (NSSize)minSize;
- (void) setPaletteLabel: (NSString *)paletteLabel;
- (void) setTag: (NSInteger)tag;
- (void) setTarget: (id)target;
- (void) setToolTip: (NSString *)toolTip;
- (void) setView: (NSView *)view;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) autovalidates;
- (void) setAutovalidates: (BOOL)autovalidates;
- (NSInteger) visibilityPriority;
- (void) setVisibilityPriority: (NSInteger)visibilityPriority;
#endif

@end /* interface of NSToolbarItem */

// Informal protocol for the toolbar validation
@interface NSObject (NSToolbarItemValidation)
- (BOOL) validateToolbarItem: (NSToolbarItem *)toolbarItem;
@end

// Extra private stuff
APPKIT_EXPORT NSString *GSMovableToolbarItemPboardType;

#endif /* _GNUstep_H_NSToolbarItem */
