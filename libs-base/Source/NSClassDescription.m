/** NSClassDescription
   Copyright (C) 2000 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date:	2000

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

   <title>NSClassDescription class reference</title>
   $Date$ $Revision$
*/

#import "common.h"
#import "Foundation/NSClassDescription.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSNotification.h"


/**
 *  Each instance of this class provides descriptive information for an
 *  Objective C class.  This is used for key-value coding, a framework
 *  used in Cocoa for scripting with Objective-C objects.  Scripting is
 *  available in GNUstep in many ways, however those implementations do
 *  not make use of class descriptions.  Therefore the primary purpose
 *  of this class is to smooth the process of porting between GNUstep
 *  and other OpenStep-derived systems.
 */
@implementation NSClassDescription

static NSRecursiveLock	*mapLock = nil;
static NSMapTable	*classMap;

/**
 * Returns the class description for aClass.  If there is no such description
 * available, sends an
 * <code>NSClassDescriptionNeededForClassNotification</code> (with aClass as
 * its object) so that objects providing class descriptions can register one,
 * and tries again to find one.<br /> Returns nil if there is no description
 * found.<br /> Handles locking to ensure thread safety and ensures that the
 * returned object will not be destroyed by other threads.
 */
+ (NSClassDescription*) classDescriptionForClass: (Class)aClass
{
  NSClassDescription	*description;

  [mapLock lock];
  description = NSMapGet(classMap, aClass);
  if (description == nil)
    {
      NSNotificationCenter	*nc;

      /*
       * As we have a recursive lock, we can ask other objects to
       * supply descriptuions right now, without having to unlock
       * and re-lock
       */
      nc = [NSNotificationCenter defaultCenter];
      [nc postNotificationName: NSClassDescriptionNeededForClassNotification
                        object: aClass];
      description = NSMapGet(classMap, aClass);
    }
  IF_NO_GC([description retain];)
  [mapLock unlock];

  return AUTORELEASE(description);
}

+ (void) initialize
{
  if (self == [NSClassDescription class])
    {
      classMap = NSCreateMapTable(NSObjectMapKeyCallBacks,
        NSObjectMapValueCallBacks, 100);
      [[NSObject leakAt: &classMap] release];
      mapLock = [NSRecursiveLock new];
      [[NSObject leakAt: &mapLock] release];
    }
}

/**
 * Invalidates the cache of class descriptions so the new descriptions
 * will be fetched as required and begin to refill the cache. You need
 * this only if you suspect that a class description should have
 * changed.
 */
+ (void) invalidateClassDescriptionCache
{
  [mapLock lock];
  NSResetMapTable(classMap);
  [mapLock unlock];
}

/**
 * Registers aDescription for aClass ... placing it in the cache and
 * replacing any previous version.
 */
+ (void) registerClassDescription: (NSClassDescription*)aDescription
			 forClass: (Class)aClass
{
  if (aDescription != nil && aClass != 0)
    {
      [mapLock lock];
      NSMapInsert(classMap, aClass, aDescription);
      [mapLock unlock];
    }
}

/** <override-subclass />
 * Returns the attribute keys - default implementation returns nil.
 */
- (NSArray*) attributeKeys
{
  return nil;
}

/** <override-subclass />
 * Returns the inverse relationship keys - default implementation returns nil.
 */
- (NSString*) inverseForRelationshipKey: (NSString*)aKey
{
  return nil;
}

/** <override-subclass />
 * Returns the to many relationship keys - default implementation returns nil.
 */
- (NSArray*) toManyRelationshipKeys
{
  return nil;
}

/** <override-subclass />
 * Returns the to one relationship keys - default implementation returns nil.
 */
- (NSArray*) toOneRelationshipKeys
{
  return nil;
}

@end



@implementation NSObject(NSClassDescriptionPrimitives)

static Class	NSClassDescriptionClass = 0;

/**
 * Returns the attribute keys supplied by the
 * <ref id="NSClassDescription" type="class"/>
 * object for the receivers class.
 */
- (NSArray*) attributeKeys
{
  return [[self classDescription] attributeKeys];
}

/**
 * Returns the <ref id="NSClassDescription" type="class" />
 * object for the receivers class.
 */
- (NSClassDescription*) classDescription
{
  if (NSClassDescriptionClass == 0)
    {
      NSClassDescriptionClass = [NSClassDescription class];
    }
  return [NSClassDescriptionClass classDescriptionForClass: [self class]];
}

/**
 * Returns the inverse relationship keys supplied by the
 * <ref id="NSClassDescription" type="class" />
 * object for the receivers class.
 */
- (NSString*) inverseForRelationshipKey: (NSString*)aKey
{
  return [[self classDescription] inverseForRelationshipKey: aKey];
}

/**
 * Returns the to many relationship keys supplied by the
 * <ref id="NSClassDescription" type="class" />
 * object for the receivers class.
 */
- (NSArray*) toManyRelationshipKeys
{
  return [[self classDescription] toManyRelationshipKeys];
}

/**
 * Returns the to one relationship keys supplied by the
 * <ref id="NSClassDescription" type="class" />
 * object for the receivers class.
 */
- (NSArray*) toOneRelationshipKeys
{
  return [[self classDescription] toOneRelationshipKeys];
}

@end

