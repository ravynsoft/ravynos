// TEST_CFLAGS -fobjc-weak

#include "test.h"
#include <objc/runtime.h>
#include <objc/objc-internal.h>
#include <objc/objc-gdb.h>
#include <dlfcn.h>
#import <Foundation/NSObject.h>

#if OBJC_HAVE_TAGGED_POINTERS

#if !__x86_64__  &&  !__arm64__
#error wrong architecture for tagged pointers
#endif

static BOOL didIt;

@interface WeakContainer : NSObject
{
  @public
    __weak id weaks[10000];
}
@end
@implementation WeakContainer
-(void) dealloc {
    for (unsigned int i = 0; i < sizeof(weaks)/sizeof(weaks[0]); i++) {
        testassert(weaks[i] == nil);
    }
    SUPER_DEALLOC();
}
@end

OBJC_ROOT_CLASS
@interface TaggedBaseClass60
@end

@implementation TaggedBaseClass60
-(id) self { return self; }

+ (void) initialize {
}

- (void) instanceMethod {
    didIt = YES;
}

- (uintptr_t) taggedValue {
    return _objc_getTaggedPointerValue((__bridge void*)self);
}

- (struct stret) stret: (struct stret) aStruct {
    return aStruct;
}

- (long double) fpret: (long double) aValue {
    return aValue;
}


-(void) dealloc {
    fail("TaggedBaseClass60 dealloc called!");
}

static void *
retain_fn(void *self, SEL _cmd __unused) {
    void * (*fn)(void *) = (typeof(fn))_objc_rootRetain;
    return fn(self); 
}

static void 
release_fn(void *self, SEL _cmd __unused) {
    void (*fn)(void *) = (typeof(fn))_objc_rootRelease;
    fn(self); 
}

static void *
autorelease_fn(void *self, SEL _cmd __unused) { 
    void * (*fn)(void *) = (typeof(fn))_objc_rootAutorelease;
    return fn(self); 
}

static unsigned long 
retaincount_fn(void *self, SEL _cmd __unused) { 
    unsigned long (*fn)(void *) = (typeof(fn))_objc_rootRetainCount;
    return fn(self); 
}

+(void) load {
    class_addMethod(self, sel_registerName("retain"), (IMP)retain_fn, "");
    class_addMethod(self, sel_registerName("release"), (IMP)release_fn, "");
    class_addMethod(self, sel_registerName("autorelease"), (IMP)autorelease_fn, "");
    class_addMethod(self, sel_registerName("retainCount"), (IMP)retaincount_fn, "");    
}

@end

@interface TaggedSubclass52: TaggedBaseClass60
@end

@implementation TaggedSubclass52

- (void) instanceMethod {
    return [super instanceMethod];
}

- (uintptr_t) taggedValue {
    return [super taggedValue];
}

- (struct stret) stret: (struct stret) aStruct {
    return [super stret: aStruct];
}

- (long double) fpret: (long double) aValue {
    return [super fpret: aValue];
}
@end

@interface TaggedNSObjectSubclass : NSObject
@end

@implementation TaggedNSObjectSubclass

- (void) instanceMethod {
    didIt = YES;
}

- (uintptr_t) taggedValue {
    return _objc_getTaggedPointerValue((__bridge void*)self);
}

- (struct stret) stret: (struct stret) aStruct {
    return aStruct;
}

- (long double) fpret: (long double) aValue {
    return aValue;
}
@end

