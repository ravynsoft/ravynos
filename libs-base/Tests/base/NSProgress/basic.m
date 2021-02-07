#import <Foundation/NSProgress.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSDictionary *dict = [NSDictionary dictionary];
  NSProgress *progress = [[NSProgress alloc] initWithParent: nil
                                                   userInfo: dict];
  PASS(progress != nil, "[NSProgress initWithParent:userInfo:] returns instance");
  
  progress = [NSProgress discreteProgressWithTotalUnitCount:100];
  PASS(progress != nil, "[NSProgress discreteProgressWithTotalUnitCount:] returns instance");
  
  progress = [NSProgress progressWithTotalUnitCount:100];
  PASS(progress != nil, "[NSProgress progressWithTotalUnitCount:] returns instance");
  
  progress = [NSProgress progressWithTotalUnitCount:100
                                             parent:progress
                                   pendingUnitCount:50];
  PASS(progress != nil, "[NSProgress progressWithTotalUnitCount:] returns instance");
  
  [progress becomeCurrentWithPendingUnitCount:50];
  NSProgress *currentProgress = [NSProgress currentProgress];
  PASS(currentProgress == progress, "Correct progress object associated with current thread");
  
  NSProgress *new_progress = [NSProgress progressWithTotalUnitCount:100
                                                             parent:progress
                                                   pendingUnitCount:50];
  [new_progress addChild:[[NSProgress alloc] initWithParent: nil userInfo: nil]
    withPendingUnitCount:50];
  
  [currentProgress resignCurrent];

  PASS([NSProgress currentProgress] == nil, "Current progress is nil after resign current");

  [arp release]; arp = nil;
  return 0;
}
