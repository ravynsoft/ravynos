#import "Testing.h"
#import <Foundation/NSObject.h>

#if defined(TESTDEV)

#if !__has_feature(objc_nonfragile_abi)
int main(void)
{
  START_SET("String throwing")
    SKIP("Unified exception model not supported")
  END_SET("String throwing");
  return 0;
}

#else
#if __has_include(<objc/capabilities.h>)
#include <objc/capabilities.h>
#endif

NSObject *foo = @"foo";
id caughtObj = nil;
id caughtStr = nil;
int final = 0;

void testThrow(void)
{
  caughtObj = nil;
  caughtStr = nil;
  final = 0;
  try
    {
      throw foo;
    }
  catch (NSString *f)
    {
      caughtStr = f;
    }
  catch (NSObject *o)
    {
      caughtObj = o;
    }
  catch(...)
    {
      final = 1;
    }
}
/**
 * Tests whether C++ exception catching uses Objective-C semantics.
 *
 * This is a completely stupid thing to do, because it means that a C++
 * language feature now doesn't behave like a C++ language feature, but Apple
 * did it so we have to do stupid things in the name of compatibility...
 */
int main(void)
{
  testThrow();
  PASS(0==final, "catchall not used to catch object");
  PASS(caughtObj == nil, "Thrown string cast to NSObject matched NSObject (Apple mode)");
  PASS(caughtStr == foo, "Thrown string cast to NSObject matched NSString (Apple mode)");
#ifdef OBJC_UNIFIED_EXCEPTION_MODEL
  // Turn off Apple compatibility mode and try again
  objc_set_apple_compatible_objcxx_exceptions(0);
  testThrow();
  PASS(0==final, "catchall not used to catch object");
  PASS(caughtObj == foo, "Thrown string cast to NSObject matched NSObject (sane mode)");
  PASS(caughtStr == nil, "Thrown string cast to NSObject matched NSString (sane mode)");
#endif
  return 0;
}
#endif
#else
int main(void)
{
  return 0;
}
#endif
