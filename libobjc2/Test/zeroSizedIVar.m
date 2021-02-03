#include <stdio.h>
#include <stdlib.h>
#include "Test.h"


typedef uintptr_t NSUInteger;

@interface NSArray : Test
{
    NSUInteger count;
    id objects[0];
}
@end

@implementation NSArray @end

@interface BitfieldTest : Test
{
	BOOL flag1:1;
	BOOL flag2:1;
	BOOL flag3:1;
}
@end

@implementation BitfieldTest @end

@interface BitfieldTest2 : Test
{
	BOOL flag1:1;
	BOOL flag2:1;
	BOOL flag3:1;
	int x;
}
@end

@implementation BitfieldTest2 @end


int main()
{
	Class nsarray = objc_getClass("NSArray");
	assert(nsarray);
	assert(class_getInstanceSize(nsarray) == (sizeof(Class) + sizeof(NSUInteger)));
	Ivar count = class_getInstanceVariable(nsarray, "count");
	assert(ivar_getOffset(count) == sizeof(id));

	Class bitfield = objc_getClass("BitfieldTest");
	assert(bitfield);
	Ivar flag1 = class_getInstanceVariable(bitfield, "flag1");
	assert(flag1);
	assert(ivar_getOffset(flag1) == sizeof(id));
	Ivar flag2 = class_getInstanceVariable(bitfield, "flag2");
	assert(flag2);
	assert(ivar_getOffset(flag2) == sizeof(id));
	Ivar flag3 = class_getInstanceVariable(bitfield, "flag3");
	assert(flag3);
	assert(ivar_getOffset(flag3) == sizeof(id));
	assert(ivar_getOffset(flag3) + sizeof(BOOL) <= class_getInstanceSize(bitfield));

	bitfield = objc_getClass("BitfieldTest2");
	flag1 = class_getInstanceVariable(bitfield, "flag1");
	flag3 = class_getInstanceVariable(bitfield, "flag3");
	Ivar x = class_getInstanceVariable(bitfield, "x");
	assert(ivar_getOffset(flag1) == ivar_getOffset(flag3));
	assert(ivar_getOffset(x) > ivar_getOffset(flag3));
	assert(ivar_getOffset(x) + sizeof(int) <= class_getInstanceSize(bitfield));
}

