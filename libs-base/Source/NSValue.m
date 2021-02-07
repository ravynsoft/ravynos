/** NSValue.m - Object encapsulation for C types.
   Copyright (C) 1993, 1994, 1996, 1999 Free Software Foundation, Inc.

   Written by:  Adam Fedor <fedor@boulder.colorado.edu>
   Date: Mar 1995
   Updated by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: Jan 2001

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

   <title>NSValue class reference</title>
   $Date$ $Revision$
*/

#import "common.h"
#import "Foundation/NSValue.h"
#import "Foundation/NSCoder.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSException.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSData.h"

@interface	GSPlaceholderValue : NSValue
@end

@class	GSValue;
@interface GSValue : NSObject	// Help the compiler
@end
@class	GSNonretainedObjectValue;
@interface GSNonretainedObjectValue : NSObject	// Help the compiler
@end
@class	GSPointValue;
@interface GSPointValue : NSObject	// Help the compiler
@end
@class	GSPointerValue;
@interface GSPointerValue : NSObject	// Help the compiler
@end
@class	GSRangeValue;
@interface GSRangeValue : NSObject	// Help the compiler
@end
@class	GSRectValue;
@interface GSRectValue : NSObject	// Help the compiler
@end
@class	GSSizeValue;
@interface GSSizeValue : NSObject	// Help the compiler
@end
@class	NSDataStatic;		// Needed for decoding.
@interface NSDataStatic : NSData	// Help the compiler
@end


static Class	abstractClass;
static Class	concreteClass;
static Class	nonretainedObjectValueClass;
static Class	pointValueClass;
static Class	pointerValueClass;
static Class	rangeValueClass;
static Class	rectValueClass;
static Class	sizeValueClass;
static Class	GSPlaceholderValueClass;


static GSPlaceholderValue	*defaultPlaceholderValue;
static NSMapTable		*placeholderMap;
static NSLock			*placeholderLock;

@implementation NSValue

+ (void) initialize
{
  if (self == [NSValue class])
    {
      abstractClass = self;
      [abstractClass setVersion: 3];	// Version 3
      concreteClass = [GSValue class];
      nonretainedObjectValueClass = [GSNonretainedObjectValue class];
      pointValueClass = [GSPointValue class];
      pointerValueClass = [GSPointerValue class];
      rangeValueClass = [GSRangeValue class];
      rectValueClass = [GSRectValue class];
      sizeValueClass = [GSSizeValue class];
      GSPlaceholderValueClass = [GSPlaceholderValue class];

      /*
       * Set up infrastructure for placeholder values.
       */
      defaultPlaceholderValue = (GSPlaceholderValue*)
	NSAllocateObject(GSPlaceholderValueClass, 0, NSDefaultMallocZone());
      [[NSObject leakAt: (id*)&defaultPlaceholderValue] release];
      placeholderMap = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
	NSNonRetainedObjectMapValueCallBacks, 0);
      [[NSObject leakAt: (id*)&placeholderMap] release];
      placeholderLock = [NSLock new];
      [[NSObject leakAt: (id*)&placeholderLock] release];
    }
}

+ (id) allocWithZone: (NSZone*)z
{
  if (self == abstractClass)
    {
      if (z == NSDefaultMallocZone() || z == 0)
	{
	  /*
	   * As a special case, we can return a placeholder for a value
	   * in the default malloc zone extremely efficiently.
	   */
	  return defaultPlaceholderValue;
	}
      else
	{
	  id	obj;

	  /*
	   * For anything other than the default zone, we need to
	   * locate the correct placeholder in the (lock protected)
	   * table of placeholders.
	   */
	  [placeholderLock lock];
	  obj = (id)NSMapGet(placeholderMap, (void*)z);
	  if (obj == nil)
	    {
	      /*
	       * There is no placeholder object for this zone, so we
	       * create a new one and use that.
	       */
	      obj = (id)NSAllocateObject(GSPlaceholderValueClass, 0, z);
	      NSMapInsert(placeholderMap, (void*)z, (void*)obj);
	    }
	  [placeholderLock unlock];
	  return obj;
	}
    }
  else
    {
      return NSAllocateObject(self, 0, z);
    }
}

