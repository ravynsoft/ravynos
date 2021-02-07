#import "Testing.h"
#import <Foundation/NSAutoreleasePool.h>
#import <Foundation/NSException.h>
#import <Foundation/NSDebug.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSString.h>
#import <Foundation/NSObjCRuntime.h>

#include        <string.h>

#if	defined(GNUSTEP)
#import		<GNUstepBase/GSObjCRuntime.h>
#else
#include        <objc/runtime.h>
#endif

static int      c1count = 0;
static int      c1initialize = 0;
static int      c1load = 0;

@interface      Class1 : NSObject
{
  int   ivar1;
  Class1 *ivar1obj;
}
- (const char *) sel1;
@end

@implementation Class1
+ (void) initialize
{
  if (self == [Class1 class])
    c1initialize = ++c1count;
}
+ (void) load
{
  c1load = ++c1count;
}
- (const char *) sel1
{
  return "";
}
@end

@protocol       SubProto
- (const char *) sel2;
@end

@interface      SubClass1 : Class1 <SubProto>
{
  int   ivar2;
}
- (const char *) sel2;
@end

@implementation SubClass1
- (const char *) sel2
{
  return "";
}
@end

@interface      SubClass1 (Cat1)
- (BOOL) catMethod;
- (const char *) sel2;
@end

@implementation SubClass1 (Cat1)
- (BOOL) catMethod
{
  return YES;
}
- (const char *) sel2
{
  return "category sel2";
}
@end

