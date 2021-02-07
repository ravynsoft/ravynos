#import <Foundation/Foundation.h>

#import "Testing.h"

static BOOL rangesEqual(NSRange r1, NSRange r2)
{
  if (&r1 == &r2) 
    return YES;

  if (r1.length == 0 && r2.length == 0)
    return YES;

  return (r1.length == r2.length && r1.location == r2.location);
}

static void dataRange(char *s0, char *s1, NSDataSearchOptions opts,
 		     NSRange range, NSRange want)
{
  NSData *d0 = [NSData dataWithBytes:s0 length:strlen(s0)];
  NSData *d1 = [NSData dataWithBytes:s1 length:strlen(s1)];

  NSRange res = [d0 rangeOfData:d1 options:opts range:range];
  PASS(rangesEqual(res,want), "NSData range for '%s' and '%s' is ok",s0,s1);
}

int main()
{
  NSAutoreleasePool *arp = [NSAutoreleasePool new];

  /* Borrowed from NSString/test00.m
   */
  dataRange("hello", "hello", NSDataSearchAnchored,
    NSMakeRange(0,5), NSMakeRange(0,5));
  dataRange("hello", "hello", NSDataSearchAnchored | NSDataSearchBackwards,
    NSMakeRange(0,5), NSMakeRange(0,5));
  dataRange("hello", "hElLo", 0,
    NSMakeRange(0,5), NSMakeRange(NSNotFound,0));
  dataRange("hello", "hell", NSDataSearchAnchored,
    NSMakeRange(0,5), NSMakeRange(0,4));
  dataRange("hello", "hell", NSDataSearchAnchored | NSDataSearchBackwards,
    NSMakeRange(0,4), NSMakeRange(0,4));
  dataRange("hello", "ello", NSDataSearchAnchored,
    NSMakeRange(0,5), NSMakeRange(NSNotFound,0));
  dataRange("hello", "hel", NSDataSearchBackwards,
    NSMakeRange(0,5), NSMakeRange(0,3));
  dataRange("hello", "he", 0,
    NSMakeRange(0,5), NSMakeRange(0,2));
  dataRange("hello", "h", 0,
    NSMakeRange(0,5), NSMakeRange(0,1));
  dataRange("hello", "l", 0,
    NSMakeRange(0,5), NSMakeRange(2,1));
  dataRange("hello", "l", NSDataSearchBackwards,
    NSMakeRange(0,5), NSMakeRange(3,1));
  dataRange("hello", "", 0,
    NSMakeRange(0,5), NSMakeRange(0,0));
  dataRange("hello", "el", 0,
    NSMakeRange(0,5), NSMakeRange(1,2));
  dataRange("hello", "el", 0,
    NSMakeRange(0,2), NSMakeRange(0,0));
  dataRange("hello", "el", 0,
    NSMakeRange(2,3), NSMakeRange(0,0));
  dataRange("hello", "ell", 0,
    NSMakeRange(0,5), NSMakeRange(1,3));
  dataRange("hello", "lo", 0,
    NSMakeRange(2,3), NSMakeRange(3,2));
  dataRange("boaboaboa", "abo", 0,
    NSMakeRange(0,9), NSMakeRange(2,3));
  dataRange("boaboaboa", "abo", NSDataSearchBackwards,
    NSMakeRange(0,9), NSMakeRange(5,3));
  dataRange("", "", 0,
    NSMakeRange(0,0), NSMakeRange(0,0));
  dataRange("x", "", 0,
    NSMakeRange(0,1), NSMakeRange(0,0));
  dataRange("x", "", NSDataSearchBackwards,
    NSMakeRange(0,1), NSMakeRange(1,0));

  [arp release];
  arp = nil;
  
  return 0;
}