// NSCopying - always a simple retain.

- (id) copy
{
  return RETAIN(self);
}

- (id) copyWithZone: (NSZone *)zone
{
  return RETAIN(self);
}

/* Returns the concrete class associated with the type encoding */
+ (Class) valueClassWithObjCType: (const char *)type
{
  Class	theClass = concreteClass;

  /* Let someone else deal with this error */
  if (!type)
    return theClass;

  /* Try for an exact type match.
   */
  if (strcmp(@encode(id), type) == 0)
    theClass = nonretainedObjectValueClass;
  else if (strcmp(@encode(NSPoint), type) == 0)
    theClass = pointValueClass;
  else if (strcmp(@encode(void *), type) == 0)
    theClass = pointerValueClass;
  else if (strcmp(@encode(NSRange), type) == 0)
    theClass = rangeValueClass;
  else if (strcmp(@encode(NSRect), type) == 0)
    theClass = rectValueClass;
  else if (strcmp(@encode(NSSize), type) == 0)
    theClass = sizeValueClass;

  /* Try for equivalent types match.
   */
  else if (GSSelectorTypesMatch(@encode(id), type))
    theClass = nonretainedObjectValueClass;
  else if (GSSelectorTypesMatch(@encode(NSPoint), type))
    theClass = pointValueClass;
  else if (GSSelectorTypesMatch(@encode(void *), type))
    theClass = pointerValueClass;
  else if (GSSelectorTypesMatch(@encode(NSRange), type))
    theClass = rangeValueClass;
  else if (GSSelectorTypesMatch(@encode(NSRect), type))
    theClass = rectValueClass;
  else if (GSSelectorTypesMatch(@encode(NSSize), type))
    theClass = sizeValueClass;

  return theClass;
}

// Allocating and Initializing

+ (NSValue*) value: (const void *)value
      withObjCType: (const char *)type
{
  Class		theClass = [self valueClassWithObjCType: type];
  NSValue	*theObj;

  theObj = [theClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: value objCType: type];
  return AUTORELEASE(theObj);
}
		
+ (NSValue*) valueWithBytes: (const void *)value
		   objCType: (const char *)type
{
  Class		theClass = [self valueClassWithObjCType: type];
  NSValue	*theObj;

  theObj = [theClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: value objCType: type];
  return AUTORELEASE(theObj);
}
		
+ (NSValue*) valueWithNonretainedObject: (id)anObject
{
  NSValue	*theObj;

  theObj = [nonretainedObjectValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &anObject objCType: @encode(id)];
  return AUTORELEASE(theObj);
}
	
+ (NSValue*) valueWithPoint: (NSPoint)point
{
  NSValue	*theObj;

  theObj = [pointValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &point objCType: @encode(NSPoint)];
  return AUTORELEASE(theObj);
}

+ (NSValue*) valueWithPointer: (const void *)pointer
{
  NSValue	*theObj;

  theObj = [pointerValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &pointer objCType: @encode(void*)];
  return AUTORELEASE(theObj);
}

+ (NSValue*) valueWithRange: (NSRange)range
{
  NSValue	*theObj;

  theObj = [rangeValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &range objCType: @encode(NSRange)];
  return AUTORELEASE(theObj);
}

+ (NSValue*) valueWithRect: (NSRect)rect
{
  NSValue	*theObj;

  theObj = [rectValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &rect objCType: @encode(NSRect)];
  return AUTORELEASE(theObj);
}

+ (NSValue*) valueWithSize: (NSSize)size
{
  NSValue	*theObj;

  theObj = [sizeValueClass allocWithZone: NSDefaultMallocZone()];
  theObj = [theObj initWithBytes: &size objCType: @encode(NSSize)];
  return AUTORELEASE(theObj);
}

