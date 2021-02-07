#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSCharacterSet.h>

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSCharacterSet *theSet = nil;
  
  theSet = [NSCharacterSet alphanumericCharacterSet];
  PASS(theSet != nil, "NSCharacterSet understands [+alphanumericCharacterSet]");
  PASS([NSCharacterSet alphanumericCharacterSet] == theSet, 
       "NSCharacterSet uniques alphanumericCharacterSet");
  
  theSet = [NSCharacterSet controlCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+controlCharacterSet]");
  PASS([NSCharacterSet controlCharacterSet] == theSet, 
       "NSCharacterSet uniques controlCharacterSet");
  
  theSet = [NSCharacterSet decimalDigitCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+decimalDigitCharacterSet]");
  PASS([NSCharacterSet decimalDigitCharacterSet] == theSet,
       "NSCharacterSet uniques [+decimalDigitCharacterSet]");
  
  theSet = [NSCharacterSet illegalCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+illegalCharacterSet]");
  PASS([NSCharacterSet illegalCharacterSet] == theSet,
       "NSCharacterSet uniques [+illegalCharacterSet]");
  
  theSet = [NSCharacterSet letterCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+letterCharacterSet]");
  PASS([NSCharacterSet letterCharacterSet] == theSet,
       "NSCharacterSet uniques [+letterCharacterSet]");
  
  theSet = [NSCharacterSet lowercaseLetterCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+lowercaseLetterCharacterSet]");
  PASS([NSCharacterSet lowercaseLetterCharacterSet] == theSet,
       "NSCharacterSet uniques [+lowercaseLetterCharacterSet]");
  
  theSet = [NSCharacterSet nonBaseCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+nonBaseCharacterSet]");
  PASS([NSCharacterSet nonBaseCharacterSet] == theSet,
       "NSCharacterSet uniques [+nonBaseCharacterSet]");
  
  theSet = [NSCharacterSet punctuationCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+punctuationCharacterSet]");
  PASS([NSCharacterSet punctuationCharacterSet] == theSet,
       "NSCharacterSet uniques [+punctuationCharacterSet]");
  
  theSet = [NSCharacterSet uppercaseLetterCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+uppercaseLetterCharacterSet]");
  PASS([NSCharacterSet uppercaseLetterCharacterSet] == theSet,
       "NSCharacterSet uniques [+uppercaseLetterCharacterSet]");
  
  theSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+whitespaceAndNewlineCharacterSet]");
  PASS([NSCharacterSet whitespaceAndNewlineCharacterSet] == theSet,
       "NSCharacterSet uniques [+whitespaceAndNewlineCharacterSet]");
  
  theSet = [NSCharacterSet whitespaceCharacterSet];
  PASS(theSet != nil,"NSCharacterSet understands [+whitespaceCharacterSet]");
  PASS([NSCharacterSet whitespaceCharacterSet] == theSet,
       "NSCharacterSet uniques [+whitespaceCharacterSet]");
  
  [arp release]; arp = nil;
  return 0;
}
