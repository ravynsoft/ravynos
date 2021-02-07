/* 
   NSSearchField.m

   Text field control class for text search

   Copyright (C) 1996 Free Software Foundation, Inc.

   Author: H. Nikolaus Schaller <hns@computer.org>
   Date: Dec 2004
   
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

#import <Foundation/NSString.h>
#import <Foundation/NSArray.h>

#import "AppKit/NSSearchField.h"
#import "AppKit/NSSearchFieldCell.h"

/*
 * Class variables
 */
static Class usedCellClass;

@implementation NSSearchField

/*
 * Class methods
 */
+ (void) initialize
{
  if (self == [NSSearchField class])
    {
      [self setVersion: 1];
      usedCellClass = [NSSearchFieldCell class];
    }
}

+ (Class) cellClass
{ 
  return usedCellClass; 
}

+ (void) setCellClass: (Class)factoryId
{ 
  usedCellClass = factoryId ? factoryId : [NSSearchFieldCell class];
}

- (NSArray*) recentSearches
{ 
  return [[self cell] recentSearches];
}

- (NSString*) recentsAutosaveName
{
  return [[self cell] recentsAutosaveName];
}

- (void) setRecentSearches: (NSArray*)searches 
{
  [[self cell] setRecentSearches: searches];
}

- (void) setRecentsAutosaveName: (NSString*)name
{
  [[self cell] setRecentsAutosaveName: name];
}

//
// Handling Events
//
- (void) mouseDown: (NSEvent*)theEvent
{
  [[self cell] trackMouse: theEvent 
                   inRect: [self bounds] 
                   ofView: self 
             untilMouseUp: YES];
}

- (void) delete: (id)sender
{
  // this may need to do more (like send action), but start here...
  [self setStringValue: @""];
  [[self cell] performClick: self];
}

// Cocoa only defines these methods on the cell, but nib loading targets the field itself
- (void) setSearchMenuTemplate: (NSMenu*)newTemplate
{
  [[self cell] setSearchMenuTemplate: newTemplate];
}

- (void) setSendsWholeSearchString: (BOOL)flag
{
  [[self cell] setSendsWholeSearchString: flag];
}

- (void) setSendsSearchStringImmediately: (BOOL)flag
{
  [[self cell] setSendsSearchStringImmediately: flag];
}

- (void) setMaximumRecents: (NSInteger)max
{
  [[self cell] setMaximumRecents: max];
}

@end /* NSSearchField */
