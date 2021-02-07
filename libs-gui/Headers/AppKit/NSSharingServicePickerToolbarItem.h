/* Definition of class NSSharingServicePickerToolbarItem
   Copyright (C) 2020 Free Software Foundation, Inc.
   
   By: Gregory John Casamento
   Date: Tue Apr  7 08:11:46 EDT 2020

   This file is part of the GNUstep Library.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.
*/

#ifndef _NSSharingServicePickerToolbarItem_h_GNUSTEP_GUI_INCLUDE
#define _NSSharingServicePickerToolbarItem_h_GNUSTEP_GUI_INCLUDE

#import "AppKit/NSToolbarItem.h"

#if OS_API_VERSION(MAC_OS_X_VERSION_10_15, GS_API_LATEST)

#if	defined(__cplusplus)
extern "C" {
#endif

@protocol NSSharingServicePickerToolbarItemDelegate;
  
@interface NSSharingServicePickerToolbarItem : NSToolbarItem

- (id) activityItemsConfiguration;

- (void) setActivityItemsConfiguration: (id)items;

- (id<NSSharingServicePickerToolbarItemDelegate>) delegate;

- (void) setDelegate: (id<NSSharingServicePickerToolbarItemDelegate>) delegate;
  
@end

@protocol NSSharingServicePickerToolbarItemDelegate

- (NSArray *) itemsForSharingServicePickerToolbarItem: (NSSharingServicePickerToolbarItem *)items;

@end

#if	defined(__cplusplus)
}
#endif

#endif	/* GS_API_MACOSX */

#endif	/* _NSSharingServicePickerToolbarItem_h_GNUSTEP_GUI_INCLUDE */

