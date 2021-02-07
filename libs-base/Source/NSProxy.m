/** Implementation for GNU Objective-C version of NSProxy
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: August 1997

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

   <title>NSProxy class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSProxy.h"
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSException.h"
#import "Foundation/NSHashTable.h"
#import "Foundation/NSDistantObject.h"
#import "Foundation/NSPortCoder.h"

// Get objc_delete_weak_refs(), if it is present in the runtime.
#ifdef __GNUSTEP_RUNTIME__
#include <objc/capabilities.h>
#ifdef OBJC_CAP_ARC
#include <objc/objc-arc.h>
#endif
#endif

@class	NSDistantObject;

/**
 * <p>The NSProxy class provides a basic implementation of a class whose
 * instances are used to <em>stand in</em> for other objects.<br />
 * The class provides the most basic methods of NSObject, and expects
 * messages for other methods to be forwarded to the <em>real</em>
 * object represented by the proxy.  You must subclass NSProxy to
 * implement -forwardInvocation: to these <em>real</em> objects.</p>
 *
 * <p>Note that <code>NSProxy</code> is a different sort of class than others
 * in the GNUstep Base library in that it is the only example of a root class
 * besides [NSObject].  Thus, it implements the <code>NSObject</code> protocol
 * but is not a subclass of NSObject.</p>
 */
@implementation NSProxy

/**
 * Allocates and returns an NSProxy instance in the default zone.
 */
+ (id) alloc
{
  return [self allocWithZone: NSDefaultMallocZone()];
}

/**
 * Allocates and returns an NSProxy instance in the specified zone z.
 */
+ (id) allocWithZone: (NSZone*)z
{
  NSProxy*	ob = (NSProxy*) NSAllocateObject(self, 0, z);
  return ob;
}

/**
 * Returns the receiver.
 */
+ (id) autorelease
{
  return self;
}

/**
 * Returns the receiver.
 */
+ (Class) class
{
  return self;
}

/**
 * Returns a string describing the receiver.
 */
+ (NSString*) description
{
  return [NSString stringWithFormat: @"<%s>", GSClassNameFromObject(self)];
}

+ (IMP) instanceMethodForSelector: (SEL)aSelector
{
  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];
  /*
   * Since 'self' is an class, class_getMethodImplementation() will get
   * the instance method.
   */
  return class_getMethodImplementation((Class)self, aSelector);
}

/**
 * Returns NO ... the NSProxy class cannot be an instance of any class.
 */
+ (BOOL) isKindOfClass: (Class)aClass
{
  return NO;
}

/**
 * Returns YES if aClass is identical to the receiver, NO otherwise.
 */
+ (BOOL) isMemberOfClass: (Class)aClass
{
  return(self == aClass);
}

/**
 * A dummy method ...
 */
+ (void) load
{
  /* Do nothing	*/
}

- (IMP) methodForSelector: (SEL)aSelector
{
  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  return class_getMethodImplementation(object_getClass((id)self), aSelector);
}

/**
 * Returns the method signature for the specified selector.
 */
+ (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  struct objc_method	*mth;

  if (0 == aSelector)
    {
      return nil;
    }
  mth = GSGetMethod(self, aSelector, NO, YES);
  if (mth != 0)
    {
      const char	*types = method_getTypeEncoding(mth);

      if (types != 0)
	{
	  return [NSMethodSignature signatureWithObjCTypes: types];
	}
    }
  return nil;
}

/**
 * A dummy method to ensure that the class can safely be held in containers.
 */
+ (oneway void) release
{
  /* Do nothing	*/
}

/**
 * Returns YES if the receiver responds to aSelector, NO otherwise.
 */
+ (BOOL) respondsToSelector: (SEL)aSelector
{
  if (class_respondsToSelector(object_getClass(self), aSelector))
    return YES;
  else
    return NO;
}

/**
 * Returns the receiver.
 */
+ (id) retain
{
  return self;
}

/**
 * Returns the maximum unsigned integer value.
 */
+ (NSUInteger) retainCount
{
  return UINT_MAX;
}

/**
 * Returns the superclass of the receiver.
 */
+ (Class) superclass
{
  return class_getSuperclass(self);
}

/**
 * Adds the receiver to the current autorelease pool and returns self.
 */
- (id) autorelease
{
  [NSAutoreleasePool addObject: self];
  return self;
}

