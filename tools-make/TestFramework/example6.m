#import	"Testing.h"

/* A sixth test ... need.
 *
 * If you run the test with 'gnustep-tests example6.m' it should
 * report one hope dashed, one test pass, and one set failed.
 */
int
main()
{
  START_SET("example set")

  /* First set a flag to say that we are not expecting tests to
   * actually pass.
   */
  testHopeful = YES;

  /* Here we demonstrate the NEED macro.  This says that a need must be met
   * in order to continue with the set ... if the test does not pass
   * we can't complete the set and we skip to the end.
   */
  NEED(PASS(1 == 0, "silly test which we don't expect to pass"))

  /* This test should never be reached because the previous test needs to
   * pass, but will not do so.
   */
  PASS(1 == 1, "integer equality works")

  /* The set should be unresolved, because the earlier need was not met.
   */
  END_SET("example set")

  /* And here we demonstrate that on exit from the set, the global
   * variable is restored to its state on entry.
   */
  pass(NO == testHopeful, "the flag state is restored outside the set");

  return 0;
}
