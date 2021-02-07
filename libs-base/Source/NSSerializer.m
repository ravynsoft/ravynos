/** Class for serialization in GNUStep
   Copyright (C) 1997 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdoanld <richard@brainstorm.co.uk>
   Date: August 1997

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

   <title>NSSerializer class reference</title>
   $Date$ $Revision$
   */

#import "common.h"
#import "Foundation/NSData.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSException.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSProxy.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSSet.h"
#import "Foundation/NSThread.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSNotificationQueue.h"
#import "Foundation/NSValue.h"

#import "GSPrivate.h"

@class	GSDictionary;
@class	GSMutableDictionary;
@interface GSMutableDictionary : NSObject	// Help the compiler
@end
@class	NSDataMalloc;
@interface NSDataMalloc : NSObject	// Help the compiler
@end

/*
 *	Setup for inline operation of string map tables.
 */
#define	GSI_MAP_KTYPES	GSUNION_OBJ
#define	GSI_MAP_VTYPES	GSUNION_NSINT
#define	GSI_MAP_RETAIN_KEY(M, X)	
#define	GSI_MAP_RELEASE_KEY(M, X)	
#define	GSI_MAP_RETAIN_VAL(M, X)	
#define	GSI_MAP_RELEASE_VAL(M, X)	
#define	GSI_MAP_HASH(M, X)	[(X).obj hash]
#define	GSI_MAP_EQUAL(M, X,Y)	[(X).obj isEqualToString: (Y).obj]
#define	GSI_MAP_NOCLEAN	1

#include "GNUstepBase/GSIMap.h"

/*
 *	Setup for inline operation of string arrays.
 */
#define	GSI_ARRAY_NO_RETAIN	1
#define	GSI_ARRAY_NO_RELEASE	1
#define	GSI_ARRAY_TYPES	GSUNION_OBJ

#include "GNUstepBase/GSIArray.h"

/*
 *	Define constants for data types and variables to hold them.
 */
#define ST_XREF		0
#define ST_CSTRING	1
#define ST_STRING	2
#define ST_ARRAY	3
#define ST_MARRAY	4
#define ST_DICT		5
#define ST_MDICT	6
#define ST_DATA		7
#define ST_DATE		8
#define ST_NUMBER	9

static char	st_xref = (char)ST_XREF;
static char	st_cstring = (char)ST_CSTRING;
static char	st_string = (char)ST_STRING;
static char	st_array = (char)ST_ARRAY;
static char	st_marray = (char)ST_MARRAY;
static char	st_dict = (char)ST_DICT;
static char	st_mdict = (char)ST_MDICT;
static char	st_data = (char)ST_DATA;
static char	st_date = (char)ST_DATE;
static char	st_number = (char)ST_NUMBER;



/*
 * Variables to cache class information. These are used to check how
 * an instance should be serialized.
 */
static Class	ArrayClass = 0;
static Class	MutableArrayClass = 0;
static Class	DataClass = 0;
static Class	DateClass = 0;
static Class	DictionaryClass = 0;
static Class	MutableDictionaryClass = 0;
static Class	StringClass = 0;
static Class	NumberClass = 0;

typedef struct {
  NSMutableData	*data;
  void		(*appImp)(NSData*,SEL,const void*,unsigned);
  void*		(*datImp)(NSMutableData*,SEL);		// Bytes pointer.
  unsigned int	(*lenImp)(NSData*,SEL);			// Length of data.
  void		(*serImp)(NSMutableData*,SEL,int);	// Serialize integer.
  void		(*setImp)(NSMutableData*,SEL,unsigned);	// Set length of data.
  unsigned	count;			// String counter.
  GSIMapTable_t	map;			// For uniquing.
  BOOL		shouldUnique;		// Do we do uniquing?
} _NSSerializerInfo;

static SEL	appSel;
static SEL	datSel;
static SEL	lenSel;
static SEL	serSel;
static SEL	setSel;

