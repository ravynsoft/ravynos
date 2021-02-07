#import <Foundation/Foundation.h>
#import "ObjectTesting.h"

int
main()
{
  NSGarbageCollector	*collector;
  NSZone		*z;

  collector = [NSGarbageCollector defaultCollector];
  if (collector == nil) return 0;

  START_SET("Garbage collection");

  PASS([collector zone] == NSDefaultMallocZone(),
    "collector zone is default")
  PASS([[NSObject new] zone] == NSDefaultMallocZone(),
    "object zone is default")
  PASS((z = NSCreateZone(1024, 128, YES)) == NSDefaultMallocZone(),
    "created zone is default")
  PASS((z = NSCreateZone(1024, 128, YES)) == NSDefaultMallocZone(),
    "created zone is default")
  PASS_RUNS(NSRecycleZone(z), "zone recycling works")

  END_SET("Garbage collection");

  return 0;
}
