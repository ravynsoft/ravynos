// TEST_CONFIG

#include "test.h"

#include "testroot.i"
#include <objc/runtime.h>
#include <string.h>

@protocol Proto
-(void) instanceMethod;
+(void) classMethod;
@optional
-(void) instanceMethod2;
+(void) classMethod2;
@end

@protocol Proto2
-(void) instanceMethod;
+(void) classMethod;
@optional
-(void) instanceMethod2;
+(void) classMethod_that_does_not_exist;
@end

@protocol Proto3
-(void) instanceMethod;
+(void) classMethod_that_does_not_exist;
@optional
-(void) instanceMethod2;
+(void) classMethod2;
@end

static int super_initialize;
static int super_cxxctor;
static int super_cxxdtor;

struct super_cxx {
    int foo;
    super_cxx() : foo(0) {
        super_cxxctor++;
    }
    ~super_cxx() {
        super_cxxdtor++;
    }
};

@interface Super : TestRoot
@property int superProp;
@end
@implementation Super {
    super_cxx _foo;
}
@dynamic superProp;
+(void)initialize { super_initialize++; } 

+(void) classMethod { fail("+[Super classMethod] called"); }
+(void) classMethod2 { fail("+[Super classMethod2] called"); }
-(void) instanceMethod { fail("-[Super instanceMethod] called"); }
-(void) instanceMethod2 { fail("-[Super instanceMethod2] called"); }
@end

static int state;

static void instance_fn(id self, SEL _cmd __attribute__((unused)))
{
    testassert(!class_isMetaClass(object_getClass(self)));
    state++;
}

static void class_fn(id self, SEL _cmd __attribute__((unused)))
{
    testassert(class_isMetaClass(object_getClass(self)));
    state++;
}

static void fail_fn(id self __attribute__((unused)), SEL _cmd)
{
    fail("fail_fn '%s' called", sel_getName(_cmd));
}


