#import "ObjectTesting.h"
#import <Foundation/NSHashTable.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2, *val3;
  NSHashTable *obj;
  id vals[3];
  
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  
  vals[0] = val1;
  vals[1] = val2;
  vals[2] = val3;

  obj = [[NSHashTable new] autorelease];
  PASS(obj != nil
    && [obj isKindOfClass:[NSHashTable class]]
    && [obj count] == 0,
    "+new creates an empty hash table");
  
  [obj addObject: (void*)@"hello"];
  PASS([obj count] == 1, "-addObject: increments count");
  PASS_EXCEPTION([obj addObject: nil];,
    NSInvalidArgumentException, "-addObject: raises with nil");

  [arp release]; arp = nil;
  return 0;
} 

