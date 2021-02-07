#import <Foundation/NSArray.h>
#import <Foundation/NSString.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id val1,val2,val3;
  id ptrvals[3];
  NSArray *obj, *old;
  val1 = @"Tom";
  val2 = @"Petty";
  val3 = @"doesn't want to live like a refugee";

  ptrvals[0] = val1;
  ptrvals[1] = val2;
  ptrvals[2] = val3;
 
  obj = [NSArray new];
  PASS((obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 0),
       "+new creates an empty array");
  [obj release];
  obj = [NSArray array];
  PASS((obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 0),
       "+array creates an empty array");
  PASS_EXCEPTION([NSArray arrayWithObject:nil];, @"NSInvalidArgumentException",
    "+arrayWithObject with nil argument throws exception");
 
  obj = [NSArray arrayWithObject:val1];
  PASS(obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 1,
       "+arrayWithObject: builds a minimal array");
  
  obj = [NSArray arrayWithObjects:ptrvals count:3];
  old = obj;
  PASS(obj != nil && [obj isKindOfClass:[NSArray class]] && [obj count] == 3,
       "+arrayWithObjects: builds an array");

  obj = [NSArray arrayWithArray:old];
  PASS(obj != nil && [old isEqual:obj], "+arrayWithArray: copies array");

  [arp release]; arp = nil;
  return 0;
}