+ (NSValue*) valueFromString: (NSString *)string
{
  NSDictionary	*dict = [string propertyList];

  if (dict == nil)
    return nil;

  if ([dict objectForKey: @"location"])
    {
      NSRange range;
      range = NSMakeRange([[dict objectForKey: @"location"] intValue],
			[[dict objectForKey: @"length"] intValue]);
      return [abstractClass valueWithRange: range];
    }
  else if ([dict objectForKey: @"width"] && [dict objectForKey: @"x"])
    {
      NSRect rect;
      rect = NSMakeRect([[dict objectForKey: @"x"] floatValue],
		       [[dict objectForKey: @"y"] floatValue],
		       [[dict objectForKey: @"width"] floatValue],
		       [[dict objectForKey: @"height"] floatValue]);
      return [abstractClass valueWithRect: rect];
    }
  else if ([dict objectForKey: @"width"])
    {
      NSSize size;
      size = NSMakeSize([[dict objectForKey: @"width"] floatValue],
			[[dict objectForKey: @"height"] floatValue]);
      return [abstractClass valueWithSize: size];
    }
  else if ([dict objectForKey: @"x"])
    {
      NSPoint point;
      point = NSMakePoint([[dict objectForKey: @"x"] floatValue],
			[[dict objectForKey: @"y"] floatValue]);
      return [abstractClass valueWithPoint: point];
    }
  return nil;
}

- (id) initWithBytes: (const void*)data objCType: (const char*)type
{
  [self subclassResponsibility: _cmd];
  return nil;
}

// Accessing Data
/* All the rest of these methods must be implemented by a subclass */
- (void) getValue: (void *)value
{
  [self subclassResponsibility: _cmd];
}

- (BOOL) isEqual: (id)other
{
  if ([other isKindOfClass: [self class]])
    {
      return [self isEqualToValue: other];
    }
  return NO;
}

- (BOOL) isEqualToValue: (NSValue*)other
{
  [self subclassResponsibility: _cmd];
  return NO;
}

- (const char *) objCType
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (id) nonretainedObjectValue
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (void *) pointerValue
{
  [self subclassResponsibility: _cmd];
  return 0;
}

- (NSRange) rangeValue
{
  [self subclassResponsibility: _cmd];
  return NSMakeRange(0,0);
}

- (NSRect) rectValue
{
  [self subclassResponsibility: _cmd];
  return NSMakeRect(0,0,0,0);
}

- (NSSize) sizeValue
{
  [self subclassResponsibility: _cmd];
  return NSMakeSize(0,0);
}

- (NSPoint) pointValue
{
  [self subclassResponsibility: _cmd];
  return NSMakePoint(0,0);
}

- (Class) classForCoder
{
  return abstractClass;
}

- (void) encodeWithCoder: (NSCoder *)coder
{
  NSUInteger	tsize;
  unsigned	size;
  const char	*data;
  const char	*objctype = [self objCType];
  NSMutableData	*d;

  size = strlen(objctype)+1;
  [coder encodeValueOfObjCType: @encode(unsigned) at: &size];
  [coder encodeArrayOfObjCType: @encode(signed char) count: size at: objctype];
  if (strncmp("{_NSSize=", objctype, 9) == 0)
    {
      NSSize    v = [self sizeValue];

      [coder encodeValueOfObjCType: objctype at: &v];
      return;
    }
  else if (strncmp("{_NSPoint=", objctype, 10) == 0)
    {
      NSPoint    v = [self pointValue];

      [coder encodeValueOfObjCType: objctype at: &v];
      return;
    }
  else if (strncmp("{_NSRect=", objctype, 9) == 0)
    {
      NSRect    v = [self rectValue];

      [coder encodeValueOfObjCType: objctype at: &v];
      return;
    }
  else if (strncmp("{_NSRange=", objctype, 10) == 0)
    {
      NSRange    v = [self rangeValue];

      [coder encodeValueOfObjCType: objctype at: &v];
      return;
    }

  NSGetSizeAndAlignment(objctype, 0, &tsize);
  data = (void *)NSZoneMalloc([self zone], tsize);
  [self getValue: (void*)data];
  d = [NSMutableData new];
  [d serializeDataAt: data ofObjCType: objctype context: nil];
  size = [d length];
  [coder encodeValueOfObjCType: @encode(unsigned) at: &size];
  NSZoneFree(NSDefaultMallocZone(), (void*)data);
  data = [d bytes];
  [coder encodeArrayOfObjCType: @encode(unsigned char) count: size at: data];
  RELEASE(d);
}

