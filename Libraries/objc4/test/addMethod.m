// TEST_CONFIG

#include "test.h"
#include "testroot.i"
#include <objc/runtime.h>

@interface Super : TestRoot @end
@implementation Super 
-(int)superMethod { return 0; }
-(int)superMethod2 { return 0; }
-(int)bothMethod { return 0; } 
@end

@interface Sub : Super @end
@implementation Sub
-(int)subMethod { return 0; }
-(int)bothMethod { return 0; }
@end

@interface Sub2 : Super @end
@implementation Sub2
-(int)subMethod { return 0; }
-(int)bothMethod { return 0; }
@end


id fn(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) { return nil; }

#define TWICE(x) x; x

int main()
{
    IMP superMethodFromSuper = class_getMethodImplementation([Super class], @selector(superMethod));
    IMP superMethod2FromSuper = class_getMethodImplementation([Super class], @selector(superMethod2));
    IMP bothMethodFromSuper = class_getMethodImplementation([Super class], @selector(bothMethod));
    IMP subMethodFromSub = class_getMethodImplementation([Sub class], @selector(subMethod));
    IMP bothMethodFromSub = class_getMethodImplementation([Sub class], @selector(bothMethod));
    IMP subMethodFromSub2 = class_getMethodImplementation([Sub2 class], @selector(subMethod));
    IMP bothMethodFromSub2 = class_getMethodImplementation([Sub2 class], @selector(bothMethod));

    testassert(superMethodFromSuper);
    testassert(superMethod2FromSuper);
    testassert(bothMethodFromSuper);
    testassert(subMethodFromSub);
    testassert(bothMethodFromSub);
    testassert(subMethodFromSub2);
    testassert(bothMethodFromSub2);

    BOOL ok;
    IMP imp;

    // Each method lookup below is performed twice, with the intent
    // that at least one of them will be a cache lookup.

    // class_addMethod doesn't replace existing implementations
    ok = class_addMethod([Super class], @selector(superMethod), (IMP)fn, NULL);
    testassert(!ok);
    TWICE(testassert(class_getMethodImplementation([Super class], @selector(superMethod)) == superMethodFromSuper));

    // class_addMethod does override superclass implementations
    ok = class_addMethod([Sub class], @selector(superMethod), (IMP)fn, NULL);
    testassert(ok);
    TWICE(testassert(class_getMethodImplementation([Sub class], @selector(superMethod)) == (IMP)fn));

    // class_addMethod does add root implementations
    ok = class_addMethod([Super class], @selector(superMethodNew2), (IMP)fn, NULL);
    testassert(ok);
    TWICE(testassert(class_getMethodImplementation([Super class], @selector(superMethodNew2)) == (IMP)fn));
    TWICE(testassert(class_getMethodImplementation([Sub class], @selector(superMethodNew2)) == (IMP)fn));

    // bincompat: some apps call class_addMethod() and class_replaceMethod() with a nil IMP
    // This has the effect of covering the superclass implementation.
    // fixme change this, possibly behind a linked-on-or-after check
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wnonnull"
    ok = class_addMethod([Sub class], @selector(superMethod2), nil, NULL);
    #pragma clang diagnostic pop
    testassert(ok);
    TWICE(testassert(class_getMethodImplementation([Sub class], @selector(superMethod2)) == (IMP)_objc_msgForward));

    // class_replaceMethod does add new implementations, 
    // returning NULL if super has an implementation
    imp = class_replaceMethod([Sub2 class], @selector(superMethod), (IMP)fn, NULL);
    testassert(imp == NULL);
    TWICE(testassert(class_getMethodImplementation([Sub2 class], @selector(superMethod)) == (IMP)fn));

    // class_replaceMethod does add new implementations, 
    // returning NULL if super has no implementation
    imp = class_replaceMethod([Sub2 class], @selector(subMethodNew), (IMP)fn, NULL);
    testassert(imp == NULL);
    TWICE(testassert(class_getMethodImplementation([Sub2 class], @selector(subMethodNew)) == (IMP)fn));
    
    // class_replaceMethod does add new implemetations
    // returning NULL if there is no super class
    imp = class_replaceMethod([Super class], @selector(superMethodNew), (IMP)fn, NULL);
    testassert(imp == NULL);
    TWICE(testassert(class_getMethodImplementation([Super class], @selector(superMethodNew)) == (IMP)fn));

    
    // class_replaceMethod does replace existing implementations, 
    // returning existing implementation (regardless of super)
    imp = class_replaceMethod([Sub2 class], @selector(subMethod), (IMP)fn, NULL);
    testassert(imp == subMethodFromSub2);
    TWICE(testassert(class_getMethodImplementation([Sub2 class], @selector(subMethod)) == (IMP)fn));

    // class_replaceMethod does replace existing implemetations, 
    // returning existing implementation (regardless of super)
    imp = class_replaceMethod([Sub2 class], @selector(bothMethod), (IMP)fn, NULL);
    testassert(imp == bothMethodFromSub2);
    TWICE(testassert(class_getMethodImplementation([Sub2 class], @selector(bothMethod)) == (IMP)fn));

    // class_replaceMethod does replace existing implemetations, 
    // returning existing implementation (regardless of super)
    imp = class_replaceMethod([Super class], @selector(superMethod), (IMP)fn, NULL);
    testassert(imp == superMethodFromSuper);
    TWICE(testassert(class_getMethodImplementation([Super class], @selector(superMethod)) == (IMP)fn));

    // fixme actually try calling them

    succeed(__FILE__);
}
