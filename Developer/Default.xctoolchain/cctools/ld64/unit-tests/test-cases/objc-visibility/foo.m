
#include <Foundation/Foundation.h>

#include "foo.h"
#include "bar.h"

@implementation Foo
- (NSString*) foo
{
	[Bar alloc];
	return [NSString stringWithUTF8String:"hello"];
}
@end