void testTaggedPointerValue(Class cls, objc_tag_index_t tag, uintptr_t value)
{
    void *taggedAddress = _objc_makeTaggedPointer(tag, value);
    testprintf("obj %p, tag %p, value %p\n", 
               taggedAddress, (void*)tag, (void*)value);

    bool ext = (tag >= OBJC_TAG_First52BitPayload);

    // _objc_makeTaggedPointer must quietly mask out of range values for now
    if (ext) {
        value = (value << 12) >> 12;
    } else {
        value = (value << 4) >> 4;
    }

    testassert(_objc_isTaggedPointer(taggedAddress));
    testassert(_objc_getTaggedPointerTag(taggedAddress) == tag);
    testassert(_objc_getTaggedPointerValue(taggedAddress) == value);
    testassert(objc_debug_taggedpointer_obfuscator != 0);

    if (ext) {
        uintptr_t slot = ((uintptr_t)taggedAddress >> objc_debug_taggedpointer_ext_slot_shift) & objc_debug_taggedpointer_ext_slot_mask;
        testassert(objc_debug_taggedpointer_ext_classes[slot] == cls);
        uintptr_t deobfuscated = (uintptr_t)taggedAddress ^ objc_debug_taggedpointer_obfuscator;
        testassert(((deobfuscated << objc_debug_taggedpointer_ext_payload_lshift) >> objc_debug_taggedpointer_ext_payload_rshift) == value);
    } 
    else {
        testassert(((uintptr_t)taggedAddress & objc_debug_taggedpointer_mask) == objc_debug_taggedpointer_mask);
        uintptr_t slot = ((uintptr_t)taggedAddress >> objc_debug_taggedpointer_slot_shift) & objc_debug_taggedpointer_slot_mask;
        testassert(objc_debug_taggedpointer_classes[slot] == cls);
        uintptr_t deobfuscated = (uintptr_t)taggedAddress ^ objc_debug_taggedpointer_obfuscator;
        testassert(((deobfuscated << objc_debug_taggedpointer_payload_lshift) >> objc_debug_taggedpointer_payload_rshift) == value);
    }

    id taggedPointer = (__bridge id)taggedAddress;
    testassert(!object_isClass(taggedPointer));
    testassert(object_getClass(taggedPointer) == cls);
    testassert([taggedPointer taggedValue] == value);

    didIt = NO;
    [taggedPointer instanceMethod];
    testassert(didIt);
    
    struct stret orig = STRET_RESULT;
    testassert(stret_equal(orig, [taggedPointer stret: orig]));
    
    long double dblvalue = 3.14156789;
    testassert(dblvalue == [taggedPointer fpret: dblvalue]);

    objc_setAssociatedObject(taggedPointer, (__bridge void *)taggedPointer, taggedPointer, OBJC_ASSOCIATION_RETAIN);
    testassert(objc_getAssociatedObject(taggedPointer, (__bridge void *)taggedPointer) == taggedPointer);
    objc_setAssociatedObject(taggedPointer, (__bridge void *)taggedPointer, nil, OBJC_ASSOCIATION_RETAIN);
    testassert(objc_getAssociatedObject(taggedPointer, (__bridge void *)taggedPointer) == nil);
}

void testGenericTaggedPointer(objc_tag_index_t tag, Class cls)
{
    testassert(cls);
    testprintf("%s\n", class_getName(cls));

    testTaggedPointerValue(cls, tag, 0);
    testTaggedPointerValue(cls, tag, 1UL << 0);
    testTaggedPointerValue(cls, tag, 1UL << 1);
    testTaggedPointerValue(cls, tag, 1UL << 50);
    testTaggedPointerValue(cls, tag, 1UL << 51);
    testTaggedPointerValue(cls, tag, 1UL << 52);
    testTaggedPointerValue(cls, tag, 1UL << 58);
    testTaggedPointerValue(cls, tag, 1UL << 59);
    testTaggedPointerValue(cls, tag, ~0UL >> 4);
    testTaggedPointerValue(cls, tag, ~0UL);

    // Tagged pointers should bypass refcount tables and autorelease pools
    // and weak reference tables
    WeakContainer *w = [WeakContainer new];

    // force sidetable retain of the WeakContainer before leak checking
    objc_retain(w);
#if !__has_feature(objc_arc)
    // prime method caches before leak checking
    id taggedPointer = (id)_objc_makeTaggedPointer(tag, 1234);
    [taggedPointer retain];
    [taggedPointer release];
    [taggedPointer autorelease];
#endif
    // prime is_debug() before leak checking
    (void)is_debug();

    leak_mark();
    testonthread(^(void) {
        for (uintptr_t i = 0; i < sizeof(w->weaks)/sizeof(w->weaks[0]); i++) {
            id o = (__bridge id)_objc_makeTaggedPointer(tag, i);
            testassert(object_getClass(o) == cls);
            
            id result = WEAK_STORE(w->weaks[i], o);
            testassert(result == o);
            testassert(w->weaks[i] == o);
            
            result = WEAK_LOAD(w->weaks[i]);
            testassert(result == o);
            
            uintptr_t rc = _objc_rootRetainCount(o);
            testassert(rc != 0);
            _objc_rootRelease(o);  testassert(_objc_rootRetainCount(o) == rc);
            _objc_rootRelease(o);  testassert(_objc_rootRetainCount(o) == rc);
            _objc_rootRetain(o);   testassert(_objc_rootRetainCount(o) == rc);
            _objc_rootRetain(o);   testassert(_objc_rootRetainCount(o) == rc);
            _objc_rootRetain(o);   testassert(_objc_rootRetainCount(o) == rc);
#if !__has_feature(objc_arc)
            [o release];  testassert(_objc_rootRetainCount(o) == rc);
            [o release];  testassert(_objc_rootRetainCount(o) == rc);
            [o retain];   testassert(_objc_rootRetainCount(o) == rc);
            [o retain];   testassert(_objc_rootRetainCount(o) == rc);
            [o retain];   testassert(_objc_rootRetainCount(o) == rc);
            objc_release(o);  testassert(_objc_rootRetainCount(o) == rc);
            objc_release(o);  testassert(_objc_rootRetainCount(o) == rc);
            objc_retain(o);   testassert(_objc_rootRetainCount(o) == rc);
            objc_retain(o);   testassert(_objc_rootRetainCount(o) == rc);
            objc_retain(o);   testassert(_objc_rootRetainCount(o) == rc);
#endif
            PUSH_POOL {
                testassert(_objc_rootRetainCount(o) == rc);
                _objc_rootAutorelease(o);
                testassert(_objc_rootRetainCount(o) == rc);
#if !__has_feature(objc_arc)
                [o autorelease];
                testassert(_objc_rootRetainCount(o) == rc);
                objc_autorelease(o);
                testassert(_objc_rootRetainCount(o) == rc);
                objc_retainAutorelease(o);
                testassert(_objc_rootRetainCount(o) == rc);
                objc_autoreleaseReturnValue(o);
                testassert(_objc_rootRetainCount(o) == rc);
                objc_retainAutoreleaseReturnValue(o);
                testassert(_objc_rootRetainCount(o) == rc);
                objc_retainAutoreleasedReturnValue(o);
                testassert(_objc_rootRetainCount(o) == rc);
#endif
            } POP_POOL;
            testassert(_objc_rootRetainCount(o) == rc);
        }
    });
    if (is_debug()) {
        // libobjc's debug lock checking makes this leak check fail
        testwarn("skipping leak check with debug libobjc build");
    } else {
        leak_check(0);
    }
    for (uintptr_t i = 0; i < 10000; i++) {
        testassert(w->weaks[i] != NULL);
        WEAK_STORE(w->weaks[i], NULL);
        testassert(w->weaks[i] == NULL);
        testassert(WEAK_LOAD(w->weaks[i]) == NULL);
    }
    objc_release(w);
    RELEASE_VAR(w);
}

