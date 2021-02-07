/** Implementation of NSArchiver for GNUstep
   Copyright (C) 1998,1999 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Created: October 1998

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

   <title>NSArchiver class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#define	EXPOSE_NSArchiver_IVARS	1
#define	EXPOSE_NSUnarchiver_IVARS	1
/*
 *	Setup for inline operation of pointer map tables.
 */
#define	GSI_MAP_KTYPES	GSUNION_NSINT | GSUNION_PTR | GSUNION_OBJ | GSUNION_CLS
#define	GSI_MAP_VTYPES	GSUNION_NSINT | GSUNION_PTR | GSUNION_OBJ
#define	GSI_MAP_RETAIN_KEY(M, X)	
#define	GSI_MAP_RELEASE_KEY(M, X)	
#define	GSI_MAP_RETAIN_VAL(M, X)	
#define	GSI_MAP_RELEASE_VAL(M, X)	
#define	GSI_MAP_HASH(M, X)	((X).nsu)
#define	GSI_MAP_EQUAL(M, X,Y)	((X).ptr == (Y).ptr)
#define	GSI_MAP_NOCLEAN	1


#include "GNUstepBase/GSIMap.h"

#define	_IN_NSARCHIVER_M
#import "Foundation/NSArchiver.h"
#undef	_IN_NSARCHIVER_M

#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSException.h"

typedef	unsigned char	uchar;

NSString * const NSInconsistentArchiveException =
  @"NSInconsistentArchiveException";

#define	PREFIX		"GNUstep archive"

static SEL serSel;
static SEL tagSel;
static SEL xRefSel;
static SEL eObjSel;
static SEL eValSel;

@class NSMutableDataMalloc;
@interface NSMutableDataMalloc : NSObject	// Help the compiler
@end
static Class	NSMutableDataMallocClass;

/**
 *  <p>Implementation of [NSCoder] capable of creating sequential archives which
 *  must be read in the same order they were written.  This class implements
 *  methods for saving to and restoring from a serial archive (usually a file
 *  on disk, but can be an [NSData] object) as well as methods that can be
 *  used by objects that need to write/restore themselves.</p>
 *
 * <p>Note, the sibling class [NSKeyedArchiver] supports a form of archive
 *  that is more robust to class changes, and is recommended over this one.</p>
 */
@implementation NSArchiver

+ (void) initialize
{
  if (self == [NSArchiver class])
    {
      serSel = @selector(serializeDataAt:ofObjCType:context:);
      tagSel = @selector(serializeTypeTag:);
      xRefSel = @selector(serializeTypeTag:andCrossRef:);
      eObjSel = @selector(encodeObject:);
      eValSel = @selector(encodeValueOfObjCType:at:);
      NSMutableDataMallocClass = [NSMutableDataMalloc class];
    }
}

/**
 *  Creates an NSMutableData instance and calls
 *  [initForWritingWithMutableData:].
 */
- (id) init
{
  NSMutableData	*d;

  d = [[NSMutableDataMallocClass allocWithZone: [self zone]] init];
  self = [self initForWritingWithMutableData: d];
  RELEASE(d);
  return self;
}

/**
 *  Init instance that will archive its data to mdata.  (Even if
 *  [archiveRootObject:toFile:] is called, this still gets written to.)
 */
