/* 
   NSTableColumn.h

   Copyright (C) 1999 Free Software Foundation, Inc.

   Author: Michael Hanni  <mhanni@sprintmail.com>
   Date: 1999

   Author: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 1999

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

#ifndef _GNUstep_H_NSTableColumn
#define _GNUstep_H_NSTableColumn

#import <Foundation/NSObject.h>
#import <AppKit/AppKitDefines.h>

@class NSSortDescriptor;
@class NSCell;
@class NSTableView;

// TODO: Finish to implement hidden, header tool tip and resizing mask 
// and update the archiving code to support them.

/**
Describe the resizing styles accepted by the column.

The final resizing behavior also depends on -[NSTableView columnAutoresizingStyle]. 
The table view uses the resizing mask set on each column and its column 
autoresizing style to determine how to resize each column. */
enum {
  NSTableColumnNoResizing = 0, 
  /** Disallow any resizing. */
  NSTableColumnAutoresizingMask = 1, 
  /** Allow automatic resizing when the table view is resized. */
  NSTableColumnUserResizingMask = 2
  /** Allow the user to resize the column manually. */
};

@interface NSTableColumn : NSObject <NSCoding>
{
  id _identifier;
  NSTableView *_tableView;
  float _width;
  float _min_width;
  float _max_width;
  NSUInteger _resizing_mask;
  BOOL _is_resizable;
  BOOL _is_editable;
  BOOL _is_hidden;
  NSCell *_headerCell;
  NSCell *_dataCell;
  NSString *_headerToolTip;
  NSSortDescriptor *_sortDescriptorPrototype;
}
/* 
 * Initializing an NSTableColumn instance 
 */
- (id) initWithIdentifier: (id)anObject;
/*
 * Managing the Identifier
 */
- (void) setIdentifier: (id)anObject;
- (id) identifier;
/*
 * Setting the NSTableView 
 */
- (void) setTableView: (NSTableView *)aTableView;
- (NSTableView *) tableView;
/*
 * Controlling size & visibility
 */
- (void) setWidth: (CGFloat)newWidth;
- (CGFloat) width; 
- (void) setMinWidth: (CGFloat)minWidth;
- (CGFloat) minWidth; 
- (void) setMaxWidth: (CGFloat)maxWidth;
- (CGFloat) maxWidth; 
- (void) setResizable: (BOOL)flag;
- (BOOL) isResizable;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_4, GS_API_LATEST)
- (void) setResizingMask: (NSUInteger)resizingMask;
- (NSUInteger) resizingMask;
#endif
- (void) sizeToFit;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) setHidden: (BOOL)hidden;
- (BOOL) isHidden;
#endif
/*
 * Controlling editability 
 */
- (void) setEditable: (BOOL)flag;
- (BOOL) isEditable;
/*
 * Setting component cells 
 */
- (void) setHeaderCell: (NSCell *)aCell;
- (NSCell *) headerCell;
#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
- (void) setHeaderToolTip: (NSString *)aString;
- (NSString *) headerToolTip;
#endif
- (void) setDataCell: (NSCell *)aCell; 
- (NSCell *) dataCell;
- (NSCell *) dataCellForRow: (NSInteger)row;
/*
 * Sorting
 */
#if OS_API_VERSION(MAC_OS_X_VERSION_10_3, GS_API_LATEST)
- (void) setSortDescriptorPrototype: (NSSortDescriptor *)aSortDescriptor;
- (NSSortDescriptor *) sortDescriptorPrototype;
#endif
@end

/* Notifications */
APPKIT_EXPORT NSString *NSTableViewColumnDidResizeNotification;
#endif
