#include "Test.h"
#include <stdio.h>
#include <inttypes.h>

BOOL deallocCalled = NO;
static const char* objc_setAssociatedObjectKey = "objc_setAssociatedObjectKey";

@interface Associated : Test
@end

@implementation Associated
-(void) dealloc
{
    deallocCalled = YES;
    [super dealloc];
}
@end

int main(void)
{
	@autoreleasepool {
		Associated *object = [Associated new];
		Test *holder = [[Test new] autorelease];
		objc_setAssociatedObject(holder, &objc_setAssociatedObjectKey, object, OBJC_ASSOCIATION_RETAIN);
		[object release];
		assert(!deallocCalled);
	}
	// dealloc should be called when holder is released during pool drain
	assert(deallocCalled);

	deallocCalled = NO;

	Associated *object = [Associated new];
	Test *holder = [Test new];
	objc_setAssociatedObject(holder, &objc_setAssociatedObjectKey, object, OBJC_ASSOCIATION_RETAIN);
	[object release]; // commuted into associated object storage
	objc_setAssociatedObject(holder, &objc_setAssociatedObjectKey, nil, OBJC_ASSOCIATION_ASSIGN);
	[holder release];

	assert(deallocCalled);

	object = [Associated new];
	holder = [Test new];
	for (uintptr_t i = 1; i <= 20; ++i)
	{
		objc_setAssociatedObject(holder, (void*)i, object, OBJC_ASSOCIATION_RETAIN);
	}
	int lost = 0;
	for (uintptr_t i = 1; i <= 20; ++i)
	{
		if (object != objc_getAssociatedObject(holder, (const void*)i))
		{
			fprintf(stderr, "lost object %" PRIuPTR "\n", i);
			++lost;
		}
	}
	[holder release];
	[object release];
	assert(0 == lost);
	return 0;
}
