/* 
   NSPopUpButton.h

   Popup list class

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author:  Scott Christley <scottc@net-community.com>
   Date: 1996
   Author:  Michael Hanni <mhanni@sprintmail.com>
   Date: 1999
   
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

#ifndef _GNUstep_H_NSPopUpButton
#define _GNUstep_H_NSPopUpButton

#import <Foundation/NSGeometry.h>

#import <AppKit/AppKitDefines.h>
#import <AppKit/NSButton.h>
#import <AppKit/NSMenuItem.h>

@class NSString;
@class NSArray;


@interface NSPopUpButton : NSButton
{
}

//
// Initializing an NSPopUpButton 
//
- (id)initWithFrame:(NSRect)frameRect
	  pullsDown:(BOOL)flag;

- (void)setPullsDown:(BOOL)flag;
- (BOOL)pullsDown;
- (void)setAutoenablesItems:(BOOL)flag;
- (BOOL)autoenablesItems;
- (void)addItemWithTitle:(NSString *)title;
- (void)addItemsWithTitles:(NSArray *)itemTitles;
- (void)insertItemWithTitle:(NSString *)title   
                    atIndex:(NSInteger)index;
- (void)removeAllItems;
- (void)removeItemWithTitle:(NSString *)title;
- (void)removeItemAtIndex:(NSInteger)index;
- (id <NSMenuItem>)selectedItem;
- (NSString *)titleOfSelectedItem;
- (NSInteger)indexOfSelectedItem;
- (void)selectItem:(id <NSMenuItem>)anObject;
- (void)selectItemAtIndex:(NSInteger)index;
- (void)selectItemWithTitle:(NSString *)title;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (BOOL) selectItemWithTag: (NSInteger)tag;
#endif
- (NSInteger)numberOfItems;
- (NSArray *)itemArray;
- (id <NSMenuItem>)itemAtIndex:(NSInteger)index;
- (NSString *)itemTitleAtIndex:(NSInteger)index;
- (NSArray *)itemTitles;
- (id <NSMenuItem>)itemWithTitle:(NSString *)title;
- (id <NSMenuItem>)lastItem;
- (NSInteger)indexOfItem:(id <NSMenuItem>)anObject;
- (NSInteger)indexOfItemWithTag:(NSInteger)tag;
- (NSInteger)indexOfItemWithTitle:(NSString *)title;
- (NSInteger)indexOfItemWithRepresentedObject:(id)anObject;
- (NSInteger)indexOfItemWithTarget:(id)target
                   andAction:(SEL)actionSelector;
- (void)setPreferredEdge:(NSRectEdge)edge;
- (NSRectEdge)preferredEdge;
- (void)setTitle:(NSString *)aString;
- (void)synchronizeTitleAndSelectedItem;
@end

APPKIT_EXPORT NSString *NSPopUpButtonWillPopUpNotification;

#endif // _GNUstep_H_NSPopUpButton
