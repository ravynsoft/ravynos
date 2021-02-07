/** Implementation of NSUnarchiver for GNUstep
   Copyright (C) 1998 Free Software Foundation, Inc.

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

   <title>NSUnarchiver class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#define	EXPOSE_NSUnarchiver_IVARS	1
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSByteOrder.h"

/*
 *	Setup for inline operation of arrays.
 */
#define	GSI_ARRAY_NO_RETAIN	1
#define	GSI_ARRAY_NO_RELEASE	1
#define	GSI_ARRAY_TYPES	GSUNION_OBJ|GSUNION_SEL|GSUNION_PTR

#include "GNUstepBase/GSIArray.h"

#define	_IN_NSUNARCHIVER_M
#import "Foundation/NSArchiver.h"
#undef	_IN_NSUNARCHIVER_M

#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSData.h"
#import "Foundation/NSArray.h"

@class NSDataMalloc;
@interface NSDataMalloc : NSObject	// Help the compiler
@end

static const char*
typeToName1(char type)
{
  switch (type)
    {
      case _C_CLASS:	return "class";
      case _C_ID:	return "object";
      case _C_SEL:	return "selector";
      case _C_CHR:	return "char";
      case _C_UCHR:	return "unsigned char";
      case _C_SHT:	return "short";
      case _C_USHT:	return "unsigned short";
      case _C_INT:	return "int";
      case _C_UINT:	return "unsigned int";
      case _C_LNG:	return "long";
      case _C_ULNG:	return "unsigned long";
      case _C_LNG_LNG:	return "long long";
      case _C_ULNG_LNG:	return "unsigned long long";
      case _C_FLT:	return "float";
      case _C_DBL:	return "double";
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:	return "_Bool";
#endif
      case _C_PTR:	return "pointer";
      case _C_CHARPTR:	return "cstring";
      case _C_ARY_B:	return "array";
      case _C_STRUCT_B:	return "struct";
      default:
	{
	  static char	buf1[32];
	  static char	buf2[32];
	  static char	*bufptr = buf1;

	  if (bufptr == buf1)
	    {
	      bufptr = buf2;
	    }
	  else
	    {
	      bufptr = buf1;
	    }
	  snprintf(bufptr, 32, "unknown type info - 0x%x", type);
	  return bufptr;
	}
    }
}

static const char*
typeToName2(char type)
{
  switch (type & _GSC_MASK)
    {
      case _GSC_CID:	return "class (encoded as id)";
      case _GSC_CLASS:	return "class";
      case _GSC_ID:	return "object";
      case _GSC_SEL:	return "selector";
      case _GSC_CHR:	return "char";
      case _GSC_UCHR:	return "unsigned char";
      case _GSC_SHT:	return "short";
      case _GSC_USHT:	return "unsigned short";
      case _GSC_INT:	return "int";
      case _GSC_UINT:	return "unsigned int";
      case _GSC_LNG:	return "long";
      case _GSC_ULNG:	return "unsigned long";
      case _GSC_LNG_LNG:	return "long long";
      case _GSC_ULNG_LNG:	return "unsigned long long";
      case _GSC_FLT:	return "float";
      case _GSC_DBL:	return "double";
      case _GSC_BOOL:	return "_Bool";
      case _GSC_PTR:	return "pointer";
      case _GSC_CHARPTR:	return "cstring";
      case _GSC_ARY_B:	return "array";
      case _GSC_STRUCT_B:	return "struct";
      default:
	{
	  static char	buf1[32];
	  static char	buf2[32];
	  static char	*bufptr = buf1;

	  if (bufptr == buf1)
	    {
	      bufptr = buf2;
	    }
	  else
	    {
	      bufptr = buf1;
	    }
	  snprintf(bufptr, 32, "unknown type info - 0x%x", type);
	  return bufptr;
	}
    }
}

/*
 *	There are thirtyone possible basic types.  We reserve a type of zero
 *	to mean that no information is specified.  The slots in this array
 *	MUST correspond to the definitions in NSData.h
 */
static char	type_map[32] = {
  0,
  _C_CHR,
  _C_UCHR,
  _C_SHT,
  _C_USHT,
  _C_INT,
  _C_UINT,
  _C_LNG,
  _C_ULNG,
  _C_LNG_LNG,
  _C_ULNG_LNG,
  _C_FLT,
  _C_DBL,
#if __GNUC__ > 2 && defined(_C_BOOL)
  _C_BOOL,
#else
  0,
#endif
  0,
  0,
  _C_ID,
  _C_CLASS,
  _C_SEL,
  _C_PTR,
  _C_CHARPTR,
  _C_ARY_B,
  _C_STRUCT_B,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};


static inline BOOL
typeCheck(char t1, char t2)
{
  if (type_map[(t2 & _GSC_MASK)] != t1)
    {
/*
 * HACK ... allow int/long/longlong types to be used interchangably
 * as the ObjC compiler currently uses quadword (q/Q) encoding for
 * integer types on some 64bit systems, so the i/l/q/I/L/Q encodings
 * can vary.
 */
      char	c = type_map[(t2 & _GSC_MASK)];
      char      s1;
      char      s2;

      switch (t1)
        {
          case _C_SHT:  s1 = _GSC_S_SHT; break;
          case _C_USHT:  s1 = _GSC_S_SHT; break;
          case _C_INT:  s1 = _GSC_S_INT; break;
          case _C_UINT:  s1 = _GSC_S_INT; break;
          case _C_LNG:  s1 = _GSC_S_LNG; break;
          case _C_ULNG:  s1 = _GSC_S_LNG; break;
          case _C_LNG_LNG:  s1 = _GSC_S_LNG_LNG; break;
          case _C_ULNG_LNG:  s1 = _GSC_S_LNG_LNG; break;
          default:      s1 = 0;
        }

      switch (t2)
        {
          case _C_SHT:  s2 = _GSC_S_SHT; break;
          case _C_USHT:  s2 = _GSC_S_SHT; break;
          case _C_INT:  s2 = _GSC_S_INT; break;
          case _C_UINT:  s2 = _GSC_S_INT; break;
          case _C_LNG:  s2 = _GSC_S_LNG; break;
          case _C_ULNG:  s2 = _GSC_S_LNG; break;
          case _C_LNG_LNG:  s2 = _GSC_S_LNG_LNG; break;
          case _C_ULNG_LNG:  s2 = _GSC_S_LNG_LNG; break;
          default:      s2 = 0;
        }

      if ((c == _C_INT || c == _C_LNG || c == _C_LNG_LNG)
        && (t1 == _C_INT || t1 == _C_LNG || t1 == _C_LNG_LNG))
        return s1 == s2 ? YES : NO;

      if ((c == _C_UINT || c == _C_ULNG || c == _C_ULNG_LNG)
        && (t1 == _C_UINT || t1 == _C_ULNG || t1 == _C_ULNG_LNG))
        return s1 == s2 ? YES : NO;

/* HACK also allow float and double to be used interchangably as MacOS-X
 * intorduced CGFloat, which may be aither a float or a double.
 */
      if ((c == _C_FLT || c == _C_DBL) && (t1 == _C_FLT || t1 == _C_DBL))
	return NO;

      [NSException raise: NSInternalInconsistencyException
		  format: @"expected %s and got %s",
		    typeToName1(t1), typeToName2(t2)];
    }
  return YES;
}

