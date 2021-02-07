#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSTimeZone.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id current;
  id localh;

  current = [NSTimeZone defaultTimeZone];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+defaultTimeZone works");

  current = [NSTimeZone localTimeZone];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+localTimeZone works");

  current = [NSTimeZone systemTimeZone];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+systemTimeZone works");

  current = [NSTimeZone timeZoneForSecondsFromGMT: 900];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+timeZoneForSecondsFromGMT works");

  current = [NSTimeZone timeZoneForSecondsFromGMT: 90000];
  PASS(current == nil,
       "+timeZoneForSecondsFromGMT fails for bad offset");

  current = [NSTimeZone timeZoneWithAbbreviation: @"MST"];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+timeZoneWithAbbreviation works");

  current = [NSTimeZone timeZoneWithName: @"GB"];
  PASS(current != nil && [current isKindOfClass: [NSTimeZone class]],
       "+timeZoneWithName works");

  [arp release]; arp = nil;
  return 0;
}
