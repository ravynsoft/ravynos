#import <Foundation/NSString.h>
#import <Foundation/NSRegularExpression.h>
#import "ObjectTesting.h"

int main(void)
{
  [NSAutoreleasePool new];
  START_SET("NSString + surrogate pairs")

  NSString              *smiley;
  NSMutableString       *mutable;
  const uint8_t         *utf8;

  smiley = @"😀";

  PASS([smiley length] == 2, "Smiley is a pair of characters")
  PASS([smiley characterAtIndex: 0] == 0xD83D, "Smiley first is 0xD83D")
  PASS([smiley characterAtIndex: 1] == 0xDE00, "Smiley second is 0xDE00")
  utf8 = (const uint8_t *)[smiley UTF8String];
  PASS(strlen(utf8) == 4, "Smiley UTF8 length correct")
  PASS(0xF0 == utf8[0], "Smiley UTF8 first byte OK")
  PASS(0x9F == utf8[1], "Smiley UTF8 second byte OK")
  PASS(0x98 == utf8[2], "Smiley UTF8 third byte OK")
  PASS(0x80 == utf8[3], "Smiley UTF8 fourth byte OK")
  PASS([smiley canBeConvertedToEncoding: NSISOLatin1StringEncoding] == NO,
    "smiley cannot be converted to ISO Latin-1 encoding")

  mutable = AUTORELEASE([smiley mutableCopy]);
  PASS([mutable length] == 2, "Smiley copy is a pair of characters")
  PASS([mutable characterAtIndex: 0] == 0xD83D, "Smiley copy first is 0xD83D")
  PASS([mutable characterAtIndex: 1] == 0xDE00, "Smiley copy second is 0xDE00")
  utf8 = (const uint8_t *)[mutable UTF8String];
  PASS(strlen(utf8) == 4, "Smiley copy UTF8 length correct")
  PASS(0xF0 == utf8[0], "Smiley copy UTF8 first byte OK")
  PASS(0x9F == utf8[1], "Smiley copy UTF8 second byte OK")
  PASS(0x98 == utf8[2], "Smiley copy UTF8 third byte OK")
  PASS(0x80 == utf8[3], "Smiley copy UTF8 fourth byte OK")

  END_SET("NSString + surrogate pairs")

  return 0;
}