static void
initSerializerInfo(_NSSerializerInfo* info, NSMutableData *d, BOOL u)
{
  Class	c;

  c = object_getClass(d);
  info->data = d;
  info->appImp = (void (*)(NSData*,SEL,const void*,unsigned))
    class_getMethodImplementation(c, appSel);
  info->datImp = (void* (*)(NSMutableData*,SEL))
    class_getMethodImplementation(c, datSel);
  info->lenImp = (unsigned int (*)(NSData*,SEL))
    class_getMethodImplementation(c, lenSel);
  info->serImp = (void (*)(NSMutableData*,SEL,int))
    class_getMethodImplementation(c, serSel);
  info->setImp = (void (*)(NSMutableData*,SEL,unsigned))
    class_getMethodImplementation(c, setSel);
  info->shouldUnique = u;
  (*info->appImp)(d, appSel, &info->shouldUnique, 1);
  if (u)
    {
      GSIMapInitWithZoneAndCapacity(&info->map, NSDefaultMallocZone(), 16);
      info->count = 0;
    }
}

static void
endSerializerInfo(_NSSerializerInfo* info)
{
  if (info->shouldUnique)
    GSIMapEmptyMap(&info->map);
}

static void
serializeToInfo(id object, _NSSerializerInfo* info)
{
  Class	c;

  if (object == nil || GSObjCIsInstance(object) == NO)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Class (%@) in property list - expected instance",
				[object description]];
    }
  c = object_getClass(object);

  if (GSObjCIsKindOf(c, StringClass)
      /*
      We can only save it as a c-string if it only contains ASCII characters.
      Other characters might be decoded incorrectly when deserialized since
      the c-string encoding might be different then.
      */
      && [object canBeConvertedToEncoding: NSASCIIStringEncoding])
    {
      GSIMapNode	node;

      if (info->shouldUnique)
	node = GSIMapNodeForKey(&info->map, (GSIMapKey)object);
      else
	node = 0;
      if (node == 0)
	{
	  unsigned	slen;
	  unsigned	dlen;

	  slen = [object length] + 1;
	  (*info->appImp)(info->data, appSel, &st_cstring, 1);
	  (*info->serImp)(info->data, serSel, slen);
	  dlen = (*info->lenImp)(info->data, lenSel);
	  (*info->setImp)(info->data, setSel, dlen + slen);
	  [object getCString: (*info->datImp)(info->data, datSel) + dlen
		   maxLength: slen
		    encoding: NSASCIIStringEncoding];
	  if (info->shouldUnique)
	    GSIMapAddPair(&info->map,
	      (GSIMapKey)object, (GSIMapVal)(NSUInteger)info->count++);
	}
      else
	{
	  (*info->appImp)(info->data, appSel, &st_xref, 1);
	  (*info->serImp)(info->data, serSel, node->value.nsu);
	}
    }
  else if (GSObjCIsKindOf(c, StringClass))
    {
      GSIMapNode	node;

      if (info->shouldUnique)
	node = GSIMapNodeForKey(&info->map, (GSIMapKey)object);
      else
	node = 0;
      if (node == 0)
	{
	  unsigned	slen;
	  unsigned	dlen;

	  slen = [object length];
	  (*info->appImp)(info->data, appSel, &st_string, 1);
	  (*info->serImp)(info->data, serSel, slen);
	  dlen = (*info->lenImp)(info->data, lenSel);
	  (*info->setImp)(info->data, setSel, dlen + slen*sizeof(unichar));
#if NEED_WORD_ALIGNMENT
	  /*
	   * When packing data, an item may not be aligned on a
	   * word boundary, so we work with an aligned buffer
	   * and use memcmpy()
	   */
 	  if ((dlen % __alignof__(uint32_t)) != 0)
	    {
	      unichar buffer[slen];
	      [object getCharacters: buffer];
	      memcpy((*info->datImp)(info->data, datSel) + dlen, buffer,
		     slen*sizeof(unichar));
	    }
	  else
#endif
	  [object getCharacters: (*info->datImp)(info->data, datSel) + dlen];
	  if (info->shouldUnique)
	    GSIMapAddPair(&info->map,
	      (GSIMapKey)object, (GSIMapVal)(NSUInteger)info->count++);
	}
      else
	{
	  (*info->appImp)(info->data, appSel, &st_xref, 1);
	  (*info->serImp)(info->data, serSel, node->value.nsu);
	}
    }
  else if (GSObjCIsKindOf(c, ArrayClass))
    {
      unsigned int count;

      if ([object isKindOfClass: MutableArrayClass])
        (*info->appImp)(info->data, appSel, &st_marray, 1);
      else
        (*info->appImp)(info->data, appSel, &st_array, 1);

      count = [object count];
      (*info->serImp)(info->data, serSel, count);

      if (count)
	{
	  id		objects[count];
	  unsigned int	i;

	  if ([object isProxy])
	    {
	      for (i = 0; i < count; i++)
		{
		  objects[i] = [object objectAtIndex: i];
		}
	    }
	  else
	    {
	      [object getObjects: objects];
	    }
	  for (i = 0; i < count; i++)
	    {
	      serializeToInfo(objects[i], info);
	    }
	}
    }
  else if (GSObjCIsKindOf(c, DictionaryClass))
    {
      NSEnumerator	*e = [object keyEnumerator];
      id		k;
      IMP		nxtImp;
      IMP		objImp;

      nxtImp = [e methodForSelector: @selector(nextObject)];
      objImp = [object methodForSelector: @selector(objectForKey:)];

      if ([object isKindOfClass: MutableDictionaryClass])
        (*info->appImp)(info->data, appSel, &st_mdict, 1);
      else
        (*info->appImp)(info->data, appSel, &st_dict, 1);

      (*info->serImp)(info->data, serSel, [object count]);
      while ((k = (*nxtImp)(e, @selector(nextObject))) != nil)
	{
	  id o = (*objImp)(object, @selector(objectForKey:), k);

	  serializeToInfo(k, info);
	  serializeToInfo(o, info);
	}
    }
  else if (GSObjCIsKindOf(c, DataClass))
    {
      (*info->appImp)(info->data, appSel, &st_data, 1);
      (*info->serImp)(info->data, serSel, [object length]);
      (*info->appImp)(info->data, appSel, [object bytes], [object length]);
    }
  else if (GSObjCIsKindOf(c, DateClass))
    {
      NSTimeInterval	ti = [object timeIntervalSinceReferenceDate];

      (*info->appImp)(info->data, appSel, &st_date, 1);
      [info->data serializeDataAt: &ti
		       ofObjCType: @encode(NSTimeInterval)
			  context: nil];
    }
  else if (GSObjCIsKindOf(c, NumberClass))
    {
      double	d = [object doubleValue];

      (*info->appImp)(info->data, appSel, &st_number, 1);
      [info->data serializeDataAt: &d
		       ofObjCType: @encode(double)
			  context: nil];
    }
  else
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"Unknown class (%@) in property list",
				[c description]];
    }
}



