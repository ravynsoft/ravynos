/** Implementation of NSMapTable for GNUStep
   Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: Feb 2009

   Based on original o_map code by Albin L. Jones <Albin.L.Jones@Dartmouth.EDU>

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

   $Date: 2008-06-08 11:38:33 +0100 (Sun, 08 Jun 2008) $ $Revision: 26606 $
   */

#import "common.h"

#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSMapTable.h"

#import "NSConcretePointerFunctions.h"
#import "NSCallBacks.h"

static Class	concreteClass = Nil;
static unsigned instanceSize = 0;


/* Here is the interface for the concrete class as used by the functions.
 */

typedef struct _GSIMapBucket GSIMapBucket_t;
typedef struct _GSIMapNode GSIMapNode_t;
typedef GSIMapBucket_t *GSIMapBucket;
typedef GSIMapNode_t *GSIMapNode;

@interface	NSConcreteMapTable : NSMapTable
{
@public
  NSZone	*zone;
  size_t	nodeCount;	/* Number of used nodes in map.	*/
  size_t	bucketCount;	/* Number of buckets in map.	*/
  GSIMapBucket	buckets;	/* Array of buckets.		*/
  GSIMapNode	freeNodes;	/* List of unused nodes.	*/
  GSIMapNode	*nodeChunks;	/* Chunks of allocated memory.	*/
  size_t	chunkCount;	/* Number of chunks in array.	*/
  size_t	increment;	/* Amount to grow by.		*/
  unsigned long	version;	/* For fast enumeration.	*/
  BOOL		legacy;		/* old style callbacks?		*/
  union {
    struct {
      PFInfo	k;
      PFInfo	v;
    } pf;
    struct {
      NSMapTableKeyCallBacks k;
      NSMapTableValueCallBacks v;
    } old;
  }cb;
}
@end

#define	GSI_MAP_TABLE_T	NSConcreteMapTable
#define	GSI_MAP_TABLE_S	instanceSize

#define	GSI_MAP_KTYPES	GSUNION_PTR | GSUNION_OBJ
#define	GSI_MAP_VTYPES	GSUNION_PTR | GSUNION_OBJ
#define IS_WEAK_KEY(M) \
  M->cb.pf.k.options & (NSPointerFunctionsZeroingWeakMemory | NSPointerFunctionsWeakMemory)
#define IS_WEAK_VALUE(M) \
  M->cb.pf.v.options & (NSPointerFunctionsZeroingWeakMemory | NSPointerFunctionsWeakMemory)
#define GSI_MAP_HASH(M, X)\
 (M->legacy ? M->cb.old.k.hash(M, X.ptr) \
 : pointerFunctionsHash(&M->cb.pf.k, X.ptr))
#define GSI_MAP_EQUAL(M, X, Y)\
 (M->legacy ? M->cb.old.k.isEqual(M, X.ptr, Y.ptr) \
 : pointerFunctionsEqual(&M->cb.pf.k, X.ptr, Y.ptr))
#define GSI_MAP_RELEASE_KEY(M, X)\
 (M->legacy ? M->cb.old.k.release(M, X.ptr) \
  : IS_WEAK_KEY(M) ? nil : pointerFunctionsRelinquish(&M->cb.pf.k, &X.ptr))
#define GSI_MAP_RETAIN_KEY(M, X)\
 (M->legacy ? M->cb.old.k.retain(M, X.ptr) \
  : IS_WEAK_KEY(M) ? nil : pointerFunctionsAcquire(&M->cb.pf.k, &X.ptr, X.ptr))
#define GSI_MAP_RELEASE_VAL(M, X)\
 (M->legacy ? M->cb.old.v.release(M, X.ptr) \
  : IS_WEAK_VALUE(M) ? nil : pointerFunctionsRelinquish(&M->cb.pf.v, &X.ptr))
#define GSI_MAP_RETAIN_VAL(M, X)\
 (M->legacy ? M->cb.old.v.retain(M, X.ptr) \
  : IS_WEAK_VALUE(M) ? nil : pointerFunctionsAcquire(&M->cb.pf.v, &X.ptr, X.ptr))

/* 2013-05-25 Here are the macros originally added for GC/ARC ...
 * but they caused map table entries to be doubly retained :-(
 * The question is, are the new versions I hacked in below to
 * fix this correct?
 
#define GSI_MAP_WRITE_KEY(M, addr, x) \
	if (M->legacy) \
	  *(addr) = x;\
	else\
	  pointerFunctionsAssign(&M->cb.pf.k, (void**)addr, (x).obj);
#define GSI_MAP_WRITE_VAL(M, addr, x) \
	if (M->legacy) \
	  *(addr) = x;\
	else\
	  pointerFunctionsAssign(&M->cb.pf.v, (void**)addr, (x).obj);
*/
#define GSI_MAP_WRITE_KEY(M, addr, x) \
	if (M->legacy) \
          *(addr) = x;\
	else\
	  (IS_WEAK_KEY(M) ? pointerFunctionsAssign(&M->cb.pf.k, (void**)addr, (x).obj) : (*(id*)(addr) = (x).obj));
