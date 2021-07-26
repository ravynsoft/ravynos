#include "Test.h"

#define SIZE 5000

int main(int argc, const char * argv[])
{    
	@autoreleasepool {
		id __weak refs[SIZE];
		id values[SIZE];

		// Setup
		for (int i=0; i<SIZE; i++)
		{
			values[i] = [Test new];
			refs[i] = values[i];	
		}

		// Sanity check
		for (int i=0; i<SIZE; i++)
		{
			assert(refs[i] != nil);
			assert(refs[i] == values[i]);
		}

		// Release the value, one by one
		for (int indexToRelease=0; indexToRelease<SIZE; indexToRelease++)
		{
			values[indexToRelease] = nil;
			// Check all refs
			for (int i=0; i<SIZE; i++)
			{
				if (i <= indexToRelease)
				{
					assert(refs[i] == nil);
				}
				else
				{
					assert(refs[i] != nil);
					assert(refs[i] == values[i]);
				}
			}
		}
	}
	return 0;
}