#define	PREFIX		"GNUstep archive"

static SEL desSel;
static SEL tagSel;
static SEL dValSel;

@interface	NSUnarchiverClassInfo : NSObject
{
@public
  NSString	*original;
  NSString	*name;
  Class		class;
}
+ (id) newWithName: (NSString*)n;
- (void) mapToClass: (Class)c withName: (NSString*)name;
@end

@implementation	NSUnarchiverClassInfo
+ (id) newWithName: (NSString*)n
{
  NSUnarchiverClassInfo	*info;

  info = (NSUnarchiverClassInfo*)NSAllocateObject(self,0,NSDefaultMallocZone());
  if (info)
    {
      info->original = [n copyWithZone: NSDefaultMallocZone()];
    }
  return info;
}
- (void) dealloc
{
  RELEASE(original);
  if (name)
    {
      RELEASE(name);
    }
  NSDeallocateObject(self);
  GSNOSUPERDEALLOC;
}
- (void) mapToClass: (Class)c withName: (NSString*)n
{
  ASSIGN(name, n);
  class = c;
}
@end

/*
 *	Dictionary used by NSUnarchiver class to keep track of
 *	NSUnarchiverClassInfo objects used to map classes by name when
 *	unarchiving.
 */
static NSMutableDictionary	*clsDict;	/* Class information	*/

@interface	NSUnarchiverObjectInfo : NSUnarchiverClassInfo
{
@public
  NSInteger	version;
  NSUnarchiverClassInfo	*overrides;
}
@end

static inline Class
mapClassObject(NSUnarchiverObjectInfo *info)
{
  if (info->overrides == nil)
    {
      info->overrides = [clsDict objectForKey: info->original];
    }
  if (info->overrides)
    {
      return info->overrides->class;
    }
  else
    {
      return info->class;
    }
}

static inline NSString*
mapClassName(NSUnarchiverObjectInfo *info)
{
  if (info->overrides == nil)
    {
      info->overrides = [clsDict objectForKey: info->original];
    }
  if (info->overrides)
    {
      return info->overrides->name;
    }
  else
   {
      return info->name;
   }
}

@implementation	NSUnarchiverObjectInfo
@end

/**
 * <p>This class reconstructs objects from an archive.</p><br />
 * <strong>Re-using the archiver</strong>
 * <p>
 *   The -resetUnarchiverWithData:atIndex: method lets you re-use
 *   the archive to decode a new data object or, in conjunction
 *   with the 'cursor' method (which reports the current decoding
 *   position in the archive), decode a second archive that exists
 *   in the data object after the first one.
 * </p>
 * <strong>Subclassing with different input format.</strong><br /><br />
 * <code>NSUnarchiver</code> normally reads directly from an [NSData]
 * object using the methods -
 * <deflist>
 *   <term>-deserializeTypeTag:andCrossRef:atCursor:</term>
 *   <desc>
 *     to decode type tags for data items, the tag is the
 *     first byte of the character encoding string for the
 *     data type (as provided by '@encode(xxx)'), possibly
 *     with the top bit set to indicate that what follows is
 *     a crossreference to an item already encoded.<br />
 *     Also decode a crossreference number either to identify the
 *     following item, or to refer to a previously encoded item.
 *     Objects, Classes, Selectors, CStrings and Pointer items
 *     have crossreference encoding, other types do not.<br />
 *   </desc>
 *   <term>[NSData-deserializeDataAt:ofObjCType:atCursor:context:]</term>
 *   <desc>
 *     to decode all other information.
 *   </desc>
 * </deflist>
 * <p>
 *   <code>NSUnarchiver</code> normally uses other [NSData] methods to read
 *   the archive header information from within the method:
 *   [-deserializeHeaderAt:version:classes:objects:pointers:]
 *   to read a fixed size header including archiver version
 *   (obtained by <code>[self systemVersion]</code>) and crossreference
 *   table sizes.
 * </p>
 * <p>
 *   To subclass <code>NSUnarchiver</code>, you must implement your own
 *   versions of the four methods above, and override the 'directDataAccess'
 *   method to return NO so that the archiver knows to use your serialization
 *   methods rather than those in the [NSData] object.
 * </p>
 */
@implementation NSUnarchiver

static Class NSDataMallocClass;
static unsigned	encodingVersion;

+ (void) initialize
{
  if ([self class] == [NSUnarchiver class])
    {
      NSArchiver	*archiver = [NSArchiver new];

      encodingVersion = [archiver systemVersion];
      [archiver release];
      desSel = @selector(deserializeDataAt:ofObjCType:atCursor:context:);
      tagSel = @selector(deserializeTypeTag:andCrossRef:atCursor:);
      dValSel = @selector(decodeValueOfObjCType:at:);
      clsDict = [[NSMutableDictionary alloc] initWithCapacity: 200];
      NSDataMallocClass = [NSDataMalloc class];
      
    }
}

