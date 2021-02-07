/**  <title>NSKeyValueBinding informal protocol reference</title>

   Implementation of KeyValueBinding for GNUStep

   Copyright (C) 2007 Free Software Foundation, Inc.

   Written by:  Chris Farber <chris@chrisfarber.net>
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
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.
   If not, see <http://www.gnu.org/licenses/> or write to the 
   Free Software Foundation, 51 Franklin Street, Fifth Floor, 
   Boston, MA 02110-1301, USA.
*/

#import <Foundation/NSArray.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSException.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSKeyValueCoding.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSMapTable.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSValueTransformer.h>
#import <GNUstepBase/GSLock.h>

#import "AppKit/NSKeyValueBinding.h"
#import "GSBindingHelpers.h"
#import "GSFastEnumeration.h"

@implementation NSObject (NSKeyValueBindingCreation)

+ (void) exposeBinding: (NSString *)binding
{
  [GSKeyValueBinding exposeBinding: binding forClass: [self class]];
}

- (NSArray *) exposedBindings
{
  NSMutableArray *exposedBindings = [NSMutableArray array];
  NSArray *tmp;
  Class class = [self class];

  while (class && class != [NSObject class])
    {
      tmp = [GSKeyValueBinding exposedBindingsForClass: class];
      if (tmp != nil)
        {
          [exposedBindings addObjectsFromArray: tmp];
        }
  
      class = [class superclass];
    }

  return exposedBindings;
}

- (Class) valueClassForBinding: (NSString *)binding
{
  return [NSString class];
}

- (void) bind: (NSString *)binding 
     toObject: (id)anObject
  withKeyPath: (NSString *)keyPath
      options: (NSDictionary *)options
{
  if ((anObject == nil)
      || (keyPath == nil))
    {
      NSLog(@"No object or path for binding on %@ for %@", self, binding);
      return;
    }

  if ([[self exposedBindings] containsObject: binding])
    {
      GSKeyValueBinding *kvb;

      [self unbind: binding];
      kvb = [[GSKeyValueBinding alloc] initWithBinding: binding 
                                              withName: binding 
                                              toObject: anObject
                                           withKeyPath: keyPath
                                               options: options
                                            fromObject: self];
      // The binding will be retained in the binding table
      RELEASE(kvb);
    }
  else
    {
      NSLog(@"No binding exposed on %@ for %@", self, binding);
    }
}

- (NSDictionary *) infoForBinding: (NSString *)binding
{
  return [GSKeyValueBinding infoForBinding: binding forObject: self];
}

- (void) unbind: (NSString *)binding
{
  [GSKeyValueBinding unbind: binding forObject: self];
}

@end

static NSRecursiveLock *bindingLock = nil;
static NSMapTable *classTable = NULL;      //available bindings
static NSMapTable *objectTable = NULL;     //bound bindings

typedef enum {
  GSBindingOperationAnd = 0,
  GSBindingOperationOr
} GSBindingOperationKind;

//TODO: document
BOOL GSBindingResolveMultipleValueBool(NSString *key, NSDictionary *bindings,
    GSBindingOperationKind operationKind);

//TODO: document
void GSBindingInvokeAction(NSString *targetKey, NSString *argumentKey,
    NSDictionary *bindings);

@implementation GSKeyValueBinding

+ (void) initialize
{
  if (self == [GSKeyValueBinding class])
    {
      bindingLock = [GSLazyRecursiveLock new];
      classTable = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
          NSObjectMapValueCallBacks, 128);
      objectTable = NSCreateMapTable(NSNonRetainedObjectMapKeyCallBacks,
          NSObjectMapValueCallBacks, 128);
    }
}

+ (void) exposeBinding: (NSString *)binding forClass: (Class)clazz
{
  NSMutableArray *bindings;

  [bindingLock lock];
  bindings = (NSMutableArray *)NSMapGet(classTable, (void*)clazz);
  if (bindings == nil)
    {
      bindings = [[NSMutableArray alloc] initWithCapacity: 5];
      NSMapInsert(classTable, (void*)clazz, (void*)bindings);
      RELEASE(bindings);
    }
  [bindings addObject: binding];
  [bindingLock unlock];
}

+ (NSArray *) exposedBindingsForClass: (Class)clazz
{
  NSArray *tmp;

  if (!classTable)
    return nil;

  [bindingLock lock];
  tmp = NSMapGet(classTable, (void*)clazz);
  [bindingLock unlock];
  
  return tmp;
}

