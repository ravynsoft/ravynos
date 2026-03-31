// TEST_CONFIG

#include "test.h"
#include "testroot.i"
#include <objc/runtime.h>
#include <objc/objc-internal.h>

// Macros for array construction.
// ten IMPs
#define IMPS10 (IMP)fn0, (IMP)fn1, (IMP)fn2, (IMP)fn3, (IMP)fn4, \
               (IMP)fn5, (IMP)fn6, (IMP)fn7, (IMP)fn8, (IMP)fn9
// ten method types
#define TYPES10 "", "", "", "", "", "", "", "", "", ""
// ten selectors of the form name0..name9
#define SELS10(name)                                                \
    @selector(name##0), @selector(name##1), @selector(name##2),     \
    @selector(name##3), @selector(name##4), @selector(name##5),     \
    @selector(name##6), @selector(name##7), @selector(name##8),     \
    @selector(name##9)


@interface Super : TestRoot @end
@implementation Super 
-(int)superMethod0 { return 0; }
-(int)superMethod1 { return 0; }
-(int)superMethod2 { return 0; }
-(int)superMethod3 { return 0; }
-(int)superMethod4 { return 0; }
-(int)superMethod5 { return 0; }
-(int)superMethod6 { return 0; }
-(int)superMethod7 { return 0; }
-(int)superMethod8 { return 0; }
-(int)superMethod9 { return 0; }
-(int)bothMethod0 { return 0; }
-(int)bothMethod1 { return 0; }
-(int)bothMethod2 { return 0; }
-(int)bothMethod3 { return 0; }
-(int)bothMethod4 { return 0; }
-(int)bothMethod5 { return 0; }
-(int)bothMethod6 { return 0; }
-(int)bothMethod7 { return 0; }
-(int)bothMethod8 { return 0; }
-(int)bothMethod9 { return 0; }
@end

@interface Sub : Super @end
@implementation Sub
-(int)subMethod0 { return 0; }
-(int)subMethod1 { return 0; }
-(int)subMethod2 { return 0; }
-(int)subMethod3 { return 0; }
-(int)subMethod4 { return 0; }
-(int)subMethod5 { return 0; }
-(int)subMethod6 { return 0; }
-(int)subMethod7 { return 0; }
-(int)subMethod8 { return 0; }
-(int)subMethod9 { return 0; }
-(int)bothMethod0 { return 0; }
-(int)bothMethod1 { return 0; }
-(int)bothMethod2 { return 0; }
-(int)bothMethod3 { return 0; }
-(int)bothMethod4 { return 0; }
-(int)bothMethod5 { return 0; }
-(int)bothMethod6 { return 0; }
-(int)bothMethod7 { return 0; }
-(int)bothMethod8 { return 0; }
-(int)bothMethod9 { return 0; }
@end

@interface Sub2 : Super @end
@implementation Sub2
-(int)subMethod0 { return 0; }
-(int)subMethod1 { return 0; }
-(int)subMethod2 { return 0; }
-(int)subMethod3 { return 0; }
-(int)subMethod4 { return 0; }
-(int)subMethod5 { return 0; }
-(int)subMethod6 { return 0; }
-(int)subMethod7 { return 0; }
-(int)subMethod8 { return 0; }
-(int)subMethod9 { return 0; }
-(int)bothMethod0 { return 0; }
-(int)bothMethod1 { return 0; }
-(int)bothMethod2 { return 0; }
-(int)bothMethod3 { return 0; }
-(int)bothMethod4 { return 0; }
-(int)bothMethod5 { return 0; }
-(int)bothMethod6 { return 0; }
-(int)bothMethod7 { return 0; }
-(int)bothMethod8 { return 0; }
-(int)bothMethod9 { return 0; }
@end


id fn0(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn1(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn2(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn3(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn4(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn5(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn6(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn7(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn8(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}
id fn9(id self __attribute__((unused)), SEL cmd __attribute__((unused)), ...) {
    return nil;
}

void testBulkMemoryOnce(void)
{
    Class c = objc_allocateClassPair([TestRoot class], "c", 0);
    objc_registerClassPair(c);
    
    SEL sels[10] = {
        SELS10(method)
    };
    IMP imps[10] = {
        IMPS10
    };
    const char *types[10] = {
        TYPES10
    };
    
    uint32_t failureCount = 0;
    SEL *failed;
    
    // Test all successes.
    failed = class_addMethodsBulk(c, sels, imps, types, 4, &failureCount);
    testassert(failed == NULL);
    testassert(failureCount == 0);
    
    // Test mixed success and failure (this overlaps the previous one, so there
    // will be one of each).
    failed = class_addMethodsBulk(c, sels + 3, imps + 3, types + 3, 2,
                                  &failureCount);
    testassert(failed != NULL);
    testassert(failureCount == 1);
    testassert(failed[0] == sels[3]);
    free(failed);
    
    // Test total failure.
    failed = class_addMethodsBulk(c, sels, imps, types, 5, &failureCount);
    testassert(failed != NULL);
    testassert(failureCount == 5);
    for(int i = 0; i < 5; i++) {
        testassert(failed[i] == sels[i]);
    }
    free(failed);
    
    class_replaceMethodsBulk(c, sels, imps, types, 10);
    
    for(int i = 0; i < 10; i++) {
        testassert(class_getMethodImplementation(c, sels[i]) == imps[i]);
    }
    
    objc_disposeClassPair(c);
}

int main()
{
    IMP dummyIMPs[130] = {
        IMPS10, IMPS10, IMPS10, IMPS10, IMPS10,
        IMPS10, IMPS10, IMPS10, IMPS10, IMPS10,
        IMPS10, IMPS10, IMPS10,
    };
    
    // similar to dummyIMPs but with different values in each slot
    IMP dummyIMPs2[130] = {
        (IMP)fn5, (IMP)fn6, (IMP)fn7, (IMP)fn8, (IMP)fn9,
        IMPS10, IMPS10, IMPS10, IMPS10, IMPS10,
        IMPS10, IMPS10, IMPS10, IMPS10, IMPS10,
        IMPS10, IMPS10,
        (IMP)fn0, (IMP)fn1, (IMP)fn2, (IMP)fn3, (IMP)fn4,
    };
    
    const char *dummyTypes[130] = {
        TYPES10, TYPES10, TYPES10, TYPES10, TYPES10,
        TYPES10, TYPES10, TYPES10, TYPES10, TYPES10,
        TYPES10, TYPES10, TYPES10,
    };
    
    SEL addSELs[20] = {
        SELS10(superMethod),
        SELS10(superMethodAddNew)
    };
    
    uint32_t failedCount = 0;
    SEL *failed;
    
    failed = class_addMethodsBulk([Super class], addSELs, dummyIMPs, dummyTypes,
                                  20, &failedCount);
    
    // class_addMethodsBulk reports failures for all methods that already exist
    testassert(failed != NULL);
    testassert(failedCount == 10);
    
    // class_addMethodsBulk failed for existing implementations
    for(int i = 0; i < 10; i++) {
        testassert(failed[i] == addSELs[i]);
        testassert(class_getMethodImplementation([Super class], addSELs[i])
                   != dummyIMPs[i]);
    }
    
    free(failed);

    // class_addMethodsBulk does add root implementations
    for(int i = 10; i < 20; i++) {
        testassert(class_getMethodImplementation([Super class], addSELs[i])
                   == dummyIMPs[i]);
    }
    
    // class_addMethod does override superclass implementations
    failed = class_addMethodsBulk([Sub class], addSELs, dummyIMPs, dummyTypes,
                                  10, &failedCount);
    testassert(failedCount == 0);
    testassert(failed == NULL);
    for(int i = 0; i < 10; i++) {
        testassert(class_getMethodImplementation([Sub class], addSELs[i])
                   == dummyIMPs[i]);
    }
    
    SEL subReplaceSELs[40] = {
        SELS10(superMethod),
        SELS10(subMethodNew),
        SELS10(subMethod),
        SELS10(bothMethod),
    };
    
    // class_replaceMethodsBulk adds new implementations or replaces existing
    // ones for methods that exist on the superclass, the subclass, both, or
    // neither
    class_replaceMethodsBulk([Sub2 class], subReplaceSELs, dummyIMPs,
                             dummyTypes, 40);
    for(int i = 0; i < 40; i++) {
        IMP newIMP = class_getMethodImplementation([Sub2 class],
                                                   subReplaceSELs[i]);
        testassert(newIMP == dummyIMPs[i]);
    }
    
    SEL superReplaceSELs[20] = {
        SELS10(superMethod),
        SELS10(superMethodNew),
    };
    
    // class_replaceMethodsBulk adds new implementations or replaces existing
    // ones in the superclass
    class_replaceMethodsBulk([Super class], superReplaceSELs, dummyIMPs,
                             dummyTypes, 20);
    for(int i = 0; i < 20; i++) {
        IMP newIMP = class_getMethodImplementation([Super class],
                                                   superReplaceSELs[i]);
        testassert(newIMP == dummyIMPs[i]);
    }


    // class_addMethodsBulk, where almost all of the requested additions
    // already exist and thus can't be added. (They were already added
    // above by class_replaceMethodsBulk([Sub2 class], subReplaceSELs, ...).)

    // This list is large in the hope of provoking any realloc() of the
    // new method list inside addMethods().
    // The runtime doesn't care that the list contains lots of duplicates.
    SEL subAddMostlyExistingSELs[130] = {
        SELS10(superMethod), SELS10(subMethodNew), SELS10(subMethod),
        SELS10(superMethod), SELS10(subMethodNew), SELS10(subMethod),
        SELS10(superMethod), SELS10(subMethodNew), SELS10(subMethod),
        SELS10(superMethod), SELS10(subMethodNew), SELS10(subMethod),
        SELS10(bothMethod),
    };
    subAddMostlyExistingSELs[16] = @selector(INDEX_16_IS_DIFFERENT);

    failed = class_addMethodsBulk([Sub2 class], subAddMostlyExistingSELs,
                                  dummyIMPs2, dummyTypes, 130, &failedCount);
    testassert(failedCount == 129);
    testassert(failed != NULL);

    for(int i = 0; i < 130; i++) {
        IMP newIMP = class_getMethodImplementation([Sub2 class],
                                                   subAddMostlyExistingSELs[i]);
        if (i == 16) {
            // the only one that was actually added
            testassert(newIMP != dummyIMPs[i]);
            testassert(newIMP == dummyIMPs2[i]);
        } else {
            // the others should all have failed
            testassert(newIMP == dummyIMPs[i]);
            testassert(newIMP != dummyIMPs2[i]);
        }
    }
    for (uint32_t i = 0; i < failedCount; i++) {
        testassert(failed[i] != NULL);
        testassert(failed[i] != subAddMostlyExistingSELs[16]);
    }

    
    // fixme actually try calling them
    
    // make sure the Bulk functions aren't leaking
    testBulkMemoryOnce();
    leak_mark();
    for(int i = 0; i < 10; i++) {
        testBulkMemoryOnce();
    }
    leak_check(0);
    
    succeed(__FILE__);
}
