#include "Test.h"

int main(int argc, const char * argv[])
{
	id __weak ref;
	@autoreleasepool {
		__block int val;
		id block = ^() { return val++; };
		assert(block != nil);
		ref = block;
		assert(ref != nil);
	}
	assert(ref == nil);
	return 0;
}
