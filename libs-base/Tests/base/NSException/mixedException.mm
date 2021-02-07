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

/**
 * Tests throwing an exception with the C++ exception throwing mechanism and
 * catching it with the Objective-C exception handling mechanism.  This
 * behaviour is required for compatibility with Apple's new exception model.  
 * The converse should also work - throwing an object with @throw and catching
 * it with catch().
 */
int main(void)
{
  NSString *foo = @"foo";
  id caught = nil;
  int final = 0;
  int wrongCatch = 0;

  @try
    {
      throw foo;
    }
  @catch(NSString *f)
    {
      caught = f;
    }
  @finally
    {
      final = 1;
    }
  PASS(caught == foo, "Unified exception model works correctly");
  PASS(1==final, "@finally works in ObjC++");
  final = 0;
  caught = nil;
  try
    {
      @throw foo;
    }
  catch (NSArray *a)
    {
      wrongCatch = 1;
    }
  catch (NSString *f)
    {
      caught = f;
    }
  catch(...)
    {
      final = 1;
    }
  PASS(0==final, "catchall not used to catch object");
  PASS(0==wrongCatch, "Incorrect object catch not used to catch object");
  PASS(caught == foo, "Unified exception model works correctly");
  return 0;
}
#endif

#else
int main(void)
{
  return 0;
}
#endif
