#import "ObjectTesting.h"
#import <Foundation/NSArray.h>
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSHost.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  test_NSObject(@"NSHost",[NSArray arrayWithObject:[NSHost currentHost]]); 
  [arp release]; arp = nil;
  return 0;
}
