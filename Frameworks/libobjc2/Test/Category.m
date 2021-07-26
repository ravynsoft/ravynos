#include "Test.h"
#include "../objc/runtime.h"

#pragma clang diagnostic ignored "-Wobjc-protocol-method-implementation"
@interface Foo : Test
+ (int)replaced;
@end
@implementation Foo
+ (int)replaced
{
	return 1;
}
@end

@implementation Foo (bar)
+ (int)replaced
{
	return 2;
}
@end

int main (void)
{
	assert([Foo replaced] == 2);
	return 0;
}
