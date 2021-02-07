#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSProxy.h>
#if __has_include(<objc/capabilities.h>)
#include <objc/capabilities.h>
#endif

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  char *prefix = "Class 'NSProxy'";
  Class theClass = Nil;
  id obj0;
  id obj1;
  NSZone *testZone = NSCreateZone(1024,1024,1);

  theClass = [NSProxy class];
  PASS(theClass != Nil, "%s exists",prefix); 
  obj0 = [NSProxy alloc];
  PASS(obj0 != nil, "%s has working alloc",prefix);
  PASS_EXCEPTION([obj0 isKindOfClass:theClass];, NSInvalidArgumentException,
  		 "NSProxy -isKindOfClass raises exception");
  
  PASS_EXCEPTION([obj0 isMemberOfClass:theClass];, 
                 NSInvalidArgumentException,
		 "NSProxy -isKindOfClass raises exception");
  
  obj1 = [NSProxy allocWithZone:testZone];
  PASS(obj1 != nil, "%s has working allocWithZone:",prefix);
#ifndef OBJC_CAP_ARC
  PASS(NSZoneFromPointer(obj1) == testZone, "%s uses zone for alloc",prefix);
  PASS([obj1 zone] == testZone, "%s -zone works",prefix);
#endif
  
  PASS([obj1 hash] != 0, "%s has working -hash",prefix);
  PASS([obj1 isEqual:obj1] == YES, "%s has working -isEqual:",prefix);
  PASS([obj1 class] == theClass, "%s has working -class",prefix);
  
  [arp release]; arp = nil;
  return 0;
}