/**
 * Dummy method ... returns the receiver.
 */
- (id) awakeAfterUsingCoder: (NSCoder*)aDecoder
{
  return self;
}

/**
 * Returns the class of the receiver.
 */
- (Class) class
{
  return object_getClass(self);
}

/**
 * Calls the -forwardInvocation: method to determine if the 'real' object
 * referred to by the proxy conforms to aProtocol.  Returns the result.<br />
 * NB. The default operation of -forwardInvocation: is to raise an exception.
 */
- (BOOL) conformsToProtocol: (Protocol*)aProtocol
{
  NSMethodSignature	*sig;
  NSInvocation		*inv;
  BOOL			ret;

  sig = [self methodSignatureForSelector: _cmd];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector: _cmd];
  [inv setArgument: &aProtocol atIndex: 2];
  [self forwardInvocation: inv];
  [inv getReturnValue: &ret];
  return ret;
}

/**
 * Frees the memory used by the receiver.
 */
- (void) dealloc
{
  NSDeallocateObject((NSObject*)self);
}

/**
 * Returns a text description of the receiver.
 */
- (NSString*) description
{
  return [NSString stringWithFormat: @"<%s %lx>",
	GSClassNameFromObject(self), (size_t)self];
}

/** <override-subclass />
 * Raises an <code>NSInvalidArgumentException</code>.
 */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  [NSException raise: NSInvalidArgumentException
	      format: @"NSProxy should not implement '%s'",
				sel_getName(_cmd)];
}

/**
 * Returns the address of the receiver ... so it can be stored in a dictionary.
 */
- (NSUInteger) hash
{
  /*
   * Ideally we would shift left to lose any zero bits produced by the
   * alignment of the object in memory ... but that depends on the
   * processor architecture and the memory allocatiion implementation.
   * In the absence of detailed information, pick a reasonable value
   * assuming the object will be aligned to an eight byte boundary.
   */
  return ((NSUInteger)(uintptr_t)self)>>3;
}

/** <init /> <override-subclass />
 * Initialises the receiver and returns the resulting instance.
 */
- (id) init
{
  [NSException raise: NSGenericException
    format: @"subclass %s should override %s", GSClassNameFromObject(self),
    sel_getName(_cmd)];
  return self;
}

/**
 * Tests for pointer equality with anObject.
 */
- (BOOL) isEqual: (id)anObject
{
  return (self == anObject);
}

/**
 * Calls the -forwardInvocation: method to determine if the 'real' object
 * referred to by the proxy is an instance of the specified class.
 * Returns the result.<br />
 * NB. The default operation of -forwardInvocation: is to raise an exception.
 */
- (BOOL) isKindOfClass: (Class)aClass
{
  NSMethodSignature	*sig;
  NSInvocation		*inv;
  BOOL			ret;

  sig = [self methodSignatureForSelector: _cmd];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector: _cmd];
  [inv setArgument: &aClass atIndex: 2];
  [self forwardInvocation: inv];
  [inv getReturnValue: &ret];
  return ret;
}

/**
 * Calls the -forwardInvocation: method to determine if the 'real' object
 * referred to by the proxy is an instance of the specified class.
 * Returns the result.<br />
 * NB. The default operation of -forwardInvocation: is to raise an exception.
 */
- (BOOL) isMemberOfClass: (Class)aClass
{
  NSMethodSignature	*sig;
  NSInvocation		*inv;
  BOOL			ret;

  sig = [self methodSignatureForSelector: _cmd];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector: _cmd];
  [inv setArgument: &aClass atIndex: 2];
  [self forwardInvocation: inv];
  [inv getReturnValue: &ret];
  return ret;
}

/**
 * Returns YES.
 */
- (BOOL) isProxy
{
  return YES;
}

- (id) notImplemented: (SEL)aSel
{
  [NSException raise: NSGenericException
	      format: @"NSProxy notImplemented %s", sel_getName(aSel)];
  return self;
}

/**
 * If we respond to the method directly, create and return a method
 * signature.  Otherwise raise an exception.
 */
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  struct objc_method	*mth;

  if (0 == aSelector)
    {
      return nil;
    }
  mth = GSGetMethod(object_getClass(self), aSelector, YES, YES);
  if (mth != 0)
    {
      const char	*types = method_getTypeEncoding(mth);

      if (types != 0)
	{
	  return [NSMethodSignature signatureWithObjCTypes: types];
	}
    }
  [NSException raise: NSInvalidArgumentException format:
    @"NSProxy should not implement 'methodSignatureForSelector:'"];
  return nil;
}

