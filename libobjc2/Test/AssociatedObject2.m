#include "Test.h"

@interface MLTestClass : Test {
@public
}
- (void)someF;
@end

@implementation MLTestClass
- (void)someF
{
}

@end

static void ff(id obj, SEL _cmd)
{
}


int main()
{
	static char static_char;
	MLTestClass * tc;
	tc =  [MLTestClass new];
	objc_setAssociatedObject(tc, &static_char, (id)1223, OBJC_ASSOCIATION_ASSIGN);
	[tc release];
	tc =  [MLTestClass new];
	objc_setAssociatedObject(tc, &static_char, (id)1223, OBJC_ASSOCIATION_ASSIGN);
	SEL some_sel = sel_registerName(".some_sel");
	const char *types = "v@:";
	class_addMethod(object_getClass(tc), some_sel,
			(IMP)ff, types);
	int j = (int)objc_getAssociatedObject(tc, &static_char); 
	assert(j == 1223);
	[tc release];
}
