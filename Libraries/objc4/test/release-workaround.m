// TEST_CONFIG ARCH=x86_64 MEM=mrc
// TEST_CFLAGS -framework Foundation

// rdar://20206767

#include <Foundation/Foundation.h>
#include "test.h"


@interface Test : NSObject @end
@implementation Test
@end


int main()
{
    id buf[1];
    buf[0] = [Test class];
    id obj = (id)buf;
    [obj retain];
    [obj retain];

    uintptr_t rax;

    [obj release];
    asm("mov %%rax, %0" : "=r" (rax));
    testassert(rax == 0);

    objc_release(obj);
    asm("mov %%rax, %0" : "=r" (rax));
    testassert(rax == 0);

    succeed(__FILE__);
}
