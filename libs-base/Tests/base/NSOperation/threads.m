#import <Foundation/NSArray.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSOperation.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"


@interface      ThreadCounter : NSObject
{
  unsigned count;
}
- (unsigned) count;
- (void) increment: (NSNotification*)n;
- (void) reset;
@end

@implementation ThreadCounter
- (unsigned) count
{
  return count;
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
  [super dealloc];
}

- (void) increment: (NSNotification*)n
{
  count++;
}

- (id) init
{
  [[NSNotificationCenter defaultCenter] addObserver: self
                                           selector: @selector(increment:)
                                               name: NSThreadWillExitNotification
                                             object: nil];
  return self;
}

- (void) reset
{
  count = 0;
}

@end

@interface      OpFlag : NSOperation
{
  NSOperationQueue      *queue;
  NSThread              *thread;
  BOOL                  flag;
}
- (void) main;
- (NSOperationQueue*) queue;
- (BOOL) ran;
- (NSThread*) thread;
@end

@implementation OpFlag
- (void) main
{
  flag = YES;
  queue = [NSOperationQueue currentQueue];
  thread = [NSThread currentThread];
}
- (NSOperationQueue*) queue
{
  return queue;
}
- (BOOL) ran
{
  return flag;
}
- (NSThread*) thread
{
  return thread;
}

- (void) release
{
  // NSLog(@"Will release %@ at %@", self, [NSThread callStackSymbols]);
  [super release];
}

- (id) retain
{
  // NSLog(@"Will retain %@ at %@", self, [NSThread callStackSymbols]);
  return [super retain];
}

@end

@interface OpExit : OpFlag
@end

@implementation OpExit 
- (void) main
{
  [super main];
  [NSThread exit];
}
@end

@interface OpRaise : OpFlag
@end

@implementation OpRaise 
- (void) main
{
  [super main];
  [NSException raise: NSGenericException format: @"done"];
}
@end

@interface      OpOrder : NSOperation
@end
@implementation OpOrder
static NSLock *lock = nil;
static NSMutableArray *list = nil;
+ (void) initialize
{
  lock = [NSLock new];
  list = [NSMutableArray new];
}
- (void) main
{
  [lock lock];
  [list addObject: self];
  [lock unlock];
}
@end

