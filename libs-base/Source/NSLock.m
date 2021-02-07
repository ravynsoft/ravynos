/** Control of executable units within a shared virtual memory space
   Copyright (C) 1996-2010 Free Software Foundation, Inc.

   Original Author:  David Chisnall <csdavec@swan.ac.uk>

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

   <title>NSLock class reference</title>
   <ignore> All autogsdoc markup is in the header
*/

#import "common.h"

#include <pthread.h>
#import "GSPrivate.h"
#define	gs_cond_t	pthread_cond_t
#define	gs_mutex_t	pthread_mutex_t
#include <math.h>

#define	EXPOSE_NSLock_IVARS	1
#define	EXPOSE_NSRecursiveLock_IVARS	1
#define	EXPOSE_NSCondition_IVARS	1
#define	EXPOSE_NSConditionLock_IVARS	1

#import "common.h"

#import "Foundation/NSLock.h"
#import "Foundation/NSException.h"
#import "Foundation/NSThread.h"

#import "GSPThread.h"

#define class_createInstance(C,E) NSAllocateObject(C,E,NSDefaultMallocZone())

static Class    baseConditionClass = Nil;
static Class    baseConditionLockClass = Nil;
static Class    baseLockClass = Nil;
static Class    baseRecursiveLockClass = Nil;

static Class    tracedConditionClass = Nil;
static Class    tracedConditionLockClass = Nil;
static Class    tracedLockClass = Nil;
static Class    tracedRecursiveLockClass = Nil;

static Class    untracedConditionClass = Nil;
static Class    untracedConditionLockClass = Nil;
static Class    untracedLockClass = Nil;
static Class    untracedRecursiveLockClass = Nil;

static BOOL     traceLocks = NO;

@implementation NSObject (GSTraceLocks)

+ (BOOL) shouldCreateTraceableLocks: (BOOL)shouldTrace
{
  BOOL  old = traceLocks;

  traceLocks = shouldTrace ? YES : NO;
  return old;
}

+ (NSCondition*) tracedCondition
{
  return AUTORELEASE([GSTracedCondition new]);
}

+ (NSConditionLock*) tracedConditionLockWithCondition: (NSInteger)value
{
  return AUTORELEASE([[GSTracedConditionLock alloc] initWithCondition: value]);
}

+ (NSLock*) tracedLock
{
  return AUTORELEASE([GSTracedLock new]);
}

+ (NSRecursiveLock*) tracedRecursiveLock
{
  return AUTORELEASE([GSTracedRecursiveLock new]);
}

@end

/* In untraced operations these macros do nothing.
 * When tracing they are defined to perform the trace methods of the thread.
 */
#define CHKT(T,X) 
#define CHK(X)

/*
 * Methods shared between NSLock, NSRecursiveLock, and NSCondition
 *
 * Note: These methods currently throw exceptions when locks are incorrectly
 * acquired.  This is compatible with earlier GNUstep behaviour.  In OS X 10.5
 * and later, these will just NSLog a warning instead.  Throwing an exception
 * is probably better behaviour, because it encourages developer to fix their
 * code.
 */

#define	MDEALLOC \
- (void) dealloc\
{\
  [self finalize];\
  [_name release];\
  [super dealloc];\
}

#if     defined(HAVE_PTHREAD_MUTEX_OWNER)

#define	MDESCRIPTION \
- (NSString*) description\
{\
  if (_mutex.__data.__owner)\
    {\
      if (_name == nil)\
        {\
          return [NSString stringWithFormat: @"%@ (locked by %llu)",\
            [super description], (unsigned long long)_mutex.__data.__owner];\
        }\
      return [NSString stringWithFormat: @"%@ '%@' (locked by %llu)",\
        [super description], _name, (unsigned long long)_mutex.__data.__owner];\
    }\
  else\
    {\
      if (_name == nil)\
        {\
          return [super description];\
        }\
      return [NSString stringWithFormat: @"%@ '%@'",\
        [super description], _name];\
    }\
}

