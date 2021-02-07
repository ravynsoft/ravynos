#define _GNU_SOURCE
#define _ISOC99_SOURCE
#import <Foundation/Foundation.h>

#import "Testing.h"

#include <limits.h>

typedef struct _GSFinePoint GSFinePoint;
struct _GSFinePoint
{
  double x;
  double y;
};
GSFinePoint
GSMakeFinePoint(double x, double y)
{
  GSFinePoint point;
  point.x = x;
  point.y = y;
  return point;
}
BOOL
GSEqualFinePoints(GSFinePoint aPoint, GSFinePoint bPoint)
{
  return ((aPoint.x == bPoint.x)
          && (aPoint.y == bPoint.y)) ? YES : NO;
}

typedef struct _GSBitField GSBitField;
struct _GSBitField
{
  unsigned int first:1;
  unsigned int unused:1;
  unsigned int second:1;
};
GSBitField
GSMakeBitField(BOOL first, BOOL second)
{
  GSBitField field;
  field.first = first;
  field.second = second;
  return field;
}
BOOL
GSEqualBitFields(GSBitField aField, GSBitField bField)
{
  return ((aField.first == bField.first)
          && (aField.second == bField.second)) ? YES : NO;
}

NSDecimal
GSMakeDecimal(unsigned long long mantissa, short exponent, BOOL negative)
{
  NSDecimalNumber       *d;
  NSDecimal             dec;

  d = [NSDecimalNumber decimalNumberWithMantissa: mantissa
                                        exponent: exponent
                                      isNegative: negative];
  dec = [d decimalValue];
  return dec;
}

NSDecimal
GSDecimalMultiply(NSDecimal left,NSDecimal right)
{
  NSDecimal dec;
  NSDecimalMultiply(&dec,&left,&right,NSRoundPlain);
  return dec;
}

BOOL
GSDecimalCompare(NSDecimal left,NSDecimal right)
{
  return NSDecimalCompare(&left,&right);
}

@interface TypeTester : NSObject
{
}
-(void)voidPvoid;
-(signed char)sCharPsChar:(signed char)v;
-(unsigned char)uCharPuChar:(unsigned char)v;
-(short)shortPshort:(short)v;
-(unsigned short)uShortPuShort:(unsigned short)v;
-(int)intPint:(int)v;
-(unsigned int)uIntPuInt:(unsigned int)v;
-(long)longPlong:(long)v;
-(unsigned long)uLongPuLong:(unsigned long)v;
-(long long)longlongPlonglong:(long long)v;
-(unsigned long long)ulonglongPulonglong:(unsigned long long)v;
-(float)floatPfloat:(float)v;
-(double)doublePdouble:(double)v;
-(long double)longdoublePlongdouble:(long double)v;
-(id)idPid:(id)v;

-(NSStringEncoding)enumPenum:(NSStringEncoding)v;
-(NSRange)rangePrange:(NSRange)v;
-(NSPoint)pointPpoint:(NSPoint)v;
-(NSDecimal)decimalPdecimal:(NSDecimal)v;

-(GSFinePoint)finePointPfinePoint:(GSFinePoint)v;
-(GSBitField)bitFieldPbitField:(GSBitField)v;

@end

@implementation TypeTester
-(void)voidPvoid {}
-(signed char)sCharPsChar:(signed char)v { return v - 1; }
-(unsigned char)uCharPuChar:(unsigned char)v { return v - 1; }
-(short)shortPshort:(short)v { return v - 1; }
-(unsigned short)uShortPuShort:(unsigned short)v { return v - 1; }
-(int)intPint:(int)v { return v - 1; }
-(unsigned int)uIntPuInt:(unsigned int)v { return v - 1; }
-(long)longPlong:(long)v { return v - 1; }
-(unsigned long)uLongPuLong:(unsigned long)v { return v - 1; }
-(long long)longlongPlonglong:(long long)v { return v - 1; }
-(unsigned long long)ulonglongPulonglong:(unsigned long long)v { return v - 1; }
-(float)floatPfloat:(float)v { return v - 1.0; }
-(double)doublePdouble:(double)v { return v - 1.0; }
-(long double)longdoublePlongdouble:(long double)v { return v - 1.0; }
-(id)idPid:(id)v { return v == [NSProcessInfo processInfo] ? [NSNull null] : nil; }

-(NSStringEncoding)enumPenum:(NSStringEncoding)v { return v == NSSymbolStringEncoding ? NSNEXTSTEPStringEncoding : 0; }
-(NSRange)rangePrange:(NSRange)v { return NSMakeRange(v.length,v.location); }
-(NSPoint)pointPpoint:(NSPoint)v { return NSMakePoint(v.y,v.x); }
-(NSDecimal)decimalPdecimal:(NSDecimal)v { return GSDecimalMultiply(v,v); }

-(GSFinePoint)finePointPfinePoint:(GSFinePoint)v { return GSMakeFinePoint((double)v.y,(double)v.x); } 
-(GSBitField)bitFieldPbitField:(GSBitField)v { return GSMakeBitField(v.second,v.first); }

