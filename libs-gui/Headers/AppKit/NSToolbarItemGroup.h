/* 
   NSToolbarItemGroup.h

   The toolbar item group class.
   
   Copyright (C) 2008 Free Software Foundation, Inc.

   Author:  Fred Kiefer <fredkiefer@gmx.de>
   Date: Dec 2008
   
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

#ifndef _GNUstep_H_NSToolbarItemGroup
#define _GNUstep_H_NSToolbarItemGroup
#import <GNUstepBase/GSVersionMacros.h>

#import <AppKit/NSToolbarItem.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

@class NSArray;

@interface NSToolbarItemGroup : NSToolbarItem
{
	NSArray *_subitems;
}

- (void) setSubitems: (NSArray *)items;
- (NSArray *) subitems;

@end

#endif
#endif /* _GNUstep_H_NSToolbarItemGroup */