#define	MISLOCKED \
- (BOOL) isLockedByCurrentThread\
{\
  if (GSPrivateThreadID() == (NSUInteger)_mutex.__data.__owner)\
    return YES;\
  else\
    return NO; \
}

#else

#define	MDESCRIPTION \
- (NSString*) description\
{\
  if (_name == nil)\
    {\
      return [super description];\
    }\
  return [NSString stringWithFormat: @"%@ '%@'",\
    [super description], _name];\
}

#define	MISLOCKED \
- (BOOL) isLockedByCurrentThread\
{\
  [NSException raise: NSGenericException format: @"Not supported"];\
  return NO;\
}

#endif

#define MFINALIZE \
- (void) finalize\
{\
  pthread_mutex_destroy(&_mutex);\
}

#define	MLOCK \
- (void) lock\
{\
  int err = pthread_mutex_lock(&_mutex);\
  if (EDEADLK == err)\
    {\
      (*_NSLock_error_handler)(self, _cmd, YES, @"deadlock");\
    }\
  else if (err != 0)\
    {\
      [NSException raise: NSLockException format: @"failed to lock mutex"];\
    }\
}

#define	MLOCKBEFOREDATE \
- (BOOL) lockBeforeDate: (NSDate*)limit\
{\
  do\
    {\
      int err = pthread_mutex_trylock(&_mutex);\
      if (0 == err)\
	{\
          CHK(Hold) \
	  return YES;\
	}\
      sched_yield();\
    } while ([limit timeIntervalSinceNow] > 0);\
  return NO;\
}

#define MNAME \
- (void) setName: (NSString*)newName\
{\
  ASSIGNCOPY(_name, newName);\
}\
- (NSString*) name\
{\
  return _name;\
}

#define MSTACK \
- (GSStackTrace*) stack \
{ \
  return nil; \
}

#define	MTRYLOCK \
- (BOOL) tryLock\
{\
  int err = pthread_mutex_trylock(&_mutex);\
  if (0 == err) \
    { \
      CHK(Hold) \
      return YES; \
    } \
  else \
    { \
      return NO;\
    } \
}

#define	MUNLOCK \
- (void) unlock\
{\
  if (0 != pthread_mutex_unlock(&_mutex))\
    {\
      [NSException raise: NSLockException\
	    format: @"failed to unlock mutex"];\
    }\
  CHK(Drop) \
}

static pthread_mutex_t deadlock;
static pthread_mutexattr_t attr_normal;
static pthread_mutexattr_t attr_reporting;
static pthread_mutexattr_t attr_recursive;

/*
 * OS X 10.5 compatibility function to allow debugging deadlock conditions.
 */
void _NSLockError(id obj, SEL _cmd, BOOL stop, NSString *msg)
{
  NSLog(@"*** -[%@ %@]: %@ (%@)", [obj class], NSStringFromSelector(_cmd),
    msg, obj);
  NSLog(@"*** Break on _NSLockError() to debug.");
  if (YES == stop)
    pthread_mutex_lock(&deadlock);
}

NSLock_error_handler  *_NSLock_error_handler = _NSLockError;

// Exceptions

NSString *NSLockException = @"NSLockException";

@implementation NSLock

+ (id) allocWithZone: (NSZone*)z
{
  if (self == baseLockClass && YES == traceLocks)
    {
      return class_createInstance(tracedLockClass, 0);
    }
  return class_createInstance(self, 0);
}

