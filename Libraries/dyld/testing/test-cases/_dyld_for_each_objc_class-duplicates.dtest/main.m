
// BUILD:  $CC linked1.m -dynamiclib -o $BUILD_DIR/liblinked1.dylib -install_name $RUN_DIR/liblinked1.dylib -lobjc
// BUILD:  $CC linked2.m -dynamiclib -o $BUILD_DIR/liblinked2.dylib -install_name $RUN_DIR/liblinked2.dylib -lobjc
// BUILD:  $CC main.m -o $BUILD_DIR/_dyld_for_each_objc_class-duplicates.exe $BUILD_DIR/liblinked1.dylib $BUILD_DIR/liblinked2.dylib -lobjc -framework Foundation

// RUN:  ./_dyld_for_each_objc_class-duplicates.exe

#include <mach-o/dyld_priv.h>
#include <stdlib.h>
#include <string.h>

#import <Foundation/NSObject.h>

#include "test_support.h"

// All the libraries have a copy of NSString
@interface NSString : NSObject
@end

@implementation NSString
@end

extern Class OBJC_CLASS_$_NSString;

// The main executable also has versions of these Foundation classes
@interface NSDictionary : NSObject
@end

@implementation NSDictionary
@end

extern Class OBJC_CLASS_$_NSDictionary;

@interface NSError : NSObject
@end

@implementation NSError
@end

extern Class OBJC_CLASS_$_NSError;

@interface NSSet : NSObject
@end

@implementation NSSet
@end

extern Class OBJC_CLASS_$_NSSet;

@interface NSArray : NSObject
@end

@implementation NSArray
@end

extern Class OBJC_CLASS_$_NSArray;

Class getMainNSString() {
  return (Class)&OBJC_CLASS_$_NSString;
}

extern id objc_getClass(const char *name);

// Get the NSString from liblinked1.dylib
extern Class getLinked1NSString();

// Get the NSString from liblinked2.dylib
extern Class getLinked2NSString();

static bool gotNSStringMain = false;
static bool gotNSStringLinked = false;
static bool gotNSStringLinked2 = false;
static bool gotNSStringFoundation = false;

void testDuplicate(const char* className, Class nonCacheClass) {
  // Walk all the implementations of the class.  There should be 2.  One in the executable and one in the shared cache
  // The shared cache one should be returned first.

  // The objc runtime should have chosen the Foundation one as the canonical definition.
  Class objcRuntimeClassImpl = (Class)objc_getClass(className);
  if (objcRuntimeClassImpl == nil) {
    FAIL("class %s not found via runtime", className);
  }

  if (objcRuntimeClassImpl == nonCacheClass) {
    FAIL("class %s from runtime should not match main exexutable", className);
  }

  __block bool foundSharedCacheImpl = false;
  __block bool foundMainExecutableImpl = false;
  __block bool foundAnyClass = false;
  __block bool foundTooManyClasses = false;
  _dyld_for_each_objc_class(className, ^(void* classPtr, bool isLoaded, bool* stop) {
    foundAnyClass = true;

    // We should walk these in the order Foundation, main exe
    if (!foundSharedCacheImpl) {
      if (classPtr != objcRuntimeClassImpl) {
        FAIL("Optimized class %s should have come from Foundation", className);
      }
      if (!isLoaded) {
        FAIL("Optimized class %s isLoaded should have been set on Foundation", className);
      }
      foundSharedCacheImpl = true;
      return;
    }
    if (!foundMainExecutableImpl) {
      if (classPtr != nonCacheClass) {
        FAIL("Optimized class %s should have come from main executable", className);
      }
      if (!isLoaded) {
        FAIL("Optimized class %s isLoaded should have been set on main executable", className);
      }
      foundMainExecutableImpl = true;
      return;
    }

    foundTooManyClasses = true;
    FAIL("class %s found somewhere other than main executable and Foundation", className);
  });

  if (!foundAnyClass) {
    FAIL("class %s not found", className);
  }

  if (foundTooManyClasses) {
    FAIL("class %s found too many times", className);
  }

  if (!foundSharedCacheImpl || !foundMainExecutableImpl) {
    FAIL("class %s not found for shared cache or main executable", className);
  }
}

