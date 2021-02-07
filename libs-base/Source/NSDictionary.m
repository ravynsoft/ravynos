/** NSDictionary - Dictionary object to store key/value pairs
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   From skeleton by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: Mar 1995

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

   <title>NSDictionary class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSOrderedSet.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSFileManager.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSKeyValueCoding.h"
#import "Foundation/NSUserDefaults.h"
// For private method _decodeArrayOfObjectsForKey:
#import "Foundation/NSKeyedArchiver.h"
#import "GSPrivate.h"
#import "GSFastEnumeration.h"
#import "GSDispatch.h"

static BOOL GSMacOSXCompatiblePropertyLists(void)
{
  if (GSPrivateDefaultsFlag(NSWriteOldStylePropertyLists) == YES)
    return NO;
  return GSPrivateDefaultsFlag(GSMacOSXCompatible);
}

@class	GSDictionary;
@interface GSDictionary : NSObject	// Help the compiler
@end
@class	GSMutableDictionary;
@interface GSMutableDictionary : NSObject	// Help the compiler
@end

extern void	GSPropertyListMake(id,NSDictionary*,BOOL,BOOL,unsigned,id*);


static Class NSArray_class;
static Class NSDictionaryClass;
static Class NSMutableDictionaryClass;
static Class GSDictionaryClass;
static Class GSMutableDictionaryClass;

static SEL	eqSel;
static SEL	nxtSel;
static SEL	objSel;
static SEL	remSel;
static SEL	setSel;
static SEL	appSel;

/**
 *  <p>This class and its subclasses store key-value pairs, where the key and
 *  the value are objects.  A great many utility methods for working with
 *  dictionaries are provided as part of this class, including the ability to
 *  retrieve multiple entries simultaneously, obtain sorted contents, and
 *  read/write from/to a serialized representation.</p>
 *
 *  <p>The keys are copied and values are retained by the implementation,
 *  and both are released when either their entry is dropped or the entire
 *  dictionary is deallocated.<br />
 *  As in the OS X implementation, keys must therefore implement the
 *  [(NSCopying)] protocol.
 *  </p>
 *
 *  <p>Objects of this class are immutable.  For a mutable version, use the
 *  [NSMutableDictionary] subclass.</p>
 *
 *  <p>The basic functionality in <code>NSDictionary</code> is similar to that
 *  in Java's <code>HashMap</code>, and like that class it includes no locking
 *  code and is not thread-safe.  If the contents will be modified and
 *  accessed from multiple threads you should enclose critical operations
 *  within locks (see [NSLock]).</p>
 */
@implementation NSDictionary

