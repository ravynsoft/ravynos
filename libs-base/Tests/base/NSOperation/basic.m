#import <Foundation/NSOperation.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"


# ifndef __has_feature
# define __has_feature(x) 0
# endif

static BOOL blockDidRun = NO;

int main()
{
  id                    obj1;
  id                    obj2;
  NSMutableArray        *testObjs = [[NSMutableArray alloc] init];
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];

  test_alloc(@"NSOperation");
  obj1 = [NSOperation new];
  PASS((obj1 != nil), "can create an operation");
  [testObjs addObject: obj1];
  test_NSObject(@"NSOperation", testObjs);

  PASS(([obj1 isReady] == YES), "operation is ready");
  PASS(([obj1 isConcurrent] == NO), "operation is not concurrent");
  PASS(([obj1 isCancelled] == NO), "operation is not cancelled");
  PASS(([obj1 isExecuting] == NO), "operation is not executing");
  PASS(([obj1 isFinished] == NO), "operation is not finished");
  PASS(([[obj1 dependencies] isEqual: [NSArray array]]),
    "operation has no dependencies");
  PASS(([obj1 queuePriority] == NSOperationQueuePriorityNormal),
    "operation has normal priority");
  [obj1 setQueuePriority: 10000];
  PASS(([obj1 queuePriority] == NSOperationQueuePriorityVeryHigh),
    "operation has very high priority");

  obj2 = [NSOperation new];
  [obj2 addDependency: obj1];
  PASS(([[obj2 dependencies] isEqual: testObjs]),
    "operation has added dependency");
  PASS(([obj2 isReady] == NO), "operation with dependency is not ready");
  [obj1 cancel];
  PASS(([[obj2 dependencies] isEqual: testObjs]),
    "cancelled dependency continues");
  PASS(([obj1 isCancelled] == YES), "operation is cancelled");
  PASS(([[obj2 dependencies] isEqual: testObjs]),
    "cancelled dependency continues");
  PASS(([obj2 isReady] == NO), "operation with cancelled dependency not ready");
  [obj2 removeDependency: obj1];
  PASS(([[obj2 dependencies] isEqual: [NSArray array]]),
    "dependency removal works");
  PASS(([obj2 isReady] == YES), "operation without dependency is ready");

  [obj1 release];
  obj1 = [NSOperation new];
  [testObjs replaceObjectAtIndex: 0 withObject: obj1];
  [obj2 addDependency: obj1];
# if __has_feature(blocks)
  [obj1 setCompletionBlock: ^(void){blockDidRun = YES;}];
# endif
  [obj1 start];
  [NSThread sleepForTimeInterval: 1.0];
  PASS(([obj1 isFinished] == YES), "operation is finished");
  PASS(([obj1 isReady] == YES), "a finished operation is ready");
# if __has_feature(blocks)
  PASS(YES == blockDidRun, "completion block is executed");
# endif
  PASS(([[obj2 dependencies] isEqual: testObjs]),
    "finished dependency continues");
  PASS(([obj2 isReady] == YES), "operation with finished dependency is ready");

  [obj2 removeDependency: obj1];
  [obj1 release];
  obj1 = [NSOperation new];
  [testObjs replaceObjectAtIndex: 0 withObject: obj1];
  [obj2 addDependency: obj1];
  [obj2 cancel];
  PASS(([obj2 isReady] == YES),
    "a cancelled object is ready even with a dependency");

  [obj2 start];
  PASS(([obj2 isFinished] == YES),
    "a cancelled object can finish");

  PASS_EXCEPTION([obj2 start];,
  		 NSInvalidArgumentException,
		 "NSOperation cannot be started twice");

  PASS(([obj2 waitUntilFinished], YES), "wait returns at once");


  test_alloc(@"NSOperationQueue");
  obj1 = [NSOperationQueue new];
  PASS((obj1 != nil), "can create an operation queue");
  [testObjs removeAllObjects];
  [testObjs addObject: obj1];
  test_NSObject(@"NSOperationQueue", testObjs);

  PASS(([obj1 isSuspended] == NO), "not suspended by default");
  [obj1 setSuspended: YES];
  PASS(([obj1 isSuspended] == YES), "set suspended yes");
  [obj1 setSuspended: NO];
  PASS(([obj1 isSuspended] == NO), "set suspended no");

  PASS(([[obj1 name] length] > 0), "name has a default");
  [obj1 setName: @"mine"];
  PASS(([[obj1 name] isEqual: @"mine"] == YES), "set name OK");
  [obj1 setName: nil];
  PASS(([[obj1 name] isEqual: @""]), "setting null name gives empty string");

  PASS(([obj1 maxConcurrentOperationCount] == NSOperationQueueDefaultMaxConcurrentOperationCount), "max concurrent set by default");
  [obj1 setMaxConcurrentOperationCount: 1];
  PASS(([obj1 maxConcurrentOperationCount] == 1), "max concurrent set to one");
  [obj1 setMaxConcurrentOperationCount: 0];
  PASS(([obj1 maxConcurrentOperationCount] == 0), "max concurrent set to zero");
  [obj1 setMaxConcurrentOperationCount: 1000000];
  PASS(([obj1 maxConcurrentOperationCount] == 1000000), "max concurrent set to a million");
  [obj1 setMaxConcurrentOperationCount: NSOperationQueueDefaultMaxConcurrentOperationCount];
  PASS(([obj1 maxConcurrentOperationCount] == NSOperationQueueDefaultMaxConcurrentOperationCount), "max concurrent set to default");
  PASS_EXCEPTION([obj1 setMaxConcurrentOperationCount: -1000000];,
  		 NSInvalidArgumentException,
		 "NSOperationQueue cannot be given negative count");

  obj2 = [NSOperation new];
  [obj1 addOperation: obj2];
  [NSThread sleepForTimeInterval: 1.0];
  PASS(([obj2 isFinished] == YES), "queue ran operation");

  [arp release]; arp = nil;
  return 0;
}
