/* Control of executable units within a shared virtual memory space
   Copyright (C) 1996 Free Software Foundation, Inc.

   Original Author:  Scott Christley <scottc@net-community.com>
   Rewritten by: Andrew McCallum <mccallum@gnu.ai.mit.edu>
   Created: 1996
   
   This file is part of the GNUstep Objective-C Library.

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

#ifndef __NSThread_h_GNUSTEP_BASE_INCLUDE
#define __NSThread_h_GNUSTEP_BASE_INCLUDE
#import	<GNUstepBase/GSVersionMacros.h>

#if	defined(GNUSTEP_BASE_INTERNAL)
#import	"Foundation/NSAutoreleasePool.h" // for struct autorelease_thread_vars
#import	"Foundation/NSException.h"	// for NSHandler
#else
#import	<Foundation/NSAutoreleasePool.h>
#import	<Foundation/NSException.h>
#endif

@class  NSArray;
@class	NSDate;
@class	NSMutableDictionary;

#if	defined(__cplusplus)
extern "C" {
#endif

/**
 * This class encapsulates OpenStep threading.  See [NSLock] and its
 * subclasses for handling synchronisation between threads.<br />
 * Each process begins with a main thread and additional threads can
 * be created using NSThread.  The GNUstep implementation of OpenStep
 * has been carefully designed so that the internals of the base
 * library do not use threading (except for methods which explicitly
 * deal with threads of course) so that you can write applications
 * without threading.  Non-threaded applications are more efficient
 * (no locking is required) and are easier to debug during development.
 */
GS_EXPORT_CLASS
@interface NSThread : NSObject
{
#if	GS_EXPOSE(NSThread)
@public
  id			_target;
  id			_arg;
  SEL			_selector;
  NSString              *_name;
  NSUInteger            _stackSize;
  BOOL			_cancelled;
  BOOL			_active;
  BOOL			_finished;
  NSHandler		*_exception_handler;    // Not retained.
  NSMutableDictionary	*_thread_dictionary;
  struct autorelease_thread_vars _autorelease_vars;
  id			_gcontext;
  void                  *_runLoopInfo;  // Per-thread runloop related info.
#endif
#if     GS_NONFRAGILE
#  if defined(GS_NSThread_IVARS)
@public GS_NSThread_IVARS;
#  endif
#else
  /* Pointer to private additional data used to avoid breaking ABI
   * when we don't have the non-fragile ABI available.
   * Use this mechanism rather than changing the instance variable
   * layout (see Source/GSInternal.h for details).
   */
  @private id _internal GS_UNUSED_IVAR;
#endif
}

/**
 * <p>
 *   Returns the NSThread object corresponding to the current thread.
 * </p>
 * <p>
 *   NB. In GNUstep the library internals use the GSCurrentThread()
 *   function as a more efficient mechanism for doing this job - so
 *   you cannot use a category to override this method and expect
 *   the library internals to use your implementation.
 * </p>
 */
+ (NSThread*) currentThread;

/**
 * <p>Create a new thread - use this method rather than alloc-init.  The new
 * thread will begin executing the message given by aSelector, aTarget, and
 * anArgument.  This should have no return value, and must set up an
 * autorelease pool if retain/release memory management is used.  It should
 * free this pool before it finishes execution.</p>
 */
+ (void) detachNewThreadSelector: (SEL)aSelector
		        toTarget: (id)aTarget
		      withObject: (id)anArgument;

/**
 * Terminates the current thread.<br />
 * Normally you don't need to call this method explicitly,
 * since exiting the method with which the thread was detached
 * causes this method to be called automatically.
 */
+ (void) exit;

/**
 * Returns a flag to say whether the application is multi-threaded or not.<br />
 * An application is considered to be multi-threaded if any thread other
 * than the main thread has been started, irrespective of whether that
 * thread has since terminated.<br />
 * NB. This method returns YES if called within a handler processing
 * <code>NSWillBecomeMultiThreadedNotification</code>
 */
+ (BOOL) isMultiThreaded;
+ (void) sleepUntilDate: (NSDate*)date;

- (NSMutableDictionary*) threadDictionary;

#if OS_API_VERSION(MAC_OS_X_VERSION_10_2,GS_API_LATEST) \
  && GS_API_VERSION( 10200,GS_API_LATEST)
+ (void) setThreadPriority: (double)pri;
+ (double) threadPriority;
#endif

#if OS_API_VERSION(MAC_OS_X_VERSION_10_5,GS_API_LATEST) \
  && GS_API_VERSION( 11501,GS_API_LATEST)

/** Returns an array of the call stack return addresses.
 */
+ (NSArray*) callStackReturnAddresses;

/** Returns a boolean indicating whether this thread is the main thread of
 * the process.
 */
+ (BOOL) isMainThread;

/** Returns the main thread of the process.
 */
+ (NSThread*) mainThread;

/** Suspends execution of the process for the specified period.
 */
+ (void) sleepForTimeInterval: (NSTimeInterval)ti;

/** Cancels the receiving thread.
 */
