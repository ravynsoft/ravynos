#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSString              *narrow = @"aaaa"; // fits in a single byte
  NSString              *wide = @"a\u20AC\u20ACa"; // Euro signs, requires UTF-16 storage
  NSString		*narrowNarrowFormat = [NSString stringWithFormat: @"a%@a", narrow];
  NSString		*narrowWideFormat = [NSString stringWithFormat: @"a%@a", wide];
  NSString		*wideNarrowFormat = [NSString stringWithFormat: @"\u20AC%@\u20ac", narrow];
  NSString		*wideWideFormat = [NSString stringWithFormat: @"\u20AC%@\u20ac", wide];
  PASS_EQUAL(narrowNarrowFormat, @"aaaaaa",
             "Formatting byte-width string into a byte-width string works.");
  PASS_EQUAL(narrowWideFormat, @"aa\u20AC\u20ACaa",
             "Formatting a 16 bit wide string into a byte-width string works.");
  PASS_EQUAL(wideNarrowFormat, @"\u20ACaaaa\u20AC",
             "Formatting a byte-width string into a 16 bit wide string works.");
  PASS_EQUAL(wideWideFormat, @"\u20ACa\u20AC\u20ACa\u20AC",
             "Formatting a 16 bit wide string into a 16 bit wide string works.");
  [arp release]; arp = nil;
  return 0;
}