+ (void) initialize
{
  if (self == [NSDictionary class])
    {
      eqSel = @selector(isEqual:);
      nxtSel = @selector(nextObject);
      objSel = @selector(objectForKey:);
      remSel = @selector(removeObjectForKey:);
      setSel = @selector(setObject:forKey:);
      appSel = @selector(appendString:);
      NSArray_class = [NSArray class];
      NSDictionaryClass = self;
      GSDictionaryClass = [GSDictionary class];
      [NSMutableDictionary class];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSDictionaryClass)
    {
      return NSAllocateObject(GSDictionaryClass, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

/**
 * Returns a new copy of the receiver.<br />
 * The default abstract implementation of a copy is to use the
 * -initWithDictionary:copyItems: method with the flag set to YES.<br />
 * Immutable subclasses generally simply retain and return the receiver.
 */
- (id) copyWithZone: (NSZone*)z
{
  NSDictionary	*copy = [NSDictionaryClass allocWithZone: z];

  return [copy initWithDictionary: self copyItems: NO];
}

/**
 * Returns an unsigned integer which is the number of elements
 * stored in the dictionary.
 */
- (NSUInteger) count
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (void) enumerateKeysAndObjectsUsingBlock:
  (GSKeysAndObjectsEnumeratorBlock)aBlock
{
  [self enumerateKeysAndObjectsWithOptions: 0
                                usingBlock: aBlock];
}

- (void) enumerateKeysAndObjectsWithOptions: (NSEnumerationOptions)opts
  usingBlock: (GSKeysAndObjectsEnumeratorBlock)aBlock
{
  /*
   * NOTE: According to the Cocoa documentation, NSEnumerationReverse is
   * undefined for NSDictionary. NSEnumerationConcurrent will be handled through
   * the GS_DISPATCH_* macros if libdispatch is available.
   */
   id<NSFastEnumeration> enumerator = [self keyEnumerator];
   SEL objectForKeySelector = @selector(objectForKey:);
   IMP objectForKey = [self methodForSelector: objectForKeySelector];
   BLOCK_SCOPE BOOL shouldStop = NO;
   id obj;

   GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
   FOR_IN(id, key, enumerator)
     obj = (*objectForKey)(self, objectForKeySelector, key);
     GS_DISPATCH_SUBMIT_BLOCK(enumQueueGroup, enumQueue,
     if (shouldStop == NO) {, }, aBlock, key, obj, &shouldStop);
     if (YES == shouldStop)
       {
	 break;
       }
   END_FOR_IN(enumerator)
   GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
}

/**
 * <p>In MacOS-X class clusters do not have designated initialisers,
 * and there is a general rule that -init is treated as the designated
 * initialiser of the class cluster, but that other intitialisers
 * may not work s expected an would need to be individually overridden
 * in any subclass.
 * </p>
 * <p>GNUstep tries to make it easier to subclass a class cluster,
 * by making class clusters follow the same convention as normal
 * classes, so the designated initialiser is the <em>richest</em>
 * initialiser.  This means that all other initialisers call the
 * documented designated initialiser (which calls -init only for
 * MacOS-X compatibility), and anyone writing a subclass only needs
 * to override that one initialiser in order to have all the other
 * ones work.
 * </p>
 * <p>For MacOS-X compatibility, you may also need to override various
 * other initialisers.  Exactly which ones, you will need to determine
 * by trial on a MacOS-X system ... and may vary between releases of
 * MacOS-X.  So to be safe, on MacOS-X you probably need to re-implement
 * <em>all</em> the class cluster initialisers you might use in conjunction
 * with your subclass.
 * </p>
 */
- (id) init
{
  self = [super init];
  return self;
}

/** <init /> <override-subclass />
 * Initializes contents to the given objects and keys.
 * The two arrays must have the same size.
 * The n th element of the objects array is associated with the n th
 * element of the keys array.<br />
 * Calls -init (which does nothing but maintain MacOS-X compatibility),
 * and needs to be re-implemented in subclasses in order to have all
 * other initialisers work.
 */
- (id) initWithObjects: (const id[])objects
	       forKeys: (const id <NSCopying>[])keys
		 count: (NSUInteger)count
{
  self = [self init];
  return self;
}

/**
 * Return an enumerator object containing all the keys of the dictionary.
 */
- (NSEnumerator*) keyEnumerator
{
  return [self subclassResponsibility: _cmd];
}

/**
 * Returns the object in the dictionary corresponding to aKey, or nil if
 * the key is not present.
 */
- (id) objectForKey: (id)aKey
{
  return [self subclassResponsibility: _cmd];
}

- (id) objectForKeyedSubscript: (id)aKey
{
  return [self objectForKey: aKey];
}

/**
 * Return an enumerator object containing all the objects of the dictionary.
 */
- (NSEnumerator*) objectEnumerator
{
  return [self subclassResponsibility: _cmd];
}

/**
 * Returns a new instance containing the same objects as
 * the receiver.<br />
 * The default implementation does this by calling the
 * -initWithDictionary:copyItems: method on a newly created object,
 * and passing it NO to tell it just to retain the items.
 */
- (id) mutableCopyWithZone: (NSZone*)z
{
  NSMutableDictionary	*copy = [NSMutableDictionaryClass allocWithZone: z];

  return [copy initWithDictionary: self copyItems: NO];
}

- (Class) classForCoder
{
  return NSDictionaryClass;
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  unsigned	count = [self count];

  if ([aCoder allowsKeyedCoding])
    {
      id	key;
      unsigned	i;

      if ([aCoder class] == [NSKeyedArchiver class])
	{
	  NSArray	*keys = [self allKeys];
	  id		objects = [NSMutableArray arrayWithCapacity: count];

	  for (i = 0; i < count; i++)
	    {
	      key = [keys objectAtIndex: i];
	      [objects addObject: [self objectForKey: key]];
	    }
	  [(NSKeyedArchiver*)aCoder _encodeArrayOfObjects: keys
						   forKey: @"NS.keys"];
	  [(NSKeyedArchiver*)aCoder _encodeArrayOfObjects: objects
						   forKey: @"NS.objects"];
	}
      else if (count > 0)
	{
	  NSEnumerator	*enumerator = [self keyEnumerator];

	  i = 0;
	  while ((key = [enumerator nextObject]) != nil)
	    {
	      NSString	*s;

	      s = [NSString stringWithFormat: @"NS.key.%u", i];
	      [aCoder encodeObject: key forKey: s];
	      s = [NSString stringWithFormat: @"NS.object.%u", i];
	      [aCoder encodeObject: [self objectForKey: key] forKey: s];
	      i++;
	    }
	}
    }
  else
    {
      [aCoder encodeValueOfObjCType: @encode(unsigned) at: &count];
      if (count > 0)
	{
	  NSEnumerator	*enumerator = [self keyEnumerator];
	  id		key;
	  IMP		enc;
	  IMP		nxt;
	  IMP		ofk;

	  nxt = [enumerator methodForSelector: @selector(nextObject)];
	  enc = [aCoder methodForSelector: @selector(encodeObject:)];
	  ofk = [self methodForSelector: @selector(objectForKey:)];

	  while ((key = (*nxt)(enumerator, @selector(nextObject))) != nil)
	    {
	      id	val = (*ofk)(self, @selector(objectForKey:), key);

	      (*enc)(aCoder, @selector(encodeObject:), key);
	      (*enc)(aCoder, @selector(encodeObject:), val);
	    }
	}
    }
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  if ([aCoder allowsKeyedCoding])
    {
      id keys = nil;
      id objects = nil;

      if ([aCoder containsValueForKey: @"NS.keys"])
        {
          keys = [(NSKeyedUnarchiver*)aCoder _decodeArrayOfObjectsForKey:
                                        @"NS.keys"];
          objects = [(NSKeyedUnarchiver*)aCoder _decodeArrayOfObjectsForKey:
                                           @"NS.objects"];
        }
      else if ([aCoder containsValueForKey: @"dict.sortedKeys"])
	{
          keys = [aCoder decodeObjectForKey: @"dict.sortedKeys"];
          objects = [aCoder decodeObjectForKey: @"dict.values"];
        }

      if (keys == nil)
	{
	  unsigned	i = 0;
	  NSString	*key;
	  id		val;

	  keys = [NSMutableArray arrayWithCapacity: 2];
	  objects = [NSMutableArray arrayWithCapacity: 2];
	  key = [NSString stringWithFormat: @"NS.object.%u", i];
	  val = [(NSKeyedUnarchiver*)aCoder decodeObjectForKey: key];

	  while (val != nil)
	    {
	      [objects addObject: val];
	      key = [NSString stringWithFormat: @"NS.key.%u", i];
	      val = [(NSKeyedUnarchiver*)aCoder decodeObjectForKey: key];
	      [keys addObject: val];
	      i++;
	      key = [NSString stringWithFormat: @"NS.object.%u", i];
	      val = [(NSKeyedUnarchiver*)aCoder decodeObjectForKey: key];
	    }
	}
      self = [self initWithObjects: objects forKeys: keys];
    }
  else
    {
      unsigned	count;

      [aCoder decodeValueOfObjCType: @encode(unsigned) at: &count];
      if (count > 0)
        {
	  id	*keys = NSZoneMalloc(NSDefaultMallocZone(), sizeof(id)*count);
	  id	*vals = NSZoneMalloc(NSDefaultMallocZone(), sizeof(id)*count);
	  unsigned	i;
	  IMP	dec;

	  dec = [aCoder methodForSelector: @selector(decodeObject)];
	  for (i = 0; i < count; i++)
	    {
	      keys[i] = (*dec)(aCoder, @selector(decodeObject));
	      vals[i] = (*dec)(aCoder, @selector(decodeObject));
	    }
	  self = [self initWithObjects: vals forKeys: keys count: count];
	  NSZoneFree(NSDefaultMallocZone(), keys);
	  NSZoneFree(NSDefaultMallocZone(), vals);
	}
    }
  return self;
}

/**
 *  Returns a new autoreleased empty dictionary.
 */
+ (id) dictionary
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()] init]);
}

