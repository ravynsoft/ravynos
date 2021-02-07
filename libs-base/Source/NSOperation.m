/**Implementation for NSOperation for GNUStep
   Copyright (C) 2009,2010 Free Software Foundation, Inc.

   Written by:  Gregory Casamento <greg.casamento@gmail.com>
   Written by:  Richard Frith-Macdonald <rfm@gnu.org>
   Date: 2009,2010

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

   <title>NSOperation class reference</title>
   $Date: 2008-06-08 11:38:33 +0100 (Sun, 08 Jun 2008) $ $Revision: 26606 $
   */

#import "common.h"

#import "Foundation/NSLock.h"

#define	GS_NSOperation_IVARS \
  NSRecursiveLock *lock; \
  NSConditionLock *cond; \
  NSOperationQueuePriority priority; \
  double threadPriority; \
  BOOL cancelled; \
  BOOL concurrent; \
  BOOL executing; \
  BOOL finished; \
  BOOL blocked; \
  BOOL ready; \
  NSMutableArray *dependencies; \
  id completionBlock;

#define	GS_NSOperationQueue_IVARS \
  NSRecursiveLock	*lock; \
  NSConditionLock	*cond; \
  NSMutableArray	*operations; \
  NSMutableArray	*waiting; \
  NSMutableArray	*starting; \
  NSString		*name; \
  BOOL			suspended; \
  NSInteger		executing; \
  NSInteger		threadCount; \
  NSInteger		count;

#import "Foundation/NSOperation.h"
#import "Foundation/NSArray.h"
#import "Foundation/NSAutoreleasePool.h"
#import "Foundation/NSDictionary.h"
#import "Foundation/NSEnumerator.h"
#import "Foundation/NSException.h"
#import "Foundation/NSKeyValueObserving.h"
#import "Foundation/NSThread.h"
#import "GNUstepBase/NSArray+GNUstepBase.h"
#import "GSPrivate.h"

#define	GSInternal	NSOperationInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSOperation)

static void     *isFinishedCtxt = (void*)"isFinished";
static void     *isReadyCtxt = (void*)"isReady";
static void     *queuePriorityCtxt = (void*)"queuePriority";

/* The pool of threads for 'non-concurrent' operations in a queue.
 */
#define	POOL	8

static NSArray	*empty = nil;

@interface	NSOperation (Private)
- (void) _finish;
@end

@implementation NSOperation

+ (BOOL) automaticallyNotifiesObserversForKey: (NSString*)theKey
{
  /* Handle all KVO manually
   */
  return NO;
}

+ (void) initialize
{
  empty = [NSArray new];
  RELEASE([NSObject leakAt: &empty]);
}

- (void) addDependency: (NSOperation *)op
{
  if (NO == [op isKindOfClass: [NSOperation class]])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@] dependency is not an NSOperation",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  if (op == self)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@] attempt to add dependency on self",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  [internal->lock lock];
  if (internal->dependencies == nil)
    {
      internal->dependencies = [[NSMutableArray alloc] initWithCapacity: 5];
    }
  NS_DURING
    {
      if (NSNotFound == [internal->dependencies indexOfObjectIdenticalTo: op])
	{
	  [self willChangeValueForKey: @"dependencies"];
          [internal->dependencies addObject: op];
	  /* We only need to watch for changes if it's possible for them to
	   * happen and make a difference.
	   */
	  if (NO == [op isFinished]
	    && NO == [self isCancelled]
	    && NO == [self isExecuting]
	    && NO == [self isFinished])
	    {
	      /* Can change readiness if we are neither cancelled nor
	       * executing nor finished.  So we need to observe for the
	       * finish of the dependency.
	       */
	      [op addObserver: self
		   forKeyPath: @"isFinished"
		      options: NSKeyValueObservingOptionNew
		      context: isFinishedCtxt];
	      if (internal->ready == YES)
		{
		  /* The new dependency stops us being ready ...
		   * change state.
		   */
		  [self willChangeValueForKey: @"isReady"];
		  internal->ready = NO;
		  [self didChangeValueForKey: @"isReady"];
		}
	    }
	  [self didChangeValueForKey: @"dependencies"];
	}
    }
  NS_HANDLER
    {
      [internal->lock unlock];
      NSLog(@"Problem adding dependency: %@", localException);
      return;
    }
  NS_ENDHANDLER
  [internal->lock unlock];
}

