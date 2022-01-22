#include "Test.h"

#import <stdatomic.h>

@interface Dummy : Test
{
	atomic_bool atomicBool;
}
@end

@implementation Dummy
- (void)test
{
  int value = 1;
  object_setIvar(self, class_getInstanceVariable(object_getClass(self), "atomicBool"), (__bridge id)(void*)(intptr_t)value);
}
@end


int main(int argc, char *argv[])
{
	[[Dummy new] test];
	return 0;
}