/**
 * Returns a newly created dictionary with the keys and objects
 * of otherDictionary.
 * (The keys and objects are not copied.)
 */
+ (id) dictionaryWithDictionary: (NSDictionary*)otherDictionary
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithDictionary: otherDictionary]);
}

/**
 * Returns a dictionary created using the given objects and keys.
 * The two arrays must have the same size.
 * The n th element of the objects array is associated with the n th
 * element of the keys array.
 */
+ (id) dictionaryWithObjects: (const id[])objects
		     forKeys: (const id <NSCopying>[])keys
		       count: (NSUInteger)count
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithObjects: objects forKeys: keys count: count]);
}

- (NSUInteger) hash
{
  return [self count];
}

/**
 * Initialises a dictionary created using the given objects and keys.
 * The two arrays must have the same size.
 * The n th element of the objects array is associated with the n th
 * element of the keys array.
 */
- (id) initWithObjects: (NSArray*)objects forKeys: (NSArray*)keys
{
  unsigned	objectCount = [objects count];

  if (objectCount != [keys count])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"init with obj and key arrays of different sizes"];
    }
  else
    {
      GS_BEGINIDBUF(o, objectCount*2);

      if ([objects isProxy])
	{
	  unsigned	i;

	  for (i = 0; i < objectCount; i++)
	    {
	      o[i] = [objects objectAtIndex: i];
	    }
	}
      else
	{
          [objects getObjects: o];
	}
      if ([keys isProxy])
	{
	  unsigned	i;

	  for (i = 0; i < objectCount; i++)
	    {
	      o[objectCount + i] = [keys objectAtIndex: i];
	    }
	}
      else
	{
          [keys getObjects: o + objectCount];
	}
      self = [self initWithObjects: o
			   forKeys: o + objectCount
			     count: objectCount];
      GS_ENDIDBUF();
    }
  return self;
}

/**
 * Initialises a dictionary created using the list given as argument.
 * The list is alternately composed of objects and keys and
 * terminated by nil.  Thus, the list's length must be even,
 * followed by nil.
 */
- (id) initWithObjectsAndKeys: (id)firstObject, ...
{
  GS_USEIDPAIRLIST(firstObject,
    self = [self initWithObjects: __objects forKeys: __pairs count: __count/2]);
  return self;
}

/**
 * Returns a dictionary created using the list given as argument.
 * The list is alternately composed of objects and keys and
 * terminated by nil.  Thus, the list's length must be even,
 * followed by nil.
 */
+ (id) dictionaryWithObjectsAndKeys: (id)firstObject, ...
{
  id	o = [self allocWithZone: NSDefaultMallocZone()];

  GS_USEIDPAIRLIST(firstObject,
    o = [o initWithObjects: __objects forKeys: __pairs count: __count/2]);
  return AUTORELEASE(o);
}

/**
 * Returns a dictionary created using the given objects and keys.
 * The two arrays must have the same length.
 * The n th element of the objects array is associated with the n th
 * element of the keys array.
 */
+ (id) dictionaryWithObjects: (NSArray*)objects forKeys: (NSArray*)keys
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithObjects: objects forKeys: keys]);
}

/**
 * Returns a dictionary containing only one object which is associated
 * with a key.
 */
+ (id) dictionaryWithObject: (id)object forKey: (id)key
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithObjects: &object forKeys: &key count: 1]);
}

/**
 * Initializes with the keys and objects of otherDictionary.
 * (The keys and objects are not copied.)
 */
- (id) initWithDictionary: (NSDictionary*)otherDictionary
{
  return [self initWithDictionary: otherDictionary copyItems: NO];
}

/**
 * Initialise dictionary with the keys and values of otherDictionary.
 * If the shouldCopy flag is YES then the values are copied into the
 * newly initialised dictionary, otherwise they are simply retained,
 * on the assumption that it is safe to retain the keys from another
 * dictionary since that other dictionary mwill have copied the keys
 * originally to ensure that they are immutable.
 */
