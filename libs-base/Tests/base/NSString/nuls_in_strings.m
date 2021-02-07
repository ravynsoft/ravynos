/*
copyright 2004 Alexander Malmberg <alexander@malmberg.org>

Tests that nul characters are handled correctly in strings and string
constants.
*/

#import "Testing.h"

#import <Foundation/NSString.h>

int main(int argc, char **argv)
{
  NSString *constantString=@"a\0b";
  NSString *normalString;
  unichar characters[3]={'a',0,'b'};
  NSRange r;

  normalString = [[NSString alloc] initWithCharacters: characters length: 3];

  PASS([constantString length] == 3, "nuls in constant strings");
  PASS([normalString length] == 3, "nuls in non-constant strings");
  PASS([constantString hash] == [normalString hash], "hashes match");
  PASS([normalString isEqual: constantString]
    && [constantString isEqual: normalString], "compare as equal");
  r = [normalString rangeOfString: @"\0"];
  PASS(1 == r.length && 1 == r.location, "find nul in string");

  return 0;
}

