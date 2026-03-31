// TEST_CFLAGS -framework Foundation
// TEST_CONFIG MEM=mrc

#include "test.h"
#include <dlfcn.h>

#include <objc/objc-gdb.h>
#include <Foundation/Foundation.h>

#define ISA(x) (*((uintptr_t *)(x)))
#define NONPOINTER(x) (ISA(x) & 1)

#if SUPPORT_NONPOINTER_ISA
# if __x86_64__
#   define RC_ONE (1ULL<<56)
# elif __arm64__ && __LP64__
#   define RC_ONE (1ULL<<45)
# elif __ARM_ARCH_7K__ >= 2  ||  (__arm64__ && !__LP64__)
#   define RC_ONE (1ULL<<25)
# else
#   error unknown architecture
# endif
#endif


void check_raw_pointer(id obj, Class cls)
{
    testassert(object_getClass(obj) == cls);
    testassert(!NONPOINTER(obj));

    uintptr_t isa = ISA(obj);
    testassert((Class)isa == cls);
    testassert((Class)(isa & objc_debug_isa_class_mask) == cls);
    testassert((Class)(isa & ~objc_debug_isa_class_mask) == 0);

    CFRetain(obj);
    testassert(ISA(obj) == isa);
    testassert([obj retainCount] == 2);
    [obj retain];
    testassert(ISA(obj) == isa);
    testassert([obj retainCount] == 3);
    CFRelease(obj);
    testassert(ISA(obj) == isa);
    testassert([obj retainCount] == 2);
    [obj release];
    testassert(ISA(obj) == isa);
    testassert([obj retainCount] == 1);
}


#if ! SUPPORT_NONPOINTER_ISA

int main()
{
#if OBJC_HAVE_NONPOINTER_ISA  ||  OBJC_HAVE_PACKED_NONPOINTER_ISA  ||  OBJC_HAVE_INDEXED_NONPOINTER_ISA
#   error wrong
#endif

    testprintf("Isa with index\n");
    id index_o = [NSObject new];
    check_raw_pointer(index_o, [NSObject class]);

    // These variables DO NOT exist without non-pointer isa support.
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_packed_isa_class_mask"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_magic_mask"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_magic_value"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_index_mask"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_index_shift"));

    // These variables DO exist even without non-pointer isa support.
    testassert(dlsym(RTLD_DEFAULT, "objc_debug_isa_class_mask"));
    testassert(dlsym(RTLD_DEFAULT, "objc_debug_isa_magic_mask"));
    testassert(dlsym(RTLD_DEFAULT, "objc_debug_isa_magic_value"));

    succeed(__FILE__);
}

#else
// SUPPORT_NONPOINTER_ISA

void check_nonpointer(id obj, Class cls)
{
    testassert(object_getClass(obj) == cls);
    testassert(NONPOINTER(obj));

    uintptr_t isa = ISA(obj);

    if (objc_debug_indexed_isa_magic_mask != 0) {
        // Indexed isa.
        testassert((isa & objc_debug_indexed_isa_magic_mask) == objc_debug_indexed_isa_magic_value);
        testassert((isa & ~objc_debug_indexed_isa_index_mask) != 0);
        uintptr_t index = (isa & objc_debug_indexed_isa_index_mask) >> objc_debug_indexed_isa_index_shift;
        testassert(index < objc_indexed_classes_count);
        testassert(objc_indexed_classes[index] == cls);
    } else {
        // Packed isa.
        testassert((Class)(isa & objc_debug_isa_class_mask) == cls);
        testassert((Class)(isa & ~objc_debug_isa_class_mask) != 0);
        testassert((isa & objc_debug_isa_magic_mask) == objc_debug_isa_magic_value);
    }

    CFRetain(obj);
    testassert(ISA(obj) == isa + RC_ONE);
    testassert([obj retainCount] == 2);
    [obj retain];
    testassert(ISA(obj) == isa + RC_ONE*2);
    testassert([obj retainCount] == 3);
    CFRelease(obj);
    testassert(ISA(obj) == isa + RC_ONE);
    testassert([obj retainCount] == 2);
    [obj release];
    testassert(ISA(obj) == isa);
    testassert([obj retainCount] == 1);
}


@interface Fake_OS_object : NSObject {
    int refcnt;
    int xref_cnt;
}
@end

@implementation Fake_OS_object
+(void)initialize {
    static bool initialized;
    if (!initialized) {
        initialized = true;
        testprintf("Nonpointer during +initialize\n");
        testassert(!NONPOINTER(self));
        id o = [Fake_OS_object new];
        check_nonpointer(o, self);
        [o release];
    }
}
@end

@interface Sub_OS_object : NSObject @end

@implementation Sub_OS_object
@end