- (id) initForWritingWithMutableData: (NSMutableData*)mdata
{
  self = [super init];
  if (self)
    {
      NSZone		*zone = [self zone];

      _data = RETAIN(mdata);
      if ([self directDataAccess] == YES)
        {
	  _dst = _data;
	}
      else
	{
	  _dst = self;
	}
      _serImp = [_dst methodForSelector: serSel];
      _tagImp = [_dst methodForSelector: tagSel];
      _xRefImp = [_dst methodForSelector: xRefSel];
      _eObjImp = [self methodForSelector: eObjSel];
      _eValImp = [self methodForSelector: eValSel];

      [self resetArchiver];

      /*
       *	Set up map tables.
       */
      _clsMap = (GSIMapTable)NSZoneMalloc(zone, sizeof(GSIMapTable_t)*6);
      _cIdMap = &_clsMap[1];
      _uIdMap = &_clsMap[2];
      _ptrMap = &_clsMap[3];
      _namMap = &_clsMap[4];
      _repMap = &_clsMap[5];
      GSIMapInitWithZoneAndCapacity(_clsMap, zone, 100);
      GSIMapInitWithZoneAndCapacity(_cIdMap, zone, 10);
      GSIMapInitWithZoneAndCapacity(_uIdMap, zone, 200);
      GSIMapInitWithZoneAndCapacity(_ptrMap, zone, 100);
      GSIMapInitWithZoneAndCapacity(_namMap, zone, 1);
      GSIMapInitWithZoneAndCapacity(_repMap, zone, 1);
    }
  return self;
}

- (void) dealloc
{
  RELEASE(_data);
  if (_clsMap)
    {
      GSIMapEmptyMap(_clsMap);
      if (_cIdMap)
	{
	  GSIMapEmptyMap(_cIdMap);
	}
      if (_uIdMap)
	{
	  GSIMapEmptyMap(_uIdMap);
	}
      if (_ptrMap)
	{
	  GSIMapEmptyMap(_ptrMap);
	}
      if (_namMap)
	{
	  GSIMapEmptyMap(_namMap);
	}
      if (_repMap)
	{
	  GSIMapEmptyMap(_repMap);
	}
      NSZoneFree(_clsMap->zone, (void*)_clsMap);
    }
  [super dealloc];
}

/**
 *  Writes serialized representation of object and, recursively, any
 *  other objects it holds references to, to byte array.
 */
+ (NSData*) archivedDataWithRootObject: (id)rootObject
{
  NSArchiver	*archiver;
  id		d;
  NSZone	*z = NSDefaultMallocZone();

  d = [[NSMutableDataMallocClass allocWithZone: z] initWithCapacity: 0];
  if (d == nil)
    {
      return nil;
    }
  archiver = [[self allocWithZone: z] initForWritingWithMutableData: d];
  RELEASE(d);
  d = nil;
  if (archiver)
    {
      NS_DURING
	{
	  [archiver encodeRootObject: rootObject];
	  d = AUTORELEASE([archiver->_data copy]);
	}
      NS_HANDLER
	{
	  RELEASE(archiver);
	  [localException raise];
	}
      NS_ENDHANDLER
      RELEASE(archiver);
    }

  return d;
}

/**
 *  Writes out serialized representation of object and, recursively, any
 *  other objects it holds references to.
 */
+ (BOOL) archiveRootObject: (id)rootObject
		    toFile: (NSString*)path
{
  id	d = [self archivedDataWithRootObject: rootObject];

  return [d writeToFile: path atomically: YES];
}