+ (GSKeyValueBinding *) getBinding: (NSString *)binding 
                         forObject: (id)anObject
{
  NSMutableDictionary *bindings;
  GSKeyValueBinding *theBinding = nil;

  if (!objectTable)
    return nil;

  [bindingLock lock];
  bindings = (NSMutableDictionary *)NSMapGet(objectTable, (void *)anObject);
  if (bindings != nil)
    {
      theBinding = (GSKeyValueBinding*)[bindings objectForKey: binding];
    }
  [bindingLock unlock];

  return theBinding;
}

+ (NSDictionary *) infoForBinding: (NSString *)binding 
                        forObject: (id)anObject
{
  GSKeyValueBinding *theBinding;

  theBinding = [self getBinding: binding forObject: anObject];
  if (theBinding == nil)
    return nil;

  return theBinding->info;
}

+ (void) unbind: (NSString *)binding 
      forObject: (id)anObject
{
  NSMutableDictionary *bindings;
  id observedObject;
  NSString *keyPath;
  GSKeyValueBinding *theBinding;

  if (!objectTable)
    return;

  [bindingLock lock];
  bindings = (NSMutableDictionary *)NSMapGet(objectTable, (void *)anObject);
  if (bindings != nil)
    {
      theBinding = (GSKeyValueBinding*)[bindings objectForKey: binding];
      if (theBinding != nil)
        {
          observedObject = [theBinding->info objectForKey: NSObservedObjectKey];
          keyPath = [theBinding->info objectForKey: NSObservedKeyPathKey];
          [observedObject removeObserver: theBinding forKeyPath: keyPath];
          [bindings setValue: nil forKey: binding];
        }
    }
  [bindingLock unlock];
}

+ (void) unbindAllForObject: (id)anObject
{
  NSEnumerator *enumerator;
  NSString *binding;
  NSDictionary *list;

  if (!objectTable)
    return;

  [bindingLock lock];
  list = (NSDictionary *)NSMapGet(objectTable, (void *)anObject);
  if (list != nil)
    {
      NSArray *keys = [list allKeys];

      enumerator = [keys objectEnumerator];
      while ((binding = [enumerator nextObject]))
        {
          [anObject unbind: binding];
        }
      NSMapRemove(objectTable, (void *)anObject);
    }
  [bindingLock unlock];
}

- (id) initWithBinding: (NSString *)binding 
              withName: (NSString *)name
              toObject: (id)dest
           withKeyPath: (NSString *)keyPath
               options: (NSDictionary *)options
            fromObject: (id)source
{
  NSMutableDictionary *bindings;
  
  src = source;
  if (options == nil)
    {
      info = [[NSDictionary alloc] initWithObjectsAndKeys:
        dest, NSObservedObjectKey,
        keyPath, NSObservedKeyPathKey,
        nil];
    }
  else
    {
      info = [[NSDictionary alloc] initWithObjectsAndKeys:
        dest, NSObservedObjectKey,
        keyPath, NSObservedKeyPathKey,
        options, NSOptionsKey,
        nil];
    }
    
  [dest addObserver: self
        forKeyPath: keyPath
        options: NSKeyValueObservingOptionNew
        context: binding];

  [bindingLock lock];
  bindings = (NSMutableDictionary *)NSMapGet(objectTable, (void *)source);
  if (bindings == nil)
    {
      bindings = [NSMutableDictionary new];
      NSMapInsert(objectTable, (void*)source, (void*)bindings);
      RELEASE(bindings);
    }
  [bindings setObject: self forKey: name];
  [bindingLock unlock];

  [self setValueFor: binding];

  return self;
}

- (void)dealloc
{
  DESTROY(info);
  src = nil; 
  [super dealloc];
}

- (id) destinationValue
{
  id newValue;
  id dest;
  NSString *keyPath;
  NSDictionary *options;

  dest = [info objectForKey: NSObservedObjectKey];
  keyPath = [info objectForKey: NSObservedKeyPathKey];
  options = [info objectForKey: NSOptionsKey];
  newValue = [dest valueForKeyPath: keyPath];
  return [self transformValue: newValue withOptions: options];
}

- (id) sourceValueFor: (NSString *)binding
{
  id newValue;
  NSDictionary *options;

  options = [info objectForKey: NSOptionsKey];
  newValue = [src valueForKeyPath: binding];
  return [self reverseTransformValue: newValue withOptions: options];
}

