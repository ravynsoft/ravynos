#import	"Testing.h"
#import	<Foundation/NSRange.h>

/* A ninth test ... skipping unsupported tests
 *
 * If you run the test with 'gnustep-tests example9.m' it should
 * report one set skipped.
 */
int
main()
{
  #define	HAVE_XXX	NO

  /* Start a set.
   */
  START_SET("example set")

    /* Here we conditionally skip the set with a message to be displayed.
     * The first line will be displayed immediately when the set
     * is skipped, and lets the user know that some functionality is missing.
     * The remainder of the message is written to the log file so the user
     * can find out what to do about the problem.
     */
    if (!HAVE_XXX)
      SKIP("Feature 'foo' is unsupported.\nThis is because the package was built without the 'XXX' library.\nIf you need 'foo' then please obtain 'XXX' and build and install the package again before re-running this testsuite.")

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
