#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSException.h>
#import <Foundation/NSScanner.h>
#import <Foundation/NSString.h>
#import <Foundation/NSValue.h>
#import <math.h>

static BOOL scanInt(int value, int *retp)
{ 
  NSString *str = [[NSNumber numberWithInt: value] description];
  NSScanner *scn = [NSScanner scannerWithString: str];
  return ([scn scanInt: retp] && value == *retp);
}
/* this didn't seem to be used in any of the NSScanner guile tests 
   so this function is untested.

static BOOL scanIntOverflow(int value, int *retp)
{
  NSString *str = [[NSNumber numberWithFloat: value] description];
  NSScanner *scn = [NSScanner scannerWithString: str];
  int intmax = 2147483647;
  int intmin = (0 - (intmax - 1));
  
  return ([scn scanInt: retp] 
    && (0 > value) ? (intmin == *retp) : (intmax == *retp));
}
*/

#if     defined(GNUSTEP_BASE_LIBRARY)
static BOOL scanRadixUnsigned(NSString *str, 
  int expectValue,
  unsigned int expectedValue,
  int expectedScanLoc,
  unsigned *retp)
{
  NSScanner *scn = [NSScanner scannerWithString: str];
  [scn scanRadixUnsignedInt: retp];
  return ((expectValue == 1) ? (expectedValue == *retp) : YES
    && expectedScanLoc == [scn scanLocation]);
}
static BOOL scanRadixUnsignedLongLong(NSString *str, 
  int expectValue,
  unsigned int expectedValue,
  int expectedScanLoc,
  unsigned long long *retp)
{
  NSScanner *scn = [NSScanner scannerWithString: str];
  [scn scanRadixUnsignedLongLong: retp];
  return ((expectValue == 1) ? (expectedValue == *retp) : YES
    && expectedScanLoc == [scn scanLocation]);
}
#endif

static BOOL scanHex(NSString *str, 
  int expectValue,
  unsigned int expectedValue,
  int expectedScanLoc,
  unsigned *retp)
{
  NSScanner *scn = [NSScanner scannerWithString: str];
  [scn scanHexInt: retp];
  return ((expectValue == 1) ? (expectedValue == *retp) : YES
    && expectedScanLoc == [scn scanLocation]);
}

static BOOL scanHexLongLong(NSString *str,
  int expectValue,
  unsigned long long expectedValue,
  int expectedScanLoc,
  unsigned long long *retp)
{
  NSScanner *scn = [NSScanner scannerWithString: str];
  [scn scanHexLongLong: retp];
  return ((expectValue == 1) ? (expectedValue == *retp) : YES
    && expectedScanLoc == [scn scanLocation]);
}

static BOOL scanDouble(NSString *str, 
  double expectedValue,
  double *retp)
{
  NSScanner *scn = [NSScanner scannerWithString: str];
  [scn scanDouble: retp];
  return ((0.00000000001 >= fabs(expectedValue - *retp))
    && [str length] == [scn scanLocation]);
}