- (void) cancel;

/** <init/>
 */
- (id) init;

/** Initialises the receiver to send the message aSelector to the object aTarget
 * with the argument anArgument (which may be nil).<br />
 * The arguments aTarget and aSelector are retained while the thread is
 * running.
 */
- (id) initWithTarget: (id)aTarget
             selector: (SEL)aSelector
               object: (id)anArgument;

/** Returns a boolean indicating whether the receiving
 * thread has been cancelled.
 */
- (BOOL) isCancelled;

/** Returns a boolean indicating whether the receiving
 * thread has been started (and has not yet finished or been cancelled).
 */
- (BOOL) isExecuting;

/** Returns a boolean indicating whether the receiving
 * thread has completed executing.
 */
- (BOOL) isFinished;

/** Returns a boolean indicating whether this thread is the main thread of
 * the process.
 */
- (BOOL) isMainThread;

/** FIXME ... what does this do?
 */
- (void) main;

/** Returns the name of the receiver.
 */
- (NSString*) name;

/** Sets the name of the receiver.
 */
- (void) setName: (NSString*)aName;

/** Sets the size of the receiver's stack.
 */
- (void) setStackSize: (NSUInteger)stackSize;

/** Returns the size of the receiver's stack.
 */
- (NSUInteger) stackSize;

/** Starts the receiver executing.
 */
- (void) start;
#endif

@end

/**
 * Extra methods to permit messages to be sent to an object such that they
 * are executed in <em>another</em> thread.<br />
 * The main thread is the thread in which the GNUstep system is started,
 * and where the GNUstep gui is used, it is the thread in which gui
 * drawing operations <strong>must</strong> be performed.
 */
@interface	NSObject(NSThreadPerformAdditions)
#if	GS_API_VERSION(MAC_OS_X_VERSION_10_2, GS_API_LATEST)
/**
 * <p>This method performs aSelector on the receiver, passing anObject as
 * an argument, but does so in the main thread of the program.  The receiver
 * and anObject are both retained until the method is performed.
 * </p>
 * <p>The selector is performed when the runloop of the main thread next
 * runs in one of the modes specified in anArray.<br />
 * Where this method has been called more than once before the runloop
 * of the main thread runs in the required mode, the order in which the
 * operations in the main thread is done is the same as that in which
 * they were added using this method.
 * </p>
 * <p>If there are no modes in anArray,
 * the method has no effect and simply returns immediately.
 * </p>
 * <p>The argument aFlag specifies whether the method should wait until
 * the selector has been performed before returning.<br />
 * <strong>NB.</strong> This method does <em>not</em> cause the runloop of
 * the main thread to be run ... so if the runloop is not executed by some
 * code in the main thread, the thread waiting for the perform to complete
 * will block forever.
 * </p>
 * <p>As a special case, if aFlag == YES and the current thread is the main
 * thread, the modes array is ignored and the selector is performed immediately.
 * This behavior is necessary to avoid the main thread being blocked by
 * waiting for a perform which will never happen because the runloop is
 * not executing.
 * </p>
 */
- (void) performSelectorOnMainThread: (SEL)aSelector
			  withObject: (id)anObject
		       waitUntilDone: (BOOL)aFlag
			       modes: (NSArray*)anArray;
/**
 * Invokes -performSelectorOnMainThread:withObject:waitUntilDone:modes:
 * using the supplied arguments and an array containing common modes.<br />
 * These modes consist of NSRunLoopMode, NSConnectionReplyMode, and if
 * in an application, the NSApplication modes.
 */
- (void) performSelectorOnMainThread: (SEL)aSelector
			  withObject: (id)anObject
		       waitUntilDone: (BOOL)aFlag;
#endif
#if	GS_API_VERSION(MAC_OS_X_VERSION_10_5, GS_API_LATEST)
/**
 * <p>This method performs aSelector on the receiver, passing anObject as
 * an argument, but does so in the specified thread.  The receiver
 * and anObject are both retained until the method is performed.
 * </p>
 * <p>The selector is performed when the runloop of aThread next
 * runs in one of the modes specified in anArray.<br />
 * Where this method has been called more than once before the runloop
 * of the thread runs in the required mode, the order in which the
 * operations in the thread is done is the same as that in which
 * they were added using this method.
 * </p>
 * <p>If there are no modes in anArray,
 * the method has no effect and simply returns immediately.
 * </p>
 * <p>The argument aFlag specifies whether the method should wait until
 * the selector has been performed before returning.<br />
 * <strong>NB.</strong> This method does <em>not</em> cause the runloop of
 * aThread to be run ... so if the runloop is not executed by some
 * code in aThread, the thread waiting for the perform to complete
 * will block forever.
 * </p>
 * <p>As a special case, if aFlag == YES and the current thread is aThread,
 * the modes array is ignored and the selector is performed immediately.
 * This behavior is necessary to avoid the current thread being blocked by
 * waiting for a perform which will never happen because the runloop is
 * not executing.
 * </p>
 */
