
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSLock.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSThread.h>
#import <Foundation/NSUserDefaults.h>

@interface	MyClass : NSObject
+ (void) run;
@end

@implementation	MyClass
+ (void) run
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSConditionLock	*lock = [NSConditionLock new];

  [lock lock];
  [lock lock];
  [arp release];
}
@end

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  
  /* We need the user defaults system set up to allow NSLog to query it
   * when logging the deadlock message, but if the main thread is
   * sleeping then it can't get set up. So we set it up before we 
   * start the test.
   */
  [NSUserDefaults standardUserDefaults];
  [NSThread detachNewThreadSelector: @selector(run)
			   toTarget: [MyClass class]
			 withObject: nil];
  [NSThread sleepForTimeInterval: 1.0];
  NSLog(@"Done.");
  [arp release];
  return 0;
}