int main()
{
  NSAutoreleasePool     *arp = [NSAutoreleasePool new];
  long long             lret;
  int                   ret;
  double                dret;
  int                   intmax = 2147483647;
  int                   intmin = (0 - (intmax - 1));
  NSScanner             *scn;
  float                 flt;
  NSString              *em;

  PASS(scanInt((intmax - 20),&ret), "NSScanner large ints"); 
  PASS(scanInt((intmin + 20),&ret), "NSScanner small ints");
  
  scn = [NSScanner scannerWithString:@"1234F00"];
  PASS(([scn scanInt:&ret] && ([scn scanLocation] == 4)),
    "NSScanner non-digits terminate scan");
  
  scn = [NSScanner scannerWithString:@"junk"];
  PASS((![scn scanInt:&ret] && ([scn scanLocation] == 0)),
    "NSScanner non-digits terminate scan");
  
  scn = [NSScanner scannerWithString:@"junk"];
  PASS(![scn scanInt:&ret] && ([scn scanLocation] == 0),
    "NSScanner non-digits dont consume characters to be skipped");
  
#if     defined(GNUSTEP_BASE_LIBRARY)
  PASS(scanRadixUnsigned(@"1234FOO", 1, 1234, 4, (unsigned*)&ret)
    && scanRadixUnsigned(@"01234FOO", 1, 01234, 5, (unsigned*)&ret)
    && scanRadixUnsigned(@"0x1234FOO", 1, 0x1234F, 7, (unsigned*)&ret)
    && scanRadixUnsigned(@"0X1234FOO", 1, 0x1234F, 7, (unsigned*)&ret)
    && scanRadixUnsigned(@"012348FOO", 1, 01234, 5, (unsigned*)&ret),
    "NSScanner radiux unsigned (non-digits terminate scan)");
  
  PASS(scanRadixUnsignedLongLong(
      @"1234FOO", 1, 1234, 4, (unsigned long long*)&lret)
    && scanRadixUnsignedLongLong(
      @"01234FOO", 1, 01234, 5, (unsigned long long*)&lret)
    && scanRadixUnsignedLongLong(
      @"0x1234FOO", 1, 0x1234F, 7, (unsigned long long*)&lret)
    && scanRadixUnsignedLongLong(
      @"0X1234FOO", 1, 0x1234F, 7, (unsigned long long*)&lret)
    && scanRadixUnsignedLongLong(
      @"012348FOO", 1, 01234, 5, (unsigned long long*)&lret),
    "NSScanner radiux unsigned (non-digits terminate scan)");
  
  PASS(scanRadixUnsigned(@"FOO", 0, 0, 0, (unsigned*)&ret)
    && scanRadixUnsigned(@"  FOO", 0, 0, 0, (unsigned*)&ret)
    && scanRadixUnsigned(@" 0x ", 0, 0, 0, (unsigned*)&ret),
    "NSScanner radiux unsigned (non-digits dont move scan)");
#endif
  
  PASS(scanHex(@"1234FOO", 1, 0x1234F, 5, (unsigned*)&ret)
    && scanHex(@"01234FOO", 1, 0x1234F, 6, (unsigned*)&ret),
    "NSScanner hex (non-digits terminate scan)");

  PASS(scanHexLongLong(@"1234FOO", 1, 0x1234F, 5, (unsigned long long*)&lret)
    && scanHexLongLong(@"01234FOO", 1, 0x1234F, 6, (unsigned long long*)&lret),
    "NSScanner hex long long (non-digits terminate scan)");

 /* dbl1 = 123.456;
  dbl2 = 123.45678901234567890123456789012345678901234567;
  dbl3 = 1.23456789;
  */
  PASS(scanDouble(@"123.456",123.456,&dret) 
       && scanDouble(@"123.45678901234567890123456789012345678901234567",
                       123.45678901234567890123456789012345678901234567,&dret)
       && scanDouble(@"0.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000123456789e+100", 1.23456789, &dret),
       "NSScanner scans doubles");
  
  PASS(scanDouble(@"1e0", 1, &dret)
       && scanDouble(@"1e1", 10, &dret)
       && scanDouble(@"1e+1", 10, &dret)
       && scanDouble(@"1e+10", 1e10, &dret)
       && scanDouble(@"1e-1", 1e-1, &dret)
       && scanDouble(@"1e-10", 1e-10, &dret),
       "NSScanner scans double with exponents");

  scn = [NSScanner scannerWithString: @"1.0e+2m"];
  flt = 0.0;
  em = @"";
  [scn scanFloat: &flt];
  [scn scanString: @"m" intoString: &em];
  PASS(flt == 1e+2f, "flt = 1e+2");
  PASS_EQUAL(em, @"m", "e is part of exponent");
  PASS([scn scanLocation] == 7u, "all scanned");
  PASS([scn isAtEnd], "is at end");


  scn = [NSScanner scannerWithString: @"1.0em"];
  flt = 0.0;
  em = @"";
  [scn scanFloat: &flt];
  [scn scanString: @"em" intoString: &em];
  PASS(flt == 1.0f, "flt = 1.0");
  PASS_EQUAL(em, @"em", "em is not part of exponent");
  PASS([scn scanLocation] == 5u, "all scanned");
  PASS([scn isAtEnd], "is at end");

  scn = [NSScanner scannerWithString: @"0.0"];
  flt = 1.0;
  [scn scanFloat: &flt];
  PASS(flt == 0.0, "flt = 0.0");
  PASS([scn isAtEnd], "is at end");

  double	r, g, b;
  NSScanner	*scanner;
  BOOL		ok = NO;
      
  scanner = [NSScanner scannerWithString: @"0 0 0"];
  PASS([scanner scanDouble: &r]
    && [scanner scanDouble: &g]
    && [scanner scanDouble: &b]
    && [scanner isAtEnd]
    && 0.0 == r && 0.0 == g && 0.0 == b,
    "scan three space separated zeros as doubles")

  [arp release]; arp = nil;
  return 0;
}