@implementation NSSerializer

static BOOL	shouldBeCompact = NO;

+ (void) initialize
{
  if (self == [NSSerializer class])
    {
      appSel = @selector(appendBytes:length:);
      datSel = @selector(mutableBytes);
      lenSel = @selector(length);
      serSel = @selector(serializeInt:);
      setSel = @selector(setLength:);
      ArrayClass = [NSArray class];
      MutableArrayClass = [NSMutableArray class];
      DataClass = [NSData class];
      DateClass = [NSDate class];
      NumberClass = [NSNumber class];
      DictionaryClass = [NSDictionary class];
      MutableDictionaryClass = [NSMutableDictionary class];
      StringClass = [NSString class];
    }
}

/**
 * Converts the supplied propertyList into a serialized representation
 * in the returned NSData object.
 */
+ (NSData*) serializePropertyList: (id)propertyList
{
  _NSSerializerInfo	info;
  NSMutableData		*d;

  NSAssert(propertyList != nil, NSInvalidArgumentException);
  d = [NSMutableData dataWithCapacity: 1024];
  initSerializerInfo(&info, d, shouldBeCompact);
  serializeToInfo(propertyList, &info);
  endSerializerInfo(&info);
  return info.data;
}

/**
 * Converts the supplied propertyList into a serialized representation
 * stored in d.
 */
