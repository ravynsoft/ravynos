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

BOOL finally_called = NO;

id e1;
void throw_id(void)
{
	@throw e1;
}

void throw_int(void);
int catchall(void);


void finally(void)
{
	@try
	{
		throw_int();
	}
	@finally
	{
		finally_called = YES;
	}
	finally_called = NO;
}


int main(void)
{
	BOOL catchall_entered = NO;
	BOOL catchid = YES;
	e1 = [Test new];
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
		catchall_entered = YES;
	}
	assert(finally_called == YES);
	assert(catchall_entered == YES);
	@try
	{
		catchall();
	}
	@catch (id x)
	{
		assert(x == e1);
	}
	assert(catchid == YES);
	[e1 dealloc];
	return 0;
}
