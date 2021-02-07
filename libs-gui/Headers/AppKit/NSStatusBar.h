/*
   NSStatusBar.h

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
#ifndef _GNUstep_H_NSStatusBar
#define _GNUstep_H_NSStatusBar

#import <Foundation/Foundation.h>

@class NSStatusItem;

@interface NSStatusBar : NSObject
{
  @private
  NSMutableArray *_items;
}

#ifndef NSSquareStatusItemLength
// length == thickness
#define NSSquareStatusItemLength ((CGFloat) -2.0)
// variable
#define NSVariableStatusItemLength ((CGFloat) -1.0)
#endif

+ (NSStatusBar*) systemStatusBar;

- (BOOL) isVertical;
- (void) removeStatusItem: (NSStatusItem*)item;
- (NSStatusItem*) statusItemWithLength: (CGFloat)length;
- (CGFloat) thickness;

@end
#endif // _GNUstep_H_NSStatusBar
