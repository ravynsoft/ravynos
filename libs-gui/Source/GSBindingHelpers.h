/** Private Bindings helper functions for GNUstep

   Copyright (C) 2007 Free Software Foundation, Inc.

   Written by: Chris Farber <chris@chrisfarber.net>
   Date: 2007
   Author: Fred Kiefer <fredkiefer@gmx.de>
   Date: Decembre 2007

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

#ifndef _GS_BINDING_HELPER_H
#define _GS_BINDING_HELPER_H

#import <Foundation/NSObject.h>

@class NSString;
@class NSDictionary;
@class NSMutableDictionary;
@class NSArray;

@interface GSKeyValueBinding : NSObject
{
@public
  NSDictionary *info;
  id src;
}

+ (void) exposeBinding: (NSString *)binding forClass: (Class)clazz;
+ (NSArray *) exposedBindingsForClass: (Class)clazz;
+ (GSKeyValueBinding *) getBinding: (NSString *)binding 
                         forObject: (id)anObject;
+ (NSDictionary *) infoForBinding: (NSString *)binding 
                        forObject: (id)anObject;
+ (void) unbind: (NSString *)binding  forObject: (id)anObject;
+ (void) unbindAllForObject: (id)anObject;

- (id) initWithBinding: (NSString *)binding 
              withName: (NSString *)name
              toObject: (id)dest
           withKeyPath: (NSString *)keyPath
               options: (NSDictionary *)options
            fromObject: (id)source;
- (void) setValueFor: (NSString *)binding;
- (void) reverseSetValue: (id)value;
- (void) reverseSetValueFor: (NSString *)binding;
- (id) destinationValue;
- (id) sourceValueFor: (NSString *)binding;

/* Transforms the value with a value transformer, if specified and available,
 * and takes care of any placeholders
 */
- (id) transformValue: (id)value withOptions: (NSDictionary *)options;
- (id) reverseTransformValue: (id)value withOptions: (NSDictionary *)options;

@end

@interface GSKeyValueOrBinding : GSKeyValueBinding 
@end

@interface GSKeyValueAndBinding : GSKeyValueBinding 
@end

@interface GSObservableArray : NSArray
{
  NSArray *_array;
}
@end

#endif //_GS_BINDING_HELPER_H
