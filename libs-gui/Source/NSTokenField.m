/** <title>NSTokenField</title>

   <abstract>
   Token field control class for token entry.  The default token is ",".
   </abstract>

   Copyright (C) 2008 Free Software Foundation, Inc.

   Author: Gregory Casamento <greg.casamento@gmail.com>
   Date: July 2008

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

#include "config.h"

#import <Foundation/NSNotification.h>
#import "AppKit/NSApplication.h"
#import "AppKit/NSCursor.h"
#import "AppKit/NSGraphics.h"
#import "AppKit/NSTokenField.h"
#import "AppKit/NSTokenFieldCell.h"
#import "AppKit/NSWindow.h"
#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"

static NSNotificationCenter *nc = nil;

/*
 * Class variables
 */
static Class usedCellClass;
static Class tokenFieldCellClass;

@implementation NSTokenField

//
// Class methods
//
+ (void) initialize
{
  if (self == [NSTokenField class])
    {
      [self setVersion: 1];
      tokenFieldCellClass = [NSTokenFieldCell class];
      usedCellClass = tokenFieldCellClass;
      nc = [NSNotificationCenter defaultCenter];

      [self exposeBinding: NSEditableBinding];
      [self exposeBinding: NSTextColorBinding];
    }
}

- (id) initWithFrame: (NSRect)frame
{
  if((self = [super initWithFrame: frame]) == nil)
    {
      return nil;
    }
  
  // initialize...
  [_cell setTokenStyle: NSDefaultTokenStyle];
  [_cell setCompletionDelay: [_cell defaultCompletionDelay]];
  [_cell setTokenizingCharacterSet: [_cell defaultTokenizingCharacterSet]];
  
  return self;
}

/*
 * Setting the Cell class
 */
+ (Class) cellClass
{
  return usedCellClass;
}

+ (void) setCellClass: (Class)factoryId
{
  usedCellClass = factoryId ? factoryId : tokenFieldCellClass;
}

// Style...
- (NSTokenStyle)tokenStyle
{
  return [_cell tokenStyle];
}

- (void)setTokenStyle:(NSTokenStyle)style
{
  [_cell setTokenStyle: style];
}

// Completion delay...
+ (NSTimeInterval)defaultCompletionDelay
{
  return [usedCellClass defaultCompletionDelay];
}

- (NSTimeInterval)completionDelay
{
  return [_cell completionDelay];
}

- (void)setCompletionDelay:(NSTimeInterval)delay
{
  [_cell setCompletionDelay: delay];
}

// Character set...
+ (NSCharacterSet *)defaultTokenizingCharacterSet
{
  return [usedCellClass defaultTokenizingCharacterSet];
}

- (void)setTokenizingCharacterSet:(NSCharacterSet *)characterSet
{
  [_cell setTokenizingCharacterSet: characterSet];
}

- (NSCharacterSet *)tokenizingCharacterSet
{
  return [_cell tokenizingCharacterSet];
}
@end
