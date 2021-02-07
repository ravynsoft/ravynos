#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

int
main (int argc, char**argv)
{
  id pool = [NSAutoreleasePool new];
  NSCharacterSet *wsnl = [NSCharacterSet whitespaceAndNewlineCharacterSet];

  PASS([wsnl characterIsMember: 0x000A], "newline 0x000A is wsnl")
  PASS([wsnl characterIsMember: 0x000D], "return 0x00D0 is wsnl")
  PASS([wsnl characterIsMember: 0x0020], "space 0x0020 is wsnl")
  PASS([wsnl characterIsMember: 0x200B], "zero-width-space 0x200B is wsnl")
  PASS([wsnl characterIsMember: 0x202F], "narrow-no-break-space 0x202F is wsnl")
  PASS([wsnl characterIsMember: 0x205F],
    "medium-mathematical-space 0x205F is wsnl")
  PASS([wsnl characterIsMember: 0x3000], "ideographic-space 0x202F is wsnl")

  PASS(![wsnl characterIsMember: 0x0030], "0x0030 is not wsnl")
  [pool release];
  return (0);
}
