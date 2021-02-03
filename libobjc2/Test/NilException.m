#include "Test.h"


#ifdef __has_attribute
#if __has_attribute(objc_root_class)
__attribute__((objc_root_class))
#endif
#endif
@interface NSObject
{
  Class isa;
}
@end

@implementation NSObject
+ (id)new
{
	return class_createInstance(self, 0);
}
@end
int main(void)
{
	BOOL caught_exception = NO;
	@try
	{
		@throw(nil);
	}
	@catch (NSObject* o)
	{
		assert(0);
	}
	@catch (id x)
	{
		assert(nil == x);
		caught_exception = YES;
	}
	assert(caught_exception == YES);
	caught_exception = NO;
	@try
	{
		@throw(nil);
	}
	@catch (...)
	{
		caught_exception = YES;
	}
	assert(caught_exception == YES);
	return 0;
}