- (void) cancel
{
  if (NO == internal->cancelled && NO == [self isFinished])
    {
      [internal->lock lock];
      if (NO == internal->cancelled && NO == [self isFinished])
	{
	  NS_DURING
	    {
	      [self willChangeValueForKey: @"isCancelled"];
	      internal->cancelled = YES;
	      if (NO == internal->ready)
		{
	          [self willChangeValueForKey: @"isReady"];
		  internal->ready = YES;
	          [self didChangeValueForKey: @"isReady"];
		}
	      [self didChangeValueForKey: @"isCancelled"];
	    }
	  NS_HANDLER
	    {
	      [internal->lock unlock];
	      NSLog(@"Problem cancelling operation: %@", localException);
	      return;
	    }
	  NS_ENDHANDLER
	}
      [internal->lock unlock];
    }
}

- (GSOperationCompletionBlock) completionBlock
{
  return (GSOperationCompletionBlock)internal->completionBlock;
}

- (void) dealloc
{
  if (internal != nil)
    {
      NSOperation	*op;

      if (!internal->finished)
        {
          [self removeObserver: self forKeyPath: @"isFinished"];
        }
      while ((op = [internal->dependencies lastObject]) != nil)
	{
	  [self removeDependency: op];
	}
      RELEASE(internal->dependencies);
      RELEASE(internal->cond);
      RELEASE(internal->lock);
      RELEASE(internal->completionBlock);
      GS_DESTROY_INTERNAL(NSOperation);
    }
  [super dealloc];
}

- (NSArray *) dependencies
{
  NSArray	*a;

  if (internal->dependencies == nil)
    {
      a = empty;	// OSX return an empty array
    }
  else
    {
      [internal->lock lock];
      a = [NSArray arrayWithArray: internal->dependencies];
      [internal->lock unlock];
    }
  return a;
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      GS_CREATE_INTERNAL(NSOperation);
      internal->priority = NSOperationQueuePriorityNormal;
      internal->threadPriority = 0.5;
      internal->ready = YES;
      internal->lock = [NSRecursiveLock new];
      [internal->lock setName:
        [NSString stringWithFormat: @"lock-for-opqueue-%p", self]];
      internal->cond = [[NSConditionLock alloc] initWithCondition: 0];
      [internal->cond setName:
        [NSString stringWithFormat: @"cond-for-opqueue-%p", self]];
      [self addObserver: self
             forKeyPath: @"isFinished"
                options: NSKeyValueObservingOptionNew
                context: isFinishedCtxt];
    }
  return self;
}

- (BOOL) isCancelled
{
  return internal->cancelled;
}

- (BOOL) isExecuting
{
  return internal->executing;
}

- (BOOL) isFinished
{
  return internal->finished;
}

- (BOOL) isConcurrent
{
  return internal->concurrent;
}

- (BOOL) isReady
{
  return internal->ready;
}

- (void) main;
{
  return;	// OSX default implementation does nothing
}

- (void) observeValueForKeyPath: (NSString *)keyPath
		       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context
{
  [internal->lock lock];

  /* We only observe isFinished changes, and we can remove self as an
   * observer once we know the operation has finished since it can never
   * become unfinished.
   */
  [object removeObserver: self forKeyPath: @"isFinished"];

  if (object == self)
    {
      /* We have finished and need to unlock the condition lock so that
       * any waiting thread can continue.
       */
      [internal->cond lock];
      [internal->cond unlockWithCondition: 1];
      [internal->lock unlock];
      return;
    }

  if (NO == internal->ready)
    {
      NSEnumerator	*en;
      NSOperation	*op;

      /* Some dependency has finished (or been removed) ...
       * so we need to check to see if we are now ready unless we know we are.
       * This is protected by locks so that an update due to an observed
       * change in one thread won't interrupt anything in another thread.
       */
      en = [internal->dependencies objectEnumerator];
      while ((op = [en nextObject]) != nil)
        {
          if (NO == [op isFinished])
	    break;
        }
      if (op == nil)
	{
          [self willChangeValueForKey: @"isReady"];
	  internal->ready = YES;
          [self didChangeValueForKey: @"isReady"];
	}
    }
  [internal->lock unlock];
}

