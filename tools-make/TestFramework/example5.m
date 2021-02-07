#import	"Testing.h"

/* A fifth test ... hope.
 *
 * If you run the test with 'gnustep-tests example5.m' it should
 * report one hope dashed and two test passes.
 */
int
main()
{
  START_SET("example set")

  /* First set a flag to say that we are not expecting tests to
   * actually pass.
   */
  testHopeful = YES;

  /* Here the test should result in a dashed hope rather than a fail.
   */
  PASS(1 == 0, "silly test which we don't expect to pass")

  /* This test should simply pass of course.
   */
  PASS(1 == 1, "integer equality works")

  END_SET("example set")

  /* And here we demonstrate that on exit from the set, the global
   * variable is restored to its state on entry.
   */
  pass(NO == testHopeful, "the flag state is restored outside the set");

  return 0;
}