+ (void) initialize
{
  static BOOL	beenHere = NO;

  if (beenHere == NO)
    {
      beenHere = YES;

      /* Initialise attributes for the different types of mutex.
       * We do it once, since attributes can be shared between multiple
       * mutexes.
       * If we had a pthread_mutexattr_t instance for each mutex, we would
       * either have to store it as an ivar of our NSLock (or similar), or
       * we would potentially leak instances as we couldn't destroy them
       * when destroying the NSLock.  I don't know if any implementation
       * of pthreads actually allocates memory when you call the
       * pthread_mutexattr_init function, but they are allowed to do so
       * (and deallocate the memory in pthread_mutexattr_destroy).
       */
      pthread_mutexattr_init(&attr_normal);
      pthread_mutexattr_settype(&attr_normal, PTHREAD_MUTEX_NORMAL);
      pthread_mutexattr_init(&attr_reporting);
      pthread_mutexattr_settype(&attr_reporting, PTHREAD_MUTEX_ERRORCHECK);
      pthread_mutexattr_init(&attr_recursive);
      pthread_mutexattr_settype(&attr_recursive, PTHREAD_MUTEX_RECURSIVE);

      /* To emulate OSX behavior, we need to be able both to detect deadlocks
       * (so we can log them), and also hang the thread when one occurs.
       * the simple way to do that is to set up a locked mutex we can
       * force a deadlock on.
       */
      pthread_mutex_init(&deadlock, &attr_normal);
      pthread_mutex_lock(&deadlock);

      baseConditionClass = [NSCondition class];
      baseConditionLockClass = [NSConditionLock class];
      baseLockClass = [NSLock class];
      baseRecursiveLockClass = [NSRecursiveLock class];

      tracedConditionClass = [GSTracedCondition class];
      tracedConditionLockClass = [GSTracedConditionLock class];
      tracedLockClass = [GSTracedLock class];
      tracedRecursiveLockClass = [GSTracedRecursiveLock class];

      untracedConditionClass = [GSUntracedCondition class];
      untracedConditionLockClass = [GSUntracedConditionLock class];
      untracedLockClass = [GSUntracedLock class];
      untracedRecursiveLockClass = [GSUntracedRecursiveLock class];
    }
}

MDEALLOC
MDESCRIPTION
MFINALIZE

/* Use an error-checking lock.  This is marginally slower, but lets us throw
 * exceptions when incorrect locking occurs.
 */
- (id) init
{
  if (nil != (self = [super init]))
    {
      if (0 != pthread_mutex_init(&_mutex, &attr_reporting))
	{
	  DESTROY(self);
	}
    }
  return self;
}

MISLOCKED
MLOCK

- (BOOL) lockBeforeDate: (NSDate*)limit
{
  do
    {
      int err = pthread_mutex_trylock(&_mutex);
      if (0 == err)
	{
          CHK(Hold)
	  return YES;
	}
      if (EDEADLK == err)
	{
	  (*_NSLock_error_handler)(self, _cmd, NO, @"deadlock");
	}
      sched_yield();
    } while ([limit timeIntervalSinceNow] > 0);
  return NO;
}

MNAME
MSTACK
MTRYLOCK
MUNLOCK

@end

@implementation NSRecursiveLock

+ (id) allocWithZone: (NSZone*)z
{
  if (self == baseRecursiveLockClass && YES == traceLocks)
    {
      return class_createInstance(tracedRecursiveLockClass, 0);
    }
  return class_createInstance(self, 0);
}

+ (void) initialize
{
  [NSLock class];	// Ensure mutex attributes are set up.
}

MDEALLOC
MDESCRIPTION
MFINALIZE

- (id) init
{
  if (nil != (self = [super init]))
    {
      if (0 != pthread_mutex_init(&_mutex, &attr_recursive))
	{
	  DESTROY(self);
	}
    }
  return self;
}

MISLOCKED
MLOCK
MLOCKBEFOREDATE
MNAME
MSTACK
MTRYLOCK
MUNLOCK
@end

@implementation NSCondition

+ (id) allocWithZone: (NSZone*)z
{
  if (self == baseConditionClass && YES == traceLocks)
    {
      return class_createInstance(tracedConditionClass, 0);
    }
  return class_createInstance(self, 0);
}

+ (void) initialize
{
  [NSLock class];	// Ensure mutex attributes are set up.
}

- (void) broadcast
{
  pthread_cond_broadcast(&_condition);
}

MDEALLOC
MDESCRIPTION

- (void) finalize
{
  pthread_cond_destroy(&_condition);
  pthread_mutex_destroy(&_mutex);
}

- (id) init
{
  if (nil != (self = [super init]))
    {
      if (0 != pthread_cond_init(&_condition, NULL))
	{
	  DESTROY(self);
	}
      else if (0 != pthread_mutex_init(&_mutex, &attr_reporting))
	{
	  pthread_cond_destroy(&_condition);
	  DESTROY(self);
	}
    }
  return self;
}