/**
 *  Creates an NSUnarchiver to read from anObject and returns result of sending
 *  [NSCoder -decodeObject] to it.
 */
+ (id) unarchiveObjectWithData: (NSData*)anObject
{
  NSUnarchiver	*unarchiver;
  id		obj;

  unarchiver = [[self alloc] initForReadingWithData: anObject];
  NS_DURING
    {
      obj = [unarchiver decodeObject];
    }
  NS_HANDLER
    {
      obj = nil;
      RELEASE(unarchiver);
      [localException raise];
    }
  NS_ENDHANDLER
  RELEASE(unarchiver);

  return obj;
}

/**
 *  Creates an NSUnarchiver to read from path and returns result of sending
 *  [NSCoder -decodeObject] to it.
 */
+ (id) unarchiveObjectWithFile: (NSString*)path
{
  NSData	*d = [NSDataMallocClass dataWithContentsOfFile: path];

  if (d != nil)
    {
      return [self unarchiveObjectWithData: d];
    }
  return nil;
}

- (void) dealloc
{
  RELEASE(data);
  RELEASE(objSave);
  RELEASE(objDict);
  if (clsMap)
    {
      NSZone	*z = clsMap->zone;

      GSIArrayClear(clsMap);
      GSIArrayClear(objMap);
      GSIArrayClear(ptrMap);
      NSZoneFree(z, (void*)clsMap);
    }
  [super dealloc];
}

/**
 *  Set up to read objects from data buffer anObject.
 */
- (id) initForReadingWithData: (NSData*)anObject
{
  if (anObject == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"nil data passed to initForReadingWithData:"];
    }

  self = [super init];
  if (self)
    {
      dValImp = [self methodForSelector: dValSel];
      zone = [self zone];
      /*
       *	If we are not deserializing directly from the data object
       *	then we cache our own deserialisation methods.
       */
      if ([self directDataAccess] == NO)
	{
	  src = self;		/* Default object to handle serialisation */
	  desImp = [src methodForSelector: desSel];
	  tagImp = (void (*)(id, SEL, unsigned char*, unsigned*, unsigned*))
	      [src methodForSelector: tagSel];
	}
      /*
       *	objDict is a dictionary of objects for mapping classes of
       *	one name to be those of another name!  It also handles
       *	keeping track of the version numbers that the classes were
       *	encoded with.
       */
      objDict = [[NSMutableDictionary allocWithZone: zone]
				   initWithCapacity: 200];
      /*
       *	objSave is an array used purely to ensure that objects
       *	we decode persist until the end of the decoding.
       */
      objSave = [[NSMutableArray allocWithZone: zone]
			      initWithCapacity: 200];

      NS_DURING
	{
	  [self resetUnarchiverWithData: anObject atIndex: 0];
	}
      NS_HANDLER
	{
	  DESTROY(self);
	  [localException raise];
	}
      NS_ENDHANDLER
    }
  return self;
}

