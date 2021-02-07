#import	"Testing.h"
#import	<Foundation/NSRange.h>

/* An eighth test ... complex code fragments
 *
 * If you run the test with 'gnustep-tests example8.m' it should
 * report one test pass.
 */
int
main()
{
  /* Start a set.
   */
  START_SET("example set")

  /* Here we demonstrate that the 'expression' evaluated by the PASS
   * macro can actually be an arbitrarily complex piece of code as
   * long as the last statement returns an integral value which can
   * be used to represent a pass (non zero) or fail (if zero).
   * Where such a code fragment contains commas, it must be written
   * inside brackets to let the macro preprocessor know that the whole
   * code fragement is the first parameter to the macro.
   */
  PASS(({
    NSRange	r = NSMakeRange(1, 10);
    
    NSEqualRanges(r, NSMakeRange(1, 10));
  }), "a long code-fragment/expression works")

  END_SET("example set")

  return 0;
}
