/** Implementation of NSObject for GNUStep
   Copyright (C) 1994-2017 Free Software Foundation, Inc.

   Written by:  Andrew Kachites McCallum <mccallum@gnu.ai.mit.edu>
   Date: August 1994

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

   <title>NSObject class reference</title>
   $Date$ $Revision$
   */

/* On some versions of mingw we need to work around bad function declarations
 * by defining them away and doing the declarations ourself later.
 */
#ifndef _WIN64
#define InterlockedIncrement	BadInterlockedIncrement
#define InterlockedDecrement	BadInterlockedDecrement
#endif

#import "common.h"
#include <objc/Protocol.h>
#import "Foundation/NSMethodSignature.h"
#import "Foundation/NSInvocation.h"
#import "Foundation/NSLock.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSException.h"
#import "Foundation/NSHashTable.h"
#import "Foundation/NSPortCoder.h"
#import "Foundation/NSDistantObject.h"
#import "Foundation/NSThread.h"
#import "Foundation/NSNotification.h"
#import "Foundation/NSMapTable.h"
#import "Foundation/NSUserDefaults.h"
#import "GNUstepBase/GSLocale.h"
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_MALLOC_H
#include	<malloc.h>
#endif

#import "GSPThread.h"

#if	defined(HAVE_SYS_SIGNAL_H)
#  include	<sys/signal.h>
#elif	defined(HAVE_SIGNAL_H)
#  include	<signal.h>
#endif

#if __GNUC__ >= 4
#if defined(__FreeBSD__)
#include <fenv.h>
#endif
#endif // __GNUC__

#define	IN_NSOBJECT_M	1
#import "GSPrivate.h"


#ifdef __GNUSTEP_RUNTIME__
#include <objc/capabilities.h>
#include <objc/hooks.h>
#ifdef OBJC_CAP_ARC
#include <objc/objc-arc.h>
#endif
#endif


/* platforms which do not support weak */
#if defined (__WIN32)
#define WEAK_ATTRIBUTE
#undef SUPPORT_WEAK
#else
/* all platforms which support weak */
#define WEAK_ATTRIBUTE __attribute__((weak))
#define SUPPORT_WEAK 1
#endif

/* When this is `YES', every call to release/autorelease, checks to
   make sure isn't being set up to release itself too many times.
   This does not need mutex protection. */
static BOOL double_release_check_enabled = NO;

/* The Class responsible for handling autorelease's.  This does not
   need mutex protection, since it is simply a pointer that gets read
   and set. */
static id autorelease_class = nil;
static SEL autorelease_sel;
static IMP autorelease_imp;


static SEL finalize_sel;
static IMP finalize_imp;
static Class	NSConstantStringClass;

@class	NSDataMalloc;
@class	NSMutableDataMalloc;

GS_ROOT_CLASS @interface	NSZombie
{
  Class	isa;
}
- (Class) class;
- (void) forwardInvocation: (NSInvocation*)anInvocation;
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector;
@end

@interface GSContentAccessingProxy : NSProxy
{
  NSObject<NSDiscardableContent> *object;
}
- (id) initWithObject: (id)anObject;
@end

/* allocationLock is needed when for protecting the map table of zombie
 * information and if atomic operations are not available.
 */
static pthread_mutex_t  allocationLock = PTHREAD_MUTEX_INITIALIZER;

BOOL	NSZombieEnabled = NO;
BOOL	NSDeallocateZombies = NO;

@class	NSZombie;
static Class		zombieClass = Nil;
static NSMapTable	*zombieMap = 0;

#ifndef OBJC_CAP_ARC
static void GSMakeZombie(NSObject *o, Class c)
{
  object_setClass(o, zombieClass);
  if (0 != zombieMap)
    {
      pthread_mutex_lock(&allocationLock);
      if (0 != zombieMap)
        {
          NSMapInsert(zombieMap, (void*)o, (void*)c);
        }
      pthread_mutex_unlock(&allocationLock);
    }
}
#endif

static void GSLogZombie(id o, SEL sel)
{
  Class	c = 0;

  if (0 != zombieMap)
    {
      pthread_mutex_lock(&allocationLock);
      if (0 != zombieMap)
        {
          c = NSMapGet(zombieMap, (void*)o);
        }
      pthread_mutex_unlock(&allocationLock);
    }
  if (c == 0)
    {
      NSLog(@"*** -[??? %@]: message sent to deallocated instance %p",
	NSStringFromSelector(sel), o);
    }
  else
    {
      NSLog(@"*** -[%@ %@]: message sent to deallocated instance %p",
	c, NSStringFromSelector(sel), o);
    }
  if (GSPrivateEnvironmentFlag("CRASH_ON_ZOMBIE", NO) == YES)
    {
      abort();
    }
}


/*
 *	Reference count and memory management
 *	Reference counts for object are stored
 *	with the object.
 *	The zone in which an object has been
 *	allocated is stored with the object.
 */

/* Now, if we are on a platform where we know how to do atomic
 * read, increment, and decrement, then we define the GSATOMICREAD
 * macro and macros or functions to increment/decrement.
 * The presence of the GSATOMICREAD macro is used later to determine
 * whether to attempt atomic operations or to use locking for the
 * retain/release mechanism.
 * The GSAtomicIncrement() and GSAtomicDecrement() functions take a
 * pointer to a 32bit integer as an argument, increment/decrement the
 * value pointed to, and return the result.
 */
#ifdef	GSATOMICREAD
#undef	GSATOMICREAD
#endif


/* Traditionally, GNUstep has been using a 32bit reference count in front
 * of the object. The automatic reference counting implementation in
 * libobjc2 uses an intptr_t instead, so NSObject will only be compatible
 * with ARC if either of the following apply:
 *
 * a) sizeof(intptr_t) == sizeof(int32_t)
 * b) we can provide atomic operations on pointer sized values, allowing
 *    us to extend the refcount to intptr_t.
 */
#ifdef GS_ARC_COMPATIBLE
#undef GS_ARC_COMPATIBLE
#endif

#if defined(__llvm__) || (defined(USE_ATOMIC_BUILTINS) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)))
/* Use the GCC atomic operations with recent GCC versions */

typedef intptr_t volatile *gsatomic_t;
typedef intptr_t gsrefcount_t;
#define GSATOMICREAD(X) (*(X))
#define GSAtomicIncrement(X)    __sync_add_and_fetch(X, 1)
#define GSAtomicDecrement(X)    __sync_sub_and_fetch(X, 1)
#define GS_ARC_COMPATIBLE 1

#elif	defined(_WIN32)

/* Set up atomic read, increment and decrement for mswindows
 */

typedef int32_t volatile *gsatomic_t;
typedef int32_t gsrefcount_t;
#ifndef _WIN64
#undef InterlockedIncrement
#undef InterlockedDecrement
LONG WINAPI InterlockedIncrement(LONG volatile *);
LONG WINAPI InterlockedDecrement(LONG volatile *);
#endif

#define	GSATOMICREAD(X)	(*(X))

#define	GSAtomicIncrement(X)	InterlockedIncrement(X)
#define	GSAtomicDecrement(X)	InterlockedDecrement(X)

#elif	defined(__linux__) && (defined(__i386__) || defined(__x86_64__))
/* Set up atomic read, increment and decrement for intel style linux
 */

typedef int32_t volatile *gsatomic_t;
typedef int32_t gsrefcount_t;

#define	GSATOMICREAD(X)	(*(X))

static __inline__ int32_t
GSAtomicIncrement(gsatomic_t X)
{
  register int32_t tmp;
  __asm__ __volatile__ (
    "movl $1, %0\n"
    "lock xaddl %0, %1"
    :"=r" (tmp), "=m" (*X)
    :"r" (tmp), "m" (*X)
    :"memory" );
  return tmp + 1;
}

static __inline__ int32_t
GSAtomicDecrement(gsatomic_t X)
{
  register int32_t tmp;
  __asm__ __volatile__ (
    "movl $1, %0\n"
    "negl %0\n"
    "lock xaddl %0, %1"
    :"=r" (tmp), "=m" (*X)
    :"r" (tmp), "m" (*X)
    :"memory" );
 return tmp - 1;
}

#elif defined(__PPC__) || defined(__POWERPC__)

typedef int32_t volatile *gsatomic_t;
typedef int32_t gsrefcount_t;

#define	GSATOMICREAD(X)	(*(X))

static __inline__ int32_t
GSAtomicIncrement(gsatomic_t X)
{
  int32_t tmp;
  __asm__ __volatile__ (
    "0:"
    "lwarx %0,0,%1 \n"
    "addic %0,%0,1 \n"
    "stwcx. %0,0,%1 \n"
    "bne- 0b \n"
    :"=&r" (tmp)
    :"r" (X)
    :"cc", "memory");
  return tmp;
}

static __inline__ int32_t
GSAtomicDecrement(gsatomic_t X)
{
  int32_t tmp;
  __asm__ __volatile__ (
    "0:"
    "lwarx %0,0,%1 \n"
    "addic %0,%0,-1 \n"
    "stwcx. %0,0,%1 \n"
    "bne- 0b \n"
    :"=&r" (tmp)
    :"r" (X)
    :"cc", "memory");
  return tmp;
}

#elif defined(__m68k__)

typedef int32_t volatile *gsatomic_t;
typedef int32_t gsrefcount_t;

