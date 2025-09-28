#include <stdio.h>
#include "Test.h"

@interface DieStrong : Test @end
@interface DieWeak : Test @end
@implementation DieStrong
- (void)dealloc
{
	fprintf(stderr, "Killing strong\n");
	void (^myBlock)() = ^()
	{
		id foo = self;
		fprintf(stderr, "strong self: %p\n", foo);
	};
	myBlock();
}
@end

@implementation DieWeak
- (void)dealloc
{
	fprintf(stderr, "Killing weak\n");
	__weak id this = self;
	void (^myBlock)() = ^()
	{
		id foo = this;
		fprintf(stderr, "weak self: %p\n", foo);
	};
	myBlock();
}
@end

int main(void)
{
	fprintf(stderr, "Test running?\n");
	id a = [DieStrong new], b = [DieWeak new];
	a = nil;
	b = nil;
}
