#import "ObjectTesting.h"
#import <Foundation/NSThread.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSNotification.h>
#include <pthread.h>




@interface ThreadExpectation : NSObject <NSLocking>
{
  NSCondition *condition;
  NSThread *origThread;
  BOOL done;
  BOOL deallocated;
}

- (void) onThreadExit: (NSNotification*)n;
- (BOOL) isDone;
@end

@implementation ThreadExpectation

- (id) init
{
  if (nil == (self = [super init]))
    {
      return nil;
    }
  condition = [NSCondition new];
  return self;
}



- (void) inThread: (NSThread*)thread
{
  NSNotificationCenter  *nc = [NSNotificationCenter defaultCenter];

  /* We explicitly don't retain this so that we can check that it actually says
   * alive until the notification is sent. That check is implicit since
   * PASS_EQUAL in the -onThreadExit method will throw or crash if that isn't
   * the case.
   */
  origThread = thread;
  [nc addObserver: self
         selector: @selector(onThreadExit:)
             name: NSThreadWillExitNotification
           object: thread];
}

- (void) onThreadExit: (NSNotification*)thr
{
  NSThread      *current = [NSThread currentThread];
  NSThread      *passed = [thr object];

  PASS_EQUAL(passed, origThread,
    "NSThreadWillExitNotification passes expected thread")
  PASS_EQUAL(origThread, current,
    "Correct thread reference can be obtained on exit")
  PASS([passed isExecuting],
    "exiting thread is still executing at point of notification")
  PASS(![passed isFinished],
    "exiting thread is not finished at point of notification")

  [[NSNotificationCenter defaultCenter] removeObserver: self];
  origThread = nil;
  [condition lock];
  done = YES;
  [condition broadcast];
  [condition unlock];
}

- (BOOL) isDone
{
  return done;
}

- (void) waitUntilDate: (NSDate*)date
{
  [condition waitUntilDate: date];
}

- (void) lock
{
  [condition lock];
}

- (void) unlock
{
  [condition unlock];
}

- (void) dealloc
{
  DESTROY(condition);
  [super dealloc];
}
@end

void *thread(void *expectation)
{
  [(ThreadExpectation*)expectation inThread: [NSThread currentThread]];
  return nil;
}



/**
 * This test checks whether we can still obtain a reference to the NSThread
 * object of a thread that is in the process of exiting without an explicit
 * call to [NSThread exit]. To do this, we pass an expectation object to
 * a thread created purely using the pthreads API. We then wait on a condition
 * until the thread exits and posts the NSThreadWillExitNotification. If that
 * does not happen within 5 seconds, we flag the test as failed.
 */
int main(void)
{
  pthread_t thr;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  NSAutoreleasePool *arp = [NSAutoreleasePool new];
  ThreadExpectation *expectation = [ThreadExpectation new];
  pthread_create(&thr, &attr, thread, expectation);

  NSDate *start = [NSDate date];
  [expectation lock];
  while (![expectation isDone] && [start timeIntervalSinceNow] > -5.0f)
  {
    [expectation waitUntilDate: [NSDate dateWithTimeIntervalSinceNow: 0.5f]];
  }
  PASS([expectation isDone], "Notification for thread exit was sent");
  [expectation unlock];
  DESTROY(expectation);
  DESTROY(arp);
  return 0;
}
