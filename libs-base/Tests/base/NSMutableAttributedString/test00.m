#import "Testing.h"
#import <Foundation/NSAttributedString.h>
#import <Foundation/NSAutoreleasePool.h>

@interface NSMutableAttributedString(evil)
-(void) _sanity;
@end
#if     !defined(GNUSTEP_BASE_LIBRARY)
@implementation NSMutableAttributedString(evil)
- (void) _sanity
{
}
@end
#endif

@interface NSColor : NSObject
+ (id) redColor;
+ (id) blueColor;
@end

@implementation NSColor
+ (id) redColor;
{
  return [[self new] autorelease];
}
+ (id) blueColor;
{
  return [[self new] autorelease];
}
@end

NSString *NSForegroundColorAttributeName = @"NSForegroundColorAttributeName";


@interface NSMutableAttributedString (TestingAdditions)
-(BOOL)checkAttributes: (NSDictionary *)attr location: (int)location;
-(BOOL)checkAttributes: (NSDictionary *)attr range: (NSRange)range;
@end
@implementation NSMutableAttributedString (TestingAdditions)
-(BOOL) checkAttributes: (NSDictionary *)attr location: (int)loc
{  
  return [[self attributesAtIndex:loc
               effectiveRange:NULL] isEqual:attr];
}

