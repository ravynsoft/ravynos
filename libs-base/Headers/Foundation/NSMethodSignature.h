/* Interface for NSMethodSignature for GNUStep
   Copyright (C) 1995, 1998 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: 1995
   Rewritten:	Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: 1998
   
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
   */ 

#ifndef __NSMethodSignature_h_GNUSTEP_BASE_INCLUDE
#define __NSMethodSignature_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif

/**
 * <p>Class encapsulating type information for method arguments and return
 * value.  It is used as a component of [NSInvocation] to implement message
 * forwarding, such as within the distributed objects framework.  Instances
 * can be obtained from the [NSObject] method
 * [NSObject-methodSignatureForSelector:].</p>
 *
 * <p>Basically, types are represented as Objective-C <code>@encode(...)</code>
 * compatible strings.  The arguments are
 * numbered starting from 0, including the implicit arguments
 * <code><em>self</em></code> (type <code>id</code>, at position 0) and
 * <code><em>_cmd</em></code> (type <code>SEL</code>, at position 1).</p>
 */
GS_EXPORT_CLASS
@interface NSMethodSignature : NSObject
{
#if	GS_EXPOSE(NSMethodSignature)
@private
  const char		*_methodTypes;
  NSUInteger		_argFrameLength;
  NSUInteger		_numArgs;
  void			*_info;
#endif
}

/**
 * Build a method signature directly from string description of return type and
 * argument types, using the Objective-C <code>@encode(...)</code> type codes.
 */
+ (NSMethodSignature*) signatureWithObjCTypes: (const char*)t;

/**
 * Number of bytes that the full set of arguments occupies on the stack, which
 * is platform(hardware)-dependent.
 */
- (NSUInteger) frameLength;

/**
 * Returns Objective-C <code>@encode(...)</code> compatible string.  Arguments
 * are numbered starting from 0, including the implicit arguments
 * <code><em>self</em></code> (type <code>id</code>, at position 0) and
 * <code><em>_cmd</em></code> (type <code>SEL</code>, at position 1).<br />
 * Type strings may include leading type qualifiers.
 */
- (const char*) getArgumentTypeAtIndex: (NSUInteger)index;

/**
 * Pertains to distributed objects; method is asynchronous when invoked and
 * return should not be waited for.
 */
- (BOOL) isOneway;

/**
 * Number of bytes that the return value occupies on the stack, which is
 * platform(hardware)-dependent.
 */
- (NSUInteger) methodReturnLength;

/**
 * Returns an Objective-C <code>@encode(...)</code> compatible string
 * describing the return type of the method.  This may include type
 * qualifiers.
 */
- (const char*) methodReturnType;

/**
 * Returns number of arguments to method, including the implicit
 * <code><em>self</em></code> and <code><em>_cmd</em></code>.
 */
- (NSUInteger) numberOfArguments;

@end

#if	defined(__cplusplus)
}
#endif

#endif /* __NSMethodSignature_h_GNUSTEP_BASE_INCLUDE */
