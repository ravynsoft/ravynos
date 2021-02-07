#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <Foundation/NSString.h>

int main()
{
  START_SET("NSDictionary Sorting")
  NSDictionary  *d;
  NSArray	*keysOrderedByKeyedValue;

  NSArray	*values = [NSArray arrayWithObjects:
    [NSNumber numberWithFloat: 2.0],
    [NSNumber numberWithFloat: 1.0],
    [NSNumber numberWithFloat: 3.0],
    [NSNumber numberWithFloat: 4.0],
    nil];

  NSArray	*keys = [NSArray arrayWithObjects:
    @"shouldSortToSecond",
    @"shouldSortToFirst",
    @"shouldSortToThird",
    @"shouldSortToFourth",
    nil];

  NSArray	*expected = [NSArray arrayWithObjects:
    @"shouldSortToFirst",
    @"shouldSortToSecond",
    @"shouldSortToThird",
    @"shouldSortToFourth",
    nil];

  d = [NSDictionary dictionaryWithObjects: values forKeys: keys];
  keysOrderedByKeyedValue = [d keysSortedByValueUsingSelector:
    @selector(compare:)];

  PASS([keysOrderedByKeyedValue isEqual: expected],
    "Can sort a dictionary's keys by its values using a selector");
# ifndef __has_feature
# define __has_feature(x) 0
# endif
# if __has_feature(blocks)
  d = [NSDictionary dictionaryWithObjects: values forKeys: keys];
  keysOrderedByKeyedValue = [d keysSortedByValueUsingComparator:
    ^NSComparisonResult(id obj1, id obj2) {
      return [(NSNumber*)obj1 compare: (NSNumber*)obj2];
    }];

  PASS([keysOrderedByKeyedValue isEqual: expected],
    "Can sort a dictionary's keys by its values using a comparator");
# else
  SKIP("No Blocks support in the compiler.")
# endif

  END_SET("NSDictionary Sorting")

  return 0;
}