MISLOCKED
MLOCK
MLOCKBEFOREDATE
MNAME

- (void) signal
{
  pthread_cond_signal(&_condition);
}

MSTACK
MTRYLOCK
MUNLOCK

- (void) wait
{
  pthread_cond_wait(&_condition, &_mutex);
}

- (BOOL) waitUntilDate: (NSDate*)limit
{
  NSTimeInterval ti = [limit timeIntervalSince1970];
  double secs, subsecs;
  struct timespec timeout;
  int retVal = 0;

  // Split the float into seconds and fractions of a second
  subsecs = modf(ti, &secs);
  timeout.tv_sec = secs;
  // Convert fractions of a second to nanoseconds
  timeout.tv_nsec = subsecs * 1e9;

  /* NB. On timeout the lock is still held even through condition is not met
   */

  retVal = pthread_cond_timedwait(&_condition, &_mutex, &timeout);
  if (retVal == 0)
    {
      return YES;
    }
  if (retVal == ETIMEDOUT)
    {
      return NO;
    }
  if (retVal == EINVAL)
    {
      NSLog(@"Invalid arguments to pthread_cond_timedwait");
    }
  return NO;
}

@end

@implementation NSConditionLock

+ (id) allocWithZone: (NSZone*)z
{
  if (self == baseConditionLockClass && YES == traceLocks)
    {
      return class_createInstance(tracedConditionLockClass, 0);
    }
  return class_createInstance(self, 0);
}

+ (void) initialize
{
  [NSLock class];	// Ensure mutex attributes are set up.
}

- (NSInteger) condition
{
  return _condition_value;
}

- (void) dealloc
{
  [_name release];
  [_condition release];
  [super dealloc];
}

- (id) init
{
  return [self initWithCondition: 0];
}

- (id) initWithCondition: (NSInteger)value
{
  if (nil != (self = [super init]))
    {
      if (nil == (_condition = [NSCondition new]))
	{
	  DESTROY(self);
	}
      else
	{
          _condition_value = value;
          [_condition setName:
            [NSString stringWithFormat: @"condition-for-lock-%p", self]];
	}
    }
  return self;
}

- (BOOL) isLockedByCurrentThread
{
  return [_condition isLockedByCurrentThread];
}

- (void) lock
{
  [_condition lock];
}

- (BOOL) lockBeforeDate: (NSDate*)limit
{
  return [_condition lockBeforeDate: limit];
}

- (void) lockWhenCondition: (NSInteger)value
{
  [_condition lock];
  while (value != _condition_value)
    {
      [_condition wait];
    }
}

- (BOOL) lockWhenCondition: (NSInteger)condition_to_meet
                beforeDate: (NSDate*)limitDate
{
  if (NO == [_condition lockBeforeDate: limitDate])
    {
      return NO;        // Not locked
    }
  if (condition_to_meet == _condition_value)
    {
      return YES;       // Keeping the lock
    }
  while ([_condition waitUntilDate: limitDate])
    {
      if (condition_to_meet == _condition_value)
	{
	  return YES;   // Keeping the lock
	}
    }
  [_condition unlock];
  return NO;            // Not locked
}

MNAME
MSTACK

- (BOOL) tryLock
{
  return [_condition tryLock];
}

- (BOOL) tryLockWhenCondition: (NSInteger)condition_to_meet
{
  if ([_condition tryLock])
    {
      if (condition_to_meet == _condition_value)
	{
	  return YES; // KEEP THE LOCK
	}
      else
	{
	  [_condition unlock];
	}
    }
  return NO;
}

- (void) unlock
{
  [_condition unlock];
}

- (void) unlockWithCondition: (NSInteger)value
{
  _condition_value = value;
  [_condition broadcast];
  [_condition unlock];
}

@end



/* Versions of the lock classes where the locking is unconditionally traced
 */

