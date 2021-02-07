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

#import <Foundation/NSArray.h>
#import "AppKit/NSToolbarItemGroup.h"

@implementation NSToolbarItemGroup
// FIXME: Most of the implementation is missing.

- (void) setSubitems: (NSArray *)items
{
  ASSIGN(_subitems, items);
}

- (NSArray *) subitems
{
  return _subitems;
}

- (void) dealloc
{
  RELEASE(_subitems);
  
  [super dealloc];
}

// NSCopying protocol
- (id) copyWithZone: (NSZone *)zone 
{
  NSToolbarItemGroup *new = (NSToolbarItemGroup *)[super copyWithZone: zone];
  
  [new setSubitems: [self subitems]];

  return new;
}

@end

