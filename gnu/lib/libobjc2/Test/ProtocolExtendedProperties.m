#include "Test.h"
#include <string.h>

@class NSString;
@protocol Foo
- (NSString*)aMethod: (void(^)(int))aBlock;
@end

int main(void)
{
	const char *encoding = _protocol_getMethodTypeEncoding(@protocol(Foo), @selector(aMethod:), YES, YES);
#ifdef GS_RUNTIME_V2
	// We expect something like this (LP64): @"NSString"24@0:8@?<v@?i>16
	assert(strstr(encoding, "@\"NSString\"") == encoding);
	assert(strstr(encoding, "@?<v@?i>") != NULL);
#else
	assert(strstr(encoding, "@?") != NULL);
#endif
}