- (void) encodeArrayOfObjCType: (const char*)type
			 count: (NSUInteger)count
			    at: (const void*)buf
{
  uint32_t      c;
  uint8_t	bytes[20];
  uint8_t	*bytePtr = 0;
  uint8_t	byteCount = 0;
  NSUInteger	i;
  NSUInteger	offset = 0;
  uint32_t	size;
  uint32_t	version;
  uchar		info;

  type = GSSkipTypeQualifierAndLayoutInfo(type);
  size = objc_sizeof_type(type);
  version = [self systemVersion];

  if (12402 == version)
    {
      NSUInteger	tmp = count;

      bytes[sizeof(bytes) - ++byteCount] = (uint8_t)(tmp % 128);
      tmp /= 128;
      while (tmp > 0)
	{
	  bytes[sizeof(bytes) - ++byteCount] = (uint8_t)(128 | (tmp % 128));
	  tmp /= 128;
	}
      bytePtr = &bytes[sizeof(bytes) - byteCount];
    }

  /* We normally store the count as a 32bit integer ... but if it's
   * very big, we store 0xffffffff and then an additional 64bit value
   * containing the actual count.
   */
  if (count >= 0xffffffff)
    {
      c = 0xffffffff;
    }
  else
    {
      c = count;
    }

  switch (*type)
    {
      case _C_ID:	info = _GSC_NONE;		break;
      case _C_CHR:	info = _GSC_CHR;		break;
      case _C_UCHR:	info = _GSC_UCHR; 		break;
      case _C_SHT:	info = _GSC_SHT | _GSC_S_SHT;	break;
      case _C_USHT:	info = _GSC_USHT | _GSC_S_SHT;	break;
      case _C_INT:	info = _GSC_INT | _GSC_S_INT;	break;
      case _C_UINT:	info = _GSC_UINT | _GSC_S_INT;	break;
      case _C_LNG:	info = _GSC_LNG | _GSC_S_LNG;	break;
      case _C_ULNG:	info = _GSC_ULNG | _GSC_S_LNG; break;
      case _C_LNG_LNG:	info = _GSC_LNG_LNG | _GSC_S_LNG_LNG;	break;
      case _C_ULNG_LNG:	info = _GSC_ULNG_LNG | _GSC_S_LNG_LNG;	break;
      case _C_FLT:	info = _GSC_FLT;	break;
      case _C_DBL:	info = _GSC_DBL;	break;
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:	info = _GSC_BOOL;	break;
#endif
      default:		info = _GSC_NONE;	break;
    }

  /*
   *	Simple types can be serialized immediately, more complex ones
   *	are dealt with by our [encodeValueOfObjCType:at:] method.
   */
  if (info == _GSC_NONE)
    {
      if (_initialPass == NO)
	{
	  (*_tagImp)(_dst, tagSel, _GSC_ARY_B);
	  if (12402 == version)
	    {
	      for (i = 0; i < byteCount; i++)
		{
		  (*_serImp)(_dst, serSel, bytePtr + i, @encode(uint8_t), nil);
		}
	    }
	  else
	    {
	      (*_serImp)(_dst, serSel, &c, @encode(uint32_t), nil);
	      if (0xffffffff == c)
		{
		  (*_serImp)(_dst, serSel, &count, @encode(NSUInteger), nil);
		}
	    }
	}

      for (i = 0; i < c; i++)
	{
	  (*_eValImp)(self, eValSel, type, (char*)buf + offset);
	  offset += size;
	}
    }
  else if (_initialPass == NO)
    {
      (*_tagImp)(_dst, tagSel, _GSC_ARY_B);
      if (12402 == version)
	{
	  for (i = 0; i < byteCount; i++)
	    {
	      (*_serImp)(_dst, serSel, bytePtr + i, @encode(uint8_t), nil);
	    }
	}
      else
	{
	  (*_serImp)(_dst, serSel, &c, @encode(uint32_t), nil);
	  if (0xffffffff == c)
	    {
	      (*_serImp)(_dst, serSel, &count, @encode(NSUInteger), nil);
	    }
	}

      (*_tagImp)(_dst, tagSel, info);
      for (i = 0; i < count; i++)
	{
	  (*_serImp)(_dst, serSel, (char*)buf + offset, type, nil);
	  offset += size;
	}
    }
}