#define GSI_MAP_WRITE_VAL(M, addr, x) \
	if (M->legacy) \
          *(addr) = x;\
	else\
	  (IS_WEAK_VALUE(M) ? pointerFunctionsAssign(&M->cb.pf.v, (void**)addr, (x).obj) : (*(id*)(addr) = (x).obj));
#define GSI_MAP_READ_KEY(M,addr) \
	(M->legacy ? *(addr)\
	  : (__typeof__(*addr))pointerFunctionsRead(&M->cb.pf.k, (void**)addr))
#define GSI_MAP_READ_VALUE(M,addr) \
	(M->legacy ? *(addr)\
	  : (__typeof__(*addr))pointerFunctionsRead(&M->cb.pf.v, (void**)addr))
#define GSI_MAP_ZEROED(M)\
        (M->legacy ? 0\
	  : (IS_WEAK_KEY(M) || IS_WEAK_VALUE(M)) ? YES : NO)

#define	GSI_MAP_ENUMERATOR	NSMapEnumerator

#include "GNUstepBase/GSIMap.h"

/**** Function Implementations ****/

/**
 * Returns an array of all the keys in the table.
 * NB. The table <em>must</em> contain objects for its keys.
 */
NSArray *
NSAllMapTableKeys(NSMapTable *table)
{
  NSMutableArray	*keyArray;
  NSMapEnumerator	enumerator;
  id			key = nil;
  void			*dummy;

  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return nil;
    }

  /* Create our mutable key array. */
  keyArray = [NSMutableArray arrayWithCapacity: NSCountMapTable(table)];

  /* Get an enumerator for TABLE. */
  enumerator = NSEnumerateMapTable(table);

  /* Step through TABLE... */
  while (NSNextMapEnumeratorPair(&enumerator, (void **)(&key), &dummy))
    {
      [keyArray addObject: key];
    }
  NSEndMapTableEnumeration(&enumerator);
  return keyArray;
}

/**
 * Returns an array of all the values in the table.
 * NB. The table <em>must</em> contain objects for its values.
 */
NSArray *
NSAllMapTableValues(NSMapTable *table)
{
  NSMapEnumerator	enumerator;
  NSMutableArray	*valueArray;
  id			value = nil;
  void			*dummy;

  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return nil;
    }

  /* Create our mutable value array. */
  valueArray = [NSMutableArray arrayWithCapacity: NSCountMapTable(table)];

  /* Get an enumerator for TABLE. */
  enumerator = NSEnumerateMapTable(table);

  /* Step through TABLE... */
  while (NSNextMapEnumeratorPair(&enumerator, &dummy, (void **)(&value)))
    {
      [valueArray addObject: value];
    }
  NSEndMapTableEnumeration(&enumerator);
  return valueArray;
}

static BOOL
equalPointers(const void *item1, const void *item2,
  NSUInteger (*size)(const void *item))
{
  return (item1 == item2) ? YES : NO;
}

/**
 * Compares the two map tables for equality.
 * If the tables are different sizes, returns NO.
 * Otherwise, compares the keys <em>(not the values)</em>
 * in the two map tables and returns NO if they differ.<br />
 * The GNUstep implementation enumerates the keys in table1
 * and uses the hash and isEqual functions of table2 for comparison.
 */
BOOL
NSCompareMapTables(NSMapTable *table1, NSMapTable *table2)
{
  if (table1 == table2)
    {
      return YES;
    }
  if (table1 == nil)
    {
      NSWarnFLog(@"Null first argument supplied");
      return NO;
    }
  if (table2 == nil)
    {
      NSWarnFLog(@"Null second argument supplied");
      return NO;
    }

  if ([table1 count] != [table2 count])
    {
      return NO;
    }

  if (object_getClass(table1) != concreteClass
    && object_getClass(table2) == concreteClass)
    {
      id	t = table1;

      table1 = table2;
      table2 = t;
    }

  if (object_getClass(table1) == concreteClass)
    {
      NSConcreteMapTable	*c1 = (NSConcreteMapTable*)table1;
      GSIMapTable	t1 = (GSIMapTable)table1;
      BOOL		result = YES;
      NSMapEnumerator	enumerator;
      GSIMapNode	n1;

      enumerator = GSIMapEnumeratorForMap(t1);
      if (object_getClass(table2) == concreteClass)
	{
	  GSIMapTable	t2 = (GSIMapTable)table2;
    
	  while ((n1 = GSIMapEnumeratorNextNode(&enumerator)) != 0)
	    {
	      GSIMapNode	n2;

	      n2 = GSIMapNodeForKey(t2, n1->key);
	      if (n2 == 0)
		{
		  result = NO;
		}
	      else
		{
		  void		*v1 = n1->value.ptr;
		  void		*v2 = n2->value.ptr;

		  result = (c1->legacy
		    ? c1->cb.old.k.isEqual(c1, v1, v2)
		    : pointerFunctionsEqual(&c1->cb.pf.v, v2, v2));
		}
	      if (result == NO)
		{
		  break;
		}
	    }
	}
      else
	{
	  while ((n1 = GSIMapEnumeratorNextNode(&enumerator)) != 0)
	    {
	      void	*k1 = n1->key.ptr;
	      void	*v1 = n1->value.ptr;
	      void	*v2 = NSMapGet(table2, k1);

	      result = (c1->legacy
		? c1->cb.old.k.isEqual(c1, v1, v2)
		: pointerFunctionsEqual(&c1->cb.pf.v, v1, v2));
	      if (result == NO)
		{
		  break;
		}
	    }
	}
      GSIMapEndEnumerator((GSIMapEnumerator)&enumerator);
      return result;
    }
  else
    {
      BOOL		result = YES;
      NSMapEnumerator	enumerator;
      void		*k1;
      void		*v1;
      NSPointerFunctions	*pf;
      BOOL (*isEqualFunction)(const void *item1, const void *item2,
        NSUInteger (*size)(const void *item));
      NSUInteger (*sizeFunction)(const void *item);

      /* Get functions needed for comparison.
       */
      pf = [table1 valuePointerFunctions];
      isEqualFunction = [pf isEqualFunction];
      sizeFunction = [pf sizeFunction];
      if (isEqualFunction == 0) isEqualFunction = equalPointers;

      enumerator = NSEnumerateMapTable(table1);
      while (NSNextMapEnumeratorPair(&enumerator, &k1, &v1) == YES)
	{
	  void	*v2 = NSMapGet(table2, k1);

	  if ((*isEqualFunction)(v1, v2, sizeFunction) == NO)
	    {
	      result = NO;
	      break;
	    }
	}
      NSEndMapTableEnumeration(&enumerator);
      return result;
    }
}

