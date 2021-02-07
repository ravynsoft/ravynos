#import "ObjectTesting.h"
#import <Foundation/NSMapTable.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *val1, *val2, *val3;
  NSMapTable *obj;
  id vals[3];
  
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  
  vals[0] = val1;
  vals[1] = val2;
  vals[2] = val3;

  obj = [[NSMapTable new] autorelease];
  PASS(obj != nil
    && [obj isKindOfClass:[NSMapTable class]]
    && [obj count] == 0,
    "+new creates an empty hash table");
  
  [obj setObject: val1 forKey: @"Key1"];
  PASS([obj count] == 1, "-setObject:forKey increments count");
  [obj setObject: nil forKey: @"Key2"];
  PASS([obj count] == 2, "-setObject:forKey: works with nil value");
  PASS_EXCEPTION([obj setObject: val1 forKey: nil];,
    NSInvalidArgumentException, "-setObject:forKey: raises with nil key");

  [arp release]; arp = nil;
  return 0;
} 