- (id) initWithCoder: (NSCoder *)coder
{
  char		type[64];
  const char	*objctype;
  Class		c;
  id		o;
  NSUInteger	tsize;
  unsigned	size;
  int		ver;

  [coder decodeValueOfObjCType: @encode(unsigned) at: &size];
  /*
   * For almost all type encodings, we can use space on the stack,
   * but to handle exceptionally large ones (possibly some huge structs)
   * we have a strategy of allocating and deallocating heap space too.
   */
  if (size <= 64)
    {
      objctype = type;
    }
  else
    {
      objctype = (void*)NSZoneMalloc(NSDefaultMallocZone(), size);
    }
  [coder decodeArrayOfObjCType: @encode(signed char)
			 count: size
			    at: (void*)objctype];
  if (strncmp("{_NSSize=", objctype, 9) == 0)
    c = [abstractClass valueClassWithObjCType: @encode(NSSize)];
  else if (strncmp("{_NSPoint=", objctype, 10) == 0)
    c = [abstractClass valueClassWithObjCType: @encode(NSPoint)];
  else if (strncmp("{_NSRect=", objctype, 9) == 0)
    c = [abstractClass valueClassWithObjCType: @encode(NSRect)];
  else if (strncmp("{_NSRange=", objctype, 10) == 0)
    c = [abstractClass valueClassWithObjCType: @encode(NSRange)];
  else
    c = [abstractClass valueClassWithObjCType: objctype];
  o = [c allocWithZone: [coder objectZone]];

  ver = [coder versionForClassName: @"NSValue"];
  if (ver > 2)
    {
      if (c == pointValueClass)
        {
          NSPoint	v;

          [coder decodeValueOfObjCType: @encode(NSPoint) at: &v];
          DESTROY(self);
          return [o initWithBytes: &v objCType: @encode(NSPoint)];
        }
      else if (c == sizeValueClass)
        {
          NSSize	v;

          [coder decodeValueOfObjCType: @encode(NSSize) at: &v];
          DESTROY(self);
          return [o initWithBytes: &v objCType: @encode(NSSize)];
        }
      else if (c == rangeValueClass)
        {
          NSRange	v;

          [coder decodeValueOfObjCType: @encode(NSRange) at: &v];
          DESTROY(self);
          return [o initWithBytes: &v objCType: @encode(NSRange)];
        }
      else if (c == rectValueClass)
        {
          NSRect	v;

          [coder decodeValueOfObjCType: @encode(NSRect) at: &v];
          DESTROY(self);
          return [o initWithBytes: &v objCType: @encode(NSRect)];
        }
    }

  if (ver < 2)
    {
      if (ver < 1)
	{
	  if (c == pointValueClass)
	    {
	      NSPoint	v;

	      [coder decodeValueOfObjCType: @encode(NSPoint) at: &v];
	      o = [o initWithBytes: &v objCType: @encode(NSPoint)];
	    }
	  else if (c == sizeValueClass)
	    {
	      NSSize	v;

	      [coder decodeValueOfObjCType: @encode(NSSize) at: &v];
	      o = [o initWithBytes: &v objCType: @encode(NSSize)];
	    }
	  else if (c == rangeValueClass)
	    {
	      NSRange	v;

	      [coder decodeValueOfObjCType: @encode(NSRange) at: &v];
	      o = [o initWithBytes: &v objCType: @encode(NSRange)];
	    }
	  else if (c == rectValueClass)
	    {
	      NSRect	v;

	      [coder decodeValueOfObjCType: @encode(NSRect) at: &v];
	      o = [o initWithBytes: &v objCType: @encode(NSRect)];
	    }
	  else
	    {
	      unsigned char	*data;

	      [coder decodeValueOfObjCType: @encode(unsigned) at: &size];
	      data = (void *)NSZoneMalloc(NSDefaultMallocZone(), size);
	      [coder decodeArrayOfObjCType: @encode(unsigned char)
				     count: size
					at: (void*)data];
	      o = [o initWithBytes: data objCType: objctype];
	      NSZoneFree(NSDefaultMallocZone(), data);
	    }
	}
      else
	{
	  NSData        *d;
	  unsigned      cursor = 0;

	  /*
	   * For performance, decode small values directly onto the stack,
	   * For larger values we allocate and deallocate heap space.
	   */
	  NSGetSizeAndAlignment(objctype, 0, &tsize);
	  if (tsize <= 64)
	    {
	      unsigned char data[tsize];

	      [coder decodeValueOfObjCType: @encode(id) at: &d];
	      [d deserializeDataAt: data
			ofObjCType: objctype
			  atCursor: &cursor
			   context: nil];
	      o = [o initWithBytes: data objCType: objctype];
	      RELEASE(d);
	    }
	  else
	    {
	      unsigned char *data;

	      data = (void *)NSZoneMalloc(NSDefaultMallocZone(), tsize);
	      [coder decodeValueOfObjCType: @encode(id) at: &d];
	      [d deserializeDataAt: data
			ofObjCType: objctype
			  atCursor: &cursor
			   context: nil];
	      o = [o initWithBytes: data objCType: objctype];
	      RELEASE(d);
	      NSZoneFree(NSDefaultMallocZone(), data);
	    }
	}
    }
  else
    {
      static NSData	*d = nil;
      unsigned  	cursor = 0;

      if (d == nil)
	{
	  d = [NSDataStatic allocWithZone: NSDefaultMallocZone()];
	}
      /*
       * For performance, decode small values directly onto the stack,
       * For larger values we allocate and deallocate heap space.
       */
      NSGetSizeAndAlignment(objctype, 0, &tsize);
      if (tsize <= 64)
	{
	  unsigned char	data[tsize];

	  [coder decodeValueOfObjCType: @encode(unsigned) at: &size];
	  {
	    unsigned char	serialized[size];

	    [coder decodeArrayOfObjCType: @encode(unsigned char)
				   count: size
				      at: (void*)serialized];
	    d = [d initWithBytesNoCopy: (void*)serialized
				length: size
			  freeWhenDone: NO];
	    [d deserializeDataAt: data
		      ofObjCType: objctype
			atCursor: &cursor
			 context: nil];
	  }
	  o = [o initWithBytes: data objCType: objctype];
	}
      else
	{
	  void	*data;

	  data = (void *)NSZoneMalloc(NSDefaultMallocZone(), tsize);
	  [coder decodeValueOfObjCType: @encode(unsigned) at: &size];
	  {
	    void	*serialized;

	    serialized = (void *)NSZoneMalloc(NSDefaultMallocZone(), size);
	    [coder decodeArrayOfObjCType: @encode(unsigned char)
				   count: size
				      at: serialized];
	    d = [d initWithBytesNoCopy: serialized length: size];
	    [d deserializeDataAt: data
		      ofObjCType: objctype
			atCursor: &cursor
			 context: nil];
	    NSZoneFree(NSDefaultMallocZone(), serialized);
	  }
	  o = [o initWithBytes: data objCType: objctype];
	  NSZoneFree(NSDefaultMallocZone(), data);
	}
    }
  if (objctype != type)
    {
      NSZoneFree(NSDefaultMallocZone(), (void*)objctype);
    }
  DESTROY(self);
  self = o;
  return self;
}

@end



@implementation	GSPlaceholderValue

- (id) autorelease
{
  NSWarnLog(@"-autorelease sent to uninitialised value");
  return self;		// placeholders never get released.
}

- (void) dealloc
{
  GSNOSUPERDEALLOC;	// placeholders never get deallocated.
}

- (void) getValue: (void*)data
{
  [NSException raise: NSInternalInconsistencyException
	      format: @"attempt to use uninitialised value"];
}

- (id) initWithBytes: (const void*)data objCType: (const char*)type
{
  Class		c = [abstractClass valueClassWithObjCType: type];

  self = (id)NSAllocateObject(c, 0, [self zone]);
  return [self initWithBytes: data objCType: type];
}

- (const char*) objCType
{
  [NSException raise: NSInternalInconsistencyException
	      format: @"attempt to use uninitialised value"];
  return 0;
}

- (oneway void) release
{
  return;		// placeholders never get released.
}

- (id) retain
{
  return self;		// placeholders never get retained.
}
@end