- (void) encodeValueOfObjCType: (const char*)type
			    at: (const void*)buf
{
  type = GSSkipTypeQualifierAndLayoutInfo(type);
  switch (*type)
    {
      case _C_ID:
	(*_eObjImp)(self, eObjSel, *(void**)buf);
	return;

      case _C_ARY_B:
	{
	  unsigned	count = atoi(++type);

	  while (isdigit(*type))
	    {
	      type++;
	    }

	  if (_initialPass == NO)
	    {
	      (*_tagImp)(_dst, tagSel, _GSC_ARY_B);
	    }

	  [self encodeArrayOfObjCType: type count: count at: buf];
	}
	return;

      case _C_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  if (_initialPass == NO)
	    {
	      (*_tagImp)(_dst, tagSel, _GSC_STRUCT_B);
	    }
	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      (*_eValImp)(self, eValSel, ftype, (char*)buf + offset);
	    }
	}
	return;

      case _C_PTR:
	if (*(void**)buf == 0)
	  {
	    if (_initialPass == NO)
	      {
		/*
		 *	Special case - a null pointer gets an xref of zero
		 */
		(*_tagImp)(_dst, tagSel, _GSC_PTR | _GSC_XREF | _GSC_X_0);
	      }
	  }
	else
	  {
	    GSIMapNode	node;

	    node = GSIMapNodeForKey(_ptrMap, (GSIMapKey)*(void**)buf);
	    if (_initialPass == YES)
	      {
		/*
		 *	First pass - add pointer to map and encode item pointed
		 *	to in case it is a conditionally encoded object.
		 */
		if (node == 0)
		  {
		    GSIMapAddPair(_ptrMap,
		      (GSIMapKey)*(void**)buf, (GSIMapVal)(NSUInteger)0);
		    type++;
		    buf = *(char**)buf;
		    (*_eValImp)(self, eValSel, type, buf);
		  }
	      }
	    else if (node == 0 || node->value.nsu == 0)
	      {
		/*
		 *	Second pass, unwritten pointer - write it.
		 */
		if (node == 0)
		  {
		    node = GSIMapAddPair(_ptrMap,
		      (GSIMapKey)*(void**)buf, (GSIMapVal)(NSUInteger)++_xRefP);
		  }
		else
		  {
		    node->value.nsu = ++_xRefP;
		  }
		(*_xRefImp)(_dst, xRefSel, _GSC_PTR, node->value.nsu);
		type++;
		buf = *(char**)buf;
		(*_eValImp)(self, eValSel, type, buf);
	      }
	    else
	      {
		/*
		 *	Second pass, write a cross-reference number.
		 */
		(*_xRefImp)(_dst, xRefSel, _GSC_PTR|_GSC_XREF,
		  node->value.nsu);
	      }
	  }
	return;

      default:	/* Types that can be ignored in first pass.	*/
	if (_initialPass)
	  {
	    return;	
	  }
	break;
    }

  switch (*type)
    {
      case _C_CLASS:
	if (*(Class*)buf == 0)
	  {
	    /*
	     *	Special case - a null pointer gets an xref of zero
	     */
	    (*_tagImp)(_dst, tagSel, _GSC_CLASS | _GSC_XREF | _GSC_X_0);
	  }
	else
	  {
	    Class	c = *(Class*)buf;
	    GSIMapNode	node;
	    BOOL	done = NO;

	    node = GSIMapNodeForKey(_clsMap, (GSIMapKey)(void*)c);
	
	    if (node != 0)
	      {
		(*_xRefImp)(_dst, xRefSel, _GSC_CLASS | _GSC_XREF,
		  node->value.nsu);
		return;
	      }
	    while (done == NO)
	      {
		int		tmp = class_getVersion(c);
		unsigned	version = tmp;
		Class		s = class_getSuperclass(c);

		if (tmp < 0)
		  {
		    [NSException raise: NSInternalInconsistencyException
				format: @"negative class version"];
		  }
		node = GSIMapAddPair(_clsMap,
		  (GSIMapKey)(void*)c, (GSIMapVal)(NSUInteger)++_xRefC);
		/*
		 *	Encode tag and crossref number.
		 */
		(*_xRefImp)(_dst, xRefSel, _GSC_CLASS, node->value.nsu);
		/*
		 *	Encode class, and version.
		 */
		(*_serImp)(_dst, serSel, &c, @encode(Class), nil);
		(*_serImp)(_dst, serSel, &version, @encode(unsigned), nil);
		/*
		 *	If we have a super class that has not been encoded,
		 *	we must loop round to encode it here so that its
		 *	version information will be available when objects
		 *	of its subclasses are decoded and call
		 *	[super initWithCoder:ccc]
		 */
		if (s == c || s == 0
		  || GSIMapNodeForKey(_clsMap, (GSIMapKey)(void*)s) != 0)
		  {
		    done = YES;
		  }
		else
		  {
		    c = s;
		  }
	      }
	    /*
	     *	Encode an empty tag to terminate the list of classes.
	     */
	    (*_tagImp)(_dst, tagSel, _GSC_NONE);
	  }
	return;

      case _C_SEL:
	if (*(SEL*)buf == 0)
	  {
	    /*
	     *	Special case - a null pointer gets an xref of zero
	     */
	    (*_tagImp)(_dst, tagSel, _GSC_SEL | _GSC_XREF | _GSC_X_0);
	  }
	else
	  {
	    SEL		s = *(SEL*)buf;
	    GSIMapNode	node = GSIMapNodeForKey(_ptrMap, (GSIMapKey)(void*)s);

	    if (node == 0)
	      {
		node = GSIMapAddPair(_ptrMap,
		  (GSIMapKey)(void*)s, (GSIMapVal)(NSUInteger)++_xRefP);
		(*_xRefImp)(_dst, xRefSel, _GSC_SEL, node->value.nsu);
		/*
		 *	Encode selector.
		 */
		(*_serImp)(_dst, serSel, buf, @encode(SEL), nil);
	      }
	    else
	      {
		(*_xRefImp)(_dst, xRefSel, _GSC_SEL|_GSC_XREF,
		  node->value.nsu);
	      }
	  }
	return;

      case _C_CHARPTR:
	if (*(char**)buf == 0)
	  {
	    /*
	     *	Special case - a null pointer gets an xref of zero
	     */
	    (*_tagImp)(_dst, tagSel, _GSC_CHARPTR | _GSC_XREF | _GSC_X_0);
	  }
	else
	  {
	    GSIMapNode	node;

	    node = GSIMapNodeForKey(_ptrMap, (GSIMapKey)*(char**)buf);
	    if (node == 0)
	      {
		node = GSIMapAddPair(_ptrMap,
		  (GSIMapKey)*(char**)buf, (GSIMapVal)(NSUInteger)++_xRefP);
		(*_xRefImp)(_dst, xRefSel, _GSC_CHARPTR, node->value.nsu);
		(*_serImp)(_dst, serSel, buf, type, nil);
	      }
	    else
	      {
		(*_xRefImp)(_dst, xRefSel, _GSC_CHARPTR|_GSC_XREF,
		  node->value.nsu);
	      }
	  }
	return;

      case _C_CHR:
	(*_tagImp)(_dst, tagSel, _GSC_CHR);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(signed char), nil);
	return;

      case _C_UCHR:
	(*_tagImp)(_dst, tagSel, _GSC_UCHR);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(unsigned char), nil);
	return;

      case _C_SHT:
	(*_tagImp)(_dst, tagSel, _GSC_SHT | _GSC_S_SHT);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(short), nil);
	return;

      case _C_USHT:
	(*_tagImp)(_dst, tagSel, _GSC_USHT | _GSC_S_SHT);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(unsigned short), nil);
	return;

      case _C_INT:
	(*_tagImp)(_dst, tagSel, _GSC_INT | _GSC_S_INT);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(int), nil);
	return;

      case _C_UINT:
	(*_tagImp)(_dst, tagSel, _GSC_UINT | _GSC_S_INT);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(unsigned int), nil);
	return;

      case _C_LNG:
	(*_tagImp)(_dst, tagSel, _GSC_LNG | _GSC_S_LNG);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(long), nil);
	return;

      case _C_ULNG:
	(*_tagImp)(_dst, tagSel, _GSC_ULNG | _GSC_S_LNG);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(unsigned long), nil);
	return;

      case _C_LNG_LNG:
	(*_tagImp)(_dst, tagSel, _GSC_LNG_LNG | _GSC_S_LNG_LNG);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(long long), nil);
	return;

      case _C_ULNG_LNG:
	(*_tagImp)(_dst, tagSel, _GSC_ULNG_LNG | _GSC_S_LNG_LNG);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(unsigned long long), nil);
	return;

      case _C_FLT:
	(*_tagImp)(_dst, tagSel, _GSC_FLT);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(float), nil);
	return;

      case _C_DBL:
	(*_tagImp)(_dst, tagSel, _GSC_DBL);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(double), nil);
	return;