+ (void) serializePropertyList: (id)propertyList
		      intoData: (NSMutableData*)d
{
  _NSSerializerInfo	info;

  NSAssert(propertyList != nil, NSInvalidArgumentException);
  NSAssert(d != nil, NSInvalidArgumentException);
  initSerializerInfo(&info, d, shouldBeCompact);
  serializeToInfo(propertyList, &info);
  endSerializerInfo(&info);
}

@end

@implementation	NSSerializer (GNUstep)
+ (void) serializePropertyList: (id)propertyList
		      intoData: (NSMutableData*)d
		       compact: (BOOL)flag
{
  _NSSerializerInfo	info;

  NSAssert(propertyList != nil, NSInvalidArgumentException);
  NSAssert(d != nil, NSInvalidArgumentException);
  initSerializerInfo(&info, d, flag);
  serializeToInfo(propertyList, &info);
  endSerializerInfo(&info);
}

+ (void) shouldBeCompact: (BOOL)flag
{
  shouldBeCompact = flag;
}
@end



static BOOL	uniquing = NO;	/* Make incoming strings unique	*/

/*
 * Variables to cache class information. These are used to create instances
 * when deserializing.
 */
static Class	MACls = 0;	/* Mutable Array	*/
static Class	DCls = 0;	/* Data			*/
static Class	MDCls = 0;	/* Mutable Dictionary	*/

typedef struct {
  NSData	*data;
  unsigned	*cursor;
  BOOL		mutable;
  BOOL		didUnique;
  void		(*debImp)();
  unsigned int	(*deiImp)();
  GSIArray_t	array;
} _NSDeserializerInfo;

static SEL debSel;
static SEL deiSel;
static SEL dInitSel;
static SEL maInitSel;
static SEL mdInitSel;
static SEL maAddSel;
static SEL mdSetSel;
static IMP dInitImp;
static IMP maInitImp;
static IMP mdInitImp;
static IMP maAddImp;
static IMP mdSetImp;

static BOOL
initDeserializerInfo(_NSDeserializerInfo* info, NSData *d, unsigned *c, BOOL m)
{
  unsigned char	u;

  info->data = d;
  info->cursor = c;
  info->mutable = m;
  info->debImp = (void (*)())[d methodForSelector: debSel];
  info->deiImp = (unsigned int (*)())[d methodForSelector: deiSel];
  (*info->debImp)(d, debSel, &u, 1, c);
  if (u == 0 || u == 1)
    {
      info->didUnique = u;		// Old (current) format
    }
  else
    {
      if (u == 'G')
	{
	  const unsigned char	*b = [d bytes];
	  unsigned int		l = [d length];

	  if (*c + 11 < l && memcmp(&b[*c-1], "GNUstepSer", 10) == 0)
	    {
	      *c += 9;
	      (*info->debImp)(d, debSel, &u, 1, c);
	      NSLog(@"Serialised data version %d not supported ..."
		@" try another version of GNUstep", u);
	      return NO;
	    }
	}
      NSLog(@"Bad serialised data");
      return NO;
    }
  if (info->didUnique)
    {
      GSIArrayInitWithZoneAndCapacity(&info->array, NSDefaultMallocZone(), 16);
    }
  return YES;
}

static void
endDeserializerInfo(_NSDeserializerInfo* info)
{
  if (info->didUnique)
    {
      GSIArrayEmpty(&info->array);
    }
}

