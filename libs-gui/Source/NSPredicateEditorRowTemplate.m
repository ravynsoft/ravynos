/** <title>NSPredicateEditorRowTemplate</title>

   <abstract>The template rows for the predicate editor</abstract>

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

#import "AppKit/NSPredicateEditorRowTemplate.h"

@implementation NSPredicateEditorRowTemplate

+ (NSArray *) templatesWithAttributeKeyPaths: (NSArray *)paths
                         inEntityDescription: (NSEntityDescription *)entityDesc
{
  return nil;
}

- (NSArray *) compoundTypes
{
  return nil;
}

- (NSArray *) displayableSubpredicatesOfPredicate: (NSPredicate *)pred
{
  return nil;
}

- (id) initWithCompoundTypes: (NSArray *)types
{
  return self;
}

- (id) initWithLeftExpressions: (NSArray *)leftExprs
  rightExpressionAttributeType: (NSAttributeType)attrType
                      modifier: (NSComparisonPredicateModifier)modif
                     operators: (NSArray *)ops
                       options: (NSUInteger)opts;
{
  return self;
}

- (id) initWithLeftExpressions: (NSArray *)leftExprs
              rightExpressions: (NSArray *)rightExprs
                      modifier: (NSComparisonPredicateModifier)modif
                     operators: (NSArray *)ops
                       options: (NSUInteger)opts;
{
  return self;
}

- (NSArray *) leftExpressions
{
  return nil;
}

- (double) matchForPredicate: (NSPredicate *)pred
{
  return 0.0;
}

- (NSComparisonPredicateModifier) modifier
{
  return NSDirectPredicateModifier;
}

- (NSArray *) operators
{
  return nil;
}

- (NSUInteger) options
{
  return 0;
}

- (NSPredicate *) predicateWithSubpredicates: (NSArray *)subpred
{
  return nil;
}

- (NSAttributeType) rightExpressionAttributeType
{
  return 0;
}

- (NSArray *) rightExpressions
{
  return nil;
}

- (void) setPredicate: (NSPredicate *)pred
{
}

- (NSArray *) templateViews
{
  return nil;
}

@end
