/* 
   NSToolbar.h

   The toolbar class.
   
   Copyright (C) 2002, 2004, 2009 Free Software Foundation, Inc.

   Author:  Gregory John Casamento <greg_casamento@yahoo.com>,
            Fabien Vallon <fabien.vallon@fr.alcove.com>,
            Quentin Mathe <qmathe@club-internet.fr>
   Date: May 2002, February 2004
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: January 2009
   
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

#ifndef _GNUstep_H_NSToolbar
#define _GNUstep_H_NSToolbar
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSArray;
@class NSString;
@class NSMutableArray;
@class NSDictionary;
@class NSMutableDictionary;
@class NSNotification;
@class NSToolbarItem;
@class GSToolbarView;
@class NSWindow;

/*
 * Constants
 */

typedef enum 
{ 
  NSToolbarDisplayModeDefault,
  NSToolbarDisplayModeIconAndLabel,
  NSToolbarDisplayModeIconOnly,
  NSToolbarDisplayModeLabelOnly
} NSToolbarDisplayMode;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
typedef enum 
{ 
  NSToolbarSizeModeDefault,
  NSToolbarSizeModeRegular,
  NSToolbarSizeModeSmall,
} NSToolbarSizeMode;
#endif

APPKIT_EXPORT NSString *NSToolbarDidRemoveItemNotification;
APPKIT_EXPORT NSString *NSToolbarWillAddItemNotification;

typedef NSString* NSToolbarItemIdentifier;

@interface NSToolbar : NSObject
{
  NSDictionary *_configurationDictionary;
  id _delegate;
  NSString *_identifier;
  NSString *_selectedItemIdentifier;
  NSMutableArray *_items;
  GSToolbarView *_toolbarView;
  NSToolbarDisplayMode _displayMode;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
  NSToolbarSizeMode _sizeMode;
#else
  int _sizeMode;
#endif
  BOOL _allowsUserCustomization;
  BOOL _autosavesConfiguration;
  BOOL _visible;
  BOOL _customizationPaletteIsRunning;
  BOOL _showsBaselineSeparator;
  BOOL _build;
  NSDictionary *_interfaceBuilderItemsByIdentifier;
  NSArray *_interfaceBuilderAllowedItemIdentifiers;
  NSArray *_interfaceBuilderDefaultItemIdentifiers;
  NSArray *_interfaceBuilderSelectableItemIdentifiers;
}

// Instance methods
- (id) initWithIdentifier: (NSString*)identifier;

- (void) insertItemWithItemIdentifier: (NSString*)itemIdentifier 
         atIndex: (NSInteger)index;
- (void) removeItemAtIndex: (NSInteger)index;
- (void) runCustomizationPalette: (id)sender;
- (void) validateVisibleItems;

// Accessors
- (BOOL) allowsUserCustomization;
- (BOOL) autosavesConfiguration;
- (NSDictionary*) configurationDictionary;
- (BOOL) customizationPaletteIsRunning;
- (id) delegate;
- (NSToolbarDisplayMode) displayMode;
- (NSString*) identifier;
- (NSArray*) items;
- (NSArray*) visibleItems;
- (BOOL) isVisible;
- (void) setAllowsUserCustomization: (BOOL)flag;
- (void) setAutosavesConfiguration: (BOOL)flag;
- (void) setConfigurationFromDictionary: (NSDictionary*)configDict;
- (void) setDelegate: (id)delegate;
- (void) setDisplayMode: (NSToolbarDisplayMode)displayMode;
- (void) setVisible: (BOOL)shown;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
- (NSToolbarSizeMode) sizeMode;
- (void) setSizeMode: (NSToolbarSizeMode)sizeMode;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (NSString *) selectedItemIdentifier;
- (void) setSelectedItemIdentifier: (NSString *) identifier;
#endif
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) showsBaselineSeparator;
- (void) setShowsBaselineSeparator: (BOOL)flag;
#endif

@end /* interface of NSToolbar */

/*
 * Methods Implemented by the Delegate
 */
@interface NSObject (NSToolbarDelegate)
// notification methods
- (void) toolbarDidRemoveItem: (NSNotification*)aNotification;
- (void) toolbarWillAddItem: (NSNotification*)aNotification;

// delegate methods
// required method
- (NSToolbarItem*)toolbar: (NSToolbar*)toolbar
    itemForItemIdentifier: (NSString*)itemIdentifier
willBeInsertedIntoToolbar: (BOOL)flag;
// required method
- (NSArray*) toolbarAllowedItemIdentifiers: (NSToolbar*)toolbar;
// required method
- (NSArray*) toolbarDefaultItemIdentifiers: (NSToolbar*)toolbar;
// optional method
- (NSArray *) toolbarSelectableItemIdentifiers: (NSToolbar *)toolbar;
@end

#endif /* _GNUstep_H_NSToolbar */