static id
deserializeFromInfo(_NSDeserializerInfo* info)
{
  char		code;
  unsigned int	size;

  (*info->debImp)(info->data, debSel, &code, 1, info->cursor);

  switch (code)
    {
      case ST_XREF:
	if (info->didUnique)
	  {
	    size = (*info->deiImp)(info->data, deiSel, info->cursor);
	    if (size < GSIArrayCount(&info->array))
	      {
		return RETAIN(GSIArrayItemAtIndex(&info->array, size).obj);
	      }
	    else
	      {
		[NSException raise: NSInvalidArgumentException
			    format: @"Bad cross reference in property list"];
	      }
	  }
	else
	  {
	    [NSException raise: NSInvalidArgumentException
			format: @"Unexpected cross reference in property list"];
	  }

      case ST_CSTRING:
	{
	  NSString	*s;
	  char		*b;
	
	  size = (*info->deiImp)(info->data, deiSel, info->cursor);
	  b = NSZoneMalloc(NSDefaultMallocZone(), size);
	  (*info->debImp)(info->data, debSel, b, size, info->cursor);
	  s = [[StringClass alloc] initWithBytesNoCopy: b
						length: size - 1
					      encoding: NSASCIIStringEncoding
					  freeWhenDone: YES];
	  /*
	   * If we are supposed to be doing uniquing of strings, handle it.
	   */
	  if (uniquing == YES)
	    {
	      s = GSUnique(s);
	    }

	  /*
           * If uniquing was done on serialisation, store the string for
	   * later reference.
	   */
	  if (info->didUnique)
	    {
	      GSIArrayAddItem(&info->array, (GSIArrayItem)((id)s));
	    }
	  GS_CONSUMED(s)
	  return s;
	}

      case ST_STRING:
	{
	  NSString	*s;
	  unichar	*b;
	
	  size = (*info->deiImp)(info->data, deiSel, info->cursor);
	  b = NSZoneMalloc(NSDefaultMallocZone(), size*sizeof(unichar));
	  (*info->debImp)(info->data, debSel, b, size*sizeof(unichar),
	    info->cursor);
	  s = [[StringClass alloc] initWithBytesNoCopy: b
						length: size*sizeof(unichar)
					      encoding: NSUnicodeStringEncoding
					  freeWhenDone: YES];
	  /*
	   * If we are supposed to be doing uniquing of strings, handle it.
	   */
	  if (uniquing == YES)
	    {
	      s = GSUnique(s);
	    }

	  /*
           * If uniquing was done on serialisation, store the string for
	   * later reference.
	   */
	  if (info->didUnique)
	    {
	      GSIArrayAddItem(&info->array, (GSIArrayItem)((id)s));
	    }
	  GS_CONSUMED(s)
	  return s;
	}

      case ST_ARRAY:
      case ST_MARRAY:
	size = (*info->deiImp)(info->data, deiSel, info->cursor);
	{
	  id		a;

	  a = NSAllocateObject(MACls, 0, NSDefaultMallocZone());
	  a = (*maInitImp)(a, maInitSel, size);
	  if (a != nil)
	    {
	      unsigned	i;

	      for (i = 0; i < size; i++)
		{
		  id	o = deserializeFromInfo(info);

		  if (o == nil)
		    {
		      RELEASE(a);
		      return nil;
		    }
		  (*maAddImp)(a, maAddSel, o);
		  RELEASE(o);
		}
	      if (code != ST_MARRAY && info->mutable == NO)
		{
                  a = GS_IMMUTABLE(a);
		}
	    }
	  return a;
	}

      case ST_DICT:
      case ST_MDICT:
	size = (*info->deiImp)(info->data, deiSel, info->cursor);
	{
	  id		d;

	  d = NSAllocateObject(MDCls, 0, NSDefaultMallocZone());
	  d = (*mdInitImp)(d, mdInitSel, size);
	  if (d != nil)
	    {
	      unsigned int	i;

	      for (i = 0; i < size; i++)
		{
		  id	k = deserializeFromInfo(info);

		  if (k == nil)
		    {
		      RELEASE(d);
		      return nil;
		    }
		  else
		    {
		      id	o = deserializeFromInfo(info);

		      if (o == nil)
			{
			  RELEASE(k);
			  RELEASE(d);
			  return nil;
			}
		      else
			{
			  (*mdSetImp)(d, mdSetSel, o, k);
			  /*
			   * Since a dictionary copies its keys rather
			   * than retaining them, we must autorelease
			   * rather than simply releasing as the key may
			   * be referred to by a cross-reference later.
			   */
			  IF_NO_GC(AUTORELEASE(k);)
			  RELEASE(o);
			}
		    }
		}
	      if (code != ST_MDICT && info->mutable == NO)
		{
                  d = GS_IMMUTABLE(d);
		}
	    }
	  return d;
	}

      case ST_DATA:
	{
	  NSData	*d;

	  size = (*info->deiImp)(info->data, deiSel, info->cursor);
	  d = (NSData*)NSAllocateObject(DCls, 0, NSDefaultMallocZone());
	  if (size > 0)
	    {
	      void	*b = NSZoneMalloc(NSDefaultMallocZone(), size);
	
	      (*info->debImp)(info->data, debSel, b, size, info->cursor);
	      d = (*dInitImp)(d, dInitSel, b, size);
	    }
	  else
	    {
	      d = (*dInitImp)(d, dInitSel, 0, 0);
	    }
	  return d;
	}

      case ST_DATE:
	{
	  NSTimeInterval	ti;

	  [info->data deserializeDataAt: &ti
			     ofObjCType: @encode(NSTimeInterval)
			       atCursor: info->cursor
				context: nil];
	  return [[NSDate alloc] initWithTimeIntervalSinceReferenceDate: ti];
	}

      case ST_NUMBER:
	{
	  double	d;

	  [info->data deserializeDataAt: &d
			     ofObjCType: @encode(double)
			       atCursor: info->cursor
				context: nil];
	  return [[NSNumber alloc] initWithDouble: d];
	}

      default:
	break;
    }
  return nil;
}



