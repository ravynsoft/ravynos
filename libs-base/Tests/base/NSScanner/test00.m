#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSException.h>
#import <Foundation/NSString.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSCharacterSet.h>

static BOOL scanAWord(NSScanner *theScanner, NSCharacterSet *cSet)
{
  BOOL ret = ([theScanner scanUpToCharactersFromSet:cSet intoString:NULL]
  	  && [theScanner scanCharactersFromSet:cSet intoString:NULL]);
  return ret;
}

static BOOL scanThreeWords(NSString *string,NSCharacterSet *separators)
{
  NSScanner *scanner = [NSScanner scannerWithString:string];
  /* so it doesn't skip the whitespace */
  [scanner setCharactersToBeSkipped:[NSCharacterSet characterSetWithCharactersInString:@""]];
  return ((scanAWord(scanner,separators)
   	    && ![scanner isAtEnd]
  	    && scanAWord(scanner,separators) 
	    && ![scanner isAtEnd]
            && scanAWord(scanner,separators)
            && [scanner isAtEnd]));
}

int main()
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  NSString *str = nil;
  id charSet = nil;
  
  charSet = [NSCharacterSet whitespaceAndNewlineCharacterSet];
    
  str = [NSString stringWithCString:"one\ntwo\nthree\n"];
  PASS(scanThreeWords(str,charSet), 
       "scanning three words separated by newlines");
  
  str = @"one two three ";
  PASS(scanThreeWords(str,charSet), 
       "scanning three words separated by spaces");
  
  str = @"one\ttwo\tthree\t";
  PASS(scanThreeWords(str,charSet), 
       "scanning three words separated by tabs");
  
  str = @"one \ntwo \nthree \n";
  PASS(scanThreeWords(str,charSet), 
       "scanning three words separated by newlines and spaces");
  
  charSet = [charSet mutableCopy];
  [charSet addCharactersInString:@";"];
  str = @"one ;two ;three ;";
  PASS(scanThreeWords(str,charSet), 
       "scanning three words separated by spaces and semi-colons");
  
  [arp release]; arp = nil;
  return 0;
}
