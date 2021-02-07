#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2, *val3;
  NSMutableArray *obj, *old;
  id vals[3];
  
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  
  vals[0] = val1;
  vals[1] = val2;
  vals[2] = val3;

  obj = [NSMutableArray arrayWithCapacity:10];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj count] == 0,
       "+arrayWithCapacity creates an empty mutable array");
  
  obj = [NSMutableArray array];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj count] == 0,
       "+array creates an empty mutable array");
  
  PASS_EXCEPTION([NSMutableArray arrayWithObject:nil];,
                 NSInvalidArgumentException,
		 "+arrayWithObject: with nil object raises an exception");
  
  obj = [NSMutableArray arrayWithObject:val1];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj count] == 1,
       "+arrayWithObject: builds minimal array");

  obj = [NSMutableArray arrayWithObjects:vals count:3];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj count] == 3,
       "+arrayWithObjects:count: builds an array"); 
  
  obj = [NSMutableArray arrayWithObjects:val1,val2,val3,nil];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj count] == 3,
       "+arrayWithObjects: builds an array");
  
  old = obj;
  obj = [NSMutableArray arrayWithArray:old];
  PASS(obj != nil &&
       [obj isKindOfClass:[NSMutableArray class]] &&
       [obj isEqual:old],
       "+arrayWithArray: copies array");
  
  [arp release]; arp = nil;
  return 0;
} 
