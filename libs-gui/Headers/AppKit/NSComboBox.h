/*
   NSComboBox.h

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author:  Gerrit van Dyk <gerritvd@decillion.net>
   Date: 1999

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, write to the Free Software Foundation,
   51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _GNUstep_H_NSComboBox
#define _GNUstep_H_NSComboBox
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSTextField.h>

@class NSArray;
@class NSString;
@class NSNotification;

@interface NSComboBox : NSTextField
{
}

- (BOOL) hasVerticalScroller;
- (void) setHasVerticalScroller: (BOOL)flag;

- (NSSize) intercellSpacing;
- (void) setIntercellSpacing: (NSSize)aSize;

- (CGFloat) itemHeight;
- (void) setItemHeight: (CGFloat)itemHeight;

- (NSInteger) numberOfVisibleItems;
- (void) setNumberOfVisibleItems: (NSInteger)visibleItems;

- (void) reloadData;
- (void) noteNumberOfItemsChanged;

- (BOOL) usesDataSource;
- (void) setUsesDataSource: (BOOL)flag;

- (void) scrollItemAtIndexToTop: (NSInteger)index;
- (void) scrollItemAtIndexToVisible: (NSInteger)index;

- (void) selectItemAtIndex: (NSInteger)index;
- (void) deselectItemAtIndex: (NSInteger)index;
- (NSInteger) indexOfSelectedItem;
- (NSInteger) numberOfItems;

/* These two methods can only be used when usesDataSource is YES */
- (id) dataSource;
- (void) setDataSource: (id)aSource;

/* These methods can only be used when usesDataSource is NO */
- (void) addItemWithObjectValue: (id)object;
- (void) addItemsWithObjectValues: (NSArray *)objects;
- (void) insertItemWithObjectValue: (id)object atIndex:(NSInteger)index;
- (void) removeItemWithObjectValue: (id)object;
- (void) removeItemAtIndex: (NSInteger)index;
- (void) removeAllItems;
- (void) selectItemWithObjectValue: (id)object;
- (id) itemObjectValueAtIndex: (NSInteger)index;
- (id) objectValueOfSelectedItem;
- (NSInteger) indexOfItemWithObjectValue: (id)object;
- (NSArray *) objectValues;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* text completion */
- (void) setCompletes: (BOOL)completes;
- (BOOL) completes;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) isButtonBordered;
- (void) setButtonBordered:(BOOL)flag;
#endif
@end

@interface NSObject (NSComboBoxDataSource)
- (NSInteger) numberOfItemsInComboBox: (NSComboBox *)aComboBox;
- (id) comboBox: (NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index;
- (NSUInteger) comboBox: (NSComboBox *)aComboBox 
  indexOfItemWithStringValue: (NSString *)string;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* text completion */
- (NSString *) comboBox: (NSComboBox *)aComboBox 
	completedString: (NSString *)aString;
#endif
@end

@interface NSObject (NSComboBoxNotifications)
- (void) comboBoxWillPopUp: (NSNotification *)notification;
- (void) comboBoxWillDismiss: (NSNotification *)notification;
- (void) comboBoxSelectionDidChange: (NSNotification *)notification;
- (void) comboBoxSelectionIsChanging: (NSNotification *)notification;
@end

APPKIT_EXPORT	NSString *NSComboBoxWillPopUpNotification;
APPKIT_EXPORT	NSString *NSComboBoxWillDismissNotification;
APPKIT_EXPORT	NSString *NSComboBoxSelectionDidChangeNotification;
APPKIT_EXPORT	NSString *NSComboBoxSelectionIsChangingNotification;

#endif /* _GNUstep_H_NSComboBox */
