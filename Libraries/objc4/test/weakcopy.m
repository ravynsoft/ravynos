// TEST_CFLAGS -fobjc-weak

#include "test.h"

#include "testroot.i"
#include <stdint.h>
#include <string.h>
#include <objc/objc-runtime.h>

@interface Weak : TestRoot {
  @public
    __weak id value;
}
@end
@implementation Weak
@end

Weak *oldObject;
Weak *newObject;

int main()
{
    testonthread(^{
        TestRoot *value;

        PUSH_POOL {
            value = [TestRoot new];
            testassert(value);
            oldObject = [Weak new];
            testassert(oldObject);
            
            oldObject->value = value;
            testassert(oldObject->value == value);
            
            newObject = [oldObject copy];
            testassert(newObject);
            testassert(newObject->value == oldObject->value);
            
            newObject->value = nil;
            testassert(newObject->value == nil);
            testassert(oldObject->value == value);
        } POP_POOL;
        
        testcollect();
        TestRootDealloc = 0;
        RELEASE_VAR(value);
    });

    testcollect();
    testassert(TestRootDealloc);

#if __has_feature(objc_arc_weak)
    testassert(oldObject->value == nil);
#endif
    testassert(newObject->value == nil);

    RELEASE_VAR(newObject);
    RELEASE_VAR(oldObject);

    succeed(__FILE__);
    return 0;
}
