#include "Test.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#	include "../safewindows.h"
#	define sleep(x) Sleep(1000 * x)
#else
#	include <unistd.h>
#endif

static int exitStatus = 0;

static void _test(BOOL X, char *expr, int line)
{
  if (!X)
  {
    exitStatus = 1;
    fprintf(stderr, "ERROR: Test failed: '%s' on %s:%d\n", expr, __FILE__, line);
  }
}
#define test(X) _test(X, #X, __LINE__)

static int stringsEqual(const char *a, const char *b)
{
  return 0 == strcmp(a,b);
}

@protocol NSCoding
@end

#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface NSObject <NSCoding>
{
	id isa;
	int refcount;
}
@end
@implementation NSObject
- (id)class
{
	return object_getClass(self);
}
+ (id)class
{
	return self;
}
+ (id)new
{
	return class_createInstance(self, 0);
}
- (void)release
{
	if (refcount == 0)
	{
		object_dispose(self);
	}
	refcount--;
}
- (id)retain
{
	refcount++;
	return self;
}
@end


@interface Foo : NSObject
{
  id a;
}
- (void) aMethod;
+ (void) aMethod;
- (int) manyTypes;
- (void) synchronizedCode;
+ (void) synchronizedCode;
+ (id) shared;
- (BOOL) basicThrowAndCatchException;
@end

@interface Bar : Foo
{
  id b;
}
- (void) anotherMethod;
+ (void) anotherMethod;
- (id) manyTypes;
- (id) aBool: (BOOL)d andAnInt: (int) w;
@end

id exceptionObj = @"Exception";

@implementation Foo
- (void) aMethod
{
}
+ (void) aMethod
{
}
- (int) manyTypes
{
  return YES;
}
- (void) synchronizedCode
{
	@synchronized(self) { [[self class] synchronizedCode]; }
}
+ (void) synchronizedCode
{
	@synchronized(self) { }
}
+ (id) shared
{
	@synchronized(self) { }
	return nil;
}
- (void) throwException
{
	@throw exceptionObj;
}
- (BOOL) basicThrowAndCatchException
{
	@try
	{
		[self throwException];
	}
	@catch (id e)
	{
		test(e == exceptionObj);
		return YES;
	}
	@catch(...)
	{
		return NO;
	}
	return NO;
}
@end

@implementation Bar
- (void) anotherMethod
{
}
+ (void) anotherMethod
{
}
- (id) manyTypes
{
  return @"Hello";
}
- (id) aBool: (BOOL)d andAnInt: (int) w
{
  return @"Hello";
}
@end


void testInvalidArguments()
{
  test(NO == class_conformsToProtocol([NSObject class], NULL));
  test(NO == class_conformsToProtocol(Nil, NULL));
  test(NO == class_conformsToProtocol(Nil, @protocol(NSCoding)));
  test(NULL == class_copyIvarList(Nil, NULL));
  test(NULL == class_copyMethodList(Nil, NULL));
  test(NULL == class_copyPropertyList(Nil, NULL));
  test(NULL == class_copyProtocolList(Nil, NULL));
  test(nil == class_createInstance(Nil, 0));
  test(0 == class_getVersion(Nil));
  test(NO == class_isMetaClass(Nil));
  test(Nil == class_getSuperclass(Nil));

  test(NULL == method_getName(NULL));
  test(NULL == method_copyArgumentType(NULL, 0));
  test(NULL == method_copyReturnType(NULL));
  method_exchangeImplementations(NULL, NULL);
  test((IMP)NULL == method_setImplementation(NULL, (IMP)NULL));
  test((IMP)NULL == method_getImplementation(NULL));
  method_getArgumentType(NULL, 0, NULL, 0);
  test(0 == method_getNumberOfArguments(NULL));
  test(NULL == method_getTypeEncoding(NULL));
  method_getReturnType(NULL, NULL, 0);

  test(NULL == ivar_getName(NULL));
  test(0 == ivar_getOffset(NULL));
  test(NULL == ivar_getTypeEncoding(NULL));

  test(nil == objc_getProtocol(NULL));

  test(stringsEqual("<null selector>", sel_getName((SEL)0)));
  test((SEL)0 == sel_getUid(NULL));
  test(0 != sel_getUid("")); // the empty string is permitted as a selector
  test(stringsEqual("", sel_getName(sel_getUid(""))));
  test(YES == sel_isEqual((SEL)0, (SEL)0));

  //test(NULL == property_getName(NULL));

  printf("testInvalidArguments() ran\n");
}