- (id) initWithDictionary: (NSDictionary*)other
		copyItems: (BOOL)shouldCopy
{
  unsigned	c = [other count];

  if (c > 0)
    {
      id		k;
      NSEnumerator	*e = [other keyEnumerator];
      unsigned		i = 0;
      IMP		nxtObj = [e methodForSelector: nxtSel];
      IMP		otherObj = [other methodForSelector: objSel];
      GS_BEGINIDBUF(o, c*2);

      if (shouldCopy)
	{
	  NSZone	*z = [self zone];

	  while ((k = (*nxtObj)(e, nxtSel)) != nil)
	    {
	      o[i] = k;
	      o[c + i] = [(*otherObj)(other, objSel, k) copyWithZone: z];
	      i++;
	    }
	  self = [self initWithObjects: o + c forKeys: o count: i];
	  while (i-- > 0)
	    {
	      [o[c + i] release];
	    }
	}
      else
	{
	  while ((k = (*nxtObj)(e, nxtSel)) != nil)
	    {
	      o[i] = k;
	      o[c + i] = (*otherObj)(other, objSel, k);
	      i++;
	    }
	  self = [self initWithObjects: o + c forKeys: o count: c];
	}
      GS_ENDIDBUF();
    }
  return self;
}

/**
 * <p>Initialises the dictionary with the contents of the specified file,
 * which must contain a dictionary in property-list format.
 * </p>
 * <p>In GNUstep, the property-list format may be either the OpenStep
 * format (ASCII data), or the MacOS-X format (UTF-8 XML data) ... this
 * method will recognise which it is.
 * </p>
 * <p>If there is a failure to load the file for any reason, the receiver
 * will be released and the method will return nil.
 * </p>
 * <p>Works by invoking [NSString-initWithContentsOfFile:] and
 * [NSString-propertyList] then checking that the result is a dictionary.
 * </p>
 */
- (id) initWithContentsOfFile: (NSString*)path
{
  NSString 	*myString;

  myString = [[NSString allocWithZone: NSDefaultMallocZone()]
    initWithContentsOfFile: path];
  if (myString == nil)
    {
      DESTROY(self);
    }
  else
    {
      id result;

      NS_DURING
	{
	  result = [myString propertyList];
	}
      NS_HANDLER
	{
          result = nil;
	}
      NS_ENDHANDLER
      RELEASE(myString);
      if ([result isKindOfClass: NSDictionaryClass])
	{
	  self = [self initWithDictionary: result];
	}
      else
	{
	  NSWarnMLog(@"Contents of file '%@' does not contain a dictionary",
	    path);
	  DESTROY(self);
	}
    }
  return self;
}

/**
 * <p>Initialises the dictionary with the contents of the specified URL,
 * which must contain a dictionary in property-list format.
 * </p>
 * <p>In GNUstep, the property-list format may be either the OpenStep
 * format (ASCII data), or the MacOS-X format (UTF-8 XML data) ... this
 * method will recognise which it is.
 * </p>
 * <p>If there is a failure to load the URL for any reason, the receiver
 * will be released and the method will return nil.
 * </p>
 * <p>Works by invoking [NSString-initWithContentsOfURL:] and
 * [NSString-propertyList] then checking that the result is a dictionary.
 * </p>
 */
- (id) initWithContentsOfURL: (NSURL*)aURL
{
  NSString 	*myString;

  myString = [[NSString allocWithZone: NSDefaultMallocZone()]
    initWithContentsOfURL: aURL];
  if (myString == nil)
    {
      DESTROY(self);
    }
  else
    {
      id result;

      NS_DURING
	{
	  result = [myString propertyList];
	}
      NS_HANDLER
	{
          result = nil;
	}
      NS_ENDHANDLER
      RELEASE(myString);
      if ([result isKindOfClass: NSDictionaryClass])
	{
	  self = [self initWithDictionary: result];
	}
      else
	{
	  NSWarnMLog(@"Contents of URL '%@' does not contain a dictionary",
	    aURL);
	  DESTROY(self);
	}
    }
  return self;
}

/**
 * Returns a dictionary using the file located at path.
 * The file must be a property list containing a dictionary as its root object.
 */
+ (id) dictionaryWithContentsOfFile: (NSString*)path
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithContentsOfFile: path]);
}

/**
 * Returns a dictionary using the contents of aURL.
 * The URL must be a property list containing a dictionary as its root object.
 */
+ (id) dictionaryWithContentsOfURL: (NSURL*)aURL
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithContentsOfURL: aURL]);
}

- (BOOL) isEqual: other
{
  if (other == self)
    return YES;

  if ([other isKindOfClass: NSDictionaryClass])
    return [self isEqualToDictionary: other];

  return NO;
}

/**
 * Two dictionaries are equal if they each hold the same number of
 * entries, each key in one <code>isEqual</code> to a key in the other,
 * and, for a given key, the corresponding value objects also satisfy
 * <code>isEqual</code>.
 */
- (BOOL) isEqualToDictionary: (NSDictionary*)other
{
  unsigned	count;

  if (other == self)
    {
      return YES;
    }
  count = [self count];
  if (count == [other count])
    {
      if (count > 0)
	{
	  NSEnumerator	*e = [self keyEnumerator];
	  IMP		nxtObj = [e methodForSelector: nxtSel];
	  IMP		myObj = [self methodForSelector: objSel];
	  IMP		otherObj = [other methodForSelector: objSel];
	  id		k;

	  while ((k = (*nxtObj)(e, @selector(nextObject))) != nil)
	    {
	      id o1 = (*myObj)(self, objSel, k);
	      id o2 = (*otherObj)(other, objSel, k);

	      if (o1 == o2)
		continue;
	      if ([o1 isEqual: o2] == NO)
		return NO;
	    }
	}
      return YES;
    }
  return NO;
}

