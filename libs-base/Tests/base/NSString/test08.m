#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>


@interface NSString (NumericSort)
- (NSComparisonResult)numericCompare: (NSString *)s;
@end

@implementation NSString (NumericSort)
- (NSComparisonResult)numericCompare: (NSString *)s
{
  return [self compare: s options: NSNumericSearch];
}
@end


int main (int argc, const char * argv[])
{
  NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
  NSArray *a1, *a2;
  NSArray *a;
  NSString *s1, *s2;
  
  s1 = @"1";
  s2 = @"2";
  PASS([s1 compare: s2 options: NSNumericSearch] == NSOrderedAscending,
    "1 is less than 2");

  s1 = @"1";
  s2 = @"10";
  PASS([s1 compare: s2 options: NSNumericSearch] == NSOrderedAscending,
    "1 is less than 10");

  s1 = @"1";
  s2 = @"11";
  PASS([s1 compare: s2 options: NSNumericSearch] == NSOrderedAscending,
    "1 is less than 11");
  
  s1 = @"11";
  s2 = @"1";
  PASS([s1 compare: s2 options: NSNumericSearch] == NSOrderedDescending,
    "11 is greater than 1");
  
  s1 = @"11";
  s2 = @"2";
  PASS([s1 compare: s2 options: NSNumericSearch] == NSOrderedDescending,
    "11 is greater than 2");
  
  a1 = [[NSArray alloc] initWithObjects:
    @"2", @"1", @"10", @"11", @"20", @"3", nil];

  a = [[NSArray alloc] initWithObjects:
    @"1", @"10", @"11", @"2", @"20", @"3", nil];
  a2 = [a1 sortedArrayUsingSelector: @selector(compare:)];
  PASS_EQUAL(a2, a, "text sort");

  a = [[NSArray alloc] initWithObjects:
    @"1", @"2", @"3", @"10", @"11", @"20", nil];
  a2 = [a1 sortedArrayUsingSelector: @selector(numericCompare:)];
  PASS_EQUAL(a2, a, "numeric sort");
  
  [pool drain];
  return 0;
}