#define	GSATOMICREAD(X)	(*(X))

static __inline__ int32_t
GSAtomicIncrement(gsatomic_t X)
{
  __asm__ __volatile__ (
    "addq%.l %#1, %0"
    :"=m" (*X));
    return *X;
}

static __inline__ int32_t
GSAtomicDecrement(gsatomic_t X)
{
  __asm__ __volatile__ (
    "subq%.l %#1, %0"
    :"=m" (*X));
    return *X;
}

#elif defined(__mips__)

typedef int32_t volatile *gsatomic_t;
typedef int32_t gsrefcount_t;

#define	GSATOMICREAD(X)	(*(X))

static __inline__ int32_t
GSAtomicIncrement(gsatomic_t X)
{
  int32_t tmp;

  __asm__ __volatile__ (
#if !defined(__mips64)
    "   .set  mips2  \n"
#endif
    "0: ll    %0, %1 \n"
    "   addiu %0, 1  \n"
    "   sc    %0, %1 \n"
    "   beqz  %0, 0b  \n"
    :"=&r" (tmp), "=m" (*X));
    return tmp;
}

static __inline__ int32_t
GSAtomicDecrement(gsatomic_t X)
{
  int32_t tmp;

  __asm__ __volatile__ (
#if !defined(__mips64)
    "   .set  mips2  \n"
#endif
    "0: ll    %0, %1 \n"
    "   addiu %0, -1 \n"
    "   sc    %0, %1 \n"
    "   beqz  %0, 0b  \n"
    :"=&r" (tmp), "=m" (*X));
    return tmp;
}
#endif

#if	!defined(GSATOMICREAD)

typedef int     gsrefcount_t;   // No atomics, use a simple integer

/* Having just one allocationLock for all leads to lock contention
 * if there are lots of threads doing lots of retain/release calls.
 * To alleviate this, instead of a single
 * allocationLock for all objects, we divide the object space into
 * chunks, each with its own lock. The chunk is selected by shifting
 * off the low-order ALIGNBITS of the object's pointer (these bits
 * are presumably always zero) and take
 * the low-order LOCKBITS of the result to index into a table of locks.
 */

#define LOCKBITS 5
#define LOCKCOUNT (1<<LOCKBITS)
#define LOCKMASK (LOCKCOUNT-1)
#define ALIGNBITS 3

static pthread_mutex_t  allocationLocks[LOCKCOUNT];

static inline pthread_mutex_t   *GSAllocationLockForObject(id p)
{
  NSUInteger i = ((((NSUInteger)(uintptr_t)p) >> ALIGNBITS) & LOCKMASK);
  return &allocationLocks[i];
}

#endif

#ifndef GS_ARC_COMPATIBLE
/*
 * If we haven't previously declared that we can work in fast-ARC mode,
 * check whether a point is 32bit (4 bytes) wide, which also enables ARC
 * integration.
 */
#  if __SIZEOF_POINTER__ == 4
#    define GS_ARC_COMPATIBLE 1
#  endif
#endif

#if defined(__GNUC__) && __GNUC__ < 4
#define __builtin_offsetof(s, f) (uintptr_t)(&(((s*)0)->f))
#endif
#define alignof(type) __builtin_offsetof(struct { const char c; type member; }, member)

/*
 *	Define a structure to hold information that is held locally
 *	(before the start) in each object.
 */
typedef struct obj_layout_unpadded {
  gsrefcount_t	retained;
} unp;
#define	UNP sizeof(unp)

/* GCC provides a defined value for the largest alignment required on a
 * machine, and we must lay objects out to that alignment.
 * For compilers that don't define it, we try to pick a likely value.
 */
#ifndef	__BIGGEST_ALIGNMENT__
#define	__BIGGEST_ALIGNMENT__ (SIZEOF_VOIDP * 2)
#endif

/*
 *	Now do the REAL version - using the other version to determine
 *	what padding (if any) is required to get the alignment of the
 *	structure correct.
 */
struct obj_layout {
  char	padding[__BIGGEST_ALIGNMENT__ - ((UNP % __BIGGEST_ALIGNMENT__)
    ? (UNP % __BIGGEST_ALIGNMENT__) : __BIGGEST_ALIGNMENT__)];
  gsrefcount_t	retained;
};
typedef	struct obj_layout *obj;

/*
 * These symbols are provided by newer versions of the GNUstep Objective-C
 * runtime.  When linked against an older version, we will use our internal
 * versions.
 */
WEAK_ATTRIBUTE
BOOL objc_release_fast_no_destroy_np(id anObject);

WEAK_ATTRIBUTE
void objc_release_fast_np(id anObject);

WEAK_ATTRIBUTE
size_t object_getRetainCount_np(id anObject);

WEAK_ATTRIBUTE
id objc_retain_fast_np(id anObject);


static BOOL objc_release_fast_no_destroy_internal(id anObject)
{
  if (double_release_check_enabled)
    {
      NSUInteger release_count;
      NSUInteger retain_count = [anObject retainCount];
      release_count = [autorelease_class autoreleaseCountForObject: anObject];
      if (release_count >= retain_count)
        [NSException raise: NSGenericException
		    format: @"Release would release object too many times."];
    }
  {
#if	defined(GSATOMICREAD)
    gsrefcount_t	result;

    result = GSAtomicDecrement((gsatomic_t)&(((obj)anObject)[-1].retained));
    if (result < 0)
      {
        if (result != -1)
          {
            [NSException raise: NSInternalInconsistencyException
              format: @"NSDecrementExtraRefCount() decremented too far"];
          }
        /* The counter has become negative so it must have been zero.
         * We reset it and return YES ... in a correctly operating
         * process we know we can safely reset back to zero without
         * worrying about atomicity, since there can be no other
         * thread accessing the object (or its reference count would
         * have been greater than zero)
         */
        (((obj)anObject)[-1].retained) = 0;
#  ifdef OBJC_CAP_ARC
        objc_delete_weak_refs(anObject);
#  endif
        return YES;
      }
#else	/* GSATOMICREAD */
    pthread_mutex_t *theLock = GSAllocationLockForObject(anObject);

    pthread_mutex_lock(theLock);
    if (((obj)anObject)[-1].retained == 0)
      {
#  ifdef OBJC_CAP_ARC
        objc_delete_weak_refs(anObject);
#  endif
        pthread_mutex_unlock(theLock);
        return YES;
      }
    else
      {
        ((obj)anObject)[-1].retained--;
        pthread_mutex_unlock(theLock);
        return NO;
      }
#endif	/* GSATOMICREAD */
  }
  return NO;
}

static BOOL release_fast_no_destroy(id anObject)
{
#ifdef SUPPORT_WEAK
  if (objc_release_fast_no_destroy_np)
    {
      return objc_release_fast_no_destroy_np(anObject);
    }
  else
#endif
    {
      return objc_release_fast_no_destroy_internal(anObject);
    }
}

static void objc_release_fast_np_internal(id anObject)
{
  if (release_fast_no_destroy(anObject))
    {
      [anObject dealloc];
    }
}

static void release_fast(id anObject)
{
#ifdef SUPPORT_WEAK
  if (objc_release_fast_np)
    {
      objc_release_fast_np(anObject);
    }
  else
#endif
    {
      objc_release_fast_np_internal(anObject);
    }
}

/**
 * Examines the extra reference count for the object and, if non-zero
 * decrements it, otherwise leaves it unchanged.<br />
 * Returns a flag to say whether the count was zero
 * (and hence whether the extra reference count was decremented).<br />
 */
inline BOOL
NSDecrementExtraRefCountWasZero(id anObject)
{
  return release_fast_no_destroy(anObject);
}

size_t object_getRetainCount_np_internal(id anObject)
{
  return ((obj)anObject)[-1].retained + 1;
}

size_t getRetainCount(id anObject)
{
#ifdef SUPPORT_WEAK
  if (object_getRetainCount_np)
    {
      return object_getRetainCount_np(anObject);
    }
  else
#endif
    {
      return object_getRetainCount_np_internal(anObject);
    }
}

/**
 * Return the extra reference count of anObject (a value in the range
 * from 0 to the maximum unsigned integer value minus one).<br />
 * The retain count for an object is this value plus one.
 */
inline NSUInteger
NSExtraRefCount(id anObject)
{
  return getRetainCount(anObject) - 1;
}

/**
 * Increments the extra reference count for anObject.<br />
 * The GNUstep version raises an exception if the reference count
 * would be incremented to too large a value.<br />
 * This is used by the [NSObject-retain] method.
 */
