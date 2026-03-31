// TEST_CONFIG MEM=mrc

#define TEST_CALLS_OPERATOR_NEW
#include "test.h"
#include "testroot.i"
#include "swift-class-def.m"

#include <objc/objc-internal.h>

#include <vector>

static Class expectedOldClass;

static std::vector<Class> observedNewClasses1;
static void handler1(Class _Nonnull oldClass, Class _Nonnull newClass) {
    testprintf("%s(%p, %p)", __func__, oldClass, newClass);
    testassert(oldClass == expectedOldClass);
    observedNewClasses1.push_back(newClass);
}

static std::vector<Class> observedNewClasses2;
static void handler2(Class _Nonnull oldClass, Class _Nonnull newClass) {
    testprintf("%s(%p, %p)", __func__, oldClass, newClass);
    testassert(oldClass == expectedOldClass);
    observedNewClasses2.push_back(newClass);
}

static std::vector<Class> observedNewClasses3;
static void handler3(Class _Nonnull oldClass, Class _Nonnull newClass) {
    testprintf("%s(%p, %p)", __func__, oldClass, newClass);
    testassert(oldClass == expectedOldClass);
    observedNewClasses3.push_back(newClass);
}

EXTERN_C Class _objc_realizeClassFromSwift(Class, void *);

EXTERN_C Class init(Class cls, void *arg) {
    (void)arg;
    _objc_realizeClassFromSwift(cls, cls);
    return cls;
}

@interface SwiftRoot: TestRoot @end
SWIFT_CLASS(SwiftRoot, TestRoot, init);

int main()
{
    expectedOldClass = [SwiftRoot class];
    Class A = objc_allocateClassPair([RawSwiftRoot class], "A", 0);
    objc_registerClassPair(A);
    testassert(observedNewClasses1.size() == 0);
    testassert(observedNewClasses2.size() == 0);
    testassert(observedNewClasses3.size() == 0);
    
    _objc_setClassCopyFixupHandler(handler1);
    
    expectedOldClass = A;
    Class B = objc_allocateClassPair(A, "B", 0);
    objc_registerClassPair(B);
    testassert(observedNewClasses1.size() == 2);
    testassert(observedNewClasses2.size() == 0);
    testassert(observedNewClasses3.size() == 0);
    testassert(observedNewClasses1[0] == B);
    
    _objc_setClassCopyFixupHandler(handler2);
    
    expectedOldClass = B;
    Class C = objc_allocateClassPair(B, "C", 0);
    objc_registerClassPair(C);
    testassert(observedNewClasses1.size() == 4);
    testassert(observedNewClasses2.size() == 2);
    testassert(observedNewClasses3.size() == 0);
    testassert(observedNewClasses1[2] == C);
    testassert(observedNewClasses2[0] == C);
    
    _objc_setClassCopyFixupHandler(handler3);
    
    expectedOldClass = C;
    Class D = objc_allocateClassPair(C, "D", 0);
    objc_registerClassPair(D);
    testassert(observedNewClasses1.size() == 6);
    testassert(observedNewClasses2.size() == 4);
    testassert(observedNewClasses3.size() == 2);
    testassert(observedNewClasses1[4] == D);
    testassert(observedNewClasses2[2] == D);
    testassert(observedNewClasses3[0] == D);
    
    succeed(__FILE__);
}
