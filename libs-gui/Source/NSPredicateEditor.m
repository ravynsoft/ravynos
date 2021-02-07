/** <title>NSPredicateEditor</title>

   <abstract>The predicate editor class</abstract>

   Copyright (C) 2020 Free Software Foundation, Inc.

   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date:   January 2020

   This file is part of the GNUstep GUI Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the
   Free Software Foundation, 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#import "AppKit/NSPredicateEditor.h"


@implementation NSPredicateEditor

- (void) dealloc
{
  RELEASE(_rowTemplates);
  [super dealloc];
}

- (NSArray *) rowTemplates
{
  return _rowTemplates;
}

- (void) setRowTemplates: (NSArray *)templates
{
  ASSIGN(_rowTemplates, templates);
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [super encodeWithCoder: aCoder];

  if ([aCoder allowsKeyedCoding])
    {
      [aCoder encodeObject: _rowTemplates forKey: @"NSRowTemplates"];
    }
  else
    {
      [aCoder encodeObject: _rowTemplates];
    }
}

- (id) initWithCoder: (NSCoder*)aDecoder
{
  self = [super initWithCoder: aDecoder];
  if (nil == self)
    {
      return nil;
    }

  if ([aDecoder allowsKeyedCoding])
    {
      NSArray *rowTemplates = [aDecoder decodeObjectForKey: @"NSRowTemplates"];

      [self setRowTemplates: rowTemplates];
    }
  else
    {
      NSArray *rowTemplates = [aDecoder decodeObject];
      
      [self setRowTemplates: rowTemplates];
    }

  return self;
}

@end
