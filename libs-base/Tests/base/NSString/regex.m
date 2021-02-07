#import <Foundation/NSString.h>
#import <Foundation/NSRegularExpression.h>
#import "ObjectTesting.h"

int main(void)
{
  [NSAutoreleasePool new];
  START_SET("NSString + regex")

#if !(__APPLE__ || GS_USE_ICU)
  SKIP("NSRegularExpression not built, please install libicu")
#else
  NSRegularExpression   *expr;
  NSString              *regex;
  NSString              *source;
  NSInteger             index;
  NSRange               r;

  source = @"abcdddddd e f g";

  regex = @"abcd*";
  r = [source rangeOfString: regex options: NSRegularExpressionSearch];
  PASS(r.length == 9, "Correct length for regex, expected 9 got %d",
    (int)r.length);

  regex = @"aBcD*";
  r = [source rangeOfString: regex
    options: (NSRegularExpressionSearch | NSCaseInsensitiveSearch)];
  PASS(r.length == 9, "Correct length for regex, expected 9 got %d",
    (int)r.length);

  source = @"h1. Real Acme\n\n||{noborder}{left}Item||{right}Price||\n|Testproduct|{right}2 x $59.50|\n| |{right}net amount: $100.00|\n| |{right}total amount: $119.00|\n\n\nh2. Thanks for your purchase!\n\n\n";

  expr = [NSRegularExpression regularExpressionWithPattern: @"h[123]\\. "
    options: NSRegularExpressionCaseInsensitive error: NULL];
  index = 33;
       
  NSLog(@"%@", [expr firstMatchInString: source
                   options: NSMatchingAnchored
                     range: NSMakeRange(index, [source length] - index - 1)]);
#endif

  END_SET("NSString + regex")
  return 0;
}