int
main(int argc, char *argv[])
{
  id            obj;
  Class         cls;
  Class         meta;
  SEL           sel;
  Ivar          ivar;
  Ivar          *ivars;
  unsigned int  count;
  Method        method;
  Method        *methods;
  Protocol      **protocols;
  NSUInteger    s;
  NSUInteger    a;
  const char    *t0;
  const char    *t1;
  const char    *n;

  t0 = "1@1:@";
  t1 = NSGetSizeAndAlignment(t0, &s, &a);
  PASS(t1 == &t0[2], "NSGetSizeAndAlignment() steps through id");
  t1 = NSGetSizeAndAlignment(t1, &s, &a);
  PASS(t1 == &t0[4], "NSGetSizeAndAlignment() steps through sel");

  PASS(NO == class_isMetaClass(Nil),
    "class_isMetaClass() returns NO for Nil");
  PASS(Nil == class_getSuperclass(Nil),
    "class_getSuperclass() returns NO for Nil");

  /* NB. the OSX documentation says that the function returns an empty string
   * when given a Nil argument, but the actual behavior on OSX 10.6 is to
   * return the string "nil"
   */
  PASS_RUNS(n = class_getName(Nil), "class_getName() for Nil does not crash")
  PASS(n != 0 && strcmp(n, "nil") == 0, "class_getName() for Nil is nil");

  PASS(0 == class_getInstanceVariable(Nil, 0), 
    "class_getInstanceVariables() for Nil,0 is 0");
  PASS(0 == class_getVersion(Nil), 
    "class_getVersion() for Nil is 0");

  obj = [NSObject new];
  cls = [SubClass1 class];

  PASS(c1initialize != 0, "+initialize was called");
  PASS(c1load != 0, "+load was called");
  PASS(c1initialize > c1load, "+load occurs before +initialize");
  PASS(strcmp(class_getName(cls), "SubClass1") == 0, "class name works");
  PASS(YES == class_respondsToSelector(cls, @selector(sel2)),
    "class_respondsToSelector() works for class method");
  PASS(YES == class_respondsToSelector(cls, @selector(sel1)),
    "class_respondsToSelector() works for superclass method");
  PASS(NO == class_respondsToSelector(cls, @selector(rangeOfString:)),
    "class_respondsToSelector() returns NO for unknown method");
  PASS(NO == class_respondsToSelector(cls, 0),
    "class_respondsToSelector() returns NO for nul selector");
  PASS(NO == class_respondsToSelector(0, @selector(sel1)),
    "class_respondsToSelector() returns NO for nul class");
  meta = object_getClass(cls);
  PASS(class_isMetaClass(meta), "object_getClass() retrieves meta class");
  PASS(strcmp(class_getName(meta), "SubClass1") == 0, "metaclass name works");
  ivar = class_getInstanceVariable(cls, 0);
  PASS(ivar == 0, "class_getInstanceVariable() returns 0 for null name");
  ivar = class_getInstanceVariable(cls, "bad name");
  PASS(ivar == 0, "class_getInstanceVariable() returns 0 for non-existent");
  ivar = class_getInstanceVariable(0, "ivar2");
  PASS(ivar == 0, "class_getInstanceVariable() returns 0 for Nil class");
  ivar = class_getInstanceVariable(cls, "ivar2");
  PASS(ivar != 0, "class_getInstanceVariable() works");
  ivar = class_getInstanceVariable(cls, "ivar1");
  PASS(ivar != 0, "class_getInstanceVariable() works for superclass ivar");
  ivar = class_getInstanceVariable(cls, "ivar1obj");
  PASS(ivar != 0, "class_getInstanceVariable() works for superclass obj ivar");

  methods = class_copyMethodList(cls, &count);
  PASS(count == 3, "SubClass1 has three methods");
  PASS(methods[count] == 0, "method list is terminated");

  method = methods[2];
  sel = method_getName(method);
  PASS(sel_isEqual(sel, sel_getUid("sel2")),
    "last method is sel2");
  PASS(method_getImplementation(method) != [cls instanceMethodForSelector: sel],
    "method 2 is the original, overridden by the category");

  method = methods[0];
  sel = method_getName(method);
  PASS(sel_isEqual(sel, sel_getUid("catMethod"))
    || sel_isEqual(sel, sel_getUid("sel2")),
    "method 0 has expected name");

  if (sel_isEqual(sel, sel_getUid("catMethod")))
    {
      method = methods[1];
      sel = method_getName(method);
      PASS(sel_isEqual(sel, sel_getUid("sel2")),
        "method 1 has expected name");
      PASS(method_getImplementation(method)
        == [cls instanceMethodForSelector: sel],
        "method 1 is the category method overriding original");
    }
  else
    {
      PASS(method_getImplementation(method)
        == [cls instanceMethodForSelector: sel],
        "method 0 is the category method overriding original");
      method = methods[1];
      sel = method_getName(method);
      PASS(sel_isEqual(sel, sel_getUid("catMethod")),
        "method 1 has expected name");
    }

  ivars = class_copyIvarList(cls, &count);
  PASS(count == 1, "SubClass1 has one ivar");
  PASS(ivars[count] == 0, "ivar list is terminated");
  PASS(strcmp(ivar_getName(ivars[0]), "ivar2") == 0,
    "ivar has correct name");
  PASS(strcmp(ivar_getTypeEncoding(ivars[0]), @encode(int)) == 0,
    "ivar has correct type");

  protocols = class_copyProtocolList(cls, &count);
  PASS(count == 1, "SubClass1 has one protocol");
  PASS(protocols[count] == 0, "protocol list is terminated");
  PASS(strcmp(protocol_getName(protocols[0]), "SubProto") == 0,
    "protocol has correct name");

  cls = objc_allocateClassPair([NSString class], "runtime generated", 0);
  PASS(cls != Nil, "can allocate a class pair");
  PASS(class_addIvar(cls, "iv1", 1, 6, "c") == YES,
    "able to add iVar 'iv1'");
  PASS(class_addIvar(cls, "iv2", 1, 5, "c") == YES,
    "able to add iVar 'iv2'");
  PASS(class_addIvar(cls, "iv3", 1, 4, "c") == YES,
    "able to add iVar 'iv3'");
  PASS(class_addIvar(cls, "iv4", 1, 3, "c") == YES,
    "able to add iVar 'iv4'");
  objc_registerClassPair (cls);
  ivar = class_getInstanceVariable(cls, "iv1");
  PASS(ivar != 0, "iv1 exists");
  PASS(ivar_getOffset(ivar) == 64, "iv1 offset is 64");
  ivar = class_getInstanceVariable(cls, "iv2");
  PASS(ivar != 0, "iv2 exists");
  PASS(ivar_getOffset(ivar) == 96, "iv2 offset is 96");
  ivar = class_getInstanceVariable(cls, "iv3");
  PASS(ivar != 0, "iv3 exists");
  PASS(ivar_getOffset(ivar) == 112, "iv3 offset is 112");
  ivar = class_getInstanceVariable(cls, "iv4");
  PASS(ivar != 0, "iv4 exists");
  PASS(ivar_getOffset(ivar) == 120, "iv4 offset is 120");

  /* NSObjCRuntime function tests.
   */
  sel = NSSelectorFromString(nil);
  PASS(sel == 0,
    "NSSelectorFromString() returns 0 for nil string");
  PASS(NSStringFromSelector(0) == nil,
    "NSStringFromSelector() returns nil for null selector");
  sel = NSSelectorFromString(@"xxxyyy_odd_name_xxxyyy");
  PASS(sel != 0,
    "NSSelectorFromString() creates for non-existent selector");
  PASS([NSStringFromSelector(sel) isEqual: @"xxxyyy_odd_name_xxxyyy"],
    "NSStringFromSelector() works for existing selector");

  return 0;
}