- (void) setValueFor: (NSString *)binding 
{
  id value = [self destinationValue];

  NSDebugLLog(@"NSBinding", @"setValueFor: binding %@, source %@ value %@", binding, src, value);
  [src setValue: value forKey: binding];
}

- (void) reverseSetValue: (id)value
{
  NSString *keyPath;
  id dest;

  keyPath = [info objectForKey: NSObservedKeyPathKey];
  dest = [info objectForKey: NSObservedObjectKey];
  NSDebugLLog(@"NSBinding", @"reverseSetValue: keyPath %@, dest %@ value %@", keyPath, dest, value);
  [dest setValue: value forKeyPath: keyPath];
}

- (void) reverseSetValueFor: (NSString *)binding
{
  [self reverseSetValue: [self sourceValueFor: binding]];
}

- (void) observeValueForKeyPath: (NSString *)keyPath
                       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context
{
  NSString *binding = (NSString *)context;
  NSDictionary *options;
  id newValue;

  if (change != nil)
    {
      options = [info objectForKey: NSOptionsKey];
      newValue = [change objectForKey: NSKeyValueChangeNewKey];
      newValue = [self transformValue: newValue withOptions: options];
      NSDebugLLog(@"NSBinding", @"observeValueForKeyPath: binding %@, keyPath %@, source %@ value %@", binding, keyPath, src, newValue);
      [src setValue: newValue forKey: binding];
    }
}

- (id) transformValue: (id)value withOptions: (NSDictionary *)options
{
  NSString *valueTransformerName;
  NSValueTransformer *valueTransformer;
  NSString *placeholder;

  if (value == NSMultipleValuesMarker)
    {
      placeholder = [options objectForKey: NSMultipleValuesPlaceholderBindingOption];
      if (placeholder == nil)
        {
          placeholder = @"Multiple Values";
        }
      return placeholder;
    }
  if (value == NSNoSelectionMarker)
    {
      placeholder = [options objectForKey: NSNoSelectionPlaceholderBindingOption];
      if (placeholder == nil)
        {
          placeholder = @"No Selection";
        }
      return placeholder;
    }
  if (value == NSNotApplicableMarker)
    {
      if ([[options objectForKey: NSRaisesForNotApplicableKeysBindingOption]
          boolValue])
        {
          [NSException raise: NSGenericException
                      format: @"This binding does not accept not applicable keys"];
        }

      placeholder = [options objectForKey:
        NSNotApplicablePlaceholderBindingOption];
      if (placeholder == nil)
        {
          placeholder = @"Not Applicable";
        }
      return placeholder;
    }
  if (value == nil)
    {
      placeholder = [options objectForKey:
        NSNullPlaceholderBindingOption];
      return placeholder;
    }

  valueTransformerName = [options objectForKey:
    NSValueTransformerNameBindingOption];
  if (valueTransformerName != nil)
    {
      valueTransformer = [NSValueTransformer valueTransformerForName:
                                                 valueTransformerName];
    }
  else
    {
      valueTransformer = [options objectForKey:
                                      NSValueTransformerBindingOption];
    }

  if (valueTransformer != nil)
    {
      if ([value isKindOfClass: [NSArray class]])
        {
          NSArray *oldValue = (NSArray *)value;
          NSMutableArray *newValue = [[NSMutableArray alloc] initWithCapacity: [oldValue count]];
          id<NSFastEnumeration> enumerator = oldValue;

          FOR_IN (id, obj, enumerator)
            [newValue addObject: [valueTransformer transformedValue: obj]];
          END_FOR_IN(enumerator)
          value = AUTORELEASE(newValue);
        }
      else
        {
          value = [valueTransformer transformedValue: value];
        }
    }

  return value;
}

- (id) reverseTransformValue: (id)value withOptions: (NSDictionary *)options
{
  NSString *valueTransformerName;
  NSValueTransformer *valueTransformer;

  valueTransformerName = [options objectForKey:
    NSValueTransformerNameBindingOption];
  if (valueTransformerName != nil)
    {
      valueTransformer = [NSValueTransformer valueTransformerForName:
                                                 valueTransformerName];
    }
  else
    {
      valueTransformer = [options objectForKey:
                                      NSValueTransformerBindingOption];
    }

  if ((valueTransformer != nil) && [[valueTransformer class]
                                       allowsReverseTransformation])
    {
      value = [valueTransformer reverseTransformedValue: value];
    }

  return value;
}