/**
 * Copy the supplied map table.<br />
 * Returns a map table, space for which is allocated in zone, which
 * has (newly retained) copies of table's keys and values.  As always,
 * if zone is 0, then NSDefaultMallocZone() is used.
 */
NSMapTable *
NSCopyMapTableWithZone(NSMapTable *table, NSZone *zone)
{
  GSIMapTable	o = (GSIMapTable)table;
  GSIMapTable	t;
  GSIMapNode	n;

  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return 0;
    }
  t = (GSIMapTable)[concreteClass allocWithZone: zone];
  t->legacy = o->legacy;
  if (t->legacy == YES)
    {
      t->cb.old.k = o->cb.old.k;
      t->cb.old.v = o->cb.old.v;
    }
  else
    {
      t->cb.pf.k = o->cb.pf.k;
      t->cb.pf.v = o->cb.pf.v;
    }
  GSIMapInitWithZoneAndCapacity(t, zone, ((GSIMapTable)table)->nodeCount);

  if (object_getClass(table) == concreteClass)
    {
      NSMapEnumerator	enumerator;

      enumerator = GSIMapEnumeratorForMap((GSIMapTable)table);
      while ((n = GSIMapEnumeratorNextNode(&enumerator)) != 0)
	{
	  GSIMapAddPair(t, n->key, n->value);
	}
      GSIMapEndEnumerator((GSIMapEnumerator)&enumerator);
    }
  else
    {
      NSEnumerator	*enumerator;
      id		k;

      enumerator = [table keyEnumerator];
      while ((k = [enumerator nextObject]) != nil)
	{
	  GSIMapAddPair(t, (GSIMapKey)k, (GSIMapVal)[table objectForKey: k]);
	}
    }

  return (NSMapTable*)t;
}

/**
 * Returns the number of key/value pairs in the table.
 */
NSUInteger
NSCountMapTable(NSMapTable *table)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return 0;
    }
  if (object_getClass(table) == concreteClass)
    {
      return ((GSIMapTable)table)->nodeCount;
    }
  return [table count];
}

/**
 * Create a new map table by calling NSCreateMapTableWithZone() using
 * NSDefaultMallocZone().<br />
 * Returns a (pointer to) an NSMapTable space for which is allocated
 * in the default zone.  If capacity is small or 0, then the returned
 * table has a reasonable capacity.
 */
NSMapTable *
NSCreateMapTable(
  NSMapTableKeyCallBacks keyCallBacks,
  NSMapTableValueCallBacks valueCallBacks,
  NSUInteger capacity)
{
  return NSCreateMapTableWithZone(keyCallBacks, valueCallBacks,
    capacity, NSDefaultMallocZone());
}

/**
 * Create a new map table using the supplied callbacks structures.
 * If any functions in the callback structures are null the default
 * values are used ... as for non-owned pointers.<br />
 * Of course, if you send 0 for zone, then the map table will be
 * created in NSDefaultMallocZone().<br />
 * The table will be created with the specified capacity ... ie ready
 * to hold at least that many items.
 */
