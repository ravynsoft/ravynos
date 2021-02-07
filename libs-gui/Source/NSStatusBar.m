/*
   NSStatusBar.m

   The status bar class

   Copyright (C) 2013 Free Software Foundation, Inc.

   Author:  Dr. H. Nikolaus Schaller
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

#import <Foundation/NSArray.h>
#import <AppKit/NSStatusBar.h>
#import <AppKit/NSStatusItem.h>

@interface NSStatusItem (Private)
- (id) _initForStatusBar: (NSStatusBar*)bar
              withLength: (CGFloat)len;
@end


@implementation NSStatusBar

- (id) init
{
  self = [super init];
  if (self)
    {
      _items = [[NSMutableArray alloc] init];
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_items);
  [super dealloc];
}

+ (NSStatusBar*) systemStatusBar
{
  return nil;
}

- (BOOL) isVertical
{
  return NO;
}

- (void) removeStatusItem: (NSStatusItem*)item
{
  [_items removeObjectIdenticalTo: item];
}

- (NSStatusItem*) statusItemWithLength: (CGFloat)length
{
  NSStatusItem *item = [[NSStatusItem alloc] _initForStatusBar: self
                                                    withLength: length];
  [_items addObject: item];

  return AUTORELEASE(item);
}

- (CGFloat) thickness
{
  return 22;
}

@end
