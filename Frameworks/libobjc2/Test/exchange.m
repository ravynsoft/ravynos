#include "Test.h"

@interface Exchange : Test
+ (int)test1;
+ (int)test2;
@end

@implementation Exchange
+ (void)noop { }

+ (int)test1 { return 1024; }
+ (int)test2 { return 2048; }
@end

int main(int argc, char** argv) {
	[Exchange noop];
	Class i32meta = object_getClass(objc_getClass("Exchange"));
	Method m1 = class_getInstanceMethod(i32meta, @selector(test1));
	Method m2 = class_getInstanceMethod(i32meta, @selector(test2));
	method_exchangeImplementations(m1, m2);
	assert(2048 == [Exchange test1]);
}