- (NSOperationQueuePriority) queuePriority
{
  return internal->priority;
}

- (void) removeDependency: (NSOperation *)op
{
  [internal->lock lock];
  NS_DURING
    {
      if (NSNotFound != [internal->dependencies indexOfObjectIdenticalTo: op])
	{
	  [op removeObserver: self forKeyPath: @"isFinished"];
	  [self willChangeValueForKey: @"dependencies"];
	  [internal->dependencies removeObject: op];
	  if (NO == internal->ready)
	    {
	      /* The dependency may cause us to become ready ...
	       * fake an observation so we can deal with that.
	       */
	      [self observeValueForKeyPath: @"isFinished"
				  ofObject: op
				    change: nil
				   context: isFinishedCtxt];
	    }
	  [self didChangeValueForKey: @"dependencies"];
	}
    }
  NS_HANDLER
    {
      [internal->lock unlock];
      NSLog(@"Problem removing dependency: %@", localException);
      return;
    }
  NS_ENDHANDLER
  [internal->lock unlock];
}

- (void) setCompletionBlock: (GSOperationCompletionBlock)aBlock
{
  ASSIGNCOPY(internal->completionBlock, (id)aBlock);
}

- (void) setQueuePriority: (NSOperationQueuePriority)pri
{
  if (pri <= NSOperationQueuePriorityVeryLow)
    pri = NSOperationQueuePriorityVeryLow;
  else if (pri <= NSOperationQueuePriorityLow)
    pri = NSOperationQueuePriorityLow;
  else if (pri < NSOperationQueuePriorityHigh)
    pri = NSOperationQueuePriorityNormal;
  else if (pri < NSOperationQueuePriorityVeryHigh)
    pri = NSOperationQueuePriorityHigh;
  else
    pri = NSOperationQueuePriorityVeryHigh;

  if (pri != internal->priority)
    {
      [internal->lock lock];
      if (pri != internal->priority)
	{
	  NS_DURING
	    {
	      [self willChangeValueForKey: @"queuePriority"];
	      internal->priority = pri;
	      [self didChangeValueForKey: @"queuePriority"];
	    }
	  NS_HANDLER
	    {
	      [internal->lock unlock];
	      NSLog(@"Problem setting priority: %@", localException);
	      return;
	    }
	  NS_ENDHANDLER
	}
      [internal->lock unlock];
    }
}

- (void) setThreadPriority: (double)pri
{
  if (pri > 1) pri = 1;
  else if (pri < 0) pri = 0;
  internal->threadPriority = pri;
}

