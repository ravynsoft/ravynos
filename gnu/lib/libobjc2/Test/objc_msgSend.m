// Needed with glibc to expose vasprintf
#define _GNU_SOURCE
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include "../objc/runtime.h"
#include "../objc/hooks.h"

//#define assert(x) if (!(x)) { printf("Failed %d\n", __LINE__); }


typedef struct { int a,b,c,d,e; } s;
@interface Fake
- (int)izero;
- (float)fzero;
- (double)dzero;
- (long double)ldzero;
@end

Class TestCls;
#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface MsgTest { id isa; } @end
@interface MsgTest (Dynamic)
+ (void)manyArgs: (int)a0
         : (int) a1
         : (int) a2
         : (int) a3
         : (int) a4
         : (int) a5
         : (int) a6
         : (int) a7
         : (int) a8
         : (int) a9
         : (int) a10
         : (float) f0
         : (float) f1
         : (float) f2
         : (float) f3
         : (float) f4
         : (float) f5
         : (float) f6
         : (float) f7
         : (float) f8
         : (float) f9
         : (float) f10;
@end
@implementation MsgTest 
- foo
{
	assert((id)1 == self);
	assert(strcmp("foo", sel_getName(_cmd)) == 0);
	return (id)0x42;
}
+ foo
{
	assert(TestCls == self);
	assert(strcmp("foo", sel_getName(_cmd)) == 0);
	return (id)0x42;
}
+ (s)sret
{
	assert(TestCls == self);
	assert(strcmp("sret", sel_getName(_cmd)) == 0);
	s st = {1,2,3,4,5};
	return st;
}
- (s)sret
{
	assert((id)3 == self);
	assert(strcmp("sret", sel_getName(_cmd)) == 0);
	s st = {1,2,3,4,5};
	return st;
}
+ (void)printf: (const char*)str, ...
{
	va_list ap;
	char s[100];

	va_start(ap, str);

	vsnprintf(s, 100, str, ap);
	va_end(ap);
	assert(strcmp(s, "Format string 42 42.000000\n") ==0);
}
+ (void)initialize
{
	[self printf: "Format %s %d %f%c", "string", 42, 42.0, '\n'];
	@throw self;
}
+ nothing { return 0; }
@end

int forwardcalls;
void fwdMany(id self,
             SEL _cmd,
             int a0,
             int a1,
             int a2,
             int a3,
             int a4,
             int a5,
             int a6,
             int a7,
             int a8,
             int a9,
             int a10,
             float f0,
             float f1,
             float f2,
             float f3,
             float f4,
             float f5,
             float f6,
             float f7,
             float f8,
             float f9,
             float f10)
{
	forwardcalls++;
	assert(self == objc_getClass("MsgTest"));
	if (sel_isEqual(_cmd, sel_registerName("manyArgs:::::::::::::::::::::")))
	assert(a0 == 0);
	assert(a1 == 1);
	assert(a2 == 2);
	assert(a3 == 3);
	assert(a4 == 4);
	assert(a5 == 5);
	assert(a6 == 6);
	assert(a7 == 7);
	assert(a8 == 8);
	assert(a9 == 9);
	assert(a10 == 10);
	assert(f0 == 0);
	assert(f1 == 1);
	assert(f2 == 2);
	assert(f3 == 3);
	assert(f4 == 4);
	assert(f5 == 5);
	assert(f6 == 6);
	assert(f7 == 7);
	assert(f8 == 8);
	assert(f9 == 9);
	assert(f10 == 10);
}

void fwd(void)
{
	forwardcalls++;
}

IMP forward(id o, SEL s)
{
	assert(o == objc_getClass("MsgTest"));
	if (sel_isEqual(s, sel_registerName("missing")))
	{
		return (IMP)fwd;
	}
	return (IMP)fwdMany;
}

static struct objc_slot slot;
struct objc_slot *forward_slot(id o, SEL s)
{
	slot.method = (IMP)fwd;
	return &slot;
}



int main(void)
{
	__objc_msg_forward2 = forward;
	__objc_msg_forward3 = forward_slot;
	TestCls = objc_getClass("MsgTest");
	int exceptionThrown = 0;
	@try {
		objc_msgSend(TestCls, @selector(foo));
	} @catch (id e)
	{
		assert((TestCls == e) && "Exceptions propagate out of +initialize");
		exceptionThrown = 1;
	}
	assert(exceptionThrown && "An exception was thrown");
	assert((id)0x42 == objc_msgSend(TestCls, @selector(foo)));
	objc_msgSend(TestCls, @selector(nothing));
	objc_msgSend(TestCls, @selector(missing));
	assert(forwardcalls == 1);
	assert(0 == objc_msgSend(0, @selector(nothing)));
	id a = objc_msgSend(objc_getClass("MsgTest"), @selector(foo));
	assert((id)0x42 == a);
	a = objc_msgSend(TestCls, @selector(foo));
	assert((id)0x42 == a);
	assert(objc_registerSmallObjectClass_np(objc_getClass("MsgTest"), 1));
	a = objc_msgSend((id)01, @selector(foo));
	assert((id)0x42 == a);
	s ret = ((s(*)(id, SEL))objc_msgSend_stret)(TestCls, @selector(sret));
	assert(ret.a == 1);
	assert(ret.b == 2);
	assert(ret.c == 3);
	assert(ret.d == 4);
	assert(ret.e == 5);
	if (sizeof(id) == 8)
	{
		assert(objc_registerSmallObjectClass_np(objc_getClass("MsgTest"), 3));
		ret = ((s(*)(id, SEL))objc_msgSend_stret)((id)3, @selector(sret));
		assert(ret.a == 1);
		assert(ret.b == 2);
		assert(ret.c == 3);
		assert(ret.d == 4);
		assert(ret.e == 5);
	}
	Fake *f = nil;
	assert(0 == [f izero]);
	assert(0 == [f dzero]);
	assert(0 == [f ldzero]);
	assert(0 == [f fzero]);
	[TestCls manyArgs: 0 : 1 : 2 : 3: 4: 5: 6: 7: 8: 9: 10 : 0 : 1 : 2 : 3 : 4 : 5 : 6 : 7 : 8 : 9 : 10];
	assert(forwardcalls == 2);
#ifdef BENCHMARK
	const int iterations = 1000000000;
	double times[3];
	clock_t c1, c2;
	c1 = clock();
	for (int i=0 ; i<iterations ; i++)
	{
		[TestCls nothing];
	}
	c2 = clock();
	times[0] = ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "Traditional message send took %f seconds. \n", times[0]);
	c1 = clock();
	for (int i=0 ; i<iterations ; i++)
	{
		objc_msgSend(TestCls, @selector(nothing));
	}
	c2 = clock();
	times[1] = ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "objc_msgSend() message send took %f seconds. \n", times[1]);
	IMP nothing = objc_msg_lookup(TestCls, @selector(nothing));
	c1 = clock();
	for (int i=0 ; i<iterations ; i++)
	{
		nothing(TestCls, @selector(nothing));
	}
	c2 = clock();
	times[2] = ((double)c2 - (double)c1) / (double)CLOCKS_PER_SEC;
	fprintf(stderr, "Direct IMP call took %f seconds. \n", times[2]);
	printf("%f\t%f\t%f\n", times[0], times[1], times[2]);
#endif
	return 0;
}
