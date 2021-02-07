#import <Foundation/Foundation.h>
#import "ObjectTesting.h"

@interface	MyClass : NSObject
+ (unsigned) finalisationCounter;
+ (unsigned) notificationCounter;
- (void) notified: (NSNotification*)n;
@end

@implementation	MyClass
static unsigned notificationCounter = 0;
static unsigned finalisationCounter = 0;
+ (unsigned) finalisationCounter
{
  return finalisationCounter;
}
+ (unsigned) notificationCounter
{
  return notificationCounter;
}
- (void) finalize
{
  finalisationCounter++;
}
- (void) notified: (NSNotification*)n
{
  notificationCounter++;
}
@end

int
main()
{
  NSGarbageCollector	*collector;
  NSNotificationCenter	*center;
  MyClass		*object;

  collector = [NSGarbageCollector defaultCollector];
  if (collector == nil) return 0;

  START_SET("Garbage Collection");
  center = [NSNotificationCenter defaultCenter];
  object = [MyClass new];
  [center addObserver: object
	     selector: @selector(notified:)
		 name: @"Notification"
	       object: nil];

  [center postNotificationName: @"Notification" object: nil];
  PASS([MyClass notificationCounter] == 1, "simple notification works")
  object = nil;
  [collector collectExhaustively];
  PASS([MyClass finalisationCounter] == 1, "finalisation done")
  [center postNotificationName: @"Notification" object: nil];
  PASS([MyClass notificationCounter] == 1, "automatic removal works")

  END_SET("Garbage Collection");
  return 0;
}