- (void) start
{
  ENTER_POOL

  double	prio = [NSThread  threadPriority];

  AUTORELEASE(RETAIN(self));	// Make sure we exist while running.
  [internal->lock lock];
  NS_DURING
    {
      if (YES == [self isExecuting])
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"[%@-%@] called on executing operation",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
	}
      if (YES == [self isFinished])
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"[%@-%@] called on finished operation",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
	}
      if (NO == [self isReady])
	{
	  [NSException raise: NSInvalidArgumentException
		      format: @"[%@-%@] called on operation which is not ready",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
	}
      if (NO == internal->executing)
	{
	  [self willChangeValueForKey: @"isExecuting"];
	  internal->executing = YES;
	  [self didChangeValueForKey: @"isExecuting"];
	}
    }
  NS_HANDLER
    {
      [internal->lock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
  [internal->lock unlock];

  NS_DURING
    {
      if (NO == [self isCancelled])
	{
	  [NSThread setThreadPriority: internal->threadPriority];
	  [self main];
	}
    }
  NS_HANDLER
    {
      [NSThread setThreadPriority:  prio];
      [localException raise];
    }
  NS_ENDHANDLER;

  [self _finish];
  LEAVE_POOL
}

- (double) threadPriority
{
  return internal->threadPriority;
}

- (void) waitUntilFinished
{
  [internal->cond lockWhenCondition: 1];	// Wait for finish
  [internal->cond unlockWithCondition: 1];	// Signal any other watchers
}
@end

@implementation	NSOperation (Private)
/* NB code calling this method must ensure that the receiver is retained
 * until after the method returns.
 */
- (void) _finish
{
  [internal->lock lock];
  if (NO == internal->finished)
    {
      if (YES == internal->executing)
        {
	  [self willChangeValueForKey: @"isExecuting"];
	  [self willChangeValueForKey: @"isFinished"];
	  internal->executing = NO;
	  internal->finished = YES;
	  [self didChangeValueForKey: @"isFinished"];
	  [self didChangeValueForKey: @"isExecuting"];
	}
      else
	{
	  [self willChangeValueForKey: @"isFinished"];
	  internal->finished = YES;
	  [self didChangeValueForKey: @"isFinished"];
	}
      if (NULL != internal->completionBlock)
	{
	  CALL_BLOCK_NO_ARGS(
	    ((GSOperationCompletionBlock)internal->completionBlock));
	}
    }
  [internal->lock unlock];
}

@end


@implementation NSBlockOperation

+ (instancetype) blockOperationWithBlock: (GSBlockOperationBlock)block
{
  NSBlockOperation *op = [[self alloc] init];

  [op addExecutionBlock: block];
  return AUTORELEASE(op);
}

- (void) addExecutionBlock: (GSBlockOperationBlock)block
{
  id	blockCopy = (id)Block_copy(block);

  [_executionBlocks addObject: blockCopy];
  RELEASE(blockCopy);
}

- (void) dealloc
{
  RELEASE(_executionBlocks);
  [super dealloc];
}

- (NSArray *) executionBlocks
{
  return _executionBlocks;
}

- (id) init
{
  self = [super init];
  if (self != nil)
    {
      _executionBlocks = [[NSMutableArray alloc] initWithCapacity: 1];
    }
  return self;
}

- (void) main
{
  NSEnumerator 		*en = [[self executionBlocks] objectEnumerator];
  GSBlockOperationBlock theBlock;

  while ((theBlock = (GSBlockOperationBlock)[en nextObject]) != NULL)
    {
      CALL_BLOCK_NO_ARGS(theBlock);
    }
}
@end


#undef	GSInternal
#define	GSInternal	NSOperationQueueInternal
#include	"GSInternal.h"
GS_PRIVATE_INTERNAL(NSOperationQueue)


@interface	NSOperationQueue (Private)
- (void) _execute;
- (void) _thread;
- (void) observeValueForKeyPath: (NSString *)keyPath
		       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context;
@end

static NSInteger	maxConcurrent = 200;	// Thread pool size

static NSComparisonResult
sortFunc(id o1, id o2, void *ctxt)
{
  NSOperationQueuePriority p1 = [o1 queuePriority];
  NSOperationQueuePriority p2 = [o2 queuePriority];

  if (p1 < p2) return NSOrderedDescending;
  if (p1 > p2) return NSOrderedAscending;
  return NSOrderedSame;
}

static NSString	*threadKey = @"NSOperationQueue";
static NSOperationQueue *mainQueue = nil;

@implementation NSOperationQueue

+ (id) currentQueue
{
  if ([NSThread isMainThread])
    {
      return mainQueue;
    }
  return [[[NSThread currentThread] threadDictionary] objectForKey: threadKey];
}

+ (void) initialize
{
  if (nil == mainQueue)
    {
      mainQueue = [self new];
    }
}

+ (id) mainQueue
{
  return mainQueue;
}

- (void) addOperation: (NSOperation *)op
{
  if (op == nil || NO == [op isKindOfClass: [NSOperation class]])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@] object is not an NSOperation",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  [internal->lock lock];
  if (NSNotFound == [internal->operations indexOfObjectIdenticalTo: op]
    && NO == [op isFinished])
    {
      [op addObserver: self
	   forKeyPath: @"isReady"
	      options: NSKeyValueObservingOptionNew
	      context: isReadyCtxt];
      [self willChangeValueForKey: @"operations"];
      [self willChangeValueForKey: @"operationCount"];
      [internal->operations addObject: op];
      [self didChangeValueForKey: @"operationCount"];
      [self didChangeValueForKey: @"operations"];
      if (YES == [op isReady])
	{
	  [self observeValueForKeyPath: @"isReady"
			      ofObject: op
				change: nil
			       context: isReadyCtxt];
	}
    }
  [internal->lock unlock];
}

