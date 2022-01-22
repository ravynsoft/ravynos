#include "Test.h"
#include <stdio.h>

// Regression test for a bug where allocating a class as a subclass of an
// unresolved class failed.

static int loaded;

static void load(Class self, SEL _cmd)
{
	loaded++;
}

int main()
{
	Class a, b, c, d, e;

	assert(class_getInstanceSize(objc_allocateClassPair(Nil, "Empty", 0)) == sizeof(Class));
	a = objc_allocateClassPair([Test class], "A", 0);
	objc_registerClassPair(a);

	b = objc_allocateClassPair(a, "B", 0);
	class_addMethod(object_getClass(b), @selector(load), (IMP)load, "@:");

	class_addIvar(b, "anIvar", 4, 2, "i");
	objc_registerClassPair(b);

	Ivar iv = class_getInstanceVariable(b, "anIvar");
	size_t superSize = class_getInstanceSize([Test class]);
	assert(ivar_getOffset(iv) == superSize);

	class_getSuperclass(b);


	c = objc_allocateClassPair(b, "C", 0);
	objc_registerClassPair(c);
	d = objc_allocateClassPair(c, "D", 0);
	objc_registerClassPair(d);
	e = objc_allocateClassPair(d, "E", 0);
	objc_registerClassPair(e);
	assert(loaded == 0);
	assert(objc_getClass("C") == c);
	assert(objc_getClass("D") == d);
	assert(objc_getClass("E") == e);
	objc_disposeClassPair(e);
	assert(objc_getClass("E") == nil);

	return 0;
}


