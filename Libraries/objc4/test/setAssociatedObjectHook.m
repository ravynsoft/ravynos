// TEST_CONFIG

#include "test.h"
#include "testroot.i"

id sawObject;
const void *sawKey;
id sawValue;
objc_AssociationPolicy sawPolicy;

objc_hook_setAssociatedObject originalSetAssociatedObject;

void hook(id _Nonnull object, const void * _Nonnull key, id _Nullable value, objc_AssociationPolicy policy) {
    sawObject = object;
    sawKey = key;
    sawValue = value;
    sawPolicy = policy;
    originalSetAssociatedObject(object, key, value, policy);
}

int main() {
    id obj = [TestRoot new];
    id value = [TestRoot new];
    const void *key = "key";
    objc_setAssociatedObject(obj, key, value, OBJC_ASSOCIATION_RETAIN);
    testassert(sawObject == nil);
    testassert(sawKey == nil);
    testassert(sawValue == nil);
    testassert(sawPolicy == 0);

    id out = objc_getAssociatedObject(obj, key);
    testassert(out == value);

    objc_setHook_setAssociatedObject(hook, &originalSetAssociatedObject);

    key = "key2";
    objc_setAssociatedObject(obj, key, value, OBJC_ASSOCIATION_RETAIN);
    testassert(sawObject == obj);
    testassert(sawKey == key);
    testassert(sawValue == value);
    testassert(sawPolicy == OBJC_ASSOCIATION_RETAIN);

    out = objc_getAssociatedObject(obj, key);
    testassert(out == value);

    succeed(__FILE__);
}