#include "Test.h"
#include "../unwind.h"

#if __cplusplus
#error This is not an ObjC++ test!
#endif

struct
{
	struct _Unwind_Exception header;
	id x;
} foreign_exception;

BOOL cleanup_called = NO;
BOOL finally_called = NO;

static void cleanup(_Unwind_Reason_Code i,struct _Unwind_Exception *e)
{
	assert(e == &foreign_exception.header);
	cleanup_called = YES;
}

int throw(void)
{
	foreign_exception.header.exception_class = 42;
	foreign_exception.header.exception_cleanup = cleanup;
	foreign_exception.x = (id)12;
	_Unwind_RaiseException(&foreign_exception.header);
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


int main(void)
{
	BOOL catchall = NO;
	@try
	{
		finally();
	}
	@catch (id x)
	{
		assert(0);
	}
	@catch(...)
	{
		catchall = YES;
	}
	assert(finally_called == YES);
	assert(catchall == YES);
	assert(cleanup_called == YES);
	return 0;
}