NSMapTable *
NSCreateMapTableWithZone(
  NSMapTableKeyCallBacks k,
  NSMapTableValueCallBacks v,
  NSUInteger capacity,
  NSZone *zone)
{
  GSIMapTable	table;

  if (concreteClass == Nil)
    {
      [NSConcreteMapTable class];	// Force +initialize
      NSCAssert(concreteClass != Nil, NSInternalInconsistencyException);
    }
  table = (GSIMapTable)[concreteClass allocWithZone: zone];

  if (k.hash == 0)
    k.hash = NSNonOwnedPointerMapKeyCallBacks.hash;
  if (k.isEqual == 0)
    k.isEqual = NSNonOwnedPointerMapKeyCallBacks.isEqual;
  if (k.retain == 0)
    k.retain = NSNonOwnedPointerMapKeyCallBacks.retain;
  if (k.release == 0)
    k.release = NSNonOwnedPointerMapKeyCallBacks.release;
  if (k.describe == 0)
    k.describe = NSNonOwnedPointerMapKeyCallBacks.describe;

  if (v.retain == 0)
    v.retain = NSNonOwnedPointerMapValueCallBacks.retain;
  if (v.release == 0)
    v.release = NSNonOwnedPointerMapValueCallBacks.release;
  if (v.describe == 0)
    v.describe = NSNonOwnedPointerMapValueCallBacks.describe;

  table->legacy = YES;
  table->cb.old.k = k;
  table->cb.old.v = v;

  GSIMapInitWithZoneAndCapacity(table, zone, capacity);

  return (NSMapTable*)table;
}

/**
 * Function to be called when finished with the enumerator.
 * This permits memory used by the enumerator to be released!
 */
void
NSEndMapTableEnumeration(NSMapEnumerator *enumerator)
{
  if (enumerator == 0)
    {
      NSWarnFLog(@"Null enumerator argument supplied");
      return;
    }
  if (enumerator->map != 0)
    {
      /* The 'map' field is non-null, so this NSMapEnumerator is actually
       * a GSIMapEnumerator.
       */
      GSIMapEndEnumerator((GSIMapEnumerator)enumerator);
    }
  else if (enumerator->node != 0)
    {
      /* The 'map' field is null but the 'node' field is not, so the
       * NSMapEnumerator structure actually contains an NSEnumerator
       * in the 'node' field, and the map table being enumerated in the
       * 'bucket' field.
       */
      [(id)enumerator->node release];
      memset(enumerator, '\0', sizeof(NSMapEnumerator));
    }
}

/**
 * Return an enumerator for stepping through a map table using the
 * NSNextMapEnumeratorPair() function.
 */
NSMapEnumerator
NSEnumerateMapTable(NSMapTable *table)
{
  if (table == nil)
    {
      NSMapEnumerator	v = {0, 0, 0};

      NSWarnFLog(@"Null table argument supplied");
      return v;
    }
  if (object_getClass(table) == concreteClass)
    {
      return GSIMapEnumeratorForMap((GSIMapTable)table);
    }
  else
    {
      NSMapEnumerator	v = {0, 0, 0};
      NSEnumerator	*e = [[table keyEnumerator] retain];

      v.node = (void*)e;
      GS_CONSUMED(e)
      v.bucket = (unsigned long)(uintptr_t)table;
      return v;
    }
}

/**
 * Destroy the map table and release its contents.<br />
 * Releases all the keys and values of table (using the key and
 * value callbacks specified at the time of table's creation),
 * and then proceeds to deallocate the space allocated for table itself.
 */
void
NSFreeMapTable(NSMapTable *table)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
    }
  else
    {
      [table release];
    }
}

/**
 * Returns the value for the specified key, or a null pointer if the
 * key is not found in the table.
 */
void *
NSMapGet(NSMapTable *table, const void *key)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return 0;
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapNode	n;

      n = GSIMapNodeForKey((GSIMapTable)table, (GSIMapKey)key);
      if (n == 0)
	{
	  return 0;
	}
      else
	{
	  return n->value.ptr;
	}
    }
  else
    {
      return [table objectForKey: (id)key];
    }
}

/**
 * Adds the key and value to table.<br />
 * If an equal key is already in table, replaces its mapped value
 * with the new one, without changing the key itself.<br />
 * If key is equal to the notAKeyMarker field of the table's
 * NSMapTableKeyCallBacks, raises an NSInvalidArgumentException.
 */
void
NSMapInsert(NSMapTable *table, const void *key, const void *value)
{
  if (table == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Attempt to place key-value in null table"];
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapTable	t = (GSIMapTable)table;
      GSIMapNode	n;

      if (t->legacy == YES)
	{
	  if (key == t->cb.old.k.notAKeyMarker)
	    {
	      [NSException raise: NSInvalidArgumentException
		          format: @"Attempt to place notAKeyMarker in map"];
	    }
	}
      else if (key == 0)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Attempt to place nil key in map"];
	}
      n = GSIMapNodeForKey(t, (GSIMapKey)key);
      if (n == 0)
	{
	  GSIMapAddPair(t, (GSIMapKey)key, (GSIMapVal)value);
	  t->version++;
	}
      else if (n->value.ptr != value)
	{
	  GSIMapVal	tmp = n->value;

	  n->value = (GSIMapVal)value;
	  GSI_MAP_RETAIN_VAL(t, n->value);
	  GSI_MAP_RELEASE_VAL(t, tmp);
	  t->version++;
	}
    }
  else
    {
      [table setObject: (id)value forKey: (id)key];
    }
}

/**
 * Adds the key and value to table and returns nul.<br />
 * If an equal key is already in table, returns the old key
 * instead of adding the new key-value pair.<br />
 * If key is equal to the notAKeyMarker field of the table's
 * NSMapTableKeyCallBacks, raises an NSInvalidArgumentException.
 */
