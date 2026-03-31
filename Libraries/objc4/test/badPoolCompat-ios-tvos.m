// Run test badPool as if it were built with an old SDK.

// TEST_CONFIG MEM=mrc OS=iphoneos,iphonesimulator,appletvos,appletvsimulator
// TEST_CRASHES
// TEST_CFLAGS -DOLD=1 -Xlinker -sdk_version -Xlinker 9.0

/*
TEST_RUN_OUTPUT
objc\[\d+\]: Invalid or prematurely-freed autorelease pool 0x[0-9a-fA-f]+\. Set a breakpoint .* Proceeding anyway .*
OK: badPool.m
END
*/

#include "badPool.m"
