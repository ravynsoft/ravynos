#include "stdio.h"
#include "Test.h"

// This is a large vector type, which the compiler will lower to some sequence
// of vector ops on the target, or scalar ops if there is no vector FPU.
typedef double __attribute__((vector_size(32))) v4d;

@interface X : Test
{
	id f;
	id g;
}
@end
@implementation X @end

@interface Vector : X
{
	v4d x;
}
@end
@implementation Vector
+ (Vector*)alloc
{
	Vector *v = class_createInstance(self, 0);
	// The initialisation might be done with memset, but will probably be a
	// vector load / store and so will likely fail if x is incorrectly aligned.
	v->x = (v4d){1,2,3,4};
	return v;
}
- (void)permute
{
	// This will become a sequence of one or more vector operations.  We must
	// have the correct alignment for x, even after the instance variable
	// munging, or this will break.
	x *= (v4d){2,3,4,5};
}
@end

typedef int v4si __attribute__ ((vector_size (16)));
@interface Foo : Test
{
			v4si var;
}
- (void)check;
@end
@implementation Foo
- (void)check
{
	size_t addr = (size_t)&var;
	fprintf(stderr, "self: %p Addr: %p\n", self, &var);
	assert(addr % __alignof__(v4si) == 0);
}
@end

#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
@interface StringLikeTest
{
  Class isa;
  char* c_string;
  int len;
}
@end

@implementation StringLikeTest
+ (Class)class
{
  return self;
}
@end

int main(void)
{
	[[Vector alloc] permute];
	[[Foo new] check];

	Ivar v_isa = class_getInstanceVariable([StringLikeTest class], "isa");
	Ivar v_c_string = class_getInstanceVariable([StringLikeTest class], "c_string");
	Ivar v_len = class_getInstanceVariable([StringLikeTest class], "len");
	ptrdiff_t o_isa = ivar_getOffset(v_isa);
	ptrdiff_t o_c_string = ivar_getOffset(v_c_string);
	assert(o_isa == 0);
	assert(o_c_string == sizeof(Class));
	assert(o_isa < o_c_string);
	assert(o_c_string < ivar_getOffset(v_len));
}