- (NSString*) description
{
  return [NSString stringWithFormat: 
                     @"GSKeyValueBinding src (%@) info %@", src, info];
}

@end

@implementation GSKeyValueOrBinding : GSKeyValueBinding 

- (void) setValueFor: (NSString *)binding 
{
  NSDictionary *bindings;
  BOOL res;
  
  if (!objectTable)
    return;

 [bindingLock lock];
  bindings = (NSDictionary *)NSMapGet(objectTable, (void *)src);
  if (!bindings)
    return;

  res = GSBindingResolveMultipleValueBool(binding, bindings,
                                          GSBindingOperationOr);
  [bindingLock unlock];
  [src setValue: [NSNumber numberWithBool: res] forKey: binding];
}

- (void) observeValueForKeyPath: (NSString *)keyPath
                       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context
{
  [self setValueFor: (NSString*)context];
}

@end

@implementation GSKeyValueAndBinding : GSKeyValueBinding 

- (void) setValueFor: (NSString *)binding 
{
  NSDictionary *bindings;
  BOOL res;
  
  if (!objectTable)
    return;

  [bindingLock lock];
  bindings = (NSDictionary *)NSMapGet(objectTable, (void *)src);
  if (!bindings)
    return;

  res = GSBindingResolveMultipleValueBool(binding, bindings,
                                          GSBindingOperationAnd);
  [bindingLock unlock];
  [src setValue: [NSNumber numberWithBool: res] forKey: binding];
}

- (void) observeValueForKeyPath: (NSString *)keyPath
                       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context
{
  [self setValueFor: (NSString*)context];
}

@end

BOOL NSIsControllerMarker(id object)
{
  return [NSMultipleValuesMarker isEqual: object]
    || [NSNoSelectionMarker isEqual: object]
    || [NSNotApplicableMarker isEqual: object];
}

//Helper functions
BOOL GSBindingResolveMultipleValueBool(NSString *key, NSDictionary *bindings,
    GSBindingOperationKind operationKind)
{
  NSString *bindingName;
  NSDictionary *info;
  int count = 1;
  id object;
  NSString *keyPath;
  id value;
  NSDictionary *options;
  GSKeyValueBinding *theBinding;

  bindingName = key;
  while ((theBinding = [bindings objectForKey: bindingName]))
    {
      info = theBinding->info;
      object = [info objectForKey: NSObservedObjectKey];
      keyPath = [info objectForKey: NSObservedKeyPathKey];
      options = [info objectForKey: NSOptionsKey];

      value = [object valueForKeyPath: keyPath];
      value = [theBinding transformValue: value withOptions: options];
      if ([value boolValue] == operationKind)
        {
          return operationKind;
        }
      bindingName = [NSString stringWithFormat: @"%@%i", key, ++count];
    }
  return !operationKind;
}

void GSBindingInvokeAction(NSString *targetKey, NSString *argumentKey,
    NSDictionary *bindings)
{
  NSString *bindingName;
  NSDictionary *info;
  NSDictionary *options;
  int count = 1;
  id object;
  id target;
  SEL selector;
  NSString *keyPath;
  NSInvocation *invocation;
  GSKeyValueBinding *theBinding;

  theBinding = [bindings objectForKey: targetKey];
  info = theBinding->info;
  object = [info objectForKey: NSObservedObjectKey];
  keyPath = [info objectForKey: NSObservedKeyPathKey];
  options = [info objectForKey: NSOptionsKey];

  target = [object valueForKeyPath: keyPath];
  selector = NSSelectorFromString([options objectForKey: 
      NSSelectorNameBindingOption]);
  if (target == nil || selector == NULL) return;

  invocation = [NSInvocation invocationWithMethodSignature:
    [target methodSignatureForSelector: selector]];
  [invocation setSelector: selector];

  bindingName = argumentKey;
  while ((theBinding = [bindings objectForKey: bindingName]))
    {
      info = theBinding->info;
      object = [info objectForKey: NSObservedObjectKey];
      keyPath = [info objectForKey: NSObservedKeyPathKey];
      if ((object = [object valueForKeyPath: keyPath]))
        {
          [invocation setArgument: object atIndex: ++count];
        }
      bindingName = [NSString stringWithFormat: @"%@%i", argumentKey, count];
    }
  [invocation invoke];
}
