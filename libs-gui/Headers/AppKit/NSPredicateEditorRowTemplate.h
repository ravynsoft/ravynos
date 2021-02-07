/* -*-objc-*-
   NSPredicateEditorRowTemplate.h

   The template rows for the predicate editor

   Copyright (C) 2020 Free Software Foundation, Inc.

   Created by Fabian Spillner on 03.12.07.

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

#ifndef _GNUstep_H_NSPredicateEditorRowTemplate
#define _GNUstep_H_NSPredicateEditorRowTemplate

#import <Foundation/Foundation.h>

@class NSPredicate, NSEntityDescription;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

/* CoreData Framework */
enum {
      NSUndefinedAttributeType  = 0,
      NSInteger16AttributeType  = 100,
      NSInteger32AttributeType  = 200,
      NSInteger64AttributeType  = 300,
      NSDecimalAttributeType    = 400,
      NSDoubleAttributeType     = 500,
      NSFloatAttributeType      = 600,
      NSStringAttributeType     = 700,
      NSBooleanAttributeType    = 800,
      NSDateAttributeType       = 900,
      NSBinaryDataAttributeType = 1000
};
typedef NSUInteger NSAttributeType;

@interface NSPredicateEditorRowTemplate : NSObject {

}

+ (NSArray *) templatesWithAttributeKeyPaths: (NSArray *)paths
                         inEntityDescription: (NSEntityDescription *)entityDesc;

- (NSArray *) compoundTypes;
- (NSArray *) displayableSubpredicatesOfPredicate: (NSPredicate *)pred;
- (id) initWithCompoundTypes: (NSArray *)types;
- (id) initWithLeftExpressions: (NSArray *)leftExprs
  rightExpressionAttributeType: (NSAttributeType)attrType
                      modifier: (NSComparisonPredicateModifier)modif
                     operators: (NSArray *)ops
                       options: (NSUInteger)opts;
- (id) initWithLeftExpressions: (NSArray *)leftExprs
              rightExpressions: (NSArray *)rightExprs
                      modifier: (NSComparisonPredicateModifier)modif
                     operators: (NSArray *)ops
                       options: (NSUInteger)opts;
- (NSArray *) leftExpressions;
- (double) matchForPredicate: (NSPredicate *)pred;
- (NSComparisonPredicateModifier) modifier;
- (NSArray *) operators;
- (NSUInteger) options;
- (NSPredicate *) predicateWithSubpredicates: (NSArray *)subpred;
- (NSAttributeType) rightExpressionAttributeType;
- (NSArray *) rightExpressions;
- (void) setPredicate: (NSPredicate *)pred;
- (NSArray *) templateViews;

@end

#endif
#endif /* _GNUstep_H_NSPredicateEditorRowTemplate */
