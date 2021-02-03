#include "Test.h"
#include <stdio.h>

static BOOL called;

@interface AllUnsafe : Test @end
@implementation AllUnsafe
- (id)retain
{
	return self;
}
- (void)release {}
- (id)autorelease
{
	return self;
}
@end

@interface Retain : AllUnsafe @end
@implementation Retain
- (id)retain
{
	called = YES;
	return self;
}
@end

@interface RetainSafe : AllUnsafe @end
@implementation RetainSafe
- (id)retain
{
	return self;
}
- (void)_ARCCompliantRetainRelease {}
@end

@interface Release : AllUnsafe @end
@implementation Release
- (void)release
{
	called = YES;
}
@end

@interface ReleaseSafe : AllUnsafe @end
@implementation ReleaseSafe
- (void)release
{
}
- (void)_ARCCompliantRetainRelease {}
@end

@interface Autorelease : AllUnsafe @end
@implementation Autorelease
- (id)autorelease
{
	called = YES;
	return self;
}
@end

@interface AutoreleaseSafe : AllUnsafe @end
@implementation AutoreleaseSafe
- (id)autorelease
{
	return self;
}
- (void)_ARCCompliantRetainRelease {}
@end

void check(id obj, BOOL expected)
{
	fprintf(stderr, "Checking %s\n", class_getName(object_getClass(obj)));
}

int main()
{
	called = NO;
	objc_retain([Retain new]);
	assert(called == YES);

	called = NO;
	objc_retain([RetainSafe new]);
	assert(called == NO);

	called = NO;
	objc_release([Release new]);
	assert(called == YES);

	called = NO;
	objc_release([ReleaseSafe new]);
	assert(called == NO);

	called = NO;
	objc_autorelease([Autorelease new]);
	assert(called == YES);

	called = NO;
	objc_autorelease([AutoreleaseSafe new]);
	assert(called == NO);

	return 0;
}

