#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *theString;
  unichar theUniChar[1] = {0xdd00};
  theString = [NSString stringWithCharacters:theUniChar length:1];
  NS_DURING
    PASS(theString == nil, "bad unichar");
  NS_HANDLER
    PASS(1,"bar");
  NS_ENDHANDLER
  
  [arp release]; arp = nil;
  return 0;
}