#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:
	(*_tagImp)(_dst, tagSel, _GSC_BOOL);
	(*_serImp)(_dst, serSel, (void*)buf, @encode(_Bool), nil);
	return;
#endif

      case _C_VOID:
	[NSException raise: NSInvalidArgumentException
		    format: @"can't encode void item"];

      default:
	[NSException raise: NSInvalidArgumentException
		    format: @"item with unknown type - %s", type];
    }
}

- (void) encodeRootObject: (id)rootObject
{
  if (_encodingRoot)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"encoding root object more than once"];
    }

  _encodingRoot = YES;

  /*
   *	First pass - find conditional objects.
   */
  _initialPass = YES;
  (*_eObjImp)(self, eObjSel, rootObject);

  /*
   *	Second pass - write archive.
   */
  _initialPass = NO;
  (*_eObjImp)(self, eObjSel, rootObject);

  /*
   *	Write sizes of crossref arrays to head of archive.
   */
  [self serializeHeaderAt: _startPos
		  version: [self systemVersion]
		  classes: _clsMap->nodeCount
		  objects: _uIdMap->nodeCount
		 pointers: _ptrMap->nodeCount];

  _encodingRoot = NO;
}

- (void) encodeConditionalObject: (id)anObject
{
  if (_encodingRoot == NO)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"conditionally encoding without root object"];
      return;
    }

  if (_initialPass)
    {
      GSIMapNode	node;

      /*
       *	Conditionally encoding 'nil' is a no-op.
       */
      if (anObject == nil)
	{
	  return;
	}

      /*
       *	If we have already conditionally encoded this object, we can
       *	ignore it this time.
       */
      node = GSIMapNodeForKey(_cIdMap, (GSIMapKey)anObject);
      if (node != 0)
	{
	  return;
	}

      /*
       *	If we have unconditionally encoded this object, we can ignore
       *	it now.
       */
      node = GSIMapNodeForKey(_uIdMap, (GSIMapKey)anObject);
      if (node != 0)
	{
	  return;
	}

      GSIMapAddPair(_cIdMap, (GSIMapKey)anObject, (GSIMapVal)(NSUInteger)0);
    }
  else if (anObject == nil)
    {
      (*_eObjImp)(self, eObjSel, nil);
    }
  else
    {
      GSIMapNode	node;

      if (_repMap->nodeCount)
	{
	  node = GSIMapNodeForKey(_repMap, (GSIMapKey)anObject);
	  if (node)
	    {
	      anObject = (id)node->value.ptr;
	    }
	}

      node = GSIMapNodeForKey(_cIdMap, (GSIMapKey)anObject);
      if (node != 0)
	{
	  (*_eObjImp)(self, eObjSel, nil);
	}
      else
	{
	  (*_eObjImp)(self, eObjSel, anObject);
	}
    }
}

