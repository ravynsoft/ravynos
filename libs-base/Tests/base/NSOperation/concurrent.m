#import <Foundation/NSArray.h>
#import <Foundation/NSKeyValueObserving.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSOperation.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"

// concurrent operation
@interface MyOperation : NSOperation
{
  int calculation;

  BOOL executing;
  BOOL finished;
}
- (void) completeOperation;
@end

@implementation MyOperation

- (id) initWithValue: (int)aVal
{
  self = [super init];
  if (self) {
    calculation = aVal;
    executing = NO;
    finished = NO;
  }
  return self;
}

- (BOOL) isConcurrent { return YES; }
- (BOOL) isExecuting { return executing; }
- (BOOL) isFinished { return finished; }
- (int) getCalculation { return calculation; }

- (void) main
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    
  [self willChangeValueForKey:@"isExecuting"];
  executing = YES;
  [self didChangeValueForKey:@"isExecuting"];
  // Do the main work of the operation here.
  calculation = 2 * calculation;
    
  [self completeOperation];
  [pool release];
}

- (void) start
{
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];

  // Always check for cancellation before launching the task.
  if ([self isCancelled])
    {
      // Must move the operation to the finished state if it is canceled.
      [self willChangeValueForKey: @"isFinished"];
      finished = YES;
      [self didChangeValueForKey: @"isFinished"];
      return;
    }
  
  // As the operation is not canceled, begin executing the task.
  [NSThread detachNewThreadSelector: @selector(main)
                           toTarget: self
                         withObject: nil];
  [pool release];
}

- (void) completeOperation
{
  [self willChangeValueForKey: @"isFinished"];
  [self willChangeValueForKey: @"isExecuting"];
  
  executing = NO;
  finished = YES;
  
  [self didChangeValueForKey: @"isExecuting"];
  [self didChangeValueForKey: @"isFinished"];
}

@end


int main()
{
  id                    obj;
  NSOperationQueue      *q;
  int                   i;
  NSMutableArray        *a;

  START_SET("concurrent operations")

  // single concurrent operation
  obj = [[MyOperation alloc] initWithValue: 1];
  q = [NSOperationQueue new];
  [q addOperation: obj];
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj isFinished] == YES), "operation ran");
  PASS(([obj isExecuting] == NO), "operation is not executing");
  PASS(([obj getCalculation] == 2), "operation was performed");
  [obj release];

  // multiple concurrent operations
  [q setMaxConcurrentOperationCount: 10];
  a = [NSMutableArray array];
  for (i = 0; i < 5; ++i)
    {
      obj = [[MyOperation alloc] initWithValue: i];
      [a addObject: obj];
      [q addOperation: obj];
    }
  [q waitUntilAllOperationsAreFinished];
  PASS(([obj isFinished] == YES), "operation ran");
  PASS(([obj isExecuting] == NO), "operation is not executing");

  for (i = 0; i < 5; ++i)
    {
      obj = [a objectAtIndex: i];
      PASS(([obj getCalculation] == (2*i)), "operation was performed");
    }

  // multiple concurrent operations
  [q setMaxConcurrentOperationCount: 5];
  a = [NSMutableArray array];
  for (i = 0; i < 10; ++i)
    {
      obj = [[MyOperation alloc] initWithValue: i];
      [a addObject: obj];
      [obj release];
    }
  [q addOperations: a waitUntilFinished: YES];
  PASS(([obj isFinished] == YES), "operation ran");
  PASS(([obj isExecuting] == NO), "operation is not executing");

  for (i = 0; i < 10; ++i)
    {
      obj = [a objectAtIndex: i];
      PASS(([obj getCalculation] == (2*i)), "operation was performed");
    }

  END_SET("concurrent operations")
  return 0;
}