/**
 * Returns an array containing all the dictionary's keys.
 */
- (NSArray*) allKeys
{
  unsigned	c = [self count];

  if (c == 0)
    {
      return [NSArray_class array];
    }
  else
    {
      NSEnumerator	*e = [self keyEnumerator];
      IMP		nxtObj = [e methodForSelector: nxtSel];
      unsigned		i;
      id		result;
      GS_BEGINIDBUF(k, c);

      for (i = 0; i < c; i++)
	{
	  k[i] = (*nxtObj)(e, nxtSel);
	  NSAssert (k[i], NSInternalInconsistencyException);
	}
      result = [[NSArray_class allocWithZone: NSDefaultMallocZone()]
	initWithObjects: k count: c];
      GS_ENDIDBUF();
      return AUTORELEASE(result);
    }
}

/**
 * Returns an array containing all the dictionary's objects.
 */
- (NSArray*) allValues
{
  unsigned	c = [self count];

  if (c == 0)
    {
      return [NSArray_class array];
    }
  else
    {
      NSEnumerator	*e = [self objectEnumerator];
      IMP		nxtObj = [e methodForSelector: nxtSel];
      id		result;
      unsigned		i;
      GS_BEGINIDBUF(k, c);

      for (i = 0; i < c; i++)
	{
	  k[i] = (*nxtObj)(e, nxtSel);
	}
      result = [[NSArray_class allocWithZone: NSDefaultMallocZone()]
	initWithObjects: k count: c];
      GS_ENDIDBUF();
      return AUTORELEASE(result);
    }
}

- (void) getObjects: (__unsafe_unretained id[])objects
            andKeys: (__unsafe_unretained id<NSCopying>[])keys
{
  NSUInteger i = 0;
  FOR_IN(id, key, self)
    if (keys != NULL) keys[i] = key;
    if (objects != NULL) objects[i] = [self objectForKey: key];
    i++;
  END_FOR_IN(self)
}

/**
 * Returns an array containing all the dictionary's keys that are
 * associated with anObject.
 */
- (NSArray*) allKeysForObject: (id)anObject
{
  unsigned	c;

  if (anObject == nil || (c = [self count]) == 0)
    {
      return nil;
    }
  else
    {
      NSEnumerator	*e = [self keyEnumerator];
      IMP		nxtObj = [e methodForSelector: nxtSel];
      IMP		myObj = [self methodForSelector: objSel];
      BOOL		(*eqObj)(id, SEL, id);
      id		k;
      id		result;
      GS_BEGINIDBUF(a, [self count]);

      eqObj = (BOOL (*)(id, SEL, id))[anObject methodForSelector: eqSel];
      c = 0;
      while ((k = (*nxtObj)(e, nxtSel)) != nil)
	{
	  id	o = (*myObj)(self, objSel, k);

	  if (o == anObject || (*eqObj)(anObject, eqSel, o))
	    {
	      a[c++] = k;
	    }
	}
      if (c == 0)
	{
	  result = nil;
	}
      else
	{
	  result = [[NSArray_class allocWithZone: NSDefaultMallocZone()]
	    initWithObjects: a count: c];
	}
      GS_ENDIDBUF();
      return AUTORELEASE(result);
    }
}

struct foo { NSDictionary *d; SEL s; IMP i; };

static NSInteger
compareIt(id o1, id o2, void* context)
{
  struct foo	*f = (struct foo*)context;
  o1 = (*f->i)(f->d, @selector(objectForKey:), o1);
  o2 = (*f->i)(f->d, @selector(objectForKey:), o2);
  return (NSInteger)(intptr_t)[o1 performSelector: f->s withObject: o2];
}

/**
 *  Returns ordered array of the keys sorted according to the values they
 *  correspond to.  To sort the values, a message with selector comp is
 *  send to each value with another value as argument, as in
 *  <code>[a comp: b]</code>.  The comp method should return
 *  <code>NSOrderedSame</code>, <code>NSOrderedAscending</code>, or
 *  <code>NSOrderedDescending</code> as appropriate.
 */
- (NSArray*) keysSortedByValueUsingSelector: (SEL)comp
{
  struct foo	info;
  id	k;

  info.d = self;
  info.s = comp;
  info.i = [self methodForSelector: objSel];
  k = [[self allKeys] sortedArrayUsingFunction: compareIt context: &info];
  return k;
}

- (NSArray *) keysSortedByValueUsingComparator: (NSComparator)cmptr
{
  return [self keysSortedByValueWithOptions: 0
			    usingComparator: cmptr];
}

- (NSArray *) keysSortedByValueWithOptions: (NSSortOptions)opts
			   usingComparator: (NSComparator)cmptr
{
  NSArray		*sortedValues;
  NSArray		*noDuplicates;
  NSMutableArray	*result;

  ENTER_POOL
  sortedValues = [[self allValues] sortedArrayWithOptions: opts
					  usingComparator: cmptr];
  noDuplicates = [[NSOrderedSet orderedSetWithArray: sortedValues] array];
  result = [[NSMutableArray alloc] initWithCapacity: [sortedValues count]];
  FOR_IN(NSObject*, value, noDuplicates)
    [result addObjectsFromArray: [self allKeysForObject: value]];
  END_FOR_IN(noDuplicates)
  LEAVE_POOL
  return AUTORELEASE(result);
}