int main()
{
    testassert(objc_debug_taggedpointer_mask != 0);
    testassert(_objc_taggedPointersEnabled());

    PUSH_POOL {
        // Avoid CF's tagged pointer tags because of rdar://11368528

        // Reserved slot should be nil until the 
        // first extended tag is registered.
        // This test no longer works because XPC now uses extended tags.
#define HAVE_XPC_TAGS 1

        uintptr_t extSlot = (~objc_debug_taggedpointer_obfuscator >> objc_debug_taggedpointer_slot_shift) & objc_debug_taggedpointer_slot_mask;
        Class extPlaceholder = objc_getClass("__NSUnrecognizedTaggedPointer");
        testassert(extPlaceholder != nil);

#if !HAVE_XPC_TAGS
        testassert(objc_debug_taggedpointer_classes[extSlot] == nil);
#endif

        _objc_registerTaggedPointerClass(OBJC_TAG_1, 
                                         objc_getClass("TaggedBaseClass60"));
        testGenericTaggedPointer(OBJC_TAG_1, 
                                 objc_getClass("TaggedBaseClass60"));
        
#if !HAVE_XPC_TAGS
        testassert(objc_debug_taggedpointer_classes[extSlot] == nil);
#endif

        _objc_registerTaggedPointerClass(OBJC_TAG_First52BitPayload, 
                                         objc_getClass("TaggedSubclass52"));
        testGenericTaggedPointer(OBJC_TAG_First52BitPayload, 
                                 objc_getClass("TaggedSubclass52"));

        testassert(objc_debug_taggedpointer_classes[extSlot] == extPlaceholder);
        
        _objc_registerTaggedPointerClass(OBJC_TAG_NSManagedObjectID, 
                                         objc_getClass("TaggedNSObjectSubclass"));
        testGenericTaggedPointer(OBJC_TAG_NSManagedObjectID, 
                                 objc_getClass("TaggedNSObjectSubclass"));
    } POP_POOL;

    succeed(__FILE__);
}

// OBJC_HAVE_TAGGED_POINTERS
#else
// not OBJC_HAVE_TAGGED_POINTERS

// Tagged pointers not supported.

int main() 
{
    testassert(objc_debug_taggedpointer_mask == 0);
    succeed(__FILE__);
}

#endif
