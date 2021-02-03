#include "Test.h"

#define SIZE 5000

int main(int argc, const char * argv[])
{    
	id t = [Test new];
	id w1;
	id w2;
	objc_initWeak(&w1, t);
	objc_initWeak(&w2, t);
	[t release];
	assert(objc_loadWeakRetained(&w1) == nil);
	assert(objc_loadWeakRetained(&w2) == nil);
	assert(w1 == nil);
	assert(w2 == nil);
	assert(objc_loadWeakRetained(&w1) == nil);
	assert(objc_loadWeakRetained(&w2) == nil);
	assert(w1 == nil);
	assert(w2 == nil);
	return 0;
}