- (void) decodeArrayOfObjCType: (const char*)type
			 count: (NSUInteger)expected
			    at: (void*)buf
{
  NSUInteger	i;
  NSUInteger	offset = 0;
  unsigned int	size = (unsigned int)objc_sizeof_type(type);
  unsigned char	info;
  unsigned char	ainfo;
  unsigned char	amask;
  NSUInteger	count;

  (*tagImp)(src, tagSel, &info, 0, &cursor);
  if ([self systemVersion] == 12402)
    {
      uint8_t	c;

      /* Unpack variable length count.
       */
      count = 0;
      for (;;)
	{
	  if (count * 128 < count)
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"overflow in array count"];
	    }
	  count *= 128;
	  (*desImp)(src, desSel, &c, @encode(uint8_t), &cursor, nil);
	  if (c & 128)
	    {
	      count += (c & 127);
	    }
	  else
	    {
	      count += c;
	      break;
	    }
	}
    }
  else
    {
      uint32_t	c;

      (*desImp)(src, desSel, &c, @encode(uint32_t), &cursor, nil);
      count = c;
      if (0xffffffff == c)
	{
	  (*desImp)(src, desSel, &count, @encode(NSUInteger), &cursor, nil);
	}
    }
  if (info != _GSC_ARY_B)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"expected array and got %s", typeToName2(info)];
    }
  if (count != expected)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"expected array count %"PRIuPTR" and got %"PRIuPTR"",
			expected, count];
    }

  switch (*type)
    {
      case _C_ID:	info = _GSC_NONE; break;
      case _C_CHR:	info = _GSC_CHR; break;
      case _C_UCHR:	info = _GSC_UCHR; break;
      case _C_SHT:	info = _GSC_SHT; break;
      case _C_USHT:	info = _GSC_USHT; break;
      case _C_INT:	info = _GSC_INT; break;
      case _C_UINT:	info = _GSC_UINT; break;
      case _C_LNG:	info = _GSC_LNG; break;
      case _C_ULNG:	info = _GSC_ULNG; break;
      case _C_LNG_LNG:	info = _GSC_LNG_LNG; break;
      case _C_ULNG_LNG:	info = _GSC_ULNG_LNG; break;
      case _C_FLT:	info = _GSC_FLT; break;
      case _C_DBL:	info = _GSC_DBL; break;
#if __GNUC__ > 2 && defined(_C_BOOL)
      case _C_BOOL:	info = _GSC_BOOL; break;
#endif
      default:		info = _GSC_NONE; break;
    }

  if (info == _GSC_NONE)
    {
      for (i = 0; i < count; i++)
	{
	  (*dValImp)(self, dValSel, type, (char*)buf + offset);
	  offset += size;
	}
      return;
    }

  (*tagImp)(src, tagSel, &ainfo, 0, &cursor);
  amask = (ainfo & _GSC_MASK);

  /* If we have a perfect type match or we are coding a class as an ID,
   * we can just decode the array simply.
   */
  if (info == amask)
    {
      for (i = 0; i < count; i++)
	{
	  (*desImp)(src, desSel, (char*)buf + offset, type, &cursor, nil);
	  offset += size;
	}
      return;
    }

  /* This will raise an exception if the types don't match at all.
   */
  typeCheck(*type, ainfo);

  /* We fall through to here only when we have to decode a value
   * whose natural size on this system is not the same as on the
   * machine on which the archive was created.
   */

  if (*type == _C_FLT)
    {
      for (i = 0; i < count; i++)
	{
	  double	d;

	  (*desImp)(src, desSel, &d, @encode(double), &cursor, nil);
	  *(float*)(buf + offset) = (float)d;
	  offset += size;
	}
    }
  else if (*type == _C_DBL)
    {
      for (i = 0; i < count; i++)
	{
	  float		f;

	  (*desImp)(src, desSel, &f, @encode(float), &cursor, nil);
	  *(double*)(buf + offset) = (double)f;
	  offset += size;
	}
    }
  else if (*type == _C_SHT
    || *type == _C_INT
    || *type == _C_LNG
    || *type == _C_LNG_LNG)
    {
      int64_t	big;
      int64_t	max;
      int64_t	min;

      switch (size)
	{
	  case 1:
	    max = INT8_MAX;
	    min = INT8_MIN;
	    break;
	  case 2:
	    max = INT16_MAX;
	    min = INT16_MAX;
	    break;
	  case 4:
	    max = INT32_MAX;
	    min = INT32_MIN;
	    break;
	  default:
	    max = INT64_MAX;
	    min = INT64_MIN;
	}

      for (i = 0; i < count; i++)
	{
	  void	*address = (void*)buf + offset;

	  switch (ainfo & _GSC_SIZE)
	    {
	      case _GSC_I16:	/* Encoded as 16-bit	*/
		{
		  int16_t	val;

		  (*desImp)(src, desSel, &val, @encode(int16_t), &cursor, nil);
		  big = val;
		  break;
		}

	      case _GSC_I32:	/* Encoded as 32-bit	*/
		{
		  int32_t	val;

		  (*desImp)(src, desSel, &val, @encode(int32_t), &cursor, nil);
		  big = val;
		  break;
		}

	      case _GSC_I64:	/* Encoded as 64-bit	*/
		{
		  (*desImp)(src, desSel, &big, @encode(int64_t), &cursor, nil);
		  break;
		}

              default:
                [NSException raise: NSGenericException
                            format: @"invalid size in archive"];
	    }
	  /*
	   * Now we copy from the big value to the destination location.
	   */
	  switch (size)
	    {
	      case 1:
		*(int8_t*)address = (int8_t)big;
		break;
	      case 2:
		*(int16_t*)address = (int16_t)big;
		break;
	      case 4:
		*(int32_t*)address = (int32_t)big;
		break;
	      default:
		*(int64_t*)address = big;
	    }
	  if (big < min || big > max)
	    {
	      NSLog(@"Lost information converting large decoded value");
	    }
	  offset += size;
	}
    }
  else
    {
      uint64_t  big;
      uint64_t	max;

      switch (size)
	{
	  case 1:
	    max = UINT8_MAX;
	    break;
	  case 2:
	    max = UINT16_MAX;
	    break;
	  case 4:
	    max = UINT32_MAX;
	    break;
	  default:
	    max = UINT64_MAX;
	}

      for (i = 0; i < count; i++)
	{
	  void	*address = (void*)buf + offset;

	  switch (info & _GSC_SIZE)
	    {
	      case _GSC_I16:	/* Encoded as 16-bit	*/
		{
		  uint16_t	val;

		  (*desImp)(src, desSel, &val, @encode(uint16_t), &cursor, nil);
		  big = val;
		  break;
		}

	      case _GSC_I32:	/* Encoded as 32-bit	*/
		{
		  uint32_t	val;

		  (*desImp)(src, desSel, &val, @encode(uint32_t), &cursor, nil);
		  big = val;
		  break;
		}

	      case _GSC_I64:	/* Encoded as 64-bit	*/
		{
		  (*desImp)(src, desSel, &big, @encode(uint64_t), &cursor, nil);
		  break;
		}

              default:
                [NSException raise: NSGenericException
                            format: @"invalid size in archive"];
	    }
	  /*
	   * Now we copy from the big value to the destination location.
	   */
	  switch (size)
	    {
	      case 1:
		*(uint8_t*)address = (uint8_t)big;
		break;
	      case 2:
		*(uint16_t*)address = (uint16_t)big;
		break;
	      case 4:
		*(uint32_t*)address = (uint32_t)big;
		break;
	      case 8:
		*(uint64_t*)address = big;
	    }
	  if (big > max)
	    {
	      NSLog(@"Lost information converting large decoded value");
	    }
	  offset += size;
	}
    }
}

static inline int
scalarSize(char type)
{
  switch (type)
    {
      case _C_SHT:
      case _C_USHT:     return _GSC_S_SHT;
      case _C_INT:
      case _C_UINT:	return _GSC_S_INT;
      case _C_LNG:
      case _C_ULNG:	return _GSC_S_LNG;
      case _C_LNG_LNG:
      case _C_ULNG_LNG:	return _GSC_S_LNG_LNG;
      default:
        [NSException raise: NSInvalidArgumentException
                    format: @"scalarSize() called with non-scalar type"];
    }
  return -1;
}

