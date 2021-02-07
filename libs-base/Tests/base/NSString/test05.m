#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *theString;
  unichar theUniChar[1] = {0xe5};
  theString = [NSString stringWithCharacters:theUniChar length:1];

  PASS([theString isEqual:[[NSString alloc] initWithCString: [theString cStringUsingEncoding: NSISOLatin1StringEncoding] encoding: NSISOLatin1StringEncoding]],"foo");

  NS_DURING
    PASS([theString isEqual:[[NSString alloc] initWithCString: [theString cString]]],"foo");
  NS_HANDLER
    PASS(1,"bar");
  NS_ENDHANDLER
  
  NS_DURING
    PASS([theString isEqual:[[NSMutableString alloc] initWithCString: [theString cString]]],"foo2");
  NS_HANDLER
    PASS(1,"bar2");
  NS_ENDHANDLER
  
  [arp release]; arp = nil;
  return 0;
}