int main(int argc, const char* argv[], const char* envp[], const char* apple[]) {
  // This API is only available with dyld3 and shared caches.  If we have dyld2 then don't do anything
  const char* testDyldMode = getenv("TEST_DYLD_MODE");
  assert(testDyldMode);

  size_t sharedCacheLen = 0;
  const void* sharedCacheStart = 0;
  sharedCacheStart = _dyld_get_shared_cache_range(&sharedCacheLen);
  bool haveSharedCache = sharedCacheStart != NULL;
  if (!strcmp(testDyldMode, "2") || !haveSharedCache) {
    __block bool sawClass = false;
    _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
      sawClass = true;
    });
    if (sawClass) {
      FAIL("dyld2 shouldn't see any classes");
    }
    PASS("dyld2 or no shared cache)");
  }

  // Check that NSString comes from Foundation as the shared cache should win here.
  id runtimeNSString = objc_getClass("NSString");
  if ( (uint64_t)runtimeNSString < (uint64_t)sharedCacheStart ) {
    FAIL("NSString should have come from Foundation but instead was %p", runtimeNSString);
  }
  if ( (uint64_t)runtimeNSString >= ((uint64_t)sharedCacheStart + sharedCacheLen) ) {
    FAIL("NSString should have come from Foundation but instead was %p", runtimeNSString);
  }

  // Walk all the implementations of "NSString"
  _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
    // We should walk these in the order Foundation, liblinked2, liblinked, main exe
    if (!gotNSStringFoundation) {
      if (classPtr != runtimeNSString) {
        FAIL("Optimized NSString should have come from Foundation");
      }
      if (!isLoaded) {
        FAIL("Optimized NSString isLoaded should have been set on Foundation");
      }
      gotNSStringFoundation = true;
      return;
    }
    if (!gotNSStringLinked2) {
      if (classPtr != getLinked2NSString()) {
        FAIL("Optimized NSString should have come from liblinked2");
      }
      if (!isLoaded) {
        FAIL("Optimized NSString isLoaded should have been set on liblinked2");
      }
      gotNSStringLinked2 = true;
      return;
    }
    if (!gotNSStringLinked) {
      if (classPtr != getLinked1NSString()) {
        FAIL("Optimized NSString should have come from liblinked");
      }
      if (!isLoaded) {
        FAIL("Optimized NSString isLoaded should have been set on liblinked");
      }
      gotNSStringLinked = true;
      return;
    }
    if (!gotNSStringMain) {
      if (classPtr != getMainNSString()) {
        FAIL("Optimized NSString should have come from main exe");
      }
      if (!isLoaded) {
        FAIL("Optimized NSString isLoaded should have been set on main exe");
      }
      gotNSStringMain = true;
      return;
    }
    FAIL("Unexpected Optimized NSString");
  });

  if ( !gotNSStringFoundation || !gotNSStringLinked2 || !gotNSStringLinked || !gotNSStringMain) {
    FAIL("Failed to find all duplicates of 'NSString'");
  }

  // Visit again, and return Foundation's NSString
  __block void* NSStringImpl = nil;
  _dyld_for_each_objc_class("NSString", ^(void* classPtr, bool isLoaded, bool* stop) {
    NSStringImpl = classPtr;
    *stop = true;
  });
  if (NSStringImpl != runtimeNSString) {
    FAIL("_dyld_for_each_objc_class should have returned NSString from Foundation");
  }

  testDuplicate("NSDictionary", (Class)&OBJC_CLASS_$_NSDictionary);
  testDuplicate("NSError", (Class)&OBJC_CLASS_$_NSError);
  testDuplicate("NSSet", (Class)&OBJC_CLASS_$_NSSet);
  testDuplicate("NSArray", (Class)&OBJC_CLASS_$_NSArray);

  PASS("Success");
}