-(BOOL) checkAttributes: (NSDictionary *)attr range: (NSRange)range
{
  NSRange aRange = range;
  
  while (aRange.length > 0)
    {
      BOOL attrEqual;
      attrEqual= [[self attributesAtIndex: aRange.location + (aRange.length - 1)
                           effectiveRange: NULL] isEqual: attr];
      if (attrEqual == NO)
        return NO;
      
      aRange.length -= 1;
    }
  return YES;
}
@end

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSMutableAttributedString *attrStr;
  NSString *baseString = @"0123456789";
  NSDictionary *red, *gray, *blue;
  
  NSMutableAttributedString *s;
  s = [[[NSMutableAttributedString alloc]
    initWithString: @"string"] autorelease];
  [s _sanity];
  PASS_EQUAL([s string], @"string", "equality OK for string value");
  PASS([s length] == 6, "length reported correctly");
  PASS_EQUAL([s attributesAtIndex: 0 effectiveRange: NULL], 
    [NSDictionary dictionary], "empty range has empty attributes dictionary");
  [s setAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
    [NSColor redColor], NSForegroundColorAttributeName, nil]
    range: NSMakeRange(0, 3)];
  [s _sanity];
  PASS([[s attributesAtIndex: 0 effectiveRange: NULL] count] == 1,
    "newly set attribute dictionary contains one attribute");
  PASS([[s attributesAtIndex: 3 effectiveRange: NULL] count] == 0,
    "attribute dictionary at 3 contains no attributes");
  [s setAttributes: [NSDictionary dictionaryWithObjectsAndKeys:
    [NSColor blueColor], NSForegroundColorAttributeName, nil]
    range: NSMakeRange(3, 3)];
  [s _sanity];
  PASS([[s attributesAtIndex: 0 effectiveRange: NULL] count] == 1,
    "attribute count at 0 unchanged");
  PASS([[s attributesAtIndex: 3 effectiveRange: NULL] count] == 1,
    "new attribute count is 1");

  red = [NSDictionary dictionaryWithObject:@"Red" forKey:@"Color"];
  gray = [NSDictionary dictionaryWithObject:@"Gray" forKey:@"Color"];
  blue = [NSDictionary dictionaryWithObject:@"Blue" forKey:@"Color"];
  
  attrStr = [[NSMutableAttributedString alloc] initWithString:baseString 
                                                      attributes:red];
  [attrStr _sanity];
  PASS([[attrStr string] isEqual:baseString] &&
       [attrStr checkAttributes:red range:NSMakeRange(0,10)],
       "-initWithString:attributes: works");
  
  [attrStr setAttributes:blue range:NSMakeRange(0,10)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:blue range:NSMakeRange(0,10)],
       "-setAttributes:range: works for the whole string");
   
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(0,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:blue range:NSMakeRange(0,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)],
       "-setAttributes:range: works for the first half of the string");
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(3,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,3)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(3,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(8,2)],
       "-setAttributes:range: works for the middle of the string");
  
  [attrStr setAttributes:blue range:NSMakeRange(4,3)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,3)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(3,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(8,2)],
       "-setAttributes:range: works for same attributes in middle of string");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(5,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,5)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(5,5)],
       "-setAttributes:range: works for the last half of the string");
   
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(0,3)];
  [attrStr _sanity];
  [attrStr setAttributes:red range:NSMakeRange(3,4)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(7,3)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:blue range:NSMakeRange(0,3)] &&
       [attrStr checkAttributes:red range:NSMakeRange(3,4)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(7,3)],
       "-setAttributes:range: works in three parts of the string");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(0,5)];
  [attrStr _sanity];
  [attrStr setAttributes:red range:NSMakeRange(3,5)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(4,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:blue range:NSMakeRange(0,3)] &&
       [attrStr checkAttributes:red range:NSMakeRange(3,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(4,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(9,1)],
       "-setAttributes:range: works with overlapping");
   
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,2)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(4,2)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(7,2)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,6)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(1,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(2,6)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(8,1)] &&
       [attrStr checkAttributes:red range:NSMakeRange(9,1)], 
       "-setAttributes:range: works with overlapping (2)");
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,5)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,2)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(2,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(7,3)], 
       "-setAttributes:range: works with replacing");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,8)];
  [attrStr _sanity];
  [attrStr setAttributes:red range:NSMakeRange(2,6)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(3,4)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(1,1)] &&
       [attrStr checkAttributes:red range:NSMakeRange(2,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(3,4)] &&
       [attrStr checkAttributes:red range:NSMakeRange(7,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(8,1)] &&
       [attrStr checkAttributes:red range:NSMakeRange(9,1)], 
       "-setAttributes:range: works with chinese boxes"); 
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,4)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(1,4)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)],
       "-setAttributes:range: works with extending at the end (diff color)");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,4)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(1,4)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)],
       "-setAttributes:range: works with extending at the end (diff color)");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,4)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(1,4)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)], 
       "-setAttributes:range: works with extending at the beginning (diff color)");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,4)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(1,4)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)], 
       "-setAttributes:range: works with extending at the beginning (same color)");

  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,2)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(1,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(2,2)] &&
       [attrStr checkAttributes:red range:NSMakeRange(4,6)], 
       "-setAttributes:range: works with subset at the end (diff color)"); 
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(1,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,2)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(1,3)] &&
       [attrStr checkAttributes:red range:NSMakeRange(4,6)], 
       "-setAttributes:range: works with subset at the end (same color)");
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,2)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,2)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(2,2)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(4,1)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)], 
       "-setAttributes:range: works with subset at the beginning (diff color)");

  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,3)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,2)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,2)] &&
       [attrStr checkAttributes:gray range:NSMakeRange(2,3)] &&
       [attrStr checkAttributes:red range:NSMakeRange(5,5)], 
       "-setAttributes:range: works with subset at the beginning (same color)");
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(2,1)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(4,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(1,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(6,4)], 
       "-setAttributes:range: works with subsets (diff color)");     
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(4,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,5)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:red range:NSMakeRange(0,1)] &&
       [attrStr checkAttributes:blue range:NSMakeRange(1,5)] &&
       [attrStr checkAttributes:red range:NSMakeRange(6,4)], 
       "-setAttributes:range: works with subsets (same color)");     
  
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(4,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(7,2)];
  [attrStr _sanity];
  [attrStr setAttributes:red range:NSMakeRange(3,2)];
  [attrStr _sanity];
  [attrStr setAttributes:gray range:NSMakeRange(0,10)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:gray range:NSMakeRange(0,10)], 
       "-setAttributes:range: works with setting attributes for the whole string"); 
   
  ASSIGN(attrStr,[[NSMutableAttributedString alloc] initWithString:baseString 
                                                        attributes:red]);
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(0,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(1,1)];
  [attrStr _sanity];
  [attrStr setAttributes:blue range:NSMakeRange(2,1)];
  [attrStr _sanity];
  PASS([attrStr checkAttributes:blue range:NSMakeRange(0,3)] && 
       [attrStr checkAttributes:red range:NSMakeRange(3,7)], 
       "-setAttributes:range: works with nearby attributes"); 
  
  [arp release]; arp = nil;
  return 0;
}

