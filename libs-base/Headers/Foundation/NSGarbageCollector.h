/** Interface for NSGarbageCollector for GNUStep
   Copyright (C) 2009 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Created: Jan 2009
   
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

   AutogsdocSource: NSGarbageCollector.m

   */ 

#ifndef _NSGarbageCollector_h_GNUSTEP_BASE_INCLUDE
#define _NSGarbageCollector_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)

#import	<Foundation/NSObject.h>

#if	defined(__cplusplus)
extern "C" {
#endif


GS_EXPORT_CLASS
@interface NSGarbageCollector : NSObject 

/** Returns the garbage collector instance ... there is only one.<br />
 * Returns nil if the process is not using garbage collection.
 */
+ (id) defaultCollector;

/** Collects some memory.
 */
- (void) collectIfNeeded;

/** Collects all collectable memory.
 */
- (void) collectExhaustively;

/** Disables garbage collection until a corresponding call to -enable is made.
 * NB. Calls to this method stack, and must be matched by the same number of
 * calls to the -enable method.
 */
- (void) disable;

/** Makes the area of memory pointed at be uncollectable ... that is to say,
 * the memory will not be collected by the garbage collector.  You must not
 * explicitly free this memory unless you re-enable collection first.<br />
 * Calls to this method do not stack, so callig it multiple times for the
 * same pointer has the same effect as calling it once.
 */
- (void) disableCollectorForPointer: (void *)ptr;

/** Enables garbage collection prevously disabled by a call to the
 * -disable method.  Since calls to -disable stack, you must make as
 * many calls to -enable as to -disable in order to re-start collection.
 */
- (void) enable;

/** Enables collection for the area of memory pointed at, which must have
 * previously been made uncollectable by a call to the
 * -disableCollectorForPointer: method.
 */
- (void) enableCollectorForPointer: (void *)ptr;      

/** Returns yes if there is a garbage collection progress.
 */
- (BOOL) isCollecting;

/** Retunrs YES if garbage collecting is currently enabled.
 */
- (BOOL) isEnabled;

/** Returns a zone for holding non-collectable pointers.<br />
 * Memory allocated in this zone will not be seen by the garbage collector
 * and will never be collected (so it needs to be freed explicitly).
 * The presence of pointers from the memory to other objects will not
 * prevent those other objects from being collected.
 */
- (NSZone*) zone;
@end

#if	defined(__cplusplus)
}
#endif

#endif
#endif