int main()
{
    Class OS_object = objc_getClass("OS_object");
    class_setSuperclass([Sub_OS_object class], OS_object);

    uintptr_t isa;

#if SUPPORT_PACKED_ISA
# if !OBJC_HAVE_NONPOINTER_ISA  ||  !OBJC_HAVE_PACKED_NONPOINTER_ISA  ||  OBJC_HAVE_INDEXED_NONPOINTER_ISA
#   error wrong
# endif
    testassert(objc_debug_isa_class_mask == (uintptr_t)&objc_absolute_packed_isa_class_mask);

    // Indexed isa variables DO NOT exist on packed-isa platforms
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_magic_mask"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_magic_value"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_index_mask"));
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_indexed_isa_index_shift"));
    
#elif SUPPORT_INDEXED_ISA
# if !OBJC_HAVE_NONPOINTER_ISA  ||  OBJC_HAVE_PACKED_NONPOINTER_ISA  ||  !OBJC_HAVE_INDEXED_NONPOINTER_ISA
#   error wrong
# endif
    testassert(objc_debug_indexed_isa_magic_mask == (uintptr_t)&objc_absolute_indexed_isa_magic_mask);
    testassert(objc_debug_indexed_isa_magic_value == (uintptr_t)&objc_absolute_indexed_isa_magic_value);
    testassert(objc_debug_indexed_isa_index_mask == (uintptr_t)&objc_absolute_indexed_isa_index_mask);
    testassert(objc_debug_indexed_isa_index_shift == (uintptr_t)&objc_absolute_indexed_isa_index_shift);

    // Packed isa variable DOES NOT exist on indexed-isa platforms.
    testassert(!dlsym(RTLD_DEFAULT, "objc_absolute_packed_isa_class_mask"));

#else
#   error unknown nonpointer isa format
#endif
    
    testprintf("Isa with index\n");
    id index_o = [Fake_OS_object new];
    check_nonpointer(index_o, [Fake_OS_object class]);

    testprintf("Weakly referenced\n");
    isa = ISA(index_o);
    id weak;
    objc_storeWeak(&weak, index_o);
    testassert(__builtin_popcountl(isa ^ ISA(index_o)) == 1);

    testprintf("Has associated references\n");
    id assoc = @"thing";
    isa = ISA(index_o);
    objc_setAssociatedObject(index_o, assoc, assoc, OBJC_ASSOCIATION_ASSIGN);
    testassert(__builtin_popcountl(isa ^ ISA(index_o)) == 1);

    testprintf("Isa without index\n");
    id raw_o = [OS_object alloc];
    check_raw_pointer(raw_o, [OS_object class]);


    id buf[4];
    id bufo = (id)buf;

    testprintf("Change isa 0 -> raw pointer\n");
    bzero(buf, sizeof(buf));
    object_setClass(bufo, [OS_object class]);
    check_raw_pointer(bufo, [OS_object class]);

    testprintf("Change isa 0 -> nonpointer\n");
    bzero(buf, sizeof(buf));
    object_setClass(bufo, [NSObject class]);
    check_nonpointer(bufo, [NSObject class]);

    testprintf("Change isa nonpointer -> nonpointer\n");
    testassert(NONPOINTER(bufo));
    _objc_rootRetain(bufo);
    testassert(_objc_rootRetainCount(bufo) == 2);
    object_setClass(bufo, [Fake_OS_object class]);
    testassert(_objc_rootRetainCount(bufo) == 2);
    _objc_rootRelease(bufo);
    testassert(_objc_rootRetainCount(bufo) == 1);
    check_nonpointer(bufo, [Fake_OS_object class]);

    testprintf("Change isa nonpointer -> raw pointer\n");
    // Retain count must be preserved.
    // Use root* to avoid OS_object's overrides.
    testassert(NONPOINTER(bufo));
    _objc_rootRetain(bufo);
    testassert(_objc_rootRetainCount(bufo) == 2);
    object_setClass(bufo, [OS_object class]);
    testassert(_objc_rootRetainCount(bufo) == 2);
    _objc_rootRelease(bufo);
    testassert(_objc_rootRetainCount(bufo) == 1);
    check_raw_pointer(bufo, [OS_object class]);

    testprintf("Change isa raw pointer -> nonpointer (doesn't happen)\n");
    testassert(!NONPOINTER(bufo));
    _objc_rootRetain(bufo);
    testassert(_objc_rootRetainCount(bufo) == 2);
    object_setClass(bufo, [Fake_OS_object class]);
    testassert(_objc_rootRetainCount(bufo) == 2);
    _objc_rootRelease(bufo);
    testassert(_objc_rootRetainCount(bufo) == 1);
    check_raw_pointer(bufo, [Fake_OS_object class]);

    testprintf("Change isa raw pointer -> raw pointer\n");
    testassert(!NONPOINTER(bufo));
    _objc_rootRetain(bufo);
    testassert(_objc_rootRetainCount(bufo) == 2);
    object_setClass(bufo, [Sub_OS_object class]);
    testassert(_objc_rootRetainCount(bufo) == 2);
    _objc_rootRelease(bufo);
    testassert(_objc_rootRetainCount(bufo) == 1);
    check_raw_pointer(bufo, [Sub_OS_object class]);


    succeed(__FILE__);
}

// SUPPORT_NONPOINTER_ISA
#endif
