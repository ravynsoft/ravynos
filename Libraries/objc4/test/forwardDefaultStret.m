/*
no arc, rdar://11368528 confused by Foundation
TEST_CONFIG MEM=mrc
TEST_CRASHES
TEST_RUN_OUTPUT
objc\[\d+\]: \+\[NSObject fakeorama\]: unrecognized selector sent to instance 0x[0-9a-fA-F]+ \(no message forward handler is installed\)
objc\[\d+\]: HALTED
END
*/

#include "test.h"

#include <objc/NSObject.h>

@interface NSObject (Fake)
-(struct stret)fakeorama;
@end

int main()
{
    [NSObject fakeorama];
    fail("should have crashed");
}

