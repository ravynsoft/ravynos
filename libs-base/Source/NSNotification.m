/** Implementation of NSNotification for GNUstep
   Copyright (C) 1996 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Created: March 1996

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

   <title>NSNotification class reference</title>
   $Date$ $Revision$
*/

#import "common.h"
#define	EXPOSE_NSNotification_IVARS	1
#import "Foundation/NSNotification.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSDictionary.h"

@class	GSNotification;
@interface GSNotification : NSObject	// Help the compiler
@end

/**
 *  <p>Represents a notification for posting to an [NSNotificationCenter].
 *  Consists of a name, an object, and an optional dictionary.  The
 *  notification center will check for observers registered to receive
 *  either notifications with the name, the object, or both and pass the
 *  notification instance on to them.</p>
 *  <p>This class is actually the interface for a class cluster, so instances
 *  will be of a (private) subclass.</p>
 */
@implementation NSNotification

static Class	abstractClass = 0;
static Class	concreteClass = 0;

+ (id) allocWithZone: (NSZone*)z
{
  if (self == abstractClass)
    {
      return (id)NSAllocateObject(concreteClass, 0, z);
    }
  return (id)NSAllocateObject(self, 0, z);
}

+ (void) initialize
{
  if (concreteClass == 0)
    {
      abstractClass = [NSNotification class];
      concreteClass = [GSNotification class];
    }
}

/**
 * Create a new autoreleased notification.
 */
+ (NSNotification*) notificationWithName: (NSString*)name
				  object: (id)object
			        userInfo: (NSDictionary*)info
{
  return [concreteClass notificationWithName: name
				      object: object
				    userInfo: info];
}

/**
 * Create a new autoreleased notification by calling
 * +notificationWithName:object:userInfo: with a nil user info argument.
 */
+ (NSNotification*) notificationWithName: (NSString*)name
				  object: (id)object
{
  return [concreteClass notificationWithName: name
				      object: object
				    userInfo: nil];
}

/**
 * The abstract class implements a copy as a simple retain ...
 * subclasses override to perform more intelligent copy operations.
 */
- (id) copyWithZone: (NSZone*)zone
{
  return [self retain];
}

/**
 * Return a description of the parts of the notification.
 */
- (NSString*) description
{
  return [[super description] stringByAppendingFormat:
    @" Name: %@ Object: %@ Info: %@",
    [self name], [self object], [self userInfo]];
}

- (NSUInteger) hash
{
  return [[self name] hash] ^ [[self object] hash];
}

- (id) init
{
  if ([self class] == abstractClass)
    {
      NSZone	*z = [self zone];

      DESTROY(self);
      self = (id)NSAllocateObject (concreteClass, 0, z);
    }
  return self;
}

- (BOOL) isEqual: (id)other
{
  NSNotification	*o;
  NSObject		*v1;
  NSObject		*v2;

  if (NO == [(o = other) isKindOfClass: [NSNotification class]]
    || ((v1 = [self name]) != (v2 = [o name]) && ![v1 isEqual: v2])
    || ((v1 = [self object]) != (v2 = [o object]) && ![v1 isEqual: v2])
    || ((v1 = [self userInfo]) != (v2 = [o userInfo]) && ![v1 isEqual: v2]))
    {
      return NO;
    }
  return YES;
}

/**
 *  Returns the notification name.
 */
- (NSString*) name
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 *  Returns the notification object.
 */
- (id) object
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/**
 * Returns the notification user information.
 */
- (NSDictionary*) userInfo
{
  [self subclassResponsibility: _cmd];
  return nil;
}

/*
 * NSCoding protocol - the MacOS-X documentation says it should conform,
 * but how can we meaningfully encode/decode the object and userInfo.
 * We do it anyway - at least it should make sense over DO.
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  id	o;

  o = [self name];
  [aCoder encodeValueOfObjCType: @encode(id) at: &o];
  o = [self object];
  [aCoder encodeValueOfObjCType: @encode(id) at: &o];
  o = [self userInfo];
  [aCoder encodeValueOfObjCType: @encode(id) at: &o];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  NSString	*name;
  id		object;
  NSDictionary	*info;
  id		n;

  [aCoder decodeValueOfObjCType: @encode(id) at: &name];
  [aCoder decodeValueOfObjCType: @encode(id) at: &object];
  [aCoder decodeValueOfObjCType: @encode(id) at: &info];
  n = [NSNotification notificationWithName: name object: object userInfo: info];
  RELEASE(name);
  RELEASE(object);
  RELEASE(info);
  DESTROY(self);
  return RETAIN(n);
}

@end
