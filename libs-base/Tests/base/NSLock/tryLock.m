#import <Foundation/Foundation.h>
#import "Testing.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  BOOL ret;
  id	lock;
  
  lock = [NSLock new];
  ret = [lock tryLock];
  if (ret)
    [lock unlock];
  PASS(ret, "NSLock with tryLock, then unlocking");
 
  ASSIGN(lock,[NSLock new]);
  [lock tryLock];
  ret = [lock tryLock];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive try lock with NSLock should return NO"); 
  
  ASSIGN(lock,[NSConditionLock new]);
  [lock lock];
  ret = [lock tryLock];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive try lock with NSConditionLock should return NO"); 
  
  ret = [lock tryLockWhenCondition: 42];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive tryLockWhenCondition: with NSConditionLock (1) should return NO"); 
  [lock unlockWithCondition: 42];
  [lock lock];
  ret = [lock tryLockWhenCondition: 42];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive tryLockWhenCondition: with NSConditionLock (2) should return NO"); 
  
  ASSIGN(lock,[NSRecursiveLock new]);
  [lock tryLock];
  ret = [lock tryLock];
  if (ret)
    [lock unlock];
  PASS(ret == YES, "Recursive try lock with NSRecursiveLock should return YES"); 
  
  ASSIGN(lock,[NSLock new]);
  ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow: 1]];
  if (ret)
    [lock unlock];
  PASS(ret, "NSLock lockBeforeDate: works");
  
  ASSIGN(lock,[NSLock new]);
  [lock tryLock];
  ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow: 1]];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive lockBeforeDate: with NSLock returns NO");
  
  ASSIGN(lock,[NSConditionLock new]);
  [lock tryLock];
  ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow: 1]];
  if (ret)
    [lock unlock];
  PASS(ret == NO, "Recursive lockBeforeDate: with NSConditionLock returns NO");
  
  ASSIGN(lock,[NSRecursiveLock new]);
  [lock tryLock];
  ret = [lock lockBeforeDate: [NSDate dateWithTimeIntervalSinceNow: 1]];
  if (ret)
    [lock unlock];
  PASS(ret == YES, "Recursive lockBeforeDate: with NSRecursiveLock returns YES");
  
  [arp release]; arp = nil;
  return 0;
}