@interface	_NSDeserializerProxy : NSProxy
{
  _NSDeserializerInfo	info;
  id			plist;
}
+ (_NSDeserializerProxy*) proxyWithData: (NSData*)d
			       atCursor: (unsigned int*)c
				mutable: (BOOL)m;
@end

@implementation	_NSDeserializerProxy
+ (_NSDeserializerProxy*) proxyWithData: (NSData*)d
			       atCursor: (unsigned int*)c
				mutable: (BOOL)m
{
  _NSDeserializerProxy	*proxy;

  proxy = (_NSDeserializerProxy*)NSAllocateObject(self,0,NSDefaultMallocZone());
  if (initDeserializerInfo(&proxy->info, RETAIN(d), c, m) == YES)
    {
      return AUTORELEASE(proxy);
    }
  else
    {
      DESTROY(proxy);
      return nil;
    }
}

- (void) dealloc
{
  RELEASE(info.data);
  endDeserializerInfo(&info);
  RELEASE(plist);
  [super dealloc];
}

- (BOOL) isEqual: (id)other
{
  if (other == self)
    return YES;
  else
    return [[self self] isEqual: other];
}

- (id) self
{
  if (plist == nil && info.data != nil)
    {
      plist = deserializeFromInfo(&info);
      RELEASE(info.data);
      info.data = nil;
    }
  return plist;
}
@end



@implementation NSDeserializer

+ (void) initialize
{
  if (self == [NSDeserializer class])
    {
      debSel = @selector(deserializeBytes:length:atCursor:);
      deiSel = @selector(deserializeIntAtCursor:);
      dInitSel = @selector(initWithBytesNoCopy:length:);
      maInitSel = @selector(initWithCapacity:);
      mdInitSel = @selector(initWithCapacity:);
      maAddSel = @selector(addObject:);
      mdSetSel = @selector(setObject:forKey:);
      MACls = [GSMutableArray class];
      DCls = [NSDataMalloc class];
      MDCls = [GSMutableDictionary class];
      dInitImp = [DCls instanceMethodForSelector: dInitSel];
      maInitImp = [MACls instanceMethodForSelector: maInitSel];
      mdInitImp = [MDCls instanceMethodForSelector: mdInitSel];
      maAddImp = [MACls instanceMethodForSelector: maAddSel];
      mdSetImp = [MDCls instanceMethodForSelector: mdSetSel];
      StringClass = [NSString class];
    }
}

