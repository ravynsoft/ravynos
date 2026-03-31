// TEST_CRASHES
/* 
TEST_RUN_OUTPUT
objc\[\d+\]: Memory corruption in class list\.
objc\[\d+\]: HALTED
END
*/

#include "test.h"
#include "testroot.i"

@interface Super : TestRoot @end
@implementation Super @end

@interface Sub : Super @end
@implementation Sub @end

int main()
{
    alarm(10);
    
    Class supercls = [Super class];
    Class subcls = [Sub class];
    id subobj __unused = [Sub alloc];

    // Create a cycle in a superclass chain (Sub->supercls == Sub)
    // then attempt to walk that chain. Runtime should halt eventually.
    _objc_flush_caches(supercls);
    ((Class *)(__bridge void *)subcls)[1] = subcls;
#ifdef CACHE_FLUSH
    _objc_flush_caches(supercls);
#else
    [subobj class];
#endif
    
    fail("should have crashed");
}
