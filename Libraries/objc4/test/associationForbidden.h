#include "testroot.i"

@interface Normal : TestRoot
@end
@implementation Normal
@end

@interface Forbidden : TestRoot
@end
@implementation Forbidden
@end

struct minimal_unrealized_class {
    void *isa;
    void *superclass;
    void *cachePtr;
    uintptr_t maskAndOccupied;
    struct minimal_class_ro *ro;
};

struct minimal_class_ro {
    uint32_t flags;
};

extern struct minimal_unrealized_class OBJC_CLASS_$_Forbidden;

#define RO_FORBIDS_ASSOCIATED_OBJECTS (1<<10)

static void *key = &key;

static void test(void);

int main()
{
    struct minimal_unrealized_class *localForbidden = &OBJC_CLASS_$_Forbidden;
    localForbidden->ro->flags |= RO_FORBIDS_ASSOCIATED_OBJECTS;
    test();
}

static inline void ShouldSucceed(id obj) {
    objc_setAssociatedObject(obj, key, obj, OBJC_ASSOCIATION_ASSIGN);
    id assoc = objc_getAssociatedObject(obj, key);
    fprintf(stderr, "Associated object is %p\n", assoc);
    testassert(obj == assoc);
}

static inline void ShouldFail(id obj) {
    objc_setAssociatedObject(obj, key, obj, OBJC_ASSOCIATION_ASSIGN);
    fail("should have crashed trying to set the associated object");
}
