/** Interface to concrete implementation of NSDictionary
   Copyright (C) 2007 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: May 2007

   This file is part of the GNUstep Base Library.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02111 USA.
   */


#import "common.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSPortCoder.h"
// For private method _decodeArrayOfObjectsForKey:
#import "Foundation/NSKeyedArchiver.h"

#import "GNUstepBase/GSObjCRuntime.h"
#import "GNUstepBase/NSDebug+GNUstepBase.h"


/*
 *	The 'Fastmap' stuff provides an inline implementation of a mapping
 *	table - for maximum performance.
 */
#define	GSI_MAP_KTYPES		GSUNION_OBJ
#define	GSI_MAP_VTYPES		GSUNION_OBJ
#define	GSI_MAP_HASH(M, X)		[[X.obj lowercaseString] hash]
#define	GSI_MAP_EQUAL(M, X,Y)		(([X.obj caseInsensitiveCompare: Y.obj]\
				== NSOrderedSame) ? YES : NO)
#define	GSI_MAP_RETAIN_KEY(M, X)	((X).obj) = \
				[((id)(X).obj) copyWithZone: map->zone]

#include	"GNUstepBase/GSIMap.h"

@interface _GSInsensitiveDictionary : NSDictionary
{
@public
  GSIMapTable_t	map;
}
@end

@interface _GSMutableInsensitiveDictionary : NSMutableDictionary
{
@public
  GSIMapTable_t	map;
  unsigned long	_version;
}
@end

@interface _GSInsensitiveDictionaryKeyEnumerator : NSEnumerator
{
  _GSInsensitiveDictionary		*dictionary;
  GSIMapEnumerator_t	enumerator;
}
- (id) initWithDictionary: (NSDictionary*)d;
@end

@interface _GSInsensitiveDictionaryObjectEnumerator : _GSInsensitiveDictionaryKeyEnumerator
@end

@implementation _GSInsensitiveDictionary

static SEL	nxtSel;
static SEL	objSel;

+ (void) initialize
{
  if (self == [_GSInsensitiveDictionary class])
    {
      nxtSel = @selector(nextObject);
      objSel = @selector(objectForKey:);
    }
}

- (id) copyWithZone: (NSZone*)zone
{
  return RETAIN(self);
}

- (NSUInteger) count
{
  return map.nodeCount;
}

- (void) dealloc
{
  GSIMapEmptyMap(&map);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      [super encodeWithCoder: aCoder];
    }
  else
    {
      NSUInteger	count = map.nodeCount;
      SEL		sel = @selector(encodeObject:);
      id (*imp)(id,SEL,id)
	= (id (*)(id,SEL,id))[aCoder methodForSelector: sel];
      GSIMapEnumerator_t	enumerator = GSIMapEnumeratorForMap(&map);
      GSIMapNode	node = GSIMapEnumeratorNextNode(&enumerator);

      [aCoder encodeValueOfObjCType: @encode(NSUInteger) at: &count];
      while (node != 0)
	{
	  (*imp)(aCoder, sel, node->key.obj);
	  (*imp)(aCoder, sel, node->value.obj);
	  node = GSIMapEnumeratorNextNode(&enumerator);
	}
      GSIMapEndEnumerator(&enumerator);
    }
}

- (NSUInteger) hash
{
  return map.nodeCount;
}

- (id) init
{
  return [self initWithObjects: 0 forKeys: 0 count: 0];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      self = [super initWithCoder: aCoder];
    }
  else
    {
      NSUInteger	count;
      id		key;
      id		value;
      SEL		sel = @selector(decodeValueOfObjCType:at:);
      void (*imp)(id,SEL,const char*,void*)
	= (void (*)(id,SEL,const char*,void*))[aCoder methodForSelector: sel];
      const char	*type = @encode(id);

      [aCoder decodeValueOfObjCType: @encode(NSUInteger)
	                         at: &count];

      GSIMapInitWithZoneAndCapacity(&map, [self zone], count);
      while (count-- > 0)
        {
	  (*imp)(aCoder, sel, type, &key);
	  (*imp)(aCoder, sel, type, &value);
	  GSIMapAddPairNoRetain(&map, (GSIMapKey)key, (GSIMapVal)value);
	}
    }
  return self;
}

