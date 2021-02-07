#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSInvocation.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSThread.h>

static unsigned counter = 0;
@interface      MyClass : NSObject
- (void) incrementCounter;
@end
@implementation MyClass
- (void) incrementCounter
{
  counter = counter + 1;
  NSLog(@"Counter is %u", counter);
}
@end

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSString              *customMode = @"CustomRunLoopMode";
  MyClass               *dummy = [MyClass new];
  NSMethodSignature     *sig;
  NSInvocation          *inv;
  NSTimer	        *dly;
  NSTimer	        *tim;
  NSRunLoop	        *run;
  NSDate	        *date;
  NSTimeInterval        ti;

  sig = [dummy methodSignatureForSelector: @selector(incrementCounter)];
  inv = [NSInvocation invocationWithMethodSignature: sig];
  [inv setSelector: @selector(incrementCounter)];
  [inv setTarget: dummy];
  
  /* Ensure the runloop has an 'input source' for events.
   */
  dly = [NSTimer scheduledTimerWithTimeInterval: 120.0
                                     invocation: inv
                                        repeats: NO];

  run = [NSRunLoop currentRunLoop];
  PASS(run != nil, "NSRunLoop understands [+currentRunLoop]");
  PASS([run currentMode] == nil, "-currentMode returns nil");

  ti = [NSDate timeIntervalSinceReferenceDate]; 
  PASS_RUNS(date = [NSDate dateWithTimeIntervalSinceNow: 1.0];
    [run runUntilDate: date];,
    "-runUntilDate: works");
  ti = [NSDate timeIntervalSinceReferenceDate] - ti;
  PASS(ti >= 1.0 && ti < 1.5, "-runUntilDate: takes the correct time");

  ti = [NSDate timeIntervalSinceReferenceDate]; 
  PASS_RUNS([run runUntilDate: [NSDate distantPast]];,
    "-runUntilDate: works for distant past");
  ti = [NSDate timeIntervalSinceReferenceDate] - ti;
  PASS(ti < 0.2, "-runUntilDate: takes very short time");

  ti = [NSDate timeIntervalSinceReferenceDate]; 
  PASS_RUNS([run runUntilDate: nil];,
    "-runUntilDate: works for nil date");
  ti = [NSDate timeIntervalSinceReferenceDate] - ti;
  PASS(ti < 0.2, "-runUntilDate: for nil date takes very short time");

  ti = [NSDate timeIntervalSinceReferenceDate]; 
  PASS_RUNS([run runMode: NSDefaultRunLoopMode beforeDate: nil];,
    "-runMode:beforeDate: works for nil date");
  ti = [NSDate timeIntervalSinceReferenceDate] - ti;
  PASS(ti < 0.2, "-runMode:beforentilDate: for nil date takes very short time");

  tim = [NSTimer scheduledTimerWithTimeInterval: 0.005
                                     invocation: inv
                                        repeats: NO];
  [NSThread sleepForTimeInterval: 0.01];
  [run runUntilDate: [NSDate distantPast]];
  PASS(1 == counter, "-runUntilDate: for distant past fires timer");

  [dly invalidate];

  /* We run in a custom mode to ensure there are no other timers or
   * input sources.
   */
  tim = [NSTimer timerWithTimeInterval: 2.0
                            invocation: inv
                               repeats: NO];
  [run addTimer: tim forMode: customMode];
  ti = [NSDate timeIntervalSinceReferenceDate]; 
  PASS_RUNS(date = [NSDate dateWithTimeIntervalSinceNow: 5.0];
    [run acceptInputForMode: customMode beforeDate: date];,
     "-acceptInputForMode:beforeDate: works with a timer");
  ti = [NSDate timeIntervalSinceReferenceDate] - ti;
  PASS(ti >= 2.0 && ti < 2.5,
    "-acceptInputForMode:beforeDate: takes the correct time");
  PASS(2 == counter, "-acceptInputForMode:beforeDate: fires timer");

  [arp release]; arp = nil;
  return 0;
}
