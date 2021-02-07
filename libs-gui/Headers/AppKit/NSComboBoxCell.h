/*
   NSComboBoxCell.h

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

#ifndef _GNUstep_H_NSComboBoxCell
#define _GNUstep_H_NSComboBoxCell
#import <GNUstepBase/GSVersionMacros.h>

#import <Foundation/NSGeometry.h>
#import <AppKit/NSTextFieldCell.h>

@class NSButtonCell;
@class NSMutableArray;
@class NSArray;
@class NSString;

@interface NSComboBoxCell : NSTextFieldCell
{
   id			_dataSource;
   NSButtonCell		*_buttonCell;
   NSMutableArray	*_popUpList;
   BOOL			_usesDataSource;
   BOOL			_hasVerticalScroller;
   BOOL                 _completes;
   NSInteger		_visibleItems;
   NSSize		_intercellSpacing;
   float		_itemHeight;
   NSInteger            _selectedItem;
   NSRect               _lastValidFrame;
   NSRange		_prevSelectedRange;
  
@private
   id		        _popup;
}

- (BOOL)hasVerticalScroller;
- (void)setHasVerticalScroller:(BOOL)flag;

- (NSSize)intercellSpacing; 
- (void)setIntercellSpacing:(NSSize)aSize; 

- (CGFloat)itemHeight;
- (void)setItemHeight:(CGFloat)itemHeight; 

- (NSInteger)numberOfVisibleItems;
- (void)setNumberOfVisibleItems:(NSInteger)visibleItems;

- (void)reloadData;
- (void)noteNumberOfItemsChanged;

- (BOOL)usesDataSource;
- (void)setUsesDataSource:(BOOL)flag;

- (void)scrollItemAtIndexToTop:(NSInteger)index;
- (void)scrollItemAtIndexToVisible:(NSInteger)index;

- (void)selectItemAtIndex:(NSInteger)index;
- (void)deselectItemAtIndex:(NSInteger)index;
- (NSInteger)indexOfSelectedItem;
- (NSInteger)numberOfItems;

/* These two methods can only be used when usesDataSource is YES */
- (id)dataSource;
- (void)setDataSource:(id)aSource; 

/* These methods can only be used when usesDataSource is NO */
- (void)addItemWithObjectValue:(id)object;
- (void)addItemsWithObjectValues:(NSArray *)objects;
- (void)insertItemWithObjectValue:(id)object atIndex:(NSInteger)index;
- (void)removeItemWithObjectValue:(id)object;
- (void)removeItemAtIndex:(NSInteger)index;
- (void)removeAllItems;
- (void)selectItemWithObjectValue:(id)object;
- (id)itemObjectValueAtIndex:(NSInteger)index;
- (id)objectValueOfSelectedItem;
- (NSInteger)indexOfItemWithObjectValue:(id)object;
- (NSArray *)objectValues;

- (BOOL) trackMouse: (NSEvent *)theEvent 
	     inRect: (NSRect)cellFrame
	     ofView: (NSView *)controlView 
       untilMouseUp: (BOOL)flag;

#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* text completion */
- (NSString *)completedString:(NSString *)substring;
- (void)setCompletes:(BOOL)completes;
- (BOOL)completes;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (BOOL) isButtonBordered;
- (void) setButtonBordered:(BOOL)flag;
#endif
@end

@protocol NSComboBoxCellDataSource <NSObject>
#if OS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST) && GS_PROTOCOLS_HAVE_OPTIONAL
@optional
#else
@end
@interface NSObject (NSComboBoxCellDataSource)
#endif
- (NSInteger)numberOfItemsInComboBoxCell:(NSComboBoxCell *)comboBoxCell;
- (id)comboBoxCell:(NSComboBoxCell *)aComboBoxCell 
  objectValueForItemAtIndex:(NSInteger)index;
- (NSUInteger)comboBoxCell:(NSComboBoxCell *)aComboBoxCell
  indexOfItemWithStringValue:(NSString *)string;
#if OS_API_VERSION(GS_API_MACOSX, GS_API_LATEST)
/* text completion */
- (NSString *)comboBoxCell:(NSComboBoxCell *)aComboBoxCell 
	   completedString:(NSString *)uncompletedString;
#endif
@end

#endif /* _GNUstep_H_NSComboBoxCell */
