/* Mutable set proxies for GNUstep's KeyValueCoding
   Copyright (C) 2007 Free Software Foundation, Inc.

   Written by:  Chris Farber <chris@chrisfarber.net>

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110 USA.

   $Date: 2007-06-08 04:04:14 -0400 (Fri, 08 Jun 2007) $ $Revision: 25230 $
   */

#import "common.h"
#import "Foundation/NSInvocation.h"

@interface NSKeyValueMutableSet : NSMutableSet
{
  @protected
  id object;
  NSString *key;
  NSMutableSet *set;
  BOOL changeInProgress;
  BOOL notifiesObservers;
}

+ (NSKeyValueMutableSet*) setForKey: (NSString*)aKey ofObject: (id)anObject;
- (id) initWithKey: (NSString*)aKey ofObject: (id)anObject;

@end

@interface NSKeyValueFastMutableSet : NSKeyValueMutableSet 
{
  @private
  NSInvocation *addObjectInvocation;
  NSInvocation *removeObjectInvocation;
  NSInvocation *addSetInvocation;
  NSInvocation *removeSetInvocation;
  NSInvocation *intersectInvocation;
  NSInvocation *setSetInvocation;
}

+ (id) setForKey: (NSString*)aKey ofObject: (id)anObject
  withCapitalizedKey: (const char *)capitalized;

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
  withCapitalizedKey: (const char *)capitalized;

@end

@interface NSKeyValueSlowMutableSet : NSKeyValueMutableSet
{
  @private
  NSInvocation *setSetInvocation;
}

+ (id) setForKey: (NSString *)aKey ofObject: (id)anObject
  withCapitalizedKey: (const char *)capitalized;

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
  withCapitalizedKey: (const char *)capitalized;

@end

@interface NSKeyValueIvarMutableSet : NSKeyValueMutableSet
{
  @private
}

+ (id) setForKey: (NSString *)aKey ofObject: (id)anObject;

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject;

@end


@implementation NSKeyValueMutableSet

+ (NSKeyValueMutableSet *) setForKey: (NSString *)aKey ofObject: (id)anObject
{
  NSKeyValueMutableSet *proxy;
  unsigned size = [aKey maximumLengthOfBytesUsingEncoding:
			  NSUTF8StringEncoding];
  char keybuf[size + 1];

  [aKey getCString: keybuf
         maxLength: size + 1
          encoding: NSUTF8StringEncoding];
  if (islower(*keybuf))
    {
      *keybuf = toupper(*keybuf);
    }


  proxy = [NSKeyValueFastMutableSet setForKey: aKey 
                                     ofObject: anObject
                           withCapitalizedKey: keybuf];
  if (proxy == nil)
    {
      proxy = [NSKeyValueSlowMutableSet setForKey: aKey 
                                         ofObject: anObject
                               withCapitalizedKey: keybuf];

      if (proxy == nil)
	{
	  proxy = [NSKeyValueIvarMutableSet setForKey: aKey 
                                             ofObject: anObject];
	}
    }
  return proxy;
}

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
{
  if ((self = [super init]) != nil)
    {
      object = anObject;
      key = [aKey copy];
      changeInProgress = NO;
      notifiesObservers = 
       [[anObject class] automaticallyNotifiesObserversForKey: aKey];
    }
  return self;
}

- (NSUInteger) count
{
  if (set == nil)
    {
      set = [object valueForKey: key];
    }
  return [set count];
}

- (id) member: (id)anObject
{
  if (set == nil)
    {
      set = [object valueForKey: key];
    }
  return [set member: anObject];
}

- (NSEnumerator *) objectEnumerator
{
  if (set == nil)
    {
      set = [object valueForKey: key];
    }
  return [set objectEnumerator];
}

- (void) removeAllObjects
{
  if (set == nil)
    {
      set = [object valueForKey: key];
    }
  [set removeAllObjects];
}

@end

@implementation NSKeyValueFastMutableSet