- (void) encodeDataObject: (NSData*)anObject
{
  unsigned	l = [anObject length];

  (*_eValImp)(self, eValSel, @encode(unsigned int), &l);
  if (l)
    {
      const void	*b = [anObject bytes];
      unsigned char	c = 0;			/* Type tag	*/

      /*
       * The type tag 'c' is used to specify an encoding scheme for the
       * actual data - at present we have '0' meaning raw data.  In the
       * future we might want zipped data for instance.
       */
      (*_eValImp)(self, eValSel, @encode(unsigned char), &c);
      [self encodeArrayOfObjCType: @encode(unsigned char)
			    count: l
			       at: b];
    }
}

- (void) encodeObject: (id)anObject
{
  if (anObject == nil)
    {
      if (_initialPass == NO)
	{
	  /*
	   *	Special case - encode a nil pointer as a crossref of zero.
	   */
	  (*_tagImp)(_dst, tagSel, _GSC_ID | _GSC_XREF, _GSC_X_0);
	}
    }
  else
    {
      GSIMapNode	node;

      /*
       *	Substitute replacement object if required.
       */
      node = GSIMapNodeForKey(_repMap, (GSIMapKey)anObject);
      if (node)
	{
	  anObject = (id)node->value.ptr;
	}

      /*
       *	See if the object has already been encoded.
       */
      node = GSIMapNodeForKey(_uIdMap, (GSIMapKey)anObject);

      if (_initialPass)
	{
	  if (node == 0)
	    {
	      /*
	       *	Remove object from map of conditionally encoded objects
	       *	and add it to the map of unconditionay encoded ones.
	       */
	      GSIMapRemoveKey(_cIdMap, (GSIMapKey)anObject);
	      GSIMapAddPair(_uIdMap,
		(GSIMapKey)anObject, (GSIMapVal)(NSUInteger)0);
	      [anObject encodeWithCoder: self];
	    }
	  return;
	}

      if (node == 0 || node->value.nsu == 0)
	{
	  Class	cls;
	  id	obj;

	  if (node == 0)
	    {
	      node = GSIMapAddPair(_uIdMap,
		(GSIMapKey)anObject, (GSIMapVal)(NSUInteger)++_xRefO);
	    }
	  else
	    {
	      node->value.nsu = ++_xRefO;
	    }

	  obj = [anObject replacementObjectForArchiver: self];
	  if (GSObjCIsInstance(obj) == NO)
	    {
	      /*
	       * If the object we have been given is actually a class,
	       * we encode it as a special case.
	       */
	      (*_xRefImp)(_dst, xRefSel, _GSC_CID, node->value.nsu);
	      (*_eValImp)(self, eValSel, @encode(Class), &obj);
	    }
	  else
	    {
	      cls = [obj classForArchiver];
	      if (_namMap->nodeCount)
		{
		  GSIMapNode	n;

		  n = GSIMapNodeForKey(_namMap, (GSIMapKey)cls);

		  if (n)
		    {
		      cls = (Class)n->value.ptr;
		    }
		}
	      (*_xRefImp)(_dst, xRefSel, _GSC_ID, node->value.nsu);
	      (*_eValImp)(self, eValSel, @encode(Class), &cls);
	      [obj encodeWithCoder: self];
	    }
	}
      else
	{
	  (*_xRefImp)(_dst, xRefSel, _GSC_ID | _GSC_XREF, node->value.nsu);
	}
    }
}

