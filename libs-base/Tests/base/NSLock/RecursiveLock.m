#import <Foundation/Foundation.h>
#import "Testing.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  BOOL ret;
  NSLock *lock = [NSRecursiveLock new];

  ret = [lock tryLock];
  if (ret)
    [lock unlock];
  PASS(ret, "NSRecursiveLock with tryLock, then unlocking");
  
  ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow:1]];
  if (ret)
    [lock unlock];
  PASS(ret, "NSRecursiveLock lockBeforeDate: works");
  
  ret = [lock tryLock];
  if (ret)
    {
      ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow:1]];
      if (ret)
        {
          [lock unlock];
        }
      [lock unlock];
    }
  PASS(ret, "NSRecursiveLock lockBeforeDate: with NSRecursiveLock returns YES");

#if     defined(GNUSTEP_BASE_LIBRARY)
  START_SET("mutex ownership extension")
#if     defined(_WIN32)
  SKIP("mutex ownership feature not available on windows")
#endif
  NS_DURING
    {
      testHopeful = YES;
      PASS([lock isLockedByCurrentThread] == NO,
        "NSRecursiveLock isLockedByCurrentThread returns NO when not locked");
      [lock lock];
      PASS([lock isLockedByCurrentThread] == YES,
        "NSRecursiveLock isLockedByCurrentThread returns YES when locked");
      [lock unlock];
      PASS([lock isLockedByCurrentThread] == NO,
        "NSRecursiveLock isLockedByCurrentThread returns NO when unlocked");
      testHopeful = NO;
    }
  NS_HANDLER
    {
      NSLog(@"-isLockedByCurrentThread not supported");
    }
  NS_ENDHANDLER
  END_SET("mutex ownership extension")
#endif

  [arp release]; arp = nil;
  return 0;
}