int main()
{
  ThreadCounter         *cnt;
  int			tries;
  id			old;
  id                    obj;
  NSMutableArray        *a;
  NSOperationQueue      *q;
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];

  cnt = [ThreadCounter new];
  PASS((cnt != nil && [cnt count] == 0), "counter was set up");

  // Check that operation runs in current thread.
  obj = [OpFlag new];
  [obj start];
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] == [NSThread currentThread]), "operation ran in this thread");
  PASS(([obj queue] == [NSOperationQueue mainQueue]), "operation ran in main queue");
  [obj release];

  // Check that monitoring of another thread works.
  obj = [OpFlag new];
  [NSThread detachNewThreadSelector: @selector(start)
                           toTarget: obj
                         withObject: nil];
  [NSThread sleepForTimeInterval: 0.1];
  PASS(([obj isFinished] == YES), "operation finished");
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] != [NSThread currentThread]), "operation ran in other thread");
  [obj release];

  // Check that exit from thread in -main causes operation tracking to fail.
  obj = [OpExit new];
  [NSThread detachNewThreadSelector: @selector(start)
                           toTarget: obj
                         withObject: nil];
  [NSThread sleepForTimeInterval: 0.1];
  PASS(([obj isFinished] == NO), "operation exited");
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] != [NSThread currentThread]), "operation ran in other thread");
  PASS(([obj isExecuting] == YES), "operation seems to be running");
  [obj release];

  // Check that raising exception in -main causes operation tracking to fail.
  obj = [OpRaise new];
  PASS_EXCEPTION([obj start];,
  		 NSGenericException,
		 "NSOperation exceptions propogate from main");
  PASS(([obj isFinished] == NO), "operation failed to finish");
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] == [NSThread currentThread]), "operation ran in this thread");
  PASS(([obj isExecuting] == YES), "operation seems to be running");
  [obj release];

  obj = [OpFlag new];
  [obj start];
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] == [NSThread currentThread]), "operation ran in this thread");
  [obj release];

  obj = [OpFlag new];
  q = [NSOperationQueue new];
  PASS(q != [NSOperationQueue mainQueue], "my queue is not main queue");
  PASS(q != [NSOperationQueue currentQueue], "my queue is not current queue");
  [cnt reset];
  [q addOperation: obj];
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj ran] == YES), "operation ran");
  PASS(([obj thread] != [NSThread currentThread]), "operation ran in other thread");
  PASS(([obj queue] == q), "operation ran in my queue");

  PASS(([cnt count] == 0), "thread did not exit immediately");
  [obj release];
  /* Observed behavior on OSX 10.6 is that the thread exits after five seconds
   * and the base library copies that 5 second lifetime... but who knows what
   * that might change to in future?
   */
  [NSThread sleepForTimeInterval: 5.0];
  /* Allow some extra time in case the machine is slow etc.
   */
  for (tries = 0; [cnt count] == 0 && tries < 50; tries++)
    {
      [NSThread sleepForTimeInterval: 0.1];
    }
  PASS(([cnt count] == 1), "thread exit occurs withing ten seconds");

  PASS(([NSOperationQueue currentQueue] == [NSOperationQueue mainQueue]), "current queue outside -main is main queue");
  PASS(([NSOperationQueue mainQueue] != nil), "main queue is not nil");
  obj = [OpFlag new];
  [q addOperation: obj];
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj isFinished] == YES), "main queue runs an operation");
  PASS(([obj thread] != [NSThread currentThread]),
    "operation ran in other thread");

  [q setSuspended: YES];
  obj = [OpFlag new];
  [q addOperation: obj];
  [NSThread sleepForTimeInterval: 0.1];
  PASS(([obj isFinished] == NO), "suspend works");
  [q setSuspended: NO];
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj isFinished] == YES), "unsuspend works");
  [obj release];

  [q setMaxConcurrentOperationCount: 0];
  obj = [OpFlag new];
  [q addOperation: obj];
  [NSThread sleepForTimeInterval: 0.1];
  PASS(([obj isFinished] == NO), "max operation count of zero suspends queue");
  [q setMaxConcurrentOperationCount: 1];
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj isFinished] == YES), "resetting max operation queue sarts it");
  [obj release];

  a = [NSMutableArray array];

  [q setSuspended: YES];
  obj = [OpOrder new];
  [a addObject: obj];
  [obj release];
  obj = [OpOrder new];
  [a addObject: obj];
  [obj release];
  obj = [OpOrder new];
  [a addObject: obj];
  [obj release];
  [q setSuspended: NO];
  [q addOperations: a waitUntilFinished: YES];
  PASS(([list isEqual: a]), "operations ran in order of addition");

  [list removeAllObjects];
  [a removeAllObjects];
  [q setSuspended: YES];
  obj = [OpOrder new];
  [obj setQueuePriority: NSOperationQueuePriorityHigh];
  [a addObject: obj];
  [q addOperation: obj];
  [obj release];
  obj = [OpOrder new];
  [a addObject: obj];
  [q addOperation: obj];
  [obj release];
  obj = [OpOrder new];
  [obj setQueuePriority: NSOperationQueuePriorityLow];
  [a addObject: obj];
  [q addOperation: obj];
  [obj release];
  obj = [a objectAtIndex: 1];
  [obj setQueuePriority: NSOperationQueuePriorityVeryLow];
  [a addObject: obj];
  [a removeObjectAtIndex: 1];
  [q setSuspended: NO];
  [q waitUntilAllOperationsAreFinished];
  PASS(([list isEqual: a]), "operations ran in order of priority");

  [list removeAllObjects];
  [a removeAllObjects];
  [q setSuspended: YES];
  old = [OpOrder new];
  [a addObject: old];
  [old release];
  [old setQueuePriority: NSOperationQueuePriorityLow];
  obj = [OpOrder new];
  [a addObject: obj];
  [obj release];
  [old addDependency: obj];
  obj = [OpOrder new];
  [a addObject: obj];
  [obj release];
  [obj setQueuePriority: NSOperationQueuePriorityHigh];
  [obj addDependency: old];
  [q setSuspended: NO];
  [q addOperations: a waitUntilFinished: YES];
  PASS(([list objectAtIndex: 0] == [a objectAtIndex: 1]
    && [list objectAtIndex: 1] == [a objectAtIndex: 0]),
    "operations ran in order of dependency");
  PASS(1 == [[old dependencies] count], "dependencies not removed when done")

  [arp release]; arp = nil;
  return 0;
}