/**
 *  Multiple version of [-objectForKey:].  Objects for each key in keys are
 *  looked up and placed into return array in same order.  For each key that
 *  has no corresponding value in this dictionary, marker is put into the
 *  array in its place.
 */
- (NSArray*) objectsForKeys: (NSArray*)keys notFoundMarker: (id)marker
{
  unsigned	c = [keys count];

  if (c == 0)
    {
      return [NSArray_class array];
    }
  else
    {
      unsigned	i;
      IMP	myObj = [self methodForSelector: objSel];
      id	result;
      GS_BEGINIDBUF(obuf, c);

      if ([keys isProxy])
	{
	  for (i = 0; i < c; i++)
	    {
	      obuf[i] = [keys objectAtIndex: i];
	    }
	}
      else
	{
          [keys getObjects: obuf];
	}
      for (i = 0; i < c; i++)
	{
	  id o = (*myObj)(self, objSel, obuf[i]);

	  if (o == nil)
	    {
	      obuf[i] = marker;
	    }
	  else
	    {
	      obuf[i] = o;
	    }
	}
      result = [[NSArray_class allocWithZone: NSDefaultMallocZone()]
	initWithObjects: obuf count: c];
      GS_ENDIDBUF();
      return AUTORELEASE(result);
    }
}

- (NSSet*) keysOfEntriesWithOptions: (NSEnumerationOptions)opts
                        passingTest: (GSKeysAndObjectsPredicateBlock)aPredicate
{
  /*
   * See -enumerateKeysAndObjectsWithOptions:usingBlock: for note about
   * NSEnumerationOptions.
   */
  id<NSFastEnumeration> enumerator = [self keyEnumerator];
  SEL objectForKeySelector = @selector(objectForKey:);
  IMP objectForKey = [self methodForSelector: objectForKeySelector];
  BLOCK_SCOPE BOOL shouldStop = NO;
  NSMutableSet *buildSet = [NSMutableSet new];
  SEL addObjectSelector = @selector(addObject:);
  IMP addObject = [buildSet methodForSelector: addObjectSelector];
  NSSet *resultSet = nil;
  id obj = nil;
  BLOCK_SCOPE NSLock *setLock = nil;

  if (opts & NSEnumerationConcurrent)
    {
      setLock = [NSLock new];
    }
  GS_DISPATCH_CREATE_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
  FOR_IN(id, key, enumerator)
    obj = (*objectForKey)(self, objectForKeySelector, key);
#if (__has_feature(blocks) && (GS_USE_LIBDISPATCH == 1))
    if (enumQueue != NULL)
      {
        dispatch_group_async(enumQueueGroup, enumQueue, ^(void){
          if (shouldStop)
            {
              return;
            }
          if (aPredicate(key, obj, &shouldStop))
            {
              [setLock lock];
              addObject(buildSet, addObjectSelector, key);
              [setLock unlock];
            }
        });
      }
    else // call block directly
#endif
    if (CALL_BLOCK(aPredicate, key, obj, &shouldStop))
      {
        addObject(buildSet, addObjectSelector, key);
      }
    if (shouldStop)
      {
        break;
      }
  END_FOR_IN(enumerator)
  GS_DISPATCH_TEARDOWN_QUEUE_AND_GROUP_FOR_ENUMERATION(enumQueue, opts)
  [setLock release];
  resultSet = [NSSet setWithSet: buildSet];
  [buildSet release];
  return resultSet;
}

- (NSSet*) keysOfEntriesPassingTest: (GSKeysAndObjectsPredicateBlock)aPredicate
{
  return [self keysOfEntriesWithOptions: 0
                            passingTest: aPredicate];
}

/**
 * <p>Writes the contents of the dictionary to the file specified by path.
 * The file contents will be in property-list format ... under GNUstep
 * this is either OpenStep style (ASCII characters using \U hexadecimal
 * escape sequences for unicode), or MacOS-X style (XML in the UTF8
 * character set).
 * </p>
 * <p>If the useAuxiliaryFile flag is YES, the file write operation is
 * atomic ... the data is written to a temporary file, which is then
 * renamed to the actual file name.
 * </p>
 * <p>If the conversion of data into the correct property-list format fails
 * or the write operation fails, the method returns NO, otherwise it
 * returns YES.
 * </p>
 * <p>NB. The fact that the file is in property-list format does not
 * necessarily mean that it can be used to reconstruct the dictionary using
 * the -initWithContentsOfFile: method.  If the original dictionary contains
 * non-property-list objects, the descriptions of those objects will
 * have been written, and reading in the file as a property-list will
 * result in a new dictionary containing the string descriptions.
 * </p>
 */
- (BOOL) writeToFile: (NSString *)path atomically: (BOOL)useAuxiliaryFile
{
  NSDictionary	*loc;
  NSString	*desc = nil;
  NSData	*data;

  loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
  if (GSMacOSXCompatiblePropertyLists() == YES)
    {
      GSPropertyListMake(self, loc, YES, NO, 2, &desc);
      data = [desc dataUsingEncoding: NSUTF8StringEncoding];
    }
  else
    {
      GSPropertyListMake(self, loc, NO, NO, 2, &desc);
      data = [desc dataUsingEncoding: NSASCIIStringEncoding];
    }
  return [data writeToFile: path atomically: useAuxiliaryFile];
}

/**
 * <p>Writes the contents of the dictionary to the specified url.
 * This functions just like -writeToFile:atomically: except that the
 * output may be written to any URL, not just a local file.
 * </p>
 */
