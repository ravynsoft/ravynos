#import "ObjectTesting.h"
#import <Foundation/NSDistributedLock.h>
#import <Foundation/NSFileManager.h>

int main()
{
  START_SET("basic")

  NSString              *path;
  NSDistributedLock     *lock1;
  NSDistributedLock     *lock2;
  
  test_NSObject(@"NSDistributedLock",
    [NSArray arrayWithObject: [NSDistributedLock new]]);

  path = [[NSFileManager defaultManager] currentDirectoryPath];
  path = [path stringByAppendingPathComponent: @"MyLock"];
  lock1 = [NSDistributedLock lockWithPath: path];
  lock2 = [NSDistributedLock lockWithPath: path];

  PASS(lock1 != lock2, "locks with the same path differ");

  PASS(YES == [lock1 tryLock], "we can lock the first lock");

  PASS(NO == [lock2 tryLock], "the locks are exclusive");

  [lock1 unlock];
  PASS(YES == [lock2 tryLock], "unlocking first lock allows second lock");
  [lock2 unlock];

  END_SET("basic")
  return 0;
}