static id objc_retain_fast_np_internal(id anObject)
{
  BOOL  tooFar = NO;

#if	defined(GSATOMICREAD)
  /* I've seen comments saying that some platforms only support up to
   * 24 bits in atomic locking, so raise an exception if we try to
   * go beyond 0xfffffe.
   */
  if (GSAtomicIncrement((gsatomic_t)&(((obj)anObject)[-1].retained))
    > 0xfffffe)
    {
      tooFar = YES;
    }
#else	/* GSATOMICREAD */
  pthread_mutex_t *theLock = GSAllocationLockForObject(anObject);

  pthread_mutex_lock(theLock);
  if (((obj)anObject)[-1].retained > 0xfffffe)
    {
      tooFar = YES;
    }
  else
    {
      ((obj)anObject)[-1].retained++;
    }
  pthread_mutex_unlock(theLock);
#endif	/* GSATOMICREAD */
  if (YES == tooFar)
    {
      static NSHashTable        *overrun = nil;

      /* We store this instance in a hash table so that we will only raise
       * an exception for it once (and can therefore expect to log the instance
       * as part of the exception derscription without recursion).
       * NB. The hash table does not retain the object, so the code in the
       * lock protected region below should be safe anyway.
       */
      [gnustep_global_lock lock];
      if (nil == overrun)
        {
          overrun = NSCreateHashTable(NSNonRetainedObjectHashCallBacks, 0);
        }
      if (0 == NSHashGet(overrun, anObject))
        {
          NSHashInsert(overrun, anObject);
        }
      else
        {
          tooFar = NO;
        }
      [gnustep_global_lock lock];
      if (YES == tooFar)
        {
          NSString      *base;

          base = [NSString stringWithFormat: @"<%s: %p>",
            class_getName([anObject class]), anObject];
          [NSException raise: NSInternalInconsistencyException
            format: @"NSIncrementExtraRefCount() asked to increment too far"
            @" for %@ - %@", base, anObject];
        }
    }
  return anObject;
}

static id retain_fast(id anObject)
{
#ifdef SUPPORT_WEAK
  if (objc_retain_fast_np)
    {
      return objc_retain_fast_np(anObject);
    }
  else
#endif
    {
      return objc_retain_fast_np_internal(anObject);
    }
}

/**
 * Increments the extra reference count for anObject.<br />
 * The GNUstep version raises an exception if the reference count
 * would be incremented to too large a value.<br />
 * This is used by the [NSObject-retain] method.
 */
inline void
NSIncrementExtraRefCount(id anObject)
{
   retain_fast(anObject);
}

#ifndef	NDEBUG
#define	AADD(c, o) GSDebugAllocationAdd(c, o)
#define	AREM(c, o) GSDebugAllocationRemove(c, o)
#else
#define	AADD(c, o)
#define	AREM(c, o)
#endif


#ifndef OBJC_CAP_ARC
static SEL cxx_construct, cxx_destruct;

/**
 * Calls the C++ constructors for this object, starting with the ones declared
 * in aClass.  The compiler generates two methods on Objective-C++ classes that
 * static instances of C++ classes as ivars.  These are -.cxx_construct and
 * -.cxx_destruct.  The -.cxx_construct methods must be called in order from
 *  the root class to all subclasses, to ensure that subclass ivars are
 *  initialised after superclass ones.  This must be done in reverse for
 *  destruction.
 *
 *  This function first calls itself recursively on the superclass, to get the
 *  IMP for the constructor function in the superclass.  It then compares the
 *  construct method for this class with the one that's already been called,
 *  and calls it if it's new.
 */
static IMP
callCXXConstructors(Class aClass, id anObject)
{
  IMP constructor = 0;

  if (class_respondsToSelector(aClass, cxx_construct))
    {
      IMP calledConstructor =
        callCXXConstructors(class_getSuperclass(aClass), anObject);
      constructor = class_getMethodImplementation(aClass, cxx_construct);
      if (calledConstructor != constructor)
        {
          constructor(anObject, cxx_construct);
        }
    }
  return constructor;
}
#endif


/*
 *	Now do conditional compilation of memory allocation functions
 *	depending on what information (if any) we are storing before
 *	the start of each object.
 */

// FIXME rewrite object allocation to use class_createInstance when we
// are using libobjc2.
inline id
NSAllocateObject(Class aClass, NSUInteger extraBytes, NSZone *zone)
{
  id	new;

#ifdef OBJC_CAP_ARC
  if ((new = class_createInstance(aClass, extraBytes)) != nil)
    {
      AADD(aClass, new);
    }
#else
  int	size;

  NSCAssert((!class_isMetaClass(aClass)), @"Bad class for new object");
  size = class_getInstanceSize(aClass) + extraBytes + sizeof(struct obj_layout);
  if (zone == 0)
    {
      zone = NSDefaultMallocZone();
    }
  new = NSZoneMalloc(zone, size);
  if (new != nil)
    {
      memset (new, 0, size);
      new = (id)&((obj)new)[1];
      object_setClass(new, aClass);
      AADD(aClass, new);
    }

  /* Don't bother doing this in a thread-safe way, because the cost of locking
   * will be a lot more than the cost of doing the same call in two threads.
   * The returned selector will persist and the runtime will ensure that both
   * calls return the same selector, so we don't need to bother doing it
   * ourselves.
   */
  if (0 == cxx_construct)
    {
      cxx_construct = sel_registerName(".cxx_construct");
      cxx_destruct = sel_registerName(".cxx_destruct");
    }
  callCXXConstructors(aClass, new);
#endif

  return new;
}

inline void
NSDeallocateObject(id anObject)
{
  Class aClass = object_getClass(anObject);

  if ((anObject != nil) && !class_isMetaClass(aClass))
    {
#ifndef OBJC_CAP_ARC
      obj	o = &((obj)anObject)[-1];
      NSZone	*z = NSZoneFromPointer(o);
#endif

      /* Call the default finalizer to handle C++ destructors.
       */
      (*finalize_imp)(anObject, finalize_sel);

      AREM(aClass, (id)anObject);
      if (NSZombieEnabled == YES)
	{
#ifdef OBJC_CAP_ARC
	  if (0 != zombieMap)
	    {
              pthread_mutex_lock(&allocationLock);
              if (0 != zombieMap)
                {
                  NSMapInsert(zombieMap, (void*)anObject, (void*)aClass);
                }
              pthread_mutex_unlock(&allocationLock);
	    }
	  if (NSDeallocateZombies == YES)
	    {
	      object_dispose(anObject);
	    }
	  else
	    {
	      object_setClass(anObject, zombieClass);
	    }
#else
	  GSMakeZombie(anObject, aClass);
	  if (NSDeallocateZombies == YES)
	    {
	      NSZoneFree(z, o);
	    }
#endif
	}
      else
	{
#ifdef OBJC_CAP_ARC
	  object_dispose(anObject);
#else
	  object_setClass((id)anObject, (Class)(void*)0xdeadface);
	  NSZoneFree(z, o);
#endif
	}
    }
  return;
}

BOOL
NSShouldRetainWithZone (NSObject *anObject, NSZone *requestedZone)
{
  return (!requestedZone || requestedZone == NSDefaultMallocZone()
    || [anObject zone] == requestedZone);
}



/**
 * <p>
 *   <code>NSObject</code> is the root class (a root class is
 *   a class with no superclass) of the GNUstep base library
 *   class hierarchy, so all classes normally inherit from
 *   <code>NSObject</code>.  There is an exception though:
 *   <code>NSProxy</code> (which is used for remote messaging)
 *   does not inherit from <code>NSObject</code>.
 * </p>
 * <p>
 *   Unless you are really sure of what you are doing, all
 *   your own classes should inherit (directly or indirectly)
 *   from <code>NSObject</code> (or in special cases from
 *   <code>NSProxy</code>).  <code>NSObject</code> provides
 *   the basic common functionality shared by all GNUstep
 *   classes and objects.
 * </p>
 * <p>
 *   The essential methods which must be implemented by all
 *   classes for their instances to be usable within GNUstep
 *   are declared in a separate protocol, which is the
 *   <code>NSObject</code> protocol.  Both
 *   <code>NSObject</code> and <code>NSProxy</code> conform to
 *   this protocol, which means all objects in a GNUstep
 *   application will conform to this protocol (btw, if you
 *   don't find a method of <code>NSObject</code> you are
 *   looking for in this documentation, make sure you also
 *   look into the documentation for the <code>NSObject</code>
 *   protocol).
 * </p>
 * <p>
 *   Theoretically, in special cases you might need to
 *   implement a new root class.  If you do, you need to make
 *   sure that your root class conforms (at least) to the
 *   <code>NSObject</code> protocol, otherwise it will not
 *   interact correctly with the GNUstep framework.  Said
 *   that, I must note that I have never seen a case in which
 *   a new root class is needed.
 * </p>
 * <p>
 *   <code>NSObject</code> is a root class, which implies that
 *   instance methods of <code>NSObject</code> are treated in
 *   a special way by the Objective-C runtime.  This is an
 *   exception to the normal way messaging works with class
 *   and instance methods: if the Objective-C runtime can't
 *   find a class method for a class object, as a last resort
 *   it looks for an instance method of the root class with
 *   the same name, and executes it if it finds it.  This
 *   means that instance methods of the root class (such as
 *   <code>NSObject</code>) can be performed by class objects
 *   which inherit from that root class !  This can only
 *   happen if the class doesn't have a class method with the
 *   same name, otherwise that method - of course - takes the
 *   precedence.  Because of this exception,
 *   <code>NSObject</code>'s instance methods are written in
 *   such a way that they work both on <code>NSObject</code>'s
 *   instances and on class objects.
 * </p>
 */
@implementation NSObject
#if  defined(GS_ARC_COMPATIBLE)
- (void)_ARCCompliantRetainRelease {}
#endif

/**
 * Semi-private function in libobjc2 that initialises the classes used for
 * blocks.
 */
extern BOOL
objc_create_block_classes_as_subclasses_of(Class super);