- (void) addOperationWithBlock: (GSBlockOperationBlock)block
{
  NSBlockOperation *bop = [NSBlockOperation blockOperationWithBlock: block];
  [self addOperation: bop];
}


- (void) addOperations: (NSArray *)ops
     waitUntilFinished: (BOOL)shouldWait
{
  NSUInteger	total;
  NSUInteger	index;

  if (ops == nil || NO == [ops isKindOfClass: [NSArray class]])
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@] object is not an NSArray",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd)];
    }
  total = [ops count];
  if (total > 0)
    {
      BOOL		invalidArg = NO;
      NSUInteger	toAdd = total;
      GS_BEGINITEMBUF(buf, total, id)

      [ops getObjects: buf];
      for (index = 0; index < total; index++)
	{
	  NSOperation	*op = buf[index];

	  if (NO == [op isKindOfClass: [NSOperation class]])
	    {
	      invalidArg = YES;
	      toAdd = 0;
	      break;
	    }
	  if (YES == [op isFinished])
	    {
	      buf[index] = nil;
	      toAdd--;
	    }
	}
      if (toAdd > 0)
	{
          [internal->lock lock];
	  [self willChangeValueForKey: @"operationCount"];
	  [self willChangeValueForKey: @"operations"];
	  for (index = 0; index < total; index++)
	    {
	      NSOperation	*op = buf[index];

	      if (op == nil)
		{
		  continue;		// Not added
		}
	      if (NSNotFound
		!= [internal->operations indexOfObjectIdenticalTo: op])
		{
		  buf[index] = nil;	// Not added
		  toAdd--;
		  continue;
		}
	      [op addObserver: self
		   forKeyPath: @"isReady"
		      options: NSKeyValueObservingOptionNew
		      context: isReadyCtxt];
	      [internal->operations addObject: op];
	      if (NO == [op isReady])
		{
		  buf[index] = nil;	// Not yet ready
		}
	    }
	  [self didChangeValueForKey: @"operationCount"];
	  [self didChangeValueForKey: @"operations"];
	  for (index = 0; index < total; index++)
	    {
	      NSOperation	*op = buf[index];

	      if (op != nil)
		{
		  [self observeValueForKeyPath: @"isReady"
				      ofObject: op
					change: nil
				       context: isReadyCtxt];
		}
	    }
          [internal->lock unlock];
	}
      GS_ENDITEMBUF()
      if (YES == invalidArg)
	{
	  [NSException raise: NSInvalidArgumentException
	    format: @"[%@-%@] object at index %"PRIuPTR" is not an NSOperation",
	    NSStringFromClass([self class]), NSStringFromSelector(_cmd),
	    index];
	}
    }
  if (YES == shouldWait)
    {
      [self waitUntilAllOperationsAreFinished];
    }
}

- (void) cancelAllOperations
{
  [[self operations] makeObjectsPerformSelector: @selector(cancel)];
}

- (void) dealloc
{
  [self cancelAllOperations];
  DESTROY(internal->operations);
  DESTROY(internal->starting);
  DESTROY(internal->waiting);
  DESTROY(internal->name);
  DESTROY(internal->cond);
  DESTROY(internal->lock);
  GS_DESTROY_INTERNAL(NSOperationQueue);
  [super dealloc];
}

- (id) init
{
  if ((self = [super init]) != nil)
    {
      GS_CREATE_INTERNAL(NSOperationQueue);
      internal->suspended = NO;
      internal->count = NSOperationQueueDefaultMaxConcurrentOperationCount;
      internal->operations = [NSMutableArray new];
      internal->starting = [NSMutableArray new];
      internal->waiting = [NSMutableArray new];
      internal->lock = [NSRecursiveLock new];
      [internal->lock setName:
        [NSString stringWithFormat: @"lock-for-op-%p", self]];
      internal->cond = [[NSConditionLock alloc] initWithCondition: 0];
      [internal->cond setName:
        [NSString stringWithFormat: @"cond-for-op-%p", self]];
    }
  return self;
}