void *
NSMapInsertIfAbsent(NSMapTable *table, const void *key, const void *value)
{
  if (table == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Attempt to place key-value in null table"];
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapTable	t = (GSIMapTable)table;
      GSIMapNode	n;

      if (t->legacy == YES)
	{
	  if (key == t->cb.old.k.notAKeyMarker)
	    {
	      [NSException raise: NSInvalidArgumentException
		format: @"Attempt to place notAKeyMarker in map table"];
	    }
	}
      else if (key == 0)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Attempt to place nil key in map"];
	}
      n = GSIMapNodeForKey(t, (GSIMapKey)key);
      if (n == 0)
	{
	  GSIMapAddPair(t, (GSIMapKey)key, (GSIMapVal)value);
	  t->version++;
	  return 0;
	}
      else
	{
	  return n->key.ptr;
	}
    }
  else
    {
      void	*v = (void*)[table objectForKey: (id)key];

      if (v == 0)
	{
	  [table setObject: (id)value forKey: (id)v];
	  return 0;
	}
      return v;
    }
}

/**
 * Adds the key and value to table and returns nul.<br />
 * If an equal key is already in table, raises an NSInvalidArgumentException.
 * <br />If key is equal to the notAKeyMarker field of the table's
 * NSMapTableKeyCallBacks, raises an NSInvalidArgumentException.
 */
void
NSMapInsertKnownAbsent(NSMapTable *table, const void *key, const void *value)
{
  if (table == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Attempt to place key-value in null table"];
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapTable	t = (GSIMapTable)table;
      GSIMapNode	n;

      if (t->legacy == YES)
	{
	  if (key == t->cb.old.k.notAKeyMarker)
	    {
	      [NSException raise: NSInvalidArgumentException
		format: @"Attempt to place notAKeyMarker in map table"];
	    }
	}
      else if (key == 0)
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"Attempt to place nil key in map"];
	}
      n = GSIMapNodeForKey(t, (GSIMapKey)key);
      if (n == 0)
	{
	  GSIMapAddPair(t, (GSIMapKey)key, (GSIMapVal)value);
	  t->version++;
	}
      else
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"NSMapInsertKnownAbsent ... key not absent"];
	}
    }
  else
    {
      void	*v = (void*)[table objectForKey: (id)key];

      if (v == 0)
	{
	  [table setObject: (id)value forKey: (id)v];
	}
      else
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"NSMapInsertKnownAbsent ... key not absent"];
	}
    }
}

/**
 * Returns a flag to say whether the table contains the specified key.
 * Returns the original key and the value it maps to.<br />
 * The GNUstep implementation checks originalKey and value to see if
 * they are null pointers, and only updates them if non-null.
 */
BOOL
NSMapMember(NSMapTable *table, const void *key,
  void **originalKey, void **value)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return NO;
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapNode	n;

      n = GSIMapNodeForKey((GSIMapTable)table, (GSIMapKey)key);
      if (n == 0)
	{
	  return NO;
	}
      else
	{
	  if (originalKey != 0)
	    {
	      *originalKey = n->key.ptr;
	    }
	  if (value != 0)
	    {
	      *value = n->value.ptr;
	    }
	  return YES;
	}
    }
  else
    {
      return [table objectForKey: (id)key] ? YES : NO;
    }
}

/**
 * Remove the specified key from the table (if present).<br />
 * Causes the key and its associated value to be released.
 */
void
NSMapRemove(NSMapTable *table, const void *key)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return;
    }
  if (object_getClass(table) == concreteClass)
    {
      if (((GSIMapTable)table)->nodeCount > 0)
	{
	  GSIMapRemoveKey((GSIMapTable)table, (GSIMapKey)key);
	  ((GSIMapTable)table)->version++;
	}
    }
  else
    {
      [table removeObjectForKey: (id)key];
    }
}

/**
 * Step through the map table ... return the next key-value pair and
 * return YES, or hit the end of the table and return NO.<br />
 * The enumerator parameter is a value supplied by NSEnumerateMapTable()
 * and must be destroyed using NSEndMapTableEnumeration().<br />
 * The GNUstep implementation permits either key or value to be a
 * null pointer, and refrains from attempting to return the appropriate
 * result in that case.
 */
