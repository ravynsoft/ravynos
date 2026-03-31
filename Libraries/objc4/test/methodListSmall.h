#include "test.h"

struct ObjCClass {
    struct ObjCClass *isa;
    struct ObjCClass *superclass;
    void *cachePtr;
    uintptr_t zero;
    struct ObjCClass_ro *data;
};

struct ObjCClass_ro {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instanceSize;
#ifdef __LP64__
    uint32_t reserved;
#endif

    const uint8_t * ivarLayout;
    
    const char * name;
    struct ObjCMethodList * baseMethodList;
    struct protocol_list_t * baseProtocols;
    const struct ivar_list_t * ivars;

    const uint8_t * weakIvarLayout;
    struct property_list_t *baseProperties;
};

struct ObjCMethod {
    char *name;
    char *type;
    IMP imp;
};

struct ObjCMethodList {
    uint32_t sizeAndFlags;
    uint32_t count;
    struct ObjCMethod methods[];
};

struct ObjCMethodSmall {
    int32_t nameOffset;
    int32_t typeOffset;
    int32_t impOffset;
};

struct ObjCMethodListSmall {
    uint32_t sizeAndFlags;
    uint32_t count;
    struct ObjCMethodSmall methods[];
};


extern struct ObjCClass OBJC_METACLASS_$_NSObject;
extern struct ObjCClass OBJC_CLASS_$_NSObject;


struct ObjCClass_ro FooMetaclass_ro = {
    .flags = 1,
    .instanceStart = 40,
    .instanceSize = 40,
    .name = "Foo",
};

struct ObjCClass FooMetaclass = {
    .isa = &OBJC_METACLASS_$_NSObject,
    .superclass = &OBJC_METACLASS_$_NSObject,
    .cachePtr = &_objc_empty_cache,
    .data = &FooMetaclass_ro,
};


int ranMyMethod1;
extern "C" void myMethod1(id self __unused, SEL _cmd) {
    testprintf("myMethod1\n");
    testassert(_cmd == @selector(myMethod1));
    ranMyMethod1 = 1;
}

int ranMyMethod2;
extern "C" void myMethod2(id self __unused, SEL _cmd) {
    testprintf("myMethod2\n");
    testassert(_cmd == @selector(myMethod2));
    ranMyMethod2 = 1;
}

int ranMyMethod3;
extern "C" void myMethod3(id self __unused, SEL _cmd) {
    testprintf("myMethod3\n");
    testassert(_cmd == @selector(myMethod3));
    ranMyMethod3 = 1;
}

int ranMyReplacedMethod1;
extern "C" void myReplacedMethod1(id self __unused, SEL _cmd) {
    testprintf("myReplacedMethod1\n");
    testassert(_cmd == @selector(myMethod1));
    ranMyReplacedMethod1 = 1;
}

int ranMyReplacedMethod2;
extern "C" void myReplacedMethod2(id self __unused, SEL _cmd) {
    testprintf("myReplacedMethod2\n");
    testassert(_cmd == @selector(myMethod2));
    ranMyReplacedMethod2 = 1;
}

struct BigStruct {
  uintptr_t a, b, c, d, e, f, g;
};

int ranMyMethodStret;
extern "C" BigStruct myMethodStret(id self __unused, SEL _cmd) {
    testprintf("myMethodStret\n");
    testassert(_cmd == @selector(myMethodStret));
    ranMyMethodStret = 1;
    BigStruct ret = {};
    return ret;
}

int ranMyReplacedMethodStret;
extern "C" BigStruct myReplacedMethodStret(id self __unused, SEL _cmd) {
    testprintf("myReplacedMethodStret\n");
    testassert(_cmd == @selector(myMethodStret));
    ranMyReplacedMethodStret = 1;
    BigStruct ret = {};
    return ret;
}

extern struct ObjCMethodList Foo_methodlistSmall;

asm(R"ASM(
.section __TEXT,__cstring
_MyMethod1Name:
    .asciz "myMethod1"
_MyMethod2Name:
    .asciz "myMethod2"
_MyMethod3Name:
    .asciz "myMethod3"
_BoringMethodType:
    .asciz "v16@0:8"
_MyMethodStretName:
    .asciz "myMethodStret"
_StretType:
    .asciz "{BigStruct=QQQQQQQ}16@0:8"
)ASM");

#if __LP64__
asm(R"ASM(
.section __DATA,__objc_selrefs,literal_pointers,no_dead_strip
_MyMethod1NameRef:
    .quad _MyMethod1Name
_MyMethod2NameRef:
    .quad _MyMethod2Name
_MyMethod3NameRef:
    .quad _MyMethod3Name
_MyMethodStretNameRef:
    .quad _MyMethodStretName
)ASM");
#else
asm(R"ASM(
.section __DATA,__objc_selrefs,literal_pointers,no_dead_strip
_MyMethod1NameRef:
    .long _MyMethod1Name
_MyMethod2NameRef:
    .long _MyMethod2Name
_MyMethod3NameRef:
    .long _MyMethod3Name
_MyMethodStretNameRef:
    .long _MyMethodStretName
)ASM");
#endif

#if MUTABLE_METHOD_LIST
asm(".section __DATA,__objc_methlist\n");
#else
asm(".section __TEXT,__objc_methlist\n");
#endif

asm(R"ASM(
    .p2align 2
_Foo_methodlistSmall:
    .long 12 | 0x80000000
    .long 4
    
    .long _MyMethod1NameRef - .
    .long _BoringMethodType - .
    .long _myMethod1 - .
    
    .long _MyMethod2NameRef - .
    .long _BoringMethodType - .
    .long _myMethod2 - .
    
    .long _MyMethod3NameRef - .
    .long _BoringMethodType - .
    .long _myMethod3 - .
    
    .long _MyMethodStretNameRef - .
    .long _StretType - .
    .long _myMethodStret - .
)ASM");

struct ObjCClass_ro Foo_ro = {
    .instanceStart = 8,
    .instanceSize = 8,
    .name = "Foo",
    .baseMethodList = &Foo_methodlistSmall,
};

struct ObjCClass FooClass = {
    .isa = &FooMetaclass,
    .superclass = &OBJC_CLASS_$_NSObject,
    .cachePtr = &_objc_empty_cache,
    .data = &Foo_ro,
};


@interface Foo: NSObject

- (void)myMethod1;
- (void)myMethod2;
- (void)myMethod3;
- (BigStruct)myMethodStret;

@end