+ (id) setForKey: (NSString *)aKey ofObject: (id)anObject
       withCapitalizedKey: (const char *)capitalized
{
  return [[[self alloc] initWithKey: aKey
                           ofObject: anObject
                 withCapitalizedKey: capitalized] autorelease];
}

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
       withCapitalizedKey: (const char *)capitalized
{
  SEL addObject;
  SEL removeObject;
  SEL addSet;
  SEL removeSet;
  SEL intersect;
  SEL setSet;
  BOOL canAdd = NO;
  BOOL canRemove = NO;


  addObject = NSSelectorFromString
    ([NSString stringWithFormat: @"add%sObject:", capitalized]);
  removeObject = NSSelectorFromString
    ([NSString stringWithFormat: @"remove%sObject:", capitalized]);
  addSet = NSSelectorFromString
    ([NSString stringWithFormat: @"add%s:", capitalized]);
  removeSet = NSSelectorFromString
    ([NSString stringWithFormat: @"remove%s:", capitalized]);

  if ([anObject respondsToSelector: addObject])
    {
      canAdd = YES;
      addObjectInvocation = [[NSInvocation invocationWithMethodSignature:
        [anObject methodSignatureForSelector: addObject]] retain];
      [addObjectInvocation setTarget: anObject];
      [addObjectInvocation setSelector: addObject];
    }
  if ([anObject respondsToSelector: removeObject])
    {
      canRemove = YES;
      removeObjectInvocation = [[NSInvocation invocationWithMethodSignature:
        [anObject methodSignatureForSelector: removeObject]] retain];
      [removeObjectInvocation setTarget: anObject];
      [removeObjectInvocation setSelector: removeObject];
    }
  if ([anObject respondsToSelector: addSet])
    {
      canAdd = YES;
      addSetInvocation = [[NSInvocation invocationWithMethodSignature:
        [anObject methodSignatureForSelector: addSet]] retain];
      [addSetInvocation setTarget: anObject];
      [addSetInvocation setSelector: addSet];
    }
  if ([anObject respondsToSelector: removeSet])
    {
      canRemove = YES;
      removeSetInvocation = [[NSInvocation invocationWithMethodSignature:
        [anObject methodSignatureForSelector: removeSet]] retain];
      [removeSetInvocation setTarget: anObject];
      [removeSetInvocation setSelector: removeSet];
    }

  if (!canAdd || !canRemove)
    {
      DESTROY(self);
      return nil;
    }

  if ((self = [super initWithKey: aKey  ofObject: anObject]) != nil)
    {
      intersect = NSSelectorFromString
        ([NSString stringWithFormat: @"intersect%s:", capitalized]);
      setSet = NSSelectorFromString
        ([NSString stringWithFormat: @"set%s:", capitalized]);

      if ([anObject respondsToSelector: intersect])
        {
          intersectInvocation = [[NSInvocation invocationWithMethodSignature:
            [anObject methodSignatureForSelector: intersect]] retain];
          [intersectInvocation setTarget: anObject];
          [intersectInvocation setSelector: intersect];
        }
      if ([anObject respondsToSelector: setSet])
        {
          setSetInvocation = [[NSInvocation invocationWithMethodSignature:
            [anObject methodSignatureForSelector: setSet]] retain];
          [setSetInvocation setTarget: anObject];
          [setSetInvocation setSelector: setSet];
        }
    }
  return self;
}

- (void) dealloc
{
  [setSetInvocation release];
  [intersectInvocation release];
  [removeSetInvocation release];
  [addSetInvocation release];
  [removeObjectInvocation release];
  [addObjectInvocation release];
  [super dealloc];
}

- (void) addObject: (id)anObject
{
  if (addObjectInvocation)
    {
      if (notifiesObservers && !changeInProgress)
	{
	  [object willChangeValueForKey: key
		        withSetMutation: NSKeyValueUnionSetMutation
		           usingObjects: [NSSet setWithObject: anObject]];
	}
      [addObjectInvocation setArgument: &anObject  atIndex: 2];
      [addObjectInvocation invoke];
      if (notifiesObservers && !changeInProgress)
	{
	  [object didChangeValueForKey: key
		       withSetMutation: NSKeyValueUnionSetMutation
		          usingObjects: [NSSet setWithObject: anObject]];
	}
    }
  else
    {
      [self unionSet: [NSSet setWithObject: anObject]];
    }
}

- (void) unionSet: (NSSet *)aSet
{
  if (notifiesObservers)
    {
      changeInProgress = YES;
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueUnionSetMutation
                       usingObjects: aSet];
    }
  if (addSetInvocation)
    {
      [addSetInvocation setArgument: &aSet  atIndex: 2];
      [addSetInvocation invoke];
    }
  else
    {
      [super unionSet: aSet];
    }
  if (notifiesObservers)
    {
      changeInProgress = NO;
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueUnionSetMutation
                      usingObjects: aSet];
    }
}

