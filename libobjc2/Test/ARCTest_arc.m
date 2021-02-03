#import "Test.h"

// Checks using non-portable APIs
#ifdef GNUSTEP_RUNTIME
void check_retain_count(id obj, size_t rc)
{
	assert(object_getRetainCount_np(obj) == rc);
}
void check_autorelease_count(id obj, size_t rc)
{
	assert(objc_arc_autorelease_count_for_object_np(obj) == rc);
}
#else
void check_retain_count(id obj, size_t rc)
{
}
void check_autorelease_count(id obj, size_t rc)
{
}
#endif

id __weak var;

@interface ARC : Test @end
@implementation ARC
- (id __autoreleasing)loadWeakAutoreleasing
{
	return var;
}
- (id)loadWeak
{
	return var;
}
- (void)setWeakFromWeak: (id __weak)anObject
{
	var = anObject;
	anObject = nil;
}
- (void)setWeak: (id)anObject
{
	var = anObject;
}
@end

@interface CheckDealloc : Test
@end
@implementation CheckDealloc
{
	BOOL *flag;
}
- (id)initWithFlag: (BOOL*)aFlag
{
	flag = aFlag;
	*flag = NO;
	return self;
}
- (void)dealloc
{
	*flag = YES;
}
@end


int main(void)
{
	ARC *obj = [ARC new];
	BOOL f1;
	BOOL f2;
	// Check that storing weak references works.
	{
		id o1 = [[CheckDealloc new] initWithFlag: &f1];
		id o2 = [[CheckDealloc new] initWithFlag: &f2];
		[obj setWeak: o1];
		assert([obj loadWeak] == o1);
		[obj setWeakFromWeak: o2];
		assert([obj loadWeak] == o2);
		@autoreleasepool
		{
			id __autoreleasing o2a = [obj loadWeakAutoreleasing];
			assert(o2a == o2);
		}
	}
	assert(f1);
	assert(f2);
	assert([obj loadWeak] == nil);
	@autoreleasepool
	{
		id __autoreleasing o1a;
		{
			id o1 = [[CheckDealloc new] initWithFlag: &f1];
			assert(!f1);
			[obj setWeak: o1];
			assert([obj loadWeak] == o1);
			o1a = [obj loadWeakAutoreleasing];
			assert(o1a == o1);
			check_autorelease_count(o1a, 1);
			check_retain_count(o1a, 1);
		}
		assert(o1a == [obj loadWeak]);
	}
	assert(f1);
	assert([obj loadWeak] == nil);
	// Try to trigger an objc_moveWeak call
	{
		id o1 = [Test new];
		{
			id __weak x = o1;
			var = x;
		}
	}
	assert([obj loadWeak] == nil);
	// Now check what happens with a constant string in a weak variable.
	{
		id x = @"foo";
		[obj setWeak: x];
	}
	assert([obj loadWeak] != nil);
	return 0;
}
