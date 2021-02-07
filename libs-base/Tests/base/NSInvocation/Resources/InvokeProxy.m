/* InvokeProxy.h - Implementation of simple target and proxy classes 

   Written By: Adam Fedor <fedor@gnu.org>
*/
#include "InvokeProxy.h"
#include <Foundation/NSMethodSignature.h>
#include <Foundation/NSInvocation.h>
#include <Foundation/NSString.h>
#include <Foundation/NSException.h>
//#include "GNUstepGuile/gstep_guile.h"

@implementation	InvokeTarget

- (char) loopChar: (char)v
{
  return v+1;
}
- (double) loopDouble: (double)v
{
  return v+1.0;
}
- (float) loopFloat: (float)v
{
  return v+1.0;
}
- (int) loopInt: (int)v
{
  return v+1;
}
- (largeStruct) loopLarge: (largeStruct)v
{
  v.i += 1;
  return v;
}
- (long) loopLong: (long)v
{
  return v+1;
}
- (largeStruct) loopLargePtr: (largeStruct*)v
{
  return *v;
}
- (id) loopObject: (id)v
{
  return v;
}
- (short) loopShort: (short)v
{
  return v+1;
}
- (smallStruct) loopSmall: (smallStruct)v
{
  v.i += 1;
  return v;
}
- (NSRect) loopRect: (NSRect)v
{
  return v;
}
- (smallStruct) loopSmallPtr: (smallStruct*)v
{
  return *v;
}
- (char*) loopString: (char*)v
{
  v[0] += 1;
  return v;
}

- (char) retChar
{
  return (char)99;
}
- (double) retDouble
{
  return 123.456;
}
- (float) retFloat
{
  return 123.456;
}
- (int) retInt
{
  return 123456;
}
- (largeStruct) retLarge
{
  static largeStruct l = {
    99, "largeStruct", 99.99
  };
  return l;
}
- (long) retLong
{
  return 123456;
}
- (id) retObject
{
  return self;
}
- (short) retShort
{
  return 12345;
}
- (smallStruct) retSmall
{
  static smallStruct s = {
    11, 22
  };
  return s;
}
- (char*) retString
{
  return "string";
}

- (id) returnIdButThrowException
{
    [NSException raise: @"AnException" format: @"Deliberately thrown"];
    return @"This string should not be returned";
}

@end

@implementation	InvokeProxy
#if 0
+ (void) load
{
  /* Make sure these classes are defined within gstep-guile, so tests
     can find them */
#define CCLS(X) gh_define(#X, gstep_id2scm([X class], NO))
    CCLS(InvokeTarget);
    CCLS(InvokeProxy);
}  
#endif 
- (id) initWithTarget: (id)target
{
  obj = target;
  return self;
}

- (void) forwardInvocation: (NSInvocation*)inv
{
#if 1
  [inv invokeWithTarget: obj];
#else
  NSData	*d = [NSArchiver archivedDataWithRootObject: inv];
  NSInvocation	*i = [NSUnarchiver unarchiveObjectWithData: d];
  unsigned	l;
  void		*b;

  [i invokeWithTarget: obj];
  d = [NSArchiver archivedDataWithRootObject: i];
  i = [NSUnarchiver unarchiveObjectWithData: d];
  l = [[i methodSignature] methodReturnLength];
  b = (void *)malloc(l);
  [i getReturnValue: b];
  [inv setReturnValue: b];
  free(b);
#endif
}

@end