- (void) removeObject: (id)anObject
{
  if (removeObjectInvocation)
    {
      if (notifiesObservers && !changeInProgress)
	{
          [object willChangeValueForKey: key
                        withSetMutation: NSKeyValueMinusSetMutation
                           usingObjects: [NSSet setWithObject: anObject]];
	}
      [removeObjectInvocation setArgument: &anObject  atIndex: 2];
      [removeObjectInvocation invoke];
      if (notifiesObservers && !changeInProgress)
	{
          [object didChangeValueForKey: key
                       withSetMutation: NSKeyValueMinusSetMutation
                          usingObjects: [NSSet setWithObject: anObject]];
	}
    }
  else
    [self minusSet: [NSSet setWithObject: anObject]];
}

- (void) minusSet: (NSSet *)aSet
{
  if (notifiesObservers)
    {
      changeInProgress = YES;
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: aSet];
    }
  if (removeSetInvocation)
    {
      [removeSetInvocation setArgument: &aSet  atIndex: 2];
      [removeSetInvocation invoke];
    }
  else
    {
      [super minusSet: aSet];
    }
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: aSet];
      changeInProgress = NO;
    }
}

- (void) intersectSet: (NSSet *)aSet
{
  if (notifiesObservers)
    {
      changeInProgress = YES;
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueIntersectSetMutation
                       usingObjects: aSet];
    }
  if (intersectInvocation)
    {
      [intersectInvocation setArgument: &aSet  atIndex: 2];
      [intersectInvocation invoke];
    }
  else
    {
      [super intersectSet: aSet];
    }
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueIntersectSetMutation
                      usingObjects: aSet];
       changeInProgress = NO;
    }
}

- (void) setSet: (NSSet *)aSet
{
  if (notifiesObservers)
    {
      changeInProgress = YES;
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueSetSetMutation
                       usingObjects: aSet];
    }
  if (setSetInvocation)
    {
      [setSetInvocation setArgument: &aSet  atIndex: 2];
      [setSetInvocation invoke];
    }
  else
    {
      [super setSet: aSet];
    }
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueSetSetMutation
                      usingObjects: aSet];
      changeInProgress = NO;
    }
}

@end

@implementation NSKeyValueSlowMutableSet

+ (id) setForKey: (NSString *)aKey ofObject: (id)anObject
       withCapitalizedKey: (const char *)capitalized
{
  return [[[self alloc] initWithKey: aKey ofObject: anObject
                withCapitalizedKey: capitalized] autorelease];
}

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
       withCapitalizedKey: (const char *)capitalized;

{
  SEL setSelector = NSSelectorFromString([NSString stringWithFormat:
    @"set%s:", capitalized]);

  if (![anObject respondsToSelector: setSelector])
    {
      DESTROY(self);
      return nil;
    }

  if ((self = [super initWithKey: aKey ofObject: anObject]) != nil)
    {
      setSetInvocation = [[NSInvocation invocationWithMethodSignature:
        [anObject methodSignatureForSelector: setSelector]] retain];
      [setSetInvocation setSelector: setSelector];
      [setSetInvocation setTarget: anObject];
    }
  return self;
}

- (void) setSet: (id)aSet
{
  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueSetSetMutation
                       usingObjects: aSet];
    }
  [setSetInvocation setArgument: &aSet  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueSetSetMutation
                      usingObjects: aSet];
    }
}

- (void) removeAllObjects
{
  NSSet *nothing;
  NSSet *theSet = [NSSet setWithSet: [object valueForKey: key]];

  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: theSet];
    }
  nothing = [NSSet set];
  [setSetInvocation setArgument: &nothing  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: theSet];
    }
}

- (void) addObject: (id)anObject
{
  NSMutableSet *temp;
  NSSet *unionSet = [NSSet setWithObject: anObject];

  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueUnionSetMutation
                       usingObjects: unionSet];
    }
  temp = [NSMutableSet setWithSet: [object valueForKey: key]];
  [temp addObject: anObject];
  [setSetInvocation setArgument: &temp  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueUnionSetMutation
                      usingObjects: unionSet];
    }
}

- (void) removeObject: (id)anObject
{
  NSMutableSet *temp;
  NSSet *minusSet = [NSSet setWithObject: anObject];

  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: minusSet];
    }
  temp = [NSMutableSet setWithSet: [object valueForKey: key]];
  [temp removeObject: anObject];
  [setSetInvocation setArgument: &temp  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: minusSet];
    }
}

