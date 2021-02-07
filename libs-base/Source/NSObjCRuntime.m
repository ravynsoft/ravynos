/** Implementation of ObjC runtime for GNUStep
   Copyright (C) 1995 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: Aug 1995

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

   <title>NSObjCRuntime class reference</title>
   $Date$ $Revision$
   */

#import "common.h"

#if !defined (__GNU_LIBOBJC__)
#  include <objc/encoding.h>
#endif

#import "Foundation/NSException.h"

/**
 * Returns a string object containing the name for
 * aProtocol.  If aProtocol is 0, returns nil.
 */
NSString *
NSStringFromProtocol(Protocol *aProtocol)
{
  if (aProtocol != (Protocol*)0)
    return [NSString stringWithUTF8String: protocol_getName(aProtocol)];
  return nil;
}

/**
 * Returns the protocol whose name is supplied in the
 * aProtocolName argument, or 0 if a nil string is supplied.
 */
Protocol *   
NSProtocolFromString(NSString *aProtocolName)
{
  if (aProtocolName != nil)
    {
      int	len = [aProtocolName length];
      char	buf[len+1];

      [aProtocolName getCString: buf
		      maxLength: len + 1
		       encoding: NSASCIIStringEncoding];
      return GSProtocolFromName (buf);
    }
  return (Protocol*)0;
}

/**
 * Returns a string object containing the name for
 * aSelector.  If aSelector is 0, returns nil.
 */
NSString *
NSStringFromSelector(SEL aSelector)
{
  if (aSelector != (SEL)0)
    return [NSString stringWithUTF8String: sel_getName(aSelector)];
  return nil;
}

/**
 * Returns (creating if necessary) the selector whose name is supplied in the
 * aSelectorName argument, or 0 if a nil string is supplied.
 */
SEL
NSSelectorFromString(NSString *aSelectorName)
{
  if (aSelectorName != nil)
    {
      int	len = [aSelectorName length];
      char	buf[len+1];

      [aSelectorName getCString: buf
		      maxLength: len + 1
		       encoding: NSASCIIStringEncoding];
      return sel_registerName (buf);
    }
  return (SEL)0;
}

/**
 * Returns the class whose name is supplied in the
 * aClassName argument, or Nil if a nil string is supplied.
 * If no such class has been loaded, the function returns Nil.
 */
Class
NSClassFromString(NSString *aClassName)
{
  if (aClassName != nil)
    {
      int	len = [aClassName length];
      char	buf[len+1];

      [aClassName getCString: buf
		   maxLength: len + 1
		    encoding: NSASCIIStringEncoding];
      return objc_lookUpClass (buf);
    }
  return (Class)0;
}

/**
 * Returns an [NSString] object containing the class name for
 * aClass.  If aClass is 0, returns nil.
 */
NSString *
NSStringFromClass(Class aClass)
{
  if (aClass != (Class)0)
    return [NSString stringWithUTF8String: (char*)class_getName(aClass)];
  return nil;
}

/**
 * When provided with a C string containing encoded type information,
 * this method extracts size and alignment information for the specified
 * type into the buffers pointed to by sizep and alignp.<br />
 * If either sizep or alignp is a null pointer, the corresponding data is
 * not extracted.<br />
 * The function returns a pointer into the type information C string
 * immediately after the decoded information.
 */
const char *
NSGetSizeAndAlignment(const char *typePtr,
  NSUInteger *sizep, NSUInteger *alignp)
{
  if (typePtr != NULL)
    {
      /* Skip any offset, but don't call objc_skip_offset() as that's buggy.
       */
      if (*typePtr == '+' || *typePtr == '-')
	{
	  typePtr++;
	}
      while (isdigit(*typePtr))
	{
	  typePtr++;
	}
      typePtr = objc_skip_type_qualifiers (typePtr);
      if (sizep)
	{
          *sizep = objc_sizeof_type (typePtr);
	}
      if (alignp)
	{
          *alignp = objc_alignof_type (typePtr);
	}
      typePtr = objc_skip_typespec (typePtr);
    }
  return typePtr;
}

