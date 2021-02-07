/* Test for unicode noncharacter codepoints
 */
#import "Testing.h"

#import <Foundation/NSString.h>

int main(int argc, char **argv)
{
  NSString *str;
  unichar u;
  unichar u2[2];
  u = (unichar)0xfdd0;
  str = [[NSString alloc] initWithCharacters: &u length: 1];
  PASS([str length] == 1, "fdd0 codepoint is permitted in string");
  PASS([str characterAtIndex: 0] == 0xfdd0, "fdd0 is returned properly");
  [str release];

  u = (unichar)0xfdef;
  str = [[NSString alloc] initWithCharacters: &u length: 1];
  PASS([str length] == 1, "fdef codepoint is permitted in string");
  PASS([str characterAtIndex: 0] == 0xfdef, "fdef is returned properly");
  [str release];

  u = (unichar)0xfffd;
  str = [[NSString alloc] initWithCharacters: &u length: 1];
  PASS([str length] == 1, "fffd codepoint is permitted in string");
  PASS([str characterAtIndex: 0] == 0xfffd, "fffd is returned properly");
  [str release];
  /* eth, so that we don't have the BOM as the first character (it would be
   * removed) */
  u2[0] = (unichar)0x00f0;
  /* BOM as second non-character codepoint should be allowed */
  u2[1] = (unichar)0xfffe;
  str = [[NSString alloc] initWithCharacters: u2 length: 2];
  PASS([str length] == 2, "fffe codepoint is permitted in string");
  PASS([str characterAtIndex: 1] == 0xfffe, "fffe is returned properly");
  [str release];

  u = (unichar)0xffff;
  str = [[NSString alloc] initWithCharacters: &u length: 1];
  PASS([str length] == 1, "ffff codepoint is permitted in string");
  PASS([str characterAtIndex: 0] == 0xffff, "ffff is returned properly");
  [str release];

  return 0;
}

