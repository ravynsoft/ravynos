#import <Foundation/NSString.h>
#import <Foundation/NSRegularExpression.h>
#import "ObjectTesting.h"

int main(void)
{
  [NSAutoreleasePool new];
  START_SET("NSString + utf8")

  NSString              *exp;
  NSString              *str;
  uint16_t              uni[2];
  uint8_t               buf[8];

  buf[0] = 0xc0;
  buf[1] = 0x00;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, nil, "bare 0xc0 is illegal")

  buf[0] = 0xc0;
  buf[1] = 0x80;
  buf[2] = 0x00;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, nil, "non-minimal sequence is illegal")

  buf[0] = 0xed;
  buf[1] = 0xa0;
  buf[2] = 0x80;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, nil, "lone high surrogate pair char is illegal")

  buf[0] = 0xed;
  buf[1] = 0xb0;
  buf[2] = 0x80;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, nil, "lone low surrogate pair char is illegal")

  buf[0] = 0xf4;
  buf[1] = 0x90;
  buf[2] = 0x80;
  buf[3] = 0x80;
  buf[4] = 0x00;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, nil, "character too large is illegal")

  uni[0] = 0xdbff;
  uni[1] = 0xdfff;
  exp = [[NSString alloc] initWithCharacters: uni length: 2];
  buf[0] = 0xf4;
  buf[1] = 0x8f;
  buf[2] = 0xbf;
  buf[3] = 0xbf;
  buf[4] = 0x00;
  str = [NSString stringWithUTF8String: buf];
  PASS_EQUAL(str, exp, "maximum unicode character ok")


  END_SET("NSString + utf8")

  return 0;
}