- (void) decodeValueOfObjCType: (const char*)type
			    at: (void*)address
{
  unsigned	xref;
  unsigned char	info;

  (*tagImp)(src, tagSel, &info, &xref, &cursor);

  switch (info & _GSC_MASK)
    {
      case _GSC_ID:
	{
	  id		obj;

	  typeCheck(*type, _GSC_ID);
	  /*
	   *	Special case - a zero crossref value size is a nil pointer.
	   */
	  if ((info & _GSC_SIZE) == 0)
	    {
	      obj = nil;
	    }
	  else
	    {
	      if (info & _GSC_XREF)
		{
		  if (xref >= GSIArrayCount(objMap))
		    {
		      [NSException raise: NSInternalInconsistencyException
				  format: @"object crossref missing - %d",
					xref];
		    }
		  obj = GSIArrayItemAtIndex(objMap, xref).obj;
		  /*
		   *	If it's a cross-reference, we need to retain it in
		   *	order to give the appearance that it's actually a
		   *	new object.
		   */
		  IF_NO_GC(RETAIN(obj));
		}
	      else
		{
		  Class	c;
		  id	rep;

		  if (xref != GSIArrayCount(objMap))
		    {
		      [NSException raise: NSInternalInconsistencyException
				  format: @"extra object crossref - %d",
					xref];
		    }
		  (*dValImp)(self, dValSel, @encode(Class), &c);

		  if (c == 0)
		    {
		      [NSException raise: NSInternalInconsistencyException
				  format: @"decoded nil class"];
		    }
		  obj = [c allocWithZone: zone];
		  GSIArrayAddItem(objMap, (GSIArrayItem)obj);

		  rep = [obj initWithCoder: self];
		  if (rep != obj)
		    {
		      obj = rep;
		      GSIArraySetItemAtIndex(objMap, (GSIArrayItem)obj, xref);
		    }

		  rep = [obj awakeAfterUsingCoder: self];
		  if (rep != obj)
		    {
		      obj = rep;
		      GSIArraySetItemAtIndex(objMap, (GSIArrayItem)obj, xref);
		    }
		  /*
		   * The objMap does not retain objects, so in order to
		   * be sure that a decoded object is not deallocated by
		   * anything before it is needed (because it is decoded
		   * later as a cross reference) we store it in objSave.
		   */
		  if (obj != nil)
		    {
		      [objSave addObject: obj];
		    }
		}
	    }
	  *(id*)address = obj;
	  return;
	}

      case _GSC_CLASS:
	{
	  Class		c;
	  NSUnarchiverObjectInfo	*classInfo;
	  Class		dummy;

	  if (*type != _C_ID)
	    {
	      typeCheck(*type, _GSC_CLASS);
	    }
	  /*
	   *	Special case - a zero crossref value size is a nil pointer.
	   */
	  if ((info & _GSC_SIZE) == 0)
	    {
	      *(SEL*)address = 0;
	      return;
	    }
	  if (info & _GSC_XREF)
	    {
	      if (xref >= GSIArrayCount(clsMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"class crossref missing - %d", xref];
		}
	      classInfo = (NSUnarchiverObjectInfo*)
		GSIArrayItemAtIndex(clsMap, xref).obj;
	      *(Class*)address = mapClassObject(classInfo);
	      return;
	    }
	  while ((info & _GSC_MASK) == _GSC_CLASS)
	    {
	      unsigned	cver;
	      NSString	*className;
	      uint16_t	nameLength;

	      if (xref != GSIArrayCount(clsMap))
		{
		  [NSException raise: NSInternalInconsistencyException
				format: @"extra class crossref - %d", xref];
		}

	      /*
	       * A class is encoded as a 16-bit length, a sequence of
	       * characters providing its name, then a version number.
	       */
	      (*desImp)(src, desSel, &nameLength, @encode(uint16_t),
		&cursor, nil);
	      if (nameLength == 0)
		{
		  className = nil;
		}
	      else
		{
		  char	name[nameLength+1];

		  [src deserializeBytes: name
				 length: nameLength
			       atCursor: &cursor];
		  name[nameLength] = '\0';
		  className = [[NSString alloc] initWithUTF8String: name];
		}
	      (*desImp)(src, desSel, &cver, @encode(unsigned), &cursor, nil);
	      if (className == 0)
		{
		  NSLog(@"[%s %s] decoded nil class name",
		    class_getName([self class]), sel_getName(_cmd));
		  className = @"_NSUnarchiverUnknownClass";
		}
	      classInfo = [objDict objectForKey: className];
	      if (nil == classInfo)
		{
		  classInfo = [NSUnarchiverObjectInfo newWithName: className];
		  c = NSClassFromString(className);
		  /*
		   * Show a warning, if the class name that's being used to
		   * build the class causes NSClassFromString to return nil.
		   * This means that the class is unknown to the runtime.
		   */
		  if (c == nil)
		    {
		      NSLog(@"Unable to find class named '%@'", className);
		    }
		  [classInfo mapToClass: c withName: className];
		  [objDict setObject: classInfo forKey: className];
		  RELEASE(classInfo);
		}
	      RELEASE(className);
	      classInfo->version = (NSInteger)cver;
	      GSIArrayAddItem(clsMap, (GSIArrayItem)((id)classInfo));
	      *(Class*)address = mapClassObject(classInfo);
	      /*
	       *	Point the address to a dummy location and read the
	       *	next tag - if it is another class, loop to get it.
	       */
	      address = &dummy;
	      (*tagImp)(src, tagSel, &info, &xref, &cursor);
	    }
	  if (info != _GSC_NONE)
	    {
	      [NSException raise: NSInternalInconsistencyException
			  format: @"class list improperly terminated"];
	    }
	  return;
	}

      case _GSC_SEL:
	{
	  SEL		sel;

	  typeCheck(*type, _GSC_SEL);
	  /*
	   *	Special case - a zero crossref value size is a nil pointer.
	   */
	  if ((info & _GSC_SIZE) == 0)
	    {
	      *(SEL*)address = 0;
	      return;
	    }
	  if (info & _GSC_XREF)
	    {
	      if (xref >= GSIArrayCount(ptrMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"sel crossref missing - %d", xref];
		}
	      sel = GSIArrayItemAtIndex(ptrMap, xref).sel;
	    }
	  else
	    {
	      if (xref != GSIArrayCount(ptrMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"extra sel crossref - %d", xref];
		}
	      (*desImp)(src, desSel, &sel, @encode(SEL), &cursor, nil);
	      GSIArrayAddItem(ptrMap, (GSIArrayItem)sel);
	    }
	  *(SEL*)address = sel;
	  return;
	}

      case _GSC_ARY_B:
	{
	  int	count;

	  typeCheck(*type, _GSC_ARY_B);
	  count = atoi(++type);
	  while (isdigit(*type))
	    {
	      type++;
	    }
	  [self decodeArrayOfObjCType: type count: count at: address];
	  return;
	}

      case _GSC_STRUCT_B:
	{
	  struct objc_struct_layout layout;

	  typeCheck(*type, _GSC_STRUCT_B);
	  objc_layout_structure (type, &layout);
	  while (objc_layout_structure_next_member (&layout))
	    {
	      unsigned		offset;
	      unsigned		align;
	      const char	*ftype;

	      objc_layout_structure_get_info (&layout, &offset, &align, &ftype);

	      (*dValImp)(self, dValSel, ftype, (char*)address + offset);
	    }
	  return;
	}

      case _GSC_PTR:
	{
	  typeCheck(*type, _GSC_PTR);
	  /*
	   *	Special case - a zero crossref value size is a nil pointer.
	   */
	  if ((info & _GSC_SIZE) == 0)
	    {
	      *(void**)address = 0;
	      return;
	    }
	  if (info & _GSC_XREF)
	    {
	      if (xref >= GSIArrayCount(ptrMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"ptr crossref missing - %d", xref];
		}
	      *(void**)address = GSIArrayItemAtIndex(ptrMap, xref).ptr;
	    }
	  else
	    {
	      unsigned	size;

	      if (GSIArrayCount(ptrMap) != xref)
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"extra ptr crossref - %d", xref];
		}

	      /*
	       *	Allocate memory for object to be decoded into and
	       *	add it to the crossref map.
	       */
	      size = objc_sizeof_type(++type);
	      *(void**)address = GSAutoreleasedBuffer(size);
	      GSIArrayAddItem(ptrMap, (GSIArrayItem)*(void**)address);

	      /*
	       *	Decode value and add memory to map for crossrefs.
	       */
	      (*dValImp)(self, dValSel, type, *(void**)address);
	    }
	  return;
	}

      case _GSC_CHARPTR:
	{
	  typeCheck(*type, _GSC_CHARPTR);
	  /*
	   *	Special case - a zero crossref value size is a nil pointer.
	   */
	  if ((info & _GSC_SIZE) == 0)
	    {
	      *(char**)address = 0;
	      return;
	    }
	  if (info & _GSC_XREF)
	    {
	      if (xref >= GSIArrayCount(ptrMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"string crossref missing - %d", xref];
		}
	      *(char**)address = GSIArrayItemAtIndex(ptrMap, xref).str;
	    }
	  else
	    {
	      if (xref != GSIArrayCount(ptrMap))
		{
		  [NSException raise: NSInternalInconsistencyException
			      format: @"extra string crossref - %d", xref];
		}
	      (*desImp)(src, desSel, address, @encode(char*), &cursor, nil);
	      GSIArrayAddItem(ptrMap, (GSIArrayItem)*(void**)address);
	    }
	  return;
	}

      case _GSC_CHR:
      case _GSC_UCHR:
	/* Encoding of chars is not consistant across platforms, so we
	   loosen the type checking a little */
	if (*type != type_map[_GSC_CHR] && *type != type_map[_GSC_UCHR])
	  {
	    [NSException raise: NSInternalInconsistencyException
		        format: @"expected %s and got %s",
		    typeToName1(*type), typeToName2(info)];
	  }
	(*desImp)(src, desSel, address, type, &cursor, nil);
	return;

      case _GSC_SHT:
      case _GSC_USHT:
	if (YES == typeCheck(*type, info & _GSC_MASK)
	  && (info & _GSC_SIZE) == scalarSize(*type))
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	    return;
	  }
	break;

      case _GSC_INT:
      case _GSC_UINT:
	if (YES == typeCheck(*type, info & _GSC_MASK)
	  &&  (info & _GSC_SIZE) == scalarSize(*type))
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	    return;
	  }
	break;

      case _GSC_LNG:
      case _GSC_ULNG:
	if (YES == typeCheck(*type, info & _GSC_MASK)
	  && (info & _GSC_SIZE) == scalarSize(*type))
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	    return;
	  }
	break;

      case _GSC_LNG_LNG:
      case _GSC_ULNG_LNG:
	if (YES == typeCheck(*type, info & _GSC_MASK)
	  && (info & _GSC_SIZE) == scalarSize(*type))
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	    return;
	  }
	break;

      case _GSC_FLT:
	if (YES == typeCheck(*type, _GSC_FLT)
	  && *type == _C_FLT)
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	  }
	else
	  {
	    float	val;

	    /* We found a float when expecting a double ... handle it.
	     */
	    (*desImp)(src, desSel, &val, @encode(float), &cursor, nil);
	    *(double*)address = (double)val;
	  }
	return;

      case _GSC_DBL:
	if (YES == typeCheck(*type, _GSC_DBL)
	  && *type == _C_DBL)
	  {
	    (*desImp)(src, desSel, address, type, &cursor, nil);
	  }
	else
	  {
	    double	val;

	    /* We found a double when expecting a float ... handle it.
	     */
	    (*desImp)(src, desSel, &val, @encode(double), &cursor, nil);
	    *(float*)address = (float)val;
	  }
	return;

      case _GSC_BOOL:
	if (*type != type_map[_GSC_BOOL])
	  {
	    [NSException raise: NSInternalInconsistencyException
		        format: @"expected %s and got %s",
		    typeToName1(*type), typeToName2(info)];
	  }
	(*desImp)(src, desSel, address, type, &cursor, nil);
	return;

      default:
	[NSException raise: NSInternalInconsistencyException
		    format: @"read unknown type info - %d", info];
    }

