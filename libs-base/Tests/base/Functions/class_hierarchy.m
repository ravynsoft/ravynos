#import "Testing.h"
#import <Foundation/Foundation.h>

// OS X doesn't seem to provide all of the runtime functions by default.
#ifndef GNUSTEP
#	include        <objc/runtime.h>
#endif

// Simple root class - test that the runtime does the right thing with new
// roots
@interface Bar { id isa; } @end
@implementation Bar @end


// Some simple classes that inherit from root classes in Foundation.  Test that
// they go in the right places in the runtime.
@interface Foo : NSObject @end
@implementation Foo @end
@interface Foo2 : Foo @end
@implementation Foo2 @end

@interface Proxy : NSProxy @end
@implementation Proxy @end
@interface Proxy2 : Proxy @end
@implementation Proxy2 @end

/**
 * Tests a root class has its superclass and metaclass correctly configured
 */
void testRootClass(const char *clsName)
{
  testHopeful = YES;
  START_SET("testRootClass")
    Class cls = objc_getClass(clsName);
    Class super = class_getSuperclass(cls);
    Class meta = object_getClass(cls);
    Class superMeta = class_getSuperclass(meta);
    Class metaMeta = object_getClass(meta);

    PASS(Nil == super, "superclass of root class is Nil");
    PASS(metaMeta == meta,
      "root class's metaclass is also its metaclass's metaclass");
    PASS(cls == superMeta, "Root class is its metaclass's superclass");
  END_SET("testRootClass")
}

/**
 * Tests a non-root class and its metaclass are in the correct place in the
 * hierarchy.
 */
void testNonRootClass(const char *clsName)
{
  testHopeful = YES;
  START_SET("testNonRootClass")
    Class cls = objc_getClass(clsName);
    Class super = class_getSuperclass(cls);
    Class meta = object_getClass(cls);
    Class superMeta = class_getSuperclass(meta);
    Class metaMeta = object_getClass(meta);
    Class metaSuper = object_getClass(super);
    Class root = super;
    Class rootMeta = Nil;

    do
      {
	rootMeta = object_getClass(root);
	root = class_getSuperclass(root);
      } while (root != Nil);

    PASS(Nil != super, "Non-root class has a superclass");
    PASS(superMeta == metaSuper,
      "Metaclass's superclass is superclass's metaclass");
    PASS(rootMeta == metaMeta,
      "Metaclass's metaclass is root class's metaclass");
  END_SET("testNonRootClass")
}

int main(void)
{
  testRootClass("NSObject");
  testRootClass("NSProxy");
  testRootClass("Bar");
  testNonRootClass("Foo");
  testNonRootClass("Foo2");
  testNonRootClass("Proxy");
  testNonRootClass("Proxy2");
  return 0;
}