- (id) performSelector: (SEL)aSelector
{
  IMP msg = objc_msg_lookup(self, aSelector);

  if (!msg)
    {
      [NSException raise: NSGenericException
		  format: @"invalid selector passed to %s",
				sel_getName(_cmd)];
      return nil;
    }
  return (*msg)(self, aSelector);
}

- (id) performSelector: (SEL)aSelector
	    withObject: (id)anObject
{
  IMP msg = objc_msg_lookup(self, aSelector);

  if (!msg)
    {
      [NSException raise: NSGenericException
		  format: @"invalid selector passed to %s",
				sel_getName(_cmd)];
      return nil;
    }
  return (*msg)(self, aSelector, anObject);
}

- (id) performSelector: (SEL)aSelector
	    withObject: (id)anObject
	    withObject: (id)anotherObject
{
  IMP msg = objc_msg_lookup(self, aSelector);

  if (!msg)
    {
      [NSException raise: NSGenericException
		  format: @"invalid selector passed to %s",
				sel_getName(_cmd)];
      return nil;
    }
  return (*msg)(self, aSelector, anObject, anotherObject);
}

/**
 * Decrement the retain count for the receiver ... deallocate if it would
 * become negative.
 */
- (oneway void) release
{
  if (NSDecrementExtraRefCountWasZero(self))
    {
#  ifdef OBJC_CAP_ARC
      objc_delete_weak_refs(self);
#  endif
      [self dealloc];
    }
}

/**
 * Returns the actual object to be encoded for sending over the
 * network on a Distributed Objects connection.
 */
- (id) replacementObjectForPortCoder: (NSPortCoder*)aCoder
{
  static Class	proxyClass = 0;
  static IMP	proxyImp = 0;

  if (proxyImp == 0)
    {
      proxyClass = [NSDistantObject class];
      /*
       * use class_getMethodImplementation() because NSDistantObject
       * doesn't implement methodForSelector:
       */
      proxyImp = class_getMethodImplementation(object_getClass((id)proxyClass),
	@selector(proxyWithLocal:connection:));
    }

  return (*proxyImp)(proxyClass, @selector(proxyWithLocal:connection:),
    self, [aCoder connection]);
}

/**
 * If we respond to the method directly, return YES, otherwise
 * forward this request to the object we are acting as a proxy for.
 */
- (BOOL) respondsToSelector: (SEL)aSelector
{
  if (aSelector == 0)
    {
      return NO;
    }
  if (class_respondsToSelector(object_getClass(self), aSelector))
    {
      return YES;
    }
  else
    {
      NSMethodSignature	*sig;
      NSInvocation	*inv;
      BOOL		ret;

      sig = [self methodSignatureForSelector: _cmd];
      inv = [NSInvocation invocationWithMethodSignature: sig];
      [inv setSelector: _cmd];
      [inv setArgument: &aSelector atIndex: 2];
      [self forwardInvocation: inv];
      [inv getReturnValue: &ret];
      return ret;
    }
}

/**
 * Increment the retain count for the receiver.
 */
- (id) retain
{
  NSIncrementExtraRefCount(self);
  return self;
}

/**
 * Return the retain count for the receiver.
 */
- (NSUInteger) retainCount
{
  return NSExtraRefCount(self) + 1;
}

/**
 * Returns the receiver.
 */
- (id) self
{
  return self;
}

/**
 * Returns the superclass of the receiver's class.
 */
- (Class) superclass
{
  return class_getSuperclass(object_getClass(self));
}

+ (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  return 0;
}
+ (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return 0;
}
+ (NSUInteger) sizeOfInstance
{
  return 0;
}


- (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  if (0 == NSHashGet(exclude, self))
    {
      Class             c = object_getClass(self);
      NSUInteger        size = class_getInstanceSize(c);

      NSHashInsert(exclude, self);
      return size;
    }
  return 0;
}
- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return 0;
}
- (NSUInteger) sizeOfInstance
{
  return class_getInstanceSize(object_getClass(self));
}

/**
 * Returns the zone in which the receiver was allocated.
 */
- (NSZone*) zone
{
  return NSZoneFromPointer(self);
}

@end