{
  uint8_t       size;

  /*
   *	We fall through to here only when we have to decode a value
   *	whose natural size on this system is not the same as on the
   *	machine on which the archive was created.
   */

  switch (*type)
    {
      case _C_SHT:
      case _C_USHT:  size = sizeof(short); break;
      case _C_INT: 
      case _C_UINT:  size = sizeof(int); break;
      case _C_LNG:
      case _C_ULNG:  size = sizeof(long); break;
      case _C_LNG_LNG:
      case _C_ULNG_LNG:  size = sizeof(long long); break;
      default:      size = 1;
    }

  /*
   *	First, we read the data and convert it to the largest size
   *	this system can support.
   */
  if (*type == _C_SHT
    || *type == _C_INT
    || *type == _C_LNG
    || *type == _C_LNG_LNG)
    {
      int64_t   big;

      switch (info & _GSC_SIZE)
        {
          case _GSC_I16:	/* Encoded as 16-bit	*/
            {
              int16_t	val;

              (*desImp)(src, desSel, &val, @encode(int16_t), &cursor, nil);
              big = val;
              break;
            }

          case _GSC_I32:	/* Encoded as 32-bit	*/
            {
              int32_t	val;

              (*desImp)(src, desSel, &val, @encode(int32_t), &cursor, nil);
              big = val;
              break;
            }

          case _GSC_I64:	/* Encoded as 64-bit	*/
            {
              (*desImp)(src, desSel, &big, @encode(int64_t), &cursor, nil);
              break;
            }

          default:		/* A 128-bit value	*/
            {
              big = 0;
              [NSException raise: NSInternalInconsistencyException
                          format: @"Archiving of 128bit integer not allowed"];
            }
        }
      /*
       *	Now we copy from the big value to the destination location.
       */
      switch (size)
        {
          case 1:
            *(int8_t*)address = (int8_t)big;
            if (big > 127 || big < -128)
              {
                NSLog(@"Lost information converting decoded value to int8_t");
              }
            return;
          case 2:
            *(int16_t*)address = (int16_t)big;
            if (big > 32767 || big < -32768)
              {
                NSLog(@"Lost information converting decoded value to int16_t");
              }
            return;
          case 4:
            *(int32_t*)address = (int32_t)big;
            if (big > 2147483647 || big < -2147483648LL)
              {
                NSLog(@"Lost information converting decoded value to int32_t");
              }
            return;
          case 8:
            *(int64_t*)address = big;
            return;
          default:
            [NSException raise: NSInternalInconsistencyException
                        format: @"type/size information error"];
        }
    }
  else
    {
      uint64_t  big;

      switch (info & _GSC_SIZE)
        {
          case _GSC_I16:	/* Encoded as 16-bit	*/
            {
              uint16_t	val;

              (*desImp)(src, desSel, &val, @encode(uint16_t), &cursor, nil);
              big = val;
              break;
            }

          case _GSC_I32:	/* Encoded as 32-bit	*/
            {
              uint32_t	val;

              (*desImp)(src, desSel, &val, @encode(uint32_t), &cursor, nil);
              big = val;
              break;
            }

          case _GSC_I64:	/* Encoded as 64-bit	*/
            {
              (*desImp)(src, desSel, &big, @encode(uint64_t), &cursor, nil);
              break;
            }

          default:		/* A 128-bit value	*/
            {
              big = 0;
              [NSException raise: NSInternalInconsistencyException
                          format: @"Archiving of 128bit integer not allowed"];
            }
        }
      /*
       * Now we copy from the big value to the destination location.
       */
      switch (size)
        {
          case 1:
            if (big & ~0xffLL)
              {
                NSLog(@"Lost information converting decoded value to uint8_t");
              }
            *(uint8_t*)address = (uint8_t)big;
            return;
          case 2:
            if (big & ~0xffffLL)
              {
                NSLog(@"Lost information converting decoded value to uint16_t");
              }
            *(uint16_t*)address = (uint16_t)big;
            return;
          case 4:
            if (big & ~0xffffffffLL)
              {
                NSLog(@"Lost information converting decoded value to uint32_t");
              }
            *(uint32_t*)address = (uint32_t)big;
            return;
          case 8:
            *(uint64_t*)address = big;
            return;
          default:
            [NSException raise: NSInternalInconsistencyException
                        format: @"type/size information error"];
        }
    }
}
}

