// fixme rdar://24624435 duplicate class warning fails with the shared cache
// OBJC_DISABLE_PREOPTIMIZATION=YES works around that problem.

// TEST_ENV OBJC_DEBUG_DUPLICATE_CLASSES=YES OBJC_DISABLE_PREOPTIMIZATION=YES
// TEST_CRASHES
/* 
TEST_RUN_OUTPUT
objc\[\d+\]: Class [^\s]+ is implemented in both .+ \(0x[0-9a-f]+\) and .+ \(0x[0-9a-f]+\)\. One of the two will be used\. Which one is undefined\.
objc\[\d+\]: HALTED
OR
OK: duplicatedClasses.m
END
 */

#include "test.h"
#include "testroot.i"

@interface WKWebView : TestRoot @end
@implementation WKWebView @end

int main()
{
    void *dl = dlopen("/System/Library/Frameworks/WebKit.framework/WebKit", RTLD_LAZY);
    if (!dl) fail("couldn't open WebKit");
    fail("should have crashed already");
}