#ifdef OBJC_CAP_ARC
static id gs_weak_load(id obj)
{
  return [obj retainCount] > 0 ? obj : nil;
}
#endif

+ (void) load
{
#ifdef OBJC_CAP_ARC
  _objc_weak_load = gs_weak_load;
#endif
  objc_create_block_classes_as_subclasses_of(self);
}

+ (void) initialize
{
  if (self == [NSObject class])
    {
#ifdef _WIN32
      {
        // See libgnustep-base-entry.m
        extern void gnustep_base_socket_init(void);
        gnustep_base_socket_init();
      }
#else /* _WIN32 */

#ifdef	SIGPIPE
    /*
     * If SIGPIPE is not handled or ignored, we will abort on any attempt
     * to write to a pipe/socket that has been closed by the other end!
     * We therefore need to ignore the signal if nothing else is already
     * handling it.
     */
#ifdef	HAVE_SIGACTION
      {
	struct sigaction	act;

	if (sigaction(SIGPIPE, 0, &act) == 0)
	  {
	    if (act.sa_handler == SIG_DFL)
	      {
		// Not ignored or handled ... so we ignore it.
		act.sa_handler = SIG_IGN;
		if (sigaction(SIGPIPE, &act, 0) != 0)
		  {
		    fprintf(stderr, "Unable to ignore SIGPIPE\n");
		  }
	      }
	  }
	else
	  {
	    fprintf(stderr, "Unable to retrieve information about SIGPIPE\n");
	  }
      }
#else /* HAVE_SIGACTION */
      {
	void	(*handler)(NSInteger);

	handler = signal(SIGPIPE, SIG_IGN);
	if (handler != SIG_DFL)
	  {
	    signal(SIGPIPE, handler);
	  }
      }
#endif /* HAVE_SIGACTION */
#endif /* SIGPIPE */
#endif /* _WIN32 */

      finalize_sel = @selector(finalize);
      finalize_imp = class_getMethodImplementation(self, finalize_sel);

#if defined(__FreeBSD__) && defined(__i386__)
      // Manipulate the FPU to add the exception mask. (Fixes SIGFPE
      // problems on *BSD)
      // Note this only works on x86
#  if defined(FE_INVALID)
      fedisableexcept(FE_INVALID);
#  else
      {
        volatile short cw;

        __asm__ volatile ("fstcw (%0)" : : "g" (&cw));
        cw |= 1; /* Mask 'invalid' exception */
        __asm__ volatile ("fldcw (%0)" : : "g" (&cw));
      }
#  endif
#endif

      /* Initialize the locks for allocation when atomic
       * operations are not available.
       */
#if !defined(GSATOMICREAD)
      {
        NSUInteger	i;

        for (i = 0; i < LOCKCOUNT; i++)
          {
            pthread_mutex_init(&allocationLocks[i], NULL);
          }
      }
#endif

      /* Create the global lock.
       * NB. Ths is one of the first things we do ... setting up a new lock
       * must not call any other Objective-C classes and must not involve
       * any use of the autorelease system.
       */
      gnustep_global_lock = [GSUntracedRecursiveLock new];

      /* Behavior debugging ... enable with environment variable if needed.
       */
      GSObjCBehaviorDebug(GSPrivateEnvironmentFlag("GNUSTEP_BEHAVIOR_DEBUG",
	GSObjCBehaviorDebug(-1)));

      /* See if we should cleanup at process exit.
       */
      if (YES == GSPrivateEnvironmentFlag("GNUSTEP_SHOULD_CLEAN_UP", NO))
	{
	  [self setShouldCleanUp: YES];
	  [self registerAtExit: @selector(_atExit)];
	}

      /* Set up the autorelease system ... we must do this before using any
       * other class whose +initialize might autorelease something.
       */
      autorelease_class = [NSAutoreleasePool class];
      autorelease_sel = @selector(addObject:);
      autorelease_imp = [autorelease_class methodForSelector: autorelease_sel];

      /* Make sure the constant string class works and set up well-known
       * string constants etc.
       */
      NSConstantStringClass = [NSString constantStringClass];

      GSPrivateBuildStrings();

      /* Now that the string class (and autorelease) is set up, we can set
       * the name of the lock to a string value safely.
       */
      [gnustep_global_lock setName: @"gnustep_global_lock"];

      /* Determine zombie management flags and set up a map to store
       * information about zombie objects.
       */
      NSZombieEnabled = GSPrivateEnvironmentFlag("NSZombieEnabled", NO);
      NSDeallocateZombies = GSPrivateEnvironmentFlag("NSDeallocateZombies", NO);
      zombieMap = NSCreateMapTable(NSNonOwnedPointerMapKeyCallBacks,
	NSNonOwnedPointerMapValueCallBacks, 0);

      /* We need to cache the zombie class.
       * We can't call +class because NSZombie doesn't have that method.
       * We can't use NSClassFromString() because that would use an NSString
       * object, and that class hasn't been initialized yet ...
       */
      zombieClass = objc_lookUpClass("NSZombie");
    }
  return;
}

+ (void) _atExit
{
  NSMapTable	*m = nil;
  pthread_mutex_lock(&allocationLock);
  m = zombieMap;
  zombieMap = nil;
  pthread_mutex_unlock(&allocationLock);
  DESTROY(m);
}

/**
 * Allocates a new instance of the receiver from the default
 * zone, by invoking +allocWithZone: with
 * <code>NSDefaultMallocZone()</code> as the zone argument.<br />
 * Returns the created instance.
 */
+ (id) alloc
{
  return [self allocWithZone: NSDefaultMallocZone()];
}

/**
 * This is the basic method to create a new instance.  It
 * allocates a new instance of the receiver from the specified
 * memory zone.
 * <p>
 *   Memory for an instance of the receiver is allocated; a
 *   pointer to this newly created instance is returned.  All
 *   instance variables are set to 0.  No initialization of the
 *   instance is performed apart from setup to be an instance of
 *   the correct class: it is your responsibility to initialize the
 *   instance by calling an appropriate <code>init</code>
 *   method.  If you are not using the garbage collector, it is
 *   also your responsibility to make sure the returned
 *   instance is destroyed when you finish using it, by calling
 *   the <code>release</code> method to destroy the instance
 *   directly, or by using <code>autorelease</code> and
 *   autorelease pools.
 * </p>
 * <p>
 *  You do not normally need to override this method in
 *  subclasses, unless you are implementing a class which for
 *  some reasons silently allocates instances of another class
 *  (this is typically needed to implement class clusters and
 *  similar design schemes).
 * </p>
 * <p>
 *   If you have turned on debugging of object allocation (by
 *   calling the <code>GSDebugAllocationActive</code>
 *   function), this method will also update the various
 *   debugging counts and monitors of allocated objects, which
 *   you can access using the <code>GSDebugAllocation...</code>
 *   functions.
 * </p>
 */
+ (id) allocWithZone: (NSZone*)z
{
  return NSAllocateObject(self, 0, z);
}

/**
 * Returns the receiver.
 */
+ (id) copyWithZone: (NSZone*)z
{
  return self;
}

/**
 * <p>
 *   This method is a short-hand for alloc followed by init, that is,
 * </p>
 * <p><code>
 *    NSObject *object = [NSObject new];
 * </code></p>
 * is exactly the same as
 * <p><code>
 *    NSObject *object = [[NSObject alloc] init];
 * </code></p>
 * <p>
 *   This is a general convention: all <code>new...</code>
 *   methods are supposed to return a newly allocated and
 *   initialized instance, as would be generated by an
 *   <code>alloc</code> method followed by a corresponding
 *   <code>init...</code> method.  Please note that if you are
 *   not using a garbage collector, this means that instances
 *   generated by the <code>new...</code> methods are not
 *   autoreleased, that is, you are responsible for releasing
 *   (autoreleasing) the instances yourself.  So when you use
 *   <code>new</code> you typically do something like:
 * </p>
 * <p>
 *   <code>
 *      NSMutableArray *array = AUTORELEASE ([NSMutableArray new]);
 *   </code>
 * </p>
 * <p>
 *   You do not normally need to override <code>new</code> in
 *   subclasses, because if you override <code>init</code> (and
 *   optionally <code>allocWithZone:</code> if you really
 *   need), <code>new</code> will automatically use your
 *   subclass methods.
 * </p>
 * <p>
 *   You might need instead to define new <code>new...</code>
 *   methods specific to your subclass to match any
 *   <code>init...</code> specific to your subclass.  For
 *   example, if your subclass defines an instance method
 * </p>
 * <p>
 *   <code>initWithName:</code>
 * </p>
 * <p>
 *   it might be handy for you to have a class method
 * </p>
 * <p>
 *    <code>newWithName:</code>
 * </p>
 * <p>
 *   which combines <code>alloc</code> and
 *   <code>initWithName:</code>.  You would implement it as follows:
 * </p>
 * <p>
 *   <code>
 *     + (id) newWithName: (NSString *)aName
 *     {
 *       return [[self alloc] initWithName: aName];
 *     }
 *   </code>
 * </p>
 */
+ (id) new
{
  return [[self alloc] init];
}

/**
 * Returns the class of which the receiver is an instance.<br />
 * The default implementation returns the actual class that the
 * receiver is an instance of.<br />
 * NB.  When NSZombie is enabled (see NSDebug.h) this is changed
 * to be the NSZombie class upon object deallocation.
 */