static void cycle(void)
{    
    Class cls;
    BOOL ok;
    objc_property_t prop;
    char namebuf[256];
    
    testassert(!objc_getClass("Sub"));
    testassert([Super class]);

    // Test subclass with bells and whistles
    
    cls = objc_allocateClassPair([Super class], "Sub", 0);
    testassert(cls);
    
    class_addMethod(cls, @selector(instanceMethod), 
                    (IMP)&instance_fn, "v@:");
    class_addMethod(object_getClass(cls), @selector(classMethod), 
                    (IMP)&class_fn, "v@:");
    class_addMethod(object_getClass(cls), @selector(initialize), 
                    (IMP)&class_fn, "v@:");
    class_addMethod(object_getClass(cls), @selector(load), 
                    (IMP)&fail_fn, "v@:");

    ok = class_addProtocol(cls, @protocol(Proto));
    testassert(ok);
    ok = class_addProtocol(cls, @protocol(Proto));
    testassert(!ok);

    char attrname[2];
    char attrvalue[2];
    objc_property_attribute_t attrs[1];
    unsigned int attrcount = sizeof(attrs) / sizeof(attrs[0]);

    attrs[0].name = attrname;
    attrs[0].value = attrvalue;
    strcpy(attrname, "T");
    strcpy(attrvalue, "x");

    strcpy(namebuf, "subProp");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(ok);
    strcpy(namebuf, "subProp");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(!ok);
    strcpy(attrvalue, "i");
    class_replaceProperty(cls, namebuf, attrs, attrcount);
    strcpy(namebuf, "superProp");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(!ok);
    bzero(namebuf, sizeof(namebuf));
    bzero(attrs, sizeof(attrs));
    bzero(attrname, sizeof(attrname));
    bzero(attrvalue, sizeof(attrvalue));

#ifndef __LP64__
# define size 4
# define align 2
#else
#define size 8
# define align 3
#endif

    /*
      {
        int ivar;
        id ivarid;
        id* ivaridstar;
        Block_t ivarblock;
      }
    */
    ok = class_addIvar(cls, "ivar", 4, 2, "i");
    testassert(ok);
    ok = class_addIvar(cls, "ivarid", size, align, "@");
    testassert(ok);
    ok = class_addIvar(cls, "ivaridstar", size, align, "^@");
    testassert(ok);
    ok = class_addIvar(cls, "ivarblock", size, align, "@?");
    testassert(ok);

    ok = class_addIvar(cls, "ivar", 4, 2, "i");
    testassert(!ok);
    ok = class_addIvar(object_getClass(cls), "classvar", 4, 2, "i");
    testassert(!ok);

    objc_registerClassPair(cls);

    // should call cls's +initialize, not super's
    // Provoke +initialize using class_getMethodImplementation(class method)
    //   in order to test getNonMetaClass's slow case
    super_initialize = 0;
    state = 0;
    class_getMethodImplementation(object_getClass(cls), @selector(class));
    testassert(super_initialize == 0);
    testassert(state == 1);

    testassert(cls == [cls class]);
    testassert(cls == objc_getClass("Sub"));

    testassert(!class_isMetaClass(cls));
    testassert(class_isMetaClass(object_getClass(cls)));

    testassert(class_getSuperclass(cls) == [Super class]);
    testassert(class_getSuperclass(object_getClass(cls)) == object_getClass([Super class]));

    testassert(class_getInstanceSize(cls) >= sizeof(Class) + 4 + 3*size);
    testassert(class_conformsToProtocol(cls, @protocol(Proto)));

    class_addMethod(cls, @selector(instanceMethod2), 
                    (IMP)&instance_fn, "v@:");
    class_addMethod(object_getClass(cls), @selector(classMethod2), 
                    (IMP)&class_fn, "v@:");

    ok = class_addIvar(cls, "ivar2", 4, 4, "i");
    testassert(!ok);
    ok = class_addIvar(object_getClass(cls), "classvar2", 4, 4, "i");
    testassert(!ok);

    ok = class_addProtocol(cls, @protocol(Proto2));
    testassert(ok);
    ok = class_addProtocol(cls, @protocol(Proto2));
    testassert(!ok);
    ok = class_addProtocol(cls, @protocol(Proto));
    testassert(!ok);

    attrs[0].name = attrname;
    attrs[0].value = attrvalue;
    strcpy(attrname, "T");
    strcpy(attrvalue, "i");

    strcpy(namebuf, "subProp2");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(ok);
    strcpy(namebuf, "subProp");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(!ok);
    strcpy(namebuf, "superProp");
    ok = class_addProperty(cls, namebuf, attrs, attrcount);
    testassert(!ok);
    bzero(namebuf, sizeof(namebuf));
    bzero(attrs, sizeof(attrs));
    bzero(attrname, sizeof(attrname));
    bzero(attrvalue, sizeof(attrvalue));

    prop = class_getProperty(cls, "subProp");
    testassert(prop);
    testassert(0 == strcmp(property_getName(prop), "subProp"));
    testassert(0 == strcmp(property_getAttributes(prop), "Ti"));
    prop = class_getProperty(cls, "subProp2");
    testassert(prop);
    testassert(0 == strcmp(property_getName(prop), "subProp2"));
    testassert(0 == strcmp(property_getAttributes(prop), "Ti"));

    // note: adding more methods here causes a false leak check failure
    state = 0;
    [cls classMethod];
    [cls classMethod2];
    testassert(state == 2);

    // put instance tests on a separate thread so they 
    // are reliably deallocated before class destruction
    testonthread(^{
        super_cxxctor = 0;
        super_cxxdtor = 0;
        id obj = [cls new];
        testassert(super_cxxctor == 1);
        testassert(super_cxxdtor == 0);
        state = 0;
        [obj instanceMethod];
        [obj instanceMethod2];
        testassert(state == 2);
        RELEASE_VAR(obj);
        testassert(super_cxxctor == 1);
        testassert(super_cxxdtor == 1);
    });

    // Test ivar layouts of sub-subclass
    Class cls2 = objc_allocateClassPair(cls, "SubSub", 0);
    testassert(cls2);

    /*
      {
        id ivarid2;
        id idarray[16];
        void* ptrarray[16];
        char a;
        char b;
        char c;
      }
    */
    ok = class_addIvar(cls2, "ivarid2", size, align, "@");
    testassert(ok);
    ok = class_addIvar(cls2, "idarray", 16*sizeof(id), align, "[16@]");
    testassert(ok);
    ok = class_addIvar(cls2, "ptrarray", 16*sizeof(void*), align, "[16^]");
    testassert(ok);
    ok = class_addIvar(cls2, "a", 1, 0, "c");
    testassert(ok);    
    ok = class_addIvar(cls2, "b", 1, 0, "c");
    testassert(ok);    
    ok = class_addIvar(cls2, "c", 1, 0, "c");
    testassert(ok);    

    objc_registerClassPair(cls2);

    // 1-byte ivars should be well packed
    testassert(ivar_getOffset(class_getInstanceVariable(cls2, "b")) == 
               ivar_getOffset(class_getInstanceVariable(cls2, "a")) + 1);
    testassert(ivar_getOffset(class_getInstanceVariable(cls2, "c")) == 
               ivar_getOffset(class_getInstanceVariable(cls2, "b")) + 1);

    objc_disposeClassPair(cls2);
    objc_disposeClassPair(cls);
    
    testassert(!objc_getClass("Sub"));

    // fixme test layout setters
}

int main()
{
    int count = 5000;

    // fixme even with this long warmup we still
    // suffer false 4096-byte leaks occasionally.
    for (int i = 0; i < 500; i++) {
        testonthread(^{ cycle(); });
    }

    leak_mark();
    while (count--) {
        testonthread(^{ cycle(); });
    }
    leak_check(4096);

    succeed(__FILE__);
}

