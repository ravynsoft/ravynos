#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableCharacterSet *testMutableNamedSet, *testMutableNamedSet1, *testMutableNamedSet2;
  NSCharacterSet *testNamedSet;
  testMutableNamedSet = [NSMutableCharacterSet letterCharacterSet];
  testNamedSet = [NSCharacterSet letterCharacterSet];
  [testMutableNamedSet invert];
  PASS([testMutableNamedSet characterIsMember:[@"." characterAtIndex:0]] &&
       ![testNamedSet characterIsMember:[@"." characterAtIndex:0]],
       "Insure defaults set accessors return the correct class");
   
  testMutableNamedSet1 = [NSMutableCharacterSet letterCharacterSet];
  testMutableNamedSet2 = [NSMutableCharacterSet letterCharacterSet];
  [testMutableNamedSet1 invert];
  PASS([testMutableNamedSet1 characterIsMember:[@"." characterAtIndex:0]] &&
       ![testMutableNamedSet2 characterIsMember:[@"." characterAtIndex:0]],
       "Test whether we always get a clean mutable set"); 
  
  [arp release]; arp = nil;
  return 0;
}