/**
 *  Returns whatever data has been encoded thus far.
 */
- (NSMutableData*) archiverData
{
  return _data;
}

/**
 *  Returns substitute class used to encode objects of given class.  This
 *  would have been set through an earlier call to
 *  [NSArchiver -encodeClassName:intoClassName:].
 */
- (NSString*) classNameEncodedForTrueClassName: (NSString*)trueName
{
  if (_namMap->nodeCount)
    {
      GSIMapNode	node;
      Class		c;

      c = objc_lookUpClass([trueName cString]);
      node = GSIMapNodeForKey(_namMap, (GSIMapKey)c);
      if (node)
	{
	  c = (Class)node->value.ptr;
	  return [NSString stringWithUTF8String: class_getName(c)];
	}
    }
  return trueName;
}

/**
 *  Specify substitute class used in archiving objects of given class.  This
 *  class is written to the archive as the class to use for restoring the
 *  object, instead of what is returned from [NSObject -classForArchiver].
 *  This can be used to provide backward compatibility across class name
 *  changes.  The object is still encoded by calling
 *  <code>encodeWithCoder:</code> as normal.
 */
- (void) encodeClassName: (NSString*)trueName
	   intoClassName: (NSString*)inArchiveName
{
  GSIMapNode	node;
  Class		tc;
  Class		ic;

  tc = objc_lookUpClass([trueName cString]);
  if (tc == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Can't find class '%@'.", trueName];
    }
  ic = objc_lookUpClass([inArchiveName cString]);
  if (ic == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Can't find class '%@'.", inArchiveName];
    }
  node = GSIMapNodeForKey(_namMap, (GSIMapKey)tc);
  if (node == 0)
    {
      GSIMapAddPair(_namMap, (GSIMapKey)(void*)tc, (GSIMapVal)(void*)ic);
    }
  else
    {
      node->value.ptr = (void*)ic;
    }
}