void testAMethod(Method m)
{
  test(NULL != m);
  test(stringsEqual("aMethod", sel_getName(method_getName(m))));

  printf("testAMethod() ran\n");
}

void testGetMethod()
{
  testAMethod(class_getClassMethod([Bar class], @selector(aMethod)));
  testAMethod(class_getClassMethod([Bar class], sel_getUid("aMethod")));

  printf("testGetMethod() ran\n");
}

void testProtocols()
{
  test(protocol_isEqual(@protocol(NSCoding), objc_getProtocol("NSCoding")));

  printf("testProtocols() ran\n");
}

void testMultiTypedSelector()
{
  test(sel_isEqual(@selector(manyTypes),sel_getUid("manyTypes")));

  Method intMethod = class_getInstanceMethod([Foo class], @selector(manyTypes));
  Method idMethod = class_getInstanceMethod([Bar class], @selector(manyTypes));

  test(sel_isEqual(method_getName(intMethod), @selector(manyTypes)));
  test(sel_isEqual(method_getName(idMethod), @selector(manyTypes)));

  char ret[10];
  method_getReturnType(intMethod, ret, 10);
  test(stringsEqual(ret, "i"));
  method_getReturnType(idMethod, ret, 10);
  test(stringsEqual(ret, "@"));

  printf("testMultiTypedSelector() ran\n");
}

void testClassHierarchy()
{
  Class nsProxy = objc_getClass("NSProxy");
  Class nsObject = objc_getClass("NSObject");
  Class nsProxyMeta = object_getClass(nsProxy);
  Class nsObjectMeta = object_getClass(nsObject);

  test(object_getClass(nsProxyMeta) == nsProxyMeta);
  test(object_getClass(nsObjectMeta) == nsObjectMeta);

  test(Nil == class_getSuperclass(nsProxy));
  test(Nil == class_getSuperclass(nsObject));

  test(nsObject == class_getSuperclass(nsObjectMeta));
  test(nsProxy == class_getSuperclass(nsProxyMeta));
  printf("testClassHierarchy() ran\n");
}

void testAllocateClass()
{
  Class newClass = objc_allocateClassPair(objc_lookUpClass("NSObject"), "UserAllocated", 0);
  test(Nil != newClass);
  // class_getSuperclass() will call objc_resolve_class().
  // Although we have not called objc_registerClassPair() yet, this works with
  // the Apple runtime and GNUstep Base relies on this behavior in
  // GSObjCMakeClass().
  test(objc_lookUpClass("NSObject") == class_getSuperclass(newClass));
  printf("testAllocateClass() ran\n");
}

void testSynchronized()
{
  Foo *foo = [Foo new];
  printf("Enter synchronized code\n");
  [foo synchronizedCode];
  [foo release];
  [Foo shared];
  printf("testSynchronized() ran\n");
}

void testExceptions()
{
  Foo *foo = [Foo new];
  test([foo basicThrowAndCatchException]);
  [foo release];
  printf("testExceptions() ran\n");

}

void testRegisterAlias()
{
  class_registerAlias_np([NSObject class], "AliasObject");
  test([NSObject class] == objc_getClass("AliasObject"));
  printf("testRegisterAlias() ran\n");
}

@interface SlowInit1 : NSObject
+ (void)doNothing;
@end
@interface SlowInit2 : NSObject
+ (void)doNothing;
@end

@implementation SlowInit1
+ (void)initialize
{
	sleep(1);
	[SlowInit2 doNothing];
}
+ (void)doNothing {}
@end
static int initCount;
@implementation SlowInit2
+ (void)initialize
{
	sleep(1);
	__sync_fetch_and_add(&initCount, 1);
}
+ (void)doNothing {}
@end



int main (int argc, const char * argv[])
{
  testInvalidArguments();
  testGetMethod();
  testProtocols();
  testMultiTypedSelector();
  testClassHierarchy();
  testAllocateClass();
  printf("Instance of NSObject: %p\n", class_createInstance([NSObject class], 0));

  testSynchronized();
  testExceptions();
  testRegisterAlias();

  return exitStatus;
}
