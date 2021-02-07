/*
   Copyright (C) 2008 Free SoftwareFoundation, Inc.

*/

/*
Testing of Various Byte Order conversion.
*/

#import "Testing.h"
#import <Foundation/Foundation.h>

int main(int argc, char **argv)
{
  NSAutoreleasePool *pool = [NSAutoreleasePool new];
  NSString *s1 = @"A";
  NSString *s2;
  NSData *d;
  const uint8_t *b;

  d = [s1 dataUsingEncoding: NSUTF16BigEndianStringEncoding];
  b = [d bytes];
  PASS([d length] > 1 && b[0] == 0 && b[1] == 65, "UTF-16 BE OK");
  if (testPassed)
    {
      s2 = [[NSString alloc] initWithBytes: b
				    length: 2
			          encoding: NSUTF16BigEndianStringEncoding];
      PASS([s1 isEqual: s2], "UTF-16 BE reverse OK");
      [s2 release];
    }

  d = [s1 dataUsingEncoding: NSUTF16LittleEndianStringEncoding];
  b = [d bytes];
  PASS([d length] > 1 && b[0] == 65 && b[1] == 0, "UTF-16 LE OK");
  if (testPassed)
    {
      s2 = [[NSString alloc] initWithBytes: b
				    length: 2
				  encoding: NSUTF16LittleEndianStringEncoding];
      PASS([s1 isEqual: s2], "UTF-16 LE reverse OK");
      [s2 release];
    }

  d = [s1 dataUsingEncoding: NSUTF32BigEndianStringEncoding];
  b = [d bytes];
  PASS([d length] > 3 && b[0] == 0 && b[1] == 0 && b[2] == 0 && b[3] == 65,
    "UTF-32 BE OK");
  if (testPassed)
    {
      s2 = [[NSString alloc] initWithBytes: b
				    length: 4
				  encoding: NSUTF32BigEndianStringEncoding];
      PASS([s1 isEqual: s2], "UTF-32 BE reverse OK");
      [s2 release];
    }

  d = [s1 dataUsingEncoding: NSUTF32LittleEndianStringEncoding];
  b = [d bytes];
  PASS([d length] > 3 && b[0] == 65 && b[1] == 0 && b[2] == 0 && b[3] == 0,
    "UTF-32 LE OK");
  if (testPassed)
    {
      s2 = [[NSString alloc] initWithBytes: b
				    length: 4
				  encoding: NSUTF32LittleEndianStringEncoding];
      PASS([s1 isEqual: s2], "UTF-32 LE reverse OK");
      [s2 release];
    }

  [pool release];
  return 0;
}

