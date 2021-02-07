#import "ObjectTesting.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSString.h>

int main()
{
  NSAutoreleasePool	*arp = [NSAutoreleasePool new];
  unichar		u = 0x00a3;	// Pound sign
  NSString		*s;
  double 		d;

  PASS([@"12" intValue] == 12, "simple intValue works");
  PASS([@"-12" intValue] == -12, "negative intValue works");
  PASS([@"+12" intValue] == 12, "positive intValue works");
  PASS([@"1.6" intValue] == 1, "intValue ignores trailing data");
  PASS([@"                                12" intValue] == 12,
    "intValue with leading space works");

  d = [@"1.2" doubleValue];
  PASS(d > 1.199999 && d < 1.200001, "simple doubleValue works");
  PASS([@"1.9" doubleValue] == 90 / 100.0 + 1.0, "precise doubleValue works");
  d = [@"-1.2" doubleValue];
  PASS(d < -1.199999 && d > -1.200001, "negative doubleValue works");
  d = [@"+1.2" doubleValue];
  PASS(d > 1.199999 && d < 1.200001, "positive doubleValue works");
  d = [@"+1.2 x" doubleValue];
  PASS(d > 1.199999 && d < 1.200001, "doubleValue ignores trailing data");
  d = [@"                                1.2" doubleValue];
  PASS(d > 1.199999 && d < 1.200001, "doubleValue with leading space works");

  s = @"50.6468746467461646";
  sscanf([s UTF8String], "%lg", &d);
  PASS(EQ([s doubleValue], d), "50.6468746467461646 -doubleValue OK");

  s = @"50.64687464674616461";
  sscanf([s UTF8String], "%lg", &d);
  PASS(EQ([s doubleValue], d), "50.64687464674616461 -doubleValue OK");

  s = @"0.646874646746164616898211";
  sscanf([s UTF8String], "%lg", &d);
  PASS(EQ([s doubleValue], d), "0.646874646746164616898211 -doubleValue OK");

  s = @"502.646874646746164";
  sscanf([s UTF8String], "%lg", &d);
  PASS(EQ([s doubleValue], d), "502.646874646746164 -doubleValue OK");

  s = @"502.6468746467461646";
  sscanf([s UTF8String], "%lg", &d);
  PASS(EQ([s doubleValue], d), "502.6468746467461646 -doubleValue OK");

  s = [NSString stringWithCharacters: &u length: 1];
  PASS_EQUAL(s, @"£", "UTF-8 string literal matches 16bit unicode string");

  [arp release]; arp = nil;
  return 0;
}
