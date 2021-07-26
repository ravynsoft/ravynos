#include "Test.h"


__attribute__((weak_import))
@interface WeakClass
- (id)class;
@end

int main(void)
{
	assert([WeakClass class] == nil);
}