BOOL
NSNextMapEnumeratorPair(NSMapEnumerator *enumerator,
			void **key, void **value)
{
  if (enumerator == 0)
    {
      NSWarnFLog(@"Null enumerator argument supplied");
      return NO;
    }
  if (enumerator->map != 0)
    {
      GSIMapNode	n;

      /* The 'map' field is non-null, so this NSMapEnumerator is actually
       * a GSIMapEnumerator and we can use the GSIMap... functions to work
       * with it.
       */
      n = GSIMapEnumeratorNextNode((GSIMapEnumerator)enumerator);
      if (n == 0)
	{
	  return NO;
	}
      else
	{
          NSConcreteMapTable *map = enumerator->map;

          if (key != 0)
            {
	      *key = GSI_MAP_READ_KEY(map, &n->key).ptr;
	    }
	  else
	    {
	      NSWarnFLog(@"Null key return address");
            }

	  if (value != 0)
	    {
	      *value = GSI_MAP_READ_VALUE(map, &n->value).ptr;
	    }
	  else
	    {
	      NSWarnFLog(@"Null value return address");
	    }
	  return YES;
	}
    }
  else if (enumerator->node != 0)
    {
      id	k;

      /* The 'map' field is null but the 'node' field is not, so the
       * NSMapEnumerator structure actually contains an NSEnumerator
       * in the 'node' field, and the map table being enumerated in the
       * 'bucket' field.
       */
      k = [(NSEnumerator*)enumerator->node nextObject];
      if (k == nil)
	{
	  return NO;
	}
      if (key != 0)
	{
	  *key = k;
	}
      else
	{
	  NSWarnFLog(@"Null key return address");
	}
      if (value != 0)
	{
	  *value = [(NSMapTable*)enumerator->bucket objectForKey: k];
	}
      else
	{
	  NSWarnFLog(@"Null value return address");
	}
      return YES;
    }
  else
    {
      return NO;
    }
}

/**
 * Empty the map table (releasing every key and value),
 * but preserve its capacity.
 */
void
NSResetMapTable(NSMapTable *table)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return;
    }
  if (object_getClass(table) == concreteClass)
    {
      if (((GSIMapTable)table)->nodeCount > 0)
	{
	  GSIMapCleanMap((GSIMapTable)table);
	  ((GSIMapTable)table)->version++;
	}
    }
  else
    {
      [table removeAllObjects];
    }
}

/**
 * Returns a string describing the table contents.<br />
 * For each key-value pair, a string of the form "key = value;\n"
 * is appended.  The appropriate describe functions are used to generate
 * the strings for each key and value.
 */
NSString *
NSStringFromMapTable(NSMapTable *table)
{
  if (table == nil)
    {
      NSWarnFLog(@"Null table argument supplied");
      return nil;
    }
  if (object_getClass(table) == concreteClass)
    {
      GSIMapTable	t = (GSIMapTable)table;
      NSMutableString	*string;
      NSMapEnumerator	enumerator;
      void		*key;
      void		*value;

      string = [NSMutableString stringWithCapacity: 0];
      enumerator = NSEnumerateMapTable(table);

      /*
       * Now, just step through the elements of the table, and add their
       * descriptions to the string.
       */
      if (t->legacy)
	{
	  while (NSNextMapEnumeratorPair(&enumerator, &key, &value) == YES)
	    {
	      [string appendFormat: @"%@ = %@;\n",
		(t->cb.old.k.describe)(table, key),
		(t->cb.old.v.describe)(table, value)];
	    }
	}
      else
	{
	  while (NSNextMapEnumeratorPair(&enumerator, &key, &value) == YES)
	    {
	      [string appendFormat: @"%@ = %@;\n",
		(t->cb.pf.k.descriptionFunction)(key),
		(t->cb.pf.v.descriptionFunction)(value)];
	    }
	}
      NSEndMapTableEnumeration(&enumerator);
      return string;
    }
  else
    {
      return [table description];
    }
}




/* These are to increase readabilty locally. */
typedef NSUInteger (*NSMT_hash_func_t)(NSMapTable *, const void *);
typedef BOOL (*NSMT_is_equal_func_t)(NSMapTable *, const void *, const void *);
typedef void (*NSMT_retain_func_t)(NSMapTable *, const void *);
typedef void (*NSMT_release_func_t)(NSMapTable *, void *);
typedef NSString *(*NSMT_describe_func_t)(NSMapTable *, const void *);


/** For keys that are pointer-sized or smaller quantities. */
const NSMapTableKeyCallBacks NSIntegerMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_int_hash,
  (NSMT_is_equal_func_t) _NS_int_is_equal,
  (NSMT_retain_func_t) _NS_int_retain,
  (NSMT_release_func_t) _NS_int_release,
  (NSMT_describe_func_t) _NS_int_describe,
  NSNotAnIntMapKey
};

/** For backward compatibility. */
const NSMapTableKeyCallBacks NSIntMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_int_hash,
  (NSMT_is_equal_func_t) _NS_int_is_equal,
  (NSMT_retain_func_t) _NS_int_retain,
  (NSMT_release_func_t) _NS_int_release,
  (NSMT_describe_func_t) _NS_int_describe,
  NSNotAnIntMapKey
};

/** For keys that are pointers not freed. */
const NSMapTableKeyCallBacks NSNonOwnedPointerMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_non_owned_void_p_hash,
  (NSMT_is_equal_func_t) _NS_non_owned_void_p_is_equal,
  (NSMT_retain_func_t) _NS_non_owned_void_p_retain,
  (NSMT_release_func_t) _NS_non_owned_void_p_release,
  (NSMT_describe_func_t) _NS_non_owned_void_p_describe,
  NSNotAPointerMapKey
};

