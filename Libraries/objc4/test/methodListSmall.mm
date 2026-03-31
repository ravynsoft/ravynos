// TEST_CFLAGS -std=c++11

#include "methodListSmall.h"

void testClass(Class c) {    
    id foo = [c new];
    [foo myMethod1];
    testassert(ranMyMethod1);
    [foo myMethod2];
    testassert(ranMyMethod2);
    [foo myMethod3];
    testassert(ranMyMethod3);
    
    Method m1 = class_getInstanceMethod(c, @selector(myMethod1));
    testassert(m1);
    testassert(method_getName(m1) == @selector(myMethod1));
    testassert(strcmp(method_getTypeEncoding(m1), "v16@0:8") == 0);
    testassert(method_getImplementation(m1) == (IMP)myMethod1);
    
    method_setImplementation(m1, (IMP)myReplacedMethod1);
    testassert(method_getImplementation(m1) == (IMP)myReplacedMethod1);
    [foo myMethod1];
    testassert(ranMyReplacedMethod1);
    
    Method m2 = class_getInstanceMethod(c, @selector(myMethod2));
    auto method_invoke_cast = (void (*)(id, Method))method_invoke;
    
    ranMyMethod2 = 0;
    method_invoke_cast(foo, m2);
    testassert(ranMyMethod2);
    
    method_setImplementation(m2, (IMP)myReplacedMethod2);
    method_invoke_cast(foo, m2);
    testassert(ranMyReplacedMethod2);
    
    Method mstret = class_getInstanceMethod(c, @selector(myMethodStret));
#if __arm64__
    // No _stret variant on ARM64. We'll test struct return through
    // method_invoke anyway just to be thorough.
    auto method_invoke_stret_cast = (BigStruct (*)(id, Method))method_invoke;
#else
    auto method_invoke_stret_cast = (BigStruct (*)(id, Method))method_invoke_stret;
#endif
    
    [foo myMethodStret];
    testassert(ranMyMethodStret);
    
    ranMyMethodStret = 0;
    method_invoke_stret_cast(foo, mstret);
    testassert(ranMyMethodStret);
    
    method_setImplementation(mstret, (IMP)myReplacedMethodStret);
    [foo myMethodStret];
    testassert(ranMyReplacedMethodStret);
    
    ranMyReplacedMethodStret = 0;
    method_invoke_stret_cast(foo, mstret);
    testassert(ranMyReplacedMethodStret);
    
    auto *desc1 = method_getDescription(m1);
    testassert(desc1->name == @selector(myMethod1));
    testassert(desc1->types == method_getTypeEncoding(m1));
    
    auto *desc2 = method_getDescription(m2);
    testassert(desc2->name == @selector(myMethod2));
    testassert(desc2->types == method_getTypeEncoding(m2));
    
    auto *descstret = method_getDescription(mstret);
    testassert(descstret->name == @selector(myMethodStret));
    testassert(descstret->types == method_getTypeEncoding(mstret));
}

int main() {
    Class fooClass = (__bridge Class)&FooClass;

    // Make sure this class can be duplicated and works as expected.
    // Duplicate it before testClass mucks around with the methods.
    // Need to realize fooClass before duplicating it, hence the
    // class message.
    Class dupedClass = objc_duplicateClass([fooClass class], "FooDup", 0);

    testprintf("Testing class.\n");
    testClass(fooClass);

    testprintf("Testing duplicate class.\n");
    testClass(dupedClass);

    succeed(__FILE__);
}
