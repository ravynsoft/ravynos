#import	"Testing.h"

/* A second test ... your first go at testing with ObjectiveC
 *
 * If you run the test with 'gnustep-tests example2.m' it should
 * report two test passes.
 */
int
main()
{
  /* We start a set here ...
   * Having a set means we do not need to bother creating an autorelease pool.
   */
  START_SET("example set")

  pass(1 == 1, "integer equality works");
  pass([[NSObject new] autorelease] != nil, "+new creates an object");

  END_SET("example set")

  return 0;
}
