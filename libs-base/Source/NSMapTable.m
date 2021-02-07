/** NSMapTable implementation for GNUStep.
 * Copyright (C) 2009  Free Software Foundation, Inc.
 *
 * Author: Richard Frith-Macdonald <rfm@gnu.org>
 *
 * This file is part of the GNUstep Base Library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110 USA.
 *
 * <title>NSMapTable class reference</title>
 * $Date$ $Revision$
 */

#import "common.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPointerFunctions.h"
#import "Foundation/NSMapTable.h"
#import "NSCallBacks.h"

@interface	NSConcreteMapTable : NSMapTable
@end

@implementation	NSMapTable

static Class	abstractClass = 0;
static Class	concreteClass = 0;

+ (id) allocWithZone: (NSZone*)aZone
{
  if (self == abstractClass)
    {
      return NSAllocateObject(concreteClass, 0, aZone);
    }
  return NSAllocateObject(self, 0, aZone);
}

+ (void) initialize
{
  if (abstractClass == 0)
    {
      abstractClass = [NSMapTable class];
      concreteClass = [NSConcreteMapTable class];
    }
}

+ (id) mapTableWithKeyOptions: (NSPointerFunctionsOptions)keyOptions
		 valueOptions: (NSPointerFunctionsOptions)valueOptions
{
  NSMapTable	*t;

  t = [self allocWithZone: NSDefaultMallocZone()];
  t = [t initWithKeyOptions: keyOptions
	       valueOptions: valueOptions
		   capacity: 0];
  return AUTORELEASE(t);
}

+ (id) mapTableWithStrongToStrongObjects
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality
			 valueOptions: NSPointerFunctionsObjectPersonality];
}

+ (id) mapTableWithStrongToWeakObjects
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality
			 valueOptions: NSPointerFunctionsObjectPersonality
    | NSPointerFunctionsZeroingWeakMemory];
}

+ (id) mapTableWithWeakToStrongObjects
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality
    | NSPointerFunctionsZeroingWeakMemory
			 valueOptions: NSPointerFunctionsObjectPersonality];
}

+ (id) mapTableWithWeakToWeakObjects
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality
    | NSPointerFunctionsZeroingWeakMemory
			 valueOptions: NSPointerFunctionsObjectPersonality
    | NSPointerFunctionsZeroingWeakMemory];
}

+ (id) strongToStrongObjectsMapTable
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality
                         valueOptions: NSPointerFunctionsObjectPersonality];
}

+ (id) strongToWeakObjectsMapTable
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality 
                         valueOptions: NSPointerFunctionsObjectPersonality |
                                         NSMapTableWeakMemory];
}

+ (id) weakToStrongObjectsMapTable
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality |
                                         NSMapTableWeakMemory
                         valueOptions: NSPointerFunctionsObjectPersonality];
}

+ (id) weakToWeakObjectsMapTable
{
  return [self mapTableWithKeyOptions: NSPointerFunctionsObjectPersonality | 
                                         NSMapTableWeakMemory
                         valueOptions: NSPointerFunctionsObjectPersonality |
                                         NSMapTableWeakMemory];
}

- (id) initWithKeyOptions: (NSPointerFunctionsOptions)keyOptions
	     valueOptions: (NSPointerFunctionsOptions)valueOptions
	         capacity: (NSUInteger)initialCapacity
{
  NSPointerFunctions	*k;
  NSPointerFunctions	*v;
  id			o;

  k = [[NSPointerFunctions alloc] initWithOptions: keyOptions];
  v = [[NSPointerFunctions alloc] initWithOptions: valueOptions];
  o = [self initWithKeyPointerFunctions: k
		  valuePointerFunctions: v
			       capacity: initialCapacity];
  [k release];
  [v release];
  return o;
}

- (id) initWithKeyPointerFunctions: (NSPointerFunctions*)keyFunctions
	     valuePointerFunctions: (NSPointerFunctions*)valueFunctions
			  capacity: (NSUInteger)initialCapacity
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) copyWithZone: (NSZone*)aZone
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (NSUInteger) count
{
  [self subclassResponsibility: _cmd];
  return (NSUInteger)0;
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state 	
				   objects: (id*)stackbuf
				     count: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return (NSUInteger)0;
}

- (NSDictionary*) dictionaryRepresentation
{
  NSEnumerator		*enumerator;
  NSMutableDictionary	*dictionary;
  id			key;

  dictionary = [NSMutableDictionary dictionaryWithCapacity: [self count]];
  enumerator = [self keyEnumerator];
  while ((key = [enumerator nextObject]) != nil)
    {
      [dictionary setObject: [self objectForKey: key] forKey: key];
    }
  return [[dictionary copy] autorelease];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [self subclassResponsibility: _cmd];
}

- (NSUInteger) hash
{
  return [self count];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: abstractClass] == NO) return NO;
  return NSCompareMapTables(self, other);
}

- (NSEnumerator*) keyEnumerator
{
  return [self subclassResponsibility: _cmd];
}

- (NSPointerFunctions*) keyPointerFunctions
{
  return [self subclassResponsibility: _cmd];
}

- (NSEnumerator*) objectEnumerator
{
  return [self subclassResponsibility: _cmd];
}

- (id) objectForKey: (id)aKey
{
  return [self subclassResponsibility: _cmd];
}

- (void) removeAllObjects
{
  NSUInteger	count = [self count];

  if (count > 0)
    {
      NSEnumerator	*enumerator;
      NSMutableArray	*array;
      id		key;

      array = [[NSMutableArray alloc] initWithCapacity: count];
      enumerator = [self objectEnumerator];
      while ((key = [enumerator nextObject]) != nil)
	{
	  [array addObject: key];
	}
      enumerator = [array objectEnumerator];
      while ((key = [enumerator nextObject]) != nil)
	{
	  [self removeObjectForKey: key];
	}
      [array release];
    }
}

- (void) removeObjectForKey: (id)aKey
{
  [self subclassResponsibility: _cmd];
}

- (void) setObject: (id)anObject forKey: (id)aKey
{
  [self subclassResponsibility: _cmd];
}

- (NSPointerFunctions*) valuePointerFunctions
{
  return [self subclassResponsibility: _cmd];
}
@end

