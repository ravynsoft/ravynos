#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCache.h>


@interface TestObject : NSObject <NSDiscardableContent>
{
  BOOL _discarded;
}
@end

@implementation TestObject

- (BOOL)beginContentAccess
{
  return YES;
}

- (void)endContentAccess
{
}

- (void)discardContentIfPossible
{
  _discarded = YES;
}



- (BOOL)isContentDiscarded
{
  return _discarded;
}
@end

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  NSCache		*cache = [[NSCache new] autorelease];

  [cache setName: @"Foo"];
  PASS_EQUAL(@"Foo", [cache name], "Name can be set an accessed");

  [cache setCountLimit: 2];
  PASS(2 == [cache countLimit], "Count limit can be set and accessed");

  [cache setTotalCostLimit: 3];
  PASS(3 == [cache totalCostLimit], "Total cost limit can be set and accessed");

  [cache setObject: @"bar" forKey: @"foo"];
  PASS_EQUAL(@"bar", [cache objectForKey: @"foo"],
    "Cached object can be returned");


  /*
   * NOTE: The following to test sets currently won't work. The only available
   * eviction strategy is to evict under the following conditions:
   *
   * - evictsObjectsWithDiscardedContent is set on the receiver
   * - the cached object implements NSDiscardableContent
   * - the content is actually discarded
   */
  START_SET("count-based eviction")
    testHopeful = YES;
    /* Let's test count based eviction: We add two more items and expect the
     * first one (foo) to be removed because the count limit is two
     */
    [cache setObject: @"baz" forKey: @"bar"];
    NSUInteger i = 0;
    for (i = 0; i < 50; i++)
      {
        /* We need to heat this object in the cache so that the first one
         * becomes elligible for eviction
         */
        [cache objectForKey: @"bar"];
      }
    [cache setObject: @"frubble" forKey: @"baz"];
    PASS_EQUAL(@"frubble", [cache objectForKey: @"baz"],
      "LRU object retained on count overflow");
    PASS_EQUAL(@"baz", [cache objectForKey: @"bar"],
      "second object retained on count overflow");
    PASS(nil == [cache objectForKey: @"foo"], "Oldest object evicted");
  END_SET("count-based eviction")
  [cache removeAllObjects];

  START_SET("cost-based eviction")
    testHopeful = YES;
    [cache setObject: @"bar" forKey: @"foo" cost: 2];
    // This should push out the previous object because the cumulative cost (4)
    // exceeds the limit (3)
    [cache setObject: @"baz" forKey: @"bar" cost: 2];
    PASS_EQUAL(@"baz", [cache objectForKey: @"bar"],
      "LRU  object retained on cost overflow");
    PASS(nil == [cache objectForKey: @"foo"], "Overflowing object evicted");
  END_SET("cost-based eviction")

  [cache removeAllObjects];
  START_SET("eviction of discardable content")
    cache = [[NSCache new] autorelease];
    [cache setCountLimit: 1];
    [cache setEvictsObjectsWithDiscardedContent: YES];
    TestObject *a = [[TestObject new] autorelease];
    TestObject *b = [[TestObject new] autorelease];
    [cache setObject: a forKey: @"foo"];
    [cache setObject: b forKey: @"bar"];
    PASS_EQUAL(b, [cache objectForKey: @"bar"],
      "LRU  object retained on count overflow");
    PASS(nil == [cache objectForKey: @"foo"],
      "Overflowing object evicted on count overflow");

    PASS([a isContentDiscarded],
      "Cache did call -discardContentIfPossible on cached object");
    [cache removeAllObjects];
    [cache setCountLimit: 0];
    [cache setTotalCostLimit: 3];
    a = [[TestObject new] autorelease];
    b = [[TestObject new] autorelease];
    [cache setObject: a forKey: @"foo" cost: 2];
    [cache setObject: b forKey: @"bar" cost: 2];
    PASS_EQUAL(b, [cache objectForKey: @"bar"],
      "LRU  object retained on cost overflow");
    PASS(nil == [cache objectForKey: @"foo"],
      "Overflowing object evicted on cost overflow");
      PASS([a isContentDiscarded],
        "Cache did call -discardContentIfPossible on cached object");
  END_SET("eviction of discardable content")


  [arp release]; arp = nil;
  return 0;
}