- (Class) class
{
  return object_getClass(self);
}

/**
 * Returns the name of the class of the receiving object by using
 * the NSStringFromClass() function.<br />
 * This is a MacOS-X addition for apple scripting, which is also
 * generally useful.
 */
- (NSString*) className
{
  return NSStringFromClass([self class]);
}

/**
 * Creates and returns a copy of the receiver by calling -copyWithZone:
 * passing NSDefaultMallocZone()
 */
- (id) copy
{
  return [(id)self copyWithZone: NSDefaultMallocZone()];
}

/**
 * Deallocates the receiver by calling NSDeallocateObject() with self
 * as the argument.<br />
 * <p>
 *   You should normally call the superclass implementation of this method
 *   when you override it in a subclass, or the memory occupied by your
 *   object will not be released.
 * </p>
 * <p>
 *   <code>NSObject</code>'s implementation of this method
 *   destroys the receiver, by returning the memory allocated
 *   to the receiver to the system.  After this method has been
 *   called on an instance, you must not refer the instance in
 *   any way, because it does not exist any longer.  If you do,
 *   it is a bug and your program might even crash with a
 *   segmentation fault.
 * </p>
 * <p>
 *   If you have turned on the debugging facilities for
 *   instance allocation, <code>NSObject</code>'s
 *   implementation of this method will also update the various
 *   counts and monitors of allocated instances (see the
 *   <code>GSDebugAllocation...</code> functions for more
 *   info).
 * </p>
 * <p>
 *   Normally you are supposed to manage the memory taken by
 *   objects by using the high level interface provided by the
 *   <code>retain</code>, <code>release</code> and
 *   <code>autorelease</code> methods (or better by the
 *   corresponding macros <code>RETAIN</code>,
 *   <code>RELEASE</code> and <code>AUTORELEASE</code>), and by
 *   autorelease pools and such; whenever the
 *   release/autorelease mechanism determines that an object is
 *   no longer needed (which happens when its retain count
 *   reaches 0), it will call the <code>dealloc</code> method
 *   to actually deallocate the object.  This means that normally,
 *   you should not need to call <code>dealloc</code> directly as
 *   the gnustep base library automatically calls it for you when
 *   the retain count of an object reaches 0.
 * </p>
 * <p>
 *   Because the <code>dealloc</code> method will be called
 *   when an instance is being destroyed, if instances of your
 *   subclass use objects or resources (as it happens for most
 *   useful classes), you must override <code>dealloc</code> in
 *   subclasses to release all objects and resources which are
 *   used by the instance, otherwise these objects and
 *   resources would be leaked.  In the subclass
 *   implementation, you should first release all your subclass
 *   specific objects and resources, and then invoke super's
 *   implementation (which will do the same, and so on up in
 *   the class hierarchy to <code>NSObject</code>'s
 *   implementation, which finally destroys the object).  Here
 *   is an example of the implementation of
 *   <code>dealloc</code> for a subclass whose instances have a
 *   single instance variable <code>name</code> which needs to
 *   be released when an instance is deallocated:
 * </p>
 * <p>
 *   <code>
 *   - (void) dealloc
 *   {
 *     RELEASE (name);
 *     [super dealloc];
 *   }
 *   </code>
 *  </p>
 *  <p>
 *    <code>dealloc</code> might contain code to release not
 *    only objects, but also other resources, such as open
 *    files, network connections, raw memory allocated in other
 *    ways, etc.
 *  </p>
 * <p>
 *   If you have allocated the memory using a non-standard mechanism, you
 *   will not call the superclass (NSObject) implementation of the method
 *   as you will need to handle the deallocation specially.<br />
 *   In some circumstances, an object may wish to prevent itself from
 *   being deallocated, it can do this simply be refraining from calling
 *   the superclass implementation.
 * </p>
 */
- (void) dealloc
{
  NSDeallocateObject(self);
}

- (void) finalize
{
#ifndef OBJC_CAP_ARC
  Class	destructorClass = Nil;
  IMP	  destructor = 0;
  /*
   * We're pretending to be the Objective-C runtime here, so we have to do some
   * unsafe things (i.e. access the class directly, and not via the
   * object_getClass() so that hidden classes get their destructors called.  If
   * the runtime supports small objects (those embedded in a pointer), then we
   * must use object_getClass() for them, because they do not have an isa
   * pointer (but can not have a hidden class interposed).
   */
#ifdef	__clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-objc-pointer-introspection"
#pragma clang diagnostic ignored "-Wdeprecated-objc-isa-usage"
#endif
#ifdef OBJC_SMALL_OBJECT_MASK
  if (((NSUInteger)self & OBJC_SMALL_OBJECT_MASK) == 0)
    {
      destructorClass = isa;                    // Potentially hidden class
    }
  else
    {
      destructorClass = object_getClass(self);  // Small object
    }
#else
  destructorClass = isa;
#endif
#ifdef	__clang__
#pragma clang diagnostic pop
#endif

  /* C++ destructors must be called in the opposite order to their
   * creators, so start at the leaf class and then go up the tree until we
   * get to the root class.  As a small optimisation, we don't bother
   * visiting any classes that don't have an implementation of this method
   * (including one inherited from a superclass).
   *
   * Care must be taken not to call inherited .cxx_destruct methods.
   */
  while (class_respondsToSelector(destructorClass, cxx_destruct))
    {
      IMP newDestructor;

      newDestructor
	= class_getMethodImplementation(destructorClass, cxx_destruct);
      destructorClass = class_getSuperclass(destructorClass);

      if (newDestructor != destructor)
	{
	  newDestructor(self, cxx_destruct);
	  destructor = newDestructor;
	}
    }
  return;
#endif
}

/**
 *  This method is an anachronism.  Do not use it.
 */
- (id) free
{
  [NSException raise: NSGenericException
	      format: @"Use `dealloc' instead of `free' for %@.", self];
  return nil;
}

/**
 * Initialises the receiver ... the NSObject implementation simply returns self.
 */
- (id) init
{
  return self;
}

/**
 * Creates and returns a mutable copy of the receiver by calling
 * -mutableCopyWithZone: passing NSDefaultMallocZone().
 */
- (id) mutableCopy
{
  return [(id)self mutableCopyWithZone: NSDefaultMallocZone()];
}

/**
 * Returns the super class from which the receiver was derived.
 */
+ (Class) superclass
{
  return class_getSuperclass(self);
}

/**
 * Returns the super class from which the receivers class was derived.
 */
- (Class) superclass
{
  return class_getSuperclass(object_getClass(self));
}

/**
 * Returns a flag to say if instances of the receiver class will
 * respond to the specified selector.  This ignores situations
 * where a subclass implements -forwardInvocation: to respond to
 * selectors not normally handled ... in these cases the subclass
 * may override this method to handle it.
 * <br />If given a null selector, raises NSInvalidArgumentException when
 * in MacOS-X compatibility more, or returns NO otherwise.
 */
+ (BOOL) instancesRespondToSelector: (SEL)aSelector
{
  if (aSelector == 0)
    {
      if (GSPrivateDefaultsFlag(GSMacOSXCompatible))
	{
	  [NSException raise: NSInvalidArgumentException
		    format: @"%@ null selector given",
	    NSStringFromSelector(_cmd)];
	}
      return NO;
    }

  if (class_respondsToSelector(self, aSelector))
    {
      return YES;
    }

  if (class_isMetaClass(self))
    {
      /* It seems convoluted to attempt to access the class from the 
         metaclass just to call +resolveClassMethod: in this rare case. */
      return NO;
    }
  else
    {
      return [self resolveInstanceMethod: aSelector];
    }
}

/**
 * Returns a flag to say whether the receiving class conforms to aProtocol
 */
+ (BOOL) conformsToProtocol: (Protocol*)aProtocol
{
#ifdef __GNU_LIBOBJC__
  Class c;

  /* Iterate over the current class and all the superclasses.  */
  for (c = self; c != Nil; c = class_getSuperclass (c))
    {
      if (class_conformsToProtocol(c, aProtocol))
	{
	  return YES;
	}
    }

  return NO;
#else
  /* libobjc2 and ObjectiveC2/ have an implementation of
     class_conformsToProtocol() which automatically looks up the
     protocol in superclasses (unlike the Apple and GNU Objective-C
     runtime ones).  */
  return class_conformsToProtocol(self, aProtocol);
#endif
}

/**
 * Returns a flag to say whether the class of the receiver conforms
 * to aProtocol.
 */
- (BOOL) conformsToProtocol: (Protocol*)aProtocol
{
  return [[self class] conformsToProtocol: aProtocol];
}

/**
 * Returns a pointer to the C function implementing the method used
 * to respond to messages with aSelector by instances of the receiving
 * class.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
+ (IMP) instanceMethodForSelector: (SEL)aSelector
{
  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];
  /*
   * Since 'self' is an class, class_getMethodImplementation() will get
   * the instance method.
   */
  return class_getMethodImplementation((Class)self, aSelector);
}

