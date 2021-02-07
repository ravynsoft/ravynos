#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>
#import <Foundation/NSData.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSCharacterSet *theSet,*iSet;
  NSData *data1 = nil;
  unichar ch;
  theSet = [NSCharacterSet alphanumericCharacterSet];
  PASS([theSet characterIsMember: 'A'] &&
       [theSet characterIsMember: 'Z'] &&
       [theSet characterIsMember: 'a'] &&
       [theSet characterIsMember: 'z'] &&
       [theSet characterIsMember: '9'] &&
       [theSet characterIsMember: '0'] &&
       ![theSet characterIsMember: '#'] &&
       ![theSet characterIsMember: ' '] &&
       ![theSet characterIsMember: '\n'],
       "Check some characters from alphanumericCharacterSet");
  
  theSet = [NSCharacterSet lowercaseLetterCharacterSet];
  PASS(![theSet characterIsMember: 'A'] &&
       ![theSet characterIsMember: 'Z'] &&
       [theSet characterIsMember: 'a'] &&
       [theSet characterIsMember: 'z'] &&
       ![theSet characterIsMember: '9'] &&
       ![theSet characterIsMember: '0'] &&
       ![theSet characterIsMember: '#'] &&
       ![theSet characterIsMember: ' '] &&
       ![theSet characterIsMember: '\n'],
       "Check some characters from lowercaseLetterCharacterSet");
  
  theSet = [NSCharacterSet newlineCharacterSet];
  PASS(![theSet characterIsMember: 'A'] &&
       ![theSet characterIsMember: 'Z'] &&
       ![theSet characterIsMember: 'a'] &&
       ![theSet characterIsMember: 'z'] &&
       ![theSet characterIsMember: '9'] &&
       ![theSet characterIsMember: '0'] &&
       ![theSet characterIsMember: '#'] &&
       ![theSet characterIsMember: ' '] &&
       [theSet characterIsMember: '\n'] &&
       ![theSet characterIsMember: '\t'],
       "Check some characters from newlineCharacterSet");
  
  theSet = [theSet invertedSet];
  PASS([theSet characterIsMember: 'A'] &&
       [theSet characterIsMember: 'Z'] &&
       [theSet characterIsMember: 'a'] &&
       [theSet characterIsMember: 'z'] &&
       [theSet characterIsMember: '9'] &&
       [theSet characterIsMember: '0'] &&
       [theSet characterIsMember: '#'] &&
       [theSet characterIsMember: ' '] &&
       ![theSet characterIsMember: '\n'] &&
       [theSet characterIsMember: '\t'] &&
       [theSet characterIsMember: 0xf6],
       "Check some characters from inverted newlineCharacterSet");

  theSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
  PASS(![theSet characterIsMember: 'A'] &&
       ![theSet characterIsMember: 'Z'] &&
       ![theSet characterIsMember: 'a'] &&
       ![theSet characterIsMember: 'z'] &&
       ![theSet characterIsMember: '9'] &&
       ![theSet characterIsMember: '0'] &&
       ![theSet characterIsMember: '#'] &&
       [theSet characterIsMember: ' '] &&
       [theSet characterIsMember: '\n'] &&
       [theSet characterIsMember: '\t'],
       "Check some characters from whitespaceAndNewlineCharacterSet");
  
  PASS([theSet characterIsMember: 0x00A0], "a non-break-space is whitespace");

  data1 = [theSet bitmapRepresentation];
  PASS(data1 != nil && [data1 isKindOfClass: [NSData class]],
       "-bitmapRepresentation works");
  
  iSet = [theSet invertedSet]; 
  PASS([iSet characterIsMember: 'A'] &&
       [iSet characterIsMember: 'Z'] &&
       [iSet characterIsMember: 'a'] &&
       [iSet characterIsMember: 'z'] &&
       [iSet characterIsMember: '9'] &&
       [iSet characterIsMember: '0'] &&
       [iSet characterIsMember: '#'] &&
       ![iSet characterIsMember: ' '] &&
       ![iSet characterIsMember: '\n'] &&
       ![iSet characterIsMember: '\t'],
       "-invertedSet works");
  {
    NSCharacterSet *firstSet,*secondSet,*thirdSet,*fourthSet;
    firstSet = [NSCharacterSet decimalDigitCharacterSet];
    secondSet = [NSCharacterSet decimalDigitCharacterSet];
    thirdSet = nil;
    fourthSet = [NSMutableCharacterSet decimalDigitCharacterSet];
    thirdSet = [[firstSet class] decimalDigitCharacterSet];
    PASS (firstSet == secondSet && 
          firstSet == thirdSet && 
	  firstSet != fourthSet,
	  "Caching of standard sets");
  }

  theSet = [NSCharacterSet characterSetWithCharactersInString:@"Not a set"];
  PASS(theSet != nil && [theSet isKindOfClass: [NSCharacterSet class]],
       "Create custom set with characterSetWithCharactersInString:");
  
  PASS([theSet characterIsMember: ' '] &&
       [theSet characterIsMember: 'N'] &&
       [theSet characterIsMember: 'o'] &&
       ![theSet characterIsMember: 'A'] &&
       ![theSet characterIsMember: '#'],
       "Check custom set");

  theSet = [NSCharacterSet URLFragmentAllowedCharacterSet];
  NSString *setString = @"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ?/:@-._~!$&'()*+,=";
  NSUInteger i = 0;
  BOOL p = YES;
  for(i = 0; i < [setString length]; i++)
    {
      char c = [setString characterAtIndex: i];
      if([theSet characterIsMember: c] == NO) // , [msg cStringUsingEncoding: NSUTF8Encoding]);
	{
	  NSLog(@"%c is not in set", c);
	  PASS(NO,"URLFragmentAllowedCharacterSet char not found");
	  p = NO;
	}
    }

  if(!p)
    {
      PASS(YES, "URLFragmentAllowedCharacterSet passwed");
    }
  /*
      PASS(//[theSet characterIsMember: 'A'] &&
       //[theSet characterIsMember: 'Z'] &&
       //[theSet characterIsMember: 'a'] &&
       //[theSet characterIsMember: 'z'] &&
       //[theSet characterIsMember: '9'] &&
       // [theSet characterIsMember: '0'] &&
       [theSet characterIsMember: '?'] &&
       [theSet characterIsMember: '/ '] &&
       [theSet characterIsMember: ':'] &&
       [theSet characterIsMember: '@ '] &&
       [theSet characterIsMember: '-'] &&
       [theSet characterIsMember: '.'] &&
       [theSet characterIsMember: '_'] &&
       [theSet characterIsMember: '~'] &&
       [theSet characterIsMember: '!'] &&
       [theSet characterIsMember: '$ '] &&
       [theSet characterIsMember: '&'] &&
       // [theSet characterIsMember: '\''] &&
       [theSet characterIsMember: '('] &&
       [theSet characterIsMember: ')'] &&
       [theSet characterIsMember: '*'] &&
       [theSet characterIsMember: '+'] &&
       [theSet characterIsMember: ','] &&
       [theSet characterIsMember: ';'] &&
       [theSet characterIsMember: '='],
       "Check some characters from URLFramgmentAllowedCharacterSet"); */
    
  [arp release]; arp = nil;
  return 0;
}

