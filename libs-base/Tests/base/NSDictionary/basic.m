#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableArray *testObjs = [NSMutableArray new];
  NSDictionary *obj;
  test_alloc(@"NSDictionary");
  obj = [NSDictionary new];
  [testObjs addObject:obj];
  PASS(obj != nil && 
       [obj isKindOfClass:[NSDictionary class]] &&
       [obj count] == 0,
       "can create an empty dictionary");
  obj = [NSDictionary dictionaryWithObject:@"Hello" forKey:@"Key"];
  [testObjs addObject:obj];
  PASS(obj != nil && 
       [obj isKindOfClass:[NSDictionary class]] &&
       [obj count] == 1, 
       "can create a dictionary with one element");
  
  test_NSObject(@"NSDictionary", testObjs);
  test_NSCoding(testObjs);
  test_keyed_NSCoding(testObjs);
  test_NSCopying(@"NSDictionary", @"NSMutableDictionary", testObjs, YES, NO);
  test_NSMutableCopying(@"NSDictionary", @"NSMutableDictionary", testObjs);
  START_SET("NSDictionary subscripting")
# ifndef __has_feature
# define __has_feature(x) 0
# endif
#if __has_feature(objc_subscripting)
  NSDictionary *dictionary = @{@123 : @123.4 ,
                               @"date" : @"today" };
  PASS([dictionary[@123] isEqual: @123.4], "Dictionary subscripting works");
# else
   SKIP("No dictionary subscripting support in the compiler.")
# endif
  END_SET("NSDictionary subscripting")


  [arp release]; arp = nil;
  return 0;
}
