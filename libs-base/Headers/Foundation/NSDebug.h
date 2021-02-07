/* Interface to debugging utilities for GNUStep and OpenStep
   Copyright (C) 1997-2020 Free Software Foundation, Inc.

   Written by:  Richard Frith-Macdonald <richard@brainstorm.co.uk>
   Date: August 1997
   Extended by: Nicola Pero <n.pero@mi.flashnet.it>
   Date: December 2000, April 2001

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

#ifndef __NSDebug_h_GNUSTEP_BASE_INCLUDE
#define __NSDebug_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#include <errno.h>

#if	!NO_GNUSTEP
#  if	defined(GNUSTEP_BASE_INTERNAL)
#    import	"Foundation/NSObject.h"
#    import	"GNUstepBase/NSDebug+GNUstepBase.h"
#  else
#    import	<Foundation/NSObject.h>
#    import	<GNUstepBase/NSDebug+GNUstepBase.h>
#  endif
#endif

#if	defined(__cplusplus)
extern "C" {
#endif

/** Protocol for a delegate, set as an extension in some classes, to handle
 * debug logging of low level I/O.  The rationale for this protocol is that
 * on occasion debug logging may be required, but the data being logged may
 * contain sensitive information which should not be writtent to file.  In
 * that situation, the delegate may filter/mask the sensitive information
 * from the logs by taking over the simpel writing to stderr that the inbuilt
 * debug logging provides.
 */ 
@protocol GSLogDelegate <NSObject>
/** Method sent to the delegate to ask it to log a chunk of data that
 * has been read.  The delegate should return YES if it has handled
 * the logging, NO if it wants the default mechanism to be used.<br />
 * The handle is the object which is performing the read operation.
 */
- (BOOL) getBytes: (const uint8_t*)bytes
         ofLength: (NSUInteger)length
         byHandle: (NSObject*)handle;

/** Method sent to the delegate to ask it to log a chunk of data that
 * has been written (or is immediately going to be written).
 * The delegate should return YES if it has handled the logging,
 * NO if it wants the default logging mechanism to be used.<br />
 * The handle is the object which is performing the write operation.
 */
- (BOOL) putBytes: (const uint8_t*)bytes
         ofLength: (NSUInteger)length
         byHandle: (NSObject*)handle;
@end

/*
 *	Functions for debugging object allocation/deallocation
 *
 *	Internal functions:
 *	GSDebugAllocationAdd()		is used by NSAllocateObject()
 *	GSDebugAllocationRemove()	is used by NSDeallocateObject()
 *
 *	Public functions:
 *	GSDebugAllocationActive()	
 *	GSDebugAllocationBytes()	
 *	GSDebugAllocationCount()	
 *      GSDebugAllocationTotal()
 *      GSDebugAllocationPeak()
 *      GSDebugAllocationClassList()
 *	GSDebugAllocationList()
 *	GSDebugAllocationListAll()
 *
 *      GSSetDebugAllocationFunctions()
 *
 * When the previous functions have allowed you to find a memory leak,
 * and you know that you are leaking objects of class XXX, but you are
 * hopeless about actually finding out where the leak is, the
 * following functions could come handy as they allow you to find
 * exactly *what* objects you are leaking (warning! these functions
 * could slow down your system appreciably - use them only temporarily
 * and only in debugging systems):
 *
 *  GSDebugAllocationRecordObjects()
 *  GSDebugAllocationListRecordedObjects() 
 *  GSDebugAllocationTagRecordedObject()
 */
#ifndef	NDEBUG

/**
 * Used internally by NSAllocateObject() ... you probably don't need this.
 */
GS_EXPORT void		GSDebugAllocationAdd(Class c, id o);

/**
 * Used internally by NSDeallocateObject() ... you probably don't need this.
 */
GS_EXPORT void		GSDebugAllocationRemove(Class c, id o);

/**
 * This function activates or deactivates object allocation debugging.<br />
 * Returns the previous state.<br />
 * You should call this function to activate
 * allocation debugging before using any of the other allocation
 * debugging functions such as GSDebugAllocationList() or
 * GSDebugAllocationTotal().<br />
 * Object allocation debugging
 * should not affect performance too much, and is very useful
 * as it allows you to monitor how many objects of each class
 * your application has allocated.
 */
