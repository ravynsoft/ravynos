#import "Testing.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSIndexSet.h>
#import <Foundation/NSString.h>
#import <Foundation/NSEnumerator.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSValue.h>

static NSUInteger fooCount = 0;
static NSUInteger lastIndex = NSNotFound;
static BOOL reverse = NO;
int main()
{
  START_SET("NSArray Blocks")
# ifndef __has_feature
# define __has_feature(x) 0
# endif
# if __has_feature(blocks)
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSLock *lock = [[[NSLock alloc] init] autorelease];
  NSArray *array = [NSArray arrayWithObjects: @"foo", @"bar", @"foo", nil];
  void(^enumBlock)(id,NSUInteger,BOOL*) =  ^(id obj, NSUInteger index, BOOL *stop) {
   [lock lock];
   if ([obj isEqual: @"foo"]) {
      fooCount++;
    }
    if (lastIndex == NSNotFound) {
      lastIndex = index;
    } else {
      lastIndex = reverse ? MIN(lastIndex, index) : MAX(lastIndex, index);
    }
    [lock unlock];
  };
  [array enumerateObjectsUsingBlock: enumBlock];
  PASS((2 == fooCount) && (lastIndex == 2),
       "Can forward enumerate array using a block");
  fooCount = 0;
  lastIndex = NSNotFound;
  [array enumerateObjectsWithOptions: NSEnumerationConcurrent
                          usingBlock: enumBlock];
  PASS((2 == fooCount) && (lastIndex == 2),
       "Can forward enumerate array concurrently using a block");
  fooCount = 0;
  lastIndex = NSNotFound;
  reverse = YES;
  [array enumerateObjectsWithOptions: NSEnumerationReverse
                          usingBlock: enumBlock];
  PASS((0 == lastIndex), "Can enumerate array in reverse using a block");
  reverse = NO;
  fooCount = 0;
  lastIndex = NSNotFound;
  enumBlock = ^(id obj, NSUInteger index, BOOL *stop){if ([obj isEqual: @"foo"]){
    fooCount++;} else if ([obj isEqual: @"bar"]){ *stop=YES;}; lastIndex =
    index;};
  [array enumerateObjectsUsingBlock: enumBlock];
  PASS(((1 == fooCount) && (lastIndex == 1)),
    "Block can stop enumeration prematurely.");

  NSIndexSet *set = [array indexesOfObjectsPassingTest: ^(id obj, NSUInteger index, BOOL* stop){ if ([obj isEqual: @"foo"]) { return YES;} return NO;}];
  PASS(((2 == [set count])
    && (YES == [set containsIndex: 0])
    && (YES == [set containsIndex: 2])
    && (NO == [set containsIndex: 1])),
    "Can select object indices based on block predicate.");
  [arp release]; arp = nil;
  
  array = [NSArray arrayWithObjects:[NSNumber numberWithInteger:2], [NSNumber numberWithInteger:5], [NSNumber numberWithInteger:3], [NSNumber numberWithInteger:2], [NSNumber numberWithInteger:10], nil];
  NSArray *sortedArray = [NSArray arrayWithObjects:[NSNumber numberWithInteger:2], [NSNumber numberWithInteger:2], [NSNumber numberWithInteger:3], [NSNumber numberWithInteger:5], [NSNumber numberWithInteger:10], nil];
  PASS([sortedArray isEqualToArray:[array sortedArrayUsingComparator:^ NSComparisonResult (NSNumber *a, NSNumber *b) { return [a compare:b]; }]], "Can sort arrays with NSComparators.");
  PASS(0 == [sortedArray indexOfObject:[NSNumber numberWithInteger:2] inSortedRange:NSMakeRange(0, [sortedArray count]) options:NSBinarySearchingFirstEqual usingComparator:^ NSComparisonResult (NSNumber *a, NSNumber *b) { return [a compare:b]; }], "Can find index of first object in sorted array");
  PASS(1 == [sortedArray indexOfObject:[NSNumber numberWithInteger:2] inSortedRange:NSMakeRange(0, [sortedArray count]) options:NSBinarySearchingLastEqual usingComparator:^ NSComparisonResult (NSNumber *a, NSNumber *b) { return [a compare:b]; }], "Can find index of first object in sorted array");
  PASS(3 == [sortedArray indexOfObject:[NSNumber numberWithInteger:4] inSortedRange:NSMakeRange(0, [sortedArray count]) options:NSBinarySearchingInsertionIndex usingComparator:^ NSComparisonResult (NSNumber *a, NSNumber *b) { return [a compare:b]; }], "Can find insertion index in sorted array");
  PASS(NSNotFound == [sortedArray indexOfObject:[NSNumber numberWithInteger:4] inSortedRange:NSMakeRange(0, [sortedArray count]) options:0 usingComparator:^ NSComparisonResult (NSNumber *a, NSNumber *b) { return [a compare:b]; }], "Can not find non existant object in sorted array");
# else
  SKIP("No Blocks support in the compiler.")
# endif
  END_SET("NSArray Blocks")
  return 0;
}
