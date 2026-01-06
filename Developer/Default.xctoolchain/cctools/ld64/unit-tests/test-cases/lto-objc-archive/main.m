
#include <Foundation/Foundation.h>
#include "foo.h"
#include "bar.h"


@interface FooSubClass : Foo
@end


@implementation FooSubClass
@end


int main()
{
	[Bar alloc];
	return 0;
}

