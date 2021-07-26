#include "../objc/runtime.h"
#include "../objc/blocks_runtime.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

struct big
{
	int a, b, c, d, e;
};

#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface Foo @end
@implementation Foo @end
@interface Foo (Dynamic)
+(int)count: (int)i;
+(struct big)sret;
@end


int main(void)
{
	__block Class cls = objc_getClass("Foo");
	__block int b = 0;
	void* blk = ^(id self, int a) {
		assert(self == cls);
		b += a; 
		return b; };
	blk = Block_copy(blk);
	IMP imp = imp_implementationWithBlock(blk);
	char *type = block_copyIMPTypeEncoding_np(blk);
	assert(NULL != type);
	class_addMethod((objc_getMetaClass("Foo")), @selector(count:), imp, type);
	assert(2 == ((int(*)(id,SEL,int))imp)(cls, @selector(count:), 2));
	free(type);
	assert(4 == [Foo count: 2]);
	assert(6 == [Foo count: 2]);
	assert(imp_getBlock(imp) == (blk));
	IMP imp2 = imp_implementationWithBlock(blk);
	assert(imp != imp2);
	imp_removeBlock(imp);
	assert(imp_getBlock(imp) != (blk));

	blk = ^(id self) {
		assert(self == cls);
		struct big b = {1, 2, 3, 4, 5};
		return b;
	};
	imp = imp_implementationWithBlock(blk);
	assert(imp && "Can't make sret IMP");
	type = block_copyIMPTypeEncoding_np(blk);
	assert(NULL != type);
	class_addMethod((objc_getMetaClass("Foo")), @selector(sret), imp, type);
	free(type);
	struct big s = [Foo sret];
	assert(s.a == 1);
	assert(s.b == 2);
	assert(s.c == 3);
	assert(s.d == 4);
	assert(s.e == 5);
	return 0;
}
