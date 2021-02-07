#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSValue.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSNumber *testObj;

  test_alloc_only(@"NSNumber");
  testObj = [NSNumber numberWithInt: 5];
  test_NSObject(@"NSNumber", [NSArray arrayWithObject:testObj]);
  test_NSCoding([NSArray arrayWithObject:testObj]);
  test_NSCopying(@"NSNumber", @"NSNumber", 
       [NSArray arrayWithObject:testObj],YES,NO);
  {
     // For more paranoid testing, you can change this to int or long long.
     short i = 0;
     int increment = 1;
     while (i >= 0)
     {
       NSNumber *n = [[NSNumber alloc] initWithInt: i];
       PASS(i == [n intValue], "int -> NSNumber -> int is identity operation for %d", i);
       [n release];
       i += increment;
       if (i > 100) { increment++; }
     }
     i = -1;
     increment = 1;
     while (i <= 0)
       {
         NSNumber *n = [[NSNumber alloc] initWithInt: i];
         PASS(i == [n intValue], "int -> NSNumber -> int is identity operation for %d", i);
         [n release];
         i -= increment;
         if (i < -100) { increment++; }
       }
  }
  [arp release]; arp = nil;
  return 0;
}