- (void) performSelector: (SEL)aSelector
                onThread: (NSThread*)aThread
              withObject: (id)anObject
           waitUntilDone: (BOOL)aFlag
                   modes: (NSArray*)anArray;
/**
 * Invokes -performSelector:onThread:withObject:waitUntilDone:modes:
 * using the supplied arguments and an array containing common modes.<br />
 * These modes consist of NSRunLoopMode, NSConnectionreplyMode, and if
 * in an application, the NSApplication modes.
 */
- (void) performSelector: (SEL)aSelector
                onThread: (NSThread*)aThread
              withObject: (id)anObject
           waitUntilDone: (BOOL)aFlag;

/**
 * Creates and runs a new background thread sending aSelector to the receiver
 * and passing anObject (which may be nil) as the argument.
 */
- (void) performSelectorInBackground: (SEL)aSelector
                          withObject: (id)anObject; 
#endif
@end

@interface NSThread (CallStackSymbols)
#if	GS_API_VERSION(MAC_OS_X_VERSION_10_6, GS_API_LATEST)
/** Returns an array of NSString objects representing the current stack
 * in an implementation-defined format. May return an empty array if
 * this feature is not available.
 */
+ (NSArray *) callStackSymbols;
#endif
@end

#if	GS_API_VERSION(GS_API_NONE, GS_API_NONE)
/*
 * Don't use the following functions unless you really know what you are 
 * doing ! 
 * The following functions are low-levelish and special. 
 * They are meant to make it possible to run GNUstep code in threads 
 * created in completely different environment, eg inside a JVM.
 *
 * If you use them, make sure you initialize the NSThread class inside
 * (what you consider to be your) main thread, before registering any
 * other thread.  To initialize NSThread, simply call GSCurrentThread
 * ().  The main thread will not need to be registered.  
 */

/*
 * Register an external thread (created using your OS thread interface
 * directly) to GNUstep.  This means that it creates a NSThread object
 * corresponding to the current thread, and sets things up so that you
 * can run GNUstep code inside the thread.  If the thread was not
 * known to GNUstep, this function registers it, and returns YES.  If
 * the thread was already known to GNUstep, this function does nothing
 * and returns NO.  */
GS_EXPORT BOOL GSRegisterCurrentThread (void);
/*
 * Unregister the current thread from GNUstep.  You must only
 * unregister threads which have been register using
 * registerCurrentThread ().  This method is basically the same as
 * `+exit', but does not exit the thread - just destroys all objects
 * associated with the thread.  Warning: using any GNUstep code after
 * this method call is not safe.  Posts an NSThreadWillExit
 * notification.  */
GS_EXPORT void GSUnregisterCurrentThread (void);

/* Internal API used by traced locks.
 */
@interface NSThread (GSLockInfo)

/* Removes the mutex (either as the one we are waiting for or as a held mutex.
 * For internal use only ... do not call this method.<br />
 */
- (NSString *) mutexDrop: (id)mutex;

/* Converts a waiting mutex to a held one (if mutex is nil), or increments
 * the recursion count of a mutex already held by this thread.<br />
 * For internal use only ... do not call this method.
 */
- (NSString *) mutexHold: (id)mutex;

/* Register the mutex that the thread is waiting for.<br />
 * For internal use only ... do not call this method.
 */
- (NSString *) mutexWait: (id)mutex;

@end

#endif

/*
 * Notification Strings.
 * NSBecomingMultiThreaded and NSThreadExiting are defined for strict
 * OpenStep compatibility, the actual notification names are the more
 * modern OPENSTEP/MacOS versions.
 */

/**
 *  Notification posted the first time a new [NSThread] is created or a
 *  separate thread from another library is registered in an application.
 *  (The initial thread that a program starts with does <em>not</em>
 *  post this notification.)  Before such a notification has been posted you
 *  can assume the application is in single-threaded mode and locks are not
 *  necessary.  Afterwards multiple threads <em>may</em> be running.
 */
GS_EXPORT NSString* const NSWillBecomeMultiThreadedNotification;
#define	NSBecomingMultiThreaded NSWillBecomeMultiThreadedNotification

/**
 *  Notification posted when an [NSThread] instance receives an exit message,
 *  or an external thread has been deregistered.
 */
GS_EXPORT NSString* const NSThreadWillExitNotification;
#define NSThreadExiting NSThreadWillExitNotification

#if	GS_API_VERSION(GS_API_NONE, GS_API_NONE)

/**
 *  Notification posted whenever a new thread is started up.  This is a
 *  GNUstep extension.
 */
GS_EXPORT NSString* const NSThreadDidStartNotification;

#endif

#if	!NO_GNUSTEP
#  if	defined(GNUSTEP_BASE_INTERNAL)
#    import	"GNUstepBase/NSThread+GNUstepBase.h"
#  else
#    import	<GNUstepBase/NSThread+GNUstepBase.h>
#  endif
#endif

#if	defined(__cplusplus)
}
#endif

#endif /* __NSThread_h_GNUSTEP_BASE_INCLUDE */
