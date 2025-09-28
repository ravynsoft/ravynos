#include "stdio.h"
#include "Test.h"

@interface T2 : Test @end
@implementation T2 @end

@compatibility_alias Foo T2;

Class alias_getClass(const char*);

int main(void)
{
	assert([T2 class] == [Foo class]);
	assert(objc_getClass("T2") == objc_getClass("Foo"));
	assert(objc_getClass("T2") == alias_getClass("Foo"));
}
