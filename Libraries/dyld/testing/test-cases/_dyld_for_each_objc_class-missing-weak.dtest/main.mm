
// BUILD:  $CC missing.m -dynamiclib -o $BUILD_DIR/libmissing.dylib -install_name $BUILD_DIR/libmissing.dylib -lobjc
// BUILD:  $CC lib1.m -dynamiclib -o $BUILD_DIR/liblinked1.dylib -install_name $RUN_DIR/liblinked1.dylib -lobjc
// BUILD:  $CC lib2.m -dynamiclib -o $BUILD_DIR/liblinked2.dylib -install_name $RUN_DIR/liblinked2.dylib -lobjc $BUILD_DIR/libmissing.dylib
// BUILD:  $CXX main.mm -o $BUILD_DIR/_dyld_for_each_objc_class-missing-weak.exe -lobjc $BUILD_DIR/liblinked1.dylib $BUILD_DIR/liblinked2.dylib $BUILD_DIR/libmissing.dylib -lc++

// BUILD: $SKIP_INSTALL $BUILD_DIR/libmissing.dylib

// RUN:  ./_dyld_for_each_objc_class-missing-weak.exe

// liblinked2 weakly links libmissing and so has a missing weak superclass.
// This means we should not see classes from liblinked be returned from _dyld_for_each_objc_class
// liblinked1 itself has classes which are fine so shoud be in the map.
// At runtime, objc is going to walk the images in reverse load order so will see the classes in liblinked2 first
// which are the ones we couldn't optimize.  But objc should then check the closure class map and choose the class
// from liblinked

#include <mach-o/dyld_priv.h>

#import <Foundation/Foundation.h>

#include "test_support.h"

// All the libraries have a copy of DyldClass
@interface DyldClass : NSObject
@end

@implementation DyldClass
@end

// Only the main executable has DyldMainClass
@interface DyldMainClass : NSObject
@end

@implementation DyldMainClass
@end

extern Class OBJC_CLASS_$_DyldClass;
extern Class OBJC_CLASS_$_DyldMainClass;

Class getMainDyldClass() {
  return (Class)&OBJC_CLASS_$_DyldClass;
}

Class getMainDyldMainClass() {
  return (Class)&OBJC_CLASS_$_DyldMainClass;
}

extern "C" id objc_getClass(const char *name);

// Get the DyldClass from liblinked1.dylib
extern "C" id getLinked1DyldClass();

// Get the DyldLinkedClass from liblinked1.dylib
extern "C" id getLinked1DyldLinkedClass();

// Get the DyldLinkedClass from liblinked.dylib
extern "C" id getLinked2DyldLinkedClass();

// Get the DyldClass from libmissing.dylib
// Note, this is weak_import and missing so this must fail
__attribute__((weak_import))
extern "C" id getMissingDyldClass();

static bool gotDyldClassMain = false;
static bool gotDyldClassLinked1 = false;

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
  // This API is only available with dyld3 and shared caches.  If we have dyld2 then don't do anything
  const char* testDyldMode = getenv("TEST_DYLD_MODE");
  assert(testDyldMode);

  size_t unusedCacheLen;
  bool haveSharedCache = _dyld_get_shared_cache_range(&unusedCacheLen) != 0;
  if (!strcmp(testDyldMode, "2") || !haveSharedCache) {
    __block bool sawClass = false;
    _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
      sawClass = true;
    });
    if (sawClass) {
      FAIL("dyld2 shouldn't see any classes");
    }
    PASS("dyld2 or no shared cache");
  }

  // Make sure libmissing.dylib is actually missing
  if (&getMissingDyldClass != nil) {
    FAIL("libmissing needs to be missing");
  }

  // DyldClass in liblinked1 should exist as its superclass is just NSObject
  if (getLinked1DyldClass() == nil) {
    FAIL("liblinked1 DyldClass should exist");
  }

  // DyldLinkedClass in liblinked1 should exist as its superclass is just NSObject
  if (getLinked1DyldLinkedClass() == nil) {
    FAIL("liblinked1 DyldLinkedClass should exist");
  }

  // DyldLinkedClass in liblinked2 should exist as its superclass is just NSObject
  if (getLinked2DyldLinkedClass() == nil) {
    FAIL("liblinked2 DyldLinkedClass should exist");
  }

  // Check that DyldMainClass comes main.exe as that is its only definition
  id runtimeDyldMainClass = objc_getClass("DyldMainClass");
  if (runtimeDyldMainClass != getMainDyldMainClass()) {
    FAIL("DyldMainClass should have come from main.exe");
  }

  // Check that DyldClass comes liblinked1 as it should be missing from liblinked2
  id runtimeDyldClass = objc_getClass("DyldClass");
  if (runtimeDyldClass != getLinked1DyldClass()) {
    FAIL("DyldClass should have come from liblinked1");
  }

  // Check that DyldLinkedClass comes from liblinked2
  // Note, this changes once the objc runtime has adopted our changes.  Don't test it for now
#if 0
  id runtimeDyldLinkedClass = objc_getClass("DyldLinkedClass");
  if (runtimeDyldLinkedClass != getLinked2DyldLinkedClass()) {
    FAIL("DyldLinkedClass should have come from liblinked2");
  }
#endif

  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order liblinked, main exe
    if (!gotDyldClassLinked1) {
      if (classPtr != getLinked1DyldClass()) {
        FAIL("DyldClass should have come from liblinked1");
      }
      if (!isLoaded) {
        FAIL("DyldClass isLoaded should have been set on liblinked1");
      }
      gotDyldClassLinked1 = true;
      return;
    }
    if (!gotDyldClassMain) {
      if (classPtr != getMainDyldClass()) {
        FAIL("DyldClass should have come from main exe");
      }
      if (!isLoaded) {
        FAIL("DyldClass isLoaded should have been set on main exe");
      }
      gotDyldClassMain = true;
      return;
    }
    FAIL("Unexpected DyldClass");
  });

  if (!gotDyldClassLinked1) {
    FAIL("_dyld_for_each_objc_class should have seen DyldClass in liblinked1");
  }

  if (!gotDyldClassMain) {
    FAIL("_dyld_for_each_objc_class should have seen DyldClass in main.exe");
  }

  // Visit again, and return liblinked1's DyldClass
  // Visit again, and return liblinked2's DyldClass
  __block void* dyldClassImpl = nil;
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    dyldClassImpl = classPtr;
    *stop = true;
  });
  if (dyldClassImpl != getLinked1DyldClass()) {
    FAIL("_dyld_for_each_objc_class should have returned DyldClass from liblinked1");
  }

  // Visit again, and return liblinked1's DyldClass
  dyldClassImpl = nil;
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order liblinked, main exe
    // And return the one from main.exe
    if (classPtr == getLinked1DyldClass())
      return;
    dyldClassImpl = classPtr;
    *stop = true;
  });
  if (dyldClassImpl != getMainDyldClass()) {
    FAIL("_dyld_for_each_objc_class should have returned DyldClass from main.exe");
  }

  PASS("SUCCESS");
}