- (BOOL) writeToURL: (NSURL *)url atomically: (BOOL)useAuxiliaryFile
{
  NSDictionary	*loc;
  NSString	*desc = nil;
  NSData	*data;

  loc = [[NSUserDefaults standardUserDefaults] dictionaryRepresentation];
  if (GSMacOSXCompatiblePropertyLists() == YES)
    {
      GSPropertyListMake(self, loc, YES, NO, 2, &desc);
      data = [desc dataUsingEncoding: NSUTF8StringEncoding];
    }
  else
    {
      GSPropertyListMake(self, loc, NO, NO, 2, &desc);
      data = [desc dataUsingEncoding: NSASCIIStringEncoding];
    }

  return [data writeToURL: url atomically: useAuxiliaryFile];
}

/**
 * Returns the result of invoking -descriptionWithLocale:indent: with a nil
 * locale and zero indent.
 */
- (NSString*) description
{
  return [self descriptionWithLocale: nil indent: 0];
}

/**
 * Returns the receiver as a text property list strings file format.<br />
 * See [NSString-propertyListFromStringsFileFormat] for details.<br />
 * The order of the items is undefined.
 */
- (NSString*) descriptionInStringsFileFormat
{
  NSMutableString	*result = nil;
  NSEnumerator		*enumerator = [self keyEnumerator];
  IMP			nxtObj = [enumerator methodForSelector: nxtSel];
  IMP			myObj = [self methodForSelector: objSel];
  id                    key;

  while ((key = (*nxtObj)(enumerator, nxtSel)) != nil)
    {
      id val = (*myObj)(self, objSel, key);

      GSPropertyListMake(key, nil, NO, YES, 0, &result);
      if (val != nil && [val isEqualToString: @""] == NO)
        {
	  [result appendString: @" = "];
	  GSPropertyListMake(val, nil, NO, YES, 0, &result);
        }
      [result appendString: @";\n"];
    }

  return result;
}

/**
 * Returns the result of invoking -descriptionWithLocale:indent: with
 * a zero indent.
 */
- (NSString*) descriptionWithLocale: (id)locale
{
  return [self descriptionWithLocale: locale indent: 0];
}

/**
 * Returns the receiver as a text property list in the traditional format.<br />
 * See [NSString-propertyList] for details.<br />
 * If locale is nil, no formatting is done, otherwise entries are formatted
 * according to the locale, and indented according to level.<br />
 * Unless locale is nil, a level of zero indents items by four spaces,
 * while a level of one indents them by a tab.<br />
 * If the keys in the dictionary respond to [NSObject-compare:], the items are
 * listed by key in ascending order.  If not, the order in which the
 * items are listed is undefined.
 */
- (NSString*) descriptionWithLocale: (id)locale
			     indent: (NSUInteger)level
{
  NSMutableString	*result = nil;

  GSPropertyListMake(self, locale, NO, YES, level == 1 ? 3 : 2, &result);
  return result;
}

/**
 * Default implementation for this class is to return the value stored in
 * the dictionary under the specified key, or nil if there is no value.<br />
 * However, if the key begins with '@' that character is stripped from
 * it and the superclass implementation of the method is used.
 */
- (id) valueForKey: (NSString*)key
{
  id	o;

  if ([key isKindOfClass: [NSString class]] && [key hasPrefix: @"@"])
    {
      o = [super valueForKey: [key substringFromIndex: 1]];
    }
  else
    {
      o = [self objectForKey: key];
    }
  return o;
}
- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state
                                   objects: (__unsafe_unretained id[])stackbuf
                                     count: (NSUInteger)len
{
  [self subclassResponsibility: _cmd];
  return 0;
}

@end


/**
 *  Mutable version of [NSDictionary].
 */
@implementation NSMutableDictionary