/**
 *  Set encoder to write out newObject in place of object.
 */
- (void) replaceObject: (id)object
	    withObject: (id)newObject
{
  GSIMapNode	node;

  if (object == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"attempt to remap nil"];
    }
  if (newObject == 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"attempt to remap object to nil"];
    }
  node = GSIMapNodeForKey(_repMap, (GSIMapKey)object);
  if (node == 0)
    {
      GSIMapAddPair(_repMap, (GSIMapKey)object, (GSIMapVal)newObject);
    }
  else
    {
      node->value.ptr = (void*)newObject;
    }
}
@end



/**
 *  Category for compatibility with old GNUstep encoding.
 */
@implementation	NSArchiver (GNUstep)

/**
 *  Allow reuse of archiver (clears class substitution maps, etc.) but
 *  do not clear out current serialized data.
 */
- (void) resetArchiver
{
  if (_clsMap)
    {
      GSIMapCleanMap(_clsMap);
      if (_cIdMap)
	{
	  GSIMapCleanMap(_cIdMap);
	}
      if (_uIdMap)
	{
	  GSIMapCleanMap(_uIdMap);
	}
      if (_ptrMap)
	{
	  GSIMapCleanMap(_ptrMap);
	}
      if (_namMap)
	{
	  GSIMapCleanMap(_namMap);
	}
      if (_repMap)
	{
	  GSIMapCleanMap(_repMap);
	}
    }
  _encodingRoot = NO;
  _initialPass = NO;
  _xRefC = 0;
  _xRefO = 0;
  _xRefP = 0;

  /*
   *	Write dummy header
   */
  _startPos = [_data length];
  [self serializeHeaderAt: _startPos
		  version: [self systemVersion]
		  classes: 0
		  objects: 0
		 pointers: 0];
}

/**
 *  Returns YES.
 */
- (BOOL) directDataAccess
{
  return YES;
}

/**
 *  Writes out header for GNUstep archive format.
 */
- (void) serializeHeaderAt: (unsigned)positionInData
		   version: (unsigned)systemVersion
		   classes: (unsigned)classCount
		   objects: (unsigned)objectCount
		  pointers: (unsigned)pointerCount
{
  unsigned	headerLength = strlen(PREFIX)+36;
  char		header[headerLength+1];
  unsigned	dataLength = [_data length];

  snprintf(header, sizeof(header), "%s%08x:%08x:%08x:%08x:",
    PREFIX, systemVersion, classCount, objectCount, pointerCount);

  if (positionInData + headerLength <= dataLength)
    {
      [_data replaceBytesInRange: NSMakeRange(positionInData, headerLength)
		      withBytes: header];
    }
  else if (positionInData == dataLength)
    {
      [_data appendBytes: header length: headerLength];
    }
  else
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"serializeHeader:at: bad location"];
    }
}

@end

