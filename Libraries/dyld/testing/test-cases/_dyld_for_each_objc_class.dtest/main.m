
// BUILD:  $CC linked1.m -dynamiclib -o $BUILD_DIR/liblinked1.dylib -install_name $RUN_DIR/liblinked1.dylib -lobjc
// BUILD:  $CC linked2.m -dynamiclib -o $BUILD_DIR/liblinked2.dylib -install_name $RUN_DIR/liblinked2.dylib -lobjc
// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_for_each_objc_class.exe $BUILD_DIR/liblinked1.dylib $BUILD_DIR/liblinked2.dylib -lobjc

// RUN:  ./_dyld_for_each_objc_class.exe

// The preoptimized objc class information is available via _dyld_for_each_objc_class().
// This test ensures that we match the objc behaviour when there are duplicates.
// For objc today, it walks the images in reverse load order, so the deepest library will be
// the canonical definition of a class.

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

extern id objc_getClass(const char *name);

// Get the DyldClass from liblinked1.dylib
extern Class getLinked1DyldClass();

// Get the DyldClass from liblinked2.dylib
extern Class getLinked2DyldClass();

// Get the DyldLinkedClass from liblinked2.dylib
extern Class getLinked2DyldLinkedClass();

static bool gotDyldClassMain = false;
static bool gotDyldClassLinked = false;
static bool gotDyldClassLinked2 = false;

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
    PASS("dyld2 or no shared cache)");
  }

  // Check that DyldClass comes from liblinked2 as it is last in load order
  id runtimeDyldClass = objc_getClass("DyldClass");
  if (runtimeDyldClass != getLinked2DyldClass()) {
    FAIL("DyldClass should have come from liblinked2");
  }

  // Check that DyldLinkedClass comes from liblinked2 as it is last in load order
  id runtimeDyldLinkedClass = objc_getClass("DyldLinkedClass");
  if (runtimeDyldLinkedClass != getLinked2DyldLinkedClass()) {
    FAIL("DyldLinkedClass should have come from liblinked2");
  }

  // Walk all the implementations of "DyldClass"
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order liblinked2, liblinked, main exe
    if (!gotDyldClassLinked2) {
      if (classPtr != getLinked2DyldClass()) {
        FAIL("Optimized DyldClass should have come from liblinked2");
      }
      if (!isLoaded) {
        FAIL("Optimized DyldClass isLoaded should have been set on liblinked2");
      }
      gotDyldClassLinked2 = true;
      return;
    }
    if (!gotDyldClassLinked) {
      if (classPtr != getLinked1DyldClass()) {
        FAIL("Optimized DyldClass should have come from liblinked");
      }
      if (!isLoaded) {
        FAIL("Optimized DyldClass isLoaded should have been set on liblinked");
      }
      gotDyldClassLinked = true;
      return;
    }
    if (!gotDyldClassMain) {
      if (classPtr != getMainDyldClass()) {
        FAIL("Optimized DyldClass should have come from main exe");
      }
      if (!isLoaded) {
        FAIL("Optimized DyldClass isLoaded should have been set on main exe");
      }
      gotDyldClassMain = true;
      return;
    }
    FAIL("Unexpected Optimized DyldClass");
  });

  if ( !gotDyldClassLinked2 || !gotDyldClassLinked || !gotDyldClassMain) {
    FAIL("Failed to find all duplicates of 'DyldClass'");
  }

  // Visit again, and return liblinked2's DyldClass
  __block void* dyldClassImpl = nil;
  _dyld_for_each_objc_class("DyldClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    dyldClassImpl = classPtr;
    *stop = true;
  });
  if (dyldClassImpl != getLinked2DyldClass()) {
    FAIL("_dyld_for_each_objc_class should have returned DyldClass from liblinked2");
  }

  // Visit DyldMainClass and make sure it makes the callback for just the result from main.exe
  __block void* dyldMainClassImpl = nil;
  _dyld_for_each_objc_class("DyldMainClass", ^(void* classPtr, bool isLoaded, bool* stop) {
    dyldMainClassImpl = classPtr;
    *stop = true;
  });
  if (dyldMainClassImpl != getMainDyldMainClass()) {
    FAIL("_dyld_for_each_objc_class should have returned DyldMainClass from main.exe");
  }

  PASS("Success");
}