/** For keys that are pointers not freed, or 0. */
const NSMapTableKeyCallBacks NSNonOwnedPointerOrNullMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_non_owned_void_p_hash,
  (NSMT_is_equal_func_t) _NS_non_owned_void_p_is_equal,
  (NSMT_retain_func_t) _NS_non_owned_void_p_retain,
  (NSMT_release_func_t) _NS_non_owned_void_p_release,
  (NSMT_describe_func_t) _NS_non_owned_void_p_describe,
  NSNotAPointerMapKey
};

/** For sets of objects without retaining and releasing. */
const NSMapTableKeyCallBacks NSNonRetainedObjectMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_non_retained_id_hash,
  (NSMT_is_equal_func_t) _NS_non_retained_id_is_equal,
  (NSMT_retain_func_t) _NS_non_retained_id_retain,
  (NSMT_release_func_t) _NS_non_retained_id_release,
  (NSMT_describe_func_t) _NS_non_retained_id_describe,
  NSNotAPointerMapKey
};

/** For keys that are objects. */
const NSMapTableKeyCallBacks NSObjectMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_id_hash,
  (NSMT_is_equal_func_t) _NS_id_is_equal,
  (NSMT_retain_func_t) _NS_id_retain,
  (NSMT_release_func_t) _NS_id_release,
  (NSMT_describe_func_t) _NS_id_describe,
  NSNotAPointerMapKey
};

/** For keys that are pointers with transfer of ownership upon insertion. */
const NSMapTableKeyCallBacks NSOwnedPointerMapKeyCallBacks =
{
  (NSMT_hash_func_t) _NS_owned_void_p_hash,
  (NSMT_is_equal_func_t) _NS_owned_void_p_is_equal,
  (NSMT_retain_func_t) _NS_owned_void_p_retain,
  (NSMT_release_func_t) _NS_owned_void_p_release,
  (NSMT_describe_func_t) _NS_owned_void_p_describe,
  NSNotAPointerMapKey
};

/** For values that are pointer-sized integer quantities. */
const NSMapTableValueCallBacks NSIntegerMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_int_retain,
  (NSMT_release_func_t) _NS_int_release,
  (NSMT_describe_func_t) _NS_int_describe
};

/** For backward compatibilty. */
const NSMapTableValueCallBacks NSIntMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_int_retain,
  (NSMT_release_func_t) _NS_int_release,
  (NSMT_describe_func_t) _NS_int_describe
};

/** For values that are pointers not freed. */
const NSMapTableValueCallBacks NSNonOwnedPointerMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_non_owned_void_p_retain,
  (NSMT_release_func_t) _NS_non_owned_void_p_release,
  (NSMT_describe_func_t) _NS_non_owned_void_p_describe
};

/** For sets of objects without retaining and releasing. */
const NSMapTableValueCallBacks NSNonRetainedObjectMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_non_retained_id_retain,
  (NSMT_release_func_t) _NS_non_retained_id_release,
  (NSMT_describe_func_t) _NS_non_retained_id_describe
};

/** For values that are objects. */
const NSMapTableValueCallBacks NSObjectMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_id_retain,
  (NSMT_release_func_t) _NS_id_release,
  (NSMT_describe_func_t) _NS_id_describe
};

/** For values that are pointers with transfer of ownership upon insertion. */
const NSMapTableValueCallBacks NSOwnedPointerMapValueCallBacks =
{
  (NSMT_retain_func_t) _NS_owned_void_p_retain,
  (NSMT_release_func_t) _NS_owned_void_p_release,
  (NSMT_describe_func_t) _NS_owned_void_p_describe
};



@interface NSConcreteMapTableKeyEnumerator : NSEnumerator
{
  NSConcreteMapTable		*table;
  GSIMapEnumerator_t		enumerator;
}
- (id) initWithMapTable: (NSConcreteMapTable*)m;
@end

@interface NSConcreteMapTableObjectEnumerator : NSConcreteMapTableKeyEnumerator
@end

@implementation	NSConcreteMapTable

- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  /* Can't safely calculate for mutable object; just buffer size
   */
  return nodeCount * sizeof(GSIMapNode);
}

+ (void) initialize
{
  if (concreteClass == Nil)
    {
      concreteClass = [NSConcreteMapTable class];
      instanceSize = class_getInstanceSize(concreteClass);
    }
}

- (id) copyWithZone: (NSZone*)aZone
{
  return NSCopyMapTableWithZone(self, aZone);
}

- (NSUInteger) count
{
  GSIMapRemoveWeak(self);
  return (NSUInteger)nodeCount;
}

- (NSUInteger) countByEnumeratingWithState: (NSFastEnumerationState*)state 	
				   objects: (id*)stackbuf
				     count: (NSUInteger)len
{
  state->mutationsPtr = &version;
  return GSIMapCountByEnumeratingWithStateObjectsCount
    (self, state, stackbuf, len);
}

- (void) dealloc
{
  GSIMapEmptyMap(self);
  [super dealloc];
}

- (void) encodeWithCoder: (NSCoder*)aCoder
{
  [self subclassResponsibility: _cmd];
}

- (void) finalize
{
  GSIMapEmptyMap(self);
}

- (NSUInteger) hash
{
  return (NSUInteger)nodeCount;
}

- (id) init
{
  return [self initWithKeyPointerFunctions: nil
		     valuePointerFunctions: nil
				  capacity: 0];
}