GS_EXPORT BOOL		GSDebugAllocationActive(BOOL active);

/**
 * This function activates or deactivates byte counting for allocation.<br />
 * Returns the previous state.<br />
 * You may call this function to activate additional checks to see how
 * much memory is allocated to hold each object allocated.  When this is
 * enabled, listing the allocated objects will also list the number of bytes
 * of heap memory allocated to hold the objects.<br />
 */
GS_EXPORT BOOL		GSDebugAllocationBytes(BOOL active);

/**
 * <p>
 *   Returns the number
 *   of instances of the specified class which are currently
 *   allocated.  This number is very important to detect memory
 *   leaks.  If you notice that this number is constantly
 *   increasing without apparent reason, it is very likely a
 *   memory leak - you need to check that you are correctly
 *   releasing objects of this class, otherwise when your
 *   application runs for a long time, it will eventually
 *   allocate so many objects as to eat up all your system's
 *   memory ...
 * </p>
 * <p>
 *   This function, like the ones below, returns the number of
 *   objects allocated/released from the time when
 *   GSDebugAllocationActive() was first called.  A negative
 *   number means that in total, there are less objects of this
 *   class allocated now than there were when you called
 *   GSDebugAllocationActive(); a positive one means there are
 *   more.
 * </p>
 */
GS_EXPORT int		GSDebugAllocationCount(Class c);

/**
 * Returns the peak
 * number of instances of the specified class which have been
 * concurrently allocated.  If this number is very high, it
 * means at some point in time you had a situation with a
 * huge number of objects of this class allocated - this is
 * an indicator that probably at some point in time your
 * application was using a lot of memory - so you might want
 * to investigate whether you can prevent this problem by
 * inserting autorelease pools in your application's
 * processing loops.
 */
GS_EXPORT int		GSDebugAllocationPeak(Class c);

/**
 * Returns the total
 * number of instances of the specified class c which have been
 * allocated - basically the number of times you have
 * allocated an object of this class.  If this number is very
 * high, it means you are creating a lot of objects of this
 * class; even if you are releasing them correctly, you must
 * not forget that allocating and deallocating objects is
 * usually one of the slowest things you can do, so you might
 * want to consider whether you can reduce the number of
 * allocations and deallocations that you are doing - for
 * example, by recycling objects of this class, uniquing
 * them, and/or using some sort of flyweight pattern.  It
 * might also be possible that you are unnecessarily creating
 * too many objects of this class.  Well - of course some times
 * there is nothing you can do about it.
 */
GS_EXPORT int		GSDebugAllocationTotal(Class c);

/**
 * This function returns a NULL
 * terminated array listing all the classes for which
 * statistical information has been collected.  Usually, you
 * call this function, and then loop on all the classes returned,
 * and for each one you get current, peak and total count by
 * using GSDebugAllocationCount(), GSDebugAllocationPeak() and
 * GSDebugAllocationTotal().
 */
GS_EXPORT Class*        GSDebugAllocationClassList(void);

/**
 * This function returns a newline separated list of the classes
 * which have instances allocated, and the instance counts.
 * If the 'changeFlag' argument is YES then the list gives the number
 * of instances allocated/deallocated since the function was
 * last called with that setting.  This function only returns the
 * current count of instances (not the peak or total count), but its
 * output is ready to be displayed or logged.
 */
GS_EXPORT const char*	GSDebugAllocationList(BOOL changeFlag);

/**
 * This function returns a newline
 * separated list of the classes which have had instances
 * allocated at any point, and the total count of the number
 * of instances allocated for each class.  The difference with
 * GSDebugAllocationList() is that this function returns also
 * classes which have no objects allocated at the moment, but
 * which had in the past.
 */
GS_EXPORT const char*	GSDebugAllocationListAll(void);

/**
 * DEPRECATED ... use GSDebugAllocationRecordObjects instead.
 */
GS_EXPORT void     GSDebugAllocationActiveRecordingObjects(Class c);