+ (void) initialize
{
  if (self == [NSMutableDictionary class])
    {
      NSMutableDictionaryClass = self;
      GSMutableDictionaryClass = [GSMutableDictionary class];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == NSMutableDictionaryClass)
    {
      return NSAllocateObject(GSMutableDictionaryClass, 0, z);
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

- (id) copyWithZone: (NSZone*)z
{
  /* a deep copy */
  unsigned	count = [self count];
  NSDictionary	*newDictionary;
  unsigned	i;
  id		key;
  NSEnumerator	*enumerator = [self keyEnumerator];
  IMP		nxtImp = [enumerator methodForSelector: nxtSel];
  IMP		objImp = [self methodForSelector: objSel];
  GS_BEGINIDBUF(o, count*2);

  for (i = 0; (key = (*nxtImp)(enumerator, nxtSel)); i++)
    {
      o[i] = key;
      o[count + i] = (*objImp)(self, objSel, key);
      o[count + i] = [o[count + i] copyWithZone: z];
    }
  newDictionary = [[GSDictionaryClass allocWithZone: z]
	  initWithObjects: o + count
		  forKeys: o
		    count: count];
  while (i-- > 0)
    {
      [o[count + i] release];
    }
  GS_ENDIDBUF();

  return newDictionary;
}

- (Class) classForCoder
{
  return NSMutableDictionaryClass;
}

/** <init /> <override-subclass />
 * Initializes an empty dictionary with memory preallocated for given number
 * of entries.  Although memory space will be grown as needed when entries
 * are added, this can avoid the reallocate-and-copy process if the size of
 * the ultimate contents is known in advance.<br />
 * Calls -init (which does nothing but maintain MacOS-X compatibility),
 * and needs to be re-implemented in subclasses in order to have all
 * other initialisers work.
 */
- (id) initWithCapacity: (NSUInteger)numItems
{
  self = [self init];
  return self;
}

/**
 *  Adds entry for aKey, mapping to anObject.  If either is nil, an exception
 *  is raised.  If aKey already in dictionary, the value it maps to is
 *  silently replaced.  The value anObject is retained, but aKey is copied
 *  (because a dictionary key must be immutable) and must therefore implement
 *  the [(NSCopying)] protocol.)
 */
- (void) setObject: anObject forKey: (id)aKey
{
  [self subclassResponsibility: _cmd];
}

- (void) setObject: (id)anObject forKeyedSubscript: (id)aKey
{
  if (anObject == nil)
    {
      [self removeObjectForKey: aKey];
    }
  else
    {
      [self setObject: anObject forKey: aKey]; 
    }
}

/**
 *  Remove key-value mapping for given key aKey.  No error if there is no
 *  mapping for the key.  A warning will be generated if aKey is nil.
 */
- (void) removeObjectForKey: (id)aKey
{
  [self subclassResponsibility: _cmd];
}

/**
 *  Returns an empty dictionary with memory preallocated for given number of
 *  entries.  Although memory space will be grown as needed when entries are
 *  added, this can avoid the reallocate-and-copy process if the size of the
 *  ultimate contents is known in advance.
 */
+ (id) dictionaryWithCapacity: (NSUInteger)numItems
{
  return AUTORELEASE([[self allocWithZone: NSDefaultMallocZone()]
    initWithCapacity: numItems]);
}

/* Override superclass's designated initializer */
/**
 * Initializes contents to the given objects and keys.
 * The two arrays must have the same size.
 * The n th element of the objects array is associated with the n th
 * element of the keys array.
 */
- (id) initWithObjects: (const id[])objects
	       forKeys: (const id <NSCopying>[])keys
		 count: (NSUInteger)count
{
  self = [self initWithCapacity: count];
  if (self != nil)
    {
      IMP	setObj;

      setObj = [self methodForSelector: setSel];
      while (count--)
	{
	  (*setObj)(self, setSel, objects[count], keys[count]);
	}
    }
  return self;
}

/**
 *  Clears out this dictionary by removing all entries.
 */
- (void) removeAllObjects
{
  id		k;
  NSEnumerator	*e = [self keyEnumerator];
  IMP		nxtObj = [e methodForSelector: nxtSel];
  IMP		remObj = [self methodForSelector: remSel];

  while ((k = (*nxtObj)(e, nxtSel)) != nil)
    {
      (*remObj)(self, remSel, k);
    }
}

/**
 *  Remove entries specified by the given keyArray.  No error is generated if
 *  no mapping exists for a key or one is nil, although a console warning is
 *  produced in the latter case.
 */
- (void) removeObjectsForKeys: (NSArray*)keyArray
{
  unsigned	c = [keyArray count];

  if (c > 0)
    {
      IMP	remObj = [self methodForSelector: remSel];
      GS_BEGINIDBUF(keys, c);

      if ([keyArray isProxy])
	{
	  unsigned	i;

	  for (i = 0; i < c; i++)
	    {
	      keys[i] = [keyArray objectAtIndex: i];
	    }
	}
      else
	{
	  [keyArray getObjects: keys];
	}
      while (c--)
	{
	  (*remObj)(self, remSel, keys[c]);
	}
      GS_ENDIDBUF();
    }
}

/**
 * Merges information from otherDictionary into the receiver.
 * If a key exists in both dictionaries, the value from otherDictionary
 * replaces that which was originally in the receiver.
 */
- (void) addEntriesFromDictionary: (NSDictionary*)otherDictionary
{
  if (otherDictionary != nil && otherDictionary != self)
    {
      id		k;
      NSEnumerator	*e = [otherDictionary keyEnumerator];
      IMP		nxtObj = [e methodForSelector: nxtSel];
      IMP		getObj = [otherDictionary methodForSelector: objSel];
      IMP		setObj = [self methodForSelector: setSel];

      while ((k = (*nxtObj)(e, nxtSel)) != nil)
	{
	  (*setObj)(self, setSel, (*getObj)(otherDictionary, objSel, k), k);
	}
    }
}

/**
 *  Remove all entries, then add all entries from otherDictionary.
 */
- (void) setDictionary: (NSDictionary*)otherDictionary
{
  [self removeAllObjects];
  [self addEntriesFromDictionary: otherDictionary];
}

/**
 * Default implementation for this class is equivalent to the
 * -setObject:forKey: method unless value is nil, in which case
 * it is equivalent to -removeObjectForKey:
 */
- (void) takeStoredValue: (id)value forKey: (NSString*)key
{
  if (value == nil)
    {
      [self removeObjectForKey: key];
    }
  else
    {
      [self setObject: value forKey: key];
    }
}

/**
 * Default implementation for this class is equivalent to the
 * -setObject:forKey: method unless value is nil, in which case
 * it is equivalent to -removeObjectForKey:
 */
- (void) takeValue: (id)value forKey: (NSString*)key
{
  if (value == nil)
    {
      [self removeObjectForKey: key];
    }
  else
    {
      [self setObject: value forKey: key];
    }
}

/**
 * Default implementation for this class is equivalent to the
 * -setObject:forKey: method unless value is nil, in which case
 * it is equivalent to -removeObjectForKey:
 */
- (void) setValue: (id)value forKey: (NSString*)key
{
  if (value == nil)
    {
      [self removeObjectForKey: key];
    }
  else
    {
      [self setObject: value forKey: key];
    }
}
@end
