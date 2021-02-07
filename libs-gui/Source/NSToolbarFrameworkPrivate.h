/* 
   <title>NSToolbarFrameworkPrivate.h</title>

   <abstract>Private methods used throughout the toolbar classes.</abstract>
   
   Copyright (C) 2009 Free Software Foundation, Inc.

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

#ifndef _NSToolbarFrameworkPrivate_h_INCLUDE
#define _NSToolbarFrameworkPrivate_h_INCLUDE

#import "AppKit/NSMenuView.h"
#import "AppKit/NSToolbar.h"
#import "AppKit/NSToolbarItem.h"
#import "GNUstepGUI/GSToolbarView.h"
#import "GNUstepGUI/GSWindowDecorationView.h"

@interface GSToolbarView (GNUstepPrivate)
- (void) _reload;

// Accessors
- (CGFloat) _heightFromLayout;
- (NSArray *) _visibleBackViews;

- (BOOL) _usesStandardBackgroundColor;
- (void) _setUsesStandardBackgroundColor: (BOOL)standard;
@end

@interface NSToolbarItem (GNUstepPrivate)
- (void) _layout;
- (void) _computeFlags;

// Accessors
- (NSView *) _backView;
- (NSMenuItem *) _defaultMenuFormRepresentation;
- (BOOL) _isModified;
- (BOOL) _isFlexibleSpace;
- (BOOL) _selectable;
- (void) _setSelectable: (BOOL)selectable;
- (BOOL) _selected;
- (void) _setSelected: (BOOL)selected;
- (void) _setToolbar: (NSToolbar *)toolbar;
@end

@interface NSToolbar (GNUstepPrivate)
// Private class method
+ (NSArray *) _toolbarsWithIdentifier: (NSString *)identifier;

// Private methods with broadcast support
- (void) _insertItemWithItemIdentifier: (NSString *)itemIdentifier 
                               atIndex: (int)index 
                             broadcast: (BOOL)broadcast;
- (void) _removeItemAtIndex: (int)index broadcast: (BOOL)broadcast;
- (void) _setAllowsUserCustomization: (BOOL)flag broadcast: (BOOL)broadcast;
- (void) _setAutosavesConfiguration: (BOOL)flag broadcast: (BOOL)broadcast;
- (void) _setConfigurationFromDictionary: (NSDictionary *)configDict 
                               broadcast: (BOOL)broadcast;
- (void) _moveItemFromIndex: (int)index toIndex: (int)newIndex broadcast: (BOOL)broadcast;
- (void) _setDisplayMode: (NSToolbarDisplayMode)displayMode 
               broadcast: (BOOL)broadcast;
- (void) _setSizeMode: (NSToolbarSizeMode)sizeMode 
            broadcast: (BOOL)broadcast;
- (void) _setVisible: (BOOL)shown broadcast: (BOOL)broadcast;

// Few other private methods
- (void) _build;
- (int) _indexOfItem: (NSToolbarItem *)item;
- (NSToolbar *) _toolbarModel;
- (void) _validate: (NSWindow *)observedWindow;
- (void) _toolbarViewWillMoveToSuperview: (NSView *)newSuperview;
- (NSDictionary *) _config;
- (void) _saveConfig;
- (void) _resetConfig;
- (BOOL) _containsItemWithIdentifier: (NSString *) identifier;

// Accessors
- (void) _setCustomizationPaletteIsRunning: (BOOL)isRunning;
- (void) _setToolbarView: (GSToolbarView *)toolbarView;
- (GSToolbarView *) _toolbarView;

// Deprecated
- (void) setUsesStandardBackgroundColor: (BOOL)standard;
- (BOOL) usesStandardBackgroundColor;

// Delegate wrappers
- (NSArray *) _allowedItemIdentifiers;
- (NSArray *) _defaultItemIdentifiers;
- (NSArray *) _selectableItemIdentifiers;
- (NSToolbarItem *) _toolbarItemForIdentifier: (NSString *)itemIdent willBeInsertedIntoToolbar: (BOOL)insert;

@end

@interface GSWindowDecorationView (ToolbarPrivate)
- (void) addToolbarView: (GSToolbarView*)toolbarView;
- (void) removeToolbarView: (GSToolbarView *)toolbarView;
- (void) adjustToolbarView: (GSToolbarView *)toolbarView;
@end
@interface GSWindowDecorationView (Menu)
- (void) addMenuView: (NSMenuView*)menuView;
- (NSMenuView*) removeMenuView;
@end

#endif // _NSToolbarFrameworkPrivate_h_INCLUDE
