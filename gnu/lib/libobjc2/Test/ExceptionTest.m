#include "Test.h"

#if __cplusplus
#error This is not an ObjC++ test!
#endif

BOOL finallyEntered = NO;
BOOL cleanupRun = NO;
BOOL idRethrown = NO;
BOOL catchallRethrown = NO;
BOOL testCaught = NO;
BOOL wrongMatch = NO;

@interface NSString : Test @end
void runCleanup(void *x)
{
	assert(cleanupRun == NO);
	cleanupRun = YES;
}

int throw(void)
{
	@throw [Test new];
}

int finally(void)
{
	__attribute__((cleanup(runCleanup)))
	int x;
	(void)x;
	@try { throw(); }
	@finally  { finallyEntered = YES; }
	return 0;
}
int rethrow_id(void)
{
	@try { finally(); }
	@catch(id x)
	{
		assert(object_getClass(x) == [Test class]);
		idRethrown = YES;
		@throw;
	}
	return 0;
}
int rethrow_test(void)
{
	@try { rethrow_id(); }
	@catch (Test *t)
	{
		testCaught = YES;
		@throw;
	}
	@catch (id x)
	{
		assert(0 && "should not be reached!");
	}
	@catch (...)
	{
		assert(0 && "should not be reached!");
	}
}
int rethrow_catchall(void)
{
	@try { rethrow_test(); }
	@catch(...)
	{
		assert(testCaught);
		catchallRethrown = YES;
		@throw;
	}
	return 0;
}
int not_matched_catch(void)
{
	@try { rethrow_catchall(); }
	@catch(NSString *s)
	{
		wrongMatch = YES;
	}
	return 0;
}

int main(void)
{
	@try
	{
		rethrow_catchall();
	}
	@catch (id x)
	{
		assert(finallyEntered == YES);
		assert(cleanupRun == YES);
		assert(idRethrown == YES);
		assert(catchallRethrown == YES);
		assert(wrongMatch == NO);
		assert(object_getClass(x) == [Test class]);
		[x dealloc];
	}
	return 0;
}