@end

@interface MyProxy : NSProxy
{
  id _remote;
}
@end

@implementation MyProxy
-(id) init
{
  _remote = nil;
  return self;
}
- (void) dealloc
{
  [_remote release];
  DEALLOC
}
-(void) setRemote:(id)remote
{
  ASSIGN(_remote,remote);
}
-(NSString *) description
{
  return [_remote description];
}
-(id) remote
{
  return _remote;
}
- (NSMethodSignature *) methodSignatureForSelector:(SEL)aSelector
{
  NSMethodSignature *sig = [_remote methodSignatureForSelector:aSelector];
  if (sig == nil)
    sig = [self methodSignatureForSelector:aSelector];
  return sig;
}
- (void) forwardInvocation:(NSInvocation *)inv
{
  [inv setTarget:_remote];
  [inv invoke];
}
@end

int
main(int argc, char *argv[])
{
  NSAutoreleasePool   *arp = [NSAutoreleasePool new];
  id obj = nil;
  id rem = [TypeTester new];
  GSFinePoint	f0;
  GSFinePoint	f1;
  NSPoint	p0;
  NSPoint	p1;
  NSRange	r0;
  NSRange	r1;
  
  obj = [[MyProxy alloc] init];
  [obj setRemote:rem];

  [obj voidPvoid]; //shoudn't raise
  PASS([obj sCharPsChar: 128] == 127, "Proxy signed char")
  PASS([obj uCharPuChar: 255] == 254, "Proxy unsigned char")
  PASS([obj shortPshort: SHRT_MAX] == (SHRT_MAX - 1), "Proxy signed short")
  PASS([obj uShortPuShort: USHRT_MAX] == (USHRT_MAX - 1),
    "Proxy unsigned short");
  PASS([obj intPint: INT_MAX] == (INT_MAX - 1),
    "Proxy signed int")
  PASS([obj uIntPuInt: UINT_MAX] == (UINT_MAX - 1U),
    "Proxy unsigned int")
  PASS([obj longPlong: LONG_MAX] == (LONG_MAX - 1L),
    "Proxy signed long")
  PASS([obj uLongPuLong: ULONG_MAX] == (ULONG_MAX - 1UL),
    "Proxy unsigned long")
  pass([obj longlongPlonglong: LLONG_MAX] == (LLONG_MAX - 1LL),
    "Proxy signed long long");
  PASS([obj ulonglongPulonglong: ULLONG_MAX] == (ULLONG_MAX - 1ULL),
    "Proxy unsigned long long")
  pass([obj floatPfloat: (float)3.14] == ((float)3.14 - (float)1.0),
    "Proxy float");
  pass([obj doublePdouble: (double)3.14] == ((double)3.14 - (double)1.0),
    "Proxy double");

  PASS([obj idPid: [NSProcessInfo processInfo]] == [NSNull null],
    "Proxy id")
  PASS([obj enumPenum: NSSymbolStringEncoding] == NSNEXTSTEPStringEncoding,
    "Proxy enum")
  r0 = [obj rangePrange: NSMakeRange(243,437)];
  r1 = NSMakeRange(437,243);
  PASS(NSEqualRanges(r0, r1), "Proxy NSRange")
  p0 = [obj pointPpoint: NSMakePoint(243.0F,437.0F)];
  p1 = NSMakePoint(437.0F,243.0F);
  PASS(NSEqualPoints(p0, p1), "Proxy NSPoint")
  f0 = [obj finePointPfinePoint: GSMakeFinePoint(243.0L,437.0L)];
  f1 = GSMakeFinePoint(437.0L,243.0L);
  PASS(GSEqualFinePoints(f0, f1), "Proxy GSFinePoint")
#if 0
  /* These next two are disabled because they test passing structures as
   * parameters, a feature which has never worked and which maybe doesn't
   * need to.
   * We should enable these tests if/when the capability is ever implemented.
   */
  BOOL ok;

  ok = GSDecimalCompare([obj decimalPdecimal: GSMakeDecimal(321,7,YES)],
    GSDecimalMultiply(GSMakeDecimal(321,7,YES),GSMakeDecimal(321,7,YES)))==0;
  PASS(ok, "Proxy NSDecimal")
#endif
#if 0
  /*
    Disabled since this currently causes segfaults.  Yet this feature is hardly ever used.
    Note also: file:///usr/share/doc/libffi-dev/html/Missing-Features.html#Missing-Features:
    libffi is missing a few features. We welcome patches to add support for these.

    * There is no support for calling varargs functions. This may work on some platforms, depending on how the ABI is defined, but it is not reliable.
    * There is no support for bit fields in structures.
    * The closure API is
    * The raw” API is undocumented.
    */
  PASS(GSEqualBitFields([obj bitFieldPbitField: GSMakeBitField(0,1)],GSMakeBitField(1,0)), "Proxy GSBitField")
#endif
  
  [arp release]; arp = nil;
  return 0;
}
