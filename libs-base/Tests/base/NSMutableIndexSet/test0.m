#include <Foundation/NSAutoreleasePool.h>
#include <Foundation/NSIndexSet.h>
#include <Testing.h>
int main()
{
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  NSMutableIndexSet *set = [[NSMutableIndexSet alloc] init];

  [set addIndex:1];
  [set addIndex:2];
  [set addIndex:1];
  PASS([set containsIndex:2], "contains index 2");
  PASS([set containsIndex:1], "contains index 1");
  [set removeIndex:1];
  PASS(![set containsIndex:1], "removed index 1");
  [set removeIndex:2];
  PASS(![set containsIndex:2], "removed index 2");

  [set addIndex:0];
  [set addIndex:2];
  [set shiftIndexesStartingAtIndex:2 by:-1];

  PASS([set containsIndexesInRange:NSMakeRange(0,2)], "contains range");

  [pool release];
  return 0;
}
