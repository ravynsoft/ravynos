#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSFileManager.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  test_NSObject(@"NSFileManager", 
                [NSArray arrayWithObject:[NSFileManager defaultManager]]);
  [arp release]; arp = nil;
  return 0;
}
