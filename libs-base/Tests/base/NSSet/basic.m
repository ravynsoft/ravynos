#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSSet.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSSet *testObj;
  NSMutableArray *testObjs = [NSMutableArray new];

  testObj = [NSSet new];
  [testObjs addObject:testObj];
  PASS(testObj != nil && [testObj count] == 0,
       "can create an empty set");
   
  testObj = [NSSet setWithObject:@"Hello"];
  [testObjs addObject:testObj];
  PASS(testObj != nil && [testObj count] == 1, "can create a set with one element");
  
  test_NSObject(@"NSSet", testObjs);
  test_NSCoding(testObjs);
  test_keyed_NSCoding(testObjs);
  test_NSCopying(@"NSSet", @"NSMutableSet", testObjs, YES, NO);
  test_NSMutableCopying(@"NSSet", @"NSMutableSet", testObjs);
  

  [arp release]; arp = nil;
  return 0;
}
