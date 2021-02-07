#import "Testing.h"
#import "ObjectTesting.h"
#import <Foundation/Foundation.h>

int
main (int argc, char**argv)
{
  id pool = [NSAutoreleasePool new];
  NSCharacterSet *illegal = [NSCharacterSet illegalCharacterSet];
  NSCharacterSet *legal = [illegal invertedSet];
  NSMutableData *data;
  NSString *s;
  unichar cp;

  PASS([illegal characterIsMember: 0xfffe], "0xfffe is illegal");
  PASS(![legal characterIsMember: 0xfffe], "0xfffe is bnot legal");
  PASS([illegal characterIsMember: 0xffff], "0xffff is illegal");
  PASS(![legal characterIsMember: 0xffff], "0xffff is not legal");
  PASS([illegal characterIsMember: 0xfdd0], "0xfdd0 is illegal");
  PASS(![legal characterIsMember: 0xfdd0], "0xfdd0 is not legal");
  PASS([illegal longCharacterIsMember: 0x0010fffe], "0x0010fffe is illegal");
  PASS(![legal longCharacterIsMember: 0x0010fffe], "0x0010fffe is not legal");

  // Null character
  PASS(![illegal characterIsMember: 0x0000], "0x0000 is not illegal");
  PASS([legal characterIsMember: 0x0000], "0x0000 is legal");
  // First half of surrogate pair
  PASS(![illegal characterIsMember: 0xd800], "0xd800 is not illegal");
  PASS([legal characterIsMember: 0xd800], "0xd800 is legal");
  // Second half of surrogate pair
  PASS(![illegal characterIsMember: 0xdc00], "0xdc00 is not illegal");
  PASS([legal characterIsMember: 0xdc00], "0xdc00 is legal");
  // Private use character in code plane 16
  PASS([illegal longCharacterIsMember: 0x0010fffd], "0x0010fffd illegal");
  PASS(![legal longCharacterIsMember: 0x0010fffd], "0x0010fffd is illegal");

  // Entire UCS-2 set (UTF-16 surrogates start above 0xD800)
  // (still looking for official information on the range of UCS-2 code points,
  //  i.e. whether UCS-4/UCS-2 are actually official code point sets
  //  or whether they are just commonly used terms to differentiate 
  //  the full UCS code point set from it's UTF-16 encoding.)
  data = [NSMutableData dataWithCapacity: sizeof(cp) * 0xD800];
  // Do not start with 0x0000 otherwise a leading BOM could misinterpreted.
  for (cp=0x0001;cp<0xFFFF;cp++)
    {
      /* Skip code points that are reserved for surrogate characters.  */
      if (cp == 0xD800) cp = 0xF900;
      if ([legal characterIsMember:cp])
	{
	  [data appendBytes: &cp length: sizeof(cp)];
	}
    }
  s = [[NSString alloc] initWithData: data encoding: NSUnicodeStringEncoding];
  PASS([s length],"legal UCS-2 set can be represented in an NSString.");
  [s release];

  [pool release];
  return (0);
}