/* Designated initialiser */
- (id) initWithObjects: (const id[])objs forKeys: (const id <NSCopying>[])keys count: (NSUInteger)c
{
  NSUInteger	i;

  GSIMapInitWithZoneAndCapacity(&map, [self zone], c);
  for (i = 0; i < c; i++)
    {
      GSIMapNode	node;

      if (keys[i] == nil)
	{
	  DESTROY(self);
	  [NSException raise: NSInvalidArgumentException
		      format: @"Tried to init dictionary with nil key"];
	}
      if (objs[i] == nil)
	{
	  DESTROY(self);
	  [NSException raise: NSInvalidArgumentException
		      format: @"Tried to init dictionary with nil value"];
	}

      node = GSIMapNodeForKey(&map, (GSIMapKey)(id)keys[i]);
      if (node)
	{
	  IF_NO_GC(RETAIN(objs[i]));
	  RELEASE(node->value.obj);
	  node->value.obj = objs[i];
	}
      else
	{
	  GSIMapAddPair(&map, (GSIMapKey)(id)keys[i], (GSIMapVal)objs[i]);
	}
    }
  return self;
}

/*
 *	This avoids using the designated initialiser for performance reasons.
 */
- (id) initWithDictionary: (NSDictionary*)other
		copyItems: (BOOL)shouldCopy
{
  NSZone	*z = [self zone];
  NSUInteger	c = [other count];

  GSIMapInitWithZoneAndCapacity(&map, z, c);
  if (c > 0)
    {
      NSEnumerator	*e = [other keyEnumerator];
      id (*nxtObj)(id, SEL) = (id (*)(id,SEL))[e methodForSelector: nxtSel];
      id (*otherObj)(id, SEL, id)
	= (id (*)(id,SEL,id))[other methodForSelector: objSel];
      BOOL		isProxy = [other isProxy];
      NSUInteger	i;

      for (i = 0; i < c; i++)
	{
	  GSIMapNode	node;
	  id		k;
	  id		o;

	  if (isProxy == YES)
	    {
	      k = [e nextObject];
	      o = [other objectForKey: k];
	    }
	  else
	    {
	      k = (*nxtObj)(e, nxtSel);
	      o = (*otherObj)(other, objSel, k);
	    }
	  k = [k copyWithZone: z];
	  if (k == nil)
	    {
	      DESTROY(self);
	      [NSException raise: NSInvalidArgumentException
			  format: @"Tried to init dictionary with nil key"];
	    }
	  if (shouldCopy)
	    {
	      o = [o copyWithZone: z];
	    }
	  else
	    {
	      o = RETAIN(o);
	    }
	  if (o == nil)
	    {
	      DESTROY(self);
	      [NSException raise: NSInvalidArgumentException
			  format: @"Tried to init dictionary with nil value"];
	    }

	  node = GSIMapNodeForKey(&map, (GSIMapKey)k);
	  if (node)
	    {
	      RELEASE(node->value.obj);
	      node->value.obj = o;
	    }
	  else
	    {
	      GSIMapAddPairNoRetain(&map, (GSIMapKey)k, (GSIMapVal)o);
	    }
	}
    }
  return self;
}

- (BOOL) isEqualToDictionary: (NSDictionary*)other
{
  NSUInteger	count;

  if (other == self)
    {
      return YES;
    }
  count = map.nodeCount;
  if (count == [other count])
    {
      if (count > 0)
	{
	  GSIMapEnumerator_t	enumerator;
	  GSIMapNode		node;
	  id (*otherObj)(id, SEL, id)
	    = (id (*)(id,SEL,id))[other methodForSelector: objSel];

	  enumerator = GSIMapEnumeratorForMap(&map);
	  while ((node = GSIMapEnumeratorNextNode(&enumerator)) != 0)
	    {
	      id o1 = node->value.obj;
	      id o2 = (*otherObj)(other, objSel, node->key.obj);

	      if (o1 != o2 && [o1 isEqual: o2] == NO)
		{
		  GSIMapEndEnumerator(&enumerator);
		  return NO;
		}
	    }
	  GSIMapEndEnumerator(&enumerator);
	}
      return YES;
    }
  return NO;
}

- (NSEnumerator*) keyEnumerator
{
  return AUTORELEASE([[_GSInsensitiveDictionaryKeyEnumerator allocWithZone:
    NSDefaultMallocZone()] initWithDictionary: self]);
}

- (BOOL) makeImmutable
{
  return YES;
}

