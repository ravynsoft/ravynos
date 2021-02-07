#import <Foundation/NSDictionary.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSLock.h>
#include <unistd.h>
#import "Testing.h"

@interface Init0 : NSObject
@end

@interface Init1 : Init0
@end

@interface SlowInit0
@end

@interface SlowInit1 : SlowInit0 
+ (void) doNothing;
@end

@interface SlowInit2
+ (void) doNothing;
@end

@interface Mutual1 : NSObject
@end

@interface Mutual2 : NSObject
@end

static NSCondition *l;
static volatile int init0, init1, init2, init3;
static int initCount = 0;

@implementation Init0
+ (void) initialize
{
  initCount++;
}
@end

@implementation	Init1
@end

@implementation SlowInit0
+ (void) initialize
{
  init0 = 1;
}
@end

@implementation SlowInit1
/**
 * Called from main thread.
 */
+ (void) initialize
{
  PASS(init0 == 1, "Superclass +initialize called before subclass");
  // Spin until we've entered the second +initialize method
  while (init2 == 0) {}
  // Wake up the other thread
  [l signal];
  [l unlock];
  // Trigger the 
  [SlowInit2 doNothing];
  init1 = 1;
}
+ (void) doNothing {}
@end

@implementation SlowInit2
/**
 * Called from the second thread. 
 */
+ (void) initialize
{
  init2++;
  // Sleep until the main thread is ready for us
  [l lock];
  /* If the runtime is doing the wrong thing and this is called twice, then
   * there will be no signal.  We don't want to deadlock, so make sure that
   * this times out after a short while.
   */
  [l waitUntilDate: [NSDate dateWithTimeIntervalSinceNow: 1.5]];
  [l unlock];
  [NSThread sleepForTimeInterval: 1];
  PASS(init1 == 0, "First initialize method did not finish too early");
  init3++;
}
static volatile int finished = 2;
+ (void) doNothing
{
  PASS(init2 == 1, "+initialize called exactly once");
  PASS(init3 == 1, "+initialize completed before another method started");
  finished--;
}
@end

@interface Trampoline : NSObject
+ (void) launch: (id)ignored;
@end

@implementation Trampoline
/**
 * Launch the second thread.  NSThread retains its arguments in the main
 * thread, we need to ensure that nothing triggers the second +initialize
 * method until we're in the second thread.
 */
+ (void) launch: (id)ignored
{
  [NSAutoreleasePool new];
  [SlowInit2 doNothing];
}
@end


static int	mutual = 0;

@implementation Mutual1
+ (void) initialize
{
  [NSThread sleepForTimeInterval: 0.5];
  [Mutual2 class];
  mutual++;
}
@end

@implementation Mutual2
+ (void) initialize
{
  [NSThread sleepForTimeInterval: 0.5];
  [Mutual1 class];
  mutual++;
}
@end

@interface NSObject (Mutual)
+ (void) mutual1: (id)ignored;
+ (void) mutual2: (id)ignored;
@end

@implementation NSObject (Mutual)
+ (void) mutual1: (id)ignored
{
  [Mutual1 class];
}
+ (void) mutual2: (id)ignored
{
  [Mutual2 class];
}
@end

static void
mutualinit(int sig)
{
  /* Generate a dashed hope for mutuality testing.
   */
  testHopeful = YES;
  PASS(0, "+initialize mutually dependent methods work");
  exit(0);
}

static void
concurrency(int sig)
{
  /* Generate a dashed hope for concurrency testing.
   */
  testHopeful = YES;
  PASS(0, "+initialize runs concurrently");
  exit(0);
}

/**
 * Test the behaviour of +initialize.
 * It's an undocumented (but nice) feature that the Apple runtime lets
 * both of the +initialize methods run concurrently, however the first
 * one will block implicitly until the second one has completed.
 */
int main(void)
{
  NSDate	*when;

  [NSAutoreleasePool new];

  START_SET("+initialize")

  /* Make sure we have initalised all the classes necessary for the test
   * framework to record a test ... by passing one.
   */
  PASS(1, "initialize test starts");

#if defined(SIGALRM)
  /* End in a signal if the concurrency test deadlocks.
   */
  signal(SIGALRM, mutualinit);
  alarm(5);
#else
  SKIP("+initialize runs concurrently. No SIGALRM present, this means we cannot stop the test on deadlock.");
#endif
  /* Make sure that classes whose +initialise methods call each other
   * can operate safely.
   */
  [NSThread detachNewThreadSelector: @selector(mutual1:)
			   toTarget: [NSObject class]
			 withObject: nil];
  [NSThread detachNewThreadSelector: @selector(mutual2:)
			   toTarget: [NSObject class]
			 withObject: nil];
  when = [NSDate dateWithTimeIntervalSinceNow: 10.0];
  while (mutual < 2 && [when timeIntervalSinceNow] > 0.0)
    {
      [NSThread sleepForTimeInterval: 0.1];
    }
#if defined(SIGALRM)
  signal(SIGALRM, SIG_IGN);
#endif
  PASS(2 == mutual, "mutually dependent +initialize is thread-safe");

  /* Make sure that when a class without its own +initialise is first used,
   * the inherited +initialize is called instead.
   * This is a traditional feature of the Apple runtime, which was 'improved'
   * by the GNU runtime (which actually checks to see if +initialize is
   * implemented before trying to call it).  Good software should not depend
   * on either behavior ... which means all good software should protext its
   * +initialize code so that being called multiple times is OK.
   * New GNU/GNUstep runtimes adopt the old Apple behavior purely for
   * consistency.
   */
  [Init1 class];
  testHopeful = YES;
  PASS(2 == initCount, "inherited +initialize is called automatically");
  testHopeful = NO;

#if defined(SIGALRM)
  /* End in a signal if the concurrency test deadlocks.
   */
  signal(SIGALRM, concurrency);
  alarm(5);
#else
  SKIP("+initialize runs concurrently. No SIGALRM present, this means we cannot stop the test on deadlock.");
#endif

  l = [NSCondition new];
  [l lock];
  [NSThread detachNewThreadSelector: @selector(launch:)
			   toTarget: [Trampoline class]
			 withObject: nil];
  [NSThread sleepForTimeInterval: 0.5];
  [SlowInit1 doNothing];
  [l lock];
  [l unlock];
  while (finished)
    {
      [NSThread sleepForTimeInterval: 0.01];
    }

  END_SET("+initialize")

  return 0;
}