- (void) unionSet: (id)anObject
{
  NSMutableSet *temp;

  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueUnionSetMutation
                       usingObjects: anObject];
    }
  temp = [NSMutableSet setWithSet: [object valueForKey: key]];
  [temp unionSet: anObject];
  [setSetInvocation setArgument: &temp  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueUnionSetMutation
                      usingObjects: anObject];
    }
}

- (void) minusSet: (id)anObject
{
  NSMutableSet *temp;
 
  if (notifiesObservers)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: anObject];
    }
  temp = [NSMutableSet setWithSet: [object valueForKey: key]];
  [temp minusSet: anObject];
  [setSetInvocation setArgument: &temp  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: anObject];
    }
}

- (void) intersectSet: (id)anObject
{
  NSMutableSet *temp;

  if (notifiesObservers)
  {
    [object willChangeValueForKey: key
                  withSetMutation: NSKeyValueIntersectSetMutation
                     usingObjects: anObject];
  }
  temp = [NSMutableSet setWithSet: [object valueForKey: key]];
  [temp intersectSet: anObject];
  [setSetInvocation setArgument: &temp  atIndex: 2];
  [setSetInvocation invoke];
  if (notifiesObservers)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueIntersectSetMutation
                      usingObjects: anObject];
    }
}

@end


@implementation NSKeyValueIvarMutableSet

+ (id) setForKey: (NSString *)aKey ofObject: (id)anObject
{
  return [[[self alloc] initWithKey: aKey ofObject: anObject] autorelease];
}

- (id) initWithKey: (NSString *)aKey ofObject: (id)anObject
{
  if ((self = [super initWithKey: aKey  ofObject: anObject]) != nil)
    {
      unsigned size = [aKey maximumLengthOfBytesUsingEncoding:
        NSUTF8StringEncoding];
      char cKey[size + 2];
      char *cKeyPtr = &cKey[0];
      const char *type = 0;

      int offset;

      
      cKey[0] = '_';
      [aKey getCString: cKeyPtr + 1
             maxLength: size + 1
              encoding: NSUTF8StringEncoding];
      if (!GSObjCFindVariable (anObject, cKeyPtr, &type, &size, &offset))
        {
          GSObjCFindVariable (anObject, ++cKeyPtr, &type, &size, &offset);
        }
      set = GSObjCGetVal (anObject, cKeyPtr, NULL, type, size, offset);
    }
  return self;
}

- (NSUInteger) count
{
  return [set count];
}

- (NSArray *) allObjects
{
  return [set allObjects];
}

- (BOOL) containsObject: (id)anObject
{
  return [set containsObject: anObject];
}

- (id) member: (id)anObject
{
  return [set member: anObject];
}

- (void) addObject: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueUnionSetMutation
	               usingObjects: [NSSet setWithObject: anObject]];
    }
  [set addObject: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueUnionSetMutation
                      usingObjects: [NSSet setWithObject:anObject]];
    }
}

- (void) removeObject: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: [NSSet setWithObject:anObject]];
    }
  [set removeObject: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: [NSSet setWithObject: anObject]];
    }
}

- (void) unionSet: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueUnionSetMutation
                       usingObjects: [NSSet setWithObject: anObject]];
    }
  [set unionSet: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueUnionSetMutation
                      usingObjects: [NSSet setWithObject:anObject]];
    }
}

- (void) minusSet: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueMinusSetMutation
                       usingObjects: [NSSet setWithObject: anObject]];
    }
  [set minusSet: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueMinusSetMutation
                      usingObjects: [NSSet setWithObject: anObject]];
    }
}

- (void) intersectSet: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueIntersectSetMutation
                       usingObjects: [NSSet setWithObject: anObject]];
    }
  [set intersectSet: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueIntersectSetMutation
                      usingObjects: [NSSet setWithObject: anObject]];
    }
}

- (void) setSet: (id)anObject
{
  if (notifiesObservers && !changeInProgress)
    {
      [object willChangeValueForKey: key
                    withSetMutation: NSKeyValueSetSetMutation
                       usingObjects: [NSSet setWithObject: anObject]];
    }
  [set setSet: anObject];
  if (notifiesObservers && !changeInProgress)
    {
      [object didChangeValueForKey: key
                   withSetMutation: NSKeyValueSetSetMutation
                      usingObjects: [NSSet setWithObject: anObject]];
    }
}
@end
