// TEST_CONFIG

#include "test.h"
#include "testroot.i"

#include <dlfcn.h>

extern uintptr_t objc_debug_realized_class_generation_count;

int main()
{
    testassert(objc_debug_realized_class_generation_count > 0);
    uintptr_t prev = objc_debug_realized_class_generation_count;
    
    void *handle = dlopen("/System/Library/Frameworks/Foundation.framework/Foundation", RTLD_LAZY);
    testassert(handle);
    Class c = objc_getClass("NSFileManager");
    testassert(c);
    testassert(objc_debug_realized_class_generation_count > prev);
    
    prev = objc_debug_realized_class_generation_count;
    c = objc_allocateClassPair([TestRoot class], "Dynamic", 0);
    testassert(objc_debug_realized_class_generation_count > prev);
    prev = objc_debug_realized_class_generation_count;
    objc_registerClassPair(c);
    testassert(objc_debug_realized_class_generation_count == prev);
    
    succeed(__FILE__);
}