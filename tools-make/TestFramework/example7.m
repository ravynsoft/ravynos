#import	"Testing.h"

/* A seventh test ... nesting sets.
 *
 * If you run the test with 'gnustep-tests example7.m' it should
 * report a one test fail, two test passes, and one set failed.
 */
int
main()
{
  /* Start a set.
   */
  START_SET("outer set")

  /* Our first test in this set will pass.
   */
  PASS(1 == 1, "integer equality works")

  /* Now we start a set nested inside the first one.
   */
  START_SET("inner set")

  /* And we say we need a test to pass, but it's actually a faulty one
   * which will fail, causing the set to be terminated.
   */
  NEED(PASS_EQUAL(@"hello", @"there", "faulty string equality test"))

  /* Here's a correct string equality test, but it's never reached because
   * the earlier test was needed.
   */
  PASS_EQUAL(@"there", @"there", "NSString equality works")

  END_SET("inner set")

  /* And here's another correct test which *is* reached because it's
   * in a different set.
   */
  PASS_EQUAL(@"there", @"there", "NSString equality works")

  END_SET("outer set")

  return 0;
}