- (NSData*) decodeDataObject
{
  unsigned	l;

  (*dValImp)(self, dValSel, @encode(unsigned int), &l);
  if (l)
    {
      unsigned char	c;

      (*dValImp)(self, dValSel, @encode(unsigned char), &c);
      if (c == 0)
	{
	  void		*b;
	  NSData	*d;

	  b = NSZoneMalloc(zone, l);
	  [self decodeArrayOfObjCType: @encode(unsigned char)
				count: l
				   at: b];
	  d = [[NSData allocWithZone: zone] initWithBytesNoCopy: b
							 length: l];
	  IF_NO_GC(AUTORELEASE(d));
	  return d;
	}
      else
	{
	  [NSException raise: NSInternalInconsistencyException
		      format: @"Decoding data object with unknown type"];
	}
    }
  return [NSData data];
}

/**
 *  Returns whether have currently read through all of data buffer or file
 *  this unarchiver was initialized with.
 */
- (BOOL) isAtEnd
{
  return (cursor >= [data length]);
}

/**
 *  Returns zone unarchived objects will be allocated from.
 */
- (NSZone*) objectZone
{
  return zone;
}

/**
 *  Sets zone unarchived objects will be allocated from.
 */
- (void) setObjectZone: (NSZone*)aZone
{
  zone = aZone;
}

/**
 *  Returns system version archive was encoded by.
 */
- (unsigned) systemVersion
{
  return version;
}

/**
 *  Returns class name unarchivers will use to instantiate encoded objects
 *  when they report their class name as nameInArchive.
 */
+ (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive
{
  NSUnarchiverClassInfo	*info = [clsDict objectForKey: nameInArchive];
  NSString		*alias;

  if (info == nil)
    {
      return nil;
    }
  alias = info->name;
  if (alias)
    {
      return alias;
    }
  return nameInArchive;
}

/**
 *  Sets class name unarchivers will use to instantiate encoded objects
 *  when they report their class name as nameInArchive.  This can be used
 *  to support backwards compatibility across class name changes.
 */
+ (void) decodeClassName: (NSString*)nameInArchive
	     asClassName: (NSString*)trueName
{
  Class	c;

  c = objc_lookUpClass([trueName cString]);
  if (c == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"can't find class %@", trueName];
    }
  else
    {
      NSUnarchiverClassInfo	*info = [clsDict objectForKey: nameInArchive];

      if (info == nil)
	{
	  info = [NSUnarchiverClassInfo newWithName: nameInArchive];
	  [clsDict setObject: info forKey: nameInArchive];
	  RELEASE(info);
	}
      [info mapToClass: c withName: trueName];
    }
}

