// TEST_CONFIG

#include "test.h"
#include <objc/Protocol.h>

int main()
{
    // Class Protocol is always a subclass of NSObject

    testassert(objc_getClass("NSObject"));

    Class cls = objc_getClass("Protocol");
    testassert(class_getInstanceMethod(cls, sel_registerName("isProxy")));
    testassert(class_getSuperclass(cls) == objc_getClass("NSObject"));

    succeed(__FILE__);
}