/**
 * Returns a pointer to the C function implementing the method used
 * to respond to messages with aSelector.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
- (IMP) methodForSelector: (SEL)aSelector
{
  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];
  /* The Apple runtime API would do:
   * return class_getMethodImplementation(object_getClass(self), aSelector);
   * but this cannot ask self for information about any method reached by
   * forwarding, so the returned forwarding function would ge a generic one
   * rather than one aware of hardware issues with returning structures
   * and floating points.  We therefore prefer the GNU API which is able to
   * use forwarding callbacks to get better type information.
   */
  return objc_msg_lookup(self, aSelector);
}

/**
 * Returns a pointer to the C function implementing the method used
 * to respond to messages with aSelector which are sent to instances
 * of the receiving class.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
+ (NSMethodSignature*) instanceMethodSignatureForSelector: (SEL)aSelector
{
  struct objc_method	*mth;

  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  mth = GSGetMethod(self, aSelector, YES, YES);
  if (0 == mth)
    return nil;
  return [NSMethodSignature
    signatureWithObjCTypes: method_getTypeEncoding(mth)];
}

/**
 * Returns the method signature describing how the receiver would handle
 * a message with aSelector.
 * <br />Returns nil if given a null selector.
 */
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  const char	*types = NULL;
  Class		c;
  unsigned int	count;
  Protocol	**protocols;

  if (0 == aSelector)
    {
      return nil;
    }

  c = object_getClass(self);

  /* Do a fast lookup to see if the method is implemented at all.  If it isn't,
   * we can give up without doing a very expensive linear search through every
   * method list in the class hierarchy.
   */
  if (!class_respondsToSelector(c, aSelector))
    {
      return nil; // Method not implemented
    }

  /* If there are protocols that this class conforms to,
   * the method may be listed in a protocol with more
   * detailed type information than in the class itself
   * and we must therefore use the information from the
   * protocol.
   * This is because protocols also carry information
   * used by the Distributed Objects system, which the
   * runtime does not maintain in classes.
   */
  protocols = class_copyProtocolList(c, &count);
  if (NULL != protocols)
    {
      struct objc_method_description mth;
      int i;

      for (i = 0 ; i < count ; i++)
        {
          mth = GSProtocolGetMethodDescriptionRecursive(protocols[i],
	    aSelector, YES, YES);
          if (NULL == mth.types)
            {
              // Search for class method
              mth = GSProtocolGetMethodDescriptionRecursive(protocols[i],
		aSelector, YES, NO);
              // FIXME: We should probably search optional methods here too.
            }

          if (NULL != mth.types)
	    {
	      break;
	    }
        }
      free(protocols);
    }

  if (types == 0)
    {
#ifdef __GNUSTEP_RUNTIME__
      struct objc_slot	*objc_get_slot(Class cls, SEL selector);
      struct objc_slot	*slot = objc_get_slot(object_getClass(self), aSelector);
      types = slot->types;
#else
      struct objc_method *mth;
      if (GSObjCIsInstance(self))
	{
	  mth = GSGetMethod(object_getClass(self), aSelector, YES, YES);
	}
      else
	{
	  mth = GSGetMethod((Class)self, aSelector, NO, YES);
	}
      types = method_getTypeEncoding (mth);
#endif
    }

  if (types == 0)
    {
      return nil;
    }
  return [NSMethodSignature signatureWithObjCTypes: types];
}

/**
 * Returns a string describing the receiver.  The default implementation
 * gives the class and memory location of the receiver.
 */
- (NSString*) description
{
  return [NSString stringWithFormat: @"<%s: %p>",
    class_getName([self class]), self];
}

/**
 * Returns a string describing the receiving class.  The default implementation
 * gives the name of the class by calling NSStringFromClass().
 */
+ (NSString*) description
{
  return NSStringFromClass(self);
}

/**
 * Sets up the ObjC runtime so that the receiver is used wherever code
 * calls for aClassObject to be used.
 */
+ (void) poseAsClass: (Class)aClassObject
{
  [NSException raise: NSInternalInconsistencyException
              format: @"Class posing is not supported"];
}

/**
 * Raises an invalid argument exception providing information about
 * the receivers inability to handle aSelector.
 */
- (void) doesNotRecognizeSelector: (SEL)aSelector
{
  [NSException raise: NSInvalidArgumentException
	      format: @"%s(%s) does not recognize %s",
	       GSClassNameFromObject(self),
	       GSObjCIsInstance(self) ? "instance" : "class",
	       aSelector ? sel_getName(aSelector) : "(null)"];
}

/**
 * This method is called automatically to handle a message sent to
 * the receiver for which the receivers class has no method.<br />
 * The default implementation calls -doesNotRecognizeSelector:
 */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  id target = [self forwardingTargetForSelector: [anInvocation selector]];

  if (nil != target)
    {
      [anInvocation invokeWithTarget: target];
      return;
    }
  [self doesNotRecognizeSelector: [anInvocation selector]];
  return;
}

/**
 * Called after the receiver has been created by decoding some sort
 * of archive.  Returns self.  Subclasses may override this to perform
 * some special initialisation upon being decoded.
 */
- (id) awakeAfterUsingCoder: (NSCoder*)aDecoder
{
  return self;
}

// FIXME - should this be added (as in OS X) now that we have NSKeyedArchiver?
// - (Class) classForKeyedArchiver
// {
//     return [self classForArchiver];
// }

/**
 * Override to substitute class when an instance is being archived by an
 * [NSArchiver].  Default implementation returns -classForCoder.
 */
- (Class) classForArchiver
{
  return [self classForCoder];
}

/**
 * Override to substitute class when an instance is being serialized by an
 * [NSCoder].  Default implementation returns <code>[self class]</code> (no
 * substitution).
 */
- (Class) classForCoder
{
  return [self class];
}

// FIXME - should this be added (as in OS X) now that we have NSKeyedArchiver?
// - (id) replacementObjectForKeyedArchiver: (NSKeyedArchiver *)keyedArchiver
// {
//     return [self replacementObjectForCoder: (NSArchiver *)keyedArchiver];
// }

/**
 * Override to substitute another object for this instance when being archived
 * by given [NSArchiver].  Default implementation returns
 * -replacementObjectForCoder:.
 */
- (id) replacementObjectForArchiver: (NSArchiver*)anArchiver
{
  return [self replacementObjectForCoder: (NSCoder*)anArchiver];
}

/**
 * Override to substitute another object for this instance when being
 * serialized by given [NSCoder].  Default implementation returns
 * <code>self</code>.
 */
- (id) replacementObjectForCoder: (NSCoder*)anEncoder
{
  return self;
}


/* NSObject protocol */

/**
 * Adds the receiver to the current autorelease pool, so that it will be
 * sent a -release message when the pool is destroyed.<br />
 * Returns the receiver.<br />
 * In GNUstep, the [NSObject+enableDoubleReleaseCheck:] method may be used
 * to turn on checking for retain/release errors in this method.
 */
- (id) autorelease
{
  if (double_release_check_enabled)
    {
      NSUInteger release_count;
      NSUInteger retain_count = [self retainCount];
      release_count = [autorelease_class autoreleaseCountForObject:self];
      if (release_count > retain_count)
        [NSException
	  raise: NSGenericException
	  format: @"Autorelease would release object too many times.\n"
	  @"%"PRIuPTR" release(s) versus %"PRIuPTR" retain(s)",
	  release_count, retain_count];
    }

  (*autorelease_imp)(autorelease_class, autorelease_sel, self);
  return self;
}

/**
 * Dummy method returning the receiver.
 */
+ (id) autorelease
{
  return self;
}

/**
 * Returns the receiver.
 */
+ (Class) class
{
  return self;
}

/**
 * Returns the hash of the receiver.  Subclasses should ensure that their
 * implementations of this method obey the rule that if the -isEqual: method
 * returns YES for two instances of the class, the -hash method returns the
 * same value for both instances.<br />
 * The default implementation returns a value based on the address
 * of the instance.
 */
- (NSUInteger) hash
{
  /*
   *  malloc() must return pointers aligned to point to any data type
   */
#define MAXALIGN (__alignof__(_Complex long double))

  static int shift = MAXALIGN==16 ? 4 : (MAXALIGN==8 ? 3 : 2);

  /* We shift left to lose any zero bits produced by the
   * alignment of the object in memory.
   */
  return (NSUInteger)((uintptr_t)self >> shift);
}

/**
 * Tests anObject and the receiver for equality.  The default implementation
 * considers two objects to be equal only if they are the same object
 * (ie occupy the same memory location).<br />
 * If a subclass overrides this method, it should also override the -hash
 * method so that if two objects are equal they both have the same hash.
 */
- (BOOL) isEqual: (id)anObject
{
  return (self == anObject);
}

/**
 * Returns YES if aClass is the NSObject class
 */
+ (BOOL) isKindOfClass: (Class)aClass
{
  if (aClass == [NSObject class])
    return YES;
  return NO;
}

/**
 * Returns YES if the class of the receiver is either the same as aClass
 * or is derived from (a subclass of) aClass.
 */
- (BOOL) isKindOfClass: (Class)aClass
{
  Class class = object_getClass(self);

  return GSObjCIsKindOf(class, aClass);
}

/**
 * Returns YES if aClass is the same as the receiving class.
 */
+ (BOOL) isMemberOfClass: (Class)aClass
{
  return (self == aClass) ? YES : NO;
}

/**
 * Returns YES if the class of the receiver is aClass
 */
- (BOOL) isMemberOfClass: (Class)aClass
{
  return ([self class] == aClass) ? YES : NO;
}

