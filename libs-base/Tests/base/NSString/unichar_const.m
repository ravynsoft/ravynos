#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  NSString		*a = @"a";
  NSString              *alpha = @"α"; // @"\u03b1";
  NSString              *rightarrow = @"→"; // @"\u2192";
  NSString              *smiley = @"😀"; // @"\U0001f600";

  PASS([a length] == 1, "'a' is one character.")
  PASS([alpha length] == 1, "alpha is one character.")
  PASS([rightarrow length] == 1, "rightarrow is one character.")
  PASS([smiley length] == 2, "smiley is a surrogate pair.")

  PASS(strcmp([a UTF8String], "a") == 0, "UTF8 encoding for 'a'.")
  PASS(strcmp([alpha UTF8String], "\xce\xb1") == 0, "UTF8 encoding for alpha.")
  PASS(strcmp([rightarrow UTF8String], "\xe2\x86\x92") == 0,
    "UTF8 encoding for rightarrow.")
  PASS(strcmp([smiley UTF8String], "\xf0\x9f\x98\x80") == 0,
    "UTF8 encoding for smiley.")

  PASS([a canBeConvertedToEncoding: NSISOLatin1StringEncoding] == YES,
    "'a' can be converted to ISO Latin-1 encoding.")
  PASS([alpha canBeConvertedToEncoding: NSISOLatin1StringEncoding] == NO,
    "alpha cannot be converted to ISO Latin-1 encoding.")
  PASS([rightarrow canBeConvertedToEncoding: NSISOLatin1StringEncoding] == NO,
    "rightarrow cannot be converted to ISO Latin-1 encoding.")
  PASS([smiley canBeConvertedToEncoding: NSISOLatin1StringEncoding] == NO,
    "smiley cannot be converted to ISO Latin-1 encoding.")

  NSMutableString *mutable;
  mutable = [NSMutableString string]; [mutable setString: a];
  PASS_EQUAL(mutable, a, "assigning 'a' to a mutable string works.")
  mutable = [NSMutableString string]; [mutable setString: alpha];
  PASS_EQUAL(mutable, alpha, "assigning alpha to a mutable string works.")
  mutable = [NSMutableString string]; [mutable setString: rightarrow];
  PASS_EQUAL(mutable, rightarrow,
    "assigning rightarrow to a mutable string works.")
  mutable = [NSMutableString string]; [mutable setString: smiley];
  PASS_EQUAL(mutable, smiley, "assigning smiley to a mutable string works.")
  [arp release];
  return 0;
}
