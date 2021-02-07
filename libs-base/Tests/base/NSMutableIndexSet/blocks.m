#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSIndexSet.h>
#import <Foundation/NSLock.h>
#include <Testing.h>
int main()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSMutableIndexSet *set = [NSMutableIndexSet indexSetWithIndexesInRange: NSMakeRange(1,5)];
  [set addIndex:20];
  [set addIndex:25];


  START_SET("NSIndexSet Blocks")
# ifndef __has_feature
# define __has_feature(x) 0
# endif
# if __has_feature(blocks)

  NSMutableIndexSet *referenceSet = nil;
  __block NSMutableIndexSet *newSet = [NSMutableIndexSet indexSet];
  __block BOOL didContainWrongIndex = NO;
  __block NSUInteger lastIndex = NSNotFound;
  NSLock *setLock = [NSLock new];
  void(^enumBlock)(NSUInteger,BOOL*) = ^(NSUInteger idx, BOOL*shouldStop){
      [setLock lock];
      lastIndex = idx;
      [newSet addIndex: idx];
      [setLock unlock];
    };

  // Test forward enumeration:
  [set enumerateIndexesUsingBlock: enumBlock];
  PASS((YES == [set isEqual: newSet]),
    "Can enumerate all indices in an index set using a block");
  PASS((25 == lastIndex),
    "Forward enumeration stops at the last index");

  newSet = [NSMutableIndexSet indexSet];
  didContainWrongIndex = NO;
  lastIndex = NSNotFound;

  // Test reverse enumeration:
  [set enumerateIndexesWithOptions: NSEnumerationReverse
                        usingBlock: enumBlock];
  PASS((YES == [set isEqual: newSet]),
    "Can reverse enumerate all indices in an index set using a block");
  PASS((1 == lastIndex),
    "Reverse enumeration stops at the first index");

  referenceSet = [NSMutableIndexSet indexSetWithIndexesInRange: (NSMakeRange(4,2))];
  [referenceSet addIndex: 20];
  newSet = [NSMutableIndexSet indexSet];
  didContainWrongIndex = NO;
  lastIndex = NSNotFound;

  // Test subrange enumeration:
  [set enumerateIndexesInRange: NSMakeRange(4,20)
                       options: 0
                    usingBlock: enumBlock];
   PASS((YES == [referenceSet isEqual: newSet]), "Can enumerate subranges of an index set");
   PASS((20 == lastIndex), "Subrange enumeration stops at the correct index");

   newSet = [NSMutableIndexSet indexSet];
   lastIndex = NSNotFound;
   referenceSet = [NSMutableIndexSet indexSetWithIndexesInRange: NSMakeRange(1,5)];
   [referenceSet addIndex: 20];
   enumBlock = ^(NSUInteger idx, BOOL*shouldStop){
       [setLock lock];
       [newSet addIndex: idx];
       [setLock unlock];
       if (20 == idx)
       {
	 *shouldStop = YES;
       }
     };

  // Test premature termination of enumeration:
  [set enumerateIndexesUsingBlock: enumBlock];
  PASS((YES == [referenceSet isEqual: newSet]), "block can prematurely terminate enumeration");

  [setLock release];
# else
  SKIP("No Blocks support in the compiler.")
# endif
  END_SET("NSIndexSet Blocks")
  [pool release];
  return 0;
}