/**
 * Returns a flag to differentiate between 'true' objects, and objects
 * which are proxies for other objects (ie they forward messages to the
 * other objects).<br />
 * The default implementation returns NO.
 */
- (BOOL) isProxy
{
  return NO;
}

/**
 * Returns YES if the receiver is aClass or a subclass of aClass.
 */
+ (BOOL) isSubclassOfClass: (Class)aClass
{
  return GSObjCIsKindOf(self, aClass);
}

/**
 * Causes the receiver to execute the method implementation corresponding
 * to aSelector and returns the result.<br />
 * The method must be one which takes no arguments and returns an object.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
- (id) performSelector: (SEL)aSelector
{
  IMP msg;

  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  /* The Apple runtime API would do:
   * msg = class_getMethodImplementation(object_getClass(self), aSelector);
   * but this cannot ask self for information about any method reached by
   * forwarding, so the returned forwarding function would ge a generic one
   * rather than one aware of hardware issues with returning structures
   * and floating points.  We therefore prefer the GNU API which is able to
   * use forwarding callbacks to get better type information.
   */
  msg = objc_msg_lookup(self, aSelector);
  if (!msg)
    {
      [NSException raise: NSGenericException
		   format: @"invalid selector '%s' passed to %s",
		     sel_getName(aSelector), sel_getName(_cmd)];
      return nil;
    }
  return (*msg)(self, aSelector);
}

/**
 * Causes the receiver to execute the method implementation corresponding
 * to aSelector and returns the result.<br />
 * The method must be one which takes one argument and returns an object.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
- (id) performSelector: (SEL)aSelector withObject: (id)anObject
{
  IMP msg;

  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  /* The Apple runtime API would do:
   * msg = class_getMethodImplementation(object_getClass(self), aSelector);
   * but this cannot ask self for information about any method reached by
   * forwarding, so the returned forwarding function would be a generic one
   * rather than one aware of hardware issues with returning structures
   * and floating points.  We therefore prefer the GNU API which is able to
   * use forwarding callbacks to get better type information.
   */
  msg = objc_msg_lookup(self, aSelector);
  if (!msg)
    {
      [NSException raise: NSGenericException
		   format: @"invalid selector '%s' passed to %s",
                   sel_getName(aSelector), sel_getName(_cmd)];
      return nil;
    }

  return (*msg)(self, aSelector, anObject);
}

/**
 * Causes the receiver to execute the method implementation corresponding
 * to aSelector and returns the result.<br />
 * The method must be one which takes two arguments and returns an object.
 * <br />Raises NSInvalidArgumentException if given a null selector.
 */
- (id) performSelector: (SEL)aSelector
	    withObject: (id) object1
	    withObject: (id) object2
{
  IMP msg;

  if (aSelector == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  /* The Apple runtime API would do:
   * msg = class_getMethodImplementation(object_getClass(self), aSelector);
   * but this cannot ask self for information about any method reached by
   * forwarding, so the returned forwarding function would ge a generic one
   * rather than one aware of hardware issues with returning structures
   * and floating points.  We therefore prefer the GNU API which is able to
   * use forwarding callbacks to get better type information.
   */
  msg = objc_msg_lookup(self, aSelector);
  if (!msg)
    {
      [NSException raise: NSGenericException
		   format: @"invalid selector '%s' passed to %s",
                   sel_getName(aSelector), sel_getName(_cmd)];
      return nil;
    }

  return (*msg)(self, aSelector, object1, object2);
}

/**
 * Decrements the retain count for the receiver if greater than zero,
 * otherwise calls the dealloc method instead.<br />
 * The default implementation calls the NSDecrementExtraRefCountWasZero()
 * function to test the extra reference count for the receiver (and
 * decrement it if non-zero) - if the extra reference count is zero then
 * the retain count is one, and the dealloc method is called.<br />
 * In GNUstep, the [NSObject+enableDoubleReleaseCheck:] method may be used
 * to turn on checking for ratain/release errors in this method.
 */
- (oneway void) release
{
  release_fast(self);
}

/**
 * The class implementation of the release method is a dummy method
 * having no effect.  It is present so that class objects can be stored
 * in containers (such as NSArray) which will send them retain and
 * release messages.
 */
+ (oneway void) release
{
  return;
}

/**
 * Returns a flag to say if the receiver will
 * respond to the specified selector.  This ignores situations
 * where a subclass implements -forwardInvocation: to respond to
 * selectors not normally handled ... in these cases the subclass
 * may override this method to handle it.
 * <br />If given a null selector, raises NSInvalidArgumentException when
 * in MacOS-X compatibility more, or returns NO otherwise.
 */
- (BOOL) respondsToSelector: (SEL)aSelector
{
  Class cls = object_getClass(self);

  if (aSelector == 0)
    {
      if (GSPrivateDefaultsFlag(GSMacOSXCompatible))
	{
	  [NSException raise: NSInvalidArgumentException
		    format: @"%@ null selector given",
	    NSStringFromSelector(_cmd)];
	}
      return NO;
    }

  if (class_respondsToSelector(cls, aSelector))
    {
      return YES;
    }

  if (class_isMetaClass(cls))
    {
      return [(Class)self resolveClassMethod: aSelector];
    }
  else
    {
      return [cls resolveInstanceMethod: aSelector];
    }
}

/**
 * Increments the reference count and returns the receiver.<br />
 * The default implementation does this by calling NSIncrementExtraRefCount()
 */
- (id) retain
{
  return retain_fast(self);
}

/**
 * The class implementation of the retain method is a dummy method
 * having no effect.  It is present so that class objects can be stored
 * in containers (such as NSArray) which will send them retain and
 * release messages.
 */
+ (id) retain
{
  return self;
}

/**
 * Returns the reference count for the receiver.  Each instance has an
 * implicit reference count of 1, and has an 'extra reference count'
 * returned by the NSExtraRefCount() function, so the value returned by
 * this method is always greater than zero.<br />
 * By convention, objects which should (or can) never be deallocated
 * return the maximum unsigned integer value.
 */
- (NSUInteger) retainCount
{
  return getRetainCount(self);
}

/**
 * The class implementation of the retainCount method always returns
 * the maximum unsigned integer value, as classes can not be deallocated
 * the retain count mechanism is a dummy system for them.
 */
+ (NSUInteger) retainCount
{
  return UINT_MAX;
}

/**
 * Returns the receiver.
 */
- (id) self
{
  return self;
}

/**
 * Returns the memory allocation zone in which the receiver is located.
 */
- (NSZone*) zone
{
  return NSZoneFromPointer(self);
}

+ (NSZone *) zone
{
  return NSDefaultMallocZone();
}

/**
 * Called to encode the instance variables of the receiver to aCoder.<br />
 * Subclasses should call the superclass method at the start of their
 * own implementation.
 */
- (void) encodeWithCoder: (NSCoder*)aCoder
{
  return;
}

/**
 * Called to intialise instance variables of the receiver from aDecoder.<br />
 * Subclasses should call the superclass method at the start of their
 * own implementation.
 */
- (id) initWithCoder: (NSCoder*)aDecoder
{
  return self;
}

+ (BOOL) resolveClassMethod: (SEL)name
{
  return NO;
}

+ (BOOL) resolveInstanceMethod: (SEL)name
{
  return NO;
}

/**
 * Sets the version number of the receiving class.  Should be nonnegative.
 */
+ (id) setVersion: (NSInteger)aVersion
{
  if (aVersion < 0)
    [NSException raise: NSInvalidArgumentException
	        format: @"%s +setVersion: may not set a negative version",
			GSClassNameFromObject(self)];
  class_setVersion(self, aVersion);
  return self;
}

/**
 *  Returns the version number of the receiving class.  This will default to
 *  a number assigned by the Objective C compiler if [NSObject -setVersion] has
 *  not been called.
 */
+ (NSInteger) version
{
  return class_getVersion(self);
}

- (id) autoContentAccessingProxy
{
  return AUTORELEASE([[GSContentAccessingProxy alloc] initWithObject: self]);
}

- (id) forwardingTargetForSelector:(SEL)aSelector
{
  return nil;
}
@end


/**
 *  Methods for compatibility with the NEXTSTEP (pre-OpenStep) 'Object' class.
 */
@implementation NSObject (NEXTSTEP)

/* NEXTSTEP Object class compatibility */

/**
 * Logs a message.  <em>Deprecated.</em>  Use NSLog() in new code.
 */
- (id) error: (const char *)aString, ...
{
#define FMT "error: %s (%s)\n%s\n"
  char fmt[(strlen((char*)FMT)+strlen((char*)GSClassNameFromObject(self))
            +((aString!=NULL)?strlen((char*)aString):0)+8)];
  va_list ap;

  snprintf(fmt, sizeof(fmt), FMT, GSClassNameFromObject(self),
    GSObjCIsInstance(self) ? "instance" : "class",
    (aString != NULL) ? aString : "");
  va_start(ap, aString);
  vfprintf (stderr, fmt, ap);
  abort ();
  va_end(ap);
#undef FMT
  return nil;
}

/*
- (const char *) name
{
  return GSClassNameFromObject(self);
}
*/

- (BOOL) isKindOf: (Class)aClassObject
{
  return [self isKindOfClass: aClassObject];
}

- (BOOL) isMemberOf: (Class)aClassObject
{
  return [self isMemberOfClass: aClassObject];
}

+ (BOOL) instancesRespondTo: (SEL)aSel
{
  return [self instancesRespondToSelector: aSel];
}

- (BOOL) respondsTo: (SEL)aSel
{
  return [self respondsToSelector: aSel];
}

+ (BOOL) conformsTo: (Protocol*)aProtocol
{
  return [self conformsToProtocol: aProtocol];
}

- (BOOL) conformsTo: (Protocol*)aProtocol
{
  return [self conformsToProtocol: aProtocol];
}

+ (IMP) instanceMethodFor: (SEL)aSel
{
  return [self instanceMethodForSelector:aSel];
}

- (IMP) methodFor: (SEL)aSel
{
  return [self methodForSelector: aSel];
}

+ (id) poseAs: (Class)aClassObject
{
  [self poseAsClass: aClassObject];
  return self;
}

- (id) doesNotRecognize: (SEL)aSel
{
  [NSException raise: NSGenericException
	       format: @"%s(%s) does not recognize %s",
	       GSClassNameFromObject(self),
	       GSObjCIsInstance(self) ? "instance" : "class",
	       aSel ? sel_getName(aSel) : "(null)"];
  return nil;
}

- (id) perform: (SEL)sel with: (id)anObject
{
  return [self performSelector: sel withObject: anObject];
}

- (id) perform: (SEL)sel with: (id)anObject with: (id)anotherObject
{
  return [self performSelector: sel withObject: anObject
                    withObject: anotherObject];
}

@end



/**
 * Some non-standard extensions mainly needed for backwards compatibility
 * and internal utility reasons.
 */
@implementation NSObject (GNUstep)

/**
 * Enables runtime checking of retain/release/autorelease operations.<br />
 * <p>Whenever either -autorelease or -release is called, the contents of any
 * autorelease pools will be checked to see if there are more outstanding
 * release operations than the objects retain count.  In which case an
 * exception is raised to say that the object is released too many times.
 * </p>
 * <p><strong>Beware</strong>, since this feature entails examining all active
 * autorelease pools every time an object is released or autoreleased, it
 * can cause a massive performance degradation ... it should only be enabled
 * for debugging.
 * </p>
 * <p>
 * When you are having memory allocation problems, it may make more sense
 * to look at the memory allocation debugging functions documented in
 * NSDebug.h, or use the NSZombie features.
 * </p>
 */
+ (void) enableDoubleReleaseCheck: (BOOL)enable
{
  double_release_check_enabled = enable;
}

/**
 * The default (NSObject) implementation of this method simply calls
 * the -description method and discards the locale
 * information.
 */
- (NSString*) descriptionWithLocale: (id)aLocale
{
  return [self description];
}

+ (NSString*) descriptionWithLocale: (id)aLocale
{
  return [self description];
}

/**
 * The default (NSObject) implementation of this method simply calls
 * the -descriptionWithLocale: method and discards the
 * level information.
 */
- (NSString*) descriptionWithLocale: (id)aLocale
			     indent: (NSUInteger)level
{
  return [self descriptionWithLocale: aLocale];
}

+ (NSString*) descriptionWithLocale: (id)aLocale
			     indent: (NSUInteger)level
{
  return [self descriptionWithLocale: aLocale];
}

- (BOOL) _dealloc
{
  return YES;
}

- (BOOL) isMetaClass
{
  return NO;
}

- (BOOL) isClass
{
  return class_isMetaClass(object_getClass(self));
}

- (BOOL) isMemberOfClassNamed: (const char*)aClassName
{
  return ((aClassName!=NULL)
          &&!strcmp(class_getName(object_getClass(self)), aClassName));
}

+ (struct objc_method_description *) descriptionForInstanceMethod: (SEL)aSel
{
  if (aSel == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  return ((struct objc_method_description *)
           GSGetMethod(self, aSel, YES, YES));
}

- (struct objc_method_description *) descriptionForMethod: (SEL)aSel
{
  if (aSel == 0)
    [NSException raise: NSInvalidArgumentException
		format: @"%@ null selector given", NSStringFromSelector(_cmd)];

  return ((struct objc_method_description *)
	  GSGetMethod((GSObjCIsInstance(self)
		       ? object_getClass(self) : (Class)self),
		      aSel,
		      GSObjCIsInstance(self),
		      YES));
}

+ (NSInteger) streamVersion: (void*)aStream
{
  GSOnceMLog(@"[NSObject+streamVersion:] is deprecated ... do not use");
  return class_getVersion (self);
}
- (id) read: (void*)aStream
{
  GSOnceMLog(@"[NSObject-read:] is deprecated ... do not use");
  return self;
}
- (id) write: (void*)aStream
{
  GSOnceMLog(@"[NSObject-write:] is deprecated ... do not use");
  return self;
}
- (id) awake
{
  GSOnceMLog(@"[NSObject-awake] is deprecated ... do not use");
  return self;
}

@end



@implementation	NSZombie
- (Class) class
{
  return object_getClass(self);
}
- (Class) originalClass
{
  Class c = Nil;

  if (0 != zombieMap)
    {
      pthread_mutex_lock(&allocationLock);
      if (0 != zombieMap)
        {
          c = NSMapGet(zombieMap, (void*)self);
        }
      pthread_mutex_unlock(&allocationLock);
    }
  return c;
}
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  NSUInteger	size = [[anInvocation methodSignature] methodReturnLength];
  unsigned char	v[size];

  memset(v, '\0', size);
  GSLogZombie(self, [anInvocation selector]);
  [anInvocation setReturnValue: (void*)v];
  return;
}
- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  Class	c;

  if (0 == aSelector)
    {
      return nil;
    }
  pthread_mutex_lock(&allocationLock);
  c = zombieMap ? NSMapGet(zombieMap, (void*)self) : Nil;
  pthread_mutex_unlock(&allocationLock);

  return [c instanceMethodSignatureForSelector: aSelector];
}
@end

