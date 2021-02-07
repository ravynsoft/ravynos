#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>
#import <Foundation/NSData.h>
#import <Foundation/NSPropertyList.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *bstr = @"\"Hello---@?pP-l\\U00a6&\\U00e4\\U00a6\"";
  NSData   *adat;
  NSString *cstr;

  /* Prepare UTF-8 string from plist style c-string.  */
  bstr = [bstr propertyList];
  adat = [bstr dataUsingEncoding: NSUTF8StringEncoding];

  PASS((adat != nil && [adat isKindOfClass: [NSData class]]),
	 "We can convert from UTF8 Encoding");

  cstr = [[NSString alloc] initWithData: adat encoding: NSUTF8StringEncoding];
  PASS((cstr != nil && [cstr isKindOfClass: [NSString class]]),
       "We can convert to UTF8 Encoding");

  [arp release]; arp = nil;
  return 0;
}

