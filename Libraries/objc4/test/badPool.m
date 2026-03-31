// TEST_CONFIG MEM=mrc
// TEST_CRASHES

// Test badPoolCompat also uses this file.

/*
TEST_RUN_OUTPUT
objc\[\d+\]: [Ii]nvalid or prematurely-freed autorelease pool 0x[0-9a-fA-F]+\.?
objc\[\d+\]: HALTED
END
*/

#include "test.h"

int main()
{
    void *outer = objc_autoreleasePoolPush();
    void *inner = objc_autoreleasePoolPush();
    objc_autoreleasePoolPop(outer);
    objc_autoreleasePoolPop(inner);

#if !OLD
    fail("should have crashed already with new SDK");
#else
    // should only warn once
    outer = objc_autoreleasePoolPush();
    inner = objc_autoreleasePoolPush();
    objc_autoreleasePoolPop(outer);
    objc_autoreleasePoolPop(inner);

    succeed(__FILE__);
#endif
}

