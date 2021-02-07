#import <Foundation/Foundation.h>
#import "Testing.h"

@interface Queue : NSObject 
{
  NSMutableArray        *queue;
  NSConditionLock       *lock;
}
- (void) enqueue: (id)object;
- (id) dequeue;
@end


@implementation Queue

- (id) init 
{
  if (nil == (self = [super init])) { return nil; }
  queue = [[NSMutableArray alloc] init];
  lock = [[NSConditionLock alloc] initWithCondition: 0];
  return self;
}

- (void) enqueue: (id)object 
{
  [lock lock];
  [queue addObject: object];
  [lock unlockWithCondition: 1];
}

- (id) dequeue
{
  id element;
  NSUInteger count;

  [lock lockWhenCondition: 1];
  element = [[[queue objectAtIndex: 0] retain] autorelease];
  [queue removeObjectAtIndex: 0];
  count = [queue count];
  if (count > 0)
    {
      [lock unlockWithCondition: 1];
    }
  else
    {
      [lock unlockWithCondition: 0];
    }
  return element;
}
@end
NSMutableArray *received;
@interface Consumer : NSObject @end
@implementation Consumer
- (void) consumeFromQueue: (Queue*)q
{
  while (1)
    {
      id p = [NSAutoreleasePool new];

      [received addObject: [q dequeue]];
      [p release];
    }
}
@end

int main(void)
{
  [NSAutoreleasePool new];
  {
    Queue *q = [Queue new];
    NSMutableArray *sent = [NSMutableArray new];
    received = [NSMutableArray new];
    unsigned int i;
    [NSThread detachNewThreadSelector: @selector(consumeFromQueue:)
                 toTarget: [Consumer new]
                 withObject: q];
    for (i = 0; i < 10000; i++)
      {
        id obj = [NSNumber numberWithUnsignedInt: i];
        [sent addObject: obj];
        if (i % 10 == 0)
          {
            [NSThread sleepForTimeInterval: 0.0];
          }
        if (i % 1000 == 0)
          {
            [NSThread sleepForTimeInterval: 1.0];
          }
        [q enqueue: obj];
      }
    [NSThread sleepForTimeInterval: 2.0];
    PASS([sent isEqual: received], "Condition lock");
  }
  return 0;
}