- (id) initWithCoder: (NSCoder*)aCoder
{
  [self subclassResponsibility: _cmd];
  return nil;
}

- (id) initWithKeyPointerFunctions: (NSPointerFunctions*)keyFunctions
	     valuePointerFunctions: (NSPointerFunctions*)valueFunctions
			  capacity: (NSUInteger)initialCapacity
{
  static NSConcretePointerFunctions	*defaultFunctions = nil;

  if (defaultFunctions == nil)
    {
      defaultFunctions
	= [[NSConcretePointerFunctions alloc] initWithOptions: 0];
    }
  legacy = NO;

  if (![keyFunctions isKindOfClass: [NSConcretePointerFunctions class]])
    {
      keyFunctions = defaultFunctions;
    }
  memcpy(&self->cb.pf.k, &((NSConcretePointerFunctions*)keyFunctions)->_x,
    sizeof(self->cb.pf.k));

  if (![valueFunctions isKindOfClass: [NSConcretePointerFunctions class]])
    {
      valueFunctions = defaultFunctions;
    }
  memcpy(&self->cb.pf.v, &((NSConcretePointerFunctions*)valueFunctions)->_x,
    sizeof(self->cb.pf.v));

#if	GC_WITH_GC
  if (self->cb.pf.k.usesWeakReadAndWriteBarriers)
    {
      if (self->cb.pf.v.usesWeakReadAndWriteBarriers)
	{
	  zone = (NSZone*)nodeWW;
	}
      else
	{
	  zone = (NSZone*)nodeWS;
	}
    }
  else
    {
      if (self->cb.pf.v.usesWeakReadAndWriteBarriers)
	{
	  zone = (NSZone*)nodeSW;
	}
      else
	{
	  zone = (NSZone*)nodeSS;
	}
    }
#endif
  GSIMapInitWithZoneAndCapacity(self, zone, initialCapacity);
  return self;
}

- (BOOL) isEqual: (id)other
{
  return NSCompareMapTables(self, other);
}

- (NSEnumerator*) keyEnumerator
{
  NSEnumerator	*e;

  e = [[NSConcreteMapTableKeyEnumerator alloc] initWithMapTable: self];
  return [e autorelease];
}

- (NSPointerFunctions*) keyPointerFunctions
{
  NSConcretePointerFunctions	*p = [NSConcretePointerFunctions new];

  p->_x = self->cb.pf.k;
  return [p autorelease];
}

- (NSEnumerator*) objectEnumerator
{
  NSEnumerator	*e;

  e = [[NSConcreteMapTableObjectEnumerator alloc] initWithMapTable: self];
  return [e autorelease];
}

- (id) objectForKey: (id)aKey
{
  if (aKey != nil)
    {
      GSIMapNode	node  = GSIMapNodeForKey(self, (GSIMapKey)aKey);

      if (node)
	{
	  return GSI_MAP_READ_VALUE(self, &node->value).obj;
	}
    }
  return nil;
}

- (void) removeAllObjects
{
  if (nodeCount > 0)
    {
      GSIMapEmptyMap(self);
      version++;
    }
}

- (void) removeObjectForKey: (id)aKey
{
  if (aKey == nil)
    {
      NSWarnMLog(@"attempt to remove nil key from map table %@", self);
      return;
    }
  if (nodeCount > 0)
    {
      GSIMapTable	map = (GSIMapTable)self;
      GSIMapBucket	bucket;
      GSIMapNode	node;

      bucket = GSIMapBucketForKey(map, (GSIMapKey)aKey);
      node = GSIMapNodeForKeyInBucket(map, bucket, (GSIMapKey)aKey);
      if (node != 0)
	{
	  GSIMapRemoveNodeFromMap(map, bucket, node);
	  GSIMapFreeNode(map, node);
	  version++;
	}
    }
}

- (void) setObject: (id)anObject forKey: (id)aKey
{
  GSIMapNode	node;

  if (aKey == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@:] given nil argument",
        NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  node = GSIMapNodeForKey(self, (GSIMapKey)aKey);
  if (node)
    {
      if (GSI_MAP_READ_VALUE(self, &node->value).obj != anObject)
	{
          GSI_MAP_RELEASE_VAL(self, node->value);
          GSI_MAP_WRITE_VAL(self, &node->value, (GSIMapVal)anObject);
          GSI_MAP_RETAIN_VAL(self, node->value);
	  version++;
	}
    }
  else
    {
      GSIMapAddPair(self, (GSIMapKey)aKey, (GSIMapVal)anObject);
      version++;
    }
}

- (NSPointerFunctions*) valuePointerFunctions
{
  NSConcretePointerFunctions	*p = [NSConcretePointerFunctions new];

  p->_x = self->cb.pf.v;
  return [p autorelease];
}

@end

@implementation NSConcreteMapTableKeyEnumerator

- (id) initWithMapTable: (NSConcreteMapTable*)t
{
  table = RETAIN(t);
  enumerator = GSIMapEnumeratorForMap(table);
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
  RELEASE(table);
  [super dealloc];
}

@end

@implementation NSConcreteMapTableObjectEnumerator

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

