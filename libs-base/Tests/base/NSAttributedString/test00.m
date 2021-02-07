#import <Foundation/NSString.h>
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSAutoreleasePool.h>
#import "ObjectTesting.h"

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSAttributedString *s;
  NSString *key1, *val1, *str1;
  NSRange r = NSMakeRange(0,6);
  NSAttributedString *astr1, *astr2;
  NSDictionary *dict1;
  NSRange range = NSMakeRange(0,0);
  id obj;

  s = [[[NSAttributedString alloc] initWithString: @"string"] autorelease];
  PASS_EQUAL([s string], @"string", "equality OK for string value");
  PASS([s length] == 6, "length reported correctly");
  PASS_EQUAL([s attributesAtIndex: 0 effectiveRange: NULL], 
    [NSDictionary dictionary], "returnsempty attributes dictionary, not nil");

  key1 = @"Helvetica 12-point";
  val1 = @"NSFontAttributeName";
  str1 = @"Attributed string test";
  dict1 = [NSDictionary dictionaryWithObject:val1 forKey:key1];

  astr1 = [[NSAttributedString alloc] initWithString: str1 attributes: dict1];
  PASS(astr1 != nil && [astr1 isKindOfClass: [NSAttributedString class]] && 
       [[astr1 string] isEqual: str1],"-initWithString:attributes: works");
  
  obj = [astr1 attributesAtIndex: 0 effectiveRange: &range];
  PASS(obj != nil && [obj isKindOfClass: [NSDictionary class]] && 
       [obj count] == 1 && range.length != 0,
       "-attributesAtIndex:effectiveRange: works");
    
  obj = [astr1 attribute: key1 atIndex: 0 effectiveRange: &range];
  PASS(obj != nil && [obj isEqual: val1] && range.length != 0,
       "-attribute:atIndex:effectiveRange: works");
  obj = [astr1 attributedSubstringFromRange: r];
  PASS(obj != nil && [obj isKindOfClass: [NSAttributedString class]] &&
       [obj length] == r.length,"-attributedSubstringFromRange works");

  r = NSMakeRange(0,[astr1 length]);
  astr2 = [astr1 attributedSubstringFromRange: r];
  PASS(astr2 != nil && [astr1 isEqualToAttributedString: astr2],
       "extract and compare using -isEqualToAttributedString works");

  [arp release]; arp = nil;
  return 0;
}

