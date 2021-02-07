#import <Foundation/Foundation.h>
#import "Testing.h"
#import "ObjectTesting.h"


int main()
{
  START_SET("URL character sets")
  NSMutableString       *m = [NSMutableString string];
  NSDictionary          *sets;
  NSEnumerator          *enumerator;
  NSString              *key;
  
  sets = [NSDictionary dictionaryWithObjectsAndKeys:
    @"!$&'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~",
    @"URLFragmentAllowedCharacterSet",
    @"!$&'()*+,-.0123456789:;=ABCDEFGHIJKLMNOPQRSTUVWXYZ[]_abcdefghijklmnopqrstuvwxyz~",
    @"URLHostAllowedCharacterSet",
    @"!$&'()*+,-.0123456789;=ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~",
    @"URLPasswordAllowedCharacterSet",
    @"!$&'()*+,-./0123456789:=@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~",
    @"URLPathAllowedCharacterSet",
    @"!$&'()*+,-./0123456789:;=?@ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~",
    @"URLQueryAllowedCharacterSet",
    @"!$&'()*+,-.0123456789;=ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~",
    @"URLUserAllowedCharacterSet",
    nil];
  enumerator = [sets keyEnumerator];
  while (nil != (key = [enumerator nextObject]))
    {
      NSString          *expect = [sets objectForKey: key];
      const char        *name = [key UTF8String];
      NSCharacterSet    *set;
      unichar           c;

      set = (NSCharacterSet*)[[NSCharacterSet class] performSelector:
        NSSelectorFromString(key)];
      [m setString: @""];
      for (c = 0; c < 128; c++)
        {
          if ([set characterIsMember: c])
            {
              NSString  *s;

              s = [[NSString alloc] initWithCharacters: &c length: 1];
              [m appendString: s];
              RELEASE(s);
            }
        }
      PASS_EQUAL(m, expect, "%s as string", name)
    }
  
  END_SET("URL character sets")
  return 0;
}