/**
 * <p>Deserializes the property list stored in data at the offset specified
 * by the value pointed to by cursor.  Upon completion the value in cursor
 * is updated to refer to a position immediately after the end of the
 * deserialized sequence.
 * </p>
 * <p>The flag is used to specify whether container objects in the
 * deserialized list should be instances of mutable or immutable classes.
 * </p>
 */
+ (id) deserializePropertyListFromData: (NSData*)data
                              atCursor: (unsigned int*)cursor
                     mutableContainers: (BOOL)flag
{
  _NSDeserializerInfo	info;
  id	o;

  if (data == nil || [data isKindOfClass: [NSData class]] == NO)
    {
      return nil;
    }
  NSAssert(cursor != 0, NSInvalidArgumentException);
  if (initDeserializerInfo(&info, data, cursor, flag) == YES)
    {
      o = deserializeFromInfo(&info);
      endDeserializerInfo(&info);
      return AUTORELEASE(o);
    }
  else
    {
      return nil;
    }
}

/**
 * <p>Deserializes the property list stored in data to produce a single
 * property list object (dictionary, array, string, data, number or date).
 * </p>
 * <p>The flag is used to specify whether container objects in the
 * deserialized list should be instances of mutable or immutable classes.
 * </p>
 */
+ (id) deserializePropertyListFromData: (NSData*)data
                     mutableContainers: (BOOL)flag
{
  _NSDeserializerInfo	info;
  unsigned int	cursor = 0;
  id		o;

  if (data == nil || [data isKindOfClass: [NSData class]] == NO)
    {
      return nil;
    }
  if (initDeserializerInfo(&info, data, &cursor, flag) == YES)
    {
      o = deserializeFromInfo(&info);
      endDeserializerInfo(&info);
      return AUTORELEASE(o);
    }
  else
    {
      return nil;
    }
}

/**
 * <p>Deserializes the property list stored in data at the offset specified
 * by the value pointed to by cursor.  Upon completion the value in cursor
 * is updated to refer to a position immediately after the end of the
 * deserialized sequence.
 * </p>
 * <p>The flag is used to specify whether container objects in the
 * deserialized list should be instances of mutable or immutable classes.
 * </p>
 * <p>The length is used to determine  whether lazy deserialization is done,
 * if the data is longer than this value, a proxy is returned rather than
 * the actual property list, and the real deserialization can be done
 * later.
 * </p>
 */
+ (id) deserializePropertyListLazilyFromData: (NSData*)data
                                    atCursor: (unsigned*)cursor
                                      length: (unsigned)length
                           mutableContainers: (BOOL)flag
{
  if (data == nil || [data isKindOfClass: [NSData class]] == NO)
    {
      return nil;
    }
  NSAssert(cursor != 0, NSInvalidArgumentException);
  if (length > [data length] - *cursor)
    {
      _NSDeserializerInfo   info;
      id    o;

      if (initDeserializerInfo(&info, data, cursor, flag) == YES)
	{
	  o = deserializeFromInfo(&info);
	  endDeserializerInfo(&info);
	  return AUTORELEASE(o);
	}
      else
	{
	  return nil;
	}
    }
  else
    {
      return [_NSDeserializerProxy proxyWithData: data
					atCursor: cursor
					 mutable: flag];
    }
}
@end

@implementation NSDeserializer (GNUstep)
/**
 * This method turns on/off uniquing of strings as they are
 * deserialized from data objects.  The uniquing mechanism
 * employs the GNUstep-specific functions documented with
 * the NSCountedSet class.
 */
+ (void) uniquing: (BOOL)flag
{
  if (flag == YES)
    GSUniquing(YES);
  uniquing = flag;
}
@end

