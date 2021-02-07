#import "Testing.h"
#import <Foundation/NSObject.h>

#if defined(TESTDEV)

#if !__has_feature(objc_nonfragile_abi)
int main(void)
{
  START_SET("Unified exception model")
    SKIP("Unified exception model not supported")
  END_SET("Unified exception model")
  return 0;
}

#else

/**
 * Tests whether void* catches Objective-C objects thrown as exceptions.
 * According to John McCall at Apple, the correct behaviour is not to catch
 * them in void* (which makes sense - void* is effectively a restricted
 * catchall, and we don't want to be catching foreign exceptions there because
 * we don't know what to do with them).  In fact, the current behaviour on OS X
 * (with g++, not tested with clang++) is not simply to not catch them, it is
 * also to crash with a segfault.  Current behaviour with clang + libobjc2 is
 * to do the Right Thing™.
 */
int main(void)
{
  NSString *foo = @"foo";
  id caught = nil;
  int final = 0;
  int wrongCatch = 0;
  try
    {
      @throw foo;
    }
  catch (void *foo)
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
