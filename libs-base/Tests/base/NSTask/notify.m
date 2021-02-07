#import <Foundation/NSTask.h>
#import <Foundation/NSFileHandle.h>
#import <Foundation/NSFileManager.h>
#import <Foundation/NSData.h>
#import <Foundation/NSNotification.h>
#import <Foundation/NSRunLoop.h>
#import <Foundation/NSAutoreleasePool.h>

#import "ObjectTesting.h"

@interface      TaskHandler : NSObject
{
  NSString     *path;
}
@end

@implementation TaskHandler

static BOOL taskTerminationNotificationReceived;

- (void) setLaunchPath: (NSString*)s
{
  ASSIGNCOPY(path, s);
}

- (void) taskDidTerminate: (NSNotification *)notification
{
  NSLog(@"Received NSTaskDidTerminateNotification %@", notification);
  taskTerminationNotificationReceived = YES;
}

- (void) testNSTaskNotifications
{
  NSDate        *deadline;
  BOOL          earlyTermination = NO;

  for (;;)
    {
      NSTask *task = [NSTask new];

      taskTerminationNotificationReceived = NO;
      [[NSNotificationCenter defaultCenter]
        addObserver: self
        selector: @selector(taskDidTerminate:)
        name: NSTaskDidTerminateNotification
        object: task];
      if (earlyTermination)
        {
          BOOL  terminated = NO;
          [task setLaunchPath:
            [path stringByAppendingPathComponent: @"testsleep"]];
          [task launch];
          NSLog(@"Launched pid %d", [task processIdentifier]);
          NSLog(@"Running run loop for 5 seconds");
          deadline = [NSDate dateWithTimeIntervalSinceNow:5.0];
          while ([deadline timeIntervalSinceNow] > 0.0
            && !taskTerminationNotificationReceived)
            {
              [[NSRunLoop currentRunLoop]
                runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 1.0]];
              if (NO == terminated)
                {
                  NSLog(@"Run loop finished, calling -[NSTask terminate]");
                  [task terminate];
                  NSLog(@"Terminate returned, waiting for termination");
                  terminated = YES;
                }
            }
          [task waitUntilExit];
          PASS([task terminationReason]
            == NSTaskTerminationReasonUncaughtSignal,
            "termination reason for signal exit works");
        }
      else
        {
          [task setLaunchPath:
            [path stringByAppendingPathComponent: @"testecho"]];
          [task launch];
          NSLog(@"Launched pid %d", [task processIdentifier]);
          NSLog(@"Running run loop for 15 seconds");
          deadline = [NSDate dateWithTimeIntervalSinceNow: 15.0];
          while ([deadline timeIntervalSinceNow] > 0.0
            && !taskTerminationNotificationReceived)
            {
              [[NSRunLoop currentRunLoop]
                runUntilDate: [NSDate dateWithTimeIntervalSinceNow: 1.0]];
            }
          PASS([task terminationReason]
            == NSTaskTerminationReasonExit,
            "termination reason for normal exit works");
        }
      [task release];
      NSAssert(taskTerminationNotificationReceived,
        @"termination notification not received");
      [[NSNotificationCenter defaultCenter]
        removeObserver: self name: NSTaskDidTerminateNotification object: nil];
      if (earlyTermination)
        break;
      earlyTermination = YES;
    }
}

@end

int main()
{
  TaskHandler   *h;
  NSFileManager *mgr;
  NSString      *helpers;

  START_SET("notify");
  mgr = [NSFileManager defaultManager];
  helpers = [mgr currentDirectoryPath];
  helpers = [helpers stringByAppendingPathComponent: @"Helpers"];
  helpers = [helpers stringByAppendingPathComponent: @"obj"];

  h = [TaskHandler new];
  [h setLaunchPath: helpers];
  [h testNSTaskNotifications];
  [h release];

  END_SET("notify");
  return 0;
}