- (NSEnumerator*) objectEnumerator
{
  return AUTORELEASE([[_GSInsensitiveDictionaryObjectEnumerator allocWithZone:
    NSDefaultMallocZone()] initWithDictionary: self]);
}

- (id) objectForKey: aKey
{
  if (aKey != nil)
    {
      GSIMapNode	node  = GSIMapNodeForKey(&map, (GSIMapKey)aKey);

      if (node)
	{
	  return node->value.obj;
	}
    }
  return nil;
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (__unsafe_unretained id[])stackbuf
                                     count: (NSUInteger)len
{
  state->mutationsPtr = (unsigned long *)self;
  return GSIMapCountByEnumeratingWithStateObjectsCount
    (&map, state, stackbuf, len);
}

@end

@implementation _GSMutableInsensitiveDictionary

+ (void) initialize
{
  if (self == [_GSMutableInsensitiveDictionary class])
    {
      GSObjCAddClassBehavior(self, [_GSInsensitiveDictionary class]);
    }
}

- (id) copyWithZone: (NSZone*)zone
{
  NSDictionary	*copy = [_GSInsensitiveDictionary allocWithZone: zone];

  return [copy initWithDictionary: self copyItems: NO];
}

- (id) init
{
  return [self initWithCapacity: 0];
}

/* Designated initialiser */
- (id) initWithCapacity: (NSUInteger)cap
{
  GSIMapInitWithZoneAndCapacity(&map, [self zone], cap);
  return self;
}

- (BOOL) makeImmutable
{
  GSClassSwizzle(self, [_GSInsensitiveDictionary class]);
  return YES;
}

- (id) makeImmutableCopyOnFail: (BOOL)force
{
  GSClassSwizzle(self, [_GSInsensitiveDictionary class]);
  return self;
}

- (void) setObject: (id)anObject forKey: (id)aKey
{
  GSIMapNode	node;

  _version++;
  if (aKey == nil)
    {
      NSException	*e;

      e = [NSException exceptionWithName: NSInvalidArgumentException
				  reason: @"Tried to add nil key to dictionary"
				userInfo: self];
      [e raise];
    }
  if ([aKey isKindOfClass: [NSString class]] == NO)
    {
      NSException	*e;

      e = [NSException exceptionWithName: NSInvalidArgumentException
				  reason: @"Tried to add non-string key"
				userInfo: self];
      [e raise];
    }
  node = GSIMapNodeForKey(&map, (GSIMapKey)aKey);
  if (node)
    {
      IF_NO_GC(RETAIN(anObject));
      RELEASE(node->value.obj);
      node->value.obj = anObject;
    }
  else
    {
      GSIMapAddPair(&map, (GSIMapKey)aKey, (GSIMapVal)anObject);
    }
  _version++;
}

- (void) removeAllObjects
{
  _version++;
  GSIMapCleanMap(&map);
  _version++;
}

- (void) removeObjectForKey: (id)aKey
{
  if (aKey == nil)
    {
      NSWarnMLog(@"attempt to remove nil key from dictionary %@", self);
      return;
    }
  _version++;
  GSIMapRemoveKey(&map, (GSIMapKey)aKey);
  _version++;
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (__unsafe_unretained id[])stackbuf
                                     count: (NSUInteger)len
{
  state->mutationsPtr = &_version;
  return GSIMapCountByEnumeratingWithStateObjectsCount
    (&map, state, stackbuf, len);
}

@end

@implementation _GSInsensitiveDictionaryKeyEnumerator

- (id) initWithDictionary: (NSDictionary*)d
{
  [super init];
  dictionary = (_GSInsensitiveDictionary*)RETAIN(d);
  enumerator = GSIMapEnumeratorForMap(&dictionary->map);
  return self;
}

- (id) nextObject
{
  GSIMapNode	node = GSIMapEnumeratorNextNode(&enumerator);

  if (node == 0)
    {
      return nil;
    }
  return node->key.obj;
}

- (void) dealloc
{
  GSIMapEndEnumerator(&enumerator);
  RELEASE(dictionary);
  [super dealloc];
}

@end

@implementation _GSInsensitiveDictionaryObjectEnumerator

- (id) nextObject
{
  GSIMapNode	node = GSIMapEnumeratorNextNode(&enumerator);

  if (node == 0)
    {
      return nil;
    }
  return node->value.obj;
}

@end

