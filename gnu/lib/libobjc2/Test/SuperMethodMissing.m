#include "Test.h"
#include "objc/hooks.h"
#include <stdio.h>

@interface Test (DoesNotExist)
- (void)run;
@end

@interface Foo : Test
@end

@implementation Foo
- (void)run
{
	[super run];
}
@end

static int missing_methods;

id forward(id self, SEL cmd, ...)
{
	Class cls = object_getClass(self);
	missing_methods++;
	fprintf(stderr, "Missing method:  %c[%s %s]\n", class_isMetaClass(cls) ? '+' : '-', class_getName(cls),sel_getName(cmd));
	return nil;
}

IMP no_method(id self, SEL cmd)
{
	return forward;
}


int
main()
{
	__objc_msg_forward2 = no_method;
	Test *t = [Test new];
	[t run];
	assert(missing_methods == 1);
	Foo *f = [Foo new];
	[f run];
	assert(missing_methods == 2);
	//[Test run];
}