/**
 *  Returns class name this unarchiver uses to instantiate encoded objects
 *  when they report their class name as nameInArchive.
 */
- (NSString*) classNameDecodedForArchiveClassName: (NSString*)nameInArchive
{
  NSUnarchiverObjectInfo	*info = [objDict objectForKey: nameInArchive];
  NSString			*alias;

  if (info == nil)
    {
      return nil;
    }
  alias = mapClassName(info);
  if (alias)
    {
      return alias;
    }
  return nameInArchive;
}


/**
 *  Set class name this unarchiver uses to instantiate encoded objects
 *  when they report their class name as nameInArchive.  This can be used
 *  to provide backward compatibility across class name changes.
 */
- (void) decodeClassName: (NSString*)nameInArchive
	     asClassName: (NSString*)trueName
{
  Class	c;

  c = objc_lookUpClass([trueName cString]);
  if (c == 0)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"can't find class %@", trueName];
    }
  else
    {
      NSUnarchiverObjectInfo	*info = [objDict objectForKey: nameInArchive];

      if (info == nil)
	{
	  info = [NSUnarchiverObjectInfo newWithName: nameInArchive];
	  [objDict setObject: info forKey: nameInArchive];
	  RELEASE(info);
	}
      [info mapToClass: c withName: trueName];
    }
}

/**
 *  Set unarchiver to replace anObject with replacement whenever it is
 *  found decoded from the archive.
 */
- (void) replaceObject: (id)anObject withObject: (id)replacement
{
  unsigned i;

  if (replacement == anObject)
    return;
  for (i = GSIArrayCount(objMap) - 1; i > 0; i--)
    {
      if (GSIArrayItemAtIndex(objMap, i).obj == anObject)
	{
	  GSIArraySetItemAtIndex(objMap, (GSIArrayItem)replacement, i);
	  return;
	}
    }
  [NSException raise: NSInvalidArgumentException
	      format: @"object to be replaced does not exist"];
}

- (NSInteger) versionForClassName: (NSString*)className
{
  NSUnarchiverObjectInfo	*info;

  info = [objDict objectForKey: className];
  if (info == nil)
    {
      return (NSInteger)NSNotFound;
    }
  return info->version;
}

@end




/**
 *  Category for compatibility with old GNUstep encoding.
 */
@implementation	NSUnarchiver (GNUstep)

/**
 *  Return current position within archive byte array.
 */
- (unsigned) cursor
{
  return cursor;
}


/**
 *  Prepare for reuse of the unarchiver to unpack a new archive, specified in
 *  anObject, starting at pos.  Reads archive header.
 */
- (void) resetUnarchiverWithData: (NSData*)anObject
			 atIndex: (unsigned)pos
{
  unsigned	sizeC;
  unsigned	sizeO;
  unsigned	sizeP;

  if (anObject == nil)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"nil passed to resetUnarchiverWithData:atIndex:"];
    }
  if (data != anObject)
    {
      Class	c;

      TEST_RELEASE(data);
      data = RETAIN(anObject);
      c = object_getClass(data);
      if (src != self)
	{
	  src = data;
	  if (c != dataClass)
	    {
	      /*
	       *	Cache methods for deserialising from the data object.
	       */
	      desImp = [src methodForSelector: desSel];
	      tagImp = (void (*)(id, SEL, unsigned char*, unsigned*, unsigned*))
		  [src methodForSelector: tagSel];
	    }
	}
      dataClass = c;
    }

  /*
   *	Read header including version and crossref table sizes.
   */
  cursor = pos;
  [self deserializeHeaderAt: &cursor
		    version: &version
		    classes: &sizeC
		    objects: &sizeO
		   pointers: &sizeP];
  if (version > encodingVersion)
    {
      [NSException raise: NSInvalidArgumentException
	format: @"Archive systemVersion (%u) not recognised", version];
    }

  if (clsMap == 0)
    {
      /*
       *	Allocate and initialise arrays to build crossref maps in.
       */
      clsMap = NSZoneMalloc(zone, sizeof(GSIArray_t)*3);
      GSIArrayInitWithZoneAndCapacity(clsMap, zone, sizeC);
      GSIArrayAddItem(clsMap, (GSIArrayItem)(void*)0);

      objMap = &clsMap[1];
      GSIArrayInitWithZoneAndCapacity(objMap, zone, sizeO);
      GSIArrayAddItem(objMap, (GSIArrayItem)(void*)0);

      ptrMap = &clsMap[2];
      GSIArrayInitWithZoneAndCapacity(ptrMap, zone, sizeP);
      GSIArrayAddItem(ptrMap, (GSIArrayItem)(void*)0);
    }
  else
    {
      clsMap->count = 1;
      objMap->count = 1;
      ptrMap->count = 1;
    }

  [objDict removeAllObjects];
  [objSave removeAllObjects];
}

/**
 *  Reads in header for GNUstep archive format.
 */
- (void) deserializeHeaderAt: (unsigned*)pos
		     version: (unsigned*)v
		     classes: (unsigned*)c
		     objects: (unsigned*)o
		    pointers: (unsigned*)p
{
  unsigned	plen = strlen(PREFIX);
  unsigned	size = plen+36;
  char		header[size+1];

  [data getBytes: header range: NSMakeRange(*pos, size)];
  *pos += size;
  header[size] = '\0';
  if (strncmp(header, PREFIX, plen) != 0)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Archive has wrong prefix"];
    }
  if (sscanf(&header[plen], "%x:%x:%x:%x:", v, c, o, p) != 4)
    {
      [NSException raise: NSInternalInconsistencyException
		  format: @"Archive has wrong prefix"];
    }
}

/**
 *  Returns YES.
 */
- (BOOL) directDataAccess
{
  return YES;
}

@end

