#import "ObjectTesting.h"
#import <Foundation/NSSet.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSCountedSet *count1 = [NSCountedSet setWithObjects: @"1", @"1", nil];
  NSCountedSet *count2 = [NSCountedSet setWithObjects: @"1", @"1", nil];
  NSCountedSet *count3 = [NSCountedSet setWithObjects: @"1", nil];
  NSSet *set = [NSSet setWithObjects: @"1", nil];
  PASS([count1 isEqualToSet: count2], "Identical counted sets are equal");
  PASS(![count1 isEqualToSet: count3], "Different counted sets are not equal");
  PASS([count3 isEqualToSet: set], "Counted set is equal to plain set");
  PASS([set isEqualToSet: count3], "Plain set is equal to counted set");
  PASS(![count1 isEqualToSet: set], "Counted set with different counts is not equal to plain set");
  PASS(![set isEqualToSet: count1], "Plain set is not equal to counted set with different counts");
  [arp release]; arp = nil;
  return 0;
}

