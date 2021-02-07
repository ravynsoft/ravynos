#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSCharacterSet *ws = [NSCharacterSet whitespaceAndNewlineCharacterSet];
  PASS([[@"" stringByTrimmingLeadSpaces] isEqual:@""],
       "'' stringByTrimmingLeadSpaces == ''"); 
  PASS([[@"home" stringByTrimmingLeadSpaces] isEqual:@"home"],
       "'home' stringByTrimmingLeadSpaces == 'home'"); 
  PASS([[@" hello" stringByTrimmingLeadSpaces] isEqual:@"hello"],
       "' hello' stringByTrimmingLeadSpaces == 'hello'"); 
  PASS([[@"\thello" stringByTrimmingLeadSpaces] isEqual:@"hello"],
       "'\\thello' stringByTrimmingLeadSpaces == 'hello'"); 
  PASS([[@"\nhello" stringByTrimmingLeadSpaces] isEqual:@"hello"],
       "'\\nhello' stringByTrimmingLeadSpaces == 'hello'"); 
  
  PASS([[@"" stringByTrimmingTailSpaces] isEqual:@""],
       "'' stringByTrimmingTailSpaces == ''"); 
  PASS([[@"home" stringByTrimmingTailSpaces] isEqual:@"home"],
       "'home' stringByTrimmingTailSpaces == 'home'"); 
  PASS([[@"hello " stringByTrimmingTailSpaces] isEqual:@"hello"],
       "'hello ' stringByTrimmingTailSpaces == 'hello'"); 
  PASS([[@"hello\t" stringByTrimmingTailSpaces] isEqual:@"hello"],
       "'hello\\t' stringByTrimmingTailSpaces == 'hello'"); 
  PASS([[@"hello\n" stringByTrimmingTailSpaces] isEqual:@"hello"],
       "'hello\\n' stringByTrimmingTailSpaces == 'hello'"); 
  
  PASS([[@"" stringByTrimmingSpaces] isEqual:@""],
       "'' stringByTrimmingSpaces == ''"); 
  PASS([[@"home" stringByTrimmingSpaces] isEqual:@"home"],
       "'home' stringByTrimmingSpaces == 'home'"); 
  PASS([[@" hello" stringByTrimmingSpaces] isEqual:@"hello"],
       "' hello' stringByTrimmingSpaces == 'hello'"); 
  PASS([[@"\thello" stringByTrimmingSpaces] isEqual:@"hello"],
       "'\\thello' stringByTrimmingSpaces == 'hello'"); 
  PASS([[@"\nhello" stringByTrimmingSpaces] isEqual:@"hello"],
       "'\\nhello' stringByTrimmingSpaces == 'hello'"); 
  
  PASS([[@"home" stringByTrimmingCharactersInSet:ws] isEqual: @"home"],
       "'home' stringByTrimmingCharactersInSet == 'home'"); 
  PASS([[@"hello\n" stringByTrimmingCharactersInSet:ws] isEqual: @"hello"],
       "'hello\\n' stringByTrimmingCharactersInSet == 'hello'"); 
  PASS([[@"\nhello" stringByTrimmingCharactersInSet:ws] isEqual: @"hello"],
       "'\\nhello' stringByTrimmingCharactersInSet == 'hello'"); 
  PASS([[@"\nhello\n" stringByTrimmingCharactersInSet:ws] isEqual: @"hello"],
       "'\nhello\n' stringByTrimmingCharactersInSet == 'hello'"); 
  PASS([[@"\n  \n" stringByTrimmingCharactersInSet:ws] isEqual: @""],
       "'\\n  \\n' stringByTrimmingCharactersInSet == ''"); 
  
  PASS([[@"hello" stringByPaddingToLength:3 
                               withString:@"." 
			  startingAtIndex:0] isEqual:@"hel"],
       "'hello' stringByPaddingToLength:3 withString:'.' startingAtIndex:0 == 'hel'");
  PASS([[@"hello" stringByPaddingToLength:8 
                               withString:@"." 
			  startingAtIndex:0] isEqual:@"hello..."],
       "'hello' stringByPaddingToLength:8 withString:'.' startingAtIndex:0 == 'hello...'");
  PASS([[@"hello" stringByPaddingToLength:8 
                               withString:@"xy" 
			  startingAtIndex:1] isEqual:@"helloyxy"],
       "'hello' stringByPaddingToLength:8 withString:'xy' startingAtIndex:0 == 'helloyxy'");
   
  [arp release]; arp = nil;
  return 0;
}
