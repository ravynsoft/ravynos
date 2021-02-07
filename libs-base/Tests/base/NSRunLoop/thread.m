#import "ObjectTesting.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSTimer.h>
#import <Foundation/NSRunLoop.h>

@interface ThreadTest : NSObject {
  char  acceptEmptyBlocks;
  char  acceptTimerBlocks;
  char  blockForEmpty;
  char  blockForInput;
  char  blockForTimer;
  char  limitForEmpty;
  char  limitForInput;
  char  limitForTimer;
  char  moreForEmpty;
  char  moreForInput;
  char  moreForTimer;
  char  performed;
}
- (void) notified: (NSNotification*)n;
- (void) timeout: (NSTimer*)t;
- (void) thread1: (id)o;
@end

@implementation ThreadTest

- (void) notified: (NSNotification*)n
{
  NSLog(@"Notified: %@", n);
}

- (void) timeout: (NSTimer*)t
{
}

- (void) thread1: (id)o
{
  NSAutoreleasePool     *pool = [NSAutoreleasePool new];
  NSNotificationCenter	*nc;
  NSRunLoop             *loop;
  NSFileHandle          *fh;
  NSTimer               *timer;
  NSDate                *end;
  NSDate                *start;
 
  nc = [NSNotificationCenter defaultCenter];
  loop = [NSRunLoop currentRunLoop];

  end = [loop limitDateForMode: NSDefaultRunLoopMode];
  if (end == nil)
    limitForEmpty = 'N';
  else
    limitForEmpty = 'Y';

  end = [NSDate dateWithTimeIntervalSinceNow: 0.2];
  start = [NSDate date];
  if ([loop runMode: NSDefaultRunLoopMode beforeDate: end] == YES)
    moreForEmpty = 'Y';
  else
    moreForEmpty = 'N';
  if (fabs([start timeIntervalSinceNow]) < 0.01)
    blockForEmpty = 'N';
  else
    blockForEmpty = 'Y';
  
  end = [NSDate dateWithTimeIntervalSinceNow: 0.2];
  start = [NSDate date];
  [loop acceptInputForMode: NSDefaultRunLoopMode beforeDate: end];
  if (fabs([start timeIntervalSinceNow]) < 0.01)
    acceptEmptyBlocks = 'N';
  else
    acceptEmptyBlocks = 'Y';
  
  timer = [NSTimer timerWithTimeInterval: 2.0
                                  target: self
                                selector: @selector(timeout:)
                                userInfo: nil
                                 repeats: NO];
  [loop addTimer: timer forMode: NSDefaultRunLoopMode];
  end = [loop limitDateForMode: NSDefaultRunLoopMode];
  if (fabs([end timeIntervalSinceDate: [timer fireDate]]) < 0.01)
    limitForTimer = 'Y';
  else
    limitForTimer = 'N';
  end = [NSDate dateWithTimeIntervalSinceNow: 0.2];
  start = [NSDate date];
  if ([loop runMode: NSDefaultRunLoopMode beforeDate: end] == YES)
    moreForTimer = 'Y';
  else
    moreForTimer = 'N';
  if (fabs([start timeIntervalSinceNow]) < 0.01)
    blockForTimer = 'N';
  else
    blockForTimer = 'Y';

  end = [NSDate dateWithTimeIntervalSinceNow: 0.2];
  start = [NSDate date];
  [loop acceptInputForMode: NSDefaultRunLoopMode beforeDate: end];
  if (fabs([start timeIntervalSinceNow]) < 0.01)
    acceptTimerBlocks = 'N';
  else
    acceptTimerBlocks = 'Y';
  
  [timer invalidate];

  fh = [[NSPipe pipe] fileHandleForReading];
  [nc addObserver: self selector:@selector(notified:) 
  	     name: nil object:fh];
  [fh readInBackgroundAndNotify];
  end = [loop limitDateForMode: NSDefaultRunLoopMode];
  if ([end isEqual: [NSDate distantFuture]] == YES)
    limitForInput = 'Y';
  else
    limitForInput = 'N';
  end = [NSDate dateWithTimeIntervalSinceNow: 0.2];
  start = [NSDate date];
  if ([loop runMode: NSDefaultRunLoopMode beforeDate: end] == YES)
    moreForInput = 'Y';
  else
    moreForInput = 'N';
  [timer invalidate];
  if (fabs([start timeIntervalSinceNow]) < 0.01)
    blockForInput = 'N';
  else
    blockForInput = 'Y';
  [nc removeObserver: self];
  [pool release];
}

- (void) thread2: (id)o
{
  NSAutoreleasePool     *pool = [NSAutoreleasePool new];
  NSRunLoop             *loop;
  NSDate                *end;
 
  loop = [NSRunLoop currentRunLoop];

  [NSTimer scheduledTimerWithTimeInterval: 2.0
                                   target: self
                                 selector: @selector(timeout:)
                                 userInfo: nil
                                  repeats: NO];

  end = [NSDate dateWithTimeIntervalSinceNow: 2.0];
  while ([end timeIntervalSinceNow] > 0)
    {
      [loop runUntilDate: end];
    }
  [pool release];
}

- (void) threadEvent: (id)ignored
{
  performed = 'Y';
}

- (void) run
{
  NSDate                *until = [NSDate dateWithTimeIntervalSinceNow: 5.0];
  NSThread              *t;
  
  [NSTimer scheduledTimerWithTimeInterval: 5.0
                                   target: self
                                 selector: @selector(timeout:)
                                 userInfo: nil
                                  repeats: YES];

  [NSThread detachNewThreadSelector: @selector(thread1:)
                           toTarget: self
                         withObject: nil];
  t = [[NSThread alloc] initWithTarget: self
                              selector: @selector(thread2:)
                                object: nil];
  [t start];

  [self performSelector: @selector(threadEvent:)
               onThread: t
             withObject: nil
          waitUntilDone: NO];
  while ([until timeIntervalSinceNow] > 0)
    {
      [[NSRunLoop currentRunLoop] runUntilDate: until];
    }

  PASS(acceptEmptyBlocks == 'N', "Accept with no inputs or timers will exit");
  PASS(acceptTimerBlocks == 'Y', "Accept with timers will not exit");
  PASS(blockForEmpty == 'N', "A loop with no inputs or timers will exit");
  PASS(blockForInput == 'Y', "A loop with an input source will block");
  PASS(blockForTimer == 'Y', "A loop with a timer will block");
  PASS(limitForEmpty == 'N', "A loop with no inputs or timers has no limit");
  PASS(limitForInput == 'Y', "A loop with an input source has distant future");
  PASS(limitForTimer == 'Y', "A loop with a timer has timer fire date");
  PASS(moreForEmpty == 'N', "A loop with no inputs or timers has no more");
  PASS(moreForInput == 'Y', "A loop with an input source has more");
  PASS(moreForTimer == 'Y', "A loop with a timer has more");
  PASS(performed == 'Y', "Methods will be performed in a loop without inputs");
}

@end

int main(int argc, char *argv[])
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  [[[ThreadTest new] autorelease] run];
  [pool release];
  return 0;
}
