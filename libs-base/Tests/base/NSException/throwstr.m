#import "Testing.h"
#import <Foundation/Foundation.h>

int main(void)
{
  START_SET("String throwing")
#if defined(BASE_NATIVE_OBJC_EXCEPTIONS) && BASE_NATIVE_OBJC_EXCEPTIONS == 1
    id caught = nil;
    id thrown = @"thrown";
    @try
      {
	@throw thrown;
      }
    @catch (id str)
      {
	caught = str;
      }
    PASS((caught == thrown), "Throwing an NSConstantString instance before the class is initialised");
#else
    SKIP("Native exceptions not supported")
#endif
  END_SET("String throwing")
  return 0;
}
