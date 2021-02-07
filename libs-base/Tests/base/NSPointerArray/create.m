#import "ObjectTesting.h"
#import <Foundation/NSPointerArray.h>
#import <Foundation/NSAutoreleasePool.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableString *ms;
  NSString *val1, *val2, *val3;
  NSPointerArray *obj, *old;
  NSUInteger rc;
  id vals[3];
  
  val1 = @"Hello";
  val2 = @"Goodbye";
  val3 = @"Testing";
  
  vals[0] = val1;
  vals[1] = val2;
  vals[2] = val3;

  obj = [[NSPointerArray new] autorelease];
  PASS(obj != nil
    && [obj isKindOfClass:[NSPointerArray class]]
    && [obj count] == 0,
    "+new creates an empty pointer array");
  
  [obj addPointer: (void*)@"hello"];
  PASS([obj count] == 1, "+addPointer: increments count");
  [obj addPointer: nil];
  PASS([obj count] == 2, "+addPointer: works with nil");

  [obj insertPointer: (void*)vals[0] atIndex: 0];
  [obj insertPointer: (void*)vals[1] atIndex: 0];
  [obj insertPointer: (void*)vals[2] atIndex: 0];
  PASS([obj count] == 5 && [obj pointerAtIndex: 2] == (void*)vals[0],
    "+insertPointer:atIndex: works");
  
  obj = [NSPointerArray pointerArrayWithWeakObjects];
  ms = [@"hello" mutableCopy];
  rc = [ms retainCount];
  [obj addPointer: ms];
  PASS(rc == [ms retainCount], "array with weak references doesn't retain");

  [arp release]; arp = nil;
  return 0;
} 