#undef CHKT
#define CHKT(T,X) \
{ \
  NSString *msg = [T mutex ## X: self]; \
  if (nil != msg) \
    { \
      (*_NSLock_error_handler)(self, _cmd, YES, msg); \
    } \
}
#undef CHK
#define CHK(X) CHKT(GSCurrentThread(), X)

#undef  MDEALLOC
#define	MDEALLOC \
- (void) dealloc \
{ \
  DESTROY(stack); \
  [super dealloc]; \
}

#undef MLOCK
#define	MLOCK \
- (void) lock\
{ \
  NSThread      *t = GSCurrentThread(); \
  int		err; \
  CHKT(t,Wait) \
  err = pthread_mutex_lock(&_mutex);\
  if (EDEADLK == err)\
    {\
      CHKT(t,Drop) \
      (*_NSLock_error_handler)(self, _cmd, YES, @"deadlock");\
    }\
  else if (err != 0)\
    {\
      CHKT(t,Drop) \
      [NSException raise: NSLockException format: @"failed to lock mutex"];\
    }\
  CHKT(t,Hold) \
}

#undef MSTACK
#define MSTACK \
- (GSStackTrace*) stack \
{ \
  if (nil == stack) \
    { \
      stack = [GSStackTrace new]; \
    } \
  return stack; \
}

@implementation GSTracedCondition
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(tracedConditionClass, 0);
}
MDEALLOC
MLOCK
MLOCKBEFOREDATE
MSTACK
MTRYLOCK

- (void) wait
{
  NSThread      *t = GSCurrentThread();
  CHKT(t,Drop)
  CHKT(t,Wait)
  pthread_cond_wait(&_condition, &_mutex);
  CHKT(t,Hold)
}

- (BOOL) waitUntilDate: (NSDate*)limit
{
  NSTimeInterval ti = [limit timeIntervalSince1970];
  NSThread      *t = GSCurrentThread();
  double secs, subsecs;
  struct timespec timeout;
  int retVal = 0;

  // Split the float into seconds and fractions of a second
  subsecs = modf(ti, &secs);
  timeout.tv_sec = secs;
  // Convert fractions of a second to nanoseconds
  timeout.tv_nsec = subsecs * 1e9;

  /* NB. On timeout the lock is still held even through condition is not met
   */

  CHKT(t,Drop)
  retVal = pthread_cond_timedwait(&_condition, &_mutex, &timeout);
  if (retVal == 0)
    {
      CHKT(t,Hold)
      return YES;
    }
  if (retVal == ETIMEDOUT)
    {
      CHKT(t,Hold)
      return NO;
    }

  if (retVal == EINVAL)
    {
      NSLog(@"Invalid arguments to pthread_cond_timedwait");
    }
  return NO;
}

MUNLOCK
@end
@implementation GSTracedConditionLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(tracedConditionLockClass, 0);
}
- (id) initWithCondition: (NSInteger)value
{
  if (nil != (self = [super init]))
    {
      if (nil == (_condition = [GSTracedCondition new]))
	{
	  DESTROY(self);
	}
      else
	{
          _condition_value = value;
          [_condition setName:
            [NSString stringWithFormat: @"condition-for-lock-%p", self]];
	}
    }
  return self;
}
@end
@implementation GSTracedLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(tracedLockClass, 0);
}
MDEALLOC
MLOCK
MLOCKBEFOREDATE
MSTACK
MTRYLOCK
MUNLOCK
@end
@implementation GSTracedRecursiveLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(tracedRecursiveLockClass, 0);
}
MDEALLOC
MLOCK
MLOCKBEFOREDATE
MSTACK
MTRYLOCK
MUNLOCK
@end


/* Versions of the lock classes where the locking is never traced
 */
@implementation GSUntracedCondition
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(baseConditionClass, 0);
}
@end
@implementation GSUntracedConditionLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(baseConditionLockClass, 0);
}
@end
@implementation GSUntracedLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(baseRecursiveLockClass, 0);
}
@end
@implementation GSUntracedRecursiveLock
+ (id) allocWithZone: (NSZone*)z
{
  return class_createInstance(baseRecursiveLockClass, 0);
}
@end