- (BOOL) isSuspended
{
  return internal->suspended;
}

- (NSInteger) maxConcurrentOperationCount
{
  return internal->count;
}

- (NSString*) name
{
  NSString	*s;

  [internal->lock lock];
  if (internal->name == nil)
    {
      internal->name
	= [[NSString alloc] initWithFormat: @"NSOperation %p", self];
    }
  s = RETAIN(internal->name);
  [internal->lock unlock];
  return AUTORELEASE(s);
}

- (NSUInteger) operationCount
{
  NSUInteger	c;

  [internal->lock lock];
  c = [internal->operations count];
  [internal->lock unlock];
  return c;
}

- (NSArray *) operations
{
  NSArray	*a;

  [internal->lock lock];
  a = [NSArray arrayWithArray: internal->operations];
  [internal->lock unlock];
  return a;
}

- (void) setMaxConcurrentOperationCount: (NSInteger)cnt
{
  if (cnt < 0
    && cnt != NSOperationQueueDefaultMaxConcurrentOperationCount)
    {
      [NSException raise: NSInvalidArgumentException
		  format: @"[%@-%@] cannot set negative (%"PRIdPTR") count",
	NSStringFromClass([self class]), NSStringFromSelector(_cmd), cnt];
    }
  [internal->lock lock];
  if (cnt != internal->count)
    {
      [self willChangeValueForKey: @"maxConcurrentOperationCount"];
      internal->count = cnt;
      [self didChangeValueForKey: @"maxConcurrentOperationCount"];
    }
  [internal->lock unlock];
  [self _execute];
}

- (void) setName: (NSString*)s
{
  if (s == nil) s = @"";
  [internal->lock lock];
  if (NO == [internal->name isEqual: s])
    {
      [self willChangeValueForKey: @"name"];
      RELEASE(internal->name);
      internal->name = [s copy];
      [self didChangeValueForKey: @"name"];
    }
  [internal->lock unlock];
}

- (void) setSuspended: (BOOL)flag
{
  [internal->lock lock];
  if (flag != internal->suspended)
    {
      [self willChangeValueForKey: @"suspended"];
      internal->suspended = flag;
      [self didChangeValueForKey: @"suspended"];
    }
  [internal->lock unlock];
  [self _execute];
}

- (void) waitUntilAllOperationsAreFinished
{
  NSOperation	*op;

  [internal->lock lock];
  while ((op = [internal->operations lastObject]) != nil)
    {
      RETAIN(op);
      [internal->lock unlock];
      [op waitUntilFinished];
      RELEASE(op);
      [internal->lock lock];
    }
  [internal->lock unlock];
}
@end

@implementation	NSOperationQueue (Private)

- (void) observeValueForKeyPath: (NSString *)keyPath
		       ofObject: (id)object
                         change: (NSDictionary *)change
                        context: (void *)context
{
  /* We observe three properties in sequence ...
   * isReady (while we wait for an operation to be ready)
   * queuePriority (when priority of a ready operation may change)
   * isFinished (to see if an executing operation is over).
   */
  if (context == isFinishedCtxt)
    {
      [internal->lock lock];
      internal->executing--;
      [object removeObserver: self forKeyPath: @"isFinished"];
      [internal->lock unlock];
      [self willChangeValueForKey: @"operations"];
      [self willChangeValueForKey: @"operationCount"];
      [internal->lock lock];
      [internal->operations removeObjectIdenticalTo: object];
      [internal->lock unlock];
      [self didChangeValueForKey: @"operationCount"];
      [self didChangeValueForKey: @"operations"];
    }
  else if (context == queuePriorityCtxt || context == isReadyCtxt)
    {
      NSInteger pos;

      [internal->lock lock];
      if (context == queuePriorityCtxt)
        {
          [internal->waiting removeObjectIdenticalTo: object];
        }
      if (context == isReadyCtxt)
        {
          [object removeObserver: self forKeyPath: @"isReady"];
          [object addObserver: self
                   forKeyPath: @"queuePriority"
                      options: NSKeyValueObservingOptionNew
                      context: queuePriorityCtxt];
        }
      pos = [internal->waiting insertionPosition: object
                                   usingFunction: sortFunc
                                         context: 0];
      [internal->waiting insertObject: object atIndex: pos];
      [internal->lock unlock];
    }
  [self _execute];
}