/**
 * This function activates (or deactivates) tracking all allocated
 * instances of the specified class c.<br />
 * Turning on tracking implicitly turns on memory debug (counts)
 * for all classes (GSAllocationActive()).<br />
 * Deactivation of tracking releases all currently tracked instances
 * of the class (but deactivation of general counting does not).<br />
 * The previous tracking state as reported as the return value of
 * this function.<br />
 * This tracking can slow your application down, so you should use it
 * only when you are into serious debugging.
 * Usually, you will monitor your application by using the functions
 * GSDebugAllocationList() and similar, which do not slow things down
 * much and return * the number of allocated instances; when
 * (if) by studying the reports generated by these functions
 * you have found a leak of objects of a certain class, and
 * if you can't figure out how to fix it by looking at the
 * code, you can use this function to start tracking
 * allocated instances of that class, and the following one
 * can sometime allow you to list the leaked objects directly.
 */
GS_EXPORT BOOL  GSDebugAllocationRecordObjects(Class c, BOOL newState);

/**
 * This function returns an array
 * containing all the allocated objects of a certain class
 * which have been recorded ... to start the recording, you need
 * to invoke GSDebugAllocationRecordObjects().
 * Presumably, you will immediately call [NSObject-description] on them
 * to find out the objects you are leaking.  The objects are
 * returned in an autoreleased array, so until the array is deallocated,
 * the objects are not released.
 */
GS_EXPORT NSArray *GSDebugAllocationListRecordedObjects(Class c);

/**
 * This function associates the supplied tag with a recorded
 * object and returns the tag which was previously associated
 * with it (if any).<br />
 * If the object was not recorded, the method returns nil<br />
 * The tag is retained while it is associated with the object.<br />
 * If the tagged object is deallocated, the tag is released
 * (so you can track the lifetime of the object by having the tag
 * perform some operation when it is released).<br />
 * See also the NSDebugFRLog() and NSDebugMRLog() macros.
 */
GS_EXPORT id GSDebugAllocationTagRecordedObject(id object, id tag);

/**
 * This functions allows to set own function callbacks for debugging allocation
 * of objects. Useful if you intend to write your own object allocation code.
 */
GS_EXPORT void  GSSetDebugAllocationFunctions(
  void (*newAddObjectFunc)(Class c, id o),
  void (*newRemoveObjectFunc)(Class c, id o));

#endif

/**
 * Enable/disable zombies.
 * <p>When an object is deallocated, its isa pointer is normally modified
 * to the hexadecimal value 0xdeadface, so that any attempt to send a
 * message to the deallocated object will cause a crash, and examination
 * of the object within the debugger will show the 0xdeadface value ...
 * making it obvious why the program crashed.
 * </p>
 * <p>Turning on zombies changes this behavior so that the isa pointer
 * is modified to be that of the NSZombie class.  When messages are
 * sent to the object, instead of crashing, NSZombie will use NSLog() to
 * produce an error message.  By default the memory used by the object
 * will not really be freed, so error messages will continue to
 * be generated whenever a message is sent to the object, and the object
 * instance variables will remain available for examination by the debugger.
 * </p>
 * The default value of this boolean is NO, but this can be controlled
 * by the NSZombieEnabled environment variable.
 */
GS_EXPORT BOOL NSZombieEnabled;

/**
 * Enable/disable object deallocation.
 * <p>If zombies are enabled, objects are by default <em>not</em>
 * deallocated, and memory leaks.  The NSDeallocateZombies variable
 * lets you say that the the memory used by zombies should be freed.
 * </p>
 * <p>Doing this makes the behavior of zombies similar to that when zombies
 * are not enabled ... the memory occupied by the zombie may be re-used for
 * other purposes, at which time the isa pointer may be overwritten and the
 * zombie behavior will cease.
 * </p>
 * The default value of this boolean is NO, but this can be controlled
 * by the NSDeallocateZombies environment variable.
 */
GS_EXPORT BOOL NSDeallocateZombies;



/**
 *  Retrieve stack information.  Use caution: uses built-in gcc functions
 *  and currently only works up to 100 frames.
 */
GS_EXPORT void *NSFrameAddress(NSUInteger offset);

/**
 *  Retrieve stack information.  Use caution: uses built-in gcc functions
 *  and currently only works up to 100 frames.
 */
GS_EXPORT void *NSReturnAddress(NSUInteger offset);

/**
 *  Retrieve stack information.  Use caution: uses built-in gcc functions
 *  and currently only works up to 100 frames.
 */
GS_EXPORT NSUInteger NSCountFrames(void);

#if	defined(__cplusplus)
}
#endif

#endif