@implementation GSContentAccessingProxy
- (void) dealloc
{
  [object endContentAccess];
  [super dealloc];
}

- (void) finalize
{
  [object endContentAccess];
}

- (id) forwardingTargetForSelector: (SEL)aSelector
{
  return object;
}
/* Support for legacy runtimes... */
- (void) forwardInvocation: (NSInvocation*)anInvocation
{
  [anInvocation invokeWithTarget: object];
}

- (id) initWithObject: (id)anObject
{
  ASSIGN(object, anObject);
  [object beginContentAccess];
  return self;
}

- (NSMethodSignature*) methodSignatureForSelector: (SEL)aSelector
{
  return [object methodSignatureForSelector: aSelector];
}
@end

NSUInteger
GSPrivateMemorySize(NSObject *self, NSHashTable *exclude)
{
  if (0 == NSHashGet(exclude, self))
    {
      NSHashInsert(exclude, self);
      return class_getInstanceSize(object_getClass(self));
    }
  return 0;
}

@implementation	NSObject (MemoryFootprint)

+ (NSUInteger) contentSizeOf: (NSObject*)obj
		   excluding: (NSHashTable*)exclude
{
  Class		cls = object_getClass(obj);
  NSUInteger	size = 0;

  while (cls != Nil)
    {
      unsigned	count;
      Ivar	*vars;

      if (0 != (vars = class_copyIvarList(cls, &count)))
	{
	  while (count-- > 0)
	    {
	      const char	*type = ivar_getTypeEncoding(vars[count]);

	      type = GSSkipTypeQualifierAndLayoutInfo(type);
	      if ('@' == *type)
		{
		  NSObject	*content = object_getIvar(obj, vars[count]);
	    
		  if (content != nil)
		    {
		      size += [content sizeInBytesExcluding: exclude];
		    }
		}
	    }
	  free(vars);
	}
      cls = class_getSuperclass(cls);
    }
  return size;
}
+ (NSUInteger) sizeInBytes
{
  return 0;
}
+ (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  return 0;
}
+ (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return 0;
}
- (NSUInteger) sizeInBytes
{
  NSUInteger	bytes;
  NSHashTable	*exclude;
 
  exclude = NSCreateHashTable(NSNonOwnedPointerHashCallBacks, 0);
  bytes = [self sizeInBytesExcluding: exclude];
  NSFreeHashTable(exclude);
  return bytes;
}
- (NSUInteger) sizeInBytesExcluding: (NSHashTable*)exclude
{
  if (0 == NSHashGet(exclude, self))
    {
      NSUInteger        size = [self sizeOfInstance];

      NSHashInsert(exclude, self);
      if (size > 0)
        {
	  size += [self sizeOfContentExcluding: exclude];
        }
      return size;
    }
  return 0;
}
- (NSUInteger) sizeOfContentExcluding: (NSHashTable*)exclude
{
  return 0;
}
- (NSUInteger) sizeOfInstance
{
  NSUInteger    size;

#if	GS_SIZEOF_VOIDP > 4
  NSUInteger    u = (NSUInteger)self;
  if (u & 0x07)
    {
      return 0;	// Small object has no size
    }
#endif

#if 	HAVE_MALLOC_USABLE_SIZE
  size = malloc_usable_size((void*)self - sizeof(intptr_t));
#else
  size = class_getInstanceSize(object_getClass(self));
#endif

  return size;
}

@end