- (void) _thread
{
  CREATE_AUTORELEASE_POOL(arp);

  [[[NSThread currentThread] threadDictionary] setObject: self
                                                  forKey: threadKey];
  for (;;)
    {
      NSOperation	*op;
      NSDate		*when;
      BOOL		found;
      /* We use a pool for each operation in case releasing the operation
       * causes it to be deallocated, and the deallocation of the operation
       * autoreleases something which needs to be cleaned up.
       */
      RECREATE_AUTORELEASE_POOL(arp);

      when = [[NSDate alloc] initWithTimeIntervalSinceNow: 5.0];
      found = [internal->cond lockWhenCondition: 1 beforeDate: when];
      RELEASE(when);
      if (NO == found)
	{
	  break;	// Idle for 5 seconds ... exit thread.
	}

      if ([internal->starting count] > 0)
	{
          op = RETAIN([internal->starting objectAtIndex: 0]);
	  [internal->starting removeObjectAtIndex: 0];
	}
      else
	{
	  op = nil;
	}

      if ([internal->starting count] > 0)
	{
	  // Signal any other idle threads,
          [internal->cond unlockWithCondition: 1];
	}
      else
	{
	  // There are no more operations starting.
          [internal->cond unlockWithCondition: 0];
	}

      if (nil != op)
	{
          NS_DURING
	    {
	      ENTER_POOL
              [NSThread setThreadPriority: [op threadPriority]];
              [op start];
	      LEAVE_POOL
	    }
          NS_HANDLER
	    {
	      NSLog(@"Problem running operation %@ ... %@",
		op, localException);
	    }
          NS_ENDHANDLER
	  [op _finish];
          RELEASE(op);
	}
    }

  [[[NSThread currentThread] threadDictionary] removeObjectForKey: threadKey];
  [internal->lock lock];
  internal->threadCount--;
  [internal->lock unlock];
  DESTROY(arp);
  [NSThread exit];
}

/* Check for operations which can be executed and start them.
 */
- (void) _execute
{
  NSInteger	max;

  [internal->lock lock];

  max = [self maxConcurrentOperationCount];
  if (NSOperationQueueDefaultMaxConcurrentOperationCount == max)
    {
      max = maxConcurrent;
    }

  NS_DURING
  while (NO == [self isSuspended]
    && max > internal->executing
    && [internal->waiting count] > 0)
    {
      NSOperation	*op;

      /* Take the first operation from the queue and start it executing.
       * We set ourselves up as an observer for the operating finishing
       * and we keep track of the count of operations we have started,
       * but the actual startup is left to the NSOperation -start method.
       */
      op = [internal->waiting objectAtIndex: 0];
      [internal->waiting removeObjectAtIndex: 0];
      [op removeObserver: self forKeyPath: @"queuePriority"];
      [op addObserver: self
	   forKeyPath: @"isFinished"
	      options: NSKeyValueObservingOptionNew
	      context: isFinishedCtxt];
      internal->executing++;
      if (YES == [op isConcurrent])
	{
          [op start];
	}
      else
	{
	  NSUInteger	pending;

	  [internal->cond lock];
	  pending = [internal->starting count];
	  [internal->starting addObject: op];

	  /* Create a new thread if all existing threads are busy and
	   * we haven't reached the pool limit.
	   */
	  if (0 == internal->threadCount
	    || (pending > 0 && internal->threadCount < POOL))
	    {
	      internal->threadCount++;
	      NS_DURING
		{
		  [NSThread detachNewThreadSelector: @selector(_thread)
					   toTarget: self
					 withObject: nil];
		}
	      NS_HANDLER
		{
		  NSLog(@"Failed to create thread for %@: %@",
		    self, localException);
		}
	      NS_ENDHANDLER
	    }
	  /* Tell the thread pool that there is an operation to start.
	   */
	  [internal->cond unlockWithCondition: 1];
	}
    }
  NS_HANDLER
    {
      [internal->lock unlock];
      [localException raise];
    }
  NS_ENDHANDLER
  [internal->lock unlock];
}

@end

