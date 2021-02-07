/*
   Copyright (C) 2013 Free Software Foundation, Inc.

   Author: German A. Arias <german@xelalug.org>
   Date: 2013

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

#import <AppKit/NSPanel.h>

@class GSAutocompleteView;
@class NSArray;
@class NSString;
@class NSNotification;
@class NSNotificationCenter;
@class NSTableColumn;
@class NSTableView;
@class NSTextView;

@interface GSAutocompleteWindow : NSPanel
{
  BOOL _stopped;
  NSRange _range;
  NSTextView *_textView;
  GSAutocompleteView *_tableView;

  //Retained
  NSString *_originalWord;
  NSArray *_words;
}

+ (GSAutocompleteWindow *) defaultWindow;

- (void) layout;
- (void) computePosition;
- (void) displayForTextView: (NSTextView *)textView;
- (NSArray *) words;

- (void) runModalWindow;
- (void) runLoop;
- (void) onWindowEdited: (NSNotification *)notification;

- (void) reloadData;
- (void) updateTextViewWithMovement: (NSInteger)movement isFinal: (BOOL)flag;

- (void) clickItem: (id)sender;
- (void) moveUpSelection;
- (void) moveDownSelection;

// Delegate
- (int) numberOfRowsInTableView: (NSTableView*)aTableView;
- (id) tableView: (NSTableView*)aTableView
      objectValueForTableColumn: (NSTableColumn*)aTableColumn
	     row: (int)rowIndex;
@end
