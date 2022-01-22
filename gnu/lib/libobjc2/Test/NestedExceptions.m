#include "Test.h"

#if __cplusplus
#error This is not an ObjC++ test!
#endif


id a;
int throw(void)
{
	@throw a;
}


int main(void)
{
	id e1 = [Test new];
	id e2 = [Test new];
	@try
	{
		a = e1;
		throw();
	}
	@catch (id x)
	{
		assert(x == e1);
		@try {
			a = e2;
			@throw a;
		}
		@catch (id y)
		{
			assert(y == e2);
		}
	}
	[e1 dealloc];
	[e2 dealloc];
	return 0;
}
