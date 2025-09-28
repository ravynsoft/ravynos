#define _GNU_SOURCE
#include "../unwind.h"
#include "Test.h"
#include "../objc/hooks.h"
#include <stdlib.h>


struct foreign_exception
{
	struct _Unwind_Exception header;
	int x;
};

BOOL finally_called = NO;


int throw(void)
{
	struct foreign_exception *foreign_exception = calloc(sizeof(struct foreign_exception), 1);
	foreign_exception->header.exception_class = 42;
	foreign_exception->x = 12;
	_Unwind_RaiseException(&foreign_exception->header);
	assert(0);
}

void finally(void)
{
	@try
	{
		throw();
	}
	@finally
	{
		finally_called = YES;
	}
	finally_called = NO;
}
@interface BoxedException : Test
{
	struct foreign_exception *exception;
}
- (int)value;
@end
@implementation BoxedException
+ (id) exceptionWithForeignException: (struct _Unwind_Exception*)ex
{
	BoxedException *b = [BoxedException new];
	b->exception = (struct foreign_exception*)ex;
	return b;
}
- (void)dealloc
{
	free(exception);
	[super dealloc];
}
- (int)value
{
	if (exception)
	{
		return exception->x;
	}
	return -1;
}
- (void)rethrow
{
	struct _Unwind_Exception *ex = &exception->header;
	exception = 0;
	[self dealloc];
	_Unwind_Resume_or_Rethrow(ex);
	abort();
}
@end


Class boxer(int64_t class)
{
	assert(class == 42);
	return [BoxedException class];
}

int main(void)
{
	_objc_class_for_boxing_foreign_exception = boxer;
	BOOL catchall = NO;
	BOOL catchboxed = NO;
	@try
	{
		finally();
	}
	@catch (BoxedException *x)
	{
		assert(x != nil);
		assert([x value] == 12);
		[x dealloc];
		catchboxed = YES;
	}
	@catch(...)
	{
		catchall = YES;
	}
	assert(finally_called == YES);
	assert(catchall == NO);
	assert(catchboxed == YES);
	return 0;
}
